
#pragma once

#include "hardware_common.h"


#include "BulletDynamics/Vehicle/btRaycastVehicle.h"
#include <list>

//////////////////////////////////////////////////////////////////////////
//

class BulletCar : public PhysicsCar
{
public:
	//! a constructor
	BulletCar( PhysicsHardware *hardware, const CarInfo *info, void *userdata=nullptr );

	//! a destructor
	virtual ~BulletCar();

	void						InitPhysics(btDynamicsWorld *dynamicWorld, btAlignedObjectArray<btCollisionShape*> &collisionShapes);

	virtual void				ApplyPlayerControl();
	
	virtual void				DrawDebug() const;

	// reset car simulation and put into the m tranformation
	void			Reset(const btTransform &tr);
	virtual void	Reset(const double *m);

	// update tm after physics simulation
	virtual void	UpdateState();

	// change physics properties during simulation
	virtual void UpdateProperties( const CarInfo &info );

	// some car stats
	virtual const double	GetSpeed();
	virtual const double	GetRPM();

private:
	btDynamicsWorld						*mDynamicsWorld;

	btRigidBody							*mCarChassis;

	btRaycastVehicle::btVehicleTuning	mTuning;
	btVehicleRaycaster*					mVehicleRayCaster;
	btRaycastVehicle*					mVehicle;
	btCollisionShape*					mWheelShape;

	// hold user input
	float								mEngineForce;
	float								mBreakingForce;
	float								mVehicleSteering;

	friend class BulletHardware;
};



///////////////////////////////////////////////////////////////////////////
//

class BulletHardware : public PhysicsHardware
{
public:

	//! a constructor
	BulletHardware(const double globalScale);
	//! a destructor
	virtual ~BulletHardware();

	void					ResetClientState();

	
	btDynamicsWorld			*GetDynamicsWorld() const;

	virtual void			DrawDebug() const;

	void					InitPhysics();


	// static collisions in the scene
	virtual void ClearLevel();
	virtual bool LoadLevel( const LevelInfo &levelInfo );

private:

	bool										mIdle;

	///this is the most important class
	btDynamicsWorld								*mDynamicsWorld;
	btAlignedObjectArray<btCollisionShape*>		mCollisionShapes;

	class btBroadphaseInterface					*mOverlappingPairCache;
	class btCollisionDispatcher					*mDispatcher;
	class btConstraintSolver					*mConstraintSolver;
	class btDefaultCollisionConfiguration		*mCollisionConfiguration;

	// for ground collision
	class btTriangleIndexVertexArray			*mIndexVertexArrays;
	class btCollisionShape						*mLevelCollisionShape;
	class btRigidBody							*mLevelRigidBody;

	btScalar									mDefaultContactProcessingThreshold;

	btRigidBody		*localCreateRigidBody(float mass, const btTransform& startTransform,btCollisionShape* shape);

protected:
	// iterate newton physics world
	virtual void PhysicsUpdate(const double dt);
};


