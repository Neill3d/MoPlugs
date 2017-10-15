
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: PhysicsUtils.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "PhysicsUtils.h"
#include "Newton_hardware.h"

// rigid body destructor
void  PhysicsBodyDestructor (const NewtonBody* body)
{
//	RenderPrimitive* primitive;

	// get the graphic object form the rigid body
//	primitive = (RenderPrimitive*) NewtonBodyGetUserData (body);

	// destroy the graphic object
	//	delete primitive;
}

NewtonBody* CreateSimpleBody (NewtonWorld* const world, void* const userData, dFloat mass, const dMatrix& matrix, NewtonCollision* const collision, int materialId)
{

	// calculate the moment of inertia and the relative center of mass of the solid
	//	dVector origin;
	//	dVector inertia;
	//	NewtonConvexCollisionCalculateInertialMatrix (collision, &inertia[0], &origin[0]);	
	//	dFloat Ixx = mass * inertia[0];
	//	dFloat Iyy = mass * inertia[1];
	//	dFloat Izz = mass * inertia[2];

	//create the rigid body
	NewtonBody* const rigidBody = NewtonCreateDynamicBody (world, collision, &matrix[0][0]);

	// set the correct center of gravity for this body (these function are for legacy)
	//	NewtonBodySetCentreOfMass (rigidBody, &origin[0]);
	//	NewtonBodySetMassMatrix (rigidBody, mass, Ixx, Iyy, Izz);

	// use a more convenient function for setting mass and inertia matrix
	NewtonBodySetMassProperties (rigidBody, mass, collision);

	// save the pointer to the graphic object with the body.
	NewtonBodySetUserData (rigidBody, userData);

	// assign the wood id
	NewtonBodySetMaterialGroupID (rigidBody, materialId);

	//  set continuous collision mode
	//	NewtonBodySetContinuousCollisionMode (rigidBody, continueCollisionMode);

	// set a destructor for this rigid body
	NewtonBodySetDestructorCallback (rigidBody, PhysicsBodyDestructor);

	// set the transform call back function
	NewtonBodySetTransformCallback (rigidBody, BaseEntity::TransformCallback);

	// set the force and torque call back function
	NewtonBodySetForceAndTorqueCallback (rigidBody, PhysicsApplyGravityForce);

	// set the matrix for both the rigid body and the graphic body
	//NewtonBodySetMatrix (rigidBody, &matrix[0][0]);
	//PhysicsSetTransform (rigidBody, &matrix[0][0], 0);

	//dVector xxx (0, -9.8f * mass, 0.0f, 0.0f);
	//NewtonBodySetForce (rigidBody, &xxx[0]);

	// force the body to be active of inactive
	int sleepMode = 0;
	NewtonBodySetAutoSleep (rigidBody, sleepMode);
	return rigidBody;
}

IBody* CreateSimpleSolid (CWorldManager* const scene, const void* const mesh, dFloat mass, const dMatrix& matrix, NewtonCollision* const collision, int materialId)
{
	dAssert (mesh);
	dAssert (collision);

	// add an new entity to the world
	CRigidBody* const pbody = new CRigidBody( scene, matrix, NULL);
	scene->GetEntityManager()->Append (pbody->GetEntityPtr());
	pbody->GetEntityPtr()->SetMesh(mesh, dGetIdentityMatrix());
	NewtonBody *newtonBody = CreateSimpleBody (scene->GetNewton(), pbody->GetEntityPtr(), mass, matrix, collision, materialId);
	pbody->GetEntityPtr()->SetNewtonBody(newtonBody);

	return pbody;
}


static dFloat RayCastPlacement (const NewtonBody* const body, const NewtonCollision* const collisionHit, const dFloat* const contact, const dFloat* const normal, dLong collisionID, void* const userData, dFloat intersetParam)
{
	// if the collision has a parent, the this can be it si a sub shape of a compound collision 
	const NewtonCollision* const parent = NewtonCollisionGetParentInstance(collisionHit);
	if (parent) {
		// you can use this to filter sub collision shapes.  
		dAssert (NewtonCollisionGetSubCollisionHandle (collisionHit));
	}


	dFloat* const paramPtr = (dFloat*)userData;
	if (intersetParam < paramPtr[0]) {
		paramPtr[0] = intersetParam;
	}
	return paramPtr[0];
}


static unsigned RayPrefilter (const NewtonBody* const body, const NewtonCollision* const collision, void* const userData)
{
	// if the collision has a parent, the this can be it si a sub shape of a compound collision 
	const NewtonCollision* const parent = NewtonCollisionGetParentInstance(collision);
	if (parent) {
		// you can use this to filter sub collision shapes.  
		dAssert (NewtonCollisionGetSubCollisionHandle (collision));
	}

	return 1;
}

dVector FindFloor (const NewtonWorld* world, const dVector& origin, dFloat dist)
{
	// shot a vertical ray from a high altitude and collect the intersection parameter.
	dVector p0 (origin); 
	dVector p1 (origin - dVector (0.0f, dAbs (dist), 0.0f, 0.0f)); 

	dFloat parameter = 1.2f;
	NewtonWorldRayCast (world, &p0[0], &p1[0], RayCastPlacement, &parameter, RayPrefilter, 0);
	if (parameter < 1.0f) {
		p0 -= dVector (0.0f, dAbs (dist) * parameter, 0.0f, 0.0f);
	}
	return p0;
}