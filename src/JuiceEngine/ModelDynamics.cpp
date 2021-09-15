/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: ModelDynamics.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>
#include <math.h>

using namespace Juice;
using namespace Juice::Dynamics;

/****************************************************************************/
/** Nothing special here.
*****************************************************************************/
void Juice::Model::vSetWorld (Dynamics::World *pWorld)
{
	m_pDynamicsWorld = pWorld;
}

/****************************************************************************/
/** This is pretty self-explanatory.
***********************************f******************************************/
Juice::Dynamics::World* Juice::Model::pGetWorld ()
{
	return m_pDynamicsWorld;
}

/****************************************************************************/
/** This will initialize the dynamics library in preparation for simulation.
***
*** It would have been simpler to use vBuildSingleList, but it's important
*** that the bodies be initialized before the joints, so the different groups
*** are handled explicitly instead.
***
*** TODO: rewrite to use a ModelVisitor
*****************************************************************************/
void Juice::Model::vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld)
{
	// set global physics parameters
//	m_pApplication->m_pWorld->vChangeParameters ();

	ComponentList Components;
	vBuildSingleList (Components);

	ComponentList::iterator iter, last = Components.end ();
	for (iter = Components.begin (); iter != last; iter++)
	{
		Component *pComponent = *iter;
		if (!pComponent)
		{
			Breakpoint ("Model::vPrepareSimulation: null in component list\n");
			continue;
		}

		pComponent->vPrepareSimulation (vecOffset, pWorld);
	}

	// iterate behaviors
	BehaviorList::iterator iterBehaviors;
	for (iterBehaviors = m_Behaviors.begin (); iterBehaviors != m_Behaviors.end (); iterBehaviors++)
	{
		Behavior *pBehavior = *iterBehaviors;
		if (!pBehavior)
		{
			Breakpoint ("Model::vPrepareSimulation: null in behavior list\n");
			continue;
		}

		pBehavior->vPrepareSimulation ();
	}

	m_rPhase = m_rPosePhase;
}

/****************************************************************************/
/** TODO: rewrite to use a ModelVisitor
*****************************************************************************/
void Juice::Model::vDestroySimulation ()
{
	ComponentList Components;
	vBuildSingleList (Components);

	ComponentList::iterator iter, last = Components.end ();
	for (iter = Components.begin (); iter != last; iter++)
	{
		Component *pComponent = *iter;
		if (!pComponent)
		{
			Breakpoint ("Model::vDestroySimulation: null in component list\n");
			continue;
		}

		pComponent->vDestroySimulation ();
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void Juice::Component::vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
void Juice::Component::vDestroySimulation ()
{
}

/****************************************************************************/
/** 
*****************************************************************************/
void Juice::Body::vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld)
{
	if (pWorld)
	{
		m_pDynamicsObject = pWorld->pCreateBody ();
	}

	if (!m_pDynamicsObject)
	{
		GenericUnexpected ("body disabled");
		return;
	}

	// create the dynamics object
	m_pDynamicsObject->vCreate (this);

	// set position
	m_pDynamicsObject->vSetPosition (m_vecPosition + vecOffset);

	// set orientation
	m_pDynamicsObject->vSetRotation (m_vecRotation);

	// set mass and collision hull
	switch (m_eShape)
	{
	case Shape::sBox:
		m_pDynamicsObject->vSetFormBox (m_vecBoxDimensions, m_rMass);
		break;

	case Shape::sCylinder:
		m_pDynamicsObject->vSetFormCylinder (m_rTubeLength / 2, m_rTubeDiameter / 2, m_rMass);
		break;

	case Shape::sCapsule:
		m_pDynamicsObject->vSetFormCapsule (m_rTubeLength, m_rTubeDiameter / 2, m_rMass);
		break;

	case Shape::sBall:
		m_pDynamicsObject->vSetFormBall (m_rBallDiameter / 2, m_rMass);
		break;

	default:
		GenericUnexpected ("collision disabled for this body");
	}
}

void Juice::Body::vDestroySimulation ()
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vDelete ();
	m_pDynamicsObject = null;
}

/****************************************************************************/
/** This adds an external force to the body.  It's mostly useful when pulling
*** bodies around with the mouse, during simulation.
*****************************************************************************/
void Juice::Body::vAddForce (VectorXYZ &vecForce)
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vAddForce (vecForce);
}

/****************************************************************************/
/** This will create a joint that attaches the body to the static environment
*****************************************************************************/
void Juice::Body::vSetStatic (bool fStatic)
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vSetStatic (fStatic);
}

/****************************************************************************/
/** This is useless... only the derived classes have dynamics object members.
*****************************************************************************/
void Juice::Joint::vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
void Juice::Joint::vDestroySimulation ()
{
}

/****************************************************************************/
/** 
*****************************************************************************/
void Juice::RevoluteJoint::vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld)
{
	if (pWorld)
	{
		m_pDynamicsObject = pWorld->pCreateRevoluteJoint ();
	}

	if (!m_pDynamicsObject)
		return;

	if (m_pBody1 && m_pBody2 && (m_pBody1 != m_pBody2))
	{
		m_pDynamicsObject->vCreate (this, m_pBody1->m_pDynamicsObject, m_pBody2->m_pDynamicsObject);

		VectorYPR vecRotation = m_vecRotation;
		vecRotation.pitch *= -1;

		m_pDynamicsObject->vSetAnchorAndAxis (m_vecPosition + vecOffset, vecRotation);
		//m_pDynamicsObject->vSetDamping ();
		m_pDynamicsObject->vSetLimits (m_fLoStop, m_rLoStop, m_fHiStop, m_rHiStop);
	}
}

void Juice::RevoluteJoint::vDestroySimulation ()
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vDelete ();

	m_pDynamicsObject = null;
}

/****************************************************************************/
/** In addition to the basic dynamics stuff (axis, orientation, bodies),
*** this function also stores some information about the positions of the 
*** slider's position relative to the bodies that it connects.  This 
*** information is used later to draw the slider in the appropriate position 
*** and with the appropriate endpoints.
*****************************************************************************/
void Juice::PrismaticJoint::vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld)
{
	if (pWorld)
	{
		m_pDynamicsObject = pWorld->pCreatePrismaticJoint ();
	}

	if (!m_pDynamicsObject)
		return;

	if (m_pBody1 && m_pBody2 && (m_pBody1 != m_pBody2))
	{
		m_pDynamicsObject->vCreate (this, m_pBody1->m_pDynamicsObject, m_pBody2->m_pDynamicsObject);

		m_pDynamicsObject->vSetAnchorAndAxis (m_vecPosition + vecOffset, m_vecRotation);
		m_pDynamicsObject->vSetLimits (m_fLoStop, m_rLoStop, m_fHiStop, m_rHiStop);

		// get equation of axis line
		VectorXYZ vecAxis (0, 0, -1);
		vecAxis.vRotateByDegrees (-m_vecRotation);

		// get intersections of axis line and planes perpendicular to axis line containing body positions
		VectorXYZ vecBody1Position, vecBody2Position;
		m_pBody1->vGetPosition (vecBody1Position);
		m_pBody2->vGetPosition (vecBody2Position);

		// (in world coordinates)
		m_vecBody1EndPoint = vecIntersectionOfLineAndPlane (m_vecPosition, vecAxis, vecAxis, vecBody1Position);
		m_vecBody2EndPoint = vecIntersectionOfLineAndPlane (m_vecPosition, vecAxis, vecAxis, vecBody2Position);

		// convert to body coordinates
		m_vecBody1EndPoint += -vecBody1Position;
		m_vecBody2EndPoint += -vecBody2Position;
	}
}

void Juice::PrismaticJoint::vDestroySimulation ()
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vDelete ();

	m_pDynamicsObject = null;
}


/****************************************************************************/
/** 
*****************************************************************************/
void Juice::SphericalJoint::vPrepareSimulation (VectorXYZ &vecOffset, Dynamics::World *pWorld)
{
	if (pWorld)
	{
		m_pDynamicsObject = pWorld->pCreateSphericalJoint ();
	}

	if (!m_pDynamicsObject)
		return;

	if (m_pBody1 && m_pBody2 && (m_pBody1 != m_pBody2))
	{
		m_pDynamicsObject->vCreate (this, m_pBody1->m_pDynamicsObject, m_pBody2->m_pDynamicsObject);

		m_pDynamicsObject->vSetAnchor (m_vecPosition + vecOffset);
	}
}

void Juice::SphericalJoint::vDestroySimulation ()
{
	if (m_pDynamicsObject)
		m_pDynamicsObject->vDelete ();

	m_pDynamicsObject = null;
}


/****************************************************************************/
/** 
*****************************************************************************/
void Juice::Model::vStep (Generic::UINT32 uMilliseconds, ControlInput *pController)
{
	if (!pController)
		return;

	if (!m_pApplication->fPaused () && (m_pApplication->eGetAppMode () == amOperate))
	{
		// compute the motor's phase 
		real rDelta = (m_rStandardSpeed / 60) * (Generic::rPi * 2) * ((real) uMilliseconds / 1000);
		rDelta *= m_rSpeedFactor;

		m_rPhase += rDelta;
	}

	// phase = phase mod 2pi
	if (m_rPhase > (rPi * 2))
		m_rPhase -= (rPi * 2);

	// Iterate joints, resetting motion accumulators
	std::list<Juice::RevoluteJoint*>::iterator iterRevoluteJoints;
	for (iterRevoluteJoints = m_RevoluteJoints.begin (); iterRevoluteJoints != m_RevoluteJoints.end (); iterRevoluteJoints++)
	{
		Joint *pJoint = (*iterRevoluteJoints);
		pJoint->vInitAccumulators ();
	}

	std::list<Juice::PrismaticJoint*>::iterator iterPrismaticJoints;
	for (iterPrismaticJoints = m_PrismaticJoints.begin (); iterPrismaticJoints != m_PrismaticJoints.end (); iterPrismaticJoints++)
	{
		Joint *pJoint = (*iterPrismaticJoints);
		pJoint->vInitAccumulators ();
	}

	// iterate behaviors
	BehaviorList::iterator iterBehavior;
	for (iterBehavior = m_Behaviors.begin (); iterBehavior != m_Behaviors.end (); iterBehavior++)
	{
		Behavior *pBehavior = *iterBehavior;
		if (!pBehavior || !pBehavior->fGetEnableState ())
			continue;

		pBehavior->vStep (m_rPhase, uMilliseconds, pController);
	}
	

	// Iterate joints, applying forces
	for (iterRevoluteJoints = m_RevoluteJoints.begin (); iterRevoluteJoints != m_RevoluteJoints.end (); iterRevoluteJoints++)
	{
		Joint *pJoint = (*iterRevoluteJoints);
		pJoint->vApplyForce ();
	}

	for (iterPrismaticJoints = m_PrismaticJoints.begin (); iterPrismaticJoints != m_PrismaticJoints.end (); iterPrismaticJoints++)
	{
		Joint *pJoint = (*iterPrismaticJoints);
		pJoint->vApplyForce ();
	}
}

void ClockworkBehavior::vStep (real rEnginePhase, Generic::UINT32 uMilliseconds, ControlInput *pController)
{
	// Set motion source
	real rMotionSourcePhase = 0;
	MotionSourceType eType = mstInvalid;

	switch (iGetMotionSource ())
	{
	case Juice::msMainEngine:
		rMotionSourcePhase = rEnginePhase;
		eType = mstEngine;
		break;

	case Juice::msJoystick1X:
		rMotionSourcePhase = pController->m_rJoystick1X;
		eType = mstControl;
		break;

	case Juice::msJoystick1Y:
		rMotionSourcePhase = pController->m_rJoystick1Y;
		eType = mstControl;
		break;

	case Juice::msJoystick1Z:
		rMotionSourcePhase = pController->m_rJoystick1Z;
		eType = mstControl;
		break;

	case Juice::msJoystick1R:
		rMotionSourcePhase = pController->m_rJoystick1R;
		eType = mstControl;
		break;

	case Juice::msJoystick2X:
		rMotionSourcePhase = pController->m_rJoystick2X;
		eType = mstControl;
		break;

	case Juice::msJoystick2Y:
		rMotionSourcePhase = pController->m_rJoystick2Y;
		eType = mstControl;
		break;

	case Juice::msJoystick2Z:
		rMotionSourcePhase = pController->m_rJoystick2Z;
		eType = mstControl;
		break;

	case Juice::msJoystick2R:
		rMotionSourcePhase = pController->m_rJoystick2R;
		eType = mstControl;
		break;

	case Juice::msBodyYaw:
	case Juice::msBodyPitch:
	case Juice::msBodyRoll:
		rMotionSourcePhase = rGetFeedback ();
		eType = mstFeedback;
		break;

	default:
		rMotionSourcePhase = 0;
		break;
	}

	// Set amplitude modifier
	real rAmpMod = 0;

	switch (iGetAmpModSource ())
	{
	case Juice::amsNone:
		rAmpMod = 1;
		break;

	case Juice::amsJoystickX:
		rAmpMod = pController->m_rJoystick1X;
		break;

	case Juice::amsJoystickY:
		rAmpMod = pController->m_rJoystick1Y;
		break;

	case Juice::amsJoystickZ:
		rAmpMod = pController->m_rJoystick1Z;
		break;

	case Juice::amsJoystickR:
		rAmpMod = pController->m_rJoystick1R;
		break;

	default:
		rAmpMod = 1;
		break;
	}

	rAmpMod *= (rGetGain () / 100);

	// iterate motion sources
	ClockworkMotorList::iterator iterMotor;
	for (iterMotor = m_lsMotors.begin (); iterMotor != m_lsMotors.end (); iterMotor++)
	{
		ClockworkMotor *pMotor = (ClockworkMotor*) *iterMotor;
		if (!pMotor || !pMotor->fGetEnableState ())
			continue;

		// Get desired joint movement
		real rDesiredPosition = 0;
		if (pMotor->fGetDesiredPosition (eType, rMotionSourcePhase, rAmpMod, rDesiredPosition))
		{			
			// Get the joint for this motor
			Joint *pJoint = pMotor->pGetJoint();
			if (!pJoint)
			{
				Breakpoint ("ClockworkBehavior::vStep: motor has no joint.");
				continue;
			}

			// Add movement to the joint's accumulator
			pJoint->vAddPosition (rDesiredPosition);
		}
	}
}

/****************************************************************************/
/** motion functions
*****************************************************************************/
real Juice::rSine (real rPhase)
{
	return sin (rPhase);
}

real Juice::rDoubleSine (real rPhase)
{
	return sin (rPhase * 2);
}

real Juice::rStep (real rPhase)
{
	const real rTwoPi = rPi * 2;

	if (rPhase < 0)
		rPhase += rTwoPi;

	if      (rPhase < (1 * rTwoPi / 8))
	{
		return sin (rPhase * 2);
	}
	else if (rPhase < (2 * rTwoPi / 8))
	{
		return 1;
	}
	else if (rPhase < (3 * rTwoPi / 8))
	{
		return 1;
	}
	else if (rPhase < (4 * rTwoPi / 8))
	{
		return sin (((rPhase - rPi) * 2) + rPi);
	}
	else if (rPhase < (5 * rTwoPi / 8))
	{
		return sin (((rPhase - rPi) * 2) + rPi);
	}
	else if (rPhase < (6 * rTwoPi / 8))
	{
		return -1;
	}
	else if (rPhase < (7 * rTwoPi / 8))
	{
		return -1;
	}
	else if (rPhase < (8 * rTwoPi / 8))
	{
		return sin (((rPhase - rTwoPi) * 2) + rTwoPi);
	}
	else
	{
		return rStep (rPhase - rTwoPi);
	}
}

/****************************************************************************/
/** 
*** rModelPhase varies from 0 to 2*pi
*** control input 'phase' varies from -1 to +1
*** m_rPhase determines this joint's phase relative to the model's phase
***
*** The 'MotionSourceType' thing is a bit of a hack... should this be many
*** functions, like 'rGetEngineEffect / rGetControlEffect' or what?
***
*****************************************************************************/
bool ClockworkMotor::fGetDesiredPosition (MotionSourceType eType, real rInputPhase, real rAmpMod, real &rDesiredPosition)
{
	// Initialize this, just for cleanliness
	rDesiredPosition = 0;

	// m_rAmplitude is the joint's range of motion, e.g. +/- 30 degrees or +/- 30 linear units
	// This scales that down according to e.g. joystick position
	real rAmplitude = m_rAmplitude * rAmpMod;

	// Control input handling
	if (eType == mstControl)
	{
		real rResult = m_rStaticOffset + (rInputPhase * rAmplitude);
		
		if (rResult > 180)
			rResult = 178;
		
		if (rResult < -180)
			rResult = -178;

		rDesiredPosition = rResult;
		return true;
	}

	// Feedback input handling
	if (eType == mstFeedback)
	{
		real rResult = m_rStaticOffset + (rInputPhase * rAmpMod);
		
		rDesiredPosition = rResult;
		return true;
	}

	// Engine handling
	if (eType == mstEngine)
	{
		// Compute the phase for this motor
		real rPhase = (m_rPhase * rDegreesToRadians) + rInputPhase;

		// Clamp it to 2*pi
		if (rPhase > (Generic::rPi * 2))
			rPhase -= (Generic::rPi * 2);

		// Compute the desired dynamic offset 
		real rDynamicOffset = 0;
		
		switch (m_eMotionType)
		{
		case mtNone:		
			return false;

		case mtSine:
			rDynamicOffset = rSine (rPhase);
			rDynamicOffset *= rAmplitude;
			break;

		case mtDoubleSine:
			rDynamicOffset = rDoubleSine (rPhase);
			rDynamicOffset *= rAmplitude;
			break;

		case mtStep:
			rDynamicOffset = rStep (rPhase);
			rDynamicOffset *= rAmplitude;
			break;

		case mtContinuous:
			if (typeid (*m_pJoint) == typeid (RevoluteJoint))
				return false;

			rDynamicOffset = rPhase * rRadiansToDegrees;
			break;

		default:
			Breakpoint ("ClockworkMotor::fGetDesiredPosition: invalid motion type.");
			return false;
		}

		rDesiredPosition = m_rStaticOffset + rDynamicOffset;

		return true;
	}

	Breakpoint ("ClockworkMotor::fGetDesiredPosition: invalid motion source.");
	return false;
}

/****************************************************************************/
/** 
*****************************************************************************/
real ClockworkBehavior::rGetFeedback ()
{
	if (!m_pFeedbackSource)
		return 0;

//	VectorYPR vecEuler;

//	m_pFeedbackSource->vGetRotation (vecEuler);

	real rRotation = 0;

	switch (m_eMotionSource)
	{
	case msBodyYaw:
		rRotation = m_pFeedbackSource->rGetRotationY ();
		break;

	case msBodyPitch:
		rRotation = m_pFeedbackSource->rGetRotationP ();
		break;

	case msBodyRoll:
		rRotation = m_pFeedbackSource->rGetRotationR ();
		break;

	default:
		Breakpoint ("ClockworkBehavior::rGetFeedback: invalid motion source.");
		rRotation = 0;
	}

	return rRotation * rRadiansToDegrees;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Juice::Joint::vApplyForce ()
{
	if (m_uVelocities)
	{
		vApplyForce (m_rVelocityAccumulator, m_rMaxForce);
		return;
	}

	if (m_uPositions)
	{
		real rDesiredPosition = m_rPositionAccumulator;

		// prevent motors from conflicting with joints
		if (m_fLoStop && (rDesiredPosition < m_rLoStop))
			rDesiredPosition = m_rLoStop;

		if (m_fHiStop && (rDesiredPosition > m_rHiStop))
			rDesiredPosition = m_rHiStop;

		vApplyForce (rDesiredPosition);
		return;
	}

	// This effectively turns off any motorization
	vApplyForce (0, 0);
}

// ModelDynamics.cpp ends here ->