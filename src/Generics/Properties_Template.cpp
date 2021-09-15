/*****************************************************************************
<HEADER>
*****************************************************************************/

#include <list>
#include <GenericTypes.h>
#include <GenericProperties.h>
#include <stdlib.h>

using namespace Generic;

/****************************************************************************/
/** 
*****************************************************************************/
PropertyBase::PropertyBase (const char *szName) : m_strName (szName), m_iType (0)
{
}

PropertyBase::~PropertyBase ()
{
}

void PropertyBase::vSetValueString (const char *sz)
{
	std::string &str = const_cast <std::string&> (m_strValueString);
	str = sz;
}

#define Implement_Property(x) \
	__declspec(dllexport) Property<x>::Property<x> (const char *szName, fnGet fnGetter, fnSet fnSetter) : \
		PropertyBase (szName), \
		m_fnGet (fnGetter), \
		m_fnSet (fnSetter) \
		{ vSetType (); }

Implement_Property(bool);
Implement_Property(real);
Implement_Property(const char*);


/****************************************************************************/
/** 
*****************************************************************************/
/*template <class T>
Property<T>::Property<T> 
	(const char *szName, 
	fnGet fnGetter,
	fnSet fnSetter) 
	:
	Property (szName), 
	m_fValue (fValue),
	m_fnGet (fnGetter),
	m_fnSet (fnSetter)
{
	vSetType ();
}
*/
/****************************************************************************/
/** 
*****************************************************************************/
/*template <class T>
Property<T>::~Property<T> ()
{
}*/

/****************************************************************************/
/** 
*****************************************************************************/
void Property<bool>::vSetType ()
{
	m_iType = iPropTypeBool;
}

void Property<real>::vSetType ()
{
	m_iType = iPropTypeReal;
}

void Property<const char*>::vSetType ()
{
	m_iType = iPropTypeString;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Property<bool>::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	bool fReturn = false;
	std::string str = sz;
	bool fValue;

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
	{
		(pObject->*m_fnSet) (fValue);
		vSetValueString (sz);
	}

	return fReturn;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Property<real>::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	bool fRelative = false;
	std::string str = sz;
	std::string strTmp;
	real rValue;

	if (str.length () < 1)
		return false;

//	if (str[0] == '@')
//	{
//		fRelative = true;
//		strTmp = str.substr (1, str.length ());
//	}
//	else
//	{
		strTmp = str;
//	}

	real rTempValue = atof (strTmp.c_str ());
//	if (fRelative)
//		rValue = rValue + rTempValue;
//	else
		rValue = rTempValue;

	(pObject->*m_fnSet) (rValue);
	vSetValueString (sz);

	// TODO: return false if the supplied string doesn't look like a number
	return true;
}


/****************************************************************************/
/** 
*****************************************************************************/
bool Property<const char*>::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	(pObject->*m_fnSet) (sz);
	vSetValueString (sz);
	return true;
}

/****************************************************************************/
/** 
*****************************************************************************/
const char* Property<bool>::szGetValueToString (HasProperties *pObject)
{
	bool fValue = ((pObject->*m_fnGet) ());

	if (fValue)
		vSetValueString ("true");
	else
		vSetValueString ("false");

	return m_strValueString.c_str ();
}

const char* Property<real>::szGetValueToString (HasProperties *pObject)
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
	else // (m_rValue < 0)
	{
		cSign = '-';
		uLeft = (UINT32) -rValue;
		uRight = (UINT32) (-rValue * 1000) - (uLeft * 1000);
	}

	wsprintf (szValue, "%c%d.%03d", cSign, uLeft, uRight);
	vSetValueString (szValue);
	return m_strValueString.c_str ();
}

const char* Property<const char*>::szGetValueToString (HasProperties *pObject)
{
	const char* szValue = (pObject->*m_fnGet) ();

	vSetValueString (szValue);
	return m_strValueString.c_str ();
}

/****************************************************************************/
/** 
*****************************************************************************/
PropertyTree::Node::Node (const char *szName, const std::list<PropertyBase*> &lsProperties) :
	m_eType (ntGroup), m_strName (szName), m_lsProperties (lsProperties), 	m_pProperty (null)
{
}

PropertyTree::Node::Node (PropertyBase *pNewProperty) :
	m_eType (ntProperty), m_pProperty (pNewProperty)
{
}

PropertyTree::Node::~Node ()
{
	delete m_pProperty;

	std::list <PropertyBase*> dlDeletable = m_lsProperties;
	while (dlDeletable.size ())
	{
		PropertyBase *pProperty = dlDeletable.front ();
		delete pProperty;
		dlDeletable.pop_front ();
	}
}

const std::list<Generic::PropertyBase*>& PropertyTree::Node::lsGetProperties () const
{
	return m_lsProperties;
}

PropertyBase* PropertyTree::Node::pGetProperty () const
{
	return m_pProperty;
}

const char* PropertyTree::Node::szGetName () const
{
	return m_strName.c_str ();
}

/****************************************************************************/
/** 
*****************************************************************************/

PropertyTree::PropertyTree ()
{
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

void PropertyTree::vAddNode (const char *szName, const std::list<PropertyBase*> &lsProperties)
{
	Node *pNewNode = new Node (szName, lsProperties);
	m_Nodes.push_back (pNewNode);
}

void PropertyTree::vAddNode (PropertyBase *pNewProperty)
{
	Node *pNewNode = new Node (pNewProperty);
	m_Nodes.push_back (pNewNode);
}

HasProperties::HasProperties () : 
	m_pPropertyTree (null)
{
}

HasProperties::~HasProperties ()
{
	delete m_pPropertyTree;
}
/*****************************************************************************
<FOOTER>
*****************************************************************************/
