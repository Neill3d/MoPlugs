
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: callbacks_particles.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "callbacks.h"

//////////////////////////////////////////////////////////////////////
//
class CGPUParticlesCallback : public CBaseShaderCallback
{
public:

	//! a constructor
	CGPUParticlesCallback(const int uniqueId)
		: CBaseShaderCallback(uniqueId)
	{
		mIsBindless = false;
		mIsEarlyZ = false;
		mNeedOverrideShading = false;
		mRenderToNormalAndMaskOutput = false;
	}

	virtual const char *shaderName() const override
	{
		return "Particles";
	}

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

	virtual const int ShaderGroupGoal() const override
	{
		return eRenderGoalSelectionId | eRenderGoalShading | eRenderGoalShadows;
	}

	virtual const bool IsForShader(FBShader *pShader) override;
	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) override;

	//
	// main callback methods during the render

	virtual void DetachRenderContext(FBRenderOptions* pFBRenderOptions, FBShader *pShader) override;

	virtual	bool OnTypeBegin(const CRenderOptions &options, bool useMRT) override
	{
		return true;
	}
	virtual void OnTypeEnd(const CRenderOptions &options) override
	{
	}
	virtual bool OnInstanceBegin(const CRenderOptions &options, FBRenderOptions* pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo) override;
	virtual void OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo) override;

	// executed once before rendering whole frame
	virtual const bool IsNeedToPrepareInstance() const override {
		return false;
	}
	//virtual void PrepareInstance(const CRenderOptions &options, FBShader *pShader) override;
	
	virtual const bool IsNeedToPrepareModel() const override {
		return false;
	}
	//virtual void PrepareModel(const CRenderOptions &options, FBModel *pModel, FBShader *pShader) override;

	// executed once per layer and pass
	virtual bool OwnModelShade(const CRenderOptions &options, FBRenderOptions* pFBRenderOptions, FBModel *pModel, FBShader *pShader, CBaseShaderInfo *pInfo) override;

protected:

	bool				mIsBindless;
	bool				mIsEarlyZ;
	bool				mNeedOverrideShading;

	bool				mRenderToNormalAndMaskOutput;

};