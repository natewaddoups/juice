/*****************************************************************************
<HEADER>
*****************************************************************************/

namespace Juice 
{ 
	namespace Dynamics 
	{ 
		class Factory;

		namespace ODE 
		{
			__declspec (dllexport) Dynamics::Factory* pCreateFactory ();
		};
	};
};

//#include <GenericTypes.h>
//#include <GenericMath.h>
#include <JuiceEngine.h>
#include <DynamicsAPI.h>

#include <ode/ode.h>
#include <DynamicsODE.h>

#include <float.h>
#include <Math.h>

#include <Terrain.h>
#include "Terrain.h"
#include <ODE/dTriList.h>

using namespace Juice;
using namespace Juice::Dynamics::ODE;

Dynamics::Factory* Juice::Dynamics::ODE::pCreateFactory ()
{
	return new Factory;
}

Factory::Factory ()
{
}

/****************************************************************************/
/** This is a bit of a hack to suppress invalid ODE memory leak reports.  The
 ** dCloseODE function will be included in the next version of ODE, and then
 ** it won't be necessary to sneak the declaration in here.
*****************************************************************************/
extern "C" void dCloseODE ();

Factory::~Factory ()
{
	dCloseODE ();
}


/// This is a helper function that builds an OpenGL-compatible matrix using 
/// ODE's position vector and orientation matrix.
void vBuildMatrix (Matrix4x4 &mx, const dReal *pPosition, const dReal *pOrientation)
{
	mx[0]  = pOrientation[0];
	mx[1]  = pOrientation[4];
	mx[2]  = pOrientation[8];
	mx[3]  = 0;
	mx[4]  = pOrientation[1];
	mx[5]  = pOrientation[5];
	mx[6]  = pOrientation[9];
	mx[7]  = 0;
	mx[8]  = pOrientation[2];
	mx[9]  = pOrientation[6];
	mx[10] = pOrientation[10];
	mx[11] = 0;
	mx[12] = pPosition[0];
	mx[13] = pPosition[1];
	mx[14] = pPosition[2];
	mx[15] = 1;
}

/****************************************************************************/
/** 
*****************************************************************************/
World::World () :
	m_WorldID (0),
	m_SpaceID (0),
	m_CageGround (0), m_CageSky (0), m_CageLeft (0), m_CageRight (0), m_CageFront (0), m_CageBack (0),
	m_Terrain (0),
	m_CollisionJointGroup (0),
	m_uLastTimerTick (0)
{
	// Create the dynamics world
	m_WorldID = dWorldCreate ();
	dWorldSetGravity (m_WorldID, 0, 0, -9.81);

	// Create the collision space
	m_SpaceID = dHashSpaceCreate ();

	// Create the collision contact joint group
	m_CollisionJointGroup = dJointGroupCreate (1000000);
}

/****************************************************************************/
/** 
*****************************************************************************/
World::~World ()
{
	vDestroyCage ();

	vDestroyTerrain ();

	if (m_CollisionJointGroup) 
	{
		dJointGroupEmpty (m_CollisionJointGroup);
		dJointGroupDestroy (m_CollisionJointGroup);
	}

	if (m_WorldID)
		dWorldDestroy (m_WorldID);

	if (m_SpaceID)
		dSpaceDestroy (m_SpaceID);
}

void World::vDelete ()
{
	delete this;
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vCreateCage ()
{
	vDestroyCage ();

	real rSize = (int) m_pApplication->m_Preferences.m_rCageSize;

	// create ground and sky
	m_CageGround = dCreatePlane (m_SpaceID, 0, 0, 1, 0);

//	m_CageSky = dCreatePlane (m_SpaceID, 0, 0, -1, rSize);

	// left and right
//	m_CageLeft = dCreatePlane (m_SpaceID, -1, 0, 0, rSize / 2);

//	m_CageRight = dCreatePlane (m_SpaceID, 1, 0, 0, -rSize / 2);

	// front and back
//	m_CageFront = dCreatePlane (m_SpaceID, 0, -1, 0, -rSize / 2);

//	m_CageBack = dCreatePlane (m_SpaceID, 01, 1, 0, rSize / 2);
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vDestroyCage ()
{
	// ground and sky
	if (m_CageGround)
	{
		dGeomDestroy (m_CageGround);
		m_CageGround = null;
	}


	if (m_CageSky)
	{
		dGeomDestroy (m_CageSky);
		m_CageSky = null;
	}

	// left and right
	if (m_CageLeft)
	{
		dGeomDestroy (m_CageLeft);
		m_CageLeft = null;
	}

	if (m_CageRight)
	{
		dGeomDestroy (m_CageRight);
		m_CageRight = null;
	}

	// front and back
	if (m_CageFront)
	{
		dGeomDestroy (m_CageFront);
		m_CageFront = null;
	}

	if (m_CageBack)
	{
		dGeomDestroy (m_CageBack);
		m_CageBack = null;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vCreateTerrain (Terrain *pTerrain)
{
	if (!pTerrain)
		return;

	Terrain::Vertex *pVertices = null;
	Terrain::Triangle *pTriangles = null;
	int iVertexCount = 0;
	int iTriangleCount = 0;

	pTerrain->vGetTriangles (&pVertices, &iVertexCount, &pTriangles, &iTriangleCount);

	if (!(pVertices && pTriangles))
		return;

	vDestroyTerrain ();

	m_Terrain = dCreateTriList (m_SpaceID, null, null);

	dGeomTriListBuild(m_Terrain, (dcVector3*) pVertices, iVertexCount, &(pTriangles->v1), iTriangleCount * 3);
}

void World::vDestroyTerrain ()
{
	if (m_Terrain)
	{
		if (dSpaceQuery (m_SpaceID, m_Terrain))
			dSpaceRemove (m_SpaceID, m_Terrain);

		dGeomDestroy (m_Terrain);
		m_Terrain = null;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vEnableCage ()
{
//	if (m_CageGround && !dSpaceQuery (m_SpaceID, m_CageGround))
//		dSpaceAdd (m_SpaceID, m_CageGround);

	if (m_Terrain && dSpaceQuery (m_SpaceID, m_Terrain))
		dSpaceRemove (m_SpaceID, m_Terrain);
}

void World::vEnableTerrain ()
{
	if (m_Terrain && !dSpaceQuery (m_SpaceID, m_Terrain))
		dSpaceAdd (m_SpaceID, m_Terrain);

//	if (m_CageGround && dSpaceQuery (m_SpaceID, m_CageGround))
//		dSpaceRemove (m_SpaceID, m_CageGround);
}

/****************************************************************************/
/** 
*****************************************************************************/
bool World::fStep (Generic::UINT32 uMilliseconds)
{
	bool fReturn = true;

	// check for collisions
	dSpaceCollide (m_SpaceID, this, CollisionCallback);

	//uMilliseconds = 1;

	// convert to seconds
	real rElapsedTime = ((real) uMilliseconds) / ((real) 1000.0);

	try 
	{
		dWorldStep (m_WorldID, rElapsedTime);
	}
	catch (...)
	{
		// This typically results from a LARGE number of contact joints 
		OutputDebugString ("Simulation failure.\n");
		fReturn = false;
	}

    // remove all contact joints
    dJointGroupEmpty (m_CollisionJointGroup);

	return fReturn;
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::CollisionCallback (void *pData, dGeomID o1, dGeomID o2)
{
	World* pThis = (World*) pData;
	if (pThis)
		pThis->CollisionCallback (o1, o2);
}

void World::CollisionCallback (dGeomID o1, dGeomID o2)
{
	if (!(o1 && o2))
	{
		Breakpoint ("CollisionCallback with null geometry\n");
		return;
	}

	// TODO: make collision filtering a user preference?
	if (0)
	{
		// ignore body-body collisions - only check for body-ground collisions
		// (to do otherwise is to invite stack overflow with large models)
		if (!((o1 == m_CageGround) || (o2 == m_CageGround)))
			return;
	}

	// ignore collisions between bodies that are connected by the same joint
	dBodyID Body1 = null, Body2 = null;

	if (o1) Body1 = dGeomGetBody (o1);
	if (o2) Body2 = dGeomGetBody (o2);

#ifdef _DEBUG
	// These are just to make debugging easier, they're not used anywhere else (yet?)
	Juice::Body *pBody1 = null;
	Juice::Body *pBody2 = null;
	
	if (Body1)
		pBody1 = (Juice::Body*) dBodyGetData (Body1);

	if (Body2)
		pBody2 = (Juice::Body*) dBodyGetData (Body2);
#endif

	if (Body1 && Body2 && dAreConnected (Body1, Body2))
		return;

	const int iContacts = 50; // ODE_MAX_CONTACTS?
	dContact contact[iContacts];

	int iCount = dCollide (o1, o2, iContacts, &contact[0].geom, sizeof (dContact));

	if (iCount)
	{
		int iMax = min (iCount, iContacts);
		for (int iContact = 0; iContact < iMax; iContact++)
		{
			contact[iContact].surface.mode = dContactBounce; 
			contact[iContact].surface.mu = m_pApplication->m_Preferences.m_rGroundFriction;
			contact[iContact].surface.bounce = m_pApplication->m_Preferences.m_rGroundBounce;
			contact[iContact].surface.bounce_vel = 0.01;
			contact[iContact].surface.slip1 = 0.1;
			contact[iContact].surface.slip2 = 0.1;
			//contact.surface.soft_erp = 0.5; // 0.5;
			//contact.surface.soft_cfm = 0.3; // 0.3;

			dJointID ContactJoint = dJointCreateContact (m_WorldID, m_CollisionJointGroup, &contact[iContact]);

			dJointAttach (ContactJoint, Body1, Body2);
		}
	}
}

void World::vChangeParameters ()
{
	dWorldSetGravity (m_WorldID, 0, 0, -m_pApplication->m_Preferences.m_rGravity);
	dWorldSetERP (m_WorldID, m_pApplication->m_Preferences.m_rGlobalERP);
	dWorldSetCFM (m_WorldID, m_pApplication->m_Preferences.m_rGlobalCFM);
}

/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::ODE::Body::Body () : 
	m_pWorld (null),
	m_BodyID (null), 
	m_HullID (null),
	m_TackJoint (null)
{
}

Dynamics::ODE::Body::~Body ()
{
	if (m_BodyID)
		dBodyDestroy (m_BodyID);

	if (m_HullID)
		dGeomDestroy (m_HullID);
}

void Dynamics::ODE::Body::vDelete ()
{
	delete this;
}

void Dynamics::ODE::Body::vCreate (Dynamics::World *pWorld, Juice::Body *pOwner)
{
	m_pWorld = dynamic_cast <ODE::World*> (pWorld);

	// destroy any existing object
	if (m_BodyID)
	{
		dBodyDestroy (m_BodyID);
		m_BodyID = 0;
	}

	// create a new object
	if (m_pWorld && !m_BodyID)
	{
		m_BodyID = dBodyCreate (m_pWorld->m_WorldID);
	}

	// prepare the new object
	if (m_BodyID)
	{
		dBodySetData (m_BodyID, pOwner);
		dBodySetLinearVel  (m_BodyID, 0, 0, 0);
		dBodySetAngularVel (m_BodyID, 0, 0, 0);
	}

	if (m_TackJoint)
	{
		dJointDestroy (m_TackJoint);
		m_TackJoint = null;
	}
}

void Dynamics::ODE::Body::vSetPosition (const VectorXYZ &vecPosition)
{
	if (!m_BodyID)
		return;
	
	dBodySetPosition (m_BodyID, vecPosition.x, vecPosition.y, vecPosition.z);
}

void Dynamics::ODE::Body::vGetPosition (VectorXYZ &vecPosition)
{
	const dReal *pPosition = null; 

	if (m_BodyID)
	{
		pPosition = dBodyGetPosition (m_BodyID);
		vecPosition.x = pPosition[0];
		vecPosition.y = pPosition[1];
		vecPosition.z = pPosition[2];
	}
	else
	{
		vecPosition.x = 0;
		vecPosition.y = 0;
		vecPosition.z = 0;
	}
}

void Dynamics::ODE::Body::vSetRotation (const VectorYPR &vecRotationDegrees)
{
	if (!m_BodyID)
		return;

	real rYaw   = rDegreesToRadians * vecRotationDegrees.yaw;
	real rPitch = rDegreesToRadians * vecRotationDegrees.pitch;
	real rRoll  = rDegreesToRadians * vecRotationDegrees.roll;

	Quaternion qYaw, qPitch, qRoll, qTotal;

	qYaw.w = cos (rYaw / 2);
	qYaw.x = qYaw.y = 0;
	qYaw.z = sin (rYaw / 2);

	qPitch.w = cos (rPitch / 2);
	qPitch.y = qPitch.z = 0;
	qPitch.x = sin (rPitch / 2);

	qRoll.w = cos (rRoll / 2);
	qRoll.x = qRoll.z = 0;
	qRoll.y = sin (rRoll / 2);

	qTotal = qYaw * qPitch * qRoll;

	dQuaternion OdeQuaternion = { qTotal.w, qTotal.x, qTotal.y, qTotal.z };

	dBodySetQuaternion (m_BodyID, OdeQuaternion);
}

/// This is suspect... not sure if Quaternion::vToEuler works right
void Dynamics::ODE::Body::vGetRotation (VectorYPR &vecRotation)
{
	if (!m_BodyID)
		return;

	Quaternion qBody;
	vGetQuaternion (qBody);

	qBody.vToEuler (vecRotation);
	vecRotation.vToDegrees ();
}

/// This is still suspect.  It's close, but I'm skeptical
void Dynamics::ODE::Body::vGetMatrix (Matrix4x4 &mx)
{
	const dReal *pOrientation = dBodyGetRotation (m_BodyID);
	const dReal *pPosition = dBodyGetPosition (m_BodyID);

	vBuildMatrix (mx, pPosition, pOrientation);
}

// If defined, inverted object will give feedback of 180 degrees
//#define AlwaysUpright

// If defined, inverted object will give feedback of zero degrees
#define ZeroInverted

real Dynamics::ODE::Body::rGetRotationYaw ()
{
	if (!m_BodyID)
		return 0;
	
	const dReal *OdeQuat = dBodyGetQuaternion (m_BodyID);
	real w = OdeQuat[0];
	real x = OdeQuat[1];
	real y = OdeQuat[2];
	real z = OdeQuat[3];

	real rQuatAngle = acos (OdeQuat[0]) * 2;

	real rTop = 2 * (w * z + y * z);
	real rBottom = ((w * w) + (x * x) - (y * y) - (z * z));
	real rAngle = atan (rTop / rBottom);

//#ifdef AlwaysUpright
	if (rBottom < 0)
		rAngle = rPi + rAngle;
//#endif

#ifdef ZeroInverted
//	if (rBottom < 0)
//		rAngle = - rAngle;
#endif

//	char szDebug[100];
//	sprintf (szDebug, "Yaw   %6.2f  Top %6.2f  Bottom %6.2f  QuatAngle %6.2f  QuatVector %6.2f %6.2f %6.2f\n", 
//		rAngle, rTop, rBottom, rQuatAngle, OdeQuat[1], OdeQuat[2], OdeQuat[3]);
//	OutputDebugString (szDebug);

	return rAngle;
}

real Dynamics::ODE::Body::rGetRotationPitch ()
{
	if (!m_BodyID)
		return 0;
	
	const dReal *OdeQuat = dBodyGetQuaternion (m_BodyID);
	real w = OdeQuat[0];
	real x = OdeQuat[1];
	real y = OdeQuat[2];
	real z = OdeQuat[3];

	real rQuatAngle = acos (OdeQuat[0]) * 2;

	real rTop = 2 * (w * x + y * z);
	real rBottom = ((w * w) - (x * x) - (y * y) + (z * z));
	real rAngle = atan (rTop / rBottom);

#ifdef AlwaysUpright
	if (rBottom < 0)
		rAngle = rPi + rAngle;
#endif

#ifdef ZeroInverted
	if (rBottom < 0)
		rAngle = - rAngle;
#endif

//	char szDebug[100];
//	sprintf (szDebug, "Pitch %6.2f  Top %6.2f  Bottom %6.2f  QuatAngle %6.2f  QuatVector %6.2f %6.2f %6.2f\n", 
//		rAngle, rTop, rBottom, rQuatAngle, OdeQuat[1], OdeQuat[2], OdeQuat[3]);
//	OutputDebugString (szDebug);

	return rAngle;
}

real Dynamics::ODE::Body::rGetRotationRoll ()
{
	if (!m_BodyID)
		return 0;
	
	const dReal *OdeQuat = dBodyGetQuaternion (m_BodyID);
	real w = OdeQuat[0];
	real x = OdeQuat[1];
	real y = OdeQuat[2];
	real z = OdeQuat[3];

	real rQuatAngle = acos (OdeQuat[0]) * 2;
	
#ifdef AlwaysUpright_Almost
	// actually buggy, but could give an 'alwaysupright' function with a little work

	Quaternion qRotation (w, x, y, z);
	VectorXYZ vecPitchAxis (1, 0, 0);
	real rPitchAngle = rGetRotationPitch ();
	Quaternion qPitch (vecPitchAxis, rPitchAngle);

	Quaternion qRoll = qRotation * qPitch.qInvert ();

	real rAngle = qRoll.rGetAngle ();

	qRoll.vNormalize ();

	char szDebug[200];
	sprintf (szDebug, "Roll  %6.2f  RollQuatVector %6.2f %6.2f %6.2f\n", 
		rAngle, qRoll.x, qRoll.y, qRoll.z);
	OutputDebugString (szDebug);

	return rAngle;
#endif

	real rLeft = x * z;
	real rRight = w * y;
	real rInput = -2 * (rLeft - rRight);
	real rAngle = asin (rInput);

	Quaternion q (w, x, y, z);
	VectorXYZ vecUp (0, 0, 1);
	vecUp.vRotateBy (q);

	if (vecUp.z == 0)
	{
		if (rAngle > 0)
		{
			OutputDebugString ("X\n");
			rAngle = rPi / 2;
		}
		else
		{
			OutputDebugString ("XXX\n");
			rAngle = -rPi / 2;
		}
	}

#ifdef AlwaysUpright
	if (vecUp.z < 0)
	{
		rAngle = rPi - rAngle;
	}
#endif

#ifdef ZeroInverted
	// no-op
#endif

//	char szDebug[200];
//	sprintf (szDebug, "Roll  %6.2f  Left %6.2f  Right %6.2f  Input %6.2f  QuatAngle %6.2f  QuatVector %6.2f %6.2f %6.2f\n", 
//		rAngle, rLeft, rRight, rInput, rQuatAngle, OdeQuat[1], OdeQuat[2], OdeQuat[3]);
//	OutputDebugString (szDebug);

	return rAngle;
}


void Dynamics::ODE::Body::vGetQuaternion (Quaternion &qRotation)
{
	if (!m_BodyID)
		return;

	const dReal *OdeQuaternion = dBodyGetQuaternion (m_BodyID);	
	qRotation.x = OdeQuaternion[0];
	qRotation.y = OdeQuaternion[1];
	qRotation.z = OdeQuaternion[2];
	qRotation.w = OdeQuaternion[3];

	qRotation.w = OdeQuaternion[0];
	qRotation.x = OdeQuaternion[1];
	qRotation.y = OdeQuaternion[2];
	qRotation.z = OdeQuaternion[3];	
}

real Dynamics::ODE::Body::rGetAngularVelocity (const VectorXYZ &vecAxis)
{
	if (!m_BodyID)
		return 0;

	dReal vecAxisLocal[3] = { vecAxis.x, vecAxis.y, vecAxis.z };

	Quaternion qBody;
	vGetQuaternion (qBody);
	qBody.qInvert ();

	const dReal* prAngularVel = dBodyGetAngularVel (m_BodyID);
	VectorXYZ vecAngularVelocity ((real*) prAngularVel);

	vecAngularVelocity.vRotateBy (qBody);

	if (vecAxis.x == 1)
		return vecAngularVelocity.x;

	if (vecAxis.y == 1)
		return vecAngularVelocity.y;

	if (vecAxis.z == 1)
		return vecAngularVelocity.z;
	
	return 0;
}

real Dynamics::ODE::Body::rGetAngularDisplacement (const VectorXYZ &vecOriginalAxis)
{
	if (!m_BodyID)
		return 0;

	Quaternion qBody;
	vGetQuaternion (qBody);

	VectorXYZ vecNewAxis = vecOriginalAxis;
	vecNewAxis.vRotateBy (qBody);

	return rRadiansBetween (vecNewAxis, vecOriginalAxis);
}

void Dynamics::ODE::Body::vGetVelocity (VectorXYZ &vecVelocity)
{
	if (!m_BodyID)
		return;

	const dReal *pVel = dBodyGetLinearVel (m_BodyID);

	if (pVel)
		vecVelocity.vSet (pVel[0], pVel[1], pVel[2]);
	else
		vecVelocity.vSet (0, 0, 0);
}

void Dynamics::ODE::Body::vSetCollisionBox (const VectorXYZ &vecDimensions)
{
	if (m_HullID)
		dGeomDestroy (m_HullID);

	if (!m_BodyID)
		return;

	m_HullID = dCreateBox (m_pWorld->m_SpaceID, vecDimensions.x, vecDimensions.y, vecDimensions.z);
	dGeomSetBody (m_HullID, m_BodyID);
}

void Dynamics::ODE::Body::vSetMassBox (real rMass, const VectorXYZ &vecDimensions)
{
	if (!m_BodyID)
		return;

	dMass mass;
	dMassSetBox (&mass, 1, vecDimensions.x, vecDimensions.y, vecDimensions.z);
	dMassAdjust (&mass, rMass);
	dBodySetMass (m_BodyID, &mass);
}

void Dynamics::ODE::Body::vSetCollisionTube (real rLength, real rRadius)
{
	if (m_HullID)
		dGeomDestroy (m_HullID);

	if (!m_BodyID)
		return;

	m_HullID = dCreateCCylinder (m_pWorld->m_SpaceID, rRadius, rLength);
	dGeomSetBody (m_HullID, m_BodyID);
}

void Dynamics::ODE::Body::vSetMassTube (real rMass, real rLength, real rRadius)
{
	if (!m_BodyID)
		return;

	dMass mass;
	dMassSetCappedCylinder (&mass, 1, 3, rRadius, rLength);
	dMassAdjust (&mass, rMass);
	dBodySetMass (m_BodyID, &mass);
}

void Dynamics::ODE::Body::vSetCollisionBall (real rRadius)
{
	if (m_HullID)
		dGeomDestroy (m_HullID);

	if (!m_BodyID)
		return;

	m_HullID = dCreateSphere (m_pWorld->m_SpaceID, rRadius);
	dGeomSetBody (m_HullID, m_BodyID);
}

void Dynamics::ODE::Body::vSetMassBall (real rMass, real rRadius)
{
	if (!m_BodyID)
		return;

	dMass mass;
	dMassSetSphere (&mass, 1, rRadius);
	dMassAdjust (&mass, rMass);
	dBodySetMass (m_BodyID, &mass);
}

void Dynamics::ODE::Body::vAddForce (const VectorXYZ &vecForce)
{
	if (!m_BodyID)
		return;

	dBodyAddForce (m_BodyID, vecForce.x, vecForce.y, vecForce.z);
}

void Dynamics::ODE::Body::vSetStatic (bool fStatic)
{
	if (!m_BodyID)
		return;

	if (fStatic)
	{
		if (!m_TackJoint)
			m_TackJoint = dJointCreateFixed (m_pWorld->m_WorldID, 0);

		if (m_TackJoint)
		{
			dJointAttach (m_TackJoint, m_BodyID, null);
			dJointSetFixed (m_TackJoint);
		}
	}
	else
	{
		if (m_TackJoint)
		{
			dJointDestroy (m_TackJoint);
			m_TackJoint = null;
		}
	}
}


/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::ODE::Joint::Joint () : 
	m_JointID (null),
	m_pWorld (null),
	m_pBody1 (null), 
	m_pBody2 (null)
{
}

Dynamics::ODE::Joint::~Joint ()
{
	if (m_JointID)
		dJointDestroy (m_JointID);
}

void Dynamics::ODE::Joint::vCreate (ODE::World *pWorld)
{
	// store the pointer to the current world
	m_pWorld = pWorld;

	// destroy the current joint
	if (m_JointID)
		dJointDestroy (m_JointID);

	// The ID will be initialized in the derived class vCreate
	m_JointID = 0;
}

void Dynamics::ODE::Joint::vJointConnectBodies (ODE::Body *pBody1, ODE::Body *pBody2)
{
	// only do this if both bodies are valid 
	if ((pBody1 && pBody2) && (pBody1->m_BodyID && pBody2->m_BodyID) && (pBody1->m_BodyID != pBody2->m_BodyID))
	{
		// store pointers to the bodies
		m_pBody1 = pBody1;
		m_pBody2 = pBody2;

		// make the association in the physics library
		dJointAttach (m_JointID, pBody1->m_BodyID, pBody2->m_BodyID);
	}
	else
	{
		// if either is invalid, set both pointers to null
		m_pBody1 = m_pBody2 = null;
		dJointAttach (m_JointID, null, null);
	}
}

bool Dynamics::ODE::Joint::fJointInSimulation ()
{
	if (m_JointID && m_pBody1 && m_pBody2)
		return true;

	return false;
}


/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::ODE::RevoluteJoint::RevoluteJoint ()
{
}

Dynamics::ODE::RevoluteJoint::~RevoluteJoint ()
{
}

void Dynamics::ODE::RevoluteJoint::vDelete ()
{
	delete this;
}

bool Dynamics::ODE::RevoluteJoint::fInSimulation ()
{
	return fJointInSimulation ();
}

void Dynamics::ODE::RevoluteJoint::vConnectBodies (Dynamics::Body *pBody1, Dynamics::Body *pBody2)
{
	return vJointConnectBodies (dynamic_cast <ODE::Body*> (pBody1), dynamic_cast <ODE::Body*> (pBody2));
}


void Dynamics::ODE::RevoluteJoint::vCreate (Dynamics::World *pWorld, Juice::RevoluteJoint *pOwner)
{
	Dynamics::ODE::Joint::vCreate (dynamic_cast <ODE::World*> (pWorld));

	if (m_pWorld && !m_JointID)
	{
		m_JointID = dJointCreateHinge (m_pWorld->m_WorldID, 0);
		dJointSetData (m_JointID, pOwner);
	}
}

real Dynamics::ODE::RevoluteJoint::rGetAngle ()
{
	if (!fInSimulation ())
		return 0;

	return dJointGetHingeAngle (m_JointID);
}

void Dynamics::ODE::RevoluteJoint::vGetPosition (VectorXYZ &vecPosition)
{
	dReal pPosition[3]; 

	if (fInSimulation ())
	{
		dJointGetHingeAnchor (m_JointID, pPosition);
		vecPosition.x = pPosition[0];
		vecPosition.y = pPosition[1];
		vecPosition.z = pPosition[2];
	}
	else
	{
		vecPosition.x = 0;
		vecPosition.y = 0;
		vecPosition.z = 0;
	}
}

// TODO: is this used anymore?
void Dynamics::ODE::RevoluteJoint::vGetRotation (VectorYPR &vecRotation)
{
	dReal Axis[3]; 

	if (fInSimulation ())
	{
		// get the axis vector
		dJointGetHingeAxis (m_JointID, Axis);

		// convert to rotations - note that yaw is ignored
		vecRotation.yaw = 0;

		VectorXYZ vecStart (0, 0, 1), vecTarget (Axis[0], Axis[1], Axis[2]);
		real rTheta = acos (DotProduct (vecStart, vecTarget));
		vecRotation.pitch = rTheta;
		vecRotation.roll = 0;

		vecRotation.vToDegrees ();
		vecStart.vRotateByDegrees (vecRotation);
		vecRotation.vToRadians ();

		vecRotation.roll = acos (DotProduct (vecStart, vecTarget));

		vecRotation.vToDegrees ();
	}
	else
	{
		vecRotation.yaw = 0;
		vecRotation.pitch = 0;
		vecRotation.roll = 0;
	}
}

void Dynamics::ODE::RevoluteJoint::vGetMatrix (Matrix4x4 &mx)
{
	dReal Position[3]; 
	dReal Rotation[12];

	dReal Axis[3];
	dJointGetHingeAxis (m_JointID, Axis);

	// get vector orthogonal to default joint axis (0,0,1) and current axis
	VectorXYZ vecDefault (0, 0, 1);
	VectorXYZ vecTarget (Axis[0], Axis[1], Axis[2]);
	VectorXYZ vecAxis = CrossProduct (vecDefault, vecTarget);

	// get rotation along that vector
	real rTheta = acos (DotProduct (vecDefault, vecTarget));

	dRFromAxisAndAngle (Rotation, vecAxis.x, vecAxis.y, vecAxis.z, rTheta);

	dJointGetHingeAnchor (m_JointID, Position);

	vBuildMatrix (mx, Position, Rotation);

	//pScene->vPushMatrix ((const real*) Position, (const real*) Rotation);
}

/****************************************************************************/
/** 

This should build a single quaternion with yaw pitch and roll, but it 
doesn't come out quite right.  Maybe some axes are swapped?

	// From http://www.nearfield.com/~dan/misc/Quater/

	Quaternion qTotal2;

	qTotal2.w = cos(rYaw/2) * cos(rPitch/2) * cos(rRoll/2)
	         + sin(rYaw/2) * sin(rPitch/2) * sin(rRoll/2);

	qTotal2.x = cos(rYaw/2) * cos(rPitch/2) * sin(rRoll/2) 
	         - sin(rYaw/2) * sin(rPitch/2) * cos(rRoll/2);

	qTotal2.y = cos(rYaw/2) * sin(rPitch/2) * cos(rRoll/2) 
	         + sin(rYaw/2) * cos(rPitch/2) * sin(rRoll/2);

	qTotal2.z = sin(rYaw/2) * cos(rPitch/2) * cos(rRoll/2) 
	         - cos(rYaw/2) * sin(rPitch/2) * sin(rRoll/2);
			 

Whatever the case, it's kind of nice having the yaw/pitch/roll sequence
spelled out explicitly below, as "qTotal = qYaw * qPitch * qRoll;"

*****************************************************************************/

void Dynamics::ODE::RevoluteJoint::vSetAnchorAndAxis (VectorXYZ &vecAnchor, VectorYPR &vecOrientationDegrees)
{
	if (!m_JointID)
		return;

	// set anchor
	dJointSetHingeAnchor (m_JointID, vecAnchor.x, vecAnchor.y, vecAnchor.z);

	VectorXYZ vecOrientation (0, 0, 1);
	vecOrientation.vRotateByDegrees (vecOrientationDegrees);

	// set the axis
	dJointSetHingeAxis (m_JointID, vecOrientation.x, vecOrientation.y, vecOrientation.z);
}

void Dynamics::ODE::RevoluteJoint::vSetLimits (bool fLo, real rLo, bool fHi, real rHi)
{
	if (!fInSimulation ())
		return;

	if (fLo)
	{
		dJointSetHingeParam (m_JointID, dParamLoStop, rLo * rDegreesToRadians);
	}
	else
	{
		dJointSetHingeParam (m_JointID, dParamLoStop, -dInfinity);
	}

	if (fHi)
	{
		dJointSetHingeParam (m_JointID, dParamHiStop, rHi * rDegreesToRadians);
	}
	else
	{
		dJointSetHingeParam (m_JointID, dParamHiStop, dInfinity);
	}
}

void Dynamics::ODE::RevoluteJoint::vAddForce (real rVel, real rForce)
{
	if (!fInSimulation ())
		return;

	if (!_isnan (rVel) && !_isnan (rForce))
	{
		dJointSetHingeParam (m_JointID, dParamVel, rVel);
		dJointSetHingeParam (m_JointID, dParamFMax, rForce);
	}
	else
	{
		dJointSetHingeParam (m_JointID, dParamVel, 0);
		dJointSetHingeParam (m_JointID, dParamFMax, 0);
	}
}

void Dynamics::ODE::RevoluteJoint::vSetDamping ()
{
	if (!fInSimulation ())
		return;

	dJointSetHingeParam (m_JointID, dParamVel, 0);
	dJointSetHingeParam (m_JointID, dParamFMax, 0);
}

/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::ODE::PrismaticJoint::PrismaticJoint ()
{
}

Dynamics::ODE::PrismaticJoint::~PrismaticJoint ()
{
}

void Dynamics::ODE::PrismaticJoint::vDelete ()
{
	delete this;
}

bool Dynamics::ODE::PrismaticJoint::fInSimulation ()
{
	return fJointInSimulation ();
}

void Dynamics::ODE::PrismaticJoint::vConnectBodies (Dynamics::Body *pBody1, Dynamics::Body *pBody2)
{
	return vJointConnectBodies (dynamic_cast <ODE::Body*> (pBody1), dynamic_cast <ODE::Body*> (pBody2));
}

void Dynamics::ODE::PrismaticJoint::vCreate (Dynamics::World *pWorld, Juice::PrismaticJoint *pOwner)
{
	Dynamics::ODE::Joint::vCreate (dynamic_cast <ODE::World*> (pWorld));

	if (m_pWorld && !m_JointID)
	{
		m_JointID = dJointCreateSlider (m_pWorld->m_WorldID, 0);
		dJointSetData (m_JointID, pOwner);
	}
}

void Dynamics::ODE::PrismaticJoint::vSetLimits (bool fLo, real rLo, bool fHi, real rHi)
{
	if (!fInSimulation ())
		return;

	if (fLo)
	{
		dJointSetSliderParam (m_JointID, dParamLoStop, rLo);
	}
	else
	{
		dJointSetSliderParam (m_JointID, dParamLoStop, -dInfinity);
	}

	if (fHi)
	{
		dJointSetSliderParam (m_JointID, dParamHiStop, rHi);
	}
	else
	{
		dJointSetSliderParam (m_JointID, dParamHiStop, dInfinity);
	}
}


void Dynamics::ODE::PrismaticJoint::vSetAnchorAndAxis (VectorXYZ &vecAnchor, VectorYPR &vecRotation)
{
	if (!m_JointID)
		return;

	VectorXYZ vecOrientation (0, 0, -1);
	vecOrientation.vRotateByDegrees (-vecRotation);

	// set the axis
	dJointSetSliderAxis (m_JointID, vecOrientation.x, vecOrientation.y, vecOrientation.z);
}

void Dynamics::ODE::PrismaticJoint::vGetAxis (VectorXYZ &vecAxis)
{
	if (!fInSimulation ())
		return;

	dVector3 dvecAxis;
	dJointGetSliderAxis (m_JointID, dvecAxis);

	vecAxis.x = dvecAxis[0];
	vecAxis.y = dvecAxis[1];
	vecAxis.z = dvecAxis[2];
}


real Dynamics::ODE::PrismaticJoint::rGetExtension ()
{
	if (!fInSimulation ())
		return 0;

	return dJointGetSliderPosition (m_JointID);
}

void Dynamics::ODE::PrismaticJoint::vAddForce (real rVel, real rForce)
{
	if (!fInSimulation ())
		return;

	if (!_isnan (rVel) && !_isnan (rForce))
	{
		dJointSetSliderParam (m_JointID, dParamVel, rVel);
		dJointSetSliderParam (m_JointID, dParamFMax, rForce);
	}
	else
	{
		dJointSetSliderParam (m_JointID, dParamVel, 0);
		dJointSetSliderParam (m_JointID, dParamFMax, 0);
	}

}

/// Note that this is a rather naive implementation.  The right way to do it 
/// would be to store the original endpoints of the slider when simulation 
/// begins - see the vGetMatrix method for example.
void Dynamics::ODE::PrismaticJoint::vGetPosition (VectorXYZ &vecPosition)
{
	if (fInSimulation ())
	{
		const dReal *pPosition1 = null; 
		const dReal *pPosition2 = null; 

		dBodyID idBody1 = dJointGetBody (m_JointID, 0);
		dBodyID idBody2 = dJointGetBody (m_JointID, 1);

		if (idBody1)
			pPosition1 = dBodyGetPosition (idBody1);

		if (idBody2)
			pPosition2 = dBodyGetPosition (idBody2);

		if (pPosition1 && pPosition2)
		{
			vecPosition.x = (pPosition1[0] + pPosition2[0]) / 2;
			vecPosition.y = (pPosition1[1] + pPosition2[1]) / 2;
			vecPosition.z = (pPosition1[2] + pPosition2[2]) / 2;
		}
		else
		{
			vecPosition.x = 0;
			vecPosition.y = 0;
			vecPosition.z = 0;
		}
	}
	else
	{
		vecPosition.x = 0;
		vecPosition.y = 0;
		vecPosition.z = 0;
	}
}

void Dynamics::ODE::PrismaticJoint::vGetMatrix (Matrix4x4 &mx, const VectorXYZ &vecBody1OriginalEndPoint, const VectorXYZ &vecBody2OriginalEndPoint)
{
	VectorXYZ vecPosition;
	Quaternion qRotation;
	Matrix4x4 mxConversion, mxTransposed;

	VectorXYZ vecBody1EndPoint;
	VectorXYZ vecBody2EndPoint;

	// convert body 1 endpoint from local to world coordinates
	m_pBody1->vGetQuaternion (qRotation);
	m_pBody1->vGetPosition (vecPosition);

	mxConversion.vSetIdentity ();
	mxConversion.vSetRotation (qRotation);
	mxConversion.vTranspose (mxTransposed);

	vecBody1EndPoint = vecBody1OriginalEndPoint * mxTransposed;
	vecBody1EndPoint = vecBody1EndPoint + vecPosition;
	
	// convert body 2 endpoint from local to world coordinates
	m_pBody2->vGetQuaternion (qRotation);
	m_pBody2->vGetPosition (vecPosition);

	mxConversion.vSetIdentity ();
	mxConversion.vSetRotation (qRotation);
	mxConversion.vTranspose (mxTransposed);

	vecBody2EndPoint = vecBody2OriginalEndPoint * mxTransposed;
	vecBody2EndPoint = vecBody2EndPoint + vecPosition;

	// find length of slider 
	VectorXYZ vecSlider = vecBody1EndPoint - vecBody2EndPoint;
	real rLength = vecSlider.rMagnitude ();

	// find position of slider
	//VectorXYZ vecHalfSlider = vecSlider * 0.5;
	//VectorXYZ vecMidpoint = vecBody2EndPoint + vecHalfSlider;
	VectorXYZ vecMidpoint = vecBody1EndPoint + vecBody2EndPoint;
	vecMidpoint /= 2;

	dReal Position[3]; 
	Position[0] = vecMidpoint.x;
	Position[1] = vecMidpoint.y;
	Position[2] = vecMidpoint.z;

	VectorXYZ vecAxis;
	vGetAxis (vecAxis);
	VectorXYZ vecCross = CrossProduct (vecAxis, VectorXYZ (0, 0, 1));
	real rTheta = -rRadiansBetween (vecAxis, VectorXYZ (0, 0, 1));

	dReal Rotation[12];
	dRFromAxisAndAngle (Rotation, vecCross.x, vecCross.y, vecCross.z, rTheta);

	vBuildMatrix (mx, Position, Rotation);

	return;
/*
	// then get rotation to the slider's axis
	dMatrix3 mxRotation;
	VectorXYZ vecAxis (0, 0, 1);

	vGetAxis (vecAxis);

	dRFrom2Axes (mxRotation, vecAxis.x, vecAxis.y, vecAxis.z, 0, 0, 1);
	dQuaternion q;
	dQFromAxisAndAngle (q, 0, 1, 0, rPi / 2);
	dMatrix3 mxTmp;
	dQtoR (q, mxTmp);
	dMatrix3 mxFinalRotation;
	dMultiply0 (mxFinalRotation, mxRotation, mx, 3, 3, 3);
	
	dReal Position[3]; 
	Position[0] = vecMidpoint.x;
	Position[1] = vecMidpoint.y;
	Position[2] = vecMidpoint.z;

	vBuildMatrix (mx, Position, mxFinalRotation);
*/
}

/*
	//dMatrix3 mxRotation;
	//dRFrom2Axes (mxRotation, vecAxis.x, vecAxis.y, vecAxis.z, 0, 0, 1);
	Matrix4x4 mxRotation;
	mxRotation.vFromTwoAxes (vecAxis, VectorXYZ (0, 0, 1));

	//dQuaternion q;
	//dQFromAxisAndAngle (q, 0, 1, 0, rPi / 2);
	Quaternion q (VectorXYZ (0, 1, 0), rPi / 2);

	//dMatrix3 mx;
	//dQtoR (q, mx);
	Matrix4x4 mxTmp (q);

	//dMatrix3 mxFinalRotation;
	//dMultiply0 (mxFinalRotation, mxRotation, mx, 3, 3, 3);
	Matrix4x4 mxFinalRotation;
	mxFinalRotation = mxRotation * mxTmp;
	
	//dReal Position[3]; 
	//Position[0] = vecMidpoint.x;
	//Position[1] = vecMidpoint.y;
	//Position[2] = vecMidpoint.z;

	// Set transform
	mxRotation.vSetTranslation (vecMidpoint);

	mx = mxRotation;
*/

/****************************************************************************/
/** 
*****************************************************************************/
Dynamics::ODE::SphericalJoint::SphericalJoint ()
{
}

Dynamics::ODE::SphericalJoint::~SphericalJoint ()
{
}

void Dynamics::ODE::SphericalJoint::vDelete ()
{
	delete this;
}

bool Dynamics::ODE::SphericalJoint::fInSimulation ()
{
	return fJointInSimulation ();
}

void Dynamics::ODE::SphericalJoint::vConnectBodies (Dynamics::Body *pBody1, Dynamics::Body *pBody2)
{
	return vJointConnectBodies (dynamic_cast <ODE::Body*> (pBody1), dynamic_cast <ODE::Body*> (pBody2));
}

void Dynamics::ODE::SphericalJoint::vCreate (Dynamics::World *pWorld, Juice::SphericalJoint *pOwner)
{
	Dynamics::ODE::Joint::vCreate (dynamic_cast <ODE::World*> (pWorld));

	if (m_pWorld && !m_JointID)
	{
		m_JointID = dJointCreateBall (m_pWorld->m_WorldID, 0);
		dJointSetData (m_JointID, pOwner);
	}
}

void Dynamics::ODE::SphericalJoint::vSetAnchor (VectorXYZ &vecAnchor)
{
	if (!m_JointID)
		return;

	// set the axis
	dJointSetBallAnchor (m_JointID, vecAnchor.x, vecAnchor.y, vecAnchor.z);
}

void Dynamics::ODE::SphericalJoint::vGetAnchor (VectorXYZ &vecAnchor)
{
	if (!fInSimulation ())
		return;

	dVector3 dvecAnchor;
	dJointGetBallAnchor (m_JointID, dvecAnchor);

	vecAnchor.x = dvecAnchor[0];
	vecAnchor.y = dvecAnchor[1];
	vecAnchor.z = dvecAnchor[2];
}

void Dynamics::ODE::SphericalJoint::vSetLimits (bool fLo, real rLo, bool fHi, real rHi)
{
	// not applicable
}

/*****************************************************************************
<FOOTER>
*****************************************************************************/
