
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

void GetVideoInfo()
{
	// write video clip
	auto fn_writeVideoClip = [&AddSomeVideo] (TiXmlElement &layerItem, FBVideo *pVideo, double &totalUncompressedSize, int i) 
	{
		if (FBIS(pVideo, FBVideoClipImage) )
		{
			FBVideoClipImage *pClip = (FBVideoClipImage*) pVideo;
			layerItem.SetAttribute( "width", pClip->Width );
			layerItem.SetAttribute( "height", pClip->Height );
			layerItem.SetAttribute( "filename", pClip->Filename );
			layerItem.SetAttribute( "format", FBVideoFormatToChar(pClip->Format) );
			layerItem.SetAttribute( "startFrame", pClip->StartFrame );
			layerItem.SetAttribute( "stopFrame", pClip->StopFrame );
			layerItem.SetAttribute( "frameRate", pClip->FrameRate );

			layerItem.SetAttribute( "imageSequence", (pClip->ImageSequence == true) ? 1 : 0 );

			totalUncompressedSize += pClip->Width * pClip->Height * VideoFormatComponentsCount( pClip->Format );

			AddSomeVideo( i, pVideo );
		}
		else
		if (FBIS(pVideo, FBVideoClip) )
		{
			FBVideoClip *pClip = (FBVideoClip*) pVideo;
			layerItem.SetAttribute( "width", pClip->Width );
			layerItem.SetAttribute( "height", pClip->Height );
			layerItem.SetAttribute( "filename", pClip->Filename );
			layerItem.SetAttribute( "format", FBVideoFormatToChar(pClip->Format) );

			totalUncompressedSize += pClip->Width * pClip->Height * VideoFormatComponentsCount( pClip->Format );

			AddSomeVideo( i, pVideo );
		}
	};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void CGPUCacherSaverQueryMOBU::GetAffectedLights(FBModelList &pList, std::vector<FBLight*> &pAffectedLights)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	pAffectedLights.reserve( pScene->Lights.GetCount() );

	for (int i=0; i<pScene->Lights.GetCount(); ++i)
		pAffectedLights.push_back(pScene->Lights[i]);
}

void CGPUCacherSaverQueryMOBU::GetAffectedMedia(const std::vector<FBTexture*> &pAffectedTextures, 
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

bool CGPUCacherSaverQueryMOBU::Init(const char *filename)
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
	}

	//
	GetAffectedMedia( lAffectedTextures, lAffectedMedia, lTextureToVideo, mTotalUncompressedSize ); 

	return (pList.GetCount() > 0);
}

void ConvertShader()
	{
		// transparency mode
	int alphaSource = 0;
	FBProperty *lProp = pShader->PropertyList.Find( "Transparency" );
	if (lProp)
		alphaSource = lProp->AsInt();	
	shdItem.SetAttribute( "alpha", alphaSource );

	lProp = pShader->PropertyList.Find( "TransparencyFactor" );
	if (lProp)
	{
		double transparency;
		lProp->GetData( &transparency, sizeof(double) );
		shdItem.SetDoubleAttribute( "transparency", transparency );
	}

	// additional properties
	if ( FBIS(pShader, ProjTexShader) )
	{
		shdItem.SetAttribute( "type", (int) eShaderTypeSuperLighting );

		ProjTexShader *pSuperShader = (ProjTexShader*) pShader;
		EShadingType shadingType = pSuperShader->ShadingType;
		
	
		TiXmlElement colorCorrItem("ColorCorrection");
		
		TiXmlElement customColorItem("CustomColor");
		FBColor color = pSuperShader->CustomColor;
		customColorItem.SetDoubleAttribute( "r", color[0] );
		customColorItem.SetDoubleAttribute( "g", color[1] );
		customColorItem.SetDoubleAttribute( "b", color[2] );
		
		colorCorrItem.InsertEndChild( customColorItem );
		
		colorCorrItem.SetAttribute( "blendType", (int) pSuperShader->CustomColorMode );
		colorCorrItem.SetDoubleAttribute( "contrast", pSuperShader->Contrast );
		colorCorrItem.SetDoubleAttribute( "saturation", pSuperShader->Saturation );
		colorCorrItem.SetDoubleAttribute( "brightness", pSuperShader->Brightness );
		colorCorrItem.SetDoubleAttribute( "gamma", pSuperShader->Gamma );
		
		shdItem.InsertEndChild( colorCorrItem );

		// Toon settings
		TiXmlElement shadingItem( "Shading" );
		shadingItem.SetAttribute( "type", (int) shadingType );
		shadingItem.SetAttribute( "toonEnabled", 0 );
		shadingItem.SetDoubleAttribute( "toonSteps", pSuperShader->ToonSteps );
		shadingItem.SetDoubleAttribute( "toonDistribution", pSuperShader->ToonDistribution );
		shadingItem.SetDoubleAttribute( "toonShadowPosition", pSuperShader->ToonShadowPosition );
		
		shdItem.InsertEndChild( shadingItem );
	}
	else if ( FBIS(pShader, FXColorCorrectionShader) )
	{
		shdItem.SetAttribute( "type", (int) eShaderTypeColorCorrection );

		FXColorCorrectionShader *pSuperShader = (FXColorCorrectionShader*) pShader;

		TiXmlElement colorCorrItem("ColorCorrection");
		
		TiXmlElement customColorItem("CustomColor");
		FBColor color = pSuperShader->CustomColor;
		customColorItem.SetDoubleAttribute( "r", color[0] );
		customColorItem.SetDoubleAttribute( "g", color[1] );
		customColorItem.SetDoubleAttribute( "b", color[2] );
		
		colorCorrItem.InsertEndChild( customColorItem );
		
		colorCorrItem.SetAttribute( "blendType", (int) pSuperShader->CustomColorMode );
		colorCorrItem.SetDoubleAttribute( "contrast", pSuperShader->Contrast );
		colorCorrItem.SetDoubleAttribute( "saturation", pSuperShader->Saturation );
		colorCorrItem.SetDoubleAttribute( "brightness", pSuperShader->Brightness );
		colorCorrItem.SetDoubleAttribute( "gamma", pSuperShader->Gamma );
		
		shdItem.InsertEndChild( colorCorrItem );
	}
	else if ( FBIS(pShader, FXProjectionMapping) )
	{
		shdItem.SetAttribute( "type", (int) eShaderTypeProjections );
	}
	else if ( FBIS(pShader, FXShadingShader) )
	{
		shdItem.SetAttribute( "type", (int) eShaderTypeShading );

		FXShadingShader *pSuperShader = (FXShadingShader*) pShader;
		EShadingType shadingType = pSuperShader->ShadingType;

		// Toon settings
		TiXmlElement shadingItem( "Shading" );
		shadingItem.SetAttribute( "type", (int) shadingType );
		shadingItem.SetAttribute( "toonEnabled", 0 );
		shadingItem.SetDoubleAttribute( "toonSteps", pSuperShader->ToonSteps );
		shadingItem.SetDoubleAttribute( "toonDistribution", pSuperShader->ToonDistribution );
		shadingItem.SetDoubleAttribute( "toonShadowPosition", pSuperShader->ToonShadowPosition );
		
		shdItem.InsertEndChild( shadingItem );
	}
	else
	{
		shdItem.SetAttribute( "type", (int) eShaderTypeDefault );
	}
	}

	void ConvertMaterial()
	{
		
		FBTexture *pTexture = pMaterial->GetTexture();
		
		int index = -1;
		if (pTexture)
		{
			bool isExist = false;
			for (int k=0; k<textures.size(); ++k)
				if (textures[k] == pTexture)
				{
					index = k;
					isExist = true;
					break;
				}
		
			if (isExist == false)
			{
				index = (int) textures.size();
				textures.push_back( pTexture );
			}
		}

	}

	virtual void GetTextureMatrix( const int index, mat4 &mat ) override
	{
		FBMatrix md(pTexture->GetMatrix());

		for (int i=0; i<16; ++i)
			mf.mat_array[i] = (float) md[i];
	}

	virtual int GetTextureVideoIndex( const int index ) override;

	// information about media
	virtual double GetTotalUncompressedSize() override
	{
		return totalUncompressedSize / 1024.0 / 1024.0;
	}