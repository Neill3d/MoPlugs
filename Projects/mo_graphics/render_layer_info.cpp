
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: render_layer_info.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "render_layer_info.h"

#include "ProjTex_shader.h"
#include "GPUCaching_shader.h"
#include "MB_IBL_shader.h"

#define RENDER_LAYER_LABEL			"Render Layer"
#define SHADING_TYPE_LABEL			"Shading"
#define TRANSPARENCY_LABEL			"Transparency"

///////////////////////////////////////////////////////////////////////////////////////
// CRenderLayerInfo

void CRenderLayerInfo::ChangeShaderCounters(FBShader *pShader, int counter)
{
	numberOfShaders += counter;

	if (FBIS(pShader, ORShaderGPUCache) )
	{
		ORShaderGPUCache *pGPUCache = (ORShaderGPUCache*) pShader;
		CGPUCacheModel *pModel = pGPUCache->GetGPUCacheModelPtr();

		if (pModel && pModel->GetNumberOfTransparencyCommands() > 0)
		{
			transparencyShadersCounter += counter;
			alphaSourceCounter[kFBAlphaSourceAccurateAlpha] += counter;
		}

		if (true == pGPUCache->OverrideShading && eShadingTypeMatte == pGPUCache->ShadingType)
			matteShadersCounter += counter;
	}
	else if (FBIS(pShader, ProjTexShader) || FBIS(pShader, ORIBLShader) )
	{
		FBProperty *pProperty=nullptr;

		pProperty = pShader->PropertyList.Find( SHADING_TYPE_LABEL );
		if ( (nullptr != pProperty) && (eShadingTypeMatte == (EShadingType) pProperty->AsInt()) )
		{
			matteShadersCounter += counter;
		}

		pProperty = pShader->PropertyList.Find( TRANSPARENCY_LABEL );
		if ( nullptr != pProperty )
		{
			const FBAlphaSource alphaSource = (FBAlphaSource) pProperty->AsInt();

			if (kFBAlphaSourceNoAlpha != alphaSource)
			{
				transparencyShadersCounter += counter;
				alphaSourceCounter[alphaSource] += counter;
			}
		}
	}
	
	hasMatteShaders = (matteShadersCounter > 0);
	hasTransparentShaders = (transparencyShadersCounter > 0);
}

void CRenderLayerInfo::EventData(FBConnectionAction action, FBShader *pShader, FBProperty *pProp)
{
	if ( kFBCandidate == action || kFBCandidated == action)
	{
		const int actionIndex = (kFBCandidated == action) ? 1 : -1;

		if ( FBIS(pShader, ORShaderGPUCache) )
		{
			ORShaderGPUCache *pCacheShader = (ORShaderGPUCache*) pShader;

			if ( pProp == &pCacheShader->OverrideShading
				|| pProp == &pCacheShader->ShadingType)
			{
				const bool overrideShading = pCacheShader->OverrideShading;
				const EShadingType shadingType = pCacheShader->ShadingType;

				if (true == overrideShading && eShadingTypeMatte == shadingType)
					matteShadersCounter += actionIndex;
			}
		}
		else if (FBIS(pShader, ProjTexShader) || FBIS(pShader, ORIBLShader) )
		{
			//ProjTexShader *pProjShader = (ProjTexShader*) pShader;

			FBProperty *pProperty=nullptr;

			pProperty = pShader->PropertyList.Find( SHADING_TYPE_LABEL );
			if ( (nullptr != pProperty) && (pProp == pProperty)
				&& (eShadingTypeMatte == (EShadingType) pProperty->AsInt()) )
			{
				matteShadersCounter += actionIndex;
			}

			pProperty = pShader->PropertyList.Find( TRANSPARENCY_LABEL );
			if ( (nullptr != pProperty) && (pProp == pProperty) )
			{
				const FBAlphaSource alphaSource = (FBAlphaSource) pProperty->AsInt();

				if (kFBAlphaSourceNoAlpha != alphaSource)
				{
					transparencyShadersCounter += actionIndex;
					alphaSourceCounter[alphaSource] += actionIndex;
				}
			}
		}
	}

	hasMatteShaders = (matteShadersCounter > 0);
	hasTransparentShaders = (transparencyShadersCounter > 0);
}

///////////////////////////////////////////////////////////////////////////////////////
// CRenderLayerArray

void CRenderLayerArray::IncLayerShaders(FBShader *pShader)
{
	ERenderLayer layerId = eRenderLayerMain;

	if ( FBIS(pShader, ProjTexShader) || FBIS(pShader, ORIBLShader) )
	{
		FBProperty *pProperty = pShader->PropertyList.Find(RENDER_LAYER_LABEL);

		if (nullptr != pProperty)
			layerId = (ERenderLayer) pProperty->AsInt();
	}

	layersInfo[layerId].ChangeShaderCounters(pShader, 1);
}

void CRenderLayerArray::DecLayerShaders(FBShader *pShader)
{
	ERenderLayer layerId = eRenderLayerMain;

	if ( FBIS(pShader, ProjTexShader) || FBIS(pShader, ORIBLShader) )
	{
		FBProperty *pProperty = pShader->PropertyList.Find(RENDER_LAYER_LABEL);

		if (nullptr != pProperty)
			layerId = (ERenderLayer) pProperty->AsInt();
	}

	layersInfo[layerId].ChangeShaderCounters(pShader, -1);
}

void CRenderLayerArray::EventDataNotify(HISender pSender, HKEvent pEvent)
{
	FBEventConnectionDataNotify	lEvent(pEvent);
	FBPlug *lPlug = lEvent.Plug;

	if (nullptr == lPlug || false == lPlug->Is(FBProperty::TypeInfo) )
		return;

	FBPlug *lPlugOwner = lEvent.Plug->GetOwner();
	if ( !FBIS(lPlugOwner, FBShader) )
		return;

	FBShader *pShader = (FBShader*) lPlugOwner;

	//FBConnectionAction
	const FBConnectionAction action = lEvent.Action;

	if ( action == kFBCandidate || action == kFBCandidated )
	{
		const bool appendLayerData = (action == kFBCandidated);

		if ( FBIS(pShader, ProjTexShader) || FBIS(pShader, ORIBLShader) )
		{
			FBProperty *pProperty = pShader->PropertyList.Find(RENDER_LAYER_LABEL);
			//ProjTexShader *pProjShader = (ProjTexShader*) pShader;
			//const ERenderLayer layerId = pProjShader->RenderLayer;

			if (nullptr != pProperty)
			{
				const ERenderLayer layerId = (ERenderLayer) pProperty->AsInt();

				if (lPlug == pProperty)
				{	
					layersInfo[layerId].ChangeShaderCounters(pShader, (appendLayerData) ? 1 : -1);
				}
				else
				{
					layersInfo[layerId].EventData(action, pShader, (FBProperty*)lPlug);
				}
			}
		}
		
	}

}


const ERenderLayer CRenderLayerArray::GetShaderRenderLayerId(FBShader *pShader)
{
	ERenderLayer layerId = eRenderLayerMain;

	if ( FBIS(pShader, ProjTexShader) || FBIS(pShader, ORIBLShader) )
	{
		FBProperty *pProperty = pShader->PropertyList.Find(RENDER_LAYER_LABEL);
		if (nullptr != pProperty)
			layerId = (ERenderLayer) pProperty->AsInt();
	}

	return layerId;
}