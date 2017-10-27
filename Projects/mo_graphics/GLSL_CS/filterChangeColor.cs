
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filterChangeColor.cs
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

struct ChangeData
{
	vec4		color1;
	vec4		replace1;
	vec4		weights1;	// w as multiplier !

	vec4		flag;	// x - change color2, y - change color3

	vec4		color2;
	vec4		replace2;
	vec4		weights2;

	vec4		color3;
	vec4		replace3;
	vec4		weights3;	
};

layout (std430, binding = 0) buffer ChangeBuffer
{
	ChangeData 	data;
	
} changeBuffer;

////////////////////////////////////////////////////////////////////////

vec3 rgb2hsv(vec3 rgb)
{
	float Cmax = max(rgb.r, max(rgb.g, rgb.b));
	float Cmin = min(rgb.r, min(rgb.g, rgb.b));
    float delta = Cmax - Cmin;

	vec3 hsv = vec3(0., 0., Cmax);
	
	if (Cmax > Cmin)
	{
		hsv.y = delta / Cmax;

		if (rgb.r == Cmax)
			hsv.x = (rgb.g - rgb.b) / delta;
		else
		{
			if (rgb.g == Cmax)
				hsv.x = 2. + (rgb.b - rgb.r) / delta;
			else
				hsv.x = 4. + (rgb.r - rgb.g) / delta;
		}
		hsv.x = fract(hsv.x / 6.);
	}
	return hsv;
}

float chromaKey(vec3 color, vec3 keyColor, vec4 inWeights)
{
	//vec3 backgroundColor = vec3(0.157, 0.576, 0.129);
	vec3 weights = inWeights.xyz * vec3(0.01, 0.01, 0.01) * inWeights.w;

	vec3 hsv = rgb2hsv(color);
	vec3 target = rgb2hsv(keyColor);
	float dist = length(weights * (target - hsv));
	return 1. - clamp(3.0 * dist - 1.5, 0.0, 1.0);
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
	
	vec3 eps = vec3(0.009, 0.009, 0.009);
	vec3 color = srccolor.rgb;

	float incrustation = chromaKey(color, changeBuffer.data.color1.rgb, changeBuffer.data.weights1);
	color = mix(color, changeBuffer.data.replace1.rgb, incrustation);

	if (changeBuffer.data.flag.x > 0.0)
	{
		incrustation = chromaKey(color, changeBuffer.data.color2.rgb, changeBuffer.data.weights2);
		color = mix(color, changeBuffer.data.replace2.rgb, incrustation);
	}
	
	if (changeBuffer.data.flag.y > 0.0)
	{
		incrustation = chromaKey(color, changeBuffer.data.color3.rgb, changeBuffer.data.weights3);
		color = mix(color, changeBuffer.data.replace3.rgb, incrustation);
	}
	
	vec4 outColor = vec4(color, srccolor.a);
	outColor = ApplyMask(texCoord, srccolor, outColor);
	
	imageStore(resultImage, pixelPosition, outColor);	
}
