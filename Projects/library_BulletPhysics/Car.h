
#pragma once

//
#include "hardware_common.h"

PhysicsCar *CreateBullerCar(PhysicsHardware *hardware, const CarInfo &info);



/*
class BasicCar: public CustomDGRayCastCar
{
public:

	static BasicCar* Create (NewtonWorld* nWorld, const double scale, const CarCreationData &data, int materailID);

	

	void	UpdateProperties( const NewtonWorld *pWorld, const CarCreationData &data );

protected:

	//! a constructor
	BasicCar (int maxTiresCount, const dMatrix& chassisMatrix, NewtonBody* carBody);

public:
	//! a destructor
	virtual ~BasicCar();

private:
	void AddTire ( const CarCreationData &data, int wheel, const double scale, dFloat width, dFloat radius );
	

	static void SetTransform  (const NewtonBody* body, const dFloat* matrix, int threadIndex);

public:
	dVector				chassis_origin;
	dVector				chassis_inertia;

private:
	dFloat				m_steerAngle;
	dMatrix				m_tireOffsetMatrix;



	//NewtonCollision		*chassisCollision;
};
*/