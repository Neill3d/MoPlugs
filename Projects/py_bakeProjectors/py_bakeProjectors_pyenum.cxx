
/**	\file	orpyfbsdk_template_pyenum.cxx
*/
#include "pyskybox_common.h"


//--- Define enums used in Python
void OREnum_Init()
{    
	//! enum_<"The C++ enum type associated with">("The enum type name used in Python")
	
	enum_<FBSkyBoxType>("FBSkyBoxType")
		.value("kFBSkyBoxTypeImages",					FBSkyBoxType::kFBSkyBoxTypeImages)
		.value("kFBSkyBoxTypeCubeMap",					FBSkyBoxType::kFBSkyBoxTypeCubeMap)
		;
}