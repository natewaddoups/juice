/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: GenerateSnakeDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "JuiceDlg.h"
#include "GenerateSnakeDlg.h"

using namespace Juice;

// CGenerateSnakeDlg dialog

IMPLEMENT_DYNAMIC(CGenerateSnakeDlg, CJuiceDlg)
CGenerateSnakeDlg::CGenerateSnakeDlg(CWnd* pParent /*=NULL*/)
	: CJuiceDlg(CGenerateSnakeDlg::IDD, pParent)
	, m_uSegments(10)
	, m_uConcurrentCycles(2)
	, m_iPhaseOffset(90)
	, m_iHorizontal(30)
	, m_iVertical(30)
{
}

CGenerateSnakeDlg::~CGenerateSnakeDlg()
{
}

void CGenerateSnakeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SNAKE_NUMBER_SEGMENTS, m_uSegments);
	DDV_MinMaxUInt(pDX, m_uSegments, 1, 1000);
	DDX_Text(pDX, IDC_SNAKE_NUMBER_CYCLES, m_uConcurrentCycles);
	DDV_MinMaxUInt(pDX, m_uConcurrentCycles, 1, 1000);
	DDX_Text(pDX, IDC_SNAKE_PHASE_DIFFERENCE, m_iPhaseOffset);
	DDV_MinMaxInt(pDX, m_iPhaseOffset, -180, 180);
	DDX_Text(pDX, IDC_SNAKE_HORIZONTAL, m_iHorizontal);
	DDV_MinMaxInt(pDX, m_iHorizontal, -180, 180);
	DDX_Text(pDX, IDC_SNAKE_VERTICAL, m_iVertical);
	DDV_MinMaxInt(pDX, m_iVertical, -180, 180);
}


BEGIN_MESSAGE_MAP(CGenerateSnakeDlg, CJuiceDlg)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CGenerateSnakeDlg message handlers

void CGenerateSnakeDlg::OnBnClickedOk()
{
	if (!m_pModel)
		CDialog::OnOK ();

	UpdateData (true);

	ClockworkBehavior *pBehavior = m_pModel->pCreateClockworkBehavior ();
	pBehavior->vSetName ("Slither");
	m_pModel->vSetStandardSpeed (60);

	RevoluteJoint *pLastHorizontalJoint = null;

	// create all of the segments, from back to front
	for (UINT u = 0; u < m_uSegments; u++)
	{
		VectorXYZ vecDimensions (0.5, 0.25, 0.5);
		real rHorizontalJointDiameter = 0.25; // should compute this as a function of joint range
		real rVerticalJointDiameter = 0.25; // should compute this as a function of joint range

		real rGain = 10;
		real rMaxForce = 250;

		// from back to front...
		real rOffsetBody2 = 0;
		real rOffsetVerticalJoint = rOffsetBody2 + (vecDimensions.y / 2) + (rVerticalJointDiameter / 2);
		real rOffsetBody1 = rOffsetVerticalJoint + (rVerticalJointDiameter / 2) + (vecDimensions.y / 2);
		real rOffsetHorizontalJoint = rOffsetBody1 + (vecDimensions.y / 2) + (rHorizontalJointDiameter / 2);

		real rSegmentLength = (2 * vecDimensions.y) + rHorizontalJointDiameter + rVerticalJointDiameter;
		VectorXYZ vecPosition (0, u * rSegmentLength, vecDimensions.z);

		char szName[100];

		// Create and initialize bodies and joints

		sprintf (szName, "Segment %d Rear", u);
		Body *pBody2 = m_pModel->pCreateBody ();
		pBody2->vSetName (szName);
		pBody2->vSetPosition (vecPosition);
		pBody2->vSetPositionY (pBody2->rGetPositionY () + rOffsetBody2);
		pBody2->vSetDimensions (vecDimensions);

		sprintf (szName, "Segment %d Front", u);
		Body *pBody1 = m_pModel->pCreateBody ();
		pBody1->vSetName (szName);
		pBody1->vSetPosition (vecPosition);
		pBody1->vSetPositionY (pBody1->rGetPositionY () + rOffsetBody1);
		pBody1->vSetDimensions (vecDimensions);

		sprintf (szName, "Segment %d Vertical", u);
		RevoluteJoint *pVerticalJoint = m_pModel->pCreateRevoluteJoint ();
		pVerticalJoint->vSetName (szName);
		pVerticalJoint->vSetPosition (vecPosition);
		pVerticalJoint->vSetPositionY (pVerticalJoint->rGetPositionY () + rOffsetVerticalJoint);
		pVerticalJoint->vSetLength (vecDimensions.z);
		pVerticalJoint->vSetDiameter (rVerticalJointDiameter);
		pVerticalJoint->vSetRotationR (90);
		pVerticalJoint->vSetGain (rGain);
		pVerticalJoint->vSetMaxForce (rMaxForce);

		sprintf (szName, "Segment %d Horizontal", u);
		RevoluteJoint *pHorizontalJoint = m_pModel->pCreateRevoluteJoint ();
		pHorizontalJoint->vSetName (szName);
		pHorizontalJoint->vSetPosition (vecPosition);
		pHorizontalJoint->vSetPositionY (pHorizontalJoint->rGetPositionY () + rOffsetHorizontalJoint);
		pHorizontalJoint->vSetLength (vecDimensions.z);
		pHorizontalJoint->vSetDiameter (rHorizontalJointDiameter);
		pHorizontalJoint->vSetGain (rGain);
		pHorizontalJoint->vSetMaxForce (rMaxForce);

		// Connect bodies
		if (pLastHorizontalJoint)
			pLastHorizontalJoint->vSetBody2 (pBody2);

		pVerticalJoint->vSetBody1 (pBody1);
		pVerticalJoint->vSetBody2 (pBody2);

		pHorizontalJoint->vSetBody1 (pBody1);

		pLastHorizontalJoint = pHorizontalJoint;

		// Set motion parameters
		real rBasePhase = ((((real) u / (real) m_uSegments)) * (real) m_uConcurrentCycles) * 360;

		ClockworkMotor *pVerticalMotion = pBehavior->pGetMotorFor (pVerticalJoint);

		if (pVerticalMotion)
		{
			pVerticalMotion->vSetAmplitude (m_iVertical);
			pVerticalMotion->vSetMotionType (mtSine);
			pVerticalMotion->vSetPhase (rBasePhase);
		}

		ClockworkMotor *pHorizontalMotion = pBehavior->pGetMotorFor (pHorizontalJoint);

		if (pHorizontalMotion)
		{
			pHorizontalMotion->vSetAmplitude (m_iHorizontal);
			pHorizontalMotion->vSetMotionType (mtSine);
			pHorizontalMotion->vSetPhase (rBasePhase + m_iPhaseOffset);
		}
	}

	CDialog::OnOK ();
}

// GenerateSnakeDlg.cpp ends here ->