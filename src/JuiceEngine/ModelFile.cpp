/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: ModelFile.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>
#include <stack>

using namespace Juice;
using namespace Generic::XML;

/// Current version of juice model files
int g_iFileVersion = 1;

/****************************************************************************/
/** This is used with the XML serialization code to ensure that strings are 
 ** copied into buffers on this DLL's heap.
*****************************************************************************/
void vStringAssign (std::string *pDest, const char *pSrc)
{
	*pDest = pSrc;
}


/****************************************************************************/
/** This class exists to define the upper levels of the Juice file format.
*****************************************************************************/
class JuiceFile : public Serializable
{
	DynamicsLibraryList &m_DynamicsLibraries;
	Model &m_Model;

public:
	JuiceFile (DynamicsLibraryList &List, Model &Model) : 
		m_DynamicsLibraries (List), m_Model (Model),
		Serializable (L"JuiceFile")
	{
	}
	
	void vSerialize (Stream &Stream)
	{
		Stream.vSerialize ((Serializable*) &m_DynamicsLibraries, L"DynamicsConstants");
		Stream.vSerialize ((Serializable*) &m_Model, L"Model");
	}	
};


/****************************************************************************/
/** Read a Juice file at the given path
*****************************************************************************/
bool Application::fReadFile (const char *szPath)
{
	const char *szExt = strrchr (szPath, '.');
	if (!szExt)
		return false;

	// Read an old-style .jm file?
	if (!strcmp (szExt, ".jm"))
	{
		bool fReturn = m_Model.fLoadFromFile (szPath);

		return fReturn;
	}

	// Read a new-style .jm3 file?
	else if (!strcmp (szExt, ".jm3"))
	{
		JuiceFile Object (m_DynamicsLibraries, m_Model);

		// Create an XML stream reader
		StreamReader Reader;

		m_Model.vReset ();

		// Read the XML file
		//return Reader.fRead (szPath, &Object, vStringAssign) && m_Model.fResetPointers ();
		if (!Reader.fRead (szPath, &Object, vStringAssign))
			return false;

		m_Model.vUpdateUI ();

		return true;
	}

	// If execution reaches this point, something has gone wrong.
	return false;
}


/****************************************************************************/
/** Write a Juice file
*****************************************************************************/
bool Application::fWriteFile (const char *szPath)
{
	// Determine the file extension
	const char *szExt = strrchr (szPath, '.');
	if (!szExt)
		return false;

	// Write an old-style .jm file?
	if (!strcmp (szExt, ".jm"))
	{
		return m_Model.fSaveToFile (szPath);
	}
	
	// Write a new-style .jm3 file?
	else if (!strcmp (szExt, ".jm3"))
	{
		JuiceFile Object (m_DynamicsLibraries, m_Model);
		StreamWriter Writer;

		const int iFileVersion = 1;
		bool fResult = Writer.fWrite (szPath, &Object, iFileVersion);				

		return fResult;
	}

	// If execution reaches this point, something has gone wrong.
	return false;
}


/****************************************************************************/
/** Deprecated, only retained for backward compatibility with the original
 ** ".jm" file format.  
*****************************************************************************/
bool Model::fSaveToFile (const char *szPath)
{
	// Create the XML stream writer
	StreamWriter Writer;

	// Write
	bool fReturn = Writer.fWrite (szPath, this, g_iFileVersion);

	// If that succeeded, set the model's "changed" flag to false
	if (fReturn)
		vSetChanged (false);

	return fReturn;	
}

/****************************************************************************/
/** Deprecated, only retained for backward compatibility with the original
 ** ".jm" file format.
*****************************************************************************/
bool Model::fLoadFromFile (const char *szPath)
{
	// Expect to succeed
	bool fReturn = true;

	// Reset the model - delete all bodies, joints, behaviors, etc
	vReset ();

	// Create an XML stream reader
	StreamReader Reader;

	// Read the XML file
	if (!Reader.fRead (szPath, this, vStringAssign))
		fReturn = false;
	
	// Simple profiling... the duration of th next operation will be measured in milliseconds
	DWORD dwStart = GetTickCount ();

	// Reset all intra-model pointers (joints pointing to bodies and so forth)
	vResetPointers ();

	// See how long that took, write the result to the debug console
	DWORD dwFinish = GetTickCount ();
	real rElapsed = dwFinish - dwStart;
	rElapsed /= 1000;
	char szTemp[100];
	sprintf (szTemp, "Reset pointers: %f seconds\n", rElapsed);
	OutputDebugString (szTemp);

	vUpdateUI ();

	vSetChanged (false);

	return fReturn;
}


/****************************************************************************/
/** This function resets all of the intra-model pointers, like pointers from
 ** joints to bodies, from motors to joints, and so forth.
*****************************************************************************/
void Model::vResetPointers ()
{
	// Turn off symmetric editing while we play with pointers
	EditMode eOldEditMode = m_pApplication->m_eEditMode;
	m_pApplication->m_eEditMode = emNormal;

	// Create hash maps of body and joint IDs and pointers
	BodyHashMap BodyMap;
	m_Bodies.vCreateHashMap (BodyMap);

	RevoluteJointHashMap RevoluteJointMap;
	m_RevoluteJoints.vCreateHashMap (RevoluteJointMap);

	PrismaticJointHashMap PrismaticJointMap;
	m_PrismaticJoints.vCreateHashMap (PrismaticJointMap);

	SphericalJointHashMap SphericalJointMap;
	m_SphericalJoints.vCreateHashMap (SphericalJointMap);

	// Reset twin and joint->body pointers
	m_Bodies.vSetTwinPointers (BodyMap);
	m_RevoluteJoints.vSetTwinPointers (RevoluteJointMap);
	m_PrismaticJoints.vSetTwinPointers (PrismaticJointMap);
	m_SphericalJoints.vSetTwinPointers (SphericalJointMap);

	m_RevoluteJoints.vSetBodyPointers (BodyMap);
	m_PrismaticJoints.vSetBodyPointers (BodyMap);
	m_SphericalJoints.vSetBodyPointers (BodyMap);

	// combine joint maps
	JointHashMap JointMap;

	JointMap.insert (RevoluteJointMap.begin (), RevoluteJointMap.end ());
	JointMap.insert (PrismaticJointMap.begin (), PrismaticJointMap.end ());
	JointMap.insert (SphericalJointMap.begin (), SphericalJointMap.end ());

	// reset behavior->body and behavior::motor->joint pointers 
	m_Behaviors.vResetPointers (BodyMap, JointMap);

	// Restore the old edit mode
	m_pApplication->m_eEditMode = eOldEditMode;
}

/****************************************************************************/
/** Component names in the UI will show default values until this method sends
 ** 'Changed' events to the UI.
 **
 ** TODO: Move this functionality into the Application class.
*****************************************************************************/
void Model::vUpdateUI ()
{
	// Save this bit of state, so it can be restored later.
	bool fChanged = m_fChanged;

	// update component names in the UI
	ComponentList lsComponents;
	ComponentList::iterator iterComponents;
	vBuildSingleList (lsComponents);

	for (iterComponents = lsComponents.begin (); iterComponents != lsComponents.end (); iterComponents++)
	{
		Component *pComponent = *iterComponents;
		if (pComponent)
		{
			// bit of a hack, but it does the trick
			pComponent->vSetName (pComponent->szGetName ());
		}
		else
		{
			Breakpoint ("Model::fResetPointers: null pComponent.");
		}
	}

	// iterate behaviors
	BehaviorList::iterator iterBehaviors;
	for (iterBehaviors = m_Behaviors.begin (); iterBehaviors != m_Behaviors.end (); iterBehaviors++)
	{
		Behavior *pBehavior = *iterBehaviors;
		if (!pBehavior)
			continue;

		// Set the UI checkbox for this behavior
		pBehavior->vSetEnableState (pBehavior->fGetEnableState ());

		// Special processing for clockwork behaviors...
		if (IsKindOfPtr <ClockworkBehavior*> (pBehavior))
		{
			ClockworkBehavior* pClockworkBehavior = dynamic_cast <ClockworkBehavior*> (pBehavior);

			// iterate motion sources
			ClockworkMotorList::iterator iterMotors;
			for (iterMotors = pClockworkBehavior->m_lsMotors.begin (); iterMotors != pClockworkBehavior->m_lsMotors.end (); iterMotors++)
			{
				ClockworkMotor *pMotor = *iterMotors;
				if (!pMotor)
					continue;

				// another hack to update the UI
				pMotor->vSetEnableState (pMotor->fGetEnableState ());
			}
		}
	}

	// Restore this
	m_fChanged = fChanged;
}

/****************************************************************************/
/** Serialize the model data
*****************************************************************************/
void Model::vSerialize(Stream &Stream) 
{
	// Physics parameters
	//
	// TODO: Abandon the .jm file format, and get rid of this stuff.  In the
	// newer .jm3 file format, this is all stored elsewhere in the file, on
	// a per-physics-engine basis.
	Stream.vSerialize (&m_pApplication->m_Preferences.m_rGravity,        L"Gravity");
	Stream.vSerialize (&m_pApplication->m_Preferences.m_rGroundFriction, L"Friction");
	Stream.vSerialize (&m_pApplication->m_Preferences.m_rGlobalERP,      L"ERP");
	Stream.vSerialize (&m_pApplication->m_Preferences.m_rGlobalCFM,      L"CFM");

	// Model parameters
	Stream.vSerialize (&m_rStandardSpeed,                   L"StandardSpeed");
	Stream.vSerialize (&m_rSpeedFactor,                     L"SpeedFactor");
	Stream.vSerialize (&m_rPoseDelay,                       L"PoseDelay");
	Stream.vSerialize (&m_rPosePhase,                       L"PosePhase");

	// Components and behaviors
	Stream.vSerialize ((Serializable*) &m_Bodies,          L"Bodies");
	Stream.vSerialize ((Serializable*) &m_RevoluteJoints,  L"RevoluteJoints");
	Stream.vSerialize ((Serializable*) &m_PrismaticJoints, L"PrismaticJoints");
	Stream.vSerialize ((Serializable*) &m_SphericalJoints, L"SphericalJoints");
	Stream.vSerialize ((Serializable*) &m_Behaviors,       L"Behaviors");

	vSetChanged (false);
}

/****************************************************************************/
/** Serialize component data (name, ID, etc)
*****************************************************************************/
void Component::vSerialize(Stream &Stream) 
{
	// TODO: abandon the .jm file format, and eliminate this "ID" code, as 
	// the new XML serialization stuff handles this automagically.
	if (Stream.eGetOperation () == Stream::oWrite)
	{
		int iThis = (int) this;
		Stream.vSerialize ((int*) &iThis,                   L"ID");
	}
	else
	{
		Stream.vSerialize ((int*) &m_uID,                   L"ID");
	}

	Stream.vSerialize (&m_strName,                          L"Name");
	Stream.vSerialize ((Serializable*) &m_vecPosition,      L"Position");
	Stream.vSerialize ((Serializable*) &m_vecRotation,      L"Rotation");
	Stream.vSerialize ((void**) &m_pTwin,                   L"Twin");
}

/****************************************************************************/
/** Serialize body data 
*****************************************************************************/
void Body::vSerialize (Stream &Stream) 
{
	Component::vSerialize (Stream);

	Stream.vSerialize ((Serializable*) &m_vecBoxDimensions, L"BoxDimensions", L"Dimensions");
	Stream.vSerialize (&m_rTubeLength,                      L"TubeLength");
	Stream.vSerialize (&m_rTubeDiameter,                    L"TubeDiameter");
	Stream.vSerialize (&m_rBallDiameter,                    L"BallDiameter");
	Stream.vSerialize ((int*) &m_eShape,                    L"Shape");
	Stream.vSerialize (&m_rMass,                            L"Mass");
}

/****************************************************************************/
/** Serialize joint base-class data 
*****************************************************************************/
void Joint::vSerialize (Stream &Stream) 
{
	Component::vSerialize (Stream);

	Stream.vSerialize ((void**) &m_pBody1,      L"Body1");
	Stream.vSerialize ((void**) &m_pBody2,      L"Body2");

	Stream.vSerialize (&m_rGain,                L"Gain");
	Stream.vSerialize (&m_rMaxForce,            L"MaxForce");

	Stream.vSerialize (&m_rLoStop,              L"LoStopValue");
	Stream.vSerialize (&m_rHiStop,              L"HiStopValue");
	Stream.vSerialize (&m_fLoStop,              L"LoStopFlag");
	Stream.vSerialize (&m_fHiStop,              L"HiStopFlag");
}

/****************************************************************************/
/** Serialize hinge data 
*****************************************************************************/
void RevoluteJoint::vSerialize (Stream &Stream) 
{
	Joint::vSerialize (Stream);

	Stream.vSerialize (&m_rLength,              L"Length");
	Stream.vSerialize (&m_rDiameter,            L"Diameter");
}

/****************************************************************************/
/** Serialize slider data 
*****************************************************************************/
void PrismaticJoint::vSerialize (Stream &Stream) 
{
	Joint::vSerialize (Stream);

	Stream.vSerialize (&m_rLength,              L"Length");
	Stream.vSerialize (&m_rDiameter,            L"Diameter");
}

/****************************************************************************/
/** Serialize ball-joint data 
*****************************************************************************/
void SphericalJoint::vSerialize (Stream &Stream) 
{
	Joint::vSerialize (Stream);

	Stream.vSerialize (&m_rDiameter,            L"Diameter");
}

/****************************************************************************/
/** Serialize behavior base-class data 
*****************************************************************************/
void Behavior::vSerialize (Stream &Stream) 
{
	Stream.vSerialize (&m_strName,              L"Name");
	Stream.vSerialize (&m_fEnabled,             L"Enabled");
	Stream.vSerialize (&m_rGain,                L"Gain");
}

/****************************************************************************/
/** Serialize clockwork behavior data 
*****************************************************************************/
void ClockworkBehavior::vSerialize (Stream &Stream) 
{
	Behavior::vSerialize (Stream);

	Stream.vSerialize ((Serializable*) &m_lsMotors, L"ClockworkMotors", L"MotionLinks");

	Stream.vSerialize ((int*) &m_eMotionSource, L"MotionSource");
	Stream.vSerialize ((int*) &m_eAmpModSource, L"AmpModSource");
	Stream.vSerialize (&m_fAbsoluteValue,       L"AbsoluteValue");
	Stream.vSerialize ((void**) &m_pFeedbackSource, L"FeedbackSource");
}

/****************************************************************************/
/** Serialize clockwork motor data 
*****************************************************************************/
void ClockworkMotor::vSerialize (Stream &Stream) 
{
	Stream.vSerialize ((void**) &m_pJoint,      L"JointID");
	Stream.vSerialize (&m_fEnabled,           L"Enabled");
	Stream.vSerialize (&m_rStaticOffset,      L"Offset");

	Stream.vSerialize ((int*) &m_eMotionType, L"MotionType");
	Stream.vSerialize (&m_rPhase,             L"Phase");
	Stream.vSerialize (&m_rAmplitude,         L"Amplitude");
}

/****************************************************************************/
/** Serialize network behavior data 
*****************************************************************************/
void NetworkBehavior::vSerialize (Stream &Stream)
{
	Behavior::vSerialize (Stream);

	Stream.vSerialize ((Serializable*) &m_lsMotorNodes,        L"MotorNodes");
	Stream.vSerialize ((Serializable*) &m_lsFunctionNodes,     L"FunctionNodes");
	Stream.vSerialize ((Serializable*) &m_lsMotionSourceNodes, L"MotionSourceNodes");
	Stream.vSerialize ((Serializable*) &m_lsEdges,             L"Edges");
}

/****************************************************************************/
/** Serialize network edge data 
*****************************************************************************/
void Edge::vSerialize (Stream &Stream)
{
	Stream.vSerialize ((void**) &m_pSource,      L"Source");
	Stream.vSerialize ((void**) &m_pDestination, L"Destination");
	Stream.vSerialize ((real*)  &m_rFactor,                 L"Factor");
}

/****************************************************************************/
/** Serialize network node base class data 
*****************************************************************************/
void Juice::Node::vSerialize (Stream &Stream)
{
	// TODO: abandon the .jm file format, and eliminate this "ID" code, as 
	// the new XML serialization stuff handles this automagically.
	if (Stream.eGetOperation () == Stream::oWrite)
	{
		int iThis = (int) this;
		Stream.vSerialize ((int*) &iThis,     L"ID");
	}
	else
	{
		Stream.vSerialize ((int*) &m_uID,     L"ID");
	}

	Stream.vSerialize (&m_strName,            L"Name");
	Stream.vSerialize ((int*) &m_iX,          L"X");
	Stream.vSerialize ((int*) &m_iY,          L"Y");
}

/****************************************************************************/
/** Serialize motion source node data 
*****************************************************************************/
void MotionSourceNode::vSerialize (Stream &Stream)
{
	Node::vSerialize (Stream);

	Stream.vSerialize ((int*) &m_eMotionSource, L"MotionSource");
	Stream.vSerialize ((int*) &m_eAmpModSource, L"AmpModSource");
	Stream.vSerialize ((void**) &m_pBody,         L"BodyFeedbackSource");
}

/****************************************************************************/
/** Serialize constant node data 
*****************************************************************************/
void ConstantNode::vSerialize (Stream &Stream)
{
	Node::vSerialize (Stream);

	Stream.vSerialize (&m_rValue,     L"Value");
}

/****************************************************************************/
/** Serialize limit node data 
*****************************************************************************/
void LimitNode::vSerialize (Stream &Stream)
{
	Node::vSerialize (Stream);

	Stream.vSerialize (&m_rMinimum,     L"Minimum");
	Stream.vSerialize (&m_rMaximum,     L"Maximum");
}

/****************************************************************************/
/** Serialize arithmetic node data 
*****************************************************************************/
void ArithmeticNode::vSerialize (Stream &Stream)
{
	Node::vSerialize (Stream);

	Stream.vSerialize (&m_rAdd,       L"Add");
	Stream.vSerialize (&m_rMultiply,  L"Multiply");
	Stream.vSerialize (&m_fAddFirst,  L"AddFirst");
}

/****************************************************************************/
/** Serialize wave function node data 
*****************************************************************************/
void WaveNode::vSerialize (Stream &Stream)
{
	Node::vSerialize (Stream);

	Stream.vSerialize (&m_rDutyCycle,    L"DutyCycle");
	Stream.vSerialize (&m_rOutputRange,  L"OutputRange");
	Stream.vSerialize (&m_rOutputOffset, L"OutputOffset");
}

/****************************************************************************/
/** Serialize motor node data 
*****************************************************************************/
void MotorNode::vSerialize (Stream &Stream)
{
	Node::vSerialize (Stream);

	Stream.vSerialize ((void**) &m_pJoint,     L"JointID");
	Stream.vSerialize ((int*) &m_eMotorType, L"MotorType");
}

// ModelFile.cpp ends here ->