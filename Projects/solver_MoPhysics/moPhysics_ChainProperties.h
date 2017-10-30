
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: moPhysics_ChainProperties.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#include "Common_Physics\physics_common.h"
#include "queryFBGeometry.h"

#define	MOCHAINPHYSPROPERTIES__CLASSNAME		MOChainPhysProperties
#define MOCHAINPHYSPROPERTIES__CLASSSTR			"MOChainPhysProperties"

////////////////////////////////////////////////////////////////////////////////////////
//

///////////////////////////////////////////////////////////////////////////////////////////
//
class MOChainPhysProperties : public FBPhysicalProperties
{
	FBPhysicalPropertiesDeclare( MOChainPhysProperties, FBPhysicalProperties );

public:
	
	//--- Creation & Destruction
	virtual bool			FBCreate() override;						//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	static void				AddPropertiesToPropertyViewManager();

	bool					IsAssigned();

public:

	FBPropertyBool			Active;

	FBPropertyListObject	RootModel;


private:

	bool							mOnline;
	
};