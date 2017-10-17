#ifndef __ORCUSTOMMANAGER_TEMPLATE_MANAGER_H__
#define __ORCUSTOMMANAGER_TEMPLATE_MANAGER_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filesizechecker_manager.h
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
#define ORCUSTOMMANAGER_TEMPLATE__CLASSNAME FileSizeChecker_Manager
#define ORCUSTOMMANAGER_TEMPLATE__CLASSSTR  "FileSizeChecker_Manager"

/** Custom Manager Template.
*/
class FileSizeChecker_Manager : public FBCustomManager
{
    //--- FiLMBOX box declaration.
    FBCustomManagerDeclare( FileSizeChecker_Manager );

public:
    virtual bool FBCreate();        //!< FiLMBOX creation function.
    virtual void FBDestroy();       //!< FiLMBOX destruction function.

    virtual bool Init();
    virtual bool Open();
    virtual bool Clear();
    virtual bool Close();

	void EventFileSaveCompleted( HISender pSender, HKEvent pEvent );

private:
};

#endif /* __ORCUSTOMMANAGER_TEMPLATE_MANAGER_H__ */
