/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Clockwork.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __ClockworkBehavior__
#define __ClockworkBehavior__

namespace Juice
{
	class Clockwork;
	class ClockworkMotor;
	class ClockworkMotorList;

	/****************************************************************************/
	/** 
	*****************************************************************************/
	enum MotionType
	{
		mtNone = 0,
		mtSine,
		mtDoubleSine,
		mtStep,
		mtContinuous
	};

	/****************************************************************************/
	/** A property class for the motion type
	*****************************************************************************/

	const int iPropTypeMotionType = 13;

	typedef MotionType (Generic::HasProperties::*fnGetMotionType) ();
	typedef void (Generic::HasProperties::*fnSetMotionType) (MotionType eMotionType);

	class JuiceExport MotionTypeProperty : public Generic::Property
	{
		fnGetMotionType m_fnGet;
		fnSetMotionType m_fnSet;

	public:
		MotionTypeProperty (const char *zzName, fnGetMotionType fnGet, fnSetMotionType fnSet, Generic::UINT32 uFlags = 0);
		virtual ~MotionTypeProperty ();

		virtual const char* szGetValueToString (HasProperties *pObject);
		virtual bool fSetValueFromString (HasProperties *pObject, const char *sz);
		void vSetValue (HasProperties *pObject, MotionType eMotionType);
	};

	/****************************************************************************/
	/** Clockwork functions
	*****************************************************************************/

	/// Basic sine motion function
	real JuiceExport rSine       (real rPhase);

	/// Double-speed sine motion function
	real JuiceExport rDoubleSine (real rPhase);

	/// "Stepped" motion function
	real JuiceExport rStep       (real rPhase);

	/****************************************************************************/
	/** This describes the events the motor class will send to listeners
	*****************************************************************************/
	class JuiceExport ClockworkMotorEvents : public Events<ClockworkMotor>
	{
	public:
		typedef ClockworkMotor EventSource;

		virtual void ClockworkMotor_Changed (ClockworkMotor *pMotor) {}
	};

	/****************************************************************************/
	/** Class from which all motors are derived
	*****************************************************************************/
	class JuiceExport ClockworkMotor :
		public Notifier <ClockworkMotorEvents>,
		public Listener <ComponentEvents>, // (to know when joints are deleted)
		public Selectable,
		public XML::Serializable
	{
		/// this allows the behavior to reset the motor's joint pointer after loading
		friend class ClockworkBehavior;

		/// this makes vCreateNew a little more readable
		friend class ClockworkMotorList;

		/// this allows the model class dtor to clean up the property tree
		friend class Model;

		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// Which behavior this object belongs to
		ClockworkBehavior *m_pBehavior;

		/// whether or not the motor is enabled
		bool m_fEnabled;

		/// which joint the motor is attached to
		Joint *m_pJoint;

		/// The offset of the motion range, from the start angle
		real m_rStaticOffset;

		/// This lets the motion know when the associated joint is deleted
		virtual void Notifier_Deleted (Component *pComponent);

		/// This lets the motion know when the joint's name changes
		virtual void Component_Change (Component *pComponent);

		/// This will be called as the model is loaded
		void vSetJoint (Joint *pJoint);

		/// Properties
		static PropertyTreePtr ms_pPropertyTree;

		MotionType m_eMotionType;

		/// Motor phase
		real m_rPhase;

		/// Range of motion, in degrees
		real m_rAmplitude;

		/// Compute desired movement for the joint (private implementation)
		bool fGetDesiredPosition (MotionSourceType eType, real rInputPhase, real rAmpMod, real &rDesiredPosition);

	public:

		ClockworkMotor (ClockworkBehavior *pBehavior, Joint *pJoint);

		virtual ~ClockworkMotor ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// assignment operator, for use when pasting over existing clockwork motors
		ClockworkMotor& operator= (const ClockworkMotor &src);

		/// Paste in the given motion link data
		bool fPaste (Selectable *pSource);

		/// Loads from / saves to file
		void vSerialize (XML::Stream &Stream);
		static wchar_t *ms_wszXmlTag;

		ClockworkMotor *pGetTwin ();

		void vSelect ();
		void vDeselect ();

		/// Returns a pointer to the behavior that 'owns' this motor
		ClockworkBehavior* pGetBehavior ();

		/// Returns a pointer to the joint this motor drives
		Joint* pGetJoint () const;

		/// Compute desired movement for the joint
		//bool fGetDesiredPosition (real rInputPhase, real &rDesiredPosition);

		/// This is called prior to simulation 
		virtual void vPrepareSimulation ();

		/// Base class method override
		Generic::PropertyTree* pGetPropertyTree ();

		/// Property manipulation
		const char* szGetName ();

		void vSetEnableState (bool fEnable);
		bool fGetEnableState ();

		void vSetStaticOffset (real rOffset);
		real rGetStaticOffset ();

		/// There's also an 'int' version for the property get/set interface
		MotionType eGetMotionType () const;

		/// Compute desired movement for the joint (public API)
		bool fGetDesiredPosition (real rInputPhase, real &rDesiredPosition);

		void vSetSource (MotionSource *pSource, real rPhase);

		MotionSource* pGetSource ();

		// Property manipulation
		void vSetMotionType (int iMotionType);
		void vSetPhase (real rPhase);
		void vSetAmplitude (real rAmplitude);
		
		int iGetMotionType ();
		real rGetPhase ();
		real rGetAmplitude ();
	};

	/****************************************************************************/
	/** 
	*****************************************************************************/

	class JuiceExport ClockworkMotorList : public XML::SerializablePointerList<ClockworkMotor>
	{
		ClockworkBehavior *m_pBehavior;

	public:
		ClockworkMotorList (ClockworkBehavior *pBehavior) : m_pBehavior (pBehavior), XML::SerializablePointerList<ClockworkMotor> (L"ClockworkMotor", L"MotionLink") {}
		void vCreateNew (ClockworkMotor **pNew, bool *pfAddToList);
		void vSetJointPointers (hash_map<Generic::UINT32, Joint*> &Map);
	};

	class ClockworkBehavior;

	class ClockworkBehaviorEvents : public Events<ClockworkBehavior>
	{
	public:
		virtual void ClockworkBehavior_AddMotor (ClockworkBehavior *pSource, ClockworkMotor *pNewMotor) {};
	};

	/****************************************************************************/
	/** This behavior is basically a list of clockwork motors
	*****************************************************************************/
	class JuiceExport ClockworkBehavior :
		public Notifier <ClockworkBehaviorEvents>,
		public Listener <ClockworkMotorEvents>,
		public Behavior
	{
		friend class Model;

		/// Necessary when copying to/from clipboard (need ctor, m_pModel and m_uID)
		friend class Clipboard;

		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// This tells the Behavior class when to delete a motion link from the list
		virtual void Notifier_Deleted (ClockworkMotor *pLink);

		/// This tells the behavior to tell the model that it's been changed - the model sets m_fChanged
		virtual void ClockworkMotor_Changed (ClockworkMotor *pLink);

		/// This tells the behavior that something's been added to the model
		virtual void Model_AddComponent (Model *pModel, Component *pComponent);

		MotionSource m_eMotionSource;
		AmpModSource m_eAmpModSource;
		bool m_fAbsoluteValue;
		Body *m_pFeedbackSource;

		/// This static object holds the properties for all objects of this class
		static PropertyTreePtr ms_pPropertyTree;

		/// This sets m_pFeedbackSource, and ensures that the behavior will be notified if the source is deleted
		void vSetFeedbackSource_Internal (Component *pFeedbackSource);

		real rGetFeedback ();

		void vCreateMotorList (const ClockworkBehavior *pSource);

		ClockworkMotorList m_lsMotors;

	public:

		/// Should constructor be protected to allow these to be created only by the Model class?
		ClockworkBehavior (Model *pModel);

		virtual ~ClockworkBehavior ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// assignment operator, for use when pasting over existing clockwork motors
		ClockworkBehavior& operator= (const ClockworkBehavior &src);

		/// Paste in the given behavior data
		bool fPaste (Selectable *pSource);

		virtual void vPrepareSimulation ();

		/// the simulation function
		virtual void vStep (real rEnginePhase, Generic::UINT32 uMilliseconds, ControlInput *pController);

		virtual void vSerialize (XML::Stream &Stream);

		virtual void vResetPointers (BodyHashMap &BodyMap, JointHashMap &JointMap);

		static wchar_t *ms_wszXmlTag;


		/// Create a motor for each of the model's joints
		void vInitializeMotorList ();

		/// Public motor-creation function
		ClockworkMotor* pCreateMotor (Joint *pJoint);

		/// return the list of motors - for use by the Model during simulation, and for use by the UI after a model is loaded
		const ClockworkMotorList& lsGetMotors ();

		/// add motors to the UI - for use by the Model class after loading from disk, and for use by the UI when a behavior is added
		void vNotifyOfMotors ();

		ClockworkMotor* pGetMotorFor (Joint *pJoint);

		virtual void Notifier_Deleted (Component *pComponent);

		Generic::PropertyTree* pGetPropertyTree ();

		void vSetMotionSource (int iSource);
		int  iGetMotionSource ();

		void vSetAmpModSource (int iSource);
		int  iGetAmpModSource ();

		void vSetAbsoluteValue (bool fAbsoluteValue);
		bool fGetAbsoluteValue ();

		void vSetFeedbackSource (Component *pFeedbackSource);
		Component *pGetFeedbackSource ();

		virtual vector<real> *pCreateVector ();
		virtual void vSetVector (const vector<real> *pVector);
	}; // end Behavior class

}; // end Juice namespace

#endif

// Clockwork.h ends here ->