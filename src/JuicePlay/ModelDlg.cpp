/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: ModelDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "ModelDlg.h"


// CModelDlg dialog

IMPLEMENT_DYNAMIC(CModelDlg, CJuiceDlg)
CModelDlg::CModelDlg(CWnd* pParent /*=NULL*/)
	: CJuiceDlg(CModelDlg::IDD, pParent)
	, m_strSpeed(_T(""))
	, m_strDelay(_T(""))
{
}

CModelDlg::~CModelDlg()
{
}

void CModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MODEL_SPEED, m_strSpeed);
	DDX_Text(pDX, IDC_MODEL_POSEDELAY, m_strDelay);
	DDX_Text(pDX, IDC_MODEL_POSEPHASE, m_strPhase);
}


BEGIN_MESSAGE_MAP(CModelDlg, CJuiceDlg)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CModelDlg message handlers

BOOL CModelDlg::OnInitDialog()
{
	sprintf (m_strSpeed.GetBufferSetLength (100), "%.2f", m_rSpeed);
	sprintf (m_strDelay.GetBufferSetLength (100), "%.2f", m_rDelay);
	sprintf (m_strPhase.GetBufferSetLength (100), "%.2f", m_rPhase);
	m_strSpeed.ReleaseBuffer ();
	m_strDelay.ReleaseBuffer ();
	m_strPhase.ReleaseBuffer ();

	CJuiceDlg::OnInitDialog();

	return TRUE;
}

void CModelDlg::OnOK()
{
	UpdateData (true);

	m_rSpeed = atof (m_strSpeed);
	m_rDelay = atof (m_strDelay);
	m_rPhase = atof (m_strPhase);

	CDialog::OnOK();
}

// ModelDlg.cpp ends here ->