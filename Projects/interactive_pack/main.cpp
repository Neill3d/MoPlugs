
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: main.cpp
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
FBLibraryDeclare( interactive_pack )
{
	// manager
	FBLibraryRegister( ORCustomManager_Interactive );

	// commands
	//FBLibraryRegister( ORCommand_Base );
	FBLibraryRegister( ORCommand_Group );
	FBLibraryRegister( ORCommand_Rewind );
	FBLibraryRegister( ORCommand_Color );
	FBLibraryRegister( ORCommand_CameraShake );
	FBLibraryRegister( ORCommand_CameraZoom );

	// boxes
	FBLibraryRegister( BoxSystem );
	FBLibraryRegister( BoxTakeList );
	FBLibraryRegister( BoxCameraList );
	FBLibraryRegister( BoxShortcut2 );
	FBLibraryRegister( BoxShortcut3 );
	FBLibraryRegister( BoxShortcutDouble );
	FBLibraryRegister( ORBoxTransportAdvance );
	FBLibraryRegister( ORBoxTransportCommands );

	FBLibraryRegister( BoxCameraShake );
	FBLibraryRegister( BoxCameraZoom );

	FBLibraryRegister( ORBoxColorWhite );
	FBLibraryRegister( ORBoxColorBlack );
	FBLibraryRegister( ORBoxColorRed );
	FBLibraryRegister( ORBoxColorGreen );
	FBLibraryRegister( ORBoxColorBlue );
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
*	\mainpage	interactive pack
*	\section	intro	Introduction
*	Special commands, boxes and manager
*	to make an interactive music play.
*/
