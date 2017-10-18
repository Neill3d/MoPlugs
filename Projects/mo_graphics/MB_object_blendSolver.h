
// object to blend normals and textures for a model blendshapes

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_object_blendSolver.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <GL\glew.h>
#include <map>

#include "algorithm\nv_math.h"

#include "shaderCallbacks_blendshapeCompute.h"

#define OBJECT_BLENDSOLVER__CLASSNAME			ObjectBlendSolver
#define OBJECT_BLENDSOLVER__CLASSSTR			"Object Blend Solver"


struct ClusterData
{
	// precached data
	FBMatrix	bindTM;

	FBModel		*pModel;
	FBModel		*pModel2;		// associate model

	FBClusterMode		mode;
	double				accuracy;

	// pointer to a cache element
	mat4		*pCacheOutput;
};

struct ClusterVertex
{

	// by default, should be limited by 4 links per vertex
	vec4		links;	// index to joint transform
	vec4		weights;

	//
	void Clear()
	{
		links = vec4(-1.0f, -1.0f, -1.0f, -1.0f);
		weights = vec4(-1.0f, -1.0f, -1.0f, -1.0f);
	}

	// return false is vertex has no more space for a link (limit is 4 clusters per vertex)
	bool AddLink(const int clusterId, const double weight)
	{
		for (int i=0; i<4; ++i)
		{
			if (links[i] < 0.0f)
			{
				links[i] = (float) clusterId;
				weights[i] = (float) weight;
				return true;
			}
		}
		return false;
	}

};

class CGPUClusterCache
{
public:

	//! a constructor
	CGPUClusterCache();

	bool	PrepFromModel( FBModel *pModel );
	void	UpdateTransforms();

	void	AssignBuffersId(const GLuint bufferVertices, const GLuint bufferTransforms);

	bool	UploadVertexData();
	bool	UploadTransforms();

	bool	BindBuffers( const GLuint vertexSlot=4, const GLuint transformsSlot=5 );

	const int GetNumberOfClusters() const
	{
		return (int) mCacheTransforms.size();
	}

protected:

	FBClusterMode		mMode;
	double				mAccuracy;

	GLuint				mBufferVertices;
	GLuint				mBufferTransforms;

	std::vector<ClusterData>	mClusterData;

	//
	// data to fill and upload on GPU

	// store links per vertex
	std::vector<ClusterVertex>		mCacheVertices;

	// store tm per joint
	std::vector<mat4>				mCacheTransforms;

	void AllocateData(const int numberOfVertices, const int numberOfLinks);

	
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectCompositeBase

class ObjectBlendSolver : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(ObjectBlendSolver, FBUserObject)
	FBDeclareUserObject(ObjectBlendSolver);

public:
	//! a constructor
	ObjectBlendSolver(const char *pName = NULL, HIObject pObject=NULL);

	virtual bool FBCreate() override;
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

	void OnPerFrameRenderingPipelineCallback    (HISender pSender, HKEvent pEvent);
	void OnSystemIdle (HISender pSender, HKEvent pEvent);

	static void SetReLoadShader(HIObject object, bool value);

public:

	FBPropertyBool							Active;
	//FBPropertyAnimatableDouble			Weight;		// percentage of an effect (like opacity factor)
	FBPropertyAction						ReLoadShader;

	FBPropertyBool							Test;

	// model which normal buffer we will drive
	FBPropertyListObject					MasterModel;

	// models where we grab normal buffers
	FBPropertyListObject					BlendshapeModels;

	// output color map and normal map
	FBPropertyListObject					OutputColorVideo;	// ?! video or texture ?!
	FBPropertyListObject					OutputNormalVideo;

public:

	
	//
	// UPDATE RESOURCE FOR COMPONENTS
	//

	// update when ogl context has changed
	void ChangeContext();

	void ChangeGlobalSettings();
	
	

protected:
	bool	mNeedProgramReload;

	FBSystem		mSystem;

public:
	
	void DoReloadShader();

protected:

	struct BlendShapeVertex
	{
		vec4	pos;
		vec4	nor;
	};

	// per-model blendshape buffers
	struct PerModelBlendshapes
	{
		int		geometryUpdateId;
		int		shapesCount;
		int		vertexCount;

		GLuint		mBufferWeights;
		GLuint		mBufferShapes;
		
		//GLuint		mBufferOutput; // write directly into a model normal buffer

		int			mState;	// 0 - write, 1 - read
		float		*mBufferTemp;

		PerModelBlendshapes()
		{
			geometryUpdateId = -1;
			shapesCount = 0;
			vertexCount = 0;

			mBufferWeights = 0;
			mBufferShapes = 0;
			//mBufferOutput = 0;

			mState = 0;
			mBufferTemp = nullptr;
		}
		~PerModelBlendshapes()
		{
			//Free();
		}

		void Bind();
		void Free()
		{
			geometryUpdateId = -1;
			shapesCount = 0;

			/*
			if (mBufferWeights > 0)
			{
				glDeleteBuffers( 1, &mBufferWeights );
				mBufferWeights = 0;
			}
			if (mBufferBlendshapes > 0)
			{
				glDeleteBuffers( 1, &mBufferBlendshapes );
				mBufferBlendshapes = 0;
			}
			if (mBufferClusterStatic > 0)
			{
				glDeleteBuffers( 1, &mBufferClusterStatic );
				mBufferClusterStatic = 0;
			}
			if (mBufferClusterDynamic > 0)
			{
				glDeleteBuffers( 1, &mBufferClusterDynamic );
				mBufferClusterDynamic = 0;
			}
			*/
			/*
			if (mBufferOutput > 0)
			{
				glDeleteBuffers( 1, &mBufferOutput );
				mBufferOutput = 0;
			}
			*/
		}
	};

	GLuint					mBuffersId[100];
	
	GLuint					mDuplicateCount;
	GLuint					mBufferDuplicates;

	PerModelBlendshapes		mPerModelBlendshapes;
	CGPUClusterCache		mClusterCache;
	
	CComputeProgram			mComputeProgram;
	
	CComputeProgram			mProgramZero;
	CComputeProgram			mProgramRecomputeNormals;
	CComputeProgram			mProgramNorm;

	void		GenerateGLBuffers();
	void		FreeGLBuffers();

	// prepare buffers data and bind buffers if needed
	const PerModelBlendshapes			PrepareModelBlendshapes(FBModel *pModel, bool fillbuffers, bool bind);
	
	// per frame - upload blendshape property weights
	bool								PrepBlendshapeWeights(FBModel *pModel, FBGeometry *pGeometry, const int numberOfShapes, const PerModelBlendshapes &data);
	
	// glsl compute shader to compute new normal buffer
	bool								RunComputeProgram(FBModel *pModel);

	bool				RunReComputeNormals(FBModel *pModel);
};
