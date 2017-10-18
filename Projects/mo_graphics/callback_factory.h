
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: callback_factory.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//-- 
#include <GL\glew.h>

//--- SDK include
#include <fbsdk/fbsdk.h>

// std
#include <vector>

// forward declaration
class CBaseShaderCallback;
enum EShaderPass;
enum ERenderGoal;

#define		SHADER_CALLBACK_GPUCACHE	0
#define		SHADER_GPU_PARTICLES		1
#define		SHADER_CALLBACK_PROJTEX		2
#define		SHADER_CALLBACK_EYE			3
#define		SHADER_CALLBACK_IBL			4
#define		SHADER_CALLBACK_UBERSHADER	5

/////////////////////////////////////////////////////////////////
//
class CShadersFactory
{

public:

	//! a constructor
	CShadersFactory();

	//! a destructor
	~CShadersFactory();

	//
	//CBaseShaderCallback		*FindTypeByShader( FBShader *pShader );
	CBaseShaderCallback		*FindTypeByShaderAndGoal( FBShader *pShader, const ERenderGoal goal );

	void					RegisterNewShaderCallback( CBaseShaderCallback *pShaderCallback );

	const int				GetCallbackCount() const;

	const std::vector<CBaseShaderCallback*> &GetShaderTypesVector() const;

protected:

	//void			*mFactoryImpl;

};


