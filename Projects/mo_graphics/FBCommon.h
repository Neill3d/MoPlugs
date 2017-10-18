
#pragma once

/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define FBElementClassImplementationMasking(ClassName,IconFileName)\
	HIObject RegisterElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
		ClassName* Class = new ClassName(pName);\
		Class->mAllocated = true;\
		if( Class->FBCreate() ){\
            __FBRemoveModelFromScene( Class->GetHIObject() ); /* Hack in MoBu2013, we shouldn't add object to the scene/entity automatically*/\
			return Class->GetHIObject();\
		} else {\
			delete Class;\
			return NULL;}}\
	FBLibraryModule(ClassName##Element){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements/Masking", #ClassName, "", RegisterElement##ClassName##Create, true, IconFileName);}


/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define FBElementClassImplementationRendering(ClassName,AssetName,IconFileName)\
	HIObject RegisterElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
		ClassName* Class = new ClassName(pName);\
		Class->mAllocated = true;\
		if( Class->FBCreate() ){\
            __FBRemoveModelFromScene( Class->GetHIObject() ); /* Hack in MoBu2013, we shouldn't add object to the scene/entity automatically*/\
			return Class->GetHIObject();\
		} else {\
			delete Class;\
			return NULL;}}\
	FBLibraryModule(ClassName##Element){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements/Rendering", AssetName, "", RegisterElement##ClassName##Create, true, IconFileName);}