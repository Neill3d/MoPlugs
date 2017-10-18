#ifndef __MODEL_CUBEMAP_RENDERING_H__
#define __MODEL_CUBEMAP_RENDERING_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_cubemap_rendering.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef ORSDK_DLL
	/** \def ORSDK_DLL
	*	Be sure that ORSDK_DLL is defined only once...
	*/
#ifdef BUILD_DLL
	#define ORSDK_DLL K_DLLEXPORT
#else 
	#define ORSDK_DLL K_DLLIMPORT
#endif

#endif

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "shared_rendering.h"
#include "Common_FBComponent_UpdateProps.h"

//--- Registration define
#define CUBEMAP_RENDERING__CLASSNAME		CubeMapRendering
#define CUBEMAP_RENDERING__CLASSSTR			"CubeMapRendering"
#define CUBEMAP_RENDERING__DESCSTR			"CubeMap Rendering"

#define CAMERA_RENDERING__CLASSNAME			CameraRendering
#define CAMERA_RENDERING__CLASSSTR			"CameraRendering"
#define CAMERA_RENDERING__DESCSTR			"Camera Rendering"

///////////////////////////////////////////////////////////////////
//
class ExtendedRenderingBase
{
public:

	// a constructor
	ExtendedRenderingBase();

	//! a destructor
	virtual ~ExtendedRenderingBase();

	void FBCreateCommonProps(FBComponent *pParent,
		fbExternalGetSetHandler OnClearActionClick,
		fbExternalGetSetHandler OnSaveActionClick,
		fbExternalGetSetHandler OnLoadActionClick,
		fbExternalGetSetHandler GetDrawNodeCount,
		fbExternalGetSetHandler GetGeomCacheCount,
		fbExternalGetSetHandler GetLastUpdateDuration,
		fbExternalGetSetHandler GetLastUpdateLocalTime);

public:
	FBPropertyBool					Enabled;

	FBPropertyBool					DrawDebug;		// draw near/far radius
	
	FBPropertyDouble				NearRadius;
	FBPropertyDouble				FarRadius;

	FBPropertyDouble				BlurFactor;
	FBPropertyInt					Resolution;
	FBPropertyBool					UseFog;			// enable fog on far distance
	FBPropertyColor					FogColor;
	FBPropertyBool					FogOnBackground;

	FBPropertyListObject			IncludeList;	// reflection only this
	FBPropertyListObject			ExcludeList;	// skip these specified objects
	FBPropertyBool					UseGPUCache;
	FBPropertyBool					RenderBackground;	// render objects under the background goal
	FBPropertyBool					RenderTransparency;
	FBPropertyBool					RenderSecondary;

	// DDS IO
	FBPropertyBool					UseFile;	// static or dynamic cubemap
	FBPropertyString				FileName;
	FBPropertyAction				Clear;
	FBPropertyAction				Save;
	FBPropertyAction				Load;

	// stats
	FBPropertyInt					DrawNodeCount;
	FBPropertyInt					DrawGeomCacheCount;
	FBPropertyDouble				LastUpdateDuration;
	FBPropertyTime					LastUpdateLocalTime;

	//

	FBPropertyInt					OutputWidth;
	FBPropertyInt					OutputHeight;

	FBPropertyBool					GenerateOutput;	// generate 2d texture from cubemap
	FBPropertyBool					GenerateMipMaps;
	FBPropertyListObject			VideoOut;

	

public:

	// TODO: rename and refactor !!
	bool CommonCheckForUpdate(bool isCubeMap);
	
	bool CheckForHiddenIncludeList();

	// 
	void AssignResult(const unsigned textureId, const int width, const int height);

	virtual void		CommonUpdate();
	virtual void		CommonClear();
	virtual void		CommonPutSaveInQueue();
	virtual void		CommonLoad();


	CubeMapRenderingData &GetData() {
		return mData;
	}

	RenderingStats	&GetStats() {
		return mStats;
	}

	virtual void DoSave(const unsigned textureId);

	//void	PrepOutputTextureObject(const bool allocDepth, const int width, const int height);

	virtual void PrepOutputTextureObject(const int newWidth, const int newHeight);


protected:

	CubeMapRenderingData		mData;

    int mPickedSubItem;

	FBSystem					mSystem;
	
	RenderingStats				mStats;

	static FBVideoMemory	*CreateDynamicVideo(const bool createTexture, const char *textureName, const char *videoName, const char *textureFolder, const char *videoFolder);
	void	FreeTextures();
};


/////////////////////////////////////////////////////////////////////////
/** Camera User Object for rendering.
*/

class ORSDK_DLL CameraRendering : public FBUserObject, 
									public ExtendedRenderingBase,
									public CFBComponentUpdateProps
{
	//--- FiLMBOX class declaration.
	FBClassDeclare(CameraRendering, FBUserObject)
	FBDeclareUserObject(CameraRendering);

public:

	CameraRendering(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	//virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	//virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);

	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }

	// put properties into the folders
	static void AddPropertiesToPropertyViewManager();

	static void OnUpdateActionClick(HIObject object, bool value);
	static void OnSaveActionClick(HIObject object, bool value);
	static void OnLoadActionClick(HIObject object, bool value);
	static void OnClearActionClick(HIObject object, bool value);

	static int GetDrawNodeCount(HIObject object);
	static int GetDrawGeomCacheCount(HIObject object);
	static double GetUpdateDuration(HIObject object);
	static FBTime GetUpdateLocalTime(HIObject object);

public: // properties

	// render from that camera
	FBPropertyListObject			Camera;

};


///////////////////////////////////////////////////////////////////////////////////////////////
/**	CubeMap User Object for rendering.
*/
class CubeMapRendering : public FBModelMarker, 
					public ExtendedRenderingBase,
					public CFBComponentUpdateProps
{
	//--- FiLMBOX class declaration.
	//FBClassDeclare(CubeMapRendering, FBModelMarker)
	//FBDeclareUserObject(CubeMapRendering);

	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( CubeMapRendering, FBModelMarker );

public:
	CubeMapRendering(const char *pName, HIObject pObject= NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	//virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;
	//virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

    /** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }

	// put properties into the folders
	static void AddPropertiesToPropertyViewManager();

	/** Function to overload to handle custom display
    *    \return True is this model class has custom display function ModelDisplay
    */
    virtual bool HasCustomDisplay() override
	{
		return true;
	}

	/** Custom display function, called when HasCustomDisplay returns true;
    *    \param    pCamera                  Current camera for display.
    *    \param    pShadingMode             Shading mode to render see FBModelShadingMode.
    *    \param    pRenderPass              Actual rendering pass, see FBModelRenderPass.
    *    \param    pPickingAreaWidth        Width of picking area.
    *    \param    pPickingAreaHeight       Height of picking area.
    */
    virtual void CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight) override;
	
	static void OnUpdateActionClick(HIObject object, bool value);
	static void OnSaveActionClick(HIObject object, bool value);
	static void OnLoadActionClick(HIObject object, bool value);
	static void OnClearActionClick(HIObject object, bool value);

	static int GetDrawNodeCount(HIObject object);
	static int GetDrawGeomCacheCount(HIObject object);
	static double GetUpdateDuration(HIObject object);
	static FBTime GetUpdateLocalTime(HIObject object);

public:

	//
	//FBPropertyListObject			Point;	// point is a center position of a cubemap

	//
	//	parallax effect on cubemap reflection

	FBPropertyBool					UseParallaxCorrection;
	FBPropertyVector3d				RoomMax;
	FBPropertyVector3d				RoomMin;
	
	// panorama real-time output
	FBPropertyBool					AutoSizeOutput;

public:

	virtual void		CommonClear() override;
	virtual void		CommonPutSaveInQueue() override;
	virtual void		CommonLoad() override;

	virtual void DoSave(const unsigned textureId) override;

	virtual void PrepOutputTextureObject(const int newWidth, const int newHeight) override;

	const int		GetLastProcessedFace() const {
		return mLastCubeMapFace;
	}
	const int		GetNumberOfFaces() const {
		return 6;
	}
	void IncCubeMapFaceIndex() {
		mLastCubeMapFace += 1;
		if (mLastCubeMapFace > 5)
			mLastCubeMapFace = 0;
	}

	virtual void ResetUpdateValues() override
	{
		mLastCubeMapFace = 0;
		CFBComponentUpdateProps::ResetUpdateValues();
	}

protected:

	int					mLastCubeMapFace;

};


#endif /* __MODEL_COLLISION_SPHERE_H__ */
