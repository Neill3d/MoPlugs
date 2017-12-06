
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
	bool lSuccess = false;
	if (mInternalClassId > 0)
	{
		lSuccess = (nullptr!=pShader && pShader->Is( mInternalClassId ) );	
	}
	else if ( 0 == strcmp(pShader->ClassName(), "GPUshader_Particles") )
	{
		FBProperty *pProp = pShader->PropertyList.Find("Internal ClassId");
		if (nullptr != pProp)
		{
			mInternalClassId = pProp->AsInt();
			lSuccess = (pShader->Is( mInternalClassId ) );
		}
	}

	return lSuccess;
	//return ( 0 == strcmp(pShader->ClassName(), "GPUshader_Particles") );
}

const bool CGPUParticlesCallback::IsForShaderAndPass(FBShader *pShader, const EShaderPass pass)
{
	bool lSuccess = false;
	if (mInternalClassId > 0)
	{
		lSuccess = (nullptr!=pShader && pShader->Is( mInternalClassId ) );	
	}
	else if ( 0 == strcmp(pShader->ClassName(), "GPUshader_Particles") )
	{
		FBProperty *pProp = pShader->PropertyList.Find("Internal ClassId");
		if (nullptr != pProp)
		{
			mInternalClassId = pProp->AsInt();
			lSuccess = (pShader->Is( mInternalClassId ) );
		}
	}

	return lSuccess;
	/*
	if (nullptr == pShader)
		return false;
	return ( 0 == strcmp(pShader->ClassName(), "GPUshader_Particles") );
	*/
}


bool CGPUParticlesCallback::OnInstanceBegin(const CRenderOptions &options, FBRenderOptions* pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo)
{
	if (nullptr == pShader)
		return false;

	const ERenderGoal goal = options.GetGoal();
	const EShaderPass pass = options.GetPass();

	bool skipShader = false;
	FBProperty *pProp = pShader->PropertyList.Find( "Transparency" );
	if ( nullptr != pProp )
	{
		const FBAlphaSource alphaSource = (FBAlphaSource) pProp->AsInt();

		switch(goal)
		{
		case eRenderGoalShading:
			switch(pass)
			{
			case eShaderPassOpaque:
				skipShader = (kFBAlphaSourceNoAlpha != alphaSource);
				break;
			case eShaderPassTransparency:
				skipShader = (kFBAlphaSourceAccurateAlpha != alphaSource);
				break;
			case eShaderPassAdditive:
				skipShader = (kFBAlphaSourceAdditiveAlpha != alphaSource);
				break;
			}
			break;
		}
	}

	if (true == skipShader)
		return false;

	// TODO: bind projectors and instance lights list
	if (eRenderGoalSelectionId == goal)
	{
		return false;
		//InternalInstanceBegin( options.IsTextureMappingEnable(), options.GetMutedTextureId() );
	}

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
