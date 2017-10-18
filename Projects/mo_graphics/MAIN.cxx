
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: main.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include

#include <fbsdk/fbsdk.h>
#include <GL\glew.h>

#include "resourceUtils.h"
#include "ProjTex_shader.h"
#include "model_cubemap_rendering.h"

#include "compositeMaster_objectFinal.h"
#include "compositeMaster_objectLUTFilter.h"

#include "compositeMaster_objectFogFilter.h"
#include "compositeMaster_objectDecalFilter.h"
#include "compositeMaster_objectDOFFilter.h"


#include "compositeMaster_objectFilters.h"
#include "compositeMaster_objectLayers.h"

//#include "MB_renderer.h"

//--- Library declaration
FBLibraryDeclare( mographics )
{
	// Composite Master
	
	FBLibraryRegister( CompositeAssociation );

	//FBLibraryRegister( ToolCompositeMaster );
	
	FBLibraryRegister(ObjectComposition);
	FBLibraryRegisterElement(ObjectComposition);
	
	FBLibraryRegister(ObjectCompositionRender);
	FBLibraryRegisterElement(ObjectCompositionRender);
	
	FBLibraryRegister(ObjectCompositionColor);
	FBLibraryRegisterElement(ObjectCompositionColor);

	FBLibraryRegister(ObjectCompositionShadow);
	FBLibraryRegisterElement(ObjectCompositionShadow);

	FBLibraryRegister(ObjectFilterColorCorrection);
	FBLibraryRegisterElement(ObjectFilterColorCorrection);

	FBLibraryRegister(ObjectFilterLUT);
	FBLibraryRegisterElement(ObjectFilterLUT);

	FBLibraryRegister(ObjectFilterBlur);
	FBLibraryRegisterElement(ObjectFilterBlur);

	FBLibraryRegister(ObjectFilterHalfTone);
	FBLibraryRegisterElement(ObjectFilterHalfTone);

	FBLibraryRegister(ObjectFilterPosterization);
	FBLibraryRegisterElement(ObjectFilterPosterization);

	FBLibraryRegister(ObjectFilterChangeColor);
	FBLibraryRegisterElement(ObjectFilterChangeColor);

	FBLibraryRegister(ObjectFilterFilmGrain);
	FBLibraryRegisterElement(ObjectFilterFilmGrain);

	FBLibraryRegister(ObjectFilterCrosshatch);
	FBLibraryRegisterElement(ObjectFilterCrosshatch);

	FBLibraryRegister(ObjectFilterCrossStitching);
	FBLibraryRegisterElement(ObjectFilterCrossStitching);

	FBLibraryRegister(ObjectFilterToonLines);
	FBLibraryRegisterElement(ObjectFilterToonLines);

	FBLibraryRegister(ObjectFilter3dFog);
	FBLibraryRegisterElement(ObjectFilter3dFog);

	FBLibraryRegisterStorable(ModelFogVolume);
	FBLibraryRegisterElement(ModelFogVolume);

	FBLibraryRegisterStorable(ModelFogTarget);
	FBLibraryRegisterElement(ModelFogTarget);

	FBLibraryRegister(ObjectFilter3dDecal);
	FBLibraryRegisterElement(ObjectFilter3dDecal);

	FBLibraryRegisterStorable(ModelDecal);
	FBLibraryRegisterElement(ModelDecal);

	FBLibraryRegister(ObjectFilter3dDOF);
	FBLibraryRegisterElement(ObjectFilter3dDOF);

	FBLibraryRegister(ObjectFilterSSAO);
	FBLibraryRegisterElement(ObjectFilterSSAO);

	FBLibraryRegisterStorable(ModelShadowZone);
	FBLibraryRegisterElement(ModelShadowZone);
	/*
	FBLibraryRegister(ObjectFilter3dShadow);
	FBLibraryRegisterElement(ObjectFilter3dShadow);
	*/
	/*
	
	

	FBLibraryRegister(FilterFXAA);
	FBLibraryRegisterElement(FilterFXAA);

	*/
	// Super Lighting shader

	FBLibraryRegister( ORShaderGPUCache );
    FBLibraryRegister( ORShaderGPUCachingLayout );

	FBLibraryRegisterStorable(ORModelGPUCache);
	FBLibraryRegisterElement(ORModelGPUCache);

	FBLibraryRegister( ProjTexShader );
    FBLibraryRegister( ProjTexLayout );

	FBLibraryRegister( ORIBLShader );
	FBLibraryRegister( ORCharacterEyeShader );
	FBLibraryRegister( ORCharacterSkinShader );

	FBLibraryRegister(ObjectBlendSolver);
	FBLibraryRegisterElement(ObjectBlendSolver);

	FBLibraryRegister(ObjectCubeMap);
	FBLibraryRegisterElement(ObjectCubeMap);

	//FBLibraryRegister( CompositeScreenShader );

	FBLibraryRegister( FXColorCorrectionShader );
	FBLibraryRegister( FXShadingShader );
	FBLibraryRegister( FXProjectionMapping );

	//FBLibraryRegister( ORToolViewTextures );

	// Dynamic masks

	FBLibraryRegister( ToolDynamicMask );

	FBLibraryRegister(ObjectMask);
	FBLibraryRegisterElement(ObjectMask);
	FBLibraryRegister(ObjectChannel);
	FBLibraryRegisterElement(ObjectChannel);
	FBLibraryRegister(ObjectShape);
	FBLibraryRegisterElement(ObjectShape);
	FBLibraryRegister(ObjectKnot);
	FBLibraryRegisterElement(ObjectKnot);

	// extended rendering object

	FBLibraryRegister(CameraRendering);
	FBLibraryRegisterElement(CameraRendering);

	FBLibraryRegisterStorable(CubeMapRendering);
	FBLibraryRegisterElement(CubeMapRendering);


	// global manager

	//FBLibraryRegister( GraphicsManager );
	//FBLibraryRegister(GraphicsGlobalSettings);
	//FBLibraryRegisterElement(GraphicsGlobalSettings);
	FBLibraryRegister( MoGraphicsAssociation );

	// morenderer

	//FBLibraryRegister( MoRendererCallback );
    //FBLibraryRegister( MoRendererCallbackLayout );

	FBLibraryRegister( MoRendererCallbackLayout );
	FBLibraryRegister( MoRendererCallback );

}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ 

	return true; 
}
bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()	{ 
	
	
	InitResourceUtils();
	
	ProjTexShader::AddPropertiesToPropertyViewManager();
	CameraRendering::AddPropertiesToPropertyViewManager();
	CubeMapRendering::AddPropertiesToPropertyViewManager();
	

	/*
	ObjectComposition::AddPropertiesToPropertyViewManager();
	ObjectCompositionColor::AddPropertiesToPropertyViewManager();
	ObjectCompositionRender::AddPropertiesToPropertyViewManager();
	ObjectFilterColorCorrection::AddPropertiesToPropertyViewManager();
	ObjectFilterLUT::AddPropertiesToPropertyViewManager();
	ObjectFilterBlur::AddPropertiesToPropertyViewManager();
	ObjectFilterHalfTone::AddPropertiesToPropertyViewManager();
	ObjectFilterPosterization::AddPropertiesToPropertyViewManager();
	ObjectFilterChangeColor::AddPropertiesToPropertyViewManager();
	ObjectFilterFilmGrain::AddPropertiesToPropertyViewManager();
	ObjectFilterCrosshatch::AddPropertiesToPropertyViewManager();
	ObjectFilterCrossStitching::AddPropertiesToPropertyViewManager();
	ObjectFilterToonLines::AddPropertiesToPropertyViewManager();
	
	ObjectFilter3dFog::AddPropertiesToPropertyViewManager();
	ModelFogVolume::AddPropertiesToPropertyViewManager();
	ModelFogTarget::AddPropertiesToPropertyViewManager();

	ObjectFilter3dDecal::AddPropertiesToPropertyViewManager();
	ModelDecal::AddPropertiesToPropertyViewManager();

	ObjectFilter3dDOF::AddPropertiesToPropertyViewManager();
	*/
	//MoRendererCallback::AddPropertiesToPropertyViewManager();
	
	return true; 
}
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease(){ return true; }
