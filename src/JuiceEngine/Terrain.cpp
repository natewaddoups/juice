/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Terrain.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>

// #include <Terrain\Terrain.h>
// #include "Terrain.h"

using namespace Juice;
using namespace Generic;

void vStringAssign (std::string *pDest, const char *pSrc);

/****************************************************************************/
/** 
*****************************************************************************/
void Application::vTerrainInitialize ()
{
//	return;

	m_pTerrain = new Juice::Terrain;

	if (!m_pTerrain)
		return;

	m_pTerrain->vInitialize ();

	m_fTerrainAvailable = false;
	m_fTerrainEnabled = false;
}

/****************************************************************************/
/** 
*****************************************************************************/
TerrainLoadError Application::eTerrainNew (const char *szElevationImage, const char *szTexture, const char *szDetailTexture, 
			real rVertexSpacing, real rElevationScale, int iMaxNumTriangles)
{
	if (!m_pTerrain)
	{
		m_fTerrainAvailable = false;
		m_fTerrainEnabled = false;
		return tleUnknown;
	}

	m_pTerrain->vSetParameters (szElevationImage, szTexture, szDetailTexture, rVertexSpacing, rElevationScale, iMaxNumTriangles);

	TerrainLoadError eError = m_pTerrain->eLoad ();

	if (eError != tleSuccess)
	{
		m_fTerrainAvailable = false;
		m_fTerrainEnabled = false;
		return eError;
	}

	if (m_pWorld)
	{
		m_pWorld->vCreateTerrain (m_pTerrain);
		m_pWorld->vEnableTerrain ();
	}
	else
	{
		GenericUnexpected ("terrain disabled");
	}

	m_fTerrainEnabled = true;
	m_fTerrainAvailable = true;
	return eError;
}

/****************************************************************************/
/** 
*****************************************************************************/
TerrainLoadError Application::eTerrainOpen (const char *szPath)
{
	if (!m_pTerrain)
	{
		m_fTerrainAvailable = false;
		m_fTerrainEnabled = false;
		return tleUnknown;
	}

	bool fReturn = true;
	XML::StreamReader Reader;

	if (!Reader.fRead (szPath, m_pTerrain, vStringAssign))
		fReturn = false;

	::TerrainLoadError eError = m_pTerrain->eLoad ();
	if (eError == tleSuccess)
	{
		m_fTerrainEnabled = m_fTerrainAvailable = true;

		if (m_pWorld)
		{
			m_pWorld->vCreateTerrain (m_pTerrain);
			m_pWorld->vEnableTerrain ();			
		}
		else
		{
			GenericUnexpected ("terrain disabled");
		}
	}
	else
	{
		m_fTerrainEnabled = m_fTerrainAvailable = false;
	}
	
	return eError;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Application::vTerrainSave (const char *szPath)
{
	XML::StreamWriter Writer;
	Writer.fWrite (szPath, m_pTerrain, 0);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Application::vTerrainDisable ()
{
	m_fTerrainEnabled = false;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Application::vTerrainEnable ()
{
	if (m_pTerrain)
		m_fTerrainEnabled = true;
	else
		GenericUnexpected ("terrain disabled");
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Application::fTerrainEnabled ()
{
	return m_fTerrainEnabled;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Application::fTerrainAvailable ()
{
	return m_fTerrainAvailable;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Application::vTerrainDraw ()
{
	if (m_pTerrain)
		m_pTerrain->vRender ();
	else
		GenericUnexpected ("terrain disabled");
}

/****************************************************************************/
/** 
*****************************************************************************/
Juice::Terrain* Application::pGetTerrain ()
{
	return m_pTerrain;
}

/****************************************************************************/
/** 
*****************************************************************************/
Terrain::Terrain () : m_pTerrain (null), m_pTriangleArray (null), m_pVertexArray (null), XML::Serializable (L"Terrain")
{
	char szPath[MAX_PATH];

	GetModuleFileName (NULL, szPath, MAX_PATH);

	char *pLastSlash = strrchr (szPath, '\\');
	*pLastSlash = 0;

	strcat (szPath, "\\Terrain");
/*
	Demeter::Settings::GetInstance()->SetMediaPath(szPath);
	Demeter::Settings::GetInstance()->SetVerbose(false);
	Demeter::Settings::GetInstance()->SetTextureCompression(true);
	*/
}

/****************************************************************************/
/** 
*****************************************************************************/
Terrain::~Terrain ()
{
	delete [] m_pTriangleArray;
	delete [] m_pVertexArray;
	delete m_pTerrain;

	m_pTriangleArray = null;
	m_pVertexArray = null;
	m_pTerrain = null;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Terrain::vInitialize ()
{
	delete [] m_pTriangleArray;
	delete [] m_pVertexArray;
	delete m_pTerrain;

	m_pTriangleArray = null;
	m_pVertexArray = null;
	m_pTerrain = null;

	try
	{
		//m_pTerrain = new Demeter::Terrain(8, 8, 1, 10000, false);
	}
/*	catch (DemeterException* pEx)
	{
		OutputDebugString (pEx->GetErrorMessage ());
	}
*/
	catch (...)
	{
		OutputDebugString ("Terrain init failed\n");
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void Terrain::vSetParameters (const char *szElevationImage, const char *szTexture, const char *szDetailTexture, 
			real rVertexSpacing, real rElevationScale, int iMaxNumTriangles)
{
	m_strElevationImage = szElevationImage;

	if (szTexture)		
		m_strTexture = szTexture;
	else
		m_strTexture = "";

	if (szDetailTexture)
		m_strDetailTexture = szDetailTexture;
	else
		m_strDetailTexture = "";

	m_rVertexSpacing = rVertexSpacing;
	m_rElevationScale = rElevationScale;
	m_iMaxNumTriangles = iMaxNumTriangles;
}

/****************************************************************************/
/** 
*****************************************************************************/
TerrainLoadError Terrain::eLoad ()
{
	vInitialize ();

	if (m_strElevationImage.length () < 3)
		return tleUnableToSetElevation;

	if (!m_pTerrain)
	{
		OutputDebugString ("Unable to create Demeter terrain engine.\n");
		return tleUnknown;
	}

	try
	{
/*		Demeter::Settings::GetInstance()->SetUseNormals (true);
		m_pTerrain->SetAllElevations (m_strElevationImage.c_str (), (float) m_rVertexSpacing, (float) m_rElevationScale);

		if (m_strTexture.length () > 3)
		{
			if (!m_pTerrain->SetTexture (m_strTexture.c_str(), false))
				return tleUnableToSetCoarseTexture;
		}

		if (m_strDetailTexture.length () > 3)
		{
			if (!m_pTerrain->SetCommonTexture (m_strDetailTexture.c_str()))
				return tleUnableToSetDetailedTexture;
		}
	}
	catch (DemeterException* pEx)
	{
		OutputDebugString (pEx->GetErrorMessage ());
		return tleUnknown;
*/
	}
	catch (...)
	{
		//delete m_pTerrain;
		//m_pTerrain = null;
		return tleUnknown;
	}

	vBuildArrays ();

	return tleSuccess;
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Terrain::fLoaded ()
{
	if (!m_pTerrain)
		return false;

	return true;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Terrain::vRender ()
{
	if (!m_pTerrain)
		return;

/*	glEnable (GL_LIGHTING);
	glShadeModel (GL_SMOOTH);
	glEnable (GL_TEXTURE_2D);

	GLfloat r = 1.0f, g = 1.0f, b = 1.0f, alpha = 1.0f;
	GLfloat ambient = 0.1f, diffuse = 1.0f, specular = 0.5f;
	GLfloat light_ambient[4],light_diffuse[4],light_specular[4];
	light_ambient[0] = r * ambient;
	light_ambient[1] = g * ambient;
	light_ambient[2] = b * ambient;
	light_ambient[3] = alpha;
	light_diffuse[0] = r * diffuse;
	light_diffuse[1] = g * diffuse;
	light_diffuse[2] = b * diffuse;
	light_diffuse[3] = alpha;
	light_specular[0] = r * specular;
	light_specular[1] = g * specular;
	light_specular[2] = b * specular;
	light_specular[3] = alpha;
	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, light_ambient);
	glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, light_diffuse);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, light_specular);
	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 0.5f);

	if (1)
	{
		m_pTerrain->ModelViewMatrixChanged ();
		m_pTerrain->Render ();
	}
	else
	{
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glBegin (GL_TRIANGLES);

		for (int iTriangle = 0; iTriangle < m_iTriangleCount; iTriangle++)
		{
			// Get vertices
			Vertex &Vertex1 = m_pVertexArray[m_pTriangleArray[iTriangle].v1];
			Vertex &Vertex2 = m_pVertexArray[m_pTriangleArray[iTriangle].v2];
			Vertex &Vertex3 = m_pVertexArray[m_pTriangleArray[iTriangle].v3];

			// find the normal
			Vertex Center;
			Center.x = (Vertex1.x + Vertex2.x + Vertex3.x) / 3;
			Center.y = (Vertex1.y + Vertex2.y + Vertex3.y) / 3;
			Center.z = (Vertex1.z + Vertex2.z + Vertex3.z) / 3;

            VectorXYZ vec1 (Vertex1.x - Center.x, Vertex1.y - Center.y, Vertex1.z - Center.z);
			VectorXYZ vec2 (Vertex2.x - Center.x, Vertex2.y - Center.y, Vertex2.z - Center.z);
			VectorXYZ vecNormal = Generic::CrossProduct (vec1, vec2);

			// draw the triangle
		    glNormal3d (vecNormal.x, vecNormal.y, vecNormal.z);
		    glVertex3d (Vertex1.x, Vertex1.y, Vertex1.z);

		    glNormal3d (vecNormal.x, vecNormal.y, vecNormal.z);
		    glVertex3d (Vertex2.x, Vertex2.y, Vertex2.z);

		    glNormal3d (vecNormal.x, vecNormal.y, vecNormal.z);
		    glVertex3d (Vertex3.x, Vertex3.y, Vertex3.z);
		}

		glEnd ();
	}
	*/
}

/****************************************************************************/
/** 
*****************************************************************************/
void Terrain::vSerialize (XML::Stream &Stream)
{
	Stream.vSerialize (&m_strElevationImage, L"ElevationImage");
	Stream.vSerialize (&m_strTexture,        L"TextureCoarse");
	Stream.vSerialize (&m_strDetailTexture,  L"TextureDetailed");
	Stream.vSerialize (&m_rVertexSpacing,    L"HorizontalScale");
	Stream.vSerialize (&m_rElevationScale,   L"VerticalScale");
	Stream.vSerialize (&m_iMaxNumTriangles,  L"TriangleCount");
}

/****************************************************************************/
/** 
*****************************************************************************/
real Terrain::rGetElevation (real rX, real rY)
{
	if (!m_pTerrain)
	{
		GenericUnexpected ("terrain disabled");
		return 0;
	}

//	return m_pTerrain->GetElevation ((float) rX, (float) rY);    
}

/****************************************************************************/
/** 
*****************************************************************************/
void Terrain::vGetDimensions (real &rX, real &rY)
{
	if (!m_pTerrain)
	{
		GenericUnexpected ("terrain disabled");
		return;
	}

//	rX = m_pTerrain->GetWidth ();
//	rY = m_pTerrain->GetHeight ();
}

/****************************************************************************/
/** 
*****************************************************************************/
void Terrain::vGetDimensions (int  &iX, int &iY)
{
	if (!m_pTerrain)
	{
		GenericUnexpected ("terrain disabled");
		return;
	}

//	iX = m_pTerrain->GetWidthVertices ();
//	iY = m_pTerrain->GetHeightVertices ();
}

/*void Terrain::vGetTriangles (Terrain::Vertex **ppVertexArray, int *pVertices,
							 Terrain::Triangle **ppTriangleArray, int *pTriangles)
{
	if (m_pTriangleArray && m_pVertexArray && pVertices && pTriangles)
	{
		*ppVertexArray = m_pVertexArray;
		*ppTriangleArray = m_pTriangleArray;
		*pVertices = m_iVertexCount;
		*pTriangles = m_iTriangleCount;

		return;
	}
}*/

/****************************************************************************/
/** 
*****************************************************************************/
void Terrain::vBuildArrays ()
{
	if (!m_pTerrain)
	{
		GenericUnexpected ("terrain disabled");
		return;
	}
/*
	const Demeter::Vector *pTerrainVertexArray = m_pTerrain->m_pVertices;

	if (!pTerrainVertexArray)
	{
		GenericUnexpected ("terrain disabled");
		return;
	}

	int iMaxX = 0, iMaxY = 0;
	vGetDimensions (iMaxX, iMaxY);

	m_iVertexCount = iMaxX * iMaxY;
	m_iTriangleCount = (iMaxX - 1) * (iMaxY - 1) * 2;

	delete m_pVertexArray;
	delete m_pTriangleArray;

	m_pVertexArray = new Terrain::Vertex[m_iVertexCount];
	m_pTriangleArray = new Terrain::Triangle[m_iTriangleCount];

	for (int iVertex = 0; iVertex < m_iVertexCount; iVertex++)
	{
		m_pVertexArray[iVertex].x = pTerrainVertexArray[iVertex].x;
		m_pVertexArray[iVertex].y = pTerrainVertexArray[iVertex].y;
		m_pVertexArray[iVertex].z = pTerrainVertexArray[iVertex].z;
	}

	int iTriangle = 0;
	int iIndex = 0;

	for (int iY= 0; iY < iMaxY - 1; iY++)
	{
		// Each square in the height map has 4 corners, as follows:
		//
		// 3 4
		// 1 2
		//
		// 1 = x,	y
		// 2 = x+1,	y
		// 3 = x,	y+1
		// 4 = x+1, y+1
		//
		// Triangles are: 1,2,3 and 4,3,2
		//
		for (int iX = 0; iX < iMaxX - 1; iX++)	
		{
			iTriangle = (iY * (iMaxY - 1) * 2) + (iX * 2);

			// X, Y
			iIndex = (iY * iMaxX) + iX;
			m_pTriangleArray[iTriangle].v1 = iIndex;

			// X+1, Y
			iIndex = (iY * iMaxX) + iX + 1;
			m_pTriangleArray[iTriangle].v2 = iIndex;
			
			// X, Y+1
			iIndex = ((iY * iMaxX) + iX) + iMaxX;
			m_pTriangleArray[iTriangle].v3 = iIndex;

			iTriangle++;
	
			// X + 1, Y + 1
			iIndex = ((iY * iMaxX) + iX) + 1 + iMaxX;
			m_pTriangleArray[iTriangle].v1 = iIndex;			
			
			// X, Y + 1
			iIndex = ((iY * iMaxX) + iX) + iMaxX;
			m_pTriangleArray[iTriangle].v2 = iIndex;

			// X+1, Y
			iIndex = (iY * iMaxX) + iX + 1;
			m_pTriangleArray[iTriangle].v3 = iIndex;
		}
	}
	*/
}

/****************************************************************************/
/** 
*****************************************************************************/
real Terrain::rCastShadow (const VectorXYZ &vecObject, const VectorXYZ &vecLightSource, VectorXYZ &vecShadowPoint)
{
	if (!m_pTerrain)
	{
		GenericUnexpected ("terrain disabled");
		return 0;
	}

	VectorXYZ vecDirection = vecObject - vecLightSource;

	float ShadowPointX, ShadowPointY, ShadowPointZ;

/*	real r = m_pTerrain->IntersectRay (
		(float) vecLightSource.x, (float) vecLightSource.y, (float) vecLightSource.z,
		(float) vecDirection.x, (float) vecDirection.y, (float) vecDirection.z,
		ShadowPointX, ShadowPointY, ShadowPointZ);
		*/
	vecShadowPoint.x = ShadowPointX;
	vecShadowPoint.y = ShadowPointY;
	vecShadowPoint.z = ShadowPointZ;

	//return r;
	return 0;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Terrain::vGetNormal (real rX, real rY, VectorXYZ &vecNormal)
{
	if (!m_pTerrain)
	{
		GenericUnexpected ("terrain disabled");
		return;
	}

	float NormalX, NormalY, NormalZ;

//	m_pTerrain->GetNormal ((float) rX, (float) rY, NormalX, NormalY, NormalZ);

	vecNormal.x = NormalX;
	vecNormal.y = NormalY;
	vecNormal.z = NormalZ;
}

// Terrain.cpp ends here ->