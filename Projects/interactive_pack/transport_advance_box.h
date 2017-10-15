#ifndef __ORBOXTRANSPORT_BOX_H__
#define __ORBOXTRANSPORT_BOX_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: transport_advance_box.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define	ORBOXTRANSPORT__CLASSNAME		ORBoxTransportAdvance
#define ORBOXTRANSPORT__CLASSSTR		"ORBoxTransportAdvance"

#define	ORBOXTRANSPORTCOMMANDS__CLASSNAME		ORBoxTransportCommands
#define ORBOXTRANSPORTCOMMANDS__CLASSSTR		"ORBoxTransportCommands"

#define	ORBOXCOLORWHITE__CLASSNAME		ORBoxColorWhite
#define ORBOXCOLORWHITE__CLASSSTR		"ORBoxColorWhite"

#define	ORBOXCOLORBLACK__CLASSNAME		ORBoxColorBlack
#define ORBOXCOLORBLACK__CLASSSTR		"ORBoxColorBlack"

#define	ORBOXCOLORRED__CLASSNAME		ORBoxColorRed
#define ORBOXCOLORRED__CLASSSTR			"ORBoxColorRed"

#define	ORBOXCOLORGREEN__CLASSNAME		ORBoxColorGreen
#define ORBOXCOLORGREEN__CLASSSTR		"ORBoxColorGreen"

#define	ORBOXCOLORBLUE__CLASSNAME		ORBoxColorBlue
#define ORBOXCOLORBLUE__CLASSSTR		"ORBoxColorBlue"

/**	Template for FBBox class.
*/
class ORBoxTransportAdvance : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( ORBoxTransportAdvance, FBBox );

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

	FBPropertyAction		Update;

private:

	FBSystem			mSystem;

	FBPlayerControl		mPlayerControl;

	FBAnimationNode		*mPull;

	FBAnimationNode		*mRecord;
	FBAnimationNode		*mPlay;
	FBAnimationNode		*mPlayPosition;
	FBAnimationNode		*mStop;

	FBAnimationNode		*mGoToNextMarker;
	FBAnimationNode		*mMarkerCount;		// manualy set a number of marker in the timeline
	FBAnimationNode		*mGoToNextCommand;	// input - set player time as a next command in story track	
	FBAnimationNode		*mNextStep;			// how many markers we would like to skip
	FBAnimationNode		*mNextRandom;			// do we need to randomize a number of steps

	bool				mMarkerActivated;
	bool				mCommandActivated;

	struct Data 
	{
		bool	gotomarker;
		int		markercount;
		bool	gotocommand;
		int		nextstep;
		bool	nextrandom;

		Data()
		{
			gotomarker = false;
			markercount = 0;
			gotocommand = false;
			nextstep = 1;
			nextrandom = false;
		}
	};

	FBCriticalSection	mSection;
	Data				mData;

	void	GotoNextMarker(const int markercount, const int step, const bool dorandom); 
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//

/**	Template for FBBox class.
*/
class ORBoxColorWhite : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( ORBoxColorWhite, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

private:

	FBAnimationNode		*mColor;		//!> out
};

/**	Template for FBBox class.
*/
class ORBoxColorBlack : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( ORBoxColorBlack, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

private:

	FBAnimationNode		*mColor;		//!> out
};

/**	Template for FBBox class.
*/
class ORBoxColorRed : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( ORBoxColorRed, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

private:

	FBAnimationNode		*mColor;		//!> out
};

/**	Template for FBBox class.
*/
class ORBoxColorGreen : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( ORBoxColorGreen, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

private:

	FBAnimationNode		*mColor;		//!> out
};

/**	Template for FBBox class.
*/
class ORBoxColorBlue : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( ORBoxColorBlue, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

private:

	FBAnimationNode		*mColor;		//!> out
};

/**	Template for FBBox class.
*/
class ORBoxTransportCommands : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( ORBoxTransportCommands, FBBox );

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

private:

	FBSystem			mSystem;

	FBPlayerControl		mPlayerControl;

	FBAnimationNode		*mPull;

	FBAnimationNode		*mAction;	// input - set player time as a next command in story track	
	FBAnimationNode		*mColor;		// COLOR KEY for searching next marker
	FBAnimationNode		*mStep;			// how many markers we would like to skip
	FBAnimationNode		*mRandom;			// do we need to randomize a number of steps

	bool				mCommandActivated;

	struct Data 
	{
		bool	gotocommand;
		double  color[3];
		int		nextstep;
		bool	nextrandom;

		Data()
		{
			gotocommand = false;
			color[0] = color[1] = color[2] = 1.0;	// white KEY by default
			nextstep = 1;
			nextrandom = false;
		}
	};

	FBCriticalSection	mSection;
	Data				mData;

	void	GotoNextCommand(const double key[3], const int step, const bool dorandom); 
};


#endif /* __ORBOXTRANSPORT_BOX_H__ */
