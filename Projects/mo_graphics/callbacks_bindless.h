
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: callbacks_bindless.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "callbacks_binded.h"
#include "shared_content.h"



////////////////////////////////////////////////////////////////////////////
//
class CShaderGPUCacheCallback : public CProjTexBindedCallback
{
public:

	//! a constructor
	CShaderGPUCacheCallback(const int uniqueId)
		: CProjTexBindedCallback(uniqueId)
	{
		mIsBindless = true;
		// TODO: for test only - override with a flat shading !
		mNeedOverrideShading = true;
	}

	//! a destructor
	virtual ~CShaderGPUCacheCallback()
	{}

	// make a higher priority, means we use this callback for a specified shaders
	virtual const int PriorityLevel() const override
	{
		return 2;
	}

	// gpu cache callback is drawing model itself rather then using FBModel geometry to render
	virtual const bool IsMaterialEffect() const override
	{
		return false;
	}

	///////////////// INFO METHODS
	
	virtual const char *shaderName() const override
	{
		return "GPUCache Callback";
	}

	// callback is for shader and specified pass (for example for drawing transparent)
	//  or callback could combine support for several shader or passes inside
	virtual const bool IsForShader(FBShader *pShader) override;
	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) override;
	
	virtual const int ShaderGroupGoal()  const override
	{
		return eRenderGoalSelectionId | eRenderGoalShading | eRenderGoalShadows;
	}

	//
	// main callback methods during the render

	virtual void DetachRenderContext(FBRenderOptions *pFBRenderOptions, FBShader *pShader) override;

	virtual	bool OnTypeBegin(const CRenderOptions &options, bool useMRT) override
	{
		return true;
	}
	virtual void OnTypeEnd(const CRenderOptions &options) override
	{
	}
	virtual bool OnInstanceBegin(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo) override;
	virtual void OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo) override;

	// executed once before rendering whole frame
	virtual const bool IsNeedToPrepareInstance() const override {
		return true;
	}
	virtual void PrepareInstance(const CRenderOptions &options, FBShader *pShader) override;
	
	virtual const bool IsNeedToPrepareModel() const override {
		return false;
	}
	virtual void PrepareModel(const CRenderOptions &options, FBModel *pModel, FBShader *pShader) override;

	// executed once per layer and pass
	virtual bool OwnModelShade(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBModel *pModel, FBShader *pShader, CBaseShaderInfo *pInfo) override;

};
