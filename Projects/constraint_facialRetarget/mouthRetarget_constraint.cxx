
/**	\file	orconstraintposition_constraint.cxx
*	Definition of a simple constraint class.
*	Simple constraint function definitions for the ORConstraintPosition
*	class.
*/

//--- Class declarations
#include "mouthRetarget_constraint.h"
#include "algorithm\math3d_mobu.h"

//--- Registration defines
#define	ORCONSTRAINTMOUTHRETARGET__CLASS		ORCONSTRAINTMOUTHRETARGET__CLASSNAME
#define ORCONSTRAINTMOUTHRETARGET__NAME			"Mouth Retarget"
#define ORCONSTRAINTMOUTHRETARGET__LABEL		"Mouth Retarget"
#define ORCONSTRAINTMOUTHRETARGET__DESC			"Mouth Retarget"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINTMOUTHRETARGET__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINTMOUTHRETARGET__NAME,
								ORCONSTRAINTMOUTHRETARGET__CLASS,
								ORCONSTRAINTMOUTHRETARGET__LABEL,
								ORCONSTRAINTMOUTHRETARGET__DESC,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)

static void MouthRetarget_AssignSource(HIObject pObject, bool value) {
	ConstraintMouthRetarget *pplug = FBCast<ConstraintMouthRetarget>(pObject);
	if (value && pplug) pplug->DoAssignSource();
}

/************************************************
 *	Creation function.
 ************************************************/
bool ConstraintMouthRetarget::FBCreate()
{
	Deformer = true;
	HasLayout = false;
	Description = "Mouth Retarget";

	FBPropertyPublish( this, StickyLips, "Sticky Lips", nullptr, nullptr );

	FBPropertyPublish( this, StickyLeftLip, "Sticky Left Lip", nullptr, nullptr );
	FBPropertyPublish( this, LeftCorner, "Left Corner", nullptr, nullptr );

	FBPropertyPublish( this, StickyRightLip, "Sticky Right Lip", nullptr, nullptr );
	FBPropertyPublish( this, RightCorner, "Right Corner", nullptr, nullptr );

	FBPropertyPublish( this, CorrespondancePostfix, "Corr Postfix", nullptr, nullptr);
	FBPropertyPublish( this, AssignSource, "Assign Source", nullptr, MouthRetarget_AssignSource );

	StickyLips.SetMinMax(0.0, 100.0, true, true);
	StickyLips = 10.0;

	StickyLeftLip.SetMinMax(0.0, 100.0, true, true);
	StickyLeftLip = 10.0;
	StickyRightLip.SetMinMax(0.0, 100.0, true, true);
	StickyRightLip = 10.0;

	LeftCorner.SetMinMax(-100.0, 100.0, false, false);
	LeftCorner = 0.0;
	RightCorner.SetMinMax(-100.0, 100.0, false, false);
	RightCorner = 0.0;

	CorrespondancePostfix = "_dst";

	// Create reference group
	mGroupSource	= ReferenceGroupAdd( "Source Objects",	MAX_NUMBER_OF_NODES );
	mGroupConstrain	= ReferenceGroupAdd( "Constrain",		MAX_NUMBER_OF_NODES );

	
	for (int i = 0; i < MAX_NUMBER_OF_NODES; ++i)
	{
		mSourceTranslation[i] = nullptr;
		mBonesInTranslation[i] = nullptr;
		mBonesOutTranslation[i] = nullptr;
		mSnapTranslation[i] = FBVector3d(0.0, 0.0, 0.0);
	}

	return true;
}


/************************************************
 *	Destruction function.
 ************************************************/
void ConstraintMouthRetarget::FBDestroy()
{
}


/************************************************
 *	Setup all of the animation nodes.
 ************************************************/
void ConstraintMouthRetarget::SetupAllAnimationNodes()
{
	if (ReferenceGetCount( mGroupSource ) > 0 && (ReferenceGetCount(mGroupSource) ==  ReferenceGetCount( mGroupConstrain )) )
	{
		for (int i=0; i<ReferenceGetCount(mGroupSource); ++i)
		{
			if (ReferenceGet(mGroupSource, i) != nullptr && ReferenceGet(mGroupConstrain, i) != nullptr)
			{
				mSourceTranslation[i]		= AnimationNodeOutCreate( i*3, ReferenceGet( mGroupSource,	i ), ANIMATIONNODE_TYPE_TRANSLATION );
				mBonesInTranslation[i] = AnimationNodeOutCreate	( i*3+1, ReferenceGet( mGroupConstrain, i ), ANIMATIONNODE_TYPE_TRANSLATION );
				mBonesOutTranslation[i] = AnimationNodeInCreate	( i*3+2, ReferenceGet( mGroupConstrain, i ), ANIMATIONNODE_TYPE_TRANSLATION );

				ReferenceGet(mGroupSource, i)->GetVector( mSnapTranslation[i] );
			}
		}

	}
}


void ConstraintMouthRetarget::DoAssignSource()
{
	const int bonesCount = ReferenceGetCount(mGroupConstrain);
	if (bonesCount == 0)
		return;

	//mSystem.BeginChange();

	if (ReferenceGetCount(mGroupSource) > 0)
	{
		for (int i=ReferenceGetCount(mGroupSource)-1; i>= 0; --i)
		{
			ReferenceRemove(mGroupSource, ReferenceGet(mGroupSource, i));
		}
	}

	for (int i=0; i<bonesCount; ++i)
	{
		FBString name( ReferenceGet(mGroupConstrain, i)->Name, CorrespondancePostfix );
		
		FBModel *pModel = FBFindModelByLabelName(name);

		if (pModel)
		{
			ReferenceAdd(mGroupSource, pModel);
		}
		else
		{
			printf ("> MouthRetarget: model not found - %s\n", name );
		}
	}

	//mSystem.EndChange();
}

/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void ConstraintMouthRetarget::RemoveAllAnimationNodes()
{
}


/************************************************
 *	FBX storage of constraint parameters.
 ************************************************/
bool ConstraintMouthRetarget::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}


/************************************************
 *	FBX retrieval of constraint parameters.
 ************************************************/
bool ConstraintMouthRetarget::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}


/************************************************
 *	Suggest a snap.
 ************************************************/
void ConstraintMouthRetarget::SnapSuggested()
{
	FBConstraint::SnapSuggested();

	for (int i=0; i<ReferenceGetCount(mGroupSource); ++i)
	{
		if (ReferenceGet(mGroupSource, i) )
			ReferenceGet(mGroupSource, i)->GetVector( mSnapTranslation[i] );
	}

}

/************************************************
 *	Suggest a freeze.
 ************************************************/
void ConstraintMouthRetarget::FreezeSuggested()
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
bool ConstraintMouthRetarget::AnimationNodeNotify(FBAnimationNode* pConnector, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo)
{
	
	double lSource[3];
	double lBoneIn[3];

	for (int i=0; i<ReferenceGetCount(mGroupSource); ++i)
	{
		if (mSourceTranslation[i] && mBonesInTranslation[i] && mBonesOutTranslation[i])
		{
			mSourceTranslation[i]->ReadData		( lSource, pEvaluateInfo );
			//mBonesInTranslation[i]->ReadData		( lBoneIn, pEvaluateInfo );

			// TODO: calculations
			//double *lSnap = mSnapTranslation[i];
			//lBoneIn[0] += lSource[0] - lSnap[0];
			//lBoneIn[1] += lSource[1] - lSnap[1];
			//lBoneIn[2] += lSource[2] - lSnap[2];

			mBonesOutTranslation[i]->WriteData	( lSource, pEvaluateInfo );
		}
	}

	return true;
}
