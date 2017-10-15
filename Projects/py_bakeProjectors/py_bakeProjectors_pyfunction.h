#ifndef __ORPYFBSDK_TEMPLATE_PYFUNCTION_H__
#define __ORPYFBSDK_TEMPLATE_PYFUNCTION_H__


/**	\file	orpyfbsdk_template_pyfunction.h
*	Python wrapper for global funtions.
*/

// =======================================================================================
// GLOBAL ENUMS:
// =======================================================================================
void ORFunctionInit();

// function ask graphics manager to grab six sides from current camera position and save it to a dds with a specified path
void GrabStreeView( const char *filename, double lat, double lng, int zoom );

//FBShader* ORCreateShader( ORShaderType pShaderType = kORShaderTemplate );

#endif /* __ORPYFBSDK_TEMPLATE_PYFUNCTION_H__ */
