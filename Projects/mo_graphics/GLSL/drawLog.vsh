//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: drawLog.vsh
//
//	simple shader to draw helper geometry in a logarithmic depth
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

uniform float 	Fcoef;	// Fcoef = 2.0 / log2(farplane + 1.0)

varying float flogz;

void main(void)
{
	float displacement = 0.0;

	vec4 position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord [0] = gl_MultiTexCoord0;
	
	position.z = log2(max(1e-6, 1.0 + position.w)) * Fcoef - 1.0;
	flogz = 1.0 + position.w + displacement;
	
	gl_Position = position;
}
