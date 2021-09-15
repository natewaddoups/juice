/*****************************************************************************
<HEADER>
*****************************************************************************/

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
using namespace Juice::Dynamics::Whatever;

/****************************************************************************/
/** 
*****************************************************************************/
World::World () :
	m_Terrain (0),
	m_CollisionJointGroup (0),
	m_uLastTimerTick (0)
{
	// Create the dynamics world

	// Create the collision space

	// Create the collision contact joint group
}

/****************************************************************************/
/** 
*****************************************************************************/
World::~World ()
{
	vDestroyCage ();

	vDestroyTerrain ();
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

	// create ground 	
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vDestroyCage ()
{
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

	// create
}

void World::vDestroyTerrain ()
{
	if (m_Terrain)
	{
		// destroy

		m_Terrain = null;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vEnableCage ()
{
	// add cage, remove terrain
}

void World::vEnableTerrain ()
{
	// remove cage, add terrain
	if (m_Terrain && !dSpaceQuery (m_SpaceID, m_Terrain))
		dSpaceAdd (m_SpaceID, m_Terrain);
}

/****************************************************************************/
/** 
*****************************************************************************/
bool World::fStep (Generic::UINT32 uMilliseconds)
{
	bool fReturn = true;

	// check for collisions
	//

	// convert elapsed time to seconds
	real rElapsedTime = ((real) uMilliseconds) / ((real) 1000.0);

	try 
	{
		// step the world
	}
	catch (...)
	{
		// This typically results from a LARGE number of contact joints 
		OutputDebugString ("Simulation failure.\n");
		fReturn = false;
	}

    // remove all contact constraints
    
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
}

void World::vChangeParameters ()
{
}

/*****************************************************************************
<FOOTER>
*****************************************************************************/
