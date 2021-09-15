/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Model.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>

using namespace Juice;

Behavior* pCreateClockworkBehavior (Model *pModel, bool *pfAddToList)
{
	*pfAddToList = false;
	return pModel->pCreateClockworkBehavior (Model::bccLoadFromFile);
}

Behavior* pCreateNetworkBehavior (Model *pModel, bool *pfAddToList)
{
	*pfAddToList = false;
	return pModel->pCreateNetworkBehavior (Model::bccLoadFromFile);
}

/****************************************************************************/
/** 
*****************************************************************************/
Model::Model () :
	Selectable (null),
	XML::Serializable (L"Model"),
	m_rStandardSpeed (15),
	m_rSpeedFactor (1),
	m_rPoseDelay (1),
	m_rPosePhase (0),
	m_pDynamicsWorld (0),
	m_rPhase (0),
	m_fChanged (false),
	m_Bodies (this),
	m_RevoluteJoints (this),
	m_PrismaticJoints (this),
	m_SphericalJoints (this),
	m_Behaviors (this)
{
	m_Behaviors.vAddClass (ClockworkBehavior::ms_wszXmlTag, ::pCreateClockworkBehavior);
	m_Behaviors.vAddClass (NetworkBehavior::ms_wszXmlTag, ::pCreateNetworkBehavior);

	// for backward compatibility - this will load files from when there was just one type of behavior
	m_Behaviors.vAddClass (L"Behavior", ::pCreateClockworkBehavior);
}

/****************************************************************************/
/** 
*****************************************************************************/
Model::~Model ()
{
	vReset ();
}

/****************************************************************************/
/** 
*****************************************************************************/
const BodyList& Model::lsGetBodies ()
{
	return m_Bodies;
}

/****************************************************************************/
/** 
*****************************************************************************/
const RevoluteJointList& Model::lsGetRevoluteJoints ()
{
	return m_RevoluteJoints;
}

/****************************************************************************/
/** 
*****************************************************************************/
const PrismaticJointList& Model::lsGetPrismaticJoints ()
{
	return m_PrismaticJoints;
}

/****************************************************************************/
/** 
*****************************************************************************/
const SphericalJointList& Model::lsGetSphericalJoints ()
{
	return m_SphericalJoints;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vGetComponents (ComponentList &lsComponents)
{
	// TODO: why this thin wrapper?
	vBuildSingleList (lsComponents);
}

/****************************************************************************/
/** 
*****************************************************************************/
const BehaviorList& Model::lsGetBehaviors ()
{
	return m_Behaviors;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vSetChanged (bool fChanged)
{
	m_fChanged = fChanged;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Model::fIsChanged ()
{
	return m_fChanged;
}

/****************************************************************************/
/** 
*****************************************************************************/
real Model::rGetStandardSpeed ()
{
	return m_rStandardSpeed;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vSetStandardSpeed (real rSpeed)
{
	m_rStandardSpeed = rSpeed;
	vSetChanged ();
}

/****************************************************************************/
/** 
*****************************************************************************/
real Model::rGetPoseDelay ()
{
	return m_rPoseDelay;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vSetPoseDelay (real rDelay)
{
	m_rPoseDelay = rDelay;
	vSetChanged ();
}

/****************************************************************************/
/** 
*****************************************************************************/
real Model::rGetPosePhase ()
{
	return m_rPosePhase;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vSetPosePhase (real rPhase)
{
	m_rPosePhase = rPhase;
	vSetChanged ();
}


/****************************************************************************/
/** 
*****************************************************************************/
real Model::rGetPhase ()
{
	return m_rPhase;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vSetPhase (real rPhase)
{
	m_rPhase = rPhase;
}

/****************************************************************************/
/** 
*****************************************************************************/
real Model::rGetSpeedFactor ()
{
	return m_rSpeedFactor;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vSetSpeedFactor (real rSpeed)
{
	m_rSpeedFactor = rSpeed;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vReset ()
{
	m_uComponentCount.vReset ();

	while (m_RevoluteJoints.size ())
	{
		RevoluteJoint *pJoint = m_RevoluteJoints.front ();
		m_RevoluteJoints.pop_front ();
		delete pJoint;
	}

	while (m_PrismaticJoints.size ())
	{
		PrismaticJoint *pJoint = m_PrismaticJoints.front ();
		m_PrismaticJoints.pop_front ();
		delete pJoint;
	}

	while (m_SphericalJoints.size ())
	{
		SphericalJoint *pJoint = m_SphericalJoints.front ();
		m_SphericalJoints.pop_front ();
		delete pJoint;
	}

	// bodies must be deleted after joints to avoid crashing dynamics code (ODE.cpp:340)
	while (m_Bodies.size ())
	{
		Body *pBody = m_Bodies.front ();
		m_Bodies.pop_front ();
		delete pBody;
	}

	while (m_Behaviors.size ())
	{
		Behavior *pBehavior = m_Behaviors.front ();
		m_Behaviors.pop_front ();
		delete pBehavior;
	}

	vSetChanged (false);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vAddComponent (Component *pComponent)
{
	Generic::Listener<ComponentEvents>::vRegisterForEventsFrom (pComponent);
	Generic::Notifier<ModelEvents>::vNotify (&ModelEvents::Model_AddComponent, pComponent);

	vSetChanged ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vAddBody (Body *pBody)
{
	m_Bodies.push_back (pBody);
	vAddComponent (pBody);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vAddRevoluteJoint (RevoluteJoint *pRevoluteJoint)
{
	m_RevoluteJoints.push_back (pRevoluteJoint);
	vAddComponent (pRevoluteJoint);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vAddPrismaticJoint (PrismaticJoint *pPrismaticJoint)
{
	m_PrismaticJoints.push_back (pPrismaticJoint);
	vAddComponent (pPrismaticJoint);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vAddSphericalJoint (SphericalJoint *pSphericalJoint)
{
	m_SphericalJoints.push_back (pSphericalJoint);
	vAddComponent (pSphericalJoint);
}

/****************************************************************************/
/** 
*****************************************************************************/
Body* Model::pCreateBody ()
{
	Juice::Body* pBody = new Body (this);
	if (!pBody)
		return null;

	// Give this new body a (temporary) name
	char szName[20];
	sprintf (szName, "Beam %d", (Generic::UINT32) m_uComponentCount);
	pBody->vSetName (szName);

	// Add it to the model
	vAddBody (pBody);

	return pBody;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Model::fCreateBodyPair (Body **ppLeft, Body **ppRight)
{
	if (!ppLeft || !ppRight)
		return false;

	(*ppLeft) = pCreateBody ();
	if (!*ppLeft)
		return false;

	(*ppRight) = pCreateBody ();
	if (!*ppRight)
		return false;

	(*ppLeft)->vSetTwin (*ppRight);
	(*ppRight)->vSetTwin (*ppLeft);

	return true;
}

/****************************************************************************/
/** 
*****************************************************************************/
RevoluteJoint* Model::pCreateRevoluteJoint ()
{
	RevoluteJoint* pJoint = new RevoluteJoint (this);
	if (!pJoint)
		return null;

	// Give this new joint a (temporary) name
	char szName[20];
	sprintf (szName, "Hinge %d", (Generic::UINT32) m_uComponentCount);
	pJoint->vSetName (szName);

	// add it to the model
	vAddRevoluteJoint (pJoint);

	return pJoint;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Model::fCreateRevoluteJointPair (RevoluteJoint **ppLeft, RevoluteJoint **ppRight)
{
	if (!ppLeft || !ppRight)
		return false;

	(*ppLeft) = pCreateRevoluteJoint ();
	if (!*ppLeft)
		return false;

	(*ppRight) = pCreateRevoluteJoint ();
	if (!*ppRight)
		return false;

	(*ppLeft)->vSetTwin (*ppRight);
	(*ppRight)->vSetTwin (*ppLeft);

	return true;
}

/****************************************************************************/
/** 
*****************************************************************************/
PrismaticJoint* Model::pCreatePrismaticJoint ()
{
	PrismaticJoint* pJoint = new PrismaticJoint (this);
	if (!pJoint)
		return null;

	// Give this new joint a (temporary) name
	char szName[20];
	sprintf (szName, "Slider %d", (Generic::UINT32) m_uComponentCount);
	pJoint->vSetName (szName);

	// add it to the model
	vAddPrismaticJoint (pJoint);

	return pJoint;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Model::fCreatePrismaticJointPair (PrismaticJoint **ppLeft, PrismaticJoint **ppRight)
{
	if (!ppLeft || !ppRight)
		return false;

	(*ppLeft) = pCreatePrismaticJoint ();
	if (!*ppLeft)
		return false;

	(*ppRight) = pCreatePrismaticJoint ();
	if (!*ppRight)
		return false;

	(*ppLeft)->vSetTwin (*ppRight);
	(*ppRight)->vSetTwin (*ppLeft);

	return true;
}


/****************************************************************************/
/** 
*****************************************************************************/
SphericalJoint* Model::pCreateSphericalJoint ()
{
	SphericalJoint* pJoint = new SphericalJoint (this);
	if (!pJoint)
		return null;

	// Give this new joint a (temporary) name
	char szName[20];
	sprintf (szName, "Ball %d", (Generic::UINT32) m_uComponentCount);
	pJoint->vSetName (szName);

	// add it to the model
	vAddSphericalJoint (pJoint);

	return pJoint;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Model::fCreateSphericalJointPair (SphericalJoint **ppLeft, SphericalJoint **ppRight)
{
	if (!ppLeft || !ppRight)
		return false;

	(*ppLeft) = pCreateSphericalJoint ();
	if (!*ppLeft)
		return false;

	(*ppRight) = pCreateSphericalJoint ();
	if (!*ppRight)
		return false;

	(*ppLeft)->vSetTwin (*ppRight);
	(*ppRight)->vSetTwin (*ppLeft);

	return true;
}

/****************************************************************************/
/** Add new behavior to the model's list, tell any 
*** listeners (e.g. the UI) that a new behavior has been created, and register
*** for a notification event when the behavior is deleted.
*****************************************************************************/
void Model::vAddBehavior (Behavior *pBehavior)
{
	m_Behaviors.push_back (pBehavior);

	Listener<BehaviorEvents>::vRegisterForEventsFrom (pBehavior);

	Notifier<ModelEvents>::vNotify (&ModelEvents::Model_AddBehavior, pBehavior);

	vSetChanged ();
}

/****************************************************************************/
/** This is the clockwork behavior creator.  At design time, the behavior must
*** be populated with a list of motors.  When loaded from a file, the motors
*** will be created as they are loaded.
*****************************************************************************/
ClockworkBehavior* Model::pCreateClockworkBehavior (BehaviorCreationContext bcc)
{
	ClockworkBehavior *pNew = new ClockworkBehavior (this);

	if (!pNew)
		return null;

	if (bccDesignTime == bcc)
		pNew->vInitializeMotorList ();

	char szName[50];
	sprintf (szName, "Clockwork %d", (Generic::UINT32) m_uComponentCount);
	pNew->vSetName (szName);

	vAddBehavior (pNew);

	return pNew;
}

/****************************************************************************/
/** This is the network behavior creator.  At design time, the behavior must
*** be populated with a list of motors.  When loaded from a file, the motors
*** will be created as they are loaded.
*****************************************************************************/
NetworkBehavior* Model::pCreateNetworkBehavior (BehaviorCreationContext bcc)
{
	NetworkBehavior *pNew = new NetworkBehavior (this);

	if (!pNew)
		return null;

	char szName[50];
	sprintf (szName, "Network %d", (Generic::UINT32) m_uComponentCount);
	pNew->vSetName (szName);

	vAddBehavior (pNew);

	return pNew;
}

/****************************************************************************/
/** 
*****************************************************************************/
Generic::UINT32 Model::uGetVisibleComponentCount ()
{
	return m_Bodies.size () + m_RevoluteJoints.size () + m_PrismaticJoints.size ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vGetStaticBounds (Generic::VectorXYZ &vecMin, Generic::VectorXYZ &vecMax)
{
	vecMax.x = vecMax.y = vecMax.z = 0;
	vecMin.x = vecMin.y = vecMin.z = 0;

	std::list<Juice::Body*>::iterator iterBodies;
	for (iterBodies = m_Bodies.begin (); iterBodies != m_Bodies.end (); iterBodies++)
	{
		Body *pBody = (*iterBodies);
		VectorXYZ vecBodyMax, vecBodyMin;

		pBody->vGetStaticBounds (vecBodyMin, vecBodyMax);

		vecMax.x = max (vecMax.x, vecBodyMax.x);
		vecMax.y = max (vecMax.y, vecBodyMax.y);
		vecMax.z = max (vecMax.z, vecBodyMax.z);

		vecMin.x = min (vecMin.x, vecBodyMin.x);
		vecMin.y = min (vecMin.y, vecBodyMin.y);
		vecMin.z = min (vecMin.z, vecBodyMin.z);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vBuildSingleList (ComponentList &lsAllComponents)
{
	std::list<Juice::Body*>::iterator iterBodies;
	for (iterBodies = m_Bodies.begin (); iterBodies != m_Bodies.end (); iterBodies++)
	{
		Body *pBody = (*iterBodies);
		lsAllComponents.push_back (pBody);
	}

	std::list<Juice::RevoluteJoint*>::iterator iterRevoluteJoints;
	for (iterRevoluteJoints = m_RevoluteJoints.begin (); iterRevoluteJoints != m_RevoluteJoints.end (); iterRevoluteJoints++)
	{
		RevoluteJoint *pRevoluteJoint = (*iterRevoluteJoints);
		lsAllComponents.push_back (pRevoluteJoint);
	}

	std::list<Juice::PrismaticJoint*>::iterator iterPrismaticJoints;
	for (iterPrismaticJoints = m_PrismaticJoints.begin (); iterPrismaticJoints != m_PrismaticJoints.end (); iterPrismaticJoints++)
	{
		PrismaticJoint *pPrismaticJoint = (*iterPrismaticJoints);
		lsAllComponents.push_back (pPrismaticJoint);
	}

	std::list<Juice::SphericalJoint*>::iterator iterSphericalJoints;
	for (iterSphericalJoints = m_SphericalJoints.begin (); iterSphericalJoints != m_SphericalJoints.end (); iterSphericalJoints++)
	{
		SphericalJoint *pSphericalJoint = (*iterSphericalJoints);
		lsAllComponents.push_back (pSphericalJoint);
	}
}

void Model::vGetCenter (VectorXYZ &vecCenter)
{
	vecCenter.x = vecCenter.y = vecCenter.z = 0;
	//std::list<Component*> lsAllComponents;
	ComponentList lsAllComponents;
	
	vBuildSingleList (lsAllComponents);

	lsAllComponents.vGetCenter (vecCenter);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::Notifier_Deleted(Component *pDeleted)
{
	if (!pDeleted)
		return;

	// this will be ComponentEvents typeinfo (why?)
	const type_info &typeDeleted = typeid (*pDeleted);

	// these all end up null
	Body *pBody = dynamic_cast <Body*> (pDeleted);
	RevoluteJoint *pRJ = dynamic_cast <RevoluteJoint*> (pDeleted);
	PrismaticJoint *pPJ = dynamic_cast <PrismaticJoint*> (pDeleted);
	SphericalJoint *pSJ = dynamic_cast <SphericalJoint*> (pDeleted);

	// Apparently, the only way to get this to work is to just try each list.

	// This code was actually a lot more elegant BEFORE the transition from
	// the eGetType() method to the use of RTTI.

	//if (pBody)
		m_Bodies.remove ((Body*) pDeleted);
	
	//else if (pRJ)
		m_RevoluteJoints.remove ((RevoluteJoint*) pDeleted);
	
	//else if (pPJ)
		m_PrismaticJoints.remove ((PrismaticJoint*) pDeleted);
	
	//else if (pSJ)
		m_SphericalJoints.remove ((SphericalJoint*) pDeleted);

	vSetChanged ();
}

void Model::Notifier_Deleted(Behavior *pBehavior)
{
	m_Behaviors.remove (pBehavior);

	vSetChanged ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vSelectAll ()
{
	ComponentList lsEverything;
	vBuildSingleList (lsEverything);
	ComponentList::iterator iter;

	for (iter = lsEverything.begin (); iter != lsEverything.end (); iter++)
		m_pApplication->m_SelectionSet.vAdd (*iter);
}

/****************************************************************************/
/** Paste the given object into the model.
 ** Note that the vAddXxxx functions called here all call vSetChanged.
*****************************************************************************/
bool Model::fPaste (Selectable *pNewObject)
{
	if (IsKindOfPtr <Component*> (pNewObject))
	{
		Component *pComponent = dynamic_cast <Component*> (pNewObject);
        
		// one day perhaps it will be possible to paste components from one model to another
		pComponent->m_pModel = this;

		// add "Copy Of" to the name
		std::string strName = "Copy of ";
		strName += pComponent->szGetName ();
		pComponent->vSetName (strName.c_str ());

		// this will help discover the type
		Body *pBody = dynamic_cast <Body*> (pComponent);
		RevoluteJoint *pRevoluteJoint = dynamic_cast <RevoluteJoint*> (pComponent);
		PrismaticJoint *pPrismaticJoint = dynamic_cast <PrismaticJoint*> (pComponent);
		SphericalJoint *pSphericalJoint = dynamic_cast <SphericalJoint*> (pComponent);

		// add the new object
		if (pBody)
			vAddBody (pBody);			

		if (pRevoluteJoint)
			vAddRevoluteJoint (pRevoluteJoint);

		if (pPrismaticJoint)
			vAddPrismaticJoint (pPrismaticJoint);

		if (pSphericalJoint)
			vAddSphericalJoint (pSphericalJoint);

		return true;
	}
    
	bool fResult = false;

	if (IsKindOfPtr <Behavior*> (pNewObject))
	{
		// paste atop any currently selected behaviors of the exact same type
		const SelectableList &lsSelected = m_pApplication->m_SelectionSet.lsGetContents ();

		bool fPastedAtop = false;

		SelectableList::const_iterator iterInner, iterLastInner = lsSelected .end ();
		for (iterInner = lsSelected.begin (); iterInner != iterLastInner; iterInner++)
		{
			Selectable *pSelected = *iterInner;
			const type_info &ti1 = typeid (*pNewObject);
			const type_info &ti2 = typeid (*pSelected);

			if (typeid (*pSelected) == typeid (*pNewObject))
			{
				pSelected->fPaste (pNewObject);
				fPastedAtop = true;

				vSetChanged ();

				fResult = true;
			}
		}

		// If there were none, then add the behavior
		if (!fPastedAtop)
		{
			vAddBehavior (dynamic_cast <Behavior*> (pNewObject));

			fResult = true;
		}
	}	

	return fResult;
}


/****************************************************************************/
/** 
*****************************************************************************/
void ComponentList::vGetCenter (Generic::VectorXYZ &vecCenter) const
{
	vecCenter.x = vecCenter.y = vecCenter.z = 0;

	if (size () == 0)
		return;

	int iItems = 0;

	ComponentList::const_iterator iter;
	for (iter = begin (); iter != end (); iter++)
	{
		Component *pComponent = (*iter);
		if (!pComponent)
		{
			Breakpoint ("ComponentList::vGetCenter: null pComponent.");
			continue;
		}

		VectorXYZ vecPosition;
		pComponent->vGetPosition (vecPosition);

		vecCenter.x += vecPosition.x;
		vecCenter.y += vecPosition.y;
		vecCenter.z += vecPosition.z;

		iItems ++;
	}

	vecCenter.x /= size ();
	vecCenter.y /= size ();
	vecCenter.z /= size ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::Component_Changed (Component *pComponent)
{
	vSetChanged ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::Behavior_Changed (Component *pComponent)
{
	vSetChanged ();
}

// Model.cpp ends here ->