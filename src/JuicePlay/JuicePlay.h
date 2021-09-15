/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: JuicePlay.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include <JuiceEngine.h>
#include "TrainingDlg.h"

class CMainFrame;

class CJuicePlayApp;

class CJuicePlayApp : public CWinApp
{
public:
	CJuicePlayApp();

//	OdeCleanup m_OdeCleanup;

	Juice::Application m_Juice;

	void vSetStatus (const char *szStatus);

	void vUpdateCommandUI ();

	bool fPromptIfChanged ();

	void vPauseAnimation ();
	void vResumeAnimation ();
	bool fAnimationPaused ();

	CMainFrame* pGetMainWindow () { return (CMainFrame*) m_pMainWnd; }

protected:
	HMENU m_hMDIMenu;
	HACCEL m_hMDIAccel;

	CString m_strFileName;
	CString m_strStatus;

	UINT m_uPauseAnimation;

	bool fOpenFile (const char *szPath);

	void vSetTitle ();

	void vUpdate3DViews ();

	CTrainingDlg m_TrainingDlg;

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	virtual BOOL OnIdle(LONG lCount);

	afx_msg BOOL OnOpenRecentFile(UINT nID);
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	afx_msg void OnDragXY();
	afx_msg void OnDragXZ();
	afx_msg void OnDragYZ();
	afx_msg void OnEnableMenuItem(CCmdUI *pCmdUI);

	afx_msg void OnUpdateAddAnything(CCmdUI *pCmdUI);
	afx_msg void OnAddBeam();
	afx_msg void OnAddHinge();
	afx_msg void OnAddSlider();
	afx_msg void OnEditAddBallJoint();
	afx_msg void OnAddClockworkBehavior();
	afx_msg void OnAddNetworkBehavior();

	afx_msg void OnUpdateModeSnap(CCmdUI *pCmdUI);
	afx_msg void OnModeSnap();

	afx_msg void OnUpdateEditSymmetricediting(CCmdUI *pCmdUI);
	afx_msg void OnEditSymmetricediting();

	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveas();
	afx_msg void OnEditCopy();

	afx_msg void OnUpdateEditNeedSelectionSet (CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditNeedClipboard(CCmdUI *pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditDelete();
	afx_msg void OnEditSelectall();
	afx_msg void OnViewLookatselection();
	afx_msg void OnUpdateModeDesign(CCmdUI *pCmdUI);
	afx_msg void OnUpdateModePause(CCmdUI *pCmdUI);
	afx_msg void OnModeDesign();
	afx_msg void OnModePause();
	afx_msg void OnModeTrain();
	afx_msg void OnUpdateModeTrain(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditAddmotion(CCmdUI *pCmdUI);
	afx_msg void OnOptionsColors();
	afx_msg void OnOptionsPhysicsParameters();
	afx_msg void OnOptionsModelParameters();
	afx_msg void OnOptionsChooseDynamicsLibrary ();
	afx_msg void OnOptionsGeneral ();
	afx_msg void OnModePlay();
	afx_msg void OnUpdateModePlay(CCmdUI *pCmdUI);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateModeFollow(CCmdUI *pCmdUI);
	afx_msg void OnModeFollow();
	afx_msg void OnHelpWebsite();

	afx_msg void OnUpdateTack (CCmdUI *pCmdUI);
	afx_msg void OnTack ();	
	afx_msg void OnGenerateSnake();
	afx_msg void OnGenerateManypede();
	afx_msg void OnFileImportJpm();
	afx_msg void OnTerrainEnable();
	afx_msg void OnUpdateTerrainEnable(CCmdUI *pCmdUI);
	afx_msg void OnTerrainNew();
	afx_msg void OnTerrainOpen();
	afx_msg void OnTerrainSaveas();
	afx_msg void OnUpdateTerrainSaveas(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTerrainNew(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTerrainOpen(CCmdUI *pCmdUI);
};

extern CJuicePlayApp theApp;

// JuicePlay.h ends here ->