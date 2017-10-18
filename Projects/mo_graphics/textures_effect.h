
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: textures_effect.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <GL\glew.h>

#include "Types.h"

//-------------- Effect system
#include "FxParser.h"

// DONE: finish implementation of this class
class ORView3DEffect : public Singleton<ORView3DEffect>
{
public:
	//! a constructor
	ORView3DEffect();

	//! a destructor
	~ORView3DEffect();

private:
	//
	// stuff effect, tech and pass interfaces
	//
	nvFX::IContainer*   fx_Effect;
        
	nvFX::ITechnique*   fx_Tech;
	nvFX::ITechnique	*fx_TechBackground;
	nvFX::IPass*        fx_pass;
	
	nvFX::IUniform		*fx_proj;
	nvFX::IUniform		*fx_mvp;
	nvFX::IUniform		*fx_quadSize;
	nvFX::IUniform		*fx_mouse;
	nvFX::IUniform		*fx_Pan;
	nvFX::IUniform		*fx_Height;

	int					mFragmentProgram;
	GLuint				mTexturesLoc;

	bool loadEffect(const char *effectFileName);

	bool			PrepTexturesUniform();
	void			UpdateTextureHandles();

public:

	const GLuint	GetTexturesLocation() const
	{
		return mTexturesLoc;
	}

	void	SetQuadSize( const float size )
	{
		if (fx_quadSize) fx_quadSize->setValue1f( size );
	}

	void	SetHeight( const float height )
	{
		if (fx_Height) fx_Height->setValue1f(height);
	}
	void	SetPan( const float pan )
	{
		if (fx_Pan) fx_Pan->setValue1f(pan);
	}
	void	SetProjectionMatrix( const float *pm )
	{
		if (fx_proj) fx_proj->setMatrix4f( (float*) pm );
	}
	void	SetMVP( const float *pm )
	{
		if (fx_mvp) fx_mvp->setMatrix4f( (float*) pm );
	}
	void	SetMousePosition( const float x, const float y )
	{
		if (fx_mouse) fx_mouse->setValue3f( x, y, 0.0f );
	}
	void	BindBackground()
	{
		if (fx_TechBackground)
		{
			fx_pass = fx_TechBackground->getPass(0);
			if (fx_pass) fx_pass->execute();
		}
	}
	void	Bind()
	{
		if (fx_Tech)
		{
			fx_pass = fx_Tech->getPass(0);
			if (fx_pass) fx_pass->execute();
		}
	}
	void	UnBind()
	{
		if (fx_pass) fx_pass->unbindProgram();
	}
};