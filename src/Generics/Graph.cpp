/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: Graph.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#include "stdafx.h"
#include "Generics.h"
#include <GenericTypes.h>
#include <GenericError.h>
#include <GenericGraphControl.h>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

using namespace Generic;
using namespace Generic::Graph;

#ifdef _WINDOWS

/// Windows-specific: indicates whether or not the graph control window class has yet been registered
bool Control::g_fClassRegistered = false;

/// Windows-specific: name of the window class used for the graph control window
TCHAR * Control::ms_szWindowClass = "NateW.Generic.Graph.Control";

#endif

/****************************************************************************/
/** Constructor
 ** 
 ** Initializes all members to appropriate defaults, registers a window class
*****************************************************************************/
Control::Control () : 
	m_pCallbacks (null), 
	m_pPickedNode (null),
	m_pPickedEdge (null),
	m_pSourceNode (null),
	m_uWidth (0), 
	m_uHeight (0),
	m_iGraphOffsetX (0), m_iGraphOffsetY (0),
	m_iPickOffsetX (0), m_iPickOffsetY (0),
	m_iClickX (0), m_iClickY (0),
	m_iMouseX (0), m_iMouseY (0)
{
	if (!g_fClassRegistered)
	{
		// Register the viewport's window class
		WNDCLASS wc;

		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof (void*);
		wc.hbrBackground = (HBRUSH) GetStockObject (GRAY_BRUSH);
		wc.hCursor = LoadCursor (NULL, IDC_ARROW); 
		wc.hIcon = NULL;
		wc.hInstance = 0;
		wc.lpfnWndProc = WndProc;
		wc.lpszClassName = ms_szWindowClass;
		wc.lpszMenuName = NULL;
		wc.style = 0;

		if (!RegisterClass (&wc))
			return;

		g_fClassRegistered = true;
	}
}

/****************************************************************************/
/** Destructor
 **
 ** Deletes the nodes and edges from the lists.  Note that the lists'
 ** pop_xxx members are invoked from the Notifier_Deleted methods, which are
 ** invoked automagically as the nodes and edges are deleted.
*****************************************************************************/
Control::~Control ()
{
	while (m_lsEdges.size ())
	{
		Edge* pEdge = m_lsEdges.front ();
		delete pEdge;
		// This is actually done in Notifier_Deleted
		// m_lsEdges.pop_front ();
	}

	while (m_lsNodes.size ())
	{
		Node* pNode = m_lsNodes.front ();
		delete pNode;
		// This is actually done in Notifier_Deleted
		// m_lsNodes.pop_front ();
	}
}

/****************************************************************************/
/** Control::vSetCallbacks 
 **
 ** The application will probably prefer to set the callback interface pointer
 ** during the constructor, but this function is provided for completeness.
*****************************************************************************/
void Control::vSetCallbacks (Callbacks *pCallbacks)
{
	m_pCallbacks = pCallbacks;
}

/****************************************************************************/
/** Creates the graph control window, with the given parent window and 
 ** callback interface.
*****************************************************************************/
WindowType Control::Create (WindowType Parent, Callbacks *pCallbacks)
{
	m_Window = null;

	try
	{
		m_ParentWindow = Parent;

	#ifdef _WINDOWS

		RECT rcParent;
		::GetClientRect (m_ParentWindow, &rcParent);

		m_Window = CreateWindow (ms_szWindowClass, ms_szWindowClass, WS_CHILD | WS_VISIBLE, 
			rcParent.left, rcParent.top, rcParent.right, rcParent.bottom,
			m_ParentWindow, NULL, NULL, this);

		if (!m_Window)
			throw_Exception ( L"CreateWindow failed");
	#endif

	}
	catch (Exception e)
	{
		AddExceptionMessage (e, L"Unable to create graph control window");
		throw e;
	}    

	return m_Window;
}

/*****************************************************************************
*****************************************************************************/

#ifdef _WINDOWS

/****************************************************************************/
/** Control::WndProc 
 **
 ** @param hWnd Window handle
 ** @param uMsg Message ID
 ** @param wParam Message Parameter
 ** @param lParam Message Parameter
 **
 ** Handles messages from the windowing system.  Platform-specific code is
 ** kept within this function as much as possible.  Platform-independent 
 ** functionality is implemented in Graph::Control methods called from this
 ** function.
*****************************************************************************/
LRESULT WINAPI Control::WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// This is roughly equivalent to a 'this' pointer 
	Control *pControl = (Control*) GetWindowLong (hWnd, 0);

	// These are really only valid for mouse messages, but putting them here
	// makes the code less repetetive.
	int xPos = LOWORD (lParam); 
	int yPos = HIWORD (lParam); 

	// Window created
	if (uMsg == WM_CREATE)
	{
		// Get the Control pointer from the message, and store it in the window data
		CREATESTRUCT *pCreateStruct = (CREATESTRUCT*) lParam;

		// Error check
		if (!pCreateStruct)
			return -1;

		// Store the 'this' pointer in the window data
		SetWindowLong (hWnd, 0, (LONG) pCreateStruct->lpCreateParams);

		// Properly initialize pControl
		pControl = (Control*) GetWindowLong (hWnd, 0);

		// Create bitmap for offscreen drawing
		Generic::UINT32 uWidth = GetSystemMetrics (SM_CXSCREEN);
		Generic::UINT32 uHeight = GetSystemMetrics (SM_CYSCREEN);

		HDC hDC = GetDC (hWnd);
		pControl->m_hbmp = CreateCompatibleBitmap (hDC, uWidth, uHeight);
		ReleaseDC (hWnd, hDC);

		return 0;
	}

	// Window destroyed
	if (uMsg == WM_DESTROY)
	{
		// Just release the bitmap created during WM_CREATE
		if (pControl)
			DeleteObject (pControl->m_hbmp);
	}

	// All other messages are processed only if pControl is valid
	if (pControl)
	{
		if (uMsg == WM_ERASEBKGND)
		{
			// Get the DC
			HDC hDC = (HDC) wParam;

			// On second thought, this is unnecessary.

			// Indicate that the background has not been erased.
			return FALSE;
		}

		if (uMsg == WM_PAINT)
		{
			// Boilerplate code...
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint (hWnd, &ps);

			// Create and select a DC for offscreen drawing, using the bitmap
			HDC hOffscreenDC = CreateCompatibleDC (hDC);
			HBITMAP hbmp = (HBITMAP) SelectObject (hOffscreenDC, pControl->m_hbmp);

			// Draw the graph's background
			pControl->OnDrawBackground (hOffscreenDC);

			// If a new edge is being created, draw that
			if (pControl->m_pSourceNode)
				pControl->OnDrawNewEdge (hOffscreenDC);

			// Draw the edges and nodes
			pControl->OnDraw (hOffscreenDC);

			// Copy everything to the screen
			BitBlt (hDC, 0, 0, pControl->m_uWidth, pControl->m_uHeight,
				hOffscreenDC, 0, 0,SRCCOPY); 

			// Clean up
			SelectObject (hOffscreenDC, hbmp);
			DeleteDC (hOffscreenDC);
			EndPaint (hWnd, &ps);
			return 0;
		}

		if (uMsg == WM_LBUTTONDOWN)
		{
			SetCapture (hWnd);
			pControl->OnLeftButtonDown (xPos, yPos, wParam);
			return 0;
		}

		if (uMsg == WM_LBUTTONUP)
		{
			ReleaseCapture ();
			pControl->OnLeftButtonUp (xPos, yPos, wParam);
			return 0;
		}

		if (uMsg == WM_LBUTTONDBLCLK)
		{
			pControl->OnLeftButtonDoubleClick (xPos, yPos, wParam);
			return 0;
		}

		if (uMsg == WM_RBUTTONDOWN)
		{
			SetCapture (hWnd);
			pControl->OnRightButtonDown (xPos, yPos, wParam);
			return 0;
		}

		if (uMsg == WM_RBUTTONUP)
		{
			ReleaseCapture ();
			pControl->OnRightButtonUp (xPos, yPos, wParam);
			return 0;
		}

		if (uMsg == WM_RBUTTONDBLCLK)
		{
			pControl->OnRightButtonDoubleClick (xPos, yPos, wParam);
			return 0;
		}

		if (uMsg == WM_MOUSEMOVE)
		{
			pControl->OnMouseMove (xPos, yPos, wParam);
			return 0;
		}

		if (uMsg == WM_KEYDOWN)
		{
			if (wParam == VK_DELETE)
			{
				// Delete the currently selected object (assuming for now that there is one)
				// note: this never happens, because the accelerators nab the keystroke first
				pControl->OnDelete ();

				// Do not pass the message on to the parent window
				return FALSE;
			}

			// Do let parent window(s) process the keystrong
			return TRUE;
		}
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);
}


/****************************************************************************/
/** Control::vResize 
 **
 ** @param uWidth The new width of the control window
 ** @param uHeight The new height of the control window
*****************************************************************************/
void Control::vResize (Generic::UINT32 uWidth, Generic::UINT32 uHeight)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;

	::MoveWindow (m_Window, 0, 0, m_uWidth, m_uHeight, FALSE);
	::InvalidateRect (m_Window, NULL, TRUE);
}

/****************************************************************************/
/** Control::vForceRedraw
*****************************************************************************/
void Control::vForceRedraw ()
{
	::InvalidateRect (m_Window, NULL, TRUE);
}

#endif


/*****************************************************************************
The following methods are for platform-independent window-system message 
processing.  
*****************************************************************************/

/****************************************************************************/
/** Control::OnDrawBackground 
 **
 ** This is just a thin wrapper around the application callback.
*****************************************************************************/
void Control::OnDrawBackground (DrawContext DC)
{
	if (!m_pCallbacks)
		return;

	m_pCallbacks->vOnDrawBackground (DC, m_uWidth, m_uHeight);
}

/****************************************************************************/
/** Control::OnDraw 
 **
 ** This directs the application to draw each edge, the each node, on the 
 ** offscreen drawing buffer.  The platform-specific code in the calling
 ** function will then copy the offscreen buffer to the screen.
*****************************************************************************/
void Control::OnDraw (DrawContext DC)
{
	if (!m_pCallbacks)
		return;

	{
		EdgeList::iterator iter, last = m_lsEdges.end ();
		for (iter = m_lsEdges.begin (); iter != last; iter++)
		{
			Edge *pEdge = *iter;
			m_pCallbacks->vOnDraw (DC, pEdge, m_iGraphOffsetX, m_iGraphOffsetY);
		}
	}

	{
		NodeList::iterator iter, last = m_lsNodes.end ();
		for (iter = m_lsNodes.begin (); iter != last; iter++)
		{
			Node *pNode = *iter;
			m_pCallbacks->vOnDraw (DC, pNode, m_iGraphOffsetX, m_iGraphOffsetY);
		}
	}
}

/****************************************************************************/
/** Control::OnDrawNewEdge 
 **
 ** When the user is creating a new edge, that new edge must be drawn with
 ** 'special case' code.  The endpoints of an edge are normally derived from
 ** the source and destination nodes, but because the new edge does not yet 
 ** have a destination node, the endpoint must be specified explicitly.
*****************************************************************************/
void Control::OnDrawNewEdge (DrawContext DC)
{
	if (!m_pCallbacks || !m_pSourceNode)
		return;

    m_pCallbacks->vOnDrawNewEdge (DC, m_pSourceNode, m_iMouseX, m_iMouseY);
}

/****************************************************************************/
/** Control::OnMouseMove
 **
 ** Mouse-move messages are processed differently depending on whether the 
 ** user is dragging a single node, dragging an entire graph, or creating a
 ** new edge.
*****************************************************************************/
void Control::OnMouseMove       (int iX, int iY, Generic::UINT32 uFlags)
{
	if (!m_pCallbacks)
		return;

	if ((iX < 0) || (iY < 0) || (iX > (int) m_uWidth) || (iY > (int) m_uHeight))
		return;

	// if dragging a node...
	if (m_pPickedNode)
	{
		m_pCallbacks->vOnMoveNode (m_pPickedNode, 
				iX + (m_iGraphOffsetX - m_iPickOffsetX), 
				iY + (m_iGraphOffsetY - m_iPickOffsetY), false);
		vForceRedraw ();
	}

	// if dragging the whole graph...
	if (!m_pPickedNode && !m_pPickedEdge && (uFlags & MK_LBUTTON))
	{
		m_iGraphOffsetX += m_iClickX - iX;
		m_iGraphOffsetY += m_iClickY - iY;
		m_iClickX = iX;
		m_iClickY = iY;
		vForceRedraw ();
	}

	// if creating an edge
	if (m_pSourceNode && (uFlags & MK_RBUTTON))
	{
		m_iMouseX = iX;
		m_iMouseY = iY;
		vForceRedraw ();
	}
}

/****************************************************************************/
/** Control::OnLeftButtonDown
 **
 ** Left button clicks are used to select or deselect objects in the graph.
 ** In the general case, selecting a new object causes all other previously
 ** selected objects to be deselected.  If the control button is held down,
 ** the newly selected object is merely added to the set of already-selected
 ** objects.
*****************************************************************************/
void Control::OnLeftButtonDown  (int iX, int iY, Generic::UINT32 uFlags)
{
	if (!m_pCallbacks)
		return;

	// Note where this click happened - this will be used later to determine 
	// how much the mouse has moved.
	m_iClickX = iX;
	m_iClickY = iY;

	// Redraw the graph
	vForceRedraw ();

	// Figure out what got picked (if anything)
	vGetPick (iX, iY);

	// Figure out whether or not the select set should be emptied
	bool fReset = (bool) ! (uFlags & MK_CONTROL);

	// Notify the application of what, if anything, was picked...

	// A node...
	if (m_pPickedNode)
	{
		m_pCallbacks->vOnPick (m_pPickedNode, fReset);
		return;
	}

	// An edge...
	if (m_pPickedEdge)
	{
		m_pCallbacks->vOnPick (m_pPickedEdge, fReset);
		return;
	}

	// ...or nothing at all.
	m_pCallbacks->vOnPickNothing ();
}

/****************************************************************************/
/** Control::OnLeftButtonUp
 **
 ** This may signal the end of a dragging operation, perhaps for just one 
 ** node, perhaps for the whole graph.
*****************************************************************************/
void Control::OnLeftButtonUp    (int iX, int iY, Generic::UINT32 uFlags)
{
	if (!m_pPickedNode && !m_pPickedEdge)
	{
		// a whole-graph-drag operation just ended, so reposition all nodes
		NodeRect rcNode;
		Node *pNode = null;

		// Iterate over the list of nodes
		NodeList::iterator iter, last = m_lsNodes.end ();
		for (iter = m_lsNodes.begin (); iter != last; iter++)
		{
			pNode = *iter;

			if (!pNode)
				continue;

			// Get the node position
			m_pCallbacks->vOnGetNodeRect (pNode, &rcNode);

			// Adjust the postion based on how much the graph just moved
            rcNode.iX -= m_iGraphOffsetX;
			rcNode.iY -= m_iGraphOffsetY;

			// Set the node's new position
			m_pCallbacks->vOnMoveNode (pNode, rcNode.iX, rcNode.iY, true);
		}

		// Reset the graph offset
		m_iGraphOffsetX = m_iGraphOffsetY = 0;

		// Redraw the graph
		vForceRedraw ();		
	}

	// Note that nothing is picked any longer
	m_pPickedNode = null;
}

/****************************************************************************/
/** Control::OnRightButtonDown 
 **
 ** Right-button gestures are used to create new edges in the graph
*****************************************************************************/
void Control::OnRightButtonDown (int iX, int iY, Generic::UINT32 uFlags)
{
	if (!m_pCallbacks)
		return;

	// Find out what got picked...
	vGetPick (iX, iY);

	// ...if a node was picked, the user is about to create an edge
	if (m_pPickedNode)
	{
		// highlight the source node
		m_pCallbacks->vOnPick (m_pPickedNode, true);

		// note where this new edge begins, and where the mouse is at
		m_pSourceNode = m_pPickedNode;
		m_iMouseX = iX;
		m_iMouseY = iY;

		// Setting this to null indicates that the user is NOT trying to drag a node around
		m_pPickedNode = null;
	}
}

/****************************************************************************/
/** Control::OnRightButtonUp 
 **
 ** This may indicate that a new edge has been created.
*****************************************************************************/
void Control::OnRightButtonUp   (int iX, int iY, Generic::UINT32 uFlags)
{
	if (!m_pCallbacks)
		return;

	if (!m_pSourceNode)
		return;

	// Find out what object the pointer was over when the right button went up
	vGetPick (iX, iY);

	// If it was over a node other than the source node...
	if (m_pPickedNode && (m_pSourceNode != m_pPickedNode))
	{
		// ...create a new edge
		m_pCallbacks->vOnCreateEdge (m_pSourceNode, m_pPickedNode);

		// Reset the control's internal state w.r.t. mouse gestures
		m_pSourceNode = null;
		m_pPickedNode = null;
		m_iMouseX = 0;
		m_iMouseY = 0;
	}
	else
	{
		// If the mouse was over empty space in the graph, merely cancel the edge-creation process
		m_pSourceNode = null;
	}

	// Redraw the graph
	vForceRedraw ();
}

/****************************************************************************/
/** Control::OnLeftButtonDoubleClick  
 **
 ** Not implemented.
*****************************************************************************/
void Control::OnLeftButtonDoubleClick  (int iX, int iY, Generic::UINT32 uFlags)
{
}

/****************************************************************************/
/** Control::OnRightButtonDoubleClick
 **
 ** Not implemented.
*****************************************************************************/
void Control::OnRightButtonDoubleClick (int iX, int iY, Generic::UINT32 uFlags)
{
}

/****************************************************************************/
/** Control::OnDelete 
 **
 ** Delete the selected edge or node
*****************************************************************************/
void Control::OnDelete ()
{
	if (!m_pCallbacks)
		return;

	if (m_pPickedEdge)
	{
		m_pCallbacks->vOnUserDelete (m_pPickedEdge);
		delete m_pPickedEdge;
		m_pPickedEdge = null;
	}

	if (m_pPickedNode)
	{
		m_pCallbacks->vOnUserDelete (m_pPickedNode);
		delete m_pPickedNode;
		m_pPickedNode = null;
	}
}

/*****************************************************************************
That's the last of the window-system message processing methods.  
*****************************************************************************/


/****************************************************************************/
/** Control::Notifier_Deleted (Node*)
 **
 ** This is invoked when a node is deleted for any reason
*****************************************************************************/
void Control::Notifier_Deleted (Graph::Node *pNode)
{
	m_lsNodes.remove (pNode);
}

/****************************************************************************/
/** Control::Notifier_Deleted (Edge*)
 **
 ** This is invoked when an edge is deleted for any reason
*****************************************************************************/
void Control::Notifier_Deleted (Graph::Edge *pEdge)
{
	m_lsEdges.remove (pEdge);
}

/****************************************************************************/
/** Control::pGetPickNode 
 **
 ** This method returns a pointer to the node at the given coordinates
*****************************************************************************/
Graph::Node* Control::pGetPickNode (Generic::INT32 iX, Generic::INT32 iY)
{
	// iterate the node list, looking for a pick
	NodeList::iterator iter, last = m_lsNodes.end ();
	for (iter = m_lsNodes.begin (); iter != last; iter++)
	{
		Node *pNode = *iter;

		// Get the node's position and dimensions
		NodeRect rcNode;
		m_pCallbacks->vOnGetNodeRect (pNode, &rcNode);

		Generic::INT32 iNodeX = rcNode.iX;
		Generic::INT32 iNodeY = rcNode.iY;

		Generic::UINT32 uHalfWidth =  rcNode.uWidth / 2;
		Generic::UINT32 uHalfHeight = rcNode.uHeight / 2;

		if ((iX > (iNodeX - (int) uHalfWidth)) &&
			(iX < (iNodeX + (int) uHalfWidth)) &&
			(iY > (iNodeY - (int) uHalfHeight)) &&
			(iY < (iNodeY + (int) uHalfHeight)))
		{	
			// put the node at the end of the list, so it gets draw atop all others
			m_lsNodes.remove (pNode);
			m_lsNodes.push_back (pNode);

			// note the offset between the pick position and the node's center
			m_iPickOffsetX = iX - iNodeX;
			m_iPickOffsetY = iY - iNodeY;

			// return the picked node
			return pNode;
		}
	}

	// if nothing was picked, return null
	return null;
}

/****************************************************************************/
/** Control::pGetPickEdge
 **
 ** This method returns a pointer to the edge at the given coordinates
*****************************************************************************/
Graph::Edge* Control::pGetPickEdge (Generic::INT32 iX, Generic::INT32 iY)
{
	// iterate the edge list, looking for a pick
	EdgeList::iterator iter, last = m_lsEdges.end ();
	for (iter = m_lsEdges.begin (); iter != last; iter++)
	{
		Edge *pEdge = *iter;

		// Get the source and destination nodes
		Node *pSource = pEdge->pGetSource ();
		Node *pDestination = pEdge->pGetDestination ();

		if (!pSource || !pDestination)
		{
			Breakpoint ("Graph::Control::pGetPickEdge: null source or destination node\n");
			return null;
		}

		// Get the source and destination rectangles
		NodeRect rcSource, rcDestination;

		m_pCallbacks->vOnGetNodeRect (pSource, &rcSource);
		m_pCallbacks->vOnGetNodeRect (pDestination, &rcDestination);

		Generic::INT32 iSourceX = rcSource.iX;
		Generic::INT32 iSourceY = rcSource.iY;

		Generic::INT32 iDestinationX = rcDestination.iX;
		Generic::INT32 iDestinationY = rcDestination.iY;

		POINT pt1, pt2;
		float M, B;
		int iRise, iRun;
		Generic::UINT32 uWidth = 5;

		// Calculate slope of edge
		iRise = iSourceY - iDestinationY;
		iRun = iSourceX - iDestinationX;

		// Is the edge closer to horizontal, or vertical?
		if (abs (iRun) > abs (iRise))
		{
			// More horizonal - imagine a line 1 above the edge, and a line 2 below
			pt1.x = iSourceX;
			pt1.y = iSourceY - uWidth;

			pt2.x = iSourceX;
			pt2.y = iSourceY + uWidth;

			// is the mouse between the endpoints?
			if (!((iX > min (iSourceX, iDestinationX)) && // to right of left end
				(iX < max (iSourceX, iDestinationX))))    // to left of right end
				continue;

			// calculate the slope of the edge
			M = (float) (iSourceY - iDestinationY) / (float) (iSourceX - iDestinationX);

			// is the mouse below line 1?
			B = (float) pt1.y - M * pt1.x;
			if ((iY < M * iX + B))
				continue;

			// is the mouse above line 2?
			B = (float) pt2.y - M * pt2.x;
			if ((iY > M * iX + B))
				continue;

			// If all tests passed, then the user has picked this edge
			return pEdge;
		}
		else
		{
			// The edge in question is closer to vertical - imagine a line 1 right of edge, and a line 2 left
			pt1.x = iSourceX + uWidth;
			pt1.y = iSourceY;

			pt2.x = iSourceX - uWidth;
			pt2.y = iSourceY;

			// is the mouse between the endpoints of the edge?
			if (!((iY > min (iSourceY, iDestinationY)) &&  // below top
				(iY < max (iSourceY, iDestinationY))))     // above bottom
				continue;

			// Vertical lines are trivial, but risk divide by zero errors
			if (iSourceX == iDestinationX)
			{
				if ((iX < pt1.x) &&	// left of 1
					(iX > pt2.x))		// right of 2
					return pEdge;
				else
					continue;
			}
			// calculate slope
			M = (float) (iSourceY - iDestinationY) / (float) (iSourceX - iDestinationX);

			// is the mouse left of line 1?
			B = (float) pt1.y - M * pt1.x;
			if ((iX > (iY - B) / M))
				continue;

			// is the mouse right of line 2?
			B = (float) pt2.y - M * pt2.x;
			if ((iX < (iY - B) / M))
				continue;

			// If all tests passed, then the user has picked this edge
			return pEdge;
		}
	}

	// If control reaches this point, no edge was picked
	return null;
}

/****************************************************************************/
/** Control::vGetPick 
 **
 ** This function wraps the two functions above, testing first for an edge,
 ** then for a node that the user might have picked.  Internal state variables
 ** are set according to the results of the hit-test methods.
*****************************************************************************/
void Control::vGetPick (Generic::INT32 iWindowX, Generic::INT32 iWindowY)
{
	// convert window coords to true coords
	int iTrueX = iWindowX + m_iGraphOffsetX;
	int iTrueY = iWindowY + m_iGraphOffsetY;

	Edge *pPickedEdge = pGetPickEdge (iTrueX, iTrueY);
	Node *pPickedNode = pGetPickNode (iTrueX, iTrueY);

	if (pPickedNode)
	{
		m_pPickedNode = pPickedNode;
		m_pPickedEdge = null;
		return;
	}

	if (pPickedEdge)
	{
		m_pPickedNode = null;
		m_pPickedEdge = pPickedEdge;
		return;
	}

	m_pPickedEdge = null;
	m_pPickedNode = null;

	return;
}

/****************************************************************************/
/** Control::pCreateNode
 **
 ** The application calls this method to create a new node in the graph
*****************************************************************************/
Graph::Node* Control::pCreateNode ()
{
	// Create the new object
	Node *pNewNode = new Node ();

	// Register for 'deleted' events from the new object
	Generic::Listener<NodeEvents>::vRegisterForEventsFrom (pNewNode);

	// Add the new object to the graph
	m_lsNodes.push_back (pNewNode);

	// Redraw the graph
	vForceRedraw ();

	// Return a pointer to the new node
	return pNewNode;
}

/****************************************************************************/
/** Control::pCreateEdge 
 **
 ** The application calls this method to create a new edge in the graph
*****************************************************************************/
Graph::Edge* Control::pCreateEdge (Node *pSource, Node *pDestination)
{
	// Create the new object
	Edge *pNewEdge = new Edge ();

	// Register for 'deleted' events from the new object
	Generic::Listener<EdgeEvents>::vRegisterForEventsFrom (pNewEdge);

	// Set the new object's source and destination nodes
    pNewEdge->vSetSource (pSource);
	pNewEdge->vSetDestination (pDestination);

	// Add the new object to the lists
	m_lsEdges.push_front (pNewEdge);

	// Treat the new object as 'picked'
	m_pCallbacks->vOnPick (pNewEdge, true);

	// Redraw the graph
	vForceRedraw ();

	// Return a pointer to the new edge
	return pNewEdge;
}

/****************************************************************************/
/** Control::pFindNodeWithUserData 
 **
 ** This searches the graph's node list for a node containing the given
 ** user data.
*****************************************************************************/
Graph::Node* Control::pFindNodeWithUserData (Generic::UINT32 uUserData)
{
	// Iterate the node list
	NodeList::iterator iter, last = m_lsNodes.end ();
	for (iter = m_lsNodes.begin (); iter != last; iter++)
	{
		Node *pNode = *iter;

		// If the user data matches, return a pointer to this node
		if (pNode->uGetUserData () == uUserData)
			return pNode;
	}

	// If no match was found, return null
	return null;
}

/****************************************************************************/
/** Control::pFindEdgeWithUserData
 **
 ** This searches the graph's edge list for an edge containing the given
 ** user data.
*****************************************************************************/
Graph::Edge* Control::pFindEdgeWithUserData (Generic::UINT32 uUserData)
{
	// Iterate the edge list
	EdgeList::iterator iter, last = m_lsEdges.end ();
	for (iter = m_lsEdges.begin (); iter != last; iter++)
	{
		Edge *pEdge = *iter;

		// If the user data matches, return a pointer to this edge
		if (pEdge->uGetUserData () == uUserData)
			return pEdge;
	}

	// If no match was found, return null
	return null;
}

/****************************************************************************/
/** Control::vCenterNode
 **
 ** Reposition the given node to the center of the graph
*****************************************************************************/
void Control::vCenterNode (Node *pNode)
{
	// Calculate the new position
	Generic::INT32 iX = m_uWidth / 2;
	Generic::INT32 iY = m_uHeight / 2;

	// Move the node
	m_pCallbacks->vOnMoveNode (pNode, iX, iY, true);

	// Redraw the graph
	vForceRedraw ();
}


/****************************************************************************/
/** Control::vDeleteNode
 **
 ** Delete a node from the graph, given the node's user-data pointer.  Also 
 ** delete any edges that were attached to the deleted node.
*****************************************************************************/
void Control::vDeleteNode (Generic::UINT32 uUserData)
{
	Node *pNode = pFindNodeWithUserData (uUserData);

	if (!pNode)
		return;

	// Delete edges attached to this node - it's a two step process
	EdgeList lsEdgesToDelete;

	// first, build a list of edges to be deleted
	EdgeList::iterator iter, last = m_lsEdges.end ();
	for (iter = m_lsEdges.begin (); iter != last; iter++)
	{
		Edge *pEdge = *iter;

		if (pEdge->pGetSource () == pNode)
			lsEdgesToDelete.push_front (pEdge);

		if (pEdge->pGetDestination () == pNode)
			lsEdgesToDelete.push_front (pEdge);
	}

	// second, delete the edges
	while (lsEdgesToDelete.size ())
	{
		Edge *pEdge = lsEdgesToDelete.front ();
		if (m_pCallbacks)
			m_pCallbacks->vOnAutoDelete (pEdge);
		delete pEdge;
		lsEdgesToDelete.pop_front ();
	}

	// Finally, delete the node itself
	delete pNode;
}

/****************************************************************************/
/** Control::vDeleteEdge 
 **
 ** Delete an edge from the graph, given the edge's user-data pointer.  
*****************************************************************************/
void Control::vDeleteEdge (Generic::UINT32 uUserData)
{
	Edge *pEdge = pFindEdgeWithUserData (uUserData);

	if (!pEdge)
		return;

	delete pEdge;
}

// Graph.cpp ends here ->