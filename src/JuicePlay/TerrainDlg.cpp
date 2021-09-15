/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: TerrainDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "JuiceDlg.h"
#include "TerrainDlg.h"

using namespace Juice;

// CTerrainDlg dialog

IMPLEMENT_DYNAMIC(CTerrainDlg, CJuiceDlg)
CTerrainDlg::CTerrainDlg(CWnd* pParent /*=NULL*/)
	: CJuiceDlg(CTerrainDlg::IDD, pParent)
	, m_strElevationMap(_T(""))
	, m_strTextureCoarse(_T(""))
	, m_strTextureDetailed(_T(""))
	, m_rScaleHorizontal(0)
	, m_rScaleVertical(0)
	, m_uTriangleCount(0)
{
}

CTerrainDlg::~CTerrainDlg()
{
}

void CTerrainDlg::DoDataExchange(CDataExchange* pDX)
{
	CJuiceDlg::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TERRAIN_TEXTURE_ELEVATION, m_strElevationMap);
	DDX_Text(pDX, IDC_TERRAIN_TEXTURE_BASE, m_strTextureCoarse);
	DDX_Text(pDX, IDC_TERRAIN_TEXTURE_DETAILED, m_strTextureDetailed);
	DDX_Text(pDX, IDC_TERRAIN_SCALE_HORIZONTAL, m_rScaleHorizontal);
	DDX_Text(pDX, IDC_TERRAIN_SCALE_VERTICAL, m_rScaleVertical);
	DDX_Text(pDX, IDC_TERRAIN_TRIANGLECOUNT, m_uTriangleCount);
}


BEGIN_MESSAGE_MAP(CTerrainDlg, CJuiceDlg)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BROWSE_ELEVATION, OnBnClickedBrowseElevation)
	ON_BN_CLICKED(IDC_BROWSE_TEXTURE_COARSE, OnBnClickedBrowseTextureCoarse)
	ON_BN_CLICKED(IDC_BROWSE_TEXTURE_DETAILED, OnBnClickedBrowseTextureDetailed)
END_MESSAGE_MAP()


// CTerrainDlg message handlers

BOOL CTerrainDlg::OnInitDialog()
{
	CJuiceDlg::OnInitDialog ();

	theApp.m_Juice.m_Preferences.vLoad ();

	m_strElevationMap = theApp.m_Juice.m_Preferences.m_strTerrainElevation.c_str ();
	m_strTextureCoarse = theApp.m_Juice.m_Preferences.m_strTerrainTextureCoarse.c_str ();
	m_strTextureDetailed = theApp.m_Juice.m_Preferences.m_strTerrainTextureDetailed.c_str ();
	m_rScaleHorizontal = (float) theApp.m_Juice.m_Preferences.m_rTerrainScaleHorizontal;
	m_rScaleVertical = (float) theApp.m_Juice.m_Preferences.m_rTerrainScaleVertical;
	m_uTriangleCount = (UINT) theApp.m_Juice.m_Preferences.m_rTerrainTriangleCount;

	UpdateData (FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTerrainDlg::OnBnClickedOk()
{
	UpdateData (TRUE);

	theApp.m_Juice.m_Preferences.vSetTerrainElevation (m_strElevationMap);
	theApp.m_Juice.m_Preferences.vSetTerrainTextureCoarse (m_strTextureCoarse);
	theApp.m_Juice.m_Preferences.vSetTerrainTextureDetailed (m_strTextureDetailed);

	theApp.m_Juice.m_Preferences.m_rTerrainScaleHorizontal = m_rScaleHorizontal;
	theApp.m_Juice.m_Preferences.m_rTerrainScaleVertical = m_rScaleVertical;
	theApp.m_Juice.m_Preferences.m_rTerrainTriangleCount = (real) m_uTriangleCount;
	
	theApp.m_Juice.m_Preferences.vSave ();

	const char *szErrorMsg = null;
	int idControl = 0;

	switch (theApp.m_Juice.eTerrainNew (m_strElevationMap, m_strTextureCoarse, m_strTextureDetailed, m_rScaleHorizontal, m_rScaleVertical, m_uTriangleCount))
	{
	case tleUnableToSetElevation:
		szErrorMsg = "Unable to set terrain elevations";
		idControl = IDC_TERRAIN_TEXTURE_ELEVATION;
		break;

	case tleUnableToSetCoarseTexture:
		szErrorMsg = "Unable to load terrain coarse texture";
		idControl = IDC_TERRAIN_TEXTURE_BASE;
		break;

	case tleUnableToSetDetailedTexture:
		szErrorMsg = "Unable to load terrain detailed texture";
		idControl = IDC_TERRAIN_TEXTURE_DETAILED;
		break;

	case tleUnknown:
		szErrorMsg = "Something went wrong while loading the terrain";
		break;
	}

	if (szErrorMsg)
	{
		MessageBox (szErrorMsg, "Terrain Creation Error", MB_OK);

		if (idControl)
		{
			CWnd *pRelevantControl = GetDlgItem (idControl);

			if (pRelevantControl)
			{
				pRelevantControl->SetFocus ();
			}
		}
	}
	else
	{
		EndDialog (IDOK);
	}
}

void CTerrainDlg::vBrowse (CString &strPath)
{
	CFileDialog Dlg (TRUE);

	char szFileName[MAX_PATH];
	szFileName[0] = 0;

	lstrcpyn (szFileName, strPath, MAX_PATH);

	Dlg.m_ofn.lStructSize = sizeof (OPENFILENAME);
	Dlg.m_ofn.hwndOwner = m_hWnd;
	Dlg.m_ofn.lpstrFilter = "Image JPG\0*.jpg\0Image PNG\0*.png\0All Files\0*.*\0\0";
	Dlg.m_ofn.nFilterIndex = 1;
	Dlg.m_ofn.lpstrFile = szFileName;
	Dlg.m_ofn.nMaxFile = MAX_PATH;
	Dlg.m_ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER;

	// Display the dialog box 
	if (IDOK == Dlg.DoModal())
	{
		strPath = szFileName;

		UpdateData (FALSE);
	}
}

void CTerrainDlg::OnBnClickedBrowseElevation()
{
	vBrowse (m_strElevationMap);
}

void CTerrainDlg::OnBnClickedBrowseTextureCoarse()
{
	vBrowse (m_strTextureCoarse);
}

void CTerrainDlg::OnBnClickedBrowseTextureDetailed()
{
	vBrowse (m_strTextureDetailed);
}

// TerrainDlg.cpp ends here ->