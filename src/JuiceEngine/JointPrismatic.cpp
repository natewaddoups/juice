/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: JointPrismatic.cpp

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
PrismaticJoint::PrismaticJoint (Model *pModel) : 
	Joint (pModel, L"PrismaticJoint"),
	m_pDynamicsObject (null)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
PrismaticJoint::~PrismaticJoint ()
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vDelete ();
}

PrismaticJoint& PrismaticJoint::operator= (const PrismaticJoint& CopyFrom)
{
	Joint::operator= (CopyFrom);

	m_rLength = CopyFrom.m_rLength;
	m_rDiameter = CopyFrom.m_rDiameter;

	return *this;
}

Selectable* PrismaticJoint::pCreateClipboardCopy () const
{
	PrismaticJoint *pNew = new PrismaticJoint (m_pModel);
	*pNew = *this;
	return (Selectable*) pNew;
}

bool PrismaticJoint::fPaste (Selectable *pSelectable)
{
	PrismaticJoint *pSource = dynamic_cast <PrismaticJoint*> (pSelectable);

	if (pSource)
	{
		VectorXYZ vecPosition = m_vecPosition;
		*this = *pSource;
		m_vecPosition = vecPosition;
		return true;
	}

	return false;
}

real PrismaticJoint::rGetExtension ()
{
	if (m_pModel->pGetApplication()->eGetAppMode () == amDesign)
		return 0;

	if (!m_pDynamicsObject)
		return 0;

    return m_pDynamicsObject->rGetExtension ();
}

void PrismaticJoint::vApplyForce (real rDesiredPosition)
{
	real rPresentPosition = rGetExtension ();
	real rError = rDesiredPosition - rPresentPosition;
	real rVel = (m_rGain / 10) * rError; // m_rMotionAccumulator;
	vApplyForce (rVel, m_rMaxForce);
}

void PrismaticJoint::vApplyForce (real rVel, real rForce)
{
	if (m_pModel->pGetApplication()->eGetAppMode () == amDesign)
		return;

	if (m_pDynamicsObject)
		m_pDynamicsObject->vAddForce (rVel, rForce);
}

void PrismaticJoint::vUpdateDynamicsObject ()
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vSetLimits (m_fLoStop, m_rLoStop, m_fHiStop, m_rHiStop);
}

void PrismaticJoint::vSetLength (real rLength)
{
	unless (rLength > 0)
		rLength = 0.01;

	m_rLength = rLength;
	Generic::Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}

real PrismaticJoint::rGetLength ()
{
	return m_rLength;
}

void PrismaticJoint::vSetDiameter (real rDiameter)
{
	unless (rDiameter > 0)
		rDiameter = 0.01;

	m_rDiameter = rDiameter;
	Generic::Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}

real PrismaticJoint::rGetDiameter ()
{
	return m_rDiameter;
}

PropertyTreePtr PrismaticJoint::ms_pPropertyTree;

Generic::PropertyTree* PrismaticJoint::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Joint::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		PrismaticJointProperty *pTwin = new PrismaticJointProperty("Twin", (fnGetComponent)&Juice::Component::pGetTwin, (fnSetComponent)&Juice::Component::vSetTwin);
		ms_pPropertyTree->vAddNode (pTwin);

		std::list<Property*> Dimension;	

		Dimension.push_back(new RealProperty("Length", (Generic::fnGetReal) &Juice::PrismaticJoint::rGetLength, (Generic::fnSetReal) &Juice::PrismaticJoint::vSetLength));
		Dimension.push_back(new RealProperty("Diameter", (Generic::fnGetReal) &Juice::PrismaticJoint::rGetDiameter, (Generic::fnSetReal) &Juice::PrismaticJoint::vSetDiameter));

		ms_pPropertyTree->vAddNode ("Dimension", Dimension);
	}

	return ms_pPropertyTree;
}

void PrismaticJoint::vGetPosition (VectorXYZ &vecPosition)
{
	vecPosition.x = vecPosition.y = vecPosition.z = 0;

	if (m_pModel && m_pModel->pGetApplication ()->eGetAppMode () == Juice::amDesign)
	{
		vecPosition = m_vecPosition;
	}
	else
	{
		if (m_pDynamicsObject)
			m_pDynamicsObject->vGetPosition (vecPosition);
	}
}

// JointPrismatic.cpp ends here ->