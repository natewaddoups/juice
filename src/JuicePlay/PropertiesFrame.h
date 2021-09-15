/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: PropertiesFrame.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

#include "MicroDlg.h"

using namespace Juice;

// CPropertiesFrame frame

class CPropertiesFrame : 
	public CMDIChildWnd,
	public Generic::Listener <SelectionSetEvents>,
	public Generic::Listener <SelectableEvents>,
	public Generic::Listener <ComponentEvents>,
	public Generic::Listener <BehaviorEvents>,
	public Generic::Listener <ClockworkMotorEvents>
{
	DECLARE_DYNCREATE(CPropertiesFrame)
protected:
	CPropertiesFrame();           // protected constructor used by dynamic creation
	virtual ~CPropertiesFrame();

	CTreeCtrl m_Tree;
	CMicroDlg m_MicroDlg;

	enum PickMode
	{
		pmPickNormal,
		pmPickBodyForJoint,
		pmPickTwin,
		pmPickNonTwinComponent
	} m_ePickMode;

	enum Children
	{
		idTree = 1,
		idMicroDlg = 2
	};

	HTREEITEM m_hCurrentItem;

	Generic::HasProperties *m_pHasProperties;
	Generic::Property *m_pCurrentProperty;
	Generic::PropertyTree *m_pProperties;
	Juice::Selectable *m_pSelected;
	Juice::Component *m_pPickTwinFor;
	Juice::SelectionSet *m_pSelectionSet;

	void vRegister ();
	void vUnregister ();

	void vSetLabel (HTREEITEM hItem);
	void vAddProperty (const Generic::Property *pNewProperty, HTREEITEM hParent = NULL);

	void Selection_Add (SelectionSet *pSet, Selectable *pSelectable, bool fAutomatic);
	void Selection_Remove (SelectionSet *pSet, Selectable *pSelectable);
	void Notifier_Deleted (Selectable *pSelectable);

	void Component_Changed  (Component *pComponent);
	void Behavior_Changed   (Behavior *pBehavior);
	void ClockworkMotor_Changed (ClockworkMotor *pMotor);

	void vReset ();

	void vUpdateSelectedObjects (Selectable *pReference, const char *szValue);
	void vUpdateSelectedObjects (Selectable *pReference, int iValue);

	void vRecursiveRefresh (HTREEITEM hItem);

public:
	// when does this get used? 
	void vShowObject (Selectable *pSelectable);

	// Works just like the above, but with a null selection-set pointer
	// to indicate that the object is not in the current selection set.
	// This is used by motion view windows, since each motion view may
	// select a different behavior, motor, etc, without affecting the 
	// application's global selection set (this makes it easier to work
	// on multiple behaviors at once).
	void vForceDisplayOf (Selectable *pSelectable);

	void vRefresh ();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
public:
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

// PropertiesFrame.h ends here ->