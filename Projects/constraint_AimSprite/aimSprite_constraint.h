#ifndef __ORCONSTRAINT_POSITION_CONSTRAINT_H__
#define __ORCONSTRAINT_POSITION_CONSTRAINT_H__


//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: aimSprite_constraint.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////
//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>

#define ORCONSTRAINTPOSITION__CLASSNAME		ConstraintAimSprite
#define ORCONSTRAINTPOSITION__CLASSSTR		"ConstraintAimSprite"

#define MAX_NUMBER_OF_NODES		1024

enum FBAimType
{
	kFBAimCylidrical,
	kFBAimSpherical
};

const char * FBPropertyBaseEnum<FBAimType>::mStrings[] = {
	"Cylindrical",
	"Spherical",
	0
};

enum ESpriteOrder
{
	eSpriteVector,
	eSpriteMatrix,
	eSpriteBlendedMatrix
};

const char *FBPropertyBaseEnum<ESpriteOrder>::mStrings[] = {
	"Vector",
	"Matrix",
	"Blended Matrix",
	nullptr
};



////////////////////////////////////////////////////////////////////////////////////////////////////
//! A simple constraint class.
class ConstraintAimSprite : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( ConstraintAimSprite, FBConstraint );

public:
	//--- Creation & Destruction
	virtual bool			FBCreate();							//!< Constructor.
	virtual void			FBDestroy();						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes();			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes();			//!< Setup animation nodes.

	virtual void			SnapSuggested() override;
	virtual void			FreezeSuggested		() override;				//!< Suggest 'freeze'.

	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo ) override;

	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Retrieval.

	//--- Local parameters
	//FBAnimationNode*	mSourceTranslation;			//!< AnimationNode: INPUT  -> Source.
	std::vector<FBAnimationNode*>	mSourceTranslation;
	std::vector<FBAnimationNode*>	mConstraintedRotation;
	std::vector<FBMatrix>			mOffsets;

	std::vector<FBVector3d>			mSourceValues;	// pre-cached source values

	int					mGroupSource;		//!< Source groupe index.
	int					mGroupConstrain;	//!< Constraint group index.

	FBSystem			mSystem;

public:

	FBPropertyBaseEnum<FBAimType>		Type;
	FBPropertyVector3d					Up;

	FBPropertyBool						FirstOffsetForAll;
	// maybe snap for only selected object ?!
	FBPropertyAction					GetOffsetFromSel;

	void	OnGetOffsetFromSelClick();

	static void ActionGetOffsetFromSel(HIObject pObject, bool value);

protected:

	FBTime							mLastTime;

	void ComputeLookAtMatrix( FBMatrix &lookAtMat, const FBVector3d &modelPos, const FBVector3d &cameraPos, const FBVector3d &upVector );
};

#endif	/* __ORCONSTRAINT_POSITION_CONSTRAINT_H__ */
