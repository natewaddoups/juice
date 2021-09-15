/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsNovodex.dll
Filename: RevoluteJoint.cpp

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
using namespace Juice::Dynamics;
//using namespace Juice::Dynamics::Novodex;

/****************************************************************************/
/** 
*****************************************************************************/
Novodex::RevoluteJoint::RevoluteJoint (Novodex::World *pWorld) : Joint (pWorld)
{
}

/****************************************************************************/
/**
*****************************************************************************/
Novodex::RevoluteJoint::~RevoluteJoint ()
{
}

/****************************************************************************/
/**
*****************************************************************************/
NxHingeJoint* Novodex::RevoluteJoint::pGetJoint ()
{
	NxHingeJoint *pJoint = null;

	if (m_pJoint)
		return m_pJoint->isHingeJoint ();

	return 0;
}

/****************************************************************************/
/**
*****************************************************************************/
void Novodex::RevoluteJoint::vDelete ()
{
	delete this;
}

/****************************************************************************/
/**
*****************************************************************************/
bool Novodex::RevoluteJoint::fInSimulation ()
{
	return fJointInSimulation ();
}

/****************************************************************************/
/**
*****************************************************************************/
void Novodex::RevoluteJoint::vCreate (Juice::RevoluteJoint *pOwner,
									  Dynamics::Body *pBody1, Dynamics::Body *pBody2)
{
	// This will set m_pWorld, m_pBody1, m_pBody2
	if (!Joint::fJointCreate (pOwner, pBody1, pBody2))
		return;

	// Create the joint
	NxHingeJoint *pJoint = m_pWorld->m_pDynamicsFactory->createHingeJoint (m_pBody1->m_pBody, m_pBody2->m_pBody);
	m_pJoint = &pJoint->getJoint ();
	m_pJoint->setUserData (this);

	m_pWorld->vAddJoint (this);
}

/****************************************************************************/
/** This is called by the joint motorization code, to figure out how far the
 ** joint is from its desired angle.
*****************************************************************************/
real Novodex::RevoluteJoint::rGetAngle ()
{
	// TODO: implement this
	return 0;
}

/****************************************************************************/
/**
*****************************************************************************/
void Novodex::RevoluteJoint::vGetPosition (VectorXYZ &vecPosition)
{
	NxHingeJoint *pJoint = pGetJoint ();

	if (pJoint && fInSimulation ())
	{
		NxrVec3 nx;
		pJoint->getAnchor (nx);

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
void Novodex::RevoluteJoint::vGetMatrix (Matrix4x4 &mx)
{
	NxHingeJoint *pJoint = pGetJoint ();
	if (!pJoint || !fInSimulation ())
	{
		mx.vSetIdentity ();
		return;
	}

	NxrVec3 nxPosition;
	NxrVec3 nxAxis;

	pJoint->getAnchor (nxPosition);
	pJoint->getAxis (nxAxis);

	// get vector orthogonal to default joint axis (0,0,1) and current axis
	NxrVec3 nxDefault (0, 0, 1);
	NxrVec3 nxOrtho = nxDefault.cross (nxAxis);

	// get a matrix with the rotation along that vector
	real rTheta = acos (nxDefault.dot (nxAxis));

	NxrQuat nxQuat;
	nxQuat.fromAngleAxis (rTheta * rRadiansToDegrees, nxOrtho);

	NxrMat33 nxOrientation;
	nxOrientation.fromQuat (nxQuat);

	vBuildMatrix (mx, nxPosition, nxOrientation);
}

/****************************************************************************/
/** 

This should build a single quaternion with yaw pitch and roll, but it 
doesn't come out quite right.  Maybe some axes are swapped?

	// From http://www.nearfield.com/~dan/misc/Quater/

	Quaternion qTotal2;

	qTotal2.w = cos(rYaw/2) * cos(rPitch/2) * cos(rRoll/2)
	         + sin(rYaw/2) * sin(rPitch/2) * sin(rRoll/2);

	qTotal2.x = cos(rYaw/2) * cos(rPitch/2) * sin(rRoll/2) 
	         - sin(rYaw/2) * sin(rPitch/2) * cos(rRoll/2);

	qTotal2.y = cos(rYaw/2) * sin(rPitch/2) * cos(rRoll/2) 
	         + sin(rYaw/2) * cos(rPitch/2) * sin(rRoll/2);

	qTotal2.z = sin(rYaw/2) * cos(rPitch/2) * cos(rRoll/2) 
	         - cos(rYaw/2) * sin(rPitch/2) * sin(rRoll/2);
			 

Whatever the case, it's kind of nice having the yaw/pitch/roll sequence
spelled out explicitly below, as "qTotal = qYaw * qPitch * qRoll;"

*****************************************************************************/

void Novodex::RevoluteJoint::vSetAnchorAndAxis (VectorXYZ &vecAnchor, VectorYPR &vecOrientationDegrees)
{
	NxHingeJoint *pJoint = pGetJoint ();

	// set anchor
	pJoint->setAnchor (NxrVec3 (vecAnchor.x, vecAnchor.y, vecAnchor.z));

	VectorXYZ vecOrientation (0, 0, 1);
	vecOrientation.vRotateByDegrees (vecOrientationDegrees);

	// set the axis
	pJoint->setAxis (NxrVec3 (vecOrientation.x, vecOrientation.y, vecOrientation.z));
}

/****************************************************************************/
/** TODO: implement this
*****************************************************************************/
void Novodex::RevoluteJoint::vSetLimits (bool fLo, real rLo, bool fHi, real rHi)
{
/*	if (!fInSimulation ())
		return;

	if (fLo)
	{
		dJointSetHingeParam (m_JointID, dParamLoStop, rLo * rDegreesToRadians);
	}
	else
	{
		dJointSetHingeParam (m_JointID, dParamLoStop, -dInfinity);
	}

	if (fHi)
	{
		dJointSetHingeParam (m_JointID, dParamHiStop, rHi * rDegreesToRadians);
	}
	else
	{
		dJointSetHingeParam (m_JointID, dParamHiStop, dInfinity);
	}
*/
}

/****************************************************************************/
/** TODO: implement this
*****************************************************************************/
void Novodex::RevoluteJoint::vAddForce (real rVel, real rForce)
{
/*	if (!fInSimulation ())
		return;

	if (!_isnan (rVel) && !_isnan (rForce))
	{
		dJointSetHingeParam (m_JointID, dParamVel, rVel);
		dJointSetHingeParam (m_JointID, dParamFMax, rForce);
	}
	else
	{
		dJointSetHingeParam (m_JointID, dParamVel, 0);
		dJointSetHingeParam (m_JointID, dParamFMax, 0);
	}
*/
}

// RevoluteJoint.cpp ends here ->