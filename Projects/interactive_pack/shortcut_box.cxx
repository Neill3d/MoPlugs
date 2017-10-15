
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shortcut_box.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "shortcut_box.h"

//--- Registration defines
#define BOXSHORTCUT__CLASS		BOXSHORTCUT__CLASSNAME
#define BOXSHORTCUT__NAME		BOXSHORTCUT__CLASSSTR
#define	BOXSHORTCUT__LOCATION	"Interactive"
#define BOXSHORTCUT__LABEL		"ShortCut2"
#define	BOXSHORTCUT__DESC		"ShortCut2"

#define BOXSHORTCUT3__CLASS		BOXSHORTCUT3__CLASSNAME
#define BOXSHORTCUT3__NAME		BOXSHORTCUT3__CLASSSTR
#define	BOXSHORTCUT3__LOCATION	"Interactive"
#define BOXSHORTCUT3__LABEL		"ShortCut3"
#define	BOXSHORTCUT3__DESC		"ShortCut3"

#define BOXSHORTCUTDOUBLE__CLASS		BOXSHORTCUTDOUBLE__CLASSNAME
#define BOXSHORTCUTDOUBLE__NAME			BOXSHORTCUTDOUBLE__CLASSSTR
#define	BOXSHORTCUTDOUBLE__LOCATION		"Interactive"
#define BOXSHORTCUTDOUBLE__LABEL		"ShortCutDouble"
#define	BOXSHORTCUTDOUBLE__DESC			"ShortCutDouble"

//--- TRANSPORT implementation and registration
FBBoxImplementation	(	BOXSHORTCUT__CLASS	);	// Box class name
FBRegisterBox		(	BOXSHORTCUT__NAME,		// Unique name to register box.
						BOXSHORTCUT__CLASS,		// Box class name
						BOXSHORTCUT__LOCATION,	// Box location ('plugins')
						BOXSHORTCUT__LABEL,		// Box label (name of box to display)
						BOXSHORTCUT__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	BOXSHORTCUT3__CLASS	);	// Box class name
FBRegisterBox		(	BOXSHORTCUT3__NAME,		// Unique name to register box.
						BOXSHORTCUT3__CLASS,		// Box class name
						BOXSHORTCUT3__LOCATION,	// Box location ('plugins')
						BOXSHORTCUT3__LABEL,		// Box label (name of box to display)
						BOXSHORTCUT3__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)


FBBoxImplementation	(	BOXSHORTCUTDOUBLE__CLASS	);	// Box class name
FBRegisterBox		(	BOXSHORTCUTDOUBLE__NAME,		// Unique name to register box.
						BOXSHORTCUTDOUBLE__CLASS,		// Box class name
						BOXSHORTCUTDOUBLE__LOCATION,	// Box location ('plugins')
						BOXSHORTCUTDOUBLE__LABEL,		// Box label (name of box to display)
						BOXSHORTCUTDOUBLE__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool BoxShortcut2::FBCreate()
{

	/*
	*	Create the nodes for the box.
	*/
	
	mOnlyFirst = AnimationNodeOutCreate	( 0, "Only First",	ANIMATIONNODE_TYPE_NUMBER );
	mOnlySecond = AnimationNodeOutCreate	( 1, "Only Second",	ANIMATIONNODE_TYPE_NUMBER );
	mBoth = AnimationNodeOutCreate	( 2, "Both",	ANIMATIONNODE_TYPE_NUMBER );
	mNone = AnimationNodeOutCreate	( 3, "None",	ANIMATIONNODE_TYPE_NUMBER );

	mAction1 = AnimationNodeInCreate	( 4, "Action 1",		ANIMATIONNODE_TYPE_NUMBER );
	mAction2 = AnimationNodeInCreate	( 5, "Action 2",		ANIMATIONNODE_TYPE_NUMBER );

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void BoxShortcut2::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxShortcut2::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue1, lValue2;
	bool lStatus1, lStatus2;

	
	lStatus1 = mAction1->ReadData( &lValue1, pEvaluateInfo );
	if (lStatus1 == false) lValue1 = 0.0;

	lStatus2 = mAction2->ReadData( &lValue2, pEvaluateInfo );
	if (lStatus2 == false) lValue2 = 0.0;
	
	double one = 1.0;
	double zero = 0.0;

	if (lValue1 > 0.0 && lValue2 > 0.0)
	{
		mOnlyFirst->WriteData( &zero, pEvaluateInfo );
		mOnlySecond->WriteData( &zero, pEvaluateInfo );
		mBoth->WriteData( &one, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
	}
	else if (lValue1 > 0.0)
	{
		mOnlyFirst->WriteData( &one, pEvaluateInfo );
		mOnlySecond->WriteData( &zero, pEvaluateInfo );
		mBoth->WriteData( &zero, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
	}
	else if (lValue2 > 0.0)
	{
		mOnlyFirst->WriteData( &zero, pEvaluateInfo );
		mOnlySecond->WriteData( &one, pEvaluateInfo );
		mBoth->WriteData( &zero, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
	}
	else
	{
		mOnlyFirst->WriteData( &zero, pEvaluateInfo );
		mOnlySecond->WriteData( &zero, pEvaluateInfo );
		mBoth->WriteData( &zero, pEvaluateInfo );
		mNone->WriteData( &one, pEvaluateInfo );
	}

	return true;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool BoxShortcut2::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool BoxShortcut2::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//

/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool BoxShortcut3::FBCreate()
{

	/*
	*	Create the nodes for the box.
	*/
	
	mOnlyFirst			= AnimationNodeOutCreate	( 0, "Only First",	ANIMATIONNODE_TYPE_NUMBER );
	mOnlySecond			= AnimationNodeOutCreate	( 1, "Only Second",	ANIMATIONNODE_TYPE_NUMBER );
	mOnlyThird			= AnimationNodeOutCreate	( 2, "Only Third",	ANIMATIONNODE_TYPE_NUMBER );
	mAll				= AnimationNodeOutCreate	( 3, "All",	ANIMATIONNODE_TYPE_NUMBER );
	mNone				= AnimationNodeOutCreate	( 4, "None",	ANIMATIONNODE_TYPE_NUMBER );
	mFirstAndSecond		= AnimationNodeOutCreate	( 5, "FirstSecond",	ANIMATIONNODE_TYPE_NUMBER );
	mFirstAndThird		= AnimationNodeOutCreate	( 6, "FirstThird",	ANIMATIONNODE_TYPE_NUMBER );
	mSecondAndThird		= AnimationNodeOutCreate	( 7, "SecondThird",	ANIMATIONNODE_TYPE_NUMBER );

	mAction1 = AnimationNodeInCreate	( 8, "Action 1",		ANIMATIONNODE_TYPE_NUMBER );
	mAction2 = AnimationNodeInCreate	( 9, "Action 2",		ANIMATIONNODE_TYPE_NUMBER );
	mAction3 = AnimationNodeInCreate	( 10, "Action 3",		ANIMATIONNODE_TYPE_NUMBER );

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void BoxShortcut3::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxShortcut3::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue1, lValue2, lValue3;
	bool lStatus1, lStatus2, lStatus3;

	
	lStatus1 = mAction1->ReadData( &lValue1, pEvaluateInfo );
	if (lStatus1 == false) lValue1 = 0.0;

	lStatus2 = mAction2->ReadData( &lValue2, pEvaluateInfo );
	if (lStatus2 == false) lValue2 = 0.0;
	
	lStatus3 = mAction3->ReadData( &lValue3, pEvaluateInfo );
	if (lStatus3 == false) lValue3 = 0.0;

	double one = 1.0;
	double zero = 0.0;

	mOnlyFirst->WriteData( &zero, pEvaluateInfo );
	mOnlySecond->WriteData( &zero, pEvaluateInfo );
	mOnlyThird->WriteData( &zero, pEvaluateInfo );
	mAll->WriteData( &zero, pEvaluateInfo );
	mNone->WriteData( &zero, pEvaluateInfo );
	mFirstAndSecond->WriteData( &zero, pEvaluateInfo );
	mFirstAndThird->WriteData( &zero, pEvaluateInfo );
	mSecondAndThird->WriteData( &zero, pEvaluateInfo );

	if (lValue1 > 0.0 && lValue2 > 0.0 && lValue3 > 0.0)
	{
		mOnlyFirst->WriteData( &zero, pEvaluateInfo );
		mOnlySecond->WriteData( &zero, pEvaluateInfo );
		mOnlyThird->WriteData( &zero, pEvaluateInfo );
			mAll->WriteData( &one, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
		mFirstAndSecond->WriteData( &zero, pEvaluateInfo );
		mFirstAndThird->WriteData( &zero, pEvaluateInfo );
		mSecondAndThird->WriteData( &zero, pEvaluateInfo );
	}
	else if (lValue1 > 0.0 && lValue2 > 0.0)
	{
		mOnlyFirst->WriteData( &zero, pEvaluateInfo );
		mOnlySecond->WriteData( &zero, pEvaluateInfo );
		mOnlyThird->WriteData( &zero, pEvaluateInfo );
		mAll->WriteData( &zero, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
			mFirstAndSecond->WriteData( &one, pEvaluateInfo );
		mFirstAndThird->WriteData( &zero, pEvaluateInfo );
		mSecondAndThird->WriteData( &zero, pEvaluateInfo );
	}
	else if (lValue1 > 0.0 && lValue3 > 0.0)
	{
		mOnlyFirst->WriteData( &zero, pEvaluateInfo );
		mOnlySecond->WriteData( &zero, pEvaluateInfo );
		mOnlyThird->WriteData( &zero, pEvaluateInfo );
		mAll->WriteData( &zero, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
		mFirstAndSecond->WriteData( &zero, pEvaluateInfo );
			mFirstAndThird->WriteData( &one, pEvaluateInfo );
		mSecondAndThird->WriteData( &zero, pEvaluateInfo );
	}
	else if (lValue2 > 0.0 && lValue3 > 0.0)
	{
		mOnlyFirst->WriteData( &zero, pEvaluateInfo );
		mOnlySecond->WriteData( &zero, pEvaluateInfo );
		mOnlyThird->WriteData( &zero, pEvaluateInfo );
		mAll->WriteData( &zero, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
		mFirstAndSecond->WriteData( &zero, pEvaluateInfo );
		mFirstAndThird->WriteData( &zero, pEvaluateInfo );
			mSecondAndThird->WriteData( &one, pEvaluateInfo );
	}
	else if (lValue1 > 0.0)
	{
			mOnlyFirst->WriteData( &one, pEvaluateInfo );
		mOnlySecond->WriteData( &zero, pEvaluateInfo );
		mOnlyThird->WriteData( &zero, pEvaluateInfo );
		mAll->WriteData( &zero, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
		mFirstAndSecond->WriteData( &zero, pEvaluateInfo );
		mFirstAndThird->WriteData( &zero, pEvaluateInfo );
		mSecondAndThird->WriteData( &zero, pEvaluateInfo );
	}
	else if (lValue2 > 0.0)
	{
		mOnlyFirst->WriteData( &zero, pEvaluateInfo );
			mOnlySecond->WriteData( &one, pEvaluateInfo );
		mOnlyThird->WriteData( &zero, pEvaluateInfo );
		mAll->WriteData( &zero, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
		mFirstAndSecond->WriteData( &zero, pEvaluateInfo );
		mFirstAndThird->WriteData( &zero, pEvaluateInfo );
		mSecondAndThird->WriteData( &zero, pEvaluateInfo );
	}
	else if (lValue3 > 0.0)
	{
		mOnlyFirst->WriteData( &zero, pEvaluateInfo );
		mOnlySecond->WriteData( &zero, pEvaluateInfo );
			mOnlyThird->WriteData( &one, pEvaluateInfo );
		mAll->WriteData( &zero, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
		mFirstAndSecond->WriteData( &zero, pEvaluateInfo );
		mFirstAndThird->WriteData( &zero, pEvaluateInfo );
		mSecondAndThird->WriteData( &zero, pEvaluateInfo );
	}
	else
	{
		mOnlyFirst->WriteData( &zero, pEvaluateInfo );
		mOnlySecond->WriteData( &zero, pEvaluateInfo );
		mOnlyThird->WriteData( &zero, pEvaluateInfo );
		mAll->WriteData( &zero, pEvaluateInfo );
		mNone->WriteData( &zero, pEvaluateInfo );
		mFirstAndSecond->WriteData( &zero, pEvaluateInfo );
		mFirstAndThird->WriteData( &zero, pEvaluateInfo );
		mSecondAndThird->WriteData( &zero, pEvaluateInfo );
	}

	return true;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool BoxShortcut3::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool BoxShortcut3::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//


/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool BoxShortcutDouble::FBCreate()
{

	/*
	*	Create the nodes for the box.
	*/
	
	mAction = AnimationNodeInCreate	( 0, "Action",		ANIMATIONNODE_TYPE_BOOL );
	mClickTime = AnimationNodeInCreate	( 1, "Click Time",	ANIMATIONNODE_TYPE_NUMBER );

	mSingle = AnimationNodeOutCreate	( 1, "Single",	ANIMATIONNODE_TYPE_BOOL );
	mDouble = AnimationNodeOutCreate	( 2, "Double",	ANIMATIONNODE_TYPE_BOOL );
	
	mNumberOfClicks = 0;
	mClicked = false;
	mLastClickTime = 0.0;
	mDblClicked = false;

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void BoxShortcutDouble::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxShortcutDouble::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue, lClickTime;
	bool lStatus;

	lStatus = mClickTime->ReadData( &lClickTime, pEvaluateInfo );
	if (lStatus == false) lClickTime = 0.2;
	else lClickTime *= 0.1;

	lStatus = mAction->ReadData( &lValue, pEvaluateInfo );
	if (lStatus == false) lValue = 0.0;

	double singleClick = 0.0;
	double doubleClick = 0.0;

	double currTime = pEvaluateInfo->GetSystemTime().GetSecondDouble();

	if (mClicked == false && lValue > 0.0)
	{
		if (mNumberOfClicks > 0 && currTime - mLastClickTime < lClickTime)
		{
			doubleClick = 1.0;
			mNumberOfClicks = 0;
			mDblClicked = true;
		}
		else
		{
			mLastClickTime = pEvaluateInfo->GetSystemTime().GetSecondDouble();
		}

		mClicked = true;
	}
	else if (mClicked == true && lValue == 0.0)
	{
		if (mDblClicked == false)
		{
			if (currTime - mLastClickTime < lClickTime)
			{
				mNumberOfClicks += 1;

				if (mNumberOfClicks >= 2)
				{
					doubleClick = 1.0;
					mNumberOfClicks = 0;
				}
				mLastClickTime = currTime;
			}
			else
			{
				singleClick = 1.0;
				mNumberOfClicks = 0;
			}
		}

		mClicked = false;
		mDblClicked = false;
	}
	else if (mClicked == false && mDblClicked == false && mNumberOfClicks > 0 && currTime - mLastClickTime > lClickTime)
	{
		singleClick = 1.0;
		mNumberOfClicks = 0;
	}

	mSingle->WriteData( &singleClick, pEvaluateInfo );
	mDouble->WriteData( &doubleClick, pEvaluateInfo );

	return true;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool BoxShortcutDouble::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool BoxShortcutDouble::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}