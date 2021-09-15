/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: ClockworkMotor.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>

using namespace Juice;

wchar_t *ClockworkMotor::ms_wszXmlTag = L"ClockworkMotor";

/****************************************************************************/
/** ClockworkMotor constructor 
***
*** Note that the caller (e.g. Behavior or JuiceParse) must set the joint 
*** pointer later with vSetJoint in order for the ClockworkMotor to be worth 
*** anything.
*****************************************************************************/
ClockworkMotor::ClockworkMotor (ClockworkBehavior* pBehavior, Joint *pJoint) :
	XML::Serializable (ms_wszXmlTag, L"MotionLink"),
	m_eMotionType (mtNone),
	m_rPhase (0),
	m_rAmplitude (0),
	Selectable (pBehavior),
	m_pBehavior (pBehavior),
	m_pJoint (pJoint),
	m_fEnabled (true),
	m_rStaticOffset (0)
{
	if (pJoint)
		Generic::Listener<ComponentEvents>::vRegisterForEventsFrom (pJoint);
}

/****************************************************************************/
/** 
*****************************************************************************/
ClockworkMotor::~ClockworkMotor ()
{
}

/****************************************************************************/
/** This is assumed to be called from the context of a clipboard copy 
*** operation, so it copies all of the members of the source object EXCEPT
*** the Behavior pointer.  The purpose here is to update the object's 
*** properies, not move it to a new behavior.
*****************************************************************************/
ClockworkMotor& ClockworkMotor::operator = (const ClockworkMotor &src)
{
	// Behavior pointer remains as-is!
	// m_pBehavior = src.m_pBehavior;
	m_pBehavior = src.m_pBehavior;
	m_pJoint = src.m_pJoint;
	m_fEnabled = src.m_fEnabled;
	m_rStaticOffset = src.m_rStaticOffset;
	m_rPhase = src.m_rPhase;
	m_rAmplitude = src.m_rAmplitude;
	m_eMotionType = src.m_eMotionType;

	return *this;
}

bool ClockworkMotor::fPaste (Selectable *pSource)
{
	if (typeid (*pSource) != typeid (*this))
		return false;

	ClockworkMotor *pNewClockworkMotor = dynamic_cast <ClockworkMotor*> (pSource);

	*this = *pNewClockworkMotor;

	return true;
}

Selectable* ClockworkMotor::pCreateClipboardCopy () const
{
	ClockworkMotor *pNew = new ClockworkMotor (m_pBehavior, m_pJoint);
	*pNew = *this;
	return (Selectable*) pNew;
}


/****************************************************************************/
/** 
*****************************************************************************/
void ClockworkMotor::vPrepareSimulation ()
{
}

ClockworkMotor* ClockworkMotor::pGetTwin ()
{
	if (!m_pJoint)
		return null;

    return m_pBehavior->pGetMotorFor (m_pJoint);
}

void ClockworkMotor::vSetJoint (Joint *pJoint)
{
	m_pJoint = pJoint;

	if (!pJoint)
	{
		Breakpoint ("Motor::vSetJoint: null pJoint.");
		return;
	}

	vRegisterForEventsFrom (pJoint);
	Notifier<ClockworkMotorEvents>::vNotify (&Juice::ClockworkMotorEvents::ClockworkMotor_Changed);
}

Joint* ClockworkMotor::pGetJoint () const
{
	return m_pJoint;
}

ClockworkBehavior* ClockworkMotor::pGetBehavior ()
{
	return m_pBehavior;
}

const char* ClockworkMotor::szGetName ()
{
	if (!m_pJoint)
		return "(null)";

    return m_pJoint->szGetName ();
}

void ClockworkMotor::vSetEnableState (bool fEnabled)
{
	m_fEnabled = fEnabled;
	Notifier<ClockworkMotorEvents>::vNotify(&Juice::ClockworkMotorEvents::ClockworkMotor_Changed);
}

bool ClockworkMotor::fGetEnableState ()
{
	return m_fEnabled;
}

void ClockworkMotor::vSetStaticOffset (real rOffset)
{
	m_rStaticOffset = rOffset;
	Notifier<ClockworkMotorEvents>::vNotify(&Juice::ClockworkMotorEvents::ClockworkMotor_Changed);
}

real ClockworkMotor::rGetStaticOffset ()
{
	return m_rStaticOffset;
}


MotionType ClockworkMotor::eGetMotionType () const
{
	return m_eMotionType;
}

void ClockworkMotor::vSetMotionType (int iMotionType)
{
	m_eMotionType = (MotionType) iMotionType;
	Notifier<ClockworkMotorEvents>::vNotify(&Juice::ClockworkMotorEvents::ClockworkMotor_Changed);
}

int ClockworkMotor::iGetMotionType ()
{
	return (int) m_eMotionType;
}

void ClockworkMotor::vSetPhase (real rPhase)
{
	m_rPhase = rPhase;
	Notifier<ClockworkMotorEvents>::vNotify(&Juice::ClockworkMotorEvents::ClockworkMotor_Changed);
}

real ClockworkMotor::rGetPhase ()
{
	return m_rPhase;
}

void ClockworkMotor::vSetAmplitude (real rAmplitude)
{
	m_rAmplitude = rAmplitude;
	Notifier<ClockworkMotorEvents>::vNotify(&Juice::ClockworkMotorEvents::ClockworkMotor_Changed);
}

real ClockworkMotor::rGetAmplitude ()
{
	return m_rAmplitude;
}

PropertyTreePtr ClockworkMotor::ms_pPropertyTree;

Generic::PropertyTree* ClockworkMotor::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		ms_pPropertyTree = new Generic::PropertyTree;

		StringProperty *pName = new StringProperty("Name", (Generic::fnGetString) &Juice::ClockworkMotor::szGetName, null);
		ms_pPropertyTree->vAddNode (pName);

		BoolProperty *pEnabled = new BoolProperty("Enabled", (Generic::fnGetBool) &Juice::ClockworkMotor::fGetEnableState, (Generic::fnSetBool) &Juice::ClockworkMotor::vSetEnableState);
		ms_pPropertyTree->vAddNode (pEnabled);

		RealProperty *pOffset = new RealProperty("Offset", (Generic::fnGetReal) &Juice::ClockworkMotor::rGetStaticOffset, (Generic::fnSetReal) &Juice::ClockworkMotor::vSetStaticOffset);
		ms_pPropertyTree->vAddNode (pOffset);

		ListProperty *pMotionType = new ListProperty("Type", (fnGetInt)&Juice::ClockworkMotor::iGetMotionType, (fnSetInt)&Juice::ClockworkMotor::vSetMotionType);

		pMotionType->vAddItem (mtNone,       "None");
		pMotionType->vAddItem (mtSine,       "Sine");
		pMotionType->vAddItem (mtDoubleSine, "DoubleSine");
		pMotionType->vAddItem (mtStep,       "Step");
		pMotionType->vAddItem (mtContinuous, "Continuous");
		ms_pPropertyTree->vAddNode (pMotionType);

		RealProperty *pPhase = new RealProperty("Phase", (Generic::fnGetReal) &Juice::ClockworkMotor::rGetPhase, (Generic::fnSetReal) &Juice::ClockworkMotor::vSetPhase);
		ms_pPropertyTree->vAddNode (pPhase);

		RealProperty *pAmplitude = new RealProperty("Amplitude", (Generic::fnGetReal) &Juice::ClockworkMotor::rGetAmplitude, (Generic::fnSetReal) &Juice::ClockworkMotor::vSetAmplitude);
		ms_pPropertyTree->vAddNode (pAmplitude);
	}

	return ms_pPropertyTree;
}

void ClockworkMotor::vSelect ()
{
	Selectable::vSelect ();
	if (m_pJoint)
		m_pJoint->vSelect ();
}

void ClockworkMotor::vDeselect ()
{
	Selectable::vDeselect ();
	if (m_pJoint)
		m_pJoint->vDeselect ();
}

void ClockworkMotor::Notifier_Deleted (Component *pComponent)
{
	if (pComponent != m_pJoint)
		return;

	if (!pComponent)
		return;

	m_pJoint = null;

	delete this;
}

void ClockworkMotor::Component_Change (Component *pComponent)
{
	if (pComponent != m_pJoint)
		return;

	if (!pComponent)
		return;

	Notifier<ClockworkMotorEvents>::vNotify(&Juice::ClockworkMotorEvents::ClockworkMotor_Changed);
}

// ClockworkMotor.cpp ends here ->