/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: Vector.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

I should note that most of the stuff in this file is pretty basic and there's
no way I could be bothered to care if you re-used it in any way.
*****************************************************************************/

#include <GenericTypes.h>
#include <math.h>

using namespace Generic;

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

real VectorXYZ::rMagnitude () const
{
	return sqrt (x*x + y*y + z*z);
}

VectorXYZ Generic::vecProject (VectorXYZ &vecProject, VectorXYZ &vecOnto)
{
	real rFactor = (DotProduct (vecProject, vecOnto)) / (DotProduct (vecOnto, vecOnto));
	VectorXYZ vecResult = vecOnto * rFactor;
	return vecResult;
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


VectorXYZ Generic::operator * (const VectorXYZ &v, const Matrix4x4 &a)
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

VectorXYZ& Generic::operator -= (VectorXYZ &a, const VectorXYZ &b)
{
	a = a - b;
	return a;
}

VectorXYZ& Generic::operator /= (VectorXYZ &a, real r)
{
	a.x /= r;
	a.y /= r;
	a.z /= r;
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

real Generic::rRadiansBetween (const VectorXYZ &a, const VectorXYZ &b)
{
	real rNumerator = DotProduct (a, b);
	real rDenominator = a.rMagnitude() * b.rMagnitude ();
	return acos (rNumerator / rDenominator);
}

/****************************************************************************/
/** Rotate a vector around a quaternion
*****************************************************************************/
void Generic::VectorXYZ::vRotateBy (const Generic::Quaternion &q)
{
#ifdef MethodOne
	real rFactor1 = 0, rFactor2 = 0, rFactor3 = 0;

	rFactor1 = (2 * q.w * q.w) - 1;
	rFactor2 = 2 * ((q.x * x) + (q.y * y) + (q.z * z));
	rFactor3 = 2 * q.w * q.w;

	x = (rFactor1 * x) + (rFactor2 * q.x) + (rFactor3 * ((q.y * z) - (q.z * y)));
	y = (rFactor1 * y) + (rFactor2 * q.y) + (rFactor3 * ((q.z * x) - (q.x * z)));
	z = (rFactor1 * z) + (rFactor2 * q.z) + (rFactor3 * ((q.x * y) - (q.y * x)));
#endif
#ifdef MethodTwo
	real phi = q.rGetAngle ();
	VectorXYZ vecAxis;
	q.vGetAxis (vecAxis);
	vecAxis.vNormalize ();
	VectorXYZ vecNormal = CrossProduct (vecAxis, *this);

	x = cos (phi) + ((1 - cos (phi)) * (vecAxis.x * x) * vecAxis.x) - (sin (phi) * vecNormal.x);
	y = cos (phi) + ((1 - cos (phi)) * (vecAxis.y * y) * vecAxis.y) - (sin (phi) * vecNormal.y);
	z = cos (phi) + ((1 - cos (phi)) * (vecAxis.z * x) * vecAxis.z) - (sin (phi) * vecNormal.z);
#endif

	// From Si Brown's buggy demo (http://freefall.freehosting.net/downloads/buggydemo.html)

	// form ([0,v].~q) directly
	Quaternion t(
		x * q.x + y * q.y + z * q.z, 
		x * q.w + z * q.y - y * q.z,
		y * q.w + x * q.z - z * q.x, 			
		z * q.w + y * q.x - x * q.y
	); 

	// return the last three components of (q.[0,v].~q) as a vector
	x = q.w * t.x + q.x * t.w + q.y * t.z - q.z * t.y;
	y = q.w * t.y + q.y * t.w + q.z * t.x - q.x * t.z;
	z = q.w * t.z + q.z * t.w + q.x * t.y - q.y * t.x;
}

/****************************************************************************/
/** Rotate a vector around a give yaw, pitch, and roll.
*****************************************************************************/
void Generic::VectorXYZ::vRotateByDegrees (const VectorYPR &vecRotationDegrees)
{
	real rYaw   = rDegreesToRadians * vecRotationDegrees.yaw;
	real rPitch = rDegreesToRadians * -vecRotationDegrees.pitch;
	real rRoll  = rDegreesToRadians * -vecRotationDegrees.roll;

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

	vRotateBy (qTotal);
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

using namespace Generic::XML;

void VectorXYZ::vSerialize (Stream &Stream)
{
	Stream.vSerialize (&x, L"X");
	Stream.vSerialize (&y, L"Y");
	Stream.vSerialize (&z, L"Z");
}

// VectorXYZ.cpp ends here ->