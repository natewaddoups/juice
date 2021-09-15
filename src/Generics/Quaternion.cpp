/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: Quaternion.cpp

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


/****************************************************************************/
/* Quaternions
*****************************************************************************/

Quaternion::Quaternion () :
	w(0), x(0), y(0), z(0)
{
}

Quaternion::Quaternion (real rW, real rX, real rY, real rZ) :
	w (rW), x (rX), y (rY), z (rZ)
{
}

Quaternion::Quaternion (const VectorXYZ &vecAxis, real rAngle)
{
	vFromAxisAndAngle (vecAxis, rAngle);
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

void Quaternion::vFromAxisAndAngle (const VectorXYZ &vecAxis, real rAngle)
{
	real rSinPhi = sin (rAngle / 2);

	w = cos (rAngle / 2);
	x = vecAxis.x * rSinPhi;
	y = vecAxis.y * rSinPhi;
	z = vecAxis.z * rSinPhi;
}

Quaternion& Quaternion::qInvert ()
{
	real rMagnitude = sqrt(w*w + x*x + y*y + z*z);
	if (rMagnitude == 0.0)
	rMagnitude = 1.0;

	real rRecip = 1.0 / rMagnitude;

	w =  w * rRecip;
	x = -x * rRecip;
	y = -y * rRecip;
	z = -z * rRecip;

	return *this;
}

void Quaternion::vGetAxis (VectorXYZ &vecAxis) const
{
	vecAxis.x = x;
	vecAxis.y = y;
	vecAxis.z = z;
}

real Quaternion::rGetAngle () const
{
	return acos (w) * 2;
}

//#define UseTheMatrixMethod

#ifndef UseTheMatrixMethod

/****************************************************************************/
/* Written with the help of http://www.nearfield.com/~dan/misc/Quater/
*****************************************************************************/
/// FIXFIX: this may not work right.  Needs testing to verify.
void Quaternion::vToEuler (VectorYPR &vecRotations)
{
	real q1 = x;
	real q2 = y;
	real q3 = z;
	real q4 = w;

	vecRotations.yaw   = atan (2 * (q1 * q2 + q4 * q3) / ((q4 * q4) + (q1 * q1) - (q2 * q2) - (q3 * q3)));
	vecRotations.pitch = atan (2 * (q4 * q1 + q2 * q3) / ((q4 * q4) - (q1 * q1) - (q2 * q2) + (q3 * q3)));
	vecRotations.roll  = asin (-2 * (q1 * q3 - q4 * q2));
}

#else

///////////////////////////////////////////////////////////////////////////////
//
// Adopted from http://www.darwin3d.com/gamedev/quat2eul.cpp
// Function:	QuatToEuler
// Purpose:		Convert a Quaternion back to Euler Angles
// Arguments:	Quaternions and target Euler vector
// Notes:		The method is to convert Quaternion to a 3x3 matrix and
//				decompose the matrix.  This is subject to the
//				ambiguities of square roots and problems with inverse trig.
//				Matrix to Euler conversion is really very ill-defined but works
//				for my purposes.
//
//              Since this was not written by Nate Waddoups, the GPL terms
//              do not apply here.  However, since the author posted this code
//              as how-to sort of thing, it's probably safe to consider it
//              freely redistributable.
///////////////////////////////////////////////////////////////////////////////

/// FIXFIX: this may not work right either.  Needs testing to verify.
void QuatToEuler(real quat_w, real quat_x, real quat_y, real quat_z, real &euler_x, real &euler_y, real &euler_z)
{
/// Local Variables ///////////////////////////////////////////////////////////
	double matrix[3][3];
	double cx,sx;
	double cy,sy,yr;
	double cz,sz;

	// Convert quaternion to matrix
	matrix[0][0] = 1.0f - (2.0f * quat_y * quat_y) - (2.0f * quat_z * quat_z);
//	matrix[0][1] = (2.0f * quat_x * quat_y) - (2.0f * quat_w * quat_z);
//	matrix[0][2] = (2.0f * quat_x * quat_z) + (2.0f * quat_w * quat_y);
	matrix[1][0] = (2.0f * quat_x * quat_y) + (2.0f * quat_w * quat_z);
//	matrix[1][1] = 1.0f - (2.0f * quat_x * quat_x) - (2.0f * quat_z * quat_z);
//	matrix[1][2] = (2.0f * quat_y * quat_z) - (2.0f * quat_w * quat_x);
	matrix[2][0] = (2.0f * quat_x * quat_z) - (2.0f * quat_w * quat_y);
	matrix[2][1] = (2.0f * quat_y * quat_z) + (2.0f * quat_w * quat_x);
	matrix[2][2] = 1.0f - (2.0f * quat_x * quat_x) - (2.0f * quat_y * quat_y);

	sy = -matrix[2][0];
	cy = sqrt(1 - (sy * sy));
	yr = (double)atan2(sy,cy);
	euler_y = (yr * 180.0f) / rPi;

	if (sy != 1.0f && sy != -1.0f)	
	{
		cx = matrix[2][2] / cy;
		sx = matrix[2][1] / cy;
		euler_x = ((double)atan2(sx,cx) * 180.0f) / rPi;	// RAD TO DEG

		cz = matrix[0][0] / cy;
		sz = matrix[1][0] / cy;
		euler_z = ((double)atan2(sz,cz) * 180.0f) / rPi;	// RAD TO DEG
	}
	else
	{
		// SINCE Cos(Y) IS 0, I AM SCREWED.  ADOPT THE STANDARD Z = 0
		// I THINK THERE IS A WAY TO FIX THIS BUT I AM NOT SURE.  EULERS SUCK
		// NEED SOME MORE OF THE MATRIX TERMS NOW
		matrix[1][1] = 1.0f - (2.0f * quat_x * quat_x) - (2.0f * quat_z * quat_z);
		matrix[1][2] = (2.0f * quat_y * quat_z) - (2.0f * quat_w * quat_x);
		cx = matrix[1][1];
		sx = -matrix[1][2];
		euler_x = ((double)atan2(sx,cx) * 180.0f) / rPi;	// RAD TO DEG

		cz = 1.0f;
		sz = 0.0f;
		euler_z = ((double)atan2(sz,cz) * 180.0f) / rPi;	// RAD TO DEG
	}
}
// QuatToEuler  ///////////////////////////////////////////////////////////////

/// FIXFIX: this may not work right either.  Needs testing to verify.
void Quaternion::vToEuler (VectorYPR &vecRotations)
{
	QuatToEuler (x, z, y, z, 
		vecRotations.pitch,
		vecRotations.roll,
		vecRotations.yaw);
}

#endif

void Quaternion::vFromTwoVectors (const VectorXYZ &vecSource, const VectorXYZ &vecDestination)
{
	real rAngleBetween = rRadiansBetween (vecSource, vecDestination);
	if (rAngleBetween < 0.01)
	{
		w = 0;
		x = 0; 
		y = 0;
		x = 1;
		vNormalize ();
		return;
	}

	VectorXYZ vecAxis = Generic::CrossProduct (vecSource, vecDestination);

	vFromAxisAndAngle (vecAxis, rAngleBetween);
}

// Quaternion.cpp ends here ->