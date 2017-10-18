
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ContentInspector.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <vector>
#include <map>

#include <stack>
#include <algorithm>

#include "graphics\UniformBuffer.h"
#include "graphics\OGL_Utils.h"
#include "shared_glsl.h"

#include "..\Common\Common_shader.h"

#include "callback_factory.h"
#include "callbacks.h"

#include "render_conveyer.h"
#include "render_layer_info.h"

// TODO: compute 10 remove items and start full update !!

// check which item we should update next frame, check for animated properties

#define RESOURCE_SINGLE_UPDATE	2
#define RESOURCE_ANIMATED		4
#define RESOURCE_DELETED		8
#define RESOURCE_SYSTEM			16
#define RESOURCE_SUBDATA_UPDATE	32		// update meshes data (attach/detach material, shader) inside a model

// for inspector classification
#define RESOURCE_NONE			0
#define	RESOURCE_TEXTURE		1
#define RESOURCE_SHADER			2
#define RESOURCE_MATERIAL		3


//
void ConstructFromFBShader( FBShader *pShader, ShaderGLSL &shader );
//void ConstructFromFBMaterial( FBMaterial *pMaterial, MaterialGLSL &mat );

////////////////////////////////////////////////////////////////////////////////////////
//

template <typename MBT, typename GLT>
class CResourceInspector
{
protected:

	virtual void DefaultGLSLResource( GLT &item )
	{}
	virtual void ConvertResource( MBT *mbitem, FBEvaluateInfo *pEvalInfo, GLT &item, bool subdata )
	{}

	virtual bool IsAnimated( MBT *mbitem ) {
		return false;
	}

	virtual bool FilterComponentHard( MBT *mbitem ) {
		return true;
	}

	/// this filtering could be added to the pending stack to be processed one more time during rendering preparation

	virtual bool FilterComponentLight( MBT *mbitem ) {
		return true;
	}

	// Hard update
	virtual void UpdateAll(FBEvaluateInfo *pEvalInfo)
	{}

	virtual bool ProcessNeedFullUpdate()
	{
		return false;
	}

	virtual void OnProcessFinished(bool updated)
	{}

public:

	// a constructor
	CResourceInspector()
	{
		mNeedFullUpdate = false;
	}

	// a destructor
	virtual ~CResourceInspector()
	{}

	virtual int	GetResourceType() const {
		return RESOURCE_NONE;
	}

	// callbacks for other classes to sync resources
	virtual void OnItemAdd( MBT *mbitem )
	{}

	virtual void OnItemDelete( MBT *mbitem)
	{}

	void CheckAnimatedFlag( MBT *mbitem )
	{
		auto iter = mResourceMap.find(mbitem);

		if (iter != end(mResourceMap) )
		{
			int &updateFlag = mUpdateVector[iter->second];

			if (IsAnimated(mbitem) )
			{
				if ( (updateFlag & RESOURCE_ANIMATED) == 0)
					updateFlag |= RESOURCE_ANIMATED;
			}
			else
			{
				if (updateFlag & RESOURCE_ANIMATED)
					updateFlag &= ~RESOURCE_ANIMATED;
			}
		}
	}

	void NeedUpdateByIndex( int itemIndex, bool animated, bool subdata )
	{
		int &updateFlag = mUpdateVector[itemIndex];

		if (animated)
			updateFlag = RESOURCE_SINGLE_UPDATE | RESOURCE_ANIMATED;
		else if ( (updateFlag & RESOURCE_SINGLE_UPDATE) == 0)
			updateFlag |= RESOURCE_SINGLE_UPDATE;
		
		if (subdata && (updateFlag & RESOURCE_SUBDATA_UPDATE) == 0)
			updateFlag |= RESOURCE_SUBDATA_UPDATE;
	}

	

	virtual void NeedUpdate( MBT *mbitem, bool animated, bool subdata )
	{
		if (false == FilterComponentHard(mbitem) )
			return;

		if (true == mNeedFullUpdate)
			return;

		auto iter = mResourceMap.find(mbitem);

		if (iter != end(mResourceMap) )
		{
			NeedUpdateByIndex(iter->second, animated, subdata);
		}
		else
		{
			// !!
			FBString name(mbitem->Name);
			printf ("how to deal with a missing resource - %s?!\n", name);

			bool isPending = false;
			
			auto iter = std::find( begin(mPendingResource), end(mPendingResource), mbitem );
			if (iter != end(mPendingResource) )
			{
				isPending = true;
				printf ("  > the resource is pending\n" );
			}
			
			// let's try to add if not exist !
			if (isPending == false)
			{
				Add(mbitem);
			}
		}
	}

	void NeedFullUpdate()
	{
		mNeedFullUpdate = true;
	}
	const bool IsFullUpdateNeeded() const {
		return mNeedFullUpdate;
	}

	void ProcessPendingResource()
	{
		size_t count= 0;
		for (auto iter = begin(mPendingResource); iter!=end(mPendingResource); ++iter)
		{
			if (nullptr == *iter)
				continue;

			if (false == Add( *iter, false ) )
				count += 1;
			else
				*iter = nullptr;
		}

		if (count == 0)
			mPendingResource.clear();
	}

	bool Process(FBEvaluateInfo *pEvalInfo)
	{
		mSubDataUpdated = false;
		bool updated = false;

		if (true == ProcessNeedFullUpdate() )
			mNeedFullUpdate = true;

		if (mNeedFullUpdate)
		{
			mPendingResource.clear();

			UpdateAll(pEvalInfo);
			mNeedFullUpdate = false;
			mSubDataUpdated = true;

			auto iter=begin(mGLSLResource);
			auto resource = begin(mResourceVector);
			auto updateIter=begin(mUpdateVector);

			for (; iter!=end(mGLSLResource); ++iter, ++resource, ++updateIter)
			{
				int updateFlag = *updateIter;
				if (updateFlag & RESOURCE_DELETED
					|| updateFlag & RESOURCE_SYSTEM)
					continue;

				ConvertResource(*resource, pEvalInfo, *iter, false);

				if (updateFlag & RESOURCE_SINGLE_UPDATE)
					updateFlag &= ~RESOURCE_SINGLE_UPDATE;
				*updateIter = updateFlag;
			}
			updated = true;
		}
		else
		{
			ProcessPendingResource();

			auto updateIter=begin(mUpdateVector);
			auto glslIter = begin(mGLSLResource);
			auto resource = begin(mResourceVector);

			for ( ; updateIter!=end(mUpdateVector); ++updateIter, ++glslIter, ++resource)
			{
				int updateFlag = *updateIter;

				if (updateFlag & RESOURCE_DELETED
					|| updateFlag & RESOURCE_SYSTEM)
					continue;

				if (updateFlag & RESOURCE_SUBDATA_UPDATE)
				{
					mSubDataUpdated = true;
					updateFlag &= ~RESOURCE_SUBDATA_UPDATE;
					ConvertResource(*resource, pEvalInfo, *glslIter, true);
					// TODO: check if that is correct ?!
					*updateIter = updateFlag;
				}

				if (updateFlag & RESOURCE_SINGLE_UPDATE)
				{
					ConvertResource(*resource, pEvalInfo, *glslIter, false);
					updateFlag &= ~RESOURCE_SINGLE_UPDATE;
					*updateIter = updateFlag;
					updated = true;
				}
				else if (updateFlag & RESOURCE_ANIMATED)
				{
					ConvertResource(*resource, pEvalInfo, *glslIter, false);
					updated = true;
				}
			}
		}

		OnProcessFinished(updated);
		return updated;
	}

	bool IsSubDataUpdated() const {
		return mSubDataUpdated;
	}

	int FindResourceIndex( MBT *mbitem )
	{
		auto iter = mResourceMap.find(mbitem);

		if (iter != end(mResourceMap) )
		{
			return iter->second;
		}

		return -1;
	}

	void Clear()
	{
		mResourceMap.clear();
		mResourceVector.clear();
		mGLSLResource.clear();
		mUpdateVector.clear();
	}

	virtual bool Add( MBT *mbitem, bool addToPendingStack=true )
	{
		if (false == FilterComponentHard(mbitem) )
		{
			return false;
		}

		if (false == FilterComponentLight(mbitem) )
		{
			if (addToPendingStack)
				mPendingResource.push_back(mbitem);

			return false;
		}

		size_t index = mGLSLResource.size();

		mResourceMap[mbitem] = (int) index;

		GLT newItem;
		DefaultGLSLResource( newItem );
		mGLSLResource.push_back(newItem);

		int updateFlag = RESOURCE_SINGLE_UPDATE;
		if (IsAnimated(mbitem) )
			updateFlag |= RESOURCE_ANIMATED;

		mUpdateVector.push_back(updateFlag);
		mResourceVector.push_back(mbitem);

		OnItemAdd( mbitem );

		return true;
	}

	virtual void Delete( MBT *mbitem )
	{
		auto iter = mResourceMap.find(mbitem);

		if (iter != end(mResourceMap) )
		{
			int &updateFlag = mUpdateVector[iter->second];
			if ( (updateFlag & RESOURCE_SYSTEM) == 0)
			{
				updateFlag = RESOURCE_DELETED;

				OnItemDelete(mbitem);
			}

			// TODO: if we already put a RESOURCE_DELETED flag, why do we need to erase element here ??!!
			mResourceMap.erase(iter);
		}

		auto pendingIter = std::find(begin(mPendingResource), end(mPendingResource), mbitem);
		if (pendingIter != end(mPendingResource) )
		{
			mPendingResource.erase(pendingIter);
		}
	}

	/*
	// global resize
	void ReSize(const size_t size)
	{
		
		mGLSLResource.resize(size);

		mNeedFullUpdate = true;
	}
	*/

	size_t GetNumberOfItems() const {
		return mGLSLResource.size();
	}
	const GLT *GetData() const {
		return mGLSLResource.data();
	}

	const std::vector<MBT*> &GetResourceVector() const {
		return mResourceVector;
	}

	const std::vector<int> &GetUpdateVector() const {
		return mUpdateVector;
	}

	bool IsDeleted(const int index) {
		return (mUpdateVector[index] & RESOURCE_DELETED);
	}

	/*
	int GetNumberOfConnections(int itemIndex, int resourceType) const
	{
		int count = 0;

		if (index >= 0 && index < mConnectionVector.size() )
		{
			ElementConnection &conn = mConnectionVector[itemIndex];

			for (int i=0; i<conn.numberOfconnections; ++i)
			{
				if (conn.connections[i].type == resourceType)
					count += 1;
			}
		}

		return count;
	}

	int GetConnectionAnotherIndex(int itemIndex, int resourceType, int connectionIndex)
	{
		int count = 0;
		if (index >= 0 && index < mConnectionVector.size() )
		{
			ElementConnection &conn = mConnectionVector[itemIndex];

			for (int i=0; i<conn.numberOfconnections; ++i)
			{
				if (conn.connections[i].type == resourceType)
				{
					if (count == connectionIndex)
						return conn.connections[i].index;

					count += 1;
				}
			}
		}

		return -1;
	}

	FBComponent *GetConnectionAnotherComponent(int itemIndex, int resourceType, int connectionIndex)
	{
		int count = 0;
		if (index >= 0 && index < mConnectionVector.size() )
		{
			ElementConnection &conn = mConnectionVector[itemIndex];

			for (int i=0; i<conn.numberOfconnections; ++i)
			{
				if (conn.connections[i].type == resourceType)
				{
					if (count == connectionIndex)
						return conn.connections[i].pcomp;

					count += 1;
				}
			}
		}

		return nullptr;
	}

	// connect this element with the element from another inspector
	bool Connect(int itemIndex, int resourceType, int anotherIndex, FBComponent *anotherElement)
	{
		ElementConnection &conn = mConnectionVector[itemIndex];

		for (int i=0; i<conn.numberOfConnections; ++i)
		{
			// ALREADY CONNECTED !
			if ( conn.connections[i].type == resourceType && conn.connections[i].pcomp == anotherElement )
				return true;
		}

		for (int i=0; i<conn.numberOfConnections; ++i)
		{
			if ( conn.connections[i].type == RESOURCE_NONE )
			{
				conn.connections[i].type = resourceType;
				conn.connections[i].pcomp = anotherElement;
				conn.connections[i].index = anotherIndex;

				return true;
			}
		}	

		return false;
	}

	bool Connect(MBT *mbt, CResourceInspector *anotherInspector, FBComponent *anotherElement)
	{
		auto iter = mResourceMap[mbt];
		if (iter != end(mResourceMap) )
		{
			ElementConnection &conn = mConnectionVector[iter->second];

			for (int i=0; i<conn.numberOfConnections; ++i)
			{
				if ( conn.connections[i].type == RESOURCE_NONE )
				{
					conn.connections[i].type = anotherInspector->GetResourceType();
					conn.connections[i].pcomp = anotherElement;

					int anotherIndex = anotherInspector->FindResourceIndex(anotherElement);
					conn.connections[i].index = anotherIndex;

					return true;
				}
				
			}
		}
		else
		{
			printf ("element is not found!\n");
		}
		return false;
	}

	// disconnect mbt with the specified component
	bool Disconnect(MBT *mbt, FBComponent *anotherElement)
	{
		auto iter = mResourceMap[mbt];
		if (iter != end(mResourceMap) )
		{
			ElementConnection &conn = mConnectionVector[iter->second];

			for (int i=0; i<conn.numberOfConnections; ++i)
			{
				if ( conn.connections[i].type != RESOURCE_NONE 
					&& conn.connections[i].pcomp == anotherElement )
				{
					conn.connections[i].type = RESOURCE_NONE;
					return true;
				}
			}
		}
		else
		{
			printf ("element is not found!\n");
		}

		return false;
	}
	*/
protected:

	bool				mNeedFullUpdate;

	bool				mSubDataUpdated;

	// convert from class pointer to glsl resources index
	std::map<MBT*, int>			mResourceMap;
	std::vector<MBT*>			mResourceVector;

	// flags for update
	std::vector<int>			mUpdateVector;

	// prepared converted GLSL struct vector to load into gpu
	std::vector<GLT>			mGLSLResource;

	// try to add components during render preparation
	std::deque<MBT*>			mPendingResource;

	// connectors for this element - for example material with texture
	//  model <-> mesh, mesh <-> material, model <-> shader
	/*
	struct ElementConnection
	{
		struct Connection
		{
			int				type;	// type of element, determine inspector
			int				index;	// index to element in another inspector
			FBComponent		*pcomp;
		};

		int					numberOfConnections;
		Connection			connections[2];


		void Empty(int index)
		{
			Connection &conn = connectons[index];
			conn.type = RESOURCE_NONE;
			conn.index = -1;
			conn.pcomp = nullptr;
		}
	};

	void EmptyConnection(ElementConnection &con)
	{
		conn.numberOfConnections = 0;
		connections.Empty(0);
		connections.Empty(1);
	}

	
	std::vector<ElementConnection>			mConnectionVector;
	*/
};


///////////////////////////////////////////////////
//

class CTexturesInspector : public CResourceInspector<FBTexture, TextureGLSL>
{
protected:

	virtual void DefaultGLSLResource( TextureGLSL &item )
	{
		item.address = 0;
		item.width = 1;
		item.height = 1;
		item.transform.identity();
		//item.format.w = 1.0f;
	}
	virtual void ConvertResource( FBTexture *mbitem, FBEvaluateInfo *pEvalInfo, TextureGLSL &item, bool subdata ) override
	{
		item.address = 0;
		item.width = mbitem->Width;
		item.height = mbitem->Height;
		
		double *matrix = mbitem->GetMatrix();
		for (int i=0; i<16; ++i)
		{
			item.transform.mat_array[i] = (float) matrix[i];
		}

		// try to get information about video format
		/*
		item.format.w = 1.0f;

		FBVideo *pVideo = mbitem->Video;
		if ( nullptr != pVideo && FBIS(pVideo, FBVideoClip) )
		{
			FBVideoClip *pClip = (FBVideoClip*) pVideo;

			FBVideoFormat format = pClip->Format;
			if (kFBVideoFormat_RGB_24 == format || kFBVideoFormat_BGR_16 == format
				|| kFBVideoFormat_BGR_24 == format )
			{
				item.format.w = 0.0f;
			}
		}
		*/
	}

	virtual bool IsAnimated( FBTexture *mbitem )
	{
		if (mbitem->Translation.IsAnimated()
			|| mbitem->Rotation.IsAnimated()
			|| mbitem->Scaling.IsAnimated() )
		{
			return true;
		}

		return false;
	}

public:

	virtual int	GetResourceType() const {
		return RESOURCE_TEXTURE;
	}

	virtual void UpdateAll(FBEvaluateInfo *pEvalInfo) override
	{
		FBScene *pScene = FBSystem::TheOne().Scene;

		int count = pScene->Textures.GetCount();
		
		mResourceVector.resize(count);
		mUpdateVector.resize(count);
		mGLSLResource.resize(count);
		mResourceMap.clear();

		for (int i=0; i<count; ++i)
		{
			FBTexture *pTexture = pScene->Textures[i];
			mResourceMap[pTexture] = i;
			mResourceVector[i] = pTexture;
			
			ConvertResource(pTexture, pEvalInfo, mGLSLResource[i], false);

			mUpdateVector[i] = 0;
			if ( IsAnimated(pTexture) )
				mUpdateVector[i] |= RESOURCE_ANIMATED;
		}
	}

	void MoBuInitTextures(FBRenderOptions *pRenderOptions) {

		/*
		auto flagIter = begin(mUpdateVector);

		for (auto iter=begin(mResourceVector); 
				iter!=end(mResourceVector); ++iter, ++flagIter)
		{

			if (*flagIter & RESOURCE_DELETED > 0)
				continue;

			FBTexture *pTexture = *iter;
			pTexture->OGLInit(pRenderOptions);
		}
		*/

		FBScene *pScene = FBSystem::TheOne().Scene;

		for (int i=0, count=pScene->Textures.GetCount(); i<count; ++i)
		{
			FBTexture *pTexture = pScene->Textures[i];
			glActiveTexture(GL_TEXTURE0);
			pTexture->OGLInit(pRenderOptions);
		}

	}
};


////////////////////////////////////////////////////////////////////////////////////
//
class CShadersInspector : public CResourceInspector<FBShader, ShaderGLSL>
{
public:

	// this wrap helps to avoid <mat>.find searching info for each shader per pass !
	struct MyFBShaderWrap
	{
		FBShader			*pShader;
		CBaseShaderInfo		*pShaderInfo;
	};

	struct MyFBShaderInfoWrap
	{
		CBaseShaderInfo		*pShaderInfo;
		CBaseShaderCallback	*pCallback;
	};

	const std::vector<CBaseShaderCallback*> &GetShaderTypesVector() const
	{
		return mShadersFactory.GetShaderTypesVector();
	}
	/*
	std::multimap<CBaseShaderCallback*, MyFBShaderWrap> &GetShaderIntancesMultimap() // (int pass)
	{
		return mShaderInstances; // [pass];
	}
	*/
	/*
	CBaseShaderInfo *AllocateANewShaderInfo( FBShader *pShader, CBaseShaderCallback *pCallback )
	{
		CBaseShaderInfo *pShaderInfo = pCallback->CreateANewShaderInfo(pShader);
		MyFBShaderInfoWrap wrap = { pShaderInfo, pCallback };

		mShaderInfos.insert( std::make_pair( pShader, wrap ) );

		return pShaderInfo;
	}
	*/
	CBaseShaderInfo *GetShaderInfo(FBShader *pShader, 
		bool createIfEmtpy, CBaseShaderCallback *pCallback);

	// process pending items, process shader info, full sort if needed

	void EventBeforeRenderNotify();
	void EventConnNotify(HISender pSender, HKEvent pEvent);

	void UpdateGPUBuffer( CGPUBuffer *pBuffer );

	// check if shader has changes important data when we have to re-sort the shader instances
	void EventDataNotify(HISender pSender, HKEvent pEvent);

	void AskToUpdateAllShadersData();
	void ChangeContext();

	CBaseShaderInfo *GetShaderCallbackInfo(const size_t ncallback, const size_t nshader) const
	{
		return mShaderCallbackInfos[ncallback][nshader];
	}

	std::vector<FBShader*>	&GetShaderGroupShaders(const int groupId) {
		return mSortedShaders[groupId];
	}

	const CRenderLayerArray &GetRenderLayersInfo() const {
		return mRenderLayers;
	}

protected:

	bool						mIsCombinationsUpdated;

	std::vector<ShaderGLSL>		mCombinationsVector;

protected:

	// pre-sorted list of shaders

	// how to determine shader type ?
	//  store index and type or put into a lot of arrays

	// multimap where key - callback type (index in the factory)
	//				values - shader instances in the ResourceVector


	//std::multimap<CBaseShaderCallback*, MyFBShaderWrap>		mShaderInstances; //[SHADER_PASS_COUNT];

	// number of shader in each callback
	//int														mShaderCounts; //[SHADER_PASS_COUNT];

	// store additional callback information depends on a shader type
	//  what if shader has several callbacks and each shader-callback combination whant's own shaderInfo
	//std::multimap<FBShader*, MyFBShaderInfoWrap>			mShaderInfos;

	// for each shader, for each call - Callback - FBShader information
	std::vector< std::vector<CBaseShaderInfo*> > mShaderCallbackInfos;

	// for each callback - store vector of shaders
	std::vector< std::vector<FBShader*> >	mSortedShaders;

	void SyncShaderCallbackInfos();

	// sort shaders by shader group id
	void SortShaders();

	CShadersFactory											mShadersFactory;
	
#define	MAX_PENDING_ITEMS		4

	bool	mNeedFullCallbackSort;	// use flag when pending add or del vectors are full

	// shaders that have beed added to a scene
	int mPendingToAddCount;
	FBShader		*mPendingShadersToAdd[MAX_PENDING_ITEMS];

	// shaders that have been removed from the a scene
	int mPendingToDelCount;
	FBShader		*mPendingShadersToDel[MAX_PENDING_ITEMS];
	
	// shaders that have been chagned some important properties that cause re-sort !
	int mPendingUpdateCount;
	FBShader		*mPendingShadersToUpdate[MAX_PENDING_ITEMS];

	// TODO: compute for each render layer

	CRenderLayerArray		mRenderLayers;

	void ClearShaderGroupsInfo();
	void SyncShaderGroups();

	void FullSortShaderCallbacks();

	void PendingAdd(FBShader *pShader);
	void PendingDel(FBShader *pShader);
	void PendingUpdate(FBShader *pShader);

	void ClearPendingData();

	void RemoveShaderInfos(FBShader *pShader);

	// do something on user events

	void SortOnShaderChange( FBShader *pShader );
	
	//	add items to a pending update stack
	// callbacks for other classes to sync resources
	virtual void OnItemAdd( FBShader *pShader ) override;
	virtual void OnItemDelete( FBShader *pShader ) override;
	

public:
	static void ConstructFromFBShader( FBShader *pShader, FBEvaluateInfo *pEvalInfo, ShaderGLSL &shader );

protected:

	virtual void DefaultGLSLResource( ShaderGLSL &item ) override;
	virtual void ConvertResource( FBShader *mbitem, FBEvaluateInfo *pEvalInfo, ShaderGLSL &item, bool subdata ) override;

	virtual bool IsAnimated( FBShader *mbitem ) override;

public:

	//! a constructor
	CShadersInspector();

	virtual int	GetResourceType() const override {
		return RESOURCE_SHADER;
	}

	virtual void UpdateAll(FBEvaluateInfo *pEvalInfo) override;

	const int FindCombination(FBShader **pShaders);
	void UpdateCombinations();
	void RemoveAllCombinations()
	{
		mCombinationsVector.clear();
		mIsCombinationsUpdated = true;
	}

	void ShaderReplace(ShaderGLSL &outshader, const ShaderGLSL &replaceWith);
	void ShaderAppend(ShaderGLSL &outshader, const ShaderGLSL &appendWith);

	const bool IsCombinationUpdated() const {
		return mIsCombinationsUpdated;
	}
	void ResetCombinationUpdateFlag() {
		mIsCombinationsUpdated = false;
	}
};


//////////////////////////////////////////////////
//

// TODO: connection with textures
//  diffuse connection vector - store FBTexture for each texture or index
//	that will help to avoid search inside a map for each update

// then we can use the same thing for the model <-> mesh connection
//		and model <-> shader connection
//			mesh <-> material connection

class CMaterialsInspector : public CResourceInspector<FBMaterial, MaterialGLSL>
{
protected:

	CTexturesInspector			*mTexturesInspector;

protected:

	virtual void DefaultGLSLResource( MaterialGLSL &item )
	{
		DefaultMaterial(item);
	}
	virtual void ConvertResource( FBMaterial *mbitem, FBEvaluateInfo *pEvalInfo, MaterialGLSL &item, bool subdata ) override
	{
		ConstructFromFBMaterial(mbitem, pEvalInfo, item);

		// update from connection ?!
	}

	virtual bool IsAnimated( FBMaterial *mbitem )
	{
		// TODO: 

		return false;
	}

public:

	//! a constructor
	CMaterialsInspector(CTexturesInspector *pTexturesInspector=nullptr)
		: mTexturesInspector(pTexturesInspector)
	{}

	//
	void SetTexturesInspector(CTexturesInspector *pTexturesInspector)
	{
		mTexturesInspector = pTexturesInspector;
	}

	void ConstructFromFBMaterial( FBMaterial *pMaterial, FBEvaluateInfo *pEvalInfo, MaterialGLSL &mat );


	virtual int	GetResourceType() const {
		return RESOURCE_MATERIAL;
	}

	void AttachTexture( FBMaterial *pmaterial, FBTexture *ptexture );
	void DetachTexture( FBMaterial *pmaterial, FBTexture *ptexture );

	virtual void UpdateAll(FBEvaluateInfo *pEvalInfo) override;
};



//////////////////////////////////////////////////
//

class CModelsInspector : public CResourceInspector<FBModel, ModelGLSL>
{
protected:

	size_t		mLastNumberOfShaders;

	CTexturesInspector			*mTexturesInspector;
	CMaterialsInspector			*mMaterialsInspector;
	CShadersInspector			*mShadersInspector;

	// mesh data

	// model to beginning of mesh, fast model->mesh access
	std::map<FBModel*, int>			mModelMeshesMap;
	std::vector<int>				mModelMeshesVector;

	// prepared converted GLSL struct vector to load into gpu
	std::vector<MeshGLSL>			mGLSLMesh;

	// render conveyer - sort meshes by callbacks (shader groups)
	struct SortModelWrap
	{
		int			startMeshId;
		FBModel		*pModel;
	};
	
	bool					mNeedToSortModels;
	CShadersFactory											mShadersFactory;
	std::vector< std::multimap<FBShader*, SortModelWrap> >	mSortedModels;
	std::vector< int >										mSortedCounts;

	void SortModels();

	virtual bool ProcessNeedFullUpdate() override
	{
		if ( mLastNumberOfShaders != mShadersInspector->GetNumberOfItems() )
		{
			mLastNumberOfShaders = mShadersInspector->GetNumberOfItems();
			return true;
		}

		return false;
	}

	virtual void OnProcessFinished(bool updated)
	{
		if (true == mNeedToSortModels)
		{
			SortModels();
			// TODO: future plan !
			//UpdateConveyer();
			mNeedToSortModels = false;
		}
	}

protected:

	std::auto_ptr<CRenderVertexConveyer>	mRenderConveyer;

	void UpdateConveyer();

public:

	const int GetNumberOfModelsInGroup(const int groupId) const
	{
		const int count = mSortedCounts.at(groupId);
		return count;
	}

	const std::multimap<FBShader*, SortModelWrap> &GetSortedModelsMap(const int groupId) const
	{
		return mSortedModels[groupId];
	}

	const CRenderVertexConveyer	*GetRenderConveyerPtr() const
	{
		return mRenderConveyer.get();
	}

	void UpdateDeformations(FBEvaluateInfo *pEvaluateInfo=nullptr);

	void BindVertexConveyer()
	{
		if (mRenderConveyer.get() )
			mRenderConveyer->Bind();
	}
	void UnBindVertexConveyer()
	{
		if (mRenderConveyer.get() )
			mRenderConveyer->UnBind();
	}

protected:

	void DefaultGLSLMesh( MeshGLSL &item )
	{
		item.color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		item.lightmap = 1;
		item.material = 0;
		item.model = 0;
		item.shader = 0;
	}

	void ConvertGLSLMesh( FBModelVertexData *pData, const int patchIndex, MeshGLSL &item )
	{
		// all data are connectors, so nothing to do in light convert
	}

	virtual void DefaultGLSLResource( ModelGLSL &item ) override
	{
		item.transform.identity();
		item.normalMatrix.identity();
	}
	virtual void ConvertResource( FBModel *pModel, FBEvaluateInfo *pEvalInfo, ModelGLSL &item, bool subdata ) override
	{
		// TODO: we do that in a Normals update function (using a faster SSE instructions)
		/*
		FBMatrix m;
		mbitem->GetMatrix(m, kModelTransformation_Geometry, true, pEvalInfo);
		
		for (int j=0; j<16; ++j)
		{
			item.transform.mat_array[j] = (float) m[j];
			//modeldata.normalMatrix.mat_array[j] = (float) m[j];
		}
		
		invert(item.normalMatrix, item.transform);
		transpose(item.normalMatrix);
		*/

		if (subdata)
		{
			int shaderIndex = FindShaderIndex(pModel);
			FBColor colorId = pModel->UniqueColorId;
			int receiveShadows = pModel->ReceiveShadows.AsInt();

			int meshStartIndex = mModelMeshesMap[pModel];
			MeshGLSL *pMeshData = &mGLSLMesh[meshStartIndex];
			FBModelVertexData *pdata = pModel->ModelVertexData;

			for (int patchIndex=0, patchCount=pdata->GetSubPatchCount(); 
				patchIndex < patchCount; 
				++patchIndex, ++pMeshData)
			{
			
				pMeshData->shader = shaderIndex;
				pMeshData->lightmap = receiveShadows;
				pMeshData->color = vec4( (float)colorId[0], (float)colorId[1], (float)colorId[2], 1.0);

				FBMaterial *pMaterial = pdata->GetSubPatchMaterial(patchIndex);
				if (mMaterialsInspector && pMaterial)
				{
					pMeshData->material = mMaterialsInspector->FindResourceIndex(pMaterial);
					if (pMeshData->material < 0)
						pMeshData->material = 0;
				}
			}
		}

	}

	const int FindShaderIndex(FBModel *pModel);

	void AreNodesAnimated( FBAnimationNode *pRootNode, bool &isAnimated )
	{
		if (pRootNode && pRootNode->KeyCount > 0)
		{
			isAnimated = true;
			return;
		}

		for (int i=0, count=pRootNode->Nodes.GetCount(); i<count; ++i)
			AreNodesAnimated( pRootNode->Nodes[i], isAnimated );
	}

	virtual bool IsAnimated( FBModel *pModel ) override
	{
		bool isAnimated = false;

		if (pModel->IsConstrained )
			return true;

		FBAnimationNode *pRootNode = pModel->AnimationNode;
		if (pRootNode)
		{
			AreNodesAnimated( pRootNode, isAnimated );
		}

		return isAnimated;
	}

	virtual bool FilterComponentHard( FBModel *pModel ) override
	{
		if (FBIS(pModel, FBScene) || FBIS(pModel, FBCamera) || FBIS(pModel, FBLight) || FBIS(pModel, FBModelNull) || FBIS(pModel, FBModelPath3D) || FBIS(pModel, FBModelSkeleton) )
			return false;
		
		FBGeometry *pGeom = pModel->Geometry;
		//FBModelVertexData *pData = pModel->ModelVertexData;
		if (nullptr == pGeom ) // || nullptr == pData)
			return false;

		return true;
	}

	/// this filtering could be added to the pending stack to be processed one more time during rendering preparation

	virtual bool FilterComponentLight( FBModel *pModel ) override
	{
		FBModelVertexData *pVertexData = pModel->ModelVertexData;
		 if (pVertexData == nullptr)
			 return false;
		 /*
		 if (pVertexData->GetSubRegionCount() == 0)
			 return false;
			 */
		 if (pVertexData->GetSubPatchCount() == 0)
			 return false;
		return true;
	}

protected:

	// used for calculate size for array allocation
	void CalculateModelCount(FBModel *parentModel, int &modelCount, int &meshCount);

	// go through scene models and fillup our arrays with models and meshes information
	void EnumerateModelAndMesh(FBModel *parentModel, int &modelIndex);

public:

	//! a constructor
	CModelsInspector(CTexturesInspector *pTexturesInspector=nullptr, CMaterialsInspector *pMaterialsInspector=nullptr,
				CShadersInspector *pShadersInspector=nullptr)
		: mTexturesInspector(pTexturesInspector)
		, mMaterialsInspector(pMaterialsInspector)
		, mShadersInspector(pShadersInspector)
	{
		mNeedToSortModels = false;
		mLastNumberOfShaders = 0;
	}

	//
	void SetTexturesInspector(CTexturesInspector *pTexturesInspector)
	{
		mTexturesInspector = pTexturesInspector;
	}
	void SetMaterialsInspector(CMaterialsInspector *pMaterialsInspector)
	{
		mMaterialsInspector = pMaterialsInspector;
	}
	void SetShadersInspector(CShadersInspector *pShadersInspector)
	{
		mShadersInspector = pShadersInspector;
	}

	size_t GetNumberOfMeshItems() const {
		return mGLSLMesh.size();
	}
	const MeshGLSL *GetMeshData() const {
		return mGLSLMesh.data();
	}

	int GetModelMeshIndex(const int modelIndex) {

		return mModelMeshesVector[modelIndex];
	}

	int GetModelMeshIndex(FBModel *pModel) {

		auto iter = mModelMeshesMap.find(pModel);

		if (iter != end(mModelMeshesMap)) {

			//return GetModelMeshIndex(iter->second);
			return iter->second;
		}
		
		return -1;
	}

	bool IsVisible(const int modelIndex) {

		if ( mUpdateVector[modelIndex] & RESOURCE_DELETED )
			return false;

		return mResourceVector[modelIndex]->IsVisible();
	}

	///////////////////////////////////////////////////////////
	// Light update

	// TODO:
	void AttachMaterial(FBModel *pModel, FBMaterial *pMaterial);
	void DetachMaterial(FBModel *pModel, FBMaterial *pMaterial);

	// TODO: direct textures to model is reflection maps, light maps, etc... not supported yet
	void AttachTexture(FBModel *pModel, FBTexture *pTexture);
	void DetachTexture(FBModel *pModel, FBTexture *pTexture);

	// DONE: at the moment we use only first shader, and skip all others (even FX)

	void AttachShader(FBModel *pModel, FBShader *pShader);
	void DetachShader(FBModel *pModel, FBShader *pShader);


	//////////////////////////////////////////////////////////
	// Hard update !
	virtual void UpdateAll(FBEvaluateInfo *pEvalInfo) override;

	void UpdateNormalMatrices( const double *modelview );

	// add to pending stack, try to add model during rendering prep step
	virtual bool Add( FBModel *pModel, bool addToPendingStack=true ) override;
	virtual void Delete( FBModel *mbitem ) override;
	virtual void NeedUpdate( FBModel *pModel, bool animated, bool subdata ) override;
};