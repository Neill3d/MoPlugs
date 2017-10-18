#ifndef __DYNAMICMASK_TOOL_H__
#define __DYNAMICMASK_TOOL_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicmask_tool.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>
#include "dynamicmask_object.h"
#include "dynamicmask_view.h"
#include "dynamicmask_viewTools.h"

//--- Registration define
#define ORTOOLTEMPLATE__CLASSNAME	ToolDynamicMask
#define ORTOOLTEMPLATE__CLASSSTR	"ToolDynamicMask"

/**	Tool template.
*/
class ToolDynamicMask : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( ToolDynamicMask, FBTool );

public:
	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore		( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	void UICreate();
	void UIConfig();
	void UIReset();

private:

	void		EventToolIdle		( HISender pSender, HKEvent pEvent );
	void		EventToolShow		( HISender pSender, HKEvent pEvent );
	void		EventToolPaint		( HISender pSender, HKEvent pEvent );
	void		EventToolResize		( HISender pSender, HKEvent pEvent );
	void		EventToolInput		( HISender pSender, HKEvent pEvent );

	void		EventFileNew		( HISender pSender, HKEvent pEvent );
	void		EventFileOpenCompleted		( HISender pSender, HKEvent pEvent );

	void		EventMask			( HISender pSender, HKEvent pEvent );
	void		EventManipulator	( HISender pSender, HKEvent pEvent );
	void		EventTree			( HISender pSender, HKEvent pEvent );
	void		EventTreeDblClick	( HISender pSender, HKEvent pEvent );
	void		EventChannelTab		( HISender pSender, HKEvent pEvent );

	void		EventOperation		( HISender pSender, HKEvent pEvent );

	void		OnMenuActivate	( HISender pSender, HKEvent pEvent );

	void		OnUpdateUI	(int type);

	void UICreateToolbar();
	void UIConfigToolbar();
	void UIResetToolbar();
	
	void UICreateOperation();
	void UIConfigOperation();
	void UIResetOperation();
	
	void UICreateNavigator();
	void UIConfigNavigator();
	void UIResetNavigator();
	
	void UICreateView();
	void UIConfigView();
	void UIResetView();

	void UICreateProperties();
	void UIConfigProperties();
	void UIResetProperties();

	void UICreateStatus();
	void UIConfigStatus();
	void UIResetStatus();

private:
	// layout 1 - main toolbar
	FBLayout	mLayoutToolbar;
	
	FBLabel		mLabelMask;
	FBList		mListMask;
	FBButton	mButtonAddMask;
	FBButton	mButtonRemoveMask;
	FBButton	mButtonRenameMask;
	FBButton	mButtonDublicateMask;
	FBButton	mButtonSelectMask;
	FBButton	mButtonRegenerateMask;

	FBLabel		mLabelModes;
	FBButton	mButtonSelect;
	FBButton	mButtonDrag;
	FBButton	mButtonMove;
	FBButton	mButtonScale;
	FBButton	mButtonAddShape;
	FBButton	mButtonAbout;

	FBLabel		mLabelTransform;
	FBButton	mButtonTransformAll;
	FBButton	mButtonTransformPoints;
	FBButton	mButtonTransformTangents;

	FBLabel		mLabelPointType;
	FBButton	mButtonTypeCorner;
	FBButton	mButtonTypeSmooth;
	FBButton	mButtonTypeBezier;

	FBGenericMenu		mMenuShapes;

	// layout 2 - operation panel
	
	FBLayout	mLayoutOperation;

	FBLayoutRegion	mRegionEdit;
	FBButton	mButtonSplitShape;
	FBButton	mButtonRemoveVertex;
	FBButton	mButtonRemoveShape;
	FBButton	mButtonClearAllShapes;

	FBLayoutRegion	mRegionAnimate;
	FBButton	mButtonAutoKey;
	FBButton	mButtonKeyAll;
	FBButton	mButtonKeySet;
	FBButton	mButtonKeyRemove;
	FBButton	mButtonKeyPrev;
	FBButton	mButtonKeyNext;
	FBButton	mButtonClearAnimation;

	// layout 3 - navigator

	FBLayout	mLayoutNavigator;
	FBTree		mTreeNavigator;

	// layout 4 - view
	
	FBLayout		mLayoutView;
	FBTabPanel		mTabPanelView;
	DYNAMICMASK::View2d			mView;

	// layout 5 - properties

	FBLayout			mLayoutProperties;
	FBBrowsingProperty	mProperties;

	// layout 6 - status bar

	FBLayout			mLayoutStatus;
	FBLabel				mLabelStatus;

private:

	FBTreeNode		*mSelectedNode;

	void	UpdateMaskList();
	void	UpdateTree();
	void	UpdateManipulatorButtons();
	void	DoRenameShape(ObjectShape *shape);
};

#endif /* __DYNAMICMASK_TOOL_H__ */
