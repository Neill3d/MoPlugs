#ifndef SHADER_CG_H
#define SHADER_CG_H

#include "shared_projectors.h"
#include "Shader_CG_types.h"
#include <memory>

#define kMaxDrawInstancedSize  100

namespace GraphicsCG
{

	struct SwitchShaderOptions
	{
		bool flat;
		bool toon;
		float toonSteps;
		float toonDistribution;
		float toonShadowPosition;
		float brightness;
		float saturation;
		float contrast;
		float customColor[3];
		int mode;

		double transparencyFactor;
		bool inverseTransparency;
	};

	//////////////////////

	struct MaterialVertex
	{
		GLuint		diffuseTexid;
		double		diffuseMatrix[16];

		void SetDiffuseTextureObject(const GLuint id)
		{
			diffuseTexid = id;
		}
		void SetDiffuseMatrix(const double *values)
		{
			memcpy( diffuseMatrix, values, sizeof(double) * 16 );
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shader

	class Shader
	{
	public:

		static Shader *CreateNewShader();

	public:

		Shader();
		virtual ~Shader();

		// Call after contruction.
        virtual bool Initialize(const char* pVertexShaderPath, const char* pPixelShaderPath, const char* pVertexShaderOptions = NULL, const char* pPixelShaderOptions = NULL);

        virtual void BeginShading(const LightSet	&lightData);
        virtual void EndShading();

		//void AssignCompose( CGPUProjectorsCompose *_compose );

        // Will be called when the rendering will change its material state.
		virtual void SwitchProjectors(const ProjectorSet &data, const GLuint *projIds, const GLuint *maskIds);
		virtual void SwitchShader(const ShaderDATA &shader);
		virtual void SwitchMaterial(const bool idBufferRender, const MaterialVertex &vertexData, const MaterialDATA &data);
		virtual void SwitchModel (const bool idBufferRender, const double *matrix, const float uniquecolor[4] );

        virtual void UploadModelMatrix(const double *pModelMatrix);
		// Upload ModelView Matrix Array for Draw Instanced.
        virtual void UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount);
	};

}

#endif // SHADER_CG_H
