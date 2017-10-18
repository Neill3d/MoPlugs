#ifndef __MODEL_3D_FOG_H__
#define __MODEL_3D_FOG_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_3d_fog.h
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

#define FOGVOLUME__CLASSNAME	ModelFogVolume
#define FOGVOLUME__CLASSSTR		"ModelFogVolume"
#define FOGVOLUME__DESCSTR	    "Model Fog Volume"

#define FOGTARGET__CLASSNAME	ModelFogTarget
#define FOGTARGET__CLASSSTR		"ModelFogTarget"
#define FOGTARGET__DESCSTR	    "Model Fog Target"

//////////////////////////////////////////////////////////////////////////////////////////////////////
/**	Model Volume Fog.
*/
class ModelFogTarget : public ObjectCompositeMarker
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( ModelFogTarget, ObjectCompositeMarker );

public:
	ModelFogTarget(const char *pName, HIObject pObject= NULL);

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


public:
	FBPropertyBool					Enabled;

	//FBPropertyAnimatableColor			TargetColor;
	FBPropertyAnimatableDouble			TargetFeather;
	FBPropertyAnimatableDouble			TargetDensity;

	FBPropertyAnimatableDouble			TargetNear;
	FBPropertyAnimatableDouble			TargetFar;

	FBPropertyBaseEnum<ECompositeBlendType>	BlendMode;		//!< type of blending two images

protected:

	HdlFBPlugTemplate<FBComponent>	mSettings;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
/**	Model Volume Fog.
*/
class ModelFogVolume : public ObjectCompositeMarker
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( ModelFogVolume, ObjectCompositeMarker );

public:
	ModelFogVolume(const char *pName, HIObject pObject= NULL);

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


public:

	FBPropertyBool					Enabled;

	//FBPropertyAnimatableColorAndAlpha	VolumeColor;
	FBPropertyAnimatableDouble			VolumeFeather;
	FBPropertyAnimatableDouble			VolumeDensity;

	FBPropertyBaseEnum<ECompositeBlendType>	BlendMode;		//!< type of blending two images

protected:

	HdlFBPlugTemplate<FBComponent>	mSettings;
};


#endif /* __MODEL_FOG_H__ */
