/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: ComponentsFrame.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "ModelTreeView.h"
#include "ComponentsFrame.h"

/****************************************************************************/
/** 
*****************************************************************************/
IMPLEMENT_DYNCREATE(CComponentsFrame, CMDIChildWnd)

/****************************************************************************/
/** 
*****************************************************************************/
CComponentsFrame::CComponentsFrame()
{
}

/****************************************************************************/
/** 
*****************************************************************************/
CComponentsFrame::~CComponentsFrame()
{
}

void CComponentsFrame::vSetModel (Model *pModel)
{
	if (m_pView)
		m_pView->vSetModel (pModel);
	else
		Breakpoint ("CComponentsFrame::vSetModel: null m_pView.");
}

/****************************************************************************/
/** 
*****************************************************************************/
BEGIN_MESSAGE_MAP(CComponentsFrame, CMDIChildWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CComponentsFrame message handlers

/****************************************************************************/
/** 
*****************************************************************************/
int CComponentsFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMenu *pSysMenu = GetSystemMenu (false);
	pSysMenu->DeleteMenu (SC_CLOSE, MF_BYCOMMAND);

	RECT rc;
	GetClientRect (&rc);

	// Create the view
	m_pView = new CModelTreeView ();
	m_pView->Create (NULL, "", WS_CHILD | WS_VISIBLE, rc, this, 0, NULL);

	return 0;
}

/****************************************************************************/
/** 
*****************************************************************************/
void CComponentsFrame::OnClose()
{
	// These windows must not respond to ctrl-F4
	// CMDIChildWnd::OnClose();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CComponentsFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIChildWnd::OnSize(nType, cx, cy);

	RECT rc;
	GetClientRect (&rc);
	m_pView->MoveWindow (&rc, TRUE);
}

// ComponentsFrame.cpp ends here ->