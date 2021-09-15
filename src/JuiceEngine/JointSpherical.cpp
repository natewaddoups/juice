/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: JointSpherical.cpp

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
SphericalJoint::SphericalJoint (Model *pModel) : 
	Joint (pModel, L"SphericalJoint"),
	m_rDiameter (1),
	m_pDynamicsObject (null)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
SphericalJoint::~SphericalJoint ()
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vDelete ();
}

SphericalJoint& SphericalJoint::operator= (const SphericalJoint& CopyFrom)
{
	Joint::operator= (CopyFrom);

	m_rDiameter = CopyFrom.m_rDiameter;

	return *this;
}

Selectable* SphericalJoint::pCreateClipboardCopy () const
{
	SphericalJoint *pNew = new SphericalJoint (m_pModel);
	*pNew = *this;
	return (Selectable*) pNew;
}

bool SphericalJoint::fPaste (Selectable *pSelectable)
{
	SphericalJoint *pSource = dynamic_cast <SphericalJoint*> (pSelectable);

	if (pSource)
	{
		VectorXYZ vecPosition = m_vecPosition;
		*this = *pSource;
		m_vecPosition = vecPosition;
		return true;
	}

	return false;
}

void SphericalJoint::vUpdateDynamicsObject ()
{
}

void SphericalJoint::vSetDiameter (real rDiameter)
{
	unless (rDiameter > 0)
		rDiameter = 0.01;

	m_rDiameter = rDiameter;
	Generic::Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}

real SphericalJoint::rGetDiameter ()
{
	return m_rDiameter;
}

PropertyTreePtr SphericalJoint::ms_pPropertyTree;

Generic::PropertyTree* SphericalJoint::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		ms_pPropertyTree = new PropertyTree (*Component::pGetPropertyTree ());

		std::list<Property*> Bodies;	
		Bodies.push_back(new BodyProperty("Beam 1", (fnGetComponent)&Juice::Joint::pGetBody1, (fnSetComponent)&Juice::Joint::vSetBody1));
		Bodies.push_back(new BodyProperty("Beam 2", (fnGetComponent)&Juice::Joint::pGetBody2, (fnSetComponent)&Juice::Joint::vSetBody2));
		ms_pPropertyTree->vAddNode ("Beams", Bodies);

		SphericalJointProperty *pTwin = new SphericalJointProperty("Twin", (fnGetComponent)&Juice::Component::pGetTwin, (fnSetComponent)&Juice::Component::vSetTwin);
		ms_pPropertyTree->vAddNode (pTwin);

		std::list<Property*> Dimension;	
		Dimension.push_back(new RealProperty("Diameter", (Generic::fnGetReal) &Juice::SphericalJoint::rGetDiameter, (Generic::fnSetReal)  &Juice::SphericalJoint::vSetDiameter));
		ms_pPropertyTree->vAddNode ("Dimension", Dimension);
	}

	return ms_pPropertyTree;
}

void SphericalJoint::vGetPosition (VectorXYZ &vecPosition)
{
	vecPosition.x = vecPosition.y = vecPosition.z = 0;

	if (m_pModel->pGetApplication ()->eGetAppMode () == Juice::amDesign)
	{
		vecPosition = m_vecPosition;
	}
	else
	{
		if (m_pDynamicsObject)
			m_pDynamicsObject->vGetAnchor (vecPosition);
	}
}

// JointSpherical.cpp ends here ->