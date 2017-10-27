
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: misc_depth.cs
//
//	composition toolkit
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

/*
float LogarithmicDepth()
{
	float d = texture2D(depthTex, gl_TexCoord[0].st).x;
	float C = 1.0;
	float z = (exp(d*log(C*zFar+1.0)) - 1.0)/C;
	return z;
}

float LinearizeDepth()
{
  float n = zNear; // camera z near
  float f = zFar; // camera z far
  float z = texture2D(depthTex, gl_TexCoord[0].st).x;
  z = 2.0 * z - 1.0; // nvidia depth range doesn't need that !
  return (2.0 * n) / (f + n - z * (f - n));	
  
  //float z_b = texture2D(depthBuffTex, vTexCoord).x;
  float z_b = z;
  float z_n = 2.0 * z_b - 1.0;
  float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
  return z_e;
}
*/

void ConvertDepthToZ(in float d, out float dz)
{
	float C = 1.0;
	float z = (exp(d*log(C*cameraFarPlane+1.0)) - 1.0)/C;
	
	float n = cameraNearPlane;
	float f = cameraFarPlane;
	float lz = d;
	
	lz = 2.0 * d - 1.0; 
	dz = (2.0 * n) / (f + n - lz * (f - n));	
}

void ComputeDepth(in float d, out float depth)
{
#ifdef DEPTH_LOG
	float C = 1.0;
	float z = (exp(d*log(C*cameraFarPlane+1.0)) - 1.0)/C;
	
	depth = z / cameraFarPlane;
#else
	float n = cameraNearPlane;
	float f = cameraFarPlane;
	
	#ifdef DEPTH_NV
		depth = (2.0 * n) / (f + n - d * (f - n));
	#else
		float lz = 2.0 * d - 1.0; 
		depth = (2.0 * n) / (f + n - lz * (f - n));
	#endif
#endif
}

void ComputeDepthNV(in float d, out float depth)
{
	float n = cameraNearPlane;
	float f = cameraFarPlane;
	
	depth = (2.0 * n) / (f + n - d * (f - n));
}

void ComputeDepthLog(in float d, out float depth)
{
	float C = 1.0;
	float z = (exp(d*log(C*cameraFarPlane+1.0)) - 1.0)/C;
	
	depth = z / cameraFarPlane;
}