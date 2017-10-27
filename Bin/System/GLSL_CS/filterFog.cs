
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filterFog.cs
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
uniform vec2	renderSize;
uniform vec2	renderBorder;
uniform float	previewScaleFactor; // 1.0 for full image, 0.5 for half size


// next uniforms are STANDARTIZED !!
uniform float 		cameraNearPlane;	// camera near plane
uniform float 		cameraFarPlane;	// camera far plane

uniform mat4		cameraViewProj;

layout(binding=0, rgba8) uniform writeonly image2D resultImage;
layout(binding=1) 		uniform sampler2D 	colorSampler;
layout(binding=2)		uniform sampler2D 	depthSampler;
layout(binding=5)		uniform sampler2D	positionSampler;

// custom sampler
layout(binding=3)		uniform sampler2D 	alphaSampler;

struct FogData
{
	vec4		color;

	float		density;
	float		feather;
	float		near;
	float		far;
	
	vec4		volumeInfo;		// x - number of volume models
	
	vec4		viewDir[4];
	vec4		viewOrig[4];
};

struct VolumeData
{
	float		density;
	float		feather;
	float		blendMode;
	float		temp2;
	
	vec4		color;
	
	vec4		volumeMin;
	vec4		volumeMax;
	mat4		invVolumeMatrix;
};

layout (std430, binding = 0) buffer FogBuffer
{
	FogData 	data;
} fogBuffer;

layout (std430, binding = 1) buffer VolumeBuffer
{
	VolumeData	data[];
} volumeBuffer;


/*
** Hue, saturation, luminance
*/

vec3 RGBToHSL(vec3 color)
{
	vec3 hsl; // init to 0 to avoid warnings ? (and reverse if + remove first part)
	
	float fmin = min(min(color.r, color.g), color.b);    //Min. value of RGB
	float fmax = max(max(color.r, color.g), color.b);    //Max. value of RGB
	float delta = fmax - fmin;             //Delta RGB value

	hsl.z = (fmax + fmin) / 2.0; // Luminance

	if (delta == 0.0)		//This is a gray, no chroma...
	{
		hsl.x = 0.0;	// Hue
		hsl.y = 0.0;	// Saturation
	}
	else                                    //Chromatic data...
	{
		if (hsl.z < 0.5)
			hsl.y = delta / (fmax + fmin); // Saturation
		else
			hsl.y = delta / (2.0 - fmax - fmin); // Saturation
		
		float deltaR = (((fmax - color.r) / 6.0) + (delta / 2.0)) / delta;
		float deltaG = (((fmax - color.g) / 6.0) + (delta / 2.0)) / delta;
		float deltaB = (((fmax - color.b) / 6.0) + (delta / 2.0)) / delta;

		if (color.r == fmax )
			hsl.x = deltaB - deltaG; // Hue
		else if (color.g == fmax)
			hsl.x = (1.0 / 3.0) + deltaR - deltaB; // Hue
		else if (color.b == fmax)
			hsl.x = (2.0 / 3.0) + deltaG - deltaR; // Hue

		if (hsl.x < 0.0)
			hsl.x += 1.0; // Hue
		else if (hsl.x > 1.0)
			hsl.x -= 1.0; // Hue
	}

	return hsl;
}

float HueToRGB(float f1, float f2, float hue)
{
	if (hue < 0.0)
		hue += 1.0;
	else if (hue > 1.0)
		hue -= 1.0;
	float res;
	if ((6.0 * hue) < 1.0)
		res = f1 + (f2 - f1) * 6.0 * hue;
	else if ((2.0 * hue) < 1.0)
		res = f2;
	else if ((3.0 * hue) < 2.0)
		res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
	else
		res = f1;
	return res;
}

vec3 HSLToRGB(vec3 hsl)
{
	vec3 rgb;
	
	if (hsl.y == 0.0)
		rgb = vec3(hsl.z); // Luminance
	else
	{
		float f2;
		
		if (hsl.z < 0.5)
			f2 = hsl.z * (1.0 + hsl.y);
		else
			f2 = (hsl.z + hsl.y) - (hsl.y * hsl.z);
			
		float f1 = 2.0 * hsl.z - f2;
		
		rgb.r = HueToRGB(f1, f2, hsl.x + (1.0/3.0));
		rgb.g = HueToRGB(f1, f2, hsl.x);
		rgb.b= HueToRGB(f1, f2, hsl.x - (1.0/3.0));
	}
	
	return rgb;
}


/*
** Float blending modes
** Adapted from here: http://www.nathanm.com/photoshop-blending-math/
** But I modified the HardMix (wrong condition), Overlay, SoftLight, ColorDodge, ColorBurn, VividLight, PinLight (inverted layers) ones to have correct results
*/

#define BlendLinearDodgef 			BlendAddf
#define BlendLinearBurnf 			BlendSubstractf
#define BlendAddf(base, blend) 		min(base + blend, 1.0)
#define BlendSubstractf(base, blend) 	max(base + blend - 1.0, 0.0)
#define BlendLightenf(base, blend) 		max(blend, base)
#define BlendDarkenf(base, blend) 		min(blend, base)
#define BlendLinearLightf(base, blend) 	(blend < 0.5 ? BlendLinearBurnf(base, (2.0 * blend)) : BlendLinearDodgef(base, (2.0 * (blend - 0.5))))
#define BlendScreenf(base, blend) 		(1.0 - ((1.0 - base) * (1.0 - blend)))
#define BlendOverlayf(base, blend) 	(base < 0.5 ? (2.0 * base * blend) : (1.0 - 2.0 * (1.0 - base) * (1.0 - blend)))
#define BlendSoftLightf(base, blend) 	((blend < 0.5) ? (2.0 * base * blend + base * base * (1.0 - 2.0 * blend)) : (sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend)))
#define BlendColorDodgef(base, blend) 	((blend == 1.0) ? blend : min(base / (1.0 - blend), 1.0))
#define BlendColorBurnf(base, blend) 	((blend == 0.0) ? blend : max((1.0 - ((1.0 - base) / blend)), 0.0))
#define BlendVividLightf(base, blend) 	((blend < 0.5) ? BlendColorBurnf(base, (2.0 * blend)) : BlendColorDodgef(base, (2.0 * (blend - 0.5))))
#define BlendPinLightf(base, blend) 	((blend < 0.5) ? BlendDarkenf(base, (2.0 * blend)) : BlendLightenf(base, (2.0 *(blend - 0.5))))
#define BlendHardMixf(base, blend) 	((BlendVividLightf(base, blend) < 0.5) ? 0.0 : 1.0)
#define BlendReflectf(base, blend) 		((blend == 1.0) ? blend : min(base * base / (1.0 - blend), 1.0))


/*
** Vector3 blending modes
*/

// Component wise blending
#define Blend(base, blend, funcf) 		vec3(funcf(base.r, blend.r), funcf(base.g, blend.g), funcf(base.b, blend.b))

#define BlendNormal(base, blend) 		(blend)
#define BlendLighten				BlendLightenf
#define BlendDarken				BlendDarkenf
#define BlendMultiply(base, blend) 		(base * blend)
#define BlendAverage(base, blend) 		((base + blend) / 2.0)
#define BlendAdd(base, blend) 		min(base + blend, vec3(1.0))
#define BlendSubstract(base, blend) 	max(base + blend - vec3(1.0), vec3(0.0))
#define BlendDifference(base, blend) 	abs(base - blend)
#define BlendNegation(base, blend) 	(vec3(1.0) - abs(vec3(1.0) - base - blend))
#define BlendExclusion(base, blend) 	(base + blend - 2.0 * base * blend)
#define BlendScreen(base, blend) 		Blend(base, blend, BlendScreenf)
#define BlendOverlay(base, blend) 		Blend(base, blend, BlendOverlayf)
#define BlendSoftLight(base, blend) 	Blend(base, blend, BlendSoftLightf)
#define BlendHardLight(base, blend) 	BlendOverlay(blend, base)
#define BlendColorDodge(base, blend) 	Blend(base, blend, BlendColorDodgef)
#define BlendColorBurn(base, blend) 	Blend(base, blend, BlendColorBurnf)
#define BlendLinearDodge			BlendAdd
#define BlendLinearBurn			BlendSubstract
// Linear Light is another contrast-increasing mode
// If the blend color is darker than midgray, Linear Light darkens the image by decreasing the brightness. If the blend color is lighter than midgray, the result is a brighter image due to increased brightness.
#define BlendLinearLight(base, blend) 	Blend(base, blend, BlendLinearLightf)
#define BlendVividLight(base, blend) 	Blend(base, blend, BlendVividLightf)
#define BlendPinLight(base, blend) 		Blend(base, blend, BlendPinLightf)
#define BlendHardMix(base, blend) 		Blend(base, blend, BlendHardMixf)
#define BlendReflect(base, blend) 		Blend(base, blend, BlendReflectf)
#define BlendGlow(base, blend) 		BlendReflect(blend, base)
#define BlendPhoenix(base, blend) 		(min(base, blend) - max(base, blend) + vec3(1.0))
#define BlendOpacity(base, blend, F, O) 	(F(base, blend) * O + base * (1.0 - O))


// Hue Blend mode creates the result color by combining the luminance and saturation of the base color with the hue of the blend color.
vec3 BlendHue(vec3 base, vec3 blend)
{
	vec3 baseHSL = RGBToHSL(base);
	return HSLToRGB(vec3(RGBToHSL(blend).r, baseHSL.g, baseHSL.b));
}

// Saturation Blend mode creates the result color by combining the luminance and hue of the base color with the saturation of the blend color.
vec3 BlendSaturation(vec3 base, vec3 blend)
{
	vec3 baseHSL = RGBToHSL(base);
	return HSLToRGB(vec3(baseHSL.r, RGBToHSL(blend).g, baseHSL.b));
}

// Color Mode keeps the brightness of the base color and applies both the hue and saturation of the blend color.
vec3 BlendColor(vec3 base, vec3 blend)
{
	vec3 blendHSL = RGBToHSL(blend);
	return HSLToRGB(vec3(blendHSL.r, blendHSL.g, RGBToHSL(base).b));
}

// Luminosity Blend mode creates the result color by combining the hue and saturation of the base color with the luminance of the blend color.
vec3 BlendLuminosity(vec3 base, vec3 blend)
{
	vec3 baseHSL = RGBToHSL(base);
	return HSLToRGB(vec3(baseHSL.r, baseHSL.g, RGBToHSL(blend).b));
}


//
// BLEND MODE SUBROUTINES !
//

#ifdef USE_VOLUME

subroutine vec3 BlendTYPE(vec3 base, vec3 blend);

//
// 25 modes in total
//
layout(index=0) subroutine(BlendTYPE) vec3 blendNormal(vec3 base, vec3 blend) {
	return blend;
}

layout(index=1) subroutine(BlendTYPE) vec3 blendLighten(vec3 base, vec3 blend) {
	return BlendLighten(base, blend);
}		
layout(index=2) subroutine(BlendTYPE) vec3 blendDarken(vec3 base, vec3 blend) {
	return BlendDarken(base, blend);
}
layout(index=3) subroutine(BlendTYPE) vec3 blendMultiply(vec3 base, vec3 blend) {
	return BlendMultiply(base, blend);
}
layout(index=4) subroutine(BlendTYPE) vec3 blendAverage(vec3 base, vec3 blend) {
	return BlendAverage(base, blend);
}
layout(index=5) subroutine(BlendTYPE) vec3 blendAdd(vec3 base, vec3 blend) {
	return BlendAdd(base, blend);
}
layout(index=6) subroutine(BlendTYPE) vec3 blendSubstract(vec3 base, vec3 blend) {
	return BlendSubstract(base, blend);
}
layout(index=7) subroutine(BlendTYPE) vec3 blendDifference(vec3 base, vec3 blend) {
	return BlendDifference(base, blend);
}
layout(index=8) subroutine(BlendTYPE) vec3 blendNegation(vec3 base, vec3 blend) {
	return BlendNegation(base, blend);
}
layout(index=9) subroutine(BlendTYPE) vec3 blendExclusion(vec3 base, vec3 blend) {
	return BlendExclusion(base, blend);
}
layout(index=10) subroutine(BlendTYPE) vec3 blendScreen(vec3 base, vec3 blend) {
	return BlendScreen(base, blend);
}
layout(index=11) subroutine(BlendTYPE) vec3 blendOverlay(vec3 base, vec3 blend) {
	return BlendOverlay(base, blend);
}
layout(index=12) subroutine(BlendTYPE) vec3 blendSoftLight(vec3 base, vec3 blend) {
	return BlendSoftLight(base, blend);
}
layout(index=13) subroutine(BlendTYPE) vec3 blendHardLight(vec3 base, vec3 blend) {
	return BlendHardLight(base, blend);
}
layout(index=14) subroutine(BlendTYPE) vec3 blendColorDodge(vec3 base, vec3 blend) {
	return BlendColorDodge(base, blend);
}
layout(index=15) subroutine(BlendTYPE) vec3 blendColorBurn(vec3 base, vec3 blend) {
	return BlendColorBurn(base, blend);
}
layout(index=16) subroutine(BlendTYPE) vec3 blendLinearDodge(vec3 base, vec3 blend) {
	return BlendLinearDodge(base, blend);
}
layout(index=17) subroutine(BlendTYPE) vec3 blendLinearBurn(vec3 base, vec3 blend) {
	return BlendLinearBurn(base, blend);
}
layout(index=18) subroutine(BlendTYPE) vec3 blendLinearLight(vec3 base, vec3 blend) {
	return BlendLinearLight(base, blend);
}
layout(index=19) subroutine(BlendTYPE) vec3 blendVividLight(vec3 base, vec3 blend) {
	return BlendVividLight(base, blend);
}
layout(index=20) subroutine(BlendTYPE) vec3 blendPinLight(vec3 base, vec3 blend) {
	return BlendPinLight(base, blend);
}
layout(index=21) subroutine(BlendTYPE) vec3 blendHardMix(vec3 base, vec3 blend) {
	return BlendHardMix(base, blend);
}
layout(index=22) subroutine(BlendTYPE) vec3 blendReflect(vec3 base, vec3 blend) {
	return BlendReflect(base, blend);
}
layout(index=23) subroutine(BlendTYPE) vec3 blendGlow(vec3 base, vec3 blend) {
	return BlendGlow(base, blend);
}
layout(index=24) subroutine(BlendTYPE) vec3 blendPhoenix(vec3 base, vec3 blend) {
	return BlendPhoenix(base, blend);
}

//
subroutine uniform BlendTYPE BlendModeFunc[25];		// store each blend mode
#endif

//
#include "misc_depth.cs"
// NOTE: when using subroutines, no way to include blend functions, only direct implementation
//#include "misc_blending.cs"
#include "misc_masking.cs"

///////////////////////////////////////////////////////////////////////////////
//

void InterpolateViewParams(in vec2 uv, out vec4 viewOrig, out vec4 viewDir)
{
	vec4 north = mix( fogBuffer.data.viewDir[1], fogBuffer.data.viewDir[2], uv.x );
	vec4 source = mix( fogBuffer.data.viewDir[0], fogBuffer.data.viewDir[3], uv.x );
	viewDir = mix( north, source, 1.0-uv.y );
	
	north = mix( fogBuffer.data.viewOrig[1], fogBuffer.data.viewOrig[2], uv.x );
	source = mix( fogBuffer.data.viewOrig[0], fogBuffer.data.viewOrig[3], uv.x );
	viewOrig = mix( north, source, 1.0-uv.y );
}

#ifdef USE_VOLUME
void ComputeVolumeObject(inout vec4 color, in int index, in vec3 viewPoint)
{
	vec4 p = volumeBuffer.data[index].invVolumeMatrix * vec4(viewPoint, 1.0);
	
	vec4 volumeMax = volumeBuffer.data[index].volumeMax;
	vec4 volumeMin = volumeBuffer.data[index].volumeMin;
	
	float distToBorder = p.x - volumeMin.x;
	distToBorder = min(distToBorder, volumeMax.x - p.x);
	
	distToBorder = min(distToBorder, p.y - volumeMin.y);
	distToBorder = min(distToBorder, volumeMax.y - p.y);
	
	distToBorder = min(distToBorder, p.z - volumeMin.z);
	distToBorder = min(distToBorder, volumeMax.z - p.z);
	
	distToBorder = clamp(distToBorder, 0.0, 1.0);
	
	vec4 fogColor = volumeBuffer.data[index].color;
	float density = volumeBuffer.data[index].density;
	float fogFactor = smoothstep(0.0, volumeBuffer.data[index].feather, distToBorder);
	
	int blendMode = int(volumeBuffer.data[index].blendMode);
	vec4 outcolor = vec4(1.0);
	outcolor.rgb = BlendModeFunc[blendMode](color.rgb, fogColor.rgb);
	//vec4 outcolor = vec4(BlendOpacity( color.rgb, fogColor.rgb, BlendOperation, 1.0 ), 1.0);
	color = mix(color, outcolor, fogFactor * density * color.a);
}
#endif

void main () 
{
	ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
	
	vec2 texCoord = vec2(outputSize.x * (0.5 + pixelPosition.x), outputSize.y * (0.5 + pixelPosition.y));
	//vec2 texRenderCoord = vec2(renderSize.x * (0.5 + 2.0*pixelPosition.x), renderSize.y * (0.5 + 2.0*pixelPosition.y));
#ifdef SAME_SIZE
	vec4 srccolor = texelFetch(colorSampler, pixelPosition, 0);
	//float d = texelFetch(depthSampler, pixelPosition, 0).x;
#else
	// do some scaling with bilinear filtering
	vec4 srccolor = texture(colorSampler, texCoord );
	//float d = texture(depthSampler, texCoord).x;
#endif
	//float d = texture(depthSampler, texCoord).x;
	//float depth = 0.0;
	//ComputeDepth(d, depth);
	
	vec3 p = texture(positionSampler, texCoord).xyz;
	vec4 clip_space_pos = cameraViewProj * vec4(p, 1.0);
	
	float ndc_depth = clip_space_pos.z / clip_space_pos.w;
	float d = (1.0 - 0.0) * 0.5 * ndc_depth + (1.0 + 0.0) * 0.5;
	
	float n = cameraNearPlane;
	float f = cameraFarPlane;
	
	float lz = 2.0 * d - 1.0; 
	float depth = (2.0 * n) / (f + n - lz * (f - n));
	
#ifdef USE_ALPHA
	vec4 alphaColor = texture(alphaSampler, texCoord);
#else
	vec4 alphaColor = vec4(1.0, 1.0, 1.0, 1.0);
#endif
	
#ifdef USE_VOLUME
/*
	ivec2 border = ivec2(previewScaleFactor*renderBorder);
	
	ivec2 cropPos = clamp(pixelPosition-border, ivec2(0), pixelPosition-border);
	vec2 cropSize = previewScaleFactor * (renderSize - 2.0*renderBorder);
	vec2 cropTexelSize = vec2(1.0) / cropSize;
	vec2 cropCoord = vec2(cropTexelSize.x * (0.5 + cropPos.x), cropTexelSize.y * (0.5 + cropPos.y));
	
	vec4 viewOrig, viewDir;
	InterpolateViewParams(cropCoord, viewOrig, viewDir);
	
	vec3 p = viewOrig.xyz + depth * (viewDir.xyz - viewOrig.xyz);
	*/
	//vec3 p = texture(positionSampler, texCoord).xyz;
	
	vec4 outcolor = srccolor;
	
	if (depth < 0.99)
	{
		int count = int(fogBuffer.data.volumeInfo.x);
		for (int i=0; i<count; ++i)
		{
			ComputeVolumeObject(outcolor, i, p);
		}
	}
#else
	
	// fog for background
	//depth = clamp(depth, zNear, zFar);
	
	float lfogNear = fogBuffer.data.near / cameraFarPlane; //fogNear;
	float lfogFar = fogBuffer.data.far / cameraFarPlane; //fogFar;
	
	float fogFactor = 0.0;
	
	vec4 fogColor = fogBuffer.data.color;
	float densityF = fogBuffer.data.density;
#ifdef USE_ALPHA
	densityF *= alphaColor.a;
	fogColor.rgb = alphaColor.rgb;
#endif
	// simple screen fog
	if (depth > lfogNear && depth < lfogFar) 
	{
		float distToBorder = abs(depth - lfogNear);
		distToBorder = min(distToBorder, abs(depth-lfogFar));
		distToBorder = clamp(distToBorder, 0.0, 1.0);
		
		fogFactor = smoothstep(0.0, 0.1*fogBuffer.data.feather, distToBorder);
	}
	//fogFactor = 1.0;
	float alphaSource = srccolor.a;
	vec4 outcolor = vec4(BlendOpacity( srccolor.rgb, fogColor.rgb, BlendOperation, 1.0 ), alphaSource);
	
	outcolor = mix(srccolor, outcolor, fogFactor * densityF);
	
#endif
	
	outcolor = ApplyMask(texCoord, srccolor, outcolor);
	imageStore(resultImage, pixelPosition, outcolor);
}