
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectFilters.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_object.h"
#include "algorithm\nv_math.h"
#include "graphics\OGL_Utils.h"
#include "graphics\UniformBuffer.h"

#define FILTERCSB__CLASSNAME				ObjectFilterColorCorrection
#define FILTERCSB__CLASSSTR					"ObjectFilterColorCorrection"
#define FILTERCSB__ASSETNAME				"Color Correction Filter"

#define FILTERBLUR__CLASSNAME				ObjectFilterBlur
#define FILTERBLUR__CLASSSTR				"ObjectFilterBlur"
#define FILTERBLUR__ASSETNAME				"Blur Filter"

#define FILTERHALFTONE__CLASSNAME			ObjectFilterHalfTone
#define FILTERHALFTONE__CLASSSTR			"ObjectFilterHalfTone"
#define FILTERHALFTONE__ASSETNAME			"HalfTone Filter"

#define FILTERPOSTERIZATION__CLASSNAME		ObjectFilterPosterization
#define FILTERPOSTERIZATION__CLASSSTR		"ObjectFilterPosterization"
#define FILTERPOSTERIZATION__ASSETNAME		"Posterization Filter"

#define FILTERCHANGECOLOR__CLASSNAME		ObjectFilterChangeColor
#define FILTERCHANGECOLOR__CLASSSTR			"ObjectFilterChangeColor"
#define FILTERCHANGECOLOR__ASSETNAME		"Change Color Filter"

#define FILTERFILMGRAIN__CLASSNAME			ObjectFilterFilmGrain
#define FILTERFILMGRAIN__CLASSSTR			"ObjectFilterFilmGrain"
#define FILTERFILMGRAIN__ASSETNAME			"Film Grain Filter"

#define FILTERCROSSHATCH__CLASSNAME			ObjectFilterCrosshatch
#define FILTERCROSSHATCH__CLASSSTR			"ObjectFilterCrosshatch"
#define FILTERCROSSHATCH__ASSETNAME			"Crosshatch Filter"

#define FILTERCROSSSTITCH__CLASSNAME		ObjectFilterCrossStitching
#define FILTERCROSSSTITCH__CLASSSTR			"ObjectFilterCrossStitching"
#define FILTERCROSSSTITCH__ASSETNAME		"Cross Stitching Filter"

#define FILTERTOONLINES__CLASSNAME			ObjectFilterToonLines
#define FILTERTOONLINES__CLASSSTR			"ObjectFilterToonLines"
#define FILTERTOONLINES__ASSETNAME			"Outline Filter"

#define FILTERSSAO__CLASSNAME				ObjectFilterSSAO
#define FILTERSSAO__CLASSSTR				"ObjectFilterSSAO"
#define FILTERSSAO__ASSETNAME				"SSAO Filter"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectFilterColorCorrection

class ObjectFilterColorCorrection : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterColorCorrection, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterColorCorrection);

public:
	ObjectFilterColorCorrection(const char *pName = NULL, HIObject pObject=NULL);

    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

public:
	
	FBPropertyAnimatableDouble		Contrast;
	FBPropertyAnimatableDouble		Brightness;
	FBPropertyAnimatableDouble		Saturation;

	FBPropertyAnimatableDouble		Gamma;

	FBPropertyBool					Inverse;	// inverse colors

	FBPropertyAnimatableDouble		Hue;
	FBPropertyAnimatableDouble		HueSaturation;
	FBPropertyAnimatableDouble		Lightness;

protected:

	virtual const char *AssetNameString() override {
		return FILTERCSB__ASSETNAME;
	}

	virtual const char *MixedProgramLabel() override {
		return "color correction";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterColor.cs";
	}

	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override 
	{
		if (stage == eBeforeProgramDispatch)
		{
			float cValue = 1.0f + 0.01f * (float) Contrast;
			float bValue = 1.0f + 0.01f * (float) Brightness;
			float sValue = 1.0f + 0.01f * (float) Saturation;
			float gValue = 0.01f * (float) Gamma;
	
			float hueValue = 0.01f * (float) Hue;
			float hueSatValue = 0.01f * (float) HueSaturation;
			float lightnessValue = 0.01f * (float) Lightness;

			mProgramUniformsColor->UploadColor(cValue, sValue, bValue, gValue, hueValue, hueSatValue, lightnessValue, Inverse);
		}
	}

protected:

	class ProgramUniformsColor : public CProgramUniformsBase
	{
	protected:
		// locations
		
		GLint		csb;
		GLint		hue;

	public:
		ProgramUniformsColor()
			: CProgramUniformsBase()
		{
			csb = -1;
			hue = -1;
		}

		virtual void Prep(const GLuint programId) override
		{
			CProgramUniformsBase::Prep(programId);

			csb = glGetUniformLocation(programId, "gCSB" );
			hue = glGetUniformLocation(programId, "gHue" );
		}

		void UploadColor(float c, float s, float b, float gamma, float _hue, float _hueSat, float _lightness, bool inverse)
		{
			if (csb >= 0)
			{
				glUniform4f(csb, c, s, b, gamma );
			}
			if (hue >= 0)
			{
				glUniform4f(hue, _hue, _hueSat, _lightness, (inverse) ? 1.0f : 0.0f );
			}
		}
	};

	ProgramUniformsColor					*mProgramUniformsColor;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// blur system

class ShaderBlurSystem
{
public:

	// incoming processing data
	struct Data
	{
		bool	needShaderReload;
		bool	hasMultisamplingSupport;

		GLuint		compositeMaskId;
		bool		invertCompositeMask;
		float		opacity;

		bool useAspectRation;
		bool useSeparateVertFactor;

		int w;
		int h;

		GLuint sourceTexId;
		GLuint dstTexId;

		double	horzFactor;
		double	vertFactor;

		// ! a constructor
		Data()
		{
			needShaderReload = false;
			hasMultisamplingSupport = false;

			compositeMaskId = 0;
			invertCompositeMask = false;
			opacity = 1.0f;

			w = 1;
			h = 1;

			sourceTexId = 0;
			dstTexId = 0;

			horzFactor = 0.0;
			vertFactor = 0.0;
		}
	};

	// shader uniforms
	class ProgramUniformsBlur : public CProgramUniformsBase
	{
	protected:
		// locations
		GLint		scale;

	public:
		ProgramUniformsBlur()
			: CProgramUniformsBase()
		{
			scale = -1;
		}

		virtual ~ProgramUniformsBlur()
		{}

		virtual void Prep(const GLuint programId)
		{
			CProgramUniformsBase::Prep(programId);
			scale = glGetUniformLocation(programId, "scale" );
		}

		void UploadScale(const float x, const float y)
		{
			if (scale >= 0)
				glUniform4f(scale, x, y, 0.0f, 0.0f);
		}
	};

	// stores information for each individual query
	struct CacheData
	{
		int					temp_w;
		int					temp_h;
		GLuint				temp_color_id;	//!> intermediate texture to process blurring
		
		CompositeComputeShader::CMixedProgram		*program;
		ProgramUniformsBlur							*uniforms;
		
		//! a constructor
		CacheData()
		{
			temp_w = 0;
			temp_h = 0;
			temp_color_id = 0;
			program = nullptr;
			uniforms = nullptr;
		}
	};

public:

	static void Create(CacheData &cacheData);
	static void Destroy(CacheData &cacheData);

	static void ChangeContext(CacheData &cacheData);

	static const char *AssetNameString() {
		return FILTERBLUR__ASSETNAME;
	}

	static const char *MixedProgramLabel() {
		return "blur";
	}
	static const char *MixedProgramPath() {
		return "\\GLSL_CS\\FilterBlur.cs";
	}

	static bool CheckIfShaderIsReady(const CCompositionInfo *pInfo, const Data &data, CacheData &cacheData);
	static bool ApplyFilter(const CCompositionInfo *pInfo, const Data &data, CompositeFinalStats &stats, CacheData &cacheData);

protected:

	static void ReSizeIntermidTexture(const int w, const int h, CacheData &cacheData);
	static void FreeIntermidTexture(CacheData &cacheData);
};

/////////////////////////////////////////////////////////////////////////////////////////////
//
class ObjectFilterBlur : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterBlur, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterBlur);

public:
	ObjectFilterBlur(const char *pName = NULL, HIObject pObject=NULL);

	virtual void FBDestroy() override;

	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

public:
	
	FBPropertyAnimatableDouble		HorzBlurFactor;
	FBPropertyBool					UseAspectRatio;
	FBPropertyBool					UseSeparateVertFactor;
	FBPropertyAnimatableDouble		VertBlurFactor;

public:
	
	virtual void ChangeContext() override
	{
		ParentClass::ChangeContext();
		ShaderBlurSystem::ChangeContext(mCacheData);
	}
	virtual void ChangeGlobalSettings() override
	{
		ParentClass::ChangeGlobalSettings();
	}

	// render to buffers with render tree
	virtual bool ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo) override;
	virtual bool ApplyFilter(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override;

protected:
	
	virtual const char *AssetNameString() override {
		return FILTERBLUR__ASSETNAME;
	}

	virtual const char *MixedProgramLabel() override {
		return "blur";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterBlur.cs";
	}
	
protected:

	ShaderBlurSystem::Data			mData;
	ShaderBlurSystem::CacheData		mCacheData;

	void PrepData();
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectFilterHalfTone

class ObjectFilterHalfTone : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterHalfTone, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterHalfTone);

public:
	ObjectFilterHalfTone(const char *pName = NULL, HIObject pObject=NULL);

    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

public:
	
	FBPropertyAnimatableDouble		Frequency;

protected:

	virtual const char *AssetNameString() override {
		return FILTERHALFTONE__ASSETNAME;
	}

	virtual const char *MixedProgramLabel() override {
		return "halftone";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterHalfTone.cs";
	}
	
	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override {
		
		if (stage == eBeforeProgramDispatch)
		{
			mProgramUniformsHalfTone->UploadFrequency( (float) Frequency );
		}
	}

protected:

	class ProgramUniformsHalfTone : public CProgramUniformsBase
	{
	protected:
		// locations
		GLint		freq;

	public:
		ProgramUniformsHalfTone()
			: CProgramUniformsBase()
		{
			freq = -1;
		}

		virtual void Prep(const GLuint programId) override
		{
			CProgramUniformsBase::Prep(programId);
			freq = glGetUniformLocation(programId, "frequency" );
		}

		void UploadFrequency(const float value)
		{
			if (freq >= 0)
				glUniform1f(freq, value);
		}
	};

	ProgramUniformsHalfTone		*mProgramUniformsHalfTone;
};



/////////////////////////////////////////////////////////////////////////////////////////
// FilterPosterization

class ObjectFilterPosterization : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterPosterization, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterPosterization);

public:
	//! a constructor
	ObjectFilterPosterization(const char *pName = NULL, HIObject pObject=NULL);
	
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

public:
	
	FBPropertyAnimatableDouble		NumberOfColors;
	FBPropertyAnimatableDouble		Gamma;
	
protected:

	virtual const char *AssetNameString() override {
		return FILTERPOSTERIZATION__ASSETNAME;
	}

	virtual const char *MixedProgramLabel() override {
		return "posterization";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterPosterization.cs";
	}
	
	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override {
		
		if (stage == eBeforeProgramDispatch)
		{
			mProgramUniformsPosterization->UploadPosterization( 0.1f * (float) NumberOfColors, 0.01f * (float) Gamma );
		}
	}

protected:

	class ProgramUniformsPosterization : public CProgramUniformsBase
	{
	protected:
		// locations
		GLint		numColors;
		GLint		gamma;

	public:
		ProgramUniformsPosterization()
			: CProgramUniformsBase()
		{
			numColors = -1;
			gamma = -1;
		}

		virtual void Prep(const GLuint programId) override
		{
			CProgramUniformsBase::Prep(programId);
			numColors = glGetUniformLocation(programId, "numColors" );
			gamma = glGetUniformLocation(programId, "gamma" );
		}

		void UploadPosterization(const float _numcolors, const float _gamma)
		{
			if (numColors >= 0)
				glUniform1f(numColors, _numcolors);
			if (gamma >= 0)
				glUniform1f(gamma, _gamma);
		}
	};


	ProgramUniformsPosterization		*mProgramUniformsPosterization;
};



/////////////////////////////////////////////////////////////////////////////////////////
// FilterChangeColor

// Weights - hsv influence + w as multiplier !
//

class ObjectFilterChangeColor : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterChangeColor, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterChangeColor);

public:
	//! a constructor
	ObjectFilterChangeColor(const char *pName = NULL, HIObject pObject=NULL);


    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

public:
	
	FBPropertyAnimatableColor		Color1;
	FBPropertyAnimatableColor		Replace1;
	FBPropertyAnimatableVector4d	Weights1;
	
	FBPropertyBool					ChangeColor2;
	FBPropertyAnimatableColor		Color2;
	FBPropertyAnimatableColor		Replace2;
	FBPropertyAnimatableVector4d	Weights2;

	FBPropertyBool					ChangeColor3;
	FBPropertyAnimatableColor		Color3;
	FBPropertyAnimatableColor		Replace3;
	FBPropertyAnimatableVector4d	Weights3;

protected:

	virtual const char *AssetNameString() override {
		return FILTERCHANGECOLOR__ASSETNAME;
	}

	virtual const char *MixedProgramLabel() override {
		return "change color";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterChangeColor.cs";
	}
	
	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override
	{
		if (stage == eBeforeProgramDispatch)
		{
			PrepData();
			mGPUBuffer.Bind(0);
		}
		else if (stage == eAfterProgramDispatch)
		{
			mGPUBuffer.UnBind();
		}
	}

protected:

	struct ChangeData
	{
		vec4		color1;
		vec4		replace1;
		vec4		weights1;	// w as multiplier !

		vec4		flag;	// x - change color2, y - change color3

		vec4		color2;
		vec4		replace2;
		vec4		weights2;

		vec4		color3;
		vec4		replace3;
		vec4		weights3;	
	};

	// gpu memory buffer
	ChangeData			mBufferData;
	CGPUBufferSSBO		mGPUBuffer;

	void		PrepData();
};



/////////////////////////////////////////////////////////////////////////////////////////
// ObjectFilterFilmGrain

class ObjectFilterFilmGrain : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterFilmGrain, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterFilmGrain);

public:
	//! a constructor
	ObjectFilterFilmGrain(const char *pName = NULL, HIObject pObject=NULL);

    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

public:
	
	FBPropertyBool					UsePlayTime;
	FBPropertyAnimatableDouble		TimeSpeed;

	FBPropertyAnimatableDouble		GrainAmount;
	FBPropertyBool					Colored;	// colored noise ?
	FBPropertyAnimatableDouble		ColorAmount;
	FBPropertyAnimatableDouble		GrainSize;	// grain particle size (1.5 - 2.5)
	FBPropertyAnimatableDouble		LumAmount;


protected:

	FBSystem					mSystem;

protected:

	virtual const char *AssetNameString() override {
		return FILTERFILMGRAIN__ASSETNAME;
	}

	virtual const char *MixedProgramLabel() override {
		return "film grain";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterFilmGrain.cs";
	}
	
	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override 
	{
		if (stage == eBeforeProgramDispatch)
		{
			FBTime systemTime = (UsePlayTime) ? mSystem.LocalTime : mSystem.SystemTime;
			float timerMult = 0.01f * TimeSpeed;
			float timer = timerMult * (float) systemTime.GetSecondDouble();
	
			float grainamount = 0.01f * GrainAmount;
			float colored = (Colored) ? 1.0f : 0.0f;
			float coloramount = 0.01f * ColorAmount;
			float grainsize = 0.01f * GrainSize;
			float lumamount = 0.01f * LumAmount;

			mProgramUniformsFilmGrain->UploadFilmGrain( timer, grainamount, colored, coloramount,
				grainsize, lumamount );
		}
	}

protected:

	class ProgramUniformsFilmGrain : public CProgramUniformsBase
	{
	protected:
		// locations
		GLint		timer;
		
		GLint		grainamount; // = 0.05; //grain amount
		GLint		colored; // = false; //colored noise?
		GLint		coloramount; // = 0.6;
		GLint		grainsize; // = 1.6; //grain particle size (1.5 - 2.5)
		GLint		lumamount; // = 1.0; //

	public:
		ProgramUniformsFilmGrain()
			: CProgramUniformsBase()
		{
			timer = -1;

			grainamount = -1;
			colored = -1;
			coloramount = -1;
			grainsize = -1;
			lumamount = -1;
		}

		virtual void Prep(const GLuint programId) override
		{
			CProgramUniformsBase::Prep(programId);
			
			timer = glGetUniformLocation(programId, "timer" );

			grainamount = glGetUniformLocation(programId, "grainamount" );
			colored = glGetUniformLocation(programId, "colored" );
			coloramount = glGetUniformLocation(programId, "coloramount" );
			grainsize = glGetUniformLocation(programId, "grainsize" );
			lumamount = glGetUniformLocation(programId, "lumamount" );
		}

		void UploadFilmGrain(float _timer, float _grainamount, float _colored, 
			float _coloramount, float _grainsize, float _lumamount)
		{
			if (timer >= 0)
				glUniform1f(timer, _timer);
			if (grainamount >= 0)
				glUniform1f(grainamount, _grainamount);
			if (colored >= 0)
				glUniform1f(colored, _colored);
			if (coloramount >= 0)
				glUniform1f(coloramount, _coloramount);
			if (grainsize >= 0)
				glUniform1f(grainsize, _grainsize);
			if (lumamount >= 0)
				glUniform1f(lumamount, _lumamount);
		}
	};


	ProgramUniformsFilmGrain		*mProgramUniformsFilmGrain;
};


/////////////////////////////////////////////////////////////////////////////////////////
// FilterCrosshatch

class ObjectFilterCrosshatch : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterCrosshatch , ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterCrosshatch );

public:
	//! a constructor
	ObjectFilterCrosshatch (const char *pName = NULL, HIObject pObject=NULL);

    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

protected:

	virtual const char *AssetNameString() override {
		return FILTERCROSSHATCH__ASSETNAME;
	}

	virtual const char *MixedProgramLabel() override {
		return "crosshatch";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterCrosshatch.cs";
	}
};


/////////////////////////////////////////////////////////////////////////////////////////
// FilterCrosshatch

class ObjectFilterCrossStitching : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterCrossStitching , ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterCrossStitching );

public:
	//! a constructor
	ObjectFilterCrossStitching (const char *pName = NULL, HIObject pObject=NULL);

    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

protected:

	virtual const char *AssetNameString() override {
		return FILTERCROSSSTITCH__ASSETNAME;
	}

	virtual const char *MixedProgramLabel() override {
		return "cross-stitching";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterCrossStitching.cs";
	}
};


/////////////////////////////////////////////////////////////////////////////////////////
// FilterToonLines

class ObjectFilterToonLines : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterToonLines , ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterToonLines );

public:
	//! a constructor
	ObjectFilterToonLines (const char *pName = NULL, HIObject pObject=NULL);

    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

public:

	FBPropertyAnimatableColor		EdgeColor;
	FBPropertyAnimatableDouble		Falloff;		//!< distance for the lines depth falloff (0 - disabled)
	FBPropertyAnimatableDouble		EdgeMultiply;

protected:

	virtual const char *AssetNameString() override {
		return FILTERTOONLINES__ASSETNAME;
	}

	virtual const char *MixedProgramLabel() override {
		return "toon lines";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterToonLines.cs";
	}
	virtual bool HasMultiSamplingSupport() override {
		return true;
	}

	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override 
	{
		if (stage == eBeforeProgramDispatch)
		{
			UpdateOffsetMatrix( prInfo.GetWidth(), prInfo.GetHeight() );
			FBColor edgeColor;
			EdgeColor.GetData( edgeColor, sizeof(FBColor) );
			mProgramUniformsOutline->UploadOutline( edgeColor, (float) Falloff, 0.01f * (float)EdgeMultiply, 9, mTextureoffsets );
		}
	}

protected:

	float		mTextureoffsets[18];

	void		UpdateOffsetMatrix(const int w, const int h);

protected:

	class ProgramUniformsOutline : public CProgramUniformsBase
	{
	protected:
		// locations
		GLint		edgeColor;
		GLint		falloff;
		GLint		edgeMult;
		GLint		textureCoordOffset;

	public:
		ProgramUniformsOutline()
			: CProgramUniformsBase()
		{
			edgeColor = -1;
			falloff = -1;
			edgeMult = -1;
			textureCoordOffset = -1;
		}

		virtual void Prep(const GLuint programId) override
		{
			CProgramUniformsBase::Prep(programId);
			
			edgeColor = glGetUniformLocation(programId, "outlineColor" );
			falloff = glGetUniformLocation(programId, "falloff" );
			edgeMult = glGetUniformLocation(programId, "edgeScale" );
			textureCoordOffset = glGetUniformLocation(programId, "textureCoordOffset" );
		}

		void UploadOutline(FBColor &color, float _falloff, float edgeScale, int _count, float *_offsets)
		{
			if (edgeColor >= 0)
				glUniform4f(edgeColor, (float)color[0], (float)color[1], (float)color[2], 1.0);
			if (falloff >= 0)
				glUniform1f(falloff, _falloff);
			if (edgeMult >= 0)
				glUniform1f(edgeMult, edgeScale);
			if (textureCoordOffset >= 0)
				glUniform2fv(textureCoordOffset, _count, _offsets );
		}
	};


	ProgramUniformsOutline		*mProgramUniformsOutline;
};


/////////////////////////////////////////////////////////////////////////////////////////
// FilterSSAO

class ObjectFilterSSAO : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterSSAO , ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterSSAO );

public:
	//! a constructor
	ObjectFilterSSAO (const char *pName = NULL, HIObject pObject=NULL);

public:

	FBPropertyDouble		DistanceThreshold;
	FBPropertyDouble		FilterRadiusX;
	FBPropertyDouble		FilterRadiusY;
	FBPropertyBool			FilterAspect;
	//FBPropertyInt			SamplesCount;
	
	FBPropertyDouble		Gamma;
	FBPropertyDouble		Contrast;

	FBPropertyBool			OnlyAO;		//!< display only ambient occlusion or multiply with child color

protected:

	virtual const char *AssetNameString() override {
		return FILTERSSAO__ASSETNAME;
	}

	virtual const char *MixedProgramLabel() override {
		return "SSAO";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterSSAO.cs";
	}
	virtual bool HasMultiSamplingSupport() override {
		return true;
	}

	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override 
	{
		if (stage == eBeforeProgramDispatch)
		{
			PrepGPUBuffer(pInfo);
			mGPUBuffer.Bind(0);
		}
	}

protected:

	struct Data
	{
		vec4	viewDir[4];
		vec4	viewOrig[4];
		vec4		filterRadius;		// = vec2( 0.02, 0.02 );

		float 		distanceThreshold;	// = 10.5;
		float		only_ao;

		float		gamma;			// = 1.0;
		float		contrast;		// = 1.0;
	};

	// gpu memory buffer
	Data							mData;
	
	CGPUBufferSSBO			mGPUBuffer;

	void PrepGPUBuffer(const CCompositionInfo *pInfo);
};