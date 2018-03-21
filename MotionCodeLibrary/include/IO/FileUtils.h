
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: FileUtils.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
//#include <fbsdk.h>

/////////////////////////////////////////////////////////////

bool IsFileExists ( const char *filename );

//
// search first of all in mobu config folder, then in all plugins folders
//
bool FindEffectLocation(const char *effect, FBString &out_path, FBString &out_fullname);