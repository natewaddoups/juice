/*****************************************************************************

Project:  Juice
Module:   JuiceDynamicsODE.dll
Filename: ConstantsDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceDynamicsODE.dll and its source code are distributed under the terms of 
the Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/
#include <JuiceEngine.h>
#include <DynamicsAPI.h>

#include <ode/ode.h>
#include "DynamicsODE.h"

#include "resource.h"

#include <atlbase.h>
//#include <atlapp.h>
#include <atlwin.h>

CAppModule _Module;

//#include <atlctrls.h>

//#include 

using namespace Juice;
using namespace Juice::Dynamics::ODE;

/****************************************************************************/
/** 
*****************************************************************************/
class Juice::Dynamics::ODE::ConstantsDlg : public CDialogImpl <ConstantsDlg>
{
	HWND m_hWnd;
	bool m_fIterative;

	CEdit m_ERP;
	CEdit m_CFM;
	CEdit m_Gravity;
	CEdit m_Bounce;
	CEdit m_Friction;
	CEdit m_Iterations;

public:

	enum { IDD = IDD_PHYSICS };

	ConstantsDlg (const World::Constants &Constants)
	{
		m_StandardConstants = Constants;
		m_fIterative = false;
	}

	ConstantsDlg (const IterativeWorld::Constants &Constants)
	{
		m_IterativeConstants = Constants;
		m_fIterative = true;
	}

	IterativeWorld::Constants m_IterativeConstants;
	StandardWorld::Constants m_StandardConstants;

BEGIN_MSG_MAP (ConstantsDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP ()

	void vSetValue (CEdit &Control, real rValue)
	{
		char szTmp[100];

		sprintf (szTmp, "%f", rValue);
		Control.SetWindowText (szTmp);
	}

	void vSetValue (CEdit &Control, Generic::UINT32 uValue)
	{
		char szTmp[100];

		sprintf (szTmp, "%u", uValue);
		Control.SetWindowText (szTmp);
	}

	bool fGetValue (CEdit &Control, real &rValue)
	{
		char szTmp[100];
		Control.GetWindowText (szTmp, 100);

		for (int i = 0; szTmp[i]; i++)
		{
			if (!(
				isdigit (szTmp[i]) ||
				szTmp[i] == '.' ||
				((szTmp[i] == '-') && (i == 0))
				))		
				return false;
		}

		rValue = atof (szTmp);

		return true;
	}

	bool fGetValue (CEdit &Control, Generic::UINT32 &uValue)
	{
		char szTmp[100];
		Control.GetWindowText (szTmp, 100);

		for (int i = 0; szTmp[i]; i++)
		{
			if (!isdigit (szTmp[i]))
				return false;
		}

		uValue = atoi (szTmp);

		return true;
	}


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		IterativeWorld::Constants Constants;
		if (m_fIterative)
			Constants = m_IterativeConstants;
		else
			Constants = m_StandardConstants;

		m_ERP.Attach (GetDlgItem (IDC_ERP));
		m_CFM.Attach (GetDlgItem (IDC_CFM));
		m_Gravity.Attach (GetDlgItem (IDC_GRAVITY));
		m_Friction.Attach (GetDlgItem (IDC_FRICTION));
		m_Bounce.Attach (GetDlgItem (IDC_BOUNCE));
		m_Iterations.Attach (GetDlgItem (IDC_ITERATIONS));

		vSetValue (m_ERP, Constants.m_rERP);
		vSetValue (m_CFM, Constants.m_rCFM);
		vSetValue (m_Gravity, Constants.m_rGravity);
		vSetValue (m_Friction, Constants.m_rFriction);
		vSetValue (m_Bounce, Constants.m_rBounce);
		vSetValue (m_Iterations, Constants.m_uIterations);

		if (!m_fIterative)
		{
			m_Iterations.ShowWindow (SW_HIDE);

			CStatic Iterations;
			HWND hWnd = GetDlgItem (IDC_ITERATIONS_ST);
			if (hWnd)
			{
				Iterations.Attach (hWnd);
				Iterations.ShowWindow (SW_HIDE);
			}
		}

		return 0;
	}

	LRESULT OnOK (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		IterativeWorld::Constants Constants; 

		fGetValue (m_ERP, Constants.m_rERP);
		fGetValue (m_CFM, Constants.m_rCFM);
		fGetValue (m_Gravity, Constants.m_rGravity);
		fGetValue (m_Friction, Constants.m_rFriction);
		fGetValue (m_Bounce, Constants.m_rBounce);
		
		if (!fGetValue (m_Iterations, Constants.m_uIterations))
		{
			MessageBox ("\"Iterations\" must be a positive integer", "Juice", MB_OK);
			return 0;
		}

		if (m_fIterative)
			m_IterativeConstants = Constants;
		else
			m_StandardConstants = Constants;

        EndDialog (IDOK);
		return 0;
	}

	LRESULT OnCancel (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog (IDCANCEL);
		return 0;
	}
};


/****************************************************************************/
/** 
*****************************************************************************/
void StandardWorld::vConstantsDialog ()
{
	ConstantsDlg Dlg (m_Constants);
	if (Dlg.DoModal () == IDOK)
	{
		m_Constants = Dlg.m_StandardConstants;
		vPrepareSimulation ();
	}	
}

/****************************************************************************/
/** 
*****************************************************************************/
void IterativeWorld::vConstantsDialog ()
{
	ConstantsDlg Dlg (m_Constants);
	if (Dlg.DoModal () == IDOK)
	{
		m_Constants = Dlg.m_IterativeConstants;

		// ! Base class only sees the derived class constants !
		//
		// Symptom of design flaw... must fix...
		World::m_Constants.m_rERP = Dlg.m_IterativeConstants.m_rERP;
		World::m_Constants.m_rCFM = Dlg.m_IterativeConstants.m_rCFM;
		World::m_Constants.m_rGravity = Dlg.m_IterativeConstants.m_rGravity;
		World::m_Constants.m_rFriction = Dlg.m_IterativeConstants.m_rFriction;
		World::m_Constants.m_rBounce = Dlg.m_IterativeConstants.m_rBounce;

		vPrepareSimulation ();
	}
}

// ConstantsDlg.cpp ends here ->