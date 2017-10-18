
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_models_newton.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "shared_models_newton.h"
#include "algorithm\nv_math.h"
// TODO: store face matid and attributes to determine specified mesh under the ray

////////////////////////////////////////////////////////////////////////////////////
// QueryGPUCacheGeometry

//! a constructor
GPUCacheGeometry::GPUCacheGeometry()
	: mNumberOfVertices(0)
	, mNumberOfPolys(0)
{
}

GPUCacheGeometry::GPUCacheGeometry( CGPUModelRenderCached *pModel )
	: mNumberOfVertices(0)
	, mNumberOfPolys(0)
{
	Prep(pModel);
}

//! a destructor
GPUCacheGeometry::~GPUCacheGeometry()
{
	Free();
}

//
bool GPUCacheGeometry::Prep( CGPUModelRenderCached *pModel )
{
	
	CGPUVertexData *pVertexData = pModel->GetVertexDataPtr();

	const int totalNumberOfVerts = pVertexData->GetNumberOfVertices();
	const int totalNumberOfPolys = pVertexData->GetNumberOfIndices() / 3;

	if (totalNumberOfPolys == 0 || totalNumberOfVerts == 0)
		return false;

	//
	// DONE: get source vertex data from gpu buffers

	const vec4 *sourcePositions = (const vec4*) pVertexData->MapPositionBuffer();
	const vec4 *sourceNormals = (const vec4*) pVertexData->MapNormalBuffer();
	const GLuint *sourceIndices = (const GLuint*) pVertexData->MapIndexBuffer();

	if (sourcePositions == nullptr || sourceNormals == nullptr || sourceIndices == nullptr)
	{
		// !! very important to unmap gpu buffers
		if (sourcePositions != nullptr)
			pVertexData->UnMapPositionBuffer();
		if (sourceNormals != nullptr)
			pVertexData->UnMapNormalBuffer();
		if (sourceIndices != nullptr)
			pVertexData->UnMapIndexBuffer();

		return false;
	}

	//
	//

	Allocate(totalNumberOfVerts, totalNumberOfPolys);

	std::vector<char> vertexFlags;
	vertexFlags.resize(totalNumberOfVerts, 0);

	//
	// apply transformation for each mesh and store submesh index attribute (from hit we could determine the submesh index and name)

	const int numberOfMeshes = pModel->GetNumberOfSubMeshes();
	auto dstPoly = mPolys.begin();
	int numberOfPolys = 0;

	for (int i=0; i<numberOfMeshes; ++i)
	{

		const int modelIndex = pModel->SubMeshIndexToModelIndex(i);
		const mat4 &tm = pModel->GetModelTransform(modelIndex);

		const int firstIndex = pModel->GetMeshFirstIndex(i);
		const int indexCount = pModel->GetMeshIndexCount(i);

		for (int j=0; j<indexCount; ++j)
		{
			unsigned int index = sourceIndices[firstIndex + j];
			
			if (vertexFlags[index] == 0)
			{
				vec4 pos(sourcePositions[index]);
				vec4 nor(sourceNormals[index]);

				vec4 srcpos, dstpos;
				mat4	mmatrix;

				//FBTVector srcpos, dstpos;
				//FBMatrix mmatrix;

				srcpos = pos;
				//for (int i=0; i<4; ++i)
				//	srcpos[i] = pos.vec_array[i];

				pos = tm * pos;	// ??!! why is not equal to mobu FBVectorMatrixMult ??!!

				mmatrix = tm;
				//for (int i=0; i<16; ++i)
				//	mmatrix[i] = tm.mat_array[i];

				//FBVectorMatrixMult(dstpos, mmatrix, srcpos);
				dstpos = mmatrix * srcpos;

				pos = dstpos;
				//for (int i=0; i<3; ++i)
				//	pos[i] = (float) dstpos[i];

				mVertices[index] = pos;
				mNormals[index] = nor;

				vertexFlags[index] = 1;
			}
		}

		for (int j=0; j<indexCount; j+=3)
		{
			dstPoly->count = 3;

			dstPoly->indices[0] = sourceIndices[firstIndex + j];
			dstPoly->indices[1] = sourceIndices[firstIndex + j + 1];
			dstPoly->indices[2] = sourceIndices[firstIndex + j + 2];

			dstPoly++;
			numberOfPolys += 1;
		}
	}

	if (numberOfPolys != totalNumberOfPolys)
		printf( "different number of polys!\n" );

	//
	/*
	int srcIndex = 0;
	for (int i=0; i<totalNumberOfPolys; ++i, ++dstPoly)
	{
		dstPoly->count = 3;

		for (int j=0; j<3; ++j)
		{
			dstPoly->indices[j] = sourceIndices[srcIndex];
			srcIndex += 1;
		}
	}
*/
	// !! very important to unmap gpu buffers
	pVertexData->UnMapPositionBuffer();
	pVertexData->UnMapNormalBuffer();
	pVertexData->UnMapIndexBuffer();

	return true;
}

const int GPUCacheGeometry::GetVertexCount() const 
{
	return (int) mVertices.size();
}

const float *GPUCacheGeometry::GetVertexPosition(const int index) const 
{
	return mVertices[index].vec_array;
}

const float *GPUCacheGeometry::GetVertexNormal(const int index) const 
{
	return mNormals[index].vec_array;
}

//

const int GPUCacheGeometry::GetPolyCount() const 
{
	return (int) mPolys.size();
}

const GPUCacheGeometry::Poly *GPUCacheGeometry::GetPoly(const int index) const 
{
	return &mPolys[index];
}

const int GPUCacheGeometry::GetPolyAttribute(const int index) const
{
	return mAttributes[index];
}

bool GPUCacheGeometry::HasVertexNormal() const
{
	return (mNormals.size() > 0);
}

bool GPUCacheGeometry::HasAttributes() const
{
	return (mAttributes.size() > 0);
}

void GPUCacheGeometry::Allocate(int numberOfVerts, int numberOfPolys)
{
	mVertices.resize(numberOfVerts);
	mNormals.resize(numberOfVerts);
	mPolys.resize(numberOfPolys);
	mAttributes.resize(numberOfPolys);

	mNumberOfVertices = numberOfVerts;
	mNumberOfPolys = numberOfPolys;
}

void GPUCacheGeometry::Free()
{
	mNumberOfVertices = 0;
	mNumberOfPolys = 0;

	mVertices.clear();
	mNormals.clear();
	mPolys.clear();
	mAttributes.clear();
}
