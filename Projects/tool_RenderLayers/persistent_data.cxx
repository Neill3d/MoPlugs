
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: persistent_data.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Class declaration
#include "persistent_data.h"

//--- FiLMBOX Registration & Implementation.
FBClassImplementation(  RenderLayersData    );
FBUserObjectImplement(  RenderLayersData,
                        "Class objects used to hold RenderLayers tool data.",
                        FB_DEFAULT_SDK_ICON     );                                          //Register UserObject class
FBElementClassImplementation( RenderLayersData, FB_DEFAULT_SDK_ICON );                  //Register to the asset system

///////////////////////////////////////////////////

extern bool	gDoRender;
extern void GlobalRunRender();
static bool g_retrieve = false;

/************************************************
 *  Constructor.
 ************************************************/
RenderLayersData::RenderLayersData( const char* pName, HIObject pObject ) 
	: FBUserObject( pName, pObject )
{
    FBClassInit;

	// base master layer

	Layers.Add( new LayerItem(true, "Master layer") );
	SetCurrentLayer(0);
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool RenderLayersData::FBCreate()
{
	FBApplication &lApp = FBApplication::TheOne();
	lApp.OnFileNew.Add( this, (FBCallback) &RenderLayersData::EventFileNew );
	//lApp.OnFileOpenCompleted.Add( this, (FBCallback) &RenderLayersData::EventFileOpenCompleted );
	
	//FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Add(this, (FBCallback)&RenderLayersData::EventFileOpenCompleted);
    
	

	return true;
}

/************************************************
 *  FiLMBOX Destructor.
 ************************************************/
void RenderLayersData::FBDestroy()
{
	FBApplication &lApp = FBApplication::TheOne();
	lApp.OnFileNew.Remove( this, (FBCallback) &RenderLayersData::EventFileNew );
	//lApp.OnFileOpenCompleted.Remove( this, (FBCallback) &RenderLayersData::EventFileOpenCompleted );

	//FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Remove(this, (FBCallback)&RenderLayersData::EventFileOpenCompleted);
}

bool RenderLayersData::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (pStoreWhat == kAttributes)
	{
		pFbxObject->FieldWriteBegin( "RenderLayers" );
		{
			// VERSION
			pFbxObject->FieldWriteI(10);

			// number of layers
			const int numberOfLayers = Layers.GetCount();

			pFbxObject->FieldWriteI( numberOfLayers );

			for (int i=0; i<numberOfLayers; ++i)
				Layers[i]->FbxStore( pFbxObject, pStoreWhat );
		}
		pFbxObject->FieldWriteEnd();
	}

    return true;
}

void RenderLayersData::Clear()
{
	const int numberOfLayers = Layers.GetCount();
	for (int i=0; i<numberOfLayers; ++i)
	{
		LayerItem *pItem = Layers[i];
		if (pItem)
		{
			delete pItem;
			pItem = nullptr;
		}
	}
	Layers.Clear();
}


bool RenderLayersData::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (pStoreWhat == kAttributes)
	{
		Clear();

		pFbxObject->FieldReadBegin( "RenderLayers" );
		{
			int version = pFbxObject->FieldReadI();
			
			if (version != 10) FBMessageBox( "Render layers", "incorrent retrieving file type", "Ok");
			else
			{
				int count = pFbxObject->FieldReadI();
			
				for (int i=0; i<count; ++i)
				{
					LayerItem *pLayer = new LayerItem();
					pLayer->FbxRetrieve( pFbxObject, pStoreWhat );
					Layers.Add(pLayer);
				}
			}
		}
		pFbxObject->FieldReadEnd();

		//
		AskMainUIForUpdate();

		g_retrieve = true;
		FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Add(this, (FBCallback)&RenderLayersData::EventFileOpenCompleted);
	}

    return true;
}

void RenderLayersData::SetCurrentLayer(const int layer)
{
	LayerItem *pItem = Layers[layer];
	gSetCurrentLayer( pItem );
	pItem->State.Restore();

	AddNewEvent( EVENT_LAYER_CHANGE );
}

void RenderLayersData::EventFileNew( HISender pSender, HKEvent pEvent )
{
	Clear();

	// base master layer

	Layers.Add( new LayerItem(true, "Master layer") );
	SetCurrentLayer(0);
	
    // Manage UI
	AskMainUIForUpdate();
}

void RenderLayersData::EventFileOpenCompleted( HISender pSender, HKEvent pEvent )
{
	if (g_retrieve)
	{
		const int count = Layers.GetCount();
		if (count == 0)
			Layers.Add( new LayerItem(true, "Master layer") );
		else
		{
			// restore group and cameras list
			for (int i=0; i<count; ++i)
			{
				LayerItem *pItem = Layers[i];
				pItem->FbxRetrieve(nullptr, kCleanup);
			}
		}

		SetCurrentLayer(0);
		//
		AskMainUIForUpdate();

		g_retrieve = false;
		FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Remove(this, (FBCallback)&RenderLayersData::EventFileOpenCompleted);

		if (gDoRender)
		{
			FBPopNormalTool( "Render Layers" );
		}
		else
		{
			GlobalRunRender();
		}
	}
}