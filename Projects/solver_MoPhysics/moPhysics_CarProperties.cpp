
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: moPhysics_CarProperties.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "moPhysics_CarProperties.h"
#include "moPhysics_solver.h"
#include <array>

FBPhysicalPropertiesImplementation( MOCarPhysProperties );
FBRegisterPhysicalProperties( "MOCarPhysProperties",
								MOCarPhysProperties,
								"Super Car",
								"Super Car Desc",
								FB_DEFAULT_SDK_ICON );


/*
         ___  ________ _____           ______ _               ______                          _   _           
         |  \/  |  _  /  __ \          | ___ \ |              | ___ \                        | | (_)          
         | .  . | | | | /  \/ __ _ _ __| |_/ / |__  _   _ ___ | |_/ / __ ___  _ __   ___ _ __| |_ _  ___  ___ 
         | |\/| | | | | |    / _` | '__|  __/| '_ \| | | / __||  __/ '__/ _ \| '_ \ / _ \ '__| __| |/ _ \/ __|
         | |  | \ \_/ / \__/\ (_| | |  | |   | | | | |_| \__ \| |  | | | (_) | |_) |  __/ |  | |_| |  __/\__ \
         \_|  |_/\___/ \____/\__,_|_|  \_|   |_| |_|\__, |___/\_|  |_|  \___/| .__/ \___|_|   \__|_|\___||___/
                                                     __/ |                   | |                              
                                                    |___/                    |_|                              
                                                       _               
                                                      | |              
                                                   ___| | __ _ ___ ___ 
                                                  / __| |/ _` / __/ __|
                                                 | (__| | (_| \__ \__ \
                                                  \___|_|\__,_|___/___/
                                                                                                                                              
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void MOCarPhysProperties::SetCarPreset( HIObject pObject, CarPreset preset )
{     
    MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->Presets.SetPropertyValue(preset);
		pProps->BeginMassiveChanges();
		pProps->DoSetPropertiesFromPreset(preset);
		pProps->EndMassiveChanges();
		pProps->DoUpdateProperties();
	}
}

void MOCarPhysProperties::SetDriveType( HIObject pObject, PHYSICS_INTERFACE::CarDriveType value )
{     
    MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->DriveType.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetWheelHasAFender( HIObject pObject, bool value )
{     
    MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->WheelHasAFender.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetTireSuspensionType( HIObject pObject, ETireSuspensionType value )
{     
    MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->TireSuspensionType.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetMass( HIObject pObject, double value )
{     
    MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->Mass.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetTopSpeed( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->TopSpeed.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetFriction( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->Friction.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetTireMass( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->TireMass.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetSuspensionLength( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->SuspensionLength.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetSuspensionSpring( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->SuspensionSpring.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetSuspensionDamper( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->SuspensionDamper.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetLateralStiffness( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->LateralStiffness.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetLongitudinalStiffness( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->LongitudinalStiffness.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetAligningMomentTrail( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->AligningMomentTrail.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetAutomaticGear( HIObject pObject, bool value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->AutomaticGear.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetDifferentialLock( HIObject pObject, bool value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->DifferentialLock.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetVehicleWeightDistribution( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->VehicleWeightDistribution.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetClutchFrictionTorque( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->ClutchFrictionTorque.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetEngineMass( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->EngineMass.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetEngineRadio( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->EngineRadio.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetMassOffsetX( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->MassXOffset.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetMassOffsetY( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->MassYOffset.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetFrontTireSteerAngle( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->FrontTireSteerAngle.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetRearTireSteerAngle( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->RearTireSteerAngle.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetTireBrakeTorque( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->TireBrakeTorque.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetTireHandBrakeTorque( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->TireHandBrakeTorque.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetUseCurve( HIObject pObject, bool value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->UseCurveForSteering.SetPropertyValue(value);
		pProps->DoUpdateProperties();
	}
}

void MOCarPhysProperties::SetCurveRadius( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->CurveSearchRadius.SetPropertyValue(value);
		pProps->DoUpdateProperties();
	}
}
void MOCarPhysProperties::SetCurveDistance( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->CurveDistanceAhead.SetPropertyValue(value);
		pProps->DoUpdateProperties();
	}
}
void MOCarPhysProperties::SetCurveWidth( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->CurvePathWidth.SetPropertyValue(value);
		pProps->DoUpdateProperties();
	}
}
void MOCarPhysProperties::SetCurveTangentMult( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->CurveTangenMult.SetPropertyValue(value);
		pProps->DoUpdateProperties();
	}
}
void MOCarPhysProperties::SetCurveDamping( HIObject pObject, double value )
{
	MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->CurveDamping.SetPropertyValue(value);
		pProps->DoUpdateProperties();
	}
}

void MOCarPhysProperties::SetDownforceWeightFactor0( HIObject pObject, double value )
{     
    MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->DownforceWeightFactor0.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetDownforceWeightFactor1( HIObject pObject, double value )
{     
    MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->DownforceWeightFactor1.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}
void MOCarPhysProperties::SetDownforceWeightFactorSpeed( HIObject pObject, double value )
{     
    MOCarPhysProperties* pProps = FBCast<MOCarPhysProperties>(pObject);
	if (pProps) {
		pProps->DownforceWeightFactorSpeed.SetPropertyValue(value);
		pProps->DoUpdateProperties();
		if (pProps->IsMassiveChanges() == false)
			pProps->Presets = kCarPresetCustom;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
bool MOCarPhysProperties::FBCreate()
{
	//
	mOnline = false;
	mWorld = nullptr;
	mCar = nullptr;

	mMassiveChanges = false;

	//
	// information properties

	FBPropertyPublish( this, Active, "Active", nullptr, nullptr );
	Active = true;

	FBPropertyPublish( this, Speed, "Speed", nullptr, nullptr );
	FBPropertyPublish( this, RPM, "RPM", nullptr, nullptr );
	FBPropertyPublish( this, DisplayGear, "Current Gear", nullptr, nullptr );

	//Speed.SetAnimated(true);
	Speed = 0.0;
	Speed.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	//RPM.SetAnimated(true);
	RPM = 0.0;
	RPM.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	DisplayGear = 0;
	DisplayGear.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	//
	// common properties

	FBPropertyPublish( this, Mass, "Mass", nullptr, SetMass );
	FBPropertyPublish( this, TopSpeed, "Top Speed", nullptr, SetTopSpeed );
	FBPropertyPublish( this, Friction, "Friction", nullptr, SetFriction );
	
	FBPropertyPublish( this, VehicleWeightDistribution, "Vehicle Weight Distribution", nullptr, SetVehicleWeightDistribution );
	FBPropertyPublish( this, ClutchFrictionTorque, "Clutch Friction Torque", nullptr, SetClutchFrictionTorque );

	FBPropertyPublish( this, EngineMass, "Engine Mass", nullptr, SetEngineMass );
	FBPropertyPublish( this, EngineRadio, "Engine Radio", nullptr, SetEngineRadio );

	FBPropertyPublish( this, MassXOffset, "Mass X Offset", nullptr, SetMassOffsetX );
	FBPropertyPublish( this, MassYOffset, "Mass Y Offset", nullptr, SetMassOffsetY );

	FBPropertyPublish( this, TireMass, "Tire Mass", nullptr, SetTireMass );
	FBPropertyPublish( this, SuspensionLength, "Suspension Length", nullptr, SetSuspensionLength );
	FBPropertyPublish( this, SuspensionSpring, "Suspension Spring", nullptr, SetSuspensionSpring );
	FBPropertyPublish( this, SuspensionDamper, "Suspension Damper", nullptr, SetSuspensionDamper );
	FBPropertyPublish( this, LateralStiffness, "Lateral Stiffness", nullptr, SetLateralStiffness );
	FBPropertyPublish( this, LongitudinalStiffness, "Longitudinal Stiffness", nullptr, SetLongitudinalStiffness );
	FBPropertyPublish( this, AligningMomentTrail, "Aligning Moment Trail", nullptr, SetAligningMomentTrail );
	FBPropertyPublish( this, Presets, "Presets", nullptr, SetCarPreset );
	FBPropertyPublish( this, DriveType, "Drive Type", nullptr, SetDriveType );
	//FBPropertyPublish( this, SteeringType, "Steering Type", nullptr, nullptr );

	VehicleWeightDistribution = 55.0;
	ClutchFrictionTorque = 2000.0;
	EngineMass = 100.0;
	EngineRadio = 12.5;
	DriveType = PHYSICS_INTERFACE::kCarDriveFrontWheels;
	//SteeringType = kCarSteeringTrigger;

	//
	// controlling properties

	FBPropertyPublish( this, Torque, "Torque", nullptr, nullptr );
	FBPropertyPublish( this, Clutch, "Clutch", nullptr, nullptr );
	FBPropertyPublish( this, Steering, "Steering", nullptr, nullptr );
	//FBPropertyPublish( this, SteeringDirect, "Steering Direct", nullptr, nullptr );
	FBPropertyPublish( this, Brake, "Brake", nullptr, nullptr );
	FBPropertyPublish( this, HandBrake, "Hand Brake", nullptr, nullptr );
	FBPropertyPublish( this, Gear, "Gear", nullptr, nullptr );
	FBPropertyPublish( this, AutomaticGear, "Automatic Gearbox", nullptr, SetAutomaticGear ); 
	FBPropertyPublish( this, DifferentialLock, "Differential Lock", nullptr, SetDifferentialLock );

	Brake = 0.0;
	Brake.SetAnimated(true);
	HandBrake = 0.0;
	HandBrake.SetAnimated(true);
	Torque = 0.0;
	Torque.SetMinMax(0.0, 100.0, true, true);
	Torque.SetAnimated(true);
	Clutch = 0.0;
	Clutch.SetMinMax(0.0, 100.0, true, true);
	Clutch.SetAnimated(true);
	Steering = 0.0;
	Steering.SetMinMax(-100.0, 100.0, false, false);
	Steering.SetAnimated(true);
	Gear = 1;
	Gear.SetMinMax(-1.0, 7.0, true, true);
	Gear.SetAnimated(true);

	DifferentialLock = false;
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

	FBPropertyPublish( this, GearBoxCount, "GearBox Count", nullptr, nullptr );

	FBPropertyPublish( this, TireGear1, "Gear 1 Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TireGear2, "Gear 2 Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TireGear3, "Gear 3 Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TireGear4, "Gear 4 Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TireGear5, "Gear 5 Ratio", nullptr, nullptr );
	FBPropertyPublish( this, TireGear6, "Gear 6 Ratio", nullptr, nullptr );

	FBPropertyPublish( this, TireGearReverse, "Gear Reverse Ratio", nullptr, nullptr );

	FBPropertyPublish( this, FrontTireSteerAngle, "Front Tire Steer Angle", nullptr, SetFrontTireSteerAngle );
	FBPropertyPublish( this, RearTireSteerAngle, "Rear Tire Steer Angle", nullptr, SetRearTireSteerAngle );
	FBPropertyPublish( this, TireBrakeTorque, "Tire Brake Torque", nullptr, SetTireBrakeTorque );
	FBPropertyPublish( this, TireHandBrakeTorque, "Tire HandBrake Torque", nullptr, SetTireHandBrakeTorque );

	FBPropertyPublish( this, DownforceWeightFactor0, "Downforce Weight Factor0", nullptr, SetDownforceWeightFactor0 );
	FBPropertyPublish( this, DownforceWeightFactor1, "Downforce Weight Factor1", nullptr, SetDownforceWeightFactor1 );
	FBPropertyPublish( this, DownforceWeightFactorSpeed, "Downforce Weight Factor Speed", nullptr, SetDownforceWeightFactorSpeed );

	FBPropertyPublish( this, WheelHasAFender, "Wheel Has A Fender", nullptr, SetWheelHasAFender );
	FBPropertyPublish( this, TireSuspensionType, "Tire Suspension Type", nullptr, SetTireSuspensionType );

	// AI curve - auto steering

	FBPropertyPublish( this, UseCurveForSteering, "Use Curve For Steering", nullptr, SetUseCurve );
	FBPropertyPublish( this, CurveSteeringWeight, "Curve Steering Weight", nullptr, nullptr );
	FBPropertyPublish( this, Curve, "Curve", nullptr, nullptr );
	FBPropertyPublish( this, CurveSamples, "Curve Samples", nullptr, nullptr );

	FBPropertyPublish( this, CurveSearchRadius, "Curve Search Radius", nullptr, SetCurveRadius );
	FBPropertyPublish( this, CurveDistanceAhead, "Curve Distance Ahead", nullptr, SetCurveDistance );
	FBPropertyPublish( this, CurvePathWidth, "Curve Path Width", nullptr, SetCurveWidth );
	FBPropertyPublish( this, CurveTangenMult, "Curve Tangent Mult", nullptr, SetCurveTangentMult );
	FBPropertyPublish( this, CurveDamping, "Curve Damping", nullptr, SetCurveDamping );

	//
	FBPropertyPublish( this, ChassisCollision, "ChassisCollision", nullptr, nullptr );

	FBPropertyPublish( this, ChassisFrontAxis, "Local Front Axis", nullptr, nullptr );
	FBPropertyPublish( this, ChassisUpAxis, "Local Up Axis", nullptr, nullptr );
	FBPropertyPublish( this, ChassisRightAxis, "Local Right Axis", nullptr, nullptr );
	
	//
	GearBoxCount.SetMinMax(0.0, 10.0, true, true);

	UseCurveForSteering = false;
	CurveSteeringWeight = 100.0;
	CurveSteeringWeight.SetMinMax(0.0, 100.0, true, true);
	CurveSteeringWeight.SetAnimated(true);
	Curve.SetSingleConnect(true);
	Curve.SetFilter( FBModelPath3D::GetInternalClassId() );
	CurveSamples = 20;

	CurveSearchRadius = 5.0;
	CurveDistanceAhead = 10.0;
	CurvePathWidth = 2.0;
	CurveTangenMult = 1.0;
	CurveDamping = 0.25;

	ChassisCollision.SetSingleConnect(true);
	ChassisCollision.SetFilter( FBModel::GetInternalClassId() );

	ChassisFrontAxis = kAxisPositiveX;
	ChassisUpAxis = kAxisPositiveY;
	ChassisRightAxis = kAxisPositiveZ;

	WheelHasAFender = true;
	TireSuspensionType = kTireSuspensionComfort;

	DownforceWeightFactor0 = 1.0;
	DownforceWeightFactor1 = 2.0;
	DownforceWeightFactorSpeed = 80.0;

	//

	const int len = 8;
	std::array<const char *, len> names = { "Wheel FL", "Wheel FR", "Wheel RL", "Wheel RR", "Wheel Collision FL", "Wheel Collision FR", "Wheel Collision RL", "Wheel Collision RR" };
	std::array<FBPropertyListObject*, len> components = { &WheelFL, &WheelFR, &WheelRL, &WheelRR, &WheelCollisionFL, &WheelCollisionFR, &WheelCollisionRL, &WheelCollisionRR };
	
	for (int i=0; i<len; ++i)
	{
		FBPropertyListObject &comp = *components[i];
		FBPropertyPublish( this, comp, names[i], nullptr, nullptr );
			
		comp.SetSingleConnect(true);
		comp.SetFilter( FBModel::GetInternalClassId() );
	}

	//
	//
	//

	Presets = kCarPresetSedan;
	DoSetPropertiesFromPreset(kCarPresetSedan);

	return true;
}

void MOCarPhysProperties::FBDestroy()
{

	// disconnect from the physics solver

	FBScene *pScene = FBSystem::TheOne().Scene;

	for (int i=0, count=pScene->ConstraintSolvers.GetCount(); i<count; ++i)
	{
		if (FBIS( pScene->ConstraintSolvers[i], MOPhysicsSolver ) )
		{
			MOPhysicsSolver *pSolver = (MOPhysicsSolver*) pScene->ConstraintSolvers[i];
			pSolver->DisconnectACar(this);
		}
	}

}

void MOCarPhysProperties::BeginMassiveChanges()
{
	mMassiveChanges = true;
}

void MOCarPhysProperties::EndMassiveChanges()
{
	mMassiveChanges = false;
}

bool MOCarPhysProperties::IsMassiveChanges()
{
	return mMassiveChanges;
}

bool MOCarPhysProperties::IsAssigned()
{
	if (GetDstCount() != 2)
		return false;

	FBPlug *dstPlug = GetDst(1);
	if ( FBIS(dstPlug, FBModelNull) && ChassisCollision.GetCount() == 0)
		return false;

	if (WheelFL.GetCount() == 0 || WheelFR.GetCount() == 0 || WheelRL.GetCount() == 0 || WheelRR.GetCount() == 0)
		return false;

	if (FBIS(WheelFL.GetAt(0), FBModelNull) && WheelCollisionFL.GetCount() == 0)
		return false;
	if (FBIS(WheelFR.GetAt(0), FBModelNull) && WheelCollisionFR.GetCount() == 0)
		return false;
	if (FBIS(WheelRL.GetAt(0), FBModelNull) && WheelCollisionRL.GetCount() == 0)
		return false;
	if (FBIS(WheelRR.GetAt(0), FBModelNull) && WheelCollisionRR.GetCount() == 0)
		return false;

	return true;
}

FBModel *MOCarPhysProperties::GetChassisObject()
{
	if (GetDstCount() != 2)
		return nullptr;

	return (FBModel*) GetDst(1);
}
FBModel *MOCarPhysProperties::GetChassisCollision()
{
	FBModel *pModel = GetChassisObject();
	if (pModel == nullptr)
		return nullptr;

	if (ChassisCollision.GetCount() > 0 && FBIS(ChassisCollision.GetAt(0), FBModel) )
		pModel = (FBModel*) ChassisCollision.GetAt(0);
	
	FBGeometry *pGeometry = pModel->Geometry;
	if (pGeometry == nullptr || pGeometry->VertexCount() == 0)
		return nullptr;

	return pModel;
}

FBModel *MOCarPhysProperties::GetWheelObject(const int index)
{
	if (WheelFL.GetCount() == 0 || WheelFR.GetCount() == 0 || WheelRL.GetCount() == 0 || WheelRR.GetCount() == 0)
		return nullptr;

	switch(index)
	{
	case 0:
		return (FBModel*) WheelFL.GetAt(0);
	case 1:
		return (FBModel*) WheelFR.GetAt(0);
	case 2:
		return (FBModel*) WheelRL.GetAt(0);
	case 3:
		return (FBModel*) WheelRR.GetAt(0);
	}

	return nullptr;
}
FBModel *MOCarPhysProperties::GetWheelCollision(const int index)
{
	FBModel *pWheelModel = GetWheelObject(index);
	if (pWheelModel == nullptr)
		return nullptr;

	switch(index)
	{
	case 0:
		if (WheelCollisionFL.GetCount() > 0)
			pWheelModel = (FBModel*) WheelCollisionFL.GetAt(0);
		break;
	case 1:
		if (WheelCollisionFR.GetCount() > 0)
			pWheelModel = (FBModel*) WheelCollisionFR.GetAt(0);
		break;
	case 2:
		if (WheelCollisionRL.GetCount() > 0)
			pWheelModel = (FBModel*) WheelCollisionRL.GetAt(0);
		break;
	case 3:
		if (WheelCollisionRR.GetCount() > 0)
			pWheelModel = (FBModel*) WheelCollisionRR.GetAt(0);
		break;
	}

	FBGeometry *pGeometry = pWheelModel->Geometry;
	if (pGeometry == nullptr || pGeometry->VertexCount() == 0)
		return nullptr;

	return pWheelModel;
}


void MOCarPhysProperties::DoSetPropertiesFromPreset(const CarPreset preset)
{
	switch(preset)
	{
	case kCarPresetViper:

		DriveType = PHYSICS_INTERFACE::kCarDriveRearWheels;

		Mass = 1560.0;
		Friction = 4.5;
	
		TireMass = 30.0;

		VehicleWeightDistribution = 55.0;		// 0.55
		ClutchFrictionTorque = 2000.0;

		EngineMass = 100.0;
		EngineRadio =  12.5; // 0.125;

		SuspensionLength = 20.0;	// 0.2
		SuspensionDamper = 1000.0;	// 10.0
		SuspensionSpring = 30000.0;	// 300.0
	
		LateralStiffness = 76726.0;	// (3380.0f * 0.454f * DEMO_GRAVITY * 10.0f) TIRE_LATERAL_STIFFNESS
		LongitudinalStiffness = 14164.8; // (3380.0f * 0.454f * DEMO_GRAVITY *  2.0f),	// TIRE_LONGITUDINAL_STIFFNESS
		AligningMomentTrail = 50.0;

		IdleTorque = 350.0;
		IdleTorqueRPM = 800.0;

		PeakTorque = 500.0;
		PeakTorqueRPM = 3000;

		PeakHorsePower = 400.0;
		PeakHorsePowerRPM = 5200.0;

		RedlineTorque = 30.0;
		RedlineTorqueRPM = 6000.0;

		MassYOffset = 0.0;	// * 0.01
		MassXOffset = 0.0;
	
		FrontTireSteerAngle = 20.0;
		RearTireSteerAngle = 0.0;

		GearBoxCount = 6;

		TireGear1 = 2.66;
		TireGear2 = 1.78;
		TireGear3 = 1.30;
		TireGear4 = 1.00;
		TireGear5 = 0.74;
		TireGear6 = 0.50;
		TireGearReverse = 2.90;

		TopSpeed = 264.0;

		TireBrakeTorque = 3000.0;
		TireHandBrakeTorque = 500.0;

		DownforceWeightFactor0 = 1.0;
		DownforceWeightFactor1 = 2.0;
		DownforceWeightFactorSpeed = 80.0;

		WheelHasAFender = true;
		TireSuspensionType = kTireSuspensionComfort;
		break;
	case kCarPresetSedan:
		DriveType = PHYSICS_INTERFACE::kCarDriveFrontWheels;

		Mass = 1560.0;
		Friction = 4.5;
	
		TireMass = 30.0;

		VehicleWeightDistribution = 55.0;		// 0.55
		ClutchFrictionTorque = 2000.0;

		EngineMass = 100.0;
		EngineRadio =  12.5; // 0.125;

		SuspensionLength = 20.0;	// 0.2
		SuspensionDamper = 1000.0;	// 10.0
		SuspensionSpring = 30000.0;	// 300.0
	
		LateralStiffness = 76726.0;	// (3380.0f * 0.454f * DEMO_GRAVITY * 10.0f) TIRE_LATERAL_STIFFNESS
		LongitudinalStiffness = 14164.8; // (3380.0f * 0.454f * DEMO_GRAVITY *  2.0f),	// TIRE_LONGITUDINAL_STIFFNESS
		AligningMomentTrail = 50.0;

		IdleTorque = 350.0;
		IdleTorqueRPM = 800.0;

		PeakTorque = 500.0;
		PeakTorqueRPM = 3000;

		PeakHorsePower = 400.0;
		PeakHorsePowerRPM = 5200.0;

		RedlineTorque = 30.0;
		RedlineTorqueRPM = 6000.0;

		MassYOffset = 0.0;	// * 0.01
		MassXOffset = 0.0;
	
		FrontTireSteerAngle = 20.0;
		RearTireSteerAngle = 0.0;

		GearBoxCount = 6;

		TireGear1 = 2.66;
		TireGear2 = 1.78;
		TireGear3 = 1.30;
		TireGear4 = 1.00;
		TireGear5 = 0.74;
		TireGear6 = 0.50;
		TireGearReverse = 2.90;

		TopSpeed = 264.0;

		TireBrakeTorque = 3000.0;
		TireHandBrakeTorque = 500.0;

		DownforceWeightFactor0 = 1.0;
		DownforceWeightFactor1 = 2.0;
		DownforceWeightFactorSpeed = 80.0;

		WheelHasAFender = true;
		TireSuspensionType = kTireSuspensionComfort;
		break;
	case kCarPresetJeep:
		DriveType = PHYSICS_INTERFACE::kCarDrive4x4;

		Mass = 1560.0;
		Friction = 4.5;
	
		TireMass = 30.0;

		VehicleWeightDistribution = 55.0;		// 0.55
		ClutchFrictionTorque = 2000.0;

		EngineMass = 100.0;
		EngineRadio =  12.5; // 0.125;

		SuspensionLength = 20.0;	// 0.2
		SuspensionDamper = 1000.0;	// 10.0
		SuspensionSpring = 30000.0;	// 300.0
	
		LateralStiffness = 76726.0;	// (3380.0f * 0.454f * DEMO_GRAVITY * 10.0f) TIRE_LATERAL_STIFFNESS
		LongitudinalStiffness = 14164.8; // (3380.0f * 0.454f * DEMO_GRAVITY *  2.0f),	// TIRE_LONGITUDINAL_STIFFNESS
		AligningMomentTrail = 50.0;

		IdleTorque = 350.0;
		IdleTorqueRPM = 800.0;

		PeakTorque = 500.0;
		PeakTorqueRPM = 3000;

		PeakHorsePower = 400.0;
		PeakHorsePowerRPM = 5200.0;

		RedlineTorque = 30.0;
		RedlineTorqueRPM = 6000.0;

		MassYOffset = 0.0;	// * 0.01
		MassXOffset = 0.0;
	
		FrontTireSteerAngle = 20.0;
		RearTireSteerAngle = 0.0;

		GearBoxCount = 6;

		TireGear1 = 2.66;
		TireGear2 = 1.78;
		TireGear3 = 1.30;
		TireGear4 = 1.00;
		TireGear5 = 0.74;
		TireGear6 = 0.50;
		TireGearReverse = 2.90;

		TopSpeed = 264.0;

		TireBrakeTorque = 3000.0;
		TireHandBrakeTorque = 500.0;

		DownforceWeightFactor0 = 1.0;
		DownforceWeightFactor1 = 2.0;
		DownforceWeightFactorSpeed = 80.0;

		WheelHasAFender = true;
		TireSuspensionType = kTireSuspensionComfort;
		break;
	}
}

void MOCarPhysProperties::AxisToVector( const EAxis &axis, float *vector )
{
	vector[0] = 0.0f;
	vector[1] = 0.0f;
	vector[2] = 0.0f;

	switch(axis)
	{
	case kAxisPositiveX:
		vector[0] = 1.0f;
		break;
	case kAxisNegativeX:
		vector[0] = -1.0f;
		break;
	case kAxisPositiveY:
		vector[1] = 1.0f;
		break;
	case kAxisNegativeY:
		vector[1] = -1.0f;
		break;
	case kAxisPositiveZ:
		vector[2] = 1.0f;
		break;
	case kAxisNegativeZ:
		vector[2] = -1.0f;
		break;
	}
}

void MOCarPhysProperties::FillCarInfo( PHYSICS_INTERFACE::CarOptions &info )
{
	info.type = DriveType;

	//AxisToVector( ChassisFrontAxis, info.localFrontAxis );
	//AxisToVector( ChassisUpAxis, info.localUpAxis );
	//AxisToVector( ChassisRightAxis, info.localRightAxis );

	info.localFrontAxis = (int) (EAxis) ChassisFrontAxis;
	info.localUpAxis = (int) (EAxis) ChassisUpAxis;
	info.localRightAxis = (int) (EAxis) ChassisRightAxis;

	info.MassYOffset = MassYOffset;
	info.MassXOffset = MassXOffset;
	info.Mass = Mass;
	
	info.EngineMass = EngineMass;
	info.EngineRadio = 0.01 * EngineRadio;

	info.WeightDistribution = 0.01 * VehicleWeightDistribution;
	info.ClutchFrictionTorque = ClutchFrictionTorque;

	for (int i=0; i<4; ++i)
	{
		PHYSICS_INTERFACE::CarOptions::Wheel &wheel = info.wheels[i];

		wheel.Friction = Friction;
		wheel.Mass = TireMass;
	
		wheel.SuspensionDamper = 0.01 * SuspensionDamper;
		wheel.SuspensionLength = 0.01 * SuspensionLength;
		wheel.SuspensionSpring = 0.01 * SuspensionSpring;

		double lateral, longitudinal, aligning;
		lateral = LateralStiffness;
		longitudinal = LongitudinalStiffness;
		aligning = 0.01 * AligningMomentTrail;

		wheel.lateralStiffness = (float) lateral;
		wheel.longitudinalStiffness = (float) longitudinal;
		wheel.aligningMomentTrail = (float) aligning;
	}

	int gearCount = GearBoxCount;
	if (gearCount < 0 || gearCount > 6) gearCount = 6;

	info.differentialLock = (DifferentialLock) ? 1 : 0;
	info.AutomaticGear = AutomaticGear;
	info.GearCount = gearCount;
	info.GearReverseRatio = TireGearReverse;

	info.gear1 = TireGear1;
	info.gear2 = TireGear2;
	info.gear3 = TireGear3;
	info.gear4 = TireGear4;
	info.gear5 = TireGear5;
	info.gear6 = TireGear6;
	info.reverseGear = 2.9f;

	info.IdleRPM = IdleTorqueRPM;
	info.IdleTorquePoundPerFoot = IdleTorque;

	info.PeakHorsePower = PeakHorsePower;
	info.PeakHorsePowerRPM = PeakHorsePowerRPM;

	info.PeakTorquePoundPerFoot = PeakTorque;
	info.PeakTorqueRPM = PeakTorqueRPM;

	info.RedLineRPM = RedlineTorqueRPM;
	info.RedLineTorquePoundPerFoot = RedlineTorque;

	info.FrontSteerAngle = FrontTireSteerAngle;
	info.RearSteerAngle = RearTireSteerAngle;
	info.UseCurveForSteering = (UseCurveForSteering == true && Curve.GetCount() > 0);
	info.CurveSteeringWeight = 0.01 * CurveSteeringWeight;

	info.steerCurveParams.searchRadius = CurveSearchRadius;
	info.steerCurveParams.distanceAhead = CurveDistanceAhead;
	info.steerCurveParams.pathWidth = CurvePathWidth;
	info.steerCurveParams.tangentMult = CurveTangenMult;
	info.steerCurveParams.damping = CurveDamping;

	info.vehicleTopSpeedKPH = TopSpeed;

	info.TireBrakeTorque = TireBrakeTorque;
	info.TireHandBrakeTorque = TireHandBrakeTorque;

	info.WheelhasFender = (WheelHasAFender) ? 1 : 0;
	info.TireSuspensionType = TireSuspensionType.AsInt();

	info.DownforceWeightFactor0 = DownforceWeightFactor0;
	info.DownforceWeightFactor1 = DownforceWeightFactor1;
	info.DownforceWeightFactorSpeed = DownforceWeightFactorSpeed;
}

void MOCarPhysProperties::StartOnline( PHYSICS_INTERFACE::IWorld *pworld, PHYSICS_INTERFACE::ICar *pcar )
{
	Active.SetEnable(false);
	Curve.SetEnable(false);
	ChassisCollision.SetEnable(false);
	WheelFL.SetEnable(false);
	WheelFR.SetEnable(false);
	WheelRL.SetEnable(false);
	WheelRR.SetEnable(false);
	WheelCollisionFL.SetEnable(false);
	WheelCollisionFR.SetEnable(false);
	WheelCollisionRL.SetEnable(false);
	WheelCollisionRR.SetEnable(false);

	//
	mOnline = true;
	mWorld = pworld;
	mCar = pcar;
}

void MOCarPhysProperties::StopOnline()
{
	Active.SetEnable(true);
	Curve.SetEnable(true);
	ChassisCollision.SetEnable(true);
	WheelFL.SetEnable(true);
	WheelFR.SetEnable(true);
	WheelRL.SetEnable(true);
	WheelRR.SetEnable(true);
	WheelCollisionFL.SetEnable(true);
	WheelCollisionFR.SetEnable(true);
	WheelCollisionRL.SetEnable(true);
	WheelCollisionRR.SetEnable(true);

	//
	mOnline = false;
	mWorld = nullptr;
	mCar = nullptr;
}

void MOCarPhysProperties::DoUpdateProperties()
{
	if (mOnline == false || mWorld == nullptr || mCar == nullptr || mMassiveChanges == true)
		return;

	// DONE: update properties for a car model
	mWorld->WaitForUpdateToFinish();

	// ...
	PHYSICS_INTERFACE::CarOptions	options;
	FillCarInfo(options);
	mCar->SetOptions(&options);
}

FBModelPath3D *MOCarPhysProperties::GetSteeringPath()
{
	if (Curve.GetCount() > 0)
		return (FBModelPath3D*) Curve[0];

	return nullptr;
}

void AddPropertyViewForCarPhysics(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
#ifndef ORSDK2013	
	FBPropertyViewManager::TheOne().AddPropertyView(MOCARPHYSPROPERTIES__CLASSSTR, pPropertyName, pHierarchy);
#endif
}

void MOCarPhysProperties::AddPropertiesToPropertyViewManager()
{
	
	AddPropertyViewForCarPhysics("Active", "");
	AddPropertyViewForCarPhysics("Presets", "");

	//
	AddPropertyViewForCarPhysics("Info", "", true);
	AddPropertyViewForCarPhysics("Speed", "Info");
	AddPropertyViewForCarPhysics("RPM", "Info");
	AddPropertyViewForCarPhysics("Current Gear", "Info");

	//
	AddPropertyViewForCarPhysics("Common", "", true);
	AddPropertyViewForCarPhysics("Drive Type", "Common");
	AddPropertyViewForCarPhysics("Mass", "Common");
	AddPropertyViewForCarPhysics("Top Speed", "Common");
	AddPropertyViewForCarPhysics("Friction", "Common");

	AddPropertyViewForCarPhysics("Vehicle Weight Distribution", "Common");
	AddPropertyViewForCarPhysics("Clutch Friction Torque", "Common");

	AddPropertyViewForCarPhysics("Mass X Offset", "Common");
	AddPropertyViewForCarPhysics("Mass Y Offset", "Common");

	AddPropertyViewForCarPhysics("Downforce Weight Factor0", "Common");
	AddPropertyViewForCarPhysics("Downforce Weight Factor1", "Common");
	AddPropertyViewForCarPhysics("Downforce Weight Factor Speed", "Common");

	AddPropertyViewForCarPhysics("Common.Tire", "Common", true);
	AddPropertyViewForCarPhysics("Wheel Has A Fender", "Common.Tire");
	AddPropertyViewForCarPhysics("Tire Suspension Type", "Common.Tire");
	AddPropertyViewForCarPhysics("Tire Mass", "Common.Tire");
	AddPropertyViewForCarPhysics("Front Tire Steer Angle", "Common.Tire");
	AddPropertyViewForCarPhysics("Rear Tire Steer Angle", "Common.Tire");
	AddPropertyViewForCarPhysics("Tire Brake Torque", "Common.Tire");
	AddPropertyViewForCarPhysics("Tire HandBrake Torque", "Common.Tire");

	AddPropertyViewForCarPhysics("Suspension Length", "Common.Tire");
	AddPropertyViewForCarPhysics("Suspension Spring", "Common.Tire");
	AddPropertyViewForCarPhysics("Suspension Damper", "Common.Tire");

	AddPropertyViewForCarPhysics("Lateral Stiffness", "Common.Tire");
	AddPropertyViewForCarPhysics("Longitudinal Stiffness", "Common.Tire");
	AddPropertyViewForCarPhysics("Aligning Moment Trail", "Common.Tire");

	AddPropertyViewForCarPhysics("Common.Engine", "Common", true);
	AddPropertyViewForCarPhysics("Engine Mass", "Common.Engine");
	AddPropertyViewForCarPhysics("Engine Radio", "Common.Engine");
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
	AddPropertyViewForCarPhysics("Differential Lock", "Controlling");
	AddPropertyViewForCarPhysics("Gear", "Controlling");
	
	AddPropertyViewForCarPhysics("Torque", "Controlling");
	AddPropertyViewForCarPhysics("Clutch", "Controlling");
	AddPropertyViewForCarPhysics("Steering", "Controlling");
	AddPropertyViewForCarPhysics("Curve Steering Weight", "Controlling");
	AddPropertyViewForCarPhysics("Brake", "Controlling");
	AddPropertyViewForCarPhysics("Hand Brake", "Controlling");
}
