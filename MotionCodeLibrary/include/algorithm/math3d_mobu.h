/**
	math functions and classes (geometry operations)
    Copyright (C) 2010 Sergey Solokhin (neill3d), e-mail to: s@neill3d.com
	home page - neill3d.com

    GitHub page - https://github.com/Neill3d/MoPlugs_Framework
	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
*/
#ifndef		__MATH_3D_
#define		__MATH_3D_

//--- SDK include
#include <fbsdk/fbsdk.h>
//#include <fbsdk/fbsdk-opengl.h>

#ifdef	MATH_3D_OPENGL
//#include <GL\glew.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>
//#include "Types.h"

typedef FBArrayTemplate<FBVector3d> FBArrayVector3d;

//const float	PI				= 3.14159265358979323846f;
const double	M_RAD2DEG		= 180.0 / M_PI;
#define 		M_DEG2RAD		0.0174532925
#define DEG2RAD(a)				( (a) * M_DEG2RAD )
#define RAD2DEG(a)				( (a) * M_RAD2DEG )

//
//
double clamp(const double x, const double lower, const double upper);
double clamp01(const double x);
float clampf(const float x, const float lower, const float upper);
double smoothstep(const double edge0, const double edge1, const double x);
float smootherstep(float edge0, float edge1, float x);

// convert from double vector to float vector
void FBVectorToVec3(const double *v, float *dst);
void FBVectorToVec4KeepW(const double *v, float *dst);
void FBVectorToVec4(const double *v, float *dst);


// vector part
double DotProduct(const FBVector3d v1, double *v2);
FBVector3d CrossProduct(const FBVector3d &left, const FBVector3d &right);
inline FBVector3d VectorSubtract( const FBVector3d &v1, const FBVector3d &v2);
void VectorSub( const FBVector2d &v1, const FBVector2d &v2, FBVector2d &res );
void VectorSub( const FBVector3d &v1, const FBVector3d &v2, FBVector3d &res );
void VectorNormalize( FBVector3d	&v );
void VectorNormalize( FBVector2d	&v );
void VectorNormalizeArray( FBVector3d*	v[], int count );
double	VectorLength( const FBVector3d &v );
double	VectorLength( const FBVector2d &v );
double VectorDistance( const double *v1, const double *v2 );
void VectorMult( FBVector3d	&v, double factor);
void VectorMult( FBVector2d &v, const double factor );
void VectorMult( const FBVector3d &v1, const FBVector3d &v2, FBVector3d &res );
void VectorMultArray( FBVector3d*	v[], int count, double factor );
FBVector3d VectorCenter( FBVector3d *v, int count );
void VectorMiddle(const FBVector3d &v1, const FBVector3d &v2, FBVector3d &middle);
void VectorScale( FBVector3d &v,	double factor );
void VectorAdd( const FBVector3d &v1, const FBVector3d &v2, FBVector3d &res );
void VectorAddArray( FBVector3d* v[], int count, const FBVector3d &v2 );
void VectorTransform ( const FBVector3d in1, const FBMatrix in2, FBVector3d &res );
void VectorTransform33 ( const FBVector3d in1, const FBMatrix in2, FBVector3d &res );
void VectorTransform33_id (const FBVector3d &in1, const FBMatrix &mat, FBVector3d &res );
void VectorRotate( const FBVector3d in1, const FBVector3d angles, FBVector3d &res ); // angles in degrees
FBVector3d ClosestPointOnPlane(FBVector4d plane, FBVector3d point);
FBVector3d ClosestPointOnLine(const FBVector3d A, const FBVector3d B, const FBVector3d P, double *t=nullptr);
void ClosestPointOnLine2(FBVector3d &pointOnLine, const FBVector3d &A, const FBVector3d &B, const FBVector3d &P, double *t=nullptr);
void rotateWithVector(const FBVector3d x, FBVector3d u, double ang, FBVector3d &Res);
FBVector3d LinearInterpolation( FBVector3d	&v1, FBVector3d	&v2, double t );
void VectorLerp( const FBVector3d &v1, const FBVector3d &v2, const double t, FBVector3d &res );
double VectorAngle(const FBVector3d &u, const FBVector3d &v);

bool FindClosestPointOnCurve( FBVector4d &pointOnCurve, double &curvePercent, FBModelPath3D *pCurve, const FBVector4d &pos, const int numberOfSubdivisions, const int segmentSubdivisions );

// matrix part
int		mNdx(int row, int col);
// put angles in radians
FBMatrix		rotateX ( double angle );
FBMatrix    rotateY ( double angle );
FBMatrix    rotateZ ( double angle );
FBMatrix		MatrixMult(const FBMatrix &M1, const FBMatrix &M2);
FBMatrix		MatrixFastInverse(const FBMatrix &in);
FBMatrix		MatrixInvert(FBMatrix	&m);

void LookAt( const FBVector3d& Eye, const FBVector3d& Center, const FBVector3d& Up, FBMatrix &matrix );

#ifdef	MATH_3D_OPENGL
bool				MatrixExtract(const GLdouble model[16], const GLdouble proj[16],
										const GLint viewport[4], GLdouble * objx, GLdouble * objy, GLdouble * objz);
bool				MatrixExtract(const FBMatrix	&tm, FBVector3d &forward, FBVector3d &up, FBVector3d &right);
#endif

FBMatrix FBMatrixFromAnimationNode( FBModel	*pModel, FBTime pTime );

FBMatrix NormalToRotation(FBTVector normal);

// angles
FBVector3d MatrixToEuler( const FBMatrix &matrix);

// quaternion
FBVector4d from_matrix( const FBMatrix &m );
FBVector4d MatrixToQuaternion (const FBMatrix &matrix);
FBVector4d	UnitAxisToQuaternion (const FBVector3d &unitAxis, double Angle);
void	MatrixInit (const FBVector4d &rotation, FBMatrix &m);
FBVector4d QuaternionMult (const FBVector4d &A, const FBVector4d &B);
FBVector4d CreateQuat(const FBVector3d &rotateFrom, const FBVector3d &rotateTo);

void QuatInverse(FBQuaternion &q);
void QuatConjugate(FBQuaternion &q);
double QuatDot(const FBQuaternion &q1, const FBQuaternion &q2);

// plane
FBVector4d PlaneMake(const FBVector3d &point, const FBVector3d &normal);
FBVector4d PlaneMake(const FBVector3d &p1, const FBVector3d &p2, FBVector3d &p3);
bool PlaneIntersection( const FBVector4d &plane1, const FBVector4d &plane2, FBVector3d &start, FBVector3d &dir );
bool RayIntersection( const FBVector4d &plane, const FBVector3d &start, const FBVector3d &dir, double &scale );

double PlaneDistance(const FBVector4d &plane, const FBVector3d &point);

// takes a point and a radius and return number of hits
bool RaySphereIntersect( const FBVector3d &p, const FBVector3d &d, const double r, double &d1, double &d2 );

//

//void ComputeFrustumFromRenderOptions(FBRenderOptions *pOptions, CFrustum &frustum);
bool ComputeCameraValues(FBRenderOptions *pOptions, FBMatrix &modelview, FBVector3d &cameraPos);
bool ComputeCameraValues(FBRenderOptions *pOptions, FBMatrix &modelview, FBMatrix &projection, FBMatrix &viewIT, FBVector3d &cameraPos, int &width, int &height, double &farPlane);
bool ComputeCameraValues(FBCamera *pCamera, FBMatrix &modelview, FBMatrix &projection, FBMatrix &viewIT, FBVector3d &cameraPos, int &width, int &height, double &farPlane);
void ComputeCameraRay( FBMatrix &modelview, FBVector3d &outRayDir );

// bezier spline
double CalcBezier(double p0, double p1, double p2, double p3, double t);
double CalcBetaPoint(double p0, double p1, double p2, double p3, double t, double beta1, double beta2);
double Lagr(int n, double xt, double *p);

// misc
#ifdef	MATH_3D_OPENGL
GLint UnProject(GLdouble winx, GLdouble winy, GLdouble winz,
	     const GLdouble model[16], const GLdouble proj[16],
	     const GLint viewport[4],
	     GLdouble * objx, GLdouble * objy, GLdouble * objz);
#endif
//*********************** 07.03.2003 **************************//
//
//      ����� ���������� ��������� ( n >= m ).
//      �������������� �����������.
//		�������� - prografix.narod.ru
//
//*********************** 03.04.2005 **************************//

class Matrix
{
    double ** p;
// ������ ��������� ������������
    void operator = ( const Matrix & );
public:
    const unsigned int nRow; // �-�� �����
    const unsigned int nCol; // �-�� ��������

    Matrix ( unsigned int r, unsigned int c );
    Matrix ( unsigned int r, unsigned int c, const double * const * a );
    Matrix ( const Matrix & );
   ~Matrix ();

    void fill ( double );
    Matrix & operator *= ( double );
    operator       double * const * ()       { return p; }
    operator const double * const * () const { return p; }
};

#undef NAT
#define NAT unsigned int

class LSS_H_Stor
{
    Matrix & a;
    NAT * pivot;
    double * alpha;
// ������ ������������ ����� � ��������� ������������:
    LSS_H_Stor ( LSS_H_Stor & );
    void operator = ( LSS_H_Stor & );
public:
    LSS_H_Stor ( Matrix & a );
    bool solve ( const double * b, double * x ) const; // b[a.nRow], x[a.nCol]
    ~LSS_H_Stor ();
};

class LSS_H
{
    Matrix matrix;
    LSS_H_Stor lss;
public:
    LSS_H ( NAT n, NAT m, const double * const * a ) : matrix ( n, m, a ), lss ( matrix ) {}
    bool solve ( const double * b, double * x ) const // b[n], x[m]
    {
        return lss.solve ( b, x );
    }
};

bool lss_h ( Matrix & a, const double * b, double * x );
bool lss_h ( NAT n, NAT m, const double * const * a, const double * b, double * x );


//********************** 16.10.2004 *********************************//
//
//      ������������� ������� ���������.
//      �������� ��������, ��� m - ��� �� ������� ��������, � �-�� �������������.
//      ������� �������� ����� m-1.
//		�������� - prografix.narod.ru
//
//********************** 24.06.2005 *********************************//

bool apprLSS_H ( int n, const double * x, const double * y, int m, double * c );


//************************ 23.06.2005 *******************************//
//
//      ������������� ����� ������
//      ������� ����� ��������� ����������
//      c - �����, r - ������
//		�������� - prografix.narod.ru
//
//************************ 11.04.2008 *******************************//

void linBuild(unsigned n, unsigned m, const double *x, const double *y,
			  double *&A, double *&B);
void linSolve(unsigned m, double *&A, double *&B, double *&c);

bool getSphere2 ( NAT n, const FBVector3d * p, FBVector3d & c, double & r );

#define	FREE_ARRAY(x)	if (x) { delete [] x;  x=NULL; }

#endif // #ifndef		__MATH_3D_