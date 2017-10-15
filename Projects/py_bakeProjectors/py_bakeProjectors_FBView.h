#ifndef __ORPYFBSDK_BAKEPROJECTORS_CLASS_H__
#define __ORPYFBSDK_BAKEPROJECTORS_CLASS_H__

/**	\file	orpyfbsdk_shader_skybox.h
*	Python wrapper for texmat shader.
*/

//--- SDK include
#include <pyfbsdk/pyfbsdk.h> 
#include "..\tool_bakeProjectors\bakeProjectors_view.h"

// =======================================================================================
// ORCgShader
// =======================================================================================
void BakeProjectorsFBView_Init();

inline ViewBakeProjectors* CreateViewBakeProjectors(const char* pName)
{
    ViewBakeProjectors* lView = new ViewBakeProjectors();
    lView->FBCreate();
    return lView;
}

class ViewBakeProjectors_Wrapper : public FBView_Wrapper 
{
protected:
	ViewBakeProjectors			*mView;

public:
    /** For the FBComponent* constructor.
    *   Used internally by the wrapper factory method to construct python wrapper from an existing C++ instance.
    */
	ViewBakeProjectors_Wrapper( FBComponent* pFBComponent ) 
        : FBView_Wrapper( pFBComponent ) 
    { 
        mView = (ViewBakeProjectors*)pFBComponent; 
    }

	/** Normal char* constructor.
    *   Create a new instance of C++ instance and wrap it to python. 
    */
	ViewBakeProjectors_Wrapper( char* pName = NULL ) 
        : FBView_Wrapper( CreateViewBakeProjectors( pName )) 
    { 
        mView = (ViewBakeProjectors*)mFBComponent; 
    }

	virtual ~ViewBakeProjectors_Wrapper( ) {}
	/*
	//! Setter and Getter methods for exposed properties.
	void SetType( FBSkyBoxType type ) { mShaderSkyBox->Type = type; }
	FBSkyBoxType GetType(  ) { return mShaderSkyBox->Type; }

	//! expose methods
	bool LoadCubeMap( const char  *filename )
	{
		//mShaderSkyBox->CubeMapPath = filename;
		FBTrace( "filename = %s\n", filename );
		return mShaderSkyBox->DoLoadCubeMap(filename, false);
	}
	*/
};

#endif /* __ORPYFBSDK_BAKEPROJECTORS_CLASS_H__ */
