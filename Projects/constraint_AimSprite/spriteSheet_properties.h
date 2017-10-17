
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: spriteSheet_properties.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define SPRITETEXPROPS__CLASSNAME				ObjectSpriteTextureProperties
#define SPRITETEXPROPS__CLASSSTR				"ObjectSpriteTextureProperties"

#define SPRITEPROPS__CLASSNAME					ObjectSpriteProperties
#define SPRITEPROPS__CLASSSTR					"ObjectSpriteProperties"

////////////////////////////////////////////////

#define DEFAULT_NUMBER_OF_FRAME_RANGE	3
#define MAX_NUMBER_OF_FRAME_RANGE		40

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// SpriteSheet Solver custom properties objects to specify input options

/**	Custom UserObject template.
*/
class ObjectSpriteTextureProperties : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(ObjectSpriteTextureProperties, FBUserObject)
	FBDeclareUserObject(ObjectSpriteTextureProperties);

public:
	ObjectSpriteTextureProperties(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);

public:

	FBPropertyInt			Type;			// 0-basic (use rows, cols for compute number of items, each item position)
											// 1 - xml based (use data inside a file to define number of items and positions)
	FBPropertyInt			Rows;
	FBPropertyInt			Cols;
	FBPropertyString		XmlFile;		// file with sprite sheet information (exported from TexturePacker)

	FBPropertyInt			SequenceCount;

	// dynamicaly added properties
	struct FrameRange
	{
		FBProperty			*inStartFrame;			// range start
		FBProperty			*inStopFrame;			// range stop

	} inFrameRange[MAX_NUMBER_OF_FRAME_RANGE];

	FBPropertyInt			FPS;		// input sequence frames per second

};

/////////////////////////////////////////////////////////////////////////////////
//
class ObjectSpriteProperties : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(ObjectSpriteProperties, FBUserObject)
	FBDeclareUserObject(ObjectSpriteProperties);

public:
	ObjectSpriteProperties(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);

public:

	FBPropertyBool			PlayLocal;
	// ?! LocalCache, Zero Non Playing
	FBPropertyTime			PlayOffset;
	FBPropertyBool			PlayLoop;
	FBPropertyDouble		PlaySpeed;

	FBPropertyBool			UseTrigger;
	FBPropertyBool			TriggerOnScaling;
	FBPropertyTime			TriggerDelay;

	FBPropertyInt			CurrentSequenceIndex;		// run a specified sequence in a sprite sheet

	FBPropertyBool			ChangeSequenceOnTrigger;	// if true, then change current sequence according to trigger input value (1 to set sequence 0, 2 to set 1, etc.
	FBPropertyBool			TriggerRandomSequence;		// randomly choose current sequence from the texture sequence count

};