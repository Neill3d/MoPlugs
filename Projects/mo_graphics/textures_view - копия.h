#ifndef __ORTOOLVIEW3D_VIEW_H__
#define __ORTOOLVIEW3D_VIEW_H__

/**	\file	ortoolview3d_view.h
*	Tool with 3D viewer.
*/

// TODO: make selection on GPU by using transform feedback or direct SSBO writing

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <GL\glew.h>
#include "graphics_manager.h"

#include "GUI.h"

#include "types.h"

#include "shared_textures.h"


///////////////////////////////////////////////////////////////
//
class CGUITexturesComponent : public CGUIComponent
{
public:
	//! a constructor
	CGUITexturesComponent()
		: CGUIComponent()
	{
		mTextureId = 0;
	}

	void Set(const double x, const double y, const double w, const double h, const int _textureId)
	{
		CGUIComponent::Set(x, y, w, h);
		mTextureId = _textureId;
	}

protected:

	int		mTextureId;		// index in main textures handle array
};

/////////////////////////////////////////////////////////////////////////
//
class CGUITexturesModel : public CGUIWidgetModel
{
public:
	//! a constructor
	CGUITexturesModel();

	//
	void	AssignTexturesManager( CGPUTexturesManager *manager );
	void	AssignContextMenu(FBGenericMenu *pMenu);

	const int GetNumberOfTextures();
	const double GetPan();

protected:

	virtual CGUILayout *CreateLayout();
	virtual void OnMouseUp(const int pMouseX, const int pMouseY, const int pButtonKey, const bool hasMotion);

private:

	CGPUTexturesManager		*mTexturesManager;
	FBGenericMenu			*mMenu;

	// TODO: make an event for updating components from that manager
	bool	UpdateFromTexturesManager();
};


////////////////////////////////////////////////////////////////
// draw view using OpenGL nvFX
class CGUITexturesView : public CGUIWidgetView
{
public:
	//! a constructor
	CGUITexturesView(CGUITexturesModel *pModel);

	// call directly model method
	void AssignTexturesManager( CGPUTexturesManager *pManager );

protected:

	virtual void OnReSize(const int w, const int h);

	// update buffers for render
	virtual void OnUpdate();

	// do some drawing operation
	virtual void OnRender();

	virtual void OnMouseMotion(const int pMouseX, const int pMouseY, const int pButtonKey);

	void InitializeDrawBuffer();

	void FreeDrawBuffer();

private:

	int			mWidth;
	int			mHeight;

	GLuint		mBufferRows;
	GLuint		mBuffer;


	struct BufferPoint
	{
		float x;
		float y;
		float layer;
		float flag;	// is point selected (texture)
	};
	std::vector<BufferPoint>		mBufferData;

	struct RowPoint
	{
		float x1;
		float y1;
		float x2;
		float y2;
	};
	std::vector<RowPoint>			mBufferRowData;
};

/////////////////////////////////////////////////////////////////////////////////////
/**	View 3D class.
*/
class ORTexturesView3D : public FBView
{
	FBClassDeclare( ORTexturesView3D, FBView );

public:
	//! a constructor.
	ORTexturesView3D();
	//! a destructor.
	virtual void FBDestroy();

	//! Refresh callback.
	virtual void Refresh(bool pNow=false);
	//! Expose callback.
	virtual void ViewExpose();

	/**	Set view's viewport
    *	\param	pX	Viewport X value.
    *	\param	pY	Viewport Y value.
    *	\param	pW	Viewport W (width) value.
    *	\param	pH	Viewport H (height) value.
    *	\return	Operation was successful (\b true or \b false).
    */
    virtual bool SetViewport(int pX,int pY,int pW,int pH)
	{
		glViewport( 0, 0, pW, pH );
		return true;
	}

	/** Input callback function.
    *	\param pMouseX		Mouse X position.
    *	\param pMouseY		Mouse Y position.
    *	\param pAction		Mouse action.
    *	\param pButtonKey	Keyboard input.
    *	\param pModifier	Keyboard input modifier.
    */
    virtual void ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier);
	
	void		Resize(const int w, const int h);

private:
	CGUITexturesView		mView;
	CGUITexturesModel		mModel;
};

/*

class ORView3D : public FBView
{
	FBClassDeclare( ORView3D, FBView );
 
public:
	//! Constructor.
	ORView3D();
	//! Destructor
	virtual void FBDestroy();

	//! Refresh callback.
	virtual void Refresh(bool pNow=false);
	//! Expose callback.
	virtual void ViewExpose();

	
    virtual bool SetViewport(int pX,int pY,int pW,int pH)
	{
		glViewport( 0, 0, pW, pH );
		return true;
	}

	
    virtual void ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier);

	void		Resize(const int w, const int h);

	void	AssignTexturesManager( CGPUTexturesManager *manager )
	{
		mTexturesManager = manager;
	}

	void	AssignContextMenu(FBGenericMenu *pMenu)
	{
		mMenu = pMenu;
	}

private:

	//
	//	UI part

	FBGenericMenu		*mMenu;

	//
	//	logic part

	FBSystem    mSystem;

	bool		mMouseDown;
	int			mMouseLastX;

	bool		mIsMotion;
	bool		mNeedUpdate;

	// items are square rects
	double			minSize;	// possible minimum size
	double			minSpaceSize;	
	
	double			itemSize;	// current calculated size
	double			spaceSize;

	double			scale;

	int				itemsPerRow;	// calculate item size depends on that parameter
	int				numRows;
	
	double			pan;			// pan a viewport in space 
	double			percent;

	bool			showTitles;

	//
	void			InitDimentions();
	void			CalculateDimentions();

	void			SelectUnderCursor( const int pMouseX, const int pMouseY, const int pButtonKey, const int pModifier );

private:

	//
	//	graphic part

	CGPUTexturesManager		*mTexturesManager;

	GLuint		mBufferRows;
	GLuint		mBuffer;

	float		mTM[16];

	int			mWidth;
	int			mHeight;

	struct BufferPoint
	{
		float x;
		float y;
		float layer;
		float flag;	// is point selected (texture)
	};
	std::vector<BufferPoint>		mBufferData;

	struct RowPoint
	{
		float x1;
		float y1;
		float x2;
		float y2;
	};
	std::vector<RowPoint>			mBufferRowData;


	// 
	void			InitializeDrawBuffer();
	void			FreeDrawBuffer();
	void			UpdateBuffers();
	void			DrawTextures();

};
*/


#endif /* __ORTOOLVIEW3D_VIEW_H__ */
