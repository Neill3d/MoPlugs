
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ParticleSystem_Generation.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <Windows.h>
#include "ParticleSystem.h"
#include "graphics\checkglerror.h"
#include "IO\FileUtils.h"
#include "algorithm\nv_math.h"
#include "algorithm\math3d.h"

using namespace ParticlesSystem;

#define _USE_MATH_DEFINES
#include <math.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void ParticleSystem::GetRandomVolumePos(const bool local, vec4 &pos)
{
	vec3 lMax = mEvaluateData.gMax;
	vec3 lMin = mEvaluateData.gMin;
			
	// if inherit emitter velocity, put particle on some random position during the way
	if (mEvaluateData.gEmitterVelocity.w > 0.0)
	{
		double randomValue = dist(e2);
		vec3 delta = mEvaluateData.gEmitterVelocity;
		delta *= (float)randomValue;
		lMax -= delta;
		lMin -= delta;
	}
	
	vec3 rnd = vec3( (float)dist(e2), (float)dist(e2), (float)dist(e2)); 
			
	pos.x = (lMax.x - lMin.x) * rnd.x + lMin.x;
	pos.y = (lMax.y - lMin.y) * rnd.y + lMin.y;
	pos.z = (lMax.z - lMin.z) * rnd.z + lMin.z;

	if (local == false)
		pos = mEvaluateData.gTM * pos;
}

/*
 r >= 0
 0 <= theta <= 180 deg (Pi)
 0 <= phi <= 360 deg (2Pi)
*/

void ConvertUnitVectorToSpherical(const vec4 &v, float &r, float &theta, float &phi)
{
	r = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	theta = acos( v.z / r );
	phi = atan( v.y / v.x );
}

void ConvertSphericalToUnitVector(const float r, const float theta, const float phi, vec4 &v)
{
	v.x = r * sin(theta) * cos(phi);
	v.y = r * sin(theta) * sin(phi);
	v.z = r * cos(theta);
}

void ParticleSystem::GetRandomVolumeDir(vec4 &vel)                                                   
{
	vec4 dirRnd = mEvaluateData.gDirRandom;

	float r, theta, phi;

	ConvertUnitVectorToSpherical(mEvaluateData.gDirection, r, theta, phi);

	const float PiPi = 2.0f * (float)M_PI;

	theta += dirRnd.x * M_PI;
	phi += dirRnd.y * PiPi;

	if (theta > M_PI) theta -= M_PI;
	else if (theta < 0.0f) theta += M_PI;

	if (phi > PiPi) phi -= PiPi;
	else if (phi < 0.0f) phi += PiPi;

	ConvertSphericalToUnitVector(r, theta, phi, vel);
}  

void ParticleSystem::GetRandomVolumeColor(const vec4 &pos, vec4 &color)
{
	vec3 lMax = mEvaluateData.gMax;
	vec3 lMin = mEvaluateData.gMin;
	
	color.x = (pos.x - lMin.x) / (lMax.x - lMin.x);
	color.y = (pos.y - lMin.y) / (lMax.y - lMin.y);
	color.z = (pos.z - lMin.z) / (lMax.z - lMin.z);
	color.w = 1.0f;
}

void ParticleSystem::GetRandomVerticesPos(const bool local, vec4 &pos, int &vertIndex)
{
	if (mSurfaceData.size() == 0)
		return;

	const int triCount = mSurfaceData.size();

	float rnd = (float) triCount * dist(e2);
	int triIndex = (int) rnd;

	_ASSERT (triIndex >= 0 || triIndex < triCount);

	if (triIndex >= triCount)
		triIndex = 0;

	rnd = dist(e2);

	if (rnd < 0.33f)
	{
		pos = mSurfaceData[triIndex].p0;
		vertIndex = triIndex * 3;
	}
	else if (rnd < 0.66f)
	{
		pos = mSurfaceData[triIndex].p1;
		vertIndex = triIndex * 3 + 1;
	}
	else
	{
		pos = mSurfaceData[triIndex].p2;
		vertIndex = triIndex * 3 + 2;
	}
	
	if (local == false)
		pos = mEvaluateData.gTM * pos;
}

void ParticleSystem::GetRandomVerticesDir(const int vertIndex, vec4 &vel) 
{

	int triIndex = vertIndex / 3;

	// TODO: randomize normal vector !!

	// USE_NORMALS_AS_DIR
	if (mEvaluateData.gDirection.w > 0.0f)
	{
		vel = mSurfaceData[triIndex].n;
	}
	else
	{
		vel = mEvaluateData.gDirection;
		vel.w = 1.0f;
	}
}   

void ParticleSystem::GetRandomVerticesColor(const int vertIndex, vec4 &color)
{
	
	if (mSurfaceTextureId == 0)
	{
		color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		// read from texture image
		color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void ParticleSystem::GetRandomSurfacePos(const bool local, const double extrudeDist, vec4 &pos, int &vertIndex, float &r1, float &r2, float &r3)
{
	if (mSurfaceData.size() == 0)
		return;

	const int triCount = mSurfaceData.size();

	float rnd = (float) triCount * dist(e2);
	int triIndex = (int) rnd;

	// barycentric coords
	float rnd1 = dist(e2);
	float rnd2 = dist(e2);

	r1 = 1.0f - sqrt(rnd1);
	r2 = sqrt(rnd1) * (1.0f - rnd2);
	r3 = sqrt(rnd1) * rnd2; 

	vec3 p0 = mSurfaceData[triIndex].p0;
	vec3 p1 = mSurfaceData[triIndex].p1;
	vec3 p2 = mSurfaceData[triIndex].p2;

	vec3 cp;
    //vec3 e0(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z);
    //vec3 e1(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z);

	cp = normalize(mSurfaceData[triIndex].n);

    //cross(cp,e0,e1);
	//normalize(cp);

	p0 *= r1;
	p1 *= r2;
	p2 *= r3;

	vec3 P = p0 + p1 + p2;
	vertIndex = triIndex * 3;

	pos = P;
	
	if (local == false)
	{
		pos = mEvaluateData.gTM * pos;
		cp = mEvaluateData.gNormalTM * cp;	
	}
	
	cp *= extrudeDist * dist(e2);
	pos += cp;
}

void ParticleSystem::GetRandomSurfaceDir(const int vertIndex, vec4 &vel) 
{

	int triIndex = vertIndex / 3;

	// TODO: randomize normal vector !!

	// USE_NORMALS_AS_DIR
	if (mEvaluateData.gDirection.w > 0.0f)
	{
		vel = mSurfaceData[triIndex].n;
	}
	else
	{
		vel = mEvaluateData.gDirection;
		vel.w = 1.0f;
	}
}   

struct SurfaceColor
{
	BYTE		red;
	BYTE		green;
	BYTE		blue;
	BYTE		alpha;
};

void ParticleSystem::GetRandomSurfaceColor(const int vertIndex, float r1, float r2, float r3, vec4 &color)
{
	if ( 0 == mSurfaceData.size() )
		return;

	int triIndex = vertIndex / 3;

	if (triIndex < 0)
		triIndex = 0;
	if (triIndex >= mSurfaceData.size() )
		triIndex = mSurfaceData.size()-1;

	if (mSurfaceTextureId == 0 )
	{
		color = mSurfaceData[triIndex].n;
	}
	else if (mSurfaceTextureData.size() > 0)
	{
		// read from texture image

		vec2 uv0 = mSurfaceData[triIndex].uv0;
		vec2 uv1 = mSurfaceData[triIndex].uv1;
		vec2 uv2 = mSurfaceData[triIndex].uv2;

		uv0 *= r1;
		uv1 *= r2;
		uv2 *= r3;

		vec2 uv = uv0 + uv1 + uv2;

		int x = (int) (uv.x * mSurfaceTextureInfo.width);
		int y = (int) (uv.y * mSurfaceTextureInfo.height);

		x -= 1;
		y -= 1;

		if (x < 0) x = 0;
		if (y < 0) y = 0;

		int pixelSize = mSurfaceTextureInfo.GetPixelMemorySize();

		SurfaceColor *pColor = (SurfaceColor*) &mSurfaceTextureData[ (y * mSurfaceTextureInfo.width + x) * pixelSize ];

		if (pixelSize > 3)
			color.w = 1.0f * pColor->alpha / 256.0f;
		else
			color.w = 1.0f;
		color.x = 1.0f * pColor->red / 256.0f;
		color.y = 1.0f * pColor->green / 256.0f;
		color.z = 1.0f * pColor->blue / 256.0f;
	}
}