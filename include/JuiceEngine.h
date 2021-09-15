/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: JuiceEngine.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __JuiceEngine__
#define __JuiceEngine__

#include <list>
#include <GenericTypes.h>
#include <GenericTrace.h>
#include <GenericNotifyListen.h>

#ifdef JUICEENGINE_EXPORTS
#define JuiceExport __declspec (dllexport)
#else
#define JuiceExport
#endif

#include <JuiceTypes.h>
#include <Model.h>
#include <RenderAPI.h>

using namespace std;
using namespace stdext;
using namespace Generic;

/****************************************************************************/
/** Contains data types for the core Juice functionality
*****************************************************************************/
namespace Juice
{

class Application;
class Model;
class Component;
class Body;
class Joint;
class RevoluteJoint;
class PrismaticJoint;
class Terrain;

namespace Render
{
	class Viewport;
};

namespace Dynamics
{
	class World;
};

/// Application modes (design or operate)
enum AppMode
{
	amInvalid = 0,
	amDesign,
	amPose,
	amOperate,
};

/// Application modes (normal or symmetric)
enum EditMode
{
	emInvalid = 0,
	emNormal,
	emSymmetric,
};

/// The plane in which objects will be dragged in the 3D view
enum DragPlane
{
	dpInvalid = 0,
	dpXY, // ground
	dpXZ, // front view
	dpYZ, // side view
};

/// Things that can go wrong while loading terrain
enum TerrainLoadError
{ 
	tleSuccess = 0, 
	tleUnableToSetElevation, 
	tleUnableToSetCoarseTexture, 
	tleUnableToSetDetailedTexture,
	tleUnknown,
};



class SelectionSet;
class Clipboard;

/// This interface declares the events that the selection set publishes
class JuiceExport SelectionSetEvents : public Events<SelectionSet>
{
public:
	virtual void Selection_Add    (SelectionSet *pSet, Selectable *pSelectable, bool fAutomatic) {}
	virtual void Selection_Remove (SelectionSet *pSet, Selectable *pSelectable) {}
};

/****************************************************************************/
/** This keeps track of objects selected in the Juice user interface
*****************************************************************************/
class JuiceExport SelectionSet :
	public Notifier<SelectionSetEvents>,
	public Listener<SelectableEvents>,
	public HasApplication
{
	// The clipboard is allowed to populate the selection set lists directly
	friend class Clipboard;

	SelectableList m_Contents;

	Clipboard *m_pClipboard;

	void Notifier_Deleted(Selectable *pSelectable);

	void vAddAndNotify (Selectable *pSelectable, bool fAutomatic);

	Generic::UINT32 uObjectsOfType (const type_info &type);

public:
	SelectionSet ();
	virtual ~SelectionSet ();

	void vSetClipboard (Clipboard *pClipboard);

	void vReset ();

	void vDelete (bool fDeleteMotorsToo = false);

	void vAdd (Selectable *pSelectable);
	void vRemove (Selectable *pSelectable);

	Generic::UINT32 uComponents ();
	Generic::UINT32 uBehaviors ();
	Generic::UINT32 uMotors ();
	Generic::UINT32 uItems ();

	SelectableList& lsGetContents ();
};

/****************************************************************************/
/** This is the application's clipboard - it's basically a SelectionSet, but
 ** with a slightly different API.
*****************************************************************************/
class JuiceExport Clipboard :
	public HasApplication
{
	friend class SelectionSet;	

	SelectionSet m_Contents;
//	EditMode m_emSelectionType;

	void vDeleteContents ();

public:
	Clipboard ();
	virtual ~Clipboard ();

	/// Copy data from the given selection set to the clipboard
	void vCopy (SelectionSet *pSource);

	/// Indicates whether or not the clipboard contains any data
	bool fHasData ();

	/// Returns the clipboard set
	SelectionSet& ssGetData () { return m_Contents; }

	/// Copy data to/from clipboard and application selection set.
	/// TODO: Make private, with specialized public wrappers - this is a bit 'raw' for a public API
	void vCopy (SelectionSet *pSource, SelectionSet *pDestination);
};

/****************************************************************************/
/** This class keeps track of the application state
*****************************************************************************/
class JuiceExport UserPreferences
{
private:
	// file MRU stuff
	list<string> m_FilesMRU;

public:

	/// Load members from persistent storage
	void vLoad ();

	/// Commit members to persistent storage
	void vSave ();

	// environment
	real m_rCageSize;

	// Appearance data
	// 3D view
	Color m_clr3DBackground;
	Color m_clr3DBackgroundMesh;
	Color m_clr3DBackgroundMeshHighlight;

	Color m_clrNetworkBackground;
	Color m_clrNetworkNode;
	Color m_clrNetworkNodeBorder;
	Color m_clrNetworkNodeText;
	Color m_clrNetworkEdge;

	Color m_clrBeam;
	Color m_clrHinge;
	Color m_clrSlider;
	Color m_clrBallJoint;
	Color m_clrSelected;
	Color m_clrShadow;

	//Color clrSelectionBox;
	//Generic::UINT32 uSelectBoxThickness;
	string m_strGroundTexture;
	string m_strSkyTexture;

	// drag/snap granularity
	bool m_fSnapEnabled;
	real m_rSnapResolution;

	bool m_fFollow;
	bool m_fRenderJoints;

	// Motion source view
	Color m_clrMotionBackground;
	Color m_clrMotionWave;
	Color m_clrMotor;

	// Physics data
	real m_rGravity;
	real m_rGroundFriction;
	real m_rGroundBounce;
	real m_rGlobalERP;
	real m_rGlobalCFM;

	real m_rMinDelta;
	real m_rMaxDelta;

	// Most recent new terrain
	string m_strTerrainElevation;
	string m_strTerrainTextureCoarse;
	string m_strTerrainTextureDetailed;
	real m_rTerrainScaleHorizontal;
	real m_rTerrainScaleVertical;
	real m_rTerrainTriangleCount;

	// application must write through these methods, due to the dual heap thing
	void vSetTerrainElevation (const char *sz);
	void vSetTerrainTextureCoarse (const char *sz);
	void vSetTerrainTextureDetailed (const char *sz);
};

/****************************************************************************/
/** This keeps a list of available dynamics libraries.
*****************************************************************************/
class DynamicsLibraryList : public XML::Serializable
{
	typedef Dynamics::World* (*FactoryProc) (Generic::INT32 iWorldID);

public:
	class Info
	{
		const char *m_szDisplayName;
		const wchar_t *m_szXmlTag;
		const char *m_szModuleName;
		HMODULE m_hDLL;
		FactoryProc m_CreateWorld;
		Dynamics::World *m_pWorld;

	public:
		Info (const char *szDisplayName, const wchar_t *szXmlTag, const char *szModuleName, Generic::INT32 iWorldID, bool fShowError = false);
		~Info ();

		const char *szGetDisplayName () const { return m_szDisplayName; } 
		const wchar_t *szGetXmlTag () const { return m_szXmlTag; }
		Dynamics::World* pGetWorld () const { return m_pWorld; }
	};

	typedef list <Info*> List;

protected:

	List m_Libraries;

	const Info *m_pActiveLibrary;

public:

	DynamicsLibraryList ();

	~DynamicsLibraryList ();

	size_t iGetCount () { return m_Libraries.size (); }

	const List& GetList () { return m_Libraries; }

	void vSetActiveLibrary (const Info* pActive) { m_pActiveLibrary = pActive; }

	Dynamics::World* pGetActiveWorld () { if (m_pActiveLibrary) return m_pActiveLibrary->pGetWorld (); else return null; }

	void vSerialize (XML::Stream &Stream);
};

/****************************************************************************/
/** All the stuff a Juice editor needs, minus the user interface.
***
*** Note that objects are destroyed in the reverse of the order listed.  This
*** matters because the model MUST be destroyed before the world.  Otherwise
*** the model tries to release world resources that have already been 
*** released.
***
*** Note also that they are constructed from top to bottom, and the user 
*** preferences must be loaded before the world is created.
*****************************************************************************/
class JuiceExport Application : 
	public Listener<ComponentEvents>
{
protected:
	// Support for pulling on bodies during simulation
	Component *m_pPullOn;
	VectorXYZ m_vecPull;

	/// This should be set to true while a drag operation is in progress
	bool m_fDragging;

	/// Elapsed time since beginning the pose phase of pose-and-start
	Generic::UINT32 m_uPoseTime;

	/// The body to 'tack' to the world for editing
	Body *m_pTackedBody;

	/// This indicates (and controls) whether the app is in design or operation mode
	Juice::AppMode m_eAppMode;

	/// This will pause the motion sources in operation mode
	bool m_fPause;

	/// This will hold data from a local joystick
	ControlInput m_ControlInput;

	/// Required frame rate; set to zero for fast-as-possible updating
	Generic::UINT32 m_uForcedFrameRate;

	/// Terrain object (a wrapper for Demeter)
	Terrain *m_pTerrain;

	/// Whether or not terrain is currently loaded
	bool m_fTerrainAvailable;

	/// Whether or not to use the currently loaded terrain
	bool m_fTerrainEnabled;

	/// The part of a step that involves physics
	bool fStepPhysics (Generic::UINT32 uMilliseconds);

	/// This keeps track of all available dynamics library factories
	DynamicsLibraryList m_DynamicsLibraries;

	/// Dynamics objects are only re-created when the factory changes; this member is used to determine when to do the re-creation
	const Dynamics::World *m_pLastWorld;

	/// Create the dynamics world
	void vCreateWorld ();

public:
	Application ();
	virtual ~Application ();

	Juice::EditMode m_eEditMode;
	Juice::DragPlane m_eDragPlane;

	Juice::UserPreferences m_Preferences;
	Juice::Dynamics::World *m_pWorld;
	Juice::Model m_Model;
	Juice::SelectionSet m_SelectionSet;
	Juice::Clipboard m_Clipboard;
	Juice::Render::Scene m_Scene;

	/// Change the application mode
	void vSetAppMode (Juice::AppMode eAppMode);

	/// Query the application mode 
	Juice::AppMode eGetAppMode ();

	/// This supports the dialog box for selection dynamics libraries
	const DynamicsLibraryList::List& GetDynamicsLibraryList () { return m_DynamicsLibraries.GetList (); }

	/// When the dynamics world changes, terrain and dynamics objects must be re-created
	void vSetActiveLibrary (const DynamicsLibraryList::Info* pActive);

	/// This enables other objects in the application to create dynamics objects
	Dynamics::World* pGetActiveWorld () { return m_DynamicsLibraries.pGetActiveWorld (); }

	/// Before beginning simulatin, create terrain and initialize the model
	void vPrepareSimulation ();

	/// Pause support
	void vPause ();
	
	/// Pause support
	void vResume ();
	
	/// Pause support
	bool fPaused ();


	/// Clipboard operation
	void vCut ();
	
	/// Clipboard operation
	void vCopy ();
	
	/// Clipboard operation
	void vPaste ();
	
	/// Clipboard operation
	void vDelete ();

	bool fReadFile (const char *szPath);
	bool fWriteFile (const char *szPath);

	/// Initialize the terrain object (must be called while a render context is active)
	void vTerrainInitialize ();

	/// Create new terrain 
	TerrainLoadError eTerrainNew (const char *szElevation, const char *szTexture, const char *szDetailTexture, 
			real rVertexSpacing, real rElevationScale, int iMaxNumTriangles);
	
	/// Open terrain from terrain-descriptor file
	TerrainLoadError eTerrainOpen (const char *szPath);
	
	/// Save current terrain
	void vTerrainSave (const char *szPath);

	/// Is terrain available?
	bool fTerrainAvailable ();
	
	/// Enable terrain
	void vTerrainEnable ();

	/// Disable terrain
	void vTerrainDisable ();
	
	/// Is terrain enabled?
	bool fTerrainEnabled ();

	/// Draw terrain
	void vTerrainDraw ();

	/// Get terrain object
	Terrain* pGetTerrain ();

	/// Animate the world and update the viewports
	bool fStep (Generic::UINT32 uMilliseconds);

	// Support for pulling on bodies during simulation
	void vBeginPull (Component *pComponent);
	void vDoPull ();
	void vEndPull ();

	bool fPulling ();
	void vGetPullVector (VectorXYZ &vecPull);

	/// Get the 'dragging' flag
	bool fDragging ();

	/// Set the 'dragging' flag
	void vDragging (bool fDragging);

	/// 'Tack' a body to the static environment
	void vTack (Body *pBody);
	Body* pTackedBody () { return m_pTackedBody; }
	void Notifier_Deleted (Component *pComponent);

	/// Force a specific frame rate for MPEG-capture purposes
	void vForceFrameRate (Generic::UINT32 uFrameRate);
};

}; // end of Juice namespace

#include "Terrain.h"

#endif

// JuiceEngine.h ends here ->