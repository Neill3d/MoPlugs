
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_object.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "compositemaster_object.h"
#include "compositemaster_common.h"
#include "compositeMaster_shaders.h"
#include "graphics\fpTexture.h"
#include "graphics\CheckGLError_MOBU.h"
#include "IO\FileUtils.h"
#include "shared_misc.h"

//
#define BLEND_COMPUTE_SHADER		"\\GLSL_CS\\blend.cs"


/** Class implementation.
*   This should be placed in the source code file for a class that derives
*   from FBComponent.
*   It is absolutely necessary in order to identify the class type.
*/

FBClassImplementation( ObjectCompositeMarker );								                //!< Register class
FBStorableCustomModelImplementation( ObjectCompositeMarker, COMPOSITEMARKER__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementationCompositionModel( ObjectCompositeMarker, COMPOSITEMARKER__DESCSTR, FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system


FBClassImplementation2(ObjectCompositeBase)
FBUserObjectImplement(ObjectCompositeBase, "Composite Base", FB_DEFAULT_SDK_ICON);			//Register UserObject class
FBElementClassImplementation2(ObjectCompositeBase, "Base", FB_DEFAULT_SDK_ICON);			//Register to the asset system

FBClassImplementation2(ObjectCompositeFilter)
FBUserObjectImplement(ObjectCompositeFilter, "Composite Base Filter", FB_DEFAULT_SDK_ICON);			//Register UserObject class
FBElementClassImplementation2(ObjectCompositeFilter, "Base Filter", FB_DEFAULT_SDK_ICON);			//Register to the asset system

FBClassImplementation2(ObjectCompositeLayer)
FBUserObjectImplement(ObjectCompositeLayer, "Composite Base Layer", FB_DEFAULT_SDK_ICON);			//Register UserObject class
FBElementClassImplementation2(ObjectCompositeLayer, "Base Layer", FB_DEFAULT_SDK_ICON);			//Register to the asset system


////////////////////////////////////////////////////////////////////////////////////////////////
// Object Composite Marker

ObjectCompositeMarker::ObjectCompositeMarker( const char* pName, HIObject pObject ) 
    : FBModelMarker( pName, pObject )
{
    FBClassInit;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Object Composite Base

const char * FBPropertyBaseEnum<ECompositeMask>::mStrings[] = {
	"Mask A",
	"Mask B",
	"Mask C",
	"Mask D",
	nullptr
};

const char * FBPropertyBaseEnum<ETextureWrapMode>::mStrings[] = {
	"Clamp To Zero",
	"Clamp To Edge",
	"Mirrored Repeat",
	"Repeat",
	nullptr
};

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/

int ObjectCompositeBase::GetOutWidth(HIObject object)
{
	ObjectCompositeBase *pBase = FBCast<ObjectCompositeBase>(object);
	if (pBase) 
	{
		return (int) pBase->mProcessingWidth;
	}
	return 0;
}
int ObjectCompositeBase::GetOutHeight(HIObject object)
{
	ObjectCompositeBase *pBase = FBCast<ObjectCompositeBase>(object);
	if (pBase) 
	{
		return (int) pBase->mProcessingHeight;
	}
	return 0;
}

int ObjectCompositeBase::GetTextureOGLId(HIObject object)
{
	ObjectCompositeBase *pBase = FBCast<ObjectCompositeBase>(object);
	if (pBase) 
	{
		return (int) pBase->GetColorTextureId();
	}
	return 0;
}

const char *ObjectCompositeBase::GetAssetName(HIObject object)
{
	ObjectCompositeBase *pBase = FBCast<ObjectCompositeBase>(object);
	if (pBase) 
	{
		return pBase->AssetNameString();
	}
	return 0;
}


void ObjectCompositeBase::SetReLoadShader(HIObject object, bool value)
{
	ObjectCompositeBase *pBase = FBCast<ObjectCompositeBase>(object);
	if (pBase && value) 
	{
		pBase->DoReloadShader();
	}
}

ObjectCompositeBase::ObjectCompositeBase(const char *pName, HIObject pObject)
		: FBUserObject( pName, pObject )
{
	FBClassInit;
	Deleted = false;

	FBPropertyPublish(this, Active, "Active", nullptr, nullptr);

#ifdef _DEBUG
	FBPropertyPublish(this, ReLoadShader, "Reload shader", nullptr, SetReLoadShader);
#endif

	//FBPropertyPublish(this, Mode, "Blend Mode", nullptr, nullptr);
	FBPropertyPublish(this, Opacity, "Opacity", nullptr, nullptr);

	FBPropertyPublish(this, UseCompositeMask, "Use Composite Mask", nullptr, nullptr);
	FBPropertyPublish(this, SelectCompositeMask, "Select Composite Mask", nullptr, nullptr);
	FBPropertyPublish(this, InvertCompositeMask, "Invert Composite Mask", nullptr, nullptr);
	FBPropertyPublish(this, CustomMask, "Custom Mask", nullptr, nullptr);

	// processing dimentions
	FBPropertyPublish(this, OutWidth, "Out Width", GetOutWidth, nullptr );
	FBPropertyPublish(this, OutHeight, "Out Height", GetOutHeight, nullptr );

	// can be used for preview
	FBPropertyPublish(this, TextureOGLId, "TextureOGLId", GetTextureOGLId, nullptr);
	FBPropertyPublish(this, AssetName, "Asset Name", GetAssetName, nullptr);

	Active = true;

	Opacity = 100.0;
	Opacity.SetMinMax(0.0, 100.0);

	UseCompositeMask = false;
	SelectCompositeMask = eCompositeMaskA;
	InvertCompositeMask = false;

	CustomMask.SetSingleConnect(true);
	CustomMask.SetFilter( FBTexture::GetInternalClassId() );

	OutWidth.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	OutHeight.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	TextureOGLId.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
	TextureOGLId.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	AssetName.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );
	AssetName.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	mRemoveChildren = true;

	//
//	mRoot = nullptr;
	//mBuffer = nullptr;

	//
	/*
	mRenderX = 0;
	mRenderY = 0;
	mRenderWidth = 1;
	mRenderHeight = 1;
	*/
	mCompositeMaskTextureId = 0;

	mProcessingWidth = 0;
	mProcessingHeight = 0;

	mNeedProgramReload = false;
}

void ObjectCompositeBase::FBDestroy()
{
	printf( "test destroing\n" );

	if (mRemoveChildren)
	{
		const int count = (int) mRemoveComponents.size();
		for (int i=0; i<count; ++i)
			mRemoveComponents[i]->FBDelete();
	}

	ParentClass::FBDestroy();

	//FreeFrameBufferBase();
}

void ObjectCompositeBase::FBDelete()		
{ 
	mRemoveChildren = true;

	const int count = Components.GetCount();
	mRemoveComponents.resize(count);
	for (int i=0; i<count; ++i)
		mRemoveComponents[i] = Components[i];

	ParentClass::FBDelete(); 
}

void ObjectCompositeBase::FBDeleteSilence()		
{ 
	//for (int i=0; i<Components.GetCount(); ++i)
	//	Components[i]->FBDelete();
	mRemoveChildren = false;
	ParentClass::FBDelete(); 
}
/*
bool ObjectCompositeBase::InitFrameBufferBase(const int width, const int height)
{
	return InitFrameBuffer(width, height, mBuffer);
}

void ObjectCompositeBase::FreeFrameBufferBase()
{
	FreeFrameBuffer(mBuffer);
}
*/
bool ObjectCompositeBase::InitFrameBuffer(const int width, const int height, FrameBuffer *& pBuffer)
{
	if (pBuffer == nullptr)
	{
		pBuffer = new FrameBuffer(0, 0);
		pBuffer->ReSize(width, height);
		return pBuffer->IsOk();
	}
	else
	{
		pBuffer->ReSize(width, height);
	}

	return (pBuffer != nullptr);
}

void ObjectCompositeBase::FreeFrameBuffer(FrameBuffer *& pBuffer)
{
	if (pBuffer)
	{
		delete pBuffer;
		pBuffer = nullptr;
	}
}

void ObjectCompositeBase::ChangeContext()
{
	const int count = GetSrcCount();
	for (int i=0; i<count; ++i)
	{
		FBComponent *pComp = (FBComponent*) GetSrc(i);

		if ( FBIS(pComp, ObjectCompositeBase) )
			((ObjectCompositeBase*) pComp)->ChangeContext();
	}
}

void ObjectCompositeBase::ChangeGlobalSettings()
{
	const int count = GetSrcCount();
	for (int i=0; i<count; ++i)
	{
		FBComponent *pComp = (FBComponent*) GetSrc(i);

		if ( FBIS(pComp, ObjectCompositeBase) )
			((ObjectCompositeBase*) pComp)->ChangeGlobalSettings();
	}
}

/*
void ObjectCompositeBase::ReSize(const int x, const int y, const int w, const int h)
{
	const int count = GetSrcCount();
	for (int i=0; i<count; ++i)
	{
		FBComponent *pComp = (FBComponent*) GetSrc(i);

		if ( FBIS(pComp, ObjectCompositeBase) )
			((ObjectCompositeBase*) pComp)->ReSize(x, y, w, h);
	}
}

const GLuint	ObjectCompositeBase::getColorTextureId()
{
	
	if (Active && mBuffer)
	{
		return mBuffer->GetColorObject();
	}
	else if (Components.GetCount() > 0)
	{
		return ( (ObjectCompositeBase*) Components[0] )->getColorTextureId();
	}
	
	return 0;
}
*/


void ObjectCompositeBase::DoReloadShader()
{
	mNeedProgramReload = true;
}

void ObjectCompositeBase::BindCompositeMask()
{
	const GLuint maskId = GetCompositeMaskTextureId();
	if (maskId > 0)
	{
		GLint param[4] = {GL_RED, GL_RED, GL_RED, GL_RED};

		switch(SelectCompositeMask)
		{
		case eCompositeMaskA:
			param[0] = param[1] = param[2] = param[3] = GL_RED;
			break;
		case eCompositeMaskB:
			param[0] = param[1] = param[2] = param[3] = GL_GREEN;
			break;
		case eCompositeMaskC:
			param[0] = param[1] = param[2] = param[3] = GL_BLUE;
			break;
		case eCompositeMaskD:
			param[0] = param[1] = param[2] = param[3] = GL_ALPHA;
			break;
		}

		// NOTE: mask texture has a default bind slot 1
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, maskId);
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, param);
		glActiveTexture(GL_TEXTURE0);
	}
}
void ObjectCompositeBase::UnBindCompositeMask()
{
	const GLuint maskId = GetCompositeMaskTextureId();
	if (maskId > 0)
	{
		glActiveTexture(GL_TEXTURE7);

		glBindTexture(GL_TEXTURE_2D, maskId);
		GLint param[4] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, param);

		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);	
	}
}

void ObjectCompositeBase::SetCompositeMaskTextureId(const GLuint texid)
{
	mCompositeMaskTextureId = texid;
}
const GLuint ObjectCompositeBase::GetCompositeMaskTextureId() 
{
	if (CustomMask.GetCount() > 0)
	{
		FBTexture *pTexture = (FBTexture*) CustomMask[0];
		FBVideo *pVideo = pTexture->Video;

		unsigned int texId = 0;

		if ( FBIS(pVideo, FBVideoMemory) )
		{
			texId = ((FBVideoMemory*) pVideo)->TextureOGLId;
		}
		else
		{
			glActiveTexture(GL_TEXTURE0);
			pTexture->OGLInit();
			texId = pTexture->GetTextureObject();
		}
		return (GLuint) texId;
	}
	else
	return mCompositeMaskTextureId;
}

bool ObjectCompositeBase::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if (pAction == kFBCandidated && (pThis == &UseCompositeMask) )
	{
		ChangeGlobalSettings();
		return true;
	}
	
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}
bool ObjectCompositeBase::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	if (pThis == &CustomMask)
	{
		if (pAction == kFBConnectedSrc)
		{
			ChangeGlobalSettings();
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			ChangeGlobalSettings();
		}
	}
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Composite Base Effect

ObjectCompositeFilter::ObjectCompositeFilter(const char *pName, HIObject pObject, CProgramUniformsBase *effectProgramUniforms)
	: ObjectCompositeBase( pName, pObject )
	, mProgramUniforms( effectProgramUniforms )
{
	FBClassInit;

	mProgram = nullptr;
	
	//mProgramUniforms.reset( this->CreateProgramUniforms() );

	FBPropertyPublish( this, RenderLayer, "Render Layer", nullptr, nullptr );
	
	RenderLayer = eRenderLayerMain;

	mProcessingWidth = 0;
	mProcessingHeight = 0;
}


bool ObjectCompositeFilter::ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo)
{
	if (false == Active)
		return false;

	if (0.0 == Opacity)
		return false;

	//const int w = prInfo.GetWidth();
	//const int h = prInfo.GetHeight();

	if (mNeedProgramReload && nullptr != mProgram)
	{
		CompositeComputeShader::CMixedProgramManager::instance().ReloadProgram(mProgram);
		mNeedProgramReload = false;

		bool shaderOk = mProgram->IsOk();
		if (false == shaderOk)
		{
			Active = false;
			return false;
		}

		mProgramUniforms->Prep( mProgram->GetProgramId() );
	}

	const bool useMask = UseCompositeMask;
	if (nullptr == mProgram)
	{
		FBString strHeader( "#version 430\n"
			"#extension GL_ARB_shading_language_include : require\n" );
		OnSetProgramExtension(strHeader);
		strHeader += "layout (local_size_x =32, local_size_y = 32) in;\n";

		FBString strDefine( "" );

		// TODO: check when to apply same size difinition
		strDefine = strDefine + "#define SAME_SIZE\n";

		if (useMask)
			strDefine = strDefine + "#define USE_MASK\n";

		if ( HasMultiSamplingSupport() )
		{
			strDefine = strDefine + ((CCompositionInfo*) pInfo)->GetRenderSamplesDefineString();
		}
		strDefine = strDefine + pInfo->GetDepthBufferDefineString();

		OnSetProgramDefines( pInfo, prInfo, strDefine );

		const char *programLabel = MixedProgramLabel();
		const char *programPath = MixedProgramPath();

		mProgram = CompositeComputeShader::CMixedProgramManager::instance().QueryAProgramMix(programLabel, strHeader, strDefine, programPath, "");
		
		bool shaderOk = mProgram->IsOk();
		if (false == shaderOk)
		{
			Active = false;
			return false;
		}

		// DONE: update uniform locations
		mProgramUniforms->Prep( mProgram->GetProgramId() );
	}

	if (nullptr == mProgram)
		return false;

	return true;
}

bool ObjectCompositeFilter::ApplyFilter(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId)
{
	const int w = prInfo.GetWidth();
	const int h = prInfo.GetHeight();

	mProcessingWidth = w;
	mProcessingHeight = h;

	const bool useMask = UseCompositeMask;

	const int computeLocalX = 32;
	const int computeLocalY = 32;
	const int groups_x = w / computeLocalX + 1;
	const int groups_y = h / computeLocalY + 1;

	OnApplyFilter(eBeforeProgramBind, pInfo, prInfo, stats, sourceTexId, dstTexId);

	mProgram->Bind();

	// update uniform values
	mProgramUniforms->Upload(pInfo, prInfo,
		mProcessingWidth, mProcessingHeight,
		InvertCompositeMask.AsInt() > 0, 0.01f * (float)Opacity);
	
	OnApplyFilter(eBeforeProgramDispatch, pInfo, prInfo, stats, sourceTexId, dstTexId);
	
	const ERenderLayer renderLayerId = RenderLayer;
	const MainFrameBuffer *pFrameBuffer = pInfo->GetFrameBufferInfo(renderLayerId);

	if (useMask)
	{
		SetCompositeMaskTextureId( pFrameBuffer->GetFinalMaskObject() );
		BindCompositeMask();
	}
	
	glBindImageTexture(0, dstTexId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	
	GLenum target = GL_TEXTURE_2D;
	GLuint depthId = pFrameBuffer->GetFinalDepthObject(); // pInfo->GetRenderDepthId();
	GLuint normalId = pFrameBuffer->GetFinalNormalObject(); // pInfo->GetRenderNormalId();
	GLuint positionId = pFrameBuffer->GetFinalPositionObject(); // pInfo->GetRenderPositionId();

	if (pInfo->IsRenderMultisampling() && HasMultiSamplingSupport() )
	{
		target = GL_TEXTURE_2D_MULTISAMPLE;
		depthId = pFrameBuffer->GetDepthObjectMS(); // pInfo->GetRenderDepthMSId();
		normalId = pFrameBuffer->GetNormalObjectMS(); // pInfo->GetRenderNormalMSId();
		positionId = pFrameBuffer->GetPositionObjectMS();
	}

	if (sourceTexId > 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, sourceTexId);
	}
	if (mProgramUniforms->IsDepthSamplerUsed() )
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(target, depthId );
	}
	if (mProgramUniforms->IsNormalSamplerUsed() )
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(target, normalId );
	}
	if (mProgramUniforms->IsPositionSamplerUsed() )
	{
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(target, positionId );
	}

	// DONE: compute the number of groups !!
	mProgram->DispatchPipeline(groups_x, groups_y, 1);

	//
	stats.CountComputeShader(1, groups_x*groups_y );
	

	mProgram->UnBind();

	OnApplyFilter(eAfterProgramDispatch, pInfo, prInfo, stats, sourceTexId, dstTexId);

	if (useMask)
	{
		UnBindCompositeMask();
	}

	glActiveTexture(GL_TEXTURE0);

	CHECK_GL_ERROR_MOBU();

	return true;
}

void ObjectCompositeFilter::ChangeContext()
{
	mProgram = nullptr;
}

void ObjectCompositeFilter::ChangeGlobalSettings()
{
	mProgram = nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Composite Base Layer

void ObjectCompositeLayer::SetTransformInit(HIObject object, bool value)
{
	ObjectCompositeLayer *pBase = FBCast<ObjectCompositeLayer>(object);
	if (pBase && value) 
	{
		pBase->DoTransformInit();
	}
}

void ObjectCompositeLayer::SetTransformFitImageSize(HIObject object, bool value)
{
	ObjectCompositeLayer *pBase = FBCast<ObjectCompositeLayer>(object);
	if (pBase && value) 
	{
		pBase->DoTransformFitImageSize();
	}
}

ObjectCompositeLayer::ObjectCompositeLayer(const char *pName, HIObject pObject)
		: ObjectCompositeBase( pName, pObject )
{
	FBClassInit;
	
	FBPropertyPublish(this, BlendMode, "Blend Mode", nullptr, nullptr);
	FBPropertyPublish(this, TextureWrapMode, "Texture Wrap Mode", nullptr, nullptr);

	FBPropertyPublish(this, Translation, "Translation", nullptr, nullptr);
	FBPropertyPublish(this, Rotation, "Rotation", nullptr, nullptr);
	FBPropertyPublish(this, UniformScaling, "Uniform Scaling", nullptr, nullptr);
	FBPropertyPublish(this, Scaling, "Scaling", nullptr, nullptr);

	FBPropertyPublish(this, PivotOffset, "Pivot Offset", nullptr, nullptr);
	FBPropertyPublish(this, NumberOfPasses, "Number Of Passes", nullptr, nullptr);

	FBPropertyPublish(this, TransformInit, "Transform Init", nullptr, SetTransformInit);
	FBPropertyPublish(this, TransformFitImageSize, "Fit Image Size", nullptr, SetTransformFitImageSize);

	BlendMode = eCompositeBlendNormal;
	TextureWrapMode = eTextureWrapClampToZero;

	Rotation.SetMinMax(0.0, 360.0);
	UniformScaling = 100.0;
	Scaling = FBVector2d(100.0, 100.0);
	PivotOffset = FBVector2d(50.0, 50.0);
	NumberOfPasses = 0;
	NumberOfPasses.SetMinMax(0.0, 10.0, true, true);

	mBlendProgram = nullptr;

	mComputeLocalX = 32;
	mComputeLocalY = 32;
}

void ObjectCompositeLayer::ChangeContext()
{
	ParentClass::ChangeContext();

	mBlendProgram = nullptr;
}

const int ObjectCompositeLayer::GetNumberOfFilters(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, bool checkFilterReadyState)
{
	int totalCount = 0;

	const int count = GetSrcCount();
	for (int i=0; i<count; ++i)
	{
		if (FBIS(GetSrc(i), ObjectCompositeFilter) )
		{
			ObjectCompositeFilter *pFilter = (ObjectCompositeFilter*) GetSrc(i);

			if (nullptr != pInfo && true == checkFilterReadyState)
			{
				if (false == pFilter->ReadyToApply(pInfo, prInfo) )
					continue;
			}

			totalCount = totalCount + 1;
		}
	}

	return totalCount;
}

bool ObjectCompositeLayer::ProcessFilters(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats)
{
	const int inputW = prInfo.GetWidth();
	const int inputH = prInfo.GetHeight();

	int totalCount = 0;

	// start with the source texture
	GLuint sourceId = GetLayerTextureId();

	// TODO: input dimentions (we should use source dimentions here !! )
	mLayerFilterBuffer.ReSizeLayersData(inputW, inputH);
	stats.CountTextures( 2, inputW * inputH * 4 );

	const int count = GetSrcCount();
	for (int i=0; i<count; ++i)
	{
		if (FBIS(GetSrc(i), ObjectCompositeFilter) )
		{
			ObjectCompositeFilter *pFilter = (ObjectCompositeFilter*) GetSrc(i);

			if ( pFilter->ReadyToApply(pInfo, prInfo) )
			{
				GLuint dstId = mLayerFilterBuffer.QueryATextureBuffer();

				pFilter->ApplyFilter(pInfo, prInfo, stats, sourceId, dstId );
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				sourceId = dstId;
				totalCount = totalCount + 1;
			}
		}
	}

	mLastLayerTextureId = sourceId;
	return (totalCount > 0);
}

bool ObjectCompositeLayer::ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo)
{
	if (Active.AsInt() == 0)
		return false;
	if (0.0 == Opacity)
		return false;

	if (mNeedProgramReload && nullptr != mBlendProgram)
	{
		CompositeComputeShader::CMixedProgramManager::instance().ReloadProgram(mBlendProgram);
		mNeedProgramReload = false;

		bool shaderOk = mBlendProgram->IsOk();
		if (false == shaderOk)
		{
			Active = false;
			return false;
		}

		mBlendUniforms.Prep( mBlendProgram->GetProgramId() );
	}

	ECompositeBlendType currMode = BlendMode;
	bool useMask = UseCompositeMask;

	if (nullptr == mBlendProgram)
	{
		FBString strHeader("#version 430\n"
			"#extension GL_ARB_shading_language_include : require\n"
			"layout(local_size_x=32, local_size_y=32) in;\n" );

		FBString strBlendMode = CompositeBlendTypeToString(currMode);
		FBString strDefine( strBlendMode );
		
		if (useMask)
			strDefine = strDefine + "#define USE_MASK\n";

		mBlendProgram = CompositeComputeShader::CMixedProgramManager::instance().QueryAProgramMix(
			"blend",
			strHeader,
			strDefine, 
			BLEND_COMPUTE_SHADER, 
			"");
		
		if (true == mNeedProgramReload && nullptr != mBlendProgram)
		{
			CompositeComputeShader::CMixedProgramManager::instance().ReloadProgram(mBlendProgram);
			mNeedProgramReload = false;
		}

		// DONE: update uniform locations
		if (mBlendProgram->IsOk() )
		{
			mBlendUniforms.Prep(mBlendProgram->GetProgramId() );
		}
		else
		{
			mBlendProgram = nullptr;
			Active = false;
			return false;
		}
	}

	if (mBlendProgram == nullptr)
		return false;

	return true;
}

bool ObjectCompositeLayer::BlendLayers(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint srcLayerId, const GLuint resultId)
{
	bool useMask = UseCompositeMask;


	const int w = prInfo.GetWidth();
	const int h = prInfo.GetHeight();

	mBlendProgram->Bind();

	// update uniform values
	mBlendUniforms.Upload(pInfo, prInfo,
		mProcessingWidth, mProcessingHeight, InvertCompositeMask, 0.01f * Opacity);

	FBVector2d tr = Translation;
	double rot = Rotation;
	double uniform = UniformScaling;
	FBVector2d scl = Scaling;
	FBVector2d pivot = PivotOffset;
	const int numberOfPasses = NumberOfPasses.AsInt();

	mBlendUniforms.UploadTransform( (TextureWrapMode==eTextureWrapClampToZero), tr, rot, uniform, scl, pivot, numberOfPasses);

	glBindImageTexture(0, resultId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	
	if (srcLayerId > 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, srcLayerId);
	}
	const GLuint dstLayerId = GetLayerTextureId();
	if (dstLayerId > 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, dstLayerId);

		const ETextureWrapMode wrapMode = TextureWrapMode;
		switch(wrapMode)
		{
		case eTextureWrapClampToEdge:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		case eTextureWrapMirroredRepeat:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			break;
		case eTextureWrapRepeat:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		}
	}
	
	if (useMask)
	{
		const MainFrameBuffer *pFrameBuffer = pInfo->GetFrameBufferInfo(eRenderLayerMain);

		SetCompositeMaskTextureId(  pFrameBuffer->GetFinalMaskObject() );
		BindCompositeMask();
	}

	glActiveTexture(GL_TEXTURE0);

	// DONE: compute the number of groups !!
	const int groups_x = w / mComputeLocalX + 1;
	const int groups_y = h / mComputeLocalY + 1;
	
	mBlendProgram->DispatchPipeline(groups_x, groups_y, 1);
	stats.CountComputeShader(1, groups_x * groups_y );

	mBlendProgram->UnBind();

	if (useMask)
		UnBindCompositeMask();

	return true;
}

bool ObjectCompositeLayer::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if (pAction == kFBCandidated && (pThis == &BlendMode) )
	{
		ChangeGlobalSettings();
	}
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}
bool ObjectCompositeLayer::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void ObjectCompositeLayer::ChangeGlobalSettings()
{
	ParentClass::ChangeGlobalSettings();
	mBlendProgram = nullptr;
}
