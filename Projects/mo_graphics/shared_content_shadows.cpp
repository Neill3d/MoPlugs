
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_content_shadows.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "shared_content.h"
#include "GPUCaching_shader.h"

#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "model_3d_shadow.h"
#include "algorithm\math3d_mobu.h"
#include "compositeMaster_objectLayers.h"

#include "mographics_common.h"

#define LIGHT_TEXTURE_MIPMAP_LEVELS		(8)

//////////////////////////////////////////////////////////////////////////////////////////
//

void CGPUFBScene::AddShadowNode(FBComponent *pNode)
{
	mShadowNodes.push_back(pNode);
}

void CGPUFBScene::RemoveShadowNode(FBComponent *pNode)
{
	auto iter = std::find(mShadowNodes.begin(), mShadowNodes.end(), pNode);
	if (iter != mShadowNodes.end() )
		mShadowNodes.erase(iter);
}

void CGPUFBScene::ClearShadowNodes()
{
	mShadowNodes.clear();
}

void CGPUFBScene::ShadowConstructor()
{
	mTextureShadowArray = 0;
	mTextureShColorArray = 0;
	mTextureTest = 0;

	mNumberOfShadows = 0;
	mBufferShadowRect = 0;
}

void CGPUFBScene::InitShadowTexture( const int texture_size, const int numberOfShadows, bool enableMS, int samples )
{
	
	if (mTextureShadowArray > 0)
	{
		glDeleteTextures(1, &mTextureShadowArray);
		mTextureShadowArray = 0;
	}
	
	if (mTextureShadowArray == 0)
		glGenTextures(1, &mTextureShadowArray);
	
	//const GLenum target = (numberOfShadows==1) ? GL_TEXTURE_2D : GL_TEXTURE_2D_ARRAY;

	mShadowCreationInfo.enableMS = enableMS;
	mShadowCreationInfo.samples = samples;

	GLenum target = (enableMS) ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;
	const int numberOfSamples = samples;

	const float BORDER_COLOR[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glBindTexture(target, mTextureShadowArray);

	if (enableMS)
	{
		glTexStorage3DMultisample(target, numberOfSamples, GL_DEPTH_COMPONENT32F, texture_size, texture_size, numberOfShadows, GL_FALSE );
	}
	else
	{
		glTexStorage3D(target, 1, GL_DEPTH_COMPONENT32F, texture_size, texture_size, numberOfShadows );
	}
	//glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	//glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, BORDER_COLOR);
	//glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0 );
	//glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0 );
	glBindTexture(target, 0);
	
	//
	//

	if (mTextureShColorArray > 0)
	{
		glDeleteTextures(1, &mTextureShColorArray);
		mTextureShColorArray = 0;
	}
	if (mTextureShColorArray == 0)
		glGenTextures(1, &mTextureShColorArray);

	glBindTexture(target, mTextureShColorArray);
	//glTexStorage3D(target, 1, GL_R32F, texture_size, texture_size, numberOfShadows );
	if (enableMS)
	{
		glTexStorage3DMultisample(target, numberOfSamples, GL_RGBA8, texture_size, texture_size, numberOfShadows, GL_FALSE );
	}
	else
	{
		glTexStorage3D(target, 1, GL_RGBA8, texture_size, texture_size, numberOfShadows );
	}
	//glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, BORDER_COLOR);
	//glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0 );
	//glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0 );
	glBindTexture(target, 0);
	
	CHECK_GL_ERROR_MOBU();
	/*
	if (mTextureTest > 0)
	{
		glDeleteTextures(1, &mTextureTest);
		mTextureTest = 0;
	}
	if (mTextureTest == 0)
		glGenTextures(1, &mTextureTest);

	FrameBuffer::CreateTexture2D(texture_size, texture_size, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, &mTextureTest );
	*/
}


void CGPUFBScene::FreeShadowResources()
{
	mFrameBufferShadow.reset(nullptr);

	if (mTextureShadowArray > 0)
	{
		glDeleteTextures( 1, &mTextureShadowArray );
		mTextureShadowArray = 0;
	}
	if (mTextureShColorArray > 0)
	{
		glDeleteTextures( 1, &mTextureShColorArray );
		mTextureShadowArray = 0;
	}
	if (mTextureTest > 0)
	{
		glDeleteTextures(1, &mTextureTest);
		mTextureTest = 0;
	}

	if (mBufferShadowRect > 0)
	{
		glDeleteBuffers(1, &mBufferShadowRect);
		mBufferShadowRect = 0;
	}

	mNumberOfShadows = 0;
}

void CGPUFBScene::BindShadowsTexture(const int location, const bool shadowCompare)
{

	GLenum target = (mShadowCreationInfo.enableMS) ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;

	//const GLenum target = GL_TEXTURE_2D_ARRAY;
	//if (mFrameBufferShadow.get() && mFrameBufferShadow->GetColorObject() > 0)
	if (mTextureShadowArray > 0)
	{
		glActiveTexture(GL_TEXTURE0+location);
		//glBindTexture(target, mTextureShadowArray);
		glBindTexture(target, mTextureShColorArray);
		/*
		if (shadowCompare)
		{
			//glBindTexture(target, mTextureShadowArray);
			glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		}
		else
		{
			//glBindTexture(target, mTextureShColorArray);
			glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		}
			*/
		//glBindTexture(target, mFrameBufferShadow->GetColorObject() );
		glActiveTexture(GL_TEXTURE0);
	}
}

void CGPUFBScene::UnBindShadowsTexture(const int location)
{
	GLenum target = (mShadowCreationInfo.enableMS) ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;

	glActiveTexture(GL_TEXTURE0+location);
	glBindTexture(target, 0);
	glActiveTexture(GL_TEXTURE0);
}

void CGPUFBScene::ShadowsOnContextChange()
{
	FreeShadowResources();

	//
	for (auto iter=begin(mShadowNodes); iter!=end(mShadowNodes); 
		++iter)
	{
		ModelShadowZone *pVolume = (ModelShadowZone*) *iter;
		pVolume->ResetUpdateValues();
	}
}

bool CGPUFBScene::RenderAllShadowMaps( const CRenderOptions &options, FBRenderOptions *pFBRenderOptions,
										const int maxShadowMapSize,
										const bool useGeomCache,
										const int numberOfSplits
										 )
{

	if ( 0 == mShadowNodes.size() )
		return false;

	if (nullptr == mUberShader.get() )
		return false;

	// TODO: shadow framebuffer with multisampling ?!

	if (nullptr == mFrameBufferShadow.get() )
	{
		mFrameBufferShadow.reset( new FrameBuffer(1, 1, 
			 FrameBuffer::eDeleteFramebufferOnCleanup, 0) );
	}

	//!
	FrameBufferInfo		lFrameBufferInfo;
	SaveFrameBuffer(&lFrameBufferInfo);

	// DONE: allocate a 2d texture array and bind to our framebuffer
	//mUberShader->BindShadow(shadowMapSize, lLightCount+1);

	// NOTE: lights + number of segments in cascaded light
	
	const int newNumberOfShadows = (int) mShadowNodes.size();
	const bool resized = mFrameBufferShadow->ReSize(maxShadowMapSize, maxShadowMapSize);

	if ( newNumberOfShadows != mNumberOfShadows || true == resized )
	{
		InitShadowTexture(maxShadowMapSize, newNumberOfShadows, 
			options.IsShadowMultisamplingEnabled(), options.GetShadowSamples());
		mNumberOfShadows = newNumberOfShadows;
	}

	// DONE: draw FBScene for shadows !
	// render each layer separately at the moment

	CRenderOptions lightOptions;
	lightOptions.SetOfflineRender( options.IsOfflineRender() );
	lightOptions.SetFrustumCulling(false);

	lightOptions.SetGoal( eRenderGoalShadows );
	lightOptions.SetPass( eShaderPassOpaque );

	mShadowLightData.resize( mShadowNodes.size() );
	auto lightDataIter=begin(mShadowLightData);

	int layerId = 0;

	if (true == mShadowCreationInfo.enableMS)
		glEnable(GL_MULTISAMPLE);


	FBTime localTime = FBSystem::TheOne().LocalTime;

	for (auto iter=begin(mShadowNodes); iter!=end(mShadowNodes); 
		++iter, ++lightDataIter, ++layerId)
	{
	
		ModelShadowZone *pZone = (ModelShadowZone*) *iter;

		if (false == pZone->Enabled || 0 == pZone->MasterLight.GetCount() )
			continue;

		// check if shadow layer (where zone is used) is off
		bool layerState = false;

		for (int i=0, count=pZone->GetDstCount(); i<count; ++i)
		{
			if (FBIS(pZone->GetDst(i), ObjectCompositionShadow) )
			{
				ObjectCompositionShadow *pShadowLayer = (ObjectCompositionShadow*) pZone->GetDst(i);

				double opacity=0.0;
				pShadowLayer->Opacity.GetData( &opacity, sizeof(double) );

				if (true == pShadowLayer->Active && opacity > 0.0)
				{
					layerState = true;
					break;
				}
			}
		}

		if (false == layerState)
			continue;

		pZone->ProcessUpdateConditions(options.IsOfflineRender(), localTime, options.GetUniqueFrameId() );

		if (false == pZone->HasUpdateFlag() )
			continue;

		pZone->ResetUpdateFlag();

		const int includeListCount = pZone->IncludeList.GetCount();
		const int excludeListCount = pZone->ExcludeList.GetCount();

		// DONE: assign models id's !
		// DONE: perhaps we could run a local frustum culling here

		lightOptions.SetFrustumCulling( false );

		if (includeListCount>0 || excludeListCount>0)
		{
			lightOptions.SetUniqueFrameId(1);
			lightOptions.SetFrustumCulling( true );
			MarkModelsWithIncludeExcludeLists(&pZone->IncludeList, &pZone->ExcludeList);
		}
		else if (true == pZone->AutoVolumeCulling)
		{
			FBVector3d pMin, pMax;
			FBMatrix tm;
			pZone->GetBoundingBox( pMin, pMax );
			pZone->GetMatrix(tm);

			for (int i=0; i<3; ++i)
			{
				pMin[i] *= 0.75;
				pMax[i] *= 0.75;
			}

			VectorTransform( pMin, tm, pMin );
			VectorTransform( pMax, tm, pMax );
			
			lightOptions.SetUniqueFrameId(1);
			lightOptions.SetFrustumCulling( true );
			MarkModelsInsideTheVolume( pMin, pMax );
		}

		GLenum target = (mShadowCreationInfo.enableMS) ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;
	
		mFrameBufferShadow->Bind();
		// we want to render into color buffer
		glDrawBuffer         ( GL_COLOR_ATTACHMENT0 );
	
		CHECK_GL_ERROR_MOBU();

		glEnable(GL_DEPTH_TEST);

		// Set the viewport to the proper size
		glViewport( 0, 0, maxShadowMapSize, maxShadowMapSize );

		mFrameBufferShadow->AttachTextureLayer(target, mTextureShColorArray, layerId, FrameBuffer::eAttachmentTypeColor0, false);
		mFrameBufferShadow->AttachTextureLayer(target, mTextureShadowArray, layerId, FrameBuffer::eAttachmentTypeDepth, false);

		if (false == mFrameBufferShadow->IsOk() )
		{
			continue;
		}

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		CHECK_GL_ERROR_MOBU();

		//  Render options
		// 1 - check if we should render geometry cache inside the volume
		// 2 - check which models we should render inside the volume (light frustum culling)
		// 3 - render models with cast shadow property on!

		FBLight *pLight = (pZone->MasterLight.GetCount() > 0) ? (FBLight*) pZone->MasterLight.GetAt(0) : nullptr;

		if (nullptr == pLight)
			continue;

		// replace camera view matrix with a light view matrix !

		lightDataIter->nearPlane = pZone->NearPlane;
		lightDataIter->farPlane = pZone->FarPlane;

		ComputeShadowLightMatrices( pLight, pZone, *lightDataIter );
		
		const mat4 &mp = lightDataIter->proj;
		const mat4 &mv = lightDataIter->modelView;

		mUberShader->UploadLightTransform( mp, mv, mat4_id ); 

		//
		// global interchange with shader plugins
		
		UpdateCameraCache( mv, mp, vec3(-mv.x, -mv.y, -mv.z) );

		// callbacks should support render to shadows !
		
		RenderingStats	&stats = pZone->GetStats();
		stats.Reset();

		const bool enableOffset = pZone->EnableOffset;
		const double offsetFactor = pZone->OffsetFactor;
		const double offsetUnits = pZone->OffsetUnits;

		if (true == enableOffset)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset( (GLfloat) offsetFactor, (GLfloat) offsetUnits);
		}

		RenderSceneShaderGroups(lightOptions, pFBRenderOptions, stats);

		if (true == enableOffset)
		{
			glDisable(GL_POLYGON_OFFSET_FILL);
		}

		if (true == pZone->RenderTransparency)
		{

			if (mShadowCreationInfo.enableMS)
			{
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			}
			else
			{
				glEnable( GL_BLEND );
				glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
			}

			lightOptions.SetPass( eShaderPassTransparency );
			RenderSceneShaderGroups(lightOptions, pFBRenderOptions, stats);

			if (mShadowCreationInfo.enableMS)
			{
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			}
			else
			{
				glDisable( GL_BLEND );
			}
		}

		//
		mFrameBufferShadow->UnBind();
	}

	if (mShadowCreationInfo.enableMS)
		glDisable(GL_MULTISAMPLE);

	CHECK_GL_ERROR_MOBU();

	//!
	RestoreFrameBuffer(&lFrameBufferInfo);

	mUberShader->UploadCameraUniforms( mCameraCache );

	return true;
}

void CGPUFBScene::PostRenderingShadows()
{
	
	// initialize shader if needed
	GetCompositeShaderPtr();
	
	if (mNumberOfShadows == 0 || mTextureShadowArray == 0 || mShaderComposite.get() == nullptr)
		return;


	FBCamera *pCamera = GetCamera();
	const int width = pCamera->CameraViewportWidth;
	const int height = pCamera->CameraViewportHeight;
	const float aspect = 1.0f * width / height;

	glm::mat4 projection = glm::ortho( 0.0f, (float) width, 0.0f, (float) height, -1.0f, 1.0f );

			
	mShaderComposite->SetMVP( glm::value_ptr(projection) );
	mShaderComposite->SetNumberOfShadows( mNumberOfShadows );
	mShaderComposite->SetTechnique( Graphics::eCompositeTechniqueShadowsDisplay );
	mShaderComposite->Bind();
	
	BindShadowsTexture(0, false);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, mTextureTest);
	/*
	unsigned char *pixels = new unsigned char[512*512*4];
	glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
	*/
	// draw textured rects with shadow maps

	if (mBufferShadowRect == 0)
	{
		glGenBuffers(1, &mBufferShadowRect);

		float uvs[8] = {0.0, 0.0, 
					0.0, 1.0,			
					1.0, 0.0,
					1.0, 1.0
						};

		glBindBuffer(GL_ARRAY_BUFFER, mBufferShadowRect);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, uvs, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, mBufferShadowRect);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	//
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, mNumberOfShadows);
	
	// finish

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
	UnBindShadowsTexture(0);
	mShaderComposite->UnBind();

	//
	//

	//mLightsManager->PostRenderingOverviewCam(width, height);
}



bool CGPUFBScene::ComputeShadowLightMatrices( FBLight *pLight, FBModel *pVolumeModel, ShadowLightDATA &data )
{
	if (nullptr == pLight)
		return false;

	bool status = false;
	if( pLight->LightType == kFBLightTypeSpot )
	{
		status = ComputeShadowSpotLightMatrices( pLight, pVolumeModel, data );
	}
	else if( pLight->LightType == kFBLightTypeInfinite )
	{
		status = ComputeShadowInfiniteLightMatrices( pLight, pVolumeModel, data );
	}
	return status;
}

bool CGPUFBScene::ComputeShadowSpotLightMatrices( FBLight *pLight, FBModel *pVolumeModel, ShadowLightDATA &data )
{
	data.proj.identity();
	data.modelView.identity();
	
	// Spotlight support

	// Get all the information necessary to setup the lighting matrix
	// Will need to create a MODELVIEWPROJ matrix using:
	//		- Transformation matrix of light
	//		- Custom projection matrix based on light

	// We need a base matrix because the transformation matrix doesn't take into account that lights
	// start out with transforms that are not represented ... grr ...

	double base[16]	=
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
	FBMatrix baseMat( base );

	// Factor out the scale, because we don't want it ...
	//FBVector3d v;
	
	FBVector3d scl;
	FBMatrix rotationMat;
	FBMatrix transformationMat;
	pLight->GetMatrix( rotationMat, kModelRotation, true );
	FBMatrixMult( transformationMat, rotationMat, baseMat );
	pLight->GetVector(scl, kModelScaling);
	
	//look_at( pLightView, lightData.position, lightData.dir, vec3(0.0f, 1.0f, 0.0f) );
	//invert(lightInvTM, pLightView);
	//pLight->GetVector( v );
	
	//transformationMat.Identity();
	transformationMat(3,0) = ((FBVector3d)pLight->Translation)[0];
	transformationMat(3,1) = ((FBVector3d)pLight->Translation)[1];
	transformationMat(3,2) = ((FBVector3d)pLight->Translation)[2];
	transformationMat(3,3) = 1.0f;

	//FBMatrixInverse( pLightView, transformationMat );
	//pLight->GetMatrix( pLightView, kModelTransformation );
	
	//pLight->GetMatrix( lightInvTM, kModelInverse_Transformation );
	
	for (int i=0; i<16; ++i)
		data.modelView.mat_array[i] = (float) transformationMat[i];

	invert( data.invModelView, data.modelView );

	double cosAngle;
	if( pLight->LightType == kFBLightTypeSpot )
		cosAngle = cos( (3.141592654*pLight->ConeAngle/180.0f)/2.0f);
	else
		cosAngle = 0.0;

	float spotAngle = (float) cosAngle;

	// Ok .. now we just need a projection matrix ...
	float fov = 1.2f * spotAngle / 2.0f;
	float fFar = data.farPlane; // lightData.radius * 2.0f; // scl[1] 
	float fNear = data.nearPlane;
	float top = tan(fov*3.14159f/360.0f) * fNear;
	float bottom = -top;
	float left = bottom;
	float right = top;
	double perspectiveValues[16] =
        {
            (2.0*fNear)/(right-left),   0,                          0,                          0,
            0,                         (2.0*fNear)/(top-bottom),    0,                          0,
            0,                         0,                           -(fFar+fNear)/(fFar-fNear), -(2.0f*fFar*fNear)/(fFar-fNear),
            0,                         0,                           -1.0f,                      0
        };

	
	perspective( data.proj, spotAngle, 1.0f, fNear, fFar );

	return true;
}

bool CGPUFBScene::ComputeShadowInfiniteLightMatrices( FBLight *pLight, FBModel *pVolumeModel, ShadowLightDATA &data )
{
	data.proj.identity();
	data.modelView.identity();

	// Directional light support

	// Get all the information necessary to setup the lighting matrix
	// Will need to create a MODELVIEWPROJ matrix using:
	//		- Transformation matrix of light
	//		- Custom projection matrix based on light

	// We need a base matrix because the transformation matrix doesn't take into account that lights
	// start out with transforms that are not represented ... grr ...
	double base[16]	=
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
	FBMatrix baseMat( base );
	
	// Factor out the scale, because we don't want it ...
	FBMatrix rotationMat;
	FBMatrix transformationMat;
	pLight->GetMatrix( rotationMat, kModelRotation, true );
	FBMatrixMult( transformationMat, rotationMat, baseMat );

	double radius;
	//FBVector4d newPos;

	// TODO: compute pos and radius from shadow volume center and size
	ModelShadowZone *pShadowZone = (ModelShadowZone*) pVolumeModel;
	double size = 0.01 * pShadowZone->Size;
	FBMatrix modelMatrix;
	
	FBVector3d sizeV, sclV, posV;

	pShadowZone->GetVector(posV);
	pShadowZone->GetVector(sclV, kModelScaling);

	sizeV = FBVector3d( size * sclV[0], size * sclV[1], size * sclV[2] );

	radius = sizeV[0];
	if (sizeV[1] > radius)
		radius = sizeV[1];
	if (sizeV[2] > radius)
		radius = sizeV[2];

	//GetWorldBounds( pLight, radius, newPos );

	transformationMat(3,0) = posV[0];
	transformationMat(3,1) = posV[1];
	transformationMat(3,2) = posV[2];
	transformationMat(3,3) = 1.0f;

	FBMatrix pLightView = transformationMat;
	FBMatrixInverse( pLightView, transformationMat );

	for (int i=0; i<16; ++i)
		data.modelView.mat_array[i] = (float) pLightView[i];

	invert( data.invModelView, data.modelView );

	//pLight->GetMatrix( lightInvTM, kModelInverse_Transformation );
	
	/*
	look_at( pLightView, -lightData.dir, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f) );
	invert( lightInvTM, pLightView );
	float radius = lightData.radius;
	*/

	// Ok .. now we just need a projection matrix ...
	float left		= -radius;
	float right	=  radius;
	float top		=  radius;
	float bottom	= -radius;

	float fNear	=  -radius;
	float fFar		=  radius;

	float diffRL	= 1.0f / (right - left);
	float diffTB	= 1.0f / (top - bottom);
	float diffFN	= 1.0f / (fFar - fNear);

	float orthoValues[16] =
        {
            2.0f * diffRL,	0.0f,			0.0f,                       0.0f,
            0.0f,           2.0f * diffTB,	0.0f,                       0.0f,
            0.0f,           0.0f,           -2.0f * diffFN,				0.0f,
            0.0f,           0.0f,           -(fFar + fNear) * diffFN,	1.0f
        };

	data.proj = orthoValues;

	return true;
}

const int CGPUFBScene::GetNumberOfShadows() const {
	return mNumberOfShadows;
}
void CGPUFBScene::GetShadowLightMatrix(const int index, mat4 &mvp)
{
	if (index < 0 || index >= mNumberOfShadows)
		return;

	ShadowLightDATA &data = mShadowLightData[index];
	mult(mvp, data.proj, data.modelView);
}
void CGPUFBScene::GetShadowLightMatrix( FBComponent *pComponent, mat4 &mvp )
{
	for (int i=0; i<mNumberOfShadows; ++i)
		if ( pComponent == mShadowNodes[i] )
		{
			GetShadowLightMatrix( i, mvp );
			break;
		}
}

void CGPUFBScene::GetShadowIdAndLightMatrix( FBComponent *pComponent, int &id, mat4 &mvp )
{
	id = 0;
	for (int i=0; i<mNumberOfShadows; ++i)
		if ( pComponent == mShadowNodes[i] )
		{
			id = i;
			GetShadowLightMatrix( i, mvp );
			break;
		}
}

const GLuint	CGPUFBScene::GetShadowTextureId() const {
	return mTextureShadowArray;
}