
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: FileUtils.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "IO/FileUtils.h"

/////////////////////////////////////////////////////////////

bool IsFileExists ( const TCHAR *filename ) {
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile( filename, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		FindClose(hFind);
		return true;
	}
}


////////////////////////////////////////////////////////////

bool FindEffectLocation(const char *effect, FBString &out_path, FBString &out_fullname)
{
	FBSystem	&lSystem = FBSystem::TheOne();
	
	out_path = lSystem.UserConfigPath;
	out_fullname = out_path + "\\";
	out_fullname = out_fullname + effect;

	bool found = false;

	if ( IsFileExists(out_fullname) == true )
	{
		found = true;
	}
	else
	{
		
		FBStringList paths;
#ifndef ORSDK2013
		paths = lSystem.GetPluginPath();
#endif
		for (int i=0; i<paths.GetCount(); ++i)
		{
			out_path = paths[i];
			out_fullname = out_path + "\\";
			out_fullname = out_fullname + effect;

			if (IsFileExists(out_fullname) == true)
			{
				found = true;
				break;
			}
		}
	}

	return found;
}