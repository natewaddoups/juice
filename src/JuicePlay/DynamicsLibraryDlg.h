/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: DynamicsLibraryDlg.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once
#include "JuiceDlg.h"
#include "afxwin.h"

// CDynamicsLibraryDlg dialog

class CDynamicsLibraryDlg : public CJuiceDlg
{
	DECLARE_DYNAMIC(CDynamicsLibraryDlg)

public:
	CDynamicsLibraryDlg(CWnd* pParent, const Juice::DynamicsLibraryList::List &List, Juice::Dynamics::World *pActiveWorld);
	virtual ~CDynamicsLibraryDlg();

	const Juice::DynamicsLibraryList::List &m_List;
	const Juice::DynamicsLibraryList::Info *m_pInfo;

// Dialog Data
	enum { IDD = IDD_DYNAMICSFACTORIES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	virtual void OnOK();

protected:
	CListBox m_ListBox;
	Juice::Dynamics::World *m_pActiveWorld;
};

// DynamicsLibraryDlg.h ends here ->