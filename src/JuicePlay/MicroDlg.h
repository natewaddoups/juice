/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: MicroDlg.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

// CMicroDlg dialog

class CMicroDlg : public CDialog
{
	DECLARE_DYNAMIC(CMicroDlg)

	// Prompt mode
	CString m_strPrompt;

	// Edit mode
	CString m_strName;
	CString m_strValue;
	CEdit m_wndEdit;

	// List mode
	std::list<std::string> m_lsStrings;
	CComboBox m_wndListBox;

public:
	CMicroDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMicroDlg();

	enum Mode
	{
		mInvalid,
		mPrompt,
		mEdit,
		mList
	};

	enum NotificationCode
	{
		ncCancel,
		ncEnter,
		ncListPick,
	};

	void vCreate (CWnd *pParent, UINT uID);

	void vShowPrompt (RECT *prc, const char *szPrompt);
	void vShowEdit   (RECT *prc, const char *szName, const char *szValue);
	void vShowList   (RECT *prc, const char *szName);

	void vHide ();

	void vSetEditValue (const char *szValue);
	void vGetEditValue (CString &strValue);

	void vClearList ();
	void vAddListItem (int iID, const char *szText);

	void vSelectListItem (int iID);

	bool fGetSelection (int *piID, const char **pszText);

// Dialog Data
	enum { IDD = IDD_MICRODIALOG };

protected:
	Mode m_eMode;
	void vSetMode (Mode eMode);

	void vShow (RECT *prc);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLbnSelChange();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg BOOL OnNcActivate(BOOL bActive);
};

typedef struct tagNM_MicroDlg
{
	NMHDR hdr;
//	CMicroDlg::NotificationCode code;
} NM_MicroDlg;

// MicroDlg.h ends here ->