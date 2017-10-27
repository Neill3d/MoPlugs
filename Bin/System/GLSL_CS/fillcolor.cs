
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: fillcolor.cs
//
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

uniform vec2	outputSize;	// stored in a format ( 1.0/w, 1.0/h )

uniform vec4 	backgroundColor;
#ifdef useGradient
uniform vec4	backgroundUpperColor;
uniform vec4	backgroundLowerColor;
#endif

// pre-defined in a header
//layout (local_size_x =32, local_size_y = 32) in;

layout(binding=0, rgba8) uniform writeonly image2D resultImage;
#ifdef USE_TEXTURE
layout(binding=1) uniform sampler2D inputImage;
#endif


void main()
{
        ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);

		vec4 outcolor = backgroundColor;
		
		#ifdef useGradient
		float f = outputSize.y*pixelPosition.y;
		outcolor = mix(backgroundLowerColor, backgroundUpperColor, f );
		#endif
		
		#ifdef USE_TEXTURE		
			#ifdef SAME_SIZE
				outcolor = texelFetch(inputImage, pixelPosition, 0);
			#else
				// do some scaling with bilinear filtering
				outcolor = texture(inputImage, vec2(outputSize.x*(0.5+pixelPosition.x), outputSize.y*(0.5+pixelPosition.y)) );
			#endif
		#endif
        
        imageStore(resultImage, pixelPosition, outcolor);
}