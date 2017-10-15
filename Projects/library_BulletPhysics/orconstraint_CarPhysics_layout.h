#ifndef __ORCONSTRAINT_TEMPLATE_LAYOUT_H__
#define __ORCONSTRAINT_TEMPLATE_LAYOUT_H__
/***************************************************************************************
 Autodesk(R) Open Reality(R) Samples
 
 (C) 2009 Autodesk, Inc. and/or its licensors
 All rights reserved.
 
 AUTODESK SOFTWARE LICENSE AGREEMENT
 Autodesk, Inc. licenses this Software to you only upon the condition that 
 you accept all of the terms contained in the Software License Agreement ("Agreement") 
 that is embedded in or that is delivered with this Software. By selecting 
 the "I ACCEPT" button at the end of the Agreement or by copying, installing, 
 uploading, accessing or using all or any portion of the Software you agree 
 to enter into the Agreement. A contract is then formed between Autodesk and 
 either you personally, if you acquire the Software for yourself, or the company 
 or other legal entity for which you are acquiring the software.
 
 AUTODESK, INC., MAKES NO WARRANTY, EITHER EXPRESS OR IMPLIED, INCLUDING BUT 
 NOT LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
 PURPOSE REGARDING THESE MATERIALS, AND MAKES SUCH MATERIALS AVAILABLE SOLELY ON AN 
 "AS-IS" BASIS.
 
 IN NO EVENT SHALL AUTODESK, INC., BE LIABLE TO ANYONE FOR SPECIAL, COLLATERAL, 
 INCIDENTAL, OR CONSEQUENTIAL DAMAGES IN CONNECTION WITH OR ARISING OUT OF PURCHASE 
 OR USE OF THESE MATERIALS. THE SOLE AND EXCLUSIVE LIABILITY TO AUTODESK, INC., 
 REGARDLESS OF THE FORM OF ACTION, SHALL NOT EXCEED THE PURCHASE PRICE OF THE 
 MATERIALS DESCRIBED HEREIN.
 
 Autodesk, Inc., reserves the right to revise and improve its products as it sees fit.
 
 Autodesk and Open Reality are registered trademarks or trademarks of Autodesk, Inc., 
 in the U.S.A. and/or other countries. All other brand names, product names, or 
 trademarks belong to their respective holders. 
 
 GOVERNMENT USE
 Use, duplication, or disclosure by the U.S. Government is subject to restrictions as 
 set forth in FAR 12.212 (Commercial Computer Software-Restricted Rights) and 
 DFAR 227.7202 (Rights in Technical Data and Computer Software), as applicable. 
 Manufacturer is Autodesk, Inc., 10 Duke Street, Montreal, Quebec, Canada, H3C 2L7.
***************************************************************************************/

/**	\file	orconstraint_template_layout.h
*	Declaration of a layout class for a simple constraint.
*	Declaration of the FBSimpleConstraintLayout class.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Class declaration
#include "orconstraint_template_constraint.h"

//! Simple constraint layout.
class ORConstraint_Template_Layout : public FBConstraintLayout
{
	//--- constraint layout declaration.
	FBConstraintLayoutDeclare( ORConstraint_Template_Layout, FBConstraintLayout );

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
	ORConstraint_Template*	mConstraint;		//!< Handle onto constraint.

private:
	FBLabel			mLabelDirections;	//!< Description label.
	FBButton		mButtonTest;		//!< Test button.
};

#endif /* __ORCONSTRAINT_TEMPLATE_LAYOUT_H__ */
