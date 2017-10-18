
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicmask_viewTools.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "dynamicmask_view.h"
#include "dynamicmask_object.h"

//////
// forward declaration

class ObjectChannel;

namespace DYNAMICMASK
{

//! plugin identify
#define	MANIPULATOR_GRAPH_PAN			"Pan"
#define	MANIPULATOR_GRAPH_ZOOM			"Zoom"
#define	MANIPULATOR_GRAPH_SELECT		"Select"
#define	MANIPULATOR_GRAPH_DRAG			"Drag"
#define	MANIPULATOR_GRAPH_MOVE			"Move"
#define	MANIPULATOR_GRAPH_SHAPE_RECT			"Make a rectangle"
#define	MANIPULATOR_GRAPH_SHAPE_CIRCLE			"Make a circle"
#define	MANIPULATOR_GRAPH_SHAPE_SPLINE			"Make a spline poly"
#define MANIPULATOR_GRAPH_SHAPE_FREELINE		"Make a free line"

#define MANIPULATOR_TRANSFORM_ALL			0
#define MANIPULATOR_TRANSFORM_POINTS		1
#define MANIPULATOR_TRANSFORM_TANGENTS		2

////////////////////////////////////////////////////////////
//! ManipGraphPan
/*!
	local manipulator
	
	moving along graph
*/
class ManipGraphPan	:	public BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphPan(View2d	*_pGraph, bool _Modal=false)
			: BaseGraphManipulator(_pGraph, _Modal)
	{}

	//! return manipulator caption for the viewport
	const char	*GetCaption()
	{ return MANIPULATOR_GRAPH_PAN; }
	//! mouse down processing
	void		MouseDown(int x, int y, int pButtonKey, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int pButtonKey, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int pButtonKey, int modifier);
	//! output 2d drawing
	void		PreDraw()
	{}
	void		PostDraw()
	{}
};

////////////////////////////////////////////////////////////
//! ManipGraphZoom
/*!
	local manipulator

	Zoom zoom zoom...
*/
class ManipGraphZoom	:	public BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphZoom(View2d	*_pGraph, bool _Modal=false)
			: BaseGraphManipulator(_pGraph, _Modal)
	{}

	//! return manipulator caption for the viewport
	const char	*GetCaption()
	{ return MANIPULATOR_GRAPH_ZOOM; }
	//! mouse down processing
	void		MouseDown(int x, int y, int pButtonKey, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int pButtonKey, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int pButtonKey, int modifier);
	//! output 2d drawing
	void		PreDraw()
	{}
	void		PostDraw()
	{}
private:
	double	wx;
	double	wy;
	int		mZoomCenterX;
	int		mZoomCenterY;
	FBVector2d		mXSpan;
	FBVector2d		mYSpan;
};

////////////////////////////////////////////////////////////
//! ManipGraphSelect
/*!
	local manipulator

	Zoom zoom zoom...
*/
class ManipGraphSelect	:	public BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphSelect(View2d	*_pGraph, bool _Modal=false)
			: BaseGraphManipulator(_pGraph, _Modal)
	{}

	//! return manipulator caption for the viewport
	const char	*GetCaption()
	{ return MANIPULATOR_GRAPH_SELECT; }
	//! mouse down processing
	void		MouseDown(int x, int y, int pButtonKey, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int pButtonKey, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int pButtonKey, int modifier);
	//! output 2d drawing
	void		PreDraw()
	{}
	void		PostDraw();
private:
	
	FBVector3d	startWorldPos;
};

////////////////////////////////////////////////////////////
//! ManipGraphDrag
/*!
	local manipulator

	drag control points
*/
class ManipGraphDrag	:	public BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphDrag(View2d	*_pGraph, bool _Modal=false)
			: BaseGraphManipulator(_pGraph, _Modal)
	{
		mRadius = 10.0;
		mLastX = 0;
		mHasMotion = false;

		mDrawCircleX = 0;
		mDrawCircleY = 0;
	}

	//! return manipulator caption for the viewport
	const char	*GetCaption()
	{ return MANIPULATOR_GRAPH_DRAG; }
	//! mouse down processing
	void		MouseDown(int x, int y, int pButtonKey, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int pButtonKey, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int pButtonKey, int modifier);
	//! output 2d drawing
	void		PreDraw()
	{}
	void		PostDraw();

protected:

	double		mRadius;		// magnet radius
	int			mLastX;
	bool		mHasMotion;

	int			mDrawCircleX;
	int			mDrawCircleY;
};

////////////////////////////////////////////////////////////
//! ManipGraphMove
/*!
	local manipulator

	move selected or all control points
*/
class ManipGraphMove	:	public BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphMove(View2d	*_pGraph, bool _Modal=false)
			: BaseGraphManipulator(_pGraph, _Modal)
	{
		hasSelection = false;
	}
	//! return manipulator caption for the viewport
	const char	*GetCaption()
	{ return MANIPULATOR_GRAPH_MOVE; }
	//! mouse down processing
	void		MouseDown(int x, int y, int pButtonKey, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int pButtonKey, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int pButtonKey, int modifier);
	//! output 2d drawing
	void		PreDraw()
	{}
	void		PostDraw();
private:
	bool		hasSelection;
};

////////////////////////////////////////////////////////////
//! ManipGraphShape
/*!
	manipulator to make rectangles shapes
*/
class ManipGraphShapeRect	:	public BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphShapeRect(View2d	*_pGraph, bool _Modal=false)
			: BaseGraphManipulator(_pGraph, _Modal)
			, mNewShape(nullptr)
	{}

	virtual ~ManipGraphShapeRect()
	{}

	//! return manipulator caption for the viewport
	const char	*GetCaption()
	{ return MANIPULATOR_GRAPH_SHAPE_RECT; }
	//! mouse down processing
	void		MouseDown(int x, int y, int pButtonKey, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int pButtonKey, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int pButtonKey, int modifier);
	//! output 2d drawing
	void		PreDraw()
	{}
	void		PostDraw()
	{}

protected:

	double		mCreationX;
	double		mCreationY;

	HdlFBPlugTemplate<ObjectShape>		mNewShape;
};

////////////////////////////////////////////////////////////
//! ManipGraphShapeCircle
/*!
	manipulator to make circles shapes
*/
class ManipGraphShapeCircle	:	public BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphShapeCircle(View2d	*_pGraph, bool _Modal=false)
			: BaseGraphManipulator(_pGraph, _Modal)
	{}
	//! return manipulator caption for the viewport
	const char	*GetCaption()
	{ return MANIPULATOR_GRAPH_SHAPE_CIRCLE; }
	//! mouse down processing
	void		MouseDown(int x, int y, int pButtonKey, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int pButtonKey, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int pButtonKey, int modifier);
	//! output 2d drawing
	void		PreDraw()
	{}
	void		PostDraw()
	{}
};

////////////////////////////////////////////////////////////
//! ManipGraphShapeCustom
/*!
	manipulator to make custom shapes
*/
class ManipGraphShapeSpline	:	public BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphShapeSpline(View2d	*_pGraph, bool _Modal=false)
			: BaseGraphManipulator(_pGraph, _Modal)
			, mCreation(false)
	{}

	virtual ~ManipGraphShapeSpline()
	{}

	//! return manipulator caption for the viewport
	const char	*GetCaption()
	{ return MANIPULATOR_GRAPH_SHAPE_SPLINE; }
	//! mouse down processing
	void		MouseDown(int x, int y, int pButtonKey, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int pButtonKey, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int pButtonKey, int modifier);
	//! output 2d drawing
	void		PreDraw()
	{}
	void		PostDraw()
	{}

protected:

	bool		mCreation;
	HdlFBPlugTemplate<ObjectShape>		mNewShape;
	HdlFBPlugTemplate<ObjectKnot>		mLastKnot;
};

////////////////////////////////////////////////////////////
//! GraphFitCommand
/*!
	fitting graph view to the current graph

	change timespan, valuespan
*/
class	GraphFitCommand : public BaseGraphCommand
{
public:
	virtual char *GetCaption()
	{
		return "GraphFit";
	}
	virtual bool	CheckInput(int pMouseX, int pMouseY, int Key, int Modifier)
	{
		if ( (Key == 'F') || (Key == 'f' ) )
			return true;
		return false;
	}
	virtual void DoIt(View2d	*pGraph)
	{
		FBVector2d	&lXSpan = pGraph->GetXSpan();
		FBVector2d	&lYSpan = pGraph->GetYSpan();

		lXSpan = pGraph->mXRange;
		lYSpan = pGraph->mYRange;
	}
};

};