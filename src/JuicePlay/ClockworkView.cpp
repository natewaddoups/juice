/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: ClockworkView.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "MainFrm.h"
#include "PropertiesFrame.h"
#include "ClockworkView.h"

using namespace Juice;

const int iBarWidth = 3;
const int iMargin = iBarWidth;

real rContinuous (real rPhase)
{
	if (rPhase > (rPi * 2))
		rPhase -= (rPi * 2);

	return ((rPhase - rPi) / (rPi));
}

ClockworkPane::ClockworkPane (MotionType eMotionType) :
	m_eMotionType (eMotionType),
	m_fnFunction (null),
	m_Motors (null)
{
	switch (m_eMotionType)
	{
	case Juice::mtSine:
		m_fnFunction = Juice::rSine;
		break;

	case Juice::mtDoubleSine:
		m_fnFunction = Juice::rDoubleSine;
		break;

	case Juice::mtStep:
		m_fnFunction = Juice::rStep;
		break;

	case Juice::mtContinuous:
		m_fnFunction = rContinuous;
		break;
	}
}

ClockworkPane::~ClockworkPane ()
{
	m_Motors.empty ();
}

void ClockworkPane::vSetRect (RECT &rc, PanePosition ePosition)
{
	m_rc = rc;
	m_ePosition = ePosition;
}

MotionType ClockworkPane::eGetMotionType ()
{
	return m_eMotionType;
}

void ClockworkPane::vAddMotion (ClockworkMotor *pMotion)
{
	m_Motors.push_front (pMotion);
}

void ClockworkPane::vRemoveMotion (const ClockworkMotor *pMotion)
{
	m_Motors.remove (const_cast <Juice::ClockworkMotor * const &> (pMotion));
}

void ClockworkPane::vResetList ()
{
	m_Motors.clear ();
}

UINT ClockworkPane::uNumberOfMotions ()
{
	return (UINT) m_Motors.size ();
}

void ClockworkPane::vDrawWave (CDC *pDC, real rPhase, real rAmplitude)
{
	if (!m_Motors.size ())
		return;

	int iSegments = 100;

	RECT rc = m_rc;

	switch (m_ePosition)
	{
	case ppSole:
		break;

	case ppTop:
		rc.bottom -= iMargin;
		break;

	case ppMiddle:
		rc.top += iMargin;
		rc.bottom -= iMargin;
		break;

	case ppBottom:
		rc.top += iMargin;
		break;

	default:
		Breakpoint ("ClockworkPane::vDrawWave: invalid position.");
		break;
	}

	//rc.left += iMargin;
	//rc.right -= iMargin;

	int iCenterline = (rc.top + rc.bottom) / 2;
	int iHeight = rc.bottom - rc.top;
	int iWidth = rc.right - rc.left;

	if (m_fnFunction)
	{
		int iLastDeviation = iHeight / 2;

		for (int i = 0; i < iSegments + 1; i++)
		{
			double rAngle = rPhase + ((rPi * 2 * i) / iSegments);
			int iDeviation = (int) ((m_fnFunction (rAngle) * rAmplitude) * ((real) iHeight) / 2.0);

			int x = (iWidth - ((i * iWidth) / (iSegments)));// + m_rc.left;
			int y = iCenterline + iDeviation;

			if ((0 == i) || (abs (iLastDeviation - iDeviation) > (iHeight / 2)))
			{
				pDC->MoveTo (x, y);
			}
			else
			{
				pDC->LineTo (x, y);
			}

			iLastDeviation = iDeviation;
		}
	}
}

void ClockworkPane::vDrawBars (CDC *pDC)
{
	ClockworkMotorList::iterator iter;
	for (iter = m_Motors.begin (); iter != m_Motors.end (); iter++)
	{
		ClockworkMotor *pMotor = *iter;

		real rPhase = ((ClockworkMotor*) pMotor)->rGetPhase ();

		vDrawBar (pDC, rPhase);
	}
}

void ClockworkPane::vDrawToolTip (CDC *pDC, POINT &ptMouse)
{
	if (!fPointInPane (ptMouse))
		return;

	ClockworkMotor *pMotor = pHitMotion (ptMouse.x);
	if (!pMotor)
		return;

	const char *szText = pMotor->szGetName ();
	if (!szText)
		return;

	int iPixPadding = 2;

	int iCenterX = (int) ((ClockworkMotor*) pMotor)->rGetPhase ();
	int iCenterY = (m_rc.top + m_rc.bottom) / 2;

	HFONT hFont = (HFONT) GetStockObject (SYSTEM_FONT);
	HFONT hFontOld = (HFONT) pDC->SelectObject (hFont);

	CSize size = pDC->GetTextExtent (szText, lstrlen (szText));
	int iWidth = size.cx + (iPixPadding * 2);
	int iHeight = size.cy + (iPixPadding * 2);

	int iBleed = 0;
	iBleed = iCenterX - (iWidth / 2);
	if (iBleed < 0)
		iCenterX -= iBleed;

	iBleed = iCenterX + (iWidth / 2);
	if (iBleed > m_rc.right)
		iCenterX -= (iBleed - m_rc.right);

	pDC->Rectangle (iCenterX - (iWidth / 2), iCenterY - (iHeight / 2), iCenterX + (iWidth / 2), iCenterY + (iHeight / 2));

	pDC->SetTextColor (GetSysColor (COLOR_INFOTEXT));
	pDC->SetBkColor (GetSysColor (COLOR_INFOBK));
	pDC->TextOut (iCenterX - (iWidth / 2) + iPixPadding, iCenterY - (iHeight / 2) + iPixPadding, szText);

	pDC->SelectObject (hFontOld);
}

void ClockworkPane::vDrawBorder (CDC *pDC, CPen &pnHighlight, CPen &pnFrame, CPen &pnShadow)
{
	if (!m_Motors.size ())
		return;

	if ((m_ePosition == ppTop) || (m_ePosition == ppMiddle))
	{
		pDC->SelectObject (&pnHighlight);
		pDC->MoveTo (m_rc.left, m_rc.bottom - 1);
		pDC->LineTo (m_rc.right, m_rc.bottom - 1);

		pDC->SelectObject (&pnFrame);
		pDC->MoveTo (m_rc.left, m_rc.bottom);
		pDC->LineTo (m_rc.right, m_rc.bottom);
	}

	if ((m_ePosition == ppBottom) || (m_ePosition == ppMiddle))
	{
		pDC->SelectObject (&pnFrame);
		pDC->MoveTo (m_rc.left, m_rc.top);
		pDC->LineTo (m_rc.right, m_rc.top);

		pDC->SelectObject (&pnShadow);
		pDC->MoveTo (m_rc.left, m_rc.top + 1);
		pDC->LineTo (m_rc.right, m_rc.top + 1);
	}
}

int ClockworkPane::iGetX (real rPhase)
{
	RECT rc = m_rc;

	//rc.left += iMargin;
	rc.right -= iMargin;

	real rX = rPhase;
	rX *= rDegreesToRadians;
	rX /= (rPi * 2.0);
	rX *= rc.right - rc.left;
    rX += ((rc.left + rc.right) / 2);

	if (rX < rc.left)
		rX = rc.left;

	if (rX > rc.right)
		rX = rc.right;
	
	return (int) rX + (iMargin / 2);
}

real ClockworkPane::rGetPhase (int iMouseX)
{
	RECT rc = m_rc;

	rc.left += iMargin;
	rc.right -= iMargin;

	real rPhase = iMouseX;
	rPhase -= ((rc.left + rc.right) / 2);
	rPhase /= rc.right - rc.left;
	rPhase *= (rPi * 2.0);
	rPhase /= rDegreesToRadians;

	if (rPhase < -180)
		rPhase = -180;

	if (rPhase > 180)
		rPhase = 180;

	rPhase = rRound (rPhase, 1.0);

	return rPhase;
}

void ClockworkPane::vDrawBar (CDC *pDC, real rPhase)
{
	rPhase = ((int) rPhase) % 360;

	if (rPhase > 180)
		rPhase -= 360;

	if (rPhase < -180)
		rPhase += 360;

	int x = iGetX (rPhase); 
	pDC->MoveTo (x, m_rc.top);
	pDC->LineTo (x, m_rc.bottom);
}

bool ClockworkPane::fPointInPane (POINT &pt)
{
	if ((pt.y > m_rc.top) && (pt.y < m_rc.bottom))
		return true;

	return false;
}

ClockworkMotor* ClockworkPane::pHitMotion (int iMouseX)
{
	int iWidth = 5;

	ClockworkMotorList::iterator iter;
	for (iter = m_Motors.begin (); iter != m_Motors.end (); iter++)
	{
		ClockworkMotor *pMotor = *iter;
		int iMotionX = iGetX (((ClockworkMotor*) pMotor)->rGetPhase ());

        if ((iMotionX > (iMouseX - iWidth)) && (iMotionX < (iMouseX + iWidth)))
			return pMotor;
	}
	return null;
}

// CClockworkView

IMPLEMENT_DYNCREATE(CClockworkView, CView)

CClockworkView::CClockworkView() :
	m_uX (0), m_uY (0),
	m_PaneSine (mtSine),
	m_PaneDoubleSine (mtDoubleSine),
	m_PaneStep (mtStep),
	m_PaneContinuous (mtContinuous),
	m_pCurrentClockworkPane (0),
	m_pCurrentMotor (0)
{
	m_brBackground.CreateSolidBrush               (WinColorRef (theApp.m_Juice.m_Preferences.m_clrMotionBackground));
	m_pnBackground.CreatePen (PS_SOLID, 1,         WinColorRef (theApp.m_Juice.m_Preferences.m_clrMotionBackground));
	m_pnWave.CreatePen       (PS_SOLID, 1,         WinColorRef (theApp.m_Juice.m_Preferences.m_clrMotionWave));
	m_pnBar.CreatePen        (PS_SOLID, iBarWidth, WinColorRef (theApp.m_Juice.m_Preferences.m_clrMotor));

	m_pnSeparatorTop.CreatePen    (PS_SOLID, 1, GetSysColor (COLOR_3DHIGHLIGHT));
	m_pnSeparator.CreatePen       (PS_SOLID, 1, GetSysColor (COLOR_3DFACE));
	m_pnSeparatorBottom.CreatePen (PS_SOLID, 1, GetSysColor (COLOR_3DSHADOW));

	pnToolTipFrame.CreatePen (PS_SOLID, 1, GetSysColor (COLOR_INFOTEXT));
	brToolTipBackground.CreateSolidBrush (GetSysColor (COLOR_INFOBK));
}

CClockworkView::~CClockworkView()
{
}

void CClockworkView::vAddMotion (ClockworkMotor *p)
{
	ClockworkMotor *pMotion = dynamic_cast <ClockworkMotor*> (p);

	switch (pMotion->iGetMotionType ())
	{
	case Juice::mtSine:
		m_PaneSine.vAddMotion (pMotion);
		break;

	case Juice::mtDoubleSine:
		m_PaneDoubleSine.vAddMotion (pMotion);
		break;

	case Juice::mtStep:
		m_PaneStep.vAddMotion (pMotion);
		break;

	case Juice::mtContinuous:
		m_PaneContinuous.vAddMotion (pMotion);
		break;
	}

	vSetPaneSizes ();

	CDC *pDC = GetDC ();
	vBlank (pDC);
	ReleaseDC (pDC);
}

void CClockworkView::vRemoveMotion (const ClockworkMotor *p)
{
	const ClockworkMotor *pMotion = dynamic_cast <const ClockworkMotor*> (p);

	if (!pMotion)
		return;

	switch (pMotion->eGetMotionType ())
	{
	case Juice::mtSine:
		m_PaneSine.vRemoveMotion (pMotion);
		break;

	case Juice::mtDoubleSine:
		m_PaneDoubleSine.vRemoveMotion (pMotion);
		break;

	case Juice::mtStep:
		m_PaneStep.vRemoveMotion (pMotion);
		break;

	case Juice::mtContinuous:
		m_PaneContinuous.vRemoveMotion (pMotion);
		break;
	}

	vSetPaneSizes ();

	CDC *pDC = GetDC ();
	vBlank (pDC);
	ReleaseDC (pDC);
}

void CClockworkView::vResetMotionList ()
{
	m_PaneSine.vResetList ();
	m_PaneDoubleSine.vResetList ();
	m_PaneStep.vResetList ();
	m_PaneContinuous.vResetList ();

	CDC *pDC = GetDC ();
	vBlank (pDC);
	ReleaseDC (pDC);
}

void CClockworkView::vBlank (CDC *pDC)
{

	CPen *pOldPen = pDC->SelectObject (&m_pnBackground);
	CBrush *pOldBrush = pDC->SelectObject (&m_brBackground);

	pDC->Rectangle (0, 0, m_uX, m_uY);

	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);
}

void CClockworkView::vSetPaneSizes ()
{
	int iPanes = 0;

	if (m_PaneSine.uNumberOfMotions ())       iPanes++;
	if (m_PaneDoubleSine.uNumberOfMotions ()) iPanes++;
	if (m_PaneStep.uNumberOfMotions ())       iPanes++;
	if (m_PaneContinuous.uNumberOfMotions ()) iPanes++;

	if (!iPanes)
		return;

	RECT rc;
	GetClientRect (&rc);

	struct
	{
		RECT rc;
		ClockworkPane::PanePosition pp;
	} PaneData[m_iPanes];

	int iPaneHeight = rc.bottom / iPanes;

	for (int iPane = 0; iPane < iPanes; iPane++)
	{
		RECT rcPane;
		rcPane.left = rc.left;
		rcPane.right = rc.right;
		rcPane.top = rc.top + (iPane * iPaneHeight);
		rcPane.bottom = rcPane.top + iPaneHeight;

		ClockworkPane::PanePosition ePosition = ClockworkPane::ppSole;

		if (iPanes == 1)
		{
			ePosition =  ClockworkPane::ppSole;
		}
		else
		{
			if (iPane == 0)
				ePosition = ClockworkPane::ppTop;
			else if (iPane == iPanes - 1)
				ePosition =  ClockworkPane::ppBottom;
			else 
				ePosition =  ClockworkPane::ppMiddle;
		}

		PaneData[iPane].rc = rcPane;
		PaneData[iPane].pp = ePosition;
	}

	iPane = 0;

	if (m_PaneSine.uNumberOfMotions ())
	{
		m_PaneSine.vSetRect (PaneData[iPane].rc, PaneData[iPane].pp);
		iPane++;
	}

	if (m_PaneDoubleSine.uNumberOfMotions ())
	{
		m_PaneDoubleSine.vSetRect (PaneData[iPane].rc, PaneData[iPane].pp);
		iPane++;
	}

	if (m_PaneStep.uNumberOfMotions ())
	{
		m_PaneStep.vSetRect (PaneData[iPane].rc, PaneData[iPane].pp);
		iPane++;
	}

	if (m_PaneContinuous.uNumberOfMotions ())
	{
		m_PaneContinuous.vSetRect (PaneData[iPane].rc, PaneData[iPane].pp);
		iPane++;
	}
}

ClockworkPane* CClockworkView::pGetPane (POINT &point)
{
	if (m_PaneSine.fPointInPane (point))
		return &m_PaneSine;

	if (m_PaneStep.fPointInPane (point))
		return &m_PaneStep;

	if (m_PaneContinuous.fPointInPane (point))
		return &m_PaneContinuous;

	if (m_PaneDoubleSine.fPointInPane (point))
		return &m_PaneDoubleSine;

	return null;
}

void CClockworkView::vOnTimer ()
{
	CDC *pDC = GetDC ();
	OnDraw (pDC);
	ReleaseDC (pDC);
}

BEGIN_MESSAGE_MAP(CClockworkView, CView)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()
	//ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()


// CClockworkView drawing

bool g_fDoubleBuffer = true;

void CClockworkView::OnDraw(CDC* pDC)
{
	// blank the offscreen canvas
	CDC dcOffscreen;
	dcOffscreen.CreateCompatibleDC (pDC);

	CDC *pDrawDC;

	CBitmap *pOldBitmap = 0;

	if (g_fDoubleBuffer)
	{
		pDrawDC = &dcOffscreen;
		pOldBitmap = pDrawDC->SelectObject (&m_bmp);
	}
	else
	{
		pDrawDC = pDC;
	}

	CPen *pOldPen = pDrawDC->SelectObject (&m_pnBackground);
	CBrush *pOldBrush = pDrawDC->SelectObject (&m_brBackground);

	pDrawDC->Rectangle (0, 0, m_uX, m_uY);

	// draw the waves
	pDrawDC->SelectObject (&m_pnWave);

	real rPhase = theApp.m_Juice.m_Model.rGetPhase ();
	real rAmplitude = 1;

	pDrawDC->SelectObject (&m_pnWave);

	m_PaneSine.vDrawWave       (pDrawDC, rPhase, rAmplitude);
	m_PaneDoubleSine.vDrawWave (pDrawDC, rPhase, rAmplitude);
	m_PaneStep.vDrawWave       (pDrawDC, rPhase, rAmplitude);
	m_PaneContinuous.vDrawWave (pDrawDC, rPhase, rAmplitude);

	pDrawDC->SelectObject (&m_pnBar);

	m_PaneSine.vDrawBars       (pDrawDC);
	m_PaneDoubleSine.vDrawBars (pDrawDC);
	m_PaneStep.vDrawBars       (pDrawDC);
	m_PaneContinuous.vDrawBars (pDrawDC);

	POINT ptMouse;
	GetCursorPos (&ptMouse);
	ScreenToClient (&ptMouse);

	pDrawDC->SelectObject (pnToolTipFrame);
	pDrawDC->SelectObject (brToolTipBackground);

	m_PaneSine.vDrawToolTip       (pDrawDC, ptMouse);
	m_PaneDoubleSine.vDrawToolTip (pDrawDC, ptMouse);
	m_PaneStep.vDrawToolTip       (pDrawDC, ptMouse);
	m_PaneContinuous.vDrawToolTip (pDrawDC, ptMouse);

	m_PaneSine.vDrawBorder       (pDrawDC, m_pnSeparatorTop, m_pnSeparator, m_pnSeparatorBottom);
	m_PaneDoubleSine.vDrawBorder (pDrawDC, m_pnSeparatorTop, m_pnSeparator, m_pnSeparatorBottom);
	m_PaneStep.vDrawBorder       (pDrawDC, m_pnSeparatorTop, m_pnSeparator, m_pnSeparatorBottom);
	m_PaneContinuous.vDrawBorder (pDrawDC, m_pnSeparatorTop, m_pnSeparator, m_pnSeparatorBottom);



	// copy the offscreen buffer to the screen buffer
	if (g_fDoubleBuffer)
	{
		pDC->BitBlt (0, 0, m_uX, m_uY, &dcOffscreen, 0, 0, SRCCOPY);
		pDrawDC ->SelectObject (pOldBitmap);        		
	}

	// clean up
	pDrawDC->SelectObject (pOldPen);
	pDrawDC->SelectObject (pOldBrush);
}

void CClockworkView::vDrawWave (real rPhase, real rAmplitude, fnMotionFunction fnFunction, CDC *pDC, RECT &rc)
{
}

void CClockworkView::vDrawLine (real rPhase, CDC *pDC, RECT &rc)
{
	rPhase = ((int) rPhase) % 360;

	if (rPhase > 180)
		rPhase -= 360;

	if (rPhase < -180)
		rPhase += 360;

	int x = (int) ((real) (rc.right - rc.left) * ((rPhase * rDegreesToRadians) / (rPi * 2)));
	x += (rc.left / 2) + (rc.right / 2);
	pDC->MoveTo (x, rc.top);
	pDC->LineTo (x, rc.bottom);
}

// CClockworkView diagnostics

#ifdef _DEBUG
void CClockworkView::AssertValid() const
{
	CView::AssertValid();
}

void CClockworkView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


// CClockworkView message handlers

void CClockworkView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	vSetPaneSizes ();

	m_uX = cx;
	m_uY = cy;

	m_bmp.DeleteObject ();
	m_bmp.CreateCompatibleBitmap (GetDC (), cx, cy);

	CDC *pDC = GetDC ();
	OnDraw (pDC);
	ReleaseDC (pDC);
}


void CClockworkView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_pCurrentClockworkPane = pGetPane (point);

	if (!m_pCurrentClockworkPane)
		return;

	m_pCurrentMotor = m_pCurrentClockworkPane->pHitMotion (point.x);

	if (!m_pCurrentMotor)
		return;

	CPropertiesFrame *pProperties = ((CMainFrame*)theApp.GetMainWnd ())->m_pProperties;
	if (pProperties)
	{
		pProperties->vShowObject (m_pCurrentMotor);
	}

	SetCapture ();

	CView::OnMButtonDown(nFlags, point);
}

void CClockworkView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_pCurrentClockworkPane && m_pCurrentMotor)
	{
		real rNewPhase = m_pCurrentClockworkPane->rGetPhase (point.x);
		((ClockworkMotor*)m_pCurrentMotor)->vSetPhase (rNewPhase);
	}

	CView::OnMouseMove(nFlags, point);
}

void CClockworkView::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture ();
	m_pCurrentMotor = 0;

	CView::OnMButtonUp(nFlags, point);
}

int CClockworkView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	EnableToolTips(TRUE);

	return 0;
}

// ClockworkView.cpp ends here ->