#ifndef __ORCONSTRAINT_RIGIDBODIES_CONSTRAINT_H__
#define __ORCONSTRAINT_RIGIDBODIES_CONSTRAINT_H__

/**	\file	orconstraint_template_constraint.h
*	Declaration of a simple constraint class.
*	Simple constraint class declaration (FBSimpleConstraint).
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Newton SDK include
#include "..\Common_Physics\physics_common.h"
#include "queryFBGeometry.h"

#define	ORCONSTRAINTRB__CLASSNAME		ORConstraint_RigidBodies
#define ORCONSTRAINTRB__CLASSSTR		"ORConstraint_RigidBodies"



//! A simple constraint class.
class ORConstraint_RigidBodies : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( ORConstraint_RigidBodies, FBConstraint );

public:
	//--- Creation & Destruction
	virtual bool			FBCreate() override;						//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes() override;			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes() override;			//!< Setup animation nodes.


	virtual void			SnapSuggested		() override;			//!< Suggest 'snap'.
	virtual void			FreezeSuggested		() override;			//!< Suggest 'freeze'.

	//--- Constraint Status interface
	virtual bool			Disable		( FBModel* pModel ) override;	//!< Disable the constraint on a model.

	//--- Real-Time Engine
	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo ) override;


	virtual void			FreezeSRT	( FBModel* pModel, bool pS, bool pR, bool pT ) override;						//!< Freeze Scaling, Rotation and Translation for a model.

	virtual bool			ReferenceAddNotify		( int pGroupIndex, FBModel* pModel ) override;					//!< Reference added: Callback.
	virtual bool			ReferenceRemoveNotify	( int pGroupIndex, FBModel* pModel ) override;					//!< Reference removed: Callback.


	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Retrieval.

	FBPropertyDouble		Mass;		// physics mass
	FBPropertyDouble		Friction;	// physcis friction
	FBPropertyBool			ConvexHullShape;	// use convex hull for collision detection

	FBPropertyListObject	Device;		// source physics device		

	void					SaveState();
	void					RestoreState();

	////////////////////////////////////////////
	// main functions for preparing phys elements to go
	void					EnterOnline();
	void					LeaveOnline();

private:

	bool					mNeedRebuild;		// for example when change convex hull shape

	int						mNodesCount;
	struct Node {

		FBAnimationNode		*T;
		FBAnimationNode		*R;

		std::auto_ptr<PHYSICS_INTERFACE::IBody>		body;
		QueryFBGeometry								geometry;
		//! a constructor
		Node()
		{
			T = nullptr;
			R = nullptr;
		}

	}	mNodes[100];

	FBAnimationNode			*mDummy_AnimationNode;

	bool AssignDevice();
};

#endif	/* __ORCONSTRAINT_RIGIDBODIES_CONSTRAINT_H__ */
