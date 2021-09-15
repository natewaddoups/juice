/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsNovodex.dll
Filename: Body.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceDynamicsNovodex.dll and its source code are distributed under the terms
of the Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <NxFoundation.h>
#include <NxRigidBody.h>
#include <NxCollision.h>

#include <JuiceEngine.h>
#include <DynamicsAPI.h>
#include "DynamicsNovodex.h"

#include <float.h>
#include <Math.h>

using namespace Juice;
using namespace Juice::Dynamics::Novodex;

/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::Novodex::Body::Body (Novodex::World *pWorld) : 
	m_pWorld (pWorld),
	m_pBody (null), 
	m_pShape (null),
	m_fTacked (false)
//	m_pTackJoint (null)
{
}

/****************************************************************************/
/** Destructor 
*****************************************************************************/
Dynamics::Novodex::Body::~Body ()
{
	if (m_pBody)
	{
		if (m_pWorld)
			m_pWorld->vRemoveBody (this);

		m_pBody->release ();
		m_pBody = null;
	}

	if (m_pShape)
	{
		if (m_pWorld)
			m_pWorld->vRemoveShape (m_pShape);

		m_pShape->release ();
		m_pShape = null;
	}
}

/****************************************************************************/
/** Use of this method ensures that the object is deleted from the same heap
 ** from which it was allocated.
*****************************************************************************/
void Dynamics::Novodex::Body::vDelete ()
{
	delete this;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::Novodex::Body::vCreate (Juice::Body *pOwner)
{
	m_pOwner = pOwner;

	// destroy any existing object
	if (m_pBody)
	{
		m_pWorld->vRemoveBody (this);
		m_pBody->release ();
		m_pBody = 0;
	}

	// create a new object
	if (m_pWorld && m_pWorld->m_pDynamicsFactory && !m_pBody)
	{
		m_pBody = m_pWorld->m_pDynamicsFactory->createBody ();
	}

	// prepare the new object
	if (m_pWorld && m_pBody)
	{
		m_pBody->setUserData (this);
		m_pBody->setVelocity (NxrVec3 (0, 0, 0));
		m_pBody->setAngularVelocity (NxrVec3 (0, 0, 0));

		// Add the body to the scene
		m_pWorld->vAddBody (this);
	}


//	if (m_pTackJoint)
//	{
//		dJointDestroy (m_TackJoint);
//		m_TackJoint = null;
//	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::Novodex::Body::vSetPosition (const VectorXYZ &vecPosition)
{
	if (!m_pBody)
		return;
	
	m_pBody->setPosition (NxrVec3 (vecPosition.x,vecPosition.y, vecPosition.z));
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::Novodex::Body::vGetPosition (VectorXYZ &vecPosition)
{
	if (m_pBody)
	{
		NxrVec3 nx;
		m_pBody->getPosition (nx);
		vecPosition.x = nx.x ();
		vecPosition.y = nx.y ();
		vecPosition.z = nx.z ();
	}
	else
	{
		vecPosition.x = 0;
		vecPosition.y = 0;
		vecPosition.z = 0;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::Novodex::Body::vSetRotation (const VectorYPR &vecRotationDegrees)
{
	if (!m_pBody)
		return;

	real rYaw   = rDegreesToRadians * vecRotationDegrees.yaw;
	real rPitch = rDegreesToRadians * vecRotationDegrees.pitch;
	real rRoll  = rDegreesToRadians * vecRotationDegrees.roll;

	NxrQuat qYaw, qPitch, qRoll, qTotal;

	qYaw.setWXYZ (
		cos (rYaw / 2),
		0,
		0,
		sin (rYaw / 2));

	qPitch.setWXYZ (
		cos (rPitch / 2),
		sin (rPitch / 2),
		0,
		0);	

	qRoll.setWXYZ (
		cos (rRoll / 2),
		0,
		sin (rRoll / 2),
		0);

	qTotal = qYaw;
	qTotal *= qPitch;
	qTotal *= qRoll;

	m_pBody->setOrientation (qTotal);
}


/****************************************************************************/
/** This is suspect... not sure if Quaternion::vToEuler works right
*****************************************************************************/
void Dynamics::Novodex::Body::vGetRotation (VectorYPR &vecRotation)
{
	if (!m_pBody)
		return;

	NxrQuat qNovodex;
	m_pBody->getOrientation (qNovodex);

	Generic::Quaternion qGeneric (qNovodex.w (), qNovodex.x (), qNovodex.y (), qNovodex.z ());

	qGeneric.vToEuler (vecRotation);
	vecRotation.vToDegrees ();
}


/****************************************************************************/
/**
*****************************************************************************/
void Dynamics::Novodex::Body::vGetMatrix (Matrix4x4 &mx)
{
	if (!m_pBody)
		return;

	const NxrMat33 mxOrientation = m_pBody->getOrientationMatrix ();
	const NxrVec3 vecPosition = m_pBody->getPosition ();

	vBuildMatrix (mx, vecPosition, mxOrientation);
}

/****************************************************************************/
/**
*****************************************************************************/
real Dynamics::Novodex::Body::rGetRotationYaw ()
{
	if (!m_pBody)
		return 0;
	
	//const dReal *OdeQuat = dBodyGetQuaternion (m_BodyID);
	const NxrQuat nxQuat = m_pBody->getOrientation ();
	real w = nxQuat.w ();
	real x = nxQuat.x ();
	real y = nxQuat.y ();
	real z = nxQuat.z ();

	real rQuatAngle = acos (nxQuat.w ()) * 2;

	real rTop = 2 * (w * z + y * z);
	real rBottom = ((w * w) + (x * x) - (y * y) - (z * z));
	real rAngle = atan (rTop / rBottom);

	if (rBottom < 0)
		rAngle = rPi + rAngle;

//	char szDebug[100];
//	sprintf (szDebug, "Yaw   %6.2f  Top %6.2f  Bottom %6.2f  QuatAngle %6.2f  QuatVector %6.2f %6.2f %6.2f\n", 
//		rAngle, rTop, rBottom, rQuatAngle, OdeQuat[1], OdeQuat[2], OdeQuat[3]);
//	OutputDebugString (szDebug);

	return rAngle;
}

/****************************************************************************/
/**
*****************************************************************************/
real Dynamics::Novodex::Body::rGetRotationPitch ()
{
	if (!m_pBody)
		return 0;
	
	//const dReal *OdeQuat = dBodyGetQuaternion (m_BodyID);
	const NxrQuat nxQuat = m_pBody->getOrientation ();
	real w = nxQuat.w ();
	real x = nxQuat.x ();
	real y = nxQuat.y ();
	real z = nxQuat.z ();

	real rQuatAngle = acos (nxQuat.w ()) * 2;

	real rTop = 2 * (w * x + y * z);
	real rBottom = ((w * w) - (x * x) - (y * y) + (z * z));
	real rAngle = atan (rTop / rBottom);

	if (rBottom < 0)
		rAngle = - rAngle;

//	char szDebug[100];
//	sprintf (szDebug, "Pitch %6.2f  Top %6.2f  Bottom %6.2f  QuatAngle %6.2f  QuatVector %6.2f %6.2f %6.2f\n", 
//		rAngle, rTop, rBottom, rQuatAngle, OdeQuat[1], OdeQuat[2], OdeQuat[3]);
//	OutputDebugString (szDebug);

	return rAngle;
}

/****************************************************************************/
/**
*****************************************************************************/
real Dynamics::Novodex::Body::rGetRotationRoll ()
{
	if (!m_pBody)
		return 0;
	
	//const dReal *OdeQuat = dBodyGetQuaternion (m_BodyID);
	const NxrQuat nxQuat = m_pBody->getOrientation ();
	real w = nxQuat.w ();
	real x = nxQuat.x ();
	real y = nxQuat.y ();
	real z = nxQuat.z ();

	real rQuatAngle = acos (nxQuat.w ()) * 2;
	
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

//	char szDebug[200];
//	sprintf (szDebug, "Roll  %6.2f  Left %6.2f  Right %6.2f  Input %6.2f  QuatAngle %6.2f  QuatVector %6.2f %6.2f %6.2f\n", 
//		rAngle, rLeft, rRight, rInput, rQuatAngle, OdeQuat[1], OdeQuat[2], OdeQuat[3]);
//	OutputDebugString (szDebug);

	return rAngle;
}


/****************************************************************************/
/**
*****************************************************************************/
void Dynamics::Novodex::Body::vGetQuaternion (Quaternion &qRotation)
{
	if (!m_pBody)
		return;

	NxrQuat qNovodex;
	m_pBody->getOrientation (qNovodex);

	Generic::Quaternion qGeneric (qNovodex.w (), qNovodex.x (), qNovodex.y (), qNovodex.z ());
	qRotation.w = qNovodex.w ();
	qRotation.x = qNovodex.x ();
	qRotation.y = qNovodex.y ();
	qRotation.z = qNovodex.z ();
}

/****************************************************************************/
/**
*****************************************************************************/
real Dynamics::Novodex::Body::rGetAngularVelocity (const VectorXYZ &vecAxis)
{
/*	if (!m_pBody)
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
*/	
	return 0;
}

/****************************************************************************/
/**
*****************************************************************************/
void Dynamics::Novodex::Body::vGetVelocity (VectorXYZ &vecVelocity)
{
	if (!m_pBody)
		return;

	NxrVec3 nxVec;
	m_pBody->getVelocity (nxVec);
	vecVelocity.x = nxVec.x ();
	vecVelocity.y = nxVec.y ();
	vecVelocity.z = nxVec.z ();
}

/****************************************************************************/
/**
*****************************************************************************/
void Dynamics::Novodex::Body::vReleaseShape ()
{
	if (m_pShape)
	{
		m_pWorld->vRemoveShape (m_pShape);
		m_pShape->release ();
		m_pShape = null;
	}
}

/****************************************************************************/
/**
*****************************************************************************/
void Dynamics::Novodex::Body::vSetFormBox (const VectorXYZ &vecDimensions, real rMass)
{
	vReleaseShape ();
	
	NxBox *pBox = 0;

	if (m_pWorld && m_pWorld->m_pCollisionFactory)
		pBox = m_pWorld->m_pCollisionFactory->createBox ();

	if (!pBox)
		return;

	pBox->setDimensions  (NxrVec3 (vecDimensions.x / 2, vecDimensions.y / 2, vecDimensions.z / 2));

	// Position
	m_vecShapePosition = m_pBody->getPosition ();
	pBox->setPosition (m_vecShapePosition);

	// Orientation
	m_mxShapeOrientation = m_pBody->getOrientationMatrix ();
	pBox->setOrientation (m_mxShapeOrientation);

	// User data
	m_pShape = &pBox->getShape ();
	m_pShape->setUserData (this);

	// Add the shape to the world
	m_pWorld->vAddShape (m_pShape);

	// Set the inertial tensor
	m_pBody->setMass (rMass);
	m_pBody->setInertiaTensorAsBox (vecDimensions.x, vecDimensions.y, vecDimensions.z);
}


/****************************************************************************/
/** Novodex doesn't actually support tubes yet
*****************************************************************************/
void Dynamics::Novodex::Body::vSetFormCylinder (real rLength, real rRadius, real rMass)
{
	vReleaseShape ();

	NxBox *pBox = 0;

	if (m_pWorld && m_pWorld->m_pCollisionFactory)
		pBox = m_pWorld->m_pCollisionFactory->createBox ();

	if (!pBox)
		return;

	pBox->setDimensions (NxrVec3 (rRadius * 2, rRadius * 2, rLength));

	// Position
	m_vecShapePosition = m_pBody->getPosition ();
	pBox->setPosition (m_vecShapePosition);

	// Orientation
	m_mxShapeOrientation = m_pBody->getOrientationMatrix ();
	pBox->setOrientation (m_mxShapeOrientation);

	// User data
	m_pShape = &pBox->getShape ();
	m_pShape->setUserData (this);

	// Add it to the world
	m_pWorld->vAddShape (m_pShape);

	// Set the inertial tensor
	m_pBody->setMass (rMass);
	m_pBody->setInertiaTensorAsBox (rRadius * 2, rRadius * 2, rLength);
}

/****************************************************************************/
/**
*****************************************************************************/
void Dynamics::Novodex::Body::vSetFormCapsule (real rLength, real rRadius, real rMass)
{
	vSetFormCylinder (rLength, rRadius, rMass);
}

/****************************************************************************/
/**
*****************************************************************************/
void Dynamics::Novodex::Body::vSetFormBall (real rRadius, real rMass)
{
	vReleaseShape ();

	NxSphere *pSphere = 0;

	if (m_pWorld && m_pWorld->m_pCollisionFactory)
		pSphere = m_pWorld->m_pCollisionFactory->createSphere ();

	if (!pSphere)
		return;

	pSphere->setRadius (rRadius);

	// Position
	m_vecShapePosition = m_pBody->getPosition ();
	pSphere->setPosition (m_vecShapePosition);

	// User data
	m_pShape = &pSphere->getShape ();
	m_pShape->setUserData (this);

	// Add to the world
	m_pWorld->vAddShape (m_pShape);

	// Set the inertial tensor
	m_pBody->setMass (rMass);
	m_pBody->setInertiaTensorAsSphere (rRadius, false);
}

/****************************************************************************/
/**
*****************************************************************************/
void Dynamics::Novodex::Body::vAddForce (const VectorXYZ &vecForce)
{
	if (!m_pBody)
		return;

	m_pBody->addForce (NxrVec3 (vecForce.x, vecForce.y, vecForce.z));
}

/****************************************************************************/
/**
*****************************************************************************/
void Dynamics::Novodex::Body::vSetStatic (bool fStatic)
{
	m_fTacked = true;

/*
	if (!m_pBody)
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
*/
}

void Dynamics::Novodex::Body::vPoseChange ()
{
	if (!m_pShape)
		return;

	NxBox *pBox = m_pShape->isBox ();
	if (pBox)
	{
		m_vecShapePosition = m_pBody->getPosition ();
		pBox->setPosition (m_vecShapePosition);

		m_mxShapeOrientation = m_pBody->getOrientationMatrix ();
		pBox->setOrientation (m_mxShapeOrientation);
		return;
	}

	NxSphere *pSphere = m_pShape->isSphere ();
	if (pSphere)
	{
		m_vecShapePosition = m_pBody->getPosition ();
		pSphere->setPosition (m_vecShapePosition);
	}
}

// Body.cpp ends here ->