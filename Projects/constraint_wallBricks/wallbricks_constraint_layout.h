
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: wallbricks_constraint_layout.h
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

//--- Class declaration
#include "wallbricks_constraint.h"

//! Simple constraint layout.
class WallBricksConstraintLayout : public FBConstraintLayout
{
	//--- constraint layout declaration.
	FBConstraintLayoutDeclare( WallBricksConstraintLayout, FBConstraintLayout );

public:
	//--- constructor/destructor
	virtual bool FBCreate();			//!< creation function.
	virtual void FBDestroy();			//!< destruction function.

	//--- UI Creation/Configuration
	void UICreate	();
	void UIConfigure();
	void UIReset ();

private:
	ORConstraintWallBricks			*mConstraint;			//!< Handle onto constraint.


	FBEditProperty					mButtonReset;	
	FBEditProperty					mButtonLoad;
	FBEditProperty					mButtonSave;

	FBEditProperty					mButtonHelp;
	FBEditProperty					mButtonAbout;

	FBEditProperty					mEditPositionX;
	FBEditProperty					mEditPositionY;
	FBEditProperty					mEditPositionZ;

	FBEditProperty					mEditRotationX;
	FBEditProperty					mEditRotationY;
	FBEditProperty					mEditRotationZ;

	FBEditProperty					mEditScalingX;
	FBEditProperty					mEditScalingY;
	FBEditProperty					mEditScalingZ;
};
