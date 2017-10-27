
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filterHalfTone.cs
//
//	composition toolkit filter
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

uniform vec2	outputSize;	// stored in a format ( 1.0/w, 1.0/h )

layout(binding=0, rgba8) uniform writeonly image2D resultImage;
layout(binding=1) 		uniform sampler2D 	colorSampler;

#include "misc_masking.cs"

float uScale = 1.0; // For imperfect, isotropic anti-aliasing in
float uYrot = 1.0;  // absence of dFdx() and dFdy() functions
uniform float frequency; // Needed globally for lame version of aastep()

/*
//T is any generic type (float,vec2, vec3 ...)
//I am assuming forward finite difference
T dFdx(T x) {
   T x2 = getValueOf(x, gl_FragCoord.x+1);
   T x1 = getValueOf(x, gl_FragCoord.x);
   return x2-x1;
}
T dFdy(T y) {
   T y2 = getValueOf(y, gl_FragCoord.y+1);
   T y1 = getValueOf(y, gl_FragCoord.y);
   return y2-y1;
}
*/

float dFdx(float value)
{
	return 0.0;
}
float dFdx(vec2 value)
{
	return 0.0;
}
float dFdy(float value)
{
	return 0.0;
}
float dFdy(vec2 value)
{
	return 0.0;
}
/*
// Anti-aliased step function. If the auto derivatives extension
// is supported, the AA is done in a fully general, anisotropic
// manner. If not, the expression for "afwidth" is a kludge for
// this particular shader and this particular view transform.
float aastep(float threshold, float value) {
  //float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
  float afwidth = frequency * (1.0/200.0) / uScale / cos(uYrot);
  return smoothstep(threshold-afwidth, threshold+afwidth, value);
}
*/
float aastep(float threshold, float value)
{
	return step(threshold, value);
}

// Explicit bilinear texture lookup to circumvent bad hardware precision.
// The extra arguments specify the dimension of the texture. (GLSL 1.30
// introduced textureSize() to get that information from the sampler.)
// 'dims' is the width and height of the texture, 'one' is 1.0/dims.
// (Precomputing 'one' saves two divisions for each lookup.)
vec4 texture2D_bilinear(sampler2D tex, vec2 st, vec2 dims, vec2 one) {
  vec2 uv = st * dims;
  vec2 uv00 = floor(uv - vec2(0.5)); // Lower left corner of lower left texel
  vec2 uvlerp = uv - uv00 - vec2(0.5); // Texel-local lerp blends [0,1]
  vec2 st00 = (uv00 + vec2(0.5)) * one;
  vec4 texel00 = texture(tex, st00);
  vec4 texel10 = texture(tex, st00 + vec2(one.x, 0.0));
  vec4 texel01 = texture(tex, st00 + vec2(0.0, one.y));
  vec4 texel11 = texture(tex, st00 + one);
  vec4 texel0 = mix(texel00, texel01, uvlerp.y); 
  vec4 texel1 = mix(texel10, texel11, uvlerp.y); 
  return mix(texel0, texel1, uvlerp.x);
}

// 2D simplex noise
 
// Description : Array- and textureless GLSL 2D simplex noise.
// Author : Ian McEwan, Ashima Arts. Version: 20110822
// Copyright (C) 2011 Ashima Arts. All rights reserved.
// Distributed under the MIT License. See LICENSE file.
// https://github.com/ashima/webgl-noise
 
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289((( x * 34.0) + 1.0) * x); }
 
float snoise(vec2 v) {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
  // First corner
  vec2 i = floor(v + dot(v, C.yy) );
  vec2 x0 = v - i + dot(i, C.xx);
  // Other corners
  vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  // Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
                           + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
                          dot(x12.zw,x12.zw)), 0.0);
  m = m*m; m = m*m;
  // Gradients
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 a0 = x - floor(x + 0.5);
  // Normalise gradients implicitly by scaling m
  m *= 1.792843 - 0.853735 * ( a0*a0 + h*h );
  // Compute final noise value at P
  vec3 g;
  g.x = a0.x * x0.x + h.x * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

void main (void)
{
	ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
	vec2 texCoord = vec2(outputSize.x * (0.5 + pixelPosition.x), outputSize.y * (0.5 + pixelPosition.y));
	
#ifdef SAME_SIZE
	vec4 srccolor = texelFetch(colorSampler, pixelPosition, 0);
#else
	// do some scaling with bilinear filtering
	vec4 srccolor = texture(colorSampler, texCoord );
#endif
	
	vec2 uDims = outputSize;
	vec2 vOne = vec2(1.0/outputSize.x, 1.0/outputSize.y);
	
	// Use a texture to modulate the size of the dots, and
    // use explicit bilinear interpolation for better precision
    vec3 texcolor = texture2D_bilinear(colorSampler, texCoord, uDims, vOne).rgb;
 
    float n = 0.1*snoise(texCoord*200.0);  // Fractal noise
    n += 0.05*snoise(texCoord*400.0);      // with three
    n += 0.025*snoise(texCoord*800.0);     // octaves
    vec3 white = vec3(n*0.2 + 0.97); // Paper color + noise
    vec3 black = vec3(n + 0.1);      // Ink density + noise
 
    // Perform a crude RGB-to-CMYK conversion
    vec4 cmyk;
    cmyk.xyz = 1.0 - texcolor; // CMY = 1-RGB
    // Black generation: K = min(C,M,Y)
    cmyk.w = min(cmyk.x, min(cmyk.y, cmyk.z));
    // Grey component replacement: subtract K from CMY
    cmyk.xyz -= cmyk.w;
 
    // Distances to nearest point in angled grids of
    // (frequency x frequency) points over the unit square
    // K component: 45 degrees screen angle
    vec2 Kst = frequency*mat2(0.707, -0.707, 0.707, 0.707)*texCoord;
    vec2 Kuv = 2.0*fract(Kst)-1.0;
    float k = aastep(0.0, sqrt(cmyk.w)-length(Kuv)+n);
    // C component: 15 degrees screen angle
    vec2 Cst = frequency*mat2(0.966, -0.259, 0.259, 0.966)*texCoord;
    vec2 Cuv = 2.0*fract(Cst)-1.0;
    float c = aastep(0.0, sqrt(cmyk.x)-length(Cuv)+n);
    // M component: -15 degrees screen angle
    vec2 Mst = frequency*mat2(0.966, 0.259, -0.259, 0.966)*texCoord;
    vec2 Muv = 2.0*fract(Mst)-1.0;
    float m = aastep(0.0, sqrt(cmyk.y)-length(Muv)+n);
    // Y component: 0 degrees screen angle
    vec2 Yst = frequency*texCoord;
    vec2 Yuv = 2.0*fract(Yst)-1.0;
    float y = aastep(0.0, sqrt(cmyk.z)-length(Yuv)+n);
 
    // CMY screen in RGB
    vec3 rgbscreen = 1.0 - 0.9*vec3(c,m,y) + n;
    // Blend in K for final color
    rgbscreen = mix(rgbscreen, black, 0.85*k + 0.3*n);
 /*
    // Blend to plain RGB texture under extreme minification
    // (handles any minification level by regular mipmapping)
    float afwidth = 2.0 * frequency * max(length(dFdx(texCoord)), length(dFdy(texCoord)));
	
    float blend = smoothstep(0.7, 1.4, afwidth); 
	vec4 outColor = vec4(mix(rgbscreen, texcolor, blend), 1.0); 
	*/
	vec4 outColor = vec4(rgbscreen, 1.0);
	
	outColor = ApplyMask(texCoord, srccolor, outColor);
	imageStore(resultImage, pixelPosition, outColor);	
}
