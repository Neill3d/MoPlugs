
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orconstraint_column_constraint.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "orconstraint_column_constraint.h"
#include "orconstraint_column_layout.h"
#include "algorithm\math3d_mobu.h"
#include <vector>

//--- Registration defines
#define	ORCONSTRAINT__CLASS		ORCONSTRAINT__CLASSNAME
#define ORCONSTRAINT__NAME		"Constraint Column Play"
#define	ORCONSTRAINT__LABEL		"Constraint Column Play"
#define ORCONSTRAINT__DESC		"Constraint Column Play"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINT__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINT__NAME,
								ORCONSTRAINT__CLASS,
								ORCONSTRAINT__LABEL,
								ORCONSTRAINT__DESC,
								FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)


////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// KDTree
///////////////////////////////////////////////////////////////////////////////////////////////////

KDTree::KDTree()
{
	mTree = nullptr;
}

KDTree::~KDTree()
{
	FreeKDTree();
}

// build K-D Tree for this frame
int KDTree::BuildKDTree(int count, double *positions)
{
	FreeKDTree();
	
	if (count <= 0) return 0;

	double *ref = new double[count * 3];
	for (int i=0; i<count; i++)
	{
		ref[i] = positions[i*3];
		ref[count + i] = positions[i*3+1];
		ref[count*2 + i] = positions[i*3+2];
	}

	int *index = new int[count];
	for (int i=0; i<count; i++)	index[i] = i;
	mTree = build_kdtree( ref, count, 3, index, count, 0 );
	if (!mTree)
	{
		if(index!=nullptr)
		{
			delete [] index;
			index = nullptr;
		}
		if (ref!=nullptr)
		{
			delete [] ref;
			ref = nullptr;
		}
		return 0;
	}
	mTree->dims = 3;

	if(index!=nullptr)
	{
		delete [] index;
		index = nullptr;
	}
	if (ref!=nullptr)
	{
		delete [] ref;
		ref = nullptr;
	}
	return 1;
}

// free kd-tree
int KDTree::FreeKDTree() {
	
	if (mTree) {
		free_tree(mTree->rootptr);
		free(mTree);
		mTree = nullptr;
	}
	return 1;
}

int KDTree::SearchForNearest(const double *pos, const double dist)
{
	if (!mTree) return -1;

	double M=0.0;
	run_queries( mTree->rootptr, (double*) pos, 1, 3, &M, (double*) &dist, 102 ); // RETURN_INDEX - 102
	return (int) M;
}

int KDTree::SearchForNearest(const double *pos, const double dist, double *res)
{
	if (!mTree) return 0;

	run_queries( mTree->rootptr, (double*)pos, 1, 3, res, (double*) &dist, 101 ); // RETURN_POSITION - 102
	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	Creation function.
 ************************************************/
bool ORConstraint_Column::FBCreate()
{
	/*
	*	1. Create Reference group & elements
	*	2. Set constraint variables (deformer,layout,description)
	*	3. Set pointers to NULL if necessary
	*/
	Deformer	= false;
	HasLayout	= true;
	Description = "Constraint Column Play";
    

	FBPropertyPublish( this, DisplaceValue, "Displace Value", nullptr, nullptr );
	FBPropertyPublish( this, DisplaceRandom, "Displace Random", nullptr, nullptr );
	FBPropertyPublish( this, ConnectDistance, "Connect Distance", nullptr, nullptr );
	FBPropertyPublish( this, Curve, "Curve", nullptr, nullptr );
	FBPropertyPublish( this, CurveStep, "Curve Step", nullptr, nullptr );
	FBPropertyPublish( this, CurveThreshold, "Curve Threshold", nullptr, nullptr );
	FBPropertyPublish( this, EvaluateDeltaTime, "Evaluate Threshold", nullptr, nullptr );
	
	FBPropertyPublish( this, Mass, "Mass", nullptr, nullptr );
	FBPropertyPublish( this, Gravity, "Gravity", nullptr, nullptr );
	FBPropertyPublish( this, FloorLevel, "Floor Level", nullptr, nullptr );
	FBPropertyPublish( this, Damping, "Damping", nullptr, nullptr );
	FBPropertyPublish( this, Q, "Q", nullptr, nullptr );

	DisplaceValue.SetAnimated(true);
	DisplaceRandom = 5.0;

	Curve.SetFilter( FBModelPath3D::GetInternalClassId() );
	Curve.SetSingleConnect( true );

	ConnectDistance = 7.0;

	CurveStep = 10.0;		// * 0.01
	CurveThreshold = 5.0;
	EvaluateDeltaTime = 2.0;	// * 0.01

	Mass = 1.0;
	Gravity = -9.8;
	FloorLevel = 0.0;
	Damping = 20.0;	// * 0.01
	Q = 12.0;

	// Create reference groups
	mGroupSource	= ReferenceGroupAdd( "Input Objects",	MAX_NUMBER_OF_ITEMS );
	//mGroupConstrain	= ReferenceGroupAdd( "Output Objects",	MAX_NUMBER_OF_ITEMS );

	for (int i=0; i<MAX_NUMBER_OF_ITEMS; ++i)
	{
		mOutput[i] = nullptr;
	}

	//mQ = 12.0;
	//mDamping = 0.2;

	mLastTime = FBTime::Infinity;

	return true;
}

/************************************************
 *	Destruction function.
 ************************************************/
void ORConstraint_Column::FBDestroy()
{
	/*
	*	Free any user memory associated to constraint
	*/
	FreeColumns();
}

void ORConstraint_Column::FreeColumns()
{
	for (int i=0; i<mColumns.size(); ++i)
	{
		ColumnItem *pItem = mColumns[i];
		if (pItem)
		{
			delete pItem;
		}
		mColumns[i] = nullptr;
	}
}

/************************************************
 *	Refrence added notification.
 ************************************************/
bool ORConstraint_Column::ReferenceAddNotify( int pGroupIndex, FBModel* pModel )
{
	/*
	*	Perform action required when a refrence is added.
	*/
	return true;
}


/************************************************
 *	Reference removed notification.
 ************************************************/
bool ORConstraint_Column::ReferenceRemoveNotify( int pGroupIndex, FBModel* pModel )
{
	/*
	*	Perform action required when a refrence is removed.
	*/
	return true;
}


/************************************************
 *	Setup all of the animation nodes.
 ************************************************/
void ORConstraint_Column::SetupAllAnimationNodes()
{
	/*
	*	- Based on the existence of the references (ReferenceGet() function),
	*	  create the required animation nodes.
	*	- A source will use an Out node, whereas a destination will use
	*	  an In Node.
	*	- If the node is with respect to a deformation, then Bind the reference
	*	  to the deformation notification ( DeformerBind() )
	*/

	for (int i=0; i<MAX_NUMBER_OF_ITEMS; ++i)
	{
		mOutput[i] = nullptr;
	}
	
	FreeColumns();

	int count = ReferenceGetCount( mGroupSource );
	if (count > 0)
	{
		
		std::uniform_real_distribution<float> uni(0.0f, 1.0f); // guaranteed unbiased

		// build k-d tree
		mLevels.resize(count);
		mColumns.resize(count);
		//
		for (int i=0; i<count; ++i)
		{
			FBModel *pModel = ReferenceGet(mGroupSource, i);
			if (pModel)
			{
				//mInput[i] = AnimationNodeOutCreate( i*2, pModel, ANIMATIONNODE_TYPE_TRANSLATION );
				mOutput[i] = AnimationNodeInCreate	( i*2+1, pModel, ANIMATIONNODE_TYPE_TRANSLATION );

				pModel->GetVector( mLevels[i] );
				mLevels[i][1] = 0.0;	// dont take y in a count
			}

			mColumns[i] = new ColumnItem();
			mColumns[i]->randomF = uni(rng);
		}

		//
		mTree.BuildKDTree(count, mLevels[0] );

		//
		ConnectColumns(ConnectDistance);
	}

	mCount = count;
}


/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void ORConstraint_Column::RemoveAllAnimationNodes()
{
	/*
	*	If nodes have been bound to deformations, remove the binding.
	*/
	mCount = 0;

	FreeColumns();

	for (int i=0; i<MAX_NUMBER_OF_ITEMS; ++i)
	{
		//mInput[i] = nullptr;
		mOutput[i] = nullptr;
	}
}

/************************************************
 *	Suggest a snap.
 ************************************************/
void ORConstraint_Column::SnapSuggested()
{
	/*
	*	Perform any pre-snap operations.
	*/

	//
	ConnectColumns(ConnectDistance);
}


/************************************************
 *	Suggest a freeze.
 ************************************************/
void ORConstraint_Column::FreezeSuggested()
{
	/*
	*	Perform any pre-freeze operations
	*/
}


/************************************************
 *	Disable the constraint on pModel.
 ************************************************/
bool ORConstraint_Column::Disable(FBModel* pModel)
{
	/*
	*	Perform any operations to disable model
	*	Must call FBConstraint::Disable()
	*/
	return FBConstraint::Disable( pModel );
}

/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/

void ORConstraint_Column::ConnectColumns(const double dist_thres)
{
	for (int i=0; i<mColumns.size(); ++i)
	{
		FBVector3d pos( mLevels[i] );
		pos[1] = 0.0;
		mColumns[i]->ClearNeighbors();

		for (int j=0; j<mColumns.size(); ++j)
			if (i != j)
			{
				FBVector3d pos2( mLevels[j] );
				pos2[1] = 0.0;

				// calc dist
				double dist = VectorLength( VectorSubtract(pos, pos2) );

				// add neighbor
				if (dist < dist_thres)
					mColumns[i]->AddNeighbor( mColumns[j] );
			}

	}
}

void ORConstraint_Column::DoStep(const double floor_level, const double gravity, const double dt, const double q, const double damping, const double mass)
{
	double accel_c = -q*dt;
	double damping_mult = exp(-dt*damping);

	// apply grid wave
	for (auto iter=mColumns.begin(); iter!=mColumns.end(); ++iter)
	{
		ColumnItem *pItem = *iter;
		
		// apply gravity
		double inv_mass = 1.0 / mass;
		pItem->vel += gravity * inv_mass * dt; // F = ma

		// no connections, no wave !
		if (pItem->GetNumberOfNeighbors() > 0)
		{
			double avg_level = pItem->GetAverageLevel();

			// apply grid wave
			pItem->vel += accel_c * (pItem->pos - avg_level);
			pItem->vel *= damping_mult;
		}

		// update pos
		pItem->pos += pItem->vel * dt;

		// floor constraint
		if (pItem->pos < floor_level)
			pItem->pos = floor_level;
	}
}

void ORConstraint_Column::ReadValues(FBEvaluateInfo* pEvaluateInfo, double *displace, double *randomF, double *mass, double *gravity, double *floorlevel, double *damping, double *q)
{
	FBAnimationNode *pNode = DisplaceValue.GetAnimationNode();
	if (pNode)
		pNode->ReadData( displace, pEvaluateInfo );
	else
		*displace = DisplaceValue;

	//
	pNode = DisplaceRandom.GetAnimationNode();
	if (pNode)
		pNode->ReadData( randomF, pEvaluateInfo );
	else
		*randomF = DisplaceRandom;

	//
	pNode = Mass.GetAnimationNode();
	if (pNode)
		pNode->ReadData( mass, pEvaluateInfo );
	else
		*mass = Mass;

	if (*mass <= 0.0)
		*mass = 1.0;

	//
	pNode = Gravity.GetAnimationNode();
	if (pNode)
		pNode->ReadData( gravity, pEvaluateInfo );
	else
		*gravity = Gravity;

	//
	pNode = FloorLevel.GetAnimationNode();
	if (pNode)
		pNode->ReadData( floorlevel, pEvaluateInfo );
	else
		*floorlevel = FloorLevel;

	//
	pNode = Damping.GetAnimationNode();
	if (pNode)
		pNode->ReadData( damping, pEvaluateInfo );
	else
		*damping = Damping;

	*damping *= 0.01;

	//
	pNode = Q.GetAnimationNode();
	if (pNode)
		pNode->ReadData( q, pEvaluateInfo );
	else
		*q = Q;
}

bool ORConstraint_Column::AnimationNodeNotify(FBAnimationNode* pConnector,FBEvaluateInfo* pEvaluateInfo,FBConstraintInfo* pConstraintInfo)
{
	/*
	*	Constraint Evaluation
	*	1. Read data from sources
	*	2. Calculate necessary operations.
	*	3. Write output to destinations
	*	Note: Not the deformation operations !
	*/
	double displace, randomF, mass, gravity, floorlevel, damping, q;

	ReadValues(pEvaluateInfo, &displace, &randomF, &mass, &gravity, &floorlevel, &damping, &q);
	
	//
	// interpolate curve

	double step = 0.01 * CurveStep;
	double threshold = CurveThreshold;
	if (step <= 0.0)
		step = 0.1;

	// curve and raise constraint
	FBModelPath3D	*pcurve = (Curve.GetCount() > 0) ? (FBModelPath3D*)Curve[0] : nullptr;
	if (pcurve)
	{
		double percent = 0.0;
		while (percent <= 100.0)
		{
			FBVector4d point = pcurve->Total_GlobalPathEvaluate(percent);
			FBVector3d p3(point);

			int index = mTree.SearchForNearest( point, threshold );
			if (index >= 0)
				mColumns[index]->Raise(displace + randomF * mColumns[index]->GetRandomF() );

			percent += step;
		}
		
	}

	//
	// some physics wave calculations
	
	FBTime currTime(pEvaluateInfo->GetSystemTime() );

	FBTime diff = currTime - mLastTime;
	if (diff.GetSecondDouble() > 2.0)
		mLastTime = currTime;

	diff = currTime - mLastTime;
	double diffSeconds = diff.GetSecondDouble();
	const double dt = 0.01 * EvaluateDeltaTime;

	while (diffSeconds > 0.0)
	{
		DoStep(floorlevel, gravity, dt, q, damping, mass);

		diffSeconds -= dt;
	}

	mLastTime = currTime;

	//
	/// output result
	//

	for (int i=0; i<mCount; ++i)
	{
		//mInput[i]->ReadData( lPosition, pEvaluateInfo );

		//lPosition[1] = mLevels[i];
		mLevels[i][1] = mColumns[i]->pos;
		mOutput[i]->WriteData( mLevels[i], pEvaluateInfo );
	}

	return true;
}


/************************************************
 *	Real-Time Deformer Evaluation.
 ************************************************/
bool ORConstraint_Column::DeformerNotify(FBModel* pModel,const FBVertex*  pSrcVertex,const FBVertex* pSrcNormal,int pCount,FBVertex*  pDstVertex,FBVertex*  pDstNormal)
{
	/*
	*	1. Get the data from the source vertices
	*	2. Calculate the necessary operations
	*	3. Write the output to the destination vertices.
	*	Note: Not the non-deformation operations !
	*/
	return true;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool ORConstraint_Column::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	FBX Storage of constraint parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORConstraint_Column::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	FBX Retrieval of constraint parameters.
	*/
	return true;
}


/************************************************
 *	Deformer Binding.
 ************************************************/
bool ORConstraint_Column::DeformerBind( FBModel* pModel )
{
	/*
	*	Perform the operations for the binding of a model to be deformed
	*	Must call FBConstraint::DeformerBind()
	*/
	return FBConstraint::DeformerBind( pModel );
}


/************************************************
 *	Deformer Unbinding.
 ************************************************/
bool ORConstraint_Column::DeformerUnBind( FBModel* pModel )
{
	/*
	*	Perform the operations for the unbinding of a model being deformed
	*	Must call FBConstraint::DeformerUnBind()
	*/
	return FBConstraint::DeformerUnBind( pModel );
}


/************************************************
 *	Freeze SRT for the constraint.
 ************************************************/
void ORConstraint_Column::FreezeSRT	( FBModel* pModel, bool pS, bool pR, bool pT )
{
	/*
	*	Freeze the model's SRT parameters (if true,true,true)
	*	Must call FBConstraint::FreezeSRT
	*/
	FBConstraint::FreezeSRT( pModel, pS, pR, pT );
}

