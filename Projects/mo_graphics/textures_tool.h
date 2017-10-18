#ifndef __ORTOOLVIEW3D_TOOL_H__
#define __ORTOOLVIEW3D_TOOL_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: textures_tool.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

//#include "textures_view.h"

//--- Registration define
#define ORTOOLVIEW3D__CLASSNAME	ORToolViewTextures
#define ORTOOLVIEW3D__CLASSSTR	"ORToolViewTextures"


/**	Tool template.
*/
class ORToolViewTextures : public FBTool
{
	//--- Tool declaration.
	FBToolDeclare( ORToolViewTextures, FBTool );

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
	void	UICreateControlPanel();
	void	UICreateViewPanel();
	void	UIConfigure	();
	void	UIConfigControlPanel();
	void	UIConfigViewPanel();
	void	UIReset		();
	void	UIResetControlPanel();
	void	UIResetViewPanel();

	// switch all the ui placecement between horizontal and vertical orientation
	void	UIOrient(const bool horizontal);
	void	UIOrientControlPanel(const bool horizontal);
	void	UIOrientViewPanel(const bool horizontal);

	// UI callbacks
	void	EventButtonSwitchClick( HISender pSender, HKEvent pEvent );
	void	EventButtonMenuClick( HISender pSender, HKEvent pEvent );
	void	EventToolIdle		( HISender pSender, HKEvent pEvent );
	void	EventToolShow		( HISender pSender, HKEvent pEvent );
	void	EventToolPaint		( HISender pSender, HKEvent pEvent );
	void	EventToolResize		( HISender pSender, HKEvent pEvent );
	void	EventToolInput		( HISender pSender, HKEvent pEvent );

	// Refresh the view
	void	RefreshView			();

private:

	FBGenericMenu	mPopupMenu;
	FBGenericMenu	mPopupMenuBind;

	//
	FBButton		mButtonUIOrient;		// change UI style from horizontal to vertical

	//
	// left control panel

	FBLayout		mLayoutControl;

	FBEdit			mEditSearch;	//!> enter a text for search a texture
	FBButton		mButtonSearch;

	FBButton		mButtonMenu;	// launch a generic menu

	//
	// main textures view
	FBLayout		mLayoutView;

	FBTabPanel		mTabTypes;			//!> a way what to show in textures view (color textures based on video clips, dynamic textures, lightmaps, filter 1, filter 2)
	FBSlider		mSliderSize;		//!> change size for the texture rects

//	ORView3D		mView;				// textures view

	FBSlider		mSlider;			// slider to scroll textures view

private:

	bool			mOrientation;	// horizontal - true, vertical - false
};

#endif /* __ORTOOLVIEW3D_TOOL_H__ */
