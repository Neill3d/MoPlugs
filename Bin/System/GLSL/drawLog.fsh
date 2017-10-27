//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: drawLog.fsh
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

varying float flogz;

uniform float 	Fcoef;	// Fcoef = 2.0 / log2(farplane + 1.0)
uniform vec4 	color;

void main()                                                                         
{
	float Fcoef_half = 0.5 * Fcoef;

	gl_FragColor = color;
	gl_FragDepth = log2(flogz) * Fcoef_half;
}
