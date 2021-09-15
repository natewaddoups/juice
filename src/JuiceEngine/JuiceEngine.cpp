/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: JuiceEngine.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include <crtdbg.h>
#include <JuiceEngine.h>
#include "Terrain.h"

using namespace Juice;

/****************************************************************************/
/** Application constructor: sets members to a usable initial state
*****************************************************************************/

Application::Application () :
	m_eAppMode (amDesign),
	m_eEditMode (emNormal),
	m_eDragPlane (dpXY),
	m_pPullOn (null),
	m_uPoseTime (0),
	m_pTackedBody (null),
	m_pTerrain (null),
	m_fTerrainEnabled (false),
	m_fTerrainAvailable (false),
	m_pWorld (null),
	m_pLastWorld (null)
{
	// These objects all contain pointers to the application object
	m_Model.vSetApplication (this);
	m_Scene.vSetApplication (this);
	m_Clipboard.vSetApplication (this);
	m_SelectionSet.vSetApplication (this);

	// Load the user preferences from the registry
	m_Preferences.vLoad ();

	// Set the scene's pointer to the model object
	m_Scene.vSetModel (&m_Model);

	// Set the selection set's pointer to the clipboard
	m_SelectionSet.vSetClipboard (&m_Clipboard);
}

/****************************************************************************/
/** Destructor
*****************************************************************************/
Application::~Application ()
{
	m_Model.vReset ();

	// Delete the terrain object
	delete m_pTerrain;

	// Delete the property trees for various classes
	Juice::Component::ms_pPropertyTree.vDelete ();
	Juice::Body::ms_pPropertyTree.vDelete ();
	Juice::Joint::ms_pPropertyTree.vDelete ();
	Juice::RevoluteJoint::ms_pPropertyTree.vDelete ();
	Juice::PrismaticJoint::ms_pPropertyTree.vDelete ();
	Juice::SphericalJoint::ms_pPropertyTree.vDelete ();

	Juice::Behavior::ms_pPropertyTree.vDelete ();
	
	Juice::ClockworkBehavior::ms_pPropertyTree.vDelete ();
	Juice::ClockworkMotor::ms_pPropertyTree.vDelete ();

	Juice::NetworkBehavior::ms_pPropertyTree.vDelete ();
	Juice::Node::ms_pPropertyTree.vDelete ();
	Juice::Edge::ms_pPropertyTree.vDelete ();
	Juice::MotionSourceNode::ms_pPropertyTree.vDelete ();
	Juice::ConstantNode::ms_pPropertyTree.vDelete ();
	Juice::ArithmeticNode::ms_pPropertyTree.vDelete ();
	Juice::MultiplyNode::ms_pPropertyTree.vDelete ();
	Juice::WaveNode::ms_pPropertyTree.vDelete ();
	Juice::MotorNode::ms_pPropertyTree.vDelete ();
}

/****************************************************************************/
/** This function is called by the application as often as possible, to 'step'
 ** the dynamics library and render the world.
*****************************************************************************/
bool Application::fStep (Generic::UINT32 uMilliseconds)
{
	// expect success
	bool fReturn = true;

	// Force a given frame rate if necessary (this is for MPEG recording)
	if (m_uForcedFrameRate)
		uMilliseconds = 1000 / (m_uForcedFrameRate * 2);

	// Switch from 'pose' to 'operate' if the time is right
	if ((m_eAppMode == amPose) && (m_uPoseTime > (m_Model.rGetPoseDelay () * 1000)))
		m_eAppMode = amOperate;
	else
		m_uPoseTime += uMilliseconds;

	// Apply pull (if any)
	if (fPulling ())
		vDoPull ();

	// Do dynamics stuff
	if (m_eAppMode != Juice::amDesign) 
	{
		// Update the controller input
		m_ControlInput.vUpdate ();

		// Do all of the physics stuff, twice if need be
		if (m_uForcedFrameRate)
			fReturn = (fStepPhysics (uMilliseconds / 2) && fStepPhysics (uMilliseconds / 2));
		else
			fReturn = fStepPhysics (uMilliseconds);
	}

#ifdef _DEBUG
	static float Iterations;
	static float TotalTime;
	float  Start = (float) GetTickCount ();
#endif

	// Redraw all 3D views
	m_Scene.vRenderAllViewports ();

#ifdef _DEBUG
	float ThisIteration = (float) GetTickCount () - Start;
	TotalTime += ThisIteration;
	Iterations++;
	static float Average = TotalTime / Iterations;
#endif


	// this tells the caller if the simulation failed
	return fReturn;
}

/****************************************************************************/
/** Apply forces to the model and compute the movements in the world
*****************************************************************************/
bool Application::fStepPhysics (Generic::UINT32 uMilliseconds)
{
	// Think positive...
	bool fReturn = true;

	// Step the model
	m_Model.vStep (uMilliseconds, &m_ControlInput);

#ifdef _DEBUG
	static float Iterations;
	static float TotalTime;
	float  Start = (float) GetTickCount ();
#endif

	if (uMilliseconds < 2)
		uMilliseconds = 2;

	// Use the dynamics library to figure out what moved where
	if (!m_pWorld->fStep (uMilliseconds))
	{
		// Return to design mode if the dynamics simulation fails
		fReturn = false;		
		m_eAppMode = amDesign;
	}
/*
	// Use the dynamics library to figure out what moved where
	if (!m_pWorld->fStep (uMilliseconds / 2))
	{
		// Return to design mode if the dynamics simulation fails
		fReturn = false;		
		m_eAppMode = amDesign;
	}
*/
#ifdef _DEBUG
	float ThisIteration = (float) GetTickCount () - Start;
	TotalTime += ThisIteration;
	Iterations++;
	static float Average = TotalTime / Iterations;
#endif

	return fReturn;
}

/****************************************************************************/
/** Begin a 'pull' operation
*****************************************************************************/
void Application::vBeginPull (Component *pComponent)
{
	m_pPullOn = pComponent;
}

/****************************************************************************/
/** Add a force to the object being pulled
*****************************************************************************/
void Application::vDoPull ()
{
	// Pull is only applicable to operation mode
	if ((m_eAppMode != amDesign) && (m_Scene.fShowCursor ()) && (m_pPullOn))
	{
		// Get the position of the object being pulled
		VectorXYZ vecStartPosition;

		m_pPullOn->vGetPosition (vecStartPosition);
		m_Scene.vSetCursorTail (vecStartPosition);

		// Get the position of the cursor
		VectorXYZ vecPullFrom;
		m_Scene.vGetCursorPosition (vecPullFrom);

		// Compute the pull vector
		m_vecPull = vecPullFrom + -vecStartPosition;

		// Get the mass and velocity of entire selection set
		real rTotalMass = 0;
		SelectableList::const_iterator iter;
		VectorXYZ vecVelocity;
		for (iter = m_SelectionSet.lsGetContents().begin (); iter != m_SelectionSet.lsGetContents().end (); iter++)
		{
			Selectable *pSelectable = *iter;

			Body *pBody = dynamic_cast <Body*> (*iter);

			if (pBody)
			{
				// Add the mass to the total
				rTotalMass += pBody->rGetMass ();

				VectorXYZ vecTemp;
				pBody->vGetVelocity (vecTemp);

				// Add the velocity to the accumulator
				vecVelocity += vecTemp;
			}
		}

		// Divide the velocity accumulator to get the velocity of the selection set
		vecVelocity /= (real) m_SelectionSet.lsGetContents ().size ();

		// Project the velocity onto the pull vector
		VectorXYZ vecProjectedVelocity = vecProject (vecVelocity, m_vecPull);

		// Get pull force (TODO: make these constants user preferences)
		m_vecPull = m_vecPull * (200 * rTotalMass);
		m_vecPull -= vecVelocity * 20;

		// Constrain the pull to a reasonable (but unlikely) maximum
		const real rMaxPull = (1500 * rTotalMass);

		// Limit pull strength to rMaxPull
		if (m_vecPull.rMagnitude () > rMaxPull)
			m_vecPull = m_vecPull * (rMaxPull / m_vecPull.rMagnitude ());
		
		// Iterate over the selection set, applying a pull force to the bodies
		for (iter = m_SelectionSet.lsGetContents().begin (); iter != m_SelectionSet.lsGetContents().end (); iter++)
		{
			Selectable *pSelectable = *iter;

			if (IsKindOf<Body*> (pSelectable))
			{
				Body *pBody = dynamic_cast <Body*> (*iter);

				pBody->vAddForce (m_vecPull);
			}
		}
	}
}

/****************************************************************************/
/** End a pull operation
*****************************************************************************/
void Application::vEndPull ()
{
	m_pPullOn = null;
}

/****************************************************************************/
/** Indicates whether or not a pull operation is in progress
*****************************************************************************/
bool Application::fPulling ()
{
	return !(!(m_pPullOn));
}

/****************************************************************************/
/** Returns the pull force vector 
*****************************************************************************/
void Application::vGetPullVector (VectorXYZ &vecPull)
{
	vecPull = m_vecPull;
}

/****************************************************************************/
/** Cut the selection set
*****************************************************************************/
void Application::vCut ()
{
	m_Clipboard.vCopy (&m_SelectionSet);
	m_SelectionSet.vDelete ();
}

/****************************************************************************/
/** Copy the selection set to the clipboard
*****************************************************************************/
void Application::vCopy ()
{
	m_Clipboard.vCopy (&m_SelectionSet);
}

/****************************************************************************/
/** Paste the clipboard contents into the current model
*****************************************************************************/
void Application::vPaste ()
{
	// Duplicate the clipboard data
	SelectionSet NewObjects;
	m_Clipboard.vCopy (&m_Clipboard.ssGetData (), &NewObjects);
	SelectableList &ClipboardObjects = NewObjects.lsGetContents ();
	SelectableList &SelectedObjects = m_SelectionSet.lsGetContents ();

	SelectableList::iterator iterClipboard, iterSelected;
	SelectableList lsPasted;

	// deselect all objects on the clipboard - they are currently selected as a result
	// of SelectionSet::vAdd.  I could special-case that function to not select things
	// when copying for clipboard use, but I think this is a little less ugly:
	for (iterClipboard = ClipboardObjects.begin (); iterClipboard != ClipboardObjects.end (); iterClipboard++)
	{
		Selectable *pObject = *iterClipboard;

		if (!pObject)
		{
			Breakpoint ("Application::vPaste: null pointer in clipboard list\n");
			lsPasted.push_front (pObject); // yeah, I know this is null...
			continue;
		}

        while (pObject->fSelected ())
			pObject->vDeselect ();
	}

	// This gets rid of any null pointers that might have found their way into 
	// the list.  I have never seen that happen, but doing this allows me to 
	// cut out a bunch of if(!pointer) stuff in the loops below, without 
	// costing me any sleep at night.
	while (lsPasted.size ())
	{
		// this will be null (it probably never will, I'm just paranoid)
		Selectable *pPasted = lsPasted.front ();

		// this will get rid of the null in the clipboard list
		ClipboardObjects.remove (pPasted);

		lsPasted.pop_front ();
	}

	// first, try to paste into selected objects of the same type 
	for (iterSelected = SelectedObjects.begin (); iterSelected != SelectedObjects.end (); iterSelected++)
	{
		Selectable *pSelected = *iterSelected;

		if (!pSelected)
		{
			Breakpoint ("Application::vPaste: null pointer in selection set\n");
			continue;
		}

		for (iterClipboard = ClipboardObjects.begin (); iterClipboard != ClipboardObjects.end (); iterClipboard++)
		{
			Selectable *pClipboard = *iterClipboard;

			if (typeid (*pSelected) == typeid (*pClipboard))
			{
				if (pSelected->fPaste (pClipboard))					
				{
					lsPasted.push_front (pClipboard);
				}
			}
		}
	}

	// remove pasted-in item from the clipboard list
	while (lsPasted.size ())
	{
		Selectable *pPasted = lsPasted.front ();
		ClipboardObjects.remove (pPasted);
		lsPasted.remove (pPasted);

		// delete the object, since it was only copied, not adopted
		delete pPasted;
	}

	// alter the positions of objects on the clipboard
	for (iterClipboard = ClipboardObjects.begin (); iterClipboard != ClipboardObjects.end (); iterClipboard++)
	{
		Selectable *pObject = *iterClipboard;

		// only one of these will succeed
		Component *pComponent = dynamic_cast <Component*> (pObject);
		Node *pNode = dynamic_cast <Node*> (pObject);

		if (pComponent)
		{
			VectorXYZ vecPosition;
			pComponent->vGetPosition (vecPosition);
			vecPosition.z += 1;
			pComponent->vSetPosition (vecPosition);
		}

		if (pNode)
		{
			pNode->vSetX (pNode->iGetX () + 10);
			pNode->vSetY (pNode->iGetY () + 10);
		}
	}

	// next, try to paste into selected objects of any type
	for (iterSelected = SelectedObjects.begin (); iterSelected != SelectedObjects.end (); iterSelected++)
	{
		Selectable *pSelected = *iterSelected;

		if (!pSelected)
		{
			Breakpoint ("Application::vPaste: null pointer in selection set\n");
			continue;
		}

		for (iterClipboard = ClipboardObjects.begin (); iterClipboard != ClipboardObjects.end (); iterClipboard++)
		{
			Selectable *pClipboard = *iterClipboard;

			if (pSelected->fPaste (pClipboard))					
			{
				lsPasted.push_front (pClipboard);
			}
		}
	}

	while (lsPasted.size ())
	{
		Selectable *pPasted = lsPasted.front ();
		ClipboardObjects.remove (pPasted);
		lsPasted.pop_front ();
	}

	// next, try to paste into parents of selected objects
	for (iterSelected = SelectedObjects.begin (); iterSelected != SelectedObjects.end (); iterSelected++)
	{
		Selectable *pSelected = *iterSelected;

		if (!pSelected)
		{
			Breakpoint ("Application::vPaste: null pointer in selection set\n");
			continue;
		}

		Selectable *pParent = pSelected->pGetParent ();

		if (!pParent)
			continue;

		for (iterClipboard = ClipboardObjects.begin (); iterClipboard != ClipboardObjects.end (); iterClipboard++)
		{
			Selectable *pClipboard = *iterClipboard;

			if (pParent->fPaste (pClipboard))					
			{
				lsPasted.push_front (pClipboard);
			}
		}
	}

	while (lsPasted.size ())
	{
		Selectable *pPasted = lsPasted.front ();
		ClipboardObjects.remove (pPasted);
		lsPasted.pop_front ();
	}

	// next, try to paste into the model itself
	for (iterClipboard = ClipboardObjects.begin (); iterClipboard != ClipboardObjects.end (); iterClipboard++)
	{
		Selectable *pClipboard = *iterClipboard;

		if (m_Model.fPaste (pClipboard))
		{
			lsPasted.push_front (pClipboard);
		}
	}

	while (lsPasted.size ())
	{
		Selectable *pPasted = lsPasted.front ();
		ClipboardObjects.remove (pPasted);
		lsPasted.pop_front ();
	}

	GenericTrace ("Application::vPaste: " << ClipboardObjects.size () << "objects remain unpasted." << std::endl);
}

/****************************************************************************/
/** Delete the objects in the selection set
*****************************************************************************/
void Application::vDelete ()
{
	if (m_eAppMode == amDesign)
		m_SelectionSet.vDelete ();
}


/****************************************************************************/
/** 
*****************************************************************************/
void Application::vSetActiveLibrary (const DynamicsLibraryList::Info* pActive)
{
	// Pass this on to the factory list 
	m_DynamicsLibraries.vSetActiveLibrary (pActive);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Application::vCreateWorld ()
{
	// Get the factory for the current dynamics library
	Dynamics::World *pWorld = m_DynamicsLibraries.pGetActiveWorld ();

	if (!pWorld)
	{
		// This error has already been announced to the user
		// GenericUnexpected ("no dynamics library loaded");
		return;
	}

	// Store the new dynamics world
	m_pWorld = pWorld;

	// Set the model's pointer to the dynamics world 
	m_Model.vSetWorld (m_pWorld);		

	// Create a cage by default
	m_pWorld->vCreateCage ();

	// Recreate the cage or terrain (whether or not its actually enabled (otherwise the logic gets more complex))
	if (m_pTerrain && m_fTerrainAvailable)
	{
		// TODO: set status bar message for this step, it could take a while
		m_pWorld->vCreateTerrain (m_pTerrain);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void Application::vPrepareSimulation ()
{
	VectorXYZ vecOffset (0, 0, 0);
	
	Dynamics::World *pWorld = m_DynamicsLibraries.pGetActiveWorld ();

	if (pWorld != m_pLastWorld)
	{
		// The user switched libraries in mid-simulation, so world re-creation
		// had to wait until now.
		vCreateWorld ();
	}
	
	m_pLastWorld = pWorld;

	// Enable terrain if there is any; enable the cage if not
	if (m_pTerrain && m_fTerrainAvailable && m_fTerrainEnabled)
	{
		m_pWorld->vEnableTerrain ();

		// Offset the model by the terrain's elevation at the 0,0 position
		m_pTerrain->vGetDimensions (vecOffset.x, vecOffset.y);
		vecOffset = vecOffset * 0.5;
		vecOffset.z = m_pTerrain->rGetElevation (vecOffset.x, vecOffset.y);
	}
	else
	{
		m_pWorld->vEnableCage ();
	}

	// Apply any changes that might have occurred
	m_pWorld->vPrepareSimulation ();

	// Have the model create all of the dynamics objects it needs for simulation
	m_Model.vPrepareSimulation (vecOffset, pWorld);

	// Tack a body to the static environment, if appropriate
	if (m_pTackedBody)
	{
		m_pTackedBody->vSetStatic (true);
	}
}

/****************************************************************************/
/** Set the application mode, and handle changing to pose mode
*****************************************************************************/
void Application::vSetAppMode (Juice::AppMode eAppMode)
{
	switch (eAppMode)
	{
	case amDesign:
		m_eAppMode = amDesign;
		m_Model.vDestroySimulation ();
		break;

	case amPose:
		m_eAppMode = amPose;
		m_uPoseTime = 0;

		vPrepareSimulation ();

		break;
	}
}

/****************************************************************************/
/** Returns the current application mode
*****************************************************************************/
Juice::AppMode Application::eGetAppMode ()
{
	return m_eAppMode;
}

/****************************************************************************/
/** Pause the simulation
*****************************************************************************/
void Application::vPause ()
{
	m_fPause = true;
}

/****************************************************************************/
/** Resume the simulation
*****************************************************************************/
void Application::vResume ()
{
	m_fPause = false;
}

/****************************************************************************/
/** Indicate whether or not the simulation is paused
*****************************************************************************/
bool Application::fPaused ()
{
	return m_fPause;
}

/****************************************************************************/
/** Set the 'dragging' flag to indicate whether or not something is being dragged
*****************************************************************************/
void Application::vDragging (bool fDragging)
{
	m_fDragging = fDragging;
}

/****************************************************************************/
/** Indicate whether or not something is being dragged
*****************************************************************************/
bool Application::fDragging ()
{
	return m_fDragging;
}

/****************************************************************************/
/** This is called when the user 'tacks' a body to the static environment
*****************************************************************************/
void Application::vTack (Body *pBody)
{
	// Is something already tacked?
	if (m_pTackedBody)
	{
		// Unregister from the formerly tacked body
		Listener<ComponentEvents>::vUnregister (m_pTackedBody);

		// Note that this body is no longer tacked
		m_pTackedBody->vSetStatic (false);
	}

	// Take note of the body that will now be tacked
	m_pTackedBody = pBody;

	if (m_pTackedBody)
	{
		// Tack the body
		m_pTackedBody->vSetStatic (true);

		// Register for a notification, so we can null out m_pTackedBody if the body is deleted
		Listener<ComponentEvents>::vRegisterForEventsFrom (m_pTackedBody);
	}
}

/****************************************************************************/
/** This will be called when a component is deleted, if the application has
 ** registered for notification from that component.  Currently this is only
 ** used for tacked bodies
*****************************************************************************/
void Application::Notifier_Deleted (Component *pComponent)
{
	// If the deleted body is the one that was tacked...
	if (pComponent == m_pTackedBody)
	{
		// Unregister
		Listener<ComponentEvents>::vUnregister (m_pTackedBody);

		// Note that no body is tacked now.
		m_pTackedBody = null;
	}
}

/****************************************************************************/
/** Force the application's frame rate to a specific value.  This is 
 ** primarily of use when recording movies.
*****************************************************************************/
void Application::vForceFrameRate (Generic::UINT32 uFrameRate)
{
	m_uForcedFrameRate = uFrameRate;
}

// JuiceEngine.cpp ends here ->