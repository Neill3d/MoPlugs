
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shaderCallbacks_blendshapeCompute.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "shaderCallbacks_blendshapeCompute.h"

#include "IO\FileUtils.h"
#include "graphics\CheckGLError_MOBU.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

CComputeProgram::CComputeProgram(const GLuint shaderid, const GLuint programid)
	: mShader(shaderid)
	, mProgram(programid)
{
	mStatus = false;
}

CComputeProgram::~CComputeProgram()
{
	Clear();
}

void CComputeProgram::Clear()
{
	if (mShader > 0)
	{
		if (mProgram > 0)
			glDetachShader(mProgram, mShader);
		glDeleteShader(mShader);
		mShader = 0;
	}
	if (mProgram > 0)
	{
		glDeleteProgram(mProgram);
		mProgram = 0;
	}
}

void CComputeProgram::CreateGLObjects()
{
	Clear();
	// TODO:
}

void CComputeProgram::ReCreateShaderObject()
{
	if (mProgram > 0 && mShader > 0)
	{
		glDetachShader(mProgram, mShader);
		glDeleteShader(mShader);

		mShader = glCreateShader(GL_COMPUTE_SHADER);
		glAttachShader(mProgram, mShader);
	}
}

bool CComputeProgram::PrepProgram(const char *filename)
{
	if (0 == mProgram || 0 == mShader)
	{
		Clear();

		mProgram = glCreateProgram();
		mShader = glCreateShader(GL_COMPUTE_SHADER);
		glAttachShader(mProgram, mShader);

		//
		FBString effectPath, effectFullName;
		if (true == FindEffectLocation( filename, effectPath, effectFullName ) )
		{
			if (false == loadComputeShader(effectFullName, mShader, mProgram) )
			{
				Clear();
				return false;
			}
			else
			{
				FBTrace ( "failed to compile a compute shader" );
			}
		}
		else
		{
			FBTrace( "failed to find effect location" );
		}
	}

	return true;
}

void CComputeProgram::Bind()
{
	if (mProgram > 0)
		glUseProgram(mProgram);
}

void CComputeProgram::UnBind()
{
	glUseProgram(0);
}

void CComputeProgram::DispatchPipeline(const int groups_x, const int groups_y, const int groups_z)
{
	glDispatchCompute(groups_x, groups_y, groups_z);
}
