#ifndef __PROJMAT_LAYOUT_H__
#define __PROJMAT_LAYOUT_H__

/**	\file	ormaterial_template_layout.h
*/

//--- Class declaration
#include "projmat_material.h"

//! Simple device layout.
class ProjMatLayout : public FBMaterialLayout
{
	//--- FiLMBOX declaration.
	FBMaterialLayoutDeclare( ProjMatLayout, FBMaterialLayout );

public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();			//!< FiLMBOX constructor.
	virtual void FBDestroy();			//!< FiLMBOX destructor.

private:
	// UI Management
	void	UICreate	();
	void	UIConfigure	();
	void	UIReset		();

	// UI Callbacks
	void	OnButtonTestClick(HISender pSender, HKEvent pEvent);

private:
	ProjMaterial	*mMaterialCustom;

private:
	// UI Elements
	FBEditPropertyModern mEditPropertyColor;
	FBEditPropertyModern mEditPropertyAnimatableColor; 

	FBEditPropertyModern mEditPropertyColorRGBA; 
	FBEditPropertyModern mEditPropertyAnimatableColorRGBA; 

	FBEditPropertyModern mEditPropertyDouble; 
	FBEditPropertyModern mEditPropertyAnimatableDouble; 

	FBEditPropertyModern mEditPropertyVector2D;
	FBEditPropertyModern mEditPropertyAnimatableVector2D; 

	FBEditPropertyModern mEditPropertyVector3D; 
	FBEditPropertyModern mEditPropertyAnimatableVector3D; 

	FBEditPropertyModern mEditPropertyVector4D; 
	FBEditPropertyModern mEditPropertyAnimatableVector4D; 

};

#endif /* __PROJMAT_LAYOUT_H__ */
