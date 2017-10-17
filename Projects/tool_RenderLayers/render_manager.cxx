
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: render_manager.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "render_manager.h"

//--- Registration defines
#define RENDERMANAGER__CLASS RENDERMANAGER__CLASSNAME
#define RENDERMANAGER__NAME  RENDERMANAGER__CLASSSTR

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation( RENDERMANAGER__CLASS  );  // Manager class name.
FBRegisterCustomManager( RENDERMANAGER__CLASS );         // Manager class name.

bool			gDoRender = false;
bool			gClose = false;

ERenderMode		gRenderMode = eRenderModeNone;
FBString		gCameraName("");

bool			gStartFrameChanged = false;
int				gStartFrame = 0;
bool			gStopFrameChanged = false;
int				gStopFrame = 0;
bool			gStepFrameChanged = false;
int				gStepFrame = 0;

int				gAntialiasing = -1;
int				gShowTimeCode = -1;
int				gShowSafeArea = -1;
int				gShowCameraLabel = -1;

int				gCodec = -1;
FBString		gCodecName("");

FBString		gRenderer("");
FBString		gPictureFormat("");
FBString		gOutputFormat("");
FBString		gOutput("");

void DefaultGlobalRenderSettings()
{
	gDoRender = false;
	gClose = false;

	gRenderMode = eRenderModeNone;
	gCameraName = "";

	gStartFrameChanged = false;
	gStartFrame = 0;
	gStopFrameChanged = false;
	gStopFrame = 0;
	gStepFrameChanged = false;
	gStepFrame = 0;

	gAntialiasing = -1;
	gShowTimeCode = -1;
	gShowSafeArea = -1;
	gShowCameraLabel = -1;

	gCodec = -1;
	gCodecName = "";

	gRenderer = "";
	gPictureFormat = "";
	gOutputFormat = "";
	gOutput = "";
}

/************************************************
 *  FiLMBOX Creation
 ************************************************/
bool RenderManager::FBCreate()
{
    return true;
}


/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void RenderManager::FBDestroy()
{
    // Free any user memory here.
}


/************************************************
 *  Execution callback.
 ************************************************/
bool RenderManager::Init()
{
    return true;
}

bool RenderManager::Open()
{

	FBStringList pList;

	FBSystem &lSystem = FBSystem::TheOne();
	pList = lSystem.GetCommandLineArgs();
	
	DefaultGlobalRenderSettings();

	auto fn_check = [&pList] (const int i, const char *name) -> bool {
		return ( strcmp(pList[i], name)==0 && pList.GetCount() > i+1);
	};

	auto fn_checkBool = [&pList] (const int i) -> int {
		int result = -1;
		FBString name(pList[i]);
		
		if (name == "yes" || name == "on" || name == "1" )
			result = 1;
		else if (name == "no" || name == "off" || name == "0")
			result = 0;

		return result;
	};

	FBString item("");
	FBString param("");

	for (int i=1; i<pList.GetCount(); ++i)
	{
		item = pList[i];
		
		if (item == "--close")
			gClose = true;
		else if ( fn_check(i, "--r") || fn_check(i, "--render") )
		{
			i += 1;
			if (i < pList.GetCount() )
				param = pList[i];

			if ( param == "layers" )
			{
				gRenderMode = eRenderModeLayers;
				gDoRender = true;
			}
			else if ( param == "offscreen" )
				gRenderMode = eRenderModeOffscreen;
			else if ( param == "window" )
				gRenderMode = eRenderModeWindow;
		}
		else if ( fn_check(i, "--cam") || fn_check(i, "--camera") )
		{
			i += 1;
			if ( i < pList.GetCount() )
				gCameraName = pList[i];
		}
		else if ( fn_check(i, "--stt") || fn_check(i, "--startTime") )
		{
			i += 1;
			if ( i < pList.GetCount() )
			{
				gStartFrameChanged = true;
				gStartFrame = atoi(pList[i]);
			}
		}
		else if ( fn_check(i, "--stp") || fn_check(i, "--stopTime") )
		{
			i += 1;
			if ( i < pList.GetCount() )
			{
				gStopFrameChanged = true;
				gStopFrame = atoi(pList[i]);
			}
		}
		else if ( fn_check(i, "--step") || fn_check(i, "--stepTime") )
		{
			i += 1;
			if ( i < pList.GetCount() )
			{
				gStepFrameChanged = true;
				gStepFrame = atoi(pList[i]);
			}
		}
		else if ( fn_check(i, "--pictureTimeCode") )
		{
		}
		else if ( fn_check(i, "--showTimeCode") )
		{
			gShowTimeCode = fn_checkBool(i+1);
		}
		else if ( fn_check(i, "--showSafeArea") )
		{
			gShowSafeArea = fn_checkBool(i+1);
		}
		else if ( fn_check(i, "--showCameraLabel") )
		{
			gShowCameraLabel = fn_checkBool(i+1);
		}
		else if ( fn_check(i, "--c") || fn_check(i, "--codec") )
		{
			i += 1;
			if ( i < pList.GetCount() )
			{
				//param = pList[i+1];
				gCodecName = pList[i];
				/*
				if (param == "uncompressed")
					gCodec = FBVideoCodecUncompressed;
				else if (param == "asked")
					gCodec = FBVideoCodecAsk;
				else if (param == "stored")
					gCodec = FBVideoCodecStored;
					*/
			}
		}
		else if ( fn_check(i, "--pctFrm") || fn_check(i, "--pictureFormat") )
		{
			i += 1;
			if (i < pList.GetCount() )
			{
				gPictureFormat = pList[i];
			}
		}
		else if ( fn_check(i, "--oFmt") || fn_check(i, "--outputFormat") )
		{
			i += 1;
			if (i < pList.GetCount() )
			{
				gOutputFormat = pList[i];
			}
		}
		else if ( fn_check(i, "--of") || fn_check(i, "--outputFile") )
		{
			i += 1;
			if (i < pList.GetCount() )
			{
				gOutput = pList[i];
			}
		}
		else if ( fn_check(i, "--aa") || fn_check(i, "--antialiasing") )
		{
			i += 1;
			if (i < pList.GetCount() )
			{
				gAntialiasing = fn_checkBool(i);
			}
		}
		else if ( fn_check(i, "--rr") || fn_check(i, "--renderer") )
		{
			i += 1;
			if (i < pList.GetCount() )
			{
				gRenderer = pList[i];
			}
		}
	}

	//mApp.OnFileOpenCompleted.Add(this, (FBCallback) &RenderManager::EventFileOpenCompleted );
	mSystem.OnUIIdle.Add( this, (FBCallback) &RenderManager::EventIdle );

    return true;
}

const FBCameraResolutionMode ConvertStringIntoResMode(const char *str)
{
	FBCameraResolutionMode mode = kFBResolutionCustom;

	if (strcmp(str, "FromCamera") == 0)
	{
		mode = kFBResolutionCustom;
	}
	else if (strcmp(str, "D1_NTSC") == 0 )
	{
		mode = kFBResolutionD1NTSC;
	}
	else if (strcmp(str, "NTSC") == 0 )
	{
		mode = kFBResolutionNTSC;
	}
	else if (strcmp(str, "PAL") == 0 )
	{
		mode = kFBResolutionPAL;
	}
	else if (strcmp(str, "D1_PAL") == 0 )
	{
		mode = kFBResolutionD1PAL;
	}
	else if (strcmp(str, "HD") == 0 )
	{
		mode = kFBResolutionHD;
	}
	else if (strcmp(str, "640x480") == 0 )
	{
		mode = kFBResolution640x480;
	}
	else if (strcmp(str, "320x200") == 0 )
	{
		mode = kFBResolution320x200;
	}
	else if (strcmp(str, "320x240") == 0 )
	{
		mode = kFBResolution320x240;
	}
	else if (strcmp(str, "128x128") == 0 )
	{
		mode = kFBResolution128x128;
	}
	else if (strcmp(str, "FullScreen") == 0 )
	{
		mode = kFBResolutionFullScreen;
	}

	return mode;
}

void GlobalRunRender()
{
	if ( gRenderMode != eRenderModeOffscreen && gRenderMode != eRenderModeWindow )
		return;

	FBVideoGrabber			VideoGrabber;
	FBVideoGrabOptions	GrabOptions = VideoGrabber.GetOptions();

	FBScene *pScene = FBSystem::TheOne().Scene;
	FBRenderer *pRenderer = FBSystem::TheOne().Renderer;

	FBVideoCodecManager lCodecMan;
	lCodecMan.SetVideoCodecMode( FBVideoCodecStored );

	if (gCodecName.GetLen() > 0 && gOutput.GetLen() > 0)
	{
		FBStringList pCodecList;
		const char *szFileName = gOutput;

		FBString fileFormat("");

		if ( strstr( szFileName, ".avi" ) != nullptr )
			fileFormat = "AVI";
		else if (strstr( szFileName, ".mov" ) != nullptr )
			fileFormat = "MOV";

		if (fileFormat.GetLen() > 0)
		{
			lCodecMan.GetCodecIdList( fileFormat, pCodecList);

			int id = -1;
			const char *szCodecKey = gCodecName;

			for (int i=0; i<pCodecList.GetCount(); ++i)
			{
				const char *szCodecName = pCodecList[i];
				if (strcmp(szCodecName, szCodecKey) == 0)
				{
					id = i;
					break;
				}
			}

			if (id >= 0)
				lCodecMan.SetDefaultCodec( fileFormat, pCodecList[id] );
		}
	}

	if (gStartFrameChanged || gStopFrameChanged)
	{
		FBTime lStart, lStop;
		lStart.SetFrame( gStartFrame );
		lStop.SetFrame( gStopFrame );
		GrabOptions.mTimeSpan.Set(lStart, lStop);
	}
	
	if (gStepFrameChanged )
	{
		FBTime lStep;
		lStep.SetFrame( gStepFrame );
		GrabOptions.mTimeSteps = lStep;
	}
	
	//GrabOptions.mCameraResolution = pLayer->PictureMode;
	//GrabOptions.mBitsPerPixel = pLayer->PixelMode;
	//GrabOptions.mFieldMode = pLayer->FieldMode;
	//GrabOptions.mViewingMode = pLayer->DisplayMode;

	if (gOutput.GetLen() > 0)
		GrabOptions.mOutputFileName = gOutput;

	if (gAntialiasing >= 0)
		GrabOptions.mAntiAliasing = (gAntialiasing > 0) ? true : false;
	if (gShowCameraLabel >= 0)
		GrabOptions.mShowCameraLabel = (gShowCameraLabel > 0) ? true : false;
	if (gShowSafeArea >= 0)
		GrabOptions.mShowSafeArea = (gShowSafeArea > 0) ? true : false;
	if (gShowTimeCode >= 0)
		GrabOptions.mShowTimeCode = (gShowTimeCode > 0) ? true : false;
	//GrabOptions.mRenderAudio = pLayer->Audio;
	//GrabOptions.mAudioRenderFormat = FBAudioFmt_GetDefaultFormat();	//44100Hz 16bits stereo

	// - set renderer
	if (gRenderer.GetLen() > 0)
	{
		const char *szRendererKey = gRenderer;
		for (int i=0, count=pRenderer->RendererCallbacks.GetCount(); i<count; ++i)
		{
			const char *theRenderer = pRenderer->RendererCallbacks[i]->Name;

			if (strcmp(theRenderer, szRendererKey) == 0)
			{
				//pRenderer->CurrentPaneCallbackIndex = i;
				GrabOptions.mRendererCallbackIndex = i;
				break;
			}
		}
	}

	if (gPictureFormat.GetLen() > 0)
	{
		GrabOptions.mCameraResolution = ConvertStringIntoResMode(gPictureFormat);
		FBTrace( gPictureFormat );
		FBTrace( "\n %d\n", (int) GrabOptions.mCameraResolution );
	}

	VideoGrabber.SetOptions(&GrabOptions);
	
	// - set camera
	if (gCameraName.GetLen() > 0) {
		FBModel *pCameraModel = FBFindModelByLabelName( gCameraName );
		if (pCameraModel)
		{
			pRenderer->CurrentCamera = (FBCamera*) pCameraModel;
		}
	}

	//Start rendering while grabbing each frame
#ifdef OLD_FBAPP_FILERENDER
	mApplication.FileRender( &GrabOptions );
#else
	if (!FBApplication::TheOne().FileRender( &GrabOptions ) )
	{
		FBMessageBox( "Render error", VideoGrabber.GetLastErrorMsg(), "Ok" );
	}
#endif

}

void	RenderManager::EventFileOpenCompleted ( HISender pSender, HKEvent pEvent )
{
	mApp.OnFileOpenCompleted.Remove(this, (FBCallback) &RenderManager::EventFileOpenCompleted );
	mSystem.OnUIIdle.Add( this, (FBCallback) &RenderManager::EventIdle );
}

void	RenderManager::EventIdle       ( HISender pSender, HKEvent pEvent )
{
	if (gDoRender)
	{
		FBPopNormalTool( "Render Layers" );
	} 
	else if ( gRenderMode == eRenderModeOffscreen || gRenderMode == eRenderModeWindow )
	{
		GlobalRunRender();

		if (gClose)
		{
			FBApplication &app = FBApplication::TheOne();
			app.FileExit();
		}
	}

	mSystem.OnUIIdle.Remove( this, (FBCallback) &RenderManager::EventIdle );
}

bool RenderManager::Clear()
{
    return true;
}

bool RenderManager::Close()
{
    return true;
}
