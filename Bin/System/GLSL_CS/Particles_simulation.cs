//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Particles_simulation.cs
//
//	Author Sergey Solokhin (Neill3d)
//
// Main GPU Particles compute shader
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#version 430

layout (local_size_x = 1024, local_size_y = 1) in;

uniform	int		gNumParticles;

uniform float	DeltaTimeSecs;
uniform float	gTime;

uniform vec4	gDynamic;
uniform vec4	gTurbulence;
uniform vec4	gGravity;			// in w - use gravity force
uniform vec4	gFloor;				// in x - floor friction, in y - level, in w - use floor level

uniform	int		gNumForces;
uniform	int		gNumCollisions;

uniform int		gUseSizeAttenuation;
uniform int		gUseColorAttenuation;

uniform int		gUpdatePosition;

	#define PARTICLE_TYPE_LAUNCHER 0.0f                                                 
	#define PARTICLE_TYPE_SHELL 1.0f                                                    
	#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f  

#define		USE_FLOOR			gFloor.w
#define		FLOOR_FRICTION		gFloor.y
#define		FLOOR_LEVEL			gFloor.z

#define		MASS				gDynamic.x
#define		DAMPING				gDynamic.y
#define 	CONSTRAINT_MAGN		gDynamic.z

#define		USE_GRAVITY			gGravity.w
#define		GRAVITY				gGravity.xyz

#define 	USE_TURBULENCE		gTurbulence.w
#define		NOISE_FREQ			gTurbulence.x
#define		NOISE_SPEED			gTurbulence.y
#define		NOISE_AMP			gTurbulence.z

#define		USE_FORCES			gNumForces > 0
#define		USE_COLLISIONS		gNumCollisions > 0
#define		EMITTER_TYPE		gFlags.z

#define		EMITTER_TYPE_VERTICES	0.0
#define		EMITTER_TYPE_VOLUME		1.0

#define		FORCE_WIND				1.0
#define		FORCE_DRAG				2.0
#define		FORCE_MOTOR				3.0

#define		COLLISION_SPHERE		1.0
#define		COLLISION_TERRIAN		4.0

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TYPES AND DATA BUFFERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TParticle
{ 
	vec4				Pos;				// in w hold lifetime from 0.0 to 1.0 (normalized)
	vec4				Vel;				// in w hold total lifetime
	vec4				Color;				// inherit color from the emitter surface, custom color simulation
	vec4				Rot;				// in w - float				AgeMillis;			// current Age
	vec4 				RotVel;				// in w - float				Index;				// individual assigned index from 0.0 to 1.0 (normalized)
};

struct TCollision
{
	vec4			position;			// use w as collision type
	vec4			velocity;
	
	vec4			terrainScale;		// .w - softness
	vec4			terrainSize;		// texture dimentions
	
	float 			radius;
	float			friction;
	
	uvec2			terrainAddress;
	
	mat4			tm;
};

struct TForce
{
	vec4			position;
	vec4			direction;		// use w as a force type
	float			magnitude;
	float			radius;
	float			noiseFreq;
	float 			noiseSpeed;
	vec4			turbulence;		// w - use turbulence or not, x-amplitude, y-frequency
	vec4			wind1;			// special wind pre-calculated force
	vec4			wind2;	
};


layout (std430, binding = 0) buffer ParticleBuffer
{
	TParticle particles[];
} particleBuffer;

layout (std430, binding = 1) readonly buffer ForcesBuffer
{
	TForce forces[];
} forceBuffer;

layout (std430, binding = 2) readonly buffer CollisionBuffer
{
	TCollision collisions[];
} collisionBuffer;

// terrain depth
layout(binding=0) uniform sampler2D 	TerrainSampler;
// particle size attenuation
layout(binding=5) uniform sampler1D 	SizeSampler;

const vec2 randN1 = vec2(0.14, -0.07);
const vec2 randN2 = vec2(0.77, 1.01);
const vec2 randN3 = vec2(-0.38, 0.15);

const float PiPi = 6.2831853;
const float PI = 3.14159265;
const float PI_2 = 1.57079632;
const float PI_4 = 0.785398163;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

highp float rand(vec2 co)
{
	highp float a = 12.9898;
	highp float b = 78.233;
	highp float c = 43758.5453;
	highp float dt= dot(co.xy ,vec2(a,b));
	highp float sn= mod(dt,3.14);
	return fract(sin(sn) * c);
}

mat3 rotationMatrix(vec3 axisIn, float angle)
{
	vec3 axis = normalize(axisIn);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	
	return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
				oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
				oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

uint get_invocation()
{
   //uint work_group = gl_WorkGroupID.x * gl_NumWorkGroups.y * gl_NumWorkGroups.z + gl_WorkGroupID.y * gl_NumWorkGroups.z + gl_WorkGroupID.z;
   //return work_group * gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z + gl_LocalInvocationIndex;

   // uint work_group = gl_WorkGroupID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x + gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationIndex;
   uint work_group = gl_GlobalInvocationID.y * (gl_NumWorkGroups.x * gl_WorkGroupSize.x) + gl_GlobalInvocationID.x;
   return work_group;
}

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
	 return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
			 i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
		   + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
		   + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
								dot(p2,x2), dot(p3,x3) ) );
}

//////////////////////////////////////////////////////////////////////
//
// out float r, out float theta, out float phi

void ConvertUnitVectorToSpherical(const vec4 v, out vec3 sv)
{
	sv.x = sqrt(v.x*v.x + v.y*v.y + v.z*v.z); // r
	sv.y = atan( v.y,  v.x ); // theta
	sv.z = atan( sqrt(v.x*v.x+v.y*v.y), v.z ); // phi
}

// const float r, const float theta, const float phi
void ConvertSphericalToUnitVector(vec3 sv, out vec4 v)
{
	v.x = sv.x * cos(sv.y) * sin(sv.z);
	v.y = sv.x * sin(sv.y) * sin(sv.z);
	v.z = sv.x * cos(sv.z);
	v.w = 1.0;
}

void GetRandomDir(in vec4 inDir, in vec2 dirRnd, out vec4 dir)                                   
{
	//float r, theta, phi;
	vec3 sv;

	ConvertUnitVectorToSpherical(inDir, sv);

	sv.y += dirRnd.x * PI;
	sv.z += dirRnd.y * PiPi;
			
	ConvertSphericalToUnitVector(sv, dir);
} 

void GetRandomDir(in vec3 inDir, in vec2 dirRnd, out vec3 dir)                                        {
	//float r, theta, phi;
	vec3 sv;

	ConvertUnitVectorToSpherical(vec4(inDir, 1.0), sv);

	sv.y += dirRnd.x * PI;
	sv.z += dirRnd.y * PiPi;
	
	vec4 result;
	ConvertSphericalToUnitVector(sv, result);
	dir = result.xyz;
}  

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRAINT AND COLLIDE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SphereCollide(TCollision data, inout vec3 x, inout vec3 vel, inout vec3 force)
{
	//vec4 untransformed = data.tm * vec4(x, 1.0);
	
	//vec3 delta = untransformed.xyz; // - data.position.xyz;
	vec3 delta = x - data.position.xyz;
	float dist = length(delta);
	if (dist < data.radius) {
//      x = center + delta*(r / dist); 
		vec3 newvel = vel * (dist / data.radius) * data.friction; 
		vel = mix(newvel, vel, data.terrainScale.w);
		//vel *= (dist / data.radius);   
		//force += normalize(delta);
	  //vel -= (1.0 - data.terrainScale.w) * (delta / dist) * data.friction;
		vel += (1.0 - data.terrainScale.w) * data.velocity.xyz;
	}
}

// constrain particle to be outside volume of a sphere
void SphereConstraint(TCollision data, inout vec3 x)
{
	//vec4 untransformed = data.tm * vec4(x, 1.0);
	
	//vec3 delta = untransformed.xyz; // - data.position.xyz;
	vec3 delta = x - data.position.xyz;
	float dist = length(delta);
	if (dist < data.radius) {
		//vec4 transformed = inverse(data.tm) * vec4(delta*(data.radius / dist), 1.0);
		//x = mix(x, transformed.xyz, data.terrainScale.w);
		vec3 newx = data.position.xyz + normalize(delta)*data.radius;
		x = mix(newx, x, data.terrainScale.w);
	}
}

// constrain particle to heightfield stored in texture
void TerrainConstraint(TCollision data, inout vec3 pos)
{
	vec3 scale = data.terrainScale.xyz;
	vec3 offset = data.position.xyz;
	
	
	if (pos.x < offset.x || pos.x > offset.x+scale.x || pos.z < offset.z || pos.z > offset.z+scale.z
		|| pos.y < offset.y || pos.y > data.terrainSize.z )
	{
		return;
	}
	
	vec2 uv = (pos.xz - offset.xz) / scale.xz;

	float h = texture(TerrainSampler, uv).x;
	
	if (pos.y < h) {
		pos.y = h;
	}
}

// constrain particle to be above floor
void FloorConstraint(inout vec3 x, float level)
{
	if (x.y < level) {
		x.y = level;
	}
}

void FloorCollide(inout vec3 x, inout vec3 vel, const float rndF, float level, float friction, const float dt)
{
	if (x.y < level) {
//        x.y = level;
//        force.y += -vel.y*friction;
		vel.y += -vel.y*friction * rand(vec2(rndF, 0.123)) * dt;
		vel.x += rand(vec2(rndF, -0.123)) * friction * dt;
		vel.z += rand(vec2(rndF, -0.543)) * friction * dt;
	}
}

void TerrainCollide(TCollision data, vec3 pos, const float rndF, inout vec3 vel)
{
	vec3 offset = data.position.xyz;
	vec3 scale = data.terrainScale.xyz;
	
	// should be predefined in shader
	if (pos.x < offset.x || pos.x > offset.x+scale.x || pos.z < offset.z || pos.z > offset.z+scale.z
		|| pos.y < offset.y || pos.y > data.terrainSize.z )
	{
		return;
	}
	 
	vec2 texelSize = vec2( 1.0 / data.terrainSize.x, 1.0 / data.terrainSize.y );
	vec2 uv = (pos.xz - offset.xz) / scale.xz;
	
	float h0 = texture(TerrainSampler, uv).x;
	
	if (pos.y < h0) {
		// calculate normal (could precalc this)
		float h1 = texture(TerrainSampler, uv + texelSize*vec2(1.0, 0.0) ).r;
		float h2 = texture(TerrainSampler, uv + texelSize*vec2(0.0, 1.0) ).r;
		
		vec3 N = cross( vec3(scale.x*texelSize.x, h1-h0, 0.0), vec3(0.0, h2-h0, scale.z*texelSize.y) );
		//vec3 N = cross( vec3(scale.x*texelSize.x, (h1-h0)*scale.y, 0), vec3(0, (h2-h0)*scale.y, scale.z*texelSize.y) );
		N = normalize(N);
		
		//GetRandomDir(N, vec2(0.1*rand(vec2(rndF, 0.11)), 0.2*rand(vec2(rndF, -0.05))), N);
		
		vel = reflect(vel, N);
		vel *= data.friction;
	}
}

////////////////////////////////////////////////////////////////////////
// FORCES
////////////////////////////////////////////////////////////////////////


void ApplyWindForce2(TForce data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel, float dt)
{
	// combining four winds.
	
	float a = snoise( vec3(data.noiseSpeed * time) + data.noiseFreq * pos_next );
	vec3 w = a*data.direction.xyz + (1.0f-a)*data.turbulence.xyz + a*data.wind1.xyz + (1.0f-a)*data.wind2.xyz;
	vec3 lforce = data.magnitude * normalize(w);
	
	float r = data.radius;
	if (r > 0.0f)
	{
		vec3 lpos = data.position.xyz;
		float len = length(lpos - pos_next);
		len = clamp(len, 0.0, r);
		len = 1.0f - len / r;
		lforce *= len;
	}
	
	vel += lforce*dt*dt;
}

void ApplyDragForce(TForce data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel)
{
	// here direction is a drag velocity
	vec3 lforce = data.magnitude * data.direction.xyz;
	float r = data.radius;
	
	if (r > 0.0)
	{
		vec3 lpos = data.position.xyz;
		float len = length(lpos - pos_next);
		if (len <= r)
		{
			len = 1.0 - len / (r + 0.001);
			force += lforce * len;
			//force += lforce * (1.0 / (len*len + 0.001) );
		}
	}
	else
	{
		force +=  lforce;
	}
}

void ApplyDragForce2(TForce data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel)
{
	// here direction is a drag velocity
	vec3 lforce = data.magnitude * data.direction.xyz;
	lforce = data.magnitude * normalize(pos_next - data.position.xyz);
	lforce.x = 0.0;
	lforce.z = 0.0;
	float r = data.radius;
	
	if (r > 0.0)
	{
		vec3 lpos = data.position.xyz;
		float len = length(lpos - pos_next);
		if (len <= r)
		{
			len = 1.0 - len / (r + 0.001);
			force += lforce * len;
			//force += lforce * (1.0 / (len*len + 0.001) );
		}
	}
	else
	{
		force +=  lforce;
	}
}

void ApplyMotorForce(TForce data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel)
{
	vec3 lpos = data.position.xyz;
	
	mat3 mat = rotationMatrix( data.direction.xyz, data.direction.w * 3.14 / 180.0 );
	
	vec3 v = pos_next - lpos;
	
	// TODO: we can make a force to move to/from the center of motor
	
	vec3 tmpPos = pos_next - lpos;
	vec3 point = (dot(tmpPos, data.direction.xyz)/dot(data.direction.xyz, data.direction.xyz) * data.direction.xyz);
	vec3 pointVec = tmpPos - point;
	
	vec3 direction = lpos - pos_next;
	float distance = length(direction);
	direction /= distance;

	//vel += direction * max(0.01, (1.0 / (distance*distance))); // * data.magnitude;
	
	vec3 v2 = v * mat;
	//v += lpos;
	vec3 lforce = vec3(0.0); // 0.1 * data.magnitude * (v - pos_next);
	vec3 spinVec = cross(direction, pointVec);
	//lforce = spinVec * data.magnitude;
	lforce += (v2 - v + direction) * max(0.01, (1.0 / (distance*distance))) * data.magnitude * 10.0;
	//vec3 lforce = data.magnitude * data.direction.xyz;
	float r = data.radius;
	
	if (r > 0.0)
	{
		
		float len = length(lpos - pos_next);
		if (len <= r)
		{
			len = 1.0 - len / (r + 0.001);
			vel = lforce * len;
			//force += lforce * (1.0 / (len*len + 0.001) );
		}
	}
	else
	{
		vel = lforce;
	}
	
}

void ApplyVortex(TForce data, in float time, in vec3 particlePos, inout vec3 force, inout vec3 vel)
{
	float height = data.radius;
	float range = data.radius;
	float curve = 1.0;
	float downPower = 1.0;
	
	vec3 center = data.position.xyz;
	vec3 direction = data.direction.xyz;
	
	vec3 tmpPos = particlePos - center;
	vec3 point = (dot(tmpPos, data.direction.xyz)/dot(data.direction.xyz, data.direction.xyz) * data.direction.xyz);

	// if the particle we are testing against is above the vortex it shouldn't affect that particle
	bool cut = bool(clamp(dot(point, direction), 0.0, 1.0));
	
	// TODO: we can make a force to move to/from the center of motor

	vec3 pointVec = tmpPos - point;
	vec3 pullVec = pointVec;
	
	float vort = length(point);
	float percentVort = ((height - vort)/height);
	range *= clamp(pow(percentVort, curve), 0.0, 1.0);
	
	float dist = length(pointVec);
	float downDist = length(point);
	
	float downPercent = ((height - downDist)/height);
	float rangePercent = ((range - dist)/range);
	
	rangePercent = clamp(rangePercent, 0.0, 1.0);
	downPercent = clamp(downPercent, 0.0, 1.0);
	
	vec3 spinVec = cross(direction, pointVec);
	vec3 downVec = normalize(direction);
	
	normalize(spinVec);
	normalize(pullVec);
	
	vel += (spinVec * data.direction.w - pullVec * data.magnitude
		+ downVec * downPower) * rangePercent * float(cut);
	/*
	float r = data.radius;
	
	if (r > 0.0)
	{
		
		float len = length(center - particlePos);
		if (len <= r)
		{
			len = 1.0 - len / (r + 0.001);
			force = lforce * len;
			//force += lforce * (1.0 / (len*len + 0.001) );
		}
	}
	else
	{
		force =  lforce;
	}
	*/
}

void ApplyConstraint(in float time, in vec3 particleHold, inout vec3 force, inout vec3 vel, inout vec3 pos)
{
	vec3 direction = pos - particleHold;
	float distance = length(direction);
	
	if (distance != 0.0)
	{
		
		pos = mix(pos, particleHold, clamp(CONSTRAINT_MAGN * time, 0.0, 1.0)) ;
		
		direction /= distance;
		vel *= 1.0 - CONSTRAINT_MAGN; // * max(1.0, (1.0 / (distance*distance))); // * data.magnitude;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
	// particle id in the array
	uint flattened_id = get_invocation();
	
	// ?! skip unused part of the array
	if (flattened_id >= gNumParticles)
		return;
	
	// Read position and velocity
	vec4 pos = particleBuffer.particles[flattened_id].Pos;
	vec4 vel = particleBuffer.particles[flattened_id].Vel;
	vec4 rot = particleBuffer.particles[flattened_id].Rot;
	
	float lifetime = vel.w;
	
	if (lifetime == 0.0)
		return;
	
	float Age = rot.w + DeltaTimeSecs; 
	
	// launcher has a negative size and negative lifetime value
	if (pos.w < 0.0 || lifetime < 0.0) // PARTICLE_TYPE_LAUNCHER
	{
		particleBuffer.particles[flattened_id].Rot = vec4(rot.xyz, Age);
		return;
	}
	else if (Age >= lifetime)
	{
		// dead particle, don't process it
		particleBuffer.particles[flattened_id].Vel = vec4(0.0);	
		return;
	}
	
	float randomF = gTime + float(flattened_id);
	
	// predicted position next timestep
	vec3 pos_next = pos.xyz + vel.xyz * DeltaTimeSecs; 

	// update velocity - gravity force
	vec3 force = GRAVITY * USE_GRAVITY;	// in w - use gravity flag
	
	if (USE_FORCES)
	{
		for (int i=0; i<gNumForces; ++i)
		{
			float type = forceBuffer.forces[i].position.w;
			
			if (type == FORCE_WIND) 
				ApplyWindForce2(forceBuffer.forces[i], gTime * 0.01, pos_next, force, vel.xyz, DeltaTimeSecs);
			else if (type == FORCE_DRAG) 
				ApplyDragForce2(forceBuffer.forces[i], gTime * 0.01, pos_next, force, vel.xyz);
			else if (type == FORCE_MOTOR) 
				ApplyMotorForce(forceBuffer.forces[i], gTime * 0.01, pos_next, force, vel.xyz);
		}
	}
	
	if (USE_FLOOR > 0.0) 
		FloorCollide(pos_next, vel.xyz, randomF, FLOOR_LEVEL, FLOOR_FRICTION, DeltaTimeSecs);
	
	//
	// Process all collisions
	//
	if (USE_COLLISIONS)
	{
		for(int i=0; i<gNumCollisions; ++i)
		{
			float type = collisionBuffer.collisions[i].position.w;
			
			if (type == COLLISION_SPHERE) 
			{
				SphereCollide(collisionBuffer.collisions[i], pos_next, vel.xyz, force);
			}
			else if (type == COLLISION_TERRIAN)
			{
				TerrainCollide(collisionBuffer.collisions[i], pos_next, randomF, vel.xyz);
			}
		}
	}	
	
	
	
	float inv_mass = 1.0 / MASS;
	float damping = DAMPING;
	vel.xyz += force * inv_mass * DeltaTimeSecs; // F = ma
	damping = 1.0 - (1.0 - damping) * DeltaTimeSecs;
	vel.xyz *= damping;
	//vel.xyz += force;
	
	// turbulence behaviour

	if (USE_TURBULENCE > 0.0)
	{
		float f2 = cos(gTime) - 2.2;
		float f3 = sin(gTime) + 0.5;
	
		vec3 noiseVel = vec3( 	snoise(pos.xyz*NOISE_FREQ + gTime*NOISE_SPEED), 
							snoise(pos.xyz*NOISE_FREQ + gTime*NOISE_SPEED+f2), 
							snoise(pos.xyz*NOISE_FREQ + gTime*NOISE_SPEED+f3) );
							
		vel.xyz += noiseVel * NOISE_AMP;
	}
	
	// update position
	//
	// new position = old position + velocity * deltaTime
	if (gUpdatePosition > 0)
	{
		//pos.xyz += vel.xyz * DeltaTimeSecs;
		pos.xyz = pos_next;
		
		if (CONSTRAINT_MAGN > 0.0)
		{
			ApplyConstraint(gTime * 0.01, rot.xyz, force, vel.xyz, pos.xyz);
		}
		
		if (USE_COLLISIONS)
		{
			for(int i=0; i<gNumCollisions; ++i)
			{
				float type = collisionBuffer.collisions[i].position.w;
				
				if (type == COLLISION_SPHERE) 
				{
					SphereConstraint(collisionBuffer.collisions[i], pos.xyz);
				}
				else if (type == COLLISION_TERRIAN)
				{
					TerrainConstraint(collisionBuffer.collisions[i], pos.xyz);
				}
			}
		}
		
		if (USE_FLOOR > 0.0) 
			FloorConstraint(pos.xyz, FLOOR_LEVEL);
	}	
	
	// animate size
	if (gUseSizeAttenuation > 0)
	{
		float normLife = Age / lifetime;
		pos.w = texture( SizeSampler, normLife ).r;
		//pos.w = normLife * 10.0;
	}
	
	// write back
	particleBuffer.particles[flattened_id].Pos = pos; // pos.w - particle size
	particleBuffer.particles[flattened_id].Vel = vel;	// in w we store lifetime
	particleBuffer.particles[flattened_id].Rot = vec4(rot.xyz, Age);
}
