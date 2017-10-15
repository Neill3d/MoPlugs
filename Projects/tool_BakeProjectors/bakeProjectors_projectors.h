
#pragma once

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "shared_projectors.h"

//////////////////////////////////////////////////////////////////////////////
//
struct FBProjectorDATA
{
	FBCamera					*pProjector; 
	FBTexture					*pTexture;
	GLuint						textureId;

	bool						ProjectorAspect;

	GLuint						maskId;
	
	FBMatrix					ClipMatrix;
	FBMatrix					ProjMatrix;
	FBMatrix					ViewMatrix;

	static void Set(	FBCamera *pCamera, 
						FBTexture *pTexture, 
						
						const bool aspect,

						FBProjectorDATA	&data)
	{
		data.ProjectorAspect = aspect;
		data.pProjector = pCamera;
		data.pTexture = pTexture;
	}
};

typedef FBProjectorDATA		CProjectorsModels[MAX_PROJECTORS_COUNT];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class CProjectors

class CProjectors
{
public:

	//! a constructor
	CProjectors();
	//! a destructor
	virtual ~CProjectors();

	static bool ShaderHasProjectors(FBShader *pShader);

	bool		Prep(FBShader *pShader);

	bool		Bind(const int shaderFragmentId, const int projectorsUniformLoc);
	void		UnBind();

public:

	const int GetNumberOfProjectors() const;

	const ProjectorSet &GetProjectorSet() const;

	const GLuint GetMaskId(const int mask) const;

protected:

	// convert properties values to the CProjectorsData (and use after in specified drawing model)
	bool	DoPropertiesToData(FBShader *pShader);
	
	int							mNumberOfProjectors;
	ProjectorSet				mProjectorsData;			// this GLSL shader data
	CProjectorsModels			mProjectorsModels;			// this is ui converted data

	FBVideo						*mMasks[2];
	GLuint						mMasksId[2];

	CGPUProjectorsModelNv		mBufferProjectors;

	int PrepareProjector(FBProjectorDATA &mobuData, ProjectorDATA &data);

};