
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
class CGPUCacheSaverQueryMOBU : public CGPUCacheSaverQuery
{
public:

	// a constructor
	CGPUCacheSaverQueryMOBU( FBModelList &modelList );
	// destructor
	virtual ~CGPUCacheSaverQueryMOBU()
	{}

	virtual bool Init(const char *filename) override;

	//
	// query infromation for header

	virtual const int GetVersion() override;
	virtual const char *GetSourceFilename() override;

	//
	// query information for lights

	virtual const int GetLightsCount() override;
	
	virtual const char *GetLightName(const int index) override;
	virtual void GetLightPosition(const int index, vec4 &pos) override;
	virtual void GetLightDirection(const int index, vec4 &dir) override;
	virtual void GetLightColor(const int index, vec4 &color) override;
	virtual void GetLightAttenuation(const int index, vec4 &att) override;
	virtual void GetLightShadowing(const int index, bool &castshadow) override;

	// query information for textures

	virtual const int GetVideoCount() override;
	virtual const char *GetVideoName(const int index) override;
	virtual const int GetVideoWidth(const int index) override;
	virtual const int GetVideoHeight(const int index) override;
	virtual const int GetVideoFormat(const int index) override;
	virtual const bool IsVideoImageSequence(const int index) override;
	virtual const int GetVideoStartFrame(const int index) override;
	virtual const int GetVideoStopFrame(const int index) override;
	virtual const int GetVideoFrameRate(const int index) override;
	virtual const char *GetVideoFilename(const int index) override;
	virtual const double GetVideoSize(const int index) override;
	virtual const bool IsVideoUsedMipmaps(const int index) override;

	// information about media
	virtual double GetTotalUncompressedSize() override;
	
	//

	virtual const int GetSamplersCount() override;
	virtual const char *GetSamplerName(const int index) override; // pTexture->LongName
	virtual const int GetSamplerVideoIndex(const int index) override;	// which video is used for that sampler
	virtual void GetSamplerMatrix( const int index, mat4 &mat ) override;


	//
	// query information for materials

	virtual const int GetMaterialsCount() override;

	virtual const char *GetMaterialName(const int index) override; // pMaterial->LongName 
	virtual void ConvertMaterial(const int index, MaterialGLSL &data) override;

	//
	// query information for shaders

	virtual const int GetShadersCount() override;

	virtual const char *GetShaderName(const int index) override;
	virtual const int GetShaderAlphaSource(const int index) override;
	virtual void ConvertShader(const int index, ShaderGLSL &data) override;

	//
	// query information for models

	virtual const int GetModelsCount() override;
	virtual const int GetSubMeshesCount() override;
	virtual const unsigned int GetTotalCounts(unsigned int &vertices, unsigned int &indices) override;
	// NOTE: should be calculated in global world space !
	virtual void GetBoundingBox(vec4 &bmin, vec4 &bmax) override;

	virtual const char *GetModelName(const int modelId) override; // longname
	virtual const int GetModelVisible(const int modelId) override; // (pModel->IsVisible()) ? 1 : 0
	virtual const int GetModelCastsShadows(const int modelId) override; // (pModel->CastsShadows) ? 1 : 0
	virtual const int GetModelReceiveShadows(const int modelId) override; // (pModel->ReceiveShadows) ? 1 : 0
	virtual void GetModelMatrix(const int modelId, mat4 &mat) override;
	virtual void GetModelTranslation(const int modelId, vec4 &pos) override;
	virtual void GetModelRotation(const int modelId, vec4 &rot) override;
	virtual void GetModelScaling(const int modelId, vec4 &scaling) override;
	// NOTE: should be calculated in global world space !
	virtual void GetModelBoundingBox(const int modelId, vec4 &bmin, vec4 &bmax) override;

	// model geometry

	virtual const int GetModelVertexCount(const int modelId) override; // pVertexData->GetVertexCount()
	virtual const int GetModelUVCount(const int modelId) override;
	virtual void ModelVertexArrayRequest(const int modelId) override;

	virtual const float *GetModelVertexArrayPoint( const bool afterDeform ) override;
	virtual const float *GetModelVertexArrayNormal( const bool afterDeform ) override;
	virtual const float *GetModelVertexArrayTangent( const bool afterDeform ) override;
	virtual const float *GetModelVertexArrayUV( const int uvset, const bool afterDeform ) override;
	virtual const int *GetModelIndexArray() override;
	virtual void ModelVertexArrayRelease() override;

	// CGPUVertexData::GetStrideFromArrayElementType( pVertexData->GetVertexArrayType(kFBGeometryArrayID_Point) ) 
	virtual const int GetModelVertexArrayPointStride(const int modelId) override;
	virtual const int GetModelVertexArrayNormalStride(const int modelId) override;
	virtual const int GetModelVertexArrayTangentStride(const int modelId) override;
	virtual const int GetModelVertexArrayUVStride(const int modelId) override;

	virtual const int GetModelSubPatchCount(const int index) override;
	virtual void GetModelSubPatchInfo(const int modelid, const int patchid, int &offset, int &size, int &materialId) override;

	virtual const unsigned int GetModelShadersCount(const int index) override;
	virtual const int GetModelShaderId(const int index, const int nshader) override;


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
};