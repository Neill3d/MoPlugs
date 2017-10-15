
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: queryFBGeometry.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "..\Common_Physics\physics_common.h"

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "algorithm\kdtree_common.h"

#include <vector>

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
	int SearchForNearest(const double *pos, const double dist) const;

	int SearchForNearest(const double *pos, const double dist, double *res) const;

private:
	Tree					*mTree;		// k-d tree
};

/////////////////////////////////////////////////////////////////////////////////////
//

class QueryFBPath3D : public PHYSICS_INTERFACE::IQueryPath
{
public:

	//! a constructor
	QueryFBPath3D();
	QueryFBPath3D(FBModelPath3D *pModel, const double globalScaling=1.0, const int numberOfSteps = 20);

	//! a destructor
	virtual ~QueryFBPath3D();

	//
	void Prep(FBModelPath3D *pModel, const double globalScaling=1.0, const int numberOfSteps = 20);

	//
	virtual void	GetCurrentMatrix(float *values) const override;
	virtual void	GetCurrentMatrixD(double *values) const override;
	virtual void	CurveDerivative (double *result, double u, int index = 1) const;
	virtual double	FindClosestKnot (double *closestPointOnCurve, const double *point, double dist_thres, int subdivitionSteps = 2) const;

	void	DrawDebug() const;

private:
	double									mGlobalScaling;
	double									mStep;

	FBVector3d								mLastSearchPoint;
	FBVector3d								mLastNearestPoint;

	double									mLastU;
	FBVector3d								mLastDerivativeRoot;
	FBVector3d								mLastDerivative;

	KDTree									mTree;

#ifdef ORSDK2013
	FBModelPath3D							*mCurve;
#else
	HdlFBPlugTemplate<FBModelPath3D>		mCurve;
#endif
};

/////////////////////////////////////////////////////////////////////////////////////
//

class QueryFBGeometry : public PHYSICS_INTERFACE::IQueryGeometry
{
public:
	//! a constructor
	QueryFBGeometry();
	QueryFBGeometry(FBModel *pTMModel, FBModel *pColModel, bool doAlign, FBModel *pParentModel, FBMatrix *alignMatrix, FBMatrix *visualAlignMatrix);

	//! a destructor
	virtual ~QueryFBGeometry();

	/*
		
		Run this method before all other or use simular form of a constructor

	*/
	bool Prep(FBModel *pModel, FBModel *pColModel, bool doAlign, FBModel *pParentModel, FBMatrix *alignMatrix, FBMatrix *visualAlignMatrix);
	
	
	virtual void GetBoundingBox(float *min_vector, float *max_vector) const override;
	virtual void GetBoundingBoxD(double *min_vector, double *max_vector) const override;

	/*

		Run this function to cache data for GetPosition, GetRotation and GetScale methods

	*/
	virtual void PrepMatrix(bool global) const;

	virtual const double	*GetMatrix(bool global) const override;
	virtual void			GetMatrixTR(bool global, double *values) const override;
	virtual void			GetMatrixTR_f(bool global, float *values) const override;

	virtual void			GetVisualAlignMatrix(float *values) const;
	virtual void			GetVisualAlignMatrixD(double *values) const override;

	virtual const double	*GetPosition(bool global) const override;
	virtual void			GetPositionD(double *values) const override;
	virtual void			GetPositionF(float *values) const override;

	virtual const double	*GetRotation(bool global) const override;
	virtual void			GetRotationD(double *values) const override;
	virtual void			GetRotationF(float *values) const override;

	virtual const double	*GetQuaternion(bool global) const override;
	virtual void			GetQuaternionD(double *values) const override;
	virtual void			GetQuaternionF(float *values) const override;

	virtual const double	*GetScale(bool global) const override;
	virtual void			GetScaleD(double *values) const override;
	virtual void			GetScaleF(float *values) const override;

	//

	virtual const int		GetVertexCount() const;
	virtual const float		*GetVertexPosition(const int index) const;

	virtual const int		GetPolyCount() const;
	virtual const Poly		*GetPoly(const int index) const;
	
	// function to record a new keyframe
	/*
	void	SetAnimNodes(FBAnimationNode *translation, FBAnimationNode *rotation);

	virtual void RecordTransform( const double time, const float *matrix) override;
	*/
private:

	//FBPlayerControl				mPlayerControl;

#ifdef ORSDK2013
	FBModel						*mModel;
#else
	HdlFBPlugTemplate<FBModel>	mModel;
#endif
	FBMatrix					matrix;

#ifdef ORSDK2013
	FBModel						*mCollision;
#else
	HdlFBPlugTemplate<FBModel>	mCollision;
#endif
	bool						mTMAsACollision;
	FBMatrix					initMatrix;

	
	FBVector3d					mPosition;
	FBVector3d					mRotation;
	FBVector3d					mScale;
	FBVector4d					mQuat;

	FBVector3d					pMin;
	FBVector3d					pMax;

	FBMatrix					mInvAlignMatrix;
	FBMatrix					mVisualAlignMatrix;

	int							mNumberOfVertices;
	FBVertex					*vertices;

	// pre-bake startup transformation (for car to orient +X)
	std::vector<FBVertex>		bakeVertices;
	/*
	// for physics state recording
	FBAnimationNode				*mAnimTranslation;
	FBAnimationNode				*mAnimRotation;
	*/
};

/////////////////////////////////////////////////////////////////////////////////////
// LevelGeometry

class LevelGeometry : public PHYSICS_INTERFACE::IQueryGeometry
{
public:

	//! a constructor
	LevelGeometry();
	LevelGeometry( FBComponentList &list );

	//! a destructor
	virtual ~LevelGeometry();

	//
	void	Prep( FBComponentList &list );

	//

	virtual const int		GetVertexCount() const;
	virtual const float		*GetVertexPosition(const int index) const;

	virtual const int		GetPolyCount() const;
	virtual const Poly		*GetPoly(const int index) const;

private:

	int							mNumberOfVertices;
	int							mNumberOfPolys;

	std::vector<FBVertex>		mVertices;
	std::vector<Poly>			mPolys;

	void Allocate(int numberOfVerts, int numberOfPolys);
	void Free();
};