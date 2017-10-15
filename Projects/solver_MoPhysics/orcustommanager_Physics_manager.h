#ifndef __ORCUSTOMMANAGER_PHYSICS_MANAGER_H__
#define __ORCUSTOMMANAGER_PHYSICS_MANAGER_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orcustommanager_Physics_manager.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define ORCUSTOMMANAGER_TEMPLATE__CLASSNAME ORCustomManager_Physics
#define ORCUSTOMMANAGER_TEMPLATE__CLASSSTR  "ORCustomManager_Physics"

/** Custom Manager Template.
*/
class ORCustomManager_Physics : public FBCustomManager
{
    //--- FiLMBOX box declaration.
    FBCustomManagerDeclare( ORCustomManager_Physics );

public:
    virtual bool FBCreate();        //!< FiLMBOX creation function.
    virtual void FBDestroy();       //!< FiLMBOX destruction function.

    virtual bool Init();
    virtual bool Open();
    virtual bool Clear();
    virtual bool Close();

	/** Demo how to utilize the per frame callback at pipeline's critical stage/timing in the application main loop.
    */
    void OnPerFrameRenderingPipelineCallback    (HISender pSender, HKEvent pEvent);

private:

	FBSystem			mSystem;
};

#endif /* __ORCUSTOMMANAGER_PHYSICS_MANAGER_H__ */
