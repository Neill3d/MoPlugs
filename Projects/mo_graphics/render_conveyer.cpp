
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: render_conveyer.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "render_conveyer.h"
#include "ContentInspector.h"

//! a constructor
CRenderVertexConveyer::CRenderVertexConveyer()
{

	for (int i=0; i<BUFFER_COUNT; ++i)
	{
		mBufferIds[i] = 0;
		mBufferElementCount[i] = 0;
		mBufferElementSize[i] = 0;
	}
		
	// fill the element sizes
	mBufferElementSize[BUFFER_POSITION] = sizeof(float) * 4;
	mBufferElementSize[BUFFER_NORMAL] = sizeof(float) * 4;
	mBufferElementSize[BUFFER_UV] = sizeof(float) * 2;
	mBufferElementSize[BUFFER_INDEX] = sizeof(int);

	mLastVertexCount = 0;
	mLastIndexCount = 0;
}
//! a destructor
CRenderVertexConveyer::~CRenderVertexConveyer()
{}

void CRenderVertexConveyer::Clear()
{
	mModelVertexCopyOffsets.clear();
}

bool CRenderVertexConveyer::CheckModelFilter(FBModel *pModel)
{
	if (mModelFilter.skipGeometryCache && FBIS(pModel, ORModelGPUCache) )
		return false;

	if (mModelFilter.skipDeformable && (true == pModel->IsDeformable) )
		return false;

	if (mModelFilter.skipHidden && (false == pModel->IsVisible() ) )
		return false;

	return true;
}

bool CRenderVertexConveyer::CheckModelVertexData( FBModelVertexData *pVertexData )
{
	if (nullptr == pVertexData || false == pVertexData->IsDrawable() )
		return false;

	const GLint positionId = pVertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Point);
	const GLint normalId = pVertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Normal );
	const GLint uvId = pVertexData->GetUVSetVBOId();

	if (positionId == 0 || normalId == 0 || uvId == 0)
		return false;

	return true;
}

const int CRenderVertexConveyer::GetModelVertexIndexCount( FBModelVertexData *pVertexData )
{
	int indexCount = 0;
	for (int npatch=0, patchCount=pVertexData->GetSubPatchCount(); npatch < patchCount; ++npatch)
	{
		int size = pVertexData->GetSubPatchIndexSize(npatch);
		int offset = pVertexData->GetSubPatchIndexOffset(npatch);

		indexCount = std::max(indexCount, offset+size);
	}

	return indexCount;
}

void CRenderVertexConveyer::AllocateFromFBModels(std::vector<FBModel*>	&models, std::vector<int> &flags)
{
	int totalVertexCount = 0;
	int totalIndexCount = 0;

	mModelVertexCopyOffsets.clear();

	auto modelIter = begin(models);
	auto flagIter = begin(flags);

	for ( ; modelIter!=end(models); ++modelIter, ++flagIter)
	{
		if ( *flagIter & RESOURCE_DELETED
				|| *flagIter & RESOURCE_SYSTEM )
		{
			continue;
		}

		FBModel *pModel = *modelIter;
		FBModelVertexData *pVertexData = pModel->ModelVertexData;

		if (false == CheckModelFilter(pModel) || nullptr == pVertexData )
			continue;

		//
		mModelVertexCopyOffsets[pModel] = VertexOffsets(totalVertexCount, totalIndexCount);

		//
		int vertexCount = pVertexData->GetVertexCount();
		totalVertexCount += vertexCount;

		//
		int indexCount = GetModelVertexIndexCount(pVertexData);
		totalIndexCount += indexCount;
	}


	// allocate temp arrays
	auto fn_deleteBuffer = [] (GLuint &bufferId) {
		if (bufferId > 0)
		{
			glDeleteBuffers(1, &bufferId);
			bufferId = 0;
		}
	};
	auto fn_allocBuffer = [] (GLuint &bufferId, unsigned int size) {
		if (bufferId == 0)
		{
			glGenBuffers(1, &bufferId);
			glBindBuffer(GL_COPY_READ_BUFFER, bufferId);
			glBufferData(GL_COPY_READ_BUFFER, size, NULL, GL_STATIC_COPY);
		}
	};

	if (totalVertexCount != mLastVertexCount || totalIndexCount != mLastIndexCount)
	{
		for (int i=0; i<BUFFER_COUNT; ++i)
			fn_deleteBuffer( mBufferIds[i] );
	}

	mBufferElementCount[BUFFER_POSITION] = totalVertexCount;
	mBufferElementCount[BUFFER_NORMAL] = totalVertexCount;
	mBufferElementCount[BUFFER_UV] = totalVertexCount;
	mBufferElementCount[BUFFER_INDEX] = totalIndexCount;

	// copy data for each buffer
	for (int i=0; i<BUFFER_COUNT; ++i)
	{
		fn_allocBuffer( mBufferIds[i], mBufferElementSize[i] * mBufferElementCount[i] );
	}

	/*
		glBindBuffer( GL_COPY_READ_BUFFER, mBufferIds[i] );

		// TODO: compute a write offset value
		int writeOffset = 0;

		for (auto iter=begin(models); iter!=end(models); ++iter)
		{

			FBModel *pModel = *iter;
			FBModelVertexData *pVertexData = pModel->ModelVertexData;

			if (false == CheckModelFilter(pModel) || nullptr != pVertexData )
				continue;

			// !
			CopyBufferSubData(i, pVertexData, writeOffset * mBufferElementSize[i]);

			// TODO: 1!!

			if (i == BUFFER_INDEX)
			{
				mModelVertexCopyOffsets[pModel].indexOffset = writeOffset;
				writeOffset += GetModelVertexIndexCount(pVertexData);
			}
			else
			{
				mModelVertexCopyOffsets[pModel].vertexOffset = writeOffset;
				writeOffset += pVertexData->GetVertexCount();
			}

			mModelVertexCopyOffsets[pModel].updated = false;
		}
	}
	*/

	mLastVertexCount = totalVertexCount;
	mLastIndexCount = totalIndexCount;

}

void CRenderVertexConveyer::CopyBufferSubData(int bufferType, FBModelVertexData *pVertexData, const unsigned int writeOffset)
{
	GLuint srcBufferId = 0;
	int vertexCount = pVertexData->GetVertexCount();

	switch(bufferType)
	{
	case BUFFER_POSITION:
		srcBufferId = pVertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Point);
		break;
	case BUFFER_NORMAL:
		srcBufferId = pVertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Normal);
		break;
	case BUFFER_UV:
		srcBufferId = pVertexData->GetUVSetVBOId();
		break;
	case BUFFER_INDEX:
		srcBufferId = pVertexData->GetIndexArrayVBOId();
		break;
	}

	// TODO: take care about GPU skinning offset in the buffer
	// const GLvoid* positionOffset = pVertexData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Point);

	if (srcBufferId > 0)
	{
		glBindBuffer( GL_ARRAY_BUFFER, srcBufferId );
		glCopyBufferSubData(GL_ARRAY_BUFFER,GL_COPY_READ_BUFFER, 0, writeOffset, mBufferElementSize[bufferType] * vertexCount);
	}
}

// DEPRICATED:
/*
bool UpdateModelSubData(FBModel *pModel)
{

	// get offset by model points

	auto iter = mModelVertexCopyOffsets.find( pModel );

	if (iter == end(mModelVertexCopyOffsets) )
		return false;

	VertexOffsets offsets = iter->second;

	// update vertex positions subdata and perhaps normals subdata (maybe)
	//	do a gpu subData copy !

	if (mBufferIds[BUFFER_POSITION] == 0 || mBufferIds[BUFFER_NORMAL] == 0)
		return false;
	FBModelVertexData *pVertexData = pModel->ModelVertexData;

	glBindBuffer( GL_COPY_READ_BUFFER, mBufferIds[BUFFER_POSITION] );
	CopyBufferSubData( BUFFER_POSITION, pVertexData, offsets.vertexOffset * mBufferElementSize[BUFFER_POSITION] );
	CopyBufferSubData( BUFFER_NORMAL, pVertexData, offsets.vertexOffset * mBufferElementSize[BUFFER_NORMAL] );

	return true;
}
*/
bool CRenderVertexConveyer::UpdateAllModelsSubData(FBEvaluateInfo *pEvaluateInfo)
{
	if (mBufferIds[BUFFER_POSITION] == 0 || mBufferIds[BUFFER_NORMAL] == 0)
		return false;

	//
	// POSITIONS
	glBindBuffer( GL_COPY_READ_BUFFER, mBufferIds[BUFFER_POSITION] );
	unsigned int bufferElementSize = mBufferElementSize[BUFFER_POSITION];

	for (auto iter=begin(mModelVertexCopyOffsets);
		iter != end(mModelVertexCopyOffsets);
		++iter)
	{
		FBModel *pModel = iter->first;

		if (true == pModel->IsVisible(pEvaluateInfo) 
			&& (true == pModel->IsDeformable || false == iter->second.updated)
			&& true == pModel->IsEvaluationReady(kFBModelEvaluationDeform, pEvaluateInfo) )
		{
			VertexOffsets offsets = iter->second;

			// update vertex positions subdata and perhaps normals subdata (maybe)
			//	do a gpu subData copy !

			FBModelVertexData *pVertexData = pModel->ModelVertexData;
			if ( false == pVertexData->IsDrawable() )
				continue;

			CopyBufferSubData( BUFFER_POSITION, pVertexData, offsets.vertexOffset * bufferElementSize );
			//CopyBufferSubData( BUFFER_NORMAL, pVertexData, offsets.vertexOffset * mBufferElementSize[BUFFER_NORMAL] );

			iter->second.updated = true;
		}

	}
	glBindBuffer( GL_COPY_READ_BUFFER, 0);
	return true;
}

void CRenderVertexConveyer::Bind()
{
	// enable and define vertex attributes

	glBindBuffer( GL_ARRAY_BUFFER, mBufferIds[BUFFER_POSITION] );
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 ); 
		
	glBindBuffer( GL_ARRAY_BUFFER, mBufferIds[BUFFER_UV] );
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 ); 

	glBindBuffer( GL_ARRAY_BUFFER, mBufferIds[BUFFER_NORMAL] );
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 ); 

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mBufferIds[BUFFER_INDEX] );
		

	glEnableVertexAttribArray(0);		// position
	glEnableVertexAttribArray(1);		// tex coords
	glEnableVertexAttribArray(2);		// normal
}

void CRenderVertexConveyer::UnBind()
{
	glDisableVertexAttribArray(0);		// position
	glDisableVertexAttribArray(1);		// tex coords
	glDisableVertexAttribArray(2);		// normal
	glDisableVertexAttribArray(3);		// tangent
}