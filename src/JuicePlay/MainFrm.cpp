/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: MainFrm.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/


#include "stdafx.h"
#include "JuicePlay.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_NEW3DWINDOW, OnViewNew3dwindow)
	ON_COMMAND(ID_VIEW_NEWMOTIONSOURCEWINDOW, OnViewNewmotionsourcewindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEW3DWINDOW, OnUpdateViewNew3dwindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEWMOTIONSOURCEWINDOW, OnUpdateViewNewmotionsourcewindow)
	ON_UPDATE_COMMAND_UI(ID_STATUS_DRAG, OnUpdateDragStatus)
	ON_COMMAND(ID_WINDOW_TILE_HORZ, OnWindowTileHorz)
	ON_WM_CLOSE()
	ON_COMMAND(ID_TOOLBARS_RESTORESTANDARDLAYOUT, OnToolbarsRestoreStandardLayout)
	ON_UPDATE_COMMAND_UI_RANGE(IDR_TB_MAIN, IDR_TB_VIEWS, OnUpdateToolbar)
	ON_COMMAND_EX_RANGE (IDR_TB_MAIN, IDR_TB_VIEWS, OnToolbar)
	ON_WM_HSCROLL()
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_VIDEOCAPTURE, OnUpdateOptionsVideoCapture)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
//	ID_STATUS_DRAG,
	ID_STATUS_FRAMERATE,
	ID_INDICATOR_CAPS,
//	ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame() : 
	m_pDragStatus (null),
	m_uLastTimerTick (0),
	m_uFramesPerSecond (0)
{
}

CMainFrame::~CMainFrame()
{
}


/****************************************************************************/
/** Place toolbars side-by-side
*** From http://www.codejock.com/developer/article06.htm
*****************************************************************************/
void CMainFrame::DockControlBarRightOf(CControlBar* Existing, CControlBar* New)
{
    CRect rect;
    DWORD dw;
    UINT n;
    
    // get MFC to adjust the dimensions of all docked ToolBars
    // so that GetWindowRect will be accurate
    RecalcLayout(TRUE);
    
    Existing->GetWindowRect(&rect);
    rect.OffsetRect(1,0);
    dw=Existing->GetBarStyle();
    n = 0;
    n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
    n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
    n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
    n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;
    
    // When we take the default parameters on rect, DockControlBar will dock
    // each Toolbar on a seperate line. By calculating a rectangle, we
    // are simulating a Toolbar being dragged to that location and docked.
    DockControlBar(New,n,&rect);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rc (0,0,0,0);
	if (!m_wndToolBarMain.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, rc, IDR_TB_MAIN) ||
		!m_wndToolBarMain.LoadToolBar(IDR_TB_MAIN))
	{
		TRACE0("Failed to create main toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBarAppModes.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, rc, IDR_TB_APPMODES) ||
		!m_wndToolBarAppModes.LoadToolBar(IDR_TB_APPMODES))
	{
		TRACE0("Failed to create app mode toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBarEditModes.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, rc, IDR_TB_EDITMODES) ||
		!m_wndToolBarEditModes.LoadToolBar(IDR_TB_EDITMODES))
	{
		TRACE0("Failed to create edit mode toolbar\n");
		return -1;      // fail to create
	}
	
	if (!m_wndToolBarAdd.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, rc, IDR_TB_EDITMODES) ||
		!m_wndToolBarAdd.LoadToolBar(IDR_TB_ADD))
	{
		TRACE0("Failed to create add toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBarViews.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, rc, IDR_TB_VIEWS) ||
		!m_wndToolBarViews.LoadToolBar(IDR_TB_VIEWS))
	{
		TRACE0("Failed to create view toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndToolBarSpeed.Create (this, IDD_SPEEDBAR, CBRS_TOP |
		CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, IDR_TB_SPEED))
	{
		TRACE0("Failed to create speed toolbar\n");
		return -1;      // fail to create
	}

	// Initialize the speed bar
	m_SpeedSlider.Attach ((m_wndToolBarSpeed.GetDlgItem (IDC_SPEED))->m_hWnd);
	m_SpeedSlider.SetRange (-200, 200);
	m_SpeedSlider.SetPos (100);
	m_SpeedSlider.SetPageSize (10);

	m_SpeedPercentage.Attach ((m_wndToolBarSpeed.GetDlgItem (IDC_SPEED_PERCENT))->m_hWnd);;
	m_SpeedPercentage.SetWindowText ("100%");

	// Create the status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Create the dockable toolbar
	m_wndToolBarMain.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarAppModes.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarEditModes.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarAdd.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarViews.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBarSpeed.EnableDocking(CBRS_ALIGN_ANY);

	EnableDocking(CBRS_ALIGN_ANY);

	// Set default layout
	OnToolbarsRestoreStandardLayout();

	// Override with saved layout, if any
	LoadBarState ("JuiceTools");

	// Animation timer has been replaced with CWinApp::OnIdle
	//SetTimer (tiAnimation, 20, NULL);

	// Set the timer that will update "command" UI  
	SetTimer (tiUpdateCmdUI, 1000, NULL);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

void CMainFrame::vTileChildren ()
{
	RECT rc;
	::GetClientRect (m_hWndMDIClient, &rc);

	// Note the edges of where the 3D views will be
	int iLeftEdge = rc.right / 4;
	int iBottomEdge = (3 * rc.bottom) / 4;

	// Put the components view along the top of the left side
	if (m_pComponents)
		m_pComponents->MoveWindow (0, 0, iLeftEdge, rc.bottom / 2, TRUE);

	// Put the properties view on the bottom of the left side
	if (m_pProperties)
		m_pProperties->MoveWindow (0, rc.bottom / 2, iLeftEdge, rc.bottom / 2, TRUE);

	// Put the motion views on the lower margin
	std::list<CMotionFrame*>::iterator iter;
	for (iter = m_MotionFrames.begin (); iter != m_MotionFrames.end (); iter++)
	{
		if (*iter)
			(*iter)->MoveWindow (iLeftEdge, iBottomEdge, rc.right - iLeftEdge, rc.bottom / 4, TRUE);
	}

	RECT rc3D = { iLeftEdge, 0, rc.right, (rc.bottom * 3) / 4};

	// TODO: clean up the 3D view tiling code - generalize it so it's not all special cases

	if (m_3DFrames.size () == 1)
	{
		C3DFrame *p3D = m_3DFrames.front ();
		p3D->MoveWindow (&rc3D, TRUE);
	}
	else if (m_3DFrames.size () == 2)
	{
		std::list<C3DFrame*>::iterator iter = m_3DFrames.begin ();
		C3DFrame *p3D = *iter;
		if (!p3D)
			return;

		RECT rcFirst = { rc3D.left, 0, (rc.right / 2) + (rc3D.left / 2), rc3D.bottom };
        p3D->MoveWindow (&rcFirst, TRUE);

		iter++;
		p3D = *iter;
		if (!p3D)
			return;

		RECT rcSecond = { rcFirst.right, 0, rc.right, rc3D.bottom };
        p3D->MoveWindow (&rcSecond, TRUE);
	}
	else if (m_3DFrames.size () == 3)
	{
		std::list<C3DFrame*>::iterator iter = m_3DFrames.begin ();
		C3DFrame *p3D = *iter;
		if (!p3D)
			return;

		RECT rcFirst = { rc3D.left, 0, (rc.right / 2) + (rc3D.left / 2), rc3D.bottom };
        p3D->MoveWindow (&rcFirst, TRUE);

		iter++;
		p3D = *iter;
		if (!p3D)
			return;

		RECT rcSecond = { rcFirst.right, 0, rc.right, rc3D.bottom / 2};
        p3D->MoveWindow (&rcSecond, TRUE);

		iter++;
		p3D = *iter;
		if (!p3D)
			return;

		RECT rcThird = { rcFirst.right, rc3D.bottom / 2, rc.right, iBottomEdge };
        p3D->MoveWindow (&rcThird, TRUE);
	}		 
	else if (m_3DFrames.size () == 4)
	{
		std::list<C3DFrame*>::iterator iter = m_3DFrames.begin ();
		C3DFrame *p3D = *iter;
		if (!p3D)
			return;

		RECT rcFirst = { rc3D.left, 0, (rc.right / 2) + (rc3D.left / 2), rc3D.bottom / 2};
        p3D->MoveWindow (&rcFirst, TRUE);

		iter++;
		p3D = *iter;
		if (!p3D)
			return;

		RECT rcSecond = { rc3D.left, rc3D.bottom / 2, (rc.right / 2) + (rc3D.left / 2), rc3D.bottom };
        p3D->MoveWindow (&rcSecond, TRUE);

		iter++;
		p3D = *iter;
		if (!p3D)
			return;

		RECT rcThird = { rcFirst.right, 0, rc.right, rc3D.bottom / 2};
        p3D->MoveWindow (&rcThird, TRUE);

		iter++;
		p3D = *iter;
		if (!p3D)
			return;

		RECT rcFourth = { rcFirst.right, rc3D.bottom / 2, rc.right, iBottomEdge };
        p3D->MoveWindow (&rcFourth, TRUE);
	}		 
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers
void CMainFrame::vCreateNew3dWindow ()
{
	// TODO: investigate these last two parameters to CreateNewChild
	m_3DFrames.push_front ((C3DFrame*) CreateNewChild(
		RUNTIME_CLASS(C3DFrame), IDR_FRAME_3D, NULL, NULL));
}

void CMainFrame::vCreateNewMotionWindow ()
{
	CMotionFrame *pNew = (CMotionFrame*) CreateNewChild(
		RUNTIME_CLASS(CMotionFrame), IDR_FRAME_MOTION, NULL, NULL);

	pNew->vSetModel (&theApp.m_Juice.m_Model);

	m_MotionFrames.push_front (pNew);
}



void CMainFrame::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == tiAnimation)
		vDoAnimationFrame ();
	
	if (nIDEvent == tiUpdateCmdUI)
		vUpdateCommandUI ();

	// not sure what this does... probably nothing?
	CMDIFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::vDoAnimationFrame ()
{
	if (theApp.fAnimationPaused ())
		return;

	// compute time delay
	Generic::UINT32 uThisTimerTick = GetTickCount ();
	Generic::UINT32 uMilliseconds = uThisTimerTick - m_uLastTimerTick;

	// update the frame rate indicator?
	if (uThisTimerTick / 1000 != m_uLastTimerTick / 1000)
	{
		char szFPS[100];
		sprintf (szFPS, "%d FPS", m_uFramesPerSecond);
		m_wndStatusBar.SetPaneText (1, szFPS, TRUE);
		m_uFramesPerSecond = 0;
	}
	m_uFramesPerSecond++;


	// save this for next time
	m_uLastTimerTick = uThisTimerTick;

	// Update pull status (if any)
	if (theApp.m_Juice.fPulling ())
	{
		VectorXYZ vecPull;
		theApp.m_Juice.vGetPullVector (vecPull);
		char szStatus[200];

		sprintf (szStatus, "Force vector: (%1.3g, %1.3g, %1.3g)",
			vecPull.x, vecPull.y, vecPull.z);

		theApp.vSetStatus (szStatus);
	}
	else
	{
//		char szStatus[200];
//		sprintf (szStatus, "Refresh %dms, Phase %3d", uMilliseconds, (int) (theApp.m_Juice.m_Model.rGetPhase () * 100.0));
//		theApp.vSetStatus (szStatus);
	}

	if (uMilliseconds > (UINT) theApp.m_Juice.m_Preferences.m_rMaxDelta)
		uMilliseconds = (UINT) theApp.m_Juice.m_Preferences.m_rMaxDelta;

	if (uMilliseconds < (UINT) theApp.m_Juice.m_Preferences.m_rMinDelta)
		uMilliseconds = (UINT) theApp.m_Juice.m_Preferences.m_rMinDelta;

	if (uMilliseconds <= 0)
		uMilliseconds = 10;

	// Adjust camera positions, if that's what the user wants, unless a pull or drag is in progress
	if (((theApp.m_Juice.eGetAppMode () == Juice::amOperate) || (theApp.m_Juice.eGetAppMode () == Juice::amPose))&&
		(!theApp.m_Juice.fPulling () && !theApp.m_Juice.fDragging ()) && 
		theApp.m_Juice.m_Preferences.m_fFollow)
	{
		theApp.OnViewLookatselection ();

		std::list<C3DFrame*>::iterator iter;
		for (iter = m_3DFrames.begin (); iter != m_3DFrames.end (); iter++)
		{
			C3DFrame *pWnd = *iter;
			pWnd->vOnMainTimer ();
		}
	}

	// Step the world
	if (!theApp.m_Juice.fStep (uMilliseconds))
		theApp.vSetStatus ("Simulation error.");

	// update motion views
	std::list<CMotionFrame*>::iterator iter;
	for (iter = m_MotionFrames.begin (); iter != m_MotionFrames.end (); iter++)
	{
		(*iter)->vOnTimer ();
	}
}

void CMainFrame::vUpdateCommandUI ()
{
	theApp.vUpdateCommandUI ();
}

void CMainFrame::OnUpdateViewNew3dwindow(CCmdUI *pCmdUI)
{
	pCmdUI->Enable ();
}

void CMainFrame::OnViewNew3dwindow()
{
	vCreateNew3dWindow ();
}

void CMainFrame::OnUpdateViewNewmotionsourcewindow(CCmdUI *pCmdUI)
{
	pCmdUI->Enable ();
}

void CMainFrame::OnViewNewmotionsourcewindow()
{
	vCreateNewMotionWindow ();
}

void CMainFrame::OnCloseWindow()
{
	CMDIChildWnd* child = MDIGetActive();
	if (child && (child != m_pComponents) && (child != m_pProperties))
		child->MDIDestroy();
}


/****************************************************************************/
/** If an MDI child is mazimized, and then the MDI child windows are 
 ** "tiled" manually, it puts the MDI client into a broken state where 
 ** activating any MDI child will cause that child to be maximized.  
 **
 ** To get around this, WM_MDITILE needs to be sent to the client.  I'm still
 ** not sure how to get around the resulting flicker as the children are 
 ** tiled and then re-tiled.
 **
 ** Sending WM_MDITILE with MDITILE_SKIPDISABLED, helps a bit, but not enough.
 ****************************************************************************/
void CMainFrame::OnWindowTileHorz()
{
	// Not good.  See notes above.
	//CMDIFrameWnd::MDITile ();

	CFrameWnd *pFrame = GetActiveFrame ();

	if (pFrame)
		pFrame->EnableWindow (FALSE);

	::SendMessage(m_hWndMDIClient, WM_MDITILE, MDITILE_SKIPDISABLED, 0);

	if (pFrame)
		pFrame->EnableWindow (TRUE);

	vTileChildren ();
}

afx_msg void CMainFrame::OnUpdateDragStatus(CCmdUI *pCmdUI)
{
	m_pDragStatus = pCmdUI;
	pCmdUI->Enable ();
	pCmdUI->SetText (m_strDragStatus);
	((CStatusBar*) pCmdUI->m_pOther)->SetPaneStyle (1, SBPS_STRETCH);
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// Disable MFC's default title munging
	// CMDIFrameWnd::OnUpdateFrameTitle(bAddToTitle);
}
void CMainFrame::OnClose()
{
	if (!theApp.fPromptIfChanged ())
		return;

	m_SpeedSlider.Detach ();
	m_SpeedPercentage.Detach ();

	SaveBarState ("JuiceTools");
	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnUpdateToolbar(CCmdUI *pCmdUI)
{
	CControlBar* pBar = GetControlBar(pCmdUI->m_nID);
	if (pBar != NULL)
	{
		pCmdUI->SetCheck((pBar->GetStyle() & WS_VISIBLE) != 0);
		return;
	}
}

BOOL CMainFrame::OnToolbar(UINT nID)
{
	CControlBar* pBar = GetControlBar(nID);
	if (pBar != NULL)
	{
		ShowControlBar(pBar, (pBar->GetStyle() & WS_VISIBLE) == 0, FALSE);
		return TRUE;
	}

	return FALSE;
}

void CMainFrame::OnToolbarsRestoreStandardLayout()
{
	DockControlBar(&m_wndToolBarMain, AFX_IDW_DOCKBAR_TOP);
	DockControlBarRightOf (&m_wndToolBarMain,      &m_wndToolBarAppModes);
	DockControlBarRightOf (&m_wndToolBarAppModes,  &m_wndToolBarEditModes);
	DockControlBarRightOf (&m_wndToolBarEditModes, &m_wndToolBarAdd);
	DockControlBarRightOf (&m_wndToolBarAdd,       &m_wndToolBarViews);
	DockControlBarRightOf (&m_wndToolBarViews,     &m_wndToolBarSpeed);
}


BOOL CMainFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR *pnm = (NMHDR*) lParam;

	if (wParam == IDC_SPEED)
	{
		char szText[20];
		real rPosition = m_SpeedSlider.GetPos ();

		real rSpeed = rPosition / 100.0;

		theApp.m_Juice.m_Model.vSetSpeedFactor (rSpeed);

		wsprintf (szText, "%d%%\n", (int) (rSpeed * 100.0));
		m_SpeedPercentage.SetWindowText (szText);
	}

/*
	// wParam identifes the child that sent the message
	switch (wParam)
	{
	case IDC_SPEED:
		real rSpeed = theApp.m_Juice.m_Model.rGetSpeedPercentage ();

		switch (pnm->code)
		{
		case TB_TOP:
			rSpeed = 2;
			break;

		case TB_PAGEUP:
			rSpeed += 0.2;
			break;

		case TB_LINEUP:
			rSpeed += 0.1;
			break;

		case TB_LINEDOWN:
			rSpeed -= 0.1;
			break;

		case TB_PAGEDOWN:
			rSpeed -= 0.2;
			break;

		case TB_BOTTOM:
			rSpeed = 0;
			break;

		case TB_THUMBTRACK:
			rSpeed = ((real) m_SpeedSlider.GetPos ()) / 100.0;
			break;
		}

		if (rSpeed < 0)
			rSpeed = 0;
		if (rSpeed > 2)
			rSpeed = 20;

		theApp.m_Juice.m_Model.vSetSpeedPercentage (rSpeed);

		char szText[20];
		wsprintf (szText, "%d%%", (int) rSpeed * 100);
		m_SpeedPercentage.SetWindowText (szText);

		//m_SpeedSlider.SetPos ((int) (rSpeed * 100.0));
		break;
	}
*/
	return CFrameWnd::OnNotify(wParam, lParam, pResult);
}

void CMainFrame::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar->m_hWnd == m_SpeedSlider.m_hWnd)
	{
		real rSpeed = ((real) m_SpeedSlider.GetPos ()) / 100.0;

		theApp.m_Juice.m_Model.vSetSpeedFactor (rSpeed);

		char szText[20];
		wsprintf (szText, "%d%%", (int) rSpeed * 100);
		m_SpeedPercentage.SetWindowText (szText);
	}

	CMDIFrameWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMainFrame::OnUpdateOptionsVideoCapture(CCmdUI *pCmdUI)
{

#ifdef SupportVideo
	// enable if the active mdi child is a 3dframe
	CWnd *pWnd = GetActiveWindow ();
	if (pWnd->IsKindOf (RUNTIME_CLASS (C3DFrame)))
		pCmdUI->Enable (TRUE);
	else
		pCmdUI->Enable (FALSE);
#else
	CMenu *pMenu = GetMenu ();
	pMenu->RemoveMenu (ID_OPTIONS_VIDEOCAPTURE, MF_BYCOMMAND);
	CMenu *pOptions = pMenu->GetSubMenu (5);
	pOptions->RemoveMenu (3, MF_BYPOSITION);
#endif


}

// MainFrm.cpp ends here ->