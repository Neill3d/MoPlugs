#ifndef __BAKEPROJECTORS_VIEW_H__
#define __BAKEPROJECTORS_VIEW_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: bakeProjectors_view.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>

enum EBakingState
{
	eBakingStateReady,
	eBakingStateWait
};

void SetBakingState( const EBakingState state );
const EBakingState GetBakingState();

/**	View 3D class.
*/
class ViewBakeProjectors : public FBView
{
	FBClassDeclare( ViewBakeProjectors, FBView );

private:

	FBSystem    mSystem;

	void		*mViewData;	// store special graphics data

	bool						mGrabImage;
	FBString					mGrabImageName;

	bool						mGrabWithFrameNumber;

	
public:

	//
	// PROPERTIES

	FBPropertyColorAndAlpha			BackgroungColor;

	FBPropertyBool					ImageAddFrameNumber;
	FBPropertyBool					SaveImagePerModel;	// for each model we create a new image
	FBPropertyBool					SaveOnlyProjectors;

	FBPropertyBool					SaveJpeg;
	FBPropertyInt					JpegQuality;


	static void ChangeWidth(HIObject pObject, int value);
	static void ChangeHeight(HIObject pObject, int value);

public:
	//! Constructor.
	ViewBakeProjectors();
	
	//! Destructor
	virtual void FBDestroy();

	//! Refresh callback.
	virtual void Refresh(bool pNow=false);
	//! Expose callback.
	virtual void ViewExpose();

	void		ClearModels();
	void		AddModel(FBModel *pModel, int _preset, int _width, int _height);

	void		SetCurrentRow(const int row);
	const int	GetCurrentRow();

	void		DoGrabCurrentFrame(bool grabWithFrameNumber, bool showDialog=true, const char *filename=nullptr);

};

#endif /* __BAKEPROJECTORS_VIEW_H__ */
