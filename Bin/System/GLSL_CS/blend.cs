
///////////////////////////////////////////////////////////////////
//
// MoPLUGS Project (c) 2014-2016
//  Blend Compute Shader for Composite Master Toolkit
//
// Author Sergey Solokhin (Neill3d)
//		e-mail to: s@neill3d.com
//	www.neill3d.com
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////

// ?! perhaps we could specify this when parsing script header
//layout(local_size_x=32, local_size_y=32) in;

uniform vec2			outputSize; // source and destination for our operation (composition size)
uniform vec2			layerTexelSize;	// blend with a layer that could have unique size
//uniform float			opacity;

uniform vec2			translation;
uniform float			rotation;
uniform vec2			scaling;

uniform vec2			pivotOffset;

uniform float			clampToZero;
uniform int				numberOfPasses;

layout(binding=0, rgba8) uniform writeonly image2D	finalImage;

layout(binding=1) uniform sampler2D 	inputSampler1;
layout(binding=2) uniform sampler2D 	inputSampler2;

#include "misc_blending.cs"
#include "misc_masking.cs"

void main()
{

	//Use the GlobalInvocation ID as a pixel position since we dont have built in GLSL features
	ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
	//ivec2 texCoord = getUVCoordinates(pixelPosition);
	
	vec2 mult = vec2( outputSize.x / layerTexelSize.x, outputSize.y / layerTexelSize.y );
	
	vec2 texCoord = vec2(outputSize.x * (0.5 + pixelPosition.x), outputSize.y * (0.5 + pixelPosition.y));
	vec2 layerCoord = vec2(mult.x * outputSize.x * (0.5 + pixelPosition.x), mult.y * outputSize.y * (0.5 + pixelPosition.y));
	//vec2 outCoord = vec2(1.0/512.0 * (0.5 + pixelPosition.x), 1.0/512.0 * (0.5 + pixelPosition.y));
#ifdef SAME_SIZE
	vec4 color1 = texelFetch(inputSampler1, pixelPosition, 0);
#else
	vec4 color1 = texture(inputSampler1, texCoord);
#endif
	
	// layer transformation
#ifdef SAME_SIZE
	//vec4 color2 = texelFetch(inputSampler2, pixelPosition, 0);
	vec4 color2 = texture(inputSampler2, texCoord);
#else
	
	float sin_factor = sin(rotation);
	float cos_factor = cos(rotation);
	texCoord -= pivotOffset;
	texCoord = texCoord * mat2(cos_factor, sin_factor, -sin_factor, cos_factor);
	texCoord.x *= scaling.x;
	texCoord.y *= scaling.y;
	texCoord += pivotOffset;
	texCoord = texCoord + translation;
	
	vec4 color2 = texture(inputSampler2, texCoord);
#endif	
	
	if (clampToZero > 0.0)
	{
		if (texCoord.x > 1.0 || texCoord.x < 0.0 || texCoord.y > 1.0 || texCoord.y < 0.0)
			color2.a = 0.0;
	}
	
	vec4 outcolor = vec4(BlendOpacity( color1.rgb, color2.rgb, BlendOperation, color2.a ), 1.0);
	
	for (int i=0; i<numberOfPasses; ++i)
	{
		outcolor = vec4(BlendOpacity( outcolor.rgb, color2.rgb, BlendOperation, color2.a ), 1.0);
	}
	//outcolor = color2;
	
	outcolor = ApplyMask(texCoord, color1, outcolor);
	imageStore(finalImage, pixelPosition, outcolor);
}