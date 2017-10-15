
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orcustommanager_Physics_manager.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "orcustommanager_physics_manager.h"
#include "moPhysics_solver.h"


#include "GL\glew.h"

//--- Registration defines
#define ORCUSTOMMANAGER_TEMPLATE__CLASS ORCUSTOMMANAGER_TEMPLATE__CLASSNAME
#define ORCUSTOMMANAGER_TEMPLATE__NAME  ORCUSTOMMANAGER_TEMPLATE__CLASSSTR

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation( ORCUSTOMMANAGER_TEMPLATE__CLASS  );  // Manager class name.
FBRegisterCustomManager( ORCUSTOMMANAGER_TEMPLATE__CLASS );         // Manager class name.

static void* AllocMemory (int sizeInBytes);
static void FreeMemory (void *ptr, int sizeInBytes);

/************************************************
 *  FiLMBOX Creation
 ************************************************/
bool ORCustomManager_Physics::FBCreate()
{
    return true;
}


/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void ORCustomManager_Physics::FBDestroy()
{
    // Free any user memory here.
}

#ifndef ORSDK2013

void ORCustomManager_Physics::OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent)
{
    FBEventEvalGlobalCallback lFBEvent(pEvent);
    switch(lFBEvent.GetTiming())
    {
    case kFBGlobalEvalCallbackBeforeRender:
        //
        // This callback occurs immediately after clearing GL back buffer in render pipeline.
        //

        break;
    case kFBGlobalEvalCallbackAfterRender:
		{
		//
        // This callback occurs just before swapping GL back/front buffers. 
        // User could do some special effect, HUD or buffer download (via PBO) here. 
        //

			// draw newton debugging
			FBConstraintSolver *pSolver = nullptr;

			FBScene *pScene = mSystem.Scene;
			for (int i=0; i<pScene->ConstraintSolvers.GetCount(); ++i)
			{
				pSolver = (FBConstraintSolver*) pScene->ConstraintSolvers.GetAt(i);

				if ( !FBIS(pSolver, MOPhysicsSolver) )
					pSolver = nullptr;
				else
					break;
			}
			if (pSolver == nullptr)
				break;
			
			
			FBCamera *pCamera = pScene->Renderer->CurrentCamera;

			if ( pCamera != nullptr && FBIS(pCamera, FBCameraSwitcher) )
				pCamera = ((FBCameraSwitcher*)pCamera)->CurrentCamera;

			if (pCamera == nullptr)
				break;

			FBMatrix projM, modelM;
			pCamera->GetCameraMatrix( projM, kFBProjection );
			pCamera->GetCameraMatrix( modelM, kFBModelView );

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadMatrixd( projM );

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadMatrixd( modelM );

			for (int i=0; i<pScene->ConstraintSolvers.GetCount(); ++i)
			{
				pSolver = (FBConstraintSolver*) pScene->ConstraintSolvers.GetAt(i);

				if ( !FBIS(pSolver, MOPhysicsSolver) )
					continue;

				MOPhysicsSolver *pPhysics = (MOPhysicsSolver*) pSolver;
					
				if (pPhysics->WorldIsReady && pPhysics->Active && pPhysics->DisplayDebug) 
					pPhysics->DrawDebug();
			}

			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			

		} break;
    default:
        break;
    }
}

#endif

/************************************************
 *  Execution callback.
 ************************************************/
bool ORCustomManager_Physics::Init()
{
    return true;
}

bool ORCustomManager_Physics::Open()
{
	//PhysicsGlobalInit();

	// Register callback function for critical timings at the pipeline.
    //FBEvaluateManager::TheOne().OnSynchronizationEvent   .Add(this, (FBCallback)&ORCustomManager_Physics::OnPerFrameSynchronizationCallback);
    //FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Add(this, (FBCallback)&ORCustomManager_Physics::OnPerFrameEvaluationPipelineCallback);
    
#ifndef ORSDK2013
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Add(this, (FBCallback)&ORCustomManager_Physics::OnPerFrameRenderingPipelineCallback);
#endif

    return true;
}

bool ORCustomManager_Physics::Clear()
{
    return true;
}

bool ORCustomManager_Physics::Close()
{
	//FBEvaluateManager::TheOne().OnSynchronizationEvent   .Remove(this, (FBCallback)&ORCustomManager_Physics::OnPerFrameSynchronizationCallback);
    //FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Remove(this, (FBCallback)&ORCustomManager_Physics::OnPerFrameEvaluationPipelineCallback);
    
#ifndef ORSDK2013
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Remove(this, (FBCallback)&ORCustomManager_Physics::OnPerFrameRenderingPipelineCallback);
#endif

	//PhysicsGlobalClose();

    return true;
}
