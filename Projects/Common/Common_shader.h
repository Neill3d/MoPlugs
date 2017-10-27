
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Common_shader.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

/**	Point cloud model.
*/
/*
enum EShadingType {
		eShadingTypeDynamic,
		eShadingTypeFlat,
		eShadingTypeToon,
		eShadingTypeMatte
		//eShadingTypeCubeMap
	};
	*/
//const char * FBPropertyBaseEnum<EShadingType>::mStrings[] = {"Phong", "Flat", "Toon", "Matte", 0};

enum ProjectorMaskEnum
{
	ProjectorMask0, // 0
	ProjectorMask1, 
};

const char * FBPropertyBaseEnum<ProjectorMaskEnum>::mStrings[] = {
	"Mask0",
	"Mask1"
};

enum ProjectorMaskChannelEnum
{
	ProjectorMaskChannelR, // 0
	ProjectorMaskChannelG,
	ProjectorMaskChannelB,
	ProjectorMaskChannelA
};

const char * FBPropertyBaseEnum<ProjectorMaskChannelEnum>::mStrings[] = {
	"Channel R",
	"Channel G",
	"Channel B",
	"Channel A"
};

enum ERenderOrder
{
	eRenderOrderNormal,
	eRenderOrderBack,
	eRenderOrderFront
};

enum EBlendType
{
	eBlendNormal, // 0
	eBlendLighten, 
	eBlendDarken,
	eBlendMultiply,
	eBlendAverage,
	eBlendAdd,
	eBlendSubstract,
	eBlendDifference,
	eBlendNegation,
	eBlendExclusion,
	eBlendScreen,
	eBlendOverlay,
	eBlendSoftLight,
	eBlendHardLight,
	eBlendColorDodge,
	eBlendColorBurn,
	eBlendLinearDodge,
	eBlendLinearBurn,
	// Linear Light is another contrast-increasing mode
	// If the blend color is darker than midgray, Linear Light darkens the image by decreasing the brightness. If the blend color is lighter than midgray, the result is a brighter image due to increased brightness.
	eBlendLinearLight,
	eBlendVividLight,
	eBlendPinLight,
	eBlendHardMix,
	eBlendReflect,
	eBlendGlow,
	eBlendPhoenix
};

const char * FBPropertyBaseEnum<EBlendType>::mStrings[] = {
	"Normal",
	"Lighten",
	"Darken",
	"Multiply",
	"Average",
	"Add",
	"Substract",
	"Difference",
	"Negation",
	"Exclusion",
	"Screen",
	"Overlay",
	"SoftLight",
	"HardLight",
	"ColorDodge",
	"ColorBurn",
	"LinearDodge",
	"LinearBurn",
	"LinearLight",
	"VividLight",
	"PinLight",
	"HardMix",
	"Reflect",
	"Glow",
	"Phoenix",
	0};