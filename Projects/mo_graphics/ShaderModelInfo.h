
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ShaderModelInfo.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////
// Organize object data and second uv and store this information in shaderModelInfo struct
//

//-- 
#include <GL\glew.h>

//--- SDK include
#include <fbsdk/fbsdk.h>

// mesh conveyer
//
#include "shared_models.h"

////////////////////////////////////////////////////////////////////
// per model information for storing second lightmap uvs (if needed)

const unsigned int	kFBGeometryArrayID_SecondUVSet          =  1 << 5;     //!< ID to the Second UVSet Array

class SuperShaderModelInfo : public FBShaderModelInfo
{
public:

	//! a constructor
	SuperShaderModelInfo(FBShader* pShader, HKModelRenderInfo pInfo, int pSubRegionIndex);

	//! a destructor
	virtual ~SuperShaderModelInfo();

	//! To be overloaded, always be called when Model or Shader version out of date.
	virtual void UpdateModelShaderInfo(int pShader_Version) override;

	// VAO
	void	Bind();
	void	UnBind();

	// only uv buffer
	void	BindUVBuffer(const GLuint locationId);
	void	UnBindUVBuffer();

	const int GetCachedMeshIndex() const {
		return mMeshIndex;
	}

protected:

	FBShader			*mShader;

	// buffer for support 2 sets of UVs
	int					mBufferCount;
	GLuint				mBufferId;
	GLuint				mLocationId;
	// VAO for faster attributes binding
	CGPUVertexData		*mVertexData;		

	// cached mesh index in the CGPUFBScene
	int					mMeshIndex;

	// UV sets

	bool			IsSecondUVSetNeeded();
	bool			PrepareUVSets();	// make a buffer with two uvsets inside
	bool			VertexDataFromFBModel(bool processTangentBuffer);
};