/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: NetworkView.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once
#include "MotionView.h"
#include <GenericGraphControl.h>
// CNetworkView view

class CNetworkView : 
		public CMotionView, 
		public Generic::Graph::Control::Callbacks,
		public Generic::Listener <BehaviorEvents>,
		public Generic::Listener <NetworkBehaviorEvents>,
		public Generic::Listener <EdgeEvents>,
		public Generic::Listener <NodeEvents>
{
	DECLARE_DYNCREATE(CNetworkView)

protected:
	CNetworkView();           // protected constructor used by dynamic creation
	virtual ~CNetworkView();	

	Graph::Control *m_pGraph;
	Juice::Behavior *m_pCurrentBehavior;

	Graph::Node *m_pPickedNode;

	virtual void Behavior_Changed (Behavior *pBehavior);
	virtual void Behavior_AddNode (NetworkBehavior *pBehavior, Juice::Node *pNewNode, bool fHasPosition);
	virtual void Behavior_AddEdge (NetworkBehavior *pBehavior, Juice::Edge *pNewEdge);
	virtual void Behavior_RemoveNode (NetworkBehavior *pBehavior, Juice::Node *pDoomedNode);
	virtual void Behavior_RemoveEdge (NetworkBehavior *pBehavior, Juice::Edge *pDoomedEdge);

	virtual void Notifier_Deleted (Juice::Node *pNode);
	virtual void Notifier_Deleted (Juice::Edge *pEdge);
	virtual void Notifier_Deleted (Juice::NetworkBehavior *pBehavior);

	// GraphControl callbacks
	virtual void vOnCreateEdge  (Graph::Node *pSource, Graph::Node *pDestination);
	virtual void vOnMoveNode    (Graph::Node *pNode, int iX, int iY, bool fWholeGraph);
	virtual void vOnGetNodeRect (Graph::Node *pNode, Graph::NodeRect *pRect);
	virtual void vOnPick        (Graph::Node *pNode, bool fReset);
	virtual void vOnPick        (Graph::Edge *pEdge, bool fReset);
	virtual void vOnPickNothing ();
	virtual void vOnDoubleClick (Graph::Node *pNode) {}
	virtual void vOnDoubleClick (Graph::Edge *pEdge) {}
	virtual void vOnRightClick  (Graph::Node *pNode) {}
	virtual void vOnRightClick  (Graph::Edge *pEdge) {}

	virtual void vOnDrawBackground (DrawContext DC, Generic::UINT32 uWidth, Generic::UINT32 uHeight);
	virtual void vOnDrawNewEdge    (DrawContext DC, Graph::Node *pSource, int iX, int iY);
	virtual void vOnDraw           (DrawContext DC, Graph::Node *pNode, int iOffsetX, int iOffsetY);
	virtual void vOnDraw           (DrawContext DC, Graph::Edge *pEdge, int iOffsetX, int iOffsetY);

	virtual void vOnUserDelete  (Graph::Node *pNode);
	virtual void vOnUserDelete  (Graph::Edge *pEdge);
	virtual void vOnAutoDelete  (Graph::Edge *pEdge);

	// helper function for edge drawing
	void vDrawEdge (DrawContext DC, POINT &ptStart, POINT &ptEnd, bool fSelected);

public:

	void vSetBehavior (Juice::Behavior *pBehavior);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnDraw(CDC* pDC);

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

// NetworkView.h ends here ->