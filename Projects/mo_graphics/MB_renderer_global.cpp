
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_renderer_global.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "MB_renderer.h"
#include "compositeMaster_objectFinal.h"

FBCamera *gLastSceneCamera = nullptr;
FBVector3d gLastCameraPos;
unsigned int gLastUniqueFrameId = 0;
CRenderOptions gLastRenderOptions;
HGLRC gLastContext = 0;
bool gCallbackAttached = false;

///////////////////////////////////////////////////////////////////////////
//

bool IsCallbackAttached()
{
	return gCallbackAttached;
}

bool IsLogarithmicDepth()
{
	FBRenderer *pRenderer = FBSystem::TheOne().Renderer;

	const int index = pRenderer->CurrentPaneCallbackIndex;
	if (index >= 0)
	{
		FBRendererCallback *pCallback = pRenderer->RendererCallbacks[index];

		if ( FBIS(pCallback, MoRendererCallback) )
		{
			return ( (MoRendererCallback*) pCallback )->IsLogarithmicDepth();
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//

kLongLong InitializeFrameDataAndBuffers( FBRenderOptions *pFBRenderOptions, FBCamera *pCamera, FBEvaluateInfo *pEvalInfo, const unsigned int uniqueFrameId,
	const bool forceToUpdate, const int maxCameraResolution, const int maxCubeMapResolution, 
	const bool processCompositions, FBComponent *pCompositionComponent, const int shadowResolution)
{

	FBSystem &lSystem = FBSystem::TheOne();
#ifdef _DEBUG
	FBTime systemTime = lSystem.SystemTime;
	kLongLong msBeginTime = systemTime.GetMilliSeconds();
#endif
	HGLRC currContext = wglGetCurrentContext();
	if (gLastContext != 0 && gLastContext != currContext)
	{
		// change context !
		CGPUFBScene *pScene = &CGPUFBScene::instance();
		pScene->ChangeContext(nullptr);

		gLastContext = currContext;
	}
	else if (gLastContext == 0)
	{
		gLastContext = currContext;
	}

	gLastRenderOptions.SetCamera(pCamera);
	gLastRenderOptions.GrabCameraViewportSize();
	gLastRenderOptions.SetCubeMapRender(false, nullptr);
	
	CHECK_GL_ERROR_MOBU();

	FBVector3d camPos;
	pCamera->GetVector(camPos);

	const CGPUFBScene::CTilingInfo tilingInfo = {0,0,0,0, nullptr};

	if (gLastUniqueFrameId != uniqueFrameId)
	{
		gLastUniqueFrameId = uniqueFrameId;
		gLastSceneCamera = pCamera;
		gLastRenderOptions.SetUniqueFrameId(uniqueFrameId);

		CGPUFBScene *pScene = &CGPUFBScene::instance();

		// we need to prepare a new camera for a next pane rendering
		//pScene->LoadShader();

		CHECK_GL_ERROR_MOBU();
#ifdef _DEBUG		
		systemTime = lSystem.SystemTime;
		kLongLong msBeforePrepBuffers = systemTime.GetMilliSeconds() - msBeginTime;
#endif
		pScene->UseCompositionLights( nullptr );
		pScene->PrepareBuffers(pEvalInfo, false);
#ifdef _DEBUG		
		systemTime = lSystem.SystemTime;
		kLongLong msAfterPrepBuffers = systemTime.GetMilliSeconds() - msBeginTime;
#endif
		if (nullptr != pCompositionComponent)
		{
			ObjectComposition *pComposition = (ObjectComposition*) pCompositionComponent;
			if (pComposition->Lights.GetCount() > 0 && false == pComposition->UseSceneLights)
			{
				pComposition->AskToUpdateLightList();
				pComposition->EventBeforeRenderNotify();
			
				CGPUShaderLights *compositionLightsPtr = pComposition->GetShaderLightsPtr();
				if ((nullptr != compositionLightsPtr) 
					&& (compositionLightsPtr->GetNumberOfDirLights() > 0 || compositionLightsPtr->GetNumberOfLights() > 0) )
				{
					pScene->UseCompositionLights( compositionLightsPtr );
				}
			}
		}
#ifdef _DEBUG
		systemTime = lSystem.SystemTime;
		kLongLong msAfterComposition = systemTime.GetMilliSeconds() - msBeginTime;
#endif
		pScene->PrepareCamera( pCamera, tilingInfo, false, 0, nullptr );
		//pScene->PrepFBSceneLights();
		pScene->PrepareBuffersFromCamera();
#ifdef _DEBUG		
		systemTime = lSystem.SystemTime;
		kLongLong msAfterPrepBuffersCamera = systemTime.GetMilliSeconds() - msBeginTime;
#endif
		// prepare local gpu data buffer for shader groups
		pScene->PreRenderSceneShaderGroups( gLastRenderOptions );
#ifdef _DEBUG		
		systemTime = lSystem.SystemTime;
		kLongLong msAfterPreRenderGroups = systemTime.GetMilliSeconds() - msBeginTime;
#endif
		gLastRenderOptions.GrabCameraViewportSize();

		// first of all, let's see if we have any offline camera, cubemap to render
		pScene->PushCameraCache();

		const bool offlineRender = gLastRenderOptions.IsOfflineRender();
		const bool updateCameras = pScene->RenderAllCameras( pFBRenderOptions, offlineRender, uniqueFrameId, maxCameraResolution );
		const bool updateCubeMaps = pScene->RenderAllCubeMaps( pFBRenderOptions, offlineRender, uniqueFrameId, maxCubeMapResolution );
		if (updateCameras || updateCubeMaps)
		{
			gLastRenderOptions.SetCubeMapRender(false, nullptr);

			pScene->PopCameraCache();
			//pScene->PrepareCamera( pCamera, nullptr, false, 0, nullptr );
			//pScene->PrepFBSceneLights();
			pScene->PrepareBuffersFromCamera();

			gLastRenderOptions.SetGLViewport();
		}
#ifdef _DEBUG
		systemTime = lSystem.SystemTime;
		kLongLong msAfterExtended = systemTime.GetMilliSeconds() - msBeginTime;
#endif
		//
		if (processCompositions && shadowResolution > 0)
		{
			pScene->SetLogarithmicDepth(false);
			pScene->RenderAllShadowMaps( gLastRenderOptions, pFBRenderOptions, shadowResolution );
		}
#ifdef _DEBUG
		systemTime = lSystem.SystemTime;
		kLongLong msAfterShadowMaps = systemTime.GetMilliSeconds() - msBeginTime;
#endif
		//
		//
		gLastRenderOptions.SetUniqueFrameId(uniqueFrameId);
		gLastRenderOptions.SetCamera(pCamera);

		gLastUniqueFrameId = uniqueFrameId;
		gLastSceneCamera = pCamera;
		gLastCameraPos = camPos;
	}
	else if (gLastSceneCamera != pCamera || true == forceToUpdate)
	{
		CGPUFBScene *pScene = &CGPUFBScene::instance();

		if (gLastSceneCamera != pCamera)
		{
			//Compositions_FindCurrent( pCamera );
			pScene->UseCompositionLights( nullptr );
			
			if (nullptr != pCompositionComponent)
			{
				ObjectComposition *pComposition = (ObjectComposition*) pCompositionComponent;
				if (pComposition->Lights.GetCount() > 0 && false == pComposition->UseSceneLights)
				{
					pComposition->AskToUpdateLightList();
					pComposition->EventBeforeRenderNotify();
			
					CGPUShaderLights *compositionLightsPtr = pComposition->GetShaderLightsPtr();
					if ((nullptr != compositionLightsPtr) 
						&& (compositionLightsPtr->GetNumberOfDirLights() > 0 || compositionLightsPtr->GetNumberOfLights() > 0) )
					{
						pScene->UseCompositionLights( compositionLightsPtr );
					}
				}
			}
		}

		pScene->PrepareCamera( pCamera, tilingInfo, false, 0, nullptr );
		//pScene->PrepFBSceneLights();
		pScene->PrepareBuffersFromCamera();

		gLastSceneCamera = pCamera;
	}

#ifdef _DEBUG
	systemTime = lSystem.SystemTime;
	kLongLong msEndTime = systemTime.GetMilliSeconds() - msBeginTime;

	return msEndTime;
#else
	return 0;
#endif
}

FBComponent *GetCurrentCompositionComponent()
{
	FBRenderer *pRenderer = FBSystem::TheOne().Renderer;

	const int index = pRenderer->CurrentPaneCallbackIndex;
	if (index >= 0)
	{
		FBRendererCallback *pCallback = pRenderer->RendererCallbacks[index];

		if ( FBIS(pCallback, MoRendererCallback) )
		{
			MoRendererCallback *morender = (MoRendererCallback*) pCallback;
			return ((morender->CurrentComposition.GetCount() > 0) ? morender->CurrentComposition[0] : nullptr);
		}
	}

	return nullptr;
}

CRenderOptions &GetLastRenderOptions()
{
	return gLastRenderOptions;
}