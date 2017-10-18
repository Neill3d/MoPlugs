#ifndef __PROJMAT_MATERIAL_H__
#define __PROJMAT_MATERIAL_H__

/**	\file	projmat_material.h
*	Class for projective mapping material.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define ORMATERIALCUSTOM__CLASSNAME	ProjMaterial
#define ORMATERIALCUSTOM__CLASSSTR	"ProjMaterial"
#define ORMATERIALCUSTOM__DESCSTR	"Projective mapping material"

/**	Custom Material template.
*/
FB_FORWARD(ProjMaterial)
class ProjMaterial : public FBMaterial
{
	//--- FiLMBOX Tool declaration.
	FBStorableClassDeclare( ProjMaterial, FBMaterial );

public:
	ProjMaterial(const char *pName = NULL, HIObject pObject=NULL);

	FBPropertyColor MyColor;
	FBPropertyAnimatableColor MyAnimatableColor;

	FBPropertyColorAndAlpha MyColorAndAlpha;
	FBPropertyAnimatableColorAndAlpha MyAnimatableColorAndAlpha;

	FBPropertyDouble MyDouble;
	FBPropertyAnimatableDouble MyAnimatableDouble;

	FBPropertyVector2d MyVector2d;
	FBPropertyAnimatableVector2d MyAnimatableVector2d;

	FBPropertyVector3d MyVector3d;
	FBPropertyAnimatableVector3d MyAnimatableVector3d;

	FBPropertyVector4d MyVector4d;
	FBPropertyAnimatableVector4d MyAnimatableVector4d;

	virtual const char* GetMaterialName() const;
	virtual const char* GetMaterialDesc() const;

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
};

#endif /* __PROJMAT_MATERIAL_H__ */
