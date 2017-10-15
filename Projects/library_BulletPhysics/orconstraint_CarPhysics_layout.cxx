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

/**	\file	orconstraint_template_layout.cxx
*	Definition of a layout class for a simple constraint.
*	Simple constraint layout class, ORConstraint_Template_Layout for
*	the simple constraint example.
*/

//--- Class declaration
#include "orconstraint_template_layout.h"

//--- Registration define
#define ORCONSTRAINT__LAYOUT	ORConstraint_Template_Layout

//--- implementation and registration
FBConstraintLayoutImplementation(	ORCONSTRAINT__LAYOUT		);		// Layout class name
FBRegisterConstraintLayout		(	ORCONSTRAINT__LAYOUT,				// Layout class name
									ORCONSTRAINT__CLASSSTR,				// Constraint class name (for association)
									FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)


/************************************************
 *	Creation function..
 ************************************************/
bool ORConstraint_Template_Layout::FBCreate()
{
	// Assign handle onto constraint (cast generic pointer).
	mConstraint	= (ORConstraint_Template*) (FBConstraint*) Constraint;

	// Create & configure the UI
	UICreate	();		// create the UI
	UIConfigure	();		// configure the UI
	UIReset		();		// set the UI values from the constraint

	return true;
}


/************************************************
 *	Destructor for layout.
 ************************************************/
void ORConstraint_Template_Layout::FBDestroy()
{
}


/************************************************
 *	UI Creation function.
 *	Create UI regions and assign them to UI elements
 ************************************************/
void ORConstraint_Template_Layout::UICreate()
{
	//int lS = 4;
	int lS = 4;
	int lH = 18;
	int lW = 150;

	// Create UI
	AddRegion( "LabelDirections", "LabelDirections",
											lS,	kFBAttachTop,	"",		1.0,
											lS,	kFBAttachLeft,	"",		1.0,
											-lS,kFBAttachRight,	"",		1.0,
											65,	kFBAttachNone,	NULL,	1.0 );

	AddRegion( "ButtonTest", "ButtonTest",	lS,	kFBAttachLeft,	"",	1.0,
											lS,	kFBAttachBottom,"LabelDirections",	1.0,
											lW,	kFBAttachNone,	NULL,	1.0,
											lH,	kFBAttachNone,	NULL,	1.0 );
	// Assign regions
	SetControl( "LabelDirections", mLabelDirections	);
	SetControl( "ButtonTest",		mButtonTest			);
}


/************************************************
 *	Reset the UI from the constraint values.
 ************************************************/
void ORConstraint_Template_Layout::UIReset()
{
}


/************************************************
 *	UI Configuration function.
 *	Assign properties & callbacks for UI elements
 ************************************************/
void ORConstraint_Template_Layout::UIConfigure()
{
	// Configure UI
	mLabelDirections.Caption =	"This is a sample constraint with no references. Due to the fact\n"
								"that there are no references in the constraint only the custom\n"
								"user interface layout is created. Below is a button demonstrating\n"
								"a simple use of the toolkit and UI callbacks.\n";

	mButtonTest.Caption = "Test";
	mButtonTest.OnClick.Add( this, (FBCallback) &ORConstraint_Template_Layout::EventButtonTestClick );
}


/************************************************
 *	Button click callback.
 ************************************************/
void ORConstraint_Template_Layout::EventButtonTestClick( HISender pSender, HKEvent pEvent )
{
}


