/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: SelectionSet.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>

using namespace Juice;

Selectable::Selectable (Selectable *pParent) : m_pParent (pParent), m_uSelectionCount (0)
{
}

Selectable::~Selectable ()
{
}

bool Selectable::fSelected ()
{
	return !(!m_uSelectionCount);
}

void Selectable::vSelect ()
{
	m_uSelectionCount++;
}

void Selectable::vDeselect ()
{
	if (m_uSelectionCount > 0)
		m_uSelectionCount--;
	else; // Breakpoint ();
}

/****************************************************************************/
/** 
*****************************************************************************/
SelectionSet::SelectionSet ()
{
}

/****************************************************************************/
/** 
*****************************************************************************/
SelectionSet::~SelectionSet ()
{
	vReset ();
}

/****************************************************************************/
/** 
*****************************************************************************/

Generic::UINT32 SelectionSet::uObjectsOfType (const type_info &tiQuery)
{
	Generic::UINT32 uResult = 0;

	SelectableList::iterator iter, iterLast = m_Contents.end ();
	for (iter = m_Contents.begin (); iter != iterLast; iter++)
	{
		const type_info &tiThisItem = typeid (*iter);
		if (tiThisItem == tiQuery)
			uResult++;
	}
	return uResult;
}

/****************************************************************************/
/** 
*****************************************************************************/

Generic::UINT32 SelectionSet::uComponents ()
{
	Generic::UINT32 uResult = 0;

	SelectableList::iterator iter, iterLast = m_Contents.end ();
	for (iter = m_Contents.begin (); iter != iterLast; iter++)
	{
		const type_info &tiThisItem = typeid (*iter);
		if (IsKindOf<Component*> (*iter))
			uResult++;
	}
	return uResult;

//	return uObjectsOfType (typeid (Juice::Component)) +
//		uObjectsOfType (typeid (Juice::Joint)) +
//		uObjectsOfType (typeid (Juice::Body)) +
//		uObjectsOfType (typeid (Juice::RevoluteJoint)) +
//		uObjectsOfType (typeid (Juice::PrismaticJoint));
}

Generic::UINT32 SelectionSet::uBehaviors ()
{
	return uObjectsOfType (typeid (Juice::Behavior));
}

Generic::UINT32 SelectionSet::uMotors ()
{
	return uObjectsOfType (typeid (Juice::ClockworkMotor));
}

/****************************************************************************/
/** 
*****************************************************************************/
Generic::UINT32 SelectionSet::uItems ()
{
	return m_Contents.size ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void SelectionSet::vSetClipboard (Clipboard *pClipboard)
{
	m_pClipboard = pClipboard;
}

/****************************************************************************/
/** 
*****************************************************************************/
void SelectionSet::vReset ()
{
	// empty the list
	Selectable *pSelectable = null;

	while (m_Contents.size ())
	{
		pSelectable = m_Contents.front (); 
		pSelectable->vDeselect ();
		vNotify (&SelectionSetEvents::Selection_Remove, pSelectable);
		m_Contents.pop_front ();
	}
}

/****************************************************************************/
/** Add the given object to the selection set.  It is essential that this be
 ** done with a push_back, so the selection set keeps the order established
 ** by Clipboard::vCopy.
*****************************************************************************/
void SelectionSet::vAddAndNotify (Selectable *pSelectable, bool fAutomatic)
{
	if (!pSelectable)
	{
		Breakpoint ("SelectionSet::vAddAndNotify: pSelectable is null.");
		return;
	}

	SelectableList::iterator iter, iterLast = m_Contents.end ();
	for (iter = m_Contents.begin (); iter != iterLast; iter++)
	{
		// Components must not be selected more than once
		if ((*iter) == pSelectable)
			return;
	}

	Generic::Listener<SelectableEvents>::vRegisterForEventsFrom (pSelectable);

	m_Contents.push_back (pSelectable);

	pSelectable->vSelect ();

	vNotify (&SelectionSetEvents::Selection_Add, pSelectable, fAutomatic);
}

void SelectionSet::vAdd (Selectable *pSelectable)
{
	if (!pSelectable)
		return;

	try
	{
		// TODO: quick little hack to help the UI
		// if there's only one object selected, and it's a joint, select the joint's motor
		// in the most recently selected behavior.
		//
		// hmmm.  good idea or bad?

		// Add the given object
	vAddAndNotify (pSelectable, false);

		if (m_pApplication && m_pApplication->m_eEditMode == emSymmetric)
		{
			if (IsKindOf <Component*> (pSelectable))
			{
				Component *pSelectedComponent = dynamic_cast <Component*> (pSelectable);
				Component *pTwin = pSelectedComponent->pGetTwin ();
				if (pTwin)
					vAddAndNotify (pTwin, true);
			}

			if (IsKindOf <ClockworkMotor*> (pSelectable))
			{
				ClockworkMotor *pSelectedMotor = dynamic_cast <ClockworkMotor*> (pSelectable);
				ClockworkMotor *pTwin = pSelectedMotor->pGetTwin ();
				if (pTwin)
					vAddAndNotify (pTwin, true);
			}
		}
	}
	catch (bad_cast e)
	{
		Breakpoint ("SelectionSet::vAdd: bad cast exception.");
	}
	catch (...)
	{
		Breakpoint ("SelectionSet::vAdd: exception.");
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void SelectionSet::vRemove (Selectable *pSelectable)
{
	if (!pSelectable)
	{
		Breakpoint ("SelectionSet::vRemove: null pSelectable.");
		return;
	}

	SelectableList::iterator iter, iterLast = m_Contents.end ();
	for (iter = m_Contents.begin (); iter != iterLast; iter++)
	{
		// Make sure the component is really in the list before removing it
		// and notifying the world of this event.
		if ((*iter) == pSelectable)
		{
			m_Contents.remove (pSelectable);
			pSelectable->vDeselect ();
			vNotify (&SelectionSetEvents::Selection_Remove, pSelectable);
			return;
		}
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void SelectionSet::Notifier_Deleted (Selectable *pSelectable)
{
	m_Contents.remove (pSelectable);
}

/****************************************************************************/
/** If this is the application's main selection set, motors should not be
*** deleted this way.  Thus fDeleteMotorsToo defaults to false.  The clipboard
*** is entited to delete its contents however, so it may call with a true
*** fDeleteMotorsToo.
*****************************************************************************/
void SelectionSet::vDelete (bool fDeleteMotorsToo)
{
	Selectable *pSelectable = null;

	SelectableList lsMotors;

	while (m_Contents.size ())
	{
		pSelectable = m_Contents.front (); 

		// Don't delete motors!  That would hose the one-to-one motor:joint relationship
		if (!fDeleteMotorsToo)
		{
			if (IsKindOfPtr <ClockworkMotor*> (pSelectable))
			{
				// stash the motors in a temporary local list to ensure
				// that m_Contents.size does eventually reach zero
				lsMotors.push_front (pSelectable);
				m_Contents.pop_front ();
				continue;
			}			
		}

		pSelectable->vDeselect ();
		vNotify (&SelectionSetEvents::Selection_Remove, pSelectable);
		m_Contents.pop_front ();
		delete pSelectable;
	}

	// Put any remaining motors back into the main list
	m_Contents = lsMotors;
}

/****************************************************************************/
/** 
*****************************************************************************/
SelectableList& SelectionSet::lsGetContents ()
{
	return m_Contents;
}

// SelectionSet.cpp ends here ->