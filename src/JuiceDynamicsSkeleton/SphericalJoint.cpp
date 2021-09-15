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
Dynamics::ODE::SphericalJoint::SphericalJoint ()
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

void Dynamics::ODE::SphericalJoint::vConnectBodies (Dynamics::Body *pBody1, Dynamics::Body *pBody2)
{
	return vJointConnectBodies (dynamic_cast <ODE::Body*> (pBody1), dynamic_cast <ODE::Body*> (pBody2));
}

void Dynamics::ODE::SphericalJoint::vCreate (Dynamics::World *pWorld, Juice::SphericalJoint *pOwner)
{
	Dynamics::ODE::Joint::vCreate (dynamic_cast <ODE::World*> (pWorld));

	if (m_pWorld && !m_JointID)
	{
		m_JointID = dJointCreateBall (m_pWorld->m_WorldID, 0);
		dJointSetData (m_JointID, pOwner);
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

/*****************************************************************************
<FOOTER>
*****************************************************************************/
