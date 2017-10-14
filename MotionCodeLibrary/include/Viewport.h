
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Viewport.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma		once

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "algorithm/math3d_mobu.h"

#define	VIEWPORT_HIT_RANGE		3

//! collect data about a viewport, camera and view plane
class	Viewport
{
public:
	static Viewport *newViewport();

public:
	
	//! ScreenToSpace
	/*!
		convert screen cursor coords to world 3d space position

		\param sX - mouse x
		\param sY - mouse y
		\param height - window height
		\return world 3d vector
	*/
	virtual FBVector3d ScreenToSpace( int sX, int sY, int height ) = 0;

	//! SpaceToScreen
	/*!
		convert world 3d pos to screen one

		\param v - world point pos
		\return screen point pos
	*/
	virtual FBVector3d SpaceToScreen( const FBVector3d	&v ) = 0;

	//! ScreenToSpaceRay
	/*!
		convert screen cursor coords to world 3d space position

		\param sX - mouse x
		\param sY - mouse y
		\param height - window height
		\param	start - ray start position (return)
		\param	end - ray end position (return)
	*/
	virtual void ScreenToSpaceRay( int sX, int sY, int height, FBVector3d &start, FBVector3d &end ) = 0;

	//! HitPoints
	/*!
		hit points with mouse pick

		\param	selState - selection state flags for each point
		\return - number of selected points
	*/
	virtual int HitPoints(int X, int Y, FBArrayTemplate<FBVector3d> &points, FBArrayTemplate<bool>	&selState) = 0;
	virtual int HitPoints(int X, int Y, FBArrayTemplate<FBVector3d> &points, int hitrange = VIEWPORT_HIT_RANGE) = 0;		// return only one top point

	//! HitPoints
	/*!
		hit points with selection rect

		\param	selState - selection state flags for each point
		\return - number of selected points
	*/
	virtual int HitPoints(int X, int Y, int X2, int Y2, FBArrayTemplate<FBVector3d> &points, FBArrayTemplate<bool>	&selState) = 0;

	//! Update
	/*!
		update viewport

		\param currentCamera - current viewport camera (manipulator variable)
	*/
	virtual void Update(FBCamera	*currentCamera) = 0;

	//! viewport height
	virtual double getHeight() = 0;
	virtual double getWidth() = 0;
	virtual double getFOV() = 0;
	virtual double getNear() = 0;

	//! return calculated viewplane
	virtual FBVector4d	getViewPlane() = 0;

	//! draw debug lines
	virtual void Draw() = 0;
};
