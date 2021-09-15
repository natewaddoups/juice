/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsNovodex.dll
Filename: DynamicsNovodex.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceDynamicsNovodex.dll and its source code are distributed under the terms
of the Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __DynamicsNovodex__
#define __DynamicsNovodex__

#include <NxFoundation.h>
#include <NxRigidBody.h>
#include <NxCollision.h>

#include <GenericTypes.h>

#include <DynamicsAPI.h>

#ifdef JUICEDYNAMICSNOVODEX_EXPORTS
#define DynamicsNovodexExport 
//__declspec (dllexport)
#else
#define DynamicsNovodexExport
#endif

using namespace Generic;
using namespace Generic::XML;

namespace Juice {

class Body;
class RevoluteJoint;
class PrismaticJoint;
class SphericalJoint;
class Terrain;

namespace Dynamics {

namespace Novodex {

class World;
class Body;
class Joint;
class RevoluteJoint;
class PrismaticJoint;
class SphericalJoint;

/****************************************************************************/
/** The Novodex implementation of the dynamics world
*****************************************************************************/
class DynamicsNovodexExport Novodex::World : public Dynamics::World, NxUserContactReport, NxUserCollisionDetection
{
	friend class Body;
	friend class Joint;
	friend class RevoluteJoint;
	friend class PrismaticJoint;
	friend class SphericalJoint;

protected:

	/// This factory will be used for body and joint creation when simulation begins
	NxRigidBodySDK *m_pDynamicsFactory;

	/// This factory will be used for body and joint creation when simulation begins
	NxCollisionSDK *m_pCollisionFactory;

	/// Container for all of Novodex's dynamics objects
	NxScene *m_pScene;

	/// Container for all of Novodex's collision objects
	NxHashGridSpace *m_pSpace;

	/// Container for static collision objects
	NxShapeSet *m_pStaticShapes;

	/// Container for dynamic collision objects
	NxShapeSet *m_pDynamicShapes;

	/// Gravity effector
	NxGravityEffector *m_pGravity;

	/// This represents the ground plane
	NxPlane *m_pGround;	

	/// Terrain vertex data in Novodex's preferred form 
	NxrVec3 *m_pVertices;

	/// The rigid body SDK calls this to invoke collision detection
	void invoke (NxScene &scene);

	/// The collision SDK calls this to report a contact
	void contact (NxShape &Shape1, NxShape &Shape2, NxcReal rSeparation, 
		const NxcVec3 &vecContactPoint, const NxcVec3 &contactNormal);

	class Constants : public Serializable
	{
	public:
		Constants ();
		virtual void vSerialize (XML::Stream &Stream);

		real m_rGravity;
		real m_rBounce;
		real m_rFriction1;
		real m_rFriction2;
	};

	virtual Serializable& GetConstants () { return m_Constants; }

	Constants m_Constants;

public:
	World ();
	virtual ~World ();

	bool fInitialized ();

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
	bool fStep (Generic::UINT32 uMilliseconds);

	/// Prepare to begin simulation - apply physical constants from the factory class
	virtual void vPrepareSimulation ();

	/// The methods above are all part of the public API
	/// The methods below are 'private' to the DLL
	void vAddBody (Novodex::Body *pBody);
	void vRemoveBody (Novodex::Body *pBody);

	void vAddJoint (Novodex::Joint *pJoint);
	void vRemoveJoint (Novodex::Joint *pJoint);

	void vAddShape (NxShape *pShape);
	void vRemoveShape (NxShape *pShape);

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
class DynamicsNovodexExport Novodex::Body : public Dynamics::Body
{
	/// This exposes the m_BodyID member to support Joint::vConnectBodies
	friend class Novodex::Joint;

	/// The joint needs access to the NxBody member during vCreate
	friend class RevoluteJoint;

	/// The joint needs access to the NxBody member during vCreate
	friend class PrismaticJoint;

	/// The joint needs access to the NxBody member during vCreate
	friend class SphericalJoint;

	/// Constructor is protected, so only the factory class can create these
	friend class Novodex::World;

	/// This indicates whether the body is 'tacked' to the static environment
	bool m_fTacked;

protected:
	Juice::Body *m_pOwner;

	World *m_pWorld;
	NxBody *m_pBody;
	NxShape *m_pShape;

	NxrVec3 m_vecShapePosition;
	NxrMat33 m_mxShapeOrientation;

	/// This may be used to tack the body to the static environment
//	dJointID m_TackJoint;

	/// Update the 'pose' of the shape to follow the rigid body
	void vPoseChange ();
	
	/// Release the current collision shape
	void vReleaseShape ();

	/// Constructor
	Body (Novodex::World *pWorld);

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
class DynamicsNovodexExport Novodex::Joint
{
	friend class World;

protected:

	/// The world in which this joint exists
	Novodex::World *m_pWorld;

	/// Identifies the joint data in the dynamics library
	NxJoint *m_pJoint;

	/// Identifies the application-space bodies which this joint connects (n.b. null pointers are not unusual here)
	Novodex::Body *m_pBody1, *m_pBody2;

	/// Constructor
	Joint (Novodex::World *pWorld);

	/// Destructor
	virtual ~Joint ();

public:

	/// Do the basic stuff that has to be done when creating a joint (returns false if there's no point continuing later)
	bool fJointCreate (Juice::Joint *pOwner,
		Dynamics::Body *pBody1, Dynamics::Body *pBody2);

	/// Returns true only if the joint ID is valid and the joint is connected to two bodies
	bool fJointInSimulation ();
};

/****************************************************************************/
/** Provides an application-oriented interface to revolute joint functionality.
*****************************************************************************/
class DynamicsNovodexExport Novodex::RevoluteJoint : public Dynamics::RevoluteJoint, Novodex::Joint
{
	/// Constructor is protected, so only the factory class can create these
	friend class Novodex::World;

	/// Get a hinge object pointer from the base class' generic joint pointer
	NxHingeJoint *pGetJoint ();

	/// Constructor
	RevoluteJoint (Novodex::World *pWorld);

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
class DynamicsNovodexExport Novodex::PrismaticJoint : public Dynamics::PrismaticJoint, Novodex::Joint
{
	/// Constructor is protected, so only the factory class can create these
	friend class Novodex::World;

	/// Get a prismatic joint pointer from the base class' generic joint pointer
	NxPrismaticJoint *pGetJoint ();

	/// Constructor
	PrismaticJoint (Novodex::World *pWorld);

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
class DynamicsNovodexExport Novodex::SphericalJoint : public Juice::Dynamics::SphericalJoint, Novodex::Joint
{
	/// Constructor is protected, so only the factory class can create these
	friend class Novodex::World;

	/// Get a spherical joint pointer from the base class' generic joint pointer
	NxSphericalJoint *pGetJoint ();

	/// Constructor
	SphericalJoint (Novodex::World *pWorld);

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

/****************************************************************************/
/** This factory creates dynamics objects implemented by Novodex
*****************************************************************************/
/*class DynamicsNovodexExport Novodex::Factory : public Dynamics::Factory
{
	/// This is the factory for Novodex dynamics objects
	NxRigidBodySDK *m_pDynamicsFactory;

	/// This is the factory for Novodex collision detection objects
	NxCollisionSDK *m_pCollisionFactory;

public:
	Factory ();
	~Factory ();

	void vDelete () { delete this; }
	Dynamics::World*          pCreateWorld ();

	bool fInitialized ();

	class Constants : public Serializable
	{
	public:
		Constants ();
		virtual void vSerialize (XML::Stream &Stream);

		real m_rGravity;
		real m_rBounce;
		real m_rFriction1;
		real m_rFriction2;
	};

	virtual Serializable& GetConstants () { return m_Constants; }

protected:
	Constants m_Constants;

};*/

}; // end namespace Novodex

}; // end namespace Dynamics

}; // end namespace Juice

/// This helper function builds a matrix that will be passed off to OpenGL before drawing an object
void vBuildMatrix (Matrix4x4 &mx, const NxrVec3 &Position, const NxrMat33 &Orientation);


#endif 

// DynamicsNovodex.h ends here ->