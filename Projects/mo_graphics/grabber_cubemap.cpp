
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: grabber_cubemap.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//-- 
#include <GL\glew.h>

#include "grabber_cubemap.h"
#include "nv_dds\nv_dds.h"
#include "graphics\Framebuffer.h"
#include "graphics\glslShader.h"
#include "graphics\CheckGLError_MOBU.h"
#include "graphics\ogl_utils.h"
#include "graphics\ParticlesDrawHelper.h"
#include "IO\FileUtils.h"

#include <vector>

CViewportGrabber::CViewportGrabber()
{
}

void CViewportGrabber::Process(FBCamera *&_pointerForRenderCamera)
{
	if (mGrabCubeMap && mOptions.filename != "")
	{
		RenderCubeMap(_pointerForRenderCamera);
		mGrabCubeMap = false;
	}
}


void CViewportGrabber::GrabViewportCubeMap( const char *filename, const int textureSize, const bool saveSeparateImages )
{
	mOptions.filename = filename;
	mOptions.imageSize = textureSize;
	mOptions.separateImages = saveSeparateImages;

	mGrabCubeMap = true;
}

void CViewportGrabber::RenderCubeMap(FBCamera *&_pointerForRenderCamera)
{
	const int imageWidth = mOptions.imageSize;
	const int imageHeight = mOptions.imageSize;

	FBScene *pScene = FBSystem::TheOne().Scene;
	FBCamera *pSceneCamera = pScene->Renderer->CurrentCamera;

	FBCamera *pCamera = new FBCamera("RenderCubeMap");
	FBRenderer *pRenderer = new FBRenderer(0);

	//
	_pointerForRenderCamera = pCamera;

	//
	FBVector3d v;
	pSceneCamera->GetVector(v);
	pCamera->SetVector(v);

	pCamera->FrameSizeMode = kFBFrameSizeFixedResolution;
	pCamera->ResolutionMode = kFBResolutionCustom;
	pCamera->ResolutionWidth = imageWidth;
	pCamera->ResolutionHeight = imageHeight;
	pCamera->FieldOfView = 90.0;
	pCamera->ViewShowAxis = false;
	pCamera->ViewShowGrid = false;
	pCamera->ViewShowName = false;
	
	pCamera->NearPlaneDistance = pSceneCamera->NearPlaneDistance;
	pCamera->FarPlaneDistance = pSceneCamera->FarPlaneDistance;
		
	//
	pRenderer->CurrentCamera = pCamera;
	FBViewingOptions *pViewingOptions = pRenderer->GetViewingOptions();

	pViewingOptions->PickingMode() = kFBPickingModeModelsOnly;
	pViewingOptions->ShadingMode() = kFBModelShadingAll;
	pViewingOptions->ShowTimeCode() = false;
	pViewingOptions->ShowCameraLabel() = false;
	pViewingOptions->ShowSafeArea() = false;
	
	pRenderer->SetViewingOptions(*pViewingOptions);

	pRenderer->AutoEvaluate = false;
		
	CHECK_GL_ERROR_MOBU();

	// we need 1 framebuffer and 6 textures

	FrameBuffer *mGrabFrameBuffer = new FrameBuffer(0, 0);
	mGrabFrameBuffer->SetFlag( FrameBuffer::eCreateDepthStencilRenderbuffer );
	mGrabFrameBuffer->ReSize( imageWidth, imageHeight );

	// +X, -X, +Y, -Y, +Z, -Z
	nv_dds::CTexture cubemapTextures[6];
	
	glEnable(GL_DEPTH_TEST);

	// render 6 times for each cubemap face
	// 1 - orient camera to direction (+X, -X, +Y, -Y, +Z, -Z)
	// 2 - assign output texture
	// 3 - render
	// 4 - grab texture data and fill-up one cubemap face

	FBVector3d angles[6] = { FBVector3d(0.0, 0.0, 0.0),
								FBVector3d(0.0, 180.0, 0.0),
								FBVector3d(0.0, 0.0, -90.0),
								FBVector3d(0.0, 0.0, 90.0),
								FBVector3d(0.0, -90.0, 0.0),
								FBVector3d(0.0, 90.0, 0.0) };

	bool isCompressed;
	int width, height, internalFormat, format, pixelMemorySize, imageSize, numberOfLods;

	pixelMemorySize = 3;
	format = GL_RGB;

	int imgsize=0;
	GLubyte *pixels = nullptr;

	for (int i=0; i<6; ++i)
	{
		// setup camera
		pCamera->SetVector( angles[i], kModelRotation );
		pCamera->EvaluateAnimationNodes(0);
		pScene->Evaluate();
		pRenderer->CurrentCamera = pCamera;

		mGrabFrameBuffer->Bind();

		glEnable(GL_DEPTH_TEST);

		if(pRenderer->RenderBegin(0, 0, imageWidth, imageHeight))
		{
			pRenderer->PreRender();
			pRenderer->Render();
			//mRenderer->RenderEnd();
		}
	
		mGrabFrameBuffer->UnBind();

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		const GLuint id = mGrabFrameBuffer->GetColorObject();

		glBindTexture(GL_TEXTURE_2D, id);

		TextureObjectGetInfo(GL_TEXTURE_2D, width, height, internalFormat, format, isCompressed, pixelMemorySize, imageSize, numberOfLods);

		if (imageSize > 0)
		{
			
			// allocate mem
			if (imageSize != imgsize)
			{
				if (pixels)
				{
					delete [] pixels;
					pixels = nullptr;
				}

				pixels = new GLubyte[imageSize];
				imgsize = imageSize;
			}

			TextureObjectGetData(GL_TEXTURE_2D, pixels, width, height, internalFormat, format, isCompressed, pixelMemorySize, imageSize, numberOfLods);
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		if (imgsize > 0 && pixels != nullptr)
		{
			cubemapTextures[i].create(imageWidth, imageHeight, pixelMemorySize, imgsize, pixels);
			
		}
	}

	//
	//
	//
	
	nv_dds::CDDSImage ddsImage;
	ddsImage.create_textureCubemap( format, pixelMemorySize, cubemapTextures[0], cubemapTextures[1], cubemapTextures[2], 
		cubemapTextures[3], cubemapTextures[4], cubemapTextures[5] );

	ddsImage.save( std::string(mOptions.filename), false );

	if (mOptions.separateImages)
	{
		// find the main part of a string
		FBString filename = mOptions.filename;

		int idx = filename.ReverseFind('.');
		if (idx > 0)
			filename = filename.Left(idx);

		std::string str = filename;

		nv_dds::CDDSImage flatImage[6];

		const char *names[6] = {
			"_posX.dds",
			"_negX.dds",
			"_posY.dds",
			"_negY.dds",
			"_posZ.dds",
			"_negZ.dds"
							};

		for (int i=0; i<6; ++i)
		{
			flatImage[i].create_textureFlat( format, pixelMemorySize, cubemapTextures[i] );
			flatImage[i].save( str + std::string(names[i]), false );
		}
	}

	/// 
	delete pRenderer;
	delete pCamera;

	_pointerForRenderCamera = nullptr;

	if (mGrabFrameBuffer)
	{
		delete mGrabFrameBuffer;
		mGrabFrameBuffer = nullptr;
	}

	if (pixels)
	{
		delete [] pixels;
		pixels = nullptr;
	}
}

bool CViewportGrabber::MakePanorama( const char *cubeMapFileName, const char *panoramaFileName, const int panoramaWidth, const int panoramaHeight )
{
	nv_dds::CDDSImage cubeImage;

	std::auto_ptr<FrameBuffer> pFrameBuffer( new FrameBuffer(0, 0) );
	if (pFrameBuffer.get() == nullptr)
		return false;

	std::auto_ptr<GLSLShader> pShader( new GLSLShader() );
	if (pShader.get() == nullptr)
		return false;

	try
	{

		if (false == cubeImage.load( cubeMapFileName, false ) )
			throw "Failed to load cubemap";

		if (panoramaWidth < 1 || panoramaHeight < 1 || panoramaWidth > 16192 || panoramaHeight > 16192)
			throw "Wrong panorama sizes";

		pFrameBuffer->ReSize( panoramaWidth, panoramaHeight );
		if (false == pFrameBuffer->IsOk() )
			throw "Failed to initialize a framebuffer";

		FBString effectPath, effectFullName;
		CHECK_GL_ERROR_MOBU();

		if ( false == FindEffectLocation( "\\GLSL\\cube2Pan.fsh", effectPath, effectFullName ) )
			throw "Failed to locate shader files";
		
		if ( false == pShader->LoadShaders( FBString(effectPath, "\\GLSL\\simple.vsh"), FBString(effectPath, "\\GLSL\\cube2Pan.fsh") ) )
			throw "Failed to load shaders";
	}
	catch (const char *message)
	{
		FBMessageBox( "Convert to panorama", message, "Ok" );
		return false;
	}


	//
	/// prepare cubemap texture object
	//

	GLuint cubeId = 0;
	glGenTextures(1, &cubeId);

	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeId);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	cubeImage.upload_textureCubemap();
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	//
	/// render
	//

	pFrameBuffer->Bind();

	glDisable(GL_DEPTH_TEST);

	pShader->Bind();

	pShader->bindTexture( GL_TEXTURE_CUBE_MAP, "cubeMapSampler", cubeId, 0 );

	// draw quad
	drawOrthoQuad2d(panoramaWidth, panoramaHeight);

	pShader->UnBind();

	pFrameBuffer->UnBind();

	
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//
	/// write result to the disc
	//

	bool isCompressed;
	int width, height, internalFormat, format, pixelMemorySize, imageSize, numberOfLods;

	const GLuint id = pFrameBuffer->GetColorObject();

	glBindTexture(GL_TEXTURE_2D, id);

	TextureObjectGetInfo(GL_TEXTURE_2D, width, height, internalFormat, format, isCompressed, pixelMemorySize, imageSize, numberOfLods);

	std::vector<GLubyte>	pixels;

	if (imageSize > 0)
	{
		pixels.resize(imageSize);	
		TextureObjectGetData(GL_TEXTURE_2D, pixels.data(), width, height, internalFormat, format, isCompressed, pixelMemorySize, imageSize, numberOfLods);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	nv_dds::CDDSImage flatImage;
	nv_dds::CTexture panTexture(width, height, pixelMemorySize, imageSize, pixels.data() );

	flatImage.create_textureFlat( format, pixelMemorySize, panTexture );
	flatImage.save( panoramaFileName, false );

	return true;
}