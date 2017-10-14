
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ResourceUtils.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// call this on dll init
void InitResourceUtils();

const char *LoadStringFromResource1(int id);
const char *LoadStringFromResource2(int id);

const char *LoadVersionFromResource();