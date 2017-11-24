
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_object_blendSolver.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "MB_object_blendSolver.h"
#include "algorithm\nv_math.h"
#include "graphics\checkglerror_MOBU.h"
#include <Windows.h>

//
#define DEFORMATIONS_COMPUTE_SHADER			"\\GLSL_CS\\computeDeformation.cs"

#define SHADER_NORMALS_ZERO			"\\GLSL_CS\\recomputeNormalsZero.cs"
#define SHADER_NORMALS_COMPUTE		"\\GLSL_CS\\recomputeNormals.cs"
#define SHADER_NORMALS_NORM			"\\GLSL_CS\\recomputeNormalsNorm.cs"

/** Class implementation.
*   This should be placed in the source code file for a class that derives
*   from FBComponent.
*   It is absolutely necessary in order to identify the class type.
*/

FBClassImplementation(ObjectBlendSolver)
FBUserObjectImplement(ObjectBlendSolver, "Blend Solver", FB_DEFAULT_SDK_ICON);			//Register UserObject class
FBElementClassImplementation(ObjectBlendSolver, FB_DEFAULT_SDK_ICON);			//Register to the asset system

extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);


/////////////////////////////////////////////////////////////////////////////////////////
// ObjectBlendSolver

void ObjectBlendSolver::SetReLoadShader(HIObject object, bool value)
{
	ObjectBlendSolver *pBase = FBCast<ObjectBlendSolver>(object);
	if (pBase && value) 
	{
		pBase->DoReloadShader();
	}
}

ObjectBlendSolver::ObjectBlendSolver(const char *pName, HIObject pObject)
		: FBUserObject( pName, pObject )
{
	FBClassInit;
	
	mNeedProgramReload = false;

	for (int i=0; i<100; ++i)
		mBuffersId[i] = 0;
	mBufferDuplicates = 0;
	mDuplicateCount = 0;
}

bool ObjectBlendSolver::FBCreate()
{
	FBPropertyPublish(this, Active, "Active", nullptr, nullptr);

#ifdef _DEBUG
	FBPropertyPublish(this, ReLoadShader, "Reload shader", nullptr, SetReLoadShader);
	FBPropertyPublish(this, Test, "Test", nullptr, nullptr);
#endif

	FBPropertyPublish(this, MasterModel, "Master Model", nullptr, nullptr);
	FBPropertyPublish(this, BlendshapeModels, "Shapes Models", nullptr, nullptr);

	FBPropertyPublish(this, OutputColorVideo, "Output Color", nullptr, nullptr);
	FBPropertyPublish(this, OutputNormalVideo, "Output Normal", nullptr, nullptr);

	Active = true;
	Test = false;

	MasterModel.SetSingleConnect(true);
	MasterModel.SetFilter(FBModel::GetInternalClassId() );
	BlendshapeModels.SetSingleConnect(false);
	BlendshapeModels.SetFilter(FBModel::GetInternalClassId() );

	OutputColorVideo.SetSingleConnect(true);
	OutputColorVideo.SetFilter(FBVideo::GetInternalClassId() );
	OutputNormalVideo.SetSingleConnect(true);
	OutputNormalVideo.SetFilter(FBVideo::GetInternalClassId() );

	mSystem.OnUIIdle.Add( this, (FBCallback) &ObjectBlendSolver::OnSystemIdle );

	return ParentClass::FBCreate();
}

void ObjectBlendSolver::FBDestroy()
{
	FreeGLBuffers();
	
	//

	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Remove(this, (FBCallback)&ObjectBlendSolver::OnPerFrameRenderingPipelineCallback);
	
	ParentClass::FBDestroy();
}


bool ObjectBlendSolver::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ObjectBlendSolver::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void ObjectBlendSolver::OnSystemIdle (HISender pSender, HKEvent pEvent)
{
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Add(this, (FBCallback)&ObjectBlendSolver::OnPerFrameRenderingPipelineCallback);

	mSystem.OnUIIdle.Remove( this, (FBCallback) &ObjectBlendSolver::OnSystemIdle );
}

void ObjectBlendSolver::OnPerFrameRenderingPipelineCallback    (HISender pSender, HKEvent pEvent)
{
	if (false == Active || MasterModel.GetCount() == 0)
		return;

	static bool firstTime = true;

	if (true == firstTime)
	{
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
		firstTime = false;
	}

	//HGLRC context = wglGetCurrentContext();

	FBModel *pModel = (FBModel*) MasterModel.GetAt(0);

	FBEventEvalGlobalCallback lFBEvent(pEvent);

	switch(lFBEvent.GetTiming() )
	{
	case kFBGlobalEvalCallbackBeforeRender:
		// store per-model user data with blendshape buffers
		PrepareModelBlendshapes(pModel, false, true);

		// run a compute program to calculate normals (if model has any blendshapes)
		if (mPerModelBlendshapes.mBufferShapes > 0)
		{
			//RunComputeProgram(pModel);
			RunReComputeNormals(pModel);
		}
		break;
	case kFBGlobalEvalCallbackAfterRender:
		// store per-model user data with blendshape buffers
		PrepareModelBlendshapes(pModel, true, false);
		break;
	}
}

// update when ogl context has changed
void ObjectBlendSolver::ChangeContext()
{
	FreeGLBuffers();
}

void ObjectBlendSolver::ChangeGlobalSettings()
{
}

void ObjectBlendSolver::DoReloadShader()
{
	mNeedProgramReload = true;
}

void ObjectBlendSolver::GenerateGLBuffers()
{
	FreeGLBuffers();
	glGenBuffers(100, mBuffersId);
}

void ObjectBlendSolver::FreeGLBuffers()
{
	if (mBuffersId[0] > 0)
		glDeleteBuffers(100, &mBuffersId[0] );
}

const ObjectBlendSolver::PerModelBlendshapes ObjectBlendSolver::PrepareModelBlendshapes(FBModel *pModel, bool fillbuffers, bool bind)
{
	PerModelBlendshapes	&data = mPerModelBlendshapes;

	FBGeometry *pGeometry = pModel->Geometry;
	FBModelVertexData *pData = pModel->ModelVertexData;

	if (nullptr == pGeometry || nullptr == pData 
		|| false == pData->IsDrawable() || false == Test)
		return data;

	const int vertexCount = pData->GetVertexCount();
	const int geomUpdateId = pModel->GeometryUpdateId;

	if ( (true == fillbuffers) 
		&& (data.geometryUpdateId != geomUpdateId || data.mBufferShapes == 0) )
	{

		GenerateGLBuffers();
	
		/*
		if (data.mBufferBlendshapes > 0)
			glDeleteBuffers(1, &data.mBufferBlendshapes);
		glGenBuffers(1, &data.mBufferBlendshapes);
		if (data.mBufferWeights > 0)
			glDeleteBuffers(1, &data.mBufferWeights);
		glGenBuffers(1, &data.mBufferWeights);
		*/

		data.mBufferWeights = mBuffersId[98];
		data.mBufferShapes = mBuffersId[99];
		
		//
		// CLUSTER INFO
		mClusterCache.PrepFromModel(pModel);
		mClusterCache.UpdateTransforms();

		mClusterCache.AssignBuffersId( mBuffersId[96], mBuffersId[97] );

		mClusterCache.UploadVertexData();
		mClusterCache.UploadTransforms();
		
		mBufferDuplicates = mBuffersId[95];

		//
		// BLENDSHAPES INFO


		// TODO: use dublicate list to copy diff normals information into blendshape data
		FBVertex *pBasePositions = (FBVertex*) pData->GetVertexArray(kFBGeometryArrayID_Point, false);
		FBNormal *pBaseNormals = (FBNormal*) pData->GetVertexArray(kFBGeometryArrayID_Normal, false);
		//const GLuint baseId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal, false );
		//const GLuint deformId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal, true );

		// only in that case we should continue with buffers creation
		const int shapeCount = pGeometry->ShapeGetCount();
		
		// normals for all shapes
		BlendShapeVertex emptyVertex = {vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f) };
		std::vector<BlendShapeVertex> shapes( vertexCount * (shapeCount+1), emptyVertex );
		
		std::vector<vec4> weights( (shapeCount+1), vec4(1.0, 0.0, 0.0, 0.0) );

		//
		// fill with a base normals

		// FBNormal is a 4 float vector and vec4 is a 4 float vector
		for (int i=0; i<vertexCount; ++i)
		{
			memcpy( shapes[i].pos.vec_array, pBasePositions[i], sizeof(float) * 4 );
			memcpy( shapes[i].nor.vec_array, pBaseNormals[i], sizeof(float) * 4 );
		}
		
		// fill with shapes
		if (shapeCount > 0)
		{
			
			for (int j=0; j<shapeCount; ++j)
			{

				const char *shapeName = pGeometry->ShapeGetName(j);
				if (shapeName != nullptr)
				{
					FBProperty *pProp = pModel->PropertyList.Find(shapeName);
					if (nullptr != pProp)
					{
						double value=0.0;
						pProp->GetData( &value, sizeof(double) );
						weights[j].x = 0.01f * (float)value;
					}
				}

				FBModel *shapeModel = (j < BlendshapeModels.GetCount() ) ? (FBModel*)BlendshapeModels.GetAt(j) : pModel;
				FBModelVertexData *shapeVertexData = shapeModel->ModelVertexData;

				if (nullptr == shapeVertexData || shapeVertexData->GetVertexCount() != vertexCount)
					shapeVertexData = pData;
				
				//shapeVertexData->VertexArrayMappingRequest();

				// TODO: grab positions from target shapes !

				FBNormal *pShapePositions = (FBVertex*) shapeVertexData->GetVertexArray(kFBGeometryArrayID_Point, false);
				FBNormal *pShapeNormals = (FBNormal*) shapeVertexData->GetVertexArray(kFBGeometryArrayID_Normal, false);

				//
				const int indexOffset = vertexCount*(j+1);

				for (int nVertex=0; nVertex<vertexCount; ++nVertex)
				{
					memcpy( shapes[nVertex + indexOffset].pos.vec_array, pShapePositions[nVertex], sizeof(float) * 4 );
					memcpy( shapes[nVertex + indexOffset].nor.vec_array, pShapeNormals[nVertex], sizeof(float) * 4 );
				}
				//memcpy( &shapeNormals[vertexCount*(j+1)], pShapeNormals, sizeof(float)*4*vertexCount);

				//shapeVertexData->VertexArrayMappingRelease();
			}
		}
			
		//
		// buffer for blendshapes (Normals0
			
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.mBufferShapes);

		size_t size = sizeof(float) * 4 * 2 * vertexCount * (shapeCount+1);

		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STREAM_READ);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, shapes.data(), GL_STREAM_READ);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		//data.mBufferBlendshapes = bufferId;
			
		//
		// buffer for duplicates

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferDuplicates);

		mDuplicateCount = 0;
		const int *duplicates = pData->GetVertexArrayDuplicationMap(mDuplicateCount);

		size = sizeof(int) * mDuplicateCount;

		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STREAM_READ);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, duplicates, GL_STREAM_READ);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		//
		// buffer for weights
			
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.mBufferWeights);
		size = sizeof(float) * 4 * (shapeCount+1);
		//glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STREAM_DRAW);
		//glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
	
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, weights.data(), GL_DYNAMIC_COPY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
		//data.mBufferWeights = bufferId;
	
		//PrepBlendshapeWeights(pModel, pGeometry, shapeCount, data);

		//
		//
		/*
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.mBufferOutput);
		size = sizeof(vec4) * vertexCount;
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STREAM_DRAW);
		std::vector<vec4> output(vertexCount, vec4(0.0, 0.0, 0.0, 0.0) );
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, output.data(), GL_STREAM_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		*/

		// final, store a count
		mPerModelBlendshapes.shapesCount = shapeCount;
		mPerModelBlendshapes.vertexCount = vertexCount;
		mPerModelBlendshapes.geometryUpdateId = geomUpdateId;

		/*
		// test

		glBindBuffer( GL_SHADER_STORAGE_BUFFER, mPerModelBlendshapes.mBufferBlendshapes );
		float * p = (float*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (p)
		{
			std::vector<vec4> output(vertexCount, vec4(0.0, 0.0, 0.0, 0.0) );
			float *pOutputData = output[0].vec_array;
			memcpy( output.data(), p, sizeof(float)*4*vertexCount );
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		
		CHECK_GL_ERROR_MOBU();
		*/
	}

	//
	//
	if (true == bind && data.mBufferShapes > 0)
	{
		// cluster info

		mClusterCache.UpdateTransforms();
		mClusterCache.UploadTransforms();	
	
		// update and upload blenshapes weights

		PrepBlendshapeWeights( pModel, pGeometry, data.shapesCount, data );

		// bind blendshapes buffers

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mPerModelBlendshapes.mBufferWeights);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mPerModelBlendshapes.mBufferShapes);

		//
		// Output 
		
		const GLuint deformId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal, true );
		const GLuint deformPosId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Point, true );
		
		const GLvoid* positionOffset = pData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Point);
		const GLvoid* normalOffset = pData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Normal);

		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, deformId );
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, deformPosId );

		// correct bind for a GPU skinning mode
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, deformPosId, (GLintptr) positionOffset, vertexCount * sizeof(vec4));
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, deformId, (GLintptr) normalOffset, vertexCount * sizeof(vec4));

		// bind cluster buffers
		mClusterCache.BindBuffers();
	}

	CHECK_GL_ERROR_MOBU();

	return data;
}

bool ObjectBlendSolver::PrepBlendshapeWeights(FBModel *pModel, FBGeometry *pGeometry, const int numberOfShapes, const PerModelBlendshapes &data)
{

	// prepare blendshape weights
	if (0 == data.mBufferWeights)
		return false;
	/*
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.mBufferWeights);
	size_t size = sizeof(float) * 4 * numberOfShapes;
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STREAM_DRAW);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
	std::vector<vec4> weights(numberOfShapes, vec4(1.0, 0.0, 0.0, 0.0) );
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, weights.data(), GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return true;
	*/
	{
	std::vector<vec4>	weights( numberOfShapes+1, vec4(0.0, 0.0, 0.0, 0.0) );
	
	for (int i=0; i<numberOfShapes; ++i)
	{
		const char *shapeName = pGeometry->ShapeGetName(i);
		FBProperty *pProp = pModel->PropertyList.Find(shapeName);

		if (nullptr != pProp)
		{
			double value;
			pProp->GetData( &value, sizeof(double) );
			weights[i] = vec4( 0.01f * (float)value, 0.0, 0.0, 0.0 );
		}
	}
	
	// send to gpu
	
	if (numberOfShapes > 0 && data.mBufferWeights > 0)
	{
		const size_t size = (numberOfShapes+1) * sizeof(float) * 4;
		/*
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.mBufferWeights);
		GLvoid* p = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		memcpy(p, weights.data(), size);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		*/
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.mBufferWeights);

		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, weights.data(), GL_DYNAMIC_COPY);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		

		CHECK_GL_ERROR_MOBU();
	}
	}
	return true;
}

bool ObjectBlendSolver::RunComputeProgram(FBModel *pModel)
{
	if (true == mNeedProgramReload)
	{
		mComputeProgram.Clear();
		mNeedProgramReload = false;
	}

	// check for a compute shader
	if (false == mComputeProgram.PrepProgram(DEFORMATIONS_COMPUTE_SHADER) )
		return false;

	//
	// prepare number of blendshapes

	FBGeometry *pGeometry = pModel->Geometry;
	FBModelVertexData *pData = pModel->ModelVertexData;

	if ( nullptr == pGeometry || nullptr == pData || false == pData->IsDrawable() )
		return false;

	const int numberOfVertices = pData->GetVertexCount();
	
	if (0 == numberOfVertices)
		return false;

	const int numberOfBlendshapes = pGeometry->ShapeGetCount();
	const int numberOfClusters = mClusterCache.GetNumberOfClusters();

	//
	// run a compute program

	const GLuint programId = mComputeProgram.GetProgramId();
	if (programId == 0)
		return false;
	/*
	if (mPerModelBlendshapes.mBufferBlendshapes > 0)
	{
		//UploadBuffersData(pModel);
	
		glBindBuffer( GL_SHADER_STORAGE_BUFFER, mPerModelBlendshapes.mBufferBlendshapes );
		float * p = (float*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (p)
		{
			std::vector<vec4> output(numberOfVertices, vec4(0.0, 0.0, 0.0, 0.0) );
			float *pOutputData = output[0].vec_array;
			memcpy( output.data(), p, sizeof(float)*4*numberOfVertices );
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		
		// bind blendshape weights
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mPerModelBlendshapes.mBufferWeights);

		// bind blendshapes diff buffer
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mPerModelBlendshapes.mBufferBlendshapes);

		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mPerModelBlendshapes.mBufferOutput);

		const GLuint deformId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal, true );
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, deformId );

		//pData->DisableOGLVertexData();
	}
	*/
	CHECK_GL_ERROR_MOBU();

	mComputeProgram.Bind();

	GLint loc = glGetUniformLocation( programId, "numberOfBlendshapes" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfBlendshapes );
	loc = glGetUniformLocation( programId, "numberOfVertices" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfVertices );

	loc = glGetUniformLocation( programId, "numberOfClusters" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfClusters );

	const int computeLocalX = 1024;
	const int x = numberOfVertices / computeLocalX + 1;

	mComputeProgram.DispatchPipeline( x, 1, 1 );

	mComputeProgram.UnBind();

	CHECK_GL_ERROR_MOBU();

	// finally we have a new normal buffer that we should bind for a draw call
	/*
	glMemoryBarrier( GL_ALL_BARRIER_BITS );

	const GLuint deformId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal, true );
	glBindBuffer( GL_COPY_READ_BUFFER, deformId );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, mPerModelBlendshapes.mBufferOutput );
	
	glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_COPY_READ_BUFFER, 0, 0, sizeof(float) * 4 * numberOfVertices);

	glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );
	glBindBuffer( GL_COPY_READ_BUFFER, 0 );

	CHECK_GL_ERROR_MOBU();
	*/
	return true;
}


bool ObjectBlendSolver::RunReComputeNormals(FBModel *pModel)
{
	if (true == mNeedProgramReload)
	{
		mProgramZero.Clear();
		mProgramNorm.Clear();
		mProgramRecomputeNormals.Clear();
		mNeedProgramReload = false;
	}

	// check for a compute shader
	if (false == mProgramZero.PrepProgram(SHADER_NORMALS_ZERO) )
		return false;

	if (false == mProgramRecomputeNormals.PrepProgram(SHADER_NORMALS_COMPUTE) )
		return false;
	
	if (false == mProgramNorm.PrepProgram(SHADER_NORMALS_NORM) )
		return false;

	//
	// prepare number of blendshapes

	FBGeometry *pGeometry = pModel->Geometry;
	FBModelVertexData *pData = pModel->ModelVertexData;

	if ( nullptr == pGeometry || nullptr == pData || false == pData->IsDrawable() )
		return false;

	const int numberOfVertices = pData->GetVertexCount();
	
	if (0 == numberOfVertices)
		return false;


	int numberOfIndices = 0;
	
	for (int i=0, count=pData->GetSubPatchCount(); i<count; ++i)
	{
		int offset = pData->GetSubPatchIndexOffset(i);
		int size = pData->GetSubPatchIndexSize(i);

		int localCount = offset+size;
		if ( localCount > numberOfIndices )
			numberOfIndices = localCount;
	}

	const int numberOfTriangles = numberOfIndices / 3;

	//
	// run a compute program

	const GLuint posId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Point, true );
	const GLuint deformId = pData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal, true );
	const GLuint indId = pData->GetIndexArrayVBOId();

	const GLvoid* positionOffset = pData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Point);
	const GLvoid* normalOffset = pData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Normal);

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, deformId );
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, deformPosId );

	// correct bind for a GPU skinning mode
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, posId, (GLintptr) positionOffset, numberOfVertices * sizeof(vec4));
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, deformId, (GLintptr) normalOffset, numberOfVertices * sizeof(vec4));

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indId );

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mBufferDuplicates );

	CHECK_GL_ERROR_MOBU();

	//
	// ZERO NORMALS
	{
	const GLuint programId = mProgramZero.GetProgramId();
	if (programId == 0)
		return false;

	mProgramZero.Bind();

	GLint loc = glGetUniformLocation( programId, "numberOfNormals" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfVertices );

	const int computeLocalX = 1024;
	const int x = numberOfVertices / computeLocalX + 1;

	mProgramZero.DispatchPipeline( x, 1, 1 );

	mProgramZero.UnBind();

	CHECK_GL_ERROR_MOBU();
	}
	
	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT ); //GL_ALL_BARRIER_BITS

	//
	// ACCUM NORMALS
	{
	const GLuint programId = mProgramRecomputeNormals.GetProgramId();
	if (programId == 0)
		return false;

	mProgramRecomputeNormals.Bind();

	GLint loc = glGetUniformLocation( programId, "numberOfTriangles" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfTriangles );
	
	const int computeLocalX = 1024;
	const int x = numberOfTriangles / computeLocalX + 1;

	mProgramRecomputeNormals.DispatchPipeline( x, 1, 1 );

	mProgramRecomputeNormals.UnBind();

	CHECK_GL_ERROR_MOBU();
	}

	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

	//
	// ZERO NORMALS


	{
	const GLuint programId = mProgramNorm.GetProgramId();
	if (programId == 0)
		return false;

	mProgramNorm.Bind();

	GLint loc = glGetUniformLocation( programId, "numberOfNormals" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfVertices );

	loc = glGetUniformLocation( programId, "duplicateStart" );
	if (loc >= 0)
		glProgramUniform1i( programId, loc, numberOfVertices - mDuplicateCount );
	
	const int computeLocalX = 1024;
	const int x = numberOfVertices / computeLocalX + 1;

	mProgramNorm.DispatchPipeline( x, 1, 1 );

	mProgramNorm.UnBind();
	}
	CHECK_GL_ERROR_MOBU();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CGPUClusterCache

CGPUClusterCache::CGPUClusterCache()
{
	mBufferVertices = 0;
	mBufferTransforms = 0;

	mMode = kFBClusterAdditive;
	mAccuracy = 100.0;
}

void CGPUClusterCache::AssignBuffersId( const GLuint vertId, const GLuint tmId )
{
	mBufferVertices = vertId;
	mBufferTransforms = tmId;
}

void CGPUClusterCache::AllocateData(const int numberOfVertices, const int numberOfLinks)
{
	ClusterVertex	emptyVertex;
	emptyVertex.Clear();

	mCacheVertices.resize(numberOfVertices, emptyVertex);
	mCacheTransforms.resize(numberOfLinks);
	mClusterData.resize(numberOfLinks);
}

bool CGPUClusterCache::PrepFromModel(FBModel *pModel)
{
	if (pModel == nullptr) 
		return false;

	FBGeometry *pGeometry = pModel->Geometry;
	FBCluster *pCluster = pModel->Cluster;
	FBModelVertexData *pVertexData = pModel->ModelVertexData;

	if ( nullptr == pGeometry || nullptr == pCluster 
		|| false == pModel->SkeletonDeformable ) 
	{
		return false;
	}

	
	const int vertCount = pVertexData->GetVertexCount();
	const int linkCount = pCluster->LinkGetCount();

	if (0 == vertCount || 0 == linkCount)
		return false;

	AllocateData(vertCount, linkCount);
	
	for (int clusterId=0; clusterId < linkCount; ++clusterId) 
	{
		ClusterData		&outData = mClusterData[clusterId];

		pCluster->ClusterBegin(clusterId);			// Set the current cluster index

		
		FBModel *linkModel = pCluster->LinkGetModel(clusterId);
		FBModel *linkAssociateModel = pCluster->LinkGetAssociateModel(clusterId);

		if (nullptr == linkModel)
		{
			outData.pModel = nullptr;
			outData.pModel2 = nullptr;
			continue;
		}

		outData.pModel = linkModel;
		outData.pModel2 = linkAssociateModel;
		outData.mode = pCluster->ClusterMode;
		outData.accuracy = pCluster->ClusterAccuracy;

		/*
		if (mode != kFBClusterAdditive)
		{
			printf ("only additive skinning is supported!\n" );
		}
		*/
		FBMatrix m, m2, tm, tmInvTranspose, pLM;
		FBTVector pos;
		FBRVector rot, pDof;
		FBSVector scale;
		FBVector3d temp;


		pCluster->VertexGetTransform(temp, rot, scale);
		pos = FBTVector(temp[0], temp[1], temp[2], 1.0);
		//scale = FBSVector(1.0, 1.0, 1.0);
		FBTRSToMatrix( tm, pos, rot, scale );
		//FBMatrixInverse( tm, tm );

		outData.bindTM = tm;

		//
		const int numLinkVerts = pCluster->VertexGetCount();	// Using the current cluster index
		for (int v=0; v < numLinkVerts; v++) 
		{
			const int vertIndex = pCluster->VertexGetNumber(v);		// Using the current cluster index
			const double vertWeight = pCluster->VertexGetWeight(v);	// Using the current cluster index

			mCacheVertices[vertIndex].AddLink(clusterId, vertWeight);
		}
		pCluster->ClusterEnd();		

		outData.pCacheOutput = &mCacheTransforms[clusterId];
	}

	return true;
}

void CGPUClusterCache::UpdateTransforms()
{
	for (auto iter=begin(mClusterData); iter!=end(mClusterData); ++iter)
	{
		if (nullptr == iter->pModel)
		{
			continue;
		}

		FBMatrix tm = iter->bindTM;
		FBMatrix m, m2;
		
		iter->pModel->GetMatrix( m, kModelTransformation, false );
			
		if (nullptr != iter->pModel2)
		{
			iter->pModel2->GetMatrix( m2, kModelTransformation_Geometry );
			FBMatrixMult( m, m2, m );
		}

		FBMatrixMult( tm, m, tm );
		
		if (nullptr != iter->pModel->Parent)
		{
			iter->pModel->Parent->GetMatrix( m2 );
			FBMatrixMult( tm, m2, tm );
		}

		//FBMatrixInverse( tmInvTranspose, tm );
		//FBMatrixTranspose( tmInvTranspose, tmInvTranspose );

		for (int i=0; i<16; ++i)
			iter->pCacheOutput->mat_array[i] = (float) tm[i];
	}
}

bool CGPUClusterCache::UploadVertexData()
{
	if (0 == mBufferVertices || 0 == mCacheVertices.size() )
		return false;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferVertices);

	size_t size = sizeof(ClusterVertex) * mCacheVertices.size();

	glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STREAM_READ);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, mCacheVertices.data(), GL_STREAM_READ);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return true;
}

bool CGPUClusterCache::UploadTransforms()
{
	if (0 == mBufferTransforms || 0 == mCacheTransforms.size() )
		return false;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferTransforms);
	size_t size = sizeof(mat4) * mCacheTransforms.size();
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STREAM_DRAW);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
	
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, mCacheTransforms.data(), GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return true;
}

bool CGPUClusterCache::BindBuffers( const GLuint vertexSlot, const GLuint transformsSlot )
{
	if ( mCacheTransforms.size() > 0 )
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, vertexSlot, mBufferVertices);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, transformsSlot, mBufferTransforms);

		return true;
	}

	return false;
}