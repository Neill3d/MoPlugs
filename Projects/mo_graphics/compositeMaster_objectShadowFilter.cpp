
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectShadowFilter.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_objectShadowFilter.h"
#include "compositemaster_common.h"
#include "IO\FileUtils.h"
#include "shared_misc.h"
#include "graphics\CheckGLError_MOBU.h"

#include "algorithm\nv_math.h"

#include "model_3d_shadow.h"

#include <algorithm>

FBClassImplementation2(ObjectFilter3dShadow)
FBUserObjectImplement(ObjectFilter3dShadow, "Composition 3d Shadow", EFFECT_ICON);					//Register UserObject class
FBElementClassImplementationFilter(ObjectFilter3dShadow, FILTER3DSHADOW__ASSETNAME, EFFECT_ICON);				//Register to the asset system


////////////////////////////////////////////////////////////////////////////////////////////////// PROCS

void AddPropertyViewForShadow(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTER3DSHADOW__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilter3dShadow::AddPropertiesToPropertyViewManager()
{
	/*
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

	AddPropertyViewForFog( "Zone Object", "" );
	AddPropertyViewForFog( "Create Zone", "Zone Object" );
	*/
}


///////////////////////////////////////////////////////////////////////////////////////////////////


/************************************************
 *	Constructor.
 ************************************************/
ObjectFilter3dShadow::ObjectFilter3dShadow( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new CProgramUniformsBase() )
{
	FBClassInit;

	for (int i=0; i<25; ++i)
		mSubroutineIndex[i] = i;
}


/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectFilter3dShadow::FBCreate()
{
	FBPropertyPublish(this, Color, "Color", nullptr, nullptr);
	FBPropertyPublish(this, Density, "Density", nullptr, nullptr);
	FBPropertyPublish(this, Feather, "Feather", nullptr, nullptr);
	FBPropertyPublish(this, BlendMode, "Mode", nullptr, nullptr);
	
	FBPropertyPublish(this, ZoneObjects, "Zone Objects", nullptr, nullptr);
	FBPropertyPublish(this, CreateZone, "Create Zone", nullptr, SetCreateZone);

	FBPropertyPublish(this, UseModelProperties, "Use Model Properties", nullptr, nullptr);
	
	FBPropertyPublish(this, SortZones, "Sort Zones", nullptr, nullptr);
	FBPropertyPublish(this, UpdateAllZones, "Update All Zones", nullptr, SetUpdateAllZonesAction);

	Color = FBColor(1.0, 1.0, 1.0);
	Density = 100.0;
	Density.SetMinMax(0.0, 100.0);

	Feather = 100.0;
	Feather.SetMinMax(0.0, 100.0);

	BlendMode = eCompositeBlendMultiply;

	ZoneObjects.SetSingleConnect(false);
	ZoneObjects.SetFilter( ModelShadowZone::GetInternalClassId() );
	UseModelProperties = true;
	SortZones = false;

	return ParentClass::FBCreate();
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/

void ObjectFilter3dShadow::FBDestroy()
{
	ParentClass::FBDestroy();
}

void ObjectFilter3dShadow::PrepShadowData(const CCompositionInfo *pInfo)
{
	ShadowData &data = mShadowData;

	FBCamera *pCamera = ((CCompositionInfo*) pInfo)->GetRenderCamera();

	FBColor vColor;
	vColor = Color;
	
	for (int i=0; i<3; ++i)
		data.color[i] = (float) vColor[i];
	/*
	data.density = fdensity;
	data.feather = ffeather;
	data.fnear = fnear;
	data.ffar = ffar;
	*/
	
	// for Zone computing
	data.zoneInfo.x = (float) ComputeNumberOfZoneModels();

	mGPUShadowBuffer.UpdateData( sizeof(ShadowData), 1, &mShadowData );
}

void ObjectFilter3dShadow::PrepZoneData(ZoneData &data, FBModel *pZoneModel, const bool useZoneProperties)
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
	double dColor[4] = {1.0, 1.0, 1.0, 1.0};
	Color.GetData( dColor, sizeof(double) * 3 );

	
	data.blendMode = (float) BlendMode.AsInt();
	data.density = 0.01f * (float) Density;
	data.feather = 0.01f * (float) Feather;

	if (useZoneProperties)
	{
		pModel->Color.GetData( dColor, sizeof(double) * 3 );
		data.feather = 0.01f * pModel->Feather;
		data.density = 0.01f * pModel->Density;
		//data.blendMode = (float) pModel->BlendMode.AsInt();
	}
	
	for (int i=0; i<4; ++i)
		data.color[i] = (float) dColor[i];

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

void ObjectFilter3dShadow::PrepAllZones(FBCamera *pCamera)
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

bool ObjectFilter3dShadow::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if ( (pAction == kFBCandidated) && (pThis == &BlendMode) )
	{
		// query a new program with updated define list
		mProgram = nullptr;
	}
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ObjectFilter3dShadow::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) 
{
	
	if (pThis == &ZoneObjects)
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

void ObjectFilter3dShadow::SetCreateZone(HIObject object, bool value)
{
	ObjectFilter3dShadow *pBase = FBCast<ObjectFilter3dShadow>(object);
	if (pBase && value) 
	{
		pBase->DoCreateZone();
	}
}

void ObjectFilter3dShadow::DoCreateZone()
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

void ObjectFilter3dShadow::SetUpdateAllZonesAction(HIObject object, bool value)
{
	ObjectFilter3dShadow *pBase = FBCast<ObjectFilter3dShadow>(object);
	if (pBase && value) 
	{
		pBase->DoUpdateAllZones();
	}
}

void ObjectFilter3dShadow::DoUpdateAllZones()
{
	for (int i=0, count=ZoneObjects.GetCount(); i<count; ++i)
		{
			ModelShadowZone *pZone = (ModelShadowZone*) ZoneObjects.GetAt(i);
			
			if (pZone->Enabled)
				pZone->SetUpdateFlag(true);
		}
}

const int ObjectFilter3dShadow::ComputeNumberOfZoneModels()
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

void ObjectFilter3dShadow::SortZoneModels(FBCamera *pCamera, const bool sorting)
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