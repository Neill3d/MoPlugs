
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_3d_decal.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "compositeMaster_object.h"
#include "compositeMaster_shaders.h"

//--- Registration define

#define DECAL__CLASSNAME	ModelDecal
#define DECAL__CLASSSTR		"ModelDecal"
#define DECAL__DESCSTR	    "Model Decal"



//////////////////////////////////////////////////////////////////////////////////////////////////////
/**	ModelDecal.
*/
class ModelDecal : public ObjectCompositeMarker
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( ModelDecal, ObjectCompositeMarker );

public:
	ModelDecal(const char *pName, HIObject pObject= NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	
	/** Clone the model.
        *    This will duplicate the current model. 
        *    \return Newly created model.
        */
    virtual FBModel* Clone() override;

    /** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	
	static void AddPropertiesToPropertyViewManager();
	static void AddPropertyView(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false);

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

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

	/**    Store and Retrieve function that can be overloaded.
        *    \param    pFbxObject    FBX Object that is used to communicate I/O operations.
        *    \param    pStoreWhat    Which attributes are currently stored/retrieved.
        */
	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;


public:

	FBPropertyBool					Enabled;
	FBPropertyAnimatableDouble		Opacity;

	FBPropertyBaseEnum<ECompositeBlendType>	BlendMode;		//!< type of blending two images

	FBPropertyListObject				Texture;

	// resolution comes from connected texture

	FBPropertyAction					Refresh;

	// read-only informative properties
	FBPropertyDouble                    ResolutionWidth;        //!< <b>Read Write Property:</b> Resolution width.
    FBPropertyDouble                    ResolutionHeight;       //!< <b>Read Write Property:</b> Resolution height.
    FBPropertyDouble                    AspectRatio;			//!< <b>Read Write Property:</b> Pixel aspect ratio.
    FBPropertyBool						UsesStoryTrack;
	FBPropertyString					VideoClipPath;

	// store camera settings
	FBPropertyDouble                    NearPlaneDistance;      //!< <b>Read Write Property:</b> Near plane distance.
    FBPropertyDouble                    FarPlaneDistance;       //!< <b>Read Write Property:</b> Far plane distance.
    FBPropertyCameraType                ProjectionType;                   //!< <b>Read Write Property:</b> Type of camera
    FBPropertyAnimatableDouble          FOV;            //!< <b>Read Write Property:</b> Field of View (used when in horizontal or vertical aperture modes).
	FBPropertyAnimatableDouble			OrthoScale;

	// color correction per decal
	FBPropertyAnimatableDouble			Contrast;
	FBPropertyAnimatableDouble			Brightness;
	FBPropertyAnimatableDouble			Saturation;

	FBPropertyAnimatableDouble			Gamma;

	FBPropertyBool					Inverse;	// inverse colors

	FBPropertyAnimatableDouble		Hue;
	FBPropertyAnimatableDouble		HueSaturation;
	FBPropertyAnimatableDouble		Lightness;

	//

	FBPropertyAction					GrabValues;
	FBPropertyAction					ApplyToCurrent;
	FBPropertyAction					ApplyToPerspective;

	static void SetRefresh(HIObject object, bool value);
	static void SetGrabValues(HIObject object, bool value);
	static void SetApplyToCurrent(HIObject object, bool value);
	static void SetApplyToPerspective(HIObject object, bool value);

	void	UpdateProjectionMatrices();

	const double *GetModelViewMatrix() const {
		return mModelView;
	}
	const double *GetProjectionMatrix() const {
		return mProjection;
	}
	const double *GetFinalMatrix() const {
		return mFinalMatrix;
	}

	const GLuint64	GetTextureAddress();

	void MakeTextureResident();
	void MakeTextureNonResident();

	void BindTexture(const GLenum slot);
	void UnBindTexture(const GLenum slot);

	void RefreshConnectedTexture();

protected:

	GLuint				mLastTextureId;
	GLuint64			mLastTextureAddress;

	// constructed matrices for the projection decal object
	FBMatrix			mModelView;
	FBMatrix			mProjection;
	FBMatrix			mFinalMatrix;

	HdlFBPlugTemplate<FBComponent>	mSettings;

	void DoRefreshTextureInfo();
	void DoGrabValues();
	void DoApplyToCurrent();
	void DoApplyToPerspective();

	void DoApplyToCamera(FBCamera *pCamera);

	void ComputePerspectiveMatrices();
	void ComputeOrthoMatrices();

	void ComputeFinalProjectorMatrix();
};
