
#include "OpenColorIO/OpenColorIO.h"
namespace OCIO = OCIO_NAMESPACE;

#include "compositeMaster_effectLUT.h"
#include "shared_misc.h"
#include "utils\CheckGLError.h"

#include <sstream>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Effect 3D LUT

FBClassImplementation2(EffectLUT)
FBUserObjectImplement(EffectLUT, "Effect 3d LUT", EFFECT_ICON);			//Register UserObject class
FBElementClassImplementation2(EffectLUT, "3d LUT", EFFECT_ICON);		//Register to the asset system

/************************************************
 *	Constructor.
 ************************************************/
void EffectLUT::SetLoad(HIObject object, bool value)
{
	EffectLUT *pBase = FBCast<EffectLUT>(object);
	if (pBase && value) 
	{
		pBase->DoLoad();
	}
}

void EffectLUT::SetEdgeSize(HIObject object, int value)
{
	EffectLUT *pBase = FBCast<EffectLUT>(object);
	if (pBase) 
	{
		if (value < 1) value = 1;
		if (value > 128) value = 128;

		pBase->mLutEdgeSize = value;
		pBase->LutSize.SetPropertyValue(value);

		pBase->AllocateLut3D();
		pBase->mLutFileLoaded = "";
	}
}

EffectLUT::EffectLUT( const char* pName, HIObject pObject ) 
	: ObjectCompositeBase( pName, pObject )
{
	FBClassInit;
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool EffectLUT::FBCreate()
{
	InitParams();
	
	FBPropertyPublish(this, Weight, "Weight", nullptr, nullptr);
	FBPropertyPublish(this, LutSize, "LUT Size", nullptr, EffectLUT::SetEdgeSize);
	FBPropertyPublish(this, FileName, "FileName", nullptr, nullptr);
	FBPropertyPublish(this, Load, "Load", nullptr, EffectLUT::SetLoad);

	Weight = 100.0;
	Weight.SetMinMax(0.0, 100.0, true, true);

	mLutEdgeSize = 64;
	LutSize = 64;
	FileName = "";

	mLut3dTexID = 0;
	mLutEdgeSizeCache = 0;

	mExposureFStop = 0.0f;
	mDisplayGamma = 1.0f;

	mLutFileLoaded = "";
	mNeedUpdate = true;

	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/

void EffectLUT::FBDestroy()
{
	if (mLut3dTexID > 0)
	{
		glDeleteTextures(1, &mLut3dTexID);
		mLut3dTexID = 0;
	}

	ParentClass::FBDestroy();	
}


void EffectLUT::InitParams()
{
	mFragShader = 0;
	mProgram = 0;
	mWeightLoc = 0;

	mLut3dCacheid = "";
	mShaderCacheid = "";
	mLutFileLoaded = "";
}

void EffectLUT::AllocateLut3D()
{
	if (mLutEdgeSize == mLutEdgeSizeCache)
		return;

	if (mLut3dTexID > 0)
	{
		glDeleteTextures( 1, &mLut3dTexID );
		mLut3dTexID = 0;
	}

	glGenTextures( 1, &mLut3dTexID );

	int num3Dentries = 3 * mLutEdgeSize*mLutEdgeSize*mLutEdgeSize;

	mLut3d.resize(num3Dentries);
	memset( mLut3d.data(), 0, sizeof(float)*num3Dentries );

	glBindTexture(GL_TEXTURE_3D, mLut3dTexID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, mLutEdgeSize, mLutEdgeSize, mLutEdgeSize,
		0, GL_RGB, GL_FLOAT, mLut3d.data() );

	glBindTexture(GL_TEXTURE_3D, 0);

	mLutEdgeSizeCache = mLutEdgeSize;
	mNeedUpdate = true;
}

const GLuint EffectLUT::CompileShaderText(const GLenum shaderType, const char *text)
{
	GLuint shader;
	GLint stat;

	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, (const GLchar **) &text, nullptr);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &stat);

	if (!stat)
	{
		GLchar log[1024];
		GLsizei len;
		glGetShaderInfoLog(shader, 1024, &len, log);
		printf( "Error: problem compiling shader: %s\n", log );
		return 0;
	}

	return shader;
}

const GLuint EffectLUT::LinkShaders(const GLuint fragShader)
{
	if (!fragShader) return 0;

	GLuint program = glCreateProgram();

	if (fragShader)
		glAttachShader(program, fragShader);
	glLinkProgram(program);

	/* check link */
	{
		GLint stat;
		glGetProgramiv(program, GL_LINK_STATUS, &stat);
		if (!stat)
		{
			GLchar log[1024];
			GLsizei len;
			glGetProgramInfoLog(program, 1024, &len, log);
			printf(" Shader Link error: \n%s\n", log );
			return 0;
		}
	}

	return program;
}

const char *g_fragShaderText = ""
	"\n"
	"uniform sampler2D tex1;\n"
	"uniform sampler3D tex2;\n"
	"uniform float weight;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	vec4 col = texture2D(tex1, gl_TexCoord[0].st);\n"
	"	vec4 result = OCIODisplay(col, tex2);\n"
	"	gl_FragColor = mix(col, result, weight);\n"
	"}\n";

bool EffectLUT::UpdateOCIOGLState()
{
	const char *filename = FileName;
	OCIO::ConstProcessorRcPtr processor;

	try
	{
		OCIO::ConstConfigRcPtr	config = OCIO::GetCurrentConfig();

		OCIO::FileTransformRcPtr transform = OCIO::FileTransform::Create();
		transform->setSrc(filename);

		transform->setDirection(OCIO::TRANSFORM_DIR_FORWARD);
		transform->setInterpolation(OCIO::INTERP_LINEAR);
	
		processor = config->getProcessor(transform, OCIO::TRANSFORM_DIR_FORWARD);
	}
	catch(OCIO::Exception &e)
	{
		printf(e.what() );
		return false;
	}
	catch(...)
	{
		return false;
	}

	// Step 1: Create a GPU Shader Description
	OCIO::GpuShaderDesc shaderDesc;
	shaderDesc.setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_0);
	shaderDesc.setFunctionName("OCIODisplay");
	shaderDesc.setLut3DEdgeLen(mLutEdgeSize);

	// Step 2: compute the 3D LUT
	std::string lut3dCacheID = processor->getGpuLut3DCacheID(shaderDesc);
	if (lut3dCacheID != mLut3dCacheid)
	{
		mLut3dCacheid = lut3dCacheID;
		processor->getGpuLut3D(mLut3d.data(), shaderDesc);

		glBindTexture(GL_TEXTURE_3D, mLut3dTexID);
		glTexSubImage3D(GL_TEXTURE_3D, 0,
			0, 0, 0,
			mLutEdgeSize, mLutEdgeSize, mLutEdgeSize,
			GL_RGB, GL_FLOAT, mLut3d.data() );
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	// Step 3: compute the shader
	std::string shaderCacheID = processor->getGpuShaderTextCacheID(shaderDesc);
	if (mProgram == 0 || shaderCacheID != mShaderCacheid)
	{
		mShaderCacheid = shaderCacheID;

		std::ostringstream os;
		os << processor->getGpuShaderText(shaderDesc) << "\n";
		os << g_fragShaderText;

		if (mFragShader) glDeleteShader(mFragShader);
		mFragShader = CompileShaderText(GL_FRAGMENT_SHADER, os.str().c_str() );
		if (mProgram) glDeleteProgram(mProgram);
		mProgram = LinkShaders(mFragShader);
	}

	glUseProgram(mProgram);
	glUniform1i(glGetUniformLocation(mProgram, "tex1"), 0);
	glUniform1i(glGetUniformLocation(mProgram, "tex2"), 1);
	mWeightLoc = glGetUniformLocation(mProgram, "weight");
	glUniform1f(mWeightLoc, 1.0f);
	glUseProgram(0);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
void EffectLUT::CopyFrom(ObjectCompositeBase *pSourceNode)
{
	if ( FBIS(pSourceNode, EffectLUT) )
	{
		EffectLUT *pSourceEffect = (EffectLUT*) pSourceNode;

		PropertyCopy( Active, pSourceEffect->Active );
		
	}
}
/*
bool EffectLUT::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool EffectLUT::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if( pStoreWhat == kAttributes )
	{
	}
	return true;
}
*/

void EffectLUT::DoLoad()
{
	FBFilePopup		dialog;

	dialog.Style = kFBFilePopupOpen;
	dialog.Caption = "Please choose a LUT file";
	dialog.FullFilename = FileName;

	if (dialog.Execute() )
	{
		FileName = dialog.FullFilename;
		Active = true;
		mNeedUpdate = true;
	}
}

void EffectLUT::LoadFromFile(const char *filename)
{
	/*
	try
	{
		OCIO::ConstConfigRcPtr	config = OCIO::GetCurrentConfig();

		OCIO::FileTransformRcPtr transform = OCIO::FileTransform::Create();
		transform->setSrc(filename);

		transform->setCCCId(m_cccid.c_str());

		transform->setDirection(OCIO::TRANSFORM_DIR_FORWARD);
		transform->setInterpolation(OCIO::INTERP_LINEAR);

		m_processor = config->getProcessor(transform, OCIO::TRANSFORM_DIR_FORWARD);
	}
	catch(OCIO::Exception &e)
	{
		printf(e.what() );
		return;
	}
	*/
}

void EffectLUT::DoReloadShader()
{
}

void EffectLUT::Render()
{
	ObjectCompositeBase::Render();

	//
	//
	//
	
	if ( Active && mNeedUpdate )
	{
		AllocateLut3D();

		if (strcmp(mLutFileLoaded.c_str(), FileName) != 0)
		{
			Active = UpdateOCIOGLState();
			mLutFileLoaded = FileName;
		}
		mNeedUpdate = false;
	}

	CHECK_GL_ERROR();

	if (Active)
		Active = InitFrameBuffer(renderWidth, renderHeight);

	if (Active == false || Components.GetCount() != 1) 
		return;

	GLuint texId = ((ObjectCompositeBase*) Components[0])->getColorTextureId();
	
	mBuffer->Bind();

	CHECK_GL_ERROR();
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, mLut3dTexID);
	
	glUseProgram( mProgram );
	if (mWeightLoc > 0)
		glUniform1f( mWeightLoc, 0.01f * Weight );

	drawOrthoQuad2d( renderWidth, renderHeight );

	glUseProgram( 0 );
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	CHECK_GL_ERROR();

	mBuffer->UnBind();
}

void EffectLUT::CleanupRender()
{
	if (mFragShader) {
		glDeleteShader(mFragShader);
		mFragShader = 0;
	}
	if (mProgram) {
		glDeleteProgram(mProgram);
		mProgram = 0;
	}
	
	InitParams();

	mNeedUpdate = true;

	ObjectCompositeBase::CleanupRender();
}