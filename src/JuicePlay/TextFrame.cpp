/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: TextFrame.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "TextFrame.h"


// CTextFrame

IMPLEMENT_DYNCREATE(CTextFrame, CMDIChildWnd)

CTextFrame::CTextFrame()
{
}

CTextFrame::~CTextFrame()
{
}


BEGIN_MESSAGE_MAP(CTextFrame, CMDIChildWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CTextFrame message handlers

int CTextFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CString strCaption;
	strCaption.LoadString (IDS_CAPTION_TEXTVIEW);
	SetWindowText (strCaption);

	return 0;
}

// TextFrame.cpp ends here ->