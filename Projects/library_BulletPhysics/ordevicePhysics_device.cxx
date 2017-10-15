
/**	\file	ordevicebullet_device.cxx
*	Definition of a simple input device class.
*	Function definitions for the ORDeviceInput class.
*/

//--- Class declaration
#include "ordevicephysics_device.h"
#include "orconstraint_CarPhysics_constraint.h"
#include "orconstraint_RigidBodies_constraint.h"
#include "..\NewtonPhysicsLibrary\newton_interface.h"

//--- Registration defines
#define ORDEVICEPHYSICS__CLASS	ORDEVICEPHYSICS__CLASSNAME
#define ORDEVICEPHYSICS__NAME	ORDEVICEPHYSICS__CLASSSTR
#define ORDEVICEPHYSICS__LABEL	"Physics Device"
#define ORDEVICEPHYSICS__DESC	"Physics Engine"
#define ORDEVICEPHYSICS__PREFIX	"Physics"

//--- FiLMBOX implementation and registration
FBDeviceImplementation	(	ORDEVICEPHYSICS__CLASS	);
FBRegisterDevice		(	ORDEVICEPHYSICS__NAME,
							ORDEVICEPHYSICS__CLASS,
							ORDEVICEPHYSICS__LABEL,
							ORDEVICEPHYSICS__DESC,
							"devices_physics.png"		);	// Icon filename (default=Open Reality icon)

void ORDevicePhysics_RebuildCollisions( HIObject pObject, bool value )
{     
    ORDevicePhysics* lDevice = FBCast<ORDevicePhysics>(pObject);
    
	// update wheel information
	if (lDevice && value) {
		lDevice->DoRebuildCollisions();
	}
}

void ORDevicePhysics_SetWorldScale( HIObject pObject, double value )
{     
    ORDevicePhysics* lDevice = FBCast<ORDevicePhysics>(pObject);
    
	// update wheel information
	if (lDevice) {
		lDevice->WorldScale.SetPropertyValue(value);
		lDevice->DoRestart();
	}
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORDevicePhysics::FBCreate()
{
	// Device sampling information
	SamplingMode = kFBSoftwareTimestamp;
	
	FBPropertyPublish(this, EvaluateRate, "Evaluate Rate", nullptr, nullptr);
	FBPropertyPublish(this, StaticCollisions, "Static Collisions", nullptr, nullptr);
	FBPropertyPublish(this, RebuildCollisions, "Rebuild Collisions", nullptr, nullptr);

	FBPropertyPublish(this, RealTimeMode, "Live Mode", nullptr, nullptr);
	FBPropertyPublish(this, ResetFrame, "Reset Frame", nullptr, nullptr);

	FBPropertyPublish(this, Gravity, "Gravity", nullptr, nullptr);
	FBPropertyPublish(this, WorldScale, "World Scale", nullptr, nullptr);

	FBPropertyPublish(this, DisplayDebug, "Display Debug", nullptr, nullptr);

	EvaluateRate = 120;
	StaticCollisions.SetFilter( FBModel::GetInternalClassId() );

	RealTimeMode = true;
	ResetFrame = 0;

	Gravity = FBVector3d(0.0, - 10.0 * 2.0, 0.0);
	WorldScale = 0.1;

	DisplayDebug = false;

	mUpdatePause = false;
	//mHardware.reset( CreateNewNewtonWorld(WorldScale) );
//	mHardware = new NewtonHardware(WorldScale);

	mNeedRebuild = true;

	return true;
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ORDevicePhysics::FBDestroy()
{
	mHardware.reset(nullptr);
}


/************************************************
 *	Device operation.
 ************************************************/
bool ORDevicePhysics::DeviceOperation( kDeviceOperations pOperation )
{
	// Must return the online/offline status of device.
	switch (pOperation)
	{
		case kOpInit:	return Init();
		case kOpStart:	return Start();
		case kOpStop:	return Stop();
		case kOpReset:	return Reset();
		case kOpDone:	return Done();
	}
	return FBDevice::DeviceOperation( pOperation );
}


/************************************************
 *	Initialization of device.
 ************************************************/
bool ORDevicePhysics::Init()
{
	mHardware.reset( CreateNewNewtonWorld(WorldScale) );
	return true;
}


/************************************************
 *	Removal of device.
 ************************************************/
bool ORDevicePhysics::Done()
{
	mHardware.reset(nullptr);
	return true;
}


/************************************************
 *	Device is stopped (offline).
 ************************************************/
bool ORDevicePhysics::Stop()
{
	FBProgress	lProgress;
	lProgress.Caption	= "Shutting down device";

	if ( mHardware.get() == nullptr )
		return false;

	// Step 1: Stop data from streaming.
	lProgress.Text		= "Stopping data stream";
	Information			= "Stopping data stream";
	if(! mHardware->StopDataStream() )
	{
		Information = "Could not stop data stream.";
	}

	// Step 2: Close down device
	lProgress.Text		= "Closing device communication";
	Information			= "Closing device communication";
	if(! mHardware->Close() )
	{
		Information = "Could not close device";
	}

    return true;
}


/************************************************
 *	Device is started (online).
 ************************************************/
bool ORDevicePhysics::Start()
{
	FBProgress	Progress;

	Progress.Caption	= "Setting up device";

	if (mHardware.get() == nullptr)
		return false;

	// Step 1: Open device
	if(! mHardware->Open() )
	{
		Information = "Could not open device";
		return false;
	}

	// Step 2: Ask hardware to get channel information
	Progress.Text	= "Device found, scanning for channel information...";
	Information		= "Retrieving channel information";
	if(!mHardware->GetSetupInfo())
	{
		Information = "Could not get channel information from device.";
		return false;
	}

	// Step 4: Start data stream
	if(! mHardware->StartDataStream() )
	{
		Information = "Could not start data stream.";
		return false;
	}

	Information = "Ready";
	HardwareVersionInfo = mHardware->GetPhysicsInfo();


    return true; // if true the device is online
}


/************************************************
 *	Reset of device.
 ************************************************/
bool ORDevicePhysics::Reset()
{
    Stop();
    return Start();
}


/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/
bool ORDevicePhysics::AnimationNodeNotify(FBAnimationNode* pAnimationNode ,FBEvaluateInfo* pEvaluateInfo)
{
    return true;
}


/************************************************
 *	Real-Time Synchronous Device IO.
 ************************************************/

void ORDevicePhysics::DeviceIONotify( kDeviceIOs pAction,FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
	
	if (mUpdatePause || (mHardware.get() == nullptr)) 
		return;

	if (RealTimeMode)
	{
		mEvalTime = mSystem.SystemTime;
	}
	else
	{
		FBTime lTime = mSystem.LocalTime;

		if (mEvalTime != lTime)
		{
			FBTime resetTime(0,0,0, ResetFrame);

			if (lTime == resetTime)
			{
				mHardware->RestoreState();
			}

			mEvalTime = lTime;
		}
	}

    switch (pAction)
	{
		case kIOPlayModeWrite:
		case kIOStopModeWrite:
		{
			// Output devices
		}
		break;

		case kIOStopModeRead:
		case kIOPlayModeRead:
		{
			// update gravity value
			double value[3];
			Gravity.GetData( value, sizeof(double)*3 );
			mHardware->SetGravity( &value[0] );

			// Input devices
			const double stime = mEvalTime.GetSecondDouble();
			while(mHardware->FetchDataPacket(stime))
			{
				//
				AckOneSampleReceived();
			}
		}
		break;
	}
}


/************************************************
 *	Record a frame of the device (recording).
 ************************************************/
void ORDevicePhysics::DeviceRecordFrame(FBTime &pTime,FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
}


/************************************************
 *	Store data in FBX.
 ************************************************/
bool ORDevicePhysics::FbxStore(FBFbxObject* pFbxObject,kFbxObjectStore pStoreWhat)
{
	return true;
}


/************************************************
 *	Retrieve data from FBX.
 ************************************************/
bool ORDevicePhysics::FbxRetrieve(FBFbxObject* pFbxObject,kFbxObjectStore pStoreWhat)
{
	return true;
}

void ORDevicePhysics::DoRebuildCollisions()
{
	mNeedRebuild = true;
}

void ORDevicePhysics::DoRestart()
{

	// rebuild collisions
	mNeedRebuild = true;
}

bool ORDevicePhysics::LoadLevel( FBComponentList &list )
{
	if (mHardware.get() == nullptr) return false;

	LevelGeometry	info(list);
	mHardware->LoadLevel( &info );

	mNeedRebuild = false;

	return true;
}


PHYSICS_INTERFACE::IBody *ORDevicePhysics::CreateNewBody( PHYSICS_INTERFACE::BodyOptions *pOptions, const PHYSICS_INTERFACE::IQueryGeometry *pGeometry, bool convexhull  )
{
	if (mHardware.get() != nullptr)
		return mHardware->CreateNewBody( pOptions, pGeometry, convexhull );

	return nullptr;
}

PHYSICS_INTERFACE::ICar *ORDevicePhysics::CreateNewCar( PHYSICS_INTERFACE::CarOptions *pOptions, const PHYSICS_INTERFACE::IQueryGeometry *pCarGeometry[5] )
{
	if (mHardware.get() != nullptr)
		return mHardware->CreateNewCar( pOptions, pCarGeometry );

	return nullptr;
}

void ORDevicePhysics::EnterOnline()
{
	WaitForUpdateToFinish();
	StartPause();
	
	

	//
	// prepare all connected constraints

	for (int i=0; i<mSystem.Scene->Constraints.GetCount(); ++i)
	{
		FBConstraint *pConstraint = mSystem.Scene->Constraints[i];
		
		if ( FBIS(pConstraint, ORConstraint_RigidBodies) )
		{
			( (ORConstraint_RigidBodies*) pConstraint)->EnterOnline();
		}
		else if (FBIS(pConstraint, ORConstraint_CarPhysics) )
		{
			( (ORConstraint_CarPhysics*) pConstraint)->EnterOnline();
		}
	}

	//
	// properties disable

	WorldScale.SetEnable(false);
	StaticCollisions.SetEnable(false);

	//
	// static collisions
	//
	FBProperty *pProperty = &StaticCollisions;
	FBPropertyListObject *pPropList = (FBPropertyListObject*) pProperty;

	if (pPropList->GetCount() == 0)
	{
		mCollisions.Clear();
		ClearLevel();
	}
	else
	{
		bool IsListChanged = false;

		if (IsRebuildNeeded() == false)
		{
			if (mCollisions.GetCount() != pPropList->GetCount() )
			{
				IsListChanged = true;
			}
			else
			{
				for (int i=0; i<mCollisions.GetCount(); ++i)
					if ( mCollisions.GetAt(i) != pPropList->GetAt(i) ) 
					{
						IsListChanged = true;
						break;
					}
			}
		}

		if (IsListChanged || IsRebuildNeeded() )
		{
			mCollisions.SetCount( pPropList->GetCount() );

			for (int i=0; i<mCollisions.GetCount(); ++i)
				mCollisions.SetAt(i, pPropList->GetAt(i) );

			LoadLevel( mCollisions );
		}
	}

	StopPause();
}

void ORDevicePhysics::LeaveOnline()
{
	WorldScale.SetEnable(true);
	StaticCollisions.SetEnable(true);

	for (int i=0; i<mSystem.Scene->Constraints.GetCount(); ++i)
	{
		FBConstraint *pConstraint = mSystem.Scene->Constraints[i];
		
		if ( FBIS(pConstraint, ORConstraint_RigidBodies) )
		{
			( (ORConstraint_RigidBodies*) pConstraint)->LeaveOnline();
		}
		else if (FBIS(pConstraint, ORConstraint_CarPhysics) )
		{
			( (ORConstraint_CarPhysics*) pConstraint)->LeaveOnline();
		}
	}
}