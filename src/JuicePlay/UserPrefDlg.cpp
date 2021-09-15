/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: UserPrefDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "UserPrefDlg.h"


// CUserPrefDlg dialog

IMPLEMENT_DYNAMIC(CUserPrefDlg, CJuiceDlg)
CUserPrefDlg::CUserPrefDlg(CWnd* pParent /*=NULL*/)
	: CJuiceDlg(CUserPrefDlg::IDD, pParent)
{
}

CUserPrefDlg::~CUserPrefDlg()
{
}

void CUserPrefDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUserPrefDlg, CJuiceDlg)
END_MESSAGE_MAP()

// UserPrefDlg.cpp ends here ->
