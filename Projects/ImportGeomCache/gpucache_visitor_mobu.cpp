
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_visitor_mobu.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "gpucache_visitor_mobu.h"
#include "..\Common\nv_dds.h"
#include "..\Common\nvImage.h"

///////////////////////////////////////////////////////////////////////////////////////////
//

CMoBuVisitor::CMoBuVisitor(bool useDynamicTextures, bool askForStoragePath, const char *storagePath)
	: mUseDynamicTextures(useDynamicTextures)
	, mAskForStoragePath(askForStoragePath)
	, mStoragePath(storagePath)
{
	mTextureIndex = 0;
}

CMoBuVisitor::~CMoBuVisitor()
{
}

void CMoBuVisitor::OnReadHeader(const char *xmlFilename, const char *sourceFilename)
{

}
	
	
bool CMoBuVisitor::OnReadTexturesBegin( const char *textures_filename, const int numberOfSamplers, const int numberOfImages )
{
	// DONE: choose folder for dds images extraction
	if (mUseDynamicTextures)
	{
		return DynamicTexturesBegin(textures_filename, numberOfSamplers, numberOfImages);
	}
	else
	{
		return ResourceTexturesBegin(textures_filename, numberOfSamplers, numberOfImages);
	}

	return true;
}

void CMoBuVisitor::OnReadTexturesImage1(const ImageHeader *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData)
{
	if (mUseDynamicTextures)
	{
		DynamicTexturesImage1(header, fileImageOffset, imageSize, imageData);
	}
	else
	{
		ResourceTexturesImage1(header, fileImageOffset, imageSize, imageData);
	}
}

void CMoBuVisitor::OnReadTexturesImage2(const BYTE type, const ImageHeader2 *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData)
{
	if (mUseDynamicTextures)
	{
		DynamicTexturesImage2(type, header, fileImageOffset, imageSize, imageData);
	}
	else
	{
		ResourceTexturesImage2(type, header, fileImageOffset, imageSize, imageData);
	}
}

void CMoBuVisitor::OnReadEmptyImage()
{
}

void CMoBuVisitor::OnReadTexturesSampler(const char *sampler_name, const char *sampler_file, const SamplerHeader *header, const size_t fileSamplerOffset, const size_t samplerSize, const BYTE *samplerData)
{
	if (mUseDynamicTextures)
	{
		DynamicTexturesSampler(sampler_name, sampler_file, header, fileSamplerOffset, samplerSize, samplerData);
	}
	else
	{
		ResourceTexturesSampler(sampler_name, sampler_file, header, fileSamplerOffset, samplerSize, samplerData);
	}
}

void CMoBuVisitor::OnReadTexturesError(const char *what)
{
	if (mUseDynamicTextures)
	{
		DynamicTexturesError(what);
	}
	else
	{
		ResourceTexturesError(what);
	}
}

void CMoBuVisitor::OnReadTexturesEnd()
{
	if (mUseDynamicTextures)
	{
		DynamicTexturesEnd();
	}
	else
	{
		ResourceTexturesEnd();
	}
}
	


bool CMoBuVisitor::OnReadMaterialsBegin(const int numberOfMaterials)
{
	mMaterials.reserve( numberOfMaterials );
	return true;
}

void CMoBuVisitor::OnReadMaterial(const char *material_name, const MaterialGLSL &material )
{
	FBMaterial *pMaterial = new FBMaterial( (material_name) ? material_name : "material" );
	
	//pMaterial->Ambient = FBColor( material.ambientColor.x, material.ambientColor.y, material.ambientColor.z );
	//pMaterial->AmbientFactor = material.ambientColor.w;
	pMaterial->Diffuse = FBColor( material.diffuseColor.x, material.diffuseColor.y, material.diffuseColor.z );
	pMaterial->DiffuseFactor = material.diffuseColor.w;

	if (material.useDiffuse > 0.0f && material.diffuse >= 0 && material.diffuse < (int)mTextures.size() )
	{
		if (mTextures[material.diffuse] != nullptr)
			pMaterial->SetTexture( mTextures[material.diffuse] );
	}

	mMaterials.push_back(pMaterial);
}

void CMoBuVisitor::OnReadMaterialsEnd()
{
}


bool CMoBuVisitor::OnReadShadersBegin(const int numberOfShaders)
{
	mShaders.reserve(numberOfShaders);
	return true;
}

void CMoBuVisitor::OnReadShader(const char *shader_name, const int alphatype, const ShaderGLSL &shaderdata )
{
	FBShader *pShader = nullptr;

	switch(shaderdata.shaderType)
	{
	case eShaderTypeColorCorrection:
		pShader = (FBShader*) FBCreateObject( "FbxStorable/Shader", 
														"FX Color Correction Plugin", 
														shader_name ) ;
		break;

	case eShaderTypeShading:
		pShader = (FBShader*) FBCreateObject( "FbxStorable/Shader", 
														"FX Shading Plugin", 
														shader_name ) ;
		break;

	case eShaderTypeProjections:

		pShader = (FBShader*) FBCreateObject( "FbxStorable/Shader", 
														"FX Projection Mapping Plugin", 
														shader_name ) ;
		break;

	default:	// projection mapping shader

		pShader = (FBShader*) FBCreateObject( "FbxStorable/Shader",
														"Projected texture Plugin", 
														shader_name ) ;
		break;
	}

	mShaders.push_back( pShader );

	if (pShader == nullptr)
	{
		return;
	}

	// set properties

	FBProperty *prop = nullptr;
	double value = 0.0;
	FBColor color;

	prop = pShader->PropertyList.Find( "Transparency" );
	if (prop) prop->SetInt( alphatype );

	prop = pShader->PropertyList.Find( "TransparencyFactor" );
	if (prop) {
		value = (double) shaderdata.transparency;
		prop->SetData( &value );
	}

	prop = pShader->PropertyList.Find( "Shading" );
	if (prop) prop->SetInt( shaderdata.shadingType );

	//

	prop = pShader->PropertyList.Find( "Toon Steps" );
	if (prop) {
		value = (double) shaderdata.toonSteps;
		prop->SetData( &value );
	}

	prop = pShader->PropertyList.Find( "Toon Distribution" );
	if (prop) {
		value = (double) shaderdata.toonDistribution;
		prop->SetData( &value );
	}

	prop = pShader->PropertyList.Find( "Toon Shadow Position" );
	if (prop) {
		value = (double) shaderdata.toonShadowPosition;
		prop->SetData( &value );
	}

	//

	prop = pShader->PropertyList.Find( "Brightness" );
	if (prop) {
		value = 100.0 * ((double) shaderdata.brightness - 1.0);
		prop->SetData( &value );
	}

	prop = pShader->PropertyList.Find( "Saturation" );
	if (prop) {
		value = 100.0 * ((double) shaderdata.saturation - 1.0);
		prop->SetData( &value );
	}

	prop = pShader->PropertyList.Find( "Contrast" );
	if (prop) {
		value = 100.0 * ((double) shaderdata.contrast - 1.0);
		prop->SetData( &value );
	}

	prop = pShader->PropertyList.Find( "Gamma" );
	if (prop) {
		value = 100.0 * (double) shaderdata.gamma;
		prop->SetData( &value );
	}

	//

	prop = pShader->PropertyList.Find( "Custom color" );
	if (prop) {
		color = FBColor(shaderdata.customColor.x, shaderdata.customColor.y, shaderdata.customColor.z );
		prop->SetData( &color );
	}

	prop = pShader->PropertyList.Find( "Custom color mode" );
	if (prop) prop->SetInt( (int) shaderdata.customColor.w );

	//

	prop = pShader->PropertyList.Find( "Depth Displacement" );
	if (prop) {
		value = (double) shaderdata.depthDisplacement;
		prop->SetData( &value );
	}
}

void CMoBuVisitor::OnReadShadersEnd()
{
}


bool CMoBuVisitor::OnReadModelsBegin(const int numberOfModels, const int numberOfMeshes, const double *bounding_min, const double *bounding_max)
{
	FBMergeTransactionBegin();
	return true;
}

void CMoBuVisitor::OnReadVertexData( FileGeometryHeader *const pHeader, const BYTE *data )
{
}

void CMoBuVisitor::OnReadModel(const char *name, 
						const double *translation, 
						const double *rotation, 
						const double *scaling, 
						const double *bounding_min,
						const double *bounding_max,
						const int numberOfShaders,
						const int *shaders,
						const VertexDataHeader *pheader, 
						const BYTE *data)
{
	
	mModel = new FBModel( name );
	mModel->Translation = FBVector3d( (double*)translation );
	mModel->Rotation = FBVector3d( (double*)rotation );
	mModel->Scaling = FBVector3d( (double*)scaling );

	for (int i=0; i<numberOfShaders; ++i)
	{
		if (shaders[i] >= 0 && shaders[i] < (int) mShaders.size() )
			if (mShaders[shaders[i]] != nullptr)
			{
				mModel->Shaders.Add( mShaders[shaders[i]] );
			}
	}

	mMesh = new FBMesh(name);

	// always call geometrybegin / geometryend in pair when editing geometry
	mMesh->GeometryBegin();

	unsigned int ids = kFBGeometryArrayID_Point | kFBGeometryArrayID_Normal;
	mMesh->VertexArrayInit( pheader->numVertices, false, ids );

	int count=0;
	FBVertex *vertices = mMesh->GetPositionsArray(count);
	if (vertices)
	{
		memcpy( vertices, (data+pheader->positionOffset), count*gPointStride );
	}
	
	FBUV *uvs = mMesh->GetUVSetDirectArray(count);
	if (uvs)
	{
		memcpy( uvs, (data+pheader->uvOffset), count*gUVStride );
	}

	FBNormal *normals = mMesh->GetNormalsDirectArray(count);
	if (normals)
	{
		memcpy( normals, (data+pheader->normalOffset), count*gNormalStride );
	}

	mIndicesData = (int*) (data + pheader->indicesOffset);
	mMatIds.clear();
}

void CMoBuVisitor::OnReadModelPatch(const int offset, const int size, const int materialId)
{
	if (mMesh)
	{
		int index = -1;

		for (size_t i=0; i<mMatIds.size(); ++i)
			if (mMatIds[i] == materialId)
			{
				index = (int) i;
				break;
			}

		if (index < 0)
		{
			index = 0;
			mMatIds.push_back(materialId);
		}

		mMesh->PolygonListAdd( 3, size, mIndicesData+offset, index );
	}
}

void CMoBuVisitor::OnReadModelFinish()
{
	if (mMesh && mModel)
	{
		mMesh->GeometryEnd();
		mModel->Geometry = mMesh;
		mModel->ShadingMode = kFBModelShadingAll;

		// connect materials
		for (size_t i=0; i<mMatIds.size(); ++i)
			if ( mMatIds[i] < (int) mMaterials.size() )
			{
				mModel->ConnectSrc( mMaterials[mMatIds[i]] );
			}
		
		mModel->Visibility = true;
		mModel->Show = true;
	}
}

void CMoBuVisitor::OnReadModelsEnd()
{
	FBMergeTransactionEnd();
}