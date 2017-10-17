
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: camera_linkvis.cpp
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

//--- Library declaration
FBLibraryDeclare( camera_linkvis )
{
    FBLibraryRegister( Manager_CameraLinkVis );
}
FBLibraryDeclareEnd;

/************************************************
 *  Library functions.
 ************************************************/
bool FBLibrary::LibInit()       { return true; }
bool FBLibrary::LibOpen()       { return true; }
bool FBLibrary::LibReady()      { return true; }
bool FBLibrary::LibClose()      { return true; }
bool FBLibrary::LibRelease()    { return true; }

/**
*   \mainpage   Custom Manager Template
*   \section    intro   Introduction
*   Template showing what needs to be done
*   in order to create a custom manager that is started at the beginning of
*   the lifetime of the application and and last until the application is
*   closed.
*/
