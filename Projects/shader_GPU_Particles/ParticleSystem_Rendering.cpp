
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ParticleSystem_Rendering.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "ParticleSystem.h"

using namespace ParticlesSystem;


// const mat4 &VP, const mat4 &MV, const vec3 &CameraPos, float size, bool useSizeCurve, vec4 color, bool useColorCurve, float AlphaFactor, const GLuint tex 

renderBlock	&ParticleSystem::GetRenderData()
{
	return mRenderData;
}

bool ParticleSystem::UploadRenderDataOnGPU()
{
	if (mShader->IsInitialized() == false)
	if (false == mShader->Initialize() )
			return false;

	mShader->UploadRenderDataBlock(mRenderData);

	return true;

	/*
	mRenderData.gCameraPos = vec4(CameraPos.x, CameraPos.y, CameraPos.z, 1.0f);
	mRenderData.gVP = VP;
	mRenderData.gMV = MV;

	mRenderData.gColor = color;
	mRenderData.gUseColorCurve = (useColorCurve) ? 1 : 0;

	mRenderData.gBillboardSize = size;
	mRenderData.gUseSizeCurve = (useSizeCurve) ? 1 : 0;
	*/
}


void ParticleSystem::SetRenderSizeAndColorCurves( GLuint sizeTextureId, GLuint colorTextureId )
{
	mSizeTextureId = sizeTextureId;
	mColorTextureId = colorTextureId;
}

void ParticleSystem::RenderParticles(int type, const bool pointSmooth)
{
	//GLuint lSizeTexId = mSizeCurve.GetTextureId();
	//GLuint lColorTexId = mColorCurve.GetTextureId();

	glDisable(GL_RASTERIZER_DISCARD);
	
	if (mSizeTextureId > 0 && mRenderData.gUseSizeCurve > 0)
	{
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_1D, mSizeTextureId);
		glActiveTexture(GL_TEXTURE0);
	}
			
	if (mColorTextureId > 0 && mRenderData.gUseColorCurve > 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_1D, mColorTextureId);
		glActiveTexture(GL_TEXTURE0);
	}

	if (pointSmooth)
	{
		glEnable(GL_POINT_SMOOTH);
	}

	//
	//
	RenderPoints();

	if (pointSmooth)
	{
		glDisable(GL_POINT_SMOOTH);
	}

	/*
	switch(type)
	{
	case 0: // points
		RenderPoints();
		break;
	case 1: // billboard
		RenderBillboards();
		break;
	case 2: // stretched billboard
		RenderStretchedBillboards();
		break;
	case 3:
		RenderInstances();
		break;
	}
	*/

	if (mSizeTextureId > 0 && mRenderData.gUseSizeCurve > 0)
	{
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_1D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
			
	if (mColorTextureId > 0 && mRenderData.gUseColorCurve > 0)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_1D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
}

void ParticleSystem::RenderPoints()
{
	//glPointSize(mPointSize);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	mShader->BindRenderPoints();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[mCurrTFB]);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);  // position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)32);  // color

	glDrawTransformFeedback(GL_POINTS, mTransformFeedback[mCurrTFB]);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	mShader->UnBindRenderPoints();

	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}
/*
void ParticleSystem::RenderBillboards()
{
	checkGlError("begin rendering billboard");

	mBillboardProgram.bind();
 
	mBillboardProgram.SetMV(MV);
	mBillboardProgram.SetVP(VP);
	//glDepthFunc(GL_LESS);
	//glDepthMask(GL_FALSE);

	if (mTexture) {
		glActiveTexture(GL_TEXTURE0);
		mBillboardProgram.SetUseColorMap(TRUE);
		mTexture->OGLInit();
		glBindTexture(GL_TEXTURE_2D, mTexture->TextureOGLId);

		//glEnable(GL_BLEND);
		//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}
	else
	{
		mBillboardProgram.SetUseColorMap(FALSE);
	}
			
	if (lSizeTexId && mUseSizeCurve)
	{
		mBillboardProgram.SetSizeCurve(5);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_1D, lSizeTexId);
		glActiveTexture(GL_TEXTURE0);
	}
			
	if (lColorTexId && mUseColorCurve)
	{
		mBillboardProgram.SetColorCurve(2);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_1D, lColorTexId);
		glActiveTexture(GL_TEXTURE0);
	}
			
	glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[mCurrTFB]);    
	glEnableVertexAttribArray(0);
			
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);       // pos
			
	glDrawTransformFeedback(GL_POINTS, mTransformFeedback[mCurrTFB]);

	glDisableVertexAttribArray(0);
			

	//glDepthMask(GL_TRUE);
			
	if (lSizeTexId && mUseSizeCurve)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
			
	if (lColorTexId && mUseColorCurve)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_1D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
			
	if (mTexture) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glDisable(GL_BLEND);
	}
			
	mBillboardProgram.unbind();

	checkGlError("end rendering billboard");
}

void ParticleSystem::RenderStretchedBillboards()
{
	checkGlError("begin rendering stretched billboard");

	mStretchedBillboardProgram.bind();
 
	mStretchedBillboardProgram.SetMV(MV);
	mStretchedBillboardProgram.SetVP(VP);
	//glDepthFunc(GL_LESS);
	//glDepthMask(GL_FALSE);

	if (mTexture) {
		glActiveTexture(GL_TEXTURE0);
		mStretchedBillboardProgram.SetUseColorMap(TRUE);
		mTexture->OGLInit();
		glBindTexture(GL_TEXTURE_2D, mTexture->TextureOGLId);

		//glEnable(GL_BLEND);
		//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}
	else
	{
		mStretchedBillboardProgram.SetUseColorMap(FALSE);
	}
			
	if (lSizeTexId && mUseSizeCurve)
	{
		mStretchedBillboardProgram.SetSizeCurve(5);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_1D, lSizeTexId);
		glActiveTexture(GL_TEXTURE0);
	}
			
	if (lColorTexId && mUseColorCurve)
	{
		mStretchedBillboardProgram.SetColorCurve(2);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_1D, lColorTexId);
		glActiveTexture(GL_TEXTURE0);
	}
			
	glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[mCurrTFB]);    
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);		// old pos
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);       // pos
			
	glDrawTransformFeedback(GL_POINTS, mTransformFeedback[mCurrTFB]);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	//glDepthMask(GL_TRUE);
			
	if (lSizeTexId && mUseSizeCurve)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
			
	if (lColorTexId && mUseColorCurve)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_1D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
			
	if (mTexture) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glDisable(GL_BLEND);
	}
			
	mStretchedBillboardProgram.unbind();

	checkGlError("end rendering stretched billboard");
}

void ParticleSystem::RenderInstances()
{
	if (mInstanceMesh.size() > 0)
	{
		
		//pModelVertexData->EnableOGLVertexData();
	
		GLuint posId = mInstanceVertex.positionId;
		GLuint norId = mInstanceVertex.normalId;
		
		glBindBuffer(GL_ARRAY_BUFFER, posId);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (const GLvoid*) 0); // position
		glBindBuffer(GL_ARRAY_BUFFER, norId);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (const GLvoid*) 0); // normals

		mInstancingProgram.bind();
		mInstancingProgram.SetVP(VP);
		mInstancingProgram.SetMV(MV);
		mInstancingProgram.SetCameraPosition(vec3(100.0, 50.0, 0.0));
			
		mInstancingProgram.SetUseSizeCurve(mUseSizeCurve);
		if (lSizeTexId && mUseSizeCurve)
		{
			mInstancingProgram.SetSizeCurve(5);

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_1D, lSizeTexId);
			glActiveTexture(GL_TEXTURE0);
		}
			
		mInstancingProgram.SetUseColorCurve(mUseColorCurve);
		if (lColorTexId && mUseColorCurve)
		{
			mInstancingProgram.SetColorCurve(2);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_1D, lColorTexId);
			glActiveTexture(GL_TEXTURE0);
		}

		glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffer[mCurrTFB]);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);  // position
		glVertexAttribDivisor(4, 1);

		int baseVertex = 0;
		int indicesCount = 0;

		GLuint elementId = mInstanceVertex.indexId;

		if (elementId)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementId);

			for (auto iter=begin(mInstanceMesh); iter!=end(mInstanceMesh); ++iter)
			{
				baseVertex = iter->offset;
				indicesCount = iter->size;

				glDrawElementsInstanced(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, (void*) (sizeof(unsigned int) * baseVertex), mInstanceCount);
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(4);
		mInstancingProgram.unbind();

		if (lSizeTexId && mUseSizeCurve)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_1D, 0);
			glActiveTexture(GL_TEXTURE0);
		}
			
		if (lColorTexId && mUseColorCurve)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_1D, 0);
			glActiveTexture(GL_TEXTURE0);
		}

		//pModelVertexData->DisableOGLVertexData();
	}
}
*/
void ParticleSystem::SwapBuffers()
{
	//
	// swap buffers
	//
	mCurrVB = mCurrTFB;
    mCurrTFB = (mCurrTFB + 1) & 0x1;
}
