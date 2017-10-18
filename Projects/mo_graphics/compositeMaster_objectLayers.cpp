
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectLayers.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_objectLayers.h"
#include "compositeMaster_objectFinal.h"
#include "model_3d_shadow.h"
#include "shared_misc_MOBU.h"

FBClassImplementation2(ObjectCompositionRender)
FBUserObjectImplement(ObjectCompositionRender, "Composition Render", SOURCE_ICON);				//Register UserObject class
FBElementClassImplementationLayer(ObjectCompositionRender, COMPOSITERENDER__ASSETNAME, SOURCE_ICON);				//Register to the asset system

FBClassImplementation2(ObjectCompositionColor)
FBUserObjectImplement(ObjectCompositionColor, "Composition Color", SOURCE_ICON);		//Register UserObject class
FBElementClassImplementationLayer(ObjectCompositionColor, COMPOSITESOLIDCOLOR__ASSETNAME,  SOURCE_ICON);		//Register to the asset system

FBClassImplementation2(ObjectCompositionShadow)
FBUserObjectImplement(ObjectCompositionShadow, "Composition Shadow", SOURCE_ICON);		//Register UserObject class
FBElementClassImplementationLayer(ObjectCompositionShadow, COMPOSITESHADOW__ASSETNAME,  SOURCE_ICON);		//Register to the asset system


const char * FBPropertyBaseEnum<ECompositeRenderOutput>::mStrings[] = {
	"Color",
	"Depth",
	"Normals",
	"Mask A",
	"Mask B",
	"Mask C",
	"Mask D",
	"Custom Model",
	nullptr
};


/************************************************
 *	Constructor.
 ************************************************/

void AddPropertyViewForRender(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(COMPOSITERENDER__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectCompositionRender::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForRender( "", "Transformation", true );
	AddPropertyViewForRender( "Translation", "Transformation" );
	AddPropertyViewForRender( "Rotation", "Transformation" );
	AddPropertyViewForRender( "Uniform Scaling", "Transformation" );
	AddPropertyViewForRender( "Scaling", "Transformation" );
	AddPropertyViewForRender( "Pivot Offset", "Transformation" );
	AddPropertyViewForRender( "Transform Init", "Transformation" );
	AddPropertyViewForRender( "Fit Image Size", "Transformation" );

	AddPropertyViewForRender( "", "Masking", true );
	AddPropertyViewForRender( "Use Composite Mask", "Masking" );
	AddPropertyViewForRender( "Select Composite Mask", "Masking" );
	AddPropertyViewForRender( "Invert Composite Mask", "Masking" );
	AddPropertyViewForRender( "Custom Mask", "Masking" );
}

ObjectCompositionRender::ObjectCompositionRender( const char* pName, HIObject pObject ) 
	: ObjectCompositeLayer( pName, pObject )
{
	FBClassInit;

	FBPropertyPublish(this, RenderLayer, "Render Layer", nullptr, nullptr);
	FBPropertyPublish(this, WhatOutput, "Output", nullptr, nullptr);

	FBPropertyPublish(this, CustomModel, "Custom Model", nullptr, nullptr);

	FBPropertyPublish(this, BackgroundColor, "Background Color", nullptr, nullptr);
	FBPropertyPublish(this, BackgroundAlpha, "Background Alpha", nullptr, nullptr);

	FBPropertyPublish(this, UseCustomColor, "Use Custom Color", nullptr, nullptr);
	FBPropertyPublish(this, CustomColor, "Custom Color", nullptr, nullptr);

	FBPropertyPublish(this, UseCustomTexture, "Use Custom Texture", nullptr, nullptr);
	FBPropertyPublish(this, CustomTexture, "Custom Texture", nullptr, nullptr);

	FBPropertyPublish(this, CompareWithMainDepth, "Compare With Main Depth", nullptr, nullptr);
	FBPropertyPublish(this, Bias, "Bias", nullptr, nullptr);

	FBPropertyPublish(this, OutputBufferScale, "Output Buffer Scale", nullptr, nullptr);

	//FBPropertyPublish(this, DepthOutput, "Depth output", nullptr, nullptr);
	//FBPropertyPublish(this, NormalOutput, "Normal output", nullptr, nullptr);
	/*
	FBPropertyPublish(this, ProcessNormal, "Process Normal", nullptr, nullptr);
	FBPropertyPublish(this, NormalX, "Normal X Color", nullptr, nullptr);
	FBPropertyPublish(this, NormalY, "Normal Y Color", nullptr, nullptr);
	FBPropertyPublish(this, NormalZ, "Normal Z Color", nullptr, nullptr);
	*/
	RenderLayer = eRenderLayerMain;
	WhatOutput = eCompositeRenderColor;

	CustomModel.SetFilter( FBModel::GetInternalClassId() );
	CustomModel.SetSingleConnect(false);

	BackgroundColor = FBColor(0.0, 0.0, 0.0);
	BackgroundAlpha = 0.0;

	UseCustomColor = true;
	CustomColor = FBColor(1.0, 1.0, 1.0);

	UseCustomTexture = false;
	CustomTexture.SetFilter( FBTexture::GetInternalClassId() );
	CustomTexture.SetSingleConnect( true );

	CompareWithMainDepth = true;
	Bias = 1.0; // * 0.001

	OutputBufferScale = 100.0;
	OutputBufferScale.SetMinMax(1.0, 200.0, true, true);

	//DepthOutput = false;
	//NormalOutput = false;
	/*
	ProcessNormal = false;

	NormalX = FBColor(1.0, 0.0, 0.0);
	NormalY = FBColor(0.0, 1.0, 0.0);
	NormalZ = FBColor(0.0, 0.0, 1.0);
	*/
	
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectCompositionRender::FBCreate()
{
	return true;
}

void ObjectCompositionRender::CopyFrom(ObjectCompositeBase *pNode)
{
	if ( FBIS(pNode, ObjectCompositionRender) )
	{
		ObjectCompositionRender *pRender = (ObjectCompositionRender*) pNode;

		//PropertyCopy( DepthOutput, pRender->DepthOutput );
		//PropertyCopy( NormalOutput, pRender->NormalOutput );
		//PropertyCopy( WhatOutput, pRender->WhatOutput );
	}
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
/*
void ObjectCompositionRender::FBDestroy()
{
	
}
*/
/*
bool ObjectCompositionRender::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ObjectCompositionRender::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if( pStoreWhat == kAttributes )
	{
	}
	return true;
}
*/



const GLuint ObjectCompositionRender::ComputeLayerTexture(const CCompositionInfo *pInfo, CompositeFinalStats &stats)
{
	const ERenderLayer layerId = RenderLayer;
	const MainFrameBuffer *pFrameBufferMain = pInfo->GetFrameBufferInfo(eRenderLayerMain);
	const MainFrameBuffer *pFrameBuffer = pInfo->GetFrameBufferInfo(layerId);

	if (nullptr == pFrameBufferMain || nullptr == pFrameBuffer)
		return 0;

	const GLuint mask_buffer_id = pFrameBufferMain->GetFinalMaskObject(); // pInfo->GetRenderMaskId();

	mProcessingWidth = pInfo->GetRenderWidth();
	mProcessingHeight = pInfo->GetRenderHeight();

	switch(WhatOutput)
	{
	case eCompositeRenderNormal:
		mLastLayerTextureId = pFrameBuffer->GetFinalNormalObject(); // pInfo->GetRenderNormalId();
		break;

	case eCompositeRenderDepth:
		mLastLayerTextureId = pFrameBuffer->GetFinalDepthObject(); // pInfo->GetRenderDepthId();
		break;

	case eCompositeRenderMaskA:
		{
			glBindTexture(GL_TEXTURE_2D, mask_buffer_id);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);

			glBindTexture(GL_TEXTURE_2D, 0);

			mLastLayerTextureId = mask_buffer_id;
		}
		break;
			
	case eCompositeRenderMaskB:
		{
			glBindTexture(GL_TEXTURE_2D, mask_buffer_id);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_GREEN);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_GREEN);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_GREEN);

			glBindTexture(GL_TEXTURE_2D, 0);

			mLastLayerTextureId = mask_buffer_id;
		}
		break;

	case eCompositeRenderMaskC:
		{
			glBindTexture(GL_TEXTURE_2D, mask_buffer_id);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_BLUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_BLUE);

			glBindTexture(GL_TEXTURE_2D, 0);

			mLastLayerTextureId = mask_buffer_id;
		}
		break;

	case eCompositeRenderMaskD:
		{
			glBindTexture(GL_TEXTURE_2D, mask_buffer_id);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ALPHA);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ALPHA);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ALPHA);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

			glBindTexture(GL_TEXTURE_2D, 0);

			mLastLayerTextureId = mask_buffer_id;
		}
		break;

	case eCompositeRenderCustomModel:
		// ! processing width and height could be changed !!
		RenderCustomModels(mProcessingWidth, mProcessingHeight, 
			pFrameBufferMain->GetFinalDepthObject(), ((CCompositionInfo*)pInfo)->GetRenderCamera() );
		
		if (mFramebuffer.get() )
			mLastLayerTextureId = mFramebuffer->GetColorObject();
		else
			mLastLayerTextureId = pFrameBuffer->GetFinalColorObject(); // pInfo->GetRenderColorId();
		break;

	default:
		mLastLayerTextureId = pFrameBuffer->GetFinalColorObject(); // pInfo->GetRenderColorId();
	}

	// apply local filters
	// TODO: use local processing info
	CProcessingInfo		processingInfo(mProcessingWidth, mProcessingHeight);
	ProcessFilters(pInfo, processingInfo, stats);

	return mLastLayerTextureId;
}

void ObjectCompositionRender::DoTransformFitImageSize()
{
}

bool ObjectCompositionRender::PrepResources(const int width, const int height)
{
	if ( nullptr == mFramebuffer.get() )
		mFramebuffer.reset( new FrameBuffer(1, 1, 
		FrameBuffer::eCreateColorTexture | FrameBuffer::eCreateDepthTexture | FrameBuffer::eDeleteFramebufferOnCleanup) );

	mFramebuffer->ReSize(width, height);

	if ( nullptr == mShaderDrawWithDepth.get() )
	{
		// try to load a shader
		FBString effectPath, effectFullName;
		if (true == FindEffectLocation( "\\GLSL\\drawWithDepthTest.vsh", effectPath, effectFullName ) )
		{
			GLSLShader *pNewShader = new GLSLShader();

			FBString vertexPath( effectPath, "\\GLSL\\drawWithDepthTest.vsh" );
			FBString fragmentPath( effectPath, "\\GLSL\\drawWithDepthTest.fsh" );

			if (false == pNewShader->LoadShaders( vertexPath, fragmentPath ) )
			{
				delete pNewShader;
				pNewShader = nullptr;
			}
			else
			{
				// load uniform locations

				mUniformLoc.screenSize = pNewShader->findLocation( "SCREEN_SIZE" );
				mUniformLoc.options = pNewShader->findLocation( "options" );
				mUniformLoc.materialColor = pNewShader->findLocation( "MATERIAL_COLOR" );
				mUniformLoc.useColorSampler = pNewShader->findLocation( "useColorSampler" );
				mUniformLoc.colorSampler = pNewShader->findLocation( "colorSampler" );
				mUniformLoc.depthSampler = pNewShader->findLocation( "depthSampler" );

				pNewShader->Bind();
				if (mUniformLoc.colorSampler >= 0)
					pNewShader->setUniformUINT(mUniformLoc.colorSampler, 0);
				if (mUniformLoc.depthSampler >= 0)
					pNewShader->setUniformUINT(mUniformLoc.depthSampler, 1);
				pNewShader->UnBind();
			}

			mShaderDrawWithDepth.reset(pNewShader);
		}
	}

	return ( nullptr != mFramebuffer.get() && nullptr != mShaderDrawWithDepth.get() );
}

bool ObjectCompositionRender::RenderCustomModels(int &width, int &height, GLuint depthId, FBCamera *pCamera)
{
	if (nullptr == pCamera || depthId == 0 || width < 1 || height < 1)
		return false;


	// check for scaledown factor
	double scaleFactor=1.0;
	OutputBufferScale.GetData(&scaleFactor, sizeof(double));
	scaleFactor *= 0.01;
	if (scaleFactor <= 0.0) scaleFactor = 1.0;
	if (scaleFactor > 2.0) scaleFactor = 2.0;

	if (scaleFactor != 1.0)
	{
		double fwidth = scaleFactor * (double) width;
		double fheight = scaleFactor * (double) height;

		width = (int) fwidth;
		height = (int) fheight;
	}

	if (false == PrepResources(width, height) )
		return false;

	//
	// check if we have anything to draw

	bool readyToRender = (CustomModel.GetCount() > 0);

	for (int i=0, count=CustomModel.GetCount(); i<count; ++i)
	{
		FBModel *pModel = (FBModel*) CustomModel.GetAt(i);
		FBModelVertexData *pVertexData = pModel->ModelVertexData;

		if (nullptr == pVertexData || false == pVertexData->IsDrawable() )
		{
			readyToRender = false;
			break;
		}
	}

	if (false == readyToRender)
		return false;

	

	//
	//

	FBMatrix mv, mp;
	pCamera->GetCameraMatrix(mv, kFBModelView);
	pCamera->GetCameraMatrix(mp, kFBProjection);

	double nearPlane = pCamera->NearPlaneDistance;
	double farPlane = pCamera->FarPlaneDistance;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(mp);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(mv);

	//
	//
	//GLint lViewport[4];
	//glGetIntegerv(GL_VIEWPORT, lViewport);

	mFramebuffer->Bind();

	glEnable(GL_DEPTH_TEST);

	FBColor color;
	double alpha;

	const bool useCustomColor = UseCustomColor;
	const bool bindColor = UseCustomTexture;
	const float compareWithDepth = (CompareWithMainDepth) ? 1.0f : 0.0f;
	const double bias = 0.001 * Bias;

	BackgroundColor.GetData(color, sizeof(double)*3);
	BackgroundAlpha.GetData(&alpha, sizeof(double));
	alpha = 0.01 * alpha;

	glViewport(0, 0, width, height);

	glClearColor(color[0], color[1], color[2], alpha);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (true == useCustomColor)
	{
		CustomColor.GetData(color, sizeof(double)*3);
		glColor3dv(color);
	}

	mShaderDrawWithDepth->Bind();
	mShaderDrawWithDepth->setUniformVector( mUniformLoc.screenSize, 0.0f, 0.0f, 1.0f/(float)width, 1.0f/(float)height );
	mShaderDrawWithDepth->setUniformVector( mUniformLoc.options, (float) nearPlane, (float) farPlane, 
		compareWithDepth, (float) bias );
	mShaderDrawWithDepth->setUniformVector( mUniformLoc.materialColor, (float)color[0], 
		(float)color[1], (float)color[2], 1.0f );
	mShaderDrawWithDepth->setUniformFloat( mUniformLoc.useColorSampler, (bindColor) ? 1.0f : 0.0f );

	glActiveTexture(GL_TEXTURE1);
	glBindTexture( GL_TEXTURE_2D, depthId );
	glActiveTexture(GL_TEXTURE0);

	if (bindColor)
	{
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		if (CustomTexture.GetCount() > 0)
		{
			FBTexture *pTexture = (FBTexture*)CustomTexture.GetAt(0);
			GLuint id = pTexture->TextureOGLId;
			if (0 == id)
			{
				pTexture->OGLInit();
				id = pTexture->TextureOGLId;
			}

			glBindTexture(GL_TEXTURE_2D, id);
		}
	}

	for (int i=0, count=CustomModel.GetCount(); i<count; ++i)
	{
		FBModel *pModel = (FBModel*) CustomModel[i];
		RenderModel(pModel, !useCustomColor, bindColor, false);
	}

	//if (bindColor)
	//	glDisable(GL_BLEND);

	mShaderDrawWithDepth->UnBind();

	mFramebuffer->UnBind();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	CHECK_GL_ERROR_MOBU();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTexture(GL_TEXTURE0);

	//glEnable(GL_DEPTH_TEST);
	//glViewport(lViewport[0], lViewport[1], lViewport[2], lViewport[3]);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void AddPropertyViewForSolidColor(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(COMPOSITESOLIDCOLOR__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectCompositionColor::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForSolidColor( "Active", "" );
	AddPropertyViewForSolidColor( "Input", "");
	AddPropertyViewForSolidColor( "Reload Shader", "" );
	AddPropertyViewForSolidColor( "Opacity", "" );

	AddPropertyViewForSolidColor( "", "Transformation", true );
	AddPropertyViewForSolidColor( "Translation", "Transformation" );
	AddPropertyViewForSolidColor( "Rotation", "Transformation" );
	AddPropertyViewForSolidColor( "Uniform Scaling", "Transformation" );
	AddPropertyViewForSolidColor( "Scaling", "Transformation" );
	AddPropertyViewForSolidColor( "Pivot Offset", "Transformation" );
	AddPropertyViewForSolidColor( "Transform Init", "Transformation" );
	AddPropertyViewForSolidColor( "Fit Image Size", "Transformation" );

	AddPropertyViewForSolidColor( "", "Masking", true );
	AddPropertyViewForSolidColor( "Use Composite Mask", "Masking" );
	AddPropertyViewForSolidColor( "Select Composite Mask", "Masking" );
	AddPropertyViewForSolidColor( "Invert Composite Mask", "Masking" );
	AddPropertyViewForSolidColor( "Custom Mask", "Masking" );

	AddPropertyViewForSolidColor("", "Size Control", true);
	AddPropertyViewForSolidColor("Override Size", "Size Control");
	AddPropertyViewForSolidColor("User Width", "Size Control");
	AddPropertyViewForSolidColor("User Height", "Size Control");
	AddPropertyViewForSolidColor("ReSize Factor", "Size Control");

	AddPropertyViewForSolidColor( "", "Advance Color", true );
	AddPropertyViewForSolidColor("Background Width", "Advance Color");
	AddPropertyViewForSolidColor("Background Height", "Advance Color");
	AddPropertyViewForSolidColor( "Use Background Gradient", "Advance Color" );
	AddPropertyViewForSolidColor( "Background Upper Color", "Advance Color" );
	AddPropertyViewForSolidColor( "Background Upper Alpha", "Advance Color" );
	AddPropertyViewForSolidColor( "Background Lower Color", "Advance Color" );
	AddPropertyViewForSolidColor( "Background Lower Alpha", "Advance Color" );
	AddPropertyViewForSolidColor( "Use Background Texture", "Advance Color" );
	AddPropertyViewForSolidColor( "Background Texture", "Advance Color" );
	AddPropertyViewForSolidColor("Back Texture Width", "Advance Color");
	AddPropertyViewForSolidColor("Back Texture Height", "Advance Color");

	AddPropertyViewForSolidColor("", "Info", true);
	AddPropertyViewForSolidColor("Out Width", "Info");
	AddPropertyViewForSolidColor("Out Height", "Info");
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectCompositionColor::ObjectCompositionColor( const char* pName, HIObject pObject ) 
	: ObjectCompositeLayer( pName, pObject )
	, mBackground( new CompositeBackground() )
{
	FBClassInit;

	mBackground->Init(this);
}

void ObjectCompositionColor::CopyFrom(ObjectCompositeBase *pNode)
{
	if ( FBIS(pNode, ObjectCompositionColor) )
	{
		ObjectCompositionColor *pColor = (ObjectCompositionColor*) pNode;
		/*
		PropertyCopy( Color, pColor->Color );
		PropertyCopy( Gradient, pColor->Gradient );
		PropertyCopy( ColorUpper, pColor->ColorUpper );
		PropertyCopy( ColorLower, pColor->ColorLower );
		PropertyCopy( UseTexture, pColor->UseTexture );
		
		for (int i=0; i<pColor->Texture.GetCount(); ++i)
			Texture.Add( pColor->Texture[i] );
			*/
		//PropertyCopy( Replaceable, pColor->Replaceable );
	}
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectCompositionColor::FBCreate()
{
	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
/*
void ObjectCompositionColor::FBDestroy()
{
	
}
*/
/*
bool ObjectCompositionColor::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ObjectCompositionColor::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if( pStoreWhat == kAttributes )
	{
	}
	return true;
}
*/
/*
void ObjectCompositionColor::ReplaceTexture(FBTexture *pTexture)
{
	if (Texture.GetCount() )
	{
		mTempTexture = (FBTexture*) Texture.GetAt(0);
	}

	Texture.Clear();
	Texture.Add(pTexture);
}

void ObjectCompositionColor::FetchTexture()
{
	if (mTempTexture)
	{
		Texture.Clear();
		Texture.Add(mTempTexture);

		mTempTexture = nullptr;
	}
}
*/


const GLuint ObjectCompositionColor::ComputeLayerTexture(const CCompositionInfo *pInfo, CompositeFinalStats &stats)
{
	// DONE: allocate texture for background
	const float previewSizeFactor = pInfo->GetPreviewSizeFactor();
	if (false == mBackground->PrepRender(pInfo, true, false, previewSizeFactor) )
	{
		Active = false;
		return 0;
	}

	mProcessingWidth = mBackground->GetProcessingWidth();
	mProcessingHeight = mBackground->GetProcessingHeight();
	
	if (false == mTextureBuffer.IsTexturesReady() || false == mTextureBuffer.IsEqualSize(mProcessingWidth, mProcessingHeight) )
	{
		mTextureBuffer.ReSizeLayersData( mProcessingWidth, mProcessingHeight );
		mBackground->NeedUpdate();
	}

	// count the texture buffer memory
	stats.CountTextures(1, mProcessingWidth*mProcessingHeight*4);

	const GLuint bufferTextureId = mBackground->ComputeTexture(pInfo, stats, mTextureBuffer.GetCurrentTextureId(), true);
	mLastLayerTextureId = bufferTextureId;

	//
	// LOCAL FILTERS
	CProcessingInfo		prInfo(mProcessingWidth, mProcessingHeight);
	ProcessFilters(pInfo, prInfo, stats);

	return mLastLayerTextureId;
}

void ObjectCompositionColor::DoTransformFitImageSize()
{
	ObjectComposition *pFinal = nullptr;

	for (int i=0, count=GetDstCount(); i<count; ++i)
	{
		if (FBIS(GetDst(i), ObjectComposition) )
		{
			pFinal = (ObjectComposition*) GetDst(i);
			break;
		}
	}

	if (nullptr != pFinal && mBackground->BackgroundTexture->GetCount() > 0)
	{
		FBTexture *pTexture = (FBTexture*) mBackground->BackgroundTexture->GetAt(0);

		const int thiswidth = pFinal->GetWidthValue();
		const int thisheight = pFinal->GetHeightValue();

		const int texwidth = pTexture->Width;
		const int texheight = pTexture->Height;

		FBVector2d scaling(100.0, 100.0);

		if (thiswidth != 0.0 && thisheight != 0.0)
		{
			scaling[0] = 100.0 * texwidth / thiswidth;
			scaling[1] = 100.0 * texheight / thisheight;
		}

		Scaling = scaling;
	}
}

bool ObjectCompositionColor::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if (mBackground.get() )
		mBackground->PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}
bool ObjectCompositionColor::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	if (mBackground.get() )
		mBackground->PlugNotifyBackground(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}



///////////////////////////////////////////////////////////////////////////////////////////////////

void ObjectCompositionShadow::AddPropertyViewForShadow(const char* pPropertyName, const char* pHierarchy, bool pIsFolder)
{
	FBPropertyViewManager::TheOne().AddPropertyView(COMPOSITESHADOW__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectCompositionShadow::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForShadow( "Active", "" );
	AddPropertyViewForShadow( "Input", "");
	AddPropertyViewForShadow( "Reload Shader", "" );
	AddPropertyViewForShadow( "Opacity", "" );

	AddPropertyViewForShadow( "", "Transformation", true );
	AddPropertyViewForShadow( "Translation", "Transformation" );
	AddPropertyViewForShadow( "Rotation", "Transformation" );
	AddPropertyViewForShadow( "Uniform Scaling", "Transformation" );
	AddPropertyViewForShadow( "Scaling", "Transformation" );
	AddPropertyViewForShadow( "Pivot Offset", "Transformation" );
	AddPropertyViewForShadow( "Transform Init", "Transformation" );
	AddPropertyViewForShadow( "Fit Image Size", "Transformation" );

	AddPropertyViewForShadow( "", "Masking", true );
	AddPropertyViewForShadow( "Use Composite Mask", "Masking" );
	AddPropertyViewForShadow( "Select Composite Mask", "Masking" );
	AddPropertyViewForShadow( "Invert Composite Mask", "Masking" );
	AddPropertyViewForShadow( "Custom Mask", "Masking" );

	AddPropertyViewForShadow("", "Size Control", true);
	AddPropertyViewForShadow("Override Size", "Size Control");
	AddPropertyViewForShadow("User Width", "Size Control");
	AddPropertyViewForShadow("User Height", "Size Control");
	AddPropertyViewForShadow("ReSize Factor", "Size Control");

	AddPropertyViewForShadow( "", "Advance Color", true );
	AddPropertyViewForShadow("Background Width", "Advance Color");
	AddPropertyViewForShadow("Background Height", "Advance Color");
	AddPropertyViewForShadow( "Use Background Gradient", "Advance Color" );
	AddPropertyViewForShadow( "Background Upper Color", "Advance Color" );
	AddPropertyViewForShadow( "Background Upper Alpha", "Advance Color" );
	AddPropertyViewForShadow( "Background Lower Color", "Advance Color" );
	AddPropertyViewForShadow( "Background Lower Alpha", "Advance Color" );
	AddPropertyViewForShadow( "Use Background Texture", "Advance Color" );
	AddPropertyViewForShadow( "Background Texture", "Advance Color" );
	AddPropertyViewForShadow("Back Texture Width", "Advance Color");
	AddPropertyViewForShadow("Back Texture Height", "Advance Color");

	AddPropertyViewForShadow("", "Info", true);
	AddPropertyViewForShadow("Out Width", "Info");
	AddPropertyViewForShadow("Out Height", "Info");
}

/************************************************
 *	Constructor.
 ************************************************/
ObjectCompositionShadow::ObjectCompositionShadow( const char* pName, HIObject pObject ) 
	: ObjectCompositeLayer( pName, pObject )
{
	FBClassInit;

	mProgramShadow = nullptr;
	mProgramShadowUniforms.reset( new CProgramUniformsBase() );
}

void ObjectCompositionShadow::CopyFrom(ObjectCompositeBase *pNode)
{
	if ( FBIS(pNode, ObjectCompositionColor) )
	{
		ObjectCompositionShadow *pShadowNode = (ObjectCompositionShadow*) pNode;
		/*
		PropertyCopy( Color, pColor->Color );
		PropertyCopy( Gradient, pColor->Gradient );
		PropertyCopy( ColorUpper, pColor->ColorUpper );
		PropertyCopy( ColorLower, pColor->ColorLower );
		PropertyCopy( UseTexture, pColor->UseTexture );
		
		for (int i=0; i<pColor->Texture.GetCount(); ++i)
			Texture.Add( pColor->Texture[i] );
			*/
		//PropertyCopy( Replaceable, pColor->Replaceable );
	}
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectCompositionShadow::FBCreate()
{
	FBPropertyPublish(this, Density, "Density", nullptr, nullptr);
	FBPropertyPublish(this, Feather, "Feather", nullptr, nullptr);
	
	FBPropertyPublish(this, Bias, "Bias", nullptr, nullptr);

	FBPropertyPublish(this, ZoneObjects, "Zone Objects", nullptr, nullptr);
	FBPropertyPublish(this, CreateZone, "Create Zone", nullptr, SetCreateZone);

	FBPropertyPublish(this, UseModelProperties, "Use Model Properties", nullptr, nullptr);
	
	FBPropertyPublish(this, SortZones, "Sort Zones", nullptr, nullptr);
	FBPropertyPublish(this, UpdateAllZones, "Update All Zones", nullptr, SetUpdateAllZonesAction);

	
	Density = 100.0;
	Density.SetMinMax(0.0, 100.0);

	Feather = 100.0;
	Feather.SetMinMax(0.0, 100.0);

	Bias = 2.0;

	ZoneObjects.SetSingleConnect(false);
	ZoneObjects.SetFilter( ModelShadowZone::GetInternalClassId() );
	UseModelProperties = false;
	SortZones = false;

	BlendMode.SetInt(eBlendMultiply);

	return ParentClass::FBCreate();
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
/*
void ObjectCompositionShadow::FBDestroy()
{
	
}
*/
/*
bool ObjectCompositionShadow::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ObjectCompositionShadow::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if( pStoreWhat == kAttributes )
	{
	}
	return true;
}
*/
/*
void ObjectCompositionShadow::ReplaceTexture(FBTexture *pTexture)
{
	if (Texture.GetCount() )
	{
		mTempTexture = (FBTexture*) Texture.GetAt(0);
	}

	Texture.Clear();
	Texture.Add(pTexture);
}

void ObjectCompositionShadow::FetchTexture()
{
	if (mTempTexture)
	{
		Texture.Clear();
		Texture.Add(mTempTexture);

		mTempTexture = nullptr;
	}
}
*/


bool ObjectCompositionShadow::ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo)
{
	if (false == PrepComputeProgram(pInfo) )
		return false;

	return ObjectCompositeLayer::ReadyToApply(pInfo, prInfo);
}

const GLuint ObjectCompositionShadow::ComputeLayerTexture(const CCompositionInfo *pInfo, CompositeFinalStats &stats)
{
	// DONE: allocate texture for background
	const float previewSizeFactor = pInfo->GetPreviewSizeFactor();
	
	mProcessingWidth = pInfo->GetRenderWidth();
	mProcessingHeight = pInfo->GetRenderHeight();
	
	if (false == mTextureShadow.IsTexturesReady() || false == mTextureShadow.IsEqualSize(mProcessingWidth, mProcessingHeight) )
	{
		mTextureShadow.ReSizeLayersData( mProcessingWidth, mProcessingHeight );
		//mBackground->NeedUpdate();
	}

	// count the texture buffer memory
	stats.CountTextures(1, mProcessingWidth*mProcessingHeight*4);

	mLastLayerTextureId = mTextureShadow.GetCurrentTextureId();

	CProcessingInfo		prInfo(mProcessingWidth, mProcessingHeight);

	RenderShadowZones(pInfo, prInfo, stats, mLastLayerTextureId);
	ProcessFilters(pInfo, prInfo, stats);

	return mLastLayerTextureId;
}



void ObjectCompositionShadow::PrepShadowData(const CCompositionInfo *pInfo)
{
	ShadowData &data = mShadowData;

	FBCamera *pCamera = ((CCompositionInfo*) pInfo)->GetRenderCamera();

	// for Zone computing
	data.zoneInfo.x = (float) ComputeNumberOfZoneModels();

	mGPUShadowBuffer.UpdateData( sizeof(ShadowData), 1, &mShadowData );
}

void ObjectCompositionShadow::PrepZoneData(ZoneData &data, FBModel *pZoneModel, const bool useZoneProperties)
{
	if (pGPUFBScene == nullptr)
		return;

	//FBMatrix model, invModel;
	
	ModelShadowZone *pModel = (ModelShadowZone*) pZoneModel;
	//pModel->GetMatrix(model);
	//FBMatrixInverse( invModel, model );

	FBLight *pLight = (pModel->MasterLight.GetCount() > 0) ? (FBLight*) pModel->MasterLight.GetAt(0) : nullptr;

	if (nullptr == pLight)
		return;

	int id;
	pGPUFBScene->GetShadowIdAndLightMatrix( pModel, id, data.lightMatrix );
	data.Id = (float) id;

	//
	
	data.density = 0.01f * (float) Density;
	data.feather = 0.01f * (float) Feather;
	data.bias = 0.001f * (float) Bias;

	if (useZoneProperties)
	{
		data.feather = 0.01f * pModel->Feather;
		data.density = 0.01f * pModel->Density;	
		data.bias = 0.001f * (float) pModel->Bias;
	}
	
	//

	FBMatrix model, invModel;
	
	pModel->GetMatrix(model);
	FBMatrixInverse( invModel, model );

	for (int i=0; i<16; ++i)
	{
		data.invMatrix.mat_array[i] = (float) invModel[i];
	}

	FBVector3d pMin, pMax;
	pModel->GetBoundingBox( pMin, pMax );

	for (int i=0; i<3; ++i)
	{
		data.volumeMin[i] = pMin[i];
		data.volumeMax[i] = pMax[i];
	}
	data.volumeMin[3] = data.volumeMax[3] = 1.0f;
}

void ObjectCompositionShadow::PrepAllZones(FBCamera *pCamera)
{
	SortZoneModels(pCamera, (ZoneObjects.AsInt() > 0));

	size_t count = mZoneSortVector.size();
	if (count == 0)
		return;

	mZoneData.resize(count);
	bool useZoneProperties = UseModelProperties;

	for (size_t i=0; i<count; ++i)
	{
		PrepZoneData(mZoneData[i], mZoneSortVector[i].first, useZoneProperties);
	}

	mGPUZoneBuffer.UpdateData( sizeof(ZoneData), count, mZoneData.data() );
}

// DONE: check alpha texture and Zone connections to query a propriate shader program !

bool ObjectCompositionShadow::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if ( (pAction == kFBCandidated) && (pThis == &BlendMode) )
	{
		// query a new program with updated define list
		//mProgramShadow = nullptr;
	}
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ObjectCompositionShadow::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) 
{
	
	if (pThis == &ZoneObjects)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
			// update program defines
			//mProgram = nullptr;
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
			//mProgram = nullptr;
		}
	}

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void ObjectCompositionShadow::SetCreateZone(HIObject object, bool value)
{
	ObjectCompositionShadow *pBase = FBCast<ObjectCompositionShadow>(object);
	if (pBase && value) 
	{
		pBase->DoCreateZone();
	}
}

void ObjectCompositionShadow::DoCreateZone()
{
	// DONE:
	ModelShadowZone *pZone = (ModelShadowZone*) FBCreateObject( MARKER__ASSETPATH, SHADOWZONE__DESCSTR, FBString(Name, " Zone") );
	if (pZone == nullptr)
		return;

	pZone->Show = true;
	pZone->Visibility = true;
	pZone->Scaling = FBVector3d(10.0, 10.0, 10.0);

	FBCamera *pCurrCamera = mSystem.Renderer->CurrentCamera;
	if (pCurrCamera)
	{
		if (FBIS(pCurrCamera, FBCameraSwitcher) )
			pCurrCamera = ( (FBCameraSwitcher*) pCurrCamera)->CurrentCamera;
		/*
		vec3 points[8];
		ComputeCameraFrustumPoints(pCurrCamera->CameraViewportWidth, pCurrCamera->CameraViewportHeight, pCurrCamera, 
			pCurrCamera->FarPlaneDistance, pCurrCamera->NearPlaneDistance, pCurrCamera->FieldOfView, points);

		vec3 nearCenter, farCenter;
		nearCenter = 0.25 * (points[0] + points[1] + points[2] + points[3]);
		farCenter = 0.25 * (points[4] + points[5] + points[6] + points[7]);

		vec3 point = nearCenter + 0.1f * (farCenter - nearCenter);
		*/

		FBVector3d v;
		pCurrCamera->GetVector(v);

		//pZone->SetVector( FBVector3d((double)point.x, (double)point.y, (double)point.z) );
		pZone->SetVector(v);
	}

	FBConnect(pZone, &this->ZoneObjects);
}

void ObjectCompositionShadow::SetUpdateAllZonesAction(HIObject object, bool value)
{
	ObjectCompositionShadow *pBase = FBCast<ObjectCompositionShadow>(object);
	if (pBase && value) 
	{
		pBase->DoUpdateAllZones();
	}
}

void ObjectCompositionShadow::DoUpdateAllZones()
{
	for (int i=0, count=ZoneObjects.GetCount(); i<count; ++i)
		{
			ModelShadowZone *pZone = (ModelShadowZone*) ZoneObjects.GetAt(i);
			
			if (pZone->Enabled)
				pZone->SetUpdateFlag(true);
		}
}

const int ObjectCompositionShadow::ComputeNumberOfZoneModels()
{
	int totalCount = 0;

	if (ZoneObjects.GetCount() > 0)
	{
		for (int i=0, count=ZoneObjects.GetCount(); i<count; ++i)
		{
			FBComponent *pComp = ZoneObjects.GetAt(i);

			if (FBIS(pComp, ModelShadowZone) )
			{
				ModelShadowZone *pZone = (ModelShadowZone*) pComp;

				if (pZone->Enabled && pZone->MasterLight.GetCount() > 0)
					totalCount = totalCount + 1;
			}
		}
	}
	return totalCount;
}

void ObjectCompositionShadow::SortZoneModels(FBCamera *pCamera, const bool sorting)
{
	bool useZoneObject = false;

	if (ZoneObjects.GetCount() > 0)
	{
		int totalCount = 0;
		for (int i=0, count=ZoneObjects.GetCount(); i<count; ++i)
		{
			FBComponent *pComp = ZoneObjects.GetAt(i);

			if (FBIS(pComp, ModelShadowZone) )
			{
				ModelShadowZone *pZone = (ModelShadowZone*) pComp;
			
				if (pZone->Enabled && pZone->MasterLight.GetCount() > 0)
					totalCount = totalCount + 1;
			}
		}

		useZoneObject = (totalCount > 0);

		mZoneSortVector.resize(totalCount);

		totalCount = 0;
		for (int i=0, count=ZoneObjects.GetCount(); i<count; ++i)
		{
			FBComponent *pComp = ZoneObjects.GetAt(i);

			if (FBIS(pComp, ModelShadowZone) )
			{
				ModelShadowZone *pZone = (ModelShadowZone*) pComp;
			
				if (pZone->Enabled && pZone->MasterLight.GetCount() > 0)
				{
					mZoneSortVector[totalCount].first = pZone;
					totalCount = totalCount + 1;
				}
			}
		}
	}

	//
	if (nullptr == pCamera)
		return;

	FBVector3d cameraPos;
	pCamera->GetVector(cameraPos);

	// check all needed blend modes

	if (true == useZoneObject && true == sorting)
	{
		int totalCount =0;
		for (int i=0, count=ZoneObjects.GetCount(); i<count; ++i)
		{
			FBComponent *pComp = ZoneObjects.GetAt(i);

			if (FBIS(pComp, ModelShadowZone) )
			{
				ModelShadowZone *pZone = (ModelShadowZone*) pComp;
			
				if (pZone->Enabled && pZone->MasterLight.GetCount() > 0)
				{
					double dist = 0.0;
					FBVector3d pos;
					pZone->GetVector(pos);
					dist = VectorLength(cameraPos, pos);

					mZoneSortVector[totalCount].second = dist;
					totalCount = totalCount + 1;
				}
			}
		}

		// sort Zones by dist value
		auto ZoneCmp = [](std::pair<FBModel*,double> const & pairA, std::pair<FBModel*,double> const & pairB) 
		{ 
			 return pairA.second < pairB.second;
		};

		std::sort( begin(mZoneSortVector), end(mZoneSortVector), ZoneCmp );
	}
}

bool ObjectCompositionShadow::PrepComputeProgram(const CCompositionInfo *pInfo)
{
	if (false == Active)
		return false;

	if (0.0 == Opacity)
		return false;

	//const int w = prInfo.GetWidth();
	//const int h = prInfo.GetHeight();

	if (mNeedProgramReload && nullptr != mProgramShadow)
	{
		CompositeComputeShader::CMixedProgramManager::instance().ReloadProgram(mProgramShadow);
		//mNeedProgramReload = false;

		bool shaderOk = mProgramShadow->IsOk();
		if (false == shaderOk)
		{
			Active = false;
			return false;
		}

		mProgramShadowUniforms->Prep( mProgramShadow->GetProgramId() );
	}

	if (nullptr == mProgramShadow)
	{
		FBString strHeader( "#version 430\n"
			"#extension GL_ARB_shading_language_include : require\n" );
		
		strHeader += "layout (local_size_x =32, local_size_y = 32) in;\n";

		FBString strDefine( "" );

		// TODO: check when to apply same size difinition
		strDefine = strDefine + "#define SAME_SIZE\n";

		if ( HasMultiSamplingSupport() )
		{
			strDefine = strDefine + ((CCompositionInfo*) pInfo)->GetRenderSamplesDefineString();
		}

		const char *programLabel = MixedProgramLabel();
		const char *programPath = MixedProgramPath();

		mProgramShadow = CompositeComputeShader::CMixedProgramManager::instance().QueryAProgramMix(programLabel, strHeader, strDefine, programPath, "");
		
		bool shaderOk = mProgramShadow->IsOk();
		if (false == shaderOk)
		{
			Active = false;
			return false;
		}

		// DONE: update uniform locations
		mProgramShadowUniforms->Prep( mProgramShadow->GetProgramId() );
	}

	if (nullptr == mProgramShadow)
		return false;

	return true;
}

bool ObjectCompositionShadow::RenderShadowZones(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, GLuint dstTexId)
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

	mProgramShadow->Bind();

	// update uniform values
	mProgramShadowUniforms->Upload(pInfo, prInfo, 
		mProcessingWidth, mProcessingHeight,
		InvertCompositeMask.AsInt() > 0, 0.01f * (float)Opacity);
	
	glBindImageTexture(0, dstTexId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	
	const MainFrameBuffer *pFrameBuffer = pInfo->GetFrameBufferInfo(eRenderLayerMain);

	GLenum target = GL_TEXTURE_2D;
	GLuint positionId = pFrameBuffer->GetFinalPositionObject(); // pInfo->GetRenderPositionId();

	if (pInfo->IsRenderMultisampling() && HasMultiSamplingSupport() )
	{
		target = GL_TEXTURE_2D_MULTISAMPLE;
		positionId = pFrameBuffer->GetPositionObjectMS();
	}

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(target, positionId );
	
	pGPUFBScene = &CGPUFBScene::instance();

	PrepShadowData(pInfo);
	mGPUShadowBuffer.Bind(0);

	const int volumeCount = ComputeNumberOfZoneModels();
	if (volumeCount > 0)
	{
		PrepAllZones( ((CCompositionInfo*)pInfo)->GetRenderCamera() );
		mGPUZoneBuffer.Bind(1);

		glActiveTexture(GL_TEXTURE3);

		if (pInfo->IsShadowMultisampling() )
		{
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, pGPUFBScene->GetShadowTextureId() );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, pGPUFBScene->GetShadowTextureId() );
		}
	}

	// DONE: compute the number of groups !!
	mProgramShadow->DispatchPipeline(groups_x, groups_y, 1);

	//
	stats.CountComputeShader(1, groups_x*groups_y );
	
	mProgramShadow->UnBind();

	
	glActiveTexture(GL_TEXTURE0);

	CHECK_GL_ERROR_MOBU();

	return true;
}