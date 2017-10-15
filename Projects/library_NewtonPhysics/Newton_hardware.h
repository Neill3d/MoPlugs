#ifndef __ORDEVICE_NEWTON_HARDWARE_H__
#define __ORDEVICE_NEWTON_HARDWARE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Newton_hardware.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
//#include <fbsdk/fbsdk.h>

#include "toolbox_stdafx.h"

#include "..\Common_Physics\physics_common.h"
#include "CustomVehicleControllerManager.h"

#include "Newton_entities.h"
#include <memory>

using namespace PHYSICS_INTERFACE;

class CRigidBody;
class CCar;
class CWorldManager;

///////////////////////////////////////////

// add force and torque to rigid body
void  PhysicsApplyGravityForce (const NewtonBody* body, dFloat timestep, int threadIndex);

NewtonBody *LoadLevelAndSceneRoot(const NewtonWorld *pWorld, const double scale, const IQueryGeometry *level, int optimized, NewtonCollision *&outLevelCollision);


//////////////////////////////////////////////////////////////////////////////////////////////
// class CWorldManager

class CWorldManager : public IWorld
{
public:
	
	//! Constructor.
	CWorldManager(const double globalScale, const int numberOfThreads, const int physicsLoops, const float physicsFPS);

	//! Destructor.
	virtual ~CWorldManager();

	// layout engine information
	virtual const char *GetPhysicsInfo();

	//--- Opens and closes connection with data server. returns true if successful
	virtual bool	Open() override;								//!< Open the connection.
	virtual bool	Close() override;							//!< Close connection.

	virtual bool	Clear() override;

	//--- Hardware communication
	virtual void	Reset();
	virtual bool	FetchDataPacket	(const double animTimeSecs) override;		//!< Fetch a data packet from the computer.
	virtual double	GetCurrPhysTimeSecs() override;
	virtual double	GetLastPhysTimeSecs() override;
	virtual bool	PollData		();						//!< Poll the device for a data packet.
	virtual bool	GetSetupInfo	();						//!< Get the setup information.
	virtual bool	StartDataStream	();						//!< Put the device in streaming mode.
	virtual bool	StopDataStream	();						//!< Take the device out of streaming mode.

	// 3 double values
	virtual void	SetGravity(const double *gravity);
	virtual const double *GetGravity() const;

	virtual void SetGlobalScale(const double scale);
	virtual const double	GetGlobalScale() const;

	
	// set default material physical parameters
	virtual void SetDefaultSoftness(const double value);
	virtual const double GetDefaultSoftness() const;
	
	virtual void SetDefaultElasticity(const double value);
	virtual const double GetDefaultElasticity() const;
	
	virtual void SetDefaultCollidable(const double value);
	virtual const double GetDefaultCollidable() const;
	
	virtual void SetDefaultFriction(const double staticValue, const double dynamicValue);
	virtual const double GetDefaultStaticFriction() const;
	virtual const double GetDefaultDynamicFriction() const;

	virtual void SetDefaultMaterialParams(double softness, double elasticity, double collidable, double staticFriction, double dynamicFriciton);

	// static collisions in the scene
	virtual void ClearLevel() override;
	virtual bool LoadLevel( const IQueryGeometry *levelInfo );

	// utility function
	//
	// return true if we have a ray hit (fill up pos and nor with ray hit position and normal)
	//
	virtual bool ClosestRayCast(const double *p0, const double *p1, double *pos, double *nor, double *dist, long long *attribute) override;

	// sync

	virtual void WaitForUpdateToFinish();

	// snapping current objects tm and restore from this state if needed
	virtual void SaveState();
	virtual void RestoreState();

	virtual void DrawDebug() const;

	virtual void Serialize(const char *filename);

	//
	virtual IBody	*CreateNewBody( const BodyOptions *info, const IQueryGeometry *pgeometry, bool convexhull );
	virtual ICar	*CreateNewCar( const CarOptions *info, const IQueryGeometry **pgeometry, const IQueryPath *curve );

	CustomVehicleControllerManager *GetVehicleManager() const;

	BaseEntityManager *GetEntityManager() const
	{
		return mManager.get();
	}

	NewtonWorld	*GetNewton() const
	{
		return mManager->GetNewton();
	}

protected:
	
	// iterate newton physics world
	void PhysicsUpdate(const double dt);

private:

	std::auto_ptr<BaseEntityManager>	mManager;

	double							m_currentTime;

	NewtonBody						*mLevelBody;
	NewtonCollision					*mLevelCollision;
	CarVehicleControllerManager		*mVehicleManager;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// NewtonBody

class CRigidBody : public IBody
{
public:
	//! a constructor
	CRigidBody( IWorld *world, const dMatrix &matrix, BaseEntity *parent )
		: IBody(world)
		, mEntity(new BaseEntity(matrix, parent))
	{}
	//! a destructor
	virtual ~CRigidBody()
	{
	}

	virtual void	SaveState()
	{
	}
	virtual void	RestoreState()
	{
	}

	virtual bool	Snap()
	{
		return false;
	}

	virtual const double *GetMatrix(IWorld *world);

	BaseEntity	*GetEntityPtr() {
		return mEntity.get();
	}

protected:

	std::auto_ptr<BaseEntity>	mEntity;

	double		m_dmatrix[16];
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// NewtonCar

class CCar : public ICar
{
public:

	// ! a constructor - attach car to a physics world
	//		we need 5 classes to query chassis and 4 wheels matrix and geometry for a car startup
	CCar( CWorldManager *worldManager, const CarOptions *options, const IQueryGeometry *chassis, const IQueryGeometry *wheelFL, const IQueryGeometry *wheelFR, const IQueryGeometry *wheelRL, const IQueryGeometry *wheelRR, const IQueryPath *curve )
		: ICar( worldManager )
		, mEntity(new CarEntity(*worldManager->GetEntityManager(), worldManager->GetVehicleManager(), options, chassis, wheelFL, wheelFR, wheelRL, wheelRR, curve ) )
	{
		// add car to the global world manager (wheels are parented to the car)
		if (mEntity.get() )
			worldManager->GetEntityManager()->Append(mEntity.get() );
	}

	//! a destructor
	virtual ~CCar()
	{}

	// controlling a car by user
	virtual void	SetPlayerControl(const CarInput &control) override
	{
		if (mEntity.get() )
			mEntity->SetPlayerInput( control );
	}
	virtual void	SetPlayerControl(const double torque, const double clutch, const double steering, const double curveSteeringBlend, const double brake, const double handbrake, int gear) override
	{
		if (mEntity.get() )
			mEntity->SetPlayerInput( torque, clutch, steering, curveSteeringBlend, brake, handbrake, gear );
	}
	virtual void	SetOptions( const CarOptions *options )
	{
		if (mEntity.get() )
			mEntity->UpdateOptions(options);
	}

	virtual void SetDriveType(const CarDriveType type) {
		mDriveType = type;
	}
	virtual CarDriveType GetDriveType()
	{
		return mDriveType;
	}

	// use current simulation state as initial (dynamic matrix pose to start matrix)
	virtual	void	SaveState()
	{
		if (mEntity.get() )
			mEntity->SaveState();
	}
	virtual void	RestoreState()
	{
		if (mEntity.get() )
		{
			CWorldManager *pmanager = (CWorldManager*) mWorld;
			mEntity->RestoreState(*(pmanager->GetEntityManager()), false);
		}
	}
	virtual void	Snap()
	{
	}
	
	//

	virtual const double *GetChassisMatrix()
	{
		if (mEntity.get() == nullptr)
			return nullptr;

		CWorldManager *pmanager = (CWorldManager*) mWorld;
		return mEntity->GetMatrixd(*(pmanager->GetEntityManager()));
	}
	virtual const double *GetWheelMatrix(const int wheel, const bool local)
	{
		if (mEntity.get() == nullptr)
			return nullptr;

		CWorldManager *pmanager = (CWorldManager*) mWorld;
		return mEntity->GetWheelMatrixd(*(pmanager->GetEntityManager()), wheel);
	}

	virtual const double	GetSpeed() override {
		if (mEntity.get() == nullptr)
			return 0.0;

		return mEntity->GetSpeed();
	}
	virtual const double	GetRPM() override
	{
		if (mEntity.get() == nullptr)
			return 0.0;

		return mEntity->GetRPM();
	}
	virtual const int GetCurrentGear() override
	{
		if (mEntity.get() == nullptr)
			return 0;
		return mEntity->GetCurrentGear();
	}

	virtual void DrawDebug() const
	{
	}

public:

	virtual void Render(dFloat timeStep, BaseEntityManager* const scene) const
	{
	}

private:
	
	std::auto_ptr<CarEntity>			mEntity;
	
	CarDriveType						mDriveType;

};

#endif /* __ORDEVICE_NEWTON_HARDWARE_H__ */
