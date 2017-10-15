#ifndef __ORCONSTRAINT_CARPHYSICS_CONSTRAINT_H__
#define __ORCONSTRAINT_CARPHYSICS_CONSTRAINT_H__

/**	\file	orconstraint_template_constraint.h
*	Declaration of a simple constraint class.
*	Simple constraint class declaration (FBSimpleConstraint).
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "..\Common_Physics\physics_common.h"
#include "queryFBGeometry.h"

#define	ORCONSTRAINT__CLASSNAME		ORConstraint_CarPhysics
#define ORCONSTRAINT__CLASSSTR		"ORConstraint_CarPhysics"

/////////////////////////////

const char * FBPropertyBaseEnum<PHYSICS_INTERFACE::CarPreset>::mStrings[] = {
	"Dodge Viper",
	"Sedan",
	"Jeep",
	0
};

const char * FBPropertyBaseEnum<PHYSICS_INTERFACE::CarDriveType>::mStrings[] = {
	"Front Wheels",
	"Rear Wheels",
	"4x4",
	0
};

//////////////////////////////////////////////////////////////////////////////
//! A simple constraint class.
class ORConstraint_CarPhysics : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( ORConstraint_CarPhysics, FBConstraint );

public:
	//--- Creation & Destruction
	virtual bool			FBCreate() override;						//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes() override;			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes() override;			//!< Setup animation nodes.


	virtual void			SnapSuggested		() override;			//!< Suggest 'snap'.
	virtual void			FreezeSuggested		() override;			//!< Suggest 'freeze'.

	//--- Constraint Status interface
	virtual bool			Disable		( FBModel* pModel ) override;	//!< Disable the constraint on a model.

	//--- Real-Time Engine
	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo ) override;

	virtual void			FreezeSRT	( FBModel* pModel, bool pS, bool pR, bool pT ) override;						//!< Freeze Scaling, Rotation and Translation for a model.

	virtual bool			ReferenceAddNotify		( int pGroupIndex, FBModel* pModel ) override;					//!< Reference added: Callback.
	virtual bool			ReferenceRemoveNotify	( int pGroupIndex, FBModel* pModel ) override;					//!< Reference removed: Callback.


	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Retrieval.

	FBPropertyDouble		Speed;		// read-only: display current car speed value
	FBPropertyDouble		RPM;		// read-only: display engine RPM value	

	FBPropertyDouble		Mass;		// car chassis mass
	FBPropertyDouble		TopSpeed;	// in km/hours
	FBPropertyDouble		Friction;	// car friction

	FBPropertyDouble		TireMass;

	FBPropertyDouble		SuspensionLength;
	FBPropertyDouble		SuspensionSpring;
	FBPropertyDouble		SuspensionDamper;

	FBPropertyBaseEnum<PHYSICS_INTERFACE::CarPreset>			Presets;
	FBPropertyBaseEnum<PHYSICS_INTERFACE::CarDriveType>			DriveType;

	//
	//	ADVANCE CAR PROPERTIES
	//

	FBPropertyDouble		IdleTorque;
	FBPropertyDouble		IdleTorqueRPM;

	FBPropertyDouble		PeakHorsePower;
	FBPropertyDouble		PeakHorsePowerRPM;

	FBPropertyDouble		PeakTorque;
	FBPropertyDouble		PeakTorqueRPM;

	FBPropertyDouble		RedlineTorque;
	FBPropertyDouble		RedlineTorqueRPM;

	FBPropertyDouble		MassYOffset;		// y offset
	FBPropertyDouble		MassXOffset;		// x offset

	FBPropertyDouble		TireSteerAngle;		// maximum angle for tire steering

	FBPropertyInt			GearBoxCount;	// number of gears in the gearbox
	FBPropertyDouble		TireGear1;
	FBPropertyDouble		TireGear2;
	FBPropertyDouble		TireGear3;
	FBPropertyDouble		TireGear4;
	FBPropertyDouble		TireGear5;
	FBPropertyDouble		TireGear6;
	FBPropertyDouble		TireGearReverse;

	FBPropertyDouble		TireBrakeTorque;
	FBPropertyDouble		TireHandBrakeTorque;

	//
	// CAR CONTROLLING PROPERTIES
	//
	FBPropertyAnimatableDouble	Brake;		// for all 4 wheels 0 - 1.0 for applying a brake
	FBPropertyAnimatableDouble	HandBrake;	// only for rear wheels
	FBPropertyAnimatableDouble	Torque;		// +15 - move forward, -15 - move back
	FBPropertyAnimatableDouble	Steering;	// 1.0 - turn left, -1.0 - turn right
	//FBPropertyAnimatableDouble	SteeringDirect;	// - direct controlling of a steer angle

	FBPropertyInt				Gear;		// read-write: display current gear or set a current gear (for manual gearbox)
	FBPropertyBool				AutomaticGear;


	FBPropertyListObject	Device;		// source physics device		

	///////////////
	void					SaveState();
	void					RestoreState();

	bool					InitCar();
	void					FreeCar();

	void					ApplyPlayerControl(FBEvaluateInfo* pEvaluateInfo);
	void					UpdatePhysStates();

	void					DebugDraw();

	// Will be automatically called when a property will be changed.
    static void SetFrictionProperty(HIObject pObject, double value);
	static void SetTireMassProperty(HIObject pObject, double value);
	static void SetSuspensionLengthProperty(HIObject pObject, double value);
	static void SetSuspensionSpringProperty(HIObject pObject, double value);
	static void SetSuspensionDamperProperty(HIObject pObject, double value);

	void UpdateProperties();

	static void AddPropertiesToPropertyViewManager();

	void DoSetPropertiesFromPreset(const PHYSICS_INTERFACE::CarPreset preset);

	////////////////////////////////////////////
	// main functions for preparing phys elements to go
	void					EnterOnline();
	void					LeaveOnline();

private:

	QueryFBGeometry						mCarGeometry[5];

	PHYSICS_INTERFACE::ICar				*mCar;

	struct Node {

		FBAnimationNode		*T;
		FBAnimationNode		*R;

		//FBMatrix				initState;
		//FBMatrix				lastState;		// store last processed state

		//! a constructor
		Node()
		{
			T = nullptr;
			R = nullptr;
		}

		void Set( FBAnimationNode *_T, FBAnimationNode *_R)
		{
			T = _T;
			R = _R;
			//initState = _initState;
		}

	}	mNodes[5];		// chassis and 4 wheels

	bool AssignDevice();
	void FillCarInfo( PHYSICS_INTERFACE::CarOptions &options );
};

#endif	/* __ORCONSTRAINT_CARPHYSICS_CONSTRAINT_H__ */
