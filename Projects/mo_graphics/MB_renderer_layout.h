#ifndef __VR_RENDERER_LAYOUT_H__
#define __VR_RENDERER_LAYOUT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_renderer_layout.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#define RENDERER_COMMON_PROPS_COUNT 9

//--- Class declaration
#include "mb_renderer.h"

//! Simple device layout.
class MoRendererCallbackLayout : public FBRendererCallbackLayout
{
	//--- FiLMBOX declaration.
	FBRendererCallbackLayoutDeclare( MoRendererCallbackLayout, FBRendererCallbackLayout );

public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();			//!< FiLMBOX constructor.
	virtual void FBDestroy();			//!< FiLMBOX destructor.

private:
	// UI Management
	void	UICreate	();
	void	UICreateOptions();
	void	UICreateTools();
	void	UIConfigure	();
	void	UIConfigureOptions();
	void	UIConfigureTools();
	void	UIReset		();
	void	UIResetOptions();
	void	UIResetTools();

	// UI Callbacks
	void	OnResizeCallback(HISender pSender, HKEvent pEvent);
	void	OnButtonUpdateClick(HISender pSender, HKEvent pEvent);

	void	OnButtonCompositionClick(HISender pSender, HKEvent pEvent);
	void	OnButtonDynamicMasksClick(HISender pSender, HKEvent pEvent);
	void	OnButtonTexturesClick(HISender pSender, HKEvent pEvent);
	void	OnButtonCleanUpClick(HISender pSender, HKEvent pEvent);
	void	OnButtonExtendedClick(HISender pSender, HKEvent pEvent);
	void	OnButtonCmdLineClick(HISender pSender, HKEvent pEvent);
	void	OnButtonLightsClick(HISender pSender, HKEvent pEvent);
	void	OnButtonAboutClick(HISender pSender, HKEvent pEvent);

private:

	FBApplication		mApplication;

	int					mLayoutWidth;
	int					mLayoutHeight;

	// common options
	FBLayout			mLayoutOptions;
	FBLayoutRegion		mLayoutRegionOptions;

	FBEditProperty		mCommonProperties[RENDERER_COMMON_PROPS_COUNT];

	/*
	FBEditProperty		mEditRealTimeUpdate;
	FBButton			mButtonUpdate;

	FBEditProperty		mEditUseGlobalResolution;
	FBEditProperty		mEditGlobalResolution;

	FBEditProperty		mEditQualityPreset;
	FBEditProperty		mEditQualityDepth;

	FBEditProperty		mEditRenderOverride;
	FBEditProperty		mEditRenderQualityPreset;
	FBEditProperty		mEditRenderQualityDepth;
	*/
	// use render properties for details

	// TODO: button to launch main tools (composition tree, dynamic masks, texture manager, etc.)
	FBLayout		mLayoutTools;
	FBLayoutRegion	mLayoutRegionTools;

	FBButton		mButtonLaunchComposition;
	FBButton		mButtonLaunchDynamicMasks;
	FBButton		mButtonLaunchTextureManager;
	FBButton		mButtonLaunchCleanupManager;
	FBButton		mButtonLaunchExtendedRendering;
	FBButton		mButtonLaunchCmdLineTool;
	FBButton		mButtonLaunchLightsTool;

	FBButton		mButtonAbout;

	//
	//
	void UICreateProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props);
	void UIConfigureProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props);
	void UIResizeProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props);
};

#endif /* __MO_RENDERER_LAYOUT_H__ */
