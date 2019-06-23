
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectFilters.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_objectFilters.h"

FBClassImplementation2(ObjectFilterColorCorrection)
FBUserObjectImplement(ObjectFilterColorCorrection, "Composition Color Correction", EFFECT_ICON);						//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterColorCorrection, FILTERCSB__ASSETNAME, EFFECT_ICON);					//Register to the asset system

FBClassImplementation2(ObjectFilterBlur)
FBUserObjectImplement(ObjectFilterBlur, "Composition Blur", EFFECT_ICON);				//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterBlur, FILTERBLUR__ASSETNAME, EFFECT_ICON);				//Register to the asset system

FBClassImplementation2(ObjectFilterHalfTone)
FBUserObjectImplement(ObjectFilterHalfTone, "Composition HalfTone", EFFECT_ICON);		//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterHalfTone, FILTERHALFTONE__ASSETNAME, EFFECT_ICON);		//Register to the asset system

FBClassImplementation2(ObjectFilterPosterization)
FBUserObjectImplement(ObjectFilterPosterization, "Composition Posterization", EFFECT_ICON);			//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterPosterization, FILTERPOSTERIZATION__ASSETNAME, EFFECT_ICON);		//Register to the asset system

FBClassImplementation2(ObjectFilterChangeColor)
FBUserObjectImplement(ObjectFilterChangeColor, "Composition Change Color", EFFECT_ICON);			//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterChangeColor, FILTERCHANGECOLOR__ASSETNAME, EFFECT_ICON);		//Register to the asset system

FBClassImplementation2(ObjectFilterFilmGrain)
FBUserObjectImplement(ObjectFilterFilmGrain, "Composition Film Grain", EFFECT_ICON);			//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterFilmGrain, FILTERFILMGRAIN__ASSETNAME, EFFECT_ICON);		//Register to the asset system

FBClassImplementation2(ObjectFilterCrosshatch)
FBUserObjectImplement(ObjectFilterCrosshatch, "Composition Crosshatch", EFFECT_ICON);			//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterCrosshatch, FILTERCROSSHATCH__ASSETNAME, EFFECT_ICON);		//Register to the asset system

FBClassImplementation2(ObjectFilterCrossStitching)
FBUserObjectImplement(ObjectFilterCrossStitching, "Composition Cross Stitching", EFFECT_ICON);			//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterCrossStitching, FILTERCROSSSTITCH__ASSETNAME, EFFECT_ICON);		//Register to the asset system

FBClassImplementation2(ObjectFilterToonLines)
FBUserObjectImplement(ObjectFilterToonLines, "Composition Toon Lines", EFFECT_ICON);					//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterToonLines, FILTERTOONLINES__ASSETNAME, EFFECT_ICON);			//Register to the asset system

FBClassImplementation2(ObjectFilterSSAO)
FBUserObjectImplement(ObjectFilterSSAO, "Composition SSAO", EFFECT_ICON);					//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterSSAO, FILTERSSAO__ASSETNAME, EFFECT_ICON);			//Register to the asset system

///////////////////////////////////////////////////////////////////////////////////////////////////
// Filter Color Correction

void AddPropertyViewForColor(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTERCSB__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilterColorCorrection::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForColor( "Active", "" );
	AddPropertyViewForColor( "Reload Shader", "" );
	AddPropertyViewForColor( "Opacity", "" );

	AddPropertyViewForColor( "", "Masking", true );
	AddPropertyViewForColor( "Use Composite Mask", "Masking" );
	AddPropertyViewForColor( "Select Composite Mask", "Masking" );
	AddPropertyViewForColor( "Invert Composite Mask", "Masking" );
	AddPropertyViewForColor( "Custom Mask", "Masking" );

	AddPropertyViewForColor( "", "Info", true );
	AddPropertyViewForColor( "Out Width", "Info" );
	AddPropertyViewForColor( "Out Height", "Info" );
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectFilterColorCorrection::ObjectFilterColorCorrection( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new ProgramUniformsColor() )
{
	FBClassInit;

	FBPropertyPublish(this, Contrast, "Contrast", nullptr, nullptr);
	FBPropertyPublish(this, Saturation, "Saturation", nullptr, nullptr);
	FBPropertyPublish(this, Brightness, "Brightness", nullptr, nullptr);

	FBPropertyPublish(this, Gamma, "Gamma", nullptr, nullptr);

	FBPropertyPublish(this, Inverse, "Inverse", nullptr, nullptr);

	FBPropertyPublish(this, Hue, "Hue", nullptr, nullptr);
	FBPropertyPublish(this, HueSaturation, "Hue Saturation", nullptr, nullptr);
	FBPropertyPublish(this, Lightness, "Lightness", nullptr, nullptr);

	Contrast = 0.0;
	Contrast.SetMinMax(-100.0, 100.0, true, true);
	Saturation = 0.0;
	Saturation.SetMinMax(-100.0, 100.0, true, true);
	Brightness = 0.0;
	Brightness.SetMinMax(-100.0, 100.0, true, true);

	Gamma = 100.0;
	Gamma.SetMinMax( -200.0, 300.0, true, true );

	Inverse = false;

	Hue = 0.0;
	Hue.SetMinMax(-100.0, 100.0, true, true);
	HueSaturation = 0.0;
	HueSaturation.SetMinMax(-100.0, 100.0, true, true);
	Lightness = 0.0;
	Lightness.SetMinMax(-100.0, 100.0, true, true);

	mProgramUniformsColor = (ProgramUniformsColor*) mProgramUniforms.get();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Blur system

void ShaderBlurSystem::Create(CacheData &cacheData)
{
	cacheData.temp_w = 0;
	cacheData.temp_h = 0;
	cacheData.temp_color_id = 0;
	cacheData.program = nullptr;
	cacheData.uniforms = new ProgramUniformsBlur();
}
void ShaderBlurSystem::Destroy(CacheData &cacheData)
{
	if (cacheData.uniforms)
	{
		delete cacheData.uniforms;
		cacheData.uniforms = nullptr;
	}

	if (cacheData.temp_color_id > 0)
	{
		FreeIntermidTexture(cacheData);
		cacheData.temp_color_id = 0;
	}
}

void ShaderBlurSystem::ChangeContext(CacheData &cacheData)
{
	if (cacheData.program)
	{
		cacheData.program = nullptr;
	}
	if (cacheData.temp_color_id > 0)
	{
		FreeIntermidTexture(cacheData);
		cacheData.temp_color_id = 0;
	}
}

bool ShaderBlurSystem::CheckIfShaderIsReady(const CCompositionInfo *pInfo, const Data &data, CacheData &cacheData)
{
	if (data.needShaderReload && nullptr != cacheData.program)
	{
		CompositeComputeShader::CMixedProgramManager::instance().ReloadProgram(cacheData.program);
		//mNeedProgramReload = false;

		bool shaderOk = cacheData.program->IsOk();
		if (false == shaderOk)
		{
			return false;
		}

		cacheData.uniforms->Prep( cacheData.program->GetProgramId() );
	}

	const bool useMask = data.compositeMaskId > 0;
	if (nullptr == cacheData.program)
	{
		FBString strHeader( "#version 430\n"
			"#extension GL_ARB_shading_language_include : require\n"
			"layout (local_size_x =32, local_size_y = 32) in;\n" );

		FBString strDefine( "" );

		// TODO: check when to apply same size difinition
		//strDefine = strDefine + "#define SAME_SIZE\n";

		if (useMask)
			strDefine = strDefine + "#define USE_MASK\n";

		if ( data.hasMultisamplingSupport )
		{
			strDefine = strDefine + ((CCompositionInfo*) pInfo)->GetRenderSamplesDefineString();
		}
		strDefine = strDefine + pInfo->GetDepthBufferDefineString();

		const char *programLabel = MixedProgramLabel();
		const char *programPath = MixedProgramPath();

		cacheData.program = CompositeComputeShader::CMixedProgramManager::instance().QueryAProgramMix(programLabel, strHeader, strDefine, programPath, "");
		
		bool shaderOk = cacheData.program->IsOk();
		if (false == shaderOk)
		{
			return false;
		}

		// DONE: update uniform locations
		cacheData.uniforms->Prep( cacheData.program->GetProgramId() );
	}

	if (nullptr == cacheData.program)
		return false;

	return true;
}
bool ShaderBlurSystem::ApplyFilter(const CCompositionInfo *pInfo, const Data &data, CompositeFinalStats &stats, CacheData &cacheData)
{
	if (nullptr == cacheData.program || nullptr == cacheData.uniforms)
		return false;

	const int computeLocalX = 32;
	const int computeLocalY = 32;

	const int w = data.w;
	const int h = data.h;

	//const bool useMask = (data.compositeMaskId > 0);

	const double horz = data.horzFactor;
	double vert = data.vertFactor;

	if (horz == 0.0 && vert == 0.0)
		return false;

	const float fhorz = 0.0001f * horz;
	const float fvert = 0.0001f * vert;

	const int groups_x = w / computeLocalX + 1;
	const int groups_y = h / computeLocalY + 1;

	cacheData.program->Bind();

	// update uniform values
	CProcessingInfo	prInfo(w, h);
	cacheData.uniforms->Upload(pInfo, prInfo, 
		w, h,
		data.invertCompositeMask, data.opacity);
	/*
	if (useMask)
	{
		SetCompositeMaskTextureId( pInfo->GetRenderMaskId() );
		BindCompositeMask();
	}
	*/
	//
	//
	
	if (horz == 0.0 || vert == 0.0)
	{
		// apply only one pass - horz or vert
	
		cacheData.uniforms->UploadScale(fhorz, fvert);
				
		glBindImageTexture(0, data.dstTexId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	
		if (data.sourceTexId > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, data.sourceTexId);
			glActiveTexture(GL_TEXTURE0);
		}

		// DONE: compute the number of groups !!
		cacheData.program->DispatchPipeline(groups_x, groups_y, 1);

		//
		stats.CountComputeShader(1, groups_x*groups_y );
	}
	else
	{
		ReSizeIntermidTexture(w, h, cacheData);

		// HORZ pass

		cacheData.uniforms->UploadScale(fhorz, 0.0f);
		
		glBindImageTexture(0, cacheData.temp_color_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	
		if (data.sourceTexId > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, data.sourceTexId);
			glActiveTexture(GL_TEXTURE0);
		}

		// DONE: compute the number of groups !!
		cacheData.program->DispatchPipeline(groups_x, groups_y, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		//  VERT pass
		cacheData.uniforms->UploadScale(0.0f, fvert);
		
		glBindImageTexture(0, data.dstTexId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, cacheData.temp_color_id);
		glActiveTexture(GL_TEXTURE0);
		//glBindImageTexture(1, temp_color_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
		
		// DONE: compute the number of groups !!
		cacheData.program->DispatchPipeline(groups_x, groups_y, 1);

		//
		stats.CountComputeShader(2, 2*groups_x*groups_y );
	}

	cacheData.program->UnBind();
	/*
	if (useMask)
	{
		UnBindCompositeMask();
	}
	*/
	return true;
}

void ShaderBlurSystem::ReSizeIntermidTexture(const int w, const int h, CacheData &cacheData)
{
	if (cacheData.temp_color_id == 0)
		glGenTextures(1, &cacheData.temp_color_id);

	if (w != cacheData.temp_w || h != cacheData.temp_h)
	{
		glBindTexture(GL_TEXTURE_2D, cacheData.temp_color_id);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );

		glBindTexture(GL_TEXTURE_2D, 0);

		cacheData.temp_w = w;
		cacheData.temp_h = h;
	}
}

void ShaderBlurSystem::FreeIntermidTexture(CacheData &cacheData)
{
	if (cacheData.temp_color_id > 0)
	{
		glDeleteTextures(1, &cacheData.temp_color_id);
		cacheData.temp_color_id = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Filter Blur

void AddPropertyViewForBlur(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTERBLUR__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilterBlur::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForBlur( "Active", "" );
	AddPropertyViewForBlur( "Reload Shader", "" );
	AddPropertyViewForBlur( "Opacity", "" );

	AddPropertyViewForBlur( "", "Masking", true );
	AddPropertyViewForBlur( "Use Composite Mask", "Masking" );
	AddPropertyViewForBlur( "Select Composite Mask", "Masking" );
	AddPropertyViewForBlur( "Invert Composite Mask", "Masking" );
	AddPropertyViewForBlur( "Custom Mask", "Masking" );

	AddPropertyViewForBlur( "", "Info", true );
	AddPropertyViewForBlur( "Out Width", "Info" );
	AddPropertyViewForBlur( "Out Height", "Info" );
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectFilterBlur::ObjectFilterBlur( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, nullptr )
{
	FBClassInit;

	FBPropertyPublish(this, HorzBlurFactor, "Blur Horz", nullptr, nullptr);
	FBPropertyPublish(this, UseAspectRatio, "Use Aspect Ratio", nullptr, nullptr);
	FBPropertyPublish(this, UseSeparateVertFactor, "Use Separate Vert Factor", nullptr, nullptr);
	FBPropertyPublish(this, VertBlurFactor, "Blur Vert", nullptr, nullptr);

	HorzBlurFactor = 0.0;
	HorzBlurFactor.SetMinMax(0.0, 1000.0, true, true); 
	VertBlurFactor = 0.0;
	VertBlurFactor.SetMinMax(0.0, 1000.0, true, true); 

	UseAspectRatio = true;
	UseSeparateVertFactor = false;

	ShaderBlurSystem::Create(mCacheData);
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/

void ObjectFilterBlur::FBDestroy()
{
	ShaderBlurSystem::Destroy(mCacheData);
	ParentClass::FBDestroy();
}

void ObjectFilterBlur::PrepData()
{
	mData.compositeMaskId = 0;
	mData.hasMultisamplingSupport = false;

	mData.opacity = 0.01f * (float) Opacity;

	const double horz = HorzBlurFactor;
	double vert = (UseSeparateVertFactor) ? VertBlurFactor : horz;
	if (UseAspectRatio)
		vert = vert * mData.h / mData.w;

	mData.horzFactor = horz;
	mData.vertFactor = vert;
}

bool ObjectFilterBlur::ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo)
{

	if (Active == false)
		return false;

	if (Opacity == 0.0)
		return false;

	mData.w = prInfo.GetWidth();
	mData.h = prInfo.GetHeight();

	PrepData();
	
	if (mData.horzFactor == 0.0 && mData.vertFactor == 0.0)
		return false;

	if (false == ShaderBlurSystem::CheckIfShaderIsReady(pInfo, mData, mCacheData) )
		return false;

	return true;
}

bool ObjectFilterBlur::ApplyFilter(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId)
{

	mData.sourceTexId = sourceTexId;
	mData.dstTexId = dstTexId;

	if (false == ShaderBlurSystem::ApplyFilter(pInfo, mData, stats, mCacheData) )
		return false;

	mProcessingWidth = mData.w;
	mProcessingHeight = mData.h;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Half tone Filter

void AddPropertyViewForHalfTone(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTERHALFTONE__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilterHalfTone::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForHalfTone( "Active", "" );
	AddPropertyViewForHalfTone( "Reload Shader", "" );
	AddPropertyViewForHalfTone( "Opacity", "" );

	AddPropertyViewForHalfTone( "", "Masking", true );
	AddPropertyViewForHalfTone( "Use Composite Mask", "Masking" );
	AddPropertyViewForHalfTone( "Select Composite Mask", "Masking" );
	AddPropertyViewForHalfTone( "Invert Composite Mask", "Masking" );
	AddPropertyViewForHalfTone( "Custom Mask", "Masking" );

	AddPropertyViewForHalfTone( "", "Info", true );
	AddPropertyViewForHalfTone( "Out Width", "Info" );
	AddPropertyViewForHalfTone( "Out Height", "Info" );
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectFilterHalfTone::ObjectFilterHalfTone( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new ProgramUniformsHalfTone() )
{
	FBClassInit;

	FBPropertyPublish(this, Frequency, "Frequency", nullptr, nullptr);

	Frequency = 40.0;
	Frequency.SetMinMax(0.0, 300.0, true, true);

	mProgramUniformsHalfTone = (ProgramUniformsHalfTone*) mProgramUniforms.get();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Posterization Filter

void AddPropertyViewForPosterization(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTERPOSTERIZATION__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilterPosterization::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForPosterization( "Active", "" );
	AddPropertyViewForPosterization( "Reload Shader", "" );
	AddPropertyViewForPosterization( "Opacity", "" );

	AddPropertyViewForPosterization( "", "Masking", true );
	AddPropertyViewForPosterization( "Use Composite Mask", "Masking" );
	AddPropertyViewForPosterization( "Select Composite Mask", "Masking" );
	AddPropertyViewForPosterization( "Invert Composite Mask", "Masking" );
	AddPropertyViewForPosterization( "Custom Mask", "Masking" );

	AddPropertyViewForPosterization( "", "Info", true );
	AddPropertyViewForPosterization( "Out Width", "Info" );
	AddPropertyViewForPosterization( "Out Height", "Info" );
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectFilterPosterization::ObjectFilterPosterization( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new ProgramUniformsPosterization() )
{
	FBClassInit;

	FBPropertyPublish(this, NumberOfColors, "Number Of Colors", nullptr, nullptr);
	FBPropertyPublish(this, Gamma, "Gamma", nullptr, nullptr);

	NumberOfColors = 80.0;
	NumberOfColors.SetMinMax(0.0, 100.0);
	Gamma = 60.0;
	Gamma.SetMinMax(0.0, 100.0);

	mProgramUniformsPosterization = (ProgramUniformsPosterization*) mProgramUniforms.get();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Change Color Filter

void AddPropertyViewForChangeColor(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTERCHANGECOLOR__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilterChangeColor::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForChangeColor( "Active", "" );
	AddPropertyViewForChangeColor( "Reload Shader", "" );
	AddPropertyViewForChangeColor( "Opacity", "" );

	AddPropertyViewForChangeColor( "", "Masking", true );
	AddPropertyViewForChangeColor( "Use Composite Mask", "Masking" );
	AddPropertyViewForChangeColor( "Select Composite Mask", "Masking" );
	AddPropertyViewForChangeColor( "Invert Composite Mask", "Masking" );
	AddPropertyViewForChangeColor( "Custom Mask", "Masking" );

	AddPropertyViewForChangeColor( "", "Info", true );
	AddPropertyViewForChangeColor( "Out Width", "Info" );
	AddPropertyViewForChangeColor( "Out Height", "Info" );
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectFilterChangeColor::ObjectFilterChangeColor( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new CProgramUniformsBase() )
{
	FBClassInit;

	FBPropertyPublish(this, Color1, "Color1", nullptr, nullptr);
	FBPropertyPublish(this, Replace1, "Replace1", nullptr, nullptr);
	FBPropertyPublish(this, Weights1, "Weights1", nullptr, nullptr);

	FBPropertyPublish(this, ChangeColor2, "Use 2nd replacement", nullptr, nullptr);
	FBPropertyPublish(this, Color2, "Color2", nullptr, nullptr);
	FBPropertyPublish(this, Replace2, "Replace2", nullptr, nullptr);
	FBPropertyPublish(this, Weights2, "Weights1", nullptr, nullptr);

	FBPropertyPublish(this, ChangeColor3, "Use 3rd replacement", nullptr, nullptr);
	FBPropertyPublish(this, Color3, "Color3", nullptr, nullptr);
	FBPropertyPublish(this, Replace3, "Replace3", nullptr, nullptr);
	FBPropertyPublish(this, Weights3, "Weights1", nullptr, nullptr);

	Color1 = FBColor(1.0, 1.0, 1.0);
	Replace1 = FBColor(1.0, 1.0, 1.0);
	Weights1 = FBVector4d(400.0, 100.0, 200.0, 100.0);

	ChangeColor2 = false;
	Color2 = FBColor(1.0, 1.0, 1.0);
	Replace2 = FBColor(1.0, 1.0, 1.0);
	Weights2 = FBVector4d(400.0, 100.0, 200.0, 100.0);

	ChangeColor3 = false;
	Color3 = FBColor(1.0, 1.0, 1.0);
	Replace3 = FBColor(1.0, 1.0, 1.0);
	Weights3 = FBVector4d(400.0, 100.0, 200.0, 100.0);
}

void ObjectFilterChangeColor::PrepData()
{
	FBColor color1, replace1, color2, replace2, color3, replace3;
	FBVector4d weights1, weights2, weights3;

	color1 = Color1;
	replace1 = Replace1;
	weights1 = Weights1;
	color2 = Color2;
	replace2 = Replace2;
	weights2 = Weights2;
	color3 = Color3;
	replace3 = Replace3;
	weights3 = Weights3;

	float changeColor2 = (ChangeColor2) ? 1.0f : 0.0f;
	float changeColor3 = (ChangeColor3) ? 1.0f : 0.0f;

	for (int i=0; i<3; ++i)
	{
		mBufferData.color1[i] = (float) color1[i];
		mBufferData.color2[i] = (float) color2[i];
		mBufferData.color3[i] = (float) color3[i];

		mBufferData.replace1[i] = (float) replace1[i];
		mBufferData.replace2[i] = (float) replace2[i];
		mBufferData.replace3[i] = (float) replace3[i];

		mBufferData.weights1[i] = (float) weights1[i];
		mBufferData.weights2[i] = (float) weights2[i];
		mBufferData.weights3[i] = (float) weights3[i];
	}

	mBufferData.weights1[3] = (float) weights1[3];
	mBufferData.weights2[3] = (float) weights2[3];
	mBufferData.weights3[3] = (float) weights3[3];

	mBufferData.flag[0] = changeColor2;
	mBufferData.flag[1] = changeColor3;

	mGPUBuffer.UpdateData( sizeof(ChangeData), 1, &mBufferData );
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Film Grain Filter

void AddPropertyViewForFilmGrain(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTERFILMGRAIN__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilterFilmGrain::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForFilmGrain( "Active", "" );
	AddPropertyViewForFilmGrain( "Reload Shader", "" );
	AddPropertyViewForFilmGrain( "Opacity", "" );

	AddPropertyViewForFilmGrain( "", "Masking", true );
	AddPropertyViewForFilmGrain( "Use Composite Mask", "Masking" );
	AddPropertyViewForFilmGrain( "Select Composite Mask", "Masking" );
	AddPropertyViewForFilmGrain( "Invert Composite Mask", "Masking" );
	AddPropertyViewForFilmGrain( "Custom Mask", "Masking" );

	AddPropertyViewForFilmGrain( "", "Info", true );
	AddPropertyViewForFilmGrain( "Out Width", "Info" );
	AddPropertyViewForFilmGrain( "Out Height", "Info" );
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectFilterFilmGrain::ObjectFilterFilmGrain( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new ProgramUniformsFilmGrain() )
{
	FBClassInit;

	FBPropertyPublish( this, UsePlayTime, "Use Play Time", nullptr, nullptr );
	FBPropertyPublish( this, TimeSpeed, "Time Speed", nullptr, nullptr );

	FBPropertyPublish( this, GrainAmount, "Grain Amount", nullptr, nullptr );
	FBPropertyPublish( this, Colored, "Colored", nullptr, nullptr );
	FBPropertyPublish( this, ColorAmount, "Color Amount", nullptr, nullptr );
	FBPropertyPublish( this, GrainSize, "Grain Size", nullptr, nullptr );
	FBPropertyPublish( this, LumAmount, "Luminance Amount", nullptr, nullptr );

	UsePlayTime = false;
	TimeSpeed = 100.0;

	GrainAmount = 5.0;
	Colored = false;
	ColorAmount = 60.0;
	GrainSize = 16.0;
	GrainSize.SetMinMax(15.0, 25.0);
	LumAmount = 100.0;

	mProgramUniformsFilmGrain = (ProgramUniformsFilmGrain*) mProgramUniforms.get();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Crosshatch Filter

void AddPropertyViewForCrosshatch(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTERCROSSHATCH__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilterCrosshatch::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForCrosshatch( "Active", "" );
	AddPropertyViewForCrosshatch( "Reload Shader", "" );
	AddPropertyViewForCrosshatch( "Opacity", "" );

	AddPropertyViewForCrosshatch( "", "Masking", true );
	AddPropertyViewForCrosshatch( "Use Composite Mask", "Masking" );
	AddPropertyViewForCrosshatch( "Select Composite Mask", "Masking" );
	AddPropertyViewForCrosshatch( "Invert Composite Mask", "Masking" );
	AddPropertyViewForCrosshatch( "Custom Mask", "Masking" );

	AddPropertyViewForCrosshatch( "", "Info", true );
	AddPropertyViewForCrosshatch( "Out Width", "Info" );
	AddPropertyViewForCrosshatch( "Out Height", "Info" );
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectFilterCrosshatch::ObjectFilterCrosshatch( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new CProgramUniformsBase() )
{
	FBClassInit;

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Cross-stitching Filter

void AddPropertyViewForCrossStitching(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTERCROSSSTITCH__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilterCrossStitching::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForCrossStitching( "Active", "" );
	AddPropertyViewForCrossStitching( "Reload Shader", "" );
	AddPropertyViewForCrossStitching( "Opacity", "" );

	AddPropertyViewForCrossStitching( "", "Masking", true );
	AddPropertyViewForCrossStitching( "Use Composite Mask", "Masking" );
	AddPropertyViewForCrossStitching( "Select Composite Mask", "Masking" );
	AddPropertyViewForCrossStitching( "Invert Composite Mask", "Masking" );
	AddPropertyViewForCrossStitching( "Custom Mask", "Masking" );

	AddPropertyViewForCrossStitching( "", "Info", true );
	AddPropertyViewForCrossStitching( "Out Width", "Info" );
	AddPropertyViewForCrossStitching( "Out Height", "Info" );
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectFilterCrossStitching::ObjectFilterCrossStitching( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new CProgramUniformsBase() )
{
	FBClassInit;

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Outline Filter

void AddPropertyViewForOutline(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTERTOONLINES__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilterToonLines::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForOutline( "Active", "" );
	AddPropertyViewForOutline( "Reload Shader", "" );
	AddPropertyViewForOutline( "Opacity", "" );

	AddPropertyViewForOutline( "", "Masking", true );
	AddPropertyViewForOutline( "Use Composite Mask", "Masking" );
	AddPropertyViewForOutline( "Select Composite Mask", "Masking" );
	AddPropertyViewForOutline( "Invert Composite Mask", "Masking" );
	AddPropertyViewForOutline( "Custom Mask", "Masking" );

	AddPropertyViewForOutline( "", "Info", true );
	AddPropertyViewForOutline( "Out Width", "Info" );
	AddPropertyViewForOutline( "Out Height", "Info" );
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectFilterToonLines::ObjectFilterToonLines( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new ProgramUniformsOutline() )
{
	FBClassInit;

	FBPropertyPublish(this, EdgeColor, "Edge Color", nullptr, nullptr);
	FBPropertyPublish(this, Falloff, "Depth Falloff", nullptr, nullptr);
	FBPropertyPublish(this, EdgeMultiply, "Edge Multiply", nullptr, nullptr);
	
	EdgeColor = FBColor(0.0, 0.0, 0.0);
	Falloff = 0.0;
	EdgeMultiply = 100.0;

	mProgramUniformsOutline = (ProgramUniformsOutline*) mProgramUniforms.get();
}

void ObjectFilterToonLines::UpdateOffsetMatrix(const int w, const int h)
{
    GLfloat	xInc = 1.0f / (GLfloat) w;
    GLfloat yInc = 1.0f / (GLfloat) h;
    
	/*
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            textureoffsets[(((i*3)+j)*2)+0] = (-1.0f * xInc) + ((GLfloat)i * xInc);
            textureoffsets[(((i*3)+j)*2)+1] = (-1.0f * yInc) + ((GLfloat)j * yInc);
        }
    }
	*/

	// center 0
	mTextureoffsets[0] = 0.0f;
	mTextureoffsets[1] = 0.0f;
	// top left
	mTextureoffsets[2] = -1.0f * xInc;
	mTextureoffsets[3] = -1.0f * yInc;
	// top
	mTextureoffsets[4] = 0.0f;
	mTextureoffsets[5] = -1.0f * yInc;
	// top right
	mTextureoffsets[6] = 1.0f * xInc;
	mTextureoffsets[7] = -1.0f * yInc;
	// right
	mTextureoffsets[8] = 1.0f * xInc;
	mTextureoffsets[9] = 0.0f;
	// bottom right
	mTextureoffsets[10] = 1.0f * xInc;
	mTextureoffsets[11] = 1.0f * yInc;
	// bottom
	mTextureoffsets[12] = 0.0f;
	mTextureoffsets[13] = 1.0f * yInc;
	// bottom left
	mTextureoffsets[14] = -1.0f * xInc;
	mTextureoffsets[15] = 1.0f * yInc;
	// left
	mTextureoffsets[16] = -1.0f * xInc;
	mTextureoffsets[17] = 0.0f;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SSAO Filter

/************************************************
 *	Constructor.
 ************************************************/
ObjectFilterSSAO::ObjectFilterSSAO( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new CProgramUniformsBase() )
{
	FBClassInit;

	FBPropertyPublish(this, DistanceThreshold, "Distance Threshold", nullptr, nullptr);
	FBPropertyPublish(this, FilterRadiusX, "Filter Radius X", nullptr, nullptr);
	FBPropertyPublish(this, FilterRadiusY, "Filter Radius Y", nullptr, nullptr);
	FBPropertyPublish(this, FilterAspect, "Filter Aspect Ratio", nullptr, nullptr);

	//FBPropertyPublish(this, SamplesCount, "Samples Count", nullptr, nullptr);
	FBPropertyPublish(this, OnlyAO, "Only AO", nullptr, nullptr);

	FBPropertyPublish(this, Gamma, "Gamma", nullptr, nullptr);
	FBPropertyPublish(this, Contrast, "Contrast", nullptr, nullptr);

	DistanceThreshold = 100.0;	// mult by 0.1
	FilterRadiusX = 200.0;		// will be mult by 0.01
	FilterRadiusY = 200.0;
	FilterAspect = true;

	
	OnlyAO = false;

	Gamma = 0.0;
	Contrast = 0.0;

	Gamma.SetMinMax(-100.0, 100.0);
	Contrast.SetMinMax(-100.0, 100.0);
}

void ObjectFilterSSAO::PrepGPUBuffer(const CCompositionInfo *pInfo)
{
	mData.contrast = 1.0 + 0.01 * Contrast;
	mData.gamma = 1.0 + 0.01 * Gamma;

	mData.distanceThreshold = 0.1 * DistanceThreshold;
	mData.filterRadius = vec4( 0.0001 * FilterRadiusX, 0.0001 * FilterRadiusY, 0.0f, 1.0f );

	mData.only_ao = (OnlyAO.AsInt() > 0) ? 1.0f : 0.0f;

	// frustum information for the 3d position reconstruction
	FBCamera *pCamera = ((CCompositionInfo*) pInfo)->GetRenderCamera();

	vec3 points[8];
	int renderWidth = pInfo->GetRenderWidth() - 2 * pInfo->GetRenderBorderX();
	int renderHeight = pInfo->GetRenderHeight() - 2 * pInfo->GetRenderBorderY();

	ComputeCameraFrustumPoints( renderWidth, renderHeight, pCamera, 
		pCamera->FarPlaneDistance, pCamera->NearPlaneDistance, pCamera->FieldOfView, points );
	
	mData.viewOrig[0] = vec4(points[0], 1.0);
	mData.viewDir[0] = vec4(points[4], 1.0);
	
	mData.viewOrig[1] = vec4(points[1], 1.0);
	mData.viewDir[1] = vec4(points[5], 1.0);
	
	mData.viewOrig[2] = vec4(points[2], 1.0);
	mData.viewDir[2] = vec4(points[6], 1.0);

	mData.viewOrig[3] = vec4(points[3], 1.0);
	mData.viewDir[3] = vec4(points[7], 1.0);

	mGPUBuffer.UpdateData(sizeof(Data), 1, &mData);
}
