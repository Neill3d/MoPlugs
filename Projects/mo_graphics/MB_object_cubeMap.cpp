
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_object_cubeMap.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "MB_object_cubeMap.h"
#include "algorithm\nv_math.h"
#include "graphics\checkglerror_MOBU.h"
#include <Windows.h>
#include "nv_dds\nv_dds.h"

#include <Windows.h>
#include <CommDlg.h>

/** Class implementation.
*   This should be placed in the source code file for a class that derives
*   from FBComponent.
*   It is absolutely necessary in order to identify the class type.
*/

FBClassImplementation(ObjectCubeMap)
FBUserObjectImplement(ObjectCubeMap, "CubeMap Texture", FB_DEFAULT_SDK_ICON);			//Register UserObject class
FBElementClassImplementation(ObjectCubeMap, FB_DEFAULT_SDK_ICON);			//Register to the asset system

extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);


/////////////////////////////////////////////////////////////////////////////////////////
// ObjectBlendSolver

void ObjectCubeMap::SetReLoadShader(HIObject object, bool value)
{
	ObjectCubeMap *pBase = FBCast<ObjectCubeMap>(object);
	if (pBase && value) 
	{
		pBase->DoReloadShader();
	}
}

void ObjectCubeMap::SetClearAction(HIObject object, bool value)
{
	ObjectCubeMap *pBase = FBCast<ObjectCubeMap>(object);
	if (pBase && value) 
	{
		pBase->OnClearClick();
	}
}

void ObjectCubeMap::SetLoadAsAction(HIObject object, bool value)
{
	ObjectCubeMap *pBase = FBCast<ObjectCubeMap>(object);
	if (pBase && value) 
	{
		pBase->OnLoadAsClick();
	}
}

void ObjectCubeMap::SetReloadAction(HIObject object, bool value)
{
	ObjectCubeMap *pBase = FBCast<ObjectCubeMap>(object);
	if (pBase && value) 
	{
		pBase->OnReloadClick();
	}
}

int ObjectCubeMap::GetCubeMapId(HIObject object)
{
	ObjectCubeMap *pBase = FBCast<ObjectCubeMap>(object);
	if (pBase) 
	{
		return pBase->GetCubeMapPtr()->GetCubeMapId();
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
ObjectCubeMap::ObjectCubeMap(const char *pName, HIObject pObject)
		: FBUserObject( pName, pObject )
{
	FBClassInit;

}

bool ObjectCubeMap::FBCreate()
{
	FBPropertyPublish(this, Active, "Active", nullptr, nullptr);

#ifdef _DEBUG
	FBPropertyPublish(this, ReLoadShader, "Reload shader", nullptr, SetReLoadShader);
	FBPropertyPublish(this, Test, "Test", nullptr, nullptr);
#endif

	FBPropertyPublish(this, Type, "Type", nullptr, nullptr);

	FBPropertyPublish(this, FileName, "File Name", nullptr, nullptr);
	FBPropertyPublish(this, LoadCompressed, "Load Compressed", nullptr, nullptr);
	FBPropertyPublish(this, LoadMipMaps, "Load MipMaps", nullptr, nullptr);
	FBPropertyPublish(this, Loaded, "Loaded", nullptr, nullptr);
	FBPropertyPublish(this, Status, "Status", nullptr, nullptr);

	FBPropertyPublish(this, Clear, "Clear", nullptr, SetClearAction);
	FBPropertyPublish(this, LoadAs, "Load As", nullptr, SetLoadAsAction);
	FBPropertyPublish(this, Reload, "Reload", nullptr, SetReloadAction);

	FBPropertyPublish(this, TextureFront, "Texture Front", nullptr, nullptr);
	FBPropertyPublish(this, TextureBack, "Texture Back", nullptr, nullptr);
	FBPropertyPublish(this, TextureTop, "Texture Top", nullptr, nullptr);
	FBPropertyPublish(this, TextureBottom, "Texture Bottom", nullptr, nullptr);
	FBPropertyPublish(this, TextureLeft, "Texture Left", nullptr, nullptr);
	FBPropertyPublish(this, TextureRight, "Texture Right", nullptr, nullptr);

	FBPropertyPublish(this, ImportSideTextures, "Import Side Textures", nullptr, nullptr);
	FBPropertyPublish(this, CreateFromTextureStack, "Create From Texture Stack", nullptr, nullptr);

	FBPropertyPublish(this, Dimention, "Dimention", nullptr, nullptr);
	FBPropertyPublish(this, MipLevels, "Mip Levels", nullptr, nullptr);
	FBPropertyPublish(this, Format, "Format", nullptr, nullptr);
	FBPropertyPublish(this, MemorySize, "Memory Size", nullptr, nullptr);

	FBPropertyPublish(this, CubeMapId, "CubeMap Id", GetCubeMapId, nullptr);

	FBPropertyPublish(this, RenderToRect, "Render To Rect", nullptr, nullptr);
	FBPropertyPublish(this, RectTexture, "Rect Texture", nullptr, nullptr);

	Active = true;
	Test = false;

	Type = kFBCubeMapFile;

	FileName = "";
	LoadCompressed = false;
	LoadMipMaps = false;
	Loaded = false;
	Status = "";

	Dimention = 0;
	MipLevels = 0;
	Format = "";
	MemorySize = 0;

	TextureFront.SetFilter( FBVideoClip::GetInternalClassId() );
	TextureFront.SetSingleConnect(true);
	TextureBack.SetFilter( FBVideoClip::GetInternalClassId() );
	TextureBack.SetSingleConnect(true);
	TextureTop.SetFilter( FBVideoClip::GetInternalClassId() );
	TextureTop.SetSingleConnect(true);
	TextureBottom.SetFilter( FBVideoClip::GetInternalClassId() );
	TextureBottom.SetSingleConnect(true);
	TextureLeft.SetFilter( FBVideoClip::GetInternalClassId() );
	TextureLeft.SetSingleConnect(true);
	TextureRight.SetFilter( FBVideoClip::GetInternalClassId() );
	TextureRight.SetSingleConnect(true);

	Dimention.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	MipLevels.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	Format.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	MemorySize.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	CubeMapId.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	FBPropertyListObject *pTextureProps[6] = {&TextureFront, &TextureBack, &TextureTop, &TextureBottom,
		&TextureLeft, &TextureRight };

	for (int i=0; i<6; ++i)
	{
		pTextureProps[i]->SetSingleConnect(true);
		pTextureProps[i]->SetFilter( FBTexture::GetInternalClassId() );
	}
	
	RenderToRect = false;
	//RectTextureId = 0;
	mNeedUpdate = true;

	mSystem.OnUIIdle.Add( this, (FBCallback) &ObjectCubeMap::OnSystemIdle );

	return ParentClass::FBCreate();
}

void ObjectCubeMap::FBDestroy()
{
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Remove(this, (FBCallback)&ObjectCubeMap::OnPerFrameRenderingPipelineCallback);
	
	ParentClass::FBDestroy();
}

bool ObjectCubeMap::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if (pStoreWhat == kCleanup)
	{
		mNeedUpdate = true;
	}
	return ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);
}

bool ObjectCubeMap::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ObjectCubeMap::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void ObjectCubeMap::OnSystemIdle (HISender pSender, HKEvent pEvent)
{
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Add(this, (FBCallback)&ObjectCubeMap::OnPerFrameRenderingPipelineCallback);

	mSystem.OnUIIdle.Remove( this, (FBCallback) &ObjectCubeMap::OnSystemIdle );
}

void ObjectCubeMap::OnPerFrameRenderingPipelineCallback    (HISender pSender, HKEvent pEvent)
{
	if (false == Active)
		return;

	static bool firstTime = true;

	if (true == firstTime)
	{
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
		firstTime = false;
	}

	
	FBEventEvalGlobalCallback lFBEvent(pEvent);

	switch(lFBEvent.GetTiming() )
	{
	case kFBGlobalEvalCallbackBeforeRender:
		Update();
		break;
	case kFBGlobalEvalCallbackAfterRender:
		
		break;
	}
}

// update when ogl context has changed
void ObjectCubeMap::ChangeContext()
{
}

void ObjectCubeMap::ChangeGlobalSettings()
{
}

void ObjectCubeMap::DoReloadShader()
{
	mNeedProgramReload = true;
}

void ObjectCubeMap::OnClearClick()
{
	
}

void ObjectCubeMap::OnLoadAsClick()
{
	FBFilePopup	lDialog;

	lDialog.Style = kFBFilePopupOpen;
	lDialog.Filter = "*.dds";

	if (lDialog.Execute() )
	{
		Type = kFBCubeMapFile;

		FileName = lDialog.FullFilename;
		mNeedUpdate = true;
	}
}

void ObjectCubeMap::OnReloadClick()
{
	mNeedUpdate = true;
}

void ObjectCubeMap::OnCreateFromTextureStackClick()
{
	if ( TextureFront.GetCount() > 0 && TextureBack.GetCount() > 0
		&& TextureTop.GetCount() > 0 && TextureBottom.GetCount() > 0
		&& TextureLeft.GetCount() > 0 && TextureRight.GetCount() > 0 )
	{
		Type = kFBCubeMapImages;
		mNeedUpdate = true;
	}
}

void ObjectCubeMap::OnImportSideTexturesClick(bool showDialog)
{
	FBVideoClip *front= (TextureFront.GetCount() > 0) ? (FBVideoClip*)TextureFront.GetAt(0) : nullptr;
	FBVideoClip *back= (TextureBack.GetCount() > 0) ? (FBVideoClip*)TextureBack.GetAt(0) : nullptr;
	FBVideoClip *top= (TextureTop.GetCount() > 0) ? (FBVideoClip*)TextureTop.GetAt(0) : nullptr;
	FBVideoClip *bottom= (TextureBottom.GetCount() > 0) ? (FBVideoClip*)TextureBottom.GetAt(0) : nullptr;
	FBVideoClip *left= (TextureLeft.GetCount() > 0) ? (FBVideoClip*)TextureLeft.GetAt(0) : nullptr;
	FBVideoClip *right= (TextureRight.GetCount() > 0) ? (FBVideoClip*)TextureRight.GetAt(0) : nullptr;

	if (front==nullptr && back==nullptr && top==nullptr && bottom==nullptr && left==nullptr && right==nullptr)
	{
		// launch a dialog to choose skybox textures
		if (showDialog && 1 == FBMessageBox( "SkyBox Shader", "Do you want to choose 6 files for skybox ?\n Order: negx-left, negy-bottom, negz-front, posx-right, posy-top, posz-back", "Ok", "Cancel" ) )
		{

			OPENFILENAME ofn;
			char szFileName[MAX_PATH] = "";

			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
			ofn.hwndOwner = 0;
			ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;
			ofn.lpstrDefExt = "";

			char szFilePath[MAX_PATH] = "";
			char szFiles[6][MAX_PATH];
			int numberOfFiles = 0;

			if(GetOpenFileName(&ofn))
			{
				// Do something usefull with the filename stored in szFileName 
				
				memcpy( szFilePath, ofn.lpstrFile, sizeof(char) * ofn.nFileOffset );
				
				char *src = &ofn.lpstrFile[ofn.nFileOffset];
				char *dst = &szFiles[0][0];
				while(numberOfFiles < 6)
				{
					
					if (*src != NULL)
					{
						*dst = *src;
						dst++;
					}
					else
					{
						if ( dst == &szFiles[numberOfFiles][0] ) 
						{
							break;
						}
						else
						{
							*dst = '\0';
							numberOfFiles++;
							dst = &szFiles[numberOfFiles][0];
						}
					}

					src++;
				}

				//
				//

				if (numberOfFiles == 6)
				{
					FBString path(szFilePath);
					path = path + "\\";

					//load textures
					FBString filename( path, szFiles[0] );
					left = new FBVideoClip( filename );
					TextureLeft.Add(left);
					
					filename = path + szFiles[1];
					bottom = new FBVideoClip( filename );
					TextureBottom.Add(bottom);

					filename = path + szFiles[2];
					front = new FBVideoClip( filename );
					TextureFront.Add(front);

					filename = path + szFiles[3];
					right = new FBVideoClip( filename );
					TextureRight.Add(right);

					filename = path + szFiles[4];
					top = new FBVideoClip( filename );
					TextureTop.Add(top);

					filename = path + szFiles[5];
					back = new FBVideoClip( filename );
					TextureBack.Add(back);
				}
				else
				{
					if (showDialog)
						FBMessageBox( "Skybox Shader", "Please choose 6 files for skybox", "Ok" );
				}
			}
		}
	}
}

void ObjectCubeMap::Update()
{
	if (false == mNeedUpdate || false == Active)
		return;

	FBCubeMapType	type = Type;

	switch(type)
	{
	case kFBCubeMapFile:
		UpdateFromFile();
		break;
	case kFBCubeMapImages:
		UpdateFromImages();
		break;
	case kFBCubeMapPanorama:
		UpdateFromPanorama();
		break;
	}

	mNeedUpdate = false;
}

bool ObjectCubeMap::UpdateFromFile()
{
	FBString filename = FileName;
	if (0 == filename.GetLen() )
		return false;

	const bool mipmaps = (LoadMipMaps.AsInt() > 0);

	mCubeMap.LoadCubeMap(filename);
	if (mipmaps)
		mCubeMap.GenerateMipMaps();

	Status = "Loaded";

	return true;
}

void ObjectCubeMap::UpdateFromImages()
{
	if (TextureFront.GetCount() > 0 && TextureBack.GetCount() > 0
		&& TextureTop.GetCount() > 0 && TextureBottom.GetCount() > 0
		&& TextureLeft.GetCount() > 0 && TextureRight.GetCount() > 0 )
	{
		FBTexture *pTextures[6] = { (FBTexture*) TextureFront.GetAt(0),
			(FBTexture*) TextureBack.GetAt(0), (FBTexture*) TextureTop.GetAt(0),
			(FBTexture*) TextureBottom.GetAt(0), (FBTexture*) TextureLeft.GetAt(0),
			(FBTexture*) TextureRight.GetAt(0) };

		bool isOk = true;
		const bool mipmaps = (LoadMipMaps.AsInt() > 0);

		FBVideoClip *pClips[6];
		for (int i=0; i<6; ++i)
		{
			FBVideo *pVideo = pTextures[i]->Video;

			if (pVideo != nullptr && FBIS(pVideo, FBVideoClip) )
				pClips[i] = (FBVideoClip*) pVideo;
			else
			{
				isOk = false;
				break;
			}
		}

		if (true == isOk)
		{
			mCubeMap.createCubeMap(pClips[0], pClips[1], pClips[2], 
				pClips[3], pClips[4], pClips[5] );

			if (mipmaps)
				mCubeMap.GenerateMipMaps();

			Status = "Loaded";
		}
	}
	
}

void ObjectCubeMap::UpdateFromPanorama()
{
}