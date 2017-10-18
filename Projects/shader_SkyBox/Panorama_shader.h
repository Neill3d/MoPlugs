
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Panorama_shader.h
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

#include "Shader.h"

#include "Panorama.h"

//--- Registration define
#define ORSHADERPANORAMA__CLASSNAME	ORShaderPanorama
#define ORSHADERPANORAMA__CLASSSTR	"ORShaderPanorama"
#define ORSHADERPANORAMA__DESCSTR		"ORShaderPanorama"

//! Panoramic shader .
class ORSDK_DLL ORShaderPanorama : public FBShader
{
	//--- FiLMBOX declaration.
	FBShaderDeclare( ORShaderPanorama, FBShader );

public:

	FBPropertyListObject			Media;
	
	FBPropertyAction				Clear;
	FBPropertyAction				Load;

	FBPropertyBool					FlipHorizontal;
	FBPropertyBool					FlipVertical;

	FBPropertyBool					InvertHorizontal;
	FBPropertyBool					InvertVertical;

	FBPropertyAnimatableVector3d	UVOffset;
	FBPropertyAnimatableVector3d	UVScale;

	FBPropertyAnimatableVector3d	WorldOffset;
	FBPropertyAnimatableVector3d	WorldScale;

	FBPropertyAnimatableVector3d	MovementAffect;
	FBPropertyAnimatableVector3d	MovementOffset;
	FBPropertyAction				CatchOffset;

	FBPropertyInt					LayerId;

	FBPropertyAlphaSource			Transparency;
    FBPropertyAnimatableDouble		TransparencyFactor;  

	//
	FBPropertyDouble				LocationLat;
	FBPropertyDouble				LocationLng;
	FBPropertyInt					Zoom;
	FBPropertyAction				Grab;

public:

	bool				needRender;

	void DoClear();
	void DoLoad(const bool showDialog=false);
	void DoCatchOffset();
	void DoGrab();

	void Render( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass );

public:
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

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

private:

	GLuint				mVao;
	GLuint				mNumberOfVertices;

	bool				needLoad;
	bool				needSetup;

	Shader				mShader;

	FBModel				*mModel;

	void				PrepareModelVAO();
};
