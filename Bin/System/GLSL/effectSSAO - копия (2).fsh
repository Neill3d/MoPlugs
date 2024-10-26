
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AO fragment shader.
//  Realization based on Hemispherical Screen-Space Ambient Occlusion
// http://blog.evoserv.at/index.php/2012/12/hemispherical-screen-space-ambient-occlusion-ssao-for-deferred-renderers-using-openglglsl/
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#version 120

uniform	sampler2D	colorSampler;
uniform sampler2D	normalSampler;
uniform sampler2D	depthSampler;

uniform float 	distanceThreshold;	// = 10.5;
//uniform float 	filterRadiusX;	// = vec2( 0.02, 0.02 );
//uniform float	filterRadiusY;
uniform vec2	filterRadius;
uniform int		only_ao;

uniform float	gamma;
uniform float	contrast;

uniform mat4	g_matInvProjection;

uniform float 		zNear = 10.0;
uniform float 		zFar = 4000.0;

const vec2 oflut[49] = vec2[](
    vec2(-0.864,-0.930),
    vec2(-0.952,-0.608),
    vec2(-0.820,-0.318),
    vec2(-0.960,-0.076),
    vec2(-0.939,0.244),
    vec2(-0.827,0.421),
    vec2(-0.871,0.561),
    vec2(-0.668,-0.810),
    vec2(-0.749,-0.557),
    vec2(-0.734,-0.422),
    vec2(-0.681,-0.043),
    vec2(-0.598,0.132),
    vec2(-0.720,0.323),
    vec2(-0.513,0.723),
    vec2(-0.371,-0.829),
    vec2(-0.366,-0.735),
    vec2(-0.259,-0.318),
    vec2(-0.344,-0.079),
    vec2(-0.386,0.196),
    vec2(-0.405,0.425),
    vec2(-0.310,0.518),
    vec2(-0.154,-0.931),
    vec2(-0.171,-0.572),
    vec2(-0.241,-0.420),
    vec2(-0.130,-0.225),
    vec2(-0.092,0.189),
    vec2(-0.018,0.310),
    vec2(-0.179,0.512),
    vec2(0.133,-0.942),
    vec2(0.235,-0.738),
    vec2(0.229,-0.380),
    vec2(0.027,-0.030),
    vec2(0.052,0.183),
    vec2(0.141,0.415),
    vec2(0.129,0.736),
    vec2(0.340,-0.931),
    vec2(0.254,-0.564),
    vec2(0.388,-0.417),
    vec2(0.363,-0.104),
    vec2(0.413,0.234),
    vec2(0.421,0.321),
    vec2(0.423,0.653),
    vec2(0.631,-0.756),
    vec2(0.665,-0.736),
    vec2(0.551,-0.350),
    vec2(0.526,-0.220),
    vec2(0.519,0.053),
    vec2(0.750,0.321),
    vec2(0.736,0.641) );

vec3 decodeNormal(in vec2 normal)
{
	// restore normal
	return normalize(texture2D(normalSampler, normal).xyz);
}

float ZBuffer2Linear( float z ,float znear, float zfar)
{
    float z_n = 2.0 * z - 1.0;
    float z_e = 2.0 * znear * zfar / (zfar+znear-z_n*(zfar-znear));
    return z_e / 10.0;
}

vec3 calculatePosition(in vec2 coord)
{
	float n = zNear; // camera z near
	float f = zFar; // camera z far
	float z = texture2D(depthSampler, coord).x * 2.0 - 1.0;
	//return vec3((2.0 * n) / (f + n - z * (f - n)));	

	//float z_b = texture2D(depthBuffTex, vTexCoord).x;
	float z_b = z;
	float z_n = 2.0 * z_b - 1.0;
	float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
	//return z_e;

	// Get x/w and y/w from the viewport position
    float x = coord.x * 2.0 - 1.0;
    float y = coord.y * 2.0 - 1.0;
    vec4 vProjectedPos = vec4(x, y, z, 1.0);
    // Transform by the inverse projection matrix
    vec4 vPositionVS = g_matInvProjection * vProjectedPos;  
    // Divide by w to get the view-space position
    vPositionVS.xyz = vPositionVS.xyz / vPositionVS.w;
	//vPositionVS.z = z_e;
	return vPositionVS.xyz;
	
	//return vec3(coord, z_e);

	// restore position
	//return texture2D(posSampler, coord).xyz;
}


void main()
{
	vec2 outUV = gl_TexCoord[0].st;
	vec4 colorSample = texture2D(colorSampler, outUV);

    float zbuffer = texture2D(depthSampler, outUV).r;

    float rz = ZBuffer2Linear( zbuffer, zNear, zFar );

    float an = fract(fract(outUV.x*0.36257507)*0.38746515+outUV.y*0.32126721);

    float k1=cos(6.2831*an);
    float k2=sin(6.2831*an);

    // calculate occlusion factor
	float scale = distanceThreshold;
	float radius = filterRadius.x;
	
    float ao = 0.0;
    for( int i=0; i<49; i++ )
    {
        vec2 of = vec2( oflut[i].x*k1 - oflut[i].y*k2,
                        oflut[i].x*k2 + oflut[i].y*k1 );

        //sampling point
        vec2 sa =  outUV.xy + radius * of;

        //difference in zbuffer
        float zd = rz - ZBuffer2Linear( texture2D(depthSampler, sa).r, zNear, zFar);

        //adjust based on scale.
        zd = zd * scale;

        //accumulate zdiffs transfer func
        ao += clamp( zd, 0.0, 1.0 ) - clamp((zd-5.0)*.02,0.0,1.0);
    }

    //add in occlusion to color.
    ao = 1.0 - ao/49.0;

    //scale occlusion toward white.
    ao = clamp(ao, 0.0, 1.0);

    //add in contrast/gamma to colorsample.
    vec4 outColor = contrast*pow(colorSample*ao,vec4(gamma));
    
    //background color as a vertical grey ramp
    vec4 bgColor = vec4(mix(0.1, 0.5, sin(outUV.y*3.14159)));

    gl_FragColor = vec4(ao, ao, ao, 1.0); // mix(bgColor, outColor, colorSample.a);
}