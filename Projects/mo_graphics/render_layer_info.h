
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: render_layer_info.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

////////////////////////////////////////////////////////////////////////////
//

enum ERenderLayer
{
	eRenderLayerBackground,
	eRenderLayerMain,
	eRenderLayerSecondary,
	eRenderLayerCount		
};

////////////////////////////////////////////////
//
struct CRenderLayerInfo
{
	ERenderLayer	layerId;

	int			numberOfShaders;		// number of shaders have been assigned to the layer

	bool		hasMatteShaders;
	bool		hasTransparentShaders;

	int			matteShadersCounter;
	int			transparencyShadersCounter;

	int			alphaSourceCounter[kFBAlphaSourceTransluscentZSortAlpha + 1];


	//! a constructor
	CRenderLayerInfo()
	{
		Clear();
	}

	void Clear()
	{
		numberOfShaders = 0;
		hasMatteShaders = false;
		hasTransparentShaders = false;

		matteShadersCounter = 0;
		transparencyShadersCounter = 0;

		for (int i=0; i<=kFBAlphaSourceTransluscentZSortAlpha; ++i)
			alphaSourceCounter[i] = 0;
	}

	void ChangeShaderCounters(FBShader *pShader, int counter);

	// DONE: change shader properties

	void EventData(FBConnectionAction action, FBShader *pShader, FBProperty *pProp);
};


///////////////////////////////////////////////////////////////////////
//
struct CRenderLayerArray
{

	CRenderLayerInfo		layersInfo[eRenderLayerCount];

	//! a constructor
	CRenderLayerArray()
	{
		Clear();
	}

	void Clear()
	{
		for (int i=0; i<eRenderLayerCount; ++i)
			layersInfo[i].Clear();
	}

	void IncLayerShaders(FBShader *pShader);
	void DecLayerShaders(FBShader *pShader);

	static const ERenderLayer GetShaderRenderLayerId(FBShader *pShader);

	// DONE: move shader from one layer into another
	void EventDataNotify(HISender pSender, HKEvent pEvent);

	// query information

	const int GetNumberOfShaders(ERenderLayer layerId) const {
		return layersInfo[layerId].numberOfShaders;
	}

	const bool HasMatteShaders(ERenderLayer layerId) const {
		return layersInfo[layerId].hasMatteShaders;
	}
	const bool HasTransparencyShaders(ERenderLayer layerId) const {
		return layersInfo[layerId].hasTransparentShaders;
	}
	const int GetAlphaSourceCount(ERenderLayer layerId, FBAlphaSource source) const {
		return layersInfo[layerId].alphaSourceCounter[source];
	}
};