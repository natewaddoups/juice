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

//#include "NovodexOutput.h"

#include <float.h>
#include <Math.h>

using namespace Juice;
using namespace Juice::Dynamics::Novodex;

/*****************************************************************************
/** This is the DLL's sole export
*****************************************************************************/
extern "C" __declspec (dllexport) Dynamics::World* CreateWorld (Generic::INT32 iWorldID)
{
	World *pWorld = new World;

	if (!pWorld->fInitialized ())
		return null;

	return pWorld;
}

/****************************************************************************/
/** The error output stream for Novodex
*****************************************************************************/
//NovodexOutput ErrorStream;




/*****************************************************************************
/** This is a helper function that builds an OpenGL-compatible matrix using 
 ** Novodex's position vector and orientation matrix.
*****************************************************************************/
void vBuildMatrix (Matrix4x4 &mx, const NxrVec3 &Position, const NxrMat33 &Orientation)
{
	mx[0]  = Orientation(0,0);
	mx[1]  = Orientation(1,0);
	mx[2]  = Orientation(2,0);
	mx[3]  = 0;
	mx[4]  = Orientation(0,1);
	mx[5]  = Orientation(1,1);
	mx[6]  = Orientation(2,1);
	mx[7]  = 0;
	mx[8]  = Orientation(0,2);
	mx[9]  = Orientation(1,2);
	mx[10] = Orientation(2,2);
	mx[11] = 0;
	mx[12] = Position.x();
	mx[13] = Position.y();
	mx[14] = Position.z();
	mx[15] = 1;

/*	mx[0]  = Orientation(0,0);
	mx[1]  = Orientation(0,1);
	mx[2]  = Orientation(0,2);
	mx[3]  = 0;
	mx[4]  = Orientation(1,0);
	mx[5]  = Orientation(1,1);
	mx[6]  = Orientation(1,2);
	mx[7]  = 0;
	mx[8]  = Orientation(2,0);
	mx[9]  = Orientation(2,1);
	mx[10] = Orientation(2,2);
	mx[11] = 0;
	mx[12] = Position.x();
	mx[13] = Position.y();
	mx[14] = Position.z();
	mx[15] = 1;*/
}

// World.cpp ends here ->