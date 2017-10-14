
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: OGL_Utils_MOBU.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "graphics\OGL_Utils_MOBU.h"
#include <exception>

//////////////////////////////////////////////////////////////////////////////////////////////////
//


void BindTextureToSlot(FBTexture *pTexture, FBRenderOptions* pRenderOptions, const int slot)
{
	if (pTexture )
	{
		//pTexture->OGLInit(pRenderOptions);
		const GLint id = pTexture->GetTextureObject();

		if (id > 0)
		{
			glActiveTexture(GL_TEXTURE0+slot);
			glBindTexture(GL_TEXTURE_2D, id);
		}
	}
}

const int GetTextureId(FBTexture *pTexture, FBRenderOptions *pOptions, bool forceInit)
{
	int resId = 0;
	if (pTexture)
	{
		FBVideo *pVideo = pTexture->Video;
		if (pVideo && FBIS(pVideo, FBVideoMemory) )
		{
			resId = ( (FBVideoMemory*) pVideo)->TextureOGLId;
		}
		else
		{
			/*
			if (false == forceInit)
				resId = pTexture->GetTextureObject();
				*/
			if (resId == 0)
			{
				pTexture->OGLInit(pOptions);
				resId = pTexture->GetTextureObject();
			}
		}
	}

	return resId;
}


//////////////////////////////////////////////////////////////////
//

bool BindVertexAttribFromFBModel(FBModel *pModel)
{
	FBModelVertexData *pVertexData = pModel->ModelVertexData;
	if (pVertexData == nullptr || pVertexData->IsDrawable() == false)
		return false;

	const unsigned int positionId = pVertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Point);
	const unsigned int normalId = pVertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Normal );
	const unsigned int uvId = pVertexData->GetUVSetVBOId();

	const GLvoid* positionOffset = pVertexData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Point);
	const GLvoid* normalOffset = pVertexData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Normal);
	const GLvoid* uvOffset = pVertexData->GetUVSetVBOOffset();

	if (positionId == 0 || normalId == 0 || uvId == 0)
	{
		return false;
	}
	
	glBindBuffer( GL_ARRAY_BUFFER, positionId );
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, positionOffset );
	glBindBuffer( GL_ARRAY_BUFFER, uvId );
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, uvOffset ); 

	glBindBuffer( GL_ARRAY_BUFFER, normalId );
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, normalOffset ); 

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//

int OGLCubeMapMOBU::VideoFormatComponentsCount( FBVideoFormat format )
{
	if (format==kFBVideoFormat_ABGR_32 || format==kFBVideoFormat_ARGB_32 || format == kFBVideoFormat_BGRA_32 || format == kFBVideoFormat_RGBA_32 )
		return 4;

	return 3;
}

bool OGLCubeMapMOBU::load_cube_map_side ( const GLuint texture, const GLenum side_target, FBVideoClip *pVideoClip ) 
{

  FBString clipname(pVideoClip->Name);

  unsigned char*  image_data = pVideoClip->GetImage();
  if (!image_data) {
    fprintf (stderr, "ERROR: could not load %s\n", clipname);
    return false;
  }

  const int width = pVideoClip->Width;
  const int height = pVideoClip->Height;
  int components = VideoFormatComponentsCount(pVideoClip->Format);
  int glFormat = (components==4) ? GL_RGBA : GL_RGB;
  int glInternalFormat = (components==4) ? GL_RGBA8 : GL_RGB8;

  // non-power-of-2 dimensions check
  if ((width & (width - 1)) != 0 || (height & (height - 1)) != 0) {
    fprintf (
      stderr, "WARNING: image %s is not power-of-2 dimensions\n", clipname
    );
  }
  
  // copy image data into 'target' side of cube map
  glTexImage2D (
    side_target,
    0,
    glInternalFormat,
    width,
    height,
    0,
    glFormat,
    GL_UNSIGNED_BYTE,
    image_data
  );

  return true;
}


bool OGLCubeMapMOBU::createCubeMap( FBVideoClip *front, FBVideoClip *back, FBVideoClip *top, FBVideoClip *bottom, FBVideoClip *left, FBVideoClip *right )
{
	// generate a cube-map texture to hold all the sides
	glActiveTexture (GL_TEXTURE0);
	if (cubeId == 0)
	{
		glGenTextures (1, &cubeId);
	}

	glBindTexture (GL_TEXTURE_CUBE_MAP, cubeId);

	// load each image and copy into a side of the cube-map texture
	try
	{
		if (!load_cube_map_side (cubeId, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front))
			throw std::exception( "failed to load cubemap side" );
		if (!load_cube_map_side (cubeId, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back))
			throw std::exception( "failed to load cubemap side" );
		if (!load_cube_map_side (cubeId, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, bottom))
			throw std::exception( "failed to load cubemap side" );

		if (!load_cube_map_side (cubeId, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, top))
			throw std::exception( "failed to load cubemap side" );
		if (!load_cube_map_side (cubeId, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left))
			throw std::exception( "failed to load cubemap side" );
		if (!load_cube_map_side (cubeId, GL_TEXTURE_CUBE_MAP_POSITIVE_X, right))
			throw std::exception( "failed to load cubemap side" );
	}
	catch (const std::exception &e)
	{
		printf ("%s\n", e.what() );
		return false;
	}

	// format cube map texture
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return true;
}