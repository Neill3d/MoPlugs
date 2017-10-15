
#pragma once

#include "toolbox_stdafx.h"
#include "dBaseHierarchy.h"
#include "dList.h"
#include "..\Common_Physics\physics_common.h"
#include "CustomVehicleControllerManager.h"

//////////////////////

class BaseEntityManager;
class BaseEntity;
class CarEntity;
class CarVehicleControllerManager;

//////////////////////////////////////////////////////////
// BaseEntityManager
class BaseEntityManager : public dList <BaseEntity*>
{
public:

	//! a constructor
	BaseEntityManager(const double globalScale, const int numberOfThreads, const int physicsLoops, const float physicsFPS);

	//! a destructor
	virtual ~BaseEntityManager();

	void Clear();

	void Cleanup();
	void RemoveEntity (BaseEntity* const ent);
	void RemoveEntity (dList<BaseEntity*>::dListNode* const entNode);

	bool UpdatePhysics(float timestep);

	bool UpdatePhysics2(double time);

	double GetLastPhysicsTimeSecs();
	double	GetCurrPhysicsTimeSecs();

	double GetPhysicsTime();

	NewtonWorld* GetNewton() const;

	void Lock(unsigned& atomicLock);
	void Unlock(unsigned& atomicLock);

	void		ResetTimer();

	// 3 double values
	void			SetGravity(const double *gravity);
	const double	*GetGravity() const;

	void			SetGlobalScale(const double scale);
	const double	GetGlobalScale() const;

	// set default material physical parameters
	void			SetDefaultSoftness(const double value);
	const double	GetDefaultSoftness() const;
	
	void			SetDefaultElasticity(const double value);
	const double	GetDefaultElasticity() const;
	
	void			SetDefaultCollidable(const double value);
	const double	GetDefaultCollidable() const;
	
	void			SetDefaultFriction(const double staticValue, const double dynamicValue);
	const double	GetDefaultStaticFriction() const;
	const double	GetDefaultDynamicFriction() const;

	void SetDefaultMaterialParams(double softness, double elasticity, double collidable, double staticFriction, double dynamicFriciton);


	//
	void SaveState();
	void RestoreState();

	void DrawDebug() const;

	void Serialize(const char *filename);

	void ResetDefaultMaterialParams();


	dFloat CalculateInteplationParam () const;
	dFloat CalculateInteplationParam (double time) const;

private:

	NewtonWorld* m_world;

	double		mGlobalScale;				//!< all phys world global scale according to render world (Newton should have at least 0.1)
	double		mGravity[3];				//!< gravity force

	int			mNumberOfThreads;
	int			mPhysicsLoops;
	float		mPhysicsFPS;

	double		mDefaultSoftness;
	double		mDefaultElasticity;
	double		mDefaultCollidable;
	double		mDefaultStaticFriction;
	double		mDefaultDynamicFriction;

	bool m_physicsUpdate;
	bool m_reEntrantUpdate;

	unsigned64 m_microsecunds;
	double	m_physicsTime;
	double	m_lastPhysicsTime;
	double	m_animTime;

	dFloat m_physThreadTime;

	dFloat	m_mainThreadPhysicsTime;
	dFloat m_mainThreadGraphicsTime;

	void	UpdateDefaultMaterialParams();

	friend class dRuntimeProfiler;
};

// for simplicity we are not going to run the demo in a separate thread at this time
// this confuses many user int thinking it is more complex than it really is  
inline void BaseEntityManager::Lock(unsigned& atomicLock)
{
	while (NewtonAtomicSwap((int*)&atomicLock, 1)) {
		NewtonYield();
	}
}

inline void BaseEntityManager::Unlock(unsigned& atomicLock)
{
	NewtonAtomicSwap((int*)&atomicLock, 0);
}

inline NewtonWorld* BaseEntityManager::GetNewton() const
{
	return m_world;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                              __,  _,  _, __, __, _, _ ___ _ ___ , _    _, _,   _,  _,  _,
//                              |_) /_\ (_  |_  |_  |\ |  |  |  |  \ |   / ` |   /_\ (_  (_ 
//                              |_) | | , ) |   |   | \|  |  |  |   \|   \ , | , | | , ) , )
//                              ~   ~ ~  ~  ~~~ ~~~ ~  ~  ~  ~  ~    )    ~  ~~~ ~ ~  ~   ~ 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BaseEntity : public dHierarchy<BaseEntity>
{
public:

	class UserData
	{
		public:
		UserData()
		{
		}

		virtual ~UserData()
		{
		}

		virtual void OnRender (dFloat timestep) const = 0;
		virtual void OnInterpolateMatrix (BaseEntityManager& world, dFloat param) const = 0;
	};

public:

	//! a constructor
	BaseEntity(const BaseEntity& copyFrom);
	BaseEntity(const dMatrix& matrix, BaseEntity* const parent);
	//BaseEntity(BaseEntityManager& world, const dScene* const scene, dScene::dTreeNode* const rootSceneNode, dTree<DemoMeshInterface*, dScene::dTreeNode*>& meshCache, DemoEntityManager::EntityDictionary& entityDictionary, DemoEntity* const parent = NULL);
	virtual ~BaseEntity(void);

	virtual void DetachFromWorld(NewtonWorld * const pWorld);

	const dMatrix & GetAlignmentMatrix() const
	{
		return mAlignmentMatrix;
	}
	void SetAlignmentMatrix(const dMatrix &matrix)
	{
		mAlignmentMatrix = matrix;
	}

	void SetBodyProxy(PHYSICS_INTERFACE::IQueryGeometry		*bodyProxy);

	const void* GetMesh() const;
	void SetMesh (const void* const m_mesh, const dMatrix& meshMatrix);

	const dMatrix& GetMeshMatrix() const;  
	void SetMeshMatrix(const dMatrix& matrix);  

	NewtonBody *GetNewtonBody();
	void SetNewtonBody(NewtonBody * const pBody);

	UserData* GetUserData ();
	void SetUserData (UserData* const data, bool autoDelete);

	dBaseHierarchy* CreateClone () const;
	void LoadNGD_mesh (const char* const fileName, NewtonWorld* const world);

	const dMatrix& GetRenderMatrix () const;
	dMatrix CalculateGlobalMatrix (const BaseEntity* const root = NULL) const;

	dMatrix GetNextMatrix () const;
	dMatrix GetCurrentMatrix () const;
	virtual void SetMatrix(BaseEntityManager& world, const dQuaternion& rotation, const dVector& position);
	virtual void SetNextMatrix (BaseEntityManager& world, const dQuaternion& rotation, const dVector& position);

	virtual void ResetMatrix(BaseEntityManager& world, const dMatrix& matrix);
	virtual void InterpolateMatrix (BaseEntityManager& world, dFloat param);
	dMatrix CalculateInterpolatedGlobalMatrix (const BaseEntity* const root = NULL) const;

	//virtual void Render(dFloat timeStep, const BaseEntityManager* const scene) const;
	//virtual void SimulationPreListener(BaseEntityManager* const scene, BaseEntityManager::dListNode* const mynode, dFloat timeStep){};
	//virtual void SimulationPostListener(BaseEntityManager* const scene, BaseEntityManager::dListNode* const mynode, dFloat timeStep){};

	virtual void MessageHandler (NewtonBody* const sender, int message, void* const data) {}

	static void TransformCallback(const NewtonBody* body, const dFloat* matrix, int threadIndex);

	virtual void SaveState();
	virtual void SaveState2(const dMatrix &matrix);
	virtual void RestoreState(BaseEntityManager& world, bool doInvalidateCache);

	const double *GetMatrixd(BaseEntityManager& world)
	{
		dFloat param = world.CalculateInteplationParam();
		InterpolateMatrix(world, param);
		//dMatrix currMatrix = GetCurrentMatrix();
	
		dMatrix currMatrix = GetRenderMatrix();

		const float globalScaling = 1.0f / (float)world.GetGlobalScale();
		currMatrix.m_posit = currMatrix.m_posit.Scale( globalScaling );

		double *d = (double*) &m_dmatrix[0];

		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
			{
				*d = (double) currMatrix[i][j];
				d++;
			}

		return m_dmatrix;
	}

	virtual void DrawDebug() const;

protected:

	NewtonBody		*mBody;

	dMatrix		mAlignmentMatrix;

	PHYSICS_INTERFACE::IQueryGeometry		*mBodyProxy;

	mutable double m_dmatrix[16];
	mutable dMatrix m_matrix;			// interpolated matrix
	dVector m_curPosition;				// position one physics simulation step in the future
	dVector m_nextPosition;             // position at the current physics simulation step
	dQuaternion m_curRotation;          // rotation one physics simulation step in the future  
	dQuaternion m_nextRotation;         // rotation at the current physics simulation step  

	dMatrix m_meshMatrix;
	//DemoMeshInterface* m_mesh;
	const void		*m_mesh;		// pointer to FBModel mesh
	UserData	*m_userData;

	dMatrix m_initialMatrix;	// matrix for restoring state (snap position)

	bool		m_deleteUserData;

	unsigned m_lock;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CarEntity

class CarEntity : public BaseEntity
{
public:

	class TireAligmentTransform: public UserData
	{
		public: 
		TireAligmentTransform (const dMatrix& matrix)
			:UserData()
			,m_matrix( matrix)
		{
		}

		virtual void OnRender (dFloat timestep) const{};
		virtual void OnInterpolateMatrix (BaseEntityManager& world, dFloat param) const{};

		dMatrix m_matrix;
	};

public:

	//! a constructor
	CarEntity(	BaseEntityManager& world,
				CustomVehicleControllerManager *manager,
				const PHYSICS_INTERFACE::CarOptions *options,
				const PHYSICS_INTERFACE::IQueryGeometry *chassis, 
				const PHYSICS_INTERFACE::IQueryGeometry *wheelFL, 
				const PHYSICS_INTERFACE::IQueryGeometry *wheelFR, 
				const PHYSICS_INTERFACE::IQueryGeometry *wheelRL, 
				const PHYSICS_INTERFACE::IQueryGeometry *wheelRR,
				const PHYSICS_INTERFACE::IQueryPath *curve);

	//! a destructor
	virtual ~CarEntity();

	virtual void DetachFromWorld(NewtonWorld * const pWorld);

	bool	Init(BaseEntityManager& world, const PHYSICS_INTERFACE::CarOptions *options);

	virtual void	SaveState();
	virtual void	RestoreState(BaseEntityManager& world, bool doInvalidateCache);

	void	SetPlayerInput(const PHYSICS_INTERFACE::CarInput &input);
	void	SetPlayerInput(const double torque, const double clutch, const double steering, const double curveSteeringBlend, const double brake, const double handbrake, int gear);
	void	ApplyPlayerControl();
	void	ApplyNPCControl(const double manualSteeringValue, const double blendValue);

	dFloat CalculateNPCControlSteerinValue (dFloat searchRadius, dFloat distanceAhead, dFloat pathWidth, dFloat tangentMult, dFloat damping, const PHYSICS_INTERFACE::IQueryPath* const pathEntity);

	const double	GetSpeed();
	const double	GetRPM();
	const int		GetCurrentGear();

	virtual void DrawDebug() const;

	void UpdateOptions(const PHYSICS_INTERFACE::CarOptions *options);


	const double *GetWheelMatrixd(BaseEntityManager& world, const int wheel)
	{
		if (m_controller == nullptr)
			return nullptr;

		int i=0;
		for (dList<CustomVehicleController::BodyPartTire>::dListNode* node = m_controller->GetFirstTire(); node; node = m_controller->GetNextTire(node)) {
			const CustomVehicleController::BodyPartTire* const tire = &node->GetInfo();
			NewtonBody* const tireBody = tire->GetBody();
		
			if (i == wheel)
			{
				BaseEntity* const tirePart = (BaseEntity*) tire->GetUserData();
				return tirePart->GetMatrixd(world);
			}

			i++;
		}
		return nullptr;
	}

private:

	const PHYSICS_INTERFACE::IQueryGeometry		*mChassis;
	const PHYSICS_INTERFACE::IQueryGeometry		*mWheelFL;
	const PHYSICS_INTERFACE::IQueryGeometry		*mWheelFR;
	const PHYSICS_INTERFACE::IQueryGeometry		*mWheelRL;
	const PHYSICS_INTERFACE::IQueryGeometry		*mWheelRR;
	const PHYSICS_INTERFACE::IQueryPath			*mCurve;

	PHYSICS_INTERFACE::CarOptions		mOptions;
	PHYSICS_INTERFACE::CarInput			mInput;

	CustomVehicleController				*m_controller;
	CustomVehicleControllerManager		*mManager;

	dVector								m_debugTargetHeading;

	int									m_gearMap[10];

	NewtonCollision* CreateChassisCollision (NewtonWorld* const world, const dFloat scale) const;

	//CustomVehicleController::TireBodyState* AddTire (const CarInfo::Wheel &wheelInfo, dFloat width, dFloat radius);
	CustomVehicleController::BodyPartTire* AddTire (	const PHYSICS_INTERFACE::CarOptions::Wheel *pWheelPart,
													const PHYSICS_INTERFACE::IQueryGeometry *pWheelGeometry,
													const dVector& offset, 
													const dFloat globalScaling,
													dFloat width, 
													dFloat radius, 
													dFloat maxSteerAngle,
													dFloat mass, 
													dFloat suspensionLength, 
													dFloat suspensionSpring, 
													dFloat suspensionDamper, 
													dFloat lateralStiffness, 
													dFloat longitudinalStiffness, 
													dFloat aligningMomentTrail,
													int hasFender,
													CustomVehicleController::BodyPartTire::Info::SuspensionType suspensionType
													) ;

	void CalculateTireDimensions (NewtonWorld *const world, const PHYSICS_INTERFACE::IQueryGeometry *pWheelGeometry, const dFloat scale, const int wheelIndex, dFloat& width, dFloat& radius) const;

	void UpdateTireTransforms();

	// this function is an example of how to make a high performance super car
	void BuildCar (const PHYSICS_INTERFACE::CarOptions *pinfo);

	void SetGearMap(CustomVehicleController::EngineController* const engine);

	friend class CarVehicleControllerManager;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
//! Hardware based on Newton physics engine

class CarVehicleControllerManager : public CustomVehicleControllerManager
{
public:
	//! a constructor
	CarVehicleControllerManager(NewtonWorld* const world, int materialsCount, int* const materialList)
		: CustomVehicleControllerManager(world, materialsCount, materialList)
	{}

	void RenderVehicleSchematic (const int width, const int height, const CustomVehicleController* const controller) const;
	void DrawSchematicCallback (const CustomVehicleController* const controller, const char* const partName, dFloat value, int pointCount, const dVector* const lines) const;

	virtual void PreUpdate (dFloat timestep)
	{
		for (dListNode* ptr = GetFirst(); ptr; ptr = ptr->GetNext()) {
			CustomVehicleController* const controller = &ptr->GetInfo();
		
			NewtonBody* const body = controller->GetBody();
			CarEntity* const vehicleEntity = (CarEntity*) NewtonBodyGetUserData(body);

			// do player control
			vehicleEntity->ApplyPlayerControl ();
		}

		// do the base class post update
		CustomVehicleControllerManager::PreUpdate(timestep);
	}

	virtual void PostUpdate (dFloat timestep)
	{
		// do the base class post update
		CustomVehicleControllerManager::PostUpdate(timestep);

		// update the visual transformation matrices for all vehicle tires
		for (dListNode* node = GetFirst(); node; node = node->GetNext()) {
			CustomVehicleController* const controller = &node->GetInfo();
			CarEntity* const vehicleEntity = (CarEntity*)NewtonBodyGetUserData (controller->GetBody());
			vehicleEntity->UpdateTireTransforms();
		}
	}
};