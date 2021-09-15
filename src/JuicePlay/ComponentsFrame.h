/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: ComponentsFrame.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

using namespace Juice;

// CComponentsFrame frame

class CComponentsFrame : 
	public CMDIChildWnd
{
	DECLARE_DYNCREATE(CComponentsFrame)
protected:
	CComponentsFrame();           // protected constructor used by dynamic creation
	virtual ~CComponentsFrame();

	CModelTreeView *m_pView;

protected:
	DECLARE_MESSAGE_MAP()
public:
	void vSetModel (Model *pModel);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
};

// ComponentsFrame.h ends here ->