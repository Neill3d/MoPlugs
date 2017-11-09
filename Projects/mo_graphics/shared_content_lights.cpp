
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_content_lights.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "shared_content.h"
#include "algorithm\math3d_mobu.h"
#include "compositeMaster_objectFinal.h"

#include "glm/gtx/matrix_decompose.hpp"

double sDefaultAttenuationNone[3] = { 1.0, 0.0, 0.0 };
double sDefaultAttenuationLinear[3] = { 0.0, 0.01, 0.0 };
double sDefaultAttenuationQuadratic[3] = { 0.0, 0.0, 0.0001 };


void CGPUFBScene::ConstructDefaultLight0(bool inEyeSpace, const mat4 &lViewMatrix, const mat4 &lViewRotationMatrix, LightDATA &light)
{
	memset( &light, 0, sizeof(LightDATA) );

	// If there is no light in the scene, we must put the two default lights
    // in order to have almost the same behavior as MB. Those two lights are
    // not present as soon as there is one light in the scene.
    const vec4 kPosition(0.0f, 0.0f, 1.0f, 0.0f);
    const vec4 kDirection0(0.2f, -0.2f, -0.6f, 0.0f);

	if (inEyeSpace)
		light.position = lViewMatrix * kPosition;
	else
		light.position = kPosition;

	light.type = LIGHT_TYPE_DIRECTION;

	light.color = vec3( 0.8f, 0.8f, 0.8f );
	light.attenuations = vec4(1.0f, 0.0f, 0.0f, 1.0f);

	if (inEyeSpace)
		light.dir = lViewRotationMatrix * kDirection0;
	else
		light.dir = kDirection0;
}

void CGPUFBScene::ConstructDefaultLight1(bool inEyeSpace, const mat4 &lViewMatrix, const mat4 &lViewRotationMatrix, LightDATA &light)
{
	memset( &light, 0, sizeof(LightDATA) );

	// If there is no light in the scene, we must put the two default lights
    // in order to have almost the same behavior as MB. Those two lights are
    // not present as soon as there is one light in the scene.
    const vec4 kPosition(0.0f, 0.0f, 1.0f, 0.0f);
    const vec4 kDirection1(-0.6f, -0.4f, 0.75f, 0.0f);

	if (inEyeSpace)
		light.position = lViewMatrix * kPosition;
	else
		light.position = kPosition;

	light.type = LIGHT_TYPE_DIRECTION;

	light.color = vec3( 0.8f, 0.8f, 0.8f );
	light.attenuations = vec4(1.0f, 0.0f, 0.0f, 1.0f);

	if (inEyeSpace)
		light.dir = lViewRotationMatrix * kDirection1;
	else
		light.dir = kDirection1;
}

void CGPUFBScene::ConstructFromFBLight( const bool ToEyeSpace, const mat4 &lViewMatrix, const mat4 &lViewRotationMatrix, FBLight *pLight, LightDATA &light )
{
	memset( &light, 0, sizeof(LightDATA) );

	// Get the light's position, direction, colour, and cone angle (stored in direction's 4th component)
	// Also setup attenuation for the lighting equation ...
	// Note that for point/spot lights, we add a 1.0 to the 4th colour
	// component so we can know to generate the direction from position
	FBVector3d   pos;
	pLight->GetVector( pos ); // Get Global Translation.
			
	// Convert the ModelView (World) space
	/*
	FBVector4d lViewPosition(pos);
	if (ToEyeSpace)
		FBVectorMatrixMult(lViewPosition, lViewMatrix, pos);
		*/

	light.position = vec3( (float)pos[0], (float)pos[1], (float)pos[2] );
	if (ToEyeSpace)
		mult( light.position, lViewMatrix, vec4(light.position.x, light.position.y, light.position.z, 1.0f) );
		//light.position =  light.position * lViewMatrix;

	// Set whether or not we take the spot factor into consideration in the last position spot ...
	switch (pLight->LightType)
	{
	case kFBLightTypeInfinite:
		light.type = LIGHT_TYPE_DIRECTION;
		break;
	case kFBLightTypePoint:
		light.type = LIGHT_TYPE_POINT;
		break;
	case kFBLightTypeSpot:
		light.type = LIGHT_TYPE_SPOT;
		break;
	}
	
	//FBVectorToVec3( lViewPosition.mValue, light.position.vec_array );

	// Setup color and attenuation 
	FBColorAndAlpha diffuseColor = (FBColor)pLight->DiffuseColor;
	double attenuation[4];

	if( pLight->LightType != kFBLightTypeInfinite )
	{
		switch(pLight->AttenuationType)
		{
		case kFBAttenuationLinear:
			attenuation[0] = sDefaultAttenuationLinear[0];
			attenuation[1] = sDefaultAttenuationLinear[1];
			attenuation[2] = sDefaultAttenuationLinear[2];
			break;
		case kFBAttenuationQuadratic:
		case kFBAttenuationCubic:
			//assert(light->AttenuationType != kFBAttenuationCubic);// "Don't support Cubic attenuation yet, use Quadratic instead.");
			attenuation[0] = sDefaultAttenuationQuadratic[0];
			attenuation[1] = sDefaultAttenuationQuadratic[1];
			attenuation[2] = sDefaultAttenuationQuadratic[2];
			break;
		case kFBAttenuationNone:
		default:
			attenuation[0] = sDefaultAttenuationNone[0];
			attenuation[1] = sDefaultAttenuationNone[1];
			attenuation[2] = sDefaultAttenuationNone[2];
			break;
		}
		attenuation[3] = pLight->Intensity * 0.01; // / 100.0;
	}
	else
	{
		attenuation[0] = 1.0; attenuation[1] = 0.0; attenuation[2] = 0.0;
		attenuation[3] = pLight->Intensity * 0.01;
	}

	FBVector3d scale(100.0, 100.0, 100.0);

	FBProperty *lprop = pLight->PropertyList.Find( "Magnitude" );
	if (lprop)
	{
		lprop->GetData( &scale[0], sizeof(double) );
	}
	else
	{
		lprop = pLight->PropertyCreate( "Magnitude", kFBPT_double, "Number", true, false );
		if (lprop)
			lprop->SetData( &scale[0] );
	}

	//pLight->GetVector( scale, kModelScaling );
	light.radius = (float) scale[0] * 20.0;


	FBVectorToVec3( diffuseColor, light.color.vec_array );
	FBVectorToVec4( attenuation, light.attenuations.vec_array );
	
	// Get the rotation matrix and multiply light vector with it
	FBMatrix rotationMatrix;
	FBVector4d direction;
	pLight->GetMatrix( rotationMatrix, kModelRotation, true );
	FBVector4d normal(0, -1, 0, 1);
	FBVectorMatrixMult( direction, rotationMatrix, normal );
    
	// Convert the ModelView (World) space
	light.dir = vec3( (float)direction[0], (float)direction[1], (float)direction[2] );
	if (ToEyeSpace)
		mult( light.dir, lViewRotationMatrix, vec4(light.dir.x, light.dir.y, light.dir.z, 1.0f) );
		//light.dir = lViewRotationMatrix * vec4(light.dir.x, light.dir.y, light.dir.z, 1.0f);
	//FBVector4d lViewDirection(direction);
	//if (ToEyeSpace)
	//	FBVectorMatrixMult(lViewDirection, lViewRotationMatrix, direction);
	//else
	//	lViewDirection = direction;

	double cosAngle;
	if( pLight->LightType == kFBLightTypeSpot )
		cosAngle = cos( (3.141592654*pLight->ConeAngle/180.0f)/2.0f);
	else
		cosAngle = 0.0;

	light.spotAngle = (float) cosAngle;
	//FBVectorToVec3( lViewDirection.mValue, light.dir.vec_array );

	//
	light.castSpecularOnObject = 0.0f;

	/*
	// TODO: let's change this property value inside the Connection Data Change Event !

	FBProperty *pProp = pLight->PropertyList.Find("Cast Specular On Object");
	if (pProp)
	{
		light.castSpecularOnObject = (float) pProp->AsInt();
	}
	else
	{
		// add a new property

		pProp = pLight->PropertyCreate( "Cast Specular On Object", kFBPT_bool, "BOOL", false, false );
		if (pProp) pProp->SetInt( 0 );
	}
	*/
}

void CGPUFBScene::PrepFBSceneLights()
{
	/*
	glm::vec3 scale, translation, skew;
	glm::quat orientation;
	glm::vec4 perspective;
	glm::mat4 glmMV;

	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			glmMV[i][j] = mCameraCache.mv4[i][j];

	glm::decompose(glmMV, scale, orientation, translation, skew, perspective);

	quat camRotation(orientation[0], orientation[1], orientation[2], orientation[3]);
	
	mat4 camModelView(mCameraCache.mv4);
	mat4 camRotationMatrix;
	camRotationMatrix.set_rot(camRotation);
	//transpose(camRotationMatrix);
	*/

	FBMatrix pCamMatrix(mCameraCache.mv); // (cameraCache.mv);
	FBMatrix lViewMatrix( pCamMatrix );
	
	FBRVector lViewRotation;
	FBMatrixToRotation(lViewRotation, lViewMatrix);

	FBMatrix lViewRotationMatrix;
	FBRotationToMatrix(lViewRotationMatrix, lViewRotation);

	mat4 camModelView(mCameraCache.mv4);
	mat4 camRotationMatrix;

	for (int i=0; i<16; ++i)
	{
		camModelView.mat_array[i] = (float) lViewMatrix[i];
		camRotationMatrix.mat_array[i] = (float) lViewRotationMatrix[i];
	}

	
	//
	FBScene *pScene = mSystem.Scene;
	int numLights = pScene->Lights.GetCount();

	auto &lights = mGPUSceneLights->GetLightsVector();
	auto &dirLights = mGPUSceneLights->GetDirLightsVector();

	if (0 == numLights)
	{
		// only 2 directional lights and not cluster work
		dirLights.resize(2);
		lights.clear();
		//pLightsManager->mLightCasters.clear();
		//pLightsManager->mLightCastersDataPtr.clear();

		ConstructDefaultLight0( false, camModelView, camRotationMatrix, dirLights[0] );
		ConstructDefaultLight1( false, camModelView, camRotationMatrix, dirLights[1] );
	}
	else
	{
		// process scene lights and enable clustering if some point/spot light exist

		mUsedSceneLights.resize(numLights);

		int numDirLights = 0;
		int numPointLights = 0;
		int numLightCasters = 0;

		for (int i=0; i<numLights; ++i)
		{
			FBLight *pLight = pScene->Lights[i];
			mUsedSceneLights[i] = pLight;

			if (pLight->CastLightOnObject)
			{
				if (pLight->LightType == kFBLightTypeInfinite) numDirLights++;
				else numPointLights++;
			}
			
			// lights for shadows
			// temproary make only one cascaded shadow
			if (pLight->LightType == kFBLightTypeInfinite && numLightCasters == 0)
			{
				if (pLight->LightType != kFBLightTypePoint && pLight->CastShadows)
					numLightCasters++;
			}	
		}

		mUsedInfiniteLights.resize(numDirLights);
		mUsedPointLights.resize(numPointLights);

		dirLights.resize(numDirLights);
		lights.resize(numPointLights);
		//pLightsManager->mLightCasters.resize(numLightCasters);
		//pLightsManager->mLightCastersDataPtr.resize(numLightCasters);

		numDirLights = 0;
		numPointLights = 0;
		numLightCasters = 0;

		for (auto iter=begin(mUsedSceneLights); iter!=end(mUsedSceneLights); ++iter)
		{
			FBLight *pLight = *iter;
			LightDATA *pLightData = nullptr;

			if (pLight->CastLightOnObject)
			{
				if (pLight->LightType != kFBLightTypeInfinite)
				{
					ConstructFromFBLight( false, camModelView, camRotationMatrix, pLight, lights[numPointLights] );
					pLightData = &lights[numPointLights];
					mUsedPointLights[numPointLights] = pLight;
					numPointLights++;
				}
				else
				{
					ConstructFromFBLight( false, camModelView, camRotationMatrix, pLight, dirLights[numDirLights] );
					pLightData = &dirLights[numDirLights];
					mUsedInfiniteLights[numDirLights] = pLight;
					numDirLights++;
				}
			}
			/*
			// lights for shadows
			// temproary make only one cascaded shadow
			if (pLight->LightType == kFBLightTypeInfinite && numLightCasters == 0)
			{
				if (pLight->LightType != kFBLightTypePoint && pLight->CastShadows)
				{
					pLightsManager->mLightCasters[numLightCasters] = pLight;
					pLightsManager->mLightCastersDataPtr[numLightCasters] = pLightData;
					
					numLightCasters++;
				}
			}
			*/
		}
	}
}

void CGPUFBScene::PrepLightsInViewSpace(CGPUShaderLights *pLights)
{
	FBMatrix pCamMatrix(mCameraCache.mv); // (cameraCache.mv);
	FBMatrix lViewMatrix( pCamMatrix );
	
	FBRVector lViewRotation;
	FBMatrixToRotation(lViewRotation, lViewMatrix);

	FBMatrix lViewRotationMatrix;
	FBRotationToMatrix(lViewRotationMatrix, lViewRotation);

	mat4 modelrotation;

	for (int i=0; i<16; ++i)
	{
		modelrotation.mat_array[i] = (float) lViewRotationMatrix[i];
	}

	if (pLights != nullptr)
		pLights->UpdateTransformedLights(mCameraCache.mv4, modelrotation);
}

void CGPUFBScene::PrepLightsFromFBList(CGPUShaderLights *pShaderLights, 
	CCameraInfoCache &cameraCache, std::vector<FBLight*> &mobuLights)
{
	/*
	FBMatrix pCamMatrix;
	pCamera->GetCameraMatrix( pCamMatrix, kFBModelView );
	FBMatrix lViewMatrix( pCamMatrix );
	
    FBRVector lViewRotation;
    FBMatrixToRotation(lViewRotation, lViewMatrix);

    FBMatrix lViewRotationMatrix;
    FBRotationToMatrix(lViewRotationMatrix, lViewRotation);
	*/

	mat4 camModelView, camRotationMatrix;
	quat camRotation;

	camModelView = cameraCache.mv4; 
	camModelView.as_rot(camRotation);
	camRotationMatrix.set_rot(camRotation);

	//
	const int numLights = (int) mobuLights.size();

	auto &dstLights = pShaderLights->GetLightsVector();
	auto &dstDirLights = pShaderLights->GetDirLightsVector();

	if (numLights == 0)
	{
		// only 2 directional lights and not cluster work
		dstDirLights.resize(2);
		dstLights.clear();
		//pLightsManager->mLightCasters.clear();
		//pLightsManager->mLightCastersDataPtr.clear();

		ConstructDefaultLight0( false, camModelView, camRotationMatrix, dstDirLights[0] );
		ConstructDefaultLight1( false, camModelView, camRotationMatrix, dstDirLights[1] );

		
	}
	else
	{
		// process scene lights and enable clustering if some point/spot light exist

		int numDirLights = 0;
		int numPointLights = 0;
		int numLightCasters = 0;

		for (auto iter=begin(mobuLights); iter!=end(mobuLights); ++iter)
		{
			FBLight *pLight = *iter;

			if (pLight->CastLightOnObject)
			{
				if (pLight->LightType == kFBLightTypeInfinite) numDirLights++;
				else numPointLights++;
			}
			
			// lights for shadows
			// temproary make only one cascaded shadow
			if (pLight->LightType == kFBLightTypeInfinite && numLightCasters == 0)
			{
				if (pLight->LightType != kFBLightTypePoint && pLight->CastShadows)
					numLightCasters++;
			}	
		}


		dstDirLights.resize(numDirLights);
		dstLights.resize(numPointLights);
		//pLightsManager->mLightCasters.resize(numLightCasters);
		//pLightsManager->mLightCastersDataPtr.resize(numLightCasters);

		numDirLights = 0;
		numPointLights = 0;
		numLightCasters = 0;

		auto &srcLights = mGPUSceneLights->GetLightsVector();
		auto &srcDirLights = mGPUSceneLights->GetDirLightsVector();

		for (auto iter=begin(mobuLights); iter!=end(mobuLights); ++iter)
		{
			FBLight *pLight = *iter;
			LightDATA *pLightData = nullptr;

			if (pLight->CastLightOnObject)
			{
				if (pLight->LightType != kFBLightTypeInfinite)
				{
					for (int i=0, count=(int)mUsedPointLights.size(); i<count; ++i)
					{
						if (pLight == mUsedPointLights[i])
						{
							dstLights[numPointLights] = srcLights[i];
							pLightData = &dstLights[numPointLights];
							numPointLights++;
							break;
						}
					}
				}
				else
				{
					for (int i=0, count=(int)mUsedInfiniteLights.size(); i<count; ++i)
					{
						if (pLight == mUsedInfiniteLights[i])
						{
							dstDirLights[numDirLights] = srcDirLights[i];
							pLightData = &dstDirLights[numDirLights];
							numDirLights++;
							break;
						}
					}
				}
			}
			/*
			// lights for shadows
			// temproary make only one cascaded shadow
			if (pLight->LightType == kFBLightTypeInfinite && numLightCasters == 0)
			{
				if (pLight->LightType != kFBLightTypePoint && pLight->CastShadows)
				{
					pLightsManager->mLightCasters[numLightCasters] = pLight;
					pLightsManager->mLightCastersDataPtr[numLightCasters] = pLightData;
					
					numLightCasters++;
				}
			}
			*/
		}
	}
}

void CGPUFBScene::MapLightsOnGPU()
{
	// TODO: update only on light changes ?!
	//PrepFBSceneLights();

	if (nullptr != mCompositionLights)
	{
		mCompositionLights->MapOnGPU();
		mCompositionLights->PrepGPUPtr();
	}
	else
	{
		mGPUSceneLights->MapOnGPU();
		mGPUSceneLights->PrepGPUPtr();
	}

}
/*
bool CGPUFBScene::PrepShaderLights( const bool useSceneLights, FBPropertyListComponent *AffectingLights, 
	std::vector<FBLight*> &shaderLightsPtr, std::auto_ptr<CGPUShaderLights> &shaderLights )
{
	if (nullptr == mUberShader.get() )
		return false;

	int numberOfExlLights = 0;

	// DONE: check exclusive light list from the current composition
	
	if (false == useSceneLights && AffectingLights->GetCount() > 0)
	{
		numberOfExlLights = 0;
		std::vector<FBLight*> &sceneLights = GetUsedLightsVector();

		shaderLightsPtr.resize(sceneLights.size() );

		for (auto iter=begin(sceneLights); iter!=end(sceneLights); ++iter)
		{
			for (int i=0, count=AffectingLights->GetCount(); i<count; ++i)
			{
				if (*iter == AffectingLights->GetAt(i))
				{
					shaderLightsPtr[numberOfExlLights] = *iter;
					numberOfExlLights += 1;
					
					break;
				}
			}
		}
	}

	float flagUseSceneLights = 1.0f;

	if (numberOfExlLights > 0)
	{
		// TODO: don't construct lights, just copy values from scene lights
		CGPUFBScene::UpdateLightsFromFBList( shaderLights.get(), GetCameraCache(), shaderLightsPtr );

		// TODO: reupdate gpu buffer only on changes, not every frame !
		shaderLights->MapOnGPU();
		shaderLights->PrepGPUPtr();

		const Graphics::CustomEffectShaderLocations *locPtr = mUberShader->GetCustomEffectShaderLocationsPtr();
		shaderLights->Bind( mUberShader->GetFragmentProgramId(), locPtr->shaderDirLights, locPtr->shaderLights );

		mUberShader->UpdateShaderLightsCount( shaderLights->GetNumberOfDirLights(), shaderLights->GetNumberOfLights() );

		flagUseSceneLights = 2.0f;
	}

	mUberShader->UpdateUseSceneLights( flagUseSceneLights );
	return true;
}
*/


bool CGPUFBScene::PrepShaderLights( const bool useSceneLights, FBPropertyListObject *AffectingLights, 
	std::vector<FBLight*> &shaderLightsPtr, CGPUShaderLights *shaderLights )
{
	if (nullptr == mMaterialShaders.Get() || nullptr == shaderLights )
		return false;

	const int numberOfExlLights = AffectingLights->GetCount();
	shaderLightsPtr.resize(numberOfExlLights);

	for (int i=0; i<numberOfExlLights; ++i)
	{
		shaderLightsPtr[i] = (FBLight*) AffectingLights->GetAt(i);
	}

	if (numberOfExlLights == 0)
		return true;

	// DONE: don't construct lights, just copy values from scene lights
	CGPUFBScene::PrepLightsFromFBList( shaderLights, GetCameraCache(), shaderLightsPtr );

	// TODO: reupdate gpu buffer only on changes, not every frame !
	//shaderLights->MapOnGPU();
	//shaderLights->PrepGPUPtr();
	
	return true;
}

bool CGPUFBScene::BindLights( const bool resetLastBind, const CGPUShaderLights *pUserLights )
{
	if ( nullptr == mMaterialShaders.Get() )
		return false;

	const CGPUShaderLights *pShaderLights = (nullptr!=mCompositionLights) ? mCompositionLights : mGPUSceneLights.get();

	if (nullptr != pUserLights)
	{
		if (pUserLights->GetNumberOfDirLights() > 0 || pUserLights->GetNumberOfLights() > 0)
		{
			pShaderLights = pUserLights;
		}
	}

	if (nullptr == pShaderLights)
	{
		mMaterialShaders->UploadLightingInformation( 0, 0 );
		return false;
	}

	// TODO: check for last uberShader lights binded
	if (true == resetLastBind)
		mLastLightsBinded = nullptr;

	if (mLastLightsBinded == pShaderLights)
	{
		return true;
	}

	// bind a new buffer
	const auto loc = mMaterialShaders->GetCurrentEffectLocationsPtr()->fptr();
	const GLint dirLights = loc->GetLocation( Graphics::eCustomLocationDirLights );
	const GLint lights = loc->GetLocation( Graphics::eCustomLocationLights );
	if ( dirLights >= 0 || lights >= 0)
	{
		pShaderLights->Bind( mMaterialShaders->GetFragmentProgramId(), dirLights, lights );
		mMaterialShaders->UploadLightingInformation( pShaderLights->GetNumberOfDirLights(), pShaderLights->GetNumberOfLights() );

		mLastLightsBinded = (CGPUShaderLights*) pShaderLights;
	}
	else
	{
		mMaterialShaders->UploadLightingInformation( 0, 0 );
		return false;
	}
	return true;
}