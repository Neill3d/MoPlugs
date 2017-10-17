
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: wallbricks_constraint_layout.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "wallbricks_constraint_layout.h"

//--- Registration defines
#define WALLBRICKSCONSTRAINT__LAYOUT	WallBricksConstraintLayout

//--- implementation and registration
FBConstraintLayoutImplementation(	WALLBRICKSCONSTRAINT__LAYOUT	);
FBRegisterConstraintLayout		(	WALLBRICKSCONSTRAINT__LAYOUT,
									ORCONSTRAINTWALLBRICKS__CLASSSTR,
									FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)



/************************************************
 *	Creation function..
 ************************************************/
bool WallBricksConstraintLayout::FBCreate()
{
	// Assign handle onto constraint (cast generic pointer).
	mConstraint	= (ORConstraintWallBricks*) (FBConstraint*) Constraint;

	// build & configure layout
	UICreate	();
	UIConfigure	();
	UIReset();
	return true;
}


/************************************************
 *	Destructor for layout.
 ************************************************/
void WallBricksConstraintLayout::FBDestroy()
{
}


/************************************************
 *	UI Creation function.
 *	Create UI regions and assign them to UI elements
 ************************************************/
void WallBricksConstraintLayout::UICreate()
{
	int lS, lH, lB, lW;	// space, width, height

	lS = 4;
	lB = 5;
	lH = 20;
	lW = 120;

	AddRegion( "ButtonReset",		"ButtonReset",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachNone,	"",						1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "ButtonLoad",		"ButtonLoad",
									lB,		kFBAttachRight,	"ButtonReset",			1.0,
									lB,		kFBAttachNone,	"",						1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "ButtonSave",		"ButtonSave",
									lB,		kFBAttachRight,	"ButtonLoad",			1.0,
									lB,		kFBAttachNone,	"",						1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "ButtonHelp",		"ButtonHelp",
									lB,		kFBAttachRight,	"ButtonSave",			1.0,
									lB,		kFBAttachNone,	"",						1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "ButtonAbout",		"ButtonAbout",
									lB,		kFBAttachRight,	"ButtonHelp",			1.0,
									lB,		kFBAttachNone,	"",						1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "EditPosX",		"EditPosX",
									lB,		kFBAttachLeft,	"",						1.0,
									lB*2,	kFBAttachBottom,"ButtonReset",			1.0,
									-lB,	kFBAttachRight,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "EditPosY",		"EditPosY",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachBottom,"EditPosX",				1.0,
									-lB,	kFBAttachRight,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "EditPosZ",		"EditPosZ",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachBottom,"EditPosY",				1.0,
									-lB,	kFBAttachRight,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "EditRotX",		"EditRotX",
									lB,		kFBAttachLeft,	"",						1.0,
									lB*2,	kFBAttachBottom,"EditPosZ",				1.0,
									-lB,	kFBAttachRight,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "EditRotY",		"EditRotY",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachBottom,"EditRotX",				1.0,
									-lB,	kFBAttachRight,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "EditRotZ",		"EditRotZ",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachBottom,"EditRotY",				1.0,
									-lB,	kFBAttachRight,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "EditSclX",		"EditSclX",
									lB,		kFBAttachLeft,	"",						1.0,
									lB*2,	kFBAttachBottom,"EditRotZ",				1.0,
									-lB,	kFBAttachRight,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "EditSclY",		"EditSclY",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachBottom,"EditSclX",				1.0,
									-lB,	kFBAttachRight,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "EditSclZ",		"EditSclZ",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachBottom,"EditSclY",				1.0,
									-lB,	kFBAttachRight,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	//
	//

	SetControl( "ButtonReset", mButtonReset );
	SetControl( "ButtonLoad", mButtonLoad );
	SetControl( "ButtonSave", mButtonSave );

	SetControl( "ButtonHelp", mButtonHelp );
	SetControl( "ButtonAbout", mButtonAbout );

	SetControl( "EditPosX", mEditPositionX );
	SetControl( "EditPosY", mEditPositionY );
	SetControl( "EditPosZ", mEditPositionZ );

	SetControl( "EditRotX", mEditRotationX );
	SetControl( "EditRotY", mEditRotationY );
	SetControl( "EditRotZ", mEditRotationZ );

	SetControl( "EditSclX", mEditScalingX );
	SetControl( "EditSclY", mEditScalingY );
	SetControl( "EditSclZ", mEditScalingZ );
}


/************************************************
 *	UI Configuration function.
 *	Assign properties & callbacks for UI elements
 ************************************************/
void WallBricksConstraintLayout::UIConfigure()
{
	const int captionSize = 0;

	mButtonReset.Property = &mConstraint->ExpressionReset;
	mButtonReset.CaptionSize = captionSize;
	mButtonReset.Caption = "Reset";

	mButtonLoad.Property = &mConstraint->ExpressionLoad;
	mButtonLoad.CaptionSize = 40;
	mButtonLoad.Caption = "Load";
	mButtonSave.Property = &mConstraint->ExpressionSave;
	mButtonSave.CaptionSize = captionSize;
	mButtonSave.Caption = "Save";

	mButtonHelp.Property = &mConstraint->Help;
	mButtonHelp.CaptionSize = captionSize;
	mButtonHelp.Caption = "Help";
	mButtonAbout.Property = &mConstraint->About;
	mButtonAbout.CaptionSize = captionSize;
	mButtonAbout.Caption = "About";

	mEditPositionX.Property = &mConstraint->ScriptPositionX;
	mEditPositionX.Caption = "PositionX:";

	mEditPositionY.Property = &mConstraint->ScriptPositionY;
	mEditPositionY.Caption = "PositionY:";
	mEditPositionZ.Property = &mConstraint->ScriptPositionZ;
	mEditPositionZ.Caption = "PositionZ:";

	mEditRotationX.Property = &mConstraint->ScriptRotationX;
	mEditRotationX.Caption = "RotationX:";

	mEditRotationY.Property = &mConstraint->ScriptRotationY;
	mEditRotationY.Caption = "RotationY:";
	mEditRotationZ.Property = &mConstraint->ScriptRotationZ;
	mEditRotationZ.Caption = "RotationZ:";

	mEditScalingX.Property = &mConstraint->ScriptScalingX;
	mEditScalingX.Caption = "ScalingX:";

	mEditScalingY.Property = &mConstraint->ScriptScalingY;
	mEditScalingY.Caption = "ScalingY:";
	mEditScalingZ.Property = &mConstraint->ScriptScalingZ;
	mEditScalingZ.Caption = "ScalingZ:";
}


void WallBricksConstraintLayout::UIReset()
{
}
