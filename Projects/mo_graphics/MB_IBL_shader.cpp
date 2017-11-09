
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_IBL_shader.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "MB_IBL_shader.h"

// Class declaration
#include "ShaderModelInfo.h"
#include <math.h>
#include <array>

#include "FX_shader.h"

#include "IO\FileUtils.h"

#include "shared_content.h"
#include "graphics\OGL_Utils.h"
#include "graphics\OGL_Utils_mobu.h"
#include "graphics\particlesDrawHelper.h"
#include "algorithm\math3d_mobu.h"

#include "MB_renderer.h"

//--- Registration defines
#define IBLSHADER__CLASS	IBLSHADER__CLASSNAME
#define EYESHADER__CLASS	EYESHADER__CLASSNAME
#define SKINSHADER__CLASS	SKINSHADER__CLASSNAME


//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	IBLSHADER__CLASS	);
FBRegisterShader	(IBLSHADER__DESCSTR,	    // Unique name
                     IBLSHADER__CLASS,			// Class
                     IBLSHADER__DESCSTR,	    // Short description
                     IBLSHADER__DESC,		    // Long description
                     FB_DEFAULT_SDK_ICON	);		// Icon filename (default=Open Reality icon)

FBShaderImplementation(	EYESHADER__CLASS	);
FBRegisterShader	(EYESHADER__DESCSTR,	    // Unique name
                     EYESHADER__CLASS,			// Class
                     EYESHADER__DESCSTR,	    // Short description
                     EYESHADER__DESC,		    // Long description
                     FB_DEFAULT_SDK_ICON	);		// Icon filename (default=Open Reality icon)

FBShaderImplementation(	SKINSHADER__CLASS	);
FBRegisterShader	(SKINSHADER__DESCSTR,	    // Unique name
                     SKINSHADER__CLASS,			// Class
                     SKINSHADER__DESCSTR,	    // Short description
                     SKINSHADER__DESC,		    // Long description
                     FB_DEFAULT_SDK_ICON	);		// Icon filename (default=Open Reality icon)


#ifdef _DEBUG
	extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);
#endif

void ORIBLShader::SetReloadShader(HIObject object, bool pValue)
{
	ORIBLShader *shader = FBCast<ORIBLShader>(object);
	if (shader && pValue) 
	{
		shader->DoReloadShader();
	}
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ORIBLShader::FBCreate()
{
	//mShaderSuccess = true;
    //mpLightShaderRefCount++;
	mGPUFBScene = &CGPUFBScene::instance();
	mLastUniqueFrameId = 0;
	
	//
	//

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
	
	
	FBPropertyPublish(	this, RenderLayer,	"Render Layer", nullptr, nullptr);
	RenderLayer = eRenderLayerMain;

	FBPropertyPublish(	this, ShadingType,	"Shading",NULL, NULL);
	ShadingType = eShadingTypeDynamic;

	FBPropertyPublish( this, ReloadShader,	"Reload Shader", nullptr, SetReloadShader);

	
	// rim lighting

	FBPropertyPublish(	this, UseRim, "Use Rim Lighting" ,NULL, NULL);
	FBPropertyPublish(	this, RimPower, "Rim Power" ,NULL, NULL);
	FBPropertyPublish(	this, RimColor, "Rim Color" ,NULL, NULL);
	FBPropertyPublish(	this, RimTexture, "Rim Texture" ,NULL, NULL);
	FBPropertyPublish(	this, RimBlend, "Rim Blend" ,NULL, NULL);

	//FBPropertyPublish(	this, MatCap, "MatCap Texture" ,NULL, NULL);

	FBPropertyPublish(	this, DiffuseEnv, "Diffuse Env" ,NULL, NULL);
	FBPropertyPublish(	this, SpecularEnv, "Specular Env" ,NULL, NULL);
	FBPropertyPublish(	this, BRDF, "BRDF" ,NULL, NULL);

	UseRim.SetMinMax(0.0, 100.0);
	UseRim = 0.0;
	RimPower.SetMinMax(0.0, 200.0);
	RimPower = 100.0;
	RimColor = FBColor(1.0, 1.0, 1.0);
	RimBlend = eBlendAdd;

	RimTexture.SetSingleConnect(true);
	RimTexture.SetFilter( FBTexture::GetInternalClassId() );

	//MatCap.SetSingleConnect(true);
	//MatCap.SetFilter( FBTexture::GetInternalClassId() );

	DiffuseEnv.SetSingleConnect(true);
	DiffuseEnv.SetFilter( FBUserObject::GetInternalClassId() );
	SpecularEnv.SetSingleConnect(true);
	SpecularEnv.SetFilter( FBUserObject::GetInternalClassId() );
	BRDF.SetSingleConnect(true);
	BRDF.SetFilter( FBTexture::GetInternalClassId() );

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
void ORIBLShader::FBDestroy()
{
    ParentClass::FBDestroy();
}

void ORIBLShader::DoReloadShader()
{
	// TODO: reload shader

	CGPUFBScene	&lGPUScene = CGPUFBScene::instance();
	lGPUScene.ReloadUberShader();
	
	//mUberShader = nullptr;
	//mShaderSuccess = true;
}

void ORIBLShader::SetLogarithmicDepth(const bool value) const
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

void ORIBLShader::ShaderPassTypeBegin    ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)
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

	mShaderCallback = nullptr;
	if ( nullptr != mGPUFBScene->GetShaderFXPtr(Graphics::MATERIAL_SHADER_IBL) )
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

void ORIBLShader::ShaderPassTypeEnd      ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)     
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

void ORIBLShader::ShaderPassInstanceBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)  
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

void ORIBLShader::ShaderPassInstanceEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass)  
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

void ORIBLShader::ShaderPassMaterialBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)  
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

void ORIBLShader::ShaderPassMaterialEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)  
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

void ORIBLShader::ShaderPassModelDraw ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo *pInfo)
{
	Graphics::BaseMaterialShaderFX *pShaderFX = mGPUFBScene->GetShaderFXPtr(Graphics::MATERIAL_SHADER_IBL);

	if (mGPUFBScene->IsWaiting()  // || pRenderOptions->IsIDBufferRendering()
		|| nullptr == pShaderFX)
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
					pShaderFX->UpdateMeshIndex( meshIndex );
					lInfo->Bind();
				//}
		}

	}

	//InternalPassModelDraw( pRenderOptions->IsIDBufferRendering(), pInfo->GetFBModel(), pInfo->GetSubRegionIndex(), pInfo, false );
	CHECK_GL_ERROR_MOBU();
}

void ORIBLShader::ShaderPassDrawShadowBegin( FBRenderOptions* pRenderOptions)
{
    //Here we should setup the shader's draw shadow states
}

void ORIBLShader::ShaderPassDrawShadowEnd( FBRenderOptions* pRenderOptions)
{
    //Here we should clean the shader's draw shadow states
}

void ORIBLShader::UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount)
{
	//if (!mUberShader) return;

    //mUberShader->UploadModelViewMatrixArrayForDrawInstanced(pModelViewMatrixArray, pCount);
}


bool ORIBLShader::ShaderNeedBeginRender()
{
	return false;
}

void ORIBLShader::ShaderBeginRender( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	//printf ("begin\n");
}

FBShaderModelInfo *ORIBLShader::NewShaderModelInfo(HKModelRenderInfo pModelRenderInfo, int pSubRegionIndex)
{
    //FBShaderModelInfo *lShaderModelInfo = ParentClass::NewShaderModelInfo(pModelRenderInfo, pSubRegionIndex);
	FBShaderModelInfo *lShaderModelInfo = new SuperShaderModelInfo(this, pModelRenderInfo, pSubRegionIndex);
    return lShaderModelInfo;
}

void ORIBLShader::UpdateModelShaderInfo( FBRenderOptions* pOptions, FBShaderModelInfo *pModelRenderInfo )
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

void ORIBLShader::SetTransparencyType( FBAlphaSource pTransparency )
{
    if (Transparency != pTransparency)
    {
        Transparency = pTransparency;
        //To trigger render to update the model-shader information.
        InvalidateShaderVersion();
    }
}

FBAlphaSource ORIBLShader::GetTransparencyType()
{
    return Transparency;
}

void ORIBLShader::SetTransparencyProperty( HIObject pObject, FBAlphaSource pState )
{     
    ORIBLShader* lShader = FBCast<ORIBLShader>(pObject);
    if (lShader->Transparency != pState)
    {
        lShader->Transparency.SetPropertyValue(pState);
        lShader->RenderingPass = GetRenderingPassNeededForAlpha(pState);
         
        // if shader use alpha and thus generate custom shape than the original geometry shape, 
        // we need to let it handle DrawShadow functiionality as well. 
        lShader->SetShaderCapacity(kFBShaderCapacityDrawShadow, pState != kFBAlphaSourceNoAlpha); 
    }
}

void ORIBLShader::DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{

	if ( mShaderLights.get() )
		mShaderLights.reset(nullptr);
}

bool ORIBLShader::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if (pAction == kFBCandidated)
	{
		if (pThis == &UseSceneLights)
		{
			mNeedUpdateLightsList = true;
		}
	}

	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ORIBLShader::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
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
	/*
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
	*/

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void ORIBLShader::AskToUpdateAll()
{
	mNeedUpdateLightsList = true;
}

void ORIBLShader::AskToUpdateLightList()
{
	mNeedUpdateLightsList = true;
}

void ORIBLShader::EventBeforeRenderNotify()
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
}



//////////////////////////////////////////////////////////////////
// ORCharacterEyeShader

void ORCharacterEyeShader::SetResetValues(HIObject object, bool pValue)
{
	ORCharacterEyeShader* lShader = FBCast<ORCharacterEyeShader>(object);
    if (nullptr != lShader && true == pValue)
    {
		lShader->SetDefaultValues();
	}
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ORCharacterEyeShader::FBCreate()
{

	FBPropertyPublish(this, EyeEnvReflection, "Eye Env Reflection" ,NULL, NULL );
	FBPropertyPublish(this, EyeEnvDiffuse, "Eye Env Diffuse" ,NULL, NULL );
	FBPropertyPublish(this, EyeEnvRefraction, "Eye Env Refraction" ,NULL, NULL );

	FBPropertyPublish(this, IrisSize, "Iris Size" ,NULL, NULL );
	FBPropertyPublish(this, CorneaBumpAmount, "Cornea Bump Amount" ,NULL, NULL );
	FBPropertyPublish(this, CorneaBumpRadiusMult, "Cornea Bump Radius Mult" ,NULL, NULL );

	FBPropertyPublish(this, PupilSize, "Pupil Size" ,NULL, NULL );
	FBPropertyPublish(this, IrisTexStart, "Iris Tex Start" ,NULL, NULL );
	FBPropertyPublish(this, IrisTexEnd, "Iris Tex End" ,NULL, NULL );
	FBPropertyPublish(this, IrisBorder, "Iris Border" ,NULL, NULL );
	FBPropertyPublish(this, IrisSize, "Iris Size" ,NULL, NULL );
	FBPropertyPublish(this, IrisEdgeFade, "Iris Edge Fade" ,NULL, NULL );
	FBPropertyPublish(this, IrisInsetDepth, "Iris Inset Depth" ,NULL, NULL );
	FBPropertyPublish(this, ScleraTexScale, "Sclera Tex Scale" ,NULL, NULL );
	FBPropertyPublish(this, ScleraTexOffset, "Sclera Tex Offset" ,NULL, NULL );
	FBPropertyPublish(this, Ior, "Ior" ,NULL, NULL );
	FBPropertyPublish(this, RefractEdgeSoftness, "Refract Edge Softness" ,NULL, NULL );

	FBPropertyPublish(this, IrisTextureCurvature, "Iris Texture Curvature" ,NULL, NULL );
	FBPropertyPublish(this, ArgIrisShadingCurvature, "Arg Iris Shading Curvature" ,NULL, NULL );

	FBPropertyPublish(this, TexUOffset, "Tex U Offset" ,NULL, NULL );
	FBPropertyPublish(this, IrisNormalOffset, "Iris Normal Offset" ,NULL, NULL );
	FBPropertyPublish(this, CorneaDensity, "Cornea Density" ,NULL, NULL );
	FBPropertyPublish(this, BumpTexture, "Bump Texture" ,NULL, NULL );
	FBPropertyPublish(this, CatShape, "Cat Shape" ,NULL, NULL );
	FBPropertyPublish(this, CybShape, "Cyb Shape" ,NULL, NULL );
	FBPropertyPublish(this, ColTexture, "Col Texture" ,NULL, NULL );

	FBPropertyPublish(this, ResetValues, "Reset Values" ,NULL, SetResetValues );

	//
	std::array<FBPropertyListObject*, 3> texturesArray = {&EyeEnvReflection, &EyeEnvDiffuse, &EyeEnvRefraction};

	std::for_each(texturesArray.begin(), texturesArray.end(), [] (FBPropertyListObject* pObject) {
		pObject->SetSingleConnect(true);
		pObject->SetFilter( FBTexture::GetInternalClassId() );
	} );

	//
	SetDefaultValues();

	return ParentClass::FBCreate();
}

void ORCharacterEyeShader::FBDestroy()
{
	return ParentClass::FBDestroy();
}

void ORCharacterEyeShader::SetDefaultValues()
{
	// most of values * 0.01
	PupilSize = 20.0;	//
	IrisTexStart = 0.9;
	IrisTexEnd = 13.0;
	IrisBorder = 0.1;
	IrisSize = 52.0;
	IrisEdgeFade = 4.0;
	IrisInsetDepth = 3.0;
	ScleraTexScale = -14.0;
	ScleraTexOffset = 4.0;
	Ior = 130.0;
	RefractEdgeSoftness = 10.0;
	IrisTextureCurvature = 51.0;
	ArgIrisShadingCurvature = 51.0;

	TexUOffset = 25.0;
	// TexVOffset = 0.0;
	CorneaBumpAmount = 10.0;
	CorneaBumpRadiusMult = 90.0;
	IrisNormalOffset = 0.1;
	CorneaDensity = 0.1;
	BumpTexture = 30.0;
	CatShape = false;
	ColTexture = true;
}

//////////////////////////////////////////////////////////////////
//

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ORCharacterSkinShader::FBCreate()
{
	return ParentClass::FBCreate();
}

void ORCharacterSkinShader::FBDestroy()
{
	return ParentClass::FBDestroy();
}