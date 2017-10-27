
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: copy.cs
//
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

layout(binding=0, rgba8) uniform writeonly image2D resultImage;
layout(binding=1) uniform sampler2D inputImage;

uniform vec2 offset = vec2(0.0, 0.0);

void main()
{
        ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
		ivec2 inPixelPosition = pixelPosition + ivec2(offset);
		
		vec4 outcolor = texelFetch(inputImage, inPixelPosition, 0 );
        imageStore(resultImage, pixelPosition, outcolor);
}