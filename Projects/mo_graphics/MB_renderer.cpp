
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_renderer.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ORSDK_DLL
/** \def ORSDK_DLL
*	Be sure that ORSDK_DLL is defined only once...
*/
#define ORSDK_DLL K_DLLEXPORT
#endif

// Class declaration
#include "mb_renderer.h"
#include <fbsdk/fbsystem.h>
#include <fbsdk/fbscene.h>
#include <fbsdk/fbrenderer.h>
#include <fbsdk/fbshader.h>
#include <fbsdk/fbsystem.h>

#include "compositeMaster_objectFinal.h"
#include "compositeMaster_objectDecalFilter.h"

#include "dynamicmask_object.h"

#include "mographics_common.h"

//--- Registration defines
#define MORendererCallback__CLASS		MORendererCallback__CLASSNAME
#define MORendererCallback__DESC		MORendererCallback__DESCSTR

FBRendererCallbackImplementation(MORendererCallback__CLASS);

FBRegisterRendererCallback	(
    MORendererCallback__CLASSSTR,
    MORendererCallback__CLASS,
    MORendererCallback__CLASSSTR,
    MORendererCallback__DESC,
    FB_DEFAULT_SDK_ICON	);

//

extern HGLRC gLastContext;
extern bool gCallbackAttached;

/*
source: DEBUG_SOURCE_X where X may be API, 
SHADER_COMPILER, WINDOW_SYSTEM, 
THIRD_PARTY, APPLICATION, OTHER
type: DEBUG_TYPE_X where X may be ERROR, 
MARKER, OTHER, DEPRECATED_BEHAVIOR, 
UNDEFINED_BEHAVIOR, PERFORMANCE, 
PORTABILITY, {PUSH, POP}_GROUP
severity: DEBUG_SEVERITY_{HIGH, MEDIUM}, 
DEBUG_SEVERITY_{LOW, NOTIFICATION}
*/
void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		printf( ">> ERROR!\n" );
		printf( "debug message - %s\n", message );
	}
	//printf( "debug message - %s\n", message );
}

//

const bool MoRendererCallback::IsLogarithmicDepth() const
{
	return (eGraphicsDepthLog == mCurrentProperties->mQuality.DisplayDepth->AsInt());
}

// define new task cycle index
FBProfiler_CreateTaskCycle( VRRenderer, 0.5f, 0.5f, 0.5f );

////////////////////////////////////////

const char * FBPropertyBaseEnum<EGraphicsResolution>::mStrings[] = {
	"Custom",
	"FullHD",
	"2K (Aspect 2.35)",
	"4K",
	0};

const char * FBPropertyBaseEnum<EGraphicsQualityPresets>::mStrings[] = {
	"Simple",
	"Custom",
	"MSAA 2x",
	"MSAA 4x",
	"MSAA 16x",
	"CSAA 4x",
	"CSAA 16x",
	"SuperSampling 2x2",
	"SuperSampling 4x4",
	"SuperSampling 6x6",
	"SuperSampling 8x8",
	0};

const char * FBPropertyBaseEnum<EResamplingFilter>::mStrings[] = {"Linear", "Kernel", "Lanczos", 0};
/*
const char * FBPropertyBaseEnum<EGraphicsDownsampleType>::mStrings[] = {
	"Simple",
	"Kernel filtering",
	0};
	*/
const char * FBPropertyBaseEnum<EGraphicsDepth>::mStrings[] = {
	"Linear",
	"Log",
	"NV",
	0};

const char * FBPropertyBaseEnum<ERendererUpdate>::mStrings[] = {
	"Manual",
	"Partly",
	"Real-Time",
	0};

const char * FBPropertyBaseEnum<ERendererEnum>::mStrings[] = {
		"Default",
		0
	};

/////////////////////////////////////////
/*
static void ORCacheSceneGraphSet(HIObject pMbObject, bool pValue)
{
    //MoRendererCallback* pFbObject = FBCast<MoRendererCallback>( pMbObject );
    //pFbObject->CacheSceneGraphSet(pValue);
}
*/
static void VRRenderer_CubeMapSizeSet(HIObject pMbObject, int pValue)
{
    MoRendererCallback* pFbObject = FBCast<MoRendererCallback>( pMbObject );
	if (pFbObject)
	{
		pFbObject->CubeMapSizeSet(pValue);
		pFbObject->CubeMapSize.SetPropertyValue(pValue);
	}
}

void MoRendererCallback::CubeMapSizeSet(int value)
{
	PanoWidth = value * 4;
	PanoHeight = value * 2;
	/*
	mCubeMapData.cubeMapSize = value;
	mCubeMapData.outputWidth = PanoWidth;
	mCubeMapData.outputHeight = PanoHeight;
	*/
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//

void MoRendererCallback::AddPropertyView(const char* pPropertyName, const char* pHierarchy, bool pIsFolder)
{
	FBPropertyViewManager::TheOne().AddPropertyView(MORendererCallback__CLASSSTR, pPropertyName, pHierarchy);
}

FBPropertyViewDefinition *AddPropertyToViewList(FBComponent *pOwner, FBProperty *pProperty, FBPropertyViewList *pViewList, 
	const char *szFolder, const char *pHierarchy, bool pSetOpen=false)
{
    //FBProperty *pProperty = pOwner->PropertyList.Find(pPropertyName);
    FBPropertyViewDefinition *pView = pViewList->AddPropertyView(pProperty, FBString(szFolder, pHierarchy));
    
    if (true == pSetOpen)
        pView->SetOpen( pSetOpen, true );
    
    return pView;
}

void AddProperties(MoRendererCallback *pCallback, MoRendererCallback::CBaseProperties *props, const char *szFolder, FBPropertyViewList *pViewList )
{
	for (int i=0; i<props->GetNumberOfGroups(); ++i)
	{
		const char *groupName = props->GroupName(i);
		FBProperty **groupProps = props->GetPropertiesPtr(i);
		//FBStringList *groupNames = props->GetPropertiesNamesPtr(i);

		for (int j=0; j<props->GetNumberOfItems(i); ++j)
		{
			AddPropertyToViewList(pCallback, groupProps[j], pViewList, szFolder, groupName );
		}
	}
}

void MoRendererCallback::AddPropertiesToPropertyViewManager(FBComponent *pComponent)
{
	const FBPropertyViewType viewType = kFBViewByObjectType;

	FBPropertyViewManager		&lMgr = FBPropertyViewManager::TheOne();
	auto pViewList = lMgr.FindPropertyList(pComponent, viewType, MORendererCallback__CLASSSTR);
    
	if (nullptr != pViewList)
        lMgr.RemovePropertyList(pComponent, viewType, MORendererCallback__CLASSSTR);

    pViewList = lMgr.CreatePropertyList(pComponent, viewType, MORendererCallback__CLASSSTR);
    
	MoRendererCallback *pCallback = (MoRendererCallback*) pComponent;

	AddProperties(pCallback, &pCallback->mDisplayProperties, "Viewport Properties.", pViewList);
	AddPropertyToViewList(pCallback, &pCallback->RenderQualityOverride, pViewList, "", "" );
	AddPropertyToViewList(pCallback, &pCallback->RenderShadingOverride, pViewList, "", "" );
	AddProperties(pCallback, &pCallback->mRenderProperties, "Render Properties.", pViewList);
	AddProperties(pCallback, &pCallback->mCompositionOptions, "", pViewList);
	AddProperties(pCallback, &pCallback->mRenderStats, "", pViewList);
	AddProperties(pCallback, &pCallback->mGlobalResolution, "", pViewList);

	AddPropertyToViewList(pCallback, &pCallback->Reset, pViewList, "", "" );
	AddPropertyToViewList(pCallback, &pCallback->NoFrustumculling, pViewList, "", "" );
	AddPropertyToViewList(pCallback, &pCallback->EarlyZTest, pViewList, "", "" );
	AddPropertyToViewList(pCallback, &pCallback->SupportLegacyShaders, pViewList, "", "" );

	AddPropertyToViewList(pCallback, &pCallback->UpdateType, pViewList, "", "" );
	AddPropertyToViewList(pCallback, &pCallback->Update, pViewList, "", "" );
	AddPropertyToViewList(pCallback, &pCallback->TimeToPartlyUpdate, pViewList, "", "" );
	AddPropertyToViewList(pCallback, &pCallback->CurrentComposition, pViewList, "", "" );

	AddPropertyToViewList(pCallback, &pCallback->CubeMapSize, pViewList, "", "Equirectangular Panorama" );
	AddPropertyToViewList(pCallback, &pCallback->PanoWidth, pViewList, "", "Equirectangular Panorama" );
	AddPropertyToViewList(pCallback, &pCallback->PanoHeight, pViewList, "", "Equirectangular Panorama" );
}

void MoRendererCallback::CProperties::Create(FBComponent *pParent, const char *prefix)
{
	mQualityNames.Add( FBString(prefix, "Display Depth" ) );
	mQualityNames.Add( FBString(prefix, "Quality Presets" ) );
	mQualityNames.Add( FBString(prefix, "Depth Samples" ) );
	mQualityNames.Add( FBString(prefix, "Coverage Samples" ) );
	mQualityNames.Add( FBString(prefix, "Supersample Scale" ) );
	mQualityNames.Add( FBString(prefix, "Downsample Filter" ) );
	mQualityNames.Add( FBString(prefix, "Enable Secondary" ) );
	mQualityNames.Add( FBString(prefix, "Quality Presets 2" ) );
	mQualityNames.Add( FBString(prefix, "Depth Samples 2" ) );
	mQualityNames.Add( FBString(prefix, "Coverage Samples 2" ) );
	mQualityNames.Add( FBString(prefix, "Supersample Scale 2" ) );
	mQualityNames.Add( FBString(prefix, "Downsample Filter 2" ) );
	mQualityNames.Add( FBString(prefix, "Use SuperSampling Tiles" ) );
	mQualityNames.Add( FBString(prefix, "Tiles Hor" ) );
	mQualityNames.Add( FBString(prefix, "Tiles Ver" ) );
	mQualityNames.Add( FBString(prefix, "Max CubeMap Size" ) );
	mQualityNames.Add( FBString(prefix, "Max Camera Size" ) );
	mQualityNames.Add( FBString(prefix, "Max Shadow Size" ) );
	mQualityNames.Add( FBString(prefix, "Enable Shadow Multisampling" ) );
	mQualityNames.Add( FBString(prefix, "Shadow Samples" ) );

	mQuality.DisplayDepth = new FBPropertyBaseEnum<EGraphicsDepth>();
	mQuality.QualityPreset = new FBPropertyBaseEnum<EGraphicsQualityPresets>();
	mQuality.DepthSamples = new FBPropertyInt();
	mQuality.CoverageSamples = new FBPropertyInt();
	mQuality.SupersampleScale = new FBPropertyDouble();
	mQuality.DownsampleType = new FBPropertyBaseEnum<EResamplingFilter>();

	mQuality.EnableSecondary = new FBPropertyBool();
	mQuality.QualityPreset2 = new FBPropertyBaseEnum<EGraphicsQualityPresets>();
	mQuality.DepthSamples2 = new FBPropertyInt();
	mQuality.CoverageSamples2 = new FBPropertyInt();
	mQuality.SupersampleScale2 = new FBPropertyDouble();
	mQuality.DownsampleType2 = new FBPropertyBaseEnum<EResamplingFilter>();

	mQuality.UseSuperSamplingTiles = new FBPropertyBool();
	mQuality.TilesHor = new FBPropertyInt();
	mQuality.TilesVer = new FBPropertyInt();

	mQuality.MaxCubeMapSize = new FBPropertyInt();
	mQuality.MaxCameraSize = new FBPropertyInt();
	mQuality.MaxShadowSize = new FBPropertyInt();
	mQuality.EnableShadowMultisampling = new FBPropertyBool();
	mQuality.ShadowSamples = new FBPropertyInt();

	for (int i=0; i<NUMBER_OF_QUALITY_PROPS; ++i)
	{
		const char *name = mQualityNames[i];
		FBProperty *pInternal = mQuality.props[i]->CreateInternal(pParent, name, nullptr, nullptr );
		pParent->PropertyAdd( pInternal );
	}

	mQuality.DepthSamples->SetMinMax( 0.0, 64.0, true, true );
	mQuality.CoverageSamples->SetMinMax( 0.0, 64.0, true, true );
	mQuality.SupersampleScale->SetMinMax( 100.0, 1000.0, false, false );
	
	mQuality.DepthSamples2->SetMinMax( 0.0, 64.0, true, true );
	mQuality.CoverageSamples2->SetMinMax( 0.0, 64.0, true, true );
	mQuality.SupersampleScale2->SetMinMax( 100.0, 1000.0, false, false );

	mQuality.UseSuperSamplingTiles->SetInt(1);
	mQuality.TilesHor->SetInt(1);
	mQuality.TilesVer->SetInt(1);

	mQuality.TilesHor->SetMinMax(1.0, 8.0, true, true);
	mQuality.TilesVer->SetMinMax(1.0, 8.0, true, true);

	mQuality.DepthSamples->SetInt(0);
	mQuality.CoverageSamples->SetInt(0);
	double dvalue = 100.0;
	mQuality.SupersampleScale->SetData(&dvalue);
	mQuality.DownsampleType->SetInt( (int) eResamplingFilterKernel);

	mQuality.DepthSamples2->SetInt(0);
	mQuality.CoverageSamples2->SetInt(0);
	mQuality.SupersampleScale2->SetData(&dvalue);
	mQuality.DownsampleType2->SetInt( (int) eResamplingFilterKernel);

	mQuality.QualityPreset->SetInt( (int) eGraphicsQualitySimple );
	mQuality.QualityPreset2->SetInt( (int) eGraphicsQualitySimple );
	mQuality.DisplayDepth->SetInt( (int) eGraphicsDepthLinear );
	
	mQuality.MaxCameraSize->SetInt(8192);
	mQuality.MaxCubeMapSize->SetInt(2048);
	mQuality.MaxShadowSize->SetInt(2048);

	mQuality.EnableShadowMultisampling->SetInt(0);
	mQuality.ShadowSamples->SetInt(8);

	//

	mStagesNames.Add( FBString( prefix, "Enable Dynamic Masks" ) );
	mStagesNames.Add( FBString( prefix, "Enable Compositions" ) );
	mStagesNames.Add( FBString( prefix, "Enable CubeMap Rendering" ) );
	mStagesNames.Add( FBString( prefix, "Enable Camera Rendering" ) );
	mStagesNames.Add( FBString( prefix, "Enable Shadows" ) );
	mStagesNames.Add( FBString( prefix, "Enable Planar Reflections" ) );
	mStagesNames.Add( FBString( prefix, "Render Normal And Mask" ) );

	for (int i=0; i<NUMBER_OF_STAGES_PROPS; ++i)
	{
		mStages.props[i] = new FBPropertyBool();

		const char *name = mStagesNames[i];
		FBProperty *pInternal = mStages.props[i]->CreateInternal(pParent, name, nullptr, nullptr );
		pParent->PropertyAdd( pInternal );

		mStages.props[i]->SetInt(1);
	}

	//

	mShadingNames.Add( FBString( prefix, "Texture Mapping" ) );
	mShadingNames.Add( FBString( prefix, "Use Scene Lights" ) );
	mShadingNames.Add( FBString( prefix, "Override Shading" ) );
	mShadingNames.Add( FBString( prefix, "Shading Type" ) );

	mShading.TextureMapping = new FBPropertyBool();
	mShading.SceneLighting = new FBPropertyBool();
	mShading.OverrideShading = new FBPropertyBool();
	mShading.ShadingType = new FBPropertyBaseEnum<EShadingType>();

	for (int i=0; i<NUMBER_OF_SHADING_PROPS; ++i)
	{
		const char *name = mShadingNames[i];
		FBProperty *pInternal = mShading.props[i]->CreateInternal(pParent, name, nullptr, nullptr );
		pParent->PropertyAdd( pInternal );
	}

	mShading.TextureMapping->SetInt(1);
	mShading.SceneLighting->SetInt(1);
	mShading.OverrideShading->SetInt(0);
	mShading.ShadingType->SetInt( eShadingTypeFlat );
}

void MoRendererCallback::CCompositionOptions::Create(FBComponent *pParent)
{
	mNames.SetString( "Render Only Current~Show As Preview~Preview As SubView"
		"~Preview Show Title~Preview Size~Preview On Play~Preview On Holding Alt"
		"~Disable Preview On Render~Compute Group Size~Timing" );

	mData.RenderOnlyCurrent = new FBPropertyBool();
	mData.ShowAsPreview = new FBPropertyBool();
	mData.PreviewAsSubView = new FBPropertyBool();
	mData.PreviewShowTitle = new FBPropertyBool();
	mData.PreviewSize = new FBPropertyDouble();

	mData.PreviewOnPlay = new FBPropertyBool();
	mData.PreviewOnHoldingAlt = new FBPropertyBool();
	mData.DisablePreviewOnRender = new FBPropertyBool();
	mData.ComputeGroupSize = new FBPropertyInt();
	mData.Timing = new FBPropertyDouble();

	for (int i=0; i<NUMBER_OF_COMP_OPTIONS; ++i)
	{
		const char *name = mNames[i];
		FBProperty *pInternal = mData.props[i]->CreateInternal(pParent, name, nullptr, nullptr );
		pParent->PropertyAdd( pInternal );
	}

	double dvalue = 50.0;
	mData.PreviewSize->SetMinMax(1.0, 100.0);
	mData.PreviewSize->SetData(&dvalue);
	mData.Timing->ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	mData.PreviewOnHoldingAlt->SetInt(1);
	mData.PreviewOnPlay->SetInt(1);
	mData.DisablePreviewOnRender->SetInt(1);
}

void MoRendererCallback::CRenderStats::Create(FBComponent *pParent)
{
	mNames.SetString( "Timing Dynamic Masks~Timing Extended Rendering"
		"~Timing Shadows~Timing Scene Preparation"
		"~Total Opaque Shaders~Total Transparency Shaders"
		"~Total Opaque Models~Total Transparent Models"
		"~Composition Width~Composition Height~Number Of Compute Shaders"
		"~Number Of Dispatch Groups~Number Of Textures~Textures Memory" );

	double dvalue = 0.0;

	for (int i=0; i<NUMBER_OF_STATS_PROPERTIES; ++i)
	{
		mData.props[i] = (i<4) ? (FBProperty*) new FBPropertyDouble() : (FBProperty*) new FBPropertyInt();

		const char *name = mNames[i];
		FBProperty *pInternal = mData.props[i]->CreateInternal(pParent, name, nullptr, nullptr );
		pParent->PropertyAdd( pInternal );

		mData.props[i]->ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
		if (i<4)
			mData.props[i]->SetData(&dvalue);
		else
			mData.props[i]->SetInt(0);
	}
}

void MoRendererCallback::CGlobalResolution::Create(FBComponent *pParent)
{
	mNames.SetString( "Use Global Resolution~Exclude Projector Cameras"
		"~Render Resolution~Custom Width~Custom Height" );

	mData.UseGlobalResolution = new FBPropertyBool();
	mData.ExcludeProjectorCameras = new FBPropertyBool();
	mData.RenderResolution = new FBPropertyBaseEnum<ERendererEnum>();
	mData.CustomWidth = new FBPropertyInt();
	mData.CustomHeight = new FBPropertyInt();

	for (int i=0; i<NUMBER_OF_GR_ITEMS; ++i)
	{
		const char *name = mNames[i];
		FBProperty *pInternal = mData.props[i]->CreateInternal(pParent, name, nullptr, nullptr );
		pParent->PropertyAdd( pInternal );
	}

	mData.UseGlobalResolution->SetInt(0);
	mData.ExcludeProjectorCameras->SetInt(1);
	//RenderResolution = eGraphicsResolutionHD;
	mData.CustomWidth->SetInt(1920);
	mData.CustomHeight->SetInt(1080);
}

bool MoRendererCallback::FBCreate()
{
	glewInit();
	
	if (glewIsSupported( "GL_ARB_bindless_texture" ) == GL_FALSE || glewIsSupported( "GL_NV_shader_buffer_load" ) == GL_FALSE )
	{
		FBMessageBox( "MoPlugs Warning", "Advance features are not supported by your video card!", "Ok" );
		FBTrace( "> !! Bindless textures are NOT supported!\n" );
		mSupported = false;
	}
	else
	{
		FBTrace( "> Bindless textures supported!\n" );
	}

	//
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif

    //
    // Register task cycle index in profiler.
    //
    FBProfiling_SetupTaskCycle( VRRenderer );

    //
    //  Enabled advanced lighting mode, include UI widgets for various advanced lighting setting, includes: 
    //   Light: area light, spot light inner/outer angles, barndoors and etc.,; 
    //   Model: PrimaryVisibility, CastsShadows and ReceiveShadows.
    //
	FBSystem::TheOne().Renderer->AdvancedMaterialMode = true;
    FBSystem::TheOne().Renderer->AdvancedLightingMode = true;

	DefaultLightGroundProjectionRendering = false;
	DefaultLightVolumeRendering = false;
	SupportIDBufferPicking  = true;

    //
    // Init properties. 
    //
    
	FBPropertyPublish(this, TestBool, "Test Bool", nullptr, nullptr );
	FBPropertyPublish(this, TestAction, "Test", nullptr, OnTestActionSet);
	FBPropertyPublish(this, Reset, "Reset", nullptr, SetReset);

	FBPropertyPublish(this, RenderQualityOverride, "Render Quality Override", nullptr, nullptr );
	FBPropertyPublish(this, RenderShadingOverride, "Render Shading Override", nullptr, nullptr);

    FBPropertyPublish(this, NoFrustumculling,       "NoFrustumculling",         NULL, NULL);
	FBPropertyPublish(this, EarlyZTest,    "EarlyZ Test",     NULL, NULL);
	
	FBPropertyPublish(this, CubeMapSize,    "CubeMap Size",     NULL, VRRenderer_CubeMapSizeSet);
	FBPropertyPublish(this, PanoWidth,    "Panorama Width",     NULL, NULL);
	FBPropertyPublish(this, PanoHeight,    "Panorama Height",     NULL, NULL);

	FBPropertyPublish(this, CurrentComposition, "Current Composition", nullptr, nullptr);

	FBPropertyPublish(this, UpdateType, "Update Type", nullptr, nullptr);
	FBPropertyPublish(this, Update, "Update", nullptr, SetRenderUpdateAction);
	FBPropertyPublish(this, TimeToPartlyUpdate, "Time To Partly Update", nullptr, nullptr );

	TestBool = true;

    RenderQualityOverride = false;
	RenderShadingOverride = false;
   
    NoFrustumculling	= false;
	EarlyZTest			= false;
	
	CubeMapSize = 512;
	PanoWidth = 2048;
	PanoHeight = 1024;

	CurrentComposition.SetSingleConnect( true );
	CurrentComposition.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	UpdateType = eRendererUpdatePartly;
	TimeToPartlyUpdate = 0.5;

	//
	FBCreateSettingsProperties();

	//
    // Init data members.

    mAttachCount            = 0;
	mLastContext = 0;
	mTestTexture = nullptr;
	mIsRendering = false;
	mVideoRendering = false;

	mLastRenderingFrameId = 0;
	mLastRenderUpdateId = 0;
	mLastDisplayUpdateId = 0;

	mNeedToRender = true;

	mLastLocalTime = FBTime::Infinity;
	mLastSystemTime = FBTime::Infinity;
	mPartlyRenderIndex = 0;

	mGPUFBScene = &CGPUFBScene::instance();

	//
	mBackgroungLayer.mFrameBuffer.InitTextureInternalFormat();
	mMainLayer.mFrameBuffer.InitTextureInternalFormat();
	mSecondaryLayer.mFrameBuffer.InitTextureInternalFormat();
	mTileLayer.mFrameBuffer.InitTextureInternalFormat();
	//mTransparencyLayer.mFrameBuffer.InitTextureInternalFormat();
	
	mNumberOfTiles = 1;
	mTilesSW = 1;
	mTilesSH = 1;

	mWait = false;

	//
	// events

	mApplication.OnFileNewCompleted.Add( this, (FBCallback) &MoRendererCallback::EventFileNew );
	mApplication.OnFileOpen.Add( this, (FBCallback) &MoRendererCallback::EventFileOpen );
	mApplication.OnFileOpenCompleted.Add( this, (FBCallback) &MoRendererCallback::EventFileOpenComplete );

	mSystem.Scene->OnChange.Add( this, (FBCallback) &MoRendererCallback::EventSceneChange );
	
	mSystem.OnUIIdle.Add( this, (FBCallback) &MoRendererCallback::OnUIIdle );

	mSystem.OnVideoFrameRendering.Add( this, (FBCallback) &MoRendererCallback::OnVideoFrameRendering );

	FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Add(this, (FBCallback)&MoRendererCallback::OnPerFrameEvaluationPipelineCallback);
	FBEvaluateManager::TheOne().OnSynchronizationEvent	 .Add(this, (FBCallback)&MoRendererCallback::OnPerFrameSynchronizationCallback);
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Add(this, (FBCallback)&MoRendererCallback::OnPerFrameRenderingPipelineCallback);

	AddPropertiesToPropertyViewManager(this);

    return true;
} 

void MoRendererCallback::FBCreateSettingsProperties()
{
	mDefaultPreset = -1;
	LoadResolutionPresets();

	mDisplayProperties.Create(this, "");
	mRenderProperties.Create(this, "Render ");
	mCurrentProperties = &mDisplayProperties;

	mCompositionOptions.Create(this);
	mRenderStats.Create(this);
	mGlobalResolution.Create(this);

	// shadows options
	/*
	// TODO: this goes to lights
	ShadowOptions &so = mShadowOptions;
	FBPropertyPublish(this, so.Size, "Shadows Size", nullptr, nullptr);
	FBPropertyPublish(this, so.OffsetFactor, "Shadows Offset Factor", nullptr, nullptr);
	FBPropertyPublish(this, so.OffsetUnits, "Shadows Offset Units", nullptr, nullptr);
	FBPropertyPublish(this, so.Bias, "Shadows Bias", nullptr, nullptr);
	FBPropertyPublish(this, so.UseGeomCache, "Shadows Use Geom Cache", nullptr, nullptr);
	FBPropertyPublish(this, so.CascadedLight, "Shadows Cascaded Light", nullptr, nullptr);
	FBPropertyPublish(this, so.CascadedSplits, "Shadows Cascaded Splits", nullptr, nullptr);
	FBPropertyPublish(this, so.CascadedSplitCorrection, "Shadows Cascaded Split Correction", nullptr, nullptr);
	FBPropertyPublish(this, so.CascadedNearPlane, "Shadows Cascaded Near Plane", nullptr, nullptr);
	FBPropertyPublish(this, so.CascadedFarPlane, "Shadows Cascaded Far Plane", nullptr, nullptr);
	*/
	
	
	//FBPropertyPublish(this, RenderResolution, "Resolution Preset", nullptr, SetResolution);
	//RenderResolution = (FBPropertyEnum*) PropertyCreate( "Resolution Preset", kFBPT_enum, "ENUM", false, false );
	
	//
	//

	
	/*
	so.Size.SetMinMax(2.0, 8096.0, true, true);
	so.CascadedLight.SetSingleConnect(true);
	so.CascadedLight.SetFilter( FBLight::GetInternalClassId() );
	so.CascadedSplits.SetMinMax(1.0, 15.0, true, true);
	so.CascadedSplitCorrection = 80.0;
	so.CascadedNearPlane = 1.0;
	so.CascadedFarPlane = 40000.0;
	*/
	
	//
	//

	FBStringList *pStringList = mGlobalResolution.mData.RenderResolution->GetEnumStringList(true);
	int refIndex = 0;

	pStringList->Clear();
	for (auto iter=begin(mPresets); iter!=end(mPresets); ++iter)
	{
		pStringList->Add(iter->name, refIndex);
		refIndex += 1;
	}

	if (mDefaultPreset >= 0)
	{
		mGlobalResolution.mData.RenderResolution->SetInt(mDefaultPreset);
		ApplyResolutionPreset(mDefaultPreset);
	}

	//
	//

	ResetToDefault();
}

bool MoRendererCallback::getOrSetBool (FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, bool defValue) 
{		
	sprintf_s( buffer, sizeof(char)*256, (defValue) ? "1" : "0" );
	const char *lbuffer = buffer;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);

	return (strcmp(buffer, "1") == 0);
};

int MoRendererCallback::getOrSetInt (FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, int defValue) 
{	
	sprintf_s( buffer, sizeof(char)*256, "%d", defValue );
	const char *lbuffer = buffer;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);

	return atoi(buffer);
};

double MoRendererCallback::getOrSetDouble (FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, double defValue)
{		
	sprintf_s( buffer, sizeof(char)*256, "%lf", defValue );
	const char *lbuffer = buffer;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);

	return atof(buffer);
};

void MoRendererCallback::getOrSetString (FBString &value, FBConfigFile &lConfigFile, const char *section, const char *name, const char *comment)
{
	//sprintf_s( buffer, sizeof(char)*256, "%s", defValue );
	const char *lbuffer = value;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);
}

void MoRendererCallback::LoadPropertiesFromConfig(CBaseProperties *props, FBConfigFile &lConfig)
{
	if (false == props->ConfigSerialization() )
		return;

	char buffer[256];

	for (int i=0; i<props->GetNumberOfGroups(); ++i)
	{
		const char *sessionName = props->GroupName(i);

		FBProperty **groupProps = props->GetPropertiesPtr(i);
		FBStringList *pNames = props->GetPropertiesNamesPtr(i);

		for (int j=0; j<props->GetNumberOfItems(i); ++j)
		{
			FBProperty *prop = groupProps[j];
			FBPropertyType type = prop->GetPropertyType();
			const char *propName = pNames->GetAt(j);

			switch(type)
			{
			case kFBPT_int:
			case kFBPT_bool:
				{
					int defValue = prop->AsInt();
					int value = getOrSetInt(lConfig, buffer, sessionName, propName, "", defValue);
					prop->SetInt(value);
				} break;
		
			case kFBPT_double:
				{
					double defValue;
					prop->GetData(&defValue, sizeof(double));
					double value = getOrSetDouble(lConfig, buffer, sessionName, propName, "", defValue);
					prop->SetData( &value );
				} break;
			case kFBPT_enum:
				{
					FBString defValue = prop->AsString();
					getOrSetString(defValue, lConfig, sessionName, propName, "");
					prop->SetString(defValue);
				} break;
			}
		}
	}
}

void MoRendererCallback::ResetToDefault()
{
	FBConfigFile	lConfigFile( "@MoPlugsSettings.txt" );

	LoadPropertiesFromConfig( &mDisplayProperties, lConfigFile );
	LoadPropertiesFromConfig( &mRenderProperties, lConfigFile );

	LoadPropertiesFromConfig( &mCompositionOptions, lConfigFile );
	LoadPropertiesFromConfig( &mRenderStats, lConfigFile );
	LoadPropertiesFromConfig( &mGlobalResolution, lConfigFile );
}

void MoRendererCallback::RunTestAction()
{
	mLastContext = (HGLRC) 9901;
}

void MoRendererCallback::FBDestroy()
{
	FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Remove(this, (FBCallback)&MoRendererCallback::OnPerFrameEvaluationPipelineCallback);
	FBEvaluateManager::TheOne().OnSynchronizationEvent	 .Remove(this, (FBCallback)&MoRendererCallback::OnPerFrameSynchronizationCallback);
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Remove(this, (FBCallback)&MoRendererCallback::OnPerFrameRenderingPipelineCallback);

	mApplication.OnFileNewCompleted.Remove( this, (FBCallback) &MoRendererCallback::EventFileNew );
	mApplication.OnFileOpen.Remove( this, (FBCallback) &MoRendererCallback::EventFileOpen );
	mApplication.OnFileMerge.Remove( this, (FBCallback) &MoRendererCallback::EventFileMerge );
	mApplication.OnFileOpenCompleted.Remove( this, (FBCallback) &MoRendererCallback::EventFileOpenComplete );

	mSystem.Scene->OnChange.Remove( this, (FBCallback) &MoRendererCallback::EventSceneChange );
	mSystem.OnConnectionNotify.Remove(this, (FBCallback)&MoRendererCallback::EventConnNotify);
	mSystem.OnConnectionDataNotify.Remove(this, (FBCallback)&MoRendererCallback::EventConnDataNotify);
	mSystem.OnVideoFrameRendering.Remove( this, (FBCallback) &MoRendererCallback::OnVideoFrameRendering );

}

const char* MoRendererCallback::GetCallbackName() const
{
    return MORendererCallback__CLASSSTR;
}

const char* MoRendererCallback::GetCallbackDesc() const
{
    // Return description which 
    return MORendererCallback__DESCSTR;
}

unsigned int MoRendererCallback::GetCallbackPrefCount() const
{
    // Set preference count to 3. And this count need to match the following GetCallbackPrefName() function. 
    return 3;
}

const char*  MoRendererCallback::GetCallbackPrefName(unsigned int pIndex) const
{
    // Return preference description.
    switch (pIndex)
    {
    case 0: 
        return "High Quality";
	case 1:
		return "Equirectangular Panorama";
	case 2:
		return "VR - under construction";
    default: 
        return "Wrong Pref Index!!!";
    }
}

void MoRendererCallback::Attach()
{
    //
    // This callback occurs when one view pane attach this renderer callback instance.
    //
    FBTrace("VRRendererCallback::Attach()\n");

    //
    // Prepare for first time. 
    //
    if (mAttachCount == 0)
    {
        // Register callback function for critical timings at the pipeline.
        //FBEvaluateManager::TheOne().OnSynchronizationEvent   .Add(this, (FBCallback)&VRRendererCallback::OnPerFrameSynchronizationCallback);
        //FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Add(this, (FBCallback)&VRRendererCallback::OnPerFrameEvaluationPipelineCallback);
        //FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Add(this, (FBCallback)&VRRendererCallback::OnPerFrameRenderingPipelineCallback);

        // Register callback function for scene change events. 
        //mSystem.Scene->OnChange.Add (this, (FBCallback) &VRRendererCallback::EventSceneChange );

        //mSystem.OnConnectionNotify.Add(this, (FBCallback)&VRRendererCallback::EventConnNotify);
        //mSystem.OnConnectionStateNotify.Add(this, (FBCallback)&VRRendererCallback::EventConnStateNotify);
        //mSystem.OnConnectionDataNotify.Add(this, (FBCallback)&VRRendererCallback::EventConnDataNotify);

		// ask for preparing uber shader next rendering frame
		
		mGPUFBScene->ChangeContext(nullptr);
		HGLRC currContext = wglGetCurrentContext();
		mLastContext = currContext;

		gCallbackAttached = true;
		mNeedToRender = true;

		mGPUFBScene->GetUberShaderPtr();
    }

    // Increase attachment count.
    mAttachCount++;
}

void MoRendererCallback::Detach()
{
    //
    // This callback occurs when one view pane detach this renderer callback instance.
    //
    FBTrace("VRRendererCallback::Detach()\n");

    // Decrease attachment count.
    mAttachCount--;

    // 
    // This callback instance is not be used by any view panes anymore. 
    //
    if (mAttachCount == 0)
    {
        // Unregister callback function for critical timings at the pipeline.
        //FBEvaluateManager::TheOne().OnSynchronizationEvent   .Remove(this, (FBCallback)&VRRendererCallback::OnPerFrameSynchronizationCallback);
        //FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Remove(this, (FBCallback)&VRRendererCallback::OnPerFrameEvaluationPipelineCallback);
        //FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Remove(this, (FBCallback)&VRRendererCallback::OnPerFrameRenderingPipelineCallback);

        // Unregister callback function for scene change events. 
		//mSystem.Scene->OnChange.Remove(this, (FBCallback) &VRRendererCallback::EventSceneChange );

        //mSystem.OnConnectionNotify.Remove(this, (FBCallback)&VRRendererCallback::EventConnNotify);
        //mSystem.OnConnectionStateNotify.Remove(this, (FBCallback)&VRRendererCallback::EventConnStateNotify);
        //mSystem.OnConnectionDataNotify.Remove(this, (FBCallback)&VRRendererCallback::EventConnDataNotify);

		mGPUFBScene->ChangeContext(nullptr);

		mLastContext = 0;
		gLastContext = 0;

		gCallbackAttached = false;
    }
}

void MoRendererCallback::DetachDisplayContext(FBViewingOptions* pViewOption)
{
    //
    // This callback occurs when no view pane use this renderer callback instance anymore, 
    // or some internal events request GL context invalidation. 
    // We should release allocated OpenGL resources here.
    //
	if (true == mVideoRendering)
		FBTrace("VRRendererCallback::DetachDisplayContext()\n");

	mBackgroungLayer.mFrameBuffer.ChangeContext();
	mMainLayer.mFrameBuffer.ChangeContext();
	mSecondaryLayer.mFrameBuffer.ChangeContext();
	mTileLayer.mFrameBuffer.ChangeContext();
	//mTransparencyLayer.mFrameBuffer.ChangeContext();

	mGPUFBScene->ChangeContext(nullptr);

	Compositions_ChangeContext();
	DynamicMasks_ChangeContext();
	
	HGLRC currContext = wglGetCurrentContext();
	mLastContext = currContext;
	gLastContext = currContext;
}

void MoRendererCallback::CheckAdditionalLayersNeeded(CRenderOptions &options)
{
	bool cameraBack = false;

	FBVideo *pVideo = options.GetCamera()->BackGroundMedia;
	FBTexture *pTexture = options.GetCamera()->BackGroundTexture;

	if (pVideo != nullptr || pTexture != nullptr)
	{
		cameraBack = true;
	}

	// TODO: we need background layer only in case we have any Matte shader 
	//		or Matte shading inside the ProjTex
	// TODO: we need transparency layer only in case we have any Transparency option
	//	inside the ProjTex, Flat, Dynamic Lighting, GPU Cache, etc.

	// TODO: let's check this inside the scene events !!

	options.SetCameraBackground(cameraBack);
	
	const CRenderLayerArray &layersInfo = mGPUFBScene->GetRenderLayersInfo();

	options.SetRenderToBackgroundLayer( (true == layersInfo.HasMatteShaders(eRenderLayerMain)) ||
		(true == cameraBack) || (layersInfo.GetNumberOfShaders(eRenderLayerBackground) > 0) );

	options.SetRenderToTransparencyLayer( layersInfo.GetNumberOfShaders(eRenderLayerSecondary) > 0 );
}

void MoRendererCallback::PrepRenderOptions(CRenderOptions &options, FBRenderOptions* pFBRenderOptions)
{
	if (mVideoRendering && RenderQualityOverride.AsInt() > 0)
	{
		mCurrentProperties = &mRenderProperties;
	}
	else
	{
		mCurrentProperties = &mDisplayProperties;
	}

	//options.GrabCameraViewportSize();
	options.SetOfflineRender( mVideoRendering );
	options.SetTextureMapping( mCurrentProperties->mShading.TextureMapping->AsInt() > 0 );
	options.SetFrustumCulling( NoFrustumculling.AsInt() == 0 );

	options.SetRenderLayerId(eRenderLayerMain);

	//
	double ssfact;
	int samples, coverageSamples;
	EGraphicsDepth currDepthQuality;
	EResamplingFilter currResamplingFilter;
	bool useSuperSamplingTiles;

	MoRendererCallback::CProperties::CQuality &quality = mCurrentProperties->mQuality;
	
	quality.SupersampleScale->GetData( &ssfact, sizeof(double) );
	ssfact = 0.01 * ssfact;

	samples = quality.DepthSamples->AsInt();
	coverageSamples = quality.CoverageSamples->AsInt();
	currDepthQuality = (EGraphicsDepth) quality.DisplayDepth->AsInt();
	currResamplingFilter = (EResamplingFilter) quality.DownsampleType->AsInt();
	useSuperSamplingTiles = (quality.UseSuperSamplingTiles->AsInt() > 0);

	int samples2 = samples;
	int coverageSamples2 = coverageSamples;
	if (quality.EnableSecondary->AsInt() > 0)
	{
		samples2 = quality.DepthSamples2->AsInt();
		coverageSamples2 = quality.CoverageSamples2->AsInt();
	}
	
	if (ssfact < 1.0) ssfact = 1.0;
	if (ssfact > 4.0) ssfact = 4.0;

	// NOTE: scale factor is depricated, use tiles instead !!
	ssfact = 1.0;

	// NOTE: color samples must not exceed coverage samples
	if (coverageSamples < samples)
		coverageSamples = samples;
	if (coverageSamples2 < samples2)
		coverageSamples2 = samples2;

	if (true == useSuperSamplingTiles)
	{
		mTilesSW = quality.TilesHor->AsInt();
		mTilesSH = quality.TilesVer->AsInt();
	}
	else
	{
		mTilesSW = 1;
		mTilesSH = 1;
	}

	options.SetSuperSamplingScale(ssfact);
	options.SetResampleFilter(currResamplingFilter);

	const bool enableMultiSampling = (samples > 0);
	const bool enableMultiSampling2 = (samples2 > 0);
	options.SetSamplingOptions(enableMultiSampling, samples, coverageSamples);
	options.SetSamplingOptions2(enableMultiSampling2, samples2, coverageSamples2);
	/*
	if (currDepthQuality == eGraphicsDepthNV)
	{
		glDepthRangedNV(-1.0, 1.0);
	}
	else
	{
		glDepthRange(0.0, 1.0);
	}
	*/
	
	options.SetShadowParams( mCurrentProperties->MaxShadowResolution(),
		mCurrentProperties->IsShadowMultisamplingEnabled(),
		mCurrentProperties->ShadowSamples() );

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MoRendererCallback::Render(FBRenderOptions* pFBRenderOptions)
{
	if (true == mVideoRendering)
		FBTrace( "begin frame render\n" );
	// wait for global scene operation (open, clear, merge, etc.)

	if ( FBMergeTransactionIsOn() || FBMergeTransactionFileRefEditIsOn() )
		return;

	//int currIndex = pFBRenderOptions->GetViewerOptions()->RenderCallbackPrefIndex();

    //
    // Start CustomRenderer task cycle profiling, 
    //
    FBProfilerHelper lProfiling( FBProfiling_TaskCycleIndex( VRRenderer ), FBGetDisplayInfo() );

#ifdef _DEBUG
	FBTime systemTime = mSystem.SystemTime;
	kLongLong msBeginTime = systemTime.GetMilliSeconds();
#endif

    //
    // Push all GL attributes and pop at the end to avoid the in perfect 
    // implementations affect the rest MoBu rendering functionality.
    //
	
#ifdef _DEBUG
	if (false == TestBool)
		return;
#endif

    glPushAttrib(GL_ALL_ATTRIB_BITS | GL_CLIENT_ALL_ATTRIB_BITS);

	memset( &mFrameBufferInfo, 0, sizeof(FrameBufferInfo) );
	SaveFrameBuffer( &mFrameBufferInfo );

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
    //
    // In this example, we demo a simple forward rendering implementation on the
    // default FBO provided by MoBu. 
    //
    // However plugin developer may want to implement their own deferred renderer
    // by creating multiple additional FBOs, and perform creative rendering algorithms,
    // blit the final result back to Color Buffer of MoBu's default FBO at the end.
    //
	//mSystem.Renderer->RendererUpdateId
	HGLRC currContext = wglGetCurrentContext();
	if (mLastContext != 0 && currContext != mLastContext)
	{
		DetachDisplayContext(nullptr);
		//LoadShader();
		mNeedToRender = true;
		DynamicMasks_Render();
	}
	else if (mLastContext == 0)
	{
		mLastContext = currContext;
	}

	//
	if (mVideoRendering || UpdateType == eRendererUpdateRealTime)
	{
		mNeedToRender = true;

		const int renderUpdateid = mSystem.Renderer->RendererUpdateId;
		if (renderUpdateid != mLastRenderUpdateId)
		{
			// mGPUFBScene->Textures_InitOGL(nullptr);
			mLastRenderUpdateId = renderUpdateid;
		}
	}
	else if (eRendererUpdatePartly == UpdateType && false == mNeedToRender)
	{
		ComputeIfNeededPartlyUpdate();
	}

	// evaluate all needed frame buffer to render !
	//if (mNeedToRender == true)
	//{
	bool logDepth = mDisplayProperties.IsLogarithmicDepth();
	if (true == mVideoRendering && true == RenderQualityOverride)
		logDepth = mRenderProperties.IsLogarithmicDepth();

	mGPUFBScene->SetLogarithmicDepth( logDepth );

	//}
	CRenderOptions	&renderOptions = GetLastRenderOptions();
	PrepRenderOptions(renderOptions, pFBRenderOptions);
	
	//
	//

	Compositions_FindCurrent( pFBRenderOptions->GetRenderingCamera() );

	//
	//
	
	const bool processCompositions = ( true == mCurrentProperties->ProcessCompositions()
		&& nullptr != mCompositionOutput );
	
	int maxShadowResolution = mCurrentProperties->MaxShadowResolution();
	if (maxShadowResolution > 4096)
		maxShadowResolution = 4096;
	else if (maxShadowResolution < 256)
		maxShadowResolution = 256;

#ifdef _DEBUG
	systemTime = mSystem.SystemTime;
	kLongLong msBeforeInit = systemTime.GetMilliSeconds() - msBeginTime;
#endif

	InitializeFrameDataAndBuffers( pFBRenderOptions,
			pFBRenderOptions->GetRenderingCamera(), 
			FBGetDisplayInfo(),
			pFBRenderOptions->GetRenderFrameId(), 
			false,
			mCurrentProperties->MaxCameraResolution(),
			mCurrentProperties->MaxCubeMapResolution(), 
			processCompositions, mCompositionOutput,
			maxShadowResolution );
	
	// interchange with other shader plugins
	SetMoPlugsRender(true);
	SetCameraCache( mGPUFBScene->GetCameraCache() );

#ifdef _DEBUG
	systemTime = mSystem.SystemTime;
	kLongLong msAfterInit = systemTime.GetMilliSeconds() - msBeginTime;
#endif

	mGPUFBScene->SetLogarithmicDepth( logDepth );
	
	// experiment - tags for frustum culling models
	if (NoFrustumculling.AsInt() == 0)
	{
		FBRenderer *pRenderer = mSystem.Renderer;
		const FBModelList &modelList = pRenderer->GetDisplayableGeometryInCameraFrustum();
		const int id = pFBRenderOptions->GetRenderFrameId();

		for (int i=0, count=modelList.GetCount(); i<count; ++i)
		{
			FBModel *pModel = modelList[i];

			FBProperty *pProp = pModel->PropertyList.Find("id");
			if (nullptr == pProp)
			{
				pProp = pModel->PropertyCreate( "id", kFBPT_int, "int", false, false );
				pProp->ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
				pProp->ModifyPropertyFlag( kFBPropertyFlagNotSavable, true );
				
			}
			pProp->SetInt(id);	
		}
	}

	FBViewingOptions* lViewingOptions = pFBRenderOptions->GetViewerOptions();
    const bool lIsSelectBufferPicking = lViewingOptions->IsInSelectionBufferPicking();
    const bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	const bool renderForSelection = (lIsSelectBufferPicking || lIsColorBufferPicking);

	if (renderForSelection == false)
	{
		renderOptions.SetGoal( eRenderGoalShading );
		CheckAdditionalLayersNeeded(renderOptions);

		bool updated = mMainLayer.mFrameBuffer.ReSize(renderOptions.GetWidth(), 
			renderOptions.GetHeight(), 
			renderOptions.GetScaleFactor(),
			renderOptions.GetDepthSamples(),
			renderOptions.GetCoverageSamples() );

		bool logUpdated = mCompositionDataExchange.logDepth != mCurrentProperties->IsLogarithmicDepth();

		Compositions_PrepDataExchange( mCompositionDataExchange, renderOptions, mMainLayer.mFrameBuffer );

		if ( (true == updated) || (true == logUpdated) )
		{
			mNeedToRender = true;
			Compositions_ChangeGlobalSettings();
		}
	}
	else
	{
		renderOptions.SetGoal( eRenderGoalSelectionId );
	}

	//
	//

	if ( (false == renderForSelection) && (true == mNeedToRender) )
	{
		RenderingStats stats;

		//renderOptions.SetGoal(eRenderGoalBackground);

		// DONE: check if need to prepare and render a background
		//	when we have a camera background texture or any background shader
	
		/*
		if (renderOptions.IsRenderToBackgroundLayer() )
		{
			RenderToBackground(renderOptions);
		}
		*/
		//
		//

		renderOptions.SetGoal(eRenderGoalShading);

		const int samples = renderOptions.GetDepthSamples();
		const int coverageSamples = renderOptions.GetCoverageSamples();
		// for a secondary framebuffer
		const int samples2 = renderOptions.GetDepthSamples2();
		const int coverageSamples2 = renderOptions.GetCoverageSamples2();
		//
		const int totalCount = mTilesSW * mTilesSH;
		mNumberOfTiles = totalCount;
		//mTiles.resize(totalCount);
		
		const bool bComps = mCurrentProperties->ProcessCompositions();

		renderOptions.TurnOffCullingMode();

		if (totalCount <= 1)
		{
			const bool moreColorAttachments = (mCompositionOutput != nullptr);

			if (nullptr != mGPUFBScene->GetUberShaderPtr() )
			{
				mGPUFBScene->GetUberShaderPtr()->SetSoftParticles(0.0f);
			}

			// TODO: check we have any "render on back" shader
			//	check if we have a camera background image plane
			if (renderOptions.IsRenderToBackgroundLayer() )
			{
				// render to background
				RenderShadingToFramebuffer( eRenderLayerBackground, mBackgroungLayer.mFrameBuffer, false,
							renderOptions.GetWidth(), renderOptions.GetHeight(), samples, coverageSamples,
							renderOptions.GetScaleFactor(), true, renderOptions, pFBRenderOptions, false, stats );
			}

			//
			// render to main layer
			RenderShadingToFramebuffer( eRenderLayerMain, mMainLayer.mFrameBuffer, moreColorAttachments,
						renderOptions.GetWidth(), renderOptions.GetHeight(), samples, coverageSamples,
						renderOptions.GetScaleFactor(), true, renderOptions, pFBRenderOptions, bComps, stats );

			if (renderOptions.IsRenderToTransparencyLayer() )
			{
				if (nullptr != mGPUFBScene->GetUberShaderPtr() )
				{
					mGPUFBScene->GetUberShaderPtr()->SetSoftParticles(1.0f);

					glActiveTexture(GL_TEXTURE18);
					glBindTexture(GL_TEXTURE_2D, mMainLayer.mFrameBuffer.GetFinalDepthObject() );
					glActiveTexture(GL_TEXTURE0);
				}

				// render to secondary layer
				RenderShadingToFramebuffer( eRenderLayerSecondary, mSecondaryLayer.mFrameBuffer, moreColorAttachments,
						renderOptions.GetWidth(), renderOptions.GetHeight(), samples2, coverageSamples2,
						renderOptions.GetScaleFactor(), true, renderOptions, pFBRenderOptions, false, stats );

				if (nullptr != mGPUFBScene->GetUberShaderPtr() )
				{
					mGPUFBScene->GetUberShaderPtr()->SetSoftParticles(0.0f);

					glActiveTexture(GL_TEXTURE18);
					glBindTexture(GL_TEXTURE_2D, 0 );
					glActiveTexture(GL_TEXTURE0);
				}
			}
		}
		else
		{
			if (nullptr != mGPUFBScene->GetUberShaderPtr() )
			{
				mGPUFBScene->GetUberShaderPtr()->SetSoftParticles(0.0f);
			}

			// render to background

			// render to main layer
			RenderTilesToFramebuffer( eRenderLayerMain, mMainLayer.mFrameBuffer, mTileLayer.mFrameBuffer,
						renderOptions.GetWidth(), renderOptions.GetHeight(), samples, coverageSamples,
						renderOptions.GetScaleFactor(), true, renderOptions, pFBRenderOptions, bComps, stats );

			// render to secondary layer
			if (renderOptions.IsRenderToTransparencyLayer() )
			{
				if (nullptr != mGPUFBScene->GetUberShaderPtr() )
				{
					mGPUFBScene->GetUberShaderPtr()->SetSoftParticles(1.0f);

					glActiveTexture(GL_TEXTURE18);
					glBindTexture(GL_TEXTURE_2D, mMainLayer.mFrameBuffer.GetFinalDepthObject() );
					glActiveTexture(GL_TEXTURE0);
				}

				// render to secondary layer
				RenderTilesToFramebuffer( eRenderLayerSecondary, mSecondaryLayer.mFrameBuffer, mTileLayer.mFrameBuffer,
						renderOptions.GetWidth(), renderOptions.GetHeight(), samples2, coverageSamples2,
						renderOptions.GetScaleFactor(), true, renderOptions, pFBRenderOptions, bComps, stats );

				if (nullptr != mGPUFBScene->GetUberShaderPtr() )
				{
					mGPUFBScene->GetUberShaderPtr()->SetSoftParticles(0.0f);

					glActiveTexture(GL_TEXTURE18);
					glBindTexture(GL_TEXTURE_2D, 0 );
					glActiveTexture(GL_TEXTURE0);
				}
			}
		}
		
		renderOptions.SetRenderLayerId(eRenderLayerMain);

		// downsample supersampling if exist
		//mMainLayer.mFrameBuffer.BlitBIGToFinalColor( renderOptions.GetResampleFilter() );

		// output stats
		
		mRenderStats.mData.TotalOpaqueShaders->SetInt( stats.opaqueShaders );
		mRenderStats.mData.TotalTransparencyShaders->SetInt( stats.transparentShaders );
		mRenderStats.mData.TotalOpaqueModels->SetInt( stats.opaqueModels );
		mRenderStats.mData.TotalTransparentModels->SetInt( stats.transparentModels );
	}
	
	//
	// compositions

	if (false == renderForSelection)
	{
		//
		// let's process compositions which don't render to output and use texture source

		if (true == mCurrentProperties->ProcessCompositions() )
		{
			Compositions_EventBeforeRenderNotify();
			Compositions_RenderTextures();
		}

		//Compositions_FindCurrent( renderOptions.GetCamera() );
		// render current composition
		if (nullptr != mCompositionOutput)
		{
			((ObjectComposition*) mCompositionOutput)->SetRenderData(mCompositionDataExchange);
			((ObjectComposition*) mCompositionOutput)->Render();
		}
	}
	
	if (mVideoRendering)
	{
		RestoreFrameBuffer( &mFrameBufferInfo );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	RestoreFrameBuffer( &mFrameBufferInfo );
	glDisable(GL_MULTISAMPLE);

	mNeedToRender = false;

	CHECK_GL_ERROR_MOBU();

	if ( true == renderForSelection )
	{
		renderOptions.SetGoal(eRenderGoalSelectionId);
		renderOptions.SetPass(eShaderPassOpaque);
		renderOptions.SetRenderLayerAllIds();
		
		RenderingStats stats;
		mGPUFBScene->RenderSceneShaderGroups( renderOptions, pFBRenderOptions, stats );
	}
	else
	{
		// plot to screen
		renderOptions.SetGLViewport();
		glDepthMask( GL_FALSE );

		// draw a resulted rect
		glColor3d(1.0, 1.0, 1.0);
		glActiveTexture(GL_TEXTURE0);
	
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		PrepareOrthoView( (double)renderOptions.GetViewportWidth(), 
			(double)renderOptions.GetViewportHeight());

		//
		const int newWidth = renderOptions.GetWidth();
		const int newHeight = renderOptions.GetHeight();

		glUseProgram(0);
		glEnable(GL_TEXTURE_2D);
			
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);

		if (nullptr != mCompositionOutput)
		{
			glBindTexture(GL_TEXTURE_2D, ((ObjectComposition*) mCompositionOutput)->TextureOGLId);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, mMainLayer.mFrameBuffer.GetFinalColorObject());
		}
		drawQuad2d(0, 0, newWidth, newHeight);

		//glDisable(GL_BLEND);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDepthMask( GL_TRUE );

		//mGPUFBScene->PostRenderingShadows();
	}

    //
    // Pop GL states.
    //
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	if ( (false == renderForSelection)
		&& pFBRenderOptions->GetViewerOptions()->PickingMode() == kFBPickingModeStandard)
	{
		// DONE: let's render to depth buffer all our models without log depth
		//		that helps to occlude helper objects like (bones, cameras, etc.)

		// TODO: switch to Linear Depth here !! to better occulude mobu objects in log depth
		mGPUFBScene->SetLogarithmicDepth(false);

		renderOptions.SetGLViewport();
		glEnable(GL_DEPTH_TEST);

		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		mGPUFBScene->RenderSceneClassic(pFBRenderOptions);

		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

    glPopAttrib();
	CHECK_GL_ERROR_MOBU();

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

#ifdef _DEBUG
	systemTime = mSystem.SystemTime;
	kLongLong msEndTime = systemTime.GetMilliSeconds() - msBeginTime;
#endif

	if (true == mVideoRendering)
		FBTrace( "finish frame render\n" );
}

void MoRendererCallback::ComputeIfNeededPartlyUpdate()
{
	if (!!(GetAsyncKeyState(VK_F9) & 0x8000) )
	{
		mNeedToRender = true;
		mPartlyRenderIndex = 0;
	}
	/*
	else if (!!(GetAsyncKeyState(VK_MBUTTON) & 0x8000) || !!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		|| !!(GetAsyncKeyState(VK_RBUTTON) & 0x8000) )
	{
		mLastSystemTime = mSystem.SystemTime;
		mPartlyRenderIndex = 0;
	}
	*/
	FBRenderer *pRenderer = mSystem.Renderer;
	const int displayUpdateId = pRenderer->DisplaySetUpdateId;
	const int renderUpdateid = pRenderer->RendererUpdateId;

	if (displayUpdateId != mLastDisplayUpdateId || renderUpdateid != mLastRenderUpdateId)
	{
		// mGPUFBScene->Textures_InitOGL(nullptr);

		mNeedToRender = true;
		mPartlyRenderIndex = 0;

		mLastDisplayUpdateId = displayUpdateId;
		mLastRenderUpdateId = renderUpdateid;

		mLastSystemTime = mSystem.SystemTime;
	}
	else
	{
		FBTime localTime = mSystem.LocalTime;
		FBTime systemTime = mSystem.SystemTime;

		double timeToPartlyUpdate = 1.0;
		TimeToPartlyUpdate.GetData( &timeToPartlyUpdate, sizeof(double) );

		if (mLastSystemTime != FBTime::Infinity 
			&& (systemTime.GetSecondDouble() - mLastSystemTime.GetSecondDouble()) > timeToPartlyUpdate )
		{
			mNeedToRender = true;

			mPartlyRenderIndex += 1;

			if (mPartlyRenderIndex > 0)
				mLastSystemTime = FBTime::Infinity;
				
			else
			{
				mLastSystemTime = systemTime;
				mLastSystemTime.SetSecondDouble( mLastSystemTime.GetSecondDouble() + timeToPartlyUpdate );
			}
			
		}
		else
		if (localTime != mLastLocalTime)
		{
			// run a time;
			mLastSystemTime = systemTime;
			mLastLocalTime = localTime;
			mPartlyRenderIndex = 0;
		}
	}
}

void MoRendererCallback::RenderShadingToFramebuffer(const ERenderLayer layerId, MainFrameBuffer &framebuffer, bool moreColorAttachments,
		const int width, const int height, const int samples, const int coverageSamples,
		const double sscale, const bool plotMultisampling, CRenderOptions &options, FBRenderOptions *pFBRenderOptions,
		const bool prepCompositions, RenderingStats &stats)
{
	options.SetRenderLayerId(layerId);

	framebuffer.ReSize(width, height, sscale, samples, coverageSamples );
	
	framebuffer.BeginRender( moreColorAttachments );

	if (true == mVideoRendering)
		FBTrace( "   -- render background\n" );

	if (eRenderLayerMain == layerId && true == options.IsRenderToBackgroundLayer() )
	{
		mGPUFBScene->DrawBackground(true, true, width, height);

		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, mBackgroungLayer.mFrameBuffer.GetFinalColorObject() );
		glActiveTexture(GL_TEXTURE0);
	}
	else
	{
		mGPUFBScene->DrawBackground(true, true, width, height);
	}

	framebuffer.UpdateViewport();

	//
	//
	mGPUFBScene->RenderScene(layerId, options, pFBRenderOptions, true, stats);
	
	framebuffer.EndRender();
	
	if ( plotMultisampling )
	{
		if (true == mVideoRendering)
			FBTrace( "   -- prep for post processing\n" );

		framebuffer.PrepForPostProcessing(false);
	}
	
	if (eRenderLayerMain == layerId && true == options.IsRenderToBackgroundLayer() )
	{
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, 0 );
		glActiveTexture(GL_TEXTURE0);
	}

	if (true == mVideoRendering)
		FBTrace( "   -- finish render -- \n" );
		
	CHECK_GL_ERROR_MOBU();

}

void MoRendererCallback::RenderTilesToFramebuffer(const ERenderLayer layerId, MainFrameBuffer &mainbuffer, MainFrameBuffer &tilebuffer, 
		const int newWidth, const int newHeight, const int samples, const int  coverageSamples,
		const double sscale, const bool plotMultisampling, CRenderOptions &options, FBRenderOptions *pFBRenderOptions,
		const bool prepCompositions, RenderingStats &stats)
{
	if (true == mVideoRendering)
		FBTrace( " == begin to render tiles\n" );

	glEnable(GL_DEPTH_TEST);

	mainbuffer.ReSize(newWidth, newHeight, 1.0, samples, coverageSamples);

	if (prepCompositions)
	{
		/*
		if (updated)
		{
			Compositions_ChangeGlobalSettings();
		}
		Compositions_PrepDataExchange(mCompositionDataExchange, options, mainbuffer);
		*/
		Compositions_FindCurrent( options.GetCamera() );

		if (nullptr != mCompositionOutput)
		{
			if (true == mVideoRendering)
				FBTrace( "   - render to color buffers\n" );
			// render to normal and mask
			RenderShadingToFramebuffer( layerId, mainbuffer, true, newWidth, newHeight, samples,
				coverageSamples, 1.0, false, options, pFBRenderOptions, false, stats );

			if (true == mVideoRendering)
				FBTrace( "   - end render\n" );
		}
	}

	GLSLShader *pDownsampleShader = mainbuffer.GetDownsampleShaderPtr(options.GetResampleFilter());
	if (nullptr != pDownsampleShader)
	{
		const double tileWidth = 1.0 * (double) newWidth / (double) mTilesSW;
		const double tileHeight = 1.0 * (double) newHeight / (double) mTilesSH;

		pDownsampleShader->Bind();
		GLint loc = pDownsampleShader->findLocation( "texelSize" );
		if (loc >= 0)
			pDownsampleShader->setUniformVector2f( loc, 0.5f / (float) tileWidth, 0.5f / (float) tileHeight );
		pDownsampleShader->UnBind();

		//
		int tileIndex = 0;
		double offsetX = 0.0;
		double offsetY = 0.0;

		mGPUFBScene->PushCameraCache();

		for (int tilex=0; tilex<mTilesSW; ++tilex)
		{
			offsetY = 0.0;
			for (int tiley=0; tiley<mTilesSH; ++tiley)
			{
				// prep tile offset matrix
					
				double matTiling[16] = {
					(double) mTilesSW, 0.0, 0.0, 0.0,
					0.0, (double) mTilesSH, 0.0, 0.0,
						0.0, 0.0, 1.0, 0.0,
						(double)mTilesSW-1.0-2.0*(double)tilex,
						(double)mTilesSH-1.0-2.0*(double)tiley,
						0.0, 1.0 };
				
					const CGPUFBScene::CTilingInfo tilingInfo = { tilex*options.GetWidth(), tiley*options.GetHeight(), 
					mTilesSW * options.GetWidth(), mTilesSH * options.GetHeight(), matTiling };

				mGPUFBScene->PrepareCamera(options.GetCamera(), tilingInfo, false, 0, nullptr);
				glEnable(GL_DEPTH_TEST);

				if (true == mVideoRendering)
					FBTrace( "   - begin to render ONE tile\n" );

				RenderShadingToFramebuffer( layerId, tilebuffer, false,
					options.GetWidth(), options.GetHeight(), samples, coverageSamples,
					1.0, true, options, pFBRenderOptions, false, stats );
					
				if (true == mVideoRendering)
					FBTrace( "   - finish render\n" );

				glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

				// DONE: plot to main buffer
				PrepareOrthoView( (double)options.GetViewportWidth(), 
					(double)options.GetViewportHeight());

				glDisable(GL_DEPTH_TEST);
				mainbuffer.BeginRender(false); // render only to main color
				pDownsampleShader->Bind();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tilebuffer.GetFinalColorObject() );
				drawQuad2d( offsetX, offsetY, offsetX+tileWidth, offsetY+tileHeight );

				pDownsampleShader->UnBind();
				mainbuffer.EndRender();
				CHECK_GL_ERROR_MOBU();
				
				if (true == mVideoRendering)
					FBTrace( "   - finish plot tile\n" );

				tileIndex += 1;
				offsetY += tileHeight;
			}
			offsetX += tileWidth;
		}

		//mGPUFBScene->PrepareCamera(options.GetCamera(), nullptr, false, 0, nullptr);
		mGPUFBScene->PopCameraCache();

		if (samples > 1)
			mainbuffer.PrepForPostProcessing(false);
	}

	if (true == mVideoRendering)
		FBTrace( " == finish render tiles\n" );
}
/*
void MoRendererCallback::RenderToBackground(CRenderOptions &op)
{	
	mGPUFBScene->PrepBackground(false, 1, op.GetWidth(), op.GetHeight());
}

void MoRendererCallback::RenderToTransparency(CRenderOptions &options)
{
}
*/
void MoRendererCallback::SetRenderSamples(HIObject object, int value)
{
	MoRendererCallback *pGlobal = FBCast<MoRendererCallback>(object);
	if (pGlobal)
	{
		int clampValue = value;
		if (clampValue < 0) clampValue = 0;
		if (clampValue > 64) clampValue = 64;

		// we need POT
		if (clampValue > 2)
		{
			int pot = 1;

			while (pot < clampValue)
			{
				pot = pot * 2;
			}

			clampValue = pot;
		}

		//pGlobal->RenderSamples.SetPropertyValue(clampValue);
	}
}

void MoRendererCallback::SetDepthSamples(HIObject object, int value)
{
	MoRendererCallback *pGlobal = FBCast<MoRendererCallback>(object);
	if (pGlobal)
	{
		int clampValue = value;
		if (clampValue < 0) clampValue = 0;
		if (clampValue > 64) clampValue = 64;

		// we need POT
		if (clampValue > 2)
		{
			int pot = 1;

			while (pot < clampValue)
			{
				pot = pot * 2;
			}

			clampValue = pot;
		}

		//pGlobal->DepthSamples.SetPropertyValue(clampValue);
	}
}

void MoRendererCallback::SetCoverageSamples(HIObject object, int value)
{
	MoRendererCallback *pGlobal = FBCast<MoRendererCallback>(object);
	if (pGlobal)
	{
		int clampValue = value;
		if (clampValue < 0) clampValue = 0;
		if (clampValue > 64) clampValue = 64;

		// we need POT
		if (clampValue > 2)
		{
			int pot = 1;

			while (pot < clampValue)
			{
				pot = pot * 2;
			}

			clampValue = pot;
		}

		//pGlobal->CoverageSamples.SetPropertyValue(clampValue);
	}
}

void MoRendererCallback::SetRenderDepthSamples(HIObject object, int value)
{
	MoRendererCallback *pGlobal = FBCast<MoRendererCallback>(object);
	if (pGlobal)
	{
		int clampValue = value;
		if (clampValue < 0) clampValue = 0;
		if (clampValue > 64) clampValue = 64;

		// we need POT
		if (clampValue > 2)
		{
			int pot = 1;

			while (pot < clampValue)
			{
				pot = pot * 2;
			}

			clampValue = pot;
		}

		//pGlobal->RenderDepthSamples.SetPropertyValue(clampValue);
	}
}

void MoRendererCallback::SetRenderCoverageSamples(HIObject object, int value)
{
	MoRendererCallback *pGlobal = FBCast<MoRendererCallback>(object);
	if (pGlobal)
	{
		int clampValue = value;
		if (clampValue < 0) clampValue = 0;
		if (clampValue > 64) clampValue = 64;

		// we need POT
		if (clampValue > 2)
		{
			int pot = 1;

			while (pot < clampValue)
			{
				pot = pot * 2;
			}

			clampValue = pot;
		}

		//pGlobal->RenderCoverageSamples.SetPropertyValue(clampValue);
	}
}

void MoRendererCallback::SetReset(HIObject object, bool value)
{
	MoRendererCallback *pGlobal = FBCast<MoRendererCallback>(object);
	if (pGlobal && value)
	{
		pGlobal->ResetToDefault();
	}
}

void MoRendererCallback::OnTestActionSet(HIObject object, bool value)
{
	MoRendererCallback *pGlobal = FBCast<MoRendererCallback>(object);
	if (pGlobal && value)
	{
		pGlobal->RunTestAction();
	}
}

void MoRendererCallback::SetRenderUpdateAction(HIObject object, bool value)
{
	MoRendererCallback *pGlobal = FBCast<MoRendererCallback>(object);
	if (pGlobal && value)
	{
		pGlobal->AskToRenderAFrame();
	}
}

void MoRendererCallback::SetResolution(HIObject object, ERendererEnum value)
{
	MoRendererCallback *pBase = FBCast<MoRendererCallback>(object);
	if (pBase) 
	{
		int ivalue = (int) value;

		pBase->ApplyResolutionPreset(ivalue);
		pBase->mGlobalResolution.mData.RenderResolution->SetPropertyValue( value );
	}
}

void MoRendererCallback::SetQualityPreset(HIObject object, EGraphicsQualityPresets	value)
{
	MoRendererCallback *pBase = FBCast<MoRendererCallback>(object);
	if (pBase) 
	{
		//pBase->ApplyQualityPreset(value);
		//pBase->QualityPreset.SetPropertyValue( value );
	}
}

void MoRendererCallback::SetRenderQualityPreset(HIObject object, EGraphicsQualityPresets	value)
{
	MoRendererCallback *pBase = FBCast<MoRendererCallback>(object);
	if (pBase) 
	{
		//pBase->ApplyRenderQualityPreset(value);
		//pBase->RenderQualityPreset.SetPropertyValue( value );
	}
}

void MoRendererCallback::ApplyQualityPreset(CProperties::CQuality &quality, const EGraphicsQualityPresets value, bool refresh)
{
	//CProperties::CQuality &quality = mCurrentProperties->mQuality;
	double dvalue = 100.0;

	if (value != eGraphicsQualityCustom)
	{
		quality.UseSuperSamplingTiles->SetInt(0);
		quality.TilesHor->SetInt(1);
		quality.TilesVer->SetInt(1);

		quality.SupersampleScale->SetData(&dvalue);
		quality.DepthSamples->SetInt(0);
		quality.CoverageSamples->SetInt(0);
		quality.DownsampleType->SetInt( (int) eResamplingFilterLanczos );
	}

	switch(value)
	{
	case eGraphicsQualityMSAA_2x:
		quality.DepthSamples->SetInt(2);
		break;
	case eGraphicsQualityMSAA_4x:
		quality.DepthSamples->SetInt(4);
		break;
	case eGraphicsQualityMSAA_16x:
		quality.DepthSamples->SetInt(16);
		break;
	case eGraphicsQualityCSAA_4x:
		quality.DepthSamples->SetInt(4);
		quality.CoverageSamples->SetInt(4);
		break;
	case eGraphicsQualityCSAA_16x:
		quality.DepthSamples->SetInt(4);
		quality.CoverageSamples->SetInt(16);
		break;
	case eGraphicsQualitySupersampling_2x2:
		quality.DepthSamples->SetInt(16);
		quality.CoverageSamples->SetInt(0);

		quality.UseSuperSamplingTiles->SetInt(1);
		quality.TilesHor->SetInt(2);
		quality.TilesVer->SetInt(2);
		break;
	case eGraphicsQualitySupersampling_4x4:
		quality.DepthSamples->SetInt(16);
		quality.CoverageSamples->SetInt(0);

		quality.UseSuperSamplingTiles->SetInt(1);
		quality.TilesHor->SetInt(4);
		quality.TilesVer->SetInt(4);
		break;
	case eGraphicsQualitySupersampling_6x6:
		quality.DepthSamples->SetInt(16);
		quality.CoverageSamples->SetInt(0);

		quality.UseSuperSamplingTiles->SetInt(1);
		quality.TilesHor->SetInt(6);
		quality.TilesVer->SetInt(6);
		break;
	case eGraphicsQualitySupersampling_8x8:
		quality.DepthSamples->SetInt(16);
		quality.CoverageSamples->SetInt(0);

		quality.UseSuperSamplingTiles->SetInt(1);
		quality.TilesHor->SetInt(8);
		quality.TilesVer->SetInt(8);
		break;
	}

	if (true == refresh)
	{
		mNeedToRender = true;
	}
}

void MoRendererCallback::ApplyQualityPreset2(CProperties::CQuality &quality, const EGraphicsQualityPresets value, bool refresh)
{
	//CProperties::CQuality &quality = mCurrentProperties->mQuality;
	double dvalue = 100.0;

	if (value != eGraphicsQualityCustom)
	{
		quality.SupersampleScale2->SetData(&dvalue);
		quality.DepthSamples2->SetInt(0);
		quality.CoverageSamples2->SetInt(0);
		quality.DownsampleType2->SetInt( (int) eResamplingFilterLanczos );
	}

	switch(value)
	{
	case eGraphicsQualityMSAA_2x:
		quality.DepthSamples2->SetInt(2);
		break;
	case eGraphicsQualityMSAA_4x:
		quality.DepthSamples2->SetInt(4);
		break;
	case eGraphicsQualityMSAA_16x:
		quality.DepthSamples2->SetInt(16);
		break;
	case eGraphicsQualityCSAA_4x:
		quality.DepthSamples2->SetInt(4);
		quality.CoverageSamples2->SetInt(4);
		break;
	case eGraphicsQualityCSAA_16x:
		quality.DepthSamples2->SetInt(4);
		quality.CoverageSamples2->SetInt(16);
		break;
	case eGraphicsQualitySupersampling_2x2:
		quality.DepthSamples2->SetInt(16);
		quality.CoverageSamples2->SetInt(0);
		break;
	case eGraphicsQualitySupersampling_4x4:
		quality.DepthSamples2->SetInt(16);
		quality.CoverageSamples2->SetInt(0);
		break;
	case eGraphicsQualitySupersampling_6x6:
		quality.DepthSamples2->SetInt(16);
		quality.CoverageSamples2->SetInt(0);
		break;
	case eGraphicsQualitySupersampling_8x8:
		quality.DepthSamples2->SetInt(16);
		quality.CoverageSamples2->SetInt(0);
		break;
	}

	if (true == refresh)
	{
		mNeedToRender = true;
	}
}

void MoRendererCallback::ApplyResolutionPreset(const int index)
{
	if (index >= 0 && index < (int)mPresets.size() )
	{
		PresetItem &currPreset = mPresets[index];

		if (currPreset.width > 0 && currPreset.height > 0)
		{
			mGlobalResolution.mData.CustomWidth->SetInt(currPreset.width);
			mGlobalResolution.mData.CustomHeight->SetInt(currPreset.height);
		}
	}
}

void MoRendererCallback::LoadResolutionPresets()
{
	// TODO: load a xml file and fillup the resolution preset property

	FBString effectPath, effectFullName;
	if (true == FindEffectLocation( "\\System\\resolutionPresets.xml", effectPath, effectFullName ) )
	{
		TiXmlDocument	doc;

		TiXmlNode *node = nullptr;
		TiXmlElement *presetsElem = nullptr;
		TiXmlElement *elem = nullptr;
		TiXmlAttribute  *attrib = nullptr;

		std::string defaultName;
		std::string	name;
		int width=0;
		int height=0;

		try
		{

			if (doc.LoadFile( effectFullName ) == false)
				throw std::exception( "failed to load a file" );
	
			// get main header attributes
			node = doc.FirstChild( "Presets" );
	
			if (node)
			{
			
				presetsElem = node->ToElement();

				if (presetsElem)
				{

					for( attrib = presetsElem->FirstAttribute();
								attrib;
								attrib = attrib->Next() )
					{
						if ( strcmp(attrib->Name(), "Default") == 0 )
						{
							defaultName = attrib->Value();
						}
					}

					elem = presetsElem->FirstChildElement("preset");

					while (elem)
					{
						name.clear();

						// enumerate attribs
						for( attrib = elem->FirstAttribute();
								attrib;
								attrib = attrib->Next() )
						{
							if ( strcmp(attrib->Name(), "Name") == 0 )
							{
								name = attrib->Value();
							}
							else if (strcmp(attrib->Name(), "Width") == 0)
							{
								width = attrib->IntValue();
							}
							else if (strcmp(attrib->Name(), "Height") == 0)
							{
								height = attrib->IntValue();
							}
						}

						// add a new preset
						if (name.size() > 0)
						{
							PresetItem newPreset;

							memset(newPreset.name, 0, sizeof(char) * 128 );
							strcpy_s(newPreset.name, 128, name.c_str() );

							newPreset.width = width;
							newPreset.height = height;

							mPresets.push_back( newPreset );

							if (defaultName.size() > 0 && strcmp(defaultName.c_str(), name.c_str())==0)
							{
								mDefaultPreset = ((int)mPresets.size()) - 1;
							}
						}

						elem = elem->NextSiblingElement();
					}
				}
			}
		}
		catch (std::exception &e)
		{
			printf( "[ERROR] while loading resolution presets file - %s\n", e.what() );
		}
	
	}

}

void MoRendererCallback::DynamicMasks_ChangeContext()
{
	// change context for dynamic masks
	for (auto iter=begin(mDynamicMasksVector); iter!=end(mDynamicMasksVector); ++iter)
	{
		ObjectMask *pMask = (ObjectMask*) *iter;
		pMask->ChangeContext();
	}
}

void MoRendererCallback::DynamicMasks_Render()
{
	for (auto iter=begin(mDynamicMasksVector); iter!=end(mDynamicMasksVector); ++iter)
	{
		ObjectMask *pMask = (ObjectMask*) *iter;
		pMask->Render();
	}
}

void MoRendererCallback::Compositions_ChangeGlobalSettings()
{
	for (auto iter=begin(mCompositionsVector); iter!=end(mCompositionsVector); ++iter)
	{
		ObjectComposition *pFinal = (ObjectComposition*) *iter;
		pFinal->ChangeGlobalSettings();
	}
}

void MoRendererCallback::Compositions_ChangeContext()
{
	for (auto iter=begin(mCompositionsVector); iter!=end(mCompositionsVector); ++iter)
	{
		ObjectComposition *pFinal = (ObjectComposition*) *iter;
		pFinal->ChangeContext();
	}
}

void MoRendererCallback::Compositions_PrepDataExchange(CompositeFinalRenderDataExchange &data,
	const CRenderOptions &renderOptions, const MainFrameBuffer &frameBuffer)
{
	
	data.showAsPreview = false;
	if (mVideoRendering == false)
		data.showAsPreview = mCompositionOptions.ShowAsPreview();

	if (mCompositionOptions.PreviewOnPlay() )
	{
		FBPlayerControl &player = FBPlayerControl::TheOne();

		if (player.IsPlaying || player.IsRecording)
			mCompositionDataExchange.showAsPreview = true;
	}
	if (mCompositionOptions.PreviewOnHoldingAlt() )
	{
		if(GetAsyncKeyState(VK_LMENU) & 0x8000 || GetAsyncKeyState(VK_RMENU) & 0x8000)
			mCompositionDataExchange.showAsPreview = true;	
	}

	mCompositionDataExchange.w = renderOptions.GetWidth();
	mCompositionDataExchange.h = renderOptions.GetHeight();
		
	mCompositionDataExchange.borderX = renderOptions.GetX();
	mCompositionDataExchange.borderY = renderOptions.GetY();
	
	FBCamera *pCurrCamera = renderOptions.GetCamera();
	mCompositionDataExchange.pCamera = pCurrCamera;
	mCompositionDataExchange.pCameraInterest = pCurrCamera->Interest;
	mCompositionDataExchange.nearPlane = mGPUFBScene->GetCameraCache().nearPlane;
	mCompositionDataExchange.farPlane = mGPUFBScene->GetCameraCache().farPlane;

	if (mVideoRendering == true)
		mCompositionDataExchange.useDOF = mCameraInternalDOF;
	else
		mCompositionDataExchange.useDOF = pCurrCamera->UseDepthOfField;
	mCompositionDataExchange.focusAngle = pCurrCamera->FocusAngle;
	mCompositionDataExchange.focusDistanceSource = pCurrCamera->FocusDistanceSource;
	mCompositionDataExchange.focusSpecificDistance = pCurrCamera->FocusSpecificDistance;

	mCompositionDataExchange.ms = (frameBuffer.getNumberOfDepthSamples() > 1);
	mCompositionDataExchange.samples = frameBuffer.getNumberOfDepthSamples();
	/*
	mCompositionDataExchange.colorId = frameBuffer.GetFinalColorObject();
	mCompositionDataExchange.depthId = frameBuffer.GetFinalDepthObject();
	mCompositionDataExchange.normalId = frameBuffer.GetFinalNormalObject();
	mCompositionDataExchange.maskId = frameBuffer.GetFinalMaskObject();
	mCompositionDataExchange.positionId = frameBuffer.GetFinalPositionObject();

	mCompositionDataExchange.colorMSId = frameBuffer.GetColorObjectMS();
	mCompositionDataExchange.depthMSId = frameBuffer.GetDepthObjectMS();
	mCompositionDataExchange.normalMSId = frameBuffer.GetNormalObjectMS();
	mCompositionDataExchange.maskMSId = frameBuffer.GetMaskObjectMS();
	mCompositionDataExchange.positionMSId = frameBuffer.GetPositionObjectMS();
	*/

	mCompositionDataExchange.frameBuffers[eRenderLayerBackground] = &mBackgroungLayer.mFrameBuffer;
	mCompositionDataExchange.frameBuffers[eRenderLayerMain] = &mMainLayer.mFrameBuffer;
	mCompositionDataExchange.frameBuffers[eRenderLayerSecondary] = &mSecondaryLayer.mFrameBuffer;

	mCompositionDataExchange.logDepth = mCurrentProperties->IsLogarithmicDepth();
	mCompositionDataExchange.nvDepth = false;
}

void MoRendererCallback::Compositions_EventBeforeRenderNotify()
{
	const bool renderOnlyCurrent = mCompositionOptions.RenderOnlyCurrent();

	for (auto iter=begin(mCompositionsVector); iter!=end(mCompositionsVector); ++iter)
	{
		ObjectComposition *pFinal = (ObjectComposition*) *iter;

		if (pFinal->Active)
		{
			// TODO: check this options ?!
			//		pFinal->Cameras.GetCount() > 0
			if (renderOnlyCurrent && pFinal->OutputToRender && pFinal != mCompositionOutput)
				continue;

			pFinal->EventBeforeRenderNotify();
		}
	} // end for each composition
}

void MoRendererCallback::Compositions_Render()
{
	const bool renderOnlyCurrent = mCompositionOptions.RenderOnlyCurrent();

	for (auto iter=begin(mCompositionsVector); iter!=end(mCompositionsVector); ++iter)
	{
		ObjectComposition *pFinal = (ObjectComposition*) *iter;

		if (pFinal->Active)
		{
			// TODO: check this options ?!
			//		pFinal->Cameras.GetCount() > 0
			if (renderOnlyCurrent && pFinal->OutputToRender && pFinal != mCompositionOutput)
				continue;

			pFinal->SetRenderData(mCompositionDataExchange);
			pFinal->Render();
		}
	} // end for each composition
}

void MoRendererCallback::Compositions_RenderTextures()
{
	//const bool renderOnlyCurrent = mCompositionOptions.RenderOnlyCurrent();

	for (auto iter=begin(mCompositionsVector); iter!=end(mCompositionsVector); ++iter)
	{
		ObjectComposition *pFinal = (ObjectComposition*) *iter;

		if (pFinal->Active)
		{
			// TODO: check this options ?!
			//		pFinal->Cameras.GetCount() > 0
			if (pFinal->OutputToRender)
				continue;

			pFinal->SetRenderData(mCompositionDataExchange);
			pFinal->Render();
		}
	} // end for each composition
}

bool MoRendererCallback::Compositions_FindCurrent(FBCamera *pCurrentCamera)
{
	mCompositionOutput = nullptr;

	if (mCurrentProperties->ProcessCompositions() )
	{
		if (pCurrentCamera && FBIS(pCurrentCamera, FBCameraSwitcher) )
			pCurrentCamera = ((FBCameraSwitcher*)pCurrentCamera)->CurrentCamera;
		
		if (pCurrentCamera == nullptr)
			return false;

		//
		// 1 - find if there is override by camera

		for (auto iter=begin(mCompositionsVector); iter!=end(mCompositionsVector); ++iter)
		{
			if (nullptr != mCompositionOutput)
				break;

			ObjectComposition *pFinal = (ObjectComposition*) *iter;
			if (pFinal->Active)
			{
				if (pFinal->Cameras.GetCount() > 0 && true == pFinal->OutputToRender)
				{
					for (int i=0, count=pFinal->Cameras.GetCount(); i<count; ++i)
					{
						FBCamera *pCamera = (FBCamera*) pFinal->Cameras.GetAt(i);
						if (pCamera == pCurrentCamera)
						{
							mCompositionOutput = pFinal;
							break;
						}
					}
				}
			}
		} // end for each composition

		//
		// 2 - check for any first active output to render
		if (nullptr == mCompositionOutput)
		{
			for (auto iter=begin(mCompositionsVector); iter!=end(mCompositionsVector); ++iter)
			{
				ObjectComposition *pFinal = (ObjectComposition*) *iter;
				if (pFinal->Active && true == pFinal->OutputToRender)
				{		
					mCompositionOutput = pFinal;
					break;
				}
			} // end for each composition
		}
	}

	if (mCompositionOutput == nullptr)
	{
		if (CurrentComposition.GetCount() > 0)
			CurrentComposition.Clear();
	}
	else
	{
		if (CurrentComposition.GetCount() > 0)
			CurrentComposition.SetAt(0, mCompositionOutput);
		else
			CurrentComposition.Add(mCompositionOutput);
	}

	return true;
}


bool MoRendererCallback::IsCameraReadyToSetResolution(FBCamera *pCamera, bool excludeFacialCameras)
{
	if (pCamera->SystemCamera)
		return false;

	// exclude facial connected cameras
	if (excludeFacialCameras)
	{
		for (int i=0, count=pCamera->GetDstCount(); i<count; ++i)
		{
			FBPlug *pPlug = pCamera->GetDst(i);
			if ( FBIS(pPlug, FBConstraint) || FBIS(pPlug, FBShader) || FBIS(pPlug, FBUserObject) )
				return false;
		}
	}

	return true;
}

void MoRendererCallback::UpdateCameraResolution(FBCamera *useSpecifiedCamera)
{
	if (0 == mGlobalResolution.mData.UseGlobalResolution->AsInt())
		return;

	mUpdatingCameraProperties = true;
	mUpdatingCamera = useSpecifiedCamera;

	const int width = mGlobalResolution.mData.CustomWidth->AsInt();
	const int height = mGlobalResolution.mData.CustomHeight->AsInt();
	const bool excludeFacialCameras = (mGlobalResolution.mData.ExcludeProjectorCameras->AsInt() > 0);

	if (useSpecifiedCamera != nullptr)
	{
		if (IsCameraReadyToSetResolution(useSpecifiedCamera, excludeFacialCameras) )
		{
			useSpecifiedCamera->FrameSizeMode = kFBFrameSizeFixedResolution;
			useSpecifiedCamera->ResolutionWidth = width;
			useSpecifiedCamera->ResolutionHeight = height;
			useSpecifiedCamera->ResolutionMode = kFBResolutionCustom;
		}
	}
	else
	{
		FBScene *pScene = mSystem.Scene;

		for (int i=0, count=pScene->Cameras.GetCount(); i<count; ++i)
		{
			FBCamera *pCamera = pScene->Cameras[i];

			if (false == IsCameraReadyToSetResolution(pCamera, excludeFacialCameras) )
				continue;

			pCamera->FrameSizeMode = kFBFrameSizeFixedResolution;
			pCamera->ResolutionWidth = width;
			pCamera->ResolutionHeight = height;
			pCamera->ResolutionMode = kFBResolutionCustom;
		}
	}

	//SetClean();

	mUpdatingCameraProperties = false;
	mUpdatingCamera = nullptr;
}

void MoRendererCallback::UpdateConnectedDecalFilters()
{
	FBScene *pScene = mSystem.Scene;

	for (int i=0, count=pScene->UserObjects.GetCount(); i<count; ++i)
	{
		if (FBIS(pScene->UserObjects[i], ObjectFilter3dDecal) )
		{
			ObjectFilter3dDecal *pDecal = (ObjectFilter3dDecal*) pScene->UserObjects[i];

			if (true == pDecal->UseGlobalResolution)
			{
				pDecal->CustomWidth.SetInt( mGlobalResolution.mData.CustomWidth->AsInt() );
				pDecal->CustomHeight.SetInt( mGlobalResolution.mData.CustomHeight->AsInt() );
			}
		}
	}
}