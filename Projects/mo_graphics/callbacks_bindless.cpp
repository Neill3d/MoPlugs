
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: callbacks_bindless.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "callbacks_bindless.h"
#include "GPUCaching_shader.h"

const bool CShaderGPUCacheCallback::IsForShader(FBShader *pShader)
{

	return FBIS(pShader, ORShaderGPUCache);
	/*
	if (FBIS( pShader,	ORShaderGPUCache ) )
	{
		ORShaderGPUCache *pGPUCache = (ORShaderGPUCache*) pShader;

		const bool isRenderOnBack = (true == pGPUCache->RenderOnBack);
		const bool isTransparent = pGPUCache->HasTransparentGeometry();

		switch (pass)
		{
		case eShaderPassBackground:
			return (true == isRenderOnBack);
			
		case eShaderPassTransparency:
			return (true == isTransparent);
			
		case eShaderPassOpaque:
			return (false == isRenderOnBack && false == isTransparent);
		}

	}

	return false;
	*/
}

const bool CShaderGPUCacheCallback::IsForShaderAndPass(FBShader *pShader, const EShaderPass pass)
{

	return FBIS(pShader, ORShaderGPUCache);
	/*
	if (FBIS( pShader,	ORShaderGPUCache ) )
	{
		ORShaderGPUCache *pGPUCache = (ORShaderGPUCache*) pShader;

		const bool isRenderOnBack = (true == pGPUCache->RenderOnBack);
		const bool isTransparent = pGPUCache->HasTransparentGeometry();

		switch (pass)
		{
		case eShaderPassBackground:
			return (true == isRenderOnBack);
			
		case eShaderPassTransparency:
			return (true == isTransparent);
			
		case eShaderPassOpaque:
			return (false == isRenderOnBack && false == isTransparent);
		}

	}

	return false;
	*/
}

void CShaderGPUCacheCallback::PrepareInstance(const CRenderOptions &options, FBShader *pShader)
{
	if (true == mGPUFBScene->IsWaiting() || nullptr == pShader || nullptr == mGPUFBScene->GetUberShaderPtr() )
		return;

	if ( !FBIS(pShader, ORShaderGPUCache) )
		return;

	mUberShader = mGPUFBScene->GetUberShaderPtr();

	ORShaderGPUCache *pGPUCache = (ORShaderGPUCache*) pShader; // pInfo->GetFBShader();
	CGPUCacheModel *pCacheModel = pGPUCache->GetGPUCacheModelPtr();

	if (pCacheModel == nullptr)
		return;

	// models normal matrix update, according to camera modelview
	pCacheModel->PrepRender(mGPUFBScene->GetCameraCache(), mUberShader, false, nullptr);

	// update projectors and local lighting gpu buffers
	CProjTexBindedCallback::PrepareInstance(options, pShader);
}

void CShaderGPUCacheCallback::PrepareModel(const CRenderOptions &options, FBModel *pModel, FBShader *pShader)
{
	if (true == mGPUFBScene->IsWaiting() || nullptr == pShader || nullptr == mGPUFBScene->GetUberShaderPtr())
		return;

	if ( !FBIS(pShader, ORShaderGPUCache) )
		return;

	mUberShader = mGPUFBScene->GetUberShaderPtr();

	ORShaderGPUCache *pGPUCache = (ORShaderGPUCache*) pShader; // pInfo->GetFBShader();
	CGPUCacheModel *pCacheModel = pGPUCache->GetGPUCacheModelPtr();

	if (pCacheModel == nullptr)
		return;

#ifdef DEBUG_RENDERBUG
			FBTrace( "  --- update receive shadows\n" );
#endif

	pCacheModel->UpdateReceiveShadows( pModel->ReceiveShadows.AsInt() );
}

void CShaderGPUCacheCallback::DetachRenderContext(FBRenderOptions *pFBRenderOptions, FBShader *pShader)
{
	if (nullptr == pShader)
		return;

	ORShaderGPUCache *pGPUCache = (ORShaderGPUCache*) pShader; // pInfo->GetFBShader();

	if (nullptr != pGPUCache)
		return;

	CGPUCacheModel *pCacheModel = pGPUCache->GetGPUCacheModelPtr();
	if (nullptr != pCacheModel)
		pCacheModel->NeedUpdateTexturePtr();
}

bool CShaderGPUCacheCallback::OnInstanceBegin(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	if (nullptr == pShader || !FBIS(pShader, ORShaderGPUCache) )
		return false;
	if (true == mGPUFBScene->IsWaiting() || nullptr == mGPUFBScene->GetUberShaderPtr() )
		return false;
	
	mUberShader = mGPUFBScene->GetUberShaderPtr();

	mShader = pShader;
	ORShaderGPUCache *pGPUCache = (ORShaderGPUCache*) pShader; // pInfo->GetFBShader();
	CGPUCacheModel *pCacheModel = pGPUCache->GetGPUCacheModelPtr();

	if (pCacheModel == nullptr)
		return false;
	
	ERenderGoal goal = options.GetGoal();
	EShaderPass pass = options.GetPass();
	bool acceptShader = true;

	switch(pass)
	{
	case eShaderPassOpaque:
		if (pCacheModel->GetNumberOfOpaqueCommands() == 0 
			|| pCacheModel->GetNumberOfTransparencyCommands() > 0)
			acceptShader = false;
		break;
	case eShaderPassTransparency:
		if (pCacheModel->GetNumberOfTransparencyCommands() == 0)
			acceptShader = false;
		break;
	case eShaderPassAdditive:
		acceptShader = false;
		break;
	}

	if (false == acceptShader)
		return false;

	const bool isEarlyZ = (eRenderGoalSelectionId == goal || eRenderGoalShadows == goal);

	if (false == isEarlyZ)
	{
		pCacheModel->OverrideShading = pGPUCache->OverrideShading;
		pCacheModel->ShadingType = pGPUCache->ShadingType;
	}

	// ?! why we should run it everytime ?!
	//pCacheModel->NeedUpdateTexturePtr();
	
	pCacheModel->RenderBegin(mGPUFBScene->GetCameraCache(), mUberShader, isEarlyZ, 
		options.IsCubeMapRender(), options.GetCubeMapData() );
	
	// check for projectors and local lighting
	if (false == isEarlyZ)
	{
		mGPUFBScene->BindLights(true);
		InternalInstanceBegin( options.IsTextureMappingEnable(), options.GetMutedTextureId() );
		//InternalModelDraw(pModel, options.IsTextureMappingEnable() );
	}

	return acceptShader;
}

bool CShaderGPUCacheCallback::OwnModelShade(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBModel *pModel, FBShader *pShader, CBaseShaderInfo *pInfo)
{

	if (true == mGPUFBScene->IsWaiting() || nullptr == pModel || nullptr == mShader
		|| nullptr == mUberShader)
	{
		return false;
	}

	if ( !FBIS( mShader, ORShaderGPUCache ) )
		return false;

	ORShaderGPUCache *pGPUCache = (ORShaderGPUCache*) mShader; // pInfo->GetFBShader();
	CGPUCacheModel *pCacheModel = pGPUCache->GetGPUCacheModelPtr();

	if (pCacheModel == nullptr)
		return false;
	
#ifdef DEBUG_RENDERBUG
			FBTrace( "  --- upload model matrix\n" );
#endif

	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	FBMatrix tm;
	pModel->GetMatrix(tm);
	//pCacheModel->SetCacheMatrix(tm);

	FBMatrix m;

		pModel->GetMatrix(m);
		FBMatrixMult( m, m, pGPUCache->GetLoadMatrixInv() );

		//mUberShader->UploadModelTransform(m);
		//pCacheModel->SetCacheMatrix(m);
		mUberShader->UploadModelTransform(m);
	//
	//
	const ERenderGoal goal = options.GetGoal();
	const EShaderPass pass = options.GetPass();

	const bool isEarlyZ = (eRenderGoalSelectionId == goal && eRenderGoalShadows != goal);

	if (true == isEarlyZ)
	{
		FBColor colorId = pModel->UniqueColorId;
		vec3	fcolorId( (float)colorId[0], (float)colorId[1], (float)colorId[2] );

		pCacheModel->UpdateColorId(fcolorId);
	}

	// check for projectors and local lighting
	if (false == isEarlyZ)
	{
#ifdef DEBUG_RENDERBUG
		FBTrace( "  --- internal model draw\n" );
#endif
		pCacheModel->UpdateReceiveShadows( pModel->ReceiveShadows.AsInt() );
		InternalModelDraw(options, pModel, options.IsTextureMappingEnable(), options.GetMutedTextureId() );
	}
	//mGPUFBScene->PrepShaderLights( UseSceneLights, &AffectingLights, mLightsPtr, mShaderLights );

#ifdef DEBUG_RENDERBUG
			FBTrace( "  --- render commands\n" );
#endif
	
	if ( eShaderPassOpaque == pass )
	{
		pCacheModel->RenderOpaque(mGPUFBScene->GetCameraCache(), mUberShader);
	}
	else
	{
		//glEnable(GL_DEPTH_TEST);
		//glDepthMask(GL_FALSE);
		//glDisable(GL_BLEND);
		//glDisable(GL_CULL_FACE);

		// don't need to draw in separate passes, sample alpha to coverage uses depth buffer
		if (options.IsMultisampling() )
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			
		pCacheModel->RenderOpaque(mGPUFBScene->GetCameraCache(), mUberShader);
		
		if (options.IsMultisampling() )
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			
		pCacheModel->RenderTransparent(mGPUFBScene->GetCameraCache(), mUberShader);
	}
	
	return true;
}

void CShaderGPUCacheCallback::OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	if (true == mGPUFBScene->IsWaiting() || nullptr == mShader || nullptr == mUberShader)
		return;

	ORShaderGPUCache *pGPUCache = (ORShaderGPUCache*) mShader; // pShader; // pInfo->GetFBShader();
	CGPUCacheModel *pCacheModel = pGPUCache->GetGPUCacheModelPtr();

	if (pCacheModel == nullptr)
		return;

	const ERenderGoal goal = options.GetGoal();
	const EShaderPass pass = options.GetPass();

	const bool isEarlyZ = (eRenderGoalSelectionId == goal && eRenderGoalShadows != goal);

	//
	//
	
	pCacheModel->RenderEnd(mGPUFBScene->GetCameraCache(), mUberShader);

	

	if (false == isEarlyZ)
	{
		InternalInstanceEnd();
	}
}