
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: bakeProjectors_viewData.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "bakeProjectors_viewData.h"

#define UBERSHADER_PATH				"\\GLSL_FX\\"
#define UBERSHADER_EFFECT			"ProjectiveBaking.glslfx"

#include "StringUtils.h"
#include "shared_projectors.h"
#include "IO\FileUtils.h"
#include "..\..\jpeg-compressor\jpge.h"

/*
source: DEBUG_SOURCE_X where X may be API, 
SHADER_COMPILER, WINDOW_SYSTEM, 
THIRD_PARTY, APPLICATION, OTHER
type: DEBUG_TYPE_X where X may be ERROR, 
MARKER, OTHER, DEPRECATED_BEHAVIOR, 
UNDEFINED_BEHAVIOR, PERFORMANCE, 
PORTABILITY, {PUSH, POP}_GROUP
severity: DEBUG_SEVERITY_{HIGH, MEDIUM}, 
DEBUG_SEVERITY_{LOW, NOTIFICATION}
*/
void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
		printf( ">> ERROR!\n" );
	printf( "debug message - %s\n", message );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
ViewBakeProjectorsData::ViewBakeProjectorsData()
{
	mLastContext = 0;
	glewInit();

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif

	mReadyToLoad = true;
	mUberShader = nullptr;

	mOnlyProjectors = nullptr;
}

ViewBakeProjectorsData::~ViewBakeProjectorsData()
{
	if (mUberShader)
	{
		delete mUberShader;
		mUberShader = nullptr;
	}
}


void ViewBakeProjectorsData::PrepProjectors()
{

}

void ViewBakeProjectorsData::ReSize(const int width, const int height)
{
	//mFramebuffer.ReSize(width, height);
}

void ViewBakeProjectorsData::ClearFrameBuffers()
{
	int count = (int)mFrameBuffers.size();
	for (int i=0; i<count; ++i)
	{
		if (mFrameBuffers[i] != nullptr)
		{
			delete mFrameBuffers[i];
			mFrameBuffers[i] = nullptr;
		}
	}
}

void ViewBakeProjectorsData::RenderToFramebuffers(bool &grabImage, FBString &grabImageName, const bool saveJpeg, const int jpegQuality, const bool grabWithFrameNumber, 
		const bool saveImagePerModel, const bool saveOnlyProjectors, FBCamera *pcamera, const FBColorAndAlpha &backcolor, bool &saveStatus)
{

	saveStatus = false;

	/// !!! check ogl context

	ChangeContext();

	//
	if (mReadyToLoad && mUberShader == nullptr)
	{
		// try to load

		FBString effectPath, effectFullName;

		if ( FindEffectLocation( FBString("\\GLSL_FX\\", UBERSHADER_EFFECT), effectPath, effectFullName ) )
		{
			mUberShader = new Graphics::ShaderEffect();
			if( !mUberShader->Initialize( FBString(effectPath, "\\GLSL_FX\\"), UBERSHADER_EFFECT, 512, 512, 1.0) )
			{
				mReadyToLoad = false;
				delete mUberShader;
				mUberShader = nullptr;

				FBTrace( "[BAKING TOOL]: Failed to initialize a baking shader!\n" );
			}
		}
		else
		{
			FBTrace( "[BAKING TOOL]: Failed to find a baking shader!\n" );

			mReadyToLoad = false;
			mUberShader = nullptr;
		}

		if (mUberShader)
		{
			mOnlyProjectors = mUberShader->FindUniform( "saveOnlyProjectors" );
			
			mUberShader->SetBindless(false);
			mUberShader->SetCubeMapRendering(false);
			mUberShader->SetLogarithmicDepth(false);
			mUberShader->SetTechnique( Graphics::eEffectTechniqueShading );

			const auto loc = mUberShader->GetCustomEffectShaderLocationsPtr();
			mLocTexture = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheTextures); // allTheTextures;
			mLocMaterial = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheMaterials); // allTheMaterials;
			mLocShader = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheShaders); // allTheShaders;
			mLocProjectors = loc->GetFragmentLocation(Graphics::eCustomLocationAllTheProjectors); // allTheProjectors;

			CHECK_GL_ERROR();
		}
	}

	//
	//

	const int numberOfModels = (int) mModels.size(); // Models.GetCount();
	if (mUberShader == nullptr || numberOfModels == 0)
		return;

	if (mOnlyProjectors)
		mOnlyProjectors->setValue1f( (saveOnlyProjectors == true) ? 1.0f : 0.0f );

	const auto loc = mUberShader->GetCustomEffectShaderLocationsPtr();

	// this matrix only for baking projection, fragment shader will use camera matrices

	if (pcamera == nullptr)
		return;

	CHECK_GL_ERROR();

	//
	TextureGLSL	defTexData;
	mBufferTexture.UpdateData( sizeof(TextureGLSL), 1, &defTexData );

	// prep data

	FBMatrix proj, modl, viewIT;
	double proj2d[16];
	FBVector3d pos;

	pcamera->GetCameraMatrix(proj, kFBProjection);
	pcamera->GetCameraMatrix(modl, kFBModelView);
	pcamera->GetVector(pos);
	double farPlane = pcamera->FarPlaneDistance;

	FBMatrixInverse(modl, viewIT);
	viewIT.Transpose();

	//
	// prepare framebuffers
	ClearFrameBuffers();
	mFrameBuffers.resize( numberOfModels );

	for (int i=0; i<numberOfModels; ++i)
	{
		FrameBuffer *pNewBuffer = new FrameBuffer(1, 1);
		pNewBuffer->ReSize( mModels[i].width, mModels[i].height );

		mFrameBuffers[i] = pNewBuffer;
	}

	// render to view

	SaveFrameBuffer( &mFramebufferInfo );

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	PrepareOrthoView(1.0, 1.0);
	glGetDoublev(GL_MODELVIEW_MATRIX, proj2d); // orthoview stores projection in modelview matrix
	
	CHECK_GL_ERROR();
	
	CCameraInfoCache cameraCache;

	for (int i=0; i<16; ++i)
	{
		cameraCache.mv[i] = modl[i];

		cameraCache.mv4.mat_array[i] = (float) modl[i];
		cameraCache.p4.mat_array[i] = (float) proj[i];
		cameraCache.proj2d.mat_array[i] = (float) proj2d[i];
		cameraCache.mvInv4.mat_array[i] = (float) viewIT[i];
	}

	cameraCache.width = 1024;
	cameraCache.height = 1024;
	cameraCache.farPlane = (float) farPlane;
	cameraCache.nearPlane = 1.0f;

	for (int i=0; i<3; ++i)
		cameraCache.pos.vec_array[i] = pos[i];

	mUberShader->SetNumberOfProjectors(0);
	//mUberShader->UploadCameraUniforms(modl, proj, proj2d, viewIT, pos, 1024, 1024, (float)farPlane, nullptr);
	mUberShader->UploadCameraUniforms(cameraCache);

	mUberShader->Bind();
	
	// DONE: subroutines for blending modes
	// set subroutine values
	GLuint index[25];
	for (int i=0; i<25; ++i)
		index[i] = i;

	glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, GLsizei(25), &index[0] );

	for (int i=0; i<numberOfModels; ++i)
	{
		const int lwidth = mModels[i].width;
		const int lheight = mModels[i].height;

		cameraCache.width = lwidth;
		cameraCache.height = lheight;

		//mUberShader->UploadCameraUniforms(modl, proj, proj2d, viewIT, pos, lwidth, lheight, (float)farPlane, nullptr);
		mUberShader->UploadCameraUniforms(cameraCache);

		mFrameBuffers[i]->Bind();
		glViewport(0, 0, lwidth, lheight);

		if ( (i==0) || (i > 0 && saveImagePerModel == true && grabImage == true) )
		{
			glClearColor((float)backcolor[0], (float)backcolor[1], (float)backcolor[2], (float)backcolor[3]);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		FBModel *pModel = mModels[i].pModel; // (FBModel*) Models.GetAt(i);

		//
		FBMatrix m;
		pModel->GetMatrix(m);
		mUberShader->UploadModelTransform(m);

		CHECK_GL_ERROR();

		// TODO: setup shader and projectors

		bool hasProjectors = false;
		for (int j=0; j<pModel->Shaders.GetCount(); ++j)
		{
			FBShader *pShader = pModel->Shaders[j];

			if (CProjectors::ShaderHasProjectors( pShader ) )
			{
				hasProjectors = true;
				mProjectors.PrepFull(pShader);
			}
		}

		CHECK_GL_ERROR();
			 
		if (hasProjectors)
		{
			mUberShader->UpdateNumberOfProjectors( mProjectors.GetNumberOfProjectors() );
			mProjectors.Bind(loc->GetFragmentId(), mLocProjectors, 0);
		}
		else
		{
			mUberShader->UpdateNumberOfProjectors(0);
		}

		//
		RenderModel(pModel);
			

		if (hasProjectors)
			mProjectors.UnBind();

		CHECK_GL_ERROR();

		if (grabImage)
		{
			if (saveImagePerModel)
			{
				FBString str(ExtractFileName(grabImageName));
				FBString path(ExtractFilePath(grabImageName));
				FBString modelName = pModel->Name;

				ChangeFileExt( str, FBString("") );
				str = str + "_" + modelName;

				if (grabWithFrameNumber)
				{
					AddFrameNumber(str, true);
				}

				FBString fullFilename = path + "\\" + str;
				SavePixelsToFile(saveJpeg, jpegQuality, lwidth, lheight, fullFilename );

				saveStatus = true;
			}
			else
			{
				FBString fullFilename( grabImageName );
				ChangeFileExt( fullFilename, FBString("") );

				if (grabWithFrameNumber)
				{
					AddFrameNumber(fullFilename, true);
				}

				SavePixelsToFile(saveJpeg, jpegQuality, lwidth, lheight, fullFilename);

				saveStatus = true;
			}
		}

		mFrameBuffers[i]->UnBind();
	}

	mUberShader->UnBind(false);

	mBufferShader.UnBind();
	

	glBindTexture(GL_TEXTURE_2D, 0);
	/*
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	
	glColor3d(0.0, 1.0, 0.0);
	glBegin(GL_TRIANGLES);
		glVertex3d(0.0, 0.0, 0.0);
		glVertex3d(1.0, 0.0, 0.0);
		glVertex3d(0.0, 1.0, 0.0);
	glEnd();
	
	glColor3d(1.0, 0.0, 0.0);
	glPointSize(3.0f);
	glBegin(GL_POINTS);
		glVertex3d(0.0, 0.0, 0.0);
	glEnd();
	*/

	CHECK_GL_ERROR();
	
	grabImage = false;

	RestoreFrameBuffer( &mFramebufferInfo );
}

void ViewBakeProjectorsData::AddFrameNumber(FBString &str, bool simpleCat)
{
	FBTime currTime = mSystem.LocalTime;
	int frame = currTime.GetFrame();

	char buffer[64];
	memset(buffer, 0, sizeof(char) * 64);

	sprintf_s(buffer, 64, "_%06d", frame);

	if (simpleCat)
	{
		str = str + buffer;
	}
	else
	{
		FileNameAddSuffix( str, FBString(buffer) );
	}
}

void ViewBakeProjectorsData::SavePixelsToFile(const bool saveJpeg, const int jpegQuality, const int w, const int h, const char *imagename)
{
	glFinish();
		
	FBImage	newImage("");

	newImage.Init(kFBImageFormatRGBA32, w, h);
	unsigned char *buffer = newImage.GetBufferAddress();
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	if (saveJpeg)
	{
		newImage.VerticalFlip();

		jpge::params	params;
		params.m_quality = jpegQuality;

		jpge::compress_image_to_jpeg_file( FBString(imagename, ".jpg"), w, h, 4, buffer, params );
	}
	else
	{
		newImage.WriteToTif( FBString(imagename, ".tif"), "Bake Projectors Sample Grab", true );
	}
}

void ViewBakeProjectorsData::ChangeContext()
{
	HGLRC currRC = wglGetCurrentContext();

	if (mLastContext == 0)
	{
		mLastContext = currRC;
	}

	if (mLastContext != currRC)
	{
		mReadyToLoad = true;
		delete mUberShader;
		mUberShader = nullptr;

		ClearFrameBuffers();
	}

	mLastContext = currRC;
}



void ViewBakeProjectorsData::RenderModel(FBModel *pModel)
{
	const auto loc = mUberShader->GetCustomEffectShaderLocationsPtr();

	FBModelVertexData *pData = pModel->ModelVertexData;

	// needed for primitives indices
	pData->EnableOGLVertexData();

	const GLint positionId = pData->GetVertexArrayVBOId(kFBGeometryArrayID_Point);
	const GLint normalId = pData->GetVertexArrayVBOId(kFBGeometryArrayID_Normal );
	const GLint uvId = pData->GetUVSetVBOId();

	glBindBuffer( GL_ARRAY_BUFFER, positionId );
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0);
	glBindBuffer( GL_ARRAY_BUFFER, uvId );
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 ); 

	glBindBuffer( GL_ARRAY_BUFFER, normalId );
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 ); 


	glEnableVertexAttribArray(0);		// position
	glEnableVertexAttribArray(1);		// tex coords
	glEnableVertexAttribArray(2);		// normal

	const int regionCount = pData->GetSubRegionCount();
	for (int i=0; i<regionCount; ++i)
	{
		FBMaterial *pMaterial = pData->GetSubRegionMaterial(i);
		UploadMaterial(pMaterial);

		mBufferMaterial.BindAsUniform( loc->GetFragmentId(), mLocMaterial, 0 );
		mBufferTexture.BindAsUniform( loc->GetFragmentId(), mLocTexture, 0 );

		//
		pData->DrawSubRegion(i);

		mBufferTexture.UnBind();
		mBufferMaterial.UnBind();
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisableVertexAttribArray(0);		// position
	glDisableVertexAttribArray(1);		// tex coords
	glDisableVertexAttribArray(2);		// normal

	pData->DisableOGLVertexData();

	CHECK_GL_ERROR();
}

void ViewBakeProjectorsData::UploadShader(FBShader *pShader)
{
	ShaderGLSL		shaderdata;
	DefaultShader(shaderdata);
	mBufferShader.UpdateData(sizeof(ShaderGLSL), 1, &shaderdata);
}

void ViewBakeProjectorsData::UploadMaterial(FBMaterial *pMaterial)
{

	// TODO: upload material properties and bind textures !

	MaterialGLSL		matdata;
	DefaultMaterial(matdata);
	
	if (pMaterial->GetTexture() )
	{
		FBTexture *pTexture = pMaterial->GetTexture();

		matdata.useDiffuse = 1.0f;

		TextureGLSL	texdata;
		memset( &texdata, 0, sizeof(texdata) );

		FBMatrix m( pTexture->GetMatrix() );
		for (int i=0; i<16; ++i)
			texdata.transform.mat_array[i] = (float) m[i];

		mBufferTexture.UpdateData( sizeof(TextureGLSL), 1, &texdata );

		glBindTexture(GL_TEXTURE_2D, pTexture->TextureOGLId );
	}

	mBufferMaterial.UpdateData( sizeof(matdata), 1, &matdata );
}