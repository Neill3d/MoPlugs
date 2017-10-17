
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: render_manager.h
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

//--- Registration defines
#define RENDERMANAGER__CLASSNAME RenderManager
#define RENDERMANAGER__CLASSSTR  "RenderManager"

enum ERenderMode
{
	eRenderModeNone,
	eRenderModeWindow,
	eRenderModeOffscreen,
	eRenderModeLayers
};

/** Custom Manager Template.
*/
class RenderManager : public FBCustomManager
{
    //--- FiLMBOX box declaration.
    FBCustomManagerDeclare( RenderManager );

public:
    virtual bool FBCreate();        //!< FiLMBOX creation function.
    virtual void FBDestroy();       //!< FiLMBOX destruction function.

    virtual bool Init();
    virtual bool Open();
    virtual bool Clear();
    virtual bool Close();


	void	EventIdle       ( HISender pSender, HKEvent pEvent );
	void	EventFileOpenCompleted ( HISender pSender, HKEvent pEvent );
private:

	FBApplication	mApp;
	FBSystem		mSystem;
};
