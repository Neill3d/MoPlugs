
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ShaderModelInfo.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "ShaderModelInfo.h"
#include "shared_content.h"

typedef unsigned char       BYTE;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

SuperShaderModelInfo::SuperShaderModelInfo(FBShader* pShader, HKModelRenderInfo pInfo, int pSubRegionIndex)
	: FBShaderModelInfo(pShader, pInfo, pSubRegionIndex)
	, mShader(pShader)
{
	mBufferCount = 0;
	mBufferId = 0;
	mLocationId = 0;

	mMeshIndex = 0;

	mVertexData = new CGPUVertexData();	// select normal, VAO or VBUM here
}

//! a destructor
SuperShaderModelInfo::~SuperShaderModelInfo()
{
	if (mBufferId)
	{
		glDeleteBuffers( 1, &mBufferId );
		mBufferId = 0;
	}

	if (mVertexData)
	{
		delete mVertexData;
	}

	//FBShaderModelInfo::~FBShaderModelInfo();
}

bool SuperShaderModelInfo::IsSecondUVSetNeeded()
{
	FBModel *pModel = GetFBModel();
	if (pModel == nullptr) return false;

	FBGeometry *pGeometry = pModel->Geometry;
	FBStringList uvSets = pGeometry->GetUVSets();

	if (uvSets.GetCount() < 2 ) return false;

	for (int i=0; i<pModel->Textures.GetCount(); ++i)
	{
		FBTexture *pTexture = pModel->Textures[i];
		FBProperty *lProp = pTexture->PropertyList.Find( "UVSet" );
		if (lProp)
		{
			FBString str( lProp->AsString() );
			
			for (int j=1; j<uvSets.GetCount(); ++j)
			{
				FBString uvSetName(uvSets[j]);

				if (str == uvSetName)
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool SuperShaderModelInfo::PrepareUVSets()
{
	FBModel *pModel = GetFBModel();
	if (pModel == nullptr) return false;

	FBModelVertexData *lModelVertexData = pModel->ModelVertexData;
	FBGeometry *pGeometry = pModel->Geometry;
	const GLuint uvId = lModelVertexData->GetUVSetVBOId();

	FBString uvset("");
	FBStringList uvSets = pGeometry->GetUVSets();

	if (uvSets.GetCount() < 2) 
	{
		mBufferId = uvId;
		return false;
	}

	for (int i=0; i<pModel->Textures.GetCount(); ++i)
	{
		FBTexture *pTexture = pModel->Textures[i];
		FBProperty *lProp = pTexture->PropertyList.Find( "UVSet" );
		if (lProp)
		{
			FBString str(lProp->AsString() );

			for (int j=0; j<uvSets.GetCount(); ++j)
				if ( str == uvSets[j] )
				{
					uvset = lProp->AsString();
					break;
				}
		}
	}

	if (uvset == "")
	{
		mBufferId = uvId;
		return false;
	}

	//
	// lets manually prepare second uvset
	
	// TODO: include two uv sets in one buffer !
				
	int uvIndCount = 0;
	int uvCount = 0;
			
	FBGeometryReferenceMode refMode = pGeometry->GetUVSetReferenceMode( uvset );
	FBGeometryMappingMode mapping = pGeometry->GetUVSetMappingMode( uvset );
			
	int *uvIndices = pGeometry->GetUVSetIndexArray(uvIndCount, uvset );
	FBUV *uvs = pGeometry->GetUVSetDirectArray( uvCount, uvset );

	//int *indices = lModelVertexData->GetIndexArray();

	int vertCount = pGeometry->VertexCount();
	int vertCountRenderable = lModelVertexData->GetVertexCount();

	unsigned int dublicatedCount = 0;
	const int *dublicatedIndices = lModelVertexData->GetVertexArrayDuplicationMap( dublicatedCount );

	int numPolyIndices = 0;
	const int *polyIndices = ( (FBMesh*) pGeometry)->PolygonVertexArrayGet( numPolyIndices );

		
	if (mBufferId && (uvIndCount != mBufferCount) )
	{
		glDeleteBuffers(1, &mBufferId);
		mBufferId = 0;
	}

	if (mBufferId == 0)
	{
				
		switch(mapping)
		{
		case kFBGeometryMapping_BY_CONTROL_POINT:
			break;

		case kFBGeometryMapping_BY_POLYGON_VERTEX:

			//printf ("polygon vertex" );

			break;
		}

		FBUV *temp = new FBUV[vertCountRenderable];
		BYTE *flags = new BYTE[vertCount];
		BYTE *dubFlags = new BYTE[dublicatedCount];

		memset( flags, 0, sizeof(BYTE) * vertCount );
		memset( dubFlags, 0, sizeof(BYTE) * dublicatedCount );

		//int lastUpperIndex = vertCount;

		for (int i=0; i<uvIndCount; ++i)
		{
			switch(refMode)
			{
			case kFBGeometryReference_DIRECT:
				temp[i] = uvs[i];
				break;
			case kFBGeometryReference_INDEX_TO_DIRECT:

				// i - index in polygon vertex space
				// idx = index in control point space
				int idx = polyIndices[i];

				FBUV uv = uvs[ uvIndices[i] ];

				if (flags[idx] == 0)
				{
					temp[idx] = uv;
					flags[idx] = 1;
				}
				else
				{
					for (unsigned int j=0; j<dublicatedCount; ++j)
					{
						if ( (dublicatedIndices[j] == idx) && (dubFlags[j] == 0) )
						{
							temp[ vertCount + j ] = uv;

							dubFlags[j] = 1;
							break;
						}
					}

				}

				break;
			}
		}

		glGenBuffers(1, &mBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, mBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * uvIndCount, temp, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		mBufferCount = uvIndCount;
			
		// free mem
		if (dubFlags)
		{
			delete [] dubFlags;
			dubFlags = nullptr;
		}

		if (flags)
		{
			delete [] flags;
			flags = nullptr;
		}

		if (temp)
		{
			delete [] temp;
			temp = nullptr;
		}
	}
	
	return true;
}

void SuperShaderModelInfo::BindUVBuffer(const GLuint locationId)
{
	// Finally, bind buffer to use it in a shader
	if (mBufferId)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mBufferId);
		glEnableVertexAttribArray(locationId);
		glVertexAttribPointer(locationId, 2, GL_FLOAT, GL_FALSE, sizeof(FBUV), (const GLvoid*) 0 );  // uv coords
		
		mLocationId = locationId;
	}
}

void SuperShaderModelInfo::UnBindUVBuffer()
{
	if (mLocationId)
		glDisableVertexAttribArray(mLocationId);
}

void SuperShaderModelInfo::UpdateModelShaderInfo(int pShader_Version)
{
	FBShaderModelInfo::UpdateModelShaderInfo(pShader_Version);

	CHECK_GL_ERROR_MOBU();

	const unsigned int arrayIds = GetGeometryArrayIds();

	if ( (arrayIds & kFBGeometryArrayID_SecondUVSet) == kFBGeometryArrayID_SecondUVSet )
	{
		PrepareUVSets();
	}

	CHECK_GL_ERROR_MOBU();

	// TODO: calculate mesh index for this shader-model combination
	auto mGPUFBScene = &CGPUFBScene::instance();
	mMeshIndex = mGPUFBScene->FindMeshIndex( GetFBModel(), GetFBMaterial(), mShader );

	// update render model, prepare VAO for fast attrib setup
	
	const bool processTangentBuffer = arrayIds & kFBGeometryArrayID_Tangent;
	VertexDataFromFBModel(processTangentBuffer);
}

void SuperShaderModelInfo::Bind()
{
	if (mVertexData->IsReady() == false)
	{
		const unsigned int arrayIds = GetGeometryArrayIds();
		const bool processTangentBuffer = arrayIds & kFBGeometryArrayID_Tangent;

		VertexDataFromFBModel(processTangentBuffer);
	}

	mVertexData->Bind();
}

void SuperShaderModelInfo::UnBind()
{
	mVertexData->UnBind();
}

bool SuperShaderModelInfo::VertexDataFromFBModel(bool processTangentBuffer)
{
	FBModel *pModel = GetFBModel();
	if (nullptr == pModel) 
		return false;

	FBModelVertexData *pVertexData = pModel->ModelVertexData;
	if (nullptr == pVertexData) 
		return false;

	// ?!
	//if (false == pVertexData->IsDrawable() )
	//	return false;

	FBGeometryArrayID	ids[5] = { kFBGeometryArrayID_Point, kFBGeometryArrayID_Normal, kFBGeometryArrayID_Tangent,
		kFBGeometryArrayID_Binormal, kFBGeometryArrayID_Color };

	FBGeometryArrayElementType	types[6];
	void *poffsets[6];

	for (int i=0; i<5; ++i)
	{
		const auto id = ids[i];
		types[i] = pVertexData->GetVertexArrayType(id);
		poffsets[i] = pVertexData->GetVertexArrayVBOOffset(id);
	}
	types[5] = pVertexData->GetUVSetArrayFormat();
	poffsets[5] = pVertexData->GetUVSetVBOOffset();

	const GLuint positionId = pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Point );
	const GLuint normalId = pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal );
	const GLuint tangentId = (processTangentBuffer) ? pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Tangent ) : 0;
	const GLuint binormalId = (processTangentBuffer) ? pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Binormal ) : 0;
	const GLuint uvId = pVertexData->GetUVSetVBOId();
	const GLuint indexId = 0; // pVertexData->GetIndexArrayVBOId();

	if (0 == positionId || 0 == normalId || 0 == uvId )
		return false;

	mVertexData->AssignBuffers( positionId, normalId, tangentId, binormalId, uvId, indexId );
	mVertexData->AssignBufferOffsets( poffsets[0], poffsets[1], poffsets[2], poffsets[3], poffsets[5], nullptr );

	// compute deformations on GPU
	pVertexData->VertexArrayMappingRelease();

	return true;
}