
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: SkyDome_shader.cxx
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
	#define ORSDK_DLL K_DLLEXPORT
#endif

// Class declaration
#include "SkyDome_shader.h"

#include <Windows.h>
#include <CommDlg.h>

#include <algorithm>
#include <vector>
#include "graphics\glslShader.h"
#include "IO\FileUtils.h"

//--- Registration defines
#define ORSHADERSKYDOME__CLASS		ORSHADERSKYDOME__CLASSNAME
#define ORSHADERSKYDOME__DESC		"SkyDome"


//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	ORSHADERSKYDOME__CLASS	);
FBRegisterShader	(	ORSHADERSKYDOME__DESCSTR,
					 ORSHADERSKYDOME__CLASS,
					 ORSHADERSKYDOME__DESCSTR,
					 ORSHADERSKYDOME__DESC,
					 FB_DEFAULT_SDK_ICON		);


////////////////////////////////////////////////////////////

static	GLSLShader	*gShader = nullptr;
static	int			gShaderRef = 0;

static GLint				gShaderCameraLoc=0;
static GLint				gShaderContrastLoc=0;
static GLint				gShaderBrightnessLoc=0;
static GLint				gShaderSaturationLoc=0;
static GLint				gShaderGammaLoc=0;

#define	SKYDOME_VERTEX		"\\GLSL\\skydome.vsh"
#define SKYDOME_FRAGMENT	"\\GLSL\\skydome.fsh"

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ORShaderSkyDome::FBCreate()
{
	FBPropertyPublish( this, LayerId, "Layer Id", nullptr, nullptr );

	FBPropertyPublish( this, Contrast, "Contrast", nullptr, nullptr );
	FBPropertyPublish( this, Brightness, "Brightness", nullptr, nullptr );
	FBPropertyPublish( this, Saturation, "Saturation", nullptr, nullptr );
	FBPropertyPublish( this, Gamma, "Gamma", nullptr, nullptr );

	FBPropertyPublish( this, Transparency, "Transparency", nullptr, nullptr );
	FBPropertyPublish( this, TransparencyFactor, "Transparency Factor", nullptr, nullptr );

	LayerId = 0;
	LayerId.SetMinMax( 0.0, 10.0, true, true );

	Brightness = 0.0;
	Brightness.SetMinMax(-100.0, 100.0, true, true);
	Saturation = 0.0;
	Saturation.SetMinMax(-100.0, 100.0, true, true);
	Contrast = 0.0;
	Contrast.SetMinMax(-100.0, 100.0, true, true);
	Gamma = 100.0;
	Gamma.SetMinMax(-300.0, 300.0, true, true);

	Transparency = kFBAlphaSourceNoAlpha;
	TransparencyFactor = 100.0;

	TransparencyFactor.SetMinMax( 0.0, 100.0, true, true );


	//
	RenderingPass = kFBPassPreRender;
	//SetShaderCapacity(kFBShaderCapacityMaterialEffect, false);
	//SetShaderPassActionCallback( (FBShaderPassActionCallback) (kFBShaderPassInstanceBegin | kFBShaderPassInstanceEnd | kFBShaderPassMaterialBegin | kFBShaderPassMaterialEnd) );

	gShaderRef++;

	if (gShader == nullptr)
	{
		
		FBString effectPath, effectFullName;

		try
		{
			gShader = new GLSLShader();
			if (gShader == nullptr)
				throw std::exception( "Failed to allocate memory for a shader" );

			if (false == FindEffectLocation( SKYDOME_FRAGMENT, effectPath, effectFullName ) )
				throw std::exception( "Failed to locate shader files" );

			// most of shaders share the same simple vertex shader
	
			if (false == gShader->LoadShaders( FBString(effectPath, SKYDOME_VERTEX), FBString(effectPath, SKYDOME_FRAGMENT) ) )
				throw std::exception( "Failed to load shader" );
			
			//
			// find locations for all neede shader uniforms
		
			GLint colorLoc = gShader->findLocation( "colorSampler" );
			gShaderCameraLoc = gShader->findLocation( "CameraPos" );
			gShaderContrastLoc = gShader->findLocation( "contrast" );
			gShaderBrightnessLoc = gShader->findLocation( "brightness" );
			gShaderSaturationLoc = gShader->findLocation( "saturation" );
			gShaderGammaLoc = gShader->findLocation( "gamma" );

			gShader->Bind();
			if (colorLoc >= 0)
				GLSLShader::setUniformUINT( colorLoc, 0 );
			gShader->UnBind();
		}
		catch ( const std::exception &e )
		{
			FBMessageBox( "SkyDome Shader", e.what(), "Ok" );
		
			if (gShader != nullptr)
			{
				delete gShader;
				gShader = nullptr;

				gShaderRef=0;
			}
		}


	}

	return true;
}



/************************************************
*	FiLMBOX Destructor.
************************************************/
void ORShaderSkyDome::FBDestroy()
{
	gShaderRef--;

	if (gShaderRef <= 0)
	{
		if (gShader)
		{
			delete gShader;
			gShader = nullptr;
			
		}

		gShaderRef = 0;
	}
}

/*
void ORShaderSkyDome::ShaderPassInstanceBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	glDepthMask(GL_FALSE);
	
	mCullFace = glIsEnabled(GL_CULL_FACE);
	glGetIntegerv( GL_CULL_FACE_MODE, &mCullFaceMode );
}


void ORShaderSkyBox::ShaderPassInstanceEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	glDepthMask(GL_TRUE);
	
	if (mCullFace == GL_TRUE) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	glCullFace(mCullFaceMode);
}

void ORShaderSkyBox::ShaderPassMaterialBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo)
{
	FBMaterial *pMaterial = pInfo->GetFBMaterial();
	FBModel *pModel = pInfo->GetFBModel();

	if (pModel == nullptr || pMaterial == nullptr) return;

	FBModelCullingMode cullMode = pModel->GetCullingMode();

	switch(cullMode)
	{
	case kFBCullingOff:
		glDisable(GL_CULL_FACE);
		break;
	case kFBCullingOnCW:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	case kFBCullingOnCCW:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	}

	pMaterial->OGLInit();
	FBTexture *pTexture = pMaterial->GetTexture();

	if (pTexture)
	{
		glEnable(GL_TEXTURE_2D);
		pTexture->OGLInit(pRenderOptions);
	}
}

void ORShaderSkyBox::ShaderPassMaterialEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo)
{
	glDisable(GL_TEXTURE_2D);
}
*/

/** Does the shader need a begin render call.
*	\remark	Re-implement this function and return true if you need it. This method is called once per shader on each render pass.
*/
bool ORShaderSkyDome::ShaderNeedBeginRender()
{
	return true;
}

/** Pre-rendering of shaders that is called only one time for a shader instance.
*	\remark	ShaderBeginRender is called as many times as the number of models affected 
*			by the instance of the shader in a given render pass. (ShaderNeedBeginRender 
*			must return true for this function to be called).
*/
void ORShaderSkyDome::ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	needRender = true;
}

void ORShaderSkyDome::ShadeModel( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass )
{
	if (needRender == false) return;

	// render all skyboxe layers here

	FBSystem	lSystem;
	FBScene		*mScene = lSystem.Scene;

	int maxIndex = 0;

	std::vector<ORShaderSkyDome*> shaders;

	const int numberOfShaders = mScene->Shaders.GetCount();
	for (int i=0; i<numberOfShaders; ++i)
	{
		if (FBIS( mScene->Shaders[i], ORShaderSkyDome) )
		{
			ORShaderSkyDome *pSkyShader = (ORShaderSkyDome*) mScene->Shaders[i];
			maxIndex = std::max(maxIndex, pSkyShader->LayerId.AsInt() );
			
			shaders.push_back( (ORShaderSkyDome*) mScene->Shaders[i] );
		}
	}

	//

	EnterRendering( pRenderOptions, pPass );

	for (int i=0; i<=maxIndex; ++i)
	{
		for (auto iter=shaders.begin(); iter!=shaders.end(); ++iter)
		{
			if ( (*iter)->LayerId == i)
			{
				bool useBlend = (*iter)->Transparency != kFBAlphaSourceNoAlpha;

				if (useBlend)
				{
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}

				int count = (*iter)->GetDstCount();

				for (int j=0; j<count; ++j)
				{
					FBPlug *pPlug = (*iter)->GetDst(j);

					if (FBIS(pPlug, FBModel) )
					{
						(*iter)->Render(pRenderOptions, (FBModel*) pPlug, useBlend );
					}
				}

				if (useBlend)
				{
					glDisable(GL_BLEND);
				}
			}
		}
	}

	LeaveRendering( pRenderOptions, pPass );
}

void SetMaterial( FBRenderOptions* pRenderOptions, FBMaterial *pMaterial )
{
	pMaterial->OGLInit();
	FBTexture *pTexture = pMaterial->GetTexture();

	if (pTexture)
	{
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		pTexture->OGLInit(pRenderOptions);
	}
}

void UnSetMaterial()
{
	glDisable(GL_TEXTURE_2D);
}

void ORShaderSkyDome::EnterRendering( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	glDepthMask(GL_FALSE);
	
	mCullFace = glIsEnabled(GL_CULL_FACE);
	glGetIntegerv( GL_CULL_FACE_MODE, &mCullFaceMode );
}


void ORShaderSkyDome::LeaveRendering  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	glDepthMask(GL_TRUE);
	
	if (mCullFace == GL_TRUE) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	glCullFace(mCullFaceMode);
}

void ORShaderSkyDome::Render( FBRenderOptions* pRenderOptions, FBModel *pModel, const bool useBlend )
{
	// DONE: render object with one diffuse texture (sky dome)

	FBCamera *pCamera = pRenderOptions->GetRenderingCamera();
	if (pCamera && FBIS(pCamera, FBCameraSwitcher) )
		pCamera = ( (FBCameraSwitcher*) pCamera)->CurrentCamera;

	if (pModel == nullptr || pCamera == nullptr) return;

	FBMatrix m;
	FBVector3d v;
	pCamera->GetCameraMatrix( m, kFBModelView );
	pCamera->GetVector(v);

	glPushMatrix();
	glLoadMatrixd(m);

	FBModelCullingMode cullMode = pModel->GetCullingMode();

	switch(cullMode)
	{
	case kFBCullingOff:
		glDisable(GL_CULL_FACE);
		break;
	case kFBCullingOnCW:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	case kFBCullingOnCCW:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	}

	// draw model here

	FBModelVertexData *pData = pModel->ModelVertexData;

	if (pData)
	{

		pModel->GetMatrix(m);
		glMultMatrixd(m);

		pData->EnableOGLVertexData();
		pData->EnableOGLUVSet();
		
		const GLuint id = pData->GetVertexArrayVBOId(kFBGeometryArrayID_Color);
		if (useBlend)
		{
			if (id > 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, id);
				glEnableClientState(GL_COLOR_ARRAY);
				glColorPointer(4, GL_FLOAT, 0, (GLvoid*) 0); 
			}
		}
		
		if (gShader)
		{
			gShader->Bind();
			if (gShaderCameraLoc >= 0)
				GLSLShader::setUniformVector( gShaderCameraLoc, (float)v[0], (float)v[1], (float)v[2], 1.0 );

			float cValue = 1.0f + 0.01f * (float) Contrast;
			float bValue = 1.0f + 0.01f * (float) Brightness;
			float sValue = 1.0f + 0.01f * (float) Saturation;
			float gValue = 0.01f * (float) Gamma;

			GLSLShader::setUniformFloat( gShaderContrastLoc, cValue );
			GLSLShader::setUniformFloat( gShaderBrightnessLoc, bValue );
			GLSLShader::setUniformFloat( gShaderSaturationLoc, sValue );
			GLSLShader::setUniformFloat( gShaderGammaLoc, gValue );
		}

		const int count = pData->GetSubRegionCount();
		for (int i=0; i<count; ++i)
		{
			SetMaterial( pRenderOptions, pData->GetSubRegionMaterial(i) );
			pData->DrawSubRegion(i);
		}
		
		if (gShader)
		{
			gShader->UnBind();
		}

		pData->DisableOGLUVSet();
		pData->DisableOGLVertexData();

		if (id > 0)
			glDisableClientState(GL_COLOR_ARRAY);

		UnSetMaterial();
	
	}

	glPopMatrix();

	//
	needRender = false;
}

bool ORShaderSkyDome::FbxStore	(FBFbxObject* pFbxObject)
{
	return true;
}

bool ORShaderSkyDome::FbxRetrieve(FBFbxObject* pFbxObject, FBRenderer* pRenderer)
{

	return true;
}


void ORShaderSkyDome::DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{

}

FBShaderModelInfo *ORShaderSkyDome::NewShaderModelInfo(HKModelRenderInfo pModelRenderInfo, int pSubRegionIndex)
{
    FBShaderModelInfo *lShaderModelInfo = ParentClass::NewShaderModelInfo(pModelRenderInfo, pSubRegionIndex);
	//FBShaderModelInfo *lShaderModelInfo = new SuperShaderModelInfo(this, pModelRenderInfo, pSubRegionIndex);
    return lShaderModelInfo;
}

void ORShaderSkyDome::UpdateModelShaderInfo( FBRenderOptions* pOptions, FBShaderModelInfo *pModelRenderInfo )
{
	
    unsigned int lVBOFormat = kFBGeometryArrayID_Point | kFBGeometryArrayID_Normal;

    if (pModelRenderInfo->GetOriginalTextureFlag())
    {
        FBMaterial *lMaterial = pModelRenderInfo->GetFBMaterial();
        if (lMaterial)
        {
            //HFBTexture lDiffuseTexture = pMaterial->GetTexture(kFBMaterialTextureDiffuse);

            FBTexture *lNormalMapTexture = lMaterial->GetTexture(kFBMaterialTextureBump);
            if (lNormalMapTexture == nullptr) 
                lNormalMapTexture = lMaterial->GetTexture(kFBMaterialTextureNormalMap);

            if (lNormalMapTexture)
                lVBOFormat = lVBOFormat | kFBGeometryArrayID_Tangent | kFBGeometryArrayID_Binormal;
        }
    }

	// check if second uv set is needed in some texture
	if (Transparency != kFBAlphaSourceNoAlpha)
		lVBOFormat = lVBOFormat | kFBGeometryArrayID_Color;


    pModelRenderInfo->SetGeometryArrayIds(lVBOFormat);
	
	pModelRenderInfo->UpdateModelShaderInfo(GetShaderVersion());
}