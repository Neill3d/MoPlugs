
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: callbacks.h
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

//#include "ContentInspector.h"

#include "shared_rendering.h"
#include "graphics_framebuffer.h"
#include "render_layer_info.h"


/////////////////////////////////////////////////
//

#define SHADER_PASS_COUNT		3

enum EShaderPass
{
	eShaderPassInvalid = 0,
	//eShaderPassBackground	= 1 << 0,	// before everything
	//eShaderPassSelection	= 1 << 1,	// simple drawing with unique color ids 
	//eShaderPassShadows		= 1 << 2,	// simple drawing with transparency / alpha support
	
	eShaderPassOpaque			= 1 << 3,	// this is a NORMAL state
	eShaderPassAlphaToCoverage	= 1 << 4,	// same render layer but second pass
	eShaderPassTransparency		= 1 << 5,	// render to Transparency render layer
	eShaderPassAdditive			= 1 << 6,
	eShaderPassWireFrame		= 1 << 7	// render simple mesh, just a wire cage

	//eShaderPassSoftParticles = 1 << 5,	// draw on top with using current or previos frame depth map
	//eShaderPassCoverageAlpha = 1 << 6	// could be used as an alternative to transparency pass when we need to do an alpha cut
};

enum ERenderOptions
{
	//eShaderPassBackground	= 1 << 0,	// before everything
	//eShaderPassSelection	= 1 << 1,	// simple drawing with unique color ids 
	//eShaderPassShadows		= 1 << 2,	// simple drawing with transparency / alpha support
	//eShaderPassSoftParticles = 1 << 5,	// draw on top with using current or previos frame depth map
	//eShaderPassCoverageAlpha = 1 << 6	// could be used as an alternative to transparency pass when we need to do an alpha cut
};

enum ERenderGoal
{
	eRenderGoalInvalid		= 1 << 0,
	eRenderGoalShading		= 1 << 1,
	//eRenderGoalTransparency = 1 << 2,	// render to special transparency buffer
	eRenderGoalSelectionId	= 1 << 3,	// simple render with models unique color id
	//eRenderGoalBackground	= 1 << 4,	// render to special background buffer (for matte)
	eRenderGoalEarlyZ		= 1 << 5,	// simple render for depth only
	eRenderGoalShadows		= 1 << 6	// simple render for shadows only (to texture array)
};

struct CRenderOptions
{
private:
	ERenderGoal		goal;
	EShaderPass		pass;

	// composition of ERenderLayer
	int				layerId;

	unsigned int	uniqueFrameId;	// rendering frame id

	bool		offlineRender;		// is we are inside the offline render process

	bool					cubeMapRendering;
	CubeMapRenderingData	*pCubeMapData;

	// global information about all the scene shaders
	
	bool		hasCameraBackground;	// assigned media to camera background
	
	// decision about rendering
	bool		renderToBackgroundLayer;		// should we render to background framebuffer
	bool		renderToTransparencyLayer;	// should we render to transparency framebuffer
	bool		renderToSecondaryLayer;

	bool		textureMapping;
	GLuint		muteTextureId; // this texture id is currently attached to a framebuffer

	bool		frustumCulling;

	int			newX;
	int			newY;
	int			newWidth;
	int			newHeight;
	int			lViewport[4];
	bool		enableMultisampling;
	int			samples;
	int			coverageSamples;
	// for a secondary framebuffer
	bool		enableMultisampling2;
	int			samples2;
	int			coverageSamples2;
	//
	double		ssfact;
	EResamplingFilter		resampleFilter;

	//
	int			shadowResolution;
	bool		enableShadowMS;
	int			shadowSamples;

	//
	FBCamera	*pCamera;

	FBModelCullingMode		lastCullingMode;

public:
	CRenderOptions()
	{
		goal = eRenderGoalInvalid;
		pass = eShaderPassInvalid;

		layerId = 1 << eRenderLayerMain;

		uniqueFrameId = 0;

		offlineRender = false;
		
		hasCameraBackground = false;	// assigned media to camera background
	
		// dicision about rendering
		renderToBackgroundLayer = false;		// should we render to background framebuffer
		renderToTransparencyLayer = false;	// should we render to transparency framebuffer
		renderToSecondaryLayer = false;

		textureMapping = true;
		frustumCulling = true;

		muteTextureId = 0;

		newX = 0;
		newY = 0;
		newWidth = 1;
		newHeight = 1;
		
		enableMultisampling = false;
		samples = 0;
		coverageSamples = 0;
		enableMultisampling2 = false;
		samples2 = 0;
		coverageSamples2 = 0;
		ssfact = 1.0;
		resampleFilter = eResamplingFilterLanczos;

		shadowResolution = 2048;
		enableShadowMS = false;
		shadowSamples = 8;

		pCamera = nullptr;

		lastCullingMode = kFBCullingOff;
	}

	const FBModelCullingMode GetLastCullingMode() const
	{
		return lastCullingMode;
	}
	void SetLastCullingMode(const FBModelCullingMode newMode)
	{
		lastCullingMode = newMode;
	}
	void TurnOffCullingMode()
	{
		lastCullingMode = kFBCullingOff;
		glDisable(GL_CULL_FACE);
	}

	void GrabCameraViewportSize()
	{
		glGetIntegerv( GL_VIEWPORT, &lViewport[0] );

		newX = pCamera->CameraViewportX;
		newY = pCamera->CameraViewportY;
		newWidth = pCamera->CameraViewportWidth;
		newHeight = pCamera->CameraViewportHeight;
	}
	
	void SetGLViewport()
	{
		glViewport( lViewport[0], lViewport[1], lViewport[2], lViewport[3]);
	}
	
	//
	// GET

	const bool IsOfflineRender() const {
		return offlineRender;
	}
	const bool IsCubeMapRender() const {
		return cubeMapRendering;
	}
	const CubeMapRenderingData *GetCubeMapData() const {
		return pCubeMapData;
	}
	const bool HasCameraBackground() const {
		return hasCameraBackground;
	}

	const bool IsRenderToBackgroundLayer() const {
		return renderToBackgroundLayer;
	}
	const bool IsRenderToTransparencyLayer() const {
		return renderToTransparencyLayer;
	}
	const bool IsRenderToSecondaryLayer() const {
		return renderToSecondaryLayer;
	}
	const bool IsTextureMappingEnable() const {
		return textureMapping;
	}
	void SetTextureMapping(const bool flag) {
		textureMapping = flag;
	}
	void MuteTextureId(const GLuint textureId) {
		muteTextureId = textureId;
	}
	const GLuint GetMutedTextureId() const {
		return muteTextureId;
	}
	//
	// SET

	
	void SetCameraBackground(const bool flag) {
		hasCameraBackground = flag;
	}
	void SetRenderToBackgroundLayer(const bool flag) {
		renderToBackgroundLayer = flag;
	}
	void SetRenderToTransparencyLayer(const bool flag) {
		renderToTransparencyLayer = flag;
	}
	void SetRenderToSecondaryLayer(const bool flag) {
		renderToSecondaryLayer = flag;
	}
	
	void SetOfflineRender(const bool flag) {
		offlineRender = flag;
	}
	void SetCubeMapRender(const bool flag, CubeMapRenderingData *pData)
	{
		cubeMapRendering = flag;
		pCubeMapData = pData;
	}
	void SetGoal(const ERenderGoal newGoal) {
		goal = newGoal;
	}
	void SetPass(const EShaderPass newPass) {
		pass = newPass;
	}
	void SetRenderLayerId(const ERenderLayer id) {
		layerId = 1 << (int) id;
	}
	void AppendRenderLayerId(const ERenderLayer id) {
		layerId = layerId | 1 << (int) id;
	}
	void SetRenderLayerAllIds() {
		layerId = 1 << eRenderLayerBackground | 1 << eRenderLayerMain | 1 << eRenderLayerSecondary;
	}
	//
	//

	const ERenderGoal GetGoal() const {
		return goal;
	}
	const EShaderPass GetPass() const {
		return pass;
	}
	const int GetRenderLayerIds() const {
		return layerId;
	}
	const bool HasRenderLayerId(int id) const
	{
		int flag = 1 << id;
		return ((layerId & flag) > 0);
	}
	void SetCamera(FBCamera *newCamera) {
		pCamera = newCamera;
	}
	FBCamera *GetCamera() const {
		return pCamera;
	}
	
	const int GetX() const {
		return newX;
	}
	const int GetY() const {
		return newY;
	}
	const int GetWidth() const {
		return newWidth;
	}
	const int GetHeight() const {
		return newHeight;
	}
	const int GetViewportWidth() const {
		return lViewport[2];
	}
	const int GetViewportHeight() const {
		return lViewport[3];
	}
	void SetSuperSamplingScale(const double scale) {
		ssfact = scale;
	}
	void SetResampleFilter(const EResamplingFilter filter) {
		resampleFilter = filter;
	}
	void SetSamplingOptions(bool multisampling, int newSamples, int newCoverage) {
		enableMultisampling = multisampling;
		samples = newSamples;
		coverageSamples = newCoverage;
	}
	void SetSamplingOptions2(bool multisampling, int newSamples, int newCoverage) {
		enableMultisampling2 = multisampling;
		samples2 = newSamples;
		coverageSamples2 = newCoverage;
	}
	const bool IsMultisampling() const {
		return enableMultisampling;
	}
	const int GetDepthSamples() const {
		return samples;
	}
	const int GetCoverageSamples() const {
		return coverageSamples;
	}
	const bool IsMultisampling2() const {
		return enableMultisampling2;
	}
	const int GetDepthSamples2() const {
		return samples2;
	}
	const int GetCoverageSamples2() const {
		return coverageSamples2;
	}
	const double GetScaleFactor() const {
		return ssfact;
	}
	const EResamplingFilter GetResampleFilter() const {
		return resampleFilter;
	}
	const unsigned int GetUniqueFrameId() const {
		return uniqueFrameId;
	}
	void SetUniqueFrameId(const int id) {
		uniqueFrameId = id;
	}

	const bool IsFrustumCullingEnabled() const
	{
		return frustumCulling;
	}
	void SetFrustumCulling(const bool value) 
	{
		frustumCulling = value;
	}

	const bool IsShadowRendering() const
	{
		return (goal == eRenderGoalShadows);
	}

	void SetShadowParams(const int resolution, const bool enableMS, const int samples)
	{
		shadowResolution = resolution;
		enableShadowMS = enableMS;
		shadowSamples = samples;
	}

	const int GetShadowResolution() const {
		return shadowResolution;
	}
	const bool IsShadowMultisamplingEnabled() const {
		return enableShadowMS;
	}
	const int GetShadowSamples() const {
		return shadowSamples;
	}

};

/*
kFBPassInvalid                  = 0,        //!< No pass selected.
		kFBPassPreRender                = 1 << 0,   //!< Before anything.
		kFBPassFlat                     = 1 << 1,   //!< Lighting off.
		kFBPassLighted                  = 1 << 2,   //!< Lighting on.
		kFBPassMatte                    = 1 << 3,   //!< Alpha > 0.5 will show up.
		kFBPassZTranslucent             = 1 << 4,   //!< Writes to depth buffer. 
		kFBPassZTranslucentAlphaTest    = 1 << 5,   //!< Writes to depth buffer where Alpha > 0.5.
		kFBPassTranslucent              = 1 << 6,   //!< Models are blended.
		kFBPassAddColor                 = 1 << 7,   //!< Models are blended additively.
		kFBPassTranslucentZSort         = 1 << 8,   //!< Models are sorted and blended. 
		kFBPassPostRender               = 1 << 9    //!< After everything.
		*/
//
// for each shader we also store a user data like light custom list (and gpu buffer)
//
class CBaseShaderInfo
{

public:

	//! a constructor
	CBaseShaderInfo( FBShader *pShader )
		: mShader(pShader)
	{}

	FBShader	*GetFBShader() const
	{
		return mShader;
	}

	virtual void EventBeforeRenderNotify()
	{
	}

	// change OGL context
	virtual void ChangeContext()
	{
	}

	virtual void EventConnNotify(HISender pSender, HKEvent pEvent)
	{}

protected:

	FBShader	*mShader;

};

//////////////////////////////////////////////////////////////////
//
class CBaseShaderCallback
{
protected:
	int mId;

public:

	//! a constructor
	CBaseShaderCallback(const int id)
		: mId( id )
	{}

	// higher level - bigger chance that callback will be used for a specified situation
	virtual const int PriorityLevel() const 
	{
		return 0;
	}

	virtual const int Id() const { return mId; } 

	// if callback could operate with that shader
	virtual const bool IsForShader(FBShader *pShader) = 0;
	virtual const bool IsForShaderAndPass(FBShader *pShader, const EShaderPass pass) = 0;

	// supported by this callback
	//	could one or multi passes
	virtual const int ShaderGroupGoal() const
	{
		return eRenderGoalInvalid;
	}

	virtual const bool IsMaterialEffect() const
	{
		return true;
	}

	virtual const bool IsNeedShaderInfo() const
	{
		return false;
	}

	// check what is supported by shader
	//	shader could draw transparency, opaque, soft particles, etc.
	/*
	virtual const int SupportedShaderPasses(FBShader *pShader) const
	{
		return eShaderPassOpaque;
	}
	*/

	// create a new shader info for a shader and the callback
	virtual CBaseShaderInfo	*CreateANewShaderInfo(FBShader *pShader)
	{
		return new CBaseShaderInfo(pShader);
	}

	///////////////// INFO METHODS
	
	virtual const char *shaderName() const
	{
		return "None";
	}

	

	////////////////// CALLBACK METHODS

	virtual	bool OnTypeBegin(const CRenderOptions &options, bool useMRT)
	{
		return false;
	}
	virtual void OnTypeEnd(const CRenderOptions &options)
	{}
	virtual bool OnInstanceBegin(const CRenderOptions &options, FBRenderOptions* pFBRenderOptions, FBShader *pShader, CBaseShaderInfo *pInfo)
	{
		return false;
	}
	virtual void OnInstanceEnd(const CRenderOptions &options, FBShader *pShader, CBaseShaderInfo *pInfo)
	{}

	virtual bool OnModelDraw(const CRenderOptions &options, FBRenderOptions* pFBRenderOptions, FBModel *pModel, CBaseShaderInfo *pInfo)
	{
		return false;
	}

	// ?!
	
	virtual bool OnMaterialBegin(const CRenderOptions &options, FBRenderOptions *pRenderOptions, FBMaterial *pMaterial, bool forceInit)
	{
		return false;
	}
	virtual void OnMaterialEnd(const CRenderOptions &options, FBMaterial *pMaterial)
	{}

	// executed one time per rendering frame (to setup gpu buffers, update data, etc.)
	//virtual void PreRenderSetup(const CRenderOptions &options, FBModel *pModel, FBShader *pShader)
	//{}

	virtual const bool IsNeedToPrepareInstance() const {
		return false;
	}
	virtual const bool IsNeedToPrepareModel() const {
		return false;
	}

	virtual const bool IsNeedToProcessFrameSync() const {
		return false;
	}
	virtual const bool IsNeedToProcessFrameEvaluation() const {
		return false;
	}

	virtual void PrepareInstance(const CRenderOptions &options, FBShader *pShader)
	{}
	virtual void PrepareModel(const CRenderOptions &options, FBModel *pModel, FBShader *pShader)
	{}

	virtual void FrameSync(FBShader *pShader)
	{}
	virtual void FrameEvaluation(FBShader *pShader)
	{}

	virtual void DetachRenderContext(FBRenderOptions *pFBRenderOptions, FBShader *pShader)
	{
	}

	// executed one time for each layer and each pass
	virtual bool OwnModelShade(const CRenderOptions &options, FBRenderOptions *pFBRenderOptions, FBModel *pModel, FBShader *pShader, CBaseShaderInfo *pInfo)
	{
		return false;
	}

	/////////////////// HELPER METHODS

	bool IsShaderTransparent(FBShader *pShader)
	{

		bool result = false;

		FBProperty *pTranspProp = pShader->PropertyList.Find("Transparency");

		if (nullptr != pTranspProp)
		{
			const int transparencyType = pTranspProp->AsInt();
			
			double transparencyValue = 1.0;
			pTranspProp = pShader->PropertyList.Find("TransparencyFactor");
			if (nullptr != pTranspProp)
				pTranspProp->GetData( &transparencyValue, sizeof(double));

			result = (transparencyType != 0 && transparencyValue < 1.0);
		}
		return result;
	}

	bool IsShaderBackground(FBShader *pShader)
	{
		FBProperty *prop = pShader->PropertyList.Find("Render On Back");

		if (nullptr != prop)
		{
			const int value = prop->AsInt();
			return (value != 0);
		}
		return false;
	}

protected:

	virtual bool OnTypeBeginPredefine(const CRenderOptions &options, bool useMRT)
	{
		return false;
	}
};

