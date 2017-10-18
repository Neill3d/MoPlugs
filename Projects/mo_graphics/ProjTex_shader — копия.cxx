
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

//--- Registration defines
#define PROJTEX__CLASS	PROJTEX__CLASSNAME

const char * FBPropertyBaseEnum<EShadingType>::mStrings[] = {"Phong", "Flat", "Toon", "Matte", "MatCap", 0};

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
static void ProjTex_SetLogarithmicDepth(HIObject object, bool pValue)
{
	ProjTexShader *shader = FBCast<ProjTexShader>(object);
	if (shader) 
	{
		shader->SetLogarithmicDepth(pValue);
		shader->LogarithmicDepth.SetPropertyValue(pValue);
	}
}

static void ProjTex_SetRenderOnBack(HIObject object, bool pValue)
{
	ProjTexShader *shader = FBCast<ProjTexShader>(object);
	if (shader) 
	{
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
	}
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
	mShaderSuccess = true;
    //mpLightShaderRefCount++;
	mGPUFBScene = &CGPUFBScene::instance();
	mUberShader = nullptr;
	mLastModel = nullptr;
	mLastMaterial = nullptr;

	mNeedUpdateLightsList = true;

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

	FBPropertyPublish( this, LogarithmicDepth, "Accurate Depth", nullptr, ProjTex_SetLogarithmicDepth );
	LogarithmicDepth = false;

	FBPropertyPublish( this, DepthDisplacement, "Depth Displacement", nullptr, nullptr );
	DepthDisplacement = 0.0;
	//DepthDisplacement.SetMinMax( -100.0, 100.0, true, true );

	FBPropertyPublish(	this, RenderOnBack,	"Render On Back", nullptr, ProjTex_SetRenderOnBack);
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
	/*
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	*/
    return true;
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
	
	mUberShader = nullptr;
	mShaderSuccess = true;
}

void ProjTexShader::SetLogarithmicDepth(const bool value) const
{
	// TODO: logarithmic depth in uber shader

	if (mUberShader)
		mUberShader->SetLogarithmicDepth(value);

	//if (mpLightShader)
	//	mpLightShader->SetLogarithmicDepth(value);
}

/************************************************
*	Shader functions.
************************************************/

void ProjTexShader::InternalPassTypeBegin(CGPUFBScene *pScene, bool renderToNormalAndMask, const bool logarithmicDepth, const bool cubeMapSetup, const CubeMapRenderingData *cubemap)
{
	Graphics::ShaderEffect *mUberShader = pScene->GetUberShaderPtr();
	if (!mUberShader || pScene->GetCamera() == nullptr) {
		
		return;
	}


	// DONE: we should do this only when draw to buffer is used !!
	// proj tex outputs to normal and mask attachments, not only color
	if ( renderToNormalAndMask ) // pScene->IsRenderToBuffer()
	{
		GLenum buffers [] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers( 3, buffers );
	}

	CHECK_GL_ERROR();

	//
	mUberShader->SetBindless(false);
	mUberShader->NoTextures(false);
	
	//mUberShader->SetLogarithmicDepth( logarithmicDepth );
	mUberShader->SetNumberOfProjectors( 0 );
	mUberShader->SetRimParameters( 0.0, 0.0, false, vec3(0.0f, 0.0f, 0.0f) );
	mUberShader->SetDepthDisplacement( 0.0f );
	
	//
	pScene->PrepRender();
	pScene->BindUberShader(false);

	//
	CGPUVertexData::renderPrep();
}

void ProjTexShader::InternalPassTypeEnd(CGPUFBScene *pScene, bool renderToNormalAndMask )
{
	Graphics::ShaderEffect *mUberShader = pScene->GetUberShaderPtr();
	if (!mUberShader) return;

	// proj tex outputs to normal and mask attachments, not only color
	if ( renderToNormalAndMask ) // pScene->IsRenderToBuffer()
	{
		GLenum buffers [] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers( 1, buffers );
	}

	mUberShader->UnsetTextures();
	pScene->UnBindUberShader();
	
	// unbind projectors

	//mProjectors.UnBind();

	// unbind matCap and shadows

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	CGPUVertexData::renderFinish();
}



bool ProjTexShader::InternalPassInstanceBegin( const bool isIDBufferRendering, FBRenderingPass pPass )
{
	mUberShader = mGPUFBScene->GetUberShaderPtr();
	if (!mUberShader || isIDBufferRendering || mGPUFBScene->GetCamera() == nullptr) {
		mSkipRender = true;
		return false;
	}
	mSkipRender = false;

	mLastModel = nullptr;
	mLastMaterial = nullptr;
	mLastFXShader = nullptr;

	// all about projections
	// TODO: update only on changes, NOT EACH FRAME !!!
	if (DynamicUpdate)
	{
		mProjectors.Prep(this);
		//DoManualUpdate();
	}

	mUberShader->UpdateDepthDisplacement( DepthDisplacement );
	mUberShader->UpdateNumberOfProjectors( mProjectors.GetNumberOfProjectors() );

	double useRim, rimPower;
	FBColor rimColor;

	UseRim.GetData( &useRim, sizeof(double) );
	RimColor.GetData( rimColor, sizeof(FBColor) );
	RimPower.GetData( &rimPower, sizeof(double) );

	mUberShader->UpdateRimParameters( 0.01 * (float)useRim, 0.01 * (float)rimPower, (RimTexture.GetCount()>0), vec4( (float)rimColor[0], (float)rimColor[1], (float)rimColor[2], (float) RimBlend.AsInt() ) );

	if (RimTexture.GetCount() > 0)
	{
		GLuint rimId = 0;

		FBTexture *pTexture = (FBTexture*) RimTexture.GetAt(0);
		pTexture->OGLInit();
		rimId = pTexture->TextureOGLId;

		glActiveTexture(GL_TEXTURE16);
		glBindTexture(GL_TEXTURE_2D, rimId);
	}

	if (ShadingType == eShadingTypeMatCap && MatCap.GetCount() > 0)
	{
		GLuint matCapId = 0;

		FBTexture *pTexture = (FBTexture*) MatCap.GetAt(0);
		pTexture->OGLInit();
		matCapId = pTexture->TextureOGLId;

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, matCapId);
	}

	if (mNeedUpdateLightsList)
	{
		mGPUFBScene->PrepShaderLights( UseSceneLights, &AffectingLights, mLightsPtr, mShaderLights );
		mNeedUpdateLightsList = false;
	}

	CHECK_GL_ERROR();

	
	
	// Bind Projective Data was here !
	//

	mProjectors.Bind( mUberShader->GetFragmentProgramId(), 
		mUberShader->GetCustomEffectShaderLocationsPtr()->allTheProjectors );
	//BindProjectionMapping();
	mFXProjectionBinded = false;

	
	//glEnableVertexAttribArray(5);

	if (UsePolygonOffset)
	{
		double factor = PolygonOffsetFactor;
		double units = PolygonOffsetUnits;

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( (float) factor, (float) units );
	}

	StoreCullMode();

	if (pPass == kFBPassPreRender)
	{
		glDepthMask(GL_FALSE);
		//mGPUFBScene->BindBackground();
	}
	else
	{
		// back background sampler to slot 7
		if (ShadingType == eShadingTypeMatte)
		{
			glActiveTexture(GL_TEXTURE7);
			mGPUFBScene->BindBackgroundTexture();
			glActiveTexture(GL_TEXTURE0);
		}
	}

	return true;
}

void ProjTexShader::InternalPassInstanceEnd  ( const bool isIDBufferRendering, FBRenderingPass pPass)  
{
	if (mSkipRender == true) {
		return;
	}

	//glDisableVertexAttribArray(5);

	if (UsePolygonOffset)
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	FetchCullMode();
	
	if (pPass == kFBPassPreRender)
	{
		glDepthMask(GL_TRUE);
		//mGPUFBScene->UnBindBackground();
	}
}



void ProjTexShader::InternalPassMaterialBegin( const bool isIDBufferRendering, FBMaterial *pMaterial )
{
	if (mSkipRender || isIDBufferRendering || !pMaterial) 
		return;
	
	if (pMaterial != mLastMaterial)
	{
		// DONE: bind material textures
		const int rId = GetTextureId( pMaterial->GetTexture(kFBMaterialTextureReflection), nullptr );
		const int sId = GetTextureId( pMaterial->GetTexture(kFBMaterialTextureSpecular), nullptr );
		const int tId = GetTextureId( pMaterial->GetTexture(kFBMaterialTextureTransparent), nullptr );
		const int dId = GetTextureId( pMaterial->GetTexture(), nullptr );
	
		if (rId > 0)
		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, rId);

			// use specified cubemap uniforms for parallax correction when computing reflections in shader
			if (rId > 0)
				mGPUFBScene->UploadCubeMapUniforms(rId);
		}
		if (sId > 0)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, sId);
		}
		if (tId > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tId);
		}
		if (dId > 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, dId);
		}
	}

	mLastMaterial = pMaterial;
	mLastModel = nullptr;
}

void ProjTexShader::InternalPassMaterialEnd( const bool isIDBufferRendering )
{
	if (mSkipRender || isIDBufferRendering ) return;
	
	//mUberShader->UnsetTextures();
}


bool ProjTexShader::InternalPassModelDraw( const bool isIDBufferRendering, FBModel *pModel, const int subRegionIndex, FBShaderModelInfo *pSuperInfo, const bool directBind )
{
	if ( mSkipRender || isIDBufferRendering || !pModel ) 
		return false;

	const int numberOfShaders = pModel->Shaders.GetCount();
	bool binded = false;

	
	for (int i=0; i<numberOfShaders; ++i)
	{
		FBShader *pShader = pModel->Shaders[i];
		if ( pShader->Enable && FBIS(pShader, FXProjectionMapping) )
		{
			mProjectors.UnBind();
			//UnBindProjectionMapping();
			
			mLastFXShader = (FXProjectionMapping*) pShader;
			//mLastFXShader->BindProjectionMapping();
			mProjectors.Prep(mLastFXShader);
			mProjectors.Bind(mUberShader->GetFragmentProgramId(), mUberShader->GetCustomEffectShaderLocationsPtr()->allTheProjectors );

			mUberShader->UpdateNumberOfProjectors( mProjectors.GetNumberOfProjectors() );

			binded = true;
			mFXProjectionBinded = true;
			
			//!!! Support only one projections override
			//break;
		}
		else
		if ( pShader->Enable && FBIS(pShader, FXShadingShader) )
		{
			if ( ( (FXShadingShader*) pShader)->ShadingType == eShadingTypeMatte )
			{
				glActiveTexture(GL_TEXTURE7);
				mGPUFBScene->BindBackgroundTexture();
				glActiveTexture(GL_TEXTURE0);
			}
		}
	}

	if (!binded && mFXProjectionBinded)
	{
		if (mLastFXShader)
		{
			mProjectors.UnBind();
			//mLastFXShader->UnBindProjectionMapping();
			mLastFXShader = nullptr;
		}

		mProjectors.Prep(this);
		mProjectors.Bind(mUberShader->GetFragmentProgramId(), 
			mUberShader->GetCustomEffectShaderLocationsPtr()->allTheProjectors);
		//BindProjectionMapping();

		// bind native projections
		mUberShader->UpdateNumberOfProjectors( mProjectors.GetNumberOfProjectors() );
	}

	SuperShaderModelInfo *lInfo = (SuperShaderModelInfo*) pSuperInfo;
	

	if (lInfo || directBind)
	{
		if (mLastModel == pModel ) 
			return true;

		if (lInfo)
		{
			const int meshIndex = lInfo->GetCachedMeshIndex();
			mUberShader->UpdateMeshIndex( meshIndex );
		}

		// upload transformation
		//mGPUFBScene->UploadModelInfo(pModel, true);
		
		/*
		// DONE: pass model or mesh GLSL ptr ?!
		if (mGPUFBScene->GetModelsManagerPtr())
		{
			const GLuint meshLoc = 5;
			const GLuint modelLoc = 6;
		
			mGPUFBScene->GetModelsManagerPtr()->BindSpecifiedAsAttrib( modelLoc, meshLoc, pModel, subRegionIndex );
		}
		*/

		// bind vertex buffers (position, normal, uv, tangent)
		if (directBind)
			BindVertexAttribFromFBModel(pModel);
		else
			lInfo->Bind();

		mLastModel = pModel;	
	}
	else
	{
		//FBMatrix m;
		//m.Identity();

		mat4 m;
		m.identity();

		mUberShader->UploadModelTransform( m );

		mLastModel = nullptr;
	} 

	// support for sprite sheets

	FBPropertyAnimatable *pAnimProp = (FBPropertyAnimatable*) pModel->PropertyList.Find( "SpriteSheet Translation" );
	if (pAnimProp)
	{
		FBVector3d	v;
		pAnimProp->GetData(v, sizeof(double)*3);

		mUberShader->UpdateTextureOffset( vec4( (float)v[0], (float)v[1], (float)v[2], 0.0) );

		pAnimProp = (FBPropertyAnimatable*) pModel->PropertyList.Find( "SpriteSheet Scaling" );
		if (pAnimProp)
		{
			pAnimProp->GetData(v, sizeof(double)*3);
			mUberShader->UpdateTextureScaling( vec4( (float)v[0], (float)v[1], (float)v[2], 0.0) );
		}
	}
	else
	{
		mUberShader->UpdateTextureOffset( vec4( 0.0, 0.0, 0.0, 0.0) );
		mUberShader->UpdateTextureScaling( vec4( 1.0, 1.0, 1.0, 1.0) );
	}

	// support for composite masks
	vec4 mask;
	mask.x = (CompositeMaskA) ? 1.0f : 0.0f;
	mask.y = (CompositeMaskB) ? 1.0f : 0.0f;
	mask.z = (CompositeMaskC) ? 1.0f : 0.0f;
	mask.w = (CompositeMaskD) ? 1.0f : 0.0f;
	
	mUberShader->UpdateShaderMask( mask );

	// support for cullmode

	FBModelCullingMode cullMode = pModel->GetCullingMode();

	switch(cullMode)
	{
	case kFBCullingOff:
		glDisable(GL_CULL_FACE);
		break;
	case kFBCullingOnCW:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	case kFBCullingOnCCW:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	}

	return (lInfo != nullptr || directBind);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProjTexShader::ShaderPassTypeBegin    ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
{
	if (mGPUFBScene->IsWaiting() || pRenderOptions->IsIDBufferRendering() )
		return;

	FBCamera *pCamera = pRenderOptions->GetRenderingCamera();
	
	if (pCamera != mGPUFBScene->GetCamera() )
	{
		// we need to prepare a new camera for a next pane rendering
		mGPUFBScene->PrepareCamera(pCamera, false, 0, nullptr);
	}




	InternalPassTypeBegin(mGPUFBScene, mGPUFBScene->IsRenderToBuffer(), LogarithmicDepth, false);
}

void ProjTexShader::ShaderPassTypeEnd      ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)     
{
	if (mGPUFBScene->IsWaiting() || pRenderOptions->IsIDBufferRendering() )
		return;

	InternalPassTypeEnd(mGPUFBScene, mGPUFBScene->IsRenderToBuffer());
}

void ProjTexShader::ShaderPassInstanceBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)  
{
	if (mGPUFBScene->IsWaiting() || pRenderOptions->IsIDBufferRendering() )
		return;

	if (pRenderOptions->IsIDBufferRendering() )
	{
		printf( "picking\n" );
	}

	InternalPassInstanceBegin( pRenderOptions->IsIDBufferRendering(), pPass );
	/*
	if (pPass == kFBPassPreRender)
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
	}
	*/
}

void ProjTexShader::ShaderPassInstanceEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)  
{
	if (mGPUFBScene->IsWaiting() || pRenderOptions->IsIDBufferRendering() )
		return;

	InternalPassInstanceEnd( pRenderOptions->IsIDBufferRendering(), pPass );
	/*
	if (pPass == kFBPassPreRender)
	{
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
	}
	*/
}

void ProjTexShader::ShaderPassMaterialBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)  
{
	if (mGPUFBScene->IsWaiting() || pRenderOptions->IsIDBufferRendering() )
		return;

	InternalPassMaterialBegin( pRenderOptions->IsIDBufferRendering(), pInfo->GetFBMaterial() );
}

void ProjTexShader::ShaderPassMaterialEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)  
{
	if (mGPUFBScene->IsWaiting() || pRenderOptions->IsIDBufferRendering())
		return;

	InternalPassMaterialEnd( pRenderOptions->IsIDBufferRendering() );
}

void ProjTexShader::ShaderPassModelDraw ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)
{
	if (mGPUFBScene->IsWaiting() || pRenderOptions->IsIDBufferRendering())
		return;

	InternalPassModelDraw( pRenderOptions->IsIDBufferRendering(), pInfo->GetFBModel(), pInfo->GetSubRegionIndex(), pInfo, false );
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
	if (!mUberShader) return;

    mUberShader->UploadModelViewMatrixArrayForDrawInstanced(pModelViewMatrixArray, pCount);
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

	CHECK_GL_ERROR();

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
	mUberShader = nullptr;
	mGPUFBScene->ChangeContext();
}

bool ProjTexShader::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	if (pThis == &AffectingLights)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
			UpdateLightsList();
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
			UpdateLightsList();
		}
	}
	else if (pThis == &Mask1 || pThis == &Mask2)
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
			}
			else if (pAction == kFBDisconnectedSrc)
			{
				DisconnectSrc(pPlug);
			}
		}
	}

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void ProjTexShader::StoreCullMode()
{
	mCullFace = glIsEnabled(GL_CULL_FACE);
	glGetIntegerv( GL_CULL_FACE_MODE, &mCullFaceMode );
}


void ProjTexShader::FetchCullMode ()
{
	if (mCullFace == GL_TRUE) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	glCullFace(mCullFaceMode);
}

void ProjTexShader::UpdateLightsList()
{
	mNeedUpdateLightsList = true;
}