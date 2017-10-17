
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: aimSprite_constraint.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "aimSprite_constraint.h"
#include "algorithm\math3d_mobu.h"

#include <math.h>
#include <cmath>
#include <limits>
#include <float.h>

//--- Registration defines
#define	ORCONSTRAINTPOSITION__CLASS		ORCONSTRAINTPOSITION__CLASSNAME
#define ORCONSTRAINTPOSITION__NAME		"Aim Sprite"
#define ORCONSTRAINTPOSITION__LABEL		"Aim Sprite"
#define ORCONSTRAINTPOSITION__DESC		"Aim Sprite Constraint"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINTPOSITION__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINTPOSITION__NAME,
								ORCONSTRAINTPOSITION__CLASS,
								ORCONSTRAINTPOSITION__LABEL,
								ORCONSTRAINTPOSITION__DESC,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)



/************************************************
 *	Creation function.
 ************************************************/

void ConstraintAimSprite::ActionGetOffsetFromSel(HIObject pObject, bool value)
{
	ConstraintAimSprite *p = FBCast<ConstraintAimSprite>(pObject);
	if (p && value)
	{
		p->OnGetOffsetFromSelClick();
	}
}

bool ConstraintAimSprite::FBCreate()
{
	// Create reference groups
	mGroupConstrain	= ReferenceGroupAdd( "Constrain",		MAX_NUMBER_OF_NODES );

	//
	//

	FBPropertyPublish( this, Type, "Type", nullptr, nullptr );
	FBPropertyPublish( this, Up, "Up Vector", nullptr, nullptr );

	Type = kFBAimCylidrical;
	Up = FBVector3d(0.0, 1.0, 0.0);

	//
	FBPropertyPublish(this, FirstOffsetForAll, "First Offset For All", nullptr, nullptr);
	FBPropertyPublish(this, GetOffsetFromSel, "Get Offset From Sel", NULL, ActionGetOffsetFromSel);

	//
	FirstOffsetForAll = true;
	mLastTime = FBTime::MinusInfinity;

	return true;
}


/************************************************
 *	Destruction function.
 ************************************************/
void ConstraintAimSprite::FBDestroy()
{
}


/************************************************
 *	Setup all of the animation nodes.
 ************************************************/
void ConstraintAimSprite::SetupAllAnimationNodes()
{
	int count = ReferenceGetCount( mGroupConstrain );
	if (/*ReferenceGet( mGroupSource, 0 ) &&*/ count > 0 )
	{
		mSourceTranslation.resize(count);
		mConstraintedRotation.resize(count);
		//mOffsets.resize(count);

		for (int i=0; i<count; ++i)
		{
			FBModel *pModel = ReferenceGet( mGroupConstrain, i );
			mSourceTranslation[i] = AnimationNodeOutCreate( i*2, pModel, ANIMATIONNODE_TYPE_TRANSLATION );
			mConstraintedRotation[i] = AnimationNodeInCreate	( i*2+1, pModel, ANIMATIONNODE_TYPE_ROTATION );

			//mOffsets[i].Identity();
		}
	}
}

void ConstraintAimSprite::OnGetOffsetFromSelClick()
{
	FBModelList	modelList;

	FBGetSelectedModels(modelList);

	if (modelList.GetCount() > 0)
	{
		FBModel *pModel = modelList[0];
		FBMatrix mat;
		mat.Identity();
		pModel->GetMatrix( mat, kModelRotation );

		for (int i=0; i<(int)mOffsets.size(); ++i)
		{
			mOffsets[i] = mat;	
		}
	}
}

/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void ConstraintAimSprite::RemoveAllAnimationNodes()
{
}


/************************************************
 *	FBX storage of constraint parameters.
 ************************************************/
bool ConstraintAimSprite::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxStore(pFbxObject, pStoreWhat);
	
	if (pStoreWhat == kAttributes)
	{
		pFbxObject->FieldWriteI( "count", (int) mOffsets.size() );

		char buffer[64];


		for (auto iter=begin(mOffsets); iter!=end(mOffsets); ++iter)
		{
			FBMatrix &mat = *iter;
			for (int i=0; i<16; ++i)
			{
				memset( buffer, 0, sizeof(char) * 64);
				sprintf_s( buffer, 64, "offsetTM%d", i );
				pFbxObject->FieldWriteD( buffer, mat[i]);
			}
		}
	}

	return true;
}


/************************************************
 *	FBX retrieval of constraint parameters.
 ************************************************/
bool ConstraintAimSprite::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxStore(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAttributes)
	{
		int count = pFbxObject->FieldReadI( "count" );

		if (mOffsets.size() != count)
			mOffsets.resize(count);

		char buffer[64];

		for (auto iter=begin(mOffsets); iter!=end(mOffsets); ++iter)
		{
			FBMatrix &mat = *iter;

			for (int i=0; i<16; ++i)
			{
				memset( buffer, 0, sizeof(char) * 64);
				sprintf_s( buffer, 64, "offsetTM%d", i );
				mat[i] = pFbxObject->FieldReadD( buffer );
			}
		}
	}


	return true;
}


/************************************************
 *	Suggest a freeze.
 ************************************************/

void ConstraintAimSprite::SnapSuggested()
{
	if (ReferenceGetCount(mGroupConstrain) != (int) mOffsets.size() )
	{
		mOffsets.resize( ReferenceGetCount(mGroupConstrain) );
	}

	if (Active == false)
	{

		FBCamera *pCamera = mSystem.Renderer->CurrentCamera;
		if (pCamera == nullptr)
			return;
		if (FBIS(pCamera, FBCameraSwitcher) )
			pCamera = ( (FBCameraSwitcher*) pCamera )->CurrentCamera;
		if (pCamera == nullptr)
			return;

		bool firstForAll = FirstOffsetForAll;

		FBMatrix cammat;
		FBMatrix offsetmat;
		pCamera->GetCameraMatrix(cammat, kFBModelView);
		
		FBVector3d pos;
		pCamera->GetVector(pos);

		FBVector3d upVector = Up;

		for (int i=0; i<(int)mOffsets.size(); ++i)
		{
			FBModel *pModel = ReferenceGet(mGroupConstrain, i);
			FBVector3d lPosition;
			pModel->GetVector(lPosition);
			FBMatrix m;
			pModel->GetMatrix(m);

			if ( !firstForAll || i==0 )
			{
				// compute only one offset matrix and populate it
				//	or compute for each element individualy
				ComputeLookAtMatrix( cammat, lPosition, pos, upVector );
				FBGetLocalMatrix( offsetmat, cammat, m );
			}
			
			mOffsets[i] = offsetmat;
			
		}

	}
}

void ConstraintAimSprite::FreezeSuggested()
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



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void ConstraintAimSprite::ComputeLookAtMatrix( FBMatrix &lookAtMat, const FBVector3d &modelPos, const FBVector3d &cameraPos, const FBVector3d &upVector )
{
	FBVector3d diff;
	VectorSub( modelPos, cameraPos, diff );
	VectorNormalize(diff);

	LookAt( FBVector3d(-diff[0], -diff[1], -diff[2]), FBVector3d(0.0, 0.0, 0.0), upVector, lookAtMat );
	FBMatrixInverse(lookAtMat, lookAtMat);
}

bool ConstraintAimSprite::AnimationNodeNotify(FBAnimationNode* pConnector,FBEvaluateInfo* pEvaluateInfo,FBConstraintInfo* pConstraintInfo)
{
	double lPosition[3];

	if (Active == false)
		return false;
	
	FBCamera *pCamera = mSystem.Renderer->CurrentCamera;
	if (pCamera == nullptr)
		return false;

	if (FBIS(pCamera, FBCameraSwitcher) )
		pCamera = ( (FBCameraSwitcher*) pCamera )->CurrentCamera;

	if (pCamera == nullptr)
		return false;

	FBVector3d pos;
	//FBMatrix modelView;
	pCamera->GetVector(pos, kModelTranslation, true, pEvaluateInfo);
	//pCamera->GetCameraMatrix( modelView, kFBModelView );
	
	FBAimType aimType = Type;
	//FBVector3d pos(0.0, 5.0, 100.0);
	if (aimType == kFBAimCylidrical)
		pos[1] = 0.0;
	
	FBVector3d upVector(Up);

	
	//
	if (mOffsets.size() != mConstraintedRotation.size() )
	{
		mOffsets.resize( mConstraintedRotation.size() );
		for (auto iter=begin(mOffsets); iter!=end(mOffsets); ++iter)
			iter->Identity();
	}

	//
	//

	int itemIndex = -1;
	int count = (int) mConstraintedRotation.size();
	mSourceValues.resize(count);

	for (int i=0; i<count; ++i)
	{
		if (mConstraintedRotation[i] == pConnector)
		{
			itemIndex = i;
			break;
		}
	}

	if (itemIndex == -1)
	{
		pConnector->DisableIfNotWritten(pEvaluateInfo);
		return false;
	}

	//
	//
	if (pConstraintInfo->GetZeroRequested() )
	{
		FBMatrix identity;
		identity.Identity();

		mOffsets[itemIndex] = identity;
	}
	
	FBAnimationNode *psource = mSourceTranslation[itemIndex];

	if (psource)
	{
		bool status = psource->ReadData( lPosition, pEvaluateInfo );

		if (status == false || _isnan(lPosition[0]) || _isnan(lPosition[1]) || _isnan(lPosition[2]) )
		{
			return false;
		}

		mSourceValues[itemIndex] = FBVector3d(lPosition);
	}
	
	FBAnimationNode *pcon = mConstraintedRotation[itemIndex];

	//bool status=false;
	
	memcpy( lPosition, mSourceValues[itemIndex], sizeof(double) * 3 );

	if (aimType == kFBAimCylidrical)
		lPosition[1] = 0.0;

	FBMatrix rotMatrix;
	ComputeLookAtMatrix(rotMatrix, lPosition, pos, upVector);

	FBMatrixMult( rotMatrix, rotMatrix, mOffsets[itemIndex] );

	FBRVector rotVector;
	FBMatrixToRotation(rotVector, rotMatrix);

	
		
		
	auto fn_checkAngle = [] (double &value) {
		if (value > 360.0) value = 360.0;
		else if (value < -360.0) value = -360.0;
		
		if ( _isnan(value) )
		{
			value = 0.0;
		}
	};

		

	fn_checkAngle( rotVector[0] );
	fn_checkAngle( rotVector[1] );
	fn_checkAngle( rotVector[2] );

	//pModel->SetVector( rotVector, kModelRotation, true );
		
	bool lStatus = pcon->WriteData	( rotVector, pEvaluateInfo );

	if (lStatus == false)
		return false;
	
	return true;
}