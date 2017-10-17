
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: wallbricks_MAIN.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include "wallbricks_constraint.h"

//--- Library declaration
FBLibraryDeclare( orconstraintwallbricks )
{
	FBLibraryRegister( ORConstraintWallBricks	);
	FBLibraryRegister( WallBricksConstraintLayout	);
	FBLibraryRegister( ORConstraintWallBricksTemp	);
	FBLibraryRegister( ORConstraintPathWrap	);
	FBLibraryRegister( ORConstraintPathTest	);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ 
	
	ORConstraintWallBricks::AddPropertiesToPropertyViewManager();
	return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }