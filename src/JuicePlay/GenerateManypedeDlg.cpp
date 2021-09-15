/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: GenerateManypedeDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "JuiceDlg.h"
#include "GenerateManypedeDlg.h"

using namespace Juice;

// CGenerateManypedeDlg dialog

IMPLEMENT_DYNAMIC(CGenerateManypedeDlg, CJuiceDlg)
CGenerateManypedeDlg::CGenerateManypedeDlg(CWnd* pParent /*=NULL*/)
	: CJuiceDlg(CGenerateManypedeDlg::IDD, pParent)
	, m_uSegments(4)
	, m_iHorizontal(10)
	, m_iVertical(10)
{
}

CGenerateManypedeDlg::~CGenerateManypedeDlg()
{
}

void CGenerateManypedeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MANYPEDE_NUMBER_SEGMENTS, m_uSegments);
	DDV_MinMaxUInt(pDX, m_uSegments, 1, 1000);
	DDX_Text(pDX, IDC_MANYPEDE_HORIZONTAL, m_iHorizontal);
	DDV_MinMaxInt(pDX, m_iHorizontal, -180, 180);
	DDX_Text(pDX, IDC_MANYPEDE_VERTICAL, m_iVertical);
	DDV_MinMaxInt(pDX, m_iVertical, -180, 180);
}


BEGIN_MESSAGE_MAP(CGenerateManypedeDlg, CJuiceDlg)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CGenerateManypedeDlg message handlers

void CGenerateManypedeDlg::OnBnClickedOk()
{

	if (!m_pModel)
		CDialog::OnOK ();

	UpdateData (true);

	ClockworkBehavior *pBehavior = m_pModel->pCreateClockworkBehavior ();
	pBehavior->vSetName ("Walk");

	RevoluteJoint *pLastVerticalJoint = null;

	theApp.m_Juice.m_eEditMode = emSymmetric;

	// create all of the segments, from back to front
	for (UINT u = 0; u < m_uSegments; u++)
	{
		char szName[100];
		real rBodyLength = 1.0;
		real rCoreHeight = 0.5;
		real rOffset = u * (rBodyLength * 2);

		// Create bodies
		sprintf (szName, "Segment %d Core", u);
		Body *pCenter = m_pModel->pCreateBody ();

		VectorXYZ vecPosition (0.0, rOffset, 1.0);
		VectorXYZ vecDimensions (1.0, rBodyLength, 0.5);

		pCenter->vSetName (szName);
		pCenter->vSetPosition (vecPosition);
		pCenter->vSetDimensions (vecDimensions);

		sprintf (szName, "Segment %d Hip LR", u);
		Body *pLeftHip = null, *pRightHip = null;
		if (!m_pModel->fCreateBodyPair (&pLeftHip, &pRightHip))
			break;

		vecPosition = VectorXYZ (0.75, rOffset, 1);
		vecDimensions = VectorXYZ (0.5, 0.5, 0.5);

		pRightHip->vSetName (szName);
		pRightHip->vSetPosition (vecPosition);
		pRightHip->vSetDimensions (vecDimensions);
		vecPosition.x *= -1;
		pLeftHip->vSetPosition (vecPosition);
		pLeftHip->vSetDimensions (vecDimensions);

		sprintf (szName, "Segment %d Thigh LR", u);
		Body *pLeftThigh = null, *pRightThigh = null;
		if (!m_pModel->fCreateBodyPair (&pLeftThigh, &pRightThigh))
			break;

		vecPosition = VectorXYZ (1.5, rOffset, 1);
		vecDimensions = VectorXYZ (1.0, 0.25, 0.25);

		pRightThigh->vSetName (szName);
		pRightThigh->vSetPosition (vecPosition);
		pRightThigh->vSetDimensions (vecDimensions);
		vecPosition.x *= -1;
		pLeftThigh->vSetPosition (vecPosition);
		pLeftThigh->vSetDimensions (vecDimensions);

		sprintf (szName, "Segment %d Shin LR", u);
		Body *pLeftShin = null, *pRightShin = null;
		if (!m_pModel->fCreateBodyPair (&pLeftShin, &pRightShin))
			break;

		vecPosition = VectorXYZ (2, rOffset, 0.5);
		vecDimensions = VectorXYZ (0.25, 0.25, 1.0);

		pRightShin->vSetName (szName);
		pRightShin->vSetPosition (vecPosition);
		pRightShin->vSetDimensions (vecDimensions);
		vecPosition.x *= -1;
		pLeftShin->vSetPosition (vecPosition);
		pLeftShin->vSetDimensions (vecDimensions);

		sprintf (szName, "Segment %d Spacer", u);
		Body *pSpacer = m_pModel->pCreateBody ();

		vecPosition = VectorXYZ (0.0, rOffset + rBodyLength, 1.0);
		vecDimensions = VectorXYZ (0.5, rBodyLength, 0.5);

		pSpacer->vSetName (szName);
		pSpacer->vSetPosition (vecPosition);
		pSpacer->vSetDimensions (vecDimensions);

		// Connect to previous segment
		if (pLastVerticalJoint)
		{
			pLastVerticalJoint->vSetBody2 (pCenter);
		}

		// Create joints
		real rGain = 10;
		real rMaxForce = 200;

		sprintf (szName, "Segment %d Vertical", u);
		vecPosition = VectorXYZ (0.0, rOffset + (rBodyLength * 1.5), 1);
		RevoluteJoint *pVerticalJoint = m_pModel->pCreateRevoluteJoint ();
		pVerticalJoint->vSetName (szName);
		pVerticalJoint->vSetPosition (vecPosition);
		pVerticalJoint->vSetRotationR (90);
		pVerticalJoint->vSetLength (rCoreHeight);
		pVerticalJoint->vSetDiameter (0.25);
		pVerticalJoint->vSetGain (rGain);
		pVerticalJoint->vSetMaxForce (rMaxForce);
		pVerticalJoint->vSetBody1 (pSpacer);

		pLastVerticalJoint = pVerticalJoint;

		sprintf (szName, "Segment %d Horizontal", u);
		vecPosition = VectorXYZ (0.0, rOffset + (rBodyLength * 0.5), 1);
		RevoluteJoint *pHorizontalJoint = m_pModel->pCreateRevoluteJoint ();
		pHorizontalJoint->vSetName (szName);
		pHorizontalJoint->vSetPosition (vecPosition);
		pHorizontalJoint->vSetLength (rCoreHeight);
		pHorizontalJoint->vSetDiameter (0.25);
		pHorizontalJoint->vSetGain (rGain);
		pHorizontalJoint->vSetMaxForce (rMaxForce);
		pHorizontalJoint->vSetBody1 (pCenter);
		pHorizontalJoint->vSetBody2 (pSpacer);

		sprintf (szName, "Segment %d ForeAft LR", u);
		vecPosition = VectorXYZ (0.5, rOffset, 1);

		RevoluteJoint *pForeAftLeft = null, *pForeAftRight = null;
		if (!m_pModel->fCreateRevoluteJointPair (&pForeAftLeft, &pForeAftRight))
			break;

		pForeAftRight->vSetName (szName);
		pForeAftRight->vSetPosition (vecPosition);
		pForeAftRight->vSetLength (rCoreHeight);
		pForeAftRight->vSetDiameter (0.25);
		pForeAftRight->vSetGain (rGain);
		pForeAftRight->vSetMaxForce (rMaxForce);
		pForeAftRight->vSetBody1 (pCenter);
		pForeAftRight->vSetBody2 (pRightHip);

		vecPosition.x *= -1;
		pForeAftLeft->vSetPosition (vecPosition);
		pForeAftLeft->vSetLength (rCoreHeight);
		pForeAftLeft->vSetDiameter (0.25);
		pForeAftLeft->vSetGain (rGain);
		pForeAftLeft->vSetMaxForce (rMaxForce);
		//pForeAftLeft->vSetBody1 (pCenter);
		//pForeAftLeft->vSetBody2 (pLeftHip);

		sprintf (szName, "Segment %d UpDown LR", u);
		vecPosition = VectorXYZ (1.0, rOffset, 1.0);

		RevoluteJoint *pUpDownLeft = null, *pUpDownRight = null;
		if (!m_pModel->fCreateRevoluteJointPair (&pUpDownLeft, &pUpDownRight))
			break;

		pUpDownRight->vSetName (szName);
		pUpDownRight->vSetPosition (vecPosition);
		pUpDownRight->vSetRotationP (-90);
		pUpDownRight->vSetLength (rCoreHeight);
		pUpDownRight->vSetDiameter (0.25);
		pUpDownRight->vSetGain (rGain);
		pUpDownRight->vSetMaxForce (rMaxForce);
		pUpDownRight->vSetBody1 (pRightHip);
		pUpDownRight->vSetBody2 (pRightThigh);

		vecPosition.x *= -1;
		pUpDownLeft->vSetPosition (vecPosition);
		pUpDownLeft->vSetRotationP (90);
		pUpDownLeft->vSetLength (rCoreHeight);
		pUpDownLeft->vSetDiameter (0.25);
		pUpDownLeft->vSetGain (rGain);
		pUpDownLeft->vSetMaxForce (rMaxForce);
		//pUpDownLeft->vSetBody1 (pLeftHip);
		//pUpDownLeft->vSetBody2 (pLeftThigh);

		sprintf (szName, "Segment %d InOut LR", u);
		vecPosition = VectorXYZ (2.0, rOffset, 1.0);

		RevoluteJoint *pInOutLeft = null, *pInOutRight = null;
		if (!m_pModel->fCreateRevoluteJointPair (&pInOutLeft, &pInOutRight))
			break;

		pInOutRight->vSetName (szName);
		pInOutRight->vSetPosition (vecPosition);
		pInOutRight->vSetRotationP (-90);
		pInOutRight->vSetLength (0.25);
		pInOutRight->vSetDiameter (0.25);
		pInOutRight->vSetGain (rGain);
		pInOutRight->vSetMaxForce (rMaxForce);
		pInOutRight->vSetBody1 (pRightThigh);
		pInOutRight->vSetBody2 (pRightShin);

		vecPosition.x *= -1;
		pInOutLeft->vSetPosition (vecPosition);
		pInOutLeft->vSetRotationP (90);
		pInOutLeft->vSetLength (0.25);
		pInOutLeft->vSetDiameter (0.25);
		pInOutLeft->vSetGain (rGain);
		pInOutLeft->vSetMaxForce (rMaxForce);
		//pInOutLeft->vSetBody1 (pLeftHip);
		//pInOutLeft->vSetBody2 (pLeftThigh);

		// Create the 'walk' behavior
		theApp.m_Juice.m_eEditMode = emSymmetric;

		real rBasePhase = 0;
		if (u & 1)
			rBasePhase += 180;
		

		ClockworkMotor *pVerticalMotion = pBehavior->pGetMotorFor (pVerticalJoint);
		pVerticalMotion->vSetAmplitude (0);
		pVerticalMotion->vSetMotionType (mtSine);
		pVerticalMotion->vSetPhase (0);

		ClockworkMotor *pHorizontalMotion = pBehavior->pGetMotorFor (pHorizontalJoint);
		pHorizontalMotion->vSetAmplitude (0);
		pHorizontalMotion->vSetMotionType (mtSine);
		pHorizontalMotion->vSetPhase (0);

		ClockworkMotor *pMotion = pBehavior->pGetMotorFor (pForeAftRight);
		pMotion->vSetAmplitude (m_iHorizontal);
		pMotion->vSetMotionType (mtSine);
		pMotion->vSetPhase (rBasePhase + 0);

		pMotion = pBehavior->pGetMotorFor (pForeAftLeft);
		pMotion->vSetAmplitude (m_iHorizontal);
		pMotion->vSetMotionType (mtSine);
		pMotion->vSetPhase (rBasePhase + 0);

		pMotion = pBehavior->pGetMotorFor (pUpDownRight);
		pMotion->vSetAmplitude (m_iVertical);
		pMotion->vSetMotionType (mtSine);
		pMotion->vSetPhase (rBasePhase + 90);

		pMotion = pBehavior->pGetMotorFor (pUpDownLeft);
		pMotion->vSetAmplitude (m_iVertical);
		pMotion->vSetMotionType (mtSine);
		pMotion->vSetPhase (rBasePhase + 270);

		pMotion = pBehavior->pGetMotorFor (pInOutRight);
		pMotion->vSetAmplitude (0);
		pMotion->vSetMotionType (mtSine);
		pMotion->vSetPhase (0);
		pMotion->vSetStaticOffset (-15);

		pMotion = pBehavior->pGetMotorFor (pInOutLeft);
		pMotion->vSetAmplitude (0);
		pMotion->vSetMotionType (mtSine);
		pMotion->vSetPhase (0);
		pMotion->vSetStaticOffset (-15);
	}

	CDialog::OnOK ();
}

// GenerateManypedeDlg.cpp ends here ->