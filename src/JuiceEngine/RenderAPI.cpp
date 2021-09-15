/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: RenderAPI.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#define VC_EXTRALEAN
#define OEMRESOURCE
#include <windows.h>
#include <gl/gl.h>

#include <JuiceEngine.h>
#include "RenderInternal.h"

using namespace Generic;
using namespace Juice::Render;

TCHAR *Viewport::m_szWindowClass = "Juice.3DView";

/****************************************************************************/
/** 
*****************************************************************************/
Scene::Scene () :
	m_pGroundTexture (0),
	m_pSkyTexture (0),
	m_pWoodTexture (0),
	m_fShowCursor (false),
	m_vecCursor (0, 0, 0),
	m_vecCursorTail (0, 0, 0),
	m_vecLookAt (0, 0, 0),
	m_fSelectionRect (false),
	m_vecLightPosition (-1, 1, 1)
{
}

/****************************************************************************/
/** 
*****************************************************************************/
Scene::~Scene ()
{
	delete m_pGroundTexture;
	delete m_pSkyTexture;
	delete m_pWoodTexture;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vSetModel (Juice::Model *pModel)
{
	m_pModel = pModel;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vSetCursorPosition (const VectorXYZ &vecCursor)
{
	m_vecCursor = vecCursor;
}

void Scene::vGetCursorPosition (VectorXYZ &vecCursor)
{
	vecCursor = m_vecCursor;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vSetCursorTail (const VectorXYZ &vecCursorTail)
{
	m_vecCursorTail = vecCursorTail;
}

//void Scene::vGetCursorTail (VectorXYZ &vecCursorTail)
//{
//	vecCursorTail = m_vecCursorTail;
//}

/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vShowCursor (bool fShowCursor)
{
	m_fShowCursor = fShowCursor;
}

bool Scene::fShowCursor ()
{
	return m_fShowCursor;
}

/****************************************************************************/
/** To be called once per process - register the 3D view window class
*****************************************************************************/
bool Scene::fInitialize ()
{
	// Register the viewport's window class
	WNDCLASS wc;

	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof (void*);
	wc.hbrBackground = (HBRUSH) GetStockObject (GRAY_BRUSH);
	wc.hCursor = (HCURSOR) LoadImage (0, MAKEINTRESOURCE (OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE);
	wc.hIcon = NULL;
	wc.hInstance = 0;
	wc.lpfnWndProc = Viewport::WndProc;
	wc.lpszClassName = Viewport::m_szWindowClass;
	wc.lpszMenuName = NULL;
	wc.style = 0;

	if (!RegisterClass (&wc))
		return false;

	return true;
}

/****************************************************************************/
/** 
*****************************************************************************/
Viewport* Scene::CreateViewport (Generic::WindowType ParentWindow)
{
	GenericTrace ("Scene::CreateViewport" << std::endl);
	Viewport* pNew = new Viewport (ParentWindow, this);

	if (m_Viewports.size () > 0)
	{
		Viewport *pExisting = m_Viewports.front ();
		pNew->vShareScene (pExisting);
	}
	else
	{
		if (!wglMakeCurrent (pNew->m_hRenderDC, pNew->m_hRenderRC))
			OutputDebugString ("Scene::CreateViewport: Unable to set render context\n");

		glPushAttrib (GL_ALL_ATTRIB_BITS);

		// Load textures
		if (!fLoadTextures ())
		{
			OutputDebugString ("Scene::CreateViewport: Unable to load textures\n");
			delete pNew;
			return null;
		}

		vCreateSharedLists ();

		glPopAttrib ();

		pGetApplication()->vTerrainInitialize ();

		if (!wglMakeCurrent (NULL, NULL))
			OutputDebugString ("Scene::CreateViewport: Unable to release render context\n");
	}


	m_Viewports.push_front (pNew);

	return  pNew;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vDestroyViewport (Viewport *pViewport)
{
	m_Viewports.remove (pViewport);
	delete pViewport;
}


/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vSetLookAt (VectorXYZ &vecLookAt)
{
	m_vecLookAt = vecLookAt; 

	if (pGetApplication ()->eGetAppMode () == Juice::amDesign)
		vRenderAllViewports ();
}


/****************************************************************************/
/** 
*****************************************************************************/
bool Scene::fLoadTextures ()
{
	char szTextureDir[MAX_PATH];
	char szTextureFile[MAX_PATH];

	// get path to installation directory
	GetModuleFileName (null, szTextureDir, MAX_PATH);
	char *pcChopStringHere = strrchr (szTextureDir, '\\');
	*pcChopStringHere = 0;

	// add texture folder name
	strcat (szTextureDir, "\\textures\\");

	// load ground texture
	strcpy (szTextureFile, szTextureDir);
	strcat (szTextureFile, "ground.ppm");

	delete m_pGroundTexture;

	m_pGroundTexture = new Texture (szTextureFile);

	return true;
}

/****************************************************************************/
/** Call this to update all viewports
*****************************************************************************/

void Scene::vRenderAllViewports ()
{
	// TODO: Animate the world

	// Create a displaylist for the current background, and for the current model
	if (m_Viewports.size () > 0)
	{
		Viewport *pViewport = m_Viewports.front ();

		if (!pViewport || !pViewport->m_hRenderDC || !pViewport->m_hRenderRC)
		{
			OutputDebugString ("Bad context in vRenderAllViewports\n");
			return;
		}

		if (!wglMakeCurrent (pViewport->m_hRenderDC, pViewport->m_hRenderRC))
		{
			OutputDebugString ("Scene::CreateViewport: Unable to set render context\n");
			return;
		}

		glPushAttrib (GL_ALL_ATTRIB_BITS);

		// Cage lists are recompiled once per frame, then shared across all viewports
		// (they need to be recompiled because they change color to indicate the current drag plane)
		glNewList (Juice::Render::dlSetup, GL_COMPILE);

			// basic stuff
			glEnable(GL_DEPTH_TEST);

			//glEnable (GL_CULL_FACE);
			//glCullFace (GL_BACK);
			//glFrontFace (GL_CCW);

			// Clear everything
			Color &clrBackground = m_pApplication->m_Preferences.m_clr3DBackground;
			glClearColor ((GLclampf) clrBackground.r, (GLclampf) clrBackground.g, (GLclampf) clrBackground.b, (GLclampf) clrBackground.a);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Prepare to draw
			glMatrixMode (GL_MODELVIEW);

		glEndList ();

		glNewList (Juice::Render::dlCage, GL_COMPILE);
			vDrawCage ();	
		glEndList ();

		glNewList (Juice::Render::dlCageFloor, GL_COMPILE);
			vDrawCageFloor ();	
		glEndList ();

		glNewList (Juice::Render::dlGround, GL_COMPILE);
			vDrawGround ();	
		glEndList ();

		glNewList (Juice::Render::dlModel, GL_COMPILE);

			glEnable (GL_LIGHTING);
			glShadeModel (GL_SMOOTH);

			GLfloat r = 0.5f, g = 0.5f, b = 0.5f, alpha = 1.0f;
			GLfloat light_ambient[4],light_diffuse[4],light_specular[4];
			light_ambient[0] = r*0.5f;
			light_ambient[1] = g*0.5f;
			light_ambient[2] = b*0.5f;
			light_ambient[3] = alpha;
			light_diffuse[0] = r*0.7f;
			light_diffuse[1] = g*0.7f;
			light_diffuse[2] = b*0.7f;
			light_diffuse[3] = alpha;
			light_specular[0] = r*0.2f;
			light_specular[1] = g*0.2f;
			light_specular[2] = b*0.2f;
			light_specular[3] = alpha;
			glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, light_ambient);
			glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, light_diffuse);
			glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, light_specular);
			glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 5.0f);

			m_pModel->vRender (this);

		glEndList ();

		glPopAttrib ();

		if (!wglMakeCurrent (NULL, NULL))
			OutputDebugString ("Scene::CreateViewport: Unable to release render context\n");
	}

	// Update all viewports
	std::list<Viewport*>::iterator iter;
	for (iter = m_Viewports.begin (); iter != m_Viewports.end (); iter++)
	{
		if (*iter)
			(*iter)->vRender ();
	}
}

/****************************************************************************/
/** 
*****************************************************************************/

void DrawNet (real size, GLint LinesX, GLint LinesY)
{
	glBegin(GL_LINES);

	real rLinesX = LinesX;
	real rLinesY = LinesY;

	for (real xc = 0; xc < LinesX + 1; xc++)
	{
		glVertex3d(	(((-size / 2.0) + xc) / rLinesX*size),
					(size / 2.0),
					0.0);
		glVertex3d(	(((-size / 2.0) + xc) / rLinesX*size),
					(size / -2.0),
					0.0);
	}
	for (int yc = 0; yc < LinesX + 1; yc++)
	{
		glVertex3d(	(size / 2.0),
					(((-size / 2.0) + yc) / rLinesY*size),
					0.0);
		glVertex3d(	(size / -2.0),
					(((-size / 2.0) + yc) / rLinesY*size),
					0.0);
	}

	glEnd();
}

void DrawNets (real Size, GLint LinesX, GLint LinesY)
{
	glTranslated (0.0, 0.0f, -Size / 2.0);
	DrawNet (Size, LinesX, LinesY);
	glTranslated (0.0, 0.0f, +Size);
	DrawNet (Size, LinesX, LinesY);
	glTranslated (0.0, 0.0f, -Size / 2.0);
}

void Scene::vDrawCage ()
{
	int iSize = (int) m_pApplication->m_Preferences.m_rCageSize;
	Color &clrNormal = m_pApplication->m_Preferences.m_clr3DBackgroundMesh;
	Color &clrHighlight = m_pApplication->m_Preferences.m_clr3DBackgroundMeshHighlight;

	glDisable (GL_LIGHTING);
	glShadeModel (GL_SMOOTH);

	glPushMatrix ();

	glTranslated (0.5, 0.5, 0.0);

	// XY plane
	if (Juice::dpXY == m_pApplication->m_eDragPlane)
		vSetLine (clrHighlight);
	else
		vSetLine (clrNormal);

	// move origin to ceiling
	glTranslated (0.0, 0.0f, +iSize);

	// draw ceiling
	DrawNet (iSize, iSize, iSize);

	// move origin to center of cage
	glTranslated (0.0, 0.0f, -(real)iSize / 2.0);

	// XZ plane
	if (Juice::dpXZ == m_pApplication->m_eDragPlane)
		vSetLine (clrHighlight);
	else
		vSetLine (clrNormal);

	glRotatef (90, 1.0, 0.0, 0.0);
	DrawNets (iSize, iSize, iSize);
	glRotatef (-90, 1.0, 0.0, 0.0);

	// YZ plane
	if (Juice::dpYZ == m_pApplication->m_eDragPlane)
		vSetLine (clrHighlight);
	else
		vSetLine (clrNormal);

	glRotated (90, 0.0, 1.0, 0.0);
	DrawNets (iSize, iSize, iSize);
	glRotated (-90, 0.0, 1.0, 0.0);


	glPopMatrix ();
}

void Scene::vDrawCageFloor ()
{
	int iSize = (int) m_pApplication->m_Preferences.m_rCageSize;
	Color &clrNormal = m_pApplication->m_Preferences.m_clr3DBackgroundMesh;
	Color &clrHighlight = m_pApplication->m_Preferences.m_clr3DBackgroundMeshHighlight;

	glDisable (GL_LIGHTING);

	glPushMatrix ();

	glTranslated (0.5, 0.5, 0.0);

	// get color for XY plane
	if (Juice::dpXY == m_pApplication->m_eDragPlane)
		vSetLine (clrHighlight);
	else
		vSetLine (clrNormal);

	// draw floor
	DrawNet (iSize, iSize, iSize);

	glPopMatrix ();
}

void Scene::vDrawGround ()
{
	int iSize = (int) m_pApplication->m_Preferences.m_rCageSize;
	Color &clrNormal = m_pApplication->m_Preferences.m_clr3DBackgroundMesh;
	Color &clrHighlight = m_pApplication->m_Preferences.m_clr3DBackgroundMeshHighlight;


	if (1)
	{
		//glPushMatrix ();
		//glLoadIdentity ();

		vSetLine (clrNormal);

		glDisable (GL_LIGHTING);
		glShadeModel (GL_FLAT);

		glEnable (GL_CULL_FACE);
		glCullFace (GL_BACK);
		glFrontFace (GL_CCW);

		glEnable (GL_TEXTURE_2D);
		m_pGroundTexture->vBind (false);

		glEnable (GL_TEXTURE_GEN_S);
		glEnable (GL_TEXTURE_GEN_T);
		glTexGeni (GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
		glTexGeni (GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
		
		GLfloat s_params[4] = {1.0f,1.0f,0.0f,1};
		GLfloat t_params[4] = {0.817f,-0.817f,0.817f,1};
		glTexGenfv (GL_S,GL_OBJECT_PLANE,s_params);
		glTexGenfv (GL_T,GL_OBJECT_PLANE,t_params);

		const float ground_scale = 100.0f/1.0f;	// ground texture scale (1/size)
		const float ground_ofsx = 0.5;		// offset of ground texture
		const float ground_ofsy = 0.5;

		const float gsize = 1000.0f;
		const float rGroundOffset = -0.1f; // ... polygon offsetting doesn't work well

		glBegin (GL_QUADS);
		glNormal3f (0,0,1);
		glTexCoord2f (-gsize*ground_scale + ground_ofsx,
				-gsize*ground_scale + ground_ofsy);
		glVertex3f (-gsize,-gsize, rGroundOffset);
		glTexCoord2f (gsize*ground_scale + ground_ofsx,
				-gsize*ground_scale + ground_ofsy);
		glVertex3f (gsize,-gsize, rGroundOffset);
		glTexCoord2f (gsize*ground_scale + ground_ofsx,
				gsize*ground_scale + ground_ofsy);
		glVertex3f (gsize,gsize, rGroundOffset);
		glTexCoord2f (-gsize*ground_scale + ground_ofsx,
				gsize*ground_scale + ground_ofsy);
		glVertex3f (-gsize,gsize, rGroundOffset);
		glEnd();


		glDisable (GL_CULL_FACE);
		glDisable (GL_TEXTURE_2D);

		//glPopMatrix ();
	}
	else
	{
		glDisable (GL_LIGHTING);

		glPushMatrix ();

		// get color for XY plane
		if (Juice::dpXY == m_pApplication->m_eDragPlane)
			vSetLine (clrHighlight);
		else
			vSetLine (clrNormal);

		// make the floor huge
		int iFloorSize = (iSize * 10);

		// FIXFIX: the fudge factor is stupid and the grid lines still don't match quite right.
		DrawNet ((iSize * 10) + 10, iFloorSize, iFloorSize);

		glPopMatrix ();
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vDrawGroundAndSky ()
{
}

/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vDrawCursor (VectorXYZ &vecCursor)
{
	glPushMatrix ();
	glTranslated (vecCursor.x, vecCursor.y, vecCursor.z);
	glCallList (Juice::Render::dlCursor);
	glPopMatrix ();
}

/****************************************************************************/
/** create display lists that don't change much - lights, ground, etc
*****************************************************************************/
void Scene::vCreateSharedLists ()
{	
	GenericTrace ("Scene::vCreateSharedLists" << std::endl);

	bool fBoxTextures, fGroundTextures;
	fBoxTextures = true;
	fGroundTextures = true;

	// Lighting
	glNewList (Juice::Render::dlLighting, GL_COMPILE);
		
		float  color[]={1.f,1.f,1.f,1.f};// RGBA color spec
		glLightfv(GL_LIGHT1,GL_DIFFUSE,color);

		float  ambient[]={.5f, .5f, .5f, 1.f};// RGBA color spec
		glLightfv(GL_LIGHT1,GL_AMBIENT,ambient);
		
		float pos[]={ 10.0f, 20.0f, 20.0f, 0.0f};
		glLightfv(GL_LIGHT1,GL_POSITION,pos);
		vGetError ();

		glEnable (GL_LIGHT1);
	glEndList ();

	// Arrows
	glNewList (Juice::Render::dlSimpleScene, GL_COMPILE);
		glDisable (GL_LIGHTING);
		glShadeModel (GL_SMOOTH);

		glBegin(GL_LINES);
			// yellow x axis arrow
			glColor3f(1.f, 1.f, 0.f);
			glVertex3f(0.0f,0.0f,0.0f);
			glVertex3f(1.0f,0.0f,0.0f);
			glVertex3f(1.0f,0.0f,0.0f);
			glVertex3f(0.9f,0.1f,0.0f);
			glVertex3f(1.0f,0.0f,0.0f);
			glVertex3f(0.9f,-0.1f,0.0f);
			// red y axis arrow
			glColor3f(1.f, 0.f, 0.f);
			glVertex3f(0.0f,0.0f,0.0f);
			glVertex3f(0.0f,1.0f,0.0f);
			glVertex3f(0.0f,1.0f,0.0f);
			glVertex3f(0.1f,0.9f,0.0f);
			glVertex3f(0.0f,1.0f,0.0f);
			glVertex3f(-0.1f,0.9f,0.0f);
			// white z axis arrow
			glColor3f(1.f,1.f,1.f);
			glVertex3f(0.0f,0.0f,0.0f);
			glVertex3f(0.0f,0.0f,1.0f);
			glVertex3f(0.0f,0.0f,1.0f);
			glVertex3f(0.0f,0.1f,0.9f);
			glVertex3f(0.0f,0.0f,1.0f);
			glVertex3f(0.0f,-0.1f,0.9f);
		glEnd();
	glEndList ();

	// Cursor
	glNewList (Juice::Render::dlCursor, GL_COMPILE);
		glDisable (GL_LIGHTING);
		glShadeModel (GL_SMOOTH);

		glBegin(GL_LINES);
			// yellow x axis 
			glColor3f(1.f, 1.f, 0.f);
			glVertex3f(-1.0f,0.0f,0.0f);
			glVertex3f(1.0f,0.0f,0.0f);

			// red y axis arrow
			glColor3f(1.f, 0.f, 0.f);
			glVertex3f(0.0f,-1.0f,0.0f);
			glVertex3f(0.0f,1.0f,0.0f);

			// white z axis arrow
			glColor3f(1.f,1.f,1.f);
			glVertex3f(0.0f,0.0f,-1.0f);
			glVertex3f(0.0f,0.0f,1.0f);
		glEnd();

		glEnable (GL_LIGHTING);
	glEndList ();

/*  These need to be recompiled once per frame because they change color to
    indicate the current drag plane.

	glNewList (Juice::Render::dlCage, GL_COMPILE);
		vDrawCage ();	
	glEndList ();

	glNewList (Juice::Render::dlCageFloor, GL_COMPILE);
		vDrawCageFloor ();	
	glEndList ();

	glNewList (Juice::Render::dlGround, GL_COMPILE);
		vDrawGround ();	
	glEndList ();
*/
}

bool Scene::fSelectionRect ()
{
	return m_fSelectionRect;
}

void Scene::vSetSelectionRect (VectorXYZ &vecOrigin, VectorXYZ &vecBase1, VectorXYZ &vecBase2, VectorXYZ &vecBase3, VectorXYZ &vecBase4)
{
	m_fSelectionRect = true;

	real rDepth = 200;

	vecBase1 = ((vecBase1 - vecOrigin) * rDepth) + vecOrigin;
	vecBase2 = ((vecBase2 - vecOrigin) * rDepth) + vecOrigin;
	vecBase3 = ((vecBase3 - vecOrigin) * rDepth) + vecOrigin;
	vecBase4 = ((vecBase4 - vecOrigin) * rDepth) + vecOrigin;

	m_vecSelectionOrigin = vecOrigin;

	m_vecSelectionBase1 = vecBase1;
	m_vecSelectionBase2 = vecBase2;
	m_vecSelectionBase3 = vecBase3;
	m_vecSelectionBase4 = vecBase4;
}

void Scene::vCancelSelectionRect ()
{
	m_fSelectionRect = false;
}

void Scene::vDrawSelectionRect ()
{
	glDisable (GL_LIGHTING);
	glBegin (GL_LINES);

	glColor3f (1.0, 1.0, 1.0);

	glVertex3d (m_vecSelectionOrigin.x, m_vecSelectionOrigin.y, m_vecSelectionOrigin.z);
	glVertex3d (m_vecSelectionBase1.x, m_vecSelectionBase1.y, m_vecSelectionBase1.z);

	glVertex3d (m_vecSelectionOrigin.x, m_vecSelectionOrigin.y, m_vecSelectionOrigin.z);
	glVertex3d (m_vecSelectionBase2.x, m_vecSelectionBase2.y, m_vecSelectionBase2.z);

	glVertex3d (m_vecSelectionOrigin.x, m_vecSelectionOrigin.y, m_vecSelectionOrigin.z);
	glVertex3d (m_vecSelectionBase3.x, m_vecSelectionBase3.y, m_vecSelectionBase3.z);

	glVertex3d (m_vecSelectionOrigin.x, m_vecSelectionOrigin.y, m_vecSelectionOrigin.z);
	glVertex3d (m_vecSelectionBase4.x, m_vecSelectionBase4.y, m_vecSelectionBase4.z);

	// left edge 
	glVertex3d (m_vecSelectionBase1.x, m_vecSelectionBase1.y, m_vecSelectionBase1.z);
	glVertex3d (m_vecSelectionBase3.x, m_vecSelectionBase3.y, m_vecSelectionBase3.z);

	// right edge
	glVertex3d (m_vecSelectionBase2.x, m_vecSelectionBase2.y, m_vecSelectionBase2.z);
	glVertex3d (m_vecSelectionBase4.x, m_vecSelectionBase4.y, m_vecSelectionBase4.z);

	// top edge
	glVertex3d (m_vecSelectionBase1.x, m_vecSelectionBase1.y, m_vecSelectionBase1.z);
	glVertex3d (m_vecSelectionBase2.x, m_vecSelectionBase2.y, m_vecSelectionBase2.z);

	// bottom edge
	glVertex3d (m_vecSelectionBase3.x, m_vecSelectionBase3.y, m_vecSelectionBase3.z);
	glVertex3d (m_vecSelectionBase4.x, m_vecSelectionBase4.y, m_vecSelectionBase4.z);	

	glEnd ();

	glEnable (GL_LIGHTING);
}

/*	// quad polygons that match the cage size... seemed like a neat idea, but probably isn't...

	real rHalfSize = iSize / 2;

	vSetLine (m_pApplication->m_Preferences.m_clr3DBackground);

	glPushMatrix ();
	glTranslated (0.0, 0.0, -iSize / 2.0);

	glBegin (GL_QUADS);
	
	// front
		// top left
		glVertex3d (-rHalfSize, rHalfSize, iSize);
		// top right
		glVertex3d (rHalfSize, rHalfSize, iSize);
		// bottom right
		glVertex3d (rHalfSize, rHalfSize, 0);
		// bottom left
		glVertex3d (-rHalfSize, rHalfSize, 0);

	// back
		// top left
		glVertex3d (-rHalfSize, -rHalfSize, iSize);
		// top right
		glVertex3d (rHalfSize, -rHalfSize, iSize);
		// bottom right
		glVertex3d (rHalfSize, -rHalfSize, 0);
		// bottom left
		glVertex3d (-rHalfSize, -rHalfSize, 0);

	// left
		// top back
		glVertex3d (-rHalfSize, -rHalfSize, iSize);
		// top front
		glVertex3d (-rHalfSize, rHalfSize, iSize);
		// bottom front
		glVertex3d (-rHalfSize, rHalfSize, 0);
		// bottom back
		glVertex3d (-rHalfSize, -rHalfSize, 0);

	// right
		// top back
		glVertex3d (rHalfSize, -rHalfSize, iSize);
		// top front
		glVertex3d (rHalfSize, rHalfSize, iSize);
		// bottom front
		glVertex3d (rHalfSize, rHalfSize, 0);
		// bottom back
		glVertex3d (rHalfSize, -rHalfSize, 0);
	glEnd ();
	glPopMatrix ();
*/

/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vPushMatrix (VectorXYZ &vecPosition, VectorYPR &vecOrientation)
{
	glPushMatrix();
	glTranslated (vecPosition.x, vecPosition.y, vecPosition.z);
	glRotated (vecOrientation.yaw, 0, 0, 1);
	glRotated (vecOrientation.pitch, 1, 0, 0);
	glRotated (vecOrientation.roll, 0, 1, 0);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vPushMatrix (const Matrix4x4 &mxRotation)
{
	glPushMatrix();

	GLdouble matrix[16];
	for (int i = 0; i < 16; i++)
		matrix[i] = mxRotation[i];

	glMultMatrixd (matrix);
}

/****************************************************************************/
/** 
*****************************************************************************/
bool Scene::fPushShadowMatrix  (VectorXYZ &vecObject)
{
	if (m_pApplication->fTerrainAvailable () && m_pApplication->fTerrainEnabled () && (m_pApplication->eGetAppMode () != amDesign))
	{
		Juice::Terrain *pTerrain = m_pApplication->pGetTerrain ();

		if (pTerrain)
		{
			VectorXYZ vecShadow, vecNormal;

			VectorXYZ vecLightSource = m_vecLightPosition * 10000;

			// cast ray from light through object to terrain
			real rDistanceToShadow = pTerrain->rCastShadow (vecObject, vecLightSource, vecShadow);

			// bad things happen if, for example, the object is on the shady side of a hill
			if ((rDistanceToShadow < 0) || (vecShadow.z > vecObject.z))
				return false;

			// get terrain normal at shadow point
			pTerrain->vGetNormal (vecShadow.x, vecShadow.y, vecNormal);

			// http://www.sgi.com/software/opengl/advanced96/node46.html

			real D = -DotProduct (vecNormal, vecShadow);
			real ground[] = { vecNormal.x, vecNormal.y, vecNormal.z, D };
			real light[] = { vecLightSource.x, vecLightSource.y, vecLightSource.z, 0 };

			real  dot;
			real  shadowMat[4][4];

			dot = ground[0] * light[0] +
					ground[1] * light[1] +
					ground[2] * light[2] +
					ground[3] * light[3];

			shadowMat[0][0] = dot - light[0] * ground[0];
			shadowMat[1][0] = 0.0 - light[0] * ground[1];
			shadowMat[2][0] = 0.0 - light[0] * ground[2];
			shadowMat[3][0] = 0.0 - light[0] * ground[3];

			shadowMat[0][1] = 0.0 - light[1] * ground[0];
			shadowMat[1][1] = dot - light[1] * ground[1];
			shadowMat[2][1] = 0.0 - light[1] * ground[2];
			shadowMat[3][1] = 0.0 - light[1] * ground[3];

			shadowMat[0][2] = 0.0 - light[2] * ground[0];
			shadowMat[1][2] = 0.0 - light[2] * ground[1];
			shadowMat[2][2] = dot - light[2] * ground[2];
			shadowMat[3][2] = 0.0 - light[2] * ground[3];

			shadowMat[0][3] = 0.0 - light[3] * ground[0];
			shadowMat[1][3] = 0.0 - light[3] * ground[1];
			shadowMat[2][3] = 0.0 - light[3] * ground[2];
			shadowMat[3][3] = dot - light[3] * ground[3];

			glPushMatrix();

			glMultMatrixd((const GLdouble*)shadowMat);
		}
	}
	else
	{
		GLdouble matrix[16];
		for (int i=0; i<16; i++) matrix[i] = 0;
		matrix[0]=1;
		matrix[5]=1;
		matrix[8]=-m_vecLightPosition.x;
		matrix[9]=-m_vecLightPosition.y;
		matrix[15]=1;
		glPushMatrix();

		glMultMatrixd (matrix);
	}

	return true;
}

// RenderAPI.cpp ends here ->