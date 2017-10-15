
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Newton_Entities.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Newton_entities.h"

//#include "dRuntimeProfiler.h"
#include "dHighResolutionTimer.h"
#include "DebugDisplay.h"

//
using namespace PHYSICS_INTERFACE;

//

#define MAX_PHYSICS_LOOPS			1024
#define MAX_PHYSICS_FPS				240.0f

//////////////////////////////////////////////////////////////////////////////////////////
//

BaseEntityManager::BaseEntityManager(const double globalScale, const int numberOfThreads, const int physicsLoops, const float physicsFPS)
	:dList <BaseEntity*>() 
	,m_world(NULL)
	,m_microsecunds(0)
	,m_physicsTime(0.0f)
	,m_lastPhysicsTime(0.0f)
	,m_physicsUpdate(true) 
	,m_reEntrantUpdate (false)
	,m_physThreadTime(0.0f)
	,mGlobalScale(globalScale)
	, mNumberOfThreads(numberOfThreads)
	, mPhysicsLoops(physicsLoops)
	, mPhysicsFPS(physicsFPS)
{
	ResetDefaultMaterialParams();

	// initialized the physics world for the new scene
	Cleanup ();

	ResetTimer();

	// Set performance counters off
	//memset (m_showProfiler, 0, sizeof (m_showProfiler));
	//m_profiler.Init(this);

	//m_context = new wxGLContext(this);
}

BaseEntityManager::~BaseEntityManager()
{
	// is we are run asynchronous we need make sure no update in on flight.
	if (m_world) {
		NewtonSyncThreadJobs(m_world);
		NewtonWaitForUpdateToFinish (m_world);
	}

	//glDeleteLists(m_font, 96);	
	//ReleaseTexture(m_fontImage);

	Cleanup ();

	// destroy the empty world
	if (m_world) {
		NewtonDestroy (m_world);
		m_world = NULL;
	}
	dAssert (NewtonGetMemoryUsed () == 0);
}

void BaseEntityManager::ResetTimer()
{
	dResetTimer();
	//m_microsecunds = dGetTimeInMicrosenconds ();
	m_microsecunds = 0;

	m_physicsTime = 0.0;
	m_lastPhysicsTime = 0.0;
	m_animTime = 0.0;
}

void BaseEntityManager::RemoveEntity (dListNode* const entNode)
{
	BaseEntity* const entity = entNode->GetInfo();
	//entity->Release();
	Remove(entNode);
}

void BaseEntityManager::RemoveEntity (BaseEntity* const ent)
{
	for (dListNode* node = dList<BaseEntity*>::GetFirst(); node; node = node->GetNext()) {
		if (node->GetInfo() == ent) {
			RemoveEntity (node);
			break;
		}
	}
}

void BaseEntityManager::Clear()
{
	// is we are run asynchronous we need make sure no update in on flight.
	if (m_world) {
		NewtonSyncThreadJobs(m_world);
		NewtonWaitForUpdateToFinish (m_world);
	}

	//glDeleteLists(m_font, 96);	
	//ReleaseTexture(m_fontImage);

	// is we are run asynchronous we need make sure no update in on flight.
	if (m_world) {
		NewtonWaitForUpdateToFinish (m_world);
	}

	// destroy all remaining visual objects
	while (dList<BaseEntity*>::GetFirst()) {
		dList<BaseEntity*>::GetFirst()->GetInfo()->DetachFromWorld(m_world);
		RemoveEntity (dList<BaseEntity*>::GetFirst());
	}

	//m_sky = NULL;

	// destroy the Newton world
	if (m_world) {
		NewtonDestroy (m_world);
		m_world = NULL;
	}

	// destroy the empty world
	if (m_world) {
		NewtonDestroy (m_world);
		m_world = NULL;
	}
	dAssert (NewtonGetMemoryUsed () == 0);
}

void BaseEntityManager::Cleanup()
{
	// is we are run asynchronous we need make sure no update in on flight.
	if (m_world) {
		NewtonWaitForUpdateToFinish (m_world);
	}

	// destroy all remaining visual objects
	while (dList<BaseEntity*>::GetFirst()) {
		dList<BaseEntity*>::GetFirst()->GetInfo()->DetachFromWorld(m_world);
		RemoveEntity (dList<BaseEntity*>::GetFirst());
	}

	//m_sky = NULL;

	// destroy the Newton world
	if (m_world) {
		NewtonDestroy (m_world);
		m_world = NULL;
	}

	//	memset (&demo, 0, sizeof (demo));
	// check that there are no memory leak on exit
	dAssert (NewtonGetMemoryUsed () == 0);


	// create the newton world
	m_world = NewtonCreate();

	// link the work with this user data
	NewtonWorldSetUserData(m_world, this);

	// add all physics pre and post listeners
	//	m_preListenerManager.Append(new DemoVisualDebugerListener("visualDebuger", m_world));
	//new DemoEntityListener (this);
	//m_cameraManager = new DemoCameraListener(this);
	//m_soundManager = new DemoSoundListener(this);
	//	m_postListenerManager.Append (new DemoAIListener("aiManager"));

	// set the default parameters for the newton world
	// set the simplified solver mode (faster but less accurate)
	NewtonSetSolverModel (m_world, 1);

	if (mNumberOfThreads == 2 || mNumberOfThreads == 4 || mNumberOfThreads == 8)
		NewtonSetThreadsCount(m_world, mNumberOfThreads);

	// newton 300 does not have world size, this is better controlled by the client application
	//dVector minSize (-500.0f, -500.0f, -500.0f);
	//dVector maxSize ( 500.0f,  500.0f,  500.0f);
	//NewtonSetWorldSize (m_world, &minSize[0], &maxSize[0]); 

	// set the performance track function
	//NewtonSetPerformanceClock (m_world, dRuntimeProfiler::GetTimeInMicrosenconds);

	// clean up all caches the engine have saved
	NewtonInvalidateCache (m_world);

	// Set the Newton world user data
	NewtonWorldSetUserData(m_world, this);

	UpdateDefaultMaterialParams();
	// we start without 2d render
	//m_renderHood = NULL;
	//m_renderHoodContext = NULL;
}

double BaseEntityManager::GetPhysicsTime()
{
	return m_physicsTime;
}

double BaseEntityManager::GetCurrPhysicsTimeSecs()
{
	return (double) m_physicsTime;
}

double BaseEntityManager::GetLastPhysicsTimeSecs()
{
	return (double) m_lastPhysicsTime;
}

bool BaseEntityManager::UpdatePhysics(float timestep)
{
	bool updated = false;
// read the controls 
	// update the physics
	if (m_world) {
		//Sleep (40);

		dFloat timestepInSecunds = 1.0f / mPhysicsFPS;
		unsigned64 timestepMicrosecunds = unsigned64 (timestepInSecunds * 1000000.0f);

		unsigned64 currentTime = dGetTimeInMicrosenconds ();
		unsigned64 nextTime = currentTime - m_microsecunds;
		int loops = 0;

		while ((nextTime >= timestepMicrosecunds) && (loops < mPhysicsLoops)) {
			loops ++;

			// run the newton update function
			if (!m_reEntrantUpdate) {
				m_reEntrantUpdate = true;
				if (m_physicsUpdate && m_world) {

					//ClearDebugDisplay(m_world);

					// update the physics world
					/*
					if (!m_mainWindow->m_physicsUpdateMode) {
						NewtonUpdate (m_world, timestepInSecunds);
					} else {
						NewtonUpdateAsync(m_world, timestepInSecunds);
					}
					*/
					//NewtonUpdateAsync(m_world, timestepInSecunds);
					NewtonUpdate (m_world, timestepInSecunds);
					updated = true;

					//m_physThreadTime = NewtonReadPerformanceTicks (m_world, NEWTON_PROFILER_WORLD_UPDATE) * 1.0e-3f;
				}
				m_reEntrantUpdate = false;
			}

			nextTime -= timestepMicrosecunds;
			m_microsecunds += timestepMicrosecunds;
		}

		if (loops) {
			m_physicsTime = dFloat (dGetTimeInMicrosenconds () - currentTime) / 1000000.0f;

			if (m_physicsTime >= mPhysicsLoops * (1.0f / mPhysicsFPS)) {
				m_microsecunds = currentTime;
			}
		}
	}

	return updated;
}


bool BaseEntityManager::UpdatePhysics2(double animTimeSecs)
{
	bool updated = false;
// read the controls 
	// update the physics
	if (m_world) {
		//Sleep (40);
		m_animTime = animTimeSecs;
		m_lastPhysicsTime = m_physicsTime;

		dFloat timestepInSecunds = 1.0f / mPhysicsFPS;
		unsigned64 timestepMicrosecunds = unsigned64 (timestepInSecunds * 1000000.0f);

		unsigned64 currentTime = timestepMicrosecunds + unsigned64 (animTimeSecs * 1000000.0);
		unsigned64 nextTime = currentTime - m_microsecunds;
		int loops = 0;

		while ((nextTime >= timestepMicrosecunds) && (loops < mPhysicsLoops)) {
			loops ++;

			// run the newton update function
			if (!m_reEntrantUpdate) {
				m_reEntrantUpdate = true;
				if (m_physicsUpdate && m_world) {

					//ClearDebugDisplay(m_world);

					// update the physics world
					/*
					if (!m_mainWindow->m_physicsUpdateMode) {
						NewtonUpdate (m_world, timestepInSecunds);
					} else {
						NewtonUpdateAsync(m_world, timestepInSecunds);
					}
					*/
					NewtonUpdateAsync(m_world, timestepInSecunds);
					//NewtonUpdate (m_world, timestepInSecunds);
					updated = true;

					//m_physThreadTime = NewtonReadPerformanceTicks (m_world, NEWTON_PROFILER_WORLD_UPDATE) * 1.0e-3f;
				}
				m_reEntrantUpdate = false;
			}

			nextTime -= timestepMicrosecunds;
			m_microsecunds += timestepMicrosecunds;
		}

		if (loops) {
			m_physicsTime = double (m_microsecunds) / 1000000.0;
			/*
			if (m_physicsTime >= mPhysicsLoops * (1.0f / mPhysicsFPS)) {
				m_microsecunds = currentTime;
			}
			*/
		}
	}

	return updated;
}

dFloat BaseEntityManager::CalculateInteplationParam () const
{
	//unsigned64 timeStep = dGetTimeInMicrosenconds () - m_microsecunds;		
	//dFloat param = (dFloat (timeStep) * MAX_PHYSICS_FPS) / 1.0e6f;
	double param = 0.0;
	if (m_physicsTime > m_lastPhysicsTime)
		param = (m_animTime - m_lastPhysicsTime) / (m_physicsTime - m_lastPhysicsTime);

	dAssert (param >= 0.0);
	if (param > 1.0) {
		param = 1.0;
	}
	return (dFloat) param;
}

dFloat BaseEntityManager::CalculateInteplationParam (double time) const
{
	double param = 0.0;
	if (m_physicsTime > m_lastPhysicsTime)
		param = (time - m_lastPhysicsTime) / (m_physicsTime - m_lastPhysicsTime);
	dAssert (param >= 0.0);
	if (param > 1.0) {
		param = 1.0;
	}
	return (dFloat) param;
}


void BaseEntityManager::ResetDefaultMaterialParams()
{
	// set default material properties
	mDefaultSoftness = 0.15f;
	mDefaultElasticity =  0.30f;
	mDefaultCollidable = 1.0f;
	mDefaultStaticFriction = 0.6f;
	mDefaultDynamicFriction = 0.5f;

	UpdateDefaultMaterialParams();
}

void BaseEntityManager::UpdateDefaultMaterialParams()
{
	if (GetNewton() == nullptr)
		return;

	int defaultID;
	defaultID = NewtonMaterialGetDefaultGroupID (GetNewton() );

	// set default material properties
	NewtonMaterialSetDefaultSoftness( GetNewton(), defaultID, defaultID, (dFloat)mDefaultSoftness );
	NewtonMaterialSetDefaultElasticity( GetNewton(), defaultID, defaultID, (dFloat)mDefaultElasticity );
	NewtonMaterialSetDefaultCollidable( GetNewton(), defaultID, defaultID, (int)mDefaultCollidable );
	NewtonMaterialSetDefaultFriction( GetNewton(), defaultID, defaultID, (dFloat)mDefaultStaticFriction, (dFloat)mDefaultDynamicFriction );
}

// 3 double values
void BaseEntityManager::SetGravity(const double *gravity) 
{
	mGravity[0] = gravity[0];
	mGravity[1] = gravity[1];
	mGravity[2] = gravity[2];
}
const double *BaseEntityManager::GetGravity() const 
{
	return mGravity;
}

void BaseEntityManager::SetGlobalScale(const double scale) 
{ 
	mGlobalScale = scale; 
}

const double BaseEntityManager::GetGlobalScale() const 
{ 
	return mGlobalScale; 
}


void BaseEntityManager::SetDefaultSoftness(const double value)
{
	mDefaultSoftness = value;
	UpdateDefaultMaterialParams();
}

const double BaseEntityManager::GetDefaultSoftness() const
{
	return mDefaultSoftness;
}
	
void BaseEntityManager::SetDefaultElasticity(const double value)
{
	mDefaultElasticity = value;
	UpdateDefaultMaterialParams();
}
const double BaseEntityManager::GetDefaultElasticity() const
{
	return mDefaultElasticity;
}
	
void BaseEntityManager::SetDefaultCollidable(const double value)
{
	mDefaultCollidable = value;
	UpdateDefaultMaterialParams();
}
const double BaseEntityManager::GetDefaultCollidable() const
{
	return mDefaultCollidable;
}
	
void BaseEntityManager::SetDefaultFriction(const double staticValue, const double dynamicValue)
{
	mDefaultStaticFriction = staticValue;
	mDefaultDynamicFriction = dynamicValue;
	UpdateDefaultMaterialParams();
}

const double BaseEntityManager::GetDefaultStaticFriction() const
{
	return mDefaultStaticFriction;
}

const double BaseEntityManager::GetDefaultDynamicFriction() const
{
	return mDefaultDynamicFriction;
}

void BaseEntityManager::SetDefaultMaterialParams(double softness, double elasticity, double collidable, double staticFriction, double dynamicFriciton)
{
	mDefaultSoftness = softness;
	mDefaultElasticity = elasticity;
	mDefaultCollidable = collidable;
	mDefaultStaticFriction = staticFriction;
	mDefaultDynamicFriction = dynamicFriciton;
	UpdateDefaultMaterialParams();
}

void BaseEntityManager::SaveState()
{
	for (dListNode* node = dList<BaseEntity*>::GetFirst(); node; node = node->GetNext()) {
		if (node->GetInfo())
		{
			node->GetInfo()->SaveState();
		}
	}
}


void BaseEntityManager::RestoreState()
{
	if (GetNewton() == nullptr)
		return;

	for (dListNode* node = dList<BaseEntity*>::GetFirst(); node; node = node->GetNext()) {
		if (node->GetInfo())
		{
			// don't invalidate cache for each body, one global invalidation at the end
			node->GetInfo()->RestoreState(*this, false);
		}
	}

	//NewtonInvalidateCache( GetNewton() );
}

void BaseEntityManager::DrawDebug() const
{

	// Render AABB

	const double unscale = 1.0 / mGlobalScale;

	glPushMatrix();

	glScaled(unscale, unscale, unscale);

	DebugRenderWorldCollision( GetNewton(), m_lines );

	RenderAABB( GetNewton() );

	for (dListNode* node = dList<BaseEntity*>::GetFirst(); node; node = node->GetNext()) {
		if (node->GetInfo())
		{
			node->GetInfo()->DrawDebug();
		}
	}

	glPopMatrix();
}

void BaseEntityManager::Serialize(const char *filename)
{
	if (GetNewton() )
	{
//		NewtonSerializeToFile( GetNewton(), filename );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// BaseEntity


BaseEntity::BaseEntity(const dMatrix& matrix, BaseEntity* const parent)
	//:dClassInfo()
	:dHierarchy<BaseEntity>()
	,mAlignmentMatrix(dGetIdentityMatrix())
	,m_matrix(matrix) 
	,m_curPosition (matrix.m_posit)
	,m_nextPosition (matrix.m_posit)
	,m_curRotation (dQuaternion (matrix))
	,m_nextRotation (dQuaternion (matrix))
	,m_meshMatrix(dGetIdentityMatrix())
	,m_mesh (NULL)
	,m_userData(NULL)
	,m_initialMatrix(matrix)
	,m_lock(0) 
{
	if (parent) {
		Attach (parent);
	}

	mBody = nullptr;
	mBodyProxy = nullptr;
}


/*
BaseEntity::BaseEntity(DemoEntityManager& world, const dScene* const scene, dScene::dTreeNode* const rootSceneNode, dTree<DemoMeshInterface*, dScene::dTreeNode*>& meshCache, DemoEntityManager::EntityDictionary& entityDictionary, DemoEntity* const parent)
	:dClassInfo()
	,dHierarchy<DemoEntity>() 
	,m_matrix(dGetIdentityMatrix()) 
	,m_curPosition (0.0f, 0.0f, 0.0f, 1.0f)
	,m_nextPosition (0.0f, 0.0f, 0.0f, 1.0f)
	,m_curRotation (1.0f, 0.0f, 0.0f, 0.0f)
	,m_nextRotation (1.0f, 0.0f, 0.0f, 0.0f)
	,m_meshMatrix(dGetIdentityMatrix())
	,m_mesh (NULL)
	,m_userData(NULL)
	,m_lock(0) 
{
	// add this entity to the dictionary
	entityDictionary.Insert(this, rootSceneNode);

	// if this is a child mesh set it as child of the entity
//	dMatrix parentMatrix (GetIdentityMatrix());
	if (parent) {
		Attach (parent);
		dAssert (scene->FindParentByType(rootSceneNode, dSceneNodeInfo::GetRttiType()));
//		dScene::dTreeNode* const parentNode = scene->FindParentByType(rootSceneNode, dSceneNodeInfo::GetRttiType());
//		dSceneNodeInfo* const parentInfo = (dSceneNodeInfo*)scene->GetInfoFromNode (parentNode);
//		dAssert (parentInfo->IsType(dSceneNodeInfo::GetRttiType()));
//		parentMatrix = parentInfo->GetTransform();
	}

	dSceneNodeInfo* const sceneInfo = (dSceneNodeInfo*) scene->GetInfoFromNode (rootSceneNode);
	//dMatrix matrix (sceneInfo->GetTransform() * parentMatrix.Inverse4x4());
	dMatrix matrix (sceneInfo->GetTransform());
	ResetMatrix (world, matrix);
	SetNameID(sceneInfo->GetName());

	// if this node has a mesh, find it and attach it to this entity
	dScene::dTreeNode* const meshNode = scene->FindChildByType(rootSceneNode, dMeshNodeInfo::GetRttiType());
	if (meshNode) {
		DemoMeshInterface* const mesh = meshCache.Find(meshNode)->GetInfo();
		SetMesh(mesh, sceneInfo->GetGeometryTransform());
	}

	// we now scan for all dSceneNodeInfo node with direct connection to this rootSceneNode, 
	// and we load the as children of this entity
	for (void* child = scene->GetFirstChildLink(rootSceneNode); child; child = scene->GetNextChildLink (rootSceneNode, child)) {
		dScene::dTreeNode* const node = scene->GetNodeFromLink(child);
		dNodeInfo* const info = scene->GetInfoFromNode(node);
		if (info->IsType(dSceneNodeInfo::GetRttiType())) {
			new DemoEntity (world, scene, node, meshCache, entityDictionary, this);
		}
	}
}
*/
BaseEntity::BaseEntity(const BaseEntity& copyFrom)
	//:dClassInfo()
	:dHierarchy<BaseEntity>(copyFrom)
	,mAlignmentMatrix(copyFrom.mAlignmentMatrix)
	,m_matrix(copyFrom.m_matrix)
	,m_curPosition(copyFrom.m_curPosition)
	,m_nextPosition(copyFrom.m_nextPosition)
	,m_curRotation(copyFrom.m_curRotation)
	,m_nextRotation(copyFrom.m_nextRotation)
	,m_meshMatrix(copyFrom.m_meshMatrix)
	,m_mesh(copyFrom.m_mesh)
	,m_userData(NULL)
	,m_initialMatrix(copyFrom.m_initialMatrix)
	,m_lock(0)
{
	if (m_mesh) {
		//m_mesh->AddRef();
	}
	m_deleteUserData = false;
	mBody = nullptr;
	mBodyProxy = nullptr;
}

BaseEntity::~BaseEntity(void)
{
	// DONE: remove from entityManager if needed
	if (mBody)
	{
		BaseEntityManager* const scene = (BaseEntityManager*) NewtonWorldGetUserData(NewtonBodyGetWorld(mBody));
		if (scene != nullptr)
		{
			scene->RemoveEntity(this);
		}
	}

	if (mBody)
	{
		NewtonDestroyBody(mBody);
		mBody = nullptr;
	}

	if (m_userData && m_deleteUserData == true) {
		delete m_userData;
		m_userData = nullptr;
	}
	SetMesh(NULL, dGetIdentityMatrix());
}

void BaseEntity::DetachFromWorld(NewtonWorld * const pWorld)
{
	if (mBody)
	{
		// NOTE: it will be destroid from the newton world
		//NewtonDestroyBody(mBody);
		mBody = nullptr;
	}
}

dBaseHierarchy* BaseEntity::CreateClone () const
{
	return new BaseEntity(*this);
}


BaseEntity::UserData* BaseEntity::GetUserData ()
{
	return m_userData;
}

void BaseEntity::SetUserData (UserData* const data, bool autoDelete)
{
	m_userData = data;
	m_deleteUserData = autoDelete;
}

void BaseEntity::SetBodyProxy(PHYSICS_INTERFACE::IQueryGeometry		*bodyProxy)
{
	mBodyProxy = bodyProxy;
}

const void* BaseEntity::GetMesh() const
{
	return m_mesh;
}

void BaseEntity::SetMesh(const void* const mesh, const dMatrix& meshMatrix)
{
	m_meshMatrix = meshMatrix;
	if (m_mesh) {
		//m_mesh->Release();
	}
	m_mesh = mesh;
	if (mesh) {
		//mesh->AddRef();
	}
}

NewtonBody *BaseEntity::GetNewtonBody()
{
	return mBody;
}

void BaseEntity::SetNewtonBody(NewtonBody *const pBody)
{
	mBody = pBody;
}

const dMatrix& BaseEntity::GetMeshMatrix() const
{
	return m_meshMatrix;
}

void BaseEntity::SetMeshMatrix(const dMatrix& matrix)
{
	m_meshMatrix = matrix;
}

dMatrix BaseEntity::GetCurrentMatrix () const
{
	return dMatrix (m_curRotation, m_curPosition);
}

dMatrix BaseEntity::GetNextMatrix () const
{
	return dMatrix (m_nextRotation, m_nextPosition);
}

dMatrix BaseEntity::CalculateGlobalMatrix (const BaseEntity* const root) const
{
	dMatrix matrix (dGetIdentityMatrix());
	for (const BaseEntity* ptr = this; ptr != root; ptr = ptr->GetParent()) {
		matrix = matrix * ptr->GetCurrentMatrix ();
	}
	return matrix;
}

dMatrix BaseEntity::CalculateInterpolatedGlobalMatrix (const BaseEntity* const root) const
{
	dMatrix matrix (dGetIdentityMatrix());
	for (const BaseEntity* ptr = this; ptr != root; ptr = ptr->GetParent()) {
		matrix = matrix * ptr->m_matrix;
	}
	return matrix;
}

void BaseEntity::SetMatrix(BaseEntityManager& world, const dQuaternion& rotation, const dVector& position)
{
	// read the data in a critical section to prevent race condition from other thread  
	world.Lock(m_lock);

	m_curPosition = m_nextPosition;
	m_curRotation = m_nextRotation;

	m_nextPosition = position;
	m_nextRotation = rotation;

	dFloat angle = m_curRotation.DotProduct(m_nextRotation);
	if (angle < 0.0f) {
		m_curRotation.Scale(-1.0f);
	}

	// release the critical section
	world.Unlock(m_lock);
}

void BaseEntity::SetNextMatrix (BaseEntityManager& world, const dQuaternion& rotation, const dVector& position)
{
	// read the data in a critical section to prevent race condition from other thread  
	world.Lock(m_lock);

	m_nextPosition = position;
	m_nextRotation = rotation;

	dFloat angle = m_curRotation.DotProduct(m_nextRotation);
	if (angle < 0.0f) {
		m_curRotation.Scale(-1.0f);
	}

	// release the critical section
	world.Unlock(m_lock);
}

void BaseEntity::ResetMatrix(BaseEntityManager& world, const dMatrix& matrix)
{
	m_initialMatrix = matrix;

	dQuaternion rot (matrix);
	SetMatrix(world, rot, matrix.m_posit);
	SetNextMatrix(world, rot, matrix.m_posit);
	InterpolateMatrix (world, 0.0f);
}

void BaseEntity::InterpolateMatrix (BaseEntityManager& world, dFloat param)
{
	// read the data in a critical section to prevent race condition from other thread  
	world.Lock(m_lock);

	dVector p0(m_curPosition);
	dVector p1(m_nextPosition);
	dQuaternion r0 (m_curRotation);
	dQuaternion r1 (m_nextRotation);

	// release the critical section
	world.Unlock(m_lock);

	dVector posit (p0 + (p1 - p0).Scale (param));
	dQuaternion rotation (r0.Slerp(r1, param));

	m_matrix = dMatrix (rotation, posit);

	if (m_userData) {
		m_userData->OnInterpolateMatrix(world, param);
	}
}


const dMatrix& BaseEntity::GetRenderMatrix () const
{
	return m_matrix;
}

void BaseEntity::TransformCallback(const NewtonBody* body, const dFloat* matrix, int threadIndex)
{
	// DONE: BaseEntity or IBody interface !!!
	BaseEntity* const ent = (BaseEntity*) NewtonBodyGetUserData(body);
	
	BaseEntityManager* const scene = (BaseEntityManager*) NewtonWorldGetUserData(NewtonBodyGetWorld(body));
	dMatrix transform (matrix);
	dMatrix transform2(matrix);
	transform2.m_posit = dVector(0.0f, 0.0f, 0.0f);
	transform2 = transform2 * ent->GetAlignmentMatrix();
	dQuaternion rot (transform2);

	//const float globalScaling = 1.0f / (float)scene->GetGlobalScale();
	//transform.m_posit.Scale( globalScaling );

	ent->SetMatrix (*scene, rot, transform.m_posit);

	/*
	if (ent->mBodyProxy != nullptr)
	{
		dMatrix nextMatrix(rot, transform.m_posit);
		ent->mBodyProxy->RecordTransform( scene->GetCurrPhysicsTimeSecs(),  &nextMatrix[0][0]);
	}
	*/
}

void BaseEntity::SaveState()
{
	m_initialMatrix = GetCurrentMatrix();
}

void BaseEntity::SaveState2(const dMatrix &matrix)
{
	m_initialMatrix = matrix;
}

void BaseEntity::RestoreState(BaseEntityManager& world, bool doInvalidateCache)
{
	ResetMatrix(world, m_initialMatrix);

	if (mBody)
	{
		
		NewtonBodySetMatrix(mBody, &m_initialMatrix[0][0]);
		
		//if (doInvalidateCache)
		//{
			const dFloat resetValues[4] = {0.0f, 0.0f, 0.0f, 0.0f};

			NewtonBodySetOmega (mBody, resetValues);
			NewtonBodySetVelocity (mBody, resetValues);
			NewtonBodySetForce (mBody, resetValues);
			NewtonBodySetTorque (mBody, resetValues);
		//}
	}
}

void BaseEntity::DrawDebug() const
{
}