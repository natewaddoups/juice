/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsODE.dll
Filename: PrismaticJoint.cpp

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
Dynamics::ODE::PrismaticJoint::PrismaticJoint (ODE::World *pWorld) : Joint (pWorld)
{
}

Dynamics::ODE::PrismaticJoint::~PrismaticJoint ()
{
}

void Dynamics::ODE::PrismaticJoint::vDelete ()
{
	delete this;
}

bool Dynamics::ODE::PrismaticJoint::fInSimulation ()
{
	return fJointInSimulation ();
}

void Dynamics::ODE::PrismaticJoint::vCreate (Juice::PrismaticJoint *pOwner,
											 Dynamics::Body *pBody1, Dynamics::Body *pBody2)
{
	if (!fJointCreate ())
		return;	

	if (m_pWorld && !m_JointID)
	{
		m_JointID = dJointCreateSlider (m_pWorld->m_WorldID, 0);
		dJointSetData (m_JointID, pOwner);
		vJointConnectBodies (pBody1, pBody2);
	}
}

void Dynamics::ODE::PrismaticJoint::vSetLimits (bool fLo, real rLo, bool fHi, real rHi)
{
	if (!fInSimulation ())
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
}


void Dynamics::ODE::PrismaticJoint::vSetAnchorAndAxis (VectorXYZ &vecAnchor, VectorYPR &vecRotation)
{
	if (!m_JointID)
		return;

	VectorXYZ vecOrientation (0, 0, -1);
	vecOrientation.vRotateByDegrees (-vecRotation);

	// set the axis
	dJointSetSliderAxis (m_JointID, vecOrientation.x, vecOrientation.y, vecOrientation.z);
}

void Dynamics::ODE::PrismaticJoint::vGetAxis (VectorXYZ &vecAxis)
{
	if (!fInSimulation ())
		return;

	dVector3 dvecAxis;
	dJointGetSliderAxis (m_JointID, dvecAxis);

	vecAxis.x = dvecAxis[0];
	vecAxis.y = dvecAxis[1];
	vecAxis.z = dvecAxis[2];
}


real Dynamics::ODE::PrismaticJoint::rGetExtension ()
{
	if (!fInSimulation ())
		return 0;

	return dJointGetSliderPosition (m_JointID);
}

void Dynamics::ODE::PrismaticJoint::vAddForce (real rVel, real rForce)
{
	if (!fInSimulation ())
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

}

/// Note that this is a rather naive implementation.  The right way to do it 
/// would be to store the original endpoints of the slider when simulation 
/// begins - see the vGetMatrix method for example.
void Dynamics::ODE::PrismaticJoint::vGetPosition (VectorXYZ &vecPosition)
{
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
}

void Dynamics::ODE::PrismaticJoint::vGetMatrix (Matrix4x4 &mx, const VectorXYZ &vecBody1OriginalEndPoint, const VectorXYZ &vecBody2OriginalEndPoint)
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

	dReal Position[3]; 
	Position[0] = vecMidpoint.x;
	Position[1] = vecMidpoint.y;
	Position[2] = vecMidpoint.z;

	VectorXYZ vecAxis;
	vGetAxis (vecAxis);
	VectorXYZ vecCross = CrossProduct (vecAxis, VectorXYZ (0, 0, 1));
	real rTheta = -rRadiansBetween (vecAxis, VectorXYZ (0, 0, 1));

	dReal Rotation[12];
	dRFromAxisAndAngle (Rotation, vecCross.x, vecCross.y, vecCross.z, rTheta);

	vBuildMatrix (mx, Position, Rotation);

	return;
/*
	// then get rotation to the slider's axis
	dMatrix3 mxRotation;
	VectorXYZ vecAxis (0, 0, 1);

	vGetAxis (vecAxis);

	dRFrom2Axes (mxRotation, vecAxis.x, vecAxis.y, vecAxis.z, 0, 0, 1);
	dQuaternion q;
	dQFromAxisAndAngle (q, 0, 1, 0, rPi / 2);
	dMatrix3 mxTmp;
	dQtoR (q, mxTmp);
	dMatrix3 mxFinalRotation;
	dMultiply0 (mxFinalRotation, mxRotation, mx, 3, 3, 3);
	
	dReal Position[3]; 
	Position[0] = vecMidpoint.x;
	Position[1] = vecMidpoint.y;
	Position[2] = vecMidpoint.z;

	vBuildMatrix (mx, Position, mxFinalRotation);
*/
}

/*
	//dMatrix3 mxRotation;
	//dRFrom2Axes (mxRotation, vecAxis.x, vecAxis.y, vecAxis.z, 0, 0, 1);
	Matrix4x4 mxRotation;
	mxRotation.vFromTwoAxes (vecAxis, VectorXYZ (0, 0, 1));

	//dQuaternion q;
	//dQFromAxisAndAngle (q, 0, 1, 0, rPi / 2);
	Quaternion q (VectorXYZ (0, 1, 0), rPi / 2);

	//dMatrix3 mx;
	//dQtoR (q, mx);
	Matrix4x4 mxTmp (q);

	//dMatrix3 mxFinalRotation;
	//dMultiply0 (mxFinalRotation, mxRotation, mx, 3, 3, 3);
	Matrix4x4 mxFinalRotation;
	mxFinalRotation = mxRotation * mxTmp;
	
	//dReal Position[3]; 
	//Position[0] = vecMidpoint.x;
	//Position[1] = vecMidpoint.y;
	//Position[2] = vecMidpoint.z;

	// Set transform
	mxRotation.vSetTranslation (vecMidpoint);

	mx = mxRotation;
*/

// PrismaticJoint.cpp ends here ->