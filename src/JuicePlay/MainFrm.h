/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: MainFrm.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>

#include "ModelTreeView.h"
#include "ClockworkView.h"

#include "MotionFrame.h"
#include "3DFrame.h"
#include "ComponentsFrame.h"
#include "PropertiesFrame.h"

#pragma once
class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
	CCmdUI *m_pDragStatus;
public:
	CMainFrame();

// Attributes
public:
	std::list <C3DFrame*> m_3DFrames;
	std::list <CMotionFrame*> m_MotionFrames;

	CComponentsFrame  *m_pComponents;
	CPropertiesFrame  *m_pProperties;

	Generic::UINT32 m_uLastTimerTick;
	Generic::UINT32 m_uFramesPerSecond;

	enum TimerID
	{
		tiInvalid,
		tiAnimation,
		tiUpdateCmdUI
	};

	void vDoAnimationFrame ();
	void vUpdateCommandUI ();


// Operations
public:
	void vCreateNew3dWindow ();
	void vCreateNewMotionWindow ();
	void vTileChildren ();
	CString m_strDragStatus;

	void CMainFrame::DockControlBarRightOf(CControlBar* Existing, CControlBar* New);

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBarMain;
	CToolBar    m_wndToolBarAppModes;
	CToolBar    m_wndToolBarEditModes;
	CToolBar    m_wndToolBarViews;
	CToolBar    m_wndToolBarAdd;
	CDialogBar  m_wndToolBarSpeed;

	CSliderCtrl m_SpeedSlider;
	CStatic		m_SpeedPercentage;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnViewNew3dwindow();
	afx_msg void OnViewNewmotionsourcewindow();
	afx_msg void OnUpdateViewNew3dwindow(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewNewmotionsourcewindow(CCmdUI *pCmdUI);
	afx_msg void OnUpdateDragStatus(CCmdUI *pCmdUI);
	afx_msg void OnCloseWindow();
	afx_msg void OnWindowTileHorz();
protected:
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
public:
	afx_msg void OnClose();
	afx_msg void OnToolbarsRestoreStandardLayout();
	afx_msg BOOL OnToolbar(UINT nID);
	afx_msg void OnUpdateToolbar(CCmdUI *pCmdUI);
protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnUpdateOptionsVideoCapture(CCmdUI *pCmdUI);
};

// MainFrm.h ends here ->