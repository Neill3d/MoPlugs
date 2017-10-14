
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: viewport_manip.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

/***************************************************************************************
 Neill3d class library - ViewInput.h
 ** operations with user input **

 (C) 2009, Inc. and/or its licensors
 All rights reserved.

 Author: Solohin Sergey (Neill)
	e-mail to: Neill.Solow@gmail.com
	www.neill3d.com

***************************************************************************************/

//--- SDK include
#include <fbsdk/fbsdk.h>

/////////////////////////////////////////////////////////////////////////////////
// MouseDrag
/*
	*	 collect data about user input, mouse down state, last cursor x, y
	*/
class MouseObserver
{
public:
	//! a constructor
	MouseObserver();

	//! mouse down processing
	void		MouseDown(int x, int y);
	//! post mouse move
	void		MouseMove(int x, int y);
	//! mouse up processing
	void		MouseUp();

	void	KeyDown(int key, int modifier);

	void	KeyUp(int key, int modifier);

	//! return last cursor x
	int	GetLastX();
	//! return last cursor y
	int GetLastY();
	//! is mouse button down
	bool	IsMouseDown();

	bool	IsSpaceDown();

private:
	bool		m_space;
	bool		m_down;
	int			lastX, lastY;
};



/////////////////////////////////////////////////////////////////////////////////
// BaseManipulator
/*!
	*	manipulator with static strategy pattern
*/
class BaseManipulator
{
public:
	//! return manipulator caption for the viewport
	virtual char	*GetCaption() = 0;
	//! mouse down processing
	virtual void		MouseDown(int x, int y) = 0;
	//! mouse move processing
	virtual void		MouseMove(int x, int y) = 0;
	//! mouse up processing
	virtual void		MouseUp(int x, int y) = 0;
	//! output 2d drawing
	virtual void		Draw2D() = 0;
	//! output 3d drawing
	virtual void		Draw3D() = 0;
};


/////////////////////////////////////////////////////////////////////////////////
// ProcessManipulator
/*!
	* class for operate with my custom manipulators
	*/
class ManipulatorManager
{
public:
	//! a constuctor
	ManipulatorManager();
	//! a destructor
	~ManipulatorManager();
	//! set current manipulator
	/*!
		\param	szManipName - manipulator name
	*/
	void SetManipulator(const char *szManipName);
	void ClearManipulator();

	//! mouse down processing
	void		MouseDown(int x, int y);
	//! mouse move processing
	void		MouseMove(int x, int y);
	//! mouse up processing
	void		MouseUp(int x, int y);
	//! output 2d drawing
	void		Draw2D();
	//! output 3d drawing
	void		Draw3D();

private:
	//! current manipulator
	BaseManipulator		*mManip;
};
