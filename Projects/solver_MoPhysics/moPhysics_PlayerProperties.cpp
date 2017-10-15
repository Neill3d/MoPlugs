
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: moPhysics_PlayerProperties.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "moPhysics_PlayerProperties.h"
#include <array>

FBPhysicalPropertiesImplementation( MOPlayerPhysProperties );
FBRegisterPhysicalProperties( "MOPlayerPhysProperties",
								MOPlayerPhysProperties,
								"Phys Player",
								"Phys Player Desc",
								FB_DEFAULT_SDK_ICON );


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//
bool MOPlayerPhysProperties::FBCreate()
{
	//
	mOnline = false;
	
	//
	// information properties

	FBPropertyPublish( this, Active, "Active", nullptr, nullptr );
	Active = true;

	return true;
}

void MOPlayerPhysProperties::FBDestroy()
{
}

void AddPropertyViewForPlayerPhysics(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(MOPLAYERPHYSPROPERTIES__CLASSSTR, pPropertyName, pHierarchy);
}

void MOPlayerPhysProperties::AddPropertiesToPropertyViewManager()
{
	
	AddPropertyViewForPlayerPhysics("Active", "");
}
