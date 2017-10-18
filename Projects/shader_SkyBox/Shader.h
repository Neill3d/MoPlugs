
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Shader.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

//-------------- Effect system
#include "FxParser.h"

#include "algorithm\nv_math.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shader class

class Shader
{
public:

	//! a constructor
	Shader();
	//! a destructor
	~Shader();

	bool Initialize();

	bool loadEffect(const char *effectFileName );
	bool setupVBO();
	void clearResources();
	
	bool IsLoaded() {
		return fx_passBox != nullptr; // && cubeId > 0;
	}

	void	Set( float *MVP, float *modelview, const float *eyePos, const float flipH, const float flipV, const double *uvOffset, const double *uvScale, const double *worldOffset, const double *worldScale, const double transFactor )
	{
		if (fx_MVP) fx_MVP->setMatrix4f(MVP);
		if (fx_ModelView) fx_ModelView->setMatrix4f(modelview);
		if (fx_EyePos) fx_EyePos->setValue3f(eyePos[0], eyePos[1], eyePos[2]);

		if (fx_FlipH) fx_FlipH->setValue1f( flipH );
		if (fx_FlipV) fx_FlipV->setValue1f( flipV );

		if (fx_UVOffset) fx_UVOffset->setValue3f( uvOffset[0], uvOffset[1], uvOffset[2] );
		if (fx_UVScale) fx_UVScale->setValue3f( uvScale[0] * 0.01, uvScale[1] * 0.01, uvScale[2] * 0.01 );

		if (fx_WorldOffset) fx_WorldOffset->setValue3f( worldOffset[0], worldOffset[1], worldOffset[2] );
		if (fx_WorldScale) fx_WorldScale->setValue3f( worldScale[0] * 0.01, worldScale[1] * 0.01, worldScale[2] * 0.01 );

		if (fx_TransparencyFactor) fx_TransparencyFactor->setValue1f( transFactor * 0.01 );
	}

	void BindBox()
	{
		if (fx_passBox)
			fx_passBox->execute();
	}

	void UnBindBox()
	{
		if (fx_passBox)
			fx_passBox->unbindProgram();
	}

	//
	void BindPano()
	{
		if (fx_passPano)
			fx_passPano->execute();
	}

	void UnBindPano()
	{
		if (fx_passPano)
			fx_passPano->unbindProgram();
	}

public:

	GLuint				vbo;
	GLuint				vao;

	//
	// stuff effect, tech and pass interfaces
	//
	nvFX::IContainer*   fx_Effect;
    
	nvFX::ITechnique*   fx_TechBox;
	nvFX::IPass*        fx_passBox;

	nvFX::ITechnique*   fx_TechPano;
	nvFX::IPass*        fx_passPano;
	
	nvFX::ITechnique*   fx_TechPlane;
	nvFX::IPass*        fx_passPlane;

	nvFX::IUniform		*fx_MVP;
	nvFX::IUniform		*fx_ModelView;
	nvFX::IUniform		*fx_EyePos;
	nvFX::IUniform		*fx_CubeTexture;
	nvFX::IUniform		*fx_PanoTexture;

	nvFX::IUniform		*fx_FlipH;
	nvFX::IUniform		*fx_FlipV;

	nvFX::IUniform		*fx_UVOffset;
	nvFX::IUniform		*fx_UVScale;

	nvFX::IUniform		*fx_WorldOffset;
	nvFX::IUniform		*fx_WorldScale;

	nvFX::IUniform		*fx_TransparencyFactor;
};