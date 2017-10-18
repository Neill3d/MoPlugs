
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectDOFFilter.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_object.h"
#include "compositeMaster_objectFilters.h"

#include <map>
#include "algorithm\nv_math.h"
#include "graphics\OGL_Utils.h"

#define FILTER3DDOF__CLASSNAME				ObjectFilter3dDOF
#define FILTER3DDOF__CLASSSTR				"ObjectFilter3dDOF"
#define FILTER3DDOF__ASSETNAME				"3d DOF Filter"



//////////////////////////////////////////////////////////////////////
//
class ObjectFilter3dDOF : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilter3dDOF, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilter3dDOF);

public:
	ObjectFilter3dDOF(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

public:
	
	FBPropertyBool		UseCameraDOFProperties;	// take focal distance, focal range and focus object from camera properties !

	FBPropertyAction	ResetAction;

	FBPropertyBool		DebugBlurValue;	// output only computed blur gradient

	FBPropertyAnimatableDouble		FocalDistance;
	FBPropertyAnimatableDouble		FocalRange;
	FBPropertyAnimatableDouble		FStop;

	FBPropertyAnimatableBool		AutoFocus;		// autofocus to some point (instead of focal distance)
	FBPropertyListObject			FocusObject;	//  get a screen position of a specified tm
	FBPropertyAction				SelectFocusObject;
	FBPropertyBool					Vignetting;		// optical lens vignetting

	FBPropertyBool					BlurForeground;	// flag to make everything clean in foreground

	FBPropertyBool					ManualFocus;
	FBPropertyAnimatableDouble		ManualNear;
	FBPropertyAnimatableDouble		ManualNearFalloff;
	FBPropertyAnimatableDouble		ManualFar;
	FBPropertyAnimatableDouble		ManualFarFalloff;

	FBPropertyBool					FastPreview;	//!< mode for using pre-calculated blur instead of clean DOF processing
	FBPropertyBaseEnum<ECompositionQuality>	PreviewQuality;
	FBPropertyDouble				PreviewBlurAmount;

	FBPropertyInt					Samples;	// samples of the first ring
	FBPropertyInt					Rings;		// ring count

	FBPropertyAnimatableDouble		CoC;	// circle of confusion size in mm (35 mm film = 0.03mm)
	
	FBPropertyAnimatableDouble		Threshold;	// highlight threshold
	FBPropertyAnimatableDouble		Gain;		// highlight gain

	FBPropertyAnimatableDouble		Bias;	// bokeh edge bias
	FBPropertyAnimatableDouble		Fringe;	// bokeh chromatic aberration/fringing

	FBPropertyBool					Noise;	// use noise instead of pattern for sample dithering

	FBPropertyBool					Pentagon;		// use pentagon as bokeh shape
	FBPropertyAnimatableDouble		PentagonFeather;	// pentagon shape feather

	static void SetResetAction(HIObject object, bool value);
	static void SetSelectFocusObject(HIObject object, bool value);
	static void SetFastPreview(HIObject object, bool value);

public:

	virtual void ChangeContext() override
	{
		ParentClass::ChangeContext();

		ShaderBlurSystem::ChangeContext(mBlurCache);
		mBlurTexture.FreeLayersData();
	}

protected:

	virtual const char *MixedProgramLabel() override {
		return "3d DOF";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterDOF.cs";
	}
	virtual void OnSetProgramDefines( const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, FBString &defineLine ) override
	{
		if (Noise.AsInt() > 0)
			defineLine += "#define USE_NOISE\n";
		if (Vignetting.AsInt() > 0)
			defineLine += "#define VIGNETTING\n";
		if (Pentagon.AsInt() > 0)
			defineLine += "#define PENTAGON\n";
		if (BlurForeground.AsInt() == 0)
			defineLine += "#define KEEP_FOREGROUND\n";

		if (FastPreview.AsInt() > 0)
			defineLine += "#define FAST_PREVIEW\n";

		/*
			SHOW_FOCUS
			VIGNETTING
			PENTAGON
			KEEP_FOREGROUND
			MANUAL_DOF

			int lVignetting = (Vignetting) ? 1 : 0;
	
			int lPentagon = (Pentagon) ? 1 : 0;
			float lBlurForeground = (BlurForeground) ? 1.0f : 0.0f;
	
		*/
	}

	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override 
	{
		if (stage == eBeforeProgramBind)
		{
			// TODO: check if are under fast preview mode

			if (true == FastPreview && true == GenBlurData(pInfo, prInfo, stats, sourceTexId, dstTexId) )
			{
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, mBlurTexture.GetCurrentTextureId() );
				glActiveTexture(GL_TEXTURE0);
			}
		}
		else
		if (stage == eBeforeProgramDispatch)
		{
			PrepData(pInfo);
			mDataBuffer.Bind(0);
		}
		else if (stage == eAfterProgramDispatch)
		{
			mDataBuffer.UnBind();
		}
	}

protected:

	
	struct Data
	{
		float 	focalDepth;		//focal distance value in meters, but you may use autofocus option below
		float 	focalLength;		//focal length in mm
		float 	fstop;			// = 0.5; //f-stop value
		//float	showFocus;		//show debug focus point and focal range (red = focal point, green = focal range)
	
		//------------------------------------------
		//user variables

		int samples; // = 3; //samples on the first ring
		int rings; // = 3; //ring count

		//float blurForeground;

		//bool manualdof; // = false; //manual dof calculation
		float ndofstart; // = 1.0; //near dof blur start
		float ndofdist; // = 2.0; //near dof blur falloff distance
		float fdofstart; // = 1.0; //far dof blur start
		float fdofdist; // = 3.0; //far dof blur falloff distance

		float CoC; // = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)

		//bool vignetting; // = true; //use optical lens vignetting?
		//bool autofocus; // = false; //use autofocus in shader? disable if you use external focalDepth value
		vec2 focus; // = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)

		float threshold; // = 0.5; //highlight threshold;
		float gain; // = 2.0; //highlight gain;

		float bias; // = 0.5; //bokeh edge bias
		float fringe; // = 0.7; //bokeh chromatic aberration/fringing

		// define USE_NOISE instead !
		//bool noise; // = true; //use noise instead of pattern for sample dithering

		/*
		next part is experimental
		not looking good with small sample and ring count
		looks okay starting from samples = 4, rings = 4
		*/

		//bool pentagon; // = false; //use pentagon as bokeh shape?
		float feather; // = 0.4; //pentagon shape feather
	
		float debugBlurValue;
		float temp2;
		float temp3;
	};

	// gpu memory buffer
	Data							mData;

	CGPUBufferSSBO					mDataBuffer;

	void		PrepData(const CCompositionInfo *pInfo);

protected:

	FBSystem					mSystem;

	CompositeComputeShader::CComputeTextureBuffer<1>	mBlurTexture;

	ShaderBlurSystem::Data			mBlurData;
	ShaderBlurSystem::CacheData		mBlurCache;

	bool GenBlurData(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId);

	void DoReset();
	void DoSelectFocusObject();
};