
/**	\file	orboxtransport_box.cxx
*/

//--- Class declaration
#include "box_sequencer_box.h"

//--- Registration defines
#define ORBOXTRANSPORT__CLASS		ORBOXTRANSPORT__CLASSNAME
#define ORBOXTRANSPORT__NAME		ORBOXTRANSPORT__CLASSSTR
#define	ORBOXTRANSPORT__LOCATION	"Neill3d"
#define ORBOXTRANSPORT__LABEL		"Image Sequencer Box"
#define	ORBOXTRANSPORT__DESC		"Image Sequencer Box"

//--- TRANSPORT implementation and registration
FBBoxImplementation	(	ORBOXTRANSPORT__CLASS	);	// Box class name
FBRegisterBox		(	ORBOXTRANSPORT__NAME,		// Unique name to register box.
						ORBOXTRANSPORT__CLASS,		// Box class name
						ORBOXTRANSPORT__LOCATION,	// Box location ('plugins')
						ORBOXTRANSPORT__LABEL,		// Box label (name of box to display)
						ORBOXTRANSPORT__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)
/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool ORBoxImageSequencer::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	mInput = AnimationNodeInCreate	( 0, "Input",		ANIMATIONNODE_TYPE_NUMBER );
	mLoop = AnimationNodeInCreate	( 1, "Loop",		ANIMATIONNODE_TYPE_BOOL );
	mLoopStart = AnimationNodeInCreate	( 2, "Loop Start",		ANIMATIONNODE_TYPE_INTEGER );
	mLoopStop = AnimationNodeInCreate	( 3, "Loop Stop",		ANIMATIONNODE_TYPE_INTEGER );
	
	mPull = AnimationNodeOutCreate	( 4, "Must Pull",			ANIMATIONNODE_TYPE_NUMBER );
	mTrigEnd = AnimationNodeOutCreate	( 5, "Trig End",		ANIMATIONNODE_TYPE_BOOL );

	FBPropertyPublish( this, Video, "Video", nullptr, nullptr );

	Video.SetSingleConnect(true);
	Video.SetFilter(FBVideoClip::GetInternalClassId() );

	mPlaying = false;
	mInputActive = false;
	mLastTriggerEnd = 0.0;

	mReference = 0;

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void ORBoxImageSequencer::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool ORBoxImageSequencer::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue, lLoop, lLoopStart, lLoopStop;
	bool lStatus;

	bool result = false;

	if (mReference != nullptr)
	{
		Video.Add( FBCast<FBVideoClip> (mReference) );
		mReference = nullptr;
	}

	if (Video.GetCount() == 0)
		return false;

	FBVideoClip *pVideo = (FBVideoClip*) Video.GetAt(0);
	lStatus = mLoop->ReadData( &lLoop, pEvaluateInfo );
	if (lStatus)
		pVideo->Loop = (lLoop > 0.0);

	lStatus = mLoopStart->ReadData( &lLoopStart, pEvaluateInfo );
	lStatus = mLoopStop->ReadData( &lLoopStop, pEvaluateInfo );


	//
	lStatus = mInput->ReadData( &lValue, pEvaluateInfo );
	if( lStatus )
	{
		mPull->WriteData( &lValue, pEvaluateInfo );
		if( lValue == 0.0 )
		{
			mInputActive = false;
		}
		else
		if( lValue != 0.0 && mInputActive == false )
		{
			pVideo->FreeRunning = false;

			pVideo->StartFrame = (int) lLoopStart;
			pVideo->StopFrame = (int) lLoopStop;

			pVideo->FreeRunning = true;

			mInputActive= true;
			mPlaying = true;
			mLastTriggerEnd = 0.0;
			mStopTime = pEvaluateInfo->GetSystemTime();
			mStopTime = mStopTime + FBTime(0,0,0, lLoopStop-lLoopStart);
		}

		if (mPlaying && lLoop == 0.0)
		{
			if (mStopTime < pEvaluateInfo->GetSystemTime() )
			{
				mPlaying = false;
				mLastTriggerEnd = 1.0;
				mInputActive = false;
			}
		}

		lStatus = mTrigEnd->WriteData( &mLastTriggerEnd, pEvaluateInfo );
		
		result = true;
	}

	return result;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool ORBoxImageSequencer::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/

	if (pStoreWhat == kAttributes)
	{
		if (Video.GetCount() > 0)
		{
			pFbxObject->FieldWriteBegin("boxImageBlock");
			pFbxObject->FieldWriteObjectReference( Video.GetAt(0) );
			pFbxObject->FieldWriteEnd();
		}
	}

	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORBoxImageSequencer::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/

	if (pStoreWhat == kAttributes)
	{
		if (pFbxObject->FieldReadBegin("boxImageBlock") )
		{
			mReference = pFbxObject->FieldReadObjectReference();
			pFbxObject->FieldReadEnd();
		}
		
	}

	return true;
}
