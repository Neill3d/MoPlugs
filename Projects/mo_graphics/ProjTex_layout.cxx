
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ProjTex_layout.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "ProjTex_layout.h"
#include "ProjTex_shader.h"

//--- Registration define
#define PROJTEX__LAYOUT		ProjTexLayout

//--- FiLMBOX implementation and registration
FBShaderLayoutImplementation(	PROJTEX__LAYOUT		);
FBRegisterShaderLayout		(	PROJTEX__LAYOUT,
                             PROJTEX__DESCSTR,
                             FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)

/************************************************
*	FiLMBOX constructor.
************************************************/
bool ProjTexLayout::FBCreate()
{
    mShader	= (ProjTexShader*)(FBShader*)Shader;

    UICreate	();
    UIConfigure	();
    UIReset		();

    return true;
}


/************************************************
*	FiLMBOX destructor.
************************************************/
void ProjTexLayout::FBDestroy()
{
}


/************************************************
*	Create the UI.
************************************************/
void ProjTexLayout::UICreate()
{
	int lB = 10;
    int lS = 4;
    int lW = 100;
    int lH = 28;


	AddRegion("About","About",
        -lW,	kFBAttachRight,	"",		1.00,
        2,		kFBAttachNone,	"",		1.00,
        lW,		kFBAttachNone,	"",		1.00,
        lH,		kFBAttachNone,	"",		1.00 );

	AddRegion("TabPanel","TabPanel",
        0,		kFBAttachLeft,	"",			1.00,
        0,		kFBAttachNone,	"",			1.00,
        0,		kFBAttachLeft,	"About",	1.00,
        lH,		kFBAttachNone,	"",			1.00 );

	AddRegion("Canvas","Canvas",
        2,		kFBAttachLeft,	"",				1.00,
        2,		kFBAttachBottom,"TabPanel",		1.00,
        -2,		kFBAttachRight,	"",				1.00,
        -2,		kFBAttachBottom,"",				1.00 );

	SetControl( "About",			mButtonAbout );
	SetControl( "TabPanel",			mTabPanel );
	SetControl( "Canvas",			mLayoutCommon );

	UICreateCommon();
	UICreateProjectors();
	UICreateShading();
	UICreateInfo();
}


void ProjTexLayout::UICreateCommon()
{
	int lB = 10;
    int lS = 4;
    int lW = 100;
    int lH = 18;

    mLayoutCommon.AddRegion("TransLabel",	"TransLabel",
        lB,		kFBAttachLeft,	"",			1.00,
        lB,		kFBAttachTop,	"",			1.00,
        lW,		kFBAttachNone,	NULL,		1.00,
        lH,		kFBAttachNone,	NULL,		1.00 );
    mLayoutCommon.AddRegion("Transparency","Transparency",
        lS,		kFBAttachRight,	"TransLabel",	1.00,
        0,		kFBAttachTop,	"TransLabel",	1.00,
        2*lW,	kFBAttachNone,	NULL,		1.00,
        0,		kFBAttachHeight,"TransLabel",	1.00 );

    mLayoutCommon.AddRegion("TransFactorLabel",	"TransFactorLabel",
        lB,		kFBAttachLeft,	"",			1.00,
        lB,		kFBAttachBottom,"TransLabel", 1.00,
        lW,		kFBAttachNone,	NULL,		1.00,
        lH,		kFBAttachNone,	NULL,		1.00 );
    mLayoutCommon.AddRegion("TransparencyFactor",	"TransparencyFactor",
        lS,		kFBAttachRight,	"TransFactorLabel",		1.00,
        0,		kFBAttachTop,   "TransFactorLabel",		1.00,
        2*lW,	kFBAttachNone,	NULL,		1.00,
        lH,		kFBAttachNone,	NULL,		1.00 );

    mLayoutCommon.AddRegion("AffectLabel",	"AffectLabel",
        lB,		kFBAttachLeft,	"",			1.00,
        lB,		kFBAttachBottom,"TransFactorLabel", 1.00,
        lW,		kFBAttachNone,	NULL,		1.00,
        lH,		kFBAttachNone,	NULL,		1.00 );
    mLayoutCommon.AddRegion("AffectingLights","AffectingLights",
        lS,		kFBAttachRight,	"AffectLabel",	1.00,
        0,		kFBAttachTop,	"AffectLabel",	1.00,
        2*lW,	kFBAttachNone,	NULL,		1.00,
        0,		kFBAttachHeight,"AffectLabel",	1.00 );
	mLayoutCommon.AddRegion("UseSceneLights","UseSceneLights",
        lB,		kFBAttachLeft,	"",			1.00,
        lB,		kFBAttachBottom,"AffectLabel", 1.00,
        2*lW,		kFBAttachNone,	NULL,		1.00,
        lH,		kFBAttachNone,	NULL,		1.00 );
	mLayoutCommon.AddRegion("AccurateDepth","AccurateDepth",
        lB,		kFBAttachLeft,	"",			1.00,
        lB,		kFBAttachBottom,"UseSceneLights", 1.00,
        2*lW,		kFBAttachNone,	NULL,		1.00,
        lH,		kFBAttachNone,	NULL,		1.00 );

	const char *maskNames[4] = { "editMaskA", "editMaskB", "editMaskC", "editMaskD" };

	FBAttachType lastType = kFBAttachNone;
	FBString lastItem("");
	for (int i=0; i<4; ++i)
	{
		mLayoutCommon.AddRegion(maskNames[i], maskNames[i],
			4*lW + 2*lB,		kFBAttachLeft,		"",			1.00,
			lB,				lastType,			lastItem,	1.00,
			2*lW,			kFBAttachNone,		NULL,		1.00,
			lH,				kFBAttachNone,		NULL,		1.00 );

		lastType = kFBAttachBottom;
		lastItem = maskNames[i];

		mLayoutCommon.SetControl( maskNames[i], mEditMasks[i] );
	}

	mLayoutCommon.SetControl( "TransLabel",   mTransLabel );
    mLayoutCommon.SetControl( "Transparency", mTransparency );

    mLayoutCommon.SetControl( "TransFactorLabel", mTransFactorLabel );
    mLayoutCommon.SetControl( "TransparencyFactor", mTransparencyFactor );

    mLayoutCommon.SetControl( "AffectLabel",      mAffectLabel);
    mLayoutCommon.SetControl( "AffectingLights",  mAffectingLights);
	mLayoutCommon.SetControl( "UseSceneLights", mUseSceneLights );
	//mLayoutCommon.SetControl( "AccurateDepth", mAccurateDepth );
}

void ProjTexLayout::UICreateProjectors()
{
	int lB = 10;
    int lS = 4;
    int lW = 100;
    int lH = 18;
	int lT = 20;

	mLayoutProjectors.AddRegion("LabelMask1",	"LabelMask1",
			lB,		kFBAttachLeft,	"",	1.00,
			lT,		kFBAttachNone,	"",	1.00,
			lW,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

	mLayoutProjectors.AddRegion("Mask1",	"Mask1",
			lB,		kFBAttachRight,	"LabelMask1",	1.00,
			lT,		kFBAttachNone,	"",	1.00,
			lW,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

	mLayoutProjectors.AddRegion("LabelMask2",	"LabelMask2",
			lB,		kFBAttachLeft,	"",	1.00,
			lT,		kFBAttachBottom,"Mask1",	1.00,
			lW,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

	mLayoutProjectors.AddRegion("Mask2",	"Mask2",
			lB,		kFBAttachRight,	"LabelMask2",	1.00,
			lT,		kFBAttachBottom,"Mask1",	1.00,
			lW,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

	FBString	lastItem("Mask2");	// "AffectLabel"
	
	const int len=128;
	char	buffer[len];
	memset(buffer, 0, sizeof(char)*128);
	
	for (int i=0; i<MAX_PROJECTORS_COUNT; ++i)
	{
		FBString prevItem;

		sprintf_s(buffer, len, "ButtonProjUse%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachLeft,		"",	1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW/4,	kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		prevItem = buffer;

		sprintf_s(buffer, len, "LabelProj%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachRight,		prevItem,			1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW*0.75,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		prevItem = buffer;
	
		sprintf_s(buffer, len, "ButtonProjAspect%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachRight,		prevItem,	1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW*0.75,kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		prevItem = buffer;
		
		sprintf_s(buffer, len, "ProjCamera%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachRight,		prevItem,	1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		prevItem = buffer;
		
		sprintf_s(buffer, len, "ProjCameraSelect%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachRight,		prevItem,	1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW/4,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		prevItem = buffer;

		sprintf_s(buffer, len, "ProjTexture%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachRight,		prevItem,	1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		prevItem = buffer;

		sprintf_s(buffer, len, "ProjTexSelect%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachRight,		prevItem,	1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW/4,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		prevItem = buffer;

		sprintf_s(buffer, len, "ProjMode%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachRight,		prevItem,	1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW*1.5,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		prevItem = buffer;
		
		sprintf_s(buffer, len, "ProjOpacity%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachRight,		prevItem,	1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW*2,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		prevItem = buffer;

		sprintf_s(buffer, len, "ProjMask%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachRight,		prevItem,	1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW*1.5,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		prevItem = buffer;
		
		sprintf_s(buffer, len, "ProjMaskChannel%d", i+1);
		mLayoutProjectors.AddRegion(buffer,	buffer,
			lB,		kFBAttachRight,		prevItem,	1.00,
			lT,		(lastItem=="") ? kFBAttachNone : kFBAttachBottom,	lastItem,	1.00,
			lW*1.5,		kFBAttachNone,		NULL,		1.00,
			lH,		kFBAttachNone,		NULL,		1.00 );

		lastItem = buffer;

		mLayoutProjectors.SetControl( "LabelMask1", mLabelMask1 );
		mLayoutProjectors.SetControl( "Mask1", mMask1 );
		mLayoutProjectors.SetControl( "LabelMask2", mLabelMask2 );
		mLayoutProjectors.SetControl( "Mask2", mMask2 );

		sprintf_s(buffer, len, "LabelProj%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mLabelProj );
		
		sprintf_s(buffer, len, "ButtonProjUse%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mButtonProjUse );

		sprintf_s(buffer, len, "ButtonProjAspect%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mButtonProjAspect );
		
		sprintf_s(buffer, len, "ProjCamera%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mContainerCamera );

		sprintf_s(buffer, len, "ProjCameraSelect%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mButtonProjSelect );

		sprintf_s(buffer, len, "ProjTexture%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mContainerTexture );

		sprintf_s(buffer, len, "ProjTexSelect%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mButtonTexSelect );

		sprintf_s(buffer, len, "ProjMode%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mEditMode );

		sprintf_s(buffer, len, "ProjOpacity%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mEditOpacity );

		sprintf_s(buffer, len, "ProjMask%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mEditMask );

		sprintf_s(buffer, len, "ProjMaskChannel%d", i+1);
		mLayoutProjectors.SetControl( buffer, mProjectorsUI[i].mEditMaskChannel );
	}
}

void ProjTexLayout::UICreateShading()
{
	int lB = 10;
    int lS = 4;
    int lW = 100;
    int lH = 18;

	mLayoutShading.AddRegion("Shading","Shading",
        lB,		kFBAttachLeft,	"",		1.00,
        lB,		kFBAttachNone,	"",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );

	mLayoutShading.AddRegion("ToonSteps","ToonSteps",
        lB,		kFBAttachLeft,	"",		1.00,
        2*lB,		kFBAttachBottom,	"Shading",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );
	mLayoutShading.AddRegion("ToonShadowPosition","ToonShadowPosition",
        lB,		kFBAttachLeft,	"",		1.00,
        lB,		kFBAttachBottom,	"ToonSteps",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );
	mLayoutShading.AddRegion("ToonDistribution","ToonDistribution",
        lB,		kFBAttachLeft,	"",		1.00,
        lB,		kFBAttachBottom,	"ToonShadowPosition",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );

	mLayoutShading.AddRegion("Brightness","Brightness",
        lB,		kFBAttachLeft,	"",		1.00,
        2*lB,		kFBAttachBottom,	"ToonDistribution",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );
	mLayoutShading.AddRegion("Saturation","Saturation",
        lB,		kFBAttachLeft,	"",		1.00,
        lB,		kFBAttachBottom,	"Brightness",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );
	mLayoutShading.AddRegion("Contrast","Contrast",
        lB,		kFBAttachLeft,	"",		1.00,
        lB,		kFBAttachBottom,	"Saturation",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );
	mLayoutShading.AddRegion("Gamma","Gamma",
        lB,		kFBAttachLeft,	"",		1.00,
        lB,		kFBAttachBottom,	"Contrast",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );

	mLayoutShading.AddRegion("CustomColor","CustomColor",
        35,		kFBAttachRight,	"ToonSteps",		1.00,
        0,		kFBAttachTop,	"ToonSteps",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );

	mLayoutShading.AddRegion("CustomColorMode","CustomColorMode",
        0,		kFBAttachLeft,	"CustomColor",		1.00,
        lB,		kFBAttachBottom,	"CustomColor",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );


	mLayoutShading.AddRegion("DetailMode","DetailMode",
        0,		kFBAttachLeft,	"CustomColorMode",		1.00,
        2*lB,	kFBAttachBottom,"CustomColorMode",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );

	mLayoutShading.AddRegion("DetailOpacity","DetailOpacity",
        0,		kFBAttachLeft,	"CustomColor",		1.00,
        lB,		kFBAttachBottom,"DetailMode",		1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );
	mLayoutShading.AddRegion("DetailMasked","DetailMasked",
        0,		kFBAttachLeft,	"CustomColor",		1.00,
        lB,		kFBAttachBottom,"DetailOpacity",	1.00,
        2*lW,	kFBAttachNone,	NULL,	1.00,
        lH,		kFBAttachNone,	"",		1.00 );

	mLayoutShading.SetControl( "Shading",	mPropertyShading );
	
	mLayoutShading.SetControl( "ToonSteps",	mEditToonSteps );
	mLayoutShading.SetControl( "ToonShadowPosition",	mEditToonShadowPosition );
	mLayoutShading.SetControl( "ToonDistribution",	mEditToonDistribution );

	mLayoutShading.SetControl( "Brightness",	mEditBrightness );
	mLayoutShading.SetControl( "Saturation",	mEditSaturation );
	mLayoutShading.SetControl( "Contrast",	mEditContrast );
	mLayoutShading.SetControl( "Gamma",	mEditGamma );

	mLayoutShading.SetControl( "CustomColor", mEditCustomColor );
	mLayoutShading.SetControl( "CustomColorMode", mEditCustomColorMode );

	mLayoutShading.SetControl( "DetailMode", mEditDetailMode );
	mLayoutShading.SetControl( "DetailOpacity", mEditDetailOpacity );
	mLayoutShading.SetControl( "DetailMasked", mEditDetailMasked );
}

void ProjTexLayout::UICreateInfo()
{
	int lB = 10;
    int lS = 4;
    int lW = 100;
    int lH = 18;

	mLayoutInfo.AddRegion("Info","Info",
        lB,		kFBAttachLeft,	"",		1.00,
        lB,		kFBAttachTop,	"",		1.00,
        -lB,	kFBAttachRight,	"",		1.00,
        -lB,	kFBAttachBottom,"",		1.00 );


	mLayoutInfo.SetControl( "Info", mLabelInfo );
}

/************************************************
*	Configure the UI.
************************************************/
void ProjTexLayout::UIConfigure()
{
	mButtonAbout.Caption = "About";
	mButtonAbout.OnClick.Add( this, (FBCallback) &ProjTexLayout::EventButtonAbout );

	mTabPanel.Items.SetString("Common~Projectors~Shading~Info");
	mTabPanel.OnChange.Add( this, (FBCallback) &ProjTexLayout::EventTabChange );

	UIConfigureCommon();
	UIConfigureProjectors();
	UIConfigureShading();
	UIConfigureInfo();
}

void ProjTexLayout::UIConfigureCommon()
{
	mTransLabel.Caption = "Transparency Type";

    FBPropertyAlphaSource lEnum;
    for( int i = 0; i < 7; ++i )
    {
        mTransparency.Items.Add( lEnum.EnumList(i), i );
    }
    mTransparency.OnChange.Add( this, (FBCallback)&ProjTexLayout::EventTransparencyChange );

    mTransFactorLabel.Caption = "Transparency Factor";
    mTransparencyFactor.Property = &mShader->TransparencyFactor;


    mAffectLabel.Caption = "Affecting Lights";
    mAffectingLights.Property = &mShader->AffectingLights;

	mUseSceneLights.Caption = "Use Scene Lights";
	mUseSceneLights.Property = &mShader->UseSceneLights;

	//mAccurateDepth.Caption = "Accurate Depth";
	//mAccurateDepth.Property = &mShader->LogarithmicDepth;

	mEditMasks[0].Caption = "Mask A";
	mEditMasks[0].Property = &mShader->CompositeMaskA;

	mEditMasks[1].Caption = "Mask B";
	mEditMasks[1].Property = &mShader->CompositeMaskB;

	mEditMasks[2].Caption = "Mask C";
	mEditMasks[2].Property = &mShader->CompositeMaskC;

	mEditMasks[3].Caption = "Mask D";
	mEditMasks[3].Property = &mShader->CompositeMaskD;
}

void ProjTexLayout::UIConfigureProjectors()
{
	auto *pProjectorUI = &mProjectorsUI[0];
	auto *pProjector = &mShader->Projectors[0];

	// projector 1
	char buffer[128];
	memset(buffer, 0, sizeof(char) * 128);

	for (int i=0; i<MAX_PROJECTORS_COUNT; ++i)
	{	
		sprintf_s(buffer, 128, "Projector %d - ", i+1 );

		pProjectorUI->mLabelProj.Caption = buffer;
		pProjectorUI->mLabelProj.Style = kFBTextStyleBold;

		pProjectorUI->mButtonProjUse.Caption = "";
		pProjectorUI->mButtonProjUse.Property = &pProjector->ProjectorUse;

		pProjectorUI->mButtonProjAspect.Caption = "Aspect";
		pProjectorUI->mButtonProjAspect.Property = &pProjector->ProjectorAspect;

		pProjectorUI->mContainerCamera.Property = &pProjector->Projector;
		pProjectorUI->mButtonProjSelect.Caption = "S";
		pProjectorUI->mButtonProjSelect.OnClick.Add( this, (FBCallback) &ProjTexLayout::EventButtonSelect );
		pProjectorUI->mContainerTexture.Property = &pProjector->Texture;
		pProjectorUI->mButtonTexSelect.Caption = "S";
		pProjectorUI->mButtonTexSelect.OnClick.Add( this, (FBCallback) &ProjTexLayout::EventButtonSelect );

		pProjectorUI->mEditMode.Caption = "Blend";
		pProjectorUI->mEditMode.Property = &pProjector->BlendMode;
		pProjectorUI->mEditOpacity.Caption = "Blend Opacity";
		pProjectorUI->mEditOpacity.Property = &pProjector->BlendOpacity;

		pProjectorUI->mEditMask.Caption = "Mask";
		pProjectorUI->mEditMask.Property = &pProjector->Mask;
		pProjectorUI->mEditMaskChannel.Caption = "Channel";
		pProjectorUI->mEditMaskChannel.Property = &pProjector->MaskChannel;

		pProjector++;
		pProjectorUI++;
	}

	mDynamicUpdate.Caption = "Dynamic Update";
	mDynamicUpdate.Property = &mShader->DynamicUpdate;
	mLabelMask1.Caption = "Mask1";
	mMask1.Property = &mShader->Mask1;
	mLabelMask2.Caption = "Mask2";
	mMask2.Property = &mShader->Mask2;
}

void ProjTexLayout::UIConfigureShading()
{
	mPropertyShading.Caption = "Shading type";
	mPropertyShading.Property = &mShader->ShadingType;

	mEditToonDistribution.Caption = "Toon distribution";
	mEditToonDistribution.Property = &mShader->ToonDistribution;

	mEditToonShadowPosition.Caption = "Toon shadow position";
	mEditToonShadowPosition.Property = &mShader->ToonShadowPosition;

	mEditToonSteps.Caption = "Toon steps";
	mEditToonSteps.Property = &mShader->ToonSteps;

	mEditBrightness.Caption = "Brightness";
	mEditBrightness.Property = &mShader->Brightness;

	mEditSaturation.Caption = "Saturation";
	mEditSaturation.Property = &mShader->Saturation;

	mEditContrast.Caption = "Contrast";
	mEditContrast.Property = &mShader->Contrast;

	mEditGamma.Caption = "Gamma";
	mEditGamma.Property = &mShader->Gamma;

	mEditCustomColor.Caption = "Custom color";
	mEditCustomColor.Property = &mShader->CustomColor;

	mEditCustomColorMode.Caption = "Custom color mode";
	mEditCustomColorMode.Property = &mShader->CustomColorMode;

	mEditDetailMasked.Caption = "Detail masked";
	mEditDetailMasked.Property = &mShader->DetailMasked;

	mEditDetailMode.Caption = "Detail blend mode";
	mEditDetailMode.Property = &mShader->DetailBlendMode;

	mEditDetailOpacity.Caption = "Detail blend opacity";
	mEditDetailOpacity.Property = &mShader->DetailOpacity;
}

void ProjTexLayout::UIConfigureInfo()
{
	//mLabelInfo.Width = 400;
	mLabelInfo.Justify = kFBTextJustifyLeft;
	mLabelInfo.SetTextColor( FBColorAndAlpha(0.7, 0.8, 0.65, 1.0) );
	mLabelInfo.Caption = "Projection shader plugin\n\n";
}

/************************************************
*	Reset the UI from the device.
************************************************/
void ProjTexLayout::UIReset()
{
	UIResetCommon();
	UIResetProjectors();
	UIResetShading();
	UIResetInfo();

	mTabPanel.ItemIndex = 0;
	SetControl("Canvas", mLayoutCommon );
}

void ProjTexLayout::UIResetCommon()
{
	mTransparency.ItemIndex = (int)mShader->GetTransparencyType();
}

void ProjTexLayout::UIResetProjectors()
{
}

void ProjTexLayout::UIResetShading()
{
}

void ProjTexLayout::UIResetInfo()
{
}

void ProjTexLayout::EventTransparencyChange( HISender /*pSender*/, HKEvent /*pEvent*/ )
{
    mShader->SetTransparencyType( (FBAlphaSource)(int)mTransparency.ItemIndex );
}

void ProjTexLayout::EventButtonSelect( HISender pSender, HKEvent pEvent )
{
	FBComponent* lComp = FBGetFBComponent( (HIObject) pSender ); 

	for (int i=0; i<MAX_PROJECTORS_COUNT; ++i)
	{
		if ( lComp == &(mProjectorsUI[i].mButtonProjSelect))
		{
			mShader->DoProjectorSelect(i);
		}
		else
		if ( lComp == &(mProjectorsUI[i].mButtonTexSelect))
		{
			mShader->DoTextureSelect(i);
		}
	}
}

void ProjTexLayout::EventButtonAbout( HISender /*pSender*/, HKEvent /*pEvent*/ )
{
    FBMessageBox( "Projective mapping plugin", "Author Sergey Solohin (Neill3d) 2013\n e-mail to s@neill3d.com\n www.neill3d.com", "Ok" );
}

void ProjTexLayout::EventTabChange( HISender /*pSender*/, HKEvent /*pEvent*/ )
{
	switch(mTabPanel.ItemIndex)
	{
	case 0: // common
		SetControl("Canvas", mLayoutCommon);
		break;
	case 1: // projectors page
		SetControl("Canvas", mLayoutProjectors);
		break;
	case 2: // shading page
		SetControl("Canvas", mLayoutShading);
		break;
	case 3: // information page
		SetControl("Canvas", mLayoutInfo);
		break;
	}
}
