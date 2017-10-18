#ifndef __ORCONSTRAINT_PYTHON_CONSTRAINT_H__
#define __ORCONSTRAINT_PYTHON_CONSTRAINT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orconstraint_column_constraint.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#include "algorithm\kdtree_common.h"
#include <vector>
#include <random>

#define	ORCONSTRAINT__CLASSNAME		ORConstraint_Column
#define ORCONSTRAINT__CLASSSTR		"ORConstraint_Column"

#define	MAX_NUMBER_OF_ITEMS		512

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

////////////////////////////////////////////////////////////////////////////////////////
//

struct ColumnItem
{
	double		pos;
	double		vel;
	double		mass;
	double		randomF;	// randomize the raise motion

	std::vector<ColumnItem*>	neighbors;

	//! a constructor
	ColumnItem()
	{
		pos = 0.0;
		vel = 0.0;
		mass = 1.0;
	}

	void Reset()
	{
		mass = 1.0;
		pos = 0.0;
		vel = 0.0;
	}

	void	Raise(const double value)
	{
		pos = value;
		vel = 0.0;
	}

	const double GetRandomF() const
	{
		return randomF;
	}

	const double GetLevel() const
	{
		return pos;
	}
	const int	GetNumberOfNeighbors()
	{
		return (int)neighbors.size();
	}
	const double GetAverageLevel()
	{
		double level = 0.0;
		for (auto iter=neighbors.begin(); iter!=neighbors.end(); ++iter)
			level += (*iter)->GetLevel();

		if (neighbors.size() > 0)
			level /= neighbors.size();

		return level;
	}

	void ClearNeighbors()
	{
		neighbors.clear();
	}
	void AddNeighbor(ColumnItem *pitem)
	{
		neighbors.push_back(pitem);
	}
};

////////////////////////////////////////////////////////////////////////////////////////
//! A column play constraint class.
class ORConstraint_Column : public FBConstraint
{
	//--- declaration
	FBClassDeclare( ORConstraint_Column, FBConstraint );

public:
	ORConstraint_Column(const char* pName)
		: FBConstraint(pName) 
		, rng(rd)
	{ 
		FBClassInit; 
	}
  
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

	//--- Deformation Interface
	//! Real-time deformation engine function.
	virtual bool			DeformerNotify(	FBModel* pModel, const FBVertex*  pSrcVertex, const FBVertex* pSrcNormal,
											int pCount,
											FBVertex*  pDstVertex,FBVertex*  pDstNormal) override;
	virtual bool			DeformerBind	( FBModel* pModel ) override;	//!< Bind a model to deformer callback.
	virtual bool			DeformerUnBind	( FBModel* pModel ) override;	//!< Unbind a model from deformer callback.


	virtual void			FreezeSRT	( FBModel* pModel, bool pS, bool pR, bool pT ) override;						//!< Freeze Scaling, Rotation and Translation for a model.

	virtual bool			ReferenceAddNotify		( int pGroupIndex, FBModel* pModel ) override;					//!< Reference added: Callback.
	virtual bool			ReferenceRemoveNotify	( int pGroupIndex, FBModel* pModel ) override;					//!< Reference removed: Callback.


	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Retrieval.

	
	FBPropertyAnimatableDouble		DisplaceValue;	//!< input voice level for animation
	FBPropertyAnimatableDouble		DisplaceRandom;	// random the input
	FBPropertyDouble				ConnectDistance;	// connect columns between each other to inherit motion
	FBPropertyListObject			Curve;	//!< curve for selecting handlers
	FBPropertyDouble				CurveStep;		// step for curve interpolation
	FBPropertyDouble				CurveThreshold;	// k-d tree search nearest distance
	FBPropertyDouble				EvaluateDeltaTime;	// timestep for each evaluate iteration

	// physical properties
	FBPropertyAnimatableDouble		Mass;
	FBPropertyAnimatableDouble		Gravity;
	FBPropertyAnimatableDouble		FloorLevel;
	FBPropertyAnimatableDouble		Damping;
	FBPropertyAnimatableDouble		Q;

public:

	int					mGroupSource;		//!< Source groupe index.
	//int					mGroupConstrain;	//!< Constraint group index.

	int					mCount;

	FBAnimationNode		*mOutput[MAX_NUMBER_OF_ITEMS];

private:

	FBTime						mLastTime;

	KDTree						mTree;	// k-d tree

	std::vector<FBVector3d>		mLevels;

	std::vector<ColumnItem*>	mColumns;

	std::random_device		rd;
	std::mt19937			rng;	// random-number engine used

	void	ConnectColumns(const double dist);
	void	FreeColumns();

	void	ReadValues(FBEvaluateInfo* pEvaluateInfo, double *displace, double *randomF, double *mass, double *gravity, double *floorlevel, double *damping, double *q);

	// column wave calculation step
	void	DoStep(const double floor_level, const double gravity, const double dt, const double q, const double damping, const double mass);

};

#endif	/* __ORCONSTRAINT_PYTHON_CONSTRAINT_H__ */
