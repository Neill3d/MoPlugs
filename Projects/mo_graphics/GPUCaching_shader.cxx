
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: GPUCaching_shader.cxx
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
#include "GPUCaching_shader.h"
#include <GL\glew.h>

#include "stringUtils.h"
//#include "shared_io.h"

//#include "textures_view.h"

#include "IO\FileUtils.h"

#include "Common_Physics\physics_common.h"
#include "library_NewtonPhysics\newton_PUBLIC.h"

#include "algorithm\math3d_mobu.h"

#include "gpucache_visitorImpl.h"
#include "MB_renderer.h"

//--- Registration defines
#define ORSHADERGPUCACHE__CLASS		ORSHADERGPUCACHE__CLASSNAME
#define ORSHADERGPUCACHE__DESC		"GPUCache"

//#define GEOM_CACHE_EFFECT			"GeomCacheShader.glslfx"

//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	ORSHADERGPUCACHE__CLASS	);
FBRegisterShader	(	ORSHADERGPUCACHE__DESCSTR,
					 ORSHADERGPUCACHE__CLASS,
					 ORSHADERGPUCACHE__DESCSTR,
					 ORSHADERGPUCACHE__DESC,
					 FB_DEFAULT_SDK_ICON		);

extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);

////////////////////////////////////////////////////////////

static void ORShaderGPUCache_LaunchSource(HIObject object, bool pValue)
{
	ORShaderGPUCache *shader = FBCast<ORShaderGPUCache>(object);
	if (shader && pValue) 
	{
		shader->DoLaunchSource();
	}
}

static void ORShaderGPUCache_Load(HIObject object, bool pValue)
{
	ORShaderGPUCache *shader = FBCast<ORShaderGPUCache>(object);
	if (shader && pValue) 
	{
		shader->DoLoad();
	}
}

static void ORShaderGPUCache_Reload(HIObject object, bool pValue)
{
	ORShaderGPUCache *shader = FBCast<ORShaderGPUCache>(object);
	if (shader && pValue) 
	{
		shader->DoReload(nullptr);
	}
}

static void ORShaderGPUCache_ReloadShader(HIObject object, bool pValue)
{
	ORShaderGPUCache *shader = FBCast<ORShaderGPUCache>(object);
	if (shader && pValue) 
	{
		shader->DoReloadShader();
	}
}

static void ORShaderGPUCache_SaveTextures(HIObject object, bool pValue)
{
	ORShaderGPUCache *shader = FBCast<ORShaderGPUCache>(object);
	if (shader && pValue) 
	{
		shader->DoSaveTextures();
	}
}


static void ORShaderGPUCache_DebugSnapshot(HIObject object, bool pValue)
{
	ORShaderGPUCache *shader = FBCast<ORShaderGPUCache>(object);
	if (shader && pValue) 
	{
		shader->DoDebugSnapshot();
	}
}

static void ORShaderGPUCache_SetDebugDisplay(HIObject object, bool pValue)
{
	ORShaderGPUCache *shader = FBCast<ORShaderGPUCache>(object);
	if (shader) 
	{
		shader->SetDebugDisplay(pValue);
		shader->DebugDisplay.SetPropertyValue(pValue);
	}
}
/*
static void ORShaderGPUCache_SetLogarithmicDepth(HIObject object, bool pValue)
{
	ORShaderGPUCache *shader = FBCast<ORShaderGPUCache>(object);
	if (shader) 
	{
		shader->SetLogarithmicDepth(pValue);
		shader->LogarithmicDepth.SetPropertyValue(pValue);
	}
}
*/
/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ORShaderGPUCache::FBCreate()
{
	bool lStatus = ParentClass::FBCreate();
	
	if (false == lStatus)
		return false;
	/*
	FBPropertyPublish(	this, AffectingLights,	"AffectingLights",		NULL, NULL);
	FBPropertyPublish(	this, UseSceneLights,	"Use scene lights",		NULL, NULL);
    AffectingLights.SetFilter(FBLight::GetInternalClassId());
    AffectingLights.SetSingleConnect(false);
	UseSceneLights = true;
	*/
	FBPropertyPublish( this, SourceFileName, "Source FileName", nullptr, nullptr );
	FBPropertyPublish( this, LaunchSourceFile, "Launch Source File", nullptr, ORShaderGPUCache_LaunchSource );
	SourceFileName = "";

	FBPropertyPublish( this, Load, "Load", nullptr, ORShaderGPUCache_Load );	
	FBPropertyPublish( this, Loaded, "Loaded", nullptr, nullptr );
	FBPropertyPublish( this, Reload, "Reload", nullptr, ORShaderGPUCache_Reload );
	FBPropertyPublish( this, ReloadShader, "Reload Shader", nullptr, ORShaderGPUCache_ReloadShader );
	FBPropertyPublish( this, SaveTextures, "Save Textures", nullptr, ORShaderGPUCache_SaveTextures );

	FBPropertyPublish( this, DebugSnapshot, "Debug", nullptr, ORShaderGPUCache_DebugSnapshot );
	FBPropertyPublish( this, DebugCamera, "Debug Camera", nullptr, nullptr );
	FBPropertyPublish( this, DebugDisplay, "Debug Display", nullptr, ORShaderGPUCache_SetDebugDisplay );

	//
	//

	FBPropertyPublish( this, OverrideShading, "Override shading", nullptr, nullptr);
	OverrideShading = false;

	//FBPropertyPublish( this, LogarithmicDepth, "Accurate Depth", nullptr, ORShaderGPUCache_SetLogarithmicDepth );
	// LogarithmicDepth = false;
	/*
	FBPropertyPublish( this, RenderOnBack, "Render On Back", nullptr, nullptr);
	RenderOnBack = false;

	
	FBPropertyPublish(	this, ShadingType,	"Shading",NULL, NULL);
	ShadingType = eShadingTypeDynamic;

	FBPropertyPublish(	this, ToonSteps, "Toon Steps" ,NULL, NULL);
	ToonSteps = 4.0;

	FBPropertyPublish(	this, ToonDistribution, "Toon Distribution" ,NULL, NULL);
	ToonDistribution = 1.0;
	ToonDistribution.SetMinMax(0.1, 3.0, true, true);

	FBPropertyPublish(	this, ToonShadowPosition, "Toon Shadow Position" ,NULL, NULL);
	ToonShadowPosition = 0.5;
	ToonShadowPosition.SetMinMax(0.1, 1.0, true, true);
	*/
	FBPropertyPublish( this, DisplayNormals, "Display Normals", nullptr, nullptr );
	FBPropertyPublish( this, NormalsLength, "Normals Length", nullptr, nullptr );
	
	DisplayNormals = false;
	NormalsLength = 10.0;

	Loaded = false;
	Loaded.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	DebugCamera.SetFilter( FBCamera::GetInternalClassId() );
	DebugCamera.SetSingleConnect(true);
	DebugDisplay = false;

	FBPropertyPublish( this, FileName, "FileName", nullptr, nullptr );

	// some statistics
	FBPropertyPublish( this, ObjectsCount, "Objects Count", nullptr, nullptr );
	FBPropertyPublish( this, MaterialsCount, "Materials Count", nullptr, nullptr );
	FBPropertyPublish( this, TexturesCount, "Textures Count", nullptr, nullptr );
	FBPropertyPublish( this, ShadersCount, "Shaders Count", nullptr, nullptr );
	FBPropertyPublish( this, LightCount, "Lights Count", nullptr, nullptr );

	FBPropertyPublish( this, MemoryUsage, "Memory Usage", nullptr, nullptr);

	FBPropertyPublish( this, SampleAlphaToCoverage, "Sample Alpha", nullptr, nullptr );
	FBPropertyPublish( this, AlphaPass, "Alpha threshold", nullptr, nullptr );

	SampleAlphaToCoverage = true;
	AlphaPass = 0.1;

	ObjectsCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	MaterialsCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	TexturesCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	ShadersCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	LightCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	MemoryUsage.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	// allocate mem
	mTexturesView = nullptr;
	mListModels = nullptr;
	mListMaterials = nullptr;
	
	mNeedUpdateTexturesPtr = true;
	mCacheModel = nullptr;
	mSuccess = true;		// ready to try a effect

	//mGPUFBScene = &CGPUFBScene::instance();
	//mUberShader = nullptr;
	mSuccess = true;		// ready to try a effect

	//FBRenderingPass pass = (FBRenderingPass) (kFBPassPreRender | kFBPassLighted);
	//RenderingPass = pass;

	mQueryCacheGeometry = nullptr;
	mCacheWorld = nullptr;

	//
	FBFileMonitoringManager::TheOne().OnFileChangeFileReference.Add( this, (FBCallback) &ORShaderGPUCache::FileChangeEvent );

	//
	SetShaderCapacity(FBShaderCapacity(kFBShaderCapacityMaterialEffect), false);

	return true;
}


/************************************************
*	FiLMBOX Destructor.
************************************************/
void ORShaderGPUCache::FBDestroy()
{
	FBString strSource = SourceFileName;

	if (strSource != "")
	{
		FBFileMonitoringManager::TheOne().RemoveFileFromMonitor( strSource );
	}

	//
	FBFileMonitoringManager::TheOne().OnFileChangeFileReference.Add( this, (FBCallback) &ORShaderGPUCache::FileChangeEvent );

	if (mCacheModel)
	{
		delete mCacheModel;
		mCacheModel = nullptr;
	}

	//
	if (mQueryCacheGeometry)
	{
		delete mQueryCacheGeometry;
		mQueryCacheGeometry = nullptr;
	}
	if (mCacheWorld)
	{
		delete mCacheWorld;
		mCacheWorld = nullptr;
	}
}

void ORShaderGPUCache::DoLaunchSource()
{
	FBString sourceFileName = SourceFileName;

	if (sourceFileName != "")
	{
		char szApplication[] = "C:\\Program Files\\Autodesk\\MotionBuilder 2014\\bin\\x64\\motionbuilder.exe";

		char szCmdline[MAX_PATH];
		sprintf_s(szCmdline, MAX_PATH, "%s -open %s", szApplication, (char*) sourceFileName );

		STARTUPINFO         siStartupInfo;
		PROCESS_INFORMATION piProcessInfo;

		memset(&siStartupInfo, 0, sizeof(siStartupInfo));
		memset(&piProcessInfo, 0, sizeof(piProcessInfo));

		siStartupInfo.cb = sizeof(siStartupInfo);

		::CreateProcess(NULL, szCmdline, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, 0, 0, &siStartupInfo, &piProcessInfo /* ... */);
	}
}

void ORShaderGPUCache::DoLoad()
{
	FBFilePopup lDialog;
	lDialog.Style = kFBFilePopupOpen;
	lDialog.Filter = "*.xml";

	Loaded = false;

	if (lDialog.Execute() )
	{
		FileName = lDialog.FullFilename;

		LoadFromFileName(nullptr);
	}
}

void ORShaderGPUCache::LoadFromFileName(FBFbxObject* pFbxObject)
{
	mSuccess = false;
	Loaded = false;

	// DONE: add local path searching
	FBString fbxFileName("");
	if (pFbxObject != nullptr)
	{
		fbxFileName = pFbxObject->GetFullFilePath( "test.txt" );
	}
	else
	{
		fbxFileName = FBApplication::TheOne().FBXFileName;
	}

	FBString fullFileName(FileName);
	FBString fileName(ExtractFileName(fullFileName));
	FBString filePath(ExtractFilePath(fullFileName));

	try
	{
		if (!IsFileExists(fullFileName) )
			if (fbxFileName != "")
			{
				// try local location
				FBString fbxFilePath( ExtractFilePath(fbxFileName) );

				fullFileName = fbxFilePath + "\\";
				fullFileName = fullFileName + fileName;

				filePath = fbxFilePath;

				if (!IsFileExists(fullFileName) )
					throw "failed to locate cache";
			}
			else
			{
				throw "failed to locate cache";
			}

	}
	catch (const char *e)
	{
		FBMessageBox( "Geometry Cache Loading", FBString(e, fileName), "Ok" );
		mSuccess = true;
		return;
	}

	//
	

	mCacheModel = new CGPUCacheModel();

	CGPUCacheLoader					loader;
	CGPUCacheLoaderVisitorImpl		visitor(mCacheModel);

	if (false == loader.Load( fullFileName, &visitor ) )
	{
		delete mCacheModel;
		mCacheModel = nullptr;

		FBMessageBox( "Geometry Cache Loading", "Failed to load a file", "Ok" );
		return;
	}
	else
	{
		float bmin[3], bmax[3];
		mCacheModel->GetBoundingBox(bmin, bmax);
		
		mMin = FBVector3d(bmin[0], bmin[1], bmin[2]);
		mMax = FBVector3d(bmax[0], bmax[1], bmax[2]);

		mCacheModel->OverrideShading = true;
		mCacheModel->ShadingType = eShadingTypeFlat;
	}



	CHECK_GL_ERROR_MOBU();


	//
	FBTVector t;
	FBRVector r;
	FBSVector s;

	FBVector3d t3 = GetModelsCenter();
	FBVector3d s3 = GetModelsSize();

	t = FBVector4d( t3[0], t3[1], t3[2], 1.0 );
	s = FBSVector( s3[0], s3[1], s3[2] );

	FBMatrix lMatrix = mLoadMatrix;
	FBTRSToMatrix( mLoadMatrix, t, r, s );

	int dstCount = GetDstCount();
	for (int i=0; i<dstCount; ++i)
	{
		FBPlug *pPlug = GetDst(i);
		
		if ( FBIS( pPlug, FBModelCube ) )
		{
			FBMatrix tm, tmDiff, tmBefore;
			FBModel *pModel = (FBModel*) pPlug;

			pModel->GetMatrix(tm);

			FBGetLocalMatrix( tmDiff, tm, lMatrix );
			FBMatrixInverse( tmDiff, tmDiff );
			FBMatrixMult( tm, mLoadMatrix, tmDiff );

			pModel->SetMatrix( tm );
			//mLoadMatrix = tm;

			break;
		}
	}

	FBMatrixInverse(mLoadMatrixInv, mLoadMatrix);
	
	// update shader UI
	if (mTexturesView)
	{
//		( (ORView3D*) mTexturesView )->Resize(10, 10);
	}

	CHECK_GL_ERROR_MOBU();

	UpdateUI();
	mSuccess = true;

	// save textures (if not compressed)

	Loaded = true;


	// add monitoring system
	FBFileMonitoringManager	&lMonitoring = FBFileMonitoringManager::TheOne();
	lMonitoring.AddFileToMonitor( mCacheModel->GetSourceFilename(), kFBFileMonitoring_FILEREFERENCE );
}

void ORShaderGPUCache::FileChangeEvent(HISender pSender, HKEvent pEvent)
{
	if (1 == FBMessageBox( Name, "Source file has been changed. Do you want to reload a cache ?", "Ok", "Cancel" ) )
	{
		DoReload(nullptr);
	}
}

void ORShaderGPUCache::UpdateUI()
{
	if (mCacheModel && mListModels && mListMaterials)
	{
		mListModels->Items.Clear();

		const int numberOfSubModels = mCacheModel->GetNumberOfSubModels();
		for (int i=0; i<numberOfSubModels; ++i)
		{
			mListModels->Items.Add( mCacheModel->GetSubModelName(i) );
		}

		mListMaterials->Items.Clear();

		const int numberOfMaterials = mCacheModel->GetNumberOfMaterials();
		for (int i=0; i<numberOfMaterials; ++i)
		{
			mListMaterials->Items.Add( mCacheModel->GetMaterialName(i) );
		}

		mListShaders->Items.Clear();
		const int numberOfShaders = mCacheModel->GetNumberOfBaseShaders();
		for (int i=0; i<numberOfShaders; ++i)
		{
			mListShaders->Items.Add( mCacheModel->GetShaderName(i) );
		}
	}
}



void ORShaderGPUCache::DoReload(FBFbxObject* pFbxObject)
{
	FBString fullFileName(FileName);
	/*
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	*/
	if (fullFileName != "")
	{
		LoadFromFileName(pFbxObject);
	}
}

void ORShaderGPUCache::DoReloadShader()
{
	if (mGPUFBScene)
		mGPUFBScene->ReloadUberShader();
	//mUberShader = nullptr;
}

void ORShaderGPUCache::DoSaveTextures()
{
	// DONE: save textures (with compression and mipmaps)
	FBString fname( FileName );

	if (fname.GetLen() > 0)
	{
		FBString fullFileName(fname);
		FBString fileName(ExtractFileName(fullFileName));
		ChangeFileExt(fileName, FBString(""));
		FBString filePath(ExtractFilePath(fullFileName));
		filePath = filePath + "\\";
		filePath = filePath + fileName;

		// check if all files are exist
		FBString texturesFileName(filePath + "_Textures.pck");

		//mTexturesManager->ReSaveTexturesPackage(texturesFileName);
	}
}

FBVector3d ORShaderGPUCache::GetModelsCenter()
{
	return FBVector3d( mMin[0]+0.5*(mMax[0]-mMin[0]), mMin[1]+0.5*(mMax[1]-mMin[1]), mMin[2]+0.5*(mMax[2]-mMin[2]) );
}

FBVector3d ORShaderGPUCache::GetModelsSize()
{
	FBVector3d res(mMax[0]-mMin[0], mMax[1]-mMin[1], mMax[2]-mMin[2] );

	if (res[0] == 0.0) res[0] = 1.0;
	if (res[1] == 0.0) res[1] = 1.0;
	if (res[2] == 0.0) res[2] = 1.0;

	return res;
}

void ORShaderGPUCache::SetDebugDisplay(const bool value)
{
	/*
	CGPULightsManager *lighting = CGPUFBScene::instance().GetLightsManagerPtr();
	if (lighting)
		lighting->SetDebugDisplay(value);
		*/
}

void ORShaderGPUCache::DoDebugSnapshot()
{
	//FBCamera *pCamera = mSystem.Renderer->CurrentCamera;
	//mLightsManager->DebugMatrixSnapshot(pCamera);
}

/************************************************
*	Shader functions.
************************************************/

bool ORShaderGPUCache::FbxStore	(FBFbxObject* pFbxObject)
{
	return true;
}

bool ORShaderGPUCache::FbxRetrieve(FBFbxObject* pFbxObject, FBRenderer* pRenderer)
{
	DoReload(pFbxObject);
	return true;
}
/*
void ORShaderGPUCache::StoreCullMode()
{
	mCullFace = glIsEnabled(GL_CULL_FACE);
	glGetIntegerv( GL_CULL_FACE_MODE, &mCullFaceMode );
}


void ORShaderGPUCache::FetchCullMode ()
{
	if (mCullFace == GL_TRUE) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	glCullFace(mCullFaceMode);
}
*/

bool ORShaderGPUCache::ShaderNeedBeginRender()
{
	return false;
}

void ORShaderGPUCache::ShaderBeginRender( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pInfo )
{	
}

// TODO: do we need to separate opaque and transparency passes here ?!
void ORShaderGPUCache::ShadeModel( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass )
{
	if (true == mGPUFBScene->IsWaiting() || nullptr == mCacheModel ) // || pRenderOptions->IsIDBufferRendering() )
		return;

	FBCamera *pCamera = pRenderOptions->GetRenderingCamera();
	InitializeFrameDataAndBuffers( pRenderOptions, pCamera, 
		FBGetDisplayInfo(), 
		pRenderOptions->GetRenderFrameId(),
		(kFBPassLighted == pPass),
		8192, 2048, false, nullptr, 0);
	
	FBViewingOptions* lViewingOptions = pRenderOptions->GetViewerOptions();
    bool lIsSelectBufferPicking = lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	mShaderCallback = nullptr;
	if (nullptr != mGPUFBScene->GetShaderFXPtr(Graphics::MATERIAL_SHADER_PROJECTORS) )
	{
		CRenderOptions &options = GetLastRenderOptions();

		if (false == lIsSelectBufferPicking && false == lIsColorBufferPicking)
		{
			mShaderCallback = mShadersFactory.FindTypeByShaderAndGoal(this, eRenderGoalShading);
			mShaderPass = eShaderPassTransparency;

			options.SetGoal(eRenderGoalShading);
		}
		else
		{
			mShaderCallback = mShadersFactory.FindTypeByShaderAndGoal(this, eRenderGoalSelectionId);
			mShaderPass = eShaderPassOpaque;

			options.SetGoal(eRenderGoalSelectionId);
		}

		options.SetPass( mShaderPass );
	}

	//mShaderInfo = mGPUFBScene->FindShaderInfo(this, mShaderCallback);
	mShaderInfo = nullptr;

	if (nullptr == mShaderCallback)
		return;

	CRenderOptions &options = GetLastRenderOptions();

	// draw a cache
	mShaderCallback->OnTypeBegin( options, false );
	if (true == mShaderCallback->OnInstanceBegin( options, pRenderOptions, this, mShaderInfo ) )
	{

		// draw opaque


		mShaderCallback->OwnModelShade( options, pRenderOptions, pShaderModelInfo->GetFBModel(), this, mShaderInfo );

		// draw transparency
		/*
		if (mCacheModel->GetNumberOfOpaqueCommands() > 0)
		{
			glEnable(GL_ALPHA_TEST);

			options.SetPass( eShaderPassTransparency );
			mShaderCallback->OwnModelShade( options, pShaderModelInfo->GetFBModel(), this, mShaderInfo );

			glDisable(GL_ALPHA_TEST);
		}
		*/
		mShaderCallback->OnInstanceEnd( options, this, mShaderInfo );
	}
	mShaderCallback->OnTypeEnd( options );
}

void ORShaderGPUCache::CustomDisplay(const int index, FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
	//if (mUberShader == nullptr || mModelRender == nullptr) return;


	glBegin(GL_LINES);

	glColor3d(1.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(10.0, 0.0, 0.0);

	glColor3d(0.0, 1.0, 0.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 10.0, 0.0);

	glColor3d(0.0, 0.0, 1.0);
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 10.0);

	glEnd();
}

void ORShaderGPUCache::DebugGeometryOutput()
{
	if (mQueryCacheGeometry == nullptr)
	{
		return;
	}

	FBModel *pNewModel = new FBModelCube("debugCacheGeometry");
	FBMesh *pNewMesh = new FBMesh("newMesh");

	//
	//

	pNewMesh->GeometryBegin();
	pNewMesh->VertexArrayInit(mQueryCacheGeometry->GetVertexCount(), false);
	
	for (int i=0; i<mQueryCacheGeometry->GetVertexCount(); ++i)
	{
		pNewMesh->VertexSet( FBVertex( (float*)mQueryCacheGeometry->GetVertexPosition(i)), i );
	}

	for (int i=0; i<mQueryCacheGeometry->GetPolyCount(); ++i)
	{
		pNewMesh->PolygonBegin();

		auto poly = mQueryCacheGeometry->GetPoly(i);

		for (int j=0; j<poly->count; ++j)
			pNewMesh->PolygonVertexAdd( poly->indices[j] );

		pNewMesh->PolygonEnd();
	}
	pNewMesh->GeometryEnd();

	//

	pNewModel->Geometry = pNewMesh;

	pNewModel->Show = true;
	pNewModel->Visibility = true;
}

bool ORShaderGPUCache::ClosestRayIntersection(const FBTVector& pRayOrigin, const FBTVector& pRayEnd, FBTVector& pIntersectPos, FBTVector& pIntersecNormal)
{

	if (mQueryCacheGeometry == nullptr)
	{
		mQueryCacheGeometry = new GPUCacheGeometry( mCacheModel->GetModelRenderPtr() );
		//DebugGeometryOutput();
	}
	
	if (mCacheWorld == nullptr && mQueryCacheGeometry != nullptr && mQueryCacheGeometry->GetVertexCount() > 0)
	{
		// TODO:
		mCacheWorld = CreateNewNewtonWorld(1.0, 1, 1, 30.0f);
		mCacheWorld->LoadLevel(mQueryCacheGeometry);
	}
	
	
	if (mCacheWorld)
	{
		FBTVector localRayOrigin, localRayEnd;

		FBVectorMatrixMult( localRayOrigin, mLoadMatrix, pRayOrigin );
		FBVectorMatrixMult( localRayEnd, mLoadMatrix, pRayEnd );

		FBTVector p0( localRayOrigin[0], localRayOrigin[1], localRayOrigin[2], 1.0 );
		FBTVector p1( localRayEnd[0], localRayEnd[1] - 10000.0, localRayEnd[2], 1.0 );

		//vec4 pos( 0.0f, 0.0f, 0.0f, 1.0f );
		//vec4 nor( 0.0f, 0.0f, 0.0f, 1.0f );

		double dist = 0.0;
		long long attribute = 0;

		if (true == mCacheWorld->ClosestRayCast( p0, p1, pIntersectPos, pIntersecNormal, &dist, &attribute ) )
		{
			//pIntersectPos = FBTVector( (double)pos[0], (double)pos[1], (double)pos[2], 1.0);
			//pIntersecNormal = FBTVector( (double)nor[0], (double)nor[1], (double)nor[2], 1.0);

			FBVectorMatrixMult( pIntersectPos, mLoadMatrixInv, pIntersectPos );
			FBVectorMatrixMult( pIntersecNormal, mLoadMatrixInv, pIntersecNormal );

			return true;
		}
	}

	pIntersectPos = FBTVector(0,  1.0, 0.0); //return the center
    pIntersecNormal = FBTVector(1, 0, 0);

	return false;
}

void ORShaderGPUCache::DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	mGPUFBScene->ReloadUberShader();
	//mUberShader = nullptr;

	mShaderCallback = mShadersFactory.FindTypeByShaderAndGoal(this, eRenderGoalShading);
	if (nullptr != mShaderCallback)
		mShaderCallback->DetachRenderContext(pOptions, this);

	// lets update textures handles on gpu
	mNeedUpdateTexturesPtr = true;
	if (nullptr != mCacheModel)
		mCacheModel->NeedUpdateTexturePtr();

	ParentClass::DetachDisplayContext(pOptions, pInfo);
}