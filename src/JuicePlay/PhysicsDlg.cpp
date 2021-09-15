/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: PhysicsDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "PhysicsDlg.h"


// CPhysicsDlg dialog

IMPLEMENT_DYNAMIC(CPhysicsDlg, CJuiceDlg)
CPhysicsDlg::CPhysicsDlg(CWnd* pParent /*=NULL*/)
	: CJuiceDlg(CPhysicsDlg::IDD, pParent)
	, m_strGravity(_T(""))
	, m_strFriction(_T(""))
	, m_strBounce(_T(""))
	, m_strERP(_T(""))
	, m_strCFM(_T(""))
	, m_strMinDelta(_T(""))
	, m_strMaxDelta(_T(""))
{
}

CPhysicsDlg::~CPhysicsDlg()
{
}

void CPhysicsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_GRAVITY, m_strGravity);
	DDX_Text(pDX, IDC_FRICTION, m_strFriction);
	DDX_Text(pDX, IDC_BOUNCE, m_strBounce);
	DDX_Text(pDX, IDC_ERP, m_strERP);
	DDX_Text(pDX, IDC_CFM, m_strCFM);
	DDX_Text(pDX, IDC_MINDELTA, m_strMinDelta);
	DDX_Text(pDX, IDC_MAXDELTA, m_strMaxDelta);
}


// CPhysicsDlg message handlers

BOOL CPhysicsDlg::OnInitDialog()
{
	CJuiceDlg::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// PhysicsDlg.cpp ends here ->