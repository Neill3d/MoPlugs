
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: SkyBox_shader.cxx
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
#include "SkyBox_shader.h"
#include <GL\glew.h>
//#include "shared_textures.h"
#include "Shader.h"

#include <vector>

#include "IO\FileUtils.h"

//--- Registration defines
#define ORSHADERSKYBOX__CLASS		ORSHADERSKYBOX__CLASSNAME
#define ORSHADERSKYBOX__DESC		"SkyBox"


//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	ORSHADERSKYBOX__CLASS	);
FBRegisterShader	(	ORSHADERSKYBOX__DESCSTR,
					 ORSHADERSKYBOX__CLASS,
					 ORSHADERSKYBOX__DESCSTR,
					 ORSHADERSKYBOX__DESC,
					 FB_DEFAULT_SDK_ICON		);


////////////////////////////////////////////////////////////

static void SkyBox_CatchOffset(HIObject object, bool pValue)
{
	ORShaderSkyBox *shader = FBCast<ORShaderSkyBox>(object);
	if (shader && pValue) 
	{
		shader->DoCatchOffset();
	}
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ORShaderSkyBox::FBCreate()
{

	mShader = nullptr;

	FBPropertyPublish( this, CubeMap, "Cube Map", nullptr, nullptr );

	FBPropertyPublish( this, ReloadShader, "Reload Shader", nullptr, SetReloadShaderAction );

	FBPropertyPublish( this, FlipHorizontal, "Flip Horizontal", nullptr, nullptr );
	FBPropertyPublish( this, FlipVertical, "Flip Vertical", nullptr, nullptr );

	FBPropertyPublish( this, UVOffset, "UV Offset", nullptr, nullptr );
	FBPropertyPublish( this, UVScale, "UV Scale", nullptr, nullptr );

	FBPropertyPublish( this, WorldOffset, "World Offset", nullptr, nullptr );
	FBPropertyPublish( this, WorldScale, "World Scale", nullptr, nullptr );

	FBPropertyPublish( this, MovementAffect, "Movement Affect", nullptr, nullptr );
	FBPropertyPublish( this, MovementOffset, "Movement Offset", nullptr, nullptr );
	FBPropertyPublish( this, CatchOffset, "Catch Offset", nullptr, SkyBox_CatchOffset );

	FBPropertyPublish( this, LayerId, "Layer Id", nullptr, nullptr );

	FBPropertyPublish( this, Transparency, "Transparency", nullptr, SetTransparencyProperty );
	FBPropertyPublish( this, TransparencyFactor, "Transparency Factor", nullptr, nullptr );

	FBPropertyPublish( this, DepthTest, "Depth Test", nullptr, nullptr );

	//FBPropertyPublish( this, LogarithmicDepth, "Accurate Depth", nullptr, ProjTex_SetLogarithmicDepth );
	//FBPropertyPublish( this, DepthDisplacement, "Depth Displacement", nullptr, nullptr );
	/*
	FBPropertyPublish(	this, Brightness, "Brightness" ,NULL, NULL);
	FBPropertyPublish(	this, Saturation, "Saturation" ,NULL, NULL);
	FBPropertyPublish(	this, Contrast, "Contrast" ,NULL, NULL);
	FBPropertyPublish(	this, Gamma, "Gamma" ,NULL, NULL);
	*/
	
	CubeMap.SetSingleConnect(true);
	CubeMap.SetFilter( FBUserObject::GetInternalClassId() );

	FlipHorizontal = true;
	FlipVertical = false;
	
	DepthTest = true;

	UVOffset = FBVector3d(0.0, 0.0, 0.0);
	UVScale = FBVector3d(100.0, 100.0, 100.0);

	WorldOffset = FBVector3d(0.0, 0.0, 0.0);
	WorldScale = FBVector3d(100.0, 100.0, 100.0);

	MovementAffect = FBVector3d(0.0, 0.0, 0.0);
	MovementOffset = FBVector3d(0.0, 0.0, 0.0);

	LayerId = 0;
	LayerId.SetMinMax( 0.0, 10.0, true, true );

	Transparency = kFBAlphaSourceNoAlpha;
	TransparencyFactor = 100.0;

	TransparencyFactor.SetMinMax( 0.0, 100.0, true, true );

	
	
	LogarithmicDepth = false;
	DepthDisplacement = 0.0;

	Brightness = 0.0;
	Brightness.SetMinMax(-100.0, 100.0, true, true);
	Saturation = 0.0;
	Saturation.SetMinMax(-100.0, 100.0, true, true);
	Contrast = 0.0;
	Contrast.SetMinMax(-100.0, 100.0, true, true);
	Gamma = 1.0/2.2*100.0;
	Gamma.SetMinMax(-300.0, 300.0, true, true);

	//
	//

	RenderingPass = kFBPassPreRender;
	//SetShaderCapacity(kFBShaderCapacityMaterialEffect, false);
	//SetShaderPassActionCallback( (FBShaderPassActionCallback) (kFBShaderPassInstanceBegin | kFBShaderPassInstanceEnd | kFBShaderPassMaterialBegin | kFBShaderPassMaterialEnd) );

	needSetup = true;
	needReload = false;

	return true;
}



/************************************************
*	FiLMBOX Destructor.
************************************************/
void ORShaderSkyBox::FBDestroy()
{
	if (mShader != nullptr)
	{
		delete mShader;
		mShader = nullptr;
	}
}


void ORShaderSkyBox::ShaderPassInstanceBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
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
		glActiveTexture(GL_TEXTURE0);
		pTexture->OGLInit(pRenderOptions);
	}
}

void ORShaderSkyBox::ShaderPassMaterialEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo)
{
	glDisable(GL_TEXTURE_2D);
}

/** Does the shader need a begin render call.
*	\remark	Re-implement this function and return true if you need it. This method is called once per shader on each render pass.
*/
bool ORShaderSkyBox::ShaderNeedBeginRender()
{
	return true;
}

/** Pre-rendering of shaders that is called only one time for a shader instance.
*	\remark	ShaderBeginRender is called as many times as the number of models affected 
*			by the instance of the shader in a given render pass. (ShaderNeedBeginRender 
*			must return true for this function to be called).
*/
void ORShaderSkyBox::ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	needRender = true;
}

void ORShaderSkyBox::ShadeModel( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass )
{
	if (needRender == false) return;

	mCullFace = glIsEnabled(GL_CULL_FACE);
	glGetIntegerv( GL_CULL_FACE_MODE, &mCullFaceMode );

	FBModelCullingMode cullMode = pShaderModelInfo->GetFBModel()->GetCullingMode();

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

	// render all skyboxe layers here

	FBSystem	lSystem;
	FBScene		*mScene = lSystem.Scene;

	int maxIndex = 0;

	std::vector<ORShaderSkyBox*> shaders;

	for (int i=0; i<mScene->Shaders.GetCount(); ++i)
	{
		if (FBIS( mScene->Shaders[i], ORShaderSkyBox) )
		{
			ORShaderSkyBox *pSkyShader = (ORShaderSkyBox*) mScene->Shaders[i];
			maxIndex = std::max(maxIndex, pSkyShader->LayerId.AsInt());
			
			shaders.push_back( (ORShaderSkyBox*) mScene->Shaders[i] );
		}
	}

	// DONE: RENDER ALL SHADER MODELS, not only current one

	for (int i=0; i<=maxIndex; ++i)
	{
		for (auto iter=shaders.begin(); iter!=shaders.end(); ++iter)
		{
			ORShaderSkyBox *pShader = *iter;

			if (pShader->LayerId == i)
			{
				const int dstCount = pShader->GetDstCount();

				for (int j=0; j<dstCount; ++j)
				{
					FBPlug *pPlug = pShader->GetDst(j);

					if (FBIS(pPlug, FBModel))
					{
						pShader->Render(pRenderOptions, pShaderModelInfo, pPass, (FBModel*) pPlug);
					}
				}

			}
		}
	}

	if (mCullFace == GL_TRUE) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	glCullFace(mCullFaceMode);
}

const int GetStrideFromArrayElementType( const FBGeometryArrayElementType format )
{
	int stride = 0;

	switch(format)
	{
	case kFBGeometryArrayElementType_Float2:
		stride = sizeof(float) * 2;
		break;
	case kFBGeometryArrayElementType_Float3:
		stride = sizeof(float) * 3;
		break;
	case kFBGeometryArrayElementType_Float4:
		stride = sizeof(float) * 4;
		break;
	default:
		printf( "> unsupported array element format!\n" );
	}

	return stride;
}

void ORShaderSkyBox::Render( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pInfo, FBRenderingPass pPass, FBModel *pModel )
{
	if (needReload)
	{
		if (mShader != nullptr)
		{
			delete mShader;
			mShader = nullptr;
		}
		needSetup = true;
		needReload = false;
	}

	if (needSetup)
	{
		//
		// load glslfx shader

		if (mShader == nullptr)
			mShader = new Shader();

		if (mShader)
			((Shader*)mShader)->Initialize();

		needSetup = false;
	}

	if (DepthTest == false)
		glDepthMask (GL_FALSE);

	int cubeId = 0;
	
	if (CubeMap.GetCount() > 0)
	{
		FBComponent *pCubeMap = CubeMap.GetAt(0);
		FBProperty *pCubeMapId = pCubeMap->PropertyList.Find("CubeMap Id");
		if (nullptr != pCubeMapId)
			cubeId = pCubeMapId->AsInt();
	}

	if ( mShader && ((Shader*)mShader)->IsLoaded() && (cubeId > 0) && (pModel != nullptr) )
	{	
		FBVector3d	eyePos;
		FBMatrix cameraMV, cameraPRJ, m;

		vec3	v3_eye;
		mat4 m4_proj, m4_view, m4_view2, m4_model, m4_mvp, m4_mvp2;

		pModel->GetMatrix(m);

		FBCamera *pCamera = pRenderOptions->GetRenderingCamera();
		if ( pCamera && FBIS(pCamera, FBCameraSwitcher) )
			pCamera = ((FBCameraSwitcher*) pCamera)->CurrentCamera;

		pCamera->GetCameraMatrix( cameraMV, kFBModelView );
		pCamera->GetCameraMatrix( cameraPRJ, kFBProjection );
		pCamera->GetVector(eyePos);

		FBVector3d lMovement = MovementAffect;
		FBVector3d lMovementOffset = MovementOffset;
		/*
		cameraMV[12] *= lMovement[0] * 0.01;
		cameraMV[13] *= lMovement[1] * 0.01;
		cameraMV[14] *= lMovement[2] * 0.01;
		*/
		v3_eye = vec3( (float)eyePos[0], (float)eyePos[1], (float)eyePos[2] );

		for (int i=0; i<16; ++i)
		{
			m4_proj.mat_array[i] = (float) cameraPRJ[i];
			m4_view.mat_array[i] = (float) cameraMV[i];
			m4_view2.mat_array[i] = (float) cameraMV[i];
			m4_model.mat_array[i] = (float) m[i];
		}

		//m4_model.identity();
		/*
		vec3 tr;
		m4_view.get_translation(tr);
		tr.x = (tr.x + lMovementOffset[0]) * lMovement[0] * 0.01;
		tr.y = (tr.y + lMovementOffset[1]) * lMovement[1] * 0.01;
		tr.z = (tr.z + lMovementOffset[2]) * lMovement[2] * 0.01;
		m4_view.set_translation( vec3(0.0f, 0.0f, 0.0f) );
		m4_model.set_translation(tr);
		*/
		m4_mvp = m4_proj * m4_view * m4_model;
		m4_mvp2 = m4_proj * m4_view2;

		FBVector3d offset, scale, woffset, wscale;
		offset = UVOffset;
		scale = UVScale;

		woffset = WorldOffset;
		wscale = WorldScale;

		double transFactor = TransparencyFactor;
		if (Transparency == kFBAlphaSourceNoAlpha)
			transFactor = 100.0;

		((Shader*)mShader)->Set(	m4_mvp.mat_array, 
						m4_view.mat_array, 
						v3_eye.vec_array, 
						(FlipHorizontal==true) ? 1.0f : -1.0f,
						(FlipVertical==true) ? 1.0f : -1.0f,
						offset,
						scale,
						woffset,
						wscale,
						transFactor );


		((Shader*)mShader)->BindBox();

		if (Transparency != kFBAlphaSourceNoAlpha)
		{
			glEnable(GL_BLEND);
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}

		

		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_CUBE_MAP, cubeId);
		//glBindVertexArray (vao);

		glBindVertexArray (0);

		//glDrawArrays (GL_TRIANGLES, 0, 36);
		
		//FBModel *pModel = pShaderModelInfo->GetFBModel();
		FBModelVertexData *pVertexData = pModel->ModelVertexData;
		
		if (pVertexData)
		{
			pVertexData->EnableOGLVertexData();
			
			const GLuint posId = pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Point );
			const GLuint norId = pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Normal );
			const GLuint elemId = pVertexData->GetIndexArrayVBOId();

			const int normalStride = GetStrideFromArrayElementType( pVertexData->GetVertexArrayType(kFBGeometryArrayID_Normal) );

			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, posId);
			glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) NULL);

			glBindBuffer(GL_ARRAY_BUFFER, norId);
			glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, normalStride, (const GLvoid*) NULL);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemId);

			const int numberOfRegions = pVertexData->GetSubRegionCount();
			for (int i=0; i<numberOfRegions; ++i)
			{
				pVertexData->DrawSubRegion(i);
			}
			
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			pVertexData->DisableOGLVertexData();
		}


		if (Transparency != kFBAlphaSourceNoAlpha)
		{
			glDisable(GL_BLEND);
		}
		
		((Shader*)mShader)->UnBindBox();
		
		glBindTexture (GL_TEXTURE_CUBE_MAP, 0);
	}

	//
	if (DepthTest == false)
			glDepthMask (GL_TRUE);

	//
	needRender = false;
}

bool ORShaderSkyBox::FbxStore	(FBFbxObject* pFbxObject)
{
	return true;
}

bool ORShaderSkyBox::FbxRetrieve(FBFbxObject* pFbxObject, FBRenderer* pRenderer)
{
	return true;
}

void ORShaderSkyBox::DoCatchOffset()
{
	FBCamera *pCamera = FBSystem::TheOne().Renderer->CurrentCamera;

	FBVector3d offset;
	pCamera->GetVector(offset);

	MovementOffset = offset;
}


void ORShaderSkyBox::DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	if (mShader)
		((Shader*)mShader)->clearResources();

	needSetup = true;
}

void ORShaderSkyBox::DoReloadShader()
{
	needReload = true;
}

void ORShaderSkyBox::SetTransparencyProperty( HIObject pObject, FBAlphaSource pState )
{     
    ORShaderSkyBox* lShader = FBCast<ORShaderSkyBox>(pObject);
    if (lShader->Transparency != pState)
    {
        lShader->Transparency.SetPropertyValue(pState);
        lShader->RenderingPass = GetRenderingPassNeededForAlpha(pState);
         
        // if shader use alpha and thus generate custom shape than the original geometry shape, 
        // we need to let it handle DrawShadow functiionality as well. 
        //lShader->SetShaderCapacity(kFBShaderCapacityDrawShadow, pState != kFBAlphaSourceNoAlpha); 
    }
}

void ORShaderSkyBox::SetReloadShaderAction( HIObject pObject, bool value )
{
	ORShaderSkyBox* lShader = FBCast<ORShaderSkyBox>(pObject);
    if (nullptr != lShader && true == value)
    {
        lShader->DoReloadShader();
    }
}