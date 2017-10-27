
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filterPosterization.cs
//
//	composition toolkit filter
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

uniform float		numColors;
uniform float		gamma;

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
	
	vec3 c = srccolor.rgb;
	c = pow(c, vec3(gamma));
	c = c * numColors;
	c = floor(c);
	c = c / numColors;
	c = pow(c, vec3(1.0/gamma));
	
	vec4 outColor = ApplyMask(texCoord, srccolor, vec4(c, srccolor.a));
	imageStore(resultImage, pixelPosition, outColor);	
}
