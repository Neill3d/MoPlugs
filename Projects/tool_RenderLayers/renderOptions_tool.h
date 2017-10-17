
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: renderOptions_tool.h
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
#include "renderCommon.h"

//--- Registration define
#define RENDEROPTIONS__CLASSNAME	ToolRenderOptions
#define RENDEROPTIONS__CLASSSTR	"ToolRenderOptions"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class for displaying popup with specified layer options
//
class ToolRenderOptions : public FBTool
{
	//--- Tool declaration.
	FBToolDeclare( ToolRenderOptions, FBTool );

public:
	//--- Construction/Destruction,
	virtual bool FBCreate();		//!< Creation function.
	virtual void FBDestroy();		//!< Destruction function.

	// FBX store/retrieve
	virtual bool FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

private:

	void UICreate();
	void UIConfigure();
	void UIReset();
	void UISave();
	void UIReconnect(bool master);

	void UICreateCommon();
	void UIConfigureCommon();
	void UIResetCommon();
	void UISaveCommon();
	void UIReconnectCommon(bool master);

	void UICreateDisplay();
	void UIConfigureDisplay();
	void UIResetDisplay();
	void UISaveDisplay();
	void UIReconnectDisplay(bool master);

	void UICreateAudio();
	void UIConfigureAudio();
	void UIResetAudio();
	void UISaveAudio();
	void UIReconnectAudio(bool master);

	void	EventTabPanelChange( HISender pSender, HKEvent pEvent );

	void	EventCommonOverride( HISender pSender, HKEvent pEvent );
	void	EventDisplayOverride( HISender pSender, HKEvent pEvent );
	void	EventAudioOverride( HISender pSender, HKEvent pEvent );
	void	EventButtonBrowse( HISender pSender, HKEvent pEvent );
	void	EventButtonScriptBrowse( HISender pSender, HKEvent pEvent );
	void	EventButtonCameraAdd( HISender pSender, HKEvent pEvent );
	void	EventButtonCameraRemove( HISender pSender, HKEvent pEvent );
	void	EventButtonCameraClear( HISender pSender, HKEvent pEvent );
	void	EventFormatChange( HISender pSender, HKEvent pEvent );
	void	EventParamChange( HISender pSender, HKEvent pEvent );

	void	EventToolIdle       ( HISender pSender, HKEvent pEvent );


private:

	LayerItem				mLayer;	// pointer to the layer data (used in reset and write back to this pointer struct)

	FBLabel					mLabelLayer;
	FBTabPanel				mTabPanel; // common, audio, display

	//
	// common options
	//
	FBLayout				mLayoutCommon;
	FBLabel					mLabelCommonOverride;	// override parameters in common or not

	FBButton				mButtonFilePathOverride;
	FBLabel					mLabelFilePath;
	FBEdit					mEditFilePath;	// not set, use default one
	FBButton				mButtonBrowse; // browse path with open dialog
	FBButton				mButtonFileNameOverride;
	FBLabel					mLabelFileName;
	FBEdit					mEditFileName;	// not set, use scene filename

	FBButton				mButtonTimeOverride;
	FBLabel					mLabelStart;
	FBEditNumber			mEditStart;
	FBLabel					mLabelStop;
	FBEditNumber			mEditStop;
	FBLabel					mLabelStep;
	FBEditNumber			mEditStep;
	FBButton				mButtonPictureOverride;
	FBLabel					mLabelPicture;
	FBList					mListPicture;
	// TODO: compression options
	FBButton				mButtonFormatOverride;
	FBLabel					mLabelFormat;
	FBList					mListFormat;
	FBLabel					mLabelCompression;
	FBList					mListCompression;

	FBButton				mButtonFieldOverride;
	FBLabel					mLabelField;
	FBList					mListField;
	FBButton				mButtonPixelOverride;
	FBLabel					mLabelPixel;
	FBList					mListPixel;

	FBButton				mButtonCamerasOverride;
	FBLabel					mLabelCameras;
	FBList					mListCameras;
	FBButton				mButtonCameraAdd;
	FBButton				mButtonCameraRemove;
	FBButton				mButtonCameraClear;

	// execute script here (pre-render script)
	FBButton				mButtonScriptOverride;
	FBLabel					mLabelScript;
	FBEdit					mEditScript; // script path
	FBButton				mButtonScript;	// browse script

	// execute script (post)
	FBButton				mButtonScriptOverridePost;
	FBLabel					mLabelScriptPost;
	FBEdit					mEditScriptPost; // script path
	FBButton				mButtonScriptPost;	// browse script

	//
	// audio options
	//
	FBLayout				mLayoutAudio;
	FBButton				mButtonAudioOverride;
	FBButton				mButtonAudio;

	//
	// display options
	//
	FBLayout				mLayoutDisplay;
	FBButton				mButtonDisplayOverride;
	FBLabel					mLabelDisplay;
	FBList					mListDisplay;
	
	FBButton				mButtonShowTime;
	FBButton				mButtonShowSafe;
	FBButton				mButtonShowLabel;
	FBButton				mButtonAntialiasing;
	FBButton				mButtonMatte;	// matte everthing out of a layer group

	void UpdateAllComponents();
	void UpdateCommonComponents();
	void UpdateDisplayComponents();
	void UpdateAudioComponents();

	void UpdateCompressionList();
};