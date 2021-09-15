/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: MotionFrame.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "MainFrm.h"
#include "MotionView.h"
#include "NetworkView.h"
#include "ClockworkView.h"
#include "ModelTreeView.h"
#include "MotionFrame.h"

// CMotionFrame

IMPLEMENT_DYNCREATE(CMotionFrame, CMDIChildWnd)

CMotionFrame::CMotionFrame() :
	m_pCurrentView (null),
	m_pClockworkView (null),
	m_pNetworkView (null),
	m_pTreeView (null),
	m_fViewsCreated (false),
	m_pCurrentBehavior (null)
{
}

CMotionFrame::~CMotionFrame()
{
}

void CMotionFrame::vSetModel (Model *pModel)
{
	if (m_pTreeView)
		m_pTreeView->vSetModel (pModel);
	else
		Breakpoint ("CMotionFrame::vSetModel: null view pointer.");
}

void CMotionFrame::vAddMotion (Juice::ClockworkMotor *pMotion)
{
	if (pMotion->pGetBehavior () != m_pCurrentBehavior)
		vSetBehavior (pMotion->pGetBehavior ());

	if (m_pCurrentView)
		m_pCurrentView->vAddMotion (pMotion);
}

void CMotionFrame::vResetMotionList ()
{
	if (m_pCurrentView)
		m_pCurrentView->vResetMotionList ();
}

void CMotionFrame::vRemoveMotion (const Juice::ClockworkMotor *pMotion)
{
	if (m_pCurrentView)
		m_pCurrentView->vRemoveMotion (pMotion);
}

void CMotionFrame::vSetBehavior (Juice::Behavior *pBehavior)
{
	if (pBehavior == m_pCurrentBehavior)
		return;

	// delete current view
	if (m_pCurrentView)
	{
		m_wndSplitter.DeleteView (0, 1);
		m_pCurrentView = null;
	}

	if (!pBehavior)
		return;

	vRegisterForEventsFrom (pBehavior);

	// create new view of the appropriate type
	SIZE sizeInitial;

	sizeInitial.cx = sizeInitial.cy = 0;

	if (IsKindOfPtr <ClockworkBehavior*> (pBehavior))
	{
		m_wndSplitter.CreateView (0, 1, RUNTIME_CLASS (CClockworkView), sizeInitial, null);
		m_pCurrentView = m_pClockworkView = (CClockworkView *) m_wndSplitter.GetPane (0, 1);
	}

	if (IsKindOfPtr <NetworkBehavior*> (pBehavior))
	{
		m_wndSplitter.CreateView (0, 1, RUNTIME_CLASS (CNetworkView), sizeInitial, null);
		m_pCurrentView = m_pNetworkView = (CNetworkView *) m_wndSplitter.GetPane (0, 1);
		m_pNetworkView->vSetBehavior (pBehavior);
	}

	m_wndSplitter.RecalcLayout ();

	m_pCurrentBehavior = pBehavior;
}

void CMotionFrame::Notifier_Deleted (Behavior *pBehavior)
{
	// TODO: compare given pointer with m_pCurrentView->m_pBehavior

	// delete current view
	if (m_pCurrentView)
	{
		m_wndSplitter.DeleteView (0, 1);
		m_pCurrentView = null;
	}

	// insert a dummy/placeholder view
	SIZE sizeInitial;
	sizeInitial.cx = sizeInitial.cy = 0;
	m_wndSplitter.CreateView (0, 1, RUNTIME_CLASS (CClockworkView), sizeInitial, null);
	m_pCurrentView = m_pClockworkView = (CClockworkView *) m_wndSplitter.GetPane (0, 1);

	m_wndSplitter.RecalcLayout ();

	m_pCurrentBehavior = null;
}

void CMotionFrame::vOnTimer ()
{
	if (m_pCurrentView)
		m_pCurrentView->vOnTimer ();
}

BOOL CMotionFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	CString strCaption;
	strCaption.LoadString (IDS_CAPTION_MOTIONVIEW);
	SetWindowText (strCaption);

	m_wndSplitter.CreateStatic(this, 1, 2);

	SIZE sizeInitial = { 300, 0 };

	m_wndSplitter.CreateView (0, 0, RUNTIME_CLASS (CBehaviorTreeView), sizeInitial, pContext);
	m_pTreeView = (CBehaviorTreeView *) m_wndSplitter.GetPane (0, 0);

	// TODO: support multiple motion view classes
	m_wndSplitter.CreateView (0, 1, RUNTIME_CLASS (CClockworkView), sizeInitial, pContext);
	m_pCurrentView = m_pClockworkView = (CClockworkView *) m_wndSplitter.GetPane (0, 1);

	m_fViewsCreated = true;

	return true;
}

/****************************************************************************/
/* Overrides of BehaviorTreeView methods
*****************************************************************************/
void CBehaviorTreeView::vSelectMotor (ClockworkMotor *pMotion)
{
	// I'm still unsure as to whether or not this should use the selection set
	// operation in the base class... this is an experiment
	CModelTreeView::vSelectMotor (pMotion);

	// tell the parent frame that a new clockwork motor has been selected,
	// so it can update the other pane
	CMotionFrame *pFrame = (CMotionFrame*) (GetParent ()->GetParent ());
	pFrame->vAddMotion (pMotion);
}

void CBehaviorTreeView::vSelectBehavior (Behavior *pBehavior)
{
	// see above - this is an experiment
	CModelTreeView::vSelectBehavior (pBehavior);

	// tell the parent frame that a new behavior has been selected,
	// so it can update the other pane
	CMotionFrame *pFrame = (CMotionFrame*) (GetParent ()->GetParent ());
	pFrame->vSetBehavior (pBehavior);

	// this is not needed now that the base class method is being called
	//theApp.pGetMainWindow()->m_pProperties->vForceDisplayOf (pBehavior);
}

void CBehaviorTreeView::vResetSelectionSet ()
{
	// tell the parent about the reset
	CMotionFrame *pFrame = (CMotionFrame*) (GetParent ()->GetParent ());
	pFrame->vResetMotionList ();
}

void CBehaviorTreeView::Notifier_Deleted (Behavior *pBehavior)
{
	CModelTreeView::Notifier_Deleted (pBehavior);

	ClockworkBehavior *pClockworkBehavior = dynamic_cast <ClockworkBehavior*> (pBehavior);
	if (!pClockworkBehavior)
		return;

	CMotionFrame *pFrame = (CMotionFrame*) (GetParent ()->GetParent ());

	const ClockworkMotorList &lsMotions = pClockworkBehavior->lsGetMotors ();
	ClockworkMotorList::const_iterator iter;
	for (iter = lsMotions.begin (); iter != lsMotions.end (); iter++)
	{
		ClockworkMotor *pMotor = *iter;
		pFrame->vRemoveMotion (pMotor);
	}	
}

void CBehaviorTreeView::Notifier_Deleted (ClockworkMotor *pMotor)
{
	CModelTreeView::Notifier_Deleted (pMotor);
	CMotionFrame *pFrame = (CMotionFrame*) (GetParent ()->GetParent ());
	pFrame->vRemoveMotion (pMotor);
}

void CBehaviorTreeView::Selection_Remove (SelectionSet *pSet, Selectable *pSelectable)
{
	// tell the parent to remove the item
	CMotionFrame *pFrame = (CMotionFrame*) (GetParent ()->GetParent ());
	if (typeid (pSelectable) == typeid (ClockworkMotor*))
		pFrame->vRemoveMotion (dynamic_cast <ClockworkMotor*> (pSelectable));

	CModelTreeView::Selection_Remove (pSet, pSelectable);
}


BEGIN_MESSAGE_MAP(CMotionFrame, CMDIChildWnd)
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CMotionFrame message handlers

void CMotionFrame::OnClose()
{
	// remove self from main window's list
	((CMainFrame*)(theApp.m_pMainWnd))->m_MotionFrames.remove (this);

	CMDIChildWnd::OnClose();
}

void CMotionFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIChildWnd::OnSize(nType, cx, cy);

	// MFC will cause a WM_SIZE message to be created while the splitter
	// is being created.  If this window tries to resize the splitter before
	// the views have been created, the splitter pukes.

	if (m_fViewsCreated)
	{
		RECT rc;
		GetClientRect (&rc);

		m_wndSplitter.MoveWindow (&rc);
	}
}

// MotionFrame.cpp ends here ->