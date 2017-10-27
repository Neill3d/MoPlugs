
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: layerShadow.cs
//
//	composition toolkit
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

uniform vec2	outputSize;	// stored in a format ( 1.0/w, 1.0/h )
uniform vec2	renderSize;
uniform vec2	renderBorder;
uniform float	previewScaleFactor; // 1.0 for full image, 0.5 for half size

layout(binding=0, rgba8) uniform writeonly image2D resultImage;
//layout(binding=5)		uniform sampler2D	positionSampler;

// #define SAMPLES 4

#ifdef USE_MS
	layout(binding=5)		uniform sampler2DMS		positionSampler;
#else
	layout(binding=5)		uniform sampler2D	positionSampler;
#endif

// custom sampler
//layout(binding=3)		uniform sampler2DArrayShadow 		shadowsSampler;

#ifdef ENABLE_MS
layout(binding=3)		uniform sampler2DMSArray 		shadowsSampler;
#else
layout(binding=3)		uniform sampler2DArray 		shadowsSampler;
#endif

struct LayerData
{
	float		density;
	float		feather;
	float		near;
	float		far;
	
	vec4		zoneInfo;		// x - number of volume models
};

struct ZoneData
{
	float		density;
	float		feather;
	float		bias;
	float		Id;
	
	vec4		volumeMin;
	vec4		volumeMax;
	mat4		lightMatrix;
	mat4		invMatrix;
};

layout (std430, binding = 0) buffer LayerBuffer
{
	LayerData 	data;
} layerBuffer;

layout (std430, binding = 1) buffer ZoneBuffer
{
	ZoneData	zones[];
} zoneBuffer;

///////////////////////////////////////////////////////////////////////////////
//

#ifdef ENABLE_MS
const int samplerSamples = 8;
#endif

const int ditherSamples = 9;

vec2 poissonDisk[16] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 ),
  
  vec2( -0.84201624, -0.49906216 ),
  vec2( 0.74558609, -0.66890725 ),
  vec2( -0.294184101, -0.72938870 ),
  vec2( 0.24495938, 0.19387760 ),
  
  vec2( -0.54201624, -0.29906216 ),
  vec2( 0.64558609, -0.36890725 ),
  vec2( -0.394184101, -0.62938870 ),
  vec2( 0.14495938, 0.49387760 ),
  
  vec2( -0.14201624, -0.32906216 ),
  vec2( 0.14558609, -0.86890725 ),
  vec2( -0.994184101, -0.12938870 ),
  vec2( 0.49495938, 0.22387760 )
);

void ComputeVolumeShadow(inout float shadow, in float shadowSize, in int index, in vec3 position, in vec2 texCoord)
{
	vec4 p = zoneBuffer.zones[index].invMatrix * vec4(position, 1.0);
	
	vec4 volumeMax = zoneBuffer.zones[index].volumeMax;
	vec4 volumeMin = zoneBuffer.zones[index].volumeMin;
	
	float distToBorder = p.x - volumeMin.x;
	distToBorder = min(distToBorder, volumeMax.x - p.x);
	
	distToBorder = min(distToBorder, p.y - volumeMin.y);
	distToBorder = min(distToBorder, volumeMax.y - p.y);
	
	distToBorder = min(distToBorder, p.z - volumeMin.z);
	distToBorder = min(distToBorder, volumeMax.z - p.z);
	
	distToBorder = clamp(distToBorder, 0.0, 1.0);
	
	float zoneFactor = smoothstep(0.0, zoneBuffer.zones[index].feather, distToBorder);
	
	if (zoneFactor <= 0.0)
		return;
	
	vec4 ShadowCoord = zoneBuffer.zones[index].lightMatrix * vec4(position, 1.0);
	ShadowCoord /= ShadowCoord.w;
	
	ShadowCoord.xyz = ShadowCoord.xyz * vec3 (0.5) + vec3 (0.5);

	//float a = texture(shadowsSampler, vec4(ShadowCoord.x, ShadowCoord.y, 0.0, ShadowCoord.z) ); // vec3(ShadowCoord.xy, 0.0)
	//color = vec4(a, a, a, 1.0);
	
	vec3 coords = vec3(ShadowCoord.x, ShadowCoord.y, zoneBuffer.zones[index].Id);
	
	float bias = zoneBuffer.zones[index].bias;
	float visibility = 0.0;
	
	if (coords.x >= 0.0 && coords.x <= 1.0 && coords.y >= 0.0 && coords.y <=1.0)
	{
		
		vec2 vShadowSize = vec2(shadowSize, shadowSize);
		
#ifdef ENABLE_MS
		for (int i=0; i<4; ++i)
		{
			//coords.xy = ShadowSize * (ShadowCoord.xy + poissonDisk[i]/700.0);
			coords.xy = vShadowSize * ShadowCoord.xy;
			float value = texelFetch(shadowsSampler, ivec3(coords), i ).z;
			
			if (value > ShadowCoord.z-bias)
				visibility += 0.25;
		}
#else
		for (int i=0; i<4; ++i)
		{
			coords.xy = vShadowSize * (ShadowCoord.xy + poissonDisk[i]/700.0);
			//coords.xy = vShadowSize * ShadowCoord.xy;
			float value = texelFetch(shadowsSampler, ivec3(coords), 0 ).z;
			
			if (value > ShadowCoord.z-bias)
				visibility += 0.25;
		}
#endif
		
		// we are on a shadow edge, do a strong filtering !
		
		if (visibility > 0.01 || visibility < 0.99)
		{
			visibility = 0.0;
#ifdef ENABLE_MS
			const float fstep = 1.0 / samplerSamples;
#else
			const float fstep = 1.0;
#endif
			for (int nsample=0; nsample<ditherSamples; ++nsample)
			{
				coords.xy = vShadowSize * (ShadowCoord.xy + poissonDisk[nsample]/700.0);
				
				float value = texelFetch(shadowsSampler, ivec3(coords), 0 ).z; // vec3(ShadowCoord.xy, 0.0)
				if ( value <  ShadowCoord.z-bias )
				{
					visibility += fstep;
				}
				
#ifdef ENABLE_MS
				for (int i=1; i<samplerSamples; ++i)
				{
					float value = texelFetch(shadowsSampler, ivec3(coords), i ).z; // vec3(ShadowCoord.xy, 0.0)
					
					if ( value <  ShadowCoord.z-bias )
					{
						visibility += fstep;
					}
				}
#endif
			}
			
			visibility /= ditherSamples;
			//visibility = 1.0 - visibility;
		}
		
	}
	
	float density = zoneBuffer.zones[index].density;
	//visibility *= (1.0 - density * zoneFactor);
	//shadow = min(shadow, visibility);
	shadow = max(shadow, clamp(visibility * density * zoneFactor, 0.0, 1.0) );
	//shadow = shadow + clamp(visibility * density * zoneFactor, 0.0, 1.0);
	//shadow = mix(shadow, visibility, density * zoneFactor);
	//color.rgb = mix(color.rgb, vec3(visibility), density * zoneFactor);
}

float ComputeShadows(in int nsample, in vec2 texCoord, in ivec2 pixelPosition)
{
#ifdef USE_MS
	vec4 p = texelFetch(positionSampler, pixelPosition, nsample); // we store receiveShadows in .w
#else
	vec4 p = texture(positionSampler, texCoord); // we store receiveShadows in .w
#endif	
	float shadow = 0.0;
	
	if (p.w > 0.0)
	{
#ifdef ENABLE_MS
		ivec3 iShadowSize = textureSize(shadowsSampler);
#else
		ivec3 iShadowSize = textureSize(shadowsSampler, 0);
#endif
		float shadowSize = float(iShadowSize.x);
		
		int count = int(layerBuffer.data.zoneInfo.x);
		for (int i=0; i<count; ++i)
		{
			ComputeVolumeShadow(shadow, shadowSize, i, p.xyz, texCoord);
		}
	}
	
	return shadow;
}

void main () 
{
	ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
	
	vec2 texCoord = vec2(outputSize.x * (0.5 + pixelPosition.x), outputSize.y * (0.5 + pixelPosition.y));
	//vec2 texRenderCoord = vec2(renderSize.x * (0.5 + 2.0*pixelPosition.x), renderSize.y * (0.5 + 2.0*pixelPosition.y));

	float shadow = 0.0;
	
	
	/*
	vec4 p = texture(positionSampler, texCoord); // we store receiveShadows in .w
	
	float shadow = 0.0;
	
	if (p.w > 0.0)
	{
#ifdef ENABLE_MS
		ivec3 iShadowSize = textureSize(shadowsSampler);
#else
		ivec3 iShadowSize = textureSize(shadowsSampler, 0);
#endif
		float shadowSize = float(iShadowSize.x);
		
		int count = int(layerBuffer.data.zoneInfo.x);
		for (int i=0; i<count; ++i)
		{
			ComputeVolumeShadow(shadow, shadowSize, i, p.xyz, texCoord);
		}
	}
	*/
	
#ifdef USE_MS
	for (int i=0; i<SAMPLES; ++i)
	{
		shadow = shadow + ComputeShadows(i, texCoord, pixelPosition);
	}

	shadow = shadow / SAMPLES;
	
#else
	shadow = ComputeShadows(0, texCoord, pixelPosition);
	
#endif	
	shadow = 1.0 - clamp(shadow, 0.0, 1.0);
	imageStore(resultImage, pixelPosition, vec4(shadow, shadow, shadow, 1.0));
}