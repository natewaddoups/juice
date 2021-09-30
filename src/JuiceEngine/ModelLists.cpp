/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: ModelLists.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

There's a lot of code duplication below.  That probably indicates a design 
flaw somewhere.  I'm not sure how else to get the various list classes to
invoke m_pModel->pCreateWhatever(), though.  Then again, maybe the 
pCreateWhatever method is the root of the problem?
*****************************************************************************/

#include <JuiceEngine.h>

using namespace Juice;
using namespace stdext;

void BodyList::vCreateNew (Body **ppNew, bool *pfAddToList)
{
	if (!m_pModel || !ppNew || !pfAddToList)
	{
		Breakpoint ("BodyList::vCreateNew: bad parameter.");
		return;
	}

	*ppNew = m_pModel->pCreateBody ();
	*pfAddToList = false;
}

void RevoluteJointList::vCreateNew (RevoluteJoint **ppNew, bool *pfAddToList)
{
	if (!m_pModel || !ppNew || !pfAddToList)
	{
		Breakpoint ("RevoluteJointList::vCreateNew: bad parameter.");
		return;
	}

	*ppNew = m_pModel->pCreateRevoluteJoint ();
	*pfAddToList = false;
}

void PrismaticJointList::vCreateNew (PrismaticJoint **ppNew, bool *pfAddToList)
{
	if (!m_pModel || !ppNew || !pfAddToList)
	{
		Breakpoint ("PrismaticJointList::vCreateNew: bad parameter");
		return;
	}

	*ppNew = m_pModel->pCreatePrismaticJoint ();
	*pfAddToList = false;
}

void SphericalJointList::vCreateNew (SphericalJoint **ppNew, bool *pfAddToList)
{
	if (!m_pModel || !ppNew || !pfAddToList)
	{
		Breakpoint ("SphericalJointList::vCreateNew: bad parameter");
		return;
	}

	*ppNew = m_pModel->pCreateSphericalJoint ();
	*pfAddToList = false;
}

/****************************************************************************/
/** 
*****************************************************************************/

template <class Class>
void XmlSplWithTwinFixing<Class>::vCreateHashMap (unordered_map<Generic::UINT32, Class*> &Map)
{
	iterator iter, iterLast = end();
	for (iter = begin (); iter != iterLast; iter++)
	{
		Class* pObject = *iter;
		Generic::UINT32 uID = pObject->uGetID ();
		Map.insert (pair <Generic::UINT32, Class*> (uID, pObject));
	}
}

template <class Class>
void XmlSplWithTwinFixing<Class>::vSetTwinPointers (unordered_map<Generic::UINT32, Class*> &Map)
{
	iterator iter, iterLast = end();
	for (iter = begin (); iter != iterLast; iter++)
	{
		Class* pObject = *iter;
		unordered_map<Generic::UINT32, Class*>::iterator iterMap = Map.find ((Generic::UINT32) pObject->pGetTwin ());
		if (iterMap == Map.end ())
		{
			pObject->m_pTwin = null;
			continue;
		}

		pObject->m_pTwin = (*iterMap).second;
	}
}

template <class Class>
void XmlSplWithTwinAndBodyFixing<Class>::vSetBodyPointers (unordered_map<Generic::UINT32, Juice::Body*> &BodyMap)
{
	iterator iter, iterLast = end();
	for (iter = begin (); iter != iterLast; iter++)
	{
		Class* pObject = *iter;

		unordered_map<Generic::UINT32, Body*>::iterator iterBodyMap = 
			BodyMap.find ((Generic::UINT32) pObject->pGetBody1 ());

		if (iterBodyMap == BodyMap.end ())
			pObject->vSetBody1 (null);
		else
			pObject->vSetBody1 ((*iterBodyMap).second);

		iterBodyMap = BodyMap.find ((Generic::UINT32) pObject->pGetBody2 ());

		if (iterBodyMap == BodyMap.end ())
			pObject->vSetBody2 (null);
		else
			pObject->vSetBody2 ((*iterBodyMap).second);
	}
}

void BehaviorList::vResetPointers (BodyHashMap &BodyMap, JointHashMap &JointMap)
{
	iterator iter, iterLast = end();
	for (iter = begin (); iter != iterLast; iter++)
	{
		Behavior *pBehavior = *iter;
		pBehavior->vResetPointers (BodyMap, JointMap);
	}
}

void ClockworkMotorList::vSetJointPointers (unordered_map<Generic::UINT32, Joint*> &Map)
{
	iterator iter, iterLast = end();
	for (iter = begin (); iter != iterLast; iter++)
	{
		ClockworkMotor *pMotor = *iter;

		unordered_map<Generic::UINT32, Joint*>::iterator iterMap = Map.find ((Generic::UINT32) pMotor->pGetJoint ());
		if (iterMap == Map.end ())
		{
			pMotor->vSetJoint (null);
			continue;
		}

		pMotor->vSetJoint ((*iterMap).second);
	}
}

// ModelLists.cpp ends here ->