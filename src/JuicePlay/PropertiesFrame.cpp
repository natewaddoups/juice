/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: PropertiesFrame.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include <crtdbg.h>
#include "JuicePlay.h"
#include <JuiceEngine.h>	
#include "PropertiesFrame.h"

using namespace Juice;

// CPropertiesFrame

/****************************************************************************/
/** 
*****************************************************************************/
IMPLEMENT_DYNCREATE(CPropertiesFrame, CMDIChildWnd)

CPropertiesFrame::CPropertiesFrame() :
	m_pProperties (null), 
	m_pCurrentProperty (null), 
	m_hCurrentItem (null),
	m_pSelected (null),
	m_pHasProperties (null),
	m_pPickTwinFor (null),
	m_ePickMode (pmPickNormal)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
CPropertiesFrame::~CPropertiesFrame()
{
	vUnregister ();
}


/****************************************************************************/
/** 
*****************************************************************************/
BEGIN_MESSAGE_MAP(CPropertiesFrame, CMDIChildWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::vSetLabel (HTREEITEM hItem)
{
	if (!hItem)
		return;

	Generic::Property *pProperty = (Generic::Property*) m_Tree.GetItemData (hItem);
	if (!pProperty)
		return;

	//if (typeid (*pProperty) == typeid (Generic::ActionProperty))
	if (Generic::IsKindOfPtr<Generic::ActionProperty*> (pProperty))
	{
		std::string strName = pProperty->szGetName();
		m_Tree.SetItemText (hItem, strName.c_str ());
	}
	else
	{
		// Get name and value
		std::string strName = pProperty->szGetName();
		std::string strValue = pProperty->szGetValueToString (m_pHasProperties);

		// remove newlines from value
		int iPosition = 0;
		while ((iPosition = strValue.find ('\n')) != std::string::npos)
		{
			strValue.replace (iPosition, 1, " ");
			strValue.insert (iPosition, "  ");
		}

		// Concatenate the name and value
		char *szName = new char [strName.length () + strValue.length () + 10];
		wsprintf (szName, "%s: %s", strName.c_str (), strValue.c_str ());

		// Set the tree item's label
		m_Tree.SetItemText (hItem, szName);

		// release the text label buffer
		delete szName;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::vAddProperty (const Generic::Property *pProperty, HTREEITEM hParent)
{
	// Add the item
	HTREEITEM hNew = m_Tree.InsertItem ("", hParent);
	m_Tree.SetItemData (hNew, (DWORD_PTR) pProperty);
	vSetLabel (hNew);
}

void CPropertiesFrame::vRegister ()
{
	// This will keep the property view items up to date
	Listener<SelectableEvents>::vRegisterForEventsFrom (m_pSelected);

	if (IsKindOfPtr <Component*> (m_pSelected))
		Listener<ComponentEvents>::vRegisterForEventsFrom (dynamic_cast <Component*> (m_pSelected));

	if (typeid (m_pSelected) == typeid (Behavior*))
		Listener<BehaviorEvents>::vRegisterForEventsFrom (dynamic_cast <Behavior*> (m_pSelected));

	if (IsKindOfPtr<ClockworkMotor*> (m_pSelected))
		Listener<ClockworkMotorEvents>::vRegisterForEventsFrom (dynamic_cast <ClockworkMotor*> (m_pSelected));
}

void CPropertiesFrame::vUnregister ()
{
	if (!m_pSelected)
		return;

	// This will keep the property view items up to date
	Listener<SelectableEvents>::vUnregister (m_pSelected);

	//if (typeid (m_pSelected) == typeid (Component*))
	if (IsKindOfPtr <Component*> (m_pSelected))
		Listener<ComponentEvents>::vUnregister (dynamic_cast <Component*> (m_pSelected));

	//if (typeid (m_pSelected) == typeid (Behavior*))
	if (IsKindOfPtr <Behavior*> (m_pSelected))
		Listener<BehaviorEvents>::vUnregister (dynamic_cast <Behavior*> (m_pSelected));

	//if (IsKindOfPtr<Motor*> (m_pSelected))
	if (IsKindOfPtr <ClockworkMotor*> (m_pSelected))
		Listener<ClockworkMotorEvents>::vUnregister (dynamic_cast <ClockworkMotor*> (m_pSelected));
}


/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::Selection_Add (SelectionSet *pSet, Selectable *pSelectable, bool fAutomatic)
{
	try
	{
		if (m_ePickMode == pmPickNormal)
		{
			// FIXFIX: get rid of: The selection set object will be needed later for RTTI-like functionality, so save this pointer
			m_pSelectionSet = pSet;

			// If this is just the twin of what's already selected, ignore it
			if (fAutomatic)
				return;

			// Wipe the slate clean
			vReset ();

			// Initialize member variables
			m_pSelected = pSelectable;
			m_pHasProperties = pSelectable;
			m_pProperties = m_pSelected->pGetPropertyTree ();

			vRegister ();
			
			// draw the tree on screen
			const std::list<Generic::PropertyTree::Node*> &Nodes = m_pProperties->lsGetNodes ();

			std::list<Generic::PropertyTree::Node*>::const_iterator iterFirstLevel;
			std::list<Generic::Property*>::const_iterator iterSecondLevel;
			
			for (iterFirstLevel = Nodes.begin (); iterFirstLevel != Nodes.end (); iterFirstLevel++)
			{
				const Generic::PropertyTree::Node *pCurrentNode = *iterFirstLevel;

				switch (pCurrentNode->eGetType ())
				{
					
				case Generic::PropertyTree::Node::Type::ntGroup:
					{
						// Create a tree item containing the group's name, and set the data pointer to null
						HTREEITEM hGroup = m_Tree.InsertItem (pCurrentNode->szGetName ());
						m_Tree.SetItemData (hGroup, NULL);

						// Add the properties under this item
						const std::list<Property*> &Properties = pCurrentNode->lsGetProperties ();
						for (iterSecondLevel = Properties.begin (); iterSecondLevel != Properties.end (); iterSecondLevel++)
						{
							const Generic::Property *pNewProperty = *iterSecondLevel;
							vAddProperty (pNewProperty, hGroup);
						}

						m_Tree.Expand (hGroup, TVE_EXPAND);
					}
					break;

				case Generic::PropertyTree::Node::Type::ntProperty:
					{
						Generic::Property *pNewProperty = pCurrentNode->pGetProperty ();
						vAddProperty (pNewProperty);
					}
					break;
				}
			}
		}
		else if (m_ePickMode == pmPickTwin)
		{
			if (typeid (*pSelectable) == typeid (*m_pPickTwinFor))
			{
				Component *pTwin = dynamic_cast <Component*> (pSelectable);

				if (m_pPickTwinFor && pTwin && 
					(typeid (*pTwin) == typeid (*m_pPickTwinFor)))
					//(m_pPickTwinFor->eGetType () == pTwin->eGetType ()))
				{
					m_pPickTwinFor->vSetTwin (pTwin);
					pTwin->vSetTwin (m_pPickTwinFor);
					vSetLabel (m_hCurrentItem);
				}
			}

			m_MicroDlg.vHide ();
			m_ePickMode = pmPickNormal;
		}
		else if (m_ePickMode == pmPickBodyForJoint)
		{
			if (pSelectable && (typeid (*pSelectable) == typeid (Body)))
			{
				Body *pBody = dynamic_cast <Body*> (pSelectable);

				if (m_pSelected && m_pCurrentProperty)				
				{
					((BodyProperty*)m_pCurrentProperty)->vSetValue (m_pSelected, pBody);

					vSetLabel (m_hCurrentItem);
				}
			}

			m_MicroDlg.vHide ();		
			m_ePickMode = pmPickNormal;
		}
		else if (m_ePickMode == pmPickNonTwinComponent)
		{
			//if (((typeid (*pSelectable) == typeid (Body)) && (typeid (*m_pCurrentProperty) == typeid (BodyProperty))) ||
			//	((typeid (*pSelectable) == typeid (RevoluteJoint)) && (typeid (*m_pCurrentProperty) == typeid (RevoluteJointProperty))) || 
			//	((typeid (*pSelectable) == typeid (PrismaticJoint)) && (typeid (*m_pCurrentProperty) == typeid (PrismaticJointProperty))))
			if ((IsKindOfPtr <Body*> (pSelectable)           && IsKindOfPtr <BodyProperty*> (m_pCurrentProperty)) ||
				(IsKindOfPtr <Joint*> (pSelectable)          && IsKindOfPtr <JointProperty*> (m_pCurrentProperty)) ||
				(IsKindOfPtr <RevoluteJoint*> (pSelectable)  && IsKindOfPtr <RevoluteJointProperty*> (m_pCurrentProperty)) ||
				(IsKindOfPtr <PrismaticJoint*> (pSelectable) && IsKindOfPtr <PrismaticJointProperty*> (m_pCurrentProperty)))
			{
				ComponentProperty *pCurrentProperty = dynamic_cast <ComponentProperty*> (m_pCurrentProperty);
				Component *pNewSelection = dynamic_cast <Component*> (pSelectable);

				Component *pComponent = dynamic_cast <Component*> (pSelectable);

				if (pCurrentProperty && pNewSelection && pComponent)
				{
					pCurrentProperty->vSetValue (m_pSelected, pComponent);
				}
				else
				{
					Breakpoint ("CPropertiesFrame::Selection_Add: ComponentProperty picked a wrong thing.");
				}
			}

			m_ePickMode = pmPickNormal;
			m_MicroDlg.vHide ();
		}
		else
		{
			// if this happens, there's been some sort of bug
			Breakpoint ("CPropertiesFrame::Selection_Add: undefined pick mode");

			// recover as gracefully as possible...
			m_MicroDlg.vHide ();
			m_ePickMode = pmPickNormal;
		}
	}
	catch (...)
	{
		// if this happens, there's been some sort of even worse bug
		Breakpoint ("CPropertiesFrame::Selection_Add: exception thrown");

		// recover as gracefully as possible...
		m_MicroDlg.vHide ();
		m_ePickMode = pmPickNormal;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::Selection_Remove (SelectionSet *pSet, Selectable *pSelectable)
{
	// Generally, deselected objects are removed from the property frame.
	// But, in these cases, the object depicted in the property frame should not be changed.
	if ((m_ePickMode == pmPickBodyForJoint) || (m_ePickMode == pmPickNonTwinComponent))
	{
	}
	else
	{
		vReset ();
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::Component_Changed  (Component *pComponent)
{
	vRefresh ();
}

void CPropertiesFrame::Behavior_Changed   (Behavior *pBehavior)
{
	vRefresh ();
}

void CPropertiesFrame::ClockworkMotor_Changed (ClockworkMotor *pMotor)
{
	vRefresh ();
}

void CPropertiesFrame::Notifier_Deleted (Selectable *pSelectable)
{
	vReset ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::vReset ()
{
	if ((m_pSelected) && (m_ePickMode == pmPickNormal))
		vUnregister ();

	m_pSelected = null;
	m_pProperties = null;
	m_pCurrentProperty = null;
	m_hCurrentItem = null;
	m_pHasProperties = null;

	// this must not be nulled out, as this function will be invoked when the twin is picked
//	m_pPickTwinFor = null;

	m_MicroDlg.vHide ();

	m_Tree.SelectItem (NULL);
	m_Tree.DeleteAllItems ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::vUpdateSelectedObjects (Selectable *pReference, const char *szValue)
{
	if (!m_pCurrentProperty || !m_pSelectionSet)
		return;

	// Apply the new value to the selection set
	const SelectableList &lsSelectionSet = m_pSelectionSet->lsGetContents ();
	SelectableList::const_iterator iter;
	for (iter = lsSelectionSet.begin (); iter != lsSelectionSet.end (); iter++)
	{
		Selectable *pSelectable = *iter;
		if (typeid (pSelectable) == typeid (pReference))
			m_pCurrentProperty->fSetValueFromString (pSelectable, szValue);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::vUpdateSelectedObjects (Selectable *pReference, int iValue)
{
	if (!m_pCurrentProperty || !m_pSelectionSet)
		return;
	
	if (typeid (*m_pCurrentProperty) != typeid (ListProperty))
		return;

	ListProperty *pListProperty = dynamic_cast <ListProperty*> (m_pCurrentProperty);

	// Apply the new value to the selection set
	const SelectableList &lsSelectionSet = m_pSelectionSet->lsGetContents ();
	SelectableList::const_iterator iter;
	for (iter = lsSelectionSet.begin (); iter != lsSelectionSet.end (); iter++)
	{
		Selectable *pSelectable = *iter;
		if (typeid (*pSelectable) == typeid (*pReference))
			pListProperty->fSetValueFromInt (pSelectable, iValue);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::vShowObject (Selectable *pSelectable)
{
	Selection_Add (&(theApp.m_Juice.m_SelectionSet), pSelectable, false);
}

void CPropertiesFrame::vForceDisplayOf (Selectable *pSelectable)
{
	Selection_Add (null, pSelectable, false);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::vRecursiveRefresh (HTREEITEM hItem)
{
	// refresh this one
	vSetLabel (hItem);

	// check all of this item's children
	HTREEITEM hChildItem = m_Tree.GetChildItem (hItem);
	while (hChildItem)
	{
		vRecursiveRefresh (hChildItem);
		hChildItem = m_Tree.GetNextItem (hChildItem, TVGN_NEXT);
	} 
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::vRefresh ()
{
	HTREEITEM hItem = m_Tree.GetChildItem (NULL);

	while (hItem)
	{
		vRecursiveRefresh (hItem);
		hItem = m_Tree.GetNextItem (hItem, TVGN_NEXT);
	}
}


/****************************************************************************/
/** 
*****************************************************************************/
int CPropertiesFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// give the microdialog a parent window handle
	m_MicroDlg.vCreate (this, idMicroDlg);

	// Set the window caption
	CString strCaption;
	strCaption.LoadString (IDS_CAPTION_PROPERTIESVIEW);
	SetWindowText (strCaption);

	// remove the 'close' item from the system menu (also disables the close box in the top-right corner)
	CMenu *pSysMenu = GetSystemMenu (false);
	pSysMenu->DeleteMenu (SC_CLOSE, MF_BYCOMMAND);
	
	// create the tree control
	RECT rc;
	GetClientRect (&rc);

	m_Tree.Create (TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | 
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rc, this, idTree);
	
	return 0;
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::OnClose()
{
	// These windows must not respond to ctrl-F4
	// MDIChildWnd::OnClose();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIChildWnd::OnSize(nType, cx, cy);

	RECT rc;
	GetClientRect (&rc);
	m_Tree.MoveWindow (&rc, TRUE);
}

/****************************************************************************/
/** 
*****************************************************************************/
BOOL CPropertiesFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR *phdr = (NMHDR*) lParam;
	if (!phdr)
		return __super::OnNotify(wParam, lParam, pResult);

	if (phdr->idFrom == idTree)
	{
		if (NM_CLICK == phdr->code)
		{
			CPoint pt;
			GetCursorPos (&pt);
			m_Tree.ScreenToClient (&pt);
			UINT uFlags = NULL;
			HTREEITEM hPick = m_Tree.HitTest (pt, &uFlags);

			// Deselect the old item
			if (m_hCurrentItem)
			{
				m_MicroDlg.vHide ();

				m_pCurrentProperty = null;
				m_hCurrentItem = null;

				m_ePickMode = pmPickNormal;
			}

			// Do select the new item
			if (hPick)
			{
				// The the bounding rectangle over the selected item
				RECT rc;
				m_Tree.GetItemRect (hPick, &rc, true);

				// make the item rect extend all the way to the edge of the frame window
				RECT rcFrame;
				GetClientRect (&rcFrame);

				rc.right = rcFrame.right - rc.left;

				Generic::Property *pProperty = (Generic::Property*) m_Tree.GetItemData (hPick);

				if (pProperty && !pProperty->fReadOnly ())
				{
					if (IsKindOfPtr <BodyProperty*> (pProperty))
					{
						// find out if this is a twin job (picking a body for a body)
						// or a joint job (picking a body for a joint)
						if (typeid (*m_pSelected) == typeid (Body))
						{
							m_ePickMode = pmPickTwin;
							m_pPickTwinFor = dynamic_cast <Component*> (m_pSelected);

							m_MicroDlg.vShowPrompt (&rc, "Pick a beam");
						}
						else // must be picking a body for a joint, then
						{
							m_ePickMode = pmPickBodyForJoint;

							m_MicroDlg.vShowPrompt (&rc, "Pick a beam");
						}
					}

					else if (IsKindOfPtr <JointProperty*> (pProperty))
					{
						m_ePickMode = pmPickNonTwinComponent;
						m_MicroDlg.vShowPrompt (&rc, "Pick a joint");
					}


					else if (IsKindOfPtr <RevoluteJointProperty*> (pProperty))
					{
						m_ePickMode = pmPickTwin;
						m_pPickTwinFor = dynamic_cast <Component*> (m_pSelected);

						if (typeid (*m_pSelected) == typeid (RevoluteJoint))
						{
							m_MicroDlg.vShowPrompt (&rc, "Pick a hinge");
						}
					}

					else if (IsKindOfPtr <PrismaticJointProperty*> (pProperty))
					{
						m_ePickMode = pmPickTwin;
						m_pPickTwinFor = dynamic_cast <Component*> (m_pSelected);

						if (typeid (*m_pSelected) == typeid (PrismaticJoint))
						{
							m_MicroDlg.vShowPrompt (&rc, "Pick a slider");
						}
					}

					else if (IsKindOfPtr <SphericalJointProperty*> (pProperty))
					{
						m_ePickMode = pmPickTwin;
						m_pPickTwinFor = dynamic_cast <Component*> (m_pSelected);

						if (typeid (*m_pSelected) == typeid (SphericalJoint))
						{
							m_MicroDlg.vShowPrompt (&rc, "Pick a ball joint");
						}
					}

					else if (IsKindOfPtr <ListProperty*> (pProperty))
					{
						ListProperty *pListProperty = (ListProperty*) pProperty;

						m_MicroDlg.vClearList ();

						int iID = 0;
						const char *szText = null;
						for (Generic::UINT32 u=0; pListProperty->fGetItem (u, &iID, &szText); u++)
							m_MicroDlg.vAddListItem (iID, szText);

						m_MicroDlg.vSelectListItem (pListProperty->iGetValueToInt (m_pSelected));
						
						m_MicroDlg.vShowList (&rc, pProperty->szGetName ());
					}

					else if (IsKindOfPtr <ActionProperty*> (pProperty))
					{
						// invoke the action
						ActionProperty *pActionProperty = (ActionProperty*) pProperty;
						pActionProperty->vInvoke (m_pHasProperties);

						// this will prevent display of the micro dialog
						hPick = NULL;
					}
					else
					{
						// Put the micro dialog over the newly selected item
						const char *szValue = pProperty->szGetValueToString (m_pHasProperties);

						m_MicroDlg.vShowEdit (&rc, pProperty->szGetName (), szValue);
					}

					if (hPick)
					{
						m_hCurrentItem = hPick;
						m_pCurrentProperty = (Generic::Property*) m_Tree.GetItemData (m_hCurrentItem);
						m_Tree.Select (hPick, TVGN_CARET);
					}
					else
					{
						m_Tree.SelectItem (NULL);
					}
				}
			}
		}
		else if (TVN_ITEMEXPANDING == phdr->code)
		{
			NMTREEVIEW *pMessage = (NMTREEVIEW*) lParam;
			m_MicroDlg.vHide ();

			m_pCurrentProperty = null;
			m_hCurrentItem = null;
		}
	}
	// end of tree message processing

	// start of microdialog message processing
	else if (phdr->idFrom == idMicroDlg && m_pCurrentProperty)
	{
		if (phdr->code == CMicroDlg::ncEnter)
		{
			if (IsKindOfPtr <ListProperty*> (m_pCurrentProperty))
			{
				int iID;
				const char *szText;
				if (m_MicroDlg.fGetSelection (&iID, &szText))
				{   
					vUpdateSelectedObjects (m_pSelected, iID);
					// TODO: could this be done better for multiple selections?
					vSetLabel (m_hCurrentItem);
				}
			}
			else
			{
				// Get the new value from the edit control
				CString strNewValue;
				m_MicroDlg.vGetEditValue (strNewValue);

				// apply the change to whatever class of objects are currently selected
				vUpdateSelectedObjects (m_pSelected, strNewValue);

				// TODO: could this be done better for multiple selections?
				vSetLabel (m_hCurrentItem);

				const char *szValue = m_pCurrentProperty->szGetValueToString (m_pHasProperties);
				m_MicroDlg.vSetEditValue (szValue);

				// clear the selection so this window will be notified if the user clicks on the same item again
				m_Tree.SelectItem (NULL);
			}
		}
		else if (phdr->code == CMicroDlg::ncCancel)
		{
			m_MicroDlg.vHide ();
		}
	}
	return __super::OnNotify(wParam, lParam, pResult);
}

/****************************************************************************/
/** 
*****************************************************************************/
/// This will happen if the user presses escape while the floating prompt has focus
void CPropertiesFrame::OnSetFocus(CWnd* pOldWnd)
{
//	__super::OnSetFocus(pOldWnd);

//	m_MicroDlg.vHide ();

//	m_ePickMode = pmPickNormal;

//	m_Tree.SelectItem (NULL);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CPropertiesFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPropertiesFrame::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_MicroDlg.vHide ();

	__super::OnVScroll(nSBCode, nPos, pScrollBar);
}

// PropertiesFrame.cpp ends here ->