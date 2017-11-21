
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: fb_shader_particles_shader.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef ORSDK_DLL
	/** \def ORSDK_DLL
	*	Be sure that ORSDK_DLL is defined only once...
	*/
	#define ORSDK_DLL K_DLLEXPORT
#endif

// Class declaration
#include "fb_shader_particles_shader.h"
#include "GL\glew.h"

#include <math.h>
#include <Windows.h>

#include <limits.h>

#include "algorithm\math3d.h"
#include "algorithm\math3d_mobu.h"
#include "graphics\checkglerror.h"

#include "model_force_drag.h"
#include "model_force_motor.h"
#include "model_force_wind.h"

#include "model_collision_sphere.h"
#include "model_collision_terrain.h"

#include "mographics_common.h"

//--- Registration defines
#define ORSHADER_TEMPLATE__CLASS		ORSHADER_TEMPLATE__CLASSNAME
#define ORSHADER_TEMPLATE__DESC			"GPU Shader Particles"

//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	ORSHADER_TEMPLATE__CLASS	);
FBRegisterShader	(	ORSHADER_TEMPLATE__DESCSTR,
						ORSHADER_TEMPLATE__CLASS,
						ORSHADER_TEMPLATE__DESCSTR,
						ORSHADER_TEMPLATE__DESC,
						FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)
/*
static long long GetCurrentTimeMillis()
{
	return GetTickCount();
}
*/

using namespace ParticlesSystem;


#ifdef _DEBUG
static void GPUshader_Particles_ReloadShader(HIObject pObject, bool value) {
	GPUshader_Particles *p = FBCast<GPUshader_Particles>(pObject);
	if (value && p)	p->DoReloadShader();
}
#endif

static void GPUshader_Particles_About(HIObject pObject, bool value) {
	GPUshader_Particles *p = FBCast<GPUshader_Particles>(pObject);
	if (value && p) {
		FBMessageBox( "Particle System Beta", 
			"Author Sergey Solokhin (Neill3d) 2016\n"
			"\t e-mail to s@neill3d.com"
			"\t\t www.neill3d.com", 
			"Ok" );
	}
}

static void GPUshader_Particles_ResetAction(HIObject pObject, bool value) {
	GPUshader_Particles *p = FBCast<GPUshader_Particles>(pObject);
	if (value && p)	p->DoReset();
}

static void GPUshader_Particles_ColorCurve(HIObject pObject, bool value) {
	GPUshader_Particles *p = FBCast<GPUshader_Particles>(pObject);
	if (value && p)	p->DoColorCurve();
}

static void GPUshader_Particles_SizeCurve(HIObject pObject, bool value) {
	GPUshader_Particles *p = FBCast<GPUshader_Particles>(pObject);
	if (value && p)	p->DoSizeCurve();
}


/************************************************
 *	Specific Constructor. Construct custom shader from an FBMaterial object.
 ************************************************/
GPUshader_Particles::GPUshader_Particles(FBMaterial *pMaterial)
	: FBShader(pMaterial->Name)
{
    FBClassInit;
    mMaterial = pMaterial;

	//RenderingPass = GetRenderingPassNeededForAlpha( kFBAlphaSourceTransluscentZSortAlpha );
	RenderingPass = GetRenderingPassNeededForAlpha( kFBAlphaSourceNoAlpha );
}

void AddPropertyViewForParticles(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(ORSHADER_TEMPLATE__CLASSSTR, pPropertyName, pHierarchy);
}
 
void GPUshader_Particles::AddPropertiesToPropertyViewManager()
{
	//Stiffness  
#ifdef _DEBUG
	AddPropertyViewForParticles("Reload Shader", "");
#endif
	
	AddPropertyViewForParticles("About", "");

	AddPropertyViewForParticles("Displayed Count", "");
	AddPropertyViewForParticles("Enable", "");
	AddPropertyViewForParticles("Play Mode", "");
	AddPropertyViewForParticles("Emitter", "");
	AddPropertyViewForParticles("Inherit Emitter Colors", "");

	//
	AddPropertyViewForParticles("Evaluation parameters", "", true);
	AddPropertyViewForParticles("Iterations", "Evaluation parameters");
	AddPropertyViewForParticles("Delta Time Limit", "Evaluation parameters");
	AddPropertyViewForParticles("Adaptive SubSteps", "Evaluation parameters");
	AddPropertyViewForParticles("SubSteps", "Evaluation parameters");

	// folder Particle generation
	AddPropertyViewForParticles("Reset", "");
	AddPropertyViewForParticles("Reset Time", "");
	AddPropertyViewForParticles("Reset Quantity", "");

	AddPropertyViewForParticles("Particle generation", "", true);
	AddPropertyViewForParticles("Use Rate", "Particle generation");
	AddPropertyViewForParticles("Particle Rate", "Particle generation");
	AddPropertyViewForParticles("Use PreGenerated Particles", "Particle generation");

	AddPropertyViewForParticles("Maximum Particles", "Particle generation");

	AddPropertyViewForParticles("Use Custom Range", "Particle generation");
	AddPropertyViewForParticles("Emit Start", "Particle generation");
	AddPropertyViewForParticles("Emit Stop", "Particle generation");
	AddPropertyViewForParticles("Life Time", "Particle generation");
	AddPropertyViewForParticles("Life Time Variation", "Particle generation");

	//
	AddPropertyViewForParticles("Particle generation.Start Direction", "Particle generation", true);
	AddPropertyViewForParticles("Emit Direction", "Particle generation.Start Direction");
	AddPropertyViewForParticles("Emit Dir Random", "Particle generation.Start Direction");
	AddPropertyViewForParticles("Use Emitter Normals As Dir", "Particle generation.Start Direction");
	
	AddPropertyViewForParticles("Particle generation.Start Velocity", "Particle generation", true);
	AddPropertyViewForParticles("Emit Velocity", "Particle generation.Start Velocity");
	AddPropertyViewForParticles("Emit Vel Random", "Particle generation.Start Velocity");
	AddPropertyViewForParticles("Inherit Emitter Velocity", "Particle generation.Start Velocity");

	//
	AddPropertyViewForParticles("Dynamic parameters", "", true );
	AddPropertyViewForParticles("Mass", "Dynamic parameters" );
	AddPropertyViewForParticles("Damping", "Dynamic parameters" );
	AddPropertyViewForParticles("Use Gravity", "Dynamic parameters" );
	AddPropertyViewForParticles("Gravity", "Dynamic parameters" );
	AddPropertyViewForParticles("Use Forces", "Dynamic parameters" );
	AddPropertyViewForParticles("Force Objects", "Dynamic parameters" );
	AddPropertyViewForParticles("Use Turbulence", "Dynamic parameters" );
	AddPropertyViewForParticles("Noise Frequency", "Dynamic parameters" );
	AddPropertyViewForParticles("Noise Speed", "Dynamic parameters" );
	AddPropertyViewForParticles("Noise Amplitude", "Dynamic parameters" );
	AddPropertyViewForParticles("Use Collisions", "Dynamic parameters" );
	AddPropertyViewForParticles("Collision Objects", "Dynamic parameters" );
	AddPropertyViewForParticles("Use Floor", "Dynamic parameters" );
	AddPropertyViewForParticles("Floor Friction", "Dynamic parameters" );
	AddPropertyViewForParticles("Floor Level", "Dynamic parameters" );

	//
	AddPropertyViewForParticles("Particle visualization", "", true);	

	//
	AddPropertyViewForParticles("Particle visualization.Particle Shape", "Particle visualization", true);
	AddPropertyViewForParticles("Primitive Type", "Particle visualization.Particle Shape");
	AddPropertyViewForParticles("Instance Object", "Particle visualization.Particle Shape");

	AddPropertyViewForParticles("Texture Object", "Particle visualization");
	AddPropertyViewForParticles("Shade Mode", "Particle visualization");
	AddPropertyViewForParticles("Affecting Lights", "Particle visualization");
	AddPropertyViewForParticles("Transparency", "Particle visualization");
	AddPropertyViewForParticles("Transparency Factor", "Particle visualization");
	
	AddPropertyViewForParticles("Particle visualization.Particle Color", "Particle visualization", true);
	AddPropertyViewForParticles("Color", "Particle visualization.Particle Color");
	AddPropertyViewForParticles("Color Variation (%)", "Particle visualization.Particle Color");
	AddPropertyViewForParticles("Use Color Curve", "Particle visualization.Particle Color");
	AddPropertyViewForParticles("Color Curve", "Particle visualization.Particle Color");

	AddPropertyViewForParticles("Particle visualization.Particle Size", "Particle visualization", true);
	AddPropertyViewForParticles("Size", "Particle visualization.Particle Size");
	AddPropertyViewForParticles("Size Variation (%)", "Particle visualization.Particle Size");
	
	AddPropertyViewForParticles("Min Point Scale", "Particle visualization.Particle Size");
	AddPropertyViewForParticles("Max Point Scale", "Particle visualization.Particle Size");
	AddPropertyViewForParticles("Point Scale Distance", "Particle visualization.Particle Size");

	AddPropertyViewForParticles("Use Size Curve", "Particle visualization.Particle Size");
	AddPropertyViewForParticles("Size Curve", "Particle visualization.Particle Size");
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool GPUshader_Particles::FBCreate()
{
    mRenderFrameId = 0;
	//mLastFrameTime = 0.0;

	mLastTimelineTime = FBTime::Infinity;
	mLastRenderFrameId = -1;
	//mIsFirst = true;

	firstBeginRender = false;
	firstShadeModel = false;

	mLastResetState = false;

	//
	// Particle generation
	//
#ifdef _DEBUG
	FBPropertyPublish( this, ReloadShader, "Reload Shader", nullptr, GPUshader_Particles_ReloadShader );
#endif

	FBPropertyPublish( this, About, "About", nullptr, GPUshader_Particles_About );

	FBPropertyPublish( this, DisplayedCount, "Displayed Count", nullptr, nullptr );
	FBPropertyPublish( this, MaximumParticles, "Maximum Particles", nullptr, nullptr );
	
	FBPropertyPublish( this, UseRate, "Use Rate", nullptr, nullptr );
	FBPropertyPublish( this, ParticleRate, "Particle Rate", nullptr, nullptr );
	FBPropertyPublish( this, UsePreGeneratedParticles, "Use PreGenerated Particles", nullptr, nullptr );

	FBPropertyPublish( this, ConstraintMagnitude, "Constraint Magnitude", nullptr, nullptr );

	FBPropertyPublish( this, Emitter, "Emitter", nullptr, SetEmitterProperty );
	FBPropertyPublish( this, InheritEmitterColors, "Inherit Emitter Colors", nullptr, nullptr );
	FBPropertyPublish( this, PlayMode, "Play Mode", nullptr, nullptr );

	FBPropertyPublish( this, RandomSeed, "Random Seed", nullptr, nullptr );

	FBPropertyPublish( this, Reset, "Reset", nullptr, GPUshader_Particles_ResetAction );
	FBPropertyPublish( this, ResetTime, "Reset Time", nullptr, nullptr );
	FBPropertyPublish( this, ResetCount, "Reset Quantity", nullptr, nullptr );

	FBPropertyPublish( this, ExtrudeResetPosition, "Extrude Reset Position", nullptr, nullptr );

	FBPropertyPublish( this, EmitterDirection, "Emit Direction", nullptr, nullptr );
	FBPropertyPublish( this, EmitterDirRandom, "Emit Dir Random", nullptr, nullptr );
	FBPropertyPublish( this, UseEmitterNormals, "Use Emitter Normals As Dir", nullptr, nullptr );

	FBPropertyPublish( this, EmitterVelocity, "Emit Velocity", nullptr, nullptr );
	FBPropertyPublish( this, InheritEmitterVelocity, "Inherit Emitter Velocity", nullptr, nullptr );
	FBPropertyPublish( this, EmitterVelRandom, "Emit Vel Random", nullptr, nullptr );

	FBPropertyPublish( this, Mass, "Mass", nullptr, nullptr );
	FBPropertyPublish( this, Damping, "Damping", nullptr, nullptr );
	FBPropertyPublish( this, UseGravity, "Use Gravity", nullptr, nullptr );
	FBPropertyPublish( this, Gravity, "Gravity", nullptr, nullptr );
	FBPropertyPublish( this, UseForces, "Use Forces", nullptr, nullptr );
	FBPropertyPublish( this, Forces, "Forces", nullptr, nullptr );
	FBPropertyPublish( this, UseCollisions, "Use Collisions", nullptr, nullptr );
	FBPropertyPublish( this, Collisions, "Collisions", nullptr, nullptr );
	FBPropertyPublish( this, SelfCollisions, "Self Collisions", nullptr, nullptr );

	FBPropertyPublish( this, UseTurbulence, "Use Turbulence", nullptr, nullptr );
	FBPropertyPublish( this, NoiseFrequency, "Noise Frequency", nullptr, nullptr );
	FBPropertyPublish( this, NoiseSpeed, "Noise Speed", nullptr, nullptr );
	FBPropertyPublish( this, NoiseAmplitude, "Noise Amplitude", nullptr, nullptr );

	FBPropertyPublish( this, UseFloor, "Use Floor", nullptr, nullptr );
	FBPropertyPublish( this, FloorFriction, "Floor Friction", nullptr, nullptr );
	FBPropertyPublish( this, FloorLevel, "Floor Level", nullptr, nullptr );

	FBPropertyPublish( this, Iterations, "Iterations", nullptr, nullptr );
	FBPropertyPublish( this, DeltaTimeLimit, "Delta Time Limit", nullptr, nullptr );
	FBPropertyPublish( this, AdaptiveSubSteps, "Adaptive SubSteps", nullptr, nullptr );
	FBPropertyPublish( this, SubSteps, "SubSteps", nullptr, nullptr );

	FBPropertyPublish( this, UseCustomRange, "Use Custom Range", nullptr, nullptr );
	FBPropertyPublish( this, EmitStart, "Emit Start", nullptr, nullptr );
	FBPropertyPublish( this, EmitStop, "Emit Stop", nullptr, nullptr );

	FBPropertyPublish( this, LifeTime, "Life Time", nullptr, nullptr );
	FBPropertyPublish( this, LifeTimeVariation, "Life Time Variation", nullptr, nullptr );

	DisplayedCount = 0;
	DisplayedCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	
	MaximumParticles = 1024*1024;
	ResetCount = 0;			// total amount of particles in a frame
	ResetCount.SetMinMax(0.0, MaximumParticles, true, false);

	ExtrudeResetPosition = 0.0;

	UseRate = true;
	ParticleRate = 100;			// particles per second
	ParticleRate.SetMinMax(0.0, 10000.0, true, false);
	UsePreGeneratedParticles = true;

	RandomSeed = 12345;

	InheritEmitterColors = true;
	Emitter = kFBParticleEmitterVertices;
	PlayMode = kFBParticleLife;

	ResetTime = FBTime(0);

	EmitterDirection = FBVector3d(0.0, 1.0, 0.0);
	EmitterDirRandom = FBVector3d(10.0, 10.0, 10.0);
	EmitterVelocity = FBVector3d(10.0, 10.0, 10.0);
	EmitterVelRandom = FBVector3d(1.0, 1.0, 1.0);
	InheritEmitterVelocity = true;
	UseEmitterNormals = true;

	Mass = 100.0;		// result is 0.01
	Damping = 99.0;		// result is * 0.01
	UseGravity = true;
	Gravity = FBVector3d(0.0, -9.8, 0.0);
	UseForces = true;
	UseCollisions = true;

	Forces.SetFilter(FBModelMarker::GetInternalClassId() );
	Forces.SetSingleConnect(false);

	Collisions.SetFilter(FBModelMarker::GetInternalClassId() );
	Collisions.SetSingleConnect(false);

	SelfCollisions = false;

	ConstraintMagnitude = 0.0;

	UseTurbulence = true;
	NoiseFrequency = 100.0;	// result is 0.01
	NoiseSpeed = 100.0;
	NoiseAmplitude = 100.0;

	UseFloor = true;
	FloorFriction = 50.0;	// result is * 0.01
	FloorLevel = 0.0;

	Iterations = 30;		// in seconds
	Iterations.SetMinMax(1.0, 240.0, true, true);
	
	DeltaTimeLimit = 2.0;	// in seconds
	DeltaTimeLimit.SetMinMax(0.1, 1000.0, true, true);
	AdaptiveSubSteps = false;
	SubSteps = 1;

	UseCustomRange = true;
	EmitStart = FBTime(0);
	EmitStop = FBTime(0,0,0,30);

	LifeTime = 10.0;			// in seconds
	LifeTimeVariation = 10.0;	// in percent

	//
	// render properties
	//
	FBPropertyPublish( this, PointSmooth, "Point Smooth", nullptr, nullptr );
	FBPropertyPublish( this, PrimitiveType, "Primitive Type", nullptr, nullptr );
	FBPropertyPublish( this, InstanceObject, "Instance Object", nullptr, nullptr );
	FBPropertyPublish( this, TextureObject, "Texture Object", nullptr, nullptr );

	FBPropertyPublish( this, ShadeMode, "Shade Mode", nullptr, nullptr );
	ShadeMode = kFBParticleShadeSimple;
	FBPropertyPublish(	this, AffectingLights,	"Affecting Lights",		NULL, NULL);
    AffectingLights.SetFilter(FBLight::GetInternalClassId());
    AffectingLights.SetSingleConnect(false);

    FBPropertyPublish(	this, Transparency,	"Transparency",		NULL, SetTransparencyProperty);
    Transparency = kFBAlphaSourceNoAlpha;
    //RenderingPass = GetRenderingPassNeededForAlpha(Transparency);

	//SetShaderCapacity(kFBShaderCapacityMaterialEffect, true);

    FBPropertyPublish(	this, TransparencyFactor,	"Transparency Factor",NULL, NULL);
    TransparencyFactor.SetMinMax(0.0, 100.0, true, true);
    TransparencyFactor = 100.0;

	FBPropertyPublish( this, Color, "Color", nullptr, nullptr );
	FBPropertyPublish( this, ColorVariation, "Color Variation (%)", nullptr, nullptr );
	FBPropertyPublish( this, UseColorCurve, "Use Color Curve", nullptr, nullptr );
	FBPropertyPublish( this, ColorCurve, "Color Curve", nullptr, GPUshader_Particles_ColorCurve );
	FBPropertyPublish( this, ColorCurveHolder, "ColorCurveHolder", nullptr, nullptr );

	FBPropertyPublish( this, Size, "Size", nullptr, nullptr );
	FBPropertyPublish( this, SizeVariation, "Size Variation (%)", nullptr, nullptr );
	
	FBPropertyPublish( this, MinPointScale, "Min Point Scale", nullptr, nullptr );
	FBPropertyPublish( this, MaxPointScale, "Max Point Scale", nullptr, nullptr );
	FBPropertyPublish( this, PointScaleDistance, "Point Scale Distance", nullptr, nullptr );

	FBPropertyPublish( this, UseSizeCurve, "Use Size Curve", nullptr, nullptr );
	FBPropertyPublish( this, SizeCurve, "Size Curve", nullptr, GPUshader_Particles_SizeCurve );
	FBPropertyPublish( this, SizeCurveHolder, "SizeCurveHolder", nullptr, nullptr );

	PointSmooth = true;
	PrimitiveType = kFBParticlePoints;
	InstanceObject.SetFilter(FBModel::GetInternalClassId() );
	InstanceObject.SetSingleConnect(true);

	TextureObject.SetFilter(FBTexture::GetInternalClassId() );
	TextureObject.SetSingleConnect(true);

	Color = FBColorAndAlpha(1.0, 1.0, 1.0, 1.0);
	ColorVariation = 0.0;
	ColorVariation.SetMinMax(0.0, 100.0, true, true);
	UseColorCurve = false;
	ColorCurveHolder = FBColorAndAlpha(1.0, 1.0, 1.0, 1.0);
	ColorCurveHolder.SetAnimated(true);
	FBTime time(0);
	ColorCurveHolder.KeyAt( time );
	time.SetSecondDouble(1.0);
	ColorCurveHolder.SetCandidate( FBColorAndAlpha(0.0, 0.0, 0.0, 1.0), sizeof(FBColorAndAlpha) );
	ColorCurveHolder.KeyAt( time );
	ColorCurveHolder.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );

	Size = 10.0;
	SizeVariation = 10.0;
	SizeVariation.SetMinMax(0.0, 100.0, true, true);

	MinPointScale = 10.0;
	MaxPointScale = 70.0;
	PointScaleDistance = 1000.0;
	PointScaleDistance.SetMinMax(1.0, 4000.0, true, false);

	UseSizeCurve = false;
	SizeCurveHolder = 5.0;
	SizeCurveHolder.SetAnimated(true);
	SizeCurveHolder.KeyAt( FBTime(0) );
	double value = 55.0;
	SizeCurveHolder.SetCandidate( &value, sizeof(double) );
	SizeCurveHolder.KeyAt( time );
	SizeCurveHolder.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );

	//
	// init gpu particle system
	//
	SetUpPropertyTextures( &SizeCurveHolder, &ColorCurveHolder );
	
	
	CHECK_GL_ERROR();

	mNeedUpdatePropertyTexture = true;
	mNeedReloadShaders = false;

	mUseSizeCurve = false;
	mUseColorCurve = false;

	UpdatePropertyTextures();

	return true;
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void GPUshader_Particles::FBDestroy()
{
	FreeParticles();
}

/************************************************
 *	Shader functions.
 ************************************************/

/** Create a new shader-model information object.
*	\param	pModelRenderInfo 	Internal Model Render Info.
*   \param  pSubRegionIndex
*	\return	Newly allocated shader-model information object.
*/

FBShaderModelInfo* GPUshader_Particles::NewShaderModelInfo(HKModelRenderInfo pModelRenderInfo, int pSubRegionIndex)
{
	FBShaderModelInfo *pInfo = ParentClass::NewShaderModelInfo(pModelRenderInfo, pSubRegionIndex);
    
	// Sinde Shader need access deformed vertex array directly on CPU memory, we need to request
	FBModel *pModel = pInfo->GetFBModel();
	pModel->NoFrustumCullingRequire();
	if (pModel->IsDeformable)
	{
		FBModelVertexData *pModelVertexData = pModel->ModelVertexData;
		//pModelVertexData->VertexArrayMappingRequest();
	}
	
	CHECK_GL_ERROR();

	return pInfo;
}

/** Update shader-model information when model, material & texture mapping or shader setting change.
*	\param	pOptions	Render options.
*	\param	pInfo		Shader-model information object to be updated.
*/

void GPUshader_Particles::UpdateModelShaderInfo( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	if (pInfo)
	{
		pInfo->UpdateModelShaderInfo(GetShaderVersion());

		unsigned int lVBOFormat = kFBGeometryArrayID_Point | kFBGeometryArrayID_Normal;
		pInfo->SetGeometryArrayIds(lVBOFormat);
	}

	CHECK_GL_ERROR();
}

void GPUshader_Particles::FreeParticles()
{
	// remove particle data from the map
	
	for (auto iter=begin(mParticleMap); iter!=end(mParticleMap); ++iter)
	{
		ParticlesSystem::ParticleSystem *pParticles = iter->second;
		delete pParticles;
	}

	mParticleMap.clear();
}

/** Destroy shader-model information object callback.
*	\param	pOptions	Render options.
*	\param	pInfo		Shader-model information object to destroy.
*/

void GPUshader_Particles::DestroyShaderModelInfo( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	FBModel *pModel = pInfo->GetFBModel();
	pModel->NoFrustumCullingRelease();
	// release vertex array mapping request
	if (pModel->IsDeformable)
	{
		FBModelVertexData *pModelVertexData = pModel->ModelVertexData;
		//pModelVertexData->VertexArrayMappingRelease();
	}

	// remove particle data from the map
	auto iter = mParticleMap.find(pModel);
	if (iter!=end(mParticleMap) )
	{
		ParticlesSystem::ParticleSystem *pParticles = iter->second;
		delete pParticles;
		mParticleMap.erase(iter);
	}
	/*
	auto lastFrameTimeIter = mLastFrameTimeMap.find(pModel);
	if (lastFrameTimeIter != end(mLastFrameTimeMap) )
	{
		mLastFrameTimeMap.erase(lastFrameTimeIter);
	}
	*/

	ParentClass::DestroyShaderModelInfo(pOptions, pInfo);
}

void GPUshader_Particles::DoReloadShader()
{
	mNeedReloadShaders = true;
	
	for (auto iter=begin(mParticleMap); iter!=end(mParticleMap); ++iter)
	{
		iter->second->ReloadShaders();
	}
}

void GPUshader_Particles::DoReset()
{
	// TODO: add some functionality here
	//mParticleSystem.UploadSimulationData( MaximumParticles, ParticleCount, UseParticleRate, ParticleRate, 0.0f, LifeTime, LifeTimeVariation, FixPosition, true );

	for (auto iter=begin(mParticleMap); iter!=end(mParticleMap); ++iter)
	{
		iter->second->NeedReset();
	}
}

void GPUshader_Particles::DoColorCurve()
{
	ORPopup_ColorEditor	ColorCurveEditor;

	ColorCurveEditor.Caption = FBString( Name, " - Color curve editor" );
	ColorCurveEditor.Modal = true;
	ColorCurveEditor.SetUp( this, &ColorCurveHolder );
	ColorCurveEditor.Show();

	mNeedUpdatePropertyTexture = true;
	//mParticleSystem.UpdatePropertyTextures();
}

void GPUshader_Particles::DoSizeCurve()
{
	ORPopup_CurveEditor	SizeCurveEditor;

	SizeCurveEditor.Caption = FBString( Name, " - Size curve editor" );
	SizeCurveEditor.Modal = true;
	SizeCurveEditor.SetUp( nullptr, &SizeCurveHolder );
	SizeCurveEditor.Show();

	mNeedUpdatePropertyTexture = true;
	//mParticleSystem.UpdatePropertyTextures();
}

void GPUshader_Particles::DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo )
{
	/*
	FBModel *pModel = pInfo->GetFBModel();

	auto iter = mParticleMap.find(pModel);
	if (iter != end(mParticleMap) )
	{
		iter->second->ChangeDisplayContext();
		iter->second->mPerModelUserData.lastFrameTime = 0.0;
		iter->second->mPerModelUserData.isResetDone = false;
		iter->second->mPerModelUserData.isFirst = true;
	}
	*/

	FreeParticles();

	mLastTimelineTime = FBTime::Infinity;

	/*
	auto lastFrameTimeIter = mLastFrameTimeMap.find(pModel);
	if (lastFrameTimeIter != end(mLastFrameTimeMap) )
		lastFrameTimeIter->second = 0.0;

	auto resetDoneIter = mIsResetDoneMap.find(pModel);
	if (resetDoneIter != end(mIsResetDoneMap) )
		iter->second = false;
		*/

	CHECK_GL_ERROR();
	//mNeedReloadShaders = true;
}

bool GPUshader_Particles::ShaderNeedBeginRender()
{
	//CHECK_GL_ERROR();

	bool resetValue;
	Reset.GetData(&resetValue, sizeof(bool) );

	if (true == resetValue && mLastResetState != resetValue)
	{
		DoReset();
	}
	mLastResetState = resetValue;

	// TODO: this one is connected to specified FBShader, should be updated only once per instance
#ifdef _DEBUG
	printf("[ShaderNeedBeginRender] Begin...\n" );
#endif
	//
	// render terrain collision depth
	UpdateConnectedTerrain();

	//
	UpdateConnectedData();

	//
	if (mNeedUpdatePropertyTexture)
	{
		UpdatePropertyTextures();
		mNeedUpdatePropertyTexture = false;
	}

	if (PrimitiveType == kFBParticleInstance)
		UpdateInstanceData();

#ifdef _DEBUG
	CHECK_GL_ERROR();

	printf("[ShaderNeedBeginRender] END...\n" );
#endif
	firstBeginRender = true;
	firstShadeModel = true;

    return true;
}

static bool shaderBegin = false;

void GPUshader_Particles::ShaderBeginRender( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo )
{
#ifdef _DEBUG
	printf("[ShaderBeginRender] Begin...\n" );
#endif
	if (false == firstBeginRender)
		return;

	if (nullptr != pRenderOptions)
	{
		const int renderFrameId = pRenderOptions->GetRenderFrameId();

		if (renderFrameId < mLastRenderFrameId)
			return;

		mLastRenderFrameId = renderFrameId;
	}
	

	firstBeginRender = false;
	shaderBegin = true;

	//bool pregeneratedEmit = UsePreGeneratedParticles;

	//
	if (mNeedReloadShaders)
	{
		auto iter = begin(mParticleMap);
		if (iter != end(mParticleMap) )
		{
			iter->second->ReloadShaders();
		}

		//FreeParticles();
	}

	for (int i=0, count=GetDstCount(); i<count; ++i)
	{
		FBPlug *pPlug = GetDst(i);
		if (FBIS(pPlug, FBModel))
		{
			LocalShaderBeginRender( pRenderOptions, (FBModel*) pPlug );
		}

	}
}


void GPUshader_Particles::ShadeModel( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass )
{
#ifdef _DEBUG
	printf("[ShadeModel] Begin...\n" );
#endif
	FBViewingOptions* lViewingOptions = pRenderOptions->GetViewerOptions();
	bool lIsSelectBufferPicking = (pRenderOptions->IsIDBufferRendering() || lViewingOptions->IsInSelectionBufferPicking());
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	
	if (lIsSelectBufferPicking || lIsColorBufferPicking)
		return;
	
	if (false == firstShadeModel)
		return;
		
	firstShadeModel = false;

	for (int i=0, count=GetDstCount(); i<count; ++i)
	{
		FBPlug *pPlug = GetDst(i);

		if (FBIS(pPlug, FBModel))
		{
			LocalShadeModel(pRenderOptions, (FBModel*) pPlug, pPass );
		}
	}

#ifdef _DEBUG
	if (shaderBegin == false)
		printf("error\n" );
#endif
	shaderBegin = false;

	mNeedReloadShaders = false;
	
	if( pShaderModelInfo == nullptr )
		return;	
}

void GPUshader_Particles::LocalShaderBeginRender( FBRenderOptions* pRenderOptions, FBModel *pModel )
{
#ifdef _DEBUG
	printf("[ShaderBeginRender] Begin...\n" );
#endif
	shaderBegin = true;

	bool pregeneratedEmit = UsePreGeneratedParticles;

	auto particleIter = mParticleMap.find(pModel);

	if (particleIter == end(mParticleMap) )
	{
		// allocate new structure for a new model assigned
		ParticlesSystem::ParticleSystem	*newParticleSystem = new ParticlesSystem::ParticleSystem();
		newParticleSystem->SetConnections(&mParticleConnections);
		bool res = newParticleSystem->InitParticleSystem( vec3(0.0f, 0.0f, 0.0f) );
		
		if (res == false)
		{
			delete newParticleSystem;
			newParticleSystem = nullptr;

			Enable = false;
			return;
		}
		else
		{
			mParticleMap[pModel] = newParticleSystem;
			particleIter = mParticleMap.find(pModel);
		}
	} 
	/*
	if (mNeedReloadShaders)
	{
		bool status = particleIter->second->ReloadShaders();

		if (status == false)
		{
			Enable = false;
			return;
		}

		CHECK_GL_ERROR();

		// TODO: this will update only first particle system (will work cause shader is a singleton) !!
		mNeedReloadShaders = false;
	}
	*/
	auto pParticles = particleIter->second;

	//
	// do we need to update surface data ?! - update once per model
	FBTime currTimelineTime = FBGetDisplayInfo()->GetLocalTime(); // mSystem.LocalTime;
	int lFrame = currTimelineTime.GetFrame();

	if (Emitter != kFBParticleEmitterVolume)
	{
		if (pregeneratedEmit)
		{
			if (mLastTimelineTime == FBTime::Infinity 
				|| (currTimelineTime == ResetTime && currTimelineTime != mLastTimelineTime))
			{
				UpdateEmitterGeometryBufferOnGPU(pModel, pParticles);
			}
		}
		else
		if (mLastTimelineTime == FBTime::Infinity || currTimelineTime != mLastTimelineTime)
		{
			UpdateEmitterGeometryBufferOnGPU(pModel, pParticles);
		}
	}

	//
	bool enableEmit = true;
	if (PlayMode == kFBParticlePlay && UseCustomRange)
	{
		FBTime emitStartTime = EmitStart;
		FBTime emitStopTime = EmitStop;

		if (lFrame < emitStartTime.GetFrame() || lFrame > emitStopTime.GetFrame() )
			enableEmit = false;
	}

	UpdateEvaluationData(pModel, pParticles, enableEmit);

	//
	// try to emit

	FBTime localTime = (PlayMode == kFBParticleLife) ? mSystem.SystemTime : mSystem.LocalTime;

	if (PlayMode == kFBParticlePlay || pRenderOptions->IsOfflineRendering())
	{
		localTime = FBGetDisplayInfo()->GetLocalTime();
	}
	else
	{
		localTime = FBGetDisplayInfo()->GetSystemTime();
	}

	lFrame = localTime.GetFrame();
	double timeNow = localTime.GetSecondDouble();

	double lastFrameTime = pParticles->mPerModelUserData.lastFrameTime;

	double deltaTime = timeNow - lastFrameTime;
	double deltaTimeLimit = DeltaTimeLimit;

	if (mLastTimelineTime == FBTime::Infinity || deltaTime < 0.0 || deltaTime > deltaTimeLimit)
	{
		lastFrameTime = timeNow;
		deltaTime = 0.0;

		pParticles->mPerModelUserData.lastFrameTime = lastFrameTime;
	}

	static int totalCycles = 0;
	

	// DONE: this function executed all the time !!!
	bool isResetDone = pParticles->mPerModelUserData.isResetDone;

	if (localTime != ResetTime)
	{
		isResetDone = false;
		pParticles->mPerModelUserData.isResetDone = isResetDone;
	}
	else
	if (isResetDone == false && PlayMode == kFBParticlePlay && localTime == ResetTime) {
		pParticles->NeedReset();

		lastFrameTime = timeNow;
		deltaTime = 0.0;
		pParticles->mPerModelUserData.lastFrameTime = lastFrameTime;

		FBVector3d pos;
		pModel->GetVector(pos);
		pParticles->SetLastEmitterPos(pos);

		totalCycles = 0;

		pParticles->mPerModelUserData.isFirst = true;

		isResetDone = true;
		pParticles->mPerModelUserData.isResetDone = isResetDone;
	}
	
	//if (pParticles->IsNeedReset() )
	//{
		//
		// upload buffers with forces and collisions
		
		UploadConnectedDataToGPU(pParticles);
	//}


	const double timeStep = (Iterations >= 0) ? 1.0 / Iterations : 0.02;

	if (pParticles->mPerModelUserData.isFirst || deltaTime > timeStep)
	{
		CHECK_GL_ERROR();

		// do emit only once per frame
		//if (isFirst)
		//{

		FBColorAndAlpha dColor = Color;
		vec4 fcolor((float)dColor[0], (float)dColor[1], (float)dColor[2], (float)dColor[3]);

		pParticles->SetParticleSize( Size, 0.01 * SizeVariation );
		pParticles->SetParticleColor( InheritEmitterColors, fcolor, 0.01 * ColorVariation );
		pParticles->PrepareParticles(MaximumParticles, RandomSeed, ResetCount, UseRate, ParticleRate, ExtrudeResetPosition);
			pParticles->UploadSimulationDataOnGPU();

			ETechEmitType	emitType = eTechEmitPreGenerated;
			if (pregeneratedEmit)
			{
				emitType = eTechEmitPreGenerated;
			}
			else
			{
				FBParticleEmitter	fbEmitter = Emitter;
				switch(fbEmitter)
				{
				case kFBParticleEmitterSurface:
					emitType = eTechEmitSurface;
					break;
				case kFBParticleEmitterVolume:
					emitType = eTechEmitVolume;
					break;
				case kFBParticleEmitterVertices:
					emitType = eTechEmitVertices;
					break;
				}
			}

			pParticles->EmitParticles(deltaTime, emitType);
		//}
		
		pParticles->mPerModelUserData.isFirst = false;

		const GLuint lSizeTexId = mSizeCurve.GetTextureId();
		if (UseSizeCurve && lSizeTexId > 0)
		{
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_1D, lSizeTexId );
			glActiveTexture(GL_TEXTURE0);
		}

		// run simulation
		unsigned int Cycles = 0;
		Cycles = pParticles->SimulateParticles( timeStep, deltaTime, deltaTimeLimit, SubSteps, SelfCollisions );

		totalCycles += (int) Cycles;

		// DONE: make it unique per model !!
		//mLastFrameTime = timeNow - deltaTime;
		lastFrameTime = pParticles->mPerModelUserData.lastFrameTime;
		lastFrameTime += Cycles * timeStep;
		/*
		if (currTimelineTime != mLastTimelineTime)
		{
			printf( "cycles for %d frame - %d, last frametime - %.2lf \n", lFrame, (int)Cycles, lastFrameTime );
		}
		*/
		if (UseSizeCurve && lSizeTexId > 0)
		{
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_1D, 0);
			glActiveTexture(GL_TEXTURE0);
		}

		pParticles->mPerModelUserData.lastFrameTime = lastFrameTime;
	}

	CHECK_GL_ERROR();
#ifdef _DEBUG
	printf("[ShaderBeginRender] END...\n" );
#endif
}

void GPUshader_Particles::LocalShadeModel( FBRenderOptions* pRenderOptions, FBModel *pModel, FBRenderingPass pPass )
{
#ifdef _DEBUG
	printf("[ShadeModel] Begin...\n" );
#endif
	FBViewingOptions* lViewingOptions = pRenderOptions->GetViewerOptions();
	bool lIsSelectBufferPicking = (pRenderOptions->IsIDBufferRendering() || lViewingOptions->IsInSelectionBufferPicking());
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	

	if (lIsSelectBufferPicking || lIsColorBufferPicking)
		return;
#ifdef _DEBUG
	if (shaderBegin == false)
		printf("error\n" );
#endif
	shaderBegin = false;

	mNeedReloadShaders = false;
	
	auto particleIter = mParticleMap.find(pModel);
	if (particleIter == end(mParticleMap) )
		return;

	auto pParticles = particleIter->second;

	FBTime currTimelineTime = mSystem.LocalTime;
	mLastTimelineTime = currTimelineTime;
	
	//FBTime localTime = (PlayMode == kFBParticleLife) ? mSystem.SystemTime : mSystem.LocalTime;
	//mLastFrameTime = localTime.GetSecondDouble();

	ParticlesSystem::renderBlock &renderData = pParticles->GetRenderData();

	if (false == IsMoPlugsRender() )
	{
		// Get current camera
		FBCamera*	lCamera	 = pRenderOptions->GetRenderingCamera();

		if ( lCamera && FBIS(lCamera, FBCameraSwitcher) )
		{
			lCamera = ( (FBCameraSwitcher*) lCamera)->CurrentCamera;
		}

		FBVector3d pos;
		FBMatrix vp, mv;
		lCamera->GetCameraMatrix( vp, kFBProjection );
		lCamera->GetCameraMatrix( mv, kFBModelView );
		lCamera->GetVector(pos);

		//
		//

		for (int i=0; i<16; ++i)
		{
			renderData.gVP.mat_array[i] = (float) vp[i];
			renderData.gMV.mat_array[i] = (float) mv[i];
		}
		for (int i=0; i<3; ++i)
		{
			renderData.gCameraPos.vec_array[i] = (float) pos[i];
		}
	}
	else
	{
		CCameraInfoCache cache;
		GetCameraCache(cache);

		renderData.gVP = cache.p4;
		renderData.gMV = cache.mv4;
		renderData.gCameraPos = cache.pos;
	}

	
		
	GLuint texId = 0;
	FBTexture *texture = nullptr;
	if (TextureObject.GetCount() )
	{
		texture = (FBTexture*) TextureObject.GetAt(0);
		texId = texture->TextureOGLId;

		if (0 == texId)
		{
			texture->OGLInit(pRenderOptions);
			texId = texture->TextureOGLId;
		}
	}

	FBColorAndAlpha color = Color;
	renderData.gColor = vec4( (float)color[0], (float)color[1], (float)color[2], (float)color[3] );
	renderData.gUseColorCurve = (UseColorCurve) ? 1 : 0;
	renderData.gTransparencyFactor = (float) (0.01 * TransparencyFactor);

	renderData.gBillboardSize = (float) Size;
	renderData.gMinPointScale = (float) (0.01 * MinPointScale);
	renderData.gMaxPointScale = (float) (0.01 * MaxPointScale);
	renderData.gPointScaleDistance = (float) (PointScaleDistance);
	renderData.gUseSizeCurve = (UseSizeCurve) ? 1 : 0;

	pParticles->UploadRenderDataOnGPU();

	pParticles->SetRenderSizeAndColorCurves(mSizeCurve.GetTextureId(), mColorCurve.GetTextureId() );
	
	glPushClientAttrib(GL_ALL_ATTRIB_BITS);
	pParticles->RenderParticles(PrimitiveType, PointSmooth);
	glPopClientAttrib();
	CHECK_GL_ERROR();
#ifdef _DEBUG
	printf("[ShadeModel] End...\n" );
#endif
}

void GPUshader_Particles::SetTransparencyType( FBAlphaSource pTransparency )
{
    if (Transparency != pTransparency)
    {
        Transparency = pTransparency;
        //To trigger render to update the model-shader information.
        InvalidateShaderVersion();
    }
}

FBAlphaSource GPUshader_Particles::GetTransparencyType()
{
    return Transparency;
}

void GPUshader_Particles::SetTransparencyProperty( HIObject pObject, FBAlphaSource pState )
{     
    GPUshader_Particles* lShader = FBCast<GPUshader_Particles>(pObject);
    if (lShader->Transparency != pState)
    {
        lShader->Transparency.SetPropertyValue(pState);
        lShader->RenderingPass = GetRenderingPassNeededForAlpha(pState);
         
        // if shader use alpha and thus generate custom shape than the original geometry shape, 
        // we need to let it handle DrawShadow functiionality as well. 
        lShader->SetShaderCapacity(kFBShaderCapacityDrawShadow, pState != kFBAlphaSourceNoAlpha); 
    }
}

void GPUshader_Particles::SetEmitterProperty( HIObject pObject, FBParticleEmitter pState )
{     
    GPUshader_Particles* lShader = FBCast<GPUshader_Particles>(pObject);
	lShader->mLastTimelineTime = FBTime::Infinity;
	lShader->Emitter.SetPropertyValue(pState);
}

void GPUshader_Particles::UpdateConnectedCollisionsData()
{
	const int srcCount = GetSrcCount();

	int collisionsCount = 0;
	for (int i=0; i<srcCount; ++i)
	{
		FBPlug *pPlug = GetSrc(i);
		if (FBIS(pPlug, CollisionSphere) || FBIS(pPlug, CollisionTerrain) )
			collisionsCount += 1;
	}

	mParticleConnections.SetCollisionsCount(collisionsCount);

	collisionsCount = 0;
	ParticlesSystem::TCollision	coldata;

	for (int i=0; i<srcCount; ++i)
	{
		FBPlug *pPlug = GetSrc(i);
		if (FBIS(pPlug, CollisionSphere) )
		{
			( (CollisionSphere*) pPlug)->FillCollisionData(coldata);
			mParticleConnections.SetCollisionData(collisionsCount, coldata);
			collisionsCount += 1;
		}
		else if ( FBIS(pPlug, CollisionTerrain)  )
		{
			( (CollisionTerrain*) pPlug)->FillCollisionData(coldata);
			mParticleConnections.SetCollisionData(collisionsCount, coldata);
			collisionsCount += 1;
		}
	}
}

void GPUshader_Particles::UpdateConnectedForcesData()
{
	const int srcCount = GetSrcCount();

	int forcesCount = 0;
	for (int i=0; i<srcCount; ++i)
	{
		FBPlug *pPlug = GetSrc(i);
		if (FBIS(pPlug, ForceDrag) || FBIS(pPlug, ForceWind) || FBIS(pPlug, ForceMotor) )
			forcesCount += 1;
	}

	mParticleConnections.SetForcesCount(forcesCount);

	forcesCount = 0;
	ParticlesSystem::TForce data;

	for (int i=0; i<srcCount; ++i)
	{
		FBPlug *pPlug = GetSrc(i);
		if (FBIS(pPlug, ForceDrag) )
		{
			( (ForceDrag*) pPlug)->FillForceData(data);
			mParticleConnections.SetForceData(forcesCount, data);
			forcesCount += 1;
		}
		else if ( FBIS(pPlug, ForceWind)  )
		{
			( (ForceWind*) pPlug)->FillForceData(data, mRenderFrameId);
			mParticleConnections.SetForceData(forcesCount, data);
			forcesCount += 1;
		}
		else if ( FBIS(pPlug, ForceMotor)  )
		{
			( (ForceMotor*) pPlug)->FillForceData(data);
			mParticleConnections.SetForceData(forcesCount, data);
			forcesCount += 1;
		}
	}
}

void GPUshader_Particles::UpdateConnectedData()
{
	const int srcCount = GetSrcCount();

	int collisionsCount = 0;
	int forcesCount = 0;

	for (int i=0; i<srcCount; ++i)
	{
		FBPlug *pPlug = GetSrc(i);
		if (FBIS(pPlug, CollisionSphere) || FBIS(pPlug, CollisionTerrain) )
			collisionsCount += 1;
		else if (FBIS(pPlug, ForceDrag) || FBIS(pPlug, ForceWind) || FBIS(pPlug, ForceMotor) )
			forcesCount += 1;
	}

	//
	mParticleConnections.SetCollisionsCount(collisionsCount);
	mParticleConnections.SetForcesCount(forcesCount);

	collisionsCount = 0;
	forcesCount = 0;

	ParticlesSystem::TCollision	coldata;
	ParticlesSystem::TForce data;

	for (int i=0; i<srcCount; ++i)
	{
		FBPlug *pPlug = GetSrc(i);

		if (FBIS(pPlug, CollisionSphere) )
		{
			( (CollisionSphere*) pPlug)->FillCollisionData(coldata);
			mParticleConnections.SetCollisionData(collisionsCount, coldata);
			collisionsCount += 1;
		}
		else if ( FBIS(pPlug, CollisionTerrain)  )
		{
			( (CollisionTerrain*) pPlug)->FillCollisionData(coldata);
			mParticleConnections.SetCollisionData(collisionsCount, coldata);
			collisionsCount += 1;
		}
		else if (FBIS(pPlug, ForceDrag) )
		{
			( (ForceDrag*) pPlug)->FillForceData(data);
			mParticleConnections.SetForceData(forcesCount, data);
			forcesCount += 1;
		}
		else if ( FBIS(pPlug, ForceWind)  )
		{
			( (ForceWind*) pPlug)->FillForceData(data, mRenderFrameId);
			mParticleConnections.SetForceData(forcesCount, data);
			forcesCount += 1;
		}
		else if ( FBIS(pPlug, ForceMotor)  )
		{
			( (ForceMotor*) pPlug)->FillForceData(data);
			mParticleConnections.SetForceData(forcesCount, data);
			forcesCount += 1;
		}
	}
}

void GPUshader_Particles::UploadConnectedDataToGPU(ParticlesSystem::ParticleSystem *pParticles)
{
	pParticles->GetSimulationData().gNumForces = mParticleConnections.GetNumberOfForces();
	pParticles->GetSimulationData().gNumCollisions = mParticleConnections.GetNumberOfCollisions();

	if (UseCollisions == false)
		pParticles->GetSimulationData().gNumCollisions = 0;
	if (UseForces == false)
		pParticles->GetSimulationData().gNumForces = 0;

	//mEvaluateData.gNumCollisions = (mConnections) ? mConnections->GetNumberOfCollisions() : 0;
	//mEvaluateData.gNumForces = (mConnections) ? mConnections->GetNumberOfForces() : 0;

	mParticleConnections.UploadForcesToGPU();
	mParticleConnections.UploadCollisionsToGPU();
}

bool GPUshader_Particles::UpdateInstanceData()
{

	FBModel *instanceModel = nullptr;
	if (InstanceObject.GetCount() )  
		instanceModel = (FBModel*) InstanceObject.GetAt(0);

	if (instanceModel == nullptr)
		return false;

	// instance data is a connection, once per fbshaderinstance, not per assigned model !

	FBModelVertexData *pVertexData = instanceModel->ModelVertexData;
	if (pVertexData == nullptr)
		return false;

	mParticleConnections.SetInstanceData(
		pVertexData->GetVertexCount(), 
		pVertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Point), 
		pVertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Normal), 
		pVertexData->GetUVSetVBOId(),
		pVertexData->GetIndexArrayVBOId(),
		pVertexData->GetSubPatchCount() );

	for (int i=0, count=pVertexData->GetSubPatchCount(); i<count; ++i)
	{
		mParticleConnections.SetInstancePatchData( i, pVertexData->GetSubPatchIndexOffset(i), pVertexData->GetSubPatchIndexSize(i) );
	}

	return true;

}

/*
bool GPUshader_Particles::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	if (pThis == &Forces)
	{
		printf ("we need this\n" );
	}

	return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool GPUshader_Particles::PlugStateNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
	return ParentClass::PlugStateNotify(pAction, pThis, pData, pDataOld, pDataSize);
}
*/
/** PlugNotify when overloaded is equivalent of FBSystem.OnConnectionNotify but in the context of the derived object only
	* \param pAction Plug action, the event type.
	* \param pThis Plug concerned.
	* \param pIndex Index of the plug.
	* \param pPlug Other plug concerned.
	* \param pConnectionType Connection type between the plugs.
	* \param pNewPlug New plug (for the replace event).
	* \return should by default return true except in the case of connection requests (kFBRequestConnectSrc or kFBRequestConnectDst)
*/
bool GPUshader_Particles::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	if (pThis == &Forces)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	else if (pThis == &Collisions)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	else if (pThis == &InstanceObject)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void GPUshader_Particles::SyncForcesPropWithComponents()
{
	for (int i=0, count=Forces.GetCount(); i<count; ++i)
	{
		int index = Components.Find(Forces[i]);
		if (index < 0)
			Components.Add(Forces[i]);
	}
}

void GPUshader_Particles::UpdateEmitterGeometryBufferOnCPU(FBModel *pModel, ParticlesSystem::ParticleSystem *pParticles)
{
	FBModelVertexData *pVertexData = pModel->ModelVertexData;

	pVertexData->VertexArrayMappingRequest();

	unsigned int vertexCount = pVertexData->GetVertexCount();
	unsigned int indexCount = 0;
		
	for (int i=0, count=pVertexData->GetSubPatchCount(); i<count; ++i)
		indexCount = std::max(indexCount, (unsigned int) (pVertexData->GetSubPatchIndexOffset(i) + pVertexData->GetSubPatchIndexSize(i)) );
			
	float *positionsdata = (float*) pVertexData->GetVertexArray( kFBGeometryArrayID_Point ); 
	float *normalsdata = (float*) pVertexData->GetVertexArray( kFBGeometryArrayID_Normal );
	float *uvdata = (float*) pVertexData->GetUVSetArray();
	int *indexdata = pVertexData->GetIndexArray();

	GLuint textureId = 0;

	if (pModel->Materials.GetCount() > 0 )
	{
		FBMaterial *pMaterial = pModel->Materials[0];
		if (pMaterial->GetTexture() != nullptr )
		{
			FBTexture *pTexture = pMaterial->GetTexture();

			textureId = pTexture->TextureOGLId;
			if (0 == textureId)
			{
				pTexture->OGLInit();
				textureId = pTexture->TextureOGLId;
			}
		}
	}

	pParticles->EmitterSurfaceUpdateOnCPU( vertexCount, positionsdata, normalsdata, uvdata, indexCount, indexdata, textureId );
	pParticles->UploadSurfaceDataToGPU();

	pVertexData->VertexArrayMappingRelease();
}

void GPUshader_Particles::UpdateEmitterGeometryBufferOnGPU(FBModel *pModel, ParticlesSystem::ParticleSystem *pParticles)
{
	FBModelVertexData *pVertexData = pModel->ModelVertexData;

	GLuint textureId = 0;

	if (pModel->Materials.GetCount() > 0 )
	{
		FBMaterial *pMaterial = pModel->Materials[0];
		if (pMaterial->GetTexture() != nullptr )
		{
			FBTexture *pTexture = pMaterial->GetTexture();

			textureId = pTexture->TextureOGLId;
			if (0 == textureId)
			{
				pTexture->OGLInit();
				textureId = pTexture->TextureOGLId;
			}
		}
	}

	pParticles->EmitterSurfaceUpdateOnGPU( pVertexData, textureId );
}

void GPUshader_Particles::UpdateConnectedTerrain()
{
	mParticleConnections.SetTextureTerrain(0);

	//  perhaps update terrain per frame ?!
	const int srcCount = GetSrcCount();
	for (int i=0; i<srcCount; ++i)
	{
		if (FBIS( GetSrc(i), CollisionTerrain ) )
		{
			( (CollisionTerrain*) GetSrc(i) )->DoUpdate();
			mParticleConnections.SetTextureTerrain( ( (CollisionTerrain*) GetSrc(i) )->GetTextureId() );
		}
	}
}

void GPUshader_Particles::UpdateEvaluationData(FBModel *pModel, ParticlesSystem::ParticleSystem *pParticles, const bool enableEmit)
{
	FBMatrix m, normalTM;
	FBVector3d min, max, pos;

	FBVector3d emitterVel(0.0, 0.0, 0.0);

	// TODO: use rotation around pivot point to compute velocity
	if (pModel)
	{
		pModel->GetVector( pos );
		pModel->GetMatrix(m);
		pModel->GetMatrix(normalTM, kModelInverse_Transformation);
		FBMatrixTranspose(normalTM, normalTM);

		pModel->GetBoundingBox(min, max);

		// store in local
		//VectorTransform( min, m, min );
		//VectorTransform( max, m, max );

		FBVector3d lastpos;
		pParticles->GetLastEmitterPos(lastpos);

		if (InheritEmitterVelocity) emitterVel = VectorSubtract( pos, lastpos );
		pParticles->SetLastEmitterPos( pos );
	}

	evaluateBlock	&data = pParticles->GetSimulationData();

	mat4 &m4 = data.gTM;
	mat4 &nm4 = data.gNormalTM;
	for (int i=0; i<16; ++i)
	{
		m4.mat_array[i] = (float) m[i];
		nm4.mat_array[i] = (float) normalTM[i];
	}

	FBVector3d direction = EmitterDirection;
	FBVector3d dirRandom = EmitterDirRandom;
	FBVector3d velocity = EmitterVelocity;
	FBVector3d velRandom = EmitterVelRandom;
	FBVector3d gravityDir = Gravity;

	EvaluationExchange::SetDirection( data, vec3((float)direction[0], (float)direction[1], (float)direction[2]), vec3( (float)dirRandom[0]*0.01f, (float)dirRandom[1]*0.01f, (float)dirRandom[2]*0.01f), UseEmitterNormals );
	EvaluationExchange::SetVelocity( data, vec3((float)velocity[0], (float)velocity[1], (float)velocity[2]), vec3((float)velRandom[0]*0.01f, (float)velRandom[1]*0.01f, (float)velRandom[2]*0.01f), vec4((float)emitterVel[0], (float)emitterVel[1], (float)emitterVel[2], (InheritEmitterVelocity) ? 1.0f : 0.0f) );
	EvaluationExchange::SetDynamicParameters( data, 0.01f * (float) Mass, 0.01f * (float) Damping );
	EvaluationExchange::SetFlags( data, enableEmit, UseCollisions, (int) Emitter );
	EvaluationExchange::SetTurbulence( data, UseTurbulence, 0.0001f * NoiseFrequency, 0.0001f * NoiseSpeed, 0.01f * NoiseAmplitude );
	EvaluationExchange::SetFloorParamaters( data, UseFloor, 0.01f * (float)FloorFriction, (float) FloorLevel );
	EvaluationExchange::SetGravity( data, vec3( (float)gravityDir[0], (float)gravityDir[1], (float)gravityDir[2]), UseGravity );
	EvaluationExchange::SetBoundingBox(data, vec3((float)min[0], (float)min[1], (float)min[2]), vec3( (float)max[0], (float)max[1], (float)max[2]) );

	double dConstraintMagn;
	ConstraintMagnitude.GetData( &dConstraintMagn, sizeof(double) );

	data.gDynamic.z = 0.01f * dConstraintMagn;

	data.gShellLifetime = (float) LifeTime;
	data.gShellLifetimeVariation = (float) LifeTimeVariation * 0.01f;

	data.gUseSizeAttenuation = (UseSizeCurve) ? 1 : 0;
	data.gUseColorAttenuation = 0;

	data.gSize = Size;
	data.gSizeVariation = 0.01f * SizeVariation;
	data.gColorVariation = 0.01f * ColorVariation;

	data.gInheritEmitterColor = (InheritEmitterColors) ? 1 : 0;
	
	FBColorAndAlpha	dColor = Color;
	vec4 color((float)dColor[0], (float)dColor[1], (float)dColor[2], (float)dColor[3]);
	
	data.gColor = color;
}