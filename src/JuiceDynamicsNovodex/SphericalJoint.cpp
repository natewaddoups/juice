/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsNovodex.dll
Filename: SphericalJoint.cpp

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

//using namespace Juice;
using namespace Juice::Dynamics;

/****************************************************************************/
/** 
*****************************************************************************/
Novodex::SphericalJoint::SphericalJoint (Novodex::World *pWorld) : Joint (pWorld)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
Novodex::SphericalJoint::~SphericalJoint ()
{
}

/****************************************************************************/
/** 
*****************************************************************************/
NxSphericalJoint* Novodex::SphericalJoint::pGetJoint ()
{
	NxSphericalJoint *pJoint = null;

	if (m_pJoint)
		return m_pJoint->isSphericalJoint ();

	return 0;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Novodex::SphericalJoint::vDelete ()
{
	delete this;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Novodex::SphericalJoint::fInSimulation ()
{
	return fJointInSimulation ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void Novodex::SphericalJoint::vCreate (Juice::SphericalJoint *pOwner,
							  Juice::Dynamics::Body *pBody1, Juice::Dynamics::Body *pBody2)
{
	if (!fJointCreate (pOwner, pBody1, pBody2))
		return;

	NxSphericalJoint *pJoint = m_pWorld->m_pDynamicsFactory->createSphericalJoint (m_pBody1->m_pBody, m_pBody2->m_pBody);
	m_pJoint = &pJoint->getJoint ();
	m_pJoint->setUserData (this);
	m_pWorld->vAddJoint (this);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Novodex::SphericalJoint::vSetAnchor (VectorXYZ &vecAnchor)
{
	NxSphericalJoint *pJoint = pGetJoint ();

	if (!pJoint || !fInSimulation ())
		return;

	pJoint->setAnchor (NxrVec3 (vecAnchor.x, vecAnchor.y, vecAnchor.z));	
}

/****************************************************************************/
/** 
*****************************************************************************/
void Novodex::SphericalJoint::vGetAnchor (VectorXYZ &vecAnchor)
{
	NxSphericalJoint *pJoint = pGetJoint ();

	if (pJoint && fInSimulation ())
	{
		NxrVec3 nx;
		pJoint->getAnchor (nx);

		vecAnchor.x = nx.x ();
		vecAnchor.y = nx.y ();
		vecAnchor.z = nx.z ();
	}
	else
	{
		vecAnchor.x = 0;
		vecAnchor.y = 0;
		vecAnchor.z = 0;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void Novodex::SphericalJoint::vSetLimits (bool fLo, real rLo, bool fHi, real rHi)
{
	// not applicable
}

// SphericalJoint.cpp ends here ->