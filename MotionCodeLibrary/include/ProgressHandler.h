//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ProgressHandler.h
//
// FBProgress Helper Class
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class ProgressHandler
{
public:
	ProgressHandler(char *szCaption, char *szInfo, int numberOfTasks=100)
		: mCurrent(0)
		, mCount(numberOfTasks)
	{
		mProgress.Caption = szCaption;
		mProgress.Text = szInfo;
		mProgress.Percent = 0;
	};

	void NextTask( char *szInfo=NULL )
	{
		if (szInfo) mProgress.Text = szInfo;
		mCurrent++;
		Update();
	}

	void Update()
	{
		double value = 100.0 * mCurrent / mCount;
		mProgress.Percent = (int) value;
	}

private:
	int			mCurrent;	// index of current task
	int			mCount;		// number of all tasks

	FBProgress	mProgress;
};
