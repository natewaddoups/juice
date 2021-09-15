/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsODE.dll
Filename: Joint.cpp

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
Dynamics::ODE::Joint::Joint (ODE::World *pWorld) : 
	m_JointID (null),
	m_pWorld (pWorld),
	m_pBody1 (null), 
	m_pBody2 (null)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::ODE::Joint::~Joint ()
{
	if (m_JointID)
		dJointDestroy (m_JointID);
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Dynamics::ODE::Joint::fJointCreate ()
{
	// destroy the current joint
	if (m_JointID)
		dJointDestroy (m_JointID);

	// The ID will be initialized in the derived class vCreate
	m_JointID = 0;

	return true;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::ODE::Joint::vJointConnectBodies (Dynamics::Body *pAbstractBody1, Dynamics::Body *pAbstractBody2)
{
	// Store body pointers
	m_pBody1 = dynamic_cast <ODE::Body*> (pAbstractBody1);
	m_pBody2 = dynamic_cast <ODE::Body*> (pAbstractBody2);

	// only do this if both bodies are valid 
	if ((m_pBody1 && m_pBody2) && (m_pBody1->m_BodyID && m_pBody2->m_BodyID) && (m_pBody1->m_BodyID != m_pBody2->m_BodyID))
	{
		// make the association in the physics library
		dJointAttach (m_JointID, m_pBody1->m_BodyID, m_pBody2->m_BodyID);
	}
	else
	{
		// if either is invalid, set both pointers to null
		m_pBody1 = m_pBody2 = null;
		dJointAttach (m_JointID, null, null);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Dynamics::ODE::Joint::fJointInSimulation ()
{
	if (m_JointID && m_pBody1 && m_pBody2)
		return true;

	return false;
}

// Joint.cpp ends here ->