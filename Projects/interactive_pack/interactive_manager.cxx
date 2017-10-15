
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: interactive_manager.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "interactive_manager.h"
#include "Common.h"

//--- Registration defines
#define ORCUSTOMMANAGER_INTERACTIVE__CLASS ORCUSTOMMANAGER_INTERACTIVE__CLASSNAME
#define ORCUSTOMMANAGER_INTERACTIVE__NAME  ORCUSTOMMANAGER_INTERACTIVE__CLASSSTR

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation( ORCUSTOMMANAGER_INTERACTIVE__CLASS  );  // Manager class name.
FBRegisterCustomManager( ORCUSTOMMANAGER_INTERACTIVE__CLASS );         // Manager class name.

/************************************************
 *  FiLMBOX Creation
 ************************************************/
bool ORCustomManager_Interactive::FBCreate()
{
    return true;
}


/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void ORCustomManager_Interactive::FBDestroy()
{
    // Free any user memory here.
}


/************************************************
 *  Execution callback.
 ************************************************/
bool ORCustomManager_Interactive::Init()
{
    return true;
}

bool ORCustomManager_Interactive::Open()
{
	InitCameraShake();

	//mSystem.OnConnectionDataNotify.Add( this, (FBCallback) &ORCustomManager_Interactive::OnConnectionData );
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Add(this, (FBCallback)&ORCustomManager_Interactive::OnRender);
	mSystem.Scene->OnChange.Add( this, (FBCallback) &ORCustomManager_Interactive::OnSceneChange );
	mSystem.OnUIIdle.Add( this, (FBCallback) &ORCustomManager_Interactive::OnSystemIdle );
    return true;
}

bool ORCustomManager_Interactive::Clear()
{
    return true;
}

bool ORCustomManager_Interactive::Close()
{
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Remove(this, (FBCallback)&ORCustomManager_Interactive::OnRender);
	//mSystem.OnConnectionDataNotify.Remove( this, (FBCallback) &ORCustomManager_Interactive::OnConnectionData );
	mSystem.Scene->OnChange.Remove( this, (FBCallback) &ORCustomManager_Interactive::OnSceneChange );
	mSystem.OnUIIdle.Remove( this, (FBCallback) &ORCustomManager_Interactive::OnSystemIdle );
    return true;
}

void ORCustomManager_Interactive::OnSystemIdle(HISender pSender, HKEvent pEvent)
{
	const char *szFilename = GetNextFile();
	if (szFilename && szFilename[0] != 0)
	{
		FBApplication().FileNew();
		FBApplication().FileOpen(szFilename);

		NextFileOpened();
	}
}

void ORCustomManager_Interactive::OnSceneChange(HISender pSender, HKEvent pEvent)
{
	FBEventSceneChange	levent(pEvent);

	FBSceneChangeType	type = levent.Type;
	/*
	if (FBIS(levent.ChildComponent, FBStoryClip) )
	{
		printf( "%d\n", type );
	}
	if (FBIS(levent.Component, FBStoryClip) )
	{
		printf( "%d\n", type );
	}
	*/
	switch(type)
	{
	case kFBSceneChangeAddChild:
		if ( FBIS(levent.ChildComponent, FBStoryClip) )
		{
			CheckCommandReferences();
		}
		break;
	case kFBSceneChangeRemoveChild:
		if ( FBIS(levent.ChildComponent, FBStoryClip) )
		{
			FBStoryClip *pClip = (FBStoryClip*) (FBComponent*) levent.ChildComponent;

			int count = pClip->PropertyList.GetCount();
			for (int i=count-1; i>=0; --i)
			{
				FBProperty *prop = pClip->PropertyList[i];
				if (prop->IsReferenceProperty() )
				{
					pClip->PropertyRemove(prop);
				}
			}
		}
		break;
	}
}

void ORCustomManager_Interactive::OnConnectionData(HISender pSender, HKEvent pEvent)
{
	FBEventConnectionDataNotify levent(pEvent);

	if ( FBIS(levent.Plug, FBProperty) && FBIS(levent.Plug->GetOwner(), FBStoryClip) )
	{
		FBStoryClip *pClip = (FBStoryClip*) levent.Plug->GetOwner();
		
		printf( "event plug - %s\n", (const char*) pClip->Name );

		FBProperty *prop = pClip->PropertyList.Find( "ClipName" );
		if (prop && levent.Plug == prop)
		{
			pClip->Name = prop->AsString();
		}
		//
		prop = pClip->PropertyList.Find( "Color Mode" );
		if (prop && levent.Plug == prop)
		{
			int colormode = prop->AsInt();

			prop = pClip->PropertyList.Find("ClipColor");
			if (prop)
			{
				FBColor &color = GetCommandColor(colormode);
				prop->SetData( &color );
			}
		}
	}
}




void ORCustomManager_Interactive::OnRender(HISender pSender, HKEvent pEvent)
{
	FBEventEvalGlobalCallback lFBEvent(pEvent);
    switch(lFBEvent.GetTiming())
    {
    case kFBGlobalEvalCallbackBeforeRender:
		{
			// apply shake effect
			FBCamera *pCamera = mSystem.Renderer->CurrentCamera;
			
			FBTime currTime(mSystem.SystemTime);
			double currTimeValue = currTime.GetSecondDouble();
			
			if (pCamera)
			{
				if ( FBIS(pCamera, FBCameraSwitcher) )
					pCamera = ((FBCameraSwitcher*) pCamera)->CurrentCamera;
						
				RenderCameraShake( currTimeValue, pCamera );
				RenderCameraZoom( currTimeValue, pCamera );
			}

		} break;
    case kFBGlobalEvalCallbackAfterRender:
		{
			//
			// This callback occurs just before swapping GL back/front buffers. 
			// User could do some special effect, HUD or buffer download (via PBO) here. 
			//

		} break;

    default:
        break;
    }
}