/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Network.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __NetworkBehavior__
#define __NetworkBehavior__

namespace Juice
{	
	class NetworkMotor;
	class NetworkBehavior;

	class Node;
	class Edge;

	/// Node hash maps are used to set edge->node pointers after deserializing or clipboard-copying
	typedef unordered_map<Generic::UINT32, Juice::Node*> NodeHashMap;

	/// This is an STL list with a couple extra functions for convenience
	class NodeList : public list<Node*> 
	{
	public:
		void vSetPointers (const BodyHashMap &BodyMap, const JointHashMap &JointMap);
		void vCreateHashMap (NodeHashMap &NodeMap);
		bool contains (Node *pNode);
	};

	/// This interface declares the events that nodes publish
	class NodeEvents : public Events<Node>
	{
	public:
		virtual void Node_Changed (Node *pChanged) {}
	};
	
	/****************************************************************************/
	/** This is the base class from which all network-behavior nodes are derived.
	 ** See also the Juice::Edge and Juice::NetworkBehavior classes.
	*****************************************************************************/

	class JuiceExport Node : 
		public Notifier<NodeEvents>,
		public Selectable, 
		public XML::Serializable
	{
		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// The node's activation level
		real m_rActivation;

		/// Name of the node
		string m_strName;

		/// X position in the graph
		Generic::INT32 m_iX;

		/// Y position in the graph
		Generic::INT32 m_iY;

		/// Width, in pixels
		Generic::UINT32 m_uWidth;

		/// Height, in pixels
		Generic::UINT32 m_uHeight;

		/// Node ID, as loaded from the file
		Generic::UINT32 m_uID;

		/// Number of inputs that remain unsatisfied - see NetworkBehavior::vCompileScript
		Generic::UINT32 m_uUnsatisfiedInputCount;

		/// The node's property tree
		static PropertyTreePtr ms_pPropertyTree;

		/// Returns the node's property tree
		virtual Generic::PropertyTree* pGetPropertyTree ();

	public:

		/// Constructor - the derived class must pass in its XML tag name
		Node (Selectable *pParent, const wchar_t *wszXmlTag);

		/// Destructor
		virtual ~Node ();

		/// Load/save
		virtual void vSerialize (XML::Stream &Stream);

		/// Assignment operator (primarily for use with pCreateClipboardCopy) 
		//virtual Node& operator= (const Node &src);
		virtual void operator= (const Node* pNode);

		virtual bool fPaste (Selectable *pSource);

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Initialize the node before the application enters simulation mode
		virtual void vPrepareSimulation ()
		{
			m_rActivation = 0;
			m_uUnsatisfiedInputCount = 0;
		}

		/// Prepare the node for a simulation step
		virtual void vBeginIteration ()
		{
			m_rActivation = 0;
		}

		/// Overridable function for post-simulation-step cleanup
		virtual void vEndIteration ()
		{			
		}

		/// Return the node's activation
		virtual real rGetActivation ()
		{
			return m_rActivation;
		}

		/// Add activation from another node (this is called from an Edge object)
		virtual void vAdd (real rInput)
		{
			m_rActivation += rInput;
		}

		/// Overridable function for simulation processing, called after all inputs have been satisfied
		virtual void vStep ()
		{
		}

		/// This function can be overridden to prevent the derived class from being used as an edge source
		virtual bool fCanBeAnEdgeSource () { return true; }

		/// This function can be overridden to prevent the derived class from being used as an edge destination
		virtual bool fCanBeAnEdgeDestination () { return true; }

		/// Getter for m_uUnsatisfiedInputCount
		Generic::UINT32 uGetUnsatisfiedInputCount () { return m_uUnsatisfiedInputCount; }

		/// Setter for m_uUnsatisfiedInputCount
		void vSetUnsatisfiedInputCount (Generic::UINT32 uCount) { m_uUnsatisfiedInputCount = uCount; }

		/// Return's the node's ID
		Generic::UINT32 uGetID () { return m_uID; }

		Generic::INT32 iGetX ();
		Generic::INT32 iGetY ();

		void vSetX (Generic::INT32 uX);
		void vSetY (Generic::INT32 uY);

		Generic::UINT32 uGetWidth ();
		Generic::UINT32 uGetHeight ();

		void vSetWidth (Generic::UINT32 uWidth);
		void vSetHeight (Generic::UINT32 uHeight);

		/// Property setter
		virtual void vSetName (const char *szName);

		/// Property getter
		virtual const char* szGetName ();
	};


	/// This interface declares the events that the Edge class publishes
	class EdgeEvents : public Events<Edge>
	{
	public:
		virtual void Edge_Changed (Edge *pChanged) {}
	};

	/****************************************************************************/
	/** The edge class propogates information from one node to another in 
	 ** a behavior network.  See also the Juice::Node and Juice::NetworkBehavior 
	 ** classes.
	*****************************************************************************/
	class JuiceExport Edge : 
		public Notifier <EdgeEvents>,
		public Selectable, 
		public XML::Serializable
	{
		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// The node this edge carries data from
		Node* m_pSource;

		/// The node this edge carries data to
		Node* m_pDestination;

		/// Scaling factor for data being carried
		real m_rFactor;

		/// Helper function to get the name of the source node
		const char *szGetSourceName ();

		/// Helper function to get the name of the destination node
		const char *szGetDestinationName ();

		/// The edge's property tree
		static PropertyTreePtr ms_pPropertyTree;

		/// Returns the edge's property tree
		virtual Generic::PropertyTree* pGetPropertyTree ();

	public:

		/// Constructor
		Edge (Selectable *pParent);

		/// Destructor
		virtual ~Edge ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator (primarily for use with pCreateClipboardCopy) 
		Edge& operator= (const Edge &src);

		/// Clipboard support
		virtual bool fPaste (Selectable *pSelectable);

		/// The simulation step function - carry activation from the source node to the destination node
		virtual void vStep ()
		{
			real rValue = m_pSource->rGetActivation ();
			rValue *= m_rFactor;
			m_pDestination->vAdd (rValue);
		}
		
		/// Load/save
		virtual void vSerialize (XML::Stream &Stream);

		/// Property setter
		void vSetSource (Node *pSource);

		/// Property setter
		void vSetDestination (Node *pDestination);

		/// Property getter
		Node* pGetSource ();

		/// Property getter
		Node* pGetDestination ();
	};	

	/// XmlStreamablePointerList specialized for the Juice::Edge class
	class JuiceExport EdgeList : public XML::SerializablePointerList<Edge>
	{
		NetworkBehavior *m_pBehavior;

		void vCreateNew (Edge **ppNew, bool *pfAddToList);
		
	public:
		EdgeList (NetworkBehavior *pBehavior) : 
			m_pBehavior (pBehavior), 
			XML::SerializablePointerList<Edge> (L"Edge")
			{}

		void vSetNodePointers (const NodeHashMap &NodeMap);
	};

	/****************************************************************************/
	/** The motion source node gets its activation from the main engine, a 
	 ** controller, the orientation of a body, or (soon...) the angle of a joint.
	*****************************************************************************/
	class JuiceExport MotionSourceNode : public Node, public Listener<ComponentEvents>
	{
		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

		/// Type of motion source - engine, body orientation, controller, etc
		MotionSource m_eMotionSource;

		/// Modulation source - which controller axis, if any, scales the node's activation
		AmpModSource m_eAmpModSource;

		/// Source object for body orientation information
		Body *m_pBody;

		/// If false, m_pBody is non-null but not a valid pointer - it's an ID loaded from a file
		bool m_fInitialized;

		/// Future feature: get input from joint position
		Joint *m_pJoint;

		/// Helper function to get the body orientation or joint position information
		real rGetFeedback ();

		/// Properties
		static PropertyTreePtr ms_pPropertyTree;

		/// Returns the node's property tree
		virtual Generic::PropertyTree* pGetPropertyTree ();

	public:

		/// Constructor
		MotionSourceNode (Selectable *pParent);

		/// Destructor
		~MotionSourceNode ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator (primarily for use with pCreateClipboardCopy) 
		//MotionSourceNode& operator= (const MotionSourceNode &src);
		virtual void operator= (const Node *pNode);

		/// Clipboard support
		//virtual bool fPaste (Selectable *pSelectable);

		/// Motion sources do not accept input from edges
		virtual bool fCanBeAnEdgeDestination () { return false; }

		/// Initialize the node prior to a simulation step
		virtual void vBeginIteration (real rEnginePhase, ControlInput *pController);

		/// Load/save
		virtual void vSerialize (XML::Stream &Stream);

		/// Property getter
		virtual const char* szGetName ();

		/// Property setter
		void vSetMotionSource (int iSource);

		/// Property getter
		int  iGetMotionSource ();

		/// Property setter
		void vSetAmpModSource (int iSource);

		/// Property getter
		int  iGetAmpModSource ();

		/// Property setter
		void vSetBody (Component *pFeedbackSource);

		/// Property getter
		Component *pGetBody ();

	};

	/// XmlStreamablePointerList specialized for the Juice::MotionSourceNode class
	class JuiceExport MotionSourceNodeList :
		public XML::SerializablePointerList<MotionSourceNode>
	{
		NetworkBehavior *m_pBehavior;

		void vCreateNew (MotionSourceNode **ppNew, bool *pfAddToList);
	public:
		MotionSourceNodeList (NetworkBehavior *pBehavior) : 
			m_pBehavior (pBehavior), 
			XML::SerializablePointerList<MotionSourceNode> (L"MotionSourceNode") 
			{}
	};

	/****************************************************************************/
	/** The arithmetic node class provides a flexible way to do arithmetic before
	 ** propogating the activation to another node.  For example, the output of
	 ** a wave node ranges from -1 to +1 by default, but a single ArithmeticNode
	 ** can scale that to a range of 30 degrees, offset +20 degrees for a range 
	 ** of 5 to 35.
	*****************************************************************************/
	class JuiceExport ArithmeticNode : public Node
	{
		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// Value to add to the activation before propogating it onward
		real m_rAdd;

		/// Value to multiply with the activation before propogating it onward
		real m_rMultiply;

		/// If true, add before multiplying; if false, multiply before adding
		bool m_fAddFirst;

		/// The node's property tree
		static PropertyTreePtr ms_pPropertyTree;

		/// Return the node's property tree
		virtual Generic::PropertyTree* pGetPropertyTree ();

		/// Property setter
		void vSetAddend (real rAddend);

		/// Property getter
		real rGetAddend ();

		/// Property setter
		void vSetFactor (real rFactor);

		/// Property getter
		real rGetFactor ();

		/// Property setter
		void vSetOrder (bool fAddFirst);

		/// Property getter
		bool fGetOrder ();

		/// Returns the node name
		virtual const char* szGetName ();

	public:

		/// Constructor
		ArithmeticNode (Selectable *pParent);

		/// Destructor
		~ArithmeticNode ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator (primarily for use with pCreateClipboardCopy) 
		//ArithmeticNode& operator= (const ArithmeticNode &src);
		virtual void operator= (const Node* pNode);

		/// Clipboard support
		//virtual bool fPaste (Selectable *pSelectable);

		/// Simulation step function
		virtual void vStep ();

		/// Load/save
		virtual void vSerialize (XML::Stream &Stream);

		/// XML tag
		static const wchar_t *ms_wszXmlTag;

		/// Creator function, for use by the FunctionNodeList class
		static Node* pCreate (NetworkBehavior *pBehavior, bool *pfAddToList);
	};

	/****************************************************************************/
	/** The absolute value node class simply applies a mathematical absolute value
	 ** function to the input sum.
	*****************************************************************************/

	class JuiceExport AbsoluteValueNode : public Node
	{
		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// The node's properties
		static PropertyTreePtr ms_pPropertyTree;

		/// Returns the node's properties
		virtual Generic::PropertyTree* pGetPropertyTree ();

	public:

		/// Constructor
		AbsoluteValueNode (Selectable *pParent);

		/// Destructor
		~AbsoluteValueNode ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator (primarily for use with pCreateClipboardCopy) 
		//WaveNode& operator= (const WaveNode &src);
		virtual void operator= (const Node* pNode);

		/// Clipboard support
		//virtual bool fPaste (Selectable *pSelectable);

		/// Simulation step function
		virtual void vStep ();

		/// XML tag
		static const wchar_t *ms_wszXmlTag;

		/// Creator function, for use by the FunctionNodeList class
		static Node* pCreate (NetworkBehavior *pBehavior, bool *pfAddToList);

		/// Property getter, and method of the base node class
		virtual const char* szGetName ();
	};



	/****************************************************************************/
	/** The constant node class provides a way to supply constants to the graph.
	 ** For example, if all 'knee' joints in a creature should be offset 180 
	 ** degrees from the 'hip' joints, that offset value can be specified in a 
	 ** constant node, then propogated to all of the wave nodes for the knees.
	*****************************************************************************/

	class ConstantNode : public Node
	{
		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// Value to add to the activation before propogating it onward
		real m_rValue;

		/// The node's property tree
		static PropertyTreePtr ms_pPropertyTree;

		/// Return the node's property tree
		virtual Generic::PropertyTree* pGetPropertyTree ();

		/// Property setter
		void vSetValue (real rValue);

		/// Property getter
		real rGetValue ();
		
		/// Returns the node name
		virtual const char* szGetName ();

	public:

		/// Constructor
		ConstantNode (Selectable *pParent);

		/// Destructor
		~ConstantNode ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator (primarily for use with pCreateClipboardCopy) 
		//ConstantNode& operator= (const ConstantNode &src);
		virtual void operator= (const Node* pNode);

		/// Clipboard support
		//virtual bool fPaste (Selectable *pSelectable);

		/// Constants do not accept input from edges
		virtual bool fCanBeAnEdgeDestination () { return false; }

		/// This is called before the script executes
		virtual void vBeginIteration ();

		/// Inputs to this node are ignored
		virtual void vAdd () {}

		/// Load/save
		virtual void vSerialize (XML::Stream &Stream);

		/// XML tag
		static const wchar_t *ms_wszXmlTag;

		/// Creator function, for use by the FunctionNodeList class
		static Node* pCreate (NetworkBehavior *pBehavior, bool *pfAddToList);
	};

	/****************************************************************************/
	/** The limit node class provides a way to limit the input value so the output
	 ** always lies between a specified minimum and maximum.
	*****************************************************************************/

	class LimitNode : public Node
	{
		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// Minimum allowed value
		real m_rMinimum;

		/// Maximum allowed value
		real m_rMaximum;

		/// The node's property tree
		static PropertyTreePtr ms_pPropertyTree;

		/// Return the node's property tree
		virtual Generic::PropertyTree* pGetPropertyTree ();

		/// Property setter
		void vSetMinimum (real rMinimum);

		/// Property getter
		real rGetMinimum ();

		/// Property setter
		void vSetMaximum (real rMaximum);

		/// Property getter
		real rGetMaximum ();

		/// Returns the node name
		virtual const char* szGetName ();

	public:

		/// Constructor
		LimitNode (Selectable *pParent);

		/// Destructor
		~LimitNode ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator (primarily for use with pCreateClipboardCopy) 
		virtual void operator= (const Node* pNode);

		/// This is called before the script executes
		virtual void vBeginIteration ();

		/// Script function
		virtual void vStep ();

		/// Load/save
		virtual void vSerialize (XML::Stream &Stream);

		/// XML tag
		static const wchar_t *ms_wszXmlTag;

		/// Creator function, for use by the FunctionNodeList class
		static Node* pCreate (NetworkBehavior *pBehavior, bool *pfAddToList);
	};

	/****************************************************************************/
	/** The multiply node class differs from most nodes in that its inputs are
	 ** multiplied together, rather than summed together.  It can be used in 
	 ** conjuction with a controller input node to scale another motion source
	 ** in proportion to controller input, or ir can be used in conjuction with a
	 ** constant node to, for example, scale a group of wave node outputs to 
	 ** yield the same range of motion.  The latter can also be done with the wave
	 ** node's own output range parameter, but use of constant and multiply nodes
	 ** allows a single shared scaling factor to be used by all of the wave nodes,
	 ** thus reducing data duplication and making the behavior easier to 
	 ** manipulate.
	*****************************************************************************/

	class MultiplyNode : public Node
	{
		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:
		
		/// Returns the node name
		virtual const char* szGetName ();

	public:

		/// Constructor
		MultiplyNode (Selectable *pParent);

		/// Destructor
		~MultiplyNode ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator (primarily for use with pCreateClipboardCopy) 
		//MultiplyNode& operator= (const MultiplyNode &src);
		virtual void operator= (const Node* pNode);

		/// Clipboard support
		//virtual bool fPaste (Selectable *pSelectable);

		/// This node's activation is set to 1, not 0, before each step.
		virtual void vBeginIteration ()
		{
			m_rActivation = 1;
		}

		/// In this case, the node actually multiplies rather than adding - perhaps vAdd 
		/// should be renamed to "vInput" or something like that?
		virtual void vAdd (real rInput)
		{
			m_rActivation *= rInput;
		}

		/// XML tag
		static const wchar_t *ms_wszXmlTag;

		/// Creator function, for use by the FunctionNodeList class
		static Node* pCreate (NetworkBehavior *pBehavior, bool *pfAddToList);
	};

	/****************************************************************************/
	/** The wave node class applies a sine (or sine-line) function to the given
	 ** activation.  The "sine-line" function has flat lines at the extremes, with
	 ** quick ramps between them, for use with knee joints and such.  The
	 ** duration of the ramp can be varied from 100% (sine wave) to 0% (square 
	 ** wave).
	*****************************************************************************/

	class JuiceExport WaveNode : public Node
	{
		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// This should probably be renamed...
		real m_rDutyCycle;

		/// Number of degrees of motion (effectively the joint's range of motion)
		real m_rOutputRange;

		/// Output offset (effectively a joint angle offset)
		real m_rOutputOffset;

		/// The node's properties
		static PropertyTreePtr ms_pPropertyTree;

		/// Returns the node's properties
		virtual Generic::PropertyTree* pGetPropertyTree ();

		/// Property getter
		real rGetDutyCycle ();

		/// Property setter
		void vSetDutyCycle (real rDutyCycle);		

		/// Property getter
		real rGetOutputRange ();

		/// Property setter
		void vSetOutputRange (real rRange);
		
		/// Property getter
		real rGetOutputOffset ();

		/// Property setter
		void vSetOutputOffset (real rOffset);

		/// Property getter
		virtual const char* szGetName ();

	public:

		/// Constructor
		WaveNode (Selectable *pParent);

		/// Destructor
		~WaveNode ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator (primarily for use with pCreateClipboardCopy) 
		//WaveNode& operator= (const WaveNode &src);
		virtual void operator= (const Node* pNode);

		/// Clipboard support
		//virtual bool fPaste (Selectable *pSelectable);

		/// Simulation step function
		virtual void vStep ();

		/// Load/save
		virtual void vSerialize (XML::Stream &Stream);		

		/// XML tag
		static const wchar_t *ms_wszXmlTag;

		/// Creator function, for use by the FunctionNodeList class
		static Node* pCreate (NetworkBehavior *pBehavior, bool *pfAddToList);
	};

	/// XmlStreamablePolymorphicPointerList specialized for objects derived from the Juice::Node class
	class JuiceExport FunctionNodeList : public XML::SerializablePolymorphicPointerList<Juice::Node, NetworkBehavior*>
	{
	public:
		FunctionNodeList (NetworkBehavior *pBehavior) : 
		  XML::SerializablePolymorphicPointerList<Juice::Node, NetworkBehavior*> (L"FunctionNodes", pBehavior)
		  {}
	};

	/****************************************************************************/
	/** The motor node class uses its activation to motorize a joint.  The input
	 ** activation is the desired joint angle, in degrees.
	*****************************************************************************/

	class JuiceExport MotorNode : public Node, Listener <ComponentEvents>
	{
		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:
		
		/// Different types of motorization
		typedef enum
		{
			mtInvalid = 0,
			mtPosition,
			mtVelocity
		} MotorType;

		/// Which type of motorization to use
		MotorType m_eMotorType;

		/// The joint that this node will motorize
		Joint *m_pJoint;

		/// The node's properties
		static PropertyTreePtr ms_pPropertyTree;

		/// Return the node's properties
		virtual Generic::PropertyTree* pGetPropertyTree ();

		/// This lets the node know when its joint gets deleted
		void Notifier_Deleted (Component *pJoint);

	public:

		/// Constructor
		MotorNode (Selectable *pParent);

		/// Destructor
		~MotorNode ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator (primarily for use with pCreateClipboardCopy) 
		//MotorNode& operator= (const MotorNode &src);
		virtual void operator= (const Node* pNode);

		/// Clipboard support
		//virtual bool fPaste (Selectable *pSelectable);

		/// Simulation step function
		virtual void vStep ();

		/// Load/save
		virtual void vSerialize (XML::Stream &Stream);		

		/// Property getter
		virtual const char* szGetName ();

		/// This specifically for use by NetworkBehavior::vResetPointers.
		/// It stops the joint from unregistering from m_pJoint when m_pJoint
		/// is actually an joint ID, not a valid joint pointer.
		void vSetJointNull ();

		/// Property getter
		Component *pGetJoint ();

		/// Property setter
		void vSetJoint (Component *pJoint);

		/// Property getter
		int iGetMotorType ();

		/// Property setter
		void vSetMotorType (int iMotorType);
	};

	/// XmlStreamablePointerList specialized for the Juice::MotorNode class
	class JuiceExport MotorNodeList : public XML::SerializablePointerList<MotorNode>
	{
		NetworkBehavior *m_pBehavior;
		void vCreateNew (MotorNode **ppNew, bool *pfAddToList);

	public:
		MotorNodeList (NetworkBehavior *pBehavior) : 
			XML::SerializablePointerList<MotorNode> (L"MotorNode"), 
			m_pBehavior (pBehavior) 
			{}
	};

	/****************************************************************************/
	/* TODO: move this functionality into the node and edge classes
	 * That will allow the script to take the form of a list of pointers to
	 * already-existing node and edge objects, rather than creating a new batch
	 * of Operation objects.  It will also make stepping through the script a 
	 * bit easier, as there won't be this layer of indirection.
	*****************************************************************************/

	class Operation
	{
	public:
		virtual void vExecute () = 0;
	};

	class NodeOperation : public Operation
	{
		Node *m_pNode;
	public:
		NodeOperation (Node *pNode) : m_pNode (pNode) {}
		virtual void vExecute () { m_pNode->vStep (); }
	};

	class EdgeOperation : public Operation
	{
		Edge *m_pEdge;
	public:
		EdgeOperation (Edge *pEdge) : m_pEdge (pEdge) {}
		virtual void vExecute () { m_pEdge->vStep (); }
	};

	typedef list<Operation*> OperationList;

	/// This interface declares the events published by the NetworkBehavior class
	class NetworkBehaviorEvents : public Events<NetworkBehavior>
	{
	public:
		typedef NetworkBehavior EventSource;

		/// If fHasPosition is false, the new node will be automatically centered by the view 
		/// FIXFIX: if multiple views, the last view to get this message will determine the position
		/// Should place the node in the model, not the view (i.e. in the NetworkBehavior method) with 
		/// an algorithm that gets the same result for all views.
		virtual void Behavior_AddNode (NetworkBehavior *pBehavior, Node *pNewNode, bool fHasPosition) {};
		virtual void Behavior_AddEdge (NetworkBehavior *pBehavior, Edge *pNewEdge) {};

		virtual void Behavior_RemoveNode (NetworkBehavior *pBehavior, Node *pDoomedNode) {};
		virtual void Behavior_RemoveEdge (NetworkBehavior *pBehavior, Edge *pDoomedEdge) {};
	};

	/****************************************************************************/
	/** The network behavior class lets the user specify behavior with a directed 
	 ** graph.  At simulation time, the graph is compiled to a string of 
	 ** operations (defined by nodes) on values (propogated by edges).
	*****************************************************************************/
	
	class JuiceExport NetworkBehavior :
		public Behavior,
		public Notifier <NetworkBehaviorEvents>,
		public Listener <NodeEvents>,
		public Listener <EdgeEvents>
	{
		friend class Model;

		/// Necessary when copying to/from clipboard (need ctor, m_pModel and m_uID)
		friend class Clipboard;

		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// This static object holds the properties for all objects of this class
		static PropertyTreePtr ms_pPropertyTree;

		/// Returns the behavior's property tree to the GUI
		Generic::PropertyTree* pGetPropertyTree ();

		/// List of motion source nodes
		MotionSourceNodeList m_lsMotionSourceNodes;

		/// List of function nodes
		FunctionNodeList m_lsFunctionNodes;

		/// List of motor nodes
		MotorNodeList m_lsMotorNodes;

		/// List of edges
		EdgeList m_lsEdges;

		/// Compile the graph to a script of simple operations
		void vCompileScript ();

		/// Discard the simulation script
		void vDiscardScript ();

		/// The 'script' to execute with each simulation step
		OperationList m_lsOperations;

		/// A helper function for node creation methods
		void vNodeCreated (Node *pNode, bool fHasPosition);

		/// A helper function for edge creation methods
		void vEdgeCreated (Edge *pEdge);

		/// A helper for the operator= method, which must add newly created nodes
		void vAddNode (Node *pNode);

	public:

		/// Should constructor be protected to allow these to be created only by the Model class?
		NetworkBehavior (Model *pModel);

		/// Destructor
		virtual ~NetworkBehavior ();

		/// Notify the caller (e.g. the view object) once for each node in the network
		void vReportAllObjects ();

		/// Create a new motion source node
		MotionSourceNode *pCreateMotionSourceNode ();

		/// Create a new constant-value node
		ConstantNode *pCreateConstantNode ();

		/// Create a new constant-value node
		LimitNode *pCreateLimitNode ();

		/// Create a new absolute-value node
		AbsoluteValueNode *pCreateAbsoluteValueNode ();

		/// Create a new multiply node
		MultiplyNode *pCreateMultiplyNode ();

		/// Create a new arithmetic node
		ArithmeticNode *pCreateArithmeticNode ();

		/// Create a new wave node
		WaveNode *pCreateWaveNode ();

		/// Create a new motor node
		MotorNode *pCreateMotorNode ();

		/// Create a new edge
		Edge *pCreateEdge (Node *pSource, Node *pDestination);

		/// wrapper for the function of the same name, with void return type to meet the fnInvoke signature
		void vCreateMotionSourceNode ();

		/// wrapper for the function of the same name, with void return type to meet the fnInvoke signature
		void vCreateConstantNode ();

		/// wrapper for the function of the same name, with void return type to meet the fnInvoke signature
		void vCreateLimitNode ();

		/// wrapper for the function of the same name, with void return type to meet the fnInvoke signature
		void vCreateAbsoluteValueNode ();

		/// wrapper for the function of the same name, with void return type to meet the fnInvoke signature
		void vCreateMultiplyNode ();

		/// wrapper for the function of the same name, with void return type to meet the fnInvoke signature
		void vCreateArithmeticNode ();

		/// wrapper for the function of the same name, with void return type to meet the fnInvoke signature
		void vCreateWaveNode ();

		/// wrapper for the function of the same name, with void return type to meet the fnInvoke signature
		void vCreateMotorNode ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// assignment operator, for use when pasting over existing clockwork motors
		NetworkBehavior& operator= (const NetworkBehavior &src);

		/// Clipboard support
		virtual bool fPaste (Selectable *pSelectable);

		/// This is called prior to simulation 
		virtual void vPrepareSimulation ();

		/// the simulation function
		virtual void vStep (real rEnginePhase, Generic::UINT32 uMilliseconds, ControlInput *pController);

		/// Load/save
		virtual void vSerialize (XML::Stream &Stream);

		/// Reset edge->node and motornode->joint pointers after loading from file
		virtual void vResetPointers (BodyHashMap &BodyMap, JointHashMap &JointMap);

		/// XML tag
		static const wchar_t *ms_wszXmlTag;

		/// This lets the behavior know when it's time to remove a node from one of its lists
		virtual void Notifier_Deleted (Node *pNode);

		/// This lets the behavior know when it's time to remove an edge from its edge list
		virtual void Notifier_Deleted (Edge *pEdge);

		/// This is used to propogate node change events up to the GUI, which listens for behavior changes
		virtual void Node_Changed (Node *pChanged);

		/// This is used to propogate edge change events up to the GUI, which listens for behavior changes
		virtual void Edge_Changed (Edge *pChanged);

	}; // end Behavior class

}; // end Juice namespace

#endif

// Network.h ends here ->