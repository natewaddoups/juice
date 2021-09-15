/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: PhysicsDlg.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once
#include "afxwin.h"

#include "JuiceDlg.h"

// CPhysicsDlg dialog

class CPhysicsDlg : public CJuiceDlg
{
	DECLARE_DYNAMIC(CPhysicsDlg)

public:
	CPhysicsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPhysicsDlg();

	void vSetHelpText (const char szText);

// Dialog Data
	enum { IDD = IDD_PHYSICS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	virtual BOOL OnInitDialog();
	CString m_strGravity;
	CString m_strFriction;
	CString m_strBounce;
	CString m_strERP;
	CString m_strCFM;
	CString m_strMinDelta;
	CString m_strMaxDelta;
};

// PhysicsDlg.h ends here ->