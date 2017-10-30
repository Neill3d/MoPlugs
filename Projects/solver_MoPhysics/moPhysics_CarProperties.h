
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: moPhysics_CarProperties.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#include "Common_Physics\physics_common.h"
#include "queryFBGeometry.h"

#define	MOCARPHYSPROPERTIES__CLASSNAME		MOCarPhysProperties
#define MOCARPHYSPROPERTIES__CLASSSTR		"MOCarPhysProperties"

////////////////////////////////////////////////////////////////////////////////////////
//

enum CarPreset
{
	kCarPresetViper,
	kCarPresetSedan,
	kCarPresetJeep,
	kCarPresetCustom
};

enum EAxis
{
	kAxisPositiveX,
	kAxisNegativeX,
	kAxisPositiveY,
	kAxisNegativeY,
	kAxisPositiveZ,
	kAxisNegativeZ
};

enum ETireSuspensionType
{
	kTireSuspensionOffroad,
	kTireSuspensionComfort,
	kTireSuspensionRace
};

const char * FBPropertyBaseEnum<CarPreset>::mStrings[] = {
	"Dodge Viper",
	"Sedan",
	"Jeep",
	"Custom",
	0
};

const char * FBPropertyBaseEnum<PHYSICS_INTERFACE::CarDriveType>::mStrings[] = {
	"Front Wheels",
	"Rear Wheels",
	"4x4",
	0
};

const char * FBPropertyBaseEnum<EAxis>::mStrings[] = {
	"Positive X",
	"Negative X",
	"Positive Y",
	"Negative Y",
	"Positive Z",
	"Negative Z",
	0
};

const char * FBPropertyBaseEnum<ETireSuspensionType>::mStrings[] = {
	"Offroad",
	"Comfort",
	"Race",
	0
};

///////////////////////////////////////////////////////////////////////////////////////////
//
class MOCarPhysProperties : public FBPhysicalProperties
{
	FBPhysicalPropertiesDeclare( MOCarPhysProperties, FBPhysicalProperties );

public:
	
	//--- Creation & Destruction
	virtual bool			FBCreate() override;						//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	
	void					DoSetPropertiesFromPreset(const CarPreset preset);
	void					FillCarInfo( PHYSICS_INTERFACE::CarOptions &options );
	bool					IsAssigned();

	FBModel					*GetChassisObject();
	FBModel					*GetChassisCollision();

	FBModel					*GetWheelObject(const int index);
	FBModel					*GetWheelCollision(const int index);

	FBModelPath3D			*GetSteeringPath();

	// change parameters during a live mode
	void					StartOnline( PHYSICS_INTERFACE::IWorld *pworld, PHYSICS_INTERFACE::ICar *pcar );
	void					StopOnline();
	// callback for changes property values
	void					DoUpdateProperties();

	void					BeginMassiveChanges();
	void					EndMassiveChanges();
	bool					IsMassiveChanges();

	static void				AddPropertiesToPropertyViewManager();

	static void				SetCarPreset( HIObject pObject, CarPreset preset );
	static void				SetDriveType( HIObject pObject, PHYSICS_INTERFACE::CarDriveType value );

	static void				SetMass( HIObject pObject, double value );
	static void				SetTopSpeed( HIObject pObject, double value );
	static void				SetFriction( HIObject pObject, double value );

	static void				SetVehicleWeightDistribution( HIObject pObject, double value );
	static void				SetClutchFrictionTorque( HIObject pObject, double value );

	static void				SetEngineMass( HIObject pObject, double value );
	static void				SetEngineRadio( HIObject pObject, double value );

	static void				SetTireMass( HIObject pObject, double value );
	static void				SetSuspensionLength( HIObject pObject, double value );
	static void				SetSuspensionSpring( HIObject pObject, double value );
	static void				SetSuspensionDamper( HIObject pObject, double value );

	static void				SetLateralStiffness( HIObject pObject, double value );
	static void				SetLongitudinalStiffness( HIObject pObject, double value );
	static void				SetAligningMomentTrail( HIObject pObject, double value );

	static void				SetAutomaticGear( HIObject pObject, bool value );
	static void				SetDifferentialLock( HIObject pObject, bool value );

	static void				SetMassOffsetX( HIObject pObject, double value );
	static void				SetMassOffsetY( HIObject pObject, double value );

	static void				SetFrontTireSteerAngle( HIObject pObject, double value );
	static void				SetRearTireSteerAngle( HIObject pObject, double value );
	static void				SetTireBrakeTorque( HIObject pObject, double value );
	static void				SetTireHandBrakeTorque( HIObject pObject, double value );

	static void				SetDownforceWeightFactor0( HIObject pObject, double value );
	static void				SetDownforceWeightFactor1( HIObject pObject, double value );
	static void				SetDownforceWeightFactorSpeed( HIObject pObject, double value );

	static void				SetWheelHasAFender( HIObject pObject, bool value );
	static void				SetTireSuspensionType( HIObject pObject, ETireSuspensionType value );

	static void				SetUseCurve( HIObject pObject, bool value );

	static void				SetCurveRadius( HIObject pObject, double value );
	static void				SetCurveDistance( HIObject pObject, double value );
	static void				SetCurveWidth( HIObject pObject, double value );
	static void				SetCurveTangentMult( HIObject pObject, double value );
	static void				SetCurveDamping( HIObject pObject, double value );

public:

	FBPropertyBool			Active;

	FBPropertyBaseEnum<CarPreset>								Presets;
	FBPropertyBaseEnum<PHYSICS_INTERFACE::CarDriveType>			DriveType;

	FBPropertyDouble		Speed;		// read-only: display current car speed value
	FBPropertyDouble		RPM;		// read-only: display engine RPM value	
	FBPropertyInt			DisplayGear;	// read-only: display engine gear (to see what engine has choosen from automatic gearbox)

	FBPropertyDouble		Mass;		// car chassis mass
	FBPropertyDouble		TopSpeed;	// in km/hours
	FBPropertyDouble		Friction;	// car friction

	FBPropertyDouble		VehicleWeightDistribution;
	FBPropertyDouble		ClutchFrictionTorque;

	FBPropertyDouble		EngineMass;
	FBPropertyDouble		EngineRadio;

	FBPropertyDouble		TireMass;

	FBPropertyDouble		MassYOffset;		// y offset
	FBPropertyDouble		MassXOffset;		// x offset

	FBPropertyDouble		SuspensionLength;
	FBPropertyDouble		SuspensionSpring;
	FBPropertyDouble		SuspensionDamper;

	FBPropertyDouble		LateralStiffness;
	FBPropertyDouble		LongitudinalStiffness;
	FBPropertyDouble		AligningMomentTrail;

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

	FBPropertyDouble		DownforceWeightFactor0;
	FBPropertyDouble		DownforceWeightFactor1;
	FBPropertyDouble		DownforceWeightFactorSpeed;

	FBPropertyDouble		FrontTireSteerAngle;		// maximum angle for tire steering
	FBPropertyDouble		RearTireSteerAngle;		// maximum angle for tire steering

	FBPropertyBool			WheelHasAFender;
	FBPropertyBaseEnum<ETireSuspensionType>			TireSuspensionType;

	//
	// CAR CONTROLLING PROPERTIES
	//
	FBPropertyAnimatableDouble	Brake;		// for all 4 wheels 0 - 1.0 for applying a brake
	FBPropertyAnimatableDouble	HandBrake;	// only for rear wheels
	FBPropertyAnimatableDouble	Torque;		// +15 - move forward, -15 - move back
	FBPropertyAnimatableDouble	Clutch;
	FBPropertyAnimatableDouble	Steering;	// 1.0 - turn left, -1.0 - turn right
	//FBPropertyAnimatableDouble	SteeringDirect;	// - direct controlling of a steer angle

	FBPropertyAnimatableInt		Gear;		// read-write: display current gear or set a current gear (for manual gearbox)
	FBPropertyBool				AutomaticGear;
	FBPropertyBool				DifferentialLock;

	// AI - curve based steering control
	FBPropertyBool				UseCurveForSteering;
	FBPropertyAnimatableDouble	CurveSteeringWeight;		// blend between manual steering and curve controlled steering
	FBPropertyListObject		Curve;
	FBPropertyInt				CurveSamples;	// number of points for curve evaluation

	FBPropertyDouble			CurveSearchRadius;
	FBPropertyDouble			CurveDistanceAhead;
	FBPropertyDouble			CurvePathWidth;
	FBPropertyDouble			CurveTangenMult;
	FBPropertyDouble			CurveDamping;

	// collision geometry (if tm object and collision are not the same)
	FBPropertyListObject		ChassisCollision;

	FBPropertyBaseEnum<EAxis>	ChassisFrontAxis;
	FBPropertyBaseEnum<EAxis>	ChassisUpAxis;
	FBPropertyBaseEnum<EAxis>	ChassisRightAxis;

	// wheels transforms

	FBPropertyListObject		WheelFL;
	FBPropertyListObject		WheelFR;
	FBPropertyListObject		WheelRL;
	FBPropertyListObject		WheelRR;

	// wheel collision (if tm and collision are not the same)
	FBPropertyListObject		WheelCollisionFL;
	FBPropertyListObject		WheelCollisionFR;
	FBPropertyListObject		WheelCollisionRL;
	FBPropertyListObject		WheelCollisionRR;

private:

	bool							mMassiveChanges;

	bool							mOnline;
	PHYSICS_INTERFACE::IWorld		*mWorld;
	PHYSICS_INTERFACE::ICar			*mCar;

	void AxisToVector( const EAxis &axis, float *vector );
};