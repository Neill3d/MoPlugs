
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filterCrosshatch.cs
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
	
	float lum = length(srccolor.rgb);
    
    vec4 dstcolor = vec4(1.0, 1.0, 1.0, 1.0);
    
	float f = 10.0;
	
    if (lum < 1.00) {
        if (mod(float(pixelPosition.x + pixelPosition.y), f) == 0.0) {
            dstcolor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
     
    if (lum < 0.75) {
        if (mod(float(pixelPosition.x - pixelPosition.y), f) == 0.0) {
            dstcolor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
     
    if (lum < 0.50) {
        if (mod(float(pixelPosition.x + pixelPosition.y) - 5.0, f) == 0.0) {
            dstcolor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
     
    if (lum < 0.3) {
        if (mod(float(pixelPosition.x - pixelPosition.y) - 5.0, f) == 0.0) {
            dstcolor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
	dstcolor.a = srccolor.a;
	
	vec4 outColor = ApplyMask(texCoord, srccolor, dstcolor);
	imageStore(resultImage, pixelPosition, outColor);	
}
