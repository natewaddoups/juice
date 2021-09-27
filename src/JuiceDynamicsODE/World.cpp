/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsODE.dll
Filename: World.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceDynamicsODE.dll and its source code are distributed under the terms of 
the Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>
#include <DynamicsAPI.h>

//#include "ode\ode.h"
#include "DynamicsODE.h"

#include <float.h>
#include <Math.h>

#include <Terrain.h>
#include "Terrain.h"
//#include <ODE/dTriList.h>

using namespace Juice;
using namespace Juice::Dynamics::ODE;

/****************************************************************************/
/** 
*****************************************************************************/
World::World () :
	m_WorldID (0),
	m_SpaceID (0),
	m_rGravity (9.8), m_rERP (1.0), m_rCFM (0.001),
	m_CageGround (0), m_CageSky (0), m_CageLeft (0), m_CageRight (0), m_CageFront (0), m_CageBack (0),
	m_Terrain (0),
	m_CollisionJointGroup (0),
	m_pVertices (0),
	m_rFriction (0), m_rBounce (0)
{
	// Create the dynamics world
	m_WorldID = dWorldCreate ();
	dWorldSetGravity (m_WorldID, 0, 0, (dReal) -9.81);

	// Create the collision space
	m_SpaceID = dHashSpaceCreate (0);

	// Create the collision contact joint group
	m_CollisionJointGroup = dJointGroupCreate (1000000);
}

StandardWorld::StandardWorld ()
{
}

IterativeWorld::IterativeWorld ()
{
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

	dCloseODE ();
}

Dynamics::Body*           World::pCreateBody ()           { return new ODE::Body (this); }
Dynamics::RevoluteJoint*  World::pCreateRevoluteJoint ()  { return new ODE::RevoluteJoint (this); }
Dynamics::PrismaticJoint* World::pCreatePrismaticJoint () { return new ODE::PrismaticJoint (this); }
Dynamics::SphericalJoint* World::pCreateSphericalJoint () { return new ODE::SphericalJoint (this); }


/****************************************************************************/
/** 
*****************************************************************************/
void World::vDelete ()
{
	delete this;
}


/****************************************************************************/
/** 
*****************************************************************************/
void World::vPrepareSimulation ()
{
	dWorldSetGravity (m_WorldID, 0, 0, -m_Constants.m_rGravity);
	dWorldSetERP (m_WorldID, m_Constants.m_rERP);
	dWorldSetCFM (m_WorldID, m_Constants.m_rCFM);
}

void IterativeWorld::vPrepareSimulation ()
{
	World::vPrepareSimulation ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vCreateCage ()
{
	vDestroyCage ();

	// create ground and sky
	m_CageGround = dCreatePlane (m_SpaceID, 0, 0, 1, 0);
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

	// get vertex data from the terrain engine
	pTerrain->vGetTriangles (&pVertices, &iVertexCount, &pTriangles, &iTriangleCount);

	if (!(pVertices && pTriangles))
		return;

	// wipe out any old data
	vDestroyTerrain ();

	/*  TODO: Terrain 
	// convert vertex data to ODE's preferred format
	m_pVertices = new dVector3[iVertexCount];

	for (int i = 0; i < iVertexCount; i++)
	{
		m_pVertices[i].x = (float) pVertices[i].x;
		m_pVertices[i].y = (float) pVertices[i].y;
		m_pVertices[i].z = (float) pVertices[i].z;
	}

	m_Terrain = dCreateTriList (m_SpaceID, null, null);
	dGeomTriListBuild(m_Terrain, m_pVertices, iVertexCount, &(pTriangles->v1), iTriangleCount * 3);
	*/
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vDestroyTerrain ()
{
	delete m_pVertices;
	m_pVertices = 0;

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
	if (m_CageGround && !dSpaceQuery (m_SpaceID, m_CageGround))
		dSpaceAdd (m_SpaceID, m_CageGround);

	if (m_Terrain && dSpaceQuery (m_SpaceID, m_Terrain))
		dSpaceRemove (m_SpaceID, m_Terrain);
}

void World::vEnableTerrain ()
{
	if (m_Terrain && !dSpaceQuery (m_SpaceID, m_Terrain))
		dSpaceAdd (m_SpaceID, m_Terrain);

	if (m_CageGround && dSpaceQuery (m_SpaceID, m_CageGround))
		dSpaceRemove (m_SpaceID, m_CageGround);
}

/****************************************************************************/
/** 
*****************************************************************************/

bool StandardWorld::fStep (Generic::UINT32 uMilliseconds)
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

bool IterativeWorld::fStep (Generic::UINT32 uMilliseconds)
{
	bool fReturn = true;

	// check for collisions
	dSpaceCollide (m_SpaceID, this, CollisionCallback);

	//uMilliseconds = 1;

	// convert to seconds
	real rElapsedTime = ((real) uMilliseconds) / ((real) 1000.0);

	try 
	{
		dWorldStep(m_WorldID, rElapsedTime); // , m_Constants.m_uIterations);
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
	{
		pThis->CollisionCallback (o1, o2);
	}
	else
	{
		GenericUnexpected ("collision detection disabled");
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
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
			contact[iContact].surface.mu = m_Constants.m_rFriction;
			contact[iContact].surface.bounce = m_Constants.m_rBounce;
			contact[iContact].surface.bounce_vel = (dReal) 0.01;
			contact[iContact].surface.slip1 = (dReal) 0.1;
			contact[iContact].surface.slip2 = (dReal) 0.1;
			//contact.surface.soft_erp = 0.5; // 0.5;
			//contact.surface.soft_cfm = 0.3; // 0.3;

			dJointID ContactJoint = dJointCreateContact (m_WorldID, m_CollisionJointGroup, &contact[iContact]);

			dJointAttach (ContactJoint, Body1, Body2);
		}
	}
}


// World.cpp ends here ->