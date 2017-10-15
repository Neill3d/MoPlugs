
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: camera_boxes.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "camera_boxes.h"
#include "Common.h"

//--- Registration defines
#define BOXCAMERASHAKE__CLASS		BOXCAMERASHAKE__CLASSNAME
#define BOXCAMERASHAKE__NAME		BOXCAMERASHAKE__CLASSSTR
#define	BOXCAMERASHAKE__LOCATION	"Interactive"
#define BOXCAMERASHAKE__LABEL		"Camera Shake"
#define	BOXCAMERASHAKE__DESC		"Camera Shake Box"

#define BOXCAMERAZOOM__CLASS		BOXCAMERAZOOM__CLASSNAME
#define BOXCAMERAZOOM__NAME			BOXCAMERAZOOM__CLASSSTR
#define	BOXCAMERAZOOM__LOCATION		"Interactive"
#define BOXCAMERAZOOM__LABEL		"Camera Zoom"
#define	BOXCAMERAZOOM__DESC			"Camera Zoom Box"

//--- TRANSPORT implementation and registration
FBBoxImplementation	(	BOXCAMERASHAKE__CLASS	);	// Box class name
FBRegisterBox		(	BOXCAMERASHAKE__NAME,		// Unique name to register box.
						BOXCAMERASHAKE__CLASS,		// Box class name
						BOXCAMERASHAKE__LOCATION,	// Box location ('plugins')
						BOXCAMERASHAKE__LABEL,		// Box label (name of box to display)
						BOXCAMERASHAKE__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	BOXCAMERAZOOM__CLASS	);	// Box class name
FBRegisterBox		(	BOXCAMERAZOOM__NAME,		// Unique name to register box.
						BOXCAMERAZOOM__CLASS,		// Box class name
						BOXCAMERAZOOM__LOCATION,	// Box location ('plugins')
						BOXCAMERAZOOM__LABEL,		// Box label (name of box to display)
						BOXCAMERAZOOM__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool BoxCameraShake::FBCreate()
{
	FBPropertyPublish( this, Camera, "Camera", nullptr, nullptr );

	Camera.SetFilter( FBCamera::GetInternalClassId() );
	Camera.SetSingleConnect( true );

	/*
	*	Create the nodes for the box.
	*/
	
	mPullNode = AnimationNodeOutCreate	( 0, "Must Pull",	ANIMATIONNODE_TYPE_NUMBER );
	
	mActionNode = AnimationNodeInCreate	( 1, "Action",		ANIMATIONNODE_TYPE_BOOL );
	mFrequencyNode = AnimationNodeInCreate	( 2, "Frequency",		ANIMATIONNODE_TYPE_NUMBER );
	mAmplitudeNode = AnimationNodeInCreate	( 3, "Amplitude",		ANIMATIONNODE_TYPE_NUMBER );
	mDurationNode = AnimationNodeInCreate	( 4, "Duration",		ANIMATIONNODE_TYPE_NUMBER );
	mRepeatOnActive = AnimationNodeInCreate	( 5, "Repeat",		ANIMATIONNODE_TYPE_BOOL );
	mUseCurrentCameraNode = AnimationNodeInCreate	( 6, "UseCurrentCamera",	ANIMATIONNODE_TYPE_BOOL );

	mShaking = false;

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void BoxCameraShake::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxCameraShake::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue;
	bool lStatus;

	
	bool result = false;

	lStatus = mActionNode->ReadData( &lValue, pEvaluateInfo );
	if( lStatus )
	{
	
		//
		double amplitude, frequency, repeat;

		mAmplitudeNode->ReadData( &amplitude, pEvaluateInfo );
		if (amplitude == 0.0)
			amplitude = 10.0;
		else amplitude *= 0.1;

		mFrequencyNode->ReadData( &frequency, pEvaluateInfo );
		if (frequency == 0.0)
			frequency = 1.0;
		else frequency *= 0.1;

		lStatus = mRepeatOnActive->ReadData( &repeat, pEvaluateInfo );
		if (lStatus == false)
			repeat = 0.0;

		UpdateCameraShake(amplitude, frequency);
		//

		if (lValue == 0.0)
		{
			mShaking = false;
		}
		if( lValue != 0.0 && mShaking == false )
		{
			// DONE: add shake
			
			double duration;

			mDurationNode->ReadData( &duration, pEvaluateInfo );
			if (duration == 0.0)
				duration = 1.0;
			else duration *= 0.1;

			RunCameraShake(amplitude, frequency, duration);

			//
			mShaking = true;
		}
		else if ( lValue != 0.0 && mShaking == true && repeat > 0.0)
		{
			ContinueCameraShake();
		}

		mPullNode->WriteData( &lValue, pEvaluateInfo );

		result = true;
	}

	return result;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool BoxCameraShake::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool BoxCameraShake::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////
//



/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool BoxCameraZoom::FBCreate()
{
	FBPropertyPublish( this, Camera, "Camera", nullptr, nullptr );

	Camera.SetFilter( FBCamera::GetInternalClassId() );
	Camera.SetSingleConnect( true );

	/*
	*	Create the nodes for the box.
	*/
	
	mPullNode = AnimationNodeOutCreate	( 0, "Must Pull",	ANIMATIONNODE_TYPE_NUMBER );
	
	mActionNode = AnimationNodeInCreate	( 1, "Action",		ANIMATIONNODE_TYPE_BOOL );
	mAmplitudeNode = AnimationNodeInCreate	( 2, "Amplitude",		ANIMATIONNODE_TYPE_NUMBER );
	mDurationNode = AnimationNodeInCreate	( 3, "Duration",		ANIMATIONNODE_TYPE_NUMBER );
	mRepeatOnActive = AnimationNodeInCreate	( 4, "Repeat",		ANIMATIONNODE_TYPE_BOOL );
	mUseCurrentCameraNode = AnimationNodeInCreate	( 5, "UseCurrentCamera",	ANIMATIONNODE_TYPE_BOOL );

	mZooming = false;

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void BoxCameraZoom::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxCameraZoom::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue;
	bool lStatus;

	
	bool result = false;

	lStatus = mActionNode->ReadData( &lValue, pEvaluateInfo );
	if( lStatus )
	{
	
		//
		double amplitude, repeat;

		mAmplitudeNode->ReadData( &amplitude, pEvaluateInfo );
		if (amplitude == 0.0)
			amplitude = 40.0;

		lStatus = mRepeatOnActive->ReadData( &repeat, pEvaluateInfo );
		if (lStatus == false)
			repeat = 0.0;

		UpdateCameraZoom( amplitude );

		//
		if (lValue == 0.0)
		{
			mZooming = false;
			StopCameraZoom();
		}
		if( lValue != 0.0 && mZooming == false )
		{
			// DONE: add shake
			
			double duration;

			mDurationNode->ReadData( &duration, pEvaluateInfo );
			if (duration == 0.0)
				duration = 1.0;
			else duration *= 0.1;

			RunCameraZoom(amplitude, duration, (repeat>0.0) );

			//
			mZooming = true;
		}
		
		mPullNode->WriteData( &lValue, pEvaluateInfo );

		result = true;
	}

	return result;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool BoxCameraZoom::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool BoxCameraZoom::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}