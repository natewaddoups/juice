/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: JuiceDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

This file defines a base class for dialog boxes.  When derived dialog boxes
are displayed, the simulation pauses.

*****************************************************************************/


#include "stdafx.h"
#include "JuicePlay.h"
#include "JuiceDlg.h"


// CJuiceDlg dialog

IMPLEMENT_DYNAMIC(CJuiceDlg, CDialog)

CJuiceDlg::CJuiceDlg(int iIDD, CWnd* pParent)
	: CDialog(iIDD, pParent), m_pHelpText (NULL)
{
	theApp.vPauseAnimation ();
}

CJuiceDlg::~CJuiceDlg()
{
	theApp.vResumeAnimation ();
}

BEGIN_MESSAGE_MAP(CJuiceDlg, CDialog)
	ON_CONTROL_RANGE(EN_SETFOCUS, 0, 0xFFFFFFFF, ControlSetFocus)
END_MESSAGE_MAP()


// CJuiceDlg message handlers

BOOL CJuiceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pHelpText = (CStatic*) GetDlgItem (IDC_HELPTEXT);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CJuiceDlg::ControlSetFocus (UINT uControlID) // , NMHDR *pnm, LRESULT *pResult)
{
	if (m_pHelpText)
	{
		CString str;
		str.LoadString (uControlID);
		m_pHelpText->SetWindowText (str);
	}
}

// JuiceDlg.cpp ends here ->