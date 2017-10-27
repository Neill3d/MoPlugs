
///////////////////////////////////////////////////////////////////
//
// MoPLUGS Project (c) 2014-2016
//  Blur Compute Shader for Composite Master Toolkit
//
// Author Sergey Solokhin (Neill3d)
//		e-mail to: s@neill3d.com
//	www.neill3d.com
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////

#version 430

layout(local_size_x=1, local_size_y=1) in;
layout(binding=0, rgba8) uniform readonly image2D 	inputSampler;
layout(binding=1, rgba8) uniform writeonly image2D	finalImage;


void main()
{

	//Use the GlobalInvocation ID as a pixel position since we dont have built in GLSL features
	ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
	//ivec2 texCoord = getUVCoordinates(pixelPosition);
	vec2 texCoord = vec2(pixelPosition.x / SCREEN_WIDTH, pixelPosition.y / SCREEN_HEIGHT);


	vec4 color = texture(inputSampler, texCoord);

	imageStore(finalImage, pixelPosition, color);
}