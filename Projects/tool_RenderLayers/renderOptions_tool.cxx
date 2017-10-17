
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: renderOptions_tool.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "renderOptions_tool.h"

//--- Registration defines
#define RENDEROPTIONS__CLASS	RENDEROPTIONS__CLASSNAME
#define RENDEROPTIONS__LABEL	"Render Options"
#define RENDEROPTIONS__DESC	"Render Options"

//--- Implementation and registration
FBToolImplementation(	RENDEROPTIONS__CLASS	);
FBRegisterTool		(	RENDEROPTIONS__CLASS,
						RENDEROPTIONS__LABEL,
						RENDEROPTIONS__DESC,
						FB_DEFAULT_SDK_ICON	);	// Icon filename (default=Open Reality icon)



//////////////////////////////////////////////////////////////////////////////////////////////////////////////// RENDER OPTIONS

/************************************************
 *	Constructor.
 ************************************************/
bool ToolRenderOptions::FBCreate()
{
#ifndef OLD_DOCK_POSITION
	SetPossibleDockPosition(kFBToolPossibleDockPosNone);
#endif
	StartSize[0] = 340;
	StartSize[1] = 590;
	MinSize[0]=340;
	MinSize[1]=590;
	MaxSize[0]=340;
	MaxSize[1]=590;

    // Manage UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	OnIdle.Add	( this, (FBCallback) &ToolRenderOptions::EventToolIdle		);

	return true;
}

/************************************************
 *	Destruction function.
 ************************************************/
void ToolRenderOptions::FBDestroy()
{
	// Remove tool callbacks
	OnIdle.Remove	( this, (FBCallback) &ToolRenderOptions::EventToolIdle	);

	// Free user allocated memory
}

void ToolRenderOptions::EventToolIdle       ( HISender pSender, HKEvent pEvent )
{
	if (PopEvent() == EVENT_LAYER_CHANGE)
	{
		UIReset();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// UICreate

void ToolRenderOptions::UICreate()
{
	// add regions
	AddRegion( "Layer",	"Layer",
			5,		kFBAttachLeft,		"",		1.0,
			0,		kFBAttachTop,		"",		1.0,
			0,		kFBAttachRight,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	AddRegion( "TabPanel",	"TabPanel",
			0,		kFBAttachLeft,		"",		1.0,
			0,		kFBAttachBottom,	"Layer",		1.0,
			0,		kFBAttachRight,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	SetControl( "Layer", mLabelLayer );
	SetControl( "TabPanel", mTabPanel );

	// common layout
	AddRegion( "common",	"common",
			0,		kFBAttachLeft,		"",		1.0,
			2,		kFBAttachBottom,	"TabPanel",		1.0,
			0,		kFBAttachRight,		"",		1.0,
			0,		kFBAttachBottom,		"",		1.0 );
	SetControl( "common", mLayoutCommon );
	

	//
	// create additional regions
	//
	UICreateCommon();
	UICreateDisplay();
	UICreateAudio();
}

void ToolRenderOptions::UICreateCommon()
{
	mLayoutCommon.AddRegion( "override",	"override",
			5,		kFBAttachLeft,		"",		1.0,
			0,		kFBAttachTop,		"",		1.0,
			0,		kFBAttachRight,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutCommon.AddRegion( "buttonPathOverride",	"buttonPathOverride",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachBottom,	"override",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelPath",	"labelPath",
			5,		kFBAttachRight,		"buttonPathOverride",		1.0,
			5,		kFBAttachBottom,	"override",		1.0,
			30,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "editPath",	"editPath",
			5,		kFBAttachRight,		"labelPath",		1.0,
			0,		kFBAttachBottom,	"override",		1.0,
			-30,	kFBAttachRight,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "buttonPath",	"buttonPath",
			-25,	kFBAttachRight,		"",		1.0,
			5,		kFBAttachBottom,	"override",		1.0,
			20,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutCommon.AddRegion( "buttonNameOverride",	"buttonNameOverride",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachBottom,	"labelPath",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelName",	"labelName",
			5,		kFBAttachRight,		"buttonNameOverride",		1.0,
			5,		kFBAttachBottom,	"labelPath",		1.0,
			30,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "editName",	"editName",
			5,		kFBAttachRight,		"labelName",		1.0,
			5,		kFBAttachBottom,	"labelPath",		1.0,
			-5,		kFBAttachRight,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutCommon.AddRegion( "buttonTimeOverride",	"buttonTimeOverride",
			5,		kFBAttachLeft,		"",		1.0,
			25,		kFBAttachBottom,	"editName",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelStart",	"labelStart",
			5,		kFBAttachRight,		"buttonTimeOverride",		1.0,
			25,		kFBAttachBottom,	"editName",		1.0,
			30,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "editStart",	"editStart",
			5,		kFBAttachRight,		"labelStart",		1.0,
			0,		kFBAttachTop,		"labelStart",		1.0,
			70,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelStop",	"labelStop",
			5,		kFBAttachRight,		"editStart",		1.0,
			0,		kFBAttachTop,		"labelStart",		1.0,
			30,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "editStop",	"editStop",
			5,		kFBAttachRight,		"labelStop",		1.0,
			0,		kFBAttachTop,		"labelStart",		1.0,
			70,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelStep",	"labelStep",
			5,		kFBAttachRight,		"editStop",		1.0,
			0,		kFBAttachTop,	"labelStart",		1.0,
			30,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "editStep",	"editStep",
			5,		kFBAttachRight,		"labelStep",		1.0,
			0,		kFBAttachTop,		"labelStart",		1.0,
			70,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutCommon.AddRegion( "buttonPictureOverride",	"buttonPictureOverride",
			5,		kFBAttachLeft,		"",		1.0,
			25,		kFBAttachBottom,	"editStart",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelPicture",	"labelPicture",
			5,		kFBAttachRight,		"buttonPictureOverride",		1.0,
			25,		kFBAttachBottom,	"editStart",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "listPicture",	"listPicture",
			5,		kFBAttachRight,		"labelPicture",		1.0,
			0,		kFBAttachTop,		"labelPicture",		1.0,
			120,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "buttonFormatOverride",	"buttonFormatOverride",
			5,		kFBAttachLeft,		"",		1.0,
			10,		kFBAttachBottom,	"labelPicture",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelFormat",	"labelFormat",
			5,		kFBAttachRight,		"buttonFormatOverride",		1.0,
			10,		kFBAttachBottom,	"labelPicture",		1.0,
			60,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "listFormat",	"listFormat",
			5,		kFBAttachRight,		"labelFormat",		1.0,
			0,		kFBAttachTop,		"labelFormat",		1.0,
			60,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelCompression",	"labelCompression",
			15,		kFBAttachRight,		"listFormat",		1.0,
			10,		kFBAttachBottom,	"labelPicture",		1.0,
			60,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "listCompression",	"listCompression",
			5,		kFBAttachRight,		"labelCompression",		1.0,
			0,		kFBAttachTop,		"labelCompression",		1.0,
			60,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "buttonFieldOverride",	"buttonFieldOverride",
			5,		kFBAttachLeft,		"",		1.0,
			10,		kFBAttachBottom,	"labelFormat",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelField",	"labelField",
			5,		kFBAttachRight,		"buttonFieldOverride",		1.0,
			10,		kFBAttachBottom,	"labelFormat",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "listField",	"listField",
			5,		kFBAttachRight,		"labelField",		1.0,
			0,		kFBAttachTop,		"labelField",		1.0,
			120,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "buttonPixelOverride",	"buttonPixelOverride",
			5,		kFBAttachLeft,		"",		1.0,
			10,		kFBAttachBottom,	"labelField",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelPixel",	"labelPixel",
			5,		kFBAttachRight,		"buttonPixelOverride",		1.0,
			10,		kFBAttachBottom,	"labelField",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "listPixel",	"listPixel",
			5,		kFBAttachRight,		"labelPixel",		1.0,
			0,		kFBAttachTop,		"labelPixel",		1.0,
			120,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutCommon.AddRegion( "buttonCamerasOverride",	"buttonCamerasOverride",
			5,		kFBAttachLeft,		"",		1.0,
			25,		kFBAttachBottom,	"labelPixel",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelCameras",	"labelCameras",
			5,		kFBAttachRight,		"buttonCamerasOverride",		1.0,
			25,		kFBAttachBottom,	"labelPixel",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "listCameras",	"listCameras",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachBottom,	"labelCameras",		1.0,
			200,	kFBAttachNone,		"",		1.0,
			100,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "buttonCameraAdd",	"buttonCameraAdd",
			5,		kFBAttachRight,		"listCameras",		1.0,
			0,		kFBAttachTop,		"listCameras",		1.0,
			100,	kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "buttonCameraRemove",	"buttonCameraRemove",
			5,		kFBAttachRight,		"listCameras",		1.0,
			10,		kFBAttachBottom,	"buttonCameraAdd",		1.0,
			100,	kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "buttonCameraClear",	"buttonCameraClear",
			5,		kFBAttachRight,		"listCameras",		1.0,
			10,		kFBAttachBottom,	"buttonCameraRemove",		1.0,
			100,	kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutCommon.AddRegion( "buttonScriptOverride",	"buttonScriptOverride",
			5,		kFBAttachLeft,		"",		1.0,
			25,		kFBAttachBottom,	"listCameras",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelScript",	"labelScript",
			5,		kFBAttachRight,		"buttonScriptOverride",		1.0,
			25,		kFBAttachBottom,	"listCameras",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "editScript",	"editScript",
			5,		kFBAttachRight,		"labelScript",		1.0,
			0,		kFBAttachTop,	"labelScript",		1.0,
			150,	kFBAttachNone,		"",		1.0,
			25,	kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "buttonScript",	"buttonScript",
			5,		kFBAttachRight,		"editScript",		1.0,
			0,		kFBAttachTop,	"labelScript",		1.0,
			30,	kFBAttachNone,		"",		1.0,
			25,	kFBAttachNone,		"",		1.0 );

	mLayoutCommon.AddRegion( "buttonScriptOverridePost",	"buttonScriptOverridePost",
			5,		kFBAttachLeft,		"",		1.0,
			25,		kFBAttachBottom,	"buttonScriptOverride",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "labelScriptPost",	"labelScriptPost",
			5,		kFBAttachRight,		"buttonScriptOverridePost",		1.0,
			25,		kFBAttachBottom,	"buttonScriptOverride",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "editScriptPost",	"editScriptPost",
			5,		kFBAttachRight,		"labelScriptPost",		1.0,
			0,		kFBAttachTop,	"labelScriptPost",		1.0,
			150,	kFBAttachNone,		"",		1.0,
			25,	kFBAttachNone,		"",		1.0 );
	mLayoutCommon.AddRegion( "buttonScriptPost",	"buttonScriptPost",
			5,		kFBAttachRight,		"editScriptPost",		1.0,
			0,		kFBAttachTop,	"labelScriptPost",		1.0,
			30,	kFBAttachNone,		"",		1.0,
			25,	kFBAttachNone,		"",		1.0 );

	mLayoutCommon.SetControl("override", mLabelCommonOverride );
	mLayoutCommon.SetControl("buttonPathOverride", mButtonFilePathOverride );
	mLayoutCommon.SetControl("labelPath", mLabelFilePath );
	mLayoutCommon.SetControl("editPath", mEditFilePath );
	mLayoutCommon.SetControl("buttonPath", mButtonBrowse );
	mLayoutCommon.SetControl("buttonNameOverride", mButtonFileNameOverride );
	mLayoutCommon.SetControl("labelName", mLabelFileName );
	mLayoutCommon.SetControl("editName", mEditFileName );

	mLayoutCommon.SetControl("buttonTimeOverride", mButtonTimeOverride );
	mLayoutCommon.SetControl("labelStart", mLabelStart );
	mLayoutCommon.SetControl("editStart", mEditStart );
	mLayoutCommon.SetControl("labelStop", mLabelStop );
	mLayoutCommon.SetControl("editStop", mEditStop );
	mLayoutCommon.SetControl("labelStep", mLabelStep );
	mLayoutCommon.SetControl("editStep", mEditStep );

	mLayoutCommon.SetControl("buttonPictureOverride", mButtonPictureOverride );
	mLayoutCommon.SetControl("labelPicture", mLabelPicture );
	mLayoutCommon.SetControl("listPicture", mListPicture );
	mLayoutCommon.SetControl("buttonFormatOverride", mButtonFormatOverride );
	mLayoutCommon.SetControl("labelFormat", mLabelFormat );
	mLayoutCommon.SetControl("listFormat", mListFormat );
	mLayoutCommon.SetControl("labelCompression", mLabelCompression );
	mLayoutCommon.SetControl("listCompression", mListCompression );
	mLayoutCommon.SetControl("buttonFieldOverride", mButtonFieldOverride );
	mLayoutCommon.SetControl("labelField", mLabelField );
	mLayoutCommon.SetControl("listField", mListField );
	mLayoutCommon.SetControl("buttonPixelOverride", mButtonPixelOverride );
	mLayoutCommon.SetControl("labelPixel", mLabelPixel );
	mLayoutCommon.SetControl("listPixel", mListPixel );

	mLayoutCommon.SetControl("buttonCamerasOverride", mButtonCamerasOverride );
	mLayoutCommon.SetControl("labelCameras", mLabelCameras );
	mLayoutCommon.SetControl("listCameras", mListCameras );
	mLayoutCommon.SetControl("buttonCameraAdd", mButtonCameraAdd );
	mLayoutCommon.SetControl("buttonCameraRemove", mButtonCameraRemove );
	mLayoutCommon.SetControl("buttonCameraClear", mButtonCameraClear );

	mLayoutCommon.SetControl("buttonScriptOverride", mButtonScriptOverride );
	mLayoutCommon.SetControl("labelScript", mLabelScript );
	mLayoutCommon.SetControl("editScript", mEditScript );
	mLayoutCommon.SetControl("buttonScript", mButtonScript );

	mLayoutCommon.SetControl("buttonScriptOverridePost", mButtonScriptOverridePost );
	mLayoutCommon.SetControl("labelScriptPost", mLabelScriptPost );
	mLayoutCommon.SetControl("editScriptPost", mEditScriptPost );
	mLayoutCommon.SetControl("buttonScriptPost", mButtonScriptPost );
}


void ToolRenderOptions::UICreateDisplay()
{
	mLayoutDisplay.AddRegion( "override",	"override",
			5,		kFBAttachLeft,		"",		1.0,
			0,		kFBAttachTop,		"",		1.0,
			0,		kFBAttachRight,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutDisplay.AddRegion( "labelDisplay",	"labelDisplay",
			5,		kFBAttachLeft,		"",		1.0,
			15,		kFBAttachBottom,	"override",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutDisplay.AddRegion( "listDisplay",	"listDisplay",
			5,		kFBAttachRight,		"labelDisplay",		1.0,
			15,		kFBAttachBottom,	"override",		1.0,
			100,	kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutDisplay.AddRegion( "buttonShowTime",	"buttonShowTime",
			15,		kFBAttachLeft,		"",		1.0,
			10,		kFBAttachBottom,	"labelDisplay",		1.0,
			150,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutDisplay.AddRegion( "buttonShowSafe",	"buttonShowSafe",
			15,		kFBAttachLeft,		"",		1.0,
			10,		kFBAttachBottom,	"buttonShowTime",		1.0,
			150,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutDisplay.AddRegion( "buttonShowLabel",	"buttonShowLabel",
			15,		kFBAttachLeft,		"",		1.0,
			10,		kFBAttachBottom,	"buttonShowSafe",		1.0,
			150,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutDisplay.AddRegion( "buttonAntialiasing",	"buttonAntialiasing",
			15,		kFBAttachLeft,		"",		1.0,
			10,		kFBAttachBottom,	"buttonShowLabel",		1.0,
			150,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	mLayoutDisplay.AddRegion( "buttonMatte",	"buttonMatte",
			15,		kFBAttachLeft,		"",		1.0,
			10,		kFBAttachBottom,	"buttonAntialiasing",		1.0,
			150,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutDisplay.SetControl( "override", mButtonDisplayOverride );
	mLayoutDisplay.SetControl( "labelDisplay", mLabelDisplay );
	mLayoutDisplay.SetControl( "listDisplay", mListDisplay );
	mLayoutDisplay.SetControl( "buttonShowTime", mButtonShowTime );
	mLayoutDisplay.SetControl( "buttonShowSafe", mButtonShowSafe );
	mLayoutDisplay.SetControl( "buttonShowLabel", mButtonShowLabel );
	mLayoutDisplay.SetControl( "buttonAntialiasing", mButtonAntialiasing );
	mLayoutDisplay.SetControl( "buttonMatte", mButtonMatte );
}



void ToolRenderOptions::UICreateAudio()
{
	mLayoutAudio.AddRegion( "override",	"override",
			5,		kFBAttachLeft,		"",		1.0,
			0,		kFBAttachTop,		"",		1.0,
			0,		kFBAttachRight,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutAudio.AddRegion( "audio",	"audio",
			15,		kFBAttachLeft,		"",		1.0,
			15,		kFBAttachBottom,	"override",		1.0,
			0,		kFBAttachRight,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	mLayoutAudio.SetControl( "override", mButtonAudioOverride );
	mLayoutAudio.SetControl( "audio", mButtonAudio );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// UIReconnect

void ToolRenderOptions::UIReconnect(bool master)
{
	UIReconnectCommon(master);
	UIReconnectDisplay(master);
	UIReconnectAudio(master);
}

void ToolRenderOptions::UIReconnectCommon(bool master)
{
	if (master)
	{
		mLayoutCommon.ClearControl("override" );
		mLayoutCommon.ClearControl("buttonPathOverride" );
		mLayoutCommon.ClearControl("buttonNameOverride" );
		mLayoutCommon.ClearControl("buttonTimeOverride" );
		mLayoutCommon.ClearControl("buttonPictureOverride" );
		mLayoutCommon.ClearControl("buttonFormatOverride" );
		mLayoutCommon.ClearControl("buttonFieldOverride" );
		mLayoutCommon.ClearControl("buttonPixelOverride" );
		mLayoutCommon.ClearControl("buttonCamerasOverride" );
		mLayoutCommon.ClearControl("buttonScriptOverride" );
	}
	else
	{
		mLayoutCommon.SetControl("override", mLabelCommonOverride );
		mLayoutCommon.SetControl("buttonPathOverride", mButtonFilePathOverride );
		mLayoutCommon.SetControl("buttonNameOverride", mButtonFileNameOverride );
		mLayoutCommon.SetControl("buttonTimeOverride", mButtonTimeOverride );
		mLayoutCommon.SetControl("buttonPictureOverride", mButtonPictureOverride );
		mLayoutCommon.SetControl("buttonFormatOverride", mButtonFormatOverride );
		mLayoutCommon.SetControl("buttonFieldOverride", mButtonFieldOverride );
		mLayoutCommon.SetControl("buttonPixelOverride", mButtonPixelOverride );
		mLayoutCommon.SetControl("buttonCamerasOverride", mButtonCamerasOverride );
		mLayoutCommon.SetControl("buttonScriptOverride", mButtonScriptOverride );
	}
}

void ToolRenderOptions::UIReconnectDisplay(bool master)
{
	if (master)
	{
		mLayoutDisplay.ClearControl( "override" );
	}
	else
	{
		mLayoutDisplay.SetControl( "override", mButtonDisplayOverride );
	}
}

void ToolRenderOptions::UIReconnectAudio(bool master)
{
	if (master)
	{
		mLayoutAudio.ClearControl( "override" );
	}
	else
	{
		mLayoutAudio.SetControl( "override", mButtonAudioOverride );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// UIConfigure

void ToolRenderOptions::UIConfigure()
{
	mLabelLayer.Caption = "Current layer: MasterLayer";
	
	mTabPanel.Items.SetString("Common~Display~Audio");
	mTabPanel.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventTabPanelChange );
	mTabPanel.Layout = &mLayoutCommon;

	//
	// other configure
	//
	UIConfigureCommon();
	UIConfigureDisplay();
	UIConfigureAudio();
}

void ToolRenderOptions::UIConfigureCommon()
{
	mLabelCommonOverride.Caption = "Use checkboxes before each parameter to override it";
	mLabelCommonOverride.Style = kFBTextStyleItalic;

	mButtonFilePathOverride.Style = kFBCheckbox;
	mButtonFilePathOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventCommonOverride );
	mButtonFilePathOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mButtonFileNameOverride.Style = kFBCheckbox;
	mButtonFileNameOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventCommonOverride );
	mButtonFileNameOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mLabelFilePath.Caption = "Path: ";
	mEditFilePath.Text = "";
	mEditFilePath.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mButtonBrowse.Caption = "...";
	mButtonBrowse.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventButtonBrowse );
	
	mLabelFileName.Caption = "Name: ";
	mEditFileName.Text = "{default}";
	mEditFileName.Hint = "if not set, scene name will be used";
	mEditFileName.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mButtonTimeOverride.Style = kFBCheckbox;
	mButtonTimeOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventCommonOverride );
	mButtonTimeOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mLabelStart.Caption = "Start:";
	mLabelStop.Caption = "Stop:";
	mLabelStep.Caption = "Step:";

	mEditStart.Precision = 0;
	mEditStart.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mEditStop.Precision = 0;
	mEditStop.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mEditStep.Precision = 0;
	mEditStep.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mButtonPictureOverride.Style = kFBCheckbox;
	mButtonPictureOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventCommonOverride );
	mButtonPictureOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mLabelPicture.Caption = "Picture Format:";
	mListPicture.Style = kFBDropDownList;
	mListPicture.Items.SetString( "From Camera~D1 NTSC~NTSC~PAL~D1 PAL~HD 1920x1080~640x480~320x200~320x240~128x128~FullScreen" );
	mListPicture.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mButtonFormatOverride.Style = kFBCheckbox;
	mButtonFormatOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventCommonOverride );
	mButtonFormatOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mLabelFormat.Caption = "Format:";
	mListFormat.Style = kFBDropDownList;
	mListFormat.Items.SetString( "AVI~MOV~JPG~TGA~TIF~TIFF~YUV~SWF" );
	mListFormat.ItemIndex = 0;
	mListFormat.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mListFormat.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventFormatChange );

	mLabelCompression.Caption = "Compression:";
	mListCompression.Style = kFBDropDownList;
	//mListCompression.Items.SetString( "" );
	mListCompression.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	UpdateCompressionList();

	mButtonFieldOverride.Style = kFBCheckbox;
	mButtonFieldOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventCommonOverride );
	mButtonFieldOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mLabelField.Caption = "Field Mode:";
	mListField.Style = kFBDropDownList;
	mListField.Items.SetString( "No Field~Field 0~Field 1~Half Field 0~Half Field 1" );
	mListField.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mButtonPixelOverride.Style = kFBCheckbox;
	mButtonPixelOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventCommonOverride );
	mButtonPixelOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mLabelPixel.Caption = "Pixel Format:";
	mListPixel.Style = kFBDropDownList;
	mListPixel.Items.SetString( "24 bits~32 bits" );
	mListPixel.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mButtonCamerasOverride.Style = kFBCheckbox;
	mButtonCamerasOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventCommonOverride );
	mButtonCamerasOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mLabelCameras.Caption = "Cameras:";
	mListCameras.Style = kFBVerticalList;
	mListCameras.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mButtonCameraAdd.Caption = "Add";
	mButtonCameraAdd.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventButtonCameraAdd );
	mButtonCameraRemove.Caption = "Remove";
	mButtonCameraRemove.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventButtonCameraRemove );
	mButtonCameraClear.Caption = "Clear";
	mButtonCameraClear.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventButtonCameraClear );

	//
	// pre and post scripts ui

	mButtonScriptOverride.Style = kFBCheckbox;
	mButtonScriptOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventCommonOverride );
	mButtonScriptOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mLabelScript.Caption = "Pre-render script:";
	mEditScript.Text = "";
	mEditScript.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mButtonScript.Caption = "...";
	mButtonScript.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventButtonScriptBrowse );

	mButtonScriptOverridePost.Style = kFBCheckbox;
	mButtonScriptOverridePost.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventCommonOverride );
	mButtonScriptOverridePost.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mLabelScriptPost.Caption = "Post-render script:";
	mEditScriptPost.Text = "";
	mEditScriptPost.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mButtonScriptPost.Caption = "...";
	mButtonScriptPost.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventButtonScriptBrowse );
}

void ToolRenderOptions::UIConfigureDisplay()
{
	mButtonDisplayOverride.Caption = "Override";
	mButtonDisplayOverride.Style = kFBCheckbox;
	mButtonDisplayOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventDisplayOverride );
	mButtonDisplayOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mLabelDisplay.Caption = "Display mode:";
	mListDisplay.Style = kFBDropDownList;
	mListDisplay.Items.SetString("Standard~Model Only~X-Ray~Current");
	mListDisplay.ItemIndex = 1;
	mListDisplay.OnChange.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mButtonShowTime.Caption = "Show time";
	mButtonShowTime.Style = kFBCheckbox;
	mButtonShowTime.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mButtonShowSafe.Caption = "Show safe frame";
	mButtonShowSafe.Style = kFBCheckbox;
	mButtonShowSafe.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mButtonShowLabel.Caption = "Show label";
	mButtonShowLabel.Style = kFBCheckbox;
	mButtonShowLabel.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mButtonAntialiasing.Caption = "Antialiasing";
	mButtonAntialiasing.Style = kFBCheckbox;
	mButtonAntialiasing.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
	mButtonMatte.Caption = "Matte out a group";
	mButtonMatte.Style = kFBCheckbox;
	mButtonMatte.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
}

void ToolRenderOptions::UIConfigureAudio()
{
	mButtonAudioOverride.Caption = "Override";
	mButtonAudioOverride.Style = kFBCheckbox;
	mButtonAudioOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventAudioOverride );
	mButtonAudioOverride.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );

	mButtonAudio.Caption = "Use audio";
	mButtonAudio.Style = kFBCheckbox;
	mButtonAudio.OnClick.Add( this, (FBCallback) &ToolRenderOptions::EventParamChange );
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// UIReset

void ToolRenderOptions::UIReset()
{
	LayerItem *pItem = gGetCurrentLayer();
	if (!pItem) {
		FBMessageBox( "Render Options", "Failed to get current layer!", "Ok" );
		return;
	}
	
	mLabelLayer.Caption = FBString("Current layer: ") + pItem->Name;

	//
	// reset all other layouts
	//
	UIResetCommon();
	UIResetDisplay();
	UIResetAudio();

	UIReconnect(pItem->IsMaster() );

	// update override states
	UpdateAllComponents();
}

void ToolRenderOptions::UIResetCommon()
{
	LayerItem *pItem = gGetCurrentLayer();
	if (!pItem) return;

	bool IsMaster = pItem->IsMaster();

	mLabelCommonOverride.Visible = IsMaster;

	mButtonFilePathOverride.Visible = !IsMaster;
	mButtonFilePathOverride.State = (pItem->FilePathOverride) ? 1 : 0;
	mButtonFileNameOverride.Visible = !IsMaster;
	mButtonFileNameOverride.State = (pItem->FileNameOverride) ? 1 : 0;

	mEditFilePath.Text = pItem->FilePath;
	mEditFileName.Text = pItem->FileName;

	mButtonTimeOverride.Visible = !IsMaster;
	mButtonTimeOverride.State = (pItem->TimeOverride) ? 1 : 0;

	mEditStart.Value = pItem->StartTime.GetFrame();
	mEditStop.Value = pItem->StopTime.GetFrame();
	mEditStep.Value = pItem->StepTime.GetFrame();

	mButtonPictureOverride.Visible = !IsMaster;
	mButtonPictureOverride.State = (pItem->PictureOverride) ? 1 : 0;

	mListPicture.ItemIndex = (int) pItem->PictureMode;
		
	mButtonFormatOverride.Visible = !IsMaster;
	mButtonFormatOverride.State = (pItem->FormatOverride) ? 1 : 0;

	for (int i=0; i<mListFormat.Items.GetCount(); ++i)
	{
		FBString str(mListFormat.Items[i]);
		if ( strcmp(str, pItem->Format) == 0 )
		{
			mListFormat.ItemIndex = i;
			break;
		}
	}
	for (int i=0; i<mListCompression.Items.GetCount(); ++i)
	{
		FBString str( mListCompression.Items[i] );
		if ( strcmp( str, pItem->Codec ) == 0 )
		{
			mListCompression.ItemIndex = i;
			break;
		}
	}

	mButtonFieldOverride.Visible = !IsMaster;
	mButtonFieldOverride.State = (pItem->FieldOverride) ? 1 : 0;

	mListField.ItemIndex = (int) pItem->FieldMode;

	mButtonPixelOverride.Visible = !IsMaster;
	mButtonPixelOverride.State = (pItem->PixelOverride) ? 1 : 0;

	mListPixel.ItemIndex = (int) pItem->PixelMode;

	mButtonCamerasOverride.Visible = !IsMaster;
	mButtonCamerasOverride.State = (pItem->CamerasOverride) ? 1 : 0;
	
	mListCameras.Items.Clear();
	for (int i=0; i<pItem->Cameras.GetCount(); ++i)
		mListCameras.Items.Add( pItem->Cameras[i]->Name );
	
	mButtonScriptOverride.Visible = !IsMaster;
	mButtonScriptOverride.State = (pItem->ScriptOverride) ? 1 : 0;

	mEditScript.Text = pItem->Script;

	mButtonScriptOverridePost.Visible = !IsMaster;
	mButtonScriptOverridePost.State = (pItem->ScriptOverride) ? 1 : 0;

	mEditScriptPost.Text = pItem->ScriptPost;
}

void ToolRenderOptions::UIResetDisplay()
{
	LayerItem *pItem = gGetCurrentLayer();
	if (!pItem) return;

	bool IsMaster = pItem->IsMaster();

	mButtonDisplayOverride.Visible = !IsMaster;
	mButtonDisplayOverride.State = (pItem->DisplayOverride) ? 1 : 0;

	mListDisplay.ItemIndex = (int) pItem->DisplayMode;
	mButtonAntialiasing.State = (pItem->Antialiasing) ? 1 : 0;
	mButtonShowSafe.State = (pItem->ShowSafe) ? 1 : 0;
	mButtonShowLabel.State = (pItem->ShowLabel) ? 1 : 0;
	mButtonShowTime.State = (pItem->ShowTime) ? 1 : 0;
	mButtonMatte.State = (pItem->Matte) ? 1 : 0;
}

void ToolRenderOptions::UIResetAudio()
{
	LayerItem *pItem = gGetCurrentLayer();
	if (!pItem) return;

	bool IsMaster = pItem->IsMaster();

	mButtonAudioOverride.Visible = !IsMaster;
	mButtonAudioOverride.State = (pItem->AudioOverride) ? 1 : 0;

	mButtonAudio.State = (pItem->Audio) ? 1 : 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// UISave

void ToolRenderOptions::UISave()
{
	UISaveCommon();
	UISaveDisplay();
	UISaveAudio();
}

void ToolRenderOptions::UISaveCommon()
{
	LayerItem *pItem = gGetCurrentLayer();
	if (!pItem) return;

	pItem->FilePathOverride = mButtonFilePathOverride.State == 1;
	pItem->FileNameOverride = mButtonFileNameOverride.State == 1;

	pItem->FilePath = mEditFilePath.Text;
	pItem->FileName = mEditFileName.Text;

	pItem->TimeOverride = mButtonTimeOverride.State == 1;
#ifdef OLD_FBTIME_SETFRAME
	pItem->StartTime.SetTime( 0,0,0, mEditStart.Value );
	pItem->StopTime.SetTime( 0,0,0, mEditStop.Value );
	pItem->StepTime.SetTime( 0,0,0, mEditStep.Value );
#else
	pItem->StartTime.SetFrame( mEditStart.Value );
	pItem->StopTime.SetFrame( mEditStop.Value );
	pItem->StepTime.SetFrame( mEditStep.Value );
#endif
	
	pItem->PictureOverride = mButtonPictureOverride.State == 1;
	pItem->PictureMode = (FBCameraResolutionMode) (int) mListPicture.ItemIndex;
		
	pItem->FormatOverride = mButtonFormatOverride.State == 1;
	pItem->Format = mListFormat.Items[mListFormat.ItemIndex];
	pItem->Codec = mListCompression.Items[mListCompression.ItemIndex];

	pItem->FieldOverride = mButtonFieldOverride.State == 1;
	pItem->FieldMode = (FBVideoRenderFieldMode) (int) mListField.ItemIndex;

	pItem->PixelOverride = mButtonPixelOverride.State == 1;
	pItem->PixelMode = (FBVideoRenderDepth) (int) mListPixel.ItemIndex;

	pItem->CamerasOverride = mButtonCamerasOverride.State == 1;	
	pItem->Cameras.Clear();
	for (int i=0; i<mListCameras.Items.GetCount(); ++i)
	{
		FBString CameraName(mListCameras.Items[i]);
		FBScene *pScene = FBSystem().Scene;
		for (int j=0; j<pScene->Cameras.GetCount(); ++j)
			if (pScene->Cameras[j]->Name == CameraName)
				pItem->Cameras.Add( pScene->Cameras[j] );
	}

	pItem->ScriptOverride = mButtonScriptOverride.State == 1;
	pItem->Script = mEditScript.Text;
	pItem->ScriptPost = mEditScriptPost.Text;
}

void ToolRenderOptions::UISaveDisplay()
{
	LayerItem *pItem = gGetCurrentLayer();
	if (!pItem) return;

	pItem->DisplayMode = (FBVideoRenderViewingMode) (int) mListDisplay.ItemIndex;
	pItem->ShowLabel = mButtonShowLabel.State == 1;
	pItem->ShowSafe = mButtonShowSafe.State == 1;
	pItem->ShowTime = mButtonShowTime.State == 1;
	pItem->Antialiasing = mButtonAntialiasing.State == 1;
	pItem->Matte = mButtonMatte.State == 1;
}

void ToolRenderOptions::UISaveAudio()
{
	LayerItem *pItem = gGetCurrentLayer();
	if (!pItem) return;

	pItem->Audio = mButtonAudio.State == 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// UPDATE COMPONENTS

void ToolRenderOptions::UpdateAllComponents()
{
	UpdateCommonComponents();
	UpdateDisplayComponents();
	UpdateAudioComponents();
}

void ToolRenderOptions::UpdateCommonComponents()
{
	LayerItem *pItem = gGetCurrentLayer();
	if (!pItem) return;

	bool IsMaster = pItem->IsMaster();
	bool active = false;

	active = (IsMaster) ? true : mButtonFilePathOverride.State == 1;
	mLabelFilePath.Enabled = active;
	mEditFilePath.Enabled = active;
	mButtonBrowse.Enabled = active;
	active = (IsMaster) ? true : mButtonFileNameOverride.State == 1;
	mLabelFileName.Enabled = active;
	mEditFileName.Enabled = active;

	active = (IsMaster) ? true : mButtonTimeOverride.State == 1;
	mLabelStart.Enabled = active;
	mEditStart.Enabled = active;
	mLabelStop.Enabled = active;
	mEditStop.Enabled = active;
	mLabelStep.Enabled = active;
	mEditStep.Enabled = active;

	active = (IsMaster) ? true : mButtonPictureOverride.State == 1;
	mLabelPicture.Enabled = active;
	mListPicture.Enabled = active;
	active = (IsMaster) ? true : mButtonFormatOverride.State == 1;
	mLabelFormat.Enabled = active;
	mListFormat.Enabled = active;
	mLabelCompression.Enabled = active;
	mListCompression.Enabled = active;
	active = (IsMaster) ? true : mButtonFieldOverride.State == 1;
	mLabelField.Enabled = active;
	mListField.Enabled = active;
	active = (IsMaster) ? true : mButtonPixelOverride.State == 1;
	mLabelPixel.Enabled = active;
	mListPixel.Enabled = active;

	active = (IsMaster) ? true : mButtonCamerasOverride.State == 1;
	mLabelCameras.Enabled = active;
	mListCameras.Enabled = active;
	mButtonCameraAdd.Enabled = active;
	mButtonCameraRemove.Enabled = active;
	mButtonCameraClear.Enabled = active;

	active = (IsMaster) ? true : mButtonScriptOverride.State == 1;
	mLabelScript.Enabled = active;
	mEditScript.Enabled = active;
	mButtonScript.Enabled = active;
	mLabelScriptPost.Enabled = active;
	mEditScriptPost.Enabled = active;
	mButtonScriptPost.Enabled = active;
}

void ToolRenderOptions::UpdateDisplayComponents()
{
	LayerItem *pItem = gGetCurrentLayer();
	if (!pItem) return;

	bool IsMaster = pItem->IsMaster();
	bool active = (IsMaster) ? true : mButtonDisplayOverride.State == 1;

	mLabelDisplay.Enabled = active;
	mListDisplay.Enabled = active;
	mButtonShowTime.Enabled = active;
	mButtonShowSafe.Enabled = active;
	mButtonShowLabel.Enabled = active;
	mButtonAntialiasing.Enabled = active;
	mButtonMatte.Enabled = active;
}

void ToolRenderOptions::UpdateAudioComponents()
{
	LayerItem *pItem = gGetCurrentLayer();
	if (!pItem) return;

	bool IsMaster = pItem->IsMaster();
	bool active = (IsMaster) ? true : mButtonAudioOverride.State == 1;

	mButtonAudio.Enabled = active;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// UI EVENTS


void ToolRenderOptions::EventFormatChange( HISender pSender, HKEvent pEvent )
{
	UpdateCompressionList();
}

void ToolRenderOptions::UpdateCompressionList()
{
	if (mListFormat.Items.GetCount() > 0 && mListFormat.ItemIndex >= 0)
	{
		FBVideoCodecManager	lCodec;
		FBString codecStr = mListFormat.Items[mListFormat.ItemIndex];

		FBStringList codecList;
		codecList.Clear();
		lCodec.GetCodecIdList(codecStr, codecList);
		FBString listStr = codecList.AsString();

		mListCompression.Items.Clear();
		mListCompression.Items.SetString( listStr );
		mListCompression.Items.InsertAt(0, "{ Uncompressed }" );
		mListCompression.Items.InsertAt(1, "{ Asked }" );
	}
}

void ToolRenderOptions::EventTabPanelChange( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
	case 0: // common
		SetControl( "common", mLayoutCommon );
		break;
	case 1: // display
		SetControl( "common", mLayoutDisplay );
		break;
	case 2: // audio
		SetControl( "common", mLayoutAudio );
		break;
	}
}


void ToolRenderOptions::EventCommonOverride( HISender pSender, HKEvent pEvent )
{
	// switch enabled state
	UpdateCommonComponents();
}

void ToolRenderOptions::EventDisplayOverride( HISender pSender, HKEvent pEvent )
{
	// switch enabled state
	UpdateDisplayComponents();
}

void ToolRenderOptions::EventAudioOverride( HISender pSender, HKEvent pEvent )
{
	// switch enabled state
	UpdateAudioComponents();
}

void ToolRenderOptions::EventParamChange( HISender pSender, HKEvent pEvent )
{
	UISave();
}

void ToolRenderOptions::EventButtonBrowse( HISender pSender, HKEvent pEvent )
{
	// choose file path
	FBFolderPopup	dialogOpen;

	dialogOpen.Caption = "Choose path";
	dialogOpen.Path = mEditFilePath.Text;

	if (dialogOpen.Execute() )
	{
		mEditFilePath.Text = dialogOpen.Path;
		UISaveCommon();
	}
}

void ToolRenderOptions::EventButtonCameraAdd( HISender pSender, HKEvent pEvent )
{
	FBScene *pScene = FBSystem().Scene;

	// add camera to the list
	for (int i=0; i<pScene->Cameras.GetCount(); ++i)
		if (pScene->Cameras[i]->Selected)
			mListCameras.Items.Add( pScene->Cameras[i]->Name );

	UISaveCommon();
}

void ToolRenderOptions::EventButtonCameraRemove( HISender pSender, HKEvent pEvent )
{
	// remove camera from the list
	if (mListCameras.ItemIndex >= 0 && mListCameras.Items.GetCount() )
		mListCameras.Items.RemoveAt( mListCameras.ItemIndex );
	
	if (mListCameras.Items.GetCount() == 0)
		mListCameras.Items.Add( FBSystem().Renderer->CurrentCamera->Name );

	UISaveCommon();
}

void ToolRenderOptions::EventButtonCameraClear( HISender pSender, HKEvent pEvent )
{
	// clear camera list (keep only one - current)
	mListCameras.Items.Clear();
	mListCameras.Items.Add( FBSystem().Renderer->CurrentCamera->Name );

	UISaveCommon();
}

void ToolRenderOptions::EventButtonScriptBrowse( HISender pSender, HKEvent pEvent )
{
	// choose script file
	FBFilePopup		dialogOpen;

	dialogOpen.Caption = "Choose a script for a pre-render execution";
	dialogOpen.Path = mEditScript.Text;
	dialogOpen.Filter = "*.py";
	dialogOpen.Style = kFBFilePopupOpen;

	if (dialogOpen.Execute() )
	{
		mEditScript.Text = dialogOpen.FullFilename;
		UISaveCommon();
	}
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool ToolRenderOptions::FbxStore	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	pFbxObject->FieldWriteBegin( "ToolRenderAdvanceSection" );
	{
	}
	pFbxObject->FieldWriteEnd();
	*/
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ToolRenderOptions::FbxRetrieve( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	pFbxObject->FieldReadBegin( "ToolRenderAdvanceSection" );
	{
	}
	pFbxObject->FieldReadEnd();
	*/
	return true;
}