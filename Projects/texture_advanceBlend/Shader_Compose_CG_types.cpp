
#include "Shader_CG_types.h"
#include <memory>

//using namespace GraphicsCG;
namespace GraphicsCG
{

void LightDATA_SetPosition4(const double *values, LightDATA &light)
{
	light.position[0] = (float) values[0];
	light.position[1] = (float) values[1];
	light.position[2] = (float) values[2];
	light.position[3] = (float) values[3];
}
void LightDATA_SetDirection4(const double *values, LightDATA &light)
{
	//memcpy( direction, _direction, sizeof(double) * 4 );
	light.direction[0] = (float) values[0];
	light.direction[1] = (float) values[1];
	light.direction[2] = (float) values[2];
	light.direction[3] = (float) values[3];
}
void LightDATA_SetColor4(const double *values, LightDATA &light)
{
	//memcpy( color, _color, sizeof(double) * 4 );
	light.color[0] = (float) values[0];
	light.color[1] = (float) values[1];
	light.color[2] = (float) values[2];
	light.color[3] = (float) values[3];
}
void LightDATA_SetAttenuation4(const double *values, LightDATA &light)
{
	//memcpy( attenuation, _attenuation, sizeof(double) * 4 );
	light.attenuation[0] = (float) values[0];
	light.attenuation[1] = (float) values[1];
	light.attenuation[2] = (float) values[2];
	light.attenuation[3] = (float) values[3];
}

void LightSet_SetAmbient3(const double *color, LightSet &lightset)
{
	lightset.global_ambient[0] = (float) color[0];
	lightset.global_ambient[1] = (float) color[1];
	lightset.global_ambient[2] = (float) color[2];
	lightset.global_ambient[3] = 1.0f;
}
void LightSet_SetCount(const int _count, LightSet &lightset)
{
	lightset.numberOfLights = _count;
}
void LightSet_SetPosition4(const int index, const double *_value, LightSet &lightset)
{
	LightDATA_SetPosition4(_value, lightset.lights[index]);
}
void LightSet_SetDirection4(const int index, const double *_value, LightSet &lightset)
{
	LightDATA_SetDirection4(_value, lightset.lights[index]);
}
void LightSet_SetColor4(const int index, const double *_value, LightSet &lightset)
{
	LightDATA_SetColor4(_value, lightset.lights[index]);
}
void LightSet_SetAttenuation4(const int index, const double *_value, LightSet &lightset)
{
	LightDATA_SetAttenuation4(_value, lightset.lights[index]);
}

void MaterialDATA_Set( const float *_ambient, const float *_diffuse, const float *_specular, const float *_emissive, const float _shininess, MaterialDATA &material)
{
	memcpy( material.ambient, _ambient, sizeof(float) * 4 );
	memcpy( material.diffuse, _diffuse, sizeof(float) * 4 );
	memcpy( material.specular, _specular, sizeof(float) * 4 );
	memcpy( material.emission, _emissive, sizeof(float) * 4 );
	material.emission[3] = _shininess;
}
void MaterailDATA_SetAmbient4( const float *values, MaterialDATA &material )
{
	memcpy( material.ambient, values, sizeof(float) * 4 );
}
void MaterailDATA_SetDiffuse4( const float *values, MaterialDATA &material )
{
	memcpy( material.diffuse, values, sizeof(float) * 4 );
}
void MaterailDATA_SetSpecular4( const float *values, MaterialDATA &material )
{
	memcpy( material.specular, values, sizeof(float) * 4 );
}
void MaterailDATA_SetEmissive4( const float *values, MaterialDATA &material )
{
	memcpy( material.emission, values, sizeof(float) * 4 );
}

////////////

};