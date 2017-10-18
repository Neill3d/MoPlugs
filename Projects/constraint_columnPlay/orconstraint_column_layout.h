#ifndef __ORCONSTRAINT_PYTHON_LAYOUT_H__
#define __ORCONSTRAINT_PYTHON_LAYOUT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orconstraint_column_layout.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Class declaration
#include "orconstraint_column_constraint.h"

//! Simple constraint layout.
class ORConstraint_Column_Layout : public FBConstraintLayout
{
	//--- constraint layout declaration.
	FBConstraintLayoutDeclare( ORConstraint_Column_Layout, FBConstraintLayout );

public:
	virtual bool FBCreate();			//!< creation function.
	virtual void FBDestroy();			//!< destruction function.

	// UI Management
	void	UICreate	();
	void	UIConfigure	();
	void	UIReset		();

	// UI Callbacks
	void	EventMemoHeaderChanged( HISender pSender, HKEvent pEvent );
	void	EventMemoScriptChanged( HISender pSender, HKEvent pEvent );

private:
	ORConstraint_Column		*mConstraint;		//!< Handle onto constraint.

private:
	
};

#endif /* __ORCONSTRAINT_PYTHON_LAYOUT_H__ */
