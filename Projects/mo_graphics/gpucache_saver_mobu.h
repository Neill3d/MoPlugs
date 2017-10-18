
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

void GetAffectedLights(FBModelList &pList, std::vector<FBLight*> &pAffectedLights)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	pAffectedLights.reserve( pScene->Lights.GetCount() );

	for (int i=0; i<pScene->Lights.GetCount(); ++i)
		pAffectedLights.push_back(pScene->Lights[i]);
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGPUCacherSaverQueryMOBU
{

public:

	virtual const int GetVersion() {
		return 1;
	}

	virtual const char *GetSourceFilename() {
		FBString fbxFileName = FBApplication::TheOne().FBXFileName;
		return fbxFileName;
	}

	virtual const int GetModelsCount()
	{
		return pList.GetCount();
	}

	void Init()
	{
		GetAffectedLights( pList, lAffectedLights );

		FBVector3d vmin, vmax, lmin, lmax;
		int totalNumberOfVertices = 0;
		int totalNumberOfIndices = 0;

		int totalNumberOfSubMeshes = 0;

		for (int i=0; i<pList.GetCount(); ++i)
		{
			FBModel *pModel = pList[i];
			pModel->GetBoundingBox( lmin, lmax);

			FBVector4d l4Min, l4Max;
			l4Min[0] = lmin[0];
			l4Min[1] = lmin[1];
			l4Min[2] = lmin[2];
			l4Min[3] = 1.0;

			l4Max[0] = lmax[0];
			l4Max[1] = lmax[1];
			l4Max[2] = lmax[2];
			l4Max[3] = 1.0;

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
			}
			totalNumberOfIndices += numberOfIndices;

		
			auto AddSomeVideo = [&media, &textureToVideo] (const int i, FBVideo *pVideo) {
			bool IsExist=false;
			for (int ii=0; ii<media.size(); ++ii)
			{
				if (media[ii] == pVideo)
				{
					IsExist = true;
					textureToVideo[i] = ii;
					break;
				}

				// check if they share the same filename
				if ( ((FBVideoClip*)pVideo)->Filename == ((FBVideoClip*)media[ii])->Filename)
				{
					IsExist = true;
					textureToVideo[i] = ii;
					break;
				}
			}

			if (IsExist == false)
			{
				const int vidIndex = (int) media.size();
				media.push_back(pVideo);
				textureToVideo[i] = vidIndex;
			}
		}


			// add material exclusivly to the material list
		int index=0;
		if (pAffectedMaterials)
		{
			bool IsExist = false;
			int k=0;
			for (auto iter=pAffectedMaterials->begin(); iter!=pAffectedMaterials->end(); ++iter, ++k)
				if ( *iter == pMaterial)
				{
					index = k;
					IsExist = true;
					break;
				}

			if (IsExist == false)
			{
				index = (int) pAffectedMaterials->size();
				pAffectedMaterials->push_back( pMaterial );
			}
		}

		//
		//

		int index = 0;
		if (pAffectedShaders)
		{
			bool IsExist = false;
			int k=0;
			for (auto iter=pAffectedShaders->begin(); iter!=pAffectedShaders->end(); ++iter, ++k)
			{
				if ( *iter == pShader)
				{
					IsExist = true;
					index = k;
					break;
				}
			}

			if (IsExist == false)
			{
				index = (int) pAffectedShaders->size();
				pAffectedShaders->push_back(pShader);
			}
		}
	}
	
	void GetShaderAlphaSource()
	{
		FBProperty *lProp = pShader->PropertyList.Find( "Transparency" );
	if (lProp)
		alphaSource = lProp->AsInt();	
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

	virtual void GetTextureMatrix( const int index, mat4 &mat )
	{
		FBMatrix md(pTexture->GetMatrix());

	for (int i=0; i<16; ++i)
		mf.mat_array[i] = (float) md[i];
	}

	virtual int GetTextureVideoIndex( const int index );

	// information about media
	virtual double GetTotalUncompressedSize()
	{
		return totalUncompressedSize / 1024.0 / 1024.0;
	}

protected:

	FBModelList &pList;
	std::vector<FBMaterial*> lAffectedMaterials;
	std::vector<FBTexture*> lAffectedTextures;
	std::vector<FBVideo*> lAffectedMedia;
	std::vector<int>	lTextureToVideo;
	std::vector<FBShader*> lAffectedShaders;
	std::vector<FBLight*> lAffectedLights;

};