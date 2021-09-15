/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: ModelTreeView.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "ModelTreeView.h"


// CModelTreeView

IMPLEMENT_DYNCREATE(CModelTreeView, CView)
IMPLEMENT_DYNCREATE(CBehaviorTreeView, CView)

/****************************************************************************/
/** 
*****************************************************************************/
CModelTreeView::CModelTreeView() :
	m_pModel (null),
	m_htiBodies (null),
	m_htiRevoluteJoints (null), 
	m_htiPrismaticJoints (null),
	m_htiSphericalJoints (null),
	m_htiBehaviors (null)
{
	Generic::Listener<SelectionSetEvents>::vRegisterForEventsFrom (&theApp.m_Juice.m_SelectionSet);
}

/****************************************************************************/
/** 
*****************************************************************************/
CModelTreeView::~CModelTreeView()
{
}

/****************************************************************************/
/** 
*****************************************************************************/

#ifdef _DEBUG
void CModelTreeView::AssertValid() const
{
	CView::AssertValid();
}

void CModelTreeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vRepaintTree ()
{
	m_Tree.InvalidateRect (NULL);
}

/****************************************************************************/
/** 
*****************************************************************************/
bool CModelTreeView::fGetCheck (HTREEITEM hItem)
{
	UINT uState = m_Tree.GetItemState (hItem, TVIF_SELECTEDIMAGE);
	uState >>= 12;
	if (uState == 2)
		return true;
	return false;
}

void CModelTreeView::vSetCheck (HTREEITEM hItem, bool fCheck)
{
	m_Tree.SetItemState (hItem, INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1)), TVIS_STATEIMAGEMASK);
	vRepaintTree ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vFindMatchingItems (void *pItemContents, fnTreeItemAction fn, void *pFnData)
{
	HTREEITEM hItem = m_Tree.GetChildItem (NULL);

	while (hItem)
	{
		vRecursiveFindMatchingItems (hItem, pItemContents, fn, pFnData);
		hItem = m_Tree.GetNextItem (hItem, TVGN_NEXT);
	}

	vRepaintTree ();
}

/// This is depth-first to allow the app to delete ALL items when shutting down
void CModelTreeView::vRecursiveFindMatchingItems (HTREEITEM hItem, void *pItemContents, fnTreeItemAction fn, void *pFnData)
{
	// check all of this item's children
	HTREEITEM hChildItem = m_Tree.GetChildItem (hItem);
	while (hChildItem)
	{
		vRecursiveFindMatchingItems (hChildItem, pItemContents, fn, pFnData);
		hChildItem = m_Tree.GetNextItem (hChildItem, TVGN_NEXT);
	} 

	// Don't check children if this item just got deleted
	TVITEM item;
	item.hItem = hItem;
	item.mask = TVIF_HANDLE;

	if (!m_Tree.GetItem (&item))
		return;

	// check this item
	TreeItemData *pData = (TreeItemData*) m_Tree.GetItemData (hItem);
	if (pData)
	{
		(this->*fn) (hItem, pItemContents, pFnData);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vDelete (HTREEITEM hItem, void *pItemContents, void *)
{
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);

	if (pItemData && (pItemData->pWhatever() == pItemContents))
	{
		// item data deletion will happen via TVN_DELETEITEM
//		delete pItemData;
		m_Tree.DeleteItem (hItem);
	}
}

/****************************************************************************/
/** Deletes EVERYTHING it finds - for use only when shutting down
*****************************************************************************/
void CModelTreeView::vDeleteEverything (HTREEITEM hItem, void *pItemContents, void* pData)
{
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);

	if (pItemData)
	{
		// item data deletion will happen via TVN_DELETEITEM
//		delete pItemData;
		m_Tree.DeleteItem (hItem);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vComponentChange (HTREEITEM hItem, void *pItemContents, void *)
{
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);

	if (pItemData && (pItemData->pWhatever() == pItemContents))
	{
		Component *pComponent = pItemData->pComponent ();

		if (pComponent)
		{
			const char *szName = pComponent->szGetName ();
			m_Tree.SetItemText (hItem, szName);
		}
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vBehaviorChange (HTREEITEM hItem, void *pItemContents, void *)
{
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);

	if (pItemData && (pItemData->pWhatever() == pItemContents))
	{
		Behavior *pBehavior = pItemData->pBehavior ();

		if (pBehavior)
		{
			const char *szName = pBehavior->szGetName();
			m_Tree.SetItemText (hItem, szName);

			bool fEnabled = pBehavior->fGetEnableState ();
			vSetCheck (hItem, fEnabled);
		}
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vMotorChange (HTREEITEM hItem, void *pItemContents, void *)
{
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);

	if (pItemData && (pItemData->pWhatever() == pItemContents))
	{
		ClockworkMotor *pMotor = pItemData->pMotor ();

		if (pMotor)
		{
			const char *szName = pMotor->szGetName();
			m_Tree.SetItemText (hItem, szName);

			bool fEnabled = pMotor->fGetEnableState ();
			vSetCheck (hItem, fEnabled);
		}
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vSelect (HTREEITEM hItem, void *pItemContents, void* p)
{
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);

	if (pItemData && (pItemData->pSelectable () == pItemContents))
	{
		UINT uState = 0;
		if (p)
			uState = TVIS_BOLD;

		m_Tree.SetItem(hItem, TVIF_STATE, NULL, 0, 0, uState, TVIS_BOLD, 0);
	}
}


void CModelTreeView::vComponentSelect (HTREEITEM hItem, void *pItemContents, void* p)
{
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);

	if (pItemData && (pItemData->pWhatever() == pItemContents))
	{
		Component *pComponent = pItemData->pComponent ();

		UINT uState = 0;
		if (p)
			uState = TVIS_BOLD;

		if (pComponent)
			m_Tree.SetItem(hItem, TVIF_STATE, NULL, 0, 0, uState, TVIS_BOLD, 0);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vBehaviorSelect (HTREEITEM hItem, void *pItemContents, void* p)
{
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);

	if (pItemData && (pItemData->pWhatever() == pItemContents))
	{
		Behavior *pBehavior = pItemData->pBehavior ();

		UINT uState = 0;
		if (p)
			uState = TVIS_BOLD;

		if (pBehavior)
			m_Tree.SetItem(hItem, TVIF_STATE, NULL, 0, 0, uState, TVIS_BOLD, 0);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vMotorSelect (HTREEITEM hItem, void *pItemContents, void* p)
{
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);

	if (pItemData && (pItemData->pWhatever() == pItemContents))
	{
		ClockworkMotor *pMotor = pItemData->pMotor ();

		UINT uState = 0;
		if (p)
			uState = TVIS_BOLD;

		if (pMotor)
			m_Tree.SetItem(hItem, TVIF_STATE, NULL, 0, 0, uState, TVIS_BOLD, 0);
	}
}



/****************************************************************************/
/** When using this, remember that this only returns the last hit of a depth-
*** first search.
*****************************************************************************/
void CModelTreeView::vGetItem (HTREEITEM hItem, void *pItemContents, void *pDesiredItem)
{
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);
	if (pItemData && (pItemData->pWhatever() == pItemContents))
	{
		*((HTREEITEM*)pDesiredItem) = hItem;
	}
}

/****************************************************************************/
/** This is apparently never called?
*****************************************************************************/
// TODO: get rid of
void CModelTreeView::vSetModel (Juice::Model *pModel)
{
	// store the model pointer
	m_pModel = pModel; 

	// add all of the model's components to the tree

/*	this strategy fails because the component list is populated from the 
    DLL's heap, but then destroyed and deallocated from the EXE's CRT.
	
	ComponentList lsComponents;
	m_pModel->vGetComponents (lsComponents);
	ComponentList::iterator iterComponents;

	for (iterComponents = lsComponents.begin (); iterComponents != lsComponents.end (); iterComponents++)
	{
		Component *pComponent = const_cast<Component*> (*iterComponents);
		Model_AddComponent (m_pModel, pComponent);
	}
*/
	const BodyList &lsBodies = m_pModel->lsGetBodies ();
	BodyList::const_iterator iterBodies;

	for (iterBodies = lsBodies.begin (); iterBodies != lsBodies.end (); iterBodies++)
	{
		Body *pBody = const_cast<Body*> (*iterBodies);
		Model_AddComponent (m_pModel, pBody);
	}

	const RevoluteJointList &lsRevoluteJoints = m_pModel->lsGetRevoluteJoints ();
	RevoluteJointList::const_iterator iterRevoluteJoints;

	for (iterRevoluteJoints = lsRevoluteJoints.begin (); iterRevoluteJoints != lsRevoluteJoints.end (); iterRevoluteJoints++)
	{
		RevoluteJoint *pRevoluteJoint = const_cast<RevoluteJoint*> (*iterRevoluteJoints);
		Model_AddComponent (m_pModel, pRevoluteJoint);
	}

	const PrismaticJointList &lsPrismaticJoints = m_pModel->lsGetPrismaticJoints ();
	PrismaticJointList::const_iterator iterPrismaticJoints;

	for (iterPrismaticJoints = lsPrismaticJoints.begin (); iterPrismaticJoints != lsPrismaticJoints.end (); iterPrismaticJoints++)
	{
		PrismaticJoint *pPrismaticJoint = const_cast<PrismaticJoint*> (*iterPrismaticJoints);
		Model_AddComponent (m_pModel, pPrismaticJoint);
	}

	const SphericalJointList &lsSphericalJoints = m_pModel->lsGetSphericalJoints ();
	SphericalJointList::const_iterator iterSphericalJoints;

	for (iterSphericalJoints = lsSphericalJoints.begin (); iterSphericalJoints != lsSphericalJoints.end (); iterSphericalJoints++)
	{
		SphericalJoint *pSphericalJoint = const_cast<SphericalJoint*> (*iterSphericalJoints);
		Model_AddComponent (m_pModel, pSphericalJoint);
	}

	// add all of the model's behaviors and motions to the tree
	const BehaviorList &lsBehaviors = m_pModel->lsGetBehaviors ();
	BehaviorList::const_iterator iterBehaviors;

	for (iterBehaviors = lsBehaviors.begin (); iterBehaviors != lsBehaviors.end (); iterBehaviors++)
	{
		Behavior *pBehavior = const_cast <Behavior*> (*iterBehaviors);

		// passing null here is a bit of a hack - see notes in Model_AddBehavior
		Model_AddBehavior (null, pBehavior);

		ClockworkBehavior *pClockworkBehavior = dynamic_cast <ClockworkBehavior*> (pBehavior);
		if (pClockworkBehavior)
		{
			const ClockworkMotorList &lsMotions = pClockworkBehavior->lsGetMotors ();
			ClockworkMotorList::const_iterator iterMotions;

			for (iterMotions = lsMotions.begin (); iterMotions != lsMotions.end (); iterMotions++)
			{
				ClockworkMotor *pMotion = const_cast<ClockworkMotor*> (*iterMotions);
				ClockworkBehavior_AddMotor (pClockworkBehavior, pMotion);
			}
		}
	}

	// register for notification of future additions to / deletions from the model
	Generic::Listener<ModelEvents>::vRegisterForEventsFrom (pModel);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vHideCheckBox (HTREEITEM hItem)
{
	TVITEM item;

	item.mask = TVIF_HANDLE | TVIF_STATE;
    item.hItem = hItem;
    item.stateMask = -1; // TVIS_STATEIMAGEMASK;

    // Image 1 in the tree-view check box image list is the unchecked box. Image 2 is the checked box.
    item.state = 0; // INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));

    m_Tree.SetItem (&item);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::Model_AddComponent (Juice::Model *pModel, Juice::Component *pComponent)
{
	if (!pComponent || (pModel != m_pModel))
	{
		Breakpoint ("CModelTreeView::Model_AddComponent: null parameter.");
		return;
	}

	HTREEITEM hParent = NULL;

	if (typeid (*pComponent) == typeid (Body))
	{
		hParent = m_htiBodies;
	}
	else if (typeid (*pComponent) == typeid (RevoluteJoint))
	{
		hParent = m_htiRevoluteJoints;
	}
	else if (typeid (*pComponent) == typeid (PrismaticJoint))
	{
		hParent = m_htiPrismaticJoints;
	}
	else if (typeid (*pComponent) == typeid (SphericalJoint))
	{
		hParent = m_htiSphericalJoints;
	}
	else
	{
		return;
	}

	HTREEITEM hNew = m_Tree.InsertItem (pComponent->szGetName (), hParent);

	if (hNew)
	{
		vHideCheckBox (hNew);
		m_Tree.SetItemData (hNew, (DWORD) new TreeItemData (pComponent));
		Generic::Listener<ComponentEvents>::vRegisterForEventsFrom (pComponent);
		vRepaintTree ();
	}
	else
	{
		Breakpoint ("CModelTreeView::Model_AddComponent: null hItem.");
	}
}

/****************************************************************************/
/** This function is called in two contexts.  The usual context is when a new
behavior has been added to the model.  In that case "pBehavior->
vNotifyOfMotors()" causes items to be added to the tree for each motor in the
behavior.  In that context, the pModel parameter is a valid pointer, because
of the way the publish/subscribe pattern works.

This function is also in the code path for creating a new motion window; it's
called from CModelTreeView::vSetModel().  In that context, pModel is null, and
the vNotifyOfMotors() call gets skipped.

The other option would be to cut/paste most of this code into vSetModel, but
I'd rather have a documented hack than duplicate code.  
*****************************************************************************/
void CModelTreeView::Model_AddBehavior (Model *pModel, Behavior *pBehavior)
{
	if (!pBehavior)
		return;

	HTREEITEM hNew = m_Tree.InsertItem (pBehavior->szGetName (), m_htiBehaviors);

	if (hNew)
	{
		m_Tree.SetItemData (hNew, (DWORD) new TreeItemData (pBehavior));

		bool fEnabled = pBehavior->fGetEnableState ();
		m_Tree.SetItemState (hNew, INDEXTOSTATEIMAGEMASK((fEnabled ? 2 : 1)), TVIS_STATEIMAGEMASK);

		Generic::Listener<BehaviorEvents>::vRegisterForEventsFrom (pBehavior);

		ClockworkBehavior *pClockworkBehavior = dynamic_cast <ClockworkBehavior*> (pBehavior);
		if (pClockworkBehavior)
			Generic::Listener<ClockworkBehaviorEvents>::vRegisterForEventsFrom (pClockworkBehavior);

		// See notes above
		if (pModel)
		{
			ClockworkBehavior *pClockworkBehavior = dynamic_cast <ClockworkBehavior*> (pBehavior);
			if (pClockworkBehavior)
				pClockworkBehavior->vNotifyOfMotors ();
		}

		vRepaintTree ();
	}
	else
	{
		Breakpoint ("CModelTreeView::Model_AddBehavior: null hItem.");
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::ClockworkBehavior_AddMotor (ClockworkBehavior *pBehavior, ClockworkMotor *pMotor)
{
	if (!pBehavior || !pMotor)
		return;

	// find parent
	HTREEITEM hParent;
	Behavior *pBaseBehavior = (Behavior*) pBehavior;
	vFindMatchingItems (pBaseBehavior, vGetItem, &hParent);

	if (!hParent)
		return;

	HTREEITEM hNew = m_Tree.InsertItem (pMotor->szGetName (), hParent);

	if (hNew)
	{
		m_Tree.SetItemData (hNew, (DWORD) new TreeItemData (pMotor));

		bool fEnabled = pMotor->fGetEnableState ();
		m_Tree.SetItemState (hNew, INDEXTOSTATEIMAGEMASK((fEnabled ? 2 : 1)), TVIS_STATEIMAGEMASK);
		Generic::Listener<ClockworkMotorEvents>::vRegisterForEventsFrom ((Notifier<ClockworkMotorEvents>*) pMotor);
		vRepaintTree ();
	}
	else
	{
		Breakpoint ("CModelTreeView::Behavior_AddMotor: null hItem.");
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::Notifier_Deleted (Component *pComponent)
{
	vFindMatchingItems (pComponent, vDelete);
	vRepaintTree ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::Notifier_Deleted (Behavior *pBehavior)
{
	vFindMatchingItems (pBehavior, vDelete);
	vRepaintTree ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::Notifier_Deleted (ClockworkMotor *pMotor)
{
	vFindMatchingItems (pMotor, vDelete);
	vRepaintTree ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::Component_Changed (Juice::Component *pComponent)
{
	if (!pComponent)
	{
		Breakpoint ("CModelTreeView::Component_Change: null pComponent.");
		return;
	}

	vFindMatchingItems (pComponent, vComponentChange);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::Behavior_Changed (Juice::Behavior *pBehavior)
{
	if (!pBehavior)
	{
		Breakpoint ("CModelTreeView::Behavior_Change: null pBehavior.");
		return;
	}

	vFindMatchingItems (pBehavior, vBehaviorChange);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::ClockworkMotor_Changed (Juice::ClockworkMotor *pMotor)
{
	if (!pMotor)
	{
		Breakpoint ("CModelTreeView::Motor_Change: null pMotor.");
		return;
	}

	vFindMatchingItems (pMotor, vMotorChange);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::Selection_Add (SelectionSet *pSet, Selectable *pSelectable, bool fAutomatic)
{
	vFindMatchingItems (pSelectable, vSelect, (void*) true);
}

/****************************************************************************/
/** I originally tr
*****************************************************************************/
void CModelTreeView::Selection_Remove (SelectionSet *pSet, Selectable *pSelectable)
{
	vFindMatchingItems (pSelectable, vSelect, (void*) false);

	// hook for the derived class
	//Motor *pMotion = pSet->pGetMotor (pSelectable);
	//if (pMotion)
	//	vDeselectMotor (pMotion);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vResetSelectionSet ()
{
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vSelectMotor (ClockworkMotor *pMotion)
{
	theApp.m_Juice.m_SelectionSet.vAdd (pMotion);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::vSelectBehavior (Behavior *pBehavior)
{
	theApp.m_Juice.m_SelectionSet.vAdd (pBehavior);
}

/****************************************************************************/
/** 
*****************************************************************************/
BEGIN_MESSAGE_MAP(CModelTreeView, CView)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_ACTIVATE()
	ON_WM_DESTROY()
	ON_MESSAGE (UWM_TV_CHECKBOX, OnPostCheckBoxOperation)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CBehaviorTreeView, CModelTreeView)
	ON_WM_CREATE()
END_MESSAGE_MAP()

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::OnDraw(CDC*)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
int CModelTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	RECT rc;
	GetClientRect (&rc);

	// Create the CTreeCtrl
	m_Tree.Create (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS |  // TVS_SHOWSELALWAYS
		TVS_EDITLABELS | // TVS_CHECKBOXES |
		WS_CHILD | WS_VISIBLE, rc, this, iTreeCtrlID);

	m_htiBodies = m_Tree.InsertItem          ("Beams",   NULL);
	m_htiRevoluteJoints = m_Tree.InsertItem  ("Hinges",  NULL);
	m_htiPrismaticJoints = m_Tree.InsertItem ("Sliders", NULL);
	m_htiSphericalJoints = m_Tree.InsertItem ("Ball Joints", NULL);
	m_htiBehaviors = m_Tree.InsertItem ("Behaviors", NULL);

	m_Tree.ModifyStyle (0, TVS_CHECKBOXES);

	vHideCheckBox (m_htiBodies);
	vHideCheckBox (m_htiRevoluteJoints);
	vHideCheckBox (m_htiPrismaticJoints);	
	vHideCheckBox (m_htiSphericalJoints);	
	vHideCheckBox (m_htiBehaviors);	

	return 0;
}

/****************************************************************************/
/** Like the CModelTreeView version, but without the top-level nodes
*** and without the checkboxes.
*****************************************************************************/
int CBehaviorTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	RECT rc;
	GetClientRect (&rc);

	// Create the CTreeCtrl
	m_Tree.Create (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS |  // TVS_SHOWSELALWAYS
		TVS_EDITLABELS | // TVS_CHECKBOXES |
		WS_CHILD | WS_VISIBLE, rc, this, 0);

	return 0;
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::OnClose()
{
	// These windows must not respond to ctrl-F4
	// CMDIChildWnd::OnClose();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	RECT rc;
	GetClientRect (&rc);
	m_Tree.MoveWindow (&rc, TRUE);
}

/****************************************************************************/
/** The following web site was a godsend!
*** http://buerger.metropolis.de/bitbucket/howto/checktree.html
*****************************************************************************/
LRESULT CModelTreeView::OnPostCheckBoxOperation (WPARAM wp, LPARAM lp)
{
	static bool fSkipThis;

	if (fSkipThis)
		return 0;

	fSkipThis = true;

	HTREEITEM hItem = (HTREEITEM) lp;

	// check/uncheck a behavior or motion link
	TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hItem);
	if (pItemData)
	{
		switch (pItemData->eGetType ())
		{
		case TreeItemData::tBehavior:
			pItemData->pBehavior()->vSetEnableState (fGetCheck (hItem));
			break;

		case TreeItemData::tMotor:
			pItemData->pMotor()->vSetEnableState (fGetCheck (hItem));
			break;
		}
	}

	vRepaintTree ();

	fSkipThis = false;

	return 0;
}

BOOL CModelTreeView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR *pnm = (NMHDR*) lParam;
	if (pnm)
	{
		if (NM_CLICK == pnm->code)
		{
			CPoint pt;
			GetCursorPos (&pt);
			m_Tree.ScreenToClient (&pt);
			UINT uFlags = NULL;
			HTREEITEM hPick = m_Tree.HitTest (pt, &uFlags);

			if (hPick)
			{
				if (uFlags & TVHT_ONITEMSTATEICON)
				{
					PostMessage(UWM_TV_CHECKBOX, pnm->idFrom, (LPARAM) hPick);

/*					// check/uncheck a behavior or motion link
					TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hPick);
					if (pItemData)
					{
						switch (pItemData->eGetType ())
						{
						case TreeItemData::tBehavior:
							pItemData->pBehavior()->vSetEnableState (!fGetCheck (hPick));
							break;

						case TreeItemData::tMotor:
							pItemData->pMotor()->vSetEnableState (!fGetCheck (hPick));
							break;
						}
					}

					vRepaintTree ();
					*pResult = 1;
					//return FALSE;
*/
				}
				else
				{
					// modify the selection set
					GetAsyncKeyState (VK_CONTROL);
					if (!GetAsyncKeyState (VK_CONTROL))
					{
						theApp.m_Juice.m_SelectionSet.vReset ();
						vResetSelectionSet ();
					}

					TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (hPick);
					if (pItemData)
					{
						switch (pItemData->eGetType ())
						{
						case TreeItemData::tComponent:
							theApp.m_Juice.m_SelectionSet.vAdd (pItemData->pComponent());
							break;

						case TreeItemData::tBehavior:
							// hook for derived class used in motion view
							vSelectBehavior (pItemData->pBehavior ());
							break;
						
						case TreeItemData::tMotor:
							// hook for derived class used in motion view
							vSelectMotor (pItemData->pMotor ());
							break;

						}
					}
				}
			}

		}
		else if (TVN_ENDLABELEDIT == pnm->code)
		{
			NMTVDISPINFO *pnmEdit = (NMTVDISPINFO*) lParam;
			Component *pComponent = null;

			if (pnmEdit)
			{
				TreeItemData *pItemData = (TreeItemData*) m_Tree.GetItemData (pnmEdit->item.hItem);
				if (pItemData)
				{
					switch (pItemData->eGetType ())
					{
					case TreeItemData::tComponent:
						if (pItemData->pComponent () && pnmEdit->item.pszText)
							pItemData->pComponent ()->vSetName (pnmEdit->item.pszText);
						break;

					case TreeItemData::tBehavior:
						if (pItemData->pBehavior () && pnmEdit->item.pszText)
							pItemData->pBehavior ()->vSetName (pnmEdit->item.pszText);
						break;
					}

					vRepaintTree ();
				}
			}
		}
		else if (TVN_DELETEITEM == pnm->code)
		{
			NMTREEVIEW *pnmDelete = (NMTREEVIEW*) pnm;
			delete (TreeItemData*) pnmDelete->itemOld.lParam;
		}
	}

	return __super::OnNotify(wParam, lParam, pResult);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CModelTreeView::OnDestroy()
{
	vFindMatchingItems (null, vDeleteEverything);
}

// ModelTreeView.cpp ends here ->