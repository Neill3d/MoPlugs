
#include "GUI.h"

#include <glm\glm\mat4x4.hpp>
#include <glm\glm\gtc\matrix_transform.hpp>
#include <glm\glm\gtc\type_ptr.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////
// CGUIWidgetModel

//! a constructor
CGUIWidgetModel::CGUIWidgetModel()
	: mComponentsLayout( CreateLayout() )
{
	mWidth = 512;
	mHeight = 512;

	mPan = 0.0;
}

//! a destructor
CGUIWidgetModel::~CGUIWidgetModel()
{
	DisconnectView();

	Free();
}

void CGUIWidgetModel::ConnectToView(CGUIWidgetView *pView)
{
	mView = pView;
	if (mView)
		mView->_mModel = this;
}

void CGUIWidgetModel::DisconnectView()
{
	if (mView)
		mView->_mModel = nullptr;
	mView = nullptr;
}

void CGUIWidgetModel::Free()
{
	mNumberOfComponents = 0;

	if (mView)
		mView->Free();

	OnFree();
}

void CGUIWidgetModel::ReSize(const int w, const int h)
{
	mWidth = w;
	mHeight = h;

	OnReSize(w, h);
	if (mView)
		mView->OnReSize(w, h);
}

// calculate world position for the elements
// INPUT: canvas position, layout style, element size, spacing size
// OUTPUT: calculated world position for the widget components
//
// TODO: this one should depend on the layout style (listbox style, grid style)
//


CGUILayout::CGUILayout()
{
}

void CGUILayout::Calculate(const int w, const int h, const int numberOfComponents, CGUIComponent *comps, const CGUIWidgetOptions &options)
{
}

CGUILayoutHGrid::CGUILayoutHGrid()
{
}

void CGUILayoutHGrid::Calculate(const int w, const int h, const int numberOfComponents, CGUIComponent *comps, const CGUIWidgetOptions &options)
{
	if (numberOfComponents == 0 || comps == nullptr) 
		return;

	// calculate number of elements per col
	int elemsPerCol = h / (options.size.element + options.size.space);
	if (elemsPerCol < 1) elemsPerCol = 1;
	int elemsPerRow = (int) ceil(1.0 * numberOfComponents / elemsPerCol);

	int i=0;
	int nCol = 0;
	int nRow = 0;

	const double fullElemSize = options.size.element + options.size.space;

	while(i < numberOfComponents)
	{
		const double elemWidth = comps->GetRegionW() * options.size.element;
		const double elemHeight = comps->GetRegionH() * options.size.element;

		comps->SetPosition( options.size.space + nCol * fullElemSize, 
							options.size.space + nRow * fullElemSize, 
							elemWidth, 
							elemHeight );

		nCol++;
		if (nCol > elemsPerCol)
		{
			nCol = 0;
			nRow++;
		}

		i++;
	}
}

void CGUIWidgetModel::CalculatePosition(const int *w, const int *h)
{
	const int width = (w==nullptr) ? mWidth : *w;
	const int height = (h==nullptr) ? mHeight : *h;

	CGUIComponent *comps = mComponents.get();
	if (comps == nullptr) return;

	if (mComponentsLayout.get() )
		mComponentsLayout->Calculate(width, height, mNumberOfComponents, comps, mOptions);
}

// function takes mouse position and check if there is any element under cursor
//  it's a logic part
void CGUIWidgetModel::SelectUnderCursor( const int pMouseX, const int pMouseY, const int pButtonKey, const int pModifier )
{
	CGUIComponent *comps = mComponents.get();
	if (mNumberOfComponents == 0 || comps == nullptr) return;

	int realY = mHeight - pMouseY;

	// rect pos - MVP * base position
	// mouse pos = Proj * mouse coords

	glm::mat4 projection = glm::ortho( 0.0f, (float) mWidth, 0.0f, (float) mHeight, -1.0f, 1.0f );
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3( (float) mPan, 0.0f, 0.0f));
	glm::mat4 MVP = projection * ViewTranslate;

	glm::vec4 mousePos( (float) pMouseX, (float) realY, 0.0, 1.0 );
	mousePos = projection * mousePos;

	glm::mat4 invMVP = glm::inverse(MVP);

	glm::vec4 worldMousePos = invMVP * mousePos;

	for (int i=0; i<mNumberOfComponents; ++i)
	{
		comps->Select(worldMousePos.x, worldMousePos.y);
	}
}

void CGUIWidgetModel::Input(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{

	switch( pAction )
	{
	case kFBButtonPress:
		mMouseDown = true;
		mMouseMotion = false;
		mMouseLastX = pMouseX;

		OnMouseDown(pMouseX, pMouseY, pButtonKey);
		if (mView)
			mView->OnMouseDown(pMouseX, pMouseY, pButtonKey);

		break;
	case kFBButtonRelease:

		if (mMouseMotion == false)
		{
			// select a node under cursor

			SelectUnderCursor( pMouseX, pMouseY, pButtonKey, pModifier );
			mNeedUpdate = true;
		}

		OnMouseUp(pMouseX, pMouseY, pButtonKey, mMouseMotion);
		if (mView)
			mView->OnMouseUp(pMouseX, pMouseY, pButtonKey, mMouseMotion);

		mMouseDown = false;
		break;

	case kFBMotionNotify:

		//ORView3DEffect::instance().SetMousePosition( (float)pMouseX, (float)(mHeight - pMouseY) );
	
		if (mMouseDown)
		{
			if (pMouseX != mMouseLastX) mMouseMotion = true;

			// do some pan
			mPan += ( pMouseX - mMouseLastX );
			mNeedUpdate = true;
		}

		OnMouseMotion(pMouseX, pMouseY, pButtonKey);
		if (mView)
			mView->OnMouseMotion(pMouseX, pMouseY, pButtonKey);

		mMouseLastX = pMouseX;
		mMouseLastY = pMouseY;

		break;
	}

}

void CGUIWidgetModel::Render()
{
	if (mView)
	{
		if (mNeedUpdate)
			mView->OnUpdate();

		mView->OnRender();
	}
}

const int CGUIWidgetModel::GetNumberOfComponents() const
{
	return mNumberOfComponents;
}

const int CGUIWidgetModel::GetNumberOfRows() const
{
	return mNumberOfRows;
}

const int CGUIWidgetModel::GetNumberOfCols() const
{
	return mNumberOfCols;
}

const double CGUIWidgetModel::GetElementSize() const
{
	return mOptions.size.element;
}

const double CGUIWidgetModel::GetSpaceSize() const
{
	return mOptions.size.space;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//

CGUIWidgetView::CGUIWidgetView(CGUIWidgetModel *pModel)
	: _mModel(pModel)
{
	_mNeedUpdate = false;
}

CGUIWidgetView::~CGUIWidgetView()
{
	DisconnectModel();

	OnFree();
}

void CGUIWidgetView::ConnectToModel( CGUIWidgetModel *pModel )
{
	_mModel = pModel;
	if (_mModel)
		_mModel->mView = this;
}

void CGUIWidgetView::DisconnectModel()
{
	if (_mModel)
		_mModel->mView = nullptr;
	_mModel = nullptr;
}

void CGUIWidgetView::Free()
{
	if (_mModel)
		_mModel->Free();

	OnFree();
}

void CGUIWidgetView::Render()
{
	OnRender();
}

void CGUIWidgetView::OnRender()
{

	if (nullptr == _mModel)
	{
		return;
	}

	const int numberOfComponents = _mModel->GetNumberOfComponents();
	for (int i=0; i<numberOfComponents; ++i)
	{
		CGUIComponent *pComp = _mModel->GetComponentPtr(i);

		pComp->Render();
	}
}