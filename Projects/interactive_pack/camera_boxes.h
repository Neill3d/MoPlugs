#ifndef __CAMERA_SHAKE_BOX_H__
#define __CAMERA_SHAKE_BOX_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: camera_boxes.h
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

//--- Registration defines
#define	BOXCAMERASHAKE__CLASSNAME		BoxCameraShake
#define BOXCAMERASHAKE__CLASSSTR		"BoxCameraShake"

#define	BOXCAMERAZOOM__CLASSNAME		BoxCameraZoom
#define BOXCAMERAZOOM__CLASSSTR		"BoxCameraZoom"

/**	BoxCameraShake class - running interactive camera shake effect.
*/
class BoxCameraShake : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( BoxCameraShake, FBBox );

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

	FBPropertyListObject		Camera;

private:
	FBSystem			mSystem;

	FBAnimationNode		*mPullNode;			//!< out

	FBAnimationNode		*mActionNode;		//!< in - run shake effect
	FBAnimationNode		*mFrequencyNode;
	FBAnimationNode		*mAmplitudeNode;
	FBAnimationNode		*mDurationNode;
	FBAnimationNode		*mRepeatOnActive;
	FBAnimationNode		*mUseCurrentCameraNode;

	bool				mShaking;
};

/////////////////////////////////////////////////////////////////////////

/**	BoxCameraZoom class - running interactive camera zoom effect.
*/
class BoxCameraZoom : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( BoxCameraZoom, FBBox );

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

	FBPropertyListObject		Camera;

private:
	FBSystem			mSystem;

	FBAnimationNode		*mPullNode;			//!< out

	FBAnimationNode		*mActionNode;		//!< in - run shake effect
	FBAnimationNode		*mAmplitudeNode;
	FBAnimationNode		*mDurationNode;
	FBAnimationNode		*mRepeatOnActive;
	FBAnimationNode		*mUseCurrentCameraNode;

	bool				mZooming;
};


#endif /* __CAMERA_SHAKE_BOX_H__ */
