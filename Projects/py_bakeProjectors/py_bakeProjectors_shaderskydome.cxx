
/**	\file	orpyfbsdk_template_pyshadercg.cxx
*/
#include "pyskybox_common.h"


void ORShaderSkyDome_Init()
{    
    REGISTER_FBWRAPPER_FORWARD( ORShaderSkyDome )
	class_<ORShaderSkyDome_Wrapper,bases<FBShader_Wrapper>, Wrapper_Container_1_Opt<ORShaderSkyDome_Wrapper, char*>, boost::noncopyable >("ORShaderSkyDome",init<char*>())
		//! Add properties and available member methods as Setter and Getter for the property.
		//.add_property( "Type", &ORShaderSkyBox_Wrapper::GetType, &ORShaderSkyBox_Wrapper::SetType)	
		//.def( "LoadCubeMap", &ORShaderSkyBox_Wrapper::LoadCubeMap )
	;

	REGISTER_FBWRAPPER( ORShaderSkyDome );
}