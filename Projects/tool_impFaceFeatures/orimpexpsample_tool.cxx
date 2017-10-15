
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orimpexpsample_tool.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////



/*

	-op <location of output pose file>, the file format is as follows: frame_number confidence detection_success X Y Z Rx Ry Rz
	-of <location of output landmark points file>, the file format is as follows: frame_number detection_success x_1 x_2 ... x_n y_1 y_2 ... y_n
	-of3D <location of output 3D landmark points file>, the file format is as follows: frame_number detection_success X_1 X_2 ... X_n Y_1 Y_2 ... Y_n Z_1 Z_2 ... Z_n

*/


//--- Class declaration
#include "orimpexpsample_tool.h"

#include <stdlib.h>

#include <string>
#include <iostream>
#include <fstream>

#include "TextUtils.h"
#include "StringUtils.h"

//-- xml
#include "IO\tinyxml.h"

#include <vector>
#include <string>
#include <algorithm>

//--- Registration defines
#define	ORTOOLSAMPLE__CLASS		ORTOOLSAMPLE__CLASSNAME
#define	ORTOOLSAMPLE__LABEL		"Import Face Features Tool"
#define	ORTOOLSAMPLE__DESC		"Import Face Features Tool"

//--- FiLMBOX implementation and registration
FBToolImplementation(	ORTOOLSAMPLE__CLASS	);
FBRegisterTool		(	ORTOOLSAMPLE__CLASS,
						ORTOOLSAMPLE__LABEL,
						ORTOOLSAMPLE__DESC,
						FB_DEFAULT_SDK_ICON	);	// Icon filename (default=Open Reality icon)


/************************************************
 *	FiLMBOX Creation Function.
 ************************************************/
bool ORToolSample::FBCreate()
{
	
	StartSize[0] = 440;
	StartSize[1] = 520;

	MinSize[0] = 200;
	MinSize[1] = 200;

	// UI Create & Configure
	UICreate	();
	UIConfigure	();

	
	return true;
}


/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void ORToolSample::FBDestroy()
{
	
}


/************************************************
 *	Create the UI.
 ************************************************/
void ORToolSample::UICreate()
{
	int lW = 150;
	int lS = 5;
	int lH = 25;

	// Add regions
	AddRegion ("LabelCommon",	"LabelCommon",
									lS,		kFBAttachLeft,		"",						1.0,
									lS,		kFBAttachTop,		"",						1.0,
									lW,		kFBAttachNone,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	AddRegion ("LabelNumberPoints",	"LabelNumberPoints",
									2*lS,	kFBAttachLeft,		"",						1.0,
									2*lS,	kFBAttachBottom,	"LabelCommon",			1.0,
									80,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	AddRegion ("EditNumberPoints",	"EditNumberPoints",
									lS,		kFBAttachRight,		"LabelNumberPoints",	1.0,
									2*lS,	kFBAttachBottom,	"LabelCommon",			1.0,
									100,	kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	AddRegion ("LabelNumberRate",	"LabelNumberRate",
									2*lS,	kFBAttachRight,		"EditNumberPoints",		1.0,
									0,		kFBAttachTop,		"EditNumberPoints",	1.0,
									60,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	AddRegion ("EditNumberRate",	"EditNumberRate",
									lS,		kFBAttachRight,		"LabelNumberRate",		1.0,
									0,		kFBAttachTop,		"EditNumberPoints",		1.0,
									100,	kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	//
	AddRegion ("LabelWidth",	"LabelWidth",
									2*lS,	kFBAttachLeft,		"",						1.0,
									2*lS,	kFBAttachBottom,	"LabelNumberRate",		1.0,
									60,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	AddRegion ("EditWidth",	"EditWidth",
									lS,		kFBAttachRight,		"LabelWidth",			1.0,
									2*lS,	kFBAttachBottom,	"LabelNumberRate",		1.0,
									100,	kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	AddRegion ("LabelHeight",	"LabelHeight",
									2*lS,	kFBAttachRight,		"EditWidth",			1.0,
									2*lS,	kFBAttachBottom,	"LabelNumberRate",		1.0,
									60,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	AddRegion ("EditHeight",	"EditHeight",
									lS,		kFBAttachRight,		"LabelHeight",			1.0,
									2*lS,	kFBAttachBottom,	"LabelNumberRate",		1.0,
									100,	kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	AddRegion ("ButtonInvHeight",	"ButtonInvHeight",
									5*lS,	kFBAttachLeft,		"",						1.0,
									2*lS,	kFBAttachBottom,	"LabelWidth",			1.0,
									100,	kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	AddRegion ("ButtonImportOptical",	"ButtonImportOptical",
									5*lS,	kFBAttachLeft,		"",						1.0,
									lS,		kFBAttachBottom,	"ButtonInvHeight",		1.0,
									200,	kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	
	AddRegion ("ButtonImportUnderTheRoot",	"ButtonImportUnderTheRoot",
									5*lS,	kFBAttachLeft,		"",						1.0,
									lS,		kFBAttachBottom,	"ButtonImportOptical",	1.0,
									200,	kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	AddRegion ("ButtonPointsJawOnly",	"ButtonPointsJawOnly",
									5*lS,	kFBAttachLeft,		"",						1.0,
									lS,		kFBAttachBottom,	"ButtonImportUnderTheRoot",			1.0,
									lW,		kFBAttachNone,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	AddRegion ("ButtonPointsEmptyJaw",	"ButtonPointsEmptyJaw",
									5*lS,	kFBAttachLeft,		"",						1.0,
									lS,		kFBAttachBottom,	"ButtonPointsJawOnly",			1.0,
									lW,		kFBAttachNone,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	AddRegion ("ButtonSwapSize",	"ButtonSwapSize",
									5*lS,	kFBAttachLeft,		"",						1.0,
									lS,		kFBAttachBottom,	"ButtonPointsEmptyJaw",			1.0,
									lW,		kFBAttachNone,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );


	//

	AddRegion ("ButtonFaceEnable",	"ButtonFaceEnable",
									lS,		kFBAttachLeft,		"",						1.0,
									3*lS,	kFBAttachBottom,	"ButtonSwapSize",		1.0,
									lW,		kFBAttachNone,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	AddRegion ("EditFaceFile",	"EditFaceFile",
									lS,		kFBAttachRight,		"ButtonFaceEnable",		1.0,
									0,		kFBAttachTop,		"ButtonFaceEnable",		1.0,
									-35,	kFBAttachRight,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );
	AddRegion ("ButtonFaceFile",	"ButtonFaceFile",
									lS,		kFBAttachRight,		"EditFaceFile",			1.0,
									0,		kFBAttachTop,		"ButtonFaceEnable",		1.0,
									-lS,	kFBAttachRight,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	//

	AddRegion ("ButtonPointsEnable",	"ButtonPointsEnable",
									lS,		kFBAttachLeft,		"",						1.0,
									2*lS,	kFBAttachBottom,	"EditFaceFile",			1.0,
									lW,		kFBAttachNone,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	AddRegion ("EditPointsFile",	"EditPointsFile",
									lS,		kFBAttachRight,		"ButtonPointsEnable",	1.0,
									0,		kFBAttachTop,		"ButtonPointsEnable",	1.0,
									-35,	kFBAttachRight,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );
	AddRegion ("ButtonPointsFile",	"ButtonPointsFile",
									lS,		kFBAttachRight,		"EditPointsFile",		1.0,
									0,		kFBAttachTop,		"ButtonPointsEnable",	1.0,
									-lS,	kFBAttachRight,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	//

	AddRegion ("ButtonPointsEnable3",	"ButtonPointsEnable3",
									lS,		kFBAttachLeft,		"",						1.0,
									2*lS,	kFBAttachBottom,	"EditPointsFile",		1.0,
									lW,		kFBAttachNone,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	AddRegion ("EditPointsFile3",	"EditPointsFile3",
									lS,		kFBAttachRight,		"ButtonPointsEnable3",	1.0,
									0,		kFBAttachTop,		"ButtonPointsEnable3",	1.0,
									-35,	kFBAttachRight,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );
	AddRegion ("ButtonPointsFile3",	"ButtonPointsFile3",
									lS,		kFBAttachRight,		"EditPointsFile3",		1.0,
									0,		kFBAttachTop,		"ButtonPointsEnable3",	1.0,
									-lS,	kFBAttachRight,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	//

	AddRegion ("ButtonShapeEnable",	"ButtonShapeEnable",
									lS,		kFBAttachLeft,		"",						1.0,
									2*lS,	kFBAttachBottom,	"EditPointsFile3",		1.0,
									lW,		kFBAttachNone,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	AddRegion ("EditShapePath",	"EditShapePath",
									lS,		kFBAttachRight,		"ButtonShapeEnable",	1.0,
									0,		kFBAttachTop,		"ButtonShapeEnable",	1.0,
									-35,	kFBAttachRight,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );
	AddRegion ("ButtonShapeBrowse",	"ButtonShapeBrowse",
									lS,		kFBAttachRight,		"EditShapePath",		1.0,
									0,		kFBAttachTop,		"ButtonShapeEnable",	1.0,
									-lS,	kFBAttachRight,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	// XML landmarks

	AddRegion ("ButtonLandmarks",	"ButtonLandmarks",
									lS,		kFBAttachLeft,		"",						1.0,
									2*lS,	kFBAttachBottom,	"EditShapePath",		1.0,
									lW,		kFBAttachNone,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );

	AddRegion ("EditLandmarksPath",	"EditLandmarksPath",
									lS,		kFBAttachRight,		"ButtonLandmarks",		1.0,
									0,		kFBAttachTop,		"ButtonLandmarks",		1.0,
									-35,	kFBAttachRight,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );
	AddRegion ("ButtonLandmarksBrowse",	"ButtonLandmarksBrowse",
									lS,		kFBAttachRight,		"EditLandmarksPath",	1.0,
									0,		kFBAttachTop,		"ButtonLandmarks",		1.0,
									-lS,	kFBAttachRight,		NULL,					1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );


    //

	AddRegion ("ButtonAbout",		"ButtonAbout",
									-0.5*lW,		kFBAttachRight,		"",					1.0,
									4*lS,	kFBAttachBottom,	"EditLandmarksPath",	1.0,
									-lS,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,	"",							1.0 );

	AddRegion ("ButtonPostFiltering",		"ButtonPostFiltering",
									-0.5*lW,		kFBAttachLeft,		"ButtonAbout",					1.0,
									4*lS,	kFBAttachBottom,	"EditLandmarksPath",	1.0,
									-lS,	kFBAttachLeft,		"ButtonAbout",						1.0,
									lH,		kFBAttachNone,	"",							1.0 );

    AddRegion ("ButtonImport",		"ButtonImport",
									lS,		kFBAttachLeft,		"",						1.0,
									4*lS,	kFBAttachBottom,	"EditLandmarksPath",	1.0,
									0.5*lW,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	AddRegion ("ButtonBatchImport",		"ButtonBatchImport",
									lS,		kFBAttachRight,		"ButtonImport",			1.0,
									4*lS,	kFBAttachBottom,	"EditLandmarksPath",	1.0,
									-lS,	kFBAttachLeft,		"ButtonPostFiltering",			1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	AddRegion ("ButtonImportOpenFace",		"ButtonImportOpenFace",
									lS,		kFBAttachLeft,		"",						1.0,
									4*lS,	kFBAttachBottom,	"ButtonImport",	1.0,
									0.5*lW,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	// Assign regions
	SetControl( "LabelCommon", mLabelCommon );
	SetControl( "LabelNumberPoints",mLabelNumberPoints	);
	SetControl( "EditNumberPoints",	mEditNumberPoints	);
	SetControl( "LabelNumberRate",	mLabelNumberRate );
	SetControl( "EditNumberRate",	mEditNumberRate	);
	
	SetControl( "LabelWidth",	mLabelWidth );
	SetControl( "EditWidth",	mEditWidth );
	SetControl( "LabelHeight",	mLabelHeight );
	SetControl( "EditHeight",	mEditHeight );
	SetControl( "ButtonInvHeight",	mButtonInvHeight );
	SetControl( "ButtonImportOptical",	mButtonImportAsOptical );
	SetControl( "ButtonImportUnderTheRoot",	mButtonImportUnderTheRoot );
	SetControl( "ButtonPointsJawOnly",mButtonPointsJawOnly );
	SetControl( "ButtonPointsEmptyJaw",mButtonPointsEmptyJaw );
	SetControl( "ButtonSwapSize", mButtonSwapSize );

	SetControl( "ButtonFaceEnable",	mButtonFaceEnable );
	SetControl( "EditFaceFile",		mEditFaceFile	);
	SetControl( "ButtonFaceFile",	mButtonFaceBrowse );

	SetControl( "ButtonPointsEnable",mButtonPointsEnable );
	SetControl( "EditPointsFile",	mEditPointsFile	);
	SetControl( "ButtonPointsFile",	mButtonPointsBrowse	);

	SetControl( "ButtonPointsEnable3",mButtonPointsEnable3 );
	SetControl( "EditPointsFile3",	mEditPointsFile3	);
	SetControl( "ButtonPointsFile3", mButtonPointsBrowse3	);

	SetControl( "ButtonShapeEnable", mButtonShapeEnable );
	SetControl( "EditShapePath",	mEditShapePath	);
	SetControl( "ButtonShapeBrowse", mButtonShapeBrowse	);

	SetControl( "ButtonLandmarks", mButtonLandmarks );
	SetControl( "EditLandmarksPath",	mEditLandmarksPath	);
	SetControl( "ButtonLandmarksBrowse", mButtonLandmarksBrowse	);
	
	SetControl( "ButtonAbout",	mButtonAbout	);
	SetControl( "ButtonImport",	mButtonImport	);
	SetControl( "ButtonPostFiltering",	mButtonPostFiltering );
	SetControl( "ButtonBatchImport",	mButtonBatchImport	);

	SetControl( "ButtonImportOpenFace",	mButtonImportOpenFace2d	);
}


/************************************************
 *	Configure the UI.
 ************************************************/
void ORToolSample::UIConfigure()
{
	mLabelCommon.Caption = "Common Settings:";

	mLabelNumberPoints.Caption = "Number Points: ";
	mEditNumberPoints.Min			= 1.0;
	mEditNumberPoints.Max			= 100.0;
	mEditNumberPoints.Precision	= 1.0;
	mEditNumberPoints.SmallStep	= 1.0;
	mEditNumberPoints.LargeStep	= 1.0;
	mEditNumberPoints.Value		= 68.0;

	mLabelNumberRate.Caption = "Frame Rate: ";
	mEditNumberRate.Min			= 1.0;
	mEditNumberRate.Max			= 100.0;
	mEditNumberRate.Precision	= 3.2;
	mEditNumberRate.SmallStep	= 0.1;
	mEditNumberRate.LargeStep	= 1.0;
	mEditNumberRate.Value		= 30.0;

	//
	mLabelWidth.Caption = "Width: ";
	mEditWidth.Min			= 100.0;
	mEditWidth.Max			= 4096.0;
	mEditWidth.Precision	= 1.0;
	mEditWidth.SmallStep	= 1.0;
	mEditWidth.LargeStep	= 1.0;
	mEditWidth.Value		= 812.0;

	mLabelHeight.Caption = "Height: ";
	mEditHeight.Min			= 100.0;
	mEditHeight.Max			= 4096.0;
	mEditHeight.Precision	= 1.0;
	mEditHeight.SmallStep	= 1.0;
	mEditHeight.LargeStep	= 1.0;
	mEditHeight.Value		= 1080.0;

	mButtonInvHeight.Caption = "Inverted Height";
	mButtonInvHeight.Style = kFBCheckbox;
	mButtonInvHeight.State = 1;

	mButtonImportAsOptical.Caption = "Import As Optical Data";
	mButtonImportAsOptical.Style = kFBCheckbox;
	mButtonImportAsOptical.State = 0;

	mButtonImportUnderTheRoot.Caption = "Import Under The Root";
	mButtonImportUnderTheRoot.Style = kFBCheckbox;
	mButtonImportUnderTheRoot.State = 0;

	mButtonPointsJawOnly.State = 0;
	mButtonPointsJawOnly.Style = kFBCheckbox;
	mButtonPointsJawOnly.Caption = "Import CLM Jaw Points Only";

	mButtonPointsEmptyJaw.State = 1;
	mButtonPointsEmptyJaw.Style = kFBCheckbox;
	mButtonPointsEmptyJaw.Caption = "Import Empty Jaw Points";


	mButtonSwapSize.State = 0;
	mButtonSwapSize.Style = kFBCheckbox;
	mButtonSwapSize.Caption = "Swap size on landmarks import";

	//

	mButtonFaceEnable.State = 0;
	mButtonFaceEnable.Style = kFBCheckbox;
	mButtonFaceEnable.Caption = "CLM Face File: ";
	mButtonFaceEnable.Enabled = false;
	mEditFaceFile.Text	= "d:\\001_HARE_TEST\\landmarks\\face.txt";
	mEditFaceFile.Enabled = false;
	mButtonFaceBrowse.Caption = "...";

	//

	mButtonPointsEnable.State = 0;
	mButtonPointsEnable.Style = kFBCheckbox;
	mButtonPointsEnable.Caption = "CLM 2d Points File: ";
	mEditPointsFile.Text	= "d:\\001_HARE_TEST\\landmarks\\points.txt";
	mButtonPointsBrowse.Caption = "...";
	
	//

	mButtonPointsEnable3.State = 0;
	mButtonPointsEnable3.Style = kFBCheckbox;
	mButtonPointsEnable3.Caption = "CLM 3d Points File: ";
	mButtonPointsEnable3.Enabled = false;
	mEditPointsFile3.Text	= "d:\\001_HARE_TEST\\landmarks\\points3.txt";
	mEditPointsFile3.Enabled = false;
	mButtonPointsBrowse3.Caption = "...";
	
	//

	mButtonShapeEnable.State = 0;
	mButtonShapeEnable.Style = kFBCheckbox;
	mButtonShapeEnable.Caption = "Shake Rotoshape data: ";
	mButtonShapeEnable.Enabled = false;
	mEditShapePath.Text	= "d:\\001_HARE_TEST2\\results\\Layer 2.ssf";
	mEditShapePath.Enabled = false;
	mButtonShapeBrowse.Caption = "...";
	
	mButtonLandmarks.State = 0;
	mButtonLandmarks.Style = kFBCheckbox;
	mButtonLandmarks.Caption = "FaceWare Landmarks xml data: ";
	mEditLandmarksPath.Text	= "d:\\landmarks.xml";
	mButtonLandmarksBrowse.Caption = "...";

	mButtonImport.Caption	= "Import";
	mButtonImportOpenFace2d.Caption = "Import OpenFace 2D";
	mButtonBatchImport.Caption = "Batch Import...";
	mButtonAbout.Caption	= "About";
	mButtonPostFiltering.Caption = "PostFilter...";

	// Add callbacks
	mButtonFaceBrowse.OnClick.Add	( this,(FBCallback)&ORToolSample::EventButtonFaceBrowseClick );
	mButtonPointsBrowse.OnClick.Add	( this,(FBCallback)&ORToolSample::EventButtonBrowseClick );
	mButtonPointsBrowse3.OnClick.Add	( this,(FBCallback)&ORToolSample::EventButton3BrowseClick );
	mButtonShapeBrowse.OnClick.Add (this, (FBCallback) &ORToolSample::EventButtonShapeBrowseClick );
	mButtonLandmarksBrowse.OnClick.Add (this, (FBCallback) &ORToolSample::EventButtonLandmarksBrowseClick );

	mButtonAbout.OnClick.Add	( this,(FBCallback)&ORToolSample::EventButtonAboutClick );
	mButtonImport.OnClick.Add	( this,(FBCallback)&ORToolSample::EventButtonImportClick );
	mButtonBatchImport.OnClick.Add	( this,(FBCallback)&ORToolSample::EventButtonBatchImportClick );
	mButtonPostFiltering.OnClick.Add	( this,(FBCallback)&ORToolSample::EventButtonPostFilteringClick );

	mButtonImportOpenFace2d.OnClick.Add	( this,(FBCallback)&ORToolSample::EventButtonImportOpenFace2DClick );
}


/************************************************
 *	Browse button callback.
 ************************************************/
void ORToolSample::EventButtonAboutClick(HIRegister pSender, HKEvent pEvent)
{
	FBMessageBox( "Import Face Data",	"MoPLUGS Project 2015\n"
										" Created by Sergey Solohin (Neill3d)\n"
										"  e-mail to: s@neill3d.com\n"
										"   www.neill3d.com",
										"Ok" );
}

void ORToolSample::EventButtonFaceBrowseClick(HIRegister pSender, HKEvent pEvent)
{
	FBFilePopup	ldialog;

	ldialog.Style = kFBFilePopupOpen;
	ldialog.Caption = "Enter face file:";
	ldialog.Filter = "*.txt";

	if (ldialog.Execute() )
	{
		mEditFaceFile.Text = ldialog.FullFilename;
	}

}

void ORToolSample::EventButtonBrowseClick(HIRegister pSender, HKEvent pEvent)
{
	FBFilePopup	ldialog;

	ldialog.Style = kFBFilePopupOpen;
	ldialog.Caption = "Enter points file:";
	ldialog.Filter = "*.txt";

	if (ldialog.Execute() )
	{
		mEditPointsFile.Text = ldialog.FullFilename;
	}

}

void ORToolSample::EventButton3BrowseClick(HIRegister pSender, HKEvent pEvent)
{
	FBFilePopup	ldialog;

	ldialog.Style = kFBFilePopupOpen;
	ldialog.Caption = "Enter 3d points file:";
	ldialog.Filter = "*.txt";

	if (ldialog.Execute() )
	{
		mEditPointsFile3.Text = ldialog.FullFilename;
	}

}

void ORToolSample::EventButtonShapeBrowseClick(HIRegister pSender, HKEvent pEvent)
{
	FBFilePopup	ldialog;

	ldialog.Style = kFBFilePopupOpen;
	ldialog.Caption = "Enter shape data file:";
	ldialog.Filter = "*.ssf";

	if (ldialog.Execute() )
	{
		mEditShapePath.Text = ldialog.FullFilename;
	}
}

void ORToolSample::EventButtonLandmarksBrowseClick(HIRegister pSender, HKEvent pEvent)
{
	FBFilePopup	ldialog;

	ldialog.Style = kFBFilePopupOpen;
	ldialog.Caption = "Enter landmarks data file:";
	ldialog.Filter = "*.xml";

	if (ldialog.Execute() )
	{
		mEditLandmarksPath.Text = ldialog.FullFilename;
	}
}

bool ORToolSample::GetNextValue(const char *line, const int len, const char separator, int &position, double &value)
{
	memset( temp_buffer, 0, sizeof(char)*32 );
	char *currPtr = temp_buffer;

	// skip sepator
	int currPos = position;
	while (currPos < len)
	{
		if (line[currPos] != separator)
		{
			break;
		}

		currPos++;
	}

	
	while (currPos < len)
	{
		if (line[currPos] == separator)
		{
			break;
		}

		*currPtr = line[currPos];
		currPtr++;

		currPos++;
	}

	value = atof(temp_buffer);
	position = currPos+1;
	return true;
}

bool ORToolSample::GetNextValue(const char *line, const int len, const char separator, int &position, int &value)
{
	memset( temp_buffer, 0, sizeof(char)*32 );
	char *currPtr = temp_buffer;

	// skip sepator
	int currPos = position;
	while (currPos < len)
	{
		if (line[currPos] != separator)
		{
			break;
		}

		currPos++;
	}

	
	while (currPos < len)
	{
		if (line[currPos] == separator)
		{
			break;
		}

		*currPtr = line[currPos];
		currPtr++;

		currPos++;
	}

	value = atoi(temp_buffer);
	position = currPos+1;
	return true;
}

void ORToolSample::EventButtonImportClick(HIRegister pSender, HKEvent pEvent)
{

	FBModel *pRoot = nullptr;

	FBModelList	selList;
	FBGetSelectedModels(selList);

	if (mButtonImportUnderTheRoot.State > 0 && selList.GetCount() > 0)
	{
		pRoot = selList.GetAt(0);
		if ( mButtonImportAsOptical.State > 0 && !FBIS(pRoot, FBModelOptical) )
		{
			FBMessageBox( "Import Face Features Tool", "Selected root should be an optical model", "Ok" );
			return;
		}
	}
	else
	{
		if (mButtonImportAsOptical.State > 0)
		{
			pRoot = new FBModelOptical( "FaceFeaturesOptical" );
			pRoot->Show = true;
		}
		else
		{
			pRoot = new FBModelNull( "FaceFeaturesRoot" );
			pRoot->Show = true;
			pRoot->Visibility = true;
		}
	}

	bool asOptical = mButtonImportAsOptical.State > 0;

	if (mButtonLandmarks.State > 0 && mEditLandmarksPath.Text != "")
		ImportLandmarks(pRoot, asOptical, mEditLandmarksPath.Text);

	if (mButtonPointsEmptyJaw.State > 0)
	{
		ImportEmptyJawPoints(pRoot, asOptical);
	}
	else
	if (mButtonPointsEnable.State > 0 && mEditPointsFile.Text != "")
		ImportPoints(pRoot, asOptical, mEditPointsFile.Text);

	/*

	// TODO:

	if (mButtonFaceEnable.State > 0 && mEditFaceFile.Text != "")
		ImportFace(mEditFaceFile.Text);

	if (mButtonPointsEnable3.State > 0 && mEditPointsFile3.Text != "")
		ImportPoints3d(mEditPointsFile3.Text);

	if (mButtonShapeEnable.State > 0 && mEditShapePath.Text != "")
		ImportShapeData(mEditShapePath.Text);
		*/
}

void ORToolSample::EventButtonImportOpenFace2DClick	( HISender pSender, HKEvent pEvent )
{

	FBFilePopup	lDialog;

	lDialog.Caption = "Please choose a 2d landmarks file";
	lDialog.Filter = "*.landmarks_2d";

	if (lDialog.Execute() )
	{
		bool asOptical = mButtonImportAsOptical.State > 0;

		FBModel *pRoot = nullptr;

		if (mButtonImportAsOptical.State > 0)
		{
			pRoot = new FBModelOptical( "OpenFace:Optical" );
			pRoot->Show = true;
		}
		else
		{
			pRoot = new FBModelNull( "OpenFace:Root" );
			pRoot->Show = true;
			pRoot->Visibility = true;
		}

		
		const double width = (mEditWidth.Value > 1.0) ? 100.0 / mEditWidth.Value : 1.0;
		const double height = (mEditHeight.Value > 1.0) ? 100.0 / mEditHeight.Value : 1.0;
		const bool invertedHeight = (mButtonInvHeight.State == 1);
		const bool applyRenameRule = false;
		const bool jawOnly = mButtonPointsJawOnly.State > 0;

		ImportOpenFaceLandmarks2d(pRoot, asOptical, lDialog.FullFilename, width, height, invertedHeight,
			applyRenameRule, jawOnly);
	}

}

void ORToolSample::EventButtonBatchImportClick(HIRegister pSender, HKEvent pEvent)
{
	// TODO: !!!

	FBFolderPopup	lDialog;

	FBString		sourcePath;
	FBString		dstPath;
	// 1 - choose source folder

	lDialog.Caption = "Please choose a source folder";

	if (lDialog.Execute() )
	{
		sourcePath = lDialog.Path;

		// 2 - choose a destination folder

		lDialog.Caption = "Please choose a destination folder";

		if (lDialog.Execute() )
		{

			dstPath = lDialog.Path;

			BatchProcessing( sourcePath, dstPath );

		}

	}

}

bool ORToolSample::ImportFace(const char *filename)
{
	FILE *fp = fopen( filename, "r" );

	if (fp == nullptr)
		return false;

	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);

	if (len == 0)
	{
		fclose(fp);
		return false;
	}

	fseek(fp, 0, SEEK_SET);

	char *data = new char[len+1];
	memset(data, 0, sizeof(char)*(len+1));
	size_t bytesRead = fread_s(data, sizeof(char)*len, sizeof(unsigned char), len, fp);
	
	fclose(fp);

	FBStringList	strings;
	strings.SetString( data, '\n' );

	int count = strings.GetCount();

	//
	if (data)
	{
		delete [] data;
		data = nullptr;
	}

	// import as optical data

	FBModelNull *pRoot = new FBModelNull( "FacePose" );
	pRoot->Show = true;
	pRoot->Visibility = true;

	// import each frame

	double values[6];

	const double pi = 3.14159265359;

	for (int i=0; i<strings.GetCount(); ++i)
	{
		FBTime currTime(0,0,0, i);

		const char *line = strings.GetAt(i);

		// read values

		int pos = 0;
		int len = strlen(line);
		double val = 0.0;

		// skip frame_number, confidence, detection_success
		GetNextValue(line, len, ' ', pos, val);
		GetNextValue(line, len, ' ', pos, val);
		GetNextValue(line, len, ' ', pos, val);

		// read face pose
		GetNextValue(line, len, ' ', pos, values[0]);
		GetNextValue(line, len, ' ', pos, values[1]);
		GetNextValue(line, len, ' ', pos, values[2]);
		GetNextValue(line, len, ' ', pos, values[3]);
		GetNextValue(line, len, ' ', pos, values[4]);
		GetNextValue(line, len, ' ', pos, values[5]);

		// convert rad to deg

		values[3] *= 180.0 / pi;
		values[4] *= 180.0 / pi;
		values[5] *= 180.0 / pi;

		// write values

		FBAnimationNode *pNode = pRoot->Translation.GetAnimationNode();

		if (pNode == nullptr)
		{
			pRoot->Translation.SetAnimated(true);
			pNode = pRoot->Translation.GetAnimationNode();
		}

		if (pNode == nullptr)
			continue;

		pNode->KeyAdd( currTime, &values[0] );

		// write rotation

		pNode = pRoot->Rotation.GetAnimationNode();

		if (pNode == nullptr)
		{
			pRoot->Rotation.SetAnimated(true);
			pNode = pRoot->Rotation.GetAnimationNode();
		}

		if (pNode == nullptr)
			continue;

		pNode->KeyAdd( currTime, &values[3] );
	}

	return true;
}

bool ORToolSample::ImportEmptyJawPoints(FBModel *pRoot, const bool asOptical)
{
	if (asOptical == false || FBIS(pRoot, FBModelOptical) == false)
		return false;

	//
	// write values to elements

	auto fn_getname = [] (const int index) -> const char * {

		switch(index)
		{
		case 7:
			return "JAW_RIGHT";
		case 8:
			return "JAW_MIDDLE";
		case 9:
			return "JAW_LEFT";
		default:
			return "FaceFeature";
		}
	};

	
	//
	FBModelOptical *pOptical = (FBModelOptical*) pRoot;
	
	for (int i=7; i<10; ++i)
	{
		
		FBModelMarkerOptical *pMarker = new FBModelMarkerOptical( fn_getname(i), pOptical );
		pMarker->Show = true;
	}

	return true;
}

bool ORToolSample::ImportPoints(FBModel *pRoot, const bool asOptical, const char *filename)
{
	FILE *fp = fopen( filename, "r" );

	if (fp == nullptr)
		return false;

	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);

	if (len == 0)
	{
		fclose(fp);
		return false;
	}

	fseek(fp, 0, SEEK_SET);

	char *data = new char[len+1];
	memset(data, 0, sizeof(char)*(len+1));
	size_t bytesRead = fread_s(data, sizeof(char)*len, sizeof(unsigned char), len, fp);
	
	fclose(fp);

	FBStringList	strings;
	strings.SetString( data, '\n' );

	int count = strings.GetCount();

	//
	if (data)
	{
		delete [] data;
		data = nullptr;
	}
	
	// All of the models (except CLM-Z) use a 68 point convention for tracking
	const int numberOfPoints = (int) (double) mEditNumberPoints.Value;
	const int numberOfFrames = strings.GetCount();
	if (numberOfPoints <= 0 || numberOfFrames <= 0)
		return false;

	double *values = new double[numberOfFrames * numberOfPoints * 3];

	if (values == nullptr)
		return false;

	const double width = (mEditWidth.Value > 1.0) ? 100.0 / mEditWidth.Value : 1.0;
	const double height = (mEditHeight.Value > 1.0) ? 100.0 / mEditHeight.Value : 1.0;
	const bool invertedHeight = (mButtonInvHeight.State == 1);


	for (int i=0; i<numberOfFrames; ++i)
	{
		double *localValues = &values[i * numberOfPoints * 3];

		const char *line = strings.GetAt(i);

		// read values

		int pos = 0;
		int len = strlen(line);
		double val = 0.0;

		// skip frame index and one more value
		GetNextValue(line, len, ' ', pos, val);
		GetNextValue(line, len, ' ', pos, val);

		// read all x values
		for (int j=0; j<numberOfPoints; ++j)
		{
			val = 0.0;
			GetNextValue(line, len, ' ', pos, val);
			//GetNextValue(strLine, sz, val);

			val = val * width;

			localValues[j*3] = val;
		}

		// read all y values and set z to zero
		for (int j=0; j<numberOfPoints; ++j)
		{
			val = 0.0;
			GetNextValue(line, len, ' ', pos, val );
			//GetNextValue(strLine, sz, val);

			val = val * height;
			
			if (invertedHeight)
				val = 100.0 - val;

			localValues[j*3+1] = val;
			localValues[j*3+2] = 0.0;
		}
	}

	//
	// write values to elements

	auto fn_getname = [] (const int index) -> const char * {

		switch(index)
		{
		case 7:
			return "JAW_RIGHT";
		case 8:
			return "JAW_MIDDLE";
		case 9:
			return "JAW_LEFT";
		default:
			return "FaceFeature";
		}
	};

	if (asOptical)
	{
		//
		FBModelOptical *pOptical = (FBModelOptical*) pRoot;
	
		FBTime	start( 0,0,0, 0 ); // FBPlayerControl().ZoomWindowStart 
		FBTime	stop( 0,0,0, numberOfFrames );
		FBTime	oneFrame( 0,0,0, 1 ); //, 0, FBPlayerControl().GetTransportFps(), FBPlayerControl().GetTransportFpsValue() );

		if (pOptical->Children.GetCount() == 0)
		{
			// set sampling characteristics
			pOptical->SamplingStart = start;
			pOptical->SamplingStop = stop;
			pOptical->SamplingPeriod = oneFrame;
		}

		for (int i=0; i<numberOfPoints; ++i)
		{
			if (mButtonPointsJawOnly.State > 0 && (i<7 || i>9) )
			{
				continue;
			}

			FBModelMarkerOptical *pMarker = new FBModelMarkerOptical( fn_getname(i), pOptical );
			pMarker->Show = true;

			int samples = pMarker->ImportBegin();
			if (samples > numberOfFrames ) 
				samples = numberOfFrames;

			for (int j=0; j<samples; ++j)
			{
				double *localValues = &values[j * numberOfPoints * 3 + i*3];
				pMarker->ImportKey( localValues[0], localValues[1] );
			}

			pMarker->ImportEnd();
		}
	}
	else
	{

		for (int i=0; i<numberOfPoints; ++i)
		{
			if (mButtonPointsJawOnly.State > 0 && (i<7 || i>9) )
			{
				continue;
			}

			FBModelNull *pPoint = new FBModelNull( fn_getname(i) );
			pPoint->Show = true;
			pPoint->Visibility = true;
			pPoint->Parent = pRoot;

			FBAnimationNode *pNode = pPoint->Translation.GetAnimationNode();

			if (pNode == nullptr)
			{
				pPoint->Translation.SetAnimated(true);
				pNode = pPoint->Translation.GetAnimationNode();
			}

			if (pNode == nullptr)
				continue;

			
			for (int j=0; j<numberOfFrames; ++j)
			{
				FBTime currTime(0,0,0, j);	

				// write values

				double *localValues = &values[j * numberOfPoints * 3 + i*3];
				pNode->KeyAdd( currTime, localValues );
			}
		}
	}

	//

	if (values)
	{
		delete [] values;
		values = nullptr;
	}

	return true;
}

bool ORToolSample::ImportPoints3d(const char *filename)
{
	FILE *fp = fopen( filename, "r" );

	if (fp == nullptr)
		return false;

	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);

	if (len == 0)
	{
		fclose(fp);
		return false;
	}

	fseek(fp, 0, SEEK_SET);

	char *data = new char[len+1];
	memset(data, 0, sizeof(char)*(len+1));
	size_t bytesRead = fread_s(data, sizeof(char)*len, sizeof(unsigned char), len, fp);
	
	fclose(fp);

	FBStringList	strings;
	strings.SetString( data, '\n' );

	int count = strings.GetCount();

	//
	if (data)
	{
		delete [] data;
		data = nullptr;
	}

	// import as optical data

	FBModelNull *pRoot = new FBModelNull( "FaceFeaturesRoot3d" );
	pRoot->Show = true;
	pRoot->Visibility = true;

	FBModelList	*pList = FBCreateModelList();
	
	// All of the models (except CLM-Z) use a 68 point convention for tracking
	int numberOfPoints = (int) (double) mEditNumberPoints.Value;
	for (int i=0; i<numberOfPoints; ++i)
	{
		FBModelNull *pPoint = new FBModelNull( "FaceFeature3d" );
		pPoint->Show = true;
		pPoint->Visibility = true;
		pPoint->Parent = pRoot;

		pList->Add(pPoint);
	}

	// import each frame

	double *values = new double[numberOfPoints * 3];

	for (int i=0; i<strings.GetCount(); ++i)
	{
		FBTime currTime(0,0,0, i);

		const char *line = strings.GetAt(i);

		// read values

		int pos = 0;
		int len = strlen(line);
		double val = 0.0;

		// skip frame index and one more value
		GetNextValue(line, len, ' ', pos, val);
		GetNextValue(line, len, ' ', pos, val);

		// read all x values
		for (int j=0; j<numberOfPoints; ++j)
		{
			val = 0.0;
			GetNextValue(line, len, ' ', pos, val);
			values[j*3] = val;
		}

		// read all y values and set z to zero
		for (int j=0; j<numberOfPoints; ++j)
		{
			val = 0.0;
			GetNextValue(line, len, ' ', pos, val );
			values[j*3+1] = val;
		}

		// read all z values
		for (int j=0; j<numberOfPoints; ++j)
		{
			val = 0.0;
			GetNextValue(line, len, ' ', pos, val );
			values[j*3+2] = val;
		}

		// write values

		for (int j=0; j<numberOfPoints; ++j)
		{
			FBModel *pPoint = pList->GetAt(j);
			FBAnimationNode *pNode = pPoint->Translation.GetAnimationNode();

			if (pNode == nullptr)
			{
				pPoint->Translation.SetAnimated(true);
				pNode = pPoint->Translation.GetAnimationNode();
			}

			if (pNode == nullptr)
				continue;

			pNode->KeyAdd( currTime, &values[j * 3] );
		}
	}

	//

	if (values)
	{
		delete [] values;
		values = nullptr;
	}

	FBDestroyModelList(pList);

	return true;
}

bool ORToolSample::ImportShapeData(const char *filename)
{

	std::ifstream input(filename);
	std::string line;

	char szBuffer[128];

	FBModelOptical *pOptical = nullptr;

	int markersCount = 0;
	int framesCount = 0;
	float	*data = nullptr;

	bool firstTime = true;

	int firstFrame = 0;
	int frameIndex = 0;

	try
	{
		while( std::getline(input, line, '\n'))
		{
			CEUtils::StringTokens	tokens(line.c_str());

			if (tokens.GetCount() > 0)
			{
				tokens.GetToken(0, szBuffer, 128);

				if (strstr(szBuffer, "shape_name") != nullptr)
				{
					// start new optical model
					tokens.GetToken(1, szBuffer, 128);

					pOptical = new FBModelOptical(szBuffer);
					pOptical->Show = true;

					/*
					FBTime	start( 0 ); // FBPlayerControl().ZoomWindowStart 
					FBTime	stop( FBPlayerControl().ZoomWindowStop );
					FBTime	oneFrame( 0,0,0,1 ); //, 0, FBPlayerControl().GetTransportFps(), FBPlayerControl().GetTransportFpsValue() );

					// set sampling characteristics
					pOptical->SamplingStart = start;
					pOptical->SamplingStop = stop;
					pOptical->SamplingPeriod = oneFrame;
					*/

					firstTime = true;
				}
				else if (strstr(szBuffer, "num_vertices") != nullptr)
				{
					// number of markers
					tokens.GetToken(1, szBuffer, 128);

					markersCount = atoi(szBuffer);

					// NOTE! 3 points per marker (2 tangents and a point)
					
					for (int i=0; i<markersCount*3; ++i)
					{
						FBModelMarkerOptical *pMarker = new FBModelMarkerOptical( "marker1", pOptical );
						pMarker->Show = true;
					}

				}
				else if (strstr(szBuffer, "num_key_times") != nullptr)
				{
					// allocate arrays for holding all frames data

					tokens.GetToken(1, szBuffer, 128);

					framesCount = atoi(szBuffer);

					// 3 points per marker (2 tangents and 1 center)
					// 2 values per point (x, y)

					data = new float[framesCount * markersCount * 3 * 2];

				}
				else if (strstr(szBuffer, "key_time") != nullptr)
				{
					// enter reading frame value
					tokens.GetToken(1, szBuffer, 128);

					if (firstTime)
					{
				 		firstFrame = atoi(szBuffer);
						frameIndex = 0;
						firstTime = false;
					}
					else
					{
						frameIndex++;
					}
				}
				else if (strstr(szBuffer, "vertex_data") != nullptr)
				{
					// process vertex data
					int count = markersCount * 3;
					
					float *pvalue = &data[frameIndex * markersCount * 3 * 2];

					// NOTE! skip borders
					int indexOffset = 1;

					for (int i=0; i<count; ++i)
					{
						int div = i % 3;
						if ( (i > 0) && (div == 0) )
						{
							indexOffset += 6;
						}

						int index = i*2 + indexOffset;

						// read x
						tokens.GetToken(index, szBuffer, 128);
						*pvalue = (float) atof(szBuffer);
						pvalue++;

						// read y
						tokens.GetToken(index+1, szBuffer, 128);
						*pvalue = (float) atof(szBuffer);
						pvalue++;
					}

				}
			}
		}


		// import markers data
		if (pOptical)
		{
			FBTime	start( 0,0,0, firstFrame ); // FBPlayerControl().ZoomWindowStart 
			FBTime	stop( 0,0,0, firstFrame+framesCount );
			FBTime	oneFrame( 0,0,0,1 ); //, 0, FBPlayerControl().GetTransportFps(), FBPlayerControl().GetTransportFpsValue() );

			// set sampling characteristics
			pOptical->SamplingStart = start;
			pOptical->SamplingStop = stop;
			pOptical->SamplingPeriod = oneFrame;

			for (int i=0; i<markersCount; ++i)
			{
				// left tangent goes first
				int pointIndex = i*3+1;
				FBModelMarkerOptical *pMarker = (FBModelMarkerOptical*) pOptical->Markers[i*3];

				int samples = pMarker->ImportBegin();
				if (samples > framesCount) samples = framesCount;
				
				for (int j=0; j<samples; ++j)
				{
					float *pValue = &data[j * markersCount * 3 * 2];

					pMarker->ImportKey( pValue[pointIndex*2], pValue[pointIndex*2+1] );
				}

				pMarker->ImportEnd();

				// point
				pointIndex = i*3;
				pMarker = (FBModelMarkerOptical*) pOptical->Markers[i*3+1];

				samples = pMarker->ImportBegin();
				if (samples > framesCount) samples = framesCount;
				
				for (int j=0; j<samples; ++j)
				{
					float *pValue = &data[j * markersCount * 3 * 2];

					pMarker->ImportKey( pValue[pointIndex*2], pValue[pointIndex*2+1] );
				}

				pMarker->ImportEnd();

				//
				// right tangent goes last
				pointIndex = i*3+2;
				pMarker = (FBModelMarkerOptical*) pOptical->Markers[i*3+2];

				samples = pMarker->ImportBegin();
				if (samples > framesCount) samples = framesCount;
				
				for (int j=0; j<samples; ++j)
				{
					float *pValue = &data[j * markersCount * 3 * 2];

					pMarker->ImportKey( pValue[pointIndex*2], pValue[pointIndex*2+1] );
				}

				pMarker->ImportEnd();
			}
		}

		

		if (data)
		{
			delete [] data;
			data = nullptr;
		}

	}
	catch(const std::exception &e)
	{
		printf ("error while reading file - %s\n", e.what() );
	}

	return true;
}

struct MetaData
{
	FBString	filePath;
	int			width;
	int			height;
	double		frameRate;

};

// struct for data in global landmarks list
struct LandmarkData
{
	char	name[128];
	char	label[128];
	int		active;

	void SetName(const char *_name)
	{
		memset(name, 0, sizeof(char) * 128);
		strcpy(name, _name);
	}

	void SetLabel(const char *_label)
	{
		memset(label, 0, sizeof(char) * 128);
		strcpy(label, _label);
	}
};

// struct for data in global markup groups list
struct MarkupGroup
{
	char name[128];

	void SetName(const char *_name)
	{
		memset(name, 0, sizeof(char) * 128);
		strcpy(name, _name);
	}
};

struct FrameLandmark
{
	int	landmarkIndex;	// index in global markup list
	float u;
	float v;

	// a constructor
	FrameLandmark()
	{
		landmarkIndex = -1;
		u = 0.0f;
		v = 0.0f;
	}
};

struct FrameGroup
{
	int groupIndex;	// index in global markup_groups
	std::vector<FrameLandmark>	landmarks;
};

//
struct FrameData
{
	int		value;
	int		valid;
	char	filePath[256];

	std::vector<FrameLandmark>		landmarks;

	void SetFilePath(const char *path)
	{
		memset(filePath, 0, sizeof(char) * 256);
		strcpy(filePath, path);
	}
};

// TODO: read frames
// 2 - convert frames into optical model markers

bool ReadRootMeta(TiXmlElement *rootElem, MetaData &outData)
{
	TiXmlElement	*metaElem = rootElem->FirstChildElement("meta");

	if (nullptr == metaElem)
		return false;

	TiXmlElement *pVideoElem = metaElem->FirstChildElement("video");

	if (nullptr == pVideoElem)
		return false;

	for (TiXmlAttribute *pAttrib = pVideoElem->FirstAttribute();
		pAttrib != nullptr;
		pAttrib = pAttrib->Next() )
	{
		if (strcmp(pAttrib->Name(), "filePath") == 0)
		{
			outData.filePath = pAttrib->Value();
		}
		else if (strcmp(pAttrib->Name(), "width") == 0)
		{
			outData.width = pAttrib->IntValue();
		}
		else if (strcmp(pAttrib->Name(), "height") == 0)
		{
			outData.height = pAttrib->IntValue();
		}
		else if (strcmp(pAttrib->Name(), "frameRate") == 0)
		{
			outData.frameRate = pAttrib->DoubleValue();
		}
	}

	return true;
}

bool ReadMarkupLandmarks(TiXmlElement *rootElem, std::vector<LandmarkData> &globalLandmarks, FrameData &outData)
{

	TiXmlElement *theLandmarkElem = rootElem->FirstChildElement("landmark");

	while (theLandmarkElem != nullptr)
	{

		FrameLandmark	frameLandmark;

		std::string		name("");
		std::string		label("");

		for (TiXmlAttribute *pAttrib = theLandmarkElem->FirstAttribute();
			pAttrib != nullptr;
			pAttrib = pAttrib->Next() )
		{
			if (strcmp(pAttrib->Name(), "name") == 0)
			{
				name = pAttrib->Value();
			}
			else if (strcmp(pAttrib->Name(), "label") == 0)
			{
				label = pAttrib->Value();
			}
		}

		// read values

		TiXmlElement *texCoordElem = theLandmarkElem->FirstChildElement("texCoord");

		if (texCoordElem)
		{
			std::string s;

			for (TiXmlAttribute *pAttrib = texCoordElem->FirstAttribute();
				pAttrib != nullptr;
				pAttrib = pAttrib->Next() )
			{
				s = pAttrib->Value();
				std::replace(s.begin(), s.end(), ',', '.');

				if (strcmp(pAttrib->Name(), "u") == 0)
				{	
					frameLandmark.u = (float) std::atof(s.c_str() );
				}
				else if (strcmp(pAttrib->Name(), "v") == 0)
				{
					frameLandmark.v = (float) std::atof(s.c_str() );
				}
			}
		}

		//
		frameLandmark.landmarkIndex = -1;

		if (name.size() > 0 && label.size() > 0)
		{
			for (size_t i=0; i<globalLandmarks.size(); ++i)
			{
				if (strcmp(label.c_str(), globalLandmarks[i].name) == 0)
				{
					frameLandmark.landmarkIndex = i;
					break;
				}
			}

			if (frameLandmark.landmarkIndex == -1)
			{
				LandmarkData	newLandmark;
				newLandmark.active = 1;
				newLandmark.SetName( name.c_str() );
				newLandmark.SetLabel( label.c_str() );

				globalLandmarks.push_back(newLandmark);
				frameLandmark.landmarkIndex = (int) globalLandmarks.size() - 1;
			}

			outData.landmarks.push_back(frameLandmark);
		}

		theLandmarkElem = theLandmarkElem->NextSiblingElement();
	}

	return true;
}

bool ReadMarkupGroups(TiXmlElement *rootElem, std::vector<MarkupGroup> &globalGroups, std::vector<LandmarkData> &globalLandmarks, FrameData &outData)
{
	TiXmlElement *theGroupElem = rootElem->FirstChildElement("markup_group");

	while(theGroupElem != nullptr)
	{

		FrameGroup	frameGroup;
		

		TiXmlAttribute *pAttrib = theGroupElem->FirstAttribute();
		if ( strcmp(pAttrib->Name(), "name") == 0 )
		{
			frameGroup.groupIndex = -1;

			for (size_t i=0; i<globalGroups.size(); ++i)
				if (strstr(globalGroups[i].name, pAttrib->Name() ) != nullptr)
				{
					frameGroup.groupIndex = (int) i;
					break;
				}

			if (frameGroup.groupIndex == -1)
			{
				MarkupGroup newGroup;
				newGroup.SetName(pAttrib->Name() );
				globalGroups.push_back(newGroup);

				frameGroup.groupIndex = (int) globalGroups.size() - 1;
			}

			TiXmlElement *landmarksElem = theGroupElem->FirstChildElement("landmarks");

			if (landmarksElem != nullptr)
				ReadMarkupLandmarks(landmarksElem, globalLandmarks, outData);

			//outData.groups.push_back(pFrameGroup);
		}
		

		theGroupElem = theGroupElem->NextSiblingElement();
	}

	return true;
}

bool ReadFrames(TiXmlElement *rootElem, std::vector<MarkupGroup> &globalGroups, std::vector<LandmarkData> &globalLandmarks, std::vector<FrameData> &frames)
{
	TiXmlElement	*framesElem = rootElem->FirstChildElement("frames");

	if (nullptr == framesElem)
		return false;

	TiXmlElement *theFrameElem = framesElem->FirstChildElement("frame");

	while(theFrameElem != nullptr)
	{
		FrameData		frameData;

		for (TiXmlAttribute *pAttrib = theFrameElem->FirstAttribute();
			pAttrib != nullptr;
			pAttrib = pAttrib->Next() )
		{
			if (strcmp(pAttrib->Name(), "filePath") == 0)
			{
				frameData.SetFilePath( pAttrib->Value() );
			}
			else if (strcmp(pAttrib->Name(), "value") == 0)
			{
				frameData.value = pAttrib->IntValue();
			}
			else if (strcmp(pAttrib->Name(), "valid") == 0)
			{
				frameData.valid = (strcmp(pAttrib->Value(), "true") == 0);
			}
		}

		TiXmlElement *markupGroupsElem = theFrameElem->FirstChildElement("markup_groups");

		if (markupGroupsElem != nullptr)
			ReadMarkupGroups(markupGroupsElem, globalGroups, globalLandmarks, frameData);

		frames.push_back(frameData);

		theFrameElem = theFrameElem->NextSiblingElement();
	}

	return true;
}

bool ORToolSample::ImportLandmarks(FBModel *pRoot, const bool asOptical, const char *filename)
{

	TiXmlDocument	doc;
	MetaData		metaData;

	std::vector<MarkupGroup>	groups;
	std::vector<LandmarkData>	landmarks;
	std::vector<FrameData>		frames;

	if (false == doc.LoadFile(filename))
		return false;

	try
	{
		TiXmlElement	*rootElem = doc.FirstChildElement("Analyzer_Feature_File");

		if (nullptr == rootElem)
			return false;

		
		if (false == ReadRootMeta(rootElem, metaData))
			return false;

		if (false == ReadFrames(rootElem, groups, landmarks, frames))
			return false;

	}
	catch(const std::exception &e)
	{
		printf ("error while reading file - %s\n", e.what() );
	}

	// UPDATE UI WIDTH and Height params

	int realWidth = (mButtonSwapSize.State > 0) ? metaData.height : metaData.width;
	int realHeight = (mButtonSwapSize.State > 0) ? metaData.width : metaData.height;

	if (metaData.width > 1)
		mEditWidth.Value = (double) realWidth;
	if (metaData.height > 1)
		mEditHeight.Value = (double) realHeight;
	FBPlayerControl::TheOne().SetTransportFps(kFBTimeModeCustom, metaData.frameRate );
	FBPlayerControl::TheOne().LoopStop = FBTime(0,0,0, (int) frames.size());
	//FBPlayerControl::TheOne().ZoomWindowStop = FBTime(0,0,0, (int) frames.size());

	// write data to root element

	FBProperty *prop;
	
	prop = pRoot->PropertyList.Find("SourceWidth"); 
	if (nullptr == prop) 
		prop = pRoot->PropertyCreate( "SourceWidth", kFBPT_int, "int", false, true );
	if (prop) prop->SetInt( realWidth  );

	prop = pRoot->PropertyList.Find("SourceHeight");
	if (nullptr == prop) 
		prop = pRoot->PropertyCreate( "SourceHeight", kFBPT_int, "int", false, true );
	if (prop) prop->SetInt( realHeight );


	//
	// markers

	if (frames.size() == 0 || landmarks.size() == 0)
		return false;

	const bool invertedHeight = (mButtonInvHeight.State == 1);

	if (asOptical)
	{

		FBModelOptical *pOptical = (FBModelOptical*) pRoot;
	
		FBTime	start( 0,0,0, 0 ); // FBPlayerControl().ZoomWindowStart 
		FBTime	stop( 0,0,0, (int) frames.size() );
		FBTime	oneFrame( 0,0,0, 1 ); //, 0, FBPlayerControl().GetTransportFps(), FBPlayerControl().GetTransportFpsValue() );

		// set sampling characteristics
		pOptical->SamplingStart = start;
		pOptical->SamplingStop = stop;
		pOptical->SamplingPeriod = oneFrame;

		//
		for (int i = 0; i< (int)landmarks.size(); ++i)
		{
			FBModelMarkerOptical *pMarker = new FBModelMarkerOptical( landmarks[i].name, pOptical );
			pMarker->Show = true;


			int samples = pMarker->ImportBegin();
			if (samples > (int) frames.size() ) samples = (int)frames.size();

			for (auto iframe=frames.begin(); iframe!=frames.end(); ++iframe)
			{

				for (auto imark=iframe->landmarks.begin(); imark!=iframe->landmarks.end(); ++imark)
				{

					if (i == imark->landmarkIndex)
					{
						double v = 100.0 * (double) imark->v;

						if (invertedHeight)
							v = 100.0 - v;

						pMarker->ImportKey( 100.0 * (double) imark->u, v );
						break;
					}

				}

			}

			pMarker->ImportEnd();

		}
	}
	else
	{
		double values[3] = {0.0, 0.0, 0.0};

		for (int i = 0; i< (int)landmarks.size(); ++i)
		{
			FBModelNull *pPoint = new FBModelNull( landmarks[i].name );
			pPoint->Show = true;
			pPoint->Visibility = true;
			pPoint->Parent = pRoot;

			FBAnimationNode *pNode = pPoint->Translation.GetAnimationNode();

			if (pNode == nullptr)
			{
				pPoint->Translation.SetAnimated(true);
				pNode = pPoint->Translation.GetAnimationNode();
			}

			if (pNode == nullptr)
				continue;

			
			for (auto iframe=frames.begin(); iframe!=frames.end(); ++iframe)
			{
				FBTime frameTime(0, 0, 0, iframe->value);
				for (auto imark=iframe->landmarks.begin(); imark!=iframe->landmarks.end(); ++imark)
				{

					if (i == imark->landmarkIndex)
					{
						double v = 100.0 * (double) imark->v;

						if (invertedHeight)
							v = 100.0 - v;

						values[0] = 100.0 * (double) imark->u;
						values[1] = v;

						pNode->KeyAdd( frameTime,  &values[0] );
						break;
					}
				}
			}
		}
	}

	return true;
}

void ORToolSample::BatchProcessing(const char *sourceFolder, const char *destinationFolder)
{

	CollectFilesFromDirectory	srcFiles(".xml");
	CollectFilesFromDirectory	dstFiles(".fbx");

	if (false == srcFiles.DoIt( sourceFolder, false ) )
		return;

	if (false == dstFiles.DoIt( destinationFolder, false ) )
		return;

	// convert each file which is not exist in the destination folder
	FBStringList &srcList = srcFiles.GetList();
	FBStringList &dstList = dstFiles.GetList();

	for (int i=0; i<srcList.GetCount(); ++i)
	{
		FBString srcPath(srcList.GetAt(i) );

		FBString srcName = ExtractFileName(srcPath);
		ChangeFileExt( srcName, FBString("") );

		bool fileExist = false;

		for (int j=0; j<dstList.GetCount(); ++j)
		{
			FBString dstPath(dstList.GetAt(j));

			FBString dstName = ExtractFileName(dstPath);
			ChangeFileExt( dstName, FBString("") );

			// compare in lower case
			if ( _stricmp(srcName, dstName) == 0 )
			{
				fileExist = true;
				break;
			}

		}


		// do import and processing
		if (false == fileExist)
		{
			mApp.FileNew();

			FBString srcPath(sourceFolder);
			srcPath = srcPath + "\\" + srcName + ".xml";

			mEditLandmarksPath.Text = srcPath;

			EventButtonImportClick(nullptr, nullptr);


			//

			FBString dstPath(destinationFolder);
			dstPath = dstPath + "\\" + srcName + ".fbx";

			mApp.FileSave( dstPath );
		}

	}

}


void ORToolSample::EventButtonPostFilteringClick	( HISender pSender, HKEvent pEvent )
{

	FBModel *pRoot = FBFindModelByLabelName( "FaceFeaturesOptical" );
	if (pRoot == nullptr)
		return;

	FBModelOptical *pOpticalRoot = (FBModelOptical*)pRoot;

	const char *rigidBodyMarkerNames[] = {	"NOSE_LOWER_MIDDLE", 
											"MOUTH_LIP_OUTER_BOTTOM_MIDDLE", 
											"MOUTH_LIP_OUTER_CORNER_RIGHT", 
											"MOUTH_LIP_OUTER_CORNER_LEFT",
											"JAW_RIGHT",
											"JAW_MIDDLE",
											"JAW_LEFT",
											nullptr};

	FBModelList *pMarkers = FBCreateModelList();

	int ndx=0;
	while(rigidBodyMarkerNames[ndx] != nullptr)
	{
		FBModel *pMarker = FBFindModelByLabelName(rigidBodyMarkerNames[ndx]);
		if (pMarker)
			pMarkers->Add(pMarker);

		ndx++;
	}

	if (pMarkers->GetCount() >= 3)
		pOpticalRoot->RigidBodies.Add(*pMarkers, "JawRigidBody");

	FBDestroyModelList(pMarkers);

	//
	//
	for (int i=0, count=pOpticalRoot->Markers.GetCount(); i<count; ++i)
	{
		pOpticalRoot->Markers[i]->Done = true;
	}

	// filter with Butterworth
	FBFilterManager		lFilterManager;
	FBFilter *pFilter = lFilterManager.CreateFilter("Butterworth");

	if (pFilter)
	{

		const char *filterMarkerNames[] = {	"NOSE_LOWER_MIDDLE", 
											"NOSTRIL_OUTER_RIGHT", 
											"NOSTRIL_OUTER_LEFT", 
											"BROW_INNER_LEFT",
											"BROW_INNER_RIGHT",
											"BROW_INNER_LEFT-BROW_MIDDLE_LEFT-0.5",
											"BROW_INNER_RIGHT-BROW_MIDDLE_RIGHT-0.5",
											"BROW_MIDDLE_LEFT",
											"BROW_MIDDLE_RIGHT",
											"JAW_RIGHT",
											"JAW_MIDDLE",
											"JAW_LEFT",
											nullptr};

		double freq = 5.0;
		pFilter->PropertyList.Find("Cut-off Frequency (Hz)")->SetData(&freq);

		for (int i=0, count=pOpticalRoot->Markers.GetCount(); i<count; ++i)
		{
			FBModel *pMarker = pOpticalRoot->Markers[i];
			FBString name(pMarker->Name);

			bool needFilter = false;

			int ndx=0;
			while (filterMarkerNames[ndx] != nullptr)
			{
				if (strcmp(name, filterMarkerNames[ndx])==0)
				{
					needFilter = true;
					break;
				}

				ndx++;
			}

			if (needFilter)
			{
				pFilter->Apply( pMarker->Translation.GetAnimationNode(), true );
			}
		}

		//

		freq = 7.0;
		pFilter->PropertyList.Find("Cut-off Frequency (Hz)")->SetData(&freq);

		for (int i=0, count=pOpticalRoot->Markers.GetCount(); i<count; ++i)
		{
			FBModel *pMarker = pOpticalRoot->Markers[i];
			FBString name(pMarker->Name);
			
			if (strstr( (char*)name, "MOUTH_LIP_OUTER") != nullptr)
			{
				pFilter->Apply( pMarker->Translation.GetAnimationNode(), true );
			}
		}



		delete pFilter;
	}
}

double SetDoubleFractionPart(int integer, int frac)
{
	double frac2 = frac;
		
	double log10 = log10l(frac2 + 1.0);
    double ceil = ceill(log10);
    double pow = powl(10.0, -ceil);

    double res = abs(integer);
	res += frac2 * pow;

	if (integer < 0)
	{
		res = -res;
	}

	return res;
}

bool ORToolSample::ImportOpenFaceLandmarks2d(FBModel *pRoot, const bool asOptical, const char *filename,
	const double width, const double height, const bool invertHeight, bool applyRenameRule, bool importJawOnly)
{
	if (pRoot == nullptr || filename == nullptr)
		return false;

	FILE *fp = fopen( filename, "r" );

	if (fp == nullptr)
		return false;

	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);

	if (len == 0)
	{
		fclose(fp);
		return false;
	}

	fseek(fp, 0, SEEK_SET);

	char *data = new char[len+1];
	memset(data, 0, sizeof(char)*(len+1));
	size_t bytesRead = fread_s(data, sizeof(char)*len, sizeof(unsigned char), len, fp);
	
	fclose(fp);

	FBStringList	strings;
	strings.SetString( data, '\n' );

	//
	if (data)
	{
		delete [] data;
		data = nullptr;
	}

	const int numberOfLines = strings.GetCount();
	if (numberOfLines == 0)
		return false;


	// read from the first line - frame,success,confidence,x0,x1,x2,x3,...y0,y1,y2,...
	FBStringList names;
	names.SetString( strings[0], ',' );

	int numberOfPoints = 0;

	// find the latest x or y index
	int frameNdx = -1;
	int xNdx = -1;
	int yNdx = -1;
	int pos = 1;
	int val = 0;
	int val_integer, val_fraction;
	double dval = 0.0;
	

	//int strpos = 0;
	for (int i=0, count=names.GetCount(); i<count; ++i)
	{
		const char *str = names.GetAt(i);	
		int len = (int)strlen(str);
		
		if ( strcmp( str, "frame" ) == 0 )
			frameNdx = i;
		else if (strstr( str, "x" ) != nullptr )
		{
			if (xNdx == -1)
				xNdx = i;
			pos = 1;
			GetNextValue(str, len, ',', pos, val);

			if (val >= numberOfPoints) 
				numberOfPoints = val + 1;
		}
		else if (yNdx == -1 && strstr(str, "y") != nullptr)
		{
			yNdx = i;
		}

		// strpos = strpos + len + 1; // 1 is a separator char ','
	}

	// we have a wrong file format
	if (frameNdx < 0 || xNdx < 0 || yNdx < 0 || numberOfPoints <= 0)
		return false;

	// All of the models (except CLM-Z) use a 68 point convention for tracking
	
	const int numberOfFrames = numberOfLines-1; // first line is an annotation
	if (numberOfFrames <= 0)
		return false;

	// alloc mem for all values + frame index
	double *values = new double[numberOfFrames * (numberOfPoints * 3 + 1)];

	if (values == nullptr)
		return false;


	for (int theFrame=0; theFrame<numberOfFrames; ++theFrame)
	{
		double *localValues = &values[theFrame * (numberOfPoints * 3 + 1)];

		// NOTE! first line is an annotation, we start from the second one
		const char *line = strings.GetAt(theFrame + 1);

		// read values
		int len = (int)strlen(line);
		
		// grab the frame index
		pos = ComputeSubLinePos(line, len, ',', frameNdx);
		GetNextValue(line, len, ',', pos, localValues[0] );

		// read all x values
		pos = ComputeSubLinePos(line, len, ',', (xNdx+1) );		// this 1 mean that we need to skip quality fraction part
		for (int thePoint=0; thePoint<numberOfPoints; ++thePoint)
		{
			dval = 0.0;
			
			val_integer = 0;
			val_fraction = 0;

			GetNextValue(line, len, ',', pos, val_integer);
			GetNextValue(line, len, ',', pos, val_fraction);
			
			dval = SetDoubleFractionPart(val_integer, val_fraction);
			dval = dval * width;

			localValues[thePoint*3+1] = dval;
		}

		// read all y values and set z to zero
		//pos = ComputeSubLinePos(line, len, ',', yNdx);
		for (int thePoint=0; thePoint<numberOfPoints; ++thePoint)
		{
			dval = 0.0;
			val_integer = 0;
			val_fraction = 0;

			GetNextValue(line, len, ',', pos, val_integer);
			GetNextValue(line, len, ',', pos, val_fraction);

			dval = SetDoubleFractionPart(val_integer, val_fraction);
			dval = dval * height;
			
			if (invertHeight)
				dval = 100.0 - dval;

			localValues[thePoint*3+2] = dval;
			localValues[thePoint*3+3] = 0.0;
		}
	}

	//
	// write values to elements

	auto fn_getname = [] (const int index, bool applyRenameRule) -> const char * {

		if (!applyRenameRule)
			return "OpenFace:FaceFeature";

		switch(index)
		{
		case 7:
			return "OpenFace:JAW_RIGHT";
		case 8:
			return "OpenFace:JAW_MIDDLE";
		case 9:
			return "OpenFace:JAW_LEFT";
		default:
			return "OpenFace:FaceFeature";
		}
	};

	if (asOptical)
	{
		//
		FBModelOptical *pOptical = (FBModelOptical*) pRoot;
	
		int firstFrameIndex = (int) values[0];

		FBTime	start( 0,0,0, firstFrameIndex ); // FBPlayerControl().ZoomWindowStart 
		FBTime	stop( 0,0,0, numberOfFrames );
		FBTime	oneFrame( 0,0,0, 1 ); //, 0, FBPlayerControl().GetTransportFps(), FBPlayerControl().GetTransportFpsValue() );

		if (pOptical->Children.GetCount() == 0)
		{
			// set sampling characteristics
			pOptical->SamplingStart = start;
			pOptical->SamplingStop = stop;
			pOptical->SamplingPeriod = oneFrame;
		}

		for (int i=0; i<numberOfPoints; ++i)
		{
			if (importJawOnly && (i<7 || i>9) )
			{
				continue;
			}

			FBModelMarkerOptical *pMarker = new FBModelMarkerOptical( fn_getname(i, applyRenameRule), pOptical );
			pMarker->Show = true;

			int samples = pMarker->ImportBegin();
			if (samples > numberOfFrames ) 
				samples = numberOfFrames;

			for (int j=0; j<samples; ++j)
			{
				double *localValues = &values[j * (numberOfPoints * 3 + 1) + i*3+1];
				pMarker->ImportKey( localValues[0], localValues[1] );
			}

			pMarker->ImportEnd();
		}
	}
	else
	{

		for (int thePoint=0; thePoint<numberOfPoints; ++thePoint)
		{
			if (importJawOnly && (thePoint<7 || thePoint>9) )
			{
				continue;
			}

			FBModelNull *pPoint = new FBModelNull( fn_getname(thePoint, applyRenameRule) );
			pPoint->Show = true;
			pPoint->Visibility = true;
			pPoint->Parent = pRoot;

			FBAnimationNode *pNode = pPoint->Translation.GetAnimationNode();

			if (pNode == nullptr)
			{
				pPoint->Translation.SetAnimated(true);
				pNode = pPoint->Translation.GetAnimationNode();
			}

			if (pNode == nullptr)
				continue;

			
			for (int theFrame=0; theFrame<numberOfFrames; ++theFrame)
			{
				double *localValues = &values[theFrame * (numberOfPoints * 3 + 1)];

				FBTime currTime(0,0,0, (int)localValues[0]);	

				// write values
				pNode->KeyAdd( currTime, &localValues[thePoint*3+1] );
			}
		}
	}

	//

	if (values)
	{
		delete [] values;
		values = nullptr;
	}

	return true;
}


int ORToolSample::ComputeSubLinePos(const char *line, const int len, const char sep, const int index)
{

	int pos = 0;
	int itemindex = 0;
	for (int i=0; i<len; ++i, ++pos)
	{
		if (index == itemindex)
			break;

		if (line[i] == sep)
		{
			itemindex += 1;
		}
	}

	return pos;
}