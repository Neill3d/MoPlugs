#ifndef __MANAGER_IMAGESEQPACK_MANAGER_H__
#define __MANAGER_IMAGESEQPACK_MANAGER_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: manager_imageseqpack_manager.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

// glew
#include <GL\glew.h>

// STL
#include <vector>

//--- Registration defines
#define MANAGER_IMAGESEQ_PACK__CLASSNAME Manager_ImageSeqPack
#define MANAGER_IMAGESEQ_PACK__CLASSSTR  "Manager_ImageSeqPack"

/** Custom Manager Template.
*/
class Manager_ImageSeqPack : public FBCustomManager
{
    //--- FiLMBOX box declaration.
	FBCustomManagerDeclare(Manager_ImageSeqPack);

public:
	// used to remove attached FBVideoClip, we need to replace it with FBVideoMemory (dynamically control each frame)
	void OnSystemIdle(HISender pSender, HKEvent pEvent);

	void OnSceneClear(HISender pSender, HKEvent pEvent);
	void OnSceneChange(HISender pSender, HKEvent pEvent);
	void OnRender(HISender pSender, HKEvent pEvent);

public:
    virtual bool FBCreate();        //!< FiLMBOX creation function.
    virtual void FBDestroy();       //!< FiLMBOX destruction function.

    virtual bool Init();
    virtual bool Open();
    virtual bool Clear();
    virtual bool Close();

private:

	FBSystem						mSystem;

	struct VIDEO {
		FBVideoClipImage		*mSourceMedia; // motionbuilder adds this into the scene
		FBVideoMemory			*mMedia;

		BLUnZip					mArchive;

		//
		VIDEO()	
		{
			mSourceMedia = nullptr;
			mMedia = nullptr;
		}
		VIDEO( FBVideoClipImage *sourceMedia, FBVideoMemory *media )
			: mSourceMedia(sourceMedia)
			, mMedia(media)
		{}
		VIDEO( const VIDEO &b )
		{
			mSourceMedia = b.mSourceMedia;
			mMedia = b.mMedia;
		}

	};

	std::vector<VIDEO>				mVideos;
	bool							mEvaluate;
	bool							mNeedReCache;

	bool							mUseSystemIdle;
	
	GLuint							mDefaultId;

	void							CreateDefaultTexture();
	void							CacheScene();

	bool					CheckArchive(FBVideoClipImage	*pclip);
};

#endif /* __MANAGER_IMAGESEQPACK_MANAGER_H__ */
