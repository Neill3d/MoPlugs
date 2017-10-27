
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

void SphereCollide(inout vec3 x, inout vec3 vel, vec3 center, float r, vec3 sphere_vel, float friction, inout vec3 force)
{
    vec3 delta = x - center;
    float dist = length(delta);
    if (dist < r) {
//      x = center + delta*(r / dist);    
      vel += (delta / dist) * friction;
      vel += sphere_vel;
    }
}

void FloorCollide(inout vec3 x, inout vec3 vel, float level, float friction, inout vec3 force)
{
    if (x.y < level) {
//        x.y = level;
//        force.y += -vel.y*friction;
        vel.y += -vel.y*friction;
    }
}

void Gravitation(vec3 pos, vec3 mass_pos, inout vec3 force, float epsilon)
{
  vec3 delta = mass_pos - pos;
  float dist = length(delta);
  vec3 dir = delta / dist;
  force += dir * (1.0 / (epsilon + dist*dist));
}

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
