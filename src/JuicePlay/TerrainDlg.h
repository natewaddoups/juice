/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: TerrainDlg.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

// CTerrainDlg dialog

class CTerrainDlg : public CJuiceDlg
{
	DECLARE_DYNAMIC(CTerrainDlg)

public:
	CTerrainDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTerrainDlg();

// Dialog Data
	enum { IDD = IDD_TERRAIN_NEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	void vBrowse (CString &strPath);

	DECLARE_MESSAGE_MAP()
public:

	virtual BOOL OnInitDialog();
	CString m_strElevationMap;
	CString m_strTextureCoarse;
	CString m_strTextureDetailed;
	float m_rScaleHorizontal;
	float m_rScaleVertical;
	UINT m_uTriangleCount;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBrowseElevation();
	afx_msg void OnBnClickedBrowseTextureCoarse();
	afx_msg void OnBnClickedBrowseTextureDetailed();
};

// TerrainDlg.h ends here ->