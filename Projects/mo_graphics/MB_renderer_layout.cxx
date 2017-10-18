
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_renderer_layout.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "mb_renderer_layout.h"

//--- Registration define
#define MORendererCallback__LAYOUT		MoRendererCallbackLayout

//--- FiLMBOX implementation and registration
FBRendererCallbackLayoutImplementation(	MORendererCallback__LAYOUT		);
FBRegisterRendererCallbackLayout	  (	MORendererCallback__LAYOUT,
                                       MORendererCallback__CLASSSTR,
                                       FB_DEFAULT_SDK_ICON			);

////////////////

const char *gCommonPropNames[RENDERER_COMMON_PROPS_COUNT] = {
		"Update Type",
		"Update",
		"Use Global Resolution",
		"Render Resolution",
		"Quality Presets",
		"Display Depth",
		"Render Quality Override",
		"Render Quality Presets",
		"Render Display Depth" };

/************************************************
*	FiLMBOX constructor.
************************************************/
bool MoRendererCallbackLayout::FBCreate()
{
    UICreate	();
    UIConfigure	();
    UIReset		();

	mLayoutWidth = 400;
	mLayoutHeight = 400;

	OnResize.Add(this, (FBCallback) &MoRendererCallbackLayout::OnResizeCallback );

    return true;
}


/************************************************
*	FiLMBOX destructor.
************************************************/
void MoRendererCallbackLayout::FBDestroy()
{
}


/************************************************
*	Create the UI.
************************************************/
void MoRendererCallbackLayout::UICreate()
{
	const int lS=4;
	const int lW=300;
	const int lP=300;
	
	AddRegion( "slider", "slider",
			lW,		kFBAttachNone,		"",			1.0,
			lS,		kFBAttachTop,		"",			1.0,
			lS,		kFBAttachNone,		"",			1.0,
			-lS,	kFBAttachBottom,	"",			1.0 );
		
	AddRegion( "options", "options",
			lS, kFBAttachLeft,	"",			1.0,
			lS,	kFBAttachTop,	"",			1.0,
			-lS, kFBAttachRight, "slider",	1.0,
			-lS, kFBAttachBottom,	"",			1.0 );

	AddRegion( "tools", "tools",
			lS, kFBAttachRight,	"slider",			1.0,
			lS,	kFBAttachTop,	"",			1.0,
			-lS, kFBAttachRight,	"",	1.0,
			-lS, kFBAttachBottom,	"",			1.0 );

	//SetControl( "slider", mSlider );
	SetControl( "options", mLayoutOptions );
	SetControl( "tools", mLayoutTools );

	UICreateOptions();
	UICreateTools();
}

void MoRendererCallbackLayout::UICreateOptions()
{
	

	UICreateProps(mLayoutOptions, 9, gCommonPropNames, mCommonProperties);
	/*
	int lS=4;
    int lH=25;
    int lW=300;

	const int count = 9;

	

	FBVisualComponent *components[count] = {
		&mEditRealTimeUpdate,
		&mButtonUpdate,
		&mEditUseGlobalResolution,
		&mEditGlobalResolution,
		&mEditQualityPreset,
		&mEditQualityDepth,
		&mEditRenderOverride,
		&mEditRenderQualityPreset,
		&mEditRenderQualityDepth };

	FBVisualComponent *pLastComponent = nullptr;

	for (int i=0; i<count; ++i)
	{
		FBAttachType topType = (pLastComponent != nullptr) ? kFBAttachBottom : kFBAttachTop;
		const char *topName = (pLastComponent != nullptr) ? names[i-1] : "";

		AddRegion( names[i], names[i],
			lS, kFBAttachLeft,	"",			1.0,
			lS,	topType,		topName,	1.0,
			lW, kFBAttachNone,	"",			1.0,
			lH, kFBAttachNone,	"",			1.0 );
		SetControl( names[i], components[i] );

		pLastComponent = components[i];
	}
	*/
}

void MoRendererCallbackLayout::UICreateTools()
{
	FBLayout &l = mLayoutTools;

	const int lS=4;
	const int lH = 25;
	const int lW=400;
	const int lP=300;
	
	l.AddRegion( "buttonComposition", "buttonComposition",
			lS,		kFBAttachLeft,		"",			1.0,
			lS,		kFBAttachTop,		"",			1.0,
			-lS,	kFBAttachRight,		"",			1.0,
			lH,		kFBAttachNone,		"",			1.0 );
	
	l.AddRegion( "buttonDynamicMasks", "buttonDynamicMasks",
			lS,		kFBAttachLeft,		"",			1.0,
			lS,		kFBAttachBottom,	"buttonComposition",			1.0,
			-lS,	kFBAttachRight,		"",			1.0,
			lH,		kFBAttachNone,		"",			1.0 );

	l.AddRegion( "buttonTextureManager", "buttonTextureManager",
			lS,		kFBAttachLeft,		"",			1.0,
			lS,		kFBAttachBottom,	"buttonDynamicMasks",			1.0,
			-lS,	kFBAttachRight,		"",			1.0,
			lH,		kFBAttachNone,		"",			1.0 );

	l.AddRegion( "buttonCleanupManager", "buttonCleanupManager",
			lS,		kFBAttachLeft,		"",			1.0,
			lS,		kFBAttachBottom,	"buttonTextureManager",			1.0,
			-lS,	kFBAttachRight,		"",			1.0,
			lH,		kFBAttachNone,		"",			1.0 );

	l.AddRegion( "buttonExtendedRendering", "buttonExtendedRendering",
			lS,		kFBAttachLeft,		"",			1.0,
			lS,		kFBAttachBottom,	"buttonCleanupManager",			1.0,
			-lS,	kFBAttachRight,		"",			1.0,
			lH,		kFBAttachNone,		"",			1.0 );

	l.AddRegion( "buttonCmdLine", "buttonCmdLine",
			lS,		kFBAttachLeft,		"",			1.0,
			lS,		kFBAttachBottom,	"buttonExtendedRendering",			1.0,
			-lS,	kFBAttachRight,		"",			1.0,
			lH,		kFBAttachNone,		"",			1.0 );

	l.AddRegion( "buttonLights", "buttonLights",
			lS,		kFBAttachLeft,		"",			1.0,
			lS,		kFBAttachBottom,	"buttonCmdLine",			1.0,
			-lS,	kFBAttachRight,		"",			1.0,
			lH,		kFBAttachNone,		"",			1.0 );

	l.AddRegion( "buttonAbout", "buttonAbout",
			lS,		kFBAttachLeft,		"",			1.0,
			lS,		kFBAttachBottom,	"buttonLights",			1.0,
			-lS,	kFBAttachRight,		"",			1.0,
			lH,		kFBAttachNone,		"",			1.0 );

	l.SetControl( "buttonComposition", mButtonLaunchComposition );
	l.SetControl( "buttonDynamicMasks", mButtonLaunchDynamicMasks);
	l.SetControl( "buttonTextureManager", mButtonLaunchTextureManager );
	l.SetControl( "buttonCleanupManager", mButtonLaunchCleanupManager );
	l.SetControl( "buttonExtendedRendering", mButtonLaunchExtendedRendering );
	l.SetControl( "buttonCmdLine", mButtonLaunchCmdLineTool );
	l.SetControl( "buttonLights", mButtonLaunchLightsTool );
	l.SetControl( "buttonAbout", mButtonAbout );
}

/************************************************
*	Configure the UI.
************************************************/
void MoRendererCallbackLayout::UIConfigure()
{
	SetSplitStyle( "slider", kFBVSplit );

	UIConfigureOptions();
	UIConfigureTools();
}

void MoRendererCallbackLayout::UIConfigureOptions()
{
	UIConfigureProps( mLayoutOptions, RENDERER_COMMON_PROPS_COUNT, gCommonPropNames, mCommonProperties );
	/*
	MoRendererCallback *pCallback = (MoRendererCallback*) (FBRendererCallback*) RendererCallback;

	mEditRealTimeUpdate.Caption = "Update Type";
	mEditRealTimeUpdate.Property = &pCallback->UpdateType;

	mButtonUpdate.Caption = "Update";
    mButtonUpdate.OnClick.Add( this, (FBCallback)&MoRendererCallbackLayout::OnButtonUpdateClick );

	mEditUseGlobalResolution.Caption = "Use Global Resolution";
	mEditUseGlobalResolution.Property = &pCallback->UseGlobalResolution;
	mEditGlobalResolution.Caption = "Resolution Presets";
	mEditGlobalResolution.Property = pCallback->RenderResolution;

	mEditQualityPreset.Caption = "Quality Presets";
	mEditQualityPreset.Property = &pCallback->QualityPreset;
	mEditQualityDepth.Caption = "Display Depth";
	mEditQualityDepth.Property = &pCallback->DisplayDepth;
	
	mEditRenderOverride.Caption = "Render Override";
	mEditRenderOverride.Property = &pCallback->RenderOverride;
	mEditRenderQualityPreset.Caption = "Render Quality Presets";
	mEditRenderQualityPreset.Property = &pCallback->RenderQualityPreset;
	mEditRenderQualityDepth.Caption = "Render Depth";
	mEditRenderQualityDepth.Property = &pCallback->RenderDepth;
	*/
}

void MoRendererCallbackLayout::UIConfigureTools()
{
	mButtonLaunchComposition.Caption = "Composition Tree";
	mButtonLaunchComposition.OnClick.Add( this, (FBCallback) &MoRendererCallbackLayout::OnButtonCompositionClick );
	mButtonLaunchDynamicMasks.Caption = "Dynamic Masks";
	mButtonLaunchDynamicMasks.OnClick.Add( this, (FBCallback) &MoRendererCallbackLayout::OnButtonDynamicMasksClick );
	mButtonLaunchTextureManager.Caption = "Textures Manager";
	mButtonLaunchTextureManager.OnClick.Add( this, (FBCallback) &MoRendererCallbackLayout::OnButtonTexturesClick );
	mButtonLaunchCleanupManager.Caption = "CleanUp Manager";
	mButtonLaunchCleanupManager.OnClick.Add( this, (FBCallback) &MoRendererCallbackLayout::OnButtonCleanUpClick );
	mButtonLaunchExtendedRendering.Caption = "Extended Rendering";
	mButtonLaunchExtendedRendering.OnClick.Add( this, (FBCallback) &MoRendererCallbackLayout::OnButtonExtendedClick );
	mButtonLaunchCmdLineTool.Caption = "CmdLine Rendering";
	mButtonLaunchCmdLineTool.OnClick.Add( this, (FBCallback) &MoRendererCallbackLayout::OnButtonCmdLineClick );
	mButtonLaunchLightsTool.Caption = "Lights Visor";
	mButtonLaunchLightsTool.OnClick.Add( this, (FBCallback) &MoRendererCallbackLayout::OnButtonLightsClick );

	mButtonAbout.Caption = "About";
	mButtonAbout.OnClick.Add( this, (FBCallback) &MoRendererCallbackLayout::OnButtonAboutClick );

}

/************************************************
*	Reset the UI from the device.
************************************************/
void MoRendererCallbackLayout::UIReset()
{
    //mButtonTest.Caption = "Renderer (MoPlugs)";

	UIResetOptions();
	UIResetTools();
}

void MoRendererCallbackLayout::UIResetOptions()
{
}

void MoRendererCallbackLayout::UIResetTools()
{
}

void MoRendererCallbackLayout::OnResizeCallback(HISender pSender, HKEvent pEvent)
{
	FBEventResize	levent(pEvent);
	
	mLayoutWidth = levent.Width;
	mLayoutHeight = levent.Height - 120;

	UIResizeProps(mLayoutOptions, RENDERER_COMMON_PROPS_COUNT, gCommonPropNames, mCommonProperties );
}

/************************************************
*	Button clicked.
************************************************/
void MoRendererCallbackLayout::OnButtonUpdateClick(HISender pSender, HKEvent pEvent)
{
    //mButtonTest.Caption = "Clicked";
	MoRendererCallback *pCallback = (MoRendererCallback*) (FBRendererCallback*) RendererCallback;

	pCallback->AskToRenderAFrame();
}

void MoRendererCallbackLayout::OnButtonCompositionClick(HISender pSender, HKEvent pEvent)
{
	FBPopNormalTool( "Composition Tree Tool" );
}

void MoRendererCallbackLayout::OnButtonDynamicMasksClick(HISender pSender, HKEvent pEvent)
{
	FBPopNormalTool( "Dynamic Masks Tool" );
}

void MoRendererCallbackLayout::OnButtonTexturesClick(HISender pSender, HKEvent pEvent)
{
	FBPopNormalTool( "Textures Manager Tool" );
}

void MoRendererCallbackLayout::OnButtonCleanUpClick(HISender pSender, HKEvent pEvent)
{
	FBPopNormalTool( "CleanUp Manager Tool" );
}

void MoRendererCallbackLayout::OnButtonExtendedClick(HISender pSender, HKEvent pEvent)
{
	FBPopNormalTool( "Extended Rendering Tool" );
}

void MoRendererCallbackLayout::OnButtonCmdLineClick(HISender pSender, HKEvent pEvent)
{
	FBPopNormalTool( "CmdLine Rendering Tool" );
}

void MoRendererCallbackLayout::OnButtonLightsClick(HISender pSender, HKEvent pEvent)
{
	FBPopNormalTool( "Lights Visor Tool [Experimental]" );
}

void MoRendererCallbackLayout::OnButtonAboutClick(HISender pSender, HKEvent pEvent)
{
	FBMessageBox( "MoPlugs Renderer", 
		" MoPlugs Project"
		"\n  Created by Sergey Solokhin (Neill3d) 2017",
		"Ok" );
}

void MoRendererCallbackLayout::UICreateProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props)
{
	int lL = 4;
	int lS=4;
	int lH=20;
	int lW=300;

	FBString lastItem("");
	FBAttachType lastType = kFBAttachNone;

	//int layoutX, layoutY, layoutwidth, layoutheight;
	int layoutwidth = 400;
	int layoutheight = 400;
	
	int accH = lS;

	for (int i=0; i<count; ++i)
	{
		// go to next column
		if (accH > layoutheight)
		{
			lL += lW + 30;
			accH = lS;
		}

		layout.AddRegion( names[i], names[i],
			lL,		kFBAttachNone,	"",	1.0,
			accH,	kFBAttachNone,	"",	1.0,
			lW, kFBAttachNone,	"",	1.0,
			lH,	kFBAttachNone,	"", 1.0 );
		layout.SetControl( names[i], props[i] );

		lastItem = names[i];
		lastType = kFBAttachBottom;

		accH += lH + lS * 2;
	}
}

void MoRendererCallbackLayout::UIConfigureProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props)
{
	MoRendererCallback *pCallback = (MoRendererCallback*) (FBRendererCallback*) RendererCallback;

	for (int i=0; i<count; ++i)
	{
		//if (i==17) break;
		props[i].Property = pCallback->PropertyList.Find(names[i]);
		props[i].Caption = names[i];
	}
}

void MoRendererCallbackLayout::UIResizeProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props)
{
	int lL = 4;
	int lS=4;
	int lH=20;
	int lW=300;

	// compute number of columns

	const int layoutwidth = (mLayoutWidth > 0) ? mLayoutWidth : 400;
	const int layoutheight = (mLayoutHeight > 0) ? mLayoutHeight + 100 : 400;

	int accH = lS * 2;

	for (int i=0; i<count; ++i)
	{
		// go to next column
		if (accH > layoutheight)
		{
			lL += lW + 30;
			accH = lS;
		}

		layout.MoveRegion( names[i], lL, accH );

		accH += lH + lS * 2;
	}

	lL += (lW + 30);
	MoveRegion( "slider", lL, lS );
}