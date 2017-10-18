
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Panorama_shader.cxx
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
#include "Panorama_shader.h"
#include <GL\glew.h>

#include <Windows.h>
#include <CommDlg.h>

#include <vector>


//--- Registration defines
#define ORSHADERPANORAMA__CLASS		ORSHADERPANORAMA__CLASSNAME
#define ORSHADERPANORAMA__DESC		"Panorama"


//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	ORSHADERPANORAMA__CLASS	);
FBRegisterShader	(	ORSHADERPANORAMA__DESCSTR,
					 ORSHADERPANORAMA__CLASS,
					 ORSHADERPANORAMA__DESCSTR,
					 ORSHADERPANORAMA__DESC,
					 FB_DEFAULT_SDK_ICON		);

////////////////////////////////////////////////////////////

static void Panorama_Clear(HIObject object, bool pValue)
{
	ORShaderPanorama *shader = FBCast<ORShaderPanorama>(object);
	if (shader && pValue) 
	{
		shader->DoClear();
	}
}

static void Panorama_Load(HIObject object, bool pValue)
{
	ORShaderPanorama *shader = FBCast<ORShaderPanorama>(object);
	if (shader && pValue) 
	{
		shader->DoLoad(true);
	}
}

static void Panorama_CatchOffset(HIObject object, bool pValue)
{
	ORShaderPanorama *shader = FBCast<ORShaderPanorama>(object);
	if (shader && pValue) 
	{
		shader->DoCatchOffset();
	}
}

static void Panorama_Grab(HIObject object, bool pValue)
{
	ORShaderPanorama *shader = FBCast<ORShaderPanorama>(object);
	if (shader && pValue) 
	{
		shader->DoGrab();
	}
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ORShaderPanorama::FBCreate()
{

	FBPropertyPublish( this, Clear, "Clear", nullptr, Panorama_Clear );
	FBPropertyPublish( this, Load, "Load", nullptr, Panorama_Load );
	
	FBPropertyPublish( this, Media, "Media", nullptr, nullptr );
	
	FBPropertyPublish( this, FlipHorizontal, "Flip Horizontal", nullptr, nullptr );
	FBPropertyPublish( this, FlipVertical, "Flip Vertical", nullptr, nullptr );

	FBPropertyPublish( this, UVOffset, "UV Offset", nullptr, nullptr );
	FBPropertyPublish( this, UVScale, "UV Scale", nullptr, nullptr );

	FBPropertyPublish( this, WorldOffset, "World Offset", nullptr, nullptr );
	FBPropertyPublish( this, WorldScale, "World Scale", nullptr, nullptr );

	FBPropertyPublish( this, MovementAffect, "Movement Affect", nullptr, nullptr );
	FBPropertyPublish( this, MovementOffset, "Movement Offset", nullptr, nullptr );
	FBPropertyPublish( this, CatchOffset, "Catch Offset", nullptr, Panorama_CatchOffset );

	FBPropertyPublish( this, LayerId, "Layer Id", nullptr, nullptr );

	FBPropertyPublish( this, Transparency, "Transparency", nullptr, nullptr );
	FBPropertyPublish( this, TransparencyFactor, "Transparency Factor", nullptr, nullptr );


	FBPropertyPublish( this, LocationLat, "Location latitude", nullptr, nullptr );
	FBPropertyPublish( this, LocationLng, "Location longitude", nullptr, nullptr );
	FBPropertyPublish( this, Zoom, "Zoom", nullptr, nullptr );
	FBPropertyPublish( this, Grab, "Grab", nullptr, Panorama_Grab );

	FlipHorizontal = true;
	FlipVertical = false;
	
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

	Media.SetFilter( FBVideoClip::GetInternalClassId() );
	Media.SetSingleConnect(true);
	
	LocationLat = 31.731037;
	LocationLng = 35.234158;

	Zoom = 3;
	Zoom.SetMinMax(1.0, 5.0, true, true);
	
	//
	//
	
	RenderingPass = kFBPassPreRender;
	mVao = 0; 

	FBApplication lApplication;
	FBFbxOptions lOptions(true);
	lOptions.UpdateRecentFiles = false;
	lOptions.Models = kFBElementActionMerge;

	mModel = nullptr;
	//lApplication.FileMerge( "C:\\Work\\SKY_BOX_DOME.fbx", false, &lOptions );
	//mModel = FBFindModelByLabelName( "Sphere" );
	//mModel->Show = false;
	
	//mModel = FBLoadFbxPrimitivesModel("C:\\Work\\SKY_BOX_DOME.fbx");
	PrepareModelVAO();

	needLoad = false;
	needSetup = true;

	return true;
}

void ORShaderPanorama::PrepareModelVAO()
{
	if (mModel == nullptr) return;
	if (mVao > 0) 
	{
		glDeleteVertexArrays(1, &mVao);
		mVao = 0;
	}

	FBModelVertexData *pVertexData = mModel->ModelVertexData;
	GLuint posId = pVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Point, true );
	int posStride = sizeof(FBVertex);
	GLuint indId = pVertexData->GetIndexArrayVBOId();
	mNumberOfVertices = pVertexData->GetVertexCount();

	glGenVertexArrays(1, &mVao);

	glBindVertexArray(mVao);

	glBindBuffer( GL_ARRAY_BUFFER, posId );
	glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indId );

	glBindVertexArray(0);
}

/************************************************
*	FiLMBOX Destructor.
************************************************/
void ORShaderPanorama::FBDestroy()
{
	if (mVao > 0) {
		glDeleteVertexArrays(1, &mVao);
		mVao = 0;
	}
}

/** Does the shader need a begin render call.
*	\remark	Re-implement this function and return true if you need it. This method is called once per shader on each render pass.
*/
bool ORShaderPanorama::ShaderNeedBeginRender()
{
	return true;
}

/** Pre-rendering of shaders that is called only one time for a shader instance.
*	\remark	ShaderBeginRender is called as many times as the number of models affected 
*			by the instance of the shader in a given render pass. (ShaderNeedBeginRender 
*			must return true for this function to be called).
*/
void ORShaderPanorama::ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	needRender = true;
}

void ORShaderPanorama::ShadeModel( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass )
{
	if (needRender == false) return;

	return;

	// render all skyboxe layers here

	FBSystem	lSystem;
	FBScene		*mScene = lSystem.Scene;

	int maxIndex = 0;

	std::vector<ORShaderPanorama*> shaders;

	for (int i=0; i<mScene->Shaders.GetCount(); ++i)
	{
		if (FBIS( mScene->Shaders[i], ORShaderPanorama) )
		{
			ORShaderPanorama *pSkyShader = (ORShaderPanorama*) mScene->Shaders[i];
			maxIndex = std::max(maxIndex, pSkyShader->LayerId.AsInt() );
			
			shaders.push_back( (ORShaderPanorama*) mScene->Shaders[i] );
		}
	}

	for (int i=0; i<=maxIndex; ++i)
	{
		for (auto iter=shaders.begin(); iter!=shaders.end(); ++iter)
		{
			if ( (*iter)->LayerId == i)
				(*iter)->Render(pRenderOptions, pShaderModelInfo, pPass);
		}
	}
}

void ORShaderPanorama::Render( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass )
{
	if (needSetup)
	{
		//
		// load glslfx shader
		mShader.Initialize();

		needSetup = false;
	}

	if (needLoad)
	{
		DoLoad(false);
		needLoad = false;
	}

	FBVideoClip *pClip = nullptr;

	if (Media.GetCount() )
		if (FBIS( Media.GetAt(0), FBVideoClip ) )
			pClip = (FBVideoClip*) Media.GetAt(0);

	const GLuint texId = (pClip) ? pClip->GetTextureID() : 0;

	if ( mShader.IsLoaded() && texId > 0 && mVao > 0 )
	{	
		FBVector3d	eyePos;
		FBMatrix cameraMV, cameraPRJ, m;

		vec3	v3_eye;
		mat4 m4_proj, m4_view, m4_view2, m4_model, m4_mvp, m4_mvp2;

		pShaderModelInfo->GetFBModel()->GetMatrix(m);

		FBCamera *pCamera = pRenderOptions->GetRenderingCamera();
		if ( FBIS(pCamera, FBCameraSwitcher) )
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

		m4_model.identity();

		vec3 tr;
		m4_view.get_translation(tr);
		tr.x = (tr.x + lMovementOffset[0]) * lMovement[0] * 0.01;
		tr.y = (tr.y + lMovementOffset[1]) * lMovement[1] * 0.01;
		tr.z = (tr.z + lMovementOffset[2]) * lMovement[2] * 0.01;
		m4_view.set_translation( vec3(0.0f, 0.0f, 0.0f) );
		m4_model.set_translation(tr);

		m4_mvp = m4_proj * m4_model * m4_view;
		m4_mvp2 = m4_proj * m4_view2;
		
		if (mShader.fx_MVP) mShader.fx_MVP->setMatrix4f(m4_mvp.mat_array);
		if (mShader.fx_ModelView) mShader.fx_ModelView->setMatrix4f(m4_view.mat_array);
		if (mShader.fx_EyePos) mShader.fx_EyePos->setValue3f(v3_eye.x, v3_eye.y, v3_eye.z);

		if (mShader.fx_FlipH) mShader.fx_FlipH->setValue1f( (FlipHorizontal==true) ? 1.0f : -1.0f );
		if (mShader.fx_FlipV) mShader.fx_FlipV->setValue1f( (FlipVertical==true) ? 1.0f : -1.0f );

		FBVector3d offset, scale;
		offset = UVOffset;
		scale = UVScale;

		if (mShader.fx_UVOffset) mShader.fx_UVOffset->setValue3f( offset[0], offset[1], offset[2] );
		if (mShader.fx_UVScale) mShader.fx_UVScale->setValue3f( scale[0] * 0.01, scale[1] * 0.01, scale[2] * 0.01 );

		offset = WorldOffset;
		scale = WorldScale;

		if (mShader.fx_WorldOffset) mShader.fx_WorldOffset->setValue3f( offset[0], offset[1], offset[2] );
		if (mShader.fx_WorldScale) mShader.fx_WorldScale->setValue3f( scale[0] * 0.01, scale[1] * 0.01, scale[2] * 0.01 );

		mShader.fx_passPano->execute();

		if (Transparency != kFBAlphaSourceNoAlpha)
		{
			glEnable(GL_BLEND);
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}

		glDepthMask (GL_FALSE);
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, texId);
	
		glEnableVertexAttribArray(0);
		glBindVertexArray( mVao );

		glDrawElements( GL_TRIANGLES, mNumberOfVertices * 3, GL_UNSIGNED_INT, nullptr );

		if (Transparency != kFBAlphaSourceNoAlpha)
		{
			glDisable(GL_BLEND);
		}
		
		
		mShader.fx_passPano->unbindProgram();
		
		/*
		if (fx_MVP) fx_MVP->setMatrix4f(m4_mvp2.mat_array);

		fx_passPlane->execute();

		glDrawArrays(GL_TRIANGLES, 36, 6);

		fx_passPlane->unbindProgram();
		*/
		glDepthMask (GL_TRUE);
		glBindVertexArray(0);
		glDisableVertexAttribArray(0);
	}


	//
	needRender = false;
}

bool ORShaderPanorama::FbxStore	(FBFbxObject* pFbxObject)
{
	return true;
}

bool ORShaderPanorama::FbxRetrieve(FBFbxObject* pFbxObject, FBRenderer* pRenderer)
{
	needLoad = true;

	return true;
}

void ORShaderPanorama::DoClear()
{
	
	if (1 == FBMessageBox( "SkyBox Shader", "Do you want to remove used video clip ?", "Ok", "Cancel" ) )
	{
		FBVideoClip *media= (Media.GetCount() > 0) ? (FBVideoClip*)Media.GetAt(0) : nullptr;
		

		if (media) media->FBDelete();
		
	}

	Media.Clear();
	
}

void ORShaderPanorama::DoGrab()
{
	FBFolderPopup	lDialog;
	lDialog.Caption = "Please choose a path for saving a panorama images";

	if (lDialog.Execute() )
	{
		CPanorama	panorama;
		panorama.Load( lDialog.Path, LocationLat, LocationLng, Zoom );
	}
}

void ORShaderPanorama::DoLoad(const bool showDialog)
{
	
	FBVideoClip *media= (Media.GetCount() > 0) ? (FBVideoClip*)Media.GetAt(0) : nullptr;
	
	if (media==nullptr)
	{
		// TODO:
	}
	
	//
	//
	if (media!=nullptr)
	{
		// TODO:
	}
	
}


void ORShaderPanorama::DoCatchOffset()
{
	FBCamera *pCamera = FBSystem::TheOne().Renderer->CurrentCamera;

	FBVector3d offset;
	pCamera->GetVector(offset);

	MovementOffset = offset;
}


void ORShaderPanorama::DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	mShader.clearResources();

	needSetup = true;
	needLoad = true;
}