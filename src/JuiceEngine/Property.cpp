/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Property.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>
#include <Model.h>

using namespace Juice;

/****************************************************************************
ComponentProperty::ComponentProperty (const std::string &strName, Component *pValue, fnOnUpdateComponent fn) :
	Property (strName), m_pValue (pValue), m_fn (fn)
{
	m_iType = iPropTypeComponent;
}

ComponentProperty::~ComponentProperty ()
{
}

bool ComponentProperty::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	return false;
}

const std::string& ComponentProperty::strValueToString ()
{
	if (m_pValue)
		vSetValueString (m_pValue->szGetName ());
	else
		vSetValueString ("(none)");
	
	return m_strValueString;
}

void ComponentProperty::vSetValue (HasProperties *pObject, Component *pComponent)
{
	m_pValue = pComponent;
	(pObject->*m_fn) (m_pValue);
}
*****************************************************************************/

/****************************************************************************/
/** 
*****************************************************************************/
MotionTypeProperty::MotionTypeProperty (const char *szName, fnGetMotionType fnGet, fnSetMotionType fnSet, Generic::UINT32 uFlags) :
	Property (szName, uFlags), m_fnGet (fnGet), m_fnSet (fnSet)
{
//	m_iType = iPropTypeMotionType;
}

MotionTypeProperty::~MotionTypeProperty ()
{
}

typedef struct
{
	char *sz;
	MotionType id;
} NameMap;

NameMap MotionTypeMap [] = {
	{ "Sine", mtSine },
	{ "DoubleSine", mtDoubleSine },
	{ "Step", mtStep },
	{ "Continuous", mtContinuous },
};

bool MotionTypeProperty::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	for (int i = 0; i < (sizeof (MotionTypeMap) / sizeof (NameMap)); i++)
	{
		if (!lstrcmpi (sz, MotionTypeMap[i].sz))
		{
			(pObject->*m_fnSet) (MotionTypeMap[i].id);
			return true;
		}
	}

	return false;
}

const char* MotionTypeProperty::szGetValueToString (HasProperties *pObject)
{
	MotionType eType = (pObject->*m_fnGet) ();

	for (int i = 0; i < (sizeof (MotionTypeMap) / sizeof (NameMap)); i++)
	{
		if (MotionTypeMap[i].id == eType)
			return MotionTypeMap[i].sz;
	}

	return "(none)";
}

void MotionTypeProperty::vSetValue (HasProperties *pObject, MotionType eMotionType)
{
	(pObject->*m_fnSet) (eMotionType);
}

/****************************************************************************/
/** 
*****************************************************************************/
ComponentProperty::ComponentProperty (const char *szName, fnGetComponent fnGet, fnSetComponent fnSet, Generic::UINT32 uFlags) :
	Property (szName, uFlags), m_fnGet (fnGet), m_fnSet (fnSet)
{
//	m_iType = iPropTypeComponent;
}

/****************************************************************************/
/** 
*****************************************************************************/
ComponentProperty::~ComponentProperty ()
{
}

/****************************************************************************/
/** 
*****************************************************************************/
bool ComponentProperty::fSetValueFromString (HasProperties *pObject, const char *sz)
{
	return false;
}

/****************************************************************************/
/** 
*****************************************************************************/
const char* ComponentProperty::szGetValueToString (HasProperties *pObject)
{
	Component *pComponent = (pObject->*m_fnGet) ();

	if (pComponent)
		vSetValueString (pComponent->szGetName ());
	else
		vSetValueString ("(none)");
	
	return m_strValueString.c_str();
}

/****************************************************************************/
/** 
*****************************************************************************/
void ComponentProperty::vSetValue (HasProperties *pObject, Component *pComponent)
{
	(pObject->*m_fnSet) (pComponent);
}

// Property.cpp ends here ->