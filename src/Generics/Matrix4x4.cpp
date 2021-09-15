/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: Matrix4x4.cpp

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
#define matrix(row,col) m_Elements[(row<<2)+col]

/****************************************************************************/
/** Adapted from GLU's matmul4
*****************************************************************************/
Matrix4x4 Generic::operator * (const Matrix4x4 &a, const Matrix4x4 &b)
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

Matrix4x4::Matrix4x4 (const Quaternion &qRotation)
{
	vSetRotation (qRotation);

	matrix(3,0) = 0;
	matrix(3,1) = 0;
	matrix(3,2) = 0;
	matrix(3,3) = 1;
}

void Matrix4x4::vSetRotation (const Quaternion &rotation)
{
	Quaternion quat = rotation;
	quat.vNormalize ();
	matrix(0,0) = 1.0 - 2*quat.y*quat.y - 2*quat.z*quat.z;
	matrix(1,0) = 2*quat.x*quat.y + 2*quat.w*quat.z;
	matrix(2,0) = 2*quat.x*quat.z - 2*quat.w*quat.y;

	matrix(0,1) = 2*quat.x*quat.y - 2*quat.w*quat.z;
	matrix(1,1) = 1.0 - 2*quat.x*quat.x - 2*quat.z*quat.z;
	matrix(2,1) = 2*quat.y*quat.z + 2*quat.w*quat.x;

	matrix(0,2) = 2*quat.x*quat.z + 2*quat.w*quat.y;
	matrix(1,2) = 2*quat.y*quat.z - 2*quat.w*quat.x;
	matrix(2,2) = 1.0 - 2*quat.x*quat.x - 2*quat.y*quat.y;
}

void Matrix4x4::vSetTranslation (const VectorXYZ &vecTranslation)
{
	matrix(3,0) = vecTranslation.x;
	matrix(3,1) = vecTranslation.y;
	matrix(3,2) = vecTranslation.z;
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

void Matrix4x4::vTranspose (Matrix4x4 &mResult)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mResult.m_Elements[j*4+i] = m_Elements[i*4+j];
		}
	}
}

void Matrix4x4::vFromTwoAxes (VectorXYZ xyz1, VectorXYZ xyz2)
{
	VectorXYZ vecAxis = CrossProduct (xyz1, xyz2);
	real rAngle = rRadiansBetween (xyz1, xyz2);
	Quaternion q (vecAxis, rAngle);
	vSetRotation (q);
}

// Matrix4x4.cpp ends here ->