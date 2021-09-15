/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: OptionsDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "OptionsDlg.h"


// COptionsDlg dialog

IMPLEMENT_DYNAMIC(COptionsDlg, CJuiceDlg)
COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CJuiceDlg(COptionsDlg::IDD, pParent)
	, m_strMinDelta(_T(""))
	, m_strMaxDelta(_T(""))
{
}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MINDELTA, m_strMinDelta);
	DDX_Text(pDX, IDC_MAXDELTA, m_strMaxDelta);
}


// COptionsDlg message handlers

BOOL COptionsDlg::OnInitDialog()
{
	CJuiceDlg::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// OptionsDlg.cpp ends here ->