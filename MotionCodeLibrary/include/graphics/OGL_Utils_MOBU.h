
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: OGL_Utils_MOBU.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//-- 
#include <GL\glew.h>

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "graphics\ogl_utils.h"

//////////////////////////////////////////////////////////////////////////////////////

void BindTextureToSlot(FBTexture *pTexture, FBRenderOptions* pRenderOptions, const int slot);
const int GetTextureId(FBTexture *pTexture, FBRenderOptions *pOptions, bool forceInit);


////////////////

bool BindVertexAttribFromFBModel(FBModel *pModel);


//////////////////

class OGLCubeMapMOBU : public OGLCubeMap
{
public:

	bool createCubeMap( FBVideoClip *front, FBVideoClip *back, FBVideoClip *top, FBVideoClip *bottom, FBVideoClip *left, FBVideoClip *right );

protected:

	int VideoFormatComponentsCount( FBVideoFormat format );
	bool load_cube_map_side ( const GLuint texture, const GLenum side_target, FBVideoClip *pVideoClip );

};