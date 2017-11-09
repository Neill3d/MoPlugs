#ifndef __PROJTEX_SHADER_H__
#define __PROJTEX_SHADER_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ProjTex_shader.h
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
#include "..\Common_Projectors\bakeProjectors_projectors.h"
#include "FX_shader.h"

#include "callback_factory.h"
#include "callbacks.h"

//--- Registration define

#define PROJTEX__CLASSSTR	"ProjTexShader"
#define PROJTEX__DESCSTR	"Projected texture Plugin"

#define PROJTEX__CLASSNAME	ProjTexShader

#define PROJTEX__DESC	"Projective mapping" // This is what shows up in the shader window ...
#define PROJTEX__ASSETPATH	    "Browsing/Templates/Shading Elements/Shaders"


////////////////////////////////////////////////////////////////////////////////////////////
//! Dynamic lighting shader using CG to support pixel lighting and normal mapping.
class ProjTexShader : public FBShader, public FXProjectionMappingContainer
{
    // Declaration.
    FBShaderDeclare( ProjTexShader, FBShader );

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
	/*
	// internal common functions
	static void InternalPassTypeBegin    (CGPUFBScene *pScene, bool renderToNormalAndMask, const bool logarithmicDepth, const bool cubeMapSetup, const CubeMapRenderingData *cubemap=nullptr);
	static void InternalPassTypeEnd      (CGPUFBScene *pScene, bool renderToNormalAndMask );     
	bool InternalPassInstanceBegin(  const bool isIDBufferRendering, FBRenderingPass pPass );  
	void InternalPassInstanceEnd  ( const bool isIDBufferRendering, FBRenderingPass pPass );  
	void InternalPassMaterialBegin( const bool isIDBufferRendering, FBMaterial *pMaterial );  
	void InternalPassMaterialEnd  ( const bool isIDBufferRendering );  
	// return false if couldn't find FBShaderModelInfo data for that model
	bool InternalPassModelDraw ( const bool isIDBufferRendering, FBModel *pModel, const int subRegionIndex, FBShaderModelInfo *pSuperInfo, const bool directBind );
	*/
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

    // Will be automatically called when the Transparency property will be changed.
    static void SetTransparencyProperty(HIObject pObject, FBAlphaSource pState);

	void		DoReloadShader();
	
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

	FBPropertyBool						RenderOnBack;	// we can render some objects in a separate 3d space (back, normal and front spaces)
	//FBPropertyBool						RenderOnLayer2;	// render into a secondary layer

	FBPropertyBaseEnum<ERenderLayer>	RenderLayer; // address shader models rendering into specified render layer

	FBPropertyBaseEnum<EShadingType>	ShadingType;	//!> style for point cloud drawing
	//FBPropertyBool						LogarithmicDepth;
	FBPropertyDouble					DepthDisplacement;	//!> use this property to setUp depth priority instead of real Z positions

	FBPropertyAction					ReloadShader;

	FBPropertyAnimatableDouble			ToonSteps;
	FBPropertyAnimatableDouble			ToonDistribution;
	FBPropertyAnimatableDouble			ToonShadowPosition;

	FBPropertyAnimatableDouble			Brightness;
	FBPropertyAnimatableDouble			Saturation;
	FBPropertyAnimatableDouble			Contrast;
	FBPropertyAnimatableDouble			Gamma;

	// MatCap and Rim lighting
	FBPropertyAnimatableDouble			UseRim;
	FBPropertyAnimatableDouble			RimPower;
	FBPropertyAnimatableColor			RimColor;
	FBPropertyListObject				RimTexture;		// use texture instead of color
	FBPropertyBaseEnum<EBlendType>		RimBlend;		//!< type of blending two images
	
	FBPropertyListObject				MatCap;		//!< connect a MatCap texture to the shader

	// custom color
	FBPropertyAnimatableColor						CustomColor;
	FBPropertyBaseEnum<EBlendType>					CustomColorMode;		//!< type of blending two images

	// detail blend mode and opacity
	FBPropertyAnimatableDouble						DetailOpacity;
	FBPropertyBaseEnum<EBlendType>					DetailBlendMode;		//!< type of blending two images
	FBPropertyBool									DetailMasked;			//!< should we use mask for blending color and detail textures ?

	// polygon offset (to avoid z fighting)
	FBPropertyAnimatableBool						UsePolygonOffset;
	FBPropertyAnimatableDouble						PolygonOffsetFactor;
	FBPropertyAnimatableDouble						PolygonOffsetUnits;

	// TODO: in progress
	/*
	FBPropertyAnimatableDouble						WireFrameWidth;
	FBPropertyBool									ShowWireFrame;
	FBModelCullingMode								WireCullingMode;
	*/

	FBPropertyBool									CompositeMaskA;
	FBPropertyBool									CompositeMaskB;
	FBPropertyBool									CompositeMaskC;
	FBPropertyBool									CompositeMaskD;

	//FBPropertyColorAndAlpha							CompositeMask;

	
protected:

	CGPUFBScene							*mGPUFBScene;

	CShadersFactory				mShadersFactory;

	CBaseShaderCallback			*mShaderCallback;
	CBaseShaderInfo				*mShaderInfo;

	EShaderPass					mShaderPass;

	bool						mNeedToCheckColorCorrection;
	bool						mApplyColorCorrection;

public:

	bool				mSelectionMode;

	bool				mNeedToUpdateProjectors;

	unsigned int		mLastUniqueFrameId;
	CProjectors			mProjectors;

	
	bool								mNeedUpdateLightsList;

	std::vector<FBLight*>				mLightsPtr;
	std::auto_ptr<CGPUShaderLights>		mShaderLights;

	const CProjectors *GetProjectorsPtr() const {
		return &mProjectors;
	}

	const CGPUShaderLights *GetShaderLightsPtr() const {
		return mShaderLights.get();
	}
	
	const bool IsApplyColorCorrection() const {
		return mApplyColorCorrection;
	}

	void		CheckApplyColorCorrection(FBPlug *pPlug);

	/*
	CProjectors							mProjectors;
	FXProjectionMapping					*mLastFXShader;

	CGPUFBScene							*mGPUFBScene;
	Graphics::ShaderEffect				*mUberShader;

	FBModel								*mLastModel;
	FBMaterial							*mLastMaterial;

	bool								mShaderSuccess;
	bool								mSkipRender;
	
private:

	GLboolean			mCullFace;
	GLint				mCullFaceMode;

	void StoreCullMode();
	void FetchCullMode();

private:

	bool								mFXProjectionBinded;
	*/
};

#endif // End __PROJTEX_H__
