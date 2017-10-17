#ifndef __ORPYFBSDK_EDITING_PYFUNCTION_H__
#define __ORPYFBSDK_EDITING_PYFUNCTION_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: pyediting_pyfunction.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <pyfbsdk/pyfbsdk.h>

// =======================================================================================
// GLOBAL ENUMS:
// =======================================================================================
void ORFunctionInit();

// this function convert imported polySpline into 3d Path model
boost::python::list ConvertPolySpline( FBModel_Wrapper *pModel, bool afterDeform );


#endif /* __ORPYFBSDK_EDITING_PYFUNCTION_H__ */
