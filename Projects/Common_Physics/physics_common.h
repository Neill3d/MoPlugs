
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: physics_common.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


namespace PHYSICS_INTERFACE
{

// forward declaration

class IBody;
class ICar;
class IWorld;
class IQueryGeometry;

///////////////////////////////////////////////////////////////////
//

// callback classes for query chassis and wheels
class IQueryGeometry
{
public:

	// supports only 3,4 verts per poly
	struct Poly
	{
		int		matId;	// physical material (friction, etc.)
		int		count;
		int		indices[4];
	};

	virtual void GetBoundingBox(float *min, float *max) const {}
	virtual void GetBoundingBoxD(double *min, double *max) const {}

	virtual void PrepMatrix(bool global) const {}

	virtual const double	*GetMatrix(bool global) const { return nullptr;}
	virtual void			GetMatrixTR(bool global, double *values) const {}
	virtual void			GetMatrixTR_f(bool global, float *values) const {}

	virtual void			GetVisualAlignMatrix(float *values) const {}
	virtual void			GetVisualAlignMatrixD(double *values) const {}

	virtual const double	*GetPosition(bool global) const { return nullptr; }
	virtual void			GetPositionD(double *values) const {}
	virtual void			GetPositionF(float *values) const {}

	virtual const double	*GetRotation(bool global) const { return nullptr; }
	virtual void			GetRotationD(double *values) const {}
	virtual void			GetRotationF(float *values) const {}

	virtual const double	*GetQuaternion(bool global) const { return nullptr; }
	virtual void			GetQuaternionD(double *values) const {}
	virtual void			GetQuaternionF(float *values) const {}

	//virtual const double	*GetCollisionScale(bool global) const { return nullptr; }
	virtual const double	*GetScale(bool global) const { return nullptr; }
	virtual void			GetScaleD(double *values) const {}
	virtual void			GetScaleF(float *values) const {}

	//

	virtual const int	GetVertexCount() const { return 0; }
	virtual const float *GetVertexPosition(const int index) const { return nullptr; }

	virtual const int	GetPolyCount() const { return 0; }
	virtual const Poly	*GetPoly(const int index) const { return nullptr; }

	// function to record a new keyframe
	/*
	virtual void RecordTransform( const double time, const float *matrix) 
	{}
	*/
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
//
class IQueryCarProxy
{

public:

	virtual	void SetChassisTransform(const double time, const float *matrix) {}
	virtual void SetWheelTransform(const double time, const int wheelIndex, const float *matrix) {}

};

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
class IQueryPath
{
public:
	//
	virtual void		GetCurrentMatrix(float *values) const
	{}
	virtual void		GetCurrentMatrixD(double *values) const
	{}

	virtual void		CurveDerivative (double *result, double u, int index = 1) const 
	{}

	virtual double FindClosestKnot (double *closestPointOnCurve, const double *point, double dist_thres, int subdivitionSteps = 2) const
	{
		return 0.0;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// information for making a new physics car or for updating car properties for simulation

enum CarDriveType
{
	kCarDriveFrontWheels,
	kCarDriveRearWheels,
	kCarDrive4x4
};

struct CarInput
{
	float			steeringVal;
	float			curveSteeringBlendVal;
	float			engineGasPedal;
	float			engineClutchPedal;
	float			brakePedal;
	float			handBrakePedal;

	bool			transmissionMode;
	int				gear;
};


///////////////////////////////////////////////////////////////////
// Structure for creating a new car
//

struct CarOptions
{
	struct Wheel {
		// user data - input 3d model
		//void							*pModel;	// pointer to FBModel*


		// physics parameters for simulation
		double							Mass;
		double							Friction;
		double							SuspensionLength;		// 0.20f
		double							SuspensionSpring;		// 15000.0f
		double							SuspensionDamper;		// 600.0f

		float							lateralStiffness;		// 20.0f
		float							longitudinalStiffness;	// 10000.0f
		float							aligningMomentTrail;	// 0.5f

	} wheels[4];
	
	//void			*pChassis;	// pointer to FBModel*

	CarDriveType	type;	// 4x4, front or rear wheels

	int		localFrontAxis;
	int		localUpAxis;
	int		localRightAxis;

	int		differentialLock;

	float		gear1;			// 2.66f
	float		gear2;			// 1.78f
	float		gear3;			// 1.30f
	float		gear4;			// 1.00f
	float		gear5;			// 0.74f
	float		gear6;			// 0.50f
	float		reverseGear;	// 2.90f

	// gearbox
	bool		AutomaticGear;
	float		GearReverseRatio;
	int			GearCount;

	// common car parameters

	double		Mass;
	double		MassXOffset;
	double		MassYOffset;
	double		EngineMass;			// 100.0
	double		EngineRadio;		// 0.125

	double		WeightDistribution;	// 0.55
	double		ClutchFrictionTorque;	// 2000.0
	
	double		FrontSteerAngle;		// steering angle limit (35 by default)
	double		RearSteerAngle;
	bool		UseCurveForSteering;
	double		CurveSteeringWeight;	// blend between manual and auto curve steering

	struct	SteerCurveParams
	{
		double	searchRadius;
		double	distanceAhead;
		double	pathWidth;
		double	tangentMult;
		double	damping;

	} steerCurveParams;

	//
	double		IdleRPM;
	double		IdleTorquePoundPerFoot;

	double		PeakTorqueRPM;
	double		PeakTorquePoundPerFoot;

	double		PeakHorsePowerRPM;
	double		PeakHorsePower;
		
	double		RedLineRPM;
	double		RedLineTorquePoundPerFoot;

	double		vehicleTopSpeedKPH;

	double		TireHandBrakeTorque;
	double		TireBrakeTorque;	// 5000.0f

	int			WheelhasFender;
	int			TireSuspensionType;	// (0-m_offroad, 1-m_confort, 2-m_race)

	double		DownforceWeightFactor0;
	double		DownforceWeightFactor1;
	double		DownforceWeightFactorSpeed;
};

////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS

// create instances only with this function
//World		*CreateNewPhysicsHardware( const EPhysicsEngine type, const double scale );
//Car			*CreateNewPhysicsCar( PhysicsHardware *hardware, const CarInfo *info, void *userdata=nullptr );


//////////////////////////////////////////////////////////////////////////////////////
// common class for physics car simulation and interation with user input

struct BodyOptions
{
	double		mass;
	double		friction;
};

class IBody
{
public:
	//! a constructor
	IBody( IWorld *world )
	{}

	virtual ~IBody()
	{}

	virtual void	SaveState() = 0;
	virtual void	RestoreState() = 0;

	virtual bool	Snap() = 0;

	virtual const double *GetMatrix(IWorld *world) = 0;

public:

	void *UserData1;
	void *UserData2;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// base interface
class ICar
{
public:
	// ! a constructor - attach car to a physics world
	//		we need 5 classes to query chassis and 4 wheels matrix and geometry for a car startup
	ICar( IWorld *world )
		: mWorld(world)
	{}

	//! a destructor
	virtual ~ICar()
	{}

	// controlling a car by user
	virtual void	SetPlayerControl(const CarInput &control) = 0;
	virtual void	SetPlayerControl(const double torque, const double clutch, const double steering, const double curveSteeringBlend, const double brake, const double handbrake, int gear) = 0;
	virtual void	SetOptions( const CarOptions *options ) = 0;

	virtual void SetDriveType(const CarDriveType type) = 0;
	
	// use current simulation state as initial (dynamic matrix pose to start matrix)
	virtual	void	SaveState() = 0;
	virtual void	RestoreState() = 0;

	virtual void	Snap() = 0;

	//

	virtual const double *GetChassisMatrix() = 0;
	virtual const double *GetWheelMatrix(const int wheel, const bool local) = 0;

	virtual const double	GetSpeed() = 0;
	virtual const double	GetRPM() = 0;
	virtual const int		GetCurrentGear() = 0;

	virtual void DrawDebug() const = 0;

protected:

	CarDriveType			mDriveType;
	IWorld					*mWorld;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
// common class to operate with physics engine

class IWorld
{
public:

	//! a constructor
	IWorld()
	{}

	virtual ~IWorld()
	{}

	// layout engine information
	virtual const char *GetPhysicsInfo() = 0;

	//--- Opens and closes connection with data server. returns true if successful
	virtual bool	Open() = 0;								//!< Open the connection.
	virtual bool	Close() = 0;							//!< Close connection.

	virtual bool	Clear() = 0;

	//--- Hardware communication
	virtual void	Reset() = 0;
	virtual bool	FetchDataPacket	(const double animTimeSecs) = 0;	//!< Fetch a data packet from the computer.
	virtual double	GetCurrPhysTimeSecs() = 0;
	virtual double	GetLastPhysTimeSecs() = 0;
	virtual bool	PollData		() = 0;						//!< Poll the device for a data packet.
	virtual bool	GetSetupInfo	() = 0;						//!< Get the setup information.
	virtual bool	StartDataStream	() = 0;						//!< Put the device in streaming mode.
	virtual bool	StopDataStream	() = 0;						//!< Take the device out of streaming mode.


	// 3 double values
	virtual void	SetGravity(const double *gravity) = 0;
	virtual const double *GetGravity() const = 0;

	virtual void SetGlobalScale(const double scale) = 0;
	virtual const double	GetGlobalScale() const = 0;

	// set default material physical parameters
	virtual void SetDefaultSoftness(const double value) = 0;
	virtual const double GetDefaultSoftness() const = 0;
	
	virtual void SetDefaultElasticity(const double value) = 0;
	virtual const double GetDefaultElasticity() const = 0;
	
	virtual void SetDefaultCollidable(const double value) = 0;
	virtual const double GetDefaultCollidable() const = 0;
	
	virtual void SetDefaultFriction(const double staticValue, const double dynamicValue) = 0;
	virtual const double GetDefaultStaticFriction() const = 0;
	virtual const double GetDefaultDynamicFriction() const = 0;

	virtual void SetDefaultMaterialParams(double softness, double elasticity, double collidable, double staticFriction, double dynamicFriciton) = 0;

	// static collisions in the scene
	virtual void ClearLevel() = 0;
	virtual bool LoadLevel( const IQueryGeometry *levelInfo ) = 0;

	// utility function
	//
	// ray should be in the level local space
	//
	// return true if we have a ray hit (fill up pos and nor with ray hit position and normal)
	//
	virtual bool ClosestRayCast(const double *p0, const double *p1, double *pos, double *nor, double *dist, long long *attribute) = 0;

	// sync

	virtual void WaitForUpdateToFinish() = 0;

	// snapping current objects tm and restore from this state if needed
	virtual void SaveState() = 0;
	virtual void RestoreState() = 0;

	virtual void DrawDebug() const = 0;

	virtual void Serialize(const char *filename) = 0;

	//
	//

	virtual IBody	*CreateNewBody( const BodyOptions *info, const IQueryGeometry *pgeometry, bool convexhull ) = 0;
	// NOTE: we should give 5 geometry classes (chassis and 4 wheels)
	virtual ICar	*CreateNewCar( const CarOptions *info, const IQueryGeometry **pgeometry, const IQueryPath *curve ) = 0;
	
};

};

