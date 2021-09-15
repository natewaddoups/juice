/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: RenderPrimitives.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

Much of this stuff was taken from the ODE sample code, which is BSD-licensed.
*****************************************************************************/

#define VC_EXTRALEAN
#define OEMRESOURCE
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#include <JuiceEngine.h>
#include "RenderInternal.h"
#include <math.h>

using namespace Generic;
using namespace Juice::Render;

/****************************************************************************/
/** 
*****************************************************************************/
void Scene::vDrawBox (VectorXYZ vecDimensions)
{
	real lx = vecDimensions.x * 0.5f;
	real ly = vecDimensions.y * 0.5f;
	real lz = vecDimensions.z * 0.5f;

	real rEdge = 0.2;

	glBegin (GL_TRIANGLES);

	// left
	glNormal3d (-1, rEdge, rEdge);
	glVertex3d (-lx,-ly,-lz); // 1

	glNormal3d (-1, rEdge, -rEdge);
	glVertex3d (-lx,-ly,lz);  // 2

	glNormal3d (-1, -rEdge, rEdge);
	glVertex3d (-lx,ly,-lz);  // 3

	glNormal3d (-1, -rEdge, rEdge);
	glVertex3d (-lx,ly,-lz);  // 3

	glNormal3d (-1, rEdge, -rEdge);
	glVertex3d (-lx,-ly,lz);  // 2

	glNormal3d (-1, -rEdge, -rEdge);
	glVertex3d (-lx,ly,lz);   // 4

	// front
	glNormal3d (rEdge, 1, -rEdge);
	glVertex3d (-lx,ly,lz);   // 4

	glNormal3d (-rEdge, 1, rEdge);
	glVertex3d (lx,ly,-lz);   // 5

	glNormal3d (rEdge, 1, rEdge);
	glVertex3d (-lx,ly,-lz);  // 3

	glNormal3d (-rEdge, 1, rEdge);
	glVertex3d (lx,ly,-lz);   // 5

	glNormal3d (rEdge, 1, -rEdge);
	glVertex3d (-lx,ly,lz);   // 4

	glNormal3d (-rEdge, 1, -rEdge);
	glVertex3d (lx,ly,lz);    // 6

	// right
	glNormal3d (1, -rEdge, rEdge);
	glVertex3d (lx,ly,-lz);   // 5

	glNormal3d (1, -rEdge, -rEdge);
	glVertex3d (lx,ly,lz);    // 6

	glNormal3d (1, rEdge, rEdge);
	glVertex3d (lx,-ly,-lz);  // 7

	glNormal3d (1, rEdge, rEdge);
	glVertex3d (lx,-ly,-lz);  // 7

	glNormal3d (1, -rEdge, -rEdge);
	glVertex3d (lx,ly,lz);    // 6

	glNormal3d (1, rEdge, -rEdge);
	glVertex3d (lx,-ly,lz);   // 8

	// back
	glNormal3d (rEdge, -1, rEdge);
	glVertex3d (-lx, -ly, -lz); // 1

	glNormal3d (-rEdge, -1, rEdge);
	glVertex3d (lx, -ly, -lz);  // 7

	glNormal3d (rEdge, -1, -rEdge);
	glVertex3d (-lx, -ly, lz);  // 2		
	
	glNormal3d (rEdge, -1, -rEdge);
	glVertex3d (-lx, -ly, lz);  // 2

	glNormal3d (-rEdge, -1, rEdge);
	glVertex3d (lx, -ly, -lz);  // 7

	glNormal3d (-rEdge, -1, -rEdge);
	glVertex3d (lx, -ly, lz);   // 8

	// top
	glNormal3d (rEdge, rEdge, 1);
	glVertex3d (-lx,-ly,lz);  // 2

	glNormal3d (-rEdge, -rEdge, 1);
	glVertex3d (lx,ly,lz);    // 6

	glNormal3d (rEdge, -rEdge, 1);
	glVertex3d (-lx,ly,lz);   // 4

	glNormal3d (-rEdge, rEdge, 1);
	glVertex3d (lx,-ly,lz);   // 8

	glNormal3d (-rEdge, -rEdge, 1);
	glVertex3d (lx,ly,lz);    // 6

	glNormal3d (rEdge, rEdge, 1);
	glVertex3d (-lx,-ly,lz);  // 2

	// bottom 
	glNormal3d (rEdge, rEdge, -1);
	glVertex3d (-lx,-ly,-lz); 

	glNormal3d (rEdge, -rEdge, -1);
	glVertex3d (-lx,ly,-lz); 

	glNormal3d (-rEdge, -rEdge, -1);
	glVertex3d (lx,ly,-lz);  

	glNormal3d (-rEdge, rEdge, -1);
	glVertex3d (lx,-ly,-lz); 

	glNormal3d (rEdge, rEdge, -1);
	glVertex3d (-lx,-ly,-lz);

	glNormal3d (-rEdge, -rEdge, -1);
	glVertex3d (lx,ly,-lz);  

	glEnd ();
}



/****************************************************************************/
/** draw a flat-ended cylinder of length l and radius r, aligned along the x 
*** axis (poached from ODE's drawstuff.cpp)
*****************************************************************************/

void Scene::vDrawCylinder (real l, real r)
{
  int i,j;
  float tmp,nx,ny,nz,start_nx,start_ny,a,ca,sa;
  int n = 16;	// number of sides to the cylinder (divisible by 4)

  l *= 0.5;
    
  a = float (Generic::rPi * 2.0) / float (n);
  sa = (float) sin(a);
  ca = (float) cos(a);

  // draw cylinder body
  ny=1; nz=0;		  // normal vector = (0,ny,nz)
  glBegin (GL_TRIANGLE_STRIP);
  for (i=0; i<=n; i++) {
    glNormal3d (ny,nz,0);
    glVertex3d (ny*r,nz*r,l);
    glNormal3d (ny,nz,0);
    glVertex3d (ny*r,nz*r,-l);
    // rotate ny,nz
    tmp = ca*ny - sa*nz;
    nz = sa*ny + ca*nz;
    ny = tmp;
  }
  glEnd();

  // draw first cylinder cap
  start_nx = 0;
  start_ny = 1;

  // this keeps the caps from getting too big and hemispherical
  float r2 = 0; // 0.1f;

  for (j=0; j<(n/4); j++) {
    // get start_n2 = rotated start_n
    float start_nx2 =  ca*start_nx + sa*start_ny;
    float start_ny2 = -sa*start_nx + ca*start_ny;
    // get n=start_n and n2=start_n2
    nx = start_nx; ny = start_ny; nz = 0;
    float nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
    glBegin (GL_TRIANGLE_STRIP);
    for (i=0; i<=n; i++) {
      //glNormal3d (ny2,nz2,nx2);
	  //glNormal3d (ny,nz,nx);
	  glNormal3d (0, 0, 1);
      glVertex3d (ny2*r,nz2*r,l+nx2*r2);
      //glNormal3d (ny,nz,nx);
	  glNormal3d (0, 0, 1);
      glVertex3d (ny*r,nz*r,l+nx*r2);
      // rotate n,n2
      tmp = ca*ny - sa*nz;
      nz = sa*ny + ca*nz;
      ny = tmp;
      tmp = ca*ny2- sa*nz2;
      nz2 = sa*ny2 + ca*nz2;
      ny2 = tmp;
    }
    glEnd();
    start_nx = start_nx2;
    start_ny = start_ny2;
  }

  // draw second cylinder cap
  start_nx = 0;
  start_ny = 1;
  for (j=0; j<(n/4); j++) {
    // get start_n2 = rotated start_n
    float start_nx2 = ca*start_nx - sa*start_ny;
    float start_ny2 = sa*start_nx + ca*start_ny;
    // get n=start_n and n2=start_n2
    nx = start_nx; ny = start_ny; nz = 0;
    float nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
    glBegin (GL_TRIANGLE_STRIP);
    for (i=0; i<=n; i++) {
      //glNormal3d (ny,nz,nx);
	  //glNormal3d (ny2,nz2,nx2);
	  glNormal3d (0, 0, -1);
      glVertex3d (ny*r,nz*r,-l+nx*r2);
      //glNormal3d (ny2,nz2,nx2);
	  glNormal3d (0, 0, -1);
      glVertex3d (ny2*r,nz2*r,-l+nx2*r2);
      // rotate n,n2
      tmp = ca*ny - sa*nz;
      nz = sa*ny + ca*nz;
      ny = tmp;
      tmp = ca*ny2- sa*nz2;
      nz2 = sa*ny2 + ca*nz2;
      ny2 = tmp;
    }
    glEnd();
    start_nx = start_nx2;
    start_ny = start_ny2;
  }
}

/****************************************************************************/
/** draw a capped cylinder of length l and radius r, aligned along the x axis
*** (poached from ODE's drawstuff.cpp)
***
*** if rDomeSize is 0, the caps will be flat; if 1 they will be spherical
*****************************************************************************/
void Scene::vDrawCapsule (real l, real r, real rDomeSize)
{
  int i,j;
  float tmp,nx,ny,nz,start_nx,start_ny,a,ca,sa;
  int n = 16;	// number of sides to the cylinder (divisible by 4)

  l *= 0.5;
    
  a = float (Generic::rPi * 2.0) / float (n);
  sa = (float) sin(a);
  ca = (float) cos(a);

  // draw cylinder body
  ny=1; nz=0;		  // normal vector = (0,ny,nz)
  glBegin (GL_TRIANGLE_STRIP);
  for (i=0; i<=n; i++) {
    glNormal3d (ny,nz,0);
    glVertex3d (ny*r,nz*r,l);
    glNormal3d (ny,nz,0);
    glVertex3d (ny*r,nz*r,-l);
    // rotate ny,nz
    tmp = ca*ny - sa*nz;
    nz = sa*ny + ca*nz;
    ny = tmp;
  }
  glEnd();

  // draw first cylinder cap
  start_nx = 0;
  start_ny = 1;

  // this keeps the caps from getting too big and hemispherical
  float r2 = (float) r * (float) rDomeSize;

  for (j=0; j<(n/4); j++) {
    // get start_n2 = rotated start_n
    float start_nx2 =  ca*start_nx + sa*start_ny;
    float start_ny2 = -sa*start_nx + ca*start_ny;
    // get n=start_n and n2=start_n2
    nx = start_nx; ny = start_ny; nz = 0;
    float nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
    glBegin (GL_TRIANGLE_STRIP);
    for (i=0; i<=n; i++) {
      glNormal3d (ny2,nz2,nx2);
	  //glNormal3d (ny,nz,nx);
	  glVertex3d (ny2*r,nz2*r,l+nx2*r2);
      glNormal3d (ny,nz,nx);
	  glVertex3d (ny*r,nz*r,l+nx*r2);
      // rotate n,n2
      tmp = ca*ny - sa*nz;
      nz = sa*ny + ca*nz;
      ny = tmp;
      tmp = ca*ny2- sa*nz2;
      nz2 = sa*ny2 + ca*nz2;
      ny2 = tmp;
    }
    glEnd();
    start_nx = start_nx2;
    start_ny = start_ny2;
  }

  // draw second cylinder cap
  start_nx = 0;
  start_ny = 1;
  for (j=0; j<(n/4); j++) {
    // get start_n2 = rotated start_n
    float start_nx2 = ca*start_nx - sa*start_ny;
    float start_ny2 = sa*start_nx + ca*start_ny;
    // get n=start_n and n2=start_n2
    nx = start_nx; ny = start_ny; nz = 0;
    float nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
    glBegin (GL_TRIANGLE_STRIP);
    for (i=0; i<=n; i++) {
      glNormal3d (ny,nz,nx);
	  //glNormal3d (ny2,nz2,nx2);
	  glVertex3d (ny*r,nz*r,-l+nx*r2);
      glNormal3d (ny2,nz2,nx2);
	  glVertex3d (ny2*r,nz2*r,-l+nx2*r2);
      // rotate n,n2
      tmp = ca*ny - sa*nz;
      nz = sa*ny + ca*nz;
      ny = tmp;
      tmp = ca*ny2- sa*nz2;
      nz2 = sa*ny2 + ca*nz2;
      ny2 = tmp;
    }
    glEnd();
    start_nx = start_nx2;
    start_ny = start_ny2;
  }
}


// Shamelessly poached from ODE's drawstuff...
//
// This is recursively subdivides a triangular area (vertices p1,p2,p3) into
// smaller triangles, and then draws the triangles. All triangle vertices are
// normalized to a distance of 1.0 from the origin (p1,p2,p3 are assumed
// to be already normalized). Note this is not super-fast because it draws
// triangles rather than triangle strips.

static void drawPatch (float p1[3], float p2[3], float p3[3], int level)
{
  int i;
  if (level > 0) {
    float q1[3],q2[3],q3[3];		 // sub-vertices
    for (i=0; i<3; i++) {
      q1[i] = 0.5f*(p1[i]+p2[i]);
      q2[i] = 0.5f*(p2[i]+p3[i]);
      q3[i] = 0.5f*(p3[i]+p1[i]);
    }
    float length1 = (float)(1.0/sqrt(q1[0]*q1[0]+q1[1]*q1[1]+q1[2]*q1[2]));
    float length2 = (float)(1.0/sqrt(q2[0]*q2[0]+q2[1]*q2[1]+q2[2]*q2[2]));
    float length3 = (float)(1.0/sqrt(q3[0]*q3[0]+q3[1]*q3[1]+q3[2]*q3[2]));
    for (i=0; i<3; i++) {
      q1[i] *= length1;
      q2[i] *= length2;
      q3[i] *= length3;
    }
    drawPatch (p1,q1,q3,level-1);
    drawPatch (q1,p2,q2,level-1);
    drawPatch (q1,q2,q3,level-1);
    drawPatch (q3,q2,p3,level-1);
  }
  else {
    glNormal3f (p1[0],p1[1],p1[2]);
    glVertex3f (p1[0],p1[1],p1[2]);
    glNormal3f (p2[0],p2[1],p2[2]);
    glVertex3f (p2[0],p2[1],p2[2]);
    glNormal3f (p3[0],p3[1],p3[2]);
    glVertex3f (p3[0],p3[1],p3[2]);
  }
}

class Quadric
{
	GLUquadricObj *m_pQ;

public:

	Quadric () : m_pQ (0)
	{
		m_pQ = gluNewQuadric ();
	}

	~Quadric ()
	{
		gluDeleteQuadric (m_pQ);
	}

	operator GLUquadric*() { return m_pQ; }
};

Quadric g_Quadric;

void Scene::vDrawSphere (real rRadius)
{
	gluSphere (g_Quadric, rRadius, 10, 10);
}

// RenderPrimitives.cpp ends here ->