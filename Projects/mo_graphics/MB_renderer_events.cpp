
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_render_events.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "MB_renderer.h"
#include "dynamicmask_common.h"
#include "compositeMaster_objectFinal.h"
#include "compositeMaster_objectDecalFilter.h"

static bool execOnlyOnce = true;

void MoRendererCallback::OnUIIdle(HISender pSender, HKEvent pEvent)
{
	//
	if (execOnlyOnce == true)
	{
		mSystem.OnConnectionNotify.Add(this, (FBCallback)&MoRendererCallback::EventConnNotify);
		mSystem.OnConnectionDataNotify.Add(this, (FBCallback)&MoRendererCallback::EventConnDataNotify);
		// TODO: at the moment disconnected because it is not used by scene manager
		// FBSystem::TheOne().OnConnectionStateNotify.Add(this, (FBCallback)&GraphicsManager::EventConnStateNotify);

		CreateMenu();

		execOnlyOnce = false;
	}

	// NOTE! in that case we have a different context !
	//mGPUFBScene->LoadShader();

	mSystem.OnUIIdle.Remove( this, (FBCallback) &MoRendererCallback::OnUIIdle );
}


void MoRendererCallback::EventFileNew(HISender pSender, HKEvent pEvent)
{
	//
	// dynamic masks

	DYNAMIC_MASK_OPERATIONS::ChooseMask(nullptr);

}

void MoRendererCallback::EventFileOpen(HISender pSender, HKEvent pEvent)
{
	//
	// dynamic masks

	DYNAMIC_MASK_OPERATIONS::ChooseMask(nullptr);
}

void MoRendererCallback::EventFileMerge(HISender pSender, HKEvent pEvent)
{

	//
	// dynamic masks

	DYNAMIC_MASK_OPERATIONS::ChooseMask(nullptr);
}

void MoRendererCallback::EventFileOpenComplete(HISender pSender, HKEvent pEvent)
{
	// this is a hack to make RayCasting work with geometry gpu caches
	ConvertAllBoxIntoGPUCacheObject();
}

void MoRendererCallback::EventSceneChange(HISender pSender, HKEvent pEvent)
{

	FBEventSceneChange	e(pEvent);

	// DONE: update handles list according to the textures changes
	const FBSceneChangeType type = e.Type;

	if ( FBIS( e.ChildComponent, ObjectComposition) && (type == kFBSceneChangeChangedParent || type == kFBSceneChangeAttach) )
	{	
		for (int i=0, count = e.ChildComponent->Parents.GetCount(); i<count; ++i)
		{
			FBComponent *pParent = e.ChildComponent->Parents.GetAt(i);
					
			if ( FBIS(pParent, FBScene) )
			{
				auto iter = std::find(begin(mCompositionsVector), end(mCompositionsVector), e.ChildComponent);
				if (iter == end(mCompositionsVector) )
					mCompositionsVector.push_back(e.ChildComponent);
				break;
			}
		}
	}
	else if ( FBIS( e.Component, ObjectComposition) && type == kFBSceneChangeChangedParent )
	{	
		for (int i=0, count = e.Component->Parents.GetCount(); i<count; ++i)
		{
			FBComponent *pParent = e.Component->Parents.GetAt(i);
					
			if ( FBIS(pParent, FBScene) )
			{
				auto iter = std::find(begin(mCompositionsVector), end(mCompositionsVector), e.Component);
				if (iter == end(mCompositionsVector) )
					mCompositionsVector.push_back(e.Component);
				break;
			}
		}
	}
	else if ( FBIS( e.ChildComponent, ObjectComposition) && type == kFBSceneChangeDetach )
	{
		if (FBIS(e.Component, FBScene) )
		{
			auto iter = std::find(begin(mCompositionsVector), end(mCompositionsVector), e.ChildComponent);
			if (iter != end(mCompositionsVector) )
				mCompositionsVector.erase(iter);
		} 
	}
	else if ( FBIS( e.ChildComponent, ObjectMask) && (type == kFBSceneChangeChangedParent || type == kFBSceneChangeAttach) )
	{	
		for (int i=0, count = e.ChildComponent->Parents.GetCount(); i<count; ++i)
		{
			FBComponent *pParent = e.ChildComponent->Parents.GetAt(i);
					
			if ( FBIS(pParent, FBScene) )
			{
				auto iter = std::find(begin(mDynamicMasksVector), end(mDynamicMasksVector), e.ChildComponent);
				if (iter == end(mDynamicMasksVector) )
					mDynamicMasksVector.push_back(e.ChildComponent);
				break;
			}
		}
	}
	else if ( FBIS( e.Component, ObjectMask) && type == kFBSceneChangeChangedParent )
	{	
		for (int i=0, count = e.Component->Parents.GetCount(); i<count; ++i)
		{
			FBComponent *pParent = e.Component->Parents.GetAt(i);
					
			if ( FBIS(pParent, FBScene) )
			{
				auto iter = std::find(begin(mDynamicMasksVector), end(mDynamicMasksVector), e.Component);
				if (iter == end(mDynamicMasksVector) )
					mDynamicMasksVector.push_back(e.Component);
				break;
			}
		}
	}
	else if ( FBIS( e.ChildComponent, ObjectMask) && type == kFBSceneChangeDetach )
	{
		if (FBIS(e.Component, FBScene) )
		{
			auto iter = std::find(begin(mDynamicMasksVector), end(mDynamicMasksVector), e.ChildComponent);
			if (iter != end(mDynamicMasksVector) )
				mDynamicMasksVector.erase(iter);
		} 
	}
	

	//
	switch (type)
	{
	case kFBSceneChangeChangedParent:
	case kFBSceneChangeAttach:
	case kFBSceneChangeDetach:
	case kFBSceneChangeAddChild:
	case kFBSceneChangeRemoveChild:
	case kFBSceneChangeReordered:
		if ( FBIS(e.ChildComponent, ObjectFilter3dDecal) )
		{
			UpdateConnectedDecalFilters();
		}
		else
		if ( FBIS(e.ChildComponent, FBCamera) )
		{
			UpdateCameraResolution();
		}
		break;

	case kFBSceneChangeLoadBegin:
    case kFBSceneChangeClearBegin:
    case kFBSceneChangeMergeTransactionBegin:
        {
            // Lots of changes coming, stop incremental scene graph cache update. 
            //CacheSceneReset();
			
        }
        break;
	case kFBSceneChangeLoadEnd:
    case kFBSceneChangeClearEnd:
    case kFBSceneChangeMergeTransactionEnd:
        {
            // Lots of changes coming, stop incremental scene graph cache update. 
            //CacheSceneReset();
			UpdateCameraResolution();
			UpdateConnectedDecalFilters();
        }
        break;
    case kFBSceneChangeTransactionBegin:
        {
            // There are medium volume of changes coming, User could choose either full or incremental rebuild.
            //CacheSceneReset();

			//mGPUScene->Wait(true);
        }
		break;
	case kFBSceneChangeTransactionEnd:
        {
			//UpdateCameraResolution();
            // There are medium volume of changes coming, User could choose either full or incremental rebuild.
            //CacheSceneReset();
			/*
			mGPUScene->UpdateTextures();
			mGPUScene->UpdateMaterials();
			mGPUScene->UpdateShaders();
			mGPUScene->AskForEvaluateModels();

			mGPUScene->Wait(false);
			*/
        }
		break;
	}

	mGPUFBScene->EventSceneChange(pSender, pEvent);
}

void MoRendererCallback::EventConnDataNotify(HISender pSender, HKEvent pEvent)
{
	FBEventConnectionDataNotify	lEvent(pEvent);
    FBPlug*						lPlug;
    
    if( lEvent.Plug )
    {
        if (lEvent.Plug->Is(FBProperty::TypeInfo))
        {
            lPlug = lEvent.Plug->GetOwner();
			
			if (lPlug == this && lEvent.Action == kFBCandidated &&
				(lEvent.Plug == mGlobalResolution.mData.UseGlobalResolution 
				|| lEvent.Plug == mGlobalResolution.mData.CustomWidth
				|| lEvent.Plug == mGlobalResolution.mData.CustomHeight))
			{
				UpdateCameraResolution();
				UpdateConnectedDecalFilters();
			}
			else if (FBIS(lPlug, FBCamera) 
				&& lEvent.Action == kFBCandidated 
				&& mUpdatingCameraProperties == false )
			{
				FBCamera *pCamera = (FBCamera*) lPlug;
				lPlug = lEvent.Plug;
				/*
				if (lPlug == &pCamera->FrameSizeMode || lPlug == &pCamera->ResolutionWidth || lPlug == &pCamera->ResolutionHeight
					|| lPlug == &pCamera->ResolutionMode )
				{
					UpdateCameraResolution( pCamera );
				}
				*/
				UpdateCameraResolution( pCamera );
			}
        }
    }

	mGPUFBScene->EventConnDataNotify(pSender, pEvent);
}

void MoRendererCallback::EventConnNotify(HISender pSender, HKEvent pEvent)
{
	mGPUFBScene->EventConnNotify(pSender, pEvent);
}

void MoRendererCallback::EventConnStateNotify (HISender pSender, HKEvent pEvent)
{
	mGPUFBScene->EventConnStateNotify(pSender, pEvent);
}

void MoRendererCallback::OnPerFrameSynchronizationCallback(HISender pSender, HKEvent pEvent)
{
	mGPUFBScene->OnPerFrameSynchronizationCallback(pSender, pEvent);
}

void MoRendererCallback::OnPerFrameEvaluationPipelineCallback(HISender pSender, HKEvent pEvent)
{
	mGPUFBScene->OnPerFrameEvaluationPipelineCallback(pSender, pEvent);
}

void MoRendererCallback::OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent)
{

	FBEventEvalGlobalCallback lFBEvent(pEvent);

	CProperties::CStages &stages = mCurrentProperties->mStages;
	bool processMasks = (stages.ProcessDynamicMasks->AsInt() > 0);

	switch(lFBEvent.GetTiming() )
	{
	case kFBGlobalEvalCallbackBeforeRender:
		
		// TODO: where to get current frame and current camera ?!
		//InitializeFrameDataAndBuffers(

		if (true == processMasks)
		{
			DynamicMasks_Render();
		}
		
		CGPUFBScene *pScene = &CGPUFBScene::instance();
		pScene->LoadShader();
		
		break;
	}

	/*
	bool processCompositions = true;
	bool processMasks = true;
	bool logarithmicDepth = false;
	bool depthNV = false;

	CProperties::CQuality &quality = mCurrentProperties->mQuality;
	CProperties::CStages &stages = mCurrentProperties->mStages;

	processCompositions = (stages.ProcessCompositions->AsInt() > 0);
	processMasks = (stages.ProcessDynamicMasks->AsInt() > 0);

	logarithmicDepth = (quality.DisplayDepth->AsInt() == eGraphicsDepthLog);
	depthNV = (quality.DisplayDepth->AsInt() == eGraphicsDepthNV);
	

	bool hasChanged = (mCompositionDataExchange.logDepth != logarithmicDepth || mCompositionDataExchange.nvDepth != depthNV);

	mCompositionDataExchange.logDepth = logarithmicDepth;
	mCompositionDataExchange.nvDepth = depthNV;
		
	// TODO: transfer logDepth and nvDepth values to gpuscene !!

	if (hasChanged)
	{
		Compositions_ChangeGlobalSettings();
	}

	processCompositions = (processCompositions==true); // && (paneCount == 1);
	processCompositions = false;
	const bool renderToBuffer = processCompositions;
	mGPUFBScene->SetRenderToBufferState(false);
	
	switch(lFBEvent.GetTiming() )
	{
	case kFBGlobalEvalCallbackBeforeRender:
		
		// TODO: where to get current frame and current camera ?!
		//InitializeFrameDataAndBuffers(

		if (true == processMasks)
		{
			DynamicMasks_Render();
		}

		break;
	}
	*/
}

void MoRendererCallback::OnVideoFrameRendering	(HISender pSender, HKEvent pEvent)
{
	FBEventVideoFrameRendering levent(pEvent);

	switch(levent.GetState() )
	{
	case FBEventVideoFrameRendering::eBeginRendering:
		{
			// turn off preview mode and switch quality settings if needed
			mVideoRendering = true;

			FBCamera *pCamera = mSystem.Renderer->CurrentCamera;
			mCameraInternalDOF = pCamera->UseDepthOfField;
			pCamera->UseDepthOfField = false;

		} break;
	case FBEventVideoFrameRendering::eEndRendering:
		{
			// turn on back preview mode and display quality settings
			mVideoRendering = false;

			FBCamera *pCamera = mSystem.Renderer->CurrentCamera;
			pCamera->UseDepthOfField = mCameraInternalDOF;
		}
		break;
	}
}

bool MoRendererCallback::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if (pAction == kFBCandidated)
	{
		if (pThis == mRenderProperties.mQuality.QualityPreset)
		{
			ApplyQualityPreset( mRenderProperties.mQuality, 
				(EGraphicsQualityPresets) mRenderProperties.mQuality.QualityPreset->AsInt(), false );
		}
		else if (pThis == mDisplayProperties.mQuality.QualityPreset)
		{
			ApplyQualityPreset( mDisplayProperties.mQuality,
				(EGraphicsQualityPresets) mDisplayProperties.mQuality.QualityPreset->AsInt(), true );
		}
		else if (pThis == mRenderProperties.mQuality.QualityPreset2)
		{
			ApplyQualityPreset2( mRenderProperties.mQuality, 
				(EGraphicsQualityPresets) mRenderProperties.mQuality.QualityPreset2->AsInt(), false );
		}
		else if (pThis == mDisplayProperties.mQuality.QualityPreset2)
		{
			ApplyQualityPreset2( mDisplayProperties.mQuality,
				(EGraphicsQualityPresets) mDisplayProperties.mQuality.QualityPreset2->AsInt(), true );
		}
		else if (pThis == mGlobalResolution.mData.RenderResolution )
		{
			ApplyResolutionPreset( mGlobalResolution.mData.RenderResolution->AsInt() );
		}
		else if (pThis == mDisplayProperties.mQuality.DisplayDepth)
		{
			mNeedToRender = true;
		}
	}

	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}