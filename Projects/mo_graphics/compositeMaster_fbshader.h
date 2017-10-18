#ifndef __COMPOSITE_SCREEN_SHADER_H__
#define __COMPOSITE_SCREEN_SHADER_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_fbshader.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define

#define COMPOSITESCREENSHADER__CLASSSTR	"CompositeScreenShader"
#define COMPOSITESCREENSHADER__DESCSTR	"Composite Screen Shader"

#define COMPOSITESCREENSHADER__CLASSNAME	CompositeScreenShader


////////////////////////////////////////////////////////////////////////////////////////////
//! Dynamic lighting shader using CG to support pixel lighting and normal mapping.
class CompositeScreenShader : public FBShader
{
    // Declaration.
    FBShaderDeclare( CompositeScreenShader, FBShader );

public:

	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }

    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate();
    virtual void FBDestroy();

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
	virtual bool ShaderNeedBeginRender() override;

	/** Pre-rendering of shaders that is called only one time for a shader instance.
	*	\remark	ShaderBeginRender is called as many times as the number of models affected 
	*			by the instance of the shader in a given render pass. (ShaderNeedBeginRender 
	*			must return true for this function to be called).
	*/
	virtual void ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo ) override;

	virtual void ShadeModel( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo, FBRenderingPass pPass ) override;

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

	// put properties into the folders
	static void AddPropertiesToPropertyViewManager();
};

#endif // End
