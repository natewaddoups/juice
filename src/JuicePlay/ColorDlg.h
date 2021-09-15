/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: ColorDlg.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once
#include "afxwin.h"
#include "JuiceDlg.h"

/****************************************************************************/
/** This dialog box allows the user to choose the colors with which different 
 ** UI elements are drawn.
*****************************************************************************/
class CColorDlg : public CJuiceDlg
{
public:
	/// Constructor
	CColorDlg(CWnd* pParent = NULL);

	/// Destructor
	virtual ~CColorDlg();

	/// Called when the dialog box is created
	virtual BOOL OnInitDialog();

	/// Called when the listbox selection changes
	afx_msg void OnLbnSelchangeList1();

	/// Called when the Choose Color button is pressed
	afx_msg void OnBnClickedChoosecolor();

	/// Called to redraw the dialog background (and the color sample rectangle)
	afx_msg void OnPaint();

protected:
	/// This listbox will show the names of the UI elements whose colors can 
	/// be changed via this dialog box.
	CListBox m_ColorList;

	/// This rectangle will be drawn with the color of the currently selected
	/// UI element.  The coordinates of this rectangle are taken from a hidden
	/// static control in the dialog box template.
	RECT m_rcColorSample;

	/// This method draws the currently selected color in the sample rectangle.
	void vDrawCurrentColor (CDC *pDC);

	/// Dialog box ID
	enum { IDD = IDD_COLORS };

	/// This points to the color currently being edited
	Generic::Color *m_pCurrentColor;
	
	/// MFC message map
	DECLARE_MESSAGE_MAP()

	/// MFC dialog control set/query 
	virtual void DoDataExchange(CDataExchange* pDX);

	// MFC dynamic creation
	DECLARE_DYNAMIC(CColorDlg)
};

// ColorDlg.h ends here ->