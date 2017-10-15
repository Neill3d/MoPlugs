
/**	\file	orpyfbsdk_template_pyshadercg.cxx
*/
#include "py_bakeProjectors_common.h"


void BakeProjectorsFBView_Init()
{    
    REGISTER_FBWRAPPER_FORWARD( ViewBakeProjectors )
	class_<ViewBakeProjectors_Wrapper,bases<FBView_Wrapper>, Wrapper_Container_1_Opt<ViewBakeProjectors_Wrapper, char*>, boost::noncopyable >("ViewBakeProjectors",init<char*>())
		//! Add properties and available member methods as Setter and Getter for the property.
		//.add_property( "Type", &ORShaderSkyBox_Wrapper::GetType, &ORShaderSkyBox_Wrapper::SetType)	
		//.def( "LoadCubeMap", &ORShaderSkyBox_Wrapper::LoadCubeMap )
	;

	REGISTER_FBWRAPPER( ViewBakeProjectors );
}