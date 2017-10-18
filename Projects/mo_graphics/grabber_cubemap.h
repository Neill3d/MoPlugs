
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: grabber_cubemap.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#include "Types.h"

struct	ViewportGrabberOptions
{
	// cubemap options
	FBString		filename;
	int				imageSize;
	bool			separateImages;

	//! a constructor
	ViewportGrabberOptions()
	{
		filename = "";
		imageSize = 512;
		separateImages = false;
	}
};


////////////////////////////////////////////////////////////////////////////////////////
//
#if defined( BUILD_DLL )
    #define IMPORT_EXPORT __declspec(dllexport)
#else
    #define IMPORT_EXPORT __declspec(dllimport)
#endif

class IMPORT_EXPORT CViewportGrabber : public Singleton<CViewportGrabber>
{
public:
	//! a constructor
	CViewportGrabber();

	void GrabViewportCubeMap( const char *cubeMapFileName, const int cubeMapSize, const bool saveSeparateImages );

	const ViewportGrabberOptions		&GetOptions()
	{
		return mOptions;
	}

	// run in render cycle in place from which we can render for cubemap in safity
	void	Process(FBCamera *&_pointerForRenderCamera);		

	//
	bool	MakePanorama( const char *cubeMapFileName, const char *panoramaFileName, const int panoramaWidth, const int panoramaHeight );

protected:

	bool						mGrabCubeMap;

	ViewportGrabberOptions		mOptions;


	void	RenderCubeMap(FBCamera *&_pointerForRenderCamera);
};
