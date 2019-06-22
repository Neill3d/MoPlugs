
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shaderCallbacks_character.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "callbacks_binded.h"
#include "graphics\OGL_Utils_MOBU.h"

#include "MB_IBL_shader.h"
#include "MB_object_cubeMap.h"

/////////////////////////////////////////////////////////////////////////////////
// Character binded info

void CCharacterBindedInfo::EventBeforeRenderNotify()
{
	if (mNeedUpdateLightsList)
	{
		ORIBLShader *pCharShader = (ORIBLShader*) mShader;

		mGPUFBScene->PrepShaderLights( pCharShader->UseSceneLights, 
			&pCharShader->AffectingLights, mLightsPtr, mShaderLights.get() );
		mNeedUpdateLightsList = false;
	}
}

void CCharacterBindedInfo::EventConnNotify(HISender pSender, HKEvent pEvent)
{
	FBEventConnectionNotify lEvent(pEvent);

	FBConnectionAction pAction = lEvent.Action;
	FBPlug *pThis = lEvent.SrcPlug;
	//FBPlug *pPlug = lEvent.DstPlug;

	ORIBLShader *pCharShader = (ORIBLShader*) mShader;

	if (pThis == &pCharShader->AffectingLights)
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

/////////////////////////////////////////////////////////////////////////////
// Callback for IBL Shader

const bool CIBLBindedCallback::IsForShader(FBShader *pShader)
{
	return FBIS( pShader, ORIBLShader );
}

const bool CIBLBindedCallback::IsForShaderAndPass(FBShader *pShader, const EShaderPass pass)
{
	return FBIS( pShader,	ORIBLShader );
}

bool CIBLBindedCallback::OnInstanceBegin(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	if (nullptr == mShaderFX)
		return false;

	//mShader = pInfo->GetFBShader();
	mShader = pShader;
	if (nullptr == mShader || !FBIS(mShader, ORIBLShader) )
		return false;

	ORIBLShader *pCharShader = (ORIBLShader*) mShader;

	//const bool renderOnBack = pProjTex->RenderOnBack;
	const FBAlphaSource alphaSource = pCharShader->Transparency;

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

void CIBLBindedCallback::InternalInstanceBegin(const bool textureMapping, const GLuint muteTextureId)
{
	ORIBLShader *pProjTex = (ORIBLShader*) mShader;

	mLastModel = nullptr;
	mLastMaterial = nullptr;
	mLastFXShader = nullptr;
	hasExclusiveLights = false;

	EShadingType shadingType = pProjTex->ShadingType;

	// all about projections

	//mUberShader->UpdateDepthDisplacement( pProjTex->DepthDisplacement );
	
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

		if (pProjTex->DiffuseEnv.GetCount() > 0)
		{
			ObjectCubeMap	*pCubeMap = (ObjectCubeMap*) pProjTex->DiffuseEnv.GetAt(0);
			const int id = pCubeMap->CubeMapId;
			if (id > 0)
			{
				glActiveTexture(GL_TEXTURE10);
				glBindTexture(GL_TEXTURE_CUBE_MAP, id);
			}
		}
		if (pProjTex->SpecularEnv.GetCount() > 0)
		{
			ObjectCubeMap	*pCubeMap = (ObjectCubeMap*) pProjTex->SpecularEnv.GetAt(0);
			const int id = pCubeMap->CubeMapId;
			if (id > 0)
			{
				glActiveTexture(GL_TEXTURE11);
				glBindTexture(GL_TEXTURE_CUBE_MAP, id);
			}
		}
		if (pProjTex->BRDF.GetCount() > 0)
		{
			FBTexture	*pTexture = (FBTexture*) pProjTex->BRDF.GetAt(0);
			int id = pTexture->TextureOGLId;
			if (id == 0)
			{
				glActiveTexture(GL_TEXTURE0);
				pTexture->OGLInit();
				id = pTexture->TextureOGLId;
			}

			if (id > 0)
			{
				glActiveTexture(GL_TEXTURE12);
				glBindTexture(GL_TEXTURE_2D, id);
			}
		}

		glActiveTexture(GL_TEXTURE0);

		CHECK_GL_ERROR_MOBU();

		/*
		else if (shadingType == eShadingTypeMatCap && pProjTex->MatCap.GetCount() > 0)
		{
			GLuint matCapId = 0;

			FBTexture *pTexture = (FBTexture*) pProjTex->MatCap.GetAt(0);
			matCapId = pTexture->TextureOGLId;

			if (0 == matCapId)
			{
				pTexture->OGLInit();
				matCapId = pTexture->TextureOGLId;
			}

			mGPUFBScene->BindMatCapTexture(matCapId);
		}
		*/
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

void CIBLBindedCallback::OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	const ERenderGoal goal = options.GetGoal();

	if (eRenderGoalSelectionId != goal && eRenderGoalShadows != goal)
	{
		InternalInstanceEnd();
	}
}

void CIBLBindedCallback::InternalInstanceEnd()
{
	if (nullptr == mShaderFX)
		return;

	if (FBIS(mShader, ORIBLShader))
	{
//		ORIBLShader *projTex = (ORIBLShader*) mShader;

		if (hasExclusiveLights)
			mGPUFBScene->BindLights(false);

	}

	//FetchCullMode(mCullFaceInfo);	
}

// model sub-mesh has a material assigned
bool CIBLBindedCallback::OnMaterialBegin(const CRenderOptions &options, FBRenderOptions *pRenderOptions, FBMaterial *pMaterial, bool forceInit)
{
	// TODO: check for the last material, to not bind the same thing !
	const ERenderGoal goal = options.GetGoal();

	if (eRenderGoalSelectionId != goal && eRenderGoalShadows != goal && true == options.IsTextureMappingEnable() )
	{
		BindMaterialTextures(pMaterial, pRenderOptions, options.GetMutedTextureId(), forceInit );
	}
	return true;
}

void CIBLBindedCallback::OnMaterialEnd(const CRenderOptions &options, FBMaterial *pMaterial)
{
}

bool CIBLBindedCallback::OnModelDraw(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBModel *pModel, CBaseShaderInfo *pInfo)
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

void CIBLBindedCallback::InternalModelDraw(const CRenderOptions &options, FBModel *pModel, const bool textureMapping, const GLuint muteTextureId)
{
	EShadingType newShadingType = ( (ORIBLShader*) mShader)->ShadingType;
	
	const int numberOfShaders = pModel->Shaders.GetCount();
	for (int i=1; i<numberOfShaders; ++i)
	{
		FBShader *pShader = pModel->Shaders[i];
		
		if (false == pShader->Enable)
			continue;

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
void CIBLBindedCallback::PrepareInstance(const CRenderOptions &options, FBShader *pShader)
{
	mShader = pShader;
	if (nullptr == mShader || false == mShader->Enable )
		return;

//	const unsigned int uniqueFrameId = options.GetUniqueFrameId();

	if (FBIS(mShader, ORIBLShader) )
	{
		ORIBLShader *pProjTex = (ORIBLShader*) mShader;

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

void CIBLBindedCallback::PrepareModel(const CRenderOptions &options, FBModel *pModel, FBShader *pShader)
{
//	const unsigned int uniqueFrameId = options.GetUniqueFrameId();

	// TODO: prepare buffers to compute blendshape normals in real-time

	if ( FBIS(pShader, ORIBLShader) )
	{
		/*
		// store per-model user data with blendshape buffers
		PrepareModelBlendshapes(pModel, true);

		// run a compute program to calculate normals (if model has any blendshapes)
		RunComputeProgram(pModel);
		*/
	}
}



void CIBLBindedCallback::FrameSync(FBShader *pShader)
{

}

void CIBLBindedCallback::FrameEvaluation(FBShader *pShader)
{
	return;

	const int dstCount = pShader->GetDstCount();

	for (int i=0; i<dstCount; ++i)
	{
		if (FBIS(pShader->GetDst(i), FBModel) )
		{
			FBModel *pModel = (FBModel*) pShader->GetDst(i);

			FBGeometry *pGeometry = pModel->Geometry;
			FBModelVertexData *pData = pModel->ModelVertexData;

			if (nullptr == pGeometry || nullptr == pData)
				return;

//			const int geomUpdateId = pModel->GeometryUpdateId;
			//if (data.geometryUpdateId != geomUpdateId || data.mBufferBlendshapes == 0)
			//{

				const int vertexCount = pData->GetVertexCount();

				// TODO: use dublicate list to copy diff normals information into blendshape data

		

				if (pGeometry->ShapeGetCount() > 0)
				{
					// only in that case we should continue with buffers creation
					const int shapeCount = pGeometry->ShapeGetCount();

					// normals for all shapes
					//std::vector<vec4> diffNormals( vertexCount * shapeCount, vec4(0.0f, 0.0f, 0.0f, 0.0f) );

					for (int i=0; i<shapeCount; ++i)
					{
						const int diffCount = pGeometry->ShapeGetDiffPointCount(i);

//						const int startIndex = vertexCount * i;

						for (int j=0; j<diffCount; ++j)
						{
							int oriIndex;
							FBVertex pPosDiff;
							FBNormal pNormalDiff;

//							bool res = pGeometry->ShapeGetDiffPoint(i, j, oriIndex, pPosDiff);

							if (oriIndex >= 0 && oriIndex < vertexCount)
							{
								//diffNormals[startIndex + oriIndex] = vec4(pNormalDiff[0], pNormalDiff[1], pNormalDiff[2], 0.0f);
							}
						}
					}
				}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// Callback for Eye Shader

const bool CEyeBindedCallback::IsForShader(FBShader *pShader)
{
	return FBIS( pShader, ORCharacterEyeShader );
}

const bool CEyeBindedCallback::IsForShaderAndPass(FBShader *pShader, const EShaderPass pass)
{
	return FBIS( pShader,	ORCharacterEyeShader );
}


void CEyeBindedCallback::CheckForSamplerSlots()
{
	if (nullptr == mShaderFX)
		return;

	const auto locPtr = mShaderFX->GetCurrentEffectLocationsPtr()->fptr(); // (Graphics::eTechCharacterPass_Eye);

	if (eyeEnvReflSamplerSlot < 0)
	{
		eyeEnvReflSamplerSlot = locPtr->GetSamplerSlot( Graphics::eCustomLocationEyeEnvReflectionSampler );
	}
	if (eyeEnvDiffSamplerSlot < 0)
	{
		eyeEnvDiffSamplerSlot = locPtr->GetSamplerSlot( Graphics::eCustomLocationEyeEnvDiffuseSampler );
	}
	if (eyeEnvRefrSamplerSlot < 0)
	{
		eyeEnvRefrSamplerSlot = locPtr->GetSamplerSlot( Graphics::eCustomLocationEyeEnvRefractionSampler );
	}
}

void CEyeBindedCallback::InternalInstanceBegin(const bool textureMapping, const GLuint muteTextureId)
{
	CIBLBindedCallback::InternalInstanceBegin(textureMapping, muteTextureId);

	//
	//

	ORCharacterEyeShader *pMBShader = (ORCharacterEyeShader*) mShader;

	if (nullptr == pMBShader)
		return;

	//
	// assign uniforms and bind samplers
	
	CheckForSamplerSlots();

	if (eyeEnvReflSamplerSlot > 0 && pMBShader->EyeEnvReflection.GetCount() > 0)
	{
		FBTexture *pTexture = (FBTexture*) pMBShader->EyeEnvReflection.GetAt(0);
		int id = pTexture->TextureOGLId;
		if (id == 0)
		{
			glActiveTexture(GL_TEXTURE0);
			pTexture->OGLInit();
			id = pTexture->TextureOGLId;	
		}

		glActiveTexture(GL_TEXTURE0 + eyeEnvReflSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	if (eyeEnvDiffSamplerSlot > 0 && pMBShader->EyeEnvDiffuse.GetCount() > 0)
	{
		FBTexture *pTexture = (FBTexture*) pMBShader->EyeEnvDiffuse.GetAt(0);
		int id = pTexture->TextureOGLId;
		if (id == 0)
		{
			glActiveTexture(GL_TEXTURE0);
			pTexture->OGLInit();
			id = pTexture->TextureOGLId;	
		}

		glActiveTexture(GL_TEXTURE0 + eyeEnvDiffSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	if (eyeEnvRefrSamplerSlot > 0 && pMBShader->EyeEnvRefraction.GetCount() > 0)
	{
		FBTexture *pTexture = (FBTexture*) pMBShader->EyeEnvRefraction.GetAt(0);
		int id = pTexture->TextureOGLId;
		if (id == 0)
		{
			glActiveTexture(GL_TEXTURE0);
			pTexture->OGLInit();
			id = pTexture->TextureOGLId;	
		}

		glActiveTexture(GL_TEXTURE0 + eyeEnvRefrSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	//
	// uniforms

	const auto locVertexPtr = mShaderFX->GetCurrentEffectLocationsPtr()->vptr();
	const auto locPtr = mShaderFX->GetCurrentEffectLocationsPtr()->fptr();

	double irisSize, pupilSize;

	pMBShader->IrisSize.GetData( &irisSize, sizeof(double) );
	pMBShader->PupilSize.GetData( &pupilSize, sizeof(double) );

	locVertexPtr->SetUniform1f( Graphics::eCustomVertexLocationIrisSize, 0.01f * (float) irisSize );
	locVertexPtr->SetUniform1f( Graphics::eCustomVertexLocationCorneaBumpAmount, 0.1f );
	locVertexPtr->SetUniform1f( Graphics::eCustomVertexLocationCorneaBumpRadiusMult, 0.9f );

	locPtr->SetUniform1f( Graphics::eCustomLocationIrisSize, 0.01f * (float) irisSize );
	locPtr->SetUniform1f( Graphics::eCustomLocationPupilSize, 0.01f * (float) pupilSize );

	locPtr->SetUniform1f( Graphics::eCustomLocationIrisTexStart, 0.009f );
	locPtr->SetUniform1f( Graphics::eCustomLocationIrisTexEnd, 0.13f );
	locPtr->SetUniform1f( Graphics::eCustomLocationIrisEdgeFade, 0.04f );
	locPtr->SetUniform1f( Graphics::eCustomLocationIrisInsetDepth, 0.03f );
	locPtr->SetUniform1f( Graphics::eCustomLocationScleraTexOffset, 0.04f );
	locPtr->SetUniform1f( Graphics::eCustomLocationScleraTexScale, -0.14f );
	locPtr->SetUniform1f( Graphics::eCustomLocationIrisBorder, 0.0001f );
	locPtr->SetUniform1f( Graphics::eCustomLocationIrisTexEnd, 0.13f );
	locPtr->SetUniform1f( Graphics::eCustomLocationRefractEdgeSoftness, 0.1f );

	locPtr->SetUniform1f( Graphics::eCustomLocationIrisTextureCurvature, 0.51f );
	locPtr->SetUniform1f( Graphics::eCustomLocationArgIrisShadingCurvature, 0.51f );

	locPtr->SetUniform1f( Graphics::eCustomLocationTexUOffset, 0.25f );
	locPtr->SetUniform1f( Graphics::eCustomLocationCorneaDensity, 0.001f );
	locPtr->SetUniform1f( Graphics::eCustomLocationIrisNormalOffset, 0.001f );
	locPtr->SetUniform1f( Graphics::eCustomLocationIor, 1.3f );
	locPtr->SetUniform1f( Graphics::eCustomLocationBumpTexture, 0.3f );

	locPtr->SetUniform1i( Graphics::eCustomLocationCatShape, 0 );
	locPtr->SetUniform1f( Graphics::eCustomLocationCybShape, 0.0f );
	locPtr->SetUniform1i( Graphics::eCustomLocationColTexture, 1 );

	locPtr->SetUniform1i( Graphics::eCustomLocationEyeEnvReflectionSampler, 14 );
	locPtr->SetUniform1i( Graphics::eCustomLocationEyeEnvDiffuseSampler, 13 );
	locPtr->SetUniform1i( Graphics::eCustomLocationEyeEnvRefractionSampler, 15 );

	//
	//

	glActiveTexture(GL_TEXTURE0);
}