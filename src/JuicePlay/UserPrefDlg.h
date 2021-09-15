/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: UserPrefDlg.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

#include "JuiceDlg.h"

// CUserPrefDlg dialog

class CUserPrefDlg : public CJuiceDlg
{
	DECLARE_DYNAMIC(CUserPrefDlg)

public:
	CUserPrefDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUserPrefDlg();

// Dialog Data
	enum { IDD = IDD_USERPREFDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

// UserPrefDlg.h ends here ->