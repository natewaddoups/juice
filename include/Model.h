/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Model.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __JuiceModel__
#define __JuiceModel__

#include <list>
#include <hash_map>
#include <GenericTypes.h>
#include <GenericProperties.h>
#include <JuiceEngine.h>
//#include <DynamicsAPI.h>

using namespace Generic;

namespace Juice {

	class Model;
	class Component;
	class Body;
	class Joint;
	class RevoluteJoint;
	class PrismaticJoint;

	class Behavior;
	class ClockworkBehavior;
	class ClockworkMotor;

	class Application;
	class SelectionSet;
	class Clipboard;

	namespace Render
	{
		class Scene;
	};

	namespace Dynamics
	{
		class World;
		class Body;
		class RevoluteJoint;
		class PrismaticJoint;
		class SphericalJoint;
	};

	/****************************************************************************/
	/* Hash-map typedefs for pointer fixing during serialization and clipboarding
	*****************************************************************************/

	typedef hash_map <Generic::UINT32, Body*> BodyHashMap;
	typedef pair <Generic::UINT32, Body*> BodyMapPair;

	typedef hash_map <Generic::UINT32, RevoluteJoint*> RevoluteJointHashMap;
	typedef pair <Generic::UINT32, RevoluteJoint*> RevoluteJointMapPair;

	typedef hash_map <Generic::UINT32, PrismaticJoint*> PrismaticJointHashMap;
	typedef pair <Generic::UINT32, PrismaticJoint*> PrismaticJointMapPair;

	typedef hash_map <Generic::UINT32, SphericalJoint*> SphericalJointHashMap;
	typedef pair <Generic::UINT32, SphericalJoint*> SphericalJointMapPair;

	typedef hash_map <Generic::UINT32, Joint*> JointHashMap;
	typedef pair <Generic::UINT32, Joint*> JointMapPair;

	/// Simple list of components
	class JuiceExport ComponentList : public list<Component*>
	{
	public:
		virtual void vGetCenter (Generic::VectorXYZ &vecCenter) const;
	};

#pragma warning (disable : 4661) // C4661

	/// XML streamable pointer list for bodies
	template <class Class>
	class XmlSplWithTwinFixing : public XML::SerializablePointerList<Class>
	{
	public:
		XmlSplWithTwinFixing (const wchar_t *wszTag) : XML::SerializablePointerList<Class> (wszTag) {}

		void vCreateHashMap (hash_map<Generic::UINT32, Class*> &Map);
		void vSetTwinPointers (hash_map<Generic::UINT32, Class*> &Map);
	};

	/// XML streamable pointer list for joints
	template <class Class>
	class XmlSplWithTwinAndBodyFixing : public XmlSplWithTwinFixing<Class>
	{
	public:
		XmlSplWithTwinAndBodyFixing (const wchar_t *wszTag) : XmlSplWithTwinFixing<Class> (wszTag) {}

		void vSetBodyPointers (hash_map<Generic::UINT32, Juice::Body*> &Map);
	};

	/// List of bodies
	class JuiceExport BodyList : public XmlSplWithTwinFixing<Body> // XmlStreamablePointerList<Body>
	{
	protected:
		Model *m_pModel;
	public:
		BodyList (Model *p) : m_pModel (p), XmlSplWithTwinFixing<Body> (L"Body") {} 

		void vCreateNew (Body **ppNew, bool *pfAddToList);

	};

	/****************************************************************************/
	/** List of joints.  This was just a std::list typedef until I realized the 
	*** pointer fixing could be useful during clipboard operations, and simpler 
	*** than creating separate revolute and prismatic lists.
	*****************************************************************************/

	class JuiceExport JointList : public XmlSplWithTwinAndBodyFixing<Juice::Joint>
	{
	public:
		JointList () : XmlSplWithTwinAndBodyFixing<Joint> (L"Joint") {}
		void vCreateNew (Juice::Joint **ppNew, bool *pfAddToList) { *ppNew = null; *pfAddToList = false; }
	};

	/// List of hinges
	class JuiceExport RevoluteJointList : public XmlSplWithTwinAndBodyFixing<RevoluteJoint>
	{
	protected:
		Model *m_pModel;
	public:
		RevoluteJointList (Model *p) : m_pModel (p), XmlSplWithTwinAndBodyFixing<RevoluteJoint> (L"RevoluteJoint") {}

		void vCreateNew (RevoluteJoint **ppNew, bool *pfAddToList);
	};

	/// List of sliders
	class JuiceExport PrismaticJointList : public XmlSplWithTwinAndBodyFixing<PrismaticJoint>
	{
	protected:
		Model *m_pModel;
	public:
		PrismaticJointList (Model *p) : m_pModel (p), XmlSplWithTwinAndBodyFixing<PrismaticJoint> (L"PrismaticJoint") {}

		void vCreateNew (PrismaticJoint **ppNew, bool *pfAddToList);
	};

	/// List of ball joints
	class JuiceExport SphericalJointList : public XmlSplWithTwinAndBodyFixing<SphericalJoint>
	{
	protected:
		Model *m_pModel;
	public:
		SphericalJointList (Model *p) : m_pModel (p), XmlSplWithTwinAndBodyFixing<SphericalJoint> (L"SphericalJoint") {}

		void vCreateNew (SphericalJoint **ppNew, bool *pfAddToList);
	};

	/// List of behaviors
	class JuiceExport BehaviorList : public XML::SerializablePolymorphicPointerList<Behavior, Model*>
	{
	public:
		BehaviorList (Model *pModel) : XML::SerializablePolymorphicPointerList<Behavior, Model*> (L"Behaviors", pModel) {}
		void vResetPointers (BodyHashMap &BodyMap, JointHashMap &JointMap);
	};

	/// The events of the model class will send to listeners
	class JuiceExport ModelEvents : public Events<Model>
	{
	public:
		virtual void Model_AddComponent (Model *pModel, Component *pComponent) {}
		virtual void Model_AddBehavior (Model *pModel, Behavior *pBehavior) {}
	};

	/// Events issued by model components
	class JuiceExport ComponentEvents : public Events<Component>
	{
	public:
		/// This event is issued when the component is changed in any way
		virtual void Component_Changed (Component *pComponent) {}
	};

	/// Events the behavior class will send to listeners
	class JuiceExport BehaviorEvents : public Events<Behavior>
	{
	public:
		virtual void Behavior_Changed (Behavior *pBehavior) {}
	};

	/****************************************************************************/
	/** A simple counter class, starts at zero and increments every time you
	*** access it.
	*****************************************************************************/
	class Counter
	{
	private:
		Generic::UINT32 m_uCount;
	public:
		Counter () : m_uCount (0) {}
		operator Generic::UINT32 () { return ++m_uCount; }
		void vReset () { m_uCount = 0; }
	};

	/****************************************************************************/
	/** This class describes the Juice 'document.'  It consists mostly of lists 
	*** of rigid bodies and joints, plus behaviors. 
	*****************************************************************************/
	class JuiceExport Model : 
		public Notifier <ModelEvents>,
		public Listener <ComponentEvents>,
		public Listener <BehaviorEvents>,
		public XML::Serializable,
		public Selectable,

		/// This application pointer allows the model to use the command history
		/// and allows the components to find out if symmetric editing is enabled
		public HasApplication
	{
		/// The behavior class must be able to look at the joint lists
		friend class Behavior;
		
		/// The behavior list class must be able to create empty behaviors
		friend class BehaviorList;

		/// Bodies
		BodyList m_Bodies;

		/// Hinges
		RevoluteJointList m_RevoluteJoints;

		/// Sliders
		PrismaticJointList m_PrismaticJoints;

		/// Ball joints
		SphericalJointList m_SphericalJoints;

		/// Behaviors
		BehaviorList m_Behaviors;
		
		/// Whether the model has been changed since the last save
		bool m_fChanged;

		/// Call this when the model has been changed
		void vSetChanged (bool fChanged = true);

		/// Reset pointers after loading a model from a file
		void vResetPointers ();

		/// This identifies the world that the model inhabits
		Dynamics::World *m_pDynamicsWorld;

		/// This will be used to name the next component added to the model
		Counter m_uComponentCount;

		/// The standard speed of the continuous motion source, in cycles per minute
		real m_rStandardSpeed;

		/// The present speed of the motion source, as a multiple of standard speed
		real m_rSpeedFactor;

		/// The present phase of the motion source
		real m_rPhase;

		/// How long to pause during the pose phase
		real m_rPoseDelay;

		/// What phase to set the motor at during the pose
		real m_rPosePhase;
		
		/// Registers the model for component events, and notifies listeners of the new component
		void vAddComponent (Component *pComponent);

		/// As above, for behaviors
		void vAddBehavior (Behavior *pBehavior);

		/// Adds a new body to the model's list
		void vAddBody (Body *pBody);

		/// Adds a new revolute joint to the model's list
		void vAddRevoluteJoint (RevoluteJoint *pRevoluteJoint);

		/// Adds a new prismatic joint to the model's list
		void vAddPrismaticJoint (PrismaticJoint *pPrismaticJoint);

		/// Adds a new spherical joint to the model's list
		void vAddSphericalJoint (SphericalJoint *pSphericalJoint);

		/// Constructs and returns a single list containing all bodies and joints
		void vBuildSingleList (ComponentList &lsAllComponents);

		/// Lets the model know that a component has changed
		void Component_Changed (Component *pComponent);

		/// Lets the model know that a behavior has changed
		void Behavior_Changed (Component *pComponent);

		/// This is called when a component is deleted
		void Notifier_Deleted(Component *pDeleted);

		/// This is called when a behavior is deleted
		void Notifier_Deleted(Behavior *pBehavior);

		/// empty override of the pure-virtual Selectable member
		PropertyTree* pGetPropertyTree () { return null; }

	public:

		/// Rendering is done in multiple passes, this enum describes each type of pass
		enum RenderMode
		{
			rmInvalid = 0,
			rmNormal,
			rmShadow
		};

	protected:

		/// Rendering is done in multiple passes, this function handles a single pass
		void vRender_Internal (Juice::Render::Scene *pScene, RenderMode eRenderMode);

	public:

		/// Constructor
		Model ();

		/// Destructor
		virtual ~Model ();

		/// Returns a const reference to the list of bodies
		const BodyList& lsGetBodies ();

		/// Returns a const reference to the list of hinges
		const RevoluteJointList& lsGetRevoluteJoints ();

		/// Returns a const reference to the list of sliders
		const PrismaticJointList& lsGetPrismaticJoints ();

		/// Returns a const reference to the list of ball joints
		const SphericalJointList& lsGetSphericalJoints ();

		/// Returns a const reference to a single list containing pointers to all bodies, hinges, and sliders
		void vGetComponents (ComponentList& lsComponents);

		/// Returns a const reference to the list of behaviors
		const BehaviorList& lsGetBehaviors ();

		/// Indicates whether or not the model has changed since it was last saved
		bool fIsChanged ();

		real rGetStandardSpeed ();
		void vSetStandardSpeed (real rSpeed);

		real rGetPoseDelay ();
		void vSetPoseDelay (real rDelay);

		real rGetPosePhase ();
		void vSetPosePhase (real rPhase);

		real rGetSpeedFactor ();
		void vSetSpeedFactor (real rSpeed);

		real rGetPhase ();
		void vSetPhase (real rPhase);

		/// Set the world that the model is to inhabit
		void vSetWorld (Dynamics::World *pWorld);

		/// Get the world that the model inhabits
		Dynamics::World* pGetWorld ();

		/// Prepare for dynamic simulation
		void vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld);

		/// Release any resources allocated during preparation
		void vDestroySimulation ();

		/// Step the model - apply motion forces
		void vStep (Generic::UINT32 uMilliseconds, ControlInput *pControls);

        /// Render the model into the scene
		void vRender (Juice::Render::Scene *pScene);

		/// Reset to empty state - delete all components, etc
		void vReset ();
	
		//bool fSetRootBody (Body *pRoot);
		//Body* pGetRootBody ();

		/// Load a new model from the given ".jm" file (deprecated - retained for compatibility with .jm files)
		bool fLoadFromFile (const char *szPath);
		
		/// Save the model to the given ".jm" file (deprecated - should probably get rid of this)
		bool fSaveToFile (const char *szPath);

		/// Load or save the model (part of the XML serialization infrastructure)
		virtual void vSerialize (XML::Stream &Stream);

		/// Send notification messages to update UI after loading a model from a file
		void vUpdateUI ();

		/// Create a single body
		Body* pCreateBody ();

		/// Create a symmetric pair of bodies
		bool fCreateBodyPair (Body** ppLeft, Body** ppRight);

		/// Create a single revolute joint
		RevoluteJoint* pCreateRevoluteJoint ();

		/// Create a symmetric pair of revolute joints
		bool fCreateRevoluteJointPair (RevoluteJoint **ppLeft, RevoluteJoint **ppRight);

		/// Create a single prismatic joint
		PrismaticJoint* pCreatePrismaticJoint ();

		/// Create a symmetric pair of prismatic joints
		bool fCreatePrismaticJointPair (PrismaticJoint **ppLeft, PrismaticJoint **ppRight);

		/// Create a single spherical joint
		SphericalJoint* pCreateSphericalJoint ();

		/// Create a symmetric pair of spherical joints
		bool fCreateSphericalJointPair (SphericalJoint **ppLeft, SphericalJoint **ppRight);

		/// New behaviors are added differently depending whether they are being loaded
		/// or just being added by the user at design time.
		enum BehaviorCreationContext
		{
			bccInvalid = 0,
			bccLoadFromFile,
			bccDesignTime
		};

		/// Add clockwork-style behavior 
		ClockworkBehavior* pCreateClockworkBehavior (BehaviorCreationContext bcc = bccDesignTime);

		/// Add network-style behavior 
		NetworkBehavior* pCreateNetworkBehavior (BehaviorCreationContext bcc = bccDesignTime);

		/// put all components into the selection set
		void vSelectAll ();

		/// Paste the given object into the model
		virtual bool fPaste (Selectable *pNewObject);

		/// returns the total number of bodies and joints
		Generic::UINT32 uGetVisibleComponentCount ();

		/// computes the extents of the model, while in design mode
		void vGetStaticBounds (Generic::VectorXYZ &vecMin, Generic::VectorXYZ &vecMax);

		/// finds the center of the model - a good 'look-at' point for the 3D view
		virtual void vGetCenter (Generic::VectorXYZ &vecCenter);
	};

	/****************************************************************************/
	/** Properties that identify different component types.
	*** Note that the 'component' type is used for the callbacks, mostly so the
	*** Component::vOnUpdateTwin doesn't need to be overridden for each of the
	*** derived component classes.
	*****************************************************************************/
	const int iPropTypeBody = 10;
	const int iPropTypeRevoluteJoint = 11;
	const int iPropTypePrismaticJoint = 12;
	const int iPropTypeSphericalJoint = 13;

	typedef Component* (Generic::HasProperties::*fnGetComponent) ();
	typedef void (Generic::HasProperties::*fnSetComponent) (Component *pComponent);

	//typedef void (Generic::HasProperties::*fnOnUpdateBody) (Body *pBody);
	//typedef void (Generic::HasProperties::*fnOnUpdateRevoluteJoint) (RevoluteJoint *pRevoluteJoint);
	//typedef void (Generic::HasProperties::*fnOnUpdatePrismaticJoint) (PrismaticJoint *pPrismaticJoint);

	/// These are used to get and set component members
	class JuiceExport ComponentProperty : public Generic::Property
	{
		fnGetComponent m_fnGet;
		fnSetComponent m_fnSet;

	public:
		ComponentProperty (const char *szName, fnGetComponent fnGet, fnSetComponent fnSet, Generic::UINT32 uFlags = 0);
		virtual ~ComponentProperty ();

		virtual const char* szGetValueToString (HasProperties *pObject);
		virtual bool fSetValueFromString (HasProperties *pObject, const char *sz);
		void vSetValue (HasProperties *pObject, Component *pComponent);
	};

	////////// TODO: could a template get the same results with far fewer lines?

	/// These are used to get and set body members
	class JuiceExport BodyProperty : public ComponentProperty
	{
	public:
		BodyProperty (const char *szName, fnGetComponent fnGet, fnSetComponent fnSet, Generic::UINT32 uFlags = 0) : 
			ComponentProperty (szName, fnGet, fnSet) {}
	};

	/// These are used to get and set members of any joint subclass
	class JuiceExport JointProperty : public ComponentProperty
	{
	public:
		JointProperty (const char *szName, fnGetComponent fnGet, fnSetComponent fnSet, Generic::UINT32 uFlags = 0) : 
			ComponentProperty (szName, fnGet, fnSet) {}
	};

	/// These are used to get and set hinge members
	class JuiceExport RevoluteJointProperty : public ComponentProperty
	{
	public:
		RevoluteJointProperty (const char *szName, fnGetComponent fnGet, fnSetComponent fnSet, Generic::UINT32 uFlags = 0) : 
			ComponentProperty (szName, fnGet, fnSet) {}
	};

	/// These are used to get and set slider members
	class JuiceExport PrismaticJointProperty : public ComponentProperty
	{
	public:
		PrismaticJointProperty (const char *szName, fnGetComponent fnGet, fnSetComponent fnSet, Generic::UINT32 uFlags = 0) : 
			ComponentProperty (szName, fnGet, fnSet) {}
	};

	/// These are used to get and set ball joint members
	class JuiceExport SphericalJointProperty : public ComponentProperty
	{
	public:
		SphericalJointProperty (const char *szName, fnGetComponent fnGet, fnSetComponent fnSet, Generic::UINT32 uFlags = 0) : 
			ComponentProperty (szName, fnGet, fnSet) {}
	};

	/****************************************************************************/
	/** Common attributes and functionality for bodies and joints.
	*****************************************************************************/

	class JuiceExport Component :
		public Notifier <ComponentEvents>,
		public Listener <ComponentEvents>,
		public XML::Serializable,
		public Selectable
	{
		/// This is necessary when the model is loaded 
		friend class Model;

		/// Necessary when copying to/from clipboard (need ctor, m_pModel and m_uID)
		friend class Clipboard;

		friend class XmlSplWithTwinFixing<Body>;
		friend class XmlSplWithTwinFixing<Joint>;
		friend class XmlSplWithTwinFixing<RevoluteJoint>;
		friend class XmlSplWithTwinFixing<PrismaticJoint>;

	protected:

		/// The model to which this component belongs
		Model *m_pModel;

		/// The twin with which this component is associated
		Component *m_pTwin;

		/// Name of this component
		string m_strName;

		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

		/// This static object holds the properties for all objects of this class
		static PropertyTreePtr ms_pPropertyTree;

		/// This is used to preserve relationships between components when 
		/// the model is saved to disk and loaded back into memory.  It holds
		/// the object's former address.  
		Generic::UINT32 m_uID;

		/// Position
		VectorXYZ m_vecPosition;

		/// Rotation
		VectorYPR m_vecRotation;

		/// This lets the component know if its twin has been deleted
		virtual void Notifier_Deleted (Component *pComponent);

		/// Rendering helper: get the color
		virtual Color* pGetColor ();

		/// Rendering helper: set the tranform matrix
		virtual void vSetTransform (Render::Scene *pScene);

		/// Rendering helper: draw the mesh
		virtual void vDrawMesh (Render::Scene *pScene);

	public:
		/// Constructor - requires pointer to model, and type ID
		Component (Model *pModel, const wchar_t *wszXmlTag);

		/// Destructor
		virtual ~Component ();

		/// Return the ID assigned to this object while it was serialized
		Generic::UINT32 uGetID () { return m_uID; }

		void vSetPosition (const VectorXYZ &vecPosition);
		virtual void vGetPosition (VectorXYZ &vecPosition);

//		void vSetRotation (const VectorYPR &vecRotation);
		virtual void vGetRotation (VectorYPR &vecRotation);

		/// Load and save
		virtual void vSerialize (XML::Stream &Stream);

		/// Render the component via OpenGL
		virtual void vRender (Juice::Render::Scene *pScene, Model::RenderMode eRenderMode);

		/// Add properties to the list
		virtual Generic::PropertyTree* pGetPropertyTree ();

		/// computes the extents of the component, while in design mode
		virtual void vGetStaticBounds (Generic::VectorXYZ &vecMin, Generic::VectorXYZ &vecMax);

		/// prepare to do dynamic simulation
		virtual void vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld);

		/// Release any resources allocated during preparation
		virtual void vDestroySimulation ();

		/// this is used when copying objects to the clipboard
		Component& operator= (const Component &Source);

		// Property manipulation
		void vSetTwin      (Component *pComponent);
		void vSetPositionX (real rX);
		void vSetPositionY (real rY);
		void vSetPositionZ (real rZ);
		void vSetRotationY (real rY);
		void vSetRotationP (real rP);
		void vSetRotationR (real rR);
		void vSetName      (const char *szName);

		Component*   pGetTwin      ();
		real         rGetPositionX ();
		real         rGetPositionY ();
		real         rGetPositionZ ();
		virtual real rGetRotationY ();
		virtual real rGetRotationP ();
		virtual real rGetRotationR ();
		const char* szGetName      ();
	};

}; // end Juice namespace

#include <Body.h>
#include <Joint.h>
#include <Behavior.h>
#include <Clockwork.h>
#include <Network.h>

#endif

// Model.h ends here ->