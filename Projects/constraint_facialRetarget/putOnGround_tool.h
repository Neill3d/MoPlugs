#ifndef __ORTOOL_TEMPLATE_TOOL_H__
#define __ORTOOL_TEMPLATE_TOOL_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: putOnGroud_tool.h
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

//--- Registration define
#define TOOLPUTONGROUND__CLASSNAME	Tool_PutOnGround
#define TOOLPUTONGROUND__CLASSSTR	"Tool_PutOnGround"

/**	Tool template.
*/
class Tool_PutOnGround : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( Tool_PutOnGround, FBTool );

public:
	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

private:
	void		EventContainerDblClick( HISender pSender, HKEvent pEvent );
	void		EventContainerDragAndDrop( HISender pSender, HKEvent pEvent );
	
	void		EventButtonProjectClick( HISender pSender, HKEvent pEvent );

private:

	FBLabel					mLabelInfo;
	FBVisualContainer		mContainerGround;
	FBButton				mButtonOrient;
	FBButton				mButtonProject;

	void ProjectModel( FBModel *pModel, FBModelList *groundModels );

};

#endif /* __FB_TOOL_TEMPLATE_TOOL_H__ */
