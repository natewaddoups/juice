/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: TrainingDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

******************************************************************************

TODO:

	listen for model-changed notifications, to refresh behavior list automatically

	listen for _Deleted events from the selected behavior

	run simulation for simulated-duration (sum of timesteps) rather than actual duration

	figure out a way to compute the critter's stability (average quaternion magnitude over time)
	
*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "TrainingDlg.h"
#include ".\trainingdlg.h"



// CTrainingDlg dialog

IMPLEMENT_DYNAMIC(CTrainingDlg, CDialog)
CTrainingDlg::CTrainingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTrainingDlg::IDD, pParent)
{
	m_pActiveBehavior = NULL;
	m_Mode = NoTraining;
	m_iGeneration = 0;
	m_iIndividual = 0;
}

CTrainingDlg::~CTrainingDlg()
{
}


void CTrainingDlg::vStartTrial ()
{
    BOOL fTrans = FALSE;
	int iDuration = GetDlgItemInt (IDC_DURATION, &fTrans, FALSE);

	if (!fTrans)
	{
		m_Duration.SetFocus ();
		return;
	}

	// make sure the population size doesn't change
	// (could support this, but would complicate the GA class a bit... maybe later)
	GetDlgItem (IDC_POPULATION_SIZE)->EnableWindow (FALSE);

	// Get genes for this behavior
	vector<real> &Genes = m_GenePool.GetIndividual (m_iIndividual);
	m_pActiveBehavior->vSetVector (&Genes);

	// Begin the trial
	theApp.m_Juice.vSetAppMode (Juice::amPose);
	SetTimer (0, iDuration * 1000, NULL);
	m_Mode = ActiveTraining;
	m_StartStop.SetWindowText ("S&top");

	vUpdateStatus ();
}

void CTrainingDlg::vEndTraining ()
{
	KillTimer (0);
	m_Mode = NoTraining;
	theApp.m_Juice.vSetAppMode (Juice::amDesign);
	m_StartStop.SetWindowText ("&Train");
	GetDlgItem (IDC_POPULATION_SIZE)->EnableWindow (TRUE);
}

void CTrainingDlg::vRefreshBehaviorList ()
{
	m_Behaviors.ResetContent ();
	int iItem = m_Behaviors.AddString ("(Please select a behavior)");
	m_Behaviors.SetItemData (iItem, NULL);

	const Juice::BehaviorList& Behaviors = theApp.m_Juice.m_Model.lsGetBehaviors ();
	Juice::BehaviorList::const_iterator iter, iterLast = Behaviors.end ();
	for (iter = Behaviors.begin (); iter != iterLast; iter++)
	{
		Juice::Behavior *pBehavior = *iter;
		if (typeid (*pBehavior) == typeid (Juice::ClockworkBehavior))
		{
			iItem = m_Behaviors.AddString (pBehavior->szGetName ());
			m_Behaviors.SetItemData (iItem, (DWORD_PTR) pBehavior);
		}
	}

	m_Behaviors.SetCurSel (0);
}


void CTrainingDlg::vUpdateStatus ()
{
	char szStatus[200];
	sprintf (szStatus, "Individual %d of generation %d", m_iIndividual+1, m_iGeneration+1);
	m_Status.SetWindowText (szStatus);
}

void CTrainingDlg::Model_AddBehavior (Juice::Model *pModel, Juice::Behavior *pBehavior)
{
	int iNew = m_Behaviors.AddString (pBehavior->szGetName ());
	m_Behaviors.SetItemData (iNew, (DWORD_PTR) pBehavior);
	Listener<Juice::BehaviorEvents>::vRegisterForEventsFrom (pBehavior);
}

void CTrainingDlg::Notifier_Deleted (Juice::Behavior *pDoomedBehavior)
{
	if (pDoomedBehavior == m_pActiveBehavior)
		m_pActiveBehavior = NULL;

	int iItems = m_Behaviors.GetCount ();
	int i = 0;
	for (int i = 0; i < iItems; i++)
	{
		if (m_Behaviors.GetItemData (i) == (int) pDoomedBehavior)
		{
			m_Behaviors.DeleteString (i);
			break;
		}
	}
}

void CTrainingDlg::Behavior_Changed (Juice::Behavior *pBehavior)
{
	int iItems = m_Behaviors.GetCount ();
	int i = 0;
	for (int i = 0; i < iItems; i++)
	{
		int iData = m_Behaviors.GetItemData (i);
		int iBehavior = (int) pBehavior;

		const char *szNewName = pBehavior->szGetName ();

		if (iData == iBehavior)
		{
			BOOL fSelected = m_Behaviors.GetCurSel () == i;
			m_Behaviors.DeleteString (i);
			int iNew = m_Behaviors.AddString (pBehavior->szGetName ());
			m_Behaviors.SetItemData (iNew, (DWORD_PTR) pBehavior);
			if (fSelected)
				m_Behaviors.SetCurSel (iNew);
			break;
		}
	}
}



void CTrainingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_POPULATION_SIZE, m_Individuals);
	DDX_Control(pDX, IDC_DURATION, m_Duration);
	DDX_Control(pDX, IDC_GO, m_StartStop);
	DDX_Control(pDX, IDC_BEHAVIOR, m_Behaviors);
	DDX_Control(pDX, IDC_STATUS, m_Status);
	DDX_Control(pDX, IDC_BESTSCORE, m_BestScore);
	DDX_Control(pDX, IDC_PREVSCORE, m_PrevScore);
	DDX_Control(pDX, IDC_SURVIVORS, m_Survivors);
	DDX_Control(pDX, IDC_CROSSOVER, m_CrossoverPercentage);
	DDX_Control(pDX, IDC_MUTATIONPERCENTAGE, m_MutationPercentage);
	DDX_Control(pDX, IDC_MUTATIONAMOUNT, m_MutationAmount);
}


BEGIN_MESSAGE_MAP(CTrainingDlg, CDialog)
	ON_BN_CLICKED(IDC_GO, OnBnClickedGo)
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_EXIT, OnBnClickedExit)
	ON_BN_CLICKED(IDC_NEW, OnBnClickedNew)
	ON_BN_CLICKED(IDC_RESTORE, OnBnClickedRestore)
END_MESSAGE_MAP()



BOOL CTrainingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Duration.SetWindowText ("15");
	m_Survivors.SetWindowText ("5");
	m_Individuals.SetWindowText ("25");
	m_CrossoverPercentage.SetWindowText ("10");
	m_MutationPercentage.SetWindowText ("10");
	m_MutationAmount.SetWindowText ("10");

	Listener<Juice::ModelEvents>::vRegisterForEventsFrom (&(theApp.m_Juice.m_Model));

	if (!m_pActiveBehavior)
		m_Status.SetWindowText ("Please select a behavior");

	((CButton*) GetDlgItem(IDC_INIT_EXISTING))->SetCheck (BST_CHECKED);

	return TRUE;
}

void CTrainingDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
	}
	else
	{
		vEndTraining ();
	}
}

int Decimal (real r)
{
	return (int) ((r * 100) - (((int) r) * 100));
}

void CTrainingDlg::OnTimer (UINT_PTR nIDEvent)
{
	// Compute this individual's score (position only, for now)
	real rScore = 0; 
	VectorXYZ vecPosition;
	theApp.m_Juice.m_Model.vGetCenter (vecPosition);
	vecPosition.z = 0;
	rScore = vecPosition.rMagnitude ();
	m_rTotalScoreSoFar += rScore;

	// Stop this individual's trial
	m_Mode = ChangingIndividual;
	KillTimer (0);
	theApp.m_Juice.vSetAppMode (Juice::amDesign);

	if (rScore > m_rBestScoreThisGeneration)
	{
		m_rBestScoreThisGeneration = rScore;
		m_iBestIndividualThisGeneration = m_iIndividual;
	}

	// Show the score for the user
	char szScore[200];
	wsprintf (szScore, "Previous score: %d.%d\r\nThis generation best score: %d.%d", 
			(int) rScore, 
			(int) Decimal (rScore),
			(int) m_rBestScoreThisGeneration,
			(int) Decimal (m_rBestScoreThisGeneration));

	m_PrevScore.SetWindowText (szScore);

	// Record the score for the GA
	m_GenePool.vSetScore (m_iIndividual, rScore);

	// Note best score overall
	if (rScore > m_rBestScore)
		m_rBestScore = rScore;

	// Time to regenerate the pool?
	m_iIndividual++;
	int iPopulationSize = m_GenePool.iPopulationSize ();
	if (m_iIndividual == iPopulationSize)
	{
		m_rTotalScoreSoFar /= iPopulationSize;

		char szScore[200];
		wsprintf (szScore, "Previous generation \r\nBest score: %d.%d \r\nAverage: %d.%d", 
			(int) m_rBestScore, 
			Decimal (m_rBestScore),			
			(int) m_rTotalScoreSoFar,
			Decimal (m_rTotalScoreSoFar));

		m_BestScore.SetWindowText (szScore);

		// Update regeneration parameters
		BOOL fTranslated = FALSE;
		int i = 0;

		i = GetDlgItemInt (IDC_SURVIVORS, &fTranslated, FALSE);
		if (fTranslated)
			m_GenePool.pGetParameters()->iSurvivors = i;

		i = GetDlgItemInt (IDC_CROSSOVER, &fTranslated, FALSE);
		if (fTranslated)
			m_GenePool.pGetParameters()->iCrossoverPercentage = i;

		i = GetDlgItemInt (IDC_MUTATIONPERCENTAGE, &fTranslated, FALSE);
		if (fTranslated)
			m_GenePool.pGetParameters()->iMutationPercentage = i;

		i = GetDlgItemInt (IDC_MUTATIONAMOUNT, &fTranslated, FALSE);
		if (fTranslated)
			m_GenePool.pGetParameters()->iMutationSize = i;

		// Regenerate and restart
		m_GenePool.vRegenerate ();
		m_iIndividual = 0;
		m_rBestScore = 0;
		m_rTotalScoreSoFar = 0;
		m_iGeneration++;
	}

	// Start the new individual's trial
	vStartTrial ();

	// Update status
	vUpdateStatus ();

	// MFC blah blah blah
	CDialog::OnTimer(nIDEvent);
}

void CTrainingDlg::OnBnClickedExit()
{
	if (m_Mode != NoTraining)
		OnBnClickedGo ();

	ShowWindow (SW_HIDE);
}

void CTrainingDlg::OnBnClickedNew()
{
	// Get the behavior to train
	int iCurSel = m_Behaviors.GetCurSel ();
	if (iCurSel == LB_ERR)
	{
		m_Status.SetWindowText ("Please select a behavior");
		return;
	}

	m_pActiveBehavior = (Juice::Behavior*) m_Behaviors.GetItemData (iCurSel);

	if (!m_pActiveBehavior)
	{
		m_Status.SetWindowText ("Internal error.  Ooops.");
		return;
	}

	// This only works if there's a behavior to base the gene upon
	if (m_pActiveBehavior)
	{
		// Get the population size
		BOOL fTranslated = false;
		int iPopulation = GetDlgItemInt (IDC_POPULATION_SIZE, &fTranslated, FALSE);
		if (!fTranslated)
		{
			GetDlgItem (IDC_POPULATION_SIZE)->SetFocus ();
			m_Status.SetWindowText ("Unable to read population size");
			return;
		}

		// Get the gene-vector size
		vector<real> *pTmp = m_pActiveBehavior->pCreateVector();
		int iVectorSize = pTmp->size ();
		
		real r0 = (*pTmp)[0];
		real r1 = (*pTmp)[1];
		real r2 = (*pTmp)[2];
		real r3 = (*pTmp)[3];

		// Null the vector pointer if the user wants a random new behavior
		if (((CButton*) GetDlgItem(IDC_INIT_EXISTING))->GetCheck () == BST_UNCHECKED)
		{
			delete pTmp;
			pTmp = NULL;
		}

		// Initialize the pool, perhaps using the base vector
		m_GenePool.Initialize (iPopulation, iVectorSize, pTmp);

		delete pTmp;

		// Update the UI
		GetDlgItem (IDC_GO)->EnableWindow (TRUE);
		m_Status.SetWindowText ("Population initialized");
	}
	else
	{
		m_Status.SetWindowText ("Internal error - no behavior selected");
	}
}

void CTrainingDlg::OnBnClickedOpen()
{
}

void CTrainingDlg::OnBnClickedSave()
{
}

void CTrainingDlg::OnBnClickedSaveas()
{
}

void CTrainingDlg::OnBnClickedGo()
{
	BOOL fTrain = m_Mode != NoTraining;

	GetDlgItem (IDC_BEHAVIOR)->EnableWindow (fTrain);
	GetDlgItem (IDC_POPULATION_SIZE)->EnableWindow (fTrain);
	GetDlgItem (IDC_NEW)->EnableWindow (fTrain);
	GetDlgItem (IDC_INIT_EXISTING)->EnableWindow (fTrain);
	GetDlgItem (IDC_INIT_RANDOM)->EnableWindow (fTrain);
	GetDlgItem (IDC_RESTORE)->EnableWindow (fTrain);

	switch (m_Mode)
	{
	case NoTraining:
		m_rBestScore = 0;
		m_iIndividual = 0;
		m_iGeneration = 0;
		m_rBestScoreThisGeneration = 0;
		m_iBestIndividualThisGeneration = 0;

		vStartTrial ();
		break;

	default:
		vEndTraining ();
		break;
	}
}


void CTrainingDlg::OnBnClickedRestore()
{
	vector<real> &Genes = m_GenePool.GetIndividual (m_iBestIndividualThisGeneration);
	m_pActiveBehavior->vSetVector (&Genes);
}
