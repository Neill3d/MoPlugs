
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_saver_mobu.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "gpucache_saver_mobu.h"
#include <algorithm>



const char *FBVideoFormatToChar( FBVideoFormat format )
{
	switch(format)
	{
	case kFBVideoFormat_Any: return "unsupported";
	case kFBVideoFormat_Other: return "unsupported";
	case kFBVideoFormat_RGBA_32: return "RGBA";
	case kFBVideoFormat_RGB_24: return "RGB";
	case kFBVideoFormat_BGRA_32: return "BGRA";
	case kFBVideoFormat_BGR_24: return "BGR";
	case kFBVideoFormat_BGR_16: return "BGR";
	case kFBVideoFormat_ABGR_32: return "ABGR";
	case kFBVideoFormat_ARGB_32: return "unsupported";
	case kFBVideoFormat_422: return "unsupported";
	}

	return "unsupported";
}

// return false if a format is unsupported
bool FBVideoFormatToOpenGL(const FBVideoFormat clipFormat, GLint &internalFormat, GLint &format, const bool compressed)
{
	switch (clipFormat)
	{
	case kFBVideoFormat_Any:
		return false;
		break;
	case kFBVideoFormat_Other:
		return false;
		break;
	case kFBVideoFormat_RGBA_32:
		internalFormat = (compressed) ? GL_COMPRESSED_RGBA : GL_RGBA8;
		format = GL_RGBA;
		break;
	case kFBVideoFormat_RGB_24:
		internalFormat = (compressed) ? GL_COMPRESSED_RGB : GL_RGB8;
		format = GL_RGB;
		break;
	case kFBVideoFormat_BGRA_32:
		internalFormat = (compressed) ? GL_COMPRESSED_RGBA : GL_BGRA;
		format = GL_RGBA;
		break;
	case kFBVideoFormat_BGR_24:
		internalFormat = (compressed) ? GL_COMPRESSED_RGB : GL_BGR; 
		format = GL_RGB;
		break;
	case kFBVideoFormat_BGR_16:
		return false;
		break;
	case kFBVideoFormat_ABGR_32:
		internalFormat = (compressed) ? GL_COMPRESSED_RGBA : GL_ABGR_EXT;
		format = GL_RGBA;
		break;
	case kFBVideoFormat_ARGB_32:
		return false;
		break;
	case kFBVideoFormat_422:
		return false;
		break;
	}

	return true;
}

int VideoFormatComponentsCount( FBVideoFormat format )
{
	if (format==kFBVideoFormat_ABGR_32 || format==kFBVideoFormat_ARGB_32 || format == kFBVideoFormat_BGRA_32 || format == kFBVideoFormat_RGBA_32 )
		return 4;

	return 3;
}




bool GetObjectsFromXML( const char *filename, FBModelList &pList )
{
	std::vector<FBMaterial*> lAffectedMaterials;
	std::vector<FBTexture*> lAffectedTextures;
	std::vector<FBVideo*> lAffectedMedia;
	std::vector<int>	lTextureToVideo;
	std::vector<FBShader*> lAffectedShaders;

	TiXmlDocument	doc;
	TiXmlNode *node = nullptr;
	TiXmlElement *modelsElem = nullptr;
	TiXmlElement *modelElem = nullptr;
	TiXmlAttribute  *attrib = nullptr;

	try
	{

		if (doc.LoadFile( filename ) == false)
			throw std::exception( "failed to open xml file" );

		pList.Clear();

		// TODO: replace models positions and global bounding box
		node = doc.FirstChild("Models");
		if (node == nullptr)
			throw std::exception( "failed to find models group in cache" );

		modelsElem = node->ToElement();
		if (modelsElem)
		{
			
			modelElem = modelsElem->FirstChildElement("Model");

			FBString modelName;

			while (modelElem)
			{

				// enumerate attribs
				for( attrib = modelElem->FirstAttribute();
						attrib;
						attrib = attrib->Next() )
				{
					if ( strcmp(attrib->Name(), "name") == 0 )
					{
						modelName = attrib->Value();

						FBModel *pModel = FBFindModelByLabelName(modelName);
						if (pModel)
						{
							pList.Add(pModel);
						}
					}
					
				}

				modelElem = modelElem->NextSiblingElement();
			}
		}

	}
	catch (const std::exception &e)
	{
		printf( "%s\n", e.what() );
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void CGPUCacheSaverQueryMOBU::GetAffectedLights(FBModelList &pList, std::vector<FBLight*> &pAffectedLights)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	pAffectedLights.reserve( pScene->Lights.GetCount() );

	for (int i=0; i<pScene->Lights.GetCount(); ++i)
		pAffectedLights.push_back(pScene->Lights[i]);
}

void CGPUCacheSaverQueryMOBU::GetAffectedMedia(const std::vector<FBTexture*> &pAffectedTextures, 
												std::vector<FBVideo*> &media, std::vector<int> &textureToVideo, double &totalUncompressedSize)
{
	totalUncompressedSize = 0.0;
	textureToVideo.resize( pAffectedTextures.size() );

	auto AddSomeVideo = [&media, &textureToVideo] (const int textureId, FBVideo *pVideo) 
	{
		bool IsExist=false;
		for (int ii=0; ii<media.size(); ++ii)
		{
			if (media[ii] == pVideo)
			{
				IsExist = true;
				textureToVideo[textureId] = ii;
				break;
			}

			// check if they share the same filename
			FBString theVideoFilename = ((FBVideoClip*)pVideo)->Filename;
			FBString theNextVideoFilename = ((FBVideoClip*)media[ii])->Filename;

			if ( theVideoFilename  == theNextVideoFilename )
			{
				IsExist = true;
				textureToVideo[textureId] = ii;
				break;
			}
		}

		if (false == IsExist)
		{
			const int vidIndex = (int) media.size();
			media.push_back(pVideo);
			textureToVideo[textureId] = vidIndex;
		}
	};

	//
	int textureNdx = 0;
	for (auto iter=begin(pAffectedTextures); iter!=end(pAffectedTextures); 
			++iter, 
			++textureNdx)
	{
		FBVideo *pVideo = (*iter)->Video;
		if (nullptr == pVideo)
			continue;

		if (FBIS(pVideo, FBVideoMemory) )
		{
			// skip dynamic textures
		}
		else
		if (FBIS(pVideo, FBVideoClipImage) )
		{
			FBVideoClipImage *pClip = (FBVideoClipImage*) pVideo;
			/*
			layerItem.SetAttribute( "width", pClip->Width );
			layerItem.SetAttribute( "height", pClip->Height );
			layerItem.SetAttribute( "filename", pClip->Filename );
			layerItem.SetAttribute( "format", FBVideoFormatToChar(pClip->Format) );
			layerItem.SetAttribute( "startFrame", pClip->StartFrame );
			layerItem.SetAttribute( "stopFrame", pClip->StopFrame );
			layerItem.SetAttribute( "frameRate", pClip->FrameRate );

			layerItem.SetAttribute( "imageSequence", (pClip->ImageSequence == true) ? 1 : 0 );
			*/
			totalUncompressedSize += pClip->Width * pClip->Height * VideoFormatComponentsCount( pClip->Format );

			AddSomeVideo( textureNdx, pVideo );
		}
		else
		if (FBIS(pVideo, FBVideoClip) )
		{
			FBVideoClip *pClip = (FBVideoClip*) pVideo;
			/*
			layerItem.SetAttribute( "width", pClip->Width );
			layerItem.SetAttribute( "height", pClip->Height );
			layerItem.SetAttribute( "filename", pClip->Filename );
			layerItem.SetAttribute( "format", FBVideoFormatToChar(pClip->Format) );
			*/
			totalUncompressedSize += pClip->Width * pClip->Height * VideoFormatComponentsCount( pClip->Format );

			AddSomeVideo( textureNdx, pVideo );
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CGPUCacheSaverQueryMOBU::CGPUCacheSaverQueryMOBU( FBModelList &modelList )
		: CGPUCacheSaverQuery()
		, pList(modelList)
{
	totalNumberOfVertices = 0;
	totalNumberOfIndices = 0;

	mModelsCount = 0;
	mSubMeshesCount = 0;
	mTotalUncompressedSize = 0.0;

	mVertexData = nullptr;
}

bool CGPUCacheSaverQueryMOBU::Init(const char *filename)
{
	GetAffectedLights( pList, lAffectedLights );

	FBVector3d lmin, lmax;

	for (int i=0; i<pList.GetCount(); ++i)
	{
		FBModel *pModel = pList[i];
		pModel->GetBoundingBox( lmin, lmax);

		FBVector4d l4Min(lmin[0], lmin[1], lmin[2], 1.0);
		FBVector4d l4Max(lmax[0], lmax[1], lmax[2], 1.0);
		
		FBMatrix lMtx;
		pModel->GetMatrix( lMtx );
		FBVectorMatrixMult( l4Min, lMtx, l4Min );
		FBVectorMatrixMult( l4Max, lMtx, l4Max );

		lmin = FBVector3d(l4Min);
		lmax = FBVector3d(l4Max);

		if (i==0)
		{
			vmin = lmin;
			vmax = lmax;
		}
		else
		{
			vmin[0] = std::min( vmin[0], lmin[0] );
			vmin[1] = std::min( vmin[1], lmin[1] );
			vmin[2] = std::min( vmin[2], lmin[2] );

			vmax[0] = std::max( vmax[0], lmax[0] );
			vmax[1] = std::max( vmax[1], lmax[1] );
			vmax[2] = std::max( vmax[2], lmax[2] );
		}

		// calculate final buffers arrays
		FBModelVertexData *pVertexData = pModel->ModelVertexData;
		totalNumberOfVertices += pVertexData->GetVertexCount();

		int numberOfIndices = 0;
		for (int j=0; j<pVertexData->GetSubPatchCount(); ++j)
		{
			const int offset =	pVertexData->GetSubPatchIndexOffset(j);
			const int size =	pVertexData->GetSubPatchIndexSize(j);

			numberOfIndices = std::max( numberOfIndices, offset+size );

			// add material exclusivly to the material list
			FBMaterial *pMaterial = pVertexData->GetSubPatchMaterial(j);
		
			auto iter = std::find( begin(lAffectedMaterials), end(lAffectedMaterials), pMaterial );

			if (iter == end(lAffectedMaterials) )
				lAffectedMaterials.push_back(pMaterial);
		}

		mSubMeshesCount += pVertexData->GetSubPatchCount();

		//
		// exclusive shaders add

		for (int j=0, count=pModel->Shaders.GetCount(); j<count; ++j)
		{
			FBShader *pShader = pModel->Shaders[j];

			auto iter = std::find( begin(lAffectedShaders), end(lAffectedShaders), pShader );

			if (iter == end(lAffectedShaders) )
				lAffectedShaders.push_back(pShader);
		}

		totalNumberOfIndices += numberOfIndices;	
		mModelsCount += 1;
	}

	//
	for (auto iter=begin(lAffectedMaterials); iter!=end(lAffectedMaterials); ++iter)
	{
		FBMaterial *pMaterial = *iter;
		if ( nullptr != pMaterial->GetTexture() )
			lAffectedTextures.push_back( pMaterial->GetTexture() );
	}

	//
	GetAffectedMedia( lAffectedTextures, lAffectedMedia, lTextureToVideo, mTotalUncompressedSize ); 

	return (pList.GetCount() > 0);
}

//
// query infromation for header

const int CGPUCacheSaverQueryMOBU::GetVersion() {
	return 1;
}
const char *CGPUCacheSaverQueryMOBU::GetSourceFilename()
{
	FBString fbxFileName = FBApplication::TheOne().FBXFileName;
	return fbxFileName;
}

//
// query information for lights

const int CGPUCacheSaverQueryMOBU::GetLightsCount() {
	return (int) lAffectedLights.size();
}
	
const char *CGPUCacheSaverQueryMOBU::GetLightName(const int index)
{
	return lAffectedLights[index]->LongName;
}

void CGPUCacheSaverQueryMOBU::GetLightPosition(const int index, vec4 &pos)
{
}

void CGPUCacheSaverQueryMOBU::GetLightDirection(const int index, vec4 &dir)
{
}

void CGPUCacheSaverQueryMOBU::GetLightColor(const int index, vec4 &color)
{
}

void CGPUCacheSaverQueryMOBU::GetLightAttenuation(const int index, vec4 &att)
{
}

void CGPUCacheSaverQueryMOBU::GetLightShadowing(const int index, bool &castshadow)
{
}

// query information for textures

const int CGPUCacheSaverQueryMOBU::GetVideoCount()
{
	return (int) lAffectedMedia.size();
}

const char *CGPUCacheSaverQueryMOBU::GetVideoName(const int index)
{
	return lAffectedMedia[index]->LongName;
}
	
const int CGPUCacheSaverQueryMOBU::GetVideoWidth(const int index)
{
	FBVideo *pVideo = lAffectedMedia[index];
		
	if (FBIS(pVideo, FBVideoClip) )
		return ( (FBVideoClip*) pVideo)->Width;
	return 1;
}

const int CGPUCacheSaverQueryMOBU::GetVideoHeight(const int index)
{
	FBVideo *pVideo = lAffectedMedia[index];
		
	if (FBIS(pVideo, FBVideoClip) )
		return ( (FBVideoClip*) pVideo)->Height;
	return 1;
}

const int CGPUCacheSaverQueryMOBU::GetVideoFormat(const int index)
{
	FBVideo *pVideo = lAffectedMedia[index];

	if (FBIS(pVideo, FBVideoClip) )
	{
		GLint internalFormat, format;
		FBVideoFormatToOpenGL(((FBVideoClip*)pVideo)->Format, internalFormat, format, false);
		
		return format;
	}
	return GL_RGB;
}

const bool CGPUCacheSaverQueryMOBU::IsVideoImageSequence(const int index)
{
	FBVideo *pVideo = lAffectedMedia[index];

	if (FBIS(pVideo, FBVideoClipImage) )
	{ 
		return (((FBVideoClipImage*)pVideo)->ImageSequence == true) ? 1 : 0;
	}
	return false;
}

const int CGPUCacheSaverQueryMOBU::GetVideoStartFrame(const int index)
{
	FBVideo *pVideo = lAffectedMedia[index];

	if (FBIS(pVideo, FBVideoClipImage) )
	{ 
		return ((FBVideoClipImage*)pVideo)->StartFrame;
	}
	return 0;
}

const int CGPUCacheSaverQueryMOBU::GetVideoStopFrame(const int index)
{
	FBVideo *pVideo = lAffectedMedia[index];

	if (FBIS(pVideo, FBVideoClipImage) )
	{ 
		return ((FBVideoClipImage*)pVideo)->StopFrame;
	}
	return 0;
}

const int CGPUCacheSaverQueryMOBU::GetVideoFrameRate(const int index)
{
	FBVideo *pVideo = lAffectedMedia[index];

	if (FBIS(pVideo, FBVideoClipImage) )
	{ 
		return ((FBVideoClipImage*)pVideo)->FrameRate;
	}
	return 0;
}

const char *CGPUCacheSaverQueryMOBU::GetVideoFilename(const int index)
{
	FBVideo *pVideo = lAffectedMedia[index];

	if (FBIS(pVideo, FBVideoClip) )
	{ 
		return ((FBVideoClip*)pVideo)->Filename;
	}
	return "";
}

const double CGPUCacheSaverQueryMOBU::GetVideoSize(const int index)
{
	FBVideo *pVideo = lAffectedMedia[index];

	if (FBIS(pVideo, FBVideoClip) )
	{ 
		FBVideoClip *pClip = (FBVideoClip*) pVideo;
		return pClip->Width * pClip->Height * VideoFormatComponentsCount( pClip->Format );
	}
	return 0;
}

const bool CGPUCacheSaverQueryMOBU::IsVideoUsedMipmaps(const int index)
{
	return false;
}

// information about media
double CGPUCacheSaverQueryMOBU::GetTotalUncompressedSize()
{
	return mTotalUncompressedSize;
}

//

const int CGPUCacheSaverQueryMOBU::GetSamplersCount()
{
	return (int) lAffectedTextures.size();
}

const char *CGPUCacheSaverQueryMOBU::GetSamplerName(const int index) // pTexture->LongName
{
	return lAffectedTextures[index]->LongName;
}

const int CGPUCacheSaverQueryMOBU::GetSamplerVideoIndex(const int index) // which video is used for that sampler
{
	return lTextureToVideo[index];
}

void CGPUCacheSaverQueryMOBU::GetSamplerMatrix( const int index, mat4 &mat )
{
	FBTexture *pTexture = lAffectedTextures[index];
	FBMatrix md(pTexture->GetMatrix());

	for (int i=0; i<16; ++i)
		mat.mat_array[i] = (float) md[i];
}

//
// query information for materials

const int CGPUCacheSaverQueryMOBU::GetMaterialsCount()
{
	return (int) lAffectedMaterials.size();
}

const char *CGPUCacheSaverQueryMOBU::GetMaterialName(const int index) // pMaterial->LongName 
{
	return lAffectedMaterials[index]->LongName;
}
void CGPUCacheSaverQueryMOBU::ConvertMaterial(const int index, MaterialGLSL &data)
{
	FBMaterial *pMaterial = lAffectedMaterials[index];
	CMaterialsInspector::ConstructFromFBMaterial(pMaterial, FBGetDisplayInfo(), nullptr, data);

	// DONE: find a diffuse texture
	if (pMaterial->GetTexture() )
	{
		FBTexture *pTexture = pMaterial->GetTexture();

		for (int i=0, count=(int)lAffectedTextures.size(); i<count; ++i)
		{
			if (pTexture == lAffectedTextures[i])
			{
				data.diffuse = i;
				break;
			}
		}
	}
}

//
// query information for shaders

const int CGPUCacheSaverQueryMOBU::GetShadersCount()
{
	return (int) lAffectedShaders.size();
}

const char *CGPUCacheSaverQueryMOBU::GetShaderName(const int index)
{
	return lAffectedShaders[index]->LongName;
}
const int CGPUCacheSaverQueryMOBU::GetShaderAlphaSource(const int index)
{
	int alphaSource = 0;

	FBProperty *lProp = lAffectedShaders[index]->PropertyList.Find( "Transparency" );
	if (lProp)
		alphaSource = lProp->AsInt();

	return alphaSource;
}
void CGPUCacheSaverQueryMOBU::ConvertShader(const int index, ShaderGLSL &data)
{
	FBShader *pShader = lAffectedShaders[index];
	CShadersInspector::ConstructFromFBShader(pShader, FBGetDisplayInfo(), data);
}

//
// query information for models

const int CGPUCacheSaverQueryMOBU::GetModelsCount()  
{
	return mModelsCount;
}
const int CGPUCacheSaverQueryMOBU::GetSubMeshesCount()
{
	return mSubMeshesCount;
}
const unsigned int CGPUCacheSaverQueryMOBU::GetTotalCounts(unsigned int &vertices, unsigned int &indices)
{
	vertices = totalNumberOfVertices;
	indices = totalNumberOfIndices;

	return totalNumberOfVertices;
}
// NOTE: should be calculated in global world space !
void CGPUCacheSaverQueryMOBU::GetBoundingBox(vec4 &bmin, vec4 &bmax)
{
	bmin = vec4( (float)vmin[0], (float)vmin[1], (float)vmin[2], 0.0f );
	bmax = vec4( (float)vmax[0], (float)vmax[1], (float)vmax[2], 0.0f );
}

const char *CGPUCacheSaverQueryMOBU::GetModelName(const int modelId) // longname
{
	FBModel *pModel = pList[modelId];
	return pModel->LongName;
}
const int CGPUCacheSaverQueryMOBU::GetModelVisible(const int modelId) // (pModel->IsVisible()) ? 1 : 0
{
	FBModel *pModel = pList[modelId];
	return (pModel->IsVisible()) ? 1 : 0;
}
const int CGPUCacheSaverQueryMOBU::GetModelCastsShadows(const int modelId) // (pModel->CastsShadows) ? 1 : 0
{
	FBModel *pModel = pList[modelId];
	return (pModel->CastsShadows) ? 1 : 0;
}
const int CGPUCacheSaverQueryMOBU::GetModelReceiveShadows(const int modelId) // (pModel->ReceiveShadows) ? 1 : 0
{
	FBModel *pModel = pList[modelId];
	return (pModel->ReceiveShadows) ? 1 : 0;
}
void CGPUCacheSaverQueryMOBU::GetModelMatrix(const int modelId, mat4 &mat)
{
	FBMatrix mdl;
	FBModel *pModel = pList[modelId];
	pModel->GetMatrix(mdl);
		
	for (int i=0; i<16; ++i)
		mat.mat_array[i] = (float) mdl[i];
}
void CGPUCacheSaverQueryMOBU::GetModelTranslation(const int modelId, vec4 &pos)
{
	FBVector3d v;
	FBModel *pModel = pList[modelId];
	pModel->GetVector(v);

	pos = vec4( (float)v[0], (float)v[1], (float)v[2], 1.0);
}
void CGPUCacheSaverQueryMOBU::GetModelRotation(const int modelId, vec4 &rot)
{
	FBVector3d v;
	FBModel *pModel = pList[modelId];
	pModel->GetVector(v, kModelRotation);

	rot = vec4( (float)v[0], (float)v[1], (float)v[2], 1.0);
}
void CGPUCacheSaverQueryMOBU::GetModelScaling(const int modelId, vec4 &scaling)
{
	FBVector3d v;
	FBModel *pModel = pList[modelId];
	pModel->GetVector(v, kModelScaling);

	scaling = vec4( (float)v[0], (float)v[1], (float)v[2], 1.0);
}
// NOTE: should be calculated in global world space !
void CGPUCacheSaverQueryMOBU::GetModelBoundingBox(const int modelId, vec4 &bmin, vec4 &bmax)
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

const int CGPUCacheSaverQueryMOBU::GetModelVertexCount(const int modelId) // pVertexData->GetVertexCount()
{
	FBModel *pModel = pList[modelId];
	FBModelVertexData *pData = pModel->ModelVertexData;
	if (nullptr != pData)
	{
		return pData->GetVertexCount();
	}
	return 0;
}
const int CGPUCacheSaverQueryMOBU::GetModelUVCount(const int modelId)
{
	FBModel *pModel = pList[modelId];
	FBModelVertexData *pData = pModel->ModelVertexData;
	if (nullptr != pData)
	{
		return pData->GetVertexCount();
	}
	return 0;
}

void CGPUCacheSaverQueryMOBU::ModelVertexArrayRequest(const int modelId)
{
	FBModel *pModel = pList[modelId];
	mVertexData = pModel->ModelVertexData;
	mVertexData->VertexArrayMappingRequest();
}
const float *CGPUCacheSaverQueryMOBU::GetModelVertexArrayPoint( const bool afterDeform ) {
	return (float*) mVertexData->GetVertexArray( kFBGeometryArrayID_Point, afterDeform );
}
const float *CGPUCacheSaverQueryMOBU::GetModelVertexArrayNormal( const bool afterDeform ) {
	return (float*) mVertexData->GetVertexArray( kFBGeometryArrayID_Normal, afterDeform );
}
const float *CGPUCacheSaverQueryMOBU::GetModelVertexArrayTangent( const bool afterDeform ) {
	return (float*) mVertexData->GetVertexArray( kFBGeometryArrayID_Tangent, afterDeform );
}
const float *CGPUCacheSaverQueryMOBU::GetModelVertexArrayUV( const int uvset, const bool afterDeform ) {
	return (float*) mVertexData->GetUVSetArray();
}
const int *CGPUCacheSaverQueryMOBU::GetModelIndexArray() {
	return mVertexData->GetIndexArray();
}
void CGPUCacheSaverQueryMOBU::ModelVertexArrayRelease() {
	mVertexData->VertexArrayMappingRelease();
}

// CGPUVertexData::GetStrideFromArrayElementType( pVertexData->GetVertexArrayType(kFBGeometryArrayID_Point) ) 
const int CGPUCacheSaverQueryMOBU::GetModelVertexArrayPointStride(const int modelId) {
	return sizeof(FBVertex);
}
const int CGPUCacheSaverQueryMOBU::GetModelVertexArrayNormalStride(const int modelId) {
	return sizeof(FBNormal);
}
const int CGPUCacheSaverQueryMOBU::GetModelVertexArrayTangentStride(const int modelId) {
	return sizeof(FBNormal);
}
const int CGPUCacheSaverQueryMOBU::GetModelVertexArrayUVStride(const int modelId) {
	return sizeof(FBUV);
}

const int CGPUCacheSaverQueryMOBU::GetModelSubPatchCount(const int index) {
	FBModel *pModel = pList[index];
	mVertexData = pModel->ModelVertexData;
	return mVertexData->GetSubPatchCount();
}
void CGPUCacheSaverQueryMOBU::GetModelSubPatchInfo(const int modelid, const int patchid, int &offset, int &size, int &materialId) {
	FBModel *pModel = pList[modelid];
	mVertexData = pModel->ModelVertexData;
	offset = mVertexData->GetSubPatchIndexOffset(patchid);
	size = mVertexData->GetSubPatchIndexSize(patchid);
	materialId = mVertexData->GetSubPatchMaterialId(patchid);
}

const unsigned int CGPUCacheSaverQueryMOBU::GetModelShadersCount(const int index) {
	FBModel *pModel = pList[index];
	return pModel->Shaders.GetCount();
}
const int CGPUCacheSaverQueryMOBU::GetModelShaderId(const int index, const int nshader) {
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
