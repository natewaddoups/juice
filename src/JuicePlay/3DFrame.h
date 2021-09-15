/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: 3DFrame.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once
#include <JuiceEngine.h>

class C3DFrame : 
	public CMDIChildWnd
{
	DECLARE_DYNCREATE(C3DFrame)
protected:
	C3DFrame();           // protected constructor used by dynamic creation
	virtual ~C3DFrame();

	DECLARE_MESSAGE_MAP()

	Juice::Render::Viewport *m_pViewport;

	CPoint m_ptLastMousePosition;
	CPoint m_ptLeftButtonDown;

	enum LeftButtonMode
	{
		lbmPick,
		lbmDrag,
		lbmPull,
		lbmSelectBox
	} m_eLeftButtonMode;

	POINT m_ptSelect1, m_ptSelect2;

	VectorXYZ m_vecCameraPosition;
	VectorXYZ m_vecDragPlaneNormal;
	VectorXYZ m_vecDragOrigin;
	VectorXYZ m_vecLastPosition;

	Generic::real m_rAngle;
	Generic::real m_rInclination;
	Generic::real m_rRadius;

	HCURSOR m_hCursor;

	void vSetCameraPosition ();
	void vSnap (VectorXYZ &vecPosition);
	void vSetDragPlane ();

	void vDrag (VectorXYZ &vecCursor);
	void vSetDragStatus (const VectorXYZ &vecCurrent);

	void vZoom (real rDelta);

public:

	void vOnMainTimer ();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnUpdateViewOption(CCmdUI *pCmdUI);
	afx_msg void OnViewTop();
	afx_msg void OnViewFront();
	afx_msg void OnViewSide();
	afx_msg void OnViewCorner();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg void OnOptionsVideoCapture();
	afx_msg void OnUpdateOptionsVideoCapture(CCmdUI *pCmdUI);
};

// 3DFrame.h ends here ->