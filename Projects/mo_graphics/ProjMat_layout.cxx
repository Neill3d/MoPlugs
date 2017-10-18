
/**	\file	projmat_layout.cxx
*/

//--- Class declarations
#include "projmat_layout.h"

//--- Registration define
#define ORMATERIALTEMPLATE__LAYOUT		ProjMatLayout

//--- FiLMBOX implementation and registration
FBMaterialLayoutImplementation(	ORMATERIALTEMPLATE__LAYOUT);
FBRegisterMaterialLayout	  (	ORMATERIALTEMPLATE__LAYOUT,
                                ORMATERIALCUSTOM__CLASSSTR,
                                FB_DEFAULT_SDK_ICON);

/************************************************
*	FiLMBOX constructor.
************************************************/
bool ProjMatLayout::FBCreate()
{
	FBMaterialLayout::FBCreate();

	mMaterialCustom	= (ProjMaterial*) (FBMaterial*) Material;

    UICreate	();
    UIConfigure	();
    UIReset		();

    return true;
}


/************************************************
*	FiLMBOX destructor.
************************************************/
void ProjMatLayout::FBDestroy()
{

}


/************************************************
*	Create the UI.
************************************************/
void ProjMatLayout::UICreate()
{
    int lS=5;
    int lH=17;
    int lW=300;

	AddRegion("MyColor","",lS,kFBAttachTop,"",1,lS,kFBAttachLeft,"",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"", 1);
	AddRegion("MyAnimatableColor","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyColor",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);
	
	AddRegion("MyColorAndAlpha","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyAnimatableColor",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);		
	AddRegion("MyAnimatableColorAndAlpha","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyColorAndAlpha",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);
	
	AddRegion("MyDouble","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyAnimatableColorAndAlpha",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);
	AddRegion("MyAnimatableDouble","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyDouble",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);
	
	AddRegion("MyVector2d","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyAnimatableDouble",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);
	AddRegion("MyAnimatableVector2d","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyVector2d",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);
	AddRegion("MyVector3d","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyAnimatableVector2d",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);
	AddRegion("MyAnimatableVector3d","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyVector3d",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);
	AddRegion("MyVector4d","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyAnimatableVector3d",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);
	AddRegion("MyAnimatableVector4d","",lS,kFBAttachLeft,"",1,lS,kFBAttachBottom,"MyVector4d",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"MyColor", 1);

}


/************************************************
*	Configure the UI.
************************************************/
void ProjMatLayout::UIConfigure()
{
	SetControl( "MyColor", mEditPropertyColor );
	SetControl( "MyAnimatableColor", mEditPropertyAnimatableColor );

	SetControl( "MyColorAndAlpha", mEditPropertyColorRGBA );
	SetControl( "MyAnimatableColorAndAlpha", mEditPropertyAnimatableColorRGBA );

	SetControl( "MyDouble", mEditPropertyDouble );
	SetControl( "MyAnimatableDouble", mEditPropertyAnimatableDouble );

	SetControl( "MyVector2d", mEditPropertyVector2D );
	SetControl( "MyAnimatableVector2d", mEditPropertyAnimatableVector2D );

	SetControl( "MyVector3d", mEditPropertyVector3D );
	SetControl( "MyAnimatableVector3d", mEditPropertyAnimatableVector3D );

	SetControl( "MyVector4d", mEditPropertyVector4D );
	SetControl( "MyAnimatableVector4d", mEditPropertyAnimatableVector4D );

	mEditPropertyColor.Caption = "Color";
	mEditPropertyColor.Property = &(mMaterialCustom->MyColor);
	mEditPropertyAnimatableColor.Property = &(mMaterialCustom->MyAnimatableColor);

	mEditPropertyColorRGBA.Property = &(mMaterialCustom->MyColorAndAlpha);
	mEditPropertyAnimatableColorRGBA.Property = &(mMaterialCustom->MyAnimatableColorAndAlpha);

	mEditPropertyDouble.Property = &(mMaterialCustom->MyDouble);
	mEditPropertyAnimatableDouble.Property = &(mMaterialCustom->MyAnimatableDouble);

	mEditPropertyVector2D.Property = &(mMaterialCustom->MyVector2d);
	mEditPropertyAnimatableVector2D.Property = &(mMaterialCustom->MyAnimatableVector2d);

	mEditPropertyVector3D.Property = &(mMaterialCustom->MyVector3d);
	mEditPropertyAnimatableVector3D.Property = &(mMaterialCustom->MyAnimatableVector3d);

	mEditPropertyVector4D.Property = &(mMaterialCustom->MyVector4d);
	mEditPropertyAnimatableVector4D.Property = &(mMaterialCustom->MyAnimatableVector4d);

}

/************************************************
*	Reset the UI from the device.
************************************************/
void ProjMatLayout::UIReset()
{

}
