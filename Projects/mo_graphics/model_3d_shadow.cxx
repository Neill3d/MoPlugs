
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_3d_shadow.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "model_3d_shadow.h"
#include "GL\glew.h"
#include "compositeMaster_common.h"
#include "compositeMaster_computeShaders.h"
#include "graphics\OGL_Utils.h"

FBClassImplementation( ModelShadowZone );								                //!< Register class
FBStorableCustomModelImplementation( ModelShadowZone, SHADOWZONE__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementationCompositionModel( ModelShadowZone, SHADOWZONE__DESCSTR, FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ModelFogVolume

void AddPropertyViewForShadowZone(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(SHADOWZONE__CLASSSTR, pPropertyName, pHierarchy);
}
void ModelShadowZone::AddPropertiesToPropertyViewManager()
{
	
	AddPropertyViewForShadowZone("Enabled", "");
	AddPropertyViewForShadowZone("Zone Blend Mode", "");
	AddPropertyViewForShadowZone("Master Light", "");

	AddPropertyViewForShadowZone( "Evaluate", "", true );
	AddPropertyViewForShadowZone( "RealTime Update", "Evaluate" );
	AddPropertyViewForShadowZone( "Update On TimeSlider Change", "Evaluate" );
	AddPropertyViewForShadowZone( "Update When Selected", "Evaluate" );
	AddPropertyViewForShadowZone( "Update Skip Frames", "Evaluate" );
	AddPropertyViewForShadowZone( "Update", "Evaluate" );
	

	AddPropertyViewForShadowZone("Shadow Color", "");
	AddPropertyViewForShadowZone("Density", "");
	AddPropertyViewForShadowZone("Feather", "");

	AddPropertyViewForShadowZone("Include Objects", "");
	AddPropertyViewForShadowZone("Exclude Objects", "");
	AddPropertyViewForShadowZone("Use GPU Cache", "");
	AddPropertyViewForShadowZone("Render Transparency", "");

	AddPropertyViewForShadowZone("Auto Volume Culling", "");

	AddPropertyViewForShadowZone("Draw Node Count", "");
	AddPropertyViewForShadowZone("Draw GeomCache Count", "");
}

/************************************************
*	Constructor.
************************************************/
ModelShadowZone::ModelShadowZone( const char* pName, HIObject pObject ) 
    : ObjectCompositeMarker( pName, pObject )
	, CFBComponentUpdateProps()
{
    FBClassInit;

	DisableObjectFlags(kFBFlagClonable);
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ModelShadowZone::FBCreate()
{
    ShadingMode = kFBModelShadingTexture;
    Size = 200.0;
	Color = FBColor(128.0/255.0, 0.0, 0.0);
    Length = 1.0;
    ResLevel = kFBMarkerMediumResolution;
    Look = kFBMarkerLookBox;
    Type = kFBMarkerTypeStandard;
    Show = true;

	FBPropertyPublish( this, Enabled, "Enabled", nullptr, nullptr );
	
	FBPropertyPublish( this, Resolution, "Resolution", nullptr, nullptr );
	FBPropertyPublish( this, Bias, "Bias", nullptr, nullptr );
	FBPropertyPublish( this, EnableOffset, "Enable Offset", nullptr, nullptr );
	FBPropertyPublish( this, OffsetFactor, "Offset Factor", nullptr, nullptr );
	FBPropertyPublish( this, OffsetUnits, "Offset Units", nullptr, nullptr );

//	FBPropertyPublish( this, BlendMode, "Zone Blend Mode", nullptr, nullptr );
	//FBPropertyPublish( this, VolumeColor, "Volume Color", nullptr, nullptr );
	FBPropertyPublish( this, MasterLight, "Master Light", nullptr, nullptr );
//	FBPropertyPublish( this, ShadowColor, "Shadow Color", nullptr, nullptr );
	
	FBPropertyPublish( this, Density, "Density", nullptr, nullptr );
	FBPropertyPublish( this, Feather, "Feather", nullptr, nullptr );

	FBPropertyPublish( this, IncludeList, "Include Objects", nullptr, nullptr );
	FBPropertyPublish( this, ExcludeList, "Exclude Objects", nullptr, nullptr );
	FBPropertyPublish( this, UseGPUCache, "Use GPU Cache", nullptr, nullptr );
	
	FBPropertyPublish( this, NearPlane, "Near Plane", nullptr, nullptr );
	FBPropertyPublish( this, FarPlane, "Far Plane", nullptr, nullptr );

	FBPropertyPublish( this, RenderTransparency, "Render Transparency", nullptr, nullptr );
	FBPropertyPublish( this, AutoVolumeCulling, "Auto Volume Culling", nullptr, nullptr );

	FBPropertyPublish( this, DrawNodeCount, "Draw Node Count", GetDrawNodeCount, nullptr );
	FBPropertyPublish( this, DrawGeomCacheCount, "Draw GeomCache Count", GetDrawGeomCacheCount, nullptr );

	DrawNodeCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	DrawGeomCacheCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	Enabled = true;

	Resolution = 2048;
	Bias = 1.0;
	EnableOffset = true;
	OffsetFactor = 4.0;
	OffsetUnits = 32.0;

//	BlendMode = eCompositeBlendMultiply;

	MasterLight.SetSingleConnect(true);
	MasterLight.SetFilter( FBLight::GetInternalClassId() );

//	ShadowColor = FBColor(0.0, 0.0, 0.0);
	
	Density = 100.0;
	Density.SetMinMax( 0.0, 100.0 );

	Feather = 100.0;
	Feather.SetMinMax( 0.0, 100.0 );

	IncludeList.SetSingleConnect( false );
	IncludeList.SetFilter( FBModel::GetInternalClassId() );
	ExcludeList.SetSingleConnect( false );
	ExcludeList.SetFilter( FBModel::GetInternalClassId() );
	UseGPUCache = true;

	NearPlane = 1.0;
	FarPlane = 4000.0;

	RenderTransparency = true;
	AutoVolumeCulling = true;

	//
	//
	FBCreateUpdateProps( this, (fbExternalGetSetHandler) SetUpdateAction );

	//
	//

	CGPUFBScene *mGPUFBScene = &CGPUFBScene::instance();
	if (mGPUFBScene)
		mGPUFBScene->AddShadowNode(this);

	return ParentClass::FBCreate();
}

void ModelShadowZone::FBDestroy()
{
	CGPUFBScene *mGPUFBScene = &CGPUFBScene::instance();
	if (mGPUFBScene)
		mGPUFBScene->RemoveShadowNode(this);

	ParentClass::FBDestroy();
}

FBModel* ModelShadowZone::Clone()
{
	FBString newName( Name );
	ModelShadowZone *pNewModel = (ModelShadowZone*) FBCreateObject( MARKER__ASSETPATH, SHADOWZONE__DESCSTR, newName );

	if (pNewModel)
	{
		pNewModel->Enabled = Enabled;
		pNewModel->Show = Show;
		//pNewModel->BlendMode = BlendMode;
		/*
		double value;
		VolumeDensity.GetData(&value, sizeof(double));
		pNewModel->VolumeDensity.SetData(&value);

		VolumeFeather.GetData(&value, sizeof(double));
		pNewModel->VolumeFeather.SetData(&value);
		*/
		bool bvalue;
		Visibility.GetData( &bvalue, sizeof(bool) );
		pNewModel->Visibility.SetData( &bvalue );
		
		FBMatrix m;
		GetMatrix(m);
		pNewModel->SetMatrix(m);
	}

	return pNewModel;
}

void ModelShadowZone::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
	FBViewingOptions* lViewingOptions = FBSystem::TheOne().Renderer->GetViewingOptions();
    bool lIsSelectBufferPicking = (pRenderPass == kFBModelRenderPassPick) || lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	if (lViewingOptions->PickingMode() >=  kFBPickingModeModelsOnly)
		return;

	const float size = 0.01f * (float) Size;

	vec4 fr[8]=
	{
		// near
		vec4(-size, -size, -size, 1.0f), vec4( size, -size, -size, 1.0f),
		vec4( size,  size, -size, 1.0f),  vec4(-size,  size, -size, 1.0f),
		// far
		vec4(-size, -size, size, 1.0f),	vec4( size, -size, size, 1.0f),	
		vec4( size,  size, size, 1.0f), vec4(-size,  size, size, 1.0f)
	};

	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_TRANSFORM_BIT | GL_LINE_BIT); //Push Current GL states.
    {
		FBColor currColor;

		if (!lIsSelectBufferPicking && !lIsColorBufferPicking) 
        { 
            if ((bool)Selected) 
            {
				currColor = FBColor( 0.0, 1.0, 0.0 );   //!< Otherwise, BLUE.
            } 
            else 
            {
				currColor = Color;
            }
        }
		else
		{
			currColor = UniqueColorId;
			glLoadName(1);
		}

        glLineWidth(3.0);    //!< Draw line wider to easy picking.
		glColor3dv(currColor);

		glPushMatrix();

		FBMatrix matrix;
		GetMatrix(matrix);
		glMultMatrixd(matrix);

		// check if have to draw in logarithmic depth

		GLSLShader *pLogShader = nullptr;

		if ( IsLogarithmicDepth() )
		{
			pLogShader = CompositeComputeShader::CMixedProgramManager::instance().QueryLogDrawShader();
		}

		if (pLogShader != nullptr)
		{
			pLogShader->Bind();
			pLogShader->setUniformFloat( "Fcoef", 2.0f / log2(pCamera->FarPlaneDistance + 1.0) );
			pLogShader->setUniformVector( "color", (float)currColor[0], (float)currColor[1], (float)currColor[2], 1.0f );
		}

		glBegin(GL_LINES);

		glVertex3fv(fr[0].vec_array);
		glVertex3fv(fr[1].vec_array);

		glVertex3fv(fr[1].vec_array);
		glVertex3fv(fr[2].vec_array);

		glVertex3fv(fr[2].vec_array);
		glVertex3fv(fr[3].vec_array);

		glVertex3fv(fr[3].vec_array);
		glVertex3fv(fr[0].vec_array);

		glVertex3fv(fr[4].vec_array);
		glVertex3fv(fr[5].vec_array);

		glVertex3fv(fr[5].vec_array);
		glVertex3fv(fr[6].vec_array);

		glVertex3fv(fr[6].vec_array);
		glVertex3fv(fr[7].vec_array);

		glVertex3fv(fr[7].vec_array);
		glVertex3fv(fr[4].vec_array);

		glVertex3fv(fr[0].vec_array);
		glVertex3fv(fr[4].vec_array);

		glVertex3fv(fr[1].vec_array);
		glVertex3fv(fr[5].vec_array);

		glVertex3fv(fr[2].vec_array);
		glVertex3fv(fr[6].vec_array);

		glVertex3fv(fr[3].vec_array);
		glVertex3fv(fr[7].vec_array);

		/*
		connect tfr points as follow:
		0-1, 1-2, 2-3, 3-0, 
		4-5, 5-6, 6-7, 7-4,
		0-4, 1-5, 2-6, 3-7
		*/
		glEnd();

		if (pLogShader != nullptr)
			pLogShader->UnBind();

		glPopMatrix();
	}
	glPopAttrib();

}

int ModelShadowZone::GetDrawNodeCount(HIObject object)
{
	ModelShadowZone *pNode = FBCast<ModelShadowZone>(object);
	if (pNode) 
	{
		return pNode->mStats.totalDrawNodeCount;
	}
	return 0;
}

int ModelShadowZone::GetDrawGeomCacheCount(HIObject object)
{
	ModelShadowZone *pNode = FBCast<ModelShadowZone>(object);
	if (pNode) 
	{
		return pNode->mStats.totalGeometryCacheCount;
	}
	return 0;
}

void ModelShadowZone::SetUpdateAction(HIObject object, bool value)
{
	ModelShadowZone *pNode = FBCast<ModelShadowZone>(object);
	if (pNode && value) 
	{
		pNode->OnSetUpdateAction();
	}
}