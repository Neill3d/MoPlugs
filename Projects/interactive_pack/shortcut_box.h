#ifndef __SHORTCUT_BOX_H__
#define __SHORTCUT_BOX_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shortcut_box.h
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
#define	BOXSHORTCUT__CLASSNAME		BoxShortcut2
#define BOXSHORTCUT__CLASSSTR		"BoxShortcut2"

#define	BOXSHORTCUT3__CLASSNAME		BoxShortcut3
#define BOXSHORTCUT3__CLASSSTR		"BoxShortcut3"

#define	BOXSHORTCUTDOUBLE__CLASSNAME	BoxShortcutDouble
#define BOXSHORTCUTDOUBLE__CLASSSTR		"BoxShortcutDouble"

/**	Template for FBBox class.
*/
class BoxShortcut2 : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( BoxShortcut2, FBBox );

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


private:

	FBAnimationNode		*mOnlyFirst;			//!< out
	FBAnimationNode		*mOnlySecond;			//!< out
	FBAnimationNode		*mBoth;			//!< out
	FBAnimationNode		*mNone;			//!< out

	FBAnimationNode		*mAction1;		//!< in - run shake effect
	FBAnimationNode		*mAction2;
	
};

/////////////////////////////////////////////////////////////////////////////////////////////

/**	Template for FBBox class.
*/
class BoxShortcut3 : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( BoxShortcut3, FBBox );

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


private:

	FBAnimationNode		*mOnlyFirst;			//!< out
	FBAnimationNode		*mOnlySecond;			//!< out
	FBAnimationNode		*mOnlyThird;			//!< out
	FBAnimationNode		*mAll;					//!< out
	FBAnimationNode		*mNone;					//!< out
	FBAnimationNode		*mFirstAndSecond;		//!< out
	FBAnimationNode		*mFirstAndThird;		//!< out
	FBAnimationNode		*mSecondAndThird;		//!< out

	FBAnimationNode		*mAction1;		//!< in - run shake effect
	FBAnimationNode		*mAction2;
	FBAnimationNode		*mAction3;

};

//////////////////////////////////////////////////////////////////////////////
//


/**	Template for FBBox class.
*/
class BoxShortcutDouble : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( BoxShortcutDouble, FBBox );

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


private:

	FBAnimationNode		*mAction;		//!< in - 
	FBAnimationNode		*mClickTime;		//!< in - time between clicks to identify them as double

	FBAnimationNode		*mSingle;			//!< out
	FBAnimationNode		*mDouble;			//!< out

	int					mNumberOfClicks;
	bool				mClicked;
	double				mLastClickTime;

	bool				mDblClicked;
};

#endif /* __SHORTCUT_BOX_H__ */
