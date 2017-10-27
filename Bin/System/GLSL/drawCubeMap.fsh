
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: drawCubeMap.fsh
//
//	simple shader to draw helper geometry in a logarithmic depth
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#version 120     

uniform samplerCube		cubeMap;
varying vec3 pos;

void main()                                                                         
{
	vec4 color = textureCube(cubeMap, pos);
	
	gl_FragColor = color;
}
