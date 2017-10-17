
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: device_shapehand_layout.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "device_shapehand_device.h"
#include "device_shapehand_layout.h"

#define DEVICESHAPEHAND_LAYOUT	DeviceShapeHandLayout

//--- FiLMBOX implementation and registration
FBDeviceLayoutImplementation(	DEVICESHAPEHAND_LAYOUT		);
FBRegisterDeviceLayout		(	DEVICESHAPEHAND_LAYOUT,
								DEVICESHAPEHAND_CLASSSTR,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX constructor.
 ************************************************/
bool DeviceShapeHandLayout::FBCreate()
{
	// Get a handle on the device.
	mDevice = ((DeviceShapeHand *)(FBDevice *)Device);

	// Create/configure UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add device & system callbacks
	mDevice->OnStatusChange.Add	( this,(FBCallback)&DeviceShapeHandLayout::EventDeviceStatusChange  );
	mSystem.OnUIIdle.Add		( this,(FBCallback)&DeviceShapeHandLayout::EventUIIdle              );

	return true;
}


/************************************************
 *	FiLMBOX destructor.
 ************************************************/
void DeviceShapeHandLayout::FBDestroy()
{
	// Remove device & system callbacks
	mSystem.OnUIIdle.Remove		( this,(FBCallback)&DeviceShapeHandLayout::EventUIIdle              );
	mDevice->OnStatusChange.Remove	( this,(FBCallback)&DeviceShapeHandLayout::EventDeviceStatusChange  );
}


/************************************************
 *	Create the UI.
 ************************************************/
void DeviceShapeHandLayout::UICreate()
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
void DeviceShapeHandLayout::UICreateLayout0()
{
	// Add regions
	mLayoutMarkers.AddRegion( "SpreadMarkers", "SpreadMarkers",
													0,		kFBAttachLeft,		"",		1.00,
													0,		kFBAttachTop,		"",		1.00,
													0,		kFBAttachRight,		"",		1.00,
													0,		kFBAttachBottom,	"",		1.00 );

	// Assign regions
	mLayoutMarkers.SetControl( "SpreadMarkers", mSpreadMarkers );
}


/************************************************
 *	Create the setup layout.
 ************************************************/
void DeviceShapeHandLayout::UICreateLayout1()
{
	int lS, lW, lH;		// space, width, height.
	lS = 4;
	lW = 250;
	lH = 18;

	// Add regions
	mLayoutSetup.AddRegion (	"LabelSetup",	"LabelSetup",
													lS,		kFBAttachLeft,		"",		1.00,
													lS,		kFBAttachTop,		"",		1.00,
													lW,		kFBAttachNone,		NULL,	1.00,
													lH,		kFBAttachNone,		NULL,	1.00 );
	// Assign regions
	mLayoutSetup.SetControl(	"LabelSetup",	mLabelSetup );

	// Add regions
	mLayoutSetup.AddRegion (	"CharacterizeButton",	"CharacterizeButton",
													lS,		kFBAttachLeft,		"",		1.00,
													lS,		kFBAttachBottom,	"LabelSetup",		1.00,
													lW,		kFBAttachNone,		NULL,	1.00,
													lH,		kFBAttachNone,		NULL,	1.00 );
	// Assign regions
	mLayoutSetup.SetControl(	"CharacterizeButton",	mButtonCharacterize );

	int lS_y	= -15;
	lW			= 100;
	lH			= 25;
	int lHlr	= 100;
	int lWlr	= 250;
	int lWrb	= 140;
	int lSlbx	= 30;
	int lSlby	= 12;
	int lWlb	= 80;

	mLayoutSetup.AddRegion( "NetworkSetup",	"NetworkSetup",
													lS,		kFBAttachLeft,		"",						1.00,
													15,		kFBAttachBottom,	"CharacterizeButton",	1.00,
													lWlr,	kFBAttachNone,	NULL,					1.00,
													lHlr,	kFBAttachNone,	NULL,					1.00 );

	// Add regions (network)
	mLayoutCommunication.AddRegion( "LayoutRegionNetwork",	"LayoutRegionNetwork",
													5,		kFBAttachLeft,		"",						1.00,
													15,		kFBAttachTop,		"NetworkSetup",			1.00,
													-10,	kFBAttachWidth,		"NetworkSetup",			1.00,
													-30,	kFBAttachHeight,	"NetworkSetup",			1.00 );
	mLayoutCommunication.AddRegion( "ButtonNetworkEnable",	"ButtonNetworkEnable",
													lS,		kFBAttachLeft,		"LayoutRegionNetwork",	1.00,
													lS_y,	kFBAttachTop,		"LayoutRegionNetwork",	1.00,
													lWrb,	kFBAttachNone,		NULL,					1.00,
													lH,		kFBAttachNone,		NULL,					1.00 );
	mLayoutCommunication.AddRegion( "LabelNetworkAddress",		"LabelNetworkAddress",
													lSlbx,	kFBAttachLeft,		"LayoutRegionNetwork",	1.00,
													lSlby,	kFBAttachTop,		"LayoutRegionNetwork",	1.00,
													lWlb,	kFBAttachNone,		NULL,					1.00,
													lH,		kFBAttachNone,		NULL,					1.00 );
	mLayoutCommunication.AddRegion( "EditNetworkAddress",		"EditNetworkAddress",
													lS,		kFBAttachRight,		"LabelNetworkAddress",	1.00,
													0,		kFBAttachTop,		"LabelNetworkAddress",	1.00,
													lW,		kFBAttachNone,		NULL,					1.00,
													lH,		kFBAttachNone,		NULL,					1.00 );
	mLayoutCommunication.AddRegion( "LabelNetworkPort",	"LabelNetworkPort",
													0,		kFBAttachLeft,		"LabelNetworkAddress",	1.00,
													lS,		kFBAttachBottom,	"LabelNetworkAddress",	1.00,
													0,		kFBAttachWidth,		"LabelNetworkAddress",	1.00,
													0,		kFBAttachHeight,	"LabelNetworkAddress",	1.00 );
	mLayoutCommunication.AddRegion( "EditNetworkPort",	"EditNetworkPort",
													lS,		kFBAttachRight,		"LabelNetworkPort",		1.00,
													0,		kFBAttachTop,		"LabelNetworkPort",		1.00,
													lW,		kFBAttachNone,		NULL,					1.00,
													0,		kFBAttachHeight,	"LabelNetworkPort",		1.00 );

	// Assign regions (network)
	mLayoutCommunication.SetControl( "ButtonNetworkEnable",		mButtonNetworkEnable	);
	mLayoutCommunication.SetControl( "LabelNetworkAddress",		mLabelNetworkAddress	);
	mLayoutCommunication.SetControl( "EditNetworkAddress",		mEditNetworkAddress		);
	mLayoutCommunication.SetControl( "LabelNetworkPort",		mLabelNetworkPort		);
	mLayoutCommunication.SetControl( "EditNetworkPort",			mEditNetworkPort		);

	mLayoutSetup.SetControl	( "NetworkSetup",	mLayoutCommunication	);
}


/************************************************
 *	Configure the UI elements (main layout).
 ************************************************/
void DeviceShapeHandLayout::UIConfigure()
{
	SetBorder ("MainLayout", kFBStandardBorder, false,true, 1, 0,90,0);

	mTabPanel.Items.SetString("Markers~Setup");
	mTabPanel.OnChange.Add( this, (FBCallback)&DeviceShapeHandLayout::EventTabPanelChange );

	UIConfigureLayout0();
	UIConfigureLayout1();
}


/************************************************
 *	Configure the UI elements (marker layout).
 ************************************************/
void DeviceShapeHandLayout::UIConfigureLayout0()
{
}


/************************************************
 *	Configure the UI elements (setup layout).
 ************************************************/
void DeviceShapeHandLayout::UIConfigureLayout1()
{
	mLabelSetup.Caption = "Character setup according to device template";
	mButtonCharacterize.Caption = "Characterize";
	mButtonCharacterize.OnClick.Add( this, (FBCallback)&DeviceShapeHandLayout::EventSetupCharacter );

    mLayoutCommunication.SetBorder( "LayoutRegionNetwork",	kFBEmbossBorder,false,true,2,1,90.0,0);

	mButtonNetworkEnable.Caption	= "Use skeleton test server";
	mButtonNetworkEnable.Style		= kFBCheckbox;
	mButtonNetworkEnable.State		= mDevice->GetNetworkUse();

	mButtonNetworkEnable.OnClick.Add( this,(FBCallback) &DeviceShapeHandLayout::EventButtonNetworkEnableClick );

	mLayoutCommunication.SetBorder( "EditNetworkAddress",	kFBStandardBorder, false,true, 1, 0,90,0);
	mLayoutCommunication.SetBorder( "EditNetworkPort",		kFBStandardBorder, false,true, 1, 0,90,0);


	mLabelNetworkAddress.Caption	= "Address :";
	mEditNetworkAddress.Text		= mDevice->GetNetworkAddress();
	mEditNetworkAddress.OnChange.Add( this, (FBCallback) &DeviceShapeHandLayout::EventEditNetworkAddressChange );

	mLabelNetworkPort.Caption		= "Port :";

	char	lBuffer[40];
	sprintf(lBuffer, "%d", mDevice->GetNetworkPort() );
	mEditNetworkPort.Text = lBuffer;
	mEditNetworkPort.OnChange.Add( this, (FBCallback) &DeviceShapeHandLayout::EventEditNetworkPortChange );
}


/************************************************
 *	Refresh the UI.
 ************************************************/
void DeviceShapeHandLayout::UIRefresh()
{
	UIRefreshSpreadSheet();
}


/************************************************
 *	Refresh the spreadsheet content.
 ************************************************/
void DeviceShapeHandLayout::UIRefreshSpreadSheet()
{
	FBRVector rot;

  	for(int i=0; i<mDevice->GetChannelCount();i++)
	{
		FBQuaternion q( mDevice->GetDataQ(i) );	
		FBQuaternionToRotation( rot, q );

		mSpreadMarkers.SetCell( i, 0, mDevice->GetDataTX(i) );
		mSpreadMarkers.SetCell( i, 1, mDevice->GetDataTY(i) );
		mSpreadMarkers.SetCell( i, 2, mDevice->GetDataTZ(i) );
		mSpreadMarkers.SetCell( i, 3, rot[0] );
		mSpreadMarkers.SetCell( i, 4, rot[1] );
		mSpreadMarkers.SetCell( i, 5, rot[2] );
	}
}


/************************************************
 *	Reset the UI values from device.
 ************************************************/
void DeviceShapeHandLayout::UIReset()
{
	UIResetSpreadSheet();
}


/************************************************
 *	Re-build the spreadsheet.
 ************************************************/
void DeviceShapeHandLayout::UIResetSpreadSheet()
{
	int i;

	mSpreadMarkers.Clear();

	// Spreadsheet
	int lColumnIndex = -1;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 200;

	// column 0: Translation X
	mSpreadMarkers.ColumnAdd ("PosX");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 1: Translation Y
	mSpreadMarkers.ColumnAdd ("PosY");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
 	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

 	// column 2: Translation Z
	mSpreadMarkers.ColumnAdd ("PosZ");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 3: Rotation X
	mSpreadMarkers.ColumnAdd ("RotX");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 4: Rotation Y
	mSpreadMarkers.ColumnAdd ("RotY");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
 	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

 	// column 5: Rotation Z
	mSpreadMarkers.ColumnAdd ("RotZ");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	for (i=0;i<mDevice->GetChannelCount();i++)
	{
 		mSpreadMarkers.RowAdd( mDevice->GetChannelName(i), i );
	 	mSpreadMarkers.GetCell(i,lColumnIndex).ReadOnly = true;
	}
}


/************************************************
 *	Tab panel change callback.
 ************************************************/
void DeviceShapeHandLayout::EventTabPanelChange( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
		case 0:	SetControl("MainLayout", mLayoutMarkers			);	break;
		case 1:	SetControl("MainLayout", mLayoutSetup		);	break;
	}
}


/************************************************
 *	Server port change callback.
 ************************************************/
void DeviceShapeHandLayout::EventButtonNetworkEnableClick( HISender pSender, HKEvent pEvent )
{
	mDevice->SetNetworkUse(((int)mButtonNetworkEnable.State)==1);
}

/************************************************
 *	Server port change callback.
 ************************************************/
void DeviceShapeHandLayout::EventEditNetworkPortChange( HISender pSender, HKEvent pEvent )
{
	int		lPort;
	char	lBuffer[40];

	sscanf(mEditNetworkPort.Text.AsString(), "%d", &lPort);
	mDevice->SetNetworkPort(lPort);
	lPort = mDevice->GetNetworkPort();
	sprintf(lBuffer, "%d", lPort );
	mEditNetworkPort.Text = lBuffer;
}


/************************************************
 *	Server port address callback.
 ************************************************/
void DeviceShapeHandLayout::EventEditNetworkAddressChange( HISender pSender, HKEvent pEvent )
{
	mDevice->SetNetworkAddress(mEditNetworkAddress.Text.AsString());
	mEditNetworkAddress.Text = mDevice->GetNetworkAddress();
}

/************************************************
 *	Device status change callback.
 ************************************************/
void DeviceShapeHandLayout::EventDeviceStatusChange( HISender pSender, HKEvent pEvent )
{
	UIReset();
}


/************************************************
 *	UI Idle callback.
 ************************************************/
void DeviceShapeHandLayout::EventUIIdle( HISender pSender, HKEvent pEvent )
{
	if( mDevice->Online )
	{
		UIRefresh();
	}
}

//simple name mapping.
static void SetupMapping(FBCharacter* pCharacter, FBModel* pParent)
{
    FBString lName = (const char*)pParent->Name; lName += "Link";
	FBProperty* lLink = pCharacter->PropertyList.Find(lName,false);
	if(lLink && lLink->IsList())
	{
		((FBPropertyListComponent*)lLink)->Add(pParent);
	}
	
	//do the same for children
	for(int i = 0; i < pParent->Children.GetCount(); i++)
	{
		SetupMapping(pCharacter,pParent->Children[i]);
	}
}

/************************************************
 *	Characterize binding models.
 ************************************************/
void DeviceShapeHandLayout::EventSetupCharacter( HISender pSender, HKEvent pEvent )
{
	if(mDevice && mDevice->ReadyForCharacterize())
	{
		FBCharacter* lCharacter = new FBCharacter("DeviceCharacter");

		FBModel* lReference = mDevice->mRootTemplate->Model;
		
		if(lReference)
		{
			SetupMapping(lCharacter,lReference);
			lCharacter->SetCharacterizeOn(true);
		}
	}
}
