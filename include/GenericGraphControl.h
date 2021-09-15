/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenericGraphControl.h 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/
#ifndef __GenericGraphControl__
#define __GenericGraphControl__

#include <GenericNotifyListen.h>

namespace Generic {

/****************************************************************************/
/** Classes for the generic graph control 
*****************************************************************************/

namespace Graph {

/****************************************************************************/
/* The Graph::Control class provides an interface to a graph window, which 
 * can be used to display and edit directed graphs.  The Graph::Node and 
 * Graph::Edge classes provide interfaces to the node and edge objects in the
 * graph.  Graph::Node and Graph::Edge both derive from Graph::Object
*****************************************************************************/

class Control;
class Object;
class Node;
class Edge;


/****************************************************************************/
/** Nodes send no class-specific events, but a NodeEvents class is 
 ** implemented for the sake of the Notifier_Deleted event.
*****************************************************************************/
class NodeEvents : public Events<Node>
{
};	

/****************************************************************************/
/** Edges send no class-specific events, but the EdgeEvents class is
 ** implemented for the sake of the Notifier_Deleted event.
*****************************************************************************/
class EdgeEvents : public Events<Edge>
{
};


/****************************************************************************/
/** The NodeRect structure contains position and dimension information for
 ** a node.
*****************************************************************************/
typedef struct 
{
	int iX;
	int iY;
	Generic::UINT32 uWidth;
	Generic::UINT32 uHeight;
} NodeRect;

/****************************************************************************/
/** The Object class provides common functionality for the graph and edge 
 ** classes.  The key thing is a 32-bit 'user data' member, which can be used
 ** to store a pointer to the application object associated with the UI 
 ** object.
*****************************************************************************/

class GenericsExport Object
{
protected:
	Generic::UINT32 m_uUserData;
public:
	Object () : m_uUserData (0) {}
	void vSetUserData (UINT32 uData) {m_uUserData = uData; }
	Generic::UINT32 uGetUserData () {return m_uUserData; }
};

/****************************************************************************/
/** The node class is pretty trivial... The graph data structure itself does 
 ** not maintain position information for the nodes - that information is 
 ** maintained by the application object.
*****************************************************************************/

class GenericsExport Node : 
	public Generic::Notifier <NodeEvents>,
	public Object
{
public:
	Node ();
	~Node ();
};

/****************************************************************************/
/** The edge class keeps track of the edge's source and destination nodes.
*****************************************************************************/

class GenericsExport Edge : 
	public Generic::Notifier <EdgeEvents>,
	public Object
{
	Node *m_pSource, *m_pDestination;
public:
	Edge ();
	~Edge ();

	void vSetSource (Node *pSource);
	void vSetDestination (Node *pSource);

	Node* pGetSource ();
	Node* pGetDestination ();
};

/****************************************************************************/
/** The application can create a Graph::Control object to display and 
 ** manipulate a directed graph on the screen.
*****************************************************************************/

class GenericsExport Control : public Listener<NodeEvents>, public Listener <EdgeEvents>
{
public:

	/****************************************************************************/
	/** The code that uses the Graph::Control class must implement this callback
	 ** interface.  This will be used to notify the application of user interface
	 ** actions, to query the application for node position information, to 
	 ** request that the application draw graph objects, and so on.
	*****************************************************************************/
	class GenericsExport Callbacks
	{
	public:
		/// This notifies the application that an edge has been created between two nodes
		virtual void vOnCreateEdge (Node *pSource, Node *pDestination) {}

		/// This notifies the application that the user has picked a node
		virtual void vOnPick (Node *pNode, bool fResetSelectionSet) {}

		/// This notifies the application that the user has picked an edge
		virtual void vOnPick (Edge *pEdge, bool fResetSelectionSet) {}

		/// This notifies the application that the user has picked the graph background
		virtual void vOnPickNothing () {}

		/// This notifies the application that a node's position has changed 
		virtual void vOnMoveNode (Node *pNode, int iX, int iY, bool fMoveWholeGraph) {}

		/// This queries the application for node position and dimensions, for hit testing and auto-centering			
		virtual void vOnGetNodeRect (Node *pNode, NodeRect *pRect) {}
		
		virtual void vOnDoubleClick (Node *pNode) {}
		virtual void vOnDoubleClick (Edge *pEdge) {}
		virtual void vOnRightClick (Node *pNode) {}
		virtual void vOnRightClick (Edge *pEdge) {}

		/// This notifies the application that user has pressed delete key while a node was selected
		virtual void vOnUserDelete (Node *pNode) {}

		/// This notifies the application that user has pressed delete key while an edge was selected
		virtual void vOnUserDelete (Edge *pEdge) {}

		/// This notifies the application that an edges is to be deleted as result of node deletion.
		virtual void vOnAutoDelete (Edge *pEdge) {}

		/// This directs the application to redraw the graph's background
		virtual void vOnDrawBackground (DrawContext DC, Generic::UINT32 uWidth, Generic::UINT32 uHeight) {}

		/// This directs the application to draw an edge from a node to an arbitrary point on the graph
		virtual void vOnDrawNewEdge    (DrawContext DC, Graph::Node *pSource, int iX, int iY) {}

		/// This directs the application to draw a node with a given offset (offset is used when dragging the entire graph)
		virtual void vOnDraw           (DrawContext DC, Graph::Node *pNode, int iOffsetX, int iOffsetY) {}

		/// This directs the application to draw an edge with a given offset (offset is used when dragging the entire graph)
		virtual void vOnDraw           (DrawContext DC, Graph::Edge *pEdge, int iOffsetX, int iOffsetY) {}
	};

protected:

	/// This is the graph's list of nodes
	typedef std::list<Graph::Node*> NodeList;
	NodeList m_lsNodes;

	/// This is the graph's list of edges
	typedef std::list<Graph::Edge*> EdgeList;
	EdgeList m_lsEdges;

	/// This is a pointer to an application-supplied callback interface
	Callbacks *m_pCallbacks;

	/// The node most recently picked with the left mouse button - if this is non-null, a node is being dragged
	Node *m_pPickedNode;

	/// The edge most recently picked
	Edge *m_pPickedEdge;

	/// The node most recently picked with the right mouse button - if this is non-null, an edge is being created
	Node *m_pSourceNode;

	/// The distance between the node center and the location of the mouse pick
	int m_iPickOffsetX;

	/// The distance between the node center and the location of the mouse pick
	int m_iPickOffsetY;

	/// true graph coordinate = window coordinate + GraphOffset
	int m_iGraphOffsetX;

	/// window coordinate = true graph coordinate - GraphOffset 
	int m_iGraphOffsetY;

	/// Where the mouse clicked (used when dragging the whole graph)
	int m_iClickX, m_iClickY;

	/// Where the mouse was last seen - only valid during edge-creation gestures
	int m_iMouseX, m_iMouseY;

	/// sets m_pPickedNode or m_pPickedEdge as appropriate
	void vGetPick (Generic::INT32 iWindowX, Generic::INT32 iWindowY);

	/// Dimensions of the graph control
	Generic::UINT32 m_uWidth, m_uHeight;

	/// Helper function to find which node (if any) the user has picked with the mouse
	Control::Node *pGetPickNode (Generic::INT32 iX, Generic::INT32 iY);

	/// Helper function to find which edge (if any) the user has picked with the mouse
	Control::Edge *pGetPickEdge (Generic::INT32 iX, Generic::INT32 iY);

#ifdef _WINDOWS

	/// Windows-specific: name of the window class used for the graph control window
	static TCHAR *ms_szWindowClass;

	/// Windows-specific: indicates whether or not the graph control window class has yet been registered
	static bool g_fClassRegistered;

	/// Windows-specific: callback function for the graph control window
	static LRESULT WINAPI WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/// Windows-specific: onscreen and offscreen device contexts for painting
	HDC m_hWindowDC, m_hMemoryDC;

	/// Windows-specific: offscreen bitmap, for flicker-free redrawing
	HBITMAP m_hbmp;

	/// Windows-specific: handles to the graph control window and the control's parent window
	WindowType m_Window, m_ParentWindow;
#endif

	// The following callbacks are invokved in response to messages from the windowing system

	void OnDrawBackground (DrawContext DC);
	void OnDraw           (DrawContext DC);

	void OnMouseMove       (int iX, int iY, Generic::UINT32 uFlags);
	void OnLeftButtonDown  (int iX, int iY, Generic::UINT32 uFlags);
	void OnLeftButtonUp    (int iX, int iY, Generic::UINT32 uFlags);
	void OnRightButtonDown (int iX, int iY, Generic::UINT32 uFlags);
	void OnRightButtonUp   (int iX, int iY, Generic::UINT32 uFlags);
	void OnLeftButtonDoubleClick  (int iX, int iY, Generic::UINT32 uFlags);
	void OnRightButtonDoubleClick (int iX, int iY, Generic::UINT32 uFlags);
	void OnDelete ();

	/// Helper function to draw an edge that does not yet have a destination node assigned
	void OnDrawNewEdge    (DrawContext DC);

	/// Invoked when a node is deleted from the graph
	void Notifier_Deleted (Graph::Node *pNode);

	/// Invoked when an edge is deleted from the graph
	void Notifier_Deleted (Graph::Edge *pEdge);

public:

	/// Constructor
	Control ();

	/// Destructor
	~Control ();

	/// Create a graph control window on the screen
	WindowType Create (WindowType Parent, Control::Callbacks *pCallbacks);

	/// This is called by the application to resize the graph control window
	void vResize (Generic::UINT32 uWidth, Generic::UINT32 uHeight);

	/// This is called internally and/or by the application to force the graph to be redrawn
	void vForceRedraw ();

	/// The application calls this function to create a new node in the graph
	Node *pCreateNode ();

	/// The application calls this function to create a new edge in the graph
	Edge *pCreateEdge (Node *pSource, Node *pDestination);

	/// /// The application calls this function to delete a node from the graph
	void vDeleteNode (Generic::UINT32 uUserData);

	/// The application calls this function to delete an edge from the graph
	void vDeleteEdge (Generic::UINT32 uUserData);

	/// /// The application calls this function to get a pointer to the graph object with the given user data member
	Node *pFindNodeWithUserData (Generic::UINT32 uUserData);

	/// The application calls this function to get a pointer to the graph object with the given user data member
	Edge *pFindEdgeWithUserData (Generic::UINT32 uUserData);

	/// This repositions a node to the center of the graph
	void vCenterNode (Node *);

	/// This returns the (platform-specific) handle to the graph control window
	WindowType wtGetWindow () { return m_Window; }

	/// This sets the application-implemented callback interface
	void vSetCallbacks (Callbacks *pCallbacks);
};

}; // end Graph namespace
}; // end Generic namespace

#endif

// GenericGraphControl.h ends here ->