
/**	\file	ortool_template.cxx
*	Library declarations.
*	Contains the basic routines to declare the DLL as a loadable
*	library.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include "resourceUtils.h"
#include <GL\glew.h>

//--- Library declaration.
FBLibraryDeclare( tooldynamicmask )
{
	FBLibraryRegister( ToolDynamicMask );

	FBLibraryRegister(ObjectMask);
	FBLibraryRegisterElement(ObjectMask);
	FBLibraryRegister(ObjectChannel);
	FBLibraryRegisterElement(ObjectChannel);
	FBLibraryRegister(ObjectShape);
	FBLibraryRegisterElement(ObjectShape);
	FBLibraryRegister(ObjectKnot);
	FBLibraryRegisterElement(ObjectKnot);

	FBLibraryRegister( DynamicMask_Manager );
	//FBLibraryRegister( Manip_DynamicMask );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ 
	// init OpenGL extensions
	InitResourceUtils();
	glewInit();
	
	return true;
}
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }

/**
*	\mainpage	Tool Template
*	\section	intro	Introduction
*	Template showing what needs to be done
*	in order to create a new tool in FiLMBOX.
*/
