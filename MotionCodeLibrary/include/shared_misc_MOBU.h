
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_misc_MOBU.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>

// OR SDK
#include <fbsdk/fbsdk.h>


#include <GL\glew.h>

/////////////////////////////////////////////////////////
// helpful functions for shadow or reflections

void RenderModel(FBModel *pModel, bool useMaterials, bool bindColor, const bool recursive);
void DrawSceneModel(FBModel *pModel);

///////////////////////////////////////////////////////////////////////////
// work with property UI browser

void PropertiesClearAll(FBBrowsingProperty *pProperty);
void PropertiesAddExclusive(FBBrowsingProperty *pProperty, FBPlug *pPlug);
void PropertiesAdd(FBBrowsingProperty *pProperty, FBPlug *pPlug);

void PropertyCopy( FBPropertyAnimatableVector4d &to, FBPropertyAnimatableVector4d &from );
void PropertyCopy( FBPropertyAnimatableColor &to, FBPropertyAnimatableColor &from );
void PropertyCopy( FBPropertyAnimatableColorAndAlpha &to, FBPropertyAnimatableColorAndAlpha &from );
void PropertyCopy( FBPropertyAnimatableDouble &to, FBPropertyAnimatableDouble &from );
void PropertyCopy( FBPropertyAnimatableBool &to, FBPropertyAnimatableBool &from );
void PropertyCopy( FBPropertyBool &to, FBPropertyBool &from );
void PropertyCopy( FBPropertyInt &to, FBPropertyInt &from );
void PropertyCopy( FBPropertyDouble &to, FBPropertyDouble &from );


//////////////////////////////////////////////////////////////////////////