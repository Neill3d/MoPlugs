#ifndef __MB_RENDERER_H__
#define __MB_RENDERER_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MB_renderer.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <fbsdk/fbrenderer.h>

#include <vector>
#include <map>

#include "shared_content.h"
#include "graphics_framebuffer.h"
#include "graphics\OGL_Utils.h"

#include "compositeMaster_common.h"

#ifndef ORSDK_DLL
	/** \def ORSDK_DLL
	*	Be sure that ORSDK_DLL is defined only once...
	*/
	#define ORSDK_DLL K_DLLIMPORT
#endif


#define MORendererCallback__CLASSNAME		MoRendererCallback
#define MORendererCallback__CLASSSTR		"MoRendererCallback"
#define MORendererCallback__DESCSTR			"MoPlugs Renderer"

#define MAX_NUMBER_OF_RENDER_TILES			64

////////////////////////////////////////////

bool	IsCallbackAttached();
bool	IsLogarithmicDepth();
FBComponent *GetCurrentCompositionComponent();

//
// call it from shaders and renderer to prepare all needed rendering data only once per frame
//	some buffers needs to be updated when camera has changed during the frame rendering

kLongLong InitializeFrameDataAndBuffers( FBRenderOptions *pFBRenderOptions, FBCamera *pCamera, FBEvaluateInfo *pEvalInfo, const unsigned int uniqueFrameId,
	const bool forceToUpdate, const int maxCameraResolution, const int maxCubeMapResolution, 
	const bool processCompositions, FBComponent *pCompositionComponent, const int shadowResolution);

CRenderOptions &GetLastRenderOptions();

////////////////////////////////////////////

enum EGraphicsResolution
{
	eGraphicsResolutionCustom,
	eGraphicsResolutionHD,
	eGraphicsResolution2K_Aspect2_35,
	eGraphicsResolution4KHD_Aspect2_35,
	eGraphicsResolution4K_Aspect2_35
};

enum EGraphicsQualityPresets
{
	eGraphicsQualitySimple,
	eGraphicsQualityCustom,
	eGraphicsQualityMSAA_2x,
	eGraphicsQualityMSAA_4x,
	eGraphicsQualityMSAA_16x,
	eGraphicsQualityCSAA_4x,
	eGraphicsQualityCSAA_16x,
	eGraphicsQualitySupersampling_2x2,
	eGraphicsQualitySupersampling_4x4,
	eGraphicsQualitySupersampling_6x6,
	eGraphicsQualitySupersampling_8x8
};

enum EGraphicsDepth
{
	eGraphicsDepthLinear,
	eGraphicsDepthLog,
	eGraphicsDepthNV	// negative range
};

enum ERendererUpdate
{
	eRendererUpdateManualy,
	eRendererUpdatePartly,
	eRendererUpdateRealTime
};

enum ERendererEnum
{
	eRendererDefault
};

/////////////////////////////////////////////////////////////////////////////
// MoPlugs Renderer

class ORSDK_DLL MoRendererCallback : public FBRendererCallback
{
    FBRendererCallbackDeclare(MoRendererCallback, FBRendererCallback);

public:
	virtual bool FBCreate() override;		//!< Constructor.
	virtual void FBDestroy() override;		//!< Destructor.

	void	FBCreateSettingsProperties();

    virtual const char* GetCallbackName() const override;
	virtual const char* GetCallbackDesc() const override;

    virtual unsigned int GetCallbackPrefCount() const override;
    virtual const char*  GetCallbackPrefName(unsigned int pIndex) const override;

    virtual void Attach() override;
    virtual void Detach() override;
    virtual void DetachDisplayContext(FBViewingOptions* pViewOption) override;

    virtual void Render(FBRenderOptions* pRenderOptions) override;

	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertyView(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false);
	static void AddPropertiesToPropertyViewManager(FBComponent *pComponent);

	virtual	bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;

public:

	void EventFileNew(HISender pSender, HKEvent pEvent);
	void EventFileOpen(HISender pSender, HKEvent pEvent);
	void EventFileMerge(HISender pSender, HKEvent pEvent);
	void EventFileOpenComplete(HISender pSender, HKEvent pEvent);

	/** Demo how to utilize the per frame callback at pipeline's critical stage/timing in the application main loop.
    */
    
    void OnPerFrameEvaluationPipelineCallback   (HISender pSender, HKEvent pEvent);
	void OnPerFrameSynchronizationCallback      (HISender pSender, HKEvent pEvent);
	void OnPerFrameRenderingPipelineCallback    (HISender pSender, HKEvent pEvent);
    

	void EventSceneChange			            (HISender pSender, HKEvent pEvent);
    void EventConnNotify                        (HISender pSender, HKEvent pEvent);
    void EventConnStateNotify                   (HISender pSender, HKEvent pEvent);
    void EventConnDataNotify                    (HISender pSender, HKEvent pEvent);


	void OnUIIdle	(HISender pSender, HKEvent pEvent);
	void OnGPUMenu	(HISender pSender, HKEvent pEvent);
	
	void OnVideoFrameRendering	(HISender pSender, HKEvent pEvent);


public:

	//
	// list of properties that could be overrided during the render process

	struct CBaseProperties
	{
		
		virtual const bool ConfigSerialization() const {
			return true;
		}

		virtual const int GetNumberOfGroups() const {
			return 0;
		}
		virtual const char *GroupName(const int groupId) const = 0;

		virtual const int GetNumberOfItems(const int groupId) const {
			return 0;
		}
		virtual FBProperty **GetPropertiesPtr(const int groupId) {
			return nullptr;
		}

		virtual FBStringList *GetPropertiesNamesPtr(const int groupId) {
			return nullptr;
		}
	};

	struct CProperties : public CBaseProperties
	{
#define NUMBER_OF_QUALITY_PROPS		20
#define NUMBER_OF_STAGES_PROPS		7
#define NUMBER_OF_SHADING_PROPS		4
		union CQuality
		{
			FBProperty			*props[NUMBER_OF_QUALITY_PROPS];

			//
			// quality properties
			struct {
			FBPropertyBaseEnum<EGraphicsDepth>			*DisplayDepth;
			FBPropertyBaseEnum<EGraphicsQualityPresets>	*QualityPreset;

			FBPropertyInt								*DepthSamples;
			FBPropertyInt								*CoverageSamples;
			FBPropertyDouble							*SupersampleScale;
			FBPropertyBaseEnum<EResamplingFilter>		*DownsampleType;

			// secondary framebuffer could have a separate quality options
			FBPropertyBool								*EnableSecondary;

			FBPropertyBaseEnum<EGraphicsQualityPresets>	*QualityPreset2;
			FBPropertyInt								*DepthSamples2;
			FBPropertyInt								*CoverageSamples2;
			FBPropertyDouble							*SupersampleScale2;
			FBPropertyBaseEnum<EResamplingFilter>		*DownsampleType2;

			// tiles for supersampling rendering
			FBPropertyBool			*UseSuperSamplingTiles;
			FBPropertyInt			*TilesHor;
			FBPropertyInt			*TilesVer;

			//
			FBPropertyInt					*MaxCubeMapSize;
			FBPropertyInt					*MaxCameraSize;
			FBPropertyInt					*MaxShadowSize;		// default 512 (better power of two)
			FBPropertyBool					*EnableShadowMultisampling;
			FBPropertyInt					*ShadowSamples;
			};

		} mQuality;
		FBStringList		mQualityNames;

		union CStages
		{
			FBProperty			*props[NUMBER_OF_STAGES_PROPS];

			struct {
				FBPropertyBool					*ProcessDynamicMasks;
				FBPropertyBool					*ProcessCompositions;

				FBPropertyBool					*ProcessCubeMapRendering;
				FBPropertyBool					*ProcessCameraRendering;
			
				FBPropertyBool					*ProcessShadows;
				FBPropertyBool					*ProcessPlanarReflection;

				// two additional color outputs (for compositing)
				FBPropertyBool					*RenderNormalAndMask;
			};
		} mStages;
		FBStringList		mStagesNames;

		//
		// shading properties
		union CShading
		{
			FBProperty			*props[NUMBER_OF_SHADING_PROPS];

			struct {
				FBPropertyBool		*TextureMapping;
				FBPropertyBool		*SceneLighting;	// use scene lights or simple 2 infinite lights
				FBPropertyBool						*OverrideShading;
				FBPropertyBaseEnum<EShadingType>	*ShadingType;
			};
		} mShading;
		FBStringList	mShadingNames;

		//! a constructor
		CProperties()
			: CBaseProperties()
		{}
		virtual ~CProperties()
		{}

		void Create(FBComponent *pParent, const char *prefix="");

		virtual const bool ConfigSerialization() const override {
			return true;
		}

		virtual const int GetNumberOfGroups() const override {
			return 3;
		}
		virtual const int GetNumberOfItems(const int groupId) const override {
			switch (groupId)
			{
			case 0:
				return NUMBER_OF_QUALITY_PROPS;
			case 1:
				return NUMBER_OF_STAGES_PROPS;
			case 2:
				return NUMBER_OF_SHADING_PROPS;
			}
			return 0;
		}
		virtual const char *GroupName(const int groupId) const override
		{
			switch (groupId)
			{
			case 0:
				return "Quality Settings";
			case 1:
				return "Rendering Stages";
			case 2:
				return "Shading Settings";
			}
			return 0;
		}
		virtual FBProperty **GetPropertiesPtr(const int groupId) override {
			switch (groupId)
			{
			case 0:
				return mQuality.props;
			case 1:
				return mStages.props;
			case 2:
				return mShading.props;
			}
			return nullptr;
		}

		virtual FBStringList *GetPropertiesNamesPtr(const int groupId) {
			switch (groupId)
			{
			case 0:
				return &mQualityNames;
			case 1:
				return &mStagesNames;
			case 2:
				return &mShadingNames;
			}
			return nullptr;
		}

		//
		//
		const bool ProcessCompositions() const {
			return (mStages.ProcessCompositions->AsInt() > 0);
		}

		const int MaxCameraResolution() const {
			return mQuality.MaxCameraSize->AsInt();
		}
		const int MaxCubeMapResolution() const {
			return mQuality.MaxCubeMapSize->AsInt();
		}
		const int MaxShadowResolution() const {
			return mQuality.MaxShadowSize->AsInt();
		}
		const bool IsShadowMultisamplingEnabled() const {
			return (mQuality.EnableShadowMultisampling->AsInt() > 0);
		}
		const int ShadowSamples() const {
			return mQuality.ShadowSamples->AsInt();
		}
		const bool IsLogarithmicDepth() const {
			return (mQuality.DisplayDepth->AsInt() == eGraphicsDepthLog);
		}
	};

	// composition work
	struct CCompositionOptions : public CBaseProperties
	{
#define NUMBER_OF_COMP_OPTIONS		10
		union
		{
			FBProperty		*props[NUMBER_OF_COMP_OPTIONS];

			struct {
				FBPropertyBool				*RenderOnlyCurrent;		// !< render first active composition

				FBPropertyBool				*ShowAsPreview;		// override properties to show compositions as preview
				FBPropertyBool				*PreviewAsSubView;
				FBPropertyBool				*PreviewShowTitle;
				FBPropertyDouble			*PreviewSize;		// percentage 0-100

				FBPropertyBool				*PreviewOnPlay;
				FBPropertyBool				*PreviewOnHoldingAlt; // preview during camera manipulations

				FBPropertyBool				*DisablePreviewOnRender;

				FBPropertyInt				*ComputeGroupSize;	// number of groups in composition compute shaders
				FBPropertyDouble			*Timing;
			};
		} mData;

		FBStringList		mNames;

		//! a constructor
		CCompositionOptions()
			: CBaseProperties()
		{}
		virtual ~CCompositionOptions()
		{}

		void Create(FBComponent *pParent);

		virtual const bool ConfigSerialization() const override {
			return true;
		}

		virtual const int GetNumberOfGroups() const override {
			return 1;
		}
		virtual const char *GroupName(const int groupId) const override
		{
			return "Composition Options";
		}
		virtual const int GetNumberOfItems(const int groupId) const override {
			return NUMBER_OF_COMP_OPTIONS;
		}
		virtual FBProperty **GetPropertiesPtr(const int groupId) override {
			return mData.props;
		}

		virtual FBStringList *GetPropertiesNamesPtr(const int groupId) override {
			return &mNames;
		}

		//
		const bool ShowAsPreview() const {
			return (mData.ShowAsPreview->AsInt() > 0);
		}
		const bool PreviewOnPlay() const {
			return (mData.PreviewOnPlay->AsInt() > 0);
		}
		const bool PreviewOnHoldingAlt() const {
			return (mData.PreviewOnHoldingAlt->AsInt() > 0);
		}
		const bool RenderOnlyCurrent() const {
			return (mData.RenderOnlyCurrent->AsInt() > 0);
		}
	} ;

	
	//
	/*
	struct ShadowOptions
	{
		FBPropertyDouble				OffsetFactor;
		FBPropertyDouble				OffsetUnits;
		FBPropertyDouble				Bias;
		FBPropertyBool					UseGeomCache;
		FBPropertyBool					CascadedOnly;	//<! show only one shadow from one infinite light
		FBPropertyListObject			CascadedLight;
		FBPropertyInt					CascadedSplits;
		FBPropertyDouble				CascadedSplitCorrection;
		FBPropertyBool					CascadedAutoPlanes;
		FBPropertyDouble				CascadedNearPlane;
		FBPropertyDouble				CascadedFarPlane;
	};

	ShadowOptions					mShadowOptions;
	*/
	//
	//	STATS
	// evaluation timings

	struct CRenderStats : public CBaseProperties
	{
#define NUMBER_OF_STATS_PROPERTIES		14

		union
		{
			FBProperty			*props[NUMBER_OF_STATS_PROPERTIES];

			struct {
				FBPropertyDouble				*TimingDynamicMasks;
	
				FBPropertyDouble				*TimingExtendedRendering;
				FBPropertyDouble				*TimingShadows;

				FBPropertyDouble				*TimingScenePreparation;

				FBPropertyInt					*TotalOpaqueShaders;
				FBPropertyInt					*TotalTransparencyShaders;
				FBPropertyInt					*TotalOpaqueModels;
				FBPropertyInt					*TotalTransparentModels;

				// composition
				FBPropertyInt					*w;
				FBPropertyInt					*h;

				FBPropertyInt					*numberOfComputeShaders;
				FBPropertyInt					*numberOfDispatchGroups;
				FBPropertyInt					*numberOfTextures;
				FBPropertyInt					*texturesMemory;
			};
		} mData;

		FBStringList	mNames;

		//! a constructor
		CRenderStats()
			: CBaseProperties()
		{}
		virtual ~CRenderStats()
		{}

		void Create(FBComponent *pParent);

		virtual const bool ConfigSerialization() const override {
			return false;
		}

		virtual const int GetNumberOfGroups() const override {
			return 1;
		}
		virtual const char *GroupName(const int groupId) const override
		{
			return "Render Statistics";
		}
		virtual const int GetNumberOfItems(const int groupId) const override {
			return NUMBER_OF_STATS_PROPERTIES;
		}
		virtual FBProperty **GetPropertiesPtr(const int groupId) override {
			return mData.props;
		}

		virtual FBStringList *GetPropertiesNamesPtr(const int groupId) override {
			return &mNames;
		}
	} ;

	//
	// GLOBAL RESOLUTION

	struct CGlobalResolution : public CBaseProperties
	{

#define NUMBER_OF_GR_ITEMS		5

		union {
			FBProperty		*props[NUMBER_OF_GR_ITEMS];

			struct {
				FBPropertyBool		*UseGlobalResolution;
				FBPropertyBool		*ExcludeProjectorCameras;	//!< cameras that are used for projTex shader
				//FBPropertyBaseEnum<EGraphicsResolution>		RenderResolution; //<! override all cameras render resolution
	
				FBPropertyBaseEnum<ERendererEnum> *RenderResolution;
	
				FBPropertyInt		*CustomWidth;
				FBPropertyInt		*CustomHeight;
			};
		} mData;

		FBStringList		mNames;

		//! a constructor
		CGlobalResolution()
			: CBaseProperties()
		{}
		virtual ~CGlobalResolution()
		{}

		void Create(FBComponent *pParent);
		
		virtual const bool ConfigSerialization() const override {
			return true;
		}

		virtual const int GetNumberOfGroups() const override {
			return 1;
		}
		virtual const char *GroupName(const int groupId) const override
		{
			return "Global Resolution";
		}
		virtual const int GetNumberOfItems(const int groupId) const override {
			return NUMBER_OF_GR_ITEMS;
		}
		virtual FBProperty **GetPropertiesPtr(const int groupId) override {
			return mData.props;
		}

		virtual FBStringList *GetPropertiesNamesPtr(const int groupId) override {
			return &mNames;
		}
	};
	
	//
	// property under the groups

	CProperties				mDisplayProperties;
	CProperties				mRenderProperties;
	CProperties				*mCurrentProperties;

	CCompositionOptions		mCompositionOptions;
	CRenderStats			mRenderStats;
	CGlobalResolution		mGlobalResolution;

	// control the custom shading
	FBPropertyAction				Reset;

	// TODO: try to implement by FBModel property draw flag 
	FBPropertyBool      NoFrustumculling;       
	FBPropertyBool		EarlyZTest;				//!< should be use an earlyZ test for rendering
	
	FBPropertyBool		SupportLegacyShaders;	// render heavy shaders inside the projective mapping and geomCache mobu shaders


	//FBPropertyBool	TransparencyPassByMaterial;		//!< when material has transparency value, let's render connected models in second pass
	//FBPropertyBool	RenderEmissiveToMask;			//!< render emissive fragment into Composition Mask A

	FBPropertyBool							RenderQualityOverride;
	FBPropertyBool							RenderShadingOverride;


	FBPropertyBaseEnum<ERendererUpdate>		UpdateType;
	FBPropertyAction						Update;		//!< do a render only when press propriate button

	FBPropertyDouble						TimeToPartlyUpdate;

	// connections
	FBPropertyListObject						CurrentComposition;		//<! access to current final composition component

	// panorama and VR customization
	FBPropertyInt			CubeMapSize;
	FBPropertyInt			PanoWidth;
	FBPropertyInt			PanoHeight;

	FBPropertyAction		TestAction;

	FBPropertyBool			TestBool;

	static void SetRenderSamples(HIObject object, int value);
	static void SetDepthSamples(HIObject object, int value);
	static void SetCoverageSamples(HIObject object, int value);

	static void SetRenderDepthSamples(HIObject object, int value);
	static void SetRenderCoverageSamples(HIObject object, int value);

	static void SetReset(HIObject object, bool value);

	static void OnTestActionSet(HIObject object, bool value);

	static void SetResolution(HIObject object, ERendererEnum value);
	static void SetQualityPreset(HIObject object, EGraphicsQualityPresets	value);
	static void SetRenderQualityPreset(HIObject object, EGraphicsQualityPresets	value);

	static void SetRenderUpdateAction(HIObject object, bool value);

	void CubeMapSizeSet(int value);

	const bool IsLogarithmicDepth() const;

	void	AskToRenderAFrame() {
		mNeedToRender = true;
	}
	
protected:

	struct PresetItem
	{
		char			name[128];
		int				width;
		int				height;
	};

	int							mDefaultPreset;	// index that should be used as a startup preset
	std::vector<PresetItem>		mPresets;

	void ApplyResolutionPreset(const int index);
	void LoadResolutionPresets();

	void ApplyQualityPreset(CProperties::CQuality &quality, const EGraphicsQualityPresets value, bool refresh);
	// for secondary framebuffer
	void ApplyQualityPreset2(CProperties::CQuality &quality, const EGraphicsQualityPresets value, bool refresh);
	//void ApplyRenderQualityPreset(EGraphicsQualityPresets value);

	void	ResetToDefault();
	void	RunTestAction();

	bool getOrSetBool (FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, bool defValue);
	int getOrSetInt (FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, int defValue);
	double getOrSetDouble (FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, double defValue);
	void getOrSetString (FBString &value, FBConfigFile &lConfigFile, const char *section, const char *name, const char *comment);

	void LoadPropertiesFromConfig(CBaseProperties *props, FBConfigFile &lConfig);

protected:

	HGLRC						mLastContext;

	// TODO:	fill flag with propriate values
	bool				mIsRendering;
	bool				mVideoRendering;
	bool				mCameraInternalDOF;

	bool				mNeedToRender;
	int					mPartlyRenderIndex;		// let's rerender several frames cause some models could be not ready

	int				mLastRenderingFrameId;
	int				mLastDisplayUpdateId;
	int				mLastRenderUpdateId;

	FBTime			mLastLocalTime;
	FBTime			mLastSystemTime;

	bool						mWait;

	bool						mGrabImage;
	FBString					mGrabImageName;

	bool						mGrabWithFrameNumber;

	// effect only on first pane !!
	CompositeFinalRenderDataExchange	mCompositionDataExchange;
	//std::auto_ptr<FrameBuffer>			mResolvedFrameBuffer;	// use it when we need to blit sampled textures into simple 2d textures
	
	// store vector of all composition in the scene
	std::vector<FBComponent*>			mCompositionsVector;
	//std::vector<CGPUShaderLights*>		mCompositionLights;		// allocate exclusive light buffer if needed
	
	FBComponent							*mCompositionOutput;		//!< current viewport composition
	// store vector of all dynamic masks in the scene
	std::vector<FBComponent*>			mDynamicMasksVector;

	// event on main layer resize
	void	Compositions_ChangeGlobalSettings();
	void	Compositions_ChangeContext();
	void	Compositions_PrepDataExchange(CompositeFinalRenderDataExchange &data,
		const CRenderOptions &renderOptions, const MainFrameBuffer &frameBuffer);
	bool	Compositions_FindCurrent(FBCamera *pCamera);
	void	Compositions_EventBeforeRenderNotify();
	void	Compositions_Render();
	void	Compositions_RenderTextures();

	void	DynamicMasks_ChangeContext();
	void	DynamicMasks_Render();

protected:
    
	CGPUFBScene							*mGPUFBScene;

	FrameBufferInfo						mFrameBufferInfo;
	
	
	// TODO: wrap into render layer class
	
	struct CLayerSingleColor
	{
		MainFrameBuffer		mFrameBuffer;

		CLayerSingleColor()
			: mFrameBuffer(false)
		{}
	};
	struct CLayerDetailedColor
	{
		MainFrameBuffer		mFrameBuffer;

		CLayerDetailedColor()
			: mFrameBuffer(true)
		{}
	};

	int			mNumberOfTiles;
	int			mTilesSW;
	int			mTilesSH;

	//MainFrameBuffer						mMainFrameBuffer;

	CLayerSingleColor		mBackgroungLayer;
	CLayerDetailedColor		mMainLayer;	// output color, normal and mask
	CLayerDetailedColor		mSecondaryLayer;

	CLayerSingleColor		mTileLayer;	// output to color only
	//CRenderLayer		mTransparencyLayer;

	FBApplication		mApplication;
	FBSystem			mSystem;
	FBMenuManager		mMenuManager;
	FBGenericMenu		*mGPUMenu;

	bool				mSupported;
	
	FBTexture			*mTestTexture;
	unsigned int        mAttachCount;          //!< How many view panes use this renderer callback instance currently.

	bool		mUpdatingCameraProperties;
	FBCamera	*mUpdatingCamera;

	void UpdateCameraResolution(FBCamera *updateSpeficiedCamera = nullptr);
	bool IsCameraReadyToSetResolution(FBCamera *pCamera, bool excludeFacialCameras);
	void UpdateConnectedDecalFilters();

	void ComputeIfNeededPartlyUpdate();
	void PrepRenderOptions(CRenderOptions &options, FBRenderOptions* pFBRenderOptions);
	// check for background and transparency layers
	void CheckAdditionalLayersNeeded(CRenderOptions &options);

	//void RenderToBackground(CRenderOptions &options);
	
	void RenderShadingToFramebuffer(const ERenderLayer layerId, MainFrameBuffer &framebuffer, bool moreColorAttachments, 
		const int width, const int height, const int samples, const int coverageSamples,
		const double sscale, const bool plotMultisampling, CRenderOptions &options, 
		FBRenderOptions *pFBRenderOptions,
		const bool prepCompositions, RenderingStats &stats);
	
	void RenderTilesToFramebuffer(const ERenderLayer layerId, MainFrameBuffer &mainbuffer, MainFrameBuffer &tilebuffer, 
		const int width, const int height, const int samples, const int coverageSamples,
		const double sscale, const bool plotMultisampling, CRenderOptions &options, 
		FBRenderOptions *pFBRenderOptions,
		const bool prepCompositions, RenderingStats &stats);

	//void RenderToTransparency(CRenderOptions &options);

	// menu actions

	void	CreateMenu();

	void	ExportGPUCache();
	void	ImportGPUCache();
	void	SelectStaticObjects();

	void	SelectGPUShader();
	void	SelectGPUObject();
	void	UpdateGPUShader();
	void	LaunchGPUCacheScene();
	void	CheckFreeGPUMemory();
	
	void	SelectAllGPUCacheObjects();
	void	SelectFirstGPUCacheObject();
	void	SelectNextGPUCacheObject();

	void SelectAllGPUCacheShaders();
	void	SelectFirstGPUCacheShader();
	void	SelectNextGPUCacheShader();

	void	ConvertAllBoxIntoGPUCacheObject();
};


#endif /* __MOPLUGS_RENDERER_H__ */
