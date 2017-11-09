
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: FX_shader.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "..\Common\Common_shader.h"
#include "ShaderFX.h"
#include "..\Common_Projectors\bakeProjectors_projectors.h"

#ifndef MAX_PROJECTORS_COUNT
#define MAX_PROJECTORS_COUNT 6
#endif

//--- Registration define

#define FX_COLORCORRECTION__CLASSSTR	"FXColorCorrectionShader"
#define FX_COLORCORRECTION__DESCSTR		"FX Color Correction Plugin"

#define FX_COLORCORRECTION__CLASSNAME	FXColorCorrectionShader
//
#define FX_SHADING__CLASSSTR			"FXShadingShader"
#define FX_SHADING__DESCSTR				"FX Shading Plugin"

#define FX_SHADING__CLASSNAME			FXShadingShader
//
#define FX_PROJECTION__CLASSSTR			"FBProjectionMapping"
#define FX_PROJECTION__DESCSTR			"FX Projection Mapping Plugin"

#define FX_PROJECTION__CLASSNAME		FXProjectionMapping

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Dynamic lighting shader using CG to support pixel lighting and normal mapping.
class FXColorCorrectionShader : public FBShader
{
    // Declaration.
    FBShaderDeclare( FXColorCorrectionShader, FBShader );

public:
	
    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate();
    virtual void FBDestroy();

public:
    
	FBPropertyAnimatableDouble			Brightness;
	FBPropertyAnimatableDouble			Saturation;
	FBPropertyAnimatableDouble			Contrast;
	FBPropertyAnimatableDouble			Gamma;

	// custom color
	FBPropertyAnimatableColor			CustomColor;
	FBPropertyBaseEnum<EBlendType>		CustomColorMode;		//!< type of blending two images
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Dynamic lighting shader using CG to support pixel lighting and normal mapping.
class FXShadingShader : public FBShader
{
    // Declaration.
    FBShaderDeclare( FXShadingShader, FBShader );

public:
	
    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate();
    virtual void FBDestroy();

public:
    
	FBPropertyBaseEnum<EShadingType>	ShadingType;	//!> flat, phong, toon
	
	FBPropertyAnimatableDouble			TransparencyFactor;	// override the global shader transparency factor

	FBPropertyAnimatableDouble			ToonSteps;
	FBPropertyAnimatableDouble			ToonDistribution;
	FBPropertyAnimatableDouble			ToonShadowPosition;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Dynamic lighting shader using CG to support pixel lighting and normal mapping.

class FXProjectionMappingContainer
{
public:
    
	// projector properties
	struct ProjectorObject
	{
		FBPropertyListObject	Projector;
		FBPropertyAction		ProjectorSelect;		//!< select assigned camera
		FBPropertyListObject	Texture;				//!< texture can be assigned from here or taken from camera foreground
		FBPropertyAction		TextureSelect;
		FBPropertyBool			ProjectorUse;
		FBPropertyBool			ProjectorAspect;

		FBPropertyBaseEnum<ProjectorMaskEnum>			Mask;
		FBPropertyBaseEnum<ProjectorMaskChannelEnum>	MaskChannel;

		FBPropertyAnimatableDouble						BlendOpacity;
		FBPropertyBaseEnum<EBlendType>	BlendMode;		//!< type of blending two images

	}	Projectors[MAX_PROJECTORS_COUNT];

	FBPropertyListObject		Mask1;
	FBPropertyListObject		Mask2;

	FBPropertyBool				DynamicUpdate;
	FBPropertyAction			ProjectorsUpdate;

public:

	bool	InitializeProperties(FBComponent *pParent);
	void	FreeProperties();

	void	DoProjectorSelect(const int index);
	void	DoTextureSelect(const int index);

	//const int GetNumberOfProjectors() const;

	bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
	{
		if (pAction == kFBCandidated)
		{
			for (int i=0; i<MAX_PROJECTORS_COUNT; ++i)
			{
				ProjectorObject &obj = Projectors[i];

				if (pThis == &obj.Projector
					|| pThis == &obj.Texture
					|| pThis == &obj.ProjectorUse
					|| pThis == &obj.ProjectorAspect
					|| pThis == &obj.Mask
					|| pThis == &obj.MaskChannel
					|| pThis == &obj.BlendOpacity
					|| pThis == &obj.BlendMode)
				{
					return true;
				}
			}

			if (pThis == &Mask1 || pThis == &Mask2 || pThis == &DynamicUpdate)
				return true;
		}
		return false;
	}

protected:

	bool								mSuccess;

};


//////////////////////////////////////////////////////////////////////////////
//
class FXProjectionMapping : public FBShader, public FXProjectionMappingContainer
{
    // Declaration.
    FBShaderDeclare( FXProjectionMapping, FBShader );

public:
	
    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate();
    virtual void FBDestroy();

	/** Does the shader need a begin render call.
	*	\remark	Re-implement this function and return true if you need it. This method is called once per shader on each render pass.
	*/
	virtual bool ShaderNeedBeginRender();

	/** Pre-rendering of shaders that is called only one time for a shader instance.
	*	\remark	ShaderBeginRender is called as many times as the number of models affected 
	*			by the instance of the shader in a given render pass. (ShaderNeedBeginRender 
	*			must return true for this function to be called).
	*/
	virtual void ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo );

	bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize) override
	{
		if (true == FXProjectionMappingContainer::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize ) )
		{
			mNeedToUpdateProjectors = true;
		}

		return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
	}

	bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override
	{
		if (pThis == &Mask1 || pThis == &Mask2)
		{
			if (pAction == kFBConnectedSrc)
			{
				ConnectSrc(pPlug);
				mNeedToUpdateProjectors = true;
			}
			else if (pAction == kFBDisconnectedSrc)
			{
				DisconnectSrc(pPlug);
				mNeedToUpdateProjectors = true;
			}
		}
		for (int i=0; i<MAX_PROJECTORS_COUNT; ++i)
		{
			if (pThis == &Projectors[i].Projector || pThis == &Projectors[i].Texture)
			{
				if (pAction == kFBConnectedSrc)
				{
					ConnectSrc(pPlug);
					mNeedToUpdateProjectors = true;
				}
				else if (pAction == kFBDisconnectedSrc)
				{
					DisconnectSrc(pPlug);
					mNeedToUpdateProjectors = true;
				}
			}
		}

		return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
	}

public:
	bool		mNeedToUpdateProjectors;

	unsigned int		mLastUniqueFrameId;
	CProjectors			mProjectors;

	const CProjectors	*GetProjectorsPtr() const {
		return &mProjectors;
	}

	void AskToUpdateAll()
	{
		mNeedToUpdateProjectors = true;	
	}

	void EventBeforeRenderNotify() 
	{
		if (mNeedToUpdateProjectors)
		{
			mProjectors.PrepFull(this);
			mNeedToUpdateProjectors = false;
		}
		else
		{
			mProjectors.PrepLight();
		}
	}
};
