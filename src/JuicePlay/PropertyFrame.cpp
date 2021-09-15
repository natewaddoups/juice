// PropertyFrame.cpp : implementation file
//

#include "stdafx.h"
#include "JuicePlay.h"
#include "PropertyFrame.h"


// CPropertyFrame

IMPLEMENT_DYNCREATE(CPropertyFrame, CMDIChildWnd)

CPropertyFrame::CPropertyFrame()
{
}

CPropertyFrame::~CPropertyFrame()
{
}


BEGIN_MESSAGE_MAP(CPropertyFrame, CMDIChildWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CPropertyFrame message handlers

int CPropertyFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CString strCaption;
	strCaption.LoadString (IDS_CAPTION_PROPERTIESVIEW);
	SetWindowText (strCaption);

	return 0;
}
