
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: newton_PUBLIC.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "..\Common_Physics\physics_common.h"

// create instances only with this function
PHYSICS_INTERFACE::IWorld		*CreateNewNewtonWorld(	const double scale,
														const int numberOfThreads, 
														const int physicsLoops, 
														const float physicsFPS );

PHYSICS_INTERFACE::ICar			*CreateNewNewtonCar(	PHYSICS_INTERFACE::IWorld *world, 
														const PHYSICS_INTERFACE::CarOptions *options, 
														const PHYSICS_INTERFACE::IQueryGeometry *carGeometry[5],
														const PHYSICS_INTERFACE::IQueryPath *curve );

PHYSICS_INTERFACE::IBody			*CreateNewNewtonBody( PHYSICS_INTERFACE::IWorld *world,
												const PHYSICS_INTERFACE::BodyOptions *options,
												const PHYSICS_INTERFACE::IQueryGeometry *geometry,
												bool convexhullshape );