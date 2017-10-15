
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Newton_CarEntity.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Newton_entities.h"
#include "Newton_hardware.h"

using namespace PHYSICS_INTERFACE;

#define		DEMO_GRAVITY		-20.0f

///////////////////////////////////////////////////////////////////////////////////////////////
// CarEntity

CarEntity::CarEntity(	BaseEntityManager& world,
						CustomVehicleControllerManager *manager,
						const CarOptions *options,
						const IQueryGeometry *chassis, 
						const IQueryGeometry *wheelFL, 
						const IQueryGeometry *wheelFR, 
						const IQueryGeometry *wheelRL, 
						const IQueryGeometry *wheelRR,
						const IQueryPath *curve)
	: BaseEntity(dGetIdentityMatrix(), nullptr)
	, mOptions(*options)
	, mManager(manager)
	, mChassis(chassis)
	, mWheelFL(wheelFL)
	, mWheelFR(wheelFR)
	, mWheelRL(wheelRL)
	, mWheelRR(wheelRR)
	, mCurve(curve)
{
	m_controller = nullptr;
	Init(world, options);
}

//! a destuctor
CarEntity::~CarEntity()
{
	if (m_controller && m_controller->GetBody())
	{
		BaseEntityManager* const scene = (BaseEntityManager*) NewtonWorldGetUserData(NewtonBodyGetWorld(m_controller->GetBody() ));
		if (scene != nullptr)
		{
			scene->RemoveEntity(this);
		}
	}

	//
	if (m_controller)
	{
		((CustomVehicleControllerManager*)m_controller->GetManager())->DestroyController(m_controller);
		m_controller = nullptr;
	}
}

void CarEntity::DetachFromWorld(NewtonWorld * const pWorld)
{
	if (m_controller)
	{
		((CustomVehicleControllerManager*)m_controller->GetManager())->DestroyController(m_controller);
		m_controller = nullptr;
	}
}

bool CarEntity::Init(BaseEntityManager& worldManager, const CarOptions *options)
{
	mOptions = *options;
/*
	void *pChassis = mOptions.pChassis;
	if (pChassis == nullptr) 
		return false;
	*/
	NewtonWorld* const world = worldManager.GetNewton();
	if (world == nullptr)
		return false;

	NewtonCollision* const chassisCollision = CreateChassisCollision (world, (dFloat)worldManager.GetGlobalScale() );
	if (chassisCollision == nullptr)
		return false;

	const float globalScaling = (float) worldManager.GetGlobalScale();
	
	dMatrix location; // (rot[0], rot[1], rot[2], pos);
	mChassis->GetMatrixTR( true, &location[0][0] );
	location.m_posit = location.m_posit.Scale( globalScaling );

	// create the vehicle controller
	dMatrix chassisMatrix;
	chassisMatrix.m_front = dVector (1.0, 0.0, 0.0, 0.0);			// this is the vehicle direction of travel
	chassisMatrix.m_up	  = dVector (0.0, 1.0, 0.0, 0.0);			// this is the downward vehicle direction
	chassisMatrix.m_right = chassisMatrix.m_front * chassisMatrix.m_up;	// this is in the side vehicle direction (the plane of the wheels)
	chassisMatrix.m_posit = dVector (0.0, 0.0, 0.0, 1.0);

	// create a default vehicle 
	m_controller = mManager->CreateVehicle (chassisCollision, chassisMatrix, (dFloat)mOptions.Mass, PhysicsApplyGravityForce, this); // dVector (0.0f, DEMO_GRAVITY, 0.0f, 0.0f));

	// get body from player
	NewtonBody* const body = m_controller->GetBody();

	// set the user data
	NewtonBodySetUserData(body, this);

	// set the transform callback
	NewtonBodySetTransformCallback (body, BaseEntity::TransformCallback);

	// set the standard force and torque call back
	//NewtonBodySetForceAndTorqueCallback(body, PhysicsApplyGravityForce);

	// set the player matrix 
	dMatrix setupLocation( dGetIdentityMatrix() );
	setupLocation.m_posit = location.m_posit;

	NewtonBodySetMatrix(body, &setupLocation[0][0]);

	// destroy the collision helper shape 
	NewtonDestroyCollision(chassisCollision);


	// build a muscle car from this vehicle controller
	BuildCar(options);
	
	SetBodyProxy( (IQueryGeometry*) mChassis);
	
	NewtonBodySetMatrix(body, &location[0][0]);
	
	dMatrix alignMatrix( location.Transpose() );
	alignMatrix.m_posit = dVector( (dFloat)0.0, (dFloat)0.0, (dFloat)0.0);
	mChassis->GetVisualAlignMatrixD( &alignMatrix[0][0] );
	SetAlignmentMatrix( alignMatrix );
	
	mBody = m_controller->GetBody();
	//SaveState();
	m_initialMatrix = location;
	RestoreState(worldManager, false);

	return true;
}

NewtonCollision* CarEntity::CreateChassisCollision (NewtonWorld* const world, const dFloat globalScaling) const
{
	if (mChassis == nullptr)
		return nullptr;

	dMatrix   localMatrix = dGetIdentityMatrix();
	
	//const double *modelScaling = mChassis->GetScale(true);

	int vertexCount = mChassis->GetVertexCount();
	const int stride = sizeof(dFloat) * 4;

	const float *posSrc = mChassis->GetVertexPosition(0);
	dFloat *posDst = new dFloat[vertexCount*4];

	for (int i=0; i<vertexCount; ++i)
	{
		posDst[i*4    ] = (dFloat)posSrc[i*4  ] * globalScaling; // * (float) modelScaling[0];
		posDst[i*4 + 1] = (dFloat)posSrc[i*4+1] * globalScaling; // * (float) modelScaling[1];
		posDst[i*4 + 2] = (dFloat)posSrc[i*4+2] * globalScaling; // * (float) modelScaling[2];
		posDst[i*4 + 3] = (dFloat) 1.0;
	}
	
	//
	NewtonCollision *result = NewtonCreateConvexHull(	world, 
									vertexCount, 
									&posDst[0], 
									stride, (dFloat)0.001, 
									0, 
									&localMatrix[0][0]);

	
	if (posDst)
	{
		delete [] posDst;
		posDst = nullptr;
	}

	return result;
}


void CarEntity::CalculateTireDimensions (NewtonWorld *const world, const IQueryGeometry *pWheelGeometry, const dFloat globalScaling, const int wheelIndex, dFloat& width, dFloat& radius) const
{
		
	// find the the tire visual mesh
	
	//const double *modelScaling = pWheelGeometry->GetScale(true);

	const int vertCount = pWheelGeometry->GetVertexCount();
	const int stride = 4 * sizeof(dFloat);
	const float *posSrc = pWheelGeometry->GetVertexPosition(0);

	dFloat *posDst = new dFloat[vertCount * 4];

	for (int i=0; i<vertCount; ++i)
	{
		posDst[i*4] = (dFloat)posSrc[i*4] * globalScaling; // * (float) modelScaling[0];
		posDst[i*4+1] = (dFloat)posSrc[i*4+1] * globalScaling; // * (float) modelScaling[1];
		posDst[i*4+2] = (dFloat)posSrc[i*4+2] * globalScaling; // * (float) modelScaling[2];
		posDst[i*4+3] = (dFloat)1.0;
	}



	NewtonCollision *collision = NewtonCreateConvexHull(	world, 
															vertCount,
															&posDst[0], // point cloud
															stride,  // stride
															(dFloat)0.0, 
															0, 
															nullptr);

	if (posDst)
	{
		delete [] posDst;
		posDst = nullptr;
	}

	// TODO: get the location of this tire relative to the car chassis

	/*
	FBMatrix m;
	pModel->GetMatrix(m);

	m[12] *= scale;
	m[13] *= scale;
	m[14] *= scale;
	*/

	//pWheelGeometry->PrepMatrix(true);

	//dQuaternion quat;
	//dVector		rot(0.0f, 0.0f, 0.0f);
	//dVector		vec(0.0f, 0.0f, 0.0f);

	//pWheelGeometry->GetPositionF( &vec[0] );
	//pWheelGeometry->GetRotationF( &rot[0] );
	//pWheelGeometry->GetQuaternionF( &quat.m_q0 );

	dMatrix tireMatrix; // ( rot[0], rot[1], rot[2], vec );
	pWheelGeometry->GetMatrixTR(false, &tireMatrix[0][0] );
	//tireMatrix.m_posit = tireMatrix.m_posit.Scale(globalScaling);
	tireMatrix.m_posit = dVector(0.0, 0.0, 0.0, 1.0);

	//dMatrix tireMatrix (tirePart->CalcGlobalMatrix() * carModel->m_matrix);
	//		dMatrix tireMatrix (tirePart->CalcGlobalMatrix());

	dVector extremePoint;
	// fin the support points tha can be use to define the tire collision mesh
	dVector upDir (tireMatrix.UnrotateVector(dVector (0.0, 1.0, 0.0, 0.0)));
	NewtonCollisionSupportVertex (collision, &upDir[0], &extremePoint[0]);
	radius = dAbs(upDir % extremePoint);

	//dVector withdDir (tireMatrix.UnrotateVector(carModel->m_matrix.m_right));
	dVector widthDir (tireMatrix.UnrotateVector(tireMatrix.m_front));
	NewtonCollisionSupportVertex (collision, &widthDir[0], &extremePoint[0]);
	width = widthDir % extremePoint;

	widthDir = widthDir.Scale (-1.0);
	NewtonCollisionSupportVertex (collision, &widthDir[0], &extremePoint[0]);
	width += widthDir % extremePoint;

	NewtonDestroyCollision( collision );

	//radius = 60.0f;
	//witdh = 30.0f;
	//witdh *= 0.4f;	
}


CustomVehicleController::BodyPartTire*  CarEntity::AddTire (	const CarOptions::Wheel *pWheelPart, 
															const IQueryGeometry *pWheelGeometry,
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
															) 
{
	NewtonBody* const body = m_controller->GetBody();

	// the tire is located at position of the tire mesh relative to the chassis mesh
	// TODO: matrix should come from local wheel positoin
	//pWheelGeometry->PrepMatrix(false);

	//dQuaternion quat;
	//dVector		rot(0.0f, 0.0f, 0.0f);
	//dVector		vec(0.0f, 0.0f, 0.0f);

	//pWheelGeometry->GetPositionF( &vec[0] );
	//pWheelGeometry->GetRotationF( &rot[0] );
	//pWheelGeometry->GetQuaternionF( &quat.m_q0 );

	dMatrix chassisMatrix, tireMatrix; // ( rot[0], rot[1], rot[2], vec ); // mInitialMatrix[0] );
	//mChassis->GetMatrixTR_f( true, &chassisMatrix[0][0] );
	pWheelGeometry->GetMatrixTR( false, &tireMatrix[0][0] );
	//tireMatrix = dGetIdentityMatrix();
	//tireMatrix = chassisMatrix.Inverse() * tireMatrix;

	// add the offset location

	mChassis->PrepMatrix(true);
	dVector modelScaling;
	mChassis->GetScaleD(&modelScaling[0]);
	/*
	tireMatrix.m_posit.m_x *= 1.0f / (float) modelScaling.m_x;
	tireMatrix.m_posit.m_y *= 1.0f / (float) modelScaling.m_y;
	tireMatrix.m_posit.m_z *= 1.0f / (float) modelScaling.m_z;
	*/
	
	tireMatrix.m_posit = tireMatrix.m_posit.Scale( globalScaling );
	
	tireMatrix.m_posit += offset;
	//tireMatrix.m_posit.m_y = 0.0f;
	// lower the tire base position by some distance
	tireMatrix.m_posit.m_y -= suspensionLength * 0.25;

	
	
	// add an alignment matrix, to match visual mesh to physics collision shape
	dMatrix aligmentMatrix (dGetIdentityMatrix());
	
	if (tireMatrix[0][2] < 0.0) {
		//aligmentMatrix = dYawMatrix(3.141592f);
		aligmentMatrix = dPitchMatrix(3.141592);
	}
	
	// add the visual representation of the is tire to as a child of the vehicle model 
	NewtonCollision*  const tireMeshGenerator = NewtonCreateChamferCylinder (NewtonBodyGetWorld(body), 0.5, 1.0, 0, NULL);
	NewtonCollisionSetScale (tireMeshGenerator, width, radius, radius);

	
/*
	DemoEntity* const tireEntity = new DemoEntity (tireMatrix, this);
	DemoMesh* const visualMesh = new DemoMesh ("tireMesh", tireMeshGenerator, "smilli.tga", "smilli.tga", "smilli.tga");
	tireEntity->SetMesh (visualMesh, dYawMatrix(3.141592f * 90.0f / 180.0f));
	visualMesh->Release();
	*/
	BaseEntity * const tireEntity = new BaseEntity(tireMatrix, this);
	tireEntity->SetUserData( new TireAligmentTransform(aligmentMatrix), true );
	tireEntity->SetBodyProxy( (IQueryGeometry*) pWheelGeometry );
	NewtonDestroyCollision (tireMeshGenerator);

	// add the tire to the vehicle
	CustomVehicleController::BodyPartTire::Info tireInfo;
	tireInfo.m_location = tireMatrix.m_posit;
	tireInfo.m_mass = mass;
	tireInfo.m_radio = radius;
	tireInfo.m_width = width;
	tireInfo.m_maxSteeringAngle = maxSteerAngle * 3.1416 / 180.0;
	tireInfo.m_dampingRatio = suspensionDamper;
	tireInfo.m_springStrength = suspensionSpring;
	tireInfo.m_suspesionlenght = suspensionLength;
	tireInfo.m_lateralStiffness = dAbs(lateralStiffness);
	tireInfo.m_longitudialStiffness = dAbs(longitudinalStiffness);
	tireInfo.m_aligningMomentTrail = aligningMomentTrail;

	// TODO: add options for next values
	tireInfo.m_hasFender = hasFender; // definition.WHEEL_HAS_FENDER;
	tireInfo.m_suspentionType = suspensionType; // definition.TIRE_SUSPENSION_TYPE;

	// DONE: add BaseEntity as a userdata
	tireInfo.m_userData = tireEntity;

	return m_controller->AddTire (tireInfo);
}

void CarEntity::UpdateOptions(const PHYSICS_INTERFACE::CarOptions *options)
{
	mOptions = *options;

	if (m_controller == nullptr)
		return;

	//
	BaseEntityManager* const scene = (BaseEntityManager*) NewtonWorldGetUserData(NewtonBodyGetWorld(m_controller->GetBody()));
	const dFloat globalScaling = (dFloat) scene->GetGlobalScale();

	//mCarInfo->MassXOffset *= scale;
	//mCarInfo->MassYOffset *= scale;

	// Muscle cars have the front engine, we need to shift the center of mass to the front to represent that
	
	m_controller->SetCenterOfGravity (dVector ( (dFloat)mOptions.MassXOffset * globalScaling, (dFloat)mOptions.MassYOffset * globalScaling, 0.0, 0.0)); 
	//m_controller->SetAerodynamicsDownforceCoefficient(maxDownforce, mOptions.vehicleTopSpeedKPH);
	CustomVehicleController::SteeringController* const steering = m_controller->GetSteering();
	//steering->SetParam((dFloat)mOptions.SteerAngle * 3.141592 / 180.0);

	// TIRES properties
	int i = 0;
	for (dList<CustomVehicleController::BodyPartTire>::dListNode* node = m_controller->GetFirstTire(); node; node = m_controller->GetNextTire(node)) {
		CustomVehicleController::BodyPartTire* const part = &node->GetInfo();	
		BaseEntity* const tirePart = (BaseEntity*) part->GetUserData();

		CustomVehicleController::BodyPartTire::Info tireInfo;
		tireInfo = part->GetInfo();

			//CustomVehicleControllerBodyStateTire::TireCreationInfo	tireInfo;
			//tire->GetInfo(tireInfo);

		auto &srcInfo = options->wheels[i];

		tireInfo.m_aligningMomentTrail = srcInfo.aligningMomentTrail;
		tireInfo.m_lateralStiffness = srcInfo.lateralStiffness;
		tireInfo.m_longitudialStiffness = srcInfo.longitudinalStiffness;

		tireInfo.m_mass = srcInfo.Mass;

		tireInfo.m_suspesionlenght = srcInfo.SuspensionLength;
		tireInfo.m_springStrength = srcInfo.SuspensionSpring;
		tireInfo.m_dampingRatio = srcInfo.SuspensionDamper;

		//tire->UpdateInfo(tireInfo);
		part->SetInfo(tireInfo);

		i++;
	}
	// BRAKES
	CustomVehicleController::BrakeController* const brakes = m_controller->GetBrakes();
	brakes->SetParam( (dFloat) mOptions.TireBrakeTorque);
	CustomVehicleController::BrakeController* const handBrakes = m_controller->GetHandBrakes();
	handBrakes->SetParam( (dFloat) mOptions.TireHandBrakeTorque);

	// ENGINE
	
	CustomVehicleController::EngineController* const engineControl = m_controller->GetEngine();

	engineControl->SetTransmissionMode( mOptions.AutomaticGear );
	engineControl->SetDifferentialLock( mOptions.differentialLock );
	/*
	dFloat viperIdleRPM = (dFloat) mOptions.IdleRPM;
	dFloat viperIdleTorquePoundPerFoot = (dFloat) mOptions.IdleTorquePoundPerFoot;

	dFloat viperPeakTorqueRPM = (dFloat) mOptions.PeakTorqueRPM;
	dFloat viperPeakTorquePoundPerFoot = (dFloat) mOptions.PeakTorquePoundPerFoot;

	dFloat viperPeakHorsePowerRPM = (dFloat) mOptions.PeakHorsePowerRPM;
	dFloat viperPeakHorsePower = (dFloat) mOptions.PeakHorsePower;

	dFloat viperRedLineRPM = (dFloat) mOptions.RedLineRPM;
	dFloat viperRedLineTorquePoundPerFoot = (dFloat) mOptions.RedLineTorquePoundPerFoot;

	dFloat vehicleTopSpeedKPH = (dFloat) mOptions.vehicleTopSpeedKPH;
	engine->InitEngineTorqueCurve (vehicleTopSpeedKPH, viperIdleTorquePoundPerFoot, viperIdleRPM, viperPeakTorquePoundPerFoot, viperPeakTorqueRPM, viperPeakHorsePower, viperPeakHorsePowerRPM, viperRedLineTorquePoundPerFoot, viperRedLineRPM);
	*/

	// do not forget to call finalize after all components are added or after any change is made to the vehicle
	m_controller->Finalize();
}

void CarEntity::SetGearMap(CustomVehicleController::EngineController* const engine)
	{
		int start = engine->GetFirstGear();
		int count = engine->GetLastGear() - start;
		for (int i = 0; i < count; i++) {
			m_gearMap[i + 2] = start + i;
		}
		m_gearMap[0] = engine->GetNeutralGear();
		m_gearMap[1] = engine->GetReverseGear();
	}

// this function is an example of how to make a high performance super car
void CarEntity::BuildCar (const CarOptions *pinfo)
{
	NewtonBody* const body = m_controller->GetBody();
	BaseEntityManager* const scene = (BaseEntityManager*) NewtonWorldGetUserData(NewtonBodyGetWorld(body));
	const dFloat globalScaling = (dFloat) scene->GetGlobalScale();

	// step one: find the location of each tire, in the visual mesh and add them one by one to the vehicle controller 
	dFloat width = 1.0;
	dFloat radius = 1.0;

	// Muscle cars have the front engine, we need to shift the center of mass to the front to represent that
	
	// DONE: add a scale factor here for MassXOffset, YOffset !!!
	
	m_controller->SetCenterOfGravity (dVector ((dFloat)pinfo->MassXOffset * globalScaling, (dFloat)pinfo->MassYOffset * globalScaling, 0.0, 0.0)); 
	//m_controller->SetCenterOfGravity( dVector(0.0f, 0.0f, 0.0f, 0.0f) );
	
	// a car may have different size front an rear tire, therefore we do this separate for front and rear tires
	CalculateTireDimensions (scene->GetNewton(), mWheelFL, (dFloat)scene->GetGlobalScale(), 0, width, radius);
	dVector offset (0.0, 0.0, 0.0, 0.0);
	const CarOptions::Wheel *pwheel = &pinfo->wheels[0];
	// TODO: FRONT_AXEL_TIRE_STEER_ANGLE define in options
	CustomVehicleController::BodyPartTire* const leftFrontTire = AddTire (pwheel, mWheelFL, offset, globalScaling, width, radius, 
		(dFloat)pinfo->FrontSteerAngle, (dFloat)pwheel->Mass, (dFloat)pwheel->SuspensionLength, (dFloat)pwheel->SuspensionSpring, 
		(dFloat)pwheel->SuspensionDamper, (dFloat)pwheel->lateralStiffness, (dFloat)pwheel->longitudinalStiffness, (dFloat)pwheel->aligningMomentTrail,
		pinfo->WheelhasFender, (CustomVehicleController::BodyPartTire::Info::SuspensionType) pinfo->TireSuspensionType);
	pwheel = &pinfo->wheels[1];
	CustomVehicleController::BodyPartTire* const rightFrontTire = AddTire (pwheel, mWheelFR, offset, globalScaling, width, radius, 
		(dFloat)pinfo->FrontSteerAngle, (dFloat)pwheel->Mass, (dFloat)pwheel->SuspensionLength, (dFloat)pwheel->SuspensionSpring, 
		(dFloat)pwheel->SuspensionDamper, (dFloat)pwheel->lateralStiffness, (dFloat)pwheel->longitudinalStiffness, (dFloat)pwheel->aligningMomentTrail,
		pinfo->WheelhasFender, (CustomVehicleController::BodyPartTire::Info::SuspensionType) pinfo->TireSuspensionType);

	// add real tires
	CalculateTireDimensions (scene->GetNewton(), mWheelRL, (dFloat)scene->GetGlobalScale(), 0, width, radius);
	dVector offset1 (0.0, 0.05, 0.0, 0.0);
	pwheel = &pinfo->wheels[2];
	// TODO: REAR_AXEL_TIRE_STEER_ANGLE define in options
	CustomVehicleController::BodyPartTire* const leftRearTire = AddTire (pwheel, mWheelRL, offset1, globalScaling, width, radius, 
		(dFloat)pinfo->RearSteerAngle, (dFloat)pwheel->Mass, (dFloat)pwheel->SuspensionLength, (dFloat)pwheel->SuspensionSpring, 
		(dFloat)pwheel->SuspensionDamper, (dFloat)pwheel->lateralStiffness, (dFloat)pwheel->longitudinalStiffness, (dFloat)pwheel->aligningMomentTrail,
		pinfo->WheelhasFender, (CustomVehicleController::BodyPartTire::Info::SuspensionType) pinfo->TireSuspensionType);
	pwheel = &pinfo->wheels[3];
	CustomVehicleController::BodyPartTire* const rightRearTire = AddTire (pwheel, mWheelRR, offset1, globalScaling, width, radius, 
		(dFloat)pinfo->RearSteerAngle, (dFloat)pwheel->Mass, (dFloat)pwheel->SuspensionLength, (dFloat)pwheel->SuspensionSpring, 
		(dFloat)pwheel->SuspensionDamper, (dFloat)pwheel->lateralStiffness, (dFloat)pwheel->longitudinalStiffness, (dFloat)pwheel->aligningMomentTrail,
		pinfo->WheelhasFender, (CustomVehicleController::BodyPartTire::Info::SuspensionType) pinfo->TireSuspensionType);
	
	//calculate the Ackerman parameters
		
	
	// add a steering Wheel component
	CustomVehicleController::SteeringController* const steering = new CustomVehicleController::SteeringController (m_controller);
	steering->AddTire(leftFrontTire);
	steering->AddTire(rightFrontTire);
	steering->AddTire(leftRearTire);
	steering->AddTire(rightRearTire);
	//steering->CalculateAkermanParameters (leftRearTire, rightRearTire, leftFrontTire, rightFrontTire);
	m_controller->SetSteering(steering);
	

	// add vehicle brakes
	CustomVehicleController::BrakeController* const brakes = new CustomVehicleController::BrakeController (m_controller, (dFloat)pinfo->TireBrakeTorque);
	brakes->AddTire (leftFrontTire);
	brakes->AddTire (rightFrontTire);
	brakes->AddTire (leftRearTire);
	brakes->AddTire (rightRearTire);
	m_controller->SetBrakes(brakes);

	// add vehicle hand brakes
	CustomVehicleController::BrakeController* const handBrakes = new CustomVehicleController::BrakeController (m_controller, (dFloat)pinfo->TireHandBrakeTorque);
	handBrakes->AddTire (leftRearTire);
	handBrakes->AddTire (rightRearTire);
	m_controller->SetHandBrakes(handBrakes);

	// add the engine, differential and transmission 
	CustomVehicleController::EngineController::Info engineInfo;
	engineInfo.m_mass = pinfo->EngineMass;			// TODO: add option for the ENGINE_MASS 
	engineInfo.m_radio = pinfo->EngineRadio;				// TODO: add option for the ENGINE_RADIO; 
	engineInfo.m_vehicleTopSpeed = pinfo->vehicleTopSpeedKPH; // definition.VEHICLE_TOP_SPEED_KMH;
	engineInfo.m_clutchFrictionTorque = pinfo->ClutchFrictionTorque;	// definition.CLUTCH_FRICTION_TORQUE;

	engineInfo.m_idleTorque = (dFloat) pinfo->IdleTorquePoundPerFoot; // definition.IDLE_TORQUE;
	engineInfo.m_idleTorqueRpm = (dFloat) pinfo->IdleRPM; // definition.IDLE_TORQUE_RPM;
	engineInfo.m_peakTorque = (dFloat) pinfo->PeakTorquePoundPerFoot; // definition.PEAK_TORQUE;
	engineInfo.m_peakTorqueRpm = (dFloat) pinfo->PeakTorqueRPM; // definition.PEAK_TORQUE_RPM;
	engineInfo.m_peakHorsePower = (dFloat) pinfo->PeakHorsePower; // definition.PEAK_HP;
	engineInfo.m_peakHorsePowerRpm = (dFloat) pinfo->PeakHorsePowerRPM; // definition.PEAK_HP_RPM;
	engineInfo.m_readLineRpm = (dFloat) pinfo->RedLineRPM; // definition.REDLINE_RPM;

	engineInfo.m_gearsCount = pinfo->GearCount;
	engineInfo.m_gearRatios[0] = pinfo->gear1; // definition.TIRE_GEAR_1;
	engineInfo.m_gearRatios[1] = pinfo->gear2; // definition.TIRE_GEAR_2;
	engineInfo.m_gearRatios[2] = pinfo->gear3; // definition.TIRE_GEAR_3;
	engineInfo.m_gearRatios[3] = pinfo->gear4; // definition.TIRE_GEAR_4;
	engineInfo.m_gearRatios[4] = pinfo->gear5; // definition.TIRE_GEAR_5;
	engineInfo.m_gearRatios[5] = pinfo->gear6; // definition.TIRE_GEAR_6;
	engineInfo.m_reverseGearRatio = pinfo->GearReverseRatio; // definition.TIRE_REVERSE_GEAR;

	CustomVehicleController::EngineController::Differential4wd differential;
	switch (pinfo->type) // definition.DIFFERENTIAL_TYPE
	{
		case kCarDriveRearWheels:
			differential.m_type = CustomVehicleController::EngineController::Differential::m_2wd;
			differential.m_axel.m_leftTire = leftRearTire;
			differential.m_axel.m_rightTire = rightRearTire;
			break;
		case kCarDriveFrontWheels:
			differential.m_type = CustomVehicleController::EngineController::Differential::m_2wd;
			differential.m_axel.m_leftTire = leftFrontTire;
			differential.m_axel.m_rightTire = rightFrontTire;
			break;

		default:
			differential.m_type = CustomVehicleController::EngineController::Differential::m_4wd;
			differential.m_axel.m_leftTire = leftRearTire;
			differential.m_axel.m_rightTire = rightRearTire;
			differential.m_secundAxel.m_axel.m_leftTire = leftFrontTire;
			differential.m_secundAxel.m_axel.m_rightTire = rightFrontTire;
	}

	engineInfo.m_differentialLock = pinfo->differentialLock;
	engineInfo.m_userData = this;
	CustomVehicleController::EngineController* const engineControl = new CustomVehicleController::EngineController (m_controller, engineInfo, differential);

	// the the default transmission type
	engineControl->SetTransmissionMode(true);	// TODO: m_automaticTransmission.GetPushButtonState()
	engineControl->SetIgnition(true);

	m_controller->SetEngine(engineControl);

	// trace the engine curve
	//engineControl->PlotEngineCurve ();

	// set the gear look up table
	SetGearMap(engineControl);

	// set the vehicle weigh distribution 
	m_controller->SetWeightDistribution (pinfo->WeightDistribution); // definition.VEHICLE_WEIGHT_DISTRIBUTION

	// set the vehicle aerodynamics
	dFloat weightRatio0 = pinfo->DownforceWeightFactor0;	// DOWNFORCE_WEIGHT_FACTOR_0
	dFloat weightRatio1 = pinfo->DownforceWeightFactor1; // definition.DOWNFORCE_WEIGHT_FACTOR_1;
	dFloat speedFactor = pinfo->DownforceWeightFactorSpeed / pinfo->vehicleTopSpeedKPH; //definition.DOWNFORCE_WEIGHT_FACTOR_SPEED / definition.VEHICLE_TOP_SPEED_KMH;
	m_controller->SetAerodynamicsDownforceCoefficient(DEMO_GRAVITY, weightRatio0, speedFactor, weightRatio1);

	// do not forget to call finalize after all components are added or after any change is made to the vehicle
	m_controller->Finalize();
}


void CarEntity::SetPlayerInput(const CarInput &input)
{
	mInput = input;
}

void CarEntity::SetPlayerInput(const double torque, const double clutch, const double steering, const double curveSteeringBlend, const double brake, const double handbrake, int gear)
{
	mInput.engineGasPedal = (float) torque;
	mInput.engineClutchPedal = (float) clutch;
	mInput.steeringVal = (float) steering;
	mInput.curveSteeringBlendVal = (float) curveSteeringBlend;
	mInput.brakePedal = (float) brake;
	mInput.handBrakePedal = (float) handbrake;
	mInput.gear = gear;
}

// based on the work of Craig Reynolds http://www.red3d.com/cwr/steer/
dFloat CarEntity::CalculateNPCControlSteerinValue (dFloat searchRadius, dFloat distanceAhead, dFloat pathWidth, dFloat tangentMult, dFloat damping, const IQueryPath* const path)
{
	const CustomVehicleController::BodyPart& chassis = *m_controller->GetChassis();
	//CustomVehicleControllerComponentSteering* const steering = m_controller->GetSteering();

	CustomVehicleController::SteeringController* const steering = m_controller->GetSteering();

	/*
	dVector veloc (chassis.GetVelocity());
	dVector lookAheadPoint(0.0f, 0.0f, 0.0f);
	if ((veloc % veloc) > 0.02f) {
		lookAheadPoint = (veloc.Scale (distanceAhead / dSqrt (veloc % veloc)));
	}
	*/

	dMatrix matrix;
	dVector veloc(0.0);
	NewtonBody* const body = chassis.GetBody();
	NewtonBodyGetVelocity(body, &veloc[0]);
	NewtonBodyGetMatrix(body, &matrix[0][0]);
	//dVector lookAheadPoint (veloc.Scale (distanceAhead / dSqrt (veloc % veloc)));
	dVector lookAheadPoint(0.0, 0.0, 0.0);
	if ((veloc % veloc) > 0.02) {
		lookAheadPoint = (veloc.Scale (distanceAhead / dSqrt (veloc % veloc)));
	}
	 
	// find the closet point to the past on the spline
	dMatrix vehicleMatrix (m_controller->GetLocalFrame() * matrix);
	//dMatrix pathMatrix (pathEntity->GetMeshMatrix() * pathEntity->GetCurrentMatrix());
	dMatrix pathMatrix;
	path->GetCurrentMatrixD( &pathMatrix[0][0] );
	vehicleMatrix = GetCurrentMatrix();
	dVector p0 (vehicleMatrix.m_posit + lookAheadPoint);
	p0.m_y = 0.0f;
		
	dBigVector q; 
	//DemoBezierCurve* const path = (DemoBezierCurve*) pathEntity->GetMesh();
	dBigVector untransformV = pathMatrix.UntransformVector(p0);
	
	dFloat64 u = path->FindClosestKnot ( &q[0], &untransformV[0], searchRadius, 4);
	//return 0.0f;
	dVector p1 (pathMatrix.TransformVector(dVector (q.m_x, q.m_y, q.m_z, q.m_w)));
	p1.m_y = 0.0f;
	dVector dist (p1 - p0);
	dFloat angle = 0.0f;
	//dFloat maxAngle = steering->GetMaxSteeringAngle ();
	dFloat maxAngle = 20.0f * 3.1314f / 180.0f;
	if ((dist % dist) < (pathWidth * pathWidth)) {
		dBigVector averageTangent (0.0f, 0.0f, 0.0f, 0.0f);
		for(int i = 0; i < 4; i ++) {
			dBigVector tangent;
			path->CurveDerivative (&tangent[0], u);
			tangent = tangent.Scale (tangentMult / dSqrt (tangent % tangent));
			averageTangent += tangent;
			q += tangent.Scale (1.0f);
			untransformV = q;
			u = path->FindClosestKnot ( &q[0], &untransformV[0], searchRadius, 4);
		}
		averageTangent = averageTangent.Scale (1.0f / dSqrt (averageTangent % averageTangent));
		dVector heading (pathMatrix.RotateVector(dVector (averageTangent.m_x, averageTangent.m_y, averageTangent.m_z, averageTangent.m_w)));
		heading.m_y = 0.0;
		heading = vehicleMatrix.UnrotateVector(heading);
		angle = dClamp (dAtan2 (heading.m_z, heading.m_x), -maxAngle, maxAngle);
	} else {

		// find a point in the past at some distance ahead
		for(int i = 0; i < 5; i ++) {
			dBigVector tangent;
			path->CurveDerivative (&tangent[0], u);
			q += tangent.Scale (tangentMult / dSqrt (tangent % tangent));
			untransformV = q;
			path->FindClosestKnot (&q[0], &untransformV[0], searchRadius, 4);
		}

		m_debugTargetHeading = pathMatrix.TransformVector(dVector (q.m_x, q.m_y, q.m_z, q.m_w));
		dVector localDir (vehicleMatrix.UntransformVector(m_debugTargetHeading));
		angle = dClamp (dAtan2 (localDir.m_z, localDir.m_x), -maxAngle, maxAngle);
	}
	dFloat param = steering->GetParam();
	return param + (-angle / maxAngle - param) * damping;
}

void CarEntity::ApplyNPCControl(const double manualSteeringValue, const double blendValue)
{
	CustomVehicleController::EngineController* const engine = m_controller->GetEngine();
		CustomVehicleController::SteeringController* const steering = m_controller->GetSteering();
		//CustomVehicleController::ClutchController* const clutch = m_controller->GetClutch();
		CustomVehicleController::BrakeController* const brakes = m_controller->GetBrakes();
		CustomVehicleController::BrakeController* const handBrakes = m_controller->GetHandBrakes();
		if (!engine) {
			return;
		}
	
	if (mCurve != nullptr)
	{
		auto &params = mOptions.steerCurveParams;
		dFloat steeringParam = CalculateNPCControlSteerinValue ((float) params.searchRadius,
																(float)params.distanceAhead, 
																(float)params.pathWidth,
																(float) params.tangentMult,
																(float) params.damping, 
																mCurve);

		dFloat lsteering = (dFloat) manualSteeringValue;
		dFloat lblend = (dFloat) blendValue;

		steering->SetParam (steeringParam * lblend + lsteering * ( (dFloat)1.0 - lblend) );
	}
}

void CarEntity::ApplyPlayerControl()
{
//	NewtonBody* const body = m_controller->GetBody();
//	NewtonWorld* const world = NewtonBodyGetWorld(body);
	
	CustomVehicleController::EngineController* const engine = m_controller->GetEngine();
	CustomVehicleController::BrakeController* const brakes = m_controller->GetBrakes();
	CustomVehicleController::BrakeController* const handBrakes = m_controller->GetHandBrakes();
	CustomVehicleController::SteeringController* const steering = m_controller->GetSteering();

	dAssert (engine);
	dAssert (brakes);
	dAssert (steering);
	dAssert (handBrakes);

	// TODO: add manual gear mode!

	/*
	engine->SetTransmissionMode(transmissionMode);
	if (!engine->GetTransmissionMode() )
	{
		engine->SetGear(gear);
	}
	*/

	if (mCurve!=nullptr && mOptions.UseCurveForSteering)
	{
		ApplyNPCControl(mInput.steeringVal, mInput.curveSteeringBlendVal);
	}
	else
	{
		steering->SetParam(mInput.steeringVal);
	}

	bool transmissionMode = engine->GetTransmissionMode();

	if (!transmissionMode) 
	{
		if (mInput.gear != engine->GetGear() && mInput.gear >= 0 && mInput.gear <= engine->GetLastGear() )
		{
			engine->SetGear(mInput.gear);
		}
	}
	else
	{
		if (mInput.gear != engine->GetGear() 
			&& (mInput.gear == engine->GetReverseGear() || mInput.gear == engine->GetNeutralGear() || mInput.gear == engine->GetFirstGear()) )
		{
			engine->SetGear(mInput.gear);
		}
	}


	brakes->SetParam(mInput.brakePedal);
	engine->SetParam(mInput.engineGasPedal);
	engine->SetClutchParam(mInput.engineClutchPedal);
	handBrakes->SetParam(mInput.handBrakePedal);
}

void CarEntity::DrawDebug () const 
{
	if (m_controller == nullptr)
		return;

	dMatrix matrix;
	dVector com(0.0f);
	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;
	dFloat mass;

	const CustomVehicleController::BodyPart* const chassis = m_controller->GetChassis ();
	NewtonBody* const chassisBody = chassis->GetBody();
		
	NewtonBodyGetCentreOfMass(chassisBody, &com[0]);
	NewtonBodyGetMass(chassisBody, &mass, &Ixx, &Iyy, &Izz);
	NewtonBodyGetMatrix(chassisBody, &matrix[0][0]);
	matrix.m_posit = matrix.TransformVector(com);
	matrix = m_controller->GetLocalFrame() * matrix;

	dFloat scale = -4.0f / (mass * DEMO_GRAVITY);
	dVector p0 (matrix.m_posit);

	glDisable (GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glLineWidth(3.0f);
	glBegin(GL_LINES);

	// draw vehicle weight at the center of mass
	dFloat lenght = scale * mass * DEMO_GRAVITY;
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f (p0.m_x, p0.m_y, p0.m_z);
	glVertex3f (p0.m_x, p0.m_y - lenght, p0.m_z);

	// draw vehicle front dir
	glColor3f(1.0f, 1.0f, 1.0f);
	dVector r0 (p0 + matrix[1].Scale (1.0f));
	dVector r1 (r0 + matrix[0].Scale (2.0f));
	glVertex3f (r0.m_x, r0.m_y, r0.m_z);
	glVertex3f (r1.m_x, r1.m_y, r1.m_z);

	// draw the velocity vector, a little higher so that is not hidden by the vehicle mesh 
	dVector veloc(0.0f);
	NewtonBodyGetVelocity(chassisBody, &veloc[0]);
	dVector q0 (p0 + matrix[1].Scale (1.0f));
	dVector q1 (q0 + veloc.Scale (0.25f));
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f (q0.m_x, q0.m_y, q0.m_z);
	glVertex3f (q1.m_x, q1.m_y, q1.m_z);

//int xxx = 0;
	for (dList<CustomVehicleController::BodyPartTire>::dListNode* node = m_controller->GetFirstTire(); node; node = m_controller->GetNextTire(node)) {
		const CustomVehicleController::BodyPartTire* const tire = &node->GetInfo();
		NewtonBody* const tireBody = tire->GetBody();

		dMatrix tireMatrix;
		NewtonBodyGetMatrix(tireBody, &tireMatrix[0][0]);

		dFloat sign = dSign ((tireMatrix.m_posit - matrix.m_posit) % matrix.m_right);
		tireMatrix.m_posit += matrix.m_right.Scale (sign * 0.25f);

		// draw the tire load 
		dVector normalLoad (m_controller->GetTireNormalForce(tire));
		dVector p0 (tireMatrix.m_posit);
		dVector p1 (p0 + normalLoad.Scale (scale));

		glColor3f (0.0f, 0.0f, 1.0f);
		glVertex3f (p0.m_x, p0.m_y, p0.m_z);
		glVertex3f (p1.m_x, p1.m_y, p1.m_z);

		// show tire lateral force
		dVector lateralForce (m_controller->GetTireLateralForce(tire));
		dVector p2 (p0 - lateralForce.Scale (scale));
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f (p0.m_x, p0.m_y, p0.m_z);
		glVertex3f (p2.m_x, p2.m_y, p2.m_z);

		// show tire longitudinal force
		dVector longitudinalForce (m_controller->GetTireLongitudinalForce(tire));
		dVector p3 (p0 - longitudinalForce.Scale (scale));
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f (p0.m_x, p0.m_y, p0.m_z);
		glVertex3f (p3.m_x, p3.m_y, p3.m_z);
//if (!xxx)
//dTrace(("%f ", tire.GetAligningTorque()));
//xxx ++;
	}
		
	glEnd();
	glLineWidth(1.0f);
}

const double CarEntity::GetSpeed()
{
	if (m_controller)
	{
		CustomVehicleController::EngineController* const engine = m_controller->GetEngine();
		return (double) engine->GetSpeed();
	}
	return 0.0;
}

const double CarEntity::GetRPM()
{
	if (m_controller)
	{
		CustomVehicleController::EngineController* const engine = m_controller->GetEngine();
		return (double) engine->GetRPM();
	}
	return 0.0;
}

const int CarEntity::GetCurrentGear()
{
	if (m_controller)
	{
		CustomVehicleController::EngineController* const engine = m_controller->GetEngine();
		return engine->GetGear();
	}
	return 0;
}

void CarEntity::SaveState()
{
	if (m_controller)
	{
		const CustomVehicleController::BodyPart* const chassis = m_controller->GetChassis ();
		NewtonBody* const chassisBody = chassis->GetBody();
		//m_initialMatrix = chassis.GetMatrix();
		NewtonBodyGetMatrix(chassisBody, &m_initialMatrix[0][0]);

		for (dList<CustomVehicleController::BodyPartTire>::dListNode* node = m_controller->GetFirstTire(); node; node = m_controller->GetNextTire(node)) 
		{
			const CustomVehicleController::BodyPartTire* const part = &node->GetInfo();
			BaseEntity* const tirePart = (BaseEntity*) part->GetUserData();

			dMatrix matrix;
			NewtonBodyGetMatrix(part->GetBody(), &matrix[0][0]);
			tirePart->SaveState2(matrix);
		}
	}
}

void CarEntity::RestoreState(BaseEntityManager& world, bool doInvalidateCache)
{
	
	const CustomVehicleController::BodyPart* const chassis = m_controller->GetChassis ();
	NewtonBody* const chassisBody = chassis->GetBody();
	
	NewtonBodySetMatrix( chassisBody, &m_initialMatrix[0][0] );
	//chassis.SetMatrix( m_initialMatrix );
	
	mBody = m_controller->GetBody();
	BaseEntity::RestoreState(world, doInvalidateCache);

	for (dList<CustomVehicleController::BodyPartTire>::dListNode* node = m_controller->GetFirstTire(); node; node = m_controller->GetNextTire(node)) 
	{
		const CustomVehicleController::BodyPartTire* const part = &node->GetInfo();
		BaseEntity* const tirePart = (BaseEntity*) part->GetUserData();

		tirePart->RestoreState(world, doInvalidateCache);
	}

	/*
	NewtonBodySetMatrix( m_controller->GetBody(), &m_initialMatrix[0][0] );
	const dFloat resetValues[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	NewtonBodySetOmega (m_controller->GetBody(), resetValues);
	NewtonBodySetVelocity (m_controller->GetBody(), resetValues);
	NewtonBodySetForce (m_controller->GetBody(), resetValues);
	NewtonBodySetTorque (m_controller->GetBody(), resetValues);
	*/
	m_controller->Finalize();
}
/*
void CarEntity::UpdateState()
{
	if (mHardware == nullptr) return;
	double unscale = 1.0 / mHardware->GetGlobalScale();

	FBMatrix m;
	dMatrix matrix;
	NewtonBodyGetMatrix( m_controller->GetBody(), &matrix[0][0] );

	m = dMatrixToFBMatrix( matrix );
	m[12] *= unscale;
	m[13] *= unscale;
	m[14] *= unscale;

	//mLastMatrix[0] = m;
	memcpy( mLastMatrix[0], m, sizeof(double) * 16 );

	CustomVehicleController::TireBodyState *node = m_controller->GetFirstTire();

	for (int i=1; node; ++i)
	{
		dMatrix aligmentMatrix (dGetIdentityMatrix());
		//if (tireMatrix[0][2] < 0.0f) {
		aligmentMatrix = dYawMatrix(0.5 * 3.141592f);

		matrix = aligmentMatrix * m_controller->GetTireLocalMatrix(node);
		m = dMatrixToFBMatrix(matrix);

		m[12] *= unscale;
		m[13] *= unscale;
		m[14] *= unscale;

		//mLastMatrix[i] = m;
		memcpy( mLastMatrix[i], m, sizeof(double) * 16 );

		node = m_controller->GetNextTire(node);
	}
}
*/
void CarEntity::UpdateTireTransforms()
{
	NewtonBody* const body = m_controller->GetBody();
	BaseEntityManager* const scene = (BaseEntityManager*) NewtonWorldGetUserData(NewtonBodyGetWorld(body));
		
#if 0
		// this is the general way for getting the tire matrices
		dMatrix rootMatrixInv (GetNextMatrix().Inverse());
		for (CustomVehicleControllerBodyStateTire* tire = m_controller->GetFirstTire(); tire; tire = m_controller->GetNextTire(tire)) {
			DemoEntity* const tirePart = (DemoEntity*) tire->GetUserData();
			TireAligmentTransform* const aligmentMatrix = (TireAligmentTransform*)tirePart->GetUserData();
			dMatrix matrix (aligmentMatrix->m_matrix * tire->CalculateGlobalMatrix() * rootMatrixInv);
			dQuaternion rot (matrix);
			tirePart->SetMatrix(*scene, rot, matrix.m_posit);
		}
#else
		// this saves some calculation since it get the tire local to the chassis
		//dMatrix parentMatrix = GetCurrentMatrix();
		dMatrix aligmentMatrix = dRollMatrix(0.5f * 3.141592f);

		for (dList<CustomVehicleController::BodyPartTire>::dListNode* node = m_controller->GetFirstTire(); node; node = m_controller->GetNextTire(node)) {
			const CustomVehicleController::BodyPartTire* const part = &node->GetInfo();
			BaseEntity* const tirePart = (BaseEntity*) part->GetUserData();
			//TireAligmentTransform* const aligmentMatrix = (TireAligmentTransform*)tirePart->GetUserData();
			
			CustomVehicleController::BodyPart* const parent = part->GetParent();

			NewtonBody* const body = part->GetBody();
			NewtonBody* const parentBody = parent->GetBody();

			dMatrix matrix;
			dMatrix parentMatrix;
			NewtonBodyGetMatrix(body, &matrix[0][0]);
			NewtonBodyGetMatrix(parentBody, &parentMatrix[0][0]);

			matrix = matrix * parentMatrix.Inverse();
			
			//
			dQuaternion rot (matrix);
			tirePart->SetMatrix(*scene, rot, matrix.m_posit);
		}
#endif
}