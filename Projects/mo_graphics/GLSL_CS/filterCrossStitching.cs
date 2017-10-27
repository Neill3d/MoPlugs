
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filterCrossStitching.cs
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

#include "misc_masking.cs"

uniform float time = 0.0;
//uniform float rt_w = 1920;
//uniform float rt_h = 1080;
uniform float stitching_size = 4.0;
uniform int invert = 1;

vec4 PostFX(sampler2D tex, vec2 uv, float time)
{
	float rt_w = 1.0 / outputSize.x;
	float rt_h = 1.0 / outputSize.y;
	
	vec4 c = vec4(0.0);
	float size = stitching_size;
	vec2 cPos = uv * vec2(rt_w, rt_h);
	vec2 tlPos = floor(cPos / vec2(size, size));
	tlPos *= size;
	
	int remX = int(mod(cPos.x, size));
	int remY = int(mod(cPos.y, size));
	
	if (remX == 0 && remY == 0)
		tlPos = cPos;
	
	vec2 blPos = tlPos;
	blPos.y += (size - 1.0);
	
	if ((remX == remY) || (((int(cPos.x) - int(blPos.x)) == (int(blPos.y) - int(cPos.y)))))
	{
		if (invert == 1)
		  c = vec4(0.2, 0.15, 0.05, 1.0);
		else
		  c = texture(tex, tlPos * vec2(1.0/rt_w, 1.0/rt_h)) * 1.4;
	}
	else
	{
		if (invert == 1)
		  c = texture(tex, tlPos * vec2(1.0/rt_w, 1.0/rt_h)) * 1.4;
		else
		  c = vec4(0.0, 0.0, 0.0, 1.0);
	}
	return c;
}

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
	
	vec4 dstcolor = PostFX(colorSampler, texCoord, time);
	vec4 outColor = ApplyMask(texCoord, srccolor, dstcolor);
	imageStore(resultImage, pixelPosition, outColor);	
}
