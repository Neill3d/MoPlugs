#ifndef __IMP_GEOM_CACHE_H__
#define __IMP_GEOM_CACHE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: impgeomcache.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

/**	Custom scene import menu item.
*	With this class created, a new menu item will appear,
*	enabling the developer to overload the execution functions
*	to import their custom scene format.
*/
class ORMenuItemImportGeomCache : public FBMenuItem
{
	//--- FiLMBOX declaration.
	FBMenuItemDeclare( ORMenuItemImportGeomCache, FBMenuItem );

public:
	//--- Overloaded FBMenuItem functions.
	virtual bool FBCreate();							//!< Overloaded FBMenuItem configuration function.
	virtual bool Execute(const char *pFilename=NULL);		//!< Overloaded FBMenuItem execution function.

	void		EventResetScene( HISender pSender, HKEvent pEvent );

private:

	FBApplication		mApplication;

	bool				mUseDynamicTextures;

	bool				mAskForStoragePath;
	FBString			mStoragePath;

};

#endif /* __ORIMPEXP_MENU_MENU_H__ */
