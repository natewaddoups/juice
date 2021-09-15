/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: MotionFrame.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

// CMotionFrame frame with splitter

class CMotionView;
class CClockworkView;
class CNetworkView;

class CMotionFrame : public CMDIChildWnd, Listener <BehaviorEvents>
{
	DECLARE_DYNCREATE(CMotionFrame)
protected:
	CMotionFrame();           // protected constructor used by dynamic creation
	virtual ~CMotionFrame();
	CSplitterWnd m_wndSplitter;

	CBehaviorTreeView *m_pTreeView;

	CClockworkView *m_pClockworkView;	
	CNetworkView *m_pNetworkView;

	CMotionView *m_pCurrentView;	

	Behavior *m_pCurrentBehavior;

	/// This is a hack to stop MFC from puking 
	bool m_fViewsCreated;

	void Notifier_Deleted (Behavior *pBehavior);

public:
	void vSetModel (Model *pModel);

	virtual void vAddMotion (Juice::ClockworkMotor *pMotion);
	virtual void vResetMotionList ();
	virtual void vRemoveMotion (const Juice::ClockworkMotor *pMotion);

	virtual void vSetBehavior (Juice::Behavior *pBehavior);

	virtual void vOnTimer ();

protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

// MotionFrame.h ends here ->