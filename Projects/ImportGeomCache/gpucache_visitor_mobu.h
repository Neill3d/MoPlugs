
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_visitor_mobu.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "gpucache_loader.h"

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <vector>

/////////////////////////////////////////////////////////////////////////////////////
//
/*
struct CGPUResource
{
	CGPUResource()
	{}
	virtual ~CGPUResource()
	{}
};

class CGPUCacheHolder
{
public:

	void Push_resource();
	void Clear_resource();

};
*/
class CMoBuVisitor : public CGPUCacheLoaderVisitor
{
public:

	//! a constructor
	CMoBuVisitor(bool useDynamicTextures, bool askForStoragePath, const char *storagePath);
	//! a destructor
	virtual ~CMoBuVisitor();

	// main header
	void OnReadHeader(const char *xmlFilename, const char *sourceFilename) override;
	
	// textures
	bool OnReadTexturesBegin( const char *textures_filename, const int numberOfSamplers, const int numberOfImages ) override;
	void OnReadTexturesImage1(const ImageHeader *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData) override;
	void OnReadTexturesImage2(const BYTE type, const ImageHeader2 *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData) override;
	void OnReadEmptyImage() override;
	void OnReadTexturesSampler(const char *sampler_name, const char *sampler_file, const SamplerHeader *header, const size_t fileSamplerOffset, const size_t samplerSize, const BYTE *samplerData) override;
	void OnReadTexturesError(const char *what) override;
	void OnReadTexturesEnd() override;
	
	// materials, shaders, etc.	
	bool OnReadMaterialsBegin(const int numberOfMaterials) override;
	void OnReadMaterial(const char *material_name, const MaterialGLSL &material ) override;
	void OnReadMaterialsEnd() override;

	bool OnReadShadersBegin(const int numberOfShaders) override;
	void OnReadShader(const char *shader_name, const int alphatype, const ShaderGLSL &shaderData ) override;
	void OnReadShadersEnd() override;

	bool OnReadModelsBegin(const int numberOfModels, const int numberOfMeshes, const double *bounding_min, const double *bounding_max) override;
	void OnReadVertexData( FileGeometryHeader *const pHeader, const BYTE *data ) override;
	void OnReadModel(const char *name, 
						const double *translation, 
						const double *rotation, 
						const double *scaling, 
						const double *bounding_min,
						const double *bounding_max,
						const int numberOfShaders,
						const int *shaders,
						const VertexDataHeader *pheader, 
						const BYTE *data) override;
	void OnReadModelPatch(const int offset, const int size, const int materialId) override;
	void OnReadModelFinish() override;
	void OnReadModelsEnd() override;

protected:

	bool			mUseDynamicTextures;
	bool			mAskForStoragePath;
	FBString		mStoragePath;

	//FBString		mTexturesPath;
	int				mTextureIndex;

	struct TextureObject
	{
		FBVideo			*pClip;
		int				width;
		int				height;
		unsigned int	textureObject;
		bool			mipmaps;

		void Set(FBVideo *_pClip, int _width, int _height, unsigned int _textureObject, bool _mipmaps)
		{
			pClip = _pClip;
			width = _width;
			height = _height;
			textureObject = _textureObject;
			mipmaps = _mipmaps;
		}
	};

	std::vector<TextureObject>		mTextureObjects;
	std::vector<FBTexture*>			mTextures;
	std::vector<FBMaterial*>		mMaterials;
	std::vector<FBShader*>			mShaders;

	FBModel							*mModel;
	FBMesh							*mMesh;
	int								*mIndicesData;

	std::vector<int>				mMatIds;

private:

	// cache textures only in video memory

	bool DynamicTexturesBegin( const char *textures_filename, const int numberOfSamplers, const int numberOfImages );
	void DynamicTexturesImage1(const ImageHeader *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData);
	void DynamicTexturesImage2(const BYTE type, const ImageHeader2 *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData);
	void DynamicTexturesSampler(const char *sampler_name, const char *sampler_file, const SamplerHeader *header, const size_t fileSamplerOffset, const size_t samplerSize, const BYTE *samplerData);
	void DynamicTexturesError(const char *what);
	void DynamicTexturesEnd();

	// load textures on disk as well

	bool ResourceTexturesBegin( const char *textures_filename, const int numberOfSamplers, const int numberOfImages );
	void ResourceTexturesImage1(const ImageHeader *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData);
	void ResourceTexturesImage2(const BYTE type, const ImageHeader2 *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData);
	void ResourceTexturesSampler(const char *sampler_name, const char *sampler_file, const SamplerHeader *header, const size_t fileSamplerOffset, const size_t samplerSize, const BYTE *samplerData);
	void ResourceTexturesError(const char *what);
	void ResourceTexturesEnd();
};