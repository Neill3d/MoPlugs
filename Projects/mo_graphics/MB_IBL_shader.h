
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_IBL_shader.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "..\Common\Common_shader.h"
#include "ShaderFX.h"
#include "shared_content.h"

#include "callback_factory.h"
#include "callbacks.h"

//--- Registration define

#define IBLSHADER__CLASSSTR			"ORIBLShader"
#define IBLSHADER__DESCSTR			"IBL Shader Plugin"

#define IBLSHADER__CLASSNAME		ORIBLShader

#define IBLSHADER__DESC				"IBL Shader" // This is what shows up in the shader window ...
#define IBLSHADER__ASSETPATH	    "Browsing/Templates/Shading Elements/IBL Shader"

// phong shading with parallax refraction and cube-map reflection

#define EYESHADER__CLASSSTR			"ORCharacterEyeShader"
#define EYESHADER__DESCSTR			"Character Eye Shader Plugin"

#define EYESHADER__CLASSNAME		ORCharacterEyeShader

#define EYESHADER__DESC				"Eye Shader" // This is what shows up in the shader window ...
#define EYESHADER__ASSETPATH	    "Browsing/Templates/Shading Elements/Eye Shader"

// skin shader with sss emulation (smooth normal)

#define SKINSHADER__CLASSSTR		"ORCharacterSkinShader"
#define SKINSHADER__DESCSTR			"Character Skin Shader Plugin"

#define SKINSHADER__CLASSNAME		ORCharacterSkinShader

#define SKINSHADER__DESC			"Skin Shader" // This is what shows up in the shader window ...
#define SKINSHADER__ASSETPATH	    "Browsing/Templates/Shading Elements/Skin Shader"


////////////////////////////////////////////////////////////////////////////////////////////
//! Dynamic lighting shader using CG to support pixel lighting and normal mapping.
class ORIBLShader : public FBShader
{
    // Declaration.
    FBShaderDeclare( ORIBLShader, FBShader );

public:

	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	// put properties into the folders
	static void AddPropertiesToPropertyViewManager();

    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate();
    virtual void FBDestroy();

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize) override;

	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,
		FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;
	
    /**
    * \name Real-time shading callbacks for kFBMaterialEffectShader. 
    */
    //@{

    /** For all the shader instances of the same type, to setup the common state for all this type of shaders. 
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    *	\param	pInfo		Shader-model information.
    */
    virtual void ShaderPassTypeBegin    ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass);  

    /** For all the shader instances of the same type, to clean the common state for all this type of shaders 
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    */
    virtual void ShaderPassTypeEnd      ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass);     

    /** For all the models associated with this shader instance, to setup the common state for this shader instance. 
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    */
    virtual void ShaderPassInstanceBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass);  

    /** For all the models associated with this shader instance, to clean the common state for this shader instance.
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    */
    virtual void ShaderPassInstanceEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass);  

    /** For all the models mapped with this material and associated with this shader instance, to setup the common material state for this shader instance.
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    *	\param	pInfo		Shader-model information.
    */
    virtual void ShaderPassMaterialBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo);  

    /** For all the models mapped with this material and associated with this shader instance, to clean the common material state for this shader instance.
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    *	\param	pInfo		Shader-model information.
    */
    virtual void ShaderPassMaterialEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo);  

    /** Callback for Cg/CgFx shader to setup WorldMaitrx/WorldMatrixIT parameters. 
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    *	\param	pInfo		Shader-model information.
    */
    virtual void ShaderPassModelDraw ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo);

    /** Shadow draw pass begin notify for shader with kFBShaderCapacityMaterialEffect and kFBShaderCapacityDrawShow.
    *	\param	pOptions	Render options.
    */
    virtual void ShaderPassDrawShadowBegin( FBRenderOptions* pRenderOptions);

    /** Shadow draw pass end notify for shader with kFBShaderCapacityMaterialEffect and kFBShaderCapacityDrawShow.
    *	\param	pOptions	Render options.
    */
    virtual void ShaderPassDrawShadowEnd( FBRenderOptions* pRenderOptions);

    //@}

	/** Does the shader need a begin render call.
	*	\remark	Re-implement this function and return true if you need it. This method is called once per shader on each render pass.
	*/
	virtual bool ShaderNeedBeginRender();

	/** Pre-rendering of shaders that is called only one time for a shader instance.
	*	\remark	ShaderBeginRender is called as many times as the number of models affected 
	*			by the instance of the shader in a given render pass. (ShaderNeedBeginRender 
	*			must return true for this function to be called).
	*/
	virtual void ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo );

    /** Create a new shader-model information object.
    *	\param	pOptions	        Render options.
    *	\param	pModelRenderInfo 	Internal Model Render Info.
    *	\return	Newly allocated shader-model information object.
    */
    virtual FBShaderModelInfo *NewShaderModelInfo(HKModelRenderInfo pModelRenderInfo, int pSubRegionIndex) ;

    /** Update shader-model information when model, material & texture mapping or shader setting change.
    *	\param	pOptions	Render options.
    *	\param	pInfo		Shader-model information object to be updated.
    */
    virtual void UpdateModelShaderInfo( FBRenderOptions* pOptions, FBShaderModelInfo *pInfo );

    /** Callback for uploading the modelview array when draw instanced.
    *   \param pModelViewMatrixArray    pointer of 4x4 column major modelview matrix array, length is 4*4*pCount;
    *   \param pCount                   number of modelview matrixs in the array.
    */
    virtual void UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount);

	/**	Detach the display context from the shader.
		*	\param	pOptions	Render options.
		*	\param	pInfo		Shader-model information object.
		*/
	virtual void DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo );

    // Accessors and mutators for transparency type property.
    void SetTransparencyType( FBAlphaSource pTransparency );
    FBAlphaSource GetTransparencyType();

	virtual void		DoReloadShader();
	
	void		AskToUpdateAll();

	// add update task to the queue
	void		AskToUpdateLightList();
	// process lighting list if update task exist
	void		EventBeforeRenderNotify();

	void SetLogarithmicDepth(const bool value) const;

public:
    FBPropertyListObject				AffectingLights;		//!< Selected Lights to illuminate the connected models (to avoid maximum lights number limitation in OpenGL)
	FBPropertyBool						UseSceneLights;			//!< should we use all avaliable scene lights or just default lighting model
	FBPropertyAlphaSource				Transparency;
    FBPropertyAnimatableDouble			TransparencyFactor;  
	FBPropertyBool						InverseTransparencyFactor;	//!< use it for back compatibility with old way of alpha calculation

	FBPropertyBool						UseAlphaFromColor;

	FBPropertyBaseEnum<ERenderLayer>	RenderLayer; // address shader models rendering into specified render layer

	FBPropertyBaseEnum<EShadingType>	ShadingType;	//!> style for point cloud drawing
	
	FBPropertyAction					ReloadShader;

	// MatCap and Rim lighting
	FBPropertyAnimatableDouble			UseRim;
	FBPropertyAnimatableDouble			RimPower;
	FBPropertyAnimatableColor			RimColor;
	FBPropertyListObject				RimTexture;		// use texture instead of color
	FBPropertyBaseEnum<EBlendType>		RimBlend;		//!< type of blending two images
	
	// BRDF function
	// TODO: select type (Lighting Model) - phong, blinn, cook-torence

	// Baked environment - connect baked cubemaps here
	// TODO: choose to light with env only or scene lights as well
	FBPropertyListObject			DiffuseEnv;
	FBPropertyListObject			SpecularEnv;
	FBPropertyListObject			BRDF;

	//FBPropertyListObject				MatCap;		//!< connect a MatCap texture to the shader

	// render models into a specified composition mask buffer
	FBPropertyBool									CompositeMaskA;
	FBPropertyBool									CompositeMaskB;
	FBPropertyBool									CompositeMaskC;
	FBPropertyBool									CompositeMaskD;
	
	// properties callbacks
	static void SetReloadShader(HIObject object, bool pValue);
	// Will be automatically called when the Transparency property will be changed.
    static void SetTransparencyProperty(HIObject pObject, FBAlphaSource pState);

protected:

	CGPUFBScene							*mGPUFBScene;

	CShadersFactory				mShadersFactory;

	CBaseShaderCallback			*mShaderCallback;
	CBaseShaderInfo				*mShaderInfo;

	EShaderPass					mShaderPass;

public:

	bool				mSelectionMode;

	unsigned int		mLastUniqueFrameId;
	
	
	bool								mNeedUpdateLightsList;

	std::vector<FBLight*>				mLightsPtr;
	std::auto_ptr<CGPUShaderLights>		mShaderLights;

	const CGPUShaderLights *GetShaderLightsPtr() const {
		return mShaderLights.get();
	}
	

	
};

///////////////////////////////////////////////////////////////////////////////
// eye shader - parallax refraction, cubemap reflection, normal map

class ORCharacterEyeShader : public ORIBLShader
{
    // Declaration.
    FBShaderDeclare( ORCharacterEyeShader, ORIBLShader );

public:

    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate() override;
    virtual void FBDestroy() override;

public:

	FBPropertyListObject			EyeEnvReflection;
	FBPropertyListObject			EyeEnvDiffuse;
	FBPropertyListObject			EyeEnvRefraction;

	FBPropertyAnimatableDouble		IrisSize;
	FBPropertyAnimatableDouble		CorneaBumpAmount;
	FBPropertyAnimatableDouble		CorneaBumpRadiusMult;

	FBPropertyAnimatableDouble		PupilSize;
	FBPropertyAnimatableDouble		IrisTexStart;
	FBPropertyAnimatableDouble		IrisTexEnd;
	FBPropertyAnimatableDouble		IrisBorder;
	//FBPropertyAnimatableDouble		IrisSize;
	FBPropertyAnimatableDouble		IrisEdgeFade;
	FBPropertyAnimatableDouble		IrisInsetDepth;
	FBPropertyAnimatableDouble		ScleraTexScale;
	FBPropertyAnimatableDouble		ScleraTexOffset;
	FBPropertyAnimatableDouble		Ior;
	FBPropertyAnimatableDouble		RefractEdgeSoftness;

	FBPropertyAnimatableDouble		IrisTextureCurvature;
	FBPropertyAnimatableDouble		ArgIrisShadingCurvature;

	FBPropertyAnimatableDouble		TexUOffset;
	FBPropertyAnimatableDouble		IrisNormalOffset;
	FBPropertyAnimatableDouble		CorneaDensity;
	FBPropertyAnimatableDouble		BumpTexture;
	FBPropertyBool					CatShape;
	FBPropertyAnimatableDouble		CybShape;
	FBPropertyInt					ColTexture;

	FBPropertyAction				ResetValues;

	// properties callbacks
	static void SetResetValues(HIObject object, bool pValue);

protected:
	void SetDefaultValues();
};

///////////////////////////////////////////////////////////////////////////////
// skin shader - normal map, "smooth" normals

class ORCharacterSkinShader : public ORIBLShader
{
    // Declaration.
    FBShaderDeclare( ORCharacterSkinShader, ORIBLShader );

public:

    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate() override;
    virtual void FBDestroy() override;

};


