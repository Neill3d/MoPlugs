
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Common_FBComponent_UpdateProps.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Common_FBComponent_UpdateProps.h"

#define FBPropertyPublishUpdateProps( Parent,Prop,PropName,Get,Set ) \
    { \
    Parent->PropertyAdd(Prop.CreateInternal( Parent,PropName,(fbExternalGetSetHandler)Get,(fbExternalGetSetHandler)Set )); \
    }

CFBComponentUpdateProps::CFBComponentUpdateProps()
	: mUpdateComponent(nullptr)
{
	ResetUpdateValues();
}

void CFBComponentUpdateProps::FBCreateUpdateProps(FBComponent *pParent, fbExternalGetSetHandler UpdateActionSet)
{
	mUpdateComponent = pParent;

	FBPropertyPublishUpdateProps( mUpdateComponent, RealTimeUpdate, "RealTime Update", nullptr, nullptr );
	FBPropertyPublishUpdateProps( mUpdateComponent, UpdateOnTimeSliderChange, "Update On TimeSlider Change", nullptr, nullptr );
	FBPropertyPublishUpdateProps( mUpdateComponent, UpdateWhenSelected, "Update When Selected", nullptr, nullptr );
	FBPropertyPublishUpdateProps( mUpdateComponent, UpdateSkipFrames, "Update Skip Frames", nullptr, nullptr ); // ModelCameraRendering_Update );

	FBPropertyPublishUpdateProps( mUpdateComponent, UpdateAction, "Update", nullptr, UpdateActionSet ); // ModelCameraRendering_Update );

	RealTimeUpdate = false;
	UpdateOnTimeSliderChange = true;
	UpdateWhenSelected = true;
	UpdateSkipFrames = 7;
}

void CFBComponentUpdateProps::ResetUpdateValues()
{
	mUpdateFlag = true;
	mLastLocalTime = FBTime::Infinity;
	mLastFrameId = 0;
}

bool CFBComponentUpdateProps::ProcessUpdateConditions( const bool offlineRender, 
								FBTime &localTime, 
								const unsigned int frameId )
{
	if (true == offlineRender || true == mUpdateFlag)
	{
		mUpdateFlag = true;
		return mUpdateFlag;
	}

	bool needUpdate = true;
	int skipFrames = UpdateSkipFrames;

	if (false == RealTimeUpdate)
	{
		needUpdate = mUpdateFlag;

		if (true == UpdateWhenSelected && true == mUpdateComponent->Selected)
			needUpdate = true;
		else
		if (true == UpdateOnTimeSliderChange)
		{
			if (skipFrames > 0)
			{
				if ( abs(mLastLocalTime.GetFrame()-localTime.GetFrame()) >= skipFrames )
				{
					needUpdate = true;
					mLastLocalTime = localTime;
				}
			}
			else
			if (mLastLocalTime != localTime)
			{
				needUpdate = true;
				mLastLocalTime = localTime;
			}
		}
		mLastFrameId = frameId;
	}
	else
	{
				
		if (skipFrames > 0)
		{
			int diff = (int)frameId - mLastFrameId;
			if ( abs(diff) < skipFrames )
				needUpdate = false;
			else
				mLastFrameId = frameId;
		}
		else
		{
			mLastFrameId = frameId;
		}
	}

	mUpdateFlag = needUpdate;
	return mUpdateFlag;
}


void CFBComponentUpdateProps::OnSetUpdateAction()
{
	mUpdateFlag = true;
}
