
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filterToonLines.cs
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

uniform vec4		outlineColor = vec4(1.0, 0.0, 0.0, 1.0);
uniform float		falloff;
uniform float		edgeScale = 1.0;

uniform vec2		textureCoordOffset[9];

layout(binding=0, rgba8) uniform writeonly image2D resultImage;
layout(binding=1) 		uniform sampler2D 	colorSampler;
#ifdef USE_MS
	layout(binding=2) 		uniform sampler2DMS 	depthSampler;
	layout(binding=3) 		uniform sampler2DMS 	normalSampler;
#else
	layout(binding=2) 		uniform sampler2D 		depthSampler;
	layout(binding=3) 		uniform sampler2D 		normalSampler;
#endif

#include "misc_depth.cs"
#include "misc_masking.cs"


void decodeNormal(in ivec2 uv, in int nsample, out vec3 normal)
{
	normal = normalize(texelFetch(normalSampler, uv, nsample).xyz);
}


float EdgeDetection(in vec2 texCoord, in int nsample, inout float depth)
{
	// Current fragment depth
	float base = 0.0;
	float colDifForce = 0.0;
	
	float Depth[9];
	vec3 Normal[9];
	float localDepth=0.0;
	
	for (int i=0; i<9; ++i)
	{
		vec2 uv = renderSize * (texCoord + previewScaleFactor * edgeScale * textureCoordOffset[i]);
		ivec2 iuv = ivec2( uv );
		
		float d = texelFetch(depthSampler, iuv, nsample).x;
		
		ComputeDepth( d, Depth[i] );
		localDepth = localDepth + Depth[i];
		
		decodeNormal( iuv, nsample, Normal[i]);
	}
	depth += localDepth / 9.0;
	
	// The result fragment sample matrix is as below, where x is the current fragment(4)
	// 0 1 2
	// 3 x 5
	// 6 7 8

	//Compute Deltas in Depth.  
	vec4 Deltas1;  
	vec4 Deltas2;  
	Deltas1.x = Depth[1];  
	Deltas1.y = Depth[2];  
	Deltas1.z = Depth[3];  
	Deltas1.w = Depth[4];  
	Deltas2.x = Depth[5];  
	Deltas2.y = Depth[6];  
	Deltas2.z = Depth[7];  
	Deltas2.w = Depth[8];  
	//Compute absolute gradients from center.  
	Deltas1 = abs(Deltas1 - Depth[0]);  
	Deltas2 = abs(Depth[0] - Deltas2);  
	//Find min and max gradient, ensuring min != 0  
	vec4 maxDeltas = max(Deltas1, Deltas2);  
	vec4 minDeltas = max(min(Deltas1, Deltas2), 0.00001);  
	// Compare change in gradients, flagging ones that change  
	// significantly.  
	// How severe the change must be to get flagged is a function of the  
	// minimum gradient. It is not resolution dependent. The constant  
	// number here would change based on how the depth values are stored  
	// and how sensitive the edge detection should be.  
	vec4 depthResults = step(minDeltas * 25.0, maxDeltas);  
	//Compute change in the cosine of the angle between normals.  
	Deltas1.x = dot(Normal[1], Normal[0]);  
	Deltas1.y = dot(Normal[2], Normal[0]);  
	Deltas1.z = dot(Normal[3], Normal[0]);  
	Deltas1.w = dot(Normal[4], Normal[0]);  
	Deltas2.x = dot(Normal[5], Normal[0]);  
	Deltas2.y = dot(Normal[6], Normal[0]);  
	Deltas2.z = dot(Normal[7], Normal[0]);  
	Deltas2.w = dot(Normal[8], Normal[0]);  
	Deltas1 = abs(Deltas1 - Deltas2);  
	
	// Compare change in the cosine of the angles, flagging changes  
	// above some constant threshold. The cosine of the angle is not a  
	// linear function of the angle, so to have the flagging be  
	// independent of the angles involved, an arccos function would be  
	// required.  
	vec4 normalResults = step(0.4, Deltas1);  
	normalResults = max(normalResults, depthResults);  
	float edge = (normalResults.x + normalResults.y +  
          normalResults.z + normalResults.w) * 0.25; 
		  
	return edge;
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

	float depth = 0.0;

#ifdef USE_MS
	float result = 0.0;
	for (int nSample=0; nSample<SAMPLES; ++nSample)
	{
		result += EdgeDetection(texCoord, nSample, depth);
	}

	result /= float(SAMPLES);
	depth /= float(SAMPLES);
#else
	float result = EdgeDetection(texCoord, 0, depth);
#endif
	
	if (falloff > 0.0)
	{
		float lfalloff = falloff / cameraFarPlane;
		
		depth = clamp(depth / lfalloff, 0.0, 1.0);
		result *= smoothstep(0.0, 1.0, 1.0-depth);
	}
	
	vec4 outColor = mix(srccolor, outlineColor, result ); 
	
	outColor = ApplyMask(texCoord, srccolor, outColor);
	imageStore(resultImage, pixelPosition, outColor);
}
