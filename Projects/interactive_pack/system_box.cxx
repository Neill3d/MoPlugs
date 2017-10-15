
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: system_box.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "system_box.h"
#include "IO\FileUtils.h"
#include "Common.h"

//--- Registration defines
#define BOXSYSTEM__CLASS		BOXSYSTEM__CLASSNAME
#define BOXSYSTEM__NAME			BOXSYSTEM__CLASSSTR
#define	BOXSYSTEM__LOCATION		"Interactive"
#define BOXSYSTEM__LABEL		"System"
#define	BOXSYSTEM__DESC			"System Box"

#define BOXTAKELIST__CLASS		BOXTAKELIST__CLASSNAME
#define BOXTAKELIST__NAME		BOXTAKELIST__CLASSSTR
#define	BOXTAKELIST__LOCATION	"Interactive"
#define BOXTAKELIST__LABEL		"Take List"
#define	BOXTAKELIST__DESC		"Take List Box"

#define BOXCAMERALIST__CLASS	BOXCAMERALIST__CLASSNAME
#define BOXCAMERALIST__NAME		BOXCAMERALIST__CLASSSTR
#define	BOXCAMERALIST__LOCATION	"Interactive"
#define BOXCAMERALIST__LABEL	"Camera List"
#define	BOXCAMERALIST__DESC		"Camera List Box"

//--- TRANSPORT implementation and registration
FBBoxImplementation	(	BOXSYSTEM__CLASS	);	// Box class name
FBRegisterBox		(	BOXSYSTEM__NAME,		// Unique name to register box.
						BOXSYSTEM__CLASS,		// Box class name
						BOXSYSTEM__LOCATION,	// Box location ('plugins')
						BOXSYSTEM__LABEL,		// Box label (name of box to display)
						BOXSYSTEM__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	BOXTAKELIST__CLASS	);	// Box class name
FBRegisterBox		(	BOXTAKELIST__NAME,		// Unique name to register box.
						BOXTAKELIST__CLASS,		// Box class name
						BOXTAKELIST__LOCATION,	// Box location ('plugins')
						BOXTAKELIST__LABEL,		// Box label (name of box to display)
						BOXTAKELIST__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	BOXCAMERALIST__CLASS	);	// Box class name
FBRegisterBox		(	BOXCAMERALIST__NAME,		// Unique name to register box.
						BOXCAMERALIST__CLASS,		// Box class name
						BOXCAMERALIST__LOCATION,	// Box location ('plugins')
						BOXCAMERALIST__LABEL,		// Box label (name of box to display)
						BOXCAMERALIST__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool BoxSystem::FBCreate()
{
	FBPropertyPublish( this, Take, "Take", nullptr, nullptr );
	FBPropertyPublish( this, Camera, "Camera", nullptr, nullptr );
	FBPropertyPublish( this, FileName, "File Name", nullptr, nullptr );

	Take.SetFilter( FBTake::GetInternalClassId() );
	Take.SetSingleConnect( true );

	Camera.SetFilter( FBCamera::GetInternalClassId() );
	Camera.SetSingleConnect( true );

	FileName = "";

	/*
	*	Create the nodes for the box.
	*/
	
	mPull = AnimationNodeOutCreate	( 0, "Must Pull",	ANIMATIONNODE_TYPE_NUMBER );
	
	mChangeTake = AnimationNodeInCreate	( 1, "Change Take",		ANIMATIONNODE_TYPE_BOOL );
	mChangeTakeByIndex = AnimationNodeInCreate	( 2, "Take By Index",		ANIMATIONNODE_TYPE_BOOL );
	mChangeTakeIndex = AnimationNodeInCreate	( 3, "Take Index",		ANIMATIONNODE_TYPE_INTEGER );
	mGoToBeginning = AnimationNodeInCreate	( 4, "Take GoTo Beginning",		ANIMATIONNODE_TYPE_BOOL );
	mContinuePlayingNode = AnimationNodeInCreate	( 5, "Take Cont. Playing",		ANIMATIONNODE_TYPE_BOOL );

	mChangeCamera = AnimationNodeInCreate	( 5, "Change Camera",		ANIMATIONNODE_TYPE_BOOL );
	mChangeCameraByIndex = AnimationNodeInCreate	( 6, "Camera By Index",		ANIMATIONNODE_TYPE_BOOL );
	mChangeCameraIndex = AnimationNodeInCreate	( 7, "Camera Index",		ANIMATIONNODE_TYPE_INTEGER );
	
	mChangeFile = AnimationNodeInCreate	( 8, "Change File",		ANIMATIONNODE_TYPE_BOOL );

	mPlay = AnimationNodeInCreate	( 9, "Play",		ANIMATIONNODE_TYPE_BOOL );

	mSystem.OnUIIdle.Add( this, (FBCallback) &BoxSystem::OnSystemIdle );

	mBeginning = 0.0;
	mContinuePlaying = 0.0;

	mReferenceCamera = -1;
	mReferenceTake = -1;

	mNeedFileActivated = false;
	mNeedFileChange = false;

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void BoxSystem::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
	mSystem.OnUIIdle.Remove( this, (FBCallback) &BoxSystem::OnSystemIdle );
}

void BoxSystem::OnSystemIdle(HISender pSender, HKEvent pEvent)
{
	/*
	if (mNeedFileChange)
	{
		bool isPlaying = FBPlayerControl::TheOne().IsPlaying;
		if (true == FBApplication::TheOne().FileNew() && IsFileExists( FileName ) )
		{
			if (true == FBApplication::TheOne().FileOpen( FileName ) )
			{
				if (mNeedFileChange)
					FBPlayerControl::TheOne().Play();
				//mSystem.OnUIIdle.Remove( this, (FBCallback) &BoxSystem::OnSystemIdle );
				return;
			}
			return;
		}

		mNeedFileChange = false;
	}
	*/

	if (mReferenceCamera >= 0 || mReferenceTake >= 0)
	{
		Camera.Clear();
		Take.Clear();

		FBScene *pScene = mSystem.Scene;

		if (mReferenceCamera >= 0 && mReferenceCamera < pScene->Cameras.GetCount() )
		{
			Camera.Add( pScene->Cameras[mReferenceCamera] );
		}
	
		//
		if (mReferenceTake >= 0 && mReferenceTake < pScene->Takes.GetCount() )
		{
			Take.Add( pScene->Takes[mReferenceTake] );
		}

		//
		mReferenceCamera = -1;
		mReferenceTake = -1;
	}

	if (mTakeFlags.needChange)
	{
		bool isPlaying = FBPlayerControl::TheOne().IsPlaying;

		if (mTakeFlags.index >= 0 && mTakeFlags.index < mSystem.Scene->Takes.GetCount() )
		{
			mSystem.CurrentTake = mSystem.Scene->Takes[ mTakeFlags.index ];	
		}
		else if (Take.GetCount() > 0)
		{
			mSystem.CurrentTake = (FBTake*) Take[0];
		}

		if (mBeginning > 0.0)
		{
			FBPlayerControl::TheOne().GotoStart();
		}

		if (/*isPlaying &&*/ mContinuePlaying > 0.0)
			FBPlayerControl::TheOne().Play();

		mTakeFlags.needChange = false;
	}

	//
	if (mCameraFlags.needChange)
	{
		if (mCameraFlags.index >= 0 && mCameraFlags.index < mSystem.Scene->Cameras.GetCount() )
		{
			mSystem.Renderer->CurrentCamera = mSystem.Scene->Cameras[ mCameraFlags.index ];
		}
		else if (Camera.GetCount() > 0)
		{
			mSystem.Renderer->CurrentCamera = (FBCamera*) Camera[0];
		}
		mCameraFlags.needChange = false;
	}
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxSystem::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lChange, lChangeByIndex, lValueIndex;
	bool lStatusChange, lStatusChangeByIndex, lStatusIndex, lStatusBeginning, lStatusPlaying;

	
	bool result = true;

	lStatusChange			= mChangeTake->ReadData( &lChange, pEvaluateInfo );
	lStatusChangeByIndex	= mChangeTakeByIndex->ReadData( &lChangeByIndex, pEvaluateInfo );
	lStatusIndex			= mChangeTakeIndex->ReadData( &lValueIndex, pEvaluateInfo );
	lStatusBeginning		= mGoToBeginning->ReadData( &mBeginning, pEvaluateInfo );
	lStatusPlaying			= mContinuePlayingNode->ReadData( &mContinuePlaying, pEvaluateInfo );

	if (lStatusChange == false)
		lChange = 0.0;
	if (lStatusChangeByIndex == false)
		lChangeByIndex = 0.0;
	if (lStatusIndex == false)
		lValueIndex = -1.0;
	if (lStatusBeginning == false)
		mBeginning = 1.0;
	if (lStatusPlaying == false)
		mContinuePlaying = 1.0;

	if (lChangeByIndex == 0.0)
	{
		mTakeFlags.hasChanged = false;
	}
	if( lChangeByIndex != 0.0 && mTakeFlags.hasChanged == false )
	{
		mTakeFlags.index = lValueIndex;
		mTakeFlags.hasChanged = true;
		mTakeFlags.needChange = true;
	}

	if (lChangeByIndex == 0.0)
	{
		if (lChange == 0.0)
		{
			mTakeFlags.hasChanged = false;
		}
		if( lChange != 0.0 && mTakeFlags.hasChanged == false )
		{
			mTakeFlags.index = -1;
			mTakeFlags.hasChanged = true;
			mTakeFlags.needChange = true;
		}
	}
	

	//
	lStatusChange			= mChangeCamera->ReadData( &lChange, pEvaluateInfo );
	lStatusChangeByIndex	= mChangeCameraByIndex->ReadData( &lChangeByIndex, pEvaluateInfo );
	lStatusIndex			= mChangeCameraIndex->ReadData( &lValueIndex, pEvaluateInfo );
	
	if (lStatusChange == false)
		lChange = 0.0;
	if (lStatusChangeByIndex == false)
		lChangeByIndex = 0.0;
	if (lStatusIndex == false)
		lValueIndex = -1.0;

	if (lChangeByIndex == 0.0)
	{
		mCameraFlags.hasChanged = false;
	}
	if( lChangeByIndex != 0.0 && mCameraFlags.hasChanged == false )
	{
		mCameraFlags.index = lValueIndex;
		mCameraFlags.hasChanged = true;
		mCameraFlags.needChange = true;
	}

	if (lChangeByIndex == 0.0)
	{
		if (lChange == 0.0)
		{
			mCameraFlags.hasChanged = false;
		}
		if( lChange != 0.0 && mCameraFlags.hasChanged == false )
		{
			mCameraFlags.index = -1;
			mCameraFlags.hasChanged = true;
			mCameraFlags.needChange = true;
		}
	}

	//
	//

	lStatusChange = mChangeFile->ReadData( &lChange, pEvaluateInfo );

	if (lStatusChange == false)
		lChange = 0.0;

	if (mNeedFileActivated == false && lChange > 0.0)
	{
		mNeedFileActivated = true;
		mNeedFileChange = true;

		OpenNextFile(FileName);
		result = true;
	}
	else if (lChange == 0.0)
	{
		mNeedFileActivated = false;
	}

	//
	//
	lStatusChange = mPlay->ReadData( &lChange, pEvaluateInfo );
	if (lStatusChange == false)
		lChange = 0.0;

	if (lChange > 0.0)
	{
		if (mPlayerControl.GetTransportMode() != kFBTransportPlay)
			mPlayerControl.Play();
		result = true;
	}

	//
	mPull->WriteData( &lChange, pEvaluateInfo );

	return result;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool BoxSystem::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/

	if (pStoreWhat == kAttributes)
	{
		FBScene *pScene = mSystem.Scene;

		if (Camera.GetCount() > 0)
		{
			pFbxObject->FieldWriteBegin("boxSystemCamera");
			int index = pScene->Cameras.Find( Camera.GetAt(0) );
			pFbxObject->FieldWriteI( index );
			pFbxObject->FieldWriteEnd();
		}
		//
		if (Take.GetCount() > 0)
		{
			pFbxObject->FieldWriteBegin("boxSystemTake");
			int index = pScene->Takes.Find( Take.GetAt(0) );
			pFbxObject->FieldWriteI( index );
			pFbxObject->FieldWriteEnd();
		}

		pFbxObject->FieldWriteBegin( "boxSystemFile");
		pFbxObject->FieldWriteC( FileName );
		pFbxObject->FieldWriteEnd();
	}


	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool BoxSystem::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	if (pStoreWhat == kAttributes)
	{
		mReferenceCamera = -1;
		mReferenceTake = -1;

		if (pFbxObject->FieldReadBegin( "boxSystemCamera" ) )
		{
			mReferenceCamera = pFbxObject->FieldReadI();
			pFbxObject->FieldReadEnd();
		}
		//
		if (pFbxObject->FieldReadBegin( "boxSystemTake" ) )
		{
			mReferenceTake = pFbxObject->FieldReadI();
			pFbxObject->FieldReadEnd();
		}
		//
		if (pFbxObject->FieldReadBegin( "boxSystemFile" ) )
		{
			FileName = pFbxObject->FieldReadC();
			pFbxObject->FieldReadEnd();
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Take List


/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool BoxTakeList::FBCreate()
{
	FBPropertyPublish( this, Takes, "Takes", nullptr, nullptr );
	FBPropertyPublish( this, ExcludeTakes, "Exclude Takes", nullptr, nullptr );

	Takes.SetFilter( FBTake::GetInternalClassId() );
	Takes.SetSingleConnect( false );

	ExcludeTakes.SetFilter( FBTake::GetInternalClassId() );
	ExcludeTakes.SetSingleConnect( false );

	/*
	*	Create the nodes for the box.
	*/
	
	mPull = AnimationNodeOutCreate	( 0, "Must Pull",	ANIMATIONNODE_TYPE_NUMBER );
	mIndex = AnimationNodeOutCreate	( 1, "Index",	ANIMATIONNODE_TYPE_NUMBER );

	mAction = AnimationNodeInCreate	( 2, "Action",		ANIMATIONNODE_TYPE_BOOL );
	mRandom = AnimationNodeInCreate	( 3, "Random",		ANIMATIONNODE_TYPE_BOOL );
	mUseCustomTakes = AnimationNodeInCreate	( 4, "Use Custom",		ANIMATIONNODE_TYPE_BOOL );

	mLastOutIndex = 0.0;
	mCurrIndex = 0;
	mHasStarted = false;

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void BoxTakeList::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
	
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxTakeList::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lAction, lRandom, lCustom;
	bool lStatusAction, lStatusRandom, lStatusCustom;

	
	bool result = false;

	lStatusAction = mAction->ReadData( &lAction, pEvaluateInfo );
	
	
	if (lStatusAction)
	{
		lStatusRandom = mRandom->ReadData( &lRandom, pEvaluateInfo );
		lStatusCustom = mUseCustomTakes->ReadData( &lCustom, pEvaluateInfo );

		if (lStatusRandom == false) lRandom = 0.0;
		if (lStatusCustom == false) lCustom = (Takes.GetCount() > 0) ? 1.0 : 0.0;

		if (lAction == 0.0)
		{
			mHasStarted = false;
		}
		else if (lAction == 1.0 && mHasStarted == false)
		{
			FBTake *newTake = nullptr;
			FBScene *pScene = mSystem.Scene;

			// TODO: do some action
			if (Takes.GetCount() > 0 && lCustom > 0.0)
			{
				// iterate through custom takes
				int count = Takes.GetCount();
				std::uniform_int_distribution<int> uni(0, count-1); // guaranteed unbiased

				int iter = 0;
				while( newTake == nullptr && iter < 10 )
				{

					if (lRandom > 0.0)
					{
						int random = uni(rng);
						newTake = (FBTake*) Takes[random];
					}
					else
					{
						if (mCurrIndex < count-1 )
							mCurrIndex += 1;
						else
							mCurrIndex = 0;
						
						newTake = (FBTake*) Takes[mCurrIndex];
					}

					// use exclude list to skip takes
					if( newTake != nullptr )
					{
						for (int i=0; i<ExcludeTakes.GetCount(); ++i)
							if (newTake == ExcludeTakes[i])
							{
								newTake = nullptr;
								break;
							}
					}

					iter += 1;
				}
				
				if (newTake == nullptr)
					newTake = (FBTake*) Takes[0];
			}
			else if (pScene->Takes.GetCount() > 1)
			{
				// iterate through scene takes
				
				int count = pScene->Takes.GetCount();
				std::uniform_int_distribution<int> uni(0, count-1); // guaranteed unbiased

				int iter = 0;
				while( newTake == nullptr && iter < 10 )
				{

					if (lRandom > 0.0)
					{
						int random = uni(rng);
						newTake = (FBTake*) pScene->Takes[random];
					}
					else
					{
						if (mCurrIndex < count-1 )
							mCurrIndex += 1;
						else
							mCurrIndex = 0;
						
						newTake = (FBTake*) pScene->Takes[mCurrIndex];
					}

					// use exclude list to skip takes
					if( newTake != nullptr )
					{
						for (int i=0; i<ExcludeTakes.GetCount(); ++i)
							if (newTake == ExcludeTakes[i])
							{
								newTake = nullptr;
								break;
							}
					}

					iter += 1;
				}

				if (newTake == nullptr)
					newTake = (FBTake*) pScene->Takes[0];
			}

			if (newTake != nullptr)
			{
				int count = pScene->Takes.GetCount();

				for (int i=0; i<count; ++i)
					if (newTake == pScene->Takes[i])
					{
						mLastOutIndex = (double) i;
						break;
					}
			}

			mHasStarted = true;
		}

		mIndex->WriteData( &mLastOutIndex, pEvaluateInfo );
		mPull->WriteData( &lAction, pEvaluateInfo );
		result = true;
	}

	return result;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool BoxTakeList::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/

	if (pStoreWhat == kAttributes)
	{
		FBScene *pScene = mSystem.Scene;

		if (Takes.GetCount() > 0)
		{
			pFbxObject->FieldWriteBegin("boxTakeListTakes");
			pFbxObject->FieldWriteI( Takes.GetCount() );
			for (int i=0; i<Takes.GetCount(); ++i)
			{
				FBComponent *pcomp = Takes.GetAt(i);
				int takeIndex = pScene->Takes.Find(pcomp);

				pFbxObject->FieldWriteI( takeIndex );
			}
			pFbxObject->FieldWriteEnd();
		}
		//
		if (ExcludeTakes.GetCount() > 0)
		{
			pFbxObject->FieldWriteBegin("boxTakeListExclude");
			pFbxObject->FieldWriteI( ExcludeTakes.GetCount() );
			for (int i=0; i<ExcludeTakes.GetCount(); ++i)
			{
				FBComponent *pcomp = ExcludeTakes.GetAt(i);
				int takeIndex = pScene->Takes.Find(pcomp);
				
				pFbxObject->FieldWriteI( takeIndex );
			}
			pFbxObject->FieldWriteEnd();
		}
	}

	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool BoxTakeList::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/

	if (pStoreWhat == kAttributes)
	{
		mReferenceTakes.clear();
		mReferenceExcludeTakes.clear();

		if (pFbxObject->FieldReadBegin( "boxTakeListTakes" ) )
		{
			int count = pFbxObject->FieldReadI();
			mReferenceTakes.resize(count);
			for (int i=0; i<count; ++i)
			{
				int pref = pFbxObject->FieldReadI();
				mReferenceTakes[i] = pref;
			}
			pFbxObject->FieldReadEnd();
		}
		//
		if (pFbxObject->FieldReadBegin( "boxTakeListExclude" ) )
		{
			int count = pFbxObject->FieldReadI();
			mReferenceExcludeTakes.resize(count);
			for (int i=0; i<count; ++i)
			{
				int pref = pFbxObject->FieldReadI();
				mReferenceExcludeTakes[i] = pref;
			}
			pFbxObject->FieldReadEnd();
		}

		mSystem.OnUIIdle.Add( this, (FBCallback) &BoxTakeList::OnSystemIdle );
	}

	return true;
}


void BoxTakeList::ProcessReferences()
{
	if (mReferenceTakes.size() > 0 || mReferenceExcludeTakes.size() > 0)
	{
		FBScene *pScene = mSystem.Scene;

		Takes.Clear();
		ExcludeTakes.Clear();

		for (auto iter=mReferenceTakes.begin(); iter!=mReferenceTakes.end(); ++iter)
		{
			int index = *iter;

			if (index >=0 && index < pScene->Takes.GetCount() )
				Takes.Add( pScene->Takes[index] );
		}

		//
		for (auto iter=mReferenceExcludeTakes.begin(); iter!=mReferenceExcludeTakes.end(); ++iter)
		{
			int index = *iter;

			if (index >= 0 && index < pScene->Takes.GetCount() )
				ExcludeTakes.Add( pScene->Takes[index] );
		}

		//
		mReferenceTakes.clear();
		mReferenceExcludeTakes.clear();
	}
}

void BoxTakeList::OnSystemIdle(HISender pSender, HKEvent pEvent)
{
	ProcessReferences();
	mSystem.OnUIIdle.Remove( this, (FBCallback) &BoxTakeList::OnSystemIdle );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Camera List


/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool BoxCameraList::FBCreate()
{
	FBPropertyPublish( this, Cameras, "Cameras", nullptr, nullptr );
	FBPropertyPublish( this, ExcludeCameras, "Exclude Cameras", nullptr, nullptr );

	Cameras.SetFilter( FBCamera::GetInternalClassId() );
	Cameras.SetSingleConnect( false );

	ExcludeCameras.SetFilter( FBCamera::GetInternalClassId() );
	ExcludeCameras.SetSingleConnect( false );

	/*
	*	Create the nodes for the box.
	*/
	
	mPull = AnimationNodeOutCreate	( 0, "Must Pull",	ANIMATIONNODE_TYPE_NUMBER );
	mIndex = AnimationNodeOutCreate	( 1, "Index",	ANIMATIONNODE_TYPE_NUMBER );

	mAction = AnimationNodeInCreate	( 2, "Action",		ANIMATIONNODE_TYPE_BOOL );
	mRandom = AnimationNodeInCreate	( 3, "Random",		ANIMATIONNODE_TYPE_BOOL );
	mUseCustomCameras = AnimationNodeInCreate	( 4, "Use Custom",		ANIMATIONNODE_TYPE_BOOL );
	mExcludeSystem = AnimationNodeInCreate	( 5, "Exclude System",		ANIMATIONNODE_TYPE_BOOL );

	mLastOutIndex = 0.0;
	mCurrIndex = 0;
	mHasStarted = false;

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void BoxCameraList::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
	
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxCameraList::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lAction, lRandom, lCustom, lExcludeSystem;
	bool lStatusAction, lStatusRandom, lStatusCustom, lStatusExclude;

	
	bool result = false;

	lStatusAction = mAction->ReadData( &lAction, pEvaluateInfo );
	
	
	if (lStatusAction)
	{
		lStatusRandom = mRandom->ReadData( &lRandom, pEvaluateInfo );
		lStatusCustom = mUseCustomCameras->ReadData( &lCustom, pEvaluateInfo );
		lStatusExclude = mExcludeSystem->ReadData( &lExcludeSystem, pEvaluateInfo );

		if (lStatusRandom == false) lRandom = 0.0;
		if (lStatusCustom == false) lCustom = (Cameras.GetCount() > 0) ? 1.0 : 0.0;
		if (lStatusExclude == false) lExcludeSystem = 1.0;

		if (lAction == 0.0)
		{
			mHasStarted = false;
		}
		else if (lAction == 1.0 && mHasStarted == false)
		{
			FBCamera *newCamera = nullptr;
			FBScene *pScene = mSystem.Scene;

			// DONE: do some action
			if (Cameras.GetCount() > 0 && lCustom > 0.0)
			{
				// iterate through custom takes
				int count = Cameras.GetCount();
				std::uniform_int_distribution<int> uni(0, count-1); // guaranteed unbiased

				int iter = 0;
				while( newCamera == nullptr && iter < 10 )
				{

					if (lRandom > 0.0)
					{
						int random = uni(rng);
						newCamera = (FBCamera*) Cameras[random];
					}
					else
					{
						if (mCurrIndex < count-1 )
							mCurrIndex += 1;
						else
							mCurrIndex = 0;
						
						newCamera = (FBCamera*) Cameras[mCurrIndex];
					}

					// use exclude list to skip takes
					if( newCamera != nullptr )
					{
						for (int i=0; i<ExcludeCameras.GetCount(); ++i)
							if (newCamera == ExcludeCameras[i])
							{
								newCamera = nullptr;
								break;
							}
					}

					iter += 1;
				}
				
				if (newCamera == nullptr)
					newCamera = (FBCamera*) Cameras[0];
			}
			else if (pScene->Cameras.GetCount() > 1)
			{
				// iterate through scene takes
				int count = pScene->Cameras.GetCount();

				int startIndex=-1;

				if (lExcludeSystem > 0.0)
				{
					for (int i=0; i<count; ++i)
						if (pScene->Cameras[i]->SystemCamera == false)
						{
							startIndex = i;
							break;
						}
				}
				else
				{
					startIndex = 0;
				}

				if (startIndex >= 0)
				{
					std::uniform_int_distribution<int> uni(startIndex, count-1); // guaranteed unbiased

					if (mCurrIndex < startIndex)
						mCurrIndex = startIndex;

					int iter = 0;
					while( newCamera == nullptr && iter < 10 )
					{

						if (lRandom > 0.0)
						{
							int random = uni(rng);
							newCamera = pScene->Cameras[random];
						}
						else
						{
							if (mCurrIndex < count-1 )
								mCurrIndex += 1;
							else
								mCurrIndex = startIndex;
						
							newCamera = pScene->Cameras[mCurrIndex];
						}

						// use exclude list to skip takes
						if( newCamera != nullptr )
						{
							for (int i=0; i<ExcludeCameras.GetCount(); ++i)
								if (newCamera == ExcludeCameras[i])
								{
									newCamera = nullptr;
									break;
								}
						}

						iter += 1;
					}
				}

				if (newCamera == nullptr)
					newCamera = (FBCamera*) pScene->Cameras[0];
			}

			if (newCamera != nullptr)
			{
				int count = pScene->Cameras.GetCount();

				for (int i=0; i<count; ++i)
					if (newCamera == pScene->Cameras[i])
					{
						mLastOutIndex = (double) i;
						break;
					}
			}

			mHasStarted = true;
		}

		mIndex->WriteData( &mLastOutIndex, pEvaluateInfo );
		mPull->WriteData( &lAction, pEvaluateInfo );
		result = true;
	}

	return result;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool BoxCameraList::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	if (pStoreWhat == kAttributes)
	{
		FBScene *pScene = mSystem.Scene;

		if (Cameras.GetCount() > 0)
		{
			pFbxObject->FieldWriteBegin("boxCameraListCameras");
			pFbxObject->FieldWriteI( Cameras.GetCount() );
			for (int i=0; i<Cameras.GetCount(); ++i)
			{
				FBComponent *pcomp = Cameras.GetAt(i);
				int camIndex = pScene->Cameras.Find(pcomp);

				pFbxObject->FieldWriteI( camIndex );
			}
			pFbxObject->FieldWriteEnd();
		}
		//
		if (ExcludeCameras.GetCount() > 0)
		{
			pFbxObject->FieldWriteBegin("boxCameraListExclude");
			pFbxObject->FieldWriteI( ExcludeCameras.GetCount() );
			for (int i=0; i<ExcludeCameras.GetCount(); ++i)
			{
				FBComponent *pcomp = ExcludeCameras.GetAt(i);
				int camIndex = pScene->Cameras.Find(pcomp);
				
				pFbxObject->FieldWriteI( camIndex );
			}
			pFbxObject->FieldWriteEnd();
		}
	}

	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool BoxCameraList::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	if (pStoreWhat == kAttributes)
	{
		mReferenceCameras.clear();
		mReferenceExcludeCameras.clear();

		if (pFbxObject->FieldReadBegin( "boxCameraListTakes" ) )
		{
			int count = pFbxObject->FieldReadI();
			mReferenceCameras.resize(count);
			for (int i=0; i<count; ++i)
			{
				int pref = pFbxObject->FieldReadI();
				mReferenceCameras[i] = pref;
			}
			pFbxObject->FieldReadEnd();
		}
		//
		if (pFbxObject->FieldReadBegin( "boxCameraListExclude" ) )
		{
			int count = pFbxObject->FieldReadI();
			mReferenceExcludeCameras.resize(count);
			for (int i=0; i<count; ++i)
			{
				int pref = pFbxObject->FieldReadI();
				mReferenceExcludeCameras[i] = pref;
			}
			pFbxObject->FieldReadEnd();
		}

		mSystem.OnUIIdle.Add( this, (FBCallback) &BoxCameraList::OnSystemIdle );
	}

	return true;
}

void BoxCameraList::ProcessReferences()
{
	if (mReferenceCameras.size() > 0 || mReferenceExcludeCameras.size() > 0)
	{
		FBScene *pScene = mSystem.Scene;

		Cameras.Clear();
		ExcludeCameras.Clear();

		for (auto iter=mReferenceCameras.begin(); iter!=mReferenceCameras.end(); ++iter)
		{
			int index = *iter;

			if (index >=0 && index < pScene->Cameras.GetCount() )
				Cameras.Add( pScene->Cameras[index] );
		}

		//
		for (auto iter=mReferenceExcludeCameras.begin(); iter!=mReferenceExcludeCameras.end(); ++iter)
		{
			int index = *iter;

			if (index >= 0 && index < pScene->Cameras.GetCount() )
				ExcludeCameras.Add( pScene->Cameras[index] );
		}

		//
		mReferenceCameras.clear();
		mReferenceExcludeCameras.clear();
	}
}

void BoxCameraList::OnSystemIdle(HISender pSender, HKEvent pEvent)
{
	ProcessReferences();
	mSystem.OnUIIdle.Remove( this, (FBCallback) &BoxCameraList::OnSystemIdle );
}
