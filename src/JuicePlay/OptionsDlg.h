/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: OptionsDlg.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once
#include "afxwin.h"

#include "JuiceDlg.h"

// COptionsDlg dialog

class COptionsDlg : public CJuiceDlg
{
	DECLARE_DYNAMIC(COptionsDlg)

public:
	COptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COptionsDlg();

	void vSetHelpText (const char szText);

// Dialog Data
	enum { IDD = IDD_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	virtual BOOL OnInitDialog();
	CString m_strMinDelta;
	CString m_strMaxDelta;
};

// OptionsDlg.h ends here ->