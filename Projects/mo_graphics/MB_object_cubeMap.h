
// object to store opengl cubemap textures

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_object_cubeMap.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <GL\glew.h>
#include <map>
#include "graphics\OGL_Utils_MoBu.h"

#define OBJECT_CUBEMAP__CLASSNAME			ObjectCubeMap
#define OBJECT_CUBEMAP__CLASSSTR			"Object CubeMap"

enum FBCubeMapType
{
	kFBCubeMapFile,
	kFBCubeMapImages,
	kFBCubeMapPanorama,
	kFBCubeMapDynamic
};

const char * FBPropertyBaseEnum<FBCubeMapType>::mStrings[] = {
	"CubeMap File",
	"Image Stack",
	"Panorama",
	"Dynamic CubeMap",
	0
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectCubeMap

class ObjectCubeMap : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(ObjectCubeMap, FBUserObject)
	FBDeclareUserObject(ObjectCubeMap);

public:
	//! a constructor
	ObjectCubeMap(const char *pName = NULL, HIObject pObject=NULL);

	virtual bool FBCreate() override;
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

	void OnPerFrameRenderingPipelineCallback    (HISender pSender, HKEvent pEvent);
	void OnSystemIdle (HISender pSender, HKEvent pEvent);

	static void SetReLoadShader(HIObject object, bool value);

	static void SetClearAction(HIObject object, bool value);
	static void SetLoadAsAction(HIObject object, bool value);
	static void SetReloadAction(HIObject object, bool value);

	static int GetCubeMapId(HIObject object);

public:

	FBPropertyBool							Active;
	//FBPropertyAnimatableDouble			Weight;		// percentage of an effect (like opacity factor)
	FBPropertyAction						ReLoadShader;

	FBPropertyBool							Test;

	FBPropertyBaseEnum<FBCubeMapType>		Type;

	// model which normal buffer we will drive
	FBPropertyString						FileName;
	FBPropertyBool							LoadCompressed;
	FBPropertyBool							LoadMipMaps;
	FBPropertyBool							Loaded;
	FBPropertyString						Status;

	FBPropertyAction						Clear;
	FBPropertyAction						LoadAs;
	FBPropertyAction						Reload;

	//
	FBPropertyListObject				TextureFront;
	FBPropertyListObject				TextureBack;
	FBPropertyListObject				TextureTop;
	FBPropertyListObject				TextureBottom;
	FBPropertyListObject				TextureLeft;
	FBPropertyListObject				TextureRight;

	FBPropertyAction					ImportSideTextures;
	FBPropertyAction					CreateFromTextureStack;

	// convert panorama (equarectangle) texture into cubemap
	FBPropertyListObject				TextureRect;
	FBPropertyBool						RenderFromRect;

	// output info about loaded cubemap
	FBPropertyInt							Dimention;
	FBPropertyInt							MipLevels;
	FBPropertyString						Format;
	FBPropertyString						MemorySize;	// coast in GPU memory

	// models where we grab normal buffers
	FBPropertyInt							CubeMapId;

	// possible feature to output cubemap->equarectangle texture on fly
	FBPropertyBool							RenderToRect;
	FBPropertyListObject					RectTexture;	// ?! video or texture ?!


public:

	
	//
	// UPDATE RESOURCE FOR COMPONENTS
	//

	// update when ogl context has changed
	void ChangeContext();

	void ChangeGlobalSettings();
	
	

protected:
	bool	mNeedProgramReload;

	FBSystem		mSystem;

public:
	
	void DoReloadShader();

	void OnClearClick();
	void OnLoadAsClick();
	void OnReloadClick();
	void OnImportSideTexturesClick(bool showDialog);
	void OnCreateFromTextureStackClick();

	const OGLCubeMapMOBU *GetCubeMapPtr() const {
		return &mCubeMap;
	}

protected:

	bool				mNeedUpdate;
	OGLCubeMapMOBU		mCubeMap;


	void		Update();

	bool	UpdateFromFile();
	void	UpdateFromImages();
	void	UpdateFromPanorama();

};
