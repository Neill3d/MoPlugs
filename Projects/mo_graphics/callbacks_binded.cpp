
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

#include "callbacks_binded.h"
#include "graphics\OGL_Utils_MOBU.h"

#include "ProjTex_shader.h"

///////////////////////////////////////////////////////////////////////////
// CShaderBindedCallback

const bool CUberShaderCallback::IsForShader(FBShader *pShader)
{
	return true;
}

const bool CUberShaderCallback::IsForShaderAndPass(FBShader *pShader, const EShaderPass pass)
{
	return true; // (pass == eShaderPassOpaque); // FBIS( pShader,	ProjTexShader );
}

bool CUberShaderCallback::OnTypeBegin(const CRenderOptions &options, bool useMRT)
{
	mShaderFX = mGPUFBScene->GetShaderFXPtr(mCurrentTech);

	if (nullptr == mShaderFX)
		return false;

	ERenderGoal goal = options.GetGoal();

	switch(goal)
	{
	case eRenderGoalSelectionId:
	case eRenderGoalShadows:
		mIsEarlyZ = true;
		mRenderToNormalAndMaskOutput = false;
		break;
	default:
		mIsEarlyZ = false;
		mRenderToNormalAndMaskOutput = true;
	}

	// DONE: we should do this only when draw to buffer is used !!
	// proj tex outputs to normal and mask attachments, not only color
	/*
	if ( useMRT && mRenderToNormalAndMaskOutput ) // pScene->IsRenderToBuffer()
	{
		GLenum buffers [] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers( 4, buffers );
	}
	else
	{
		GLenum buffers [] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers( 1, buffers );
	}
	*/
	CHECK_GL_ERROR_MOBU();
	mShaderFX->ModifyShaderFlags( Graphics::eShaderFlag_Bindless, mIsBindless );
	//mUberShader->SetTechnique( mCurrentTech );
	//mUberShader->SetBindless(mIsBindless);
	//mUberShader->SetNumberOfProjectors( 0 );
	//mUberShader->SetRimParameters( 0.0, 0.0, false, vec3(0.0f, 0.0f, 0.0f) );
	//mUberShader->SetDepthDisplacement( 0.0f );
	
	mShaderFX->SetTextureOffset( vec4( 0.0, 0.0, 0.0, 0.0) );
	mShaderFX->SetTextureScaling( vec4( 1.0, 1.0, 1.0, 1.0) );

	//
	/*
	if (options.IsCubeMapRender() && options.GetCubeMapData() )
	{
		const CubeMapRenderingData *cubemap = options.GetCubeMapData();
		
		mUberShader->UploadCubeMapUniforms(	cubemap->zmin, 
											cubemap->zmax, 
											cubemap->position, 
											cubemap->max, 
											cubemap->min, 
											cubemap->useParallax);
		mUberShader->SetCubeMapRendering(true);

		// TODO: 
		
		// mUberShader->UploadCameraUniforms(cameraMV, cameraPRJ, viewIT, vEyePos, width, height, farPlane, nullptr);
	}
	else
	{
		mUberShader->SetCubeMapRendering(false);
	}
	*/

	mShaderFX->ModifyShaderFlags( Graphics::eShaderFlag_NoTextures, false == options.IsTextureMappingEnable() );
	mShaderFX->ModifyShaderFlags( Graphics::eShaderFlag_EarlyZ, mIsEarlyZ );
	
	//
	OnTypeBeginPredefine( options, useMRT );

	//
	mGPUFBScene->PrepRender();
	mGPUFBScene->BindUberShader( mNeedOverrideShading ); // || options.IsCubeMapRender() );

	CGPUVertexData::renderPrep();

	StoreCullMode(mCullFaceInfo);
	return true;
}

void CUberShaderCallback::OnTypeEnd(const CRenderOptions &options)
{
	if (nullptr == mShaderFX)
		return;
	/*
	// proj tex outputs to normal and mask attachments, not only color
	if ( mRenderToNormalAndMaskOutput ) // pScene->IsRenderToBuffer()
	{
		GLenum buffers [] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers( 1, buffers );
	}
	*/
	mShaderFX->UnsetTextures();
	mGPUFBScene->UnBindUberShader();

	// unbind matCap and shadows

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	CGPUVertexData::renderFinish();

	FetchCullMode(mCullFaceInfo);
}

bool CUberShaderCallback::OnInstanceBegin(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	// TODO: bind projectors and instance lights list

	mLastModel = nullptr;
	mLastMaterial = nullptr;
	mLastFXShader = nullptr;

	//StoreCullMode(mCullFaceInfo);

	return (options.GetPass() == eShaderPassOpaque);
}

void CUberShaderCallback::OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	//FetchCullMode(mCullFaceInfo);
}

// model sub-mesh has a material assigned
bool CUberShaderCallback::OnMaterialBegin(const CRenderOptions &options, FBRenderOptions *pRenderOptions, FBMaterial *pMaterial, bool forceInit)
{
	// TODO: check for the last material, to not bind the same thing !
	const ERenderGoal goal = options.GetGoal();

	if (eRenderGoalSelectionId != goal && eRenderGoalShadows != goal && true == options.IsTextureMappingEnable() )
	{
		BindMaterialTextures(pMaterial, pRenderOptions, options.GetMutedTextureId(), forceInit );
	}
	CHECK_GL_ERROR_MOBU();
	return true;
}

void CUberShaderCallback::OnMaterialEnd(const CRenderOptions &options, FBMaterial *pMaterial)
{
}

bool CUberShaderCallback::OnModelDraw(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBModel *pModel, CBaseShaderInfo *pInfo)
{
	
	// support for cullmode
	// TODO: check for the latest cullmode to not set the same thing !

	FBModelCullingMode cullMode = pModel->GetCullingMode();

	if (cullMode != options.GetLastCullingMode() )
	{
		switch(cullMode)
		{
		case kFBCullingOff:
			glDisable(GL_CULL_FACE);
			break;
		case kFBCullingOnCW:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;
		case kFBCullingOnCCW:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;
		}

		// mLastCullingMode = cullMode;
		((CRenderOptions&)options).SetLastCullingMode(cullMode);
	}

	return true;
}

void CUberShaderCallback::BindMaterialTextures(FBMaterial *pMaterial, FBRenderOptions *pRenderOptions, const GLuint mutedTextureId, bool forceInit)
{
	if (nullptr == pMaterial)
		return;

	// DONE: bind material textures
	const int nId = GetTextureId( pMaterial->GetTexture(kFBMaterialTextureNormalMap), pRenderOptions, forceInit );
	const int rId = GetTextureId( pMaterial->GetTexture(kFBMaterialTextureReflection), pRenderOptions, forceInit );
	const int sId = GetTextureId( pMaterial->GetTexture(kFBMaterialTextureSpecular), pRenderOptions, forceInit );
	const int tId = GetTextureId( pMaterial->GetTexture(kFBMaterialTextureTransparent), pRenderOptions, forceInit );
	const int dId = GetTextureId( pMaterial->GetTexture(), pRenderOptions, forceInit );
	
	if (nId > 0 && nId != mutedTextureId)
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, nId);
	}
	if (rId > 0 && rId != mutedTextureId)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, rId);

		// use specified cubemap uniforms for parallax correction when computing reflections in shader
		//if (rId > 0)
		//{
			mGPUFBScene->UploadCubeMapUniforms(rId);
		//}
	}
	if (sId > 0 && sId != mutedTextureId)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, sId);
	}
	if (tId > 0 && tId != mutedTextureId)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tId);
	}
	if (dId > 0 && dId != mutedTextureId)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, dId);
	}
}


/////////////////////////////////////////////////////////////////////////////
// Callback for ProjTex Shader

const bool CProjTexBindedCallback::IsForShader(FBShader *pShader)
{
	return (FBIS( pShader, ProjTexShader )
		|| FBIS(pShader, FXProjectionMapping) );
}

const bool CProjTexBindedCallback::IsForShaderAndPass(FBShader *pShader, const EShaderPass pass)
{
	return (FBIS( pShader,	ProjTexShader )
		|| FBIS(pShader, FXProjectionMapping ) );

	/*
	if (FBIS( pShader,	ProjTexShader ) )
	{
		ProjTexShader *pProjTex = (ProjTexShader*) pShader;

		const bool isRenderOnBack = (true == pProjTex->RenderOnBack);
		const bool isTransparent = (pProjTex->Transparency != kFBAlphaSourceNoAlpha && pProjTex->TransparencyFactor < 1.0);

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

bool CProjTexBindedCallback::OnInstanceBegin(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	if (nullptr == mShaderFX)
		return false;

	//mShader = pInfo->GetFBShader();
	mShader = pShader;
	if (nullptr == mShader || !FBIS(mShader, ProjTexShader) )
		return false;

	ProjTexShader *pProjTex = (ProjTexShader*) mShader;

	//const bool renderOnBack = pProjTex->RenderOnBack;
	const FBAlphaSource alphaSource = pProjTex->Transparency;

	const ERenderGoal goal = options.GetGoal();
	const EShaderPass pass = options.GetPass();

	bool skipShader = false;

	switch(goal)
	{
	case eRenderGoalShading:
		switch(pass)
		{
		case eShaderPassOpaque:
			skipShader = (kFBAlphaSourceNoAlpha != alphaSource);
			break;
		case eShaderPassTransparency:
			skipShader = (kFBAlphaSourceAccurateAlpha != alphaSource);
			break;
		case eShaderPassAdditive:
			skipShader = (kFBAlphaSourceAdditiveAlpha != alphaSource);
			break;
		}
		break;
		/*
		if (true == renderOnBack)
		{
			skipShader = true;
		}
		else
		{
			switch(pass)
			{
			case eShaderPassOpaque:
				skipShader = (kFBAlphaSourceNoAlpha != alphaSource);
				break;
			case eShaderPassTransparency:
				skipShader = (kFBAlphaSourceAccurateAlpha != alphaSource);
				break;
			case eShaderPassAdditive:
				skipShader = (kFBAlphaSourceAdditiveAlpha != alphaSource);
				break;
			}
		}
		break;
		
	case eRenderGoalBackground:
		skipShader = (false == renderOnBack);
		break;
		*/
	}

	if (true == skipShader)
		return false;

	// TODO: bind projectors and instance lights list
	if (eRenderGoalSelectionId != goal && eRenderGoalShadows != goal)
	{
		InternalInstanceBegin( options.IsTextureMappingEnable(), options.GetMutedTextureId() );
	}

	return true;
}

void CProjTexBindedCallback::InternalInstanceBegin(const bool textureMapping, const GLuint muteTextureId)
{
	ProjTexShader *pProjTex = (ProjTexShader*) mShader;

	mLastModel = nullptr;
	mLastMaterial = nullptr;
	mLastFXShader = nullptr;
	hasExclusiveLights = false;

	EShadingType shadingType = pProjTex->ShadingType;

	// all about projections

	//mUberShader->UpdateDepthDisplacement( pProjTex->DepthDisplacement );
	/*
	double useRim, rimPower;
	FBColor rimColor;

	pProjTex->UseRim.GetData( &useRim, sizeof(double) );
	pProjTex->RimColor.GetData( rimColor, sizeof(FBColor) );
	pProjTex->RimPower.GetData( &rimPower, sizeof(double) );

	mUberShader->UpdateRimParameters( 0.01 * (float)useRim, 0.01 * (float)rimPower, 
		(pProjTex->RimTexture.GetCount()>0), 
		vec4( (float)rimColor[0], (float)rimColor[1], (float)rimColor[2], (float) pProjTex->RimBlend.AsInt() ) );
		*/
	
	if (eShadingTypeMatte == shadingType)
	{
		//mGPUFBScene->BindBackgroundTexture();
	}
	else
	{
		// rim lighting
		if (pProjTex->RimTexture.GetCount() > 0)
		{
			GLuint rimId = 0;

			FBTexture *pTexture = (FBTexture*) pProjTex->RimTexture.GetAt(0);
			//pTexture->OGLInit();
			rimId = pTexture->TextureOGLId;

			if (0 == rimId)
			{
				glActiveTexture(GL_TEXTURE0);
				pTexture->OGLInit();
				rimId = pTexture->TextureOGLId;
			}

			mGPUFBScene->BindRimTexture(rimId);
		}

		// lighting or MatCap baked light
		if ( (false == pProjTex->UseSceneLights && pProjTex->AffectingLights.GetCount() > 0)
			&& (eShadingTypeDynamic == shadingType || eShadingTypeToon == shadingType) )
		{
			mGPUFBScene->BindLights( false, pProjTex->GetShaderLightsPtr() );
			hasExclusiveLights = true;
		}
		else if (shadingType == eShadingTypeMatCap && pProjTex->MatCap.GetCount() > 0)
		{
			GLuint matCapId = 0;

			FBTexture *pTexture = (FBTexture*) pProjTex->MatCap.GetAt(0);
			matCapId = pTexture->TextureOGLId;

			if (0 == matCapId)
			{
				glActiveTexture(GL_TEXTURE0);
				pTexture->OGLInit();
				matCapId = pTexture->TextureOGLId;
			}

			mGPUFBScene->BindMatCapTexture(matCapId);
		}
	}
	

	// Bind Projective Data was here !
	//
	hasProjectors = (true == textureMapping && (shadingType != eShadingTypeMatte) );
	mFXProjectionBinded = false;

	if (hasProjectors)
		hasProjectors = mGPUFBScene->BindProjectors( pProjTex->GetProjectorsPtr(), muteTextureId );

	if (pProjTex->UsePolygonOffset)
	{
		double factor = pProjTex->PolygonOffsetFactor;
		double units = pProjTex->PolygonOffsetUnits;

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( (float) factor, (float) units );
	}

	// support for composite masks
	/*
	vec4 mask;
	mask.x = (pProjTex->CompositeMaskA) ? 1.0f : 0.0f;
	mask.y = (pProjTex->CompositeMaskB) ? 1.0f : 0.0f;
	mask.z = (pProjTex->CompositeMaskC) ? 1.0f : 0.0f;
	mask.w = (pProjTex->CompositeMaskD) ? 1.0f : 0.0f;
	
	mUberShader->UpdateShaderMask( mask );
	*/
}

void CProjTexBindedCallback::OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	const ERenderGoal goal = options.GetGoal();

	if (eRenderGoalSelectionId != goal && eRenderGoalShadows != goal)
	{
		InternalInstanceEnd();
	}
}

void CProjTexBindedCallback::InternalInstanceEnd()
{
	if (nullptr == mShaderFX)
		return;

	if (FBIS(mShader, ProjTexShader))
	{
		ProjTexShader *projTex = (ProjTexShader*) mShader;

		if (hasProjectors)
			projTex->mProjectors.UnBind();

		if (hasExclusiveLights)
			mGPUFBScene->BindLights(false);

		if (projTex->UsePolygonOffset)
		{
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
	}

	//FetchCullMode(mCullFaceInfo);	
}

// model sub-mesh has a material assigned
bool CProjTexBindedCallback::OnMaterialBegin(const CRenderOptions &options, FBRenderOptions *pRenderOptions, FBMaterial *pMaterial, bool forceInit)
{
	// TODO: check for the last material, to not bind the same thing !
	const ERenderGoal goal = options.GetGoal();

	if (eRenderGoalSelectionId != goal && eRenderGoalShadows != goal && true == options.IsTextureMappingEnable() )
	{
		BindMaterialTextures(pMaterial, pRenderOptions, options.GetMutedTextureId(), forceInit );
	}
	return true;
}

void CProjTexBindedCallback::OnMaterialEnd(const CRenderOptions &options, FBMaterial *pMaterial)
{
}

bool CProjTexBindedCallback::OnModelDraw(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBModel *pModel, CBaseShaderInfo *pInfo)
{
	if (nullptr == mShaderFX)
		return false;

	// TODO: bind model sprite sheet property values !
	// TODO: bind model cull mode

	const ERenderGoal goal = options.GetGoal();

	if (eRenderGoalSelectionId != goal && eRenderGoalShadows != goal)
	{
		InternalModelDraw(options, pModel, options.IsTextureMappingEnable(), options.GetMutedTextureId() );

		// support for sprite sheets

		FBPropertyAnimatable *pAnimProp = (FBPropertyAnimatable*) pModel->PropertyList.Find( "SpriteSheet Translation" );
		if (pAnimProp)
		{
			FBVector3d	v;
			pAnimProp->GetData(v, sizeof(double)*3);

			mShaderFX->UpdateTextureOffset( vec4( (float)v[0], (float)v[1], (float)v[2], 0.0) );

			pAnimProp = (FBPropertyAnimatable*) pModel->PropertyList.Find( "SpriteSheet Scaling" );
			if (pAnimProp)
			{
				pAnimProp->GetData(v, sizeof(double)*3);
				mShaderFX->UpdateTextureScaling( vec4( (float)v[0], (float)v[1], (float)v[2], 0.0) );
			}
		}
		else
		{
			mShaderFX->UpdateTextureOffset( vec4( 0.0, 0.0, 0.0, 0.0) );
			mShaderFX->UpdateTextureScaling( vec4( 1.0, 1.0, 1.0, 1.0) );
		}
	}

	return true;
}

void CProjTexBindedCallback::InternalModelDraw(const CRenderOptions &options, FBModel *pModel, const bool textureMapping, const GLuint muteTextureId)
{
	EShadingType newShadingType = ( (ProjTexShader*) mShader)->ShadingType;
	bool binded = false;
	const CProjectors *newProjectors = nullptr;

	const int numberOfShaders = pModel->Shaders.GetCount();
	for (int i=1; i<numberOfShaders; ++i)
	{
		FBShader *pShader = pModel->Shaders[i];
		
		if (false == pShader->Enable)
			continue;

		if ( FBIS(pShader, FXProjectionMapping) )
		{
			//( (ProjTexShader*) mShader)->mProjectors.UnBind();
			//UnBindProjectionMapping();
			
			mLastFXShader = (FXProjectionMapping*) pShader;
			
			newProjectors = mLastFXShader->GetProjectorsPtr();

			if (newProjectors->GetNumberOfProjectors() > 0)
			{
				binded = true;
				mFXProjectionBinded = true;
			}
			else
			{
				mLastFXShader = nullptr;
				newProjectors = nullptr;
				mFXProjectionBinded = false;
			}
			//!!! Support only one projections override
			//break;
		}
		else if ( FBIS(pShader, FXShadingShader) )
		{
			newShadingType = ( (FXShadingShader*) pShader)->ShadingType;
		}
	}

	if (true == textureMapping)
	{
		// don't bind the same texture, check for changes
		if (eShadingTypeMatte == newShadingType)
		{
			//mGPUFBScene->BindBackgroundTexture();
		}
		

		if (binded && nullptr != newProjectors)
		{
			mGPUFBScene->BindProjectors( newProjectors, muteTextureId );
		}
		else if (!binded && mFXProjectionBinded)
		{
			if (mLastFXShader)
			{
				//mLastFXShader->mProjectors.UnBind();
				mLastFXShader = nullptr;
				mFXProjectionBinded = false;

				hasProjectors = mGPUFBScene->BindProjectors( ( (ProjTexShader*) mShader)->GetProjectorsPtr(), muteTextureId );
			}
		}
	}

	// support for cullmode
	// TODO: check for the latest cullmode to not set the same thing !

	const FBModelCullingMode cullMode = pModel->GetCullingMode();

	if (cullMode != options.GetLastCullingMode())
	{
		switch(cullMode)
		{
		case kFBCullingOff:
			glDisable(GL_CULL_FACE);
			break;
		case kFBCullingOnCW:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			break;
		case kFBCullingOnCCW:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			break;
		}
		//mLastCullingMode = cullMode;
		((CRenderOptions&)options).SetLastCullingMode(cullMode);
	}
}

// executed one time per rendering frame (to setup gpu buffers, update data, etc.)
void CProjTexBindedCallback::PrepareInstance(const CRenderOptions &options, FBShader *pShader)
{
	mShader = pShader;
	if (nullptr == mShader || false == mShader->Enable )
		return;

	const unsigned int uniqueFrameId = options.GetUniqueFrameId();

	if (FBIS(mShader, ProjTexShader) )
	{
		ProjTexShader *pProjTex = (ProjTexShader*) mShader;

		// TODO: update only on changes, NOT EACH FRAME !!!
		// update only once, could be assigned to many models !

		pProjTex->EventBeforeRenderNotify();

		/*
		if ( true == options.IsTextureMappingEnable() 
			&& true == pProjTex->DynamicUpdate 
			&& uniqueFrameId > pProjTex->mLastUniqueFrameId )
		{
			pProjTex->mProjectors.Prep( pProjTex );
			pProjTex->mLastUniqueFrameId = uniqueFrameId;
			//DoManualUpdate();
		}
		*/
	}
	else if (FBIS(pShader, FXProjectionMapping) )
	{
		mLastFXShader = (FXProjectionMapping*) pShader;
		mLastFXShader->EventBeforeRenderNotify();
	}
	
}

void CProjTexBindedCallback::PrepareModel(const CRenderOptions &options, FBModel *pModel, FBShader *pShader)
{
	const unsigned int uniqueFrameId = options.GetUniqueFrameId();

	// check if we should update FX shader here as well
	// TODO: update only once (could be assigned to many of models)
	/*
	const int numberOfShaders = pModel->Shaders.GetCount();
	
	for (int i=0; i<numberOfShaders; ++i)
	{
		FBShader *pShader = pModel->Shaders[i];
		if ( pShader->Enable && FBIS(pShader, FXProjectionMapping) )
		{
			mLastFXShader = (FXProjectionMapping*) pShader;

			if (uniqueFrameId > mLastFXShader->mLastUniqueFrameId)
			{
				mLastFXShader->mProjectors.Prep(mLastFXShader);
				mLastFXShader->mLastUniqueFrameId = uniqueFrameId;
			}
		}
	}
	*/
	/*
	FBModelVertexData *pData = pModel->ModelVertexData;
	if (nullptr != pData)
	{
		if (false == pData->IsDrawable() )
		{
			pData->VertexArrayMappingRelease();
			//pData->EnableOGLVertexData();
			//pData->DisableOGLVertexData();
		}
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////////
// ProjTex binded info

void CProjTexBindedInfo::EventBeforeRenderNotify()
{
	if (mNeedUpdateLightsList)
	{
		ProjTexShader *pProjTex = (ProjTexShader*) mShader;

		mGPUFBScene->PrepShaderLights( pProjTex->UseSceneLights, 
			&pProjTex->AffectingLights, mLightsPtr, mShaderLights.get() );
		mNeedUpdateLightsList = false;
	}
}

void CProjTexBindedInfo::EventConnNotify(HISender pSender, HKEvent pEvent)
{
	FBEventConnectionNotify lEvent(pEvent);

	FBConnectionAction pAction = lEvent.Action;
	FBPlug *pThis = lEvent.SrcPlug;
	//FBPlug *pPlug = lEvent.DstPlug;

	ProjTexShader *pProjTex = (ProjTexShader*) mShader;

	if (pThis == &pProjTex->AffectingLights)
	{
		if (pAction == kFBConnectedSrc)
		{
			//pProjTex->ConnectSrc(pPlug);
			UpdateLightsList();
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			//pProjTex->DisconnectSrc(pPlug);
			UpdateLightsList();
		}
	}
}