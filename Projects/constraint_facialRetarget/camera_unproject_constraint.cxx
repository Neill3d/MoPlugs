
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: camera_unproject_constraint.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declarations
#include "camera_unproject_constraint.h"
#include "algorithm\math3d_mobu.h"

//--- Registration defines
#define	ORCONSTRAINTPOSITION__CLASS		ORCONSTRAINTUNPROJECT__CLASSNAME
#define ORCONSTRAINTPOSITION__NAME		"UnProject Points"
#define ORCONSTRAINTPOSITION__LABEL		"UnProject Points"
#define ORCONSTRAINTPOSITION__DESC		"UnProject Points"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINTPOSITION__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINTPOSITION__NAME,
								ORCONSTRAINTPOSITION__CLASS,
								ORCONSTRAINTPOSITION__LABEL,
								ORCONSTRAINTPOSITION__DESC,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)

static void UnProject_CreateDestination(HIObject pObject, bool value) {
	ConstraintCameraUnProject *pplug = FBCast<ConstraintCameraUnProject>(pObject);
	if (value && pplug) pplug->DoCreateDestination();
}

/************************************************
 *	Creation function.
 ************************************************/
bool ConstraintCameraUnProject::FBCreate()
{
	Deformer = true;
	HasLayout = false;
	Description = "UnProject Points";

	FBPropertyPublish( this, Camera, "Camera", nullptr, nullptr );
	FBPropertyPublish( this, Meshes, "Meshes", nullptr, nullptr );
	FBPropertyPublish( this, UnProjectDepth, "UnProject Depth", nullptr, nullptr );
	FBPropertyPublish( this, CreateDestination, "Create Destination", nullptr, UnProject_CreateDestination );

	Camera.SetSingleConnect(true);
	Camera.SetFilter(FBCamera::GetInternalClassId() );

	Meshes.SetSingleConnect(false);
	Meshes.SetFilter(FBModel::GetInternalClassId() );

	UnProjectDepth.SetMinMax(0.0, 100.0, true, true);
	UnProjectDepth = 10.0;

	// Create reference group
	mGroupSource	= ReferenceGroupAdd( "Source Objects",	MAX_NUMBER_OF_NODES );
	mGroupConstrain	= ReferenceGroupAdd( "Constrain",		MAX_NUMBER_OF_NODES );

	
	for (int i = 0; i < MAX_NUMBER_OF_NODES; ++i)
	{
		mSourceTranslation[i] = nullptr;
		mConstrainedTranslation[i] = nullptr;
	}

	return true;
}


/************************************************
 *	Destruction function.
 ************************************************/
void ConstraintCameraUnProject::FBDestroy()
{
}


/************************************************
 *	Setup all of the animation nodes.
 ************************************************/
void ConstraintCameraUnProject::SetupAllAnimationNodes()
{
	if (ReferenceGetCount( mGroupSource ) > 0 && (ReferenceGetCount(mGroupSource) ==  ReferenceGetCount( mGroupConstrain )) )
	{
		for (int i=0; i<ReferenceGetCount(mGroupSource); ++i)
		{
			mSourceTranslation[i]		= AnimationNodeOutCreate( i*2, ReferenceGet( mGroupSource,	i ), ANIMATIONNODE_TYPE_TRANSLATION );
			mConstrainedTranslation[i] = AnimationNodeInCreate	( i*2+1, ReferenceGet( mGroupConstrain, i ), ANIMATIONNODE_TYPE_TRANSLATION );
		}

	}
}


void ConstraintCameraUnProject::DoCreateDestination()
{
	const int sourceCount = ReferenceGetCount(mGroupSource);
	if (sourceCount == 0)
		return;

	//mSystem.BeginChange();

	if (ReferenceGetCount(mGroupConstrain) > 0)
	{
		for (int i=ReferenceGetCount(mGroupConstrain)-1; i>= 0; --i)
		{
			ReferenceRemove(mGroupConstrain, ReferenceGet(mGroupConstrain, i));
		}
	}

	for (int i=0; i<sourceCount; ++i)
	{
		FBString name = ReferenceGet(mGroupSource, i)->Name;
		FBModelNull *pNewNull = new FBModelNull( FBString(name, "_dst") );
		pNewNull->Show = true;

		FBProperty *pNewProp = pNewNull->PropertyCreate("Landmark", kFBPT_charptr, "CHARPTR", false, true );
		if (pNewProp)
			pNewProp->SetString(name);

		ReferenceAdd(mGroupConstrain, pNewNull);
	}

	//mSystem.EndChange();
}

/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void ConstraintCameraUnProject::RemoveAllAnimationNodes()
{
}


/************************************************
 *	FBX storage of constraint parameters.
 ************************************************/
bool ConstraintCameraUnProject::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}


/************************************************
 *	FBX retrieval of constraint parameters.
 ************************************************/
bool ConstraintCameraUnProject::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}


/************************************************
 *	Suggest a freeze.
 ************************************************/
void ConstraintCameraUnProject::FreezeSuggested()
{
	
	FBConstraint::FreezeSuggested();

	if( ReferenceGet( 0,0 ) )
	{
		FreezeSRT( (FBModel*)ReferenceGet( 0, 0), true, true, true );
	}
}


/************************************************
 *	Real-Time Engine Evaluation
 ************************************************/
bool ConstraintCameraUnProject::AnimationNodeNotify(FBAnimationNode* pConnector, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo)
{
	FBVector3d lCameraPos;
	FBMatrix lCamM, lCamInvM;

	const double lDepth = 0.01 * UnProjectDepth;
	double lPosition[3];
	FBTVector lResult;

	if (Camera.GetCount() == 0)
		return false;

	FBCamera *pCamera = (FBCamera*) Camera.GetAt(0);
	pCamera->GetVector(lCameraPos);

	pCamera->GetCameraMatrix( lCamM, kFBModelViewProj );
	FBMatrixInverse( lCamInvM, lCamM );

	for (int i=0; i<ReferenceGetCount(mGroupSource); ++i)
	{
		if (mSourceTranslation[i] && mConstrainedTranslation[i])
		{
			mSourceTranslation[i]->ReadData		( lPosition, pEvaluateInfo );

			lPosition[0] = 0.01 * lPosition[0] * 2.0 - 1.0;
			lPosition[1] = 0.01 * lPosition[1] * 2.0 - 1.0;
			lPosition[2] = lDepth;

			FBVectorMatrixMult(lResult, lCamInvM, FBTVector(lPosition[0], lPosition[1], lPosition[2], 1.0));

			if (lResult[3] != 0.0)
			{
				lResult[0] /= lResult[3];
				lResult[1] /= lResult[3];
				lResult[2] /= lResult[3];
				lResult[3] = 1.0;
			}

			// find closest point if meshes assigned

			if (Meshes.GetCount() > 0)
			{

				for (int j=0; j<Meshes.GetCount(); ++j)
				{
					// mesh & tri intersection point
					FBModel *pModel = (FBModel*) Meshes.GetAt(j);

					FBMatrix m, mInv;
					pModel->GetMatrix(m, kModelTransformation_Geometry);
					FBMatrixInverse(mInv, m);

					FBTVector rayOrigin(lCameraPos[0], lCameraPos[1], lCameraPos[2], 1.0);
					FBTVector rayEnd(lResult[0], lResult[1], lResult[2], 1.0);
					FBTVector pos, n;

					FBVectorMatrixMult(rayOrigin, mInv, rayOrigin);
					FBVectorMatrixMult(rayEnd, mInv, rayEnd);

					if (true == pModel->ClosestRayIntersection(rayOrigin, rayEnd, pos, (FBNormal&)n))
					{
						FBVectorMatrixMult(lResult, m, pos);
						break;
					}
				}
			}

			mConstrainedTranslation[i]->WriteData	( lResult, pEvaluateInfo );
		}
	}

	return true;
}
