
#pragma once


//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: pathwrap_constraint.h
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
#include <map>

#define ORCONSTRAINTPATHTEST__CLASSNAME		ORConstraintPathTest
#define ORCONSTRAINTPATHTEST__CLASSSTR		"ORConstraintPathTest"

#define ORCONSTRAINTPATHWRAP__CLASSNAME		ORConstraintPathWrap
#define ORCONSTRAINTPATHWRAP__CLASSSTR		"ORConstraintPathWrap"

#ifndef MAX_NUMBER_OF_BRICKS		
#define	MAX_NUMBER_OF_BRICKS		1024
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////
//! A test animatino constraint class


class ORConstraintPathTest : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( ORConstraintPathTest, FBConstraint );

public:

	FBPropertyAnimatableDouble		Percent;		// path percent to recompute attached object

public:
	//--- Creation & Destruction
	virtual bool			FBCreate() override;							//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	virtual void			SnapSuggested() override;

	virtual void SetupAllAnimationNodes() override;		//!< Setup animation nodes.
	
	/** Notification callback for connectors.
	*	\param	pAnimationNode		Animation node being notified.
	*	\param	pEvaluateInfo		Information for evaluation.
	*	\param	pConstraintInfo		Information for constraint.
	*	\return \b true if successful.
	*/
	virtual bool AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo ) override;


protected:

	FBAnimationNode		*mSourceCurve;		// 3d curve, path along which we animate our bricks
	FBAnimationNode		*mConstraintPosition;
	FBAnimationNode		*mConstraintRotation;

	int						mGroupCurve;		//!< Source groupe index.
	int						mGroupConstrained;

	FBMatrix			mInitialTM;

	double				mCreationPercent;
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//! A curve deformer constraint class

class ORConstraintPathWrap : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( ORConstraintPathWrap, FBConstraint );

public:

	FBPropertyInt			CurveSubdivisions;	// used for point placement
	FBPropertyInt			CurveSegmentSubdivisions;
	FBPropertyInt			ControlAxis;

public:
	//--- Creation & Destruction
	virtual bool			FBCreate() override;							//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	virtual bool ReferenceAddNotify		( int pGroupIndex, FBModel* pModel ) override;
	virtual bool ReferenceRemoveNotify	( int pGroupIndex, FBModel* pModel ) override;

	virtual void			SnapSuggested() override;

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
									FBVertex*  pDstVertex,FBVertex*  pDstNormal) override;

protected:

	FBAnimationNode		*mSourceCurve;		// 3d curve, path along which we animate our bricks


	int						mGroupCurve;		//!< Source groupe index.
	int						mGroupDeformed;

	struct ElementData
	{
		bool needCompute;

		double		creationPercent;
		double		length;		// computed from bounding box X value
		double		bbMin;

		FBVector3d	translation;
		FBVector3d	rotation;
		FBVector3d	scaling;
		
		FBMatrix	originTM;
		FBMatrix	localTM;

		bool		cached;
		std::vector<FBVertex>	cache;
	};

	std::map<FBModel*, ElementData*>		mModelsData;

	void ComputeModelData( FBModel *pModel, ElementData *pData );

};