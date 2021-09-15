/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: 3DFrame.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "BehaviorView.h"


// CBehaviorView

IMPLEMENT_DYNCREATE(CBehaviorView, CView)

CBehaviorView::CBehaviorView()
{
}

CBehaviorView::~CBehaviorView()
{
}

BEGIN_MESSAGE_MAP(CBehaviorView, CView)
END_MESSAGE_MAP()


// CBehaviorView drawing

void CBehaviorView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


// CBehaviorView diagnostics

#ifdef _DEBUG
void CBehaviorView::AssertValid() const
{
	CView::AssertValid();
}

void CBehaviorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

// BehaviorView.cpp ends here ->