#ifndef __OBJECT_SHAPE_H__
#define __OBJECT_SHAPE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_object.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>
#include <GL\glew.h>
#include <vector>
#include "graphics\Framebuffer.h"
#include "compositeMaster_common.h"
#include "compositeMaster_shaders.h"
#include "compositeMaster_computeShaders.h"
#include "MB_renderer.h"
#include "algorithm\math3d.h"
#include "graphics_framebuffer.h"
#include "render_layer_info.h"

#define		SOURCE_ICON			"\\CompositeMaster\\sourceNode.png"
#define		EFFECT_ICON			"\\CompositeMaster\\fxNode.png"
#define		BLEND_ICON			"\\CompositeMaster\\blendNode.png"
#define		MASTER_ICON			"\\CompositeMaster\\masterNode.png"

#define MARKER__ASSETPATH	    "Browsing/Templates/Shading Elements/Composition/Models"

//--- Registration define
#define COMPOSITEMARKER__CLASSNAME		ObjectCompositeMarker
#define COMPOSITEMARKER__CLASSSTR		"ObjectCompositeMarker"
#define COMPOSITEMARKER__DESCSTR	    "Object Composite Marker"

#define COMPOSITEBASE__CLASSNAME			ObjectCompositeBase
#define COMPOSITEBASE__CLASSSTR				"Composite Base"

#define COMPOSITEFILTER__CLASSNAME			ObjectCompositeFilter
#define COMPOSITEFILTER__CLASSSTR			"Composite Filter"

#define COMPOSITELAYER__CLASSNAME			ObjectCompositeLayer
#define COMPOSITELAYER__CLASSSTR			"Composite Layer"

/** Class declaration.
*   Overloads the necessary functions when a class inherits from
*   FBComponent (or its derivatives).
*   \param Name     Name of class to declare.
*   \param Parent   Parent class to associate with \e Name.
*/

//
// NEED THIS TO MAKE CUSTOM FBDELETE PROC
//

#define FBClassDeclare2( Name,Parent )   \
public:                                       \
    typedef Parent ParentClass;               \
    virtual const char *ClassName() override ;\
    static int TypeInfo;                      \
    virtual bool Is( int pTypeId )  override ;\
    virtual int GetTypeId()         override ;\
    virtual ~Name();                          \
/** Return to the default scope (private) */\
private:  


/////////////////// FORWARD DECLARATION

enum ECompositeMask
{
	eCompositeMaskA,
	eCompositeMaskB,
	eCompositeMaskC,
	eCompositeMaskD
};

class ObjectCompositeBase;
class ObjectCompositeFilter;
class ObjectCompositeLayer;

//////////////////////////////////////////////////////////////////////
// interface for communicate with a root element

class CProcessingInfo
{

public:

	//! a constructor
	CProcessingInfo(const int w, const int h)
	{
		mWidth = w;
		mHeight = h;
	}

	int					GetWidth() const {
		return mWidth;
	}
	int					GetHeight() const {
		return mHeight;
	}

	bool				IsMasked();
	const GLuint		GetMaskId();

	void SetFilterInAndOutIds(const GLuint src, const GLuint dst)
	{
		srcTexId = src;
		dstTexId = dst;
	}

	const GLuint GetSrcTexId() const {
		return srcTexId;
	}
	const GLuint GetDstTexId() const {
		return dstTexId;
	}

protected:

	int				mWidth;
	int				mHeight;

	bool			mMasked;
	GLuint			mMaskId;

	// info about filter processing

	GLuint			srcTexId;
	GLuint			dstTexId;

};

class CCompositionInfo
{

public:
	//! a constructor
	CCompositionInfo(ObjectCompositeBase *pFinal)
	{}
	//! a destructor
	virtual ~CCompositionInfo()
	{}

	virtual bool UsedForBatchProcessing() const = 0;

	// TODO: layer should use it's own texture buffer if needed !
	//virtual const GLuint	QueryANewTextureBuffer() const = 0;

	//virtual void SetProcessingDimentions(const int w, const int h) = 0;

	// return composition dimentions
	virtual const int		GetWidth() const = 0;
	virtual const int		GetHeight() const = 0;

	virtual const float		GetPreviewSizeFactor() const = 0;

	// info about the current viewport rendering

	virtual FBCamera		*GetRenderCamera() = 0;
	virtual FBModel			*GetRenderCameraInterest() = 0;
	virtual const double	*GetRenderCameraMVP() const = 0;

	virtual const int		GetRenderWidth() const = 0;
	virtual const int		GetRenderHeight() const = 0;

	virtual const int		GetRenderBorderX() const = 0;
	virtual const int		GetRenderBorderY() const = 0;

	virtual const double	GetCameraNearPlane() const = 0;
	virtual const double	GetCameraFarPlane() const = 0;

	virtual bool IsRenderMultisampling() const = 0;
	virtual int	GetNumberOfSamples() const = 0;
	virtual bool IsLogarithmicDepth() const = 0;
	virtual bool IsReverseDepthRange() const = 0;

	virtual bool							UseCameraDOFSettings() const = 0;
	virtual FBCameraFocusDistanceSource		FocusDistanceSource() const = 0;
	virtual double							FocusSpecificDistance() const = 0;
	virtual double							FocusAngle() const = 0;

	/*
	strDefine = strDefine + "#define USE_MS\n";
			strDefine = strDefine + "#define SAMPLES 4\n";
			*/

	virtual const char *GetRenderSamplesDefineString() = 0;
	virtual const char *GetDepthBufferDefineString() const = 0;

	virtual const MainFrameBuffer *GetFrameBufferInfo(ERenderLayer layerId) const = 0;

	virtual bool IsShadowMultisampling() const = 0;
	virtual int GetShadowSamplesCount() const = 0;

	/*
	virtual const GLuint	GetRenderColorId() const = 0;
	virtual const GLuint	GetRenderDepthId() const = 0;
	virtual const GLuint	GetRenderNormalId() const = 0;
	virtual const GLuint	GetRenderMaskId() const = 0;
	virtual const GLuint	GetRenderPositionId() const = 0;

	// get multisample version of a texture (used to SSAO, toon shading, etc.)

	virtual const GLuint	GetRenderColorMSId() const = 0;
	virtual const GLuint	GetRenderDepthMSId() const = 0;
	virtual const GLuint	GetRenderNormalMSId() const = 0;
	virtual const GLuint	GetRenderMaskMSId() const = 0;
	virtual const GLuint	GetRenderPositionMSId() const = 0;
	*/
};


/////////////////////////////////////////////////////////////////////
//

class CProgramUniformsBase
{
protected:
	GLint		outputTexelSize;
	GLint		layerTexelSize;
	GLint		renderTexelSize;
	GLint		renderSize;
	GLint		renderBorder;
	GLint		previewScaleFactor;

	GLint		maskInverse;
	GLint		weight;

	GLint		cameraNearPlane;
	GLint		cameraFarPlane;

	GLint		cameraViewProj;

	GLint		colorSampler;
	GLint		depthSampler;
	GLint		normalSampler;
	GLint		maskSampler;
	GLint		positionSampler;

public:
	// a constructor
	CProgramUniformsBase()
	{
		outputTexelSize = -1;
		layerTexelSize = -1;
		renderTexelSize = -1;
		renderSize = -1;
		renderBorder = -1;
		previewScaleFactor = -1;

		maskInverse = -1;
		weight = -1;

		cameraNearPlane = -1;
		cameraFarPlane = -1;
		cameraViewProj = -1;

		colorSampler = -1;
		depthSampler = -1;
		normalSampler = -1;
		maskSampler = -1;
		positionSampler = -1;
	}

	virtual ~CProgramUniformsBase()
	{}

	virtual void Prep(const GLuint programId)
	{
		outputTexelSize = glGetUniformLocation(programId, "outputTexelSize" );
		if (outputTexelSize < 0)
			outputTexelSize = glGetUniformLocation(programId, "outputSize" );
		layerTexelSize = glGetUniformLocation(programId, "layerTexelSize" );

		renderTexelSize = glGetUniformLocation(programId, "renderTexelSize" );
		renderSize = glGetUniformLocation(programId, "renderSize" );

		renderBorder = glGetUniformLocation(programId, "renderBorder" );
		previewScaleFactor = glGetUniformLocation(programId, "previewScaleFactor" );

		maskInverse = glGetUniformLocation(programId, "maskInverse" );
		weight = glGetUniformLocation(programId, "weight" );

		cameraNearPlane = glGetUniformLocation(programId, "cameraNearPlane" );
		cameraFarPlane = glGetUniformLocation(programId, "cameraFarPlane" );
		cameraViewProj = glGetUniformLocation(programId, "cameraViewProj" );

		colorSampler = glGetUniformLocation(programId, "colorSampler" );
		depthSampler = glGetUniformLocation(programId, "depthSampler" );
		normalSampler = glGetUniformLocation(programId, "normalSampler" );
		maskSampler = glGetUniformLocation(programId, "maskSampler" );
		positionSampler = glGetUniformLocation(programId, "positionSampler" );
	}

	bool IsColorSamplerUsed() { return colorSampler >= 0; }
	bool IsDepthSamplerUsed() { return depthSampler >= 0; }
	bool IsNormalSamplerUsed() { return normalSampler >= 0; }
	bool IsMaskSamplerUsed() { return maskSampler >= 0; }
	bool IsPositionSamplerUsed() { return positionSampler >= 0; }

	virtual void Upload(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, 
		int layerWidth, int layerHeight, const bool invertCompositeMask, const float _weight)
	{
		if (outputTexelSize >= 0)
		{
			const int w = prInfo.GetWidth();
			const int h = prInfo.GetHeight();

			glUniform2f(outputTexelSize, 1.0f / (float)w, 1.0f / (float)h);
		}
		if (layerTexelSize >= 0)
		{
			glUniform2f(layerTexelSize, 1.0f / (float)layerWidth, 1.0f / (float)layerHeight);
		}
		if (renderTexelSize >= 0)
		{
			const int w = pInfo->GetRenderWidth();
			const int h = pInfo->GetRenderHeight();

			glUniform2f(renderTexelSize, 1.0f / (float)w, 1.0f / (float)h);
		}
		if (renderSize >= 0)
		{
			const int w = pInfo->GetRenderWidth();
			const int h = pInfo->GetRenderHeight();

			glUniform2f(renderSize, (float)w, (float)h);
		}
		if (renderBorder >= 0)
		{
			const int x = pInfo->GetRenderBorderX();
			const int y = pInfo->GetRenderBorderY();

			glUniform2f(renderBorder, (float)x, (float)y);
		}
		if (previewScaleFactor >= 0)
		{
			float scaleFactor = 1.0f * prInfo.GetWidth() / pInfo->GetRenderWidth();
			glUniform1f(previewScaleFactor, scaleFactor);
		}
		if (maskInverse >= 0)
		{
			glUniform1f(maskInverse, (invertCompositeMask) ? 1.0f : 0.0f);
		}
		if (weight >= 0)
			glUniform1f(weight, _weight);

		if (cameraNearPlane >= 0)
		{
			const double nearPlane = pInfo->GetCameraNearPlane();
			glUniform1f(cameraNearPlane, (float) nearPlane );
		}
		if (cameraFarPlane >= 0)
		{
			const double farPlane = pInfo->GetCameraFarPlane();
			glUniform1f(cameraFarPlane, (float) farPlane );
		}
		if (cameraViewProj >= 0)
		{
			const double *mvp = pInfo->GetRenderCameraMVP();
			if (nullptr != mvp)
			{
				float fmatrix[16];
				for (int i=0; i<16; ++i)
					fmatrix[i] = (float) mvp[i];

				glUniformMatrix4fv( cameraViewProj, 1, GL_FALSE, fmatrix );
			}
		}

	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectCompositeMarker

class ObjectCompositeMarker : public FBModelMarker
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( ObjectCompositeMarker, FBModelMarker );

public:
	ObjectCompositeMarker(const char *pName, HIObject pObject= NULL);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectCompositeBase

class ObjectCompositeBase : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectCompositeBase, FBUserObject)
	FBDeclareUserObject(ObjectCompositeBase);

public:
	//! a constructor
	ObjectCompositeBase(const char *pName = NULL, HIObject pObject=NULL);

	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	//! Open Reality deletion function.
	virtual void FBDelete() override;
	void FBDeleteSilence();

public:

	FBPropertyBool							Active;
	//FBPropertyAnimatableDouble			Weight;		// percentage of an effect (like opacity factor)
	FBPropertyAction						ReLoadShader;

	FBPropertyAnimatableDouble				Opacity;	//!< control opacity of a blended image


	FBPropertyBool							UseCompositeMask;
	FBPropertyBaseEnum<ECompositeMask>		SelectCompositeMask;
	FBPropertyBool							InvertCompositeMask;
	FBPropertyListObject					CustomMask;				// attach a dynamic mask channels

	// processing dimentions (for layer-input size, for filter-processing size, for composition-output size)
	FBPropertyInt				OutWidth;
	FBPropertyInt				OutHeight;

	// layer or filter result - opengl texture id

	FBPropertyInt							TextureOGLId;

	// store name of the element in assets (to create a new element from the FBCreateObject)
	FBPropertyString						AssetName;

public:

	virtual void CopyFrom(ObjectCompositeBase *pNode) {}


	//
	// UPDATE RESOURCE FOR COMPONENTS
	//

	// update when ogl context has changed
	virtual void ChangeContext();

	virtual void ChangeGlobalSettings();
	
	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;


	// update when resolutin has changed
	//virtual void ReSize(const int x, const int y, const int w, const int h);

	//virtual void PrepRender(ObjectCompositeBase *pRoot);
	virtual bool ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo)
	{
		if (Active.AsInt() == 0)
			return false;
		if (0.0 == Opacity)
			return false;

		return true;
	}
	/*
	virtual bool ApplyEffect(const CObjectCompositionInfo *pInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId)
	{
		return false;
	}
	*/
	virtual const GLuint GetColorTextureId() const
	{
		return 0;
	}

	void ModifyDeleteFlag(const bool value)
	{
		Deleted = value;
	}

	//
	// Composite mask

	void SetCompositeMaskTextureId(const GLuint texid);
	const GLuint GetCompositeMaskTextureId();

	void BindCompositeMask();
	void UnBindCompositeMask();

	
protected:
	bool	mNeedProgramReload;

	bool				mRemoveChildren;
	std::vector<FBComponent*>	mRemoveComponents;

	int						mProcessingWidth;
	int						mProcessingHeight;

	GLuint					mCompositeMaskTextureId;

	bool		Deleted;	// flag for being deleted
	
	//bool	InitFrameBufferBase(const int width, const int height);
	//void	FreeFrameBufferBase();
	static bool	InitFrameBuffer(const int width, const int height, FrameBuffer *& pBuffer);
	static void	FreeFrameBuffer(FrameBuffer *& pBuffer);

	static void SetActive(HIObject object, bool value);
	static void SetUseMask(HIObject object, bool value);
	static int GetOutWidth(HIObject object);
	static int GetOutHeight(HIObject object);
	static int GetTextureOGLId(HIObject object);
	static void SetReLoadShader(HIObject object, bool value);

	static const char *GetAssetName(HIObject object);

	virtual const char *AssetNameString() {
		return "Base Composition";
	}

public:
	
	virtual void DoReloadShader();
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectCompositeFilter

enum EApplyFilterStage
{
	eBeforeProgramBind,
	eBeforeProgramDispatch,
	eAfterProgramDispatch
};

class ObjectCompositeFilter : public ObjectCompositeBase
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectCompositeFilter, ObjectCompositeBase)
	FBDeclareUserObject(ObjectCompositeFilter);

public:
	//! a constructor
	ObjectCompositeFilter(const char *pName = NULL, HIObject pObject=NULL, CProgramUniformsBase *effectProgramUniforms=nullptr);

	FBPropertyBaseEnum<ERenderLayer>		RenderLayer;

	// update when ogl context has changed
	virtual void ChangeContext() override;
	virtual void ChangeGlobalSettings() override;

	// render to buffers with render tree
	virtual bool ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo) override;
	virtual bool ApplyFilter(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId);

protected:

	CompositeComputeShader::CMixedProgram		*mProgram;
	std::auto_ptr<CProgramUniformsBase>			mProgramUniforms;

	virtual const char *MixedProgramLabel() {
		return "";
	}
	virtual const char *MixedProgramPath() {
		return "";
	}
	// do we need to bind multisample textures instead of resolved one
	virtual bool HasMultiSamplingSupport() {
		return false;
	}
	/*
	virtual CProgramUniformsBase *CreateProgramUniforms() {
		return new CProgramUniformsBase();
	}
	*/

	virtual void OnSetProgramExtension( FBString &strHeader )
	{}
	virtual void OnSetProgramDefines( const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, FBString &defineLine )
	{}

	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId)
	{}

};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectCompositeLayer

enum ETextureWrapMode
{
	eTextureWrapClampToZero,
	eTextureWrapClampToEdge,
	eTextureWrapMirroredRepeat,
	eTextureWrapRepeat
};

// TODO: add input source dimentions (input image size)

class ObjectCompositeLayer : public ObjectCompositeBase
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectCompositeLayer, ObjectCompositeBase)
	FBDeclareUserObject(ObjectCompositeLayer);

public:
	//! a constructor
	ObjectCompositeLayer(const char *pName = NULL, HIObject pObject=NULL);

public:

	FBPropertyBaseEnum<ECompositeBlendType>			BlendMode;		//!< type of blending for this layer
	FBPropertyBaseEnum<ETextureWrapMode>			TextureWrapMode;

	// layer is transformable in 2d space

	FBPropertyAnimatableVector2d					Translation;
	FBPropertyAnimatableDouble						Rotation;
	FBPropertyAnimatableDouble						UniformScaling;
	FBPropertyAnimatableVector2d					Scaling;


	// transformation pivot point offset
	FBPropertyAnimatableVector2d					PivotOffset;

	FBPropertyInt									NumberOfPasses;

	FBPropertyAction								TransformInit;
	FBPropertyAction								TransformFitImageSize;

	static void SetTransformInit(HIObject object, bool value);
	static void SetTransformFitImageSize(HIObject object, bool value);

public:

	virtual void DoTransformInit()
	{
		Translation = FBVector2d(0.0, 0.0);
		Rotation = 0.0;
		UniformScaling = 100.0;
		Scaling = FBVector2d(100.0, 100.0);
		PivotOffset = FBVector2d(50.0, 50.0);
	}
	virtual void DoTransformFitImageSize()
	{
		// TODO: override and implement function in render and solid color
	}

	// update when ogl context has changed
	virtual void ChangeContext() override;
	virtual void ChangeGlobalSettings() override;

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;


	// 1 - render render texture id or solid color texture id
	virtual const GLuint	ComputeLayerTexture(const CCompositionInfo *pInfo, CompositeFinalStats &stats)
	{
		return 0;
	}

	// 3 - 
	virtual const GLuint	GetLayerTextureId()
	{
		return mLastLayerTextureId;
	}

	virtual bool ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo);
	bool BlendLayers(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint operandAId, const GLuint resultId);

	const int GetNumberOfFilters(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, bool checkFilterReadyState);
	bool ProcessFilters(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats);

protected:

	GLuint					mLastLayerTextureId;


	struct ProgramUniformsBlend : public CProgramUniformsBase
	{
	protected:
		// locations

		GLint		translation;
		GLint		rotation;
		GLint		scaling;
		GLint		pivotOffset;

		GLint		clampToZero;
		GLint		numberOfPasses;

	public:
		// a constructor
		ProgramUniformsBlend()
			: CProgramUniformsBase()
		{
			translation = -1;
			rotation = -1;
			scaling = -1;
			pivotOffset = -1;
			clampToZero = -1;
			numberOfPasses = -1;
		}

		void Prep(const GLuint programId) override
		{
			CProgramUniformsBase::Prep(programId);

			translation = glGetUniformLocation(programId, "translation" );
			rotation = glGetUniformLocation(programId, "rotation" );
			scaling = glGetUniformLocation(programId, "scaling" );
			pivotOffset = glGetUniformLocation(programId, "pivotOffset" );
			clampToZero = glGetUniformLocation(programId, "clampToZero" );
			numberOfPasses = glGetUniformLocation(programId, "numberOfPasses" );
		}

		void UploadTransform(const bool _clampToZero, const double *tr, const double rot, 
			const double uniformScl, const double *scl, const double *pivot, const int _numberOfPasses)
		{
			if (translation >= 0)
			{
				glUniform2f(translation, 0.01f * (float)tr[0], 0.01f * (float)tr[1] ); 
			}
			if (rotation >= 0)
			{
				glUniform1f(rotation, nv_to_rad * (float) rot );
			}
			if (scaling >= 0)
			{
				double uniform = uniformScl;
				FBVector2d v( (double*) scl);

				if (uniform != 0.0) uniform = 100.0 / uniform;
				if (v[0] != 0.0) v[0] = uniform * 100.0 / v[0];
				if (v[1] != 0.0) v[1] = uniform * 100.0 / v[1];

				glUniform2f(scaling, (float)v[0], (float)v[1] ); 
			}
			if (pivotOffset >= 0)
			{
				glUniform2f(pivotOffset, 0.01f * (float)pivot[0], 0.01f * (float)pivot[1] );
			}
			if (clampToZero >= 0)
			{
				glUniform1f(clampToZero, (_clampToZero) ? 1.0f : 0.0f);
			}
			if (numberOfPasses >= 0)
			{
				glUniform1i(numberOfPasses, _numberOfPasses);
			}
		}
	};

	int mComputeLocalX;
	int mComputeLocalY;

	// apply local filters, each filter will process according to the input layer size

	CompositeComputeShader::CComputeTextureBuffer<2>	mLayerFilterBuffer;

	// blend layers with other layers

	CompositeComputeShader::CMixedProgram	*mBlendProgram;
	ProgramUniformsBlend					mBlendUniforms;

	
};


#endif /* __OBJECT_SHAPE_H__ */
