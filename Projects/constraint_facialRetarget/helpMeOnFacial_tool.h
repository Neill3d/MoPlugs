
#pragma once

/**	\file	helpMeOnFacial_tool.h
*	Template for tool development.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define TOOLHELPMEONFACIAL__CLASSNAME	Tool_HelpMeOnFacial
#define TOOLHELPMEONFACIAL__CLASSSTR	"Tool_HelpMeOnFacial"

/**	Tool template.
*/
class Tool_HelpMeOnFacial : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( Tool_HelpMeOnFacial, FBTool );

public:
	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

private:

	FBWebView		mHelpPage;

};