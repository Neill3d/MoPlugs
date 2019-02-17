#ifndef __SYSTEM_BOX_H__
#define __SYSTEM_BOX_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: system_box.h
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

#include <vector>
#include <random>

//--- Registration defines
#define	BOXSYSTEM__CLASSNAME		BoxSystem
#define BOXSYSTEM__CLASSSTR			"BoxSystem"

#define	BOXTAKELIST__CLASSNAME		BoxTakeList
#define BOXTAKELIST__CLASSSTR		"BoxTakeList"

#define	BOXCAMERALIST__CLASSNAME	BoxCameraList
#define BOXCAMERALIST__CLASSSTR		"BoxCamerasList"

/**	Template for FBBox class.
*/
class BoxSystem : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( BoxSystem, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval functionc
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	void	OnSystemIdle(HISender pSender, HKEvent pEvent);

	FBPropertyListObject		Take;
	FBPropertyListObject		Camera;

	FBPropertyString			FileName;

private:
	
	FBSystem			mSystem;
	FBPlayerControl		mPlayerControl;

	FBAnimationNode		*mPull;			//!< out

	FBAnimationNode		*mChangeTake;			//!< in - change take
	FBAnimationNode		*mChangeTakeByIndex;	//!< in - change take
	FBAnimationNode		*mChangeTakeIndex;		//!< in - change take
	FBAnimationNode		*mGoToBeginning;		//!< in - go to beginning on take change
	FBAnimationNode		*mContinuePlayingNode;	//!< continue playing after take has changed

	FBAnimationNode		*mChangeCamera;			//!< in - change camera
	FBAnimationNode		*mChangeCameraByIndex;	//!< in - change camera
	FBAnimationNode		*mChangeCameraIndex;	//!< in - change camera

	FBAnimationNode		*mChangeFile;			//!< in - change whole scene, load another file

	FBAnimationNode		*mPlay;

	struct Flags
	{
		int					index;			// -1 - means no change by index, change by property
		bool				hasChanged;
		bool				needChange;

		Flags()
		{
			index = -1;
			hasChanged = false;
			needChange = false;
		}
	};

	// for IO
	int					mReferenceCamera;
	int					mReferenceTake;

	double				mBeginning;
	double				mContinuePlaying;

	Flags				mTakeFlags;
	Flags				mCameraFlags;

	bool				mNeedFileActivated;
	bool				mNeedFileChange;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////
/**	Template for FBBox class.
*/
class BoxTakeList : public FBBox
{
	//--- box declaration.
	//FBBoxDeclare( BoxTakeList, FBBox );

	FBClassDeclare( BoxTakeList, FBBox );

public: 
    BoxTakeList(const char* pName)
		: FBBox(pName) 
		, rng(rd())
	{ 
		FBClassInit; 
	} 

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval functionc
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	void	OnSystemIdle(HISender pSender, HKEvent pEvent);
	
	FBPropertyListObject		Takes;	//
	FBPropertyListObject		ExcludeTakes;	// exclude these takes from the list

private:
	
	FBSystem			mSystem;

	FBAnimationNode		*mPull;			//!< out
	FBAnimationNode		*mIndex;		//!< out take index that has calculated

	FBAnimationNode		*mAction;		//!< go to the next index
	FBAnimationNode		*mRandom;		//!< choose next take in random order
	FBAnimationNode		*mUseCustomTakes;

	int		mCurrIndex;

	double mLastOutIndex;

	bool	mHasStarted;

	std::random_device		rd;
	std::mt19937			rng;	// random-number engine used

	std::vector<int>	mReferenceTakes;
	std::vector<int>	mReferenceExcludeTakes;

	void		ProcessReferences();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
/**	Template for FBBox class.
*/
class BoxCameraList : public FBBox
{
	//--- box declaration.
	//FBBoxDeclare( BoxCameraList, FBBox );

	FBClassDeclare( BoxCameraList, FBBox );

public: 
    BoxCameraList(const char* pName)
		: FBBox(pName) 
		, rng(rd())
	{ 
		FBClassInit; 
	} 

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval functionc
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	void	OnSystemIdle(HISender pSender, HKEvent pEvent);
	
	FBPropertyListObject		Cameras;	//
	FBPropertyListObject		ExcludeCameras;	// exclude these takes from the list

private:
	
	FBSystem			mSystem;

	FBAnimationNode		*mPull;			//!< out
	FBAnimationNode		*mIndex;		//!< out take index that has calculated

	FBAnimationNode		*mAction;		//!< go to the next index
	FBAnimationNode		*mRandom;		//!< choose next take in random order
	FBAnimationNode		*mUseCustomCameras;
	FBAnimationNode		*mExcludeSystem;	//!< exclude system cameras from the list

	int		mCurrIndex;

	double mLastOutIndex;

	bool	mHasStarted;

	std::random_device		rd;
	std::mt19937			rng;	// random-number engine used

	std::vector<int>	mReferenceCameras;
	std::vector<int>	mReferenceExcludeCameras;

	void		ProcessReferences();
};

#endif /* __SYSTEM_BOX_H__ */
