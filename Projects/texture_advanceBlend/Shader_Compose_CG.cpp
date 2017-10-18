
#ifdef LINUX
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#endif

#include "Shader_CG.h"


#ifdef USE_CG
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif

#ifdef _WIN32
	#include <windows.h>
#endif

#ifdef LINUX
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#endif

#include <math.h>
#include <stdio.h>

#include "graphics\checkGLerror.h"

namespace GraphicsCG
{

#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

#ifndef GL_TEXTURE0_ARB
#define GL_TEXTURE0_ARB							0x84c0
#endif

#ifndef GL_ARRAY_BUFFER_ARB
#define GL_ARRAY_BUFFER_ARB 0x8892
#endif

namespace
{
#ifdef _WIN32
    typedef void (APIENTRYP PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
    typedef void (APIENTRYP PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
	typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
    PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;
    PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = NULL;
	PFNGLBINDBUFFERPROC glBindBufferARB =  NULL;
#endif
	
    void SetTexture(CGparameter pParamTextureValidVS, CGparameter pTexture, CGparameter pMatrix, unsigned int pTextureObject, const double* pTextureMatrix)
    {
        if (pTextureObject > 0)
        {
            cgSetParameter1i( pParamTextureValidVS, 1 );
            //cgSetParameter1i( pParamTextureValidPS, 1 );
            cgGLSetTextureParameter( pTexture, pTextureObject );
            cgGLEnableTextureParameter( pTexture );
            cgSetMatrixParameterdc( pMatrix, pTextureMatrix );
        }
        else
        {
            cgSetParameter1i( pParamTextureValidVS, 0 );
            //cgSetParameter1i( pParamTextureValidPS, 0 );
        }
    }

    void SetMaterial(CGparameter pParamAmbient, CGparameter pParamDiffuse, CGparameter pParamSpecular, CGparameter pParamEmissive, 
                     const float* pAmbient, const float* pDiffuse, const float* pSpecular, const float* pEmission, const float pShininess)
    {
        cgSetParameter4fv( pParamAmbient, pAmbient );
        cgSetParameter4fv( pParamDiffuse, pDiffuse );
        float pBuffer[4];
        pBuffer[0] = pSpecular[0];
        pBuffer[1] = pSpecular[1];
        pBuffer[2] = pSpecular[2];
        pBuffer[3] = pShininess;
        cgSetParameter4fv( pParamSpecular, pBuffer );
        cgSetParameter4fv( pParamEmissive, pEmission );
    }

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader

Shader::Shader()
{
}

Shader::~Shader()
{
}

// Call after contruction.
bool Shader::Initialize(const char* pVertexShaderPath, const char* pPixelShaderPath, const char* pVertexShaderOptions, const char* pPixelShaderOptions)
{
	return false;
}

void Shader::BeginShading(const LightSet	&lightData)
{
}

void Shader::EndShading()
{
}

//void AssignCompose( CGPUProjectorsCompose *_compose );

// Will be called when the rendering will change its material state.
void Shader::SwitchProjectors(const ProjectorSet &data, const GLuint *projIds, const GLuint *maskIds)
{
}

void Shader::SwitchShader(const ShaderDATA &shader)
{
}

void Shader::SwitchMaterial(const bool idBufferRender, const MaterialVertex &vertexData, const MaterialDATA &data)
{
}

void Shader::SwitchModel (const bool idBufferRender, const double *matrix, const float uniquecolor[4] )
{
}

void Shader::UploadModelMatrix(const double *pModelMatrix)
{
}

// Upload ModelView Matrix Array for Draw Instanced.
void Shader::UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ShaderImpl

class ShaderImpl : public Shader
{
public:

	ShaderImpl();
	virtual ~ShaderImpl();

    // Call after contruction.
    bool Initialize(const char* pVertexShaderPath, const char* pPixelShaderPath, const char* pVertexShaderOptions = NULL, const char* pPixelShaderOptions = NULL);

    void BeginShading(const LightSet	&lightData);
    void EndShading();

	//void AssignCompose( CGPUProjectorsCompose *_compose );

    // Will be called when the rendering will change its material state.
	void SwitchProjectors(const ProjectorSet &data, const GLuint *projIds, const GLuint *maskIds);
	void SwitchShader(const ShaderDATA &shader);
	void SwitchMaterial(const bool idBufferRender, const MaterialVertex &vertexData, const MaterialDATA &data);
	void SwitchModel (const bool idBufferRender, const double *matrix, const float uniquecolor[4] );

    void UploadModelMatrix(const double *pModelMatrix);
	// Upload ModelView Matrix Array for Draw Instanced.
    void UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount);
    // setup per model parameters. 
        
protected:

    static void ShowError(const char* pText);
    // Call those two functions around rendering.
    virtual void BindShaderPrograms();
    virtual void UnbindShaderPrograms();
    virtual void UnsetTextures();

    // Methods to create shaders based on file input.
	CGprogram CreateShader(CGprofile pProfile, const char* pPath, const char** pArgs);
    bool CreateVertexShader( const char* pPath, const char* pOptions );
	bool CreatePixelShader( const char* pPath, const char* pOptions );

    // Global context stuff.
    CGcontext	mContext;
    CGprofile	mVertexProfile;
    CGprofile	mPixelProfile;
	CGprogram	mVertexShader;
	CGprogram	mPixelShader;

	// Parameters needed to upload data to cg shaders
    CGbuffer    mParamModelViewArrayBuffer;
    int         mParamModelViewArrayBufferOffset;
		
	CGparameter	mParamColorTextureMatrix;
    CGparameter mParamColorTextureValidVS;

	CGparameter	mParamModelMatrix;

	//
	// pixel shader parameters
		
    CGparameter				mParamColorTexture;			// sampler
	CGparameter				mParamUniqueColor;			// for render color id

	std::auto_ptr<UniformBufferCG>	mBufferMaterial;
	std::auto_ptr<UniformBufferCG>	mBufferLighting;
	std::auto_ptr<UniformBufferCG>	mBufferShading;
        
private:

        ShaderImpl(const Shader&);
        ShaderImpl& operator = (const ShaderImpl&);

		std::auto_ptr<CGPUProjectorsModelCg>	mProjectors;

};

//

Shader *Shader::CreateNewShader()
{
	return new ShaderImpl();
}


//

ShaderImpl::ShaderImpl()
	: Shader()
	//: mProjectors(nullptr)
	, mBufferMaterial( new UniformBufferCG() )
	, mBufferLighting( new UniformBufferCG() )
	, mBufferShading( new UniformBufferCG() )
	, mProjectors( new CGPUProjectorsModelCg() )
{
    static bool init = true;
    if ( init )
    {
#ifdef _WIN32
         glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
         glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
		 glBindBufferARB = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBufferARB");
#endif
        init = false;
		/*
		int lW = 64;
		int lH = 64;

		struct COLOR
		{
			BYTE r;
			BYTE g;
			BYTE b;
			BYTE a;

			void Grey()
			{
				r=g=b=a=128;
			}
		};
		COLOR *lImageBits = new COLOR[lW*lH];
		for (int i=0; i<lW*lH; ++i)
			lImageBits[i].Grey();

		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lW, lH, 0, GL_RGBA, GL_UNSIGNED_BYTE, lImageBits);
		glBindTexture(GL_TEXTURE_2D, 0);

		mVideoMemory.SetObjectImageSize(lW, lH);
		mVideoMemory.TextureOGLId = mTexId; // set external OGL texture ID.
		*/
    }
}

ShaderImpl::~ShaderImpl()
{
    // Destroy the cg context (and subsequently everything in it)
    cgDestroyContext( mContext );
}

bool ShaderImpl::Initialize(const char* pVertexShaderPath, const char* pPixelShaderPath, const char* pVertexShaderOptions, const char* pPixelShaderOptions)
{
    // Create the cg context (need to create anything in cg)
    mContext = cgCreateContext();
    if( !mContext )
    {
        ShowError("Could not create CG context");
        return false;
    }

	cgSetContextBehavior(mContext,CG_BEHAVIOR_3100);
	if (cgGetContextBehavior(mContext)<CG_BEHAVIOR_3100) {
		fprintf(stderr, "%s: Cg 3.1 behavior required.\n", "ProjText shader" );    
		return false;
	}

    // Setup for best shader model
    CGprofile lVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
    CGprofile lPixelProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

    if( !cgGLIsProfileSupported(lVertexProfile) ) 
    {
        ShowError("Vertex programming extensions not supported");
        return false;
    }
    if( !cgGLIsProfileSupported(lPixelProfile) ) 
    {
        ShowError("Pixel programming extensions not supported");
        return false;
    }

    cgGLSetOptimalOptions(lVertexProfile);
    cgGLSetOptimalOptions(lPixelProfile);
    mVertexProfile = lVertexProfile;
    mPixelProfile = lPixelProfile;

    // Create shaders
    if( !CreateVertexShader( pVertexShaderPath, pVertexShaderOptions ) )
    {
        return false;
    }
    if( !CreatePixelShader( pPixelShaderPath, pPixelShaderOptions ) )
    {
        return false;
    }

	cgGLSetDebugMode( GL_TRUE );	// Enables debug mode

	CHECK_GL_ERROR();

    return true;
}

void ShaderImpl::BeginShading(const GraphicsCG::LightSet &lightData)
{
    BindShaderPrograms();

	mBufferLighting->UpdateData( 0, sizeof(GraphicsCG::LightSet), &lightData );
}

void ShaderImpl::EndShading()
{
    UnsetTextures();
    UnbindShaderPrograms();
}

void ShaderImpl::BindShaderPrograms()
{
	cgGLBindProgram(mVertexShader);	//Bind program. (You can bind only one vertex and one fragment program at a time)
	cgGLBindProgram(mPixelShader);

    // Set the pixel/vertex shader profiles
    cgGLEnableProfile(mVertexProfile);
    cgGLEnableProfile(mPixelProfile);
}

void ShaderImpl::UnbindShaderPrograms()
{
    cgGLDisableProfile(mVertexProfile);
    cgGLDisableProfile(mPixelProfile);

    cgGLUnbindProgram(mVertexProfile);
    cgGLUnbindProgram(mPixelProfile);
}

void ShaderImpl::UnsetTextures()
{
    // Deactivate all.
	cgGLDisableTextureParameter( mParamColorTexture );
	
	mProjectors->UnBind();

//    cgGLDisableTextureParameter( mParamColorTexture );
//    cgGLDisableTextureParameter( mParamNormalMap );
}

CGprogram ShaderImpl::CreateShader(CGprofile pProfile, const char* pPath, const char** pArgs)
{
    // Create shader based file.
    CGprogram lProgram = cgCreateProgramFromFile( mContext, CG_SOURCE, pPath, 
                                                  pProfile, NULL, pArgs );

    // If shader was created ok, load it so it's ready to bind at render time
    if( lProgram ) 
    {
        cgGLLoadProgram( lProgram );
    }	
    else
    {
        // Throw an error if something went wrong
        ShowError( cgGetLastListing(mContext) );
    }

    return lProgram;
}

bool ShaderImpl::CreateVertexShader( const char* pPath, const char* pOptions )
{
    const char* args[2] = { pOptions, NULL };

    // Create it.
	mVertexShader = CreateShader(mVertexProfile, pPath, args);
    if ( !mVertexShader )
    {
        return false;
    }

	// Now that the shader are created, store the parameters so
	// we can upload data to them at render time
    //mParamModelViewArrayBuffer      = cgCreateBuffer(mContext, kMaxDrawInstancedSize * 4 * 4 * sizeof(double), NULL, CG_BUFFER_USAGE_DYNAMIC_DRAW);
    //cgSetProgramBuffer(mVertexShader, cgGetParameterBufferIndex(cgGetNamedParameter(mVertexShader, "ModelViewArrayBuffer")), mParamModelViewArrayBuffer);
    //mParamModelViewArrayBufferOffset = cgGetParameterBufferOffset(cgGetNamedParameter(mVertexShader, "ModelViewArrayBuffer"));

	mParamModelMatrix = cgGetNamedParameter( mVertexShader, "ModelMatrix" );

    mParamColorTextureValidVS   = cgGetNamedParameter( mVertexShader, "ColorTextureValid" );
    mParamColorTextureMatrix    = cgGetNamedParameter( mVertexShader, "ColorTextureMatrix" );
	
	assert(mParamModelMatrix);
    assert(mParamColorTextureValidVS);
    assert(mParamColorTextureMatrix);

    return true;
}

bool ShaderImpl::CreatePixelShader( const char* pPath, const char* pOptions )
{
    // Prepare compilation option.
    char lMaxLightArg[64];
	sprintf_s(lMaxLightArg, sizeof(char)*64, "-DMAX_LIGHT_COUNT=%d", MAX_LIGHT_COUNT);
	char lMaxProjArg[64];
	sprintf_s(lMaxProjArg, sizeof(char)*64, "-DMAX_PROJECTORS_COUNT=&d", MAX_PROJECTORS_COUNT);

    const char* args[4] = { lMaxLightArg, lMaxProjArg, pOptions, NULL };

    // Create it.
    mPixelShader = CreateShader(mPixelProfile, pPath, args);
    if ( !mPixelShader )
    {
        return false;
    }

    // Now that the shader are created, store the parameters so
    // we can upload data to them at render time

	mParamColorTexture  = cgGetNamedParameter( mPixelShader, "colorSampler" );
	mParamUniqueColor	= cgGetNamedParameter( mPixelShader, "uniquecolor" );
	
	if (false == mBufferMaterial->InitGL(mContext, mPixelShader, "MaterialBuffer", sizeof(MaterialDATA) ) )
		return false;

	if (false == mBufferLighting->InitGL(mContext, mPixelShader, "LightsBuffer", sizeof(LightSet) ) )
		return false;

	if (false == mBufferShading->InitGL(mContext, mPixelShader, "ShaderBuffer", sizeof(ShaderDATA) ) )
		return false;
		
    assert(mParamColorTexture);
	assert(mParamUniqueColor);
    
	if (false == mProjectors->InitParams( mContext, mPixelShader ) )
		return false;

	return true;
}

void ShaderImpl::SwitchProjectors(const ProjectorSet &data, const GLuint *projIds, const GLuint *maskIds)
{
	mProjectors->UploadBuffer(data);
	mProjectors->Bind(data.numberOfProjectors, projIds, 2, maskIds);
}

void ShaderImpl::SwitchShader(const ShaderDATA &shader)
{
	mBufferShading->UpdateData( 0, sizeof(ShaderDATA), &shader );

	CHECK_GL_ERROR();
}

void ShaderImpl::SwitchMaterial(const bool idBufferRender, const MaterialVertex &vertexData, const GraphicsCG::MaterialDATA &data)
{
	CHECK_GL_ERROR();

    if (idBufferRender)
    {
//            SetTexture(mParamNormalMapValidVS, mParamNormalMapValidPS, mParamNormalMap, mParamNormalMapMatrix, 0, NULL);
    }
    else
    {
		mBufferMaterial->UpdateData( 0, sizeof(GraphicsCG::MaterialDATA), &data );

		if (vertexData.diffuseTexid > 0)
		{
			SetTexture(mParamColorTextureValidVS, mParamColorTexture, mParamColorTextureMatrix, vertexData.diffuseTexid, vertexData.diffuseMatrix );
		}
		else
		{
			SetTexture(mParamColorTextureValidVS, mParamColorTexture, mParamColorTextureMatrix, 0, NULL);
		}
    }   

	CHECK_GL_ERROR();
}

void ShaderImpl::UploadModelMatrix(const double *pModelMatrix)
{
	cgSetMatrixParameterdc( mParamModelMatrix, pModelMatrix );	
}

void ShaderImpl::UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount)
{
    //cgSetBufferSubData(mParamModelViewArrayBuffer, mParamModelViewArrayBufferOffset, 4 * 4 * sizeof (double) * pCount, pModelViewMatrixArray);
}

void ShaderImpl::SwitchModel (const bool idBufferRender, const double *matrix, const float uniquecolor[4] )
{
	UploadModelMatrix(matrix);

	if (idBufferRender)
	{
		cgSetParameter4fv( mParamUniqueColor, uniquecolor );
	}
}

void ShaderImpl::ShowError(const char* pText)
{
    const char* lErrStr = ( pText ) ? pText : cgGetErrorString(cgGetError());
    printf("Cg Error - %s\n", lErrStr );
}

} // Graphics namespace
