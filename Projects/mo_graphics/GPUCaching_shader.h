
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: GPUCaching_shader.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ORSDK_DLL
	/** \def ORSDK_DLL
	*	Be sure that ORSDK_DLL is defined only once...
	*/
	#define ORSDK_DLL K_DLLIMPORT
#endif

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "ShaderFX.h"
#include "..\Common\Common_shader.h"

#include "ProjTex_shader.h"

#include "gpucache_model.h"
#include "gpucache_loader.h"

#include "shared_content.h"
#include "shared_models_newton.h"

//--- Registration define
#define ORSHADERGPUCACHE__CLASSNAME	ORShaderGPUCache
#define ORSHADERGPUCACHE__CLASSSTR	"ORShaderGPUCache"
#define ORSHADERGPUCACHE__DESCSTR	"GPUCache"

//////////////////////////////////////////////////////////////////////
//! GPU PreCached geometry
class ORSDK_DLL ORShaderGPUCache : public ProjTexShader
{
	//--- FiLMBOX declaration.
	FBShaderDeclare( ORShaderGPUCache, ProjTexShader );

public:

	//FBPropertyListObject				AffectingLights;		//!< Selected Lights to illuminate the connected models (to avoid maximum lights number limitation in OpenGL)
	//FBPropertyBool						UseSceneLights;			//!< should we use all avaliable scene lights or just default lighting model

	FBPropertyString		SourceFileName;	//!< scene from which we have saved a cache
	FBPropertyAction		LaunchSourceFile;

	FBPropertyString		FileName;	//!< cache filename to work with
	
	FBPropertyAction		Load;
	FBPropertyAction		Reload;		//!< reload cached objects
	FBPropertyAction		ReloadShader;	//

	FBPropertyBool			Loaded;		//!< flag for checking load status

	FBPropertyAction		SaveTextures;	//!< resave textures to store compressed data inside

	// global cache override
	FBPropertyBool						OverrideShading;
	/*
	FBPropertyBaseEnum<EShadingType>	ShadingType;

	FBPropertyAnimatableDouble			ToonSteps;
	FBPropertyAnimatableDouble			ToonDistribution;
	FBPropertyAnimatableDouble			ToonShadowPosition;

	FBPropertyBool						RenderOnBack;
	*/
	// some statistics
	FBPropertyInt			ObjectsCount;
	FBPropertyInt			MaterialsCount;
	FBPropertyInt			TexturesCount;
	FBPropertyInt			ShadersCount;
	FBPropertyInt			LightCount;

	FBPropertyInt			MemoryUsage;	//!< how much gpu memory is used by this cache

	//FBPropertyBool			LogarithmicDepth;

	FBPropertyBool			SampleAlphaToCoverage;	//!< do we need to sample by using alpha
	FBPropertyDouble		AlphaPass;				//!< amount of alpha to discard in the shader

	FBPropertyAction		DebugSnapshot;	//!< make a snapshot of clusters
	FBPropertyListObject	DebugCamera;
	FBPropertyBool			DebugDisplay;

	FBPropertyBool			DisplayNormals;
	FBPropertyDouble		NormalsLength;

public:
	virtual bool FBCreate() override;		//!< FiLMBOX Constructor.
	virtual void FBDestroy() override;		//!< FiLMBOX Destructor.

	//
	virtual bool ShaderNeedBeginRender() override;
	virtual void ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo ) override;

	virtual void ShadeModel( FBRenderOptions* pRenderOptions, 
		FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass ) override;

	//
	//void InternalShadeModel( FBCamera *pCamera, FBModel *pModel, const bool cubemapSetup, const CubeMapRenderingData *data=nullptr );

	void CustomDisplay(const int index, FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight);

	void	DebugGeometryOutput();
	bool ClosestRayIntersection(const FBTVector& pRayOrigin, const FBTVector& pRayEnd, FBTVector& pIntersectPos, FBTVector& pIntersecNormal);

	virtual bool FbxStore	(FBFbxObject* pFbxObject);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, FBRenderer* pRenderer);

	/**	Detach the display context from the shader.
		*	\param	pOptions	Render options.
		*	\param	pInfo		Shader-model information object.
		*/
	virtual void DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo );

	void RenderForShadows( FBCamera *pCamera, FBModel *pModel );


public:

	void FileChangeEvent(HISender pSender, HKEvent pEvent);
	
public:

	void DoLaunchSource();
	void DoLoad();
	void DoReload(FBFbxObject* pFbxObject);
	void DoReloadShader();
	void DoDebugSnapshot();
	void DoSaveTextures();

	CGPUCacheModel		*GetGPUCacheModelPtr()
	{
		return mCacheModel;
	}

	void AssignTexturesView( FBView *pView )
	{
		mTexturesView = pView;
	}
	void AssignUILists( FBList *pListModels, FBList *pListMaterials, FBList *pListShaders )
	{
		mListModels = pListModels;
		mListMaterials = pListMaterials;
		mListShaders = pListShaders;
	}

	void		UpdateUI();

	FBVector3d		GetModelsCenter();
	FBVector3d		GetModelsSize();
	/*
	void SetLogarithmicDepth(const bool value) const
	{
		// TODO: where to store logarithmic depth value ?!
		if (mUberShader)
		{
			mUberShader->SetLogarithmicDepth(value);
		}
	}
	*/
	void SetDebugDisplay(const bool value);

	const FBMatrix &GetLoadMatrix() const {
		return mLoadMatrix;
	}

	const bool HasTransparentGeometry() {
		//return mModelRender->HasTransparentGeometry();
		return false;
	}
	/*
private:

	GLboolean			mCullFace;
	GLint				mCullFaceMode;

	void				StoreCullMode();
	void				FetchCullMode();
	*/

	const FBMatrix &GetLoadMatrix() {
		return mLoadMatrix;
	}
	const FBMatrix &GetLoadMatrixInv() {
		return mLoadMatrixInv;
	}
private:
    //FBSystem				mSystem;

	bool					mSuccess;

	bool						mNeedUpdateTexturesPtr;

	//std::vector<FBLight*>				mLightsPtr;
	//std::auto_ptr<CGPUShaderLights>		mShaderLights;
	
	// cached resources
	CGPUCacheModel				*mCacheModel;

	// pre-cached model information 
	//CGPUModelRenderCached		*mModelRender;
	//CGPUVertexData				*mVertexData;

	// for ray casting
	GPUCacheGeometry			*mQueryCacheGeometry;
	PHYSICS_INTERFACE::IWorld	*mCacheWorld;

	CRenderOptions			mOptions;

	// GUI pointer to update textures view
	FBView					*mTexturesView;
	FBList					*mListModels;
	FBList					*mListMaterials;
	FBList					*mListShaders;

	// bb
	FBVector3d				mMin;
	FBVector3d				mMax;

	FBMatrix				mLoadMatrix;
	FBMatrix				mLoadMatrixInv;

	// gfx
	//CGPUFBScene				*mGPUFBScene;
	//Graphics::ShaderEffect	*mUberShader;

	GLuint					mQueryOcclusion;

	// pass exchange variables
	//float					mCalculatedRealFarPlane;
	mat4					mModelView;
	mat4					mProjection;

	void	LoadFromFileName(FBFbxObject* pFbxObject);

	//void PassPreRender(FBCamera *pCamera, FBModel *pModel, const bool cubemapSetup, const CubeMapRenderingData *data=nullptr);
	//void PassLighted(FBCamera *pCamera, FBModel *pModel, const bool cubemapSetup);
};
