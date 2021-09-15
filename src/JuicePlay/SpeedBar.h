/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: SpeedBar.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

// CSpeedBar

class CSpeedBar : public CControlBar
{
	DECLARE_DYNAMIC(CSpeedBar)

public:
	CSpeedBar();
	virtual ~CSpeedBar();
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	DECLARE_MESSAGE_MAP()
};

// SpeedBar.h ends here ->