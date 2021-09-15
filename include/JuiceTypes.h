/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: JuiceTypes.hp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __JuiceTypes__
#define __JuiceTypes__

#ifndef JuiceExport
#ifdef JUICEENGINE_EXPORTS
#define JuiceExport __declspec (dllexport)
#else
#define JuiceExport __declspec (dllimport)
#endif
#endif

#ifdef _WINDOWS
	//#pragma warning (disable : 4251) // 
	#pragma warning (disable: 4355) // 'this' used in base class initializer
	#define VC_EXTRALEAN
	#include <windows.h>
#endif

#include <GenericProperties.h>

using namespace std;
using namespace stdext;
using namespace Generic;

namespace Juice {

	class Model;
	class Component;
	class Body;
	class Joint;
	class RevoluteJoint;
	class PrismaticJoint;
	class SphericalJoint;
	class Behavior;
	class ClockworkBehavior;
	class ClockworkMotor;
	class NetworkBehavior;

	class Application;
	class SelectionSet;
	class Clipboard;

	namespace Render
	{
		class Scene;
	};


	class Application;

	/****************************************************************************/
	/** A base class for application components that need to access the app object
	*****************************************************************************/	
	class JuiceExport HasApplication
	{
	protected:
		Application *m_pApplication;
	public:
		HasApplication () : m_pApplication (null) {}

		void vSetApplication (Application *pApplication)
		{
			m_pApplication = pApplication;
		}

		Application* pGetApplication () { return m_pApplication; }
	};

	/****************************************************************************/
	/** A base class for things that can be selected and put into the clipboard
	*****************************************************************************/

	class Selectable;

	class SelectableEvents : public Events<Selectable>
	{
		// this has no events of its own - the important thing is Notifier_Deleted
	};

	/****************************************************************************/
	/** 
	*****************************************************************************/

	class SelectionSet;

	class JuiceExport Selectable :
		public Generic::Notifier<SelectableEvents>,
		public Generic::HasProperties		
	{
	protected:
		Generic::UINT32 m_uSelectionCount;
		Selectable *m_pParent;

	public:
		Selectable (Selectable *pParent);
		virtual ~Selectable ();

		bool fSelected ();

		virtual void vSelect ();
		virtual void vDeselect ();

		Selectable *pGetParent () const { return m_pParent; }

		/// Create a copy of this object, to be placed on the clipboard
		virtual Selectable* pCreateClipboardCopy () const
		{ 
			Breakpoint ("Selectable::pCreateClipboardCopy: base class implementation invoked."); 
			return null; 
		}

		/// Paste the given object into/onto this object; return true if the paste succeeded
		virtual bool fPaste (Selectable *pNewObject)
		{
			Breakpoint ("Selectable::fPaste: base class implementation invoked."); 
			return false;
		}

		//Selectable* pGetParent ();
		//virtual void vCut (SelectionSet *pSet) {}
		//virtual void vCopy (SelectionSet *pSet) {}
		//virtual void vDelete (SelectionSet *pSet) {}
	};

	typedef list<Selectable*> SelectableList;

	/****************************************************************************/
	/** Control inputs - values may come from local hardware or via the network
	*****************************************************************************/	
	class ControlInput
	{
	public:
		void vUpdate ();

		real m_rJoystick1X;
		real m_rJoystick1Y;
		real m_rJoystick1Z;
		real m_rJoystick1R;
		bool m_fJoystick1Button1;
		bool m_fJoystick1Button2;
		bool m_fJoystick1Button3;
		bool m_fJoystick1Button4;
		int m_iJoystick1Hat;

		real m_rJoystick2X;
		real m_rJoystick2Y;
		real m_rJoystick2Z;
		real m_rJoystick2R;
		bool m_fJoystick2Button1;
		bool m_fJoystick2Button2;
		bool m_fJoystick2Button3;
		bool m_fJoystick2Button4;
		int m_iJoystick2Hat;
	};

	/****************************************************************************/
	/** Enumeration types
	*****************************************************************************/	

	enum MotionSource
	{
		msInvalid = 0,
		msMainEngine,
		msJoystick1X,
		msJoystick1Y,
		msJoystick1Z,
		msJoystick1R,
		msBodyYaw,
		msBodyPitch,
		msBodyRoll,
		msRevoluteJoint,
		msJoystick2X,
		msJoystick2Y,
		msJoystick2Z,
		msJoystick2R,
		msKeyUpDown,
		msKeyLeftRight,
	};

	enum MotionSourceType
	{
		mstInvalid = 0,
		mstEngine,
		mstControl,
		mstFeedback
	};

	enum AmpModSource
	{
		amsNone = 0,
		amsJoystickX,
		amsJoystickY,
		amsJoystickZ,
		amsJoystickR
	};

};

#endif

// JuiceTypes.h ends here ->