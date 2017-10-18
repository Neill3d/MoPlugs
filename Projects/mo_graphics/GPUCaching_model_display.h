
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: GPUCaching_model_display.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define ORMODELGPUCACHE__CLASSNAME	ORModelGPUCache
#define ORMODELGPUCACHE__CLASSSTR	"ORModelGPUCache"
#define ORMODELGPUCACHE__DESCSTR	"GPU Cache Model Display"

/**	Custom Model template.
*/
class ORModelGPUCache : public FBModelCube
{
	//--- FiLMBOX Tool declaration.
	FBStorableClassDeclare( ORModelGPUCache, FBModelCube );

public:
	ORModelGPUCache(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

    virtual void CreateGeometry();
	void	ChangeGeometryBox(const FBVector3d vmin, const FBVector3d vmax);

	virtual bool HasCustomDisplay() override { return true; }

	/** Custom display function called when HasCustomDisplay returns true;
	*/
	virtual void CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight) override;

	/** Custom picking for selection
	*/
	virtual bool CustomModelPicking(	int pNbHits, unsigned int *pSelectBuffer, FBCamera* pCamera, 
										int pMouseX,int pMouseY,
										FBTVector* localRaySrc, FBTVector* localRayDir,
										FBTVector* pWorldRaySrc, FBTVector* pWorldRayDir,
										FBMatrix* pGlobalInverseMatrix,
										FBTVector* pOutPickedPoint) override;

    /** Custom Closest Ray Intersection
    *   Hover ruler endpoint over this model, then press and hold left mouse button, this callback will be invoked. 
    */
    virtual bool ClosestRayIntersection(const FBTVector& pRayOrigin, const FBTVector& pRayEnd, FBTVector& pIntersectPos, FBNormal& pIntersecNormal) override;

    /** Override from FBPlug 
    */
    virtual	bool PlugStateNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);

    void SetScale(double pScale)   { Scale = pScale; CreateGeometry(); }

    FBPropertyDouble	Scale;

private:
    FBMesh* mMesh;
    int mPickedSubItem;
};
