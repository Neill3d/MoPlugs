
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_background.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <fbsdk/fbsdk.h>
#include <GL\glew.h>
#include "compositeMaster_object.h"
#include "compositeMaster_computeShaders.h"
#include "compositeMaster_common.h"

/////////////////////////////////////////////////////
//
class CompositeBackground
{
protected:
	// input values
	int					mInputWidth;
	int					mInputHeight;

	// output processing values
	int					mProcessingWidth;
	int					mProcessingHeight;

	//
	//
	GLint				mBackgroundTextureInternalFormat;
	GLint				mBackgroundTextureWidth;
	GLint				mBackgroundTextureHeight;
	GLuint				mBackgroundTextureId;

	struct RGBA
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;

		RGBA()
		{
			r=0;
			g=0;
			b=0;
			a=255;
		}
		RGBA(const FBColorAndAlpha &color)
		{
			r = (unsigned char)(255.0 * color[0]);
			g = (unsigned char)(255.0 * color[1]);
			b = (unsigned char)(255.0 * color[2]);
			a = (unsigned char)(255.0 * color[3]);
		}

		bool operator != (const RGBA &operatorB)
		{
			if (r!=operatorB.r || g!=operatorB.g || b!=operatorB.b || a != operatorB.a)
				return true;
			return false;
		}
	};

	struct StaticBackground
	{
		RGBA				lastColor;
		int					width;
		int					height;
		GLuint				textureId;

		StaticBackground()
		{
			width = 512;
			height = 512;
			textureId = 0;
		}
		~StaticBackground()
		{
			Free();
		}

		void Free()
		{
			if (textureId > 0)
			{
				glDeleteTextures(1, &textureId);
				textureId = 0;
			}
		}
	};
	StaticBackground		mStaticBackground;

	struct BackgroundProgramUniforms
	{
		// locations
		GLint		imageSize;

		GLint		backgroundColor;
		GLint		backgroundUpperColor;
		GLint		backgroundLowerColor;
	};

	int		mComputeLocalX;
	int		mComputeLocalY;

	CompositeComputeShader::CMixedProgram	*mBackgroundProgram;
	BackgroundProgramUniforms				mBackgroundProgramUniforms;

	bool				mNeedUpdate;
	bool				mNeedReload;

	bool				mForceToChangeSize;
	bool				mLastChangeSize;
public:

	FBPropertyBaseEnum<ERenderLayer>		*RenderLayer;
	FBPropertyBaseEnum<ECompositionInput>	*Input;

	FBPropertyInt							*BackgroundWidth;
	FBPropertyInt							*BackgroundHeight;

	FBPropertyAnimatableColor				*BackgroundColor;
	FBPropertyAnimatableDouble				*BackgroundAlpha;
	
	FBPropertyBool							*UseBackgroundGradient;
	FBPropertyAnimatableColor				*BackgroundUpperColor;
	FBPropertyAnimatableDouble				*BackgroundUpperAlpha;
	FBPropertyAnimatableColor				*BackgroundLowerColor;
	FBPropertyAnimatableDouble				*BackgroundLowerAlpha;
	
	FBPropertyListObject					*BackgroundTexture;
	FBPropertyInt							*BackgroundTextureWidth;
	FBPropertyInt							*BackgroundTextureHeight;
	//FBPropertyBool							*UseBackgroundTexture;
	//FBPropertyBool							*UseViewportRenderTexture;

	// possible to scaledown render image forexample to change output processing size

	FBPropertyBool							*OverrideSize;
	FBPropertyInt							*UserWidth;
	FBPropertyInt							*UserHeight;

	FBPropertyDouble						*ImageResizeFactor;							

	struct PropValues
	{
		ERenderLayer				layerId;
		ECompositionInput			input;

		bool						useGradient;
		//bool						useTexture;
		//bool						useViewportRenderTexture;

		FBColorAndAlpha				background;
		FBColorAndAlpha				backgroundUpper;
		FBColorAndAlpha				backgroundLower;

		PropValues()
		{
			input = eCompositionInputColor;
			useGradient = false;
			//useTexture = false;
			//useViewportRenderTexture = false;
		}
		PropValues(CompositeBackground *pComp)
		{
			GrabValues(pComp);
		}

		void GrabValues(CompositeBackground *pComp)
		{
			layerId = (ERenderLayer) pComp->RenderLayer->AsInt();
			input = (ECompositionInput) pComp->Input->AsInt();

			pComp->UseBackgroundGradient->GetData(&useGradient, sizeof(bool) );
			//pComp->UseBackgroundTexture->GetData(&useTexture, sizeof(bool) );
			//pComp->UseViewportRenderTexture->GetData(&useViewportRenderTexture, sizeof(bool) );

			pComp->BackgroundColor->GetData(background, sizeof(FBColor) );
			pComp->BackgroundAlpha->GetData(&background[3], sizeof(double) );
			background[3] *= 0.01;
			pComp->BackgroundUpperColor->GetData(backgroundUpper, sizeof(FBColor) );
			pComp->BackgroundUpperAlpha->GetData(&backgroundUpper[3], sizeof(double) );
			backgroundUpper[3] *= 0.01;
			pComp->BackgroundLowerColor->GetData(backgroundLower, sizeof(FBColorAndAlpha) );
			pComp->BackgroundLowerAlpha->GetData(&backgroundLower[3], sizeof(double) );
			backgroundLower[3] *= 0.01;
		}

		bool operator != (const PropValues &b)
		{
			if (layerId != b.layerId)
				return true;
			if (input != b.input)
				return true;
			if (useGradient != b.useGradient)
				return true;
			/*
			if (useTexture != b.useTexture)
				return true;
			if (useViewportRenderTexture != b.useViewportRenderTexture)
				return true;
				*/
			if (background != b.background)
				return true;
			if (backgroundUpper != b.backgroundUpper)
				return true;
			if (backgroundLower != b.backgroundLower)
				return true;

			return false;
		}
	};

	PropValues				mLastValues;

public:

	public:
	//! a constructor
	CompositeBackground();
	//! a destructor
	~CompositeBackground();

	void Init(FBComponent *pParent);

	void ChangeContext();
	
	void NeedUpdate();
	void NeedReload();
	void NeedToUpdateShaderDefines();

	bool IsShaderOk() const;

	void SetUserSize(const int w, const int h) {
		UserWidth->SetInt(w);
		UserHeight->SetInt(h);
	}

	const int GetProcessingWidth() const {
		return mProcessingWidth;
	}
	const int GetProcessingHeight() const {
		return mProcessingHeight;
	}

	bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0);
	bool PlugNotifyBackground(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug );

	// return result texture id of a background
	bool PrepRender(const CCompositionInfo *pInfo, 
		bool allowToReturnDirectTextureId,
		bool renderOutput,
		const float previewSizeFactor);

	const GLuint	ComputeTexture(	const CCompositionInfo *pInfo, 
									CompositeFinalStats &stats, 
									const GLuint bufferTextureId, 
									bool allowToReturnDirectTextureId );

	const GLuint GetTextureId() const { return mBackgroundTextureId; }
	const int GetTextureWidth() const { return mBackgroundTextureWidth; }
	const int GetTextureHeight() const { return mBackgroundTextureHeight; }

protected:

	void	PrepBackgroundProgramUniforms();
	void	UpdateBackgroundProgramUniforms(const int w, const int h);

	void	PrepStaticBackgroundTexture(const FBColorAndAlpha &color, const int w, const int h);

	void PrepBackgroundTextureData();
};