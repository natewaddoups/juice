/*****************************************************************************
<HEADER>
*****************************************************************************/

#include <JuiceEngine.h>

using namespace Juice;

/****************************************************************************/
/** ClockworkMotor constructor 
***
*** Note that the caller (e.g. Behavior or JuiceParse) must set the joint 
*** pointer later with vSetJoint in order for the ClockworkMotor to be worth 
*** anything.
*****************************************************************************/
ClockworkMotor::ClockworkMotor (Behavior* pBehavior, Joint *pJoint) :
	Motor (pBehavior, pJoint, L"ClockworkMotor"),
	m_fEnabled (true),
//	m_pSource (null),
	m_eMotionType (mtNone),
	m_rPhase (0),
	m_rAmplitude (0),
	m_rStaticOffset (0)
{
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

	m_pJoint = src.m_pJoint;
	m_fEnabled = src.m_fEnabled;
	m_rPhase = src.m_rPhase;
	m_rAmplitude = src.m_rAmplitude;
	m_rStaticOffset = src.m_rStaticOffset;
	m_eMotionType = src.m_eMotionType;

	return *this;
}

Behavior* ClockworkMotor::pGetBehavior ()
{
	return m_pBehavior;
}

void ClockworkMotor::vPrepareSimulation ()
{
}

void ClockworkMotor::vPaste (ClockworkMotor *pNewClockworkMotor)
{
	m_fEnabled = pNewClockworkMotor->m_fEnabled;
	m_rPhase = pNewClockworkMotor->m_rPhase;
	m_rAmplitude = pNewClockworkMotor->m_rAmplitude;
	m_rStaticOffset = pNewClockworkMotor->m_rStaticOffset;
	m_eMotionType = pNewClockworkMotor->m_eMotionType;
}

const char* ClockworkMotor::szGetName ()
{
	if (m_pJoint)
		return m_pJoint->szGetName ();
	else
		return "(null)";
}

MotionType ClockworkMotor::eGetMotionType () const
{
	return m_eMotionType;
}

void ClockworkMotor::vSetJoint (Joint *pJoint)
{
	m_pJoint = pJoint;

	if (!pJoint)
	{
		Breakpoint ();
		return;
	}

	vRegisterForEventsFrom (pJoint);
	Notifier<ClockworkMotorEvents>::vNotify (ClockworkMotorEvents::ClockworkMotor_Change);
}

Joint* ClockworkMotor::pGetJoint ()
{
	return m_pJoint;
}

void ClockworkMotor::vSetEnableState (bool fEnabled)
{
	m_fEnabled = fEnabled;
	Notifier<ClockworkMotorEvents>::vNotify (ClockworkMotorEvents::ClockworkMotor_Change);
}

bool ClockworkMotor::fGetEnableState ()
{
	return m_fEnabled;
}
/*
void ClockworkMotor::vSetMotionType (MotionType eMotionType)
{
	m_eMotionType = eMotionType;
}

MotionType ClockworkMotor::eGetMotionType ()
{
	return m_eMotionType;
}
*/
void ClockworkMotor::vSetMotionType (int iMotionType)
{
	m_eMotionType = (MotionType) iMotionType;
	Notifier<ClockworkMotorEvents>::vNotify (ClockworkMotorEvents::ClockworkMotor_Change);
}

int ClockworkMotor::iGetMotionType ()
{
	return (int) m_eMotionType;
}

void ClockworkMotor::vSetPhase (real rPhase)
{
	m_rPhase = rPhase;
	Notifier<ClockworkMotorEvents>::vNotify (ClockworkMotorEvents::ClockworkMotor_Change);
}

real ClockworkMotor::rGetPhase ()
{
	return m_rPhase;
}

void ClockworkMotor::vSetAmplitude (real rAmplitude)
{
	m_rAmplitude = rAmplitude;
	Notifier<ClockworkMotorEvents>::vNotify (ClockworkMotorEvents::ClockworkMotor_Change);
}

real ClockworkMotor::rGetAmplitude ()
{
	return m_rAmplitude;
}

void ClockworkMotor::vSetStaticOffset (real rOffset)
{
	m_rStaticOffset = rOffset;
	Notifier<ClockworkMotorEvents>::vNotify (ClockworkMotorEvents::ClockworkMotor_Change);
}

real ClockworkMotor::rGetStaticOffset ()
{
	return m_rStaticOffset;
}

PropertyTree* ClockworkMotor::ms_pPropertyTree;

Generic::PropertyTree* ClockworkMotor::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		ms_pPropertyTree = new Generic::PropertyTree;

// just an idea...
//		ComponentProperty *pBody = new ComponentProperty ("Feedback Source", m_pBody, (fnSetComponent) vSetBody);
//		ms_pPropertyTree->vAddNode (pBody);

		StringProperty *pName = new StringProperty ("Name", (Generic::fnGetString) szGetName, null);
		ms_pPropertyTree->vAddNode (pName);

		BoolProperty *pEnabled = new BoolProperty ("Enabled", (Generic::fnGetBool) fGetEnableState, (Generic::fnSetBool) vSetEnableState);
		ms_pPropertyTree->vAddNode (pEnabled);

//		MotionTypeProperty *pMotionType = new MotionTypeProperty ("Type", (fnGetMotionType) eGetMotionType, (fnSetMotionType) vSetMotionType);
//		ms_pPropertyTree->vAddNode (pMotionType);

		ListProperty *pMotionType = new ListProperty ("Type", (fnGetInt) (iGetMotionType), (fnSetInt) vSetMotionType);

		pMotionType->vAddItem (mtNone,       "None");
		pMotionType->vAddItem (mtSine,       "Sine");
		pMotionType->vAddItem (mtDoubleSine, "DoubleSine");
		pMotionType->vAddItem (mtStep,       "Step");
		pMotionType->vAddItem (mtContinuous, "Continuous");
		ms_pPropertyTree->vAddNode (pMotionType);

		RealProperty *pPhase = new RealProperty ("Phase", (Generic::fnGetReal) rGetPhase, (Generic::fnSetReal) vSetPhase);
		ms_pPropertyTree->vAddNode (pPhase);

		RealProperty *pAmplitude = new RealProperty ("Amplitude", (Generic::fnGetReal) rGetAmplitude, (Generic::fnSetReal) vSetAmplitude);
		ms_pPropertyTree->vAddNode (pAmplitude);

		RealProperty *pOffset = new RealProperty ("Offset", (Generic::fnGetReal) rGetStaticOffset, (Generic::fnSetReal) vSetStaticOffset);
		ms_pPropertyTree->vAddNode (pOffset);
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

	Notifier<ClockworkMotorEvents>::vNotify (ClockworkMotorEvents::ClockworkMotor_Change);
}

/*****************************************************************************
<FOOTER>
*****************************************************************************/
