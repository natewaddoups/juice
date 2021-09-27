/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsODE.dll
Filename: DynamicsODE.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceDynamicsODE.dll and its source code are distributed under the terms of 
the Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

This file defines the interface to the dynamics library.  The interface 
encapsulates as many dynamics details as possible, so that it will be
as easy as possible to replace the dynamics library with minimal impact 
on the application itself.

*****************************************************************************/

#ifndef __DynamicsODE__
#define __DynamicsODE__

#include <GenericTypes.h>

#include <ode\ode.h>
//#include "ode\ode.h"

#include <DynamicsAPI.h>

#ifdef JUICEDYNAMICSODE_EXPORTS
#define DynamicsODEExport
//__declspec (dllexport)
#else
#define DynamicsODEExport
#endif

struct dcVector3;

using namespace Generic;
using namespace Generic::XML;

namespace Juice {

class Body;
class RevoluteJoint;
class PrismaticJoint;
class SphericalJoint;
class Terrain;

namespace Dynamics {

namespace ODE {

class StandardFactory;
class IterativeFactory;
class World;
class Body;
class StandardWorld;
class IterativeWorld;
class Joint;
class RevoluteJoint;
class PrismaticJoint;
class SphericalJoint;
class ConstantsDlg;

/****************************************************************************/
/** Base class for ODE implementations of the dynamics world
*****************************************************************************/
class DynamicsODEExport ODE::World : public Dynamics::World
{
	friend class Body;
	friend class Joint;
	friend class RevoluteJoint;
	friend class PrismaticJoint;
	friend class SphericalJoint;
	friend class ConstantsDlg;
	
protected:
	/// This is a container for all of ODE's dynamics objects
	dWorldID m_WorldID;

	/// This is a container for all of ODE's collision objects
	dSpaceID m_SpaceID;

	/// This is a container for "joints" created when objects collide
	dJointGroupID m_CollisionJointGroup;

	/// Gravity
	real m_rGravity;

	/// Error reduction parameter (see ODE docs)
	real m_rERP;

	/// Constraint force mixing (see ODE docs)
	real m_rCFM;

	/// This represents the ground plane
	//dGeomID m_Ground;	

	dGeomID m_CageGround, m_CageSky;
	dGeomID m_CageLeft, m_CageRight, m_CageFront, m_CageBack;
	dGeomID m_Terrain;

	/// Vertex data in ODE's preferred form (single-precision!)
	dVector3 *m_pVertices;

	/// This wrapper is called by the physics library to get information about
	/// object collisions
	static void CollisionCallback (void *pData, dGeomID o1, dGeomID o2);

	/// This actually implements the collision callback 
	void CollisionCallback (dGeomID o1, dGeomID o2);

	/// Local copies of physical parameters (these could be fetched from the 
	/// Factory, but they're cached here instead)
	real m_rFriction, m_rBounce;

	virtual Serializable& GetConstants () { return m_Constants; }

public:

	class Constants : public Serializable
	{
	public:
		real m_rGravity;
		real m_rERP;
		real m_rCFM;
		real m_rFriction;
		real m_rBounce;

		Constants ();
		virtual void vSerialize (XML::Stream &Stream);
	};

	Constants m_Constants;

	/// The constructor takes a pointer to the factory that created the world
	World ();

	/// Destructor
	virtual ~World ();

	Dynamics::Body*           pCreateBody ();
	Dynamics::RevoluteJoint*  pCreateRevoluteJoint ();
	Dynamics::PrismaticJoint* pCreatePrismaticJoint ();
	Dynamics::SphericalJoint* pCreateSphericalJoint ();

	/// Juice uses multiple DLLs with their own heaps - the vDelete method 
	/// allows the object to be deleted from proper heap.
	virtual void vDelete ();

	/// Create the ground, ceiling and the sides of the cage
	void vCreateCage ();

	/// Add the cage to the collision world
	void vEnableCage ();

	/// Release the ground resources
	void vDestroyCage ();

	/// Create terrain
	void vCreateTerrain (Juice::Terrain *pTerrain);

	/// Add the terrain to the collision world
	void vEnableTerrain ();

	/// Release terrain
	void vDestroyTerrain ();

	/// This is where the dynamics calculations happen
	virtual bool fStep (Generic::UINT32 uMilliseconds) = 0;

	/// Prepare to begin simulation - apply physical constants from the factory class
	virtual void vPrepareSimulation ();
};

/****************************************************************************/
/** The ODE implementation of the dynamics world, with standard solver
*****************************************************************************/
class DynamicsODEExport ODE::StandardWorld : public ODE::World
{
	friend class ConstantsDlg;

public:
	/// Constructor
	StandardWorld ();

	/// Step, using the standard solver
	virtual bool fStep (Generic::UINT32 uMilliseconds);

	/// Display the physical-constants dialog box
	virtual void vConstantsDialog  ();
};

/****************************************************************************/
/** The ODE implementation of the dynamics world, with iterative solver
*****************************************************************************/
class DynamicsODEExport ODE::IterativeWorld : public ODE::World
{
	friend class ConstantsDlg;

	/// The iterative world's constants are the same as the base class, with
	/// the addition of a member to indicate how many iterations the solver
	/// should use
	class Constants : public World::Constants
	{
	public:
		Generic::UINT32 m_uIterations;

		Constants ();
		virtual void vSerialize (XML::Stream &Stream);

		Constants& operator= (ODE::World::Constants &Src)
		{
			m_rERP = Src.m_rERP;
			m_rCFM = Src.m_rCFM;
			m_rGravity = Src.m_rGravity;
			m_rFriction = Src.m_rFriction;
			m_rBounce = Src.m_rBounce;
			m_uIterations = 0;

			return *this;
		}
	};
	
	Constants m_Constants;

	virtual Serializable& GetConstants () { return m_Constants; }

public:

	/// Constructor
	IterativeWorld ();

	/// Step, using the iterative solver
	virtual bool fStep (Generic::UINT32 uMilliseconds);

	/// Prepare for simulation
	virtual void vPrepareSimulation ();

	/// Display the physical-constants dialog box
	virtual void vConstantsDialog  ();
};

/****************************************************************************/
/** Provides an application-oriented interface to rigid body functionality.
*** Presently all bodies are rectangular solids; more flexible body 
*** visualization may be implemented via derived classes.  Function names
*** ending in -Box may eventually be accompanied by similar functions ending
*** with -Cylinder or -Pyramid or whatever.
*****************************************************************************/
class DynamicsODEExport ODE::Body : public Dynamics::Body
{
	/// This exposes the m_BodyID member to support Joint::vConnectBodies
	friend class ODE::Joint;

	/// The joint needs access to the BodyID member during simulation
	friend class RevoluteJoint;

	/// The joint needs access to the BodyID member during simulation
	friend class PrismaticJoint;

	/// Constructor is protected, so only the world can create these
	friend class ODE::World;

protected:
	World *m_pWorld;
	dBodyID m_BodyID;
	dGeomID m_HullID;

	/// This may be used to tack the body to the static environment
	dJointID m_TackJoint;

	/// Constructor
	Body (ODE::World *pWorld);

	/// Destructor
	virtual ~Body ();

public:

	/// Juice uses multiple DLLs with their own heaps - the vDelete method 
	/// allows the object to be deleted from proper heap.
	virtual void vDelete ();

	/// Create the underlying object in the dynamics library
	void vCreate (Juice::Body *pOwner);

	/// Set the object's position
	void vSetPosition (const VectorXYZ &vecPosition);

	/// Get the object's position
	void vGetPosition (VectorXYZ &vecPosition);

	/// Set the object's rotation, in yaw-pitch-roll form (BUGBUG: this doesn't seem to work right)
	void vSetRotation (const VectorYPR &vecRotation);

	/// Get the object's transformation matrix, for use by OpenGL
	void vGetMatrix (Matrix4x4 &mx);

	/// Get the object's rotation, in yaw-pitch-roll form
	void vGetRotation (VectorYPR &vecRotation);

	/// Return present yaw 
	real rGetRotationYaw ();

	/// Return present pitch
	real rGetRotationPitch ();

	/// Return present roll
	real rGetRotationRoll ();

	/// Get the object's rotation, in quaternion form
	void vGetQuaternion (Quaternion &qRotation);

	/// Returns the body's angular velocity about the given axis
	real rGetAngularVelocity (const VectorXYZ &vecAxis);

	/// Return the body's linear velocity
	void vGetVelocity (VectorXYZ &vecVelocity);

	/// Set the object's collision geometry and inertial tensor to box
	void vSetFormBox (const VectorXYZ &vecDimensions, real rMass);

	/// Set the object's collision geometery and inertial tensor to cylinder
	void vSetFormCylinder (real rLength, real rRadius, real rMass);

	/// Set the object's collision geometery and inertial tensor to capsule
	void vSetFormCapsule (real rLength, real rRadius, real rMass);

	/// Set the object's collision geometery and inertial tensor to ball
	void vSetFormBall (real rRadius, real rMass);

	/// Apply an external force to the body
	void vAddForce (const VectorXYZ &vecForce);

	/// 'Tack' a body to the static environment
	void vSetStatic (bool fStatic);
};


/****************************************************************************/
/** Provides common joint functionality for subclasses; RevoluteJoint and 
    PrismaticJoint derive from this
*****************************************************************************/
class DynamicsODEExport ODE::Joint
{
protected:

	/// The world in which this joint exists
	ODE::World *m_pWorld;

	/// Identifies the joint data in the dynamics library
	dJointID m_JointID;

	/// Identifies the bodies which this joint connects (n.b. null pointers are not unusual here)
	ODE::Body *m_pBody1, *m_pBody2;

	/// Constructor
	Joint (ODE::World *pWorld);

	/// Destructor
	virtual ~Joint ();

public:

	/// Create a generic joint object (ODE's API is funny that way)
	bool fJointCreate ();

	/// Returns true only if the joint ID is valid and the joint is connected to two bodies
	bool fJointInSimulation ();

	/// Connect the given bodies with this joint
	void vJointConnectBodies (Dynamics::Body *pBody1, Dynamics::Body *pBody2);
};

/****************************************************************************/
/** Provides an application-oriented interface to revolute joint functionality.
*****************************************************************************/
class DynamicsODEExport ODE::RevoluteJoint : public Dynamics::RevoluteJoint, ODE::Joint
{
	/// Constructor is protected, so only the world can create these
	friend class ODE::World;

	/// Constructor
	RevoluteJoint (ODE::World *pWorld);

	/// Destructor
	virtual ~RevoluteJoint ();

public:

	/// Juice uses multiple DLLs with their own heaps - the vDelete method 
	/// allows the object to be deleted from proper heap.
	virtual void vDelete ();

	/// Create the dynamics object and store the address of the Juice object that owns it
	virtual void vCreate (Juice::RevoluteJoint *pOwner,
		Dynamics::Body *pBody1, Dynamics::Body *pBody2);

	/// Set the hinge position and axis (design time only)
	void vSetAnchorAndAxis (VectorXYZ &vecAnchor, VectorYPR &vecRotation);

	/// Get the current angle of the hinge
	real rGetAngle ();

	/// Get the object's position
	void vGetPosition (VectorXYZ &vecPosition);

	/// Get the object's position and orientation in matrix form for OpenGL
	void vGetMatrix (Matrix4x4 &mx);

	/// Set the joint's limits (done with motors instead)
	virtual void vSetLimits (bool fLo, real rLo, bool fHi, real rHi);

	/// Apply force to the bodies
	void vAddForce (real rVel, real rForce);

	/// Returns true only if the joint ID is valid and the joint is connected to two bodies
	virtual bool fInSimulation ();
};

/****************************************************************************/
/** Provides an application-oriented interface to prismatic joint functionality.
*****************************************************************************/
class DynamicsODEExport ODE::PrismaticJoint : public Dynamics::PrismaticJoint, ODE::Joint
{
	/// Constructor is protected, so only the world can create these
	friend class ODE::World;

	/// Constructor
	PrismaticJoint (ODE::World *pWorld);

	/// Destructor
	virtual ~PrismaticJoint ();

public:

	/// Juice uses multiple DLLs with their own heaps - the vDelete method 
	/// allows the object to be deleted from proper heap.
	virtual void vDelete ();

	/// Create the dynamics object and store the address of the Juice object that owns it
	virtual void vCreate (Juice::PrismaticJoint *pOwner,
		Dynamics::Body *pBody1, Dynamics::Body *pBody2);

	/// Set the slider's location and working axis
	void vSetAnchorAndAxis (VectorXYZ &vecAnchor, VectorYPR &vecRotation);

	/// Set the joint's limits (done with motors instead)
	virtual void vSetLimits (bool fLo, real rLo, bool fHi, real rHi);

	/// Get the slider's current length
	virtual real rGetExtension ();

	/// Get the object's position
	void vGetPosition (VectorXYZ &vecPosition);

	/// Get the slider axis (used as a helper function for GetMatrix, maybe not worth separating out?)
	void vGetAxis (VectorXYZ &vecAxis);

	/// Get the object's position and orientation in matrix form for OpenGL
	virtual void vGetMatrix (Matrix4x4 &mx, const VectorXYZ &vecBody1EndPoint, const VectorXYZ &vecBody2EndPoint);

	/// apply force to the bodies
	void vAddForce (real rVel, real rForce);

	/// Returns true only if the joint ID is valid and the joint is connected to two bodies
	virtual bool fInSimulation ();
};

/****************************************************************************/
/** Provides an application-oriented interface to spherical joint functionality.
*****************************************************************************/
class DynamicsODEExport ODE::SphericalJoint : public Dynamics::SphericalJoint, ODE::Joint
{
	/// Constructor is protected, so only the world can create these
	friend class ODE::World;

	/// Constructor
	SphericalJoint (ODE::World *pWorld);

	/// Destructor
	virtual ~SphericalJoint ();

public:

	/// Juice uses multiple DLLs with their own heaps - the vDelete method 
	/// allows the object to be deleted from proper heap.
	virtual void vDelete ();

	/// Create the dynamics object and store the address of the Juice object that owns it
	virtual void vCreate (Juice::SphericalJoint *pOwner,
		Dynamics::Body *pBody1, Dynamics::Body *pBody2);

	/// Set the joint's location in space (design time only)
	void vSetAnchor (VectorXYZ &vecAnchor);

	/// TODO: is this still used? 
	/// Get the object's position (design time or simulation time)
	void vGetAnchor (VectorXYZ &vecPosition);

	/// Get the object's position and orientation in matrix form for OpenGL
	void vGetMatrix (Matrix4x4 &mx);

	/// Returns true only if the joint ID is valid and the joint is connected to two bodies
	virtual bool fInSimulation ();

	/// Set the joint's limits (done with motors instead)
	virtual void vSetLimits (bool fLo, real rLo, bool fHi, real rHi);
};
          

}; // end namespace ODE

}; // end namespace Dynamics

}; // end namespace Juice

/// This helper function builds a matrix that will be passed off to OpenGL before drawing an object
void vBuildMatrix (Matrix4x4 &mx, const dReal *pPosition, const dReal *pOrientation);

#pragma warning (disable : 4244)
#endif 

// DynamicsODE.h ends here ->