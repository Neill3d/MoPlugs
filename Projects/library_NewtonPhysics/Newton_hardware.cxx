
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Newton_hardware.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#define PHYSICS_FPS		  120.0
#define FPS_IN_MICROSECUNDS  (int (1000000.0/DEMO_PHYSICS_FPS))
#define FPS_IN_SECONDS	(1.0 / PHYSICS_FPS)

//--- Class declaration
#include "Newton_hardware.h"
#include "newton_public.h"
#include "dHighResolutionTimer.h"

//#include "..\orconstraint_CarPhysics_constraint.h"

#define DEMO_GRAVITY  -10.0f

#define VIPER_ENGINE_MOMENT_OF_INERTIA   1.0f

/////////////////////////////////////////////////////////////////////////////////////////
// BodyEntity

const double *CRigidBody::GetMatrix(IWorld *world)
{
	if (mEntity.get() == nullptr)
		return nullptr;

	// get render matrix
	dMatrix currMatrix = (mEntity.get()->GetCurrentMatrix() );
	
	CWorldManager *pmanager = (CWorldManager*) world;
	const float globalScaling = 1.0f / (float)pmanager->GetGlobalScale();
	currMatrix.m_posit = currMatrix.m_posit.Scale(globalScaling);

	double *d = (double*) &m_dmatrix[0];

	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
		{
			*d = (double) currMatrix[i][j];
			d++;
		}

	return m_dmatrix;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Vehicle Controller

void CarVehicleControllerManager::RenderVehicleSchematic (const int width, const int height, const CustomVehicleController* const controller) const
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable (GL_BLEND);

	dFloat scale = 100.0f;
	dFloat dwidth = (dFloat) width;
	dFloat dheight = (dFloat) height;

	dMatrix origin (dGetIdentityMatrix());
	origin.m_posit = dVector(dwidth - 300, dheight - 200, 0.0f, 1.0f);

	glPushMatrix();
	glMultMatrixd (&origin[0][0]);
	DrawSchematic (controller, scale);
	glPopMatrix();

	glLineWidth(1.0f);
	glEnable(GL_TEXTURE_2D);
}


void CarVehicleControllerManager::DrawSchematicCallback (const CustomVehicleController* const controller, const char* const partName, dFloat value, int pointCount, const dVector* const lines) const
{
	if (!strcmp (partName, "chassis")) {
		glLineWidth(3.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_LINES);
		dVector p0 (lines[pointCount - 1]);
		for (int i = 0; i < pointCount; i ++) {
			dVector p1 (lines[i]);
			glVertex3d(p0.m_x, p0.m_y, p0.m_z);
			glVertex3d(p1.m_x, p1.m_y, p1.m_z);
			p0 = p1;
		}
		glEnd();
	}

	if (!strcmp (partName, "tire")) {
		glLineWidth(2.0f);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glBegin(GL_LINES);
		dVector p0 (lines[pointCount - 1]);
		for (int i = 0; i < pointCount; i ++) {
			dVector p1 (lines[i]);
			glVertex3d(p0.m_x, p0.m_y, p0.m_z);
			glVertex3d(p1.m_x, p1.m_y, p1.m_z);
			p0 = p1;
		}
		glEnd();
	}

	if (!strcmp (partName, "velocity")) {
		glLineWidth(2.0f);
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
		glBegin(GL_LINES);
		dVector p0 (lines[0]);
		dVector p1 (lines[1]);
		glVertex3d(p0.m_x, p0.m_y, p0.m_z);
		glVertex3d(p1.m_x, p1.m_y, p1.m_z);
		glEnd();
	}

	if (!strcmp (partName, "lateralForce")) {
		glLineWidth(2.0f);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glBegin(GL_LINES);
		dVector p0 (lines[0]);
		dVector p1 (lines[1]);
		glVertex3d(p0.m_x, p0.m_y, p0.m_z);
		glVertex3d(p1.m_x, p1.m_y, p1.m_z);
		glEnd();
	}

	if (!strcmp (partName, "longitudinalForce")) {
		glLineWidth(2.0f);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glBegin(GL_LINES);
		dVector p0 (lines[0]);
		dVector p1 (lines[1]);
		glVertex3d(p0.m_x, p0.m_y, p0.m_z);
		glVertex3d(p1.m_x, p1.m_y, p1.m_z);
		glEnd();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Newton Physics Engine
//
/////////////////////////////////////////////////////////////////////////////////////////

// add force and torque to rigid body
void  PhysicsApplyGravityForce (const NewtonBody* body, dFloat timestep, int threadIndex)
{
	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;
	dFloat mass;

	NewtonWorld *const world = NewtonBodyGetWorld(body);
	BaseEntityManager *pHardware = (BaseEntityManager*) NewtonWorldGetUserData(world);

	const double *gravity = pHardware->GetGravity();

	NewtonBodyGetMass (body, &mass, &Ixx, &Iyy, &Izz);
	dVector force (mass * gravity[0], mass * gravity[1], mass * gravity[2]);
	NewtonBodySetForce (body, &force.m_x);
}



NewtonBody *LoadLevelAndSceneRoot( const NewtonWorld *pWorld, const double globalScaling, const IQueryGeometry *level, int optimized, NewtonCollision *&outLevelCollision)
{
	
	if (level->GetPolyCount() == 0)
		return nullptr;

	//
	// build collision object
	//
	
	NewtonMesh *pmesh = NewtonMeshCreate(pWorld);

	// DONE: load mesh
	
	// iterate the entire geometry an build the collision

	dVector face[4];

	const IQueryGeometry::Poly *ptrPoly = nullptr;

	NewtonMeshBeginFace(pmesh);
	for (int i=0; i<level->GetPolyCount(); ++i)
	{
		ptrPoly = level->GetPoly(i);

		assert(ptrPoly->count <= 4);

		for (int j=0; j<ptrPoly->count; ++j)
		{
			const float *vert = level->GetVertexPosition(ptrPoly->indices[j]);

			face[j][0] = (dFloat) vert[0];
			face[j][1] = (dFloat) vert[1];
			face[j][2] = (dFloat) vert[2];
			face[j][3] = 1.0f;

			face[j] = face[j].Scale(globalScaling);
		}
		
		NewtonMeshAddFace(pmesh, ptrPoly->count, &face[0][0], sizeof (dVector), ptrPoly->matId);
		
	}
	NewtonMeshEndFace(pmesh);
	
	NewtonMeshPolygonize(pmesh);
	outLevelCollision = NewtonCreateTreeCollisionFromMesh(pWorld, pmesh, 0);
	NewtonMeshDestroy(pmesh);
	
	/*
	// create the collision tree geometry
	NewtonCollision *tree = NewtonCreateTreeCollision(pWorld, 0);


	// prepare to create collision geometry
	NewtonTreeCollisionBeginBuild(tree);


	// iterate the entire geometry an build the collision

	dVector face[4];

	const Poly *ptrPoly = (level->GetPolyCount() > 0) ? level->GetPoly(0) : nullptr;

	for (int i=0; i<level->GetPolyCount(); ++i)
	{
		ptrPoly = level->GetPoly(i);

		assert(ptrPoly->count <= 4);

		for (int j=0; j<ptrPoly->count; ++j)
		{
			const float *vert = level->GetVertexPosition(ptrPoly->indices[j]);

			face[j][0] = (dFloat) vert[0];
			face[j][1] = (dFloat) vert[1];
			face[j][2] = (dFloat) vert[2];
			face[j][3] = 1.0f;

			face[j] = face[j].Scale(globalScaling);
		}

		NewtonTreeCollisionAddFace(tree, ptrPoly->count, &face[0][0], sizeof (dVector), ptrPoly->matId);
	}

	NewtonTreeCollisionEndBuild(tree, 0); // optimized);
	*/

	//
	// create the level rigid body
	//
	dMatrix   m_matrix( dGetIdentityMatrix() );
	//m_matrix = dPitchMatrix (15.0f* 3.141592f / 180.0f);
	
	NewtonBody *pLevel = NewtonCreateDynamicBody(pWorld, outLevelCollision, &m_matrix[0][0]);

	// release the collision tree (this way the application does not have to do book keeping of Newton objects
	
	//NewtonDestroyCollision (tree);

	// set the global position of this body
	NewtonBodySetMatrix (pLevel, (dFloat*) &m_matrix[0][0]);

	// save the pointer to the graphic object with the body.
	NewtonBodySetUserData (pLevel, nullptr);

  //
  // world settings
  //
//  NewtonBodySetDestructorCallback (gLevel, PhisicsBodyDestructor);

  int defaultID;
  defaultID = NewtonMaterialGetDefaultGroupID (pWorld);

  // set default material properties
  NewtonMaterialSetDefaultSoftness( pWorld, defaultID, defaultID, 0.15f );
  NewtonMaterialSetDefaultElasticity( pWorld, defaultID, defaultID, 0.30f );
  NewtonMaterialSetDefaultCollidable( pWorld, defaultID, defaultID, 1 );
  NewtonMaterialSetDefaultFriction( pWorld, defaultID, defaultID, 5.0f, 1.05f );

  // set the island update callback
  //NewtonSetIslandUpdateEvent( gWorld, PhysicsIslandUpdate );

  return pLevel;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//

CWorldManager::CWorldManager(const double globalScale, const int numberOfThreads, const int physicsLoops, const float physicsFPS)
	: IWorld()
	, mManager(new BaseEntityManager(globalScale, numberOfThreads, physicsLoops, physicsFPS))
{
	m_currentTime = 0;

	mLevelBody = nullptr;
	mLevelCollision = nullptr;

	// create a vehicle controller manager
	int defaulMaterial = NewtonMaterialGetDefaultGroupID(mManager->GetNewton());
	NewtonMaterialSetDefaultFriction(mManager->GetNewton(), defaulMaterial, defaulMaterial, 0.6f, 0.5f);

	int materialList[] = {defaulMaterial };

	mVehicleManager = new CarVehicleControllerManager ( mManager->GetNewton(), 1, materialList );
}

CWorldManager::~CWorldManager()
{
	ClearLevel();
	/*
	// NOTE: this will be deleted from Newton "DestroyAllController()"
	if (mVehicleManager)
	{
		delete mVehicleManager;
		mVehicleManager = nullptr;
	}
	*/
}

bool CWorldManager::Clear()
{
	ClearLevel();
	mManager->Clear();

	return true;
}

const char *CWorldManager::GetPhysicsInfo()
{
	static char szVersion[128];

	memset(szVersion, 0, sizeof(char)*128);
	sprintf_s( szVersion, sizeof(char)*128, "Newton - %d", NewtonWorldGetVersion() );
	
	return &szVersion[0];
}

void CWorldManager::SetGravity(const double *gravity)
{
	mManager->SetGravity(gravity);
}
const double *CWorldManager::GetGravity() const
{
	return mManager->GetGravity();
}

void CWorldManager::SetGlobalScale(const double scale)
{
	mManager->SetGlobalScale(scale);
}
const double CWorldManager::GetGlobalScale() const
{
	return mManager->GetGlobalScale();
}


void CWorldManager::SetDefaultSoftness(const double value)
{
	mManager->SetDefaultSoftness(value);
}

const double CWorldManager::GetDefaultSoftness() const
{
	return mManager->GetDefaultSoftness();
}
	
void CWorldManager::SetDefaultElasticity(const double value)
{
	mManager->SetDefaultElasticity(value);
}
const double CWorldManager::GetDefaultElasticity() const
{
	return mManager->GetDefaultElasticity();
}
	
void CWorldManager::SetDefaultCollidable(const double value)
{
	mManager->SetDefaultCollidable(value);
}
const double CWorldManager::GetDefaultCollidable() const
{
	return mManager->GetDefaultCollidable();
}
	
void CWorldManager::SetDefaultFriction(const double staticValue, const double dynamicValue)
{
	mManager->SetDefaultFriction(staticValue, dynamicValue);
}
const double CWorldManager::GetDefaultStaticFriction() const
{
	return mManager->GetDefaultStaticFriction();
}
const double CWorldManager::GetDefaultDynamicFriction() const
{
	return mManager->GetDefaultDynamicFriction();
}

void CWorldManager::SetDefaultMaterialParams(double softness, double elasticity, double collidable, double staticFriction, double dynamicFriciton)
{
	mManager->SetDefaultMaterialParams(softness, elasticity, collidable, staticFriction, dynamicFriciton);
}


void CWorldManager::ClearLevel()
{
	if (mLevelBody && mManager->GetNewton())
	{
		NewtonDestroyBody( mLevelBody );
		mLevelBody = nullptr;

		if (mLevelCollision)
		{
			// release the collision tree (this way the application does not have to do book keeping of Newton objects
			NewtonDestroyCollision (mLevelCollision);
			mLevelCollision = nullptr;
		}
	}
}

// level will be scaled according to the global Phys engine scale
bool CWorldManager::LoadLevel(  const IQueryGeometry *levelInfo )
{
	ClearLevel();

	if ( mManager->GetNewton() )
	{
		mLevelBody = LoadLevelAndSceneRoot( mManager->GetNewton(), GetGlobalScale(), levelInfo, 1, mLevelCollision );
	}

	return (mLevelBody != nullptr);
}


bool CWorldManager::ClosestRayCast(const double *p0, const double *p1, double *pos, double *nor, double *dist, long long *attribute)
{
	if ( mManager->GetNewton() && mLevelCollision != nullptr )
	{
		*dist = NewtonCollisionRayCast( mLevelCollision, p0, p1, nor, attribute );
		
		for (int i=0; i<3; ++i)
			pos[i] = *dist * (p1[i] - p0[i]) + p0[i];

		return ( *dist < 1.0 && *dist > 0.0 );
	}

	return false;
}

void CWorldManager::WaitForUpdateToFinish()
{
	if ( mManager->GetNewton() )
	{
		NewtonWaitForUpdateToFinish( mManager->GetNewton() );
	}
}

CustomVehicleControllerManager *CWorldManager::GetVehicleManager() const
{
	return mVehicleManager;
}

IBody *CWorldManager::CreateNewBody( const BodyOptions *info, const IQueryGeometry *pgeometry, bool convexhull )
{
	auto pbody = CreateNewNewtonBody( this, info, pgeometry, convexhull );
	return pbody;
}

ICar	*CWorldManager::CreateNewCar( const CarOptions *info, const IQueryGeometry **pgeometry, const IQueryPath *curve  )
{
	auto pcar = CreateNewNewtonCar( this, info, pgeometry, curve );
	return pcar;
}

//--- Opens and closes connection with data server. returns true if successful
bool CWorldManager::Open()
{
	
	SaveState();
	return true;
}

bool CWorldManager::Close()
{
	return true;
}

//--- Hardware communication
void CWorldManager::Reset()
{
	m_currentTime = 0;
	mManager->ResetTimer();
}

bool CWorldManager::FetchDataPacket (const double time)
{
	PollData();

	if ( (m_currentTime == 0) || (m_currentTime > time) ) 
	{
		m_currentTime = time; // - FPS_IN_SECONDS;
	}

	double deltaTime = time - m_currentTime;

	bool result = mManager->UpdatePhysics2( (float) time);

	m_currentTime = time;

	/*
	static bool firstRun = true;

	if (firstRun)
	{
		firstRun = false;

		NewtonSerializeToFile( GetNewton(), "C:\\world_snapshot.dat" );
	}
	*/
	return result;
}

double CWorldManager::GetCurrPhysTimeSecs()
{
	return mManager->GetCurrPhysicsTimeSecs();
}

double CWorldManager::GetLastPhysTimeSecs()
{
	return mManager->GetLastPhysicsTimeSecs();
}

bool CWorldManager::PollData()
{
	return true;
}

bool CWorldManager::GetSetupInfo()
{
	return true;
}

bool CWorldManager::StartDataStream()
{
	return true;
}

bool CWorldManager::StopDataStream()
{
	return true;
}

	// snapping current objects tm and restore from this state if needed
void CWorldManager::SaveState()
{
	if (mManager.get() )
		mManager->SaveState();
}

void CWorldManager::RestoreState()
{
	if (mManager.get() )
		mManager->RestoreState();
}

void CWorldManager::DrawDebug() const
{
	if (mManager.get() )
		mManager->DrawDebug();
}

void CWorldManager::Serialize(const char *filename)
{
	if (mManager.get() )
		mManager->Serialize(filename);
}
