/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsNovodex.dll
Filename: NovodexOutput.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceDynamicsNovodex.dll and its source code are distributed under the terms
of the Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/
#pragma once

extern __declspec (dllimport) Generic::Trace Generic::g_Trace;

class NovodexOutput
{
public:
	/**
	Inserts a tab character into the stream. 
	*/
	virtual void tab()
	{
		g_Trace << '\t';
	}

	/**
	Inserts a newline character into the stream. 
	*/
	virtual void nl()
	{
		g_Trace << "\r\n";
	}

	/**
	Flushes the output.
	*/
	virtual void flush()
	{
	}

	/**
	Inserts a string into the stream.
	*/
    virtual NxUserOutputStream & operator<<(const char *s)
	{
		g_Trace << s;
		return (NxUserOutputStream&) *this;
	}

	/**
	Inserts a boolean into the stream.
	*/
    virtual NxUserOutputStream & operator<<(bool n)
	{
		if (n)
			g_Trace << "true";
		else
			g_Trace << "false";

		return (NxUserOutputStream&) *this;
	}

	/**
	Inserts a character into the stream.
	*/
    virtual NxUserOutputStream & operator<<(char c)
	{
		g_Trace << c;
		return (NxUserOutputStream&) *this;
	}

	/**
	Inserts a memory address (should be displayed as hex) into the stream.
	*/
    virtual NxUserOutputStream & operator<<(const void * n)
	{
		g_Trace << n;
		return (NxUserOutputStream&) *this;
	}

	/**
	Inserts a 32 bit signed integer into the stream.
	*/
    virtual NxUserOutputStream & operator<<(NxI32 n)
	{
		g_Trace << n;
		return (NxUserOutputStream&) *this;
	}

	/**
	Inserts a 32 bit unsigned integer into the stream.
	*/
    virtual NxUserOutputStream & operator<<(NxU32 n)
	{
		g_Trace << n;
		return (NxUserOutputStream&) *this;
	}

	/**
	Inserts a 32 bit float into the stream.
	*/
    virtual NxUserOutputStream & operator<<(NxF32 n)
	{
		g_Trace << n;
		return (NxUserOutputStream&) *this;
	}

	/**
	Inserts a 64 bit float into the stream.
	*/
    virtual NxUserOutputStream & operator<<(NxF64 n)
	{
		g_Trace << n;
		return (NxUserOutputStream&) *this;
	}

};

// NovodexOutput.h ends here ->