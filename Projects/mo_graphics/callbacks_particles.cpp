
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: callbacks_particles.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "callbacks_particles.h"

// TODO: replace classname with internal class id

const bool CGPUParticlesCallback::IsForShader(FBShader *pShader)
{
	if (nullptr == pShader)
		return false;
	return ( 0 == strcmp(pShader->ClassName(), "GPUshader_Particles") );
}

const bool CGPUParticlesCallback::IsForShaderAndPass(FBShader *pShader, const EShaderPass pass)
{
	if (nullptr == pShader)
		return false;
	return ( 0 == strcmp(pShader->ClassName(), "GPUshader_Particles") );
}


bool CGPUParticlesCallback::OnInstanceBegin(const CRenderOptions &options, FBRenderOptions* pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	if (nullptr == pShader)
		return false;

	if (true == pShader->ShaderNeedBeginRender() )
		pShader->ShaderBeginRender(pFBRenderOptions, nullptr);

	return true;
}

void CGPUParticlesCallback::OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo)
{
}


void CGPUParticlesCallback::DetachRenderContext(FBRenderOptions* pFBRenderOptions, FBShader *pShader)
{
	pShader->DetachDisplayContext(pFBRenderOptions, nullptr);
}

// executed once per layer and pass
bool CGPUParticlesCallback::OwnModelShade(const CRenderOptions &options, FBRenderOptions* pFBRenderOptions, FBModel *pModel, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	if (nullptr == pModel || nullptr == pShader)
		return false;

	pShader->ShadeModel(pFBRenderOptions, nullptr, kFBPassLighted);

	return true;
}
