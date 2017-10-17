
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: pyediting_pyfunction.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996) // 'function': was declared deprecated.

//--- SDK include
#include <pyfbsdk/pyfbsdk.h>

#ifdef KARCH_ENV_WIN
#include <windows.h>
#endif

#include "pyediting_pyfunction.h"

boost::python::list GetVertexArray( FBModel_Wrapper *modelWrapper, bool afterDeform )
{
	FBModel *pModel = modelWrapper->mFBModel;
	FBModelVertexData *pData = pModel->ModelVertexData;

	int count = pData->GetVertexCount();
	FBVertex *pvertices = (FBVertex*) pData->GetVertexArray( kFBGeometryArrayID_Point, afterDeform );

	
	boost::python::list ret;

	for (int i=0; i<count; ++i)
	{
		ret.append( FBVertex_Wrapper(pvertices[i]) );
	}

	return ret;
}


//--- Use the Macro BOOST_PYTHON_FUNCTION_OVERLOADS to help define function with default arguments.
//--- BOOST_PYTHON_FUNCTION_OVERLOADS(Shader__CreateShader_Wrapper_Overloads, ORCreateShader_Wrapper, "minimum number arguments", "maximum number arguments")
BOOST_PYTHON_FUNCTION_OVERLOADS(Global__GetVertexArray_Overloads, GetVertexArray, 2, 2)


//--- Define non-member function used in Python
void ORFunctionInit()
{    
	// skybox
	def("GetVertexArray", GetVertexArray, Global__GetVertexArray_Overloads());
}