
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: textures_effect.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "textures_effect.h"
#include "FileUtils.h"

#define TEXTURES_EFFECT "TexturesBrowser.glslfx"


//////////////////////////////////////////////////////////////////////////
//

ORView3DEffect::ORView3DEffect()
{
	fx_Effect = nullptr;
        
	fx_Tech = nullptr;
	fx_TechBackground = nullptr;
	fx_pass = nullptr;
	
	fx_proj = nullptr;
	fx_mvp = nullptr;
	fx_quadSize = nullptr;
	fx_mouse = nullptr;
	fx_Pan = nullptr;
	fx_Height = nullptr;

	mFragmentProgram = 0;
	mTexturesLoc = 0;

	//
	// load glslfx shader

	FBString effectPath, effectFullName;
	
	if ( FindEffectLocation(FBString("\\GLSLFX\\", TEXTURES_EFFECT), effectPath, effectFullName) )
	{
		if (false == loadEffect( effectFullName ) )
			FBMessageBox( "Textures Browser", "Failed to load textures browser effect", "Ok" );
	}
	else
	{
		FBMessageBox( "Textures Browser", "Failed to locate effect file", "Ok" );
	}
}

ORView3DEffect::~ORView3DEffect()
{

}


//-----------------------------------------------------------------------------
// Load scene effect
//-----------------------------------------------------------------------------
bool ORView3DEffect::loadEffect(const char *effectFileName)
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
    
    fx_Tech = fx_Effect->findTechnique("t0");
    if(fx_Tech && (!fx_Tech->validate()))
        return false;

	fx_TechBackground = fx_Effect->findTechnique("background");
    if(fx_TechBackground && (!fx_TechBackground->validate()))
        return false;

	fx_pass = fx_Tech->getPass(0);
	if (fx_pass == nullptr)
		return false;

	fx_proj = fx_Effect->findUniform("Proj");
	if (fx_proj == nullptr)
		return false;

	fx_mvp = fx_Effect->findUniform("MVP");
	if (fx_mvp == nullptr)
		return false;

	fx_quadSize = fx_Effect->findUniform("gQuadSize");
	if (fx_quadSize == nullptr)
		return false;

	fx_mouse = fx_Effect->findUniform("gMouse");
	if (fx_mouse == nullptr)
		return false;

	fx_Pan = fx_Effect->findUniform("gPan");
	if (fx_Pan == nullptr)
		return false;

	fx_Height = fx_Effect->findUniform("gHeight");
	if (fx_Height == nullptr)
		return false;

	//return true;
	return PrepTexturesUniform();
}


bool ORView3DEffect::PrepTexturesUniform()
{
	// Compile GLSL shader using sampler uniform <u>.  The shader itself
    // needs no special #extension directive as long as <u> is a uniform in
    // the default partition.  Link the program, and query the location of
    // <u>, which we will store in <location>.
	
	mFragmentProgram = 0;
	mTexturesLoc = 0;

	if (fx_pass == nullptr)
		return false;

	nvFX::IProgram *glslProgram = nullptr;
    bool bSp;
	nvFX::IProgramPipeline *glslProgramPipeline = fx_pass->getExInterface()->getProgramPipeline(0);
    if(glslProgramPipeline)
    {
		glslProgram = glslProgramPipeline->getShaderProgram(2);	// Fragment glsl program !
        bSp = true;
    } else {
		glslProgram = fx_pass->getExInterface()->getProgram(-1);
        bSp = false;
    }
    mFragmentProgram = (glslProgram != nullptr) ? glslProgram->getProgram() : 0;

	if (mFragmentProgram > 0)
	{
		mTexturesLoc = glGetUniformLocation( mFragmentProgram, "texAddress" );
	}

	return true;
}

void ORView3DEffect::UpdateTextureHandles()
{
	// Compile GLSL shader using sampler uniform <u>.  The shader itself
    // needs no special #extension directive as long as <u> is a uniform in
    // the default partition.  Link the program, and query the location of
    // <u>, which we will store in <location>.
	/*
	if ( (mFragmentProgram > 0) && (mTexturesLoc > 0) && (mTexturesLoc < 4096) )
	{
		GPUContentManager::instance().BindTextures( mFragmentProgram, mTexturesLoc );
	}
	*/
}