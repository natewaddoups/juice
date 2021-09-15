/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: BehaviorView.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

// CBehaviorView view

class CBehaviorView : public CView
{
	DECLARE_DYNCREATE(CBehaviorView)

protected:
	CBehaviorView();           // protected constructor used by dynamic creation
	virtual ~CBehaviorView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
};

// BehaviorView.h ends here ->