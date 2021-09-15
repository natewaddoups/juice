/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: 3DFrame.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "3DFrame.h"
#include <math.h>

#include <Model.h>

// this is required because the 3D frame removes itself from the main frame's window list
#include "MainFrm.h"


// C3DFrame

using namespace Generic;

IMPLEMENT_DYNCREATE(C3DFrame, CMDIChildWnd)

C3DFrame::C3DFrame() :
	m_eLeftButtonMode (LeftButtonMode::lbmPick),
	m_hCursor (NULL)
{
	m_rAngle = 3.0f * rPi / 4;
	m_rRadius = 10;
	m_rInclination = 3 * (rPi / 8);

	m_hCursor = ::LoadCursor (NULL, IDC_ARROW);
}

C3DFrame::~C3DFrame()
{
	theApp.m_Juice.m_Scene.vDestroyViewport (m_pViewport);

	// this would cause heap corruption since it was allocated from JuiceEngine.dll's heap
	// delete m_pViewport; 
}

void C3DFrame::vOnMainTimer ()
{
	vSetCameraPosition ();
}

void C3DFrame::vSetCameraPosition ()
{
	// don't move the camera while dragging, it's disconcerting (you end up controlling velocity instead of position)
	// don't move the camera while pulling, it goes nuts (for no good reason - this is a hack to hide a bug)
	if ((LeftButtonMode::lbmDrag == m_eLeftButtonMode) || (LeftButtonMode::lbmPull == m_eLeftButtonMode))
		return;

	if (m_rInclination < 0.1f)
		m_rInclination = 0.1f;

	if (m_rInclination > rPi - 0.1f)
		m_rInclination = rPi - 0.1f;

	m_vecCameraPosition.x = m_rRadius * sin (m_rInclination) * cos (m_rAngle);
	m_vecCameraPosition.y = m_rRadius * sin (m_rInclination) * sin (m_rAngle);
	m_vecCameraPosition.z = m_rRadius * cos (m_rInclination);

	m_vecCameraPosition += theApp.m_Juice.m_Scene.vecLookAt ();

	m_pViewport->vSetCameraPosition (m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z);

//	vSetDragPlane ();
}

void C3DFrame::vZoom (real rDelta)
{
	m_rRadius += m_rRadius * (rDelta / 10);

	if (m_rRadius < 0.25)
		m_rRadius = 0.25;

	vSetCameraPosition ();
}

void C3DFrame::vSetDragPlane ()
{
	VectorXYZ vecDragPlaneTest = m_vecCameraPosition - theApp.m_Juice.m_Scene.vecLookAt ();
	vecDragPlaneTest.x = (vecDragPlaneTest.x > 0) ? vecDragPlaneTest.x : -vecDragPlaneTest.x;
	vecDragPlaneTest.y = (vecDragPlaneTest.y > 0) ? vecDragPlaneTest.y : -vecDragPlaneTest.y;
	vecDragPlaneTest.z = (vecDragPlaneTest.z > 0) ? vecDragPlaneTest.z : -vecDragPlaneTest.z;

	if ((vecDragPlaneTest.x > vecDragPlaneTest.y) && (vecDragPlaneTest.x > vecDragPlaneTest.z))
	{
		theApp.m_Juice.m_eDragPlane = Juice::dpYZ;
	}
	else if ((vecDragPlaneTest.y > vecDragPlaneTest.x) && (vecDragPlaneTest.y > vecDragPlaneTest.z))
	{
		theApp.m_Juice.m_eDragPlane = Juice::dpXZ;
	}
	else if ((vecDragPlaneTest.z > vecDragPlaneTest.x) && (vecDragPlaneTest.z > vecDragPlaneTest.y))
	{
		theApp.m_Juice.m_eDragPlane = Juice::dpXY;
	}
}

void C3DFrame::vSnap (VectorXYZ &vecPosition)
{
	if (!theApp.m_Juice.m_Preferences.m_fSnapEnabled)
		return;

	real rSnapResolution = theApp.m_Juice.m_Preferences.m_rSnapResolution;

	if (theApp.m_Juice.m_eDragPlane != Juice::dpYZ)
	{
		vecPosition.x = rRound (vecPosition.x, rSnapResolution);
	}

	if (theApp.m_Juice.m_eDragPlane != Juice::dpXZ)
	{
		vecPosition.y = rRound (vecPosition.y, rSnapResolution);
	}

	if (theApp.m_Juice.m_eDragPlane != Juice::dpXY)
	{
		vecPosition.z = rRound (vecPosition.z, rSnapResolution);
	}
}

void C3DFrame::vSetDragStatus (const VectorXYZ &vecCurrent)
{
	char szStatus[200];
	VectorXYZ vecDelta = vecCurrent - m_vecDragOrigin;

	sprintf (szStatus, "Start: (%1.3f, %1.3f, %1.3f) Current: (%1.3f, %1.3f, %1.3f) Delta: (%1.3f, %1.3f, %1.3f)",
		m_vecDragOrigin.x, m_vecDragOrigin.y, m_vecDragOrigin.z, 
		vecCurrent.x, vecCurrent.y, vecCurrent.z,
		vecDelta.x, vecDelta.y, vecDelta.z);

    theApp.vSetStatus (szStatus);

}

BEGIN_MESSAGE_MAP(C3DFrame, CMDIChildWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOP, OnUpdateViewOption)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FRONT, OnUpdateViewOption)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SIDE, OnUpdateViewOption)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CORNER, OnUpdateViewOption)
	ON_COMMAND(ID_VIEW_CORNER, OnViewCorner)
	ON_COMMAND(ID_VIEW_FRONT, OnViewFront)
	ON_COMMAND(ID_VIEW_SIDE, OnViewSide)
	ON_COMMAND(ID_VIEW_TOP, OnViewTop)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_CLOSE()
	//ON_WM_MDIACTIVATE()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_OPTIONS_VIDEOCAPTURE, OnOptionsVideoCapture)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_VIDEOCAPTURE, OnUpdateOptionsVideoCapture)
END_MESSAGE_MAP()

// C3DFrame message handlers

int C3DFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create 3D view window
	GenericTrace ("C3DFrame::OnCreate" << std::endl);
	CJuicePlayApp *pApp = (CJuicePlayApp*) AfxGetApp ();
	m_pViewport = pApp->m_Juice.m_Scene.CreateViewport (m_hWnd);
	//m_h3DView = m_pViewport->GetWindow ();

	vSetCameraPosition ();

	SetTimer (0, 20, NULL);

	return 0;
}

void C3DFrame::OnClose()
{
	// Remove self from the main window's view list
	((CMainFrame*)(theApp.m_pMainWnd))->m_3DFrames.remove (this);

	CMDIChildWnd::OnClose();
}

void C3DFrame::OnSize(UINT nType, int cx, int cy)
{
	if (m_pViewport->fRecordingVideo ())
		return;

	CMDIChildWnd::OnSize(nType, cx, cy);
	m_pViewport->vResize ((::UINT32) cx, (::UINT32) cy);
}

BOOL C3DFrame::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	// return CMDIChildWnd::OnEraseBkgnd(pDC);
}

void C3DFrame::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus ();
	SetCapture ();

	Render::Viewport::PickMode ePickMode = Render::Viewport::pmStandard;

	if (nFlags & (MK_CONTROL | MK_SHIFT))
		ePickMode = Render::Viewport::pmToggle;

	Component *pPicked = m_pViewport->pPick (point.x-1, point.y-1, point.x+1, point.y+1,
		Render::Viewport::pqSingleNearest, ePickMode);

	if (pPicked) 
	{		
		m_ptLeftButtonDown = point;
		theApp.m_Juice.vDragging (true);

		// define the drag plane		
		switch (theApp.m_Juice.m_eDragPlane)
		{
		case Juice::dpXY:
			m_vecDragPlaneNormal.x = 0;
			m_vecDragPlaneNormal.y = 0;
			m_vecDragPlaneNormal.z = 1;
			break;
		case Juice::dpXZ:
			m_vecDragPlaneNormal.x = 0;
			m_vecDragPlaneNormal.y = 1;
			m_vecDragPlaneNormal.z = 0;
			break;
		case Juice::dpYZ:
			m_vecDragPlaneNormal.x = 1;
			m_vecDragPlaneNormal.y = 0;
			m_vecDragPlaneNormal.z = 0;
			break;
		}

		pPicked->vGetPosition (m_vecDragOrigin);
		m_vecLastPosition = m_vecDragOrigin;

		// Get model-space coordinates of pick point
		VectorXYZ vecPickPoint;
		m_pViewport->vScreenToWorld (point.x, point.y, vecPickPoint);

		// compute intersection of pick vector and the drag plane
		VectorXYZ vecCameraDirection = theApp.m_Juice.m_Scene.vecLookAt() - m_vecCameraPosition;
		//vecCameraDirection +=  -m_vecCameraPosition;

		VectorXYZ vecPickDirection = m_vecCameraPosition - vecPickPoint;
		VectorXYZ ptIntersection = vecIntersectionOfLineAndPlane (
			m_vecCameraPosition, vecPickDirection, m_vecDragPlaneNormal, m_vecDragOrigin);

		theApp.m_Juice.m_Scene.vSetCursorPosition (ptIntersection);

		if (theApp.m_Juice.eGetAppMode () == amDesign)
		{
			// this pick began the positioning process
			vSetDragStatus (ptIntersection);
			m_eLeftButtonMode = LeftButtonMode::lbmPick;

			theApp.m_Juice.m_Scene.vSetCursorPosition (m_vecDragOrigin); // (ptIntersection);
			theApp.m_Juice.m_Scene.vSetCursorTail (m_vecDragOrigin);
			theApp.m_Juice.m_Scene.vShowCursor (true);			
		}
		else 
		{
			if (typeid (*pPicked) == typeid (Juice::Body))
			{
				// this pick began the pull process
				theApp.m_Juice.vBeginPull (pPicked);
				m_eLeftButtonMode = LeftButtonMode::lbmPull;

				theApp.m_Juice.m_Scene.vSetCursorPosition (ptIntersection);
				theApp.m_Juice.m_Scene.vSetCursorTail (m_vecDragOrigin);
				theApp.m_Juice.m_Scene.vShowCursor (true);			
			}
		}
	}
	else
	{
		// if no object was picked, prepare to do selection-rect picking
		m_eLeftButtonMode = lbmSelectBox;
		m_ptSelect1 = point;
	}

	CMDIChildWnd::OnLButtonDown(nFlags, point);
}

void C3DFrame::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture ();

	CMDIChildWnd::OnLButtonUp(nFlags, point);
	m_eLeftButtonMode = LeftButtonMode::lbmPick;

	m_pViewport->vCancelSelectionRect ();

	theApp.m_Juice.m_Scene.vShowCursor (false);

	// this will end any pull operation that might have been in progress
	theApp.m_Juice.vEndPull ();

	// this will let the rest of the app know that no pull operation is in progress
	theApp.m_Juice.vDragging (false);

	theApp.vSetStatus ("");

	// TODO: create command object, use it to reposition the dragged components
}

void C3DFrame::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus ();
	SetCapture ();
	m_ptLastMousePosition = point;

	CMDIChildWnd::OnRButtonDown(nFlags, point);
}

void C3DFrame::OnRButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture ();

	CMDIChildWnd::OnRButtonUp(nFlags, point);
}

BOOL C3DFrame::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (0)
	{
		// move along camera direction (doesn't work)
		real rYaw, rPitch, rRoll;
		m_pViewport->vGetCameraOrientation (&rYaw, &rPitch, &rRoll);

		real rCos = (real) cos (rYaw * rDegreesToRadians);
		real rSin = (real) sin (rYaw * rDegreesToRadians);

		real rX, rY, rZ;
		m_pViewport->vGetCameraPosition (&rX, &rY, &rZ);

		rX += rCos * 0.001f * ((real) zDelta);
		rY += rSin * 0.001f * ((real) zDelta);

		m_pViewport->vSetCameraPosition (rX, rY, rZ);

		// move along z axis
		m_pViewport->vGetCameraPosition (&rX, &rY, &rZ);

		rZ += 0.01f * ((real) zDelta);

		m_pViewport->vSetCameraPosition (rX, rY, rZ);
	}

	vZoom (0.005f * ((real) zDelta));

	return CMDIChildWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void C3DFrame::OnMouseMove(UINT nFlags, CPoint point)
{
	if (::GetCapture () == m_hWnd)
	{
		if (nFlags & MK_RBUTTON)
		{
			if (0)
			{
				// pitch and yaw the camera
				real rYaw, rPitch, rRoll;
				m_pViewport->vGetCameraOrientation (&rYaw, &rPitch, &rRoll);

				rYaw += (m_ptLastMousePosition.x - point.x) * 0.5f;
				rPitch += (m_ptLastMousePosition.y - point.y) * 0.5f;

				m_pViewport->vSetCameraOrientation (rYaw, rPitch, rRoll);

				// move camera along cartesian x & y directions
				real rX, rY, rZ;
				m_pViewport->vGetCameraPosition (&rX, &rY, &rZ);
				rX += 0.01f * (m_ptLastMousePosition.x - point.x);
				rY -= 0.01f * (m_ptLastMousePosition.y - point.y);
				m_pViewport->vSetCameraPosition (rX, rY, rZ);
			}
			m_rAngle += rDegreesToRadians * ((real) (point.x - m_ptLastMousePosition.x));
			m_rInclination += rDegreesToRadians * ((real) (point.y - m_ptLastMousePosition.y));

			vSetCameraPosition ();

			// do not change drag plane in mid-drag
			if (!(nFlags & MK_LBUTTON))
				vSetDragPlane ();
		}

		RECT rc;
		GetClientRect (&rc);

		if ((nFlags & MK_LBUTTON) && (PtInRect (&rc, point)))
		{
			if (m_eLeftButtonMode == lbmPick)
			{
				int iDragDeadband = 2;
				if ((point.x > m_ptLeftButtonDown.x + iDragDeadband) ||
					(point.x < m_ptLeftButtonDown.x - iDragDeadband) ||
					(point.y > m_ptLeftButtonDown.y + iDragDeadband) ||
					(point.y < m_ptLeftButtonDown.y - iDragDeadband))
				{
					if (theApp.m_Juice.eGetAppMode () == amDesign)
						m_eLeftButtonMode = LeftButtonMode::lbmDrag;
					else
						m_eLeftButtonMode = LeftButtonMode::lbmPull;
				}
			}

			if (m_eLeftButtonMode == lbmSelectBox)
			{

// I'm tired of trying to make this stuff work... maybe in a future version...
#ifdef ThisWillDisableSelectionRectForNow

				m_ptSelect2 = point;
				m_pViewport->pPick (m_ptSelect1.x, m_ptSelect1.y, m_ptSelect2.x, m_ptSelect2.y, 
					Render::Viewport::pqMultiple, Render::Viewport::pmStandard);
#endif

			}
			else if ((m_eLeftButtonMode == lbmDrag) || (m_eLeftButtonMode == lbmPull))
			{
				//if ((theApp.m_Juice.m_eAppMode == amDesign) || theApp.m_Juice.fPulling ())
				{
					// Get model-space coordinates of pick point
					VectorXYZ vecPickPoint;
					m_pViewport->vScreenToWorld (point.x, point.y, vecPickPoint);

					// compute intersection of pick vector and the drag plane
					VectorXYZ vecCameraDirection = theApp.m_Juice.m_Scene.vecLookAt() - m_vecCameraPosition;
					VectorXYZ vecPickDirection = m_vecCameraPosition - vecPickPoint;
					VectorXYZ vecNewPosition = vecIntersectionOfLineAndPlane (
						m_vecCameraPosition, vecPickDirection, m_vecDragPlaneNormal, m_vecDragOrigin);

#ifdef Why_Does_vecNewPosition_GoBallistic_Grrrrr
					char szDebug[1000];
					sprintf (szDebug, "Start: %4.2fx %4.2fy, %4.2fz   Current: %4.2fx %4.2fy, %4.2fz   PickDirection: %4.2fx %4.2fy, %4.2fz CameraPosition:  %4.2fx %4.2fy, %4.2fz   DragPlaneNormal:  %4.2fx %4.2fy, %4.2fz\n",
						m_vecDragOrigin.x, m_vecDragOrigin.y, m_vecDragOrigin.z,
						vecNewPosition.x, vecNewPosition.y, vecNewPosition.z,
						vecPickDirection.x, vecPickDirection.y, vecPickDirection.z,
						m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z,
						//vecPickPoint.x, vecPickPoint.y, vecPickPoint.z);
						//m_vecDragOrigin.x, m_vecDragOrigin.y, m_vecDragOrigin.z);
						m_vecDragPlaneNormal.x, m_vecDragPlaneNormal.y, m_vecDragPlaneNormal.z);
					OutputDebugString (szDebug);
#endif


					// set cursor to new position
					theApp.m_Juice.m_Scene.vSetCursorPosition (vecNewPosition);

					vDrag (vecNewPosition);
				}
			}
		}

		m_ptLastMousePosition = point;
	}

	CMDIChildWnd::OnMouseMove(nFlags, point);
}

void C3DFrame::OnUpdateViewOption(CCmdUI *pCmdUI)
{
	pCmdUI->Enable ();
}

void C3DFrame::OnViewCorner()
{
	m_rInclination = 3.0f * rPi / 8.0f;
	m_rAngle = 3.0f * rPi / 4.0f;
	m_rRadius = 10;

	vSetCameraPosition ();
}

void C3DFrame::OnViewFront()
{
	m_rInclination = (rPi / 2) - (rPi / 10);
	m_rAngle = rPi / 2;
	m_rRadius = 10;

	vSetCameraPosition ();
}

void C3DFrame::OnViewSide()
{
	m_rInclination = (rPi / 2) - (rPi / 10);
	m_rAngle = rPi ;
	m_rRadius = 10;

	vSetCameraPosition ();
}

void C3DFrame::OnViewTop()
{
	m_rInclination = 0.1f;
	m_rAngle = -rPi / 2;
	m_rRadius = 10;

	vSetCameraPosition ();
}

void C3DFrame::vDrag (VectorXYZ &vecNewPosition)
{
	// Is the user dragging?
	if ((m_eLeftButtonMode == LeftButtonMode::lbmDrag) && 
		theApp.m_Juice.m_SelectionSet.uComponents ())
	{
		// apply the snap function
		vSnap (vecNewPosition);

		if (theApp.m_Juice.eGetAppMode () == amDesign)
		{
			// figure out how much to move the selected objects
			VectorXYZ vecDragDelta = vecNewPosition - m_vecLastPosition;

			// Create a component list from the list of selected objects
			SelectableList lsSelection = theApp.m_Juice.m_SelectionSet.lsGetContents ();
			ComponentList lsComponents;
			SelectableList::const_iterator iter, iterLast = lsSelection.end ();
			for (iter = lsSelection.begin (); iter != iterLast; iter++)
			{
				if (Generic::IsKindOf <Component*> (*iter))
					lsComponents.push_front (dynamic_cast <Component*> (*iter));
			}

			// Move those components
			ComponentList::iterator iterComponents;
			for (iterComponents = lsComponents.begin (); iterComponents != lsComponents.end (); iterComponents++)
			{
				Component *pComponent = (*iterComponents);

				VectorXYZ vecPosition;
				pComponent->vGetPosition (vecPosition);
				vecPosition = vecPosition + vecDragDelta;
				pComponent->vSetPosition (vecPosition);
			}

			// store the current position for the next iteration
			m_vecLastPosition = vecNewPosition;

			// Update the status bar
			vSetDragStatus (vecNewPosition);
		}
	}
}
BOOL C3DFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	::SetCursor(m_hCursor);
	return CMDIChildWnd::OnSetCursor(pWnd, nHitTest, message);
}

void C3DFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
/*	case VK_UP: // up arrow
		vZoom (-0.125);
		break;

	case VK_DOWN: // down arrow
		vZoom (0.125);
		break;
*/
	case VK_PRIOR: // page up
		//vZoom (-1);
		m_rRadius *= 1.05;
		break;

	case VK_NEXT: // page down
		//vZoom (1);
		m_rRadius *= 0.95;
		break;

	case VK_LEFT:
		m_rAngle -= 0.1;
		break;

	case VK_RIGHT:
		m_rAngle += 0.1;
		break;

	case VK_UP:
		m_rInclination -= 0.1;
		break;

	case VK_DOWN:
		m_rInclination += 0.1;
		break;
	}

	if (m_rRadius < 1)
		m_rRadius = 1;

	vSetCameraPosition ();

	CMDIChildWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

/****************************************************************************/
/** CS_OWNDC is crucial.  This function came from a sample app at 
*** http://www.codeguru.com/opengl/oglwrapperclass.shtml
*****************************************************************************/
BOOL C3DFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.lpszClass = AfxRegisterWndClass(CS_OWNDC | CS_HREDRAW | CS_VREDRAW);
	return CMDIChildWnd::PreCreateWindow(cs);
}

void C3DFrame::OnOptionsVideoCapture()
{
	CMenu *pMenu = theApp.m_pMainWnd->GetMenu ();

	if (m_pViewport->fRecordingVideo ())
	{
		try
		{
			m_pViewport->vStopRecording ();
			
		}
		catch (...)
		{
		}

		SetWindowLong (m_hWnd, GWL_STYLE, GetWindowLong (m_hWnd, GWL_STYLE) | WS_THICKFRAME);

		if (pMenu)
			pMenu->ModifyMenu (ID_OPTIONS_VIDEOCAPTURE, MF_BYCOMMAND | MF_STRING, ID_OPTIONS_VIDEOCAPTURE, "Begin Video Capture");
	}
	else
	{
		theApp.vPauseAnimation ();

		CFileDialog Dlg (FALSE);
		char szFileName[MAX_PATH];
		szFileName[0] = 0;

		Dlg.m_ofn.lStructSize = sizeof (OPENFILENAME);
		Dlg.m_ofn.hwndOwner = m_hWnd;
		Dlg.m_ofn.lpstrFilter = "MPEG Video\0*.mpg\0All Files\0*.*\0\0";
		Dlg.m_ofn.lpstrDefExt = ".mpg";
		Dlg.m_ofn.nFilterIndex = 1;
		Dlg.m_ofn.lpstrFile = szFileName;
		Dlg.m_ofn.nMaxFile = MAX_PATH;
		Dlg.m_ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER;

		// Display the dialog box 
		if (IDOK == Dlg.DoModal())
		{
			SetWindowPos (NULL, 0, 0, 640, 480, SWP_NOZORDER | SWP_NOMOVE);
			SetWindowLong (m_hWnd, GWL_STYLE, GetWindowLong (m_hWnd, GWL_STYLE) ^ WS_THICKFRAME);

			m_pViewport->fBeginRecording ("c:\\temp\\test.mpg");

			if (pMenu)
				pMenu->ModifyMenu (ID_OPTIONS_VIDEOCAPTURE, MF_BYCOMMAND | MF_STRING, ID_OPTIONS_VIDEOCAPTURE, "End Video Capture");
		}

		theApp.vResumeAnimation ();
	}
}

void C3DFrame::OnUpdateOptionsVideoCapture(CCmdUI *pCmdUI)
{
	// the CMainFrame version will set this to false if no 3DFrame catches it first
	pCmdUI->Enable (TRUE);
}

// 3DFrame.cpp ends here ->