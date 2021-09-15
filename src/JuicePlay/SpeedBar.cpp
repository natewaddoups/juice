/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: SpeedBar.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "SpeedBar.h"


// CSpeedBar

IMPLEMENT_DYNAMIC(CSpeedBar, CControlBar)

CSpeedBar::CSpeedBar()
{
}

CSpeedBar::~CSpeedBar()
{
}

BEGIN_MESSAGE_MAP(CSpeedBar, CControlBar)
END_MESSAGE_MAP()

// CSpeedBar message handlers

void CSpeedBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
}

// SpeedBar.cpp ends here ->