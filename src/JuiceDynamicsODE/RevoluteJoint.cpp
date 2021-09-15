/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsODE.dll
Filename: RevoluteJoint.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceDynamicsODE.dll and its source code are distributed under the terms of 
the Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>
#include <DynamicsAPI.h>

#include <ode/ode.h>
#include "DynamicsODE.h"

#include <float.h>
#include <Math.h>

using namespace Juice;
using namespace Juice::Dynamics::ODE;

/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::ODE::RevoluteJoint::RevoluteJoint (ODE::World *pWorld) : Joint (pWorld)
{
}

Dynamics::ODE::RevoluteJoint::~RevoluteJoint ()
{
}

void Dynamics::ODE::RevoluteJoint::vDelete ()
{
	delete this;
}

bool Dynamics::ODE::RevoluteJoint::fInSimulation ()
{
	return fJointInSimulation ();
}

void Dynamics::ODE::RevoluteJoint::vCreate (Juice::RevoluteJoint *pOwner,
											Dynamics::Body *pBody1, Dynamics::Body *pBody2)
{
	if (!fJointCreate ())
		return;	

	if (m_pWorld && !m_JointID)
	{
		m_JointID = dJointCreateHinge (m_pWorld->m_WorldID, 0);
		dJointSetData (m_JointID, pOwner);
		vJointConnectBodies (pBody1, pBody2);
	}
}

real Dynamics::ODE::RevoluteJoint::rGetAngle ()
{
	if (!fInSimulation ())
		return 0;

	return dJointGetHingeAngle (m_JointID);
}

void Dynamics::ODE::RevoluteJoint::vGetPosition (VectorXYZ &vecPosition)
{
	dReal pPosition[3]; 

	if (fInSimulation ())
	{
		dJointGetHingeAnchor (m_JointID, pPosition);
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


/* This is no longer used, and never worked quite right anyhow
void Dynamics::ODE::RevoluteJoint::vGetRotation (VectorYPR &vecRotation)
{
	dReal Axis[3]; 

	if (fInSimulation ())
	{
		// get the axis vector
		dJointGetHingeAxis (m_JointID, Axis);

		// convert to rotations - note that yaw is ignored
		vecRotation.yaw = 0;

		VectorXYZ vecStart (0, 0, 1), vecTarget (Axis[0], Axis[1], Axis[2]);
		real rTheta = acos (DotProduct (vecStart, vecTarget));
		vecRotation.pitch = rTheta;
		vecRotation.roll = 0;

		vecRotation.vToDegrees ();
		vecStart.vRotateByDegrees (vecRotation);
		vecRotation.vToRadians ();

		vecRotation.roll = acos (DotProduct (vecStart, vecTarget));

		vecRotation.vToDegrees ();
	}
	else
	{
		vecRotation.yaw = 0;
		vecRotation.pitch = 0;
		vecRotation.roll = 0;
	}
}
*/

void Dynamics::ODE::RevoluteJoint::vGetMatrix (Matrix4x4 &mx)
{
	dReal Position[3]; 
	dReal Rotation[12];

	dReal Axis[3];
	dJointGetHingeAxis (m_JointID, Axis);

	// get vector orthogonal to default joint axis (0,0,1) and current axis
	VectorXYZ vecDefault (0, 0, 1);
	VectorXYZ vecTarget (Axis[0], Axis[1], Axis[2]);
	VectorXYZ vecAxis = CrossProduct (vecDefault, vecTarget);

	// get rotation along that vector
	real rTheta = acos (DotProduct (vecDefault, vecTarget));

	dRFromAxisAndAngle (Rotation, vecAxis.x, vecAxis.y, vecAxis.z, rTheta);

	dJointGetHingeAnchor (m_JointID, Position);

	vBuildMatrix (mx, Position, Rotation);

	//pScene->vPushMatrix ((const real*) Position, (const real*) Rotation);
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

void Dynamics::ODE::RevoluteJoint::vSetAnchorAndAxis (VectorXYZ &vecAnchor, VectorYPR &vecOrientationDegrees)
{
	if (!m_JointID)
		return;

	// set anchor
	dJointSetHingeAnchor (m_JointID, vecAnchor.x, vecAnchor.y, vecAnchor.z);

	VectorXYZ vecOrientation (0, 0, 1);
	vecOrientation.vRotateByDegrees (vecOrientationDegrees);

	// set the axis
	dJointSetHingeAxis (m_JointID, vecOrientation.x, vecOrientation.y, vecOrientation.z);
}

void Dynamics::ODE::RevoluteJoint::vSetLimits (bool fLo, real rLo, bool fHi, real rHi)
{
	if (!fInSimulation ())
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
}

void Dynamics::ODE::RevoluteJoint::vAddForce (real rVel, real rForce)
{
	if (!fInSimulation ())
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
}

// RevoluteJoint.cpp ends here ->