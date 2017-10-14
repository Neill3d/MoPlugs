
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Viewport.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Viewport.h"
#include <GL\glew.h>

//! collect data about a viewport, camera and view plane
class ViewportImpl : public Viewport
{
public:
	//! a constructor
	ViewportImpl();

	//! ScreenToSpace
	/*!
		convert screen cursor coords to world 3d space position

		\param sX - mouse x
		\param sY - mouse y
		\param height - window height
		\return world 3d vector
	*/
	virtual FBVector3d ScreenToSpace( int sX, int sY, int height );

	//! SpaceToScreen
	/*!
		convert world 3d pos to screen one

		\param v - world point pos
		\return screen point pos
	*/
	virtual FBVector3d SpaceToScreen( const FBVector3d	&v );

	//! ScreenToSpaceRay
	/*!
		convert screen cursor coords to world 3d space position

		\param sX - mouse x
		\param sY - mouse y
		\param height - window height
		\param	start - ray start position (return)
		\param	end - ray end position (return)
	*/
	virtual void ScreenToSpaceRay( int sX, int sY, int height, FBVector3d &start, FBVector3d &end );

	//! HitPoints
	/*!
		hit points with mouse pick

		\param	selState - selection state flags for each point
		\return - number of selected points
	*/
	virtual int HitPoints(int X, int Y, FBArrayTemplate<FBVector3d> &points, FBArrayTemplate<bool>	&selState);
	virtual int HitPoints(int X, int Y, FBArrayTemplate<FBVector3d> &points, int hitrange = VIEWPORT_HIT_RANGE);		// return only one top point

	//! HitPoints
	/*!
		hit points with selection rect

		\param	selState - selection state flags for each point
		\return - number of selected points
	*/
	virtual int HitPoints(int X, int Y, int X2, int Y2, FBArrayTemplate<FBVector3d> &points, FBArrayTemplate<bool>	&selState);

	//! Update
	/*!
		update viewport

		\param currentCamera - current viewport camera (manipulator variable)
	*/
	virtual void Update(FBCamera	*currentCamera);

	//! viewport height
	virtual double getHeight();
	virtual double getWidth();
	virtual double getFOV();
	virtual double getNear();

	//! return calculated viewplane
	virtual FBVector4d	getViewPlane();

	//! draw debug lines
	virtual void Draw();

private:
	GLint	lViewport[4];
	FBMatrix	modelview;
	FBMatrix	projection;

	//! perspective or ortho projection
	bool				mPerspective;
	bool				mSystem; //! user camera or default views
	FBVector3d		mCamera;
	FBVector3d		mInterest;	//! importang to have camera target
	int				mScreenWidth;
	int				mScreenHeight;
	double			mFOV;
	double			mNearDist;
	double			mFarDist;

	FBVector4d		mViewPlane;
	FBVector3d		mViewRight;
	FBVector3d		mViewUp;

	//! calc viewplane for current camera
	void BuildViewPlane();
	// sort selection by depth
	int GetTopPoint(FBArrayTemplate<FBVector3d> &points, FBArrayTemplate<bool>	&selState);
};


//! a constructor
ViewportImpl::ViewportImpl()
{
	mPerspective = false;
	mSystem = true;
}

//! convert screen cursor coords to world 3d space position
/*!
	\param sX - mouse x
	\param sY - mouse y
	\param height - window height
	\return world 3d vector
*/
FBVector3d ViewportImpl::ScreenToSpace( int sX, int sY, int height )
{
	double    x,y,z;
  FBVector3d  res;

	//	Y -= height - viewport[3];
	gluUnProject(sX, height-sY, 0.0, modelview, projection, lViewport, &x, &y, &z);
	res = FBVector3d( x, y, z );

	return res;
}

FBVector3d ViewportImpl::SpaceToScreen( const FBVector3d	&v )
{
	double x,y,z;
	gluProject(v[0], v[1], v[2], modelview, projection, lViewport, &x, &y, &z);
	y = lViewport[3] - y;
	return FBVector3d(x, y, z);
}

void ViewportImpl::ScreenToSpaceRay( int sX, int sY, int height, FBVector3d &start, FBVector3d &end )
{
	//	Y -= height - viewport[3];
	gluUnProject(sX, height-sY, -10.0, modelview, projection, lViewport, &start[0], &start[1], &start[2]);
	gluUnProject(sX, height-sY, 0.0, modelview, projection, lViewport, &end[0], &end[1], &end[2]);
}

//return 1 if a is near of b
inline bool Near(int a, int b, int f)
{
	return ( (a+f)>b && (a-f)<b );
}

int ViewportImpl::GetTopPoint(FBArrayTemplate<FBVector3d> &points, FBArrayTemplate<bool>	&selState)
{
	int res = -1;
	int count = points.GetCount();

	int i=0;
	while ( (i < count) && (res < 0) )
	{
		if (selState[i]) res = i;
		i++;
	}
	if (res < 0) return res;

	double len = 0.0;
	FBVector3d	v;
	FBVector3d origin( modelview[12], modelview[13], modelview[14] );

	v = VectorSubtract(points[res], origin);
	len = VectorLength(v);

	for(i=res; i<count; i++)
	{
		if (!selState[i]) continue;
		double ilen = 0.0;
		v = VectorSubtract(points[i], origin);
		ilen = VectorLength(v);

		if (len > ilen)
		{
			len = ilen;
			res = i;
		}
	}
	return res;
}

//! hit points with mouse pick
/*!
	\param	selState - selection state flags for each point
*/
int ViewportImpl::HitPoints(int X, int Y, FBArrayTemplate<FBVector3d> &points, FBArrayTemplate<bool>	&selState)
{
	int res=0;
	FBVector3d	v;

	for (int i=0; i<points.GetCount(); i++)
	{
		v = SpaceToScreen( points[i] );

		selState[i] = Near(X, v[0], VIEWPORT_HIT_RANGE) && Near(Y, v[1], VIEWPORT_HIT_RANGE);
		if ( selState[i] )
			res++;
	}
	//if (!res) selState.Clear();

	// sort
	int ndx = GetTopPoint(points, selState);
	if (ndx >= 0)
	{
		for (int i=0; i<selState.GetCount(); i++)
			selState[i] = false;
		selState[ndx] = true;
	}

	return res;
}

int ViewportImpl::HitPoints(int X, int Y, FBArrayTemplate<FBVector3d> &points, int hitrange)
{
	FBVector3d	v;
	FBArrayTemplate<bool>	selState;
	selState.SetCount(points.GetCount() );

	for (int i=0; i<points.GetCount(); i++)
	{
		v = SpaceToScreen( points[i] );
		selState[i] = Near(X, v[0], hitrange) && Near(Y, v[1], hitrange);
	}

	// sort
	int ndx = GetTopPoint(points, selState);
	return ndx;
}

//! hit point with selection rect
int ViewportImpl::HitPoints(int X, int Y, int X2, int Y2, FBArrayTemplate<FBVector3d> &points, FBArrayTemplate<bool>	&selState)
{
	int res=0;
	FBVector3d	v;

	for (int i=0; i<points.GetCount(); i++)
	{
		v = SpaceToScreen( points[i] );

		selState[i] = (v[0] > X) && (v[0] < X2) && (v[1] > Y) && (v[1] < Y2);
		if ( selState[i] )
			res++;
	}
	//if (!res) selState.Clear();
	return res;
}

//! update viewport
/*!
	\param currentCamera - current viewport camera (manipulator variable)
*/
void ViewportImpl::Update(FBCamera	*currentCamera)
{
	glGetIntegerv(GL_VIEWPORT,lViewport);
#ifdef ORSDK2013
	modelview = currentCamera->GetMatrix(kFBModelView);
	projection = currentCamera->GetMatrix(kFBProjection);
#else
	currentCamera->GetCameraMatrix(modelview, kFBModelView);
	currentCamera->GetCameraMatrix(projection, kFBProjection);
#endif
	

	mNearDist = currentCamera->NearPlaneDistance;
	mFarDist = currentCamera->FarPlaneDistance;
	mPerspective = (currentCamera->Type == kFBCameraTypePerspective);

	mScreenHeight = lViewport[3];
	mScreenWidth = lViewport[2];
	mFOV = currentCamera->FieldOfView;

	mSystem = true;
	if (!currentCamera->SystemCamera)
	{
		if (currentCamera->Interest)
		{
			currentCamera->GetVector(mCamera);
			currentCamera->Interest->GetVector(mInterest);

			mSystem = false;
		}
	}

	//BuildViewPlane();
}

//! viewport height
double ViewportImpl::getHeight()
{
	return lViewport[3];
}
double ViewportImpl::getWidth()
{
	return lViewport[2];
}
double ViewportImpl::getFOV()
{
	return mFOV;
}
double ViewportImpl::getNear()
{
	return mNearDist;
}

//! return calculated viewplane
FBVector4d	ViewportImpl::getViewPlane()
{
	return mViewPlane;
}

//! draw debug lines
void ViewportImpl::Draw()
{
	double factor = 50.0;

	glColor3f(0.0, 1.0, 1.0);
	glBegin(GL_LINES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(mViewRight[0]*factor, mViewRight[1]*factor, mViewRight[2]*factor);

		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(mViewUp[0]*factor, mViewUp[1]*factor, mViewUp[2]*factor);

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(mViewPlane[0]*factor, mViewPlane[1]*factor, mViewPlane[2]*factor);
	glEnd();
}


Viewport *Viewport::newViewport()
{
	return new ViewportImpl();
}