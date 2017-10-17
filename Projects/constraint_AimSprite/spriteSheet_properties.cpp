
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: spriteSheet_properties.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "spriteSheet_properties.h"

//--- Class declaration

/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define PropertiesClassImplementation(ClassName, BrowsingName, IconFileName)\
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
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Elements/Sprite Properties", BrowsingName, "", RegisterElement##ClassName##Create, true, IconFileName);}


FBClassImplementation(ObjectSpriteTextureProperties)
FBUserObjectImplement(ObjectSpriteTextureProperties, "Properties for the Sprite Sheet Texture", FB_DEFAULT_SDK_ICON);	//Register UserObject class
PropertiesClassImplementation(ObjectSpriteTextureProperties, "Sprite Texture Properties", FB_DEFAULT_SDK_ICON);					//Register to the asset system

FBClassImplementation(ObjectSpriteProperties)
FBUserObjectImplement(ObjectSpriteProperties, "Properties for the Sprite Sheet Models", FB_DEFAULT_SDK_ICON);	//Register UserObject class
PropertiesClassImplementation(ObjectSpriteProperties, "Sprite Model Properties", FB_DEFAULT_SDK_ICON);					//Register to the asset system

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectSpriteTextureProperties

/************************************************
 *	Constructor.
 ************************************************/
ObjectSpriteTextureProperties::ObjectSpriteTextureProperties( const char* pName, HIObject pObject ) 
	: FBUserObject( pName, pObject )
{
	FBClassInit;

	FBPropertyPublish( this, Type, "Type", nullptr, nullptr );

	FBPropertyPublish( this, Rows, "Number Of Rows", nullptr, nullptr );
	FBPropertyPublish( this, Cols, "Number Of Cols", nullptr, nullptr );
	FBPropertyPublish( this, XmlFile, "Xml File", nullptr, nullptr );

	FBPropertyPublish( this, SequenceCount, "Sequence Count", nullptr, nullptr );

	FBPropertyPublish( this, FPS, "FPS", nullptr, nullptr );

	Type = 0;
	Rows = 4;
	Cols = 4;
	XmlFile = "";

	SequenceCount = 3;
	FPS = 12;
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectSpriteTextureProperties::FBCreate()
{
	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ObjectSpriteTextureProperties::FBDestroy()
{
}

bool ObjectSpriteTextureProperties::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	
	return true;
}

bool ObjectSpriteTextureProperties::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectSpriteProperties

/************************************************
 *	Constructor.
 ************************************************/
ObjectSpriteProperties::ObjectSpriteProperties( const char* pName, HIObject pObject ) 
	: FBUserObject( pName, pObject )
{
	FBClassInit;

	FBPropertyPublish( this, PlayLocal, "Play Local", nullptr, nullptr );
	FBPropertyPublish( this, PlayOffset, "Play Offset", nullptr, nullptr );
	FBPropertyPublish( this, PlayLoop, "Play Loop", nullptr, nullptr );
	FBPropertyPublish( this, PlaySpeed, "Play Speed", nullptr, nullptr );

	FBPropertyPublish( this, UseTrigger, "Use Triggers", nullptr, nullptr );
	FBPropertyPublish( this, TriggerOnScaling, "Trigger On Scaling", nullptr, nullptr );
	FBPropertyPublish( this, TriggerDelay, "Trigger Delay", nullptr, nullptr );

	// ?! this property should be individualy for each connected model ? make a reference here perhaps
	FBPropertyPublish( this, CurrentSequenceIndex, "Current Sequence Index", nullptr, nullptr );

	FBPropertyPublish( this, ChangeSequenceOnTrigger, "Change Sequence On Trigger", nullptr, nullptr );
	FBPropertyPublish( this, TriggerRandomSequence, "Trigger Random Sequence", nullptr, nullptr );

	PlayLocal = false;
	PlayOffset = FBTime(0);
	PlayLoop = true;
	PlaySpeed = 100.0;

	UseTrigger = false;
	TriggerOnScaling = false;
	TriggerDelay = FBTime(0);

	CurrentSequenceIndex = 0;

	ChangeSequenceOnTrigger = false;
	TriggerRandomSequence = false;
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectSpriteProperties::FBCreate()
{
	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ObjectSpriteProperties::FBDestroy()
{
}

bool ObjectSpriteProperties::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	
	return true;
}

bool ObjectSpriteProperties::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}