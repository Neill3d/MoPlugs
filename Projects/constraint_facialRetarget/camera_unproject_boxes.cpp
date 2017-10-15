
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: camera_unproject_boxes.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "camera_unproject_boxes.h"


//--- Registration defines
#define BOXCLOSESTPOINT__CLASS			BOXCLOSESTPOINT__CLASSNAME
#define BOXCLOSESTPOINT__NAME			BOXCLOSESTPOINT__CLASSSTR
#define	BOXCLOSESTPOINT__LOCATION		"MoPlugs"
#define BOXCLOSESTPOINT__LABEL			"Closest Point"
#define	BOXCLOSESTPOINT__DESC			"Cast a ray and returns position and normal"

#define BOXUNPROJECT__CLASS			BOXUNPROJECT__CLASSNAME
#define BOXUNPROJECT__NAME			BOXUNPROJECT__CLASSSTR
#define	BOXUNPROJECT__LOCATION		"MoPlugs"
#define BOXUNPROJECT__LABEL			"UnProject"
#define	BOXUNPROJECT__DESC			"Convert uv camera into the world point"


//--- implementation and registration
FBBoxImplementation(BOXCLOSESTPOINT__CLASS);	// Box class name
FBRegisterBox(BOXCLOSESTPOINT__NAME,			// Unique name to register box.
	BOXCLOSESTPOINT__CLASS,			// Box class name
	BOXCLOSESTPOINT__LOCATION,		// Box location ('plugins')
	BOXCLOSESTPOINT__LABEL,			// Box label (name of box to display)
	BOXCLOSESTPOINT__DESC,			// Box long description.
	FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXUNPROJECT__CLASS);	// Box class name
FBRegisterBox(BOXUNPROJECT__NAME,			// Unique name to register box.
	BOXUNPROJECT__CLASS,			// Box class name
	BOXUNPROJECT__LOCATION,		// Box location ('plugins')
	BOXUNPROJECT__LABEL,			// Box label (name of box to display)
	BOXUNPROJECT__DESC,			// Box long description.
	FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)


////////////////////////////////////////////////////////////////////////////////////
//


/************************************************
*	Creation
************************************************/
bool Box_ClosestPoint::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	if (FBBox::FBCreate())
	{
		// Create the input node.
		mNodeMesh = AnimationNodeInCreate(0, "Mesh", ANIMATIONNODE_TYPE_VECTOR);
		mRayOrigin = AnimationNodeInCreate(1, "Ray Origin", ANIMATIONNODE_TYPE_VECTOR);
		mRayEnd = AnimationNodeInCreate(2, "Ray End", ANIMATIONNODE_TYPE_VECTOR);

		// Create the output nodes
		mIntersectPoint = AnimationNodeOutCreate(3, "Point", ANIMATIONNODE_TYPE_VECTOR);
		mIntersectNormal = AnimationNodeOutCreate(4, "Normal", ANIMATIONNODE_TYPE_VECTOR);

		return true;
	}
	return false;
}

/************************************************
*	Destruction.
************************************************/
void Box_ClosestPoint::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
	//	FBBox::Destroy();
}




/************************************************
*	Real-time engine evaluation
************************************************/
bool Box_ClosestPoint::AnimationNodeNotify(FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo)
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/


	FBTVector		lMeshVector, lRayOrigin, lRayEnd;


	bool			lStatus;	// Status of input node

	// Read the input nodes.
	if (!mNodeMesh->ReadData(lMeshVector, pEvaluateInfo)) return false;
	if (!mRayOrigin->ReadData(lRayOrigin, pEvaluateInfo)) return false;
	if (!mRayEnd->ReadData(lRayEnd, pEvaluateInfo)) return false;

	// If the read was not from a dead node.
	int count = mNodeMesh->GetSrcCount();
	for (int i = 0; i<count; ++i)
	{
		FBPlug *pPlug = mNodeMesh->GetSrc(i);
		pPlug = pPlug->GetOwner();

		if (pPlug->Is(FBModelPlaceHolder::TypeInfo))
		{
			FBModelPlaceHolder *pPlaceHolder = (FBModelPlaceHolder*)pPlug;
			FBModel *pModel = pPlaceHolder->Model;

			//FBGeometry *pGeom = pModel->Geometry;
			//FBString name = pModel->Name;
			//printf("%s\n", name);

			FBTVector pos, nor;

			FBMatrix m, invM;

			pModel->GetMatrix(m, kModelTransformation_Geometry);
			FBMatrixInverse(invM, m);

			FBTVector localRayOrigin, localRayEnd;

			FBVectorMatrixMult(localRayOrigin, invM, lRayOrigin);
			FBVectorMatrixMult(localRayEnd, invM, lRayEnd);

			if (true == pModel->ClosestRayIntersection(localRayOrigin, localRayEnd, pos, (FBNormal&)nor))
			{
				FBVectorMatrixMult(pos, m, pos);

				mIntersectPoint->WriteData(pos, pEvaluateInfo);
				mIntersectNormal->WriteData(nor, pEvaluateInfo);
				return true;
			}
		}
	}


	return false;
}


/************************************************
*	FBX Storage.
************************************************/
bool Box_ClosestPoint::FbxStore(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat)
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
*	FBX Retrieval.
************************************************/
bool Box_ClosestPoint::FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat)
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}



////////////////////////////////////////////////////////////////////////////////////
//


/************************************************
*	Creation
************************************************/
bool Box_UnProject::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	if (FBBox::FBCreate())
	{
		// Create the input node.
		mNodeCamera = AnimationNodeInCreate(0, "Camera", ANIMATIONNODE_TYPE_VECTOR);
		mTexCoords = AnimationNodeInCreate(1, "TexCoords", ANIMATIONNODE_TYPE_VECTOR);
		mDepth = AnimationNodeInCreate(2, "Depth", ANIMATIONNODE_TYPE_NUMBER);

		// Create the output nodes
		mWorldPoint = AnimationNodeOutCreate(3, "Point", ANIMATIONNODE_TYPE_VECTOR);

		return true;
	}
	return false;
}

/************************************************
*	Destruction.
************************************************/
void Box_UnProject::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
	//	FBBox::Destroy();
}




/************************************************
*	Real-time engine evaluation
************************************************/
bool Box_UnProject::AnimationNodeNotify(FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo)
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/

	FBTVector		lCameraVector, lTexCoords;
	double			lDepth;


	bool			lStatus;	// Status of input node

	// Read the input nodes.
	if (!mNodeCamera->ReadData(lCameraVector, pEvaluateInfo)) return false;
	if (!mTexCoords->ReadData(lTexCoords, pEvaluateInfo)) return false;
	if (!mDepth->ReadData(&lDepth, pEvaluateInfo)) return false;

	// If the read was not from a dead node.
	int count = mNodeCamera->GetSrcCount();
	for (int i = 0; i < count; ++i)
	{
		FBPlug *pPlug = mNodeCamera->GetSrc(i);
		pPlug = pPlug->GetOwner();

		if (pPlug->Is(FBModelPlaceHolder::TypeInfo))
		{
			FBModelPlaceHolder *pPlaceHolder = (FBModelPlaceHolder*)pPlug;
			FBModel *pModel = pPlaceHolder->Model;

			if (FBIS(pModel, FBCamera))
			{
				FBCamera *pCamera = (FBCamera*)pModel;

				FBMatrix projMatrix, mdlMatrix, m, invM;

				pCamera->GetCameraMatrix(projMatrix, kFBProjection);
				pCamera->GetCameraMatrix(mdlMatrix, kFBModelView);
				pCamera->GetCameraMatrix(m, kFBModelViewProj);
				FBMatrixInverse(invM, m);

				lTexCoords[0] = lTexCoords[0] * 2.0 - 1.0;
				lTexCoords[1] = lTexCoords[1] * 2.0 - 1.0;
				lTexCoords[2] = lDepth;
				lTexCoords[3] = 1.0;

				FBTVector lResult;

				FBVectorMatrixMult(lResult, invM, lTexCoords);

				if (lResult[3] != 0.0)
				{
					lResult[0] /= lResult[3];
					lResult[1] /= lResult[3];
					lResult[2] /= lResult[3];
					lResult[3] = 1.0;
				}
				
				mWorldPoint->WriteData(lResult, pEvaluateInfo);
			}

			return true;
		}
	}
	
	return false;
}


/************************************************
*	FBX Storage.
************************************************/
bool Box_UnProject::FbxStore(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat)
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
*	FBX Retrieval.
************************************************/
bool Box_UnProject::FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat)
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}