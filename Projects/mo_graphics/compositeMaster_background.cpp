
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_background.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "compositeMaster_background.h"
#include "graphics\CheckGLError_MOBU.h"

CompositeBackground::CompositeBackground()
{
	mBackgroundTextureId = 0;
	mBackgroundTextureWidth = mBackgroundTextureHeight = 1;
	mBackgroundTextureInternalFormat = GL_RGBA8;
	mBackgroundProgram = nullptr;
	//InitStaticBackgroundTextureData();

	mNeedUpdate = true;
	mNeedReload = false;

	mComputeLocalX = 32;
	mComputeLocalY = 32;
}

CompositeBackground::~CompositeBackground()
{
}

void CompositeBackground::Init(FBComponent *pParent)
{
	RenderLayer = (FBPropertyBaseEnum<ERenderLayer>*) pParent->PropertyCreate("Render Layer", kFBPT_enum, "enum", false, false );
	FBStringList *inputList = RenderLayer->GetEnumStringList(true);
	if (inputList)
	{
		inputList->SetString("Background~Main~Secondary");
	}

	Input = (FBPropertyBaseEnum<ECompositionInput>*) pParent->PropertyCreate("Input", kFBPT_enum, "enum", false, false );
	inputList = Input->GetEnumStringList(true);
	if (inputList)
	{
		inputList->SetString("Color~Texture~Render");
	}

	BackgroundWidth = (FBPropertyInt*) pParent->PropertyCreate("Background Width", kFBPT_int, "int", false, false);
	BackgroundHeight = (FBPropertyInt*) pParent->PropertyCreate("Background Height", kFBPT_int, "int", false, false);

	BackgroundColor = (FBPropertyAnimatableColor*) pParent->PropertyCreate("Background Color", kFBPT_ColorRGB, "Color", true, false);
	BackgroundAlpha = (FBPropertyAnimatableDouble*) pParent->PropertyCreate("Background Alpha", kFBPT_double, "double", true, false);

	UseBackgroundGradient = (FBPropertyBool*) pParent->PropertyCreate("Use Background Gradient", kFBPT_bool, "bool", false, false);
	
	BackgroundUpperColor = (FBPropertyAnimatableColor*) pParent->PropertyCreate("Background Upper Color", kFBPT_ColorRGB, "Color", true, false);
	BackgroundUpperAlpha = (FBPropertyAnimatableDouble*) pParent->PropertyCreate("Background Upper Alpha", kFBPT_double, "double", true, false);
	BackgroundLowerColor = (FBPropertyAnimatableColor*) pParent->PropertyCreate("Background Lower Color", kFBPT_ColorRGB, "Color", true, false);
	BackgroundLowerAlpha = (FBPropertyAnimatableDouble*) pParent->PropertyCreate("Background Lower Alpha", kFBPT_double, "double", true, false);

	//UseBackgroundTexture = (FBPropertyBool*) pParent->PropertyCreate("Use Background Texture", kFBPT_bool, "bool", false, false);
	BackgroundTexture = (FBPropertyListObject*) pParent->PropertyCreate("Background Texture", kFBPT_object, "object", false, false);
	BackgroundTextureWidth = (FBPropertyInt*) pParent->PropertyCreate("Back Texture Width", kFBPT_int, "int", false, false);
	BackgroundTextureHeight = (FBPropertyInt*) pParent->PropertyCreate("Back Texture Height", kFBPT_int, "int", false, false);
	//UseViewportRenderTexture = (FBPropertyBool*) pParent->PropertyCreate("Use Viewport Render", kFBPT_bool, "bool", false, false);

	OverrideSize = (FBPropertyBool*) pParent->PropertyCreate("Override Size", kFBPT_bool, "bool", false, false);
	UserWidth = (FBPropertyInt*) pParent->PropertyCreate("User Width", kFBPT_int, "int", false, false);
	UserHeight = (FBPropertyInt*) pParent->PropertyCreate("User Height", kFBPT_int, "int", false, false);

	ImageResizeFactor = (FBPropertyDouble*) pParent->PropertyCreate("ReSize Factor", kFBPT_double, "double", false, false);

	*RenderLayer = eRenderLayerMain;
	*Input = eCompositionInputColor;
	*BackgroundWidth = 512;
	*BackgroundHeight = 512;
	*BackgroundColor = FBColor(0.0, 0.0, 0.0);
	*BackgroundAlpha = 100.0;
	BackgroundAlpha->SetMinMax(0.0, 100.0, true, true);
	*BackgroundUpperColor = FBColor(1.0, 1.0, 1.0);
	*BackgroundUpperAlpha = 100.0;
	BackgroundUpperAlpha->SetMinMax(0.0, 100.0, true, true);
	*BackgroundLowerColor = FBColor(0.0, 0.0, 0.0);
	*BackgroundLowerAlpha = 100.0;
	BackgroundLowerAlpha->SetMinMax(0.0, 100.0, true, true);
	*UseBackgroundGradient = false;
	//*UseBackgroundTexture = false;
	//*UseViewportRenderTexture = false;
	BackgroundTexture->SetSingleConnect(true);
	BackgroundTexture->SetFilter(FBTexture::GetInternalClassId() );

	BackgroundTextureWidth->ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	BackgroundTextureHeight->ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	*OverrideSize = false;
	*UserWidth = 512;
	*UserHeight = 512;
	*ImageResizeFactor = 100.0;
	ImageResizeFactor->SetMinMax(1.0, 200.0);
}

bool CompositeBackground::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	// pThis == UseBackgroundTexture || pThis == UseViewportRenderTexture
	if (pAction == kFBCandidated && (pThis == UseBackgroundGradient || pThis == Input) )
	{
		mBackgroundProgram = nullptr;
		return true;
	}
	
	return false;
}

bool CompositeBackground::PlugNotifyBackground(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	if (pThis == BackgroundTexture)
	{
		if (pAction == kFBConnectedSrc)
		{
			mBackgroundTextureId = 0;
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			mBackgroundTextureId = 0;
		}
	}

	return true;
}

void CompositeBackground::PrepStaticBackgroundTexture(const FBColorAndAlpha &color, const int w, const int h)
{
	StaticBackground &data = mStaticBackground;

	RGBA backcolor(color);

	bool lneedupdate = false;
	bool isDifferent = false;

	if (data.textureId == 0)
	{
		glGenTextures(1, &data.textureId);
		lneedupdate = true;
	}

	if (w != data.width || h != data.height)
	{
		data.width = w;
		data.height = h;
		lneedupdate = true;
	}
	else
	{
		isDifferent = (backcolor != data.lastColor);
	}

	auto fn_createbuffer = [] (std::vector<RGBA> &buffer, const int width, const int height, const RGBA &color1)
	{
		const int imagesize = width * height;
		buffer.resize(imagesize);
		
		for (int i=0; i<imagesize; ++i)
		{
			buffer[i] = color1;
		}
	};

	if (lneedupdate)
	{
		std::vector<RGBA>	buffer;
		fn_createbuffer(buffer, data.width, data.height, backcolor);
		
		glBindTexture(GL_TEXTURE_2D, data.textureId);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
		glBindTexture(GL_TEXTURE_2D, 0);

	}
	else if (isDifferent)
	{
		std::vector<RGBA>	buffer;
		fn_createbuffer(buffer, data.width, data.height, backcolor);
		
		glBindTexture(GL_TEXTURE_2D, data.textureId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.width, data.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	data.lastColor = backcolor;
}

void CompositeBackground::PrepBackgroundProgramUniforms()
{
	BackgroundProgramUniforms &loc = mBackgroundProgramUniforms;

	loc.imageSize = glGetUniformLocation(mBackgroundProgram->GetProgramId(), "outputSize" );
	loc.backgroundColor = glGetUniformLocation(mBackgroundProgram->GetProgramId(), "backgroundColor" );
	loc.backgroundUpperColor = glGetUniformLocation(mBackgroundProgram->GetProgramId(), "backgroundUpperColor" );
	loc.backgroundLowerColor = glGetUniformLocation(mBackgroundProgram->GetProgramId(), "backgroundLowerColor" );
}

void CompositeBackground::UpdateBackgroundProgramUniforms(const int w, const int h)
{
	BackgroundProgramUniforms &loc = mBackgroundProgramUniforms;
	FBColorAndAlpha backcolor;

	if (loc.imageSize >= 0)
	{
		glUniform2f(loc.imageSize, 1.0f / (float)w, 1.0f / (float)h);
	}
	CHECK_GL_ERROR_MOBU();
	if (loc.backgroundColor >= 0)
	{
		BackgroundColor->GetData( backcolor, sizeof(FBColor) );
		BackgroundAlpha->GetData( &backcolor[3], sizeof(double) );
		backcolor[3] *= 0.01;
		glUniform4f(loc.backgroundColor, (float)backcolor[0], (float)backcolor[1], (float)backcolor[2], (float)backcolor[3] );
	}
	CHECK_GL_ERROR_MOBU();
	if (loc.backgroundUpperColor >= 0)
	{
		BackgroundUpperColor->GetData( backcolor, sizeof(FBColor) );
		BackgroundUpperAlpha->GetData( &backcolor[3], sizeof(double) );
		backcolor[3] *= 0.01;
		glUniform4f(loc.backgroundUpperColor, (float)backcolor[0], (float)backcolor[1], (float)backcolor[2], (float)backcolor[3] );
	}
	CHECK_GL_ERROR_MOBU();
	if (loc.backgroundLowerColor >= 0)
	{
		BackgroundLowerColor->GetData( backcolor, sizeof(FBColor) );
		BackgroundLowerAlpha->GetData( &backcolor[3], sizeof(double) );
		backcolor[3] *= 0.01;
		glUniform4f(loc.backgroundLowerColor, (float)backcolor[0], (float)backcolor[1], (float)backcolor[2], (float)backcolor[3] );
	}
	CHECK_GL_ERROR_MOBU();
}


bool CompositeBackground::PrepRender(const CCompositionInfo *pInfo, bool allowToReturnDirectTextureId, 
	bool renderOutput, const float previewSizeFactor)
{
	
	// TODO: compute processing width and height here
	/*
	if (false == allowToReturnDirectTextureId)
	{
		//mProcessingWidth = outputWidth;
		//mProcessingHeight = outputHeight;
		mProcessingWidth = BackgroundWidth->AsInt();
		mProcessingHeight = BackgroundHeight->AsInt();
	}
	else
	{
	*/

	if (renderOutput)
	{
		mInputWidth = pInfo->GetRenderWidth();
		mInputHeight = pInfo->GetRenderHeight();
	}
	else
	{
		switch(Input->AsInt() )
		{
		case eCompositionInputColor:
			mInputWidth = BackgroundWidth->AsInt();
			mInputHeight = BackgroundHeight->AsInt();
			break;
		case eCompositionInputRender:
			mInputWidth = pInfo->GetRenderWidth();
			mInputHeight = pInfo->GetRenderHeight();
			break;
		case eCompositionInputTexture:
			PrepBackgroundTextureData();
			mInputWidth = mBackgroundTextureWidth;
			mInputHeight = mBackgroundTextureHeight;
			break;
		}
	}
	//}

	mProcessingWidth = mInputWidth;
	mProcessingHeight = mInputHeight;

	mForceToChangeSize = false;

	if (OverrideSize->AsInt() > 0)
	{
		mProcessingWidth = UserWidth->AsInt();
		mProcessingHeight = UserHeight->AsInt();

		mForceToChangeSize = true;
	}

	double scalefactor=1.0;
	ImageResizeFactor->GetData(&scalefactor, sizeof(double) );

	scalefactor *= 0.01 * previewSizeFactor;

	if (scalefactor != 1.0)
	{
		mProcessingWidth = (int) (scalefactor * mProcessingWidth);
		mProcessingHeight = (int) (scalefactor * mProcessingHeight);

		mForceToChangeSize = true;
	}

	if (mForceToChangeSize != mLastChangeSize)
	{
		mBackgroundProgram = nullptr;
		mLastChangeSize = mForceToChangeSize;
	}

	if (true == allowToReturnDirectTextureId)
	{
		switch (Input->AsInt() )
		{
		case eCompositionInputRender:
			if (false == mForceToChangeSize)
				return true;
			break;

		case eCompositionInputColor:
			// we are using userwidth and user height here !
			if (UseBackgroundGradient->AsInt() == 0)
			{
				FBColorAndAlpha color(*BackgroundColor);
				BackgroundAlpha->GetData( &color[3], sizeof(double) );
				color[3] *= 0.01;
				
				PrepStaticBackgroundTexture(color, mProcessingWidth, mProcessingHeight);
				return true;
			}
			break;

		case eCompositionInputTexture:
			if (false == mForceToChangeSize)
			{
				GLuint srcBackgroundId = (Input->AsInt() == eCompositionInputTexture) ? mBackgroundTextureId : 0;
				return (srcBackgroundId > 0);
			}
			break;
		}
	}

	// check properties values with old one
	PropValues	currValues(this);
	if (currValues != mLastValues)
		mNeedUpdate = true;
	mLastValues = currValues;

	// always update when we are using a viewport render texture
	if (Input->AsInt() == eCompositionInputTexture)
		mNeedUpdate = true;

	//
	//
	bool shaderOk = true;

	if (mNeedReload && nullptr != mBackgroundProgram)
	{
		CompositeComputeShader::CMixedProgramManager::instance().ReloadProgram(mBackgroundProgram);
		PrepBackgroundProgramUniforms();
		mNeedReload = false;
		shaderOk = mBackgroundProgram->IsOk();
	}

	//const int w = mProcessingWidth;
//	const int h = mProcessingHeight;
	
	if (nullptr == mBackgroundProgram)
	{
		FBString strHeader( "#version 430\n"
			"#extension GL_ARB_shading_language_include : require\n"
			"layout (local_size_x =32, local_size_y = 32) in;\n" );

		FBString strDefine( "" );
		if ( (Input->AsInt() == eCompositionInputRender) 
			|| (Input->AsInt() == eCompositionInputTexture && BackgroundTexture->GetCount() > 0))
		{
			strDefine = strDefine + "#define USE_TEXTURE\n";

			if (false == mForceToChangeSize)
				strDefine = strDefine + "#define SAME_SIZE\n";
		}		
		else if (UseBackgroundGradient->AsInt() > 0)
		{
			strDefine = strDefine + "#define useGradient\n";
		}

		mBackgroundProgram = CompositeComputeShader::CMixedProgramManager::instance().QueryAProgramMix("fillcolor", strHeader, strDefine, "\\GLSL_CS\\fillcolor.cs", "");
		CompositeComputeShader::CMixedProgramManager::instance().GetGroupLocalSize(mComputeLocalX, mComputeLocalY);

		mNeedUpdate = true;
		mNeedReload = false;

		shaderOk = mBackgroundProgram->IsOk();

		if (shaderOk)
		{
			// DONE: update uniform locations
			PrepBackgroundProgramUniforms();
		}
	}

	return shaderOk;
}

const GLuint CompositeBackground::ComputeTexture(	const CCompositionInfo *pInfo, 
													CompositeFinalStats &stats, 
													const GLuint bufferTextureId, 
													bool allowToReturnDirectTextureId )
{
	ERenderLayer layerId = (ERenderLayer) RenderLayer->AsInt();
	const MainFrameBuffer *pFrameBuffer = pInfo->GetFrameBufferInfo(layerId);

	if (true == allowToReturnDirectTextureId)
	{
		switch (Input->AsInt() )
		{
		case eCompositionInputRender:
			if (false == mForceToChangeSize)
				return pFrameBuffer->GetFinalColorObject(); // pInfo->GetRenderColorId();
			break;

		case eCompositionInputColor:
			// we are using userwidth and user height here !
			if (UseBackgroundGradient->AsInt() == 0)
			{
				return mStaticBackground.textureId;
			}
			break;

		case eCompositionInputTexture:
			if (false == mForceToChangeSize)
			{
				GLuint srcBackgroundId = (Input->AsInt() == eCompositionInputTexture) ? mBackgroundTextureId : 0;
				return srcBackgroundId;
			}
			break;
		}
	}

	if (bufferTextureId == 0)
		return 0;

	GLuint srcBackgroundId = 0;
	
	switch (Input->AsInt() )
	{
	case eCompositionInputRender:
		srcBackgroundId = pFrameBuffer->GetFinalColorObject(); // pInfo->GetRenderColorId();
		break;
	case eCompositionInputTexture:
		srcBackgroundId = mBackgroundTextureId;
		break;
	}

	if (nullptr == mBackgroundProgram || false == mBackgroundProgram->IsOk())
		return bufferTextureId;

	const int w = mProcessingWidth;
	const int h = mProcessingHeight;

	mBackgroundProgram->Bind();

	// update uniform values
	UpdateBackgroundProgramUniforms(w, h);
	CHECK_GL_ERROR_MOBU();

	glBindImageTexture(0, bufferTextureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	
	if (srcBackgroundId > 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, srcBackgroundId);
		glActiveTexture(GL_TEXTURE0);
	}

	const int groups_x = w / mComputeLocalX + 1;
	const int groups_y = h / mComputeLocalY + 1;

	// DONE: compute the number of groups !!
	mBackgroundProgram->DispatchPipeline(groups_x, groups_y, 1);
	stats.CountComputeShader(1, groups_x*groups_y );

	mBackgroundProgram->UnBind();

	mNeedUpdate = false;
	return bufferTextureId;
}


void CompositeBackground::PrepBackgroundTextureData()
{
	
	if (mBackgroundTextureId == 0 && BackgroundTexture->GetCount() > 0)
	{
		FBTexture *pTexture = (FBTexture*) BackgroundTexture->GetAt(0);
		glActiveTexture(GL_TEXTURE0);
		pTexture->OGLInit();
		mBackgroundTextureId = pTexture->TextureOGLId;

		mBackgroundTextureInternalFormat = GL_RGBA8;
		if (mBackgroundTextureId > 0)
		{
			glBindTexture(GL_TEXTURE_2D, mBackgroundTextureId);
				
			GLint red, green, blue, alpha;

			glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &mBackgroundTextureWidth );
			glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &mBackgroundTextureHeight );

			glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &red);
			glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &green );
			glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &blue );
			glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &alpha );

			mBackgroundTextureInternalFormat = (alpha>0) ? GL_RGBA : GL_RGB;

			//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &mBackgroundTextureInternalFormat);

			glBindTexture(GL_TEXTURE_2D, 0);

			BackgroundTextureWidth->SetInt(mBackgroundTextureWidth);
			BackgroundTextureHeight->SetInt(mBackgroundTextureHeight);
		}
		else
		{
			// failed to use texture
			Input->SetInt( (int) eCompositionInputColor );
		}
	}
	
}


void CompositeBackground::ChangeContext()
{
	mBackgroundProgram = nullptr;
	mNeedUpdate = true;
}

void CompositeBackground::NeedToUpdateShaderDefines()
{
	mBackgroundProgram = nullptr;
}

void CompositeBackground::NeedReload()
{
	mNeedReload = true;
}

void CompositeBackground::NeedUpdate()
{
	mNeedUpdate = true;
}

bool CompositeBackground::IsShaderOk() const
{
	return (mBackgroundProgram && mBackgroundProgram->IsOk() );
}