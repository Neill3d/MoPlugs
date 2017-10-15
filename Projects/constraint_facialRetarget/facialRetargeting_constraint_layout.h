#ifndef __ORCONSTRAINT_FACIALRETARGETING_LAYOUT_H__
#define __ORCONSTRAINT_FACIALRETARGETING_LAYOUT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: facialRetargeting_constraint_layout.h
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
#include "facialRetargeting_constraint.h"

//! Simple constraint layout.
class ConstraintFacialRetargeting_Layout : public FBConstraintLayout
{
	//--- constraint layout declaration.
	FBConstraintLayoutDeclare( ConstraintFacialRetargeting_Layout, FBConstraintLayout );

public:
	virtual bool FBCreate();			//!< creation function.
	virtual void FBDestroy();			//!< destruction function.

	// UI Management
	void	UICreate	();
	void	UIConfigure	();
	void	UIReset		();

	// UI Callbacks
	void	EventButtonTestClick( HISender pSender, HKEvent pEvent );

private:
	ConstraintFacialRetargeting*	mConstraint;		//!< Handle onto constraint.

private:
	FBBrowsingProperty			mPropertyWindow;
};

#endif /* __ORCONSTRAINT_TEMPLATE_LAYOUT_H__ */
