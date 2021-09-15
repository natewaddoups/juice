/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: NetworkView.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "MainFrm.h"
#include "PropertiesFrame.h"
#include "NetworkView.h"

using namespace Juice;
using namespace Generic;
using namespace std;

// CNetworkView

IMPLEMENT_DYNCREATE(CNetworkView, CView)

CNetworkView::CNetworkView() : 
	m_pGraph (null), 
	m_pCurrentBehavior (null), 
	m_pPickedNode (null)
{
}

CNetworkView::~CNetworkView()
{
	if (m_pCurrentBehavior)
		Generic::Listener<NetworkBehaviorEvents>::vUnregister ((NetworkBehavior*) m_pCurrentBehavior);

	if (m_pGraph)
		delete m_pGraph;
}

/*****************************************************************************
*****************************************************************************/

void CNetworkView::vSetBehavior (Juice::Behavior *pBehavior)
{
	if (m_pCurrentBehavior)
	{
		Generic::Listener<BehaviorEvents>::vUnregister ((Behavior*) m_pCurrentBehavior);
		Generic::Listener<NetworkBehaviorEvents>::vUnregister ((NetworkBehavior*) m_pCurrentBehavior);
	}

	m_pCurrentBehavior = pBehavior;

	if (!m_pCurrentBehavior)
		return;

	m_pGraph->vSetCallbacks (this); // dynamic_cast <GraphControl::Callbacks*> );

	Generic::Listener<BehaviorEvents>::vRegisterForEventsFrom ((Behavior*) m_pCurrentBehavior);
	Generic::Listener<NetworkBehaviorEvents>::vRegisterForEventsFrom ((NetworkBehavior*) m_pCurrentBehavior);

	((NetworkBehavior*) m_pCurrentBehavior)->vReportAllObjects ();
}

/*****************************************************************************
*****************************************************************************/

void CNetworkView::Behavior_Changed (Behavior *pBehavior)
{
	m_pGraph->vForceRedraw ();
}

void CNetworkView::Behavior_AddNode (NetworkBehavior *pBehavior, Juice::Node *pNewModelNode, bool fHasPosition)
{
	if (!pBehavior || !pNewModelNode)
		return;

	// create a matching node in the control
	Graph::Node *pNewViewNode = m_pGraph->pCreateNode ();
	if (!pNewViewNode)
	{
		Breakpoint ("CNetworkView::Behavior_AddNode: null node.");
		return;
	}

	pNewViewNode->vSetUserData ((Generic::UINT32) pNewModelNode);

	if (!fHasPosition)
		m_pGraph->vCenterNode (pNewViewNode);

	Generic::Listener<NodeEvents>::vRegisterForEventsFrom (pNewModelNode);
}

void CNetworkView::Behavior_AddEdge (NetworkBehavior *pBehavior, Juice::Edge *pNewModelEdge)
{
	if (!pBehavior || !pNewModelEdge)
		return;

	// Find the source and destination nodes in the control
	Graph::Node *pSourceNode = m_pGraph->pFindNodeWithUserData ((Generic::UINT32) pNewModelEdge->pGetSource ());
	Graph::Node *pDestinationNode = m_pGraph->pFindNodeWithUserData ((Generic::UINT32) pNewModelEdge->pGetDestination ());

	if (!pSourceNode || !pDestinationNode)
	{
		Breakpoint ("CNetworkView::Behavior_AddEdge: null edge source or destination.\n");
		return;
	}

	// Create the edge in the control
	Graph::Edge *pNewViewEdge = m_pGraph->pCreateEdge (pSourceNode, pDestinationNode);

	if (!pNewViewEdge)
	{
		Breakpoint ("CNetworkView::Behavior_AddEdge: null edge.");
		return;
	}

	pNewViewEdge->vSetUserData ((Generic::UINT32) pNewModelEdge);

	Generic::Listener<EdgeEvents>::vRegisterForEventsFrom (pNewModelEdge);
}

void CNetworkView::Behavior_RemoveNode (NetworkBehavior *pBehavior, Juice::Node *pDoomedNode)
{
	if (!m_pGraph)
		return;

	m_pGraph->vDeleteNode ((Generic::UINT32) pDoomedNode);
	Generic::Listener<NodeEvents>::vUnregister (pDoomedNode);
}

void CNetworkView::Behavior_RemoveEdge (NetworkBehavior *pBehavior, Edge *pDoomedEdge)
{
	if (!m_pGraph)
		return;

	m_pGraph->vDeleteEdge ((Generic::UINT32) pDoomedEdge);
	Generic::Listener<EdgeEvents>::vUnregister (pDoomedEdge);
}

/*****************************************************************************
*****************************************************************************/

void CNetworkView::Notifier_Deleted (Juice::Node *pNode)
{
	if (!m_pGraph)
		return;

	m_pGraph->vDeleteNode ((Generic::UINT32) pNode);
}

void CNetworkView::Notifier_Deleted (Juice::Edge *pEdge)
{
	if (!m_pGraph)
		return;

	m_pGraph->vDeleteEdge ((Generic::UINT32) pEdge);
}

void CNetworkView::Notifier_Deleted (Juice::NetworkBehavior *pBehavior)
{
	//if (!m_pGraph)
	//	return;

	//delete m_pGraph;
	//m_pGraph = null;

	m_pCurrentBehavior = null;
}

/*****************************************************************************
*****************************************************************************/

void CNetworkView::vOnCreateEdge  (Graph::Node *pSource, Graph::Node *pDestination)
{
	if (!pSource || !pDestination)
		return;

	Juice::Node *pSourceNode = (Juice::Node*) pSource->uGetUserData ();
	Juice::Node *pDestinationNode = (Juice::Node*) pDestination->uGetUserData ();

	if (!pSourceNode->fCanBeAnEdgeSource () || !pDestinationNode->fCanBeAnEdgeDestination ())
		return;

	Juice::Edge *pEdge = ((NetworkBehavior*) m_pCurrentBehavior)->pCreateEdge (pSourceNode, pDestinationNode);

	theApp.m_Juice.m_SelectionSet.vReset ();
	theApp.m_Juice.m_SelectionSet.vAdd (pEdge);
}

void CNetworkView::vOnPick (Graph::Node *pNode, bool fReset)
{
	m_pPickedNode = pNode;

	if (!pNode)
		return;

	Juice::Node *pModelNode = (Juice::Node*) pNode->uGetUserData ();

	if (!pModelNode)
		return;

	if (fReset)
		theApp.m_Juice.m_SelectionSet.vReset ();

	theApp.m_Juice.m_SelectionSet.vAdd (pModelNode);
}

void CNetworkView::vOnPick (Graph::Edge *pEdge, bool fReset)
{
	if (!pEdge)
		return;

	Juice::Edge *pModelEdge = (Juice::Edge*) pEdge->uGetUserData ();

	if (!pModelEdge)
		return;

	if (fReset)
		theApp.m_Juice.m_SelectionSet.vReset ();

	theApp.m_Juice.m_SelectionSet.vAdd (pModelEdge);
}

void CNetworkView::vOnPickNothing ()
{
	theApp.m_Juice.m_SelectionSet.vReset ();
	theApp.m_Juice.m_SelectionSet.vAdd (m_pCurrentBehavior);
}

/*****************************************************************************
*****************************************************************************/

void CNetworkView::vOnGetNodeRect (Graph::Node *pNode, Graph::NodeRect *pRect)
{
    if (!pRect)
		return;

	Juice::Node *pModelNode = (Juice::Node*) pNode->uGetUserData ();

	if (!pModelNode)
		return;

	pRect->iX = pModelNode->iGetX ();
	pRect->iY = pModelNode->iGetY ();

	pRect->uWidth = pModelNode->uGetWidth ();
	pRect->uHeight = pModelNode->uGetHeight ();
}

/****************************************************************************/
/** Reposition a node.  This is called both after the user has dragged a 
 ** selected node or set of nodes, or after after the user has repositioned
 ** the entire graph.
 **
 ** In the former case, fWholeGraph is set to false, and only the selected 
 ** nodes are moved.  In the latter case, fWholeGraph is set to true, and 
 ** the given node is repositioned whether it is selected or not.  The control
 ** will call this function repeatedly to reposition all of the nodes in the 
 ** graph.
*****************************************************************************/
void CNetworkView::vOnMoveNode (Graph::Node *pNode, int iX, int iY, bool fWholeGraph)
{
	if (!pNode)
		return;

	Juice::Node *pModelNode = (Juice::Node*) pNode->uGetUserData ();

	if (!pModelNode)
		return;

	int iDeltaX = iX - pModelNode->iGetX ();
	int iDeltaY = iY - pModelNode->iGetY ();

	const SelectableList &lsSelected = theApp.m_Juice.m_SelectionSet.lsGetContents ();
	SelectableList::const_iterator iter, last = lsSelected.end ();

	if (fWholeGraph)
	{
		pModelNode->vSetX (pModelNode->iGetX () + iDeltaX);
		pModelNode->vSetY (pModelNode->iGetY () + iDeltaY);
	}
	else
	{
		for (iter = lsSelected.begin (); iter != last; iter++)
		{
			Selectable *pSelectable = *iter;

			if (!pSelectable)
				continue;

			if (IsKindOfPtr <Juice::Node*> (pSelectable))
			{
				const Juice::Node *pConstNode = dynamic_cast <const Juice::Node*> (pSelectable);

				if (!pConstNode)
					continue;

				Juice::Node *pMoveMe = const_cast <Juice::Node*> (pConstNode);

				if (!pMoveMe)
					continue;

				pMoveMe->vSetX (pMoveMe->iGetX () + iDeltaX);
				pMoveMe->vSetY (pMoveMe->iGetY () + iDeltaY);
			}
		}
	}
}

void CNetworkView::vOnDrawBackground (DrawContext DC, Generic::UINT32 uWidth, Generic::UINT32 uHeight)
{
	CDC dc;
	dc.Attach (DC);

	CPen pnNode (PS_SOLID, 2, WinColorRef (theApp.m_Juice.m_Preferences.m_clrNetworkBackground));
	CPen *ppnOld = dc.SelectObject (&pnNode);	

	CBrush brNode (WinColorRef (theApp.m_Juice.m_Preferences.m_clrNetworkBackground));
	CBrush *pbrOld = dc.SelectObject (&brNode);

	dc.Rectangle (0, 0, uWidth, uHeight);

	dc.SelectObject (ppnOld);
	dc.SelectObject (pbrOld);	

	dc.Detach ();
}

CSize sizeGetTextRect (const char *szName, CDC &dc)
{
	CSize sizeResult;
	sizeResult.cx = sizeResult.cy = 0;

	const char *szLine = szName;

	// iterate through each line in the string
	while (lstrlen (szLine))
	{
		// find the end of this line
		const char *szEndOfLine = strchr (szLine, '\n');

		int nChars = 0;

		if (szEndOfLine)
			// find the length of this line
			nChars = ((int) szEndOfLine - (int) szLine);
		else
			nChars = lstrlen (szLine);

		// find the size of this line
		CSize sizeTmp = dc.GetTextExtent (szLine, nChars);

		// use the width if it's the biggest yet
		if (sizeTmp.cx > sizeResult.cx)
			sizeResult.cx = sizeTmp.cx;

		// add the height
		sizeResult.cy += sizeTmp.cy;

		if (!szEndOfLine)
			break;

		// get set to do the next line
		szLine = (const char*) ((int) szEndOfLine) + 1;
	}

	return sizeResult;
}

void vDrawText (const char *szName, CDC &dc, int iX, int iY)
{
	CString strTmp = szName;
	char *szLine = strTmp.GetBuffer ();

	// iterate through each line in the string
	while (lstrlen (szLine))
	{
		// find the end of this line
		char *szEndOfLine = strchr (szLine, '\n');

		// make the \n into a \0
		if (szEndOfLine)
			*szEndOfLine = '\0';

		// print the line
		dc.TextOut (iX, iY, szLine);

		// get set to do the next line
		CSize size = dc.GetTextExtent (szLine, lstrlen (szLine));

		if (!szEndOfLine)
			break;

		iY += size.cy;

		szLine = (char*) ((int) szEndOfLine) + 1;
	}
}

void CNetworkView::vOnDraw (DrawContext DC, Graph::Node *pViewNode, int iOffsetX, int iOffsetY)
{
	Juice::Node *pModelNode = (Juice::Node*) pViewNode->uGetUserData ();

	if (!pModelNode)
	{
		Breakpoint ("CNetworkView::vOnDraw: null pModelNode");
		return;
	}

	CDC dc;
	dc.Attach (DC);

	COLORREF clrPen, clrBrush, clrText;

	if (pModelNode->fSelected ())
	{
		clrBrush = WinColorRef (theApp.m_Juice.m_Preferences.m_clrNetworkNodeText);
		clrText = WinColorRef (theApp.m_Juice.m_Preferences.m_clrNetworkNode);
        clrPen = clrText; // WinColorRef (theApp.m_Juice.m_Preferences.m_clrNetworkNode);
	}
	else
	{
		clrBrush = WinColorRef (theApp.m_Juice.m_Preferences.m_clrNetworkNode);
		clrText = WinColorRef (theApp.m_Juice.m_Preferences.m_clrNetworkNodeText);
		clrPen = clrText; // WinColorRef (theApp.m_Juice.m_Preferences.m_clrNetworkNodeText);        
	}

	CPen pnNode (PS_SOLID, 1, clrPen);
	CPen *ppnOld = dc.SelectObject (&pnNode);	

	CBrush brNode (clrBrush);
	CBrush *pbrOld = dc.SelectObject (&brNode);
	
	CString strName = pModelNode->szGetName ();

	// get size of node text
	CSize size = sizeGetTextRect (strName, dc);

	// store the size in the node for future reference
	const int iExtraHeight = 4;
	const int iExtraWidth = 10;

	pModelNode->vSetHeight (size.cy + iExtraHeight);
	pModelNode->vSetWidth (size.cx + iExtraWidth);

	int iX = pModelNode->iGetX () - iOffsetX;
	int iY = pModelNode->iGetY () - iOffsetY;

	Generic::UINT32 uWidth = pModelNode->uGetWidth () / 2;
	Generic::UINT32 uHeight = pModelNode->uGetHeight () / 2;

	dc.Rectangle (iX - uWidth, iY - uHeight, iX + uWidth, iY + uHeight);

	dc.SetTextColor (clrText);
	dc.SetBkColor (clrBrush);

	vDrawText (strName, dc, iX - uWidth + (iExtraWidth / 2), iY - uHeight + ((iExtraHeight / 2) - 1));

	dc.SelectObject (ppnOld);
	dc.SelectObject (pbrOld);	

	dc.Detach ();
}

POINT operator- (POINT &pt1, POINT &pt2)
{
	POINT result = { pt1.x - pt2.x, pt1.y - pt2.y };
	return result;
}

void CNetworkView::vDrawEdge (DrawContext DC, POINT &ptStart, POINT &ptEnd, bool fSelected)
{
	CDC dc;
	dc.Attach (DC);

	COLORREF clrEdge;

	if (fSelected)
	{
		clrEdge = WinColorRef (theApp.m_Juice.m_Preferences.m_clrNetworkNodeBorder);
	}
	else
	{
		clrEdge = WinColorRef (theApp.m_Juice.m_Preferences.m_clrNetworkEdge);
	}

	POINT ptOneThird = { ((ptEnd.x - ptStart.x) / 3), ((ptEnd.y - ptStart.y) / 3) };
	POINT ptMidOneThird = { ptStart.x + ptOneThird.x, ptStart.y + ptOneThird.y };
	POINT ptMidTwoThirds = { ptMidOneThird.x + ptOneThird.x, ptMidOneThird.y + ptOneThird.y };

	CPen pnNode (PS_SOLID, 5, clrEdge);
	CPen *ppnOld = (CPen*) dc.SelectObject (&pnNode);

	dc.MoveTo (ptStart);
	dc.LineTo (ptMidOneThird);

	dc.SelectObject (ppnOld);
	pnNode.DeleteObject ();
	pnNode.CreatePen (PS_SOLID, 3, clrEdge);
	ppnOld = (CPen*) dc.SelectObject (&pnNode);

	dc.LineTo (ptMidTwoThirds);

	dc.SelectObject (ppnOld);
	pnNode.DeleteObject ();
	pnNode.CreatePen (PS_SOLID, 1, clrEdge);
	ppnOld = (CPen*) dc.SelectObject (&pnNode);

	dc.LineTo (ptEnd);

	dc.SelectObject (ppnOld);

	dc.Detach ();
}

void CNetworkView::vOnDrawNewEdge (DrawContext DC, Graph::Node *pViewSourceNode, int iX, int iY)
{
	if (!pViewSourceNode)
		return;

	Juice::Node *pSourceNode = (Juice::Node*) pViewSourceNode->uGetUserData ();

	if (!pSourceNode)
		return;

	POINT ptStart = { pSourceNode->iGetX (), pSourceNode->iGetY () };
	POINT ptEnd = { iX, iY };

	vDrawEdge (DC, ptStart, ptEnd, true);   
}

void CNetworkView::vOnDraw (DrawContext DC, Graph::Edge *pEdge, int iOffsetX, int iOffsetY)
{
	Juice::Edge *pModelEdge = (Juice::Edge*) pEdge->uGetUserData ();

	if (!pModelEdge)
	{
		Breakpoint ("CNetworkView::vOnDraw: null pModelEdge");
		return;
	}

	Generic::Graph::Node *pViewSourceNode =      pEdge->pGetSource ();
	Generic::Graph::Node *pViewDestinationNode = pEdge->pGetDestination ();

	if (!pViewSourceNode || !pViewDestinationNode)
	{
		Breakpoint ("CNetworkView::vOnDraw: null src/dst pointer in gui edge");
		return;
	}
    
	Juice::Node *pSourceNode = (Juice::Node*)     pViewSourceNode->uGetUserData ();
	Juice::Node *pDestinationNode = (Juice::Node*) pViewDestinationNode->uGetUserData ();

	if (!pSourceNode || !pDestinationNode)
	{
		Breakpoint ("CNetworkView::vOnDraw: bad src/dst node");
		return;
	}	

	POINT ptStart = { pSourceNode->iGetX (), pSourceNode->iGetY () };
	POINT ptEnd = { pDestinationNode->iGetX (), pDestinationNode->iGetY () };
	POINT ptOffset = { iOffsetX, iOffsetY };

	vDrawEdge (DC, ptStart - ptOffset, ptEnd - ptOffset, pModelEdge->fSelected ());
}

/*****************************************************************************
*****************************************************************************/

void CNetworkView::vOnUserDelete (Graph::Node *pNode)
{
	theApp.m_Juice.m_SelectionSet.vDelete ();
}

void CNetworkView::vOnUserDelete (Graph::Edge *pEdge)
{
	theApp.m_Juice.m_SelectionSet.vDelete ();
}

void CNetworkView::vOnAutoDelete  (Graph::Edge *pEdge)
{
//	Juice::Edge *pModelEdge = (Juice::Edge*) pEdge->uGetUserData ();
//	delete pModelEdge;
}

/*****************************************************************************
*****************************************************************************/

BEGIN_MESSAGE_MAP(CNetworkView, CView)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()
	//ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()


// CNetworkView drawing
void CNetworkView::OnDraw(CDC* pDC)
{
}

// CNetworkView diagnostics

#ifdef _DEBUG
void CNetworkView::AssertValid() const
{
	CView::AssertValid();
}

void CNetworkView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/*****************************************************************************
// CNetworkView message handlers
*****************************************************************************/

void CNetworkView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (!m_pGraph)
		return;

	m_pGraph->vResize (cx, cy);
}

int CNetworkView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pGraph = new Generic::Graph::Control;
	if (!m_pGraph)
		return -1;

	m_pGraph->Create (m_hWnd, this);

	DWORD dwStyle = GetStyle ();
	dwStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	::SetWindowLong (m_hWnd, GWL_STYLE, dwStyle);

	return 0;
}

// NetworkView.cpp ends here ->