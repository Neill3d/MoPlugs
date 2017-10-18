
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicmask_tool.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "dynamicmask_tool.h"
#include <Windows.h>
#include <GL\glew.h>

#include "dynamicmask_common.h"
#include "shared_misc_mobu.h"

#include "resourceUtils.h"

//--- Registration defines
#define ORTOOLTEMPLATE__CLASS	ORTOOLTEMPLATE__CLASSNAME
#define ORTOOLTEMPLATE__LABEL	"Dynamic Masks Tool"
#define ORTOOLTEMPLATE__DESC	"Dynamic Masks Tool"

//--- FiLMBOX implementation and registration
FBToolImplementation(	ORTOOLTEMPLATE__CLASS	);
FBRegisterTool		(	ORTOOLTEMPLATE__CLASS,
						ORTOOLTEMPLATE__LABEL,
						ORTOOLTEMPLATE__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ToolDynamicMask::FBCreate()
{
	const int width = 1050;
	const int height = 600;

	// Tool options
	StartSize[0] = width;
	MinSize[0] = width;
	//MaxSize[0] = width;
	StartSize[1] = height;
	MinSize[1] = height;
	//MaxSize[1] = height;

	mSelectedNode = nullptr;

	// Configure layout

	UICreate();
	UIConfig();
	UIReset();

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ToolDynamicMask::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &ToolDynamicMask::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ToolDynamicMask::EventToolResize	);
	OnPaint.Add	( this, (FBCallback) &ToolDynamicMask::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ToolDynamicMask::EventToolInput		);

	FBApplication::TheOne().OnFileNew.Add( this, (FBCallback) &ToolDynamicMask::EventFileNew );
	FBApplication::TheOne().OnFileOpenCompleted.Add( this, (FBCallback) &ToolDynamicMask::EventFileOpenCompleted );

	return true;
}

/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void ToolDynamicMask::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ToolDynamicMask::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ToolDynamicMask::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ToolDynamicMask::EventToolPaint );
	OnInput.Remove	( this, (FBCallback) &ToolDynamicMask::EventToolInput );
	OnResize.Remove	( this, (FBCallback) &ToolDynamicMask::EventToolResize);

	FBApplication::TheOne().OnFileNew.Remove( this, (FBCallback) &ToolDynamicMask::EventFileNew );
	FBApplication::TheOne().OnFileOpenCompleted.Remove( this, (FBCallback) &ToolDynamicMask::EventFileOpenCompleted );

	// Free user allocated memory
}

void ToolDynamicMask::UICreate()
{
	const int operation_width = 120;

	AddRegion( "LayoutToolbar", "LayoutToolbar",
										5,	kFBAttachLeft,	"",	1.0	,
										5,	kFBAttachTop,	"",	1.0,
										-5,	kFBAttachRight,	"",	1.0,
										50,	kFBAttachNone,	"",	1.0 );
	AddRegion( "LayoutStatus", "LayoutStatus",
										0,	kFBAttachLeft,	"",	1.0	,
										-25,	kFBAttachBottom,	"0",	1.0,
										0,	kFBAttachRight,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );
	AddRegion( "LayoutOperation", "LayoutOperation",
										5,	kFBAttachLeft,	"",	1.0	,
										5,	kFBAttachBottom,	"LayoutToolbar",	1.0,
										operation_width,	kFBAttachNone,	"",	1.0,
										240,kFBAttachNone,	"",	1.0 );
	AddRegion( "LayoutNavigator", "LayoutNavigator",
										5,	kFBAttachLeft,	"",	1.0	,
										5,	kFBAttachBottom,	"LayoutOperation",	1.0,
										operation_width,	kFBAttachNone,	"",	1.0,
										-5,	kFBAttachTop,	"LayoutStatus",	1.0 );
	AddRegion( "LayoutProperties", "LayoutProperties",
										-255,	kFBAttachRight,	"",	1.0	,
										5,	kFBAttachBottom,	"LayoutToolbar",	1.0,
										-5,	kFBAttachRight,	"",	1.0,
										-5,	kFBAttachTop,	"LayoutStatus",	1.0 );
	AddRegion( "LayoutView", "LayoutView",
										5,	kFBAttachRight,	"LayoutOperation",	1.0	,
										5,	kFBAttachBottom,	"LayoutToolbar",	1.0,
										-5,	kFBAttachLeft,	"LayoutProperties",	1.0,
										-5,	kFBAttachTop,	"LayoutStatus",	1.0 );
	


	SetControl( "LayoutToolbar", mLayoutToolbar );
	SetControl( "LayoutStatus", mLayoutStatus );
	SetControl( "LayoutOperation", mLayoutOperation );
	SetControl( "LayoutNavigator", mLayoutNavigator );
	SetControl( "LayoutView", mLayoutView );
	SetControl( "LayoutProperties", mLayoutProperties );

	UICreateToolbar();
	UICreateStatus();
	UICreateOperation();
	UICreateNavigator();
	UICreateView();
	UICreateProperties();
}

void ToolDynamicMask::UIConfig()
{
	// Configure button
	SetBorder( "LayoutToolbar", kFBStandardBorder, false,false, 1, 0,90,0 );
	SetBorder( "LayoutStatus", kFBStandardBorder, false,false, 1, 0,90,0 );
	SetBorder( "LayoutOperation", kFBStandardBorder, false,false, 1, 0,90,0 );
	SetBorder( "LayoutNavigator", kFBStandardBorder, false,false, 1, 0,90,0 );
	SetBorder( "LayoutView", kFBStandardBorder, false,false, 1, 0,90,0 );
	SetBorder( "LayoutProperties", kFBStandardBorder, false,false, 1, 0,90,0 );

	UIConfigToolbar();
	UIConfigStatus();
	UIConfigOperation();
	UIConfigNavigator();
	UIConfigView();
	UIConfigProperties();
}

void ToolDynamicMask::UIReset()
{
	UIResetToolbar();
	UIResetStatus();
	UIResetOperation();
	UIResetNavigator();
	UIResetView();
	UIResetProperties();
}

void ToolDynamicMask::UICreateToolbar()
{
	mLayoutToolbar.AddRegion( "LabelMask", "LabelMask",
										5,	kFBAttachLeft,	"",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										80,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ListMask", "ListMask",
										5,	kFBAttachRight,	"LabelMask",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										120,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonAddMask", "ButtonAddMask",
										5,	kFBAttachRight,	"ListMask",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										32,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonRemoveMask", "ButtonRemoveMask",
										5,	kFBAttachRight,	"ButtonAddMask",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										32,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonRenameMask", "ButtonRenameMask",
										5,	kFBAttachRight,	"ButtonRemoveMask",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										32,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonDublicateMask", "ButtonDublicateMask",
										5,	kFBAttachRight,	"ButtonRenameMask",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										32,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonSelectMask", "ButtonSelectMask",
										5,	kFBAttachRight,	"ButtonDublicateMask",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										32,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonRegenerateMask", "ButtonRegenerateMask",
										5,	kFBAttachRight,	"ButtonSelectMask",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										32,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );

	mLayoutToolbar.AddRegion( "LabelModes", "LabelModes",
										25,	kFBAttachRight,	"ButtonRegenerateMask",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										60,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonSelect", "ButtonSelect",
										5,	kFBAttachRight,	"LabelModes",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										60,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonDrag", "ButtonDrag",
										5,	kFBAttachRight,	"ButtonSelect",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										60,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonMove", "ButtonMove",
										5,	kFBAttachRight,	"ButtonDrag",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										60,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonScale", "ButtonScale",
										5,	kFBAttachRight,	"ButtonMove",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										60,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutToolbar.AddRegion( "ButtonAddShape", "ButtonAddShape",
										50,	kFBAttachRight,	"ButtonScale",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										90,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );

	mLayoutToolbar.AddRegion( "ButtonAbout", "ButtonAbout",
										-100,	kFBAttachRight,	"",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										90,	kFBAttachNone,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );

	//

	mLayoutToolbar.AddRegion( "LabelTransform", "LabelTransform",
										-150,	kFBAttachLeft,	"LabelModes",	1.0	,
										5,	kFBAttachBottom,"LabelModes",	1.0,
										60, kFBAttachNone,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	mLayoutToolbar.AddRegion( "ButtonTransformAll", "ButtonTransformAll",
										5,	kFBAttachRight,	"LabelTransform",	1.0	,
										5,	kFBAttachBottom,"LabelModes",	1.0,
										60, kFBAttachNone,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	mLayoutToolbar.AddRegion( "ButtonTransformPoints", "ButtonTransformPoints",
										5,	kFBAttachRight,	"ButtonTransformAll",	1.0	,
										5,	kFBAttachBottom,"LabelModes",	1.0,
										60, kFBAttachNone,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	mLayoutToolbar.AddRegion( "ButtonTransformTangents", "ButtonTransformTangents",
										5,	kFBAttachRight,	"ButtonTransformPoints",	1.0	,
										5,	kFBAttachBottom,"LabelModes",	1.0,
										60, kFBAttachNone,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	//

	mLayoutToolbar.AddRegion( "LabelType", "LabelType",
										100,	kFBAttachRight,	"ButtonTransformTangents",	1.0	,
										5,	kFBAttachBottom,"LabelModes",	1.0,
										60, kFBAttachNone,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	mLayoutToolbar.AddRegion( "ButtonTypeCorner", "ButtonTypeCorner",
										5,	kFBAttachRight,	"LabelType",	1.0	,
										5,	kFBAttachBottom,"LabelModes",	1.0,
										60, kFBAttachNone,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	mLayoutToolbar.AddRegion( "ButtonTypeSmooth", "ButtonTypeSmooth",
										5,	kFBAttachRight,	"ButtonTypeCorner",	1.0	,
										5,	kFBAttachBottom,"LabelModes",	1.0,
										60, kFBAttachNone,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	mLayoutToolbar.AddRegion( "ButtonTypeBezier", "ButtonTypeBezier",
										5,	kFBAttachRight,	"ButtonTypeSmooth",	1.0	,
										5,	kFBAttachBottom,"LabelModes",	1.0,
										60, kFBAttachNone,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	mLayoutToolbar.SetControl( "LabelMask", mLabelMask );
	mLayoutToolbar.SetControl( "ListMask", mListMask );
	mLayoutToolbar.SetControl( "ButtonAddMask", mButtonAddMask );
	mLayoutToolbar.SetControl( "ButtonRemoveMask", mButtonRemoveMask );
	mLayoutToolbar.SetControl( "ButtonRenameMask", mButtonRenameMask );
	mLayoutToolbar.SetControl( "ButtonDublicateMask", mButtonDublicateMask );
	mLayoutToolbar.SetControl( "ButtonSelectMask", mButtonSelectMask );
	mLayoutToolbar.SetControl( "ButtonRegenerateMask", mButtonRegenerateMask );

	mLayoutToolbar.SetControl( "LabelModes", mLabelModes );
	mLayoutToolbar.SetControl( "ButtonSelect", mButtonSelect );
	mLayoutToolbar.SetControl( "ButtonDrag", mButtonDrag );
	mLayoutToolbar.SetControl( "ButtonMove", mButtonMove );
	//mLayoutToolbar.SetControl( "ButtonRotate", mButtonRotate );
	//mLayoutToolbar.SetControl( "ButtonScale", mButtonScale );
	mLayoutToolbar.SetControl( "ButtonAddShape", mButtonAddShape );
	mLayoutToolbar.SetControl( "ButtonAbout", mButtonAbout );


	mLayoutToolbar.SetControl( "LabelTransform", mLabelTransform );
	mLayoutToolbar.SetControl( "ButtonTransformAll", mButtonTransformAll );
	mLayoutToolbar.SetControl( "ButtonTransformPoints", mButtonTransformPoints );
	mLayoutToolbar.SetControl( "ButtonTransformTangents", mButtonTransformTangents );

	mLayoutToolbar.SetControl( "LabelType", mLabelPointType );
	mLayoutToolbar.SetControl( "ButtonTypeCorner", mButtonTypeCorner );
	mLayoutToolbar.SetControl( "ButtonTypeSmooth", mButtonTypeSmooth );
	mLayoutToolbar.SetControl( "ButtonTypeBezier", mButtonTypeBezier );
}

void ToolDynamicMask::UICreateStatus()
{
	mLayoutStatus.AddRegion( "Label", "Label",
										5,	kFBAttachLeft,	"",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										600,	kFBAttachNone,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	mLayoutStatus.SetControl( "Label", mLabelStatus );
}

void ToolDynamicMask::UICreateOperation()
{
	mLayoutOperation.AddRegion( "Split", "Split",
										5,	kFBAttachLeft,	"",	1.0	,
										5,	kFBAttachTop,	"",	1.0,
										-5,	kFBAttachRight,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutOperation.AddRegion( "Remove", "Remove",
										5,	kFBAttachLeft,	"",	1.0	,
										5,	kFBAttachBottom,	"Split",	1.0,
										-5,	kFBAttachRight,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutOperation.AddRegion( "RemoveShape", "RemoveShape",
										5,	kFBAttachLeft,	"",	1.0	,
										5,	kFBAttachBottom,	"Remove",	1.0,
										-5,	kFBAttachRight,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );

	// animation regions

	mLayoutOperation.AddRegion( "AutoKey", "AutoKey",
										5,	kFBAttachLeft,	"",	1.0	,
										35,	kFBAttachBottom,	"RemoveShape",	1.0,
										-5,	kFBAttachRight,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutOperation.AddRegion( "KeyAll", "KeyAll",
										5,	kFBAttachLeft,	"",	1.0	,
										5,	kFBAttachBottom,	"AutoKey",	1.0,
										-5,	kFBAttachWidth,	"",	0.5,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutOperation.AddRegion( "KeySelected", "KeySelected",
										5,	kFBAttachRight,		"KeyAll",	1.0	,
										5,	kFBAttachBottom,	"AutoKey",	1.0,
										-5,	kFBAttachRight,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutOperation.AddRegion( "KeyPrev", "KeyPrev",
										5,	kFBAttachLeft,	"",	1.0	,
										5,	kFBAttachBottom,	"KeyAll",	1.0,
										-5,	kFBAttachWidth,	"",	0.33,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutOperation.AddRegion( "KeyRemove", "KeyRemove",
										5,	kFBAttachRight,	"KeyPrev",	1.0	,
										5,	kFBAttachBottom,	"KeyAll",	1.0,
										-5,	kFBAttachWidth,	"",	0.33,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutOperation.AddRegion( "KeyNext", "KeyNext",
										5,	kFBAttachRight,	"KeyRemove",	1.0	,
										5,	kFBAttachBottom,	"KeyAll",	1.0,
										-5,	kFBAttachRight,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );
	mLayoutOperation.AddRegion( "ClearAnimation", "ClearAnimation",
										5,	kFBAttachLeft,	"",	1.0	,
										5,	kFBAttachBottom,	"KeyPrev",	1.0,
										-5,	kFBAttachRight,	"",	1.0,
										25,	kFBAttachNone,	"",	1.0 );


	mLayoutOperation.SetControl( "Split", mButtonSplitShape );
	mLayoutOperation.SetControl( "Remove", mButtonRemoveVertex );
	mLayoutOperation.SetControl( "RemoveShape", mButtonRemoveShape );

	// animation controls
	mLayoutOperation.SetControl( "AutoKey", mButtonAutoKey );
	mLayoutOperation.SetControl( "KeyAll", mButtonKeyAll );
	mLayoutOperation.SetControl( "KeySelected", mButtonKeySet );
	mLayoutOperation.SetControl( "KeyPrev", mButtonKeyPrev );
	mLayoutOperation.SetControl( "KeyRemove", mButtonKeyRemove );
	mLayoutOperation.SetControl( "KeyNext", mButtonKeyNext);
	mLayoutOperation.SetControl( "ClearAnimation", mButtonClearAnimation);
}

void ToolDynamicMask::UICreateNavigator()
{
	mLayoutNavigator.AddRegion( "Tree", "Tree",
										0,	kFBAttachLeft,	"",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										0,	kFBAttachRight,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	mLayoutNavigator.SetControl( "Tree", mTreeNavigator );
}

void ToolDynamicMask::UICreateView()
{
	mLayoutView.AddRegion( "Tab", "Tab",
										0,	kFBAttachLeft,	"",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										0,	kFBAttachRight,	"",	1.0,
										35,	kFBAttachNone,	"",	1.0 );
	mLayoutView.AddRegion( "View", "View",
										2,	kFBAttachLeft,	"",	1.0	,
										2,	kFBAttachBottom,	"Tab",	1.0,
										-2,	kFBAttachRight,	"",	1.0,
										-2,	kFBAttachBottom,	"",	1.0 );

	mLayoutView.SetControl( "Tab", mTabPanelView );
	mLayoutView.SetView( "View", mView );
}

void ToolDynamicMask::UICreateProperties()
{
	mLayoutProperties.AddRegion( "Properties", "Properties",
										0,	kFBAttachLeft,	"",	1.0	,
										0,	kFBAttachTop,	"",	1.0,
										0,	kFBAttachRight,	"",	1.0,
										0,	kFBAttachBottom,	"",	1.0 );

	mLayoutProperties.SetControl( "Properties", mProperties );
}

//

void ToolDynamicMask::UIConfigToolbar()
{
	mLabelMask.Caption = "Current Mask: ";

	mListMask.OnChange.Add( this, (FBCallback) &ToolDynamicMask::EventMask );

	mButtonAddMask.Caption = "+";
	mButtonAddMask.Hint = "Add a new dynamic mask";
	mButtonAddMask.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventMask );
	mButtonRemoveMask.Caption = "-";
	mButtonRemoveMask.Hint = "Remove the current mask";
	mButtonRemoveMask.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventMask );
	mButtonRenameMask.Caption = "R";
	mButtonRenameMask.Hint = "Rename the current mask";
	mButtonRenameMask.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventMask );
	mButtonDublicateMask.Caption = "D";
	mButtonDublicateMask.Hint = "Dublicate the current mask";
	mButtonDublicateMask.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventMask );
	mButtonSelectMask.Caption = "S";
	mButtonSelectMask.Hint = "Select current mask components (channels, shape, knots) or select mask video resources";
	mButtonSelectMask.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventMask );
	mButtonRegenerateMask.Caption = "Rg";
	mButtonRegenerateMask.Hint = "Regenerate current mask dynamic texture and video resources";
	mButtonRegenerateMask.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventMask );

	mLabelModes.Caption = "Modes:";
	
	mButtonSelect.Caption = "Select";
	mButtonSelect.Hint = "Enter the select shape knots mode";
	mButtonSelect.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );
	mButtonSelect.State = 1;
	mButtonDrag.Caption = "Drag";
	mButtonDrag.Hint = "Enter the drag shape knots mode";
	mButtonDrag.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );
	mButtonMove.Caption = "Move";
	mButtonMove.Hint = "Enter the move shape knots mode";
	mButtonMove.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );
	mButtonScale.Caption = "Scale";
	mButtonScale.Hint = "Enter the scale shape knots mode";
	mButtonScale.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );
	mButtonAddShape.Caption = "Add Shape >";
	mButtonAddShape.Hint = "Open a menu with possible primitive creation modes";
	mButtonAddShape.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );

	mLabelTransform.Caption = "Transform:";

	mButtonTransformAll.Caption = "All";
	mButtonTransformAll.Hint = "Transform will affect points and tangents";
	mButtonTransformAll.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );
	mButtonTransformAll.State = 1;
	mButtonTransformPoints.Caption = "Points";
	mButtonTransformPoints.Hint = "Transform will affect points only";
	mButtonTransformPoints.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );
	mButtonTransformTangents.Caption = "Tangents";
	mButtonTransformTangents.Hint = "Transform will affect tangents only";
	mButtonTransformTangents.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );

	mLabelPointType.Caption = "Type:";

	mButtonTypeCorner.Caption = "Corner";
	mButtonTypeCorner.Hint = "Change interpolation type for selected points to corner";
	mButtonTypeCorner.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );
	mButtonTypeSmooth.Caption = "Smooth";
	mButtonTypeSmooth.Hint = "Change interpolation type for selected points to smooth";
	mButtonTypeSmooth.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );
	mButtonTypeBezier.Caption = "Bezier";
	mButtonTypeBezier.Hint = "Change interpolation type for selected points to bezier";
	mButtonTypeBezier.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );

	//
	// context menu
	//
	mMenuShapes.InsertLast( "Add a new rectangle", 0 );
	mMenuShapes.InsertLast( "Add a new circle", 1 );
	mMenuShapes.InsertLast( "Add a new spline poly", 2 );
	mMenuShapes.InsertLast( "Add a new free spline", 3 );
	
	mMenuShapes.OnMenuActivate.Add( this, (FBCallback) &ToolDynamicMask::OnMenuActivate );

	mButtonAbout.Caption = "About";
	mButtonAbout.Hint = "Show an about dialog box";
	mButtonAbout.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventManipulator );
}

void ToolDynamicMask::UIConfigStatus()
{
	mLabelStatus.Caption = "Hint information here";
}

void ToolDynamicMask::UIConfigOperation()
{
	mButtonSplitShape.Caption = "Split edge";
	mButtonSplitShape.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventOperation );

	mButtonRemoveVertex.Caption = "Remove vertex";
	mButtonRemoveVertex.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventOperation );

	mButtonRemoveShape.Caption = "Remove shape";
	mButtonRemoveShape.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventOperation );

	mButtonAutoKey.Caption = "Auto Key";
	mButtonAutoKey.Style = kFB2States;
	mButtonAutoKey.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventOperation );

	mButtonKeyAll.Caption = "Key All";
	mButtonKeyAll.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventOperation );
	mButtonKeySet.Caption = "Key Sel";
	mButtonKeySet.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventOperation );
	mButtonKeyPrev.Caption = "<";
	mButtonKeyPrev.Hint = "Go to previous keyframe of a shape control point";
	mButtonKeyPrev.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventOperation );
	mButtonKeyRemove.Caption = "X";
	mButtonKeyRemove.Hint = "Remove keyframes at current time from selection or from all existing shape control points";
	mButtonKeyRemove.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventOperation );
	mButtonKeyNext.Caption = ">";
	mButtonKeyNext.Hint = "Go to next keyframe of a shape control point";
	mButtonKeyNext.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventOperation );
	mButtonClearAnimation.Caption = "Clear animation";
	mButtonClearAnimation.OnClick.Add( this, (FBCallback) &ToolDynamicMask::EventOperation );
}

void ToolDynamicMask::UIConfigNavigator()
{
	mTreeNavigator.OnSelect.Add( this, (FBCallback) &ToolDynamicMask::EventTree );
	mTreeNavigator.OnDblClick.Add( this, (FBCallback) &ToolDynamicMask::EventTreeDblClick );
}

void ToolDynamicMask::UIConfigView()
{
	mTabPanelView.Items.SetString( "R~G~B~A~All in One" );
	mTabPanelView.OnChange.Add( this, (FBCallback) &ToolDynamicMask::EventChannelTab );

	mView.OnUpdateUI.Bind( this, &ToolDynamicMask::OnUpdateUI );
	mView.mBrowsingProperty = &mProperties;
}

void ToolDynamicMask::UIConfigProperties()
{

}

//

void ToolDynamicMask::UIResetToolbar()
{
	UpdateMaskList();

	//
	//
	const int currentManipulator = mView.GetCurrentManipulator();
	const int currentTransformMode = mView.GetCurrentTransformMode();

	mButtonSelect.State = 0;
	mButtonDrag.State = 0;
	mButtonMove.State = 0;

	switch (currentManipulator)
	{
	case 2:
		mButtonSelect.State = 1;
		break;
	case 3:
		mButtonDrag.State = 1;
		break;
	case 4:
		mButtonMove.State = 1;
		break;
	}

	//
	//

	mButtonTransformAll.State = 0;
	mButtonTransformPoints.State = 0;
	mButtonTransformTangents.State = 0;

	switch (currentTransformMode)
	{
	case MANIPULATOR_TRANSFORM_ALL:
		mButtonTransformAll.State = 1;
		break;
	case MANIPULATOR_TRANSFORM_POINTS:
		mButtonTransformPoints.State = 1;
		break;
	case MANIPULATOR_TRANSFORM_TANGENTS:
		mButtonTransformTangents.State = 1;
		break;
	}
}

void ToolDynamicMask::UIResetStatus()
{
}

void ToolDynamicMask::UIResetOperation()
{
}

void ToolDynamicMask::UIResetNavigator()
{
	UpdateTree();
}

void ToolDynamicMask::UIResetView()
{
}

void ToolDynamicMask::UIResetProperties()
{
}

/************************************************
 *	Handle all mask control elements
 ************************************************/
void ToolDynamicMask::EventMask( HISender pSender, HKEvent pEvent )
{
	FBComponent* lComp = FBGetFBComponent( (HIObject) pSender ); 
	
	char strValue[128];
	memset( strValue, 0, sizeof(char) * 128 );

	ObjectMask *pCurrMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();

	if (lComp == &mButtonAddMask)
	{
		sprintf_s( strValue, 128, "%s\0", "DynamicMask" );

		if (1 == FBMessageBoxGetUserValue( "Dynamic mask", "Enter a new mask name:", strValue, kFBPopupString, "Ok", "Cancel" ) )
		{
			ObjectMask *newMask = DYNAMIC_MASK_OPERATIONS::AddMask(strValue);
			//newMask->Enable = false;
			DYNAMIC_MASK_OPERATIONS::ChooseMask(newMask);
		}
	}
	else if (lComp == &mButtonRemoveMask && pCurrMask )
	{
		if (1 == FBMessageBox("Dynamic Mask", "Are you sure you want to run remove operation in one of modes (current, all, unused)?", "Yes", "Cancel" ) )
		{
			int mode = FBMessageBox( "Dynamic Mask", "Please select a remove mode", "Current", "All", "Unused" );

			switch(mode)
			{
			case 1: DYNAMIC_MASK_OPERATIONS::RemoveMask( pCurrMask );
				break;
			case 2: DYNAMIC_MASK_OPERATIONS::RemoveAllMasks();
				break;
			case 3: DYNAMIC_MASK_OPERATIONS::RemoveUnusedMasks();
				break;
			}
		}
	}
	else if (lComp == &mButtonRenameMask && pCurrMask )
	{
		FBString name = pCurrMask->Name;
		sprintf_s(strValue, 128, "%s\0", name );
		if (1 == FBMessageBoxGetUserValue( "Dynamic mask", "Enter a new mask name:", strValue, kFBPopupString, "Ok", "Cancel" ) )
		{
			DYNAMIC_MASK_OPERATIONS::RenameMask( pCurrMask, strValue);
		}
	}
	else if (lComp == &mButtonDublicateMask && pCurrMask )
	{
		if (1 == FBMessageBox("Dynamic Mask", "Are you sure you want to dublicate the current mask?", "Yes", "Cancel" ) )
		{
			DYNAMIC_MASK_OPERATIONS::DublicateMask( pCurrMask );
		}
	}
	else if (lComp == &mButtonSelectMask && pCurrMask )
	{
		int mode = FBMessageBox("Dynamic Mask", "Do you want to select all masks components or only mask video resources ?", "Components", "Resources", "Cancel" );
		
		if (mode == 1) 
		{
			DYNAMIC_MASK_OPERATIONS::SelectMask( pCurrMask );
		}
		else if (mode == 2)
		{
			DYNAMIC_MASK_OPERATIONS::SelectMaskResources( pCurrMask );
		}
	}
	else if (lComp == &mButtonRegenerateMask && pCurrMask )
	{
		DYNAMIC_MASK_OPERATIONS::RegenerateMask( pCurrMask );
	}
	else if (lComp == &mListMask && mListMask.ItemIndex >= 0)
	{
		// change mask list event
		FBString compName = mListMask.Items[mListMask.ItemIndex];

		DYNAMIC_MASK_OPERATIONS::ChooseMaskByName(compName);
		PropertiesClearAll( &mProperties );
	}

	UpdateMaskList();
	UpdateTree();
}

/************************************************
 *	Handle navigator tree events
 ************************************************/
void ToolDynamicMask::EventTree( HISender pSender, HKEvent pEvent )
{
	FBEventTreeSelect	e(pEvent);

	mSelectedNode = nullptr;

	if (e.TreeNode)
	{
		FBTreeNode *pNode = e.TreeNode;
		kReference ref = pNode->Reference;
		
		if (ref)
		{
			FBComponent *pComp = (FBComponent*) ref;
			PropertiesAddExclusive(&mProperties, pComp);

			mSelectedNode = pNode;
		}
	}
}

void ToolDynamicMask::EventTreeDblClick( HISender pSender, HKEvent pEvent )
{
	FBEventTreeSelect	e(pEvent);

	if (e.TreeNode)
	{
		FBTreeNode *pNode = e.TreeNode;
		kReference ref = pNode->Reference;
		
		if (ref)
		{
			FBComponent *pComp = (FBComponent*) ref;
			if (pComp->Is(ObjectShape::TypeInfo) )
			{
				DoRenameShape( (ObjectShape*) pComp );
				pNode->Name = pComp->Name;
			}
		}
	}
}

/************************************************
 *	Handle operation buttons
 ************************************************/
void ToolDynamicMask::EventOperation( HISender pSender, HKEvent pEvent )
{
	FBComponent* lComp = FBGetFBComponent( (HIObject) pSender ); 

	if (lComp == &mButtonSplitShape) 
	{
		DYNAMIC_MASK_OPERATIONS::SplitEdge();
	}
	else if (lComp == &mButtonRemoveVertex) 
	{
		bool updateUI;
		DYNAMIC_MASK_OPERATIONS::RemoveVertices(updateUI);

		if (updateUI)
			UpdateTree();
	}
	else if (lComp == &mButtonRemoveShape) 
	{
		if (mSelectedNode)
		{
			kReference ref = mSelectedNode->Reference;
			if (ref)
			{
				FBComponent *pComp = (FBComponent*) ref;
				if (FBIS( pComp, ObjectShape ) )
				{
					// unparent
					PropertiesClearAll( &mProperties );
					DYNAMIC_MASK_OPERATIONS::RemoveShape( (ObjectShape*) pComp );
					UpdateTree();
				}
			}
		}
	}
	else if (lComp == &mButtonAutoKey)
	{
		DYNAMIC_MASK_OPERATIONS::SetAutoKey( mButtonAutoKey.State == 1 );
	}
	else if (lComp == &mButtonKeyAll)
	{
		DYNAMIC_MASK_OPERATIONS::KeyAll();
	}
	else if (lComp == &mButtonKeySet)
	{
		DYNAMIC_MASK_OPERATIONS::KeySelected();
	}
	else if (lComp == &mButtonKeyPrev)
	{
		DYNAMIC_MASK_OPERATIONS::GoToPrevKey();
	}
	else if (lComp == &mButtonKeyRemove)
	{
		DYNAMIC_MASK_OPERATIONS::KeyRemove();
	}
	else if (lComp == &mButtonKeyNext)
	{
		DYNAMIC_MASK_OPERATIONS::GoToNextKey();
	}
	else if (lComp == &mButtonClearAnimation)
	{
		if (1 == FBMessageBox( "Dynamic Masks Tool", "Do you want to clear animation for control points?", "Ok", "Cancel" ) )
			DYNAMIC_MASK_OPERATIONS::ClearAnimation();
	}
}


/************************************************
 *	Handle all manipulator buttons
 ************************************************/
void ToolDynamicMask::EventManipulator( HISender pSender, HKEvent pEvent )
{
	FBComponent* lComp = FBGetFBComponent( (HIObject) pSender ); 

	if (lComp == &mButtonSelect) 
	{
		mView.SetManipulator( MANIPULATOR_GRAPH_SELECT );
	}
	else if (lComp == &mButtonDrag) 
	{
		mView.SetManipulator( MANIPULATOR_GRAPH_DRAG );
	}
	else if (lComp == &mButtonMove) 
	{
		mView.SetManipulator( MANIPULATOR_GRAPH_MOVE );
	}
	else if (lComp == &mButtonAddShape) {
		POINT p;
		if (GetCursorPos(&p))
		{
			mMenuShapes.Execute(p.x, p.y);
		}

		//mView.SetManipulator( MANIPULATOR_GRAPH_SHAPE );
	}
	else if (lComp == &mButtonAbout) {
		const char *szVersion = LoadVersionFromResource();
		FBMessageBox( FBString("Dynamic Masks Tool ", szVersion), "\tMoPlugs Project\nAuthor Sergey Solohin (Neill3d) 2014\n e-mail to s@neill3d.com\n www.neill3d.com", "Ok" );
	}
	else if (lComp == &mButtonTransformAll) {
		mView.SetTransformMode( MANIPULATOR_TRANSFORM_ALL );
	}
	else if (lComp == &mButtonTransformPoints) {
		mView.SetTransformMode( MANIPULATOR_TRANSFORM_POINTS );
	}
	else if (lComp == &mButtonTransformTangents) {
		mView.SetTransformMode( MANIPULATOR_TRANSFORM_TANGENTS );
	}
	else if (lComp == &mButtonTypeCorner)
	{
		DYNAMIC_MASK_OPERATIONS::ChangeInterpolationType(0);
	}
	else if (lComp == &mButtonTypeSmooth)
	{
		DYNAMIC_MASK_OPERATIONS::ChangeInterpolationType(2);
	}
	else if (lComp == &mButtonTypeBezier)
	{
		DYNAMIC_MASK_OPERATIONS::ChangeInterpolationType(1);
	}
}

void ToolDynamicMask::OnUpdateUI(int type)
{
	switch(type)
	{
	case UPDATE_UI_ALL:
		UpdateManipulatorButtons();
		UpdateTree();
		break;
	case UPDATE_UI_TOOLBAR: UpdateManipulatorButtons();
		break;
	case UPDATE_UI_TREE: UpdateTree();
		break;
	}
	
}

/************************************************
 *	Show shapes context menu
 ************************************************/
void ToolDynamicMask::OnMenuActivate( HISender pSender, HKEvent pEvent )
{
	FBEventMenu	e(pEvent);

	switch (e.Id)
	{
	case 0: mView.SetManipulator( MANIPULATOR_GRAPH_SHAPE_RECT );
		break;
	case 1: mView.SetManipulator( MANIPULATOR_GRAPH_SHAPE_CIRCLE );
		break;
	case 2: mView.SetManipulator( MANIPULATOR_GRAPH_SHAPE_SPLINE );
		break;
	}
}


void ToolDynamicMask::EventChannelTab( HISender pSender, HKEvent pEvent )
{
	DYNAMIC_MASK_OPERATIONS::ChooseChannel( mTabPanelView.ItemIndex );
}

/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ToolDynamicMask::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		if (DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr() == nullptr) 
		{
			DYNAMIC_MASK_OPERATIONS::MakeSomeMaskCurrent();
		}

		// Reset the UI here.
		UIReset();
		//OnUpdateUI( UPDATE_UI_ALL );
	}
	else
	{
	}
}


/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void ToolDynamicMask::EventToolPaint( HISender pSender, HKEvent pEvent )
{
	
}


/************************************************
 *	Tool resize callback.
 ************************************************/
void ToolDynamicMask::EventToolResize( HISender pSender, HKEvent pEvent )
{
	int x, y, w, h;
	mLayoutView.GetRegionPositions( "View", true, &x, &y, &w, &h );
	mView.ViewReSize(w-x, h-y);
}


/************************************************
 *	Handle input into the tool.
 ************************************************/
void ToolDynamicMask::EventToolInput( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Handle idle cycle
 ************************************************/
void ToolDynamicMask::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	//mView.ViewExpose();
	mView.Refresh();
	mLabelStatus.Caption = mView.mStatusLine;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool ToolDynamicMask::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ToolDynamicMask::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

void ToolDynamicMask::EventFileNew ( HISender pSender, HKEvent pEvent )
{
	// update UI for the empty channels
	DYNAMIC_MASK_OPERATIONS::ChooseMask(nullptr);
	UIReset();
	OnUpdateUI(UPDATE_UI_ALL);
}

void ToolDynamicMask::EventFileOpenCompleted( HISender pSender, HKEvent pEvent )
{
	if (DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr() == nullptr)
	{
		DYNAMIC_MASK_OPERATIONS::MakeSomeMaskCurrent();
	}

	UIReset();
	OnUpdateUI(UPDATE_UI_ALL);
}

void ToolDynamicMask::UpdateMaskList()
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	mListMask.Items.Clear();

	int itemIndex = -1;
	
	for (int i=0; i<pScene->UserObjects.GetCount(); ++i)
		if (pScene->UserObjects[i]->Is( ObjectMask::TypeInfo ) )
		{
			int idx = mListMask.Items.Add(pScene->UserObjects[i]->Name, mListMask.Items.GetCount());
			
			if (pScene->UserObjects[i] == DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr() ) 
				itemIndex = idx;
		}
	
	if (itemIndex >= 0) {
		mListMask.ItemIndex = itemIndex;

		mTabPanelView.ItemIndex = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelIndex();
	}
}



void ToolDynamicMask::DoRenameShape(ObjectShape *shape)
{
	char strValue[128];
	memset( strValue, 0, sizeof(char) * 128 );

	FBString name = shape->Name;
	sprintf_s(strValue, 128, "%s\0", name );
	if (1 == FBMessageBoxGetUserValue( "Dynamic mask", "Enter a new shape name:", strValue, kFBPopupString, "Ok", "Cancel" ) )
	{
		shape->Name = strValue;
	}
}

void ToolDynamicMask::UpdateTree()
{
	mTreeNavigator.Clear();
	mSelectedNode = nullptr;

	// add mask
	ObjectMask *currMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
	if (currMask)
	{
		FBTreeNode *pNode = mTreeNavigator.InsertLast( mTreeNavigator.GetRoot(), currMask->Name );
		if (pNode) {
			pNode->Reference = (kReference) currMask;

			// add each channel
			for (int i=0; i<currMask->Components.GetCount(); ++i)
			{
				FBComponent *pComp = currMask->Components[i];

				FBTreeNode *pChannelNode = mTreeNavigator.InsertLast( pNode, pComp->Name );
				if (pChannelNode) {
					pChannelNode->Reference = (kReference) pComp;

					// add each shape
					for (int j=0; j<pComp->Components.GetCount(); ++j)
					{
						FBComponent *pShape = pComp->Components[j];

						FBTreeNode *pShapeNode = mTreeNavigator.InsertLast( pChannelNode, pShape->Name );
						if (pShapeNode) {
							pShapeNode->Reference = (kReference) pShape;
						}
					}
				}
			}
		}
	}
}


void ToolDynamicMask::UpdateManipulatorButtons()
{
	mButtonSelect.State = 0;
	mButtonDrag.State = 0;
	mButtonMove.State = 0;
	mButtonScale.State = 0;
	mButtonAddShape.State = 0;

	switch(mView.GetCurrentManipulator() )
	{
	case 2: mButtonSelect.State = 1;
		break;
	case 3: mButtonDrag.State = 1;
		break;
	case 4: mButtonMove.State = 1;
		break;
	case 5: mButtonAddShape.State = 1;
		break;
	}

	//
	//

	mButtonTransformAll.State = 0;
	mButtonTransformPoints.State = 0;
	mButtonTransformTangents.State = 0;

	switch (mView.GetCurrentTransformMode() )
	{
	case MANIPULATOR_TRANSFORM_ALL:
		mButtonTransformAll.State = 1;
		break;
	case MANIPULATOR_TRANSFORM_POINTS:
		mButtonTransformPoints.State = 1;
		break;
	case MANIPULATOR_TRANSFORM_TANGENTS:
		mButtonTransformTangents.State = 1;
		break;
	}

}