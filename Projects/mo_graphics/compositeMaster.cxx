
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

#include <GL\glew.h>

//--- Library declaration.
FBLibraryDeclare( toolcompositemaster )
{
	FBLibraryRegister( CompositeMaster_Manager );

	FBLibraryRegister( ToolCompositeMaster );

	FBLibraryRegister(ObjectCompositeFinal);
	FBLibraryRegisterElement(ObjectCompositeFinal);

	FBLibraryRegister(ObjectCompositeRender);
	FBLibraryRegisterElement(ObjectCompositeRender);

	FBLibraryRegister(ObjectCompositeSolidColor);
	FBLibraryRegisterElement(ObjectCompositeSolidColor);

	FBLibraryRegister(ObjectCompositeBlend);
	FBLibraryRegisterElement(ObjectCompositeBlend);

	FBLibraryRegister(ObjectEffectBlur);
	FBLibraryRegisterElement(ObjectEffectBlur);

	FBLibraryRegister(ObjectEffectCSB);
	FBLibraryRegisterElement(ObjectEffectCSB);

	FBLibraryRegister(ObjectEffectHalfTone);
	FBLibraryRegisterElement(ObjectEffectHalfTone);

	FBLibraryRegister(ObjectEffect3dDOF);
	FBLibraryRegisterElement(ObjectEffect3dDOF);

	FBLibraryRegister(ObjectEffect3dFog);
	FBLibraryRegisterElement(ObjectEffect3dFog);

	FBLibraryRegister(ObjectSourceMask);
	FBLibraryRegisterElement(ObjectSourceMask);

	FBLibraryRegister(EffectToonLines);
	FBLibraryRegisterElement(EffectToonLines);

	FBLibraryRegister(EffectSSAO);
	FBLibraryRegisterElement(EffectSSAO);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ 
	// init OpenGL extensions
	//extgl_Initialize();

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
