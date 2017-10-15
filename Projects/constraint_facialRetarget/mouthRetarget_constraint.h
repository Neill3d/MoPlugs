#ifndef __MOUTH_RETARGET_CONSTRAINT_H__
#define __MOUTH_RETARGET_CONSTRAINT_H__

/**	\file	orconstraintposition_constraint.h
*	Declaration of a simple constraint class.
*	Simple constraint class declaration (FBSimpleConstraint).
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <vector>

#define ORCONSTRAINTMOUTHRETARGET__CLASSNAME	ConstraintMouthRetarget
#define ORCONSTRAINTMOUTHRETARGET__CLASSSTR		"ConstraintMouthRetarget"

#define MAX_NUMBER_OF_NODES					128

//! A simple constraint class.
class ConstraintMouthRetarget : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare(ConstraintMouthRetarget, FBConstraint);

public:
	//--- Creation & Destruction
	virtual bool			FBCreate();							//!< Constructor.
	virtual void			FBDestroy();						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes();			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes();			//!< Setup animation nodes.

	virtual void			SnapSuggested();				//!< Suggest 'snap'.
	virtual void			FreezeSuggested		();				//!< Suggest 'freeze'.

	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo ) override;


	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Retrieval.

	//--- Local parameters
	FBAnimationNode*	mBonesInTranslation[MAX_NUMBER_OF_NODES];			//!< AnimationNode: INPUT  -> 
	FBAnimationNode*	mSourceTranslation[MAX_NUMBER_OF_NODES];
	FBAnimationNode*	mBonesOutTranslation[MAX_NUMBER_OF_NODES];			//!< AnimationNode: OUTPUT -> Constraint (world space result).
	
	FBVector3d			mSnapTranslation[MAX_NUMBER_OF_NODES];

	int					mGroupSource;		//!< Source groupe index - uv coords in camera view
	int					mGroupConstrain;	//!< Constraint group index - output 3d points on a face

	//
	FBPropertyAnimatableDouble		StickyLips;

	FBPropertyAnimatableDouble		StickyLeftLip;
	FBPropertyAnimatableDouble		LeftCorner;

	FBPropertyAnimatableDouble		StickyRightLip;
	FBPropertyAnimatableDouble		RightCorner;

	FBPropertyAction				AssignSource;		//!< dublicate input point with destination null object and constraint them

	FBPropertyString				CorrespondancePostfix;	// name + "_dst"
	
	
	void DoAssignSource();

protected:

	FBSystem		mSystem;

};

#endif	/* __MOUTH_RETARGET_CONSTRAINT_H__ */
