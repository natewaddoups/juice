/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Clipboard.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>

using namespace Juice;

/****************************************************************************/
/** Constructor - nothing too interesting
*****************************************************************************/
Clipboard::Clipboard ()
{
}

/****************************************************************************/
/** The destructor just calls vDeleteContacts to clean up any objects that 
 ** might still be on the clipboard.
*****************************************************************************/
Clipboard::~Clipboard ()
{
	vDeleteContents ();
}

/****************************************************************************/
/** This is primarily for benefit of the user interface, e.g. to enble or
 ** disable items on the Edit menu.
*****************************************************************************/
bool Clipboard::fHasData ()
{
	return (!(!(m_Contents.uItems ())));
}

/****************************************************************************/
/** Free any objects on the clipboard
*****************************************************************************/
void Clipboard::vDeleteContents ()
{
	m_Contents.vDelete (true);
}

/****************************************************************************/
/** Copy objects from the given selection set, making duplicates of them.
*****************************************************************************/
void Clipboard::vCopy (SelectionSet *pSource)
{
	// Delete all items in the clipboard contents
	vDeleteContents ();

	vCopy (pSource, &m_Contents);
}

/****************************************************************************/
/** Copy objects from one selection set to another.  Different classes get
 ** handled slightly differently, and pointer relationships between objects
 ** are preserved among objects in the destination selection set. 
*****************************************************************************/
void Clipboard::vCopy (SelectionSet *pSource, SelectionSet *pDestination)
{
	pDestination->vReset ();

	Juice::EditMode eOldEditMode = m_pApplication->m_eEditMode;
	m_pApplication->m_eEditMode = Juice::emNormal;

	// duplicate objects
	// the objects are initially kept in separate lists to make it a little
	// bit easier to preserve the relationships between them - see below...
	BodyList lsBodies (null);
	JointList lsJoints;
	BehaviorList lsBehaviors (null);
	ClockworkMotorList lsMotors (null);
	NodeList lsNodes;
	EdgeList lsEdges (null);

	SelectableList::const_iterator iterSelection;

	// duplicate components
	for (iterSelection = pSource->lsGetContents().begin (); iterSelection != pSource->lsGetContents().end (); iterSelection++)
	{
		Selectable *pObject = (*iterSelection);

		const type_info &ti = typeid (*pObject);

		if (typeid (*pObject) == typeid (Body))
		{
			Body *pBody = dynamic_cast <Body*> (pObject->pCreateClipboardCopy ());
			lsBodies.push_back (pBody);
			continue;
		}

		Joint *pJoint = dynamic_cast<Joint*> (pObject);
		if (pJoint)
		{
			Joint *pNewJoint = (Joint*) pJoint->pCreateClipboardCopy ();
			lsJoints.push_back (pNewJoint);
			continue;
		}

		Behavior *pBehavior = dynamic_cast <Behavior*> (pObject);
		if (pBehavior)
		{
			Behavior *pNewBehavior = (Behavior*) pBehavior->pCreateClipboardCopy ();
			lsBehaviors.push_back (pNewBehavior);
			continue;
		}

		if (IsKindOf <ClockworkMotor*> (pObject))
		{
			ClockworkMotor *pMotor = dynamic_cast <ClockworkMotor*> (pObject->pCreateClipboardCopy ());
			lsMotors.push_back (pMotor);
			continue;
		}

		if (IsKindOf <Node*> (pObject))
		{
			Node *pNode = dynamic_cast <Node*> (pObject->pCreateClipboardCopy ());
			lsNodes.push_back (pNode);
			continue;
		}

		if (IsKindOf <Edge*> (pObject))
		{
			Edge *pEdge = dynamic_cast <Edge*> (pObject->pCreateClipboardCopy ());
			lsEdges.push_back (pEdge);
			continue;
		}
	}

	// Create hash maps of body and joint and node IDs and pointers
	BodyHashMap BodyMap;
	lsBodies.vCreateHashMap (BodyMap);

	JointHashMap JointMap;
	lsJoints.vCreateHashMap (JointMap);

	NodeHashMap NodeMap;
	lsNodes.vCreateHashMap (NodeMap);

    // fix pointers among clipboard objects
	lsBodies.vSetTwinPointers (BodyMap);
	lsJoints.vSetTwinPointers (JointMap);
	lsJoints.vSetBodyPointers (BodyMap);

	// There is an exception in this case... nodes should still point to the 
	// bodies and joints in the original set of objects.
	//
	// lsNodes.vSetPointers (BodyMap, JointMap); 

	lsEdges.vSetNodePointers (NodeMap);
    
	// Restore the edit mode
	m_pApplication->m_eEditMode = eOldEditMode;
	
	// add the lists to the destination set
	for (BodyList::iterator iterBody = lsBodies.begin (); iterBody != lsBodies.end (); iterBody++)
		pDestination->vAdd (*iterBody);

	for (JointList::iterator iterJoint = lsJoints.begin (); iterJoint != lsJoints.end (); iterJoint++)
		pDestination->vAdd (*iterJoint);

	for (BehaviorList::iterator iterBehaviors = lsBehaviors.begin (); iterBehaviors != lsBehaviors.end (); iterBehaviors++)
		pDestination->vAdd (*iterBehaviors);

	for (ClockworkMotorList::iterator iterMotors = lsMotors.begin (); iterMotors != lsMotors.end (); iterMotors++)
        pDestination->vAdd (*iterMotors);

	for (NodeList::iterator iterNodes = lsNodes.begin (); iterNodes != lsNodes.end (); iterNodes++)
        pDestination->vAdd (*iterNodes);

	for (EdgeList::iterator iterEdges = lsEdges.begin (); iterEdges != lsEdges.end (); iterEdges++)
        pDestination->vAdd (*iterEdges);
}

// Clipboard.cpp ends here ->