/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: JuiceDlg.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

// CJuiceDlg dialog

class CJuiceDlg : public CDialog
{
	DECLARE_DYNAMIC(CJuiceDlg)

	CStatic *m_pHelpText;

public:
	CJuiceDlg(int iIDD, CWnd* pParent = NULL);   // standard constructor
	virtual ~CJuiceDlg();
	virtual BOOL OnInitDialog();
protected:
	DECLARE_MESSAGE_MAP()
	virtual void ControlSetFocus (UINT uControlID); // , NMHDR *pnm, LRESULT *pResult);
};

// JuiceDlg.h ends here ->