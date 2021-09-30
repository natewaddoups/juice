/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: NetoworkNodes.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>
#include <math.h>

using namespace Juice;

/****************************************************************************/
/** This restores node->body and node->joint pointers, for use when creating
 ** copies of nodes for the clipboard.
*****************************************************************************/
void NodeList::vSetPointers (const BodyHashMap &BodyMap, const JointHashMap &JointMap)
{
	iterator iter, last = end ();
	for (iter = begin (); iter != last; iter++)
	{
		Node *pNode = *iter;

		MotionSourceNode *pMotionSourceNode = dynamic_cast <MotionSourceNode*> (pNode);
		if (pMotionSourceNode)
		{
			BodyHashMap::const_iterator HashIter = BodyMap.find ((Generic::UINT32) pMotionSourceNode->pGetBody ());

			if (HashIter == BodyMap.end ())
				pMotionSourceNode->vSetBody (null);
			else
				pMotionSourceNode->vSetBody ((*HashIter).second);
		}

		MotorNode *pMotorNode = dynamic_cast <MotorNode*> (pNode);
		if (pMotorNode)
		{
			JointHashMap::const_iterator HashIter = JointMap.find ((Generic::UINT32) pMotorNode->pGetJoint ());

			if (HashIter == JointMap.end ())
				pMotorNode->vSetJoint (null);
			else
				pMotorNode->vSetJoint ((*HashIter).second);
		}
	}
}

void NodeList::vCreateHashMap (NodeHashMap &NodeMap)
{
	iterator iter, last = end ();
	for (iter = begin (); iter != last; iter++)
	{
		Node *pNode = *iter;
		Generic::UINT32 uID = pNode->uGetID ();
		NodeMap.insert (std::pair <Generic::UINT32, Node*> (uID, pNode));
	}
}

bool NodeList::contains (Node *pNode)
{
	iterator iter, last = end ();
	for (iter = begin (); iter != last; iter++)
	{
		Node *pIterNode = *iter;
		if (pIterNode == pNode)
			return true;
	}

	return false;
}

void EdgeList::vSetNodePointers (const NodeHashMap &NodeMap)
{
	EdgeList::iterator iter, last = end ();
	for (iter = begin (); iter != last; iter++)
	{
		Edge *pEdge = *iter;

		Generic::UINT32 uSource = (Generic::UINT32) pEdge->pGetSource ();
		Generic::UINT32 uDestination = (Generic::UINT32) pEdge->pGetDestination ();

		unordered_map<Generic::UINT32, Node*>::const_iterator iterMap;

		iterMap = NodeMap.find ((Generic::UINT32) uSource);
		if (iterMap == NodeMap.end ())
			pEdge->vSetSource (null);
		else
			pEdge->vSetSource ((*iterMap).second);


		iterMap = NodeMap.find ((Generic::UINT32) uDestination);
		if (iterMap == NodeMap.end ())
			pEdge->vSetDestination (null);
		else
			pEdge->vSetDestination ((*iterMap).second);
	}
}

/****************************************************************************/
/** Edge
*****************************************************************************/

Edge::Edge (Selectable *pParent) : 
	Selectable (pParent), 
	XML::Serializable (L"Edge"), 
	m_pSource (null), 
	m_pDestination (null), 
	m_rFactor (1.0)
{
}

Edge::~Edge ()
{
	m_pSource = (Node*) 1;
	m_pDestination = (Node*) 2;
}

Edge& Edge::operator= (const Edge &src)
{
	m_pSource      = src.m_pSource;
	m_pDestination = src.m_pDestination;
	m_rFactor      = src.m_rFactor;

	return *this;
}

Selectable* Edge::pCreateClipboardCopy () const
{
	Edge *pCopy = new Edge (m_pParent);

	*pCopy = *this;

	return (Selectable*) pCopy; 
}

bool Edge::fPaste (Selectable *pSelectable)
{
	Edge *pSource = dynamic_cast <Edge*> (pSelectable);

	if (pSource)
	{
		*this = *pSource;
		return true;
	}

	return false;
}

PropertyTreePtr Edge::ms_pPropertyTree;

Generic::PropertyTree* Edge::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		ms_pPropertyTree = new Generic::PropertyTree;

		StringProperty *pSrcName = new StringProperty ("Source", (Generic::fnGetString) &Juice::Edge::szGetSourceName, null);
		ms_pPropertyTree->vAddNode (pSrcName);

		StringProperty *pDstName = new StringProperty("Destination", (Generic::fnGetString) &Juice::Edge::szGetDestinationName, null);
		ms_pPropertyTree->vAddNode (pDstName);
	}
	return ms_pPropertyTree;
}

const char* Edge::szGetSourceName ()
{
	if (m_pSource)
	{
		return m_pSource->szGetName ();
	}
	else
	{
		return "";
	}
}

const char* Edge::szGetDestinationName ()
{
	if (m_pDestination)
	{
		return m_pDestination->szGetName ();
	}
	else
	{
		return "";
	}
}

void Edge::vSetSource (Node *pSource)
{
	m_pSource = pSource;
	Generic::Notifier<EdgeEvents>::vNotify (&EdgeEvents::Edge_Changed);
}

void Edge::vSetDestination (Node *pDestination)
{
	m_pDestination = pDestination;
	Generic::Notifier<EdgeEvents>::vNotify (&EdgeEvents::Edge_Changed);
}

Node* Edge::pGetSource ()
{
	return m_pSource;
}

Node* Edge::pGetDestination ()
{
	return m_pDestination;
}

void EdgeList::vCreateNew (Edge **ppNew, bool *pfAddToList)
{
	*ppNew = m_pBehavior->pCreateEdge (null, null); 
	*pfAddToList = false;
}

/****************************************************************************/
/** Node
*****************************************************************************/

Node::Node (Selectable *pParent, const wchar_t *wszXmlTag) : 
	XML::Serializable (wszXmlTag),
	Selectable ((Selectable*) pParent), 
	m_rActivation (0), 
	m_iX (0),
	m_iY (0),
	m_uUnsatisfiedInputCount (0)
{
}

Node::~Node ()
{
}

/*Node& Node::operator= (const Node &src)
{
	m_iX = src.m_iX;
	m_iY = src.m_iY;
	m_uID = (Generic::UINT32) &src;

	return *this;
}
*/

void Node::operator= (const Node *pNode)
{
	m_iX = pNode->m_iX;
	m_iY = pNode->m_iY;
	m_uID = (Generic::UINT32) pNode;
}

Selectable* Node::pCreateClipboardCopy () const
{
	Node *pNew = new Node (m_pParent, szGetPreferredTagName ());

	*pNew = this;

	return (Selectable*) pNew;
}

bool Node::fPaste (Selectable *pSource)
{
	if (typeid (*this) == typeid (*pSource))
	{
		Node *pNode = dynamic_cast <Node*> (pSource);

		int iX = m_iX, iY = m_iY;

		*this = pNode;

		m_iX = iX;
		m_iY = iY;

		Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);

		return true;
	}

	return false;
}

void Node::vSetName (const char *szName)
{
	m_strName = szName;
}

const char* Node::szGetName ()
{
	return m_strName.c_str ();
}

PropertyTreePtr Node::ms_pPropertyTree;

Generic::PropertyTree* Node::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		ms_pPropertyTree = new Generic::PropertyTree;

		StringProperty *pName = new StringProperty("Name", (Generic::fnGetString) &Juice::Node::szGetName, (Generic::fnSetString) &Juice::Node::vSetName);
		ms_pPropertyTree->vAddNode (pName);

		RealProperty *pActivation = new RealProperty ("Activation", (Generic::fnGetReal) &Juice::Node::rGetActivation, null);
		ms_pPropertyTree->vAddNode (pActivation);
	}

	return ms_pPropertyTree;
}

Generic::INT32 Juice::Node::iGetX ()
{
	return m_iX;
}

Generic::INT32 Juice::Node::iGetY ()
{
	return m_iY;
}

void Juice::Node::vSetX (Generic::INT32 iX)
{
	m_iX = iX;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

void Juice::Node::vSetY (Generic::INT32 iY)
{
	m_iY = iY;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

Generic::UINT32 Juice::Node::uGetWidth ()
{
	return m_uWidth;
}

Generic::UINT32 Juice::Node::uGetHeight ()
{
	return m_uHeight;
}

void Juice::Node::vSetWidth (Generic::UINT32 uWidth)
{
	m_uWidth = uWidth;
}

void Juice::Node::vSetHeight (Generic::UINT32 uHeight)
{
	m_uHeight = uHeight;
}

/****************************************************************************/
/** MotionSourceNode
*****************************************************************************/

MotionSourceNode::MotionSourceNode (Selectable *pParent) :
	Node (pParent, L"MotionSourceNode"),
	m_pBody (null),
	m_fInitialized (false)
{
}

MotionSourceNode::~MotionSourceNode ()
{
}

/*MotionSourceNode& MotionSourceNode::operator= (const MotionSourceNode &src)
{
	Node::operator= (src);

	m_pBody = src.m_pBody;

	return *this;
}*/

void MotionSourceNode::operator= (const Node *pNode)
{
	const MotionSourceNode *pSource = dynamic_cast <const MotionSourceNode*> (pNode);

	if (!pSource)
		return;

	Node::operator= (pNode);
	
	vSetBody (pSource->m_pBody);
	m_eMotionSource = pSource->m_eMotionSource;
	m_eAmpModSource = pSource->m_eAmpModSource;
}

Selectable* MotionSourceNode::pCreateClipboardCopy () const
{
	MotionSourceNode *pNew = new MotionSourceNode (m_pParent);

	*pNew = (Node*) this;

	return (Selectable*) pNew;
}

/*bool MotionSourceNode::fPaste (Selectable *pSelectable)
{
	MotionSourceNode *pSource = dynamic_cast <MotionSourceNode*> (pSelectable);

	if (pSource)
	{
		*this = *pSource;
		return true;
	}

	return false;
}
*/
void MotionSourceNode::vBeginIteration (real rEnginePhase, ControlInput *pController)
{
	switch (m_eMotionSource)
	{
	case msMainEngine:
		m_rActivation = rEnginePhase * rRadiansToDegrees;
		break;

	case msJoystick1X:
		m_rActivation = pController->m_rJoystick1X;
		break;

	case msJoystick1Y:
		m_rActivation = pController->m_rJoystick1Y;
		break;

	case msJoystick1Z:
		m_rActivation = pController->m_rJoystick1Z;
		break;

	case msJoystick1R:
		m_rActivation = pController->m_rJoystick1R;
		break;

	case msJoystick2X:
		m_rActivation = pController->m_rJoystick2X;
		break;

	case msJoystick2Y:
		m_rActivation = pController->m_rJoystick2Y;
		break;

	case msJoystick2Z:
		m_rActivation = pController->m_rJoystick2Z;
		break;

	case msJoystick2R:
		m_rActivation = pController->m_rJoystick2R;
		break;

#ifdef _WINDOWS
	case msKeyUpDown:
		m_rActivation = 0;
		if (::GetAsyncKeyState (VK_UP))
			m_rActivation = -1;
		if (::GetAsyncKeyState (VK_DOWN))
			m_rActivation = 1;
		break;

	case msKeyLeftRight:
		m_rActivation = 0;
		if (::GetAsyncKeyState (VK_LEFT))
			m_rActivation = -1;
		if (::GetAsyncKeyState (VK_RIGHT))
			m_rActivation = 1;
		break;
#endif


	case msBodyYaw:
	case msBodyPitch:
	case msBodyRoll:
		m_rActivation = rGetFeedback ();
		break;

	default:
		m_rActivation = 0;
		break;
	}
}

real MotionSourceNode::rGetFeedback ()
{
	if (!m_pBody)
		return 0;

//	VectorYPR vecEuler;

//	m_pBody->vGetRotation (vecEuler);

	real rRotation = 0;

	switch (m_eMotionSource)
	{
	case msBodyYaw:
		rRotation = m_pBody->rGetRotationY ();
		break;

	case msBodyPitch:
		rRotation = m_pBody->rGetRotationP ();
		rRotation *= cos (m_pBody->rGetRotationR ());
		break;

	case msBodyRoll:
		rRotation = m_pBody->rGetRotationR ();
		rRotation *= cos (m_pBody->rGetRotationP ());
		break;

	default:
		Breakpoint ("MotionSourceNode::rGetFeedback: invalid motion source.");
		rRotation = 0;
	}

	return rRotation * rRadiansToDegrees;
}

const char* MotionSourceNode::szGetName ()
{
	switch (m_eMotionSource)
	{
	case msMainEngine:
		m_strName = "Main Engine";
		break;

	case msJoystick1X:
		m_strName = "Joystick 1 X";
		break;

	case msJoystick1Y:
		m_strName = "Joystick 1 Y";
		break;

	case msJoystick1Z:
		m_strName = "Joystick 1 Z";
		break;

	case msJoystick1R:
		m_strName = "Joystick 1 R";
		break;

	case msJoystick2X:
		m_strName = "Joystick 2 X";
		break;

	case msJoystick2Y:
		m_strName = "Joystick 2 Y";
		break;

	case msJoystick2Z:
		m_strName = "Joystick 2 Z";
		break;

	case msJoystick2R:
		m_strName = "Joystick 2 R";
		break;

	case msBodyYaw:
		m_strName = "Yaw: ";
		break;

	case msBodyPitch:
		m_strName = "Pitch: ";
		break;

	case msBodyRoll:
		m_strName = "Roll: ";
		break;

	case msKeyUpDown:
		m_strName = "Key Up/Dn";
		break;

	case msKeyLeftRight:
		m_strName = "Key Lt/Rt";
		break;

	default:
		m_strName = "Motion Source";
		break;
	}

	if (((int) m_eMotionSource >= msBodyYaw) && ((int) m_eMotionSource <= msBodyRoll))
	{
		if (m_pBody)
		{
			m_strName += m_pBody->szGetName ();
		}
		else
		{
			m_strName += "unassigned";
		}
	}

	return m_strName.c_str ();
}

void MotionSourceNode::vSetMotionSource (int iSource)
{
	m_eMotionSource = (MotionSource) iSource;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

int MotionSourceNode::iGetMotionSource ()
{
	return m_eMotionSource;
}

void MotionSourceNode::vSetAmpModSource (int iSource)
{
	m_eAmpModSource = (AmpModSource) iSource;
	Generic::Notifier<NodeEvents>::vNotify(&Juice::NodeEvents::Node_Changed);
}

int MotionSourceNode::iGetAmpModSource ()
{
	return m_eAmpModSource;
}

void MotionSourceNode::vSetBody (Component *pFeedbackSource)
{
	// TODO: make this work for both bodies and joints
	if (m_pBody && m_fInitialized)
		Generic::Listener<ComponentEvents>::vUnregister (m_pBody);

	m_pBody = (Body*) pFeedbackSource;
	m_fInitialized = true;

	if (m_pBody)
		Generic::Listener<ComponentEvents>::vRegisterForEventsFrom (m_pBody);

	Generic::Notifier<NodeEvents>::vNotify(&Juice::NodeEvents::Node_Changed);
}

Component* MotionSourceNode::pGetBody ()
{
	return m_pBody;
}


PropertyTreePtr MotionSourceNode::ms_pPropertyTree;

Generic::PropertyTree* MotionSourceNode::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Node::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		ListProperty *pMotionSource = new ListProperty("Control Source", (fnGetInt)&Juice::MotionSourceNode::iGetMotionSource, (fnSetInt)&Juice::MotionSourceNode::vSetMotionSource);
		pMotionSource->vAddItem (msMainEngine, "Main Engine");
		pMotionSource->vAddItem (msJoystick1X, "Joystick 1 X");
		pMotionSource->vAddItem (msJoystick1Y, "Joystick 1 Y");
		pMotionSource->vAddItem (msJoystick1Z, "Joystick 1 Z");
		pMotionSource->vAddItem (msJoystick1R, "Joystick 1 R");
		pMotionSource->vAddItem (msJoystick2X, "Joystick 2 X");
		pMotionSource->vAddItem (msJoystick2Y, "Joystick 2 Y");
		pMotionSource->vAddItem (msJoystick2Z, "Joystick 2 Z");
		pMotionSource->vAddItem (msJoystick2R, "Joystick 2 R");
		pMotionSource->vAddItem (msKeyUpDown, "Keyboard Up/Down");
		pMotionSource->vAddItem (msKeyLeftRight, "Keyboard Left/Right");		
		pMotionSource->vAddItem (msBodyYaw,   "Body Yaw Feedback");
		pMotionSource->vAddItem (msBodyPitch, "Body Pitch Feedback");
		pMotionSource->vAddItem (msBodyRoll,  "Body Roll Feedback");
		// TODO: revolute joint
		// TODO: prismatic joint

		ms_pPropertyTree->vAddNode (pMotionSource);

		BodyProperty *pFeedbackSource = new BodyProperty("Feedback Source", (fnGetComponent)&Juice::MotionSourceNode::pGetBody, (fnSetComponent)&Juice::MotionSourceNode::vSetBody);
		ms_pPropertyTree->vAddNode (pFeedbackSource);

		ListProperty *pAmpModSource = new ListProperty("Amp Mod", (fnGetInt)&Juice::MotionSourceNode::iGetAmpModSource, (fnSetInt)&Juice::MotionSourceNode::vSetAmpModSource);
		pAmpModSource->vAddItem (amsNone, "None");
		pAmpModSource->vAddItem (amsJoystickX, "Joystick X");
		pAmpModSource->vAddItem (amsJoystickY, "Joystick Y");
		pAmpModSource->vAddItem (amsJoystickZ, "Joystick Z");
		pAmpModSource->vAddItem (amsJoystickR, "Joystick R");

		ms_pPropertyTree->vAddNode (pAmpModSource);
	}
	return ms_pPropertyTree;
}

void MotionSourceNodeList::vCreateNew (MotionSourceNode **ppNew, bool *pfAddToList) 
{
	*ppNew = m_pBehavior->pCreateMotionSourceNode (); 
	*pfAddToList = false; 
}

/****************************************************************************/
/** ArithmeticNode
*****************************************************************************/

ArithmeticNode::ArithmeticNode (Selectable *pParent) :
	Node (pParent, ms_wszXmlTag),
	m_fAddFirst (false),
	m_rAdd (0),
	m_rMultiply (1)
{
}

ArithmeticNode::~ArithmeticNode ()
{
}

/*ArithmeticNode& ArithmeticNode::operator= (const ArithmeticNode &src)
{
	Node::operator= (src);

	m_fAddFirst = src.m_fAddFirst;
	m_rAdd = src.m_rAdd;
	m_rMultiply = src.m_rMultiply;

	return *this;
}*/

void ArithmeticNode::operator= (const Node *pNode)
{
	const ArithmeticNode *pSource = dynamic_cast <const ArithmeticNode*> (pNode);

	if (!pSource)
		return;

	Node::operator= (pNode);
	
	m_fAddFirst = pSource->m_fAddFirst;
	m_rAdd      = pSource->m_rAdd;
	m_rMultiply = pSource->m_rMultiply;
}


Selectable* ArithmeticNode::pCreateClipboardCopy () const
{
	ArithmeticNode *pNew = new ArithmeticNode (m_pParent);

	*pNew = this;

	return (Selectable*) pNew;
}

/*bool ArithmeticNode::fPaste (Selectable *pSelectable)
{
	ArithmeticNode *pSource = dynamic_cast <ArithmeticNode*> (pSelectable);

	if (pSource)
	{
		*this = *pSource;
		return true;
	}

	return false;
}*/


void ArithmeticNode::vStep ()
{
	if (m_fAddFirst)
		m_rActivation = (m_rActivation + m_rAdd) * m_rMultiply;
	else
		m_rActivation = (m_rActivation * m_rMultiply) + m_rAdd;
}

const char* ArithmeticNode::szGetName ()
{
	char szTemp[200];
	if (m_fAddFirst)
	{
		sprintf (szTemp, "+ %g * %g", m_rAdd, m_rMultiply);
	}
	else
	{
		sprintf (szTemp, "* %g + %g", m_rMultiply, m_rAdd);
	}

	m_strName = szTemp;
	return m_strName.c_str ();
}

PropertyTreePtr ArithmeticNode::ms_pPropertyTree;

Generic::PropertyTree* ArithmeticNode::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Node::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		RealProperty *pAddend = new RealProperty("Add", (Generic::fnGetReal) &Juice::ArithmeticNode::rGetAddend, (Generic::fnSetReal) &Juice::ArithmeticNode::vSetAddend);
		ms_pPropertyTree->vAddNode (pAddend);

		RealProperty *pFactor = new RealProperty("Multiply", (Generic::fnGetReal) &Juice::ArithmeticNode::rGetFactor, (Generic::fnSetReal) &Juice::ArithmeticNode::vSetFactor);
		ms_pPropertyTree->vAddNode (pFactor);

		BoolProperty *pOrder = new BoolProperty("Add First", (Generic::fnGetBool) &Juice::ArithmeticNode::fGetOrder, (Generic::fnSetBool) &Juice::ArithmeticNode::vSetOrder);
		ms_pPropertyTree->vAddNode (pOrder);
	}
	return ms_pPropertyTree;
}

void ArithmeticNode::vSetAddend (real rAddend)
{
	m_rAdd = rAddend;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

real ArithmeticNode::rGetAddend ()
{
	return m_rAdd;
}

void ArithmeticNode::vSetFactor (real rFactor)
{
	m_rMultiply = rFactor;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

real ArithmeticNode::rGetFactor ()
{
	return m_rMultiply;
}

void ArithmeticNode::vSetOrder (bool fAddFirst)
{
	m_fAddFirst = fAddFirst;
	Generic::Notifier<NodeEvents>::vNotify(&Juice::NodeEvents::Node_Changed);
}

bool ArithmeticNode::fGetOrder ()
{
	return m_fAddFirst;
}

/****************************************************************************/
/** AbsoluteValueNode
*****************************************************************************/

const wchar_t* AbsoluteValueNode::ms_wszXmlTag = L"AbsoluteValueNode";

AbsoluteValueNode::AbsoluteValueNode (Selectable *pParent) :
	Node (pParent, ms_wszXmlTag)
{
	m_strName = "Absolute";
}

AbsoluteValueNode::~AbsoluteValueNode ()
{
}

void AbsoluteValueNode::operator= (const Node *pNode)
{
	const AbsoluteValueNode *pSource = dynamic_cast <const AbsoluteValueNode*> (pNode);

	if (!pSource)
		return;

	Node::operator= (pNode);
}


Selectable* AbsoluteValueNode::pCreateClipboardCopy () const
{
	AbsoluteValueNode *pNew = new AbsoluteValueNode (m_pParent);

	*pNew = (Node*) this;

	return (Selectable*) pNew;
}

/*bool AbsoluteValueNode::fPaste (Selectable *pSelectable)
{
	AbsoluteValueNode *pSource = dynamic_cast <AbsoluteValueNode*> (pSelectable);

	if (pSource)
	{
		*this = *pSource;
		return true;
	}

	return false;
}*/


const char* AbsoluteValueNode::szGetName ()
{
	return m_strName.c_str ();
}

PropertyTreePtr AbsoluteValueNode::ms_pPropertyTree;

Generic::PropertyTree* AbsoluteValueNode::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Node::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);
	}
	return ms_pPropertyTree;
}

void AbsoluteValueNode::vStep ()
{
	m_rActivation = fabs (m_rActivation);
}


/****************************************************************************/
/** MultiplyNode
*****************************************************************************/

const wchar_t *MultiplyNode::ms_wszXmlTag = L"MultiplyNode";

MultiplyNode::MultiplyNode (Selectable *pParent) :
	Node (pParent, ms_wszXmlTag)
{
}

MultiplyNode::~MultiplyNode ()
{
}

/*MultiplyNode& MultiplyNode::operator= (const MultiplyNode &src)
{
	Node::operator= (src);

	return *this;
}*/

void MultiplyNode::operator= (const Node *pNode)
{
	const MultiplyNode *pSource = dynamic_cast <const MultiplyNode*> (pNode);

	if (!pSource)
		return;

	Node::operator= (pNode);
}


Selectable* MultiplyNode::pCreateClipboardCopy () const
{
	MultiplyNode *pNew = new MultiplyNode (m_pParent);

	*pNew = (Node*) this;

	return (Selectable*) pNew;
}


/*bool MultiplyNode::fPaste (Selectable *pSelectable)
{
	MultiplyNode *pSource = dynamic_cast <MultiplyNode*> (pSelectable);

	if (pSource)
	{
		*this = *pSource;
		return true;
	}

	return false;
}*/


const char* MultiplyNode::szGetName ()
{
	char szTemp[200];
	sprintf (szTemp, "Multiply");
	m_strName = szTemp;
	return m_strName.c_str ();
}

/****************************************************************************/
/** ConstantNode
*****************************************************************************/

const wchar_t* ConstantNode::ms_wszXmlTag = L"ConstantNode";

ConstantNode::ConstantNode (Selectable *pParent) :
	m_rValue (0),
	Node (pParent, ms_wszXmlTag)
{
}

ConstantNode::~ConstantNode ()
{
}

/*ConstantNode& ConstantNode::operator= (const ConstantNode &src)
{
	Node::operator= (src);

	m_rValue = src.m_rValue;

	return *this;
}*/

void ConstantNode::operator= (const Node *pNode)
{
	const ConstantNode *pSource = dynamic_cast <const ConstantNode*> (pNode);

	if (!pSource)
		return;

	Node::operator= (pNode);
	
	m_rValue = pSource->m_rValue;
}


Selectable* ConstantNode::pCreateClipboardCopy () const
{
	ConstantNode *pNew = new ConstantNode (m_pParent);

	*pNew = (Node*) this;

	return (Selectable*) pNew;
}

/*bool ConstantNode::fPaste (Selectable *pSelectable)
{
	ConstantNode *pSource = dynamic_cast <ConstantNode*> (pSelectable);

	if (pSource)
	{
		*this = *pSource;
		return true;
	}

	return false;
}*/


const char* ConstantNode::szGetName ()
{
	char szTemp[200];
	sprintf (szTemp, "%g", m_rValue);
	m_strName = szTemp;
	return m_strName.c_str ();
}

PropertyTreePtr ConstantNode::ms_pPropertyTree;

Generic::PropertyTree* ConstantNode::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Node::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		RealProperty *pValue = new RealProperty("Value", (Generic::fnGetReal) &Juice::ConstantNode::rGetValue, (Generic::fnSetReal) &Juice::ConstantNode::vSetValue);
		ms_pPropertyTree->vAddNode (pValue);
	}
	return ms_pPropertyTree;
}

void ConstantNode::vSetValue (real rValue)
{
	m_rValue = rValue;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

real ConstantNode::rGetValue ()
{
	return m_rValue;
}

void ConstantNode::vBeginIteration ()
{
	m_rActivation = m_rValue;
}

/****************************************************************************/
/** LimitNode
*****************************************************************************/

const wchar_t* LimitNode::ms_wszXmlTag = L"LimitNode";

LimitNode::LimitNode (Selectable *pParent) :
	m_rMinimum (-1), m_rMaximum (1),
	Node (pParent, ms_wszXmlTag)
{
}

LimitNode::~LimitNode ()
{
}

void LimitNode::operator= (const Node *pNode)
{
	const LimitNode *pSource = dynamic_cast <const LimitNode*> (pNode);

	if (!pSource)
		return;

	Node::operator= (pNode);
	
	m_rMinimum = pSource->m_rMinimum;
	m_rMaximum = pSource->m_rMaximum;
}


Selectable* LimitNode::pCreateClipboardCopy () const
{
	LimitNode *pNew = new LimitNode (m_pParent);

	*pNew = (Node*) this;

	return (Selectable*) pNew;
}

const char* LimitNode::szGetName ()
{
	char szTemp[200];
	sprintf (szTemp, "%g to %g", m_rMinimum, m_rMaximum);
	m_strName = szTemp;
	return m_strName.c_str ();
}

PropertyTreePtr LimitNode::ms_pPropertyTree;

Generic::PropertyTree* LimitNode::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Node::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		RealProperty *pMinimum = new RealProperty("Minimum", (Generic::fnGetReal) &Juice::LimitNode::rGetMinimum, (Generic::fnSetReal) &Juice::LimitNode::vSetMinimum);
		ms_pPropertyTree->vAddNode (pMinimum);

		RealProperty *pMaximum = new RealProperty("Maximum", (Generic::fnGetReal) &Juice::LimitNode::rGetMaximum, (Generic::fnSetReal) &Juice::LimitNode::vSetMaximum);
		ms_pPropertyTree->vAddNode (pMaximum);
	}
	return ms_pPropertyTree;
}

void LimitNode::vSetMinimum (real rMinimum)
{
	m_rMinimum = rMinimum;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

real LimitNode::rGetMinimum ()
{
	return m_rMinimum;
}

void LimitNode::vSetMaximum (real rMaximum)
{
	m_rMaximum = rMaximum;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

real LimitNode::rGetMaximum ()
{
	return m_rMaximum;
}

void LimitNode::vBeginIteration ()
{
	m_rActivation = 0;
}

void LimitNode::vStep ()
{
	if (m_rActivation < m_rMinimum)
		m_rActivation = m_rMinimum;

	if (m_rActivation > m_rMaximum)
		m_rActivation = m_rMaximum;
}

/****************************************************************************/
/** WaveNode
*****************************************************************************/

WaveNode::WaveNode (Selectable *pParent) :
	Node (pParent, ms_wszXmlTag),
	m_rDutyCycle (100),
	m_rOutputRange (45),
	m_rOutputOffset (0)
{
}

WaveNode::~WaveNode ()
{
}

/*WaveNode& WaveNode::operator= (const WaveNode &src)
{
	Node::operator= (src);

	m_rDutyCycle = src.m_rDutyCycle;
	m_rOutputRange = src.m_rOutputRange;
	m_rOutputOffset = src.m_rOutputOffset;

	return *this;
}*/

void WaveNode::operator= (const Node *pNode)
{
	const WaveNode *pSource = dynamic_cast <const WaveNode*> (pNode);

	if (!pSource)
		return;

	Node::operator= (pNode);
	
	m_rDutyCycle    = pSource->m_rDutyCycle;
	m_rOutputRange  = pSource->m_rOutputRange;
	m_rOutputOffset = pSource->m_rOutputOffset;
}


Selectable* WaveNode::pCreateClipboardCopy () const
{
	WaveNode *pNew = new WaveNode (m_pParent);

	*pNew = (Node*) this;

	return (Selectable*) pNew;
}

/*bool WaveNode::fPaste (Selectable *pSelectable)
{
	WaveNode *pSource = dynamic_cast <WaveNode*> (pSelectable);

	if (pSource)
	{
		*this = *pSource;
		return true;
	}

	return false;
}*/


void WaveNode::vStep ()
{
	const real rTwoPi = rPi * 2;
	real rAngle = m_rActivation * rDegreesToRadians;

	while (rAngle > rTwoPi)
		rAngle -= rTwoPi;

	real rRampPercentage = m_rDutyCycle / 100;
	
	if (rRampPercentage > 1.0)
		rRampPercentage = 1.0;

	real rRampWidth = (rTwoPi / 4.0) * rRampPercentage;

	if (rRampWidth < 0.01)
		rRampWidth = 0.01;

	// first eighth 
	// input:  0 to ((pi / 4) * rw)
	// output: rise from 0 to 1
	if ((rAngle < rRampWidth) || (rAngle > rTwoPi))
	{
		m_rActivation = sin (rAngle * (1.0 / rRampPercentage));
	}

	// second and third eighth
	// input:  as above to ((2pi / 2) - rw)
	// output: 1
	else if (rAngle < ((rTwoPi / 2.0) - rRampWidth))
	{
		m_rActivation = 1;
	}

	// fourth and fifth eighth 
	// input:  as above to ((2pi / 2) + rw)
	// output: drop from 1 to -1
	else if (rAngle < ((rTwoPi / 2.0) + rRampWidth))
	{
		m_rActivation = sin (((rAngle - rPi) * (1 / rRampPercentage)) + rPi);
	}

	// sixth and seventh eighth 
	// input:  as above to (2pi - rw)
	// output: -1
	else if (rAngle < (rTwoPi - rRampWidth))
	{
		m_rActivation = -1;
	}

	// eighth eighth 
	// input:  as above to 2pi
	// output: rise from -1 to 0
	else if (rAngle < (rTwoPi))
	{
		m_rActivation = sin (((rAngle - rTwoPi) * (1 / rRampPercentage)) + rTwoPi);
	}

	m_rActivation *= m_rOutputRange;
	m_rActivation += m_rOutputOffset;
}

const char* WaveNode::szGetName ()
{
	char szTemp[100];
	sprintf (szTemp, "Wave %g%%\n+/-%g, +%g", m_rDutyCycle, m_rOutputRange, m_rOutputOffset);
	m_strName = szTemp;
	return m_strName.c_str ();
}

PropertyTreePtr WaveNode::ms_pPropertyTree;

Generic::PropertyTree* WaveNode::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Node::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		RealProperty *pFactor = new RealProperty("Ramp Duration", (Generic::fnGetReal) &Juice::WaveNode::rGetDutyCycle, (Generic::fnSetReal) &Juice::WaveNode::vSetDutyCycle);
		ms_pPropertyTree->vAddNode (pFactor);

		RealProperty *pRange = new RealProperty("Range", (Generic::fnGetReal) &Juice::WaveNode::rGetOutputRange, (Generic::fnSetReal) &Juice::WaveNode::vSetOutputRange);
		ms_pPropertyTree->vAddNode (pRange);

		RealProperty *pOffset = new RealProperty("Offset", (Generic::fnGetReal) &Juice::WaveNode::rGetOutputOffset, (Generic::fnSetReal) &Juice::WaveNode::vSetOutputOffset);
		ms_pPropertyTree->vAddNode (pOffset);
	}
	return ms_pPropertyTree;
}

void WaveNode::vSetDutyCycle (real rDutyCycle)
{
	m_rDutyCycle = rDutyCycle;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

real WaveNode::rGetDutyCycle ()
{
	return m_rDutyCycle;
}

void WaveNode::vSetOutputRange (real rOutputRange)
{
	m_rOutputRange = rOutputRange;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

real WaveNode::rGetOutputRange ()
{
	return m_rOutputRange;
}

void WaveNode::vSetOutputOffset (real rOutputOffset)
{
	m_rOutputOffset = rOutputOffset;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

real WaveNode::rGetOutputOffset ()
{
	return m_rOutputOffset;
}

/****************************************************************************/
/** MotorNode
*****************************************************************************/

MotorNode::MotorNode (Selectable *pParent) : 
	m_pJoint (null),
	Node (pParent, L"MotorNode"),
	m_eMotorType (MotorType::mtPosition)
{
}

MotorNode::~MotorNode ()
{
}

/*MotorNode& MotorNode::operator= (const MotorNode &src)
{
	Node::operator= (src);

	m_pJoint = src.m_pJoint;

	return *this;
}*/

void MotorNode::operator= (const Node *pNode)
{
	const MotorNode *pSource = dynamic_cast <const MotorNode*> (pNode);

	if (!pSource)
		return;

	Node::operator= (pNode);
	
	//m_pJoint = pSource->m_pJoint;
	vSetJoint (pSource->m_pJoint);
}

Selectable* MotorNode::pCreateClipboardCopy () const
{
	MotorNode *pNew = new MotorNode (m_pParent);

	*pNew = (Node*) this;

	return (Selectable*) pNew;
}

/*bool MotorNode::fPaste (Selectable *pSelectable)
{
	MotorNode *pSource = dynamic_cast <MotorNode*> (pSelectable);

	if (pSource)
	{
		*this = *pSource;
		return true;
	}

	return false;
}*/


void MotorNode::vStep ()
{
	if (!m_pJoint)
		return;

	switch (m_eMotorType)
	{
	case MotorType::mtPosition:
		m_pJoint->vAddPosition (m_rActivation);
		break;

	case MotorType::mtVelocity:
		if (typeid (*m_pJoint) == typeid (Juice::RevoluteJoint))
			m_pJoint->vAddVelocity (m_rActivation);
		break;
	}
}

const char* MotorNode::szGetName ()
{
	if (m_pJoint)
	{
		m_strName = m_pJoint->szGetName ();
	}
	else
	{
		m_strName = "Unassigned Motor";
	}

	return m_strName.c_str ();
}

PropertyTreePtr MotorNode::ms_pPropertyTree;

Generic::PropertyTree* MotorNode::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Node::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		JointProperty *pJoint = new JointProperty("Joint", (Juice::fnGetComponent) &Juice::MotorNode::pGetJoint, (Juice::fnSetComponent) &Juice::MotorNode::vSetJoint);
		ms_pPropertyTree->vAddNode (pJoint);

		ListProperty *pMotorType = new ListProperty("Motor Type", (fnGetInt)&Juice::MotorNode::iGetMotorType, (fnSetInt)&Juice::MotorNode::vSetMotorType);
		pMotorType->vAddItem (MotorType::mtPosition, "Position");
		pMotorType->vAddItem (MotorType::mtVelocity, "Velocity");

		ms_pPropertyTree->vAddNode (pMotorType);
	}
	return ms_pPropertyTree;
}

int MotorNode::iGetMotorType ()
{
	return (int) m_eMotorType;
}

void MotorNode::vSetMotorType (int iMotorType)
{
	m_eMotorType = (MotorType) iMotorType;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

Component *MotorNode::pGetJoint ()
{
	return m_pJoint;
}

void MotorNode::vSetJointNull ()
{
	m_pJoint = null;
}

void MotorNode::vSetJoint (Component *pJoint)
{
	if (pJoint == m_pJoint)
	{
		Breakpoint ("MotorNode::vSetJoint: set to same joint again.");
		return;
	}

	// disconnect from current joint
	if (m_pJoint)
	{
		vUnregister (m_pJoint);
		m_pJoint = null;
	}

	if (!pJoint)
		return;

	if (!IsKindOfPtr <Joint*> (pJoint))
	{
		Breakpoint ("MotorNode::vSetJoint: given object is not a joint.");
		Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
		return;
	}

	m_pJoint = dynamic_cast <Joint*> (pJoint);

	if (m_pJoint)
		vRegisterForEventsFrom (m_pJoint);

	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

void MotorNode::Notifier_Deleted (Component *pJoint)
{
	if (!pJoint)
	{
		Breakpoint ("MotorNode::Notifier_Deleted: null pJoint.");
		return;
	}

	if (pJoint != m_pJoint)
	{
		Breakpoint ("MotorNode::Notifier_Deleted: wrong pJoint.");
		return;
	}

	// Absolutely do not ever call vUnregister (pNotifier) during 
	// Notifier_Deleted.  Bad stuff will happen, but not until much later.  
	// It's done automatically by the Listener base class anyhow though.  
	//
	// vUnregister (m_pJoint);

	m_pJoint = null;
	Generic::Notifier<NodeEvents>::vNotify (&Juice::NodeEvents::Node_Changed);
}

void MotorNodeList::vCreateNew (MotorNode **ppNew, bool *pfAddToList) 
{ 
	*ppNew = m_pBehavior->pCreateMotorNode (); 
	*pfAddToList = false; 
}

// NetworkNodes.cpp ends here ->