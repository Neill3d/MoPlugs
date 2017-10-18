
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: GPUCaching_model_display.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "GPUCaching_model_display.h"
#include "GPUCaching_shader.h"
#include <GL\glew.h>


FBClassImplementation( ORModelGPUCache );					                            //!< Register class
FBStorableCustomModelImplementation( ORModelGPUCache, ORMODELGPUCACHE__DESCSTR );	//!< Register to the store/retrieve system
FBElementClassImplementation( ORModelGPUCache, FB_DEFAULT_SDK_ICON );			        //!< Register to the asset system

/************************************************
*	Constructor.
************************************************/
ORModelGPUCache::ORModelGPUCache( const char* pName, HIObject pObject ) 
    : FBModelCube( pName, pObject )
    , mMesh(NULL)
    , mPickedSubItem(-1)
{
    FBClassInit;
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ORModelGPUCache::FBCreate()
{
	ParentClass::FBCreate();

    ShadingMode = kFBModelShadingTexture;

    
    // create *fake* geometry for bounding box computation.
    //CreateGeometry();

    return true;
}

/************************************************
*	FiLMBOX Destructor.
************************************************/
void ORModelGPUCache::FBDestroy()
{
	ParentClass::FBDestroy();
}

/** CreateGeometry
* The purpose of this method is solely to give the model a valid bounding box
* for the purposes of framing. The bounding box is invalid if no geometry exists.
*/

void ORModelGPUCache::CreateGeometry()
{
    // create *fake* geometry for bounding box computation.
    FBMesh* lMesh = new FBMesh("Model Box");
    this->Geometry = lMesh;

	const double lScale = 1.0;

    lMesh->GeometryBegin();

    lMesh->VertexAdd(	-lScale,	lScale,	-lScale );
	lMesh->VertexAdd(	lScale,		lScale,	-lScale );
	lMesh->VertexAdd(	lScale,		lScale,	lScale );
	lMesh->VertexAdd(	-lScale,	lScale,	lScale );

    lMesh->VertexAdd(	-lScale,	-lScale,	-lScale );
	lMesh->VertexAdd(	lScale,		-lScale,	-lScale );
	lMesh->VertexAdd(	lScale,		-lScale,	lScale );
	lMesh->VertexAdd(	-lScale,	-lScale,	lScale );

	lMesh->PolygonBegin();
	lMesh->PolygonVertexAdd(0);
	lMesh->PolygonVertexAdd(1);
	lMesh->PolygonVertexAdd(2);
	lMesh->PolygonVertexAdd(3);
	lMesh->PolygonEnd();

	lMesh->PolygonBegin();
	lMesh->PolygonVertexAdd(4);
	lMesh->PolygonVertexAdd(5);
	lMesh->PolygonVertexAdd(6);
	lMesh->PolygonVertexAdd(7);
	lMesh->PolygonEnd();

    lMesh->GeometryEnd();
}

void ORModelGPUCache::ChangeGeometryBox(const FBVector3d vmin, const FBVector3d vmax)
{
	FBMesh *lMesh = (FBMesh*) (FBGeometry*) this->Geometry;
	if (lMesh == nullptr) return;

	lMesh->GeometryBegin();

	lMesh->VertexSet(	vmin[0],	vmax[1],	vmin[2], 0 );
	lMesh->VertexSet(	vmax[0],	vmax[1],	vmin[2], 1 );
	lMesh->VertexSet(	vmax[0],	vmax[1],	vmax[2], 2 );
	lMesh->VertexSet(	vmin[0],	vmax[1],	vmax[2], 3 );

    lMesh->VertexSet(	vmin[0],	vmin[1],	vmin[2], 4 );
	lMesh->VertexSet(	vmax[0],	vmin[1],	vmin[2], 5 );
	lMesh->VertexSet(	vmax[0],	vmin[1],	vmax[2], 6 );
	lMesh->VertexSet(	vmin[0],	vmin[1],	vmax[2], 7 );

	lMesh->GeometryEnd();
}

/** Custom display
*/
void ORModelGPUCache::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
	const int numberOfShaders = this->Shaders.GetCount();
	if (numberOfShaders == 0)
		return;
	
	FBShader *pShader = this->Shaders[0];
	if ( !FBIS(pShader, ORShaderGPUCache) ) 
		return;

	ORShaderGPUCache *pCacheShader = (ORShaderGPUCache*) pShader;

    FBMatrix		 MatrixView;
    FBMatrix		 MatrixProjection;

    FBViewingOptions* lViewingOptions = FBSystem::TheOne().Renderer->GetViewingOptions();
    bool lIsSelectBufferPicking = lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	pCacheShader->CustomDisplay( mPickedSubItem, pCamera, pShadingMode, pRenderPass, pPickingAreaWidth, pPickingAreaHeight );
}

/** Custom picking process for selection buffer based picking mode only
*/
bool ORModelGPUCache::CustomModelPicking(int pNbHits, unsigned int *pSelectBuffer, FBCamera* pCamera, 
                                              int pMouseX,int pMouseY,
                                              FBTVector* localRaySrc, FBTVector* localRayDir,
                                              FBTVector* pWorldRaySrc, FBTVector* pWorldRayDir,
                                              FBMatrix* pGlobalInverseMatrix,
                                              FBTVector* pOutPickedPoint)
{
    // no hits 
    if( pNbHits <= 0) 
        return false;

    mPickedSubItem = -1;

    for (int Count=0; Count<pNbHits; Count++) 
    {
        /*  Each hit record contains the following information stored as unsigned ints:
        *   - Number of names in the name stack for this hit record
        *   - Minimum depth value of primitives (range 0 to 2^32-1)
        *   - Maximum depth value of primitives (range 0 to 2^32-1)
        *   - Name stack contents (one name for each unsigned int).
        *   see http://www.opengl.org/archives/resources/faq/technical/selection.htm
        */

        unsigned int NameCount = *pSelectBuffer++;
        assert(NameCount == 1); 

        // Skip two items  (Min & Max Depth)
        pSelectBuffer+=2; 

        // Fetch the first name from stack.
        unsigned int NameIndex = *pSelectBuffer++;
		/*
        if(NameIndex==1)
        {
            mPickedSubItem = 0;

            if(pOutPickedPoint)
            {
                *pOutPickedPoint = FBTVector(100,100,100,0);
            }
        } 
        else if(NameIndex==2)
        {
            mPickedSubItem = 1;

            if(pOutPickedPoint)
            {
                *pOutPickedPoint = FBTVector(-100,100,100,0);
            }
        }
		*/
		mPickedSubItem = NameIndex;
    }

    FBTrace("Select %s SubItem[%d]", GetFullName(), mPickedSubItem);

    return true; 
}

bool ORModelGPUCache::ClosestRayIntersection(const FBTVector& pRayOrigin, const FBTVector& pRayEnd, FBTVector& pIntersectPos, FBNormal& pIntersecNormal)
{
	const int numberOfShaders = this->Shaders.GetCount();
	if (numberOfShaders == 0)
		return false;
	
	FBShader *pShader = this->Shaders[0];
	if ( !FBIS(pShader, ORShaderGPUCache) ) 
		return false;

	ORShaderGPUCache *pCacheShader = (ORShaderGPUCache*) pShader;

	// TODO:
	return pCacheShader->ClosestRayIntersection(pRayOrigin, pRayEnd, pIntersectPos, (FBTVector&) pIntersecNormal );

    // Here we always return the triangle center, user could write real intersection code for production usage. 
    //pIntersectPos = FBTVector(0,  2.0 * Scale / 3.0, 2.0 * Scale / 3.0); //return the center
    //pIntersecNormal = FBNormal(1, 0, 0);
    //return true;
}

/** Custom picking process for color buffer based picking mode.
*/
bool ORModelGPUCache::PlugStateNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData/*=NULL*/,void* pDataOld/*=NULL*/,int pDataSize/*=0*/)
{
    if (FBSystem::TheOne().Renderer->IDBufferPicking)
    {   
        switch(pAction)
        {
        case kFBSelect:
        case kFBReselect:
            {
                FBPickInfosList* lPickInfoList = FBCreatePickInfosList();
                if (FBSystem::TheOne().Renderer->GetLastPickInfoList(*lPickInfoList))
                {

                    for (int lIndex = 0; lIndex < lPickInfoList->GetCount(); lIndex++)
                    {
                        FBPickInfos lPickInfo = lPickInfoList->GetAt(lIndex);
                        if (lPickInfo.mModel == this)
                        {
                            FBTrace("Select %s SubItem[%d]", GetFullName(), lPickInfo.mSubItemIndex);
                            mPickedSubItem = lPickInfo.mSubItemIndex;
                        }
                    }
                }
            }
            break;
        case kFBUnselect:
            {           
                FBTrace("Unselect %s", GetFullName());
                mPickedSubItem = -1;
            }
            break;
        default:
            break;
        }
    }

    // Important, continue forward notification to parent class.
    return ParentClass::PlugStateNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ORModelGPUCache::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

bool ORModelGPUCache::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}
