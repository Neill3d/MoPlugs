
#include "World.h"
#include "Newton\dMathDefines.h"
#include "Newton\DebugDisplay.h"
#include "Newton\dMatrix.h"
#include "Newton\dVector.h"

//--- SDK include
//#include <fbsdk/fbsdk.h>

//////////////////////////////////////////////////////

bool			gDebugDisplay = true;

double			gGravity[3] = {0.0, -20.0, 0.0}; 

///////////////////////////////////////////////////////////

dMatrix FBMatrixTodMatrix(FBMatrix m)
{
	dMatrix matrix;

	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			matrix[i][j] = (dFloat) m(i,j);

	return matrix;
}

FBMatrix dMatrixToFBMatrix(const dMatrix matrix)
{
	FBMatrix m;

	for (int ii=0; ii<4; ++ii)
			for (int jj=0; jj<4; ++jj)
				m(ii, jj) = (double) matrix[ii][jj];

	return m;
}

//////////////////////////////////////////////////////////

void SetGravity(const double *value) 
{
	memcpy( gGravity, value, sizeof(double)*3 );
}

void SetDebugDisplay(const bool value) {
	gDebugDisplay = value;
}

bool GetDebugDisplay() {
	return gDebugDisplay;
}

// this is the call back for allocation newton memory
void* AllocMemory (int sizeInBytes)
{
	return malloc (sizeInBytes);
}

// this is the callback for freeing Newton Memory
void FreeMemory (void *ptr, int sizeInBytes)
{
	free (ptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void PhysicsApplyForceAndTorque0 (const NewtonBody* body, dFloat timestep, int threadIndex)
{
	float f[3];
	f[0]=0;
	f[1]=0;
	f[2]=0;
	NewtonBodyAddForce(body,f);
}

void PhysicsApplyForceAndTorque1 (const NewtonBody* body, dFloat timestep, int threadIndex)
{
	float f[3];
	f[0]=0;
	f[1]=0;
	f[2]=-10;
	NewtonBodyAddForce(body,f);
}

// set the transformation of a rigid body
void PhysicsSetTransform (const NewtonBody* body, const dFloat* matrix, int threadIndex)
{
	// get the graphic object form the rigid body
	FBModel *pModel = (FBModel*) NewtonBodyGetUserData (body);
	if (pModel == nullptr) return;
	_ASSERTE (pModel);
	/*
	if (gDebugDisplay) {
		
		dMatrix& mat = *((dMatrix*)matrix);
		dMatrix mm = mat;
		NewtonWorldCriticalSectionLock(gWorld);
//		DebugDrawCollision (boxCollision, mm, dVector (1.0f, 1.0f, 0.0f, 1.0f));
		//DebugDrawLine (tire.m_contactPoint, car->dbpos, dVector (0.5f, 0.0f, 1.0f, 1.0f));
		NewtonWorldCriticalSectionUnlock(gWorld);
		return;
	}
	*/
	// set the transformation matrix for this rigid body
	dMatrix& mat = *((dMatrix*)matrix);

	FBMatrix m;

	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			m(i,j) = (double) mat[i][j];

	//m[13] = mat.m_posit.m_z;
	//m[14] = mat.m_posit.m_y;

	FBTVector	T;
	FBRVector	R;
	FBSVector	S;

	FBMatrixToTRS( T, R, S, m );

//	pModel->Translation.SetGlobalCandidate( T, sizeof(double)*3 );
//	pModel->Rotation.SetGlobalCandidate( R, sizeof(double)*3 );
}

// add force and torque to rigid body
void  PhysicsApplyGravityForce (const NewtonBody* body, dFloat timestep, int threadIndex)
{
	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;
	dFloat mass;

	NewtonBodyGetMassMatrix (body, &mass, &Ixx, &Iyy, &Izz);
	dVector force (mass * gGravity[0], mass * gGravity[1], mass * gGravity[2]);
	NewtonBodySetForce (body, &force.m_x);
}


NewtonBody *LoadLevelAndSceneRoot( const NewtonWorld *pWorld, const double scale, FBComponentList &components, int optimized)
{
	FBMatrix scaleM;
	FBScalingToMatrix( scaleM, FBSVector(scale, scale, scale) );

	//
	// build collision object
	//
	NewtonCollision* collision;

	// create the collision tree geometry
	collision = NewtonCreateTreeCollision(pWorld, 0);

	// prepare to create collision geometry
	NewtonTreeCollisionBeginBuild(collision);

	// iterate the entire geometry an build the collision

//	int ndx = 0;
	int matID = 0;
	FBVertex face[4];

	bool isOk = true;

	for (int i=0; i<components.GetCount(); ++i)
	if ( FBIS( components[i], FBModel ) )
	{
		FBModel *pModel = (FBModel*) components[i];

		FBMatrix m;
		pModel->GetMatrix(m);
		FBGeometry *pGeometry = pModel->Geometry;
		FBMesh *pMesh = (FBMesh*) pGeometry;
		
		FBVertex *pVertices = (FBVertex*) pMesh->GetPositionArray();
	
		FBFastTessellator *tess = pMesh->FastTessellatorPolygon;

		int idx=0;
		int polyCount = tess->PrimitiveCount;
		
		for (int j=0; j<polyCount; ++j)
		{
			const int polyVertCount = tess->PrimitiveVertexCount[j];

			if (polyVertCount > 4) 
			{
				isOk = false;
				continue;	// skip polygons creater then tri or quad
			}

			for (int k=0; k<polyVertCount; ++k)
			{
				FBVertexMatrixMult( face[k], m, pVertices[ tess->VertexIndex[idx] ] );
				FBVertexMatrixMult( face[k], scaleM, face[k] );

				idx++;
			}

			NewtonTreeCollisionAddFace(collision, polyVertCount, &face[0][0], sizeof (dVector), matID);
		}

		/*
		if (pMesh->IsTriangleMesh() )
		{

			int count=0;
			const int *indices = pMesh->PolygonVertexArrayGet(count);

	
			for (int j=0; j<count; j+=3)
			{
				for (int k=0; k<3; ++k)
				{
					FBVertexMatrixMult( face[k], m, pVertices[ indices[j+k] ] );
					FBVertexMatrixMult( face[k], scaleM, face[k] );
				}

				NewtonTreeCollisionAddFace(collision, 3, &face[0][0], sizeof (dVector), matID);
			}
		}
		else
		{
			int count = pMesh->PolygonCount();

			for (int j=0; j<count; ++j)
			{
				int polyCount = pMesh->PolygonVertexCount(j);

				if (polyCount <= 4)
				{
					for (int k=0; k<polyCount; ++k)
					{
						FBVertexMatrixMult( face[k], m, pVertices[ pMesh->PolygonVertexIndex(j, k) ] );
						FBVertexMatrixMult( face[k], scaleM, face[k] );
					}
					NewtonTreeCollisionAddFace(collision, polyCount, &face[0][0], sizeof (dVector), matID);
				}
			}
		}
		*/
	}

	NewtonTreeCollisionEndBuild(collision, optimized);

	if ( isOk == false )
	{
		FBMessageBox( "Newton Physics", "Static collision containts a face with more than 4 vertices!", "Ok" ); 
	}

	//
	// create the level rigid body
	//
	dMatrix   m_matrix = GetIdentityMatrix();
	//m_matrix = dPitchMatrix (15.0f* 3.141592f / 180.0f);
	
	NewtonBody *pLevel = NewtonCreateBody(pWorld, collision, &m_matrix[0][0]);

	// release the collision tree (this way the application does not have to do book keeping of Newton objects
	NewtonReleaseCollision (pWorld, collision);

	// set the global position of this body
	NewtonBodySetMatrix (pLevel, (float*) &m_matrix[0][0]);

	// save the pointer to the graphic object with the body.
	NewtonBodySetUserData (pLevel, NULL);


	// set a destructor for this rigid body
//	NewtonBodySetDestructorCallback (m_level, Destructor);


	dVector boxP0;
	dVector boxP1;
	// get the position of the aabb of this geometry
	NewtonCollisionCalculateAABB (collision, (float*) &m_matrix[0][0], &boxP0.m_x, &boxP1.m_x);

	// add some extra padding the world size
	boxP0.m_x -=  500.0f * scale;
	boxP0.m_y -=  500.0f * scale;
	boxP0.m_z -=  500.0f * scale;
	boxP1.m_x +=  500.0f * scale;
	boxP1.m_y +=  500.0f * scale;
	boxP1.m_z +=  500.0f * scale;

	// set the world size
	NewtonSetWorldSize (pWorld, &boxP0.m_x, &boxP1.m_x);


  //
  // world settings
  //
//  NewtonBodySetDestructorCallback (gLevel, PhisicsBodyDestructor);

  int defaultID;
  defaultID = NewtonMaterialGetDefaultGroupID (pWorld);

  // set default material properties
  NewtonMaterialSetDefaultSoftness( pWorld, defaultID, defaultID, 0.05f );
  NewtonMaterialSetDefaultElasticity( pWorld, defaultID, defaultID, 0.4f );
  NewtonMaterialSetDefaultCollidable( pWorld, defaultID, defaultID, 1.0f );
  NewtonMaterialSetDefaultFriction( pWorld, defaultID, defaultID, 1.0f, 0.5f );

  // set the island update callback
  //NewtonSetIslandUpdateEvent( gWorld, PhysicsIslandUpdate );

  return pLevel;
}

// rigid body destructor
void  PhysicsBodyDestructor (const NewtonBody* body)
{
	//RenderPrimitive* primitive;

	// get the graphic object form the rigid body
	//primitive = (RenderPrimitive*) NewtonBodyGetUserData (body);

	// destroy the graphic object
//	delete primitive;
}

void	PhysicsBodyLeaveWorld (const NewtonBody* const body, int threadIndex)
{
	printf ("leave world\n" );
}

NewtonBody *CreateBox(const NewtonWorld *pWorld, const double mass, FBModel *pModel)
{
	if (pModel == nullptr) return nullptr;

	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;
	dFloat lMass = (dFloat) mass;
	dVector origin(0, 0, 0, 1);
	dVector inertia(0, 0, 0, 1);

	FBVector3d min, max;
	pModel->GetBoundingBox( min, max );

	NewtonCollision *boxCollision = NewtonCreateBox (	pWorld, 
														(dFloat) (max[0]-min[0]), 
														(dFloat) (max[1]-min[1]), 
														(dFloat) (max[2]-min[2]), 
														0, 
														NULL);

// calculate the moment of inertia and the relative center of mass of the solid
	NewtonConvexCollisionCalculateInertialMatrix (boxCollision, &inertia[0], &origin[0]);
	Ixx = lMass * inertia[0];
	Iyy = lMass * inertia[1];
	Izz = lMass * inertia[2];

	dMatrix matrix;
	matrix = GetIdentityMatrix();

  	//create the rigid body
	NewtonBody *boxBody = NewtonCreateBody (pWorld, boxCollision, &matrix[0][0]);

	// release the collision geometry when not need it
	NewtonReleaseCollision (pWorld, boxCollision);


	// set the correct center of gravity for this body
	NewtonBodySetCentreOfMass (boxBody, &origin[0]);

	// set the mass matrix
	NewtonBodySetMassMatrix (boxBody, lMass, Ixx, Iyy, Izz);

	// activate
	//	NewtonBodyCoriolisForcesMode (blockBoxBody, 1);

	// save the pointer to the graphic object with the body.
	NewtonBodySetUserData (boxBody, (void*) pModel );

	// assign the wood id
	int matId = NewtonMaterialGetDefaultGroupID(pWorld);

	NewtonBodySetMaterialGroupID (boxBody, matId);

//  set continue collision mode
//	NewtonBodySetContinuousCollisionMode (rigidBody, continueCollisionMode);

	// set a destructor for this rigid body
	NewtonBodySetDestructorCallback (boxBody, PhysicsBodyDestructor);

	// set the transform call back function
	//NewtonBodySetTransformCallback (boxBody, PhysicsSetTransform);

	// set the force and torque call back function
	NewtonBodySetForceAndTorqueCallback (boxBody, PhysicsApplyGravityForce);

  
	FBMatrix m;
	pModel->GetMatrix(m);

	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			matrix[i][j] = (dFloat) m(i,j);

	// set the matrix for both the rigid body and the graphic body
	NewtonBodySetMatrix (boxBody, &matrix[0][0]);
	PhysicsSetTransform (boxBody, &matrix[0][0], 0);

//dVector xxx (0, -9.8f * mass, 0.0f, 0.0f);
//NewtonBodySetForce (rigidBody, &xxx[0]);

	// force the body to be active of inactive
	NewtonBodySetAutoSleep (boxBody, 0);

	return boxBody;
}