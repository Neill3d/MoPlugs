
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectFinal.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_objectFinal.h"

#include "compositeMaster_object.h"
#include "compositemaster_common.h"
#include "compositeMaster_shaders.h"
#include "graphics\fpTexture.h"
#include "graphics\CheckGLError_MOBU.h"
#include "IO\FileUtils.h"
#include "shared_misc_mobu.h"

#include "algorithm\nv_math.h"

FBClassImplementation2(ObjectComposition)
FBUserObjectImplement(ObjectComposition, "Composition Final", MASTER_ICON);					//Register UserObject class
FBElementClassImplementation2(ObjectComposition, COMPOSITEFINAL__ASSETNAME, MASTER_ICON);	//Register to the asset system

extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);

const char * FBPropertyBaseEnum<ECompositionQuality>::mStrings[] = {
	"Full",
	"Preview2",
	"Preview4",
	0};
/*
const char * FBPropertyBaseEnum<ECompositionSizeControl>::mStrings[] = {
	"Input",
	"Input POT",
	"Viewport",
	"Custom",
	0};
	*/
const char *stats_names[eCompositionStatsCount] = {
		"Stats Width",
		"Stats Height",
		"Stats Compute Shaders",
		"Stats Dispatch Groups",
		"Stats Textures Count",
		"Stats Textures Memory"
	};

/////////////////////////////////////////////////////////////////////////////////////////////

class CObjectCompositionInfoImpl : public CCompositionInfo
{
public:
	//! a constructor
	CObjectCompositionInfoImpl(ObjectCompositeBase *_pFinal)
		: CCompositionInfo(_pFinal)
		, mFinal( (ObjectComposition*)_pFinal)
	{
		mLastMultisampleMode = false;
		mLastNumberOfSamples = 0;

		for (int i=0; i<256; ++i)
			mStrSamplesDefine[i] = 0;
	}

	//! a destructor
	virtual ~CObjectCompositionInfoImpl()
	{}

	virtual bool UsedForBatchProcessing() const
	{
		return false;	// TODO: !!??
	}

	virtual const int		GetWidth() const override
	{
		//return mProcessingWidth;
		return mFinal->GetWidthValue();
	}

	virtual const int		GetHeight() const override
	{
		//return mProcessingHeight;
		return mFinal->GetHeightValue();
	}

	virtual const float		GetPreviewSizeFactor() const override
	{
		return mFinal->GetPreviewSizeFactor();
	}

	// info about the current viewport rendering

	virtual FBCamera		*GetRenderCamera() override
	{
		return mFinal->GetRenderData().pCamera;
	}

	virtual FBModel			*GetRenderCameraInterest() override
	{
		return mFinal->GetRenderData().pCameraInterest;
	}

	virtual const double	*GetRenderCameraMVP() const override
	{
		if (nullptr != mFinal->GetRenderData().pCamera)
		{
			mFinal->GetRenderData().pCamera->GetCameraMatrix( (FBMatrix&)mCameraMVP, kFBModelViewProj);
			return mCameraMVP;
		}
		return nullptr;
	}

	virtual const int		GetRenderWidth() const override
	{
		return mFinal->GetRenderData().w;
	}

	virtual const int		GetRenderHeight() const override
	{
		return mFinal->GetRenderData().h;
	}

	virtual const int		GetRenderBorderX() const override
	{
		return mFinal->GetRenderData().borderX;
	}

	virtual const int		GetRenderBorderY() const override
	{
		return mFinal->GetRenderData().borderY;
	}

	virtual const double	GetCameraNearPlane() const override
	{
		return mFinal->GetRenderData().nearPlane;
	}
	virtual const double	GetCameraFarPlane() const override
	{
		return mFinal->GetRenderData().farPlane;
	}

	virtual bool IsRenderMultisampling() const override
	{
		return mFinal->GetRenderData().ms;
	}

	virtual int	GetNumberOfSamples() const override
	{
		return mFinal->GetRenderData().samples;
	}

	virtual bool IsLogarithmicDepth() const override
	{
		return mFinal->GetRenderData().logDepth;
	}

	virtual bool IsReverseDepthRange() const override
	{
		return mFinal->GetRenderData().nvDepth;
	}

	virtual bool							UseCameraDOFSettings() const override
	{
		return mFinal->GetRenderData().useDOF;
	}

	virtual FBCameraFocusDistanceSource		FocusDistanceSource() const override
	{
		return mFinal->GetRenderData().focusDistanceSource;
	}

	virtual double							FocusSpecificDistance() const override
	{
		return mFinal->GetRenderData().focusSpecificDistance;
	}

	virtual double							FocusAngle() const override
	{
		return mFinal->GetRenderData().focusAngle;
	}

	virtual const char *GetRenderSamplesDefineString() override
	{
		bool samplesMode = mFinal->GetRenderData().ms;
		int samples = mFinal->GetRenderData().samples;

		SetSamplesDefine(samplesMode, samples);
		return mStrSamplesDefine;
	}
	virtual const char *GetDepthBufferDefineString() const override
	{
		if (mFinal->GetRenderData().logDepth)
			return "#define DEPTH_LOG\n";
		else if (mFinal->GetRenderData().nvDepth)
			return "#define DEPTH_NV\n";

		return "";
	}

	virtual bool IsShadowMultisampling() const override
	{
		return false;
	}
	virtual int GetShadowSamplesCount() const override
	{
		return 8;
	}


	/*
	virtual const GLuint	GetRenderColorId() const override
	{
		return mFinal->GetRenderData().colorId;
	}
	virtual const GLuint	GetRenderDepthId() const override
	{
		return mFinal->GetRenderData().depthId;
	}
	virtual const GLuint	GetRenderNormalId() const override
	{
		return mFinal->GetRenderData().normalId;
	}
	virtual const GLuint	GetRenderMaskId() const override
	{
		return mFinal->GetRenderData().maskId;
	}
	virtual const GLuint	GetRenderPositionId() const override
	{
		return mFinal->GetRenderData().positionId;
	}

	// get multisample version of a texture (used to SSAO, toon shading, etc.)

	virtual const GLuint	GetRenderColorMSId() const override
	{
		return mFinal->GetRenderData().colorMSId;
	}
	virtual const GLuint	GetRenderDepthMSId() const override
	{
		return mFinal->GetRenderData().depthMSId;
	}
	virtual const GLuint	GetRenderNormalMSId() const override
	{
		return mFinal->GetRenderData().normalMSId;
	}
	virtual const GLuint	GetRenderMaskMSId() const override
	{
		return mFinal->GetRenderData().maskMSId;
	}
	virtual const GLuint	GetRenderPositionMSId() const override
	{
		return mFinal->GetRenderData().positionMSId;
	}
	*/


	virtual const MainFrameBuffer *GetFrameBufferInfo(ERenderLayer layerId) const override
	{
		if (nullptr != mFinal)
			return mFinal->GetRenderData().frameBuffers[layerId];
		return nullptr;
	}

protected:

	ObjectComposition	*mFinal;


	int		mProcessingWidth;
	int		mProcessingHeight;

	bool			mLastMultisampleMode;
	int				mLastNumberOfSamples;
	char			mStrSamplesDefine[256];

	FBMatrix		mCameraMVP;

	void SetSamplesDefine(bool mode, int samples)
	{
		if (mode != mLastMultisampleMode || samples != mLastNumberOfSamples)
		{
			for (int i=0; i<256; ++i)
				mStrSamplesDefine[i] = 0;

			if (mode)
			{
				sprintf_s( mStrSamplesDefine, 256*sizeof(char),
					"#define USE_MS\n#define SAMPLES %d\n", samples );
			}

			mLastMultisampleMode = mode;
			mLastNumberOfSamples = samples;
		}
	}
};

/************************************************
 *	Constructor.
 ************************************************/

void ObjectComposition::SetOutput(HIObject object, bool value)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		/*
		if (value)
		{
			// switch off "render output" property for all other existing compositions
			SetOutputCompositePtr(pFinal);
		}
		else if (pFinal == GetOutputCompositePtr() )
		{
			ClearOutputCompositePtr();
		}
		*/
		pFinal->OutputToRender.SetPropertyValue(value);
	}
}


void ObjectComposition::SetUpdate(HIObject object, bool value)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal && value) 
	{
		pFinal->NeedUpdate();
	}
}

void ObjectComposition::SetSizeFromBackground(HIObject object, bool value)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal && value) 
	{
		//pFinal->SizeControl = eCompositionSizeCustom;
		//pFinal->UserWidth = pFinal->GetBackgroundWidth();
		//pFinal->UserHeight = pFinal->GetBackgroundHeight();
		pFinal->mBackground->SetUserSize(pFinal->GetBackgroundWidth(), pFinal->GetBackgroundHeight());
	}
}
			
void ObjectComposition::SetQuality(HIObject object, ECompositionQuality value)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		pFinal->Quality.SetPropertyValue(value);
	}
}

void ObjectComposition::SetUserWidth(HIObject object, int value)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		//if (pFinal->SizeControl == eCompositionSizeCustom)
		//	pFinal->SetInputWidthValue(value);
		//pFinal->UserWidth.SetPropertyValue(value);
	}
}

void ObjectComposition::SetUserHeight(HIObject object, int value)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		/*
		if (pFinal->SizeControl == eCompositionSizeCustom)
			pFinal->SetInputHeightValue(value);
		pFinal->UserHeight.SetPropertyValue(value);
		*/
	}
}

int ObjectComposition::GetWidth(HIObject object)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		return pFinal->GetWidthValue();
	}
	return 1;
}

int ObjectComposition::GetHeight(HIObject object)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		return pFinal->GetHeightValue();
	}
	return 1;
}

int ObjectComposition::GetStatsWidth(HIObject object)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		return pFinal->GetStats().w;
	}
	return 1;
}

int ObjectComposition::GetStatsHeight(HIObject object)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		return pFinal->GetStats().h;
	}
	return 1;
}

int ObjectComposition::GetStatsNumberOfComputeShaders(HIObject object)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		return pFinal->GetStats().computeShaders;
	}
	return 1;
}

int ObjectComposition::GetStatsNumberOfDispatchGroups(HIObject object)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		return pFinal->GetStats().numberOfDispatchGroups;
	}
	return 1;
}

int ObjectComposition::GetStatsNumberOfTextures(HIObject object)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		return pFinal->GetStats().numberOfTextures;
	}
	return 1;
}

int ObjectComposition::GetStatsTexturesMemory(HIObject object)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		return (int) pFinal->GetStats().texturesMemoryMb;
	}
	return 0;
}

/*
static void CompositeFinal_SetSamples(HIObject object, int pValue)
{
	ObjectComposition *pFinal = FBCast<ObjectComposition>(object);
	if (pFinal) 
	{
		int value = pValue;
		if (value < 0) value = 1;
		if (value > 64) value = 64;

		int i=1;
		while (i < value)
			i *= 2;
		value = i;
		if (value > 64) value = 64;

		//pFinal->RenderSamples.SetPropertyValue(value);
	}
}
*/

void AddPropertyViewForCompositionFinal(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(COMPOSITEFINAL__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectComposition::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForCompositionFinal("", "Statistics", true);

	for (int i=0; i<eCompositionStatsCount; ++i)
		AddPropertyViewForCompositionFinal( stats_names[i], "Statistics" );

	AddPropertyViewForCompositionFinal("", "Background", true);
	AddPropertyViewForCompositionFinal("Background Width", "Background");
	AddPropertyViewForCompositionFinal("Background Height", "Background");
	AddPropertyViewForCompositionFinal("Background Color", "Background");
	AddPropertyViewForCompositionFinal("Background Alpha", "Background");
	AddPropertyViewForCompositionFinal("Use Background Gradient", "Background");
	AddPropertyViewForCompositionFinal("Background Upper Color", "Background");
	AddPropertyViewForCompositionFinal("Background Upper Alpha", "Background");
	AddPropertyViewForCompositionFinal("Background Lower Color", "Background");
	AddPropertyViewForCompositionFinal("Background Lower Alpha", "Background");
	//AddPropertyViewForCompositionFinal("Use Background Texture", "Background");
	AddPropertyViewForCompositionFinal("Background Texture", "Background");
	AddPropertyViewForCompositionFinal("Back Texture Width", "Background");
	AddPropertyViewForCompositionFinal("Back Texture Height", "Background");

	AddPropertyViewForCompositionFinal("", "Size Control", true);
	AddPropertyViewForCompositionFinal("Override Size", "Size Control");
	AddPropertyViewForCompositionFinal("User Width", "Size Control");
	AddPropertyViewForCompositionFinal("User Height", "Size Control");
	AddPropertyViewForCompositionFinal("ReSize Factor", "Size Control");

	AddPropertyViewForCompositionFinal("", "Batch processing", true);
	AddPropertyViewForCompositionFinal("Use For Batch Processing", "Batch processing");
	AddPropertyViewForCompositionFinal("Batch input", "Batch processing");
	AddPropertyViewForCompositionFinal("Batch output", "Batch processing");

	AddPropertyViewForCompositionFinal("Input", "");
	AddPropertyViewForCompositionFinal("Out Width", "");
	AddPropertyViewForCompositionFinal("Out Height", "");
}

ObjectComposition::ObjectComposition( const char* pName, HIObject pObject ) 
	: ObjectCompositeBase( pName, pObject )
	, mBackground( new CompositeBackground() )
	//, mBufferNormals( FBO_WIDTH, FBO_HEIGHT, FrameBuffer::depth32, 2 )
{
	FBClassInit;

	// hide layer properties
	
	Opacity.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );

	UseCompositeMask.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
	SelectCompositeMask.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
	InvertCompositeMask.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
	CustomMask.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );

	// renderoutput is an exclusive option (only one composition can be outputed)

	FBPropertyPublish(this, OutputToRender, "Output To Render", nullptr, SetOutput);
	//FBPropertyPublish(this, RenderAutoSize, "Render Auto Size", nullptr, nullptr);

	FBPropertyPublish(this, Quality, "Quality", nullptr, nullptr );
	
	//FBPropertyPublish(this, SizeControl, "Size Control", nullptr, nullptr );
	FBPropertyPublish(this, SizeFromBackground, "Size From Back Texture", nullptr, SetSizeFromBackground);

	FBPropertyPublish(this, ManualUpdate, "Manual Update", nullptr, nullptr );
	FBPropertyPublish(this, Update, "Update", nullptr, SetUpdate );

	//FBPropertyPublish(this, UserWidth, "User Width", nullptr, SetUserWidth );
	//FBPropertyPublish(this, UserHeight, "User Height", nullptr, SetUserHeight );

	FBPropertyPublish(this, Cameras, "Cameras", nullptr, nullptr);
	FBPropertyPublish(this, UseSceneLights, "Use Scene Lights", nullptr, nullptr);
	FBPropertyPublish(this, Lights, "Lights", nullptr, nullptr);
	FBPropertyPublish(this, Groups, "Groups", nullptr, nullptr);
	FBPropertyPublish(this, Video, "Video", nullptr, nullptr);

	FBPropertyPublish(this, UseForBatchProcessing, "Use For Batch processing", nullptr, nullptr);
	//FBPropertyPublish(this, BatchAutoSize, "Batch auto size", nullptr, nullptr);

	FBPropertyPublish(this, BatchInput, "Batch input", nullptr, nullptr);
	FBPropertyPublish(this, BatchOutput, "Batch output", nullptr, nullptr);

	//
	//

	FBPropertyPublish(this, TypeInfoComposition, "Type Info Composition", nullptr, nullptr);
	FBPropertyPublish(this, TypeInfoLayer, "Type Info Layer", nullptr, nullptr);
	FBPropertyPublish(this, TypeInfoFilter, "Type Info Filter", nullptr, nullptr);
	FBPropertyPublish(this, TypeInfoMarker, "Type Info Marker", nullptr, nullptr);

	TypeInfoComposition.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	TypeInfoComposition.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );

	TypeInfoLayer.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	TypeInfoLayer.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );

	TypeInfoFilter.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	TypeInfoFilter.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );

	TypeInfoMarker.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	TypeInfoMarker.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );

	TypeInfoComposition = ObjectComposition::TypeInfo;
	TypeInfoLayer = ObjectCompositeLayer::TypeInfo;
	TypeInfoFilter = ObjectCompositeFilter::TypeInfo;
	TypeInfoMarker = ObjectCompositeMarker::TypeInfo;

	//
	// stats properties

	

	fbExternalGetSetHandler	stats_handlers[eCompositionStatsCount] = {
		(fbExternalGetSetHandler) GetStatsWidth,
		(fbExternalGetSetHandler) GetStatsHeight,
		(fbExternalGetSetHandler) GetStatsNumberOfComputeShaders,
		(fbExternalGetSetHandler) GetStatsNumberOfDispatchGroups,
		(fbExternalGetSetHandler) GetStatsNumberOfTextures,
		(fbExternalGetSetHandler) GetStatsTexturesMemory
	};

	for (int i=0; i<eCompositionStatsCount; ++i)
	{
		FBPropertyPublish(this, mCompositionStats[i], stats_names[i], stats_handlers[i], nullptr);
		mCompositionStats[i].ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	}

	mBackground->Init(this);
	mBackground->Input->SetInt( eCompositionInputRender );

	UseForBatchProcessing = false;
	//BatchAutoSize = false;

	Cameras.SetSingleConnect(false);
	Cameras.SetFilter( FBCamera::GetInternalClassId() );
	UseSceneLights = false;
	//Lights.SetSingleConnect(false);
	//Lights.SetFilter( FBLight::GetInternalClassId() );
	Groups.SetSingleConnect(false);
	Groups.SetFilter( FBGroup::GetInternalClassId() );

	Video.SetFilter( FBVideoMemory::GetInternalClassId() );
	Video.SetSingleConnect(true);
	Video.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	BatchInput.SetFilter( FBVideo::GetInternalClassId() );
	BatchInput.SetSingleConnect(false);
	BatchOutput.SetFilter( FBVideoMemory::GetInternalClassId() );
	BatchOutput.SetSingleConnect(false);

	Quality = eCompositionQualityFull;
	mLastQuality = eCompositionQualityFull;

	//SizeControl = eCompositionSizeInput;
	//mLastSizeControl = eCompositionSizeInput;

	OutputToRender = true;
	//RenderAutoSize = false;

	mNeedUpdate = true;

	mBatchBufferId = 0;
	/*
	mRenderX = 0;
	mRenderY = 0;
	mRenderWidth = 1;
	mRenderHeight = 1;
	*/
	mBatchMaxWidth = 1;
	mBatchMaxHeight = 1;

	mHasANewSize = true;
	mTryToSetWidth = 512;
	mTryToSetHeight = 512;

	//UserWidth = 512;
	//UserHeight = 512;

	mSystem.OnUIIdle.Add( this, (FBCallback) &ObjectComposition::EventIdle );

	mInfo.reset( new CObjectCompositionInfoImpl(this) );
}

void ObjectComposition::PrepVideoOutput()
{
	if (Video.GetCount() == 0)
	{

		// add a new texture and memory video clip and connect them
		FBTexture *pTexture = new FBTexture("CompositeTexture");
		FBVideoMemory *pVideo = new FBVideoMemory("CompositeVideo");

		FBFolder *pFolderTextures = nullptr;
		FBFolder *pFolderVideos = nullptr;

		FBScene *pScene = FBSystem::TheOne().Scene;
		const int numberOfFolders = pScene->Folders.GetCount();
		for (int i=0; i<numberOfFolders; ++i)
		{
			if (strcmp(pScene->Folders[i]->Name, "Composite Textures") == 0)
				pFolderTextures = pScene->Folders[i];
			else
			if (strcmp(pScene->Folders[i]->Name, "Composite Videos") == 0)
				pFolderVideos = pScene->Folders[i];

			//
			if (pFolderTextures && pFolderVideos)
				break;
		}

		if (pFolderTextures == nullptr)
			pFolderTextures = new FBFolder( "Composite Textures", pTexture );
		else
			pFolderTextures->Items.Add( pTexture );

		if (pFolderVideos == nullptr)
			pFolderVideos = new FBFolder( "Composite Videos", pVideo );
		else
			pFolderVideos->Items.Add(pVideo);

		//
		pVideo->TextureOGLId = 0;
		pVideo->SetObjectImageSize(512, 512);

		//
		pTexture->Video = pVideo;

		FBConnect(this, pVideo);
		FBConnect(pVideo, &Video);
	}

	CHECK_GL_ERROR_MOBU();
}

void ObjectComposition::EventIdle		( HISender pSender, HKEvent pEvent )
{
	PrepVideoOutput();

	mSystem.OnUIIdle.Remove( this, (FBCallback) &ObjectComposition::EventIdle );
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectComposition::FBCreate()
{
	//FBScene *pScene = mSystem.Scene;
	//FBConnect(this, pScene);

	FBScene *pScene = mSystem.Scene;
	int index = Parents.Find(pScene);
	if (index < 0)
		Parents.Add(pScene);

	return ParentClass::FBCreate();
}

void ObjectComposition::FBDestroy()
{
	mSystem.OnUIIdle.Remove( this, (FBCallback) &ObjectComposition::EventIdle );
	ParentClass::FBDestroy();
}

void ObjectComposition::FBDelete()		
{
	mSystem.OnUIIdle.Remove( this, (FBCallback) &ObjectComposition::EventIdle );
	/*
	if (OutputToRender == true)
	{
		ClearOutputCompositePtr();
	}
	*/
	// delete dynamic video and texture data
	if (Video.GetCount() > 0 )
	{
		FBVideo *pVideo = (FBVideo*) Video.GetAt(0);
		
		if (pVideo)
		{
			int count = pVideo->GetDstCount();

			for (int i=count-1; i>=0; --i)
			{
				if (FBIS(pVideo->GetDst(i), FBTexture))
				{
					FBTexture *pTexture = (FBTexture*) pVideo->GetDst(i);
					pTexture->FBDelete();
				}
			}

			pVideo->FBDelete();
		}
	}

	ParentClass::FBDelete(); 
}

bool ObjectComposition::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	/*
	if (pAction == kFBCandidated && pThis == &SizeControl)
	{
		mBackground->NeedToUpdateShaderDefines();
	}
	*/
	if (pAction == kFBCandidated && pThis == &UseSceneLights)
	{
		AskToUpdateLightList();
	}
	mBackground->PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);

	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}


bool ObjectComposition::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	if (pThis == &Cameras)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectDst(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectDst(pPlug);
		}
	}
	else if (pThis == &Lights)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectDst(pPlug);
			AskToUpdateLightList();
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectDst(pPlug);
			AskToUpdateLightList();
		}
	}
	else if (pThis == &Groups)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectDst(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectDst(pPlug);
		}
	}
	else if (pThis == &Video)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectDst(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectDst(pPlug);
		}
	}
	else if (pThis == &BatchInput)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
			ComputeBatchMaxSizes();
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
			ComputeBatchMaxSizes();
		}
	}
	else if (pThis == &BatchOutput)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectDst(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectDst(pPlug);
		}
	}

	mBackground->PlugNotifyBackground(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
/*
void ObjectComposition::FBDestroy()
{
	
}
*/
/*
bool ObjectComposition::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ObjectComposition::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if( pStoreWhat == kAttributes )
	{
	}
	return true;
}
*/
void ObjectComposition::CopyFrom(ObjectCompositeBase *pNode)
{
	if ( FBIS(pNode, ObjectComposition) )
	{
		ObjectComposition *pFinal = (ObjectComposition*) pNode;

		PropertyCopy( Active, pFinal->Active );
		PropertyCopy( OutputToRender, pFinal->OutputToRender );

		//PropertyCopy( UserWidth, pFinal->UserWidth );
		//PropertyCopy( UserHeight, pFinal->UserHeight );
	}
}

void ObjectComposition::InitBatchTextures()
{
}

void ObjectComposition::ClearBatchTextures()
{
	if (mBatchBufferId)
	{
		glDeleteTextures(BatchInput.GetCount(), &mBatchBufferId);
		mBatchBufferId = 0;
	}
}

void ObjectComposition::OnActiveChange(const bool value)
{
}


void ObjectComposition::PrepareTexturesForBatch(const int w, const int h)
{
	const int count = BatchInput.GetCount();

	if (count == 0)
	{
		FreeBatchTextures();
	}
	else
	{
		if (count != mNumberOfBatchTextures)
		{
			FreeBatchTextures();
		}

		if (mBatchBufferId == 0)
		{
			glGenTextures( count, &mBatchBufferId );

			for (int i=0; i<count; ++i)
			{
				glBindTexture(GL_TEXTURE_2D, mBatchBufferId + i);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		else if (w != mBatchTextureWidth || h != mBatchTextureHeight)
		{
			for (int i=0; i<count; ++i)
			{
				glBindTexture(GL_TEXTURE_2D, mBatchBufferId + i);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		mBatchTextureWidth = w;
		mBatchTextureHeight = h;
		mNumberOfBatchTextures = count;
	}

	CHECK_GL_ERROR_MOBU();
}

void ObjectComposition::FreeBatchTextures()
{
	if (mBatchBufferId) {
		glDeleteTextures( BatchInput.GetCount(), &mBatchBufferId );
		mBatchBufferId = 0;
	}

	mNumberOfBatchTextures = 0;
	mBatchTextureWidth = 1;
	mBatchTextureHeight = 1;

	CHECK_GL_ERROR_MOBU();
}


void ObjectComposition::Render()
{
	CHECK_GL_ERROR_MOBU();

		//
	
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	
	if (Active == false) 
		return;

	bool anyBlendLayer = false;
	const int srccount = GetSrcCount();

	for (int i=0; i<srccount; ++i)
	{
		FBComponent *pSrc = (FBComponent*) GetSrc(i);

		if (FBIS(pSrc, ObjectCompositeLayer) )
		{
			ObjectCompositeLayer *pLayer = (ObjectCompositeLayer*) pSrc;

			//if (false == ((ObjectCompositeLayer*) pSrc)->ReadyToApply(mInfo.get()) )
			//	continue;

			if (true == pLayer->Active && pLayer->Opacity > 0.0)
				anyBlendLayer = true;
		}
		else
		if (FBIS(pSrc, ObjectCompositeFilter) )
		{
			//if (false == ((ObjectCompositeBase*) pSrc)->ReadyToApply(mInfo.get()) )
			//	continue;

			// TODO: do we need check and copy background here ??!!
			ObjectCompositeFilter *pFilter = (ObjectCompositeFilter*) pSrc;

			if (true == pFilter->Active && pFilter->Opacity > 0.0)
				anyBlendLayer = true;
		}
	}


	const bool allowToUseDirectTextureId = (true == anyBlendLayer);

	ECompositionQuality currQuality = Quality;
	mPreviewSizeFactor = 1.0f;

	if (true == mRenderData.showAsPreview)
		currQuality = eCompositionQualityPreview2;

	switch(currQuality)
	{
	case eCompositionQualityPreview4:
		mPreviewSizeFactor = 0.25f;
		break;
	case eCompositionQualityPreview2:
		mPreviewSizeFactor = 0.5f;
		break;
	}

	//PrepRender();
	if (false == mBackground->PrepRender(mInfo.get(), allowToUseDirectTextureId, OutputToRender, mPreviewSizeFactor) )
	{
		Active = false;
		return;
	}

	const int processingW = mBackground->GetProcessingWidth();
	const int processingH = mBackground->GetProcessingHeight();

	if (false == mTextureBuffer.IsTexturesReady() || false == mTextureBackground.IsTexturesReady() 
		|| mProcessingWidth != processingW || mProcessingHeight != processingH )
	{
		// DONE: add preview quality 4 times less image size
			
		mTextureBackground.ReSizeLayersData(processingW, processingH);	// TODO: ?! texture background could be a smaller size !
		mTextureBuffer.ReSizeLayersData(processingW, processingH);
		mBackground->NeedUpdate();

		mProcessingWidth = processingW;
		mProcessingHeight = processingH;
		mNeedUpdate = true;
	}

	//

	if (ManualUpdate && mNeedUpdate == false)
		return;

	CHECK_GL_ERROR_MOBU();

	mStats.Clear();

	if (UseForBatchProcessing && (BatchInput.GetCount() > 0) )
	{
		if (BatchInput.GetCount() != BatchOutput.GetCount() )
		{
			// TODO: sync input and output textures
		}

		RenderVideoBatches();
	}
	else
	{
		// count the 2 texture buffers and 1 more for background
		mStats.CountTextures( 3, mProcessingWidth * mProcessingHeight * 4 );

		RenderLayers(allowToUseDirectTextureId);

		// output
		if (Video.GetCount() > 0)
		{
			if ( FBIS(Video.GetAt(0), FBVideoMemory) )
			{
				FBVideoMemory *pVideo = (FBVideoMemory*) Video.GetAt(0);
				pVideo->SetObjectImageSize(mProcessingWidth, mProcessingHeight);
				pVideo->TextureOGLId = mTextureBuffer.GetCurrentTextureId();
			}
		}
	}

	mNeedUpdate = false;

	CHECK_GL_ERROR_MOBU();

	mStats.UpdateTextureMemoryMb();
	UpdateStatsProps();
}


void ObjectComposition::RenderLayers(bool allowToReturnDirectTextureId)
{
	
	//bool shaderOk = true;
	GLuint backgroundId = mBackground->ComputeTexture( mInfo.get(), mStats, mTextureBuffer.GetCurrentTextureId(), allowToReturnDirectTextureId );
	
	CHECK_GL_ERROR_MOBU();

	// lastTextureId - buffer from TextureBuffer with a size = [renderWidth, renderHeight], processing size

	// apply each effect and blend with each layer
	const int srccount = GetSrcCount();
	const CProcessingInfo	prInfo(mProcessingWidth, mProcessingHeight);

	for (int i=0; i<srccount; ++i)
	{
		FBComponent *pSrc = (FBComponent*) GetSrc(i);

		if (FBIS(pSrc, ObjectCompositeLayer) )
		{
			ObjectCompositeLayer *pLayer = (ObjectCompositeLayer*) pSrc;
			if (false == pLayer->ReadyToApply(mInfo.get(), prInfo) )
				continue;

			const GLuint operand1 = (backgroundId > 0) ? backgroundId : mTextureBuffer.GetCurrentTextureId();
//			const GLuint operand2 = pLayer->ComputeLayerTexture(mInfo.get(), mStats);
			const GLuint result = mTextureBuffer.QueryATextureBuffer();

			pLayer->BlendLayers(mInfo.get(), prInfo, mStats, operand1, result);

			// use backgroundId only once
			backgroundId = 0;
		}
		else
		if (FBIS(pSrc, ObjectCompositeFilter) )
		{
			if (false == ((ObjectCompositeBase*) pSrc)->ReadyToApply(mInfo.get(), prInfo) )
				continue;

			// 1- currentTextureId
			const GLuint currTextureId = (backgroundId > 0) ? backgroundId : mTextureBuffer.GetCurrentTextureId();
			const GLuint newTextureId = mTextureBuffer.QueryATextureBuffer();

			// TODO: do we need check and copy background here ??!!

			((ObjectCompositeFilter*) pSrc)->ApplyFilter(mInfo.get(), prInfo, mStats, currTextureId, newTextureId);

			backgroundId = 0;
		}
	}

	CHECK_GL_ERROR_MOBU();
}

void ObjectComposition::ChangeContext()
{
	ParentClass::ChangeContext();
	
	mBackground->ChangeContext();

	//
	ClearBatchTextures();

}

void ObjectComposition::DoReloadShader()
{
	mBackground->NeedReload();
}

void ObjectComposition::RenderVideoBatches()
{
	if (Active == false || UseForBatchProcessing == false)
		return;

//	const bool useMask = UseCompositeMask;

	for (int nBatch=0; nBatch < BatchInput.GetCount(); ++nBatch)
	{
		/*
		// replace input image
		ReplaceImageSource( this, (FBTexture*) BatchInput.GetAt(nBatch) );

		ObjectCompositeBase::Render();

		mShaderMan.Bind( eCompositeShaderBlit, useMask );

		// show result - should be only one input connection
		if (Components.GetCount() )
		{
			GLuint texId = ((ObjectCompositeBase*) Components[0])->getColorTextureId();
			//texId = color_buffer_id;

			int w = Width;
			int h = Height;

			mBuffer->Bind();
			mBuffer->AttachTexture2D(GL_TEXTURE_2D, batch_buffer_id + nBatch );
								
			glBindTexture(GL_TEXTURE_2D, texId);
			drawOrthoQuad2d(w, h);

			mBuffer->UnBind();

			// bind to a dynamic texture
			FBTexture *pOutTexture = (FBTexture*) BatchOutput.GetAt(nBatch);
			FBVideoMemory *vidMem = (FBVideoMemory*) (FBVideo*) pOutTexture->Video;
			vidMem->SetObjectImageSize(Width, Height);

			if (batch_buffer_id)
				vidMem->TextureOGLId = batch_buffer_id + nBatch;
		}

		FetchImageSource( this );

		CHECK_GL_ERROR_MOBU();

		mShaderMan.UnBind();
		*/
	}
}

void ObjectComposition::SetInputWidthValue(const int value)
{
	mTryToSetWidth = value;
	mHasANewSize = true;
}

void ObjectComposition::SetInputHeightValue(const int value)
{
	mTryToSetHeight = value;
	mHasANewSize = true;
}

void ObjectComposition::SetInputWidthHeight(const int w, const int h)
{
	mTryToSetWidth = w;
	mTryToSetHeight = h;

	mHasANewSize = true;
}

const int ObjectComposition::GetBackgroundWidth() const
{
	if ( mBackground->GetTextureId() > 0 )
	{
		return mBackground->GetTextureWidth();
	}

	return mBackground->BackgroundWidth->AsInt(); // UserWidth;
}

const int ObjectComposition::GetBackgroundHeight() const
{
	if ( mBackground->GetTextureId() > 0 )
	{
		return mBackground->GetTextureHeight();
	}

	return mBackground->BackgroundHeight->AsInt(); // UserHeight;
}


void ObjectComposition::CopyTexture(const GLuint srcLayerId, const GLuint dstLayerId)
{
	// mStats.CountComputeShader(1, n);
}

void ObjectComposition::ComputeBatchMaxSizes()
{
	mBatchMaxWidth = 1;
	mBatchMaxHeight = 1;

	for (int i=0, count=BatchInput.GetCount(); i<count; ++i)
	{
		FBTexture *pTexture = (FBTexture*) BatchInput.GetAt(i);
		FBVideo *pVideo = pTexture->Video;

		if (pVideo && FBIS(pVideo, FBVideoClip) )
		{
			FBVideoClip *pVideoClip = (FBVideoClip*) pVideo;

			mBatchMaxWidth = std::max(mBatchMaxWidth, pVideoClip->Width.AsInt() );
			mBatchMaxHeight = std::max(mBatchMaxHeight, pVideoClip->Height.AsInt() );
		}
	}
}

void ObjectComposition::UpdateStatsProps()
{
	for (int i=0; i<eCompositionStatsCount; ++i)
	{
		mCompositionStats[i].SetInt( mStats.GetStatsValue(i) );
	}
}

void ObjectComposition::EventBeforeRenderNotify()
{

	if ( false == UseSceneLights && Lights.GetCount() > 0 )
		mNeedUpdateLightList = true;

	if (mNeedUpdateLightList)
	{
		if (nullptr == mExclusiveLights.get() )
			mExclusiveLights.reset( new CGPUShaderLights() );

		CGPUFBScene::instance().PrepShaderLights( UseSceneLights, 
			&Lights, mLightsPtr, mExclusiveLights.get() );

		mNeedUpdateLightList = false;
	}
}