
#pragma once


//--- SDK include
//#include <fbsdk/fbsdk.h>

#include <Newton.h>

///////////////////////////////////////////

//dMatrix FBMatrixTodMatrix(FBMatrix m);
//FBMatrix dMatrixToFBMatrix(const dMatrix matrix);

///////////////////////////////////////////

void SetGravity(const double *value) ;

void SetDebugDisplay(const bool value);
bool GetDebugDisplay();

void PhysicsApplyForceAndTorque0 (const NewtonBody* body, dFloat timestep, int threadIndex);

void PhysicsApplyForceAndTorque1 (const NewtonBody* body, dFloat timestep, int threadIndex);

// set the transformation of a rigid body
void PhysicsSetTransform (const NewtonBody* body, const dFloat* matrix, int threadIndex);

// add force and torque to rigid body
void  PhysicsApplyGravityForce (const NewtonBody* body, dFloat timestep, int threadIndex);

void	PhysicsBodyLeaveWorld (const NewtonBody* const body, int threadIndex);

//NewtonBody *LoadLevelAndSceneRoot(const NewtonWorld *pWorld, const double scale, FBComponentList &components, int optimized);

//NewtonBody *CreateBox(const NewtonWorld *pWorld, const double mass, FBModel *pModel);

// rigid body destructor
void  PhysicsBodyDestructor (const NewtonBody* body);