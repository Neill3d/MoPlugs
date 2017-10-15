
/**	\file	orpyfbsdk_template_pyfunction.cxx
*/
#include "pyskybox_common.h"

#include "../shader_SkyBox/Panorama.h"
#include "..\Common\grabber_CubeMap.h"


void GrabStreetView( const char *filename, double lat, double lng, int zoom )
{
	// TODO: 
	CPanorama	panorama;
	panorama.Load( filename, lat, lng, zoom );
}


//--- Use the Macro BOOST_PYTHON_FUNCTION_OVERLOADS to help define function with default arguments.
//--- BOOST_PYTHON_FUNCTION_OVERLOADS(Shader__CreateShader_Wrapper_Overloads, ORCreateShader_Wrapper, "minimum number arguments", "maximum number arguments")
BOOST_PYTHON_FUNCTION_OVERLOADS(Global__GrabStreetView_Wrapper_Overloads, GrabStreetView, 4, 4)
BOOST_PYTHON_FUNCTION_OVERLOADS(Global__GrabViewportCubeMap_Wrapper_Overloads, GrabViewportCubeMap, 3, 3)
BOOST_PYTHON_FUNCTION_OVERLOADS(Global__MakePanorama_Wrapper_Overloads, MakePanorama, 4, 4)


//--- Define non-member function used in Python
void ORFunctionInit()
{    
	// skybox
	def("GrabStreetView", GrabStreetView, Global__GrabStreetView_Wrapper_Overloads());
	def("GrabViewportCubeMap", GrabViewportCubeMap, Global__GrabViewportCubeMap_Wrapper_Overloads());
	def("MakePanorama", MakePanorama, Global__MakePanorama_Wrapper_Overloads());
}