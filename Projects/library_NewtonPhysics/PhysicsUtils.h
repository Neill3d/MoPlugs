
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: PhysicsUtils.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <Newton.h>
#include "Newton_entities.h"
#include "Newton_hardware.h"

NewtonBody* CreateSimpleBody (NewtonWorld* const world, void* const userData, dFloat mass, const dMatrix& matrix, NewtonCollision* const collision, int materialId);
IBody* CreateSimpleSolid (CWorldManager* const scene, const void* const mesh, dFloat mass, const dMatrix& matrix, NewtonCollision* const collision, int materialId);


dVector FindFloor (const NewtonWorld* world, const dVector& origin, dFloat dist);