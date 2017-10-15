
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


//--- Class declaration
#include "bakeProjectors_tool.h"

//--- Registration defines
#define TOOL_BAKEPROJECTORS__CLASS		TOOL_BAKEPROJECTORS__CLASSNAME
#define TOOL_BAKEPROJECTORS__LABEL		"Bake Projectors Tool"
#define TOOL_BAKEPROJECTORS__DESC		"Bake Projectors Tool"

//--- Implementation and registration
FBToolImplementation(	TOOL_BAKEPROJECTORS__CLASS	);
FBRegisterTool		(	TOOL_BAKEPROJECTORS__CLASS,
						TOOL_BAKEPROJECTORS__LABEL,
						TOOL_BAKEPROJECTORS__DESC,
						FB_DEFAULT_SDK_ICON	);	// Icon filename (default=Open Reality icon)

/************************************************
 *	Constructor.
 ************************************************/
bool ToolBakeProjectors::FBCreate()
{
	StartSize[0] = 640;
	StartSize[1] = 480;

	// Manage UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ToolBakeProjectors::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &ToolBakeProjectors::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ToolBakeProjectors::EventToolResize		);
	OnPaint.Add	( this, (FBCallback) &ToolBakeProjectors::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ToolBakeProjectors::EventToolInput		);

	mSystem.Scene->OnChange.Add	( this, (FBCallback) &ToolBakeProjectors::EventSceneChange );

	mApp.OnFileNew.Add( this, (FBCallback) &ToolBakeProjectors::EventFileCustom );
	mApp.OnFileOpen.Add( this, (FBCallback) &ToolBakeProjectors::EventFileCustom );
	mApp.OnFileMerge.Add( this, (FBCallback) &ToolBakeProjectors::EventFileCustom );

	mCurrentRow = -1;

	return true;
}

/************************************************
 *	Create, configure & reset UI.
 ************************************************/
void ToolBakeProjectors::UICreate()
{
	// Tool options
	int lS = 4;
	int lW = 160;
	int lH = 120;

	// Configure layout
	
	AddRegion( "Models", "Models",
										lS,kFBAttachLeft,"",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,kFBAttachNone,	"",	1.0 );

	AddRegion( "Options", "Options",
										-lW,kFBAttachRight,"",	1.0	,
										lS,	kFBAttachBottom,"Models",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										-lS,kFBAttachBottom,	"",	1.0 );
	AddRegion( "View3D", "View3D",
										lS,	kFBAttachLeft,	"",	1.0,
										lS,	kFBAttachBottom,"Models",	1.0,
										-lS,kFBAttachLeft,	"Options",1.0,
										-lS,kFBAttachBottom,"",1.0 );

	// Assign regions
	SetControl	("Models",		mLayoutModels );
	SetControl	( "Options",	mLayoutOptions );
	SetView		( "View3D",		mView			);

	UICreateModels();
	UICreateOptions();
}

void ToolBakeProjectors::UICreateModels()
{
	// Tool options
	int lS = 4;
	int lH = 25;
	int lW = 120;

	// configure layout regions

	mLayoutModels.AddRegion( "Add", "Add",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutModels.AddRegion( "Remove", "Remove",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"Add",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutModels.AddRegion( "RemoveAll", "RemoveAll",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"Remove",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutModels.AddRegion( "Spread", "Spread",
										lS,	kFBAttachRight,	"Add",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										-lS,kFBAttachBottom,	"",	1.0 );

	//
	mLayoutModels.SetControl( "Add", mButtonAdd );
	mLayoutModels.SetControl( "Remove", mButtonRemove );
	mLayoutModels.SetControl( "RemoveAll", mButtonRemoveAll );

	mLayoutModels.SetControl( "Spread", mSpreadModels );
}

void ToolBakeProjectors::UICreateOptions()
{
	// Tool options
	int lS = 4;
	int lH = 25;

	// configure layout regions

	mLayoutOptions.AddRegion( "EditSaveImage", "EditSaveImage",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOptions.AddRegion( "EditSaveOnlyProj", "EditSaveOnlyProj",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"EditSaveImage",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOptions.AddRegion( "EditAddFrameNumber", "EditAddFrameNumber",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"EditSaveOnlyProj",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOptions.AddRegion( "EditBackColor", "EditBackColor",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"EditAddFrameNumber",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOptions.AddRegion( "EditSaveJpeg", "EditSaveJpeg",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"EditBackColor",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOptions.AddRegion( "EditJpegQuality", "EditJpegQuality",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"EditSaveJpeg",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	//

	mLayoutOptions.AddRegion( "ButtonIsolate", "ButtonIsolate",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS*6,	kFBAttachBottom,"EditJpegQuality",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOptions.AddRegion( "ButtonBakeFrame", "ButtonBakeFrame",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS*6,	kFBAttachBottom,"ButtonIsolate",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOptions.AddRegion( "ButtonBakeRange", "ButtonBakeRange",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"ButtonBakeFrame",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOptions.AddRegion( "ButtonAbout", "ButtonAbout",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"ButtonBakeRange",	1.0,
										-lS,kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	// assign regions

	mLayoutOptions.SetControl( "EditSaveImage", mEditSaveImagePerModel );
	mLayoutOptions.SetControl( "EditSaveOnlyProj", mEditSaveOnlyProjectors );
	mLayoutOptions.SetControl( "EditAddFrameNumber", mEditAddFrameNumber );

	mLayoutOptions.SetControl( "EditBackColor", mEditBackColor );

	mLayoutOptions.SetControl( "EditSaveJpeg", mEditSaveJpeg );
	mLayoutOptions.SetControl( "EditJpegQuality", mEditJpegQuality );

	mLayoutOptions.SetControl( "ButtonIsolate", mButtonIsolate );
	mLayoutOptions.SetControl( "ButtonBakeFrame", mButtonBakeFrame );
	mLayoutOptions.SetControl( "ButtonBakeRange", mButtonBakeRange );
	mLayoutOptions.SetControl( "ButtonAbout", mButtonAbout );

}

void ToolBakeProjectors::UIConfigure()
{
	UIConfigureModels();
	UIConfigureOptions();
}

void ToolBakeProjectors::UIConfigureModels()
{
	mButtonAdd.Caption = "Add";
	mButtonAdd.OnClick.Add( this, (FBCallback) &ToolBakeProjectors::EventButtonAddClick );
	
	mButtonRemove.Caption = "Remove";
	mButtonRemove.OnClick.Add( this, (FBCallback) &ToolBakeProjectors::EventButtonRemoveClick );

	mButtonRemoveAll.Caption = "Remove All";
	mButtonRemoveAll.OnClick.Add( this, (FBCallback) &ToolBakeProjectors::EventButtonRemoveAllClick );

	mSpreadModels.OnCellChange.Add( this, (FBCallback) &ToolBakeProjectors::EventSpreadSpreadChange );
	mSpreadModels.OnRowClick.Add( this, (FBCallback) &ToolBakeProjectors::EventSpreadSpreadChange );

	FillSpreadSheet();
}

void ToolBakeProjectors::UIConfigureOptions()
{
	
	//mEditModels.Caption = "Models: ";
	//mEditModels.Property = &mView.Models;

	mEditSaveImagePerModel.Caption = "Save Image Per Model";
	mEditSaveImagePerModel.Property = &mView.SaveImagePerModel;

	mEditSaveOnlyProjectors.Caption = "Save Only Projectors";
	mEditSaveOnlyProjectors.Property = &mView.SaveOnlyProjectors;

	mEditAddFrameNumber.Caption = "Add Frame Number Suffix";
	mEditAddFrameNumber.Property = &mView.ImageAddFrameNumber;

	mEditBackColor.Caption = "Background: ";
	mEditBackColor.Property = &mView.BackgroungColor;

	mEditSaveJpeg.Caption = "Save To Jpeg";
	mEditSaveJpeg.Property = &mView.SaveJpeg;

	mEditJpegQuality.Caption = "Jpeg Quality";
	mEditJpegQuality.Property = &mView.JpegQuality;

	mButtonIsolate.Caption = "Isolate Models";
	mButtonIsolate.OnClick.Add( this, (FBCallback) &ToolBakeProjectors::EventButtonIsolateClick );

	mButtonBakeFrame.Caption = "Bake Frame!";
	mButtonBakeFrame.OnClick.Add( this, (FBCallback) &ToolBakeProjectors::EventButtonBakeFrameClick );

	mButtonBakeRange.Caption = "Bake Timeslider Range!";
	mButtonBakeRange.OnClick.Add( this, (FBCallback) &ToolBakeProjectors::EventButtonBakeRangeClick );

	mButtonAbout.Caption = "About";
	mButtonAbout.OnClick.Add( this, (FBCallback) &ToolBakeProjectors::EventButtonAboutClick );
}

void ToolBakeProjectors::UIReset()
{
	UIResetModels();
	UIResetOptions();
}

void ToolBakeProjectors::UIResetModels()
{
}

void ToolBakeProjectors::UIResetOptions()
{
}

/************************************************
 *	Destruction function.
 ************************************************/
void ToolBakeProjectors::FBDestroy()
{
	mApp.OnFileNew.Remove( this, (FBCallback) &ToolBakeProjectors::EventFileCustom );
	mApp.OnFileOpen.Remove( this, (FBCallback) &ToolBakeProjectors::EventFileCustom );
	mApp.OnFileMerge.Remove( this, (FBCallback) &ToolBakeProjectors::EventFileCustom );

	mSystem.Scene->OnChange.Remove ( this, (FBCallback) &ToolBakeProjectors::EventSceneChange );

	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ToolBakeProjectors::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ToolBakeProjectors::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ToolBakeProjectors::EventToolPaint	);
	OnInput.Remove	( this, (FBCallback) &ToolBakeProjectors::EventToolInput	);
	OnResize.Remove	( this, (FBCallback) &ToolBakeProjectors::EventToolResize	);

	// Free user allocated memory
	//mEditModels.Property = nullptr;

	mEditBackColor.Property = nullptr;
	mEditSaveImagePerModel.Property = nullptr;
	mEditAddFrameNumber.Property = nullptr;

	mEditSaveJpeg.Property = nullptr;
	mEditJpegQuality.Property = nullptr;
}

/************************************************
 *	Scene Change Event.
 ************************************************/
void ToolBakeProjectors::EventFileCustom( HISender pSender, HKEvent pEvent )
{
	ClearVisMap();
}

void ToolBakeProjectors::EventSceneChange( HISender pSender, HKEvent pEvent )
{
	FBEventSceneChange	lEvent(pEvent);

	bool needUIupdate = false;

	if (lEvent.Type == kFBSceneChangeDetach && FBIS(lEvent.Component, FBScene) )
	{
		if ( FBIS(lEvent.ChildComponent, FBModel) )
			ClearVisMap();

		for (auto iter=mModels.begin(); iter!=mModels.end(); ++iter)
		{
			if (iter->pModel==lEvent.ChildComponent)
			{
				mModels.erase(iter);
				needUIupdate = true;
				break;
			}
		}
	}
	else if (lEvent.Type == kFBSceneChangeChangedName)
	{
		for (auto iter=mModels.begin(); iter!=mModels.end(); ++iter)
		{
			if (iter->pModel==lEvent.Component)
			{
				needUIupdate = true;
				break;
			}
		}
	}

	//
	if (needUIupdate)
	{
		FillSpreadSheet();
	}
}

/************************************************
 *	UI Idle callback.
 ************************************************/
void ToolBakeProjectors::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	RefreshView();
}

void ToolBakeProjectors::RefreshView()
{
	mView.SetCurrentRow(mCurrentRow);

	if (mModels.size() > 0 && mCurrentRow < 0)
		mView.SetCurrentRow(0);

	UpdateViewModels();
	mView.Refresh(true);
}

/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ToolBakeProjectors::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		// Reset the UI here.
	}
	else
	{
	}
}

/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void ToolBakeProjectors::EventToolPaint( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	Tool resize callback.
 ************************************************/
void ToolBakeProjectors::EventToolResize( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	Handle input into the tool.
 ************************************************/
void ToolBakeProjectors::EventToolInput( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool ToolBakeProjectors::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ToolBakeProjectors::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

void ToolBakeProjectors::EventButtonAddClick( HISender pSender, HKEvent pEvent )
{
	FBModelList list;
	FBGetSelectedModels(list);

	for (int i=0, count = list.GetCount(); i<count; ++i)
	{
		FBModel *pModel = list.GetAt(i);

		bool isExist = false;
		for (auto iter=mModels.begin(); iter!=mModels.end(); ++iter)
			if (iter->pModel == pModel)
			{
				isExist = true;
				break;
			}

		if (isExist == false)
		{
			ViewBakeProjectorsData::SpreadModel newRow;

			newRow.pModel = pModel;
			newRow.preset = 1;
			newRow.width = 1024;
			newRow.height = 1024;

			mModels.push_back(newRow);
		}
	}

	FillSpreadSheet();
}

void ToolBakeProjectors::EventButtonRemoveClick( HISender pSender, HKEvent pEvent )
{
	const int rowNdx = mCurrentRow;

	FBModel *pModel = nullptr;
	if (rowNdx >= 0 && rowNdx < mModels.size() )
		pModel = mModels[rowNdx].pModel;

	if (pModel != nullptr)
	{
		// TODO: remove current model !

		for (auto iter=mModels.begin(); iter!=mModels.end(); ++iter)
		{
			if (iter->pModel == pModel)
			{
				mModels.erase(iter);
				break;
			}
		}

	}

	FillSpreadSheet();
}

void ToolBakeProjectors::FillSpreadSheet()
{
	mSpreadModels.Clear();
	mCurrentRow = -1;

	mSpreadModels.Caption = "Models";
	mSpreadModels.ColumnAdd( "Preset", 0 );
	mSpreadModels.ColumnAdd( "Width", 1 );
	mSpreadModels.ColumnAdd( "Height", 2 );

	mSpreadModels.GetColumn(-1).Width = 160;
	mSpreadModels.GetColumn(0).Width = 60;
	mSpreadModels.GetColumn(1).Width = 80;
	mSpreadModels.GetColumn(2).Width = 80;

	int ndx = 0;
	for (auto iter = mModels.begin(); iter!=mModels.end(); ++iter)
	{
		FBModel *pModel = iter->pModel;
		mSpreadModels.RowAdd(pModel->Name, (kReference) ndx);

		mSpreadModels.GetCell( ndx, 0 ).Style = kFBCellStyleMenu;
		mSpreadModels.SetCell( ndx, 0, "512~1024~2048~4096~Custom" );
		mSpreadModels.SetCell( ndx, 0, iter->preset );

		mSpreadModels.GetCell( ndx, 1).Style = kFBCellStyleInteger;
		mSpreadModels.SetCell( ndx, 1, iter->width );
		mSpreadModels.GetCell( ndx, 2).Style = kFBCellStyleInteger;
		mSpreadModels.SetCell( ndx, 2, iter->height );

		if (iter->preset < 4)
		{
			mSpreadModels.GetCell( ndx, 1 ).Enabled = false;
			mSpreadModels.GetCell( ndx, 2 ).Enabled = false;
		}
		else
		{
			mSpreadModels.GetCell( ndx, 1 ).Enabled = true;
			mSpreadModels.GetCell( ndx, 2 ).Enabled = true;
		}

		ndx += 1;
	}

}

void ToolBakeProjectors::UpdateSpreadCells()
{
	for (int i=0, count=(int)mModels.size(); i<count; ++i)
	{
		ViewBakeProjectorsData::SpreadModel item = mModels[i];

		mSpreadModels.GetCell( i, 0, item.preset );
		mSpreadModels.GetCell( i, 1, item.width );
		mSpreadModels.GetCell( i, 2, item.height );

		if (item.preset < 4)
		{
			switch(item.preset)
			{
			case 0:
				item.width = item.height = 512;
				break;
			case 1:
				item.width = item.height = 1024;
				break;
			case 2:
				item.width = item.height = 2048;
				break;
			case 3:
				item.width = item.height = 4096;
				break;
			}

			mSpreadModels.SetCell( i, 1, item.width );
			mSpreadModels.GetCell( i, 1 ).Enabled = false;
			mSpreadModels.SetCell( i, 2, item.height );
			mSpreadModels.GetCell( i, 2 ).Enabled = false;
		}
		else
		{
			mSpreadModels.GetCell( i, 1 ).Enabled = true;
			mSpreadModels.GetCell( i, 2 ).Enabled = true;
		}

		mModels[i] = item;
	}
}

void ToolBakeProjectors::EventButtonRemoveAllClick( HISender pSender, HKEvent pEvent )
{
	mSpreadModels.Clear();
	mModels.clear();
}

void ToolBakeProjectors::EventSpreadSpreadChange( HISender pSender, HKEvent pEvent )
{
	FBEventSpread	lEvent(pEvent);

	mCurrentRow = lEvent.Row;

	UpdateSpreadCells();
}

void ToolBakeProjectors::UpdateVisMap(FBModel *parentModel)
{
	if (parentModel == nullptr)
		return;

	mVisMap[parentModel] = (parentModel->Show==true) ? 1 : 0;

	for (int i=0, count=parentModel->Children.GetCount(); i<count; ++i)
		UpdateVisMap( parentModel->Children[i] );

}

void ToolBakeProjectors::ShowModelHeirarchy(FBModel *parentModel, bool show)
{

	if (parentModel == nullptr)
		return;

	parentModel->Show = show;

	for (int i=0, count=parentModel->Children.GetCount(); i<count; ++i)
		ShowModelHeirarchy( parentModel->Children[i], show );

}

void ToolBakeProjectors::ClearVisMap()
{
	mVisMap.clear();
}

void ToolBakeProjectors::EventButtonIsolateClick( HISender pSender, HKEvent pEvent )
{
	if (mVisMap.size() > 0)
	{
		for (auto iter=mVisMap.begin(); iter!=mVisMap.end(); ++iter)
		{
			iter->first->Show = iter->second > 0;
		}
		mVisMap.clear();
	}
	else
	{
		UpdateVisMap(FBSystem::TheOne().SceneRootModel);
		ShowModelHeirarchy(FBSystem::TheOne().SceneRootModel, false);

		for (auto iter=begin(mModels); iter!=end(mModels); ++iter)
			iter->pModel->Show = true;
	}
}

void ToolBakeProjectors::EventButtonBakeFrameClick( HISender pSender, HKEvent pEvent )
{
	if (mModels.size() == 0)
	{
		FBMessageBox( "Bake Tool", "No models to bake!", "Ok" );
		return;
	}

	UpdateViewModels();
	mView.DoGrabCurrentFrame(mView.ImageAddFrameNumber);
}

void ToolBakeProjectors::UpdateViewModels()
{
	mView.ClearModels();
	for (auto iter=begin(mModels); iter!=end(mModels); ++iter)
		mView.AddModel( iter->pModel, iter->preset, iter->width, iter->height );
}

void ToolBakeProjectors::EventButtonBakeRangeClick( HISender pSender, HKEvent pEvent )
{
	if (mModels.size() == 0)
	{
		FBMessageBox( "Bake Tool", "No models to bake!", "Ok" );
		return;
	}

	// DONE: do grab time range

	FBFilePopup		lDialog;

	lDialog.Style = kFBFilePopupSave;
	lDialog.Filter = "*.*";

	FBString fullFilename("");

	if (lDialog.Execute() )
	{
		fullFilename = lDialog.FullFilename;
	}

	if (fullFilename == "")
		return;

	//
	//

	UpdateViewModels();

	//
	//

	FBProgress		lprogress;
	
	mPlayer.GotoStart();
	FBTime currTime = mPlayer.ZoomWindowStart;
	FBTime stopTime = mPlayer.ZoomWindowStop;

	int totalLen = stopTime.GetFrame() - currTime.GetFrame();

	lprogress.Caption = "time range baking";
	lprogress.ProgressBegin();

	int idx = 0;
	FBScene *pFBScene = mSystem.Scene;

	while(currTime <= stopTime)
	{
		mPlayer.Goto(currTime);
		pFBScene->Evaluate();
		pFBScene->EvaluateDeformations();

		mApp.UpdateAllWidgets();
		mApp.FlushEventQueue();

		SetBakingState( eBakingStateWait );

		mView.DoGrabCurrentFrame(true, false, fullFilename);

		mApp.UpdateAllWidgets();
		mApp.FlushEventQueue();

		// wait until view finish saving a file
		
		while (eBakingStateWait == GetBakingState() 
			&& false == lprogress.UserRequestCancell() )
		{
			FBSleep(100);
		}
			

		if (lprogress.UserRequestCancell() == true)
			break;

		currTime = currTime + FBTime(0,0,0,1);
		lprogress.Percent = idx * 100 / totalLen;

		idx++;
	}

	lprogress.ProgressDone();
}

void ToolBakeProjectors::EventButtonAboutClick( HISender pSender, HKEvent pEvent )
{
	FBMessageBox( "Bake Projectors Tool", "Created by Sergey Solohin (Neill3d) 2015\n"
											" MoPLUGS Project\n",
											"Ok" );
}