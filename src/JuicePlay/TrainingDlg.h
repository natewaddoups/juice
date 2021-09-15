/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: TrainingDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once
#include "afxwin.h"
#include <JuiceEngine.h>
#include <GenePool.h>

class CJuicePlayApp;

// CTrainingDlg dialog

class CTrainingDlg : public CDialog,
	public Generic::Listener <Juice::BehaviorEvents>,
	public Generic::Listener <Juice::ModelEvents>
{
	DECLARE_DYNAMIC(CTrainingDlg)

public:
	CTrainingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrainingDlg();

	void vStartTrial ();
	void vEndTraining ();
	void vRefreshBehaviorList ();
	void vUpdateStatus ();

	void Model_AddBehavior (Juice::Model *pModel, Juice::Behavior *pBehavior);
	void Notifier_Deleted (Juice::Behavior *pDoomedBehavior);
	void Behavior_Changed (Juice::Behavior *pBehavior);

// Dialog Data
	enum { IDD = IDD_TRAINING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	enum Mode { NoTraining, ActiveTraining, ChangingIndividual, ChangingPopulation };
	Mode m_Mode;
	Juice::Behavior *m_pActiveBehavior;
	CGenePool m_GenePool;
	int m_iGeneration;
	int m_iIndividual;

	// Best of all generations
	real m_rBestScore;

	// Total of scores
	real m_rTotalScoreSoFar; // used to compute average
	int m_iBestIndividualThisGeneration;
	real m_rBestScoreThisGeneration;

	CEdit m_Individuals;
	CEdit m_Duration;	
	CButton m_StartStop;
	CListBox m_Behaviors;
	CStatic m_Status;
	CStatic m_BestScore;
	CStatic m_PrevScore;
	CEdit m_Survivors;
	CEdit m_CrossoverPercentage;
	CEdit m_MutationPercentage;
	CEdit m_MutationAmount;

	afx_msg void OnBnClickedGo();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedExit();
	afx_msg void OnBnClickedNew();
	afx_msg void OnBnClickedOpen();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedSaveas();
	afx_msg void OnBnClickedRestore();
};
