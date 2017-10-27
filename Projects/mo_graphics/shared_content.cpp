
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_content.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "shared_content.h"
#include "shared_camera.h"

#include "Shader.h"
#include "ProjTex_shader.h"

// STL
#include <algorithm>

#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "IO\FileUtils.h"

#include "algorithm\math3d_mobu.h"
#include "graphics\OGL_Utils.h"

#include "mographics_common.h"

#define UBERSHADER_EFFECT			"ProjectiveMapping.glslfx"
#define COMPOSITE_EFFECT			"CompositeMaster.glslfx"

//extern Graphics::ShaderEffect*		mpLightShader;

// global shader

//int							mpLightShaderRefCount = 0;

/////////////////////////////////////////////
// made a GPU copy of a scene data

/*
	FBCamera *lCamera = pCamera;
	FBMatrix pCamMatrix;
	lCamera->GetCameraMatrix( pCamMatrix, kFBModelView );
	FBMatrix lViewMatrix( pCamMatrix );
	
    FBRVector lViewRotation;
    FBMatrixToRotation(lViewRotation, lViewMatrix);

    FBMatrix lViewRotationMatrix;
    FBRotationToMatrix(lViewRotationMatrix, lViewRotation);
	*/




/////////////////////////////////////////////////////////////////////////////// ContentManager ////////////////////////////////////////
// 
CGPUFBScene::CGPUFBScene()
	: mGPUSceneLights( new CGPUShaderLights() )
	//, mFrameBufferBack( new FrameBuffer( 0, 0, FrameBuffer::eCreateColorTexture | FrameBuffer::eCreateDepthTexture | FrameBuffer::eDeleteFramebufferOnCleanup, 1 ) )
	, mUberShader(nullptr)
	, mShaderComposite(nullptr)
{

	mLocAllTheMeshes = -1;
	mLocAllTheModels = -1;

//	mLogarithmicDepthHint = false;
	mNeedEvaluate = true;
	mWait = false;
	mRenderToBuffer = false;
	
	mSettingsDepthLog = false;
	mSettingsDepthNV = false;

	mRealFarPlane = 400000.0f;

	mLastModelView.Identity();
	mLastTime = FBTime::Infinity;

	mCompositionLights = nullptr;

	//
	mCriticalSection.Init();

	//
	//

	mMaterialsInspector.SetTexturesInspector( &mTexturesInspector );

	mModelsInspector.SetTexturesInspector( &mTexturesInspector );
	mModelsInspector.SetMaterialsInspector( &mMaterialsInspector );
	mModelsInspector.SetShadersInspector( &mShadersInspector );

	mTexturesInspector.NeedFullUpdate();
	mShadersInspector.NeedFullUpdate();
	mMaterialsInspector.NeedFullUpdate();
	mModelsInspector.NeedFullUpdate();

	mLockSmallEvents = false;

	mLastLightsBinded = nullptr;

	ShadowConstructor();
}

CGPUFBScene::~CGPUFBScene()
{
	FreeShadowResources();

	//mFrameBufferBack.reset(nullptr);

	mUberShader.reset(nullptr);
	mShaderComposite.reset(nullptr);
}

////////////////////////////////////////////////////
// set settings

void CGPUFBScene::SetFarPlaneCritical(const float value)
{
	mCriticalSection.Enter();
	mRealFarPlane = value;
	mCriticalSection.Leave();
}

const float CGPUFBScene::GetFarPlane() const
{
	return mRealFarPlane;
}

FBCamera *CGPUFBScene::GetCamera() const
{
	if (mCamera.Ok() )
		return mCamera;
	else
		return nullptr;
}
CCameraInfoCache &CGPUFBScene::GetCameraCache()
{
	return mCameraCache;
}

void CGPUFBScene::ClearCache()
{
	mUberShader.reset(nullptr);

	mLastTime = FBTime::Infinity;
}

void CGPUFBScene::SetLogarithmicDepth(const bool value)
{
	if (mUberShader.get() )
		mUberShader->SetLogarithmicDepth(value);
}

void CGPUFBScene::LoadShader()
{
	if (mNeedCompositeShader && mShaderComposite.get() == nullptr)
	{
		// try to load

		FBString effectPath, effectFullName;

		Graphics::ShaderComposite	*pEffects = nullptr;

		if ( FindEffectLocation( FBString("\\GLSL_FX\\", COMPOSITE_EFFECT), effectPath, effectFullName ) )
		{
			pEffects =  new Graphics::ShaderComposite();
			if( !pEffects->Initialize( FBString(effectPath, "\\GLSL_FX\\"), COMPOSITE_EFFECT, 512, 512, 1.0) )
			{
				mNeedCompositeShader = false;
				pEffects = nullptr;
				//mUberShader.reset(nullptr);
				
				FBTrace( "[VR RENDERER]: Failed to initialize a Composite shader!\n" );
			}
		}
		else
		{
			FBTrace( "[VR RENDERER]: Failed to find a Composite shader!\n" );

			mNeedCompositeShader = false;
			pEffects = nullptr;
			//mUberShader.reset(nullptr);
		}

		if (pEffects != nullptr)
		{
			CHECK_GL_ERROR_MOBU();

			//mLastContext = wglGetCurrentContext();
			mShaderComposite.reset(pEffects);
		}
	}

	//
	if (mNeedUberShader && mUberShader.get() == nullptr)
	{
		// try to load

		FBString effectPath, effectFullName;

		Graphics::ShaderEffect	*pEffects = nullptr;

		if ( FindEffectLocation( FBString("\\GLSL_FX\\", UBERSHADER_EFFECT), effectPath, effectFullName ) )
		{
			pEffects =  new Graphics::ShaderEffect();
			if( !pEffects->Initialize( FBString(effectPath, "\\GLSL_FX\\"), UBERSHADER_EFFECT, 512, 512, 1.0) )
			{
				mNeedUberShader = false;
				pEffects = nullptr;
				//mUberShader.reset(nullptr);
				
				FBTrace( "[VR RENDERER]: Failed to initialize an Uber shader!\n" );
			}
		}
		else
		{
			FBTrace( "[VR RENDERER]: Failed to find an Uber shader!\n" );

			mNeedUberShader = false;
			pEffects = nullptr;
			//mUberShader.reset(nullptr);
		}

		if (pEffects != nullptr)
		{
			pEffects->SetBindless(false);
			pEffects->SetCubeMapRendering(false);
			//pEffects->SetLogarithmicDepth(false);
			//pEffects->SetDepthDisplacement( 0.0f );
			pEffects->SetTechnique( Graphics::eEffectTechniqueShading );
			pEffects->PrepCurrentTech();

			const auto loc = pEffects->GetCustomEffectShaderLocationsPtr();
			mLocTexture = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheTextures);
			mLocMaterial = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheMaterials);
			mLocShader = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheShaders);
			mLocProjectors = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheProjectors);

			CHECK_GL_ERROR_MOBU();

			//mLastContext = wglGetCurrentContext();
			mUberShader.reset(pEffects);
		}
	}
}


void CGPUFBScene::PrepareCamera(FBCamera *pCamera, const CTilingInfo &tilingInfo, const bool cubeMapSetup, int cubeMapFace, CubeMapRenderingData *const cubemap)
{
	// this matrix only for baking projection, fragment shader will use camera matrices
	/*
	FBCamera *pcamera = pRenderOptions->GetRenderingCamera();
	if (FBIS(pcamera, FBCameraSwitcher))
		pcamera = ( (FBCameraSwitcher*) pcamera)->CurrentCamera;
		*/

	mCamera = pCamera;

	if (pCamera != nullptr)
	{
		CCameraInfoCachePrep(pCamera, tilingInfo, mCameraCache);
	}
	if (cubeMapSetup)
	{
		mCameraCache.farPlane = cubemap->zmax;
		mCameraCache.nearPlane = cubemap->zmin;
		mCameraCache.width = cubemap->cubeMapSize;
		mCameraCache.height = cubemap->cubeMapSize;
		mCameraCache.pos = vec4(cubemap->position.x, cubemap->position.y, cubemap->position.z, 1.0f);

		perspective(mCameraCache.p4, 90.0f, 1.0f, mCameraCache.nearPlane, mCameraCache.farPlane);
		vec3 pos = mCameraCache.pos;
		//mCameraCache.mv4.get_translation(pos);
		
		mCameraCache.mv4.identity();
		switch(cubeMapFace)
		{
		//Negative X
		case 0: look_at(mCameraCache.mv4, pos, pos+vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) );
			break;
		//Positive X
		case 1: look_at(mCameraCache.mv4, pos, pos+vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) );
			break;
		//Positive Y
		case 2: look_at(mCameraCache.mv4, pos, pos+vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) );
			break;
		//Negative Y
		case 3: look_at(mCameraCache.mv4, pos, pos+vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f) );
			break;
		//Positive Z
		case 4: look_at(mCameraCache.mv4, pos, pos+vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f) );
			break;
		//Negative Z
		case 5: look_at(mCameraCache.mv4, pos, pos+vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f) );
			break;
		}
		
		/*
		mCameraCache.mv4.identity();
		switch(cubeMapFace)
		{
		//Negative X
		case 0: look_at(mCameraCache.mv4, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) );
			break;
		//Positive X
		case 1: look_at(mCameraCache.mv4, vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) );
			break;
		//Positive Y
		case 2: look_at(mCameraCache.mv4, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) );
			break;
		//Negative Y
		case 3: look_at(mCameraCache.mv4, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f) );
			break;
		//Positive Z
		case 4: look_at(mCameraCache.mv4, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f) );
			break;
		//Negative Z
		case 5: look_at(mCameraCache.mv4, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f) );
			break;
		}
		*/
		//mCameraCache.mv4.set_translation( pos );

		/*
		if (cubemap)
		{
			mCameraCache.mv4.identity();
			mCameraCache.mv4.set_translation( -mCameraCache.pos );
		}
		*/
		//mUberShader->UploadCubeMapUniforms(cubemap->zmin, cubemap->zmax, cubemap->position, cubemap->max, cubemap->min, cubemap->useParallax);
		//mUberShader->SetCubeMapRendering(true);
		/*
		mCameraCache.p4.identity();
		*/

		invert(mCameraCache.mvInv4, mCameraCache.mv4 );
		mCameraCache.mvInv4.set_translation( vec3(0.0f, 0.0f, 0.0f) );
		transpose(mCameraCache.mvInv4);
		
		//mCameraCache.width = cubemap->cubeMapSize;
		//mCameraCache.height = cubemap->cubeMapSize;
		for (int j=0; j<16; ++j)
			mCameraCache.mv[j] = (double) mCameraCache.mv4.mat_array[j];
	}
	/*
	else
	{
		//mUberShader->SetCubeMapRendering(false);
	}
	*/
	if (mUberShader.get() )
	{
		mUberShader->UploadCameraUniforms(mCameraCache);
	}

	//FBColorAndAlpha backcolor( pcamera->BackGroundColor );
	//glClearColor((float)backcolor[0], (float)backcolor[1], (float)backcolor[2], (float)backcolor[3]);

	// global interchange with shader plugins
	SetCameraCache(mCameraCache);

	CHECK_GL_ERROR_MOBU();
}

void CGPUFBScene::PushCameraCache()
{
	mPushCameraCacheInfo.pCamera = mCamera;
	mPushCameraCacheInfo.cameraCache = mCameraCache;
}

void CGPUFBScene::PopCameraCache()
{
	mCamera = mPushCameraCacheInfo.pCamera;
	mCameraCache = mPushCameraCacheInfo.cameraCache;

	// global interchange with shader plugins
	SetCameraCache(mCameraCache);

	if (mUberShader.get() )
	{
		mUberShader->UploadCameraUniforms(mCameraCache);
	}

	CHECK_GL_ERROR_MOBU();
}

void CGPUFBScene::PrepareBuffers(FBEvaluateInfo *pEvalInfo, bool cubeMapSetup)
{
	CHECK_GL_ERROR_MOBU();

	/*
	FBRenderer* lRenderer = mSystem.Renderer;
	int numberOfModels = lRenderer->DisplayableGeometryCount;

	for (int i=0; i<numberOfModels; ++i)
	{
		FBModel *pModel = lRenderer->GetDisplayableGeometry(i);

		if (true == IsModelSkippable(pModel, lRenderer, cubeMapSetup) )
			numberOfModels = numberOfModels - 1;
	}

	mDisplayableGeometry.resize(numberOfModels);
	if (numberOfModels > 0)
	{
		for (int i=0; i<numberOfModels; ++i)
		{
			FBModel *pModel = lRenderer->GetDisplayableGeometry(i);

			if (true == IsModelSkippable(pModel, lRenderer, cubeMapSetup) )
				continue;

			mDisplayableGeometry[i] = pModel;
		}

	}
	*/
	
	// update all materials
	/*
	if (mSystem.Scene->Textures.GetCount() > 0)
	{
		const int numberOfTextures = mSystem.Scene->Textures.GetCount();
		mTexturesVector.resize(numberOfTextures);

		int i=0;
		for (auto iter=begin(mTexturesVector); iter!=end(mTexturesVector); ++iter, ++i)
		{
			FBTexture *ptexture = (FBTexture*) mSystem.Scene->Textures.GetAt(i);
			mTexturesMap[ptexture] = i;

			iter->address = 0;
			iter->width = ptexture->Width;
			iter->height = ptexture->Height;
			iter->transform.identity();

			double *texmat = ptexture->GetMatrix();
			for (int j=0; j<16; j++)
			{
				iter->transform.mat_array[j] = (float) texmat[j];
			}
		}

		mBufferTexture.UpdateData( sizeof(TextureGLSL) * (int)mTexturesVector.size(), mTexturesVector.data() );
	}
	else
	{
		TextureGLSL	defTexData;
		defTexData.transform.identity();
		defTexData.address = 0;
		defTexData.width = 512;
		defTexData.height = 512;
		mBufferTexture.UpdateData( sizeof(TextureGLSL), &defTexData );
	}
	*/

	
	UnBindBuffers();
	
	// lights
	PrepFBSceneLights();

	//
	//

	bool updated;

	updated = mTexturesInspector.Process(pEvalInfo);

	if (updated || mTexturesInspector.GetNumberOfItems() != mBufferTexture.GetCount() )
	{
		mBufferTexture.UpdateData( sizeof(TextureGLSL), mTexturesInspector.GetNumberOfItems(), mTexturesInspector.GetData() );
	}

	//
	// SHADERS

	bool shadersUpdated = mShadersInspector.Process(pEvalInfo);

	/*
	if (mSystem.Scene->Shaders.GetCount() > 0)
	{
		const int numberOfShaders = mSystem.Scene->Shaders.GetCount();
		mShadersVector.resize(numberOfShaders);
		
		int i=0;
		for (auto iter=begin(mShadersVector); iter!=end(mShadersVector); ++iter, ++i)
		{
			FBShader *pShader = mSystem.Scene->Shaders[i];
			mShadersMap[pShader] = i;

			DefaultShader(*iter);
			ConstructFromFBShader( pShader, *iter);
			
			iter->shadingType = eShadingTypeFlat;
		}

		mBufferShader.UpdateData( sizeof(ShaderGLSL) * (int)mShadersVector.size(), mShadersVector.data() );
	}
	else
	{
		ShaderGLSL	defShader;
		DefaultShader( defShader );
		mBufferShader.UpdateData( sizeof(ShaderGLSL), &defShader );
	}
	*/
	//
	// MATERIALS

	/*
	if (mSystem.Scene->Materials.GetCount() > 0)
	{
		const int numberOfMaterials = mSystem.Scene->Materials.GetCount();
		mMaterialsVector.resize(numberOfMaterials);

		int i=0;
		for (auto iter=begin(mMaterialsVector); iter!=end(mMaterialsVector); ++iter, ++i)
		{
			FBMaterial *pMaterial = mSystem.Scene->Materials[i];
			mMaterialsDataMap[pMaterial] = i;

			DefaultMaterial(*iter);
			ConstructFromFBMaterial( pMaterial, *iter);
			
			
		}

		mBufferMaterial.UpdateData( sizeof(MaterialGLSL) * (int) mMaterialsVector.size(), mMaterialsVector.data() );
	}
	else
	{
		MaterialGLSL defMaterial;
		DefaultMaterial( defMaterial );

		mBufferMaterial.UpdateData( sizeof(MaterialGLSL), &defMaterial );
	}
	*/

	updated = mMaterialsInspector.Process(pEvalInfo);

	if (updated || mMaterialsInspector.GetNumberOfItems() != mBufferMaterial.GetCount() )
	{
		mBufferMaterial.UpdateData( sizeof(MaterialGLSL), mMaterialsInspector.GetNumberOfItems(), mMaterialsInspector.GetData() );
	}

	//
	// MESHES and MODELS

	// TODO: separate models transform update from the meshes data

	updated = mModelsInspector.Process(pEvalInfo);

	/*
	if (updated || mModelsInspector.GetNumberOfItems() != mBufferModel.GetCount() )
	{
		mBufferModel.UpdateData( sizeof(ModelGLSL), mModelsInspector.GetNumberOfItems(), mModelsInspector.GetData() );
	}
	*/
	if (mModelsInspector.IsSubDataUpdated() || mModelsInspector.GetNumberOfMeshItems() != mBufferMesh.GetCount() )
	{
		mBufferMesh.UpdateData( sizeof(MeshGLSL), mModelsInspector.GetNumberOfMeshItems(), mModelsInspector.GetMeshData() );
	}

	// update base shaders and combinations
	if (shadersUpdated || mBufferShader.GetCount() == 0 || mShadersInspector.IsCombinationUpdated() )
	{
		mShadersInspector.UpdateCombinations();
		
		// DONE: update shader combinations !!
		// upload on gpu
		mShadersInspector.UpdateGPUBuffer(&mBufferShader);
	}

	mBufferShader.MakeBufferResident();
	mBufferMaterial.MakeBufferResident();
	mBufferTexture.MakeBufferResident();
	mBufferMesh.MakeBufferResident();
	//mBufferModel.MakeBufferResident();
	CHECK_GL_ERROR_MOBU();

	// process shader info before passes rendering
	mShadersInspector.EventBeforeRenderNotify();


	/*
	int numberOfMeshes = 0;

	for(auto iter=begin(mDisplayableGeometry); iter!=end(mDisplayableGeometry); ++iter)
	{
		FBModel *pModel = *iter;
		FBModelVertexData *pData = pModel->ModelVertexData;
		const int regionCount = pData->GetSubRegionCount();

		numberOfMeshes += regionCount;
	}

	mMeshesVector.resize(numberOfMeshes);
	mModelsVector.resize(numberOfModels);

	int meshIndex = 0;
	int modelIndex = 0;
	
	FBEvaluateInfo *pEvalInfo = FBGetDisplayInfo();

	for(auto iter=begin(mDisplayableGeometry); iter!=end(mDisplayableGeometry); ++iter, ++modelIndex)
	{
		FBModel *pModel = *iter;
		
		FBMatrix m;
		pModel->GetMatrix(m, kModelTransformation, true, pEvalInfo);
		
		ModelGLSL &modeldata = mModelsVector[modelIndex];

		for (int j=0; j<16; ++j)
		{
			modeldata.transform.mat_array[j] = (float) m[j];
			//modeldata.normalMatrix.mat_array[j] = (float) m[j];
		}

		invert(modeldata.normalMatrix, modeldata.transform);
		transpose(modeldata.normalMatrix);

		int shaderIndex = 0;
		if (pModel->Shaders.GetCount() > 0)
		{
			shaderIndex = mShadersInspector.FindResourceIndex((FBShader*)pModel->Shaders.GetAt(0));
			if (shaderIndex == -1)
				shaderIndex = 0;
		}	
		
		assert( shaderIndex >= 0 );

		FBColor uniqueColor = pModel->UniqueColorId;

		FBModelVertexData *pData = pModel->ModelVertexData;
		const int regionCount = pData->GetSubRegionCount();
		for (int i=0; i<regionCount; ++i)
		{
			FBMaterial *pMaterial = pData->GetSubRegionMaterial(i);

			MeshGLSL &meshdata = mMeshesVector[meshIndex+i];

			meshdata.model = modelIndex;
			meshdata.shader = shaderIndex;
			meshdata.lightmap = 0;
			meshdata.color = vec4( (float)uniqueColor[0], (float)uniqueColor[1], (float)uniqueColor[2], 1.0 );

			meshdata.material = 0;
			if (pMaterial != nullptr && mMaterialsInspector.GetNumberOfItems() > 0)
				meshdata.material = mMaterialsInspector.FindResourceIndex(pMaterial);
		
			if (meshdata.material < 0)
				meshdata.material = 0;
		}

		meshIndex += regionCount;
	}

	if (numberOfMeshes > 0)
		mBufferMesh.UpdateData( sizeof(MeshGLSL) * numberOfMeshes, mMeshesVector.data() );
	if (numberOfModels > 0)
		mBufferModel.UpdateData( sizeof(ModelGLSL) * numberOfModels, mModelsVector.data() );
		*/
}

void CGPUFBScene::PrepareBuffersFromCamera()
{
	//mBufferModel.UnBind();
	mModelsInspector.UpdateNormalMatrices( mCameraCache.mv );
	mBufferModel.UpdateData( sizeof(ModelGLSL), (int) mModelsInspector.GetNumberOfItems(), mModelsInspector.GetData() );
	mBufferModel.MakeBufferResident();

	// lights in eye space
	if (nullptr != mCompositionLights)
		PrepLightsInViewSpace(mCompositionLights);
	else
		PrepLightsInViewSpace(mGPUSceneLights.get());

	MapLightsOnGPU();
}




bool CGPUFBScene::PrepRender()
{
	if (nullptr == mUberShader.get() )
		return false;
	//
	//PrepareCamera(pRenderOptions, cubeMapSetup, cubeMapFace, cubemap);

	mUberShader->PrepCurrentTech();

	const auto loc = mUberShader->GetCustomEffectShaderLocationsPtr();
	mLocTexture = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheTextures);
	mLocMaterial = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheMaterials);
	mLocShader = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheShaders);
	mLocProjectors = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheProjectors);

	mLocAllTheMeshes = loc->GetVertexLocation(Graphics::eCustomVertexLocationAllTheMeshes);
	mLocAllTheModels = loc->GetVertexLocation(Graphics::eCustomVertexLocationAllTheModels);

	//
	// render to view

	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*
	FBCamera *pCamera = pRenderOptions->GetRenderingCamera();
	if (FBIS(pCamera, FBCameraSwitcher))
		pCamera = ( (FBCameraSwitcher*) pCamera)->CurrentCamera;
		*/
	//if (cubeMapFace == 0)
	//{
	/*
	if (loc->dirLights >= 0 && loc->lights >= 0)
	{
		mGPUSceneLights->Bind( mUberShader->GetFragmentProgramId(), loc->dirLights, loc->lights );
	}
	*/
	// this could be used for every camera
	//mShaderLights->Prep(mCameraCache, nullptr);
	
	mUberShader->SetNumberOfProjectors(0);
	mUberShader->UploadModelTransform( mat4(array16_id) );
	
	FBColor ambientColor = FBGlobalLight::TheOne().AmbientColor;
	
	mUberShader->UploadLightingInformation( false,
			vec4((float)ambientColor[0], (float)ambientColor[1], (float)ambientColor[2], 0.0f), 
			0, 0);
	
	BindLights(true);
	//}

	mBufferTexture.BindAsUniform( mUberShader->GetFragmentProgramId(), mLocTexture, 0 );
	mBufferShader.BindAsUniform( mUberShader->GetFragmentProgramId(), mLocShader, 0 );
	mBufferMaterial.BindAsUniform( loc->GetFragmentId(), mLocMaterial, 0 );

	mBufferModel.BindAsUniform( mUberShader->GetVertexProgramId(), mLocAllTheModels, 0 );
	mBufferMesh.BindAsUniform( mUberShader->GetVertexProgramId(), mLocAllTheMeshes, 0 );

	return true;
}

bool CGPUFBScene::BindUberShader(bool overrideShading, const EShadingType overrideShadingType)
{
	if (mUberShader.get() == nullptr) 
		return false;

	//mUberShader->SetLogarithmicDepth( mSettings->DisplayDepth == eGraphicsDepthLog );
	
	mUberShader->Bind();

	
	if ( (true == mUberShader->IsWallTechnique()) 
		&& (false == mUberShader->IsEarlyZ()) )
	{
		// DONE: subroutines for blending modes
		// set subroutine values
		GLuint index[30];
		for (int i=0; i<30; ++i)
			index[i] = i;
	
		if (overrideShading)
		{
			const int indexOffset = 25;

			for (int i=0; i<eShadingTypeCount; ++i)
				index[indexOffset + i] = indexOffset + (int) overrideShadingType;
		}

		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, GLsizei(30), &index[0] );
	}

	CHECK_GL_ERROR_MOBU();
	return true;
}

void CGPUFBScene::UnBindUberShader()
{
	if (mUberShader.get() == nullptr) 
		return;

	mUberShader->UnBind();

	mLastLightsBinded = nullptr;
}



void CGPUFBScene::UploadModelInfo(FBModel *pModel, bool uploadAndBind)
{
	ModelGLSL		modeldata;

	FBMatrix m;
	pModel->GetMatrix(m);

	for (int i=0; i<16; ++i)
	{
		modeldata.transform.mat_array[i] = (float) m[i];
		modeldata.normalMatrix.mat_array[i] = (float) m[i];
	}

	mBufferModel.UpdateData( sizeof(ModelGLSL), 1, &modeldata );
	
	if (true == uploadAndBind && mLocAllTheModels >= 0)
		mBufferModel.BindAsUniform( mUberShader->GetVertexProgramId(), mLocAllTheModels, 0 );
}

void CGPUFBScene::UploadMeshInfo(const MeshGLSL &meshinfo, bool uploadAndBind)
{
	/*
	MeshGLSL	meshdata;
	
	meshdata.material = 0;
	meshdata.model = 0;
	meshdata.shader = 0;
	*/
	mBufferMesh.UpdateData( sizeof(MeshGLSL), 1, &meshinfo );

	if (true == uploadAndBind && mLocAllTheMeshes >= 0)
		mBufferMesh.BindAsUniform( mUberShader->GetVertexProgramId(), mLocAllTheMeshes, 0 );
}

void CGPUFBScene::UploadShader(FBShader *pShader)
{
	ShaderGLSL		shaderdata;
	DefaultShader(shaderdata);
	mBufferShader.UpdateData(sizeof(ShaderGLSL), 1, &shaderdata);
}

Graphics::ShaderEffect *CGPUFBScene::GetUberShaderPtr()
{	
	if (mUberShader.get() == nullptr)
	{
		mNeedUberShader = true;
	}
	
	return mUberShader.get();
}

Graphics::ShaderComposite *CGPUFBScene::GetCompositeShaderPtr()
{
	if (mShaderComposite.get() == nullptr)
	{
		mNeedCompositeShader = true;
	}

	return mShaderComposite.get();
}

void CGPUFBScene::ReloadUberShader()
{
	mUberShader.reset(nullptr);
	mNeedUberShader = true;
}

void CGPUFBScene::ReloadCompositeShader()
{
	mShaderComposite.reset(nullptr);
	mNeedCompositeShader = true;
}

void CGPUFBScene::AskToUpdateSceneData()
{
	mShadersInspector.AskToUpdateAllShadersData();

	mTexturesInspector.NeedFullUpdate();
	mShadersInspector.NeedFullUpdate();
	mMaterialsInspector.NeedFullUpdate();
	mModelsInspector.NeedFullUpdate();
}

void CGPUFBScene::ChangeContext(FBRenderOptions *pFBRenderOptions)
{
	// free framebuffer for live camera and cubemaps
	// free background framebuffer 

	//mFrameBufferBack.reset(nullptr);
	
	mUberShader.reset(nullptr);
	mShaderComposite.reset(nullptr);

	mShadersInspector.ChangeContext();
	
	ChangeContextForShaderGroups(pFBRenderOptions);

	mBufferMaterial.Free();
	mBufferMesh.Free();
	mBufferModel.Free();
	mBufferShader.Free();
	mBufferTexture.Free();

	ShadowsOnContextChange();
	CameraOnContextChange();
	CubeMapOnContextChange();

	AskToUpdateSceneData();
}
/*
void CGPUFBScene::PrepBackground(const bool useMS, const int samples, const int width, const int height)
{
	if (mCamera.Ok() == false )
		return;

	if (mFrameBufferBack.get() == nullptr)
		mFrameBufferBack.reset( new FrameBuffer(1, 1, FrameBuffer::eCreateColorTexture | FrameBuffer::eCreateDepthTexture | FrameBuffer::eDeleteFramebufferOnCleanup, 1) );

	mFrameBufferBack->ReSize(width, height);

	// do some rendering (MS or normal FBO)

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	if (false == mFrameBufferBack->Bind() )
		return;

	FBColor backColor;
	backColor = mCamera->BackGroundColor;
	if (mCamera->UseFrameColor)
		backColor = mCamera->FrameColor;

	glClearColor( (float)backColor[0], (float)backColor[1], (float)backColor[2], 0.0f );

	glViewport(0, 0, width, height);

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	FBVideo *pVideo = mCamera->BackGroundMedia;
	FBTexture *pTexture = mCamera->BackGroundTexture;

	if (pVideo || pTexture)
	{
		GLuint id = 0;
		int twidth = 1;
		int theight = 1;

		if (pTexture)
		{
			FBVideo *pTexVideo = pTexture->Video;

			if (pTexVideo && FBIS(pTexVideo, FBVideoMemory) )
				id =  ((FBVideoMemory*) pTexVideo)->TextureOGLId;
			else
			{
				pTexture->OGLInit();
				id = pTexture->TextureOGLId;
			}

			twidth = pTexture->Width;
			theight = pTexture->Height;
		}


		if (id == 0 && pVideo && FBIS(pVideo, FBVideoClip) )
		{
			FBVideoClip *pVideoClip = (FBVideoClip*) pVideo;
			id = pVideoClip->GetTextureID();
			twidth = pVideoClip->Width;
			theight = pVideoClip->Height;
		}

		// DONE: render background camera plane
		if (id > 0)
		{
			bool fit = mCamera->BackGroundImageFit;
//			bool crop = mCamera->BackGroundImageCrop;
			bool center = mCamera->BackGroundImageCenter;
			bool keepRatio = mCamera->BackGroundImageKeepRatio;

			int frameWidth = mFrameBufferBack->GetWidth();
			int frameHeight = mFrameBufferBack->GetHeight();

			int resWidth=1, resHeight=1;
			int x=0, y=0;
			double posX=0.0, posY=0.0;

			if (fit)
			{
				resWidth = frameWidth;
				resHeight = frameHeight;

				if (keepRatio)
					FitImageWithAspect(twidth, theight, frameWidth, frameHeight, x, y, resWidth, resHeight);
				
				if (center)
				{
					posX = x;
					posY = y;
				}
			}
			else
			{
				resWidth = twidth;
				resHeight = theight;

				if (center)
				{
					posX = (frameWidth - twidth) / 2;
					posY = (frameHeight - theight) / 2;
				}
			}

			

			glDepthMask(GL_FALSE);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, id );

			drawOrthoQuad2dOffset( posX, posY, resWidth, resHeight );

			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			glDepthMask(GL_TRUE);
		}
	}

	//
	// TODO: render some back objects here !
	//
	// render objects here or in a shader ?!
	mFrameBufferBack->UnBind();
	return;

	
}
*/
// TODO: DEPRICATED !!
const int CGPUFBScene::GetNumberOfBackgroundShadersToDraw() const
{
	return 0;
	/*
	int count = 0;
	EShaderPass pass = eShaderPassBackground;
	int intpass = (int) pass;

	auto &callbacks = mShadersInspector.GetShaderTypesVector();
	auto &fbshaders = mShadersInspector.GetResourceVector();

	// shader could be used by callback only once 
	
	for (size_t callbackNdx=0;
		callbackNdx < callbacks.size();
		++callbackNdx)
	{

		CBaseShaderCallback *pCallback = callbacks[callbackNdx];
		
		// check if callback supports current pass
		if ( 0 == (intpass & pCallback->CallbackPasses()) )
			continue;

		for (size_t i=0; i<fbshaders.size(); ++i)
		{
			FBShader *pShader = fbshaders[i];
			
			if (pShader == nullptr || pShader->Enable == false)
			{
				continue;
			}

			if (false == pCallback->IsForShaderAndPass(pShader, pass) )
				continue;

			if (pShader->GetDstCount() == 0)
				continue;

			count += 1;
		}
	}

	return count;
	*/
}

void CGPUFBScene::DrawBackground(const bool drawSimple, const bool clearBuffers, int regionWidth, int regionHeight)
{
	if (mCamera.Ok() == false)
		return;

	// TODO: check if we use matte shading somewhere in the scene
	const bool useMatte = true;

	if (drawSimple == true || useMatte == false)
	{
		FBColor backColor = mCamera->BackGroundColor;
		if (mCamera->UseFrameColor)
			backColor = mCamera->FrameColor;

		glClearColor( backColor[0], backColor[1], backColor[2], 0.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		//float fBackColor[4] = { backColor[0] , backColor[1], backColor[2], 1.0f };
		//float fBackColor2[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		//float fBackDepth = 0.0f;

		//glClearBufferfv( GL_COLOR, 0, fBackColor );
		//glClearBufferfv( GL_COLOR, 1, fBackColor2 );
		//glClearBufferfv( GL_DEPTH, 0, &fBackDepth );
		
		//glDepthMask(GL_FALSE);
		//drawOrthoGradientQuad2d(newWidth, newHeight);
		//glDepthMask(GL_TRUE);
	
		FBVideo *pVideo = mCamera->BackGroundMedia;
		FBTexture *pTexture = mCamera->BackGroundTexture;

		if (nullptr != pVideo || nullptr != pTexture)
		{
			GLuint id = 0;
			int twidth = 1;
			int theight = 1;

			if (pTexture)
			{
				FBVideo *pTexVideo = pTexture->Video;

				if (pTexVideo && FBIS(pTexVideo, FBVideoMemory) )
					id =  ((FBVideoMemory*) pTexVideo)->TextureOGLId;
				else
				{
					id = pTexture->TextureOGLId;
					if (0 == id)
					{
						pTexture->OGLInit();
						id = pTexture->TextureOGLId;
					}
				}

				twidth = pTexture->Width;
				theight = pTexture->Height;
			}


			if (id == 0 && pVideo && FBIS(pVideo, FBVideoClip) )
			{
				FBVideoClip *pVideoClip = (FBVideoClip*) pVideo;
				id = pVideoClip->GetTextureID();
				twidth = pVideoClip->Width;
				theight = pVideoClip->Height;
			}

			// DONE: render background camera plane
			if (id > 0)
			{
				bool fit = mCamera->BackGroundImageFit;
	//			bool crop = mCamera->BackGroundImageCrop;
				bool center = mCamera->BackGroundImageCenter;
				bool keepRatio = mCamera->BackGroundImageKeepRatio;

				int frameWidth = regionWidth; // mFrameBufferBack->GetWidth();
				int frameHeight = regionHeight; // mFrameBufferBack->GetHeight();

				int resWidth=1, resHeight=1;
				int x=0, y=0;
				double posX=0.0, posY=0.0;

				if (fit)
				{
					resWidth = frameWidth;
					resHeight = frameHeight;

					if (keepRatio)
						FitImageWithAspect(twidth, theight, frameWidth, frameHeight, x, y, resWidth, resHeight);
				
					if (center)
					{
						posX = x;
						posY = y;
					}
				}
				else
				{
					resWidth = twidth;
					resHeight = theight;

					if (center)
					{
						posX = (frameWidth - twidth) / 2;
						posY = (frameHeight - theight) / 2;
					}
				}

			

				glDepthMask(GL_FALSE);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, id );

				drawOrthoQuad2dOffset( posX, posY, resWidth, resHeight );

				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
				glDepthMask(GL_TRUE);
			}
		}
		
	}
	else
	{
		/*
		if (clearBuffers)
			glClear( GL_DEPTH_BUFFER_BIT ); // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glUseProgram(0);

		glDepthMask(GL_FALSE);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mFrameBufferBack->GetColorObject() );

		drawOrthoQuad2d( mFrameBufferBack->GetWidth(), mFrameBufferBack->GetHeight() );

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		glDepthMask(GL_TRUE);
		*/
	}
}


void CGPUFBScene::CCameraInfoCachePrep(FBCamera *pCamera, const CTilingInfo &tilingInfo, CCameraInfoCache &cache)
{
	FBMatrix mv, p, mvInv;

	pCamera->GetCameraMatrix( mv, kFBModelView );
	pCamera->GetCameraMatrix( p, kFBProjection );
	
	if (tilingInfo.tilingMatrix != nullptr)
	{
		FBMatrix tilem(tilingInfo.tilingMatrix);
		FBMatrixMult( p, tilem, p );
	}

	FBMatrixInverse(mvInv, mv);

	for (int i=0; i<16; ++i)
	{
		cache.mv4.mat_array[i] = (nv_scalar) mv[i];
		cache.mvInv4.mat_array[i] = (nv_scalar) mvInv[i];
		cache.p4.mat_array[i] = (nv_scalar) p[i];

		cache.mv[i] = mv[i];
	}

	FBVector3d v;
	pCamera->GetVector(v);
	for (int i=0; i<3; ++i)
		cache.pos[i] = (float) v[i];
	//cache.pos = vec4( &cache.mv4.x );

	cache.fov = pCamera->FieldOfView;
	cache.width = pCamera->CameraViewportWidth;
	cache.height = pCamera->CameraViewportHeight;
	cache.nearPlane = pCamera->NearPlaneDistance;
	cache.farPlane = pCamera->FarPlaneDistance;

	if (tilingInfo.tilingMatrix != nullptr)
	{
		cache.width = tilingInfo.width;
		cache.height = tilingInfo.height;
		cache.offsetX = tilingInfo.offsetX;
		cache.offsetY = tilingInfo.offsetY;
	}
}

const int CGPUFBScene::FindMeshIndex( FBModel *pModel, FBMaterial *pMaterial, FBShader *pShader )
{

	const int modelIndex = mModelsInspector.FindResourceIndex(pModel);
	const int matIndex = mMaterialsInspector.FindResourceIndex(pMaterial);
	const int shaderIndex = mShadersInspector.FindResourceIndex(pShader);

	if (modelIndex < 0 || matIndex < 0 || shaderIndex < 0)
		return -1;

	const int modelCount = (int) mModelsInspector.GetNumberOfItems();
	const int meshCount = (int) mModelsInspector.GetNumberOfMeshItems();

	int meshIndex = mModelsInspector.GetModelMeshIndex(modelIndex);
	int meshIndexStop = meshIndex;

	if (modelIndex == modelCount-1)
		meshIndexStop = meshCount;
	else
		meshIndexStop = mModelsInspector.GetModelMeshIndex(modelIndex+1);

	const MeshGLSL *allMeshes = mModelsInspector.GetMeshData();
	for (int i=meshIndex; i<meshIndexStop; ++i)
	{
		const MeshGLSL *thisMesh = allMeshes + i;

		// TODO: take care, cause shader index could be a result of compound from several shaders !!
		if (thisMesh->model == modelIndex && thisMesh->shader == shaderIndex && thisMesh->material == matIndex)
			return i;
	}

	return meshIndex;
	//return -1;
}

bool CGPUFBScene::BindProjectors( const CProjectors *pProjectors, const GLuint muteTextureId )
{
	if (mUberShader.get() == nullptr || pProjectors == nullptr)
		return false;

	const int count = pProjectors->GetNumberOfProjectors();
	mUberShader->UpdateNumberOfProjectors( count );
	
	if ( count > 0 )
	{
		GLint loc = mUberShader->GetCustomEffectShaderLocationsPtr()->GetFragmentLocation(Graphics::eCustomLocationAllTheProjectors);
		pProjectors->Bind( mUberShader->GetFragmentProgramId(), loc, muteTextureId );

		return true;
	}

	return false;
}

void CGPUFBScene::BindRimTexture(const GLuint id)
{
	glActiveTexture(GL_TEXTURE16);
	glBindTexture(GL_TEXTURE_2D, id);
	glActiveTexture(GL_TEXTURE0);
}

void CGPUFBScene::BindMatCapTexture(const GLuint id)
{
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, id);
	glActiveTexture(GL_TEXTURE0);
}
/*
void CGPUFBScene::BindBackgroundTexture()
{
	if (mFrameBufferBack.get() )
	{
		glActiveTexture(GL_TEXTURE7);
		glBindTexture( GL_TEXTURE_2D, mFrameBufferBack->GetColorObject() );
	}
}
*/
void CGPUFBScene::MarkModelsWithIncludeExcludeLists(FBPropertyListObject *pIncludeList, FBPropertyListObject *pExcludeList)
{
	int includeListCount = pIncludeList->GetCount();
	int excludeListCount = pExcludeList->GetCount();

	if (includeListCount > 0)
	{
		auto &models = mModelsInspector.GetResourceVector();
		auto &updateVector = mModelsInspector.GetUpdateVector();

		auto modelIter = begin(models);
		auto updateIter = begin(updateVector);

		for ( ; modelIter!=end(models); ++modelIter, ++updateIter)
		{
			if ( *updateIter & RESOURCE_DELETED )
				continue;

			FBModel *pModel = *modelIter;
			FBProperty *pProp = pModel->PropertyList.Find("id");
			if (nullptr == pProp)
			{
				pProp = pModel->PropertyCreate( "id", kFBPT_int, "int", false, false );
				pProp->ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
				pProp->ModifyPropertyFlag( kFBPropertyFlagNotSavable, true );
			}
			pProp->SetInt(0);	
		}

		for (int i=0; i<includeListCount; ++i)
		{
			FBModel *pModel = (FBModel*) pIncludeList->GetAt(i);

			FBProperty *pProp = pModel->PropertyList.Find("id");
			if (nullptr == pProp)
			{
				pProp = pModel->PropertyCreate( "id", kFBPT_int, "int", false, false );
				pProp->ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
				pProp->ModifyPropertyFlag( kFBPropertyFlagNotSavable, true );
			}
			pProp->SetInt(1);
		}
	}
	else
	if (excludeListCount > 0)
	{
		auto &models = mModelsInspector.GetResourceVector();
		auto &updateVector = mModelsInspector.GetUpdateVector();

		auto modelIter = begin(models);
		auto updateIter = begin(updateVector);

		for ( ; modelIter!=end(models); ++modelIter, ++updateIter)
		{
			if ( *updateIter & RESOURCE_DELETED )
				continue;

			FBModel *pModel = *modelIter;
			FBProperty *pProp = pModel->PropertyList.Find("id");
			if (nullptr == pProp)
			{
				pProp = pModel->PropertyCreate( "id", kFBPT_int, "int", false, false );
				pProp->ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
				pProp->ModifyPropertyFlag( kFBPropertyFlagNotSavable, true );
			}
			pProp->SetInt(1);	
		}

		for (int i=0; i<excludeListCount; ++i)
		{
			FBModel *pModel = (FBModel*) pExcludeList->GetAt(i);

			FBProperty *pProp = pModel->PropertyList.Find("id");
			if (nullptr == pProp)
			{
				pProp = pModel->PropertyCreate( "id", kFBPT_int, "int", false, false );
				pProp->ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
				pProp->ModifyPropertyFlag( kFBPropertyFlagNotSavable, true );
			}
			pProp->SetInt(0);
		}
	}
}


void CGPUFBScene::MarkModelsInsideTheVolume(const FBVector3d &vmin, const FBVector3d &vmax)
{
	
	FBVector3d vcenter;
	VectorCenter( vmin, vmax, vcenter );
	double vR = VectorDistance( vmin, vcenter );

	auto &models = mModelsInspector.GetResourceVector();
	auto &updateVector = mModelsInspector.GetUpdateVector();

	auto modelIter = begin(models);
	auto updateIter = begin(updateVector);

	for ( ; modelIter!=end(models); ++modelIter, ++updateIter)
	{
		if (*updateIter & RESOURCE_DELETED)
			continue;

		FBModel *pModel = *modelIter;

		//
		FBVector3d locMin, locMax;
		FBMatrix tm;

		pModel->GetBoundingBox(locMin, locMax);
		pModel->GetMatrix(tm);

		VectorTransform( locMin, tm, locMin );
		VectorTransform( locMax, tm, locMax );

		FBVector3d locCenter;
		VectorCenter( locMin, locMax, locCenter );
		double locR = VectorDistance( locMin, locCenter );

		int vis = 1;
		if ( locR + VectorDistance( vcenter, locCenter ) > vR )
			vis = 0;

		//

		FBProperty *pProp = pModel->PropertyList.Find("id");
		if (nullptr == pProp)
		{
			pProp = pModel->PropertyCreate( "id", kFBPT_int, "int", false, false );
			pProp->ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
			pProp->ModifyPropertyFlag( kFBPropertyFlagNotSavable, true );
		}
		pProp->SetInt( vis );	
	}


}