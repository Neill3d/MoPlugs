
/**	\file	orconstraint_template_constraint.cxx
*	Definition of a simple constraint class.
*	Simple constraint function definitions for the FBSimpleConstraint
*	class.
*/

//--- Class declarations
#include "orconstraint_CarPhysics_constraint.h"
#include "ordevicePhysics_device.h"

#include "../NewtonPhysicsLibrary/newton_interface.h"

//--- Registration defines
#define	ORCONSTRAINT__CLASS		ORCONSTRAINT__CLASSNAME
#define ORCONSTRAINT__NAME		"Car Physics"
#define	ORCONSTRAINT__LABEL		"Car Physics"
#define ORCONSTRAINT__DESC		"Car Physics Constraint"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINT__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINT__NAME,
								ORCONSTRAINT__CLASS,
								ORCONSTRAINT__LABEL,
								ORCONSTRAINT__DESC,
								FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)


void ORConstraint_CarPhysics::SetSuspensionLengthProperty( HIObject pObject, double value )
{     
    ORConstraint_CarPhysics* lConstraint = FBCast<ORConstraint_CarPhysics>(pObject);
    
	// update wheel information
	if (lConstraint) {
		lConstraint->SuspensionLength.SetPropertyValue(value);
		if (lConstraint->Active) lConstraint->UpdateProperties();
	}
}

void ORConstraint_CarPhysics::SetSuspensionSpringProperty( HIObject pObject, double value )
{     
    ORConstraint_CarPhysics* lConstraint = FBCast<ORConstraint_CarPhysics>(pObject);
    
	// update wheel information
	if (lConstraint) {
		lConstraint->SuspensionSpring.SetPropertyValue(value);
		if (lConstraint->Active) lConstraint->UpdateProperties();
	}
}

void ORConstraint_CarPhysics::SetSuspensionDamperProperty( HIObject pObject, double value )
{     
    ORConstraint_CarPhysics* lConstraint = FBCast<ORConstraint_CarPhysics>(pObject);
    
	// update wheel information
	if (lConstraint) {
		lConstraint->SuspensionDamper.SetPropertyValue(value);
		if (lConstraint->Active) lConstraint->UpdateProperties();
	}
}

void ORConstraint_CarPhysics::SetTireMassProperty( HIObject pObject, double value )
{     
    ORConstraint_CarPhysics* lConstraint = FBCast<ORConstraint_CarPhysics>(pObject);
    
	// update wheel information
	if (lConstraint) {
		lConstraint->TireMass.SetPropertyValue(value);
		if (lConstraint->Active) lConstraint->UpdateProperties();
	}
}

void ORConstraint_CarPhysics::SetFrictionProperty( HIObject pObject, double value )
{     
    ORConstraint_CarPhysics* lConstraint = FBCast<ORConstraint_CarPhysics>(pObject);
    
	// update wheel information
	if (lConstraint) {
		lConstraint->Friction.SetPropertyValue(value);
		if (lConstraint->Active) lConstraint->UpdateProperties();
	}
}

void SetMassProperty( HIObject pObject, double value )
{     
    ORConstraint_CarPhysics* lConstraint = FBCast<ORConstraint_CarPhysics>(pObject);
    
	// update wheel information
	if (lConstraint) {
		lConstraint->Mass.SetPropertyValue(value);
		if (lConstraint->Active) lConstraint->UpdateProperties();
	}
}

void SetMassXOffsetProperty( HIObject pObject, double value )
{     
    ORConstraint_CarPhysics* lConstraint = FBCast<ORConstraint_CarPhysics>(pObject);
    
	// update wheel information
	if (lConstraint) {
		lConstraint->MassXOffset.SetPropertyValue(value);
		if (lConstraint->Active) lConstraint->UpdateProperties();
	}
}

void SetMassYOffsetProperty( HIObject pObject, double value )
{     
    ORConstraint_CarPhysics* lConstraint = FBCast<ORConstraint_CarPhysics>(pObject);
    
	// update wheel information
	if (lConstraint) {
		lConstraint->MassYOffset.SetPropertyValue(value);
		if (lConstraint->Active) lConstraint->UpdateProperties();
	}
}

void SetCarPreset( HIObject pObject, PHYSICS_INTERFACE::CarPreset preset )
{     
    ORConstraint_CarPhysics* lConstraint = FBCast<ORConstraint_CarPhysics>(pObject);
    
	// update wheel information
	if (lConstraint) {
		lConstraint->DoSetPropertiesFromPreset(preset);
		lConstraint->Presets.SetPropertyValue(preset);
	}
}

void AddPropertyViewForCarPhysics(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(ORCONSTRAINT__CLASSSTR, pPropertyName, pHierarchy);
}

void ORConstraint_CarPhysics::AddPropertiesToPropertyViewManager()
{
	
	AddPropertyViewForCarPhysics("Active", "");
	AddPropertyViewForCarPhysics("Lock", "");
	AddPropertyViewForCarPhysics("Snap", "");
	AddPropertyViewForCarPhysics("Weight", "");

	AddPropertyViewForCarPhysics("Device", "");
	AddPropertyViewForCarPhysics("Presets", "");

	//
	AddPropertyViewForCarPhysics("Info", "", true);
	AddPropertyViewForCarPhysics("Speed", "Info");
	AddPropertyViewForCarPhysics("RPM", "Info");
	
	//
	AddPropertyViewForCarPhysics("Common", "", true);
	AddPropertyViewForCarPhysics("Drive Type", "Common");
	AddPropertyViewForCarPhysics("Mass", "Common");
	AddPropertyViewForCarPhysics("Top Speed", "Common");
	AddPropertyViewForCarPhysics("Friction", "Common");

	AddPropertyViewForCarPhysics("Mass X Offset", "Common");
	AddPropertyViewForCarPhysics("Mass Y Offset", "Common");

	AddPropertyViewForCarPhysics("Common.Tire", "Common", true);
	AddPropertyViewForCarPhysics("Tire Mass", "Common.Tire");
	AddPropertyViewForCarPhysics("Tire Steer Angle", "Common.Tire");
	AddPropertyViewForCarPhysics("Tire Brake Torque", "Common.Tire");

	AddPropertyViewForCarPhysics("Suspension Length", "Common.Tire");
	AddPropertyViewForCarPhysics("Suspension Spring", "Common.Tire");
	AddPropertyViewForCarPhysics("Suspension Damper", "Common.Tire");

	AddPropertyViewForCarPhysics("Common.Engine", "Common", true);
	AddPropertyViewForCarPhysics("Idle Torque", "Common.Engine");
	AddPropertyViewForCarPhysics("Idle Torque RPM", "Common.Engine");
	AddPropertyViewForCarPhysics("Peak Torque", "Common.Engine");
	AddPropertyViewForCarPhysics("Peak Torque RPM", "Common.Engine");
	AddPropertyViewForCarPhysics("Peak Horse Power", "Common.Engine");
	AddPropertyViewForCarPhysics("Peak Horse Power RPM", "Common.Engine");
	AddPropertyViewForCarPhysics("Redline Torque", "Common.Engine");
	AddPropertyViewForCarPhysics("Redline Torque RPM", "Common.Engine");

	AddPropertyViewForCarPhysics("Common.Gearbox", "Common", true);
	AddPropertyViewForCarPhysics("GearBox Count", "Common.Gearbox");
	AddPropertyViewForCarPhysics("Gear 1 Ratio", "Common.Gearbox");
	AddPropertyViewForCarPhysics("Gear 2 Ratio", "Common.Gearbox");
	AddPropertyViewForCarPhysics("Gear 3 Ratio", "Common.Gearbox");
	AddPropertyViewForCarPhysics("Gear 4 Ratio", "Common.Gearbox");
	AddPropertyViewForCarPhysics("Gear 5 Ratio", "Common.Gearbox");
	AddPropertyViewForCarPhysics("Gear 6 Ratio", "Common.Gearbox");
	AddPropertyViewForCarPhysics("Gear Reverse Ratio", "Common.Gearbox");

	// 
	AddPropertyViewForCarPhysics("Controlling", "", true);
	AddPropertyViewForCarPhysics("Automatic Gearbox", "Controlling");
	AddPropertyViewForCarPhysics("Gear", "Controlling");
	//AddPropertyViewForCarPhysics("Steering Type", "Controlling");
	
	AddPropertyViewForCarPhysics("Torque", "Controlling");
	AddPropertyViewForCarPhysics("Steering", "Controlling");
	//AddPropertyViewForCarPhysics("Steering Direct", "Controlling");
	AddPropertyViewForCarPhysics("Brake", "Controlling");
	AddPropertyViewForCarPhysics("Hand Brake", "Controlling");
}

/************************************************
 *	Creation function.
 ************************************************/
bool ORConstraint_CarPhysics::FBCreate()
{
	/*
	*	1. Create Reference group & elements
	*	2. Set constraint variables (deformer,layout,description)
	*	3. Set pointers to NULL if necessary
	*/
	Deformer	= false;
	HasLayout	= true;
	Description = "Car Physics Constraint";
  
	ReferenceGroupAdd( "Chassis", 1 );
	ReferenceGroupAdd( "Front Left Wheel", 1 );
	ReferenceGroupAdd( "Front Right Wheel", 1 );
	ReferenceGroupAdd( "Rear Left Wheel", 1 );
	ReferenceGroupAdd( "Rear Right Wheel", 1 );

	mCar = nullptr;

	//
	// information properties

	FBPropertyPublish( this, Speed, "Speed", nullptr, nullptr );
	FBPropertyPublish( this, RPM, "RPM", nullptr, nullptr );
	
	Speed = 0.0;
	Speed.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	RPM = 0.0;
	RPM.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	//
	// common properties

	FBPropertyPublish( this, Mass, "Mass", nullptr, SetMassProperty );
	FBPropertyPublish( this, Friction, "Friction", nullptr, SetFrictionProperty );
	
	FBPropertyPublish( this, TireMass, "Tire Mass", nullptr, SetTireMassProperty );
	FBPropertyPublish( this, SuspensionLength, "Suspension Length", nullptr, SetSuspensionLengthProperty );
	FBPropertyPublish( this, SuspensionSpring, "Suspension Spring", nullptr, SetSuspensionSpringProperty );
	FBPropertyPublish( this, SuspensionDamper, "Suspension Damper", nullptr, SetSuspensionDamperProperty );
	FBPropertyPublish( this, Presets, "Presets", nullptr, SetCarPreset );
	FBPropertyPublish( this, DriveType, "Drive Type", nullptr, nullptr );
	//FBPropertyPublish( this, SteeringType, "Steering Type", nullptr, nullptr );

	DriveType = PHYSICS_INTERFACE::kCarDriveFrontWheels;
	//SteeringType = kCarSteeringTrigger;

	//
	// controlling properties

	FBPropertyPublish( this, Torque, "Torque", nullptr, nullptr );
	FBPropertyPublish( this, Steering, "Steering", nullptr, nullptr );
	//FBPropertyPublish( this, SteeringDirect, "Steering Direct", nullptr, nullptr );
	FBPropertyPublish( this, Brake, "Brake", nullptr, nullptr );
	FBPropertyPublish( this, HandBrake, "Hand Brake", nullptr, nullptr );
	FBPropertyPublish( this, Gear, "Gear", nullptr, nullptr );
	FBPropertyPublish( this, AutomaticGear, "Automatic Gearbox", nullptr, nullptr ); 
	
	Brake = 0.0;
	Brake.SetAnimated(true);
	HandBrake = 0.0;
	HandBrake.SetAnimated(true);
	Torque = 0.0;
	Torque.SetMinMax(0.0, 100.0, true, true);
	Torque.SetAnimated(true);
	Steering = 0.0;
	Steering.SetMinMax(-100.0, 100.0, false, false);
	Steering.SetAnimated(true);
	/*
	SteeringDirect = 0.0;
	SteeringDirect.SetMinMax(-45.0, 45.0, false, false);
	SteeringDirect.SetAnimated(true);
	*/
	//
	// advance properties

	FBPropertyPublish( this, IdleTorque, "Idle Torque", nullptr, nullptr );
	FBPropertyPublish( this, IdleTorqueRPM, "Idle Torque RPM", nullptr, nullptr );

	FBPropertyPublish( this, PeakTorque, "Peak Torque", nullptr, nullptr );
	FBPropertyPublish( this, PeakTorqueRPM, "Peak Torque RPM", nullptr, nullptr );

	FBPropertyPublish( this, PeakHorsePower, "Peak Horse Power", nullptr, nullptr );
	FBPropertyPublish( this, PeakHorsePowerRPM, "Peak Horse Power RPM", nullptr, nullptr );

	FBPropertyPublish( this, RedlineTorque, "Redline Torque", nullptr, nullptr );
	FBPropertyPublish( this, RedlineTorqueRPM, "Redline Torque RPM", nullptr, nullptr );

	FBPropertyPublish( this, MassXOffset, "Mass X Offset", nullptr, SetMassXOffsetProperty );
	FBPropertyPublish( this, MassYOffset, "Mass Y Offset", nullptr, SetMassYOffsetProperty );

	FBPropertyPublish( this, GearBoxCount, "GearBox Count", nullptr, nullptr );

	FBPropertyPublish( this, TireGear1, "Gear 1 Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TireGear2, "Gear 2 Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TireGear3, "Gear 3 Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TireGear4, "Gear 4 Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TireGear5, "Gear 5 Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TireGear6, "Gear 6 Ratio", nullptr, nullptr );

	FBPropertyPublish( this, TireGearReverse, "Gear Reverse Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TopSpeed, "Top Speed", nullptr, nullptr );

	FBPropertyPublish( this, TireSteerAngle, "Tire Steer Angle", nullptr, nullptr );
	FBPropertyPublish( this, TireBrakeTorque, "Tire Brake Torque", nullptr, nullptr );
	FBPropertyPublish( this, TireHandBrakeTorque, "Tire HandBrake Torque", nullptr, nullptr );

	GearBoxCount.SetMinMax(0.0, 6.0, true, true);

	//
	//
	//

	FBPropertyPublish( this, Device, "Device", nullptr, nullptr );

	Device.SetFilter( ORDevicePhysics::GetInternalClassId() );

	AssignDevice();

	Presets = PHYSICS_INTERFACE::kCarPresetViper;
	DoSetPropertiesFromPreset(PHYSICS_INTERFACE::kCarPresetViper);
	/*
	Active.SetEnable(false);
	Snap.SetEnable(false);
	Lock.SetEnable(false);
	*/
	return true;
}

void ORConstraint_CarPhysics::DoSetPropertiesFromPreset(const PHYSICS_INTERFACE::CarPreset preset)
{
	switch(preset)
	{
	case PHYSICS_INTERFACE::kCarPresetViper:
		Mass = 1560.0;
		Friction = 4.5;
	
		TireMass = 30.0;

		SuspensionLength = 50.0;	// 0.2
		SuspensionDamper = 600.0;
		SuspensionSpring = 15000.0;
	
		IdleTorque = 300.0;
		IdleTorqueRPM = 500.0;

		PeakTorque = 490.0;
		PeakTorqueRPM = 3700;

		PeakHorsePower = 450.0;
		PeakHorsePowerRPM = 5200.0;

		RedlineTorque = 30.0;
		RedlineTorqueRPM = 6000.0;

		MassYOffset = 0.0;	// * 0.01
		MassXOffset = 0.0;
	
		TireSteerAngle = 35.0;

		GearBoxCount = 6;

		TireGear1 = 2.66;
		TireGear2 = 1.78;
		TireGear3 = 1.30;
		TireGear4 = 1.00;
		TireGear5 = 0.74;
		TireGear6 = 0.50;
		TireGearReverse = 2.90;

		TopSpeed = 264.0;

		TireBrakeTorque = 2000.0;
		TireHandBrakeTorque = 500.0;
		break;
	case PHYSICS_INTERFACE::kCarPresetSedan:
		break;
	case PHYSICS_INTERFACE::kCarPresetJeep:
		break;
	}
}

void ORConstraint_CarPhysics::UpdateProperties()
{
	PHYSICS_INTERFACE::CarOptions	options;
	FillCarInfo(options);

	if (Device.GetCount() && mCar) 
		mCar->SetOptions(&options);
}

/************************************************
 *	Destruction function.
 ************************************************/
void ORConstraint_CarPhysics::FBDestroy()
{
	/*
	*	Free any user memory associated to constraint
	*/
	FreeCar();
}


/************************************************
 *	Refrence added notification.
 ************************************************/
bool ORConstraint_CarPhysics::ReferenceAddNotify( int pGroupIndex, FBModel* pModel )
{
	/*
	*	Perform action required when a refrence is added.
	*/
	return true;
}


/************************************************
 *	Reference removed notification.
 ************************************************/
bool ORConstraint_CarPhysics::ReferenceRemoveNotify( int pGroupIndex, FBModel* pModel )
{
	/*
	*	Perform action required when a refrence is removed.
	*/
	return true;
}


/************************************************
 *	Setup all of the animation nodes.
 ************************************************/



void ORConstraint_CarPhysics::FillCarInfo( PHYSICS_INTERFACE::CarOptions &info )
{
	info.MassYOffset = MassYOffset;
	info.MassXOffset = MassXOffset;
	info.Mass = Mass;
	info.pChassis = ReferenceGet(0);

	for (int i=0; i<4; ++i)
	{
		PHYSICS_INTERFACE::CarOptions::Wheel &wheel = info.wheels[i];

		wheel.Friction = Friction;
		wheel.Mass = TireMass;
		wheel.pModel = ReferenceGet(i+1);
		wheel.SuspensionDamper = SuspensionDamper;
		wheel.SuspensionLength = 20.0; // SuspensionLength;
		wheel.SuspensionSpring = SuspensionSpring;

		wheel.lateralStiffness = 20.0f;
		wheel.longitudinalStiffness = 10000.0f;
		wheel.aligningMomentTrail = 0.5f;
	}

	int gearCount = GearBoxCount;
	if (gearCount < 0 || gearCount > 6) gearCount = 6;

	info.GearCount = gearCount;
	info.GearReverseRatio = TireGearReverse;

	info.gear1 = TireGear1;
	info.gear2 = TireGear2;
	info.gear3 = TireGear3;
	info.gear4 = TireGear4;
	info.gear5 = TireGear5;
	info.gear6 = TireGear6;

	info.IdleRPM = IdleTorqueRPM;
	info.IdleTorquePoundPerFoot = IdleTorque;

	info.PeakHorsePower = PeakHorsePower;
	info.PeakHorsePowerRPM = PeakHorsePowerRPM;

	info.PeakTorquePoundPerFoot = PeakTorque;
	info.PeakTorqueRPM = PeakTorqueRPM;

	info.RedLineRPM = RedlineTorqueRPM;
	info.RedLineTorquePoundPerFoot = RedlineTorque;

	info.SteerAngle = TireSteerAngle;

	info.vehicleTopSpeedKPH = TopSpeed;

	info.TireBrakeTorque = TireBrakeTorque;
	info.TireHandBrakeTorque = TireHandBrakeTorque;
}

bool ORConstraint_CarPhysics::AssignDevice()
{
	if ( Device.GetCount() ) return true;

	// assign existing device or create a new one

	FBDevice *pDevice = nullptr;
	FBScene *pScene = FBSystem().Scene;
	for (int i=0; i<pScene->Devices.GetCount(); ++i)
	{
		if (FBIS( pScene->Devices[i], ORDevicePhysics) )
		{
			pDevice = pScene->Devices[i];
			break;
		}
	}

	if (pDevice != nullptr && (1==FBMessageBox("Physics", "Do you want to assign an existing device?", "Yes", "No")) )
	{
		Device.Add( pDevice );
		return true;
	}

	return false;
}



bool ORConstraint_CarPhysics::InitCar()
{
	if ( !ReferenceGet(0) || !ReferenceGet(1) || !ReferenceGet(2) || !ReferenceGet(3) || !ReferenceGet(4) )
	{
		return false;
	}

	FreeCar();

	if (AssignDevice() == false) return false;

	PHYSICS_INTERFACE::CarOptions options;
	FillCarInfo(options);

	for (int i=0; i<5; ++i)
		mCarGeometry[i].Prep( ReferenceGet(i) );

	const PHYSICS_INTERFACE::IQueryGeometry *ptr[5] = {&mCarGeometry[0], &mCarGeometry[1], &mCarGeometry[2], &mCarGeometry[3], &mCarGeometry[4]};

	mCar = ((ORDevicePhysics*) Device.GetAt(0))->CreateNewCar( &options, ptr );
	//mCar = CreateNewNewtonCar( ((ORDevicePhysics*) Device.GetAt(0))->GetWorldPtr(), &options, ptr, this );
	return (mCar != nullptr);
}

void ORConstraint_CarPhysics::FreeCar()
{
	if (mCar)	
	{
		// remove car structure
		delete mCar;
		mCar = nullptr;
	}
}

void ORConstraint_CarPhysics::SetupAllAnimationNodes()
{
	/*
	*	- Based on the existence of the references (ReferenceGet() function),
	*	  create the required animation nodes.
	*	- A source will use an Out node, whereas a destination will use
	*	  an In Node.
	*	- If the node is with respect to a deformation, then Bind the reference
	*	  to the deformation notification ( DeformerBind() )
	*/
	
	if ( ReferenceGet(0) && ReferenceGet(1) && ReferenceGet(2) && ReferenceGet(3) && ReferenceGet(4) )
	{
		
		// chassis
		mNodes[0].T = AnimationNodeInCreate ( 0, ReferenceGet(0), ANIMATIONNODE_TYPE_TRANSLATION );
		mNodes[0].R = AnimationNodeInCreate ( 1, ReferenceGet(0), ANIMATIONNODE_TYPE_ROTATION );

		// wheels
		for (int i=1; i<=4; ++i)
		{
			mNodes[i].T = AnimationNodeInCreate ( i*2, ReferenceGet(i), ANIMATIONNODE_TYPE_LOCAL_TRANSLATION );
			mNodes[i].R = AnimationNodeInCreate ( i*2+1, ReferenceGet(i), ANIMATIONNODE_TYPE_LOCAL_ROTATION );
		}

	}
	

	// TODO: resetup nodes for car chassis and wheels

}


/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void ORConstraint_CarPhysics::RemoveAllAnimationNodes()
{
	/*
	*	If nodes have been bound to deformations, remove the binding.
	*/
	/*
	if (mDummy_AnimationNode)
	{
		AnimationNodeDestroy(mDummy_AnimationNode);
		mDummy_AnimationNode = nullptr;
	}
	*/

	//
	//!		Nodes in constraint are automaticaly removed!
	//

	
	//FreeCar();
}


/************************************************
 *	Suggest a snap.
 ************************************************/
void ORConstraint_CarPhysics::SnapSuggested()
{
	/*
	*	Perform any pre-snap operations.
	*/

	if (ReferenceGet(0) && ReferenceGet(1) && ReferenceGet(2) && ReferenceGet(3) && ReferenceGet(4) )
	{
		AssignDevice();
		ORDevicePhysics *pDevice = (Device.GetCount() ) ? (ORDevicePhysics*) Device.GetAt(0) : nullptr;
		if (pDevice == nullptr) 
		{
			Active = false;
			FreeCar();
			return;
		}

		if (mCar == nullptr)
		{
			if (InitCar() == false) return;
			if (mCar == nullptr) return;
		}

		if (mCar) mCar->Snap();
	}
}


/************************************************
 *	Suggest a freeze.
 ************************************************/
void ORConstraint_CarPhysics::FreezeSuggested()
{
	/*
	*	Perform any pre-freeze operations
	*/
}


/************************************************
 *	Disable the constraint on pModel.
 ************************************************/
bool ORConstraint_CarPhysics::Disable(FBModel* pModel)
{
	/*
	*	Perform any operations to disable model
	*	Must call FBConstraint::Disable()
	*/
	return FBConstraint::Disable( pModel );
}

/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/
void ORConstraint_CarPhysics::ApplyPlayerControl(FBEvaluateInfo* pEvaluateInfo)
{
	// TODO: if property animated, better read using evaluate info

	//
	// Update forces (car control)
	//
	if (mCar)
	{
		PHYSICS_INTERFACE::CarInput	control;

		double gas, steering, brake, handbrake;

		if (Torque.GetAnimationNode() != nullptr)
			Torque.GetAnimationNode()->ReadData( &gas, pEvaluateInfo );
		else
			Torque.GetData( &gas, sizeof(double), pEvaluateInfo );
		
		if (Steering.GetAnimationNode() != nullptr)
			Steering.GetAnimationNode()->ReadData( &steering, pEvaluateInfo );
		else
			Steering.GetData( &steering, sizeof(double), pEvaluateInfo );

		if (Brake.GetAnimationNode() != nullptr)
			Brake.GetAnimationNode()->ReadData( &brake, pEvaluateInfo );
		else
			Brake.GetData( &brake, sizeof(double), pEvaluateInfo );

		if (HandBrake.GetAnimationNode() != nullptr)
			HandBrake.GetAnimationNode()->ReadData( &handbrake, pEvaluateInfo );
		else
			HandBrake.GetData( &handbrake, sizeof(double), pEvaluateInfo );

		//

		control.engineGasPedal = 0.01f * (float)gas;
		control.steeringVal = 0.01f * (float) steering;

		control.brakePedal = 0.01f * (float) brake;
		control.handBrakePedal = 0.01f * (float) handbrake;

		control.gear = (int) Gear;
		control.transmissionMode = AutomaticGear;

		mCar->SetPlayerControl( control );


		/*
		// apply values * dt
		mCar->SetDriveType( DriveType );
		mCar->ApplyTorque( torque * 0.1 );

		switch( SteeringType )
		{
		case kCarSteeringTrigger:
			mCar->ApplySteering( steering * 0.01 );
			break;
		case kCarSteeringDirect:
			mCar->ApplySteeringDirect( steeringDirect * 0.01 );
			break;
		}
		*/
	}
}

void ORConstraint_CarPhysics::UpdatePhysStates()
{
	Speed = mCar->GetSpeed();
	RPM = mCar->GetRPM();
}

bool ORConstraint_CarPhysics::AnimationNodeNotify(FBAnimationNode* pConnector,FBEvaluateInfo* pEvaluateInfo,FBConstraintInfo* pConstraintInfo)
{
	/*
	*	Constraint Evaluation
	*	1. Read data from sources
	*	2. Calculate necessary operations.
	*	3. Write output to destinations
	*	Note: Not the deformation operations !
	*/
	
	if ( Active == false ) return false;

	ORDevicePhysics *pDevice = (Device.GetCount() ) ? (ORDevicePhysics*) Device.GetAt(0) : nullptr;
	if (pDevice == nullptr) 
	{
		//Active = false;
		FreeCar();
		return false;
	}

	if (mCar == nullptr)
	{
		if (InitCar() == false) return false;
		if (mCar == nullptr) return false;
	}

	/*
	// is we are run asynchronous we need make sure no update in on flight.
	if (m_world) {
		NewtonWaitForUpdateToFinish (m_world);
	}
	*/

	ApplyPlayerControl(pEvaluateInfo);

	//
	// Update animation nodes
	//

	FBMatrix m;
	m.Set( mCar->GetChassisMatrix() );

	FBTVector T;
	FBRVector R;
	FBSVector S;

	FBMatrixToTRS( T, R, S, m );

	mNodes[0].T->WriteData(T, pEvaluateInfo);
	mNodes[0].R->WriteData(R, pEvaluateInfo);

	//
	// write data for each wheel
	//

	for (int i=1; i<=4; ++i)
	{
		m.Set( mCar->GetWheelMatrix(i-1, true) );
		FBMatrixToTRS( T, R, S, m );

		mNodes[i].T->WriteData(T, pEvaluateInfo);
		mNodes[i].R->WriteData(R, pEvaluateInfo);
	}

	//
	// update car information

	double speed = mCar->GetSpeed();
	double rpm = mCar->GetRPM();

	Speed = speed;
	RPM = rpm;

	return true;
}

void ORConstraint_CarPhysics::SaveState()
{
	if (mCar != nullptr)
		mCar->SaveState();
}

void ORConstraint_CarPhysics::RestoreState()
{
	if (mCar == nullptr)
		mCar->RestoreState();
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool ORConstraint_CarPhysics::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	FBX Storage of constraint parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORConstraint_CarPhysics::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	FBX Retrieval of constraint parameters.
	*/

	AssignDevice();
	ORDevicePhysics *pDevice = (Device.GetCount() ) ? (ORDevicePhysics*) Device.GetAt(0) : nullptr;
	if (pDevice == nullptr) 
	{
		Active = false;
		FreeCar();
		return false;
	}

	if (mCar == nullptr)
	{
		if (InitCar() == false) return false;
		if (mCar == nullptr) return false;
	}

	return true;
}


/************************************************
 *	Freeze SRT for the constraint.
 ************************************************/
void ORConstraint_CarPhysics::FreezeSRT	( FBModel* pModel, bool pS, bool pR, bool pT )
{
	/*
	*	Freeze the model's SRT parameters (if true,true,true)
	*	Must call FBConstraint::FreezeSRT
	*/
	FBConstraint::FreezeSRT( pModel, pS, pR, pT );
}


void ORConstraint_CarPhysics::DebugDraw()
{
	/*
	for (int i=0; i<mCar->GetTiresCount(); ++i)
	{
		const CustomDGRayCastCar::Tire& tire = mCar->GetTire (i);
		dMatrix matrix;
		matrix = mCar->CalculateTireMatrix(i);

		NewtonCollisionInfoRecord info;
		const NewtonCollision *collision = tire.m_shape;

		NewtonCollisionGetInfo (collision, &info);
	
		switch (info.m_collisionType) 
		{
			//case SERIALIZE_ID_TREE:
			//case SERIALIZE_ID_SCENE:
			case SERIALIZE_ID_USERMESH:
			case SERIALIZE_ID_HEIGHTFIELD:
			{
				break;
			}

			default: 
			{
				glBegin(GL_LINES);
				NewtonCollisionForEachPolygonDo (collision, &matrix[0][0], DebugShowGeometryCollision, NULL);
				glEnd();
				break;
			}
		}
	}
	*/
}


void ORConstraint_CarPhysics::EnterOnline()
{
}

void ORConstraint_CarPhysics::LeaveOnline()
{
}