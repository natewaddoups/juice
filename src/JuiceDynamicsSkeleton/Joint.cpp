/*****************************************************************************
<HEADER>
*****************************************************************************/
#include <JuiceEngine.h>
#include <DynamicsAPI.h>

#include <ode/ode.h>
#include <DynamicsODE.h>

#include <float.h>
#include <Math.h>

using namespace Juice;
using namespace Juice::Dynamics::ODE;

/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::ODE::Joint::Joint () : 
	m_pWorld (null),
	m_pBody1 (null), 
	m_pBody2 (null)
{
}

Dynamics::ODE::Joint::~Joint ()
{
}

void Dynamics::ODE::Joint::vCreate (ODE::World *pWorld)
{
	// store the pointer to the current world
	m_pWorld = pWorld;

	// destroy the current joint

	// The ID will be initialized in the derived class vCreate
	m_JointID = 0;
}

void Dynamics::ODE::Joint::vJointConnectBodies (ODE::Body *pBody1, ODE::Body *pBody2)
{
	// only do this if both bodies are valid 
	if ((pBody1 && pBody2) && (pBody1->m_BodyID && pBody2->m_BodyID) && (pBody1->m_BodyID != pBody2->m_BodyID))
	{
		// store pointers to the bodies
		m_pBody1 = pBody1;
		m_pBody2 = pBody2;

		// make the association in the physics library
		dJointAttach (m_JointID, pBody1->m_BodyID, pBody2->m_BodyID);
	}
	else
	{
		// if either is invalid, set both pointers to null
		m_pBody1 = m_pBody2 = null;
		dJointAttach (m_JointID, null, null);
	}
}

bool Dynamics::ODE::Joint::fJointInSimulation ()
{
	if (m_JointID && m_pBody1 && m_pBody2)
		return true;

	return false;
}

/*****************************************************************************
<FOOTER>
*****************************************************************************/
