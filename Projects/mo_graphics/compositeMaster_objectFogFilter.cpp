
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectFogFilter.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_objectFogFilter.h"
#include "compositemaster_common.h"
#include "IO\FileUtils.h"
#include "shared_misc.h"
#include "graphics\CheckGLError_MOBU.h"

#include "algorithm\nv_math.h"

#include "model_3d_fog.h"

#include <algorithm>

const char * FBPropertyBaseEnum<EFilterFogTargetType>::mStrings[] = {
	"Manual",
	"Center",
	"NearFar",
	"Volume",
	0};

FBClassImplementation2(ObjectFilter3dFog)
FBUserObjectImplement(ObjectFilter3dFog, "Composition 3d Fog", EFFECT_ICON);					//Register UserObject class
FBElementClassImplementationFilter(ObjectFilter3dFog, FILTER3DFOG__ASSETNAME, EFFECT_ICON);				//Register to the asset system


////////////////////////////////////////////////////////////////////////////////////////////////// PROCS

void AddPropertyViewForFog(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTER3DFOG__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilter3dFog::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForFog( "Active", "" );
	AddPropertyViewForFog( "Reload Shader", "" );
	AddPropertyViewForFog( "Opacity", "" );

	AddPropertyViewForFog( "", "Masking", true );
	AddPropertyViewForFog( "Use Composite Mask", "Masking" );
	AddPropertyViewForFog( "Select Composite Mask", "Masking" );
	AddPropertyViewForFog( "Invert Composite Mask", "Masking" );
	AddPropertyViewForFog( "Custom Mask", "Masking" );

	AddPropertyViewForFog( "", "Info", true );
	AddPropertyViewForFog( "Out Width", "Info" );
	AddPropertyViewForFog( "Out Height", "Info" );

	AddPropertyViewForFog( "Color", "" );
	AddPropertyViewForFog( "Density", "" );
	AddPropertyViewForFog( "Mode", "" );
	AddPropertyViewForFog( "Alpha Texture", "" );
	AddPropertyViewForFog( "Fog Near", "" );
	AddPropertyViewForFog( "Fog Far", "" );
	AddPropertyViewForFog( "Fog Feather", "" );

	AddPropertyViewForFog( "Target Type", "" );
	AddPropertyViewForFog( "Target Object", "" );
	AddPropertyViewForFog( "Create Target", "Target Object" );

	AddPropertyViewForFog( "Target Near Object", "" );
	AddPropertyViewForFog( "Target Far Object", "" );
	AddPropertyViewForFog( "Create Near/Far Targets", "Target Near Object" );

	AddPropertyViewForFog( "Volume Object", "" );
	AddPropertyViewForFog( "Create Volume", "Volume Object" );
}


///////////////////////////////////////////////////////////////////////////////////////////////////


/************************************************
 *	Constructor.
 ************************************************/
ObjectFilter3dFog::ObjectFilter3dFog( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new CProgramUniformsBase() )
{
	FBClassInit;

	for (int i=0; i<25; ++i)
		mSubroutineIndex[i] = i;
}


/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectFilter3dFog::FBCreate()
{
	FBPropertyPublish(this, Color, "Color", nullptr, nullptr);
	FBPropertyPublish(this, Density, "Density", nullptr, nullptr);
	FBPropertyPublish(this, Mode, "Mode", nullptr, nullptr);
	FBPropertyPublish(this, AlphaTexture, "Alpha Texture", nullptr, nullptr);
	FBPropertyPublish(this, FogNear, "Fog Near", nullptr, nullptr);
	FBPropertyPublish(this, FogFar, "Fog Far", nullptr, nullptr);
	FBPropertyPublish(this, FogFeather, "Fog Feather", nullptr, nullptr);


	FBPropertyPublish(this, TargetType, "Target Type", nullptr, nullptr);
	FBPropertyPublish(this, TargetCenter, "Target Object", nullptr, nullptr);
	FBPropertyPublish(this, CreateTarget, "Create Target", nullptr, SetCreateTarget);
	FBPropertyPublish(this, TargetNear, "Target Near Object", nullptr, nullptr);
	FBPropertyPublish(this, TargetFar, "Target Far Object", nullptr, nullptr);
	FBPropertyPublish(this, CreateNearFarTargets, "Create Near/Far Targets", nullptr, SetCreateNearFarTargets);

	FBPropertyPublish(this, VolumeObject, "Volume Object", nullptr, nullptr);
	FBPropertyPublish(this, CreateVolume, "Create Volume", nullptr, SetCreateVolume);

	FBPropertyPublish(this, UseModelProperties, "Use Model Properties", nullptr, nullptr);
	

	Color = FBColor(1.0, 1.0, 1.0);
	Density = 100.0;

	Mode = eCompositeBlendAdd;

	AlphaTexture.SetSingleConnect(false);
	AlphaTexture.SetFilter( FBTexture::GetInternalClassId() );

	FogNear = 10.0;
	FogFar = 4000.0;
	FogFeather = 10.0;

	TargetType = eFilterFogTargetManual;
	TargetCenter.SetSingleConnect(true);
	TargetCenter.SetFilter( ModelFogTarget::GetInternalClassId() );
	TargetNear.SetSingleConnect(true);
	TargetNear.SetFilter( ModelFogTarget::GetInternalClassId() );
	TargetFar.SetSingleConnect(true);
	TargetFar.SetFilter( ModelFogTarget::GetInternalClassId() );

	VolumeObject.SetSingleConnect(false);
	VolumeObject.SetFilter( ModelFogVolume::GetInternalClassId() );
	UseModelProperties = true;

	return ParentClass::FBCreate();
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/

void ObjectFilter3dFog::FBDestroy()
{
	ParentClass::FBDestroy();
}

void ObjectFilter3dFog::PrepFogData(const CCompositionInfo *pInfo)
{
	FogData &data = mFogData;

	FBCamera *pCamera = ((CCompositionInfo*) pInfo)->GetRenderCamera();

	FBColor vColor;
	vColor = Color;

	float fdensity = 0.01f * (float) Density;
	float ffeather = 0.01f * (float) FogFeather;
	float fnear = (float) FogNear;
	float ffar = (float) FogFar;

	
	FBMatrix modelView, modelViewI;

	if (TargetType != eFilterFogTargetManual)
	{
		( (FBModel*) pCamera)->GetMatrix(modelView);
		modelViewI = modelView.Inverse();
	}

	switch(TargetType)
	{
	case eFilterFogTargetCenter:
		if (TargetCenter.GetCount() )
		{
			FBVector3d lPos;
			ModelFogTarget *pNull = (ModelFogTarget*) TargetCenter.GetAt(0);
			pNull->GetVector(lPos);

			FBTVector p( lPos[0], lPos[1], lPos[2] );
			FBVectorMatrixMult( p, modelViewI, p );
			double dist = p[0];

			if (UseModelProperties)
			{
				fnear = dist - pNull->TargetNear;
				ffar = dist + pNull->TargetFar;
				ffeather = 0.01 * (float)pNull->TargetFeather;
				fdensity = 0.01 * (float)pNull->TargetDensity;
				vColor = pNull->Color;
			}
			else
			{
				fnear = dist - fnear;
				ffar = dist + ffar;
			}
		}
		break;
	case eFilterFogTargetNearFar:
		if (TargetNear.GetCount() )
		{
			FBVector3d lPos1;
			ModelFogTarget *pNull = (ModelFogTarget*) TargetNear.GetAt(0);
			pNull->GetVector(lPos1);
			
			FBTVector p;
			double distNear=0.0;

			FBVectorMatrixMult( p, modelViewI, FBTVector( lPos1[0], lPos1[1], lPos1[2] ) );
			distNear = p[0];
			
			fnear = distNear;

			if (UseModelProperties)
			{
				ffeather = 0.01 * (float)pNull->TargetFeather;
				fdensity = 0.01 * (float)pNull->TargetDensity;
				vColor = pNull->Color;
			}

			//!! we have changed prop value here
			//FogNear = distNear;
		}
		if (TargetFar.GetCount() )
		{
			FBVector3d lPos2;
			
			ModelFogTarget *pNull = (ModelFogTarget*) TargetFar.GetAt(0);
			pNull->GetVector(lPos2);

			FBTVector p;
			double distFar=0.0;

			FBVectorMatrixMult( p, modelViewI, FBTVector( lPos2[0], lPos2[1], lPos2[2] ) );
			distFar = p[0];

			ffar = distFar;
			//!! we have changed prop value here
			//FogFar = distFar;
		}

		break;
	}

	for (int i=0; i<3; ++i)
		data.color[i] = (float) vColor[i];

	data.density = fdensity;
	data.feather = ffeather;
	data.fnear = fnear;
	data.ffar = ffar;

	// frustum information for the 3d position reconstruction
	vec3 points[8];

	int renderWidth = pInfo->GetRenderWidth() - 2 * pInfo->GetRenderBorderX();
	int renderHeight = pInfo->GetRenderHeight() - 2 * pInfo->GetRenderBorderY();

	ComputeCameraFrustumPoints( renderWidth, renderHeight, pCamera, 
		pCamera->FarPlaneDistance, pCamera->NearPlaneDistance, pCamera->FieldOfView, points );
	/*
	data.viewOrig[0] = vec4(points[0], 1.0);
	data.viewDir[0] = vec4(points[4], 1.0);
	
	data.viewOrig[1] = vec4(points[3], 1.0);
	data.viewDir[1] = vec4(points[7], 1.0);
	
	data.viewOrig[2] = vec4(points[2], 1.0);
	data.viewDir[2] = vec4(points[6], 1.0);

	data.viewOrig[3] = vec4(points[1], 1.0);
	data.viewDir[3] = vec4(points[5], 1.0);
	*/
	data.viewOrig[0] = vec4(points[0], 1.0);
	data.viewDir[0] = vec4(points[4], 1.0);
	
	data.viewOrig[1] = vec4(points[1], 1.0);
	data.viewDir[1] = vec4(points[5], 1.0);
	
	data.viewOrig[2] = vec4(points[2], 1.0);
	data.viewDir[2] = vec4(points[6], 1.0);

	data.viewOrig[3] = vec4(points[3], 1.0);
	data.viewDir[3] = vec4(points[7], 1.0);

	// for volume computing
	data.volumeInfo.x = (float) ComputeNumberOfVolumeModels();

	mGPUFogBuffer.UpdateData( sizeof(FogData), 1, &mFogData );
}

void ObjectFilter3dFog::PrepVolumeData(VolumeData &data, FBModel *pVolumeModel, const bool useVolumeProperties)
{
	FBMatrix model, invModel;
	
	ModelFogVolume *pModel = (ModelFogVolume*) pVolumeModel;
	pModel->GetMatrix(model);
	FBMatrixInverse( invModel, model );

	for (int i=0; i<16; ++i)
	{
		data.invVolumeMatrix.mat_array[i] = (float) invModel[i];
	}

	FBVector3d pMin, pMax;
	pModel->GetBoundingBox( pMin, pMax );

	for (int i=0; i<3; ++i)
	{
		data.volumeMin[i] = pMin[i];
		data.volumeMax[i] = pMax[i];
	}
	data.volumeMin[3] = data.volumeMax[3] = 1.0f;

	//
	double dColor[4] = {1.0, 1.0, 1.0, 1.0};
	Color.GetData( dColor, sizeof(double) * 3 );

	data.feather = 0.01f * (float) FogFeather;
	data.density = 0.01f * (float) Density;

	if (useVolumeProperties)
	{
		pModel->Color.GetData( dColor, sizeof(double) * 3 );
		data.feather = 0.01f * pModel->VolumeFeather;
		data.density = 0.01f * pModel->VolumeDensity;
		data.blendMode = (float) pModel->BlendMode.AsInt();
	}

	for (int i=0; i<4; ++i)
		data.color[i] = (float) dColor[i];
}

void ObjectFilter3dFog::PrepAllVolumes(FBCamera *pCamera)
{
	SortVolumeModels(pCamera);

	size_t count = mVolumesSortVector.size();
	if (count == 0)
		return;

	mVolumeData.resize(count);
	bool useVolumeProperties = UseModelProperties;

	for (size_t i=0; i<count; ++i)
	{
		PrepVolumeData(mVolumeData[i], mVolumesSortVector[i].first, useVolumeProperties);
	}

	mGPUVolumeBuffer.UpdateData( sizeof(VolumeData), count, mVolumeData.data() );
}

/*
void ObjectFilter3dFog::Render()
{
	ObjectCompositeBase::Render();

	//
	if (mSettings.Ok() == false)
		mSettings = GetGraphicsGlobalSettingsPtr("MoPlugsSettings");

	if (Components.GetCount() != 1)
		return;

	//
	//
	//

	const bool useMask = UseCompositeMask;
	bool useVolumeObject = false;
	
	if (TargetType == eFilterFogTargetVolume && VolumeObject.GetCount() > 0)
	{
		int totalCount = 0;
		for (int i=0, count=VolumeObject.GetCount(); i<count; ++i)
		{
			ModelVolumeFog *pVolume = (ModelVolumeFog*) VolumeObject.GetAt(i);
			
			if (pVolume->Enabled)
				totalCount = totalCount + 1;
		}

		useVolumeObject = (totalCount > 0);

		mVolumesSortVector.resize(totalCount);

		totalCount = 0;
		for (int i=0, count=VolumeObject.GetCount(); i<count; ++i)
		{
			ModelVolumeFog *pVolume = (ModelVolumeFog*) VolumeObject.GetAt(i);
			
			if (pVolume->Enabled)
			{
				mVolumesSortVector[totalCount].first = pVolume;
				totalCount = totalCount + 1;
			}
		}
	}
	
	//
	FBCamera *lCamera = mSystem.Scene->Renderer->CurrentCamera;
	if ( lCamera && FBIS(lCamera, FBCameraSwitcher) )
		lCamera = ((FBCameraSwitcher*) lCamera)->CurrentCamera;

	if (nullptr == lCamera)
		return;

	FBVector3d cameraPos;
	lCamera->GetVector(cameraPos);

	// check all needed blend modes

	if (useVolumeObject && UseVolumeProperties)
	{
		int totalCount =0;
		for (int i=0, count=VolumeObject.GetCount(); i<count; ++i)
		{
			ModelVolumeFog *pVolume = (ModelVolumeFog*) VolumeObject.GetAt(i);
			
			if (pVolume->Enabled)
			{
				const ECompositeBlendType blendType = (ECompositeBlendType) pVolume->VolumeBlendMode;
				const ECompositeShader shaderType = (ECompositeShader) (eCompositeShaderFogNormal + (int) blendType);

				if (Active)
				{
					Active = mShaderMan.CheckAndLoadShader( shaderType, useMask );

					double dist = 0.0;
					FBVector3d pos;
					pVolume->GetVector(pos);
					dist = VectorLength(cameraPos, pos);

					mVolumesSortVector[totalCount].second = dist;
					totalCount = totalCount + 1;
				}
			}
		}

		// sort volumes by dist value
		auto volumeCmp = [](std::pair<FBModel*,double> const & pairA, std::pair<FBModel*,double> const & pairB) 
		{ 
			 return pairA.second < pairB.second;
		};

		std::sort( begin(mVolumesSortVector), end(mVolumesSortVector), volumeCmp );
	}
	else
	{
		const ECompositeBlendType blendType = Mode;
		const ECompositeShader shaderType = (ECompositeShader) (eCompositeShaderFogNormal + (int) blendType);

		if (Active)
			Active = mShaderMan.CheckAndLoadShader( shaderType, useMask );
	}
	
	// TODO: check all volume models blend types !!
	// TODO: we need two framebuffers for multipass volume rendering

	if (Active)
		Active = InitFrameBufferBase(renderWidth, renderHeight);
	if (useVolumeObject && Active)
		Active = InitFrameBuffer(renderWidth, renderHeight, mBufferSecond);

	if (Active == false) 
		return;

	// should be two input connections
	
	GLuint texId = ((ObjectCompositeBase*) Components[0])->getColorTextureId();
	GLuint depthId = FindDepthId(this);

	if (texId == 0 || depthId == 0)
		return;

	//
	// DONE: prepare depth information for volume fog

	ECompositeBlendType blendType = Mode;

	if (useVolumeObject && UseVolumeProperties)
	{
		for (int i=0, count=VolumeObject.GetCount(); i<count; ++i)
		{
			ModelVolumeFog *pVolume = (ModelVolumeFog*) VolumeObject.GetAt(i);
			
			if (pVolume->Enabled)
			{
				blendType = pVolume->VolumeBlendMode;

				break;
			}
		}
	}

	ECompositeShader shaderType = (ECompositeShader) (eCompositeShaderFogNormal + (int) blendType);

	//
	// main fog composition

	//
	glViewport(0, 0, renderWidth, renderHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, renderWidth, 0.0, renderHeight, -1.0, 1.0); 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	PrepQuad(lCamera);

	//
	//
	if (useVolumeObject)
	{
		GLuint lastTexId = texId;

		mCurrentBufferIndex = 0;
		FrameBuffer *buffers[2] = { mBuffer, mBufferSecond };

		bool firstRun = true;

		for (auto iter=begin(mVolumesSortVector); iter!=end(mVolumesSortVector); ++iter)
		{
			ModelVolumeFog *pVolume = (ModelVolumeFog*) iter->first;
			
			if (firstRun == true)
			{
				firstRun = false;
			}
			else
			{
				// swap
				mCurrentBufferIndex = (mCurrentBufferIndex==0) ? 1 : 0;
			}

			buffers[mCurrentBufferIndex]->Bind();

			if (UseVolumeProperties)
			{
				blendType = pVolume->VolumeBlendMode;
			}
			shaderType = (ECompositeShader) (eCompositeShaderFogNormal + (int) blendType);
			
			mShaderMan.Bind(shaderType, useMask);
		
			if (useMask)
			{
				mShaderMan.SetMaskUniforms( InvertCompositeMask );
				BindCompositeMask();
			}

			//
			SetUpAlphaUniform();

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthId);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, lastTexId);

			SetUpObjectUniforms(lCamera, useVolumeObject);
			CHECK_GL_ERROR_MOBU();

			SetUpVolumeUniforms(lCamera, pVolume);
			CHECK_GL_ERROR_MOBU();

			// draw textured rects with shadow maps
			DrawQuad();
	
			if (useMask)
				UnBindCompositeMask();

			mShaderMan.UnBind();

			buffers[mCurrentBufferIndex]->UnBind();

			glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

			lastTexId = buffers[mCurrentBufferIndex]->GetColorObject();
		}
	}
	else
	{
		mCurrentBufferIndex = 0;

		mBuffer->Bind();

		mShaderMan.Bind(shaderType, useMask);
		
		if (useMask)
		{
			mShaderMan.SetMaskUniforms( InvertCompositeMask );
			BindCompositeMask();
		}

		//
		SetUpAlphaUniform();

	
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texId);

		SetUpObjectUniforms(lCamera, useVolumeObject);
		CHECK_GL_ERROR_MOBU();

		//-------------------------

		// draw textured rects with shadow map
		DrawQuad();
	
		if (useMask)
			UnBindCompositeMask();

		mShaderMan.UnBind();
		mBuffer->UnBind();
	}

	FinishQuad();
	CHECK_GL_ERROR_MOBU();
}


void ObjectFilter3dFog::SetUpAlphaUniform()
{
	if (AlphaTexture.GetCount() )
	{
		FBTexture *pTexture = (FBTexture*) AlphaTexture.GetAt(0);
		GLuint alphaId = pTexture->TextureOGLId;
		if (alphaId == 0)
		{
			pTexture->OGLInit();
			alphaId = pTexture->TextureOGLId;
		}

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, alphaId);

		mShaderMan.SetFogUniforms( 1 );
	}
	else
	{
		mShaderMan.SetFogUniforms( 0 );
	}
}

void ObjectFilter3dFog::SetUpObjectUniforms(FBCamera *pCamera, const bool useVolumeObject)
{
	double vColor[3];
	Color.GetData( vColor, sizeof(double) * 3 );

	float border = 0.01 * (float) FogBorder;
	float fogNear = (float) FogNear;
	float fogFar = (float) FogFar;

	float lznear = 10.0f;
	float lzfar = 4000.0f;

	lznear = (float) pCamera->NearPlaneDistance;
	lzfar = (float) pCamera->FarPlaneDistance;
	
	FBMatrix modelView, modelViewI;

	if (TargetType != eFilterFogTargetManual)
	{
		( (FBModel*) pCamera)->GetMatrix(modelView);
		modelViewI = modelView.Inverse();
	}

	switch(TargetType)
	{
	case eFilterFogTargetCenter:
		if (TargetCenter.GetCount() )
		{
			FBVector3d lPos;
			FBModelNull *pNull = (FBModelNull*) TargetCenter.GetAt(0);
			pNull->GetVector(lPos);

			FBTVector p( lPos[0], lPos[1], lPos[2] );
			FBVectorMatrixMult( p, modelViewI, p );
			double dist = p[0];

			fogNear = dist - fogNear;
			fogFar = dist + fogFar;
		}
		break;
	case eFilterFogTargetNearFar:
		if (TargetNear.GetCount() )
		{
			FBVector3d lPos1;
			FBModelNull *pNull = (FBModelNull*) TargetNear.GetAt(0);
			pNull->GetVector(lPos1);
			
			FBTVector p;
			double distNear=0.0;

			FBVectorMatrixMult( p, modelViewI, FBTVector( lPos1[0], lPos1[1], lPos1[2] ) );
			distNear = p[0];
			
			fogNear = distNear;
			FogNear = distNear;
		}
		if (TargetFar.GetCount() )
		{
			FBVector3d lPos2;
			
			FBModelNull *pNull = (FBModelNull*) TargetFar.GetAt(0);
			pNull->GetVector(lPos2);

			FBTVector p;
			double distFar=0.0;

			FBVectorMatrixMult( p, modelViewI, FBTVector( lPos2[0], lPos2[1], lPos2[2] ) );
			distFar = p[0];

			fogFar = distFar;
			FogFar = distFar;
		}

		break;
	}

	float fLogarithmicDepth = 0.0f;
	float fHighQualityDepth = 0.0f;
	if (mSettings.Ok() )
	{
		fLogarithmicDepth = (mSettings->LogarithmicDepth) ? 1.0f : 0.0f;
		fHighQualityDepth = (mSettings->RenderDepthNV) ? 1.0f : 0.0f;
	}

	float volumeF = (useVolumeObject) ? 1.0f : 0.0f;
	float fDensity = 0.01f * Density;

	mShaderMan.SetFogUniforms( (float) vColor[0], (float) vColor[1], (float) vColor[2],
								fDensity,
								lznear,
								lzfar,
								fogNear,
								fogFar,
								border,
								fLogarithmicDepth,
								fHighQualityDepth,
								volumeF );
}

void ObjectFilter3dFog::SetUpVolumeUniforms(FBCamera *pCamera, FBModel *_model)
{
	FBMatrix model, invModel;
	
	ModelVolumeFog *pModel = (ModelVolumeFog*) _model;
	pModel->GetMatrix(model);
	FBMatrixInverse( invModel, model );

	float finvmodel[16];
	float fmin[4], fmax[4];

	for (int i=0; i<16; ++i)
	{
		finvmodel[i] = invModel[i];
	}

	FBVector3d pMin, pMax;
	pModel->GetBoundingBox( pMin, pMax );

	for (int i=0; i<3; ++i)
	{
		fmin[i] = pMin[i];
		fmax[i] = pMax[i];
	}
	fmin[3] = fmax[3] = 1.0f;

	//
	double dColor[4] = {1.0, 1.0, 1.0, 1.0};
	Color.GetData( dColor, sizeof(double) * 3 );

	float feather = 0.01f * (float) FogBorder;
	float fDensity = 0.01f * Density;

	if (UseVolumeProperties)
	{
		pModel->VolumeColor.GetData( dColor, sizeof(double) * 4 );
		feather = 0.01f * pModel->VolumeFeather;
		fDensity = 0.01f * pModel->VolumeDensity;
	}

	float fcolor[4];
	for (int i=0; i<4; ++i)
		fcolor[i] = (float) dColor[i];

	mShaderMan.SetFogUniforms( finvmodel, fmin, fmax, fcolor, fDensity, feather );
}
*/

// DONE: check alpha texture and volume connections to query a propriate shader program !

bool ObjectFilter3dFog::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if (pAction == kFBCandidated && (pThis == &Mode || pThis == &TargetType) )
	{
		// query a new program with updated define list
		mProgram = nullptr;
	}
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ObjectFilter3dFog::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) 
{
	if (pThis == &TargetCenter)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	else if (pThis == &TargetNear)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	else if (pThis == &TargetFar)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	else if (pThis == &VolumeObject)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
			// update program defines
			mProgram = nullptr;
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
			mProgram = nullptr;
		}
	}
	else if (pThis == &AlphaTexture)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
			// update program defines
			mProgram = nullptr;
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
			mProgram = nullptr;
		}
	}

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void ObjectFilter3dFog::SetCreateTarget(HIObject object, bool value)
{
	ObjectFilter3dFog *pBase = FBCast<ObjectFilter3dFog>(object);
	if (pBase && value) 
	{
		pBase->DoCreateTarget();
	}
}

void ObjectFilter3dFog::SetCreateNearFarTargets(HIObject object, bool value)
{
	ObjectFilter3dFog *pBase = FBCast<ObjectFilter3dFog>(object);
	if (pBase && value) 
	{
		pBase->DoCreateNearFarTargets();
	}
}

void ObjectFilter3dFog::SetCreateVolume(HIObject object, bool value)
{
	ObjectFilter3dFog *pBase = FBCast<ObjectFilter3dFog>(object);
	if (pBase && value) 
	{
		pBase->DoCreateVolume();
	}
}

void ObjectFilter3dFog::DoCreateTarget()
{
	// DONE:
	ModelFogTarget *pTarget = (ModelFogTarget*) FBCreateObject( MARKER__ASSETPATH, FOGTARGET__DESCSTR, FBString(Name, " Target") );  // new ModelFogTarget( FBString(Name, " Target") );
	if (pTarget == nullptr)
		return;

	pTarget->Show = true;
	pTarget->Visibility = true;

	FBCamera *pCurrCamera = mSystem.Renderer->CurrentCamera;
	if (pCurrCamera)
	{
		if (FBIS(pCurrCamera, FBCameraSwitcher) )
			pCurrCamera = ( (FBCameraSwitcher*) pCurrCamera)->CurrentCamera;

		vec3 points[8];
		ComputeCameraFrustumPoints(pCurrCamera->CameraViewportWidth, pCurrCamera->CameraViewportHeight, pCurrCamera, 
			pCurrCamera->FarPlaneDistance, pCurrCamera->NearPlaneDistance, pCurrCamera->FieldOfView, points);

		vec3 nearCenter, farCenter;
		nearCenter = 0.25 * (points[0] + points[1] + points[2] + points[3]);
		farCenter = 0.25 * (points[4] + points[5] + points[6] + points[7]);

		vec3 point = nearCenter + 0.1f * (farCenter - nearCenter);
		pTarget->SetVector( FBVector3d((double)point.x, (double)point.y, (double)point.z) );
	}

	this->TargetCenter.Clear();
	FBConnect(pTarget, &this->TargetCenter);

	this->TargetType = eFilterFogTargetCenter;
}

void ObjectFilter3dFog::DoCreateNearFarTargets()
{
	// DONE:
	ModelFogTarget *pTargetNear = (ModelFogTarget*) FBCreateObject( MARKER__ASSETPATH, FOGTARGET__DESCSTR, FBString(Name, " TargetNear") ); 
	ModelFogTarget *pTargetFar = (ModelFogTarget*) FBCreateObject( MARKER__ASSETPATH, FOGTARGET__DESCSTR, FBString(Name, " TargetFar") );
	if (pTargetNear == nullptr || pTargetFar == nullptr)
		return;

	pTargetNear->Show = true;
	pTargetNear->Visibility = true;
	pTargetFar->Show = true;
	pTargetFar->Visibility = true;

	FBCamera *pCurrCamera = mSystem.Renderer->CurrentCamera;
	if (pCurrCamera)
	{
		if (FBIS(pCurrCamera, FBCameraSwitcher) )
			pCurrCamera = ( (FBCameraSwitcher*) pCurrCamera)->CurrentCamera;

		vec3 points[8];
		ComputeCameraFrustumPoints(pCurrCamera->CameraViewportWidth, pCurrCamera->CameraViewportHeight, pCurrCamera, 
			pCurrCamera->FarPlaneDistance, pCurrCamera->NearPlaneDistance, pCurrCamera->FieldOfView, points);

		vec3 nearCenter, farCenter;
		nearCenter = 0.25 * (points[0] + points[1] + points[2] + points[3]);
		farCenter = 0.25 * (points[4] + points[5] + points[6] + points[7]);

		vec3 point = nearCenter + 0.1f * (farCenter - nearCenter);
		pTargetNear->SetVector( FBVector3d((double)point.x, (double)point.y, (double)point.z) );
		
		point = nearCenter + 0.2f * (farCenter - nearCenter);
		pTargetFar->SetVector( FBVector3d((double)point.x, (double)point.y, (double)point.z) );
	}

	this->TargetNear.Clear();
	FBConnect(pTargetNear, &this->TargetNear);
	this->TargetFar.Clear();
	FBConnect(pTargetFar, &this->TargetFar);

	this->TargetType = eFilterFogTargetNearFar;
}

void ObjectFilter3dFog::DoCreateVolume()
{
	// DONE:
	ModelFogVolume *pVolume = (ModelFogVolume*) FBCreateObject( MARKER__ASSETPATH, FOGVOLUME__DESCSTR, FBString(Name, " Volume") );
	if (pVolume == nullptr)
		return;

	pVolume->Show = true;
	pVolume->Visibility = true;
	pVolume->Scaling = FBVector3d(10.0, 10.0, 10.0);

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

		//pVolume->SetVector( FBVector3d((double)point.x, (double)point.y, (double)point.z) );
		pVolume->SetVector(v);
	}

	FBConnect(pVolume, &this->VolumeObject);
	this->TargetType = eFilterFogTargetVolume;
}

const int ObjectFilter3dFog::ComputeNumberOfVolumeModels()
{
	int totalCount = 0;

	if (TargetType == eFilterFogTargetVolume && VolumeObject.GetCount() > 0)
	{
		for (int i=0, count=VolumeObject.GetCount(); i<count; ++i)
		{
			ModelFogVolume *pVolume = (ModelFogVolume*) VolumeObject.GetAt(i);
			
			if (pVolume->Enabled)
				totalCount = totalCount + 1;
		}
	}
	return totalCount;
}

void ObjectFilter3dFog::SortVolumeModels(FBCamera *pCamera)
{
	bool useVolumeObject = false;

	if (TargetType == eFilterFogTargetVolume && VolumeObject.GetCount() > 0)
	{
		int totalCount = 0;
		for (int i=0, count=VolumeObject.GetCount(); i<count; ++i)
		{
			ModelFogVolume *pVolume = (ModelFogVolume*) VolumeObject.GetAt(i);
			
			if (pVolume->Enabled)
				totalCount = totalCount + 1;
		}

		useVolumeObject = (totalCount > 0);

		mVolumesSortVector.resize(totalCount);

		totalCount = 0;
		for (int i=0, count=VolumeObject.GetCount(); i<count; ++i)
		{
			ModelFogVolume *pVolume = (ModelFogVolume*) VolumeObject.GetAt(i);
			
			if (pVolume->Enabled)
			{
				mVolumesSortVector[totalCount].first = pVolume;
				totalCount = totalCount + 1;
			}
		}
	}

	//
	if (nullptr == pCamera)
		return;

	FBVector3d cameraPos;
	pCamera->GetVector(cameraPos);

	// check all needed blend modes

	if (useVolumeObject)
	{
		int totalCount =0;
		for (int i=0, count=VolumeObject.GetCount(); i<count; ++i)
		{
			ModelFogVolume *pVolume = (ModelFogVolume*) VolumeObject.GetAt(i);
			
			if (pVolume->Enabled)
			{
				if (Active)
				{
					double dist = 0.0;
					FBVector3d pos;
					pVolume->GetVector(pos);
					dist = VectorLength(cameraPos, pos);

					mVolumesSortVector[totalCount].second = dist;
					totalCount = totalCount + 1;
				}
			}
		}

		// sort volumes by dist value
		auto volumeCmp = [](std::pair<FBModel*,double> const & pairA, std::pair<FBModel*,double> const & pairB) 
		{ 
			 return pairA.second < pairB.second;
		};

		std::sort( begin(mVolumesSortVector), end(mVolumesSortVector), volumeCmp );
	}
}