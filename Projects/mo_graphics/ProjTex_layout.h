#ifndef __PROJTEX_LAYOUT_H__
#define __PROJTEX_LAYOUT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ProjTex_layout.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <fbsdk.h>
#include "ProjTex_shader.h"


//! Simple device layout.
class ProjTexLayout : public FBShaderLayout
{
    // Declaration.
    FBShaderLayoutDeclare( ProjTexLayout, FBShaderLayout );

public:

    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate();
    virtual void FBDestroy();

    // Callback for transparency type changes in UI.
    void EventTransparencyChange(HISender pSender, HKEvent pEvent);
	void EventButtonAbout(HISender pSender, HKEvent pEvent);

	void EventTabChange(HISender pSender, HKEvent pEvent);

	void EventButtonSelect(HISender pSender, HKEvent pEvent);

private:

    // UI Management
    void	UICreate	();
	void		UICreateCommon	();
	void		UICreateProjectors	();
	void		UICreateShading	();
	void		UICreateInfo	();
    void	UIConfigure	();
	void		UIConfigureCommon	();
	void		UIConfigureProjectors	();
	void		UIConfigureShading	();
	void		UIConfigureInfo	();
    void	UIReset		();
	void		UIResetCommon		();
	void		UIResetProjectors	();
	void		UIResetShading		();
	void		UIResetInfo		();

private:

    // The shader.
    ProjTexShader* mShader;

    // UI variables.
    FBTabPanel	mTabPanel;

	//
	// Common items and tab
	//
	FBLayout	mLayoutCommon;

	FBLabel						mTransLabel;
    FBLabel						mAffectLabel;
    FBLabel						mTransFactorLabel;
    FBList						mTransparency;
    FBEditProperty				mTransparencyFactor;
    FBPropertyConnectionEditor	mAffectingLights;
	FBEditProperty				mUseSceneLights;
	//FBEditProperty				mAccurateDepth;

	FBEditProperty				mEditMasks[4];

	FBButton					mButtonAbout;

	
	//
	// 2 - projectors layout
	//
	FBLayout	mLayoutProjectors;

	// projective mapping - 4 projectors
	struct
	{
		 
		FBLabel							mLabelProj;
		FBEditProperty					mButtonProjUse;	// flag if we will use projection
		FBEditProperty					mButtonProjAspect;
		FBPropertyConnectionEditor		mContainerCamera;	// texture projection based on camera view matrix
		FBButton						mButtonProjSelect;
		FBPropertyConnectionEditor		mContainerTexture;
		FBButton						mButtonTexSelect;
		FBEditProperty					mEditMode;
		FBEditProperty					mEditOpacity;
		FBEditProperty					mEditMask;
		FBEditProperty					mEditMaskChannel;

	} mProjectorsUI[MAX_PROJECTORS_COUNT];

	FBEditProperty						mDynamicUpdate;
	FBLabel								mLabelMask1;
	FBPropertyConnectionEditor			mMask1;
	FBLabel								mLabelMask2;
	FBPropertyConnectionEditor			mMask2;
	
	//
	// 3 shading options
	//
	FBLayout			mLayoutShading;

	FBEditProperty		mPropertyShading;	// choose a shading type

	FBEditProperty		mEditToonSteps;
	FBEditProperty		mEditToonDistribution;
	FBEditProperty		mEditToonShadowPosition;

	FBEditProperty		mEditBrightness;
	FBEditProperty		mEditSaturation;
	FBEditProperty		mEditContrast;
	FBEditProperty		mEditGamma;

	FBEditProperty		mEditCustomColor;
	FBEditProperty		mEditCustomColorMode;

	FBEditProperty		mEditDetailMode;
	FBEditProperty		mEditDetailOpacity;
	FBEditProperty		mEditDetailMasked;

	//
	//	4 help information page (tips for fast goes up)
	//
	FBLayout			mLayoutInfo;

	FBLabel				mLabelInfo;
};

#endif /* __PROJTEX_LAYOUT_H__ */
