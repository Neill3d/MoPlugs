#ifndef __MODEL_3D_SHADOW_H__
#define __MODEL_3D_SHADOW_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_3d_shadow.h
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

#include "Common_FBComponent_UpdateProps.h"

//--- Registration define

#define SHADOWZONE__CLASSNAME		ModelShadowZone
#define SHADOWZONE__CLASSSTR		"ModelShadowZone"
#define SHADOWZONE__DESCSTR	    "Model Shadow Zone"


//////////////////////////////////////////////////////////////////////////////////////////////////////
/**	Model Shadow Volume.
*/
class ModelShadowZone : public ObjectCompositeMarker, public CFBComponentUpdateProps
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( ModelShadowZone, ObjectCompositeMarker );

public:
	ModelShadowZone(const char *pName, HIObject pObject= NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;
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

	// important quality settings

	FBPropertyInt					Resolution;
	FBPropertyDouble				Bias;
	FBPropertyBool					EnableOffset;
	FBPropertyDouble				OffsetFactor;
	FBPropertyDouble				OffsetUnits;

	// components connections and properties

	FBPropertyListObject			MasterLight;	// connected light that casting a shadow inside the volume

	FBPropertyAnimatableDouble		Density;
	FBPropertyAnimatableDouble		Feather;

	FBPropertyListObject			IncludeList;	// reflection only this
	FBPropertyListObject			ExcludeList;	// skip these specified objects
	FBPropertyBool					UseGPUCache;

	FBPropertyDouble				NearPlane;
	FBPropertyDouble				FarPlane;

	FBPropertyBool					RenderTransparency;	// do a transparency pass for shadows
	FBPropertyBool					AutoVolumeCulling;	// render models which are only inside the volume

	FBPropertyInt					DrawNodeCount;
	FBPropertyInt					DrawGeomCacheCount;

	static int GetDrawNodeCount(HIObject object);
	static int GetDrawGeomCacheCount(HIObject object);
	static void SetUpdateAction(HIObject object, bool value);

	RenderingStats	&GetStats() {
		return mStats;
	}

protected:

	RenderingStats				mStats;
};


#endif /* __MODEL_FOG_H__ */
