
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectFinal.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_object.h"
#include "compositeMaster_computeShaders.h"
#include "compositeMaster_background.h"
#include "compositeMaster_common.h"

//--- Registration define

#define COMPOSITEFINAL__CLASSNAME			ObjectComposition
#define COMPOSITEFINAL__CLASSSTR			"ObjectComposition"
#define COMPOSITEFINAL__ASSETNAME			"Composition"


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// one final for one render tree
//

class ObjectComposition : public ObjectCompositeBase
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectComposition, ObjectCompositeBase)
	FBDeclareUserObject(ObjectComposition);

public:
	ObjectComposition(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	//! Open Reality deletion function.
	virtual void FBDelete() override;

	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

	void		EventIdle		( HISender pSender, HKEvent pEvent );

	virtual	bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

public:

	//virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	//virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	
	FBPropertyBool				OutputToRender;		//<! use this composite to output to the viewport
	// TODO: change on source image if no render node is used
	//FBPropertyBool				RenderAutoSize;		//<! use it to auto size final rendering image (based on camera resolution)

	// dimensions of an output image

	//FBPropertyBaseEnum<ECompositionSizeControl>	SizeControl;

	// we can use HUD to output result of our composition
	// but during the render, the quality will be in original size
	FBPropertyBaseEnum<ECompositionQuality>	Quality; // preview has 4 times less image size
	//FBPropertyBool				PreviewAsSubView;

	FBPropertyBool				ManualUpdate;
	FBPropertyAction			Update;
	/*
	FBPropertyInt				UserWidth;
	FBPropertyInt				UserHeight;
	*/
	FBPropertyListObject		Cameras;		// affected cameras (exclusive list)
	FBPropertyBool				UseSceneLights;	// should we use exclusive light list or all scene lights
	FBPropertyListObject			Lights;			// affected lights (that could be used only in projective mapping and shadowing)
	FBPropertyListObject		Groups;			// affected groups (display them only for that composition)

	FBPropertyListObject		Video;			// output composition dynamic video

	FBPropertyAction			SizeFromBackground;

	//
	FBPropertyBool				UseForBatchProcessing;

	// take textures as input and reconnect video as processed video memory components
	FBPropertyListObject		BatchInput;			//!> input images
	FBPropertyListObject		BatchOutput;		//!> output images

	//FBPropertyReference			BackgroundReference;

	FBPropertyInt					mCompositionStats[eCompositionStatsCount];

	FBPropertyInt				TypeInfoComposition;
	FBPropertyInt				TypeInfoLayer;
	FBPropertyInt				TypeInfoFilter;
	FBPropertyInt				TypeInfoMarker;

public:
	
	void CopyFrom(ObjectCompositeBase *pNode);

	void PrepVideoOutput();

	void PrepareTexturesForBatch(const int w, const int h);
	void FreeBatchTextures();

	virtual void ChangeContext() override;
	
	virtual void DoReloadShader() override;

	// render to buffers with render tree
	void Render();
	const CompositeFinalStats &GetStats() const
	{
		return mStats;
	}

	virtual const GLuint GetColorTextureId() const override
	{
		//return mInfo->GetRenderColorId();
		return mTextureBuffer.GetCurrentTextureId();
	}

	/*
	const GLuint	GetRenderBufferId() const;
	const GLuint	GetNormalBufferId() const;
	const GLuint	GetDepthBufferId() const;
	const GLuint	GetMaskBufferId() const;
	*/
	/*
	const bool IsInputSizeControl() const
	{
		return (SizeControl == eCompositionSizeInput || SizeControl == eCompositionSizeInputPOT);
	}
	*/
	void SetInputWidthValue(const int value);
	void SetInputHeightValue(const int value);

	// input from render viewport, from backgroung, etc.
	void SetInputWidthHeight(const int w, const int h);

	// set some data from render viewport
	void SetRenderData(const CompositeFinalRenderDataExchange &data)
	{
		mRenderData = data;

		// TODO: set composition size from render data if needed !
		/*
		if (SizeControl == eCompositionSizeViewport || (OutputToRender && SizeControl == eCompositionSizeInput) )
		{
			SetInputWidthHeight(data.w, data.h);
		}
		*/
	}
	const CompositeFinalRenderDataExchange &GetRenderData() const
	{
		return mRenderData;
	}

	const int GetWidthValue() const { return mProcessingWidth; }
	const int GetHeightValue() const { return mProcessingHeight; }

	const int GetBackgroundWidth() const;
	const int GetBackgroundHeight() const;

	const float GetPreviewSizeFactor() const { return mPreviewSizeFactor; }

	const GLuint	QueryANewTextureBuffer()
	{
		mTextureBuffer.QueryATextureBuffer();
	}

	CompositeBackground *GetCompositeBackgroundPtr() {
		return mBackground.get();
	}

	void NeedUpdate() { mNeedUpdate = true; }
	bool IsNeedUpdate() { return mNeedUpdate; }

	virtual void OnActiveChange(const bool value);

	void AskToUpdateLightList()
	{
		mNeedUpdateLightList = true;
	}

	// prepare exclusive light list for the active composition
	void EventBeforeRenderNotify();

	CGPUShaderLights *GetShaderLightsPtr() {
		return mExclusiveLights.get();
	}

protected:

	FBSystem			mSystem;

	ECompositionQuality					mLastQuality;
	//ECompositionSizeControl			mLastSizeControl;

	std::auto_ptr<CCompositionInfo>						mInfo;
	CompositeFinalStats									mStats;

	CompositeComputeShader::CComputeTextureBuffer<1>	mTextureBackground;
	CompositeComputeShader::CComputeTextureBuffer<2>	mTextureBuffer;
	std::auto_ptr<CompositeBackground>					mBackground;
	
	// TODO: implement exclusive light list for composition !
	bool											mNeedUpdateLightList;
	std::vector<FBLight*>							mLightsPtr;
	std::auto_ptr<CGPUShaderLights>					mExclusiveLights;

	CompositeFinalRenderDataExchange		mRenderData;
	
	/*
	GLuint				mRenderBufferId;			
	GLuint				mDepthBufferId;
	GLuint				mNormalBufferId;
	GLuint				mMaskBufferId;
	*/
	//
	// in case we do batch processing - allocate output textures

	GLuint				mBatchBufferId;	//> buffer id for the batch processing textures

	int					mNumberOfBatchTextures;
	int					mBatchTextureWidth;
	int					mBatchTextureHeight;

	//
	// in case it's original composition, allocate output texture

	//
	//GLuint				mCurrentTextureId;

	// buffers for texture processing
	//int					mCurrentTexture;
	//GLuint				mTextures[2];	// double buffer for compute shaders src and dst

	

	
	// size of camera window screen
	/*
	int			mRenderX;
	int			mRenderY;
	int			mRenderWidth;
	int			mRenderHeight;
	*/

	float		mPreviewSizeFactor;

	int			mBatchMaxWidth;
	int			mBatchMaxHeight;

	bool		mHasANewSize;
	int			mTryToSetWidth;
	int			mTryToSetHeight;

	bool		mNeedUpdate;

	
	//
	//void PrepRender();

	void ComputeBatchMaxSizes();

	void RenderVideoBatches();
	void RenderLayers(bool allowToReturnDirectTextureId);

	
	void CopyTexture(const GLuint srcLayerId, const GLuint dstLayerId);

	static void SetOutput(HIObject object, bool value);
	static void SetUpdate(HIObject object, bool value);
	static void SetQuality(HIObject object, ECompositionQuality value);
	static void SetSizeFromBackground(HIObject object, bool value);

	static void SetUserWidth(HIObject object, int value);
	static void SetUserHeight(HIObject object, int value);
	static int GetWidth(HIObject object);
	static int GetHeight(HIObject object);

	static int GetStatsWidth(HIObject object);
	static int GetStatsHeight(HIObject object);
	static int GetStatsNumberOfComputeShaders(HIObject object);
	static int GetStatsNumberOfDispatchGroups(HIObject object);
	static int GetStatsNumberOfTextures(HIObject object);
	static int GetStatsTexturesMemory(HIObject object);

	void InitBatchTextures();
	void ClearBatchTextures();

	void UpdateStatsProps();

	virtual const char *AssetNameString() override {
		return COMPOSITEFINAL__ASSETNAME;
	}
};