
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filterDOF.cs
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

uniform float 		cameraNearPlane;	// camera near plane
uniform float 		cameraFarPlane;	// camera far plane

layout(binding=0, rgba8) uniform writeonly image2D resultImage;
layout(binding=1) 		uniform sampler2D 	colorSampler;
layout(binding=2) 		uniform sampler2D 	depthSampler;
layout(binding=3)		uniform sampler2D	blurSampler;

#include "misc_depth.cs"
#include "misc_masking.cs"


struct Data
{
	float 	focalDepth;		//focal distance value in meters, but you may use autofocus option below
	float 	focalLength;		//focal length in mm
	float 	fstop;			// = 0.5; //f-stop value
	//float	showFocus;		//show debug focus point and focal range (red = focal point, green = focal range)
	
	//------------------------------------------
	//user variables

	int samples; // = 3; //samples on the first ring
	int rings; // = 3; //ring count

	//float blurForeground;

	//bool manualdof; // = false; //manual dof calculation
	float ndofstart; // = 1.0; //near dof blur start
	float ndofdist; // = 2.0; //near dof blur falloff distance
	float fdofstart; // = 1.0; //far dof blur start
	float fdofdist; // = 3.0; //far dof blur falloff distance

	float CoC; // = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)

	//bool vignetting; // = true; //use optical lens vignetting?

	//bool autofocus; // = false; //use autofocus in shader? disable if you use external focalDepth value
	
	vec2 focus; // = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)


	float threshold; // = 0.5; //highlight threshold;
	float gain; // = 2.0; //highlight gain;

	float bias; // = 0.5; //bokeh edge bias
	float fringe; // = 0.7; //bokeh chromatic aberration/fringing

	// define USE_NOISE instead !
	//bool noise; // = true; //use noise instead of pattern for sample dithering

	/*
	next part is experimental
	not looking good with small sample and ring count
	looks okay starting from samples = 4, rings = 4
	*/

	//bool pentagon; // = false; //use pentagon as bokeh shape?
	float feather; // = 0.4; //pentagon shape feather
	
	float debugBlurValue;
	float temp2;
	float temp3;
};

layout (std430, binding = 0) buffer DataBuffer
{
	Data 	data;
	
} dataBuffer;


#define PI  3.14159265

float width = 1.0/outputSize.x; //texture width
float height = 1.0/outputSize.y; //texture height

vec2 texel = outputSize;

const vec2 rOffset = vec2(0.0,1.0)*texel;
const vec2 gOffset = vec2(-0.866,-0.5)*texel;
const vec2 bOffset = vec2(0.866,-0.5)*texel;

float vignout = 1.3; //vignetting outer border
float vignin = 0.0; //vignetting inner border
float vignfade = 22.0; //f-stops till vignete fades

float maxblur = 1.0; //clamp value of max blur (0.0 = no blur,1.0 default)
float namount = 0.0001; //dither amount

bool depthblur = false; //blur the depth buffer?
float dbsize = 1.25; //depthblursize

/* 
make sure that these two values are the same for your camera, otherwise distances will be wrong.
*/

float znear = cameraNearPlane; //camera clipping start
float zfar = cameraFarPlane; //camera clipping end


//------------------------------------------


float penta(vec2 coords) //pentagonal shape
{
	float scale = float(dataBuffer.data.rings) - 1.3;
	vec4  HS0 = vec4( 1.0,         0.0,         0.0,  1.0);
	vec4  HS1 = vec4( 0.309016994, 0.951056516, 0.0,  1.0);
	vec4  HS2 = vec4(-0.809016994, 0.587785252, 0.0,  1.0);
	vec4  HS3 = vec4(-0.809016994,-0.587785252, 0.0,  1.0);
	vec4  HS4 = vec4( 0.309016994,-0.951056516, 0.0,  1.0);
	vec4  HS5 = vec4( 0.0        ,0.0         , 1.0,  1.0);
	
	vec4  one = vec4( 1.0 );
	
	vec4 P = vec4((coords),vec2(scale, scale)); 
	
	vec4 dist = vec4(0.0);
	float inorout = -4.0;
	
	dist.x = dot( P, HS0 );
	dist.y = dot( P, HS1 );
	dist.z = dot( P, HS2 );
	dist.w = dot( P, HS3 );
	
	dist = smoothstep( -dataBuffer.data.feather, dataBuffer.data.feather, dist );
	
	inorout += dot( dist, one );
	
	dist.x = dot( P, HS4 );
	dist.y = HS5.w - abs( P.z );
	
	dist = smoothstep( -dataBuffer.data.feather, dataBuffer.data.feather, dist );
	inorout += dist.x;
	
	return clamp( inorout, 0.0, 1.0 );
}

float bdepth(vec2 coords) //blurring depth
{
	float d = 0.0;
	float kernel[9];
	vec2 offset[9];
	
	vec2 wh = vec2(texel.x, texel.y) * dbsize;
	
	offset[0] = vec2(-wh.x,-wh.y);
	offset[1] = vec2( 0.0, -wh.y);
	offset[2] = vec2( wh.x -wh.y);
	
	offset[3] = vec2(-wh.x,  0.0);
	offset[4] = vec2( 0.0,   0.0);
	offset[5] = vec2( wh.x,  0.0);
	
	offset[6] = vec2(-wh.x, wh.y);
	offset[7] = vec2( 0.0,  wh.y);
	offset[8] = vec2( wh.x, wh.y);
	
	kernel[0] = 1.0/16.0;   kernel[1] = 2.0/16.0;   kernel[2] = 1.0/16.0;
	kernel[3] = 2.0/16.0;   kernel[4] = 4.0/16.0;   kernel[5] = 2.0/16.0;
	kernel[6] = 1.0/16.0;   kernel[7] = 2.0/16.0;   kernel[8] = 1.0/16.0;
	
	
	for( int i=0; i<9; i++ )
	{
		float tmp = texture(depthSampler, coords + offset[i]).r;
		d += tmp * kernel[i];
	}
	
	return d;
}

vec3 color(in vec2 coords,float blur) //processing the sample
{
	vec3 col = vec3(0.0);
	
	col.r = texture(colorSampler, coords + rOffset * dataBuffer.data.fringe * blur).r;
	col.g = texture(colorSampler, coords + gOffset * dataBuffer.data.fringe * blur).g;
	col.b = texture(colorSampler, coords + bOffset * dataBuffer.data.fringe * blur).b;
	
	vec3 lumcoeff = vec3(0.299,0.587,0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum - dataBuffer.data.threshold) * dataBuffer.data.gain, 0.0);
	return col + mix(vec3(0.0),col, thresh * blur);
}

vec2 rand(in vec2 coord) //generating noise/pattern texture for dithering
{
	#ifdef USE_NOISE
		float noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
		float noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
	#else
		float noiseX = ((fract(1.0-coord.s*(width/2.0))*0.25)+(fract(coord.t*(height/2.0))*0.75))*2.0-1.0;
		float noiseY = ((fract(1.0-coord.s*(width/2.0))*0.75)+(fract(coord.t*(height/2.0))*0.25))*2.0-1.0;
	#endif
	
	return vec2(noiseX,noiseY);
}

vec3 debugFocus(vec3 col, float blur, float depth)
{
	float edge = 0.002*depth; //distance based edge smoothing
	float m = clamp(smoothstep(0.0,edge,blur),0.0,1.0);
	float e = clamp(smoothstep(1.0-edge,1.0,blur),0.0,1.0);
	
	col = mix(col,vec3(1.0,0.5,0.0),(1.0-m)*0.6);
	col = mix(col,vec3(0.0,0.5,1.0),((1.0-e)-(1.0-m))*0.2);

	return col;
}

float linearize(float depth)
{
	return -zfar * znear / (depth * (zfar - znear) - zfar);
}

float vignette(in vec2 texCoord)
{
	float dist = distance(texCoord.xy, vec2(0.5,0.5)); // gl_TexCoord[3].xy
	dist = smoothstep(vignout + (dataBuffer.data.fstop / vignfade), 
		vignin + (dataBuffer.data.fstop / vignfade), dist);
	return clamp(dist,0.0,1.0);
}

void textureWithKernel(in sampler2D sampler, in vec2 tx, in vec2 texelSize, out vec4 result)
{
	vec4 color, color2;
	vec4 tap0 = texture(sampler, tx);
	vec4 tap1 = texture(sampler, tx + texelSize * vec2( 0.4,  0.9));
	vec4 tap2 = texture(sampler, tx + texelSize * vec2(-0.4, -0.9));
	vec4 tap3 = texture(sampler, tx + texelSize * vec2(-0.9,  0.4));
	vec4 tap4 = texture(sampler, tx + texelSize * vec2( 0.9, -0.4));
	
	color = 0.2 * (tap0 + tap1 + tap2 + tap3 + tap4);
	
	vec4 tap11 = texture(sampler, tx + texelSize * vec2( 0.9,  1.9));
	vec4 tap21 = texture(sampler, tx + texelSize * vec2(-0.9, -1.9));
	vec4 tap31 = texture(sampler, tx + texelSize * vec2(-1.9,  0.9));
	vec4 tap41 = texture(sampler, tx + texelSize * vec2( 1.9, -0.9));
	
	color2 = 0.2 * (tap0 + tap11 + tap21 + tap31 + tap41);
	
	float mask = clamp(color2.w, 0.0, 1.0);
	
	result.rgb = color.rgb * mask + color2.rgb * (1.0-mask);
	result.w = mask;
}

/////////////////////////////////////////////////////////////////////////////
// MAIN
void main(void)
{
	// Assign these variables now because they will be used next
	ivec2 pixelPosition = ivec2(gl_GlobalInvocationID.xy);
	vec2 texCoord = vec2(outputSize.x * (0.5 + pixelPosition.x), outputSize.y * (0.5 + pixelPosition.y));
	
#ifdef SAME_SIZE
	vec4 srccolor = texelFetch(colorSampler, pixelPosition, 0);
	//float dValue = texelFetch(depthSampler, pixelPosition, 0).x;
#else
	// do some scaling with bilinear filtering
	vec4 srccolor = texture(colorSampler, texCoord );
	//float dValue = texture(depthSampler, texCoord).x;
#endif
	float dValue = texture(depthSampler, texCoord).x;
	vec4 blurcolor = texture(blurSampler, texCoord);
	//vec4 blurcolor = vec4(1.0);
	//textureWithKernel(blurSampler, texCoord, outputSize, blurcolor);

	if (depthblur)
	{
		dValue = bdepth(texCoord);
	}

	float texelDepth = 0.0;
	ComputeDepth(dValue, texelDepth);

	//focal plane calculation
	
	float focalDepth = dataBuffer.data.focalDepth / zfar;
	//dValue = texture(depthSampler, vec2(0.1,0.1)).x;
	//ComputeDepth(dValue, fDepth);
	//dof blur factor calculation
	
	float blur = 0.0;
	
#ifdef MANUAL_DOF
	
	float a = texelDepth-focalDepth; //focal plane
	float b = (a - dataBuffer.data.fdofstart) / dataBuffer.data.fdofdist; //far DoF
	float c = (-a - dataBuffer.data.ndofstart) / dataBuffer.data.ndofdist; //near Dof
	blur = (a>0.0)?b:c;
#else
	
	float f = dataBuffer.data.focalLength * 0.01; //focal length in mm
	float d = focalDepth *1000.0; //focal plane in mm
	float o = texelDepth *1000.0; //depth in mm
	
	float a = (o*f)/(o-f); 
	float b = (d*f)/(d-f); 
	float c = (d-f)/(d*dataBuffer.data.fstop*dataBuffer.data.CoC); 
	
	blur = 10.0 * abs(a-b)*c;
	
	a = texelDepth - focalDepth; // focal plane
#ifdef KEEP_FOREGROUND
	if (a < 0.0)
		blur = 0.0;
#endif

#endif
	/*
	if (depth < 0.1)
		blur = 1.0;
	else blur = 0.0;
	*/
	blur = clamp(blur,0.0,1.0);
	
	if (dataBuffer.data.debugBlurValue > 0.0)
	{
		vec4 outColor2 = ApplyMask(texCoord, srccolor, vec4(blur, blur, blur, 1.0));
		imageStore(resultImage, pixelPosition, outColor2);
		return;
	}

#ifdef FAST_PREVIEW

	vec3 col = mix(srccolor.rgb, blurcolor.rgb, blur);
	
#else
	// calculation of pattern for ditering
	
	vec2 noise = rand(texCoord)*namount*blur;
	
	// getting blur x and y step factor
	
	float w = (1.0/width)*blur*maxblur+noise.x;
	float h = (1.0/height)*blur*maxblur+noise.y;
	
	// calculation of final color
	
	vec3 col = srccolor.rgb;
	if (blur > 0.05) //some optimization thingy
	{
		col = srccolor.rgb;
		float s = 1.0;
		int ringsamples;
		
		for (int i = 1; i <= dataBuffer.data.rings; i += 1)
		{   
			ringsamples = i * dataBuffer.data.samples;
			
			for (int j = 0 ; j < ringsamples ; j += 1)   
			{
				float step = PI*2.0 / float(ringsamples);
				float pw = (cos(float(j)*step)*float(i));
				float ph = (sin(float(j)*step)*float(i));
				float p = 1.0;
				 
				#ifdef PENTAGON
					p = penta(vec2(pw,ph));
				#endif
				
				col += p * color(texCoord + vec2(pw*w,ph*h),blur)*mix(1.0,(float(i))/(float(dataBuffer.data.rings)), dataBuffer.data.bias);  
				s += 1.0 * p * mix(1.0,(float(i))/(float(dataBuffer.data.rings)), dataBuffer.data.bias);   
			}
		}
		col /= s; //divide by sample count
	}
#endif
	
	
#ifdef SHOW_FOCUS
		col = debugFocus(col, blur, depth);
#endif
	
#ifdef VIGNETTING
		col *= vignette(texCoord);
#endif
	
	vec4 outColor = ApplyMask(texCoord, srccolor, vec4(col, 1.0));
	imageStore(resultImage, pixelPosition, outColor);
}
