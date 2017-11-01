
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_saver_mobu.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "gpucache_saver.h"
#include "ContentInspector.h"

#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGPUCacherSaverQueryMOBU : public CGPUCacheSaverQuery
{
public:

	// a constructor
	CGPUCacherSaverQueryMOBU( FBModelList &modelList )
		: CGPUCacheSaverQuery()
		, pList(modelList)
	{}

	virtual bool Init(const char *filename) override;

	//
	// query infromation for header

	virtual const int GetVersion() {
		return 1;
	}
	virtual const char *GetSourceFilename()
	{
		FBString fbxFileName = FBApplication::TheOne().FBXFileName;
		return fbxFileName;
	}

	//
	// query information for lights

	virtual const int GetLightsCount() {
		return (int) lAffectedLights.size();
	}
	
	virtual const char *GetLightName(const int index) override
	{
		return lAffectedLights[index]->LongName;
	}
	virtual void GetLightPosition(const int index, vec4 &pos) override;
	virtual void GetLightDirection(const int index, vec4 &dir) override;
	virtual void GetLightColor(const int index, vec4 &color) override;
	virtual void GetLightAttenuation(const int index, vec4 &att) override;
	virtual void GetLightShadowing(const int index, bool &castshadow) override;

	// query information for textures

	virtual const int GetVideoCount() override
	{
		return (int) lAffectedMedia.size();
	}
	virtual const char *GetVideoName(const int index) override
	{
		return lAffectedMedia[index]->LongName;
	}
	virtual const int GetVideoWidth(const int index) override
	{
		FBVideo *pVideo = lAffectedMedia[index];
		
		if (FBIS(pVideo, FBVideoClip) )
			return ( (FBVideoClip*) pVideo)->Width;
		return 1;
	}
	virtual const int GetVideoHeight(const int index) override
	{
		FBVideo *pVideo = lAffectedMedia[index];
		
		if (FBIS(pVideo, FBVideoClip) )
			return ( (FBVideoClip*) pVideo)->Height;
		return 1;
	}
	virtual const int GetVideoFormat(const int index) override;
	virtual const bool IsVideoImageSequence(const int index) override;
	virtual const int GetVideoStartFrame(const int index) override;
	virtual const int GetVideoStopFrame(const int index) override;
	virtual const int GetVideoFrameRate(const int index) override;
	virtual const char *GetVideoFilename(const int index) override;
	virtual const double GetVideoSize(const int index) override;
	virtual const bool IsVideoUsedMipmaps(const int index) override;

	// information about media
	virtual double GetTotalUncompressedSize() override
	{
		return mTotalUncompressedSize;
	}

	//

	virtual const int GetSamplersCount() override
	{
		return (int) lAffectedTextures.size();
	}
	virtual const char *GetSamplerName(const int index) override // pTexture->LongName
	{
		return lAffectedTextures[index]->LongName;
	}
	virtual const int GetSamplerVideoIndex(const int index) override	// which video is used for that sampler
	{
		return lTextureToVideo[index];
	}

	virtual void GetSamplerMatrix( const int index, mat4 &mat ) override
	{
		FBTexture *pTexture = lAffectedTextures[index];
		FBMatrix md(pTexture->GetMatrix());

		for (int i=0; i<16; ++i)
			mat.mat_array[i] = (float) md[i];
	}


	//
	// query information for materials

	virtual const int GetMaterialsCount() override
	{
		return (int) lAffectedMaterials.size();
	}

	virtual const char *GetMaterialName(const int index) override // pMaterial->LongName 
	{
		return lAffectedMaterials[index]->LongName;
	}
	virtual void ConvertMaterial(const int index, MaterialGLSL &data) override
	{
		FBMaterial *pMaterial = lAffectedMaterials[index];
		CMaterialsInspector::ConstructFromFBMaterial(pMaterial, FBGetDisplayInfo(), data);
	}

	//
	// query information for shaders

	virtual const int GetShadersCount() override
	{
		return (int) lAffectedShaders.size();
	}

	virtual const char *GetShaderName(const int index) override
	{
		return lAffectedShaders[index]->LongName;
	}
	virtual const int GetShaderAlphaSource(const int index) override
	{
		int alphaSource = 0;

		FBProperty *lProp = lAffectedShaders[index]->PropertyList.Find( "Transparency" );
		if (lProp)
			alphaSource = lProp->AsInt();

		return alphaSource;
	}
	virtual void ConvertShader(const int index, ShaderGLSL &data) override
	{
		FBShader *pShader = lAffectedShaders[index];
		CShadersInspector::ConstructFromFBShader(pShader, FBGetDisplayInfo(), data);
	}

	//
	// query information for models

	virtual const int GetModelsCount() override {
		return mModelsCount;
	}
	virtual const int GetSubMeshesCount() override {
		return mSubMeshesCount;
	}
	virtual const unsigned int GetTotalCounts(unsigned int &vertices, unsigned int &indices) override
	{
		vertices = totalNumberOfVertices;
		indices = totalNumberOfIndices;
	}
	// NOTE: should be calculated in global world space !
	virtual void GetBoundingBox(vec4 &bmin, vec4 &bmax) override
	{
		bmin = vec4( (float)vmin[0], (float)vmin[1], (float)vmin[2], 0.0f );
		bmax = vec4( (float)vmax[0], (float)vmax[1], (float)vmax[2], 0.0f );
	}

	virtual const char *GetModelName(const int modelId) override // longname
	{
		FBModel *pModel = pList[modelId];
		return pModel->LongName;
	}
	virtual const int GetModelVisible(const int modelId) override // (pModel->IsVisible()) ? 1 : 0
	{
		FBModel *pModel = pList[modelId];
		return (pModel->IsVisible()) ? 1 : 0;
	}
	virtual const int GetModelCastsShadows(const int modelId) override // (pModel->CastsShadows) ? 1 : 0
	{
		FBModel *pModel = pList[modelId];
		return (pModel->CastsShadows) ? 1 : 0;
	}
	virtual const int GetModelReceiveShadows(const int modelId) override // (pModel->ReceiveShadows) ? 1 : 0
	{
		FBModel *pModel = pList[modelId];
		return (pModel->ReceiveShadows) ? 1 : 0;
	}
	virtual void GetModelMatrix(const int modelId, mat4 &mat) override
	{
		FBMatrix mdl;
		FBModel *pModel = pList[modelId];
		pModel->GetMatrix(mdl);
		
		for (int i=0; i<16; ++i)
			mat.mat_array[i] = (float) mdl[i];
	}
	virtual void GetModelTranslation(const int modelId, vec4 &pos) override
	{
		FBVector3d v;
		FBModel *pModel = pList[modelId];
		pModel->GetVector(v);

		pos = vec4( (float)v[0], (float)v[1], (float)v[2], 1.0);
	}
	virtual void GetModelRotation(const int modelId, vec4 &rot) override
	{
		FBVector3d v;
		FBModel *pModel = pList[modelId];
		pModel->GetVector(v, kModelRotation);

		rot = vec4( (float)v[0], (float)v[1], (float)v[2], 1.0);
	}
	virtual void GetModelScaling(const int modelId, vec4 &scaling) override
	{
		FBVector3d v;
		FBModel *pModel = pList[modelId];
		pModel->GetVector(v, kModelScaling);

		scaling = vec4( (float)v[0], (float)v[1], (float)v[2], 1.0);
	}
	// NOTE: should be calculated in global world space !
	virtual void GetModelBoundingBox(const int modelId, vec4 &bmin, vec4 &bmax) override
	{
		FBVector3d v1, v2;

		FBMatrix mdl;
		FBModel *pModel = pList[modelId];
		pModel->GetMatrix(mdl);
		pModel->GetBoundingBox(v1, v2);

		FBVector4d lmin, lmax;
		FBVectorMatrixMult( lmin, mdl, FBVector4d(v1[0], v1[1], v1[2], 1.0) );
		FBVectorMatrixMult( lmax, mdl, FBVector4d(v2[0], v2[1], v2[2], 1.0) );

		bmin = vec4( (float)lmin[0], (float)lmin[1], (float)lmin[2], 1.0 );
		bmax = vec4( (float)lmax[0], (float)lmax[1], (float)lmax[2], 1.0 );
	}

	// model geometry

	virtual const int GetModelVertexCount(const int modelId) override // pVertexData->GetVertexCount()
	{
		FBModel *pModel = pList[modelId];
		FBModelVertexData *pData = pModel->ModelVertexData;
		if (nullptr != pData)
		{
			return pData->GetVertexCount();
		}
		return 0;
	}
	virtual const int GetModelUVCount(const int modelId) override
	{
		FBModel *pModel = pList[modelId];
		FBModelVertexData *pData = pModel->ModelVertexData;
		if (nullptr != pData)
		{
			return pData->GetVertexCount();
		}
		return 0;
	}

	virtual void ModelVertexArrayRequest(const int modelId) override
	{
		FBModel *pModel = pList[modelId];
		mVertexData = pModel->ModelVertexData;
		mVertexData->VertexArrayMappingRequest();
	}
	virtual const float *GetModelVertexArrayPoint( const bool afterDeform ) override
	{
		return (float*) mVertexData->GetVertexArray( kFBGeometryArrayID_Point, afterDeform );
	}
	virtual const float *GetModelVertexArrayNormal( const bool afterDeform ) override
	{
		return (float*) mVertexData->GetVertexArray( kFBGeometryArrayID_Normal, afterDeform );
	}
	virtual const float *GetModelVertexArrayTangent( const bool afterDeform ) override
	{
		return (float*) mVertexData->GetVertexArray( kFBGeometryArrayID_Tangent, afterDeform );
	}
	virtual const float *GetModelVertexArrayUV( const int uvset, const bool afterDeform ) override
	{
		return (float*) mVertexData->GetUVSetArray();
	}
	virtual const int *GetModelIndexArray() override
	{
		return mVertexData->GetIndexArray();
	}
	virtual void ModelVertexArrayRelease() override
	{
		mVertexData->VertexArrayMappingRelease();
	}

	// CGPUVertexData::GetStrideFromArrayElementType( pVertexData->GetVertexArrayType(kFBGeometryArrayID_Point) ) 
	virtual const int GetModelVertexArrayPointStride(const int modelId) override
	{
		return sizeof(FBVertex);
	}
	virtual const int GetModelVertexArrayNormalStride(const int modelId) override
	{
		return sizeof(FBNormal);
	}
	virtual const int GetModelVertexArrayTangentStride(const int modelId) override
	{
		return sizeof(FBNormal);
	}
	virtual const int GetModelVertexArrayUVStride(const int modelId) override
	{
		return sizeof(FBUV);
	}

	virtual const int GetModelSubPatchCount(const int index) override
	{
		FBModel *pModel = pList[index];
		mVertexData = pModel->ModelVertexData;
		return mVertexData->GetSubPatchCount();
	}
	virtual void GetModelSubPatchInfo(const int modelid, const int patchid, int &offset, int &size, int &materialId) override
	{
		FBModel *pModel = pList[modelid];
		mVertexData = pModel->ModelVertexData;
		offset = mVertexData->GetSubPatchIndexOffset(patchid);
		size = mVertexData->GetSubPatchIndexSize(patchid);
		materialId = mVertexData->GetSubPatchMaterialId(patchid);
	}

	virtual const unsigned int GetModelShadersCount(const int index) override
	{
		FBModel *pModel = pList[index];
		return pModel->Shaders.GetCount();
	}
	virtual const int GetModelShaderId(const int index, const int nshader) override
	{
		FBModel *pModel = pList[index];
		FBShader *pShader = pModel->Shaders[nshader];

		int result = -1;
		int shaderNdx = 0;
		for (auto iter=begin(lAffectedShaders); iter!=end(lAffectedShaders); 
			++iter, ++shaderNdx)
		{
			if (pShader == *iter)
			{
				result = shaderNdx;
				break;
			}
		}

		return result;
	}


protected:

	FBModelList &pList;
	std::vector<FBMaterial*> lAffectedMaterials;
	std::vector<FBTexture*> lAffectedTextures;
	std::vector<FBVideo*> lAffectedMedia;
	std::vector<int>	lTextureToVideo;
	std::vector<FBShader*> lAffectedShaders;
	std::vector<FBLight*> lAffectedLights;

	int		mModelsCount;
	int		mSubMeshesCount;

	FBVector3d vmin, vmax;
	int totalNumberOfVertices;
	int totalNumberOfIndices;

	double	mTotalUncompressedSize;

	FBModelVertexData		*mVertexData;

	static void GetAffectedLights(FBModelList &pList, std::vector<FBLight*> &pAffectedLights);
	static void GetAffectedMedia(const std::vector<FBTexture*> &pAffectedTextures, 
		std::vector<FBVideo*> &pAffectedMedia, std::vector<int> &lTextureToVideo, double &totalUncompressedSize);

	void ConvertShader();
	void ConvertMaterial();
};