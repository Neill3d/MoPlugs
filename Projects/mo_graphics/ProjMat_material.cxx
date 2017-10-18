
/**	\file	projmat_material.cxx
*/

//--- Class declaration
#include "projmat_material.h"


FBClassImplementation( ProjMaterial );					//Register class
FBStorableCustomMaterialImplementation( ProjMaterial, Material );	//Register to the store/retrieve system
FBShadingElementClassImplementation( ProjMaterial, "browsing/template_material.png" );			//Register to the asset system

/************************************************
 *	Constructor.
 ************************************************/
ProjMaterial::ProjMaterial( const char* pName, HIObject pObject ) 
	: FBMaterial( pName, pObject )
{
	FBClassInit;
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ProjMaterial::FBCreate()
{
	FBPropertyInitTextureConnectable(this, MyColor, "MyColor" );
	FBPropertyInitTextureConnectable(this, MyAnimatableColor, "MyAnimatableColor" );

	FBPropertyInitTextureConnectable(this, MyColorAndAlpha, "MyColorAndAlpha" );
	FBPropertyInitTextureConnectable(this, MyAnimatableColorAndAlpha, "MyAnimatableColorAndAlpha" );

	FBPropertyInitTextureConnectable(this, MyDouble, "MyDouble" );
	FBPropertyInitTextureConnectable(this, MyAnimatableDouble, "MyAnimatableDouble" );

	FBPropertyInitTextureConnectable(this, MyVector2d, "MyVector2d" );
	FBPropertyInitTextureConnectable(this, MyAnimatableVector2d, "MyAnimatableVector2d" );

	FBPropertyInitTextureConnectable(this, MyVector3d, "MyVector3d" );
	FBPropertyInitTextureConnectable(this, MyAnimatableVector3d, "MyAnimatableVector3d" );

	FBPropertyInitTextureConnectable(this, MyVector4d, "MyVector4d" );
	FBPropertyInitTextureConnectable(this, MyAnimatableVector4d, "MyAnimatableVector4d" );


	Ambient.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);
	AmbientFactor.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);

	Emissive.ModifyPropertyFlag(kFBPropertyFlagDisableProperty, true);
	EmissiveFactor.ModifyPropertyFlag(kFBPropertyFlagDisableProperty, true);

	return true;
}

const char* ProjMaterial::GetMaterialName() const
{
	return ORMATERIALCUSTOM__CLASSSTR;
}

const char* ProjMaterial::GetMaterialDesc() const
{
	return ORMATERIALCUSTOM__DESCSTR;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ProjMaterial::FBDestroy()
{
	ParentClass::FBDestroy();
}

bool ProjMaterial::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ProjMaterial::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}
