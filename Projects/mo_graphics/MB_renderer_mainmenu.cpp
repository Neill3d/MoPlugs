
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_renderer_mainmenu.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "MB_renderer.h"
#include "stringUtils.h"
#include "IO\FileUtils.h"
#include "IO\tinyxml.h"
#include "ShaderModelInfo.h"

#include "shared_content.h"

#include "GPUCaching_shader.h"
#include "GPUCaching_model_display.h"

#include "gpucache_saver_mobu.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/*

	1 - not light
	2 - not camera
	3 - not null
	4 - not skeleton
	5 - not path object

	b1 - should be visible
	b2 - not deformable
	b3 - not constrained
	b4 - geometry exists

	c1 - not animated on tr, rt, sc
	c2 - parent heirarchy should be static
	c3 - no animation textures (sequences)
	c4 - no additive shaders assignment
*/

bool CheckIfStatic(FBModel *pModel)
{
	const bool IsDeformable = pModel->IsDeformable;
	if (IsDeformable == true) return false;
	
	const bool IsConstrained = pModel->IsConstrained;	
	if (IsConstrained == true) return false;

	// c1 - not animated
	FBAnimationNode *pNode;
	
	pNode = pModel->Translation.GetAnimationNode();
	if (pNode && pNode->Nodes.GetCount() == 3)
		if (pNode->Nodes[0]->KeyCount > 1 || pNode->Nodes[1]->KeyCount > 1 || pNode->Nodes[2]->KeyCount > 1) return false;
	
	pNode = pModel->Rotation.GetAnimationNode();
	if (pNode && pNode->Nodes.GetCount() == 3)
		if (pNode->Nodes[0]->KeyCount > 1 || pNode->Nodes[1]->KeyCount > 1 || pNode->Nodes[2]->KeyCount > 1) return false;
	
	pNode = pModel->Scaling.GetAnimationNode();
	if (pNode && pNode->Nodes.GetCount() == 3)
		if (pNode->Nodes[0]->KeyCount > 1 || pNode->Nodes[1]->KeyCount > 1 || pNode->Nodes[2]->KeyCount > 1) return false;

	// c2 - check parent heirarchy
	if (pModel->Parent)
		if (CheckIfStatic(pModel->Parent) == false)
			return false;

	return true;
}

bool CheckTexture(FBTexture *pTexture)
{
	if (pTexture)
	{
		FBVideo *pVideo = pTexture->Video;

		if (pVideo)
			if ( FBIS(pVideo, FBVideoClipImage) )
			{
				FBVideoClipImage *pVideoImage = (FBVideoClipImage*) pVideo;
				if (pVideoImage->ImageSequence == true || (pVideoImage->StartFrame != pVideoImage->StopFrame))
					return false;
			}
	}

	return true;
}

bool CheckModelMaterials(FBModel *pModel)
{
	const int numberOfMaterials = pModel->Materials.GetCount();
	for (int i=0; i<numberOfMaterials; ++i)
	{
		FBMaterial *pMaterial = pModel->Materials[i];

		if ( CheckTexture( pMaterial->GetTexture() ) == false )
			return false;
	}

	return true;
}

bool CheckShader (FBShader *pShader)
{
	if (pShader)
	{
		FBProperty *lProp = pShader->PropertyList.Find( "Transparency" );
		if (lProp)
		{
			const int iValue = lProp->AsInt();

			FBAlphaSource alphaSource = (FBAlphaSource) iValue;

			if (alphaSource > kFBAlphaSourceAccurateAlpha)
				return false;
		}
	}

	return true;
}

bool CheckModelShaders(FBModel *pModel)
{
	const int numberOfShaders = pModel->Shaders.GetCount();
	for (int i=0; i<numberOfShaders; ++i)
	{
		FBShader *pShader = pModel->Shaders[i];

		if (CheckShader(pShader) == false)
			return false;
	}

	return true;
}

void SelectIfStatic(FBModel *pModel)
{
	bool select = CheckIfStatic(pModel);
	pModel->Selected = select;

	for (int i=0; i<pModel->Children.GetCount(); ++i)
	{
		SelectIfStatic( pModel->Children[i] );
	}
}

bool AreNodesConnected(FBPlug *pA, FBPlug *pB)
{
	int dstCount = pB->GetDstCount();
	for (int j=0; j<dstCount; ++j)
	{
		FBPlug *pDst = pB->GetDst(j);

		if ((pA == pDst) || (AreNodesConnected(pA, pDst) == true))
			return true;
	}
	return false;
}



void MoRendererCallback::SelectStaticObjects()
{
	FBScene *pScene = mSystem.Scene;
	FBModel *pRoot = pScene->RootModel;
	const int count = pScene->Components.GetCount();

	const int modelType = FBModel::TypeInfo;
	const int lightType = FBLight::TypeInfo;
	const int pathType = FBModelPath3D::TypeInfo;
	const int cameraType = FBCamera::TypeInfo;
	const int nullType = FBModelNull::TypeInfo;

	FBBeginChangeAllModels();

	for (int i=0; i<count; ++i)
	{
		FBComponent *pComponent = pScene->Components[i];
		if (pComponent == pRoot)
			continue;

		pComponent->Selected = false;

		const int id = pComponent->GetTypeId();

		if (id == modelType && id != lightType && id != pathType && id != cameraType && id != nullType)
		{
			FBModel *pModel = (FBModel*) pComponent;
			if (pModel->IsVisible() == false) 
				continue;

			if (CheckModelMaterials(pModel) == false)
				continue;

			if (CheckModelShaders(pModel) == false)
				continue;

			FBGeometry *pGeometry = pModel->Geometry;
			if (pGeometry == nullptr)
				continue;

			pComponent->Selected = CheckIfStatic( (FBModel*) pComponent );
		}
	}

	FBEndChangeAllModels();

	/*
	for (int i=0; i<count; ++i)
	{
		FBComponent *pComponent = pScene->Components[i];
		if (pComponent != pRoot && FBIS( pComponent, FBModel ) )
			pComponent->Selected = CheckIfStatic( (FBModel*) pComponent );
	}
	*/
}

void MoRendererCallback::ExportGPUCache()
{
	bool modeOverride = true;	// override or merge mode for exporting ?

	// selection prefer to be static
	FBModelList pList;
	FBGetSelectedModels( pList );

	int count = pList.GetCount();
	for (int i=count-1; i>=0; --i)
	{
		bool isStatic = CheckIfStatic(pList[i]);
		if (isStatic == false) pList.RemoveAt(i);
	}

	if (pList.GetCount() == 0)
	{
		FBMessageBox( "Export GPU Cache", "Please select some static objects in the scene", "Ok" );
		return;
	}

	FBString fullFileName("");
	
	FBFilePopup lDialog;
	lDialog.Style = kFBFilePopupSave;
	lDialog.Filter = "*.xml";

	if (lDialog.Execute() )
	{
		fullFileName = lDialog.FullFilename;
		
	}
	
	if (fullFileName != "")
	{
	
		FBString fileName(ExtractFileName(fullFileName));
		ChangeFileExt(fileName, FBString(""));
		FBString filePath(ExtractFilePath(fullFileName));
		filePath = filePath + "\\";
		filePath = filePath + fileName;

		CGPUCacheSaverQueryMOBU	fbQuery(pList);
		CGPUCacheSaver	saver;

		bool lSuccess = saver.Save( fullFileName, &fbQuery );

		/*
		WriteObjectsToXML(	fullFileName, 
							FBString(filePath + "_Geometry.pck"), 
							FBString(filePath + "_Textures.pck"), 
							pList );
*/
		
		FBString lMessage( "Export is Done!" );
		if (false == lSuccess)
			lMessage = "Failed to Export Objects!";

		FBMessageBox( "GPU Caching Export", lMessage, "Ok" );

	}
}

void MoRendererCallback::ImportGPUCache()
{
	// if shader or mesh with shader selected, then update it's cache
	//   otherwise create new objects and assign a shader
	FBModelList pList;
	FBGetSelectedModels( pList );

	// create a new gpu cache shader

	FBComponent *pNewComponent = FBCreateObject(	"FbxStorable/Shader", 
													"GPUCache", 
													"GPU Cache Shader" ) ;

	bool status = false;

	if (pNewComponent)
	{
		FBProperty *pProp = pNewComponent->PropertyList.Find( "Load" );
		if (pProp)
		{
			pProp->SetInt( 1 );
			status = true;
		}

		status = ((ORShaderGPUCache*) pNewComponent)->Loaded;
	}

	
	if (status == true)
	{
		ORModelGPUCache *pModel = new ORModelGPUCache( "CachedWorld" );
		pModel->Show = true;
		pModel->Pickable = false;

		if (pNewComponent) {
			pModel->Shaders.Add( pNewComponent );

			pModel->SetMatrix( ((ORShaderGPUCache*) pNewComponent)->GetLoadMatrix() );

		}
	}
	else
	{
		if (pNewComponent)
			pNewComponent->FBDelete();
	}

	FBMessageBox( "GPU Caching", (status) ? "Import is Done!" : "Failed to import!", "Ok" );
}


void MoRendererCallback::SelectGPUShader()
{
	FBScene *pScene = mSystem.Scene;

	int count = pScene->Shaders.GetCount();
	for (int i=0; i<count; ++i)
	{
		FBShader *pShader = pScene->Shaders[i];

		if (FBIS(pShader, ORShaderGPUCache) )
		{
			pShader->Selected = true;
		}
	}
}

void MoRendererCallback::SelectGPUObject()
{
	FBScene *pScene = mSystem.Scene;

	int count = pScene->Shaders.GetCount();
	for (int i=0; i<count; ++i)
	{
		FBShader *pShader = pScene->Shaders[i];

		if (FBIS(pShader, ORShaderGPUCache) )
		{
			int dstCount = pShader->GetDstCount();
			for (int j=0; j<dstCount; ++j)
			{
				FBPlug *pPlug = pShader->GetDst(j);
				if (FBIS(pPlug, FBModel))
				{
					((FBModel*)pPlug)->Selected = true;
				}
			}
		}
	}
}

void MoRendererCallback::UpdateGPUShader()
{
	FBScene *pScene = mSystem.Scene;
	FBShader *pShaderONE = nullptr;

	int count = pScene->Shaders.GetCount();
	for (int i=0; i<count; ++i)
	{
		FBShader *pShader = pScene->Shaders[i];

		if (FBIS(pShader, ORShaderGPUCache) )
		{
			if (pShaderONE == nullptr || pShader->Selected) pShaderONE = pShader;
		}
	}

	if (pShaderONE) ((ORShaderGPUCache*) pShaderONE)->Reload();
}

void MoRendererCallback::LaunchGPUCacheScene()
{
	FBScene *pScene = mSystem.Scene;
	FBShader *pShaderONE = nullptr;

	int count = pScene->Shaders.GetCount();
	for (int i=0; i<count; ++i)
	{
		FBShader *pShader = pScene->Shaders[i];

		if (FBIS(pShader, ORShaderGPUCache) )
		{
			if (pShaderONE == nullptr || pShader->Selected) pShaderONE = pShader;
		}
	}

	if (pShaderONE)
	{
		((ORShaderGPUCache*) pShaderONE)->LaunchSourceFile();
	}
}

void MoRendererCallback::CheckFreeGPUMemory()
{
	// NVX_gpu_memory_info

	// DONE: check if we have enough free video memory to save everything in compressed format
	// mem returns in Kb
	int memInfo[4] = {0, 0, 0, 0};
	glGetIntegerv( GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, memInfo );

	char buffer[128];
	sprintf_s( buffer, 128, "avaliable free memory - %d (Kb)\n", memInfo[0] );

	FBMessageBox( "GPU Memory", buffer, "Ok" );
}

void MoRendererCallback::ConvertAllBoxIntoGPUCacheObject()
{
	FBScene *pScene = mSystem.Scene;
	FBShader *pShaderONE = nullptr;

	std::vector<FBModel*>	connections;

	int count = pScene->Shaders.GetCount();
	connections.reserve(count);
	for (int i=0; i<count; ++i)
	{
		FBShader *pShader = pScene->Shaders[i];

		if (FBIS(pShader, ORShaderGPUCache) )
		{
			int dstCount = pShader->GetDstCount();
			for (int j=0; j<dstCount; ++j)
			{
				FBPlug *pPlug = pShader->GetDst(j);
				if ( FBIS(pPlug, FBModel) && !FBIS(pPlug, ORModelGPUCache) && !FBIS(pPlug, FBScene) )
				{
					connections.push_back( (FBModel*) pPlug );
				}
			}
		}
	}

	//
	
	for (auto iter=begin(connections); iter!=end(connections); ++iter)
	{
		FBModel *pSrcModel = *iter;
		if (pSrcModel == nullptr)
			continue;

		FBModel *pParent = pSrcModel->Parent;

		FBString srcName( pSrcModel->LongName );
		pSrcModel->Name = "temproaryGPUBoxName";

		ORModelGPUCache *pModel = new ORModelGPUCache( "newGPUCacheModel" );
		pModel->LongName = srcName;

		if (pModel) {

			pModel->Show = pSrcModel->Show;
			pModel->Pickable = pSrcModel->Pickable;
			//pModel->Visibility = pSrcModel->Visibility;

			if (pSrcModel->Visibility.IsAnimated() )
			{
				pModel->Visibility.SetAnimated(true);
				pModel->Visibility.GetAnimationNode()->FCurve->KeyReplaceBy( *pSrcModel->Visibility.GetAnimationNode()->FCurve );
			}
			else
			{
				bool vis = pSrcModel->Visibility.AsInt() > 0;
				pModel->Visibility.SetData( &vis );
			}

			pModel->Parent = pSrcModel->Parent;

			for (int i=0, count=pSrcModel->Shaders.GetCount(); i<count; ++i)
				pModel->Shaders.Add( pSrcModel->Shaders[i] );
			pSrcModel->Shaders.RemoveAll();

			FBMatrix matrix;
			pSrcModel->GetMatrix(matrix);
			pModel->SetMatrix( matrix );

			//
			// groups and sets

			for (int i=0, count=pScene->Groups.GetCount(); i<count; ++i)
			{
				FBGroup *pGroup = pScene->Groups[i];
				if (pGroup->Contains(pSrcModel) )
				{
					pGroup->Items.Add(pModel);
				}
			}
			for (int i=0, count=pScene->Sets.GetCount(); i<count; ++i)
			{
				FBSet *pSet = pScene->Sets[i];
				if (pSet->Contains(pSrcModel) )
				{
					pSet->Items.Add(pModel);
				}
			}
		}
	}
	
	
	FBBeginChangeAllModels();
	
	for (auto iter=begin(connections); iter!=end(connections); ++iter)
	{
		FBModel *pSrcModel = *iter;
		if (pSrcModel == nullptr)
			continue;

		pSrcModel->FBDelete();
	}

	FBEndChangeAllModels();
	
}

void MoRendererCallback::CreateMenu()
{

	// add GPU Caching menu

	//FBMenuManager *pMenuManager = new FBMenuManager();
	FBGenericMenuItem *pMenuItem = mMenuManager.InsertAfter(nullptr, "Window", "GPU Caching");
	if (pMenuItem)
	{
		mGPUMenu = pMenuItem->Menu;
		mGPUMenu->OnMenuActivate.Add( this, (FBCallback) &MoRendererCallback::OnGPUMenu );
		if (mGPUMenu)
		{
			mGPUMenu->InsertLast( "Save static objects to file", 0 );
			mGPUMenu->InsertLast( "Select all static objects in the scene", 1 );
			mGPUMenu->InsertLast( "Import a GPU Cache", 2 );
			mGPUMenu->InsertLast( "", 3 );

			// select first, select next, select all
			FBGenericMenu *pSelectShaderMenu = new FBGenericMenu();
			if (pSelectShaderMenu)
			{
				pSelectShaderMenu->InsertLast( "Select All", 100 );
				pSelectShaderMenu->InsertLast( "Select First", 101 );
				pSelectShaderMenu->InsertLast( "Select Next", 102 );

				pSelectShaderMenu->OnMenuActivate.Add( this, (FBCallback) &MoRendererCallback::OnGPUMenu );
			}
			mGPUMenu->InsertLast( "Select a Geometry Cache Shader", 4, pSelectShaderMenu );
			
			//
			FBGenericMenu *pSelectModelMenu = new FBGenericMenu();
			if (pSelectModelMenu)
			{
				pSelectModelMenu->InsertLast( "Select All", 200 );
				pSelectModelMenu->InsertLast( "Select First", 201 );
				pSelectModelMenu->InsertLast( "Select Next", 202 );

				pSelectModelMenu->OnMenuActivate.Add( this, (FBCallback) &MoRendererCallback::OnGPUMenu );
			}
			mGPUMenu->InsertLast( "Select a Geometry Cache Object", 5, pSelectModelMenu );
			
			mGPUMenu->InsertLast( "", 6 );
			mGPUMenu->InsertLast( "Update a Geometry Cache", 7 );
			mGPUMenu->InsertLast( "", 8 );
			mGPUMenu->InsertLast( "Launch a Cache Source File", 9 );
			mGPUMenu->InsertLast( "", 10 );
			mGPUMenu->InsertLast( "Check Free GPU Memory", 11 );
			mGPUMenu->InsertLast( "", 12 );
			mGPUMenu->InsertLast( "Update Scene", 13 );
		}
	}

}

void MoRendererCallback::OnGPUMenu( HISender, HKEvent e )
{
	FBEventMenu pEvent(e);
	const int id = pEvent.Id;

	switch( id )
	{
	case 0:
		ExportGPUCache();
		break;
	case 1:
		SelectStaticObjects();
		break;
	case 2:
		ImportGPUCache();
		break;
	case 4: // select gpu shader
		SelectGPUShader();
		break;
	case 5: // select gpu model
		SelectGPUObject();
		break;
	case 7: // update gpu shader
		UpdateGPUShader();
		break;
	case 9: // DONE: open mobu with source filename
		LaunchGPUCacheScene();
		break;
	case 11:
		CheckFreeGPUMemory();
		break;
	case 13:
		//mGPUScene->AskForEvaluateModels();
		break;
	case 100:
		SelectAllGPUCacheShaders();
		break;
	case 101:
		SelectFirstGPUCacheShader();
		break;
	case 102:
		SelectNextGPUCacheShader();
		break;
	case 200:
		// select all the geometry cache models
		SelectAllGPUCacheObjects();
		break;
	case 201:
		// select first geom cache model
		SelectFirstGPUCacheObject();
		break;
	case 202:
		// select next geom cache model
		SelectNextGPUCacheObject();
		break;
	}
}

void MoRendererCallback::SelectAllGPUCacheObjects()
{
	 // ORModelGPUCache

	FBScene *pScene = mSystem.Scene;
	int count = pScene->Components.GetCount();

	for (int i=0; i<count; ++i)
	{
		FBComponent *pComp =  pScene->Components[i];
		
		if (FBIS(pComp, ORModelGPUCache) )
		{
			pComp->Selected = true;
		}
	}
}

void MoRendererCallback::SelectFirstGPUCacheObject()
{
	 // ORModelGPUCache

	FBScene *pScene = mSystem.Scene;
	int count = pScene->Components.GetCount();

	for (int i=0; i<count; ++i)
	{
		FBComponent *pComp =  pScene->Components[i];
		
		if (FBIS(pComp, ORModelGPUCache) )
		{
			pComp->Selected = false;
		}
	}

	// let's select only a first one
	for (int i=0; i<count; ++i)
	{
		FBComponent *pComp =  pScene->Components[i];
		
		if (FBIS(pComp, ORModelGPUCache) )
		{
			pComp->Selected = true;
			break;
		}
	}
}

void MoRendererCallback::SelectNextGPUCacheObject()
{
	 // ORModelGPUCache

	FBScene *pScene = mSystem.Scene;
	int count = pScene->Components.GetCount();

	int currSelected = -1;
	std::vector<FBComponent*>	objects;

	for (int i=0; i<count; ++i)
	{
		FBComponent *pComp =  pScene->Components[i];
		
		if (FBIS(pComp, ORModelGPUCache) )
		{
			if (pComp->Selected)
				currSelected = (int) objects.size();
			objects.push_back(pComp);
			pComp->Selected = false;
		}
	}

	if (objects.size() == 0)
		return;

	if (currSelected == -1 || currSelected == objects.size()-1)
		currSelected = 0;
	else
		currSelected += 1;

	objects[currSelected]->Selected = true;
}

void MoRendererCallback::SelectAllGPUCacheShaders()
{
	FBScene *pScene = mSystem.Scene;
	for (int i=0, count=pScene->Shaders.GetCount(); i<count; ++i)
	{
		FBShader *pShader = pScene->Shaders[i];
		if (FBIS(pShader, ORShaderGPUCache) )
		{
			pShader->Selected = true;
		}
	}
}

void MoRendererCallback::SelectFirstGPUCacheShader()
{
	FBScene *pScene = mSystem.Scene;
	const int count = pScene->Shaders.GetCount();
	for (int i=0; i<count; ++i)
	{
		FBShader *pShader = pScene->Shaders[i];
		if (FBIS(pShader, ORShaderGPUCache) )
		{
			pShader->Selected = false;
		}
	}
	// select a first one
	for (int i=0; i<count; ++i)
	{
		FBShader *pShader = pScene->Shaders[i];
		if (FBIS(pShader, ORShaderGPUCache) )
		{
			pShader->Selected = true;
			break;
		}
	}
}

void MoRendererCallback::SelectNextGPUCacheShader()
{
	int currShader = -1;
	std::vector<FBShader*>	shaders;

	FBScene *pScene = mSystem.Scene;
	const int count = pScene->Shaders.GetCount();
	for (int i=0; i<count; ++i)
	{
		FBShader *pShader = pScene->Shaders[i];
		if (FBIS(pShader, ORShaderGPUCache) )
		{
			currShader = (int) shaders.size();
			pShader->Selected = false;
			shaders.push_back(pShader);
		}
	}
	
	if (shaders.size() == 0)
		return;

	if (currShader == -1 || currShader == shaders.size()-1)
		currShader = 0;
	else
		currShader += 1;

	shaders[currShader]->Selected = true;
}