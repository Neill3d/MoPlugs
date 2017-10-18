
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectDOFFilter.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_objectDOFFilter.h"

FBClassImplementation2(ObjectFilter3dDOF)
FBUserObjectImplement(ObjectFilter3dDOF, "Composition 3d DOF", EFFECT_ICON);					//Register UserObject class
FBElementClassImplementationFilter(ObjectFilter3dDOF, FILTER3DDOF__ASSETNAME, EFFECT_ICON);				//Register to the asset system


////////////////////////////////////////////////////////////////////////////////////////////////// PROCS

void AddPropertyViewForDOF(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTER3DDOF__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilter3dDOF::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForDOF( "Active", "" );
	AddPropertyViewForDOF( "Reload Shader", "" );
	AddPropertyViewForDOF( "Opacity", "" );

	AddPropertyViewForDOF( "", "Masking", true );
	AddPropertyViewForDOF( "Use Composite Mask", "Masking" );
	AddPropertyViewForDOF( "Select Composite Mask", "Masking" );
	AddPropertyViewForDOF( "Invert Composite Mask", "Masking" );
	AddPropertyViewForDOF( "Custom Mask", "Masking" );

	AddPropertyViewForDOF( "", "Info", true );
	AddPropertyViewForDOF( "Out Width", "Info" );
	AddPropertyViewForDOF( "Out Height", "Info" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////


/************************************************
 *	Constructor.
 ************************************************/
ObjectFilter3dDOF::ObjectFilter3dDOF( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new CProgramUniformsBase() )
{
	FBClassInit;
}


/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectFilter3dDOF::FBCreate()
{
	FBPropertyPublish(this, UseCameraDOFProperties, "Use Camera DOF Properties", nullptr, nullptr);

	FBPropertyPublish(this, ResetAction, "Reset", nullptr, SetResetAction);

	FBPropertyPublish(this, DebugBlurValue, "Debug Blur Value", nullptr, nullptr);

	FBPropertyPublish(this, FocalDistance, "Focal Distance", nullptr, nullptr);
	FBPropertyPublish(this, FocalRange, "Focal Range", nullptr, nullptr);
	FBPropertyPublish(this, FStop, "F-Stop", nullptr, nullptr);

	FBPropertyPublish(this, AutoFocus, "Auto Focus", nullptr, nullptr);
	FBPropertyPublish(this, FocusObject, "Focus object", nullptr, nullptr);
	FBPropertyPublish(this, SelectFocusObject, "Select Focus object", nullptr, SetSelectFocusObject);
	
	FBPropertyPublish(this, FastPreview, "Fast Preview", nullptr, SetFastPreview);
	FBPropertyPublish(this, PreviewQuality, "Preview Quality", nullptr, nullptr);
	FBPropertyPublish(this, PreviewBlurAmount, "Preview Blur Amount", nullptr, nullptr);

	FBPropertyPublish(this, BlurForeground, "Blur Foreground", nullptr, nullptr);
	/*
	FBPropertyPublish(this, ManualFocus, "Manual mode", nullptr, nullptr);
	FBPropertyPublish(this, ManualNear, "Manual Near Distance", nullptr, nullptr);
	FBPropertyPublish(this, ManualNearFalloff, "Manual Near Falloff", nullptr, nullptr);
	FBPropertyPublish(this, ManualFar, "Manual Far Distance", nullptr, nullptr);
	FBPropertyPublish(this, ManualFarFalloff, "Manual Far Falloff", nullptr, nullptr);
	*/
	FBPropertyPublish(this, Vignetting, "Vignetting", nullptr, nullptr);

	FBPropertyPublish(this, Samples, "Samples", nullptr, nullptr);
	FBPropertyPublish(this, Rings, "Ring count", nullptr, nullptr);

	FBPropertyPublish(this, CoC, "Circle of confusion", nullptr, nullptr);

	FBPropertyPublish(this, Threshold, "Highlight Threshold", nullptr, nullptr);
	FBPropertyPublish(this, Gain, "Highlight Gain", nullptr, nullptr);

	FBPropertyPublish(this, Bias, "Bokeh Bias", nullptr, nullptr);
	FBPropertyPublish(this, Fringe, "Bokeh Fringe", nullptr, nullptr);

	FBPropertyPublish(this, Noise, "Noise", nullptr, nullptr);

	FBPropertyPublish(this, Pentagon, "Pentagon", nullptr, nullptr);
	FBPropertyPublish(this, PentagonFeather, "Pentagon feather", nullptr, nullptr);

	
	FocusObject.SetSingleConnect(true);
	FocusObject.SetFilter( FBModelNull::GetInternalClassId() );

	Samples.SetMinMax(1, 12, true, true);
	Rings.SetMinMax(1, 32, true, true);
	PentagonFeather.SetMinMax(0.0, 1.0, true, true);
	
	//
	DoReset();

	ShaderBlurSystem::Create(mBlurCache);

	return ParentClass::FBCreate();
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/

void ObjectFilter3dDOF::FBDestroy()
{
	ShaderBlurSystem::Destroy(mBlurCache);
	ParentClass::FBDestroy();
}

void ObjectFilter3dDOF::PrepData(const CCompositionInfo *pInfo)
{
	bool autofocus = false;
	AutoFocus.GetData( &autofocus, sizeof(bool) );

	FBCamera *pCamera = ((CCompositionInfo*)pInfo)->GetRenderCamera();

	if (UseCameraDOFProperties && pInfo->UseCameraDOFSettings() )
	{
		mData.focalDepth = (float) pInfo->FocusSpecificDistance();
		mData.focalLength = (float) pInfo->FocusAngle();

		FBModel *pInterest = ((CCompositionInfo*)pInfo)->GetRenderCameraInterest();
		if (pInfo->FocusDistanceSource() == kFBFocusDistanceCameraInterest && pInterest != nullptr)
		{
			FBMatrix modelView, modelViewI;

			( (FBModel*) ((CCompositionInfo*)pInfo)->GetRenderCamera())->GetMatrix(modelView);
			modelViewI = modelView.Inverse();

			FBVector3d lPos;
			pInterest->GetVector(lPos);

			FBTVector p( lPos[0], lPos[1], lPos[2] );
			FBVectorMatrixMult( p, modelViewI, p );
			double dist = p[0];

			// Dont write to property
			// FocalDistance = dist;
			mData.focalDepth = dist;
		}
	}
	else
	{
		mData.focalDepth = FocalDistance;
		mData.focalLength = FocalRange;

		if ( autofocus && FocusObject.GetCount() > 0)
		{
			FBMatrix modelView, modelViewI;

			( (FBModel*) ((CCompositionInfo*)pInfo)->GetRenderCamera())->GetMatrix(modelView);
			modelViewI = modelView.Inverse();
	
			FBVector3d lPos;
			FBModel *pModel = (FBModel*) FocusObject.GetAt(0);
			pModel->GetVector(lPos);

			FBTVector p( lPos[0], lPos[1], lPos[2] );
			FBVectorMatrixMult( p, modelViewI, p );
			double dist = p[0];

			// Dont write to property
			// FocalDistance = dist;
			mData.focalDepth = dist;
		}
	}

	mData.fstop = FStop;
	mData.samples = Samples;
	mData.rings = Rings;

	mData.ndofstart = 1.0;
	mData.ndofdist = 2.0;
	mData.fdofstart = 1.0;
	mData.fdofdist = 3.0;

	mData.CoC = 0.01f * (float) CoC;
	mData.threshold = 0.01f * (float) Threshold;
	mData.focus = vec2(0.5f, 0.5f);

	mData.gain = 0.01f * (float) Gain;
	mData.bias = 0.01f * (float) Bias;
	mData.fringe = 0.01f * (float) Fringe;
	mData.feather = 0.01f * (float) PentagonFeather;

	mData.debugBlurValue = (DebugBlurValue.AsInt() > 0) ? 1.0f : 0.0f;

	//
	//

	mDataBuffer.UpdateData( sizeof(Data), 1, &mData );
}

bool ObjectFilter3dDOF::GenBlurData(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId)
{
	mBlurData.horzFactor = 0.01 * PreviewBlurAmount;
	mBlurData.vertFactor = 0.01 * PreviewBlurAmount;

	mBlurData.w = prInfo.GetWidth();
	mBlurData.h = prInfo.GetHeight();

	if (pInfo->GetPreviewSizeFactor() >= 1.0f)
	{
		switch(PreviewQuality)
		{
		case eCompositionQualityPreview2:
			mBlurData.w = mBlurData.w / 2;
			mBlurData.h = mBlurData.h / 2;
			break;
		case eCompositionQualityPreview4:
			mBlurData.w = mBlurData.w / 4;
			mBlurData.h = mBlurData.h / 4;
			break;
		}
	}

	mBlurTexture.ReSizeLayersData( mBlurData.w, mBlurData.h );

	mBlurData.opacity = 1.0f;

	if (false == ShaderBlurSystem::CheckIfShaderIsReady(pInfo, mBlurData, mBlurCache) )
		return false;

	mBlurData.sourceTexId = sourceTexId;
	mBlurData.dstTexId = mBlurTexture.QueryATextureBuffer();

	if (false == ShaderBlurSystem::ApplyFilter(pInfo, mBlurData, stats, mBlurCache) )
		return false;

	return true;
}

bool ObjectFilter3dDOF::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if (pAction == kFBCandidated && (pThis == &Noise || pThis == &Vignetting
		|| pThis == &BlurForeground || pThis == &Pentagon) )
	{
		// query a new program with updated define list
		mProgram = nullptr;
	}
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ObjectFilter3dDOF::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) 
{
	if (pThis == &FocusObject)
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

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void ObjectFilter3dDOF::SetResetAction(HIObject object, bool value)
{
	ObjectFilter3dDOF *pBase = FBCast<ObjectFilter3dDOF>(object);
	if (pBase && value) 
	{
		pBase->DoReset();
	}
}

void ObjectFilter3dDOF::SetSelectFocusObject(HIObject object, bool value)
{
	ObjectFilter3dDOF *pBase = FBCast<ObjectFilter3dDOF>(object);
	if (pBase && value) 
	{
		pBase->DoSelectFocusObject();
	}
}

void ObjectFilter3dDOF::SetFastPreview(HIObject object, bool value)
{
	ObjectFilter3dDOF *pBase = FBCast<ObjectFilter3dDOF>(object);
	if (pBase) 
	{
		pBase->ChangeGlobalSettings();
		pBase->FastPreview.SetPropertyValue(value);
	}
}

void ObjectFilter3dDOF::DoReset()
{
	UseCameraDOFProperties = true;
	DebugBlurValue = false;

	FocalDistance = 40.0;
	FocalRange = 4.0;
	FStop = 0.5;

	BlurForeground = true;
	/*
	ManualFocus = false;
	ManualNear = 1.0;
	ManualNearFalloff = 2.0;
	ManualFar = 1.0;
	ManualFarFalloff = 3.0;
	*/
	AutoFocus = false;

	FastPreview = false;
	PreviewQuality = eCompositionQualityPreview2;
	PreviewBlurAmount = 100.0;

	Samples = 3;
	Rings = 3;
	
	CoC = 3.0;
	Threshold = 50.0;
	Gain = 200.0;
	Bias = 50.0;
	Fringe = 70.0;
	Noise = true;

	Pentagon = false;
	PentagonFeather = 40.0;
}

void ObjectFilter3dDOF::DoSelectFocusObject()
{
	// TODO: crashes a system
	/*
	if (FocusObject.GetCount() > 0)
	{
		FBBeginChangeAllModels();

		FBScene *pScene = mSystem.Scene;

		for (int i=0; i<pScene->Components.GetCount(); ++i)
		{
			FBComponent *pComp = pScene->Components[i];
			if ( pComp->HasObjectFlags(kFBFlagSystem) )
				continue;

			pComp->Selected = false;
		}

		FocusObject.GetAt(0)->Selected = true;

		FBEndChangeAllModels();
	}
	*/
}