
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectLayers.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_object.h"
#include "compositeMaster_background.h"
#include "render_layer_info.h"

#define COMPOSITERENDER__CLASSNAME			ObjectCompositionRender
#define COMPOSITERENDER__CLASSSTR			"ObjectCompositionRender"
#define COMPOSITERENDER__ASSETNAME			"Render Layer"

#define COMPOSITESOLIDCOLOR__CLASSNAME		ObjectCompositionColor
#define COMPOSITESOLIDCOLOR__CLASSSTR		"ObjectCompositionColor"
#define COMPOSITESOLIDCOLOR__ASSETNAME		"Color Layer"

#define COMPOSITESHADOW__CLASSNAME			ObjectCompositionShadow
#define COMPOSITESHADOW__CLASSSTR			"ObjectCompositionShadow"
#define COMPOSITESHADOW__ASSETNAME			"Shadow Layer"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//

enum ECompositeRenderOutput
{
	eCompositeRenderColor,
	eCompositeRenderDepth,
	eCompositeRenderNormal,
	eCompositeRenderMaskA,
	eCompositeRenderMaskB,
	eCompositeRenderMaskC,
	eCompositeRenderMaskD,
	eCompositeRenderCustomModel
};



class ObjectCompositionRender : public ObjectCompositeLayer
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectCompositionRender, ObjectCompositeLayer)
	FBDeclareUserObject(ObjectCompositionRender);

public:
	ObjectCompositionRender(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	//virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	//virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	//virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	
	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

public:
	
	virtual void DoTransformFitImageSize() override;

	FBPropertyBaseEnum<ERenderLayer>				RenderLayer;	//!< choose a render layer
	FBPropertyBaseEnum<ECompositeRenderOutput>		WhatOutput;		//!< type of blending two images

	// render a custom model into a render layer (for masking)
	FBPropertyListObject				CustomModel;

	FBPropertyAnimatableColor			BackgroundColor;
	FBPropertyAnimatableDouble			BackgroundAlpha;

	FBPropertyBool						UseCustomColor;
	FBPropertyAnimatableColor			CustomColor;

	FBPropertyBool						UseCustomTexture;
	FBPropertyListObject				CustomTexture;

	FBPropertyBool						CompareWithMainDepth;
	FBPropertyDouble					Bias;	// compare main depth

	// scale down buffer if needed to compute faster
	FBPropertyDouble					OutputBufferScale;

	void CopyFrom(ObjectCompositeBase *pNode);

	
	// setup buffers and data if not setup already
	//virtual void PreRender(FBCamera *pCamera, const int width, const int height) override;
	// render to buffers with render tree
	//virtual void Render() override;

	//
	virtual const GLuint	ComputeLayerTexture(const CCompositionInfo *pInfo, CompositeFinalStats &stats) override;
	
	virtual void DoReloadShader() override
	{
		mShaderDrawWithDepth.reset(nullptr);
		ParentClass::DoReloadShader();
	}

	virtual void ChangeContext() override
	{
		mFramebuffer.reset(nullptr);
		mShaderDrawWithDepth.reset(nullptr);
		ParentClass::ChangeContext();
	}
	/*
	virtual void ChangeGlobalSettings() override
	{
		mFramebuffer.reset(nullptr);
		mShaderDrawWithDepth.reset(nullptr);
		ParentClass::ChangeGlobalSettings();
	}
	*/
protected:

	virtual const char *AssetNameString() override {
		return COMPOSITERENDER__ASSETNAME;
	}

protected:

	// framebuffer for render a custom model
	std::auto_ptr<FrameBuffer>		mFramebuffer;
	std::auto_ptr<GLSLShader>		mShaderDrawWithDepth;

	struct {

		GLint		screenSize;
		GLint		options;
		GLint		materialColor;
		GLint		useColorSampler;
		GLint		colorSampler;
		GLint		depthSampler;

	} mUniformLoc;

	bool		PrepResources(const int width, const int height);
	bool			RenderCustomModels(int &width, int &height, GLuint depthId, FBCamera *pCamera);

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class ObjectCompositionColor : public ObjectCompositeLayer
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectCompositionColor, ObjectCompositeLayer)
	FBDeclareUserObject(ObjectCompositionColor);

public:
	ObjectCompositionColor(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	//virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	//virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	//virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	
	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }

	static void AddPropertiesToPropertyViewManager();

public:

	FBPropertyInt				UserWidth;
	FBPropertyInt				UserHeight;

public:
	
	virtual void DoTransformFitImageSize() override;

	void CopyFrom(ObjectCompositeBase *pNode);
	
	//
	virtual const GLuint	ComputeLayerTexture(const CCompositionInfo *pInfo, CompositeFinalStats &stats) override;

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

	virtual void DoReloadShader() override
	{
		mBackground->NeedReload();
		ParentClass::DoReloadShader();
	}

	virtual void ChangeContext() override
	{
		mBackground->ChangeContext();
		ParentClass::ChangeContext();
	}

protected:
	
	CompositeComputeShader::CComputeTextureBuffer<1>		mTextureBuffer;
	std::auto_ptr<CompositeBackground>						mBackground;

	virtual const char *AssetNameString() override {
		return COMPOSITESOLIDCOLOR__ASSETNAME;
	}
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class ObjectCompositionShadow : public ObjectCompositeLayer
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectCompositionShadow, ObjectCompositeLayer)
	FBDeclareUserObject(ObjectCompositionShadow);

public:
	ObjectCompositionShadow(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	//virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	//virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	//virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	
	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }

	static void AddPropertyViewForShadow(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false);
	static void AddPropertiesToPropertyViewManager();

public:

	FBPropertyInt				UserWidth;
	FBPropertyInt				UserHeight;

	FBPropertyAnimatableDouble		Density;	//!< shadow density
	FBPropertyAnimatableDouble		Feather;

	FBPropertyDouble				Bias;		// shadow projection bias

	FBPropertyAction				CreateZone;	// create a new shadow volume

	FBPropertyBool					UseModelProperties;	// color, composite mode ?!
	FBPropertyListObject			ZoneObjects;		// could be multiple objects with multipass effect applying
	
	FBPropertyBool					SortZones;
	FBPropertyAction				UpdateAllZones;

	static void SetCreateZone(HIObject object, bool value);
	static void SetUpdateAllZonesAction(HIObject object, bool value);

public:
	
	void CopyFrom(ObjectCompositeBase *pNode);
	
	virtual bool ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo) override;

	//
	virtual const GLuint	ComputeLayerTexture(const CCompositionInfo *pInfo, CompositeFinalStats &stats) override;

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

	virtual void ChangeContext() override
	{
		mProgramShadow = nullptr;
		ParentClass::ChangeContext();
	}

	virtual void ChangeGlobalSettings() override
	{
		mProgramShadow = nullptr;
		ParentClass::ChangeGlobalSettings();
	}
	
	virtual bool HasMultiSamplingSupport() {
		return true;
	}

protected:
	
	CompositeComputeShader::CMixedProgram		*mProgramShadow;
	std::auto_ptr<CProgramUniformsBase>			mProgramShadowUniforms;

	const char *MixedProgramLabel() {
		return "3d Shadow";
	}
	const char *MixedProgramPath() {
		return "\\GLSL_CS\\layerShadow.cs";
	}

	CompositeComputeShader::CComputeTextureBuffer<1>		mTextureShadow;

	struct ShadowData
	{
		float		density;
		float		feather;
		float		fnear;
		float		ffar;
	
		vec4		zoneInfo;	// x - number of volume models connected
	};

	struct ZoneData
	{
		float		density;
		float		feather;
		float		bias;
		float		Id;
	
		vec4		volumeMin;
		vec4		volumeMax;
		mat4		lightMatrix;
		mat4		invMatrix;
	};

	// gpu memory buffer
	ShadowData						mShadowData;
	std::vector<ZoneData>			mZoneData;

	CGPUBufferSSBO		mGPUShadowBuffer;
	CGPUBufferSSBO		mGPUZoneBuffer;

	// intermid buffers for processing many volumes inside one filter
	CompositeComputeShader::CComputeTextureBuffer<2>	mVolumeTextureBuffer;

	void		PrepShadowData(const CCompositionInfo *pInfo);
	void		PrepZoneData(ZoneData &data, FBModel *pZoneModel, const bool useModelProperties);
	void		PrepAllZones(FBCamera *pCamera);

protected:

	FBSystem					mSystem;
	
	CGPUFBScene					*pGPUFBScene;

	std::vector<std::pair<FBModel*, double>>		mZoneSortVector;
	
	// list of all combination of blend modes for volume models
	std::vector<CompositeComputeShader::CMixedProgram*>		mZoneProgram;

	const int ComputeNumberOfZoneModels();
	void	SortZoneModels(FBCamera *pCamera, const bool sorting);

	bool PrepComputeProgram(const CCompositionInfo *pInfo);
	bool RenderShadowZones(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, GLuint dstTexId);

	void SetUpAlphaUniform();
	void SetUpObjectUniforms(FBCamera *pCamera, const bool useZoneObject);
	void SetUpZoneUniforms(FBCamera *pCamera, FBModel *pModel);

	void DoCreateZone();
	void DoUpdateAllZones();

	virtual const char *AssetNameString() override {
		return COMPOSITESOLIDCOLOR__ASSETNAME;
	}
};