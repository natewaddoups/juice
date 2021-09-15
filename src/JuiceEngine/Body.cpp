/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Body.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>
#include <Model.h>

using namespace Juice;

/****************************************************************************/
/** Constructor - just initialize everything.  The string passed to the
 ** Component constructor is the XML tag name for serialization.
*****************************************************************************/
Body::Body (Model *pModel) : 
	Component (pModel, L"Body"), // ctBody, 
	m_rMass (1),
	m_rTubeDiameter (1), m_rTubeLength (1),
	m_rBallDiameter (1),
	m_eShape (sBox),
	m_pDynamicsObject (null)
{	
	//m_pDynamicsObject = m_pModel->pGetApplication()->pGetDynamicsFactory()->pCreateBody ();
}

/****************************************************************************/
/** Destructor - nothing to see here, move along...
*****************************************************************************/
Body::~Body ()
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vDelete ();
}

/****************************************************************************/
/** The assignment operator is pretty straightforward, and is used only 
 ** during clipboard operations.
*****************************************************************************/

Body& Body::operator= (const Body& CopyFrom)
{
	Component::operator= (CopyFrom);

	m_vecBoxDimensions = CopyFrom.m_vecBoxDimensions;
	m_rTubeLength = CopyFrom.m_rTubeLength;
	m_rTubeDiameter = CopyFrom.m_rTubeDiameter;
	m_eShape = CopyFrom.m_eShape;
	m_rMass = CopyFrom.m_rMass;

	return *this;
}

/****************************************************************************/
/** Create a copy of this object.  This is used only during clipboard 
 ** operations.  The copy will be placed on the clipboard, or perhaps be
 ** placed directly into the model.
*****************************************************************************/
Selectable* Body::pCreateClipboardCopy () const
{
	Body *pNew = new Body (m_pModel);
	*pNew = *this;
	return (Selectable*) pNew;
}

/****************************************************************************/
/** If the user tries to paste an object over a selected object, this function
 ** tweaks the selected object to match the one on the clipboard.
*****************************************************************************/
bool Body::fPaste (Selectable *pSelectable)
{
	Body *pSource = dynamic_cast <Body*> (pSelectable);

	if (pSource)
	{
		VectorXYZ vecPosition = m_vecPosition;
		*this = *pSource;
		m_vecPosition = vecPosition;
		return true;
	}

	return false;
}

/****************************************************************************/
/** Set the X, Y, and Z dimensions of the box, all at once.  This primarily 
 ** for use by automated model generator code.  The rest of the code uses
 ** the property system, or (during serialization) direct access to the data.
*****************************************************************************/
void Body::vSetDimensions (const VectorXYZ &vecDimensions)
{
	m_vecBoxDimensions = vecDimensions;
	Generic::Notifier<ComponentEvents>::vNotify (&Juice::ComponentEvents::Component_Changed);
}


/****************************************************************************/
/* 'Getter' and 'setter' functions for property infrastructure. these are 
 * pretty straightforward.
*****************************************************************************/

void Body::vSetDimensionX (real rX)
{
	unless (rX > 0)
		rX = 0.01;

	m_vecBoxDimensions.x = rX;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

void Body::vSetDimensionY (real rY)
{
	unless (rY > 0)
		rY = 0.01;

	m_vecBoxDimensions.y = rY;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

void Body::vSetDimensionZ (real rZ)
{
	unless (rZ > 0)
		rZ = 0.01;

	m_vecBoxDimensions.z = rZ;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

void Body::vSetTubeDiameter (real rDiameter)
{
	unless (rDiameter > 0)
		rDiameter = 0.01;

	m_rTubeDiameter = rDiameter;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

void Body::vSetTubeLength (real rLength)
{
	unless (rLength > 0)
		rLength = 0.01;

	m_rTubeLength = rLength;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

void Body::vSetBallDiameter (real rDiameter)
{
	unless (rDiameter > 0)
		rDiameter = 0.01;

	m_rBallDiameter = rDiameter;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

void Body::vSetShape (int eShape)
{
	m_eShape = (Shape) eShape;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

void Body::vSetMass (real rMass)
{
	unless (rMass > 0.01)
		rMass = 0.01;

	m_rMass = rMass;
	Generic::Notifier<ComponentEvents>::vNotify(&Juice::ComponentEvents::Component_Changed);
}

real Body::rGetDimensionX ()
{
	return m_vecBoxDimensions.x;
}

real Body::rGetDimensionY ()
{
	return m_vecBoxDimensions.y;
}

real Body::rGetDimensionZ ()
{
	return m_vecBoxDimensions.z;
}

real Body::rGetTubeDiameter ()
{
	return m_rTubeDiameter;
}

real Body::rGetTubeLength ()
{
	return m_rTubeLength;
}

real Body::rGetBallDiameter ()
{
	return m_rBallDiameter;
}

int Body::iGetShape ()
{
	return m_eShape;
}

real Body::rGetMass ()
{
	return m_rMass;
}

real Body::rGetRotationY ()
{
	real rResult = 0;

	if (m_pModel->pGetApplication()->eGetAppMode () == amDesign)
	{
		rResult = m_vecRotation.yaw;
	}
	else
	{
		if (m_pDynamicsObject)
			rResult = m_pDynamicsObject->rGetRotationYaw ();
	}

	return rResult;
}

real Body::rGetRotationP ()
{
	real rResult = 0;

	if (m_pModel->pGetApplication()->eGetAppMode () == amDesign)
	{
		rResult = m_vecRotation.pitch;
	}
	else
	{
		if (m_pDynamicsObject)
			rResult = m_pDynamicsObject->rGetRotationPitch ();
	}

	return rResult;
}

real Body::rGetRotationR ()
{
	real rResult = 0;

	if (m_pModel->pGetApplication()->eGetAppMode () == amDesign)
	{
		rResult = m_vecRotation.roll;
	}
	else
	{
		if (m_pDynamicsObject)
			rResult = m_pDynamicsObject->rGetRotationRoll ();
	}

	return rResult;
}


/****************************************************************************/
/* Build the property tree
*****************************************************************************/
PropertyTreePtr Body::ms_pPropertyTree;

Generic::PropertyTree* Body::pGetPropertyTree ()
{
	if (!ms_pPropertyTree)
	{
		PropertyTree *pParentTree = Component::pGetPropertyTree ();
		ms_pPropertyTree = new PropertyTree (*pParentTree);

		BodyProperty *pTwin = new BodyProperty("Twin", (fnGetComponent)&Juice::Body::pGetTwin, (fnSetComponent)&Juice::Body::vSetTwin);
		ms_pPropertyTree->vAddNode (pTwin);

		ListProperty *pShape = new ListProperty("Shape", (fnGetInt)&Juice::Body::iGetShape, (fnSetInt)&Juice::Body::vSetShape);
		pShape->vAddItem (Body::Shape::sBox, "Box");
		pShape->vAddItem (Body::Shape::sCapsule, "Capsule");
		pShape->vAddItem (Body::Shape::sCylinder, "Cylinder");
		pShape->vAddItem (Body::Shape::sBall, "Ball");
		ms_pPropertyTree->vAddNode (pShape);

		std::list<Property*> BoxDimensions;	
		BoxDimensions.push_back(new RealProperty("X", (Generic::fnGetReal) &Juice::Body::rGetDimensionX, (Generic::fnSetReal) &Juice::Body::vSetDimensionX));
		BoxDimensions.push_back(new RealProperty("Y", (Generic::fnGetReal) &Juice::Body::rGetDimensionY, (Generic::fnSetReal) &Juice::Body::vSetDimensionY));
		BoxDimensions.push_back(new RealProperty("Z", (Generic::fnGetReal) &Juice::Body::rGetDimensionZ, (Generic::fnSetReal) &Juice::Body::vSetDimensionZ));
		ms_pPropertyTree->vAddNode ("Box Dimensions", BoxDimensions);

		std::list<Property*> TubeDimensions;	
		TubeDimensions.push_back(new RealProperty("Length", (Generic::fnGetReal) &Juice::Body::rGetTubeLength, (Generic::fnSetReal) &Juice::Body::vSetTubeLength));
		TubeDimensions.push_back(new RealProperty("Diameter", (Generic::fnGetReal) &Juice::Body::rGetTubeDiameter, (Generic::fnSetReal)&Juice::Body::vSetTubeDiameter));
		ms_pPropertyTree->vAddNode ("Tube Dimensions", TubeDimensions);

		ms_pPropertyTree->vAddNode(new RealProperty("Ball Diameter", (Generic::fnGetReal) &Juice::Body::rGetBallDiameter, (Generic::fnSetReal) &Juice::Body::vSetBallDiameter));

		ms_pPropertyTree->vAddNode(new RealProperty("Mass", (Generic::fnGetReal) &Juice::Body::rGetMass, (Generic::fnSetReal) &Juice::Body::vSetMass));
	}

	// TODO: add body properties to m_PropertyList
	return ms_pPropertyTree;
}

/****************************************************************************/
/** Get the body's bounding box.  This is used when adding objects to the
 ** model, it ensures that the new object is added outside the existing model
 ** rather than within it.
 **
 ** TODO: factor in the body's orientation
 ** TODO: handle sphere and cylinder shapes
*****************************************************************************/
void Body::vGetStaticBounds (Generic::VectorXYZ &vecMin, Generic::VectorXYZ &vecMax)
{
	// TODO: compute the body's bounding box, given position and orientation, then compare the corners
	real rMaxDimension = max (m_vecBoxDimensions.x, max (m_vecBoxDimensions.y, m_vecBoxDimensions.z));

	vecMax.x = m_vecPosition.x + rMaxDimension;
	vecMax.y = m_vecPosition.y + rMaxDimension;
	vecMax.z = m_vecPosition.z + rMaxDimension;

	vecMin.x = m_vecPosition.x - rMaxDimension;
	vecMin.y = m_vecPosition.y - rMaxDimension;
	vecMin.z = m_vecPosition.z - rMaxDimension;
}

/****************************************************************************/
/** The the body's current position.  The position vector comes from member
 ** data if the app is in design mode; it's fetched from the dynamics library
 ** if the app is in simulation mode.
*****************************************************************************/
void Body::vGetPosition (VectorXYZ &vecPosition)
{
	if (m_pModel->pGetApplication()->eGetAppMode () == amDesign)
	{
		// get position from static data
		vecPosition = m_vecPosition;
	}
	else
	{
		// get position from dynamics object
		if (m_pDynamicsObject)
			m_pDynamicsObject->vGetPosition (vecPosition);
	}
}

/****************************************************************************/
/** The the body's current rotation.  The rotation values come from member
 ** data if the app is in design mode; it's fetched from the dynamics library
 ** if the app is in simulation mode.
*****************************************************************************/
void Body::vGetRotation (VectorYPR &vecRotation)
{
	if (m_pModel->pGetApplication()->eGetAppMode () == amDesign)
	{
		vecRotation = m_vecRotation;
	}
	else
	{
		// TODO: fix the problem in vGetRotation
		GenericTrace ("Body::vGetRotation (VectorYPR) doesn't work right yet!" << std::endl);
		//m_pDynamicsObject->vGetRotation (vecRotation);
	}
}


/****************************************************************************/
/** The the body's current rotation, in quaternion form.  This is used by
 ** the prismatic joint's rendering code during simulation.
*****************************************************************************/
void Body::vGetQuaternion (Quaternion &qRotation)
{
	if (m_pModel->pGetApplication()->eGetAppMode () == amDesign)
	{
		// TODO: get position from static data
		GenericTrace ("Body::vGetQuaternion () doesn't work in design mode!" << std::endl);
	}
	else
	{
		// get position from dynamics object
		if (m_pDynamicsObject)
			m_pDynamicsObject->vGetQuaternion (qRotation);
	}
}

/****************************************************************************/
/** The the body's angular rotation.  No longer used, but it seemed like it
 ** might make a good motion source.  And maybe it would, so I'm keeping it
 ** just in case someone asks for it.
*****************************************************************************/
real Body::rGetAngularVelocity (const VectorXYZ &vecAxis)
{
	if (m_pModel->pGetApplication()->eGetAppMode () == amDesign)
	{
		return 0;
	}
	else
	{
		// get position from dynamics object
		if (m_pDynamicsObject)
			return m_pDynamicsObject->rGetAngularVelocity (vecAxis);
	}

	return 0;
}

/****************************************************************************/
/** The the body's current rotation, in quaternion form.  This is used by
 ** the prismatic joint's rendering code during simulation.
*****************************************************************************/
void Body::vGetVelocity (VectorXYZ &vecVelocity)
{
	if (m_pModel->pGetApplication()->eGetAppMode () == amDesign)
	{
		vecVelocity.vSet (0, 0, 0);
	}
	else
	{
		// get position from dynamics object
		if (m_pDynamicsObject)
			return m_pDynamicsObject->vGetVelocity (vecVelocity);
	}
}

// Body.cpp ends here ->