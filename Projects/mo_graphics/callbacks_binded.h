
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: callbacks_binded.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "callbacks.h"
#include "shared_content.h"

#include "FX_shader.h"



//////////////////////////////////////////////////////////////////////
//
class CUberShaderCallback : public CBaseShaderCallback
{
public:

	//! a constructor
	CUberShaderCallback(const int uniqueId)
		: CBaseShaderCallback(uniqueId)
	{
		mGPUFBScene = &CGPUFBScene::instance();
		mShaderFX = nullptr;
		mLastModel = nullptr;
		mLastMaterial = nullptr;

		mIsBindless = false;
		mIsEarlyZ = false;
		mNeedOverrideShading = false;
		mRenderToNormalAndMaskOutput = false;

		mCurrentTech = Graphics::MATERIAL_SHADER_LEGACY;
	}

	virtual const char *shaderName() const override
	{
		return "Binded";
	}

	virtual const int ShaderGroupGoal() const override
	{
		return eRenderGoalSelectionId | eRenderGoalShading | eRenderGoalShadows;
	}

	virtual const bool IsForShader(FBShader *pShader) override;
	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) override;

	virtual bool OnTypeBegin(const CRenderOptions &options, bool useMRT) override;
	virtual void OnTypeEnd(const CRenderOptions &options) override;

	virtual bool OnInstanceBegin(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo) override;
	virtual void OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo) override;

	// model sub-mesh has a material assigned
	virtual bool OnMaterialBegin(const CRenderOptions &options, FBRenderOptions *pRenderOptions, FBMaterial *pMaterial, bool forceInit) override;
	virtual void OnMaterialEnd(const CRenderOptions &options, FBMaterial *pMaterial) override;
	
	virtual bool OnModelDraw(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBModel *pModel, CBaseShaderInfo *pInfo) override;

protected:

	//Graphics::EEffectTechnique		mCurrentTech;
	int					mCurrentTech;

	bool				mIsBindless;
	bool				mIsEarlyZ;
	bool				mNeedOverrideShading;

	bool				mRenderToNormalAndMaskOutput;

	CGPUFBScene							*mGPUFBScene;
	Graphics::BaseMaterialShaderFX		*mShaderFX;

	OGLCullFaceInfo		mCullFaceInfo;
	
	FXProjectionMapping					*mLastFXShader;

	FBModel								*mLastModel;
	FBMaterial							*mLastMaterial;

	//FBModelCullingMode					mLastCullingMode;

	void		BindMaterialTextures(FBMaterial *pMaterial, FBRenderOptions *pRenderOptions, const GLuint mutedTextureId, bool forceInit);
};

///////////////////////////////////////////////////////////////////////////////////
//
class CProjTexBindedInfo : CBaseShaderInfo
{
public:

	//! a constructor
	CProjTexBindedInfo(FBShader *pShader)
		: CBaseShaderInfo(pShader)
	{
		mGPUFBScene = &CGPUFBScene::instance();
		mNeedUpdateLightsList = true;
	}

	//! a destructor
	virtual ~CProjTexBindedInfo()
	{
	}

	virtual void EventBeforeRenderNotify() override;

	virtual void ChangeContext() override
	{
		mNeedUpdateLightsList = true;
	}

	virtual void EventConnNotify(HISender pSender, HKEvent pEvent) override;

protected:

	CGPUFBScene					*mGPUFBScene;

	bool								mNeedUpdateLightsList;

	std::vector<FBLight*>				mLightsPtr;
	std::auto_ptr<CGPUShaderLights>		mShaderLights;


	void	UpdateLightsList()
	{
		mNeedUpdateLightsList = true;
	}
};

///////////////////////////////////////////////////////////////////////////////////
//
class CProjTexBindedCallback : public CUberShaderCallback
{
public:

	//! a constructor
	CProjTexBindedCallback(const int uniqueId)
		: CUberShaderCallback(uniqueId)
	{
		mFXProjectionBinded = false;
		hasProjectors = false;
		mShader = nullptr;
		mCurrentTech = Graphics::MATERIAL_SHADER_PROJECTORS;
	}

	//! a destructor
	virtual ~CProjTexBindedCallback()
	{}

	// make a higher priority, means we use this callback for a specified shaders
	virtual const int PriorityLevel() const override
	{
		return 1;
	}

	virtual const bool IsNeedShaderInfo() const override
	{
		return true;
	}

	virtual CBaseShaderInfo	*CreateANewShaderInfo(FBShader *pShader) override
	{
		return (CBaseShaderInfo*) new CProjTexBindedInfo(pShader);
	}

	// callback is for shader and specified pass (for example for drawing transparent)
	//  or callback could combine support for several shader or passes inside
	virtual const bool IsForShader(FBShader *pShader) override;
	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) override;

	virtual const int ShaderGroupGoal()  const override
	{
		return eRenderGoalSelectionId | eRenderGoalShading | eRenderGoalShadows;
	}
	
	//virtual const int GetSupportedPassesByShader(FBShader *pShader);
	
	//virtual bool OnTypeBegin(const EShaderPass pass) override;
	//virtual void OnTypeEnd(const EShaderPass pass) override;

	virtual bool OnInstanceBegin(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo) override;
	virtual void OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo) override;

	// model sub-mesh has a material assigned
	virtual bool OnMaterialBegin(const CRenderOptions &options, FBRenderOptions *pRenderOptions, FBMaterial *pMaterial, bool forceInit) override;
	virtual void OnMaterialEnd(const CRenderOptions &options, FBMaterial *pMaterial) override;
	
	virtual bool OnModelDraw(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBModel *pModel, CBaseShaderInfo *pInfo) override;

	// executed one time per rendering frame (to setup gpu buffers, update data, etc.)
	//  update local light list and projectors
	//virtual void PreRenderSetup(const CRenderOptions &options, FBModel *pModel, FBShader *pShader) override;

	virtual const bool IsNeedToPrepareInstance() const override {
		return true;
	}
	virtual const bool IsNeedToPrepareModel() const override {
		return false;
	}

	virtual void PrepareInstance(const CRenderOptions &options, FBShader *pShader) override;
	virtual void PrepareModel(const CRenderOptions &options, FBModel *pModel, FBShader *pShader) override;

protected:

	bool				mFXProjectionBinded;
	bool				hasProjectors;
	bool				hasExclusiveLights;

	FBShader			*mShader;

	void InternalInstanceBegin(const bool textureMapping, const GLuint muteTextureId);
	void InternalInstanceEnd();
	void InternalModelDraw(const CRenderOptions &options, FBModel *pModel, const bool textureMapping, const GLuint muteTextureId);
};


///////////////////////////////////////////////////////////////////////////////////
//
class CCharacterBindedInfo : CBaseShaderInfo
{
public:

	//! a constructor
	CCharacterBindedInfo(FBShader *pShader)
		: CBaseShaderInfo(pShader)
	{
		mGPUFBScene = &CGPUFBScene::instance();
		mNeedUpdateLightsList = true;
	}

	//! a destructor
	virtual ~CCharacterBindedInfo()
	{
	}

	virtual void EventBeforeRenderNotify() override;

	virtual void ChangeContext() override
	{
		mNeedUpdateLightsList = true;
	}

	virtual void EventConnNotify(HISender pSender, HKEvent pEvent) override;

protected:

	CGPUFBScene					*mGPUFBScene;

	bool								mNeedUpdateLightsList;

	std::vector<FBLight*>				mLightsPtr;
	std::auto_ptr<CGPUShaderLights>		mShaderLights;


	void	UpdateLightsList()
	{
		mNeedUpdateLightsList = true;
	}
};

///////////////////////////////////////////////////////////////////////////////////
//
class CIBLBindedCallback : public CUberShaderCallback
{
public:

	//! a constructor
	CIBLBindedCallback(const int uniqueId)
		: CUberShaderCallback(uniqueId)
	{
		mShader = nullptr;
		//mCurrentTech = Graphics::eEffectTechniqueIBL;
		mCurrentTech = Graphics::MATERIAL_SHADER_IBL;
		eyePass = false;
	}

	//! a destructor
	virtual ~CIBLBindedCallback()
	{}

	// make a higher priority, means we use this callback for a specified shaders
	virtual const int PriorityLevel() const override
	{
		return 1;
	}

	virtual const bool IsNeedShaderInfo() const override
	{
		return true;
	}

	virtual CBaseShaderInfo	*CreateANewShaderInfo(FBShader *pShader) override
	{
		return (CBaseShaderInfo*) new CCharacterBindedInfo(pShader);
	}

	// callback is for shader and specified pass (for example for drawing transparent)
	//  or callback could combine support for several shader or passes inside
	virtual const bool IsForShader(FBShader *pShader) override;
	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) override;

	virtual const int ShaderGroupGoal()  const override
	{
		return eRenderGoalSelectionId | eRenderGoalShading | eRenderGoalShadows;
	}
	
	//virtual const int GetSupportedPassesByShader(FBShader *pShader);
	
	//virtual bool OnTypeBegin(const EShaderPass pass) override;
	//virtual void OnTypeEnd(const EShaderPass pass) override;

	virtual bool OnInstanceBegin(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo) override;
	virtual void OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo) override;

	// model sub-mesh has a material assigned
	virtual bool OnMaterialBegin(const CRenderOptions &options, FBRenderOptions *pRenderOptions, FBMaterial *pMaterial, bool forceInit) override;
	virtual void OnMaterialEnd(const CRenderOptions &options, FBMaterial *pMaterial) override;
	
	virtual bool OnModelDraw(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBModel *pModel, CBaseShaderInfo *pInfo) override;

	// executed one time per rendering frame (to setup gpu buffers, update data, etc.)
	//  update local light list and projectors
	//virtual void PreRenderSetup(const CRenderOptions &options, FBModel *pModel, FBShader *pShader) override;

	virtual const bool IsNeedToPrepareInstance() const override {
		return true;
	}
	virtual const bool IsNeedToPrepareModel() const override {
		return true;
	}

	virtual void PrepareInstance(const CRenderOptions &options, FBShader *pShader) override;
	virtual void PrepareModel(const CRenderOptions &options, FBModel *pModel, FBShader *pShader) override;

	virtual const bool IsNeedToProcessFrameSync() const override {
		return true;
	}
	virtual const bool IsNeedToProcessFrameEvaluation() const override {
		return true;
	}

	virtual void FrameSync(FBShader *pShader) override;
	virtual void FrameEvaluation(FBShader *pShader) override;

protected:

	bool				eyePass;
	bool				hasExclusiveLights;

	FBShader			*mShader;

	virtual void InternalInstanceBegin(const bool textureMapping, const GLuint muteTextureId);
	void InternalInstanceEnd();
	void InternalModelDraw(const CRenderOptions &options, FBModel *pModel, const bool textureMapping, const GLuint muteTextureId);

protected:

	// TODO: assign eye pass if needed
	virtual bool OnTypeBeginPredefine(const CRenderOptions &options, bool useMRT) override
	{
		bool lSuccess = false;
		mShaderFX = mGPUFBScene->GetShaderFXPtr(mCurrentTech);

		if ( nullptr != mShaderFX )
		{
			mShaderFX->ModifyShaderFlags( Graphics::eShaderFlag_EyePass, eyePass );
			lSuccess = true;
		}
		
		return lSuccess;
	}
	
public:

	// bind output normal buffer for a draw call
	//bool		BindOutputNormalBuffer(FBModel *pModel, GLuint slot);
};


///////////////////////////////////////////////////////////////////////////////////
//
class CEyeBindedCallback : public CIBLBindedCallback
{
public:

	//! a constructor
	CEyeBindedCallback(const int uniqueId)
		: CIBLBindedCallback(uniqueId)
	{
		eyePass = true;

		eyeEnvReflSamplerSlot = -1;
		eyeEnvDiffSamplerSlot = -1;
		eyeEnvRefrSamplerSlot = -1;
	}

	//! a destructor
	virtual ~CEyeBindedCallback()
	{}

	virtual const bool IsForShader(FBShader *pShader) override;
	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) override;

protected:

	int eyeEnvReflSamplerSlot;
	int eyeEnvDiffSamplerSlot;
	int eyeEnvRefrSamplerSlot;

	void CheckForSamplerSlots();

	virtual void InternalInstanceBegin(const bool textureMapping, const GLuint muteTextureId) override;

};

//////////////////////////////////////////////////////////////////////////
//
/*
class CShaderBindedBackground : public CUberShaderCallback
{
public:
	
	// a constructor
	CShaderBindedBackground(const int uniqueId)
		: CUberShaderCallback(uniqueId)
	{}

	
	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) override
	{
		return (true == IsShaderBackground(pShader) && pass == eShaderPassBackground);
	}

	virtual const char *shaderName() const override
	{
		return "Binded Background";
	}

	virtual const int CallbackPasses() const override
	{
		return eShaderPassBackground;
	}
};
*/
////////////////////////////////////////////////////////////////////////////
//
/*
class CShaderBindedTransparent : public CUberShaderCallback
{
public:

	//! a constructor
	CShaderBindedTransparent(const int uniqueId)
		: CUberShaderCallback(uniqueId)
	{}

	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) override
	{
		return (true == IsShaderTransparent(pShader) && pass == eShaderPassTransparency );
	}


	virtual const char *shaderName() const override
	{
		return "Binded Transparent";
	}

	virtual const int CallbackPasses()  const override
	{
		return eShaderPassTransparency;
	}
};
*/

////////////////////////////////////////////////////////////////////
// render with models unique color id
class CShaderSelectionCallback : public CUberShaderCallback
{
public:

	//! a constructor
	CShaderSelectionCallback(const int uniqueId)
		: CUberShaderCallback(uniqueId)
	{
		mNeedOverrideShading = true;
		mIsEarlyZ = true;
	}

	//! a destructor
	virtual ~CShaderSelectionCallback()
	{
	}

	virtual const char *shaderName() const override
	{
		return "Simple";
	}

	// supported passes by this callback
	virtual const int ShaderGroupGoal()  const override
	{
		return eRenderGoalSelectionId;
	}

	// simple shader could render every model (for id selection for example)
	//	used for SELECTION pass for example
	virtual const bool IsForShader(FBShader *pShader) override
	{
		return true;
	}

	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) override
	{
		return true;
	}

	virtual bool OnInstanceBegin(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo) override;
	virtual void OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo) override;

	// model sub-mesh has a material assigned
	virtual bool OnMaterialBegin(const CRenderOptions &options, FBRenderOptions *pRenderOptions, FBMaterial *pMaterial, bool forceInit) override
	{}
	virtual void OnMaterialEnd(const CRenderOptions &options, FBMaterial *pMaterial) override
	{}
	virtual bool OnModelDraw(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBModel *pModel, CBaseShaderInfo *pInfo) override
	{
		return true;
	}

protected:

	FBShader	*mLastShader;
};

//////////////////////////////////////////////////////////////////////
//
class CShaderShadowsCallback : public CBaseShaderCallback
{
public:

	//! a constructor
	CShaderShadowsCallback(const int uniqueId)
		: CBaseShaderCallback(uniqueId)
	{}

	virtual const char *shaderName() const override
	{
		return "Shadows";
	}

	// supported passes by this callback
	virtual const int ShaderGroupGoal()  const override
	{
		return eRenderGoalShadows;
	}

	// TODO: bind a shadows technique !

	// simple shader could render every model (for id selection for example)
	//	used for SELECTION pass for example
	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) override
	{
		return true;
	}
};