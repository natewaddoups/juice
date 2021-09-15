/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll, JuiceDynamicsXxxx.dll
Filename: DynamicsAPI.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

******************************************************************************

This file defines the interface to the dynamics library.  The interface 
encapsulates as many dynamics details as possible, so that it will be
as easy as possible to replace the dynamics library with minimal impact 
on the application itself.

*****************************************************************************/

#ifndef __DynamicsAPI__
#define __DynamicsAPI__

#include <GenericXmlStreaming.h>
#include <GenericTypes.h>

using namespace ::Generic;
//using namespace ::Generic::XML;

namespace Juice {

class Body;
class RevoluteJoint;
class PrismaticJoint;
class SphericalJoint;
class Terrain;

/****************************************************************************/
/** These classes provide a layer of abstraction around various dynamics 
 ** libraries such as ODE and Novodex.
*****************************************************************************/

namespace Dynamics {

class World;
class Body;
class Joint;
class RevoluteJoint;
class PrismaticJoint;
class SphericalJoint;

/****************************************************************************/
/** The 'root' interface for the dynamics library, through which all other
    dynamics objects and interfaces are created.  This interface will have
	implementations for each support dynamics library (ODE, Novodex, etc)
*****************************************************************************/
/*class Factory
{
protected:
	~Factory () {}

public:
	virtual void vDelete () = 0;
	virtual Dynamics::World* pCreateWorld () = 0;
	virtual Dynamics::Body*  pCreateBody () = 0;
	virtual Dynamics::RevoluteJoint* pCreateRevoluteJoint () = 0;
	virtual Dynamics::PrismaticJoint* pCreatePrismaticJoint () = 0;
	virtual Dynamics::SphericalJoint* pCreateSphericalJoint () = 0;
	virtual Serializable& GetConstants () = 0;
};
/*
/****************************************************************************/
/** This interface defines the world in which all other dynamics objects
    interact.
*****************************************************************************/
class World
{

protected:

	/// Destructor is protected to force clients to use vDelete
	~World () {}

public:

	virtual Dynamics::Body*  pCreateBody () = 0;
	virtual Dynamics::RevoluteJoint* pCreateRevoluteJoint () = 0;
	virtual Dynamics::PrismaticJoint* pCreatePrismaticJoint () = 0;
	virtual Dynamics::SphericalJoint* pCreateSphericalJoint () = 0;
	virtual XML::Serializable& GetConstants () = 0;

	/// Juice uses multiple DLLs with their own heaps - the vDelete method 
	/// allows the object to be deleted from proper heap.
	virtual void vDelete () = 0;

	/// Create the ground, ceiling and the sides of the cage
	virtual void vCreateCage () = 0;

	/// Add the cage to the collision world
	virtual void vEnableCage () = 0;

	/// Release the ground resources
	virtual void vDestroyCage () {} 

	/// Create terrain
	virtual void vCreateTerrain (Juice::Terrain *pTerrain) = 0;

	/// Add the terrain to the collision world
	virtual void vEnableTerrain () = 0;

	/// Release terrain
	virtual void vDestroyTerrain () {}

	/// This is where the dynamics calculations happen
	virtual bool fStep (Generic::UINT32 uMilliseconds) = 0;

	/// Prepare to begin simulation - apply physical constants from the factory class
	virtual void vPrepareSimulation () = 0;

	/// Call this to update gravity, bounce, etc, from user-preference information
	virtual void vConstantsDialog () = 0;
};

/****************************************************************************/
/** Provides an application-oriented interface to rigid body functionality.
*****************************************************************************/
class Body
{
protected:

	/// Destructor is protected to clients must use vDelete
	~Body () {};

public:
	/// Juice uses multiple DLLs with their own heaps - the vDelete method 
	/// allows the object to be deleted from proper heap.
	virtual void vDelete () = 0;

	/// Create the underlying object in the dynamics library
	virtual void vCreate (Juice::Body *pOwner) = 0;

	/// Set the object's position
	virtual void vSetPosition (const VectorXYZ &vecPosition) = 0;

	/// Get the object's position
	virtual void vGetPosition (VectorXYZ &vecPosition) = 0;

	/// Set the object's rotation, in yaw-pitch-roll form
	virtual void vSetRotation (const VectorYPR &vecRotation) = 0;

	/// Get the object's rotation, in yaw-pitch-roll form
	virtual void vGetRotation (VectorYPR &vecRotation) = 0;

	/// Get the object's transformation matrix, for use by OpenGL
	virtual void vGetMatrix (Matrix4x4 &mx) = 0;

	/// Return present yaw 
	virtual real rGetRotationYaw () = 0;

	/// Return present pitch
	virtual real rGetRotationPitch () = 0;

	/// Return present roll
	virtual real rGetRotationRoll () = 0;

	/// Get the object's rotation, in quaternion form (used when computing joint positions and orientations)
	virtual void vGetQuaternion (Quaternion &qRotation) = 0;

	/// Returns the body's angular velocity about the given axis (not used but kept around as a future motion source)
	virtual real rGetAngularVelocity (const VectorXYZ &vecAxis) = 0;

	/// Return the body's linear velocity
	virtual void vGetVelocity (VectorXYZ &vecVelocity) = 0;

	/// Set the object's collision geometry and inertial tensor to box
	virtual void vSetFormBox (const VectorXYZ &vecDimensions, real rMass) = 0;

	/// Set the object's collision geometery and inertial tensor to cylinder
	virtual void vSetFormCylinder (real rLength, real rRadius, real rMass) = 0;

	/// Set the object's collision geometery and inertial tensor to capsule
	virtual void vSetFormCapsule (real rLength, real rRadius, real rMass) = 0;

	/// Set the object's collision geometery and inertial tensor to ball
	virtual void vSetFormBall (real rRadius, real rMass) = 0;

	/// Apply an external force to the body
	virtual void vAddForce (const VectorXYZ &vecForce) = 0;

	/// 'Tack' a body to the static environment
	virtual void vSetStatic (bool fStatic) = 0;
};


/****************************************************************************/
/** Provides common joint functionality for subclasses; RevoluteJoint and 
PrismaticJoint derive from this
*****************************************************************************/
class Joint
{
protected:

	/// Destructor is protected to force clients to use the vDelete method
	~Joint () {};

public:
	/// Juice uses multiple DLLs with their own heaps - the vDelete method 
	/// allows the object to be deleted from proper heap.
	virtual void vDelete () = 0;

	/// Returns true only if the joint ID is valid and the joint is connected to two bodies
	virtual bool fInSimulation () = 0;

	/// Set the joint's limits (done with motors instead)
	virtual void vSetLimits (bool fLo, real rLo, bool fHi, real rHi) = 0;
};

/****************************************************************************/
/** Provides an application-oriented interface to revolute joint functionality.
*****************************************************************************/
class RevoluteJoint : public Joint
{
public:
	/// Create the dynamics object and store the address of the Juice object that owns it
	virtual void vCreate (Juice::RevoluteJoint *pOwner, 
		Dynamics::Body *pBody1, Dynamics::Body *pBody2) = 0;

	/// Set the hinge position and axis (design time only)
	virtual void vSetAnchorAndAxis (VectorXYZ &vecAnchor, VectorYPR &vecRotation) = 0;

	/// Get the current angle of the hinge
	virtual real rGetAngle () = 0;

	/// Get the object's position
	virtual void vGetPosition (VectorXYZ &vecPosition) = 0;

	/// Get the object's transformation matrix, for use by OpenGL
	virtual void vGetMatrix (Matrix4x4 &mx) = 0;

	/// Set the joint's limits (done with motors instead)
	virtual void vSetLimits (bool fLo, real rLo, bool fHi, real rHi) = 0;

	/// Apply force to the bodies
	virtual void vAddForce (real rVel, real rForce) = 0;
};

/****************************************************************************/
/** Provides an application-oriented interface to prismatic joint functionality.
*****************************************************************************/
class PrismaticJoint : public Joint
{
public:
	/// Create the dynamics object and store the address of the Juice object that owns it
	virtual void vCreate (Juice::PrismaticJoint *pOwner,
		Dynamics::Body *pBody1, Dynamics::Body *pBody2) = 0;

	/// Set the slider's location and working axis
	virtual void vSetAnchorAndAxis (VectorXYZ &vecAnchor, VectorYPR &vecRotation) = 0;

	/// Set the joint's limits (done with motors instead)
	virtual void vSetLimits (bool fLo, real rLo, bool fHi, real rHi) = 0;

	/// get the slider's current length
	virtual real rGetExtension () = 0;

	/// Get the object's position
	virtual void vGetPosition (VectorXYZ &vecPosition) = 0;

	/// Get the slider axis (not necessary for public API, is it?)
	//virtual void vGetAxis (VectorXYZ &vecAxis) = 0;

	/// Get the object's transformation matrix, for use by OpenGL
	virtual void vGetMatrix (Matrix4x4 &mx, const VectorXYZ &vecBody1EndPoint, const VectorXYZ &vecBody2EndPoint) = 0;

	/// apply force to the bodies
	virtual void vAddForce (real rVel, real rForce) = 0;
};

/****************************************************************************/
/** Provides an application-oriented interface to spherical joint functionality.
*****************************************************************************/
class SphericalJoint : public Joint
{
public:
	/// Create the dynamics object and store the address of the Juice object that owns it
	virtual void vCreate (Juice::SphericalJoint *pOwner,
		Dynamics::Body *pBody1, Dynamics::Body *pBody2) = 0;

	/// Set the joint's location in space (design time only)
	virtual void vSetAnchor (VectorXYZ &vecAnchor) = 0;

	/// Get the object's position (design time or simulation time)
	virtual void vGetAnchor (VectorXYZ &vecPosition) = 0;
};

}; // end namespace Dynamics

}; // end namespace Juice

#endif 

// DynamicsAPI.h ends here ->