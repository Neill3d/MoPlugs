
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: helpMeOnFacial_tool.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "helpMeOnFacial_tool.h"


//--- Registration defines
#define TOOLHELPMEONFACIAL__CLASS	TOOLHELPMEONFACIAL__CLASSNAME
#define TOOLHELPMEONFACIAL__LABEL	"Help Me On Facial"
#define TOOLHELPMEONFACIAL__DESC	"Help Me On Facial Tool"

//--- FiLMBOX implementation and registration
FBToolImplementation(	TOOLHELPMEONFACIAL__CLASS	);
FBRegisterTool		(	TOOLHELPMEONFACIAL__CLASS,
						TOOLHELPMEONFACIAL__LABEL,
						TOOLHELPMEONFACIAL__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Tool_HelpMeOnFacial::FBCreate()
{
	// Tool options
	StartSize[0] = 800;
	StartSize[1] = 600;

    int lB = 10;
	//int lS = 4;
	int lW = 100;
	int lH = 18;
	
	// Configure layout

	AddRegion( "WebPage", "WebPage",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										-lB,	kFBAttachRight,	"",	1.0,
										-lB,	kFBAttachBottom,	"",	1.0 );

	//
	SetControl( "WebPage", mHelpPage );
	
	// Configure button

	mHelpPage.Load( "https://docs.google.com/document/d/1JI3CTqlX8v_zwEqoKipyKDvSlDRJpdObXSrWGAky6OM/edit?usp=sharing" );
	
	return true;
}


/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void Tool_HelpMeOnFacial::FBDestroy()
{
	// Remove tool callbacks
	
	// Free user allocated memory
}
