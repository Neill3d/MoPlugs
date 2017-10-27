
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: drawCubeMap.vsh
//
//	simple shader to draw helper geometry in a logarithmic depth
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

varying vec3 pos;

void main(void)
{

	vec4 position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord [0] = gl_MultiTexCoord0;
	
	pos = normalize(gl_Vertex.xyz);
	gl_Position = position;
}
