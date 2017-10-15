#ifndef __HARDWARE_H__
#define __HARDWARE_H__

/**	\file	hardware_common.h
*	Declaration of a virtual hardware class.
*	Contains the declaration of the virtual input hardware class
*	ORDeviceBulletHardware.
*/

#include <list>

// forward declaration

class PhysicsCar;
class PhysicsHardware;

//
// GLOBAL FUNCTION TO CREATE AN ENGINE AND A CAR INSTANCES
//

enum EPhysicsHardwareEngine
{
	ePhysicsHardwareNewton,
	ePhysicsHardwareBullet
};

#define		PHYSICS_HARDWARE_NEWTON_TITLE			"Newton"
#define		PHYSICS_HARDWARE_BULLET_TITLE			"Bullet"

#define		PHYSICS_HARDWARE_NEWTON_VERSION			3.16
#define		PHYSICS_HARDWARE_BULLET_VERSION			2.82


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// information for making a new physics car or for updating car properties for simulation

enum CarDriveType
{
	kCarDriveFrontWheels,
	kCarDriveRearWheels,
	kCarDrive4x4
};

///////////////////////////////////////////////////////////////////
//

// supports only 3,4 verts per poly

struct VertInfo
{
	double	pos[4];
};

struct PolyInfo
{
	int		matId;	// physical material (friction, etc.)
	int		count;
	int		indices[4];
};

struct LevelInfo
{
	int			vertCount;
	VertInfo	*vertices;

	int			polyCount;
	PolyInfo	*polys;

	//! a constructor
	LevelInfo();

	void Allocate(const int vertsCount, const int polysCount)
	{
		vertCount = vertsCount;
		vertices = new VertInfo[vertsCount];

		polyCount = polysCount;
		polys = new PolyInfo[polysCount];
	}

	//! a destructor (free mem)
	~LevelInfo()
	{
		if (vertices)
		{
			delete [] vertices;
			vertices = nullptr;
		}
		if (polys)
		{
			delete [] polys;
			polys = nullptr;
		}
	}
};

struct CarInput
{
	double			steeringVal;
	double			engineGasPedal;
	double			brakePedal;
	double			handBrakePedal;

	bool			transmissionMode;
	int				gear;
};

///////////////////////////////////////////////////////////////////
// Structure for creating a new car
//
class CarInfo
{
public:

	//! a constructor
	CarInfo();
	//! a copy constructor
	CarInfo(const CarInfo &info);
	//! a destructor
	~CarInfo();

	CarInfo &operator = (const CarInfo &info);

protected:

	// callback classes for query chassis and wheels

	class QueryGeometry
	{
	public:
		
		virtual void Begin(void *pModel) const;

		virtual const int GetVertexCount() const
		{
			return 0;
		}
		virtual const float *GetVertexPosition(const int index) const
		{
			return nullptr;
		}

		virtual void End() const;

	};

	QueryGeometry	*mQueryChassis;
	QueryGeometry	*mQueryWheel;

public:

	void			*pChassis;	// pointer to FBModel*

	const QueryGeometry *GetQueryChassisGeometryPtr() const
	{
		return mQueryChassis;
	}
	const QueryGeometry *GetQueryWheelGeometryPtr() const
	{
		return mQueryWheel;
	}

	// wheels
	
	struct Wheel {
		// user data - input 3d model
		void							*pModel;	// pointer to FBModel*

		// physics parameters for simulation
		double							Mass;
		double							Friction;
		double							SuspensionLength;		// 0.20f
		double							SuspensionSpring;		// 15000.0f
		double							SuspensionDamper;		// 600.0f

		float							lateralStiffness;		// 20.0f
		float							longitudinalStiffness;	// 10000.0f
		float							aligningMomentTrail;	// 0.5f

		// result - local and evaluated global matrix
		double							InitialLcl[16];
		double							EvaluatedLcl[16];
		double							EvaluatedGlb[16];

	} wheels[4];
	
	
	double							InitialGlb[16];
	double							EvaluatedGlb[16];

	float		gear1;			// 2.66f
	float		gear2;			// 1.78f
	float		gear3;			// 1.30f
	float		gear4;			// 1.00f
	float		gear5;			// 0.74f
	float		gear6;			// 0.50f
	float		reverseGear;	// 2.90f

	// gearbox
	float		GearReverseRatio;
	int			GearCount;
	float		*GearRatios;

	// common car parameters

	double		Mass;
	double		MassXOffset;
	double		MassYOffset;

	double		SteerAngle;		// steering angle limit (35 by default)

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
};


////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS


// create instances only with this function
PhysicsHardware		*CreateNewPhysicsHardware( const EPhysicsHardwareEngine type, const double scale );
PhysicsCar			*CreateNewPhysicsCar( PhysicsHardware *hardware, const CarInfo *info, void *userdata=nullptr );


//////////////////////////////////////////////////////////////////////////////////////
// common class for physics car simulation and interation with user input

class PhysicsCar
{
public:

	virtual PhysicsCar	*CreateNewCar( const CarInfo &info, void *userdata=nullptr )
	{
		return nullptr;
	}


public:

	//! a constructor
	PhysicsCar(PhysicsHardware *hardware, const CarInfo *info, void *userdata=nullptr )
		: mHardware(hardware)
		, mCarInfo(info)
		, mUserData(userdata)
	{}

	//! a destructor
	virtual ~PhysicsCar()
	{}

	// controlling a car by user
	void	SetPlayerControl(const CarInput &control)
	{
		mInputControl = control;
	}

	
	void SetDriveType(const CarDriveType type) { mDriveType = type; }
	CarDriveType GetDriveType() { return mDriveType; }

	// 4 float per vertex
	//  uses wheel geometry from car info wheels[0] !
	virtual void CalculateTireDimensions (float& width, float& radius) const;

	virtual void UpdateProperties()
	{}

	virtual bool	Init();

	// use current simulation state as initial (dynamic matrix pose to start matrix)
	virtual	void	SaveEvaluatedState()
	{}

	virtual void	UpdateState() = 0;

	// assign initial matrices from the 3d scene object positions
	virtual bool	Snap();	// use mobu models global matrix
	virtual bool	Snap(const double *chassisMatrix, const double *wheelFL, const double *wheelFR, const double *RL, const double *RR);
	
	// reset car simulation and put into the m tranformation
	//  \param m - use nullptr value to reset from Initial matrix
	virtual void	Reset(const double *m) = 0;

	// 4x4 matrix output
	//  cpy result to m
	virtual const double *GetChassisMatrix() const
	{
		if (mCarInfo)
			return mCarInfo->EvaluatedGlb;
		else
			return nullptr;
	}
	// 4x4 matrix output
	//  cpy result to m
	virtual const double *GetWheelMatrix(const int index, const bool local=true) const
	{
		if (mCarInfo)
			return (local) ? mCarInfo->wheels[index].EvaluatedLcl : mCarInfo->wheels[index].EvaluatedGlb;
		else
			return nullptr;
	}

	virtual const double	GetSpeed() { return 0.0; }
	virtual const double	GetRPM() { return 0.0; }

	virtual void DrawDebug() const
	{}

	void	SetUserData( void *data ) { mUserData = data; }
	void	*GetUserData() { return mUserData; }

protected:

	double				mWheelWidth;
	double				mWheelRadius;

	bool				mNPC;		// controlled by curve

	CarDriveType		mDriveType;
	const CarInfo		*mCarInfo;
	CarInputControl		mInputControl;		// store information about last user input

	PhysicsHardware		*mHardware;			// use this hardware engine for this structure

	void				*mUserData;			// store car physics constraint here (for exchange car information between user and 3d scene)

	//double			mInitialMatrix[5][16];		// for chassis and wheels
	//double			mLastMatrix[5][16];

	/////////////////////////////
	// functions used by hardware
	

	// function for self destruction (case when device and hardware is closing)
	void DestroyCar();

	// should be executed by the engine to apply all user input before car phys state calculation
	virtual void ApplyPlayerControl()
	{}

	friend class PhysicsHardware;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
// common class to operate with physics engine

class PhysicsHardware
{
public:
	//
	// allocate specified car class according to the hardware engine

	virtual PhysicsHardware	*CreateNewHardware( const CarInfo &info, void *userdata=nullptr )
	{
		return nullptr;
	}

public:

	//! a constructor
	PhysicsHardware(const double globalScale);
	//! a destructor
	virtual ~PhysicsHardware();

	// layout engine information
	virtual const char *GetPhysicsInfo() { return ""; }

	//--- Opens and closes connection with data server. returns true if successful
	virtual bool	Open();								//!< Open the connection.
	virtual bool	Close();							//!< Close connection.

	//--- Hardware communication
	virtual void	Reset();
	virtual bool	FetchDataPacket	(const double time);		//!< Fetch a data packet from the computer.
	virtual bool	PollData		();						//!< Poll the device for a data packet.
	virtual bool	GetSetupInfo	();						//!< Get the setup information.
	virtual bool	StartDataStream	();						//!< Put the device in streaming mode.
	virtual bool	StopDataStream	();						//!< Take the device out of streaming mode.

	// static collisions in the scene
	virtual void ClearLevel()
	{}
	virtual bool LoadLevel( const LevelInfo &levelInfo )
	{
		return false;
	}

	virtual void WaitForUpdateToFinish() {}

	virtual void	SetGravity(const double *gravity) { memcpy(mGravity, gravity, sizeof(double)*3); }	// three doubles
	const double *GetGravity() const { return &mGravity[0]; }

	void SetGlobalScale(const double scale) { mGlobalScale = scale; }
	const double	GetGlobalScale() const { return mGlobalScale; }

	// snapping current objects tm and restore from this state if needed
	virtual void SaveState();
	virtual void RestoreState();

	void	RegisterCar( PhysicsCar *car_ptr )
	{
		for (auto iter=mCars.begin(); iter!= mCars.end(); ++iter)
		{
			if ( *iter == car_ptr ) return;
		}
		
		mCars.push_back( car_ptr );

	}
	void	UnRegisterCar( PhysicsCar *car_ptr )
	{
		mCars.remove( car_ptr );
	}

	void DrawDebug() const;

protected:
	std::list<PhysicsCar*>			mCars;		//!< registered cars for this device

protected:
	
	double		mGlobalScale;				//!< all phys world global scale according to render world (Newton should have at least 0.1)
	double		mGravity[3];				//!< gravity force

	double m_currentTime;
	double m_timeAccumulator;

	void ApplyPlayerControl()
	{
		for (auto iter=mCars.begin(); iter!=mCars.end(); ++iter)
		{
			(*iter)->ApplyPlayerControl();
		}
	}

	// apply forces according to user input
	virtual void PhysicsPreUpdate();

	// iterate newton physics world
	virtual void PhysicsUpdate(const double dt)
	{}

	// apply transform to scene objects
	virtual void PhysicsPostUpdate();
};


#endif /* __HARDWARE_H__ */
