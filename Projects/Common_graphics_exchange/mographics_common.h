
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: mographics_common.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef MOBU_DLL
	#define GRAPHICS_COMMON __declspec( dllexport )
#else 
	#define GRAPHICS_COMMON __declspec(dllimport)
#endif

#include "shared_camera.h"

// private functions

void SetMoPlugsRender(const bool mode);
void SetCameraCache(const CCameraInfoCache &cache);
void UpdateCameraCache(const mat4 &mv, const mat4 &mp, const vec3 &pos);

// public functions

bool GRAPHICS_COMMON IsMoPlugsRender();
void GRAPHICS_COMMON GetCameraCache(CCameraInfoCache &cache);