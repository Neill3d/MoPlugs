

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicmask_view.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//
// TODO: user options for grid visibility, grid density and grid snapping
//


//--- OR SDK include
#include <fbsdk/fbsdk.h>
//-- MCL include
//#include "Types.h"
#include <memory>
#include "Delegate.h"
#include "graphics\glslShader.h"

class ObjectChannel;

namespace DYNAMICMASK
{

//! declare classes
class BaseGraphManipulator;
class BaseGraphCommand;
class View2d;

// class for controling CEPoint memory allocations and deallocation
// auto garbage collector on Clear method
typedef std::shared_ptr<BaseGraphManipulator>		ManipulatorPtr;
typedef std::shared_ptr<BaseGraphCommand>			CommandPtr;

//////////////////////////////////////////////////////
//! BaseGraphManipulator
/*
	base class for a graph manipulator
*/
class BaseGraphManipulator
{
public:
	BaseGraphManipulator(View2d *_pGraph, bool _Modal=false)
		: pGraph(_pGraph)
		, Modal(_Modal)
	{
		mActive = false;
		down = false;
	}

	//! return manipulator caption for the viewport
	virtual const char	*GetCaption() = 0;
	//! mouse down processing
	virtual void		MouseDown(int x, int y, int pButtonKey, int modifier) = 0;
	//! mouse move processing
	virtual void		MouseMove(int x, int y, int pButtonKey, int modifier) = 0;
	//! mouse up processing
	virtual void		MouseUp(int x, int y, int pButtonKey, int modifier) = 0;
	//! output 2d drawing
	virtual void		PreDraw() = 0;
	virtual void		PostDraw() = 0;

	View2d		*GetGraphPtr()	{	return pGraph;	}
	bool		IsModal()	{	return Modal;		}
	void		SetModal(bool modal)	{	Modal = modal; }

	bool		IsActive()	{ return mActive; }
	void		SetActive(bool active) { mActive = active; }
private:
	//! graph view pointer
	View2d				*pGraph;
	//! is graph modal (global manip overlaps locals)
	bool				Modal;
	//! is manipulator is working now
	bool				mActive;

protected:

	FBVector3d	worldPos;
	FBVector3d	initialWorldPos;
	bool		down;

	void	DrawChannel(ObjectChannel *pChannel);
	void	SelectChannel(ObjectChannel *pChannel, const bool selectPoints, const bool selectTangents, const bool singleClick, const double limit, const FBVector3d &startPos);
	
	void	DragChannelDown(ObjectChannel *pChannel, const bool affectPoints, const bool affectTangents, const double limit);
	void	DragChannelMotion(ObjectChannel *pChannel);
	void	DragChannelUp(ObjectChannel *pChannel);

	void	MoveChannelDown(ObjectChannel *pChannel, bool &hasSelection);
	void	MoveChannelMotion(ObjectChannel *pChannel, FBVector4d &oldSpacePos, FBVector4d &newSpacePos, const bool hasSelection);
};


//////////////////////////////////////////////////////
//! BaseGraphCommand
/*!
	base class for a graph command
*/
class	BaseGraphCommand
{
public:
	virtual char *GetCaption() = 0;
	virtual bool	CheckInput(int pMouseX, int pMouseY, int Key, int Modifier) = 0;
	virtual void DoIt(View2d		*pGraph) = 0;
};

//////////////////////////////////////////////////////
//! GraphManager
/*!
	 class for operate with my custom graph manipulators
*/
//! delegate to a callback function (pass scan filename)
typedef Gallant::Delegate1<int> UpdateUIFunc;

#define	UPDATE_UI_TREE			1
#define UPDATE_UI_TOOLBAR		2
#define	UPDATE_UI_ALL			4

class GraphManager
{
public:
	//! a constructor
	GraphManager(View2d	*Graph);
	//! a destructor
	~GraphManager();
	//! add new global manipulator to a graph
	void AddGlobal();
	//! clear all global manips from a graph
	void ClearGlobalManip();
	//! set current local manipulator as current
	void SetManipulator(const char *szManipName);
	void SetManipulatorActive(const char *szManipName, bool active);
	//! clear local manipulator
	void ClearManipulator();
	//! add new graph command
	void AddCommand();
	void ClearCommands();

	void SetTransformMode(const int mode) {
		mCurrentTransformMode = mode;
	}

	BaseGraphManipulator	*GetCurrent() {
		return mManip;
	}

	const int GetCurrentManipulator() const {
		return mCurrentManipulator;
	}
	const int GetCurrentTransformMode() const {
		return mCurrentTransformMode;
	}

	//! mouse down processing
	void		MouseDown(int x, int y, int pButtonKey, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int pButtonKey, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int pButtonKey, int modifier);
	//! user input
	void		UserInput(int x, int y, int key, int modifier);
	//! output 2d drawing
	void		PreDraw();		//	drawing before scene
	void		PostDraw();		//	drawing after

private:
	//! pointer to a graph class
	View2d										*pGraph;
	//! current local manipulator
	BaseGraphManipulator						*mManip;

	int			mCurrentManipulator;	// current edit mode (select, move, rotate, scale, add shape)

	int			mCurrentTransformMode;	// transform mode (all, points, tangents)

	//! list of global manipulators
	FBArrayTemplate<ManipulatorPtr>				mGlobalManip;
	//! list of graph commands
	FBArrayTemplate<CommandPtr>					mCommands;
};


/////////////////////////////////////////////////////////
//! View2d
/**	
	Handle with 2d drawing canvas
*/
class View2d : public FBView, public	GraphManager
{
	FBClassDeclare( View2d, FBView );

protected:
	void BeginViewExpose(bool predraw = true);
	void EndViewExpose();

public:
	//! Constructor.
	View2d();
	
	//! Refresh callback.
	void Refresh(bool pNow=false);
	//! Resize callback.
	void ViewReSize(int w, int h);

	int CalcWidth();
	int CalcHeight();

	double	CalcXSens() {
		return (mXSpan[1] - mXSpan[0]) / mW;
	}
	double	CalcYSens() {
		return (mYSpan[1] - mYSpan[0]) / mH;
	}

	virtual void SetManipulator(const char *szManipName)
	{
		GraphManager::SetManipulator(szManipName);
		
		OnUpdateUI(UPDATE_UI_TOOLBAR);
	}

	virtual void SetTransformMode(const int mode)
	{
		GraphManager::SetTransformMode(mode);

		OnUpdateUI(UPDATE_UI_TOOLBAR);
	}

	UpdateUIFunc		OnUpdateUI;

	void ForceUpdateUI(int updateType);		// run update UI in main tool

	// transfer from cursor pos to world graph pos
	void ScreenToWorld( int x, int y, double *wx, double *wy )
	{
		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX, winY, winZ;
	
		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );
 
		winX = (float)x;
		//winY = (float)viewport[3] - (float)y;
		winY = y;
		//glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
		winZ = 0;

//		GLdouble wz = 0.0;
		//gluUnProject( winX, winY, winZ, mModelView, mProjection, mViewport, wx, wy, &wz);

		double sx = (mXSpan[1] - mXSpan[0]) / (mXRange[1] - mXRange[0]);
		double sy = (mYSpan[1] - mYSpan[0]) / (mYRange[1] - mYRange[0]);

		*wx = (double) x;
		*wy = (double) (mH -  y); 

		*wx -= mXSpan[0];
		*wy -= mYSpan[0];

		*wx /= sx;
		*wy /= sy;
		
		//glTranslatef( (float) mXSpan[0], (float) mYSpan[0], 0.0f);
		//glScalef( (float) sx, (float) sy, 0.0f );

	}

	//! Expose callback.
	void ViewExpose();
	/** Input callback function.
	*	\param pMouseX		Mouse X position.
	*	\param pMouseY		Mouse Y position.
	*	\param pAction		Mouse action.
	*	\param pButtonKey	Keyboard input.
	*	\param pModifier	Keyboard intput modifier.
	*/
	virtual void ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier);

	
	//! X axes - time range
	FBVector2d	&GetXSpan() {	return mXSpan;	}
	//! Y axes - value range
	FBVector2d	&GetYSpan() {	return mYSpan;	}
	bool	IsMouseDown() {	return mDown;	}
	int GetLastX() {	return mLastX;	}
	int GetLastY() {	return mLastY;	}
	int GetWidth() {	return mW;	}
	int GetHeight() {	return mH;	}

	void	SetClearColor(const FBVector4d	clearcolor)
	{
		mClearColor = clearcolor;
	}

	FBVector2d			mXRange;		// min & max of the X axes
	FBVector2d			mYRange;		// min & max of the Y axes

public:
	bool				mGrid;			// shot/hide grid
	FBVector2d			mGridDensity;	// grid spacing

	FBString			mStatusLine;

	FBBrowsingProperty	*mBrowsingProperty;		// component for displaying elements properties

private:
	int mW, mH;
	FBVector2d			mXSpan;		//!> x axes - time range
	FBVector2d			mYSpan;		//!> y axes - value range

	FBVector4d			mClearColor;

	bool				mDown;
	int					mLastX, mLastY;

	GLdouble			mModelView[16];
	GLdouble			mProjection[16];
	GLint				mViewport[4];

	int		mCanvasWidth;
	int		mCanvasHeight;

};

};