/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: RenderViewport.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#define VC_EXTRALEAN
#define OEMRESOURCE
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <math.h>
#include <JuiceEngine.h>
#include <GenericError.h>
#include "RenderInternal.h"

#ifdef SupportVideo
#include <MPEGout.h>
#endif

using namespace Juice;
using namespace Juice::Render;

// Frame rate to use while recording video
const int g_iRecordingFrameRate = 24;

/****************************************************************************/
/** Viewport::Viewport
*****************************************************************************/
Viewport::Viewport (Generic::WindowType ParentWindow, Scene *pScene) :
	m_pScene (pScene), m_CameraLookAt (0, 0, 0), m_uContextDepth (0), 
	m_pVideoRecorder (null), m_pFrameBits (null), m_uBPP (0),
	m_hDefaultBMP (null), m_hCaptureBMP (null),
	m_hMemoryDC (null), m_hMemoryRC (null), 
	m_hWindowDC (null), m_hWindowRC (null),
	m_hRenderDC (null), m_hRenderRC (null)
{
	GenericTrace ("Viewport::Viewport" << std::endl);

	// Initialize camera position
	m_CameraXYZ.x = 1;
	m_CameraXYZ.y = -1;
	m_CameraXYZ.z = 1;

	// Initialize camera orientation
	m_CameraYPR.x = Generic::rPi * 1.5f;
	m_CameraYPR.y = Generic::rPi * 0.5f;
	m_CameraYPR.z = 0;

	// Store the parent window handle
	m_ParentWindow = ParentWindow;

	try
	{
#ifdef _WINDOWS

		// Get the size of the parent window
		RECT rcParent;
		::GetClientRect (m_ParentWindow, &rcParent);

		// Create a new window that fills the parent window
		m_Window = CreateWindow (m_szWindowClass, m_szWindowClass, WS_CHILD | WS_VISIBLE, 
			rcParent.left, rcParent.top, rcParent.right, rcParent.bottom,
			m_ParentWindow, NULL, NULL, this);

		// Error test
		if (!m_Window)
			throw_Exception (L"Unable to create 3D window");

		// Create window DC and RC
		m_hWindowDC = GetDC (m_Window);
		if (!m_hWindowDC)
			throw_Exception (L"Unable to get DC for 3D window");
		
		// Set pixel format
		vSetPixelFormat (m_hWindowDC, PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER);

		// Create a rendering context for the window
		m_hWindowRC = wglCreateContext (m_hWindowDC);

		// Error test
		if (!m_hWindowRC)
			throw_Exception (GetLastError ());

//#ifdef SupportVideo

		// Create a bitmap to render to for MPEG captures
		BITMAPINFO bmi;

		// Create a bitmap the same size as the screen (the window is unlikely
		// to get any bigger than that)

		// fixed at 640x480 for testing
		m_uWidth = 640; //GetSystemMetrics (SM_CXSCREEN);
		m_uHeight = 480; //GetSystemMetrics (SM_CYSCREEN);

		// Initialize the bitmapinfo structure
		memset(&bmi, 0, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = m_uWidth;
		bmi.bmiHeader.biHeight = m_uHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 24;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = m_uWidth * m_uHeight * 3;

		// Create the bitmap
		m_hCaptureBMP = CreateDIBSection (m_hWindowDC, &bmi, DIB_RGB_COLORS, (void**) &m_pFrameBits, NULL, 0);

		// Create memory DC and RC
		m_hMemoryDC = CreateCompatibleDC (null);
		if (!m_hMemoryDC)
			throw_Exception (L"Unable to create offscreen DC for 3D window");

		// Select the bitmap
		m_hDefaultBMP = (HBITMAP) SelectObject (m_hMemoryDC, m_hCaptureBMP);

		// Set the pixel format for the bitmap
		vSetPixelFormat (m_hMemoryDC, PFD_SUPPORT_OPENGL | PFD_DRAW_TO_BITMAP);

		// Create a rendering context for the bitmap
		m_hMemoryRC = wglCreateContext (m_hMemoryDC);

		// Error test
		if (!m_hMemoryRC)
			throw_Exception (GetLastError ());

		// Make some adjustments to the memory render context
		m_hRenderDC = m_hMemoryDC;
		m_hRenderRC = m_hMemoryRC;

		// Grab GL resources
		vGetRenderContext ();

		// Load textures for the memory render context
		//m_pScene->fLoadTextures ();

		// perform hidden line/surface removal (enabling Z-Buffer)
		glEnable(GL_DEPTH_TEST);
		vGetError ();

		// set clear Z-Buffer value
		glClearDepth(1.0f);
		vGetError ();

		// Release the GL resources
		vReleaseRenderContext ();
		// Draw to the window by default
		m_hRenderDC = m_hWindowDC;
		m_hRenderRC = m_hWindowRC;

//#endif // SupportVideo
#endif // _WINDOWS

		// Grab GL resources again, make the same adjustments to the on-screen render context
		vGetRenderContext ();

		// perform hidden line/surface removal (enabling Z-Buffer)
		glEnable(GL_DEPTH_TEST);
		vGetError ();

		// set clear Z-Buffer value
		glClearDepth(1.0f);
		vGetError ();

		// Release GL resources
		vReleaseRenderContext ();

#ifdef SupportVideo

		// Create an MPEG recorder object - this stuff is rather unstable, so
		// it's not included in regular release builds.
		m_pVideoRecorder = new CMPEGout (m_uWidth, m_uHeight, 
			m_pFrameBits, PICTYPE_BMP, 
			"untitled.mpg", g_iRecordingFrameRate,
			3,     // compression level
			true); // use decoded previous picture for motion vector search (see MPEGout.h)
#endif
	}
	catch (Exception e)
	{
		// Report exception info to the debug console
		GenericTrace ("Viewport::Viewport exception" << std::endl);
		GenericTrace (e.strGetMessages ().c_str () << std::endl);

		// Release GL resources, just in case
		vReleaseRenderContext ();

		// Throw a new excpetion up the chain
		AddExceptionMessage (e, L"Unable to create 3D View window");
		throw e;
	}    

	// double check context release before exiting
	if (m_uContextDepth)
	{
		Breakpoint ("Render context depth should be zero at this point.  Healing.");
		vReleaseRenderContext ();
	}
}

/****************************************************************************/
/** Viewport::~Viewport
*****************************************************************************/
Viewport::~Viewport ()
{
#ifdef _WINDOWS
	wglDeleteContext (m_hWindowRC);
	wglDeleteContext (m_hMemoryRC);

	ReleaseDC (m_Window, m_hWindowDC);

	SelectObject (m_hMemoryDC, m_hDefaultBMP);
	DeleteDC (m_hMemoryDC);

	DestroyWindow (m_Window);
#endif
	try
	{
#ifdef SupportVideo
		delete m_pVideoRecorder;
#endif
	}
	catch (...)
	{
	}
}

/****************************************************************************/
/** This is the callback for window-systems messages under Win32.  It does
 ** very little other than delegate a few messages to the parent window.
*****************************************************************************/
#ifdef _WINDOWS

LRESULT WINAPI Viewport::WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Viewport *pViewport = (Viewport*) GetWindowLong (hWnd, 0);

	switch (uMsg)
	{
	case WM_CREATE:
		{
			// Get the Viewport pointer from the message, and store it in the window data
			CREATESTRUCT *pCreateStruct = (CREATESTRUCT*) lParam;

			if (!pCreateStruct)
				return -1;
	
			SetWindowLong (hWnd, 0, (LONG) pCreateStruct->lpCreateParams);

			return 0;
		}

	case WM_ERASEBKGND:
		return TRUE;

// This was supposed to fix the focus problem w/ 3D view.  It didn't help.
//	case WM_SETFOCUS:
//		if (pViewport && pViewport->m_ParentWindow)
//			SetFocus (pViewport->m_ParentWindow);
//		return 0;		

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
	case 0x020A: // WM_MOUSEWHEEL
		if (pViewport)
			return SendMessage (pViewport->m_ParentWindow, uMsg, wParam, lParam);		
	}
	return DefWindowProc (hWnd, uMsg, wParam, lParam);

	//return 0;
}

#endif

/****************************************************************************/
/** This might be called in nested fashion, but it only needs to happen once,
*** so it keeps track of the nesting level.
*****************************************************************************/
void Viewport::vGetRenderContext ()
{
	if (!m_uContextDepth)
	{
#ifdef _WINDOWS
		if (!wglMakeCurrent (m_hRenderDC, m_hRenderRC))
		{
			OutputDebugString ("wglMakeCurrent failed.\n");
			vShowLastError ("wglMakeCurrent");
		}
#endif
	}

	m_uContextDepth++;
}

/****************************************************************************/
/** Viewport::vReleaseRenderContext
***
*** The releases the rendering resources when the nesting level reaches zero.
*****************************************************************************/
void Viewport::vReleaseRenderContext ()
{
	if (m_uContextDepth == 0)
	{
		Breakpoint ("Viewport::vReleaseRenderContext: depth is already zero.");
		return;
	}

	m_uContextDepth--;

	if (!m_uContextDepth)
	{
#ifdef _WINDOWS
		if (!wglMakeCurrent (m_hRenderDC, NULL))
		{
			OutputDebugString ("wglMakeCurrent failed.\n");
			vShowLastError ("wglMakeCurrent");
		}
#endif
	}
}


/****************************************************************************/
/** Return the camera position
*****************************************************************************/
void Viewport::vGetCameraPosition (real *prX, real *prY, real *prZ)
{
	if (!prX || !prY || !prZ)
		return;

	*prX = m_CameraXYZ.x;
	*prY = m_CameraXYZ.y;
	*prZ = m_CameraXYZ.z;
}

/****************************************************************************/
/** Set the camera position
*****************************************************************************/
void Viewport::vSetCameraPosition (real rX, real rY, real rZ)
{
	m_CameraXYZ.x = rX;
	m_CameraXYZ.y = rY;
	m_CameraXYZ.z = rZ;
}

/****************************************************************************/
/** Return the camera orientation
*****************************************************************************/
void Viewport::vGetCameraOrientation (real *prYaw, real *prPitch, real *prRoll)
{
	if (!prYaw || !prPitch || !prRoll)
		return;
    
	*prYaw   = m_CameraYPR.y;
	*prPitch = m_CameraYPR.x;
	*prRoll  = m_CameraYPR.z;
}

/****************************************************************************/
/** Get the camera orientation
*****************************************************************************/
void Viewport::vSetCameraOrientation (real rYaw, real rPitch, real rRoll)
{
	m_CameraYPR.y = rYaw;
	m_CameraYPR.x = rPitch;
	m_CameraYPR.z = rRoll;
}

/****************************************************************************/
/** Convert screen coordinates to world coordinates
***
*** from OpenGL programming guide page 148-149(unproject.c)
*** (courtesy of http://user.chollian.net/~kimhs20/tgsqa.html#q26)
*****************************************************************************/
void Viewport::vScreenToWorld (Generic::UINT32 uScreenX, Generic::UINT32 uScreenY, VectorXYZ &vecWorld)
{
	// Viewport parameters
	GLint viewport[4]; 

	// Modelview and projection matrices
	GLdouble mvmatrix[16], projmatrix[16];

	// Screen Y coordinate (in OpenGL's coordinate system)
	GLint realy; 

	// World x, y, z coordinates
	GLdouble wx, wy, wz; 

	// Get the GL context
	vGetRenderContext ();

	// Get the viewport parameters and modelview and projection matrices
	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);

	// Convert the given Y coordinate to GL-style Y coordinate (viewport[3] is height of window in pixels)
	realy = viewport[3] - ((GLint) uScreenY) - 1;

	// Convert to world coordinates
	gluUnProject ((GLdouble) uScreenX, (GLdouble) realy, 0, mvmatrix, projmatrix, viewport, &wx, &wy, &wz); 

	// Store the results in the given vector
	vecWorld.x = wx;
	vecWorld.y = wy;
	vecWorld.z = wz;

	// Release OpenGL resources
	vReleaseRenderContext ();
}

/****************************************************************************/
/** This tells OpenGL to share the display lists between multiple viewports
*****************************************************************************/
#ifdef _WINDOWS
void Viewport::vShareScene (Viewport *pOther)
{
	wglShareLists (pOther->m_hRenderRC, m_hMemoryRC);
	wglShareLists (pOther->m_hRenderRC, m_hWindowRC);
}
#endif

/****************************************************************************/
/** Set the pixel format for the given device context
*****************************************************************************/
#ifdef _WINDOWS
void Viewport::vSetPixelFormat (HDC hDC, int iFlags)
{
	PIXELFORMATDESCRIPTOR pfd;

	// Initialize the pixel format descriptor structure
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1 ;
	pfd.dwFlags    = iFlags;
	pfd.iPixelType = PFD_TYPE_RGBA ;
	pfd.cColorBits = 24 ;
	pfd.cDepthBits = 32 ;
	pfd.cStencilBits = 8 ;
	pfd.iLayerType = PFD_MAIN_PLANE ;

	// get the best available match of pixel format for the device context
	int iPixelFormat = ChoosePixelFormat (hDC,&pfd);
	if (!iPixelFormat)
		throw_Exception (L"Unable to find a good OpenGL pixel format");

	m_uBPP = pfd.cColorBits;

	// set the pixel format of the device context
	if (!SetPixelFormat (hDC,iPixelFormat,&pfd))
	{
		vShowLastError ("SetPixelFormat");
		throw_Exception (L"Unable to set pixel format for OpenGL");
	}
}
#endif

/****************************************************************************/
/** Set the frustum 
*****************************************************************************/
void Viewport::vSetFrustum ()
{
	// Set the viewport dimensions
	glViewport (0, 0, m_uWidth, m_uHeight);

	// Set an indentity projection matrix
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();

	// Get the viewport parameters
	int Viewport[4];
	glGetIntegerv (GL_VIEWPORT, Viewport);

	// Compute the window's aspect ratio (x / y)
	real ratio = (Viewport[2]+0.0) / Viewport[3];

	// Set the perspective matrix
	gluPerspective(45, (float) ratio,0.1,1000);
}

/****************************************************************************/
/** Resize the GL viewport to fit the given dimensions
*****************************************************************************/
void Viewport::vResize (Generic::UINT32 uWidth, Generic::UINT32 uHeight)
{
	// Store the new dimensions
	m_uWidth = uWidth;
	m_uHeight = uHeight;

#ifdef _WINDOWS
	// Move the GL window
	::MoveWindow (m_Window, 0, 0, m_uWidth, m_uHeight, FALSE);
#endif

	// Get the render context
	vGetRenderContext ();

	// Set perspective and such
	vSetFrustum ();

	// Release render context
	vReleaseRenderContext ();

#ifdef _WINDOWS
	// Repaint the window
	::InvalidateRect (m_Window, NULL, TRUE);
#endif
}

/****************************************************************************/
/** This is called from both vRender and vPick.  It draws the model's 
 ** components and the terrain, for both display and picking purposes.
*****************************************************************************/
void Viewport::vRender_Internal ()
{
	glMatrixMode(GL_MODELVIEW);

	// Compute camera orientation
	VectorXYZ vecUp (0, 0, 1);
	vComputeCameraTransform (m_CameraXYZ, m_pScene->vecLookAt (), vecUp);

	glPushMatrix ();

		// add a light 
		glEnable (GL_LIGHTING);

		// Working with light 2...
		glEnable (GL_LIGHT2);

		float  aColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glLightfv (GL_LIGHT2, GL_DIFFUSE, aColor);

		float aAmbient[] = { 1.0, 1.0, 1.0, 1.0 };
		glLightfv (GL_LIGHT2, GL_AMBIENT, aAmbient);

		float  aSpecular[] = {1, 1, 1, 1};
		glLightfv (GL_LIGHT2, GL_SPECULAR, aSpecular);

		glLightf (GL_LIGHT2, GL_SPOT_EXPONENT, 64);
		
		// Compute light position
		VectorXYZ vecLightPosition = m_CameraLookAt + (m_pScene->vecGetLightPosition () * (m_CameraXYZ.rMagnitude () * 2));
		GLfloat aLightPosition[] = 	{ (GLfloat) vecLightPosition.x,  (GLfloat) vecLightPosition.y,  (GLfloat) vecLightPosition.z,  (GLfloat) 1.0 };
		GLfloat aLightDirection[] =	{ (GLfloat) -vecLightPosition.x, (GLfloat) -vecLightPosition.y, (GLfloat) -vecLightPosition.z, (GLfloat) 0 };

		// Set light postion and direction
		glLightfv (GL_LIGHT2, GL_SPOT_DIRECTION, aLightDirection);
		glLightfv (GL_LIGHT2, GL_POSITION, aLightPosition);

	glPopMatrix ();

	// Clear the background (see Scene::vRenderAllViewports in RenderAPI.cpp)
	glCallList (Juice::Render::dlSetup);

	// Draw terrain?
	if (m_pScene->pGetApplication()->fTerrainEnabled () && (m_pScene->pGetApplication()->eGetAppMode() != amDesign))
	{
		// Draw terrain
		m_pScene->pGetApplication()->vTerrainDraw ();
	}
	else
	{
		// Draw axis arrows
		glCallList (Juice::Render::dlSimpleScene);

		if (m_pScene->pGetApplication()->eGetAppMode() == amDesign)
		{
			glCallList (Juice::Render::dlCage);
			glCallList (Juice::Render::dlCageFloor);
		}
		else
		{
			glCallList (Juice::Render::dlGround);
		}
	}

	// Tweak GL state a bit
	glEnable (GL_LIGHTING);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Draw the model 
	glCallList (Juice::Render::dlModel);

	// Display the cursor
	if (m_pScene->m_fShowCursor)
	{
		// The cursor 
		m_pScene->vDrawCursor (m_pScene->m_vecCursor);

		// The cursor's tail
		glDisable (GL_LIGHTING);
		glBegin(GL_LINES);
			glColor3d (1.f, 1.f, 1.f);
			glVertex3d (m_pScene->m_vecCursor.x, m_pScene->m_vecCursor.y, m_pScene->m_vecCursor.z);
			glVertex3d (m_pScene->m_vecCursorTail.x, m_pScene->m_vecCursorTail.y, m_pScene->m_vecCursorTail.z);
		glEnd ();
		glEnable (GL_LIGHTING);
	}

	// Display the selection zone
	if (m_pScene->fSelectionRect ())
		m_pScene->vDrawSelectionRect ();

	// Set camera orientation and position 	
	glLoadIdentity ();
	glMultMatrixd((real*) m_CameraTransform);
	glTranslated(-m_CameraXYZ.x, -m_CameraXYZ.y, -m_CameraXYZ.z);
}

/****************************************************************************/
/** Render the scene
*****************************************************************************/
void Viewport::vRender ()
{
	// Error test (highly unlikely to fail)
	if (!m_pScene)
		return;

	// Get GL resources
	vGetRenderContext ();

	glPushAttrib (GL_ALL_ATTRIB_BITS);
	glRenderMode (GL_RENDER);
	vSetFrustum ();

	// Do the actual rendering (this code is shared with picking)
	vRender_Internal ();

	// Finish up
	glFlush ();
	glFinish();

	// Selection rectangles handle SwapBuffers during the vPick function
	if (!m_pScene->fSelectionRect ())
	{
		SwapBuffers (m_hWindowDC);
	}

	// Recording video?
	if (fRecordingVideo ())
	{
		// Copy the image from the bitmap to the screen
		BitBlt (
			m_hWindowDC, 0, 0, m_uWidth, m_uHeight,
			m_hMemoryDC, 0, 0, SRCCOPY);

		// MPEGout stuff needs to be wrapped in try/catch(...) until it's stabilized
		try
		{
#ifdef SupportVideo
			// Add a frame to the video file
			m_pVideoRecorder->AddFrame ();
#endif
		}
		catch (...)
		{
		}			
	}

	// Clean up
	glPopAttrib ();	
	vReleaseRenderContext ();
}

/****************************************************************************/
/** Viewport::pPick 
***
*** There's code here for both single-object (point) and multi-object (rect)
*** selection, but the multi-object stuff doesn't work right.
*** TODO: fix the multi-object selection rect stuff
***
*** Incoming Y values are assumed to be window coordinates with 0,0 in the
*** upper left corner.
*****************************************************************************/
Component* Viewport::pPick (Generic::UINT32 uX1, Generic::UINT32 uY1, Generic::UINT32 uX2, Generic::UINT32 uY2, 
							PickQuantity ePickQuantity, PickMode ePickMode)
{
	// Get the model pointer
	Model *pModel = m_pScene->m_pModel;

	// Error test
	if (!pModel)
		return null;

	// Get the extents and center of the selection rect
	int iLeft = min (uX1, uX2);
	int iTop = min (uY1, uY2);

	int iWidth = max (abs ((int) (uX1 - uX2)), 1);
	int iHeight = max (abs ((int) (uY1 - uY2)), 1);

	int iCenterX = (uX1 + uX2) / 2;
	int iCenterY = (uY1 + uY2) / 2;

	// Get GL resources
	vGetRenderContext ();

	// Create selection buffer
	const int uBufferSize = 50;
	GLuint pPickBuffer[uBufferSize]; // = new GLuint [uBufferSize];

	// Give selection buffer to GL
	glSelectBuffer (uBufferSize, pPickBuffer); vGetError ();

	// Switch to pick mode
	glRenderMode (GL_SELECT); 

	// This is helpful for testing
	//glRenderMode (GL_RENDER); 
	
	vGetError ();

	// Set pick window and perspective
	vSetFrustum ();

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();

	int Viewport[4];
	glGetIntegerv (GL_VIEWPORT, Viewport);
	vGetError ();

	real ratio = (Viewport[2]+0.0) / Viewport[3];

	gluPerspective(45, (float) ratio,0.1,1000);
	vGetError ();

	// Draw the selection rect
	if (ePickQuantity == pqMultiple)
		if (!fSetSelectionRect (iLeft, iTop, iWidth, iHeight))

	// Re-set the projection matrix, this time using pick box coordinates
	glMatrixMode (GL_PROJECTION);

	glLoadIdentity ();

	gluPickMatrix (iCenterX, m_uHeight - iCenterY, iWidth, iHeight, Viewport);

	gluPerspective(45, (float) ratio,0.1,1000);

	// "Render" the model for picking purposes
	vRender_Internal ();

	glFlush();
	glFinish();

	SwapBuffers (m_hWindowDC);

	// Switch back to render mode
	GLuint uRecords = glRenderMode (GL_RENDER); 
	
	// Examine the pick buffer
	Generic::UINT32 u = 0;
	Generic::UINT32 uNearest = -1;
	Juice::Selectable *pBestPick = null; 

	ComponentList lsPicked;

	// Iterate over GL's pick records
	while ((u < uBufferSize) && (uRecords > 0))
	{
		Generic::UINT32 uItems = pPickBuffer[u++];
		Generic::UINT32 uZMin = pPickBuffer[u++];
		Generic::UINT32 uZMax = pPickBuffer[u++];

		// Iterate over the items in this pick record
		for (Generic::UINT32 uHit = 0; (uHit < uItems) && (u < uBufferSize); uHit++)
		{
			// Get a pointer to the picked component
			Juice::Component *pComponent = (Juice::Component*) pPickBuffer[u++];

			// If null, continue
			if (!pComponent)
				continue;

			// Add this object to the list of picked objects
			lsPicked.push_front (pComponent);

			// If this object is nearest the camera, it's the best pick so far
			if (uZMin < uNearest)
			{
				pBestPick = pComponent;				
				uNearest = uZMin;				
			}
		}

		uRecords--;
	}

	// Picking single objects, or multiple objects?
	if (ePickQuantity == pqSingleNearest)
	{
		// Picking the single nearest object...

		// Iterate over the selection set, see if this object is already in there.
		// If it's in there, remove it.
		SelectableList::const_iterator iterSet;
		const SelectableList &lsSelected = m_pScene->m_pApplication->m_SelectionSet.lsGetContents ();
		bool fFound = false;

		// This must be done in two steps, since removing an object from the selection set
		// while iterating the selection set causes the iterator to go stale.
		for (iterSet = lsSelected.begin (); iterSet != lsSelected.end (); iterSet++)
		{
			if (*iterSet == pBestPick)
			{
				fFound = true;
			}				
		}

		// Toggling selection? (read: is control key down?)
		if (ePickMode == pmToggle)
		{
			// Remove or add the picked object, depending on whether it was already in there
			if (fFound)
				m_pScene->m_pApplication->m_SelectionSet.vRemove (pBestPick);
			else
				m_pScene->m_pApplication->m_SelectionSet.vAdd (pBestPick);
		}
		else
		{
			// Reset the selection set, and add the picked object
			if (!fFound)
			{
				m_pScene->m_pApplication->m_SelectionSet.vReset ();
			}

			m_pScene->m_pApplication->m_SelectionSet.vAdd (pBestPick);
		}
	}
	else
	{
		// In the multi-object case, it's add only.  Otherwise, objects 
		// flicker as the selection rectangle moves around.
		ComponentList::iterator iter;
		for (iter = lsPicked.begin (); iter != lsPicked.end (); iter++)
			m_pScene->m_pApplication->m_SelectionSet.vAdd (*iter);
	}

	vReleaseRenderContext ();
	return dynamic_cast <Component*> (pBestPick);
}

/****************************************************************************/
/** Viewport::fSetSelectionRect
***
*** This begins the multiple-object selection process.
*****************************************************************************/
bool Viewport::fSetSelectionRect (Generic::INT32 uLeft, Generic::INT32 uTop, Generic::INT32 uWidth, Generic::INT32 uHeight)
{
	// Sanity check
	if (!m_pScene)
		return false;

	// Get selection rect coordinates
	VectorXYZ vecLeftTop, vecRightTop, vecLeftBottom, vecRightBottom;

	vScreenToWorld (uLeft, uTop, vecLeftTop);
	vScreenToWorld (uLeft + uWidth, uTop, vecRightTop);
	vScreenToWorld (uLeft, uTop + uHeight, vecLeftBottom);
	vScreenToWorld (uLeft + uWidth, uTop + uHeight, vecRightBottom);

	/*
	char szDebug[200];
	sprintf (szDebug, "LeftTop: %f, RightTop: %f, LeftBottom: %f, RightBottom: %f, Total: %f\n", 
		vecLeftTop.rMagnitude (),
		vecRightTop.rMagnitude (),
		vecLeftBottom.rMagnitude (),
		vecRightBottom.rMagnitude (),
		vecLeftTop.rMagnitude () + vecRightTop.rMagnitude () + vecLeftBottom.rMagnitude () + vecRightBottom.rMagnitude ());
	OutputDebugString (szDebug);
	*/

	// If the rect is negligible, forget it
	if ((vecLeftTop.rMagnitude () + vecRightTop.rMagnitude () + vecLeftBottom.rMagnitude () + vecRightBottom.rMagnitude ()) < 1)
		return false;
	
	// Set the selection rect in the scene
	m_pScene->vSetSelectionRect (m_CameraXYZ, vecLeftTop, vecRightTop, vecLeftBottom, vecRightBottom);

	return true;
}

/****************************************************************************/
/** This ends the multiple-object selection process.
*****************************************************************************/
void Viewport::vCancelSelectionRect ()
{
	if (!m_pScene)
		return;

	m_pScene->vCancelSelectionRect ();
}

/****************************************************************************/
/** This was basically copied from gluLookAt.  I needed to retain the camera 
*** matrix for use when dragging stuff around in the scene.
*****************************************************************************/
void Viewport::vComputeCameraTransform (VectorXYZ vecCameraPosition, VectorXYZ vecLookAt, VectorXYZ vecUp)
{
   GLdouble x[3], y[3], z[3];
   GLdouble mag;

   real rEyeX = vecCameraPosition.x;
   real rEyeY = vecCameraPosition.y;
   real rEyeZ = vecCameraPosition.z;

   real rCenterX = vecLookAt.x;
   real rCenterY = vecLookAt.y;
   real rCenterZ = vecLookAt.z;

   real rUpX = vecUp.x;
   real rUpY = vecUp.y;
   real rUpZ = vecUp.z;

   /* Make rotation matrix */

   /* Z vector */
   z[0] = rEyeX - rCenterX;
   z[1] = rEyeY - rCenterY;
   z[2] = rEyeZ - rCenterZ;
   mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
   if (mag) {			/* mpichler, 19950515 */
      z[0] /= mag;
      z[1] /= mag;
      z[2] /= mag;
   }

   /* Y vector */
   y[0] = rUpX;
   y[1] = rUpY;
   y[2] = rUpZ;

   /* X vector = Y cross Z */
   x[0] = y[1] * z[2] - y[2] * z[1];
   x[1] = -y[0] * z[2] + y[2] * z[0];
   x[2] = y[0] * z[1] - y[1] * z[0];

   /* Recompute Y = Z cross X */
   y[0] = z[1] * x[2] - z[2] * x[1];
   y[1] = -z[0] * x[2] + z[2] * x[0];
   y[2] = z[0] * x[1] - z[1] * x[0];

   /* mpichler, 19950515 */
   /* cross product gives area of parallelogram, which is < 1.0 for
    * non-perpendicular unit-length vectors; so normalize x, y here
    */

   mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
   if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
   }

   mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
   if (mag) {
      y[0] /= mag;
      y[1] /= mag;
      y[2] /= mag;
   }

#define M(row,col)  m_CameraTransform[col*4+row]
   M(0, 0) = x[0];
   M(0, 1) = x[1];
   M(0, 2) = x[2];
   M(0, 3) = 0.0;
   M(1, 0) = y[0];
   M(1, 1) = y[1];
   M(1, 2) = y[2];
   M(1, 3) = 0.0;
   M(2, 0) = z[0];
   M(2, 1) = z[1];
   M(2, 2) = z[2];
   M(2, 3) = 0.0;
   M(3, 0) = 0.0;
   M(3, 1) = 0.0;
   M(3, 2) = 0.0;
   M(3, 3) = 1.0;
#undef M
}

/****************************************************************************/
/** This begins the MPEG recording process.  The process relies heavily on
 ** David Whittaker's MPEGout.dll:
 **
 ** ftp://ftp.wowgod.net/ODE/MPEGout-src.zip
 ** ftp://ftp.wowgod.net/ODE/MPEGout-rel.zip
 **
*****************************************************************************/
bool Viewport::fBeginRecording (const char *szFile)
{
#ifdef SupportVideo
	// Stop any recording that was already in progress
	if (fRecordingVideo ())
		vStopRecording ();

	// Force the frame rate to the MPEG frame rate
	m_pScene->m_pApplication->vForceFrameRate (g_iRecordingFrameRate);

#ifdef _WINDOWS
	// Render to memory for video recording
	m_hRenderDC = m_hMemoryDC;
	m_hRenderRC = m_hMemoryRC;
#endif

	// Set the file name, begin recording
	m_pVideoRecorder->SetFileName (szFile);
	m_pVideoRecorder->Start ();
#endif

	return true;
}

/****************************************************************************/
/** This ends the MPEG recording process.
*****************************************************************************/
void Viewport::vStopRecording ()
{
#ifdef SupportVideo
	// This has to be wrapped with try/catch until the bugs are out
	try { m_pVideoRecorder->Stop (); }
	catch (...) {}

	// Go back to rendering direct to the screen
	m_hRenderDC = m_hWindowDC;
	m_hRenderRC = m_hWindowRC;

	// Restore the fastest possible frame rate
	m_pScene->m_pApplication->vForceFrameRate (0);
#endif
}

/****************************************************************************/
/** Indicates whether or not video recording is in progress
*****************************************************************************/
bool Viewport::fRecordingVideo ()
{
	return (m_hRenderDC == m_hMemoryDC);
		//m_pVideoRecorder->IsRunning ();
}

// RenderViewport.cpp ends here ->