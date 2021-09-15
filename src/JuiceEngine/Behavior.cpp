/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Behavior.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>

using namespace Juice;

/****************************************************************************/
/** Caller should invoke the vInitializeMotorList method after creating.
*** I would call it from the ctor, but then you get a pure virtual function
*** call during
*****************************************************************************/
Behavior::Behavior (Model *pModel, const wchar_t *wszXmlTag) : 
	XML::Serializable (wszXmlTag),
	Selectable (pModel),
	m_pModel (pModel), 
	m_fEnabled (true), 
	m_rGain (100.0)
{
	if (pModel)
		Generic::Listener<ModelEvents>::vRegisterForEventsFrom (pModel);
}

/****************************************************************************/
/** Destructor
*****************************************************************************/
Behavior::~Behavior ()
{
}

/****************************************************************************/
/** Assignment operator, used almost exclusively during clipboard operations
*****************************************************************************/
Behavior& Behavior::operator= (const Behavior &src)
{
	m_strName = src.m_strName;
	m_pModel = src.m_pModel;
	m_fEnabled = src.m_fEnabled;
	m_rGain = src.m_rGain;

	return *this;
}

/****************************************************************************/
/** This method is not needed in the base class, perhaps should be {} in
*** the header file?
*****************************************************************************/
void Behavior::vResetPointers (BodyHashMap &BodyMap, JointHashMap &JointMap)
{
}

/****************************************************************************/
/** This method is not needed in the base class, perhaps should be {} in
*** the header file?
*****************************************************************************/
void Behavior::vPrepareSimulation ()
{
}

/****************************************************************************/
/* 'Getter' and 'setter' functions for behavior properties. these are pretty
 * straightforward.
*****************************************************************************/
void Behavior::vSetName (const char *szName)
{
	m_strName = szName;

	Generic::Notifier<BehaviorEvents>::vNotify (&Juice::BehaviorEvents::Behavior_Changed);
}

const char* Behavior::szGetName ()
{
	return m_strName.c_str();
}

bool Behavior::fGetEnableState ()
{
	return m_fEnabled;
}

Model* Behavior::pGetModel ()
{
	return m_pModel;
}

void Behavior::vSetEnableState (bool fEnabled)
{
	m_fEnabled = fEnabled;
	Generic::Notifier<BehaviorEvents>::vNotify(&Juice::BehaviorEvents::Behavior_Changed);
}

void Behavior::vSetGain (real rGain)
{
	m_rGain = rGain;
	Generic::Notifier<BehaviorEvents>::vNotify(&Juice::BehaviorEvents::Behavior_Changed);
}

real Behavior::rGetGain ()
{
	return m_rGain;
}

/****************************************************************************/
/* Build the property tree
*****************************************************************************/

PropertyTreePtr Behavior::ms_pPropertyTree;

Generic::PropertyTree* Behavior::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		ms_pPropertyTree = new Generic::PropertyTree;

		StringProperty *pName = new StringProperty("Name", (Generic::fnGetString) &Juice::Behavior::szGetName, (Generic::fnSetString) &Juice::Behavior::vSetName);
		ms_pPropertyTree->vAddNode (pName);

		BoolProperty *pEnabled = new BoolProperty("Enabled", (Generic::fnGetBool) &Juice::Behavior::fGetEnableState, (Generic::fnSetBool) &Juice::Behavior::vSetEnableState);
		ms_pPropertyTree->vAddNode (pEnabled);

		RealProperty *pGain = new RealProperty("Gain", (Generic::fnGetReal) &Juice::Behavior::rGetGain, (Generic::fnSetReal) &Juice::Behavior::vSetGain);
		ms_pPropertyTree->vAddNode (pGain);
	}

	return ms_pPropertyTree;
}

/****************************************************************************/
/** vChanged is just a wrapper for the Notifier::vNotify call.  This is 
 ** mostly just because it's less typing this way. :-)
*****************************************************************************/
void Behavior::vChanged ()
{
	Generic::Notifier<BehaviorEvents>::vNotify(&Juice::BehaviorEvents::Behavior_Changed);
}

// Behavior.cpp ends here ->