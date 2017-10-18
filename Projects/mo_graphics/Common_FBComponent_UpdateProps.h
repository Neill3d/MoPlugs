
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Common_FBComponent_UpdateProps.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

class CFBComponentUpdateProps
{
public:

	// ! a constructor
	CFBComponentUpdateProps();

	virtual ~CFBComponentUpdateProps()
	{}

public:

	FBPropertyBool					RealTimeUpdate;
	FBPropertyBool					UpdateOnTimeSliderChange;
	FBPropertyBool					UpdateWhenSelected;
	FBPropertyInt					UpdateSkipFrames;

	FBPropertyAction				UpdateAction;

	virtual void ResetUpdateValues();
	void FBCreateUpdateProps(FBComponent *pParent, fbExternalGetSetHandler UpdateActionSet);

	bool ProcessUpdateConditions( const bool offlineRender, 
								FBTime &localTime, 
								const unsigned int frameId );

	bool HasUpdateFlag() const
	{
		return mUpdateFlag;
	}
	void SetUpdateFlag(bool flag) {
		mUpdateFlag = flag;
	}
	void ResetUpdateFlag() {
		mUpdateFlag = false;
	}

protected:

	FBComponent			*mUpdateComponent;
	bool				mUpdateFlag;

	FBTime				mLastLocalTime;
	unsigned int		mLastFrameId;

	void OnSetUpdateAction();
};