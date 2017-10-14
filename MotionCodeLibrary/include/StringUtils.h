
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: StringUtils.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

// return filename from a full path
FBString ExtractFileName(FBString &str);

FBString ExtractFilePath(FBString &str);

bool SplitPath(FBString &path, FBStringList &list);

// change name in full path str to the new_name (inc extension)
void ChangeFileName(FBString &str, FBString &new_name);

// change filename extension to the specified new_ext
void ChangeFileExt(FBString &str, FBString &new_ext);

void FileNameAddSuffix(FBString &str, const char *suffix);