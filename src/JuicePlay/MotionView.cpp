/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: MotionView.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "MotionView.h"


// CMotionView

IMPLEMENT_DYNCREATE(CMotionView, CView)

CMotionView::CMotionView()
{
}

CMotionView::~CMotionView()
{
}

BEGIN_MESSAGE_MAP(CMotionView, CView)
END_MESSAGE_MAP()


// CMotionView diagnostics

#ifdef _DEBUG
void CMotionView::AssertValid() const
{
	CView::AssertValid();
}

void CMotionView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


void CMotionView::OnDraw(class CDC *)
{
}
void CMotionView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	CSplitterWnd *pParent = (CSplitterWnd*) GetParent ();
	if (!pParent)
		return;

	int i, ix, iy;
	pParent->GetRowInfo (0, iy, i);
	pParent->GetColumnInfo (1, ix, i);

	RECT rc;
	pParent->GetClientRect (&rc);

	MoveWindow (rc.right - ix, 0, ix, iy, TRUE);
}

// MotionView.cpp ends here ->