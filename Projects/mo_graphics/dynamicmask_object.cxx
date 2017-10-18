
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicmask_object.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "dynamicmask_object.h"
#include "dynamicmask_common.h"
#include "FBCommon.h"
#include "IO\FileUtils.h"
#include "graphics\CheckGLError_MOBU.h"
#include "shared_misc.h"
#include "algorithm\math3d_mobu.h"

FBClassImplementation(ObjectMask)
FBUserObjectImplement(ObjectMask, "Dynamic mask", FB_DEFAULT_SDK_ICON);	//Register UserObject class
FBElementClassImplementationMasking(ObjectMask, FB_DEFAULT_SDK_ICON);					//Register to the asset system

FBClassImplementation(ObjectChannel)
FBUserObjectImplement(ObjectChannel, "Dynamic masking channel", FB_DEFAULT_SDK_ICON);	//Register UserObject class
FBElementClassImplementationMasking(ObjectChannel, FB_DEFAULT_SDK_ICON);					//Register to the asset system


FBClassImplementation(ObjectShape)
FBUserObjectImplement(ObjectShape, "Dynamic masking shape", FB_DEFAULT_SDK_ICON);	//Register UserObject class
FBElementClassImplementationMasking(ObjectShape, FB_DEFAULT_SDK_ICON);					//Register to the asset system

FBClassImplementation(ObjectKnot)
FBUserObjectImplement(ObjectKnot, "Dynamic masking knot", FB_DEFAULT_SDK_ICON);	//Register UserObject class
FBElementClassImplementationMasking(ObjectKnot, FB_DEFAULT_SDK_ICON);					//Register to the asset system

extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);

const GLint FBO_WIDTH = 512;
const GLint FBO_HEIGHT = 512;

const int EObjectMaskSizeToInt( const EObjectMaskSize esize )
{
	int size = 512;

	switch(esize)
	{
	case eMaskSize32:
		size = 32;
		break;
	case eMaskSize64:
		size = 64;
		break;
	case eMaskSize128:
		size = 128;
		break;
	case eMaskSize256:
		size = 256;
		break;
	case eMaskSize512:
		size = 512;
		break;
	case eMaskSize1024:
		size = 1024;
		break;
	case eMaskSize2048:
		size = 2048;
		break;
	case eMaskSize4096:
		size = 4096;
		break;
	}

	return size;
}

void ObjectMask::SetCanvasWidth(HIObject object, int pValue)
{
	ObjectMask *pObject = FBCast<ObjectMask>(object);
	if (pObject) 
	{
		//const int size = EObjectMaskSizeToInt(pValue);
		pObject->NeedResize();
		pObject->mWidth = pValue;
		/*
		const int size = pValue;
		pObject->DoReSize(&size, nullptr);
		pObject->CanvasWidth.SetPropertyValue(pValue);
		*/
	}
}

void ObjectMask::SetCanvasHeight(HIObject object, int pValue)
{
	ObjectMask *pObject = FBCast<ObjectMask>(object);
	if (pObject) 
	{
		//const int size = EObjectMaskSizeToInt(pValue);
		pObject->NeedResize();
		pObject->mHeight = pValue;
		/*
		const int size = pValue;
		pObject->DoReSize(nullptr, &size);
		pObject->CanvasHeight.SetPropertyValue(pValue);
		*/
	}
}

static void ObjectMask_Update(HIObject object, int pValue)
{
	ObjectMask *pObject = FBCast<ObjectMask>(object);
	if (pObject) 
	{
		pObject->DoUpdate();
	}
}

int ObjectMask::GetCanvasWidth(HIObject object)
{
	ObjectMask *pObject = FBCast<ObjectMask>(object);
	if (pObject) 
	{
		return pObject->mWidth;
	}
	return 1;
}

int ObjectMask::GetCanvasHeight(HIObject object)
{
	ObjectMask *pObject = FBCast<ObjectMask>(object);
	if (pObject) 
	{
		return pObject->mHeight;
	}
	return 1;
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectMask::ObjectMask( const char* pName, HIObject pObject ) 
	: FBUserObject( pName, pObject )
	//, mVideoMemory( "DynamicMasked" )
	//, mTexture( "DynamicTextureed" )
{
	FBClassInit;
	
	//
	fbo_id = 0;
	color_buffer_id = 0;
	depth_buffer_id = 0;
	depth_id = 0;
	blur_h_texture_id = 0;
	blur_v_texture_id = 0;

	mNeedUpdate = true;

	//
	FBPropertyPublish(this, Enable, "Enable", nullptr, nullptr);

	FBPropertyPublish(this, CanvasWidth, "Canvas Width", GetCanvasWidth, SetCanvasWidth);
	FBPropertyPublish(this, CanvasHeight, "Canvas Height", GetCanvasHeight, SetCanvasHeight);

	FBPropertyPublish(this, RecalculatePoints, "Recalculate Points", nullptr, nullptr);

	FBPropertyPublish(this, Video, "Video", nullptr, nullptr);

	FBPropertyPublish(this, ManualUpdate, "Manual Update", nullptr, nullptr);
	FBPropertyPublish(this, Update, "Update", nullptr, ObjectMask_Update);

	FBPropertyPublish(this, ShapeColor, "Shape Color", nullptr, nullptr);
	FBPropertyPublish(this, Stroke, "Outline", nullptr, nullptr);
	FBPropertyPublish(this, StrokeColor, "Outline Color", nullptr, nullptr);
	FBPropertyPublish(this, ShapeTransparency, "Shape Transparency", nullptr, nullptr);
	FBPropertyPublish(this, ShowGrid, "Show Grid", nullptr, nullptr);
	FBPropertyPublish(this, BlurFactor, "Blur Factor", nullptr, nullptr);

	FBPropertyPublish(this, ShowBackgroundTexture, "Show Back Texture", nullptr, nullptr);
	FBPropertyPublish(this, BackgroundTexture, "Background Texture", nullptr, nullptr);

	FBPropertyPublish(this, UVLayout, "UV layout", nullptr, nullptr);
	FBPropertyPublish(this, ShowTexture, "Show Texture", nullptr, nullptr);
	FBPropertyPublish(this, ShowWire, "Show Wireframe", nullptr, nullptr);

	Video.SetSingleConnect(false);
	Video.SetFilter( FBVideoMemory::GetInternalClassId() );

	ManualUpdate = true;

	ShapeColor = FBColor(1.0, 1.0, 1.0);

	Stroke = false;
	StrokeColor = FBColor(0.0, 0.0, 1.0);

	ShapeTransparency = 0.0;
	ShapeTransparency.SetMinMax( 0.0, 100.0, true, true );
	ShowGrid = true;
	BlurFactor = 0;
	BlurFactor.SetMinMax( 0.0, 100.0, true, true );

	ShowBackgroundTexture = true;
	BackgroundTexture.SetSingleConnect(true);
	BackgroundTexture.SetFilter(FBTexture::GetInternalClassId() );

	UVLayout.SetSingleConnect(false);
	UVLayout.SetFilter( FBModel::GetInternalClassId() );

	ShowTexture = false;
	ShowWire = true;

	//DisableObjectFlags(kFBFlagBrowsable);

	//FBTexture *pTexture = new FBTexture("DynamicTexture");
	//pTexture->Video = &mVideoMemory;
	//mTexture.Video = &mVideoMemory;

	
	//
	Enable = true;
	/*
	CanvasWidth = eMaskSize512;
	CanvasHeight = eMaskSize512;
	*/
	
	CanvasWidth.SetMinMax(32.0, 4096.0, true, true);
	CanvasHeight.SetMinMax(32.0, 4096.0, true, true);

	mWidth = FBO_WIDTH;
	mHeight = FBO_HEIGHT;
	mLastWidth = mWidth;
	mLastHeight = mHeight;

	RecalculatePoints = true;

	mLoaded = false;
	mNeedUpdate = true;
	mNeedResize = false;

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
}

bool ObjectMask::PrepCompositeShader()
{
	bool lresult = true;

	FBString effectPath, effectFullName;
	CHECK_GL_ERROR_MOBU();

	if (nullptr != mShaderComposite.get() )
		return true;

	GLSLShader *pNewShader = nullptr;

	try
	{
		pNewShader = new GLSLShader();

		if ( false == FindEffectLocation( "\\GLSL\\dynamicMask.fsh", effectPath, effectFullName ) )
			throw "Failed to locate shader files";
		
		if ( false == pNewShader->LoadShaders( FBString(effectPath, "\\GLSL\\simple.vsh"), FBString(effectPath, "\\GLSL\\dynamicMask.fsh") ) )
			throw "Failed to load shaders";
		
		mShaderCompositeLoc.rSampler = pNewShader->findLocation( "rSampler" );
		mShaderCompositeLoc.gSampler = pNewShader->findLocation( "gSampler" );
		mShaderCompositeLoc.bSampler = pNewShader->findLocation( "bSampler" );
		mShaderCompositeLoc.aSampler = pNewShader->findLocation( "aSampler" );
	}
	catch (const char *message)
	{
		FBMessageBox( "Dynamic Mask", message, "Ok" );
		mLoaded = false;

		if (nullptr != pNewShader)
		{
			delete pNewShader;
			pNewShader = nullptr;
		}
		lresult = false;
	}

	mShaderComposite.reset( pNewShader );

	CHECK_GL_ERROR_MOBU();

	return lresult;
}

bool ObjectMask::PrepBlurShader()
{
	bool lresult = true;
	FBString effectPath, effectFullName;
	CHECK_GL_ERROR_MOBU();

	if (nullptr != mShaderBlur.get() )
		return true;

	GLSLShader *pNewShader = nullptr;

	try
	{
		pNewShader = new GLSLShader();

		//
		if ( false == FindEffectLocation( "\\GLSL\\blur2.fsh", effectPath, effectFullName ) )
			throw "Failed to locate blur files";

		if (false == pNewShader->LoadShaders( FBString(effectPath, "\\GLSL\\simple.vsh"), FBString(effectPath, "\\GLSL\\blur2.fsh") ) )
			throw "Failed to load blur shaders";
				
		mShaderBlurLoc.sampler = pNewShader->findLocation( "mainTex" );
		mShaderBlurLoc.scale = pNewShader->findLocation( "scale" );
	}
	catch (const char *message)
	{
		FBMessageBox( "Dynamic Mask", message, "Ok" );
		mLoaded = false;

		if (nullptr != pNewShader)
		{
			delete pNewShader;
			pNewShader = nullptr;
		}
		lresult = false;
	}

	mShaderBlur.reset( pNewShader );

	CHECK_GL_ERROR_MOBU();
	return lresult;
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectMask::FBCreate()
{
	return ParentClass::FBCreate();
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ObjectMask::FBDestroy()
{
	CleanupFBO();
	ParentClass::FBDestroy();
}

void ObjectMask::GenerateFBO()
{
	const int width = mWidth;
	const int height = mHeight;

	GLenum status;
 
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// create a texture we use as the color buffer
	if (color_buffer_id == 0)
		glGenTextures(1, &color_buffer_id);
	
	glBindTexture(GL_TEXTURE_2D, color_buffer_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
 
	if (Video.GetCount() )
	{
		FBComponent *pComp = Video.GetAt(0);
		if (pComp->Is(FBVideoMemory::TypeInfo))
		{
			FBVideoMemory *pVideoMem = (FBVideoMemory*) pComp;
			pVideoMem->TextureOGLId = getColorTextureId();
			pVideoMem->SetObjectImageSize( width, height );
		}
	}

	//
	/*
	if (depth_buffer_id == 0)
		glGenTextures(1, &depth_buffer_id);
	
	glBindTexture(GL_TEXTURE_2D, depth_buffer_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, CanvasWidth, CanvasHeight, 0, GL_DEPTH24_STENCIL8, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	*/
	
	CHECK_GL_ERROR_MOBU();

	// create a renderbuffer for the depth/stencil buffer
	if (depth_id == 0)
		glGenRenderbuffers(1, &depth_id);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_id);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
 
	CHECK_GL_ERROR_MOBU();

	if (fbo_id == 0)
		glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	if (color_buffer_id > 0)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buffer_id, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_buffer_id, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencil_buffer_id, 0);
	
	if (depth_id > 0)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_id);
 
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if ( GL_FRAMEBUFFER_COMPLETE != status )
	{
		// something went wrong :(
		printf( "Failed to bind FBO with stencil\n" );
	}
	else
	{
		// profit :)
		//printf( "fbo setup is well!" );
	}
 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ObjectMask::CleanupFBO()
{
	//Delete resources
	if (color_buffer_id > 0)
	{
		glDeleteTextures(1, &color_buffer_id);
		color_buffer_id = 0;
	}
	if (depth_buffer_id > 0)
	{
		glDeleteTextures(1, &depth_buffer_id);
		depth_buffer_id = 0;
	}
	if (depth_id > 0)
	{
		glDeleteRenderbuffers(1, &depth_id);
		depth_id = 0;
	}
   //Bind 0, which means render to back buffer, as a result, fb is unbound
   //glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	if (fbo_id > 0)
	{
		glDeleteFramebuffers(1, &fbo_id);
		fbo_id = 0;
	}

	if (blur_h_texture_id > 0)
	{
		glDeleteTextures(1, &blur_h_texture_id);
		blur_h_texture_id = 0;
	}
	if (blur_v_texture_id > 0)
	{
		glDeleteTextures(1, &blur_v_texture_id);
		blur_v_texture_id = 0;
	}

   //mTexture.TextureOGLId = -1;
}

void ObjectMask::DoReSize()
{
	int lwidth = mWidth;
	int lheight = mHeight;

	if (lwidth < 32) lwidth = 32;
	if (lheight < 32) lheight = 32;

	if (lwidth > 8196) lwidth = 8196;
	if (lheight > 8196) lheight = 8196;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (color_buffer_id > 0)
	{
		glBindTexture(GL_TEXTURE_2D, color_buffer_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, lwidth, lheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	if (depth_buffer_id > 0)
	{
		glBindTexture(GL_TEXTURE_2D, depth_buffer_id);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, lwidth, lheight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, lwidth, lheight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	}

	if (blur_h_texture_id > 0)
	{
		glBindTexture(GL_TEXTURE_2D, blur_h_texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	if (blur_v_texture_id > 0)
	{
		glBindTexture(GL_TEXTURE_2D, blur_v_texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	CHECK_GL_ERROR_MOBU();

	if (depth_id > 0)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		if (fbo_id > 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		CHECK_GL_ERROR_MOBU();

		glDeleteRenderbuffers(1, &depth_id);
		depth_id = 0;
		glGenRenderbuffers(1, &depth_id);

		glBindRenderbuffer(GL_RENDERBUFFER, depth_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, lwidth, lheight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		CHECK_GL_ERROR_MOBU();

		if (fbo_id > 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_id);

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if ( GL_FRAMEBUFFER_COMPLETE != status )
			{
				// something went wrong :(
				printf( "Failed to bind FBO with stencil\n" );
			}
			else
			{
				// profit :)
				//printf( "fbo setup is well!" );
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	CHECK_GL_ERROR_MOBU();

	const double scalex = 1.0 * mLastWidth / lwidth;
	const double scaley = 1.0 * mLastHeight / lheight;

	for (int i=0; i<Components.GetCount(); ++i)
	{
		ObjectChannel *pChannel = (ObjectChannel*) Components[i];
		pChannel->ReSize(lwidth, lheight);

		if (RecalculatePoints)
		{
			pChannel->RecalculatePoints( scalex, scaley );
		}
	}

	CHECK_GL_ERROR_MOBU();

	mWidth = lwidth;
	mHeight = lheight;

	mLastWidth = lwidth;
	mLastHeight = lheight;

	mNeedUpdate = true;
}

void ObjectMask::DoUpdate()
{
	mNeedUpdate = true;
}

void ObjectMask::ChangeContext()
{
	CleanupFBO();

	if (Video.GetCount() > 0 && FBIS(Video.GetAt(0), FBVideoMemory) )
	{
		FBVideoMemory *pVideoMemory = (FBVideoMemory*) Video.GetAt(0);
		
		pVideoMemory->TextureOGLId = 0;
		pVideoMemory->SetObjectImageSize(32, 32);
	}

	for (int i=0; i<Components.GetCount(); ++i)
	{
		ObjectChannel *pChannel = (ObjectChannel*) Components[i];
		pChannel->ChangeContext();
	}

	mNeedUpdate = true;
	//mNeedResize = true;
}

void ObjectMask::Render()
{
	// skip if mask is disabled
	if (Enable == false) return;

	// render each of 4 channels
	// and write a final composited image

	if (fbo_id == 0)
		GenerateFBO();
	
	if (mNeedResize)
	{
		DoReSize();
		mNeedResize = false;
		mNeedUpdate = true;
	}

	if (ManualUpdate && mNeedUpdate == false)
		return;

	if (Components.GetCount() == 4)
	{
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		GLuint	channelIds[4];

		for (int i=0; i<Components.GetCount(); ++i)
		{
			ObjectChannel *pChannel = (ObjectChannel*) Components[i];

			// bind buffer
			const GLuint texture_id = pChannel->getColorTextureId();

			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
			glDrawBuffer( GL_COLOR_ATTACHMENT0 );
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

			// setup some display options
			pChannel->SetShapeDisplay( ShowGrid, ShapeColor, ShapeTransparency );

			pChannel->Render();

			// unbind (and blur if needed)
		
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// store result in the channel variable
			channelIds[i] = texture_id;

			// do we need to blur channel final image ?
			if (pChannel->BlurFactor > 0.0)
			{
				const GLuint blur_h = getBlurHTextureId();
				const GLuint blur_v = getBlurVTextureId();

				float f = 0.01f * pChannel->BlurFactor;

				RenderBlur( blur_h, texture_id, f, 0.0f );
				RenderBlur( blur_v, blur_h, 0.0f, f );

				channelIds[i] = blur_v;
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
		glDrawBuffer( GL_COLOR_ATTACHMENT0 );
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buffer_id, 0);

		const int width = mWidth;
		const int height = mHeight;

		glViewport(0, 0, width, height);

		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixLoadIdentityEXT(GL_PROJECTION);
		glMatrixLoadIdentityEXT(GL_MODELVIEW);
		glMatrixOrthoEXT(GL_MODELVIEW, 0, width, 0, height, -1, 1);
		//-------------------------
	
	
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		if (nullptr == mShaderComposite.get() )
			PrepCompositeShader();

		if (nullptr != mShaderComposite.get() )
		{
			mShaderComposite->Bind();

			mShaderComposite->bindTexture(GL_TEXTURE_2D, mShaderCompositeLoc.aSampler, channelIds[3], 3);
			mShaderComposite->bindTexture(GL_TEXTURE_2D, mShaderCompositeLoc.bSampler, channelIds[2], 2);
			mShaderComposite->bindTexture(GL_TEXTURE_2D, mShaderCompositeLoc.gSampler, channelIds[1], 1);
			mShaderComposite->bindTexture(GL_TEXTURE_2D, mShaderCompositeLoc.rSampler, channelIds[0], 0);
		
			glBegin(GL_QUADS);

			glTexCoord2d(0.0,		0.0);
				glVertex2d(0.0,			0.0);
			glTexCoord2d(1.0,		0.0);
				glVertex2d(width,	0.0);
			glTexCoord2d(1.0,		1.0);
				glVertex2d(width,	height);
			glTexCoord2d(0.0,		1.0);
				glVertex2d(0.0,		height);

			glEnd();

			mShaderComposite->UnBind();
			mShaderComposite->unbindTexture(GL_TEXTURE_2D, 0);
			mShaderComposite->unbindTexture(GL_TEXTURE_2D, 1);
			mShaderComposite->unbindTexture(GL_TEXTURE_2D, 2);
			mShaderComposite->unbindTexture(GL_TEXTURE_2D, 3);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//DONE: Add a blur feature for the whole mask
		
		if (BlurFactor > 0.0)
		{
			const GLuint blur_h = getBlurHTextureId();
			const GLuint blur_v = getBlurVTextureId();

			float f = 0.001f * BlurFactor;

			RenderBlur( blur_h, color_buffer_id, f, 0.0f );
			RenderBlur( blur_v, blur_h, 0.0f, f );

			if (Video.GetCount() )
				( (FBVideoMemory*) Video.GetAt(0) )->TextureOGLId = blur_v;
		}
		else
		{
			if (Video.GetCount() )
				( (FBVideoMemory*) Video.GetAt(0) )->TextureOGLId = color_buffer_id;
		}

	}
	mNeedUpdate = false;
	CHECK_GL_ERROR_MOBU();
}

bool ObjectMask::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ObjectMask::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if( pStoreWhat == kAttributes )
	{
	}
	mNeedUpdate = true;
	mNeedResize = true;

	return true;
}

const GLuint ObjectMask::getBlurHTextureId()
{
	if (blur_h_texture_id == 0)
		GenerateResources();

	return blur_h_texture_id;
}

const GLuint ObjectMask::getBlurVTextureId()
{
	if (blur_v_texture_id == 0)
		GenerateResources();

	return blur_v_texture_id;
}

void ObjectMask::GenerateResources()
{
	// create a texture we use as the color buffer
	if (blur_h_texture_id == 0)
		glGenTextures(1, &blur_h_texture_id);
	glBindTexture(GL_TEXTURE_2D, blur_h_texture_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
 
	CHECK_GL_ERROR_MOBU();

	// create a texture we use as the color buffer
	if (blur_v_texture_id == 0)
		glGenTextures(1, &blur_v_texture_id);
	glBindTexture(GL_TEXTURE_2D, blur_v_texture_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	CHECK_GL_ERROR_MOBU();
}

void ObjectMask::RenderBlur(const GLuint color_buffer_id, const GLuint color_output_id, const float hFactor, const float vFactor)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buffer_id, 0);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	const int width = mWidth;
	const int height = mHeight;

	glViewport(0, 0, width, height);
	glMatrixLoadIdentityEXT(GL_PROJECTION);
    glMatrixLoadIdentityEXT(GL_MODELVIEW);
	glMatrixOrthoEXT(GL_MODELVIEW, 0, width, 0, height, -1, 1);
	//-------------------------

	if (nullptr == mShaderBlur.get() )
		PrepBlurShader();

	if (nullptr != mShaderBlur.get() )
	{
		mShaderBlur->Bind();
		mShaderBlur->bindTexture(GL_TEXTURE_2D, mShaderBlurLoc.sampler, color_output_id, 0);
		//mShaderBlur.setUniformVector2f( mShaderScaleLoc, hFactor, vFactor );
		mShaderBlur->setUniformVector( mShaderBlurLoc.scale, hFactor, vFactor, 0.0f, 1.0f );

		glBegin(GL_QUADS);

		glTexCoord2d(0.0,		0.0);
			glVertex2d(0.0,			0.0);
		glTexCoord2d(1.0,		0.0);
			glVertex2d(width,	0.0);
		glTexCoord2d(1.0,		1.0);
			glVertex2d(width,	height);
		glTexCoord2d(0.0,		1.0);
			glVertex2d(0.0,		height);

		glEnd();

		mShaderBlur->UnBind();
		mShaderBlur->unbindTexture(GL_TEXTURE_2D, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	Constructor.
 ************************************************/
ObjectChannel::ObjectChannel( const char* pName, HIObject pObject ) 
	: FBUserObject( pName, pObject )
{
	FBClassInit;

	color_texture_id = 0;
	
	mWidth = FBO_WIDTH;
	mHeight = FBO_HEIGHT;

	FBPropertyPublish(this, OverrideDisplay, "Override Display", nullptr, nullptr);
	FBPropertyPublish(this, ShapeColor, "Shape Color", nullptr, nullptr);
	FBPropertyPublish(this, Stroke, "Outline", nullptr, nullptr);
	FBPropertyPublish(this, StrokeColor, "Outline Color", nullptr, nullptr);
	FBPropertyPublish(this, ShapeTransparency, "Shape Transparency", nullptr, nullptr);
	FBPropertyPublish(this, ShowGrid, "Show Grid", nullptr, nullptr);
	FBPropertyPublish(this, BlurFactor, "Blur Factor", nullptr, nullptr);
	FBPropertyPublish(this, ShowBackgroundTexture, "Show Back Texture", nullptr, nullptr);
	FBPropertyPublish(this, BackgroundTexture, "Background Texture", nullptr, nullptr);
	FBPropertyPublish(this, UVLayout, "UV layout", nullptr, nullptr);
	FBPropertyPublish(this, ShowTexture, "Show Texture", nullptr, nullptr);
	FBPropertyPublish(this, ShowWire, "Show Wireframe", nullptr, nullptr);

	OverrideDisplay = false;
	ShapeColor = FBColor(1.0, 1.0, 1.0);
	ShapeTransparency = 0.0;
	ShapeTransparency.SetMinMax(0.0, 100.0, true, true);

	Stroke = false;
	StrokeColor = FBColor(0.0, 0.0, 1.0);

	BlurFactor = 0.0;
	BlurFactor.SetMinMax(0.0, 10.0, true, true); 

	ShowBackgroundTexture = false;
	BackgroundTexture.SetSingleConnect(true);
	BackgroundTexture.SetFilter( FBTexture::GetInternalClassId() );

	UVLayout.SetSingleConnect(false);
	UVLayout.SetFilter( FBModel::GetInternalClassId() );

	ShowGrid = true;
	ShowTexture = false;
	ShowWire = true;

	//DisableObjectFlags(kFBFlagBrowsable);
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectChannel::FBCreate()
{
	
	color_texture_id=0;
	
	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ObjectChannel::FBDestroy()
{
	Cleanup();
}

const GLuint ObjectChannel::getColorTextureId()
{
	if (color_texture_id == 0)
		GenerateResources();

	return color_texture_id;
}


void ObjectChannel::GenerateResources()
{
	// create a texture we use as the color buffer
	if (color_texture_id == 0)
		glGenTextures(1, &color_texture_id);
	
	glBindTexture(GL_TEXTURE_2D, color_texture_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	CHECK_GL_ERROR_MOBU();
}


void ObjectChannel::Cleanup()
{
	//Delete resources
	if (color_texture_id > 0)
	{
		glDeleteTextures(1, &color_texture_id);
		color_texture_id = 0;
	}
	
}

void ScaleCurveKeys( FBFCurve *pCurve, const double f )
{
	if (pCurve == nullptr) return;

	const int numberOfKeys = pCurve->Keys.GetCount();
	double value;
	for (int i=0; i<numberOfKeys; ++i)
	{
		value = pCurve->Keys[i].Value;
		value *= f;
		pCurve->Keys[i].Value = value;
	}
}

void ObjectChannel::RecalculatePoints(const double x, const double y)
{
	// draw each shape
	for (int i=0; i<Components.GetCount(); ++i)
	{
		ObjectShape *shape = (ObjectShape*) Components[i];
		int count = shape->Components.GetCount();
		
		for (int j=0; j<count; ++j)
		{
			ObjectKnot *pKnot = (ObjectKnot*) shape->Components[j];

			FBAnimationNode *pNode = pKnot->Position.GetAnimationNode();

			if (pNode)
			{
				// scale keyframes

				FBAnimationNode *pX = pNode->Nodes[0];
				FBAnimationNode *pY = pNode->Nodes[1];

				if (pX) ScaleCurveKeys(pX->FCurve, x);
				if (pY) ScaleCurveKeys(pY->FCurve, y);
			}
			else
			{
				// scale only property values
				double values[2];
				pKnot->Position.GetData( values, sizeof(double)*2 );

				values[0] *= x;
				values[1] *= y;

				pKnot->Position.SetData( values );
			}
		}
	}
}

void ObjectChannel::ReSize(const int w, const int h)
{
	if (mWidth != w || mHeight != h)
	{

		mWidth = w;
		mHeight = h;

		if (color_texture_id > 0)	
		{
			glBindTexture(GL_TEXTURE_2D, color_texture_id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void ObjectChannel::ChangeContext()
{
	Cleanup();
}

void ObjectChannel::PreRender()
{
	// clear buffers
	glClearColor(1,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* Before rendering to a window with a stencil buffer, clear the stencil
    buffer to zero and the color buffer to black: */
	
    glClearStencil(0);
    glClearColor(0,0,0,0);
	GLuint flag = 0;
    glStencilMask(~flag);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	// draw scene
	//nvprFillPath(pathObj, 0.0, 1.0, 0.0);

	//initPathFromData();
	/* Use an orthographic path-to-clip-space transform to map the
    [0..500]x[0..400] range of the star's path coordinates to the [-1..1]
    clip space cube: */
	
	glViewport(0, 0, mWidth, mHeight);

    glMatrixLoadIdentityEXT(GL_PROJECTION);
    glMatrixLoadIdentityEXT(GL_MODELVIEW);
	glMatrixLoadIdentityEXT(GL_TEXTURE);
	glMatrixOrthoEXT(GL_PROJECTION, 0, mWidth, 0, mHeight, -1, 1);
	
	/*
	glColor3d(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex2d(0.0, 0.0);
	glVertex2d(500.0, 500.0);
	glEnd();
	*/
}


void initPathFromData(GLuint path, int numCommands, GLubyte *pathCommands, int numCoords, GLshort *pathCoords)
{
    
	glPathCommandsNV(path, numCommands, pathCommands, numCoords, GL_SHORT, pathCoords);
    

    /* Before rendering, configure the path object with desirable path
    parameters for stroking.  Specify a wider 6.5-unit stroke and
    the round join style: */

    glPathParameteriNV(path, GL_PATH_JOIN_STYLE_NV, GL_ROUND_NV);
    glPathParameterfNV(path, GL_PATH_STROKE_WIDTH_NV, 6.5);
}

void nvprFillPath(GLuint path, double r, double g, double b, double a)
{
	const int even_odd = 0;

	//initPathFromData();
	/* Use an orthographic path-to-clip-space transform to map the
    [0..500]x[0..400] range of the star's path coordinates to the [-1..1]
    clip space cube: */
	/*
    glMatrixLoadIdentityEXT(GL_PROJECTION);
    glMatrixLoadIdentityEXT(GL_MODELVIEW);
    glMatrixOrthoEXT(GL_MODELVIEW, 0, 500, 0, 400, -1, 1);
	*/
	/* Stencil the path: */

    glStencilFillPathNV(path, GL_COUNT_UP_NV, 0x1F);

    /* The 0x1F mask means the counting uses modulo-32 arithmetic. In
    principle the star's path is simple enough (having a maximum winding
    number of 2) that modulo-4 arithmetic would be sufficient so the mask
    could be 0x3.  Or a mask of all 1's (~0) could be used to count with
    all available stencil bits.

    Now that the coverage of the star and the heart have been rasterized
    into the stencil buffer, cover the path with a non-zero fill style
    (indicated by the GL_NOTEQUAL stencil function with a zero reference
    value): */

	glPushAttrib(GL_STENCIL_BUFFER_BIT);

    glEnable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
    if (even_odd) {
        glStencilFunc(GL_NOTEQUAL, 0, 0x1);
    } else {
        glStencilFunc(GL_NOTEQUAL, 0, 0x1F);
    }
    glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
    glColor4d(r,g,b, a); // green
    glCoverFillPathNV(path, GL_BOUNDING_BOX_NV);


	glPopAttrib();
}

void nvprStrokePath(GLuint path, const double r, const double g, const double b)
{
 /* Now stencil the path's stroked coverage into the stencil buffer,
     setting the stencil to 0x1 for all stencil samples within the
     transformed path. */

	glStencilStrokePathNV(path, 0x3, 0xF);

         /* Cover the path's stroked coverage (with a hull this time instead
         of a bounding box; the choice doesn't really matter here) while
         stencil testing that writes white to the color buffer and again
         zero the stencil buffer. */

	glPushAttrib(GL_STENCIL_BUFFER_BIT);

	glEnable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);

	glStencilFunc(GL_EQUAL, 3, 0xF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

	glColor3f(r, g, b); // yellow
	glCoverStrokePathNV(path, GL_BOUNDING_BOX_NV);

         /* In this example, constant color shading is used but the application
         can specify their own arbitrary shading and/or blending operations,
         whether with Cg compiled to fragment program assembly, GLSL, or
         fixed-function fragment processing.

         More complex path rendering is possible such as clipping one path to
         another arbitrary path.  This is because stencil testing (as well
         as depth testing, depth bound test, clip planes, and scissoring)
         can restrict path stenciling. */
    glPopAttrib();
}

void ObjectChannel::Render()
{
	//
	if (OverrideDisplay)
	{
		mShowGrid = ShowGrid;
		mColor = ShapeColor;
		mTransparency = ShapeTransparency;
	}

	// render channel to a framebuffer and blur it if needed
	
	PreRender();

	/// TODO: compile all shapes into one display list !

	// draw each shape
	for (int i=0; i<Components.GetCount(); ++i)
	{
		ObjectShape *shape = (ObjectShape*) Components[i];
		FBVector3d	pos, prePos, v;
			
		if (shape->OverrideDisplay)
		{
			shape->Color.GetData(mColor, sizeof(double)*3);
		}
		mColor = FBColor(1.0, 1.0, 1.0);
			/*
		if (mTransparency > 0.0)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
		}
		*/

		shape->UpdateTangents();
		shape->CalculateSegments();
		shape->PrepPathCommandsNV();

		glPushMatrix();
		glMultMatrixd( shape->GetMatrix() );

		if (shape->GetNumberOfCommands() > 0)
		{
			const GLuint path = shape->GetPathObjectNV();

			initPathFromData(path, shape->GetNumberOfCommands(), shape->GetCommandsArray(),
				shape->GetNumberOfCoords()*2, shape->GetCoordsArray() );
			
			if (shape->ClosePath && shape->Fill)
				nvprFillPath( path, mColor[0], mColor[1], mColor[2], 1.0 );
			else
				nvprStrokePath( path, mColor[0], mColor[1], mColor[2] );
		}

		glPopMatrix();
		/*
		if (mTransparency > 0.0)
		{
			glDisable(GL_BLEND);
		}
		*/

		CHECK_GL_ERROR_MOBU();
	}

}

bool ObjectChannel::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ObjectChannel::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if( pStoreWhat == kAttributes )
	{
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	Constructor.
 ************************************************/
ObjectShape::ObjectShape( const char* pName, HIObject pObject ) 
	: FBUserObject( pName, pObject )
{
	FBClassInit;

	FBPropertyPublish(this, ClosePath, "Close Path", nullptr, nullptr);
	FBPropertyPublish(this, Fill, "Fill", nullptr, nullptr);

	FBPropertyPublish(this, OverrideDisplay, "Override Display", nullptr, nullptr);
	FBPropertyPublish(this, Color, "Color", nullptr, nullptr);
	FBPropertyPublish(this, Stroke, "Outline", nullptr, nullptr);
	FBPropertyPublish(this, StrokeColor, "Outline Color", nullptr, nullptr);
	FBPropertyPublish(this, Position, "Position", nullptr, nullptr);
	FBPropertyPublish(this, Rotation, "Rotation", nullptr, nullptr);
	FBPropertyPublish(this, Scale, "Scale", nullptr, nullptr);

	ClosePath = true;
	Fill = true;

	OverrideDisplay = false;
	Color = FBColor(1.0, 1.0, 1.0);
	Scale = FBVector2d(1.0, 1.0);

	Stroke = false;
	StrokeColor = FBColor(0.0, 0.0, 1.0);

	//DisableObjectFlags(kFBFlagBrowsable);

	mPathId = 0;
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectShape::FBCreate()
{
	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ObjectShape::FBDestroy()
{
	ClearPathObjectNV();
}

bool ObjectShape::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ObjectShape::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if( pStoreWhat == kAttributes )
	{
	}
	return true;
}

void ObjectShape::CalculateSmoothTangents(const int index, const FBVector2d &nextPoint, const FBVector2d &prevPoint)
{
	double scale = 1.0;

	FBVector2d tangent, q0, q1;

	VectorSub(prevPoint, nextPoint, tangent);
	scale = 0.25 * VectorLength(tangent);
	VectorNormalize(tangent);

	ObjectKnot *pKnot = (ObjectKnot*) Components[index];
	VectorMult( tangent, scale );

	// store in local coords ( relative to pKnot->Position )
	pKnot->LeftTangent = FBVector2d( -tangent[0], -tangent[1] );
	pKnot->RightTangent = tangent;

	FBVector2d p = pKnot->Position;
	pKnot->incoming = FBVector2d( p[0]-tangent[0], p[1]-tangent[1] );
	pKnot->outgoing = FBVector2d( p[0]+tangent[0], p[1]+tangent[1] );
}


void ObjectShape::UpdateTangents()
{
	if (Components.GetCount() < 2)
		return;

	const bool IsClosePath = ClosePath;
	int num = Components.GetCount();

	for (int i=0; i<num; ++i)
	{
		const ObjectKnotType type = ((ObjectKnot*) Components[i])->Type;

		if ( type == eObjectKnotType_SMOOTH )
		{
			FBVector2d p1, p2;

			// first knot
			if (i==0)
			{
				if (IsClosePath)
				{
					p1 = ( (ObjectKnot*) Components[num-1])->Position;
					p2 = ( (ObjectKnot*) Components[1])->Position;
				}
				else
				{
					p1 = ( (ObjectKnot*) Components[0])->Position;
					p2 = ( (ObjectKnot*) Components[1])->Position;
				}
			}
			else if (i==num-1)
			{
				if (IsClosePath)
				{
					p1 = ( (ObjectKnot*) Components[i-1])->Position;
					p2 = ( (ObjectKnot*) Components[0])->Position;
				}
				else
				{
					p1 = ( (ObjectKnot*) Components[i-1])->Position;
					p2 = ( (ObjectKnot*) Components[i])->Position;
				}
			}
			else
			{
				p1 = ( (ObjectKnot*) Components[i-1])->Position;
				p2 = ( (ObjectKnot*) Components[i+1])->Position;
			}

			CalculateSmoothTangents( i, p1, p2 );
		}
		else if ( type == eObjectKnotType_BEZIER )
		{
			ObjectKnot *pKnot = (ObjectKnot*) Components[i];

			FBVector2d p = pKnot->Position;
			FBVector2d q0 = pKnot->LeftTangent;
			FBVector2d q1 = pKnot->RightTangent;

			pKnot->incoming = FBVector2d( p[0]+q0[0], p[1]+q0[1] );
			pKnot->outgoing = FBVector2d( p[0]+q1[0], p[1]+q1[1] );
		}
		else if ( type == eObjectKnotType_CORNER )
		{
			ObjectKnot *pKnot = (ObjectKnot*) Components[i];

			FBVector2d p = pKnot->Position;
			
			pKnot->LeftTangent = FBVector2d(0.0, 0.0);
			pKnot->RightTangent = FBVector2d(0.0, 0.0);

			pKnot->incoming = p;
			pKnot->outgoing = p;
		}
	}
}

void ObjectShape::CalculateSegments()
{
	if (Components.GetCount() < 2)
		return;

	const bool IsClosePath = ClosePath;
	const int num = Components.GetCount()-1;

	if (IsClosePath == false)
	{
		mSegments.resize(num);
	}
	else
	{
		mSegments.resize(num+1);

		ObjectKnot *pKnot1 = (ObjectKnot*) Components[num];
		ObjectKnot *pKnot2 = (ObjectKnot*) Components[0];

		mSegments[num].Set(pKnot1->Position, pKnot1->outgoing, pKnot2->incoming, pKnot2->Position);

		if (pKnot1->Type != eObjectKnotType_CORNER || pKnot2->Type != eObjectKnotType_CORNER)
			mSegments[num].type = eBezierSegmentTypeCurve;
		else
			mSegments[num].type = eBezierSegmentTypeLine;
	}

	//
	for (int i=0; i<num; ++i)
	{
		ObjectKnot *pKnot1 = (ObjectKnot*) Components[i];
		ObjectKnot *pKnot2 = (ObjectKnot*) Components[i+1];

		mSegments[i].Set(pKnot1->Position, pKnot1->outgoing, pKnot2->incoming, pKnot2->Position);

		if (pKnot1->Type != eObjectKnotType_CORNER || pKnot2->Type != eObjectKnotType_CORNER)
			mSegments[i].type = eBezierSegmentTypeCurve;
		else
			mSegments[i].type = eBezierSegmentTypeLine;
	}
}

void ObjectShape::PrepPathCommandsNV()
{

	if (mSegments.size() == 0)
		return;

	const bool IsClosePath = ClosePath;

	int numCommands = (int) mSegments.size()+1;
	int numCoords = 1;

	if (IsClosePath)
		numCommands += 1;

	mCommands.resize(numCommands);
	mCommands[0] = 'M'; // GL_MOVE_TO_NV;

	for (int i=0; i<mSegments.size(); ++i)
	{
		if (mSegments[i].type == eBezierSegmentTypeCurve)
		{
			numCoords += 3; // 3 more coords
			mCommands[i+1] = 'C'; // curve to
		}
		else
		{
			numCoords += 1; // simple more to specified point
			mCommands[i+1] = GL_LINE_TO_NV;
		}
	}

	if (IsClosePath)
		mCommands[numCommands-1] = 'Z'; // GL_CLOSE_PATH_NV;

	//
	//
	mCoords.resize(numCoords);

	numCoords=0;
	for (int i=0; i<mSegments.size(); ++i)
	{
		BezierSegment &segment = mSegments[i];

		// move to
		if (i==0)
		{
			mCoords[numCoords].x = (GLshort) segment.ctrlpoints[0][0];
			mCoords[numCoords].y = (GLshort) segment.ctrlpoints[0][1];
			numCoords++;
		}

		// line to or curve to
		if (segment.type == eBezierSegmentTypeCurve)
		{
			mCoords[numCoords].x = (GLshort) segment.ctrlpoints[1][0];
			mCoords[numCoords].y = (GLshort) segment.ctrlpoints[1][1];
			numCoords++;
			mCoords[numCoords].x = (GLshort) segment.ctrlpoints[2][0];
			mCoords[numCoords].y = (GLshort) segment.ctrlpoints[2][1];
			numCoords++;
			mCoords[numCoords].x = (GLshort) segment.ctrlpoints[3][0];
			mCoords[numCoords].y = (GLshort) segment.ctrlpoints[3][1];
			numCoords++;
		}
		else
		{
			mCoords[numCoords].x = (GLshort) segment.ctrlpoints[3][0];
			mCoords[numCoords].y = (GLshort) segment.ctrlpoints[3][1];
			numCoords++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
static void ObjectKnot_SetType(HIObject object, ObjectKnotType pValue)
{
	ObjectKnot *pKnot = FBCast<ObjectKnot>(object);
	if (pKnot) pKnot->SetType(pValue);
}

static ObjectKnotType ObjectKnot_GetType(HIObject object)
{
	ObjectKnotType type = ObjectKnot_LINEAR;

	ObjectKnot *pKnot = FBCast<ObjectKnot>(object);
	if (pKnot) type = pKnot->GetType();

	return type;
}
*/
/************************************************
 *	Constructor.
 ************************************************/
ObjectKnot::ObjectKnot( const char* pName, HIObject pObject ) 
	: FBUserObject( pName, pObject )
{
	FBClassInit;

	FBPropertyPublish( this, Type, "Type", nullptr, nullptr ); // ObjectKnot_GetType, ObjectKnot_SetType );

	FBPropertyPublish(this, Position, "Position", nullptr, nullptr);
	FBPropertyPublish(this, LeftTangent, "Left Tangent", nullptr, nullptr);
	FBPropertyPublish(this, RightTangent, "Right Tangent", nullptr, nullptr);
	
	FBPropertyPublish(this, LockTangents, "Lock Tangents", nullptr, nullptr);

	Type = eObjectKnotType_SMOOTH;
	Position = FBVector2d(0.0, 0.0);
	LeftTangent = FBVector2d(0.0, 0.0);
	RightTangent = FBVector2d(0.0, 0.0);
	LockTangents = true;

	Drag = false;
	Selected = false;
	SelectedLeftTangent = false;
	SelectedRightTangent = false;

	//mType = ObjectKnot_LINEAR;
	//DisableObjectFlags(kFBFlagBrowsable);
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectKnot::FBCreate()
{
	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ObjectKnot::FBDestroy()
{
}

bool ObjectKnot::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ObjectKnot::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if( pStoreWhat == kAttributes )
	{
	}
	return true;
}

