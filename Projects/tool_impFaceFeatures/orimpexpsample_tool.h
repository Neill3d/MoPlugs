#ifndef __ORIMPEXP_SAMPLE_TOOL_H__
#define __ORIMPEXP_SAMPLE_TOOL_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orimpexpsample_tool.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////



//--- Class declaration
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define	ORTOOLSAMPLE__CLASSNAME		ORToolSample
#define ORTOOLSAMPLE__CLASSSTR		"ORToolSample"

///////////////////////////////////////////////////////////////////////////////
//

class CollectFilesFromDirectory : public FBVisitDirectoryCallback
{
public:

	//! a constructor
	CollectFilesFromDirectory(const char *extension="")
		: mExtension(extension)
	{
	}

	virtual ~CollectFilesFromDirectory()
	{}

	virtual void FileVisited(const char*  pSrc) const override
	{
		if ( strstr(pSrc, mExtension) != nullptr )
		{
			FBStringList *pList = (FBStringList*) &mList;
			pList->Add(pSrc);
		}
	}
	virtual void DirectoryVisited(const char*  pSrc) const override
	{}

	FBStringList	&GetList()
	{
		return mList;
	}

protected:

	FBString			mExtension;
	FBStringList		mList;
};

/////////////////////////////////////////////////////////////////////////////////////////
/**	Import/Export Tool.
*	Tool with the correct buttons to import/export scenes & information.
*/
class ORToolSample : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( ORToolSample, FBTool );

public:
	//--- FiLMBOX constructor & destructor
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

	// UI Management
	void	UICreate	();
	void	UIConfigure	();

	// UI Callbacks
	void	EventButtonFaceBrowseClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonBrowseClick	( HISender pSender, HKEvent pEvent );
	void	EventButton3BrowseClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonShapeBrowseClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonLandmarksBrowseClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonImportClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonBatchImportClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonPostFilteringClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonAboutClick	( HISender pSender, HKEvent pEvent );
	
	void	EventButtonImportOpenFace2DClick	( HISender pSender, HKEvent pEvent );

private:
	
	// common settings
	FBLabel				mLabelCommon;
	FBLabel				mLabelNumberPoints;
	FBEditNumber		mEditNumberPoints;		// set up a number of points inside points database
	FBLabel				mLabelNumberRate;
	FBEditNumber		mEditNumberRate;		//!< Import rate.

	FBLabel				mLabelWidth;
	FBEditNumber		mEditWidth;
	FBLabel				mLabelHeight;
	FBEditNumber		mEditHeight;
	
	FBButton			mButtonInvHeight;
	FBButton			mButtonImportAsOptical;		// import with null points or as an optical data
	FBButton			mButtonImportUnderTheRoot;	// reparent import points to a selected element
	FBButton			mButtonPointsJawOnly;	// import only 3 jaw markers
	FBButton			mButtonPointsEmptyJaw;		// import 3 empty jaw markers (reconstruct using rigid body later)
	FBButton			mButtonSwapSize;			// for a 90 degree rotated images, swap width and height for CLM

	// face file
	FBButton			mButtonFaceEnable;
	FBEdit				mEditFaceFile;		//!< Import file.
	FBButton			mButtonFaceBrowse;

	// 2d points file
	FBButton			mButtonPointsEnable;
	
	FBEdit				mEditPointsFile;		//!< Import file.
	FBButton			mButtonPointsBrowse;

	// 3d points file
	FBButton			mButtonPointsEnable3;
	FBEdit				mEditPointsFile3;		//!< Import file.
	FBButton			mButtonPointsBrowse3;
	
	// SSF shape markers
	FBButton			mButtonShapeEnable;
	FBEdit				mEditShapePath;
	FBButton			mButtonShapeBrowse;

	// XML Faceware landmarks
	FBButton			mButtonLandmarks;
	FBEdit				mEditLandmarksPath;
	FBButton			mButtonLandmarksBrowse;

	FBButton			mButtonImport;		//!< Browse button.
	FBButton			mButtonBatchImport;
	FBButton			mButtonPostFiltering;
	FBButton			mButtonAbout;

	FBButton			mButtonImportOpenFace2d;		// import OpenFace 2d landmarks
private:
	
	FBApplication		mApp;
	FBSystem			mSystem;			//!< To interact with FiLMBOX.
	
	char temp_buffer[32];

	bool GetNextValue(const char *line, const int len, const char separator, int &position, double &value);
	bool GetNextValue(const char *line, const int len, const char separator, int &position, int &value);

	static int ComputeSubLinePos(const char *line, const int len, const char sep, const int index);

	bool ImportFace(const char *filename);
	bool ImportPoints3d(const char *filename);
	bool ImportShapeData(const char *filename);

	bool ImportPoints(FBModel *pRoot, const bool asOptical, const char *filename);
	bool ImportLandmarks(FBModel *pRoot, const bool asOptical, const char *filename);

	bool ImportOpenFaceLandmarks2d(FBModel *pRoot, const bool asOptical, const char *filename, 
		const double width, const double height, const bool invertHeight, bool applyRenameRule,
		bool importJawOnly);

	bool ImportEmptyJawPoints(FBModel *pRoot, const bool asOptical);

	void BatchProcessing(const char *sourceFolder, const char *destinationFolder);
};

#endif /* __ORIMPEXP_SAMPLE_TOOL_H__ */
