/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Win32.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include <mmsystem.h>
#include <crtdbg.h>
#include <JuiceEngine.h>

using namespace Juice;

extern "C" void dCloseODE ();

/****************************************************************************/
/** 
*****************************************************************************/

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ulEvent, 
                       LPVOID lpReserved
					 )
{
	if (DLL_PROCESS_ATTACH == ulEvent)
	{
		//_CrtSetBreakAlloc (73);
	}

	if (DLL_PROCESS_DETACH == ulEvent)
	{
		OutputDebugString ("JuiceEngine memory leak test...");
		if (_CrtDumpMemoryLeaks ())
		{
			OutputDebugString ("Bummer!\n");
		}
		else
		{
			OutputDebugString ("none found.\n");
		}
	}

    return TRUE;
}


/****************************************************************************/
/** Read the joystick X, Y, Z, and R positions, and scale them to +/- 1.0.
*** Results are stored in the application object's m_ControlInput member.
*****************************************************************************/

#define GetAxis(s,a) \
	m_rJoystick##s##a = (real) state.dw##a##pos; \
	m_rJoystick##s##a /= (0xFFFF / 2); \
	m_rJoystick##s##a -= 1;


#define GetButton(s,b) \
	m_fJoystick##s##Button##b = ((state.dwButtons & (1 << (b - 1))) > 0);

void ControlInput::vUpdate ()
{
	JOYINFOEX state;
	MMRESULT mmr = 0;

	state.dwSize = sizeof (JOYINFOEX);
	state.dwFlags = JOY_RETURNALL;

	mmr = joyGetPosEx (JOYSTICKID1, &state);

	if (mmr == JOYERR_NOERROR)
	{
		GetAxis(1,X);
		GetAxis(1,Y);
		GetAxis(1,Z);
		GetAxis(1,R);

		GetButton(1,1);
		GetButton(1,2);
		GetButton(1,3);
		GetButton(1,4);

		m_iJoystick1Hat = state.dwPOV;
	}
	else
	{
		m_rJoystick1X = 0;
		m_rJoystick1Y = 0;
		m_rJoystick1Z = 0;
		m_rJoystick1R = 0;

		m_fJoystick1Button1 = 0;
		m_fJoystick1Button2 = 0;
		m_fJoystick1Button3 = 0;
		m_fJoystick1Button4 = 0;

		m_iJoystick1Hat = -1;
	}

	mmr = joyGetPosEx (JOYSTICKID2, &state);

	if (mmr == JOYERR_NOERROR)
	{
		GetAxis(2,X);
		GetAxis(2,Y);
		GetAxis(2,Z);
		GetAxis(2,R);

		GetButton(2,1);
		GetButton(2,2);
		GetButton(2,3);
		GetButton(2,4);

		m_iJoystick2Hat = state.dwPOV;
	}
	else
	{
		m_rJoystick2X = 0;
		m_rJoystick2Y = 0;
		m_rJoystick2Z = 0;
		m_rJoystick2R = 0;

		m_fJoystick2Button1 = 0;
		m_fJoystick2Button2 = 0;
		m_fJoystick2Button3 = 0;
		m_fJoystick2Button4 = 0;

		m_iJoystick2Hat = -1;
	}
}

// Win32.cpp ends here ->