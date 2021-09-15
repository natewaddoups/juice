/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: GenerateSnakeDlg.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

// CGenerateSnakeDlg dialog

class CGenerateSnakeDlg : public CJuiceDlg
{
	DECLARE_DYNAMIC(CGenerateSnakeDlg)

public:
	CGenerateSnakeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGenerateSnakeDlg();

// Dialog Data
	enum { IDD = IDD_GENERATE_SNAKE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	Juice::Model *m_pModel;

	// Number of segments in the snake
	UINT m_uSegments;

	// Number of contact points at any one time
	UINT m_uConcurrentCycles;

	// Relative phase of the horizonal and vertical hinges
	int m_iPhaseOffset;

	// Range of the horizontal joints, in degrees
	INT m_iHorizontal;

	// Range of the vertical joints, in degrees
	INT m_iVertical;


	afx_msg void OnBnClickedOk();
};

// GenerateSnakeDlg.h ends here ->