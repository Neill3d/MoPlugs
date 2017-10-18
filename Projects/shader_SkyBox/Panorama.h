
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Panorama.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <string>

#ifndef ORSDK_DLL
	/** \def ORSDK_DLL
	*	Be sure that ORSDK_DLL is defined only once...
	*/
	#define ORSDK_DLL __declspec(dllexport)
#endif

class ORSDK_DLL CPanorama
{
public:

	//! a constructor
	CPanorama();
	//! a destructor
	~CPanorama();

	bool	Load(const char *folder, const double lat, const double lng, const int zoom);

	void	Free();

private:

	void	AdaptTextureToZoom();

	bool	LoadPano(const char *folder, const double lat, const double lng, const char *id);
	bool	ComposePanorama(const char *folder);
	bool	ComposeFromTile(const int x, const int y, const char *filename);
	bool	SaveTile(const char *name, const int length, const unsigned char *data);

	int				mZoom;
	std::string		mId;

	int				mPanoWidth;
	int				mPanoHeight;

	unsigned char	*mPanoCanvas;	// full resulted panorama image

	float			mWc;
	float			mHc;
};