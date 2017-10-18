
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: FX_shader.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Class declaration
#include "FX_shader.h"

//--- Registration defines
#define FX_COLORCORRECTION__CLASS	FX_COLORCORRECTION__CLASSNAME
#define FX_COLORCORRECTION__DESC	"FX Color Correction" // This is what shows up in the shader window ...

#define FX_SHADING__CLASS			FX_SHADING__CLASSNAME
#define FX_SHADING__DESC			"FX Shading" // This is what shows up in the shader window ...

#define FX_PROJECTION__CLASS		FX_PROJECTION__CLASSNAME
#define FX_PROJECTION__DESC			"FX Projection Mapping" // This is what shows up in the shader window ...

//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	FX_COLORCORRECTION__CLASS	);
FBRegisterShader	(FX_COLORCORRECTION__DESCSTR,	    // Unique name
                     FX_COLORCORRECTION__CLASS,		// Class
                     FX_COLORCORRECTION__DESCSTR,	    // Short description
                     FX_COLORCORRECTION__DESC,		    // Long description
                     FB_DEFAULT_SDK_ICON	);		// Icon filename (default=Open Reality icon)


FBShaderImplementation(	FX_SHADING__CLASS	);
FBRegisterShader	(FX_SHADING__DESCSTR,	    // Unique name
                     FX_SHADING__CLASS,		// Class
                     FX_SHADING__DESCSTR,	    // Short description
                     FX_SHADING__DESC,		    // Long description
                     FB_DEFAULT_SDK_ICON	);		// Icon filename (default=Open Reality icon)


FBShaderImplementation(	FX_PROJECTION__CLASS	);
FBRegisterShader	(FX_PROJECTION__DESCSTR,	    // Unique name
                     FX_PROJECTION__CLASS,		// Class
                     FX_PROJECTION__DESCSTR,	    // Short description
                     FX_PROJECTION__DESC,		    // Long description
                     FB_DEFAULT_SDK_ICON	);		// Icon filename (default=Open Reality icon)


/************************************************
*	FiLMBOX Constructor.
************************************************/
bool FXColorCorrectionShader::FBCreate()
{
	FBPropertyPublish(	this, Brightness, "Brightness" ,NULL, NULL);
	Brightness = 0.0;
	Brightness.SetMinMax(-100.0, 100.0, true, true);

	FBPropertyPublish(	this, Saturation, "Saturation" ,NULL, NULL);
	Saturation = 0.0;
	Saturation.SetMinMax(-100.0, 100.0, true, true);

	FBPropertyPublish(	this, Contrast, "Contrast" ,NULL, NULL);
	Contrast = 0.0;
	Contrast.SetMinMax(-100.0, 100.0, true, true);

	FBPropertyPublish(	this, Gamma, "Gamma" ,NULL, NULL);
	Gamma = 1.0/2.2 * 100.0;
	Gamma.SetMinMax(-300.0, 300.0, true, true);

	// custom color

	FBPropertyPublish(this, CustomColor, "Custom color" ,NULL, NULL );
	FBPropertyPublish( this, CustomColorMode, "Custom color mode", NULL, NULL );

	CustomColor = FBColor(1.0, 1.0, 1.0);
	CustomColorMode = eBlendMultiply;
	
    return true;
}


/************************************************
*	FiLMBOX Destructor.
************************************************/
void FXColorCorrectionShader::FBDestroy()
{
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool FXShadingShader::FBCreate()
{
	FBPropertyPublish(	this, ShadingType,	"Shading",NULL, NULL);
	ShadingType = eShadingTypeDynamic;

	FBPropertyPublish(	this, TransparencyFactor,	"TransparencyFactor",NULL, NULL);
	TransparencyFactor.SetMinMax(0.0, 1.0);
	TransparencyFactor = 1.0;

	FBPropertyPublish(	this, ToonSteps, "Toon Steps" ,NULL, NULL);
	ToonSteps = 4.0;

	FBPropertyPublish(	this, ToonDistribution, "Toon Distribution" ,NULL, NULL);
	ToonDistribution = 1.0;
	ToonDistribution.SetMinMax(0.1, 3.0, true, true);

	FBPropertyPublish(	this, ToonShadowPosition, "Toon Shadow Position" ,NULL, NULL);
	ToonShadowPosition = 0.5;
	ToonShadowPosition.SetMinMax(0.1, 1.0, true, true);
	
    return true;
}


/************************************************
*	FiLMBOX Destructor.
************************************************/
void FXShadingShader::FBDestroy()
{

}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

static void FXProjection_ManualUpdate(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		
	}
}

static void FXProjection_ProjectorSelect0(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoProjectorSelect(0);
	}
}

static void FXProjection_ProjectorSelect1(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoProjectorSelect(1);
	}
}

static void FXProjection_ProjectorSelect2(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoProjectorSelect(2);
	}
}

static void FXProjection_ProjectorSelect3(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoProjectorSelect(3);
	}
}

static void FXProjection_ProjectorSelect4(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoProjectorSelect(4);
	}
}

static void FXProjection_ProjectorSelect5(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoProjectorSelect(5);
	}
}

static void FXProjection_TextureSelect0(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoTextureSelect(0);
	}
}

static void FXProjection_TextureSelect1(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoTextureSelect(1);
	}
}

static void FXProjection_TextureSelect2(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoTextureSelect(2);
	}
}

static void FXProjection_TextureSelect3(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoTextureSelect(3);
	}
}

static void FXProjection_TextureSelect4(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoTextureSelect(4);
	}
}

static void FXProjection_TextureSelect5(HIObject object, bool pValue)
{
	FXProjectionMapping *shader = FBCast<FXProjectionMapping>(object);
	if (shader && pValue) 
	{
		shader->DoTextureSelect(5);
	}
}

#define FBPropertyPublishInternal( Parent,Prop,PropName,Get,Set ) \
	{ \
	Parent->PropertyAdd(Prop.CreateInternal( Parent,PropName,(fbExternalGetSetHandler)Get,(fbExternalGetSetHandler)Set )); \
	}

bool FXProjectionMappingContainer::InitializeProperties(FBComponent *pParent)
{
	
	mSuccess = true;

	char buffer[128];
	for (int i=0; i<MAX_PROJECTORS_COUNT; ++i)
	{
		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector%d", i+1 );
		FBPropertyPublishInternal(pParent, Projectors[i].Projector,	buffer,	NULL, NULL);
		Projectors[i].Projector.SetFilter(FBCamera::GetInternalClassId() );
		Projectors[i].Projector.SetSingleConnect(true);

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Select%d", i+1 );
		switch(i)
		{
		case 0: FBPropertyPublishInternal(pParent, Projectors[i].ProjectorSelect,	buffer,	NULL, FXProjection_ProjectorSelect0);
			break;
		case 1: FBPropertyPublishInternal(pParent, Projectors[i].ProjectorSelect,	buffer,	NULL, FXProjection_ProjectorSelect1);
			break;
		case 2: FBPropertyPublishInternal(pParent, Projectors[i].ProjectorSelect,	buffer,	NULL, FXProjection_ProjectorSelect2);
			break;
		case 3: FBPropertyPublishInternal(pParent, Projectors[i].ProjectorSelect,	buffer,	NULL, FXProjection_ProjectorSelect3);
			break;
		case 4: FBPropertyPublishInternal(pParent, Projectors[i].ProjectorSelect,	buffer,	NULL, FXProjection_ProjectorSelect4);
			break;
		case 5: FBPropertyPublishInternal(pParent, Projectors[i].ProjectorSelect,	buffer,	NULL, FXProjection_ProjectorSelect5);
			break;
		}
		
		
		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Texture%d", i+1 );
		FBPropertyPublishInternal(pParent, Projectors[i].Texture,	buffer,	NULL, NULL);
		Projectors[i].Texture.SetFilter(FBTexture::GetInternalClassId() );
		Projectors[i].Texture.SetSingleConnect(true);

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Texture Select%d", i+1 );
		switch(i)
		{
		case 0: FBPropertyPublishInternal(pParent, Projectors[i].TextureSelect,	buffer,	NULL, FXProjection_TextureSelect0 );
			break;
		case 1: FBPropertyPublishInternal(pParent, Projectors[i].TextureSelect,	buffer,	NULL, FXProjection_TextureSelect1 );
			break;
		case 2: FBPropertyPublishInternal(pParent, Projectors[i].TextureSelect,	buffer,	NULL, FXProjection_TextureSelect2 );
			break;
		case 3: FBPropertyPublishInternal(pParent, Projectors[i].TextureSelect,	buffer,	NULL, FXProjection_TextureSelect3 );
			break;
		case 4: FBPropertyPublishInternal(pParent, Projectors[i].TextureSelect,	buffer,	NULL, FXProjection_TextureSelect4 );
			break;
		case 5: FBPropertyPublishInternal(pParent, Projectors[i].TextureSelect,	buffer,	NULL, FXProjection_TextureSelect5 );
			break;
		}

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Usage%d", i+1 );
		FBPropertyPublishInternal( pParent, Projectors[i].ProjectorUse, buffer, NULL, NULL );
		Projectors[i].ProjectorUse = true;

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Keep Aspect%d", i+1 );
		FBPropertyPublishInternal( pParent, Projectors[i].ProjectorAspect, buffer, NULL, NULL );
		Projectors[i].ProjectorAspect = true;

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Blend Mode%d", i+1 );
		FBPropertyPublishInternal( pParent, Projectors[i].BlendMode, buffer, NULL, NULL );
		Projectors[i].BlendMode = eBlendNormal;

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Blend Opacity%d", i+1 );
		FBPropertyPublishInternal( pParent, Projectors[i].BlendOpacity, buffer, NULL, NULL );
		Projectors[i].BlendOpacity = 100.0;
		Projectors[i].BlendOpacity.SetMinMax(0.0, 100.0, true, true);

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Mask%d", i+1 );
		FBPropertyPublishInternal( pParent, Projectors[i].Mask, buffer, NULL, NULL );
		Projectors[i].Mask = ProjectorMask0;

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Mask Channel%d", i+1 );
		FBPropertyPublishInternal( pParent, Projectors[i].MaskChannel, buffer, NULL, NULL );
		Projectors[i].MaskChannel = ProjectorMaskChannelR;
	}

	Projectors[0].MaskChannel = ProjectorMaskChannelR;
	Projectors[1].MaskChannel = ProjectorMaskChannelG;
	Projectors[2].MaskChannel = ProjectorMaskChannelB;
	Projectors[3].MaskChannel = ProjectorMaskChannelA;
	Projectors[4].MaskChannel = ProjectorMaskChannelR;
	Projectors[5].MaskChannel = ProjectorMaskChannelG;

	FBPropertyPublishInternal( pParent, Mask1, "Mask1", nullptr, nullptr );
	Mask1.SetFilter( FBTexture::GetInternalClassId() );
	Mask1.SetSingleConnect(true);

	FBPropertyPublishInternal( pParent, Mask2, "Mask2", nullptr, nullptr );
	Mask2.SetFilter( FBTexture::GetInternalClassId() );
	Mask2.SetSingleConnect(true);

	FBPropertyPublishInternal( pParent, DynamicUpdate, "Dynamic Update", nullptr, nullptr );
	DynamicUpdate = true;

	FBPropertyPublishInternal( pParent, ProjectorsUpdate, "Manual Update", nullptr, FXProjection_ManualUpdate );

	return true;
}

void FXProjectionMappingContainer::FreeProperties()
{
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool FXProjectionMapping::FBCreate()
{
	InitializeProperties(this);
	mLastUniqueFrameId = 0;
	mNeedToUpdateProjectors = true;
	//
	SetShaderCapacity(FBShaderCapacity(kFBShaderCapacityMaterialEffect), false);

    return true;
}


/************************************************
*	FiLMBOX Destructor.
************************************************/
void FXProjectionMapping::FBDestroy()
{
	FreeProperties();
}


/////////////////


bool FXProjectionMapping::ShaderNeedBeginRender()
{
	return true;
}


void FXProjectionMapping::ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	if (DynamicUpdate)
	{
		
	}
}


////////////////


void FXProjectionMappingContainer::DoProjectorSelect(const int index)
{
	if (Projectors[index].Projector.GetCount() > 0)
		Projectors[index].Projector.GetAt(0)->Selected = true;
}

void FXProjectionMappingContainer::DoTextureSelect(const int index)
{
	if (Projectors[index].Projector.GetCount() > 0)
	{
		FBCamera *pCamera = (FBCamera*) Projectors[index].Projector.GetAt(0);
		FBVideo *pVideo = pCamera->ForeGroundMedia;
		if (pVideo)
			pVideo->Selected = true;
	}

	if (Projectors[index].Texture.GetCount() > 0)
		Projectors[index].Texture.GetAt(0)->Selected = true;
}