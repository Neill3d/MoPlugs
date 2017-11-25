
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: bakeProjectors_viewData.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


// GLSLFX shader
#include "ShaderFX.h"
#include "graphics\Framebuffer.h"
#include "graphics\UniformBuffer.h"

#include "..\Common_Projectors\bakeProjectors_projectors.h"

////////////////////////////////////////////////////////////////////////////
//
class ViewBakeProjectorsData
{
public:

	HGLRC		mLastContext;

	std::vector<FrameBuffer*>	mFrameBuffers;

	FrameBufferInfo				mFramebufferInfo;

	bool						mReadyToLoad;
	Graphics::ShaderEffect		*mUberShader;

	nvFX::IUniform				*mOnlyProjectors;

	int							mLocTexture;
	int							mLocMaterial;
	int							mLocShader;
	int							mLocProjectors;

	CGPUBufferNV				mBufferTexture;
	CGPUBufferNV				mBufferMaterial;
	CGPUBufferNV				mBufferShader;
	
	CProjectors					mProjectors;

	FBSystem					mSystem;

	void	RenderModel(FBModel *pModel);

	void	UploadShader(FBShader *pShader);
	void	UploadMaterial(FBMaterial *pMaterial);

	void PrepProjectors();

	void ReSize(const int width, const int height);

	void	ClearFrameBuffers();

	void RenderToFramebuffers(bool &grabImage, FBString &grabImageName, const bool saveJpeg, const int jpegQuality, const bool grabWithFrameNumber, 
		const bool saveImagePerModel, const bool saveOnlyProjectors, FBCamera *pcamera, const FBColorAndAlpha &backcolor, bool &saveStatus);

	void ChangeContext();

	void	SavePixelsToFile(const bool saveJpeg, const int jpegQuality, const int w, const int h, const char *imagename);

	void	AddFrameNumber(FBString &str, bool simpleCat);


public:

	//! a constructor
	ViewBakeProjectorsData();
	//! a destructor
	~ViewBakeProjectorsData();


		// models to be baked
	
	struct SpreadModel
	{
		FBModel		*pModel;

		int			preset;
		int			width;
		int			height;
	};

	int								mCurrentRow;
	std::vector<SpreadModel>		mModels;

	void ClearModels()
	{
		mCurrentRow = -1;
		mModels.clear();
	}

	void AddModel(FBModel *pModel, int _preset, int _width, int _height)
	{
		SpreadModel modelData;

		modelData.pModel = pModel;
		modelData.preset = _preset;
		modelData.width = _width;
		modelData.height = _height;

		mModels.push_back( modelData );

		if (mCurrentRow == -1)
			mCurrentRow = 0;
	}

	void SetCurrentRow(const int row)
	{
		mCurrentRow = row;
	}
	const int GetCurrentRow()
	{
		return mCurrentRow;
	}

	const int GetNumberOfFrameBuffers()
	{
		return (int) mFrameBuffers.size();
	}

	FrameBuffer *GetFrameBufferPtr(const int index)
	{
		return mFrameBuffers[index];
	}

	const GLuint GetFrameBufferColorId(const int index)
	{
		return mFrameBuffers[index]->GetColorObject();
	}

};