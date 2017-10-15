#ifndef _ORPYFBSDK_COMMON_H_
#define _ORPYFBSDK_COMMON_H_

/**	\file	orpyfbsdk_common.h
*	common header file
*/

#pragma warning(disable:4996) // 'function': was declared deprecated.

//--- SDK include
#include <pyfbsdk/pyfbsdk.h>

#ifdef KARCH_ENV_WIN
#include <windows.h>
#endif

#include "py_bakeProjectors_FBView.h"


#endif //_ORPYFBSDK_COMMON_H_