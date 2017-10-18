
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_fbshader.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Class declaration
#include "compositeMaster_fbshader.h"

//--- Registration defines
#define COMPOSITESCREENSHADER__CLASS	COMPOSITESCREENSHADER__CLASSNAME
#define COMPOSITESCREENSHADER__DESC	"Composite Screen Shader" // This is what shows up in the shader window ...



//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	COMPOSITESCREENSHADER__CLASS	);
FBRegisterShader	(COMPOSITESCREENSHADER__DESCSTR,	    // Unique name
                     COMPOSITESCREENSHADER__CLASS,		// Class
                     COMPOSITESCREENSHADER__DESCSTR,	    // Short description
                     COMPOSITESCREENSHADER__DESC,		    // Long description
                     FB_DEFAULT_SDK_ICON	);		// Icon filename (default=Open Reality icon)




void AddPropertyViewForScreenShader(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(COMPOSITESCREENSHADER__CLASSSTR, pPropertyName, pHierarchy);
}
 
void CompositeScreenShader::AddPropertiesToPropertyViewManager()
{
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool CompositeScreenShader::FBCreate()
{
    //Set up shader capacity.  It seems cg2.0 has problem regarding INSTNCEID currently.
    //SetShaderCapacity(FBShaderCapacity(kFBShaderCapacityMaterialEffect | kFBShaderCapacityDrawInstanced), true);
	SetShaderCapacity(FBShaderCapacity(kFBShaderCapactiyDrawTextureLayer), true);

    //Hook up the callback 

	//kFBPassPreRender
	RenderingPass = FBRenderingPass(kFBPassPreRender | kFBPassPostRender);
    //SetShaderPassActionCallback(FBShaderPassActionCallback(kFBShaderPassTypeBegin | kFBShaderPassTypeEnd | kFBShaderPassInstanceBegin | kFBShaderPassInstanceEnd | kFBShaderPassMaterialBegin | kFBShaderPassMaterialEnd | kFBShaderPassModelDraw));

    return true;
}


/************************************************
*	FiLMBOX Destructor.
************************************************/
void CompositeScreenShader::FBDestroy()
{
    ParentClass::FBDestroy();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////

void CompositeScreenShader::ShaderPassTypeBegin    ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{

}

void CompositeScreenShader::ShaderPassTypeEnd      ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)     
{
	
}


void CompositeScreenShader::ShaderPassInstanceBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)  
{
	
}

void CompositeScreenShader::ShaderPassInstanceEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)  
{
	
}

void CompositeScreenShader::ShaderPassMaterialBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)  
{
	
}

void CompositeScreenShader::ShaderPassMaterialEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)  
{
	
}

void CompositeScreenShader::ShaderPassModelDraw ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)
{
	
}

void CompositeScreenShader::ShaderPassDrawShadowBegin( FBRenderOptions* pRenderOptions)
{
    //Here we should setup the shader's draw shadow states
}

void CompositeScreenShader::ShaderPassDrawShadowEnd( FBRenderOptions* pRenderOptions)
{
    //Here we should clean the shader's draw shadow states
}

void CompositeScreenShader::UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount)
{
	

}


bool CompositeScreenShader::ShaderNeedBeginRender()
{
	return false;
}

void CompositeScreenShader::ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	printf ("begin\n");
}

void CompositeScreenShader::ShadeModel( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo, FBRenderingPass pPass )
{
	printf ("shade model\n");
}

FBShaderModelInfo *CompositeScreenShader::NewShaderModelInfo(HKModelRenderInfo pModelRenderInfo, int pSubRegionIndex)
{
    FBShaderModelInfo *lShaderModelInfo = ParentClass::NewShaderModelInfo(pModelRenderInfo, pSubRegionIndex);
	//FBShaderModelInfo *lShaderModelInfo = new SuperShaderModelInfo(this, pModelRenderInfo, pSubRegionIndex);
    return lShaderModelInfo;
}

void CompositeScreenShader::UpdateModelShaderInfo( FBRenderOptions* pOptions, FBShaderModelInfo *pModelRenderInfo )
{
    unsigned int lVBOFormat = kFBGeometryArrayID_Point | kFBGeometryArrayID_Normal;
	pModelRenderInfo->UpdateModelShaderInfo(GetShaderVersion());
}

void CompositeScreenShader::SetTransparencyType( FBAlphaSource pTransparency )
{
	/*
    if (Transparency != pTransparency)
    {
        Transparency = pTransparency;
        //To trigger render to update the model-shader information.
        InvalidateShaderVersion();
    }
	*/
}
/*
FBAlphaSource ProjTexShader::GetTransparencyType()
{
    return Transparency;
}

void ProjTexShader::SetTransparencyProperty( HIObject pObject, FBAlphaSource pState )
{     
    ProjTexShader* lShader = FBCast<ProjTexShader>(pObject);
    if (lShader->Transparency != pState)
    {
        lShader->Transparency.SetPropertyValue(pState);
        lShader->RenderingPass = GetRenderingPassNeededForAlpha(pState);
         
        // if shader use alpha and thus generate custom shape than the original geometry shape, 
        // we need to let it handle DrawShadow functiionality as well. 
        lShader->SetShaderCapacity(kFBShaderCapacityDrawShadow, pState != kFBAlphaSourceNoAlpha); 
    }
}
*/
void CompositeScreenShader::DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	
}