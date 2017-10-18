
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ProjTex_shader.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Class declaration
#include "ProjTex_shader.h"
#include "ShaderModelInfo.h"
#include <math.h>

#include "FX_shader.h"

#include "IO\FileUtils.h"

#include "shared_content.h"
#include "graphics\OGL_Utils.h"
#include "graphics\OGL_Utils_mobu.h"
#include "graphics\particlesDrawHelper.h"
#include "algorithm\math3d_mobu.h"

#include "MB_renderer.h"

//--- Registration defines
#define PROJTEX__CLASS	PROJTEX__CLASSNAME

const char * FBPropertyBaseEnum<EShadingType>::mStrings[] = {"Phong", "Flat", "Toon", "Matte", "MatCap", 0};
const char * FBPropertyBaseEnum<ERenderLayer>::mStrings[] = {"Background", "Main", "Secondary", 0};

//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	PROJTEX__CLASS	);
FBRegisterShader	(PROJTEX__DESCSTR,	    // Unique name
                     PROJTEX__CLASS,		// Class
                     PROJTEX__DESCSTR,	    // Short description
                     PROJTEX__DESC,		    // Long description
                     FB_DEFAULT_SDK_ICON	);		// Icon filename (default=Open Reality icon)


#define concatString(str, idx) str #idx

#ifdef _DEBUG
	extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);
#endif

static void ProjTex_ReloadShader(HIObject object, bool pValue)
{
	ProjTexShader *shader = FBCast<ProjTexShader>(object);
	if (shader && pValue) 
	{
		shader->DoReloadShader();
	}
}
/*
static void ProjTex_ManualUpdate(HIObject object, bool pValue)
{
	ProjTexShader *shader = FBCast<ProjTexShader>(object);
	if (shader && pValue) 
	{
		shader->DoManualUpdate();
	}
}
*/
/*
static void ProjTex_SetLogarithmicDepth(HIObject object, bool pValue)
{
	ProjTexShader *shader = FBCast<ProjTexShader>(object);
	if (shader) 
	{
		shader->SetLogarithmicDepth(pValue);
		shader->LogarithmicDepth.SetPropertyValue(pValue);
	}
}
*/
static void ProjTex_SetRenderOnBack(HIObject object, bool pValue)
{
	ProjTexShader *shader = FBCast<ProjTexShader>(object);
	if (shader) 
	{
		if (pValue)
			shader->RenderLayer = eRenderLayerBackground;

		/*
		if (pValue)
			shader->RenderingPass = kFBPassPreRender;
		else
		{
			auto tr = shader->Transparency;
			shader->Transparency = kFBAlphaSourceTransluscentAlpha;
			shader->Transparency = tr;
		}

		shader->InvalidateShaderVersion();

		shader->RenderOnBack.SetPropertyValue(pValue);
		*/
	}
}

static bool ProjTex_GetRenderOnBack(HIObject object)
{
	ProjTexShader *shader = FBCast<ProjTexShader>(object);
	if (shader) 
	{
		return (eRenderLayerBackground == shader->RenderLayer);
	}
	return false;
}

void AddPropertyViewForProjTex(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(PROJTEX__CLASSSTR, pPropertyName, pHierarchy);
}
 
void ProjTexShader::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForProjTex( "Enable", "" );
	AddPropertyViewForProjTex( "Shading", "" );
	AddPropertyViewForProjTex( "Accurate Depth", "" );
	AddPropertyViewForProjTex( "Reload Shader", "" );
	//
	AddPropertyViewForProjTex( "Lights Setup", "", true );
	AddPropertyViewForProjTex( "AffectingLights", "Lights Setup" );
	AddPropertyViewForProjTex( "Use scene lights", "Lights Setup" );
	//
	AddPropertyViewForProjTex( "Transparency Setup", "", true );
	AddPropertyViewForProjTex( "Transparency", "Transparency Setup" );
	AddPropertyViewForProjTex( "TransparencyFactor", "Transparency Setup" );
	AddPropertyViewForProjTex( "Inverse Transparency Factor", "Transparency Setup" );
	//
	AddPropertyViewForProjTex( "Toon Setup", "", true );
	AddPropertyViewForProjTex( "Toon Steps", "Toon Setup" );
	AddPropertyViewForProjTex( "Toon Distribution", "Toon Setup" );
	AddPropertyViewForProjTex( "Toon Shadow Position", "Toon Setup" );
	//
	AddPropertyViewForProjTex( "Color Correction Setup", "", true );
	AddPropertyViewForProjTex( "Brightness", "Color Correction Setup" );
	AddPropertyViewForProjTex( "Saturation", "Color Correction Setup" );
	AddPropertyViewForProjTex( "Contrast", "Color Correction Setup" );
	AddPropertyViewForProjTex( "Gamma", "Color Correction Setup" );

	AddPropertyViewForProjTex( "Custom color", "Color Correction Setup" );
	AddPropertyViewForProjTex( "Custom color mode", "Color Correction Setup" );
	//
	AddPropertyViewForProjTex( "Detail Blend Setup", "", true );
	AddPropertyViewForProjTex( "Detail opacity", "Detail Blend Setup" );
	AddPropertyViewForProjTex( "Detail blend mode", "Detail Blend Setup" );
	AddPropertyViewForProjTex( "Detail masked", "Detail Blend Setup" );
	//
	AddPropertyViewForProjTex( "Polygon Offset Setup", "", true );
	AddPropertyViewForProjTex( "Use polygon offset", "Polygon Offset Setup" );
	AddPropertyViewForProjTex( "Poly offset factor", "Polygon Offset Setup" );
	AddPropertyViewForProjTex( "Poly offset units", "Polygon Offset Setup" );
	//
	// DONE: add projection properties to the property view
	AddPropertyViewForProjTex( "Projectors Setup", "", true );
	char buffer[128];
	for (int i=0; i<MAX_PROJECTORS_COUNT; ++i)
	{
		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector%d", i+1 );
		AddPropertyViewForProjTex( buffer, "Projectors Setup" );

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Select%d", i+1 );
		AddPropertyViewForProjTex( buffer, "Projectors Setup" );

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Texture%d", i+1 );
		AddPropertyViewForProjTex( buffer, "Projectors Setup" );

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Texture Select%d", i+1 );
		AddPropertyViewForProjTex( buffer, "Projectors Setup" );

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Usage%d", i+1 );
		AddPropertyViewForProjTex( buffer, "Projectors Setup" );

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Keep Aspect%d", i+1 );
		AddPropertyViewForProjTex( buffer, "Projectors Setup" );

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Blend Mode%d", i+1 );
		AddPropertyViewForProjTex( buffer, "Projectors Setup" );

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Blend Opacity%d", i+1 );
		AddPropertyViewForProjTex( buffer, "Projectors Setup" );

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Mask%d", i+1 );
		AddPropertyViewForProjTex( buffer, "Projectors Setup" );

		memset(buffer, 0, sizeof(char)*128);
		sprintf( buffer, "Projector Mask Channel%d", i+1 );
		AddPropertyViewForProjTex( buffer, "Projectors Setup" );
	}
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ProjTexShader::FBCreate()
{
	//mShaderSuccess = true;
    //mpLightShaderRefCount++;
	mGPUFBScene = &CGPUFBScene::instance();
	mLastUniqueFrameId = 0;
	mNeedToCheckColorCorrection = true;
	mApplyColorCorrection = false;
	mNeedToUpdateProjectors = true;
	//mUberShader = nullptr;
	//mLastModel = nullptr;
	//mLastMaterial = nullptr;

	//mNeedUpdateLightsList = true;

	//mProjectorsManager = new CGPUProjectorsManager();

    FBPropertyPublish(	this, AffectingLights,	"AffectingLights",		NULL, NULL);
	FBPropertyPublish(	this, UseSceneLights,	"Use scene lights",		NULL, NULL);
    AffectingLights.SetFilter(FBLight::GetInternalClassId());
    AffectingLights.SetSingleConnect(false);
	UseSceneLights = true;
	
    FBPropertyPublish(	this, Transparency,	"Transparency",		NULL, SetTransparencyProperty);
    Transparency = kFBAlphaSourceNoAlpha;
    RenderingPass = GetRenderingPassNeededForAlpha(Transparency);

	FBPropertyPublish(	this, TransparencyFactor,	"TransparencyFactor",NULL, NULL);
	TransparencyFactor.SetMinMax(0.0, 1.0);
	TransparencyFactor = 1.0;

	FBPropertyPublish(	this, InverseTransparencyFactor,	"Inverse Transparency Factor",NULL, NULL);
	InverseTransparencyFactor = true;
	
	FBPropertyPublish(	this, UseAlphaFromColor,	"Use Alpha From Color",NULL, NULL);
	UseAlphaFromColor = false;
	
	/*
	FBPropertyPublish( this, LogarithmicDepth, "Accurate Depth", nullptr, ProjTex_SetLogarithmicDepth );
	LogarithmicDepth = false;
	*/
	FBPropertyPublish( this, DepthDisplacement, "Depth Displacement", nullptr, nullptr );
	DepthDisplacement = 0.0;
	//DepthDisplacement.SetMinMax( -100.0, 100.0, true, true );

	FBPropertyPublish(	this, RenderLayer,	"Render Layer", nullptr, nullptr);
	RenderLayer = eRenderLayerMain;

	FBPropertyPublish(	this, RenderOnBack,	"Render On Back", ProjTex_GetRenderOnBack, ProjTex_SetRenderOnBack);
	RenderOnBack = false;

	FBPropertyPublish(	this, ShadingType,	"Shading",NULL, NULL);
	ShadingType = eShadingTypeDynamic;

	FBPropertyPublish( this, ReloadShader,	"Reload Shader", nullptr, ProjTex_ReloadShader);

	FBPropertyPublish(	this, ToonSteps, "Toon Steps" ,NULL, NULL);
	ToonSteps = 4.0;

	FBPropertyPublish(	this, ToonDistribution, "Toon Distribution" ,NULL, NULL);
	ToonDistribution = 1.0;
	ToonDistribution.SetMinMax(0.1, 3.0, true, true);

	FBPropertyPublish(	this, ToonShadowPosition, "Toon Shadow Position" ,NULL, NULL);
	ToonShadowPosition = 0.5;
	ToonShadowPosition.SetMinMax(0.1, 1.0, true, true);

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
	Gamma = 1.0/2.2*100.0;
	Gamma.SetMinMax(-300.0, 300.0, true, true);

	// rim lighting

	FBPropertyPublish(	this, UseRim, "Use Rim Lighting" ,NULL, NULL);
	FBPropertyPublish(	this, RimPower, "Rim Power" ,NULL, NULL);
	FBPropertyPublish(	this, RimColor, "Rim Color" ,NULL, NULL);
	FBPropertyPublish(	this, RimTexture, "Rim Texture" ,NULL, NULL);
	FBPropertyPublish(	this, RimBlend, "Rim Blend" ,NULL, NULL);

	FBPropertyPublish(	this, MatCap, "MatCap Texture" ,NULL, NULL);

	UseRim.SetMinMax(0.0, 100.0);
	UseRim = 0.0;
	RimPower.SetMinMax(0.0, 200.0);
	RimPower = 100.0;
	RimColor = FBColor(1.0, 1.0, 1.0);
	RimBlend = eBlendAdd;

	RimTexture.SetSingleConnect(true);
	RimTexture.SetFilter( FBTexture::GetInternalClassId() );

	MatCap.SetSingleConnect(true);
	MatCap.SetFilter( FBTexture::GetInternalClassId() );

	// custom color

	FBPropertyPublish(this, CustomColor, "Custom color" ,NULL, NULL );
	FBPropertyPublish( this, CustomColorMode, "Custom color mode", NULL, NULL );

	CustomColor = FBColor(1.0, 1.0, 1.0);
	CustomColorMode = eBlendMultiply;
	
	// detail blending

	FBPropertyPublish(this, DetailOpacity, "Detail opacity" ,NULL, NULL );
	FBPropertyPublish( this, DetailBlendMode, "Detail blend mode", NULL, NULL );
	FBPropertyPublish( this, DetailMasked, "Detail masked", NULL, NULL );

	DetailOpacity = 100.0;
	DetailBlendMode = eBlendNormal;
	DetailMasked = true;

	// polygon offset (to avoid z fighting)

	FBPropertyPublish(this, UsePolygonOffset, "Use polygon offset" ,NULL, NULL );
	FBPropertyPublish(this, PolygonOffsetFactor, "Poly offset factor" ,NULL, NULL );
	FBPropertyPublish(this, PolygonOffsetUnits, "Poly offset units" ,NULL, NULL );

	UsePolygonOffset = false;
	PolygonOffsetFactor = -2.0;
	PolygonOffsetUnits = -2.0;


	//FBPropertyPublish(this, CompositeMask, "Composite Mask" ,NULL, NULL );
	//CompositeMask = FBColorAndAlpha(1.0, 0.0, 0.0, 0.0);

	FBPropertyPublish(this, CompositeMaskA, "Composite Mask A" ,NULL, NULL );
	FBPropertyPublish(this, CompositeMaskB, "Composite Mask B" ,NULL, NULL );
	FBPropertyPublish(this, CompositeMaskC, "Composite Mask C" ,NULL, NULL );
	FBPropertyPublish(this, CompositeMaskD, "Composite Mask D" ,NULL, NULL );

	CompositeMaskA = false;
	CompositeMaskB = false;
	CompositeMaskC = false;
	CompositeMaskD = false;

	

	InitializeProperties(this);

    //Set up shader capacity.  It seems cg2.0 has problem regarding INSTNCEID currently.
    //SetShaderCapacity(FBShaderCapacity(kFBShaderCapacityMaterialEffect | kFBShaderCapacityDrawInstanced), true);
    SetShaderCapacity(FBShaderCapacity(kFBShaderCapacityMaterialEffect), true);

    //Hook up the callback 
    SetShaderPassActionCallback(FBShaderPassActionCallback(kFBShaderPassTypeBegin | kFBShaderPassTypeEnd | kFBShaderPassInstanceBegin | kFBShaderPassInstanceEnd | kFBShaderPassMaterialBegin | kFBShaderPassMaterialEnd | kFBShaderPassModelDraw));

    SetDrawInstancedMaximumSize(kMaxDrawInstancedSize);
	
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	
	return ParentClass::FBCreate();
}


/************************************************
*	FiLMBOX Destructor.
************************************************/
void ProjTexShader::FBDestroy()
{
	FreeProperties();

    ParentClass::FBDestroy();
}

void ProjTexShader::DoReloadShader()
{
	// TODO: reload shader

	CGPUFBScene	&lGPUScene = CGPUFBScene::instance();
	lGPUScene.ReloadUberShader();
	
	//mUberShader = nullptr;
	//mShaderSuccess = true;
}

void ProjTexShader::SetLogarithmicDepth(const bool value) const
{
	// TODO: logarithmic depth in uber shader

	//if (mUberShader)
	//	mUberShader->SetLogarithmicDepth(value);

	//if (mpLightShader)
	//	mpLightShader->SetLogarithmicDepth(value);
}

/************************************************
*	Shader functions.
************************************************/


////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProjTexShader::ShaderPassTypeBegin    ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	if (mGPUFBScene->IsWaiting()) // || pRenderOptions->IsIDBufferRendering() )
		return;

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif

	CHECK_GL_ERROR_MOBU();

	FBCamera *pCamera = pRenderOptions->GetRenderingCamera();

	FBViewingOptions* lViewingOptions = pRenderOptions->GetViewerOptions();
    bool lIsSelectBufferPicking = lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	mSelectionMode = (lIsSelectBufferPicking || lIsColorBufferPicking);
	
	if (true == mSelectionMode)
		return;

	CRenderOptions &options = GetLastRenderOptions();
	
	CHECK_GL_ERROR_MOBU();

	InitializeFrameDataAndBuffers(pRenderOptions, pCamera, 
		FBGetDisplayInfo(), 
		pRenderOptions->GetRenderFrameId(),
		(kFBPassLighted == pPass),
		8192, 2048, false, nullptr, 0);

	if (nullptr == mGPUFBScene->GetUberShaderPtr() )
	{
		mShaderCallback = nullptr;
	}
	else
	{
		if (lIsSelectBufferPicking || lIsColorBufferPicking)
		{
			mShaderCallback = mShadersFactory.FindTypeByShaderAndGoal(this, eRenderGoalSelectionId);
			mShaderPass = eShaderPassOpaque;

			options.SetGoal(eRenderGoalSelectionId);
		}
		else
		{
			mShaderCallback = mShadersFactory.FindTypeByShaderAndGoal(this, eRenderGoalShading);
			options.SetGoal(eRenderGoalShading);

			if (kFBAlphaSourceAdditiveAlpha == Transparency)
			{
				mShaderPass = eShaderPassAdditive;
			}
			else if (kFBAlphaSourceAccurateAlpha == Transparency)
			{
				mShaderPass = eShaderPassTransparency;
			}
			else
			{
				mShaderPass = eShaderPassOpaque;
			}
			options.SetPass(mShaderPass);
		}
	}

	if (nullptr != mShaderCallback)
	{
		//options.SetPass(eShaderPassOpaque);

		mShaderCallback->OnTypeBegin( options, false );
	}
	//InternalPassTypeBegin(mGPUFBScene, mGPUFBScene->IsRenderToBuffer(), LogarithmicDepth, false);

	CHECK_GL_ERROR_MOBU();
}

void ProjTexShader::ShaderPassTypeEnd      ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)     
{
	if (mGPUFBScene->IsWaiting() ) // || pRenderOptions->IsIDBufferRendering() )
		return;

	if (true == mSelectionMode)
		return;

	CRenderOptions &options = GetLastRenderOptions();
	if (nullptr != mShaderCallback)
		mShaderCallback->OnTypeEnd( options );

	//InternalPassTypeEnd(mGPUFBScene, mGPUFBScene->IsRenderToBuffer());
	CHECK_GL_ERROR_MOBU();
}

void ProjTexShader::ShaderPassInstanceBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)  
{
	if (mGPUFBScene->IsWaiting() ) // || pRenderOptions->IsIDBufferRendering() )
		return;

	if (true == mSelectionMode)
		return;

	CRenderOptions &options = GetLastRenderOptions();

	if (nullptr != mShaderCallback)
	{
		mShaderInfo = nullptr; // mGPUFBScene->FindShaderInfo(this, mShaderCallback);
		
		// wait for updating shader-callback info
		/*
		if (nullptr == mShaderInfo)
		{
			mShaderCallback->OnTypeEnd( options );
			mShaderCallback = nullptr;
		}
		else
		*/
			mShaderCallback->OnInstanceBegin( options, pRenderOptions, this, mShaderInfo );
	}

	//InternalPassInstanceBegin( pRenderOptions->IsIDBufferRendering(), pPass );
	/*
	if (pPass == kFBPassPreRender)
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
	}
	*/
	CHECK_GL_ERROR_MOBU();
}

void ProjTexShader::ShaderPassInstanceEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)  
{
	if (mGPUFBScene->IsWaiting() ) // || pRenderOptions->IsIDBufferRendering() )
		return;

	if (true == mSelectionMode)
		return;

	CRenderOptions &options = GetLastRenderOptions();
	if (nullptr != mShaderCallback)
	{
		mShaderCallback->OnInstanceEnd( options, this, mShaderInfo );
	}

	//InternalPassInstanceEnd( pRenderOptions->IsIDBufferRendering(), pPass );
	/*
	if (pPass == kFBPassPreRender)
	{
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
	}
	*/
	CHECK_GL_ERROR_MOBU();
}

void ProjTexShader::ShaderPassMaterialBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)  
{
	if (mGPUFBScene->IsWaiting() ) // || pRenderOptions->IsIDBufferRendering() )
		return;

	if (true == mSelectionMode)
		return;

	CRenderOptions &options = GetLastRenderOptions();
	if (nullptr != mShaderCallback)
	{
		mShaderCallback->OnMaterialBegin( options, pRenderOptions, pInfo->GetFBMaterial(), true );
	}

	//InternalPassMaterialBegin( pRenderOptions->IsIDBufferRendering(), pInfo->GetFBMaterial() );
	CHECK_GL_ERROR_MOBU();
}

void ProjTexShader::ShaderPassMaterialEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)  
{
	if (mGPUFBScene->IsWaiting() ) // || pRenderOptions->IsIDBufferRendering())
		return;

	if (true == mSelectionMode)
		return;

	CRenderOptions &options = GetLastRenderOptions();
	if (nullptr != mShaderCallback)
	{
		mShaderCallback->OnMaterialEnd( options, pInfo->GetFBMaterial() );
	}

	//InternalPassMaterialEnd( pRenderOptions->IsIDBufferRendering() );
	CHECK_GL_ERROR_MOBU();
}

void ProjTexShader::ShaderPassModelDraw ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)
{
	if (mGPUFBScene->IsWaiting()  // || pRenderOptions->IsIDBufferRendering()
		|| nullptr == mGPUFBScene->GetUberShaderPtr())
		return;

	if (true == mSelectionMode)
		return;

	int regionIndex = pInfo->GetSubRegionIndex();
	/*
	if (regionIndex > 0)
	{
		printf( "what to do ?!\n" );
	}*/

	CRenderOptions &options = GetLastRenderOptions();
	if ((nullptr != mShaderCallback) ) // && (regionIndex == 0) )
	{
		mShaderCallback->OnModelDraw( options, pRenderOptions, pInfo->GetFBModel(), mShaderInfo );
		
		SuperShaderModelInfo *lInfo = (SuperShaderModelInfo*) pInfo;

		if (lInfo)
		{
			/*
				const int meshIndex = lInfo->GetCachedMeshIndex();

				if (meshIndex < 0)
				{
					mGPUFBScene->UnBindUberShader();

					// something still unsync
					InvalidateShaderVersion();
					Enable = false;
				}
				else
				{
				*/

#ifdef _DEBUG
			FBModel *pModel = pInfo->GetFBModel();
			FBString modelName( (pModel) ? pModel->LongName : "" );

			FBMaterial *pMaterial = pInfo->GetFBMaterial();
			FBString matName( (pMaterial) ? pMaterial->LongName : "" );
#endif

					int meshIndex = mGPUFBScene->FindMeshIndex(pInfo->GetFBModel(), pInfo->GetFBMaterial(), this);
					
					if (meshIndex < 0)
					{
						meshIndex = 0;
					}

					// NOTE: meshIndex == patch Index (not region Index) !!!
					/*
					if (regionIndex > 0)
					{
						meshIndex = meshIndex + regionIndex;
					}
					*/
					mGPUFBScene->GetUberShaderPtr()->UpdateMeshIndex( meshIndex );
					lInfo->Bind();
				//}
		}

	}

	//InternalPassModelDraw( pRenderOptions->IsIDBufferRendering(), pInfo->GetFBModel(), pInfo->GetSubRegionIndex(), pInfo, false );
	CHECK_GL_ERROR_MOBU();
}

void ProjTexShader::ShaderPassDrawShadowBegin( FBRenderOptions* pRenderOptions)
{
    //Here we should setup the shader's draw shadow states
}

void ProjTexShader::ShaderPassDrawShadowEnd( FBRenderOptions* pRenderOptions)
{
    //Here we should clean the shader's draw shadow states
}

void ProjTexShader::UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount)
{
	//if (!mUberShader) return;

    //mUberShader->UploadModelViewMatrixArrayForDrawInstanced(pModelViewMatrixArray, pCount);
}


bool ProjTexShader::ShaderNeedBeginRender()
{
	return false;
}

void ProjTexShader::ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	//printf ("begin\n");
}

FBShaderModelInfo *ProjTexShader::NewShaderModelInfo(HKModelRenderInfo pModelRenderInfo, int pSubRegionIndex)
{
    //FBShaderModelInfo *lShaderModelInfo = ParentClass::NewShaderModelInfo(pModelRenderInfo, pSubRegionIndex);
	FBShaderModelInfo *lShaderModelInfo = new SuperShaderModelInfo(this, pModelRenderInfo, pSubRegionIndex);
    return lShaderModelInfo;
}

void ProjTexShader::UpdateModelShaderInfo( FBRenderOptions* pOptions, FBShaderModelInfo *pModelRenderInfo )
{
    unsigned int lVBOFormat = kFBGeometryArrayID_Point | kFBGeometryArrayID_Normal;

	CHECK_GL_ERROR_MOBU();

    if (pModelRenderInfo->GetOriginalTextureFlag())
    {
        FBMaterial *lMaterial = pModelRenderInfo->GetFBMaterial();
        if (lMaterial)
        {
            //HFBTexture lDiffuseTexture = pMaterial->GetTexture(kFBMaterialTextureDiffuse);

            FBTexture *lNormalMapTexture = lMaterial->GetTexture(kFBMaterialTextureBump);
            if (lNormalMapTexture == nullptr) 
                lNormalMapTexture = lMaterial->GetTexture(kFBMaterialTextureNormalMap);

            if (lNormalMapTexture)
                lVBOFormat = lVBOFormat | kFBGeometryArrayID_Tangent | kFBGeometryArrayID_Binormal;
        }
    }

	// check if second uv set is needed in some texture
	if (pModelRenderInfo->GetFBModel() )
	{
		bool needSecondUV = false;

		FBModel *pModel = pModelRenderInfo->GetFBModel();

		for (int i=0; i<pModel->Textures.GetCount(); ++i)
		{
			FBTexture *pTexture = pModel->Textures[i];
			
			FBString uvset("");
			FBProperty *lProp = pTexture->PropertyList.Find( "UVSet" );
			if (lProp)
			{
				uvset = lProp->AsString();
			}

			if (uvset != "")
			{
				needSecondUV = true;
				break;
			}
		}

		if (needSecondUV)
		{
			lVBOFormat = lVBOFormat | kFBGeometryArrayID_SecondUVSet;
		}
	}

    pModelRenderInfo->SetGeometryArrayIds(lVBOFormat);
	pModelRenderInfo->UpdateModelShaderInfo(GetShaderVersion());
}

void ProjTexShader::SetTransparencyType( FBAlphaSource pTransparency )
{
    if (Transparency != pTransparency)
    {
        Transparency = pTransparency;
        //To trigger render to update the model-shader information.
        InvalidateShaderVersion();
    }
}

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

void ProjTexShader::DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{

	if ( mShaderLights.get() )
		mShaderLights.reset(nullptr);
	mProjectors.ChangeContext();

	//mUberShader = nullptr;
	// do we need it ?!
	// mGPUFBScene->ChangeContext();
	mNeedToUpdateProjectors = true;
}

void ProjTexShader::CheckApplyColorCorrection(FBPlug *pPlug)
{
	mApplyColorCorrection = false;

	if (eBlendMultiply != CustomColorMode.AsInt() )
	{
		mApplyColorCorrection = true;
		return;
	}

	if (CustomColor.IsAnimated() || Contrast.IsAnimated() || Saturation.IsAnimated() || Brightness.IsAnimated())
	{
		mApplyColorCorrection = true;
		return;
	}

	FBColor color;
	double value;

	CustomColor.GetData(color, sizeof(FBColor) );
	if (color[0] != 1.0 || color[1] != 1.0 || color[2] != 1.0)
	{
		mApplyColorCorrection = true;
		return;
	}
		
	Contrast.GetData(&value, sizeof(double) );
	if (value != 0.0)
	{
		mApplyColorCorrection = true;
		return;
	}

	Saturation.GetData(&value, sizeof(double) );
	if (value != 0.0)
	{
		mApplyColorCorrection = true;
		return;
	}

	Brightness.GetData(&value, sizeof(double) );
	if (value != 0.0)
	{
		mApplyColorCorrection = true;
		return;
	}

	Gamma.GetData(&value, sizeof(double) );
	if (value != 0.0)
	{
		mApplyColorCorrection = true;
		return;
	}
}

bool ProjTexShader::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if (pAction == kFBCandidated)
	{
		if (pThis == &UseSceneLights)
		{
			mNeedUpdateLightsList = true;
		}
		else
		if (pThis == &CustomColor || pThis == &CustomColorMode
			|| pThis == &Contrast || pThis == &Saturation || pThis == &Brightness)
		{
			mNeedToCheckColorCorrection = true;
		}
		else if (true == FXProjectionMappingContainer::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize ) )
		{
			mNeedToUpdateProjectors = true;
		}
		/*
		else if (pThis == &RenderOnBack)
		{
			if (true == RenderOnBack)
				RenderLayer = eRenderLayerBackground;
		}
		else if (pThis == &RenderLayer)
		{
			if (eRenderLayerBackground == RenderLayer)
				RenderOnBack = true;
			else
				RenderOnBack = false;
		}
		*/
	}

	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ProjTexShader::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	if (pThis == &AffectingLights)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
			AskToUpdateLightList();
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
			AskToUpdateLightList();
		}
	}
	else if (pThis == &Mask1 || pThis == &Mask2)
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
	else if ( pThis == &MatCap )
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
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

void ProjTexShader::AskToUpdateAll()
{
	mNeedUpdateLightsList = true;
	mNeedToUpdateProjectors = true;
	mNeedToCheckColorCorrection = true;
}

void ProjTexShader::AskToUpdateLightList()
{
	mNeedUpdateLightsList = true;
}

void ProjTexShader::EventBeforeRenderNotify()
{
	// if we have any lights attached
	//	we should update lights every frame

	if (true == Enable 
		&& false == UseSceneLights
		&& AffectingLights.GetCount() > 0 )
	{
		mNeedUpdateLightsList = true;
	}

	if (mNeedUpdateLightsList && false == UseSceneLights)
	{
		if (nullptr == mShaderLights.get() )
			mShaderLights.reset( new CGPUShaderLights() );

		mGPUFBScene->PrepShaderLights( UseSceneLights, 
			&AffectingLights, mLightsPtr, mShaderLights.get() );

		mGPUFBScene->PrepLightsInViewSpace( mShaderLights.get() );
		mShaderLights->MapOnGPU();
		mShaderLights->PrepGPUPtr();

		mNeedUpdateLightsList = false;
	}

	if (mNeedToCheckColorCorrection)
	{
		CheckApplyColorCorrection(nullptr);
		mNeedToCheckColorCorrection = false;
	}

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