
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: moPhysics_solver.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Class declaration
#include "..\Common_Physics\physics_common.h"
#include "queryFBGeometry.h"
#include <vector>

//--- Registration defines
#define MOPHYSICSSOLVER__CLASSNAME		MOPhysicsSolver
#define MOPHYSICSSOLVER__CLASSSTR		"MOPhysicsSolver"

#define MAX_NUMBER_OF_CHAIN_JOINS		12

//////////////////////////////////////////////////////////////////////////////
// MOPhysicsSolver

enum EPhysicsEngine
{
	ePhysicsNewton,
	ePhysicsBullet
};

const char * FBPropertyBaseEnum<EPhysicsEngine>::mStrings[] = {
	"Newton Engine 3.11",
	"Bullet 2.82",
	0
};

class MOPhysicsSolver : public FBConstraintSolver
{
	FBConstraintSolverDeclare( MOPhysicsSolver, FBConstraintSolver );

public:
	//--- Creation & Destruction
	virtual bool			FBCreate() override;						//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	virtual void ActiveChanged() override;				//!< Notification for Activation Status Change.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes() override;			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes() override;			//!< Setup animation nodes.


	virtual void			SnapSuggested		() override;			//!< Suggest 'snap'.
	virtual void			FreezeSuggested		() override;			//!< Suggest 'freeze'.

	//--- Constraint Status interface
	virtual bool			Disable		( FBModel* pModel ) override;	//!< Disable the constraint on a model.

	//--- Real-Time Engine

	/**	Evaluation of non TRS nodes that needs to be evaluated.
    *	This function is called by the real-time engine in order to process animation information.
    *	\param	pEvaluateInfo	Information concerning the evaluation of the animation (time, etc.)
    *	\return \b true if animation node notification is successful.
    */
    virtual bool EvaluateAnimationNodes( FBEvaluateInfo* pEvaluateInfo ) override;

	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo ) override;


	virtual void			FreezeSRT	( FBModel* pModel, bool pS, bool pR, bool pT ) override;						//!< Freeze Scaling, Rotation and Translation for a model.

	virtual bool			ReferenceAddNotify		( int pGroupIndex, FBModel* pModel ) override;					//!< Reference added: Callback.
	virtual bool			ReferenceRemoveNotify	( int pGroupIndex, FBModel* pModel ) override;					//!< Reference removed: Callback.


	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Retrieval.


	virtual void LiveChanged() override;

#ifdef ORSDK2013
	void		DoEvaluate();
	static void OnPerFrameEvaluationPipelineCallback(FBEvaluateInfo* pEvaluteInfo);
#else
	void OnPerFrameEvaluationPipelineCallback   (HISender pSender, HKEvent pEvent);
#endif

public:

	static void ActionRebuildCollisions( HIObject pObject, bool value );
	static void ActionResetToStart( HIObject pObject, bool value );
	static void ActionSaveState( HIObject pObject, bool value );
	static void ActionSerialize( HIObject pObject, bool value );
	
	static bool GetLiveMode( HIObject pObject );
	static void SetLiveMode( HIObject pObject, bool value );
	static bool GetRecording( HIObject pObject );
	static void SetRecording( HIObject pObject, bool value );

	static void SetWorldScale( HIObject pObject, double value );

	static void SetDefaultSoftness( HIObject pObject, double value );
	static void SetDefaultElasticity( HIObject pObject, double value );
	static void SetDefaultCollidable( HIObject pObject, double value );
	static void SetDefaultStaticFriction( HIObject pObject, double value );
	static void SetDefaultDynamicFriction( HIObject pObject, double value );

public:
	FBPropertyInt						PhysicsThreads;		// number of samples per second (default 120)
	FBPropertyInt						PhysicsSamples;
	FBPropertyDouble					PhysicsFPS;

	FBPropertyBool						LiveState;
	FBPropertyBool						RecordState;

	FBPropertyAction					ResetToStart;
	FBPropertyAction					SetStartState;
	FBPropertyAction					Serialize;

	FBPropertyBaseEnum<EPhysicsEngine>	PhysicsEngine;

	FBPropertyInt						ResetFrame;			// frame for resetting physics state in player mode

	FBPropertyVector3d					Gravity;			// give option to customize gravity force
	FBPropertyBool						AutoScale;			// calculate scaling according to the original phys sizes
	FBPropertyDouble					WorldScale;			// coeff for scaling whole physics world to renderable world

	FBPropertyListObject				StaticCollisions;	// generate world static collision tree from these objects
	FBPropertyDouble					DefaultSoftness;
	FBPropertyDouble					DefaultElasticity;
	FBPropertyDouble					DefaultCollidable;
	FBPropertyDouble					DefaultStaticFriction;
	FBPropertyDouble					DefaultDynamicFriction;

	FBPropertyBool						DisplayDebug;		//! draw debug information
	bool								WorldIsReady;

	void ClearLevel() { if (mHardware.get()) mHardware->ClearLevel(); }
	bool LoadLevel( FBComponentList &list );

	PHYSICS_INTERFACE::IWorld	*GetWorldPtr() {
		return mHardware.get();
	}

	// create a new car depends on a current physics engine
	PHYSICS_INTERFACE::IBody		*CreateNewBody( PHYSICS_INTERFACE::BodyOptions *pOptions, const PHYSICS_INTERFACE::IQueryGeometry *pGeometry, bool convexhull );
	PHYSICS_INTERFACE::ICar			*CreateNewCar( PHYSICS_INTERFACE::CarOptions *pOptions, const PHYSICS_INTERFACE::IQueryGeometry *pCarGeometry[5], const PHYSICS_INTERFACE::IQueryPath *curve );

	bool	DisconnectACar( FBPhysicalProperties	*props );

	void	ResetPhysics() { if (mHardware.get()) mHardware->Reset(); }

	void	SaveState() { if (mHardware.get()) mHardware->SaveState(); }
	void	RestoreState() { if (mHardware.get()) mHardware->RestoreState(); }

	// setup collisions, and phys elements with a connected constraints
	void	EnterOnline();
	// goes to offline mode
	void	LeaveOnline();

	void WaitForUpdateToFinish() { if (mHardware.get()) mHardware->WaitForUpdateToFinish(); }
	
	void DrawDebug() const;

	void DoRebuildCollisions();
	// after we change world scale - rebuild all physics elements
	void DoRestart();
	void DoUpdateDefaultMaterialParams();
	void DoSerialize() { if (mHardware.get()) mHardware->Serialize("C:\\world_snapshot.dat"); }

	bool IsRebuildNeeded() { return mNeedRebuild; }

protected:
	std::auto_ptr<PHYSICS_INTERFACE::IWorld>	mHardware;					//!< Handle onto hardware.
	FBPlayerControl						mPlayerControl;				//!< To get play mode for recording.
	FBSystem							mSystem;

	FBTime								mEvalTime;

	FBComponentList						mCollisions;

	bool								mNeedRebuild;

	bool								mLastTimeLocal;			// local or system
	bool								mLastIsStop;			// stop or play
	bool								mLastIsRecording;

	double								mAnimTimeSecs;				// anim time computed as interpolation between last phys time and curr phys time
	
	double								mPhysTimeSecs;				// phys time is always should be >= anim time
	double								mLastPhysTimeSecs;

	double								mLastTorque;

protected:

	double				mWorldScaling;

	struct PhysNode
	{
		QueryFBGeometry		geometry;

		FBAnimationNode		*tr;
		FBAnimationNode		*rot;

		PHYSICS_INTERFACE::IBody	*body;
	};

	std::vector<PhysNode>		mRigidBodies;

	void		AllocateRigidBodies(const int count);
	void		FreePhysNode(PhysNode &pNode);
	void		FreeRigidBodies();

	bool		WritePhysNodeData(PhysNode &node, bool writedata, FBEvaluateInfo* pEvaluateInfo);

	// dynamic chains - bodies connected with a ball sockets
	//

	struct ChainNode
	{
		PhysNode			nodes[MAX_NUMBER_OF_CHAIN_JOINS];
	};

	std::vector<ChainNode>			mChainNodes;

	void	AllocateChainNodes(const int count);
	void	FreeChainNodes();

	//
	//

	struct CarNode
	{
		struct Node
		{
			QueryFBGeometry		geometry;

			FBAnimationNode		*tr;
			FBAnimationNode		*rot;
		};

		Node				chassis;
		Node				wheels[4];
		
		QueryFBPath3D	steeringCurve;

		PHYSICS_INTERFACE::ICar		*car;

		// properties for input
		FBAnimationNode		*torque;
		FBAnimationNode		*clutch;
		FBAnimationNode		*steering;
		FBAnimationNode		*steeringBlend;
		FBAnimationNode		*brake;
		FBAnimationNode		*handbrake;
		FBAnimationNode		*gear;

		// properties for output
		FBAnimationNode		*speed;
		FBAnimationNode		*rpm;
		FBProperty			*speedProp;
		FBProperty			*rpmProp;
		FBProperty			*currentGearProp;

		// pointer to phys properties
		FBPhysicalProperties	*props;
	};

	std::vector<CarNode>		mCars;

	void		AllocateCars(const int count);
	void		FreeCar(std::vector<CarNode>::iterator	&iter);
	void		FreeCars();

	bool	UpdateInput(FBEvaluateInfo* pEvaluateInfo);
	bool	UpdatePhysics(FBEvaluateInfo* pEvaluateInfo);
	bool	UpdateAllCars(FBEvaluateInfo *pEvaluateInfo);
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class MOPhysicsSolverLayout : public FBConstraintLayout
{
	FBConstraintLayoutDeclare( MOPhysicsSolverLayout, FBConstraintLayout );

public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();			//!< FiLMBOX constructor.
	virtual void FBDestroy();			//!< FiLMBOX destructor.

	FBEditProperty		mEditActive;

};