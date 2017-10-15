
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: newton_PUBLIC.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "newton_public.h"
#include "PhysicsUtils.h"
#include "Newton_hardware.h"

using namespace PHYSICS_INTERFACE;


IWorld *CreateNewNewtonWorld( const double scale, const int numberOfThreads, const int physicsLoops, const float physicsFPS )
{
	return new CWorldManager(scale, numberOfThreads, physicsLoops, physicsFPS);
}

ICar *CreateNewNewtonCar( IWorld *world, const CarOptions *options, const PHYSICS_INTERFACE::IQueryGeometry *carGeometry[5], const PHYSICS_INTERFACE::IQueryPath *curve )
{
	CWorldManager *pManager = (CWorldManager*) world;
	
	return new CCar( pManager, options, carGeometry[0], carGeometry[1], carGeometry[2], carGeometry[3], carGeometry[4], curve );
}

NewtonCollision *CreateConvexCollision(NewtonWorld* const world, const dFloat globalScaling, const IQueryGeometry *geometry)
{
	if (world == nullptr || geometry == nullptr)
		return nullptr;

	dMatrix   localMatrix = dGetIdentityMatrix();
	
	const double *modelScaling = geometry->GetScale(false);

	int vertexCount = geometry->GetVertexCount();
	int stride = sizeof(dFloat) * 4;

	const float *posSrc = geometry->GetVertexPosition(0);
	dFloat *posDst = new dFloat[vertexCount*4];

	for (int i=0; i<vertexCount; ++i)
	{
		posDst[i*4    ] = (dFloat)posSrc[i*4  ] * globalScaling * (dFloat) modelScaling[0];
		posDst[i*4 + 1] = (dFloat)posSrc[i*4+1] * globalScaling * (dFloat) modelScaling[1];
		posDst[i*4 + 2] = (dFloat)posSrc[i*4+2] * globalScaling * (dFloat) modelScaling[2];
		posDst[i*4 + 3] = (dFloat)1.0;
	}
	
	//
	NewtonCollision *result = NewtonCreateConvexHull(	world, 
									vertexCount, 
									&posDst[0], 
									stride, 0.001, 
									0, 
									&localMatrix[0][0]);

	if (posDst)
	{
		delete [] posDst;
		posDst = nullptr;
	}

	return result;
}

IBody *CreateNewNewtonBody( IWorld *world, const BodyOptions *options, const IQueryGeometry *geometry, bool convexhull )
{
	CWorldManager *pManager = (CWorldManager*) world;
	const float globalScaling = (float) pManager->GetGlobalScale();

	if (world == nullptr || pManager->GetNewton() == nullptr || options == nullptr || geometry == nullptr) 
		return nullptr;

	const bool convexCollision = convexhull;
	NewtonCollision *boxCollision = nullptr;

	if (convexCollision)
	{
		boxCollision = CreateConvexCollision( pManager->GetNewton(), globalScaling, geometry );
	}
	else
	{
		dVector dmin, dmax;
		geometry->GetBoundingBoxD(&dmin[0], &dmax[0]);
		const double *srcScale = geometry->GetScale(true);

		for (int i=0; i<3; ++i)
		{
			dmin[i] *= (dFloat)srcScale[i] * globalScaling;
			dmax[i] *= (dFloat)srcScale[i] * globalScaling;
		}


		boxCollision = NewtonCreateBox (	pManager->GetNewton(), 
														dmax[0]-dmin[0], 
														dmax[1]-dmin[1], 
														dmax[2]-dmin[2], 
														0, 
														NULL);
	}

	// assign the wood id
	int matId = NewtonMaterialGetDefaultGroupID(pManager->GetNewton());

	//create the rigid body
	//geometry->PrepMatrix(true);
	dMatrix dstTM( dGetIdentityMatrix() );
	geometry->GetMatrixTR(true, &dstTM[0][0]);
	//const double *srcPos = geometry->GetPosition(true);
	//const double *srcQuat = geometry->GetQuaternion(true);
	
	dstTM.m_posit.m_x *= globalScaling;
	dstTM.m_posit.m_y *= globalScaling;
	dstTM.m_posit.m_z *= globalScaling;

	//dVector dstPos(0.0f, 0.0f, 0.0f, 1.0);
	//dVector dstRot;

	//geometry->GetPositionF( &dstPos[0] );
	//geometry->GetRotationF( &dstRot[0] );

	//dMatrix dstTM( dstRot[1], dstRot[0], dstRot[2], dstPos );
	IBody *pBodyEnt = CreateSimpleSolid( pManager, geometry, (dFloat) options->mass, dstTM, boxCollision, matId );
	
	NewtonDestroyCollision(boxCollision);

	return pBodyEnt;
}