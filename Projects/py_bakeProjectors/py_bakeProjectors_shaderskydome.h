#ifndef __ORPYFBSDK_SHADER_SKYDOME_H__
#define __ORPYFBSDK_SHADER_SKYDOME_H__

/**	\file	orpyfbsdk_shader_skydome.h
*	Python wrapper for texmat shader.
*/

#include "..\shader_skybox\SkyDome_shader.h"

// =======================================================================================
// ORCgShader
// =======================================================================================
void ORShaderSkyDome_Init();

inline ORShaderSkyDome* CreateORShaderSkyDome(const char* pName)
{
    ORShaderSkyDome* lShader = new ORShaderSkyDome(pName);
    lShader->FBCreate();
    return lShader;
}

class ORShaderSkyDome_Wrapper : public FBShader_Wrapper 
{
protected:
	ORShaderSkyDome			*mShaderSkyDome;

public:
    /** For the FBComponent* constructor.
    *   Used internally by the wrapper factory method to construct python wrapper from an existing C++ instance.
    */
	ORShaderSkyDome_Wrapper( FBComponent* pFBComponent ) 
        : FBShader_Wrapper( pFBComponent ) 
    { 
        mShaderSkyDome = (ORShaderSkyDome*)pFBComponent; 
    }

	/** Normal char* constructor.
    *   Create a new instance of C++ instance and wrap it to python. 
    */
	ORShaderSkyDome_Wrapper( char* pName = NULL ) 
        : FBShader_Wrapper( CreateORShaderSkyDome( pName )) 
    { 
        mShaderSkyDome = (ORShaderSkyDome*)mFBComponent; 
    }

	virtual ~ORShaderSkyDome_Wrapper( ) {}

};

#endif /* __ORPYFBSDK_SHADER_SKYDOME_H__ */
