
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Main.cxx
//
//	Author Sergey Solokhin (Neill3d)
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

#include "GL\glew.h"

#include "moPhysics_CarProperties.h"
#include "moPhysics_ChainProperties.h"
#include "moPhysics_PlayerProperties.h"

//--- Library declaration
FBLibraryDeclare( orconstraint_carphysics )
{
	FBLibraryRegister( ORCustomManager_Physics );

	FBLibraryRegister( MOPhysicsSolver );
	FBLibraryRegister( MOPhysicsSolverLayout );

	FBLibraryRegister( MOCarPhysProperties );
	FBLibraryRegister( MOChainPhysProperties );
	FBLibraryRegister( MOPlayerPhysProperties );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ 

	glewInit();

	MOCarPhysProperties::AddPropertiesToPropertyViewManager();
	MOChainPhysProperties::AddPropertiesToPropertyViewManager();
	MOPlayerPhysProperties::AddPropertiesToPropertyViewManager();

	return true; }
bool FBLibrary::LibClose()		{ 
	
	return true; }
bool FBLibrary::LibRelease()	{ return true; }

/**
*	\mainpage	Constraint Template
*	\section	intro	Introduction
*	Template showing what needs to be done
*	in order to create a new constraint.
*/
