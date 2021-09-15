/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenericTypes.h 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#ifndef __GenericTypes__
#define __GenericTypes__

#ifndef GenericsExport
#ifdef GENERICS_EXPORTS
#define GenericsExport __declspec (dllexport)
#else
#define GenericsExport __declspec (dllimport)
#endif
#endif

#ifdef _WINDOWS
#pragma warning (disable : 4251) // C4251 non-exported base class 
#pragma warning (disable : 4275) // C4275 non-exported base class (ofstream)
#define VC_EXTRALEAN
#include <windows.h>
#endif

#include <string>
#include <fstream>
#include <stack>

/****************************************************************************/
/** Things that might be useful in many applications 
*****************************************************************************/

namespace Generic {
//#pragma warning (disable : 4244) // C4244 float/double conversion

/****************************************************************************/
/** 
*****************************************************************************/
class Quaternion;
class VectorYPR;
class VectorXYZ;
class Matrix4x4;

/****************************************************************************/
/** I learned to enjoy this keyword in perl, so...
*****************************************************************************/
#define unless(x) if (!(x))

/****************************************************************************/
/** 
*****************************************************************************/

	typedef double real;
	//typedef float real;

	typedef int INT32;
	typedef unsigned UINT32;

	typedef char INT8;
	typedef unsigned char UINT8;

	#ifdef _WINDOWS
		typedef HWND WindowType;
		typedef HDC DrawContext;
	#else
		// this is just for testing w/ console apps
		typedef int WindowType;
		typedef int DrawContext;
	#endif

/****************************************************************************/
/** 
*****************************************************************************/

	const int null = 0;

	const real rPi = (real) 3.14159265358979323846;
	const real rDegreesToRadians = rPi / 180;
	const real rRadiansToDegrees = 180 / rPi;

	real GenericsExport rRound (real rInput, real rResolution);

/****************************************************************************/
/** Thanks to DuneBoy on #c++ for suggesting this
*****************************************************************************/

template<typename Type1, typename Type2>
bool IsKindOf (Type2 t2)
{
	try
	{
		const type_info &ti = typeid (t2);
		Type1 foo = dynamic_cast <Type1> (t2);

		if (!foo)
			return false;

		return true;
	}
	catch (...)
	{
		return false;
	}
}

template<typename Type1, typename Type2>
bool IsKindOfPtr (Type2 t2)
{
	try
	{
		// this is just for debugging
		const type_info &tiSource = typeid (*t2);

		// this will throw if the types are unrelated
		Type1 Unused = dynamic_cast <Type1> (t2);

		// casts across the inheritance tree might return null (this don't seem right to me, but it happens)
		if (!Unused)
			return false;

		return true;
	}
	catch (...)
	{
		return false;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/

	typedef struct
	{
		real r;
		real g;
		real b;
		real a;
	} Color;

	Generic::UINT32 GenericsExport WinColorRef (Color clr);

/****************************************************************************/
/** 
*****************************************************************************/

	#ifdef _DEBUG
	#define Breakpoint(szReason) Generic::DoBreakpoint (szReason); // DebugBreak ();
	#else
	#define Breakpoint(szReason) 
	#endif


	void GenericsExport DoBreakpoint (const char *szReason = null);

	void GenericsExport vShowLastError (const char *szCaption);

}; // end Generic namespace

#include <GenericXmlStreaming.h>
#include <GenericMath.h>


#endif

// GenericTypes.h ends here ->