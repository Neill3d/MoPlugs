
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_3d_fog.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "model_3d_fog.h"
#include "GL\glew.h"
#include "compositeMaster_common.h"
#include "compositeMaster_computeShaders.h"
#include "graphics\OGL_Utils.h"

FBClassImplementation( ModelFogVolume );								                //!< Register class
FBStorableCustomModelImplementation( ModelFogVolume, FOGVOLUME__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementationCompositionModel( ModelFogVolume, FOGVOLUME__DESCSTR, FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system

FBClassImplementation( ModelFogTarget );								                //!< Register class
FBStorableCustomModelImplementation( ModelFogTarget, FOGTARGET__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementationCompositionModel( ModelFogTarget, FOGTARGET__DESCSTR, FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ModelFogVolume

void AddPropertyViewForFogTarget(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FOGTARGET__CLASSSTR, pPropertyName, pHierarchy);
}
void ModelFogTarget::AddPropertiesToPropertyViewManager()
{
	
	AddPropertyViewForFogTarget("Enabled", "");
	AddPropertyViewForFogTarget("Target Blend Mode", "");
	AddPropertyViewForFogTarget("Color", "");
	AddPropertyViewForFogTarget("Target Density", "");
	AddPropertyViewForFogTarget("Target Feather", "");
	AddPropertyViewForFogTarget("Target Near Plane", "");
	AddPropertyViewForFogTarget("Target Far Plane", "");
}

/************************************************
*	Constructor.
************************************************/
ModelFogTarget::ModelFogTarget( const char* pName, HIObject pObject ) 
    : ObjectCompositeMarker( pName, pObject )
{
    FBClassInit;

	DisableObjectFlags(kFBFlagClonable);
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ModelFogTarget::FBCreate()
{
	ShadingMode = kFBModelShadingFlat;
    Size = 200.0;
	Color = FBColor(1.0, 0.0, 0.0);
    Length = 1.0;
    ResLevel = kFBMarkerMediumResolution;
    Look = kFBMarkerLookHardCross;
    Type = kFBMarkerTypeStandard;
    Show = true;

	FBPropertyPublish( this, Enabled, "Enabled", nullptr, nullptr );
	
	FBPropertyPublish( this, BlendMode, "Target Blend Mode", nullptr, nullptr );
	//FBPropertyPublish( this, TargetColor, "Target Color", nullptr, nullptr );
	FBPropertyPublish( this, TargetDensity, "Target Density", nullptr, nullptr );
	FBPropertyPublish( this, TargetFeather, "Target Feather", nullptr, nullptr );
	
	FBPropertyPublish( this, TargetNear, "Target Near Plane", nullptr, nullptr );
	FBPropertyPublish( this, TargetFar, "Target Far Plane", nullptr, nullptr );

	Enabled = true;

	BlendMode = eCompositeBlendMultiply;
	TargetDensity = 100.0;
	TargetFeather = 0.0;
	TargetNear = 100.0;
	TargetFar = 100.0;

	return ParentClass::FBCreate();
}

FBModel* ModelFogTarget::Clone()
{
	FBString newName( Name );
	ModelFogTarget *pNewModel = (ModelFogTarget*) FBCreateObject( MARKER__ASSETPATH, FOGTARGET__DESCSTR, newName );

	if (pNewModel)
	{
		pNewModel->Enabled = Enabled;
		pNewModel->Show = Show;
		pNewModel->BlendMode = BlendMode;

		// copy animatable properties
		double value;
		TargetDensity.GetData(&value, sizeof(double));
		pNewModel->TargetDensity.SetData(&value);
		TargetFeather.GetData(&value, sizeof(double));
		pNewModel->TargetFeather.SetData(&value);

		TargetNear.GetData(&value, sizeof(double));
		pNewModel->TargetNear.SetData(&value);
		TargetFar.GetData(&value, sizeof(double));
		pNewModel->TargetFar.SetData(&value);

		bool bvalue;
		Visibility.GetData( &bvalue, sizeof(bool) );
		pNewModel->Visibility.SetData( &bvalue );
		
		FBMatrix m;
		GetMatrix(m);
		pNewModel->SetMatrix(m);
	}

	return pNewModel;
}


void ModelFogTarget::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
	FBViewingOptions* lViewingOptions = FBSystem::TheOne().Renderer->GetViewingOptions();
    bool lIsSelectBufferPicking = (pRenderPass == kFBModelRenderPassPick) || lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	if (lViewingOptions->PickingMode() >=  kFBPickingModeModelsOnly)
		return;

	const float size = 0.01f * Size;

	vec4 fr[6]=
	{
		vec4(-size, 0.0f, 0.0f, 1.0f), vec4( size, 0.0f, 0.0f, 1.0f),
		vec4( 0.0f,  size, 0.0f, 1.0f),  vec4(0.0f,  size, 0.0f, 1.0f),
		vec4( 0.0f, 0.0f, size, 1.0f),	vec4( 0.0f, 0.0f, size, 1.0f)	
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

		glVertex3fv(fr[2].vec_array);
		glVertex3fv(fr[3].vec_array);

		glVertex3fv(fr[4].vec_array);
		glVertex3fv(fr[5].vec_array);

		glEnd();

		if (pLogShader != nullptr)
			pLogShader->UnBind();

		glPopMatrix();
	}
	glPopAttrib();

}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ModelFogVolume

void AddPropertyViewForVolumeFog(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FOGVOLUME__CLASSSTR, pPropertyName, pHierarchy);
}
void ModelFogVolume::AddPropertiesToPropertyViewManager()
{
	
	AddPropertyViewForVolumeFog("Enabled", "");
	AddPropertyViewForVolumeFog("Volume Blend Mode", "");
	AddPropertyViewForVolumeFog("Color", "");
	AddPropertyViewForVolumeFog("Volume Density", "");
	AddPropertyViewForVolumeFog("Volume Feather", "");

}

/************************************************
*	Constructor.
************************************************/
ModelFogVolume::ModelFogVolume( const char* pName, HIObject pObject ) 
    : ObjectCompositeMarker( pName, pObject )
{
    FBClassInit;

	DisableObjectFlags(kFBFlagClonable);
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ModelFogVolume::FBCreate()
{
    ShadingMode = kFBModelShadingTexture;
    Size = 200.0;
	Color = FBColor(0.0, 128.0/255.0, 128.0/255.0);
    Length = 1.0;
    ResLevel = kFBMarkerMediumResolution;
    Look = kFBMarkerLookBox;
    Type = kFBMarkerTypeStandard;
    Show = true;

	FBPropertyPublish( this, Enabled, "Enabled", nullptr, nullptr );
	
	FBPropertyPublish( this, BlendMode, "Volume Blend Mode", nullptr, nullptr );
	//FBPropertyPublish( this, VolumeColor, "Volume Color", nullptr, nullptr );
	FBPropertyPublish( this, VolumeDensity, "Volume Density", nullptr, nullptr );
	FBPropertyPublish( this, VolumeFeather, "Volume Feather", nullptr, nullptr );
	
	Enabled = true;

	BlendMode = eCompositeBlendMultiply;
	VolumeDensity = 100.0;
	VolumeFeather = 0.0;

	return ParentClass::FBCreate();
}

FBModel* ModelFogVolume::Clone()
{
	FBString newName( Name );
	ModelFogVolume *pNewModel = (ModelFogVolume*) FBCreateObject( MARKER__ASSETPATH, FOGVOLUME__DESCSTR, newName );

	if (pNewModel)
	{
		pNewModel->Enabled = Enabled;
		pNewModel->Show = Show;
		pNewModel->BlendMode = BlendMode;

		double value;
		VolumeDensity.GetData(&value, sizeof(double));
		pNewModel->VolumeDensity.SetData(&value);

		VolumeFeather.GetData(&value, sizeof(double));
		pNewModel->VolumeFeather.SetData(&value);

		bool bvalue;
		Visibility.GetData( &bvalue, sizeof(bool) );
		pNewModel->Visibility.SetData( &bvalue );
		
		FBMatrix m;
		GetMatrix(m);
		pNewModel->SetMatrix(m);
	}

	return pNewModel;
}

void ModelFogVolume::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
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
