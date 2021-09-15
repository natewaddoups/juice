/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsODE.dll
Filename: SphericalJoint.cpp

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
Dynamics::ODE::SphericalJoint::SphericalJoint (ODE::World *pWorld) : Joint (pWorld)
{
}

Dynamics::ODE::SphericalJoint::~SphericalJoint ()
{
}

void Dynamics::ODE::SphericalJoint::vDelete ()
{
	delete this;
}

bool Dynamics::ODE::SphericalJoint::fInSimulation ()
{
	return fJointInSimulation ();
}

void Dynamics::ODE::SphericalJoint::vCreate (Juice::SphericalJoint *pOwner,
											 Dynamics::Body *pBody1, Dynamics::Body *pBody2)
{
	if (!fJointCreate ())
		return;	

	if (m_pWorld && !m_JointID)
	{
		m_JointID = dJointCreateBall (m_pWorld->m_WorldID, 0);
		dJointSetData (m_JointID, pOwner);
		vJointConnectBodies (pBody1, pBody2);
	}
}

void Dynamics::ODE::SphericalJoint::vSetAnchor (VectorXYZ &vecAnchor)
{
	if (!m_JointID)
		return;

	// set the axis
	dJointSetBallAnchor (m_JointID, vecAnchor.x, vecAnchor.y, vecAnchor.z);
}

void Dynamics::ODE::SphericalJoint::vGetAnchor (VectorXYZ &vecAnchor)
{
	if (!fInSimulation ())
		return;

	dVector3 dvecAnchor;
	dJointGetBallAnchor (m_JointID, dvecAnchor);

	vecAnchor.x = dvecAnchor[0];
	vecAnchor.y = dvecAnchor[1];
	vecAnchor.z = dvecAnchor[2];
}

void Dynamics::ODE::SphericalJoint::vSetLimits (bool fLo, real rLo, bool fHi, real rHi)
{
	// not applicable
}

// SphericalJoint.cpp ends here ->