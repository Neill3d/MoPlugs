
#pragma once

#define MAX_LIGHT_COUNT			8

namespace GraphicsCG
{

typedef	float	float3[3];
typedef float	float4[4];
typedef float	float4x4[16];


// material data
typedef struct {
	float4 	emission;
	float4 	ambient;
	float4 	diffuse;
	float4 	specular;
	
	int 	colorSamplerValid;
} MaterialDATA;

// shading data
typedef struct {
	int 	IDBufferRender;
	int		shading;
	float	toonSteps;
	float	toonDistribution;

	float	toonShadowPosition;
	float	brightness;
	float	saturation;
	float	contrast;
	
	float3 	customColor;			// w - custom color mode !
	int		customColorMode;
} ShaderDATA;

// Lighting data
typedef struct 
{
	float4	position;
	float4	direction;
	float4	color;
	float4	attenuation;
} LightDATA;

typedef struct {
	float4 		global_ambient;
	LightDATA	lights[MAX_LIGHT_COUNT];
	int 		numberOfLights;
} LightSet;




/////////////////////////////////////////////////////////////////////////////////////////////////////
// some helpful functions

void LightDATA_SetPosition4(const double *values, LightDATA &light);
void LightDATA_SetDirection4(const double *values, LightDATA &light);
void LightDATA_SetColor4(const double *values, LightDATA &light);
void LightDATA_SetAttenuation4(const double *values, LightDATA &light);

void LightSet_SetAmbient3(const double *color, LightSet &lightset);
void LightSet_SetCount(const int _count, LightSet &lightset);
void LightSet_SetPosition4(const int index, const double *_value, LightSet &lightset);
void LightSet_SetDirection4(const int index, const double *_value, LightSet &lightset);
void LightSet_SetColor4(const int index, const double *_value, LightSet &lightset);
void LightSet_SetAttenuation4(const int index, const double *_value, LightSet &lightset);

void MaterialDATA_Set( const float *_ambient, const float *_diffuse, const float *_specular, const float *_emissive, const float _shininess, MaterialDATA &material);
void MaterailDATA_SetAmbient4( const float *values, MaterialDATA &material );
void MaterailDATA_SetDiffuse4( const float *values, MaterialDATA &material );
void MaterailDATA_SetSpecular4( const float *values, MaterialDATA &material );
void MaterailDATA_SetEmissive4( const float *values, MaterialDATA &material );

////////////

};