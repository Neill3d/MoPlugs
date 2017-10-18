
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: mographics_common.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "mographics_common.h"

//

bool moplugs_renderMode = false;
CCameraInfoCache gCameraCache;


///////////////////////////////////////////////////////////

// private functions

void SetMoPlugsRender(const bool mode)
{
	moplugs_renderMode = mode;
}

void SetCameraCache(const CCameraInfoCache &cache)
{
	gCameraCache = cache;
}

void UpdateCameraCache(const mat4 &mv, const mat4 &mp, const vec3 &pos)
{
	gCameraCache.mv4 = mv;
	gCameraCache.p4 = mp;
	gCameraCache.pos = pos;
}

// public functions

bool GRAPHICS_COMMON IsMoPlugsRender()
{
	return moplugs_renderMode;
}

void GRAPHICS_COMMON GetCameraCache(CCameraInfoCache &cache)
{
	cache = gCameraCache;
}