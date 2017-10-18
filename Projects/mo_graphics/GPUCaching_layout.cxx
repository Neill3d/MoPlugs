
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: GPUCaching_layout.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "GPUCaching_layout.h"

//--- Registration define
#define ORSHADERGPUCACHE__LAYOUT		ORShaderGPUCachingLayout

//--- FiLMBOX implementation and registration
FBShaderLayoutImplementation(	ORSHADERGPUCACHE__LAYOUT		);
FBRegisterShaderLayout		(	ORSHADERGPUCACHE__LAYOUT,
								ORSHADERGPUCACHE__DESCSTR,
								FB_DEFAULT_SDK_ICON			);

/************************************************
 *	FiLMBOX constructor.
 ************************************************/
bool ORShaderGPUCachingLayout::FBCreate()
{
	mShader	= (ORShaderGPUCache*)(FBShader*)Shader;
//	mShader->AssignTexturesView( &mViewTextures );
	mShader->AssignUILists( &mListModels, &mListMaterials, &mListShaders );
	mShader->UpdateUI();

	UICreate	();
	UIConfigure	();
	UIReset		();

	OnPaint.Add( this, (FBCallback) &ORShaderGPUCachingLayout::OnPaintEvent );
	OnResize.Add( this, (FBCallback) &ORShaderGPUCachingLayout::OnResizeEvent );
	OnIdle.Add( this, (FBCallback) &ORShaderGPUCachingLayout::OnPaintEvent );

	return true;
}


/************************************************
 *	FiLMBOX destructor.
 ************************************************/
void ORShaderGPUCachingLayout::FBDestroy()
{
}


/************************************************
 *	Create the UI.
 ************************************************/
void ORShaderGPUCachingLayout::UICreate()
{
	const int lS=4;
	const int lH=28;
	//int lW=100;

	AddRegion( "Tab", "Tab",
			lS, kFBAttachLeft,	"",	1.0,
			lS,	kFBAttachTop,	"",	1.0,
			-lS,kFBAttachRight,	"",	1.0,
			lH, kFBAttachNone,	"", 1.0 );
	
	AddRegion( "TabCanvas", "TabCanvas",
			lS, kFBAttachLeft,	"",	1.0,
			lS,	kFBAttachBottom,"Tab",	1.0,
			-lS,kFBAttachRight,	"",	1.0,
			-lS,kFBAttachBottom,"", 1.0 );

	SetControl( "Tab", mTabPanel );
	SetControl( "TabCanvas", mLayoutModels );

	UICreateModels();
	UICreateMaterials();
	UICreateTextures();
	UICreateShaders();
}

void ORShaderGPUCachingLayout::UICreateModels()
{
	const int lS=4;

	mLayoutModels.AddRegion( "List", "List",
			lS, kFBAttachLeft,	"",	1.0,
			lS,	kFBAttachTop,	"",	1.0,
			-lS,kFBAttachRight,	"",	1.0,
			-lS,kFBAttachBottom,"", 1.0 );

	mLayoutModels.SetControl( "List", mListModels );
}

void ORShaderGPUCachingLayout::UICreateMaterials()
{
	const int lS=4;

	mLayoutMaterials.AddRegion( "List", "List",
			lS, kFBAttachLeft,	"",	1.0,
			lS,	kFBAttachTop,	"",	1.0,
			-lS,kFBAttachRight,	"",	1.0,
			-lS,kFBAttachBottom,"", 1.0 );

	mLayoutMaterials.SetControl( "List", mListMaterials );
}

void ORShaderGPUCachingLayout::UICreateTextures()
{
	int lS=4;

	mLayoutTextures.AddRegion( "View", "View",
			lS, kFBAttachLeft,	"",	1.0,
			lS,	kFBAttachTop,	"",	1.0,
			-lS,kFBAttachRight,	"",	1.0,
			-lS,kFBAttachBottom,"", 1.0 );
	
//	mLayoutTextures.SetView( "View", mViewTextures );
}

void ORShaderGPUCachingLayout::UICreateShaders()
{
	const int lS=4;

	mLayoutShaders.AddRegion( "List", "List",
			lS, kFBAttachLeft,	"",	1.0,
			lS,	kFBAttachTop,	"",	1.0,
			-lS,kFBAttachRight,	"",	1.0,
			-lS,kFBAttachBottom,"", 1.0 );

	mLayoutShaders.SetControl( "List", mListShaders );
}

/************************************************
 *	Configure the UI.
 ************************************************/
void ORShaderGPUCachingLayout::UIConfigure()
{
	mTabPanel.OnChange.Add( this, (FBCallback) &ORShaderGPUCachingLayout::OnTabChangeEvent );

	mTabPanel.Items.SetString( "SubModels~Materials~Textures~Shaders~Lights" );

	//mButtonTest.OnClick.Add( this, (FBCallback)&ORShaderGPUCachingLayout::OnButtonTestClick );

	UIConfigureModels();
	UIConfigureMaterials();
	UIConfigureTextures();
	UIConfigureShaders();
}

void ORShaderGPUCachingLayout::UIConfigureModels()
{
	mListModels.Style = kFBVerticalList;
}

void ORShaderGPUCachingLayout::UIConfigureMaterials()
{
	mListMaterials.Style = kFBVerticalList;
}

void ORShaderGPUCachingLayout::UIConfigureTextures()
{
//	mViewTextures.AssignTexturesManager( mShader->GetTexturesManager() );
}

void ORShaderGPUCachingLayout::UIConfigureShaders()
{
	mListShaders.Style = kFBVerticalList;
}

/************************************************
 *	Reset the UI from the device.
 ************************************************/
void ORShaderGPUCachingLayout::UIReset()
{
	mButtonTest.Caption = "Test";

	UIResetModels();
	UIResetMaterials();
	UIResetTextures();
	UIResetShaders();
}

void ORShaderGPUCachingLayout::UIResetModels()
{
}

void ORShaderGPUCachingLayout::UIResetMaterials()
{
}

void ORShaderGPUCachingLayout::UIResetTextures()
{
}

void ORShaderGPUCachingLayout::UIResetShaders()
{
}

/************************************************
 *	Button clicked.
 ************************************************/
void ORShaderGPUCachingLayout::OnButtonTestClick(HISender pSender, HKEvent pEvent)
{
	mButtonTest.Caption = "Clicked";
}

void ORShaderGPUCachingLayout::OnPaintEvent(HISender pSender, HKEvent pEvent)
{
//	mViewTextures.Refresh();
}


void ORShaderGPUCachingLayout::OnResizeEvent(HISender pSender, HKEvent pEvent)
{
//	mViewTextures.Resize(10, 10);
}

void ORShaderGPUCachingLayout::OnTabChangeEvent(HISender pSender, HKEvent pEvent)
{
	switch(mTabPanel.ItemIndex)
	{
	case 0: SetControl( "TabCanvas", mLayoutModels );
		break;
	case 1: SetControl( "TabCanvas", mLayoutMaterials );
		break;
	case 2: SetControl( "TabCanvas", mLayoutTextures );
		break;
	case 3: SetControl( "TabCanvas", mLayoutShaders );
		break;
	}
}