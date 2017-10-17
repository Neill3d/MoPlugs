
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: renderCommon.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "renderCommon.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <tchar.h>

// global variables

LayerItem *g_CurrentLayer = nullptr;
FBArrayTemplate<int>	g_EventStack;

bool		g_mainUIUPdate = false;

void		AskMainUIForUpdate()
{
	g_mainUIUPdate = true;
}
const bool	IsMainUINeedUpdate(const bool reset)
{
	if (g_mainUIUPdate)
	{
		g_mainUIUPdate = reset;
		return true;
	}
	
	return false;
}


void gSetCurrentLayer( LayerItem *pItem )
{
	g_CurrentLayer = pItem;
}

LayerItem *gGetCurrentLayer()
{
	return g_CurrentLayer;
}

void AddNewEvent(int eventCode)
{
	g_EventStack.Add(eventCode);
}

int PopEvent()
{
	if (g_EventStack.GetCount() )
	{
		int e = g_EventStack.GetAt(0);
		g_EventStack.RemoveAt(0);
		return e;
	}

	return EVENT_EMPTY_STACK;
}

void ClearEvents()
{
	g_EventStack.Clear();
}

bool dirExists(const char *dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

bool CreateDirectoryTree(const char*szPathTree){
	LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL;
    bool bSuccess = false;
    const BOOL bCD = CreateDirectory(szPathTree, lpSecurityAttributes);
    DWORD dwLastError = 0;
    if(!bCD){
        dwLastError = GetLastError();
    }else{
        return true;
    }
    switch(dwLastError){
        case ERROR_ALREADY_EXISTS:
            bSuccess = true;
            break;
        case ERROR_PATH_NOT_FOUND:
            {
                TCHAR szPrev[MAX_PATH] = {0};
                LPCTSTR szLast = _tcsrchr(szPathTree,'\\');
                _tcsnccpy(szPrev,szPathTree,(int)(szLast-szPathTree));
                if(CreateDirectoryTree(szPrev)){
                    bSuccess = CreateDirectory(szPathTree,lpSecurityAttributes)!=0;
                    if(!bSuccess){
                        bSuccess = (GetLastError()==ERROR_ALREADY_EXISTS);
                    }
                }else{
                    bSuccess = false;
                }
            }
            break;
        default:
            bSuccess = false;
            break;
    }

    return bSuccess;
}

FBGroup *FindGroupByName(const char *groupName)
{
	FBScene *pScene = FBSystem().Scene;
	for (int i=0; i<pScene->Groups.GetCount(); ++i)
	{
		FBString str(pScene->Groups[i]->Name);
		if ( strcmp(groupName, str) == 0 )
		{
			return pScene->Groups[i];
		}
	}
	return nullptr;
}

FBCamera *FindCameraByName(const char *camName)
{
	FBScene *pScene = FBSystem().Scene;
	for (int i=0; i<pScene->Cameras.GetCount(); ++i)
	{
		FBString str(pScene->Cameras[i]->Name);
		if ( strcmp(camName, str) == 0 )
		{
			return pScene->Cameras[i];
		}
	}
	return nullptr;
}

FBMaterial *FindMaterialByName(const char *matName)
{
	FBScene *pScene = FBSystem().Scene;
	for (int i=0; i<pScene->Materials.GetCount(); ++i)
	{
		FBString str(pScene->Materials[i]->Name);
		if ( strcmp(matName, str) == 0 )
		{
			return pScene->Materials[i];
		}
	}
	return nullptr;
}

FBTexture *FindTextureByName(const char *texName)
{
	FBScene *pScene = FBSystem().Scene;
	for (int i=0; i<pScene->Textures.GetCount(); ++i)
	{
		FBString str(pScene->Textures[i]->Name);
		if ( strcmp(texName, str) == 0 )
		{
			return pScene->Textures[i];
		}
	}
	return nullptr;
}

FBShader *FindShaderByName(const char *shaderName)
{
	FBScene *pScene = FBSystem().Scene;
	for (int i=0; i<pScene->Shaders.GetCount(); ++i)
	{
		FBString str(pScene->Shaders[i]->Name);
		if ( strcmp(shaderName, str) == 0 )
		{
			return pScene->Shaders[i];
		}
	}
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// LayerItem class

void LayerItem::Default()
{
	FilePathOverride = false;
	FilePath = "C:\\";
	FileNameOverride = false;
	FileName = "output.avi";
	
	TimeOverride = false;
	StartTime = FBTime::Zero;
	StopTime = FBTime::Zero;
#ifdef OLD_FBTIME_SETFRAME
	StepTime.SetTime(0,0,0,1);
#else
	StepTime.SetFrame(1);
#endif

	PictureOverride = false;
	PictureMode = kFBResolutionCustom;
	FieldOverride = false;
	FieldMode = FBFieldModeNoField;
	PixelOverride = false;
	PixelMode = FBVideoRender24Bits;

	FormatOverride = false;
	Format = "TIF";
	Codec = "{ Uncompressed }";

	CamerasOverride = false;
	Cameras.Clear();

	ScriptOverride = false;
	Script = "";
	ScriptPost = "";

	AudioOverride = false;
	Audio = false;

	DisplayOverride = false;
	DisplayMode = FBViewingModeModelsOnly;
	ShowTime = false;
	ShowSafe = false;
	ShowLabel = false;
	Antialiasing = false;
	Matte = true;
}

void LayerItem::CheckOptions(LayerItem *pMasterLayer)
{
	// copy params from master layer
	if (!IsMaster() && !FilePathOverride) FilePath = pMasterLayer->FilePath;
	if (!IsMaster() && !FileNameOverride) FileName = pMasterLayer->FileName;
	
	if (!IsMaster() && !TimeOverride) {
		StartTime = pMasterLayer->StartTime;
		StopTime = pMasterLayer->StopTime;
		StepTime = pMasterLayer->StepTime;
	}

	if (!IsMaster() && !PictureOverride) PictureMode = pMasterLayer->PictureMode;
	if (!IsMaster() && !FieldMode) FieldMode = pMasterLayer->FieldMode;
	if (!IsMaster() && !PixelOverride) PixelMode = pMasterLayer->PixelMode;

	if (!IsMaster() && !FormatOverride) {
		Format = pMasterLayer->Format;
		Codec = pMasterLayer->Codec;
	}

	if (!IsMaster() && !CamerasOverride) {
		Cameras.Clear();
		for (int i=0; i<pMasterLayer->Cameras.GetCount(); ++i)
			Cameras.Add( pMasterLayer->Cameras[i] );
	}

	if (!IsMaster() && !ScriptOverride) 
	{
		Script = pMasterLayer->Script;
		ScriptPost = pMasterLayer->ScriptPost;
	}

	if (!IsMaster() && !AudioOverride) Audio = pMasterLayer->Audio;

	if (!IsMaster() && !DisplayOverride) {
		DisplayMode = pMasterLayer->DisplayMode;
		ShowTime = pMasterLayer->ShowTime;
		ShowSafe = pMasterLayer->ShowSafe;
		ShowLabel = pMasterLayer->ShowLabel;
		Antialiasing = pMasterLayer->Antialiasing;
		Matte = pMasterLayer->Matte;
	}

	//
	// check cameras, path, etc.
	//
	if (Cameras.GetCount() == 0) Cameras.Add( FBSystem().Renderer->CurrentCamera );
	if (FilePath == "" || !dirExists(FilePath))
	{
		FilePath = "C:\\";
	}

	int lastNdx = FilePath.GetLen()-1;
	if (FilePath.ReverseFind('\\') != lastNdx && FilePath.ReverseFind('/') != lastNdx)
		FilePath = FilePath + "\\";

	if (FileName == "")
	{
		FileName = FBApplication().FBXFileName;
		FileName = FileName + FBString("_") + FBString(Name);
	}
	else
	{
		// check extension
		if (FileName.Find('.') > 0)
		{
			int ndx = FileName.ReverseFind('.');
			FileName = FileName.Left( ndx+1 );
			FileName = FileName + Format;
		}
	}
}

// FBX store/retrieve
bool LayerItem::FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	// common attributes
	pFbxObject->FieldWriteI( (int) Master );
	pFbxObject->FieldWriteC( Name );
	pFbxObject->FieldWriteI( (int) Renderable );
	pFbxObject->FieldWriteC( (Group) ? Group->Name : "{None}" );

	// options
	pFbxObject->FieldWriteI( (int) FilePathOverride );
	pFbxObject->FieldWriteC( FilePath );
	pFbxObject->FieldWriteI( (int) FileNameOverride );
	pFbxObject->FieldWriteC( FileName );
	
	pFbxObject->FieldWriteI( (int) TimeOverride );
	pFbxObject->FieldWriteI( StartTime.GetFrame() );
	pFbxObject->FieldWriteI( StopTime.GetFrame() );
	pFbxObject->FieldWriteI( StepTime.GetFrame() );

	pFbxObject->FieldWriteI( (int) PictureOverride );
	pFbxObject->FieldWriteI( (int) PictureMode );
	pFbxObject->FieldWriteI( (int) FieldOverride );
	pFbxObject->FieldWriteI( (int) FieldMode );
	pFbxObject->FieldWriteI( (int) PixelOverride );
	pFbxObject->FieldWriteI( (int) PixelMode );

	pFbxObject->FieldWriteI( (int) FormatOverride );
	pFbxObject->FieldWriteC( Format );
	pFbxObject->FieldWriteC( Codec );

	pFbxObject->FieldWriteI( (int) CamerasOverride );
	pFbxObject->FieldWriteI( Cameras.GetCount() );
	for (int i=0; i<Cameras.GetCount(); ++i)
		pFbxObject->FieldWriteC( Cameras[i]->Name );

	pFbxObject->FieldWriteI( (int) ScriptOverride );
	pFbxObject->FieldWriteC( Script );
	pFbxObject->FieldWriteC( ScriptPost );

	pFbxObject->FieldWriteI( (int) AudioOverride );
	pFbxObject->FieldWriteI( (int) Audio );

	pFbxObject->FieldWriteI( (int) DisplayOverride );
	pFbxObject->FieldWriteI( (int) DisplayMode );
	pFbxObject->FieldWriteI( (int) ShowTime );
	pFbxObject->FieldWriteI( (int) ShowSafe );
	pFbxObject->FieldWriteI( (int) ShowLabel );
	pFbxObject->FieldWriteI( (int) Antialiasing );
	pFbxObject->FieldWriteI( (int) Matte );	// matte objects out of a layer group

	// output a layer state info
	State.FbxStore(pFbxObject, pStoreWhat);

	return true;
}

bool LayerItem::FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (pStoreWhat == kAttributes)
	{
		Master = (pFbxObject->FieldReadI( ) != 0);
		SetName( pFbxObject->FieldReadC() );
		Renderable = (pFbxObject->FieldReadI( ) != 0);
		GroupName = pFbxObject->FieldReadC();

		FilePathOverride = (pFbxObject->FieldReadI() != 0);
		FilePath = pFbxObject->FieldReadC();
		FileNameOverride = (pFbxObject->FieldReadI() != 0);
		FileName = pFbxObject->FieldReadC();
	
		TimeOverride = (pFbxObject->FieldReadI() != 0);
#ifdef OLD_FBTIME_SETFRAME
		StartTime.SetTime( 0,0,0, pFbxObject->FieldReadI() );
		StopTime.SetTime( 0,0,0, pFbxObject->FieldReadI() );
		StepTime.SetTime( 0,0,0, pFbxObject->FieldReadI() );
#else
		StartTime.SetFrame( pFbxObject->FieldReadI() );
		StopTime.SetFrame( pFbxObject->FieldReadI() );
		StepTime.SetFrame( pFbxObject->FieldReadI() );
#endif
		PictureOverride = (pFbxObject->FieldReadI() != 0);
		PictureMode = (FBCameraResolutionMode) pFbxObject->FieldReadI();
		FieldOverride = (pFbxObject->FieldReadI() != 0);
		FieldMode = (FBVideoRenderFieldMode) pFbxObject->FieldReadI();
		PixelOverride = (pFbxObject->FieldReadI() != 0);
		PixelMode = (FBVideoRenderDepth) pFbxObject->FieldReadI();

		FormatOverride = (pFbxObject->FieldReadI() != 0);
		Format = pFbxObject->FieldReadC();
		Codec = pFbxObject->FieldReadC();

		CamerasOverride = (pFbxObject->FieldReadI() != 0);
		CamerasNames.Clear();
		int count = pFbxObject->FieldReadI();
		for (int i=0; i<count; ++i)
		{
			CamerasNames.Add( pFbxObject->FieldReadC(), i );
		}

		ScriptOverride = (pFbxObject->FieldReadI() != 0);
		Script = pFbxObject->FieldReadC();

		AudioOverride = (pFbxObject->FieldReadI() != 0);
		Audio = (pFbxObject->FieldReadI() != 0);

		DisplayOverride = (pFbxObject->FieldReadI() != 0);
		DisplayMode = (FBVideoRenderViewingMode) pFbxObject->FieldReadI();
		ShowTime = (pFbxObject->FieldReadI() != 0);
		ShowSafe = (pFbxObject->FieldReadI() != 0);
		ShowLabel = (pFbxObject->FieldReadI() != 0);
		Antialiasing = (pFbxObject->FieldReadI() != 0);
		Matte = (pFbxObject->FieldReadI() != 0);

		// retreive a layer state info
		State.FbxRetrieve( pFbxObject, pStoreWhat );
	}
	else if (pStoreWhat == kCleanup)
	{
		SetGroup(GroupName);
		Cameras.Clear();
		for (int j=0; j<CamerasNames.GetCount(); ++j)
		{
			FBCamera *pCamera = FindCameraByName(CamerasNames[j]);
			if (pCamera) Cameras.Add(pCamera);
		}

		// setup state data
		State.FbxRetrieve( pFbxObject, pStoreWhat );
	}

	return true;
}


FBString ConvertTypeToString(FBSceneChangeType type)
{
	switch(type)
	{
		case kFBSceneChangeNone: return "kFBSceneChangeNone";
		case kFBSceneChangeDestroy: return "kFBSceneChangeDestroy";
		case kFBSceneChangeAttach: return "kFBSceneChangeAttach";
		case kFBSceneChangeDetach: return "kFBSceneChangeDetach";
		case kFBSceneChangeAddChild: return "kFBSceneChangeAddChild";
		case kFBSceneChangeRemoveChild: return "kFBSceneChangeRemoveChild";
		case kFBSceneChangeSelect: return "kFBSceneChangeSelect";
		case kFBSceneChangeUnselect: return "kFBSceneChangeUnselect";
		case kFBSceneChangeRename: return "kFBSceneChangeRename";
		case kFBSceneChangeRenamePrefix: return "kFBSceneChangeRenamePrefix";
		case kFBSceneChangeRenameUnique: return "kFBSceneChangeRenameUnique";
		case kFBSceneChangeRenameUniquePrefix: return "kFBSceneChangeRenameUniquePrefix";
		case kFBSceneChangeRenamed: return "kFBSceneChangeRenamed";
		case kFBSceneChangeRenamedPrefix: return "kFBSceneChangeRenamedPrefix";
		case kFBSceneChangeRenamedUnique: return "kFBSceneChangeRenamedUnique";
		case kFBSceneChangeRenamedUniquePrefix: return "kFBSceneChangeRenamedUniquePrefix";
		case kFBSceneChangeSoftSelect: return "kFBSceneChangeSoftSelect";
		case kFBSceneChangeSoftUnselect: return "kFBSceneChangeSoftUnselect";
		case kFBSceneChangeHardSelect: return "kFBSceneChangeHardSelect";
		case kFBSceneChangeActivate: return "kFBSceneChangeActivate";
		case kFBSceneChangeDeactivate: return "kFBSceneChangeDeactivate";
		case kFBSceneChangeLoadBegin: return "kFBSceneChangeLoadBegin";
		case kFBSceneChangeLoadEnd: return "kFBSceneChangeLoadEnd";
		case kFBSceneChangeClearBegin: return "kFBSceneChangeClearBegin";
		case kFBSceneChangeClearEnd: return "kFBSceneChangeClearEnd";
		case kFBSceneChangeTransactionBegin: return "kFBSceneChangeTransactionBegin";
		case kFBSceneChangeTransactionEnd: return "kFBSceneChangeTransactionEnd";
#ifndef OLD_SCENE_EVENTS
		case kFBSceneChangeMergeTransactionBegin: return "kFBSceneChangeMergeTransactionBegin";
		case kFBSceneChangeMergeTransactionEnd: return "kFBSceneChangeMergeTransactionEnd";
#endif
		case kFBSceneChangeReSelect: return "kFBSceneChangeReSelect";
		case kFBSceneChangeChangeName: return "kFBSceneChangeChangeName";
		case kFBSceneChangeChangedName: return "kFBSceneChangeChangedName";
		case kFBSceneChangePreParent: return "kFBSceneChangePreParent";
		case kFBSceneChangePreUnparent: return "kFBSceneChangePreUnparent";
		case kFBSceneChangeFocus: return "kFBSceneChangeFocus";
		case kFBSceneChangeChangedParent: return "kFBSceneChangeChangedParent";
		case kFBSceneChangeReorder: return "kFBSceneChangeReorder";
		case kFBSceneChangeReordered: return "kFBSceneChangeReordered";
	}
	return "kFBSceneChangeNone";
}