
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicMask.fsh
//
//	Fragment GLSL shader
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


uniform	sampler2D		rSampler;
uniform	sampler2D		gSampler;
uniform	sampler2D		bSampler;
uniform	sampler2D		aSampler;

void main (void)
{
	vec2 tx  = gl_TexCoord [0].st;
	vec4 rColor = texture2D ( rSampler, tx );
	vec4 gColor = texture2D ( gSampler, tx );
	vec4 bColor = texture2D ( bSampler, tx );
	vec4 aColor = texture2D ( aSampler, tx );
	
	gl_FragColor = vec4(rColor.r, gColor.g, bColor.b, aColor.a);
}
