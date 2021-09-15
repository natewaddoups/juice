/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Component.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>

using namespace Juice;


/****************************************************************************/
/** 
*****************************************************************************/
Component::Component (Model *pModel, const wchar_t *wszXmlTag) :
	Selectable (pModel),
	m_pModel (pModel), 
	m_pTwin (null),
	XML::Serializable (wszXmlTag)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
Component::~Component ()
{
}

/****************************************************************************/
/** 
*****************************************************************************/
void Component::vSetTransform (Render::Scene *pScene)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
Color* Component::pGetColor ()
{
	static Color clr;
	return &clr;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Component::vDrawMesh(Render::Scene *)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
void Component::vSetPosition (const VectorXYZ &vecPosition)
{
	m_vecPosition = vecPosition;
	Generic::Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Component::vGetPosition (VectorXYZ &vecPosition)
{
	vecPosition = m_vecPosition;
}

/****************************************************************************/
/** 
*****************************************************************************/
/*void Component::vSetRotation (const VectorYPR &vecRotation)
{
	m_vecRotation = vecRotation;
}*/

/****************************************************************************/
/** 
*****************************************************************************/
void Component::vGetRotation (VectorYPR &vecRotation)
{
	vecRotation = m_vecRotation;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Component::vSetName (const char *szName)
{
	if (m_pModel->pGetApplication()->m_eEditMode == emSymmetric)
	{
		char *szThisSide = "Left", *szTwinSide = "Right";

		if (m_vecPosition.x > 0)
		{
			szThisSide = "Right";
			szTwinSide = "Left";
		}

		std::string strThisName, strTwinName;
		strThisName = strTwinName = szName;

		std::string::size_type iPos = strThisName.find ("LR");
		if (iPos != std::string::npos)
		{
			strThisName.replace (iPos, 2, szThisSide);
			strTwinName.replace (iPos, 2, szTwinSide);
		}

		m_pModel->pGetApplication()->m_eEditMode = emNormal;
		if (m_pTwin)
			m_pTwin->vSetName (strTwinName.c_str ());
		m_pModel->pGetApplication()->m_eEditMode = emSymmetric;

		m_strName = strThisName;
	}
	else
	{
		m_strName = szName;
	}
	
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

/****************************************************************************/
/** 
*****************************************************************************/
const char* Component::szGetName ()
{
	return m_strName.c_str();
}

/****************************************************************************/
/** Note that this function is not called when loading a model from a file.
*** Instead, Model::fLoadFromFile directly sets m_pTwin and calls 
*** vRegisteryForEventsFrom.  It's a bit inelegant, but this function's 
*** signature must conform to the basic 'get' format specified by the
*** ComponentProperty class.
*****************************************************************************/
void Component::vSetTwin (Component *pComponent)
{
	// No point in doing this twice
	if (pComponent == m_pTwin)
		return;

	// Undo the associations with the old twin
	if (m_pTwin)
	{
		Generic::Listener<ComponentEvents>::vUnregister (m_pTwin);
		m_pTwin->Generic::Listener<ComponentEvents>::vUnregister (this);
		m_pTwin->m_pTwin = null;
	}

	// Store the reference to the new twin
	m_pTwin = pComponent;

	if (m_pTwin)
	{
		// Register for notifications from the new twin 
		Generic::Listener<ComponentEvents>::vRegisterForEventsFrom (m_pTwin);
		m_pTwin->Generic::Listener<ComponentEvents>::vRegisterForEventsFrom (this);

		// If the new twin had an old twin...
		if (m_pTwin->m_pTwin)
		{
			// ...tell the new twin's old twin that it no longer has a twin.  
			if (m_pTwin->m_pTwin->m_pTwin != m_pTwin)
				m_pTwin->m_pTwin->m_pTwin = null;
		}

		// Tell the new twin who it's twin is now.
		m_pTwin->m_pTwin = this;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
Component* Component::pGetTwin ()
{
	return m_pTwin;
}

void Component::vSetPositionX (real rX)
{
	if (m_pModel->pGetApplication()->m_eEditMode == emSymmetric)
	{
		if (((m_vecPosition.x > 0) && (rX > 0)) || ((m_vecPosition.x < 0) && (rX < 0)))
		{
			if (m_pModel->pGetApplication()->m_eEditMode == emSymmetric)
			{
				m_pModel->pGetApplication()->m_eEditMode = emNormal;

				if (m_pTwin)
					m_pTwin->vSetPositionX (-rX);

				m_pModel->pGetApplication()->m_eEditMode = emSymmetric;
			}

			m_vecPosition.x = rX;
		}
		else
		{
			if (m_pModel->pGetApplication()->m_eEditMode == emSymmetric)
			{
				m_pModel->pGetApplication()->m_eEditMode = emNormal;

				if (m_pTwin)
					m_pTwin->vSetPositionX (rX);

				m_pModel->pGetApplication()->m_eEditMode = emSymmetric;
			}

			m_vecPosition.x = -rX;
		}
	}
	else
	{
		m_vecPosition.x = rX;
	}

	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

real Component::rGetPositionX ()
{
	return m_vecPosition.x;
}

void Component::vSetPositionY (real rY)
{
	m_vecPosition.y = rY;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

real Component::rGetPositionY ()
{
	return m_vecPosition.y;
}

void Component::vSetPositionZ (real rZ)
{
	m_vecPosition.z = rZ;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

real Component::rGetPositionZ ()
{
	return m_vecPosition.z;
}

void Component::vSetRotationY (real rY)
{
//	m_vecRotation.yaw = rY;
//	Generic::Notifier<ComponentEvents>::vNotify (ComponentEvents::Component_Changed);

	if (m_pModel->pGetApplication()->m_eEditMode == emSymmetric)
	{
		if (((true) && (rY > 0)) || ((true) && (rY < 0))) // m_vecRotation.yaw > 0, m_vecRotation.yaw < 0
		{
			if (m_pModel->pGetApplication()->m_eEditMode == emSymmetric)
			{
				m_pModel->pGetApplication()->m_eEditMode = emNormal;

				if (m_pTwin)
					m_pTwin->vSetRotationY (-rY);

				m_pModel->pGetApplication()->m_eEditMode = emSymmetric;
			}

			m_vecRotation.yaw = rY;
		}
		else
		{
			if (m_pModel->pGetApplication()->m_eEditMode == emSymmetric)
			{
				m_pModel->pGetApplication()->m_eEditMode = emNormal;

				if (m_pTwin)
					m_pTwin->vSetRotationY (rY);

				m_pModel->pGetApplication()->m_eEditMode = emSymmetric;
			}

			m_vecRotation.yaw = -rY;
		}
	}
	else
	{
		m_vecRotation.yaw = rY;
	}

	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

real Component::rGetRotationY ()
{
	return m_vecRotation.yaw;
}

void Component::vSetRotationP (real rP)
{
	m_vecRotation.pitch = rP;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

real Component::rGetRotationP ()
{
	return m_vecRotation.pitch;
}

void Component::vSetRotationR (real rR)
{
//	m_vecRotation.roll = rR;
//	Generic::Notifier<ComponentEvents>::vNotify (ComponentEvents::Component_Changed);

	if (m_pModel->pGetApplication()->m_eEditMode == emSymmetric)
	{
		if (((m_vecRotation.roll > 0) && (rR > 0)) || ((m_vecRotation.roll < 0) && (rR < 0)))
		{
			if (m_pModel->pGetApplication()->m_eEditMode == emSymmetric)
			{
				m_pModel->pGetApplication()->m_eEditMode = emNormal;

				if (m_pTwin)
					m_pTwin->vSetRotationR (-rR);

				m_pModel->pGetApplication()->m_eEditMode = emSymmetric;
			}

			m_vecRotation.roll = rR;
		}
		else
		{
			if (m_pModel->pGetApplication()->m_eEditMode == emSymmetric)
			{
				m_pModel->pGetApplication()->m_eEditMode = emNormal;

				if (m_pTwin)
					m_pTwin->vSetRotationR (rR);

				m_pModel->pGetApplication()->m_eEditMode = emSymmetric;
			}

			m_vecRotation.roll = -rR;
		}
	}
	else
	{
		m_vecRotation.roll = rR;
	}

	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

real Component::rGetRotationR ()
{
	return m_vecRotation.roll;
}


/****************************************************************************/
/** 
*****************************************************************************/
void Component::vGetStaticBounds (Generic::VectorXYZ &vecMin, Generic::VectorXYZ &vecMax)
{
	vecMax.x = vecMax.y = vecMax.z = 0;
	vecMin.x = vecMin.y = vecMin.z = 0;
}

/****************************************************************************/
/** 
*****************************************************************************/
PropertyTreePtr Component::ms_pPropertyTree;

Generic::PropertyTree* Component::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		ms_pPropertyTree = new Generic::PropertyTree;

		StringProperty *pName = new StringProperty("Name", (Generic::fnGetString) &Juice::Component::szGetName, (Generic::fnSetString) &Juice::Component::vSetName);
		ms_pPropertyTree->vAddNode (pName);

//		ComponentProperty *pTwin = new ComponentProperty ("Twin", m_pTwin, (fnSetComponent) vOnUpdateTwin);
//		ms_pPropertyTree->vAddNode (pTwin);

		std::list<Property*> Position;	
		Position.push_back(new RealProperty("X", (Generic::fnGetReal) &Juice::Component::rGetPositionX, (Generic::fnSetReal) &Juice::Component::vSetPositionX));
		Position.push_back(new RealProperty("Y", (Generic::fnGetReal) &Juice::Component::rGetPositionY, (Generic::fnSetReal) &Juice::Component::vSetPositionY));
		Position.push_back(new RealProperty("Z", (Generic::fnGetReal) &Juice::Component::rGetPositionZ, (Generic::fnSetReal) &Juice::Component::vSetPositionZ));
		ms_pPropertyTree->vAddNode ("Position", Position);

		std::list<Property*> Rotation;	
		Rotation.push_back(new RealProperty("Yaw", (Generic::fnGetReal) &Juice::Component::rGetRotationY, (Generic::fnSetReal) &Juice::Component::vSetRotationY));
		Rotation.push_back(new RealProperty("Pitch", (Generic::fnGetReal) &Juice::Component::rGetRotationP, (Generic::fnSetReal) &Juice::Component::vSetRotationP));
		Rotation.push_back(new RealProperty("Roll", (Generic::fnGetReal) &Juice::Component::rGetRotationR, (Generic::fnSetReal) &Juice::Component::vSetRotationR));
		ms_pPropertyTree->vAddNode ("Rotation", Rotation);
	}

	return ms_pPropertyTree;
}

/****************************************************************************/
/** 
*****************************************************************************/
Component& Component::operator= (const Component &Source)
{
	m_pModel = Source.m_pModel;
	m_pTwin = Source.m_pTwin;
	m_strName = Source.m_strName;
	m_vecPosition = Source.m_vecPosition;
	m_vecRotation = Source.m_vecRotation;

	m_uID = (Generic::UINT32) &Source;

	return *this;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Component::Notifier_Deleted (Component *pComponent)
{
	if (m_pTwin == pComponent)
		m_pTwin = null;
}

// Component.cpp ends here ->