#ifndef __RENDERLAYERS_TOOL_H__
#define __RENDERLAYERS_TOOL_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: renderLayers_tool.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "renderCommon.h"
//#include "renderadvance_view.h"
#include "persistent_data.h"

//--- Registration define
#define RENDERLAYERS__CLASSNAME	ToolRenderLayers
#define RENDERLAYERS__CLASSSTR	"ToolRenderLayers"


/**	Tool template.
*/
class ToolRenderLayers : public FBTool
{
	//--- Tool declaration.
	FBToolDeclare( ToolRenderLayers, FBTool );

public:
	//--- Construction/Destruction,
	virtual bool FBCreate();		//!< Creation function.
	virtual void FBDestroy();		//!< Destruction function.

	void Clear();

	// FBX store/retrieve
	//virtual bool FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );
	//virtual bool FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

private:
	// UI Management
	void	UICreate	();
	void	UIConfigure	();
	void	UIReset		();

	// UI callbacks
	void	EventButtonAdd( HISender pSender, HKEvent pEvent );
	void	EventButtonRemove( HISender pSender, HKEvent pEvent );
	void	EventButtonUp( HISender pSender, HKEvent pEvent );
	void	EventButtonDown( HISender pSender, HKEvent pEvent );
	void	EventButtonProperties( HISender pSender, HKEvent pEvent );
	void	EventButtonAssignSet( HISender pSender, HKEvent pEvent );
	void	EventButtonClearSet( HISender pSender, HKEvent pEvent );
	void	EventButtonMenu( HISender pSender, HKEvent pEvent );

	void	EventButtonDialogOk( HISender pSender, HKEvent pEvent );
	void	EventButtonDialogCancel( HISender pSender, HKEvent pEvent );

	void	EventButtonStateOk( HISender pSender, HKEvent pEvent );

	void	EventTreeSelect( HISender pSender, HKEvent pEvent );
	void	EventTreeCheck( HISender pSender, HKEvent pEvent );
	void	EventTreeDblClk( HISender pSender, HKEvent pEvent );

	void	EventButtonAbout( HISender pSender, HKEvent pEvent );
	void	EventButtonRender( HISender pSender, HKEvent pEvent );

	void	EventToolIdle       ( HISender pSender, HKEvent pEvent );
	void	EventToolShow       ( HISender pSender, HKEvent pEvent );
	void	EventToolPaint      ( HISender pSender, HKEvent pEvent );
	void	EventToolResize     ( HISender pSender, HKEvent pEvent );
	void	EventToolInput      ( HISender pSender, HKEvent pEvent );

	void	EventFileNew	( HISender pSender, HKEvent pEvent );
	void	EventFileOpenCompleted	( HISender pSender, HKEvent pEvent );
	void	EventSceneChange	( HISender pSender, HKEvent pEvent );

	void	OnMenuActivate ( HISender pSender, HKEvent pEvent );

private:
	
	FBSystem				mSystem;
	FBApplication			mApplication;
	
	// UI components

	FBTree		mLayers;	// layers with active flag and contribution sets of objects
	FBTreeNode	*mMasterLayer;	// always existing
	//FBArrayTemplate <FBTreeNode*>	mSelectedNodes;
	FBTreeNode	*mSelectedNode;

	FBGenericMenu		mContextMenu;

	FBPopup		mDialogState;
	FBLayout	mLayoutState;
	FBButton	mButtonStateOk;
	FBList		mListState;

	FBLabel		mLabel;	// display current layer
	FBLabel		mLabel2;

	FBList		mDebugList;		// list for debugging event

	FBButton	mButtonAdd;
	FBButton	mButtonRemove;
	FBButton	mButtonUp;
	FBButton	mButtonDown;
	FBButton	mButtonProperties;
	FBButton	mButtonAssignSet;
	FBButton	mButtonClearSet;
	FBButton	mButtonMenu;	// launch a context menu

	FBPopup		mDialogSets;	// choose set from a list

	FBButton	mButtonDialogOk;
	FBButton	mButtonDialogCancel;
	FBList		mListDialogSets;

	//
	//	rendering components
	//
	FBPopup		mDialogStats;	// display rendering statistics

	FBLayout			mLayoutSettings;
	FBButton			mButtonRender;
	FBButton			mButtonAbout;

	//ORView3D			mRenderView;

	FBLabel				mTotalFrameCount;
	FBLabel				mRemainingFrameCount;
	FBLabel				mTotalTimeElapsed;
	FBLabel				mEstimatedTime;
	FBLabel				mEstimatedTimeRemaining;
	FBLabel				mTimePerFrame;

	FBVideoGrabber			VideoGrabber;
	FBVideoGrabStats		mStats;

	void RenderLayer(LayerItem *pLayer, FBCamera *pCamera);
	
	//
	// layers data
	//
	int							mCurrentLayer;	// by default 0 - master layer
	

	bool						mReadingState;	// block scene remove event while reading

	void FindSelectedLayerData(LayerItem * &pItem);
	void SetCurrentLayer(int layer);

	void ShowStateInfo(LayerItem *pItem);

public:
	void RefreshStats();
	static void RunRefreshStats(void *pThis);

	static RenderLayersData		*FindPersistentData( bool pCreate );
    static void                 DeletePersistentData();
};

#endif /* __RENDERLAYERS_TOOL_H__ */
