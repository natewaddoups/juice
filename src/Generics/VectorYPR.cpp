/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: VectorYPR.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

I should note that most of the stuff in this file is pretty basic and there's
no way I could be bothered to care if you re-used it in any way.
*****************************************************************************/

#include <GenericTypes.h>
#include <math.h>

using namespace Generic;

/****************************************************************************/
/* YPR radian <-> degree conversions
*****************************************************************************/

void Generic::VectorYPR::vToRadians ()
{
	yaw *= rDegreesToRadians;
	pitch *= rDegreesToRadians;
	roll *= rDegreesToRadians;
}

void Generic::VectorYPR::vToDegrees ()
{
	yaw *= rRadiansToDegrees;
	pitch *= rRadiansToDegrees;
	roll *= rRadiansToDegrees;
}

void Generic::VectorYPR::vGetMatrix (Matrix4x4 &mxRotation)
{
	real rYaw   = rDegreesToRadians * yaw;
	real rPitch = rDegreesToRadians * pitch;
	real rRoll  = rDegreesToRadians * roll;

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

	mxRotation.vSetIdentity ();
	mxRotation.vSetRotation (qTotal);
}

using namespace Generic::XML;

void VectorYPR::vSerialize (Stream &Stream)
{
	Stream.vSerialize (&yaw,   L"Y");
	Stream.vSerialize (&pitch, L"P");
	Stream.vSerialize (&roll,  L"R");
}


// VectorYPR.cpp ends here ->