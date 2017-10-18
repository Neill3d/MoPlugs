
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MAIN.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include "fb_shader_particles_shader.h"
#include "model_force_drag.h"
#include "model_force_wind.h"
#include "model_collision_terrain.h"

#include "Shader_ParticleSystem.h"
#include "IO\FileUtils.h"

#define PARTICLES_EFFECT				"Particles.glslfx"

//--- Library declaration
FBLibraryDeclare( gpushader_particles )
{
	FBLibraryRegister( GPUshader_Particles );
	FBLibraryRegister( GPUshader_ParticlesLayout );

	FBLibraryRegisterStorable(ForceWind);
	FBLibraryRegisterElement(ForceWind);

	FBLibraryRegisterStorable(ForceMotor);
	FBLibraryRegisterElement(ForceMotor);

	FBLibraryRegisterStorable(ForceDrag);
	FBLibraryRegisterElement(ForceDrag);

	FBLibraryRegisterStorable(CollisionSphere);
	FBLibraryRegisterElement(CollisionSphere);

	FBLibraryRegisterStorable(CollisionTerrain);
	FBLibraryRegisterElement(CollisionTerrain);

	FBLibraryRegister( KShaderParticlesAssociation );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()	{ 
	
	glewInit();
	GPUshader_Particles::AddPropertiesToPropertyViewManager();
	ForceWind::AddPropertiesToPropertyViewManager();
	ForceDrag::AddPropertiesToPropertyViewManager();
	CollisionTerrain::AddPropertiesToPropertyViewManager();

	//
	FBString effectPath, effectFullName;

	if ( FindEffectLocation( FBString("\\GLSLFX\\", PARTICLES_EFFECT), effectPath, effectFullName ) )
	{
		ParticlesSystem::Shader::SetShaderEffectLocation( effectFullName, effectFullName.GetLen() );

		FBString computeLocation(effectPath, "\\GLSLFX\\Particles_simulation.cs");
		ParticlesSystem::Shader::SetComputeShaderLocation( computeLocation, computeLocation.GetLen() );

		FBString computeSelfCollisionsLocation(effectPath, "\\GLSLFX\\Particles_selfcollisions.cs");
		ParticlesSystem::Shader::SetComputeSelfCollisionsShaderLocation( computeSelfCollisionsLocation, computeSelfCollisionsLocation.GetLen() );

		FBString computeIntegrateLocation(effectPath, "\\GLSLFX\\Particles_integrate.cs");
		ParticlesSystem::Shader::SetComputeIntegrateLocation( computeIntegrateLocation, computeIntegrateLocation.GetLen() );
	}

	return true; 
}
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease(){ return true; }
