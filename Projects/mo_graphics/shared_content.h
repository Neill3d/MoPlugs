
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_content.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//-- 
#include <GL\glew.h>

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "ShaderFX.h"

// STL
#include <vector>
#include <map>

#include "Types.h"

#include "IO\tinyxml.h"

#include "shared_models.h"
#include "shared_textures.h"
#include "shared_materials.h"
#include "shared_lights.h"
#include "shared_shaders.h"
#include "shared_projectors.h"
#include "shared_rendering.h"
#include "shared_camera.h"

#include "graphics\Framebuffer.h"
#include "graphics\glslShader.h"

#include "gpucache_model.h"
#include "shared_models_newton.h"

#include "ContentInspector.h"

#include "..\Common_Projectors\bakeProjectors_projectors.h"

///////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////////////////////////
// CGPUFBScene - get handles and cache data from the FBScene

// UberShader - rendering geometry from that shader
//  track current rendering camera
//  framebuffer for shadows
//  framebuffer for background
// collect information for all modules from FBScene

class CGPUFBScene : public Singleton<CGPUFBScene>
{
public:

	//! a constructor
	CGPUFBScene();

	//~ a destructor
	virtual ~CGPUFBScene();

	// inform us about context changes
	void	AskToUpdateSceneData();
	void		ChangeContext(FBRenderOptions *pFBRenderOptions);
	void		ChangeContextForShaderGroups(FBRenderOptions *pFBRenderOptions);
	void		ReloadUberShader();
	void		ReloadCompositeShader();

	// mobu resource collector
	void EventSceneChange			            (HISender pSender, HKEvent pEvent);
    void EventConnNotify                        (HISender pSender, HKEvent pEvent);
    void EventConnStateNotify                   (HISender pSender, HKEvent pEvent);
    void EventConnDataNotify                    (HISender pSender, HKEvent pEvent);

	void OnPerFrameEvaluationPipelineCallback   (HISender pSender, HKEvent pEvent);
	void OnPerFrameSynchronizationCallback      (HISender pSender, HKEvent pEvent);

	//
	// this one we should run once per frame
	void		PrepareBuffers(FBEvaluateInfo *pEvalInfo, bool cubeMapSetup);

	// this one we should run once per pair<frame, camera>
	void		PrepareBuffersFromCamera();

	// prepare all needed information about camera (modelview, proj, far, near, etc.)

	struct CTilingInfo
	{
		int offsetX;
		int offsetY;
		int width;
		int height;

		double *tilingMatrix;
	};

	void		PrepareCamera(FBCamera *pCamera, const CTilingInfo &tilingInfo, const bool cubeMapSetup, int cubeMapFace, CubeMapRenderingData *const cubemap);

	void		PushCameraCache();
	void		PopCameraCache();

	//
	//

	
	bool		PrepRender();
	bool		BindUberShader(bool overrideShading=false, const EShadingType overrideShadingType=eShadingTypeFlat);
	void		UnBindUberShader();

	// per pass approach
	void		RenderPass(const EShaderPass pass);

	void		RenderPassShaderModels(	const EShaderPass pass, 
											CBaseShaderCallback *pCallback, 
											FBShader *pShader, 
											CBaseShaderInfo *pShaderInfo, 
											CBaseShaderCallback *&pLastType,
											const bool IsFBModelDraw);

	bool		PrepRenderForFBModel(FBModel *pModel);
	bool		RenderPassModelDraw(CBaseShaderCallback *pCallback, const CRenderOptions &options, FBModel *pModel);
	bool		RenderPassModelDraw(CBaseShaderCallback *pCallback, const CRenderOptions &options, FBModel *pModel, const int meshIndex);
	bool		RenderPassModelDraw2(CBaseShaderCallback *pCallback, const CRenderOptions &options, FBModel *pModel, const int meshIndex);

	const int	GetNumberOfBackgroundShadersToDraw() const;

	CBaseShaderInfo *FindShaderInfo(FBShader *pShader, CBaseShaderCallback *pCallback)
	{
		return mShadersInspector.GetShaderInfo(pShader, false, pCallback);
	}

	CShadersInspector &GetShadersInspector() {
		return mShadersInspector;
	}

	// classic drawing 
	void		RenderSceneClassic(FBRenderOptions *pRenderOptions);
	
	void		PreRenderSceneShaderGroups(const CRenderOptions &options);
	void		RenderSceneShaderGroups(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, RenderingStats &stats);
	void		RenderSceneShaderGroups2(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions);

	void		RenderScene( ERenderLayer layerId, CRenderOptions &options, FBRenderOptions *pFBRenderOptions, bool renderTransparency, RenderingStats &stats );

	void		RenderSceneCallbacks(bool selectionPass, bool useEarlyZ, bool backModelsOnly);
	bool		RenderModel(bool bindTextures, FBModel *pModel, const int modelIndex);

	void				RenderSceneForSelection(FBRenderOptions *pRenderOptions);

	void		Textures_InitOGL(FBRenderOptions *pRenderOptions);
	void		BindMaterialTextures(FBMaterial *pMaterial, FBRenderOptions *pRenderOptions, bool forceInit);

	const int			FindMeshIndex( FBModel *pModel, FBMaterial *pMaterial, FBShader *pShader );

	void				UploadModelInfo(FBModel *pModel, bool uploadAndBind);
	void				UploadMeshInfo(const MeshGLSL &meshInfo, bool uploadAndBind);
	void				UploadShader(FBShader *pShader);
	//void				UploadMaterial(FBMaterial *pMaterial);
	void				ConstructFromFBMaterial( FBMaterial *pMaterial, MaterialGLSL &mat );
	void				ConstructFromFBShader( FBShader *pShader, ShaderGLSL &shader );

	void				PrepProjectors();

	bool				BindProjectors( const CProjectors	*pProjectors, const GLuint muteTextureId );

	void			BindRimTexture(const GLuint id);
	void			BindMatCapTexture(const GLuint id);
	
	void		UnBindBuffers()
	{
		mBufferShader.UnBind();
		mBufferTexture.UnBind();
		mBufferMaterial.UnBind();
		mBufferMesh.UnBind();
		mBufferModel.UnBind();
	}


protected:
	// LIGHTS

	// list of used lights, could be exclusive from current composition lights list
	std::vector<FBLight*>					mUsedSceneLights;
	std::vector<FBLight*>					mUsedInfiniteLights;
	std::vector<FBLight*>					mUsedPointLights;

	std::auto_ptr<CGPUShaderLights>			mGPUSceneLights;
	
	// last lights for a frame - default if no lights, scene lights if no composition override
	CGPUShaderLights					*mLastLightsBinded;

	// these lights have a big priority and override the scene light list
	CGPUShaderLights					*mCompositionLights;

public:

	static void ConstructDefaultLight0(bool inEyeSpace, const mat4 &lViewMatrix, const mat4 &lViewRotationMatrix, LightDATA &light);
	static void ConstructDefaultLight1(bool inEyeSpace, const mat4 &lViewMatrix, const mat4 &lViewRotationMatrix, LightDATA &light);
	static void ConstructFromFBLight( const bool ToEyeSpace, const mat4 &lViewMatrix, 
		const mat4 &lViewRotationMatrix, FBLight *pLight, LightDATA &light );

	// this is a GPU buffer preparation, not an actual binding
	void			PrepFBSceneLights();
	void			PrepLightsInViewSpace(CGPUShaderLights *pLights);
	
	void			PrepLightsFromFBList(CGPUShaderLights *pLightsManager, CCameraInfoCache &cameraCache, std::vector<FBLight*> &lights);

	void		MapLightsOnGPU();

	// when shaderLights nullptr, we will bind all the fbscene lights
	bool		BindLights( const bool resetLastBind, const CGPUShaderLights *pShaderLights=nullptr );

	void UseCompositionLights( CGPUShaderLights *pShaderLights ) {
		mCompositionLights = pShaderLights;
	}

	const int GetNumberOfUsedLights() const {
		return (int) mUsedSceneLights.size();
	}
	std::vector<FBLight*> &GetUsedLightsVector() {
		return mUsedSceneLights;
	}

	// just prepare gpu buffer (no binding)
	bool PrepShaderLights( const bool useSceneLights, FBPropertyListObject *AffectingLights, 
							std::vector<FBLight*> &shaderLightsPtr, CGPUShaderLights *shaderLights );

	// TODO: bind a light buffer to the uber shader and update light count

	CGPUShaderLights			*GetGPUSceneLightsPtr() {
		return mGPUSceneLights.get();
	}

public:
	
	// NOTE: real far plane is needed only for cluster lighting
	//		at the moment is not used
	void SetFarPlaneCritical(const float value);
	//void CalculateRealFarPlane(CCameraInfoCache *pCache);
	const float GetFarPlane() const;

	FBCamera *GetCamera() const;
	CCameraInfoCache &GetCameraCache();

	void	ClearCache();

	// call this value when force is false, then value will be used only if hint if false
	//void		SetLogarithmicDepth(const bool value);

	// GetShaderFXPtr
	//	- initialize - flag to 
	Graphics::BaseMaterialShaderFX		*GetShaderFXPtr(const int shaderIndex, bool initialize=true);
	void EvaluateOnShaderFX( std::function<void(Graphics::BaseMaterialShaderFX*)> f )
	{
		for (int i=0; i<Graphics::MATERIAL_SHADER_COUNT; ++i)
		{
			if ( nullptr != GetShaderFXPtr(i, false) )
				f( GetShaderFXPtr(i, false) );
		}
	}


	Graphics::ShaderComposite	*GetCompositeShaderPtr();

	

	const bool IsWaiting() { return mWait; }
	void Wait(const bool value) { mWait = value; }

	const bool IsRenderToBuffer() const { return mRenderToBuffer; }
	void SetRenderToBufferState(const bool state) { mRenderToBuffer = state; }

	void	LoadShader();
	void SetLogarithmicDepth(const bool value);

	const CRenderLayerArray &GetRenderLayersInfo() const {
		return mShadersInspector.GetRenderLayersInfo();
	}

public:

	bool			RenderAllShadowMaps(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions,
										const int maxShadowMapSize = 512,
										const bool useGeomCache=true,
										const int numberOfSplits=4
										);

	// output 2d quads with shadows textures on screen (for debug purpose)
	void		PostRenderingShadows();
	void		PostRenderingOverviewCam();

	void	AddShadowNode(FBComponent *pNode);
	void	RemoveShadowNode(FBComponent *pNode);
	void	ClearShadowNodes();
	
	// get light MVP
	const int GetNumberOfShadows() const;
	void GetShadowLightMatrix(const int index, mat4 &mvp);
	void GetShadowLightMatrix( FBComponent *pComponent, mat4 &mvp );

	void GetShadowIdAndLightMatrix( FBComponent *pComponent, int &id, mat4 &mvp );

	const GLuint	GetShadowTextureId() const;

protected:

	struct ShadowLightDATA
	{
		mat4		modelView;
		mat4		proj;
		mat4		invModelView;

		float		nearPlane;
		float		farPlane;

		vec2		temp;
	};

	// number of shadows in scene
	std::vector<FBComponent*>		mShadowNodes;
	std::vector<ShadowLightDATA>	mShadowLightData;


	// shadows work
	int							mNumberOfShadows;
	std::auto_ptr<FrameBuffer>	mFrameBufferShadow;
	GLuint						mTextureShadowArray;	// 2d texture array
	GLuint						mTextureShColorArray;	// color needed for shadow masking and transparency
	GLuint						mBufferShadowRect;
	GLuint						mTextureTest;

	struct
	{
		bool			enableMS;
		int				samples;
	}	mShadowCreationInfo;

	void			ShadowConstructor();

	void			InitShadowTexture( const int texture_size, const int numberOfShadows, bool enableMS, int samples );
		
	void			FreeShadowResources();

	void			BindShadowsTexture(const int location, const bool shadowCompare);
	void			UnBindShadowsTexture(const int location);

	bool ComputeShadowLightMatrices( FBLight *pLight, FBModel *pVolumeModel, ShadowLightDATA &data );
	bool ComputeShadowSpotLightMatrices( FBLight *pLight, FBModel *pVolumeModel, ShadowLightDATA &data );
	bool ComputeShadowInfiniteLightMatrices( FBLight *pLight, FBModel *pVolumeModel, ShadowLightDATA &data );

	void	ShadowsOnContextChange();
	
protected:
	
	// background FBO needed in only one case - some shader uses matte shading !!!

	// std::auto_ptr<FrameBuffer>			mFrameBufferBack;
	// CFrameBufferMS						mFrameBufferBackMS;

public:
	
	// first of all render this background
	void	DrawBackground(const bool drawSimple, const bool clearBuffers, int regionWidth, int regionHeight);

	// TODO: how to prep background when render to cubemap ?!
	/*
	void PrepBackground(const bool useMS, const int samples, const int width, const int height);
	// then blit to screen or specified FBO

	void	DrawBackground(const bool drawSimple, const bool clearBuffers);

	void BindBackground()
	{
		if (mFrameBufferBack.get() )
			mFrameBufferBack->Bind();
	}
	void UnBindBackground()
	{
		if (mFrameBufferBack.get() )
			mFrameBufferBack->UnBind();
	}

	void	BindBackgroundTexture();
	*/
protected:
	// MoBu scene data

	FBSystem			mSystem;
	FBCriticalSection	mCriticalSection;
	FBFastLock			mFastLock;
	//FBProgress			mProgress;

	HdlFBPlugTemplate<FBCamera>					mCamera;		// gpu scene camera for processing

protected:

	//
	bool				mNeedEvaluate;

	//bool				mNeedProjectorsShader;
	//bool				mNeedIBLShader;
	bool				mNeedCompositeShader;
	//bool				mLogarithmicDepthHint;		//!< global value that we should take in account

	float					mRealFarPlane;

	bool				mSettingsDepthLog;
	bool				mSettingsDepthNV;

	bool				mWait;
	bool				mRenderToBuffer;

	// stats about scene shader groups

	FBTime				mLastTime;
	FBMatrix			mLastModelView;
	
	int					mNumberOfUnReadyModels;

	// pass common scene data to a shader
	GLint						mVertexId;
	GLint						mFragmentId;

	GLint						mLocAllTheMeshes;
	GLint						mLocAllTheModels;

	GLint						mLocTexture;
	GLint						mLocMaterial;
	GLint						mLocShader;
	GLint						mLocProjectors;

	CGPUBufferNV				mBufferModel;
	CGPUBufferNV				mBufferMesh;
	CGPUBufferNV				mBufferTexture;
	CGPUBufferNV				mBufferMaterial;
	CGPUBufferNV				mBufferShader;

	CCameraInfoCache				mCameraCache;

	struct CPushCameraCacheInfo
	{
		FBCamera				*pCamera;
		CCameraInfoCache		cameraCache;
	};
	CPushCameraCacheInfo			mPushCameraCacheInfo;

	
	Graphics::MaterialShaderManager		mMaterialShaders;

	std::auto_ptr<Graphics::ShaderComposite>	mShaderComposite;

	bool						mLockSmallEvents; // don't process small events during clear, load or merge

	CTexturesInspector			mTexturesInspector;
	
	CShadersInspector			mShadersInspector;
	CShadersFactory				mShadersFactory;
	std::vector<char>			mUsedShaderFlags;

	CMaterialsInspector			mMaterialsInspector;

	CModelsInspector			mModelsInspector;

	CProjectors					mProjectors;

	void CCameraInfoCachePrep(FBCamera *pCamera, const CTilingInfo &tilingInfo, CCameraInfoCache &cache);

	void MarkModelsWithIncludeExcludeLists(FBPropertyListObject *pIncludeList, FBPropertyListObject *pExcludeList);
	void MarkModelsInsideTheVolume(const FBVector3d &vmin, const FBVector3d &vmax);

// reflections
public:

	void	AddCubeMapNode(FBComponent *pReflectionNode);
	void	RemoveCubeMapNode(FBComponent *pReflectionNode);
	void	ClearCubeMapNodes();

	bool	RenderAllCubeMaps(FBRenderOptions *pFBRenderOptions, const bool videoRendering, unsigned int frameId, const int maxCubeMapSize);

	const unsigned GetCubeMapColor() {
		if (mFrameBufferCubeMap.get() )
			return mFrameBufferCubeMap->GetColorObject();

		return 0;
	}

	void UploadCubeMapUniforms(const GLuint textureObject);

	//
	// camera nodes are dynamic masks with render from camera option !

	void	AddCameraNode(FBComponent *pCameraNode);
	void	RemoveCameraNode(FBComponent *pCameraNode);
	void	ClearCameraNodes();

	bool	RenderAllCameras(FBRenderOptions *pFBRenderOptions, const bool videoRendering, unsigned int frameId, const int maxOutputSize);

	void CameraOnContextChange();
	void CubeMapOnContextChange();

	void FreeCameraResources();
	void FreeCubeMapResources();

protected:

	// buffer for rendering cubemap 6 sides at once
	std::auto_ptr<FrameBuffer>			mFrameBufferCubeMap;
	// buffer for rendering cubemap into panorama
	std::auto_ptr<FrameBuffer>			mFrameBufferPano;
	// buffer for rendering camera into the texture
	std::auto_ptr<FrameBuffer>			mFrameBufferCamera;

	std::auto_ptr<GLSLShader>			mShaderPano;

	// number of cubemaps in scene
	std::vector<FBComponent*>		mCameraNodes;
	std::vector<FBComponent*>		mCubeMapNodes;

	// this sould be called once per frame (not per render cycle or camera)
	void	RenderCubeMap(std::auto_ptr<FrameBuffer> &framebuffer, FBRenderOptions *pFBRenderOptions, CubeMapRenderingData &data, const bool logDepth, bool videoRendering, RenderingStats &stats);
	bool	RenderPano(CubeMapRenderingData &data, const GLuint cubemapId, const bool generateMipMaps);
	void	RenderCamera(FBRenderOptions *pFBRenderOptions, CubeMapRenderingData &data, const bool logDepth, RenderingStats &stats);

	static void	PrepCubeMapMatrices(const int faceId, const float nearPlane, const float farPlane, 
		const vec3 &pos, mat4 &proj, mat4 &mdl, mat4 &inv);

	

//
// Geometry cache in video memory !
	
// DEPRICATED: use remove gpu duplicate script intead !!
	/*
public:

	struct GeometryCacheInstance
	{
		// filePath is a key to avoid making caches duplicates
		//FBString					filePath;

		int		numberOfInstances;

		// cached resources
		CGPUCacheModel				*cacheModel;

		// pre-cached model information 
		//CGPUModelRenderCached		*mModelRender;
		//CGPUVertexData				*mVertexData;

		// for ray casting
		GPUCacheGeometry			*queryCacheGeometry;
		PHYSICS_INTERFACE::IWorld	*cacheWorld;

		GeometryCacheInstance()
		{
			numberOfInstances = 0;
			//filePath = "";
			cacheModel = nullptr;
			queryCacheGeometry = nullptr;
			cacheWorld = nullptr;
		}
	};

	std::map<std::string, GeometryCacheInstance>	mGeomCaches;

	//std::vector<GeometryCacheInstance>		mGeomCaches;

public:

	void ClearAllCaches();

	void LoadCache( const char *filename );
	void UnLoadCache( const char *filename );

	GeometryCacheInstance &GetCacheInstance( const char *filePath );
	*/
};