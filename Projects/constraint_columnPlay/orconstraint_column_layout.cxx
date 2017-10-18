
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orconstraint_column_layout.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "orconstraint_column_layout.h"

//--- Registration define
#define ORCONSTRAINT__LAYOUT	ORConstraint_Column_Layout

//--- implementation and registration
FBConstraintLayoutImplementation(	ORCONSTRAINT__LAYOUT		);		// Layout class name
FBRegisterConstraintLayout		(	ORCONSTRAINT__LAYOUT,				// Layout class name
									ORCONSTRAINT__CLASSSTR,				// Constraint class name (for association)
									FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)


/************************************************
 *	Creation function..
 ************************************************/
bool ORConstraint_Column_Layout::FBCreate()
{
	// Assign handle onto constraint (cast generic pointer).
	mConstraint	= (ORConstraint_Column*) (FBConstraint*) Constraint;

	// Create & configure the UI
	UICreate	();		// create the UI
	UIConfigure	();		// configure the UI
	UIReset		();		// set the UI values from the constraint

	return true;
}


/************************************************
 *	Destructor for layout.
 ************************************************/
void ORConstraint_Column_Layout::FBDestroy()
{
}


/************************************************
 *	UI Creation function.
 *	Create UI regions and assign them to UI elements
 ************************************************/
void ORConstraint_Column_Layout::UICreate()
{
	/*
	//int lS = 4;
	int lS = 4;
	int lH = 18;
	int lW = 150;

	// Create UI
	AddRegion( "Header", "Header",
											lS,	kFBAttachTop,	"",		1.0,
											lS,	kFBAttachLeft,	"",		1.0,
											-lS,kFBAttachRight,	"",		1.0,
											0, kFBAttachHeight, "",		0.2 );
	AddRegion( "Script", "Script",
											lS,	kFBAttachLeft,	"",		1.0,
											lS,	kFBAttachBottom,"Header",		1.0,
											-lS,kFBAttachRight,	"",		1.0,
											0,kFBAttachHeight,	"",		0.7 );
	AddRegion( "Console", "Console",
											lS,	kFBAttachLeft,		"",			1.0,
											lS,	kFBAttachBottom,	"Script",	1.0,
											-lS,kFBAttachRight,		"",			1.0,
											-lS, kFBAttachBottom,	"",			1.0 );

	// Assign regions
	SetControl( "Header", mMemoHeader );
	SetControl( "Script", mMemoScript );
	SetControl( "Console", mMemoConsole );
	*/
}


/************************************************
 *	Reset the UI from the constraint values.
 ************************************************/
void ORConstraint_Column_Layout::UIReset()
{

}


/************************************************
 *	UI Configuration function.
 *	Assign properties & callbacks for UI elements
 ************************************************/
void ORConstraint_Column_Layout::UIConfigure()
{
	// Configure UI
	
	//mMemoHeader.OnChange.Add( this, (FBCallback) &ORConstraint_Python_Layout::EventMemoHeaderChanged );
	//mMemoScript.OnChange.Add( this, (FBCallback) &ORConstraint_Python_Layout::EventMemoScriptChanged );
	
}


/************************************************
 *	Memo callback.
 ************************************************/
void ORConstraint_Column_Layout::EventMemoHeaderChanged( HISender pSender, HKEvent pEvent )
{

}

void ORConstraint_Column_Layout::EventMemoScriptChanged( HISender pSender, HKEvent pEvent )
{

}

