
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: pyediting.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "pyediting_pyfunction.h"

//--- Python module initialization functions.
BOOST_PYTHON_MODULE(pyediting) //! Must exactly match the name of the module,usually it's the same name as $(TargetName).
{
	//! Preferred practice, invokes initialization functions defined in separated CXX files.
	
	ORFunctionInit();
}
//! Set the target extension to ".pyd" on property page of project, Or
//! Use post-build event "copy $(TargetPath)  ..\..\..\..\bin\$(PlatformName)\plugins\$(TargetName).pyd"
//! to rename the module by file extension “.pyd” into python directory to make sure the module could be found.

//--- Library declaration.
FBLibraryDeclare( pyediting )
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
