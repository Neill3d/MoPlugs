#ifndef __ORDEVICE_PHYSICS_DEVICE_H__
#define __ORDEVICE_PHYSICS_DEVICE_H__

/**	\file	ordevicePhysics_device.h
*	Declaration of a simple input device class.
*	Declaration of the ORDeviceBullet class.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Class declaration
#include "..\Common_Physics\physics_common.h"

#include <memory.h>

#define		PHYSICS_NEWTON_TITLE			"Newton"
#define		PHYSICS_BULLET_TITLE			"Bullet"

#define		PHYSICS_NEWTON_VERSION			3.16
#define		PHYSICS_BULLET_VERSION			2.82

//--- Registration defines
#define ORDEVICEPHYSICS__CLASSNAME		ORDevicePhysics
#define ORDEVICEPHYSICS__CLASSSTR		"ORDevicePhysics"


//! Bullet physics engine device.
class ORDevicePhysics : public FBDevice
{
	//--- FiLMBOX declaration
	FBDeviceDeclare( ORDevicePhysics, FBDevice );

public:
	//--- FiLMBOX Construction/Destruction
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

	//--- Initialisation/Shutdown
	bool  Init();					//!< Initialization routine.
	bool  Done();					//!< Device removal.
	bool  Reset();					//!< Reset function.
	bool  Stop();					//!< Device online routine.
	bool  Start();					//!< Device offline routine.

	//--- Real-Time Engine callbacks
	virtual bool AnimationNodeNotify( FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo	);		//!< Real-time evaluation function.
	virtual void DeviceIONotify		( kDeviceIOs  pAction, FBDeviceNotifyInfo &pDeviceNotifyInfo	);		//!< Hardware I/O notification.

	//--- Device operation
	virtual bool DeviceOperation	( kDeviceOperations pOperation									);		//!< Operate device.

	//--- Load/Save.
	virtual bool FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat		);		//!< Store configuration in FBX.
	virtual bool FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat		);		//!< Retrieve configuration from FBX.

	//--- Recording of frame information
	void	DeviceRecordFrame			(FBTime &pTime,FBDeviceNotifyInfo &pDeviceNotifyInfo);

	void ClearLevel() { if (mHardware.get()) mHardware->ClearLevel(); }
	bool LoadLevel( FBComponentList &list );

	//PhysicsCar	*CreateCar( const CarInfo *info, void *userdata=nullptr ) { if (mHardware) return CreateNewPhysicsCar( mHardware, info, userdata ); else return nullptr; }
	//void RegisterCar( PhysicsCar *car ) { if (mHardware) mHardware->RegisterCar(car); }
	//void UnRegisterCar( PhysicsCar *car ) { if (mHardware) mHardware->UnRegisterCar(car); }

	PHYSICS_INTERFACE::IWorld	*GetWorldPtr() {
		return mHardware.get();
	}

	// create a new car depends on a current physics engine
	PHYSICS_INTERFACE::IBody		*CreateNewBody( PHYSICS_INTERFACE::BodyOptions *pOptions, const PHYSICS_INTERFACE::IQueryGeometry *pGeometry, bool convexhull );
	PHYSICS_INTERFACE::ICar			*CreateNewCar( PHYSICS_INTERFACE::CarOptions *pOptions, const PHYSICS_INTERFACE::IQueryGeometry *pCarGeometry[5] );

	void	ResetPhysics() { if (mHardware.get()) mHardware->Reset(); }

	void	SaveState() { if (mHardware.get()) mHardware->SaveState(); }
	void	RestoreState() { if (mHardware.get()) mHardware->RestoreState(); }

	// setup collisions, and phys elements with a connected constraints
	void	EnterOnline();
	// goes to offline mode
	void	LeaveOnline();

	void WaitForUpdateToFinish() { if (mHardware.get()) mHardware->WaitForUpdateToFinish(); }
	void StartPause() { mUpdatePause = true; }
	void StopPause() { 
		mUpdatePause = false; 
		if (mHardware.get()) mHardware->Reset();
	}

	void DrawDebug() const { if (mHardware.get()) mHardware->DrawDebug(); }

	void DoRebuildCollisions();
	// after we change world scale - rebuild all physics elements
	void DoRestart();

	bool IsRebuildNeeded() { return mNeedRebuild; }

public:
	FBPropertyInt						EvaluateRate;		// number of samples per second (default 120)
	FBPropertyListObject				StaticCollisions;	// generate world static collision tree from these objects
	FBPropertyAction					RebuildCollisions;	// execute action for rebuilding collision next activation time

	FBPropertyBool						RealTimeMode;		// play physics in live mode or player mode
	FBPropertyInt						ResetFrame;			// frame for resetting physics state in player mode

	FBPropertyVector3d					Gravity;			// give option to customize gravity force
	FBPropertyDouble					WorldScale;			// coeff for scaling whole physics world to renderable world

	FBPropertyBool						DisplayDebug;		//! draw debug information

private:
	std::auto_ptr<PHYSICS_INTERFACE::IWorld>	mHardware;					//!< Handle onto hardware.
	FBPlayerControl						mPlayerControl;				//!< To get play mode for recording.
	FBSystem							mSystem;

	FBTime								mEvalTime;

	FBComponentList						mCollisions;

	bool								mNeedRebuild;

	bool								mUpdatePause;		// check if we are re calculating the static collisions
};

#endif /* __ORDEVICE_BULLET_DEVICE_H__ */
