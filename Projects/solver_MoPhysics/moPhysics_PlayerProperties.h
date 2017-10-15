
#pragma once

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "..\Common_Physics\physics_common.h"
#include "queryFBGeometry.h"

#define	MOPLAYERPHYSPROPERTIES__CLASSNAME		MOPlayerPhysProperties
#define MOPLAYERPHYSPROPERTIES__CLASSSTR		"MOPlayerPhysProperties"

///////////////////////////////////////////////////////////////////////////////////////////
//
class MOPlayerPhysProperties : public FBPhysicalProperties
{
	FBPhysicalPropertiesDeclare( MOPlayerPhysProperties, FBPhysicalProperties );

public:
	
	//--- Creation & Destruction
	virtual bool			FBCreate() override;						//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	
	static void				AddPropertiesToPropertyViewManager();

public:

	FBPropertyBool			Active;

private:

	bool							mOnline;
};