
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: persistent_data.h
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
#include "renderCommon.h"

//! This class will make sure that the data related to the tool is persistent.
class RenderLayersData : public FBUserObject {
    //--- FiLMBOX declaration.
    FBClassDeclare(RenderLayersData, FBUserObject)
    FBDeclareUserObject(RenderLayersData);

public:
    RenderLayersData( const char *pName=NULL, HIObject pObject=NULL );

    //--- FiLMBOX Construction/Destruction,
    virtual bool FBCreate();        //!< FiLMBOX Creation function.
    virtual void FBDestroy();       //!< FiLMBOX Destruction function.

    /**	Store and Retrieve function that can be overloaded.
	*	\param	pFbxObject	FBX Object that is used to communicate I/O operations.
	*	\param	pStoreWhat	Which attributes are currently stored/retrieved.
	*/
	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

	void	EventFileNew	( HISender pSender, HKEvent pEvent );
	void	EventFileOpen	( HISender pSender, HKEvent pEvent );
	void	EventFileOpenCompleted	( HISender pSender, HKEvent pEvent );

public:
	
    FBArrayTemplate<LayerItem*>		Layers;

	void		Clear();
	void		SetCurrentLayer(const int layer);
};

