
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_content_rendering_depricated.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_content.h"



void CGPUFBScene::RenderSceneShaderGroups2(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions)
{

//	const int intpass = (int) options.GetPass();
	const int intgoal = (int) options.GetGoal();
	const auto &shaderGroups = mShadersFactory.GetShaderTypesVector();

	mModelsInspector.BindVertexConveyer();

	for (auto iter=begin(shaderGroups); iter!=end(shaderGroups); ++iter)
	{
		CBaseShaderCallback *pShaderGroup = *iter;

		// check if callback supports current pass
		if ( 0 == (intgoal & pShaderGroup->ShaderGroupGoal()) )
			continue;

		const int id = pShaderGroup->Id();
		const int numberOfModels = mModelsInspector.GetNumberOfModelsInGroup(id);

		if (numberOfModels == 0)
			continue;

		//
		bool lStatus = false;

		lStatus = pShaderGroup->OnTypeBegin( options, true );

		if (false == lStatus)
			continue;

		// draw connected meshes with current shader group

		auto &sortedMap = mModelsInspector.GetSortedModelsMap(id);
		FBShader *pLastShader = (FBShader*) 1;
		bool isFirstShader = true;

		for (auto shaderIter=begin(sortedMap);
			shaderIter != end(sortedMap);
			++shaderIter )
		{
			//
			// TODO: put a shader info !!
			FBShader *pShader = shaderIter->first;

			if (pLastShader != pShader)
			{
				if (false == isFirstShader)
				{
					pShaderGroup->OnInstanceEnd( options, pShader, nullptr);
				}

				lStatus = pShaderGroup->OnInstanceBegin( options, pFBRenderOptions, pShader, nullptr );

				if (false == lStatus)
					continue;
				
				pLastShader = pShader;
			}
			
			isFirstShader = false;
			
			// iteration for shader models
			FBModel *pModel = shaderIter->second.pModel;
			FBEvaluateInfo *pEvalInfo = FBGetDisplayInfo();
			if (pModel->IsVisible(pEvalInfo) == false || pModel->Show == false)
				continue;
			
			if ( false == pModel->IsEvaluationReady(kFBModelEvaluationDeform) )
			{
				mNumberOfUnReadyModels += 1;
				continue;
			}

			if ( pShaderGroup->IsMaterialEffect() )
			{
				lStatus = pShaderGroup->OnModelDraw( options, pFBRenderOptions, pModel, nullptr);
				if (true == lStatus)
				{
					//RenderPassModelDraw( pShaderGroup, options, pModel, shaderIter->second.startMeshId );
					RenderPassModelDraw( pShaderGroup, options, pModel, shaderIter->second.startMeshId );
				}
			}
			else
			{
				lStatus = pShaderGroup->OwnModelShade( options, pFBRenderOptions, pModel, pShader, nullptr );
			}
		}

		if (false == isFirstShader)
		{
			pShaderGroup->OnInstanceEnd( options, pLastShader, nullptr);
		}

		pShaderGroup->OnTypeEnd( options );
	}

	mModelsInspector.UnBindVertexConveyer();
}

void CGPUFBScene::RenderPass(const EShaderPass pass)
{
	/*
	int intpass = (int) pass;
	auto &callbacks = mShadersInspector.GetShaderTypesVector();
	auto &fbshaders = mShadersInspector.GetResourceVector();

	CBaseShaderCallback *pLastType = nullptr; 

	// CALLBACKS SHOULD BE SORTED IN PRIORITY ORDER !

	// shader could be used by callback only once 
	mUsedShaderFlags.resize(fbshaders.size());
	std::fill( begin(mUsedShaderFlags), end(mUsedShaderFlags), 0 );

	for (size_t callbackNdx=0;
		callbackNdx < callbacks.size();
		++callbackNdx)
	{

		CBaseShaderCallback *pCallback = callbacks[callbackNdx];
		
		

		for (size_t i=0; i<fbshaders.size(); ++i)
		{
			if (mUsedShaderFlags[i] > 0)
				continue;

			FBShader *pShader = fbshaders[i];
			
			if (pShader == nullptr || pShader->Enable == false)
			{
				mUsedShaderFlags[i] = 1;
				continue;
			}

			if (false == pCallback->IsForShaderAndPass(pShader, pass) )
				continue;

			// get callback-shader info
			CBaseShaderInfo *pInfo = mShadersInspector.GetShaderCallbackInfo(callbackNdx, i);

			// let's render this shader with current callback for the pass
			
			RenderPassShaderModels(pass, pCallback, pShader, 
				pInfo, pLastType, pCallback->IsMaterialEffect());

			mUsedShaderFlags[i] = 1;
		}

	}

	if (pLastType != nullptr)
	{
		pLastType->OnTypeEnd( (EShaderPass) pass );
		pLastType = nullptr;
	}
	*/
}

/*
void CGPUFBScene::RenderPass(const EShaderPass pass)
{
	int intpass = (int) pass;

	//auto &shaderTypes = mShadersInspector.GetShaderTypesVector();
	auto &shaderInstances = mShadersInspector.GetShaderIntancesMultimap(); // (pass);
	
	//! multimap is sorted ! so it goes type by type

	CBaseShaderCallback *pLastType = nullptr; 

	for (auto iter=begin(shaderInstances); iter!=end(shaderInstances); ++iter)
	{
		CBaseShaderCallback *pCallback = iter->first;
		
		// check if callback supports current pass
		if ( 0 == (intpass & pCallback->CallbackPasses()) )
			continue;
		
		FBShader *pShader = iter->second.pShader;
		CBaseShaderInfo *pInfo = iter->second.pShaderInfo;

		if (false == pCallback->IsForShader(pShader, pass) )
			continue;

		if (false == pShader->Enable)
			continue;

		// TODO: Do we need to cache connected models, we could sort them !
		// skip empty shaders
		const int numShaderModels = pShader->GetDstCount();
		if (numShaderModels == 0)
			continue;

		//
		RenderPassShaderModels(pass, pCallback, 
			pShader, pInfo, pLastType, pCallback->IsMaterialEffect());
		
	}

	if (pLastType != nullptr)
	{
		pLastType->OnTypeEnd( (EShaderPass) pass );
		pLastType = nullptr;
	}
}
*/
void CGPUFBScene::RenderPassShaderModels(const EShaderPass pass, CBaseShaderCallback *pCallback, 
	FBShader *pShader, CBaseShaderInfo *pInfo, CBaseShaderCallback *&pLastType, const bool IsFBModelDraw)
{
	/*
	int intpass = (int) pass;

	//FBShader *pShader = iter->second.pShader;

	if (pLastType != nullptr && pCallback != pLastType)
	{
		pLastType->OnTypeEnd( (EShaderPass) pass );
		pLastType = nullptr;
	}
	
	// TODO: Do we need to cache connected models, we could sort them !
	// skip empty shaders
	const int numShaderModels = pShader->GetDstCount();
	
	bool lStatus = false;

	if (pCallback != pLastType)
	{
		lStatus = pCallback->OnTypeBegin( (EShaderPass) pass );

		if (false == lStatus)
			return;

		pLastType = pCallback;
	}
		
	//
	// TODO: put a shader info !!

	//CBaseShaderInfo *pInfo = mShadersInspector.GetShaderInfo(pShader);
	//CBaseShaderInfo *pInfo = iter->second.pShaderInfo;

	lStatus = pCallback->OnInstanceBegin( (EShaderPass) pass, pShader, pInfo);

	if (false == lStatus)
		return;

	FBEvaluateInfo *pEvalInfo = FBGetDisplayInfo();

	// iteration for shader models
	for (int j=0; j<numShaderModels; ++j)
	{
		FBPlug *pPlug = pShader->GetDst(j);

		if ( !FBIS(pPlug, FBModel) )
			continue;

		FBModel *pModel = (FBModel*) pPlug;

		if (pModel->IsVisible(pEvalInfo) == false || pModel->Show == false)
			continue;

		//RenderModel(true, pModel, 0);

		if (IsFBModelDraw)
		{
			lStatus = pCallback->OnModelDraw( (EShaderPass) pass, pModel, pInfo);

			if (true == lStatus)
			{
				// TODO: Render model
				// ...

				RenderPassModelDraw( pCallback, pass, pModel );
			}
		}
		else
		{
			lStatus = pCallback->OwnModelShade( (EShaderPass) pass, pModel, pShader, pInfo );
		}
	}

	pCallback->OnInstanceEnd( (EShaderPass) pass, pShader, pInfo);
	*/
}



bool CGPUFBScene::RenderPassModelDraw2(CBaseShaderCallback *pCallback, const CRenderOptions &options, FBModel *pModel, const int meshIndex)
{
	//const auto loc = mUberShader->GetCustomEffectShaderLocationsPtr();

	FBModelVertexData *pData = pModel->ModelVertexData;
	
	//const int regionCount = pData->GetSubRegionCount();
	const int patchCount = pData->GetSubPatchCount();

	//
	for (int i=0; i<patchCount; ++i)
	{
//		int offset = pData->GetSubPatchIndexOffset(i);
//		int size = pData->GetSubPatchIndexSize(i);

		// TODO: mesh index is a vertex attribute !
		//mUberShader->UpdateMeshIndex(meshIndex + i);
		//mBufferMesh.BindAsAttribute( 5, sizeof(MeshGLSL)*(meshIndex+i) );
		
		// TODO: replace with bindless textures !
		/*
		if (bindTextures)
		{
			FBMaterial *pMaterial = pData->GetSubPatchMaterial(i);
			BindMaterialTextures(pMaterial);
		}
		*/

		FBMaterial *pMaterial = pData->GetSubPatchMaterial(i);

		if (pCallback)
		{
			pCallback->OnMaterialBegin( options, nullptr, pMaterial, false );
		}

		//
		pData->DrawSubPatch(i);
		//glDrawArrays( GL_POINTS, offset, size );

		if (pCallback)
		{
			pCallback->OnMaterialEnd( options, pMaterial );
		}
	}

	CHECK_GL_ERROR_MOBU();
	return true;
}


// TODO: DEPRICATED !
void CGPUFBScene::RenderSceneCallbacks(bool selectionPass, bool useEarlyZ, bool backModelsOnly)
{
	FBRenderer* lRenderer = mSystem.Renderer;
	const int numberOfModels = lRenderer->DisplayableGeometryCount;

	if ( false == mMaterialShaders.IsOk() || 0 == numberOfModels )
		return;

	// process shader info before passes rendering
	mShadersInspector.EventBeforeRenderNotify();

	// TODO: FIRST PASS - early-z rendering, also MRT rendering

	if (selectionPass)
	{
		RenderPass( eShaderPassInvalid );
	}
	else if (backModelsOnly)
	{
		RenderPass( eShaderPassInvalid );
	}
	else
	{
		// render passes opaque and transparency pass
		//

		RenderPass( eShaderPassOpaque );
		RenderPass( eShaderPassTransparency );
	}
	/*
	if (true == useEarlyZ && false == backModelsOnly)
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_TRUE);

		mUberShader->SetEarlyZ(true);

		//
		PrepRender();

		BindUberShader(overrideShading);

		int modelIndex = 0;
		for(auto modelIter=begin(modelsVector); modelIter!=end(modelsVector); ++modelIter, ++modelIndex)
		{
			if (mModelsInspector.IsDeleted(modelIndex) )
				continue;

			FBModel *pModel = *modelIter;

			if (pModel->IsVisible(pEvalInfo) == false || pModel->Show == false)
				continue;

			// without any texture binding
			RenderModel(false, pModel, modelIndex);
		}

		UnBindUberShader();

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_FALSE);
	}
	*/

	CHECK_GL_ERROR_MOBU();
}