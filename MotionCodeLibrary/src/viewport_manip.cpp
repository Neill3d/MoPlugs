
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Viewport_manip.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>

#include "Viewport.h"
#include "viewport_manip.h"


////////////////////////////////////////////////////////////////////////////////////
// Mouse drag

//! a constructor
MouseObserver::MouseObserver()
{
	m_space = false;
	m_down = false;
}
//! mouse down processing
void MouseObserver::MouseDown(int x, int y)
{
	m_down = true;
	lastX = x;
	lastY = y;
}
//! post mouse move
void MouseObserver::MouseMove(int x, int y)
{
	lastX = x;
	lastY = y;
}
//! mouse up processing
void MouseObserver::MouseUp()
{
	m_down = false;
}

void MouseObserver::KeyDown(int key, int modifier)
{
	if ((key == VK_SPACE) && (modifier == 0))
		m_space = true;
}
void MouseObserver::KeyUp(int key, int modifier)
{
	if ( (key == VK_SPACE) && (modifier == 0))
		m_space = false;
}

//! return last cursor x
int	MouseObserver::GetLastX() {return lastX; }
//! return last cursor y
int MouseObserver::GetLastY() {return lastY; }
//! is mouse button down
bool	MouseObserver::IsMouseDown() {return m_down; }

bool	MouseObserver::IsSpaceDown()	{return m_space; }


////////////////////////////////////////////////////////////////////////////////////
// ProcessManipulator

ManipulatorManager::ManipulatorManager()
{
	mManip = NULL;
}
ManipulatorManager::~ManipulatorManager()
{
	if (mManip)
	{
		delete mManip;
		mManip = NULL;
	}
}
void ManipulatorManager::SetManipulator(const char *szManipName)
{
	ClearManipulator();
	/*
	// select new manipulator
	if ( strcmp(szManipName, VIEW_MANIPULATOR_SELECTION) == 0 )
		mManip = new ManipulatorSelection;
	else
		if (	strcmp(szManipName, VIEW_MANIPULATOR_PICK) == 0 )
			mManip = new ManipulatorPick;
			*/
}
void ManipulatorManager::ClearManipulator()
{
	if (mManip)
	{
		delete mManip;
		mManip = NULL;
	}
}

//! mouse down processing
void		ManipulatorManager::MouseDown(int x, int y)
{
	if (mManip)
		mManip->MouseDown(x, y);
}
//! mouse move processing
void		ManipulatorManager::MouseMove(int x, int y)
{
	if (mManip)
		mManip->MouseMove(x, y);
}
//! mouse up processing
void		ManipulatorManager::MouseUp(int x, int y)
{
	if (mManip)
		mManip->MouseUp(x, y);
}
//! output 2d drawing
void		ManipulatorManager::Draw2D()
{
	if (mManip) {
		mManip->Draw2D();
	}
}
//! output 3d drawing
void		ManipulatorManager::Draw3D()
{
	if (mManip)
		mManip->Draw3D();
}

