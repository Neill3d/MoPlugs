
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_content_rendering.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "shared_content.h"
#include "model_cubemap_rendering.h"
#include "ProjTex_shader.h"
#include "GPUCaching_shader.h"
#include "IO\FileUtils.h"
#include <algorithm>
#include "graphics\OGL_Utils_MOBU.h"

#include "callbacks.h"
#include "callback_factory.h"
#include "MB_renderer.h"

#include "GPUCaching_model_display.h"
#include "glm/gtc/type_ptr.hpp"

#include "MB_IBL_shader.h"
#include "callbacks_binded.h"

//
/*

	TODO:

	1) pre-cache include and exclude list if used (to not enumerate all models each render cycle)

*/

extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);

/////////////////////////////////////////////////////////////////////////////////

void CGPUFBScene::AddCubeMapNode(FBComponent *pReflectionNode)
{
	mCubeMapNodes.push_back(pReflectionNode);
}
void CGPUFBScene::RemoveCubeMapNode(FBComponent *pReflectionNode)
{
	auto iter = std::find( mCubeMapNodes.begin(), mCubeMapNodes.end(), pReflectionNode );
	if (iter != mCubeMapNodes.end() )
		mCubeMapNodes.erase(iter);
}
void CGPUFBScene::ClearCubeMapNodes()
{
	mCubeMapNodes.clear();
}


bool CGPUFBScene::RenderAllCubeMaps(FBRenderOptions *pFBRenderOptions, const bool videoRendering, unsigned int frameId, const int maxCubeMapSize)
{
	if (mCubeMapNodes.size() == 0)
		return false;

	FBTime localTime = mSystem.LocalTime;

	CubeMapRendering *pCubeMap = nullptr;

	for (auto iter=mCubeMapNodes.begin(); iter!=mCubeMapNodes.end(); ++iter)
	{
		
		pCubeMap = (CubeMapRendering*) (*iter);
		
		// disabled or hidden from group
		if (false == pCubeMap->Enabled || false == pCubeMap->Show)
			continue;

		// check if we have hidden include list
		if (false == pCubeMap->CheckForHiddenIncludeList() )
			continue;

		if (true == pCubeMap->CommonCheckForUpdate(true) )
			pCubeMap->SetUpdateFlag(true);

		pCubeMap->ProcessUpdateConditions(videoRendering, localTime, frameId);

		// we have started with the first face, so let's continue with others
		if (pCubeMap->GetLastProcessedFace() > 0)
			pCubeMap->SetUpdateFlag(true);

		if ( false == pCubeMap->HasUpdateFlag() )
			continue;

		RenderingStats &stats = pCubeMap->GetStats();
		stats.Reset();
		pCubeMap->ResetUpdateFlag();

		CubeMapRenderingData	&data = pCubeMap->GetData();
		data.maxCubeMapSize = maxCubeMapSize;

		//FBModel *pLocation = (FBModel*) pCubeMap->Point.GetAt(0);

		FBVector3d v, scl, pBoxMin, pBoxMax;
		FBMatrix	worldToLocal;

		pCubeMap->GetVector(v);
		pCubeMap->GetVector(scl, kModelScaling);
		pCubeMap->GetBoundingBox( pBoxMin, pBoxMax );
		
		pBoxMin[0] *= scl[0];
		pBoxMin[1] *= scl[1];
		pBoxMin[2] *= scl[2];
		pBoxMax[0] *= scl[0];
		pBoxMax[1] *= scl[1];
		pBoxMax[2] *= scl[2];

		pCubeMap->GetMatrix( worldToLocal, kModelInverse_Transformation );
		//worldToLocal[12] = 0.0;
		//worldToLocal[13] = 0.0;
		//worldToLocal[14] = 0.0;

		float *ptr = glm::value_ptr(data.worldToLocal);
		for (int i=0; i<16; ++i, ++ptr)
			*ptr = (float) worldToLocal[i];

		data.position = glm::vec3( (float)v[0], (float)v[1], (float)v[2] );
		data.pComponent = pCubeMap;

		data.useParallax = (pCubeMap->UseParallaxCorrection) ? 1.0f : 0.0f;
		data.useStaticCubeMap = pCubeMap->UseFile;

		FBVector3d vMax, vMin;
		vMax = pBoxMax; // pCubeMap->RoomMax;
		vMin = pBoxMin; // pCubeMap->RoomMin;

		//data.max = data.position + glm::vec3( (float)vMax[0], vMax[1], vMax[2] );
		//data.min = data.position + glm::vec3( (float)vMin[0], vMin[1], vMin[2] );

		data.max = glm::vec3( (float)vMax[0], (float)vMax[1], (float)vMax[2] );
		data.min = glm::vec3( (float)vMin[0], (float)vMin[1], (float)vMin[2] );

		data.zmax = pCubeMap->FarRadius;
		data.zmin = pCubeMap->NearRadius;

		// DONE: collect parameters
		GLuint cubemapId = data.cubeMapId;
		bool needUpdatePanorama = data.needUpdateOutput;

		if (data.useStaticCubeMap == false)
		{
			int cubeMapSize = pCubeMap->Resolution;
			if (cubeMapSize <= 0)
				cubeMapSize = 512;
			if (cubeMapSize > data.maxCubeMapSize)
				cubeMapSize = data.maxCubeMapSize;

			pCubeMap->PrepOutputTextureObject(cubeMapSize, cubeMapSize);


			data.renderGeomCache = pCubeMap->UseGPUCache;

			RenderCubeMap(mFrameBufferCubeMap, pFBRenderOptions, data, mSettingsDepthLog, videoRendering, stats);
			needUpdatePanorama = true;
			
			if (mFrameBufferCubeMap.get() )
				cubemapId = mFrameBufferCubeMap->GetColorObject();
		}
		else
		{
			//data.panoWidth = data.staticCubeMapSize * 4;
			//data.panoHeight = data.staticCubeMapSize * 2;

			pCubeMap->PrepOutputTextureObject(data.staticCubeMapSize * 4, data.staticCubeMapSize * 2);
		}

		if (cubemapId > 0 && data.saveDynamicCubeMap)
		{
			pCubeMap->DoSave(cubemapId);

			data.saveDynamicCubeMap = false;
		}

		//
		if (pCubeMap->GenerateOutput && needUpdatePanorama)
		{
			// now unroll cubemap to pano
			if (RenderPano(data, cubemapId, pCubeMap->GenerateMipMaps) )
			{
				// output texture
				//if (mFrameBufferPano.get() )
				//	pCubeMap->AssignResult( mFrameBufferPano->GetColorObject(), mFrameBufferPano->GetWidth(), mFrameBufferPano->GetHeight() );
			}
			else
			{
				//pCubeMap->AssignResult( 0, 0, 0 );
			}

			data.needUpdateOutput = false;
		}
	}

	return true;
}

void CGPUFBScene::PrepCubeMapMatrices(const int faceId, const float nearPlane, const float farPlane, 
	const vec3 &pos, mat4 &proj, mat4 &mdl, mat4 &inv)
{
	perspective(proj, 90.0f, 1.0f, nearPlane, farPlane);
		
	mdl.identity();
	switch(faceId)
	{
	//Negative X
	case 0: look_at(mdl, pos, pos+vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) );
		break;
	//Positive X
	case 1: look_at(mdl, pos, pos+vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) );
		break;
	//Positive Y
	case 2: look_at(mdl, pos, pos+vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) );
		break;
	//Negative Y
	case 3: look_at(mdl, pos, pos+vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f) );
		break;
	//Positive Z
	case 4: look_at(mdl, pos, pos+vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f) );
		break;
	//Negative Z
	case 5: look_at(mdl, pos, pos+vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f) );
		break;
	}

	invert(inv, mdl );
	inv.set_translation( vec3(0.0f, 0.0f, 0.0f) );
	transpose(inv);
}

void CGPUFBScene::RenderCubeMap(std::auto_ptr<FrameBuffer> &framebuffer, FBRenderOptions *pFBRenderOptions, CubeMapRenderingData &data, const bool logDepth, bool videoRendering, RenderingStats &stats)
{
	if (mUberShader.get() == nullptr)
		return;

	if (framebuffer.get() == nullptr)
	{
		/*
		framebuffer.reset( new FrameBuffer(1, 1, 
			FrameBuffer::eCreateCubemap | FrameBuffer::eCreateColorTexture | FrameBuffer::eCreateDepthTexture | FrameBuffer::eDeleteFramebufferOnCleanup, 1) );
			*/
		framebuffer.reset( new FrameBuffer(1, 1, 
			FrameBuffer::eDeleteFramebufferOnCleanup, 1) );
	}

	// DONE: resize cubemap texture !
	framebuffer->ReSize(data.cubeMapSize, data.cubeMapSize);
	//mFrameBufferCubeMap->bindCubeMap(data.cubeMapId);

	// do some rendering (MS or normal FBO)

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	
	FBColor backColor;
	if (mCamera.Ok() )
	{
		backColor = mCamera->BackGroundColor;
		if (mCamera->UseFrameColor)
			backColor = mCamera->FrameColor;
	}
	glClearColor( (float)backColor[0], (float)backColor[1], (float)backColor[2], 0.0f );
	glViewport(0, 0, data.cubeMapSize, data.cubeMapSize);

	
	//
	//
	// render objects here or in a shader ?!
	CubeMapRendering *pCubeMap = (CubeMapRendering*) data.pComponent;
	int includeListCount = pCubeMap->IncludeList.GetCount();
	int excludeListCount = pCubeMap->ExcludeList.GetCount();

	CRenderOptions cameraRenderOptions = GetLastRenderOptions();

	cameraRenderOptions.SetGoal( eRenderGoalShading );
	cameraRenderOptions.SetPass( eShaderPassOpaque );
	cameraRenderOptions.SetCamera( mCamera );
	cameraRenderOptions.SetCubeMapRender( true, &data );
	
	cameraRenderOptions.SetUniqueFrameId(1);
	cameraRenderOptions.SetFrustumCulling( (includeListCount>0 || excludeListCount>0) );
	MarkModelsWithIncludeExcludeLists(&pCubeMap->IncludeList, &pCubeMap->ExcludeList);

	// DONE: render in 6 steps - each cubemap face separately
	framebuffer->Bind();
	framebuffer->AttachTexture2D( GL_TEXTURE_2D, GL_TEXTURE_2D, data.depthForCubeMapId, FrameBuffer::eAttachmentTypeDepth, false );
	framebuffer->UnBind();

	//int i=3;
	//static int cubeMapFace = 0;
	
	const CGPUFBScene::CTilingInfo tilingInfo = {0,0,0,0, nullptr};

	const bool renderBackground = (pCubeMap->RenderBackground.AsInt() > 0);
	const bool renderTransparency = (pCubeMap->RenderTransparency.AsInt() > 0);
	const bool renderSecondary = (pCubeMap->RenderSecondary.AsInt() > 0);

	auto fn_renderCubeFace = [&] (int cubeMapFace) 
	{
		//
		PrepareCamera(nullptr, tilingInfo, true, cubeMapFace, &data);
		PrepareBuffersFromCamera();

		// bind face
		framebuffer->Bind();
		// we want to render into color buffer
		glDrawBuffer         ( GL_COLOR_ATTACHMENT0 );
		CHECK_GL_ERROR_MOBU();

		glEnable(GL_DEPTH_TEST);

		//framebuffer->AttachTexture2D( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 
		//	data.cubeMapId, FrameBuffer::eAttachmentTypeColor0, false );
		framebuffer->AttachTextureLayer( GL_TEXTURE_CUBE_MAP, data.cubeMapId, cubeMapFace );
		// prepare frame to draw
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		//
		cameraRenderOptions.MuteTextureId( data.outputId );

		//
		
		if (renderBackground)
		{
			RenderScene(eRenderLayerBackground, cameraRenderOptions, pFBRenderOptions, renderTransparency, stats );

			//
			glClear( GL_DEPTH_BUFFER_BIT );
		}

		RenderScene(eRenderLayerMain, cameraRenderOptions, pFBRenderOptions, renderTransparency, stats);

		if (renderSecondary)
		{
			RenderScene(eRenderLayerSecondary, cameraRenderOptions, pFBRenderOptions, renderTransparency, stats);
		}

		cameraRenderOptions.SetRenderLayerId(eRenderLayerMain);
		cameraRenderOptions.SetPass(eShaderPassOpaque);

		framebuffer->UnBind();
	};

	//
	//
	if (videoRendering)
	{
		for (int i=0; i<6; ++i)
			fn_renderCubeFace(i);
	}
	else
	{
		fn_renderCubeFace( pCubeMap->GetLastProcessedFace() );
		pCubeMap->IncCubeMapFaceIndex();
	}
	
}

bool CGPUFBScene::RenderPano(CubeMapRenderingData &data, const GLuint cubemapId, const bool generateMipMaps)
{
	if (mFrameBufferCubeMap.get() == nullptr || data.outputId == 0)
		return false;

	// prep framebuffer

	if (mFrameBufferPano.get() == nullptr)
	{
		mFrameBufferPano.reset( new FrameBuffer(1, 1, 
			FrameBuffer::eDeleteFramebufferOnCleanup, 1) );
	}

	//
	mFrameBufferPano->ReSize(data.outputWidth, data.outputHeight);

	// prep shader

	if (mShaderPano.get() == nullptr)
	{
		FBString effectPath, effectFullName;
		CHECK_GL_ERROR_MOBU();

		mShaderPano.reset( new GLSLShader() );

		if ( false == FindEffectLocation( "\\GLSL\\cube2Pan.fsh", effectPath, effectFullName ) )
		{
			printf( "Failed to locate shader files\n" );
			return false;
		}
		
		if ( false == mShaderPano->LoadShaders( FBString(effectPath, "\\GLSL\\simple.vsh"), FBString(effectPath, "\\GLSL\\cube2Pan.fsh") ) )
		{
			printf( "Failed to load shaders\n" );
			return false;
		}
	}


	//
	// rendering

	glMemoryBarrier( GL_TEXTURE_UPDATE_BARRIER_BIT );

	mFrameBufferPano->Bind();
	// we want to render into color buffer
	glDrawBuffer         ( GL_COLOR_ATTACHMENT0 );
	mFrameBufferPano->AttachTexture2D( GL_TEXTURE_2D, GL_TEXTURE_2D, data.outputId );

	mShaderPano->Bind();

	mShaderPano->bindTexture( GL_TEXTURE_CUBE_MAP, "cubeMapSampler", cubemapId, 0 );

	// draw quad
	drawOrthoQuad2d(data.outputWidth, data.outputHeight);

	mShaderPano->UnBind();

	mFrameBufferPano->UnBind();

	if (generateMipMaps)
	{
		glMemoryBarrier( GL_TEXTURE_UPDATE_BARRIER_BIT );

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, data.outputId);
		glGenerateMipmap(GL_TEXTURE_2D); // /!\ Allocate the mipmaps
		
	}

	return true;
}

void CGPUFBScene::UploadCubeMapUniforms(const GLuint textureId)
{
	if (textureId == 0 || mCubeMapNodes.size() == 0)
		return;

	CubeMapRendering *pCubeMap = nullptr;

	for (auto iter=mCubeMapNodes.begin(); iter!=mCubeMapNodes.end(); ++iter)
	{
		pCubeMap = (CubeMapRendering*) (*iter);

		CubeMapRenderingData	&data = pCubeMap->GetData();

		if (data.outputId == textureId)
		{
			mUberShader->UploadCubeMapUniforms(data.zmin, data.zmax, data.worldToLocal, data.position, data.max, data.min, data.useParallax );

			//if (data.useParallax > 0.0f)
			//{
				glActiveTexture(GL_TEXTURE17);
				glBindTexture(GL_TEXTURE_CUBE_MAP, data.cubeMapId);

				glActiveTexture(GL_TEXTURE0);
			//}

			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void CGPUFBScene::AddCameraNode(FBComponent *pNode)
{
	mCameraNodes.push_back(pNode);
}
void CGPUFBScene::RemoveCameraNode(FBComponent *pNode)
{
	auto iter = std::find(mCameraNodes.begin(), mCameraNodes.end(), pNode);
	if (iter != mCameraNodes.end() )
		mCameraNodes.erase(iter);
}
void CGPUFBScene::ClearCameraNodes()
{
	mCameraNodes.clear();
}

void iclamp(int &value, const int minVal, const int maxVal)
{
	if (value < minVal) value = minVal;
	else if (value > maxVal) value = maxVal;
}

bool CGPUFBScene::RenderAllCameras(FBRenderOptions *pFBRenderOptions, const bool videoRendering, unsigned int frameId, const int maxOutputSize)
{
	if (mCameraNodes.size() == 0)
		return false;

	CCameraInfoCache cacheCopy = mCameraCache;

	FBTime localTime = mSystem.LocalTime;

	CameraRendering *pCameraNode = nullptr;

	for (auto iter=mCameraNodes.begin(); iter!=mCameraNodes.end(); ++iter)
	{
	
		pCameraNode = (CameraRendering*) (*iter);

		if (false == pCameraNode->Enabled || 0 == pCameraNode->Camera.GetCount() )
			continue;

		if (false == pCameraNode->CheckForHiddenIncludeList() )
			continue;

		if (true == pCameraNode->CommonCheckForUpdate(false) )
			pCameraNode->SetUpdateFlag(true);
		pCameraNode->ProcessUpdateConditions( videoRendering, localTime, frameId );

		if ( false == pCameraNode->HasUpdateFlag() )
			continue;

		RenderingStats &stats = pCameraNode->GetStats();
		stats.Reset();
		pCameraNode->ResetUpdateFlag();

		CubeMapRenderingData	&data = pCameraNode->GetData();
		data.maxOutputSize = maxOutputSize;

		FBCamera *pCamera = (FBCamera*) pCameraNode->Camera.GetAt(0);

		//
		// prepare global uniforms and lighting
		const CGPUFBScene::CTilingInfo tilingInfo = {0,0,0,0, nullptr};
		
		PrepareCamera( pCamera, tilingInfo, false, 0, nullptr );
		//PrepFBSceneLights();
		PrepareBuffersFromCamera();

		FBVector3d v;
		pCamera->GetVector(v);
		data.position = glm::vec3( (float)v[0], (float)v[1], (float)v[2] );
		data.pComponent = pCameraNode;

		data.useStaticCubeMap = pCameraNode->UseFile;

		data.zmax = pCamera->FarPlaneDistance;
		data.zmin = pCamera->NearPlaneDistance;

		

		// DONE: collect parameters
		
		int newWidth = pCameraNode->OutputWidth;
		int newHeight = pCameraNode->OutputHeight;

		// DONE: check global settings maximum resolution value
		iclamp( newWidth, 512, data.maxOutputSize );
		iclamp( newHeight, 512, data.maxOutputSize );
		
		pCameraNode->PrepOutputTextureObject(newWidth, newHeight);

		data.renderGeomCache = pCameraNode->UseGPUCache;

		//
		//

		RenderCamera(pFBRenderOptions, data, mSettingsDepthLog, stats);
	}
	return true;
}

void CGPUFBScene::RenderScene( ERenderLayer layerId, CRenderOptions &options, 
	FBRenderOptions *pFBRenderOptions, bool renderTransparency, RenderingStats &stats )
{
	const bool videoRendering = options.IsOfflineRender();

	options.SetRenderLayerId(layerId);

	//
	// Opaque models

	if (true == videoRendering)
		FBTrace( "   -- render opaque\n" );

	options.SetGoal( eRenderGoalShading );
	options.SetPass( eShaderPassOpaque );

	RenderingStats opaqueStats;
	RenderSceneShaderGroups( options, pFBRenderOptions, opaqueStats );
	
	stats.CountOpaqueObjects( opaqueStats.totalDrawNodeCount );
	stats.CountOpaqueShaders( opaqueStats.totalShadersCount );

	//
	// Transparency pass
	const CRenderLayerArray &layersInfo = GetRenderLayersInfo();

	if ( true == layersInfo.HasTransparencyShaders(layerId)
		&& true == renderTransparency)
	{
		if (true == videoRendering)
			FBTrace( "   -- render transparency\n" );
		
		if (layersInfo.GetAlphaSourceCount(layerId, kFBAlphaSourceAccurateAlpha) > 0)
		{
			if (options.IsMultisampling() /*&& layerId != eRenderLayerSecondary*/ )
			{
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			}
			else
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			
			//
			options.SetPass( eShaderPassTransparency );
			RenderingStats trStats;
			RenderSceneShaderGroups( options, pFBRenderOptions, trStats );

			stats.CountTransparentModels( trStats.totalDrawNodeCount );
			stats.CountTransparencyShaders( trStats.totalShadersCount );
			
			if (options.IsMultisampling() /*&& layerId != eRenderLayerSecondary*/ )
			{
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			}
			else
			{
				glDisable(GL_BLEND);
			}
			
		}

		if (layersInfo.GetAlphaSourceCount(layerId, kFBAlphaSourceAdditiveAlpha) > 0)
		{
			glDepthMask(GL_FALSE);
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			
			//
			options.SetPass( eShaderPassAdditive );
			RenderingStats trStats;
			RenderSceneShaderGroups( options, pFBRenderOptions, trStats );

			stats.CountTransparentModels( trStats.totalDrawNodeCount );
			stats.CountTransparencyShaders( trStats.totalShadersCount );
			
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
	}
}


void CGPUFBScene::RenderCamera(FBRenderOptions *pFBRenderOptions, CubeMapRenderingData &data, const bool logDepth, RenderingStats &stats)
{
	if (mUberShader.get() == nullptr)
		return;

	stats.Reset();

	// prep framebuffer

	if (mFrameBufferCamera.get() == nullptr)
	{
		mFrameBufferCamera.reset( new FrameBuffer(1, 1, 
			FrameBuffer::eDeleteFramebufferOnCleanup, 1) );
	}

	//
	if (true == mFrameBufferCamera->ReSize(data.outputWidth, data.outputHeight) )
	{
		/*
		if (false == mFrameBufferCamera->IsOk() )
		{
			FBTrace("[ERROR] Failed while resizing a Camera framebuffer\n" );
			return;
		}
		*/
	}

	mFrameBufferCamera->Bind();

	mFrameBufferCamera->AttachTexture2D( GL_TEXTURE_2D, GL_TEXTURE_2D, data.outputId, FrameBuffer::eAttachmentTypeColor0, false );
	mFrameBufferCamera->AttachTexture2D( GL_TEXTURE_2D, GL_TEXTURE_2D, data.depthId, FrameBuffer::eAttachmentTypeDepth, false );

	// TODO: do some rendering (MS or normal FBO)

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	FBColor backColor;
	backColor = mCamera->BackGroundColor;
	if (mCamera->UseFrameColor)
		backColor = mCamera->FrameColor;

	glClearColor( (float)backColor[0], (float)backColor[1], (float)backColor[2], 0.0f );

	glViewport(0, 0, data.outputWidth, data.outputHeight);

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//
	// TODO: pre-cache render list for camera rendering, too many switches and checks
	// TODO: 1 - do an early-z rendering, 2 - do a projTex shader rendering
	// render objects here or in a shader ?!
	CameraRendering *pCameraComponent = (CameraRendering*) data.pComponent;
	int includeListCount = pCameraComponent->IncludeList.GetCount();
	int excludeListCount = pCameraComponent->ExcludeList.GetCount();

	CRenderOptions cameraRenderOptions = GetLastRenderOptions();

	cameraRenderOptions.SetGoal( eRenderGoalShading );
	cameraRenderOptions.SetPass( eShaderPassOpaque );
	cameraRenderOptions.SetCamera( mCamera );
	cameraRenderOptions.SetUniqueFrameId(1);
	cameraRenderOptions.SetFrustumCulling( (includeListCount>0 || excludeListCount>0) );
	MarkModelsWithIncludeExcludeLists(&pCameraComponent->IncludeList, &pCameraComponent->ExcludeList);

	cameraRenderOptions.MuteTextureId( data.outputId );

	const bool renderBackground = (pCameraComponent->RenderBackground.AsInt() > 0);
	const bool renderTransparency = (pCameraComponent->RenderTransparency.AsInt() > 0);
	const bool renderSecondary = (pCameraComponent->RenderSecondary.AsInt() > 0);

	if (renderBackground)
	{
		RenderScene(eRenderLayerBackground, cameraRenderOptions, pFBRenderOptions, renderTransparency, stats );

		//
		glClear( GL_DEPTH_BUFFER_BIT );
	}

	RenderScene(eRenderLayerMain, cameraRenderOptions, pFBRenderOptions, renderTransparency, stats);

	if (renderSecondary)
	{
		RenderScene(eRenderLayerSecondary, cameraRenderOptions, pFBRenderOptions, renderTransparency, stats);
	}

	cameraRenderOptions.SetRenderLayerId(eRenderLayerMain);
	cameraRenderOptions.SetPass(eShaderPassOpaque);

	//
	mFrameBufferCamera->UnBind();
}


bool CGPUFBScene::RenderModel(bool bindTextures, FBModel *pModel, const int modelIndex)
{
	const auto loc = mUberShader->GetCustomEffectShaderLocationsPtr();

	FBModelVertexData *pData = pModel->ModelVertexData;

	if (false == pData->IsDrawable() )
	{
		return false;
	}

	//const int regionCount = pData->GetSubRegionCount();
	const int patchCount = pData->GetSubPatchCount();

	// needed for primitives indices
	pData->VertexArrayMappingRelease();
	pData->EnableOGLVertexData();

	BindVertexAttribFromFBModel(pModel);
	//
	
	int meshIndex = mModelsInspector.GetModelMeshIndex(modelIndex);

	for (int i=0; i<patchCount; ++i)
	{
		int offset = pData->GetSubPatchIndexOffset(i);
		int size = pData->GetSubPatchIndexSize(i);

		mUberShader->UpdateMeshIndex(meshIndex + i);
		//mBufferMesh.BindAsAttribute( 5, sizeof(MeshGLSL)*(meshIndex+i) );
		
		// TODO: replace with bindless textures !
		if (bindTextures)
		{
			FBMaterial *pMaterial = pData->GetSubPatchMaterial(i);
			BindMaterialTextures(pMaterial, nullptr, false);
		}

		//
		pData->DrawSubPatch(i);
		//glDrawArrays( GL_POINTS, offset, size );
	}

	pData->DisableOGLVertexData();

	CHECK_GL_ERROR_MOBU();
	return true;
}

void CGPUFBScene::Textures_InitOGL(FBRenderOptions *pRenderOptions)
{
	if (mSystem.Scene->Textures.GetCount() != mTexturesInspector.GetNumberOfItems()
		&& false == mTexturesInspector.IsFullUpdateNeeded() )
	{
		FBTrace( "!!! ERROR in Textures Inspector count !!!\n" );
	}

	mTexturesInspector.MoBuInitTextures(pRenderOptions);
}

void CGPUFBScene::BindMaterialTextures(FBMaterial *pMaterial, FBRenderOptions *pRenderOptions, bool forceInit)
{
	// DONE: bind material textures
	const int rId = GetTextureId( pMaterial->GetTexture(kFBMaterialTextureReflection), pRenderOptions, forceInit );
	const int sId = GetTextureId( pMaterial->GetTexture(kFBMaterialTextureSpecular), pRenderOptions, forceInit );
	const int tId = GetTextureId( pMaterial->GetTexture(kFBMaterialTextureTransparent), pRenderOptions, forceInit );
	const int dId = GetTextureId( pMaterial->GetTexture(), pRenderOptions, forceInit );
	
	if (rId > 0)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, rId);

		// use specified cubemap uniforms for parallax correction when computing reflections in shader
		if (rId > 0)
			UploadCubeMapUniforms(rId);
	}
	if (sId > 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, sId);
	}
	if (tId > 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tId);
	}
	if (dId > 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, dId);
	}
}

void CGPUFBScene::PreRenderSceneShaderGroups(const CRenderOptions &options)
{

	//
	// Prepare data per model if needed for specified shader group

	const auto &shaderGroups = mShadersFactory.GetShaderTypesVector();

	for (auto iter=begin(shaderGroups); iter!=end(shaderGroups); ++iter)
	{
		CBaseShaderCallback *pShaderGroup = *iter;

		// check if callback supports current pass
		//if ( 0 == (intgoal & pShaderGroup->ShaderGroupGoal()) )
		//	continue;

		const int id = pShaderGroup->Id();

		if (pShaderGroup->IsNeedToPrepareInstance() )
		{
			auto &sortedVector = mShadersInspector.GetShaderGroupShaders(id);

			for (auto shaderIter=begin(sortedVector);
				shaderIter != end(sortedVector);
				++shaderIter )
			{

				pShaderGroup->PrepareInstance( options, *shaderIter );
			}
		}

		if (pShaderGroup->IsNeedToPrepareModel() )
		{
			const int numberOfModels = mModelsInspector.GetNumberOfModelsInGroup(id);

			if (numberOfModels == 0)
				continue;

			auto &sortedMap = mModelsInspector.GetSortedModelsMap(id);
		
			for (auto shaderIter=begin(sortedMap);
				shaderIter != end(sortedMap);
				++shaderIter )
			{
				pShaderGroup->PrepareModel(options, shaderIter->second.pModel, shaderIter->first);
			}
		}
	}
}

void CGPUFBScene::ChangeContextForShaderGroups(FBRenderOptions *pFBRenderOptions)
{
	const auto &shaderGroups = mShadersFactory.GetShaderTypesVector();

	for (auto iter=begin(shaderGroups); iter!=end(shaderGroups); ++iter)
	{
		CBaseShaderCallback *pShaderGroup = *iter;

		// check if callback supports current pass
		//if ( 0 == (intgoal & pShaderGroup->ShaderGroupGoal()) )
		//	continue;

		const int id = pShaderGroup->Id();

		
		auto &sortedVector = mShadersInspector.GetShaderGroupShaders(id);

		for (auto shaderIter=begin(sortedVector);
			shaderIter != end(sortedVector);
			++shaderIter )
		{
			pShaderGroup->DetachRenderContext( pFBRenderOptions, *shaderIter );
		}
	}
}

void CGPUFBScene::RenderSceneShaderGroups(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, RenderingStats &stats)
{

	const int intpass = (int) options.GetPass();
	const int intgoal = (int) options.GetGoal();
	
	const auto &shaderGroups = mShadersFactory.GetShaderTypesVector();

	for (auto iter=begin(shaderGroups); iter!=end(shaderGroups); ++iter)
	{
		CBaseShaderCallback *pShaderGroup = *iter;

		// check if callback supports current pass
		if ( 0 == (intgoal & pShaderGroup->ShaderGroupGoal()) )
			continue;

		const int id = pShaderGroup->Id();
		const int numberOfModels = mModelsInspector.GetNumberOfModelsInGroup(id);
#ifdef DEBUG_RENDERBUG
		FBTrace( "id - %d, models - %d\n", id, numberOfModels );
#endif
		
		if (numberOfModels == 0)
			continue;

		//
		bool lStatus = false;

		lStatus = pShaderGroup->OnTypeBegin( options, true );

		if (false == lStatus)
			continue;

		// draw connected meshes with current shader group

		auto &sortedMap = mModelsInspector.GetSortedModelsMap(id);
		FBShader *pLastShader = nullptr; // (FBShader*) 1;
		bool isFirstShader = true;
		bool isBlendEnabled = false;

		for (auto shaderIter=begin(sortedMap);
			shaderIter != end(sortedMap);
			++shaderIter )
		{
			//
			// TODO: put a shader info !!
			FBShader *pShader = shaderIter->first;
			if (nullptr != pShader && false == pShader->Enable)
				continue;

			// DONE: check render layer id
			const int shaderLayerId = CRenderLayerArray::GetShaderRenderLayerId(pShader);

			if ( false == options.HasRenderLayerId(shaderLayerId) )
				continue;

			//
			if (pLastShader != pShader || true == isFirstShader)
			{
				if (false == isFirstShader)
				{
					pShaderGroup->OnInstanceEnd( options, pLastShader, nullptr);

					if (options.IsMultisampling() && (true == isBlendEnabled) )
					{
						//glDisable(GL_BLEND);
						//glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
					}
				}

				lStatus = pShaderGroup->OnInstanceBegin( options, pFBRenderOptions, pShader, nullptr );

				if (false == lStatus)
					continue;
#ifdef DEBUG_RENDERBUG			
				FBTrace( "count one shader\n" );
#endif
				stats.CountOneShader();

				if (nullptr != pShader && eShaderPassTransparency == options.GetPass() )
				{
					FBProperty *pProp = pShader->PropertyList.Find("Transparency");
					if (nullptr != pProp)
					{
						FBAlphaSource alphaSource = (FBAlphaSource) pProp->AsInt();
						if (kFBAlphaSourceAdditiveAlpha == alphaSource)
						{
							//glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
							//glEnable(GL_BLEND);
							//glBlendFunc(GL_ONE, GL_DST_ALPHA);
							//glBlendFunc(GL_ONE, GL_ONE);
							//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

							//glBlendEquation(GL_FUNC_ADD);

							//glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
							//glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ZERO); // GL_ONE_MINUS_SRC_ALPHA

							isBlendEnabled = true;
						}
						else if (kFBAlphaSourceAccurateAlpha == alphaSource)
						{
							//glEnable(GL_BLEND);
							//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							/*
							if (options.IsMultisampling() )
							{
								glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
							}
							else
							{
								glEnable(GL_BLEND);
								glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
								//glEnable(GL_ALPHA_TEST);
								//glAlphaFunc(GL_GEQUAL, 0.25f);
							}
							*/
							isBlendEnabled = true;
						}
					}
				}

				pLastShader = pShader;
			}
			
			isFirstShader = false;
			
			// iteration for shader models
			FBModel *pModel = shaderIter->second.pModel;
#ifdef DEBUG_RENDERBUG
			FBString modelName(pModel->LongName);
			const char *szModelName = modelName;
//#ifdef DEBUG_RENDERBUG
			FBTrace( "model - %s\n", szModelName );
//#endif
#endif
			if (options.IsFrustumCullingEnabled() )
			{
				FBProperty *pProp = pModel->PropertyList.Find("id");
				if (nullptr != pProp)
					if (pProp->AsInt() != options.GetUniqueFrameId() )
						continue;
			}

			if (options.IsShadowRendering() )
			{
				if (false == pModel->CastsShadows)
					continue;
			}

			FBEvaluateInfo *pEvalInfo = FBGetDisplayInfo();
			if (pModel->IsVisible(pEvalInfo) == false || pModel->Show == false)
				continue;
			/*
			if ( false == pModel->IsEvaluationReady(kFBModelEvaluationDeform) )
			{
				mNumberOfUnReadyModels += 1;
				continue;
			}
			*/

			if ( pShaderGroup->IsMaterialEffect() )
			{
				lStatus = pShaderGroup->OnModelDraw( options, pFBRenderOptions, pModel, nullptr);
				if (true == lStatus)
				{
					RenderPassModelDraw( pShaderGroup, options, pModel, shaderIter->second.startMeshId );
				}
			}
			else
			{
				lStatus = pShaderGroup->OwnModelShade( options, pFBRenderOptions, pModel, pShader, nullptr );
			}

#ifdef DEBUG_RENDERBUG
			FBTrace( "count one model\n" );
#endif
			stats.CountOneObject();
		}

		if (false == isFirstShader)
		{
			pShaderGroup->OnInstanceEnd( options, pLastShader, nullptr);

			if (options.IsMultisampling() && (true == isBlendEnabled) )
			{
				//glDisable(GL_BLEND);
				//glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			}
		}
#ifdef DEBUG_RENDERBUG
		FBTrace( "type end\n" );
#endif
		pShaderGroup->OnTypeEnd( options );
	}
}


bool CGPUFBScene::PrepRenderForFBModel(FBModel *pModel)
{

	if (mUberShader.get() == nullptr || pModel == nullptr)
		return false;

	const auto loc = mUberShader->GetCustomEffectShaderLocationsPtr();

	FBModelVertexData *pData = pModel->ModelVertexData;

	if (nullptr == pData || false == pData->IsDrawable() )
	{
		return false;
	}

	//const int regionCount = pData->GetSubRegionCount();
	const int patchCount = pData->GetSubPatchCount();

	// needed for primitives indices
	pData->VertexArrayMappingRelease();
	pData->EnableOGLVertexData();

	BindVertexAttribFromFBModel(pModel);
	//

	return true;
}

bool CGPUFBScene::RenderPassModelDraw(CBaseShaderCallback *pCallback, const CRenderOptions &options, FBModel *pModel, const int meshIndex)
{
	const auto loc = mUberShader->GetCustomEffectShaderLocationsPtr();

	FBModelVertexData *pData = pModel->ModelVertexData;
	
	if (nullptr == pData || false == pData->IsDrawable() )
	{
		return false;
	}

	//const int regionCount = pData->GetSubRegionCount();
	const int patchCount = pData->GetSubPatchCount();

	// needed for primitives indices
	//pData->VertexArrayMappingRelease();
	pData->EnableOGLVertexData();
	
	if (false == BindVertexAttribFromFBModel(pModel) )
	{
		pData->DisableOGLVertexData();
		return false;
	}
	/*
	if ( SHADER_CALLBACK_CHARACTER == pCallback->Id() )
	{
		CCharacterBindedCallback *pCharCallback = (CCharacterBindedCallback*) pCallback;
		pCharCallback->BindOutputNormalBuffer(pModel, 2);
	}
	*/
	CHECK_GL_ERROR_MOBU();

	FBMaterial *pLastMaterial = nullptr;

	//
	for (int i=0; i<patchCount; ++i)
	{
		int indexOffset = pData->GetSubPatchIndexOffset(i);
		int indexSize = pData->GetSubPatchIndexSize(i);

		mUberShader->UpdateMeshIndex(meshIndex + i);
		//mBufferMesh.BindAsAttribute( 5, sizeof(MeshGLSL)*(meshIndex+i) );
		
		// TODO: replace with bindless textures !
		/*
		if (bindTextures)
		{
			FBMaterial *pMaterial = pData->GetSubPatchMaterial(i);
			BindMaterialTextures(pMaterial);
		}
		*/

		CHECK_GL_ERROR_MOBU();

		FBMaterial *pMaterial = pData->GetSubPatchMaterial(i);

		if (pCallback && nullptr != pMaterial)
		{
			if (pLastMaterial != pMaterial)
			{
				if (pLastMaterial != nullptr)
					pCallback->OnMaterialEnd( options, pLastMaterial );

				pCallback->OnMaterialBegin( options, nullptr, pMaterial, false );
				pLastMaterial = pMaterial;
			}
		}

		//
		CHECK_GL_ERROR_MOBU();
		pData->DrawSubPatch(i);
		CHECK_GL_ERROR_MOBU();
		//const int instanceCount = 1;
		//glDrawElementsInstanced( GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, (GLvoid*) indexOffset, instanceCount);
		//glDrawArrays( GL_POINTS, indexOffset, indexSize );
	}

	pData->DisableOGLVertexData();

	
	if (pCallback && pLastMaterial != nullptr)
	{
		pCallback->OnMaterialEnd( options, pLastMaterial );
	}

	CHECK_GL_ERROR_MOBU();
	return true;
}


bool CGPUFBScene::RenderPassModelDraw(CBaseShaderCallback *pCallback, const CRenderOptions &options, FBModel *pModel)
{
	int meshIndex = mModelsInspector.GetModelMeshIndex(pModel);
	return RenderPassModelDraw(pCallback, options, pModel, meshIndex);
}

void CGPUFBScene::RenderSceneClassic(FBRenderOptions *pRenderOptions)
{
	FBRenderer* lRenderer = mSystem.Renderer;
  
	int numberOfModels = lRenderer->DisplayableGeometryCount;

	if (mUberShader.get() == nullptr || numberOfModels == 0)
		return;

	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glDepthMask(GL_TRUE);

	mUberShader->SetBindless(false);
	mUberShader->SetEarlyZ(true);
	
	//
	PrepRender();

	BindUberShader(false);

	FBEvaluateInfo *pEvalInfo = FBGetDisplayInfo();
	auto &modelsVector = mModelsInspector.GetResourceVector();
	auto &updateVector = mModelsInspector.GetUpdateVector();

	auto modelIter = begin(modelsVector);
	auto updateIter = begin(updateVector);

	int modelIndex = 0;
	for( ; modelIter!=end(modelsVector); 
		++modelIter, ++updateIter, ++modelIndex)
	{
		//if (mModelsInspector.IsDeleted(modelIndex) )
		//	continue;

		if ( *updateIter & RESOURCE_DELETED )
			continue;

		FBModel *pModel = *modelIter;

		if (FBIS(pModel, ORModelGPUCache) )
			continue;

		if (pModel->IsVisible(pEvalInfo) == false || pModel->Show == false)
			continue;

		// without any texture binding
		RenderModel(false, pModel, modelIndex);
	}

	UnBindUberShader();

	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//glDepthMask(GL_FALSE);
}

void CGPUFBScene::CameraOnContextChange()
{
	FreeCameraResources();

	//
	for (auto iter=begin(mCameraNodes); iter!=end(mCameraNodes); 
		++iter)
	{
		CameraRendering *pNode = (CameraRendering*) *iter;
		pNode->ResetUpdateValues();
	}
}

void CGPUFBScene::CubeMapOnContextChange()
{
	FreeCubeMapResources();

	//
	for (auto iter=begin(mCubeMapNodes); iter!=end(mCubeMapNodes); 
		++iter)
	{
		CubeMapRendering *pNode = (CubeMapRendering*) *iter;
		pNode->ResetUpdateValues();
	}
}

void CGPUFBScene::FreeCameraResources()
{
	mFrameBufferCamera.reset(nullptr);
}

void CGPUFBScene::FreeCubeMapResources()
{
	mFrameBufferCubeMap.reset(nullptr);
	mFrameBufferPano.reset(nullptr);
	mShaderPano.reset(nullptr);
}