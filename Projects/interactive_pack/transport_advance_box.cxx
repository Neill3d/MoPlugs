
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: transport_advance_box.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "transport_advance_box.h"
#include "Common.h"

//--- Registration defines
#define ORBOXTRANSPORT__CLASS			ORBOXTRANSPORT__CLASSNAME
#define ORBOXTRANSPORT__NAME			ORBOXTRANSPORT__CLASSSTR
#define	ORBOXTRANSPORT__LOCATION		"Interactive"
#define ORBOXTRANSPORT__LABEL			"Advanced Transport"
#define	ORBOXTRANSPORT__DESC			"Advanced Transport Control Box"

#define ORBOXTRANSPORTCOMMANDS__CLASS		ORBOXTRANSPORTCOMMANDS__CLASSNAME
#define ORBOXTRANSPORTCOMMANDS__NAME		ORBOXTRANSPORTCOMMANDS__CLASSSTR
#define	ORBOXTRANSPORTCOMMANDS__LOCATION	"Interactive"
#define ORBOXTRANSPORTCOMMANDS__LABEL		"Command Transport"
#define	ORBOXTRANSPORTCOMMANDS__DESC		"Command Transport Control Box"

#define ORBOXCOLORWHITE__CLASS		ORBOXCOLORWHITE__CLASSNAME
#define ORBOXCOLORWHITE__NAME		ORBOXCOLORWHITE__CLASSSTR
#define	ORBOXCOLORWHITE__LOCATION	"Interactive"
#define ORBOXCOLORWHITE__LABEL		"Color White"
#define	ORBOXCOLORWHITE__DESC		"Color White Box"

#define ORBOXCOLORBLACK__CLASS		ORBOXCOLORBLACK__CLASSNAME
#define ORBOXCOLORBLACK__NAME		ORBOXCOLORBLACK__CLASSSTR
#define	ORBOXCOLORBLACK__LOCATION	"Interactive"
#define ORBOXCOLORBLACK__LABEL		"Color Black"
#define	ORBOXCOLORBLACK__DESC		"Color Black Box"

#define ORBOXCOLORRED__CLASS		ORBOXCOLORRED__CLASSNAME
#define ORBOXCOLORRED__NAME			ORBOXCOLORRED__CLASSSTR
#define	ORBOXCOLORRED__LOCATION		"Interactive"
#define ORBOXCOLORRED__LABEL		"Color Red"
#define	ORBOXCOLORRED__DESC			"Color Red Box"

#define ORBOXCOLORGREEN__CLASS		ORBOXCOLORGREEN__CLASSNAME
#define ORBOXCOLORGREEN__NAME		ORBOXCOLORGREEN__CLASSSTR
#define	ORBOXCOLORGREEN__LOCATION	"Interactive"
#define ORBOXCOLORGREEN__LABEL		"Color Green"
#define	ORBOXCOLORGREEN__DESC		"Color Green Box"

#define ORBOXCOLORBLUE__CLASS		ORBOXCOLORBLUE__CLASSNAME
#define ORBOXCOLORBLUE__NAME		ORBOXCOLORBLUE__CLASSSTR
#define	ORBOXCOLORBLUE__LOCATION	"Interactive"
#define ORBOXCOLORBLUE__LABEL		"Color Blue"
#define	ORBOXCOLORBLUE__DESC		"Color Blue Box"

//--- TRANSPORT implementation and registration
FBBoxImplementation	(	ORBOXTRANSPORT__CLASS	);	// Box class name
FBRegisterBox		(	ORBOXTRANSPORT__NAME,		// Unique name to register box.
						ORBOXTRANSPORT__CLASS,		// Box class name
						ORBOXTRANSPORT__LOCATION,	// Box location ('plugins')
						ORBOXTRANSPORT__LABEL,		// Box label (name of box to display)
						ORBOXTRANSPORT__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	ORBOXTRANSPORTCOMMANDS__CLASS	);	// Box class name
FBRegisterBox		(	ORBOXTRANSPORTCOMMANDS__NAME,		// Unique name to register box.
						ORBOXTRANSPORTCOMMANDS__CLASS,		// Box class name
						ORBOXTRANSPORTCOMMANDS__LOCATION,	// Box location ('plugins')
						ORBOXTRANSPORTCOMMANDS__LABEL,		// Box label (name of box to display)
						ORBOXTRANSPORTCOMMANDS__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	ORBOXCOLORWHITE__CLASS	);	// Box class name
FBRegisterBox		(	ORBOXCOLORWHITE__NAME,		// Unique name to register box.
						ORBOXCOLORWHITE__CLASS,		// Box class name
						ORBOXCOLORWHITE__LOCATION,	// Box location ('plugins')
						ORBOXCOLORWHITE__LABEL,		// Box label (name of box to display)
						ORBOXCOLORWHITE__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	ORBOXCOLORBLACK__CLASS	);	// Box class name
FBRegisterBox		(	ORBOXCOLORBLACK__NAME,		// Unique name to register box.
						ORBOXCOLORBLACK__CLASS,		// Box class name
						ORBOXCOLORBLACK__LOCATION,	// Box location ('plugins')
						ORBOXCOLORBLACK__LABEL,		// Box label (name of box to display)
						ORBOXCOLORBLACK__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	ORBOXCOLORRED__CLASS	);	// Box class name
FBRegisterBox		(	ORBOXCOLORRED__NAME,		// Unique name to register box.
						ORBOXCOLORRED__CLASS,		// Box class name
						ORBOXCOLORRED__LOCATION,	// Box location ('plugins')
						ORBOXCOLORRED__LABEL,		// Box label (name of box to display)
						ORBOXCOLORRED__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	ORBOXCOLORGREEN__CLASS	);	// Box class name
FBRegisterBox		(	ORBOXCOLORGREEN__NAME,		// Unique name to register box.
						ORBOXCOLORGREEN__CLASS,		// Box class name
						ORBOXCOLORGREEN__LOCATION,	// Box location ('plugins')
						ORBOXCOLORGREEN__LABEL,		// Box label (name of box to display)
						ORBOXCOLORGREEN__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	ORBOXCOLORBLUE__CLASS	);	// Box class name
FBRegisterBox		(	ORBOXCOLORBLUE__NAME,		// Unique name to register box.
						ORBOXCOLORBLUE__CLASS,		// Box class name
						ORBOXCOLORBLUE__LOCATION,	// Box location ('plugins')
						ORBOXCOLORBLUE__LABEL,		// Box label (name of box to display)
						ORBOXCOLORBLUE__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool ORBoxTransportAdvance::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	mPlay = AnimationNodeInCreate	( 0, "Play",		ANIMATIONNODE_TYPE_BOOL );
	mPlayPosition = AnimationNodeInCreate	( 1, "Play Frame",		ANIMATIONNODE_TYPE_NUMBER );
	mGoToNextMarker = AnimationNodeInCreate	( 2, "GoTo Next Marker",		ANIMATIONNODE_TYPE_INTEGER );
	mMarkerCount = AnimationNodeInCreate	( 3, "Marker Count",		ANIMATIONNODE_TYPE_INTEGER );
	mGoToNextCommand = AnimationNodeInCreate	( 4, "GoTo Next Cmd",		ANIMATIONNODE_TYPE_INTEGER );
	mNextStep = AnimationNodeInCreate	( 5, "Next Step",		ANIMATIONNODE_TYPE_INTEGER );
	mNextRandom = AnimationNodeInCreate	( 6, "Next Random",		ANIMATIONNODE_TYPE_BOOL );

	// out
	mPull = AnimationNodeOutCreate	( 7, "Must Pull",	ANIMATIONNODE_TYPE_NUMBER );

	mMarkerActivated = false;
	mCommandActivated = false;

	FBPropertyPublish( this, Update, "Update", nullptr, nullptr );

	mSection.Init();

	mSystem.OnUIIdle.Add( this, (FBCallback) &ORBoxTransportAdvance::OnSystemIdle );

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void ORBoxTransportAdvance::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
	mSystem.OnUIIdle.Remove( this, (FBCallback) &ORBoxTransportAdvance::OnSystemIdle );
}

//////////////////////////////////////////////////
//

void ORBoxTransportAdvance::OnSystemIdle(HISender pSender, HKEvent pEvent)
{
	Data	data;

	mSection.Enter();
	data = mData;
	mSection.Leave();

	//
	
	if (data.gotomarker == true && data.markercount > 0)
	{
		// TODO: next marker

	}
	else if (data.gotocommand == true)
	{
		// TODO: next command

	}
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool ORBoxTransportAdvance::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue;
	bool lStatus;

	double playPos=0.0;

	bool result = false;

	lStatus = mPlayPosition->ReadData( &playPos, pEvaluateInfo );
	lStatus = mPlay->ReadData( &lValue, pEvaluateInfo );
	if( lStatus )
	{
		mPull->WriteData( &lValue, pEvaluateInfo );
		if( lValue != 0.0 )
		{
			if( mPlayerControl.GetTransportMode() != kFBTransportPlay )
			{
				
				FBTime playTime( 0,0,0, (int) playPos );
				mPlayerControl.Goto( playTime );
				mPlayerControl.Play(true); // don't forget to stop on markers
				
			}
		}
		result = true;
	}

	//
	//

	Data		data;

	bool lStatusMarker, lStatusCommand;
	double lValueMarker, lValueCommand;

	lStatusMarker = mGoToNextMarker->ReadData( &lValueMarker, pEvaluateInfo );
	lStatusCommand = mGoToNextCommand->ReadData( &lValueCommand, pEvaluateInfo );

	if (lStatusMarker || lStatusCommand)
	{
		//
		lStatus = mMarkerCount->ReadData( &lValue, pEvaluateInfo );
		if (lStatus == false) lValue = 0.0;
		data.markercount = (int) lValue;

		lStatus = mNextStep->ReadData( &lValue, pEvaluateInfo );
		if (lStatus == false) lValue = 1.0;
		data.nextstep = (int) lValue;

		lStatus = mNextRandom->ReadData( &lValue, pEvaluateInfo );
		if (lStatus == false) lValue = 0.0;
		data.nextrandom = (lValue > 0.0);

		//
		if (lStatusMarker)
		{
			if (lValueMarker == 0.0)
			{
				mMarkerActivated = false;
			}
			else if (mMarkerActivated == false)
			{
				data.gotomarker = true;
				mMarkerActivated = true;
			}
		}

		//
		if (lStatusCommand)
		{
			if (lValueCommand == 0.0)
			{
				mCommandActivated = false;
			}
			else if (mCommandActivated == false)
			{
				data.gotocommand = true;
				mCommandActivated = true;
			}
		}

		lValue = (data.gotocommand || data.gotomarker) ? 1.0 : 0.0;
		mPull->WriteData( &lValue, pEvaluateInfo );

		result = true;
	}
	 
	// sync data with care

	mSection.Enter();
	mData = data;
	mSection.Leave();


	return result;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool ORBoxTransportAdvance::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORBoxTransportAdvance::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	COLOR Creation
 ************************************************/
bool ORBoxColorWhite::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	mColor = AnimationNodeOutCreate	( 0, "Color",	ANIMATIONNODE_TYPE_COLOR );
	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void ORBoxColorWhite::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool ORBoxColorWhite::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue[3] = {1.0, 1.0, 1.0};
	mColor->WriteData( lValue, pEvaluateInfo );
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	COLOR Creation
 ************************************************/
bool ORBoxColorBlack::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	mColor = AnimationNodeOutCreate	( 0, "Color",	ANIMATIONNODE_TYPE_COLOR );
	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void ORBoxColorBlack::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool ORBoxColorBlack::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue[3] = {.0, .0, .0};
	mColor->WriteData( lValue, pEvaluateInfo );
	
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	COLOR Creation
 ************************************************/
bool ORBoxColorRed::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	mColor = AnimationNodeOutCreate	( 0, "Color",	ANIMATIONNODE_TYPE_COLOR );
	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void ORBoxColorRed::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool ORBoxColorRed::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue[3] = {1.0, .0, .0};
	mColor->WriteData( lValue, pEvaluateInfo );
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	COLOR Creation
 ************************************************/
bool ORBoxColorGreen::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	mColor = AnimationNodeOutCreate	( 0, "Color",	ANIMATIONNODE_TYPE_COLOR );
	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void ORBoxColorGreen::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool ORBoxColorGreen::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue[3] = {.0, 1.0, .0};
	mColor->WriteData( lValue, pEvaluateInfo );
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	COLOR Creation
 ************************************************/
bool ORBoxColorBlue::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	mColor = AnimationNodeOutCreate	( 0, "Color",	ANIMATIONNODE_TYPE_COLOR );
	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void ORBoxColorBlue::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool ORBoxColorBlue::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue[3] = {.0, .0, 1.0};
	mColor->WriteData( lValue, pEvaluateInfo );
	
	return true;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	COMMANDS Creation
 ************************************************/
bool ORBoxTransportCommands::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	mAction = AnimationNodeInCreate	( 0, "Action",		ANIMATIONNODE_TYPE_BOOL );
	mColor = AnimationNodeInCreate	( 1, "Color",		ANIMATIONNODE_TYPE_COLOR );
	mStep = AnimationNodeInCreate	( 2, "Step",		ANIMATIONNODE_TYPE_INTEGER );
	mRandom = AnimationNodeInCreate	( 3, "Random",		ANIMATIONNODE_TYPE_BOOL );
	
	// out
	mPull = AnimationNodeOutCreate	( 4, "Must Pull",	ANIMATIONNODE_TYPE_NUMBER );

	mCommandActivated = false;

	mSection.Init();
	mSystem.OnUIIdle.Add( this, (FBCallback) &ORBoxTransportCommands::OnSystemIdle );

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void ORBoxTransportCommands::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
	mSystem.OnUIIdle.Remove( this, (FBCallback) &ORBoxTransportCommands::OnSystemIdle );
}

//////////////////////////////////////////////////
//

void ORBoxTransportCommands::OnSystemIdle(HISender pSender, HKEvent pEvent)
{
	Data	data;

	mSection.Enter();
	data = mData;
	mData.gotocommand = false;
	mSection.Leave();

	//
	
	if (data.gotocommand == true)
	{
		// DONE: next command
		GotoNextCommand(data.color, data.nextstep, data.nextrandom);
	}
}

void ORBoxTransportCommands::GotoNextCommand(const double key[3], const int step, const bool dorandom)
{
	FBTime localtime = mSystem.LocalTime;
	FBTime nextTime = FindNextCommandTime(localtime, key, step, dorandom);

	bool isPlaying = FBPlayerControl::TheOne().IsPlaying;

	if (nextTime != FBTime::Infinity)
	{
		FBPlayerControl::TheOne().Goto(nextTime);
	}

	if (isPlaying)
		FBPlayerControl::TheOne().Play();
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool ORBoxTransportCommands::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue;
	double lColor[3]={1.0, 1.0, 1.0};
	bool lStatus;

	double playPos=0.0;

	bool result = false;

	Data		data;

	lStatus = mAction->ReadData( &lValue, pEvaluateInfo );

	if (lStatus)
	{

		//
		if (lValue == 0.0)
		{
			mCommandActivated = false;
		}
		else if (mCommandActivated == false)
		{
			data.gotocommand = true;
			mCommandActivated = true;
		}

		//
		lStatus = mColor->ReadData( lColor, pEvaluateInfo );
		if (lStatus == false) lColor[0]=lColor[1]=lColor[2] = 1.0;
		memcpy(data.color, lColor, sizeof(double) * 3 );

		lStatus = mStep->ReadData( &lValue, pEvaluateInfo );
		if (lStatus == false) lValue = 0.0;
		data.nextstep = (int) lValue;

		lStatus = mRandom->ReadData( &lValue, pEvaluateInfo );
		if (lStatus == false) lValue = 0.0;
		data.nextrandom = (lValue > 0.0);

		// sync data with care

		if (data.gotocommand == true)
		{
			mSection.Enter();
			mData = data;
			mSection.Leave();

			lValue = 1.0;
			mPull->WriteData( &lValue, pEvaluateInfo );
		}

		result = true;
	}

	return result;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool ORBoxTransportCommands::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORBoxTransportCommands::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}
