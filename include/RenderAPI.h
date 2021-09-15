/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: RenderAPI.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __Render__
#define __Render__

#include <GenericTypes.h>
#include <JuiceTypes.h>

/*****************************************************************************
*****************************************************************************/

class CMPEGout;

namespace Juice {

struct tagVectorXYZ;
struct tagVectorYPR;
class Model;

/****************************************************************************/
/** These classes provide Juice with a wrapper around OpenGL
*****************************************************************************/

namespace Render {

class API;
class Viewport;
class Object;
class Box;
class Cylinder;
class Line;
class Texture;

using namespace Generic;

void vSetColor (Color &clr);

/****************************************************************************/
/** The root interface for the 3D rendering software.
*****************************************************************************/
class JuiceExport Scene :
	/// The application pointer allows the scene to draw the background in 
	/// such a way as to indicate the current drag plane
	public HasApplication
{
	friend class Viewport;

protected:
	bool fLoadTextures ();
	
	std::list <Viewport*> m_Viewports;

	void vCreateSharedLists ();

	void vDrawCage ();
	void vDrawCageFloor ();
	void vDrawGround ();
	void vDrawGroundAndSky ();

	/// so it knows what to do draw, and what to pick from
	Juice::Model *m_pModel;

	bool m_fShowCursor;
	VectorXYZ m_vecCursor;
	VectorXYZ m_vecCursorTail;

	/// Position of the light that casts shadows
	VectorXYZ m_vecLightPosition;

	/// Where to aim the camera
	VectorXYZ m_vecLookAt;

	/// Whether or not to show the selection "rect" in the 3D views
	/// (rect in quotes because it's a rect from the user's point of view, but really a pyramid)
	bool m_fSelectionRect;

	/// position of the peak of the selection pyramid
	VectorXYZ m_vecSelectionOrigin;

	/// vectors to the base of the selection pyramid
	VectorXYZ m_vecSelectionBase1, m_vecSelectionBase2, m_vecSelectionBase3, m_vecSelectionBase4;

public:
	Scene ();
	virtual ~Scene ();
	
	static bool fInitialize ();
	void vSetModel (Model *pModel);

	Viewport* CreateViewport (Generic::WindowType ParentWindow);
	void vDestroyViewport (Viewport *pViewport);

	bool fSelectionRect ();
	void vSetSelectionRect (VectorXYZ &vecOrigin, VectorXYZ &vecBase1, VectorXYZ &vecBase2, VectorXYZ &vecBase3, VectorXYZ &vecBase4);
	void vCancelSelectionRect ();
	void vDrawSelectionRect ();

	enum Background
	{
		bgCage,
		bgSky
	} m_eBackground;

	void vSetBackground (Background bg) { m_eBackground = bg; }

	void vRenderAllViewports ();

	/// The following are for benefit of the model components, not the application
	Texture *m_pGroundTexture, *m_pSkyTexture, *m_pWoodTexture;

	/// Set position and orientation from two vectors, taken from component static attributes
	void vPushMatrix (VectorXYZ &vecPosition, VectorYPR &vecOrientation);

	/// Set position and orientation from a single matrix
	void vPushMatrix (const Matrix4x4 &mxRotation);

	/// Transform the current matrix for shadowing
	bool fPushShadowMatrix (VectorXYZ &vecObject);

	/// Draw a box
	void vDrawBox (VectorXYZ vecDimensions);

	/// Draw a cylinder
	void vDrawCylinder (real rLength, real rRadius);

	/// Draw a capsule (capped cylinder)
	void vDrawCapsule (real rLength, real rRadius, real rDomeSize);

	/// Draw a ball
	void vDrawSphere (real rRadius);

	/// Draw a cursor (primarily for internal debugging use)
	void vDrawCursor (VectorXYZ &vecPosition);

	/// Set the cursor location
	void vSetCursorPosition (const VectorXYZ &vecCursor);
	void vGetCursorPosition (VectorXYZ &vecCursor);

	void vSetCursorTail (const VectorXYZ &vecCursorTail);
	//void vGetCursorTail (const VectorXYZ &vecCursorTail);

	/// Turn the cursor on and off
	void vShowCursor (bool fShowCursor);

	/// Determine cursor state
	bool fShowCursor ();

	/// Aim the camera
	void vSetLookAt (VectorXYZ &vecLookAt);

	/// Get the camera look-at point
	VectorXYZ &vecLookAt () { return m_vecLookAt; }

	/// Returns the light position
	const VectorXYZ& vecGetLightPosition () { return m_vecLightPosition; }
};

/****************************************************************************/
/** Interface to 3D rendering windows
*****************************************************************************/
class JuiceExport Viewport
{
	friend class Scene;

#ifdef _WINDOWS
	static TCHAR *m_szWindowClass;
	static LRESULT WINAPI WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HGLRC m_hWindowRC, m_hMemoryRC, m_hRenderRC;
	HDC m_hWindowDC, m_hMemoryDC, m_hRenderDC;


	HBITMAP m_hDefaultBMP, m_hCaptureBMP;
#endif

	Scene *m_pScene;
	CMPEGout *m_pVideoRecorder;

	Generic::UINT8 *m_pFrameBits;
	Generic::UINT32 m_uBPP;

	Generic::WindowType m_Window;
	Generic::WindowType m_ParentWindow;

	Generic::UINT32 m_uWidth;
	Generic::UINT32 m_uHeight;
	VectorXYZ m_CameraXYZ;
	VectorXYZ m_CameraYPR;
	VectorXYZ m_CameraLookAt;

	Matrix4x4 m_CameraTransform;

	void vComputeCameraTransform (VectorXYZ vecCameraPosition, VectorXYZ vecLookAt, VectorXYZ vecUp);
	
#ifdef _WINDOWS
	/// OpenGL voodoo
	void vSetPixelFormat (HDC hDC, int iFlags);
#endif

	/// This is used to enable nested v[Get|Release]RenderContext calls
	Generic::UINT32 m_uContextDepth;

	/// This will be called before any OpenGL rendering
	void vGetRenderContext ();

	/// This will be called after any OpenGL rendering
	void vReleaseRenderContext ();

	/// Causes this viewport to use the same scene (aka display-list) as the given viewport
	void vShareScene (Viewport *pViewport);

	/// sets viewing frustum (used during rendering and picking)
	void vSetFrustum ();

	/// (called by API) private constructor - can only create these thru the 
	/// API, and thus, must delete thru API as well
	Viewport (WindowType ParentWindow, Scene *pScene);

	/// (called by API) private destructor too - see ctor notes
	virtual ~Viewport ();

	/// (called by API) Call this to render the scene in the viewport
	void vRender ();

	/// Wrapper for a call to Scene::SetSelectionRect
	bool fSetSelectionRect (Generic::INT32 uLeft, Generic::INT32 uTop, Generic::INT32 uWidth, Generic::INT32 uHeight);

	void vRender_Internal ();

public:

	/// Begin recording
	bool fBeginRecording (const char *szFile);

	/// Stop recording
	void vStopRecording ();

	/// Indicates whether or not movie-making is in progress
	bool fRecordingVideo ();

	/// Call this to keep the viewport sized up with the parent window
	void vResize (Generic::UINT32 uWidth, Generic::UINT32 uHeight);

	/// Self-explanatory
	void vGetCameraPosition (real *prX, real *prY, real *prZ);
	void vSetCameraPosition (real rX, real rY, real rZ);

	/// Self-explanatory
	void vGetCameraOrientation (real *prYaw, real *prPitch, real *prRoll);
	void vSetCameraOrientation (real rYaw, real rPitch, real rRoll);

	/// In case the application needs anything not supported by this Viewport interface
	WindowType GetWindow () { return m_Window; }

	enum PickQuantity
	{
		pqInavlid = 0,
		pqSingleNearest,
		pqMultiple
	};

	enum PickMode
	{
		pmInvalid = 0,
		pmStandard,
		pmToggle
	};

	Component* pPick (
		Generic::UINT32 uX1, Generic::UINT32 uY1, 
		Generic::UINT32 uX2, Generic::UINT32 uY2, 
		PickQuantity ePickQuantity, PickMode ePickMode);

	void vCancelSelectionRect ();

	void vScreenToWorld (Generic::UINT32 uScreenX, Generic::UINT32 uScreenY, VectorXYZ &vecWorld);
};

}; // end namespace Render

}; // end namespace Juice

#endif
// RenderAPI.h ends here ->