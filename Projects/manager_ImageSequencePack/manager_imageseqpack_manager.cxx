
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: manager_imageseqpack_manager.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "manager_imageseqpack_manager.h"

#include <algorithm>

#include <Windows.h>
#include "BLUnZip.h"



//--- Registration defines
#define MANAGER_IMAGESEQ_PACK__CLASS MANAGER_IMAGESEQ_PACK__CLASSNAME
#define MANAGER_IMAGESEQ_PACK__NAME  MANAGER_IMAGESEQ_PACK__CLASSSTR

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation(MANAGER_IMAGESEQ_PACK__CLASS);  // Manager class name.
FBRegisterCustomManager(MANAGER_IMAGESEQ_PACK__CLASS);         // Manager class name.

/************************************************
 *  FiLMBOX Creation
 ************************************************/
bool Manager_ImageSeqPack::FBCreate()
{
	mNeedReCache = false;
	mEvaluate = true;
	mUseSystemIdle = false;

	mDefaultId = 0;

	// ?! ddsPack, ddsSeq
	FBVideoCodecManager::RegisterExternalVideoFormat("imgPack");

    return true;
}


/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void Manager_ImageSeqPack::FBDestroy()
{
    // Free any user memory here.
}


/************************************************
 *  Execution callback.
 ************************************************/
bool Manager_ImageSeqPack::Init()
{
    return true;
}

bool Manager_ImageSeqPack::Open()
{
	mSystem.Scene->OnChange.Add( this, (FBCallback) &Manager_ImageSeqPack::OnSceneChange );
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add(this, (FBCallback)&Manager_ImageSeqPack::OnRender);

    return true;
}

bool Manager_ImageSeqPack::Clear()
{
    return true;
}

bool Manager_ImageSeqPack::Close()
{
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove(this, (FBCallback)&Manager_ImageSeqPack::OnRender);

    return true;
}

void Manager_ImageSeqPack::OnSceneClear(HISender pSender, HKEvent pEvent)
{
	mNeedReCache = true;
}

bool Manager_ImageSeqPack::CheckArchive(FBVideoClipImage	*pclip)
{
	FBProperty *prop = pclip->PropertyList.Find("Path");
	if (prop == nullptr)
		return false;

	if (strstr(prop->AsString(), ".imgPack") == nullptr)
		return false;


	if (false == BLUnZip::TryOpen( prop->AsString() ) )
		return false;

	return true;
}

void Manager_ImageSeqPack::OnSceneChange(HISender pSender, HKEvent pEvent)
{
	FBEventSceneChange	levent(pEvent);

	FBComponent *comp = levent.Component;
	FBComponent *child = levent.ChildComponent;
	/*
	if (comp)
	{
		FBString name( comp->Name );
		printf( "name - %s, classname - %s\n", name, comp->ClassName() );
	}
	if (child)
	{
		FBString name( child->Name );
		printf( "name - %s, classname - %s\n", name, child->ClassName() );
	}
	*/

	if ( FBIS(comp, FBScene) && FBIS(child, FBVideoClipImage))
	{
		FBVideoClipImage *pVideo = (FBVideoClipImage*) (FBComponent*) child;

		if (false == CheckArchive(pVideo))
			return;

		
		switch (levent.Type)
		{
		case kFBSceneChangeAttach:
			{

				FBVideoMemory *pNewVideo = new FBVideoMemory( FBString( "sequence_", pVideo->Name ) );
				mVideos.push_back( VIDEO(pVideo, pNewVideo) );

				FBProperty *prop = pNewVideo->PropertyCreate( "Seq Path", kFBPT_charptr, "string", false, false );
				if (prop)
					prop->SetString( pVideo->Filename );

				if (mUseSystemIdle == false)
				{
					mSystem.OnUIIdle.Add( this, (FBCallback) &Manager_ImageSeqPack::OnSystemIdle );
					mUseSystemIdle = true;
				}

			} break;
		}

	}
	else if (FBIS(comp, FBScene) && FBIS(child, FBVideoMemory))
	{
		FBVideoMemory *pVideo = (FBVideoMemory*) (FBComponent*) child;

		FBProperty *prop = pVideo->PropertyList.Find("Seq Path");
		if (prop == nullptr)
			return;

		switch (levent.Type)
		{
		case kFBSceneChangeDetach:
		{
			auto iter = mVideos.begin();
			
			while (iter != mVideos.end() )
			{
				if (iter->mMedia == pVideo)
					break;

				iter++;
			}
			
			if (iter != mVideos.end())
				mVideos.erase(iter);

		} break;
			
		case kFBSceneChangeMergeTransactionBegin:
		case kFBSceneChangeClearBegin:
		case kFBSceneChangeLoadBegin:
			mEvaluate = false;
			break;

		case kFBSceneChangeMergeTransactionEnd:
		case kFBSceneChangeClearEnd:
		case kFBSceneChangeLoadEnd:
			mEvaluate = true;
			mNeedReCache = true;
			break;

		}
	}
}

void Manager_ImageSeqPack::OnSystemIdle(HISender pSender, HKEvent pEvent)
{
	mSystem.OnUIIdle.Remove( this, (FBCallback) &Manager_ImageSeqPack::OnSystemIdle );
	mUseSystemIdle = false;

	for (auto iter = mVideos.begin(); iter != mVideos.end(); ++iter)
	{
		FBVideoMemory *pVideo = iter->mMedia;

		pVideo->SetObjectImageSize(2, 2);
		pVideo->TextureOGLId = mDefaultId;

		// cleanup source media

		if (iter->mSourceMedia != nullptr)
		{
			FBVideoClipImage *pClip = iter->mSourceMedia;

			for (int i=0, count = pClip->GetDstCount(); i<count; ++i)
			{
				if (FBIS( pClip->GetDst(i), FBTexture ) )
				{
					((FBTexture*) pClip->GetDst(i) )->Video = pVideo;
				}
			}

			pClip->FBDelete();

			iter->mSourceMedia = nullptr;
		}
	}
}

void Manager_ImageSeqPack::OnRender(HISender pSender, HKEvent pEvent)
{
	FBEventEvalGlobalCallback levent(pEvent);

	if (mDefaultId == 0)
		CreateDefaultTexture();

	if (mNeedReCache)
		CacheScene();

	if (mEvaluate && levent.GetTiming() == kFBGlobalEvalCallbackBeforeRender)
	{
		// update all dynamic videos from texture pack

		for (auto iter = mVideos.begin(); iter != mVideos.end(); ++iter)
		{
			FBVideoMemory *pVideo = iter->mMedia;

			pVideo->SetObjectImageSize(2, 2);
			pVideo->TextureOGLId = mDefaultId;
		}

	}
}

void Manager_ImageSeqPack::CreateDefaultTexture()
{
	glGenTextures( 1, &mDefaultId );

	glBindTexture(GL_TEXTURE_2D, mDefaultId);

	unsigned char pixels[2*2*3] = { 0, 0, 0,
									255, 255, 255,
									0, 0, 0,
									255, 255, 255 };
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Manager_ImageSeqPack::CacheScene()
{
	mNeedReCache = false;

	mVideos.clear();

	for (int i = 0; i < mSystem.Scene->VideoClips.GetCount(); ++i)
	{

		if (FBIS(mSystem.Scene->VideoClips[i], FBVideoClipImage) )
		{
			FBVideoClipImage *pVideo = (FBVideoClipImage*) mSystem.Scene->VideoClips[i]; 

			if (false == CheckArchive(pVideo))
				continue;

			//
			FBVideoMemory *pNewVideo = new FBVideoMemory( FBString( "sequence_", pVideo->Name ) );
			mVideos.push_back( VIDEO(pVideo, pNewVideo) );

			FBProperty *prop = pNewVideo->PropertyCreate( "Seq Path", kFBPT_charptr, "string", false, false );
			if (prop)
				prop->SetString( pVideo->Filename );

		}
		else
		if (FBIS(mSystem.Scene->VideoClips[i], FBVideoMemory))
		{
			FBVideoMemory *pVideo = (FBVideoMemory*) mSystem.Scene->VideoClips[i];

			FBProperty *prop = pVideo->PropertyList.Find("Seq Path");
			if (prop == nullptr)
				continue;

			mVideos.push_back( VIDEO( nullptr, pVideo) );
		}
	}

	// check scene for temproary medias
	mSystem.OnUIIdle.Add( this, (FBCallback) &Manager_ImageSeqPack::OnSystemIdle );
}
