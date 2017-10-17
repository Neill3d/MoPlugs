
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: renderLayers_tool.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// DONE: catch Group delete EVENT to free set in layer data

// TODO: check if camera from the options list is still exist, check in delete EVENT !!!
// DONE: connect group to render

// 28.03.2013 FIXED: crashes on render (metter shaders arrays, 1 level - set count, 2 level set count (items per block is zero), 
//											cause not constructor execution from the first setcount)


//--- Class declaration
#include "renderlayers_tool.h"
#include <Windows.h>

#include "resource.h"
#include "resourceUtils.h"

//--- Registration defines
#define RENDERLAYERS__CLASS	RENDERLAYERS__CLASSNAME
#define RENDERLAYERS__LABEL	"Render Layers"
#define RENDERLAYERS__DESC	"Manager render layers"

//--- Implementation and registration
FBToolImplementation(	RENDERLAYERS__CLASS	);
FBRegisterTool		(	RENDERLAYERS__CLASS,
						RENDERLAYERS__LABEL,
						RENDERLAYERS__DESC,
						FB_DEFAULT_SDK_ICON	);	// Icon filename (default=Open Reality icon)

// stats captions
#define TOTALFRAMECOUNT					"Total Frame Count: "
#define REMAININGFRAMECOUNT				"Remaining Frame Count: "
#define	TOTALTIMEELAPSED				"Total Time Elapsed: "
#define	ESTIMATEDTIME					"Estimated Time: "
#define ESTIMATEDTIMEREMAINING			"Estimated Time Remaining: "
#define	TIMEPERFRAME					"Time Per Frame: "

FBProgress		*g_pProgress = nullptr;

extern bool			gDoRender;
extern bool			gClose;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////// TOOL RENDER LAYERS

/************************************************
 *	Constructor.
 ************************************************/
bool ToolRenderLayers::FBCreate()
{
	StartSize[0] = 240;
	StartSize[1] = 360;
	MinSize[0]=240;
	MinSize[1]=360;
	MaxSize[0]=240;
	MaxSize[1]=360;

	mReadingState = false;

	mSelectedNode = nullptr;
	mCurrentLayer = 0;

    // Manage UI
	UICreate	();
	UIConfigure	();
	UIReset		();
	
	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ToolRenderLayers::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &ToolRenderLayers::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ToolRenderLayers::EventToolResize		);
	OnPaint.Add	( this, (FBCallback) &ToolRenderLayers::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ToolRenderLayers::EventToolInput		);

	
	mSystem.Scene->OnChange.Add( this, (FBCallback) &ToolRenderLayers::EventSceneChange );
	//mApplication.OnFileNew.Add( this, (FBCallback) &ToolRenderLayers::EventFileNew );
	//mApplication.OnFileOpenCompleted.Add( this, (FBCallback) &ToolRenderLayers::EventFileOpenCompleted );

	return true;
}

/************************************************
 *	Destruction function.
 ************************************************/
void ToolRenderLayers::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ToolRenderLayers::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ToolRenderLayers::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ToolRenderLayers::EventToolPaint	);
	OnInput.Remove	( this, (FBCallback) &ToolRenderLayers::EventToolInput	);
	OnResize.Remove	( this, (FBCallback) &ToolRenderLayers::EventToolResize	);

	mSystem.Scene->OnChange.Remove( this, (FBCallback) &ToolRenderLayers::EventSceneChange );
	//mApplication.OnFileNew.Remove( this, (FBCallback) &ToolRenderLayers::EventFileNew );
	//mApplication.OnFileOpenCompleted.Remove( this, (FBCallback) &ToolRenderLayers::EventFileOpenCompleted );

	// Free user allocated memory
	Clear();
}

void ToolRenderLayers::Clear()
{
	RenderLayersData *pLayersData = FindPersistentData(true);
	if (pLayersData)
	{
		pLayersData->Clear();
	}
}

/************************************************
 *	Create, configure & reset UI.
 ************************************************/
void ToolRenderLayers::UICreate()
{
	int bw = 30;
	int bh = 25;

	int tw = 230;
	int th = 150;

	// add regions
	AddRegion( "ButtonAdd",	"ButtonAdd",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			bw,		kFBAttachNone,		"",		1.0,
			bh,		kFBAttachNone,		"",		1.0 );
	AddRegion( "ButtonRemove",	"ButtonRemove",
			5,		kFBAttachRight,		"ButtonAdd",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			bw,		kFBAttachNone,		"",		1.0,
			bh,		kFBAttachNone,		"",		1.0 );
	AddRegion( "ButtonUp",	"ButtonUp",
			5,		kFBAttachRight,		"ButtonRemove",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			bw,		kFBAttachNone,		"",		1.0,
			bh,		kFBAttachNone,		"",		1.0 );
	AddRegion( "ButtonDown",	"ButtonDown",
			5,		kFBAttachRight,		"ButtonUp",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			bw,		kFBAttachNone,		"",		1.0,
			bh,		kFBAttachNone,		"",		1.0 );
	AddRegion( "ButtonProperties",	"ButtonProperties",
			5,		kFBAttachRight,		"ButtonDown",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			bw,		kFBAttachNone,		"",		1.0,
			bh,		kFBAttachNone,		"",		1.0 );
	AddRegion( "ButtonAssignSet",	"ButtonAssignSet",
			5,		kFBAttachRight,		"ButtonProperties",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			bw,		kFBAttachNone,		"",		1.0,
			bh,		kFBAttachNone,		"",		1.0 );
	AddRegion( "ButtonClearSet",	"ButtonClearSet",
			5,		kFBAttachRight,		"ButtonAssignSet",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			bw,		kFBAttachNone,		"",		1.0,
			bh,		kFBAttachNone,		"",		1.0 );

	AddRegion( "Label",	"Label",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachBottom,		"ButtonAdd",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			bh,		kFBAttachNone,		"",		1.0 );
	AddRegion( "Label2",	"Label2",
			5,		kFBAttachRight,		"Label",		1.0,
			5,		kFBAttachBottom,		"ButtonAdd",		1.0,
			tw,		kFBAttachNone,		"",		1.0,
			bh,		kFBAttachNone,		"",		1.0 );
	AddRegion( "Tree",	"Tree",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachBottom,		"Label",		1.0,
			tw,		kFBAttachNone,		"",		1.0,
			th,		kFBAttachNone,		"",		1.0 );
	AddRegion( "Debug",	"Debug",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachBottom,	"Tree",		1.0,
			tw,		kFBAttachNone,		"",		1.0,
			-40,	kFBAttachBottom,	"",		1.0 );


	// Assign regions
	SetControl("ButtonAdd", mButtonAdd);
	SetControl("ButtonRemove", mButtonRemove);
	SetControl("ButtonUp", mButtonUp);
	SetControl("ButtonDown", mButtonDown);
	SetControl("ButtonProperties", mButtonProperties);
	//SetControl("ButtonAssignSet", mButtonAssignSet);
	SetControl("ButtonClearSet", mButtonMenu );	// mButtonClearSet
	SetControl("Tree", mLayers);
	SetControl("Label", mLabel);
	SetControl("Label2", mLabel2);
	SetControl("Debug", mDebugList);

	//
	// Popup dialogs
	//
	
	mDialogSets.AddRegion( "List",	"List",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			-30,	kFBAttachBottom,	"",		1.0,
			-5,		kFBAttachRight,		"",		1.0 );
	mDialogSets.AddRegion( "ButtonOk",	"ButtonOk",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachBottom,	"List",		1.0,
			20,		kFBAttachBottom,	"",		1.0,
			100,	kFBAttachNone,		"",		1.0 );
	mDialogSets.AddRegion( "ButtonCancel",	"ButtonCancel",
			5,		kFBAttachRight,		"ButtonOk",		1.0,
			5,		kFBAttachBottom,	"List",		1.0,
			20,		kFBAttachBottom,	"",		1.0,
			100,	kFBAttachNone,		"",		1.0 );

	mDialogSets.SetControl("ButtonOk", mButtonDialogOk);
	mDialogSets.SetControl("ButtonCancel", mButtonDialogCancel);
	mDialogSets.SetControl("List", mListDialogSets);

	//
	
	mDialogState.AddRegion( "List",	"List",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachTop,		"",		1.0,
			-5,		kFBAttachRight,	"",		1.0,
			-30,	kFBAttachBottom,		"",		1.0 );
	mDialogState.AddRegion( "ButtonOk",	"ButtonOk",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachBottom,	"List",		1.0,
			100,	kFBAttachNone,	"",		1.0,
			-5,		kFBAttachBottom,		"",		1.0 );

	mDialogState.SetControl("ButtonOk", mButtonStateOk);
	mDialogState.SetControl("List", mListState);

	// render elements

	// add regions
	
	AddRegion( "ButtonRender",	"ButtonRender",
			5,		kFBAttachLeft,		"",		1.0,
			-35,	kFBAttachBottom,	"",		1.0,
			110,	kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	AddRegion( "ButtonAbout",	"ButtonAbout",
			5,		kFBAttachRight,		"ButtonRender",		1.0,
			-35,	kFBAttachBottom,	"",		1.0,
			110,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );

	// Assign regions
	SetControl( "ButtonRender", mButtonRender );
	SetControl( "ButtonAbout", mButtonAbout );

	//
	// stats
	//
	mDialogStats.Region.X = 100;
	mDialogStats.Region.Y = 100;
	mDialogStats.Region.Width = 220;
	mDialogStats.Region.Height = 200;

	mDialogStats.AddRegion( "Settings",	"Settings",
			5,			kFBAttachLeft,		"",		1.0,
			5,			kFBAttachNone,		"",		1.0,
			200,		kFBAttachNone,		"",		1.0,
			150,		kFBAttachNone,		"",		1.0 );

	mLayoutSettings.AddRegion( "TotalFrameCount",	"TotalFrameCount",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	mLayoutSettings.AddRegion( "RemainingFrameCount",	"RemainingFrameCount",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachBottom,		"TotalFrameCount",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	mLayoutSettings.AddRegion( "TotalTimeElapsed",	"TotalTimeElapsed",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachBottom,		"RemainingFrameCount",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	mLayoutSettings.AddRegion( "EstimatedTime",	"EstimatedTime",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachBottom,		"TotalTimeElapsed",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	mLayoutSettings.AddRegion( "EstimatedTimeRemaining",	"EstimatedTimeRemaining",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachBottom,		"EstimatedTime",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	mLayoutSettings.AddRegion( "TimePerFrame",	"TimePerFrame",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachBottom,		"EstimatedTimeRemaining",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );

	mDialogStats.SetControl( "Settings", mLayoutSettings );
	mLayoutSettings.SetControl( "TotalFrameCount", mTotalFrameCount );
	mLayoutSettings.SetControl( "RemainingFrameCount", mRemainingFrameCount );
	mLayoutSettings.SetControl( "TotalTimeElapsed", mTotalTimeElapsed );
	mLayoutSettings.SetControl( "EstimatedTime", mEstimatedTime );
	mLayoutSettings.SetControl( "EstimatedTimeRemaining", mEstimatedTimeRemaining );
	mLayoutSettings.SetControl( "TimePerFrame", mTimePerFrame );

	//
	// context menu
	//
	mContextMenu.InsertLast( "Store Layer State", 0 );
	mContextMenu.InsertLast( "Clear Layer State", 1 );
	mContextMenu.InsertLast( "Show State Info", 2 );
	mContextMenu.InsertLast( "-", 3 );
	mContextMenu.InsertLast( "Assign Group to Layer", 4 );
	mContextMenu.InsertLast( "Clear Layer Group Assignment", 5 );
	mContextMenu.OnMenuActivate.Add( this, (FBCallback) &ToolRenderLayers::OnMenuActivate );

	//mLayers.AssignMenu(&mContextMenu);
}

/************************************************
 *	config UI (params, events)
 ************************************************/
void ToolRenderLayers::UIConfigure()
{
	mButtonAdd.Caption = "Add";
	mButtonAdd.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonAdd );
	mButtonRemove.Caption = "Rmv";
	mButtonRemove.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonRemove );
	mButtonUp.Caption = "Up";
	mButtonUp.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonUp );
	mButtonDown.Caption = "Down";
	mButtonDown.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonDown );
	mButtonProperties.Caption = "Prs";
	mButtonProperties.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonProperties );
	mButtonAssignSet.Caption = "Grp";
	mButtonAssignSet.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonAssignSet );
	mButtonClearSet.Caption = "ClrG";
	mButtonClearSet.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonClearSet );
	mButtonMenu.Caption = ">>";
	mButtonMenu.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonMenu );

	mLayers.CheckBoxes = true;
	mLayers.ShowLines = true;
	mLayers.OnClickCheck.Add( this, (FBCallback) &ToolRenderLayers::EventTreeCheck );
	mLayers.OnDblClick.Add( this, (FBCallback) &ToolRenderLayers::EventTreeDblClk );
	mLayers.OnSelect.Add( this, (FBCallback) &ToolRenderLayers::EventTreeSelect );
	//mLayers.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventTreeClick );

	mLabel.Caption = "Current layer: ";
	mLabel2.Style = kFBTextStyleBold;

	//
	// dialog sets
	//
	mButtonDialogOk.Caption = "Ok";
	mButtonDialogOk.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonDialogOk );
	mButtonDialogCancel.Caption = "Cancel";
	mButtonDialogCancel.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonDialogCancel );

	mDialogSets.Region.X = 100;
	mDialogSets.Region.Y = 100;
	mDialogSets.Region.Width = 100;
	mDialogSets.Region.Height = 100;

	mDebugList.Style = kFBVerticalList;

	//

	mButtonStateOk.Caption = "Ok";
	mButtonStateOk.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonStateOk );

	mDialogState.Region.X = 100;
	mDialogState.Region.Y = 100;
	mDialogState.Region.Width = 300;
	mDialogState.Region.Height = 600;

	mListState.Style = kFBVerticalList;

	//
	// rendering componets
	//
	mButtonRender.Caption = "Render Movie!";
	mButtonRender.OnClick.Add( this, (FBCallback) &ToolRenderLayers::EventButtonRender );

	mButtonAbout.Caption = "About";
	mButtonAbout.OnClick.Add(this, (FBCallback) &ToolRenderLayers::EventButtonAbout );
}


void ToolRenderLayers::UIReset()
{
	mLayers.Clear();

	RenderLayersData *pLayersData = FindPersistentData(true);
	if (pLayersData == nullptr) return;

	const int numberOfLayers = pLayersData->Layers.GetCount();

	for (int i=numberOfLayers-1; i>=0; --i)
	{
		LayerItem *pItem = pLayersData->Layers[i];

		pItem->Node = mLayers.InsertLast(mLayers.GetRoot(), pItem->Name);
		pItem->Node->Checked = pItem->Renderable;
		pItem->Node->Reference = i+1;
		
		if (pItem->Group != nullptr)
		{
			FBTreeNode *pNode = mLayers.InsertLast(pItem->Node, pItem->Group->Name);
			if (pNode)
				pNode->Reference = i+1;
		}
		if (pItem->IsStateEmpty() == false)
		{
			FBTreeNode *pNode = mLayers.InsertLast(pItem->Node, "State Holder");
			if (pNode)
				pNode->Reference = i+1;
		}
	}

	//mLayers.SelectedNodes.Add( mLayersData[mCurrentLayer].Node );
	mLabel2.Caption = pLayersData->Layers[mCurrentLayer]->Name;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// SetCurrentLayer

void ToolRenderLayers::SetCurrentLayer(int layer)
{
	mReadingState = true;
	{
		RenderLayersData *pLayersData = FindPersistentData(true);
		if (pLayersData)
		{
			LayerItem *pItem = pLayersData->Layers[layer];
			mLabel2.Caption = pItem->Name;

			mCurrentLayer = layer;
			gSetCurrentLayer( pItem );

			pItem->State.Restore();
		}
		AddNewEvent( EVENT_LAYER_CHANGE );
	}
	mReadingState = false;
}

void ToolRenderLayers::FindSelectedLayerData(LayerItem * &pItem)
{
	pItem = nullptr;

#ifdef OLD_FBTREE
	if (mSelectedNode && mSelectedNode->Reference > 0)
	{
		int j = mSelectedNode->Reference - 1;
		pItem = mLayersData[j];
	}
#else
	for (int i=0; i<mLayers.SelectedNodes.GetCount(); ++i)
		if (mLayers.SelectedNodes[i]->Reference > 0)
		{
			int j = mLayers.SelectedNodes[i]->Reference - 1;
			
			RenderLayersData *pLayersData = FindPersistentData(true);
			if (pLayersData) pItem = pLayersData->Layers[j];
			
			return;
		}
#endif
}

/************************************************
 *	Refresh grabbing statistics
 ************************************************/
void ToolRenderLayers::RunRefreshStats(void *pThis)
{
	ToolRenderLayers* lThis = (ToolRenderLayers*)pThis;
	if (lThis) lThis->RefreshStats();
	
}

void ToolRenderLayers::RefreshStats()
{
	FBTime		lTime;
	mStats = VideoGrabber.GetStatistics();
	char	text[256];
	
	sprintf( text, "%s %d", TOTALFRAMECOUNT, mStats.mTotalFrameCount );
	mTotalFrameCount.Caption = text;
	
	sprintf( text, "%s %d", REMAININGFRAMECOUNT, mStats.mRemainingFrameCount );
	mRemainingFrameCount.Caption = text;
	mRemainingFrameCount.Refresh(true);

	lTime = mStats.mTotalTimeElapsed;
	sprintf( text, "%s %-10.2lf", TOTALTIMEELAPSED, lTime.GetSecondDouble() );
	mTotalTimeElapsed.Caption = text;
	mTotalTimeElapsed.Refresh(true);

	lTime = mStats.mEstimatedTime;
	sprintf( text, "%s %-10.2lf", ESTIMATEDTIME, lTime.GetSecondDouble() );
	mEstimatedTime.Caption = text;
	mEstimatedTime.Refresh(true);

	lTime = mStats.mEstimatedTimeRemaining;
	sprintf( text, "%s %-10.2lf", ESTIMATEDTIMEREMAINING, lTime.GetSecondDouble() );
	mEstimatedTimeRemaining.Caption = text;
	mEstimatedTimeRemaining.Refresh(true);

	lTime = mStats.mTimePerFrame;
	sprintf( text, "%s %-10.2lf", TIMEPERFRAME, lTime.GetSecondDouble() );
	mTimePerFrame.Caption = text;
	mTimePerFrame.Refresh(true);

	// update progress bar (if exists)
	if (g_pProgress)
	{
		double percent = 100.0 * (mStats.mTotalFrameCount - mStats.mRemainingFrameCount) / mStats.mTotalFrameCount;
		g_pProgress->Percent = (int) percent;
		g_pProgress->Refresh(true);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// EVENT SCENE CHANGE

void ToolRenderLayers::EventFileNew( HISender pSender, HKEvent pEvent )
{
	Clear();

	// base master layer
	RenderLayersData *pLayersData = FindPersistentData(true);
	if (pLayersData)
	{
		pLayersData->Layers.Add( new LayerItem(true, "Master layer") );
		SetCurrentLayer(0);
	}
    // Manage UI
	UIReset		();

}

void ToolRenderLayers::EventFileOpenCompleted( HISender pSender, HKEvent pEvent )
{
	RenderLayersData *pLayersData = FindPersistentData(true);
	if (!pLayersData) return;

	const int count = pLayersData->Layers.GetCount();
	if (count == 0)
		pLayersData->Layers.Add( new LayerItem(true, "Master layer") );
	else
	{
		// restore group and cameras list
		for (int i=0; i<count; ++i)
		{
			LayerItem *pItem = pLayersData->Layers[i];
			pItem->FbxRetrieve(nullptr, kCleanup);
		}
	}

	SetCurrentLayer(0);
	UIReset();

	mReadingState = false;
}

void ToolRenderLayers::EventSceneChange( HISender pSender, HKEvent pEvent )
{
	FBEventSceneChange	sceneEvent(pEvent);
	int type = sceneEvent.Type;

/*
	mDebugList.Items.Add( ConvertTypeToString(sceneEvent.Type) + " - " + ((sceneEvent.Component) ? sceneEvent.Component->Name : "___") 
		+ "; " + ((sceneEvent.ChildComponent) ? sceneEvent.ChildComponent->Name : "____"));
*/

	RenderLayersData *pLayersData = FindPersistentData(false);
	if (!pLayersData) return;

	const int count = pLayersData->Layers.GetCount();

	if (type == kFBSceneChangeRemoveChild)
	{
		if (sceneEvent.Component->Is( FBGroup::TypeInfo ) )
		{

			for (int i=0; i<count; ++i)
			{
				FBComponent *pComp = sceneEvent.Component;
				if ( pComp == pLayersData->Layers[i]->Group )
				{
					pLayersData->Layers[i]->Group = nullptr;
					UIReset();
				}
			}
		}
	}
	else if (type == kFBSceneChangeDetach)
	{
		FBComponent *pComp = sceneEvent.ChildComponent;

		if (mReadingState == false && (FBIS(pComp, FBModel) || FBIS(pComp, FBMaterial) || FBIS(pComp, FBShader) || FBIS(pComp, FBTexture)) )
		{
			// update state information
			for (int i=0; i<count; ++i)
			{
				pLayersData->Layers[i]->ProcessDeleteEvent(pComp);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// UI EVENTS

void ToolRenderLayers::EventTreeCheck( HISender pSender, HKEvent pEvent )
{
	FBEventTreeSelect	e(pEvent);

	RenderLayersData *pLayersData = FindPersistentData(true);
	if (!pLayersData) return;

	FBTreeNode *pNode = e.TreeNode;
	if (pNode && (pNode->Reference > 0) )
	{
		int ndx = pNode->Reference - 1;
		pLayersData->Layers[ndx]->Renderable = pNode->Checked;
	}
}

void ToolRenderLayers::EventButtonMenu( HISender pSender, HKEvent pEvent )
{
	POINT p;
	if (GetCursorPos(&p))
	{
		mContextMenu.Execute(p.x, p.y);
	}
}

void ToolRenderLayers::OnMenuActivate( HISender pSender, HKEvent pEvent )
{
	FBEventMenu	e(pEvent);
	
	// find selected item
	LayerItem *pItem = nullptr;
	FindSelectedLayerData(pItem);

	if (pItem != nullptr)
		switch(e.Id)
		{
		case 0: // store state
			pItem->State.Store();
			// update ui
			UIReset();
			break;
		case 1: // clear state
			pItem->State.Clear();
			// update ui
			UIReset();
			break;
		case 2: // show state info
			ShowStateInfo(pItem);
			break;
		case 4:
			EventButtonAssignSet( pSender, pEvent );
			break;
		case 5:
			EventButtonClearSet( pSender, pEvent );
			break;
		}
}

void ToolRenderLayers::ShowStateInfo(LayerItem *pItem)
{
	if (pItem != nullptr)
	{
		FBString strCaption(pItem->Name);
		strCaption = strCaption + " State Info";

		mListState.Items.Clear();
		pItem->CollectStateInformation( &mListState.Items );
		
		

		mDialogState.Modal = true;
		mDialogState.Caption = strCaption;
		mDialogState.Show(this);
	}
}

void ToolRenderLayers::EventTreeSelect( HISender pSender, HKEvent pEvent )
{
	FBEventTreeSelect	e(pEvent);

	FBTreeNode *pNode = e.TreeNode;
	if (pNode && (pNode->Reference > 0) )
	{
		int ndx = pNode->Reference - 1;
		mSelectedNode = pNode;
		SetCurrentLayer(ndx);
	}
}

void ToolRenderLayers::EventTreeDblClk( HISender pSender, HKEvent pEvent )
{
	FBEventTreeSelect	e(pEvent);

	RenderLayersData *pLayersData = FindPersistentData(true);
	if (!pLayersData) return;

	FBTreeNode *pNode = e.TreeNode;
	if (pNode && (pNode->Reference > 0) )
	{
		int ndx = pNode->Reference - 1;
		
		char newName[256];
		memset(newName, 0, sizeof(char) * 256);
		memcpy_s(newName, 256, pLayersData->Layers[ndx]->Name, strlen(pLayersData->Layers[ndx]->Name) );
		if (FBMessageBoxGetUserValue("Render layers", "Choose a new layer name", newName, kFBPopupString, "Ok", "Cancel") == 1 )
		{
			pLayersData->Layers[ndx]->SetName(newName);
			UIReset();
		}
	}
}

void ToolRenderLayers::EventButtonAdd( HISender pSender, HKEvent pEvent )
{
	RenderLayersData *pLayersData = FindPersistentData(true);
	if (!pLayersData) return;

	static FBString value = "layer";
	if (FBMessageBoxGetUserValue( "Add a new layer", "Choose a new layer name", value, kFBPopupString, "Add", "Cancel" ) == 1)
	{
		if (value == "") {
			FBMessageBox("Add a new layer", "Layer name is empty!", "Ok" );
			return;
		}

		// TODO: add unique name

		pLayersData->Layers.Add( new LayerItem(false, value, mLayers.InsertLast(mLayers.GetRoot(), value) ) );
	}

	UIReset();
}

void ToolRenderLayers::EventButtonRemove( HISender pSender, HKEvent pEvent )
{
	RenderLayersData *pLayersData = FindPersistentData(true);
	if (!pLayersData) return;

#ifdef OLD_FBTREE
	if (mSelectedNode && mSelectedNode->Reference > 0)
	{
		int j = mSelectedNode->Reference;
		LayerItem *pItem = mLayersData[j - 1];
		
		if (pItem->IsMaster() )
		{
			FBMessageBox("Render layers", "It's not possible to remove master layer", "Ok");
		}
		else
		{
			mLayersData.RemoveAt(j - 1);
			if (pItem)
			{
				delete pItem;
				pItem = nullptr;
			}
			SetCurrentLayer(0); // make master layer default
		}
	}
#else
	for (int i=0; i<mLayers.SelectedNodes.GetCount(); ++i)
		if (mLayers.SelectedNodes[i]->Reference > 0)
		{
			LayerItem *pItem = pLayersData->Layers[mLayers.SelectedNodes[i]->Reference - 1];
		
			if (pItem->IsMaster() )
			{
				FBMessageBox("Render layers", "It's not possible to remove master layer", "Ok");
			}
			else
			{
				pLayersData->Layers.RemoveAt(mLayers.SelectedNodes[i]->Reference - 1);
				if (pItem)
				{
					delete pItem;
					pItem = nullptr;
				}
				SetCurrentLayer(0); // make master layer default
			}
		}
#endif
	UIReset();
}

void ToolRenderLayers::EventButtonUp( HISender pSender, HKEvent pEvent )
{
	RenderLayersData *pLayersData = FindPersistentData(true);
	if (!pLayersData) return;

#ifdef OLD_FBTREE

	if (mSelectedNode && mSelectedNode->Reference > 0)
	{
		int j = mSelectedNode->Reference;
		if (j < mLayersData.GetCount())
		{
			j -= 1;

			LayerItem *pItem = mLayersData[j];
			mLayersData[j] = mLayersData[j+1];
			mLayersData[j+1] = pItem;

			mCurrentLayer += 1;
		}
	}

#else
	for (int i=0; i<mLayers.SelectedNodes.GetCount(); ++i)
		if (mLayers.SelectedNodes[i]->Reference > 0)
		{
			int j = mLayers.SelectedNodes[i]->Reference;
			if (j < pLayersData->Layers.GetCount())
			{
				j -= 1;

				LayerItem *pItem = pLayersData->Layers[j];
				pLayersData->Layers[j] = pLayersData->Layers[j+1];
				pLayersData->Layers[j+1] = pItem;

				mCurrentLayer += 1;
			}
		}
#endif
	UIReset();
}

void ToolRenderLayers::EventButtonDown( HISender pSender, HKEvent pEvent )
{
	RenderLayersData *pLayersData = FindPersistentData(true);
	if (!pLayersData) return;

#ifdef OLD_FBTREE
	if (mSelectedNode && mSelectedNode->Reference > 0)
	{
		int j = mSelectedNode->Reference;
		if (j > 0)
		{
			LayerItem *pItem = mLayersData[j];
			mLayersData[j] = mLayersData[j-1];
			mLayersData[j-1] = pItem;

			mCurrentLayer -= 1;
		}
	}
#else
	for (int i=0; i<mLayers.SelectedNodes.GetCount(); ++i)
		if (mLayers.SelectedNodes[i]->Reference > 0)
		{
			int j = mLayers.SelectedNodes[i]->Reference - 1;
			if (j > 0)
			{
				LayerItem *pItem = pLayersData->Layers[j];
				pLayersData->Layers[j] = pLayersData->Layers[j-1];
				pLayersData->Layers[j-1] = pItem;

				mCurrentLayer -= 1;
			}
		}
#endif
	UIReset();
}

void ToolRenderLayers::EventButtonProperties( HISender pSender, HKEvent pEvent )
{
	AddNewEvent(EVENT_LAYER_CHANGE);

	// show properties options
	FBPopNormalTool( "Render Options" );
}


void ToolRenderLayers::EventButtonStateOk( HISender pSender, HKEvent pEvent )
{
	mDialogState.Close(true);
}

void ToolRenderLayers::EventButtonDialogOk( HISender pSender, HKEvent pEvent )
{
	mDialogSets.Close(true);
}

void ToolRenderLayers::EventButtonDialogCancel( HISender pSender, HKEvent pEvent )
{
	mDialogSets.Close(false);
}

void ToolRenderLayers::EventButtonAssignSet( HISender pSender, HKEvent pEvent )
{
	FBScene *pScene = FBSystem().Scene;

	// find selected item
	LayerItem *pItem = nullptr;
	FindSelectedLayerData(pItem);

	// assign selected set to the render layer
	if (pItem == nullptr)
	{
		FBMessageBox( "Render layers", "Please select some layer", "Ok");
	}
	else
	{
		/*
		mDialogSets.Modal = true;
		mDialogSets.Caption = "Choose set from the list";
		mDialogSets.Show(this);
		*/

		for (int i=0; i<pScene->Groups.GetCount(); ++i)
			if (pScene->Groups[i]->Selected)
			{
				pItem->Group = (FBGroup*) pScene->Groups.GetAt(i);
			}

		// update ui
		UIReset();
	}
}

void ToolRenderLayers::EventButtonClearSet( HISender pSender, HKEvent pEvent )
{

	// find selected item
	LayerItem *pItem = nullptr;
	FindSelectedLayerData(pItem);

	// assign selected set to the render layer
	if (pItem == nullptr)
	{
		FBMessageBox( "Render layers", "Please select some layer", "Ok");
	}
	else
	{
		pItem->Group = nullptr;

		// update ui
		UIReset();
	}
}

/************************************************
 *	show about info
 ************************************************/
void ToolRenderLayers::EventButtonAbout( HISender pSender, HKEvent pEvent )
{
	const char *szVersion = LoadVersionFromResource();

	FBMessageBox( FBString("Render Layers ", szVersion), "Author: Sergey Solohin (Neill3d) 2014\n e-mail to: s@neill3d.com\n web-site: http:\\\\neill3d.com\n"
					"- Freelance MoCap studio AnimArt - http:\\\\mocap.com.ua", "Ok" );
}

/************************************************
 *	define grab options and grab all time range
 ************************************************/
void ToolRenderLayers::RenderLayer(LayerItem *pLayer, FBCamera *pCamera)
{
	FBVideoGrabOptions	GrabOptions = VideoGrabber.GetOptions();

	FBTime lStart, lStop, lStep;
	lStart = pLayer->StartTime;	//Frame 0
	lStop = pLayer->StopTime;	//Frame 150
	lStep = pLayer->StepTime;	//1 Frame

	// TODO: FBVideoCodecManager

	// add folder for the layer and if needed for the camera
	FBString FullFilePath = pLayer->FilePath + FBString(pLayer->Name) + FBString("\\");

	// and for the camera
	if (pLayer->Cameras.GetCount() > 1) 
	{
		// TODO: add before extension
		FullFilePath = FullFilePath + pCamera->Name + FBString("\\");
	}

	CreateDirectoryTree(FullFilePath);

	FullFilePath = FullFilePath + pLayer->FileName;

	GrabOptions.mTimeSpan.Set(lStart, lStop);
	GrabOptions.mTimeSteps = lStep;
	GrabOptions.mCameraResolution = pLayer->PictureMode;
	GrabOptions.mBitsPerPixel = pLayer->PixelMode;
	GrabOptions.mFieldMode = pLayer->FieldMode;
	GrabOptions.mViewingMode = pLayer->DisplayMode;
	GrabOptions.mOutputFileName = FullFilePath;
	GrabOptions.mAntiAliasing = pLayer->Antialiasing;
	GrabOptions.mShowCameraLabel = pLayer->ShowLabel;
	GrabOptions.mShowSafeArea = pLayer->ShowSafe;
	GrabOptions.mShowTimeCode = pLayer->ShowTime;
	GrabOptions.mRenderAudio = pLayer->Audio;
	GrabOptions.mAudioRenderFormat = FBAudioFmt_GetDefaultFormat();	//44100Hz 16bits stereo

	VideoGrabber.SetOptions(&GrabOptions);
	//VideoGrabber.SetRefreshViewFunc(&mView, mView.RefreshViewCallback);
	//VideoGrabber.SetPostRenderFrameCallback( this, ToolRenderLayers::RunRefreshStats );

	//RefreshStats();
	//mDialogStats.Caption = "Statistics";
	//mDialogStats.Modal = false;
	//mDialogStats.Show(this);

	//FBProgress		lProgress;
	//lProgress.Caption = FBString("Render ") + pLayer->Name;
	//lProgress.Text = "in progress";
	//lProgress.Percent = 0;
	//g_pProgress = &lProgress;

	// - set camera
	if (pCamera) FBSystem().Renderer->CurrentCamera = pCamera;
	// - execute pre-render script
	if (pLayer->Script != "") FBApplication().ExecuteScript( pLayer->Script );

	//Start rendering while grabbing each frame
#ifdef OLD_FBAPP_FILERENDER
	mApplication.FileRender( &GrabOptions );
#else
	if (!mApplication.FileRender( &GrabOptions ) )
	{
		FBMessageBox( "Render error", VideoGrabber.GetLastErrorMsg(), "Ok" );
	}
#endif
	//g_pProgress = nullptr;
	//mDialogStats.Close(true);

	// execute post-render script
	if (pLayer->ScriptPost != "") FBApplication().ExecuteScript( pLayer->ScriptPost );
}

struct	ShaderInformation
{
	FBModel		*pModel;
	int			numberOfShaders;
	FBShader	*shaders[10];		// maximum number of shaders - 10 !
};

const bool CheckComponent( FBScene *pScene, FBComponent *pComponent )
{
	if (pComponent == pScene->RootModel) return false;
	if ( FBIS(pComponent, FBModel) == false || FBIS(pComponent, FBLight) || FBIS(pComponent, FBCamera) || FBIS(pComponent, FBModelNull) ) return false;
	FBModel *pModel = (FBModel*) pComponent;
	FBGeometry *pGeometry = pModel->Geometry;
	if (pGeometry == nullptr) return false;
	return true;
}

void ToolRenderLayers::EventButtonRender( HISender pSender, HKEvent pEvent )
{
	FBVideoCodecManager lCodecMan;
	FBScene *pScene = FBSystem().Scene;

	RenderLayersData *pLayersData = FindPersistentData(true);
	if (!pLayersData) return;

	FBShader *pShader;
	FBShaderManager	shaderMan;
	pShader = shaderMan.CreateShader( "ZShader" );
	if (!pShader) {
		FBMessageBox( "Render Layers", "Failed to connect matte shader", "Ok" );
		return;
	}

	FBProperty *pProp = pShader->PropertyList.Find("ShowIt");
	if (pProp)
	{
		pProp->SetInt(0);
	}

	// for matting
	//	1 - store all shaders information for the objects
	//  2 - replace with matte shader
	//	3 - return back shaders, remove matte shader and free memory
	
	bool matte = true;

	FBArrayTemplate<ShaderInformation>		shaderStorage;
	shaderStorage.SetCount( pScene->Components.GetCount() );

	const int numberOfComponents = pScene->Components.GetCount();
	for (int i=0; i<numberOfComponents; ++i)
	{
		ShaderInformation &shaderInfo = shaderStorage[i];
		shaderInfo.pModel = nullptr;
		shaderInfo.numberOfShaders = 0;

		if ( CheckComponent(pScene, pScene->Components[i]) )
		{

			FBModel *pModel = (FBModel*) pScene->Components[i];
			shaderInfo.pModel = pModel;	

			const int numberOfFBModelShaders = pModel->Shaders.GetCount();

			if (numberOfFBModelShaders >= 10)
			{
				FBMessageBox( "Render Layers", FBString("Too many shaders in model ", pModel->Name), "Ok" );
			}
			else
			{
				for (int j=0; j<numberOfFBModelShaders; ++j)
				{
					shaderInfo.shaders[shaderInfo.numberOfShaders] = pModel->Shaders[j];
					shaderInfo.numberOfShaders++;
				}
			}
		}
					
	}

	LayerItem *pMasterLayer = nullptr;
	for (int i=0; i<pLayersData->Layers.GetCount(); ++i)
		if (pLayersData->Layers[i]->IsMaster() )
		{
			pMasterLayer = pLayersData->Layers[i];
			break;
		}

	if (!pMasterLayer)
	{
		FBMessageBox( "Render Layers", "Failed to find a master layer", "Ok");
		return;
	}

	//
	// BLOCK ALL REMOVE OPERATIONS, WE CURRENTLY ONLY RESTORE EACH LAYER STATES
	//
	mReadingState = true;

	for (int i=0; i<pLayersData->Layers.GetCount(); ++i)
	{
		LayerItem *pLayer = pLayersData->Layers[i];
		if (pLayer && pLayer->Renderable) 
		{
			// switch to current layer (assign state)
			pLayer->State.Restore();

			// check path, file, camera, etc. and override params from master layer
			pLayer->CheckOptions(pMasterLayer);
			
			// prepare objects for this layer (if group is specified, then hide everything else)
			FBGroup *pGroup = pLayer->Group;
			
			FBArrayTemplate<bool>	vis;
			
			// matte all objects out of group
			matte = pLayer->Matte;
			if (pGroup)
			{
				vis.SetCount(numberOfComponents);

				for (int i=0; i<numberOfComponents; ++i)
				{
					if ( CheckComponent(pScene, pScene->Components[i]) )
					{
						FBModel *pModel = (FBModel*) pScene->Components[i];
						vis[i] = pModel->Show;
						
						if (matte)
						{
							if (!pGroup->Contains(pScene->Components[i]) )
							{
								pModel->Shaders.RemoveAll();
								pModel->Shaders.Add( pShader );
							}
						}
						else
						{
							// simple hide
							pModel->Show = pGroup->Contains(pScene->Components[i]);
						}
					}
					
				}
			}

			// set codec
			if (pLayer->Codec == "{ Uncompressed }")
				lCodecMan.SetVideoCodecMode( FBVideoCodecUncompressed );
			else if (pLayer->Codec == "{ Asked }")
				lCodecMan.SetVideoCodecMode( FBVideoCodecAsk );
			else
			{
				lCodecMan.SetVideoCodecMode( FBVideoCodecStored );
				lCodecMan.SetDefaultCodec(pLayer->Format, pLayer->Codec);
			}

			//
			// render each camera as a separate sequence
			//
			for (int j=0; j<pLayer->Cameras.GetCount(); ++j)
			{
				RenderLayer(pLayer, pLayer->Cameras[j]);
			}

			// return back scene objects visibility
			if (pGroup && vis.GetCount() == pScene->Components.GetCount())
			{

				for (int i=0; i<numberOfComponents; ++i)
				{
					ShaderInformation &shaderInfo = shaderStorage[i];

					if (shaderInfo.pModel && shaderInfo.numberOfShaders > 0)
					{
						shaderInfo.pModel->Shaders.RemoveAll();

						for (int j=0; j<shaderInfo.numberOfShaders; ++j)
						{
							shaderInfo.pModel->Shaders.Add( shaderInfo.shaders[j] );
						}
					}
				}
			}

		}
	}

	pLayersData->Layers[mCurrentLayer]->State.Restore();
	mReadingState = false;

	if (pShader)
	{
		pShader->FBDelete();
		pShader = nullptr;
	}
}

/************************************************
 *	UI Idle callback.
 ************************************************/
void ToolRenderLayers::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	if ( IsMainUINeedUpdate(false) )
	{
		UIReset();
		AddNewEvent(EVENT_LAYER_CHANGE);
	}
}


/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ToolRenderLayers::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		if (gDoRender)
		{
			EventButtonRender(nullptr, nullptr);
			gDoRender = false;

			if (gClose)
			{
				FBApplication &app = FBApplication::TheOne();
				app.FileExit();
			}
		}
	}
}

/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void ToolRenderLayers::EventToolPaint( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	Tool resize callback.
 ************************************************/
void ToolRenderLayers::EventToolResize( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	Handle input into the tool.
 ************************************************/
void ToolRenderLayers::EventToolInput( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *  
 ************************************************/
RenderLayersData *ToolRenderLayers::FindPersistentData( bool pCreate )
{
    // First we want to see if there is any infos for this tool present in the scene
	FBScene* lScene = FBSystem::TheOne().Scene;
    int lIdx = 0;
    
	RenderLayersData* lData = nullptr;

    for( lIdx = 0; lIdx < lScene->UserObjects.GetCount(); ++lIdx ) 
	{    
		FBUserObject* lObject = lScene->UserObjects[lIdx];
        
		if( FBIS(lObject, RenderLayersData) ) 
		{
			if (lData == nullptr)
				lData = (RenderLayersData*) lObject;
			else
				lObject->FBDelete();
        }
    }

    if( (lData == nullptr) && pCreate ) {
        lData = new RenderLayersData( "RenderLayersData" );
    }
    
    return lData;
}

void ToolRenderLayers::DeletePersistentData()
{
    RenderLayersData* lData = FindPersistentData( false );
    if( lData ) {
        lData->FBDelete();
    }
}