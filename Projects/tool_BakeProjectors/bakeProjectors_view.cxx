
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: bakeProjectors_view.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "bakeProjectors_view.h"
#include "bakeProjectors_viewData.h"

#include "StringUtils.h"

FBClassImplementation( ViewBakeProjectors );

/////////////////////////////////////////////////////

// global state
EBakingState gGlobalState = eBakingStateReady;

void SetBakingState( const EBakingState state )
{
	gGlobalState = state;
}

const EBakingState GetBakingState()
{
	return gGlobalState;
}


/************************************************
 *	Constructor
 ************************************************/

ViewBakeProjectors::ViewBakeProjectors()
	: FBView()
{
	mViewData = new ViewBakeProjectorsData();
	
//	FBPropertyPublish(this, Models, "Models", nullptr, nullptr);
	FBPropertyPublish(this, BackgroungColor, "Background Color", nullptr, nullptr);
	
	FBPropertyPublish(this, SaveImagePerModel, "Save Image Per Model", nullptr, nullptr);
	FBPropertyPublish(this, SaveOnlyProjectors, "Save Only Projectors", nullptr, nullptr);
	FBPropertyPublish(this, ImageAddFrameNumber, "Add Frame Number Suffix", nullptr, nullptr);

	FBPropertyPublish(this, SaveJpeg, "Save Jpeg", nullptr, nullptr);
	FBPropertyPublish(this, JpegQuality, "Jpeg Quality", nullptr, nullptr);

//	Models.SetSingleConnect(false);
//	Models.SetFilter(FBModel::GetInternalClassId() );

	BackgroungColor = FBColorAndAlpha(1.0, 1.0, 1.0, 0.0);

	SaveImagePerModel = false;
	SaveOnlyProjectors = false;
	ImageAddFrameNumber = false;

	SaveJpeg = false;
	JpegQuality = 80;

	mGrabWithFrameNumber = false;
	mGrabImage = false;
	mGrabImageName = "";


}

/************************************************
 *	Constructor
 ************************************************/
void ViewBakeProjectors::FBDestroy()
{
	if (mViewData)
	{
		ViewBakeProjectorsData *pData = (ViewBakeProjectorsData*) mViewData;
		delete pData;
		mViewData = nullptr;
	}
}

/************************************************
 *	Refresh callback
 ************************************************/
void ViewBakeProjectors::Refresh(bool pNow)
{
	FBView::Refresh(pNow);
}

/************************************************
 *	Expose callback
 ************************************************/

void ViewBakeProjectors::ViewExpose()
{
	
	ViewBakeProjectorsData *pData = (ViewBakeProjectorsData*) mViewData;
	if (pData == nullptr)
		return;

	//
	//

	FBRenderer *prender = mSystem.Renderer;
	FBCamera *pcamera = prender->CurrentCamera;

	if (FBIS(pcamera, FBCameraSwitcher))
		pcamera = ( (FBCameraSwitcher*) pcamera)->CurrentCamera;
	FBColorAndAlpha backcolor( BackgroungColor );

	//
	//
	bool saveStatus = false;

	pData->RenderToFramebuffers(mGrabImage, mGrabImageName, SaveJpeg, JpegQuality, mGrabWithFrameNumber, 
		SaveImagePerModel, SaveOnlyProjectors, pcamera, backcolor, saveStatus);


	// render to view
	
	const int x = Region.Position.X[0];
	const int y = Region.Position.Y[0];
	const int w = Region.Position.X[1]-Region.Position.X[0]; 
	const int h = Region.Position.Y[1]-Region.Position.Y[0];

	glViewport(x, y, w, h);
	PrepareOrthoView(1.0, 1.0);

	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	
	int currentRow = pData->GetCurrentRow();
	int numberOfFrameBuffers = pData->GetNumberOfFrameBuffers();

	if (currentRow >= 0 && currentRow < numberOfFrameBuffers 
		&& pData->GetFrameBufferPtr(currentRow) != nullptr)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, pData->GetFrameBufferColorId(currentRow) );
		glColor3d(1.0, 1.0, 1.0);
		//DrawQuad2d();
		drawOrthoQuad2d(w, h);
		glBindTexture(GL_TEXTURE_2D, 0 );
	}

	//
	if (true == saveStatus && eBakingStateWait == GetBakingState() )
	{
		SetBakingState( eBakingStateReady );
	}
}

void ViewBakeProjectors::DoGrabCurrentFrame(bool withFrameNumber, bool showDialog, const char *filename)
{
	mGrabWithFrameNumber = withFrameNumber;
	mGrabImage = false;
	if (filename != nullptr)
	{
		mGrabImage = true;
		mGrabImageName = filename;
	}

	if (showDialog)
	{
		FBFilePopup		lDialog;

		lDialog.Style = kFBFilePopupSave;
		lDialog.Filter = "*.*";

		if (lDialog.Execute() )
		{
			mGrabImage = true;
			mGrabImageName = lDialog.FullFilename;
		}
	}
}

void ViewBakeProjectors::ClearModels()
{
	ViewBakeProjectorsData *pData = (ViewBakeProjectorsData*) mViewData;
	if (pData == nullptr)
		return;

	pData->ClearModels();
}

void ViewBakeProjectors::AddModel(FBModel *pModel, int _preset, int _width, int _height)
{
	ViewBakeProjectorsData *pData = (ViewBakeProjectorsData*) mViewData;
	if (pData == nullptr)
		return;

	pData->AddModel(pModel, _preset, _width, _height);
}


void ViewBakeProjectors::SetCurrentRow(const int row)
{
	ViewBakeProjectorsData *pData = (ViewBakeProjectorsData*) mViewData;
	if (pData == nullptr)
		return;

	pData->SetCurrentRow(row);
}

const int ViewBakeProjectors::GetCurrentRow()
{
	ViewBakeProjectorsData *pData = (ViewBakeProjectorsData*) mViewData;
	if (pData == nullptr)
		return -1;

	return pData->GetCurrentRow();
}