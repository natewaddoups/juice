/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Joint.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __Joint__
#define __Joint__

#include <list>
#include <GenericTypes.h>
#include <GenericProperties.h>
#include <JuiceEngine.h>
//#include <DynamicsAPI.h>

using namespace Generic;

namespace Juice {

	/****************************************************************************/
	/** This is the base class from which RevoluteJoint and PrismaticJoint are
	** derived.  Joints are the objects that bind bodies together.
	*****************************************************************************/
	class JuiceExport Joint : 
		public Component
	{
		/// This is necessary when the model is loaded 
		friend class JuiceParser;

		/// Necessary for model loading and for Model::vStep
		friend class Model;

		/// Necessary when copying to/from clipboard (need ctor, m_pModel and m_uID)
		friend class Clipboard;

		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:

		/// Identifies the first of the two bodies that the joint joins
		Body *m_pBody1;

		/// Identifies the second of the two bodies that the joint joins
		Body *m_pBody2;

		/// The maximum force available to move the joint to get the desired position
		real m_rMaxForce;

		/// The actual force used will be the ((Actual_Position - Desired_Position) * Gain * MaxForce)
		real m_rGain;

		/// Movement limit
		real m_rLoStop;
		real m_rHiStop;
		bool m_fLoStop;
		bool m_fHiStop;

		/// This static object holds the properties for all objects of this class
		static PropertyTreePtr ms_pPropertyTree;

		/// This is used to sum the result of all of the joint's position requests
		real m_rPositionAccumulator;

		/// This records the number of positions requests received during a simulation step
		Generic::UINT32 m_uPositions;

		/// This is used to sum the results of all of the joint's velocity requests
		real m_rVelocityAccumulator;

		/// This records the number of velocity requests received during a simulation step
		Generic::UINT32 m_uVelocities;

		/// This lets the joint null out the body pointers before they go stale
		virtual void Notifier_Deleted (Component *pComponent);

		/// This is called at the start of Model::vStep to initialize the desired-position accumulator to zero
		void vInitAccumulators ();

		/// This is called from vApplyForce (void) to actually figure out how much force is necessary
		virtual void vApplyForce (real rDesiredPosition);

		/// This is called by vApplyForce(void) to actually apply the force - it must be overridden in derived classes.
		virtual void vApplyForce (real rVelocity, real rMaxForce);

		/// This is used to modify the joint's dynamics object, the type of which varies in derived classes
		virtual void vUpdateDynamicsObject ();

		/// Constructor (protected, for access by the Model class only)
		Joint (Model *pModel, const wchar_t *wszXmlTag);

		/// Destructor (protected, for access by the SelectionSet class only)
		virtual ~Joint ();

	public:

		/// This is called repeatedly during Model::vStep to add a position request to the accumulator
		void vAddPosition (real rDelta);

		/// This is called repeatedly during Model::vStep to add a velocity request to the accumulator
		void vAddVelocity (real rDelta);

		/// Load and save
		virtual void vSerialize (XML::Stream &Stream);

		/// Property tree accessor
		Generic::PropertyTree* pGetPropertyTree ();

		/// prepare to do dynamic simulation
		virtual void vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld);

		/// Release resources allocated prior to simulation
		virtual void vDestroySimulation ();

		/// This is called near the end of Model::vStep to compute the force on the joint.
		void vApplyForce ();

		/// Assignment operator - primarily for use by pCreateDuplicate
		Joint& operator= (const Joint& CopyFrom);

		// Property manipulation
		virtual void vSetBody1 (Component *pBody);
		virtual void vSetBody2 (Component *pBody);
		void vSetGain (real rGain);
		void vSetMaxForce (real rMaxForce);

		void vSetLoStop (real rStop);
		void vSetHiStop (real rStop);
		void vSetLoStop (bool rStop);
		void vSetHiStop (bool rStop);

		Component* pGetBody1 ();
		Component* pGetBody2 ();
		real rGetGain ();
		real rGetMaxForce ();	

		real rGetLoStop ();
		real rGetHiStop ();
		bool fGetLoStop ();
		bool fGetHiStop ();
	};

	/****************************************************************************/
	/** Revolute joints are what non-geeks refer to as "hinges."
	*****************************************************************************/
	class JuiceExport RevoluteJoint : 
		public Joint
	{
		/// This is necessary when the model is loaded 
		friend class JuiceParser;
		friend class Model;

		/// Necessary when copying to/from clipboard (need ctor, m_pModel and m_uID)
		friend class Clipboard;

		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:
		Dynamics::RevoluteJoint *m_pDynamicsObject;

		/// The length of the cylinder that will graphically represent this joint
		real m_rLength;

		/// The diameter of the cylinder that will graphically represent this joint
		real m_rDiameter;

		/// This static object holds the properties for all objects of this class
		static PropertyTreePtr ms_pPropertyTree;

		/// Constructor (protected, for access by the Model class only)
		RevoluteJoint (Model *pModel);

		/// Destructor (protected, for access by the SelectionSet class only)
		virtual ~RevoluteJoint ();

		/// This is used to modify the joint's dynamics object, the type of which varies in derived classes
		virtual void vUpdateDynamicsObject ();

		/// Move the joint toward the given position (this is a wrapper for the two-parameter method of the same name)
		virtual void vApplyForce (real rDesiredPosition);

		/// Apply force to the joint
		virtual void vApplyForce (real rVel, real rForce);

	public:

		/// Clipboard support
		bool fPaste (Selectable *pSelectable);

		/// Read / write the joint's data to the given output stream
		virtual void vSerialize (XML::Stream &Stream);

		/// Returns the component's list of properties
		Generic::PropertyTree* pGetPropertyTree ();

		/// Rendering helper: set the tranform matrix
		virtual void vSetTransform (Render::Scene *pScene);

		/// Rendering helper: get the color
		virtual Color* pGetColor ();

		/// Rendering helper: draw the mesh
		virtual void vDrawMesh (Render::Scene *pScene);
		
		/// prepare to do dynamic simulation
		virtual void vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld);

		/// Release resources allocated prior to simulation
		virtual void vDestroySimulation ();

		/// get the current angle between the bodies to which the hinge is attached
		real rGetAngle ();

		/// Get the position of the joint
		virtual void vGetPosition (VectorXYZ &vecPosition);

		/// Get the rotation of the joint's axis angle
		virtual void vGetRotation (VectorYPR &vecRotation);

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator - primarily for use by pCreateDuplicate
		RevoluteJoint& operator= (const RevoluteJoint& CopyFrom);

		// Property manipulation
		void vSetLength (real rLength);
		void vSetDiameter (real rDiameter);

		real rGetLength ();
		real rGetDiameter ();
	};

	/****************************************************************************/
	/** Prismatic joints are what non-geeks refer to as "sliders."
	*****************************************************************************/
	class JuiceExport PrismaticJoint : 
		public Joint
	{
		/// This is necessary when the model is loaded 
		friend class JuiceParser;

		/// This allows the model to access the protected constructor
		friend class Model;

		/// This is necessary when copying to/from clipboard (need ctor, m_pModel and m_uID)
		friend class Clipboard;

		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

	protected:
		/// Interface to the joint's data in the dynamics library
		Dynamics::PrismaticJoint *m_pDynamicsObject;

		/// Length of the joint as drawn in the 3D view (has no effect on simulation)
		real m_rLength;

		/// Diameter of the joint as drawn in the 3D view (has no effect on simulation)
		real m_rDiameter;

		/// endpoint of the joint for its connection to Body1, in Body1 coordinates (for simulation rendering only)
		VectorXYZ m_vecBody1EndPoint;

		/// endpoint of the joint for its connection to Body2, in Body2 coordinates (for simulation rendering only)
		VectorXYZ m_vecBody2EndPoint;

		/// This static object holds the properties for all objects of this class
		static PropertyTreePtr ms_pPropertyTree;

	protected:

		/// Constructor (protected, for access by the Model class only)
		PrismaticJoint (Model *pModel);

		/// Destructor (protected, for access by the SelectionSet class only)
		virtual ~PrismaticJoint ();

		/// Apply force to the connected bodies
		virtual void vApplyForce (real rDesiredPosition);
		virtual void vApplyForce (real rVel, real rForce);

		/// This is used to modify the joint's dynamics object, the type of which varies in derived classes
		virtual void vUpdateDynamicsObject ();

	public:

		/// Clipboard support
		bool fPaste (Selectable *pSelectable);

		/// Save the joint data to the given output file
		virtual void vSerialize (XML::Stream &Stream);

		/// Returns the joint's property tree for use in the application's property pane
		Generic::PropertyTree* pGetPropertyTree ();

		/// Rendering helper: set the tranform matrix
		virtual void vSetTransform (Render::Scene *pScene);

		/// Rendering helper: get the color
		virtual Color* pGetColor ();

		/// Rendering helper: draw the mesh
		virtual void vDrawMesh (Render::Scene *pScene);
		
		/// prepare to do dynamic simulation
		virtual void vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld);

		/// Release resources allocated prior to simulation
		virtual void vDestroySimulation ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator - primarily for use by pCreateDuplicate
		PrismaticJoint& operator= (const PrismaticJoint& CopyFrom);

		/// Get the extension of the slider, the relative position of the connected bodies
		virtual real rGetExtension ();

		/// Get the position of the joint
		virtual void vGetPosition (VectorXYZ &vecPosition);

		// Property manipulation
		void vSetLength (real rLength);
		void vSetDiameter (real rDiameter);

		real rGetLength ();
		real rGetDiameter ();

	}; // end PrismaticJoint class

	/****************************************************************************/
	/** Spherical joints are what non-geeks refer to as "ball joints."
	*****************************************************************************/
	class JuiceExport SphericalJoint : 
		public Joint
	{
		/// This is necessary when the model is loaded 
		friend class JuiceParser;

		/// This allows the model to access the protected constructor
		friend class Model;

		/// This is necessary when copying to/from clipboard (need ctor, m_pModel and m_uID)
		friend class Clipboard;

		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

		friend class XmlSplWithTwinFixing<SphericalJoint>;

	protected:
		/// Interface to the joint's data in the dynamics library
		Dynamics::SphericalJoint *m_pDynamicsObject;

		/// Diameter of the joint as drawn in the 3D view (has no effect on simulation)
		real m_rDiameter;
		
		/// This static object holds the properties for all objects of this class
		static PropertyTreePtr ms_pPropertyTree;

		/// Constructor (protected, for access by the Model class only)
		SphericalJoint (Model *pModel);

		/// Destructor (protected, for access by the SelectionSet class only)
		virtual ~SphericalJoint ();

		/// This is used to modify the joint's dynamics object, the type of which varies in derived classes
		virtual void vUpdateDynamicsObject ();

	public:

		/// Clipboard support
		bool fPaste (Selectable *pSelectable);

		/// Save the joint data to the given output file
		virtual void vSerialize (XML::Stream &Stream);

		/// Returns the joint's property tree for use in the application's property pane
		Generic::PropertyTree* pGetPropertyTree ();

		/// Rendering helper: set the tranform matrix
		virtual void vSetTransform (Render::Scene *pScene);

		/// Rendering helper: get the color
		virtual Color* pGetColor ();

		/// Rendering helper: draw the mesh
		virtual void vDrawMesh (Render::Scene *pScene);
		
		/// prepare to do dynamic simulation
		virtual void vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld);

		/// Release resources allocated prior to simulation
		virtual void vDestroySimulation ();

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// Assignment operator - primarily for use by pCreateDuplicate
		SphericalJoint& operator= (const SphericalJoint& CopyFrom);

		/// Get the position of the joint
		virtual void vGetPosition (VectorXYZ &vecPosition);

		/// Property setter
		void vSetDiameter (real rDiameter);

		/// Property getter
		real rGetDiameter ();

	}; // end PrismaticJoint class

}; // end Juice namespace

#endif

// Joint.h ends here ->