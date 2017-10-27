
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Shader.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Shader.h"
#include <GL\glew.h>
#include "IO\FileUtils.h"
#include "nv_dds\nv_dds.h"

#define SKYBOX_EFFECT				"\\GLSL_FX\\SkyBox.glslfx"

///////////////////////////////////////////////////////////////////////////////////////////////////////
//

Shader::Shader()
{
	fx_Effect = nullptr;
        
	fx_TechBox = nullptr;
	fx_passBox = nullptr;

	fx_TechPano = nullptr;
	fx_passPano = nullptr;

	fx_TechPlane = nullptr;
	fx_passPlane = nullptr;

	fx_EyePos = nullptr;

	fx_FlipH = nullptr;
	fx_FlipV = nullptr;

	fx_UVOffset = nullptr;
	fx_UVScale = nullptr;

	fx_WorldOffset = nullptr;
	fx_WorldScale = nullptr;

	fx_TransparencyFactor = nullptr;

	//
	vbo = 0;
	vao = 0;
}

Shader::~Shader()
{
	clearResources();
}


bool Shader::Initialize()
{
	//
	// load glslfx shader

	FBString effectPath, effectFullName;

	if ( FindEffectLocation( SKYBOX_EFFECT, effectPath, effectFullName ) )
	{

		if (false == loadEffect( effectFullName ) )
		{
			FBMessageBox( "SkyBox shader", "Failed to load skybox shader", "Ok" );
			return false;
		}

		setupVBO();
	}
	else
	{
		FBMessageBox( "SkyBox shader", "Failed to locate effect files", "Ok" );
	}

	return true;
}


bool Shader::setupVBO()
{
	// 36 cubemap + 6 horizontal plane

	const float size = 100.0f;
	const float size2 = 1000.0f;
	float points[] = {
	  -size,  size, -size,
	  -size, -size, -size,
	   size, -size, -size,
	   size, -size, -size,
	   size,  size, -size,
	  -size,  size, -size,
  
	  -size, -size,  size,
	  -size, -size, -size,
	  -size,  size, -size,
	  -size,  size, -size,
	  -size,  size,  size,
	  -size, -size,  size,
  
	   size, -size, -size,
	   size, -size,  size,
	   size,  size,  size,
	   size,  size,  size,
	   size,  size, -size,
	   size, -size, -size,
   
	  -size, -size,  size,
	  -size,  size,  size,
	   size,  size,  size,
	   size,  size,  size,
	   size, -size,  size,
	  -size, -size,  size,
  
	  -size,  size, -size,
	   size,  size, -size,
	   size,  size,  size,
	   size,  size,  size,
	  -size,  size,  size,
	  -size,  size, -size,
  
	  -size, -size, -size,
	  -size, -size,  size,
	   size, -size, -size,
	   size, -size, -size,
	  -size, -size,  size,
	   size, -size,  size,

	   -size2, 0.0, -size2,
	   -size2, 0.0, size2,
	   size2, 0.0, -size2,

	   -size2, 0.0, size2,
	   size2, 0.0, size2,
	   size2, 0.0, -size2,
	};

	if (vbo == 0)
	{
		glGenBuffers(1, &vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData( GL_ARRAY_BUFFER, 3 * 42 * sizeof(float), &points, GL_STATIC_DRAW );

	if (vao == 0)
	{
		glGenVertexArrays (1, &vao);
	}

	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);

	return true;
}


void Shader::clearResources()
{
	if(fx_Effect)
    {
        FBTrace("Desroying previous material Effect\n");
        //fx_EffectMaterial->destroy();
        //or
        FBTrace("=========> Destroying effect\n");
        nvFX::IContainer::destroy(fx_Effect);
        fx_Effect = nullptr;

		fx_passBox = nullptr;
		fx_passPano = nullptr;
		fx_passPlane = nullptr;
    }

	if (vbo > 0)
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}
	if (vao > 0)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
}


//-----------------------------------------------------------------------------
// Load scene effect
//-----------------------------------------------------------------------------
bool Shader::loadEffect(const char *effectFileName)
{
    if(fx_Effect)
    {
        FBTrace("Desroying previous material Effect\n");
        //fx_EffectMaterial->destroy();
        //or
        FBTrace("=========> Destroying effect\n");
        nvFX::IContainer::destroy(fx_Effect);
        fx_Effect = nullptr;
    }
    FBTrace("Creating Effect (material)\n");
    fx_Effect = nvFX::IContainer::create("textureEffect");
    bool bRes = nvFX::loadEffectFromFile(fx_Effect, effectFileName);
        
    if(!bRes)
    {
        FBTrace("Failed\n");
        return false;
    }
    FBTrace("Loaded\n");
    //
    // Let's keep track in interface pointers everything, for purpose of clarity
    //
    
	fx_TechBox = fx_Effect->findTechnique("t0");
    if(fx_TechBox && (!fx_TechBox->validate()))
        return false;

	fx_passBox = fx_TechBox->getPass(0);
	if (fx_passBox == nullptr)
		return false;

    fx_TechPano = fx_Effect->findTechnique("tp");
    if(fx_TechPano && (!fx_TechPano->validate()))
        return false;

	fx_passPano = fx_TechPano->getPass(0);
	if (fx_passPano == nullptr)
		return false;

	fx_TechPlane = fx_Effect->findTechnique("t1");
    if(fx_TechPlane && (!fx_TechPlane->validate()))
        return false;

	fx_passPlane = fx_TechPlane->getPass(0);
	if (fx_passPlane == nullptr)
		return false;

	fx_MVP = fx_Effect->findUniform("MVP");
	if (fx_MVP == nullptr)
		return false;

	fx_ModelView = fx_Effect->findUniform("ModelView");
	if (fx_ModelView == nullptr)
		return false;

	fx_EyePos = fx_Effect->findUniform("EyePos");
	if (fx_EyePos == nullptr)
		return false;

	fx_FlipH = fx_Effect->findUniform("FlipH");
	if (fx_FlipH == nullptr)
		return false;

	fx_FlipV = fx_Effect->findUniform("FlipV");
	if (fx_FlipV == nullptr)
		return false;

	fx_UVOffset = fx_Effect->findUniform("UVOffset");
	if (fx_UVOffset == nullptr)
		return false;

	fx_UVScale = fx_Effect->findUniform("UVScale");
	if (fx_UVScale == nullptr)
		return false;

	fx_WorldOffset = fx_Effect->findUniform("WorldOffset");
	if (fx_WorldOffset == nullptr)
		return false;

	fx_WorldScale = fx_Effect->findUniform("WorldScale");
	if (fx_WorldScale == nullptr)
		return false;

	fx_TransparencyFactor = fx_Effect->findUniform("TransparencyFactor");
	if (fx_TransparencyFactor == nullptr)
		return false;

	fx_CubeTexture = fx_Effect->findUniform("cube_texture");
	if (fx_CubeTexture) fx_CubeTexture->setSamplerUnit(0);

	fx_PanoTexture = fx_Effect->findUniform("panorama_texture");
	if (fx_PanoTexture) fx_PanoTexture->setSamplerUnit(1);

	//return true;
	return true;
}