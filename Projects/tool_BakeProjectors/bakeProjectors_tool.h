#ifndef __BAKEPROJECTORS_TOOL_H__
#define __BAKEPROJECTORS_TOOL_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: bakeProjectors_tool.cpp
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

#include "bakeProjectors_view.h"
#include "bakeProjectors_viewData.h"

#include <vector>
#include <map>

//--- Registration define
#define TOOL_BAKEPROJECTORS__CLASSNAME		ToolBakeProjectors
#define TOOL_BAKEPROJECTORS__CLASSSTR		"ToolBakeProjectors"

/**	Tool template.
*/
class ToolBakeProjectors : public FBTool
{
	//--- Tool declaration.
	FBToolDeclare( ToolBakeProjectors, FBTool );

public:
	//--- Construction/Destruction,
	virtual bool FBCreate();		//!< Creation function.
	virtual void FBDestroy();		//!< Destruction function.

	// FBX store/retrieve
	virtual bool FbxStore		( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

private:
	// UI Management
	void	UICreate	();
	void	UICreateModels();
	void	UICreateOptions();
	void	UIConfigure	();
	void	UIConfigureModels();
	void	UIConfigureOptions();
	void	UIReset		();
	void	UIResetModels();
	void	UIResetOptions();

	// UI callbacks
	
	void	EventButtonAddClick( HISender pSender, HKEvent pEvent );
	void	EventButtonRemoveClick( HISender pSender, HKEvent pEvent );
	void	EventButtonRemoveAllClick( HISender pSender, HKEvent pEvent );

	void	EventButtonIsolateClick( HISender pSender, HKEvent pEvent );
	void	EventButtonBakeFrameClick( HISender pSender, HKEvent pEvent );
	void	EventButtonBakeRangeClick( HISender pSender, HKEvent pEvent );
	void	EventButtonAboutClick( HISender pSender, HKEvent pEvent );

	void	EventToolIdle		( HISender pSender, HKEvent pEvent );
	void	EventToolShow		( HISender pSender, HKEvent pEvent );
	void	EventToolPaint		( HISender pSender, HKEvent pEvent );
	void	EventToolResize		( HISender pSender, HKEvent pEvent );
	void	EventToolInput		( HISender pSender, HKEvent pEvent );

	void	EventSpreadSpreadChange( HISender pSender, HKEvent pEvent );

	void	EventModelsDblClick( HISender pSender, HKEvent pEvent );
	void	EventModelsDragNDrop( HISender pSender, HKEvent pEvent );

	void	EventSceneChange( HISender pSender, HKEvent pEvent );
	void	EventFileCustom( HISender pSender, HKEvent pEvent );

	// Refresh the view
	void	RefreshView			();

protected:

	int							mCurrentRow;

	std::vector<ViewBakeProjectorsData::SpreadModel>		mModels;

	void UpdateViewModels();

private:
	
	FBApplication			mApp;
	FBSystem				mSystem;
	FBPlayerControl			mPlayer;

	FBLayout				mLayoutModels;

	FBButton				mButtonAdd;
	FBButton				mButtonRemove;
	FBButton				mButtonRemoveAll;
	FBSpread				mSpreadModels;

	FBLayout				mLayoutOptions;

	FBEditProperty			mEditSaveImagePerModel;
	FBEditProperty			mEditSaveOnlyProjectors;
	FBEditProperty			mEditAddFrameNumber;
	FBEditProperty			mEditBackColor;

	FBEditProperty			mEditSaveJpeg;
	FBEditProperty			mEditJpegQuality;

	FBButton				mButtonBakeFrame;
	FBButton				mButtonBakeRange;	// render using zoom slider range
	FBButton				mButtonAbout;

	FBButton				mButtonIsolate;		// make visible only objects in the list

	ViewBakeProjectors		mView;				// 3D view

	std::map<FBModel*, char>	mVisMap;			

	void			FillSpreadSheet();
	void			UpdateSpreadCells();

	void	UpdateVisMap(FBModel *parentModel);
	void	ShowModelHeirarchy(FBModel *parentModel, bool show);
	void	ClearVisMap();
};

#endif /* __BAKEPROJECTORS_TOOL_H__ */
