
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MoBuOpticalUtils.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

//
// work with FBModelOptical model markers
//  used for key shortcuts for cut and remove segment operations
//
void OpticalMarkerSegmentCutOnCurrentTime();
void OpticalMarkerSegmentRemoveOnCurrentTime();

void OpticalMarkerGapChangeInterpolation(int newMode);

void OpticalDone();