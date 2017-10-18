
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicmask_viewTools.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "dynamicmask_viewTools.h"

#include <math.h>
#include "dynamicmask_common.h"
#include "shared_misc_mobu.h"
#include "algorithm\math3d_mobu.h"
#include "graphics\ParticlesDrawHelper.h"

using namespace DYNAMICMASK;

/////////////////////////////////////////////////////////
// ManipGraphPan

//! mouse down processing
void		ManipGraphPan::MouseDown(int x, int y, int pButtonKey, int modifier) 
{
}

//! mouse move processing
void		ManipGraphPan::MouseMove(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	FBVector2d		&lXSpan	= GetGraphPtr()->GetYSpan();
	FBVector2d		&lYSpan = GetGraphPtr()->GetXSpan();

	// x translation
	double xShift = (x - GetGraphPtr()->GetLastX());
	double yShift = (y - GetGraphPtr()->GetLastY());
	
	lXSpan[0] -= yShift;
	lXSpan[1] -= yShift;

	// y translation
	
	lYSpan[0] += xShift;
	lYSpan[1] += xShift;
}

//! mouse up processing
void		ManipGraphPan::MouseUp(int x, int y, int pButtonKey, int modifier)
{
}


/////////////////////////////////////////////////////////
// ManipGraphZoom

//! mouse down processing
void		ManipGraphZoom::MouseDown(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;
	mZoomCenterX = x;
	mZoomCenterY = y;

	mXSpan	= GetGraphPtr()->GetXSpan();
	mYSpan = GetGraphPtr()->GetYSpan();

	GetGraphPtr()->ScreenToWorld( x, y, &wx, &wy );
}

//! mouse move processing
void		ManipGraphZoom::MouseMove(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	FBVector2d		&lXSpan	= GetGraphPtr()->GetXSpan();
	FBVector2d		&lYSpan = GetGraphPtr()->GetYSpan();

	double	scaleFactorX, scaleFactorY;
	scaleFactorX = 1.0;
	scaleFactorY = 1.0;
	
	double deltaX = 0.01 * (GetGraphPtr()->GetLastX() - x);
	double deltaY = 0.01 * (GetGraphPtr()->GetLastY() - y);

		
	scaleFactorX -= deltaX;
    if (scaleFactorX <= 0.0)
        scaleFactorX = 1.0;
    scaleFactorY -= deltaY;
    if (scaleFactorY <= 0.0)
        scaleFactorY = 1.0;

	lXSpan[0] = (lXSpan[0] - wx) * scaleFactorX + wx;
	lXSpan[1] = (lXSpan[1] - wx) * scaleFactorX + wx;

	lYSpan[0] = (lYSpan[0] - wy) * scaleFactorX + wy;
	lYSpan[1] = (lYSpan[1] - wy) * scaleFactorX + wy;
	
}

//! mouse up processing
void		ManipGraphZoom::MouseUp(int x, int y, int pButtonKey, int modifier)
{
}


/////////////////////////////////////////////////////////
// ManipGraphSelect

//! mouse down processing
void ManipGraphSelect::MouseDown(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	if ( !GetGraphPtr() ) return;
	GetGraphPtr()->ScreenToWorld(x, y, &worldPos[0], &worldPos[1]);

	// grab the point
	down = false;
	if (pButtonKey == 1 && modifier == 0)
	{
		down = true;

		startWorldPos = worldPos;
	}
}

//! mouse move processing
void ManipGraphSelect::MouseMove(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	if (down) {
		GetGraphPtr()->ScreenToWorld(x, y, &worldPos[0], &worldPos[1]);
	}
}

//! mouse up processing
void ManipGraphSelect::MouseUp(int x, int y, int pButtonKey, int modifier)
{
	if (down)
	{
		if (pButtonKey == 1 && modifier == 0)
		{
			bool singleClick = VectorDistance(worldPos, startWorldPos) < 0.1;

			double limit = 10.0;
			limit *= 1.0 / GetGraphPtr()->CalcXSens();

			PropertiesClearAll(GetGraphPtr()->mBrowsingProperty);

			//PropertiesAdd( GetGraphPtr()->mBrowsingProperty, GetCurrentMaskPtr() );
			//PropertiesAdd( GetGraphPtr()->mBrowsingProperty, GetCurrentChannelPtr() );
			//PropertiesAdd( GetGraphPtr()->mBrowsingProperty, GetCurrentShapePtr() );

			// draw view content here
			ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
			
			const bool selectPoints = (GetGraphPtr()->GetCurrentTransformMode() == MANIPULATOR_TRANSFORM_ALL
				|| GetGraphPtr()->GetCurrentTransformMode() == MANIPULATOR_TRANSFORM_POINTS);

			const bool selectTangents = (GetGraphPtr()->GetCurrentTransformMode() == MANIPULATOR_TRANSFORM_ALL
				|| GetGraphPtr()->GetCurrentTransformMode() == MANIPULATOR_TRANSFORM_TANGENTS);

			if (channel)
			{
				SelectChannel(channel, selectPoints, selectTangents, singleClick, limit, startWorldPos);
			}
			else
			{
				// draw all 4 channels
				ObjectMask *mask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
				if (mask)
				{
					for (int i=0; i<mask->Components.GetCount(); ++i)
						SelectChannel( (ObjectChannel*) mask->Components[i], selectPoints, selectTangents, singleClick, limit, startWorldPos );

					mask->DoUpdate();
				}
			}
		}
	}

	down = false;
}

void ManipGraphSelect::PostDraw()
{
	double limit = 10.0;
	limit *= 1.0 / GetGraphPtr()->CalcXSens();

	if (down)
	{
		glColor3d(0.2, 0.6, 0.6);
		glBegin(GL_LINE_LOOP);

		glVertex2d(startWorldPos[0], startWorldPos[1]);
		glVertex2d(worldPos[0], startWorldPos[1]);
		glVertex2d(worldPos[0], worldPos[1]);
		glVertex2d(startWorldPos[0], worldPos[1]);

		glEnd();
	}


	glPointSize(5.0f);
	glColor3f(0.5, 0.5, 0.5);

	// draw view content here
	
	// draw view content here
	ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
	if (channel)
	{
		DrawChannel(channel);
	}
	else
	{
		// draw all 4 channels
		ObjectMask *mask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();

		if (mask)
		{
			for (int i=0; i<mask->Components.GetCount(); ++i)
				DrawChannel( (ObjectChannel*) mask->Components[i] );
		}
	}

	/*
	ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
	if (channel && channel->Components.GetCount() )
	{
		
		int numberOfPointsNormal = 0;
		int numberOfPointsSelected = 0;

		int numberOfComponents = channel->Components.GetCount();

		for (int i=0; i<numberOfComponents; ++i)
		{
			FBComponent *pComp = channel->Components[i];
			FBVector4d	pos;

			ObjectShape *shape = (ObjectShape*) pComp;

			if (pComp->Components.GetCount() )
			{
				FBMatrix m = shape->GetMatrix();

				for (int j=0; j<pComp->Components.GetCount(); ++j)
				{
					ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
				
					if (pKnot->Selected)
						numberOfPointsSelected++;
					else
						numberOfPointsNormal++;
				}
			}
		}

		// draw each shape (only selected points first
		if (numberOfPointsNormal > 0)
		{
			glPointSize(2.0f);
			glBegin(GL_POINTS);
			glColor3d(0.8, 0.8, 0.8);

			for (int i=0; i<numberOfComponents; ++i)
			{
				FBComponent *pComp = channel->Components[i];
				FBVector4d	pos;

				ObjectShape *shape = (ObjectShape*) pComp;

				if (pComp->Components.GetCount() )
				{
					FBMatrix m = shape->GetMatrix();

					for (int j=0; j<pComp->Components.GetCount(); ++j)
					{
						ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
						
						if (pKnot->Selected == false)
						{
							pKnot->Position.GetData(pos, sizeof(double)*3);	
							FBVectorMatrixMult(pos, m, pos);
							glVertex3dv(pos);
						}
					}
				}
			}
		
			glEnd();
		}

		//
		if (numberOfPointsSelected > 0)
		{
			glPointSize(5.0f);
			glBegin(GL_POINTS);
			glColor3d(0.2, 0.8, 0.2);

			for (int i=0; i<numberOfComponents; ++i)
			{
				FBComponent *pComp = channel->Components[i];
				FBVector4d	pos;

				ObjectShape *shape = (ObjectShape*) pComp;

				if (pComp->Components.GetCount() )
				{
					FBMatrix m = shape->GetMatrix();

					for (int j=0; j<pComp->Components.GetCount(); ++j)
					{
						ObjectKnot *pKnot = (ObjectKnot*) pComp->Components[j];
					
						if (pKnot->Selected)
						{
							pKnot->Position.GetData(pos, sizeof(double)*3);
							FBVectorMatrixMult(pos, m, pos);
							glVertex3dv(pos);
						}
					}
				}
			}
		
			glEnd();
		}
	}
	*/
}



/////////////////////////////////////////////////////////
// ManipGraphDrag

//! mouse down processing
void ManipGraphDrag::MouseDown(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	GetGraphPtr()->ScreenToWorld(x, y, &worldPos[0], &worldPos[1]);

	// grab the point
	down = false;
	mHasMotion = false;

	if (pButtonKey == 1 && modifier == 0)
	{
		double limit = (mRadius > 1.0) ? mRadius : 1.0;
		limit *= 1.0 / GetGraphPtr()->CalcXSens();

		const bool affectPoints = (GetGraphPtr()->GetCurrentTransformMode() == MANIPULATOR_TRANSFORM_ALL
				|| GetGraphPtr()->GetCurrentTransformMode() == MANIPULATOR_TRANSFORM_POINTS);

		const bool affectTangents = (GetGraphPtr()->GetCurrentTransformMode() == MANIPULATOR_TRANSFORM_ALL
				|| GetGraphPtr()->GetCurrentTransformMode() == MANIPULATOR_TRANSFORM_TANGENTS);

		// draw view content here
		ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
		ObjectMask *mask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
		if (channel)
		{
			DragChannelDown(channel, affectPoints, affectTangents, limit);
		}
		else if (mask)
		{
			// draw all 4 channels
			for (int i=0; i<mask->Components.GetCount(); ++i)
				DragChannelDown( (ObjectChannel*) mask->Components[i], affectPoints, affectTangents, limit );
		}
		if (mask)
			mask->DoUpdate();

		down = true;
	}
	else if (pButtonKey == 3 && modifier == 0)
	{
		down = true;
	}

	mLastX = x;

	mDrawCircleX = x;
	mDrawCircleY = y;
}

//! mouse move processing
void ManipGraphDrag::MouseMove(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	GetGraphPtr()->ScreenToWorld(x, y, &worldPos[0], &worldPos[1]);

	// draw view content here
	ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
	ObjectMask *mask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();

	if (channel)
	{
		DragChannelMotion(channel);
	}
	else if (mask)
	{
		// draw all 4 channels
		for (int i=0; i<mask->Components.GetCount(); ++i)
			DragChannelMotion( (ObjectChannel*) mask->Components[i] );
	}
	if (mask)
		mask->DoUpdate();

	if (pButtonKey == 3 && down)
	{
		if ( (x - mLastX) != 0 ) mHasMotion = true;

		mRadius += 0.1 * (x - mLastX);
	}
	else
	{
		mDrawCircleX = x;
		mDrawCircleY = y;
	}

	mLastX = x;
}

//! mouse up processing
void ManipGraphDrag::MouseUp(int x, int y, int pButtonKey, int modifier)
{
	down = false;

	ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
	ObjectMask *mask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
	if (channel)
	{
		DragChannelUp(channel);
	}
	else if (mask)
	{
		// draw all 4 channels
		for (int i=0; i<mask->Components.GetCount(); ++i)
			DragChannelUp( (ObjectChannel*) mask->Components[i] );
	}
	if (mask)
	{
		mask->DoUpdate();
	}

	// right mouse button - cancel the operation
	if (modifier == 0 && pButtonKey == 3 && mHasMotion == false)
	{
		GetGraphPtr()->SetManipulator( MANIPULATOR_GRAPH_SELECT );
		return;
	}
}



void ManipGraphDrag::PostDraw()
{
	double limit = 10.0;
	limit *= 1.0 / GetGraphPtr()->CalcXSens();


	glPointSize(5.0f);
	glColor3f(0.5, 0.5, 0.5);

	// draw view content here
	ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
	ObjectMask *mask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
	if (channel)
	{
		DrawChannel(channel);
	}
	else if (mask)
	{
		// draw all 4 channels
		for (int i=0; i<mask->Components.GetCount(); ++i)
			DrawChannel( (ObjectChannel*) mask->Components[i] );
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	const int width = GetGraphPtr()->GetWidth();
	const int height = GetGraphPtr()->GetHeight();

	glOrtho(0.0f, width, 0.0f, height, -1.0f, 1.0f);

	glTranslatef( (float) mDrawCircleX, (float) height - mDrawCircleY, 0.0f );

	glColor3d(1.0, 0.0, 0.0);
	DrawCircle( mRadius, 16 );

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


/////////////////////////////////////////////////////////
// ManipGraphMove

//! mouse down processing
void ManipGraphMove::MouseDown(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	// right mouse button - cancel the operation
	if (modifier == 0 && pButtonKey == 3)
	{
		GetGraphPtr()->SetManipulator( MANIPULATOR_GRAPH_SELECT );
		return;
	}

	GetGraphPtr()->ScreenToWorld(x, y, &worldPos[0], &worldPos[1]);

	// grab the point
	down = false;
	hasSelection = false;

	if (pButtonKey == 1 && modifier == 0)
	{
		down = true;

		// check if there any selected knot
		//
		ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
		ObjectMask *mask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
		if (channel)
		{
			MoveChannelDown(channel, hasSelection);
		}
		else if (mask)
		{
			// draw all 4 channels
			for (int i=0; i<mask->Components.GetCount(); ++i)
				MoveChannelDown( (ObjectChannel*) mask->Components[i], hasSelection );

		}
		if (mask)
			mask->DoUpdate();
			
	}
}

//! mouse move processing
void ManipGraphMove::MouseMove(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;
	if (down == false) return;

	FBVector4d oldSpacePos(worldPos[0], worldPos[1], 0.0, 1.0);
	FBVector4d newSpacePos(0.0, 0.0, 0.0, 1.0);
	GetGraphPtr()->ScreenToWorld(x, y, &newSpacePos[0], &newSpacePos[1]);

	//
	ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
	ObjectMask *mask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
	if (channel)
	{
		MoveChannelMotion(channel, oldSpacePos, newSpacePos, hasSelection);
	}
	else if (mask)
	{
		// draw all 4 channels
		
		for (int i=0; i<mask->Components.GetCount(); ++i)
			MoveChannelMotion( (ObjectChannel*) mask->Components[i], oldSpacePos, newSpacePos, hasSelection );
	}
	if (mask)
		mask->DoUpdate();
		

	worldPos[0] = newSpacePos[0];
	worldPos[1] = newSpacePos[1];
}

//! mouse up processing
void ManipGraphMove::MouseUp(int x, int y, int pButtonKey, int modifier)
{
	down = false;
}



void ManipGraphMove::PostDraw()
{
	double limit = 10.0;
	limit *= 1.0 / GetGraphPtr()->CalcXSens();


	glPointSize(5.0f);
	glColor3f(0.5, 0.5, 0.5);

	// draw view content here
	ObjectChannel *channel = DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr();
	ObjectMask *mask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();

	if (channel)
	{
		DrawChannel(channel);
	}
	else if (mask)
	{
		// draw all 4 channels
		for (int i=0; i<mask->Components.GetCount(); ++i)
			DrawChannel( (ObjectChannel*) mask->Components[i] );
	}
}

/////////////////////////////////////////////////////////
// ManipGraphShapeRect

//! mouse down processing
void		ManipGraphShapeRect::MouseDown(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	// right mouse button - cancel the operation
	if (modifier == 0 && pButtonKey == 3)
	{
		if (mNewShape.Ok() )
		{
			if (mNewShape.Ok() && pButtonKey == 3)
	
			DYNAMIC_MASK_OPERATIONS::RemoveShape(mNewShape);
			GetGraphPtr()->ForceUpdateUI(UPDATE_UI_TREE);

			mNewShape = nullptr;

			ObjectMask *pMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
			if (pMask)
				pMask->DoUpdate();
		}
		else
		{
			GetGraphPtr()->SetManipulator( MANIPULATOR_GRAPH_SELECT );
		}
	}

	//
	if (modifier == 0 && pButtonKey == 1)
	{

		if (mNewShape.Ok() )
		{
			DYNAMIC_MASK_OPERATIONS::RemoveShape(mNewShape);
			mNewShape = nullptr;
		}

		// create shape with 4 knots

		GetGraphPtr()->ScreenToWorld(x, y, &mCreationX, &mCreationY);

		mNewShape = DYNAMIC_MASK_OPERATIONS::AddShape( DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr() );
		DYNAMIC_MASK_OPERATIONS::AddKnot(mNewShape, FBVector3d( mCreationX-100.0,	mCreationY-100.0, 0.0), eObjectKnotType_CORNER );
		DYNAMIC_MASK_OPERATIONS::AddKnot(mNewShape, FBVector3d( mCreationX-100.0,	mCreationY+100.0, 0.0), eObjectKnotType_CORNER );
		DYNAMIC_MASK_OPERATIONS::AddKnot(mNewShape, FBVector3d( mCreationX+100.0,	mCreationY+100.0, 0.0), eObjectKnotType_CORNER );
		DYNAMIC_MASK_OPERATIONS::AddKnot(mNewShape, FBVector3d( mCreationX+100.0,	mCreationY-100.0, 0.0), eObjectKnotType_CORNER );
		mNewShape->ClosePath = true;

		GetGraphPtr()->ForceUpdateUI(UPDATE_UI_TREE);

		ObjectMask *pMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
		if (pMask)
			pMask->DoUpdate();
	}
}

//! mouse move processing
void		ManipGraphShapeRect::MouseMove(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	// control the shape size with mouse move event
	if (mNewShape.Ok() )
	{
		double wx, wy;

		GetGraphPtr()->ScreenToWorld(x, y, &wx, &wy);

		wx = abs(wx - mCreationX);
		wy = abs(wy - mCreationY);

		ObjectShape *pShape = mNewShape;
		if (pShape->Components.GetCount() == 4)
		{
			( (ObjectKnot*) pShape->Components[0] )->Position = FBVector2d(mCreationX-wx, mCreationY-wy);
			( (ObjectKnot*) pShape->Components[1] )->Position = FBVector2d(mCreationX-wx, mCreationY+wy);
			( (ObjectKnot*) pShape->Components[2] )->Position = FBVector2d(mCreationX+wx, mCreationY+wy);
			( (ObjectKnot*) pShape->Components[3] )->Position = FBVector2d(mCreationX+wx, mCreationY-wy);
		}

		ObjectMask *pMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
		if (pMask)
			pMask->DoUpdate();
	}
}

//! mouse up processing
void		ManipGraphShapeRect::MouseUp(int x, int y, int pButtonKey, int modifier)
{
	// right mouse button - cancel the operation

	if (mNewShape.Ok() && pButtonKey == 1)
	{
		mNewShape = nullptr;
	}
}

/////////////////////////////////////////////////////////
// ManipGraphShapeRect

//! mouse down processing
void		ManipGraphShapeCircle::MouseDown(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	// right mouse button - cancel the operation
	if (modifier == 0 && pButtonKey == 3)
		GetGraphPtr()->SetManipulator( MANIPULATOR_GRAPH_SELECT );

	if (modifier == 0 && pButtonKey == 1)
	{
		// create shape with 4 knots

		double wx, wy;

		GetGraphPtr()->ScreenToWorld(x, y, &wx, &wy);

		ObjectShape *newShape = DYNAMIC_MASK_OPERATIONS::AddShape( DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr() );
		DYNAMIC_MASK_OPERATIONS::AddKnot(newShape, FBVector3d( wx-100.0,	wy, 0.0),		eObjectKnotType_SMOOTH );
		DYNAMIC_MASK_OPERATIONS::AddKnot(newShape, FBVector3d( wx,			wy+100.0, 0.0), eObjectKnotType_SMOOTH );
		DYNAMIC_MASK_OPERATIONS::AddKnot(newShape, FBVector3d( wx+100.0,	wy, 0.0),		eObjectKnotType_SMOOTH );
		DYNAMIC_MASK_OPERATIONS::AddKnot(newShape, FBVector3d( wx,			wy-100.0, 0.0), eObjectKnotType_SMOOTH );
		newShape->ClosePath = true;

		GetGraphPtr()->ForceUpdateUI(UPDATE_UI_TREE);

		ObjectMask *pMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
		if (pMask)
			pMask->DoUpdate();
	}
}

//! mouse move processing
void		ManipGraphShapeCircle::MouseMove(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;


}

//! mouse up processing
void		ManipGraphShapeCircle::MouseUp(int x, int y, int pButtonKey, int modifier)
{
}

/////////////////////////////////////////////////////////
// ManipGraphShapeRect

//! mouse down processing
void		ManipGraphShapeSpline::MouseDown(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	// right mouse button - cancel the operation
	if (modifier == 0 && pButtonKey == 3)
	{
		if (mCreation == false)
		{
			GetGraphPtr()->SetManipulator( MANIPULATOR_GRAPH_SELECT );
		}
		else
		{
			if (mLastKnot.Ok() )
			{
				mNewShape->Components.Remove(mLastKnot);
				mLastKnot->FBDelete();
				mLastKnot = nullptr;
			}

			if (mNewShape->Components.GetCount() > 3)
			{
				if (1 == FBMessageBox( "Spline creation", "Do you want to close a path?", "Yes", "No" ) )
					mNewShape->ClosePath = true;
			}
			else if (mNewShape->Components.GetCount() < 2)
			{
				DYNAMIC_MASK_OPERATIONS::RemoveShape(mNewShape);
			}

			GetGraphPtr()->ForceUpdateUI(UPDATE_UI_TREE);

			ObjectMask *pMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
			if (pMask)
				pMask->DoUpdate();

			mNewShape = nullptr;
			mCreation = false;
		}
	}

	if (modifier == 0 && pButtonKey == 1)
	{
		// create shape with 4 knots

		double wx, wy;

		GetGraphPtr()->ScreenToWorld(x, y, &wx, &wy);

		if (mCreation == false)
		{
			mNewShape = DYNAMIC_MASK_OPERATIONS::AddShape( DYNAMIC_MASK_OPERATIONS::GetCurrentChannelPtr() );
			mLastKnot = DYNAMIC_MASK_OPERATIONS::AddKnot(mNewShape, FBVector3d( wx, wy, 0.0),		eObjectKnotType_SMOOTH );
			mLastKnot = DYNAMIC_MASK_OPERATIONS::AddKnot(mNewShape, FBVector3d( wx, wy, 0.0),		eObjectKnotType_SMOOTH );
			mNewShape->ClosePath = false;

			mCreation = true;
		}
		else if (mNewShape.Ok() )
		{
			mLastKnot = DYNAMIC_MASK_OPERATIONS::AddKnot(mNewShape, FBVector3d( wx, wy, 0.0),		eObjectKnotType_SMOOTH );
		}

		ObjectMask *pMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
		if (pMask)
			pMask->DoUpdate();
	}
}

//! mouse move processing
void		ManipGraphShapeSpline::MouseMove(int x, int y, int pButtonKey, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	// TODO: control the last knot
	if (mLastKnot.Ok() )
	{
		double wx, wy;

		GetGraphPtr()->ScreenToWorld(x, y, &wx, &wy);

		mLastKnot->Position = FBVector2d(wx, wy);

		ObjectMask *pMask = DYNAMIC_MASK_OPERATIONS::GetCurrentMaskPtr();
		if (pMask)
			pMask->DoUpdate();
	}
}

//! mouse up processing
void		ManipGraphShapeSpline::MouseUp(int x, int y, int pButtonKey, int modifier)
{
	// check if we can close a path
}