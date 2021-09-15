/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Body.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __Body__
#define __Body__

#include <list>
#include <GenericTypes.h>
#include <GenericProperties.h>
#include <JuiceEngine.h>
//#include <DynamicsAPI.h>

using namespace Generic;

namespace Juice {

	/****************************************************************************/
	/** A body is a rigid object, these are the 'lego bricks' from which models
	*** are constructed.
	*****************************************************************************/
	class JuiceExport Body : 
		public Component
	{
		/// This is necessary when the model is loaded 
		friend class Model;

		/// This is necessary to allow joints to set twin body pointers
		friend class Joint;

		/// Necessary when copying to/from clipboard (need ctor, m_pModel and m_uID)
		friend class Clipboard;

		/// This gives joint classes access to m_DynamicsObject
		friend class RevoluteJoint;
		friend class PrismaticJoint;
		friend class SphericalJoint;

	protected:
		/// Shape
		typedef enum 
		{
			sInvalid = 0,
			sBox,
			sCapsule,
			sBall,
			sCylinder,
		} Shape;

		/// Shape
		Shape m_eShape;

		/// Box Dimensions
		VectorXYZ m_vecBoxDimensions;

		/// Tube diameter
		real m_rTubeDiameter;

		/// Tube Length
		real m_rTubeLength;

		/// Ball Diameter
		real m_rBallDiameter;

		/// Mass
		real m_rMass;

		/// This object's peer in the dynamics subsystem
		Dynamics::Body *m_pDynamicsObject;

		/// This allows the Application object to delete the property tree as the application exits
		friend class Application;

		/// This static object holds the properties for all objects of this class
		static PropertyTreePtr ms_pPropertyTree;

	protected:

		/// Constructor (protected, for access by the Model class only)
		Body (Model *pModel);

		/// Destructor (protected, for access by the SelectionSet class only)
		virtual ~Body ();

		/// Rendering helper: set the tranform matrix
		virtual void vSetTransform (Render::Scene *pScene);

		/// Rendering helper: get the color
		virtual Color* pGetColor ();

		/// Rendering helper: draw the mesh
		virtual void vDrawMesh (Render::Scene *pScene);

	public:

		/// Get this object's property hierarchy
		virtual Generic::PropertyTree* pGetPropertyTree ();

		/// Get the position of the body (valid in both design and operation)
		virtual void vGetPosition (VectorXYZ &vecPosition);

		/// Get the rotation of the body (valid in both design and operation)
		virtual void vGetRotation (VectorYPR &vecRotation);

		/// Get the rotation of the body (valid in operation mode only)
		virtual void vGetQuaternion (Quaternion &qRotation);

		/// Returns the body's angular velocity about the given axis
		real rGetAngularVelocity (const VectorXYZ &vecAxis);

		/// Return the amount (in radians) that the given axis has been displaced
		/// TODO: if it runs without this, delete it, and the function body
		//real rGetAngularDisplacement (const VectorXYZ &vecOriginalAxis);

		/// Returns the body's velocity (will be zero in design mode)
		void vGetVelocity (VectorXYZ &vecVelocity);

		/// Get the bounding box of the body, in world coordinates
		virtual void vGetStaticBounds (Generic::VectorXYZ &vecMin, Generic::VectorXYZ &vecMax);

		virtual void vSerialize (XML::Stream &Stream);

		/// Prepare to do dynamic simulation
		virtual void vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld);

		/// Release resources allocated prior to simulation
		virtual void vDestroySimulation ();

		/// 'Tack' a body to the static environment
		void vSetStatic (bool fStatic);

		/// Add an external force to the body (this is only useful during simulation)
		void vAddForce (VectorXYZ &vecForce);

		/// Used when copying to and from the clipboard
		virtual Selectable* pCreateClipboardCopy () const;

		/// This supports pCreateDuplicate
		Body& operator= (const Body& CopyFrom);

		/// Clipboard support
		virtual bool fPaste (Selectable *pSelectable);

		/// This primarily for use by automated model generator code
		void vSetDimensions (const VectorXYZ &vecDimensions);

		// Property manipulation
		void vSetDimensionX (real rX);
		void vSetDimensionY (real rY);
		void vSetDimensionZ (real rZ);
		void vSetTubeDiameter   (real rDiamter);		
		void vSetTubeLength     (real rLength);
		void vSetBallDiameter   (real rDiamter);
		void vSetMass       (real rMass);
		void vSetShape      (int eShape);

		real rGetDimensionX ();
		real rGetDimensionY ();
		real rGetDimensionZ ();
		real rGetTubeDiameter ();
		real rGetTubeLength ();
		real rGetBallDiameter ();
		real rGetMass ();
		int iGetShape ();

		virtual real        rGetRotationY ();
		virtual real        rGetRotationP ();
		virtual real        rGetRotationR ();

	}; // end Body class

}; // end Juice namespace

#endif

// Body.h ends here ->