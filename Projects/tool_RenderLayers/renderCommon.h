	
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: renderCommon.cpp
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

struct LayerItem;

//
// Events between layers dialog and options dialog
//
#define EVENT_EMPTY_STACK		0
#define	EVENT_LAYER_CHANGE		1

void		AskMainUIForUpdate();
const bool	IsMainUINeedUpdate(const bool reset);

void gSetCurrentLayer( LayerItem *pItem );
LayerItem *gGetCurrentLayer();

void AddNewEvent(int eventCode);
int PopEvent();
void ClearEvents();

//
// some helpful filesystem function
//
bool dirExists(const char *dirName_in);
bool CreateDirectoryTree(const char *szPathTree);

//
// helpful function for fb sdk
//
FBString ConvertTypeToString(FBSceneChangeType type);

FBGroup *FindGroupByName(const char *groupName);

FBCamera *FindCameraByName(const char *camName);

FBMaterial *FindMaterialByName(const char *matName);

FBTexture *FindTextureByName(const char *texName);

FBShader *FindShaderByName(const char *shaderName);

//
// store information about layer materials/shaders state
//

struct StaticString
{
	//! a constructor
	StaticString()
	{
		memset(&str[0], 0, sizeof(char) * 128);
	}

	void Set(const char *text, size_t len=0) {
		if (len == 0) len = strlen(text);
		memcpy_s(&str[0], 128, text, sizeof(char) * len ); 
	}

	const char* c_str() { return &str[0]; }

private:
	char		str[128];
};

struct LayerStateMaterial
{
public:
	/*
	enum FBMaterialTextureType {        
    kFBMaterialTextureEmissive,
    kFBMaterialTextureEmissiveFactor,
    kFBMaterialTextureAmbient,
    kFBMaterialTextureAmbientFactor,
    kFBMaterialTextureDiffuse,
    kFBMaterialTextureDiffuseFactor,
    kFBMaterialTextureSpecular,
    kFBMaterialTextureSpecularFactor,
    kFBMaterialTextureShiness,
    kFBMaterialTextureBump,
    kFBMaterialTextureNormalMap,
    kFBMaterialTextureTransparent,
    kFBMaterialTextureTransparentFactor,
    kFBMaterialTextureReflection,
    kFBMaterialTextureReflectionFactor,
    kFBMaterialTextureDisplacementColor
	};
	*/

	FBMaterial*					pThis;
	FBTexture*					Channels[15];		// store texture connection for each material slot

	FBString					materialName;
	FBString					texturesNames[15];

	void Clear(FBMaterial *newMaterial=nullptr)
	{
		pThis = newMaterial;
		for (int i=0; i<15; ++i)
			Channels[i] = nullptr;
	}

	void ClearTexturesNames()
	{
		for (int i=0; i<15; ++i)
			texturesNames[i] = "None";
	}

	void Store()
	{
		if (pThis)
			for (int i=0; i<15; ++i)
				Channels[i] = pThis->GetTexture( (FBMaterialTextureType) i );
	}

	void Restore()
	{
		if (pThis)
			for (int i=0; i<15; ++i)
				if (Channels[i]) pThis->SetTexture( Channels[i], (FBMaterialTextureType) i );
	}

	// FBX store/retrieve
	bool FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
	{
		if (pThis)
		{
			pFbxObject->FieldWriteC( pThis->Name );

			for (int i=0; i<15; ++i)
				pFbxObject->FieldWriteC( (Channels[i]) ? Channels[i]->Name : "None" );
		}
		else
		{
			pFbxObject->FieldWriteC( "None" );
		}
		return true;
	}
	bool FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
	{
		
		if (pStoreWhat == kAttributes)
		{
			pThis = nullptr;
			materialName = pFbxObject->FieldReadC();
			ClearTexturesNames();

			if ( strcmp(materialName, "None") != 0 ) 
			{
				FBString texName;
				for (int i=0; i<15; ++i)
				{
					texName = pFbxObject->FieldReadC();
					texturesNames[i] = texName;
				}
			}
		}
		else if (pStoreWhat == kCleanup)
		{
			pThis = FindMaterialByName( materialName );

			if (pThis)
			{
				for (int i=0; i<15; ++i)
					Channels[i] = FindTextureByName( texturesNames[i] );
			}
		}
		return true;
	}

	void CollectStateInformation( FBPropertyStringList *pList )
	{
		if (pThis)
		{
			pList->Add( FBString( "Material - ", pThis->Name ) );

			for (int i=0; i<15; ++i)
				if (Channels[i]) 
					pList->Add( FBString( " > texture - ", Channels[i]->Name ) );
		}
		else
		{
			pList->Add( "Material is empty!" );
		}
	}

	void ProcessDeleteEvent(FBComponent *pComp)
	{
		if ( pComp->Is( FBMaterial::TypeInfo ) && pComp == pThis )
		{
			Clear(nullptr);
		}
		else if (pComp->Is( FBTexture::TypeInfo ) )
		{
			for (int i=0; i<15; ++i)
				if (Channels[i] && Channels[i] == pComp) Channels[i] = nullptr; 
		}
	}
};

struct LayerStateModel
{
	FBModel							*pThis;	// pointer to the state model

	FBArrayTemplate<FBMaterial*>	materials;
	FBArrayTemplate<FBShader*>		shaders;

	FBString						modelName;
	FBStringList					materialsNames;
	FBStringList					shadersNames;

	//! a constructor
	LayerStateModel( FBModel *newModel=nullptr )
		: pThis(newModel)
	{
	}

	void Clear(FBModel *newModel=nullptr)
	{
		pThis = newModel;
		materials.Clear();
		shaders.Clear();
	}

	void Store()
	{
		if (pThis)
		{
			materials.SetCount( pThis->Materials.GetCount() );

			for (int i=0; i<pThis->Materials.GetCount(); ++i)
				materials[i] = pThis->Materials[i];

			shaders.SetCount( pThis->Shaders.GetCount() );

			for (int i=0; i<pThis->Shaders.GetCount(); ++i)
				shaders[i] = pThis->Shaders[i];
		}
	}

	void Restore()
	{
		if (pThis)
		{
			pThis->Materials.RemoveAll();
			for (int i=0; i<materials.GetCount(); ++i)
				pThis->Materials.Add(materials[i]);

			pThis->Shaders.RemoveAll();
			for (int i=0; i<shaders.GetCount(); ++i)
				pThis->Shaders.Add( shaders[i] );
		}
	}

	// FBX store/retrieve
	bool FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
	{
		if (pThis)
		{
			pFbxObject->FieldWriteC( pThis->Name );

			pFbxObject->FieldWriteI( materials.GetCount() );
			for (int i=0; i<materials.GetCount(); ++i)
				pFbxObject->FieldWriteC( materials[i]->Name );

			pFbxObject->FieldWriteI( shaders.GetCount() );
			for (int i=0; i<shaders.GetCount(); ++i)
				pFbxObject->FieldWriteC( shaders[i]->Name );
		}
		else
		{
			pFbxObject->FieldWriteC( "None" );
		}
		return true;
	}
	bool FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
	{
		if (pStoreWhat == kAttributes)
		{
			pThis = nullptr;
			modelName = pFbxObject->FieldReadC();

			if ( strcmp(modelName, "None") != 0 )
			{
				int count; 
				count = pFbxObject->FieldReadI();
				materialsNames.Clear();
				for (int i=0; i<count; ++i)
					materialsNames.Add( pFbxObject->FieldReadC() );

				count = pFbxObject->FieldReadI();
				shadersNames.Clear();
				for (int i=0; i<count; ++i)
					shadersNames.Add( pFbxObject->FieldReadC() );
			}
		}
		else
		{
#ifdef OLD_FIND_MODEL_BY_NAME
			pThis = FBFindModelByName(modelName);
#else
			pThis = FBFindModelByLabelName(modelName);
#endif
			if (pThis)
			{
				materials.SetCount( materialsNames.GetCount() );
				for (int i=0; i<materials.GetCount(); ++i)
					materials[i] = FindMaterialByName(materialsNames[i]);

				shaders.SetCount( shadersNames.GetCount() );
				for (int i=0; i<shaders.GetCount(); ++i)
					shaders[i] = FindShaderByName(shadersNames[i]);
			}
		}
		return true;
	}

	void CollectStateInformation( FBPropertyStringList *pList )
	{
		if (pThis)
		{
			char buffer[128];
			memset( buffer, 0, sizeof(char)*128);
		
			pList->Add( FBString( "Model name - ", pThis->Name ) );

			sprintf( buffer, " > model has %d materials\0", materials.GetCount() );
			pList->Add(buffer);
			for (int i=0; i<materials.GetCount(); ++i)
				pList->Add( FBString( " > material name - ", materials[i]->Name ) );

			sprintf( buffer, " > model has %d shaders\0", shaders.GetCount() );
			pList->Add(buffer);
			for (int i=0; i<shaders.GetCount(); ++i)
				pList->Add( FBString( " > shader name - ", shaders[i]->Name ) );
		}
		else
		{
			pList->Add( "Model is empty!" );
		}
	}

	void ProcessDeleteEvent(FBComponent *pComp)
	{
		if (pComp->Is( FBModel::TypeInfo ) && pComp == pThis)
		{
			Clear(nullptr);
		}
		else if (pComp->Is( FBMaterial::TypeInfo ) )
		{
			materials.RemoveIt( (FBMaterial*) pComp );
		}
		else if (pComp->Is( FBShader::TypeInfo ) )
		{
			shaders.RemoveIt( (FBShader*) pComp );
		}
	}
};

struct LayerState
{
public:
	FBArrayTemplate<LayerStateMaterial*>		materials;
	FBArrayTemplate<LayerStateModel*>			models;

	//! destructor
	~LayerState()
	{
		Clear();
	}

	void Clear()
	{
		for (int i=0; i<materials.GetCount(); ++i)
			if (materials[i]) delete materials[i];
		materials.Clear();

		for (int i=0; i<models.GetCount(); ++i)
			if (models[i]) delete models[i];
		models.Clear();
	}

	bool IsEmpty()
	{
		return (materials.GetCount() == 0 && models.GetCount() == 0);
	}

	void SetMaterialsCount(const int count)
	{
		materials.SetCount(count);
		for (int i=0; i<count; ++i)
		{
			LayerStateMaterial *pStateMaterial = new LayerStateMaterial;
			materials[i] = pStateMaterial;
		}
	}

	void SetModelsCount(const int count)
	{
		models.SetCount(count);
		for (int i=0; i<count; ++i)
		{
			LayerStateModel *pStateModel = new LayerStateModel();
			models[i] = pStateModel;
		}
	}

	void StoreModelChildren( FBModel *RootModel, FBModel *pModel )
	{
		if ( pModel != RootModel && pModel->Is( FBModel::TypeInfo ) )
		{
			LayerStateModel *pState = new LayerStateModel( pModel );
			pState->Store();
			models.Add( pState );
		}

		for (int i=0; i<pModel->Children.GetCount(); ++i)
			StoreModelChildren( RootModel, pModel->Children[i] );
	}

	void Store()
	{
		Clear();

		FBScene *pScene = FBSystem().Scene;

		materials.SetCount( pScene->Materials.GetCount() );
		for (int i=0; i<pScene->Materials.GetCount(); ++i)
		{
			LayerStateMaterial *pStateMaterial = new LayerStateMaterial;
			pStateMaterial->Clear(pScene->Materials[i]);
			pStateMaterial->Store();

			materials[i] = pStateMaterial;
		}

		StoreModelChildren( pScene->RootModel, pScene->RootModel );
	}

	void Restore()
	{
		for (int i=0; i<materials.GetCount(); ++i)
		{
			materials[i]->Restore();
		}

		for (int i=0; i<models.GetCount(); ++i)
		{
			models[i]->Restore();
		}
	}


	// FBX store/retrieve
	bool FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
	{
		pFbxObject->FieldWriteI( materials.GetCount() );
		for (int i=0; i<materials.GetCount(); ++i)
			materials[i]->FbxStore( pFbxObject, pStoreWhat );

		pFbxObject->FieldWriteI( models.GetCount() );
		for (int i=0; i<models.GetCount(); ++i )
			models[i]->FbxStore( pFbxObject, pStoreWhat );
		return true;
	}
	bool FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
	{
		if (pStoreWhat == kAttributes)
		{
			int count;
	
			count = pFbxObject->FieldReadI();
			SetMaterialsCount(count);
			for (int i=0; i<count; ++i)
				materials[i]->FbxRetrieve(pFbxObject, pStoreWhat);

			count = pFbxObject->FieldReadI();
			SetModelsCount(count);
			for (int i=0; i<count; ++i)
			{
				models[i]->FbxRetrieve(pFbxObject, pStoreWhat);
			}
		}
		else if (pStoreWhat == kCleanup)
		{
			for (int i=0; i<materials.GetCount(); ++i)
				materials[i]->FbxRetrieve( pFbxObject, pStoreWhat );

			for (int i=0; i<models.GetCount(); ++i )
				models[i]->FbxRetrieve( pFbxObject, pStoreWhat );
		}
		return true;
	}

	void CollectStateInformation( FBPropertyStringList *pList )
	{
		char buffer[128];
		memset( buffer, 0, sizeof(char)*128);
		sprintf( buffer, "state has %d materials\0", materials.GetCount() );
		pList->Add( buffer );

		for (int i=0; i<materials.GetCount(); ++i)
		{
			materials[i]->CollectStateInformation(pList);
		}

		sprintf( buffer, "state has %d models\0", models.GetCount() );
		pList->Add( buffer );

		for (int i=0; i<models.GetCount(); ++i)
		{
			models[i]->CollectStateInformation(pList);
		}
	}

	void ProcessDeleteEvent(FBComponent *pComp)
	{
		for (int i=0; i<materials.GetCount(); ++i)
		{
			materials[i]->ProcessDeleteEvent(pComp);
			if (materials[i]->pThis == nullptr) {
				materials.RemoveAt(i);
				break;
			}
		}
		for (int i=0; i<models.GetCount(); ++i)
		{
			models[i]->ProcessDeleteEvent(pComp);
			if (models[i]->pThis == nullptr) {
				models.RemoveAt(i);
				break;
			}
		}
	}
};

//
// store infromation about the layer
//
struct LayerItem
{

public:
	// layer options
	bool					FilePathOverride;
	FBString				FilePath;
	bool					FileNameOverride;
	FBString				FileName;
	
	bool					TimeOverride;
	FBTime					StartTime;
	FBTime					StopTime;
	FBTime					StepTime;

	bool					PictureOverride;
	FBCameraResolutionMode	PictureMode;
	bool					FieldOverride;
	FBVideoRenderFieldMode	FieldMode;
	bool					PixelOverride;
	FBVideoRenderDepth		PixelMode;

	bool					FormatOverride;
	FBString				Format;
	FBString				Codec;

	bool					CamerasOverride;
	FBArrayTemplate<FBCamera*> Cameras;
	FBStringList			CamerasNames;	// ! Only for retreving data to restore camera pointers

	bool					ScriptOverride;
	FBString				Script;
	FBString				ScriptPost;

	bool					AudioOverride;
	bool					Audio;

	bool					DisplayOverride;
	FBVideoRenderViewingMode	DisplayMode;
	bool					ShowTime;
	bool					ShowSafe;
	bool					ShowLabel;
	bool					Antialiasing;
	bool					Matte;	// matte objects out of a layer group

	//
	// Default - set all parameter to their default values
	//
	void	Default();

	//
	// CheckOptions - check if filepath, filename, cameras, etc. are correct
	//
	void	CheckOptions(LayerItem *pMasterLayer);

public:
	bool			Master;	// master layer (couldnt delete it)
	bool			Renderable; // layer checkbox

	char			Name[64];

	FBGroup			*Group;	// assigned group of objects in layer
	FBString		GroupName;	// ! only for retreiving operation

	// connected UI element
	FBTreeNode	*Node;

	// materials and shaders state
	LayerState		State;
	bool	IsStateEmpty() { return State.IsEmpty(); }

	//! a constructor
	LayerItem(bool master=false, const char *name="Layer", FBTreeNode *node=nullptr)
		: Master(master)
		, Node(node)
	{
		memset( Name, 0, sizeof(char) * 64);
		strcpy( Name, name );

		Renderable = true;
		Group = nullptr;

		Default();
	}

	bool IsMaster() {return Master; }

	void SetName(const char *newName)
	{
		memset( Name, 0, sizeof(char) * 64 );
		memcpy_s( Name, 64, newName, strlen(newName) );
	}
	void SetGroup(const char *groupName)
	{
		Group = nullptr;
		
		FBScene *pScene = FBSystem().Scene;
		for (int i=0; i<pScene->Groups.GetCount(); ++i)
		{
			FBString str(pScene->Groups[i]->Name);
			if ( strcmp(groupName, str) == 0 )
			{
				Group = pScene->Groups[i];
				break;
			}
		}
	}

	// FBX store/retrieve
	bool FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );
	bool FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

	// display stat information
	void CollectStateInformation( FBPropertyStringList *pList )
	{
		if ( IsStateEmpty() )
		{
			pList->Add( "Layer has no state information!" );
		}
		else
		{
			State.CollectStateInformation( pList );
		}
	}

	void ProcessDeleteEvent(FBComponent *pComp)
	{
		State.ProcessDeleteEvent(pComp);
	}
};