/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: XML.cpp 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#include <GenericTypes.h>
#include <malloc.h> // for alloca
#include "MSXML2.h"
#include "XmlInternal.h"

using namespace Generic;
using namespace Generic::XML;

const char Generic::XML::g_szFileVersionAttribute[] = "FileVersion";
const wchar_t Generic::XML::g_szWideFileVersionAttribute[] = L"FileVersion";
const char Generic::XML::g_szSerializationVersionAttribute[] = "SerializationVersion";
const wchar_t Generic::XML::g_szWideSerializationVersionAttribute[] = L"SerializationVersion";
const int Generic::XML::g_iSerializationVersion = 2;



/****************************************************************************/
/** Create a node with the given name
*****************************************************************************/
Node::Node (const wchar_t *szName) :
	m_szName (null), m_szContent (null), m_pParent (null), m_nContentLength (0)
{
	if (szName)
	{
		m_szName = new wchar_t [wcslen (szName) + 1];
		if (m_szName)
			wcscpy (m_szName, szName);
	}
}


/****************************************************************************/
/** Delete member data and child nodes
*****************************************************************************/
Node::~Node ()
{
	delete [] m_szName;
	delete [] m_szContent;

	List::iterator iter, iterLast = m_Children.end ();

	for (iter = m_Children.begin (); iter != iterLast; iter++)
	{
		delete *iter;
	}

	while (m_Attributes.size ())
	{
		Attribute *pAttr = m_Attributes.front ();
		m_Attributes.pop_front ();
		delete pAttr;
	}
}


/****************************************************************************/
/** Add the given data to the node's content buffer
*****************************************************************************/
void Node::vAddContent (const wchar_t *pContent, Generic::UINT32 nCharacters)
{
	wchar_t *szNewBuffer = null;
	wchar_t *szAppendAt = null;

	if (m_szContent)
	{			
		szNewBuffer = new wchar_t [m_nContentLength + nCharacters + 1];
		wcscpy (szNewBuffer, m_szContent);
		szAppendAt = szNewBuffer + m_nContentLength;
	}
	else
	{
		szNewBuffer = new wchar_t [nCharacters + 1];
		szAppendAt = szNewBuffer;
	}

	UINT32 nAppend = nCharacters * sizeof (wchar_t);
	memcpy (szAppendAt, pContent, nAppend);
	szAppendAt[nCharacters] = 0;

	delete [] m_szContent;
	m_szContent = szNewBuffer;
	m_nContentLength += nCharacters;
}

/****************************************************************************/
/** Get the node name
*****************************************************************************/
const wchar_t* Node::szGetName ()
{
	return m_szName;
}


/****************************************************************************/
/** Get the node content
*****************************************************************************/
const wchar_t* Node::szGetContent ()
{
	return m_szContent;
}


/****************************************************************************/
/** Get the node's parent node
*****************************************************************************/
Node* Node::pGetParent ()
{
	return m_pParent;
}


/****************************************************************************/
/** Add a child node
*****************************************************************************/
void Node::vAddChild (Node *pChild)
{
	pChild->m_pParent = this;
	m_Children.push_front (pChild);
}


/****************************************************************************/
/** Return true if the node name matches the given string
*****************************************************************************/
int Node::operator== (const wchar_t *szCompare)
{
	return !wcscmp (m_szName, szCompare);
}


/****************************************************************************/
/** Return a child node with the given name.
 **
 ** TODO: rewrite this to return a list of child nodes with the given name
*****************************************************************************/
Node* Node::pGetChild (const wchar_t *szName)
{
	List::iterator iter, iterLast = m_Children.end ();

	for (iter = m_Children.begin (); iter != iterLast; iter++)
	{
		if (!wcscmp ((*iter)->szGetName (), szName))
			break;
	}

	if (iter == m_Children.end ())
		return null;

	return *iter;
}


/****************************************************************************/
/** Return true if the given node has the same name as this node
*****************************************************************************/
int Node::operator== (Node &Val)
{
	return !wcscmp (Val.szGetName (), m_szName);
}


/****************************************************************************/
/** 
*****************************************************************************/
void Node::AddAttribute (const wchar_t* szGivenName, int cchName, const wchar_t *szGivenValue, int cchValue)
{
	Attribute *pAttr = new Attribute (szGivenName, cchName, szGivenValue, cchValue);
	m_Attributes.push_front (pAttr);
}


/****************************************************************************/
/** 
*****************************************************************************/
const wchar_t *Node::szGetAttributeValue (const wchar_t *szName)
{
	AttributeList::iterator iter, iterLast = m_Attributes.end ();
	for (iter = m_Attributes.begin (); iter != iterLast; iter++)
	{
		Attribute *pAttribute = *iter;
		if (!pAttribute)
			continue;

		if (!wcscmp (pAttribute->szGetName (), szName))
			return pAttribute->szGetValue ();
	}

	return null;
}


/****************************************************************************/
/** 
*****************************************************************************/
Attribute::Attribute (const wchar_t* szName, int cchName, const wchar_t *szValue, int cchValue) :
	m_szName (null), m_szValue (null)
{
	m_szName = new wchar_t[cchName + 1];
	wcsncpy (m_szName, szName, cchName);
	m_szName[cchName] = 0;
	
	m_szValue = new wchar_t[cchValue + 1];
	wcsncpy (m_szValue, szValue, cchValue);
	m_szValue[cchValue] = 0;
}

/****************************************************************************/
/** 
*****************************************************************************/
Attribute::~Attribute ()
{
	delete [] m_szName;
	delete [] m_szValue;
}

// XML.cpp ends here ->