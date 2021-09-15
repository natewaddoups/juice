/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenericMath.h 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#ifndef __GenericMath__
#define __GenericMath__

namespace Generic
{

/****************************************************************************/
/** 
*****************************************************************************/
class Quaternion;
class VectorYPR;
class VectorXYZ;	
class Matrix4x4;

/****************************************************************************/
/** This is used for rotations
*****************************************************************************/

	class GenericsExport Quaternion
	{
	public:
		real w;
		real x;
		real y;
		real z;

		Quaternion ();
		Quaternion (const VectorXYZ &vecAxis, real rAngle);
		Quaternion (real rW, real rX, real rY, real rZ);
		virtual ~Quaternion ();

		void vNormalize ();

		Quaternion& qInvert ();

		void vToEuler (VectorYPR &vecRotations);

		void vGetAxis (VectorXYZ &vecAxis) const;
		real rGetAngle () const;

		void vFromTwoVectors (const VectorXYZ &vecSource, const VectorXYZ &vecDestination);
		void vFromAxisAndAngle (const VectorXYZ &vecAxis, real rAngle);
	};

	Quaternion GenericsExport operator * (const Quaternion &a, const Quaternion &b);

/****************************************************************************/
/** This is used to define orientation via yaw, pitch, and roll rotations.
*****************************************************************************/

	class GenericsExport VectorYPR : public XML::Serializable
	{
	public:

        real yaw;
		real pitch;
		real roll;

		VectorYPR& vSet (real y, real p, real r) { yaw = y; pitch = p; roll = r; return *this; }
		VectorYPR (real y = 0, real p = 0, real r = 0) { yaw = y; pitch = p; roll = r; }
		void vToRadians ();
		void vToDegrees ();

		void vGetMatrix (Matrix4x4 &mxRotation);

		VectorYPR operator- () { VectorYPR result (-yaw, -pitch, -roll); return result; }

		virtual void vSerialize (XML::Stream &Stream);
	};

/****************************************************************************/
/** This is used to define vectors, points, and so on.
*****************************************************************************/

	class GenericsExport VectorXYZ : public XML::Serializable
	{
	public:

        real x;
		real y;
		real z;

		//VectorXYZ () : x(0), y(0), z(0)
		//{
		//}

		VectorXYZ (real xx = 0, real yy = 0, real zz = 0) : x (xx), y (yy), z (zz)
		{ 
		}

		VectorXYZ (real *p) : x (p[0]), y (p[1]), z (p[2]) {}

		VectorXYZ& vSet (real xx, real yy, real zz)
        {
			x = xx; y = yy; z = zz; return *this;
		}

		void vRotateBy (const Quaternion &q);
		void vRotateByDegrees (const VectorYPR &vecRotation);
		real rMagnitude () const;
		void vNormalize ();		

		virtual void vSerialize (XML::Stream &Stream);

		// this is only a test, do not take it seriously just yet
		operator real* () { return &x; }
	};

	VectorXYZ GenericsExport operator - (const VectorXYZ &a);
	VectorXYZ GenericsExport operator + (const VectorXYZ &a, real r);
	VectorXYZ GenericsExport operator + (const VectorXYZ &a, const VectorXYZ &b);
	VectorXYZ GenericsExport &operator += (VectorXYZ &a, const VectorXYZ &b);
	VectorXYZ GenericsExport &operator -= (VectorXYZ &a, const VectorXYZ &b);
	VectorXYZ GenericsExport &operator /= (VectorXYZ &a, real r);
	VectorXYZ GenericsExport operator - (const VectorXYZ &a, const VectorXYZ &b);
	VectorXYZ GenericsExport operator * (const VectorXYZ &a, real r);
	real      GenericsExport DotProduct (const VectorXYZ &a, const VectorXYZ &b);
	VectorXYZ GenericsExport CrossProduct (const VectorXYZ &a, const VectorXYZ &b);

	real GenericsExport rRadiansBetween (const VectorXYZ &a, const VectorXYZ &b);

	VectorXYZ GenericsExport vecIntersectionOfLineAndPlane (
		const VectorXYZ &vecLineOrigin, 
		const VectorXYZ &vecLineDirection, 
		const VectorXYZ &vecPlaneNormal, 
		const VectorXYZ &vecPointOnPlane);

	VectorXYZ GenericsExport vecProject (VectorXYZ &vecProject, VectorXYZ &vecOnto);

/****************************************************************************/
/** 
*****************************************************************************/

	class GenericsExport Matrix4x4
	{
	protected:
		real m_Elements[16];
	public:
		Matrix4x4 ();
		Matrix4x4 (const Quaternion &qRotation);

		void vSetIdentity ();
		void vSetRotation (const Quaternion &qRotation);
		void vSetTranslation (const VectorXYZ &vecTranslation);
		void vFromTwoAxes (VectorXYZ xyz1, VectorXYZ xyz2);

		void vTranspose (Matrix4x4 &mResult);

		real& operator[](int i) { return m_Elements[i]; }
		operator real*() { return m_Elements; }
		operator const real*() const { return m_Elements; }

		friend Matrix4x4 GenericsExport operator* (const Matrix4x4 &a, const Matrix4x4 &b);
		friend VectorXYZ GenericsExport operator* (const VectorXYZ &v, const Matrix4x4 &m);

	};

	Matrix4x4 GenericsExport operator* (const Matrix4x4 &a, const Matrix4x4 &b);
	VectorXYZ GenericsExport operator* (const VectorXYZ &v, const Matrix4x4 &a);

};

#endif // ifdef __GenericMath__

// GenericMath.h ends here ->