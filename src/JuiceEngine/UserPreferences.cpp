/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: UserPreferences.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <windows.h>
#include <JuiceEngine.h>

using namespace Juice;
using namespace Generic;

/// This class is declared here to keep Win32 details out of the rest of the code.
class Registry
{
    //HKEY m_hCurrentUser;
	//HKEY m_hSoftware;
public:
	Registry ();
	~Registry ();

	HKEY m_hJuice;
	HKEY m_hAppearance;
	HKEY m_hPhysics;
	HKEY m_hTerrain;

	// Registry read methods
	bool fLoadString (HKEY hKey, const char* szName, std::string *pstrValue, const char *szDefault);
	bool fLoadNumber (HKEY hKey, const char* szName, real *prValue, real rDefault);
	bool fLoadColor  (HKEY hKey, const char* szName, Color *pclrValue, Color &clrDefault);
	bool fLoadBool   (HKEY hKey, const char* szName, bool *pfValue, bool fDefault);

	// Registry write methods
	bool fSaveString (HKEY hKey, const char* szName, const char *szValue);
	bool fSaveNumber (HKEY hKey, const char* szName, real *prValue);
	bool fSaveColor  (HKEY hKey, const char* szName, Color *pclrValue);
	bool fSaveBool   (HKEY hKey, const char* szName, bool *pfValue);
};

typedef struct
{ 
	const char *szName;
	std::string *pstrValue;
	const char *szDefault;
} RegString;

typedef struct
{ 
	const char *szName;
	Color *pclrValue;
	Color clrDefault;
} RegColor;

typedef struct
{ 
	const char *szName;
	real *prValue;
	real rDefault;
} RegNumber;

typedef struct
{ 
	const char *szName;
	bool *pfValue;
	bool fDefault;
} RegBool;

#define DeclareRegBaseBools(x) \
	RegBool x[] = \
	{ \
		{ "Snap Enabled", &m_fSnapEnabled, true }, \
		{ "Camera Follows Model", &m_fFollow, true }, \
		{ "Render Joints", &m_fRenderJoints, true }, \
	};

#define DeclareRegBaseNumbers(x) \
	RegNumber x[] = \
	{ \
		{ "Snap Resolution", &m_rSnapResolution, 0.25 }, \
		{ "Cage Size", &m_rCageSize, 50 }, \
	};

#define DeclareRegAppearanceStrings(x) \
	RegString x[] = \
	{ \
		{ "Ground Texture", &m_strGroundTexture, "ground.ppm" }, \
		{ "Sky Texture", &m_strSkyTexture, "sky.ppm" }, \
	};

#define DeclareRegAppearanceColors(x) \
	RegColor x[] = \
	{ \
		{ "3D Background", &m_clr3DBackground, {0.0, 0.0, 0.0, 0.0} }, \
		{ "3D Background Mesh", &m_clr3DBackgroundMesh, {0.5, 0.5, 0.5, 0.0} }, \
		{ "3D Background Mesh Highlight", &m_clr3DBackgroundMeshHighlight, {0.8, 0.8, 0.3, 0.0} }, \
		{ "Network Background", &m_clrNetworkBackground, { 0, 0, 0, 0} }, \
		{ "Network Node", &m_clrNetworkNode, { 0, 0, 1, 0 } }, \
		{ "Network Node Text", &m_clrNetworkNodeText, { 1, 1, 1, 0 } }, \
		{ "Network Node Border", &m_clrNetworkNodeBorder, { .7, .7, .7, 0 } }, \
		{ "Network Edge", &m_clrNetworkEdge, { 1, 1, 0, 0 } }, \
		{ "Beam", &m_clrBeam, {0.3, 0.3, 0.8, 0.0} }, \
		{ "Hinge", &m_clrHinge, {0.5, 0.5, 0.0, 0.0} }, \
		{ "Slider", &m_clrSlider, {0.5, 0.0, 0.0, 0.0} }, \
		{ "Ball Joint", &m_clrBallJoint, {0.0, 0.2, 0.0} }, \
		{ "Selected", &m_clrSelected, {1.0, 1.0, 1.0, 0.0} }, \
		{ "Shadow", &m_clrShadow, {0.25, 0.25, 0.25, 0.0} }, \
		{ "Motion Background", &m_clrMotionBackground, {1.0, 1.0, 1.0, 0.0} }, \
		{ "Motion Wave", &m_clrMotionWave, {0.0, 0.0, 1.0, 0.0} }, \
		{ "Motion Link", &m_clrMotor, {1.0, 0.0, 0.0, 0.0} }, \
	};

#define DeclareRegPhysicsNumbers(x) \
	RegNumber x[] = \
	{ \
		{ "Gravity", &m_rGravity, 9.81 }, \
		{ "GroundFriction", &m_rGroundFriction, 10 }, \
		{ "GroundBounce", &m_rGroundBounce, 0.0 }, \
		{ "GlobalERP", &m_rGlobalERP, 0.5 }, \
		{ "GlobalCFM", &m_rGlobalCFM, 0.01 }, \
		{ "MinDelta", &m_rMinDelta, 5 }, \
		{ "MaxDelta", &m_rMaxDelta, 25 }, \
	};

#define DeclareRegTerrainString(x) \
	RegString x[] = \
	{ \
		{ "Elevation", &m_strTerrainElevation, "Terrain\\SampleElevation.jpg" }, \
		{ "TextureCoarse", &m_strTerrainTextureCoarse, "Terrain\\SampleTextureCoarse.jpg" }, \
		{ "TextureDetailed", &m_strTerrainTextureDetailed, "Terrain\\SampleTextureDetailed.jpg" }, \
	};

#define DeclareRegTerrainNumber(x) \
	RegNumber x[] = \
	{ \
		{ "HorizontalScale", &m_rTerrainScaleHorizontal, 20.0 }, \
		{ "VerticalScale", &m_rTerrainScaleVertical, 0.4 }, \
		{ "TriangleCount", &m_rTerrainTriangleCount, 10000.0 }, \
	};


#define DeclareRegistryData() \
	DeclareRegBaseBools(RegBaseBool) \
	DeclareRegBaseNumbers(RegBaseNumber) \
	DeclareRegAppearanceStrings(RegAppearanceString) \
	DeclareRegAppearanceColors(RegAppearanceColor) \
	DeclareRegPhysicsNumbers(RegPhysicsNumber) \
	DeclareRegTerrainString(RegTerrainString) \
	DeclareRegTerrainNumber(RegTerrainNumber)

Registry g_Registry;

/****************************************************************************/
/** Open registry keys
*****************************************************************************/
Registry::Registry ()
{
	DWORD dwAccess = KEY_CREATE_SUB_KEY | KEY_QUERY_VALUE | KEY_SET_VALUE;
    //RegOpenCurrentUser (dwAccess, &m_hCurrentUser);
	RegCreateKeyEx (HKEY_CURRENT_USER, "Software\\NateW\\Juice", 0, NULL, REG_OPTION_NON_VOLATILE, dwAccess, NULL, &m_hJuice, NULL);
	//RegOpenKeyEx (m_hCurrentUser, "Software", NULL, dwAccess, &m_hSoftware);
	//RegCreateKeyEx (m_hSoftware, "Juice", NULL, NULL, NULL, dwAccess, NULL, &m_hJuice, NULL);
	RegCreateKeyEx (m_hJuice, "Appearance", NULL, NULL, NULL, dwAccess, NULL, &m_hAppearance, NULL);
 	RegCreateKeyEx (m_hJuice, "Physics", NULL, NULL, NULL, dwAccess, NULL, &m_hPhysics, NULL);
	RegCreateKeyEx (m_hJuice, "Terrain", NULL, NULL, NULL, dwAccess, NULL, &m_hTerrain, NULL);
}

/****************************************************************************/
/** Close registry keys
*****************************************************************************/
Registry::~Registry ()
{
//	RegCloseKey (m_hCurrentUser);
	RegCloseKey (m_hJuice);
	RegCloseKey (m_hAppearance);
	RegCloseKey (m_hPhysics);
	RegCloseKey (m_hTerrain);
}

/****************************************************************************/
/** 
*****************************************************************************/
// Registry read methods
bool Registry::fLoadString (HKEY hKey, const char* szName, std::string *pstrValue, const char *szDefault)
{
	DWORD dwType = 0;
	const int icbBufferSize = MAX_PATH;
	char szBuffer[icbBufferSize];
	DWORD icbBufferUsed = icbBufferSize;

	LONG lResult = RegQueryValueEx (hKey, szName, NULL, &dwType, (LPBYTE) szBuffer, &icbBufferUsed);
	if (ERROR_SUCCESS == lResult)
	{
		if (icbBufferUsed > 0)
			*pstrValue = szBuffer;
		else
			*pstrValue = "";
		return true;
	}
	else
	{
		//vShowLastError ("Unable to load preference data");
		*pstrValue = szDefault;
		return false;
	}
}

bool Registry::fLoadNumber (HKEY hKey, const char* szName, real *prValue, real rDefault)
{
	std::string strValue;
	if (!fLoadString (hKey, szName, &strValue, ""))
	{
		*prValue = rDefault;
		return false;
	}

	*prValue = (real) atof (strValue.c_str ());
	return true;
}

bool Registry::fLoadColor  (HKEY hKey, const char* szName, Color *pclrValue, Color &clrDefault)
{
	std::string strValue;
	if (!fLoadString (hKey, szName, &strValue, ""))
	{
		*pclrValue = clrDefault;
		return false;
	}

	std::string strRed = strValue.substr (0, 3);
	std::string strGreen = strValue.substr (4, 3);
	std::string strBlue = strValue.substr (8, 3);
	std::string strAlpha = strValue.substr (12, 3);

	pclrValue->r = (float) atoi (strRed.c_str ()) / 255.0f;
	pclrValue->g = (float) atoi (strGreen.c_str ()) / 255.0f;
	pclrValue->b = (float) atoi (strBlue.c_str ()) / 255.0f;
	pclrValue->a = (float) atoi (strAlpha.c_str ()) / 255.0f;

	return true;
}

bool Registry::fLoadBool   (HKEY hKey, const char* szName, bool *pfValue, bool fDefault)
{
	std::string strValue;
	if (!fLoadString (hKey, szName, &strValue, ""))
	{
		*pfValue = fDefault;
		return false;
	}

	char c = strValue[0];

	if ((c == 't') || (c == 'T') || (c == 'y') || (c == 'Y'))
	{
		*pfValue = true;
		return true;
	}
	else if ((c == 'f') || (c == 'F') || (c == 'n') || (c == 'N'))
	{
		*pfValue = false;
		return true;
	}
	else 
	{
		*pfValue = fDefault;
		return false;
	}
}


// Registry write methods
bool Registry::fSaveString (HKEY hKey, const char* szName, const char *szValue)
{
	return (ERROR_SUCCESS == RegSetValueEx (hKey, szName, NULL, REG_SZ, (const BYTE*) szValue, strlen (szValue)));
}

bool Registry::fSaveNumber (HKEY hKey, const char* szName, real *prValue)
{
	char szTmp[50];
	sprintf (szTmp, "%f", (double) *prValue);
	return fSaveString (hKey, szName, szTmp);
}

bool Registry::fSaveColor  (HKEY hKey, const char* szName, Color *pclrValue)
{
	char szTmp[50];
	int r = (int) (pclrValue->r * 255.0);
	int g = (int) (pclrValue->g * 255.0);
	int b = (int) (pclrValue->b * 255.0);
	int a = (int) (pclrValue->a * 255.0);
	sprintf (szTmp, "%03d %03d %03d %03d", r, g, b, a);
	return fSaveString (hKey, szName, szTmp);
}

bool Registry::fSaveBool   (HKEY hKey, const char* szName, bool *pfValue)
{
	if (*pfValue)
	{
		return fSaveString (hKey, szName, "true");
	}
	else
	{
		return fSaveString (hKey, szName, "false");
	}
}

/****************************************************************************/
/** Load all settings from the registry
*****************************************************************************/
void UserPreferences::vLoad ()
{
	DeclareRegistryData();

	int i;

	// Base/bool
	for (i = 0; i < sizeof (RegBaseBool) / sizeof (RegBool); i++)
		g_Registry.fLoadBool (g_Registry.m_hJuice, RegBaseBool[i].szName, RegBaseBool[i].pfValue, RegBaseBool[i].fDefault);
	
	// Base/number
	for (i = 0; i < sizeof (RegBaseNumber) / sizeof (RegNumber); i++)
		g_Registry.fLoadNumber (g_Registry.m_hJuice, RegBaseNumber[i].szName, RegBaseNumber[i].prValue, RegBaseNumber[i].rDefault);

	// Base/physics/number
	for (i = 0; i < sizeof (RegPhysicsNumber) / sizeof (RegNumber); i++)
		g_Registry.fLoadNumber (g_Registry.m_hPhysics, RegPhysicsNumber[i].szName, RegPhysicsNumber[i].prValue, RegPhysicsNumber[i].rDefault);

	// Base/appearance/string
	for (i = 0; i < sizeof (RegAppearanceString) / sizeof (RegString); i++)
		g_Registry.fLoadString (g_Registry.m_hAppearance, RegAppearanceString[i].szName, RegAppearanceString[i].pstrValue, RegAppearanceString[i].szDefault);

	// Base/appearance/color
	for (i = 0; i < sizeof (RegAppearanceColor) / sizeof (RegColor); i++)
		g_Registry.fLoadColor (g_Registry.m_hAppearance, RegAppearanceColor[i].szName, RegAppearanceColor[i].pclrValue, RegAppearanceColor[i].clrDefault);

	// Base/terrain/string
	for (i = 0; i < sizeof (RegTerrainString) / sizeof (RegString); i++)
		g_Registry.fLoadString (g_Registry.m_hTerrain, RegTerrainString[i].szName, RegTerrainString[i].pstrValue, RegTerrainString[i].szDefault);

	// Base/terrain/number
	for (i = 0; i < sizeof (RegTerrainNumber) / sizeof (RegNumber); i++)
		g_Registry.fLoadNumber (g_Registry.m_hTerrain, RegTerrainNumber[i].szName, RegTerrainNumber[i].prValue, RegTerrainNumber[i].rDefault);

}

/****************************************************************************/
/** Save all settings to the registry
*****************************************************************************/
void UserPreferences::vSave ()
{
	DeclareRegistryData();

	int i;

	// Base/bool
	for (i = 0; i < sizeof (RegBaseBool) / sizeof (RegBool); i++)
		g_Registry.fSaveBool (g_Registry.m_hJuice, RegBaseBool[i].szName, RegBaseBool[i].pfValue);
	
	// Base/number
	for (i = 0; i < sizeof (RegBaseNumber) / sizeof (RegNumber); i++)
		g_Registry.fSaveNumber (g_Registry.m_hJuice, RegBaseNumber[i].szName, RegBaseNumber[i].prValue);

	// Base/physics/number
	for (i = 0; i < sizeof (RegPhysicsNumber) / sizeof (RegNumber); i++)
		g_Registry.fSaveNumber (g_Registry.m_hPhysics, RegPhysicsNumber[i].szName, RegPhysicsNumber[i].prValue);

	// Base/appearance/string
	for (i = 0; i < sizeof (RegAppearanceString) / sizeof (RegString); i++)
		g_Registry.fSaveString (g_Registry.m_hAppearance, RegAppearanceString[i].szName, RegAppearanceString[i].pstrValue->c_str());

	// Base/appearance/color
	for (i = 0; i < sizeof (RegAppearanceColor) / sizeof (RegColor); i++)
		g_Registry.fSaveColor (g_Registry.m_hAppearance, RegAppearanceColor[i].szName, RegAppearanceColor[i].pclrValue);

	// Base/terrain/string
	for (i = 0; i < sizeof (RegTerrainString) / sizeof (RegString); i++)
		g_Registry.fSaveString (g_Registry.m_hTerrain, RegTerrainString[i].szName, RegTerrainString[i].pstrValue->c_str ());

	// Base/terrain/number
	for (i = 0; i < sizeof (RegTerrainNumber) / sizeof (RegNumber); i++)
		g_Registry.fSaveNumber (g_Registry.m_hTerrain, RegTerrainNumber[i].szName, RegTerrainNumber[i].prValue);
}


/****************************************************************************/
/* Setter methods so the application can write to these strings
*****************************************************************************/
void UserPreferences::vSetTerrainElevation (const char *sz)
{
	m_strTerrainElevation = sz;
}

void UserPreferences::vSetTerrainTextureCoarse (const char *sz)
{
	m_strTerrainTextureCoarse = sz;
}

void UserPreferences::vSetTerrainTextureDetailed (const char *sz)
{
	m_strTerrainTextureDetailed = sz;
}

// UserPreferences.cpp ends here ->