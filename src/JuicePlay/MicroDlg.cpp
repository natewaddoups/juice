/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: MicroDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

This defines the class for the 'micro' dialog that appears when the user edits
property values in the property tree.

*****************************************************************************/


#include "stdafx.h"
#include "JuicePlay.h"
#include "MicroDlg.h"


// CMicroDlg dialog

const int iEditChildID = 1;
const int iListChildID = 2;

IMPLEMENT_DYNAMIC(CMicroDlg, CDialog)
CMicroDlg::CMicroDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMicroDlg::IDD, pParent),
	m_eMode (mInvalid)
{
}

CMicroDlg::~CMicroDlg()
{
}

void CMicroDlg::vCreate (CWnd *pParent, UINT uID)
{
	if (!IsWindow (m_hWnd))
	{
		Create (IDD_MICRODIALOG, pParent);

		SetWindowLong (m_hWnd, GWL_ID, uID);
		
		RECT rc = { 0, 0, 0, 0 };

		m_wndEdit.Create (WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, rc, this, iEditChildID);

		m_wndListBox.Create (WS_CHILD | WS_BORDER | WS_VSCROLL | CBS_DROPDOWN, rc, this, iListChildID);
		m_wndListBox.SetDroppedWidth (200);
	}
	else
        SetParent (pParent);
}

void CMicroDlg::vShow (RECT *prc)
{
	SetWindowPos (NULL, 
		prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
		SWP_NOZORDER);

	ShowWindow (SW_SHOW);

	// bit of a hack, but the event doesn't get sent
	OnShowWindow(TRUE, 0);

	SetFocus ();
}

void CMicroDlg::vHide ()
{
	if (IsWindow (m_hWnd))
		ShowWindow (SW_HIDE);
}

void CMicroDlg::vShowPrompt (RECT *prc, const char *szPrompt)
{
	vSetMode (mPrompt);
	m_strPrompt = szPrompt;
	vShow (prc);
}

void CMicroDlg::vShowEdit (RECT *prc, const char *szName, const char *szValue)
{
	vSetMode (mEdit);
	m_strName = szName;
	m_strValue = szValue;
	vShow (prc);
}

void CMicroDlg::vShowList (RECT *prc, const char *szName)
{
	vSetMode (mList);
	m_strName = szName;
	vShow (prc);
}

void CMicroDlg::vSetMode (Mode eMode)
{
	m_wndListBox.ShowWindow (SW_HIDE);
	m_wndEdit.ShowWindow (SW_HIDE);

	m_eMode = eMode;

	switch (m_eMode)
	{
	case mEdit:
		m_wndEdit.ShowWindow (SW_SHOW);
		break;

	case mList:
		m_wndListBox.ShowWindow (SW_SHOW);
		break;
	}
}

void CMicroDlg::vGetEditValue (CString &strValue)
{
	m_wndEdit.GetWindowText (strValue);
}

void CMicroDlg::vSetEditValue (const char *szValue)
{
	m_wndEdit.SetWindowText (szValue);
}

void CMicroDlg::vClearList ()
{
	m_wndListBox.ResetContent ();
}

void CMicroDlg::vAddListItem (int iID, const char *szText)
{
	int iIndex = m_wndListBox.InsertString (0, szText);
	m_wndListBox.SetItemData (iIndex, iID);
}

bool CMicroDlg::fGetSelection (int *piID, const char **pszText)
{
	int iIndex = m_wndListBox.GetCurSel ();
	if (iIndex == CB_ERR)
		return false;

	*piID = m_wndListBox.GetItemData (iIndex);
	m_wndListBox.GetLBText (iIndex, m_strValue);
	*pszText = m_strValue;

	return true;
}

void CMicroDlg::vSelectListItem (int iID)
{
	for (int iIndex = 0; iIndex < m_wndListBox.GetCount (); iIndex++)
	{
		if (m_wndListBox.GetItemData (iIndex) == iID)
		{
			m_wndListBox.SetCurSel (iIndex);
			return;
		}
	}
}

void CMicroDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMicroDlg, CDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_WM_SETFOCUS()
	ON_WM_TIMER()
	ON_WM_KILLFOCUS()
	ON_LBN_SELCHANGE(iListChildID, OnLbnSelChange)
	ON_WM_ACTIVATE()
	ON_WM_NCACTIVATE()
END_MESSAGE_MAP()

const int iPixPadding = 2;
const int iCharMaxValueWidth = 10;

void CMicroDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		// Adjust the size - start by getting the font size
		CDC *pDC = GetDC ();
		HFONT hFont = (HFONT) GetStockObject (SYSTEM_FONT);
		HFONT hFontOld = (HFONT) pDC->SelectObject (hFont);
		pDC->SelectObject (hFontOld);

		// Get desired dimensions
		CSize sizeChar = pDC->GetTextExtent ("X");
		CSize sizeName = pDC->GetTextExtent (m_strName);
		int iPixNameWidth = sizeName.cx + sizeChar.cx;
		int iPixTotalHeight = sizeChar.cy + (iPixPadding * 5);

		// Set new position - same left edge, same width, same vertical center, new height
		RECT rc;
		GetWindowRect (&rc);

		if (GetParent ())
			GetParent ()->ScreenToClient (&rc);

		int iVerticalCenter = (rc.top + rc.bottom) / 2;

		// I have no idea why this " / 2" is necessary.  
		SetWindowPos (NULL, 
			rc.left, iVerticalCenter - (iPixTotalHeight / 2),
			rc.right - (rc.left / 2) - 1, iPixTotalHeight, SWP_NOZORDER);

		// position the control
		rc.right -= rc.left;
		rc.left = iPixPadding + iPixNameWidth;
		rc.top = iPixPadding;
		rc.bottom = iPixTotalHeight - (iPixPadding * 2);

		switch (m_eMode)
		{
		case mEdit:
			// show the edit box
			m_wndEdit.SetWindowText (m_strValue);
			m_wndEdit.MoveWindow (&rc);

			// select the edit box text
			m_wndEdit.SetSel (0, -1);
			break;

		case mList:
			rc.bottom = 100;
			m_wndListBox.MoveWindow (&rc);
			break;
		}
	}

	SetTimer (0, 1, NULL);
}

void CMicroDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	RECT rc;
	GetClientRect (&rc);

	dc.SetTextColor (GetSysColor (COLOR_BTNTEXT));
	dc.SetBkColor (GetSysColor (COLOR_BTNFACE));

	HFONT hFont = (HFONT) GetStockObject (SYSTEM_FONT);
	HFONT hFontOld = (HFONT) dc.SelectObject (hFont);


	switch (m_eMode)
	{
	case mEdit:
	case mList:
		dc.TextOut (iPixPadding, iPixPadding, m_strName);
		break;

	case mPrompt:
		dc.TextOut (iPixPadding, iPixPadding, m_strPrompt);
		break;
	}

	dc.SelectObject (hFontOld);
}

// CMicroDlg message handlers

void CMicroDlg::OnBnClickedOk()
{
	NM_MicroDlg nm;
	nm.hdr.hwndFrom = m_hWnd;
	nm.hdr.idFrom = GetWindowLong (m_hWnd, GWL_ID);
	nm.hdr.code = (int) ncEnter;	

	if (GetParent ())
		GetParent ()->SendMessage (WM_NOTIFY, NULL, (LPARAM) &nm);
}

void CMicroDlg::OnBnClickedCancel()
{
	NM_MicroDlg nm;
	nm.hdr.hwndFrom = m_hWnd;
	nm.hdr.idFrom = GetWindowLong (m_hWnd, GWL_ID);
	nm.hdr.code = (int) ncCancel;

	if (GetParent ())
		GetParent ()->SendMessage (WM_NOTIFY, NULL, (LPARAM) &nm);
}

void CMicroDlg::OnSetFocus(CWnd* pOldWnd)
{
	CDialog::OnSetFocus(pOldWnd);

	switch (m_eMode)
	{
	case mEdit:
		m_wndEdit.SetFocus ();
		break;

	case mList:
		m_wndListBox.SetFocus ();
		break;
	}
}

void CMicroDlg::OnTimer(UINT nIDEvent)
{
	// this is because the tree always steals the focus back after NM_CLICK processing
	SetFocus ();

	KillTimer (nIDEvent);

	CDialog::OnTimer(nIDEvent);
}

void CMicroDlg::OnKillFocus(CWnd* pNewWnd)
{
	CDialog::OnKillFocus(pNewWnd);
/*
	if (GetParent() && (pNewWnd->m_hWnd == GetParent()->m_hWnd))
		return;

	if (GetParent() && GetParent()->GetParent () && (pNewWnd->m_hWnd == GetParent()->GetParent()->m_hWnd))
		return;

	if (pNewWnd->GetParent() && pNewWnd->GetParent()->GetParent() == GetParent ())
		return;

	NM_MicroDlg nm;
	nm.hdr.hwndFrom = m_hWnd;
	nm.hdr.idFrom = GetWindowLong (m_hWnd, GWL_ID);
	nm.hdr.code = (int) ncCancel;

	if (GetParent ())
		GetParent ()->SendMessage (WM_NOTIFY, NULL, (LPARAM) &nm);
*/
}

void CMicroDlg::OnLbnSelChange()
{
	OnBnClickedOk();
}

void CMicroDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);

	switch (nState)
	{
	case WA_INACTIVE:
		OutputDebugString ("CMicroDlg::OnActivate: WA_INACTIVE\n");
		break;

	case WA_ACTIVE:
		OutputDebugString ("CMicroDlg::OnActivate: WA_ACTIVE\n");
		break;

	case WA_CLICKACTIVE:
		OutputDebugString ("CMicroDlg::OnActivate: WA_CLICKACTIVE\n");
		break;
	}
	// TODO: Add your message handler code here
}

BOOL CMicroDlg::OnNcActivate(BOOL bActive)
{
	if (bActive)
		OutputDebugString ("CMicroDlg::OnNcActivate: true\n");
	else
		OutputDebugString ("CMicroDlg::OnNcActivate: false\n");

	return CDialog::OnNcActivate(bActive);
}

// MicroDlg.cpp ends here ->