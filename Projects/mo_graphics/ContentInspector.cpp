
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ContentInspector.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ContentInspector.h"

//#include "..\mo_geomCache\GPUCaching_shader.h"
#include "ProjTex_shader.h"
#include "GPUCaching_shader.h"
#include "MB_IBL_shader.h"

#include "callbacks.h"

#include "glm/gtx/simd_mat4.hpp"
#include "glm/gtc/type_ptr.hpp"

void CShadersInspector::ConstructFromFBShader( FBShader *pShader, FBEvaluateInfo *pEvalInfo, ShaderGLSL &shader )
{
	DefaultShader(shader);

	if ( FBIS(pShader, ProjTexShader) )
	{
		shader.shaderType = eShaderTypeSuperLighting;

		ProjTexShader *pSuperShader = (ProjTexShader*) pShader;

		EShadingType shadingType = pSuperShader->ShadingType;
		shader.shadingType = (int) shadingType;
	
		double transF;
		pSuperShader->TransparencyFactor.GetData( &transF, sizeof(double), pEvalInfo );
		shader.transparency = (float) transF;

		shader.alphaFromColor = (pSuperShader->UseAlphaFromColor.AsInt() > 0) ? 1.0f : 0.0f;

		// check if we should apply color correction
		shader.applyColorCorrection = 0.0f;
		
		if (true == pSuperShader->IsApplyColorCorrection() )
		{
			shader.applyColorCorrection = 1.0f;

			FBColor color;
			pSuperShader->CustomColor.GetData(color, sizeof(FBColor), pEvalInfo);
		
			EBlendType blendType = pSuperShader->CustomColorMode;
			shader.customColor = vec4((float)color[0], (float)color[1], (float)color[2], (float) blendType);

			double contrast, sat, br, gam;

			pSuperShader->Contrast.GetData( &contrast, sizeof(double), pEvalInfo );
			pSuperShader->Saturation.GetData( &sat, sizeof(double), pEvalInfo );
			pSuperShader->Brightness.GetData( &br, sizeof(double), pEvalInfo );
			pSuperShader->Gamma.GetData( &gam, sizeof(double), pEvalInfo );

			shader.contrast = 1.0f + 0.01f * (float)contrast;
			shader.saturation = 1.0f + 0.01f * (float)sat;
			shader.brightness = 1.0f + 0.01f * (float)br;
			shader.gamma = 0.01f * (float)gam;

		}

		// Toon settings
		if (eShadingTypeToon == shadingType)
		{
			double toonSteps, toonDist, toonSh;

			pSuperShader->ToonSteps.GetData( &toonSteps, sizeof(double), pEvalInfo );
			pSuperShader->ToonDistribution.GetData( &toonDist, sizeof(double), pEvalInfo );
			pSuperShader->ToonShadowPosition.GetData( &toonSh, sizeof(double), pEvalInfo );

			shader.toonEnabled = 0.0f;
			shader.toonSteps = (float) toonSteps;
			shader.toonDistribution = (float) toonDist;
			shader.toonShadowPosition = (float) toonSh;
		}

		//
		shader.depthDisplacement = 0.01f * pSuperShader->DepthDisplacement;

		//
		shader.mask = vec4( (pSuperShader->CompositeMaskA.AsInt() > 0) ? 1.0f : 0.0f,
			(pSuperShader->CompositeMaskB.AsInt() > 0) ? 1.0f : 0.0f,
			(pSuperShader->CompositeMaskC.AsInt() > 0) ? 1.0f : 0.0f,
			(pSuperShader->CompositeMaskD.AsInt() > 0) ? 1.0f : 0.0f );

		double useRim=0.0;
		pSuperShader->UseRim.GetData( &useRim, sizeof(double), pEvalInfo );
		if ( useRim > 0.0 )
		{
			double drimPower;
			pSuperShader->RimPower.GetData( &drimPower, sizeof(double), pEvalInfo );

			shader.rimOptions = vec4( 0.01f * (float) useRim,
				0.01f * (float) drimPower,
				(pSuperShader->RimTexture.GetCount() > 0) ? 1.0f : 0.0f, 
				0.0f);

			FBColor drimColor;
			pSuperShader->RimColor.GetData( &drimColor, sizeof(FBColor), pEvalInfo );
			shader.rimColor = vec4( (float)drimColor[0], (float)drimColor[1], 
				(float)drimColor[2], 1.0f );
		}		
	}
	else if ( FBIS(pShader, ORIBLShader) )
	{
		shader.shaderType = eShaderTypeSuperLighting;

		ORIBLShader *pSuperShader = (ORIBLShader*) pShader;

		EShadingType shadingType = pSuperShader->ShadingType;
		shader.shadingType = (int) shadingType;
	
		double transF;
		pSuperShader->TransparencyFactor.GetData( &transF, sizeof(double), pEvalInfo );
		shader.transparency = (float) transF;

		shader.alphaFromColor = (pSuperShader->UseAlphaFromColor.AsInt() > 0) ? 1.0f : 0.0f;

		// check if we should apply color correction
		shader.applyColorCorrection = 0.0f;
		
		//
		shader.mask = vec4( (pSuperShader->CompositeMaskA.AsInt() > 0) ? 1.0f : 0.0f,
			(pSuperShader->CompositeMaskB.AsInt() > 0) ? 1.0f : 0.0f,
			(pSuperShader->CompositeMaskC.AsInt() > 0) ? 1.0f : 0.0f,
			(pSuperShader->CompositeMaskD.AsInt() > 0) ? 1.0f : 0.0f );

		double useRim=0.0;
		pSuperShader->UseRim.GetData( &useRim, sizeof(double), pEvalInfo );
		if ( useRim > 0.0 )
		{
			double drimPower;
			pSuperShader->RimPower.GetData( &drimPower, sizeof(double), pEvalInfo );

			shader.rimOptions = vec4( 0.01f * (float) useRim,
				0.01f * (float) drimPower,
				(pSuperShader->RimTexture.GetCount() > 0) ? 1.0f : 0.0f, 
				0.0f);

			FBColor drimColor;
			pSuperShader->RimColor.GetData( &drimColor, sizeof(FBColor), pEvalInfo );
			shader.rimColor = vec4( (float)drimColor[0], (float)drimColor[1], 
				(float)drimColor[2], 1.0f );
		}
	}
	else if ( FBIS(pShader, FXColorCorrectionShader) )
	{
		shader.shaderType = eShaderTypeColorCorrection;

		FXColorCorrectionShader *pSuperShader = (FXColorCorrectionShader*) pShader;

		// TODO: check if we should apply color correction here
		shader.applyColorCorrection = 1.0f;

		FBColor color;
		pSuperShader->CustomColor.GetData(color, sizeof(FBColor), pEvalInfo);
		
		EBlendType blendType = pSuperShader->CustomColorMode;
		shader.customColor = vec4((float)color[0], (float)color[1], (float)color[2], (float) blendType);

		double contrast, sat, br, gam;

		pSuperShader->Contrast.GetData( &contrast, sizeof(double), pEvalInfo );
		pSuperShader->Saturation.GetData( &sat, sizeof(double), pEvalInfo );
		pSuperShader->Brightness.GetData( &br, sizeof(double), pEvalInfo );
		pSuperShader->Gamma.GetData( &gam, sizeof(double), pEvalInfo );

		shader.contrast = 1.0f + 0.01f * (float)contrast;
		shader.saturation = 1.0f + 0.01f * (float)sat;
		shader.brightness = 1.0f + 0.01f * (float)br;
		shader.gamma = 0.01f * (float)gam;
	}
	else if ( FBIS(pShader, FXProjectionMapping) )
	{
		shader.shaderType = eShaderTypeProjections;
	}
	else if ( FBIS(pShader, FXShadingShader) )
	{
		shader.shaderType = eShaderTypeShading;

		FXShadingShader *pSuperShader = (FXShadingShader*) pShader;

		EShadingType shadingType = pSuperShader->ShadingType;
		shader.shadingType = (int) shadingType;
	
		double transF;
		pSuperShader->TransparencyFactor.GetData( &transF, sizeof(double), pEvalInfo );
		shader.transparency = (float) transF;

		// Toon settings
		if (eShadingTypeToon == shadingType)
		{
			double toonSteps, toonDist, toonSh;

			pSuperShader->ToonSteps.GetData( &toonSteps, sizeof(double), pEvalInfo );
			pSuperShader->ToonDistribution.GetData( &toonDist, sizeof(double), pEvalInfo );
			pSuperShader->ToonShadowPosition.GetData( &toonSh, sizeof(double), pEvalInfo );

			shader.toonEnabled = 0.0f;
			shader.toonSteps = (float) toonSteps;
			shader.toonDistribution = (float) toonDist;
			shader.toonShadowPosition = (float) toonSh;
		}
	}
	else
	{
		FBProperty *pProp = pShader->PropertyList.Find( "Transparency Factor" );

		if (nullptr != pProp)
		{
			double transF;
			pProp->GetData( &transF, sizeof(double), pEvalInfo );
			shader.transparency = (float) transF;
		}

		
	}
}

CShadersInspector::CShadersInspector()
{
	mIsCombinationsUpdated = false;
	ClearShaderGroupsInfo();
}

void CShadersInspector::ClearShaderGroupsInfo()
{

	// sorted shaders by callback id

	auto &callbacks = mShadersFactory.GetShaderTypesVector();
	mSortedShaders.resize( callbacks.size() );

	for (auto iter=begin(mSortedShaders); iter!=end(mSortedShaders); ++iter)
	{
		iter->clear();
	}
}

void CShadersInspector::FullSortShaderCallbacks()
{
	/*
	// find a callback for each shader

	EShaderPass		passesVector[SHADER_PASS_COUNT] = { eShaderPassBackground, 
		eShaderPassSelection, eShaderPassOpaque, eShaderPassTransparency, eShaderPassSoftParticles };

	mShaderCounts = 0; // [i] = 0;
	mShaderInstances.clear(); // [i].clear();

	for (int i=0; i<SHADER_PASS_COUNT; ++i)
	{
		for (auto iter=begin(mResourceVector); iter!=end(mResourceVector); ++iter)
		{
			FBShader *pShader = *iter;

			if (pShader == nullptr)
				continue;

			CBaseShaderCallback *pCallback = mShadersFactory.FindTypeByShader(pShader, passesVector[i]);

			if (nullptr != pCallback)
			{
				CBaseShaderInfo *pShaderInfo = GetShaderInfo(pShader, true, pCallback);
				MyFBShaderWrap wrap = { pShader, pShaderInfo};

				mShaderInstances.insert( std::make_pair( pCallback, wrap ) );
				mShaderCounts += 1;
			}
		}
	}
	*/
}

void CShadersInspector::SortShaders()
{
	auto &callbacks = mShadersFactory.GetShaderTypesVector();

	mSortedShaders.resize(callbacks.size() );

	for (auto iter=begin(mResourceVector); iter!=end(mResourceVector); ++iter)
	{

	}
}

void CShadersInspector::SyncShaderCallbackInfos()
{
	//EShaderPass		passesVector[SHADER_PASS_COUNT] = { eShaderPassBackground, 
	//	eShaderPassSelection, eShaderPassOpaque, eShaderPassTransparency, eShaderPassSoftParticles };

	EShaderPass		passesVector[SHADER_PASS_COUNT] = { eShaderPassOpaque };

	// for each callback (number of callback is syncs and unchanged)

	auto &callbacks = mShadersFactory.GetShaderTypesVector();

	std::vector<CBaseShaderInfo*>	tempVector;
	std::vector<char>				usedInfos;

	//if (mResourceVector.size() > 0)
	//	tempVector.resize(mResourceVector.size(), nullptr);

	if (mShaderCallbackInfos.size() == 0)
	{
		mShaderCallbackInfos.resize(callbacks.size() );
	}

	for (size_t callbackNdx=0; 
		callbackNdx<mShaderCallbackInfos.size(); 
		++callbackNdx )
	{
		auto &callbackInfos = mShaderCallbackInfos[callbackNdx];
		size_t numberOfInfos = callbackInfos.size();

		tempVector.resize(numberOfInfos );
		usedInfos.resize(numberOfInfos);

		for (size_t i=0; i<numberOfInfos; ++i)
		{
			tempVector[i] = callbackInfos[i];
			usedInfos[i] = 0;
		}


		// resize and sync to resouce vector
		callbackInfos.resize( mResourceVector.size() );

		// let's find a old info for a new resouce place
		numberOfInfos = callbackInfos.size();

		for (size_t i=0; i<numberOfInfos; ++i)
		{
			if ( mUpdateVector[i] & RESOURCE_DELETED )
			{
				callbackInfos[i] = nullptr;
				continue;
			}

			FBShader *pShader = mResourceVector[i];
			if (pShader == nullptr)
			{
				callbackInfos[i] = nullptr;
				continue;
			}

			CBaseShaderInfo *pInfo = nullptr;

			
			for (size_t j=0; j<tempVector.size(); ++j)
			{
				CBaseShaderInfo *pTempInfo = tempVector[j];
					
				if (pTempInfo != nullptr 
					&& pTempInfo->GetFBShader() == pShader)
				{
					pInfo = pTempInfo;
					usedInfos[j] = 1;
					break;
				}
			}

			if (nullptr == pInfo)
			{
				// create a new callback-shader info
				CBaseShaderCallback *pCallback = callbacks[callbackNdx];

				bool isForShader = false;

				for (int ii=0; ii<SHADER_PASS_COUNT; ++ii)
					if (true == pCallback->IsForShaderAndPass(pShader, passesVector[ii]) )
					{
						isForShader = true;
						break;
					}

				if (isForShader && pCallback->IsNeedShaderInfo())
				{
					pInfo = pCallback->CreateANewShaderInfo(pShader);
				}
			}

			callbackInfos[i] = pInfo;
		}

		// TODO: if some resource missing, 

		for (size_t i=0; i<tempVector.size(); ++i)
		{
			if (usedInfos[i] == 0)
			{
				// this info is not needed anymore, let's free memory
				// FREE MEM

				CBaseShaderInfo *pInfo = tempVector[i];

				delete pInfo;
				pInfo = nullptr;

			}
		}

	}

}

void CShadersInspector::OnItemAdd( FBShader *pShader )
{
	if (mNeedFullUpdate == true)
		return;

	//
	// 
	PendingAdd(pShader);
}

void CShadersInspector::OnItemDelete( FBShader *pShader )
{
	if (mNeedFullUpdate == true)
		return;

	//
	// 
	PendingDel(pShader);
}


void CShadersInspector::PendingAdd(FBShader *pShader)
{
	if (mNeedFullCallbackSort)
		return;

	if (mPendingToAddCount >= MAX_PENDING_ITEMS-1)
	{
		mNeedFullCallbackSort = true;
		return;
	}

	mPendingShadersToAdd[mPendingToAddCount] = pShader;
	mPendingToAddCount += 1;
}

void CShadersInspector::PendingDel(FBShader *pShader)
{
	if (mNeedFullCallbackSort)
		return;

	if (mPendingToDelCount >= MAX_PENDING_ITEMS-1)
	{
		mNeedFullCallbackSort = true;
		return;
	}

	mPendingShadersToDel[mPendingToDelCount] = pShader;
	mPendingToDelCount += 1;
}

void CShadersInspector::PendingUpdate(FBShader *pShader)
{
	if (mNeedFullCallbackSort)
		return;

	if (mPendingUpdateCount >= MAX_PENDING_ITEMS-1)
	{
		mNeedFullCallbackSort = true;
		return;
	}

	mPendingShadersToUpdate[mPendingUpdateCount] = pShader;
	mPendingUpdateCount += 1;
}

void CShadersInspector::ClearPendingData()
{
	mPendingToAddCount = 0;
	mPendingToDelCount = 0;
	mPendingUpdateCount = 0;
}

void CShadersInspector::EventBeforeRenderNotify()
{

	if (mNeedFullCallbackSort)
	{
		//FullSortShaderCallbacks();
		//SyncShaderCallbackInfos();
		SyncShaderGroups();

		mNeedFullCallbackSort = false;
		ClearPendingData();
	}
	else
	{

		if (mPendingToAddCount > 0 || mPendingToDelCount > 0 || mPendingUpdateCount > 0)
		{
			//SyncShaderCallbackInfos();
			SyncShaderGroups();
			ClearPendingData();
		}

		/*
		EShaderPass		passesVector[SHADER_PASS_COUNT] = { eShaderPassBackground, 
			eShaderPassSelection, eShaderPassOpaque, eShaderPassTransparency, eShaderPassSoftParticles };

		// check what's new we have to add to our multimap
		for (int nShader=0; nShader<mPendingToAddCount; ++nShader)
		{
			FBShader *pShader = mPendingShadersToAdd[nShader];

			for (int i=0; i<SHADER_PASS_COUNT; ++i)
			{
				CBaseShaderCallback *pCallback = mShadersFactory.FindTypeByShader(pShader, passesVector[i]);

				if (nullptr != pCallback)
				{
					// DONE: find or allocate a shader info for this shader !!
					CBaseShaderInfo *pShaderInfo = GetShaderInfo(pShader, true, pCallback);
					MyFBShaderWrap wrap = { pShader, pShaderInfo };

					mShaderInstances.insert( std::make_pair( pCallback, wrap ) );
					mShaderCounts += 1;
				}
			}
		}

		// check what we have to remove
		for (int nShader=0; nShader<mPendingToDelCount; ++nShader)
		{
			FBShader *pShader = mPendingShadersToDel[nShader];

			std::vector<std::multimap<FBShader*, MyFBShaderInfoWrap>::iterator>	toDeleteInfos;

			auto shaderpair = mShaderInfos.equal_range(pShader);
			for (auto it=begin(shaderpair); it!=end(shaderpair); ++it)
			{

				std::vector<std::multimap<CBaseShaderCallback*, MyFBShaderWrap>::iterator>	toDeleteVector;

				auto iterpair = mShaderInstances.equal_range(it->second.pCallback);
				for (auto callbackit=begin(iterpair); callbackit!=end(iterpair); ++callbackit)
				{

					if (callbackit->second.pShader == pShader) {
						toDeleteVector.push_back(callbackit);
						//mShaderInstances.erase(callbackit);
						//mShaderCounts -= 1;
					}
				}

				for (auto iter=begin(toDeleteVector); iter!=end(toDeleteVector); ++iter)
				{
					mShaderInstances.erase(*iter);
					mShaderCounts -= 1;
				}

				// free shader-callback info
				CBaseShaderInfo *pInfo = it->second.pShaderInfo;
				if (pInfo)
				{
					delete pInfo;
				}
				//mShaderInfos.erase(it);

				toDeleteInfos.push_back(it);
			}

			for (auto iter=begin(toDeleteInfos); iter!=end(toDeleteInfos); ++iter)
				mShaderInfos.erase(*iter);
		}

		// TODO: check what we have to update !!
		*/
	}

	//
	// update shader infos !
	/*
	for (auto callbackIter=begin(mShaderCallbackInfos);
		callbackIter != end(mShaderCallbackInfos);
		++callbackIter )
	{
		for (auto iter=callbackIter->begin();
			iter != callbackIter->end();
			++iter)
		{
			if (nullptr != *iter)
				(*iter)->EventBeforeRenderNotify();
		}
	}
	*/
	/*
	for (auto iter=begin(mShaderInfos); iter!=end(mShaderInfos); ++iter)
	{
		CBaseShaderInfo *pInfo = iter->second.pShaderInfo;
		if (nullptr != pInfo)
		{
			pInfo->EventBeforeRenderNotify();
		}
	}
	*/
}

void CShadersInspector::EventConnNotify(HISender pSender, HKEvent pEvent)
{

	for (auto callbackIter=begin(mShaderCallbackInfos);
		callbackIter != end(mShaderCallbackInfos);
		++callbackIter )
	{
		for (auto iter=callbackIter->begin();
			iter != callbackIter->end();
			++iter)
		{
			if (nullptr != *iter)
				(*iter)->EventConnNotify(pSender, pEvent);
		}
	}
	/*
	for (auto iter=begin(mShaderInfos); iter!=end(mShaderInfos); ++iter)
	{
		CBaseShaderInfo *pShaderInfo = iter->second.pShaderInfo;

		if (nullptr != pShaderInfo)
		{
			pShaderInfo->EventConnNotify(pSender, pEvent);
		}
	}
	*/
}

void CShadersInspector::AskToUpdateAllShadersData()
{
	// NOTE: we are doing the update inside the Update All method !

	/*
	for (auto iter=begin(mResourceVector); iter!=end(mResourceVector); ++iter)
	{
		FBShader *pShader = *iter;
		
		if (nullptr != pShader)
		{
			if (FBIS(pShader, ProjTexShader) )
				( (ProjTexShader*) pShader)->AskToUpdateAll();
			else if (FBIS(pShader, FXProjectionMapping) )
				( (FXProjectionMapping*) pShader)->AskToUpdateAll();
		}
	}
	*/
}

void CShadersInspector::ChangeContext()
{
	for (auto callbackIter=begin(mShaderCallbackInfos);
		callbackIter != end(mShaderCallbackInfos);
		++callbackIter )
	{
		for (auto iter=begin(*callbackIter);
			iter != end(*callbackIter);
			++iter)
		{
			if (nullptr != *iter)
				(*iter)->ChangeContext();
		}
	}

	/*
	for (auto iter=begin(mShaderInfos); iter!=end(mShaderInfos); ++iter)
		iter->second.pShaderInfo->ChangeContext();
		*/

	AskToUpdateAllShadersData();
}



CBaseShaderInfo *CShadersInspector::GetShaderInfo(FBShader *pShader, bool createIfEmtpy, CBaseShaderCallback *pCallback)
{
	auto &callbacks = mShadersFactory.GetShaderTypesVector();


	for (int i=0; i<callbacks.size(); ++i)
	{
		if (pCallback == callbacks[i])
		{
			for (auto iter=begin(mShaderCallbackInfos[i]); 
				iter != end(mShaderCallbackInfos[i]);
				++iter)
			{
				if ( (*iter) != nullptr 
					&& (*iter)->GetFBShader() == pShader)
				{
					return (*iter);
				}
			}
			break;

		}
	}

	return nullptr;

	/*
	typedef std::multimap<FBShader*, MyFBShaderInfoWrap>::iterator iterator;
	std::pair<iterator, iterator> iterpair = mShaderInfos.equal_range(pShader);

	// Erase (pCallback, pShader) pair
	//
	iterator it = iterpair.first;
	for (; it != iterpair.second; ++it) {

		if (it->second.pCallback == pCallback)
		{
			return it->second.pShaderInfo;
		}

	}

	// here we start to create a new shader info
	
	if (createIfEmtpy)
	{
		CBaseShaderInfo *pShaderInfo = pCallback->CreateANewShaderInfo(pShader);
		MyFBShaderInfoWrap wrap = { pShaderInfo, pCallback };

		mShaderInfos.insert( std::make_pair( pShader, wrap ) );
		return pShaderInfo;
	}

	return nullptr;
	*/
}

void CShadersInspector::RemoveShaderInfos(FBShader *pShader)
{
	/*
	if (nullptr == pShader)
		return;

	typedef std::multimap<FBShader*, MyFBShaderInfoWrap>::iterator iterator;
	std::pair<iterator, iterator> iterpair = mShaderInfos.equal_range(pShader);

	// Erase (pCallback, pShader) pair
	//
	iterator it = iterpair.first;
	for (; it != iterpair.second; ++it) {

		CBaseShaderInfo *pShaderInfo = it->second.pShaderInfo;

		if (pShaderInfo)
		{
			delete pShaderInfo;
		}
	}

	mShaderInfos.erase( pShader );
	*/
}

void CShadersInspector::DefaultGLSLResource( ShaderGLSL &item )
{
	DefaultShader(item);
}
void CShadersInspector::ConvertResource( FBShader *mbitem, FBEvaluateInfo *pEvalInfo, ShaderGLSL &item, bool subdata )
{
	ConstructFromFBShader(mbitem, pEvalInfo, item);
	//item.shadingType = eShadingTypeFlat;

	// should be check if shader has changed callback or passes here ?!

	// who could know about shader change ?! - it's callback and shader info !

}

// check if shader has changes important data when we have to re-sort the shader instances
void CShadersInspector::EventDataNotify(HISender pSender, HKEvent pEvent)
{

	// TODO: check for changing "render on back", "transparency"
	//	transparency factor is animated, that is a problem !
	
	mRenderLayers.EventDataNotify(pSender, pEvent);
}

bool CShadersInspector::IsAnimated( FBShader *mbitem )
{
	// DONE:

	for (int i=0, count=mbitem->PropertyList.GetCount(); i<count; ++i)
	{
		FBProperty *pProp = mbitem->PropertyList[i];
		if (true == pProp->IsAnimated() )
			return true;
	}


	return false;
}

void CShadersInspector::SyncShaderGroups()
{	
	ClearShaderGroupsInfo();

	std::vector<FBShader*>::iterator resIter = begin(mResourceVector);
	std::vector<int>::iterator flagIter = begin(mUpdateVector);

	for (;
		resIter!=end(mResourceVector); 
		++resIter, ++flagIter)
	{
		int updateFlag = *flagIter;
		if (updateFlag & RESOURCE_DELETED)
			continue;

		FBShader *pShader = *resIter;

		CBaseShaderCallback *pCallback = mShadersFactory.FindTypeByShaderAndGoal(pShader, eRenderGoalShading);
		if (pCallback)
			mSortedShaders[pCallback->Id()].push_back(pShader);

		mRenderLayers.IncLayerShaders(pShader);
	}

}

void CShadersInspector::UpdateAll(FBEvaluateInfo *pEvalInfo)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	int count = pScene->Shaders.GetCount();
		
	ShaderGLSL defShader;
	DefaultShader(defShader);
	//defShader.shadingType = eShadingTypeFlat;

	mResourceVector.resize(count+1);
	mUpdateVector.resize(count+1);
	mGLSLResource.resize(count+1);
	mResourceMap.clear();

	mUpdateVector[0] = RESOURCE_SYSTEM;
	mResourceVector[0] = nullptr;
	mGLSLResource[0] = defShader;

	for (int i=1; i<=count; ++i)
	{
		FBShader *pShader = pScene->Shaders[i-1];
			
		mResourceMap[pShader] = i;
		mResourceVector[i] = pShader;
		
		if ( FBIS(pShader, ProjTexShader) )
		{
			( (ProjTexShader*) pShader)->CheckApplyColorCorrection(nullptr);
		}

		ConvertResource(pShader, pEvalInfo, mGLSLResource[i], false);

		mUpdateVector[i] = 0;
		if ( IsAnimated(pShader) )
			mUpdateVector[i] |= RESOURCE_ANIMATED;

		if (FBIS(pShader, ProjTexShader) )
			( (ProjTexShader*) pShader)->AskToUpdateAll();
		else if (FBIS(pShader, FXProjectionMapping) )
			( (FXProjectionMapping*) pShader)->AskToUpdateAll();
	}

	//
	mNeedFullCallbackSort = true;
}

void CShadersInspector::UpdateCombinations()
{
	for (auto iter=begin(mCombinationsVector); iter!=end(mCombinationsVector); ++iter)
	{
		ShaderGLSL &item = *iter;

		if (item.shader1 >= 0)
			ShaderReplace( item, mGLSLResource[item.shader1] );
		if (item.shader2 >= 0)
			ShaderAppend( item, mGLSLResource[item.shader2] );
		if (item.shader3 >= 0)
			ShaderAppend( item, mGLSLResource[item.shader3] );
		if (item.shader4 >= 0)
			ShaderAppend( item, mGLSLResource[item.shader4] );
		if (item.shader5 >= 0)
			ShaderAppend( item, mGLSLResource[item.shader5] );
	}

	mIsCombinationsUpdated = false;
}

void CShadersInspector::ShaderReplace(ShaderGLSL &outshader, const ShaderGLSL &replaceWith)
{
	outshader.shaderType = replaceWith.shaderType;
	outshader.shadingType = replaceWith.shadingType;
	outshader.transparency = replaceWith.transparency;

	if (eShadingTypeToon == outshader.shadingType)
	{
		outshader.toonDistribution = replaceWith.toonDistribution;
		outshader.toonShadowPosition = replaceWith.toonShadowPosition;
		outshader.toonSteps = replaceWith.toonSteps;
		outshader.toonEnabled = replaceWith.toonEnabled;
	}
	
	outshader.applyColorCorrection = replaceWith.applyColorCorrection;

	if (outshader.applyColorCorrection > 0.0f)
	{
		outshader.contrast = replaceWith.contrast;
		outshader.brightness = replaceWith.brightness;
		outshader.saturation = replaceWith.saturation;
		outshader.gamma = replaceWith.gamma;

		outshader.customColor = replaceWith.customColor;
	}

	outshader.depthDisplacement = replaceWith.depthDisplacement;
	outshader.mask = replaceWith.mask;
	outshader.rimOptions = replaceWith.rimOptions;
	outshader.rimColor = replaceWith.rimColor;
	outshader.shadow = replaceWith.shadow;
}

void CShadersInspector::ShaderAppend(ShaderGLSL &outshader, const ShaderGLSL &appendWith)
{
	switch(appendWith.shaderType)
	{
	case eShaderTypeColorCorrection:
		outshader.applyColorCorrection = appendWith.applyColorCorrection;

		if (outshader.applyColorCorrection > 0.0f)
		{
			outshader.contrast = appendWith.contrast;
			outshader.brightness = appendWith.brightness;
			outshader.saturation = appendWith.saturation;
			outshader.gamma = appendWith.gamma;

			outshader.customColor = appendWith.customColor;
		}
		break;
	case eShaderTypeShading:

		outshader.shadingType = appendWith.shadingType;
		if (eShadingTypeToon == outshader.shadingType)
		{
			outshader.toonDistribution = appendWith.toonDistribution;
			outshader.toonShadowPosition = appendWith.toonShadowPosition;
			outshader.toonSteps = appendWith.toonSteps;
			outshader.toonEnabled = appendWith.toonEnabled;
		}
		break;
	}
}

const int CShadersInspector::FindCombination(FBShader **pShaders)
{
	int indices[5] = {-1, -1, -1, -1, -1};

	for (int i=0; i<5; ++i)
		if (pShaders[i] != nullptr)
			indices[i] = FindResourceIndex(pShaders[i]);

	// find combination
	int combinationId = -1;

	for (size_t i=0; i<mCombinationsVector.size(); ++i)
	{
		ShaderGLSL &item = mCombinationsVector[i];
		
		if (indices[0]==item.shader1 && indices[1]==item.shader2
			&& indices[2]==item.shader3 && indices[3]==item.shader4 && indices[4]==item.shader5)
		{
			combinationId = (int) i;
			break;
		}
	}

	if (combinationId < 0)
	{
		// create a new combination
		ShaderGLSL newShader;
		
		newShader.shader1 = indices[0];
		newShader.shader2 = indices[1];
		newShader.shader3 = indices[2];
		newShader.shader4 = indices[3];
		newShader.shader5 = indices[4];

		mCombinationsVector.push_back(newShader);
		combinationId = (int)mCombinationsVector.size() - 1;

		mIsCombinationsUpdated = true;
	}

	return combinationId;
}

void CShadersInspector::UpdateGPUBuffer( CGPUBuffer *pBuffer )
{
	pBuffer->UpdateData( sizeof(ShaderGLSL), GetNumberOfItems(), GetData(),
		mCombinationsVector.size(), mCombinationsVector.data() );

	//mBufferShader.UpdateData( sizeof(ShaderGLSL), mShadersInspector.GetNumberOfItems(), mShadersInspector.GetData() );
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CMaterialsInspector

void CMaterialsInspector::ConstructFromFBMaterial( FBMaterial *pMaterial, FBEvaluateInfo *pEvalInfo, CTexturesInspector *pTexInspector, MaterialGLSL &mat )
{
	//DefaultMaterial(mat);

	FBColor lEmissive, lAmbient, lDiffuse, lSpecular, lTransparency, lReflect;
	
	pMaterial->Emissive.GetData( lEmissive, sizeof(FBColor), pEvalInfo );
	pMaterial->Ambient.GetData( lAmbient, sizeof(FBColor), pEvalInfo );
	pMaterial->Diffuse.GetData( lDiffuse, sizeof(FBColor), pEvalInfo );
	pMaterial->Specular.GetData( lSpecular, sizeof(FBColor), pEvalInfo );
	pMaterial->TransparentColor.GetData( lTransparency, sizeof(FBColor), pEvalInfo );
	pMaterial->Reflection.GetData( lReflect, sizeof(FBColor), pEvalInfo );

	double lEmissiveFactor, lAmbientFactor, lDiffuseFactor;
	double lSpecularFactor, lTransparencyFactor, lReflectionFactor;

	pMaterial->EmissiveFactor.GetData( &lEmissiveFactor, sizeof(double), pEvalInfo );
	pMaterial->AmbientFactor.GetData( &lAmbientFactor, sizeof(double), pEvalInfo );
	pMaterial->DiffuseFactor.GetData( &lDiffuseFactor, sizeof(double), pEvalInfo );
	pMaterial->SpecularFactor.GetData( &lSpecularFactor, sizeof(double), pEvalInfo );
	pMaterial->TransparencyFactor.GetData( &lTransparencyFactor, sizeof(double), pEvalInfo );
	pMaterial->ReflectionFactor.GetData( &lReflectionFactor, sizeof(double), pEvalInfo );

	// TODO: don't remember why to reassign Shininess, bug ?!
	double shininess = 0.0; // pMaterial->Shininess;
	pMaterial->Shininess.GetData( &shininess, sizeof(double), pEvalInfo );
	//pMaterial->Shininess = shininess;

	mat.emissiveColor = vec4( (float)lEmissive[0], (float)lEmissive[1], (float)lEmissive[2], (float)lEmissiveFactor);
	mat.ambientColor = vec4((float)lAmbient[0], (float)lAmbient[1], (float)lAmbient[2], (float)lAmbientFactor);
	mat.diffuseColor = vec4((float)lDiffuse[0], (float)lDiffuse[1], (float)lDiffuse[2], (float)lDiffuseFactor);
	mat.reflectColor = vec4((float)lReflect[0], (float)lReflect[1], (float)lReflect[2], (float)lReflectionFactor);
	mat.specexp = (float) shininess;
	mat.specularColor = vec4((float)lSpecular[0], (float)lSpecular[1], (float)lSpecular[2], (float)lSpecularFactor);
	mat.transparencyColor = vec4((float)lTransparency[0], (float)lTransparency[2], (float)lTransparency[2], (float)lTransparencyFactor);
	
	// PBR params
	double roughness = 0.0;
	double metal = 0.0;

	FBProperty *pProp = pMaterial->PropertyList.Find( "roughness" );
	if (nullptr != pProp) pProp->GetData( &roughness, sizeof(double) );
	pProp = pMaterial->PropertyList.Find( "metal" );
	if (nullptr != pProp) pProp->GetData( &metal, sizeof(double) );

	mat.roughness = 0.01f * (float) roughness;
	mat.metal = 0.01f * (float) metal;

	//
	mat.useAmbient = 0.0f;
	mat.useDiffuse = 0.0f;
	mat.useNormalmap = 0.0f;
	mat.useReflect = 0.0f;
	mat.useEmissive = 0.0f;
	mat.useSpecular = 0.0f;
	mat.useTransparency = 0.0f;

	if (nullptr == pTexInspector)
		return;

	// AO term
	if (pMaterial->GetTexture(kFBMaterialTextureAmbient) )
	{
		FBTexture *ptexture = pMaterial->GetTexture(kFBMaterialTextureAmbient);
		int index = pTexInspector->FindResourceIndex(ptexture);

		if (index >= 0)
		{
			mat.ambient = index;
			mat.useAmbient = 1.0f;
		}
	}

	// diffuse map
	if (pMaterial->GetTexture() )
	{
		FBTexture *ptexture = pMaterial->GetTexture();
		int index = pTexInspector->FindResourceIndex(ptexture);

		if (index >= 0)
		{
			mat.diffuse = index;
			mat.useDiffuse = 1.0f;
		}
	}
	// reflection map
	if (pMaterial->GetTexture(kFBMaterialTextureReflection) )
	{
		FBTexture *ptexture = pMaterial->GetTexture(kFBMaterialTextureReflection);
		int index = pTexInspector->FindResourceIndex(ptexture);

		if (index >= 0)
		{
			mat.reflect = index;
			mat.useReflect = 1.0f;
		}
	}
	// transparency map
	if (pMaterial->GetTexture(kFBMaterialTextureTransparent) )
	{
		FBTexture *ptexture = pMaterial->GetTexture(kFBMaterialTextureTransparent);
		int index = pTexInspector->FindResourceIndex(ptexture);

		if (index >= 0)
		{
			mat.transparency = index;
			mat.useTransparency = 1.0f;
		}
	}
	// normal map
	if (pMaterial->GetTexture(kFBMaterialTextureNormalMap) )
	{
		FBTexture *ptexture = pMaterial->GetTexture(kFBMaterialTextureNormalMap);
		int index = pTexInspector->FindResourceIndex(ptexture);

		if (index >= 0)
		{
			mat.normalmap = index;
			mat.useNormalmap = 1.0f;
		}
	}
	// emissive map
	if (pMaterial->GetTexture(kFBMaterialTextureEmissive) )
	{
		FBTexture *ptexture = pMaterial->GetTexture(kFBMaterialTextureEmissive);
		int index = pTexInspector->FindResourceIndex(ptexture);

		if (index >= 0)
		{
			mat.emissive = index;
			mat.useEmissive = 1.0f;
		}
	}
	// specular map
	if (pMaterial->GetTexture(kFBMaterialTextureSpecular) )
	{
		FBTexture *ptexture = pMaterial->GetTexture(kFBMaterialTextureSpecular);
		int index = pTexInspector->FindResourceIndex(ptexture);

		if (index >= 0)
		{
			mat.specular = index;
			mat.useSpecular = 1.0f;
		}
	}

//	void *textures = nullptr;
	/*
	mat.useDiffuse = 0.0f;

	if (pMaterial->GetTexture())
	{
		FBTexture *pTexture = pMaterial->GetTexture();
		if (mTexturesInspector)
		{
			mat.diffuse = mTexturesInspector->FindResourceIndex( pTexture );
		}
		else
		{
			mat.diffuse = 0;
		}
		mat.useDiffuse = (mat.diffuse >= 0) ? 1.0f : 0.0f;
	}
	*/
	/*
	else
	if (textures)
	{
		FBTexture *lTexture = nullptr;

		// DIFFUSE map channel
		lTexture = pMaterial->GetTexture();
		if (lTexture) 
		{
			mat.useDiffuse = 1.0f;
			//GLuint id = lTexture->GetTextureObject();
			//mat.diffuse = textures->GetTextureGPUPtr( id );
			mat.diffuse = textures->GetTextureIndex(lTexture);
			if (mat.diffuse < 0)
				mat.diffuse = 0;
		}

		// SPECULAR
		lTexture = pMaterial->GetTexture(kFBMaterialTextureSpecular);
		if (lTexture) {
			mat.useSpecular = 1.0f;
			//mat.specular = textures->GetTextureGPUPtr( lTexture->GetTextureObject() );
			//mat.specular = lTexture->GetTextureObject();
			mat.specular = textures->GetTextureIndex(lTexture);
			if (mat.specular < 0)
				mat.specular = 0;
		}

		// EMISSIVE
		lTexture = pMaterial->GetTexture(kFBMaterialTextureEmissive);
		if (lTexture) {
			mat.useEmissive = 1.0f;
			//mat.emissive = textures->GetTextureGPUPtr( lTexture->GetTextureObject() );
			//mat.emissive = lTexture->GetTextureObject();
			mat.emissive = textures->GetTextureIndex(lTexture);
			if (mat.emissive < 0)
				mat.emissive = 0;
		}

		// TRANSPARENCY
		lTexture = pMaterial->GetTexture(kFBMaterialTextureTransparent);
		if (lTexture) {
			mat.useTransparency = 1.0f;
			//mat.transparency = textures->GetTextureGPUPtr( lTexture->GetTextureObject() );
			mat.transparency = textures->GetTextureIndex(lTexture);
			if (mat.transparency < 0)
				mat.transparency = 0;
		}

		// NORMAL MAP
		lTexture = pMaterial->GetTexture(kFBMaterialTextureNormalMap);
		if (lTexture) {
			mat.useNormalmap = 1.0f;
			//mat.normalmap = textures->GetTextureGPUPtr( lTexture->GetTextureObject() );
			//mat.normalmap = lTexture->GetTextureObject();
			mat.normalmap = textures->GetTextureIndex(lTexture);
			if (mat.normalmap < 0)
				mat.normalmap = 0;
		}
	
		// REFLECTION MAP
		lTexture = pMaterial->GetTexture(kFBMaterialTextureReflection);
		if (lTexture) {
			mat.useReflect = 1.0f;
			//mat.reflect = textures->GetTextureGPUPtr( lTexture->GetTextureObject() );
			//mat.reflect = lTexture->GetTextureObject();
			mat.reflect = textures->GetTextureIndex(lTexture);
			if (mat.reflect < 0)
				mat.reflect = 0;
		}
	}
	*/
}

void CMaterialsInspector::AttachTexture( FBMaterial *pMaterial, FBTexture *ptexture )
{
	if (mTexturesInspector == nullptr)
		return;
	/*
	int matIndex = FindResourceIndex(pmaterial);
	int texIndex = mTexturesInspector->FindResourceIndex(ptexture);

	if (matIndex >= 0 && texIndex >= 0)
	{
		mGLSLResource[matIndex].useDiffuse = 1.0f;	
		mGLSLResource[matIndex].diffuse = texIndex;

		NeedUpdate(matIndex, false);
	}
	*/
	const int matIndex = FindResourceIndex(pMaterial);
	if (matIndex >= 0)
		NeedUpdateByIndex(matIndex, false, false);

}

void CMaterialsInspector::DetachTexture( FBMaterial *pmaterial, FBTexture *ptexture )
{
	if (mTexturesInspector == nullptr)
		return;
	/*
	int matIndex = FindResourceIndex(pmaterial);
	int texIndex = mTexturesInspector->FindResourceIndex(ptexture);

	if (matIndex >= 0 && texIndex >= 0)
	{
		mGLSLResource[matIndex].useDiffuse = 0.0f;	
		mGLSLResource[matIndex].diffuse = 0;

		NeedUpdate(matIndex, false);
	}
	*/
	const int matIndex = FindResourceIndex(pmaterial);
	if (matIndex >= 0)
		NeedUpdateByIndex(matIndex, false, false);
}

void CMaterialsInspector::UpdateAll(FBEvaluateInfo *pEvalInfo)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	int count = pScene->Materials.GetCount();
		
	MaterialGLSL defMaterial;
	DefaultMaterial(defMaterial);

	mResourceVector.resize(count+1);
	mUpdateVector.resize(count+1);
	mGLSLResource.resize(count+1);
	//mConnectionVector.resize(count+1);
	mResourceMap.clear();

	mUpdateVector[0] = RESOURCE_SYSTEM;
	mResourceVector[0] = nullptr;
	mGLSLResource[0] = defMaterial;
	//EmptyConnection(mConnectionVector[0]);

	for (int i=1; i<=count; ++i)
	{
		FBMaterial *pMaterial = pScene->Materials[i-1];
			
		mResourceMap[pMaterial] = i;
		mResourceVector[i] = pMaterial;
			
		//EmptyConnection(mConnectionVector[i]);

		ConvertResource(pMaterial, pEvalInfo, mGLSLResource[i], false);

		mUpdateVector[i] = 0;
		if ( IsAnimated(pMaterial) )
			mUpdateVector[i] |= RESOURCE_ANIMATED;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//


void CModelsInspector::UpdateNormalMatrices( const double  *modelview )
{
	// SIMD version
	
	glm::mat4 glmMV;

	const double *ptr = modelview;
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
		{
			glmMV[i][j] = (float) *ptr;
			ptr++;
		}

	glm::detail::fmat4x4SIMD simdMV(glmMV);

	std::vector<ModelGLSL>::iterator iter = begin(mGLSLResource);
	std::vector<FBModel*>::iterator modelIter = begin(mResourceVector);
	std::vector<int>::iterator flagIter = begin(mUpdateVector);

	for ( ;
		iter != end(mGLSLResource); 
		++iter, ++modelIter, ++flagIter)
	{
		//mat4 tm(iter->transform);

		if ( *flagIter & RESOURCE_DELETED
					|| *flagIter & RESOURCE_SYSTEM )
		{
			continue;
		}

		FBMatrix modelMatrix;
		(*modelIter)->GetMatrix(modelMatrix, kModelTransformation_Geometry);

		for (int i=0; i<16; ++i)
			iter->transform.mat_array[i] = (float) modelMatrix[i];

		glm::mat4 glmTM;
		
		memcpy( glm::value_ptr(glmTM), iter->transform.mat_array, sizeof(float)*16);

		glm::detail::fmat4x4SIMD simdTM(glmTM);
			
		glm::detail::fmat4x4SIMD m = simdMV * simdTM; // glm::matrixCompMult(simdMV, simdTM);
		m[3] = glm::detail::fvec4SIMD(0.0f, 0.0f, 0.0f, 1.0f);

		//m = glm::inverse(m);
		glm::detail::sse_inverse_ps(&m[0].Data, &simdTM[0].Data);
		m = glm::transpose(simdTM);

		glmTM = glm::mat4_cast(m);

		memcpy( &iter->normalMatrix.mat_array[0], glm::value_ptr(glmTM), sizeof(float) * 16 );
	}
	
	// classic version
	/*
	FBMatrix MV(modelview);

	std::vector<FBModel*>::iterator modelIter=begin(mResourceVector);
	std::vector<int>::iterator flagIter = begin(mUpdateVector);

	for (auto iter=begin(mGLSLResource); 
		iter!=end(mGLSLResource); 
		++iter, ++modelIter, ++flagIter)
	{

		if ( *flagIter & RESOURCE_DELETED
					|| *flagIter & RESOURCE_SYSTEM )
		{
			continue;
		}

		FBMatrix modelMatrix, normalMatrix;
		(*modelIter)->GetMatrix(modelMatrix, kModelTransformation_Geometry);

		FBMatrixMult(normalMatrix, MV, modelMatrix ); 
		//(*modelIter)->GetMatrix( normalMatrix, kModelInverse_Transformation );
		FBMatrixInverse( normalMatrix, normalMatrix );
		FBMatrixTranspose( normalMatrix, normalMatrix );

		for (int i=0; i<16; ++i)
		{
			iter->transform.mat_array[i] = (float) modelMatrix[i];
			iter->normalMatrix.mat_array[i] = (float) normalMatrix[i];
		}
	}
	*/
}


void CModelsInspector::SortModels()
{

	const auto &shaderGroups = mShadersFactory.GetShaderTypesVector();
	const size_t numberOfGroups = shaderGroups.size();
	//const size_t numberOfModels = mResourceVector.size();

	if (mSortedModels.size() == 0)
	{
		mSortedModels.resize( numberOfGroups );
		mSortedCounts.resize( numberOfGroups );
	}

	std::fill( begin(mSortedCounts), end(mSortedCounts), 0 );
		
	for (size_t ngroup=0; ngroup < numberOfGroups; ++ngroup )
	{
		//mSortedMeshes[ngroup].resize( numberOfMeshes );
		//std::fill( begin(mSortedMeshes[ngroup]), end(mSortedMeshes[ngroup]), -1 );
		mSortedModels[ngroup].clear();
	}

	//
	for (auto iter=begin(mModelMeshesMap);
		iter!=end(mModelMeshesMap);
		++iter)
	{
		
		FBModel *pModel = iter->first;
		const int meshIndex = iter->second;

		//FBShader *pShader = nullptr;
		//if (pModel->Shaders.GetCount() > 0)
		//	pShader = pModel->Shaders[0];

		const int numShaders = pModel->Shaders.GetCount();

		if (numShaders > 0)
		{
			for (int i=0, numShaders=pModel->Shaders.GetCount(); i<numShaders; ++i)
			{
				FBShader *pShader = pModel->Shaders[i];

				// find a callback for that shader
				if ( FBIS(pShader, FXShadingShader) || FBIS(pShader, FXColorCorrectionShader)
					|| FBIS(pShader, FXProjectionMapping) )
				{
					printf ( "what these shaders are here ?!\n" );
					continue;
				}

				CBaseShaderCallback *pCallback = mShadersFactory.FindTypeByShaderAndGoal(pShader, eRenderGoalShading);
		
				if (nullptr != pCallback)
				{
					const int id = pCallback->Id();
					const int index = mSortedCounts[id]; 
			
					//mSortedMeshes[id][index] = meshIndex;
					SortModelWrap wrap = {meshIndex, pModel};
					mSortedModels[id].insert( std::make_pair(pShader, wrap) );
					mSortedCounts[id] = index + 1;
				}
			}
		}
		else
		{
			FBShader *pShader = nullptr;
			CBaseShaderCallback *pCallback = mShadersFactory.FindTypeByShaderAndGoal(pShader, eRenderGoalShading);
		
			if (nullptr != pCallback)
			{
				const int id = pCallback->Id();
				const int index = mSortedCounts[id]; 
			
				//mSortedMeshes[id][index] = meshIndex;
				SortModelWrap wrap = {meshIndex, pModel};
				mSortedModels[id].insert( std::make_pair(pShader, wrap) );
				mSortedCounts[id] = index + 1;
			}
		}
	}
}

// used for calculate size for array allocation
void CModelsInspector::CalculateModelCount(FBModel *parentModel, int &modelCount, int &meshCount)
{
	if (true == FilterComponentHard(parentModel) )
	{
		FBModelVertexData *pVertexData = parentModel->ModelVertexData;

		if (nullptr != pVertexData)
		{
			modelCount += 1;
			meshCount += pVertexData->GetSubPatchCount();
		}
		else
		{
			// wait until this model will have a render vertex data !
			mPendingResource.push_back(parentModel);
		}
	}

	for( int i=0, count=parentModel->Children.GetCount(); i<count; ++i)
	{
		CalculateModelCount( parentModel->Children[i], modelCount, meshCount );
	}
}

// go through scene models and fillup our arrays with models and meshes information
void CModelsInspector::EnumerateModelAndMesh(FBModel *parentModel, int &modelIndex)
{
	if (true == FilterComponentHard(parentModel) )
	{
		FBModelVertexData *pVertexData = parentModel->ModelVertexData;
		if (nullptr != pVertexData)
		{
			mResourceVector[modelIndex] = parentModel;
			modelIndex += 1;
		}
	}

	for( int i=0, count=parentModel->Children.GetCount(); i<count; ++i)
	{
		EnumerateModelAndMesh( parentModel->Children[i], modelIndex );
	}
}

void CModelsInspector::UpdateAll(FBEvaluateInfo *pEvalInfo)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	int modelCount = 0;
	int meshCount = 0;
	
	if (mShadersInspector)
		mShadersInspector->RemoveAllCombinations();

	CalculateModelCount(pScene->RootModel, modelCount, meshCount);

	mResourceVector.resize(modelCount);
	mUpdateVector.resize(modelCount);
	mGLSLResource.resize(modelCount);
	mModelMeshesVector.resize(modelCount);
	mModelMeshesMap.clear();

	if (modelCount != mResourceMap.size() )
	{
		mResourceMap.clear();
	}

	mGLSLMesh.resize(meshCount);

	// prepare mResourceVector data
	modelCount = 0;
	EnumerateModelAndMesh(pScene->RootModel, modelCount);

	meshCount = 0;
	for (int i=0, count=(int)mResourceVector.size(); i<count; ++i)
	{
		FBModel *pModel = mResourceVector[i];
			
		mResourceMap[pModel] = i;
		mModelMeshesMap[pModel] = meshCount;
		mModelMeshesVector[i] = meshCount;

		ConvertResource(pModel, pEvalInfo, mGLSLResource[i], false);

		mUpdateVector[i] = 0;
		if ( IsAnimated(pModel) )
			mUpdateVector[i] |= RESOURCE_ANIMATED;

		//
		// model shader
		
		int shaderIndex = FindShaderIndex(pModel);
		FBColor colorId = pModel->UniqueColorId;
		int receiveShadows = pModel->ReceiveShadows.AsInt();

		//
		// model meshes

		FBModelVertexData *pdata = pModel->ModelVertexData;
		for (int patchIndex=0, patchCount=pdata->GetSubPatchCount(); patchIndex<patchCount; ++patchIndex)
		{
			MeshGLSL &meshdata = mGLSLMesh[meshCount];

			DefaultGLSLMesh( meshdata );

			meshdata.model = i;
			meshdata.shader = shaderIndex;
			meshdata.lightmap = receiveShadows;
			meshdata.color = vec4( (float)colorId[0], (float)colorId[1], (float)colorId[2], 1.0);

			FBMaterial *pMaterial = pdata->GetSubPatchMaterial(patchIndex);
			if (mMaterialsInspector && pMaterial)
			{
				meshdata.material = mMaterialsInspector->FindResourceIndex(pMaterial);
				if (meshdata.material < 0)
					meshdata.material = 0;
			}

			meshCount += 1;
		}
	}

	mNeedToSortModels = true;
}

bool CModelsInspector::Add( FBModel *pModel, bool addToPendingStack )
{
	// we need name to exclude "Motion Blend Result" FBModel from inspector
	FBString name(pModel->LongName);

	if (true == mNeedFullUpdate)
		return false;

	if (pModel->HasObjectFlags( kFBFlagSystem ) )
		return false;

	if (FBIS(pModel, FBCamera) || FBIS(pModel, FBLight) )
		return false;

	if (strcmp(name, "Motion Blend Result") == 0)
		return false;

	//FBModelVertexData *pVertexData = pModel->ModelVertexData;
	//if (nullptr == pVertexData)
	//	return false;
	
	//HdlFBPlugTemplate<FBModel>	checkModel(pModel);
	if (false == CResourceInspector::Add(pModel, addToPendingStack) )
		return false;

	// TODO: add a new model and all it's mesh

	int modelIndex = (int) mGLSLResource.size() - 1; // latest element
	int meshCount = (int) mGLSLMesh.size();

	mModelMeshesMap[pModel] = meshCount;
	mModelMeshesVector.push_back(meshCount);
	mUpdateVector[modelIndex] |= RESOURCE_SUBDATA_UPDATE;

	//
	// model shader

	int shaderIndex = FindShaderIndex(pModel);
	FBColor colorId = pModel->UniqueColorId;
	int receiveShadows = pModel->ReceiveShadows.AsInt();

	//
	// model meshes

	FBModelVertexData *pdata = pModel->ModelVertexData;
	for (int patchIndex=0, patchCount=pdata->GetSubPatchCount(); patchIndex<patchCount; ++patchIndex)
	{
		MeshGLSL meshdata;

		DefaultGLSLMesh( meshdata );

		meshdata.model = modelIndex;
		meshdata.shader = shaderIndex;
		meshdata.color = vec4( (float)colorId[0], (float)colorId[1], (float)colorId[2], 1.0);
		meshdata.lightmap = receiveShadows;

		FBMaterial *pMaterial = pdata->GetSubPatchMaterial(patchIndex);
		if (mMaterialsInspector && pMaterial)
		{
			meshdata.material = mMaterialsInspector->FindResourceIndex(pMaterial);
			if (meshdata.material < 0)
				meshdata.material = 0;
		}

		meshCount += 1;

		mGLSLMesh.push_back(meshdata);
	}

	mNeedToSortModels = true;
	return true;
}

void CModelsInspector::Delete( FBModel *mbitem )
{
	FBString name(mbitem->LongName);
	CResourceInspector::Delete(mbitem);

	std::map<FBModel*, int>::iterator iter = mModelMeshesMap.find(mbitem);
	if( iter != end(mModelMeshesMap) )
	{
		mModelMeshesMap.erase(iter);
	}

	mNeedToSortModels = true;
}

void CModelsInspector::NeedUpdate( FBModel *pModel, bool animated, bool subdata )
{
	// TODO: temproary I'm doing model matrix update each frame
	CResourceInspector::NeedUpdate(pModel, animated, subdata);
	/*
	for (int i=0, count=pModel->Children.GetCount(); i<count; ++i)
		NeedUpdate( pModel->Children[i], animated );
		*/
}

// TODO:
void CModelsInspector::AttachMaterial(FBModel *pModel, FBMaterial *pMaterial)
{
	if (mMaterialsInspector == nullptr)
		return;

	if (true == mNeedFullUpdate)
		return;

	int modelIndex = FindResourceIndex(pModel);
	int meshIndex = mModelMeshesMap[pModel];

	if (modelIndex >=0 && meshIndex >= 0)
	{
		FBModelVertexData *pdata = pModel->ModelVertexData;
		for (int patchIndex=0, patchCount=pdata->GetSubPatchCount(); patchIndex<patchCount; ++patchIndex)
		{
			MeshGLSL &meshdata = mGLSLMesh[meshIndex];

			FBMaterial *pMaterial = pdata->GetSubPatchMaterial(patchIndex);
			if (mMaterialsInspector && pMaterial)
			{
				meshdata.material = mMaterialsInspector->FindResourceIndex(pMaterial);
				if (meshdata.material < 0)
					meshdata.material = 0;
			}

			meshIndex += 1;
		}

		int &updateFlag = mUpdateVector[modelIndex];
		if ( (updateFlag & RESOURCE_SUBDATA_UPDATE) == 0 )
			updateFlag |= RESOURCE_SUBDATA_UPDATE;
	}
}

void CModelsInspector::DetachMaterial(FBModel *pModel, FBMaterial *pMaterial)
{
	AttachMaterial(pModel, pMaterial);
}

// TODO: direct textures to model is reflection maps, light maps, etc... not supported yet
void CModelsInspector::AttachTexture(FBModel *pModel, FBTexture *pTexture)
{
}

void CModelsInspector::DetachTexture(FBModel *pModel, FBTexture *pTexture)
{
}

const int CModelsInspector::FindShaderIndex(FBModel *pModel)
{
	FBShader *pshader = nullptr;
	if (pModel->Shaders.GetCount() > 0)
		pshader = pModel->Shaders[0];

	int shaderIndex = 0;
	if (mShadersInspector && pshader)
	{
		if (pModel->Shaders.GetCount() > 1)
		{
			FBShader *shaders[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
			int shaderCount = pModel->Shaders.GetCount();
			if (shaderCount > 5)
				shaderCount = 5;

			for (int i=0; i<shaderCount; ++i)
				shaders[i] = pModel->Shaders[i];

			shaderIndex = (int)mShadersInspector->GetNumberOfItems() + mShadersInspector->FindCombination(shaders);
		}
		else
		{
			shaderIndex = mShadersInspector->FindResourceIndex(pshader);
		}

		if (shaderIndex < 0)
			shaderIndex = 0;
	}

	return shaderIndex;
}

void CModelsInspector::AttachShader(FBModel *pModel, FBShader *pShader)
{
	if (mShadersInspector == nullptr)
		return;

	if (true == mNeedFullUpdate)
		return;

	int modelIndex = FindResourceIndex(pModel);
	int meshIndex = mModelMeshesMap[pModel];

	//
	// model shader
	
	int shaderIndex = FindShaderIndex(pModel);

	if (modelIndex >=0 && meshIndex >= 0)
	{
		FBModelVertexData *pdata = pModel->ModelVertexData;
		for (int patchIndex=0, patchCount=pdata->GetSubPatchCount(); patchIndex<patchCount; ++patchIndex)
		{
			MeshGLSL &meshdata = mGLSLMesh[meshIndex];

			meshdata.shader = shaderIndex;

			meshIndex += 1;
		}

		int &updateFlag = mUpdateVector[modelIndex];
		if ( (updateFlag & RESOURCE_SUBDATA_UPDATE) == 0 )
			updateFlag |= RESOURCE_SUBDATA_UPDATE;
	}

	mNeedToSortModels = true;
}

void CModelsInspector::DetachShader(FBModel *pModel, FBShader *pShader)
{
	AttachShader(pModel, pShader);

	mNeedToSortModels = true;
}

void CModelsInspector::UpdateConveyer()
{
	if (nullptr == mRenderConveyer.get() )
		mRenderConveyer.reset( new CRenderVertexConveyer() );

	mRenderConveyer->AllocateFromFBModels( mResourceVector, mUpdateVector );
}

void CModelsInspector::UpdateDeformations(FBEvaluateInfo *pEvaluateInfo)
{
	if (mRenderConveyer.get() )
		mRenderConveyer->UpdateAllModelsSubData(pEvaluateInfo);
}