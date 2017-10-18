#ifndef __ORTEXTURE_ADVANCEBLEND_LAYOUT_H__
#define __ORTEXTURE_ADVANCEBLEND_LAYOUT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ortexture_advanceBlend_layout.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "ortexture_advanceBlend_texture.h"

//! Simple device layout.
class ORTextureAdvanceBlendLayout : public FBTextureLayout
{
	//--- FiLMBOX declaration.
	FBTextureLayoutDeclare( ORTextureAdvanceBlendLayout, FBTextureLayout );

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
	ORTextureAdvanceBlend		*mTextureCustom;

private:
	// UI Elements
	FBEditPropertyModern		mEditAuxLayer; 
};

#endif /* __ORTEXTURE_ADVANCEBLEND_LAYOUT_H__ */
