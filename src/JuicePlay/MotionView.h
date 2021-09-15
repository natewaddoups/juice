/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: MotionView.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

// CMotionView view

class CClockworkView;
class CNetworkView;

class CMotionView : public CView
{
	DECLARE_DYNCREATE(CMotionView)

protected:
	CMotionView();           // protected constructor used by dynamic creation
	virtual ~CMotionView();	

public:

	virtual void vAddMotion (Juice::ClockworkMotor *pMotion) {}
	virtual void vResetMotionList () {}
	virtual void vRemoveMotion (const Juice::ClockworkMotor *pMotion) {}
	virtual void vOnTimer () {}


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnDraw(CDC* pDC);

public:
	virtual void OnInitialUpdate();
};

// MotionView.h ends here ->