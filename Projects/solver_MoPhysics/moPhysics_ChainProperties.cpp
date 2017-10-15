
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: moPhysics_ChainProperties.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "moPhysics_ChainProperties.h"
#include <array>

FBPhysicalPropertiesImplementation( MOChainPhysProperties );
FBRegisterPhysicalProperties( "MOChainPhysProperties",
								MOChainPhysProperties,
								"Dynamic Chain",
								"Dynamic Chain Desc",
								FB_DEFAULT_SDK_ICON );


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//
bool MOChainPhysProperties::FBCreate()
{
	//
	mOnline = false;
	
	//
	// information properties

	FBPropertyPublish( this, Active, "Active", nullptr, nullptr );
	FBPropertyPublish( this, RootModel, "Root Model", nullptr, nullptr );
	
	RootModel.SetSingleConnect(true);
	RootModel.SetFilter( FBModel::GetInternalClassId() );

	Active = true;

	return true;
}

void MOChainPhysProperties::FBDestroy()
{
}

void AddPropertyViewForChainPhysics(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(MOCHAINPHYSPROPERTIES__CLASSSTR, pPropertyName, pHierarchy);
}

void MOChainPhysProperties::AddPropertiesToPropertyViewManager()
{
	
	AddPropertyViewForChainPhysics("Active", "");
}

bool MOChainPhysProperties::IsAssigned()
{
	if (RootModel.GetCount() == 0)
		return false;

	return true;
}