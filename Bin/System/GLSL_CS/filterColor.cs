
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filterColor.cs
//
//	color correction
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

uniform vec2	outputSize;	// stored in a format ( 1.0/w, 1.0/h )

layout(binding=0, rgba8) uniform writeonly image2D resultImage;
layout(binding=1) 		uniform sampler2D 	colorSampler;

// contrast, saturation, brightness, gamma
uniform vec4			gCSB;	

// hue, saturation, lightness, inverse state
uniform vec4			gHue;

#include "misc_blending.cs"
#include "misc_masking.cs"

void main (void)
{
	ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
	vec2 texCoord = vec2(outputSize.x * (0.5 + pixelPosition.x), outputSize.y * (0.5 + pixelPosition.y));
	
#ifdef SAME_SIZE
	vec4 srccolor = texelFetch(colorSampler, pixelPosition, 0);
#else
	// do some scaling with bilinear filtering
	vec4 srccolor = texture(colorSampler, texCoord );
#endif
	
	float gamma = gCSB.w;
	vec3 color = ContrastSaturationBrightness( srccolor.xyz, gCSB.z, gCSB.y, gCSB.x );
	color.x = pow(color.x, 1.0/gamma);
	color.y = pow(color.y, 1.0/gamma);
	color.z = pow(color.z, 1.0/gamma);
	
	vec3 blendHSL = RGBToHSL(color);
	blendHSL.x += gHue.x;
	blendHSL.y += gHue.y;
	blendHSL.z += gHue.z;
	color = HSLToRGB(blendHSL);
	
	vec3 inverseColor = vec3(1.0) - color;
	float inverse = gHue.w;
	
	vec4 outColor = vec4( mix(color, inverseColor, inverse), 1.0);
	outColor = ApplyMask(texCoord, srccolor, outColor);
	
	imageStore(resultImage, pixelPosition, outColor);	
}
