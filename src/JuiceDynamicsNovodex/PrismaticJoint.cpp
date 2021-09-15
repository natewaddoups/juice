/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsNovodex.dll
Filename: PrismaticJoint.cpp

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
Dynamics::Novodex::PrismaticJoint::PrismaticJoint (Novodex::World *pWorld) : Joint (pWorld)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::Novodex::PrismaticJoint::~PrismaticJoint ()
{
}

/****************************************************************************/
/** 
*****************************************************************************/
NxPrismaticJoint* Dynamics::Novodex::PrismaticJoint::pGetJoint ()
{
	NxPrismaticJoint *pJoint = null;

	if (m_pJoint)
		return m_pJoint->isPrismaticJoint ();

	return 0;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::Novodex::PrismaticJoint::vDelete ()
{
	delete this;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Dynamics::Novodex::PrismaticJoint::fInSimulation ()
{
	return fJointInSimulation ();
}

void Dynamics::Novodex::PrismaticJoint::vCreate (Juice::PrismaticJoint *pOwner,
												 Dynamics::Body *pBody1, Dynamics::Body *pBody2)
{
	if (!Joint::fJointCreate (pOwner, pBody1, pBody2))
		return;

	NxPrismaticJoint *pJoint = m_pWorld->m_pDynamicsFactory->createPrismaticJoint (m_pBody1->m_pBody, m_pBody2->m_pBody);
	m_pJoint = &pJoint->getJoint ();
	m_pJoint->setUserData (this);
	m_pWorld->vAddJoint (this);
}

/****************************************************************************/
/** TODO: implement this
*****************************************************************************/
void Dynamics::Novodex::PrismaticJoint::vSetLimits (bool fLo, real rLo, bool fHi, real rHi)
{
/*	if (!fInSimulation ())
		return;

	if (fLo)
	{
		dJointSetSliderParam (m_JointID, dParamLoStop, rLo);
	}
	else
	{
		dJointSetSliderParam (m_JointID, dParamLoStop, -dInfinity);
	}

	if (fHi)
	{
		dJointSetSliderParam (m_JointID, dParamHiStop, rHi);
	}
	else
	{
		dJointSetSliderParam (m_JointID, dParamHiStop, dInfinity);
	}
*/
}


/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::Novodex::PrismaticJoint::vSetAnchorAndAxis (VectorXYZ &vecAnchor, VectorYPR &vecRotation)
{
	NxPrismaticJoint *pJoint = pGetJoint ();
	if (!pJoint)
		return;

	VectorXYZ vecOrientation (0, 0, -1);
	vecOrientation.vRotateByDegrees (-vecRotation);

	// set the axis
	pJoint->setAnchor (NxrVec3 (vecAnchor.x, vecAnchor.y, vecAnchor.z));
	pJoint->setAxis (NxrVec3 (vecOrientation.x, vecOrientation.y, vecOrientation.z));
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::Novodex::PrismaticJoint::vGetAxis (VectorXYZ &vecAxis)
{
	NxPrismaticJoint *pJoint = pGetJoint ();
	if (!pJoint || !fInSimulation ())
	{
		vecAxis.x = vecAxis.y = vecAxis.z = 0;
		return;
	}

	NxrVec3 nxAxis;
	pJoint->getAxis (nxAxis);

	vecAxis.x = nxAxis.x();
	vecAxis.y = nxAxis.y();
	vecAxis.z = nxAxis.z();
}


/****************************************************************************/
/** This is called by the motorization code when comparing the present 
 ** extension with the desired extension.
 **
 ** TODO: implement this
*****************************************************************************/
real Dynamics::Novodex::PrismaticJoint::rGetExtension ()
{
	NxPrismaticJoint *pJoint = pGetJoint ();
	if (!pJoint || !fInSimulation ())
		return 0;

	return 0;
}

/****************************************************************************/
/** TODO: implement this
*****************************************************************************/
void Dynamics::Novodex::PrismaticJoint::vAddForce (real rVel, real rForce)
{
/*	if (!fInSimulation ())
		return;

	if (!_isnan (rVel) && !_isnan (rForce))
	{
		dJointSetSliderParam (m_JointID, dParamVel, rVel);
		dJointSetSliderParam (m_JointID, dParamFMax, rForce);
	}
	else
	{
		dJointSetSliderParam (m_JointID, dParamVel, 0);
		dJointSetSliderParam (m_JointID, dParamFMax, 0);
	}
*/
}

/****************************************************************************/
/**  Note that this is a rather naive implementation.  The right way to do it 
 **  would be to store the original endpoints of the slider when simulation 
 **  begins - see the vGetMatrix method for example.
*****************************************************************************/
void Dynamics::Novodex::PrismaticJoint::vGetPosition (VectorXYZ &vecPosition)
{
	/*
	if (fInSimulation ())
	{
		const dReal *pPosition1 = null; 
		const dReal *pPosition2 = null; 

		dBodyID idBody1 = dJointGetBody (m_JointID, 0);
		dBodyID idBody2 = dJointGetBody (m_JointID, 1);

		if (idBody1)
			pPosition1 = dBodyGetPosition (idBody1);

		if (idBody2)
			pPosition2 = dBodyGetPosition (idBody2);

		if (pPosition1 && pPosition2)
		{
			vecPosition.x = (pPosition1[0] + pPosition2[0]) / 2;
			vecPosition.y = (pPosition1[1] + pPosition2[1]) / 2;
			vecPosition.z = (pPosition1[2] + pPosition2[2]) / 2;
		}
		else
		{
			vecPosition.x = 0;
			vecPosition.y = 0;
			vecPosition.z = 0;
		}
	}
	else
	{
		vecPosition.x = 0;
		vecPosition.y = 0;
		vecPosition.z = 0;
	}
	*/
}

/****************************************************************************/
/** 
*****************************************************************************/
void Dynamics::Novodex::PrismaticJoint::vGetMatrix (Matrix4x4 &mx, const VectorXYZ &vecBody1OriginalEndPoint, const VectorXYZ &vecBody2OriginalEndPoint)
{
	VectorXYZ vecPosition;
	Quaternion qRotation;
	Matrix4x4 mxConversion, mxTransposed;

	VectorXYZ vecBody1EndPoint;
	VectorXYZ vecBody2EndPoint;

	// convert body 1 endpoint from local to world coordinates
	m_pBody1->vGetQuaternion (qRotation);
	m_pBody1->vGetPosition (vecPosition);

	mxConversion.vSetIdentity ();
	mxConversion.vSetRotation (qRotation);
	mxConversion.vTranspose (mxTransposed);

	vecBody1EndPoint = vecBody1OriginalEndPoint * mxTransposed;
	vecBody1EndPoint = vecBody1EndPoint + vecPosition;
	
	// convert body 2 endpoint from local to world coordinates
	m_pBody2->vGetQuaternion (qRotation);
	m_pBody2->vGetPosition (vecPosition);

	mxConversion.vSetIdentity ();
	mxConversion.vSetRotation (qRotation);
	mxConversion.vTranspose (mxTransposed);

	vecBody2EndPoint = vecBody2OriginalEndPoint * mxTransposed;
	vecBody2EndPoint = vecBody2EndPoint + vecPosition;

	// find length of slider 
	VectorXYZ vecSlider = vecBody1EndPoint - vecBody2EndPoint;
	real rLength = vecSlider.rMagnitude ();

	// find position of slider
	//VectorXYZ vecHalfSlider = vecSlider * 0.5;
	//VectorXYZ vecMidpoint = vecBody2EndPoint + vecHalfSlider;
	VectorXYZ vecMidpoint = vecBody1EndPoint + vecBody2EndPoint;
	vecMidpoint /= 2;

	NxrVec3 nxPosition; 
	nxPosition.set (vecMidpoint.x, vecMidpoint.y, vecMidpoint.z);

	VectorXYZ vecAxis;
	vGetAxis (vecAxis);
	VectorXYZ vecCross = CrossProduct (vecAxis, VectorXYZ (0, 0, 1));
	real rTheta = -rRadiansBetween (vecAxis, VectorXYZ (0, 0, 1));

	NxrQuat nxQuat;
	nxQuat.fromAngleAxis (rTheta * rRadiansToDegrees, NxrVec3 (vecCross.x, vecCross.y, vecCross.z));

	NxrMat33 nxOrientation;
	nxOrientation.fromQuat (nxQuat);

	vBuildMatrix (mx, nxPosition, nxOrientation);
}

// PrismaticJoint.cpp ends here ->