/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: ClockworkView.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once
#include "MotionView.h"

typedef real (*fnMotionFunction) (real);

class ClockworkPane
{
protected:

	RECT m_rc;
	MotionType m_eMotionType;
	ClockworkMotorList m_Motors;
	fnMotionFunction m_fnFunction;

	void vDrawBar (CDC *pDC, real rPhase);

public:
	ClockworkPane (MotionType eMotionType);
	~ClockworkPane ();

	enum PanePosition
	{
		ppSole,
		ppTop,
		ppMiddle,
		ppBottom
	} m_ePosition;

	void vSetRect (RECT &rc, PanePosition ePosition);

	void vAddMotion (Juice::ClockworkMotor *pMotion);
	void vRemoveMotion (const Juice::ClockworkMotor *pMotion);
	void vResetList ();
	UINT uNumberOfMotions ();

	void vDrawWave (CDC *pDC, real rPhase, real rAmplitude);
	void vDrawBars (CDC *pDC);
	void vDrawBorder (CDC *pDC, CPen &pnHighlight, CPen &pnFrame, CPen &pnShadow);
	void vDrawToolTip (CDC *pDC, POINT &ptMouse);

	bool fPointInPane (POINT &pt);

	ClockworkMotor* pHitMotion (int iMouseX);

	int iGetX (real rPhase);
	real rGetPhase (int iMouseX);

	MotionType eGetMotionType ();
};

// CClockworkView view

class CClockworkView : public CMotionView
{
	DECLARE_DYNCREATE(CClockworkView)

protected:
	CClockworkView();           // protected constructor used by dynamic creation
	virtual ~CClockworkView();	

	ClockworkPane m_PaneSine;
	ClockworkPane m_PaneDoubleSine;
	ClockworkPane m_PaneStep;
	ClockworkPane m_PaneContinuous;

	const static int m_iPanes = 4;

	ClockworkPane *m_pCurrentClockworkPane;
	ClockworkMotor *m_pCurrentMotor;

	void vSetPaneSizes ();

	void vBlank (CDC *pDC);

	void vDrawWave (real rPhase, real rAmplitude, fnMotionFunction fn, CDC *pDC, RECT &rc);
	void vDrawLine (real rPhase, CDC *pDC, RECT &rc);

	ClockworkPane* pGetPane (POINT &pt);

	// drawing objects
	CDC m_OffscreenDC;
	CBrush m_brBackground;
	CPen m_pnBackground, m_pnWave, m_pnBar, m_pnSeparatorTop, m_pnSeparator, m_pnSeparatorBottom;

	// tooltip-drawing objects
	CPen pnToolTipFrame;
	CBrush brToolTipBackground;

	CBitmap m_bmp;
	UINT m_uX, m_uY;

public:
	void vAddMotion (Juice::ClockworkMotor *pMotion);
	void vResetMotionList ();
	void vRemoveMotion (const Juice::ClockworkMotor *pMotion);
	
	void vOnTimer ();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnDraw(CDC* pDC);

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

// ClockworkView.h ends here ->