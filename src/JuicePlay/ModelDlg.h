/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: ModelDlg.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

#include "JuiceDlg.h"

// CModelDlg dialog

class CModelDlg : public CJuiceDlg
{
	DECLARE_DYNAMIC(CModelDlg)

public:
	CModelDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CModelDlg();

	real m_rSpeed;
	real m_rDelay;
	real m_rPhase;

// Dialog Data
	enum { IDD = IDD_MODEL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CString m_strSpeed;
	CString m_strDelay;
	CString m_strPhase;
protected:
	virtual void OnOK();
};

// ModelDlg.h ends here ->