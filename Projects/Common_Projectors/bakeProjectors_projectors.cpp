
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: bakeProjectors_projectors.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "bakeProjectors_projectors.h"

//
#define PROJTEX_CLASSNAME			"ProjTexShader"

////////////////

CProjectors::CProjectors()
{
}

CProjectors::~CProjectors()
{
}

int CProjectors::PrepareProjector(FBProjectorDATA &mobuData, ProjectorDATA &data)
{
	int result = 0;

	FBCamera *projCamera = mobuData.pProjector;
	FBTexture *lProjTexture = mobuData.pTexture;

	int textureId = 0;
	int maskId = 0;

    if (projCamera)
	{
		if (mobuData.pTexture == nullptr)
			lProjTexture = projCamera->ForeGroundTexture;

		if (lProjTexture)
		{
			FBVideo *pVideo = lProjTexture->Video;
			if ( pVideo && FBIS(pVideo, FBVideoMemory) )
				textureId = ((FBVideoMemory*) pVideo)->TextureOGLId;
			else
			{
				lProjTexture->OGLInit();
				textureId = lProjTexture->GetTextureObject();
			}
		}
	}

	if (projCamera == nullptr)
		return result;

	if (data.maskLayer >= 0 && mMasks[(int)data.maskLayer] != nullptr)
		maskId = mMasksId[(int)data.maskLayer];
		

	if (textureId > 0)
	{
		FBMatrix projMatrix, clipMatrix, viewMatrix, modelMatrix, modelViewMatrix, textureMatrix;

		FBVector3d texTranslation, texRotation, texScale;

		lProjTexture->Translation.GetData( texTranslation, sizeof(double) * 3 );
		lProjTexture->Rotation.GetData( texRotation, sizeof(double) * 3 );
		lProjTexture->Scaling.GetData( texScale, sizeof(double) * 3 );

		clipMatrix.Identity();
		clipMatrix[0] = 0.5 * texScale[0];
		clipMatrix[5] = 0.5 * texScale[1];
		clipMatrix[10] = 0.5 * texScale[2];

		clipMatrix[12] = 0.5 + texTranslation[0];
		clipMatrix[13] = 0.5 + texTranslation[1];
		clipMatrix[14] = texTranslation[2];
			

		FBCameraFrameSizeMode lMode = projCamera->FrameSizeMode;
		
		int lWidth = projCamera->ResolutionWidth;
		int lHeight = projCamera->ResolutionHeight;

		if (mobuData.ProjectorAspect)
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

		
		// grab camera matrix
		projCamera->GetCameraMatrix( projMatrix, kFBProjection );
		projCamera->GetCameraMatrix( viewMatrix, kFBModelView );

		if (mobuData.ProjectorAspect)
		{
			projCamera->FrameSizeMode = lMode;
			projCamera->ResolutionWidth = lWidth;
			projCamera->ResolutionHeight = lHeight;
		}

		mobuData.textureId = textureId;
		mobuData.maskId = maskId;
		mobuData.ClipMatrix = clipMatrix;
		mobuData.ProjMatrix = projMatrix;
		mobuData.ViewMatrix = viewMatrix;

		FBMatrix	resultMatrix;
		FBMatrixMult( resultMatrix, projMatrix, viewMatrix );
		FBMatrixMult( resultMatrix, clipMatrix, resultMatrix );

		for (int i=0; i<16; ++i)
			data.matrix.mat_array[i] = (float) resultMatrix[i];

		result = 1;
	}
	else
	{
		FBMatrix initMatrix;
		initMatrix.Identity();

		mobuData.textureId = 0;
		mobuData.maskId = 0;
	}

	return result;
}

bool CProjectors::ShaderHasProjectors(FBShader *pShader)
{
	if (pShader->PropertyList.Find( "Projector1" ) != nullptr)
	{
		return true;
	}

	return false;
}



bool CProjectors::DoPropertiesToData(FBShader *pShader)
{
	if (ShaderHasProjectors(pShader) == false)
		return false;

	FBPropertyListObject *Mask1 = (FBPropertyListObject *) pShader->PropertyList.Find( "Mask1" );
	FBPropertyListObject *Mask2 = (FBPropertyListObject *) pShader->PropertyList.Find( "Mask2" );

	FBTexture *pMaskTexture1 = (Mask1 && Mask1->GetCount()) ? (FBTexture*)Mask1->GetAt(0) : nullptr;
	FBTexture *pMaskTexture2 = (Mask2 && Mask2->GetCount()) ? (FBTexture*)Mask2->GetAt(0) : nullptr;

	mMasks[0] = mMasks[1] = nullptr;

	if (pMaskTexture1 != nullptr)
		mMasks[0] = pMaskTexture1->Video;
	if (pMaskTexture2 != nullptr)
		mMasks[1] = pMaskTexture2->Video;

	mMasksId[0] = mMasksId[1] = 0;

	auto fn_getMaskId = [] (FBVideo *pVideo) -> GLuint {
		
		GLuint id = 0;
		
		if (pVideo != nullptr)
		{
			if ( FBIS(pVideo, FBVideoMemory) )
			{
				id = ( (FBVideoMemory*) pVideo )->TextureOGLId;
			}
			else
			if ( FBIS(pVideo, FBVideoClip) )
			{
				FBVideoClip *pClip = (FBVideoClip*) pVideo;
				id = pClip->GetTextureID();	
			}
		}

		return id;
	};

	mMasksId[0] = fn_getMaskId(mMasks[0]);
	mMasksId[1] = fn_getMaskId(mMasks[1]);

	//

	int realNumberOfProjectors=0;
	
	FBStringList strIndices;
	strIndices.SetString( "1~2~3~4~5~6~7" );

	for (int i=0; i<MAX_PROJECTORS_COUNT; ++i)
	{
		ProjectorDATA_Zero( mProjectorsData.projectors[i] );
	
		// projector active means: 1) use flag on; 2) camera assigned; 3) foreground video clip assigned
		FBCamera* lProjector = nullptr;

		FBPropertyBool *pProjectorUse = (FBPropertyBool*) pShader->PropertyList.Find( FBString("Projector Usage", strIndices[i]) );
		FBPropertyListObject *pProjector = (FBPropertyListObject *) pShader->PropertyList.Find( FBString("Projector", strIndices[i]) );
		FBPropertyListObject *pProjectorTexture = (FBPropertyListObject *) pShader->PropertyList.Find( FBString("Projector Texture", strIndices[i]) );
		
		if (pProjector && pProjectorUse && *pProjectorUse)
		{
			for(int j = 0; j < pProjector->GetCount(); ++j)
			{
				FBCamera* lFBCamera = FBCast<FBCamera>(pProjector->GetAt(j)->GetHIObject());
				if (lFBCamera)
				{
					// check first of all override video
					
					FBVideo *pVideo = lFBCamera->ForeGroundMedia;
					if ( (pProjectorTexture->GetCount() > 0) || (pVideo != nullptr) )
						lProjector = lFBCamera;
				}
			}
		}

		if (lProjector == nullptr) continue;

		FBTexture *lTexture = (FBTexture*) ( (pProjectorTexture->GetCount() > 0) ? pProjectorTexture->GetAt(0) : nullptr );
		FBProperty *propBlendMode = pShader->PropertyList.Find( FBString("Projector Blend Mode", strIndices[i]) );
		FBProperty *propBlendOpacity = pShader->PropertyList.Find( FBString("Projector Blend Opacity", strIndices[i]) );
		
		FBProperty *propMask = pShader->PropertyList.Find( FBString("Projector Mask", strIndices[i]) );
		FBProperty *propMaskChannel = pShader->PropertyList.Find( FBString("Projector Mask Channel", strIndices[i]) );

		FBProperty *propAspect = pShader->PropertyList.Find( FBString("Projector Keep Aspect", strIndices[i]) );

		if (propBlendMode == nullptr || propBlendOpacity == nullptr || propMask == nullptr || propMaskChannel == nullptr || propAspect == nullptr)
			continue;
		
		double opacity = 1.0;
		propBlendOpacity->GetData( &opacity, sizeof(double) );

		ProjectorDATA_Set( propMask->AsInt(), 
							propMaskChannel->AsInt(), 
							(int) propBlendMode->AsInt(), 
							0.01f * opacity, 
							mProjectorsData.projectors[realNumberOfProjectors] );
		
		// check if a mask is not assigned
		if (mMasksId[(int)mProjectorsData.projectors[realNumberOfProjectors].maskLayer] == 0)
			mProjectorsData.projectors[realNumberOfProjectors].maskLayer = -1.0f;

		const bool keepAspect = propAspect->AsInt() > 0;
		FBProjectorDATA::Set( lProjector, lTexture, keepAspect, mProjectorsModels[realNumberOfProjectors] );

		//
		PrepareProjector(mProjectorsModels[realNumberOfProjectors], mProjectorsData.projectors[realNumberOfProjectors]);

		realNumberOfProjectors++;
	}
	
	mNumberOfProjectors = realNumberOfProjectors;
	mProjectorsData.numberOfProjectors = realNumberOfProjectors;

	return true;
}

const int CProjectors::GetNumberOfProjectors() const
{
	return mNumberOfProjectors;
}

const ProjectorSet &CProjectors::GetProjectorSet() const
{
	return mProjectorsData;
}

const GLuint CProjectors::GetMaskId(const int mask) const
{
	return mMasksId[mask];
}

bool CProjectors::PrepFull(FBShader *pShader)
{
	if (false == DoPropertiesToData(pShader) )
		return false;

	mBufferProjectors.Prep(mNumberOfProjectors, mProjectorsData);

	return true;
}

bool CProjectors::PrepLight()
{
	for (int i=0; i<mNumberOfProjectors; ++i)
		PrepareProjector( mProjectorsModels[i], mProjectorsData.projectors[i] );

	if (mNumberOfProjectors > 0)
		mBufferProjectors.Prep(mNumberOfProjectors, mProjectorsData);

	return true;
}

bool CProjectors::Bind(const int shaderFragmentId, const int projectorsUniformLoc, const GLuint muteTextureId) const
{
	
	mBufferProjectors.BindAsUniform( shaderFragmentId, projectorsUniformLoc );

	mBufferProjectors.BindProjectionMapping(	mNumberOfProjectors,
												mProjectorsData,
												mMasksId[0], 
												mMasksId[1] );


	// bind projector textures

	for (int i=0; i<mNumberOfProjectors; ++i)
	{
		GLuint texId = mProjectorsModels[i].textureId;
		if (texId > 0 && texId != muteTextureId)
		{
			glActiveTexture(GL_TEXTURE10 + i);
			glBindTexture(GL_TEXTURE_2D, texId );
		}
	}

	glActiveTexture( GL_TEXTURE0 );

	return true;
}

void CProjectors::UnBind() const
{
	mBufferProjectors.UnBindProjectionMapping();
}