
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: queryFBGeometry.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "queryFBGeometry.h"
#include <GL\glew.h>
//////////////////////////////////////////////////////////////////////////////
// QueryFBGeometry

//! a constructor
QueryFBGeometry::QueryFBGeometry()
	: mNumberOfVertices(0)
	, vertices(nullptr)
{
	mVisualAlignMatrix.Identity();
	//mAnimTranslation = nullptr;
	//mAnimRotation = nullptr;
}

QueryFBGeometry::QueryFBGeometry(FBModel *pModel, FBModel *pColModel, bool doAlign, FBModel *pParentModel, FBMatrix *alignMatrix, FBMatrix *visualAlignMatrix)
	: mNumberOfVertices(0)
	, vertices(nullptr)
{
	mVisualAlignMatrix.Identity();
	Prep(pModel, pColModel, doAlign, pParentModel, alignMatrix, visualAlignMatrix);
	//mAnimTranslation = nullptr;
	//mAnimRotation = nullptr;
}

//! a destructor
QueryFBGeometry::~QueryFBGeometry()
{}

// DONE: apply transform matrix for vertices
bool QueryFBGeometry::Prep(FBModel *pModel, FBModel *pColModel, bool doAlign, FBModel *pParentModel, FBMatrix *alignMatrix, FBMatrix *visualAlignMatrix)
{
	if (pModel == nullptr || pColModel == nullptr)
		return false;

	mModel = pModel;
	mCollision = pColModel;

	FBGeometry *pGeom = pColModel->Geometry;
	if (pGeom == nullptr)
		return false;

#ifdef ORSDK2013
	mNumberOfVertices = pGeom->VertexCount();
	vertices = pGeom->GetVertexes();
#else
	vertices = pGeom->GetPositionsArray(mNumberOfVertices);
#endif

	if (visualAlignMatrix)
		mVisualAlignMatrix = *visualAlignMatrix;
	else
		mVisualAlignMatrix.Identity();

	mTMAsACollision = (pModel == pColModel);
	
	//
	//

	FBVector3d rot, scale, pos, posCol;
	FBQuaternion q, q1, q2;

	pModel->GetVector(pos, kModelTranslation, true);
	pColModel->GetVector(rot, kModelRotation, true);
	pColModel->GetVector(scale, kModelScaling, true);
	pColModel->GetVector(posCol, kModelTranslation, true);

	FBTRSToMatrix( initMatrix, FBTVector(posCol[0]-pos[0], posCol[1]-pos[1], posCol[2]-pos[2]), rot, FBSVector(scale[0], scale[1], scale[2]) );
	
	FBMatrix identity;
	identity.Identity();

	
	if (initMatrix != identity)
	{
		bakeVertices.resize(mNumberOfVertices);

		for (int i=0; i<mNumberOfVertices; ++i)
		{
			FBVertexMatrixMult( bakeVertices[i], initMatrix, vertices[i] );
		}

		vertices = &bakeVertices[0];
	}

	// apply align pivot rotation
	mInvAlignMatrix.Identity();

	if (alignMatrix)
	{
		initMatrix = *alignMatrix;

		if (initMatrix != identity)
		{
			bakeVertices.resize(mNumberOfVertices);

			for (int i=0; i<mNumberOfVertices; ++i)
			{
				FBVertexMatrixMult( bakeVertices[i], initMatrix, vertices[i] );
			}

			vertices = &bakeVertices[0];
		}
	
		// transform back offset
		mInvAlignMatrix = *alignMatrix;
		mInvAlignMatrix.Transpose();
	}

	return true;
}


void QueryFBGeometry::GetBoundingBox(float *min_vector, float *max_vector) const
{
#ifdef ORSDK2013
	if (nullptr == mCollision)
		return;
#else
	if (mCollision.Ok() == false)
		return;
#endif
	mCollision->GetBoundingBox( (FBVector3d&) pMin, (FBVector3d&) pMax);

	FBVector4d min4( pMin[0], pMin[1], pMin[2], 1.0 );
	FBVector4d max4( pMax[0], pMax[1], pMax[2], 1.0 );

	FBVectorMatrixMult( min4, initMatrix, min4 );
	FBVectorMatrixMult( max4, initMatrix, max4 );

	for (int i=0; i<3; ++i)
	{
		min_vector[i] = min4[i];
		max_vector[i] = max4[i];
	}
}

void QueryFBGeometry::GetBoundingBoxD(double *min_vector, double *max_vector) const
{
#ifdef ORSDK2013
	if (nullptr == mCollision)
		return;
#else
	if (mCollision.Ok() == false)
		return;
#endif
	mCollision->GetBoundingBox( (FBVector3d&) pMin, (FBVector3d&) pMax);

	FBVector4d min4( pMin[0], pMin[1], pMin[2], 1.0 );
	FBVector4d max4( pMax[0], pMax[1], pMax[2], 1.0 );

	FBVectorMatrixMult( min4, initMatrix, min4 );
	FBVectorMatrixMult( max4, initMatrix, max4 );

	for (int i=0; i<3; ++i)
	{
		min_vector[i] = min4[i];
		max_vector[i] = max4[i];
	}
}

void QueryFBGeometry::PrepMatrix(bool global) const
{
#ifdef ORSDK2013
	if (nullptr != mModel)
#else
	if (mModel.Ok() )
#endif
	{
		mModel->GetMatrix( (FBMatrix&) matrix, kModelTransformation, global);
		mModel->GetVector( (FBVector3d&)mPosition, kModelTranslation, global);
		mModel->GetVector( (FBVector3d&)mRotation, kModelRotation, global);
		FBRotationToQuaternion( (FBQuaternion&)mQuat, mRotation, kFBZYX);
		mModel->GetVector( (FBVector3d&)mScale, kModelScaling, global);
	}
}

const double *QueryFBGeometry::GetMatrix(bool global) const
{
#ifdef ORSDK2013
	if (nullptr != mModel)
#else
	if (mModel.Ok())
#endif
	{
		mModel->GetMatrix( (FBMatrix&) matrix, kModelTransformation, global);
	}
	return matrix;
}

void QueryFBGeometry::GetMatrixTR_f(bool global, float *values) const
{
#ifdef ORSDK2013
	if (nullptr != mModel)
#else
	if (mModel.Ok() )
#endif
	{
		FBModel *pParent = mModel->Parent;
		if (global == false && pParent != nullptr)
		{
			FBMatrix parentMatrix;

			pParent->GetMatrix( (FBMatrix&) parentMatrix, kModelRotation, true);
			pParent->GetVector( (FBVector3d&) mPosition, kModelTranslation, true); 

			//parentMatrix.Set( mInvAlignMatrix );
			memcpy( parentMatrix, mInvAlignMatrix, sizeof(double) * 16 );

			parentMatrix[12] = mPosition[0];
			parentMatrix[13] = mPosition[1];
			parentMatrix[14] = mPosition[2];

			//
			mModel->GetMatrix( (FBMatrix&) matrix, kModelRotation, true);
			mModel->GetVector( (FBVector3d&)mPosition, kModelTranslation, true); 

			matrix[12] = mPosition[0];
			matrix[13] = mPosition[1];
			matrix[14] = mPosition[2];

			//
			FBGetLocalMatrix( (FBMatrix&) matrix, parentMatrix, matrix );
		}
		else
		{
			mModel->GetMatrix( (FBMatrix&) matrix, kModelRotation, global);
			mModel->GetVector( (FBVector3d&)mPosition, kModelTranslation, global); 

			FBMatrixMult( (FBMatrix&) matrix, matrix, mInvAlignMatrix );
			
			memcpy( matrix, mInvAlignMatrix, sizeof(double) * 16 );
			//matrix.Set( mInvAlignMatrix );
			//matrix.Set(mInvAlignMatrix.GetData());

			matrix[12] = mPosition[0];
			matrix[13] = mPosition[1];
			matrix[14] = mPosition[2];
		}

		for (int i=0; i<16; ++i)
			values[i] = (float) matrix[i];
	}
}

void QueryFBGeometry::GetMatrixTR(bool global, double *values) const
{
#ifdef ORSDK2013
	if (nullptr != mModel)
#else
	if (mModel.Ok() )
#endif
	{
		FBModel *pParent = mModel->Parent;
		if (global == false && pParent != nullptr)
		{
			FBMatrix parentMatrix;

			pParent->GetMatrix( (FBMatrix&) parentMatrix, kModelRotation, true);
			pParent->GetVector( (FBVector3d&) mPosition, kModelTranslation, true); 

			//parentMatrix.Set( mInvAlignMatrix );
			memcpy( parentMatrix, mInvAlignMatrix, sizeof(double) * 16 );

			parentMatrix[12] = mPosition[0];
			parentMatrix[13] = mPosition[1];
			parentMatrix[14] = mPosition[2];

			//
			mModel->GetMatrix( (FBMatrix&) matrix, kModelRotation, true);
			mModel->GetVector( (FBVector3d&)mPosition, kModelTranslation, true); 

			matrix[12] = mPosition[0];
			matrix[13] = mPosition[1];
			matrix[14] = mPosition[2];

			//
			FBGetLocalMatrix( (FBMatrix&) matrix, parentMatrix, matrix );
		}
		else
		{
			mModel->GetMatrix( (FBMatrix&) matrix, kModelRotation, global);
			mModel->GetVector( (FBVector3d&)mPosition, kModelTranslation, global); 

			FBMatrixMult( (FBMatrix&) matrix, matrix, mInvAlignMatrix );
			
			memcpy( matrix, mInvAlignMatrix, sizeof(double) * 16 );
			//matrix.Set( mInvAlignMatrix );
			//matrix.Set(mInvAlignMatrix.GetData());

			matrix[12] = mPosition[0];
			matrix[13] = mPosition[1];
			matrix[14] = mPosition[2];
		}

		for (int i=0; i<16; ++i)
			values[i] = matrix[i];
	}
}

void QueryFBGeometry::GetVisualAlignMatrix(float *values) const
{
	for (int i=0; i<16; ++i)
		values[i] = (float) mVisualAlignMatrix[i];
}

void QueryFBGeometry::GetVisualAlignMatrixD(double *values) const
{
	for (int i=0; i<16; ++i)
		values[i] = mVisualAlignMatrix[i];
}

const double *QueryFBGeometry::GetPosition(bool global) const
{
#ifdef ORSDK2013
	if (nullptr != mModel)
#else
	if (mModel.Ok() )
#endif
	{
		mModel->GetVector( (FBVector3d&)mPosition, kModelTranslation, global);
	}
	return mPosition;
}

void QueryFBGeometry::GetPositionF(float *values) const
{
	values[0] = (float) mPosition[0];
	values[1] = (float) mPosition[1];
	values[2] = (float) mPosition[2];
}

void QueryFBGeometry::GetPositionD(double *values) const
{
	values[0] = mPosition[0];
	values[1] = mPosition[1];
	values[2] = mPosition[2];
}


const double *QueryFBGeometry::GetRotation(bool global) const
{
#ifdef ORSDK2013
	if (nullptr != mModel)
#else
	if (mModel.Ok() )
#endif
	{
		mModel->GetVector( (FBVector3d&)mRotation, kModelRotation, global);
	}
	return mRotation;
}

void QueryFBGeometry::GetRotationF(float *values) const
{
	values[0] = (float) mRotation[0];
	values[1] = (float) mRotation[1];
	values[2] = (float) mRotation[2];
}

void QueryFBGeometry::GetRotationD(double *values) const
{
	values[0] = mRotation[0];
	values[1] = mRotation[1];
	values[2] = mRotation[2];
}

const double *QueryFBGeometry::GetQuaternion(bool global) const
{
#ifdef ORSDK2013
	if (nullptr != mModel)
#else		
	if (mModel.Ok() )
#endif
	{
		mModel->GetVector( (FBVector3d&)mRotation, kModelRotation, global);
	}
	FBRotationToQuaternion( (FBQuaternion&)mQuat, mRotation, kFBZYX);
	return mQuat;
}

void QueryFBGeometry::GetQuaternionF(float *values) const
{
	values[0] = (float) mQuat[0];
	values[1] = (float) mQuat[1];
	values[2] = (float) mQuat[2];
	values[3] = (float) mQuat[2];
}

void QueryFBGeometry::GetQuaternionD(double *values) const
{
	values[0] = mQuat[0];
	values[1] = mQuat[1];
	values[2] = mQuat[2];
	values[3] = mQuat[2];
}

const double *QueryFBGeometry::GetScale(bool global) const
{
#ifdef ORSDK2013
	if (nullptr != mModel)
#else
	if (mModel.Ok() )
#endif
	{
		mModel->GetVector( (FBVector3d&)mScale, kModelScaling, global);
	}
	return mScale;
}

void QueryFBGeometry::GetScaleF(float *values) const
{
	values[0] = (float) mScale[0];
	values[1] = (float) mScale[1];
	values[2] = (float) mScale[2];
}

void QueryFBGeometry::GetScaleD(double *values) const
{
	values[0] = mScale[0];
	values[1] = mScale[1];
	values[2] = mScale[2];
}

const int QueryFBGeometry::GetVertexCount() const {
	return mNumberOfVertices;
}
const float *QueryFBGeometry::GetVertexPosition(const int index) const {
	return vertices[index];
}

//

const int QueryFBGeometry::GetPolyCount() const {
	return 0;
}
const QueryFBGeometry::Poly *QueryFBGeometry::GetPoly(const int index) const {
	return nullptr;
}
/*
// function to record a new keyframe
void QueryFBGeometry::SetAnimNodes(FBAnimationNode *translation, FBAnimationNode *rotation)
{
	mAnimTranslation = translation;
	mAnimRotation = rotation;
}

void QueryFBGeometry::RecordTransform( const double _time, const float *matrix) 
{
	
	if (mPlayerControl.IsRecording && mPlayerControl.GetTransportMode() == kFBTransportPlay)
	{
		FBAnimationNode *Data;

		FBTime fbtime;
		fbtime.SetSecondDouble(_time);

		FBMatrix m;
		for (int i=0; i<16; ++i)
			m[i] = (double) matrix[i];

		if (mAnimTranslation)
		{
			FBTVector Pos( &m[12] );

			Data = mAnimTranslation->GetAnimationToRecord();
			if (Data)
			{
				Data->KeyAdd( fbtime, Pos );	// TODO: how to get time ?!
			}
		}
	}
	
}
*/

////////////////////////////////////////////////////////////////////////////////////
// QueryLevelGeometry

//! a constructor
LevelGeometry::LevelGeometry()
	: mNumberOfVertices(0)
	, mNumberOfPolys(0)
{
}

LevelGeometry::LevelGeometry( FBComponentList &list )
	: mNumberOfVertices(0)
	, mNumberOfPolys(0)
{
	Prep(list);
}

//! a destructor
LevelGeometry::~LevelGeometry()
{
	Free();
}

//
void LevelGeometry::Prep( FBComponentList &list )
{
	int totalNumberOfVerts = 0;
	int totalNumberOfPolys = 0;


	for (int i=0; i<list.GetCount(); ++i)
	{
		FBModel *pModel = (FBModel*) list[i];
		FBGeometry *pGeometry = pModel->Geometry;
		FBMesh *pMesh = pModel->TessellatedMesh; // (FBMesh*) pGeometry;

		totalNumberOfVerts += pMesh->VertexCount();

		for (int j=0; j<pMesh->PolygonCount(); ++j)
			if (pMesh->PolygonVertexCount(j) <= 4) 
			{
				totalNumberOfPolys++;
			}
		
	}

	Allocate(totalNumberOfVerts, totalNumberOfPolys);

	totalNumberOfVerts = 0;
	totalNumberOfPolys = 0;

	auto dstVertex = mVertices.begin();
	auto dstPoly = mPolys.begin();

	for (int i=0; i<list.GetCount(); ++i)
	{
		FBModel *pModel = (FBModel*) list[i];
		FBGeometry *pGeometry = pModel->Geometry;
		FBMesh *pMesh = pModel->TessellatedMesh; // (FBMesh*) pGeometry;

		FBMatrix tm;
		pModel->GetMatrix(tm);

		//
		int count = 0;
		
		FBVertex *vertices = nullptr;
#ifdef ORSDK2013
		count = pMesh->VertexCount();
		vertices = pMesh->GetVertexes();
#else
		vertices = pMesh->GetPositionsArray(count);
#endif	
		// apply TM to make a finish coords of the vertices
		FBVertex *srcVertex = vertices;
			
		for (int j=0; j<count; ++j)
		{
			FBVertexMatrixMult( *dstVertex, tm, *srcVertex );
			dstVertex++;
			srcVertex++;
		}

		//
		for (int j=0; j<pMesh->PolygonCount(); ++j)
		{
			int polyVertCount = pMesh->PolygonVertexCount(j);

			if (polyVertCount <= 4)
			{
				mPolys[totalNumberOfPolys].count = polyVertCount;

				// SHIFT indices
				for (int k=0; k<polyVertCount; ++k)
					mPolys[totalNumberOfPolys].indices[k] = totalNumberOfVerts + pMesh->PolygonVertexIndex(j, k);
				mPolys[totalNumberOfPolys].matId = 0;

				totalNumberOfPolys++;
			}
		}
		

		//
		totalNumberOfVerts += count;
	}
}

const int LevelGeometry::GetVertexCount() const 
{
	return (int) mVertices.size();
}

const float *LevelGeometry::GetVertexPosition(const int index) const 
{
	return &mVertices[index][0];
}

//

const int LevelGeometry::GetPolyCount() const 
{
	return (int) mPolys.size();
}

const LevelGeometry::Poly *LevelGeometry::GetPoly(const int index) const 
{
	return &mPolys[index];
}


void LevelGeometry::Allocate(int numberOfVerts, int numberOfPolys)
{
	mVertices.resize(numberOfVerts);
	mPolys.resize(numberOfPolys);

	mNumberOfVertices = numberOfVerts;
	mNumberOfPolys = numberOfPolys;
}

void LevelGeometry::Free()
{
	mNumberOfVertices = 0;
	mNumberOfPolys = 0;

	mVertices.clear();
	mPolys.clear();
}


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

int KDTree::SearchForNearest(const double *pos, const double dist) const
{
	if (!mTree) return -1;

	double M=0.0;
	run_queries( mTree->rootptr, (double*) pos, 1, 3, &M, (double*) &dist, 102 ); // RETURN_INDEX - 102
	return (int) M;
}

int KDTree::SearchForNearest(const double *pos, const double dist, double *res) const
{
	if (!mTree) return 0;

	run_queries( mTree->rootptr, (double*)pos, 1, 3, res, (double*) &dist, 101 ); // RETURN_POSITION - 102
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// QueryFBPath3D

QueryFBPath3D::QueryFBPath3D()
{
	mGlobalScaling = 1.0;
	mStep = 0.05;
}

QueryFBPath3D::QueryFBPath3D(FBModelPath3D *pModel, const double globalScaling, const int numberOfSteps)
{
	mGlobalScaling = globalScaling;
	mStep = 0.05;
	Prep(pModel, globalScaling);
}

QueryFBPath3D::~QueryFBPath3D()
{
}

void QueryFBPath3D::Prep(FBModelPath3D *pModel, const double globalScaling, const int numberOfSteps)
{
	mCurve = pModel;
#ifdef ORSDK2013
	if (nullptr != mCurve || numberOfSteps <= 0)
#else
	if (mCurve.Ok() == false || numberOfSteps <= 0)
#endif
	{
		return;
	}

	mGlobalScaling = globalScaling;
	//const int numberOfSteps = 1.0 / mStep;
	mStep = 1.0 / numberOfSteps;

	std::vector<FBVector3d>		positions(numberOfSteps, FBVector3d() );

	double u = 0.0;
	for (int i=0; i<numberOfSteps; ++i)
	{
		FBVector4d vec = mCurve->Total_GlobalPathEvaluate(u * 100.0);
		positions[i] = FBVector3d( vec[0]*globalScaling, vec[1]*globalScaling, vec[2]*globalScaling );

		u += mStep;
	}

	mTree.BuildKDTree( (int) positions.size(), positions[0] );
}

void QueryFBPath3D::GetCurrentMatrix(float *values) const
{
#ifdef ORSDK2013
	if (nullptr != mCurve)
#else
	if (mCurve.Ok() == false)
#endif
	{
		return;
	}

	FBMatrix matrix;
	matrix.Identity();

	//mCurve->GetMatrix(matrix);

	for (int i=0; i<16; ++i)
		values[i] = (float) matrix[i];
}

void QueryFBPath3D::GetCurrentMatrixD(double *values) const
{
#ifdef ORSDK2013
	if (nullptr != mCurve)
#else
	if (mCurve.Ok() == false)
#endif
	{
		return;
	}

	FBMatrix matrix;
	matrix.Identity();

	//mCurve->GetMatrix(matrix);

	for (int i=0; i<16; ++i)
		values[i] = matrix[i];
}


void QueryFBPath3D::CurveDerivative (double *result, double u, int index) const
{
#ifdef ORSDK2013
	if (nullptr != mCurve)
#else
	if (mCurve.Ok() == false)
#endif
	{
		return;
	}

	FBVector4d d, d2;

	d = mCurve->Total_GlobalPathEvaluateDerivative(u * 100.0);
	result[0] = d[0] * mGlobalScaling;
	result[1] = d[1] * mGlobalScaling;
	result[2] = d[2] * mGlobalScaling;

	//mLastU = u * 100.0;
	d = mCurve->Total_GlobalPathEvaluate(u * 100.0);
	mLastDerivativeRoot[0] = d[0] * mGlobalScaling;
	mLastDerivativeRoot[1] = d[1] * mGlobalScaling;
	mLastDerivativeRoot[2] = d[2] * mGlobalScaling;

	d2 = mCurve->Total_GlobalPathEvaluateDerivative(u * 100.0);
	mLastDerivative[0] = d2[0] * mGlobalScaling;
	mLastDerivative[1] = d2[1] * mGlobalScaling;
	mLastDerivative[2] = d2[2] * mGlobalScaling;

	FBSub( d, d2, d );
	FBMult( d, d, 1.0 / FBLength(d) );

	result[0] = d[0];
	result[1] = d[1];
	result[2] = d[2];
}

double QueryFBPath3D::FindClosestKnot (double *closestPointOnCurve, const double *point, double dist_thres, int subdivitionSteps) const
{
#ifdef ORSDK2013
	if (nullptr != mCurve)
#else
	if (mCurve.Ok() == false)
#endif
	{
		return 0.0;
	}

	mLastSearchPoint[0] = point[0];
	mLastSearchPoint[1] = point[1];
	mLastSearchPoint[2] = point[2];

	// find closest control points, find closest point in the subdivided segments
	// kd-tree ?
	int result = mTree.SearchForNearest( point, dist_thres );
	mTree.SearchForNearest( point, dist_thres, closestPointOnCurve );

	mLastNearestPoint[0] = closestPointOnCurve[0];
	mLastNearestPoint[1] = closestPointOnCurve[1];
	mLastNearestPoint[2] = closestPointOnCurve[2];

	if (result >= 0)
		return (mStep * result);
	
	return 0.0;
}

void QueryFBPath3D::DrawDebug() const
{
	double backScaling = 1.0 / mGlobalScaling;

	glPushMatrix();

	glScaled(backScaling, backScaling, backScaling);
	glPointSize(5.0f);
	
	glBegin(GL_POINTS);
	
		glColor3d(0.0, 1.0, 0.0);
		glVertex3dv( mLastSearchPoint );

		glColor3d(1.0, 0.0, 0.0);
		glVertex3dv( mLastNearestPoint );
	glEnd();
	glPointSize(1.0f);

	glLineWidth(3.0f);
	glColor3d(0.0, 0.0, 0.0);
	glBegin(GL_LINES);

	glVertex3dv( mLastDerivativeRoot);
	glVertex3dv( mLastDerivative);

	glEnd();
	glLineWidth(1.0f);

	glPopMatrix();
}
