#ifndef __ORDEVICE_SKELETON_LAYOUT_H__
#define __ORDEVICE_SKELETON_LAYOUT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: device_shapehand_layout.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "device_shapehand_device.h"


//////////////////////////////////////////////////////////////////////////////
//! ShapeHand device layout.
class DeviceShapeHandLayout : public FBDeviceLayout
{
	//--- FiLMBOX declaration.
	FBDeviceLayoutDeclare( DeviceShapeHandLayout, FBDeviceLayout );

public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();			//!< FiLMBOX constructor.
	virtual void FBDestroy();			//!< FiLMBOX destructor.

	// UI Management
	void	UICreate				();		// Create & assign UI regions
	void		UICreateLayout0		();
	void		UICreateLayout1		();
	void	UIConfigure				();		// Configure UI options
	void		UIConfigureLayout0	();
	void		UIConfigureLayout1	();
	void	UIRefresh				();		// Refresh on idle
	void		UIRefreshSpreadSheet();
	void	UIReset					();		// Reset from device values
	void		UIResetSpreadSheet	();

	// Main Layout: Events
	void	EventDeviceStatusChange					( HISender pSender, HKEvent pEvent );
	void	EventUIIdle								( HISender pSender, HKEvent pEvent );
	void	EventTabPanelChange						( HISender pSender, HKEvent pEvent );

	// Setup Layout: Events
	void	EventSetupCharacter						( HISender pSender, HKEvent pEvent );
	void	EventButtonNetworkEnableClick			( HISender pSender, HKEvent pEvent );
	void	EventEditNetworkAddressChange			( HISender pSender, HKEvent pEvent );
	void	EventEditNetworkPortChange				( HISender pSender, HKEvent pEvent );

private:
	FBTabPanel			mTabPanel;

	FBLayout			mLayoutMarkers;
		FBSpread			mSpreadMarkers;

	FBLayout			mLayoutSetup;
		FBLabel				mLabelSetup;
		FBButton			mButtonCharacterize;

	FBLayout			mLayoutCommunication;
		FBButton			mButtonNetworkEnable;
			FBLabel				mLabelNetworkAddress;
			FBEdit				mEditNetworkAddress;
			FBLabel				mLabelNetworkPort;
			FBEdit				mEditNetworkPort;

private:
	FBSystem				mSystem;					//!< System interface.
	DeviceShapeHand			*mDevice;					//!< Handle onto device.
};

#endif /* __ORDEVICE_SKELETON_LAYOUT_H__ */
