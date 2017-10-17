#ifndef __ORCONSTRAINT_WALLBRICKS_CONSTRAINT_H__
#define __ORCONSTRAINT_WALLBRICKS_CONSTRAINT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: wallbricks_constraint.h
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
#include "expr.h"
#include <vector>
#include <random>

#include "algorithm\kdtree_common.h"

#define ORCONSTRAINTWALLBRICKS__CLASSNAME		ORConstraintWallBricks
#define ORCONSTRAINTWALLBRICKS__CLASSSTR		"ORConstraintWallBricks"

#define ORCONSTRAINTWALLBRICKSTEMP__CLASSNAME		ORConstraintWallBricksTemp
#define ORCONSTRAINTWALLBRICKSTEMP__CLASSSTR		"ORConstraintWallBricksTemp"

#ifndef MAX_NUMBER_OF_BRICKS		
#define	MAX_NUMBER_OF_BRICKS		16384
#endif

////////////////////////////////////////////////////////////////////////////////
// KDTree

class KDTree
{
public:
	//! a constructor
	KDTree();
	//! a destructor
	virtual ~KDTree();

	// build K-D Tree for this frame
	// each point - xyz
	int	BuildKDTree(int count, double *positions);

	// free kd-tree
	int FreeKDTree();

	/*! SearchForNearest

		\param pos - x,y,z of input point
		\param index - result index of a nearest point in k-d tree
		\param dist - distance from the input points to nearest

		\return index of nearest point in k-d tree
		 return -1 if nearest point is not found
		 retur (-1, -1, -1) if it's a vector point
	*/
	int SearchForNearest(const double *pos, const double dist);

	int SearchForNearest(const double *pos, const double dist, double *res);

private:
	Tree					*mTree;		// k-d tree
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//! A curve deformer constraint class

class ORConstraintPathWrap : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( ORConstraintPathWrap, FBConstraint );

public:
	//--- Creation & Destruction
	virtual bool			FBCreate() override;							//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	virtual bool ReferenceAddNotify		( int pGroupIndex, FBModel* pModel ) override;
	virtual bool ReferenceRemoveNotify	( int pGroupIndex, FBModel* pModel ) override;

	/**	Deformation notification function
	*	\param	pModel		Model to deform.
	*	\param	pSrcVertex	Source vertex array (i.e. vertex to deform).
	*	\param	pSrcNormal	Normals at source vertex array.
	*	\param	pCount		Number of vertices in source/destination arrays
	*	\param	pDstVertex	Destination vertex array (i.e. where source vertex gets deformed to)
	*	\param	pDstNormal	Normals at destination vertex array.
	*	\return \b true if successful.
	*/
	virtual bool DeformerNotify(	FBModel* pModel, const FBVertex*  pSrcVertex, const FBVertex* pSrcNormal,
									int pCount, 
									FBVertex*  pDstVertex,FBVertex*  pDstNormal);

protected:

	FBAnimationNode		*mSourceCurve;		// 3d curve, path along which we animate our bricks


	int						mGroupCurve;		//!< Source groupe index.
	int						mGroupDeformed;

};


//////////////////////////////////////////////////////////////////////////////////////////////////
//! A simple deformer constraint class - needed for recover geometry instances after scene load

class ORConstraintWallBricksTemp : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( ORConstraintWallBricksTemp, FBConstraint );

public:
	//--- Creation & Destruction
	virtual bool			FBCreate() override;							//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	virtual bool ReferenceAddNotify		( int pGroupIndex, FBModel* pModel ) override;
	virtual bool ReferenceRemoveNotify	( int pGroupIndex, FBModel* pModel ) override;

protected:


	int						mGroupDeformed;

};

//////////////////////////////////////////////////////////////////////////////////////////////////
//! A wall bricks animation constraint class.
class ORConstraintWallBricks : public FBConstraint
{
	//--- declaration
	//FBConstraintDeclare( ORConstraintWallBricks, FBConstraint );

	FBClassDeclare( ORConstraintWallBricks, FBConstraint ); 
public: 
	ORConstraintWallBricks(const char* pName)
		: FBConstraint(pName) 
		, e2(rd())
		, dist(0, 1.0)
	{ 
		FBClassInit; 
	}

private:

public:
	//--- Creation & Destruction
	virtual bool			FBCreate();							//!< Constructor.
	virtual void			FBDestroy();						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes() override;			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes() override;			//!< Setup animation nodes.

	virtual void			SnapSuggested() override;
	virtual void			FreezeSuggested		() override;				//!< Suggest 'freeze'.

	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo );

	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Retrieval.

	//--- Local parameters

	FBAnimationNode		*mSourceCurve;		// 3d curve, path along which we animate our bricks

	struct BrickNode
	{
		bool			active;			// import flag to activate brick animation
		double			creationPercent;
		double			localTime;	// local animation time
		double			animFactor;

		FBAnimationNode *posNode;
		FBAnimationNode *rotNode;
		FBAnimationNode *scaleNode;

		FBMatrix		initTM;		// start tm (position and angle on the curve)
		FBMatrix		curveTM;	// animation TM in each frame

		// some random value for this element
		double			randomValue;

		// cached data
		double			cachedAnimFactor;
		double			cachedDistToCamera;

		double			cachedPosx;
		double			cachedPosy;
		double			cachedPosz;

		double			cachedRotx;
		double			cachedRoty;
		double			cachedRotz;
		
		double			cachedSclx;
		double			cachedScly;
		double			cachedSclz;
	};

	
	// use curve point and tangent to calculate start TM
	void	InitBrick( BrickNode &node );
	void	ComputeInitTM( FBMatrix &TM, const double percent );

	bool	IsBrickCached( const BrickNode &node, double f, double distToCam );
	void	GetBrickCache( const BrickNode &node, double &x, double &y, double &z, double &rx, double &ry, double &rz, double &sx, double &sy, double &sz );  
	void	CacheBrick( BrickNode &node, double f, double distToCam, double x, double y, double z, double rx, double ry, double rz, double sx, double sy, double sz );

	std::vector<BrickNode>			mConstrainedBricks;


	int					mGroupCurve;		//!< Source groupe index.
	int					mGroupConstrained;	//!< Constraint group index.

	FBTVector			mPosition;			//!< Position vector.
	FBTVector			mOldPosition;		//!< Old position vector.
	FBTVector			mDisplacement;		//!< Displacement vector.
	bool				mFirstTime;			//!< First time that the constraint is called?
	FBSVector			mScaling;			//!< Scaling vector.

	FBTime				mLastTime;
	bool				mNeedExpressionParsing;
	bool				mExpressionFailed;

	CExpression			mExpression;
	bool				mIsDistToCamUsed;

	bool				mEvalCurve;

	std::random_device					rd;
	std::mt19937						e2;		// engine
	std::uniform_real_distribution<>	dist;	// distribution

	double					mLastTrigger;
	int						mTriggerElement;
	FBTime					mLastAutoTriggerTime;

	void		SetExpressionsToDefault();

public:

	FBPropertyAnimatableDouble	Progress;
	FBPropertyAction			AnimateProgress;

	FBPropertyDouble			ProgressOffset;	// make an offset on the curve
	FBPropertyBool				ProgressInverse;

	FBPropertyBool				ShowOnlyWhileProcessing;	// make nodes visible only when progress spot on them

	FBPropertyBool				TriggerMode;
	FBPropertyAnimatableDouble	Trigger;		// make active some random element
	FBPropertyBool				TurnOffAfterProcessing;
	FBPropertyDouble			AutoTriggerTimer;	// if greater then 0, then trigger from time to time

	//FBPropertyDouble		BrickSideLength;	// used for creation place calculation
	FBPropertyInt			CurveSubdivisions;	// used for point placement
	FBPropertyInt			CurveSegmentSubdivisions;
	FBPropertyDouble		BrickAnimationTime;

	FBPropertyBool			AutoAlignRoot;	// used for manualy computing a rotation angle along a curve
	FBPropertyDouble		SnapThreshold;

	FBPropertyBool			LiveAnimation;

	FBPropertyTime			ResetTime;
	FBPropertyAction		ResetAction;

	FBPropertyBool			StaticCurve;		// evaluate pos and der along a curve only once or each eval ?!

	FBPropertyInt			ScriptErrorCount;
	FBPropertyString		ScriptError;

	FBPropertyString		ScriptPositionX;
	FBPropertyString		ScriptPositionY;
	FBPropertyString		ScriptPositionZ;

	FBPropertyString		ScriptRotationX;
	FBPropertyString		ScriptRotationY;
	FBPropertyString		ScriptRotationZ;

	FBPropertyString		ScriptScalingX;
	FBPropertyString		ScriptScalingY;
	FBPropertyString		ScriptScalingZ;

	FBPropertyAction		ExpressionReset;
	FBPropertyAction		ExpressionLoad;
	FBPropertyAction		ExpressionSave;

	FBPropertyAction		FixInstances;

	FBPropertyBool							UseSizeCurve;
	FBPropertyAction						SizeCurve;
	FBPropertyAnimatableDouble				SizeCurveHolder;

	FBPropertyAction		About;
	FBPropertyAction		Help;

	void DoReset();
	void DoExpressionParse();
	void DoFixInstances();
	void DoSetLiveAnimation();
	void OnButtonSizeCurve();
	void OnChangeStaticCurve();

	void OnAnimateProgressClick();

	void OnProgressOffsetChange();
	void OnProgressInverseChange();

	void OnExpressionResetClick();
	void OnExpressionLoadClick();
	void OnExpressionSaveClick();

	void OnAboutClick();
	void OnHelpClick();

	static void AddPropertiesToPropertyViewManager();

	static void SetScriptPositionX(HIObject pObject, const char *value);
	static void SetScriptPositionY(HIObject pObject, const char *value);
	static void SetScriptPositionZ(HIObject pObject, const char *value);

	static void SetScriptRotationX(HIObject pObject, const char *value);
	static void SetScriptRotationY(HIObject pObject, const char *value);
	static void SetScriptRotationZ(HIObject pObject, const char *value);

	static void SetScriptScalingX(HIObject pObject, const char *value);
	static void SetScriptScalingY(HIObject pObject, const char *value);
	static void SetScriptScalingZ(HIObject pObject, const char *value);

	static void SetLiveAnimation(HIObject pObject, bool value);
	static void ActionFixInstances(HIObject pObject, bool value);
	static void ActionSizeCurve(HIObject pObject, bool value);
	static void ActionStaticCurve(HIObject pObject, bool value);

	static void ActionAnimateProgress(HIObject pObject, bool value);

	static void ActionExpressionReset(HIObject pObject, bool value);
	static void ActionExpressionLoad(HIObject pObject, bool value);
	static void ActionExpressionSave(HIObject pObject, bool value);

	static void SetProgressOffset(HIObject pObject, double value);
	static void SetProgressInverse(HIObject pObject, bool value);

	static void ActionAbout(HIObject pObject, bool value);
	static void ActionHelp(HIObject pObject, bool value);
};

#endif	/* __ORCONSTRAINT_WALLBRICKS_CONSTRAINT_H__ */
