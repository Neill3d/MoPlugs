
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: pathwrap_constraint.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "pathwrap_constraint.h"
#include "algorithm\math3d_mobu.h"

//--- Registration defines
#define	ORCONSTRAINTPATHWRAP__CLASS		ORCONSTRAINTPATHWRAP__CLASSNAME
#define ORCONSTRAINTPATHWRAP__NAME		"Path Wrap"
#define ORCONSTRAINTPATHWRAP__LABEL		"Path Wrap"
#define ORCONSTRAINTPATHWRAP__DESC		"Path Wrap"

#define	ORCONSTRAINTPATHTEST__CLASS		ORCONSTRAINTPATHTEST__CLASSNAME
#define ORCONSTRAINTPATHTEST__NAME		"Path Test"
#define ORCONSTRAINTPATHTEST__LABEL		"Path Test"
#define ORCONSTRAINTPATHTEST__DESC		"Path Test"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINTPATHWRAP__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINTPATHWRAP__NAME,
								ORCONSTRAINTPATHWRAP__CLASS,
								ORCONSTRAINTPATHWRAP__LABEL,
								ORCONSTRAINTPATHWRAP__DESC,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)

FBConstraintImplementation	(	ORCONSTRAINTPATHTEST__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINTPATHTEST__NAME,
								ORCONSTRAINTPATHTEST__CLASS,
								ORCONSTRAINTPATHTEST__LABEL,
								ORCONSTRAINTPATHTEST__DESC,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)


////////////////////////////////////////////////////////////////////////////// PATH TEST

/////////////////////////////////////////////////////////////////////////////////////
// path wrap deformer constraint

bool ORConstraintPathTest::FBCreate()
{
	// public properties
	FBPropertyPublish( this, Percent, "Percent", nullptr, nullptr );
	
	Percent.SetMinMax( 0.0, 100.0, true, true );

	// Create reference groups
	mGroupCurve			= ReferenceGroupAdd( "Path",	1 );
	mGroupConstrained	= ReferenceGroupAdd( "Constrained",	1 );
	
	Deformer = false;
	HasLayout = false;

	return true;
}

void ORConstraintPathTest::FBDestroy()
{
}

void ORConstraintPathTest::SnapSuggested()
{
	// TODO:

	if (Active == false)
	{
		if (ReferenceGetCount(mGroupConstrained) > 0)
			ReferenceGet(mGroupConstrained)->GetMatrix( mInitialTM );
	}
}

void ORConstraintPathTest::SetupAllAnimationNodes()
{
	// TODO:
	if ( ReferenceGetCount(mGroupCurve) > 0 && ReferenceGetCount(mGroupConstrained) > 0 )
	{
		mSourceCurve		= AnimationNodeOutCreate( 0, ReferenceGet( mGroupCurve,	0 ), ANIMATIONNODE_TYPE_TRANSLATION );

		mConstraintPosition = AnimationNodeInCreate	( 1, ReferenceGet( mGroupConstrained,	0 ), ANIMATIONNODE_TYPE_TRANSLATION );
		mConstraintRotation = AnimationNodeInCreate	( 2, ReferenceGet( mGroupConstrained,	0 ), ANIMATIONNODE_TYPE_ROTATION );
	}
}

/** Notification callback for connectors.
	*	\param	pAnimationNode		Animation node being notified.
	*	\param	pEvaluateInfo		Information for evaluation.
	*	\param	pConstraintInfo		Information for constraint.
	*	\return \b true if successful.
	*/

void Normalize(FBVector4d &v)
{
	double len = FBLength(v);
	if (len != 0.0)
		FBMult( v, v, 1.0 / len );
}

void GetPathRotation( FBQuaternion &qResult, FBModelPath3D *pPath, const double percent, const FBVector4d &up )
{
	FBVector4d tangent = pPath->Total_GlobalPathEvaluateDerivative( percent );
	Normalize(tangent);

	FBVector4d lUp(up);
	Normalize(lUp);

	FBVector4d axis;
	FBMult( axis, lUp, tangent );
	Normalize(axis);

	double radians = acos( FBDot( lUp, tangent ) );
	qResult = UnitAxisToQuaternion( FBVector3d(axis), radians );
}

void GetPathRotation2( FBQuaternion &qResult, FBModelPath3D *pPath, const double percent, const FBVector4d &up )
{
	FBVector4d prevTangent = pPath->Total_GlobalPathEvaluateDerivative( percent - 10.0 );
	FBVector4d tangent = pPath->Total_GlobalPathEvaluateDerivative( percent );

	Normalize(prevTangent);
	Normalize(tangent);

	FBVector4d normal;
	FBMult( normal, tangent, up );

	FBVector4d perp;
	FBMult( perp, prevTangent, tangent );
	double perpLen = FBLength(perp);

	double cosBetweenTangents = FBDot(tangent, prevTangent );
	double angleBetweenTans = acos(cosBetweenTangents);
	double angle = atan2( perpLen, cosBetweenTangents);

	Normalize(perp);
	qResult = UnitAxisToQuaternion( FBVector3d(perp), angle );
}


bool ORConstraintPathTest::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo )
{

	double lPosition[3];

	bool lStatus = mSourceCurve->ReadData( lPosition, pEvaluateInfo );

	if (lStatus == false)
		return false;

	FBModelPath3D *pPath = (FBModelPath3D*) ReferenceGet(mGroupCurve);

	double lPercent = 0.0;
	Percent.GetData( &lPercent, sizeof(double), pEvaluateInfo );

	FBVector4d pos, der, nextpos;
	
	pos = pPath->Total_GlobalPathEvaluate(0.0);
	der = pPath->Total_GlobalPathEvaluateDerivative(0.0);
	/*
	double len = FBLength(der);
	if (len > 0.0)
		FBMult(der, der, 1.0 / len );
		*/
	FBMatrix initialRotMatrix;
	//initialRotMatrix = NormalToRotation(der);
	//LookAt(FBVector3d(pos), FBVector3d(pos[0]+der[0], pos[1]+der[1], pos[2]+der[2]), FBVector3d(0.0, 0.0, 1.0), initialRotMatrix);

	FBQuaternion initialQuat;
	GetPathRotation( initialQuat, pPath, 0.0, FBVector4d(0.0, 1.0, 0.0, 0.0) );

	FBQuaternionToMatrix( initialRotMatrix, initialQuat );

	pos = pPath->Total_GlobalPathEvaluate(lPercent);
	der = pPath->Total_GlobalPathEvaluateDerivative(lPercent);
	/*
	len = FBLength(der);
	if (len > 0.0)
		FBMult(der, der, 1.0 / len );
		*/
	FBMatrix rotMatrix;
	//rotMatrix = NormalToRotation(der);
	LookAt(FBVector3d(0.0, 0.0, 0.0), FBVector3d(der[0], der[1], der[2]), FBVector3d(0.0, 0.0, 1.0), rotMatrix);

	//angle = 90.0 + 180.0 / 3.1415 * atan2(der[0], der[1]);
	//FBTRSToMatrix( rotMatrix, FBVector4d(0.0, 0.0, 0.0, 1.0), FBVector3d(0.0, 0.0, angle), FBSVector(1.0, 1.0, 1.0) );
	
	FBQuaternion rotQuat;
	GetPathRotation( rotQuat, pPath, lPercent, FBVector4d(0.0, 1.0, 0.0, 0.0) );

	FBRVector eulerRotation;

	
	FBQuaternionToMatrix( rotMatrix, rotQuat );
	
	FBMatrix localRotMatrix;
	FBGetLocalMatrix( localRotMatrix, initialRotMatrix, rotMatrix );

	FBMatrix curTM;
	FBMatrixMult( curTM, mInitialTM, localRotMatrix );

	FBMatrixToRotation( eulerRotation, curTM );
	
	//FBQuaternionToRotation( eulerRotation, rotQuat, kFBXYZ );
	

	//FBRVector eulerRotation;
	//FBMatrixToRotation( eulerRotation, rotMatrix, kFBZYX );

	//FBModel *pModel = ReferenceGet(mGroupConstrained);
	//pModel->SetMatrix( rotMatrix, kModelRotation );

	//pModel->GetVector( eulerRotation, kModelRotation, false );

	FBVector4d v;
	FBAdd( v, pos, der );

	mConstraintPosition->WriteData( pos, pEvaluateInfo );
	mConstraintRotation->WriteData( eulerRotation, pEvaluateInfo );

	return true;
}


////////////////////////////////////////////////////////////////////////////// PATH WRAP

/////////////////////////////////////////////////////////////////////////////////////
// path wrap deformer constraint

bool ORConstraintPathWrap::FBCreate()
{
	// public properties
	FBPropertyPublish( this, CurveSubdivisions, "Curve Subdivisions", nullptr, nullptr );
	FBPropertyPublish( this, CurveSegmentSubdivisions, "Curve Segment Subdivisions", nullptr, nullptr );
	FBPropertyPublish( this, ControlAxis, "Control Axis", nullptr, nullptr );

	CurveSubdivisions.SetMinMax(1.0, 16192.0, true, true);
	CurveSubdivisions = 4096;

	CurveSegmentSubdivisions.SetMinMax(1.0, 8096.0, true, true);
	CurveSegmentSubdivisions = 32;

	ControlAxis.SetMinMax(1.0, 3.0, true, true);
	ControlAxis = 3.0;

	// Create reference groups
	mGroupCurve	= ReferenceGroupAdd( "Path",	1 );
	mGroupDeformed	= ReferenceGroupAdd( "Deformed",	MAX_NUMBER_OF_BRICKS );
	
	Deformer = true;

	return true;
}

void ORConstraintPathWrap::FBDestroy()
{
}

void ORConstraintPathWrap::ComputeModelData( FBModel *pModel, ElementData *pData )
{
	pData->cached = false;
	pData->creationPercent = 0.0;
	pData->length = 0.0;
		
	//FBMatrix scalingMat;
	//pModel->GetVector( pData->scaling, kModelScaling );
	//pModel->GetMatrix( scalingMat, kModelScaling );

	pModel->GetVector( pData->translation, kModelTranslation, false );
	pModel->GetVector( pData->rotation, kModelRotation, false );
	pModel->GetVector( pData->scaling, kModelScaling, false );

	FBMatrix originMat, localMat;
	pModel->GetMatrix( originMat, kModelTransformation_Geometry );
	pModel->GetMatrix( localMat, kModelTransformation, false );

	FBVector3d vmin, vmax;
	pModel->GetBoundingBox(vmin, vmax);

	FBVector4d vmin4, vmax4;
		
	FBVectorMatrixMult(vmin4, originMat, FBVector4d(vmin[0], vmin[1], vmin[2], 1.0));
	FBVectorMatrixMult(vmax4, originMat, FBVector4d(vmax[0], vmax[1], vmax[2], 1.0));


	// not used
	switch(ControlAxis)
	{
	case 1:
		pData->length = vmax4[0] - vmin4[0];
		pData->bbMin = vmin4[0];
		break;
	case 2:
		pData->length = vmax4[1] - vmin4[1];
		pData->bbMin = vmin4[1];
		break;
	case 3:
		pData->length = vmax4[2] - vmin4[2];
		pData->bbMin = vmin4[2];
		break;
	default:
		pData->length = vmax4[0] - vmin4[0];
		pData->bbMin = vmin4[0];
		break;
	}

	pData->originTM = originMat;
	pData->localTM = localMat;
}

bool ORConstraintPathWrap::ReferenceAddNotify		( int pGroupIndex, FBModel* pModel )
{
	if (pGroupIndex == mGroupDeformed && ReferenceGetCount(mGroupCurve) > 0)
	{
		ElementData	*pData = new ElementData;
		pData->needCompute = true;
		pData->cached = false;

		mModelsData[pModel] = pData;
		return DeformerBind(pModel);
	}

	if (pGroupIndex == mGroupCurve)
		return FBIS(pModel, FBModelPath3D);

	return false;
}

bool ORConstraintPathWrap::ReferenceRemoveNotify	( int pGroupIndex, FBModel* pModel )
{
	if (pGroupIndex == mGroupDeformed)
	{
		auto iter = mModelsData.find(pModel);
		if (iter != end(mModelsData) )
		{
			ElementData *pData = iter->second;
			if (pData)
			{
				delete pData;
				pData = nullptr;
			}
		}

		mModelsData.erase(pModel);
		return DeformerUnBind(pModel);
	}

	return true;
}

void ORConstraintPathWrap::SnapSuggested()
{
	if (Active == false && ReferenceGetCount(mGroupCurve) > 0)
	{
		for (int i=0, count=ReferenceGetCount(mGroupDeformed); i<count; ++i)
		{
			FBModel *pModel = ReferenceGet(mGroupDeformed, i);
			auto iter = mModelsData.find(pModel);
			if (iter != end(mModelsData))
			{
				ElementData *pData = iter->second;
				pData->needCompute = true;
				pData->cached = false;
			}
		}
	}
}

bool ORConstraintPathWrap::DeformerNotify(	FBModel* pModel, const FBVertex*  pSrcVertex, const FBVertex* pSrcNormal,
									int pCount, 
									FBVertex*  pDstVertex,FBVertex*  pDstNormal)
{

	if ( ReferenceGetCount(mGroupCurve) == 0 || !FBIS(ReferenceGet(mGroupCurve), FBModelPath3D) )
		return false;

	auto iter = mModelsData.find(pModel);
	if (iter == end(mModelsData) )
		return false;

	FBModelPath3D *pPath = (FBModelPath3D*) ReferenceGet(mGroupCurve);
	double curveLen = pPath->PathLength;

	ElementData *pData = iter->second;

	//FBMatrix invMatrix;
	//pModel->GetMatrix( invMatrix, kModelInverse_Transformation_Geometry );
	/*
	invMatrix[12] = 0.0;
	invMatrix[13] = 0.0;
	invMatrix[14] = 0.0;
	*/
	if (pData->cached)
	{
		if (pData->cache.size() != pCount)
		{
			pData->cached = false;
		}
		else
		{
			memcpy( pDstVertex, pData->cache.data(), sizeof(FBVertex) * pCount );
		}
	}
	else
	{
		if (pData->needCompute)
		{
			ComputeModelData(pModel, pData);

			// calculate closest point on a curve and creation percent
			FBVector3d pos;
			FBVector4d pointOnCurve;
			pModel->GetVector(pos);

			if (false == FindClosestPointOnCurve(pointOnCurve, pData->creationPercent, pPath, FBVector4d(pos[0], pos[1], pos[2], 1.0), CurveSubdivisions, CurveSegmentSubdivisions ) )
			{
				printf( "[PATH WRAP] failed to find a closest point on a curve!\n" );
			}
			pData->needCompute = false;
		}

		//FBMatrix scalingMat;
		//FBScalingToMatrix( scalingMat, FBSVector(pData->scaling) );

		FBMatrix originMat( pData->originTM );
		
		//pModel->GetMatrix( originMat );
		FBMatrix invMatrix; // = originMat.Inverse();
		FBMatrixInverse( invMatrix, originMat );


		FBMatrix localMat;
		FBTRSToMatrix( localMat, FBTVector(pData->translation[0], pData->translation[1], pData->translation[2], 1.0),
			FBRVector(pData->rotation[0], pData->rotation[1], pData->rotation[2]),
			FBSVector(pData->scaling[0], pData->scaling[1], pData->scaling[2]) );
		FBMatrix localInvMat;
		
		FBMatrix scalingMat;
		FBScalingToMatrix( scalingMat, FBSVector(pData->scaling[0], pData->scaling[1], pData->scaling[2]) );
		
		FBVector4d pivotOffset;
		pivotOffset = FBVector4d(localMat[12], localMat[13], localMat[14]);

		localMat[12] = 0.0;
		localMat[13] = 0.0;
		localMat[14] = 0.0;
		
		FBMatrixInverse( localInvMat, localMat );

		pData->cache.resize(pCount);

		FBVector4d originPos, pos, der;
		FBVector4d localOriginPos, localPos;

		originPos = pPath->Total_GlobalPathEvaluate( pData->creationPercent );
		der = pPath->Total_GlobalPathEvaluateDerivative( pData->creationPercent );
    
		FBVectorMatrixMult( localOriginPos, invMatrix, originPos );
		FBVectorMatrixMult( localOriginPos, localMat, localOriginPos );

		

		//pivotOffset = FBVector4d(originMat[12]-originPos[0], originMat[13]-originPos[1], originMat[14]-originPos[2]);
		

		FBMatrix pivotTM;
		//LookAt( FBVector3d(pos), FBVector3d(pos[0]+der[0], pos[1]+der[1], pos[2]+der[2]), FBVector3d(0.0, 1.0, 0.0), pivotTM );
		
		FBTVector nor(der);
		double len = FBLength(nor);
		if (len > 0.0)
			FBMult( nor, nor, 1.0 / len );

		FBQuaternion q;
		GetPathRotation( q, pPath, pData->creationPercent, FBVector4d(0.0, 1.0, 0.0, 1.0) );
		FBQuaternionToMatrix( pivotTM, q );

		FBRVector pivotRotation;
		FBMatrixToRotation( pivotRotation, pivotTM );

		for (int i=0; i<pCount; ++i)
		{
			const FBVertex &vertex = pSrcVertex[i];

			// apply only scaling, but still not translation and rotation
			FBVertex transformedVertex;
			FBVertexMatrixMult( transformedVertex, localMat, vertex );

			// this is when we have direction in -X    
			double vertexLen, vertexF;
			double temp=0.0;

			switch(ControlAxis)
			{
			case 1:
				vertexLen = transformedVertex[0];
				vertexF = pData->creationPercent + 100.0 * vertexLen / curveLen;
    
				transformedVertex[0] = 0.0;
				break;
			case 2:
				vertexLen = transformedVertex[1];
				vertexF = pData->creationPercent + 100.0 * vertexLen / curveLen;
    
				temp = transformedVertex[1];
				transformedVertex[1] = 0.0;
				break;
			case 3:
				vertexLen = transformedVertex[2]; 
				vertexF = pData->creationPercent + 100.0 * vertexLen / curveLen;
    
				transformedVertex[2] = 0.0;
				break;
			default:
				vertexLen = transformedVertex[0]; 
				vertexF = pData->creationPercent + 100.0 * vertexLen / curveLen;
    
				transformedVertex[0] = 0.0;
			}

			FBVertexMatrixMult( transformedVertex, scalingMat, vertex );
			transformedVertex[0] = 0.0;

			//FBVertexMatrixMult( transformedVertex, localInvMat, transformedVertex );
			//FBVertexMatrixMult( transformedVertex, originMat, transformedVertex );
			/*
			transformedVertex[0] -= originPos[0];
			transformedVertex[1] -= originPos[1];
			transformedVertex[2] -= originPos[2];
			*/
			pos = pPath->Total_GlobalPathEvaluate( vertexF );
			der = pPath->Total_GlobalPathEvaluateDerivative( vertexF );
			nor = der;

			FBVectorMatrixMult( localPos, invMatrix, pos );
			FBVectorMatrixMult( localPos, localMat, localPos );

			FBVector4d diff;
			FBSub( diff, pos, originPos );
			/*
			transformedVertex[0] -= pivotOffset[0];
			transformedVertex[1] -= pivotOffset[1];
			transformedVertex[2] -= pivotOffset[2];
			*/

			len = FBLength(nor);
			if (len > 0.0)
				FBMult( nor, nor, 1.0 / len );
				
			FBMatrix curTM;
			
			GetPathRotation( q, pPath, vertexF, FBVector4d(0.0, 1.0, 0.0, 1.0) );
			FBQuaternionToMatrix( curTM, q );

			//LookAt( FBVector3d(pos), FBVector3d(pos[0]+der[0], pos[1]+der[1], pos[2]+der[2]), FBVector3d(0.0, 1.0, 0.0), curTM );
			/*
			curTM = NormalToRotation(nor);

			FBRVector vertexRotation;
			FBMatrixToRotation( vertexRotation, curTM );

			curTM[12] = 0.0;
			curTM[13] = 0.0;
			curTM[14] = 0.0;
			*/
			//FBGetLocalMatrix( curTM, pivotTM, curTM );
			
			curTM[12] = 0.0;
			curTM[13] = 0.0;
			curTM[14] = 0.0;
			
			FBVertex rotatedVertex;
			FBVertexMatrixMult( rotatedVertex, curTM, transformedVertex );
			//rotatedVertex = transformedVertex;
			
			
			FBVertex &dstVertex = pDstVertex[i];
			
			//dstVertex = rotatedVertex;
			//dstVertex[1] += temp;

			rotatedVertex[0] += pos[0];
			rotatedVertex[1] += pos[1];
			rotatedVertex[2] += pos[2];

			//dstVertex[1] = temp * 0.5;
			/*
			dstVertex[0] += pData->originTM[12];
			dstVertex[1] += pData->originTM[14];
			dstVertex[2] += pData->originTM[13];
			*/
			//dstVertex = FBVertex( pos[0]+rotatedVertex[0], pos[1]+rotatedVertex[1], pos[2]+rotatedVertex[2], 1.0f );
			

			FBVertexMatrixMult( dstVertex, invMatrix, rotatedVertex );
			//FBVertexMatrixMult( dstVertex, localInvMat, dstVertex );

			pData->cache[i] = dstVertex;
		}

		pData->cached = true;
	}

	return true;
}