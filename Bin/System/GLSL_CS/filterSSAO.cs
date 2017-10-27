
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filterSSAO.cs
//
//	composition toolkit filter
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////
// next uniforms are STANDARTIZED !!

uniform vec2	renderTexelSize;	// size of a renderer textures (depth, normal, mask)
uniform vec2	renderSize;			
uniform vec2	outputTexelSize;	// texelSize, stored in a format ( 1.0/w, 1.0/h )
uniform float	previewScaleFactor; // 1.0 for full image, 0.5 for half size

// #define SAMPLES 4

uniform float 		cameraNearPlane;	// camera near plane
uniform float 		cameraFarPlane;	// camera far plane

//////////////////////////////////////

layout (std430, binding = 0) buffer DataBuffer
{
	vec4		viewDir[4];
	vec4		viewOrig[4];
	vec4		filterRadius;		// = vec2( 0.02, 0.02 );
	
	float 		distanceThreshold;	// = 10.5;
	float			only_ao;

	float		gamma;			// = 1.0;
	float		contrast;		// = 1.0;
	
} dataBuffer;

layout(binding=0, rgba8) uniform writeonly image2D resultImage;
layout(binding=1) 		uniform sampler2D 	colorSampler;
#ifdef USE_MS
	layout(binding=2) 		uniform sampler2DMS 	depthSampler;
	layout(binding=3) 		uniform sampler2DMS 	normalSampler;
#else
	layout(binding=2) 		uniform sampler2D 		depthSampler;
	layout(binding=3) 		uniform sampler2D 		normalSampler;
#endif

const int 	sample_count = 49;
const vec2 oflut[49] = vec2[](
    vec2(-0.864,-0.930),
    vec2(-0.952,-0.608),
    vec2(-0.820,-0.318),
    vec2(-0.960,-0.076),
    vec2(-0.939,0.244),
    vec2(-0.827,0.421),
    vec2(-0.871,0.561),
    vec2(-0.668,-0.810),
    vec2(-0.749,-0.557),
    vec2(-0.734,-0.422),
    vec2(-0.681,-0.043),
    vec2(-0.598,0.132),
    vec2(-0.720,0.323),
    vec2(-0.513,0.723),
    vec2(-0.371,-0.829),
    vec2(-0.366,-0.735),
    vec2(-0.259,-0.318),
    vec2(-0.344,-0.079),
    vec2(-0.386,0.196),
    vec2(-0.405,0.425),
    vec2(-0.310,0.518),
    vec2(-0.154,-0.931),
    vec2(-0.171,-0.572),
    vec2(-0.241,-0.420),
    vec2(-0.130,-0.225),
    vec2(-0.092,0.189),
    vec2(-0.018,0.310),
    vec2(-0.179,0.512),
    vec2(0.133,-0.942),
    vec2(0.235,-0.738),
    vec2(0.229,-0.380),
    vec2(0.027,-0.030),
    vec2(0.052,0.183),
    vec2(0.141,0.415),
    vec2(0.129,0.736),
    vec2(0.340,-0.931),
    vec2(0.254,-0.564),
    vec2(0.388,-0.417),
    vec2(0.363,-0.104),
    vec2(0.413,0.234),
    vec2(0.421,0.321),
    vec2(0.423,0.653),
    vec2(0.631,-0.756),
    vec2(0.665,-0.736),
    vec2(0.551,-0.350),
    vec2(0.526,-0.220),
    vec2(0.519,0.053),
    vec2(0.750,0.321),
    vec2(0.736,0.641) );

#include "misc_depth.cs"
#include "misc_masking.cs"

void InterpolateViewParams(in vec2 uv, out vec4 viewOrig, out vec4 viewDir)
{
	vec4 north = mix( dataBuffer.viewDir[1], dataBuffer.viewDir[2], uv.x );
	vec4 source = mix( dataBuffer.viewDir[0], dataBuffer.viewDir[3], uv.x );
	viewDir = mix( north, source, 1.0-uv.y );
	
	north = mix( dataBuffer.viewOrig[1], dataBuffer.viewOrig[2], uv.x );
	source = mix( dataBuffer.viewOrig[0], dataBuffer.viewOrig[3], uv.x );
	viewOrig = mix( north, source, 1.0-uv.y );
}

void calculatePosition(in vec2 uv, in ivec2 iuv, in int nsample, out vec3 pos)
{
	float d = texelFetch(depthSampler, iuv, nsample).x;
		
	float Depth = 1.0;
	ComputeDepth( d, Depth );
	
	vec4 viewOrig, viewDir;
	InterpolateViewParams(uv, viewOrig, viewDir);
	
	pos = viewOrig.xyz + Depth * (viewDir.xyz - viewOrig.xyz);
}
	
void decodeNormal(in ivec2 uv, in int nsample, out vec3 normal)
{
	normal = normalize(texelFetch(normalSampler, uv, nsample).xyz);
}

/*
// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }


// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
*/
float CalculateAO(in vec2 texCoord, in float k1, in float k2, in int nsample, in vec3 viewPos, in vec3 viewNormal)
{
	// perform AO
	float ambientOcclusion = 0.0;
	vec2 screenDimentions = vec2( 1.0/outputTexelSize.x, 1.0/outputTexelSize.y );
	screenDimentions = renderSize;
	
	vec3 samplePos;
	
    for (int i = 0; i < sample_count; ++i)
    {
		vec3 of = vec3( oflut[i].x*k1 - oflut[i].y*k2,
                        oflut[i].x*k2 + oflut[i].y*k1, 1.0 );
						
		//vec3 randomNormal = vec3( of.x, of.y, gl_TexCoord[0].s + gl_TexCoord[0].t - 2.2 );
		//mat3 rotateMatrix = GetRotationMatrix( viewNormal, randomNormal );
		
		//of = rotateMatrix * of;
	
        // sample at an offset specified by the current Poisson-Disk sample and scale it by a radius (has to be in Texture-Space)
        vec2 sampleTexCoord = texCoord + dataBuffer.filterRadius.xy * of.st;
		vec2 rasterCoord = sampleTexCoord * screenDimentions;
		
        calculatePosition( sampleTexCoord, ivec2(rasterCoord.st), nsample, samplePos );
        vec3 sampleDir = normalize(samplePos - viewPos);

        // angle between SURFACE-NORMAL and SAMPLE-DIRECTION (vector from SURFACE-POSITION to SAMPLE-POSITION)
        float NdotS = max(dot(viewNormal, sampleDir), 0);
        // distance between SURFACE-POSITION and SAMPLE-POSITION
        float VPdistSP = distance(viewPos, samplePos);

        // a = distance function
        float a = 1.0 - smoothstep(dataBuffer.distanceThreshold, dataBuffer.distanceThreshold * 2, VPdistSP);
		//a = 1.0;
        // b = dot-Product
		if (abs(NdotS) < 0.15) NdotS = 0.0;
        float b = NdotS;

        ambientOcclusion += (a * b);
    }

    return 1.0 - (ambientOcclusion / sample_count);
}


/////////////////////////////////////////////////////////////////////////////
// MAIN
void main(void)
{
	// Assign these variables now because they will be used next
	ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
	vec2 texCoord = vec2(outputTexelSize.x * (0.5 + pixelPosition.x), outputTexelSize.y * (0.5 + pixelPosition.y));
	
#ifdef SAME_SIZE
	vec4 srccolor = texelFetch(colorSampler, pixelPosition, 0);
#else
	// do some scaling with bilinear filtering
	vec4 srccolor = texture(colorSampler, texCoord );
#endif
	
	float ao = 0.0;
	
	float an = fract(fract(texCoord.x*0.36257507)*0.38746515+texCoord.y*0.32126721);

    float k1=cos(6.2831*an);
    float k2=sin(6.2831*an);
	
	vec3 viewPos, viewNormal;
	
#ifdef USE_MS

	for (int nsample=0; nsample<SAMPLES; ++nsample)
	{
		// reconstruct position from depth, USE YOUR CODE HERE
		calculatePosition(texCoord, pixelPosition, nsample, viewPos);

		// get the view space normal, USE YOUR CODE HERE
		decodeNormal(pixelPosition, nsample, viewNormal);
		
		//
		ao += CalculateAO(texCoord, k1, k2, nsample, viewPos, viewNormal);
		
	}
	ao /= float(SAMPLES);
	
#else
	ivec2 pixelPosition2 = ivec2(texCoord * renderSize);
	// reconstruct position from depth, USE YOUR CODE HERE
	calculatePosition(texCoord, pixelPosition2, 0, viewPos);

	// get the view space normal, USE YOUR CODE HERE
	decodeNormal(pixelPosition2, 0, viewNormal);

	//
	ao = CalculateAO(texCoord, k1, k2, 0, viewPos, viewNormal);

#endif

	//scale occlusion toward white.
    ao = clamp(ao, 0.0, 1.0);
	ao = dataBuffer.contrast * pow(ao, dataBuffer.gamma);
	
	vec4 outColor = vec4(ao*srccolor.r, ao*srccolor.g, ao*srccolor.b, srccolor.a);
	//outColor = vec4(ao, ao, ao, 1.0);
	/*
	if (dataBuffer.only_ao > 0.0)
	{
		outColor = vec4(ao, ao, ao, 1.0);
	}
*/
	outColor = ApplyMask(texCoord, srccolor, outColor);
	imageStore(resultImage, pixelPosition, outColor);
}

