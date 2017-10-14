
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: OpSkeleton.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "algorithm\math3d_mobu.h"

#ifdef	USE_MATLAB
#include "engine.h"

class ComputePivot
{
public:
	ComputePivot(Engine	*_ep,	HFBModel	_parentRB, HFBModel	_candidateRB)
		: ep(_ep)
		, parentRB(_parentRB)
		, candidateRB(_candidateRB)
	{
		if (parentRB && candidateRB)
		{
			mxArray *data = NULL;

			CalculateLocalMovement();
			CalculatePivot();
		}
	}

private:
	//!	nodes
	HFBModel	parentRB;
	HFBModel	candidateRB;
	//! matlab data
	Engine	*ep;
	int				mCount;
	mxArray		*data;
	//! matrix for local2world conversion
	FBMatrix	parentTM;

	void CreateLocalObject()
	{
		static FBSystem	sSystem;
		HFBModel	LclObj	= FBFindModelByName("Null");

		if (!LclObj)
		{
			// create local object, snap it to candidate and parent to parentRB
			LclObj = new FBModelNull("Null");

			FBMatrix	m;
			candidateRB->GetMatrix(m);
			LclObj->SetMatrix(m);

			LclObj->Parent = parentRB;

			// assign parent/child constraint
			FBConstraintManager		lConstraintManager;
			HFBConstraint		lConstraint = NULL;
			int i, c = lConstraintManager.TypeGetCount();
			for( i = 0; i < c; i++ )
			{
				if( strstr(lConstraintManager.TypeGetName(i), "Parent/Child") )
				{
					lConstraint = lConstraintManager.TypeCreateConstraint(i);
					break;
				}
			}
			
			if (lConstraint){
				lConstraint->ReferenceAdd( 0, LclObj );
				lConstraint->ReferenceAdd( 1, candidateRB );

				// Activate constraint
				lConstraint->Active = true;
				//lConstraint->Snap = true;
			}
			
			LclObj->Selected = true;
			FBTime	lPeriod(0,0,0,1);
			sSystem.CurrentTake->PlotTakeOnSelected(lPeriod);
			lConstraint->Active = false;
		}
	}

	void CalculateLocalMovement(bool test=true)
	{
		static FBSystem	sSystem;

		HFBModel	LclObj	= FBFindModelByName("Null");

		int mStartTime = 0;
		int mStopTime = 500;

		// progress
		FBProgress	lProgress;
		lProgress.Caption = "Actor pivot";
		lProgress.Text = "calculate movement";
		lProgress.Percent = 0;

		// set keys for each frame
		FBPlayerControl		lPlayer;

		FBTime lTime, lStep;
		
		lTime = lPlayer.ZoomWindowStop;
		mStopTime = lTime.GetFrame(true);
		lTime = lPlayer.ZoomWindowStart;
		mStartTime = lTime.GetFrame(true);
		//lTime.SetTime( 0,0,0, mStartTime );
		//lPlayer.Goto(lTime);
		lStep.SetTime( 0,0,0, 1 );

		mCount = (int) (mStopTime - mStartTime);
	
		FBVector3d		lPos;
		FBMatrix	m, TM, pTM;	// candidate TM, parent TM

		//
		//	allocate output array
		//
		data = mxCreateDoubleMatrix(1, mCount*3, mxREAL);
		double *currData = mxGetPr(data);

		//
		// create output marker (test mode)
		//
		HFBAnimationNode	lAnimNode;
		HFBAnimationNode	lTransNode=NULL, lNode;

		if (test)
		{
			HFBModelNull			pNull;
			pNull	= new FBModelNull("rbOutput");
			pNull->Size = 30.0;
			pNull->Show = true;

			//
			//	make all animation nodes for position
			//
			lAnimNode = pNull->AnimationNode;
			lTransNode = lAnimNode->Nodes.Find( "Lcl Translation");
			lNode	= lTransNode->Nodes.Find("X");
			if ( !lNode->FCurve )
				lNode->FCurve = new FBFCurve;
		
			lNode	= lTransNode->Nodes.Find("Y");
			if ( !lNode->FCurve )
				lNode->FCurve = new FBFCurve;

			lNode	= lTransNode->Nodes.Find("Z");
			if ( !lNode->FCurve )
				lNode->FCurve = new FBFCurve;
		}

		//
		// putting animation key's
		//
		parentRB->GetMatrix(parentTM);

		int n=0;
		int delta = mCount / 100;
		while (n<mCount) {

			//sSystem.LocalTime = lTime;
			//sSystem.Scene->Evaluate();

			lProgress.Percent = n / delta;

			TM = FBMatrixFromAnimationNode(candidateRB, lTime);
			pTM = FBMatrixFromAnimationNode(parentRB, lTime);
			//candidateRB->GetMatrix(TM);
			//parentRB->GetMatrix(pTM);

			if (pTM.Inverse() )
			{
				m.Identity();
				m = MatrixMult(pTM, TM);

				//-- keys null model translation
				if (LclObj)
				{
					HFBAnimationNode	lTranslationNode = LclObj->Translation.GetAnimationNode();
					lPos[0] = lTranslationNode->Nodes[0]->FCurve->Evaluate(lTime);
					lPos[1] = lTranslationNode->Nodes[1]->FCurve->Evaluate(lTime);
					lPos[2] = lTranslationNode->Nodes[2]->FCurve->Evaluate(lTime);
				}
				else
					lPos = FBVector3d( m[12], m[13], m[14] );

				*currData = lPos[0];
				currData++;
				*currData = lPos[1];
				currData++;
				*currData = lPos[2];
				currData++;

				if (test && lTransNode)
					lTransNode->KeyAdd( lTime, &lPos[0] );
			}

			n++;
			lTime = lTime + lStep;
		}
	}


	void CalculatePivot()
	{
		double r;
		FBVector3d	c, res;

		double *value = mxGetPr(data);
		getSphere2( mCount, (FBVector3d*)value, c, r );

		// output result in local space
		HFBModelSkeleton		pModel = new FBModelSkeleton("result");
		pModel->Size = r * 60.0;
		pModel->Show = true;
		// output vector as sphere center
		pModel->SetVector( c );
		
		// try to output result in world space
		HFBModelNull			pWorldModel	= new FBModelNull("worldResult");
		pWorldModel->Show = true;

		VectorTransform( c, parentTM, res );
		pWorldModel->SetVector( res );

		// free input array
		mxDestroyArray(data);
	}

	void CalculatePivotMatlab()
	{
		// Place the variables for vertices positions 
		engPutVariable(ep, "data", data);
		//! engPutVariable(ep, "n", mCount);
		
		// put data in handy form
		//engEvalString( ep, "n = 500;" );	// number of points samples
		engEvalString( ep, "Y = reshape(data,3,n);" );
		engEvalString( ep, "X = rot90(Y);" );
		// evalute equation using least squares algorithm
		engEvalString( ep, "x=X(:,1); y=X(:,2);	z=X(:,3);" );
		engEvalString( ep, "C = [x y z ones(length(x),1)] \\ -(x.^2 + y.^2 + z.^2);" );
		engEvalString( ep, "C(1)=-C(1)/2;	C(2)=-C(2)/2; C(3)=-C(3)/2;");
		engEvalString( ep, "C(4) = sqrt((C(1)^2 + C(2)^2 + C(3)^2) - C(4));" );

		// get convex hull coeff.
		mxArray *indData = engGetVariable( ep, "C" );

		if (indData != NULL)
		{
			double *value = mxGetPr(indData);
			FBVector3d	res, v(value[0], value[1], value[2]);

			// output result in local space
			HFBModelSkeleton		pModel = new FBModelSkeleton("result");
			pModel->Size = value[3] * 60.0;
			pModel->Show = true;
			// output vector as sphere center
			pModel->SetVector( v );
			
			// try to output result in world space
			HFBModelNull			pWorldModel	= new FBModelNull("worldResult");
			pWorldModel->Show = true;

			VectorTransform( v, parentTM, res );
			pWorldModel->SetVector( res );
			

			// free output array
			mxDestroyArray(indData);
		}

		// free input array
		mxDestroyArray(data);
	}

};
#endif