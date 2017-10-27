
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: misc_masking.cs
//
//	composition toolkit
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

uniform	float	weight;

#ifdef USE_MASK
	layout(binding=7) 	uniform 	sampler2D		maskSampler;
						uniform 	float			maskInverse;
						
	bool CheckForTexelReject(vec2 incoords)
	{
		float maskValue = abs(maskInverse - texture2D( maskSampler, incoords ).r);
		return (maskValue == 0.0);
	}
	vec4 ApplyMask(vec2 incoords, vec4 srccolor, vec4 dstcolor)
	{
		float maskValue = abs(maskInverse - texture2D( maskSampler, incoords ).r);
		return mix(srccolor, dstcolor, weight*maskValue);
	}
#else
	bool CheckForTexelReject(vec2 incoords)
	{
		return false;
	}
	vec4 ApplyMask(vec2 incoords, vec4 srccolor, vec4 dstcolor)
	{
		return mix(srccolor, dstcolor, weight);
	}
#endif