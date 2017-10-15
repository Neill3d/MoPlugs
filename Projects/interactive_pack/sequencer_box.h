#ifndef __ORBOXTRANSPORT_BOX_H__
#define __ORBOXTRANSPORT_BOX_H__

/**	\file	orboxtransport_box.h
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define	ORBOXTRANSPORT__CLASSNAME		ORBoxImageSequencer
#define ORBOXTRANSPORT__CLASSSTR		"ORBoxImageSequencer"

/**	Template for FBBox class.
*/
class ORBoxImageSequencer : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( ORBoxImageSequencer, FBBox );

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

	FBPropertyListObject		Video;
	
private:
	
	FBAnimationNode*	mPull;
	FBAnimationNode		*mTrigEnd;

	FBAnimationNode		*mInput;
	FBAnimationNode		*mLoop;
	FBAnimationNode		*mLoopStart;
	FBAnimationNode		*mLoopStop;

	bool			mInputActive;
	
	bool			mPlaying;
	FBTime			mStopTime;
	double			mLastTriggerEnd;

	HIObject		mReference;
};

#endif /* __ORBOXTRANSPORT_BOX_H__ */
