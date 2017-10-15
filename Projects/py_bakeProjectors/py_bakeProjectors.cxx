
/**	\file	pymoplugs.cxx
*	Library declarations.
*	Contains the basic routines to declare the DLL as a loadable
*	library.
*/

#include "py_bakeProjectors_common.h"

//--- Python module initialization functions.
BOOST_PYTHON_MODULE(pybakeprojectors) //! Must exactly match the name of the module,usually it's the same name as $(TargetName).
{
	//! Preferred practice, invokes initialization functions defined in separated CXX files.
	
	BakeProjectorsFBView_Init();
	
}
//! Set the target extension to ".pyd" on property page of project, Or
//! Use post-build event "copy $(TargetPath)  ..\..\..\..\bin\$(PlatformName)\plugins\$(TargetName).pyd"
//! to rename the module by file extension “.pyd” into python directory to make sure the module could be found.

//--- Library declaration.
FBLibraryDeclare( pybakeprojectors )
{
	
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }

/**
*	\mainpage	pyfbsdk Template
*	\section	intro Introduction
*	Template showing what needs to be done
*	in order further expose what you have developed in ORSDK to Python using Boost.Python.
*	Mainly include various situations regarding class, function and enumm.
*	For the details reference, see the tutorial document "Custom Development of ORSDK and pyfbsdk".
*/
