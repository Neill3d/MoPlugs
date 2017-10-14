
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Prediction.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "algorithm\Prediction.h"

void AnimationNodeToPrediction( HFBAnimationNode translate, HFBAnimationNode rotate, Prediction6DOF &prediction )
{
	prediction.Clear();

	if (translate->IsKey() || rotate->IsKey() )
	{
		FBVector3d v,r;
		translate->ReadData( v );
		rotate->ReadData( r );

		// use existing value as a predicted one
		double tm[6] = {v[0], v[1], v[2], r[0], r[1], r[2]};
		prediction.PushValue( tm );
	}
	else
	{
		/*
		FBVector3d v,r;
		kPredictDirectionEnum	dir = prediction.GetDir();
		FBTime lTime = FBSystem().LocalTime;
		FBTime	oneframe(0,0,0,1);
		FBTime	fiveframes(0,0,0,5);

		// add nearest 10 frames
		switch(dir)
		{
		case kPredictItemBoth:
			lTime -= fiveframes;
			for (int i=0; i<10; i++)
			{
				lTime += oneframe;
				translate->Evaluate( v, lTime );
				rotate->Evaluate( r, lTime );
				
				double tm[6] = {v[0], v[1], v[2], r[0], r[1], r[2]};
				prediction.PushValue( tm );
			}
			break;
		case kPredictItemForward:
			// add last 10 frames
			for (int i=0; i<10; i++)
			{
				lTime -= oneframe;
				translate->Evaluate( v, lTime );
				rotate->Evaluate( r, lTime );
				
				double tm[6] = {v[0], v[1], v[2], r[0], r[1], r[2]};
				prediction.PushValue( tm );
			}
			break;
		case kPredictItemBackward:
			for (int i=0; i<10; i++)
			{
				lTime += oneframe;
				translate->Evaluate( v, lTime );
				rotate->Evaluate( r, lTime );
				
				double tm[6] = {v[0], v[1], v[2], r[0], r[1], r[2]};
				prediction.PushValue( tm );
			}
			break;
		}
		*/
	}
}