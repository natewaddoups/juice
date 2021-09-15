/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GraphObjects.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#include "stdafx.h"
#include "Generics.h"
#include <GenericTypes.h>
#include <GenericError.h>
#include <GenericGraphControl.h>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

using namespace Generic;

Graph::Node::Node ()
{
}

Graph::Node::~Node ()
{
}

/*****************************************************************************
*****************************************************************************/

Graph::Edge::Edge () : m_pSource (null), m_pDestination (null)
{
}

Graph::Edge::~Edge ()
{
}

void Graph::Edge::vSetSource (Node *pSource)
{
	m_pSource = pSource;
}

void Graph::Edge::vSetDestination (Node *pDestination)
{
	m_pDestination = pDestination;

}

Graph::Node* Graph::Edge::pGetSource ()
{
	return m_pSource;
}

Graph::Node* Graph::Edge::pGetDestination ()
{
	return m_pDestination;
}

// GraphObjects.cpp ends here ->