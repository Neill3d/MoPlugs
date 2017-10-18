
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: textures_tool.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>

//--- Class declaration
#include "textures_tool.h"

#include "shared_content.h"

//--- Registration defines
#define ORTOOLVIEW3D__CLASS	ORTOOLVIEW3D__CLASSNAME
#define ORTOOLVIEW3D__LABEL	"Textures Browser"
#define ORTOOLVIEW3D__DESC	"Textures Browser Tool"

//--- Implementation and registration
FBToolImplementation(	ORTOOLVIEW3D__CLASS	);
FBRegisterTool		(	ORTOOLVIEW3D__CLASS,
						ORTOOLVIEW3D__LABEL,
						ORTOOLVIEW3D__DESC,
						FB_DEFAULT_SDK_ICON	);	// Icon filename (default=Open Reality icon)

/************************************************
 *	Constructor.
 ************************************************/
bool ORToolViewTextures::FBCreate()
{
	StartSize[0] = 1024;
	StartSize[1] = 256;

	// Manage UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	mOrientation = true;
	UIOrient( mOrientation );

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ORToolViewTextures::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &ORToolViewTextures::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ORToolViewTextures::EventToolResize		);
	OnPaint.Add	( this, (FBCallback) &ORToolViewTextures::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ORToolViewTextures::EventToolInput		);

	return true;
}

/************************************************
 *	Create, configure & reset UI.
 ************************************************/
void ORToolViewTextures::UICreate()
{
	// Tool options
	const int lS = 4;
	const int lW = 140;
	const int lH = 140;

	// Configure layout
	AddRegion( "ButtonOrient", "ButtonOrient",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										24, kFBAttachNone,	"",	1.0,
										24,	kFBAttachNone,	"",	1.0 );

	AddRegion( "HControlPanel", "HControlPanel",
										lS,	kFBAttachRight,	"ButtonOrient",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										lW, kFBAttachNone,	"",	1.0,
										-lS,kFBAttachBottom,"",	1.0 );
	
	AddRegion( "HView", "HView",
										lS,	kFBAttachRight,	"HControlPanel",	1.0,
										lS,	kFBAttachTop,	"",	1.0,
										-lS,kFBAttachRight,	"", 1.0,
										-lS,kFBAttachBottom,"", 1.0 );

	AddRegion( "VControlPanel", "VControlPanel",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"ButtonOrient",	1.0,
										-lS, kFBAttachRight,"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	
	AddRegion( "VView", "VView",
										lS,	kFBAttachLeft,	"",	1.0,
										lS,	kFBAttachBottom,"VControlPanel",	1.0,
										-lS,kFBAttachRight,	"", 1.0,
										-lS,kFBAttachBottom,"", 1.0 );

	SetControl( "ButtonOrient", mButtonUIOrient );

	UICreateControlPanel();
	UICreateViewPanel();
}

void ORToolViewTextures::UIOrient(const bool horizontal)
{
	if (horizontal)
	{
		// Assign regions
		SetControl	( "HControlPanel",	mLayoutControl);
		SetControl	( "HView",			mLayoutView);
	}
	else
	{
		// Assign regions
		SetControl	( "VControlPanel",	mLayoutControl);
		SetControl	( "VView",			mLayoutView);
	}

	UIOrientControlPanel(horizontal);
	UIOrientViewPanel(horizontal);
}

void ORToolViewTextures::UICreateControlPanel()
{
	
	FBLayout *pLayout = &mLayoutControl;

	const int lS = 5;
	const int lW = 90;
	const int lH = 25;

	// Configure layout
	pLayout->AddRegion( "EditSearch", "EditSearch",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										lW, kFBAttachNone,	"",	1.0,
										lH, kFBAttachNone,	"",	1.0 );
	pLayout->AddRegion( "ButtonSearch", "ButtonSearch",
										lS,	kFBAttachRight,	"EditSearch",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										lH, kFBAttachNone,	"",	1.0,
										lH, kFBAttachNone,	"",	1.0 );

	pLayout->AddRegion( "ButtonMenu", "ButtonMenu",
										lS,	kFBAttachLeft,	"",	1.0	,
										2 * lS,	kFBAttachBottom,"EditSearch",	1.0,
										lW, kFBAttachNone,	"",	1.0,
										lH, kFBAttachNone,	"",	1.0 );



	pLayout->SetControl( "EditSearch", mEditSearch );
	pLayout->SetControl( "ButtonSearch", mButtonSearch );
	pLayout->SetControl( "ButtonMenu", mButtonMenu );
}

void ORToolViewTextures::UIOrientControlPanel(const bool horizontal)
{

}

void ORToolViewTextures::UICreateViewPanel()
{
	const int lS = 5;
	const int lH = 24;

	// Configure layout
	mLayoutView.AddRegion( "HTab", "HTab",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										0, kFBAttachWidth,	"",	0.75,
										lH, kFBAttachNone,	"",	1.0 );

	mLayoutView.AddRegion( "HSliderSize", "HSliderSize",
										0,kFBAttachWidth,	"",	0.85,
										lS,	kFBAttachTop,	"",	1.0,
										-lS, kFBAttachRight,"",	1.0,
										lH, kFBAttachNone,	"",	1.0 );
	mLayoutView.AddRegion( "HView", "HView",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,	"HTab",	1.0,
										-lS, kFBAttachRight,"",	0.8,
										-lS-lH, kFBAttachBottom,	"",	1.0 );

	mLayoutView.AddRegion( "HSlider", "HSlider",
										lS,	kFBAttachLeft,	"",	1.0	,
										lS,	kFBAttachBottom,"HView",	1.0,
										-lS, kFBAttachRight,"",	0.8,
										-lS, kFBAttachBottom,	"",	1.0 );

	// Configure layout
	mLayoutView.AddRegion( "VTab", "VTab",
										lS,	kFBAttachLeft,	"",	1.0,
										lS,	kFBAttachTop,	"",	1.0,
										lH, kFBAttachNone,	"",	1.0,
										0,kFBAttachHeight,	"",	0.75 );

	mLayoutView.AddRegion( "VSliderSize", "VSliderSize",
										lS, kFBAttachLeft,	"",	1.0,
										0,	kFBAttachHeight,"",	0.85,
										lH, kFBAttachNone,	"",	1.0,
										-lS,kFBAttachBottom,"",	1.0 );
	
	mLayoutView.AddRegion( "VSlider", "VSlider",
										-lS-lH,	kFBAttachRight,	"",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										-lS, kFBAttachRight,"",	1.0,
										-lS, kFBAttachBottom,	"",	1.0 );
	
	mLayoutView.AddRegion( "VView", "VView",
										lS,	kFBAttachRight,	"VTab",	1.0	,
										lS,	kFBAttachTop,	"",	1.0,
										-lS, kFBAttachLeft, "VSlider",	1.0,
										-lS, kFBAttachBottom,	"",	1.0 );

}

void ORToolViewTextures::UIOrientViewPanel(const bool horizontal)
{
	if (horizontal)
	{
		//mLayoutView.SetControl( "HTab", mTabTypes );
		mLayoutView.SetControl( "HSliderSize", mSliderSize );

//		mLayoutView.SetView( "HView", mView );

		mLayoutView.SetControl( "HSlider", mSlider );

		mSliderSize.Orientation = kFBHorizontal;
		mSlider.Orientation = kFBHorizontal;
	}
	else
	{
		//mLayoutView.SetControl( "VTab", mTabTypes );
		mLayoutView.SetControl( "VSliderSize", mSliderSize );

//		mLayoutView.SetView( "VView", mView );

		mLayoutView.SetControl( "VSlider", mSlider );

		mSliderSize.Orientation = kFBVertical;
		mSlider.Orientation = kFBVertical;
	}
}

void ORToolViewTextures::UIConfigure()
{
	mButtonUIOrient.Caption = "<>";
	mButtonUIOrient.OnClick.Add( this, (FBCallback) &ORToolViewTextures::EventButtonSwitchClick );

	UIConfigControlPanel();
	UIConfigViewPanel();


	//
	// context menu
	//

	mPopupMenuBind.InsertLast( "Material Diffuse", 0 );
	mPopupMenuBind.InsertLast( "Material Specular", 1 );
	mPopupMenuBind.InsertLast( "Material Normal", 2 );
	mPopupMenuBind.InsertLast( "Material Emissive", 3 );
	mPopupMenuBind.InsertLast( "As object texture", 4 );

	mPopupMenu.InsertLast( "Convert Scene selection to Current", 0 );
	mPopupMenu.InsertLast( "Convert Current to Scene selection", 1 );
	mPopupMenu.InsertLast( "Select None", 2 );
	mPopupMenu.InsertLast( "-", 3 );
	mPopupMenu.InsertLast( "Bind to selected models", 4, &mPopupMenuBind );
	
	mPopupMenu.InsertLast( "-", 5 );
	mPopupMenu.InsertLast( "Reload", 6 );
	
//	mView.AssignContextMenu( &mPopupMenu );
//	mView.AssignTexturesManager( CGPUFBScene::instance().GetTexturesManagerPtr() );
}

void ORToolViewTextures::UIConfigControlPanel()
{
	mButtonSearch.Caption = ">";

	mButtonMenu.Caption = "Menu ->";
	mButtonMenu.OnClick.Add( this, (FBCallback) &ORToolViewTextures::EventButtonMenuClick );
}

void ORToolViewTextures::UIConfigViewPanel()
{
	mTabTypes.Items.SetString( "Video Textures~Dynamic Textures~Lightmap Textures~Filter 1~Filter 2" );

	mSliderSize.Max = 100;
	mSliderSize.Min = 0;
	
	mSlider.Max = 100;
	mSlider.Min = 0;
}

void ORToolViewTextures::UIReset()
{
	UIResetControlPanel();
	UIResetViewPanel();
}

void ORToolViewTextures::UIResetControlPanel()
{
}

void ORToolViewTextures::UIResetViewPanel()
{
}

void ORToolViewTextures::EventButtonSwitchClick( HISender pSender, HKEvent pEvent )
{
	mOrientation = !mOrientation;
	// restruct ui
	UIOrient( mOrientation );
}

void ORToolViewTextures::EventButtonMenuClick( HISender pSender, HKEvent pEvent )
{
	POINT p;
	if (GetCursorPos(&p))
	{
		mPopupMenu.Execute(p.x, p.y);
	}
}

/************************************************
 *	Destruction function.
 ************************************************/
void ORToolViewTextures::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ORToolViewTextures::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ORToolViewTextures::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ORToolViewTextures::EventToolPaint	);
	OnInput.Remove	( this, (FBCallback) &ORToolViewTextures::EventToolInput	);
	OnResize.Remove	( this, (FBCallback) &ORToolViewTextures::EventToolResize	);

	// Free user allocated memory
}

/************************************************
 *	UI Idle callback.
 ************************************************/
void ORToolViewTextures::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	RefreshView();
}

void ORToolViewTextures::RefreshView()
{
//	mView.Refresh(true);
}

/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ORToolViewTextures::EventToolShow( HISender pSender, HKEvent pEvent )
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
void ORToolViewTextures::EventToolPaint( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	Tool resize callback.
 ************************************************/
void ORToolViewTextures::EventToolResize( HISender pSender, HKEvent pEvent )
{
//	mView.Resize( -1, -1 );
}

/************************************************
 *	Handle input into the tool.
 ************************************************/
void ORToolViewTextures::EventToolInput( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool ORToolViewTextures::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	pFbxObject->FieldWriteBegin( "ORToolView3DSection" );
	{
	}
	pFbxObject->FieldWriteEnd();
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORToolViewTextures::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	pFbxObject->FieldReadBegin( "ORToolView3DSection" );
	{
	}
	pFbxObject->FieldReadEnd();
	return true;
}

