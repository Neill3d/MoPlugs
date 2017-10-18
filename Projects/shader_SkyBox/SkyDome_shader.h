
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: SkyDome_shader.h
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

#include <GL\glew.h>

//--- Registration define
#define ORSHADERSKYDOME__CLASSNAME	ORShaderSkyDome
#define ORSHADERSKYDOME__CLASSSTR	"ORShaderSkyDome"
#define ORSHADERSKYDOME__DESCSTR	"ORShaderSkyDome"

//! Simple shader texmat.
class ORSDK_DLL ORShaderSkyDome : public FBShader
{
	//--- FiLMBOX declaration.
	FBShaderDeclare( ORShaderSkyDome, FBShader );

public:

	FBPropertyInt			LayerId;

	FBPropertyDouble		Contrast;
	FBPropertyDouble		Brightness;
	FBPropertyDouble		Saturation;
	FBPropertyDouble		Gamma;

	FBPropertyAlphaSource		Transparency;
    FBPropertyAnimatableDouble	TransparencyFactor;  

public:

	bool				needRender;

	void Render( FBRenderOptions* pRenderOptions, FBModel *pModel, const bool useBlend );

public:
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

	virtual bool ShaderNeedBeginRender();

	virtual void ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo );

	void ShadeModel( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass );

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

	virtual bool FbxStore	(FBFbxObject* pFbxObject);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, FBRenderer* pRenderer);


	/**	Detach the display context from the shader.
		*	\param	pOptions	Render options.
		*	\param	pInfo		Shader-model information object.
		*/
	virtual void DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo );

private:

	GLboolean			mCullFace;
	GLint				mCullFaceMode;

	void EnterRendering ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass);
	void LeaveRendering ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass);
};
