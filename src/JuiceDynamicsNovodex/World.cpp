/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsNovodex.dll
Filename: World.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceDynamicsNovodex.dll and its source code are distributed under the terms
of the Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <NxFoundation.h>
#include <NxRigidBody.h>
#include <NxCollision.h>

#include <JuiceEngine.h>
#include <DynamicsAPI.h>
#include "DynamicsNovodex.h"

#include <float.h>
#include <Math.h>

#include <Terrain.h>
#include "Terrain.h"

using namespace Juice;
using namespace Juice::Dynamics::Novodex;

/****************************************************************************/
/** 
*****************************************************************************/
World::World () :
	m_pScene (0),
	m_pSpace (0),
	m_pGround (0),
	//m_pTerrain (0),
	m_pGravity (0),
	m_pVertices (0),
	m_pStaticShapes (0),
	m_pDynamicShapes (0)
{
	m_pDynamicsFactory =  NxCreateRigidBodySDK (NX_RIGIDBODY_SDK_VERSION);
	m_pCollisionFactory = NxCreateCollisionSDK (NX_COLLISION_SDK_VERSION);

	if (m_pDynamicsFactory && m_pCollisionFactory)
	{
		m_pScene = m_pDynamicsFactory->createScene ();
		//m_pSpace = m_pCollisionFactory->createHashGridSpace (256, 1, 29);
		m_pStaticShapes = m_pCollisionFactory->createShapeSet ();
		m_pDynamicShapes = m_pCollisionFactory->createShapeSet ();
		m_pGravity = m_pDynamicsFactory->createGravityEffector ();

		m_pScene->setUserCollisionDetection (this);
		m_pScene->addEffector (*m_pGravity);	
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
World::~World ()
{
	vDestroyCage ();

	vDestroyTerrain ();

	if (m_pStaticShapes)
	{
		m_pStaticShapes->removeAllShapes ();
		m_pStaticShapes->release ();
		m_pStaticShapes = 0;
	}

	if (m_pDynamicShapes)
	{
		m_pDynamicShapes->removeAllShapes ();
		m_pDynamicShapes->release ();
		m_pDynamicShapes = 0;
	}

	if (m_pScene)
	{
		m_pScene->resetEffectorIterator ();
		while (m_pScene->hasMoreEffectors ())
		{
			NxEffector *p = m_pScene->getNextEffector ();
			m_pScene->removeEffector (*p);
		}

		m_pScene->resetJointIterator ();
		while (m_pScene->hasMoreJoints ())
		{
			NxJoint *p = m_pScene->getNextJoint ();
			m_pScene->removeJoint (*p);
		}

		m_pScene->resetBodyIterator ();
		while (m_pScene->hasMoreBodies ())
		{
			NxBody *p = m_pScene->getNextBody ();
			m_pScene->removeBody (*p);
		}

		m_pScene->release ();
		m_pScene = 0;
	}

	if (m_pSpace)
	{
		m_pSpace->release ();
		m_pSpace = 0;
	}

	if (m_pDynamicsFactory)
		m_pDynamicsFactory->release ();

	m_pDynamicsFactory = null;

	if (m_pCollisionFactory)
		m_pCollisionFactory->release ();

	m_pCollisionFactory = null;
}


/*****************************************************************************
/** 
*****************************************************************************/
bool World::fInitialized ()
{
	return (m_pCollisionFactory && m_pDynamicsFactory);
}


/*****************************************************************************
/** 
*****************************************************************************/
Dynamics::Body* World::pCreateBody ()
{
	return new Novodex::Body (this);
}


/*****************************************************************************
/** 
*****************************************************************************/
Dynamics::RevoluteJoint* World::pCreateRevoluteJoint ()
{
	return new Novodex::RevoluteJoint (this); 
}


/*****************************************************************************
/** 
*****************************************************************************/
Dynamics::PrismaticJoint* World::pCreatePrismaticJoint ()
{
	return new Novodex::PrismaticJoint (this); 
}


/*****************************************************************************
/** 
*****************************************************************************/
Dynamics::SphericalJoint* World::pCreateSphericalJoint () 
{
	return new Novodex::SphericalJoint (this); 
}


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
void World::vCreateCage ()
{
	vDestroyCage ();

	if (m_pCollisionFactory)
	{
		m_pGround = m_pCollisionFactory->createPlane ();

		NxrVec3 vecNormal (0, 0, 1);
		m_pGround->setPlane (vecNormal, 0);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vDestroyCage ()
{
	if (m_pStaticShapes && m_pGround)
	{
		NxShape &shape = m_pGround->getShape ();

		m_pStaticShapes->resetIterator ();
		while (m_pStaticShapes->hasMoreElements ())
		{
			NxShape *p = m_pStaticShapes->getNextElement ();
			if (p == &shape)
			{
				m_pStaticShapes->removeShape (shape);
				break;
			}
		}		
		
		shape.release ();
		m_pGround = null;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vCreateTerrain (Terrain *pTerrain)
{
/*	if (!pTerrain)
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
*/
}

void World::vDestroyTerrain ()
{
/*
	if (m_Terrain)
	{
		// destroy

		m_Terrain = null;
	}
*/
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vEnableCage ()
{
	if (!m_pGround)
		vCreateCage ();

	// Enable cage
	if (m_pGround && m_pStaticShapes)
		m_pStaticShapes->addShape ((NxShape&) *m_pGround);

	// Disable terrain
//	if (m_pTerrain && m_pStaticShapes)
//		m_pStaticShapes->removeShape (m_pTerrain->getShape ());
}

void World::vEnableTerrain ()
{
//	if (!m_pTerrain)
//		vCreateTerrain ();

	// remove cage, add terrain
	if (m_pGround && m_pStaticShapes)
		m_pStaticShapes->removeShape (m_pGround->getShape ());

//	if (m_pTerrain && m_pStaticShapes)
//		m_pStaticShapes->addShape ((NxShape&) *m_pTerrain);
}

/****************************************************************************/
/** For each step:
 **		1) Move all shapes in the scene
 **		2) Find contacts 
 **		3) create contact joints in the callback
 **		4) Move all bodies in the scene
 **
 ** (1) and (2) are done in the 'invoke' method, which is called from 
 ** NxScene::run  (3) is done in the 'contact' method, which is called from
 ** findContacts (which is called from invoke (which is called from run (which
 ** is called below))).  (4) is done in run, below.
 **
 ** Note that the caller is responsible for ensuring that uMilliseconds > 0.
*****************************************************************************/
bool World::fStep (Generic::UINT32 uMilliseconds)
{
	bool fReturn = false;

	try 
	{
		m_pScene->run ((real) uMilliseconds / (real) 1000.0);

		// Yay
		fReturn = true;
	}
	catch (...)
	{
		// This typically results from a LARGE number of contact joints 
		OutputDebugString ("Internal simulation failure.\n");
	}
   
	return fReturn;
}


/****************************************************************************/
/** 
*****************************************************************************/
void World::invoke (NxScene &scene)
{
	// Move all shapes in the scene (this must be done first to set the initial positions of collision shapes)
	//m_pDynamicShapes->poseChangeNotify ();
	NxShape *pShape = 0;
	m_pDynamicShapes->resetIterator ();
	while (m_pDynamicShapes->hasMoreElements ())
	{
		pShape = m_pDynamicShapes->getNextElement ();
		if (pShape)
		{
			Body *pBody = (Body*) pShape->getUserData ();
			if (pBody)
			{
				pBody->vPoseChange ();
			}
		}
	}

	// Check for collisions and create contact joints
	// TODO: use a hash space
	//GenericTrace ("Checking collisions:" << std::endl);
	m_pDynamicShapes->findContacts (*m_pDynamicShapes, (NxUserContactReport&) *this);
	m_pStaticShapes->findContacts  (*m_pDynamicShapes, (NxUserContactReport&) *this);
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::contact (NxShape &Shape1, NxShape &Shape2, NxcReal rSeparation, 
		const NxcVec3 &vecContactPoint, const NxcVec3 &vecContactNormal)

{
	// Unwrap the various objects
	Novodex::Body *pBodyWrapper1 = (Novodex::Body*) Shape1.getUserData ();
	Novodex::Body *pBodyWrapper2 = (Novodex::Body*) Shape2.getUserData ();
	NxBody *pBody1 = null;
	NxBody *pBody2 = null;

	if (pBodyWrapper1)
		pBody1 = pBodyWrapper1->m_pBody;

	if (pBodyWrapper2)
		pBody2 = pBodyWrapper2->m_pBody;

	// ignore collisions between bodies that are connected by the same joint
	// FIXFIX this could be done via CollisionVeto
	if (pBody1 && pBody2 && pBody1->isAttatchedByJoint (*pBody2))
		return;

#ifdef _DEBUG
	// These are just to make debugging easier, they're not used anywhere else (yet?)
	Juice::Body *pJuiceBody1 = null;
	Juice::Body *pJuiceBody2 = null;
	
	if (pBody1)
		pJuiceBody1 = (Juice::Body*) pBodyWrapper1->m_pOwner;

	if (pBody2)
		pJuiceBody2 = (Juice::Body*) pBodyWrapper2->m_pOwner;

	//GenericTrace ("\tCollision:" << std::endl);

	if (pJuiceBody1)
	{
		//GenericTrace ("\t\tBody1: " << pJuiceBody1->szGetName () << std::endl);
	}

	if (pJuiceBody2)
	{
		//GenericTrace ("\t\tBody1: " << pJuiceBody2->szGetName () << std::endl);
	}
#endif

	NxContact *pContact = m_pScene->createContact (pBody1, pBody2, vecContactPoint, vecContactNormal);
	if (!pContact)
		return;

	pContact->setGeometry (rSeparation);
	//pContact->setMaterial (m_pApplication->m_Preferences.m_rGroundBounce, m_pApplication->m_Preferences.m_rGroundFriction / 300, m_pApplication->m_Preferences.m_rGroundFriction / 300);
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vPrepareSimulation ()
{
	if (m_pGravity)
		m_pGravity->setGravity (NxrVec3 (0, 0, -m_Constants.m_rGravity));
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vAddBody (Dynamics::Novodex::Body *pBody)
{
	if (m_pScene && pBody && pBody->m_pBody)
	{
		m_pScene->addBody (*pBody->m_pBody);
	}
}

void World::vRemoveBody (Dynamics::Novodex::Body *pBody)
{
	if (m_pScene && pBody && pBody->m_pBody)
	{
		m_pScene->removeBody (*pBody->m_pBody);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vAddJoint (Dynamics::Novodex::Joint *pJoint)
{
	if (m_pScene && pJoint && pJoint->m_pJoint)
	{
		m_pScene->addJoint (*pJoint->m_pJoint);
	}
}

void World::vRemoveJoint (Dynamics::Novodex::Joint *pJoint)
{
	if (m_pScene && pJoint && pJoint->m_pJoint)
	{
		m_pScene->removeJoint (*pJoint->m_pJoint);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/

void World::vAddShape (NxShape *pShape)
{
	m_pDynamicShapes->addShape (*pShape);
}

void World::vRemoveShape (NxShape *pShape)
{
	m_pDynamicShapes->removeShape (*pShape);
}


/****************************************************************************/
/** 
*****************************************************************************/
World::Constants::Constants () :
	m_rGravity ((real) 9.8),
	m_rBounce ((real) 0.01),
	m_rFriction1 ((real) 10),
	m_rFriction2 ((real) 100)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::Constants::vSerialize (XML::Stream &Stream)
{
	Stream.vSerialize (&m_rGravity, L"Gravity");
	Stream.vSerialize (&m_rBounce, L"Bounce");
	Stream.vSerialize (&m_rFriction1, L"Friction1");
	Stream.vSerialize (&m_rFriction2, L"Friction2");
}

/****************************************************************************/
/** 
*****************************************************************************/
void World::vConstantsDialog ()
{
}


// World.cpp ends here ->