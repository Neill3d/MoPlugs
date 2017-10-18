
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_cubemap_rendering.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "model_cubemap_rendering.h"
#include "GL\glew.h"
#include "graphics\CheckGLError_MOBU.h"
#include "graphics\ParticlesDrawHelper.h"
#include "shared_content.h"
#include "nvImage.h"
#include "nv_dds.h"
#include "FBCommon.h"

#include "MB_renderer.h"
#include "compositeMaster_computeShaders.h"

FBClassImplementation( CameraRendering )
FBUserObjectImplement(CameraRendering, "Camera Rendering", FB_DEFAULT_SDK_ICON);				//Register UserObject class
FBElementClassImplementationRendering(CameraRendering, "Camera Rendering", FB_DEFAULT_SDK_ICON);					//Register to the asset system

FBClassImplementation( CubeMapRendering );								                //!< Register class
FBStorableCustomModelImplementation( CubeMapRendering, CUBEMAP_RENDERING__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementationRendering( CubeMapRendering, CUBEMAP_RENDERING__DESCSTR, FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system


extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);


///////////////////////////////////////////////////////////////////////////////
//

#define FBPropertyPublish2( Parent,Prop,PropName,Get,Set ) \
    { \
    Parent->PropertyAdd(Prop.CreateInternal( Parent,PropName,(fbExternalGetSetHandler)Get,(fbExternalGetSetHandler)Set )); \
    }

ExtendedRenderingBase::ExtendedRenderingBase()
{
	//! inside we have index for cubemap texture (should be zero from the beginning)
	memset( &mData, 0, sizeof(CubeMapRenderingData) );
	mData.maxOutputSize = 8192;
	mData.maxCubeMapSize = 2048;
}

void ExtendedRenderingBase::FBCreateCommonProps(FBComponent *pParent,
												fbExternalGetSetHandler OnClearActionClick,
												fbExternalGetSetHandler OnSaveActionClick,
												fbExternalGetSetHandler OnLoadActionClick,
												fbExternalGetSetHandler GetDrawNodeCount,
												fbExternalGetSetHandler GetGeomCacheCount,
												fbExternalGetSetHandler GetLastUpdateDuration,
												fbExternalGetSetHandler GetLastUpdateLocalTime)
{
	FBPropertyPublish2( pParent, Enabled, "Enabled", nullptr, nullptr );
	FBPropertyPublish2( pParent, DrawDebug, "Draw debug", nullptr, nullptr );

	//FBPropertyPublish2( pParent, Camera, "Camera", nullptr, nullptr );

	FBPropertyPublish2( pParent, NearRadius, "Near Radius", nullptr, nullptr );
	FBPropertyPublish2( pParent, FarRadius, "Far Radius", nullptr, nullptr );

	FBPropertyPublish2( pParent, BlurFactor, "Blur Factor", nullptr, nullptr );
	FBPropertyPublish2( pParent, Resolution, "Resolution", nullptr, nullptr );
	FBPropertyPublish2( pParent, UseFog, "Use Fog", nullptr, nullptr );
	FBPropertyPublish2( pParent, FogColor, "Fog Color", nullptr, nullptr );
	FBPropertyPublish2( pParent, FogOnBackground, "Fog On Background", nullptr, nullptr );

	FBPropertyPublish2( pParent, UseFile, "Use File", nullptr, nullptr );
	FBPropertyPublish2( pParent, FileName, "File Name", nullptr, nullptr );
	FBPropertyPublish2( pParent, Clear, "Clear", nullptr, OnClearActionClick ); // ModelCameraRendering_Clear );
	FBPropertyPublish2( pParent, Save, "Save", nullptr, OnSaveActionClick ); // ModelCameraRendering_Save );
	FBPropertyPublish2( pParent, Load, "Load", nullptr, OnLoadActionClick ); // ModelCameraRendering_Load );

	FBPropertyPublish2( pParent, IncludeList, "Include Object List", nullptr, nullptr );
	FBPropertyPublish2( pParent, ExcludeList, "Exclude Object List", nullptr, nullptr );
	FBPropertyPublish2( pParent, UseGPUCache, "Use GPU Cache", nullptr, nullptr );
	FBPropertyPublish2( pParent, RenderBackground, "Render Background", nullptr, nullptr );
	FBPropertyPublish2( pParent, RenderTransparency, "Render Transparency", nullptr, nullptr );
	FBPropertyPublish2( pParent, RenderSecondary, "Render Secondary", nullptr, nullptr );

	// stats information
	FBPropertyPublish2( pParent, DrawNodeCount, "Draw Node Count", GetDrawNodeCount, nullptr ); // GetDrawNodeCount, nullptr );
	FBPropertyPublish2( pParent, DrawGeomCacheCount, "Draw GeomCache Count", GetGeomCacheCount, nullptr ); //GetDrawGeomCacheCount, nullptr );
	FBPropertyPublish2( pParent, LastUpdateDuration, "Last Update Duration", GetLastUpdateDuration, nullptr ); // GetUpdateDuration, nullptr );
	FBPropertyPublish2( pParent, LastUpdateLocalTime, "Last Update LocalTime", GetLastUpdateLocalTime, nullptr ); // GetUpdateLocalTime, nullptr );

	FBPropertyPublish2( pParent, OutputWidth, "Output Width", nullptr, nullptr );
	FBPropertyPublish2( pParent, OutputHeight, "Output Height", nullptr, nullptr );

	FBPropertyPublish2( pParent, GenerateOutput, "Generate 2d output", nullptr, nullptr );
	FBPropertyPublish2( pParent, GenerateMipMaps, "Generate MipMaps", nullptr, nullptr );
	FBPropertyPublish2( pParent, VideoOut, "Video Output", nullptr, nullptr );

	Enabled = true;
	DrawDebug = true;

	NearRadius = 10.0;
	FarRadius = 4000.0;
	
	BlurFactor = 0.0;

	UseFog = true;
	FogColor = FBColor(0.5, 0.5, 0.5);
	FogOnBackground = false;

	UseFile = false;
	FileName = "";

	IncludeList.SetSingleConnect( false );
	IncludeList.SetFilter( FBModel::GetInternalClassId() );
	ExcludeList.SetSingleConnect( false );
	ExcludeList.SetFilter( FBModel::GetInternalClassId() );
	UseGPUCache = true;
	RenderBackground = true;
	RenderTransparency = true;
	RenderSecondary = true;

	OutputWidth = 512;
	OutputHeight = 512;

	GenerateOutput = true;
	GenerateMipMaps = false;
	VideoOut.SetSingleConnect(true);
	VideoOut.SetFilter( FBVideoMemory::GetInternalClassId() );

	Resolution.SetMinMax( 16.0, 4096.0, true, true );
	Resolution = 512.0;

	DrawNodeCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	DrawGeomCacheCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	LastUpdateDuration.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	LastUpdateLocalTime.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
}

ExtendedRenderingBase::~ExtendedRenderingBase()
{
}

FBVideoMemory *ExtendedRenderingBase::CreateDynamicVideo(const bool createTexture, const char *textureName, const char *videoName, const char *textureFolder, const char *videoFolder)
{
	// add a new texture and memory video clip and connect them
	FBTexture *pTexture = (createTexture) ? new FBTexture(textureName) : nullptr;
	FBVideoMemory *pVideo = new FBVideoMemory( videoName );

	FBFolder *pFolderTextures = nullptr;
	FBFolder *pFolderVideos = nullptr;

	FBScene *pScene = FBSystem::TheOne().Scene;
	const int numberOfFolders = pScene->Folders.GetCount();
	for (int i=0; i<numberOfFolders; ++i)
	{
		if (strcmp(pScene->Folders[i]->Name, textureFolder) == 0)
			pFolderTextures = pScene->Folders[i];
		else
		if (strcmp(pScene->Folders[i]->Name, videoFolder) == 0)
			pFolderVideos = pScene->Folders[i];

		//
		if (pFolderTextures && pFolderVideos)
			break;
	}

	if (pTexture)
	{
		if (pFolderTextures == nullptr)
			pFolderTextures = new FBFolder( textureFolder, pTexture );
		else
			pFolderTextures->Items.Add( pTexture );
	}

	if (pFolderVideos == nullptr)
		pFolderVideos = new FBFolder( videoFolder, pVideo );
	else
		pFolderVideos->Items.Add(pVideo);

	//
	pVideo->TextureOGLId = 0;
	pVideo->SetObjectImageSize(512, 512);

	//
	if (pTexture)
		pTexture->Video = pVideo;

	return pVideo;
}


void ExtendedRenderingBase::CommonClear()
{
	if (1 == FBMessageBox("Rendering Camera", "Do you want to clear last loaded image ?", "Yes", "Cancel") )
	{
		UseFile = false;
		FileName = "";
		FreeTextures();
	}
}

void ExtendedRenderingBase::CommonUpdate()
{
	// ask for manual rendering
	mData.needUpdateOutput = true;
}

void ExtendedRenderingBase::CommonPutSaveInQueue()
{
	mData.saveOutputMap = true;
}

void ExtendedRenderingBase::DoSave(const unsigned textureId)
{
}

void ExtendedRenderingBase::FreeTextures()
{
	if (mData.cubeMapId > 0)
	{
		glDeleteTextures(1, &mData.cubeMapId);
		mData.cubeMapId = 0;
	}
	if (mData.outputId > 0)
	{
		glDeleteTextures(1, &mData.outputId);
		mData.outputId = 0;
	}
	/*
	if (mData.depthId > 0)
	{
		glDeleteTextures(1, &mData.depthId);
		mData.depthId = 0;
	}
	*/
}

void ExtendedRenderingBase::CommonLoad()
{
}

void ExtendedRenderingBase::AssignResult(const unsigned textureId, const int width, const int height)
{
	if (VideoOut.GetCount() > 0)
	{
		FBVideo *pVideo = (FBVideo*) VideoOut.GetAt(0);
		
		if (pVideo && FBIS(pVideo, FBVideoMemory) )
		{
			FBVideoMemory *pMemory = (FBVideoMemory*) pVideo;

			pMemory->TextureOGLId = textureId;
			pMemory->SetObjectImageSize(width, height);
		}
	}
}

void ExtendedRenderingBase::PrepOutputTextureObject(const int newWidth, const int newHeight)
{

	if (mData.outputId == 0 
		|| newWidth != mData.outputWidth 
		|| newHeight != mData.outputHeight /*|| (allocDepth && (mData.depthId==0))*/ )
	{
		if (mData.outputId > 0)
		{
			glDeleteTextures(1, &mData.outputId);
			mData.outputId = 0;
		}
		if (mData.depthId > 0)
		{
			glDeleteTextures(1, &mData.depthId);
			mData.depthId = 0;
		}

		glGenTextures(1, &mData.outputId);
		FrameBuffer::CreateTexture2D(newWidth, newHeight, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, &mData.outputId, GL_CLAMP_TO_EDGE, FrameBuffer::filterMipmap, true);
		
		glGenTextures(1, &mData.depthId);
		FrameBuffer::CreateTexture2D(newWidth, newHeight, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT, &mData.depthId);
	

		mData.outputWidth = newWidth;
		mData.outputHeight = newHeight;

		AssignResult( mData.outputId, newWidth, newHeight );
		mData.needUpdateOutput = true;
	}
}


bool ExtendedRenderingBase::CommonCheckForUpdate(bool isCubeMap)
{
	bool needUpdate = false;

	// generate video and texture if not assigned yet
	// true == GenerateOutput && 
	if (0 == VideoOut.GetCount())
	{
		FBVideoMemory *pVideoMemory = CreateDynamicVideo(true, "Camera Texture", "Camera Video", "Dynamic Textures", "Dynamic Videos" );
		VideoOut.Add(pVideoMemory);
	}

	// TODO: check if we need to update resolution
	const int lResolution = Resolution;

	if (true == isCubeMap && lResolution != mData.cubeMapSize)
	{
		needUpdate = true;
		//mData.cubeMapSize = Resolution;
	}

	return needUpdate;
}

bool ExtendedRenderingBase::CheckForHiddenIncludeList()
{

	if (IncludeList.GetCount() > 0)
	{
		int visibleCount = 0;

		for (int i=0, count=IncludeList.GetCount(); i<count; ++i)
		{
			FBModel *pModel = (FBModel*) IncludeList[i];

			if (true == pModel->IsVisible() )
			{
				visibleCount += 1;
				return true;
			}
		}

		if (0 == visibleCount)
			return false;
	}

	return true;
}

/************************************************
*	Constructor.
************************************************/
CameraRendering::CameraRendering( const char* pName, HIObject pObject ) 
	: FBUserObject( pName, pObject )
	, ExtendedRenderingBase()
	, CFBComponentUpdateProps()
{
    FBClassInit;

}

void AddPropertyViewForCamera(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(CAMERA_RENDERING__CLASSSTR, pPropertyName, pHierarchy);
}

void CameraRendering::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForCamera( "Enabled", "" );
	AddPropertyViewForCamera( "Draw Debug", "" );
	
	AddPropertyViewForCamera( "Camera", "" );

	AddPropertyViewForCamera( "Common", "", true );
	AddPropertyViewForCamera( "Near Radius", "Common" );
	AddPropertyViewForCamera( "Far Radius", "Common" );
	AddPropertyViewForCamera( "Blur Factor", "Common" );
	AddPropertyViewForCamera( "Resolution", "Common" );

	AddPropertyViewForCamera( "Fog", "", true );
	AddPropertyViewForCamera( "Use Fog", "Fog" );
	AddPropertyViewForCamera( "Fog Color", "Fog" );
	AddPropertyViewForCamera( "Fog On Background", "Fog" );

	AddPropertyViewForCamera( "Evaluate", "", true );
	AddPropertyViewForCamera( "RealTime Update", "Evaluate" );
	AddPropertyViewForCamera( "Update On TimeSlider Change", "Evaluate" );
	AddPropertyViewForCamera( "Update When Selected", "Evaluate" );
	AddPropertyViewForCamera( "Update Skip Frames", "Evaluate" );
	AddPropertyViewForCamera( "Update", "Evaluate" );

	AddPropertyViewForCamera( "Elements", "", true );
	AddPropertyViewForCamera( "Include Object List", "Elements" );
	AddPropertyViewForCamera( "Exclude Object List", "Elements" );
	AddPropertyViewForCamera( "Use GPU Cache", "Elements" );
	AddPropertyViewForCamera( "Use Background", "Elements" );

	AddPropertyViewForCamera( "Parallax Correction", "", true );
	AddPropertyViewForCamera( "Use Parallax Correction", "Parallax Correction" );
	AddPropertyViewForCamera( "Room Max", "Parallax Correction" );
	AddPropertyViewForCamera( "Room Min", "Parallax Correction" );

	AddPropertyViewForCamera( "File Operations", "", true );
	AddPropertyViewForCamera( "Use File", "File Operations" );
	AddPropertyViewForCamera( "File Name", "File Operations" );
	AddPropertyViewForCamera( "Clear", "File Operations" );
	AddPropertyViewForCamera( "Save", "File Operations" );
	AddPropertyViewForCamera( "Load", "File Operations" );

	AddPropertyViewForCamera( "Video Output", "", true );
	AddPropertyViewForCamera( "Auto Size Output", "Video Output" );
	AddPropertyViewForCamera( "Output Width", "Video Output" );
	AddPropertyViewForCamera( "Output Height", "Video Output" );
	AddPropertyViewForCamera( "Generate 2d output", "Video Output" );
	AddPropertyViewForCamera( "Generate MipMaps", "Video Output" );
	AddPropertyViewForCamera( "Video Output", "Video Output" );
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool CameraRendering::FBCreate()
{
	FBPropertyPublish( this, Camera, "Camera", nullptr, nullptr );

	Camera.SetSingleConnect(true);
	Camera.SetFilter( FBCamera::GetInternalClassId() );

	FBCreateCommonProps(this,
		(fbExternalGetSetHandler) OnClearActionClick,
		(fbExternalGetSetHandler) OnSaveActionClick,
		(fbExternalGetSetHandler) OnLoadActionClick,
		(fbExternalGetSetHandler) GetDrawNodeCount,
		(fbExternalGetSetHandler) GetDrawGeomCacheCount,
		(fbExternalGetSetHandler) GetUpdateDuration,
		(fbExternalGetSetHandler) GetUpdateLocalTime );

	FBCreateUpdateProps( this,
		(fbExternalGetSetHandler) OnUpdateActionClick );

	//
	//

	CGPUFBScene *mGPUFBScene = &CGPUFBScene::instance();
	if (mGPUFBScene)
		mGPUFBScene->AddCameraNode(this);

	return ParentClass::FBCreate();
}

bool CameraRendering::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	if (pThis == &Camera || pThis == &IncludeList || pThis == &ExcludeList)
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
	else if (pThis == &VideoOut)
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

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}



/************************************************
*	FiLMBOX Destructor.
************************************************/
void CameraRendering::FBDestroy()
{
	CGPUFBScene *mGPUFBScene = &CGPUFBScene::instance();
	if (mGPUFBScene)
		mGPUFBScene->RemoveCameraNode(this);

	FreeTextures();

    ParentClass::FBDestroy();
}

void CameraRendering::OnUpdateActionClick(HIObject object, bool value)
{
	CameraRendering *pNode = FBCast<CameraRendering>(object);
	if (pNode && value) 
	{
		pNode->CommonUpdate();
		pNode->OnSetUpdateAction();
	}
}

void CameraRendering::OnSaveActionClick(HIObject object, bool value)
{
	CameraRendering *pNode = FBCast<CameraRendering>(object);
	if (pNode && value) 
	{
		pNode->CommonPutSaveInQueue();
	}
}

void CameraRendering::OnLoadActionClick(HIObject object, bool value)
{
	CameraRendering *pNode = FBCast<CameraRendering>(object);
	if (pNode && value) 
	{
		pNode->CommonLoad();
	}
}

void CameraRendering::OnClearActionClick(HIObject object, bool value)
{
	CameraRendering *pNode = FBCast<CameraRendering>(object);
	if (pNode && value) 
	{
		pNode->CommonClear();
	}
}

int CameraRendering::GetDrawNodeCount(HIObject object)
{
	CameraRendering *pNode = FBCast<CameraRendering>(object);
	if (pNode) 
	{
		return pNode->mStats.totalDrawNodeCount;
	}
	return 0;
}

int CameraRendering::GetDrawGeomCacheCount(HIObject object)
{
	CameraRendering *pNode = FBCast<CameraRendering>(object);
	if (pNode) 
	{
		return pNode->mStats.totalGeometryCacheCount;
	}
	return 0;
}

double CameraRendering::GetUpdateDuration(HIObject object)
{
	CameraRendering *pNode = FBCast<CameraRendering>(object);
	if (pNode) 
	{
		return pNode->mStats.totalUpdateDuration;
	}
	return 0.0;
}

FBTime CameraRendering::GetUpdateLocalTime(HIObject object)
{
	CameraRendering *pNode = FBCast<CameraRendering>(object);
	if (pNode) 
	{
		return pNode->mStats.updateLocalTime;
	}
	return FBTime::Zero;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CubeMap Rendering

/************************************************
*	Constructor.
************************************************/
CubeMapRendering::CubeMapRendering( const char* pName, HIObject pObject ) 
    : FBModelMarker( pName, pObject )
	, ExtendedRenderingBase()
	, CFBComponentUpdateProps()
{
    FBClassInit;

	mLastCubeMapFace = 0;

}

void AddPropertyViewForCubeMap(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(CUBEMAP_RENDERING__CLASSSTR, pPropertyName, pHierarchy);
}

void CubeMapRendering::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForCubeMap( "Enabled", "" );
	AddPropertyViewForCubeMap( "Draw Debug", "" );
	
	AddPropertyViewForCubeMap( "CubeMap Location", "" );

	AddPropertyViewForCubeMap( "Common", "", true );
	AddPropertyViewForCubeMap( "Near Radius", "Common" );
	AddPropertyViewForCubeMap( "Far Radius", "Common" );
	AddPropertyViewForCubeMap( "Blur Factor", "Common" );
	AddPropertyViewForCubeMap( "Resolution", "Common" );

	AddPropertyViewForCubeMap( "Fog", "", true );
	AddPropertyViewForCubeMap( "Use Fog", "Fog" );
	AddPropertyViewForCubeMap( "Fog Color", "Fog" );
	AddPropertyViewForCubeMap( "Fog On Background", "Fog" );

	AddPropertyViewForCubeMap( "Evaluate", "", true );
	AddPropertyViewForCubeMap( "RealTime Update", "Evaluate" );
	AddPropertyViewForCubeMap( "Update On TimeSlider Change", "Evaluate" );
	AddPropertyViewForCubeMap( "Update When Selected", "Evaluate" );
	AddPropertyViewForCubeMap( "Update Skip Frames", "Evaluate" );
	AddPropertyViewForCubeMap( "Update", "Evaluate" );

	AddPropertyViewForCubeMap( "Elements", "", true );
	AddPropertyViewForCubeMap( "Include Object List", "Elements" );
	AddPropertyViewForCubeMap( "Exclude Object List", "Elements" );
	AddPropertyViewForCubeMap( "Use GPU Cache", "Elements" );
	AddPropertyViewForCubeMap( "Use Background", "Elements" );
	AddPropertyViewForCubeMap( "Render Transparency", "Elements" );

	AddPropertyViewForCubeMap( "Parallax Correction", "", true );
	AddPropertyViewForCubeMap( "Use Parallax Correction", "Parallax Correction" );
	AddPropertyViewForCubeMap( "Room Max", "Parallax Correction" );
	AddPropertyViewForCubeMap( "Room Min", "Parallax Correction" );

	AddPropertyViewForCubeMap( "File Operations", "", true );
	AddPropertyViewForCubeMap( "Use File", "File Operations" );
	AddPropertyViewForCubeMap( "File Name", "File Operations" );
	AddPropertyViewForCubeMap( "Clear", "File Operations" );
	AddPropertyViewForCubeMap( "Save", "File Operations" );
	AddPropertyViewForCubeMap( "Load", "File Operations" );

	AddPropertyViewForCubeMap( "Video Output", "", true );
	AddPropertyViewForCubeMap( "Auto Size Output", "Video Output" );
	AddPropertyViewForCubeMap( "Output Width", "Video Output" );
	AddPropertyViewForCubeMap( "Output Height", "Video Output" );
	AddPropertyViewForCubeMap( "Generate 2d output", "Video Output" );
	AddPropertyViewForCubeMap( "Generate MipMaps", "Video Output" );
	AddPropertyViewForCubeMap( "Video Output", "Video Output" );

	AddPropertyViewForCubeMap( "Stats", "", true );
	AddPropertyViewForCubeMap( "Draw Node Count", "Stats" );
	AddPropertyViewForCubeMap( "Draw GeomCache Count", "Stats" );
	AddPropertyViewForCubeMap( "Last Update Duration", "Stats" );
	AddPropertyViewForCubeMap( "Last Update LocalTime", "Stats" );

}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool CubeMapRendering::FBCreate()
{
	
	//FBPropertyPublish( this, Point, "CubeMap Location", nullptr, nullptr );

	FBPropertyPublish( this, UseParallaxCorrection, "Use Parallax Correction", nullptr, nullptr );
	FBPropertyPublish( this, RoomMax, "Room Max", nullptr, nullptr );
	FBPropertyPublish( this, RoomMin, "Room Min", nullptr, nullptr );

	FBPropertyPublish( this, AutoSizeOutput, "Auto Size Output", nullptr, nullptr );
	
	//

	//Point.SetSingleConnect(true);
	//Point.SetFilter( FBModel::GetInternalClassId() );

	UseParallaxCorrection = false;
	RoomMax = FBVector3d(100.0, 100.0, 100.0);
	RoomMin = FBVector3d(-100.0, -100.0, -100.0);

	AutoSizeOutput = true;

	FBCreateCommonProps(this,
		(fbExternalGetSetHandler) OnClearActionClick,
		(fbExternalGetSetHandler) OnSaveActionClick,
		(fbExternalGetSetHandler) OnLoadActionClick,
		(fbExternalGetSetHandler) GetDrawNodeCount,
		(fbExternalGetSetHandler) GetDrawGeomCacheCount,
		(fbExternalGetSetHandler) GetUpdateDuration,
		(fbExternalGetSetHandler) GetUpdateLocalTime );

	FBCreateUpdateProps( this,
		(fbExternalGetSetHandler) OnUpdateActionClick );

	// don't generate panorama by default, use direct cubemap binding
	GenerateOutput = false;

	//
	//

	CGPUFBScene *mGPUFBScene = &CGPUFBScene::instance();
	if (mGPUFBScene)
		mGPUFBScene->AddCubeMapNode(this);

	Color = FBColor(128.0/255.0, 128.0/255.0, 0.0);

	return ParentClass::FBCreate();
}

/************************************************
*	FiLMBOX Destructor.
************************************************/
void CubeMapRendering::FBDestroy()
{
	CGPUFBScene *mGPUFBScene = &CGPUFBScene::instance();
	if (mGPUFBScene)
		mGPUFBScene->RemoveCubeMapNode(this);

	FreeTextures();

    ParentClass::FBDestroy();
}

/** Custom display
*/




void CubeMapRendering::CommonClear()
{
	if (1 == FBMessageBox("Rendering CubeMap", "Do you want to clear last loaded cubemap ?", "Yes", "Cancel") )
	{
		UseFile = false;
		FileName = "";
		FreeTextures();
	}
}

void CubeMapRendering::CommonPutSaveInQueue()
{
	mData.saveDynamicCubeMap = true;
}

void CubeMapRendering::DoSave(const unsigned textureId)
{
	// DONE: save to cubemap DDS
	// we save cubemap from the framebuffer or from last loaded cubemap
	/*
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	*/
	CHECK_GL_ERROR_MOBU();

	FBFilePopup	lDialog;
	lDialog.Style = kFBFilePopupSave;
	lDialog.Filter = "*.dds";
	lDialog.Caption = "Please choose a cubemap DDS file";

	if (lDialog.Execute() )
	{
		FileName = lDialog.FullFilename;
		
		// +X, -X, +Y, -Y, +Z, -Z
		nv_dds::CTexture cubemapTextures[6];

		bool isCompressed;
		int width, height, internalFormat, format, pixelMemorySize, imageSize, numberOfLods;

		pixelMemorySize = 3;
		format = GL_RGB;

		int imgsize=0;
		GLubyte *pixels = nullptr;

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// DONE: assign id
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

		for (int i=0; i<6; ++i)
		{

			TextureObjectGetInfo(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, width, height, internalFormat, format, isCompressed, pixelMemorySize, imageSize, numberOfLods);

			if (imageSize > 0)
			{
			
				// allocate mem
				if (imageSize != imgsize)
				{
					if (pixels)
					{
						delete [] pixels;
						pixels = nullptr;
					}

					pixels = new GLubyte[imageSize];
					imgsize = imageSize;
				}

				TextureObjectGetData(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, pixels, width, height, internalFormat, format, isCompressed, pixelMemorySize, imageSize, numberOfLods);
			}

			

			if (imgsize > 0 && pixels != nullptr)
			{
				cubemapTextures[i].create(width, height, pixelMemorySize, imgsize, pixels);
			}
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		CHECK_GL_ERROR_MOBU();

		//
		//
		//
	
		nv_dds::CDDSImage ddsImage;
		ddsImage.create_textureCubemap( format, pixelMemorySize, cubemapTextures[0], cubemapTextures[1], cubemapTextures[2], 
			cubemapTextures[3], cubemapTextures[4], cubemapTextures[5] );

		ddsImage.save( std::string(FileName) );		
	}

	CHECK_GL_ERROR_MOBU();
}

void CubeMapRendering::CommonLoad()
{
	// TODO: load from cubemap DDS

	FBFilePopup	lDialog;
	lDialog.Style = kFBFilePopupOpen;
	lDialog.Filter = "*.dds";
	lDialog.Caption = "Please choose a cubemap DDS file";

	if (lDialog.Execute() )
	{
		FileName = lDialog.FullFilename;

		nv::Image	image;
		bool bRes = image.loadImageFromFile(FileName);

		if (!bRes)
			return;

		if (image.getFaces() == 0)
			if (!image.convertCrossToCubemap() )
				return;

		if (image.isCubeMap() == false)
			return;

		// ? do we need to delete previous texture ?
		FreeTextures();

		//
		mData.staticCubeMapSize = image.getWidth();

		glGenTextures(1, &mData.cubeMapId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mData.cubeMapId);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

		// load face data
        GLenum internalFormat = image.getInternalFormat();
        for(int i=0; i<6; i++) 
        {
            int w = image.getWidth();
            int h = image.getHeight();
            //int d = image.getDepth();
            for(int l=0; l<image.getMipLevels(); l++)
            {
                GLenum imgfmt  = image.getFormat();
                GLenum imgtype = image.getType();
                void *imgdata = image.getLevel(l, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
                if(image.isCompressed())
                    glCompressedTexImage2DARB(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, l,
                    internalFormat, w, h, 0, image.getImageSize(l), imgdata);
                else
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, l,
                                internalFormat, w, h, 0, 
                                imgfmt, imgtype, 
                                imgdata);
                w /= 2; h /= 2;
            }
        } 

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		mData.needUpdateOutput = true;
	}
}

void CubeMapRendering::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
	FBViewingOptions* lViewingOptions = FBSystem::TheOne().Renderer->GetViewingOptions();
    bool lIsSelectBufferPicking = (pRenderPass == kFBModelRenderPassPick) || lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	if (lViewingOptions->PickingMode() >=  kFBPickingModeModelsOnly)
		return;

	const float size = 0.01f * (float) Size;

	vec4 fr[8]=
	{
		// near
		vec4(-size, -size, -size, 1.0f), vec4( size, -size, -size, 1.0f),
		vec4( size,  size, -size, 1.0f),  vec4(-size,  size, -size, 1.0f),
		// far
		vec4(-size, -size, size, 1.0f),	vec4( size, -size, size, 1.0f),	
		vec4( size,  size, size, 1.0f), vec4(-size,  size, size, 1.0f)
	};

	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_TRANSFORM_BIT | GL_LINE_BIT); //Push Current GL states.
    {
		FBColor currColor;

		if (!lIsSelectBufferPicking && !lIsColorBufferPicking) 
        { 
            if ((bool)Selected) 
            {
				currColor = FBColor( 0.0, 1.0, 0.0 );   //!< Otherwise, BLUE.
            } 
            else 
            {
				currColor = Color;
            }
        }
		else
		{
			currColor = UniqueColorId;
			glLoadName(1);
		}

        glLineWidth(3.0);    //!< Draw line wider to easy picking.
		glColor3dv(currColor);

		glPushMatrix();

		FBMatrix matrix;
		GetMatrix(matrix);
		glMultMatrixd(matrix);

		// check if have to draw in logarithmic depth

		glBegin(GL_LINES);

		glVertex3fv(fr[0].vec_array);
		glVertex3fv(fr[1].vec_array);

		glVertex3fv(fr[1].vec_array);
		glVertex3fv(fr[2].vec_array);

		glVertex3fv(fr[2].vec_array);
		glVertex3fv(fr[3].vec_array);

		glVertex3fv(fr[3].vec_array);
		glVertex3fv(fr[0].vec_array);

		glVertex3fv(fr[4].vec_array);
		glVertex3fv(fr[5].vec_array);

		glVertex3fv(fr[5].vec_array);
		glVertex3fv(fr[6].vec_array);

		glVertex3fv(fr[6].vec_array);
		glVertex3fv(fr[7].vec_array);

		glVertex3fv(fr[7].vec_array);
		glVertex3fv(fr[4].vec_array);

		glVertex3fv(fr[0].vec_array);
		glVertex3fv(fr[4].vec_array);

		glVertex3fv(fr[1].vec_array);
		glVertex3fv(fr[5].vec_array);

		glVertex3fv(fr[2].vec_array);
		glVertex3fv(fr[6].vec_array);

		glVertex3fv(fr[3].vec_array);
		glVertex3fv(fr[7].vec_array);

		/*
		connect tfr points as follow:
		0-1, 1-2, 2-3, 3-0, 
		4-5, 5-6, 6-7, 7-4,
		0-4, 1-5, 2-6, 3-7
		*/
		glEnd();

		glPopMatrix();

		// TODO: add a support for log depth drawing ?!
		if (true == DrawDebug && false == lIsSelectBufferPicking && false == lIsColorBufferPicking) 
		{
			GLSLShader *pCubeMapShader = nullptr;

			pCubeMapShader = CompositeComputeShader::CMixedProgramManager::instance().QueryCubeMapShader();
		
			if (pCubeMapShader != nullptr)
			{
				pCubeMapShader->Bind();
				pCubeMapShader->setUniformUINT( "cubeMap", 0 );
			}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, mData.cubeMapId);

			FBVector3d scl;
			GetVector(scl, kModelScaling);

			glPushMatrix();
			
			glTranslated(matrix[12], matrix[13], matrix[14]);
			glTranslatef(0.0f, 0.8f * size, 0.0f);
			glScaled(scl[0], scl[1], scl[2]);

			DrawSphere(0.2f * size, 16);
			glPopMatrix();

			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		
			if (pCubeMapShader != nullptr)
			{
				pCubeMapShader->UnBind();
			}
		}

		
	}
	glPopAttrib();
}

void CubeMapRendering::PrepOutputTextureObject(const int newWidth, const int newHeight)
{

	if (false == mData.useStaticCubeMap)
	{
		if (mData.cubeMapId == 0 || mData.cubeMapSize != newWidth 
			|| mData.depthForCubeMapId == 0)
		{

			if (mData.cubeMapId > 0)
			{
				glDeleteTextures(1, &mData.cubeMapId);
				mData.cubeMapId = 0;
			}
			if (mData.depthForCubeMapId > 0)
			{
				glDeleteTextures(1, &mData.depthForCubeMapId );
				mData.depthForCubeMapId = 0;
			}

			mData.cubeMapSize = newWidth;
			mData.cubeMapId = FrameBuffer::CreateTextureCube(mData.cubeMapSize, mData.cubeMapSize, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE );
			mData.depthForCubeMapId = FrameBuffer::CreateTexture2D(newWidth, newWidth, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT );
		}
	}

	int panoWidth = newWidth * 4;
	int panoHeight = newHeight * 2;
	
	ExtendedRenderingBase::PrepOutputTextureObject( panoWidth, panoHeight );
}


void CubeMapRendering::OnUpdateActionClick(HIObject object, bool value)
{
	CubeMapRendering *pNode = FBCast<CubeMapRendering>(object);
	if (pNode && value) 
	{
		pNode->CommonUpdate();
		pNode->OnSetUpdateAction();
	}
}

void CubeMapRendering::OnSaveActionClick(HIObject object, bool value)
{
	CubeMapRendering *pNode = FBCast<CubeMapRendering>(object);
	if (pNode && value) 
	{
		pNode->CommonPutSaveInQueue();
	}
}

void CubeMapRendering::OnLoadActionClick(HIObject object, bool value)
{
	CubeMapRendering *pNode = FBCast<CubeMapRendering>(object);
	if (pNode && value) 
	{
		pNode->CommonLoad();
	}
}

void CubeMapRendering::OnClearActionClick(HIObject object, bool value)
{
	CubeMapRendering *pNode = FBCast<CubeMapRendering>(object);
	if (pNode && value) 
	{
		pNode->CommonClear();
	}
}

int CubeMapRendering::GetDrawNodeCount(HIObject object)
{
	CubeMapRendering *pNode = FBCast<CubeMapRendering>(object);
	if (pNode) 
	{
		return pNode->mStats.totalDrawNodeCount;
	}
	return 0;
}

int CubeMapRendering::GetDrawGeomCacheCount(HIObject object)
{
	CubeMapRendering *pNode = FBCast<CubeMapRendering>(object);
	if (pNode) 
	{
		return pNode->mStats.totalGeometryCacheCount;
	}
	return 0;
}

double CubeMapRendering::GetUpdateDuration(HIObject object)
{
	CubeMapRendering *pNode = FBCast<CubeMapRendering>(object);
	if (pNode) 
	{
		return pNode->mStats.totalUpdateDuration;
	}
	return 0.0;
}

FBTime CubeMapRendering::GetUpdateLocalTime(HIObject object)
{
	CubeMapRendering *pNode = FBCast<CubeMapRendering>(object);
	if (pNode) 
	{
		return pNode->mStats.updateLocalTime;
	}
	return FBTime::Zero;
}