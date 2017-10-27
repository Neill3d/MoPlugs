
-- vertex

#version 330                                                                        

layout (location = 0) in float Type;                                                
layout (location = 1) in vec4 Position;                                             
layout (location = 2) in vec4 Velocity;                                             
layout (location = 3) in float Age;       
//layout (location = 4) in vec4 Rotation;
//layout (location = 5) in vec4 RotVelocity;                                          
  
out float Type0;                                                                    
out vec4 Position0;                                                                 
out vec4 Velocity0;                       
//out vec4 Rotation0;
//out vec4 RotVelocity0;                                          
out float Age0;                                                                     
out float Index0;

void main()                                                                         
{                                                                                   
    Type0 = Type;                                                                   
    Position0 = Position;                                                           
    Velocity0 = Velocity;                                                           
    Age0 = Age;  
	Index0 = 10.0;
	//Rotation0 = Rotation;
	//RotVelocity0 = RotVelocity;
}

-- geometry

#version 330                                                                        

precision highp int;

layout(points) in;                                                                  
layout(points) out;                                                                 
layout(max_vertices = 30) out;                                                      

in float Type0[];                                                                   
in vec4 Position0[];                                                                
in vec4 Velocity0[];                                                                
in float Age0[];  
in float Index0[];                                                                  
//in vec4 Rotation0[];                                                                
//in vec4 RotVelocity0[]; 

out float Type1;                                                                    
out vec4 Position1;                                                                 
out vec4 Velocity1;                                                                 
out float Age1;   
out float Index1;                                                                  
//out vec4 Rotation1;                                                                 
//out vec4 RotVelocity1; 

uniform float 					gDeltaTimeMillis;                                                     
uniform float 					gTime;                                                                                                                  
uniform int						gMode;		// actual particle count or use rate ?

uniform samplerBuffer 			gPositionBuffer;
//uniform samplerBuffer			gOldPositionBuffer;
uniform samplerBuffer			gNormalBuffer;

uniform float					gFixPosition;

uniform	vec4					gTerrain;			// w - use terrain ?, x-friction
uniform vec3					gTerrainOffset;
uniform vec3					gTerrainScale;
uniform vec3					gTerrainSize;		// texture dimentions
uniform sampler2D				gTerrainData;		// texture terrain 2D sampler

uniform int						gPositionCount;	// number of position and normal vertices (emitter positions)
uniform mat4					gTM;			// emitter transform
uniform float 					gLauncherLifetime;                                                    
uniform float 					gShellLifetime;  
uniform float					gShellLifetimeVariation;	// a percent of randomization                                                     
uniform float 					gSecondaryShellLifetime;                                              

uniform vec4					gDirection;		// vec3 - direction, 4th - use normals as dir or not
uniform vec3					gDirRandom;
uniform vec3					gVelocity;		// start particle velocity
uniform vec3					gVelRandom;		// randomize start velocity
uniform vec4					gEmitterVelocity; // use this velocity to inherit emitter speed
uniform vec4					gDynamic;		// 1st - mass, 2nd - damping
uniform vec4					gGravity;		// vec3 - gravity direction XYZ, 4-th component - use or not to use gravity
uniform vec4					gFlags;			// 1st - useForces, 2nd - useCollisions, 3rd - emitter type(0.0-vertices, 1.0-volume)
uniform vec4					gTurbulence;
uniform vec4					gFloor;			// 1st - use/not use, 2nd - friction, 3rd - Y level
uniform	vec3					gMin;
uniform	vec3					gMax;

#define		USE_TERRAIN			gTerrain.w
#define		TERRAIN_FRICTION	gTerrain.x

#define		DIRECTION			gDirection.xyz
#define		USE_NORMALS_AS_DIR	gDirection.w

#define		MASS				gDynamic.x
#define		DAMPING				gDynamic.y

#define		USE_GRAVITY			gGravity.w
#define		GRAVITY				gGravity.xyz

#define 	USE_TURBULENCE		gTurbulence.w
#define		NOISE_FREQ			gTurbulence.x
#define		NOISE_SPEED			gTurbulence.y
#define		NOISE_AMP			gTurbulence.z

#define		USE_FORCES			gFlags.x
#define		USE_COLLISIONS		gFlags.y
#define		EMITTER_TYPE		gFlags.z

#define		EMITTER_TYPE_VERTICES	0.0
#define		EMITTER_TYPE_VOLUME		1.0

#define		FORCE_WIND				1.0
#define		FORCE_DRAG				2.0
#define		FORCE_MOTOR				3.0

#define		USE_FLOOR			gFloor.w
#define		FLOOR_FRICTION		gFloor.y
#define		FLOOR_LEVEL			gFloor.z

struct Collision
{
	vec4			position;			// use w as collision type
	vec4			velocity;
	float 			radius;
	float			friction;
	float			dummy1;
	float			dummy2;
};
const int 						maxCollisions = 4;

uniform		int					gNumCollisions;
uniform		Collision 			gCollisions[maxCollisions];

struct	Force
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
const int						maxForces = 4;

uniform		int					gNumForces;
uniform		Force				gForces[maxForces];


#define PARTICLE_TYPE_LAUNCHER 0.0f                                                 
#define PARTICLE_TYPE_SHELL 1.0f                                                    
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f                                          

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//rotate vector
vec3 qrot(vec4 q, vec3 v)	{
	return v + 2.0*cross(q.xyz, cross(q.xyz,v) + q.w*v);
}

//rotate vector (alternative)
vec3 qrot_2(vec4 q, vec3 v)	{
	return v*(q.w*q.w - dot(q.xyz,q.xyz)) + 2.0*q.xyz*dot(q.xyz,v) + 2.0*q.w*cross(q.xyz,v);
}

//to matrix
mat3 qmatrix(vec4 q)	{
	return mat3(
		vec3(q.w+q.x*q.x, 0.0, -q.x),
		vec3(0.0, q.w+q.y*q.y, -q.y),
		vec3(q.x, q.y, q.w*q.w) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMMON FUNCTIONS


// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }


// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

// constrain particle to be outside volume of a sphere
void SphereConstraint(inout vec3 x, vec3 center, float r)
{
    vec3 delta = x - center;
    float dist = length(delta);
    if (dist < r) {
        x = center + delta*(r / dist);
    }
}

// constrain particle to be above floor
void FloorConstraint(inout vec3 x, float level)
{
    if (x.y < level) {
        x.y = level;
    }
}

float unpackFloatFromVec4i(const vec4 value)
{
  const vec4 bitSh = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
  return(dot(value, bitSh));
}

// constrain particle to heightfield stored in texture
void TerrainConstraint(inout vec3 pos, vec3 scale, vec3 offset)
{
    vec2 uv = (pos.xz - offset.xz) / scale.xz;
	float h0 = unpackFloatFromVec4i( texture(gTerrainData, uv) );
    float h = h0*scale.y + offset.y;
    if (pos.y < h) {
        pos.y = h;
    }
}

void SphereCollide(inout vec3 x, inout vec3 vel, vec3 center, float r, vec3 sphere_vel, float friction, inout vec3 force)
{
    vec3 delta = x - center;
    float dist = length(delta);
    if (dist < r) {
//      x = center + delta*(r / dist);    
      vel -= (delta / dist) * friction;
      vel += sphere_vel;
    }
}

void FloorCollide(inout vec3 x, inout vec3 vel, const float time, float level, float friction, inout vec3 force, const float dt)
{
    if (x.y < level) {
//        x.y = level;
//        force.y += -vel.y*friction;
        vel.y += -vel.y*friction * random(time + 0.123) * dt;
		vel.x += random(time) * friction * dt;
		vel.z += random(time - 0.543) * friction * dt;
    }
}

void Gravitation(vec3 pos, vec3 mass_pos, inout vec3 force, float epsilon)
{
  vec3 delta = mass_pos - pos;
  float dist = length(delta);
  vec3 dir = delta / dist;
  force += dir * (1.0 / (epsilon + dist*dist));
}

void TerrainCollide(vec3 pos, inout vec3 vel, vec3 scale, vec3 offset, float friction)
{
	// should be predefined in shader
    vec2 texelSize = vec2( 1.0 / gTerrainSize.x, 1.0 / gTerrainSize.y );
    vec2 uv = (pos.xz - offset.xz) / scale.xz;
    float h0 = unpackFloatFromVec4i( texture(gTerrainData, uv) );
    float h = h0*scale.y + offset.y;
    if (pos.y < h) {
        // calculate normal (could precalc this)
        float h1 = texture(gTerrainData, uv + texelSize*vec2(1, 0) ).r;
        float h2 = texture(gTerrainData, uv + texelSize*vec2(0, 1) ).r;
        vec3 N = cross( vec3(scale.x*texelSize.x, (h1-h0)*scale.y, 0), vec3(0, (h2-h0)*scale.y, scale.z*texelSize.y) );
        N = normalize(N);
        vel = reflect(vel, N);
        vel *= friction;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

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

float calcNoise(float x)
{
	const float pi=3.1454;
	return cos(x*pi)*cos(x*3.0*pi)*cos(x*5.0*pi)*cos(x*7*pi) + sin(x*25*pi)*0.1;
}

float calcNoise2(float x)
{
	const float pi=3.1454;
	return (cos(x * pi)*cos(x*pi)) * cos(x*3*pi) * cos(x*5*pi)*0.5 + sin(x*25.0*pi)*0.02;
}

// hash based 3d value noise
float noisehash( float n )
{
    return fract(sin(n)*43758.5453);
}
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;
    return mix(mix(mix( noisehash(n+  0.0), noisehash(n+  1.0),f.x),
                   mix( noisehash(n+ 57.0), noisehash(n+ 58.0),f.x),f.y),
               mix(mix( noisehash(n+113.0), noisehash(n+114.0),f.x),
                   mix( noisehash(n+170.0), noisehash(n+171.0),f.x),f.y),f.z);
}

vec4 map( in vec3 p )
{
	float d = 0.2 - p.y;

	vec3 q = p - vec3(1.0,0.1,0.0)*gTime;
	float f;
    f  = 0.5000*noise( q ); q = q*2.02;
    f += 0.2500*noise( q ); q = q*2.03;
    f += 0.1250*noise( q ); q = q*2.01;
    f += 0.0625*noise( q );

	d += 3.0 * f;

	d = clamp( d, 0.0, 1.0 );
	
	vec4 res = vec4( d );

	//res.xyz = mix( 1.15*vec3(1.0,0.95,0.8), vec3(0.7,0.7,0.7), res.x );
	
	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN

vec3 GetRandomPos(float TexCoord, inout int vertIndex)
{
	float rnd = float(gPositionCount) * random( TexCoord );
	vertIndex = int(rnd);
	vec4 pos = texelFetch(gPositionBuffer, vertIndex);
	pos = gTM * pos;
	pos = pos;
	pos.xyz += gFixPosition * vec3(gTM[3][0], gTM[3][1], gTM[3][2]);	// try to fix a bug with matrix*pos mult
	
	return pos.xyz;
}

vec3 GetRandomDir(float TexCoord, int vertIndex)                                                   
{
	vec3 rnd = vec3(gDirRandom.x*random(TexCoord)-0.5*gDirRandom.x, 
					gDirRandom.y*random(TexCoord-1.1)-0.5*gDirRandom.y, 
					gDirRandom.z*random(TexCoord+0.76)-0.5*gDirRandom.z); 
	
	vec3 Dir;
	if (USE_NORMALS_AS_DIR > 0.0)
	{
		Dir = texelFetch(gNormalBuffer, vertIndex).xyz;
	}
	else
	{
		Dir = gDirection.xyz;
	}
    return (Dir - rnd);                                                                    
}                                                                                   

vec3 GetRandomVolumePos(float TexCoord)
{
	vec3 lMax = gMax;
	vec3 lMin = gMin;
	
	// if inherit emitter velocity, put particle on some random position during the way
	if (gEmitterVelocity.w > 0.0)
	{
		vec3 delta = gEmitterVelocity.xyz * random(TexCoord - 0.854);
		lMax -= delta;
		lMin -= delta;
	}

	vec3 rnd = vec3(random(TexCoord), random(TexCoord-1.23), random(TexCoord+0.26)); 
	
	vec3 pos;
	pos.x = (gMax.x - gMin.x) * rnd.x + gMin.x;
	pos.y = (gMax.y - gMin.y) * rnd.y + gMin.y;
	pos.z = (gMax.z - gMin.z) * rnd.z + gMin.z;
	
	return pos;
}

vec3 GetRandomVolumeDir(float TexCoord)                                                   
{
	vec3 rnd = vec3(gDirRandom.x*random(TexCoord)-0.5*gDirRandom.x, 
					gDirRandom.y*random(TexCoord-1.1)-0.5*gDirRandom.y, 
					gDirRandom.z*random(TexCoord+0.76)-0.5*gDirRandom.z); 
	
	vec3 Dir = gDirection.xyz;
	
    return (Dir - rnd);                                                                    
}  

vec3 GetRandomVelocity(float TexCoord)
{
	vec3 rnd = vec3(gVelRandom.x*random(TexCoord)-0.5*gVelRandom.x, 
					gVelRandom.y*random(TexCoord-2.1)-0.5*gVelRandom.y, 
					gVelRandom.z*random(TexCoord+2.76)-0.5*gVelRandom.z);
	return (gVelocity - rnd);
}

void ApplyWindForce(Force data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel, float dt)
{
	vec3 lforce = data.magnitude * normalize(data.direction.xyz);
	vec3 lvel = vec3(0.0);
	float r = data.radius;
	
	// calculate turbulence for wind if needed
	vec4 lTurbulence = data.turbulence;
	if (lTurbulence.w > 0.0)
	{
		//vec3 localPos = pos_next; // - gForces[i].position.xyz;
	/*
		//float noise = calcNoise( lTurbulence.y * ( 0.01 * DeltaTimeSecs + 0.01 * localPos.y + 0.01 * localPos.x + 0.01 * localPos.z) );
		float noise2 = calcNoise2( lTurbulence.y * gTime*0.001 );
		//float noise3 = snoise( vec3(1.0, 1.0, 1.0) * DeltaTimeSecs );
	
		float theta = lTurbulence.x * noise2 * 3.1415 / 180.0;
		//theta = lTurbulence.x * cos(lTurbulence.y * gTime*0.001) * 3.1415 / 180.0;
		
		float cs = cos(theta);
		float sn = sin(theta);
	
		vec3 d = normalize(gForces[i].direction.xyz);
		float x = 0.0;
		float y = 1.0;
	
		float px = x * cs - y * sn; 
		float py = x * sn + y * cs;
	
		lforce.x = px;
		lforce.y = py;
		lforce.z = 0.0;
	
		lforce = gForces[i].magnitude * normalize(lforce);
	*/
	
		float base_freq = lTurbulence.y; // SPIDER
		float t = time;
		vec3 MCposition = vec3(t, t+2.2, t-3.3);
		//MCposition += pos_next;
		
	
		vec4 noisevec;
		noisevec.x = snoise(MCposition * base_freq*1.0 + pos_next.x*0.001) * 8.0;
		noisevec.y = snoise(MCposition * base_freq*2.0 + pos_next.y*0.001) * 4.0;
		noisevec.z = snoise(MCposition * base_freq*4.0 + pos_next.z*0.001) * 2.0;
		noisevec.w = snoise(MCposition * base_freq*8.0) * 1.0;
		 //noisevec = (noisevec / lTurbulence.x + 1.0) / 2.0;
		noisevec = noisevec / (lTurbulence.x + 0.001);
		
		float intensity = abs(noisevec[0] - 0.20) + 
			  abs(noisevec[1] - 0.10) + 
							  abs(noisevec[2] - 0.05) +
							  abs(noisevec[3] - 0.025);
		
		//lforce *= intensity;
		lvel = noisevec.xyz;			// TODO: use it to customize particle motion in space (globaly)
		//lforce *= noisevec.xyz;
		
	}
	
	if (r > 0.0)
	{
		vec3 lpos = data.position.xyz;
		float len = length(lpos - pos_next);
		if (len <= r)
		{
			len = 1.0 - len / (r + 0.001);
			force += lforce * len;
			vel += lvel * len * dt;
			//force += lforce * (1.0 / (len*len + 0.001) );
		}
	}
	else
	{
		force +=  lforce;
		vel += lvel * dt;
	}
}

void ApplyWindForce2(Force data, in float Index, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel, float dt)
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

void ApplyDragForce(Force data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel)
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

void ApplyMotorForce(Force data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel)
{
	vec3 lpos = data.position.xyz;
	
	mat3 mat = rotationMatrix( data.direction.xyz, data.direction.w * 3.14 / 180.0 );
	
	vec3 v = pos_next - lpos;
	
	// TODO: we can make a force to move to/from the center of motor
	
	v = v * mat;
	v += lpos;
	vec3 lforce = data.magnitude * (v - pos_next);
	
	//vec3 lforce = data.magnitude * data.direction.xyz;
	float r = data.radius;
	
	if (r > 0.0)
	{
		
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

void main()                                                                         
{                                                                                   
    float Age = Age0[0] + gDeltaTimeMillis;                                         
	float randomF = gTime + gl_PrimitiveIDIn;
	
    if (Type0[0] == PARTICLE_TYPE_LAUNCHER) {                                       
        
		// check if we need this launch node after it has generated shell
		bool genNew = true;
		
		if (Age >= gLauncherLifetime) {

			Type1 = PARTICLE_TYPE_SHELL; 
			
			vec3 Dir;
			if (EMITTER_TYPE == EMITTER_TYPE_VOLUME)
			{
				// emit from volume
				Position1.xyz = GetRandomVolumePos(randomF);
				Dir = GetRandomVolumeDir(randomF); 
			}
			else
			{
				// emit from vertices
				int vertIndex=0;
				Position1.xyz = GetRandomPos(randomF, vertIndex);
				Dir = GetRandomDir(randomF, vertIndex);                                  
			}
			
			Position1.xyz = Position1.xyz - (random(randomF+0.487) * 2.0 - 1.0) * gEmitterVelocity.xyz;
			Velocity1.xyz = normalize(Dir) * GetRandomVelocity(randomF+0.55) + gEmitterVelocity.xyz;                                      
			Age1 = 0.0;                  
			Position1.w = 0.0;
			Velocity1.w = gShellLifetime + (random(randomF+0.487) * 2.0 - 1.0) * gShellLifetime * gShellLifetimeVariation;
			Index1 = random(randomF);
			//Rotation1 = Rotation0[0];
			//RotVelocity1 = RotVelocity0[0];
			EmitVertex();                                                           
			EndPrimitive();                                                         
			
            Age = gLauncherLifetime + (random(randomF+1.234) * 2.0 - 1.0) * gLauncherLifetime;   

			if (gMode < 1.0) genNew = false;
        }                                                                           
		
		if (genNew)
		{
			Type1 = PARTICLE_TYPE_LAUNCHER;                                             
			Position1 = Position0[0];
			Position1.xyz = vec3(0.0);
			Velocity1 = Velocity0[0]; 
			Velocity1.xyz = vec3(0.0);
			//Rotation1 = Rotation0[0];
			//RotVelocity1 = RotVelocity0[0];		
			Age1 = Age;      
			Index1 = Index0[0];
			EmitVertex();                                                               
			EndPrimitive();                                                             
		}
    }                                                                               
    else {                                                                          
        float DeltaTimeSecs = gDeltaTimeMillis;                                                

		vec3 pos = Position0[0].xyz;
		vec3 vel = Velocity0[0].xyz;
		//float Index = pos.x * 0.001;
		float Index = Index0[0];
		Index = gl_PrimitiveIDIn;
		Index = Index / 65535.0f;
		
		vec3 pos_next = pos + vel * DeltaTimeSecs; // predicted position next timestep
		
		// update velocity - gravity force
		vec3 force = vec3(0.0, 0.0, 0.0);
		if (USE_GRAVITY > 0.0) 
			force = GRAVITY;
		
		if (USE_FORCES > 0.0)
		{
			for (int i=0; i<gNumForces; ++i)
			{
				if (gForces[i].position.w == FORCE_WIND) ApplyWindForce2(gForces[i], Index, gTime * 0.01, pos_next, force, vel, DeltaTimeSecs);
				else if (gForces[i].position.w == FORCE_DRAG) ApplyDragForce(gForces[i], gTime * 0.01, pos_next, force, vel);
				else if (gForces[i].position.w == FORCE_MOTOR) ApplyMotorForce(gForces[i], gTime * 0.01, pos_next, force, vel);
			}
		}
		
		if (USE_FLOOR > 0.0) 
			FloorCollide(pos_next, vel, randomF, FLOOR_LEVEL, FLOOR_FRICTION, force, DeltaTimeSecs);
		
		//
		// Process all collisions
		//
		if (USE_COLLISIONS > 0.0)
		{
			for(int i=0; i<gNumCollisions; ++i)
			{
				if (gCollisions[i].position.w > 0.0) 
					SphereCollide(pos_next, vel, gCollisions[i].position.xyz, gCollisions[i].radius, gCollisions[i].velocity.xyz, gCollisions[i].friction, force);
			}
			
			if (USE_TERRAIN > 0.0)
			{
				TerrainCollide(pos_next, vel, gTerrainScale, gTerrainOffset, TERRAIN_FRICTION); 
			}
		}		
		
		float inv_mass = 1.0 / MASS;
		float damping = DAMPING;
		vel += force * inv_mass * DeltaTimeSecs; // F = ma
		damping = 1.0 - (1.0 - damping) * DeltaTimeSecs;
		vel *= damping;
		
		// turbulence behaviour
		
		if (USE_TURBULENCE > 0.0)
		{
			float f2 = cos(gTime) - 2.2;
			float f3 = sin(gTime) + 0.5;
		
			vec3 noiseVel = vec3( 	snoise(pos*NOISE_FREQ+gTime*NOISE_SPEED), 
								snoise(pos*NOISE_FREQ+gTime*NOISE_SPEED+f2), 
								snoise(pos*NOISE_FREQ+gTime*NOISE_SPEED+f3) );
								
			vel += noiseVel * NOISE_AMP;
		}
		
		// update position
		if (USE_FLOOR > 0.0) FloorConstraint(pos, FLOOR_LEVEL);
		
		if (USE_COLLISIONS > 0.0)
		{
			for(int i=0; i<gNumCollisions; ++i)
			{
				if (gCollisions[i].position.w > 0.0) 
					SphereConstraint(pos, gCollisions[i].position.xyz, gCollisions[i].radius);
			}
			
			if (USE_TERRAIN > 0.0)
			{
				TerrainConstraint(pos, gTerrainScale, gTerrainOffset);
			}
		}
		
		// new position = old position + velocity * deltaTime
		pos += vel * DeltaTimeSecs;
		
        if (Type0[0] == PARTICLE_TYPE_SHELL)  {                                     
	        if (Age < Velocity0[0].w) {                                             
	            Type1 = PARTICLE_TYPE_SHELL;                                        
	            Position1.xyz = pos;
				Velocity1.xyz = vel;                                
	            Age1 = Age;  
				Position1.w = Age / Velocity0[0].w;
				Velocity1.w = Velocity0[0].w;
				//Rotation1 = Rotation0[0];
				//RotVelocity1 = RotVelocity0[0];
				Index1 = Index0[0];
	            EmitVertex();                                                       
	            EndPrimitive();                                                     
	        }  
			else if (gMode == 0.0)
			{
				Type1 = PARTICLE_TYPE_LAUNCHER;                                             
				Position1 = Position0[0];
				Position1.xyz = vec3(0.0);
				Velocity1 = Velocity0[0]; 
				Velocity1.xyz = vec3(0.0);
				Age1 = gLauncherLifetime + (random(randomF+1.234) * 2.0 - 1.0) * gLauncherLifetime;                                                                 
				Index1 = random(randomF);
				EmitVertex();                                                               
				EndPrimitive(); 
			}
                                                                                  
        }                                                                                                                                                    
    }                                                                               
}