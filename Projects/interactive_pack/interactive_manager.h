#ifndef __INTERACTIVE_MANAGER_H__
#define __INTERACTIVE_MANAGER_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: interactive_manager.h
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
#define ORCUSTOMMANAGER_INTERACTIVE__CLASSNAME ORCustomManager_Interactive
#define ORCUSTOMMANAGER_INTERACTIVE__CLASSSTR  "ORCustomManager_Interactive"

/** Custom Manager Template.
*/
class ORCustomManager_Interactive : public FBCustomManager
{
    //--- FiLMBOX box declaration.
    FBCustomManagerDeclare( ORCustomManager_Interactive );

public:
    virtual bool FBCreate();        //!< FiLMBOX creation function.
    virtual void FBDestroy();       //!< FiLMBOX destruction function.

    virtual bool Init();
    virtual bool Open();
    virtual bool Clear();
    virtual bool Close();

public:

	void	OnRender(HISender pSender, HKEvent pEvent);
	void	OnConnectionData(HISender pSender, HKEvent pEvent);
	void	OnSceneChange(HISender pSender, HKEvent pEvent);
	void	OnSystemIdle(HISender pSender, HKEvent pEvent);

private:

	FBSystem			mSystem;
};

#endif /* __INTERACTIVE_MANAGER_H__ */
