
#include "Car.h"

//--- Bullet include
#include "BulletDynamics/Vehicle/btRaycastVehicle.h"



#define CUSTOM_VEHICLE_JOINT_ID				0xF4ED
#define CAR_USE_CONVEX_CAST                 1

#define CAR_CENTRE_OF_MASS_OFFSET	        (0.002f)
#define CAR_ENGINE_CENTRE_OF_MASS_OFFSET    (0.001f)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DECLARATION

//////////////////////////////////////////////////////////////////////////////////////
// class for making a car based on Bullet physics engine

class BulletCar : public PhysicsCar
{
public:
	static PhysicsCar *Create(BulletHardware *hardware, const CarData &data);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTION

//! a constructor
BasicCar::BasicCar (int maxTiresCount, const dMatrix& chassisMatrix, NewtonBody* carBody)
	:CustomDGRayCastCar (maxTiresCount, chassisMatrix, carBody)
{
	m_steerAngle = 0.0f;
	mDriveType = kCarDriveFrontWheels;
	mWheelRadius = 5.0;
	mWheelWidth = 1.0;
	// assign a type information to locate the joint in the callbacks
	SetJointID (CUSTOM_VEHICLE_JOINT_ID);
}

//! a destructor
BasicCar::~BasicCar()
{
	/*
	for (int i = 0; i < GetTiresCount(); i ++) {
		TireAnimation* tireAnim;
		const CustomDGRayCastCar::Tire& tire = GetTire (i);
		tireAnim = (TireAnimation*) tire.m_userData;
		delete tireAnim;
	}
	*/
}

BasicCar* BasicCar::Create (NewtonWorld* nWorld, const double scale, const CarCreationData &data, int materailID)
{
	if (data.pChassis == nullptr) return nullptr;

	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;
	dFloat mass;


	NewtonBody* rigidBody;
	NewtonCollision* chassisCollision;

	FBMatrix scaleM;
	FBScalingToMatrix( scaleM, FBSVector(scale, scale, scale) );

	FBMatrix m;
	data.pChassis->GetMatrix(m);
	//FBMatrixMult( m, m, scaleM );
	dMatrix matrix;

	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			matrix[i][j] = (float) m(i,j);

	matrix.m_posit.m_x *= scale;
	matrix.m_posit.m_y *= scale;
	matrix.m_posit.m_z *= scale;

	dMatrix   localMatrix = GetIdentityMatrix();
	FBGeometry *pGeometry = data.pChassis->Geometry;

	int count = 0;
	int stride = 4 * sizeof(float);
	
	count = pGeometry->GetArrayElementCount( kFBGeometryArrayID_Point );
	FBVertex *posArray = (FBVertex*) pGeometry->GetPositionArray();
	FBVertex *newArray = new FBVertex[count];

	if (scale != 1.0)
	{
		for (int i=0; i<count; ++i)
			FBVertexMatrixMult( newArray[i], scaleM, posArray[i] );
	}

	chassisCollision = NewtonCreateConvexHull(	nWorld, 
												count,
												&newArray[0][0], // point cloud
												stride,  // stride
												0.1f, // tolerance 
												0, // shapeId
												&localMatrix[0][0] ); //part->mObject->GetMatrix(true).Get()

	if (newArray)
	{
		delete [] newArray;
		newArray = nullptr;
	}

	//create the rigid body
	rigidBody = NewtonCreateBody (nWorld, chassisCollision, &matrix[0][0]);

	// save the pointer to the graphic object with the body.
	NewtonBodySetUserData (rigidBody, data.pChassis);

	// set the material group id for vehicle
	//	NewtonBodySetMaterialGroupID (m_vehicleBody, vehicleID);
	NewtonBodySetMaterialGroupID (rigidBody, materailID);

	// set a destructor for this rigid body
	NewtonBodySetDestructorCallback (rigidBody, PhysicsBodyDestructor);

	// set the force and torque call back function
	NewtonBodySetForceAndTorqueCallback (rigidBody, PhysicsApplyGravityForce);

	// set the transform call back function
	//NewtonBodySetTransformCallback (rigidBody, SetTransform);

	// set the matrix for both the rigid body and the graphic body
	

	NewtonBodySetMatrix (rigidBody, &matrix[0][0]);

	dVector origin (0, 0, 0, 1);
	dVector inertia (0, 0, 0, 1);

	// calculate the moment of inertia and the relative center of mass of the solid
	NewtonConvexCollisionCalculateInertialMatrix (chassisCollision, &inertia[0], &origin[0]);

	dVector base_origin = origin;
	dVector base_inertia = inertia;

	// Set the vehicle Center of mass
	// the rear spoilers race the center of mass by a lot for a race car
	// we need to lower some more for the geometrical value of the y axis
	//origin.m_y *= JEEP_CENTRE_OF_MASS_OFFSET;

	// Some value is so high here
	mass = data.Mass;
	origin.m_x += -CAR_ENGINE_CENTRE_OF_MASS_OFFSET;
	origin.m_y += -CAR_CENTRE_OF_MASS_OFFSET;

	Ixx = mass * inertia[0];
	Iyy = mass * inertia[1];
	Izz = mass * inertia[2];
	/*
	Ixx = mass * 1.2;
	Iyy = mass * 0.8;
	Izz = mass * 0.8;
	*/
	NewtonBodySetCentreOfMass (rigidBody, &origin[0]);
	// set the mass matrix
	NewtonBodySetMassMatrix (rigidBody, mass, Ixx, Iyy, Izz);


	// release the collision
	NewtonReleaseCollision (nWorld, chassisCollision);

	// set the vehicle local coordinate system
	dMatrix chassisMatrix = GetIdentityMatrix();
	// if you vehicle move along the z direction you can use
	//	chassisMatrix.m_front = dVector (0.0f, 0.0f, 1.0f, 0.0);
	chassisMatrix.m_front = dVector (1.0f, 0.0f, 0.0f, 0.0f);			// this is the vehicle direction of travel
	chassisMatrix.m_up	  = dVector (0.0f, 1.0f, 0.0f, 0.0f);			// this is the downward vehicle direction
	chassisMatrix.m_right = chassisMatrix.m_front * chassisMatrix.m_up;	// this is in the side vehicle direction (the plane of the wheels)
	chassisMatrix.m_posit = dVector (0.0f, 0.0f, 0.0f, 1.0f);

	// create a vehicle joint with 4 tires
	BasicCar* carJoint;
	carJoint = new BasicCar (4, chassisMatrix, rigidBody);	// int maxTiresCount, const dMatrix& chassisMatrix, NewtonBody* carBody
	//carJoint->m_tireOffsetMatrix = carRoot->CalcGlobalMatrix();
	carJoint->m_tireOffsetMatrix = matrix;

	carJoint->chassis_origin = base_origin;
	carJoint->chassis_inertia = base_inertia;

	// get radio and width of the tire
	dFloat witdh;
	dFloat radius;
	carJoint->CalculateTireDimensions (data.wheels[0].pModel, witdh, radius);

	witdh *= scale;
	radius *= scale;

	carJoint->mWheelWidth = witdh;
	carJoint->mWheelRadius = radius;

	// add all tire to car
	carJoint->AddTire (data, 0, scale, witdh, radius);
	carJoint->AddTire (data, 1, scale, witdh, radius);
	carJoint->AddTire (data, 2, scale, witdh, radius);
	carJoint->AddTire (data, 3, scale, witdh, radius);
	return carJoint;
}



void BasicCar::ApplySteering (dFloat value)
{
	dFloat vEngineSteerAngle = GenerateTiresSteerAngle( value );
	dFloat vEngineSteerForce = GenerateTiresSteerForce( vEngineSteerAngle );
	// Set the both generate steer angle and force value's
	SetTireSteerAngleForce( 0, vEngineSteerAngle, vEngineSteerForce );
	SetTireSteerAngleForce( 1, vEngineSteerAngle, vEngineSteerForce );
	/*
	if (vId==1){
		SetTireSteerAngleForce( 1, -vEngineSteerAngle, -(vEngineSteerForce/2) );
		SetTireSteerAngleForce( 3, -vEngineSteerAngle, -(vEngineSteerForce/2) );
	}
	*/
}

void BasicCar::ApplySteeringDirect (dFloat value)
{
	m_steerAngle = value;
	dFloat vEngineSteerAngle = m_steerAngle;
	dFloat vEngineSteerForce = GenerateTiresSteerForce( vEngineSteerAngle );
	// Set the both generate steer angle and force value's
	SetTireSteerAngleForce( 0, vEngineSteerAngle, vEngineSteerForce );
	SetTireSteerAngleForce( 1, vEngineSteerAngle, vEngineSteerForce );
	/*
	if (vId==1){
		SetTireSteerAngleForce( 1, -vEngineSteerAngle, -(vEngineSteerForce/2) );
		SetTireSteerAngleForce( 3, -vEngineSteerAngle, -(vEngineSteerForce/2) );
	}
	*/
}

void BasicCar::ApplyBrake (dFloat value)
{
	// Set the generate brake value
	SetTireBrake( 0, value);
	SetTireBrake( 1, value);
	SetTireBrake( 3, value);
	SetTireBrake( 4, value);
}

void BasicCar::ApplyHandBrake (dFloat value)
{
	// Set the generate brake value
	SetTireBrake( 0, value);
	SetTireBrake( 1, value);
}

void BasicCar::ApplyTorque (dFloat value)
{

	dFloat vEngineTorque;
	//		vEngineTorque = GenerateTiresTorque( value );
	vEngineTorque = GenerateEngineTorque (value);

	//		// Set the generate torque value
	switch(mDriveType)
	{
		case kCarDriveFrontWheels:
		SetTireTorque( 0, vEngineTorque );
		SetTireTorque( 1, vEngineTorque );
		break;
		case kCarDriveRearWheels:
		SetTireTorque( 2, vEngineTorque );
		SetTireTorque( 3, vEngineTorque );
		break;
		case kCarDrive4x4:
		SetTireTorque( 0, vEngineTorque );
		SetTireTorque( 1, vEngineTorque );
		SetTireTorque( 2, vEngineTorque );
		SetTireTorque( 3, vEngineTorque );
		break;
	}
/*
	SetTireTorque( 0, vEngineTorque );
	SetTireTorque( 2, vEngineTorque );
	switch (vId)
	{
		case 0:
		{
			_ASSERTE (0);
//				SetCustomTireTorque( 0, vEngineTorque );
//				SetCustomTireTorque( 1, vEngineTorque );
			break;
		}
		case 1:
		{
			_ASSERTE (0);
//				SetCustomTireTorque( 0, vEngineTorque );
//				SetCustomTireTorque( 1, vEngineTorque );
			dFloat speed;
			speed = dAbs( GetSpeed() );
			if (speed<5.0f) {
				_ASSERTE (0);
				// Can make a little wheely (:
//					SetVarTireMovePointForceUp(2,-1.35f);
//					SetVarTireMovePointForceUp(3,-1.35f);
			} else {
				_ASSERTE (0);
//					SetVarTireMovePointForceUp(2,0.0f);
//					SetVarTireMovePointForceUp(3,0.0f);
			}
			break;
		}
		case 2:
		{
//				dFloat speed;
//				speed = dAbs( GetSpeed() );
//				if ((speed<25) && (speed>20)) {
//					_ASSERTE (0);
//					SetVarTireMovePointForceUp(2,0.2f);
//					SetVarTireMovePointForceUp(3,0.2f);
//				} else if (speed<20) {
//					_ASSERTE (0);
//					SetVarTireMovePointForceUp(2,0.25f);
//					SetVarTireMovePointForceUp(3,0.25f);
//				} else {
//					_ASSERTE (0);
//					SetVarTireMovePointForceUp(2,0.0f);
//					SetVarTireMovePointForceUp(3,0.0f);
//				}
		}
	}*/
}


void BasicCar::AddTire ( const CarCreationData &data, int wheelId, const double scale, dFloat width, dFloat radius )
{

	// add this tire, get local position and rise it by the suspension length

	const CarCreationData::Wheel &wheel = data.wheels[wheelId];

	FBVector3d pos;
	wheel.pModel->GetVector(pos, kModelTranslation, false);

	/*
	//matrix.m_posit.m_x -= 50.0f;
	matrix.m_posit.m_y = m[14];
	matrix.m_posit.m_z = m[13];

	matrix.m_posit.m_x *= CAR_SCALE;
	matrix.m_posit.m_y *= CAR_SCALE;
	matrix.m_posit.m_z *= CAR_SCALE;*/
	//matrix.m_posit.m_z = m[14] - width * 0.5;
	
	dVector tirePosition;
	for (int i=0; i<3; ++i)
		tirePosition[i] = (float) pos[i];
	
	tirePosition.m_x *= scale;
	tirePosition.m_y *= scale;
	tirePosition.m_z *= scale;

	// not need anymore because it is done internally already now...
	//tirePosition += matrix .m_up.Scale (JEEP_SUSPENSION_LENGTH);
	/*
	if (strcmp(config.m_name,"jeep.mdl")==0) {
		AddSingleSuspensionTire (tire, tirePosition, JEEP_TIRE_MASS, radius, width, JEEP_FRICTION, JEEP_SUSPENSION_LENGTH, JEEP_SUSPENSION_SPRING, JEEP_SUSPENSION_DAMPER, JEEP_USE_CONVEX_CAST);
	} else if (strcmp(config.m_name,"monstertruck.mdl")==0) {
		AddSingleSuspensionTire (tire, tirePosition, 50, radius, width, TRUCK_FRICTION, 0.6f, 65.0f, 2.0f, JEEP_USE_CONVEX_CAST);
	} else if (strcmp(config.m_name,"f1.mdl")==0) {
		AddSingleSuspensionTire (tire, tirePosition, 25, radius, width, F1_FRICTION, 0.2f, 200.0f, 6.0f, JEEP_USE_CONVEX_CAST);
	}
	*/
	AddSingleSuspensionTire (	wheel.pModel, 
								tirePosition, 
								wheel.Mass, 
								radius, 
								width, 
								wheel.Friction,
								wheel.SuspensionLength * scale, 
								wheel.SuspensionSpring, 
								wheel.SuspensionDamper, 
								CAR_USE_CONVEX_CAST);
}


void BasicCar::UpdateProperties( const NewtonWorld *pWorld,  const CarInfo &info )
{
	if (pWorld == nullptr) return;

	NewtonWorldCriticalSectionLock(pWorld);

	//
	// chassis properties
	//

	dVector origin = chassis_origin;
	dVector inertia = chassis_inertia;

	// Set the vehicle Center of mass
	// the rear spoilers race the center of mass by a lot for a race car
	// we need to lower some more for the geometrical value of the y axis
	//origin.m_y *= JEEP_CENTRE_OF_MASS_OFFSET;

	// Some value is so high here
	dFloat mass = data.Mass;
	origin.m_x += -data.MassXOffset;
	origin.m_y += -data.MassYOffset;

	dFloat Ixx = mass * inertia[0];
	dFloat Iyy = mass * inertia[1];
	dFloat Izz = mass * inertia[2];
	/*
	Ixx = mass * 1.2;
	Iyy = mass * 0.8;
	Izz = mass * 0.8;
	*/
	NewtonBodySetCentreOfMass ( GetBody0(), &origin[0]);
	// set the mass matrix
	NewtonBodySetMassMatrix ( GetBody0(), mass, Ixx, Iyy, Izz);

	//
	// wheels properties
	//
	for (int i=0; i<m_tiresCount; ++i)
	{
		m_tires[i].m_posit = data.wheels[i].SuspensionLength;
		m_tires[i].m_suspensionLenght = data.wheels[i].SuspensionLength;
		m_tires[i].m_springConst = data.wheels[i].SuspensionSpring;
		m_tires[i].m_springDamper = data.wheels[i].SuspensionDamper;

		m_tires[i].m_groundFriction = data.wheels[i].Friction;

		m_tires[i].m_mass = data.wheels[i].Mass;
		m_tires[i].m_Ixx = data.wheels[i].Mass * mWheelRadius * mWheelRadius / 2.0f;
		m_tires[i].m_IxxInv	= 1.0f / m_tires[i].m_Ixx;
	}
	NewtonWorldCriticalSectionUnlock(pWorld);
}

void BasicCar::CalculateTireDimensions (FBModel *pModel, dFloat& witdh, dFloat& radius) const
{
	dVector extremePoint;
	NewtonWorld* world;
	NewtonCollision* collision;

	// find the the tire visual mesh
	world = NewtonBodyGetWorld(GetBody0());

	FBGeometry *pGeometry = pModel->Geometry;

	int count = 0;
	int stride = 4 * sizeof(float);

	count = pGeometry->GetArrayElementCount( kFBGeometryArrayID_Point );
	FBVertex *posArray = (FBVertex*) pGeometry->GetPositionArray();

	collision = NewtonCreateConvexHull(	world, 
										count,
										&posArray[0][0], // point cloud
										stride,  // stride
										0.0f, 
										0, 
										NULL);

	dMatrix tireMatrix = GetIdentityMatrix();
	FBMatrix m;
	pModel->GetMatrix(m);

	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			tireMatrix[0][0] = (float) m(i,j);
	
	//dMatrix tireMatrix (tirePart->CalcGlobalMatrix() * carModel->m_matrix);
	//		dMatrix tireMatrix (tirePart->CalcGlobalMatrix());

	// fin the support points tha can be use to define the tire collision mesh
	dVector upDir (tireMatrix.UnrotateVector(dVector (0.0f, 1.0f, 0.0f, 0.0f)));
	NewtonCollisionSupportVertex (collision, &upDir[0], &extremePoint[0]);
	radius = upDir % extremePoint;


	//dVector withdDir (tireMatrix.UnrotateVector(carModel->m_matrix.m_right));
	dVector withdDir (tireMatrix.UnrotateVector(tireMatrix.m_right));
	NewtonCollisionSupportVertex (collision, &withdDir[0], &extremePoint[0]);
	witdh = withdDir % extremePoint;

	withdDir = withdDir.Scale (-1.0f);
	NewtonCollisionSupportVertex (collision, &withdDir[0], &extremePoint[0]);
	witdh += withdDir % extremePoint;

	NewtonReleaseCollision (world, collision);

	//radius = 60.0f;
	//witdh = 30.0f;
	witdh *= 0.4f;
}
/*
static void UdateSuspentionParts (void* userData, dFloat* posit, dFloat* rotation)
{
	dBone* node;
	node = (dBone*) userData;

	dVector p (posit[0], posit[1], posit[2], 1.0f);
	dQuaternion r (rotation[0], rotation[1], rotation[2], rotation[3]);
	node->SetMatrix (dMatrix (r, p));
}
*/

void BasicCar::SetTransform  (const NewtonBody* body, const dFloat* matrix, int threadIndex)
{
	BasicCar* car;
	NewtonJoint* joint;

//	NewtonWorld *gWorld = GetWorldPtr();
	bool debugDisplay = GetDebugDisplay();

	// set the transform of the main body
	PhysicsSetTransform (body, matrix, threadIndex);

	// find the car joint attached to the body
	car = NULL;
	for (joint = NewtonBodyGetFirstJoint(body); joint; joint = NewtonBodyGetNextJoint(body, joint)) {
		car = (BasicCar*) NewtonJointGetUserData(joint);
		if (car->GetJointID() == CUSTOM_VEHICLE_JOINT_ID) {
			break;
		}
	}

	if (car) {
		// position all visual tires matrices
		int count;
		const dMatrix& carMatrix = *((dMatrix*)matrix);
		dMatrix mm = carMatrix;
		/*
		if ( debugDisplay )
		{
			NewtonWorldCriticalSectionLock(gWorld);
			//DebugDrawCollision( car->GetChassisCollisionPtr(), mm );
			NewtonWorldCriticalSectionUnlock(gWorld);
		}
		*/
		dMatrix rootMatrixInv (car->m_tireOffsetMatrix * carMatrix);
		rootMatrixInv = rootMatrixInv.Inverse();
		count = car->GetTiresCount();

		// for each tire get the global matrix postion, and calculate the local matrix relative to the main body
		for (int i = 0; i < count; i ++) {

			const CustomDGRayCastCar::Tire& tire = car->GetTire (i);

			//tireAnim = (TireAnimation*) tire.m_userData;
			//tireNode = tireAnim->m_tire;
			FBModel *pModel = (FBModel*) tire.m_userData;
/*
			// if the tire has an animation, update the animation matrices
			if (tireAnim->m_animation) {
				dFloat param;
				param = (1.0f - car->GetTireParametricPosition(i));

				if (param > 1.0f) {
					param = 1.0f;
				}
				if (param < 0.0f) {
					param = 0.0f;
				}

				tireAnim->m_animation->GeneratePose (param);
				tireAnim->m_animation->UpdatePose();
			}
*/
			// calculate the tire local matrix
			//dMatrix matrix (car->CalculateTireMatrix(i) * rootMatrixInv);
			dMatrix matrix( car->CalculateTireMatrix(i) /** car->GetChassisMatrixLocal() * carMatrix*/ );

			FBMatrix m;
			FBTVector T;
			FBRVector R;
			FBSVector S;

			for (int i=0; i<4; ++i)
				for (int j=0; j<4; ++j)
					m(i,j) = (double) matrix[i][j];

			FBMatrixToTRS(T, R, S, m);

			//pModel->Translation.SetCandidate( T, sizeof(double)*3 );
			//pModel->Rotation.SetCandidate( R, sizeof(double)*3 );
			
		/*
			if (debugDisplay) {
				// if debug display show the contact and tire collsion shape in debug mode
				NewtonWorldCriticalSectionLock(gWorld);
				dMatrix tireBaseMatrix (car->CalculateSuspensionMatrix(i, tire.m_posit) * car->GetChassisMatrixLocal() * carMatrix);
				DebugDrawCollision (tire.m_shape, tireBaseMatrix, dVector (1.0f, 1.0f, 0.0f, 1.0f));

				dVector span (tire.m_contactPoint + tire.m_contactNormal.Scale (1.0f));
				DebugDrawLine (tire.m_contactPoint, span, dVector (1.0f, 0.0f, 0.0f, 1.0f));

				//DebugDrawLine (tire.m_contactPoint, car->dbpos, dVector (0.5f, 0.0f, 1.0f, 1.0f));
				NewtonWorldCriticalSectionUnlock(gWorld);
			}
			*/
		}
	}
}