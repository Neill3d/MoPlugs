#ifndef __ORDEVICE_PHYSICS_LAYOUT_H__
#define __ORDEVICE_PHYSICS_LAYOUT_H__

/**	\file	ordevicephysics_layout.h
*	Declaration for the layout of a simple physics device.
*	Contains the declaration for the layout of a simple physics
*	device class, ORDeviceBulletLayout.
*/

//--- Class declaration
#include "ordevicePhysics_device.h"

//! Simple device layout.
class ORDevicePhysicsLayout : public FBDeviceLayout
{
	//--- FiLMBOX declaration.
	FBDeviceLayoutDeclare( ORDevicePhysicsLayout, FBDeviceLayout );

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

	void	EventButtonSaveState						( HISender pSender, HKEvent pEvent );
	void	EventButtonRestoreState						( HISender pSender, HKEvent pEvent );

private:
	FBTabPanel			mTabPanel;

	FBLayout			mLayoutMarkers;
		FBButton				mButtonSaveState;
		FBButton				mButtonRestoreState;
	
		FBEditProperty			mPropLiveMode;
		FBEditProperty			mPropResetFrame;

		FBEditProperty			mPropEvaluateRate;
		FBEditProperty			mPropStaticCollisions;

		FBEditProperty			mPropGravity;
		FBEditProperty			mPropWorldScale;

	FBLayout			mLayoutInformation;
		FBLabel				mLabelInformation;

private:
	FBSystem				mSystem;					//!< System interface.
	ORDevicePhysics*		mDevice;					//!< Handle onto device.
};

#endif /* __ORDEVICE_PHYSICS_LAYOUT_H__ */
