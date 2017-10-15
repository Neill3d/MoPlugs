
/**	\file	ordeviceinput_hardware.cxx
*	Definition of a virtual hardware class.
*	Contains the definition of the functions for the
*	ORDeviceInputHardware class.
*/

#define PHYSICS_FPS		  120.0
#define FPS_IN_MICROSECUNDS  (int (1000000.0/DEMO_PHYSICS_FPS))
#define FPS_IN_SECONDS	(1.0 / PHYSICS_FPS)
#define MAX_PHYSICS_LOOPS		  1

//--- Class declaration
#include "hardware_common.h"
#include "GL\glew.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

extern PhysicsHardware	*CreateNewBulletHardware(const double scale);

PhysicsHardware	*CreateNewPhysicsHardware( const EPhysicsHardwareEngine type, const double scale )
{
	return CreateNewBulletHardware(scale);
}

extern PhysicsCar *CreateNewBulletCar( PhysicsHardware *hardware, const CarInfo *info, void *userdata );

PhysicsCar	*CreateNewPhysicsCar( PhysicsHardware *hardware, const CarInfo *info, void *userdata )
{
	return CreateNewBulletCar( hardware, info, userdata );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//

LevelInfo::LevelInfo()
{
	vertCount = 0;
	vertices = nullptr;

	polyCount = 0;
	polys = nullptr;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//

CarInfo::CarInfo()
{
	GearCount = 0;
	GearRatios = nullptr;

	mQueryChassis = nullptr;
	mQueryWheel = nullptr;


}

//! a copy constructor
CarInfo::CarInfo(const CarInfo &info)
{
	pChassis = info.pChassis;

	for (int i=0; i<4; ++i)
	{
		wheels[i].pModel = info.wheels[i].pModel;
		wheels[i].Friction = info.wheels[i].Friction;
		wheels[i].Mass = info.wheels[i].Mass;

		wheels[i].SuspensionDamper = info.wheels[i].SuspensionDamper;
		wheels[i].SuspensionLength = info.wheels[i].SuspensionLength;
		wheels[i].SuspensionSpring = info.wheels[i].SuspensionSpring;
	}

	// gearbox
	GearReverseRatio = info.GearReverseRatio; 
	GearCount = info.GearCount;
	GearRatios = new float[GearCount];
	for (int i=0; i<GearCount; ++i)
		GearRatios[i] = info.GearRatios[i];

	Mass = info.Mass;
	MassXOffset = info.MassXOffset;
	MassYOffset = info.MassYOffset;

	SteerAngle = info.SteerAngle;

	//
	IdleRPM = info.IdleRPM;
	IdleTorquePoundPerFoot = info.IdleTorquePoundPerFoot;

	PeakTorqueRPM = info.PeakTorqueRPM;
	PeakTorquePoundPerFoot = info.PeakTorquePoundPerFoot;

	PeakHorsePowerRPM = info.PeakHorsePowerRPM;
	PeakHorsePower = info.PeakHorsePower;
		
	RedLineRPM = info.RedLineRPM;
	RedLineTorquePoundPerFoot = info.RedLineTorquePoundPerFoot;

	vehicleTopSpeedKPH = info.vehicleTopSpeedKPH;

	TireBrakeTorque = info.TireBrakeTorque;
	TireHandBrakeTorque = info.TireHandBrakeTorque;
}

CarInfo &CarInfo::operator = (const CarInfo &info)
{
	pChassis = info.pChassis;

	for (int i=0; i<4; ++i)
	{
		wheels[i].pModel = info.wheels[i].pModel;
		wheels[i].Friction = info.wheels[i].Friction;
		wheels[i].Mass = info.wheels[i].Mass;

		wheels[i].SuspensionDamper = info.wheels[i].SuspensionDamper;
		wheels[i].SuspensionLength = info.wheels[i].SuspensionLength;
		wheels[i].SuspensionSpring = info.wheels[i].SuspensionSpring;
	}

	// gearbox
	GearReverseRatio = info.GearReverseRatio; 
		
	if (GearRatios && (GearCount == info.GearCount) )
	{
		for (int i=0; i<GearCount; ++i)
			GearRatios[i] = info.GearRatios[i];
	}
	else
	{
		GearCount = info.GearCount;

		if (GearRatios)
		{
			delete [] GearRatios;
			GearRatios = nullptr;
		}

		GearRatios = new float[GearCount];
		for (int i=0; i<GearCount; ++i)
			GearRatios[i] = info.GearRatios[i];
	}

	Mass = info.Mass;
	MassXOffset = info.MassXOffset;
	MassYOffset = info.MassYOffset;

	SteerAngle = info.SteerAngle;

	//
	IdleRPM = info.IdleRPM;
	IdleTorquePoundPerFoot = info.IdleTorquePoundPerFoot;

	PeakTorqueRPM = info.PeakTorqueRPM;
	PeakTorquePoundPerFoot = info.PeakTorquePoundPerFoot;

	PeakHorsePowerRPM = info.PeakHorsePowerRPM;
	PeakHorsePower = info.PeakHorsePower;
		
	RedLineRPM = info.RedLineRPM;
	RedLineTorquePoundPerFoot = info.RedLineTorquePoundPerFoot;

	vehicleTopSpeedKPH = info.vehicleTopSpeedKPH;

	TireBrakeTorque = info.TireBrakeTorque;

	return *this;
}

//! a destructor
CarInfo::~CarInfo()
{
	if (GearRatios)
	{
		delete [] GearRatios;
		GearRatios = nullptr;
	}

	//

	if (mQueryChassis)
	{
		delete mQueryChassis;
		mQueryChassis = nullptr;
	}
	if (mQueryWheel)
	{
		delete mQueryWheel;
		mQueryWheel = nullptr;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
//

/************************************************
 *	Constructor.
 ************************************************/
PhysicsHardware::PhysicsHardware(const double globalScale) 
	: mGlobalScale(globalScale)
{
	m_currentTime=0;
	m_timeAccumulator = FPS_IN_SECONDS;
}


/************************************************
 *	Destructor.
 ************************************************/
PhysicsHardware::~PhysicsHardware()
{
}


/************************************************
 *	Open device communications.
 ************************************************/
bool PhysicsHardware::Open()
{
	SaveState();
	return true;
}


/************************************************
 *	Get device setup information.
 ************************************************/
bool PhysicsHardware::GetSetupInfo()
{

	return true;
}


/************************************************
 *	Close device communications.
 ************************************************/
bool PhysicsHardware::Close()
{
	return true;
}


/************************************************
 *	Fetch a data packet from the device.
 ************************************************/
void PhysicsHardware::SaveState()
{
	for (auto iter=mCars.begin(); iter!=mCars.end(); ++iter)
	{
		(*iter)->SaveState();
	}
}

void PhysicsHardware::RestoreState()
{
	for (auto iter=mCars.begin(); iter!=mCars.end(); ++iter)
	{
		(*iter)->Reset(nullptr);
	}

	Reset();
}

void PhysicsHardware::Reset()
{
	m_currentTime = 0;
}

bool PhysicsHardware::FetchDataPacket(const double time)
{
	// TODO: Replace this bogus code with real NON-BLOCKING TCP, UDP or serial calls
	// to your data server.
	PollData();

	// As soon as enough bytes have been read to have data for all makers,
	// return immediately, even if another data packer is waiting
	// This function is nested in a while loop and will be called immediately
	// as soon as the current data frame is processed.

	// TODO: mCounter is a bogus variable there to simulate a time-reference
/*	if(mCounter%2)
	{
		return true; // data for all elements of mChannelData[] has been found.
	}
	else
	{
		return false; // incomplete data packet has been read.
	}*/

	// get the time step
	if ( (m_currentTime == 0) || (m_currentTime > time) ) 
	{
		m_currentTime = time; // - FPS_IN_SECONDS;
	}

	double deltaTime = time - m_currentTime;
	m_currentTime = time;
	m_timeAccumulator += deltaTime;

	if ((m_timeAccumulator >= FPS_IN_SECONDS))
	{
		PhysicsPreUpdate();
		// run the newton update function
		PhysicsUpdate(FPS_IN_SECONDS);

		// call the visual debugger to show the physics scene
#ifdef USE_VISUAL_DEBUGGER
		NewtonDebuggerServe (g_newtonDebugger, g_world);
#endif

		// subtract time from time accumulator
		m_timeAccumulator -= FPS_IN_SECONDS;

		PhysicsPostUpdate();

		return true;
	}


	return false;
}


/************************************************
 *	Poll device.
 *	Device should get itself to send more data.
 ************************************************/
bool PhysicsHardware::PollData()
{
	//Rotational data is in Euler angles in degrees
	//the order is: XYZ. In other words:
	//- rotation around the 'X' axis
	//- followed by a rotation around the 'Y' axis
	//- followed by a rotation around the 'Z' axis

	return true;
}

/************************************************
 *	Start data streaming from device.
 ************************************************/
bool PhysicsHardware::StartDataStream()
{
	return true;
}

/************************************************
 *	Stop data streaming from device.
 ************************************************/
bool PhysicsHardware::StopDataStream()
{
	return true;
}

void PhysicsHardware::DrawDebug() const
{
	const double unscale = 1.0 / mGlobalScale;

	glPushMatrix();

	glScaled(unscale, unscale, unscale);

	for (auto iter=mCars.begin(); iter!=mCars.end(); ++iter)
	{
		(*iter)->DrawDebug();
	}

	glPopMatrix();
}

void PhysicsHardware::PhysicsPreUpdate()
{
	for (auto iter=mCars.begin(); iter!=mCars.end(); ++iter)
	{
		PhysicsCar *pCar = *iter;

		ORConstraint_CarPhysics *pConstraint = (ORConstraint_CarPhysics*) pCar->GetUserData();
		if (pConstraint)
		{
			pConstraint->ApplyPlayerControl();
		}
	}
}

void PhysicsHardware::PhysicsPostUpdate()
{
	for (auto iter=mCars.begin(); iter!=mCars.end(); ++iter)
	{
		PhysicsCar *pCar = (PhysicsCar*) *iter;

		pCar->UpdateState();

		ORConstraint_CarPhysics *pConstraint = (ORConstraint_CarPhysics*) pCar->GetUserData();
		if (pConstraint)
		{
			pConstraint->UpdatePhysStates();
		}
	}
}

////////////////////////////////////////////////////////////////////////
//

bool PhysicsCar::Init()
{
	return false;
}

bool PhysicsCar::Snap()
{
	if (mHardware == nullptr || mCarInfo == nullptr || mCarInfo->pChassis == nullptr) 
		return false;

	FBMatrix chassis, wheelFL, wheelFR, wheelRL, wheelRR;

	FBModel *pModel = (FBModel*) mCarInfo->pChassis;
	if (pModel)
		pModel->GetMatrix(chassis);

	pModel = (FBModel*) mCarInfo->wheels[0].pModel;
	if (pModel)
		pModel->GetMatrix(wheelFL);

	pModel = (FBModel*) mCarInfo->wheels[1].pModel;
	if (pModel)
		pModel->GetMatrix(wheelFL);

	pModel = (FBModel*) mCarInfo->wheels[2].pModel;
	if (pModel)
		pModel->GetMatrix(wheelFL);

	pModel = (FBModel*) mCarInfo->wheels[3].pModel;
	if (pModel)
		pModel->GetMatrix(wheelFL);

	return Snap( chassis, wheelFL, wheelFR, wheelRL, wheelRR );
}

bool PhysicsCar::Snap(const double *chassisMatrix, const double *wheelFL, const double *wheelFR, const double *RL, const double *RR)
{
	if (mHardware == nullptr || mCarInfo == nullptr || mCarInfo->pChassis == nullptr) 
		return false;

	const double scale = mHardware->GetGlobalScale();
	FBMatrix m;

	if (chassisMatrix)
		memcpy( &mCarInfo->InitialGlb[0], chassisMatrix, sizeof(double) * 16 );

	Reset(nullptr);

	return true;
}

