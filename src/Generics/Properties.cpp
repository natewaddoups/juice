/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: Properties.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#include <list>
#include <GenericTypes.h>
#include <GenericProperties.h>
#include <stdlib.h>

using namespace Generic;

/****************************************************************************/
/** 
*****************************************************************************/
Property::Property (const char* szName, Generic::UINT32 uFlags) : m_strName (szName), m_uReferenceCount (1), m_uFlags (uFlags)
{
}

Property::~Property ()
{
}

void Property::vAddReference ()
{
	m_uReferenceCount++;
}

void Property::vRelease ()
{
	m_uReferenceCount--;

	if (!m_uReferenceCount)
		delete this;
}

void Property::vSetValueString (const char *sz)
{
	std::string &str = const_cast <std::string&> (m_strValueString);
	str = sz;
}

/****************************************************************************/
/** 
*****************************************************************************/
BoolProperty::BoolProperty (const char* szName, fnGetBool fnGet, fnSetBool fnSet, Generic::UINT32 uFlags) :
	Property (szName, uFlags), m_fnGet (fnGet), m_fnSet (fnSet)
{
//	m_iType = iPropTypeBool;
}

/****************************************************************************/
/** 
*****************************************************************************/
RealProperty::RealProperty (const char* szName, fnGetReal fnGet, fnSetReal fnSet, Generic::UINT32 uFlags) :
	Property (szName, uFlags), m_fnGet (fnGet), m_fnSet (fnSet)
{
//	m_iType = iPropTypeReal;
}

/****************************************************************************/
/** 
*****************************************************************************/
IntProperty::IntProperty (const char* szName, fnGetInt fnGet, fnSetInt fnSet, Generic::UINT32 uFlags) :
	Property (szName, uFlags), m_fnGet (fnGet), m_fnSet (fnSet)
{
//	m_iType = iPropTypeInt;
}

/****************************************************************************/
/** 
*****************************************************************************/
ListProperty::ListProperty (const char* szName, fnGetInt fnGet, fnSetInt fnSet, Generic::UINT32 uFlags) :
	Property (szName, uFlags), m_fnGet (fnGet), m_fnSet (fnSet)
{
//	m_iType = iPropTypeList;
}


/****************************************************************************/
/** 
*****************************************************************************/
StringProperty::StringProperty (const char* szName, fnGetString fnGet, fnSetString fnSet, Generic::UINT32 uFlags) :
	Property (szName, uFlags), m_fnGet (fnGet), m_fnSet (fnSet)
{
//	m_iType = iPropTypeString;
}

bool StringProperty::fReadOnly ()
{
	return (!(m_fnSet));
}

/****************************************************************************/
/** 
*****************************************************************************/
ActionProperty::ActionProperty (const char* szName, fnInvoke fn, Generic::UINT32 uFlags) :
	Property (szName, uFlags), m_fnInvoke (fn)
{
//	m_iType = iPropTypeAction;
}

ActionProperty::~ActionProperty ()
{
}

void ActionProperty::vInvoke (HasProperties *pObject)
{
	(pObject->*m_fnInvoke) ();
}

/****************************************************************************/
/** 
*****************************************************************************/
bool BoolProperty::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	if (!pObject || !m_fnSet)
		return false;

	bool fValue = false;
	bool fReturn = false;
	std::string str = sz;

	if (str.length () < 1)
		return false;

	char cFirst = str[0];

	if ((cFirst == 't') || (cFirst == 'T') || (cFirst == 'y') || (cFirst == 'Y'))
	{
		fReturn = true;
		fValue = true;
	}
	else if ((cFirst == 'f') || (cFirst == 'F') || (cFirst == 'n') || (cFirst == 'N'))
	{
		fReturn = true;
		fValue = false;
	}

	if (fReturn)
		(pObject->*m_fnSet) (fValue);

	return fReturn;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool RealProperty::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	if (!pObject || !m_fnSet)
		return false;

	real rValue = 0;
	bool fRelative = false;
	std::string str = sz;
	std::string strTmp;

	if (str.length () < 1)
		return false;

	if (str[0] == '@')
	{
		fRelative = true;
		strTmp = str.substr (1, str.length ());
	}
	else
	{
		strTmp = str;
	}

	real rTempValue = atof (strTmp.c_str ());
	if (fRelative)
		rValue = ((pObject->*m_fnGet) ()) + rTempValue;
	else
		rValue = rTempValue;

	(pObject->*m_fnSet) (rValue);

	// TODO: return false if the supplied string doesn't look like a number
	return true;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool IntProperty::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	if (!pObject || !m_fnSet)
		return false;

	int iValue = 0;
	bool fRelative = false;
	std::string str = sz;
	std::string strTmp;

	if (str.length () < 1)
		return false;

/*	if (str[0] == '@')
	{
		fRelative = true;
		strTmp = str.substr (1, str.length ());
	}
	else
	{
		strTmp = str;
	}

	real rTempValue = atof (strTmp.c_str ());
	if (fRelative)
		m_rValue = m_rValue + rTempValue;
	else
		rValue = rTempValue;
*/
	iValue = atoi (sz);

	(pObject->*m_fnSet) (iValue);

	// TODO: return false if the supplied string doesn't look like a number
	return true;
}


/****************************************************************************/
/** 
*****************************************************************************/
bool StringProperty::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	if (!pObject || !m_fnSet)
		return false;

	(pObject->*m_fnSet) (sz);
	return true;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool ListProperty::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	if (!pObject || !m_fnSet)
		return false;

	std::list<ListProperty::Item>::iterator iter;
	for (iter = m_lsItems.begin (); iter != m_lsItems.end (); iter++)
	{
		Item &item = *iter;

		if (!strcmpi (sz, item.m_strText.c_str ()))
		{
			(pObject->*m_fnSet) (item.m_iID);
		}
	}
	return true;
}

bool ListProperty::fSetValueFromInt (HasProperties *pObject, int iID)
{
	if (!pObject || !m_fnSet)
		return false;

	(pObject->*m_fnSet) (iID);

	return true;
}

int ListProperty::iGetValueToInt (HasProperties *pObject)
{
	if (!pObject || !m_fnSet)
		return false;

	return (pObject->*m_fnGet) ();
}

/****************************************************************************/
/** 
*****************************************************************************/
const char* BoolProperty::szGetValueToString (HasProperties *pObject)
{
	bool fValue = (pObject->*m_fnGet) ();

	if (fValue)
		vSetValueString ("true");
	else
		vSetValueString ("false");

	return m_strValueString.c_str ();
}

const char* IntProperty::szGetValueToString (HasProperties *pObject)
{
	int iValue = (pObject->*m_fnGet) ();
	char szValue[100];

	sprintf (szValue, "%d", iValue);
	vSetValueString (szValue);
	return m_strValueString.c_str ();
}

const char* RealProperty::szGetValueToString (HasProperties *pObject)
{
	real rValue = (pObject->*m_fnGet) ();

	char szValue[50];
	char cSign = 'X';
	Generic::UINT32 uLeft = 0, uRight = 0;

	if (0 == rValue)
	{
		cSign = ' ';
		uLeft = 0;
		uRight = 0;
	}
	else if (rValue > 0)
	{
		cSign = '+';
		uLeft = (UINT32) rValue;
		uRight = (UINT32) (rValue * 1000) - (uLeft * 1000);
	}
	else // (rValue < 0)
	{
		cSign = '-';
		uLeft = (UINT32) -rValue;
		uRight = (UINT32) (-rValue * 1000) - (uLeft * 1000);
	}

	wsprintf (szValue, "%c%d.%03d", cSign, uLeft, uRight);
	vSetValueString (szValue);
	return m_strValueString.c_str();
}

const char* StringProperty::szGetValueToString (HasProperties *pObject)
{
	const char *szValue = (pObject->*m_fnGet) ();
	vSetValueString (szValue);
	return m_strValueString.c_str ();
}

const char* ListProperty::szGetValueToString (HasProperties *pObject)
{
	int iValue = (pObject->*m_fnGet) ();
	std::list<ListProperty::Item>::iterator iter;
	for (iter = m_lsItems.begin (); iter != m_lsItems.end (); iter++)
	{
		Item &item = *iter;

		if (item.m_iID == iValue)
		{
			return item.m_strText.c_str ();
		}
	}

	return "";
}


/****************************************************************************/
/** 
*****************************************************************************/
ListProperty::Item::Item (int iID, const char *szText) : 
	m_iID (iID), 
	m_strText (szText)
{
}

void ListProperty::vAddItem (int iID, const char *szText)
{
	m_lsItems.push_front (Item (iID, szText));
}

Generic::UINT32 ListProperty::uItems ()
{
	return (Generic::UINT32) m_lsItems.size ();
}

bool ListProperty::fGetItem (Generic::UINT32 uItem, int *piID, const char **pszText)
{
	if (uItem > m_lsItems.size ())
		return false;

	std::list<ListProperty::Item>::iterator iter;
	UINT32 uThisItem = 0;
	for (iter = m_lsItems.begin (); iter != m_lsItems.end (); iter++)
	{
		Item &item = *iter;

		if (uThisItem == uItem)
		{
			*piID = item.m_iID;
			*pszText = item.m_strText.c_str ();
			return true;
		}

		uThisItem++;
	}

	return false;
}

/****************************************************************************/
/** 
*****************************************************************************/
PropertyTree::Node::Node (const char* szName, const std::list<Property*> &lsProperties) :
	m_eType (ntGroup), m_strName (szName), m_lsProperties (lsProperties), 	m_pProperty (null)
{
}

PropertyTree::Node::Node (Property *pNewProperty) :
	m_eType (ntProperty), m_pProperty (pNewProperty)
{
}

PropertyTree::Node::Node (Node &Source) :
	m_eType (Source.m_eType), m_strName (Source.m_strName), m_pProperty (null)
{
	switch (m_eType)
	{
	case ntGroup:
		{
			std::list<Property*> lsSource = Source.m_lsProperties;
			std::list<Property*>::iterator iter;
			for (iter = lsSource.begin (); iter != lsSource.end (); iter++)
			{
				Property *pSourceProperty = *iter;
				pSourceProperty->vAddReference ();

				// Experiments from when m_lsProperties was const.  Still bugs me that this don't work!
				//m_lsProperties.push_front (pSourceProperty);
				//std::list<Property*> &lsMyProperties = m_lsProperties;
				//lsMyProperties = 	(const_cast < std::list<Property*> > (m_lsProperties));

				m_lsProperties.push_back (pSourceProperty);
			}
		}
		break;

	case ntProperty:
		m_pProperty = Source.m_pProperty;
		m_pProperty->vAddReference ();
		break;
	}
}

PropertyTree::Node::~Node ()
{
	if (m_pProperty)
	{
		m_pProperty->vRelease ();
		m_pProperty = null;
	}

	std::list <Property*> dlDeletable = m_lsProperties;
	while (dlDeletable.size ())
	{
		Property *pProperty = dlDeletable.front ();

		if (pProperty)
			pProperty->vRelease ();
		else
			Breakpoint ("PropertyTree::Node::~Node: null pointer in m_lsProperties.");

		dlDeletable.pop_front ();
	}
}

const std::list<Generic::Property*>& PropertyTree::Node::lsGetProperties () const
{
	return m_lsProperties;
}

Property* PropertyTree::Node::pGetProperty () const
{
	return m_pProperty;
}

const char* PropertyTree::Node::szGetName () const
{
	return m_strName.c_str();
}

/****************************************************************************/
/** 
*****************************************************************************/

PropertyTree::PropertyTree ()
{
}

PropertyTree::PropertyTree (PropertyTree &Source)
{
	std::list<Node*> lsSource = Source.m_Nodes;
	std::list<Node*>::iterator iter;

	for (iter = lsSource.begin (); iter != lsSource.end (); iter++)
	{
		Node *pNode = *iter;
		Node *pNewNode = new Node (*pNode);
		m_Nodes.push_back (pNewNode);
	}
}

PropertyTree::~PropertyTree ()
{
	std::list<Node*> lsDeletable = m_Nodes;

	while (lsDeletable.size ())
	{
		Node *pNode = lsDeletable.front ();
		delete pNode;
		lsDeletable.pop_front ();
	}
}

void PropertyTree::vAddNode (const char* szName, const std::list<Property*> &lsProperties)
{
	Node *pNewNode = new Node (szName, lsProperties);
	m_Nodes.push_back (pNewNode);
}

void PropertyTree::vAddNode (Property *pNewProperty)
{
	Node *pNewNode = new Node (pNewProperty);
	m_Nodes.push_back (pNewNode);
}

HasProperties::HasProperties ()
{
}

HasProperties::~HasProperties ()
{
}

// Properties.cpp ends here ->