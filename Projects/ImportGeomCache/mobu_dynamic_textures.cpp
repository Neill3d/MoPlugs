
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: mobu_dynamic_textures.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "gpucache_visitor_mobu.h"

//////////////////////////////////////////////////////////////////////////////////

bool CMoBuVisitor::DynamicTexturesBegin( const char *textures_filename, const int numberOfSamplers, const int numberOfImages )
{
	// DONE: choose folder for dds images extraction
	/*
	FBFolderPopup	lDialog;

	lDialog.Caption = "Choose a folder for the dds images extraction";
	bool res = lDialog.Execute();
	
	mTexturesPath = lDialog.Path;
	*/
	mTextureIndex = 0;

	TextureObject defTextureObject;
	defTextureObject.Set(nullptr, 0, 0, 0, false);

	mTextureObjects.resize(numberOfImages, defTextureObject);
	mTextures.reserve( numberOfSamplers );

	return true;
}

void CMoBuVisitor::DynamicTexturesImage1(const ImageHeader *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData)
{
	//nv_dds::CTexture	ddsTexture;

	BYTE *stream = (BYTE*) imageData;
	stream += sizeof(ImageHeader);

	int components = (header->format==GL_RGBA) ? 4 : 3;
//	int d = (header->format==GL_RGBA) ? 32 : 24;

	GLuint texId=0;
	glGenTextures(1, &texId); 

	if (header->size == (header->width * header->height * components) )
	{
		/*
		char szFilename[256];
		memset(szFilename, 0, sizeof(char)*256);
		sprintf_s( szFilename, sizeof(char)*256, "%s\\%d.tif\0", mTexturesPath, mTextureIndex );

		FBImage *pNewImage = new FBImage( szFilename );
		pNewImage->Init( (header->format==GL_RGBA) ? kFBImageFormatRGBA32 : kFBImageFormatRGB24, header->width, header->height );
		
		BYTE *dstBuffer = pNewImage->GetBufferAddress();
		memcpy(dstBuffer, stream, sizeof(BYTE) * header->size);

		pNewImage->WriteToTif( szFilename, "Test Image", true );

		delete pNewImage;
		*/
		
		/*
		ddsTexture.create( header->width, header->height, d, header->size, stream );
		stream += header->size;

		for (int i=0; i<header->numberOfLODs; ++i)
		{
			ImageLODHeader *lodHeader = (ImageLODHeader*) stream;
			stream += sizeof(ImageLODHeader);

			nv_dds::CSurface	ddsSurface;
			ddsSurface.create( lodHeader->width, lodHeader->height, d, lodHeader->size, stream );
			ddsTexture.add_mipmap( ddsSurface );

			stream += lodHeader->size;
		}

		nv_dds::CDDSImage	ddsImage;
		ddsImage.create_textureFlat( header->format, components, ddsTexture );

		char szFilename[256];
		memset(szFilename, 0, sizeof(char)*256);
		sprintf_s( szFilename, sizeof(char)*256, "%s\\%d.dds\0", mTexturesPath, mTextureIndex );
		ddsImage.save( szFilename, false );

		FBVideoClip	*pNewClip = new FBVideoClip( szFilename );
		*/

		/*
		char szFilename[256];
		memset(szFilename, 0, sizeof(char)*256);
		sprintf_s( szFilename, sizeof(char)*256, "%s\\%d.dds\0", mTexturesPath, mTextureIndex );

		nv::Image	image;

		image.setImage( header->width, header->height, header->format, GL_UNSIGNED_BYTE, stream );
		image.saveImageToFile( szFilename );
		*/

		glBindTexture(GL_TEXTURE_2D, texId);

		//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, header->internalFormat, header->width, header->height, 0, header->format, GL_UNSIGNED_BYTE, stream);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, texId);

		//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		glCompressedTexImage2D(GL_TEXTURE_2D, 0, header->internalFormat, header->width, header->height, 0, header->size, stream);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	mTextureObjects[mTextureIndex].Set(nullptr, header->width, header->height, texId, (header->numberOfLODs > 1) );
	mTextureIndex += 1;
}

void CMoBuVisitor::DynamicTexturesImage2(const BYTE type, const ImageHeader2 *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData)
{
	// DONE: load image data directly to video memory

	BYTE *stream = (BYTE*) imageData;
	GLuint texId=0;

	// skip image type
	stream += sizeof(BYTE);

	if (type == IMAGE_TYPE_STILL)
	{
		stream += sizeof(ImageHeader2);
	
		int components = (header->format==GL_RGBA) ? 4 : 3;
//		int d = (header->format==GL_RGBA) ? 32 : 24;

		
		glGenTextures(1, &texId); 

		if (header->size == (header->width * header->height * components) )
		{
			glBindTexture(GL_TEXTURE_2D, texId);

			//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, header->internalFormat, header->width, header->height, 0, header->format, GL_UNSIGNED_BYTE, stream);

			stream += header->size;
			for (int i=1; i<header->numberOfLODs; ++i)
			{
				ImageLODHeader2		*lodHeader = (ImageLODHeader2*) stream;
				stream += sizeof(ImageLODHeader2);

				glTexImage2D(GL_TEXTURE_2D, i, header->internalFormat, lodHeader->width, lodHeader->height, 0, header->format, GL_UNSIGNED_BYTE, stream);
				stream += lodHeader->size;
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, texId);

			//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

			glCompressedTexImage2D(GL_TEXTURE_2D, 0, header->internalFormat, header->width, header->height, 0, header->size, stream);

			stream += header->size;
			for (int i=1; i<header->numberOfLODs; ++i)
			{
				ImageLODHeader2		*lodHeader = (ImageLODHeader2*) stream;
				stream += sizeof(ImageLODHeader2);

				glCompressedTexImage2D(GL_TEXTURE_2D, i, header->internalFormat, lodHeader->width, lodHeader->height, 0, lodHeader->size, stream);
				stream += lodHeader->size;
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	
	mTextureObjects[mTextureIndex].Set(nullptr, header->width, header->height, texId, (header->numberOfLODs > 1) );
	mTextureIndex += 1;
}

void CMoBuVisitor::DynamicTexturesSampler(const char *sampler_name, const char *sampler_file, const SamplerHeader *header, const size_t fileSamplerOffset, const size_t samplerSize, const BYTE *samplerData)
{
	// DONE: import as fbtexture and connect with dds video clip

	GLuint index = 0;
	if (header->videoIndex >= 0 && header->videoIndex < (int) mTextureObjects.size() )
		index = header->videoIndex;

	if (mTextureObjects[index].pClip == nullptr && mTextureObjects[index].textureObject > 0)
	{
		glBindTexture(GL_TEXTURE_2D, mTextureObjects[index].textureObject);

		if (mTextureObjects[index].mipmaps)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, header->magFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, header->minFilter);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, header->sWrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, header->tWrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, header->rWrap);

		glBindTexture(GL_TEXTURE_2D, 0);

		// load from video memory
		
		FBVideoMemory	*pNewClip = new FBVideoMemory( "videoClip" );

		pNewClip->SetObjectImageSize( mTextureObjects[index].width, mTextureObjects[index].height );
		pNewClip->TextureOGLId = mTextureObjects[index].textureObject;

		mTextureObjects[index].pClip = pNewClip;
	}

	//
	//

	FBTexture *pTexture = new FBTexture ( (sampler_name) ? sampler_name : "videoTexture" );
	mTextures.push_back(pTexture);

	if (mTextureObjects[index].pClip != nullptr)
	{
		pTexture->Video = mTextureObjects[index].pClip;
	}

	FBMatrix matrix;
	for (int i=0; i<16; ++i)
		matrix[i] = (double) header->matrix[i];

	FBTVector tr;
	FBRVector rt;
	FBSVector scl;

	FBMatrixToTRS(tr, rt, scl, matrix);

	pTexture->Translation = rt;
	pTexture->Rotation = rt;
	pTexture->Scaling = scl;
}

void CMoBuVisitor::DynamicTexturesError(const char *what)
{
	FBMessageBox( "Reading GPU Cache ERROR", what, "Ok" );
}

void CMoBuVisitor::DynamicTexturesEnd()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}