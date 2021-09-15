/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Joint.cpp

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
Joint::Joint (Model *pModel, const wchar_t *wszXmlTag) :
    Component (pModel, wszXmlTag),
	m_pBody1 (null), 
	m_pBody2 (null),
	m_rGain (10),
	m_rMaxForce (100),
	m_rPositionAccumulator (0),
	m_uPositions (0),
	m_rVelocityAccumulator (0),
	m_uVelocities (0),
	m_rLoStop (0),
	m_fLoStop (false),
	m_rHiStop (0),
	m_fHiStop (false)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
Joint::~Joint ()
{
}

/****************************************************************************/
/** The assignment is primarily used for clipboard operations
*****************************************************************************/
Joint& Joint::operator= (const Joint& CopyFrom)
{
	Component::operator= (CopyFrom);

	m_pBody1 = CopyFrom.m_pBody1;
	m_pBody2 = CopyFrom.m_pBody2;

	m_rLoStop = CopyFrom.m_rLoStop;
	m_rHiStop = CopyFrom.m_rHiStop;

	m_fLoStop = CopyFrom.m_fLoStop;
	m_fHiStop = CopyFrom.m_fHiStop;

	m_rGain     = CopyFrom.m_rGain;
	m_rMaxForce = CopyFrom.m_rMaxForce;

	return *this;
}

/****************************************************************************/
/* Motion functions
*****************************************************************************/

void Joint::vInitAccumulators ()
{
	m_rPositionAccumulator = 0;
	m_uPositions = 0;

	m_rVelocityAccumulator = 0;
	m_uVelocities = 0;
}

void Joint::vAddPosition (real rDelta)
{
	m_rPositionAccumulator += rDelta;
	m_uPositions++;
}

void Joint::vAddVelocity (real rDelta)
{
	m_rVelocityAccumulator += rDelta;
	m_uVelocities++;
}

void Joint::vApplyForce (real vVelocity, real rMaxForce)
{
	Breakpoint ("Joint::vApplyForce(2): base class implementation invoked.");
}

void Joint::vApplyForce (real rDesiredPosition)
{
	Breakpoint ("Joint::vApplyForce(1): base class implementation invoked.");
}

/****************************************************************************/
/* Property get/set functions
*****************************************************************************/
void Joint::vSetBody1 (Component *pBody)
{
	if ((m_pModel->pGetApplication()->m_eEditMode == emSymmetric) && m_pTwin)
	{
		Component *pThisBody = 0, *pTwinBody = 0;

		if (pBody)
		{
			VectorXYZ vecBodyPosition;
			pBody->vGetPosition (vecBodyPosition);

			if (pBody->pGetTwin ())
			{
				if (((m_vecPosition.x > 0) && (vecBodyPosition.x > 0)) ||
					(m_vecPosition.x < 0) && (vecBodyPosition.x < 0))
				{
					pThisBody = pBody;
					pTwinBody = pBody->pGetTwin ();
				}
				else
				{
					pThisBody = pBody->pGetTwin ();
					pTwinBody = pBody;
				}
			}
			else
			{
				pThisBody = pBody;
				pTwinBody = pBody;
			}
		}

		m_pBody1 = (Body*) pThisBody;
		m_pModel->pGetApplication()->m_eEditMode = emNormal;
		((Joint*) m_pTwin)->vSetBody1 (pTwinBody);
		m_pModel->pGetApplication()->m_eEditMode = emSymmetric;
	}
	else
	{
		m_pBody1 = (Body*) pBody;
	}

	if (m_pBody1)
		Generic::Listener<ComponentEvents>::vRegisterForEventsFrom (m_pBody1);

	Generic::Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}

Component* Joint::pGetBody1 ()
{
	return m_pBody1;
}

void Joint::vSetBody2 (Component *pBody)
{
	if ((m_pModel->pGetApplication()->m_eEditMode == emSymmetric) && m_pTwin)
	{
		Component *pThisBody = 0, *pTwinBody = 0;

		if (pBody)
		{
			VectorXYZ vecBodyPosition;
			pBody->vGetPosition (vecBodyPosition);

			if (pBody->pGetTwin ())
			{
				if (((m_vecPosition.x > 0) && (vecBodyPosition.x > 0)) ||
					(m_vecPosition.x < 0) && (vecBodyPosition.x < 0))
				{
					pThisBody = pBody;
					pTwinBody = pBody->pGetTwin ();
				}
				else
				{
					pThisBody = pBody->pGetTwin ();
					pTwinBody = pBody;
				}
			}
			else
			{
				pThisBody = pBody;
				pTwinBody = pBody;
			}
		}

		m_pBody2 = (Body*) pThisBody;
		m_pModel->pGetApplication()->m_eEditMode = emNormal;
		((Joint*) m_pTwin)->vSetBody2 (pTwinBody);
		m_pModel->pGetApplication()->m_eEditMode = emSymmetric;
	}
	else
	{
		m_pBody2 = (Body*) pBody;
	}

	if (m_pBody2)
		Generic::Listener<ComponentEvents>::vRegisterForEventsFrom (m_pBody2);

	Generic::Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}

Component* Joint::pGetBody2 ()
{
	return m_pBody2;
}

void Joint::vSetGain (real rGain)
{
	m_rGain = rGain;
	Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}

real Joint::rGetGain ()
{
	return m_rGain;
}

void Joint::vSetMaxForce (real rMaxForce)
{
	m_rMaxForce = rMaxForce;
	Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}

real Joint::rGetMaxForce ()
{
	return m_rMaxForce;
}

real Joint::rGetLoStop ()
{
	return m_rLoStop;
}

real Joint::rGetHiStop ()
{
	return m_rHiStop;
}

bool Joint::fGetLoStop ()
{
	return m_fLoStop;
}

bool Joint::fGetHiStop ()
{
	return m_fHiStop;
}

void Joint::vSetLoStop (real rStop)
{
	m_rLoStop = rStop;
	Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
	vUpdateDynamicsObject ();
}

void Joint::vSetHiStop (real rStop)
{
	m_rHiStop = rStop;
	Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
	vUpdateDynamicsObject ();
}

void Joint::vSetLoStop (bool fStop)
{
	m_fLoStop = fStop;
	Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
	vUpdateDynamicsObject ();
}

void Joint::vSetHiStop (bool fStop)
{
	m_fHiStop = fStop;
	Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
	vUpdateDynamicsObject ();
}


/****************************************************************************/
/**
*****************************************************************************/
PropertyTreePtr Joint::ms_pPropertyTree;

Generic::PropertyTree* Joint::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Component::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		std::list<Property*> Bodies;	

		Bodies.push_back(new BodyProperty("Beam 1", (fnGetComponent)&Juice::Joint::pGetBody1, (fnSetComponent)&Juice::Joint::vSetBody1));
		Bodies.push_back(new BodyProperty("Beam 2", (fnGetComponent)&Juice::Joint::pGetBody2, (fnSetComponent)&Juice::Joint::vSetBody2));

		ms_pPropertyTree->vAddNode ("Beams", Bodies);

		RealProperty *pGain = new RealProperty("Gain", (Generic::fnGetReal) &Juice::Joint::rGetGain, (Generic::fnSetReal) &Juice::Joint::vSetGain);
		ms_pPropertyTree->vAddNode (pGain);

		RealProperty *pMaxForce = new RealProperty("Max. Force", (Generic::fnGetReal) &Juice::Joint::rGetMaxForce, (Generic::fnSetReal) &Juice::Joint::vSetMaxForce);
		ms_pPropertyTree->vAddNode (pMaxForce);

		std::list<Property*> Stops;
		Stops.push_back(new RealProperty("Limit Lo", (Generic::fnGetReal) &Juice::Joint::rGetLoStop, (Generic::fnSetReal) &Juice::Joint::vSetLoStop));
		Stops.push_back(new RealProperty("Limit Hi", (Generic::fnGetReal) &Juice::Joint::rGetHiStop, (Generic::fnSetReal) &Juice::Joint::vSetHiStop));
		Stops.push_back(new BoolProperty("Limit Lo Enabled", (Generic::fnGetBool) &Juice::Joint::fGetLoStop, (Generic::fnSetBool) &Juice::Joint::vSetLoStop));
		Stops.push_back(new BoolProperty("Limit Hi Enabled", (Generic::fnGetBool) &Juice::Joint::fGetHiStop, (Generic::fnSetBool) &Juice::Joint::vSetHiStop));

		ms_pPropertyTree->vAddNode ("Limits", Stops);
	}

	return ms_pPropertyTree;
}


/****************************************************************************/
/**
*****************************************************************************/
void Joint::Notifier_Deleted (Component *pComponent)
{
	Component::Notifier_Deleted (pComponent);

	if (pComponent == m_pBody1)
		m_pBody1 = null;

	if (pComponent == m_pBody2)
		m_pBody2 = null;
}

/****************************************************************************/
/** Only the derived class version should ever be called.
*****************************************************************************/
void Joint::vUpdateDynamicsObject ()
{
	Breakpoint ("Joint::vUpdateDynamicsObject: base implementation invoked.");
}

// Joint.cpp ends here ->