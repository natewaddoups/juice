/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: NetworkBehavior.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>

using namespace Juice;

const wchar_t *NetworkBehavior::ms_wszXmlTag = L"NetworkBehavior";
const wchar_t *ArithmeticNode::ms_wszXmlTag = L"ArithmeticNode";
const wchar_t *WaveNode::ms_wszXmlTag = L"WaveNode";

Behavior* NetworkBehaviorCreator (Model *pModel, bool fLoadFromFile)
{
	return (Behavior*) new NetworkBehavior (pModel);
}

/*BehaviorClass NetworkBehaviorClass =
{
	NetworkBehaviorCreator,
	NetworkBehavior::ms_wszXmlTag
};*/

Node* ArithmeticNode::pCreate (NetworkBehavior *pBehavior, bool *pfAddToList)
{
	*pfAddToList = false;
	return (Node*) pBehavior->pCreateArithmeticNode ();
}

Node* WaveNode::pCreate (NetworkBehavior *pBehavior, bool *pfAddToList)
{
	*pfAddToList = false;
	return (Node*) pBehavior->pCreateWaveNode ();
}

Node* ConstantNode::pCreate (NetworkBehavior *pBehavior, bool *pfAddToList)
{
	*pfAddToList = false;
	return (Node*) pBehavior->pCreateConstantNode ();
}

Node* LimitNode::pCreate (NetworkBehavior *pBehavior, bool *pfAddToList)
{
	*pfAddToList = false;
	return (Node*) pBehavior->pCreateLimitNode ();
}

Node* AbsoluteValueNode::pCreate (NetworkBehavior *pBehavior, bool *pfAddToList)
{
	*pfAddToList = false;
	return (Node*) pBehavior->pCreateAbsoluteValueNode ();
}

Node* MultiplyNode::pCreate (NetworkBehavior *pBehavior, bool *pfAddToList)
{
	*pfAddToList = false;
	return (Node*) pBehavior->pCreateMultiplyNode ();
}

/****************************************************************************/
/** 
*****************************************************************************/
NetworkBehavior::NetworkBehavior (Model *pModel) : 
	Behavior (pModel, ms_wszXmlTag),
	m_lsFunctionNodes (this),
	m_lsMotorNodes (this),
	m_lsEdges (this),
	m_lsMotionSourceNodes (this)
{
	m_lsFunctionNodes.vAddClass (ArithmeticNode::ms_wszXmlTag, ArithmeticNode::pCreate);
	m_lsFunctionNodes.vAddClass (WaveNode::ms_wszXmlTag, WaveNode::pCreate);
	m_lsFunctionNodes.vAddClass (ConstantNode::ms_wszXmlTag, ConstantNode::pCreate);
	m_lsFunctionNodes.vAddClass (AbsoluteValueNode::ms_wszXmlTag, AbsoluteValueNode::pCreate);
	m_lsFunctionNodes.vAddClass (LimitNode::ms_wszXmlTag, LimitNode::pCreate);
	m_lsFunctionNodes.vAddClass (MultiplyNode::ms_wszXmlTag, MultiplyNode::pCreate);
}

/****************************************************************************/
/** 
*****************************************************************************/
NetworkBehavior::~NetworkBehavior ()
{
	while (m_lsMotionSourceNodes.size ())
	{
		Node *pNode = m_lsMotionSourceNodes.front ();
		delete pNode;

		// This is actually done in Notifier_Deleted
		//m_lsMotionSourceNodes.pop_front ();
	}

	while (m_lsMotorNodes.size ())
	{
		Node *pNode = m_lsMotorNodes.front ();
		delete pNode;

		// This is actually done in Notifier_Deleted
		//m_lsMotorNodes.pop_front ();
	}

	while (m_lsFunctionNodes.size ())
	{
		Node *pNode = m_lsFunctionNodes.front ();
		delete pNode;

		// This is actually done in Notifier_Deleted
		//m_lsFunctionNodes.pop_front ();
	}

	while (m_lsEdges.size ())
	{
		Edge *pEdge = m_lsEdges.front ();
		delete pEdge;

		// This is actually done in Notifier_Deleted
		//m_lsEdges.pop_front ();
	}

	vDiscardScript ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void NetworkBehavior::vAddNode (Node *pNode)
{
	if (IsKindOf <MotionSourceNode*> (pNode))
	{
		m_lsMotionSourceNodes.push_front ((MotionSourceNode*) pNode);
	}
	else if (IsKindOf <MotorNode*> (pNode))
	{
		m_lsMotorNodes.push_front ((MotorNode*) pNode);
	}
	else
	{
		m_lsFunctionNodes.push_front (pNode);
	}

	vNodeCreated (pNode, true);
}

/****************************************************************************/
/** 
*****************************************************************************/
NetworkBehavior& NetworkBehavior::operator= (const NetworkBehavior &src)
{
	Behavior::operator= (src);

	unordered_map <Edge*, Edge*> EdgeMap;
	unordered_map <Node*, Node*> NodeMap;

	// Duplicate the edges
	{
		EdgeList::const_iterator iter, last = src.m_lsEdges.end ();
		for (iter = src.m_lsEdges.begin (); iter != last; iter++)
		{
			Edge *pGivenEdge = *iter;

			if (!pGivenEdge)
			{
				Breakpoint ("Error in edge duplication loop");
				continue;
			}

			if (!pGivenEdge->pGetSource ())
			{
				Breakpoint ("Null source node in edge duplication loop");
				continue;
			}

			if (!pGivenEdge->pGetDestination ())
			{
				Breakpoint ("Null destination node in edge duplication loop");
				continue;
			}

			Selectable *pNewEdgeAsSelectable = pGivenEdge->pCreateClipboardCopy ();

			if (!pNewEdgeAsSelectable)
			{
				Breakpoint ("Unable to duplicate edge");
				continue;
			}

			Edge *pNewEdge = dynamic_cast <Edge*> (pNewEdgeAsSelectable);

			if (!pNewEdge)
			{
				delete pNewEdgeAsSelectable;
				Breakpoint ("bad pointer in NetworkBehavior::operator=");
				continue;
			}

			if (!pNewEdge->pGetSource ())
			{
				Breakpoint ("Null new source node in edge duplication loop");
				continue;
			}

			if (!pNewEdge->pGetDestination ())
			{
				Breakpoint ("Null new destination node in edge duplication loop");
				continue;
			}

			EdgeMap.insert (pair <Edge*, Edge*> (pGivenEdge, pNewEdge));
		}
	}

	// Make the new edges point to the new nodes
	{
		unordered_map <Edge*, Edge*>::iterator iter, last = EdgeMap.end ();
		for (iter = EdgeMap.begin (); iter != last; iter++)
		{
			pair <Edge*, Edge*> EdgePair = *iter;
			Edge *pOldEdge = EdgePair.first;
			Edge *pNewEdge = EdgePair.second;

			if (!pOldEdge)
			{
				Breakpoint ("NetworkBehavior::operator=: Null edge pointer in map");
				continue;
			}

			unordered_map <Node*, Node*>::iterator iterSource = NodeMap.find (pOldEdge->pGetSource ());
			unordered_map <Node*, Node*>::iterator iterDestination = NodeMap.find (pOldEdge->pGetDestination ());

			// Duplicate the source node
			if (iterSource == NodeMap.end ())
			{
				Node *pOldNode = pOldEdge->pGetSource ();

				if (!pOldNode)
				{
					Breakpoint ("NetworkBehavior::operator=: Null edge source pointer");
					continue;
				}

				Node *pNewNode = (Node*) pOldNode->pCreateClipboardCopy ();

				if (!pNewNode)
				{
					Breakpoint ("NetworkBehavior::operator=: Null edge source copy pointer");
					continue;
				}

				pNewEdge->vSetSource (pNewNode);
				NodeMap.insert (pair <Node*, Node*> (pOldNode, pNewNode));
			}
			else
			{
				pNewEdge->vSetSource ((*iterSource).second);
			}

			// Duplicate the destination node
			if (iterDestination == NodeMap.end ())
			{
				Node *pOldNode = pOldEdge->pGetDestination ();

				if (!pOldNode)
				{
					Breakpoint ("NetworkBehavior::operator=: Null edge destination pointer");
					continue;
				}

				Node *pNewNode = (Node*) pOldNode->pCreateClipboardCopy ();

				if (!pNewNode)
				{
					Breakpoint ("NetworkBehavior::operator=: Null edge destination copy pointer");
					continue;
				}

				pNewEdge->vSetDestination (pNewNode);
				NodeMap.insert (pair <Node*, Node*> (pOldNode, pNewNode));
			}
			else
			{
				pNewEdge->vSetDestination ((*iterDestination).second);
			}
		}
	}

	// Add the nodes
	{
		unordered_map <Node*, Node*>::iterator iter, last = NodeMap.end ();
		for (iter = NodeMap.begin (); iter != last; iter++)
		{
			pair <Node*, Node*> NodePair = *iter;
			Node *pNewNode = NodePair.second;
            
			vAddNode (pNewNode);
		}
	}

	// Add the edges
	{
		unordered_map <Edge*, Edge*>::iterator iter, last = EdgeMap.end ();
		for (iter = EdgeMap.begin (); iter != last; iter++)
		{
			pair <Edge*, Edge*> EdgePair = *iter;
			Edge *pNewEdge = EdgePair.second;
            
			m_lsEdges.push_front (pNewEdge);
		}
	}

	// Check integrity
	{
		NodeList lsNodes;
		lsNodes.insert (lsNodes.end (), m_lsMotionSourceNodes.begin (), m_lsMotionSourceNodes.end ());
		lsNodes.insert (lsNodes.end (), m_lsFunctionNodes.begin (), m_lsFunctionNodes.end ());
		lsNodes.insert (lsNodes.end (), m_lsMotorNodes.begin (), m_lsMotorNodes.end ());

		EdgeList::const_iterator iter, last = m_lsEdges.end ();
		for (iter = m_lsEdges.begin (); iter != last; iter++)
		{
			// Check edge
			Edge *pEdge = *iter;
			if (!pEdge)
			{
				Breakpoint ("NetworkBehavior::operator=: Null edge in check loop");
				continue;
			}

			// Check edge node pointers
			Node *pSource = pEdge->pGetSource ();
			Node *pDestination = pEdge->pGetDestination ();

			if (!pSource || !pDestination)
			{
				Breakpoint ("NetworkBehavior::operator=: Null node in check loop");
				continue;
			}

			// Check source
			if (!lsNodes.contains (pSource))
			{
				Breakpoint ("NetworkBehavior::operator=: Null motion source node in check loop");
				continue;
			}

			// Check destination
			if (!lsNodes.contains (pDestination))
			{
				Breakpoint ("NetworkBehavior::operator=: Null motion destination node in check loop");
				continue;
			}
		}
	}


/*

		// copy source and destination nodes
		Node *pGivenSourceNode = pNewEdge->pGetSource ();
		Node *pGivenDestinationNode = pNewEdge->pGetDestination ();
		Node *pNewSourceNode = null;
		Node *pNewDestinationNode = null;

		if (pGivenSourceNode)
			pNewSourceNode = dynamic_cast <Node*> (pGivenSourceNode->pCreateClipboardCopy ());

		if (pGivenDestinationNode)
			pNewDestinationNode = dynamic_cast <Node*> (pGivenDestinationNode->pCreateClipboardCopy ());

		if (pGivenSourceNode && pGivenDestinationNode)
		{
			// add nodes - this must happen before the edge is added, because 
			// when the view receivse the '_AddEdge' message, it verifies the
			// existence of the nodes referenced by the edges
			vAddNode (pNewSourceNode);			
			vAddNode (pNewDestinationNode);

			// update new edge
			pNewEdge->vSetSource (pNewSourceNode);
			pNewEdge->vSetDestination (pNewDestinationNode);

			// add new edge 
			m_lsEdges.push_front (pNewEdge);
			vEdgeCreated (pNewEdge);
		}
		else
		{
			delete pNewSourceNode;
			delete pNewDestinationNode;
		}
	}
*/
	return *this;
}

/****************************************************************************/
/** 
*****************************************************************************/
Selectable* NetworkBehavior::pCreateClipboardCopy () const
{
	NetworkBehavior *pNew = new NetworkBehavior (m_pModel);

//	pNew->vSetName (szGetName ());
	
	*pNew = *this;
	
	return (Selectable*) pNew;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool NetworkBehavior::fPaste (Selectable *pNewObject)
{
	NetworkBehavior *pNewNetworkBehavior = dynamic_cast <NetworkBehavior*> (pNewObject);
	if (pNewNetworkBehavior)
	{
		*this = *pNewNetworkBehavior;
		return true;
	}

	MotionSourceNode *pMotionSource = dynamic_cast <MotionSourceNode*> (pNewObject);
	if (pMotionSource)
	{
		m_lsMotionSourceNodes.push_front (pMotionSource);
		vNodeCreated (pMotionSource, true);
		return true;
	}

	ArithmeticNode *pArithmetic = dynamic_cast <ArithmeticNode*> (pNewObject);
	if (pArithmetic)
	{
		m_lsFunctionNodes.push_front (pArithmetic);
		vNodeCreated (pArithmetic, true);
		return true;
	}

	ConstantNode *pConstant = dynamic_cast <ConstantNode*> (pNewObject);
	if (pConstant)
	{
		m_lsFunctionNodes.push_front (pConstant);
		vNodeCreated (pConstant, true);
		return true;
	}

	AbsoluteValueNode *pAbsolute = dynamic_cast <AbsoluteValueNode*> (pNewObject);
	if (pConstant)
	{
		m_lsFunctionNodes.push_front (pAbsolute);
		vNodeCreated (pAbsolute, true);
		return true;
	}

	LimitNode *pLimit = dynamic_cast <LimitNode*> (pNewObject);
	if (pConstant)
	{
		m_lsFunctionNodes.push_front (pLimit);
		vNodeCreated (pLimit, true);
		return true;
	}

	MultiplyNode *pMultiply = dynamic_cast <MultiplyNode*> (pNewObject);
	if (pMultiply)
	{
		m_lsFunctionNodes.push_front (pMultiply);
		vNodeCreated (pMultiply, true);
		return true;
	}

	WaveNode *pWave = dynamic_cast <WaveNode*> (pNewObject);
	if (pWave)
	{
		m_lsFunctionNodes.push_front (pWave);
		vNodeCreated (pWave, true);
		return true;
	}

	MotorNode *pMotor = dynamic_cast <MotorNode*> (pNewObject);
	if (pMotor)
	{
		m_lsMotorNodes.push_front (pMotor);
		vNodeCreated (pMotor, true);
		return true;
	}

	Edge *pEdge = dynamic_cast <Edge*> (pNewObject);
	if (pEdge)
	{
		if (pEdge->pGetSource () && pEdge->pGetDestination ())
		{
			m_lsEdges.push_front (pEdge);
			vEdgeCreated (pEdge);
			return true;
		}
		else
		{
			delete pEdge;
			return true;
		}
	}

	return false;
}

/****************************************************************************/
/** 
*****************************************************************************/
void NetworkBehavior::vResetPointers (BodyHashMap &BodyMap, JointHashMap &JointMap)
{
	Behavior::vResetPointers (BodyMap, JointMap);

	// point motion source nodes at bodies
	MotionSourceNodeList::iterator iter, last = m_lsMotionSourceNodes.end ();
	for (iter = m_lsMotionSourceNodes.begin (); iter != last; iter++)
	{
		MotionSourceNode *pNode = *iter;
		BodyHashMap::iterator iter = BodyMap.find ((Generic::UINT32) pNode->pGetBody ());

		if (iter == BodyMap.end ())
			pNode->vSetBody (null);
		else
			pNode->vSetBody ((*iter).second);
	}

	// point motion source nodes at joints
	// (to do)

	// point edges at nodes - first, build a map
	NodeHashMap NodeMap;

	
	{
		MotionSourceNodeList::iterator iter, last = m_lsMotionSourceNodes.end ();
		for (iter = m_lsMotionSourceNodes.begin (); iter != last; iter++)
		{
			Node *pNode = *iter;
			NodeMap.insert (pair <Generic::UINT32, Node*> (pNode->uGetID (), pNode));
		}
	}
	{
		FunctionNodeList::iterator iter, last = m_lsFunctionNodes.end ();
		for (iter = m_lsFunctionNodes.begin (); iter != last; iter++)
		{
			Node *pNode = *iter;
			NodeMap.insert (pair <Generic::UINT32, Node*> (pNode->uGetID (), pNode));
		}
	}
	{
		MotorNodeList::iterator iter, last = m_lsMotorNodes.end ();
		for (iter = m_lsMotorNodes.begin (); iter != last; iter++)
		{
			Node *pNode = *iter;
			NodeMap.insert (pair <Generic::UINT32, Node*> (pNode->uGetID (), pNode));
		}
	}

	// fix edge->node pointers
	m_lsEdges.vSetNodePointers (NodeMap);

	// point motor nodes at joints
	{
		MotorNodeList::iterator iter, last = m_lsMotorNodes.end ();
		for (iter = m_lsMotorNodes.begin (); iter != last; iter++)
		{
			MotorNode *pNode = *iter;

			JointHashMap::iterator iter = JointMap.find ((Generic::UINT32) pNode->pGetJoint ());

			if (iter == JointMap.end ())
			{
				pNode->vSetJoint (null);
			}
			else
			{
				pNode->vSetJointNull ();
				pNode->vSetJoint ((*iter).second);            			
			}
		}
	}
}

/****************************************************************************/
/** 
*****************************************************************************/

void NetworkBehavior::Notifier_Deleted (Node *pNode)
{
	m_lsMotionSourceNodes.remove ((MotionSourceNode*) pNode);
	m_lsMotorNodes.remove ((MotorNode*) pNode);
	m_lsFunctionNodes.remove (pNode);

	// Find edges connected to this node
	EdgeList lsEdgesToDelete (this);
	EdgeList::iterator iter, last = m_lsEdges.end ();	

	for (iter = m_lsEdges.begin (); iter != last; iter++)
	{
		Edge *pEdge = *iter;

		if (pEdge->pGetSource () == pNode)
			lsEdgesToDelete.push_front (pEdge);

		if (pEdge->pGetDestination () == pNode)
			lsEdgesToDelete.push_front (pEdge);
	}

	// Delete those edges.  This is done in a separate loop because "iter" 
	// gets hosed if the delete happens during the iteration above.
	while (lsEdgesToDelete.size ())
	{
		Edge *pEdge = lsEdgesToDelete.front ();
		delete pEdge;
		lsEdgesToDelete.pop_front ();
		m_lsEdges.remove (pEdge);
	}

	Generic::Notifier<BehaviorEvents>::vNotify (&Juice::BehaviorEvents::Behavior_Changed);
}

void NetworkBehavior::Notifier_Deleted (Edge *pEdge)
{
	m_lsEdges.remove (pEdge);

	vCompileScript ();

	Generic::Notifier<BehaviorEvents>::vNotify(&Juice::BehaviorEvents::Behavior_Changed);
}

void NetworkBehavior::Node_Changed (Node *pChanged)
{
	Generic::Notifier<BehaviorEvents>::vNotify(&Juice::BehaviorEvents::Behavior_Changed);
}

void NetworkBehavior::Edge_Changed (Edge *pChanged)
{
	Generic::Notifier<BehaviorEvents>::vNotify(&Juice::BehaviorEvents::Behavior_Changed);
}

/****************************************************************************/
/** 
*****************************************************************************/
void NetworkBehavior::vPrepareSimulation ()
{
	{
		MotionSourceNodeList::iterator iter, last = m_lsMotionSourceNodes.end ();
		for (iter = m_lsMotionSourceNodes.begin (); iter != last; iter++)
			(*iter)->vPrepareSimulation ();
	}
	{
		FunctionNodeList::iterator iter, last = m_lsFunctionNodes.end ();
		for (iter = m_lsFunctionNodes.begin (); iter != last; iter++)
			(*iter)->vPrepareSimulation ();
	}
	{
		MotorNodeList::iterator iter, last = m_lsMotorNodes.end ();
		for (iter = m_lsMotorNodes.begin (); iter != last; iter++)
			(*iter)->vPrepareSimulation ();
	}

	vCompileScript ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void NetworkBehavior::vCompileScript ()
{
	// Throw out the current script
	vDiscardScript ();

	// Create a temporary list of edges
	EdgeList lsRemainingEdges = m_lsEdges;

	// Create lists of nodes that need input
	NodeList lsDestinationNodes;
	lsDestinationNodes.insert (lsDestinationNodes.end (), m_lsFunctionNodes.begin (), m_lsFunctionNodes.end ());
	lsDestinationNodes.insert (lsDestinationNodes.end (), m_lsMotorNodes.begin (), m_lsMotorNodes.end ());

	// reset all destination-node input-counters to zero
	{
		NodeList::iterator iter, last = lsDestinationNodes.end ();
		for (iter = lsDestinationNodes.begin (); iter != last; iter++)
			(*iter)->vSetUnsatisfiedInputCount (0);
	}

	// compute number of unsatisfied inputs for each node
	{
		EdgeList::iterator iter, last = lsRemainingEdges.end ();
		for (iter = lsRemainingEdges.begin (); iter != last; iter++)
		{
			Edge *pEdge = *iter;

			if (!pEdge)
				continue;

			Node *pNode = pEdge->pGetDestination ();

			if (!pNode)
				continue;

            pNode->vSetUnsatisfiedInputCount (pNode->uGetUnsatisfiedInputCount () + 1);
		}
	}

	// Create operations to propogate activations from source nodes to destination nodes.
	// As destination nodes get all of their inputs satisfied, they become usable as source nodes.
	// As edges are processed, they are removed from the list
	while (lsRemainingEdges.size ())
	{
		EdgeList lsEdgesToRemove (this);

		EdgeList::iterator iter, last = lsRemainingEdges.end ();
		for (iter = lsRemainingEdges.begin (); iter != last; iter++)
		{
			Edge *pEdge = *iter;

			// Sanity check
			if (!pEdge)
			{
				lsEdgesToRemove.push_front (pEdge);
				continue;
			}

			Node *pSource = pEdge->pGetSource ();
			Node *pDestination = pEdge->pGetDestination ();
            
			// Sanity check
			if (!pSource || !pDestination)
			{
				lsEdgesToRemove.push_front (pEdge);
				continue;
			}

			// Only process edges whose source nodes are fully satisfied
			if (pSource->uGetUnsatisfiedInputCount () == 0)
			{
				if (pDestination->uGetUnsatisfiedInputCount () == 0)
				{
					Breakpoint ("NetworkBehavior::vCompileScript: destination node already satisfied.");
					continue;
				}

				// Create an operation to process this edge, and add that operation to the script
				EdgeOperation *pEdgeOperation = new EdgeOperation (pEdge);
				m_lsOperations.push_back (pEdgeOperation);

				// Make sure this edge doesn't get processed again
				lsEdgesToRemove.push_front (pEdge);

				// Decrement the destination node's unsatisfied-input counter
				pDestination->vSetUnsatisfiedInputCount (pDestination->uGetUnsatisfiedInputCount () - 1);

				// If the destination node is fully satisfied, process it
				if (pDestination->uGetUnsatisfiedInputCount () == 0)
				{
					// Create an operation to process this node, and add that operation to the script
					NodeOperation *pNodeOperation = new NodeOperation (pDestination);
					m_lsOperations.push_back (pNodeOperation);
				}
			}
		}	

		// Sometimes the graph dead-ends before all the edges have been processed
		// (e.g. two disjoint subgraphs)
		if (!lsEdgesToRemove.size ())
			break;

		// Remove any processed edges
		{
			EdgeList::iterator iter, last = lsEdgesToRemove.end ();
			for (iter = lsEdgesToRemove.begin (); iter != last; iter++)
			{
				lsRemainingEdges.remove (*iter);
			}
		}
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void NetworkBehavior::vDiscardScript ()
{
	while (m_lsOperations.size ())
	{
		Operation *pOperation = m_lsOperations.front ();
		delete pOperation;
		m_lsOperations.pop_front ();
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void NetworkBehavior::vStep (real rEnginePhase, Generic::UINT32 uMilliseconds, ControlInput *pController)
{
	// reset all nodes
	{
		MotionSourceNodeList::iterator iter, last = m_lsMotionSourceNodes.end ();
		for (iter = m_lsMotionSourceNodes.begin (); iter != last; iter++)
		{
			(*iter)->vBeginIteration (rEnginePhase, pController);
		}
	}
	{
		FunctionNodeList::iterator iter, last = m_lsFunctionNodes.end ();
		for (iter = m_lsFunctionNodes.begin (); iter != last; iter++)
			(*iter)->vBeginIteration ();
	}
	{
		MotorNodeList::iterator iter, last = m_lsMotorNodes.end ();
		for (iter = m_lsMotorNodes.begin (); iter != last; iter++)
			(*iter)->vBeginIteration ();
	}

	// Execute the script
	{
		OperationList::iterator iter, last = m_lsOperations.end ();
		for (iter = m_lsOperations.begin (); iter != last; iter++)
		{
			Operation *pOperation = *iter;

			if (!pOperation)
				continue;

			pOperation->vExecute ();
		}
	}
}

/****************************************************************************/
/** 
*****************************************************************************/

/// If fHasPosition is false, the new node will be automatically centered by 
/// the last view object to receive the _AddNode event.
void NetworkBehavior::vNodeCreated (Node *pNode, bool fHasPosition)
{
	Generic::Listener<NodeEvents>::vRegisterForEventsFrom (pNode);
	Generic::Notifier<BehaviorEvents>::vNotify(&Juice::BehaviorEvents::Behavior_Changed);
	Notifier<NetworkBehaviorEvents>::vNotify (&NetworkBehaviorEvents::Behavior_AddNode, pNode, fHasPosition);
}

void NetworkBehavior::vEdgeCreated (Edge *pEdge)
{
	Generic::Listener<EdgeEvents>::vRegisterForEventsFrom (pEdge);
	Generic::Notifier<BehaviorEvents>::vNotify(&Juice::BehaviorEvents::Behavior_Changed);
	Notifier<NetworkBehaviorEvents>::vNotify (&NetworkBehaviorEvents::Behavior_AddEdge, pEdge);
}

MotionSourceNode* NetworkBehavior::pCreateMotionSourceNode ()
{
	MotionSourceNode *pNewNode = new MotionSourceNode (this);

	if (!pNewNode)
		return null;

	vNodeCreated (pNewNode, false);

	m_lsMotionSourceNodes.push_front (pNewNode);

	return pNewNode;
}

void NetworkBehavior::vCreateMotionSourceNode ()
{
	pCreateMotionSourceNode ();
}

ConstantNode *NetworkBehavior::pCreateConstantNode ()
{
	ConstantNode *pNewNode = new ConstantNode (this);

	if (!pNewNode)
		return null;

	vNodeCreated (pNewNode, false);

	m_lsFunctionNodes.push_front (pNewNode);

	return pNewNode;
}

void NetworkBehavior::vCreateConstantNode ()
{
	pCreateConstantNode ();
}

LimitNode *NetworkBehavior::pCreateLimitNode ()
{
	LimitNode *pNewNode = new LimitNode (this);

	if (!pNewNode)
		return null;

	vNodeCreated (pNewNode, false);

	m_lsFunctionNodes.push_front (pNewNode);

	return pNewNode;
}

void NetworkBehavior::vCreateLimitNode ()
{
	pCreateLimitNode ();
}

AbsoluteValueNode *NetworkBehavior::pCreateAbsoluteValueNode ()
{
	AbsoluteValueNode *pNewNode = new AbsoluteValueNode (this);

	if (!pNewNode)
		return null;

	vNodeCreated (pNewNode, false);

	m_lsFunctionNodes.push_front (pNewNode);

	return pNewNode;
}

void NetworkBehavior::vCreateAbsoluteValueNode ()
{
	pCreateAbsoluteValueNode ();
}

MultiplyNode *NetworkBehavior::pCreateMultiplyNode ()
{
	MultiplyNode *pNewNode = new MultiplyNode (this);

	if (!pNewNode)
		return null;

	vNodeCreated (pNewNode, false);

	m_lsFunctionNodes.push_front (pNewNode);

	return pNewNode;
}

void NetworkBehavior::vCreateMultiplyNode ()
{
	pCreateMultiplyNode ();
}

ArithmeticNode* NetworkBehavior::pCreateArithmeticNode ()
{
	Node *pNewNode = new ArithmeticNode (this);

	if (!pNewNode)
		return null;

	vNodeCreated (pNewNode, false);

	m_lsFunctionNodes.push_front (pNewNode);

	return (ArithmeticNode*) pNewNode;
}

void NetworkBehavior::vCreateArithmeticNode ()
{
	pCreateArithmeticNode ();
}

WaveNode* NetworkBehavior::pCreateWaveNode ()
{
	Node *pNewNode = new WaveNode (this);

	if (!pNewNode)
		return null;

	vNodeCreated (pNewNode, false);

	m_lsFunctionNodes.push_front (pNewNode);

	return (WaveNode*) pNewNode;
}

void NetworkBehavior::vCreateWaveNode ()
{
	pCreateWaveNode ();
}

MotorNode* NetworkBehavior::pCreateMotorNode ()
{
	MotorNode *pNewNode = new MotorNode (this);

	if (!pNewNode)
		return null;

	vNodeCreated (pNewNode, false);

	m_lsMotorNodes.push_front (pNewNode);

	return pNewNode;
}

void NetworkBehavior::vCreateMotorNode ()
{
	pCreateMotorNode ();
}

/****************************************************************************/
/** 
*****************************************************************************/
Edge* NetworkBehavior::pCreateEdge (Node *pSource, Node *pDestination)
{
	Edge *pNewEdge = new Edge (this);

	if (!pNewEdge)
		return null;

	pNewEdge->vSetSource (pSource);
	pNewEdge->vSetDestination (pDestination);

	m_lsEdges.push_front (pNewEdge);

	vEdgeCreated (pNewEdge);

	// during serialization, these are null, so don't compile
	// at run time, these will be valid
	if (pSource && pDestination)
		vCompileScript ();

	return pNewEdge;
}

/****************************************************************************/
/** 
*****************************************************************************/
void NetworkBehavior::vReportAllObjects ()
{
	{
		MotionSourceNodeList::iterator iter, last = m_lsMotionSourceNodes.end ();
		for (iter = m_lsMotionSourceNodes.begin (); iter != last; iter++)
		{
			Notifier<NetworkBehaviorEvents>::vNotify (&NetworkBehaviorEvents::Behavior_AddNode, *iter, true);		
		}
	}
	{
		FunctionNodeList::iterator iter, last = m_lsFunctionNodes.end ();
		for (iter = m_lsFunctionNodes.begin (); iter != last; iter++)
		{
			Notifier<NetworkBehaviorEvents>::vNotify (&NetworkBehaviorEvents::Behavior_AddNode, *iter, true);		
		}
	}
	{
		MotorNodeList::iterator iter, last = m_lsMotorNodes.end ();
		for (iter = m_lsMotorNodes.begin (); iter != last; iter++)
		{
			Notifier<NetworkBehaviorEvents>::vNotify (&NetworkBehaviorEvents::Behavior_AddNode, *iter, true);		
		}
	}
	{
		EdgeList::iterator iter, last = m_lsEdges.end ();
		for (iter = m_lsEdges.begin (); iter != last; iter++)
		{
			Notifier<NetworkBehaviorEvents>::vNotify (&NetworkBehaviorEvents::Behavior_AddEdge, *iter);		
		}
	}
	
}

/****************************************************************************/
/** 
*****************************************************************************/
PropertyTreePtr NetworkBehavior::ms_pPropertyTree;

Generic::PropertyTree* NetworkBehavior::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Behavior::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		ActionProperty *pCreateMotionSource = new ActionProperty ("Create Motion Source", (fnInvoke) &Juice::NetworkBehavior::vCreateMotionSourceNode);
		ms_pPropertyTree->vAddNode (pCreateMotionSource);

		ActionProperty *pCreateConstant = new ActionProperty("Create Constant", (fnInvoke)&Juice::NetworkBehavior::vCreateConstantNode);
		ms_pPropertyTree->vAddNode (pCreateConstant);

		ActionProperty *pCreateMultiply = new ActionProperty("Create Multiplier", (fnInvoke)&Juice::NetworkBehavior::vCreateMultiplyNode);
		ms_pPropertyTree->vAddNode (pCreateMultiply);

		ActionProperty *pCreateArithmetic = new ActionProperty("Create Arithmetic Node", (fnInvoke)&Juice::NetworkBehavior::vCreateArithmeticNode);
		ms_pPropertyTree->vAddNode (pCreateArithmetic);

		ActionProperty *pCreateAbsolute = new ActionProperty("Create Absolute-Value Node", (fnInvoke)&Juice::NetworkBehavior::vCreateAbsoluteValueNode);
		ms_pPropertyTree->vAddNode (pCreateAbsolute);

		ActionProperty *pCreateLimit = new ActionProperty("Create Limit Node", (fnInvoke)&Juice::NetworkBehavior::vCreateLimitNode);
		ms_pPropertyTree->vAddNode (pCreateLimit);

		ActionProperty *pCreateWaveFunction = new ActionProperty("Create Wave Function Node", (fnInvoke)&Juice::NetworkBehavior::vCreateWaveNode);
		ms_pPropertyTree->vAddNode (pCreateWaveFunction);

		ActionProperty *pCreateMotor = new ActionProperty("Create Motor", (fnInvoke)&Juice::NetworkBehavior::vCreateMotorNode);
		ms_pPropertyTree->vAddNode (pCreateMotor);
	}

	return ms_pPropertyTree;
}

// NetworkBehavior.cpp ends here ->