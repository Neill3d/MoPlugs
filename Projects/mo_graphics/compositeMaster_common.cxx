
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_common.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositemaster_common.h"
#include "compositeMaster_object.h"
#include "compositeMaster_objectLayers.h"
//#include "compositeMaster_effects.h"
#include "compositeMaster_objectFinal.h"
#include "compositeMaster_objectLUTFilter.h"
#include "compositeMaster_objectFogFilter.h"
#include "compositeMaster_objectFilters.h"

// global variables

HdlFBPlugTemplate<ObjectComposition>		gCurrentComposite;
HdlFBPlugTemplate<ObjectComposition>		gOutputComposite;

/*
ObjectComposition			*gCurrentComposite = nullptr;
ObjectComposition			*gOutputComposite = nullptr;
*/
/*
// 2 color buffers (rgba color and view space normal)
FrameBuffer		gDefaultFrameBuffer(0, 0, FrameBuffer::eCreateColorTexture | FrameBuffer::eCreateDepthTexture | FrameBuffer::eDeleteFramebufferOnCleanup, 3);

bool			gUseAttachedIds=false;
GLuint			gAttachedFBO=0;
GLuint			gColorTextureId=0;
GLuint			gNormalTextureId=0;
GLuint			gDepthTextureId=0;
GLuint			gMaskTextureId=0;

GLuint			gNormalTextureMS=0;
GLuint			gDepthTextureMS=0;
GLuint			gMaskTextureMS=0;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void	SetAttachState(const bool defaultFramebuffer, const GLuint fbo)
{
	gUseAttachedIds = !defaultFramebuffer;
	gAttachedFBO = fbo;
}

void	SetAttachedTextures(const GLuint textureId, const GLuint normalId, const GLuint depthId, const GLuint maskId)
{
	gColorTextureId = textureId;
	gNormalTextureId = normalId;
	gDepthTextureId = depthId;
	gMaskTextureId = maskId;
}

void	SetAttachedTexturesMS( const GLuint normalMS, const GLuint depthMS, const GLuint maskMS )
{
	gNormalTextureMS = normalMS;
	gDepthTextureMS = depthMS;
	gMaskTextureMS = maskMS;
}

const GLuint gGetAttachedFBO()
{
	return gAttachedFBO;
}

FrameBuffer &GetDefaultFrameBuffer()
{
	return gDefaultFrameBuffer;
}

const bool gIsUseAttachedIds()
{
	return gUseAttachedIds;
}
const GLuint gGetColorTextureId()
{
	return gColorTextureId;
}
const GLuint gGetNormalTextureId()
{
	return gNormalTextureId;
}
const GLuint gGetDepthTextureId()
{
	return gDepthTextureId;
}
const GLuint gGetMaskTextureId()
{
	return gMaskTextureId;
}

const GLuint gGetNormalTextureIdMS()
{
	return gNormalTextureMS;
}
const GLuint gGetDepthTextureIdMS()
{
	return gDepthTextureMS;
}
const GLuint gGetMaskTextureIdMS()
{
	return gMaskTextureMS;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////

ObjectComposition *GetCurrentCompositePtr()
{
	if (gCurrentComposite.Ok() )
		return gCurrentComposite;
	return nullptr;
}

ObjectCompositionRender *FindRenderNodePtr(FBComponent *root)
{
	ObjectCompositionRender *pResult = nullptr;
	/*
	if ( FBIS(root, ObjectCompositionRender) ) {
		pResult = (ObjectCompositionRender*) root;
		return pResult;
	}
	*/
	for (int i=0; i<root->Components.GetCount(); ++i)
	{
		pResult = FindRenderNodePtr(root->Components[i]);
		if (pResult) return pResult;
	}

	return nullptr;
}

ObjectComposition *FindParentCompositePtr(FBComponent *startNode)
{
	FBComponent *pNode = nullptr;

	if (startNode)
		pNode = (startNode->Parents.GetCount() > 0) ? startNode->Parents[0] : nullptr;

	while (pNode)
	{
		if (FBIS(pNode, ObjectComposition) )
			return (ObjectComposition*) pNode;

		pNode = (pNode->Parents.GetCount() > 0) ? pNode->Parents[0] : nullptr;
	}

	return nullptr;
}
/*
EffectSSAO *FindEffectSSAOPtr(FBComponent *root)
{
	EffectSSAO *pResult = nullptr;
	
	for (int i=0; i<root->Components.GetCount(); ++i)
	{
		pResult = FindEffectSSAOPtr(root->Components[i]);
		if (pResult) return pResult;
	}

	return nullptr;
}
*/
/*
void ReplaceImageSource(FBComponent *root, FBTexture *pTexture)
{
	ObjectCompositionColor *pColor = nullptr;

	if ( FBIS(root, ObjectCompositionColor) ) {
		pColor = (ObjectCompositionColor*) root;
		
		if (pColor->Replaceable) {
			pColor->UseTexture = true;
			pColor->ReplaceTexture(pTexture);
		}
	}

	for (int i=0; i<root->Components.GetCount(); ++i)
	{
		ReplaceImageSource(root->Components[i], pTexture);
	}
}

void FetchImageSource(FBComponent *root)
{
	ObjectCompositionColor *pColor = nullptr;

	if ( FBIS(root, ObjectCompositionColor) ) {
		pColor = (ObjectCompositionColor*) root;
		
		if (pColor->Replaceable) {
			pColor->UseTexture = false;
			pColor->FetchTexture();
		}
	}

	for (int i=0; i<root->Components.GetCount(); ++i)
	{
		FetchImageSource(root->Components[i]);
	}
}
*/
void ClearCurrentAssignment()
{
	gCurrentComposite = nullptr;
}

ObjectComposition *GetOutputCompositePtr()
{
	if (gOutputComposite.Ok() )
		return gOutputComposite;
	return nullptr;
}

bool AnyCompositionWithRender()
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	int count = pScene->UserObjects.GetCount();
	for (int i=0; i<count; ++i)
		if ( FBIS(pScene->UserObjects[i], ObjectComposition) )
		{
			ObjectComposition *pFinal = (ObjectComposition*) pScene->UserObjects[i];
			/*
			if (pFinal->IsRenderNodeExist() )
			{
				return true;
			}
			*/
		}

	return false;
}

void ClearOutputCompositePtr()
{
	gOutputComposite = nullptr;
}

void SetOutputCompositePtr(FBComponent *exceptThis)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	int count = pScene->UserObjects.GetCount();
	for (int i=0; i<count; ++i)
		if ( FBIS(pScene->UserObjects[i], ObjectComposition) && (pScene->UserObjects[i] != exceptThis) )
		{
			((ObjectComposition*) pScene->UserObjects[i])->OutputToRender = false;
			break;
		}

	gOutputComposite = dynamic_cast<ObjectComposition*> (exceptThis);
}

void MakeSomeCompositeCurrent()
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	int count = pScene->UserObjects.GetCount();
	for (int i=0; i<count; ++i)
		if ( FBIS(pScene->UserObjects[i], ObjectComposition) )
		{
			//ChooseComposite( (ObjectComposition*) pScene->UserObjects[i]);
			break;
		}
}

void ChooseCompositeByName(const char *name)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	int count = pScene->UserObjects.GetCount();
	for (int i=0; i<count; ++i)
		if ( FBIS(pScene->UserObjects[i], ObjectComposition) && strcmp(pScene->UserObjects[i]->Name, name) == 0 )
		{
			//ChooseComposite( (ObjectComposition*) pScene->UserObjects[i]);
			break;
		}
}

ObjectComposition *FindCompositeByName(const char *name)
{
	FBScene *pScene = FBSystem::TheOne().Scene;
	ObjectComposition *pFinal = nullptr;

	int count = pScene->UserObjects.GetCount();
	for (int i=0; i<count; ++i)
		if ( FBIS(pScene->UserObjects[i], ObjectComposition) && strcmp(pScene->UserObjects[i]->Name, name) == 0 )
		{
			pFinal = (ObjectComposition*) pScene->UserObjects[i];
			break;
		}
	return pFinal;
}

ObjectCompositeBase *FindCompositeNodeByName(const char *name)
{
	FBScene *pScene = FBSystem::TheOne().Scene;
	ObjectCompositeBase *pNode = nullptr;

	int count = pScene->UserObjects.GetCount();
	for (int i=0; i<count; ++i)
		if ( FBIS(pScene->UserObjects[i], ObjectCompositeBase ) && strcmp(pScene->UserObjects[i]->Name, name) == 0 )
		{
			pNode = (ObjectComposition*) pScene->UserObjects[i];
			break;
		}
	return pNode;
}

void ChooseComposite(ObjectComposition *pNewComposite)
{
	gCurrentComposite = pNewComposite;
}


ObjectComposition *AddComposite(const char *name)
{
	ObjectComposition *newComposite = (ObjectComposition*) FBCreateObject( "FbxStorable/User", 
								"ObjectComposition", 
								name ) ;
	/*
	if (newComposite) {

		
	}
	*/
	return newComposite;
}


void RemoveComposite(ObjectComposition *composite)
{
	if (composite)
	{
		// remove mask components (channels, shapes, knots)

		if (gCurrentComposite.Ok() && (composite == gCurrentComposite)) 
			ClearCurrentAssignment();

		composite->FBDelete();
		composite = nullptr;

		MakeSomeCompositeCurrent();
	}
}

ObjectCompositeBase *DublicateCompositeNode(ObjectCompositeBase *pSourceNode)
{
	ObjectCompositeBase *newNode = nullptr;
	
	if (FBIS(pSourceNode, ObjectComposition) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
								"ObjectComposition", 
								pSourceNode->Name ) ;

		ObjectComposition *pFinal = (ObjectComposition*) newNode;

		if (pFinal) {

			// add a new texture and memory video clip and connect them
			FBTexture *pTexture = new FBTexture("CompositeTexture");
			FBVideoMemory *pVideo = new FBVideoMemory("CompositeVideo");

			pTexture->Video = pVideo;
			pFinal->Video.Clear();
			pFinal->Video.Add( pVideo );
		}
	}
	/*
	else if (FBIS(pSourceNode, ObjectCompositionRender) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"ObjectCompositionRender", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, ObjectCompositionColor) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"ObjectCompositionColor", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, ObjectCompositeBlend) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"ObjectCompositeBlend", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, ObjectEffectBlur) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"ObjectEffectBlur", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, ObjectEffectCSB) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"ObjectEffectCSB", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, ObjectEffectHalfTone) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"ObjectEffectHalfTone", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, ObjectEffect3dDOF) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"ObjectEffect3dDOF", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, ObjectEffect3dFog) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"ObjectEffect3dFog", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, ObjectSourceMask) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"ObjectSourceMask", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, EffectToonLines) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"EffectToonLines", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, EffectSSAO) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"EffectSSAO", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, EffectPosterization) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"EffectPosterization", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, EffectChangeColor) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"EffectChangeColor", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, EffectLUT) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"EffectLUT", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, EffectFilmGrain) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"EffectFilmGrain", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, EffectFXAA) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"EffectFXAA", 
										pSourceNode->Name ) ;
	}
	else if (FBIS(pSourceNode, EffectCrosshatch) )
	{
		newNode = (ObjectCompositeBase*) FBCreateObject( "FbxStorable/User", 
										"EffectCrosshatch", 
										pSourceNode->Name ) ;
	}
	*/
	if (newNode)
	{
		newNode->CopyFrom(pSourceNode);

		// dublicate all children
		for (int i=0; i<pSourceNode->Components.GetCount(); ++i)
		{
			ObjectCompositeBase *newChild = DublicateCompositeNode( (ObjectCompositeBase*) pSourceNode->Components[i] );
			newNode->Components.Add(newChild);
		}
	}

	return newNode;
}

ObjectComposition *DublicateComposite(ObjectComposition *composite)
{
	ObjectComposition *newComposite = (ObjectComposition*) DublicateCompositeNode(composite);
	return newComposite;
}


void RenameComposite(ObjectComposition *composite, const char *newname)
{
	if (composite && (newname != nullptr) && (strlen(newname) > 0) )
	{
		composite->Name = newname;
	}
}



///////////////////////////////////////////////////////////////////////////////////////////


GLuint FindDepthId(FBComponent *root)
{
	GLuint depthId = 0;

	auto renderNode = FindRenderNodePtr(root);
//	if (renderNode) depthId = renderNode->getDepthTextureId();

	return depthId;
}

GLuint FindNormalsId(FBComponent *root)
{
	GLuint normalsId = 0;

	auto renderNode = FindRenderNodePtr(root);
//	if (renderNode) normalsId = renderNode->getNormalsTextureId();

	return normalsId;
}

GLuint FindPosId(FBComponent *root)
{
	GLuint posId = 0;

	auto renderNode = FindRenderNodePtr(root);
//	if (renderNode) posId = renderNode->getPosTextureId();

	return posId;
}

GLuint FindMaskTextureId()
{
	return 0;
}

/////////////////////////////////////////////////
//

bool IsNodeFinal(FBComponent *pNode)
{
	return (FBIS(pNode, ObjectComposition) );
}

bool IsNodeFilter(FBComponent *pNode)
{
	/*
	return (FBIS(pNode, ObjectEffectBlur) || FBIS(pNode, ObjectEffectCSB) || FBIS(pNode, ObjectEffectHalfTone) 
		|| FBIS(pNode, ObjectEffect3dDOF) || FBIS(pNode, ObjectEffect3dFog) || FBIS(pNode, EffectToonLines) 
		|| FBIS(pNode, EffectSSAO) || FBIS(pNode, ObjectSourceMask) || FBIS(pNode, EffectPosterization) 
		|| FBIS(pNode, EffectChangeColor) || FBIS(pNode, EffectLUT) || FBIS(pNode, EffectFilmGrain)
		|| FBIS(pNode, EffectFXAA) || FBIS(pNode, EffectCrosshatch) );
		*/
	return (FBIS(pNode, ObjectCompositeFilter) );
}

bool IsNodeSource(FBComponent *pNode)
{
	return (FBIS(pNode, ObjectCompositeLayer) );
}


///////////////////////////////////////////////////////////////////////////
//



void ComputeCameraOrthoPoints(const float renderWidth, const float renderHeight, 
	FBModel *pCamera, double farPlane, double nearPlane, vec3 *points)
{
	FBMatrix modelview, invmodelview;
	FBVector3d camerapos;

	pCamera->GetVector(camerapos);
	//pCamera->GetCameraMatrix(modelview, kFBModelView);
	pCamera->GetMatrix(modelview);
	FBMatrixInverse(invmodelview, modelview);
	
	mat4 mvInv4;
	for (int i=0; i<16; ++i)
		mvInv4.mat_array[i] = (float) invmodelview[i];

	float fard = (float) farPlane;
	float neard = (float) nearPlane;

	//float fov = (float) DEG2RAD(FieldOfView);
	float ratio = renderWidth / renderHeight;

	mat3 invRot;
	mvInv4.get_rot(invRot);
	vec3 view_dir = invRot * vec3(0.0f, 0.0f, -1.0f);
	//vec3 view_dir(-0.7f, 0.0f, 0.7f);
	vec3 up = invRot * vec3(0.0f, 1.0f, 0.0f);
	//vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 right = invRot * vec3(1.0f, 0.0f, 0.0f);
	//vec3 right;
	//cross(right, view_dir, up);


	vec3 center( (float)camerapos[0], (float)camerapos[1], (float)camerapos[2] );
	//center = vec3(0.0f, 0.0f, 0.0f);
	//f.fard = pCache->farPlane;
	//f.neard = pCache->nearPlane;

	vec3 fc = center + fard*view_dir;
	vec3 nc = center + neard*view_dir;

	//right = normalize(right);
	//cross(up, right, view_dir);
	//up = normalize(up);

	neard = renderHeight;
	fard = renderHeight;

	// these heights and widths are half the heights and widths of
	// the near and far plane rectangles
	float near_height = neard;
	float near_width = near_height * ratio;
	float far_height = fard;
	float far_width = far_height * ratio;
	/*
	f.point[0] = nc - 0.5f*near_height*up - 0.5f*near_width*right;
	f.point[1] = nc + 0.5f*near_height*up - 0.5f*near_width*right;
	f.point[2] = nc + 0.5f*near_height*up + 0.5f*near_width*right;
	f.point[3] = nc - 0.5f*near_height*up + 0.5f*near_width*right;

	f.point[4] = fc - 0.5f*far_height*up - 0.5f*far_width*right;
	f.point[5] = fc + 0.5f*far_height*up - 0.5f*far_width*right;
	f.point[6] = fc + 0.5f*far_height*up + 0.5f*far_width*right;
	f.point[7] = fc - 0.5f*far_height*up + 0.5f*far_width*right;
	*/

	points[0] = nc - near_height*up - near_width*right;
	points[1] = nc + near_height*up - near_width*right;
	points[2] = nc + near_height*up + near_width*right;
	points[3] = nc - near_height*up + near_width*right;

	points[4] = fc - far_height*up - far_width*right; // bottom left
	points[5] = fc + far_height*up - far_width*right; // up left
	points[6] = fc + far_height*up + far_width*right; // up right
	points[7] = fc - far_height*up + far_width*right; // bottom right

}

// DONE: FIX culling winding

void ComputeCameraFrustumPoints(const float renderWidth, const float renderHeight, 
	FBModel *pCamera, double farPlane, double nearPlane, double FieldOfView, vec3 *points)
{
	FBMatrix modelview, invmodelview;
	FBVector3d camerapos;

	pCamera->GetVector(camerapos);
	if (FBIS(pCamera, FBCamera))
	{
		((FBCamera*)pCamera)->GetCameraMatrix(modelview, kFBModelView);
		FBMatrixInverse(invmodelview, modelview);
	}
	else
	{
		pCamera->GetMatrix(modelview);
		FBMatrixInverse(invmodelview, modelview);
		FBMatrixTranspose(invmodelview, invmodelview);
	}

	mat4 mvInv4;
	for (int i=0; i<16; ++i)
		mvInv4.mat_array[i] = (float) invmodelview[i];

	float fard = (float) farPlane;
	float neard = (float) nearPlane;

	float fov = nv_to_rad  * (float) (FieldOfView);
	float ratio = renderWidth / renderHeight;

	mat3 invRot;
	mvInv4.get_rot(invRot);
	vec3 view_dir = invRot * vec3(0.0f, 0.0f, -1.0f);
	//vec3 view_dir(-0.7f, 0.0f, 0.7f);
	vec3 up = invRot * vec3(0.0f, 1.0f, 0.0f);
	//vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 right = invRot * vec3(1.0f, 0.0f, 0.0f);
	//vec3 right;
	//cross(right, view_dir, up);


	vec3 center( (float)camerapos[0], (float)camerapos[1], (float)camerapos[2] );
	//center = vec3(0.0f, 0.0f, 0.0f);
	//f.fard = pCache->farPlane;
	//f.neard = pCache->nearPlane;

	vec3 fc = center + fard*view_dir;
	vec3 nc = center + neard*view_dir;

	//right = normalize(right);
	//cross(up, right, view_dir);
	//up = normalize(up);

	// these heights and widths are half the heights and widths of
	// the near and far plane rectangles
	float near_height = tan(fov/2.0f) * neard;
	float near_width = near_height * ratio;
	float far_height = tan(fov/2.0f) * fard;
	float far_width = far_height * ratio;
	/*
	f.point[0] = nc - 0.5f*near_height*up - 0.5f*near_width*right;
	f.point[1] = nc + 0.5f*near_height*up - 0.5f*near_width*right;
	f.point[2] = nc + 0.5f*near_height*up + 0.5f*near_width*right;
	f.point[3] = nc - 0.5f*near_height*up + 0.5f*near_width*right;

	f.point[4] = fc - 0.5f*far_height*up - 0.5f*far_width*right;
	f.point[5] = fc + 0.5f*far_height*up - 0.5f*far_width*right;
	f.point[6] = fc + 0.5f*far_height*up + 0.5f*far_width*right;
	f.point[7] = fc - 0.5f*far_height*up + 0.5f*far_width*right;
	*/

	points[0] = nc - near_height*up - near_width*right;
	points[1] = nc + near_height*up - near_width*right;
	points[2] = nc + near_height*up + near_width*right;
	points[3] = nc - near_height*up + near_width*right;

	points[4] = fc - far_height*up - far_width*right; // bottom left
	points[5] = fc + far_height*up - far_width*right; // up left
	points[6] = fc + far_height*up + far_width*right; // up right
	points[7] = fc - far_height*up + far_width*right; // bottom right

}