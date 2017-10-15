
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: facialRetargeting_helper.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "facialRetargeting_constraint.h"

// #define PRINT_RAY_INTERSECT_INFO

FBModel *ConstraintFacialRetargeting::FindParent(FBModel *pModel)
{
	FBModel *pParent = pModel->Parent;
	if (pParent)
		return FindParent(pParent);
	
	return pModel;
}

void ConstraintFacialRetargeting::CollectModels(FBModelList *pList, FBModel *pModel)
{
	pList->Add(pModel);

	for (int i=0; i<pModel->Children.GetCount(); ++i)
	{
		CollectModels(pList, pModel->Children[i]);
	}
}

void ConstraintFacialRetargeting::FixMappingName(FBString &name)
{
	char *str = name;

	for (int i=0; i<name.GetLen(); ++i)
		if (str[i] == '.' || str[i] == '-')
		{
			str[i] = '_';
		}
}

FBModel *ConstraintFacialRetargeting::FindModelInList(FBModelList *pList, const char *name, const bool fixNaming)
{
	for (int i=0; i<pList->GetCount(); ++i)
	{
		FBString modelName(pList->GetAt(i)->Name);

		if (fixNaming)
			FixMappingName(modelName);

		if (strcmp(modelName, name) == 0)
			return pList->GetAt(i);
	}

	return nullptr;
}

void ConstraintFacialRetargeting::ReadModelMatrix(FBEvaluateInfo *pEvaluateInfo, CollisionAnimNodes &nodes, FBMatrix &lResult)
{
	if (nodes.tr == nullptr || nodes.rot == nullptr || nodes.scl == nullptr)
		return;

	double lPosition[3], lRotation[3], lScaling[3];
	nodes.tr->ReadData		( lPosition, pEvaluateInfo );
	nodes.rot->ReadData		( lRotation, pEvaluateInfo );
	nodes.scl->ReadData		( lScaling, pEvaluateInfo );
	
	FBTRSToMatrix( lResult, FBTVector(lPosition[0], lPosition[1], lPosition[2], 1.0), FBRVector(lRotation), FBSVector(lScaling) );
}

bool ConstraintFacialRetargeting::UnProjectPoint(const char *debugName, const FBVector3d &inputCoords, 
	const double lDepth, const FBMatrix &invCamMVP, const FBVector3d &cameraPos, FBModel *collisionModel, const FBMatrix &collisionMatrix,
	FBTVector &result)
{
	bool lStatus = false;
	double lPosition[3];

	lPosition[0] = 0.01 * inputCoords[0] * 2.0 - 1.0;
	lPosition[1] = 0.01 * inputCoords[1] * 2.0 - 1.0;
	lPosition[2] = lDepth;

	FBVectorMatrixMult(result, invCamMVP, FBTVector(lPosition[0], lPosition[1], lPosition[2], 1.0));

	if (result[3] != 0.0)
	{
		result[0] /= result[3];
		result[1] /= result[3];
		result[2] /= result[3];
		result[3] = 1.0;
	}

	// find closest point if meshes assigned

	if (collisionModel != nullptr)
	{
		// mesh & tri intersection point
					
		FBMatrix m, mInv;
		//collisionModel->GetMatrix(m, kModelTransformation_Geometry);
		m = collisionMatrix;
		FBMatrixInverse(mInv, m);

		FBTVector rayOrigin(cameraPos[0], cameraPos[1], cameraPos[2], 1.0);
		FBTVector rayEnd(result[0], result[1], result[2], 1.0);
		FBTVector pos, n;

		FBVectorMatrixMult(rayOrigin, mInv, rayOrigin);
		FBVectorMatrixMult(rayEnd, mInv, rayEnd);

		FBString colName = collisionModel->LongName;
		/*
		if ( false == collisionModel->IsEvaluationReady(kFBModelEvaluationDeform) )
		{
			FBTrace( "collision deform is not ready - %s\n", colName );
		}
		if (false == collisionModel->IsEvaluationReady(kFBModelEvaluationTranform) )
		{
			FBTrace( "collision transform is not ready - %s\n", colName );
		}
		*/
		if (true == collisionModel->ClosestRayIntersection(rayOrigin, rayEnd, pos, (FBNormal&)n))
		{
			FBVectorMatrixMult(result, m, pos);
			lStatus = true;
		}
		else
		{
#ifdef PRINT_RAY_INTERSECT_INFO

			
			
			FBTrace( "failed to find a closest ray intersection between %s and %s!\n", colName, debugName );
#endif
		}
	}

	return lStatus;
}

bool ConstraintFacialRetargeting::PointCollideWith(FBEvaluateInfo *pInfo, FBModel *pColModel, const FBMatrix &colMatrix, const double thickness, FBVector4d &point)
{
	bool lStatus = false;

	FBVector3d vmin, vmax;
	pColModel->GetBoundingBox(vmin, vmax);
	FBVector4d c(vmin[0]+0.5*(vmax[0]-vmin[0]), vmin[1]+0.5*(vmax[1]-vmin[1]), vmin[2]+0.5*(vmax[2]-vmin[2]), 1.0);

	FBMatrix m, mInv;
	// TODO: read tr, rot and scl from animation nodes !
	pColModel->GetMatrix(m, kModelTransformation, true, pInfo);
	//m = colMatrix;
	FBMatrixInverse(mInv, m);

	FBVector4d rayOrigin(point[0], point[1], point[2], 1.0);
	FBVector4d rayEnd;
	FBVector4d pos, n;

	FBVectorMatrixMult(rayEnd, m, c);
	
	FBVector4d diff, dir;
	FBSub(diff, rayEnd, rayOrigin);
	double len = FBLength(diff);
	FBMult(dir, diff, 1.0 / len);
	FBMult(diff, diff, 3.0);
	FBAdd(rayEnd, rayOrigin, diff);

	FBVectorMatrixMult(rayOrigin, mInv, rayOrigin);
	FBVectorMatrixMult(rayEnd, mInv, rayEnd);

	if (true == pColModel->ClosestRayIntersection(rayOrigin, rayEnd, pos, (FBNormal&)n))
	{
		FBVectorMatrixMult(pos, m, pos);

		FBSub(diff, pos, rayEnd);
		double resLen = FBLength(diff);

		if (len < resLen + thickness)
		{
			point[0] = pos[0] + dir[0] * thickness;
			point[1] = pos[1] + dir[1] * thickness;
			point[2] = pos[2] + dir[2] * thickness;
		}

		lStatus = true;
	}
	else
	{
#ifdef PRINT_RAY_INTERSECT_INFO
		printf( "failed to find a closest ray intersection!\n" );
#endif
	}

	return lStatus;
}