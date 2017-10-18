
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: GUI.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
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

	mPanX = 0.0;
	mPanY = 0.0;
	mPanZ = 0.0;
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

void CGUILayout::Calculate(std::list<CGUIWidgetModel*> &components, const CGUIWidgetOptions &options)
{
}

CGUILayoutHGrid::CGUILayoutHGrid()
{
}

void CGUILayoutHGrid::Calculate(std::list<CGUIWidgetModel*> &components, const CGUIWidgetOptions &options)
{
	if (components.size() == 0) 
		return;

	const int numberOfComponents = (int) components.size();

	// calculate number of elements per col
	int elemsPerCol = options.height / (options.size.element + options.size.space);
	if (elemsPerCol < 1) elemsPerCol = 1;
	int elemsPerRow = (int) ceil(1.0 * numberOfComponents / elemsPerCol);

	int nCol = 0;
	int nRow = 0;

	const double fullElemSize = options.size.element + options.size.space;

	auto iter = components.begin();
	while(iter!=components.end() )
	{
		const double elemWidth = (*iter)->GetRegionW() * options.size.element;
		const double elemHeight = (*iter)->GetRegionH() * options.size.element;

		(*iter)->SetPosition( options.size.space + nCol * fullElemSize, 
							options.size.space + nRow * fullElemSize, 
							elemWidth, 
							elemHeight );

		nCol++;
		if (nCol > elemsPerCol)
		{
			nCol = 0;
			nRow++;
		}

		iter++;
	}
}

void CGUIWidgetModel::CalculatePosition(const int *w, const int *h)
{
	mOptions.width = (w==nullptr) ? mWidth : *w;
	mOptions.height = (h==nullptr) ? mHeight : *h;

	if (mComponentsLayout.get() )
		mComponentsLayout->Calculate(mComponents, mOptions);
}

// function takes mouse position and check if there is any element under cursor
//  it's a logic part
void CGUIWidgetModel::SelectUnderCursor( const int pMouseX, const int pMouseY, const int pButtonKey, const int pModifier )
{
	if (mComponents.size() == 0) return;

	int realY = mHeight - pMouseY;

	// rect pos - MVP * base position
	// mouse pos = Proj * mouse coords

	glm::mat4 projection = glm::ortho( 0.0f, (float) mWidth, 0.0f, (float) mHeight, -1.0f, 1.0f );
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3( (float) mPanX, mPanY, mPanZ));
	glm::mat4 MVP = projection * ViewTranslate;

	glm::vec4 mousePos( (float) pMouseX, (float) realY, 0.0, 1.0 );
	mousePos = projection * mousePos;

	glm::mat4 invMVP = glm::inverse(MVP);

	glm::vec4 worldMousePos = invMVP * mousePos;

	/*
	for (int i=0; i<mNumberOfComponents; ++i)
	{
		comps->Select(worldMousePos.x, worldMousePos.y);
	}
	*/
}

void CGUIWidgetModel::Input(const double pMouseX, const double pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{

	// compute local coords
	glm::mat4 projection = glm::ortho( 0.0f, (float) mWidth, 0.0f, (float) mHeight, -1.0f, 1.0f );
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3( (float) mPanX, (float) mPanY, (float) mPanZ ));
	glm::mat4 MVP = projection * ViewTranslate;

	glm::vec4 mousePos( (float) pMouseX, (float) pMouseY, 0.0, 1.0 );
	mousePos = projection * mousePos;

	glm::mat4 invMVP = glm::inverse(MVP);

	glm::vec4 worldMousePos = invMVP * mousePos;

	// do we need to change focues to a new component under a cursor ?!
	
	const bool mouseDown = (GetKeyState( VK_LBUTTON ) == -127);

	if (mouseDown == false && mComponents.size() > 0)
	{
		bool looseFocus = false;

		if (mFocusedComponent != nullptr)
		{
			looseFocus = mFocusedComponent->IsInside(worldMousePos.x, worldMousePos.y) == false;
		}

		if (mFocusedComponent == nullptr || looseFocus == true)
		{
			for (auto iter=mComponents.begin(); iter!=mComponents.end(); ++iter)
			{
				if ( ((*iter) != mFocusedComponent) && (*iter)->IsInside(worldMousePos.x, worldMousePos.y) == true)
				{
					if (mFocusedComponent != nullptr)
						mFocusedComponent->Input( worldMousePos.x, worldMousePos.y, kFBMouseLeave, pButtonKey, pModifier );

					mFocusedComponent = *iter;
					mFocusedComponent->Input( worldMousePos.x, worldMousePos.y, kFBMouseEnter, pButtonKey, pModifier );
				}
			}
		}
	}


	if (mFocusedComponent)
	{
		mFocusedComponent->Input(worldMousePos.x, worldMousePos.y, pAction, pButtonKey, pModifier);
	}
	else
	{
		switch( pAction )
		{
		case kFBButtonPress:
		
			OnMouseDown(pMouseX, pMouseY, pButtonKey);
			if (mView)
				mView->OnMouseDown(pMouseX, pMouseY, pButtonKey);

			break;
		case kFBButtonRelease:

			OnMouseUp(pMouseX, pMouseY, pButtonKey);
			if (mView)
				mView->OnMouseUp(pMouseX, pMouseY, pButtonKey);
			
			break;

		case kFBMotionNotify:

			OnMouseMotion(pMouseX, pMouseY, pButtonKey);
			if (mView)
				mView->OnMouseMotion(pMouseX, pMouseY, pButtonKey);

			break;
		}
	}
}

const int CGUIWidgetModel::GetNumberOfComponents() const
{
	return mNumberOfComponents;
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
	if (nullptr == _mModel)
	{
		return;
	}

	const int numberOfComponents = _mModel->GetNumberOfComponents();
	for (int i=0; i<numberOfComponents; ++i)
	{
		CGUIWidgetModel *pComp = _mModel->GetComponentPtr(i);

		pComp->Render();
	}

	//
	OnRender();
}

void CGUIWidgetView::OnRender()
{
}