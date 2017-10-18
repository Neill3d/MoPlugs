
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_content_events.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_content.h"


void CGPUFBScene::EventSceneChange( HISender pSender, HKEvent pEvent )
{
	/*
    if (! CacheSceneGraph)
        return;
    */
    FBEventSceneChange lEvent( pEvent );

    switch( lEvent.Type )
    {
    case kFBSceneChangeLoadBegin:
    case kFBSceneChangeClearBegin:
    case kFBSceneChangeMergeTransactionBegin:
        {
            // Lots of changes coming, stop incremental scene graph cache update. 
            //CacheSceneReset();
			//mWait = true;

			mLockSmallEvents = true;

			mTexturesInspector.NeedFullUpdate();
			mShadersInspector.NeedFullUpdate();
			mMaterialsInspector.NeedFullUpdate();
			mModelsInspector.NeedFullUpdate();
        }
        break;
		
	case kFBSceneChangeLoadEnd:
    case kFBSceneChangeClearEnd:
    case kFBSceneChangeMergeTransactionEnd:
		{
			//mWait = false;
			//mTexturesInspector.NeedFullUpdate();
			mTexturesInspector.NeedFullUpdate();
			mShadersInspector.NeedFullUpdate();
			mMaterialsInspector.NeedFullUpdate();
			mModelsInspector.NeedFullUpdate();

			mLockSmallEvents = false;

		} break;
	}

	if (false == mLockSmallEvents)
	{
		switch( lEvent.Type )
		{
		case kFBSceneChangeTransactionBegin:
		case kFBSceneChangeTransactionEnd:
			{
				// There are medium volume of changes coming, User could choose either full or incremental rebuild.
				//CacheSceneReset();

				if ( FBIS(lEvent.Component, FBTexture) )
				{
					FBTexture *ptexture = (FBTexture*) (FBComponent*)lEvent.Component;

					mTexturesInspector.NeedUpdate( ptexture, false, false );
					mTexturesInspector.CheckAnimatedFlag( ptexture );
				}
				else if ( FBIS(lEvent.Component, FBShader) )
				{
					FBShader *pshader = (FBShader*) (FBComponent*)lEvent.Component;

					mShadersInspector.NeedUpdate( pshader, false, false );
					mShadersInspector.CheckAnimatedFlag( pshader );
				}
				else if ( FBIS(lEvent.Component, FBMaterial) )
				{
					FBMaterial *pmat = (FBMaterial*) (FBComponent*)lEvent.Component;

					mMaterialsInspector.NeedUpdate( pmat, false, false );
					mMaterialsInspector.CheckAnimatedFlag( pmat );
				}
				else if ( FBIS(lEvent.Component, FBModel) )
				{
					FBModel *pmodel = (FBModel*) (FBComponent*)lEvent.Component;

					mModelsInspector.NeedUpdate( pmodel, false, true );
					mModelsInspector.CheckAnimatedFlag( pmodel );
				}
			}
			break;
		case kFBSceneChangeAttach:
			{
				if (FBIS(lEvent.Component, FBScene) && FBIS(lEvent.ChildComponent, FBTexture) )
				{
					FBTexture *ptexture = (FBTexture*) (FBComponent*)lEvent.ChildComponent;
					mTexturesInspector.Add( ptexture );
				}
				else if (FBIS(lEvent.Component, FBScene) && FBIS(lEvent.ChildComponent, FBShader) )
				{
					FBShader *pshader = (FBShader*) (FBComponent*)lEvent.ChildComponent;
					mShadersInspector.Add( pshader );
				}
				else if (FBIS(lEvent.Component, FBScene) && FBIS(lEvent.ChildComponent, FBMaterial) )
				{
					FBMaterial *pmat = (FBMaterial*) (FBComponent*)lEvent.ChildComponent;
					mMaterialsInspector.Add( pmat );
				}
				else if (FBIS(lEvent.Component, FBScene) && FBIS(lEvent.ChildComponent, FBModel) )
				{
					FBModel *pmodel = (FBModel*) (FBComponent*)lEvent.ChildComponent;
					mModelsInspector.Add( pmodel );
				}
			}
			break;
		case kFBSceneChangeDetach:
			{

				if (FBIS(lEvent.Component, FBScene) && FBIS(lEvent.ChildComponent, FBTexture) )
				{
					FBTexture *ptexture = (FBTexture*) (FBComponent*)lEvent.ChildComponent;
					mTexturesInspector.Delete( ptexture );
				}
				else if (FBIS(lEvent.Component, FBScene) && FBIS(lEvent.ChildComponent, FBShader) )
				{
					FBShader *pshader = (FBShader*) (FBComponent*)lEvent.ChildComponent;
					mShadersInspector.Delete( pshader );
				}
				else if (FBIS(lEvent.Component, FBScene) && FBIS(lEvent.ChildComponent, FBMaterial) )
				{
					FBMaterial *pmat = (FBMaterial*) (FBComponent*)lEvent.ChildComponent;
					mMaterialsInspector.Delete( pmat );
				}
				else if (FBIS(lEvent.Component, FBScene) && FBIS(lEvent.ChildComponent, FBModel) )
				{
					FBModel *pmodel = (FBModel*) (FBComponent*)lEvent.ChildComponent;
					mModelsInspector.Delete( pmodel );
				}
			}
			break;
		default:
			break;
		}
	}
}

void CGPUFBScene::EventConnNotify(HISender pSender, HKEvent pEvent)
{
	/*
    if (! CacheSceneGraph || mNeedFullRestructure )
        return;
		*/
    FBEventConnectionNotify		lEvent(pEvent);
    FBString					lText;
    char						lTmp[32];
    FBPlug*						lPlug;
    static int					lCounter = 0;

	/*
    sprintf( lTmp, "%d", lCounter );
    lText += lTmp;
    lText += ": Action=";
    lText += FBGetConnectionActionString(lEvent.Action);
	*/
	if (lEvent.Action == kFBConnectedSrc && lEvent.SrcPlug && lEvent.DstPlug)
	{
		if (lEvent.SrcPlug->Is(FBComponent::TypeInfo) && FBIS(lEvent.SrcPlug, FBTexture)
			&& lEvent.DstPlug->Is(FBProperty::TypeInfo) && FBIS(lEvent.DstPlug->GetOwner(), FBMaterial) )
		{
			mMaterialsInspector.AttachTexture( (FBMaterial*)(FBPlug*)lEvent.DstPlug->GetOwner(), (FBTexture*)(FBPlug*)lEvent.SrcPlug );
		}
		else if (lEvent.SrcPlug->Is(FBComponent::TypeInfo) && FBIS(lEvent.SrcPlug, FBMaterial)
			&& lEvent.DstPlug->Is(FBComponent::TypeInfo) && FBIS(lEvent.DstPlug, FBModel) )
		{
			mModelsInspector.AttachMaterial( (FBModel*)(FBPlug*)lEvent.DstPlug, (FBMaterial*)(FBPlug*)lEvent.SrcPlug ); 
		}
		else if (lEvent.SrcPlug->Is(FBComponent::TypeInfo) && FBIS(lEvent.SrcPlug, FBShader)
			&& lEvent.DstPlug->Is(FBComponent::TypeInfo) && FBIS(lEvent.DstPlug, FBModel) )
		{
			mModelsInspector.AttachShader( (FBModel*)(FBPlug*)lEvent.DstPlug, (FBShader*)(FBPlug*)lEvent.SrcPlug ); 
		}

	}
	else if (lEvent.Action == kFBDisconnectedSrc && lEvent.SrcPlug && lEvent.DstPlug)
	{
		if (lEvent.SrcPlug->Is(FBComponent::TypeInfo) && FBIS(lEvent.SrcPlug, FBTexture)
			&& lEvent.DstPlug->Is(FBProperty::TypeInfo) && FBIS(lEvent.DstPlug->GetOwner(), FBMaterial) )
		{
			mMaterialsInspector.DetachTexture( (FBMaterial*)(FBPlug*)lEvent.DstPlug->GetOwner(), (FBTexture*)(FBPlug*)lEvent.SrcPlug );
		}
		else if (lEvent.SrcPlug->Is(FBComponent::TypeInfo) && FBIS(lEvent.SrcPlug, FBMaterial)
			&& lEvent.DstPlug->Is(FBComponent::TypeInfo) && FBIS(lEvent.DstPlug, FBModel) )
		{
			mModelsInspector.DetachMaterial( (FBModel*)(FBPlug*)lEvent.DstPlug, (FBMaterial*)(FBPlug*)lEvent.SrcPlug ); 
		}
		else if (lEvent.SrcPlug->Is(FBComponent::TypeInfo) && FBIS(lEvent.SrcPlug, FBShader)
			&& lEvent.DstPlug->Is(FBComponent::TypeInfo) && FBIS(lEvent.DstPlug, FBModel) )
		{
			mModelsInspector.DetachShader( (FBModel*)(FBPlug*)lEvent.DstPlug, (FBShader*)(FBPlug*)lEvent.SrcPlug ); 
		}
	}
	

	// shader info should collect some information

	mShadersInspector.EventConnNotify(pSender, pEvent);

}

void CGPUFBScene::EventConnStateNotify(HISender pSender, HKEvent pEvent)
{
	/*
    if (! CacheSceneGraph || mNeedFullRestructure )
        return;
		*/
	/*
    FBEventConnectionStateNotify	lEvent(pEvent);
    FBString						lText;
    char							lTmp[32];
    FBPlug*							lPlug;
    static int						lCounter = 0;

    sprintf( lTmp, "%d", lCounter );
    lText += lTmp;
    lText += ": Action=";
    lText += FBGetConnectionActionString(lEvent.Action);

    if (lEvent.Plug)
    {
        if (lEvent.Plug->Is(FBProperty::TypeInfo))
        {
            lPlug = lEvent.Plug->GetOwner();
            lText += ",  Plug=";
            lText += ((FBComponent*)lPlug)->Name;
            lText += ".";
            lPlug = lEvent.Plug;
            lText += ((FBProperty*)lPlug)->GetName();
        }
        else if (lEvent.Plug->Is(FBComponent::TypeInfo))
        {
            lPlug = lEvent.Plug;
            lText += ",  Plug=";
            lText += ((FBComponent*)lPlug)->Name;
        }
    }

    lText += ",  Data=";
    sprintf(lTmp, "0x%lX", lEvent.GetData());
    lText += lTmp;
    lText += ",  OldData=";
    sprintf(lTmp, "0x%lX", lEvent.GetOldData());
    lText += lTmp;
	*/
    //FBTrace(lText);
}

void CGPUFBScene::EventConnDataNotify(HISender pSender, HKEvent pEvent)
{
	/*
    if (! CacheSceneGraph || mNeedFullRestructure )
        return;
		*/
	/*
    FBEventConnectionDataNotify	lEvent(pEvent);
    FBString					lText;
    char						lTmp[32];
    FBPlug*						lPlug;
    static int					lCounter = 0;

    sprintf( lTmp, "%d", lCounter );
    lText += lTmp;
    lText += ": Action=";
    lText += FBGetConnectionActionString(lEvent.Action);
    if( lEvent.Plug )
    {
        if (lEvent.Plug->Is(FBProperty::TypeInfo))
        {
            lPlug = lEvent.Plug->GetOwner();
            lText += ",  Plug=";
            lText += ((FBComponent*)lPlug)->Name;
            lText += ".";
            lPlug = lEvent.Plug;
            lText += ((FBProperty*)lPlug)->GetName();

			lPlug = lEvent.Plug->GetOwner();
			if ( strstr(((FBComponent*)lPlug)->Name, "DefaultMaterial") == nullptr )
			{
				
				if (FBIS ( lPlug, FBTexture ) )
				{
					mTexturesInspector.NeedUpdate( (FBTexture*) lPlug, ((FBProperty*)(FBPlug*)lEvent.Plug)->IsAnimated() );
				}
			}
        }
        else if (lEvent.Plug->Is(FBComponent::TypeInfo))
        {
            lPlug = lEvent.Plug;
            lText += ",  Plug=";
            lText += ((FBComponent*)lPlug)->Name;

			if (FBIS ( lPlug, FBTexture ) )
			{
				mTexturesInspector.NeedUpdate( (FBTexture*) lPlug, false );
			}
        }
    }
    lText += ",  Data=";
    sprintf(lTmp, "0x%lX", lEvent.GetData());
    lText += lTmp;
    lText += ",  OldData=";
    sprintf(lTmp, "0x%lX", lEvent.GetOldData());
    lText += lTmp;
	*/

	FBEventConnectionDataNotify	lEvent(pEvent);
	FBPlug *eventPlug = lEvent.Plug;
	FBPlug *lPlug = nullptr;

	if( eventPlug && lEvent.Action == kFBCandidated )
    {
		
		if ( FBIS(eventPlug, FBProperty) && (false == ((FBProperty*) eventPlug)->GetPropertyFlag(kFBPropertyFlagHideProperty) ) )
        {
			lPlug = eventPlug->GetOwner();
			
			if (FBIS ( lPlug, FBModel ) )
			{
				
				//FBString plugClass( eventPlug->ClassName() );
				//FBString plugName ( ((FBProperty*) eventPlug)->GetName() );
				bool subdata = ( kFBPT_bool == ((FBProperty*)eventPlug)->GetPropertyType() );
				mModelsInspector.NeedUpdate( (FBModel*) lPlug, ((FBProperty*)eventPlug)->IsAnimated(), subdata );
			}
			else if (FBIS ( lPlug, FBMaterial ) )
			{
				mMaterialsInspector.NeedUpdate( (FBMaterial*) lPlug, ((FBProperty*)eventPlug)->IsAnimated(), false );
			}
			else if (FBIS ( lPlug, FBShader ) )
			{
				mShadersInspector.NeedUpdate( (FBShader*) lPlug, ((FBProperty*)eventPlug)->IsAnimated(), false );
			}
			else if (FBIS ( lPlug, FBTexture ) )
			{
				mTexturesInspector.NeedUpdate( (FBTexture*) lPlug, ((FBProperty*)eventPlug)->IsAnimated(), false );
			}
        }
        else if (eventPlug->Is(FBComponent::TypeInfo))
        {
            
			if (FBIS(eventPlug, FBModel) )
			{
				mModelsInspector.NeedUpdate( (FBModel*) eventPlug, false, false );
			}
			else if (FBIS(eventPlug, FBShader) )
			{
				mShadersInspector.NeedUpdate( (FBShader*) eventPlug, false, false );
			}
			else if (FBIS(eventPlug, FBMaterial) )
			{
				mMaterialsInspector.NeedUpdate( (FBMaterial*) eventPlug, false, false );
			}
			else if (FBIS ( eventPlug, FBTexture ) )
			{
				mTexturesInspector.NeedUpdate( (FBTexture*) eventPlug, false, false );
			}
        }
    }

	mShadersInspector.EventDataNotify(pSender, pEvent);
    //FBTrace(lText);
}



void CGPUFBScene::OnPerFrameSynchronizationCallback(HISender pSender, HKEvent pEvent)
{
    FBEventEvalGlobalCallback lFBEvent(pEvent);
    if (lFBEvent.GetTiming() == kFBGlobalEvalCallbackSyn)
    {
        ///
        // This callback occurs when both rendering and evaluation pipeline are stopped,
        // plugin developer could add some lightweight scene modification tasks here
        // and no need to worry complicated thread issues. 
        //
		const auto &shaderGroups = mShadersFactory.GetShaderTypesVector();

		for (auto iter=begin(shaderGroups); iter!=end(shaderGroups); ++iter)
		{
			CBaseShaderCallback *pShaderGroup = *iter;

			if (true == pShaderGroup->IsNeedToProcessFrameEvaluation() )
			{
				const int id = pShaderGroup->Id();
				auto &sortedVector = mShadersInspector.GetShaderGroupShaders(id);

				for (auto shaderIter=begin(sortedVector);
					shaderIter != end(sortedVector);
					++shaderIter )
				{
					pShaderGroup->FrameSync( *shaderIter );
				}
			}
		}
    }
}

void CGPUFBScene::OnPerFrameEvaluationPipelineCallback(HISender pSender, HKEvent pEvent)
{
    //
    // Add custom evaluation tasks here will improve the performance if with 
    // parallel pipeline (default) because they occur in the background thread.
    //

    FBEventEvalGlobalCallback lFBEvent(pEvent);
    switch(lFBEvent.GetTiming())
    {
    case kFBGlobalEvalCallbackBeforeDAG:
        //
        // We could add custom tasks here before MoBu perform standard evaluation tasks. 
        //

        break;
    case kFBGlobalEvalCallbackAfterDAG:
		{
        //
        // We could add custom tasks here after MoBu finish standard animation & deformation evaluation tasks. 
        //

		} break;
    case kFBGlobalEvalCallbackAfterDeform:
		{
        //
        // We could add custom tasks here after MoBu finish standard deformation evaluation tasks 
        // (if not use GPU deformation). E.g, update the accelerated spatial scene structure for rendering. 
        //
		
		//mGPUScene->UpdateAfterDeform();


			const auto &shaderGroups = mShadersFactory.GetShaderTypesVector();

			for (auto iter=begin(shaderGroups); iter!=end(shaderGroups); ++iter)
			{
				CBaseShaderCallback *pShaderGroup = *iter;

				if (true == pShaderGroup->IsNeedToProcessFrameEvaluation() )
				{
					const int id = pShaderGroup->Id();
					auto &sortedVector = mShadersInspector.GetShaderGroupShaders(id);

					for (auto shaderIter=begin(sortedVector);
						shaderIter != end(sortedVector);
						++shaderIter )
					{
						pShaderGroup->FrameEvaluation( *shaderIter );
					}
				}
			}

		} break;
    default:
        break;
    }    
}
