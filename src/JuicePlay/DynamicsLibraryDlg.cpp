/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: DynamicsLibraryDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "DynamicsLibraryDlg.h"


using namespace Juice;

// CDynamicsLibraryDlg dialog

IMPLEMENT_DYNAMIC(CDynamicsLibraryDlg, CJuiceDlg)

CDynamicsLibraryDlg::CDynamicsLibraryDlg(CWnd* pParent /*=NULL*/, const Juice::DynamicsLibraryList::List &List, Juice::Dynamics::World *pActiveWorld)
	: CJuiceDlg(CDynamicsLibraryDlg::IDD, pParent), m_List (List), m_pInfo (null), m_pActiveWorld (pActiveWorld)
{
}

CDynamicsLibraryDlg::~CDynamicsLibraryDlg()
{
}

void CDynamicsLibraryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIBRARIES, m_ListBox);
}


BEGIN_MESSAGE_MAP(CDynamicsLibraryDlg, CJuiceDlg)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDynamicsLibraryDlg message handlers

BOOL CDynamicsLibraryDlg::OnInitDialog()
{
	CJuiceDlg::OnInitDialog();

	DynamicsLibraryList::List::const_iterator iter, iterLast = m_List.end ();
	for (iter = m_List.begin (); iter != iterLast; iter++)
	{
		const DynamicsLibraryList::Info *pInfo = *iter;
		int iItem = m_ListBox.AddString (pInfo->szGetDisplayName ());
		m_ListBox.SetItemData (iItem, (DWORD_PTR) pInfo);
	}

	for (int iItem = 0; iItem < m_ListBox.GetCount (); iItem++)
	{
		DynamicsLibraryList::Info *pInfo = (DynamicsLibraryList::Info *) m_ListBox.GetItemData (iItem);

		if (pInfo->pGetWorld () == m_pActiveWorld)
			m_ListBox.SetCurSel (iItem);
	}

	return TRUE;
}

void CDynamicsLibraryDlg::OnOK()
{
	UpdateData (true);

	m_pInfo = (const DynamicsLibraryList::Info *) m_ListBox.GetItemData (m_ListBox.GetCurSel ());

	CDialog::OnOK();
}

// DynamicsLibraryDlg.cpp ends here ->