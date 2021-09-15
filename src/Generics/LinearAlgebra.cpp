/*****************************************************************************
<HEADER>
*****************************************************************************/

#include <GenericTypes.h>
#include <math.h>

using namespace Generic;

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

/****************************************************************************/
/** Adapted from GLU's matmul4
*****************************************************************************/
Matrix4x4 operator * (Matrix4x4 &a, Matrix4x4 &b)
{
	Matrix4x4 product;

	int i;
	for (i = 0; i < 4; i++)
	{
		const real ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
		P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
		P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
		P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
		P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
	}

	return product;
}

Matrix4x4::Matrix4x4 ()
{
	for (int i = 0; i < 16; i++)
		m_Elements[i] = 0;
}

Matrix4x4::Matrix4x4 (Quaternion &quat)
{
#define matrix(row,col) m_Elements[(row<<2)+col]

	matrix(0,0) = 1.0 - 2*quat.y*quat.y - 2*quat.z*quat.z;
	matrix(1,0) = 2*quat.x*quat.y + 2*quat.w*quat.z;
	matrix(2,0) = 2*quat.x*quat.z - 2*quat.w*quat.y;
		matrix(3,0) = 0;

	matrix(0,1) = 2*quat.x*quat.y - 2*quat.w*quat.z;
	matrix(1,1) = 1.0 - 2*quat.x*quat.x - 2*quat.z*quat.z;
	matrix(2,1) = 2*quat.y*quat.z + 2*quat.w*quat.x;
		matrix(3,1) = 0;

	matrix(0,2) = 2*quat.x*quat.z + 2*quat.w*quat.y;
	matrix(1,2) = 2*quat.y*quat.z - 2*quat.w*quat.x;
	matrix(2,2) = 1.0 - 2*quat.x*quat.x - 2*quat.y*quat.y;
		matrix(3,2) = 0;

	matrix(3,1) = 0;
	matrix(3,1) = 0;
	matrix(3,1) = 0;
		matrix(3,3) = 1;
}

void Matrix4x4::vSetIdentity ()
{
	for (int i = 0; i < 15; i++)
		m_Elements[i] = 0;
	m_Elements[0] = 1;
	m_Elements[5] = 1;
	m_Elements[10] = 1;
	m_Elements[15] = 1;
}

VectorXYZ Generic::operator * (VectorXYZ &v, Matrix4x4 &a)
{
	VectorXYZ result;

	result.x = 
		v.x * A(0,0) +
		v.y * A(0,1) +
        v.z * A(0,2) +
              A(0,3);

	result.y = 
		v.x * A(1,0) +
        v.y * A(1,1) +
        v.z * A(1,2) +
              A(1,3);

	result.z = 
		v.x * A(2,0) +
        v.y * A(2,1) +
        v.z * A(2,2) +
              A(2,3);

	return result;
}


VectorXYZ Generic::operator- (const VectorXYZ &a)
{
	VectorXYZ result;
	result.x = -a.x;
	result.y = -a.y;
	result.z = -a.z;
	return result;
}

/****************************************************************************/
/* Quaternions
*****************************************************************************/

Quaternion::Quaternion () :
	w(0), x(0), y(0), z(0)
{
}

Quaternion::~Quaternion ()
{
}

Quaternion Generic::operator * (const Quaternion &a, const Quaternion &b)
{
	Quaternion result;

	result.w = (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z);
	result.x = (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y);
	result.y = (a.w * b.y) + (a.y * b.w) + (a.z * b.x) - (a.x * b.z);
	result.z = (a.w * b.z) + (a.z * b.w) + (a.x * b.y) - (a.y * b.x);

	return result;
}

void Quaternion::vNormalize ()
{
	real rMagnitude = sqrt (w*w + x*x + y*y + z*z);
	w /= rMagnitude;
	x /= rMagnitude;
	y /= rMagnitude;
	z /= rMagnitude;
}


/****************************************************************************/
/** Computes the intersection of the given line and plane

From http://www.frontiernet.net/~imaging/rt_how2.html

      The intersection of (point "P" on) the pick line and the drag plane is:
                                                 N * Po - D
             P = Po + t * V   where t =   -   ---------------   and  D = N * Pop
                                                    N * V 
where
	Po 	= the line's point of origin (hinge location)
	V 	= the line's direction vector (hinge axis)
	N	= the plane's normal (hinge axis again)
	Pop	= point on plane (the body's position)
*****************************************************************************/

VectorXYZ Generic::vecIntersectionOfLineAndPlane (
		const VectorXYZ &vecLineOrigin, 
		const VectorXYZ &vecLineDirection, 
		const VectorXYZ &vecPlaneNormal, 
		const VectorXYZ &vecPointOnPlane)
{
	real D = DotProduct (vecPlaneNormal, vecPointOnPlane);
	real t_Numerator = (DotProduct (vecPlaneNormal, vecLineOrigin) - D);
	real t_Denominator = DotProduct (vecPlaneNormal, vecLineDirection);
	real t = - (t_Numerator / t_Denominator);
	VectorXYZ vecResult = vecLineOrigin + (vecLineDirection * t);
	return vecResult;
}

VectorXYZ Generic::operator + (const VectorXYZ &a, real r)
{
	VectorXYZ result;
	result.x = a.x + r;
	result.y = a.y + r;
	result.z = a.z + r;
	return result;
}

VectorXYZ Generic::operator * (const VectorXYZ &a, real r)
{
	VectorXYZ result;
	result.x = a.x * r;
	result.y = a.y * r;
	result.z = a.z * r;
	return result;
}

VectorXYZ Generic::operator + (const VectorXYZ &a, const VectorXYZ &b)
{
	VectorXYZ result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

VectorXYZ& Generic::operator += (VectorXYZ &a, const VectorXYZ &b)
{
	a = a + b;
	return a;
}


VectorXYZ Generic::operator - (const VectorXYZ &a, const VectorXYZ &b)
{
	VectorXYZ result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

real VectorXYZ::rMagnitude ()
{
	return sqrt (x*x + y*y + z*z);
}


real Generic::DotProduct (const VectorXYZ &a, const VectorXYZ &b)
{
	real r = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
	return r;
}

VectorXYZ Generic::CrossProduct (const VectorXYZ &a, const VectorXYZ &b)
{
	VectorXYZ result;

	result.x = (a.y * b.z) - (a.z * b.y);
	result.y = (a.z * b.x) - (a.x * b.z);
	result.z = (a.x * b.y) - (a.y * b.x);

	return result;
}

/****************************************************************************/
/** Rotate a vector around a quaternion
*****************************************************************************/
void Generic::VectorXYZ::vRotateBy (const Generic::Quaternion &q)
{
	real rFactor1 = 0, rFactor2 = 0, rFactor3 = 0;

	rFactor1 = (2 * q.w * q.w) - 1;
	rFactor2 = 2 * ((q.x * x) + (q.y * y) + (q.z * z));
	rFactor3 = 2 * q.w * q.w;

	x = (rFactor1 * x) + (rFactor2 * q.x) + (rFactor3 * ((q.y * z) - (q.z * y)));
	y = (rFactor1 * y) + (rFactor2 * q.y) + (rFactor3 * ((q.z * x) - (q.x * z)));
	z = (rFactor1 * z) + (rFactor2 * q.z) + (rFactor3 * ((q.x * y) - (q.y * x)));
}

/****************************************************************************/
/** Rotate a vector around a give yaw, pitch, and roll.
*****************************************************************************/
void Generic::VectorXYZ::vRotateBy (const VectorYPR &vecRotationDegrees)
{
	real rYaw   = rDegreesToRadians * vecRotationDegrees.yaw;
	real rPitch = rDegreesToRadians * vecRotationDegrees.pitch;
	real rRoll  = rDegreesToRadians * vecRotationDegrees.roll;

	Quaternion qYaw, qPitch, qRoll, qTotal;

	qYaw.w = cos (rYaw / 2);
	qYaw.x = qYaw.y = 0;
	qYaw.z = sin (rYaw / 2);

	qPitch.w = cos (rPitch / 2);
	qPitch.y = qPitch.z = 0;
	qPitch.x = sin (rPitch / 2);

	qRoll.w = cos (rRoll / 2);
	qRoll.x = qRoll.z = 0;
	qRoll.y = sin (rRoll / 2);

	qTotal = qYaw * qPitch * qRoll;

	Matrix4x4 mxRot (qTotal);
	VectorXYZ vecOrientation (0, 0, 1);
	*this = *this * mxRot;
}

void Generic::VectorXYZ::vNormalize ()
{
	real rFactor = rMagnitude ();
	if (!rFactor)
		return;

	x /= rFactor;
	y /= rFactor;
	z /= rFactor;
}

/****************************************************************************/
/* YPR radian <-> degree conversions
*****************************************************************************/

void Generic::VectorYPR::vToRadians ()
{
	yaw *= rDegreesToRadians;
	pitch *= rDegreesToRadians;
	roll *= rDegreesToRadians;
}

void Generic::VectorYPR::vToDegrees ()
{
	yaw *= rRadiansToDegrees;
	pitch *= rRadiansToDegrees;
	roll *= rRadiansToDegrees;
}

/*****************************************************************************
<FOOTER>
*****************************************************************************/
