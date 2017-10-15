/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

/// September 2006: VehicleDemo is work in progress, this file is mostly just a placeholder
/// This VehicleDemo file is very early in development, please check it later
/// One todo is a basic engine model:
/// A function that maps user input (throttle) into torque/force applied on the wheels
/// with gears etc.
#include "btBulletDynamicsCommon.h"

class btVehicleTuning;
struct btVehicleRaycaster;
class btCollisionShape;



//
// By default, Bullet Vehicle uses Y as up axis.
// You can override the up axis, for example Z-axis up. Enable this define to see how to:
//#define FORCE_ZAXIS_UP 1
//

#ifdef FORCE_ZAXIS_UP
		int rightIndex = 0; 
		int upIndex = 2; 
		int forwardIndex = 1;
		btVector3 wheelDirectionCS0(0,0,-1);
		btVector3 wheelAxleCS(1,0,0);
#else
		int rightIndex = 0;
		int upIndex = 1;
		int forwardIndex = 2;
		btVector3 wheelDirectionCS0(0,-1,0);
		btVector3 wheelAxleCS(-1,0,0);
#endif


#include <stdio.h> //printf debugging
#include "Bullet_hardware.h"

const int maxProxies = 32766;
const int maxOverlap = 65535;

///btRaycastVehicle is the interface for the constraint that implements the raycast vehicle
///notice that for higher-quality slow-moving vehicles, another approach might be better
///implementing explicit hinged-wheel constraints with cylinder collision, rather then raycasts


float	maxEngineForce = 1000.f;//this should be engine/velocity dependent
float	maxBreakingForce = 100.f;

float	steeringIncrement = 0.04f;
float	steeringClamp = 0.3f;
float	wheelRadius = 0.5f;
float	wheelWidth = 0.4f;
float	wheelFriction = 1000;//BT_LARGE_FLOAT;
float	suspensionStiffness = 20.f;
float	suspensionDamping = 2.3f;
float	suspensionCompression = 4.4f;
float	rollInfluence = 0.1f;//1.0f;


btScalar suspensionRestLength(0.6);

#define CUBE_HALF_EXTENTS 1

///////////////////////////////////////////////////////////////////////////////////////////////////


PhysicsHardware	*CreateNewBulletHardware(const double scale)
{
	return new BulletHardware(scale);
}

PhysicsCar *CreateNewBulletCar( PhysicsHardware *hardware, const CarInfo *info, void *userdata )
{
	return new BulletCar( hardware, info, userdata );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// VEHICLE


BulletCar::BulletCar(PhysicsHardware *hardware, const CarInfo &info, void *userdata )
	: PhysicsCar(hardware, info, userdata)
	, mCarChassis(0)
{
	mVehicle = 0;
	mWheelShape = 0;

	mEngineForce = 0.0f;
	mBreakingForce = 0.0f;
	mVehicleSteering = 0.0f;
}


BulletCar::~BulletCar()
{
	//delete dynamics world

	if (mVehicle)
	{
		mDynamicsWorld->removeVehicle(mVehicle);
	}

	delete mVehicleRayCaster;
	delete mVehicle;
	delete mWheelShape;
}

void BulletCar::InitPhysics(btDynamicsWorld *dynamicWorld, btAlignedObjectArray<btCollisionShape*> &collisionShapes)
{
	mDynamicsWorld = dynamicWorld;

	btTransform tr;
tr.setIdentity();

#ifdef FORCE_ZAXIS_UP
//   indexRightAxis = 0; 
//   indexUpAxis = 2; 
//   indexForwardAxis = 1; 
	btCollisionShape* chassisShape = new btBoxShape(btVector3(1.f,2.f, 0.5f));
	btCompoundShape* compound = new btCompoundShape();
	btTransform localTrans;
	localTrans.setIdentity();
	//localTrans effectively shifts the center of mass with respect to the chassis
	localTrans.setOrigin(btVector3(0,0,1));
#else
	btCollisionShape* chassisShape = new btBoxShape(btVector3(1.f,0.5f,2.f));
	collisionShapes.push_back(chassisShape);

	btCompoundShape* compound = new btCompoundShape();
	collisionShapes.push_back(compound);
	btTransform localTrans;
	localTrans.setIdentity();
	//localTrans effectively shifts the center of mass with respect to the chassis
	localTrans.setOrigin(btVector3(0,1,0));
#endif

	compound->addChildShape(localTrans,chassisShape);

	tr.setOrigin(btVector3(0,0.f,0));

	//m_carChassis = localCreateRigidBody(800,tr,compound);//chassisShape);
	//m_carChassis->setDamping(0.2,0.2);
	
	mWheelShape = new btCylinderShapeX(btVector3(wheelWidth,wheelRadius,wheelRadius));
	
	Reset(tr);

	/// create vehicle
	{
		
		mVehicleRayCaster = new btDefaultVehicleRaycaster(dynamicWorld);
		mVehicle = new btRaycastVehicle(mTuning, mCarChassis, mVehicleRayCaster);
		
		///never deactivate the vehicle
		mCarChassis->setActivationState(DISABLE_DEACTIVATION);

		dynamicWorld->addVehicle(mVehicle);

		float connectionHeight = 1.2f;
		bool isFrontWheel=true;

		//choose coordinate system
		mVehicle->setCoordinateSystem(rightIndex,upIndex,forwardIndex);

#ifdef FORCE_ZAXIS_UP
		btVector3 connectionPointCS0(CUBE_HALF_EXTENTS-(0.3*wheelWidth),2*CUBE_HALF_EXTENTS-wheelRadius, connectionHeight);
#else
		btVector3 connectionPointCS0(CUBE_HALF_EXTENTS-(0.3*wheelWidth),connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);
#endif

		mVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,mTuning,isFrontWheel);
#ifdef FORCE_ZAXIS_UP
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),2*CUBE_HALF_EXTENTS-wheelRadius, connectionHeight);
#else
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);
#endif

		mVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,mTuning,isFrontWheel);
#ifdef FORCE_ZAXIS_UP
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),-2*CUBE_HALF_EXTENTS+wheelRadius, connectionHeight);
#else
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3*wheelWidth),connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
#endif //FORCE_ZAXIS_UP
		isFrontWheel = false;
		mVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,mTuning,isFrontWheel);
#ifdef FORCE_ZAXIS_UP
		connectionPointCS0 = btVector3(CUBE_HALF_EXTENTS-(0.3*wheelWidth),-2*CUBE_HALF_EXTENTS+wheelRadius, connectionHeight);
#else
		connectionPointCS0 = btVector3(CUBE_HALF_EXTENTS-(0.3*wheelWidth),connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
#endif
		mVehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,mTuning,isFrontWheel);
		
		for (int i=0;i<mVehicle->getNumWheels();i++)
		{
			btWheelInfo& wheel = mVehicle->getWheelInfo(i);
			wheel.m_suspensionStiffness = suspensionStiffness;
			wheel.m_wheelsDampingRelaxation = suspensionDamping;
			wheel.m_wheelsDampingCompression = suspensionCompression;
			wheel.m_frictionSlip = wheelFriction;
			wheel.m_rollInfluence = rollInfluence;
		}
	}
}

void BulletCar::ApplyPlayerControl()
{
	mBreakingForce = mInputControl.brakePedal;
	mEngineForce = mInputControl.engineGasPedal;
	mVehicleSteering = mInputControl.steeringVal;

	{			
		int wheelIndex = 2;
		mVehicle->applyEngineForce(mEngineForce,wheelIndex);
		mVehicle->setBrake(mBreakingForce,wheelIndex);
		wheelIndex = 3;
		mVehicle->applyEngineForce(mEngineForce,wheelIndex);
		mVehicle->setBrake(mBreakingForce,wheelIndex);


		wheelIndex = 0;
		mVehicle->setSteeringValue(mVehicleSteering,wheelIndex);
		wheelIndex = 1;
		mVehicle->setSteeringValue(mVehicleSteering,wheelIndex);

	}
}

void BulletCar::Reset(const btTransform &tr)
{
	mVehicleSteering = 0.f;
	mCarChassis->setCenterOfMassTransform(tr); // btTransform::getIdentity()
	mCarChassis->setLinearVelocity(btVector3(0,0,0));
	mCarChassis->setAngularVelocity(btVector3(0,0,0));
	mDynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(mCarChassis->getBroadphaseHandle(),mDynamicsWorld->getDispatcher());
	if (mVehicle)
	{
		mVehicle->resetSuspension();
		for (int i=0;i<mVehicle->getNumWheels();i++)
		{
			//synchronize the wheels with the (interpolated) chassis worldtransform
			mVehicle->updateWheelTransform(i,true);
		}
	}
}

void BulletCar::Reset(const double *m)
{
	btTransform tm;

	tm.setFromOpenGLMatrix( (m!=nullptr) ? m : mInitialMatrix[0] );

	Reset(tm);
}

void BulletCar::DrawDebug() const
{
	for (int i=0;i<mVehicle->getNumWheels();i++)
	{
		//synchronize the wheels with the (interpolated) chassis worldtransform
		mVehicle->updateWheelTransform(i,true);
		//draw wheels (cylinders)
		//m_vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix(m);
		//m_shapeDrawer->drawOpenGL(m,m_wheelShape,wheelColor,getDebugMode(),worldBoundsMin,worldBoundsMax);
	}
}

void BulletCar::UpdateProperties( const CarInfo &info )
{
	
	
	// update wheels information
	for (int i=0; i<4; ++i)
	{
		btWheelInfo &btInfo = mVehicle->getWheelInfo(i);

		btInfo.m_suspensionRestLength1 = info.wheels[i].SuspensionLength;
		btInfo.m_suspensionStiffness = info.wheels[i].SuspensionSpring;
		btInfo.m_wheelsDampingCompression = info.wheels[i].SuspensionDamper;
		btInfo.m_frictionSlip = info.wheels[i].Friction;
	}

}

void BulletCar::UpdateState()
{
	const btTransform tm = mVehicle->getChassisWorldTransform();
	tm.getOpenGLMatrix( mLastMatrix[0] );

	for (int i=0;i<mVehicle->getNumWheels();i++)
	{
		//synchronize the wheels with the (interpolated) chassis worldtransform
		mVehicle->updateWheelTransform(i,true);
		mVehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix( mLastMatrix[i+1] );
	}
}

const double BulletCar::GetSpeed()
{
	return mVehicle->getCurrentSpeedKmHour();
}

const double BulletCar::GetRPM()
{
	return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VehicleManager


/////////////////////////////////////////////////////////////////////////////
//

BulletHardware::BulletHardware(const double globalScale)
	: PhysicsHardware(globalScale)
	, mDefaultContactProcessingThreshold(BT_LARGE_FLOAT)
{
	mLevelCollisionShape = nullptr;
	mLevelRigidBody = nullptr;
}

BulletHardware::~BulletHardware()
{
		//cleanup in the reverse order of creation/initialization

	ClearLevel();

	//remove the rigidbodies from the dynamics world and delete them
	int i;
	for (i=mDynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
	{
		btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		mDynamicsWorld->removeCollisionObject( obj );
		delete obj;
	}

	//delete collision shapes
	for (int j=0;j<mCollisionShapes.size();j++)
	{
		btCollisionShape* shape = mCollisionShapes[j];
		delete shape;
	}

	//delete dynamics world
	delete mDynamicsWorld;
	
	//delete solver
	delete mConstraintSolver;

	//delete broadphase
	delete mOverlappingPairCache;

	//delete dispatcher
	delete mDispatcher;

	delete mCollisionConfiguration;

}

void BulletHardware::InitPhysics()
{
	
#ifdef FORCE_ZAXIS_UP
	m_cameraUp = btVector3(0,0,1);
	m_forwardAxis = 1;
#endif

	btCollisionShape* groundShape = new btBoxShape(btVector3(50,3,50));
	mCollisionShapes.push_back(groundShape);
	mCollisionConfiguration = new btDefaultCollisionConfiguration();
	mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
	mOverlappingPairCache = new btAxisSweep3(worldMin,worldMax);
	mConstraintSolver = new btSequentialImpulseConstraintSolver();
	mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mOverlappingPairCache, mConstraintSolver, mCollisionConfiguration);
#ifdef FORCE_ZAXIS_UP
	m_dynamicsWorld->setGravity(btVector3(0,0,-10));
#endif 

	//m_dynamicsWorld->setGravity(btVector3(0,0,0));


}

btDynamicsWorld *BulletHardware::GetDynamicsWorld() const
{
	return mDynamicsWorld;
}

void BulletHardware::DrawDebug() const
{
	btVector3	worldBoundsMin,worldBoundsMax;
	GetDynamicsWorld()->getBroadphase()->getBroadphaseAabb(worldBoundsMin,worldBoundsMax);

	for (auto iter=mCars.begin(); iter!=mCars.end(); ++iter)
	{
		(*iter)->DrawDebug();
	}
}

void BulletHardware::PhysicsUpdate(const double dt)
{
	// apply for all connected to this world cars
	ApplyPlayerControl();

	double ldt = dt;

	if (mDynamicsWorld)
	{
		//during idle mode, just run 1 simulation step maximum
		int maxSimSubSteps = mIdle ? 1 : 2;
		if (mIdle)
			ldt = 1.0/420.f;

		int numSimSteps = mDynamicsWorld->stepSimulation(ldt,maxSimSubSteps);
		

//#define VERBOSE_FEEDBACK
#ifdef VERBOSE_FEEDBACK
		if (!numSimSteps)
			printf("Interpolated transforms\n");
		else
		{
			if (numSimSteps > maxSimSubSteps)
			{
				//detect dropping frames
				printf("Dropped (%i) simulation steps out of %i\n",numSimSteps - maxSimSubSteps,numSimSteps);
			} else
			{
				printf("Simulated (%i) steps\n",numSimSteps);
			}
		}
#endif //VERBOSE_FEEDBACK

	}
}

void BulletHardware::ResetClientState()
{
	for (auto iter=mCars.begin(); iter!=mCars.end(); ++iter)
	{
		(*iter)->Reset(nullptr);
	}
}

// static collisions in the scene
void BulletHardware::ClearLevel()
{
	// TODO: clear static collisions

	if (mLevelRigidBody)
	{
		mDynamicsWorld->removeRigidBody( mLevelRigidBody );
	}

	//delete collision shapes
	if (mLevelCollisionShape)
	{
		delete mLevelCollisionShape;
		mLevelCollisionShape = nullptr;
	}
}

bool BulletHardware::LoadLevel( const LevelInfo &levelInfo )
{
	// TODO: load static collisions
	
	const int vertStride = sizeof(VertInfo);
	const int indexStride = sizeof(PolyInfo);

	mIndexVertexArrays = new btTriangleIndexVertexArray(levelInfo.polyCount,
		levelInfo.polys[0].indices,
		indexStride,
		levelInfo.vertCount,(btScalar*) &levelInfo.vertices[0].pos,vertStride);

	bool useQuantizedAabbCompression = true;
	mLevelCollisionShape = new btBvhTriangleMeshShape(mIndexVertexArrays,useQuantizedAabbCompression);

	//create ground object
	btTransform tr;
	tr.setOrigin(btVector3(0,0,0));//-64.5f,0));
	mLevelRigidBody = localCreateRigidBody(0,tr,mLevelCollisionShape);	// create static rigid body

	return true;
}


btRigidBody *BulletHardware::localCreateRigidBody(float mass, const btTransform& startTransform,btCollisionShape* shape)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);

	btRigidBody* body = new btRigidBody(cInfo);
	body->setContactProcessingThreshold(mDefaultContactProcessingThreshold);

#else
	btRigidBody* body = new btRigidBody(mass,0,shape,localInertia);	
	body->setWorldTransform(startTransform);
#endif//

	mDynamicsWorld->addRigidBody(body);

	return body;
}



