
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: camera_unproject_boxes.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define	BOXCLOSESTPOINT__CLASSNAME		Box_ClosestPoint
#define BOXCLOSESTPOINT__CLASSSTR		"Box_ClosestPoint"

#define	BOXUNPROJECT__CLASSNAME			Box_UnProject
#define BOXUNPROJECT__CLASSSTR			"Box_UnProject"

/**	Box for find closest ray<->mesh intersection point
*/
class Box_ClosestPoint : public FBBox
{
	//--- box declaration.
	FBBoxDeclare(Box_ClosestPoint, FBBox);

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat);

	//! FBX Retrieval function
	virtual bool FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat);

private:

	FBAnimationNode		*mNodeMesh;		//!> input - mesh node
	FBAnimationNode		*mRayOrigin;	//!> input - ray start position
	FBAnimationNode		*mRayEnd;		//!> input - ray direction point

	FBAnimationNode		*mIntersectPoint;		//!> output - mesh intersection world point
	FBAnimationNode		*mIntersectNormal;		//!> output - average normal at the intersection point
};



/**	UnProject u,v coords in local camera view space into the world space
*/
class Box_UnProject : public FBBox
{
	//--- box declaration.
	FBBoxDeclare(Box_UnProject, FBBox);

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat);

	//! FBX Retrieval function
	virtual bool FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat);

private:

	FBAnimationNode		*mNodeCamera;		//!> input - camera node, take projection and modelview matrices
	FBAnimationNode		*mTexCoords;	//!> input - u,v in camera view space
	FBAnimationNode		*mDepth;		//!> input - computation z

	FBAnimationNode		*mWorldPoint;		//!> output - unprojected point in world space
};