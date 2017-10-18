
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: mobu_resource_textures.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "gpucache_visitor_mobu.h"

///////////////////////////////////////////////////////////////////////////

bool CMoBuVisitor::ResourceTexturesBegin( const char *textures_filename, const int numberOfSamplers, const int numberOfImages )
{
	// DONE: choose folder for dds images extraction
	
	if (mAskForStoragePath)
	{

		FBFolderPopup	lDialog;

		lDialog.Caption = "Choose a folder for the images extraction";
		if (lDialog.Execute() )
		{
			mStoragePath = lDialog.Path;
		}
	}

	mTextureIndex = 0;

	TextureObject defTextureObject;
	defTextureObject.Set(nullptr, 0, 0, 0, false);

	mTextureObjects.resize(numberOfImages, defTextureObject);
	mTextures.reserve( numberOfSamplers );

	return true;
}

void CMoBuVisitor::ResourceTexturesImage1(const ImageHeader *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData)
{
	// check for the source image, if doesn't exist then create an empty video clip (will find resource after using the texture manager)

	mTextureObjects[mTextureIndex].Set(nullptr, header->width, header->height, 0, 0 );
	mTextureIndex += 1;
}

void CMoBuVisitor::ResourceTexturesImage2(const BYTE type, const ImageHeader2 *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData)
{
	// DONE: load image data directly to video memory
	
	mTextureObjects[mTextureIndex].Set(nullptr, header->width, header->height, 0, 0 );
	mTextureIndex += 1;
}

void CMoBuVisitor::ResourceTexturesSampler(const char *sampler_name, const char *sampler_file, const SamplerHeader *header, const size_t fileSamplerOffset, const size_t samplerSize, const BYTE *samplerData)
{
	// DONE: import as fbtexture and connect with dds video clip

	// video clip from file

	FBVideoClipImage *pNewClip = new FBVideoClipImage( "Video" );
	if (sampler_file != nullptr)
		pNewClip->Filename = sampler_file;

	if ( !pNewClip->IsValid() && sampler_file )
		pNewClip->Name = sampler_file;

	//
	//

	FBTexture *pTexture = new FBTexture ( (sampler_name) ? sampler_name : "Texture" );
	mTextures.push_back(pTexture);

	pTexture->Video = pNewClip;
	

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

void CMoBuVisitor::ResourceTexturesError(const char *what)
{
	FBMessageBox( "Reading Resource Cache ERROR", what, "Ok" );
}

void CMoBuVisitor::ResourceTexturesEnd()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}