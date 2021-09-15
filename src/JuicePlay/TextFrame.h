/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: TextFrame.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

// CTextFrame frame

class CTextFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CTextFrame)
protected:
	CTextFrame();           // protected constructor used by dynamic creation
	virtual ~CTextFrame();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

// TextFrame.h ends here ->