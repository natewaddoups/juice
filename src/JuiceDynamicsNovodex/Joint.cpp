/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsNovodex.dll
Filename: Joint.cpp

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
Dynamics::Novodex::Joint::Joint (Novodex::World *pWorld) : 
	m_pWorld (pWorld),
	m_pBody1 (null), 
	m_pBody2 (null),
	m_pJoint (null)
{
}

/****************************************************************************/
/**
*****************************************************************************/
Dynamics::Novodex::Joint::~Joint ()
{
	if (m_pJoint)
	{
		m_pWorld->vRemoveJoint (this);
		m_pJoint->release ();
		m_pJoint = null;
	}
}

/****************************************************************************/
/**
*****************************************************************************/
bool Dynamics::Novodex::Joint::fJointCreate (Juice::Joint *pOwner,
											 Dynamics::Body *pAbstractBody1, Dynamics::Body *pAbstractBody2)
{
	if (m_pWorld && m_pJoint)
	{
		m_pWorld->vRemoveJoint (this);
	}

	// Store body pointers
	m_pBody1 = dynamic_cast <Novodex::Body*> (pAbstractBody1);
	m_pBody2 = dynamic_cast <Novodex::Body*> (pAbstractBody2);

	if (!pOwner || !m_pWorld || !m_pBody1 || !m_pBody2)
		return false;

	if (!m_pBody1->m_pBody || !m_pBody2->m_pBody)
		return false;

	// destroy the current joint
	if (m_pJoint)
	{
		m_pJoint->release ();
		m_pJoint = 0;
	}

	return true;
}

/****************************************************************************/
/**
*****************************************************************************/
bool Dynamics::Novodex::Joint::fJointInSimulation ()
{
	if (m_pJoint && m_pBody1 && m_pBody2)
		return true;

	return false;
}

// Joint.cpp ends here ->