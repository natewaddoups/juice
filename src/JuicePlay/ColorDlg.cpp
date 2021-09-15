/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: ColorDlg.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include "stdafx.h"
#include "JuicePlay.h"
#include "ColorDlg.h"

// Definition of CColorDlg's static copy of the user preferences
//Juice::UserPreferences CColorDlg::m_Preferences;

// This structure associates a color's displayable name with a Generic::Color object in the user preferences object
typedef struct
{
	const char *szName;
	Generic::Color *pColor;
} ColorMap;

// An array of color names and values
ColorMap g_ColorMap[] =
{
	{ "3D Background",     &theApp.m_Juice.m_Preferences.m_clr3DBackground },
	{ "3D Cage",           &theApp.m_Juice.m_Preferences.m_clr3DBackgroundMesh },
	{ "3D Cage Highlight", &theApp.m_Juice.m_Preferences.m_clr3DBackgroundMeshHighlight },
	{ "Beam",              &theApp.m_Juice.m_Preferences.m_clrBeam     },
	{ "Hinge",             &theApp.m_Juice.m_Preferences.m_clrHinge    },
	{ "Slider",            &theApp.m_Juice.m_Preferences.m_clrSlider   },
	{ "Ball Joint",        &theApp.m_Juice.m_Preferences.m_clrBallJoint},
	{ "Selected Object",   &theApp.m_Juice.m_Preferences.m_clrSelected },
	{ "Shadow",            &theApp.m_Juice.m_Preferences.m_clrShadow   },
};

// MFC Voodoo for dynamic object creation
IMPLEMENT_DYNAMIC(CColorDlg, CDialog)

/****************************************************************************/
/** Constructor... just initializes a couple of members
*****************************************************************************/
CColorDlg::CColorDlg(CWnd* pParent /*=NULL*/)
	: CJuiceDlg(CColorDlg::IDD, pParent),
	m_pCurrentColor (null)
{
}

/****************************************************************************/
/** Destrutor does nothing of interest 
*****************************************************************************/
CColorDlg::~CColorDlg()
{
}

/****************************************************************************/
/** Draws a rectangle on the dialog box to show the color of the currently
 ** selected item in the list.
*****************************************************************************/
void CColorDlg::vDrawCurrentColor (CDC *pDC)
{
	// Create a brush...
	CBrush br;

	if (m_pCurrentColor)
	{
		// Create a brush using the current color
		LOGBRUSH lb = { BS_SOLID, RGB (m_pCurrentColor->r * 255, m_pCurrentColor->g * 255, m_pCurrentColor->b * 255), 0 };
		br.CreateBrushIndirect (&lb);
	}
	else
	{
		// If there is no current color, just use black for now (this should never happen)
		LOGBRUSH lb = { BS_SOLID, RGB (0,0,0), 0 };
		br.CreateBrushIndirect (&lb);
	}

	// Create a black pen for the border of the colors-sample rectangle
	CPen pn;
	pn.CreatePen (PS_SOLID, 1, (COLORREF) 0);

	// Select the pen and brush
	CPen *pnOld = pDC->SelectObject (&pn);
	CBrush *brOld = pDC->SelectObject (&br);

	// Draw the rectangle
	pDC->Rectangle (&m_rcColorSample);

	// Deselect the GDI objects
	pDC->SelectObject (pnOld);
	pDC->SelectObject (brOld);

	// The destructors will destroy the GDI objects
}

/****************************************************************************/
/** MFC dialog-control data exchange voodoo.  Let the IDE edit this code.
*****************************************************************************/
void CColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLORLIST, m_ColorList);
}


/****************************************************************************/
/** MFC message map voodoo.  Let the IDE edit this code.
*****************************************************************************/
BEGIN_MESSAGE_MAP(CColorDlg, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST1, OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_CHOOSECOLOR, OnBnClickedChoosecolor)
	ON_WM_PAINT()
END_MESSAGE_MAP()


/****************************************************************************/
/** This is invoked before the dialog box is displayed
*****************************************************************************/
BOOL CColorDlg::OnInitDialog()
{
	// Let the base class do its thing
	CDialog::OnInitDialog();

	// A hidden static-text control is used as a template for the color-sample 
	// rectangle.  This code gets the control's dimensions.

	HWND hStatic = NULL;
	GetDlgItem (IDC_COLORSAMPLE, &hStatic);

	if (hStatic)
	{
		::GetWindowRect (hStatic, &m_rcColorSample);
		ScreenToClient (&m_rcColorSample);
		::DestroyWindow (hStatic);
	}

	// Populate the listbox with color names
	for (UINT u = 0; u < sizeof (g_ColorMap) / sizeof (ColorMap); u++)
		m_ColorList.AddString (g_ColorMap[u].szName);

	// One should return TRUE unless the focus was explicitly set to a control
	return TRUE;  
}

/****************************************************************************/
/** This is called when the listbox selection changes
*****************************************************************************/
void CColorDlg::OnLbnSelchangeList1()
{
	// Get the new selection
	UINT uSelection = m_ColorList.GetCurSel ();

	// Get the entry from the color map
	if (uSelection < sizeof (g_ColorMap) / sizeof (ColorMap))
	{
		m_pCurrentColor = g_ColorMap[uSelection].pColor;
	}
	else
	{
		// This should never happen, but just in case...
		m_pCurrentColor = null;
	}

	// Draw the current color in the sample rectangle
	CDC *pDC = GetDC ();
	vDrawCurrentColor (pDC);
}

/****************************************************************************/
/** This is called when the user hits the 'choose color' button.  This 
 ** function creates a 'Choose Color' common dialog box.
*****************************************************************************/
void CColorDlg::OnBnClickedChoosecolor()
{
	// Custom colors are stored here for the lifetime of the application
	// TODO: store the custom colors as user preferences for the lifetime of
	// the installation.
	static COLORREF CustomColors[16];

	// Initialize the data structure for the 'choose color' dialog box
	CHOOSECOLOR cc;
	cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	cc.hInstance = NULL;
	cc.hwndOwner = m_hWnd;
	cc.lCustData = NULL;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;
	cc.lStructSize = sizeof (cc);
	cc.rgbResult = RGB (m_pCurrentColor->r * 255, m_pCurrentColor->g * 255, m_pCurrentColor->b * 255);
	cc.lpCustColors = CustomColors;
	
	// Run the 'choose color' dialog box
	if (ChooseColor (&cc))
	{
		// If the user chose a color, these integer values will describe the color.
		// Copy them into the floating-point variables used by the application.
		m_pCurrentColor->r = (real) GetRValue (cc.rgbResult) / 255.0;
		m_pCurrentColor->g = (real) GetGValue (cc.rgbResult) / 255.0;
		m_pCurrentColor->b = (real) GetBValue (cc.rgbResult) / 255.0;
		m_pCurrentColor->a = 0;

		CDC *pDC = GetDC ();
		vDrawCurrentColor (pDC);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void CColorDlg::OnPaint()
{
	CPaintDC dc(this); 
	vDrawCurrentColor (&dc);
}

// ColorDlg.cpp ends here ->