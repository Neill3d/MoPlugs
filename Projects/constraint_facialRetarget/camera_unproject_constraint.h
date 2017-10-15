#ifndef __ORCONSTRAINT_POSITION_CONSTRAINT_H__
#define __ORCONSTRAINT_POSITION_CONSTRAINT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: camera_unproject_constraint.h
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

#define ORCONSTRAINTUNPROJECT__CLASSNAME	ConstraintCameraUnProject
#define ORCONSTRAINTUNPROJECT__CLASSSTR		"ConstraintCameraUnProject"

#define MAX_NUMBER_OF_NODES					128

//! A simple constraint class.
class ConstraintCameraUnProject : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare(ConstraintCameraUnProject, FBConstraint);

public:
	//--- Creation & Destruction
	virtual bool			FBCreate();							//!< Constructor.
	virtual void			FBDestroy();						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes();			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes();			//!< Setup animation nodes.


	virtual void			FreezeSuggested		();				//!< Suggest 'freeze'.

	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo ) override;


	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Retrieval.

	//--- Local parameters
	FBAnimationNode*	mSourceTranslation[MAX_NUMBER_OF_NODES];			//!< AnimationNode: INPUT  -> Source (uv in camera space.
	FBAnimationNode*	mConstrainedTranslation[MAX_NUMBER_OF_NODES];	//!< AnimationNode: OUTPUT -> Constraint (world space result).
	
	int					mGroupSource;		//!< Source groupe index - uv coords in camera view
	int					mGroupConstrain;	//!< Constraint group index - output 3d points on a face

	FBPropertyListObject	Camera;			//!< INPUT -> get camera proj and modelview matrices
	FBPropertyListObject	Meshes;			//!< AnimationNode: INPUT -> closest point on that meshes

	FBPropertyDouble		UnProjectDepth;			// reassign value for the z of view camera coords
	FBPropertyAction		CreateDestination;		//!< dublicate input point with destination null object and constraint them

	void DoCreateDestination();

protected:

	FBSystem		mSystem;

};

#endif	/* __ORCONSTRAINT_POSITION_CONSTRAINT_H__ */
