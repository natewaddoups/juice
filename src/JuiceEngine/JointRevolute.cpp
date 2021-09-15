/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: JointRevolute.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>
#include <Model.h>

using namespace Juice;


/****************************************************************************/
/** 
*****************************************************************************/
RevoluteJoint::RevoluteJoint (Model *pModel) : 
	Joint (pModel, L"RevoluteJoint"),
	m_rLength (1),
	m_rDiameter (0.25),
	m_pDynamicsObject (null)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
RevoluteJoint::~RevoluteJoint ()
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vDelete ();
}

RevoluteJoint& RevoluteJoint::operator= (const RevoluteJoint& CopyFrom)
{
	Joint::operator= (CopyFrom);

	m_rLength = CopyFrom.m_rLength;
	m_rDiameter = CopyFrom.m_rDiameter;

	return *this;
}

Selectable* RevoluteJoint::pCreateClipboardCopy () const
{
	RevoluteJoint *pNew = new RevoluteJoint (m_pModel);
	*pNew = *this;
	return (Selectable*) pNew;
}

bool RevoluteJoint::fPaste (Selectable *pSelectable)
{
	RevoluteJoint *pSource = dynamic_cast <RevoluteJoint*> (pSelectable);

	if (pSource)
	{
		VectorXYZ vecPosition = m_vecPosition;
		*this = *pSource;
		m_vecPosition = vecPosition;
		return true;
	}


	return false;
}


/****************************************************************************/
/** 
*****************************************************************************/
void RevoluteJoint::vSetLength (real rLength)
{
	unless (rLength > 0)
		rLength = 0.01;

	m_rLength = rLength;
	Generic::Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}

real RevoluteJoint::rGetLength ()
{
	return m_rLength;
}

void RevoluteJoint::vSetDiameter (real rDiameter)
{
	unless (rDiameter > 0)
		rDiameter = 0.01;

	m_rDiameter = rDiameter;
	Generic::Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}

real RevoluteJoint::rGetDiameter ()
{
	return m_rDiameter;
}

/****************************************************************************/
/** 
*****************************************************************************/
PropertyTreePtr RevoluteJoint::ms_pPropertyTree;

Generic::PropertyTree* RevoluteJoint::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Joint::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		RevoluteJointProperty *pTwin = new RevoluteJointProperty("Twin", (fnGetComponent)&Juice::Component::pGetTwin, (fnSetComponent)&Juice::Component::vSetTwin);
		ms_pPropertyTree->vAddNode (pTwin);

		std::list<Property*> Dimension;	

		Dimension.push_back(new RealProperty("Length", (Generic::fnGetReal) &Juice::RevoluteJoint::rGetLength, (Generic::fnSetReal) &Juice::RevoluteJoint::vSetLength));
		Dimension.push_back(new RealProperty("Diameter", (Generic::fnGetReal) &Juice::RevoluteJoint::rGetDiameter, (Generic::fnSetReal) &Juice::RevoluteJoint::vSetDiameter));

		ms_pPropertyTree->vAddNode ("Dimension", Dimension);

		//ms_pPropertyTree->vAddNode (new RealProperty ("Present Angle", (Generic::fnGetReal) rGetPresentAngle, (Generic::fnSetReal) vSetPresentAngle));
	}

	return ms_pPropertyTree;
}

real RevoluteJoint::rGetAngle ()
{
	real rResult = 0;

	if (m_pModel->pGetApplication ()->eGetAppMode () == Juice::amDesign)
	{
		return 0;
	}
	else
	{
		rResult = m_pDynamicsObject->rGetAngle ();
		rResult *= Generic::rRadiansToDegrees;
	}	

	return rResult;
}

void RevoluteJoint::vGetPosition (VectorXYZ &vecPosition)
{
	vecPosition.x = vecPosition.y = vecPosition.z = 0;

	if (m_pModel->pGetApplication ()->eGetAppMode () == Juice::amDesign)
	{
		vecPosition = m_vecPosition;
	}
	else
	{
		if (m_pDynamicsObject)
			m_pDynamicsObject->vGetPosition (vecPosition);
	}
}

void RevoluteJoint::vGetRotation (VectorYPR &vecRotation)
{
	vecRotation.yaw = vecRotation.pitch = vecRotation.roll = 0;

	if (m_pModel->pGetApplication ()->eGetAppMode () == Juice::amDesign)
	{
		vecRotation = m_vecRotation;
	}
	else
	{
		// TODO: just cut this out if it's not being used
		GenericTrace ("I thought this code was dead?" << std::endl);
//		m_pDynamicsObject->vGetRotation (vecRotation);
	}
}

void RevoluteJoint::vApplyForce (real rDesiredPosition)
{
	real rPresentPosition = rGetAngle ();
	real rError = rDesiredPosition - rPresentPosition;

	// if present or actual position change sign (wrap around) at different
	// times, the error result gets way out of hand...
	while (rError > 180) rError -= 360.0;
	while (rError < -180) rError += 360.0;

#ifdef _DEBUG_NoLongerNecessary
	char szDebug[100];
	sprintf (szDebug, "Actual: %4d  Desired: %4d  Error: %4d\n", (int) rPresentPosition, (int) rDesiredPosition, (int) rError);
	OutputDebugString (szDebug);
#endif
	real rVel = (m_rGain / 10) * rError; //m_rMotionAccumulator;
	vApplyForce (rVel, m_rMaxForce);
}

void RevoluteJoint::vApplyForce (real rVel, real rForce)
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vAddForce (rVel, rForce);
}

void RevoluteJoint::vUpdateDynamicsObject ()
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vSetLimits (m_fLoStop, m_rLoStop, m_fHiStop, m_rHiStop);
}

// JointRevolute.cpp ends here ->