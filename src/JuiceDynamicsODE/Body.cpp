/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsODE.dll
Filename: Body.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceDynamicsODE.dll and its source code are distributed under the terms of 
the Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>
#include <DynamicsAPI.h>

#include <ode/ode.h>
//#include <ode/dCylinder.h>
#include "DynamicsODE.h"

#include <float.h>
#include <Math.h>

using namespace Juice;
using namespace Juice::Dynamics::ODE;

/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::ODE::Body::Body (ODE::World *pWorld) : 
	m_pWorld (pWorld),
	m_BodyID (null), 
	m_HullID (null),
	m_TackJoint (null)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::ODE::Body::~Body ()
{
	if (m_BodyID)
		dBodyDestroy (m_BodyID);

	if (m_HullID)
		dGeomDestroy (m_HullID);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::ODE::Body::vDelete ()
{
	delete this;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::ODE::Body::vCreate (Juice::Body *pOwner)
{
	// destroy any existing object
	if (m_BodyID)
	{
		dBodyDestroy (m_BodyID);
		m_BodyID = 0;
	}

	// create a new object
	if (m_pWorld && !m_BodyID)
	{
		m_BodyID = dBodyCreate (m_pWorld->m_WorldID);
	}

	// prepare the new object
	if (m_BodyID)
	{
		dBodySetData (m_BodyID, pOwner);
		dBodySetLinearVel  (m_BodyID, 0, 0, 0);
		dBodySetAngularVel (m_BodyID, 0, 0, 0);
	}

	if (m_TackJoint)
	{
		dJointDestroy (m_TackJoint);
		m_TackJoint = null;
	}
}

/****************************************************************************/
/** Return the body's position in Generic::VectorXYZ format
*****************************************************************************/
void Dynamics::ODE::Body::vSetPosition (const VectorXYZ &vecPosition)
{
	if (!m_BodyID)
		return;
	
	dBodySetPosition (m_BodyID, vecPosition.x, vecPosition.y, vecPosition.z);
}

/****************************************************************************/
/** Return the body's position in Generic::VectorXYZ format
*****************************************************************************/
void Dynamics::ODE::Body::vGetPosition (VectorXYZ &vecPosition)
{
	const dReal *pPosition = null; 

	if (m_BodyID)
	{
		pPosition = dBodyGetPosition (m_BodyID);
		vecPosition.x = pPosition[0];
		vecPosition.y = pPosition[1];
		vecPosition.z = pPosition[2];
	}
	else
	{
		vecPosition.x = 0;
		vecPosition.y = 0;
		vecPosition.z = 0;
	}
}

/****************************************************************************/
/** Set the body's orienation using the given yaw, pitch, and roll rotations
*****************************************************************************/
void Dynamics::ODE::Body::vSetRotation (const VectorYPR &vecRotationDegrees)
{
	if (!m_BodyID)
		return;

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

	dQuaternion OdeQuaternion = { qTotal.w, qTotal.x, qTotal.y, qTotal.z };

	dBodySetQuaternion (m_BodyID, OdeQuaternion);
}


/****************************************************************************/
/** This is suspect... not sure if Quaternion::vToEuler works right
*****************************************************************************/
void Dynamics::ODE::Body::vGetRotation (VectorYPR &vecRotation)
{
	if (!m_BodyID)
		return;

	Quaternion qBody;
	vGetQuaternion (qBody);

	qBody.vToEuler (vecRotation);
	vecRotation.vToDegrees ();
}

/****************************************************************************/
/** Get the body's rotation matrix in a form that OpenGL can use
*****************************************************************************/
void Dynamics::ODE::Body::vGetMatrix (Matrix4x4 &mx)
{
	const dReal *pOrientation = dBodyGetRotation (m_BodyID);
	const dReal *pPosition = dBodyGetPosition (m_BodyID);

	vBuildMatrix (mx, pPosition, pOrientation);
}

// If defined, inverted object will give feedback of 180 degrees
//#define AlwaysUpright

// If defined, inverted object will give feedback of zero degrees
#define ZeroInverted

/****************************************************************************/
/** 
*****************************************************************************/
real Dynamics::ODE::Body::rGetRotationYaw ()
{
	if (!m_BodyID)
		return 0;
	
	const dReal *OdeQuat = dBodyGetQuaternion (m_BodyID);
	real w = OdeQuat[0];
	real x = OdeQuat[1];
	real y = OdeQuat[2];
	real z = OdeQuat[3];

	real rQuatAngle = acos (OdeQuat[0]) * 2;

	real rTop = 2 * (w * z + y * z);
	real rBottom = ((w * w) + (x * x) - (y * y) - (z * z));
	real rAngle = atan (rTop / rBottom);

//#ifdef AlwaysUpright
	if (rBottom < 0)
		rAngle = rPi + rAngle;
//#endif

#ifdef ZeroInverted
//	if (rBottom < 0)
//		rAngle = - rAngle;
#endif

//	char szDebug[100];
//	sprintf (szDebug, "Yaw   %6.2f  Top %6.2f  Bottom %6.2f  QuatAngle %6.2f  QuatVector %6.2f %6.2f %6.2f\n", 
//		rAngle, rTop, rBottom, rQuatAngle, OdeQuat[1], OdeQuat[2], OdeQuat[3]);
//	OutputDebugString (szDebug);

	return rAngle;
}

/****************************************************************************/
/** 
*****************************************************************************/
real Dynamics::ODE::Body::rGetRotationPitch ()
{
	if (!m_BodyID)
		return 0;
	
	const dReal *OdeQuat = dBodyGetQuaternion (m_BodyID);
	real w = OdeQuat[0];
	real x = OdeQuat[1];
	real y = OdeQuat[2];
	real z = OdeQuat[3];

	real rQuatAngle = acos (OdeQuat[0]) * 2;

	real rTop = 2 * (w * x + y * z);
	real rBottom = ((w * w) - (x * x) - (y * y) + (z * z));
	real rAngle = atan (rTop / rBottom);

#ifdef AlwaysUpright
	if (rBottom < 0)
		rAngle = rPi + rAngle;
#endif

#ifdef ZeroInverted
	if (rBottom < 0)
		rAngle = - rAngle;
#endif

//	char szDebug[100];
//	sprintf (szDebug, "Pitch %6.2f  Top %6.2f  Bottom %6.2f  QuatAngle %6.2f  QuatVector %6.2f %6.2f %6.2f\n", 
//		rAngle, rTop, rBottom, rQuatAngle, OdeQuat[1], OdeQuat[2], OdeQuat[3]);
//	OutputDebugString (szDebug);

	return rAngle;
}

/****************************************************************************/
/** 
*****************************************************************************/
real Dynamics::ODE::Body::rGetRotationRoll ()
{
	if (!m_BodyID)
		return 0;
	
	const dReal *OdeQuat = dBodyGetQuaternion (m_BodyID);
	real w = OdeQuat[0];
	real x = OdeQuat[1];
	real y = OdeQuat[2];
	real z = OdeQuat[3];

	real rQuatAngle = acos (OdeQuat[0]) * 2;
	
#ifdef AlwaysUpright_Almost
	// actually buggy, but could give an 'alwaysupright' function with a little work

	Quaternion qRotation (w, x, y, z);
	VectorXYZ vecPitchAxis (1, 0, 0);
	real rPitchAngle = rGetRotationPitch ();
	Quaternion qPitch (vecPitchAxis, rPitchAngle);

	Quaternion qRoll = qRotation * qPitch.qInvert ();

	real rAngle = qRoll.rGetAngle ();

	qRoll.vNormalize ();

	char szDebug[200];
	sprintf (szDebug, "Roll  %6.2f  RollQuatVector %6.2f %6.2f %6.2f\n", 
		rAngle, qRoll.x, qRoll.y, qRoll.z);
	OutputDebugString (szDebug);

	return rAngle;
#endif

	real rLeft = x * z;
	real rRight = w * y;
	real rInput = -2 * (rLeft - rRight);
	real rAngle = asin (rInput);

	Quaternion q (w, x, y, z);
	VectorXYZ vecUp (0, 0, 1);
	vecUp.vRotateBy (q);

	if (vecUp.z == 0)
	{
		if (rAngle > 0)
		{
			OutputDebugString ("X\n");
			rAngle = rPi / 2;
		}
		else
		{
			OutputDebugString ("XXX\n");
			rAngle = -rPi / 2;
		}
	}

#ifdef AlwaysUpright
	if (vecUp.z < 0)
	{
		rAngle = rPi - rAngle;
	}
#endif

#ifdef ZeroInverted
	// no-op
#endif

//	char szDebug[200];
//	sprintf (szDebug, "Roll  %6.2f  Left %6.2f  Right %6.2f  Input %6.2f  QuatAngle %6.2f  QuatVector %6.2f %6.2f %6.2f\n", 
//		rAngle, rLeft, rRight, rInput, rQuatAngle, OdeQuat[1], OdeQuat[2], OdeQuat[3]);
//	OutputDebugString (szDebug);

	return rAngle;
}


/****************************************************************************/
/** Return the body's orientation in Generic::Quaternion form
*****************************************************************************/
void Dynamics::ODE::Body::vGetQuaternion (Quaternion &qRotation)
{
	if (!m_BodyID)
		return;

	const dReal *OdeQuaternion = dBodyGetQuaternion (m_BodyID);	
/*
	qRotation.x = OdeQuaternion[0];
	qRotation.y = OdeQuaternion[1];
	qRotation.z = OdeQuaternion[2];
	qRotation.w = OdeQuaternion[3];
*/
	qRotation.w = OdeQuaternion[0];
	qRotation.x = OdeQuaternion[1];
	qRotation.y = OdeQuaternion[2];
	qRotation.z = OdeQuaternion[3];	
}

/****************************************************************************/
/** I don't think this actually gets used.
*****************************************************************************/
real Dynamics::ODE::Body::rGetAngularVelocity (const VectorXYZ &vecAxis)
{
	if (!m_BodyID)
		return 0;

	dReal vecAxisLocal[3] = { vecAxis.x, vecAxis.y, vecAxis.z };

	Quaternion qBody;
	vGetQuaternion (qBody);
	qBody.qInvert ();

	const dReal* prAngularVel = dBodyGetAngularVel (m_BodyID);
	VectorXYZ vecAngularVelocity ((real*) prAngularVel);

	vecAngularVelocity.vRotateBy (qBody);

	if (vecAxis.x == 1)
		return vecAngularVelocity.x;

	if (vecAxis.y == 1)
		return vecAngularVelocity.y;

	if (vecAxis.z == 1)
		return vecAngularVelocity.z;
	
	return 0;
}

/****************************************************************************/
/** At one point I thought this would be useful.  I forgot why.
*****************************************************************************/
/*real Dynamics::ODE::Body::rGetAngularDisplacement (const VectorXYZ &vecOriginalAxis)
{
	if (!m_BodyID)
		return 0;

	Quaternion qBody;
	vGetQuaternion (qBody);

	VectorXYZ vecNewAxis = vecOriginalAxis;
	vecNewAxis.vRotateBy (qBody);

	return rRadiansBetween (vecNewAxis, vecOriginalAxis);
}*/

/****************************************************************************/
/** This is used for damping when pulling objects around with the mouse
*****************************************************************************/
void Dynamics::ODE::Body::vGetVelocity (VectorXYZ &vecVelocity)
{
	if (!m_BodyID)
		return;

	const dReal *pVel = dBodyGetLinearVel (m_BodyID);

	if (pVel)
		vecVelocity.vSet (pVel[0], pVel[1], pVel[2]);
	else
		vecVelocity.vSet (0, 0, 0);
}

/****************************************************************************/
/** Give the body the collision geometry and inertial tensor of a box
*****************************************************************************/
void Dynamics::ODE::Body::vSetFormBox (const VectorXYZ &vecDimensions, real rMass)
{
	if (m_HullID)
		dGeomDestroy (m_HullID);

	if (!m_BodyID)
		return;

	// Collision geometry
	m_HullID = dCreateBox (m_pWorld->m_SpaceID, vecDimensions.x, vecDimensions.y, vecDimensions.z);
	dGeomSetBody (m_HullID, m_BodyID);

	// Inertial tensor
	dMass mass;
	dMassSetBox (&mass, 1, vecDimensions.x, vecDimensions.y, vecDimensions.z);
	dMassAdjust (&mass, rMass);
	dBodySetMass (m_BodyID, &mass);
}

/****************************************************************************/
/** Give the body the collision geometry and inertial tensor of a capsule
*****************************************************************************/
void Dynamics::ODE::Body::vSetFormCapsule (real rLength, real rRadius, real rMass)
{
	if (m_HullID)
		dGeomDestroy (m_HullID);

	if (!m_BodyID)
		return;

	// Collision geometry
	m_HullID = dCreateCCylinder (m_pWorld->m_SpaceID, rRadius, rLength);
	dGeomSetBody (m_HullID, m_BodyID);

	// Inertial tensor
	dMass mass;
	dMassSetCappedCylinder (&mass, 1, 3, rRadius, rLength);
	dMassAdjust (&mass, rMass);
	dBodySetMass (m_BodyID, &mass);
}

/****************************************************************************/
/** In this case, m_HullID actually identifies a GeomTransform object.
 ** Fortunately ODE has a neat feature where destroying the Transform will
 ** destroy the encapsulated collision geometry object as well, so only
 ** the transform handle needs to be saved.
*****************************************************************************/
void Dynamics::ODE::Body::vSetFormCylinder (real rLength, real rRadius, real rMass)
{
	if (m_HullID)
		dGeomDestroy (m_HullID);

	if (!m_BodyID)
		return;

	// Collision geometry
	dGeomID CylinderID = dCreateCylinder (0, rRadius, rLength);

	// cylinder is on Y axis by default, this transform will convert to Z axis
	m_HullID  = dCreateGeomTransform (m_pWorld->m_SpaceID);
	dGeomTransformSetGeom (m_HullID, CylinderID);

	// cleanup mode 1: destroy the encapsulated object
	dGeomTransformSetCleanup (m_HullID, 1);

	// reporting mode 0: use the ID of the encapsulated object in collision callbacks
	dGeomTransformSetInfo (m_HullID, 0);

	dMatrix3 RYtoZ;
	dRFromAxisAndAngle (RYtoZ, 1, 0, 0, rPi / 2);
	dGeomSetRotation (CylinderID, RYtoZ);

	dGeomSetBody (m_HullID, m_BodyID);

	// Inertial tensor (cheat a bit, use the capsule form)
	dMass mass;
	dMassSetCappedCylinder (&mass, 1, 3, rRadius, rLength);
	dMassAdjust (&mass, rMass);
	dBodySetMass (m_BodyID, &mass);
}

/****************************************************************************/
/** Give the body the collision geometry and inertial tensor of a sphere
*****************************************************************************/
void Dynamics::ODE::Body::vSetFormBall (real rRadius, real rMass)
{
	if (m_HullID)
		dGeomDestroy (m_HullID);

	if (!m_BodyID)
		return;

	// Collision geometry
	m_HullID = dCreateSphere (m_pWorld->m_SpaceID, rRadius);
	dGeomSetBody (m_HullID, m_BodyID);

	// Inertial tensor
	dMass mass;
	dMassSetSphere (&mass, 1, rRadius);
	dMassAdjust (&mass, rMass);
	dBodySetMass (m_BodyID, &mass);
}

/****************************************************************************/
/** This is used when pulling objects around with the mouse
*****************************************************************************/
void Dynamics::ODE::Body::vAddForce (const VectorXYZ &vecForce)
{
	if (!m_BodyID)
		return;

	dBodyAddForce (m_BodyID, vecForce.x, vecForce.y, vecForce.z);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::ODE::Body::vSetStatic (bool fStatic)
{
	if (!m_BodyID)
		return;

	if (fStatic)
	{
		if (!m_TackJoint)
			m_TackJoint = dJointCreateFixed (m_pWorld->m_WorldID, 0);

		if (m_TackJoint)
		{
			dJointAttach (m_TackJoint, m_BodyID, null);
			dJointSetFixed (m_TackJoint);
		}
	}
	else
	{
		if (m_TackJoint)
		{
			dJointDestroy (m_TackJoint);
			m_TackJoint = null;
		}
	}
}

// Body.cpp ends here ->