
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_3d_decal.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "model_3d_decal.h"
#include "GL\glew.h"
#include "compositeMaster_common.h"
#include "compositeMaster_computeShaders.h"
#include "MB_renderer.h"
#include <math.h>
#include "graphics\OGL_Utils.h"

FBClassImplementation( ModelDecal );								                //!< Register class
FBStorableCustomModelImplementation( ModelDecal, DECAL__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementationCompositionModel( ModelDecal, DECAL__DESCSTR, FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ModelDecal

void ModelDecal::AddPropertyView(const char* pPropertyName, const char* pHierarchy, bool pIsFolder)
{
	FBPropertyViewManager::TheOne().AddPropertyView(DECAL__CLASSSTR, pPropertyName, pHierarchy);
}
void ModelDecal::AddPropertiesToPropertyViewManager()
{
	
	AddPropertyView("Enabled", "");
	AddPropertyView("Blend Mode", "");

}

/************************************************
*	Constructor.
************************************************/
ModelDecal::ModelDecal( const char* pName, HIObject pObject ) 
    : ObjectCompositeMarker( pName, pObject )
{
    FBClassInit;

	mLastTextureId = 0;
	mLastTextureAddress = 0;

	DisableObjectFlags(kFBFlagClonable);
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ModelDecal::FBCreate()
{
    ShadingMode = kFBModelShadingTexture;
    Size = 200.0;
	Color = FBColor(0.0, 0.0, 128.0/255.0);
    Length = 1.0;
    ResLevel = kFBMarkerMediumResolution;
    Look = kFBMarkerLookBox;
    Type = kFBMarkerTypeStandard;
    Show = true;

	FBPropertyPublish( this, Enabled, "Enabled", nullptr, nullptr );
	FBPropertyPublish( this, Opacity, "Opacity", nullptr, nullptr );
	FBPropertyPublish( this, BlendMode, "Blend Mode", nullptr, nullptr );
	
	FBPropertyPublish( this, Texture, "Texture", nullptr, nullptr );
	FBPropertyPublish( this, Refresh, "Refresh", nullptr, SetRefresh );

	FBPropertyPublish( this, ResolutionWidth, "Resolution Width", nullptr, nullptr );
	FBPropertyPublish( this, ResolutionHeight, "Resolution Height", nullptr, nullptr );
	FBPropertyPublish( this, AspectRatio, "Aspect Ratio", nullptr, nullptr );
	FBPropertyPublish( this, UsesStoryTrack, "Uses Story Track", nullptr, nullptr );
	FBPropertyPublish( this, VideoClipPath, "Video Clip Path", nullptr, nullptr );
	
	FBPropertyPublish( this, NearPlaneDistance, "Near Plane Distance", nullptr, nullptr );
	FBPropertyPublish( this, FarPlaneDistance, "Far Plane Distance", nullptr, nullptr );
	FBPropertyPublish( this, ProjectionType, "Projection Type", nullptr, nullptr );
	FBPropertyPublish( this, FOV, "Field Of View", nullptr, nullptr );
	FBPropertyPublish( this, OrthoScale, "Ortho Scale", nullptr, nullptr );

	FBPropertyPublish(this, Contrast, "Contrast", nullptr, nullptr);
	FBPropertyPublish(this, Saturation, "Saturation", nullptr, nullptr);
	FBPropertyPublish(this, Brightness, "Brightness", nullptr, nullptr);

	FBPropertyPublish(this, Gamma, "Gamma", nullptr, nullptr);

	FBPropertyPublish(this, Inverse, "Inverse", nullptr, nullptr);

	FBPropertyPublish(this, Hue, "Hue", nullptr, nullptr);
	FBPropertyPublish(this, HueSaturation, "Hue Saturation", nullptr, nullptr);
	FBPropertyPublish(this, Lightness, "Lightness", nullptr, nullptr);

	FBPropertyPublish( this, GrabValues, "Grab Values", nullptr, SetGrabValues );
	FBPropertyPublish( this, ApplyToCurrent, "Apply To Current", nullptr, SetApplyToCurrent );
	FBPropertyPublish( this, ApplyToPerspective, "Apply To Perspective", nullptr, SetApplyToPerspective );

	Enabled = true;
	Opacity = 100.0;
	Opacity.SetMinMax(0.0, 100.0);
	BlendMode = eCompositeBlendMultiply;
	
	ResolutionWidth = 512.0;
	ResolutionHeight= 512.0;
	AspectRatio = 1.0;
	NearPlaneDistance = 1.0;
	FarPlaneDistance = 1000.0;

	ProjectionType = kFBCameraTypePerspective;
	FOV = 40.0;
	OrthoScale = 100.0;
	OrthoScale.SetMinMax(1.0, 200.0);

	Contrast = 0.0;
	Contrast.SetMinMax(-100.0, 100.0, true, true);
	Saturation = 0.0;
	Saturation.SetMinMax(-100.0, 100.0, true, true);
	Brightness = 0.0;
	Brightness.SetMinMax(-100.0, 100.0, true, true);

	Gamma = 100.0;
	Gamma.SetMinMax( -200.0, 300.0, true, true );

	Inverse = false;

	Hue = 0.0;
	Hue.SetMinMax(-100.0, 100.0, true, true);
	HueSaturation = 0.0;
	HueSaturation.SetMinMax(-100.0, 100.0, true, true);
	Lightness = 0.0;
	Lightness.SetMinMax(-100.0, 100.0, true, true);

	Texture.SetSingleConnect(true);
	Texture.SetFilter( FBTexture::GetInternalClassId() );

	ResolutionWidth.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	ResolutionHeight.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	AspectRatio.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	UsesStoryTrack.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	VideoClipPath.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	return ParentClass::FBCreate();
}


FBModel* ModelDecal::Clone()
{
	FBString newName( Name );

	ModelDecal *pNewModel = (ModelDecal*) FBCreateObject( MARKER__ASSETPATH, DECAL__DESCSTR, newName );

	if (pNewModel)
	{
		pNewModel->Enabled = Enabled;
		pNewModel->Opacity = Opacity;
		pNewModel->Show = Show;

		pNewModel->BlendMode = BlendMode;

		// ?! connect the same texture ?!
		if (Texture.GetCount() )
			pNewModel->Texture.Add( Texture[0] );

		pNewModel->ResolutionWidth = ResolutionWidth;
		pNewModel->ResolutionHeight = ResolutionHeight;
		pNewModel->AspectRatio = AspectRatio;
		pNewModel->UsesStoryTrack = UsesStoryTrack;
		pNewModel->VideoClipPath = VideoClipPath;

		pNewModel->NearPlaneDistance = NearPlaneDistance;
		pNewModel->FarPlaneDistance = FarPlaneDistance;
		pNewModel->ProjectionType = ProjectionType;

		// copy values of animatable properties
		double value;
		FOV.GetData(&value, sizeof(double));
		pNewModel->FOV.SetData(&value);

		OrthoScale.GetData(&value, sizeof(double));
		pNewModel->OrthoScale.SetData(&value);
		Contrast.GetData(&value, sizeof(double));
		pNewModel->Contrast.SetData(&value);
		Brightness.GetData(&value, sizeof(double));
		pNewModel->Brightness.SetData(&value);
		Saturation.GetData(&value, sizeof(double));
		pNewModel->Saturation.SetData(&value);
		Gamma.GetData(&value, sizeof(double));
		pNewModel->Gamma.SetData(&value);
		
		bool bvalue;
		Visibility.GetData( &bvalue, sizeof(bool) );
		pNewModel->Visibility.SetData( &bvalue );
		
		FBMatrix m;
		GetMatrix(m);
		pNewModel->SetMatrix(m);
	}

	return pNewModel;
}

void ModelDecal::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
	FBViewingOptions* lViewingOptions = FBSystem::TheOne().Renderer->GetViewingOptions();
    bool lIsSelectBufferPicking = (pRenderPass == kFBModelRenderPassPick) || lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	if (lViewingOptions->PickingMode() >=  kFBPickingModeModelsOnly)
		return;

	//vec3 points[8];
	/*
	switch(ProjectionType)
	{
	case kFBCameraTypeOrthogonal:
		ComputeCameraOrthoPoints(ResolutionWidth, ResolutionHeight, this, FarPlaneDistance, NearPlaneDistance, points);
		break;
	case kFBCameraTypePerspective:
		ComputeCameraFrustumPoints(ResolutionWidth, ResolutionHeight, this, FarPlaneDistance, NearPlaneDistance, FieldOfView, points); 
		break;
	}
	*/

	UpdateProjectionMatrices();

	FBMatrix temp;
	FBMatrixMult(temp, mProjection, mModelView);
	FBMatrix inv;
	FBMatrixInverse(inv, temp);

	vec4 fr[8]=
	{
	// near
	vec4(-1.0f, -1.0f, -1.0f, 1.0f), vec4( 1.0f, -1.0f, -1.0f, 1.0f),
	vec4( 1.0f,  1.0f, -1.0f, 1.0f),  vec4(-1.0f,  1.0f, -1.0f, 1.0f),
	// far
	vec4(-1.0f, -1.0f, 1.0f, 1.0f),	vec4( 1.0f, -1.0f, 1.0f, 1.0f),	
	vec4( 1.0f,  1.0f, 1.0f, 1.0f), vec4(-1.0f,  1.0f, 1.0f, 1.0f)
	};
	// Transform all vertices:
	// multiply vertex array (fr) by matrix. result is transformed vertex array (tfr)
	vec4 tfr[8];

	mat4 finv;

	for (int i=0; i<16; ++i)
		finv.mat_array[i] = (float) inv[i];

	for (int i=0; i<8; ++i)
	{
		tfr[i] = finv * fr[i];

		tfr[i].x /= tfr[i].w;
		tfr[i].y /= tfr[i].w;
		tfr[i].z /= tfr[i].w;
		tfr[i].w = 1.0f;
	}
	//transform_points(fr, 8, inv, tfr); 

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

		glVertex3fv(tfr[0].vec_array);
		glVertex3fv(tfr[1].vec_array);

		glVertex3fv(tfr[1].vec_array);
		glVertex3fv(tfr[2].vec_array);

		glVertex3fv(tfr[2].vec_array);
		glVertex3fv(tfr[3].vec_array);

		glVertex3fv(tfr[3].vec_array);
		glVertex3fv(tfr[0].vec_array);

		glVertex3fv(tfr[4].vec_array);
		glVertex3fv(tfr[5].vec_array);

		glVertex3fv(tfr[5].vec_array);
		glVertex3fv(tfr[6].vec_array);

		glVertex3fv(tfr[6].vec_array);
		glVertex3fv(tfr[7].vec_array);

		glVertex3fv(tfr[7].vec_array);
		glVertex3fv(tfr[4].vec_array);

		glVertex3fv(tfr[0].vec_array);
		glVertex3fv(tfr[4].vec_array);

		glVertex3fv(tfr[1].vec_array);
		glVertex3fv(tfr[5].vec_array);

		glVertex3fv(tfr[2].vec_array);
		glVertex3fv(tfr[6].vec_array);

		glVertex3fv(tfr[3].vec_array);
		glVertex3fv(tfr[7].vec_array);

		/*
		connect tfr points as follow:
		0-1, 1-2, 2-3, 3-0, 
		4-5, 5-6, 6-7, 7-4,
		0-4, 1-5, 2-6, 3-7
		*/
		glEnd();

		if (pLogShader != nullptr)
			pLogShader->UnBind();
	}
	glPopAttrib();

	//CHECK_GL_ERROR_MOBU();
}



bool ModelDecal::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	/*
	if (pAction == kFBCandidated && (pThis == &Mode || pThis == &TargetType) )
	{
		// query a new program with updated define list
		mProgram = nullptr;
	}
	*/
	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ModelDecal::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) 
{
	if (pThis == &Texture)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
			// update program defines
			
			FBTexture *pTexture = (FBTexture*) pPlug;

			ResolutionWidth = pTexture->Width;
			ResolutionHeight = pTexture->Height;
			if (ResolutionHeight != 0.0)
				AspectRatio = ResolutionWidth / ResolutionHeight;
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
			
			ResolutionWidth = 512.0;
			ResolutionHeight = 512.0;
			AspectRatio = 1.0;
		}
	}

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void ModelDecal::SetRefresh(HIObject object, bool value)
{
	ModelDecal *pBase = FBCast<ModelDecal>(object);
	if (pBase && value) 
	{
		pBase->DoRefreshTextureInfo();
	}
}

void ModelDecal::SetGrabValues(HIObject object, bool value)
{
	ModelDecal *pBase = FBCast<ModelDecal>(object);
	if (pBase && value) 
	{
		pBase->DoGrabValues();
	}
}

void ModelDecal::SetApplyToCurrent(HIObject object, bool value)
{
	ModelDecal *pBase = FBCast<ModelDecal>(object);
	if (pBase && value) 
	{
		pBase->DoApplyToCurrent();
	}
}

void ModelDecal::SetApplyToPerspective(HIObject object, bool value)
{
	ModelDecal *pBase = FBCast<ModelDecal>(object);
	if (pBase && value) 
	{
		pBase->DoApplyToPerspective();
	}
}

void ModelDecal::DoRefreshTextureInfo()
{
	if (Texture.GetCount() > 0 )
	{
		FBTexture *pTexture = (FBTexture*) Texture.GetAt(0);

		ResolutionWidth = pTexture->Width;
		ResolutionHeight = pTexture->Height;
		if (ResolutionHeight != 0.0)
			AspectRatio = ResolutionWidth / ResolutionHeight;

		RefreshConnectedTexture();
	}
	else
	{
		ResolutionWidth = 512.0;
		ResolutionHeight = 512.0;
		AspectRatio = 1.0;
	}
}

void ModelDecal::DoGrabValues()
{
	DoRefreshTextureInfo();

	FBCamera *pCamera  = FBSystem::TheOne().Renderer->CurrentCamera;

	FBCameraFrameSizeMode lMode = pCamera->FrameSizeMode;
		
	int lWidth = pCamera->ResolutionWidth;
	int lHeight = pCamera->ResolutionHeight;

	bool projectorAspect = true;

	if (projectorAspect)
	{
		pCamera->FrameSizeMode = kFBFrameSizeFixedResolution;
		pCamera->ResolutionWidth = ResolutionWidth;
		pCamera->ResolutionHeight = ResolutionHeight;
		pCamera->ForeGroundImageKeepRatio = false;
	}
	else
	{
		pCamera->ForeGroundImageKeepRatio = false;
	}

	
	// grab camera matrix
	pCamera->GetCameraMatrix( mProjection, kFBProjection );
	pCamera->GetCameraMatrix( mModelView, kFBModelView );

	FBMatrix matrix;
	( (FBModel*) pCamera)->GetMatrix(matrix);
	SetMatrix(matrix);

	NearPlaneDistance = pCamera->NearPlaneDistance;
	FarPlaneDistance = pCamera->FarPlaneDistance;
	ProjectionType = pCamera->Type;
	FOV = pCamera->FieldOfView;

	if (projectorAspect)
	{
		pCamera->FrameSizeMode = lMode;
		pCamera->ResolutionWidth = lWidth;
		pCamera->ResolutionHeight = lHeight;
	}
}

void ModelDecal::UpdateProjectionMatrices()
{
	
	switch(ProjectionType)
	{
	case kFBCameraTypeOrthogonal:
		ComputeOrthoMatrices();
		break;
	case kFBCameraTypePerspective:
		ComputePerspectiveMatrices();
		break;
	}
	
	ComputeFinalProjectorMatrix();
}

void ModelDecal::ComputeOrthoMatrices()
{
	mModelView.Identity();
	mProjection.Identity();

	// Directional light support

	// Get all the information necessary to setup the lighting matrix
	// Will need to create a MODELVIEWPROJ matrix using:
	//		- Transformation matrix of light
	//		- Custom projection matrix based on light

	// We need a base matrix because the transformation matrix doesn't take into account that lights
	// start out with transforms that are not represented ... grr ...
	double base[16]	=
        {
            1.0, 0.0, 0.0, 0.0,
            0.0, 0.0, -1.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        };
	FBMatrix baseMat( base );
	baseMat.Identity();

	// Factor out the scale, because we don't want it ...
	FBMatrix rotationMat;
	FBMatrix transformationMat;
	this->GetMatrix( rotationMat, kModelRotation, true );
	FBMatrixMult( transformationMat, rotationMat, baseMat );

	FBMatrix rot2;
	FBRotationToMatrix( rot2, FBRVector(0.0, -90.0, 0.0) );

	FBMatrixMult( transformationMat, transformationMat, rot2 );

	double radius;
	FBVector3d newPos;
	this->GetVector(newPos);
	//GetWorldBounds( pLight, radius, newPos );

	transformationMat(3,0) = newPos[0];
	transformationMat(3,1) = newPos[1];
	transformationMat(3,2) = newPos[2];
	transformationMat(3,3) = 1.0f;

	FBMatrixInverse( mModelView, transformationMat );

	//pLight->GetMatrix( lightInvTM, kModelInverse_Transformation );

	// Ok .. now we just need a projection matrix ...
	const double scaleFactor = 0.01 * OrthoScale;

	double left		= -ResolutionWidth * 0.5 * scaleFactor;
	double right	=  ResolutionWidth * 0.5 * scaleFactor;
	double top		=  ResolutionHeight * 0.5 * scaleFactor;
	double bottom	= -ResolutionHeight * 0.5 * scaleFactor;

	double fNear	=  NearPlaneDistance;
	double fFar		=  FarPlaneDistance;

	double diffRL	= 1.0 / (right - left);
	double diffTB	= 1.0 / (top - bottom);
	double diffFN	= 1.0 / (fFar - fNear);

	double orthoValues[16] =
        {
            2.0 * diffRL, 0,            0,                        0,
            0,            2.0 * diffTB, 0,                        0,
            0,            0,            -2.0 * diffFN,            0,
            0,            0,            -(fFar + fNear) * diffFN, 1.0
        };

	mProjection = FBMatrix( orthoValues );
}

void ModelDecal::ComputePerspectiveMatrices()
{
	mModelView.Identity();
	mProjection.Identity();

	// Spotlight support

	// Get all the information necessary to setup the lighting matrix
	// Will need to create a MODELVIEWPROJ matrix using:
	//		- Transformation matrix of light
	//		- Custom projection matrix based on light

	// We need a base matrix because the transformation matrix doesn't take into account that lights
	// start out with transforms that are not represented ... grr ...

	double base[16]	=
        {
            1.0, 0.0, 0.0, 0.0,
            0.0, 0.0, -1.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        };
	FBMatrix baseMat( base );
	baseMat.Identity();

	// Factor out the scale, because we don't want it ...
	//FBVector3d v;
	FBVector3d scl;
	FBMatrix rotationMat;
	FBMatrix transformationMat;
	this->GetMatrix( rotationMat, kModelRotation, true );
	FBMatrixMult( transformationMat, rotationMat, baseMat );
	this->GetVector(scl, kModelScaling);

	FBMatrix rot2;
	FBRotationToMatrix( rot2, FBRVector(0.0, -90.0, 0.0) );

	FBMatrixMult( transformationMat, transformationMat, rot2 );

	//pLight->GetVector( v );

	//transformationMat.Identity();
	FBVector3d translation = this->Translation;
	FBVector3d rotation = this->Rotation;
	transformationMat(3,0) = translation[0];
	transformationMat(3,1) = translation[1];
	transformationMat(3,2) = translation[2];
	transformationMat(3,3) = 1.0f;

	FBMatrixInverse( mModelView, transformationMat );
	//mModelView = transformationMat;
	//pLight->GetMatrix( pLightView, kModelTransformation );
	/*
	FBTRSToMatrix(mModelView, FBTVector(translation[0], translation[1], translation[2], 1.0), 
		FBRVector(rotation[0], rotation[1], rotation[2]), FBSVector(1.0, 1.0, 1.0) );
		*/
	//this->GetMatrix( lightInvTM, kModelInverse_Transformation );
	
	// Ok .. now we just need a projection matrix ...
	float fov = (float) FOV;
	float fFar =  (float) FarPlaneDistance; // (double) scl[1] * 2.0f;
	float fNear = (float) NearPlaneDistance; // 1.0f;
	float top = tan(fov*3.14159f/360.0f) * fNear;
	float bottom = -top;
	float left = bottom;
	float right = top;
	double perspectiveValues[16] =
        {
            (2.0*fNear)/(right-left),   0,                          0,                          0,
            0,                         (2.0*fNear)/(top-bottom),    0,                          0,
            0,                         0,                           -(fFar+fNear)/(fFar-fNear), -(2.0f*fFar*fNear)/(fFar-fNear),
            0,                         0,                           -1.0f,                      0
        };

	
	mat4 m4;
	perspective( m4, (float)FOV, (float)AspectRatio, fNear, fFar );

	for (int i=0; i<16; ++i)
		perspectiveValues[i] = (double) m4.mat_array[i];
		
	mProjection = FBMatrix( perspectiveValues );
}

void ModelDecal::ComputeFinalProjectorMatrix()
{
	if (Texture.GetCount() == 0)
		return;
	FBTexture *pTexture = (FBTexture*) Texture.GetAt(0);
	GLuint textureId = 0;

	if (textureId == 0)
	{
		glActiveTexture(GL_TEXTURE0);
		pTexture->OGLInit();
		textureId = pTexture->TextureOGLId;

		mLastTextureId = textureId;
		//mLastTextureAddress = glGetTextureHandleARB(textureId);
	}

	if (textureId > 0)
	{
		FBMatrix projMatrix, clipMatrix, viewMatrix, modelMatrix, modelViewMatrix;

		FBVector3d texTranslation, texRotation, texScale;

		pTexture->Translation.GetData( texTranslation, sizeof(double) * 3 );
		pTexture->Rotation.GetData( texRotation, sizeof(double) * 3 );
		pTexture->Scaling.GetData( texScale, sizeof(double) * 3 );

		clipMatrix.Identity();
		clipMatrix[0] = 0.5 * texScale[0];
		clipMatrix[5] = 0.5 * texScale[1];
		clipMatrix[10] = 0.5 * texScale[2];

		clipMatrix[12] = 0.5 + texTranslation[0];
		clipMatrix[13] = 0.5 + texTranslation[1];
		clipMatrix[14] = texTranslation[2];
			
//		int lWidth = ResolutionWidth;
//		int lHeight = ResolutionHeight;
		/*
		if (data.ProjectorAspect)
		{
			projCamera->FrameSizeMode = kFBFrameSizeFixedResolution;
			projCamera->ResolutionWidth = lProjTexture->Width;
			projCamera->ResolutionHeight = lProjTexture->Height;
			projCamera->ForeGroundImageKeepRatio = false;
		}
		else
		{
			projCamera->ForeGroundImageKeepRatio = false;
		}
		*/
		
		// grab camera matrix
		//projCamera->GetCameraMatrix( projMatrix, kFBProjection );
		//projCamera->GetCameraMatrix( viewMatrix, kFBModelView );

		projMatrix = mProjection;
		viewMatrix = mModelView;

		/*
		if (data.ProjectorAspect)
		{
			projCamera->FrameSizeMode = lMode;
			projCamera->ResolutionWidth = lWidth;
			projCamera->ResolutionHeight = lHeight;
		}
		*/
		/*
		dst.textureId = textureId;
		dst.maskId = maskId;
		dst.ClipMatrix = clipMatrix;
		dst.ProjMatrix = projMatrix;
		dst.ViewMatrix = viewMatrix;
		*/
		//FBMatrix	resultMatrix;
		mFinalMatrix.Identity();
		FBMatrixMult( mFinalMatrix, projMatrix, viewMatrix );
		FBMatrixMult( mFinalMatrix, clipMatrix, mFinalMatrix );
		/*
		for (int i=0; i<16; ++i)
			dst.matrix.mat_array[i] = (float) resultMatrix[i];

		result = 1;
		*/
	}
	else
	{
		//FBMatrix initMatrix;
		//initMatrix.Identity();
		mFinalMatrix.Identity();
		//dst.textureId = 0;
		//dst.maskId = 0;
	}
}

void ModelDecal::DoApplyToCurrent()
{
	FBCamera *pCamera = FBSystem::TheOne().Renderer->CurrentCamera;
	if (FBIS(pCamera, FBCameraSwitcher) )
		pCamera = ( (FBCameraSwitcher*) pCamera)->CurrentCamera;

	DoApplyToCamera(pCamera);
}

void ModelDecal::DoApplyToPerspective()
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	for (int i=0, count=pScene->Cameras.GetCount(); i<count; ++i)
	{
		FBCamera *pCamera = pScene->Cameras[i];

		if (pCamera->SystemCamera && pCamera->Type == kFBCameraTypePerspective)
		{
			DoApplyToCamera(pCamera);
			break;
		}
	}
}

void ModelDecal::DoApplyToCamera(FBCamera *pCamera)
{
	if (pCamera == nullptr)
		return;

	FBMatrix matrix;
	GetMatrix(matrix);
	( (FBModel*) pCamera)->SetMatrix(matrix);

	pCamera->NearPlaneDistance = NearPlaneDistance;
	pCamera->FarPlaneDistance = FarPlaneDistance;
	pCamera->Type = ProjectionType;
	pCamera->FieldOfView = FOV;

	pCamera->FrameSizeMode = kFBFrameSizeFixedResolution;
	pCamera->ResolutionWidth = ResolutionWidth;
	pCamera->ResolutionHeight = ResolutionHeight;
}

const GLuint64 ModelDecal::GetTextureAddress()
{
	if (Texture.GetCount() == 0)
		return 0;

	if (mLastTextureAddress == 0)
	{
		FBTexture *pTexture = (FBTexture*) Texture.GetAt(0);
		pTexture->OGLInit();
		const GLuint textureId = pTexture->TextureOGLId;

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, textureId);

		mLastTextureAddress = glGetTextureHandleARB(textureId);

		//glBindTexture(GL_TEXTURE_2D, 0);

		mLastTextureId = textureId;
	}

	return mLastTextureAddress;
}

void ModelDecal::MakeTextureResident()
{
	if (mLastTextureAddress > 0)
		glMakeTextureHandleResidentARB(mLastTextureAddress);
}

void ModelDecal::MakeTextureNonResident()
{
	if (mLastTextureAddress > 0)
		glMakeTextureHandleNonResidentARB(mLastTextureAddress);
}

void ModelDecal::BindTexture(const GLenum slot)
{
	mLastTextureId = 0;

	if (Texture.GetCount() == 0)
		return;
	
	FBTexture *pTexture = (FBTexture*) Texture.GetAt(0);
	GLuint textureId = 0;

	if (textureId == 0)
	{
		//pTexture->OGLInit();
		textureId = pTexture->TextureOGLId;

		mLastTextureId = textureId;
	}

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, textureId);
}

void ModelDecal::UnBindTexture(const GLenum slot)
{
	if (mLastTextureId > 0)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

bool ModelDecal::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if (pStoreWhat == kAttributes)
	{
		pFbxObject->FieldWriteBegin( "ModelDecalData" );
		{
			// VERSION
			pFbxObject->FieldWriteI(10);

			for (int i=0; i<16; ++i)
				pFbxObject->FieldWriteD( mProjection[i] );

			for (int i=0; i<16; ++i)
				pFbxObject->FieldWriteD( mModelView[i] );
		}
		pFbxObject->FieldWriteEnd();
	}

	return ParentClass::FbxStore(pFbxObject, pStoreWhat);
}

bool ModelDecal::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{

	if (pStoreWhat == kAttributes)
	{
		if (true == pFbxObject->FieldReadBegin( "ModelDecalData" ) )
		{
			int version = pFbxObject->FieldReadI();
			
			if (version == 10)
			{
				for (int i=0; i<16; ++i)
					mProjection[i] = pFbxObject->FieldReadD();

				for (int i=0; i<16; ++i)
					mModelView[i] = pFbxObject->FieldReadD();
			}
		}
		pFbxObject->FieldReadEnd();
	}

	return ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);
}

void ModelDecal::RefreshConnectedTexture()
{
	for (int j=0, count=GetSrcCount(); j<count; ++j)
	{
		if ( FBIS(GetSrc(j), FBTexture) )
		{
			FBTexture *pTexture = (FBTexture*) GetSrc(j);

			FBVideo *pVideo = pTexture->Video;
			if (FBIS(pVideo, FBVideoClip) )
			{
				FBVideoClip *pClip = (FBVideoClip*) pVideo;
				FBString filename = pClip->Filename;
				pClip->Filename = filename;
			}

			break;
		}
	}
}