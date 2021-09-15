/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: JuicePlay.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/


#include "stdafx.h"
#include "afxadv.h"
#include <float.h> // required for the _isnan in ViewLookAtSelection
#include "resource.h"
#include "JuicePlay.h"

#include "colordlg.h"
#include "modeldlg.h"
#include "OptionsDlg.h"
#include "DynamicsLibraryDlg.h"
#include "TerrainDlg.h"
#include "GenerateSnakeDlg.h"
#include "GenerateManypedeDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const char* g_szFileLoadTypes = "Juice Model (*.jm, *.jm3)\0*.jm;*.jm3\0All Files\0*.*\0\0";
const char* g_szFileSaveTypes = "Juice Model (*.jm3)\0*.jm3\0Old Juice Model (*.jm)\0*.jm\0All Files\0*.*\0\0";
const char* g_szDefaultFileExt = ".jm3";

// The one and only CJuicePlayApp object

CJuicePlayApp theApp;

BEGIN_MESSAGE_MAP(CJuicePlayApp, CWinApp)
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE15, OnOpenRecentFile)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)

	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVEAS, OnFileSaveas)

	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditNeedSelectionSet)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditNeedSelectionSet)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditNeedSelectionSet)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditNeedClipboard)

	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectall)

	ON_UPDATE_COMMAND_UI(ID_EDIT_SYMMETRICEDITING, OnUpdateEditSymmetricediting)
	ON_COMMAND(ID_EDIT_SYMMETRICEDITING, OnEditSymmetricediting)

	ON_UPDATE_COMMAND_UI (ID_EDIT_ADDBEAM, OnUpdateAddAnything)
	ON_UPDATE_COMMAND_UI (ID_EDIT_ADDHINGE, OnUpdateAddAnything)
	ON_UPDATE_COMMAND_UI (ID_EDIT_ADDSLIDER, OnUpdateAddAnything)
	ON_UPDATE_COMMAND_UI (ID_EDIT_ADDBALLJOINT, OnUpdateAddAnything)
	ON_UPDATE_COMMAND_UI (ID_EDIT_ADD_CLOCKWORK_BEHAVIOR, OnUpdateAddAnything)
	ON_UPDATE_COMMAND_UI (ID_EDIT_ADD_NETWORK_BEHAVIOR, OnUpdateAddAnything)
	ON_UPDATE_COMMAND_UI (ID_EDIT_ADDMOTION, OnUpdateEditAddmotion)

	ON_COMMAND(ID_EDIT_ADDBEAM, OnAddBeam)
	ON_COMMAND(ID_EDIT_ADDSLIDER, OnAddSlider)
	ON_COMMAND(ID_EDIT_ADDHINGE, OnAddHinge)
	ON_COMMAND(ID_EDIT_ADDBALLJOINT, OnEditAddBallJoint)
	ON_COMMAND(ID_EDIT_ADD_CLOCKWORK_BEHAVIOR, OnAddClockworkBehavior)
	ON_COMMAND(ID_EDIT_ADD_NETWORK_BEHAVIOR, OnAddNetworkBehavior)

	ON_COMMAND(ID_VIEW_LOOKATSELECTION, OnViewLookatselection)
	
	ON_UPDATE_COMMAND_UI(ID_MODE_DESIGN, OnUpdateModeDesign)
	ON_UPDATE_COMMAND_UI(ID_MODE_PAUSE, OnUpdateModePause)
	ON_UPDATE_COMMAND_UI(ID_MODE_PLAY, OnUpdateModePlay)
	ON_UPDATE_COMMAND_UI(ID_MODE_TRAIN, OnUpdateModeTrain)
	ON_UPDATE_COMMAND_UI(ID_MODE_FOLLOW, OnUpdateModeFollow)
	
	ON_COMMAND(ID_MODE_DESIGN, OnModeDesign)
	ON_COMMAND(ID_MODE_PAUSE, OnModePause)
	ON_COMMAND(ID_MODE_PLAY, OnModePlay)
	ON_COMMAND(ID_MODE_TRAIN, OnModeTrain)
	ON_COMMAND(ID_MODE_FOLLOW, OnModeFollow)

	ON_UPDATE_COMMAND_UI(ID_MODE_SNAP, OnUpdateModeSnap)
	ON_COMMAND(ID_MODE_SNAP, OnModeSnap)

	ON_COMMAND(ID_DRAG_XY, OnDragXY)
	ON_COMMAND(ID_DRAG_XZ, OnDragXZ)
	ON_COMMAND(ID_DRAG_YZ, OnDragYZ)

	ON_COMMAND(ID_OPTIONS_MODEL, OnOptionsModelParameters)
	ON_COMMAND(ID_OPTIONS_COLORS, OnOptionsColors)
	ON_COMMAND(ID_OPTIONS_PHYSICSPARAMETERS, OnOptionsPhysicsParameters)
	ON_COMMAND(ID_OPTIONS_GENERAL, OnOptionsGeneral)
	ON_COMMAND(ID_OPTIONS_CHOOSEDYNAMICSLIBRARY, OnOptionsChooseDynamicsLibrary)	

	ON_UPDATE_COMMAND_UI(ID_TERRAIN_ENABLE, OnUpdateTerrainEnable)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_SAVEAS, OnUpdateTerrainSaveas)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_NEW, OnUpdateTerrainNew)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_OPEN, OnUpdateTerrainOpen)
	ON_COMMAND(ID_TERRAIN_NEW, OnTerrainNew)
	ON_COMMAND(ID_TERRAIN_OPEN, OnTerrainOpen)
	ON_COMMAND(ID_TERRAIN_SAVEAS, OnTerrainSaveas)

	ON_UPDATE_COMMAND_UI(ID_TACK, OnUpdateTack)
	ON_COMMAND(ID_TACK, OnTack)
	ON_COMMAND(ID_GENERATE_SNAKE, OnGenerateSnake)
	ON_COMMAND(ID_GENERATE_MANYPEDE, OnGenerateManypede)
	ON_COMMAND(ID_FILE_IMPORT_JPM, OnFileImportJpm)
	ON_COMMAND(ID_TERRAIN_ENABLE, OnTerrainEnable)

	ON_COMMAND(ID_HELP_WEBSITE, OnHelpWebsite)	
END_MESSAGE_MAP()


// CJuicePlayApp construction

/****************************************************************************/
/** 
*****************************************************************************/
CJuicePlayApp::CJuicePlayApp() :
	m_uPauseAnimation (0)
{
	// _CrtSetBreakAlloc (96);
}


/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::vPauseAnimation ()
{
	m_uPauseAnimation++;
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::vResumeAnimation ()
{
	if (m_uPauseAnimation > 0)
		m_uPauseAnimation--;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool CJuicePlayApp::fAnimationPaused ()
{
	return (m_uPauseAnimation > 0);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::vSetTitle ()
{
	CString strTitle = "Juice: ";
	strTitle += m_strFileName;
	m_pMainWnd->SetWindowText (strTitle);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::vSetStatus (const char *szStatus)
{
	m_strStatus = szStatus;
	CWnd *pWnd = ((CFrameWnd*)m_pMainWnd)->GetMessageBar ();
	if (pWnd)
		pWnd->SetWindowText (szStatus);
}

/****************************************************************************/
/** 
*****************************************************************************/
bool CJuicePlayApp::fPromptIfChanged ()
{
	if (m_Juice.m_Model.fIsChanged ())
	{
		CString strCaption;
		CString strMessage;
		strCaption.LoadString (AFX_IDS_APP_TITLE);
		strMessage.LoadString (IDS_PROMPTSAVE_MESSAGE);

        int iChoice = MessageBox (NULL, strMessage, strCaption, MB_YESNOCANCEL);

		switch (iChoice)
		{
		case IDYES:
			OnFileSave ();
			return true;
			break;

		case IDCANCEL:
			return false;

		case IDNO:
			return true;
		}
	}
	
	return true;
}


/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::vUpdate3DViews ()
{
	CMainFrame *pMainFrame = (CMainFrame*) theApp.GetMainWnd ();
	std::list<C3DFrame*>::iterator iter;
	for (iter = pMainFrame->m_3DFrames.begin (); iter != pMainFrame->m_3DFrames.end (); iter++)
	{
		C3DFrame *pWnd = *iter;
		pWnd->vOnMainTimer ();
	}
}


/****************************************************************************/
/** 
*****************************************************************************/
BOOL CJuicePlayApp::InitInstance()
{
	if (!m_Juice.pGetActiveWorld ())
	{
		CString strMessage, strAppName;
		strMessage.LoadString (IDS_NODYNAMICSDLL);
		strAppName.LoadString (IDS_APPNAME);

		MessageBox (NULL, strMessage, strAppName, MB_OK);
		return FALSE;
	}

	if (!lstrcmpi (m_lpCmdLine, "debug"))
		g_Trace.fOpen ("c:\\temp\\JuicePlayDebug.log");

	GenericTrace ("CJuicePlayApp::InitInstance" << std::endl);

	CWinApp::InitInstance();

	// Initialize the application 
	GenericTrace ("Loading user preference data" << std::endl);
	m_Juice.m_Preferences.vLoad ();

	// Initialize 3D subsystem
	if (m_Juice.m_Scene.fInitialize ())
	{
		// TODO: investigate and eliminate
		// m_Juice.m_Scene.vSetBackground (Juice::Render::Scene::bgSky);
	}
	else
	{
		CString strMessage, strAppName;
		strMessage.LoadString (IDS_3D_INITFAILED);
		strAppName.LoadString (IDS_APPNAME);

		MessageBox (NULL, strMessage, strAppName, MB_OK);
		return FALSE;
	}

	// Standard initialization
	GenericTrace ("Loading MRU data" << std::endl);
	SetRegistryKey(_T("NateW"));
	LoadStdProfileSettings (10);

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMDIFrameWnd* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create main MDI frame window
	GenericTrace ("Creating main frame" << std::endl);
	if (!pFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	// load shared MDI menus and accelerator table
	HINSTANCE hInst = AfxGetResourceHandle();
	m_hMDIMenu  = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_MAINFRAME));
	m_hMDIAccel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_MAINFRAME));

	// Create but hide the training dialog
	m_TrainingDlg.Create (IDD_TRAINING, GetMainWnd ());
	m_TrainingDlg.ShowWindow (SW_HIDE);

	// The main window has been initialized, so show and update it
	pFrame->ShowWindow(m_nCmdShow);
	pFrame->UpdateWindow();

	// Create new MDI child windows: 3D view, text view, and property view, at minimum
	//
	// This cast and variable name are ugly, but m_pMainWnd is a base class member 
	// of the main window's parent class CMainFrame

	GenericTrace ("Creating MDI children" << std::endl);

	CMainFrame *pFrame2 = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);

	pFrame2->m_pComponents = (CComponentsFrame*) pFrame->CreateNewChild(
		RUNTIME_CLASS(CComponentsFrame), IDR_FRAME_COMPONENTS, m_hMDIMenu, m_hMDIAccel);

	pFrame2->m_pProperties = (CPropertiesFrame*) pFrame->CreateNewChild(
		RUNTIME_CLASS(CPropertiesFrame), IDR_FRAME_PROPERTIES, m_hMDIMenu, m_hMDIAccel);

	pFrame2->vCreateNew3dWindow ();
	pFrame2->vCreateNewMotionWindow ();

	// Register the properties from for selection set events
	pFrame2->m_pProperties->Listener<SelectionSetEvents>::vRegisterForEventsFrom (&m_Juice.m_SelectionSet);

	// Let the components frame register itself for notifications from the model
	pFrame2->m_pComponents->vSetModel (&m_Juice.m_Model);

	// Tile windows
	pFrame2->vTileChildren ();

	return TRUE;
}


/****************************************************************************/
/** 
*****************************************************************************/
int CJuicePlayApp::ExitInstance() 
{
	// Save user preferences
	m_Juice.m_Preferences.vSave ();

	if (m_hMDIMenu != NULL)
		FreeResource(m_hMDIMenu);

	if (m_hMDIAccel != NULL)
		FreeResource(m_hMDIAccel);

	return CWinApp::ExitInstance();
}

/****************************************************************************/
/** This does the basic CWinThread housekeeping, minus the CmdUI updating.
*** CmdUI updating via this method was too unreliable anyhow.
*****************************************************************************/
BOOL CJuicePlayApp::OnIdle(LONG lCount)
{
	ASSERT_VALID(this);

#if defined(_DEBUG) && !defined(_AFX_NO_DEBUG_CRT)
	// check MFC's allocator (before idle)
	if (_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & _CRTDBG_CHECK_ALWAYS_DF)
		ASSERT(AfxCheckMemory());
#endif

	if (lCount > 0)
	{
		//AFX_MODULE_THREAD_STATE* pState = m_pModuleState->m_thread; //_AFX_CMDTARGET_GETSTATE()->m_thread;
		//if (pState->m_nTempMapLock == 0)
		{
			// free temp maps, OLE DLLs, etc.
			AfxLockTempMaps();
			AfxUnlockTempMaps();
		}
	}
	else
	{
		if (!m_pMainWnd->IsIconic ())
			((CMainFrame*)m_pMainWnd)->vDoAnimationFrame ();
	}

#if defined(_DEBUG) && !defined(_AFX_NO_DEBUG_CRT)
	// check MFC's allocator (after idle)
	if (_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & _CRTDBG_CHECK_ALWAYS_DF)
		ASSERT(AfxCheckMemory());
#endif

	return lCount < 0;  // nothing more to do if lCount >= 0
}


/****************************************************************************/
/** stolen from AfxPriv.h
*****************************************************************************/
#define WM_IDLEUPDATECMDUI  0x0363  // wParam == bDisableIfNoHandler

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::vUpdateCommandUI ()
{
	// send WM_IDLEUPDATECMDUI to the main window
	CWnd* pMainWnd = m_pMainWnd;
	if (pMainWnd != NULL && pMainWnd->m_hWnd != NULL &&
		pMainWnd->IsWindowVisible())
	{
		AfxCallWndProc(pMainWnd, pMainWnd->m_hWnd,
			WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0);
		pMainWnd->SendMessageToDescendants(WM_IDLEUPDATECMDUI,
			(WPARAM)TRUE, 0, TRUE, TRUE);
	}
	// send WM_IDLEUPDATECMDUI to all frame windows

/*	AFX_MODULE_THREAD_STATE* pState = m_pModuleState->m_thread; //_AFX_CMDTARGET_GETSTATE()->m_thread;
	CFrameWnd* pFrameWnd = pState->m_frameList;
	while (pFrameWnd != NULL)
	{
		if (pFrameWnd->m_hWnd != NULL && pFrameWnd != pMainWnd)
		{
			if (pFrameWnd->m_nShowDelay == SW_HIDE)
				pFrameWnd->ShowWindow(pFrameWnd->m_nShowDelay);
			if (pFrameWnd->IsWindowVisible() ||
				pFrameWnd->m_nShowDelay >= 0)
			{
				AfxCallWndProc(pFrameWnd, pFrameWnd->m_hWnd,
					WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0);
				pFrameWnd->SendMessageToDescendants(WM_IDLEUPDATECMDUI,
					(WPARAM)TRUE, 0, TRUE, TRUE);
			}
			if (pFrameWnd->m_nShowDelay > SW_HIDE)
				pFrameWnd->ShowWindow(pFrameWnd->m_nShowDelay);
			pFrameWnd->m_nShowDelay = -1;
		}
		pFrameWnd = pFrameWnd->m_pNextFrameWnd;
	}
*/
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnEnableMenuItem (CCmdUI *pCmdUI)
{
	pCmdUI->Enable ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnDragXY()
{
	m_Juice.m_eDragPlane = (Juice::DragPlane) Juice::dpXY;
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnDragXZ()
{
	m_Juice.m_eDragPlane = (Juice::DragPlane) Juice::dpXZ;
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnDragYZ()
{
	m_Juice.m_eDragPlane = (Juice::DragPlane) Juice::dpYZ;
}


/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateAddAnything(CCmdUI *pCmdUI)
{
	if (m_Juice.eGetAppMode () == amDesign)
		pCmdUI->Enable (TRUE);
	else
		pCmdUI->Enable (FALSE);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnAddBeam()
{
	// doing this during operation would cause all sorts of problems
	if (m_Juice.eGetAppMode () != amDesign)
		return;

	// Find bounding box
	VectorXYZ vecMin, vecMax;
	m_Juice.m_Model.vGetStaticBounds (vecMin, vecMax);

	VectorXYZ vecPosition (0, 0, vecMax.z);

	m_Juice.m_SelectionSet.vReset ();

	if (m_Juice.m_eEditMode == emNormal)
	{
		Juice::Body *pBody = m_Juice.m_Model.pCreateBody ();

		// set initial position
		pBody->vSetPositionX (0);
		pBody->vSetPositionY (0);
		pBody->vSetPositionZ (vecMax.z);

		// set initial dimensions
		pBody->vSetDimensionX (1);
		pBody->vSetDimensionY (1);
		pBody->vSetDimensionZ (1);
	}
	else
	{
		Juice::Body *pBodyLeft = 0, *pBodyRight = 0;
		m_Juice.m_Model.fCreateBodyPair (&pBodyLeft, &pBodyRight);

		// set initial position
		pBodyLeft->vSetPositionX (-2);
		pBodyLeft->vSetPositionY (0);
		pBodyLeft->vSetPositionZ (vecMax.z);

		pBodyRight->vSetPositionX (+2);
		pBodyRight->vSetPositionY (0);
		pBodyRight->vSetPositionZ (vecMax.z);

		// set initial dimensions
		pBodyLeft->vSetDimensionX (1);
		pBodyLeft->vSetDimensionY (1);
		pBodyLeft->vSetDimensionZ (1);

		pBodyRight->vSetDimensionX (1);
		pBodyRight->vSetDimensionY (1);
		pBodyRight->vSetDimensionZ (1);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnAddHinge()
{
	// doing this during operation would cause all sorts of problems
	if (m_Juice.eGetAppMode () != amDesign)
		return;

	// Find bounding box
	VectorXYZ vecMin, vecMax;
	m_Juice.m_Model.vGetStaticBounds (vecMin, vecMax);

	m_Juice.m_SelectionSet.vReset ();

	if (m_Juice.m_eEditMode == emNormal)
	{
		Juice::RevoluteJoint *pJoint = m_Juice.m_Model.pCreateRevoluteJoint ();

		// set initial position
		pJoint->vSetPositionX (0);
		pJoint->vSetPositionY (0);
		pJoint->vSetPositionZ (vecMax.z);

		// set initial dimensions
		pJoint->vSetLength (1.0);
		pJoint->vSetDiameter (0.25);
	}
	else
	{
		Juice::RevoluteJoint *pLeft = 0, *pRight = 0;
		m_Juice.m_Model.fCreateRevoluteJointPair (&pLeft, &pRight);

		// set initial positions
		pLeft->vSetPositionX (-2);
		pLeft->vSetPositionY (0);
		pLeft->vSetPositionZ (vecMax.z);

		pRight->vSetPositionX (+2);
		pRight->vSetPositionY (0);
		pRight->vSetPositionZ (vecMax.z);

		// set initial dimensions
		pLeft->vSetLength (1.0);
		pLeft->vSetDiameter (0.25);
		pRight->vSetLength (1.0);
		pRight->vSetDiameter (0.25);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnAddSlider()
{
	// doing this during operation would cause all sorts of problems
	if (m_Juice.eGetAppMode () != amDesign)
		return;

	// Find bounding box
	VectorXYZ vecMin, vecMax;
	m_Juice.m_Model.vGetStaticBounds (vecMin, vecMax);

	m_Juice.m_SelectionSet.vReset ();

	if (m_Juice.m_eEditMode == emNormal)
	{
		Juice::PrismaticJoint *pJoint = m_Juice.m_Model.pCreatePrismaticJoint ();

		// set initial position
		pJoint->vSetPositionX (0);
		pJoint->vSetPositionY (0);
		pJoint->vSetPositionZ (vecMax.z);

		// set initial dimensions
		pJoint->vSetLength (1.0);
		pJoint->vSetDiameter (0.25);
	}
	else
	{
		Juice::PrismaticJoint *pLeft = 0, *pRight = 0;
		m_Juice.m_Model.fCreatePrismaticJointPair (&pLeft, &pRight);

		// set initial positions
		pLeft->vSetPositionX (-2);
		pLeft->vSetPositionY (0);
		pLeft->vSetPositionZ (vecMax.z);

		pRight->vSetPositionX (+2);
		pRight->vSetPositionY (0);
		pRight->vSetPositionZ (vecMax.z);

		// set initial dimensions
		pLeft->vSetLength (1.0);
		pLeft->vSetDiameter (0.25);
		pRight->vSetLength (1.0);
		pRight->vSetDiameter (0.25);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnEditAddBallJoint()
{
	// doing this during operation would cause all sorts of problems
	if (m_Juice.eGetAppMode () != amDesign)
		return;

	// Find bounding box
	VectorXYZ vecMin, vecMax;
	m_Juice.m_Model.vGetStaticBounds (vecMin, vecMax);

	m_Juice.m_SelectionSet.vReset ();

	if (m_Juice.m_eEditMode == emNormal)
	{
		Juice::SphericalJoint *pJoint = m_Juice.m_Model.pCreateSphericalJoint ();

		// set initial position
		pJoint->vSetPositionX (0);
		pJoint->vSetPositionY (0);
		pJoint->vSetPositionZ (vecMax.z);

		pJoint->vSetDiameter (1);
	}
	else
	{
		Juice::SphericalJoint *pLeft = 0, *pRight = 0;
		m_Juice.m_Model.fCreateSphericalJointPair (&pLeft, &pRight);

		// set initial positions
		pLeft->vSetPositionX (-2);
		pLeft->vSetPositionY (0);
		pLeft->vSetPositionZ (vecMax.z);

		pRight->vSetPositionX (+2);
		pRight->vSetPositionY (0);
		pRight->vSetPositionZ (vecMax.z);

		// set initial dimensions
		pLeft->vSetDiameter (1);
		pRight->vSetDiameter (1);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnAddClockworkBehavior()
{
	m_Juice.m_Model.pCreateClockworkBehavior ();
}

void CJuicePlayApp::OnAddNetworkBehavior()
{
	m_Juice.m_Model.pCreateNetworkBehavior ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateEditAddmotion(CCmdUI *pCmdUI)
{
	if (m_Juice.m_SelectionSet.uBehaviors() == 1)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnModeSnap()
{
	if (m_Juice.m_Preferences.m_fSnapEnabled)
		m_Juice.m_Preferences.m_fSnapEnabled = false;
	else
		m_Juice.m_Preferences.m_fSnapEnabled = true;
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateModeSnap(CCmdUI *pCmdUI)
{
	if (m_Juice.m_Preferences.m_fSnapEnabled)
		pCmdUI->SetCheck (TRUE);
	else
		pCmdUI->SetCheck (FALSE);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnFileNew() 
{
	if (!fPromptIfChanged ())
		return;

	m_Juice.vSetAppMode (Juice::amDesign);

	m_Juice.m_Model.vReset ();

	//VectorXYZ vecCenter (0, 0, 0);
	//m_Juice.m_Model.vGetCenter (vecCenter);

	OnViewLookatselection();

	m_strFileName = "";
	vSetTitle ();
}

/****************************************************************************/
/** 
*****************************************************************************/
bool CJuicePlayApp::fOpenFile (const char *szPath)
{
	m_Juice.vSetAppMode (Juice::amDesign);

	OnViewLookatselection();

	m_strFileName = szPath;
	if (GetFileAttributes (szPath) == -1)
	{
		CString strCaption;
		strCaption.LoadString (AFX_IDS_APP_TITLE);

		char szPrompt[MAX_PATH+100];
		wsprintf (szPrompt, "Are you sure this file exists?\n%s", szPath);

		MessageBox (NULL, szPrompt, strCaption, MB_OK);
		return false;
	}

	//if (!m_Juice.m_Model.fLoadFromFile (m_strFileName))
	if (!m_Juice.fReadFile (m_strFileName))
	{
		return false;
	}

	CMainFrame *pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CSliderCtrl *pSlider = (CSliderCtrl*) pFrame->m_wndToolBarSpeed.GetDlgItem (IDC_SPEED);
	pSlider->SetPos ((int) (m_Juice.m_Model.rGetSpeedFactor () * 100));

	OnViewLookatselection();

	vUpdate3DViews ();

	vSetTitle ();

	if (m_pRecentFileList != NULL)
	{
		m_pRecentFileList->Add (m_strFileName);
	}

	return true;
}

void CJuicePlayApp::OnFileOpen()
{
	if (!fPromptIfChanged ())
		return;

	// Create common file-open dialog box
	CFileDialog Dlg (TRUE);
	char szFileName[MAX_PATH];
	szFileName[0] = 0;

	Dlg.m_ofn.lStructSize = sizeof (OPENFILENAME);
	Dlg.m_ofn.hwndOwner = m_pMainWnd->m_hWnd;
	Dlg.m_ofn.lpstrFilter = g_szFileLoadTypes; 
	Dlg.m_ofn.lpstrDefExt = g_szDefaultFileExt;
	Dlg.m_ofn.nFilterIndex = 1;
	Dlg.m_ofn.lpstrFile = szFileName;
	Dlg.m_ofn.nMaxFile = MAX_PATH;
	Dlg.m_ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER;

	// Display the dialog box 
	if (IDOK == Dlg.DoModal())
	{
		fOpenFile (Dlg.m_ofn.lpstrFile);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnFileSave()
{
	if (m_strFileName.GetLength ())
		//m_Juice.m_Model.fSaveToFile (m_strFileName);
		m_Juice.fWriteFile (m_strFileName);
	else
		OnFileSaveas();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnFileSaveas()
{
	// Create common save-as dialog box
	CFileDialog Dlg (FALSE);
	char szFileName[MAX_PATH];
	szFileName[0] = 0;

	Dlg.m_ofn.lStructSize = sizeof (OPENFILENAME);
	Dlg.m_ofn.hwndOwner = m_pMainWnd->m_hWnd;
	Dlg.m_ofn.lpstrFilter = g_szFileSaveTypes;
	Dlg.m_ofn.lpstrDefExt = g_szDefaultFileExt;
	Dlg.m_ofn.nFilterIndex = 1;
	Dlg.m_ofn.lpstrFile = szFileName;
	Dlg.m_ofn.nMaxFile = MAX_PATH;
	Dlg.m_ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER;

	// Display the dialog box 
	if (IDOK == Dlg.DoModal())
	{
		m_strFileName = Dlg.m_ofn.lpstrFile;
		//m_Juice.m_Model.fSaveToFile (m_strFileName);
		m_Juice.fWriteFile (m_strFileName);

		if (m_pRecentFileList != NULL)
			m_pRecentFileList->Add (m_strFileName);
	
		vSetTitle ();
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateEditNeedSelectionSet(CCmdUI *pCmdUI)
{
	pCmdUI->Enable (FALSE);

	if (m_Juice.m_SelectionSet.uItems ())
		pCmdUI->Enable (TRUE);		
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateEditNeedClipboard(CCmdUI *pCmdUI)
{
	pCmdUI->Enable (m_Juice.m_Clipboard.fHasData ());
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnEditCut()
{
	// TODO: all clipboard operations should be commands 
	m_Juice.vCut ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnEditCopy()
{
	m_Juice.vCopy ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnEditPaste()
{
	m_Juice.vPaste ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnEditDelete()
{
	m_Juice.vDelete ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnEditSelectall()
{
	m_Juice.m_Model.vSelectAll ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnEditSymmetricediting()
{
	if (m_Juice.m_eEditMode == emNormal)
		m_Juice.m_eEditMode = emSymmetric;
	else
		m_Juice.m_eEditMode = emNormal;
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateEditSymmetricediting(CCmdUI *pCmdUI)
{
	if (m_Juice.m_eEditMode == emSymmetric)
		pCmdUI->SetCheck (TRUE);
	else
		pCmdUI->SetCheck (FALSE);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnViewLookatselection()
{
	const SelectableList &lsSelection = m_Juice.m_SelectionSet.lsGetContents ();
	VectorXYZ vecCenter;

	// Create a component list from the list of selected objects
	ComponentList lsComponents;
	SelectableList::const_iterator iter, iterLast = lsSelection.end ();
	for (iter = lsSelection.begin (); iter != iterLast; iter++)
	{
		if (Generic::IsKindOf<Component*> (*iter))
		{
			Component *p = dynamic_cast <Component*> (*iter);

			// the fact that this is necessary kinda bothers me
			// how can a Behavior* pass the IsKindOf<Component*> test???
			if (p)
			{
				lsComponents.push_front (p);
			}
		}
	}

	// Find the center of the components in the component list
	if (lsComponents.size ())
	{
		lsComponents.vGetCenter (vecCenter);
	}
	else
	{
		m_Juice.m_Model.vGetCenter (vecCenter);
	}

	// Aim the camera at the center
	int iClass = _fpclass (vecCenter.x);
	if (!(iClass & (_FPCLASS_QNAN | _FPCLASS_SNAN | _FPCLASS_NINF | _FPCLASS_PINF)))
		m_Juice.m_Scene.vSetLookAt (vecCenter);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateModeDesign(CCmdUI *pCmdUI)
{
	if (m_Juice.eGetAppMode () != Juice::amDesign)
		pCmdUI->Enable (TRUE);
	else
		pCmdUI->Enable (FALSE);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateModePause(CCmdUI *pCmdUI)
{
	pCmdUI->Enable (TRUE);

	if (m_Juice.fPaused ())
		pCmdUI->SetCheck (1);	
	else
		pCmdUI->SetCheck (0);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateModePlay(CCmdUI *pCmdUI)
{
	if (m_Juice.eGetAppMode () == amDesign)
		pCmdUI->Enable (TRUE);
	else
		pCmdUI->Enable (FALSE);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnModeDesign()
{
	m_Juice.vSetAppMode (Juice::amDesign);
	OnViewLookatselection();

	vUpdate3DViews ();

	vUpdateCommandUI ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnModePause()
{
	if (m_Juice.fPaused ())
		m_Juice.vResume ();
	else
		m_Juice.vPause ();

	vUpdateCommandUI ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnModePlay()
{
	m_Juice.vSetAppMode (Juice::amPose); 
	vUpdateCommandUI ();
}

/****************************************************************************/
/** 
*****************************************************************************/
BOOL CJuicePlayApp::OnOpenRecentFile(UINT nID)
{
	if (!fPromptIfChanged ())
		return TRUE;

	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList != NULL);
	if (!m_pRecentFileList) return FALSE;

	ASSERT(nID >= ID_FILE_MRU_FILE1);
	ASSERT(nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
	int nIndex = nID - ID_FILE_MRU_FILE1;
	ASSERT((*m_pRecentFileList)[nIndex].GetLength() != 0);

	TRACE(traceAppMsg, 0, _T("MRU: open file (%d) '%s'.\n"), (nIndex) + 1,
			(LPCTSTR)(*m_pRecentFileList)[nIndex]);

	if (!fOpenFile ((*m_pRecentFileList)[nIndex]))
		m_pRecentFileList->Remove(nIndex);

	return TRUE;
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnOptionsColors()
{
	CColorDlg Dlg;
	
	Juice::UserPreferences Preferences = m_Juice.m_Preferences;

	if (Dlg.DoModal () == IDCANCEL)
	{
		m_Juice.m_Preferences = Preferences;
	}
	else
	{
		m_Juice.m_Preferences.vSave ();
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnOptionsPhysicsParameters()
{
	Juice::Dynamics::World *pWorld = m_Juice.pGetActiveWorld ();
	pWorld->vConstantsDialog ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnOptionsGeneral()
{
	COptionsDlg Dlg;
	sprintf (Dlg.m_strMinDelta.GetBuffer(100), "%d", (UINT) m_Juice.m_Preferences.m_rMinDelta);
	sprintf (Dlg.m_strMaxDelta.GetBuffer(100), "%d", (UINT) m_Juice.m_Preferences.m_rMaxDelta);

	Dlg.m_strMinDelta.ReleaseBuffer ();
	Dlg.m_strMaxDelta.ReleaseBuffer ();

	if (Dlg.DoModal () == IDOK)
	{
		m_Juice.m_Preferences.m_rMinDelta = (real) atoi (Dlg.m_strMinDelta);
		m_Juice.m_Preferences.m_rMaxDelta = (real) atoi (Dlg.m_strMaxDelta);

		if (m_Juice.m_Preferences.m_rMinDelta == 0)
			m_Juice.m_Preferences.m_rMinDelta = 10;

		if (m_Juice.m_Preferences.m_rMaxDelta == 0)
			m_Juice.m_Preferences.m_rMaxDelta = 25;

		m_Juice.m_Preferences.vSave ();
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnOptionsModelParameters()
{
	CModelDlg Dlg;

	Dlg.m_rSpeed = m_Juice.m_Model.rGetStandardSpeed ();
	Dlg.m_rDelay = m_Juice.m_Model.rGetPoseDelay ();
	Dlg.m_rPhase = m_Juice.m_Model.rGetPosePhase ();

	if (Dlg.DoModal () == IDOK)
	{
		m_Juice.m_Model.vSetStandardSpeed (Dlg.m_rSpeed);
		m_Juice.m_Model.vSetPoseDelay (Dlg.m_rDelay);
		m_Juice.m_Model.vSetPosePhase (Dlg.m_rPhase);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnOptionsChooseDynamicsLibrary ()
{
	CDynamicsLibraryDlg Dlg (m_pMainWnd, m_Juice.GetDynamicsLibraryList (), m_Juice.pGetActiveWorld ());

	if (Dlg.DoModal () == IDOK)
	{
		m_Juice.vSetActiveLibrary (Dlg.m_pInfo);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateModeFollow(CCmdUI *pCmdUI)
{
	if (m_Juice.m_Preferences.m_fFollow)
		pCmdUI->SetCheck (1);
	else
		pCmdUI->SetCheck (0);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnModeFollow()
{
	m_Juice.m_Preferences.m_fFollow = !m_Juice.m_Preferences.m_fFollow;
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnHelpWebsite()
{
	char szURL[] = "http://www.natew.com/juice/frames.cgi/help/html.Help";
	HINSTANCE hInst = ShellExecute (NULL, "open", szURL, NULL, ".", SW_SHOW);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateTack (CCmdUI *pCmdUI)
{
	// Create a component list from the list of selected objects
	const SelectableList &lsSelection = m_Juice.m_SelectionSet.lsGetContents ();

	ComponentList lsComponents;
	SelectableList::const_iterator iter, iterLast = lsSelection.end ();
	for (iter = lsSelection.begin (); iter != iterLast; iter++)
	{
		if (Generic::IsKindOf<Component*> (*iter))
			lsComponents.push_front (dynamic_cast <Component*> (*iter));
	}

	// Depending on the size of the list of components...
	if (lsComponents.size () == 0)
	{
		pCmdUI->Enable (FALSE);
		pCmdUI->SetCheck (m_Juice.pTackedBody () != NULL);
	}
	else if (lsComponents.size () == 1)
	{
		pCmdUI->Enable (TRUE);

		if (lsComponents.front () == m_Juice.pTackedBody ())
		{
			pCmdUI->SetCheck (1);
		}
		else
		{
			pCmdUI->SetCheck (0);
		}
	}
	else
	{
		pCmdUI->Enable (FALSE);
		pCmdUI->SetCheck (m_Juice.pTackedBody () != NULL);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnTack ()
{
	// Create a component list from the list of selected objects
	const SelectableList &lsSelection = m_Juice.m_SelectionSet.lsGetContents ();

	ComponentList lsComponents;
	SelectableList::const_iterator iter, iterLast = lsSelection.end ();
	for (iter = lsSelection.begin (); iter != iterLast; iter++)
	{
		if (Generic::IsKindOf<Component*> (*iter))
			lsComponents.push_front (dynamic_cast <Component*> (*iter));
	}

	// Depending on the size of the list of components...
	if (lsComponents.size () == 0)
	{
		m_Juice.vTack (null);
	}
	else if (lsComponents.size () == 1)
	{
		if (lsComponents.front () == m_Juice.pTackedBody ())
		{
			m_Juice.vTack (null);
		}
		else
		{
			m_Juice.vTack (dynamic_cast<Body*> ((Body*) lsComponents.front ()));
		}
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnGenerateSnake()
{
	if (!fPromptIfChanged ())
		return;

	m_Juice.vSetAppMode (Juice::amDesign);

	m_Juice.m_Model.vReset ();

	m_strFileName = "";
	vSetTitle ();

	CGenerateSnakeDlg Dlg;
	Dlg.m_pModel = &m_Juice.m_Model;
	Dlg.DoModal ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnGenerateManypede()
{
	if (!fPromptIfChanged ())
		return;

	m_Juice.vSetAppMode (Juice::amDesign);

	m_Juice.m_Model.vReset ();

	m_strFileName = "";
	vSetTitle ();

	CGenerateManypedeDlg Dlg;
	Dlg.m_pModel = &m_Juice.m_Model;
	Dlg.DoModal ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnFileImportJpm()
{
/*	if (!fPromptIfChanged ())
		return;

	// Create common file-open dialog box
	CFileDialog Dlg (TRUE);
	char szFileName[MAX_PATH];
	szFileName[0] = 0;

	Dlg.m_ofn.lStructSize = sizeof (OPENFILENAME);
	Dlg.m_ofn.hwndOwner = m_pMainWnd->m_hWnd;
	Dlg.m_ofn.lpstrFilter = "Juice Models\0*.jpm\0All Files\0*.*\0\0";
	Dlg.m_ofn.lpstrDefExt = ".jpm";
	Dlg.m_ofn.nFilterIndex = 1;
	Dlg.m_ofn.lpstrFile = szFileName;
	Dlg.m_ofn.nMaxFile = MAX_PATH;
	Dlg.m_ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER;

	// Display the dialog box 
	if (IDOK == Dlg.DoModal())
	{
		m_Juice.vSetAppMode (Juice::amDesign);

		OnViewLookatselection();

		m_strFileName = Dlg.m_ofn.lpstrFile;
		if (GetFileAttributes (m_strFileName) == -1)
		{
			CString strCaption;
			strCaption.LoadString (AFX_IDS_APP_TITLE);

			char szPrompt[MAX_PATH+100];
			wsprintf (szPrompt, "Are you sure this file exists?\n%s", m_strFileName);

			MessageBox (NULL, szPrompt, strCaption, MB_OK);
			return;
		}

		if (!m_Juice.m_Model.fLoadFromFile_OldFormat (m_strFileName))
		{
			m_strFileName = "";
			return;
		}

		CMainFrame *pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
		CSliderCtrl *pSlider = (CSliderCtrl*) pFrame->m_wndToolBarSpeed.GetDlgItem (IDC_SPEED);
		pSlider->SetPos ((int) (m_Juice.m_Model.rGetSpeedFactor () * 100));

		if (m_strFileName[lstrlen (m_strFileName) - 1] == 'm')
		{
			m_strFileName.Delete (lstrlen (m_strFileName) - 2, 2);
			m_strFileName += "m";
		}

		OnViewLookatselection();

		vUpdate3DViews ();

		vSetTitle ();

		return;
	}
*/
}


/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnTerrainEnable()
{
	if (m_Juice.fTerrainEnabled ())
		m_Juice.vTerrainDisable ();
	else
		m_Juice.vTerrainEnable ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateTerrainEnable(CCmdUI *pCmdUI)
{
	pCmdUI->Enable (m_Juice.fTerrainAvailable () && (m_Juice.eGetAppMode() == Juice::amDesign));
	pCmdUI->SetCheck (m_Juice.fTerrainEnabled ());
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnTerrainNew()
{
	CTerrainDlg Dlg;
	Dlg.DoModal ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateTerrainNew(CCmdUI *pCmdUI)
{
	pCmdUI->Enable (m_Juice.eGetAppMode() == Juice::amDesign);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnTerrainOpen()
{
	// Create common file-open dialog box
	CFileDialog Dlg (TRUE);
	char szFileName[MAX_PATH];
	szFileName[0] = 0;

	Dlg.m_ofn.lStructSize = sizeof (OPENFILENAME);
	Dlg.m_ofn.hwndOwner = m_pMainWnd->m_hWnd;
	Dlg.m_ofn.lpstrFilter = "Juice Terrain\0*.jt\0All Files\0*.*\0\0";
	Dlg.m_ofn.lpstrDefExt = ".jt";
	Dlg.m_ofn.nFilterIndex = 1;
	Dlg.m_ofn.lpstrFile = szFileName;
	Dlg.m_ofn.nMaxFile = MAX_PATH;
	Dlg.m_ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER;

	// Display the dialog box 
	if (Dlg.DoModal() == IDOK)
	{
		const char *szErrorMsg = "Terrain loaded";

		switch (m_Juice.eTerrainOpen (Dlg.m_ofn.lpstrFile))
		{
		case tleUnableToSetElevation:
			szErrorMsg = "Unable to set terrain elevations";
			break;

		case tleUnableToSetCoarseTexture:
			szErrorMsg = "Unable to load terrain coarse texture";
			break;

		case tleUnableToSetDetailedTexture:
			szErrorMsg = "Unable to load terrain detailed texture";
			break;

		case tleUnknown:
			szErrorMsg = "Something went wrong while loading the terrain";
			break;
		}

		vSetStatus (szErrorMsg);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateTerrainOpen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable (m_Juice.eGetAppMode() == Juice::amDesign);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnTerrainSaveas()
{
	// Create common save-as dialog box
	CFileDialog Dlg (FALSE);
	char szFileName[MAX_PATH];
	szFileName[0] = 0;

	Dlg.m_ofn.lStructSize = sizeof (OPENFILENAME);
	Dlg.m_ofn.hwndOwner = m_pMainWnd->m_hWnd;
	Dlg.m_ofn.lpstrFilter = "Juice Terrain\0*.jt\0All Files\0*.*\0\0";
	Dlg.m_ofn.lpstrDefExt = ".jt";
	Dlg.m_ofn.nFilterIndex = 1;
	Dlg.m_ofn.lpstrFile = szFileName;
	Dlg.m_ofn.nMaxFile = MAX_PATH;
	Dlg.m_ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER;

	// Display the dialog box 
	if (IDOK == Dlg.DoModal())
		m_Juice.vTerrainSave (Dlg.m_ofn.lpstrFile);
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnUpdateTerrainSaveas(CCmdUI *pCmdUI)
{
	pCmdUI->Enable (m_Juice.fTerrainAvailable ());
}

void CJuicePlayApp::OnUpdateModeTrain(CCmdUI *pCmdUI)
{
#ifndef _DEBUG
	CMenu *pMenu = m_pMainWnd->GetMenu ();
	pMenu->RemoveMenu (ID_MODE_TRAIN, MF_BYCOMMAND);
#endif
}

/****************************************************************************/
/** 
*****************************************************************************/
void CJuicePlayApp::OnModeTrain()
{
	m_TrainingDlg.ShowWindow (SW_SHOWNORMAL);
}

// JuicePlay.cpp ends here ->