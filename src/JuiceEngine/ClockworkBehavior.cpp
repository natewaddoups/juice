/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: ClockworkBehavior.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>

using namespace Juice;

wchar_t *ClockworkBehavior::ms_wszXmlTag = L"Clockwork";

Behavior* ClockworkBehaviorCreator (Model *pModel, bool fLoadFromFile)
{
	return (Behavior*) new ClockworkBehavior (pModel);
}

void ClockworkMotorList::vCreateNew (ClockworkMotor **ppNew, bool *pfAddToList)
{
	*ppNew = m_pBehavior->pCreateMotor (null);
	*pfAddToList = false;
}

BehaviorClass ClockworkBehaviorClass =
{
	ClockworkBehaviorCreator,
	ClockworkBehavior::ms_wszXmlTag
};


/****************************************************************************/
/** Constructor - just initializes various members to reasonable defaults.
*****************************************************************************/
ClockworkBehavior::ClockworkBehavior (Model *pModel) : 
	Behavior (pModel, ms_wszXmlTag),
	m_eMotionSource (msMainEngine),
	m_eAmpModSource (amsNone),
	m_fAbsoluteValue (false),
	m_pFeedbackSource (null),
	m_lsMotors (this)
{
}

/****************************************************************************/
/** The destructor empties the motor list, deleting motors as it goes
*****************************************************************************/
ClockworkBehavior::~ClockworkBehavior ()
{
	while (m_lsMotors.size ())
	{
		ClockworkMotor *pMotor = m_lsMotors.front ();
		m_lsMotors.pop_front ();
		delete pMotor;
	}
}

/****************************************************************************/
/** Ensure that the behavior will be notified if the source is deleted
*****************************************************************************/
void ClockworkBehavior::vSetFeedbackSource_Internal (Component *pFeedbackSource)
{
	if (m_pFeedbackSource)
		Generic::Listener<ComponentEvents>::vUnregister (m_pFeedbackSource);

	m_pFeedbackSource = (Body*) pFeedbackSource;

	if (m_pFeedbackSource)
		Generic::Listener<ComponentEvents>::vRegisterForEventsFrom (m_pFeedbackSource);
}

/****************************************************************************/
/** The assignment operator is pretty straightforward, and is used only 
 ** during clipboard operations.
*****************************************************************************/
ClockworkBehavior& ClockworkBehavior::operator = (const ClockworkBehavior &src)
{
	Behavior::operator= (src);

	m_eMotionSource = src.m_eMotionSource;
	m_eAmpModSource = src.m_eAmpModSource;
	m_fAbsoluteValue = src.m_fAbsoluteValue;

	vSetFeedbackSource_Internal (src.m_pFeedbackSource);

	return *this;
}

/****************************************************************************/
/** If the user tries to paste an object over a selected object, this function
 ** tweaks the selected object to match the one on the clipboard.
*****************************************************************************/
bool ClockworkBehavior::fPaste (Selectable *pSource)
{
	if (typeid (*pSource) != typeid (*this))
		return false;

	ClockworkBehavior *pNewClockworkBehavior = dynamic_cast <ClockworkBehavior*> (pSource);

	*this = *pNewClockworkBehavior;

	return true;
}

/****************************************************************************/
/** Create a copy of this object.  This is used only during clipboard 
 ** operations.  The copy will be placed on the clipboard, or perhaps be
 ** placed directly into the model.
*****************************************************************************/
Selectable* ClockworkBehavior::pCreateClipboardCopy () const
{
	ClockworkBehavior *pNew = new ClockworkBehavior (m_pModel);

	pNew->vCreateMotorList (this);

	// Would have preferred to use szGetName () here, but that would require
	// making szGetName() const, which breaks the signature needed for the 
	// property get/set stuff.
	pNew->vSetName (m_strName.c_str()); 
	
	*pNew = *this;
	
	return (Selectable*) pNew;
}

/****************************************************************************/
/** This is called as the application goes from design mode to simulation 
 ** mode.  It delegates the call to each of the motor objects.
*****************************************************************************/
void ClockworkBehavior::vPrepareSimulation ()
{
	// iterate motors
	ClockworkMotorList::iterator iterMotors;
	for (iterMotors = m_lsMotors.begin (); iterMotors != m_lsMotors.end (); iterMotors++)
	{
		ClockworkMotor *pMotor = *iterMotors;
		if (!pMotor)
			continue;

		pMotor->vPrepareSimulation ();
	}
}

/****************************************************************************/
/** Initialize this object's motor list with a list or motors corresponding to 
 ** the motors in the given object.
 **
 ** The role of this function escapes me... is it left over from the pre-
 ** network behavior days?
*****************************************************************************/
void ClockworkBehavior::vCreateMotorList (const ClockworkBehavior *pSourceBehavior)
{
	const ClockworkMotorList &lsSourceMotors = pSourceBehavior->m_lsMotors;
	ClockworkMotorList::const_iterator iter, iterLast = lsSourceMotors.end ();
	for (iter = lsSourceMotors.begin (); iter != iterLast; iter++)
	{
		const ClockworkMotor *pSourceMotor = dynamic_cast <const ClockworkMotor*> (*iter);

		// this creates the new motor, adds it to the list, and subscribes to events from the joint
		ClockworkMotor *pNewMotor = dynamic_cast <ClockworkMotor*> (pCreateMotor (pSourceMotor->pGetJoint ()));
		*pNewMotor = *pSourceMotor;		
	}
}

/****************************************************************************/
/** Return a reference to the list of motors.  
*****************************************************************************/
const ClockworkMotorList& ClockworkBehavior::lsGetMotors ()
{
	return m_lsMotors;
}

/****************************************************************************/
/** Builds the behaviors motor list, based on the joints in the model.  This 
 ** function just iterates the model's component list, the real work is done
 ** in the Mode_AddComponent method.
*****************************************************************************/
void ClockworkBehavior::vInitializeMotorList ()
{
	// Get the model's component list
	ComponentList lsComponents;
	m_pModel->vGetComponents (lsComponents);

	// Iterate the component list
	ComponentList::iterator iter;
	for (iter = lsComponents.begin (); iter != lsComponents.end (); iter++)
	{
		Component *pComponent = *iter;

		// Is the component a motorizable joint?
		if (pComponent && 
			(typeid (*pComponent) == typeid (RevoluteJoint)) ||
			(typeid (*pComponent) == typeid (PrismaticJoint)))
		{
			// Pretend this component just got added to the model (create
			// the motor, etc).
			Model_AddComponent (m_pModel, pComponent);
		}
	}
}

/****************************************************************************/
/** Create a motor, notify the world of the motor's creation, and listen
 ** for events from the motor (the 'deleted' event is the main thing).
*****************************************************************************/
ClockworkMotor* ClockworkBehavior::pCreateMotor (Joint *pJoint)
{
	ClockworkMotor *pNew = new ClockworkMotor (this, pJoint);

	m_lsMotors.push_back (pNew);
	Generic::Listener<ClockworkMotorEvents>::vRegisterForEventsFrom (pNew);
	Generic::Notifier<ClockworkBehaviorEvents>::vNotify (&Juice::ClockworkBehaviorEvents::ClockworkBehavior_AddMotor, pNew);

	return pNew;
}

/****************************************************************************/
/** Return a pointer for the motor that powers the given joint.
*****************************************************************************/
ClockworkMotor* ClockworkBehavior::pGetMotorFor (Joint *pJoint)
{
	ClockworkMotorList::iterator iter;
	for (iter = m_lsMotors.begin (); iter != m_lsMotors.end (); iter++)
	{
		ClockworkMotor *pMotor = *iter;
		if (!pMotor)
			continue;
		
		Joint *pMotorJoint = pMotor->pGetJoint();
		
		if (pMotorJoint == pJoint)
			return pMotor;
	}

	return null;
}


/****************************************************************************/
/** The behavior constructor adds a motion link for each joint, but issuing
*** Behavior_AddMotionLink during that time is useless.  The Behavior is still
*** being constructed, so nobody has a pointer to it yet, so the UI hasn't
*** had a chance to call vRegisterEventsFrom (pTheNewBehavior) just yet.
***
*** This function basically allows the UI a chance to get all those 
*** AddMotionLink notifications that it missed while the Behavior was being
*** constructed.
***
*** I kinda have mixed feelings about it, seems like a hack.
*****************************************************************************/
void ClockworkBehavior::vNotifyOfMotors ()
{
	ClockworkMotorList::iterator iter;
	for (iter = m_lsMotors.begin (); iter != m_lsMotors.end (); iter++)
	{
		ClockworkMotor *pMotor = *iter;
		if (pMotor)
			Generic::Notifier<ClockworkBehaviorEvents>::vNotify(&Juice::ClockworkBehaviorEvents::ClockworkBehavior_AddMotor, pMotor);
		else
			Breakpoint ("ClockworkBehavior::vNotifyOfMotors: null motor.");
	}
}

/****************************************************************************/
/** Each motor has a pointer to the joint it drives.  After reading a model
 ** from a file, these 'pointers' are bogus, they're actually IDs.  This
 ** function turns the IDs into pointers, using information in the given maps.
 **
 ** Note that this function does NOT use vSetFeedbackSource_Internal, because
 ** unregistering from the old 'source' would fail because it's really an ID, 
 ** not a pointer.
*****************************************************************************/
void ClockworkBehavior::vResetPointers (BodyHashMap &BodyMap, JointHashMap &JointMap)
{
	Behavior::vResetPointers (BodyMap, JointMap);

	int iBodies = BodyMap.size ();
	int iJoints = JointMap.size ();

	m_lsMotors.vSetJointPointers (JointMap);

	BodyHashMap::iterator iter = BodyMap.find ((Generic::UINT32) m_pFeedbackSource);

	if (iter == BodyMap.end ())
	{
		m_pFeedbackSource = null;
	}
	else
	{		
		m_pFeedbackSource = (Body*) (*iter).second;

		if (m_pFeedbackSource)
			Generic::Listener<ComponentEvents>::vRegisterForEventsFrom (m_pFeedbackSource);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void ClockworkBehavior::vSetMotionSource (int iSource)
{
	m_eMotionSource = (MotionSource) iSource;
	vChanged ();
}

int ClockworkBehavior::iGetMotionSource ()
{
	return m_eMotionSource;
}

void ClockworkBehavior::vSetAmpModSource (int iSource)
{
	m_eAmpModSource = (AmpModSource) iSource;
	vChanged ();
}

int ClockworkBehavior::iGetAmpModSource ()
{
	return m_eAmpModSource;
}

void ClockworkBehavior::vSetAbsoluteValue (bool fAbsoluteValue)
{
	m_fAbsoluteValue = fAbsoluteValue;
	vChanged ();
}

bool ClockworkBehavior::fGetAbsoluteValue ()
{
	return m_fAbsoluteValue;
}

void ClockworkBehavior::vSetFeedbackSource (Component *pFeedbackSource)
{
	vSetFeedbackSource_Internal (pFeedbackSource); 
	vChanged ();
}

Component* ClockworkBehavior::pGetFeedbackSource ()
{
	return m_pFeedbackSource;
}


/****************************************************************************/
/** 
*****************************************************************************/

PropertyTreePtr ClockworkBehavior::ms_pPropertyTree;

Generic::PropertyTree* ClockworkBehavior::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		//ms_pPropertyTree = new Generic::PropertyTree;
		PropertyTree *pParentTree = Behavior::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		ListProperty *pMotionSource = new ListProperty("Control Source", (fnGetInt)&Juice::ClockworkBehavior::iGetMotionSource, (fnSetInt)&Juice::ClockworkBehavior::vSetMotionSource);
		pMotionSource->vAddItem (msMainEngine, "Main Engine");
		pMotionSource->vAddItem (msJoystick1X, "Joystick 1 X");
		pMotionSource->vAddItem (msJoystick1Y, "Joystick 1 Y");
		pMotionSource->vAddItem (msJoystick1Z, "Joystick 1 Z");
		pMotionSource->vAddItem (msJoystick1R, "Joystick 1 R");
		pMotionSource->vAddItem (msJoystick2X, "Joystick 2 X");
		pMotionSource->vAddItem (msJoystick2Y, "Joystick 2 Y");
		pMotionSource->vAddItem (msJoystick2Z, "Joystick 2 Z");
		pMotionSource->vAddItem (msJoystick2R, "Joystick 2 R");
		pMotionSource->vAddItem (msBodyYaw,   "Body Yaw Feedback");
		pMotionSource->vAddItem (msBodyPitch, "Body Pitch Feedback");
		pMotionSource->vAddItem (msBodyRoll,  "Body Roll Feedback");

		ms_pPropertyTree->vAddNode (pMotionSource);

		BodyProperty *pFeedbackSource = new BodyProperty("Feedback Source", (fnGetComponent)&Juice::ClockworkBehavior::pGetFeedbackSource, (fnSetComponent)&Juice::ClockworkBehavior::vSetFeedbackSource);
		ms_pPropertyTree->vAddNode (pFeedbackSource);

		ListProperty *pAmpModSource = new ListProperty("Amp Mod", (fnGetInt)&Juice::ClockworkBehavior::iGetAmpModSource, (fnSetInt)&Juice::ClockworkBehavior::vSetAmpModSource);
		pAmpModSource->vAddItem (amsNone, "None");
		pAmpModSource->vAddItem (amsJoystickX, "Joystick X");
		pAmpModSource->vAddItem (amsJoystickY, "Joystick Y");
		pAmpModSource->vAddItem (amsJoystickZ, "Joystick Z");
		pAmpModSource->vAddItem (amsJoystickR, "Joystick R");

		ms_pPropertyTree->vAddNode (pAmpModSource);

		BoolProperty *pAbsVal = new BoolProperty("Absolute Value", (Generic::fnGetBool) &Juice::ClockworkBehavior::fGetAbsoluteValue, (Generic::fnSetBool) &Juice::ClockworkBehavior::vSetAbsoluteValue);
		ms_pPropertyTree->vAddNode (pAbsVal);
	}

	return ms_pPropertyTree;
}

void ClockworkBehavior::Notifier_Deleted (Component *pComponent)
{
	if (m_pFeedbackSource == pComponent)
	{
		m_pFeedbackSource = null;
	}
	else
	{
		Breakpoint ("ClockworkBehavior::Notifier_Deleted: notifier != member");
	}
}

void ClockworkBehavior::ClockworkMotor_Changed (ClockworkMotor *pLink)
{
	Generic::Notifier<BehaviorEvents>::vNotify(&Juice::BehaviorEvents::Behavior_Changed);
}

void ClockworkBehavior::Model_AddComponent (Model *pModel, Component *pComponent)
{
	if (!pComponent)
		return;
	
	if ((typeid (*pComponent) != typeid (RevoluteJoint)) &&
		(typeid (*pComponent) != typeid (PrismaticJoint)))
	{
		return;
	}

	ClockworkMotor *pMotor = pCreateMotor ((Joint*) pComponent);
}

void ClockworkBehavior::Notifier_Deleted (ClockworkMotor *pMotor)
{
	m_lsMotors.remove (pMotor);
}

/****************************************************************************/
/** 
*****************************************************************************/
std::vector<real> *ClockworkBehavior::pCreateVector ()
{
	std::vector<real> *pVector = new std::vector<real> ();
	std::vector<real> &Vector = *pVector; // just makes for less typing

	int iParametersPerMotor = 3; // offset, phase, amplitude
	int iParameters = iParametersPerMotor * m_lsMotors.size ();
	pVector->resize (iParameters); 

	ClockworkMotorList::iterator iterMotors;
	int i = 0;
	for (iterMotors = m_lsMotors.begin (); iterMotors != m_lsMotors.end (); iterMotors++)
	{
		ClockworkMotor *pMotor = *iterMotors;
		if (!pMotor)
			continue;

		Vector[i++] = pMotor->rGetStaticOffset () / 180.0;
		Vector[i++] = pMotor->rGetPhase () / 180.0;

		if (typeid (*(pMotor->pGetJoint())) == typeid (Juice::PrismaticJoint))
			Vector[i++] = pMotor->rGetAmplitude () / 1;
		else
			Vector[i++] = pMotor->rGetAmplitude () / 180.0;
	}

	return pVector;
}


/****************************************************************************/
/** 
*****************************************************************************/
void ClockworkBehavior::vSetVector (const std::vector<real> *pVector)
{
	int i = 0;
	ClockworkMotorList::iterator iterMotors;
	for (iterMotors = m_lsMotors.begin (); iterMotors != m_lsMotors.end (); iterMotors++)
	{
		ClockworkMotor *pMotor = *iterMotors;
		if (!pMotor)
			continue;

		// No offset for now
		i++; // pMotor->vSetStaticOffset ((*pVector)[i++] * 180.0);

		pMotor->vSetPhase ((*pVector)[i++] * 180.0);

		if (typeid (*(pMotor->pGetJoint())) == typeid (Juice::PrismaticJoint))
			pMotor->vSetAmplitude ((*pVector)[i++] * 1); 
		else
			pMotor->vSetAmplitude ((*pVector)[i++] * 180.0); 
	}

	vChanged ();
}


// ClockworkBehavior.cpp ends here ->