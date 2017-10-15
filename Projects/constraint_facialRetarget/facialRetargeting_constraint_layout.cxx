
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: facialRetargeting_constraint_layout.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "facialRetargeting_constraint_layout.h"

//--- Registration define
#define ORCONSTRAINTFACIALRETARGET__LAYOUT	ConstraintFacialRetargeting_Layout

//--- implementation and registration
FBConstraintLayoutImplementation(	ORCONSTRAINTFACIALRETARGET__LAYOUT		);		// Layout class name
FBRegisterConstraintLayout		(	ORCONSTRAINTFACIALRETARGET__LAYOUT,				// Layout class name
									ORCONSTRAINTFACIALRETARGET__CLASSSTR,				// Constraint class name (for association)
									FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)


/************************************************
 *	Creation function..
 ************************************************/
bool ConstraintFacialRetargeting_Layout::FBCreate()
{
	// Assign handle onto constraint (cast generic pointer).
	mConstraint	= (ConstraintFacialRetargeting*) (FBConstraint*) Constraint;

	// Create & configure the UI
	UICreate	();		// create the UI
	UIConfigure	();		// configure the UI
	UIReset		();		// set the UI values from the constraint

	return true;
}


/************************************************
 *	Destructor for layout.
 ************************************************/
void ConstraintFacialRetargeting_Layout::FBDestroy()
{
}


/************************************************
 *	UI Creation function.
 *	Create UI regions and assign them to UI elements
 ************************************************/
void ConstraintFacialRetargeting_Layout::UICreate()
{
	//int lS = 4;
	int lS = 4;
	int lH = 18;
	int lW = 150;

	// Create UI
	AddRegion( "propertywindow", "propertywindow",
											lS,	kFBAttachTop,	"",		1.0,
											lS,	kFBAttachLeft,	"",		1.0,
											-lS,kFBAttachRight,	"",		1.0,
											-lS,kFBAttachBottom,"",	1.0 );

	
	// Assign regions
	SetControl( "propertywindow", mPropertyWindow );
}


/************************************************
 *	Reset the UI from the constraint values.
 ************************************************/
void ConstraintFacialRetargeting_Layout::UIReset()
{
}


/************************************************
 *	UI Configuration function.
 *	Assign properties & callbacks for UI elements
 ************************************************/
void ConstraintFacialRetargeting_Layout::UIConfigure()
{
	// Configure UI
	mPropertyWindow.AddObject( mConstraint );
}


/************************************************
 *	Button click callback.
 ************************************************/
void ConstraintFacialRetargeting_Layout::EventButtonTestClick( HISender pSender, HKEvent pEvent )
{
}


