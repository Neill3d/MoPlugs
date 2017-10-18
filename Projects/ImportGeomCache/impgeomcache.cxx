
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: impgeomcache.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declarations
#include "impgeomcache.h"
#include "gpucache_loader.h"
#include "gpucache_visitor_mobu.h"

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

//--- Library declaration
FBLibraryDeclare( impgeomcache )
{
	FBLibraryRegister( ORMenuItemImportGeomCache );
}
FBLibraryDeclareEnd;

//--- FiLMBOX implementation and registration
// Note: These lines can not precede the above library declaration.
FBMenuItemImplementation( ORMenuItemImportGeomCache );
FBRegisterMenuItem		( ORMenuItemImportGeomCache );

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()
{

	// Menu item activation
	FBMenuItemActivation( ORMenuItemImportGeomCache );
	return true;
}
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease()
{
    ORMenuItemImportGeomCache* lMenu = NULL;

    // Get a handle onto the menu item.
    FBMenuItemHandle( ORMenuItemImportGeomCache, lMenu );

    // Cleanup the menu entries.
    lMenu->FBDelete();

    return true;
}


/************************************************
 *	Scene import configuration.
 ************************************************/
bool ORMenuItemImportGeomCache::FBCreate()
{
	// Configure menu item
	Type		= kFBMenuItemMotionImport;
	Extension	= "*.xml";
	Label		= "GPU Geometry Cache File";
	Description	= "GPU Geometry Cache File";

	mUseDynamicTextures = false;
	mAskForStoragePath = false;
	mStoragePath = "C:\\Temp";

	glewInit();

	FBConfigFile	pConfig("@ImportGeomCache.txt");

	char buffer[256];
	auto fn_settext = [&buffer] (const char *text) {
		memset (buffer, 0, sizeof(char) * 256);
		sprintf_s(buffer, 256, text);
	};
	

	const char *ptrbuf = buffer;

	fn_settext("No");
	ptrbuf = buffer;
	if (true == pConfig.GetOrSet( "Common", "DynamicTextures", ptrbuf, "Do we cache data in video memory ? (Yes / No)" ) )
		mUseDynamicTextures = (strstr(ptrbuf, "Yes") != nullptr);

	fn_settext("No");
	ptrbuf = buffer;
	if (true == pConfig.GetOrSet( "Textures", "AskForStoragePath", ptrbuf, "Promt a dialog for choosing storage for textures ? (Yes / No)" ) )
		mAskForStoragePath = (strstr(ptrbuf, "Yes") != nullptr);

	fn_settext("C:\\Temp");
	ptrbuf = buffer;
	if (true == pConfig.GetOrSet( "Textures", "StoragePath", ptrbuf, "storage path for storing imported images" ) )
		mStoragePath = ptrbuf;

	mApplication.OnFileNew.Add( this, (FBCallback) &ORMenuItemImportGeomCache::EventResetScene );

	return true;
}


/************************************************
 *	Import: Execution function
 ************************************************/
bool ORMenuItemImportGeomCache::Execute(const char *pFilename)
{
	// Execute importation here.
	CMoBuVisitor visitor(mUseDynamicTextures, mAskForStoragePath, mStoragePath);
	CGPUCacheLoader loader;

	return loader.Load(pFilename, &visitor);
}

void ORMenuItemImportGeomCache::EventResetScene( HISender pSender, HKEvent pEvent )
{

}

/**
*	\mainpage	Custom Menu Item
*	\section	intro	Introduction
*	This plugin is a good example for the creation of generic
*	import/export functions, without the development required in
*	order to develop a tool for FiLMBOX. In order to use this plugin
*	the developer needs only to derive a special import/export class,
*	(derivative) of the FBMenuItem class.
*	<p>
*	With this step done, the only functionality needed to implement
*	is to be found in the Execute() function, which will execute the
*	actual import/export functionality.
*/
