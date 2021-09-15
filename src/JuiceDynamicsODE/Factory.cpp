/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsODE.dll
Filename: Factory.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceDynamicsODE.dll and its source code are distributed under the terms of 
the Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/
#include <JuiceEngine.h>
#include <DynamicsAPI.h>

#include <ode/ode.h>
#include "DynamicsODE.h"

using namespace Juice;
using namespace Juice::Dynamics::ODE;

/*****************************************************************************
/** This is the DLL's sole export
*****************************************************************************/
extern "C" __declspec (dllexport) Dynamics::World* CreateWorld (Generic::INT32 iFactoryID)
{
	switch (iFactoryID)
	{
	case 0: return new StandardWorld;
	case 1: return new IterativeWorld;	
	}

	return null;
}

/*****************************************************************************
/** 
*****************************************************************************/
World::Constants::Constants () :
	m_rGravity (9.8),
	m_rERP (1.0),
	m_rCFM (0.001),
	m_rFriction (100),
	m_rBounce (0.1f)
{
}

void World::Constants::vSerialize (XML::Stream &Stream)
{
	Stream.vSerialize (&m_rGravity, L"Gravity");
	Stream.vSerialize (&m_rERP, L"ERP");
	Stream.vSerialize (&m_rCFM, L"CFM");
	Stream.vSerialize (&m_rFriction, L"Friction");
	Stream.vSerialize (&m_rBounce, L"Bounce");
}

IterativeWorld::Constants::Constants () :
	m_uIterations (5)
{
}

void IterativeWorld::Constants::vSerialize (XML::Stream &Stream)
{
	World::Constants::vSerialize (Stream);
	Stream.vSerialize (&m_uIterations, L"Iterations");
}



/*****************************************************************************
/** This is a helper function that builds an OpenGL-compatible matrix using 
*** ODE's position vector and orientation matrix.
*** Probably shouldn't be in this file, really...
*****************************************************************************/
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

// Factory.cpp ends here ->