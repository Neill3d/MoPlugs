
/**	\file	ordevicebullet_layout.cxx
*	Definition of a layout class for a simple bullet device.
*	Definition of the functions of the ORDeviceBulletLayout class,
*	representing the layout for a simple bullet device.
*/

//--- Class declarations
#include "ordevicePhysics_device.h"
#include "ordevicePhysics_layout.h"

#define ORDEVICEPHYSICS__LAYOUT	ORDevicePhysicsLayout

//--- FiLMBOX implementation and registration
FBDeviceLayoutImplementation(	ORDEVICEPHYSICS__LAYOUT		);
FBRegisterDeviceLayout		(	ORDEVICEPHYSICS__LAYOUT,
								ORDEVICEPHYSICS__CLASSSTR,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX constructor.
 ************************************************/
bool ORDevicePhysicsLayout::FBCreate()
{
	// Get a handle on the device.
	mDevice = ((ORDevicePhysics *)(FBDevice *)Device);

	// Create/configure UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add device & system callbacks
	mDevice->OnStatusChange.Add	( this,(FBCallback)&ORDevicePhysicsLayout::EventDeviceStatusChange  );
	mSystem.OnUIIdle.Add		( this,(FBCallback)&ORDevicePhysicsLayout::EventUIIdle              );

	return true;
}


/************************************************
 *	FiLMBOX destructor.
 ************************************************/
void ORDevicePhysicsLayout::FBDestroy()
{
	// Remove device & system callbacks
	mSystem.OnUIIdle.Remove		( this,(FBCallback)&ORDevicePhysicsLayout::EventUIIdle              );
	mDevice->OnStatusChange.Remove	( this,(FBCallback)&ORDevicePhysicsLayout::EventDeviceStatusChange  );
}


/************************************************
 *	Create the UI.
 ************************************************/
void ORDevicePhysicsLayout::UICreate()
{
	int lS, lH;		// space, height
	lS = 4;
	lH = 25;

	// Create regions
	AddRegion	( "TabPanel",	"TabPanel",		0,		kFBAttachLeft,		"",			1.00,
												0,		kFBAttachTop,		"",			1.00,
												0,		kFBAttachRight,		"",			1.00,
												lH,		kFBAttachNone,		NULL,		1.00 );
	AddRegion	( "MainLayout",	"MainLayout",	lS,		kFBAttachLeft,		"TabPanel",	1.00,
												lS,		kFBAttachBottom,	"TabPanel",	1.00,
												-lS,	kFBAttachRight,		"TabPanel",	1.00,
												-lS,	kFBAttachBottom,	"",			1.00 );

	// Assign regions
	SetControl	( "TabPanel",	mTabPanel		);
	SetControl	( "MainLayout",	mLayoutMarkers	);

	// Create sub layouts
	UICreateLayout0();
	UICreateLayout1();
}


/************************************************
 *	Create the markers layout.
 ************************************************/
void ORDevicePhysicsLayout::UICreateLayout0()
{
	// Add regions
	 int lB = 10;
	//int lS = 4;
	int lW = 120;
	int lH = 18;

	// Configure layout
	
	mLayoutMarkers.AddRegion( "ButtonSaveState", "ButtonSaveState",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutMarkers.AddRegion( "ButtonRestoreState", "ButtonRestoreState",
										lB*2,	kFBAttachRight,	"ButtonSaveState",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	
	mLayoutMarkers.AddRegion( "LiveMode", "LiveMode",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachBottom,	"ButtonSaveState",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutMarkers.AddRegion( "ResetFrame", "ResetFrame",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachBottom,	"LiveMode",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutMarkers.AddRegion( "TimeStep", "TimeStep",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB*2,	kFBAttachBottom,	"ResetFrame",	1.0,
										lW*3,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutMarkers.AddRegion( "Collisions", "Collisions",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachBottom,	"TimeStep",	1.0,
										lW*3,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutMarkers.AddRegion( "Gravity", "Gravity",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachBottom,	"Collisions",	1.0,
										lW*3,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutMarkers.AddRegion( "WorldScale", "WorldScale",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachBottom,	"Gravity",	1.0,
										lW*3,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	// Assign regions
	mLayoutMarkers.SetControl( "ButtonSaveState", mButtonSaveState );
	mLayoutMarkers.SetControl( "ButtonRestoreState", mButtonRestoreState );
	
	mLayoutMarkers.SetControl( "LiveMode", mPropLiveMode );
	mLayoutMarkers.SetControl( "ResetFrame", mPropResetFrame );

	mLayoutMarkers.SetControl( "TimeStep", mPropEvaluateRate );
	mLayoutMarkers.SetControl( "Collisions", mPropStaticCollisions );

	mLayoutMarkers.SetControl( "Gravity", mPropGravity );
	mLayoutMarkers.SetControl( "WorldScale", mPropWorldScale );
}


/************************************************
 *	Create the information layout.
 ************************************************/
void ORDevicePhysicsLayout::UICreateLayout1()
{
	int lS, lW, lH;		// space, width, height.
	lS = 4;
	lW = 200;
	lH = 18;

	// Add regions
	mLayoutInformation.AddRegion (	"LabelInformation",	"LabelInformation",
													lS,		kFBAttachLeft,		"",		1.00,
													lS,		kFBAttachTop,		"",		1.00,
													lW,		kFBAttachNone,		NULL,	1.00,
													lH,		kFBAttachNone,		NULL,	1.00 );
	// Assign regions
	mLayoutInformation.SetControl(	"LabelInformation",	mLabelInformation );
}


/************************************************
 *	Configure the UI elements (main layout).
 ************************************************/
void ORDevicePhysicsLayout::UIConfigure()
{
	SetBorder ("MainLayout", kFBStandardBorder, false,true, 1, 0,90,0);

	mTabPanel.Items.SetString("Parameters~Information");
	mTabPanel.OnChange.Add( this, (FBCallback)&ORDevicePhysicsLayout::EventTabPanelChange );

	UIConfigureLayout0();
	UIConfigureLayout1();
}


/************************************************
 *	Configure the UI elements (marker layout).
 ************************************************/
void ORDevicePhysicsLayout::UIConfigureLayout0()
{
	mButtonSaveState.Caption = "Save State";
	mButtonSaveState.OnClick.Add( this, (FBCallback) &ORDevicePhysicsLayout::EventButtonSaveState );

	mButtonRestoreState.Caption = "Restore State";
	mButtonRestoreState.OnClick.Add( this, (FBCallback) &ORDevicePhysicsLayout::EventButtonRestoreState );

	mPropLiveMode.Caption = "Live Mode";
	mPropLiveMode.Property = &mDevice->RealTimeMode;

	mPropResetFrame.Caption = "Reset Frame";
	mPropResetFrame.Property = &mDevice->ResetFrame;

	mPropEvaluateRate.Caption = "Evaluate Rate";
	mPropEvaluateRate.Property = &mDevice->EvaluateRate;
	mPropStaticCollisions.Caption = "Static Collisions";
	mPropStaticCollisions.Property = &mDevice->StaticCollisions;

	mPropGravity.Caption = "Gravity";
	mPropGravity.Property = &mDevice->Gravity;

	mPropWorldScale.Caption = "World Scale";
	mPropWorldScale.Property = &mDevice->WorldScale;
}


/************************************************
 *	Configure the UI elements (information layout).
 ************************************************/
void ORDevicePhysicsLayout::UIConfigureLayout1()
{
	mLabelInformation.Caption = "Information...";
}


/************************************************
 *	Refresh the UI.
 ************************************************/
void ORDevicePhysicsLayout::UIRefresh()
{
	UIRefreshSpreadSheet();
}


/************************************************
 *	Refresh the spreadsheet content.
 ************************************************/
void ORDevicePhysicsLayout::UIRefreshSpreadSheet()
{
  	
}


/************************************************
 *	Reset the UI values from device.
 ************************************************/
void ORDevicePhysicsLayout::UIReset()
{
	UIResetSpreadSheet();
}


/************************************************
 *	Re-build the spreadsheet.
 ************************************************/
void ORDevicePhysicsLayout::UIResetSpreadSheet()
{
	
}


/************************************************
 *	Tab panel change callback.
 ************************************************/
void ORDevicePhysicsLayout::EventTabPanelChange( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
		case 0:	SetControl("MainLayout", mLayoutMarkers			);	break;
		case 1:	SetControl("MainLayout", mLayoutInformation		);	break;
	}
}


/************************************************
 *	Device status change callback.
 ************************************************/
void ORDevicePhysicsLayout::EventDeviceStatusChange( HISender pSender, HKEvent pEvent )
{
	// update static collisions if we go online

	// TODO: rebuild all constraints with physics connected to that device !!!
	
	if (mDevice->Online)
	{
		mDevice->EnterOnline();
	}
	else
	{	
		mDevice->LeaveOnline();
	}

	UIReset();
}


/************************************************
 *	UI Idle callback.
 ************************************************/
void ORDevicePhysicsLayout::EventUIIdle( HISender pSender, HKEvent pEvent )
{
	/*
	if( mDevice->Online )
	{
		UIRefresh();
	}
	*/
}

void ORDevicePhysicsLayout::EventButtonSaveState( HISender pSender, HKEvent pEvent )
{
	mDevice->SaveState();
}

void ORDevicePhysicsLayout::EventButtonRestoreState( HISender pSender, HKEvent pEvent )
{
	mDevice->RestoreState();
}

