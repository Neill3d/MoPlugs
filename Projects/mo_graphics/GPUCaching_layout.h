
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: GPUCaching_layout.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "GPUCaching_shader.h"

//#include "textures_view.h"

//! Simple device layout.
class ORShaderGPUCachingLayout : public FBShaderLayout
{
	//--- FiLMBOX declaration.
	FBShaderLayoutDeclare( ORShaderGPUCachingLayout, FBShaderLayout );

public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();			//!< FiLMBOX constructor.
	virtual void FBDestroy();			//!< FiLMBOX destructor.

private:
	// UI Management
	void	UICreate	();
	void	UICreateModels();
	void	UICreateMaterials();
	void	UICreateTextures();
	void	UICreateShaders();
	void	UIConfigure	();
	void	UIConfigureModels	();
	void	UIConfigureMaterials	();
	void	UIConfigureTextures	();
	void	UIConfigureShaders();
	void	UIReset		();
	void	UIResetModels		();
	void	UIResetMaterials		();
	void	UIResetTextures		();
	void	UIResetShaders();

	// UI Callbacks
	void	OnButtonTestClick(HISender pSender, HKEvent pEvent);

	void	OnPaintEvent(HISender pSender, HKEvent pEvent);
	void	OnResizeEvent(HISender pSender, HKEvent pEvent);

	void	OnTabChangeEvent(HISender pSender, HKEvent pEvent);

private:
	// UI Elements
	FBButton		mButtonTest;

	FBTabPanel		mTabPanel;

	FBLayout		mLayoutModels;
	FBList			mListModels;

	FBLayout		mLayoutMaterials;
	FBList			mListMaterials;

	FBLayout		mLayoutTextures;
	//ORView3D		mViewTextures;

	FBLayout		mLayoutShaders;
	FBList			mListShaders;

private:
	// The shader.
	ORShaderGPUCache	*mShader;

};
