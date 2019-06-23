
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectDecalFilter.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "compositeMaster_objectDecalFilter.h"
#include "model_3d_decal.h"

#include "IO\tga.h"
#include "graphics\CheckGLError_MOBU.h"
#include <shellapi.h>

FBClassImplementation2(ObjectFilter3dDecal)
FBUserObjectImplement(ObjectFilter3dDecal, "Composition 3d Decal", EFFECT_ICON);					//Register UserObject class
FBElementClassImplementationFilter(ObjectFilter3dDecal, FILTER3DDECAL__ASSETNAME, EFFECT_ICON);				//Register to the asset system

/*
const char * FBPropertyBaseEnum<EDecalResolution>::mStrings[] = {
	"Custom",
	"Global Settings",
	"FullHD",
	"2K",
	"4K",
	0};
	*/
////////////////////////////////////////////////////////////////////////////////////////////////// PROCS

void ObjectFilter3dDecal::AddPropertyView(const char* pPropertyName, const char* pHierarchy, bool pIsFolder)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTER3DDECAL__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilter3dDecal::AddPropertiesToPropertyViewManager()
{
	AddPropertyView( "Active", "" );
	AddPropertyView( "Reload Shader", "" );
	AddPropertyView( "Opacity", "" );

	AddPropertyView( "", "Masking", true );
	AddPropertyView( "Use Composite Mask", "Masking" );
	AddPropertyView( "Select Composite Mask", "Masking" );
	AddPropertyView( "Invert Composite Mask", "Masking" );
	AddPropertyView( "Custom Mask", "Masking" );

	AddPropertyView( "", "Info", true );
	AddPropertyView( "Out Width", "Info" );
	AddPropertyView( "Out Height", "Info" );
}



///////////////////////////////////////////////////////////////////////////////////////////////////


/************************************************
 *	Constructor.
 ************************************************/
ObjectFilter3dDecal::ObjectFilter3dDecal( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new CProgramUniformsBase() )
{
	FBClassInit;

	mCreateNewDecal = false;
	mGrabImage = false;

	for (int i=0; i<25; ++i)
		mSubroutineIndex[i] = i;
}


/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectFilter3dDecal::FBCreate()
{
	
	FBPropertyPublish(this, BlendMode, "Blend Mode", nullptr, nullptr);
	
	//FBPropertyPublish(this, Resolution, "Resolution", nullptr, SetResolution);
	FBPropertyPublish(this, UseGlobalResolution, "Use Global Resolution", nullptr, nullptr);
	FBPropertyPublish(this, CustomWidth, "Custom Width", nullptr, nullptr);
	FBPropertyPublish(this, CustomHeight, "Custom Height", nullptr, nullptr);

	FBPropertyPublish(this, DecalObjects, "Decal Objects", nullptr, nullptr);
	FBPropertyPublish(this, CreateDecal, "Create Decal", nullptr, SetCreateDecal);
	FBPropertyPublish(this, RefreshTexture, "Refresh Texture", nullptr, SetRefreshTexture);
	FBPropertyPublish(this, GrabImage, "Grab Image", nullptr, SetGrabImage);

	FBPropertyPublish(this, RunPhotoshopOnCreation, "Run PS On Creation", nullptr, nullptr);
	FBPropertyPublish(this, PhotoshopPath, "PS Path", nullptr, nullptr);

	FBPropertyPublish(this, UseModelProperties, "Use Model Properties", nullptr, nullptr);
	
	//
	//

	BlendMode = eCompositeBlendAdd;

	//Resolution = eDecalResolutionCustom;
	UseGlobalResolution = true;
	CustomWidth = 1920;
	CustomHeight = 1080;

	RunPhotoshopOnCreation = true;
	PhotoshopPath = "C:\\Program Files (x86)\\Photoshop\\x64\\Photoshop.exe";

	DecalObjects.SetSingleConnect(false);
	// DecalObjects.SetFilter( ModelFogVolume::GetInternalClassId() );
	UseModelProperties = true;

	return ParentClass::FBCreate();
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/

void ObjectFilter3dDecal::FBDestroy()
{
	ParentClass::FBDestroy();
}


// DONE: check alpha texture and volume connections to query a propriate shader program !

bool ObjectFilter3dDecal::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if (pAction == kFBCandidated && (pThis == &BlendMode) )
	{
		// query a new program with updated define list
		mProgram = nullptr;
	}
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ObjectFilter3dDecal::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) 
{
	
	if (pThis == &DecalObjects)
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


void ObjectFilter3dDecal::SetCreateDecal(HIObject object, bool value)
{
	ObjectFilter3dDecal *pBase = FBCast<ObjectFilter3dDecal>(object);
	if (pBase && value) 
	{
		pBase->DoCreateDecal();
	}
}

void ObjectFilter3dDecal::SetRefreshTexture(HIObject object, bool value)
{
	ObjectFilter3dDecal *pBase = FBCast<ObjectFilter3dDecal>(object);
	if (pBase && value) 
	{
		pBase->DoRefreshTexture();
	}
}

void ObjectFilter3dDecal::SetGrabImage(HIObject object, bool value)
{
	ObjectFilter3dDecal *pBase = FBCast<ObjectFilter3dDecal>(object);
	if (pBase && value) 
	{
		pBase->DoGrabImage();
	}
}
/*
void ObjectFilter3dDecal::SetResolution(HIObject object, EDecalResolution value)
{
	ObjectFilter3dDecal *pBase = FBCast<ObjectFilter3dDecal>(object);
	if (pBase) 
	{
		switch(value)
		{
		case eDecalResolutionHD:
			pBase->CustomWidth = 1920;
			pBase->CustomHeight = 1080;
			break;
		case eDecalResolution2K:
			pBase->CustomWidth = 2560;
			pBase->CustomHeight = 1440;
			break;
		case eDecalResolution4K:
			pBase->CustomWidth = 4000;
			pBase->CustomHeight = 3000;
			break;
		}

		pBase->Resolution.SetPropertyValue(value);
	}
}
*/
void ObjectFilter3dDecal::DoCreateDecal()
{
	mCreateNewDecal = true;
}

void ObjectFilter3dDecal::DoGrabImage()
{
	mGrabImage = true;
}

void ObjectFilter3dDecal::CropTexture(const GLuint texId, 
		const int texWidth,
		const int texHeight,
		const int borderX,
		const int borderY,
		const GLuint dstId,
		const int dstWidth,
		const int dstHeight)
{

	FBString strHeader( "#version 430\n"
			"layout (local_size_x =32, local_size_y = 32) in;\n" );
	FBString strDefine( "" );

	const char *programPath = COPY_COMPUTE_SHADER;

	CompositeComputeShader::CMixedProgram		*mCopyProgram = nullptr;
	mCopyProgram = CompositeComputeShader::CMixedProgramManager::instance().QueryAProgramMix( "copy", 
		strHeader, strDefine, programPath, "" );

	if (mCopyProgram == nullptr || mCopyProgram->IsOk() == false)
		return;

	mCopyProgram->Bind();

	const GLint offsetLoc = glGetUniformLocation(mCopyProgram->GetProgramId(), "offset");
	if (offsetLoc >= 0)
		glUniform2f( offsetLoc, (float)borderX, (float)borderY );

	glBindImageTexture(0, dstId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texId);

	const int computeLocalX = 32;
	const int computeLocalY = 32;
	const int groups_x = dstWidth / computeLocalX + 1;
	const int groups_y = dstHeight / computeLocalY + 1;

	mCopyProgram->DispatchPipeline(groups_x, groups_y, 1);

	mCopyProgram->UnBind();

	glActiveTexture(GL_TEXTURE0);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	CHECK_GL_ERROR_MOBU();
}

void ObjectFilter3dDecal::DoSaveTextureToFile(const GLuint texId, const char *filename, bool needResize, int newWidth, int newHeight)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);
			

	int width, height, internalFormat, format, pixelMemorySize, imageSize, numberOfLods;
	bool isCompressed;

	TextureObjectGetInfo(GL_TEXTURE_2D, width, height, internalFormat, format, isCompressed, pixelMemorySize, imageSize, numberOfLods);

	if (isCompressed)
		return;

	unsigned char *buffer = new unsigned char[width * height * pixelMemorySize];

	TextureObjectGetData(GL_TEXTURE_2D, buffer, width, height, internalFormat, format, isCompressed, pixelMemorySize, imageSize, numberOfLods);

	// DONE: save as TGA
	//unsigned char pixelDepth = pixelMemorySize * 8;	// number of bits
	//tgaSave(filename, width, height, pixelDepth, buffer);

	FBImage	image(filename);

	image.Init(kFBImageFormatRGBA32, width, height);

	unsigned char *imageBuffer = image.GetBufferAddress();
	memcpy(imageBuffer, buffer, sizeof(unsigned char) * width * height * pixelMemorySize);

	// TODO: ConvertImageSize to the final decal resolution !!
	if (needResize)
	{
		image.ConvertSize(newWidth, newHeight);
	}

	image.WriteToTif(filename, "", true);

	delete [] buffer;

	glBindTexture(GL_TEXTURE_2D, 0);
}

void ObjectFilter3dDecal::DoRunPhotoshop(const char *psPath, const char *filename)
{
	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = psPath;        
	ShExecInfo.lpParameters = filename;   
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL; 
	
	::ShellExecuteEx(&ShExecInfo);
}

const int ObjectFilter3dDecal::ComputeRealDecalCount()
{
	int realCount = 0;
	int count = DecalObjects.GetCount();

	for (int i=0; i<count; ++i)
	{
		if (FBIS(DecalObjects[i], ModelDecal) && ( (ModelDecal*) DecalObjects[i])->Enabled == true )
		{
			realCount += 1;
		}
	}

	return realCount;
}

void ObjectFilter3dDecal::PrepGPUBuffer(const CCompositionInfo *pInfo)
{
	const int realcount = ComputeRealDecalCount();
	mData.info = vec4((float) realcount, 0.0f, 0.0f, 0.0f);

	// frustum information for the 3d position reconstruction
	FBCamera *pCamera = ((CCompositionInfo*) pInfo)->GetRenderCamera();

	vec3 points[8];
	int renderWidth = pInfo->GetRenderWidth() - 2 * pInfo->GetRenderBorderX();
	int renderHeight = pInfo->GetRenderHeight() - 2 * pInfo->GetRenderBorderY();

	ComputeCameraFrustumPoints( renderWidth, renderHeight, pCamera, 
		pCamera->FarPlaneDistance, pCamera->NearPlaneDistance, pCamera->FieldOfView, points );
	
	mData.viewOrig[0] = vec4(points[0], 1.0);
	mData.viewDir[0] = vec4(points[4], 1.0);
	
	mData.viewOrig[1] = vec4(points[1], 1.0);
	mData.viewDir[1] = vec4(points[5], 1.0);
	
	mData.viewOrig[2] = vec4(points[2], 1.0);
	mData.viewDir[2] = vec4(points[6], 1.0);

	mData.viewOrig[3] = vec4(points[3], 1.0);
	mData.viewDir[3] = vec4(points[7], 1.0);

	// fill with each decal matrix

	//mData.decals.resize(realcount);
	//mData.decal.matrix.identity();

	for (int i=0; i<realcount; ++i)
	{
		if (i >= MAX_DECALS_COUNT)
			break;

		ModelDecal *pDecal = (ModelDecal*) GetRealDecal(i);

		pDecal->UpdateProjectionMatrices();
		const double *dmatrix = pDecal->GetFinalMatrix();
		
		for (int j=0; j<16; ++j)
			mData.decals[i].matrix.mat_array[j] = (float) dmatrix[j];
		/*
		for (int j=0; j<16; ++j)
			mData.decal.matrix.mat_array[j] = (float) dmatrix[j];
		*/
		//mData.decal.textureAddress = pDecal->GetTextureAddress();

		if (UseModelProperties)
		{
			mData.decals[i].opacity = 0.01f * (float) pDecal->Opacity;
			mData.decals[i].blendMode = (float) pDecal->BlendMode.AsInt();
		}
		else
		{
			mData.decals[i].opacity = 1.0f;
			mData.decals[i].blendMode = (float) BlendMode.AsInt();
		}

		mData.decals[i].contrast = 1.0f + 0.01f * (float) pDecal->Contrast;
		mData.decals[i].saturation = 1.0f + 0.01f * (float) pDecal->Saturation;
		mData.decals[i].brightness = 1.0f + 0.01f * (float) pDecal->Brightness;
		mData.decals[i].gamma = 0.01f * (float) pDecal->Gamma;

		mData.decals[i].hue = 0.01f * (float) pDecal->Hue;
		mData.decals[i].hueSat = 0.01f * (float) pDecal->HueSaturation;
		mData.decals[i].lightness = 0.01f * (float) pDecal->Lightness;
		mData.decals[i].inverse = (pDecal->Inverse) ? 1.0f : 0.0f;
	}

	mGPUBuffer.UpdateData( sizeof(vec4)*9 + sizeof(DecalData)*realcount, 1, &mData );
}

FBComponent *ObjectFilter3dDecal::GetRealDecal(const int index)
{
	int count = DecalObjects.GetCount();
	int ndx=0;
	for (int i=0; i<count; ++i)
	{
		if (FBIS(DecalObjects[i], ModelDecal) && ( (ModelDecal*) DecalObjects[i])->Enabled == true )
		{
			if (ndx == index)
			{
				return DecalObjects[i];
			}

			ndx += 1;
		}
	}

	return nullptr;
}

void ObjectFilter3dDecal::ConnectOrUpdateDecal(const char *imageName)
{
//	const int decalCount = ComputeRealDecalCount();

	ModelDecal *pDecal = nullptr;
	/*
	if (decalCount == 0)
	{
		pDecal = (ModelDecal*) FBCreateObject( MARKER__ASSETPATH, DECAL__DESCSTR, "Decal Model" );

		if (pDecal != nullptr)
			FBConnect(pDecal, &DecalObjects);
	}
	else
	{
		pDecal = (ModelDecal*) GetRealDecal(0);
	}
	*/

	pDecal = (ModelDecal*) FBCreateObject( MARKER__ASSETPATH, DECAL__DESCSTR, "Decal Model" );

	if (pDecal != nullptr)
		FBConnect(pDecal, &DecalObjects);

	if (pDecal == nullptr)
		return;

	// check for a connected texture ?!

	FBTexture *pTexture = nullptr;

	for (int i=0, count=pDecal->GetSrcCount(); i<count; ++i)
	{
		if ( FBIS(pDecal->GetSrc(i), FBTexture) )
		{
			pTexture = (FBTexture*) pDecal->GetSrc(i);
			break;
		}
	}

	if (pTexture == nullptr)
	{
		// create a new texture and video clip
		pTexture = new FBTexture( imageName );
		FBConnect( pTexture, &pDecal->Texture );
	}
	else
	{
		FBVideo *pVideo = pTexture->Video;
		if ( FBIS(pVideo, FBVideoClip) )
		{
			( (FBVideoClip*) pVideo)->Filename = imageName;
		}
		else
		{
			FBVideoClip *newClip = new FBVideoClip(imageName);
			pTexture->Video = newClip;
		}
	}

	// grab current camera matrix for a new texture
	pDecal->GrabValues();
}

void ObjectFilter3dDecal::DoRefreshTexture()
{
	const int decalCount = ComputeRealDecalCount();

	for (int i=0; i<decalCount; ++i)
	{
		ModelDecal *pDecal = (ModelDecal*) GetRealDecal(i);
		if (pDecal == nullptr)
			continue;

		pDecal->RefreshConnectedTexture();
	}
}

void ObjectFilter3dDecal::OnApplyFilter(	EApplyFilterStage stage, 
								const CCompositionInfo *pInfo, 
								const CProcessingInfo &prInfo, 
								CompositeFinalStats &stats, 
								const GLuint sourceTexId, 
								const GLuint dstTexId) 
{
	// IF new decal creation !
	//		grab source texture data
	//		save as a file
	//		run photoshop for file editing 
	//
	if (stage == eBeforeProgramBind && (mCreateNewDecal || mGrabImage))
	{
		mGrabImage = false;

		//const char *imageFileName = "D:\\testOutput.tif";

		FBFilePopup		lDialog;
		lDialog.Caption = "Please enter a filename to save into";
		lDialog.Style = kFBFilePopupSave;
		lDialog.Filter = "*.tif";

		if (lDialog.Execute() )
		{
			const char *imageFileName = lDialog.FullFilename;

			// TODO: if we have renderBorders, render into a new texture without borders !!

			const int borderX = pInfo->GetRenderBorderX();
			const int borderY = pInfo->GetRenderBorderY();
			
			GLuint saveId = sourceTexId;

			CompositeComputeShader::CComputeTextureBuffer<1>	mCroppedTexture;

			if (borderX > 0 || borderY > 0)
			{
				const int renderW = pInfo->GetRenderWidth();
				const int renderH = pInfo->GetRenderHeight();

				const int dstW = renderW - 2 * borderX;
				const int dstH = renderH - 2 * borderY;

				mCroppedTexture.ReSizeLayersData( dstW, dstH );
				saveId = mCroppedTexture.QueryATextureBuffer();

				CropTexture(sourceTexId, prInfo.GetWidth(), prInfo.GetHeight(), borderX, borderY,
					saveId, dstW, dstH);
			}

			DoSaveTextureToFile( saveId, imageFileName, true, CustomWidth, CustomHeight);
			
			mCroppedTexture.FreeLayersData();

			if (mCreateNewDecal)
			{
				mCreateNewDecal = false;

				if (RunPhotoshopOnCreation)
				{
					DoRunPhotoshop(PhotoshopPath.AsString(), imageFileName);
				}

				ConnectOrUpdateDecal(imageFileName);
			}
		}
	}

	if (stage == eBeforeProgramDispatch)
	{
		glUniformSubroutinesuiv( GL_COMPUTE_SHADER, GLsizei(25), &mSubroutineIndex[0] );

		PrepGPUBuffer(pInfo);
		mGPUBuffer.Bind(0);

		int count = ComputeRealDecalCount();
		for (int i=0; i<count; ++i)
		{
			if (i >= MAX_DECALS_COUNT)
				break;

			ModelDecal *pDecal = (ModelDecal*) GetRealDecal(i);
			//pDecal->MakeTextureResident();
			pDecal->BindTexture(3+i);
		}
		/*
		if (count > 0)
		{
			FBComponent *pComp = GetRealDecal(0);

			for (int i=0; i<pComp->GetSrcCount(); ++i)
			{
				if (FBIS(pComp->GetSrc(i), FBTexture))
				{
					FBTexture *pTexture = (FBTexture*) pComp->GetSrc(i);
					const GLuint textureId = pTexture->TextureOGLId;

					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, textureId);
					break;
				}
			}
		}
		*/
	}
	else if (stage == eAfterProgramDispatch)
	{
		int count = ComputeRealDecalCount();
		
		for (int i=0; i<count; ++i)
		{
			if (i >= MAX_DECALS_COUNT)
				break;

			ModelDecal *pDecal = (ModelDecal*) GetRealDecal(i);
			//pDecal->MakeTextureNonResident();
			pDecal->UnBindTexture(3+i);
		}
	}
}