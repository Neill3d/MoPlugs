

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: facialRetarget_MAIN.cpp
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

#include "facialRetargeting_constraint.h"

//--- Library declaration
FBLibraryDeclare( constraintcameraunproject )
{
	FBLibraryRegister(ConstraintCameraUnProject);
	FBLibraryRegister(ConstraintFacialRetargeting);
	FBLibraryRegister(ConstraintFacialRetargeting_Layout);
	FBLibraryRegister(Box_ClosestPoint);
	FBLibraryRegister(Box_UnProject);
	FBLibraryRegister(Tool_PutOnGround);
	//FBLibraryRegister(Tool_HelpMeOnFacial);
	FBLibraryRegister(FacialRetargetingAssociation);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ 
	
	ConstraintFacialRetargeting::AddPropertiesToPropertyViewManager();
	
	return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }