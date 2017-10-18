
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: skybox_MAIN.cxx
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

#include <GL\glew.h>

//--- Library declaration
FBLibraryDeclare( orshader_skybox )
{
	FBLibraryRegister( ORShaderSkyBox );
	FBLibraryRegister( ORShaderSkyDome );
	//FBLibraryRegister( ORShaderPanorama );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	{ 
	
	glewInit();
	
	return true; }
bool FBLibrary::LibReady()	{ return true; }
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease(){ return true; }
