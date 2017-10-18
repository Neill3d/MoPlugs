
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: graphics_framebuffer.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "graphics_framebuffer.h"

////////////////////////////////////////////////////////////
//

#define RESAMPLING_LINEAR_VERTEX		"\\GLSL\\simple.vsh"
#define RESAMPLING_LINEAR_FRAGMENT		"\\GLSL\\downsample.fsh"
#define RESAMPLING_KERNEL_FRAGMENT		"\\GLSL\\downsample3.fsh"
#define RESAMPLING_LANCZOS_VERTEX		"\\GLSL\\downsampleLanczos.vsh"
#define RESAMPLING_LANCZOS_FRAGMENT		"\\GLSL\\downsampleLanczos.fsh"


///////////////////////////////////////////////////////////////////////////////////////////////////
//

extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);

/*
const GLuint AttachedFrameBufferData::prepTexture(GLuint id, int width, int height, GLenum internalFormat, GLenum format, GLenum type)
{
	if (id == 0)
		return 0;

	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
			
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			
	glBindTexture(GL_TEXTURE_2D, 0);

	CHECK_GL_ERROR_MOBU();

	return id;
}
*/




//////////////////////////////////////////////////////////////////////////////////////////////////////////
// MainFrameBuffer::TexturesPack

MainFrameBuffer::texture_pack::texture_pack()
{
	color_texture = 0;
	depth_texture = 0;
	stencil_texture = 0;
	normal_texture = 0;
	mask_texture = 0;
	position_texture = 0;
}

MainFrameBuffer::texture_pack::~texture_pack()
{
	deleteTextures();
}

void MainFrameBuffer::texture_pack::deleteTextures()
{
	if (color_texture > 0)
	{
		glDeleteTextures(1, &color_texture);
		color_texture = 0;
	}
	if (depth_texture > 0)
	{
		glDeleteTextures(1, &depth_texture);
		depth_texture = 0;
	}
	if (stencil_texture > 0)
	{
		glDeleteTextures(1, &stencil_texture);
		stencil_texture = 0;
	}
	if (normal_texture > 0)
	{
		glDeleteTextures(1, &normal_texture);
		normal_texture = 0;
	}
	if (mask_texture > 0)
	{
		glDeleteTextures(1, &mask_texture);
		mask_texture = 0;
	}
	if (position_texture > 0)
	{
		glDeleteTextures(1, &position_texture);
		position_texture = 0;
	}

	// additional textures
	if (backdepth_texture > 0)
	{
		glDeleteTextures(1, &backdepth_texture);
		backdepth_texture = 0;
	}
	if (tcolor_texture > 0)
	{
		glDeleteTextures(1, &tcolor_texture);
		tcolor_texture = 0;
	}
	if (tdepth_texture > 0)
	{
		glDeleteTextures(1, &tdepth_texture);
		tdepth_texture = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MainFrameBuffer

MainFrameBuffer::MainFrameBuffer(const bool addontextures)
	: UseAddOnTextures( addontextures )
{
	InitialValues();
}

void MainFrameBuffer::InitTextureInternalFormat()
{
	colorInternalFormat = GL_RGBA8;
	colorFormat = GL_RGBA;
	colorType = GL_UNSIGNED_BYTE;

	// TODO: is depth component32F supported !!
	depthAttachment = GL_DEPTH_ATTACHMENT;
	depthInternalFormat = GL_DEPTH_COMPONENT32F;
	depthFormat = GL_DEPTH_COMPONENT;
	depthType = GL_FLOAT;

	/*
	depthAttachment = GL_DEPTH_STENCIL_ATTACHMENT;
	depthInternalFormat = GL_DEPTH32F_STENCIL8;
	depthFormat = GL_DEPTH_STENCIL;
	depthType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
	*/
	stencilInternalFormat = GL_STENCIL_INDEX8;
	stencilFormat = GL_STENCIL_COMPONENTS;
	stencilType = GL_UNSIGNED_BYTE;

	maskInternalFormat = GL_RGBA8;
	maskFormat = GL_RGBA;
	maskType = GL_UNSIGNED_BYTE;

	normalFormat = GL_RGBA;	// TODO: why RGBA and not RGB ?!
	normalType = GL_FLOAT;
	GLenum format = fpRgbaFormatWithPrecision();
	if (format == 0)
	{
		//FBMessageBox( "Composite GBuffer", "Floating texture format is not supported by your graphic card!", "Ok" ); 
		normalInternalFormat = GL_RGBA8;
	}
	else
	{
		normalInternalFormat = format;
	}

	positionFormat = GL_RGBA;
	positionType = GL_FLOAT;
	positionInternalFormat = fpRgbaFormatWithPrecision();
}

void MainFrameBuffer::InitialValues()
{
	fbo_attached = 0;
	fbobig = 0;
	fboms = 0;
	fbo = 0;
}

void MainFrameBuffer::DeleteFBO()
{
	if (fbobig > 0)
	{
		glDeleteFramebuffers(1, &fbobig);
		fbobig = 0;
	}
	if (fboms > 0)
	{
		glDeleteFramebuffers(1, &fboms);
		fboms = 0;
	}
	if (fbo > 0)
	{
		glDeleteFramebuffers(1, &fbo);
		fbo = 0;
	}
}

void MainFrameBuffer::DeleteTextures()
{
	textures.deleteTextures();
	textures_ms.deleteTextures();
	textures_big.deleteTextures();
}

void MainFrameBuffer::PrepAttachedFBO()
{
	if (fbo_attached == 0)
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo_attached);

	//
	// collect information about the mobu framebuffer

	int objectType, objectName;
	GLint width=0, height=0, format=0, samples=0;

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &objectType);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &objectName);

	if (objectType == GL_RENDERBUFFER && glIsRenderbuffer(objectName) == GL_TRUE)
	{
		glBindRenderbuffer(GL_RENDERBUFFER,objectName);
		
		glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_HEIGHT, &height);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_INTERNAL_FORMAT, &format);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER,GL_RENDERBUFFER_SAMPLES, &samples);
		/*
		if (samples > 0)
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, linfo.samples, linfo.depthInternalFormat, linfo.width, linfo.height);
		}
		else
		{
			glRenderbufferStorage     ( GL_RENDERBUFFER, linfo.depthInternalFormat, linfo.width, linfo.height );
		}
		*/
		glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_DEPTH_ATTACHMENT,
		                    GL_RENDERBUFFER, 0 );	// objectName 
		glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_STENCIL_ATTACHMENT,
		                    GL_RENDERBUFFER, 0 );	// COMPOSITE MASTER USE FORMAT WITHOUT STENCIL !!!

		glBindRenderbuffer(GL_RENDERBUFFER,0);
	}

	extendedInfo.Set(1.0, samples, 0, width, height);

	//
	// detach mobu resources

	detachFBOAttachment(0, GL_COLOR_ATTACHMENT0, samples);

	/*
	CreateTextures(textures_extended, extendedInfo.GetBufferWidth(), extendedInfo.GetBufferHeight(), 
		extendedInfo.GetNumberOfSamples(), extendedInfo.GetNumberOfCoverageSamples(), true, true, false, true, true);

	attachTexture2D(0, GL_COLOR_ATTACHMENT0, textures_extended.color_texture, extendedInfo.GetNumberOfSamples());
	attachTexture2D(0, depthAttachment, textures_extended.depth_texture, extendedInfo.GetNumberOfSamples());
	//attachTexture2D(0, GL_STENCIL_ATTACHMENT, textures_extended.stencil_texture, extendedInfo.GetNumberOfSamples());
	attachTexture2D(0, GL_COLOR_ATTACHMENT1, textures_extended.normal_texture, extendedInfo.GetNumberOfSamples());
	attachTexture2D(0, GL_COLOR_ATTACHMENT2, textures_extended.mask_texture, extendedInfo.GetNumberOfSamples());
	*/
	
	CreateTextures(textures_extended, extendedInfo.GetBufferWidth(), extendedInfo.GetBufferHeight(), 
		extendedInfo.GetNumberOfSamples(), extendedInfo.GetNumberOfCoverageSamples(), 
		true, true, false, UseAddOnTextures, UseAddOnTextures, UseAddOnTextures);

	attachTexture2D(0, GL_COLOR_ATTACHMENT0, textures_extended.color_texture, extendedInfo.GetNumberOfSamples());
	attachTexture2D(0, depthAttachment, textures_extended.depth_texture, extendedInfo.GetNumberOfSamples());
	//attachTexture2D(0, GL_STENCIL_ATTACHMENT, textures_extended.stencil_texture, mainInfo.GetNumberOfSamples());

	if (UseAddOnTextures)
	{
		attachTexture2D(0, GL_COLOR_ATTACHMENT1, textures_extended.normal_texture, extendedInfo.GetNumberOfSamples());
		attachTexture2D(0, GL_COLOR_ATTACHMENT2, textures_extended.mask_texture, extendedInfo.GetNumberOfSamples());
		attachTexture2D(0, GL_COLOR_ATTACHMENT3, textures_extended.position_texture, extendedInfo.GetNumberOfSamples());
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

const GLuint MainFrameBuffer::prepTexture(GLuint id, int depthSamples, int coverageSamples, int width, int height, GLenum internalFormat, GLenum format, GLenum type)
{
	GLuint texId = id;
	if (texId == 0)
	{
		glGenTextures(1, &texId);
	}

	CHECK_GL_ERROR_MOBU();

	if (depthSamples <= 1)
	{
		glTextureStorage2DEXT(texId, GL_TEXTURE_2D, 1, internalFormat, width, height);
		glTextureParameterfEXT(texId, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameterfEXT(texId, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameterfEXT(texId, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameterfEXT(texId, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		if (coverageSamples > 1)
		{
			glTextureImage2DMultisampleCoverageNV(texId, GL_TEXTURE_2D_MULTISAMPLE, coverageSamples, depthSamples, internalFormat, width, height, GL_TRUE);
		}
		else
		{
			glTextureStorage2DMultisampleEXT(texId, GL_TEXTURE_2D_MULTISAMPLE, depthSamples, internalFormat, width, height, GL_TRUE);
		}
	}

	CHECK_GL_ERROR_MOBU();
	return texId;
}

void MainFrameBuffer::DetachTexturesFromFBO(const GLuint _fbo, const int samples)
{
	if (_fbo > 0)
	{
		bindFBO(_fbo);
		detachFBOAttachment(0, GL_COLOR_ATTACHMENT0, samples);

		if (UseAddOnTextures)
		{
			detachFBOAttachment(0, GL_COLOR_ATTACHMENT1, samples);
			detachFBOAttachment(0, GL_COLOR_ATTACHMENT2, samples);
			detachFBOAttachment(0, GL_COLOR_ATTACHMENT3, samples);
		}
		detachFBOAttachment(0, depthAttachment, samples);
		//detachFBOAttachment(0, GL_STENCIL_ATTACHMENT, samples);

		unbindFBO();
	}
}

void MainFrameBuffer::AttachTexturesToFBO(const GLuint _fbo, const texture_pack &pack, const int samples)
{
	if (_fbo > 0)
	{
		bindFBO(_fbo);
			
		attachTexture2D(0, GL_COLOR_ATTACHMENT0, pack.color_texture, samples);

		if (UseAddOnTextures)
		{
			attachTexture2D(0, GL_COLOR_ATTACHMENT1, pack.normal_texture, samples);
			attachTexture2D(0, GL_COLOR_ATTACHMENT2, pack.mask_texture, samples);
			attachTexture2D(0, GL_COLOR_ATTACHMENT3, pack.position_texture, samples);
		}
		attachTexture2D(0, depthAttachment, pack.depth_texture, samples);
		//attachTexture2D(0, GL_STENCIL_ATTACHMENT, pack.stencil_texture, samples);

		checkFboStatus(_fbo);
		unbindFBO();
	}
}
/*
void MainFrameBuffer::CreateTextures(int _width, int _height, double ssfact, int _depthSamples, int _coverageSamples, bool createColor, bool createDepth, bool createNormal, bool createMask)
{
	scaleFactor = ssfact;

	depthSamples = _depthSamples;
	coverageSamples = _coverageSamples;

//	multisample = (_depthSamples > 1);
//	csaa = (coverageSamples > depthSamples);

	bool ismultisample = depthSamples > 1;

	width = _width;
	height = _height;

	bufw = (int) (scaleFactor * (double)width);
	bufh = (int) (scaleFactor * (double)height);

	if (createColor)
	{
		if (scaleFactor > 1.0)
			textures_big.color_texture = prepTexture(0, 0, 0, bufw, bufh, colorInternalFormat, colorFormat, colorType);

		if (ismultisample)
			textures_ms.color_texture = prepTexture(0, depthSamples, coverageSamples, bufw, bufh, colorInternalFormat, colorFormat, colorType);

		textures.color_texture = prepTexture(0, 0, 0, width, height, colorInternalFormat, colorFormat, colorType);
	}

	if (createDepth)
	{
		if (scaleFactor > 1.0)
			textures_big.depth_texture = prepTexture(0, 0, 0, bufw, bufh, depthInternalFormat, depthFormat, depthType);

		if (ismultisample)
			textures_ms.depth_texture = prepTexture(0, depthSamples, coverageSamples, bufw, bufh, depthInternalFormat, depthFormat, depthType);

		textures.depth_texture = prepTexture(0, 0, 0, bufw, bufh, depthInternalFormat, depthFormat, depthType);
	}

	if (createNormal)
	{
		if (scaleFactor > 1.0)
			textures_big.normal_texture = prepTexture(0, 0, 0, bufw, bufh, normalInternalFormat, normalFormat, normalType);

		if (ismultisample)
			textures_ms.normal_texture = prepTexture(0, depthSamples, coverageSamples, bufw, bufh, normalInternalFormat, normalFormat, normalType);

		textures.normal_texture = prepTexture(0, 0, 0, bufw, bufh, normalInternalFormat, normalFormat, normalType);
	}

	if (createMask)
	{
		if (scaleFactor > 1.0)
			textures_big.mask_texture = prepTexture(0, 0, 0, bufw, bufh, maskInternalFormat, maskFormat, maskType);

		if (ismultisample)
			textures_ms.mask_texture = prepTexture(0, depthSamples, coverageSamples, bufw, bufh, maskInternalFormat, maskFormat, maskType);

		textures.mask_texture = prepTexture(0, 0, 0, bufw, bufh, maskInternalFormat, maskFormat, maskType);
	}
}
*/
void MainFrameBuffer::CreateTextures(texture_pack &pack, int width, int height, int depthSamples, int coverageSamples, bool createColor, bool createDepth, bool createStencil, bool createNormal, bool createMask, bool createPosition)
{
	if (createColor)
		pack.color_texture = prepTexture(0, depthSamples, coverageSamples, width, height, colorInternalFormat, colorFormat, colorType);
	
	if (createStencil)
		pack.stencil_texture = prepTexture(0, depthSamples, coverageSamples, width, height, stencilInternalFormat, stencilFormat, stencilType);

	if (createDepth)
		pack.depth_texture = prepTexture(0, depthSamples, coverageSamples, width, height, depthInternalFormat, depthFormat, depthType);
	
	if (createNormal)
		pack.normal_texture = prepTexture(0, depthSamples, coverageSamples, width, height, normalInternalFormat, normalFormat, normalType);
	
	if (createMask)
		pack.mask_texture = prepTexture(0, depthSamples, coverageSamples, width, height, maskInternalFormat, maskFormat, maskType);

	if (createPosition)
		pack.position_texture = prepTexture(0, depthSamples, coverageSamples, width, height, positionInternalFormat, positionFormat, positionType);
}

bool MainFrameBuffer::ReSize(const int newWidth, const int newHeight, double ssfact, int _depthSamples, int _coverageSamples)
{
	bool result = false;

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif

	bool needUpdate = false;
	bool firstTime = false;
	if (fbo == 0)
	{
		fbo = createFBO();
		needUpdate = true;
		firstTime = true;
	}
	if (fbobig == 0)
	{
		fbobig = createFBO();
		needUpdate = true;
		firstTime = true;
	}
	if (fboms == 0)
	{
		fboms = createFBO();
		needUpdate = true;
		firstTime = true;
	}

	CHECK_GL_ERROR_MOBU();

	double local_ssfact = ssfact;
	int local_samples = _depthSamples;
	int local_coverageSamples = _coverageSamples;
	int local_width = newWidth;
	int local_height = newHeight;

	if (isFboAttached() )
	{
		local_ssfact = 1.0;
		local_samples = extendedInfo.GetNumberOfSamples();
		local_coverageSamples = extendedInfo.GetNumberOfCoverageSamples();
		local_width = extendedInfo.GetBufferWidth();
		local_height = extendedInfo.GetBufferHeight();
	}

	if ( needUpdate == true 
		|| mainInfo.NotEqual(local_ssfact, local_samples, local_coverageSamples, local_width, local_height) )
	{
		result = true;

		DeleteTextures();
		if (firstTime == false)
		{
			DetachTexturesFromFBO(fbo, 0);
			if (mainInfo.GetScaleFactor() > 1.0)
				DetachTexturesFromFBO(fbobig, 0);
			if (mainInfo.GetNumberOfSamples() > 1)
				DetachTexturesFromFBO(fboms, mainInfo.GetNumberOfSamples() );
		}

		CHECK_GL_ERROR_MOBU();

		mainInfo.Set(local_ssfact, local_samples, local_coverageSamples, local_width, local_height);

		unbindFBO();
		
		if (local_ssfact > 1.0)
		{
			CreateTextures(textures_big, mainInfo.GetBufferWidth(), mainInfo.GetBufferHeight(), 
				0, 0, true, true, false, UseAddOnTextures, UseAddOnTextures, UseAddOnTextures);
			AttachTexturesToFBO(fbobig, textures_big, 0);
		}
		if (false == isFboAttached() && local_samples > 1)
		{
			CreateTextures(textures_ms, mainInfo.GetBufferWidth(), mainInfo.GetBufferHeight(), 
				local_samples, local_coverageSamples, true, true, false, UseAddOnTextures, UseAddOnTextures, UseAddOnTextures);
			AttachTexturesToFBO(fboms, textures_ms, mainInfo.GetNumberOfSamples());
		}
		CreateTextures(textures, mainInfo.GetWidth(), mainInfo.GetHeight(), 0, 0, 
			true, true, false, UseAddOnTextures, UseAddOnTextures, UseAddOnTextures); 
		AttachTexturesToFBO(fbo, textures, 0);
		
		CHECK_GL_ERROR_MOBU();
	}

	return result;
}

bool MainFrameBuffer::PrepShader()
{
	if (mShaderDownsample3.get() == nullptr)
	{
		FBString effectPath, effectFullName;

		FBString vertex_filename("\\GLSL\\simple.vsh");
		FBString fragment_filename("\\GLSL\\downsample3.fsh");

		GLSLShader *pShader = new GLSLShader();
					
		if (false == FindEffectLocation( fragment_filename, effectPath, effectFullName )
			|| false == pShader->LoadShaders( FBString(effectPath, vertex_filename), FBString(effectPath, fragment_filename) ))
		{
			delete pShader;
			return false;
		}
		else
		{
			pShader->Bind();
			GLint loc = pShader->findLocation( "sampler" );
			if (loc >= 0)
				pShader->setUniformUINT(loc, 0);
			locTexelSize = pShader->findLocation( "texelSize" );
			//if (loc >= 0)
			//	glUniform2f(loc, 1.0f/(float)bigw, 1.0f/(float)bigh );
			pShader->UnBind();

			mShaderDownsample3.reset(pShader);
		}
	}

	return true;
}

void MainFrameBuffer::DownscaleToFinalTextures(GLuint drawToFBO, GLSLShader *downscaleShader, texture_pack &big, int bigw, int bigh, int width, int height)
{
	if (drawToFBO > 0)
		bindFBO(drawToFBO);

	glDepthMask(GL_TRUE);

	if (downscaleShader != nullptr)
	{
		downscaleShader->Bind();
		downscaleShader->setUniformVector2f("texelSize", 1.0f/(float)bigw, 1.0f/(float)bigh);
	}

	if (big.color_texture > 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, big.color_texture);
	}
	/*
	if (textures.depth_texture > 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures_big.depth_texture);
	}
	if (textures.normal_texture > 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures_big.normal_texture);
	}
	if (textures.mask_texture > 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textures_big.mask_texture);
	}
	*/
	glActiveTexture(GL_TEXTURE0);

	drawOrthoQuad2d(0, 0, width, height, width, height);
				
	if (downscaleShader != nullptr)
		downscaleShader->UnBind();

	glDepthMask(GL_FALSE);

	if (drawToFBO > 0)
		unbindFBO();
}

void MainFrameBuffer::PrepForPostProcessing(bool drawToBack)
{
	if (isFboAttached() )
		PrepForPostProcessingExtended(drawToBack);
	else
		PrepForPostProcessingInternal(drawToBack);
}

void MainFrameBuffer::PrepForPostProcessingExtended(bool drawToBack)
{
	const int bufw = extendedInfo.GetBufferWidth();
	const int bufh = extendedInfo.GetBufferHeight();

	const int width = extendedInfo.GetWidth();
	const int height = extendedInfo.GetHeight();

	if (extendedInfo.GetNumberOfSamples() > 1)
	{
		BlitFBOToFBO( fbo_attached, bufw, bufh,
					fbo, bufw, bufh,
					UseAddOnTextures, UseAddOnTextures, UseAddOnTextures );
		
		/*
		if (drawToBack)
		{
			BlitFBOToScreen(fbo, bufw, bufh, width, height);
		}
		*/
	} // if multisample
	
	if (drawToBack)
	{
		BlitFBOToScreen(fbo_attached, bufw, bufh, width, height);
	}
}

void MainFrameBuffer::PrepForPostProcessingInternal(bool drawToBack)
{
	const int bufw = mainInfo.GetBufferWidth();
	const int bufh = mainInfo.GetBufferHeight();

	const int width = mainInfo.GetWidth();
	const int height = mainInfo.GetHeight();

	if (mainInfo.GetNumberOfSamples() > 1)
	{
		if (mainInfo.GetScaleFactor() > 1.0)
		{
			/*
			BlitFBOToFBO( fboms, bufw, bufh,
						fbobig, bufw, bufh,
					true, true );

			CHECK_GL_ERROR_MOBU();

			// downsample textures with specified filtering
			DownscaleToFinalTextures(drawToBack);
			*/

			BlitFBOToFBO( fboms, bufw, bufh,
						fbobig, bufw, bufh,
						UseAddOnTextures, UseAddOnTextures, UseAddOnTextures );
			/*
			BlitFBOToFBO( fbobig, bufw, bufh,
						fbo, width, height,
					false, false );
					*/
		}
		else
		{
			
			BlitFBOToFBO( fboms, bufw, bufh,
						fbo, bufw, bufh,
					UseAddOnTextures, UseAddOnTextures, UseAddOnTextures );
			
		}
	} // if non multisample
	else
	{	
		if (mainInfo.GetScaleFactor() > 1.0)
		{
			/*
			BlitFBOToFBO( fbobig, bufw, bufh,
						fbo, width, height,
					false, false );
					*/
		}
	}

	//
	if (drawToBack)
	{
		BlitFBOToScreen(fbo, width, height, width, height);
	}
}

void MainFrameBuffer::ChangeContext()
{
	if (mShaderDownsample3.get() != nullptr)
	{
		mShaderDownsample3->Free();
		mShaderDownsample3.reset(nullptr);
	}
	DeleteTextures();
	DeleteFBO();

	DetachFBO();
}

void MainFrameBuffer::BeginRender(const bool moreColorAttachment)
{
	if (fbo_attached > 0)
		BeginRenderAttached(moreColorAttachment);
	else
		BeginRenderInternal(moreColorAttachment);
}

void MainFrameBuffer::UpdateViewport()
{
	if (fbo_attached > 0)
		UpdateViewportAttached();
	else
		UpdateViewportInternal();
}

void MainFrameBuffer::UpdateViewportAttached()
{
	const int bufw = extendedInfo.GetBufferWidth();
	const int bufh = extendedInfo.GetBufferHeight();
	
	glViewport(0, 0, bufw, bufh);
}

void MainFrameBuffer::UpdateViewportInternal()
{
	const int bufw = mainInfo.GetBufferWidth();
	const int bufh = mainInfo.GetBufferHeight();
	
	// bind proper buffer
	glViewport(0, 0, bufw, bufh);
}

void MainFrameBuffer::BeginRenderAttached(const bool moreColorAttachment)
{
	
	const int bufw = extendedInfo.GetBufferWidth();
	const int bufh = extendedInfo.GetBufferHeight();
	const int samples = extendedInfo.GetNumberOfSamples();
	
	glViewport(0, 0, bufw, bufh);
	
	bindFBO(fbo_attached);

	if (UseAddOnTextures && moreColorAttachment)
	{
		GLenum buffers [] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers( 3, buffers );
		glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	GLenum buffers2 [] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, buffers2 );

	// enable multisample
	if (samples > 1)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);
}

void MainFrameBuffer::BeginRenderInternal(const bool moreColorAttachment)
{
	const int bufw = mainInfo.GetBufferWidth();
	const int bufh = mainInfo.GetBufferHeight();
	const int samples = mainInfo.GetNumberOfSamples();

	// bind proper buffer
	glViewport(0, 0, bufw, bufh);
		
	if (samples > 1)
		bindFBO( fboms );
	else
	{
		if (mainInfo.GetScaleFactor() > 1.0)
			bindFBO( fbobig );
		else
			bindFBO( fbo );
	}

	if (UseAddOnTextures && moreColorAttachment)
	{
		GLenum buffers [] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers( 3, buffers );
		glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		GLenum buffers2 [] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers( 4, buffers2 );
	}
	else
	{
		GLenum buffers2 [] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers( 1, buffers2 );
	}

	// enable multisample
	if (samples > 1)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);
}

void MainFrameBuffer::EndRender()
{
	if (UseAddOnTextures)
	{
		GLenum buffers2 [] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers( 1, buffers2 );
	}

	unbindFBO();
}

void MainFrameBuffer::BlitBIGToFinalColor(const EResamplingFilter resamplingFilter)
{
	if ( mainInfo.GetScaleFactor() <= 1.0 )
		return;
		
	GLSLShader *pDownsampleShader = GetDownsampleShaderPtr(resamplingFilter);

	// apply a shader with a 9 kernel downsampling
				
	if (nullptr != pDownsampleShader && fbo > 0)
	{
		const int newWidth = mainInfo.GetWidth();
		const int newHeight = mainInfo.GetHeight();

		glViewport(0, 0, newWidth, newHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		
		GLenum buffers2 [] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers( 1, buffers2 );
		
		pDownsampleShader->Bind();

		glBindTexture(GL_TEXTURE_2D, textures_big.color_texture );

		GLint loc = pDownsampleShader->findLocation( "texelSize" );
		if (loc >= 0)
		{
			pDownsampleShader->setUniformVector2f( loc, 0.5f / (float) newWidth, 0.5f / (float) newHeight );
		}
		
		//mMainFrameBuffer.GetBufferWidth(), mMainFrameBuffer.GetBufferHeight() );
		drawQuad2d( 0, 0, newWidth, newHeight);

		pDownsampleShader->UnBind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

bool MainFrameBuffer::LoadDownsampleShader(const char *vertex, const char *fragment, GLSLShader *&pShader)
{
	bool result = true;

	FBString effectPath, effectFullName;

	FBString vertex_filename(vertex);
	FBString fragment_filename(fragment);

	//GLSLShader *pShader = new GLSLShader();
	pShader = new GLSLShader();

	if (false == FindEffectLocation( fragment_filename, effectPath, effectFullName )
		|| false == pShader->LoadShaders( FBString(effectPath, vertex_filename), FBString(effectPath, fragment_filename) ))
	{
		delete pShader;
		pShader = nullptr;
		result = false;
	}
	else
	{
		pShader->Bind();
		GLint loc = pShader->findLocation( "sampler" );
		if (loc >= 0)
			pShader->setUniformUINT(loc, 0);
		loc = pShader->findLocation( "texelSize" );
		if (loc >= 0)
			glUniform2f(loc, 1.0f / 512.0f, 1.0f/512.0f );
		pShader->UnBind();

		//mShaderDownsample3.reset(pShader);
	}
	
	return result;
}


GLSLShader *MainFrameBuffer::GetDownsampleShaderPtr(const EResamplingFilter resamplingFilter)
{
	switch(resamplingFilter)
	{
	case eResamplingFilterLinear:
		if (mShaderDownsample.get() == nullptr)
		{
			GLSLShader *newShader = nullptr;
			LoadDownsampleShader( RESAMPLING_LINEAR_VERTEX, RESAMPLING_LINEAR_FRAGMENT, newShader );
			mShaderDownsample.reset(newShader);
		}
		return mShaderDownsample.get();
		break;
	case eResamplingFilterKernel:
		if (mShaderDownsample3.get() == nullptr)
		{
			GLSLShader *newShader = nullptr;
			LoadDownsampleShader( RESAMPLING_LINEAR_VERTEX, RESAMPLING_KERNEL_FRAGMENT, newShader );
			mShaderDownsample3.reset(newShader);
		}
		return mShaderDownsample3.get();
		break;
	case eResamplingFilterLanczos:
		if (mShaderDownsampleLanczos.get() == nullptr)
		{
			GLSLShader *newShader = nullptr;
			LoadDownsampleShader( RESAMPLING_LANCZOS_VERTEX, RESAMPLING_LANCZOS_FRAGMENT, newShader );
			mShaderDownsampleLanczos.reset(newShader);
		}
		return mShaderDownsampleLanczos.get();
		break;
	}

	return nullptr;
}