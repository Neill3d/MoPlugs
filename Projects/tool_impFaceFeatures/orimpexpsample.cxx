
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orimpexpsample.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////



//--- SDK Include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

//--- Library Declaration
FBLibraryDeclare( orimpexpsample )
{
	FBLibraryRegister( ORToolSample );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()	{ return true; }
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease(){ return true; }

/**
*	\mainpage	Sampled Data Exporter.
*	\section	intro	Introduction
*	Shows how to export sampled data from a tool (not
*	needing more than an Animation license of FiLMBOX.
*/
