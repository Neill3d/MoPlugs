
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: textures_view.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "textures_view.h"

#include <glm\glm\mat4x4.hpp>
#include <glm\glm\gtc\matrix_transform.hpp>
#include <glm\glm\gtc\type_ptr.hpp>

#include "utils\CheckGLError_MOBU.h"

// STL
#include <algorithm>

FBClassImplementation( ORTexturesView3D );



/////////////////////////////////////////////////////////////////////////////////////
// CGUITexturesModel

//! a constructor
WidgetTexturesModel::WidgetTexturesModel()
	: CGUIWidgetModel()
{
	mTexturesManager = nullptr;
	mMenu = nullptr;
	// TODO: register event on change ?!
}

void WidgetTexturesModel::AssignTexturesManager( CGPUTexturesManager *manager )
{
	mTexturesManager = manager;
}

void WidgetTexturesModel::AssignContextMenu(FBGenericMenu *pMenu)
{
	mMenu = pMenu;
}

const int WidgetTexturesModel::GetNumberOfTextures()
{
	return mNumberOfComponents;
}

const double WidgetTexturesModel::GetPan()
{
	return mPan;
}

CGUILayout *WidgetTexturesModel::CreateLayout()
{
	return new CGUILayoutHGrid();

	FBPythonContext
}

void WidgetTexturesModel::OnMouseUp(const int pMouseX, const int pMouseY, const int pButtonKey, const bool hasMotion)
{
	if ( (hasMotion == false) && (pButtonKey==3) && (mMenu != nullptr) )
	{
		POINT p;
		if (GetCursorPos(&p))
		{			
			mMenu->Execute( p.x, p.y );
		}
	}
}

bool WidgetTexturesModel::UpdateFromTexturesManager()
{
	if (mTexturesManager == nullptr)
		return false;

	const int numCachedTextures = mTexturesManager->GetNumberOfCachedTextures();

	mComponents.resize(numCachedTextures);

	double width, height;
	for (int i=0; i<numCachedTextures; ++i)
	{
		WidgetTexture *pTextureWidget = new WidgetTexture();

		int lw, lh;
		mTexturesManager->GetTextureSize(i, lw, lh);
		double aspect = 1.0;
				
		width = 1.0;
		height = 1.0;

		if (lh > 0 && lw > 0)
		{
			aspect = aspect * width / height;

			if (width > height)
			{
				height = (int) (width / aspect);
			}
			else
			{
				width = (int) (aspect * height);
			}
		}

		pTextureWidget->Set(0.0, 0.0, width, height, mTexturesManager->GetTextureId(i) );
		mComponents.push_back(pTextureWidget);
	}
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////
// CGUITexturesView

//! a constructor
CGUITexturesView::CGUITexturesView(CGUITexturesModel *pModel)
	: CGUIWidgetView(pModel)
{
	mWidth = 1;
	mHeight = 1;

	mBuffer = 0;
	mBufferRows = 0;
}


void CGUITexturesView::AssignTexturesManager( CGPUTexturesManager *pManager )
{
	if (mModel)
		( (CGUITexturesModel*) mModel)->AssignTexturesManager( pManager );
}

void CGUITexturesView::OnReSize(const int w, const int h)
{
	mWidth = w;
	mHeight = h;
}

void CGUITexturesView::OnMouseMotion(const int pMouseX, const int pMouseY, const int pButtonKey)
{
	ORView3DEffect::instance().SetMousePosition( (float)pMouseX, (float)(mHeight - pMouseY) );
}


void CGUITexturesView::InitializeDrawBuffer()
{
	if (mBuffer == 0)
		glGenBuffers( 1, &mBuffer );

	if (mBufferData.size() > 0)
	{
		glBindBuffer( GL_ARRAY_BUFFER, mBuffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(BufferPoint) * mBufferData.size(), mBufferData.data(), GL_STATIC_DRAW );
	}

	// background rows buffer
	if (mBufferRows == 0)
		glGenBuffers( 1, &mBufferRows );

	if (mBufferRowData.size() > 0)
	{
		glBindBuffer( GL_ARRAY_BUFFER, mBufferRows );
		glBufferData( GL_ARRAY_BUFFER, sizeof(RowPoint) * mBufferRowData.size(), mBufferRowData.data(), GL_STATIC_DRAW );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void CGUITexturesView::FreeDrawBuffer()
{
	if (mBuffer)
	{
		glDeleteBuffers( 1, &mBuffer );
		mBuffer = 0;
	}
	if (mBufferRows)
	{
		glDeleteBuffers( 1, &mBufferRows );
		mBufferRows = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	Constructor
 ************************************************/
ORTexturesView3D::ORTexturesView3D()
	: mView( &mModel )
{
}

/************************************************
 *	Constructor
 ************************************************/
void ORTexturesView3D::FBDestroy()
{
	mView.Free();
}

/************************************************
 *	Refresh callback
 ************************************************/
void ORTexturesView3D::Refresh(bool pNow)
{
	FBView::Refresh(pNow);
}

void ORTexturesView3D::Resize(const int w, const int h)
{
	mModel.ReSize(w, h);
}

/************************************************
 *	Expose callback
 ************************************************/
void ORTexturesView3D::ViewExpose()
{
	mView.Render();
	/*
	if (mNeedUpdate)
	{
		int lViewport[4];
		glGetIntegerv( GL_VIEWPORT, lViewport );

		//mWidth = lViewport[2];
		//mHeight = lViewport[3];
	
		mWidth = Region.Position.X[1]-Region.Position.X[0];
		mHeight = Region.Position.Y[1]-Region.Position.Y[0];

		CalculateDimentions();

		//
		UpdateBuffers();
		mNeedUpdate = false;
	}
	*/
	// ortho view

	glDisable(GL_DEPTH_TEST);

	//UpdateTextureHandles();
	/*
	if (mTexturesManager)
	{
		printf( "lock textures\n" );
		mTexturesManager->LockTextures();
		//DrawTextures();
		printf( "unlock textures\n" );
		mTexturesManager->UnLockTextures();
	}
	*/
	glEnable(GL_DEPTH_TEST);


	CHECK_GL_ERROR_MOBU();
}


void ORTexturesView3D::UpdateBuffers()
{
	if (itemsPerRow == 0 || mTexturesManager == nullptr)
	{
		return;
	}

	printf( "update buffers - %d\n", itemsPerRow );
	
	if (mBuffer == 0)
		glGenBuffers(1, &mBuffer);

	glBindBuffer( GL_ARRAY_BUFFER, mBuffer );

	float x=spaceSize + itemSize * 0.5;
	float y=spaceSize * 0.5 + itemSize * 0.5;

	const int numCachedTextures = mTexturesManager->GetNumberOfCachedTextures();

	if (numCachedTextures > 0)
	{
		mBufferData.resize(numCachedTextures);

		int index = 0;
		while( index < numCachedTextures )
		{
			for (int j=0; j<itemsPerRow; ++j)
			{
				if (index == numCachedTextures) break;

				mBufferData[index].x = x;
				mBufferData[index].y = mHeight - y;

				
				mBufferData[index].layer = (float) mTexturesManager->GetTextureId(index);

				y += itemSize + spaceSize;

				index++;
			}

			x += itemSize + spaceSize;
			y = spaceSize * 0.5 + itemSize * 0.5;
		}

		glBufferData( GL_ARRAY_BUFFER, sizeof(BufferPoint) * numCachedTextures, mBufferData.data(), GL_STATIC_DRAW );
	}
	else
	{
		FBScene *pScene = mSystem.Scene;
	
		const int numTextures = pScene->Textures.GetCount();
		textures_map &cacheTextures = mTexturesManager->GetTexturesMap();

		mBufferData.resize(numTextures);

		int index = 0;
		while( index < numTextures )
		{
			for (int j=0; j<itemsPerRow; ++j)
			{
				if (index == numTextures) break;

				mBufferData[index].x = x;
				mBufferData[index].y = mHeight - y;

				GLuint id = 0;
				if (index < numTextures)
					id = pScene->Textures[index]->GetTextureObject();
			
				textures_map::iterator iter = cacheTextures.find(id);
				int layerId = (iter!=cacheTextures.end()) ? iter->second.layer : -1;
				//layerId = -1;
				mBufferData[index].layer = (float) layerId;

				y += itemSize + spaceSize;

				index++;
			}

			x += itemSize + spaceSize;
			y = spaceSize * 0.5 + itemSize * 0.5;
		}

		glBufferData( GL_ARRAY_BUFFER, sizeof(BufferPoint) * numTextures, mBufferData.data(), GL_STATIC_DRAW );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	//
	// update background rows buffer
	//
	
	if (mBufferRows == 0) glGenBuffers(1, &mBufferRows);

	glBindBuffer( GL_ARRAY_BUFFER, mBufferRows );

	//float x=spaceSize + itemSize * 0.5;
	y=spaceSize * 0.5 + itemSize * 0.5;
	float yOffset = itemSize + spaceSize;

	mBufferRowData.resize(itemsPerRow);

		for (int j=0; j<itemsPerRow; ++j)
		{
			mBufferRowData[j].x1 = 0;
			mBufferRowData[j].y1 = mHeight - (y + j * yOffset - itemSize * 0.5);
			mBufferRowData[j].x2 = mWidth;
			mBufferRowData[j].y2 = mHeight - (y + j * yOffset + itemSize * 0.5);
		}

	glBufferData( GL_ARRAY_BUFFER, sizeof(RowPoint) * itemsPerRow, mBufferRowData.data(), GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	ORView3DEffect::instance().SetQuadSize( (float) itemSize );

	CHECK_GL_ERROR_MOBU();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void CGUITexturesView::OnRender()
{
	if (mModel == nullptr)
		return;

	CGUITexturesModel *pTexModel = dynamic_cast<CGUITexturesModel*> (mModel);
	if (pTexModel == nullptr)
		return;

	const int numTextures = pTexModel->GetNumberOfTextures();
	bool fromScene = false;
	
	if (numTextures == 0)
		return;

	//
	
	glViewport(0, 0, mWidth, mHeight);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, mWidth, 0.0, mHeight, -1.0, 1.0); 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	
	//
	const int numberOfRows = pTexModel->GetNumberOfRows();

	ORView3DEffect &effect = ORView3DEffect::instance();

	effect.SetHeight( (float)mHeight );
	effect.SetPan( (float) pTexModel->GetPan() );

	glm::mat4 projection = glm::ortho( 0.0f, (float) mWidth, 0.0f, (float) mHeight, -1.0f, 1.0f );
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3( (float) pTexModel->GetPan(), 0.0f, 0.0f));
	glm::mat4 MVP = projection * ViewTranslate;

	effect.SetProjectionMatrix( glm::value_ptr(projection) );
	effect.SetMVP( glm::value_ptr(MVP) );
	
	// draw background lines
	printf( "draw background\n" );
	if ( numberOfRows > 0 )
	{
		glBindBuffer( GL_ARRAY_BUFFER, mBufferRows );
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(RowPoint), (const GLvoid*) 0 );  // point coord

		glEnableVertexAttribArray(0);

		effect.BindBackground();

		glDrawArrays( GL_POINTS, 0, numberOfRows );

		effect.UnBind();
	}

	glBindBuffer( GL_ARRAY_BUFFER, mBuffer );
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(BufferPoint), (const GLvoid*) 0 );  // point coord
	
	printf( "draw main\n" );
	effect.Bind();
	
	if (mTexturesManager)
		mTexturesManager->BindTexturesAsUniform( effect.GetTexturesLocation() );

	glDrawArrays( GL_POINTS, 0, numTextures );
	
	effect.UnBind();

	glDisableVertexAttribArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	//
	// Draw titles if needed

	printf( "draw titles\n" );

	// DONE: what to do with the titles
	
	if (showTitles && (numberOfRows > 0))
	{
		const double halfSize = 0.5 * itemSize;
		const double yOffset = halfSize + 0.8 * spaceSize;

		const int NameMaxLen = 12;

		glTranslated(pTexModel->GetPan(), 0.0, 0.0);

		for (int i=0; i<numTextures; ++i)
		{
			FBString TextureName;
			if (fromScene) 
				TextureName = pScene->Textures[i]->Name;
			else
				TextureName = mTexturesManager->GetTextureIdName(i);
			
			if (TextureName.GetLen() > NameMaxLen)
				TextureName = TextureName.Left(NameMaxLen) + "...";

			DrawString( TextureName, mBufferData[i].x - halfSize, mBufferData[i].y - yOffset );
		}
	}
	else
	{
		DrawString( "Empty", 10.0f, mHeight - 10.0f );
	}
}

void ORTexturesView3D::DrawTextures()
{
	int numTextures=0;
	bool fromScene = false;
	FBScene *pScene = mSystem.Scene;

	if (mTexturesManager && mTexturesManager->GetNumberOfCachedTextures() > 0)
	{
		numTextures = mTexturesManager->GetNumberOfCachedTextures();
	}
	else
	{	
		numTextures =  pScene->Textures.GetCount();
		fromScene = true;
	}

	//
	
	glViewport(0, 0, mWidth, mHeight);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, mWidth, 0.0, mHeight, -1.0, 1.0); 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	
	//

	ORView3DEffect &effect = ORView3DEffect::instance();

	effect.SetHeight( (float)mHeight );
	effect.SetPan( (float) pan );

	glm::mat4 projection = glm::ortho( 0.0f, (float) mWidth, 0.0f, (float) mHeight, -1.0f, 1.0f );
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3( (float) pan, 0.0f, 0.0f));
	glm::mat4 MVP = projection * ViewTranslate;

	effect.SetProjectionMatrix( glm::value_ptr(projection) );
	effect.SetMVP( glm::value_ptr(MVP) );
	
	// draw background lines
	printf( "draw background\n" );
	if ( itemsPerRow > 0 )
	{
		glBindBuffer( GL_ARRAY_BUFFER, mBufferRows );
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(RowPoint), (const GLvoid*) 0 );  // point coord

		glEnableVertexAttribArray(0);

		effect.BindBackground();

		glDrawArrays( GL_POINTS, 0, itemsPerRow );

		effect.UnBind();
	}

	glBindBuffer( GL_ARRAY_BUFFER, mBuffer );
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(BufferPoint), (const GLvoid*) 0 );  // point coord
	
	printf( "draw main\n" );
	effect.Bind();
	
	if (mTexturesManager)
		mTexturesManager->BindTexturesAsUniform( effect.GetTexturesLocation() );

	glDrawArrays( GL_POINTS, 0, numTextures );
	
	effect.UnBind();

	glDisableVertexAttribArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	//
	// Draw titles if needed

	printf( "draw titles\n" );

	// DONE: what to do with the titles
	
	if (showTitles && (itemsPerRow > 0))
	{
		const double halfSize = 0.5 * itemSize;
		const double yOffset = halfSize + 0.8 * spaceSize;

		const int NameMaxLen = 12;

		glTranslated(pan, 0.0, 0.0);

		for (int i=0; i<numTextures; ++i)
		{
			FBString TextureName;
			if (fromScene) 
				TextureName = pScene->Textures[i]->Name;
			else
				TextureName = mTexturesManager->GetTextureIdName(i);
			
			if (TextureName.GetLen() > NameMaxLen)
				TextureName = TextureName.Left(NameMaxLen) + "...";

			DrawString( TextureName, mBufferData[i].x - halfSize, mBufferData[i].y - yOffset );
		}
	}
	else
	{
		DrawString( "Empty", 10.0f, mHeight - 10.0f );
	}
}

void ORTexturesView3D::ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{
	mModel.Input(pMouseX, pMouseY, pAction, pButtonKey, pModifier);	
}

void ORTexturesView3D::SelectUnderCursor( const int pMouseX, const int pMouseY, const int pButtonKey, const int pModifier )
{
	if (mTexturesManager == nullptr) 
	{
		return;
	}

	int realY = mHeight - pMouseY;

	const int numTextures = mTexturesManager->GetNumberOfTextureHandles();

	// rect pos - MVP * base position
	// mouse pos = Proj * mouse coords

	glm::mat4 projection = glm::ortho( 0.0f, (float) mWidth, 0.0f, (float) mHeight, -1.0f, 1.0f );
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3( (float) pan, 0.0f, 0.0f));
	glm::mat4 MVP = projection * ViewTranslate;

	glm::vec4 mousePos( (float) pMouseX, (float) realY, 0.0, 1.0 );
	mousePos = projection * mousePos;

	const float ls = 0.5f * (float) itemSize;

	for (int i=0; i<numTextures; ++i)
		mBufferData[i].flag = 0.0f;

	for (int i=0; i<numTextures; ++i)
	{
		glm::vec4 pos1 ( mBufferData[i].x-ls, mBufferData[i].y-ls, 0.0, 1.0 );
		glm::vec4 pos2 ( mBufferData[i].x+ls, mBufferData[i].y+ls, 0.0, 1.0 );
		pos1 = MVP * pos1;
		pos2 = MVP * pos2;

		// TODO: clip by viewport size [0; width]

		if ( mousePos.x > pos1.x && mousePos.x < pos2.x && mousePos.y > pos1.y && mousePos.y < pos2.y )
		{
			mBufferData[i].flag = 1.0f;
			break;
		}
	}
}