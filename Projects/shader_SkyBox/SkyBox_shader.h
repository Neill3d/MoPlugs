
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: SkyBox_shader.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef ORSDK_DLL
	/** \def ORSDK_DLL
	*	Be sure that ORSDK_DLL is defined only once...
	*/
	#define ORSDK_DLL K_DLLIMPORT
#endif

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define ORSHADERSKYBOX__CLASSNAME	ORShaderSkyBox
#define ORSHADERSKYBOX__CLASSSTR	"ORShaderSkyBox"
#define ORSHADERSKYBOX__DESCSTR		"ORShaderSkyBox"

////////////////////////////////////////////////////////////////////////////////////////////
//! Shader for use a cubemap user object to draw object albedo with the cubemap texture.
class ORSDK_DLL ORShaderSkyBox : public FBShader
{
	//--- FiLMBOX declaration.
	FBShaderDeclare( ORShaderSkyBox, FBShader );

public:

	FBPropertyListObject				CubeMap;

	FBPropertyAction					ReloadShader;

	FBPropertyBool						FlipHorizontal;
	FBPropertyBool						FlipVertical;

	FBPropertyBool						InvertHorizontal;
	FBPropertyBool						InvertVertical;

	FBPropertyAnimatableVector3d		UVOffset;
	FBPropertyAnimatableVector3d		UVScale;

	FBPropertyAnimatableVector3d		WorldOffset;
	FBPropertyAnimatableVector3d		WorldScale;

	FBPropertyAnimatableVector3d		MovementAffect;
	FBPropertyAnimatableVector3d		MovementOffset;
	FBPropertyAction					CatchOffset;

	FBPropertyInt						LayerId;

	FBPropertyAlphaSource				Transparency;
    FBPropertyAnimatableDouble			TransparencyFactor;  

	FBPropertyBool						DepthTest;

	FBPropertyBool						LogarithmicDepth;
	FBPropertyDouble					DepthDisplacement;	//!> use this property to setUp depth priority instead of real Z positions

	FBPropertyAnimatableDouble			Brightness;
	FBPropertyAnimatableDouble			Saturation;
	FBPropertyAnimatableDouble			Contrast;
	FBPropertyAnimatableDouble			Gamma;

public:

	bool				needRender;

	void DoCatchOffset();
	void DoReloadShader();

	void Render( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass, FBModel *pModel );

public:
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

	/** For all the models associated with this shader instance, to setup the common state for this shader instance. 
		*	\param	pRenderOptions	Render options.
		*	\param	pPass		Rendering pass.
		*/
		virtual void ShaderPassInstanceBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass);  

		/** For all the models associated with this shader instance, to clean the common state for this shader instance.
		*	\param	pRenderOptions	Render options.
		*	\param	pPass		Rendering pass.
		*/
		virtual void ShaderPassInstanceEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass); 

		/** For all the models mapped with this material and associated with this shader instance, to setup the common material state for this shader instance.
		*	\param	pRenderOptions	Render options.
		*	\param	pPass		Rendering pass.
		*	\param	pInfo		Shader-model information.
		*/
		virtual void ShaderPassMaterialBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo);  

		/** For all the models mapped with this material and associated with this shader instance, to clean the common material state for this shader instance.
		*	\param	pRenderOptions	Render options.
		*	\param	pPass		Rendering pass.
		*	\param	pInfo		Shader-model information.
		*/
		virtual void ShaderPassMaterialEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo);  

	virtual bool ShaderNeedBeginRender();

	virtual void ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo );

	void ShadeModel( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass );

	virtual bool FbxStore	(FBFbxObject* pFbxObject);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, FBRenderer* pRenderer);


	/**	Detach the display context from the shader.
		*	\param	pOptions	Render options.
		*	\param	pInfo		Shader-model information object.
		*/
	virtual void DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo );

	static void SetTransparencyProperty( HIObject pObject, FBAlphaSource pState );
	static void SetReloadShaderAction( HIObject pObject, bool value );

private:

	bool				needReload;
	bool				needSetup;

	void				*mShader;
	
	unsigned char		mCullFace;
	int					mCullFaceMode;

};
