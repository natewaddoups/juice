/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Behavior.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __Behavior__
#define __Behavior__

#include <list>
#include <GenericTypes.h>
#include <GenericProperties.h>
#include <JuiceEngine.h>
#include <DynamicsAPI.h>

using namespace Generic;

namespace Juice {

	/****************************************************************************/
	/** This struct brings behaviors one step closer to being plugins.  
	***
	*** The model class keeps a list of these and uses them to keep track of what
	*** behavior types are available.
	***
	*** It provides a text description of the behavior class for the UI?
	*****************************************************************************/
	struct BehaviorClass
	{
		// to clean up properties
		//PropertyTree *m_pBehaviorPropertyTree;
		//PropertyTree *m_pMotorPropertyTree;

		// behavior-creation function
		Behavior* (*CreatorFunction) (Model *pModel, bool fLoadFromFile);

		const wchar_t* wszTagName;
	};

	/****************************************************************************/
	/** Base class from which other behavior classes are derived
	*****************************************************************************/
	class JuiceExport Behavior :
		public Notifier <BehaviorEvents>,
		public Listener <ModelEvents>,
		public Listener <ComponentEvents>,
		public Selectable,
		public XML::Serializable
	{
		friend class Model;

		/// Necessary when copying to/from clipboard (need ctor, m_pModel and m_uID)
		friend class Clipboard;

		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:
		std::string m_strName;
		Model *m_pModel;
		bool m_fEnabled;
		real m_rGain;

		/// Properties
		static PropertyTreePtr ms_pPropertyTree;

		Generic::PropertyTree* pGetPropertyTree ();

		/// This wraps the vNotify call - calling it directly from a derived class doesn't work
		void vChanged ();

	public:

		Behavior (Model *pModel, const wchar_t *wszXmlTag);

		~Behavior ();

		Behavior& Behavior::operator= (const Behavior &src);

		// the simulation function
		virtual void vStep (real rEnginePhase, Generic::UINT32 uMilliseconds, ControlInput *pController) = 0;

		Model* pGetModel ();

		virtual void vPrepareSimulation ();

		virtual void vSerialize (XML::Stream &Stream);

		// this is called after loading from disk
		virtual void vResetPointers (BodyHashMap &BodyMap, JointHashMap &JointMap);

		// Property manipulation
		void vSetName (const char *szName);
		const char* szGetName ();

		void vSetGain (real rGain);
		real rGetGain ();

		void vSetEnableState (bool fEnabled);
		bool fGetEnableState ();

		// Genetic algorithm support
		// Note that vector values must be normalized to +/- 1.0
		virtual std::vector<real> *pCreateVector () { return null; }
		virtual void vSetVector (const std::vector<real> *pVector) {}

	}; // end Behavior class

}; // end Juice namespace

#endif

// Behavior.h ends here ->