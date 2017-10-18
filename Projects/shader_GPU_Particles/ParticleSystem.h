
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ParticleSystem.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "GL\glew.h"
#include "algorithm\math3d.h"

#include "ParticleSystem_types.h"
#include "Shader_ParticleSystem.h"
#include "graphics\UniformBuffer.h"

#include <vector>
#include <random>

//TODO: evaluate exchange, collision and force should have the same data struct as a shader, no need to have two struct for the same

namespace ParticlesSystem
{


////////////////////////////////////////////////////////////////////////////////////////
// forces and collisions, geometry instances (textures)

class ParticleSystemConnections
{
public:

	//! a constructor
	ParticleSystemConnections();
	//! a destructor
	virtual ~ParticleSystemConnections();

	void	SetCollisionsCount(const int count);
	void	SetCollisionData(const int index, const TCollision &data);
	void	UploadCollisionsToGPU();

	void	SetForcesCount(const int count);
	void	SetForceData(const int index, const TForce &data);
	void	UploadForcesToGPU();

	void	SetInstanceData( unsigned int vertexCount, GLuint posId, GLuint norId, GLuint uvId, GLuint indexId, unsigned int patchCount );
	void	SetInstancePatchData( const int index, unsigned int offset, unsigned int size );

	const int GetNumberOfCollisions() {
		return (int) mCollisionData.size();
	}
	const int GetNumberOfForces() {
		return (int) mForcesData.size();
	}

	void BindCollisions(const GLuint slot)
	{
		mBufferCollisions.Bind(slot);
	}
	void BindForces(const GLuint slot)
	{
		mBufferForces.Bind(slot);
	}

	void SetTextureTerrain(const GLuint _textureId)
	{
		mTextureTerrain = _textureId;
	}

	const GLuint GetTextureTerrain()
	{
		return mTextureTerrain;
	}

protected:

	TInstanceVertex				mInstanceVertex;
	std::vector<TMeshPatch>		mInstanceMesh;

	std::vector<TCollision>		mCollisionData;
	std::vector<TForce>			mForcesData;

protected:

	GLuint						mTextureTerrain;

	CGPUBufferSSBO				mBufferCollisions;
	CGPUBufferSSBO				mBufferForces;

};

////////////////////////////////////////////////////////////////////////////////////////
//
class ParticleSystem
{
public:
	//! a constructor
    ParticleSystem(unsigned int maxparticles=1048576);
    //! a destructor
    ~ParticleSystem();
    
	void	ClearParticleSystem();
    bool	InitParticleSystem(const vec3 &Pos);
    
	bool	ReloadShaders();

	void ChangeDisplayContext();

	void	NeedReset();
	bool	IsNeedReset()
	{
		return mNeedReset;
	}
	// generate launchers and startup particles
	void	GenerateParticle(const int emitType, const bool local, const double extrudeDist, vec4 &pos, vec4 &vel, vec4 &color);
	bool	ResetParticles(unsigned int maxparticles, const int randomSeed, const int rate, const int preCount, const double extrudeDist);

	vec4	GenerateParticleColor(const vec4 &color, const float variation);
	float	GenerateParticleSize(const float size, const float variation);

    //void Render(unsigned int DeltaTimeMillis, const mat4 VP, const vec3 CameraPos);
    
	void SetMaxParticles(unsigned int count)
	{
		mMaxParticles = count;
	}
	void SetLifeTime(unsigned int value)
	{
		mLifeTime = value;
	}

	void SetLastEmitterPos( const vec3 &lastpos )
	{
		mLastEmitterPos = lastpos;
	}
	void SetLastEmitterPos( const double *lastpos )
	{
		mLastEmitterPos = vec3( (float)lastpos[0], (float)lastpos[1], (float)lastpos[2] );
	}
	void GetLastEmitterPos( double *lastpos )
	{
		lastpos[0] = (double) mLastEmitterPos.x;
		lastpos[1] = (double) mLastEmitterPos.y;
		lastpos[2] = (double) mLastEmitterPos.z;
	}

public:

	struct PerModelUserData
	{
		double	lastFrameTime;
		bool	isResetDone;
		bool	isFirst;
	};

	PerModelUserData		mPerModelUserData;

public:

	// this is for surface particles emitting
	bool EmitterSurfaceUpdateOnCPU(const int vertexCount, float *positionsArray, float *normalArray, float *uvArray, const int indexCount, const int *indexArray, const GLuint textureId );
	bool EmitterSurfaceUpdateOnGPU(const GLuint positionsId, const GLuint normalsId, const GLuint uvId, const int indexCount, const GLuint indexId, const GLuint textureId);

	void	UploadSurfaceDataToGPU();

	// run evaluate shader

	void SetParticleSize(const double size, const double size_variation);
	void SetParticleColor(const bool inheritSurfaceColor, const vec4 color, const double color_variation);

	void PrepareParticles(unsigned int maxparticles, const int randomSeed, unsigned int particleCount, bool useRate, unsigned int rate, const double extrudeDist);

	ParticlesSystem::evaluateBlock		&GetSimulationData();
	bool UploadSimulationDataOnGPU();

	void EmitParticles(const double timeStep, const ETechEmitType	type);
	const unsigned int SimulateParticles(const double timeStep, double &DeltaTime, const double limit, const int SubSteps, const bool selfCollisions);
	// todo: VP, MV, camera pos should be updated into the uniform buffer
	
	// run render shader

	

	ParticlesSystem::renderBlock		&GetRenderData();
	bool UploadRenderDataOnGPU();

	// textures for size and color lookup ( 0 - to disable )
	void	SetRenderSizeAndColorCurves( GLuint sizeTextureId, GLuint colorTextureId );
    void RenderParticles(int type, const bool pointSmooth);

	void SetConnections(ParticleSystemConnections	*pConnections)
	{
		mConnections = pConnections;
	}

	

protected:
    
    bool						mIsFirst;				// running system first time

	std::random_device					rd;
	std::mt19937						e2;		// engine
	std::uniform_real_distribution<>	dist;	// distribution

	ParticlesSystem::evaluateBlock		mEvaluateData;			// common exchange parameters between UI and evaluate shader
	ParticlesSystem::renderBlock		mRenderData;

	bool						mNeedReset;
	unsigned int				mMaxParticles;			// initial amount of particles
	unsigned int				mParticleCount;
	bool						mUseRate;
	unsigned int				mParticleRate;
	unsigned int				mLifeTime;		

    double						mTime;					// lets use high definition timer

	float						mPointSize;
	float						mPointSizeVariation;
	bool						mInheritSurfaceColor;
	vec4						mPointColor;
	float						mPointColorVariation;

	vec3						mLastEmitterPos;

	// forces and collisions
	ParticleSystemConnections	*mConnections;

	// read for particle color inheritance
	struct TextureInfo
	{
		int red;
		int green;
		int blue;
		int alpha;

		int width;
		int height;

		int GetPixelMemorySize() {
			return (red + green + blue + alpha) / 8;
		}

		int GetImageSize() {
			return width * height * GetPixelMemorySize();
		}
	};
	TextureInfo					mSurfaceTextureInfo;
	std::vector<unsigned char>	mSurfaceTextureData;

	GLuint						mSurfaceTextureId;
	std::vector<TTriangle>		mSurfaceData;

	

	void			PrepNoiseTexture();
	void			FreeNoiseTexture();

	bool	ReadSurfaceTextureData();

	void GetRandomVolumePos(const bool local, vec4 &pos);
	void GetRandomVolumeDir(vec4 &pos);
	void GetRandomVolumeColor(const vec4 &pos, vec4 &color);

	void GetRandomVerticesPos(const bool local, vec4 &pos, int &vertIndex);
	void GetRandomVerticesDir(const int vertIndex, vec4 &vel);
	void GetRandomVerticesColor(const int vertIndex, vec4 &color);

	// r1, r2, r3 - return barycentric coords
	void GetRandomSurfacePos(const bool local, const double extrudeDist, vec4 &pos, int &vertIndex, float &r1, float &r2, float &r3);
	void GetRandomSurfaceDir(const int vertIndex, vec4 &vel);
	void GetRandomSurfaceColor(const int vertIndex, float r1, float r2, float r3, vec4 &color);

protected:

    unsigned int				mCurrVB;
    unsigned int				mCurrTFB;
    GLuint						mParticleBuffer[2];
    GLuint						mTransformFeedback[2];

	//	query transform feedback count
	GLuint						mInstanceCount;	
	GLuint						mQuery;

	// ! Use particles system shader (terrain technique)
	ParticlesSystem::Shader			*mShader;

	// emitter positions and normals
	// TODO: replace buffer texture with a NV pointer uniform buffer
	//BufferTexture				mPositionTexture;	// hold vertices positions in a buffer texture
	//BufferTexture				mNormalTexture;		// hold vertices normals in a buffer texture

	CGPUBufferNV				mBufferSurface;

	GLuint						mTexture;

	GLuint						mSizeTextureId;
	GLuint						mColorTextureId;

	GLuint						mNoiseTexture;	// 3d texture for the turbulence field


	void RenderPoints();
	void RenderBillboards();
	void RenderStretchedBillboards();
	void RenderInstances();

	void SwapBuffers();	// operation to switch update and render double-buffers

};

};