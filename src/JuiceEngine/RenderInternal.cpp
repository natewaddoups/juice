/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: RenderInternal.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#define VC_EXTRALEAN
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#include <JuiceEngine.h>
#include <GenericError.h>
#include "RenderInternal.h"

using namespace Juice;
using namespace Juice::Render;

/*****************************************************************************
*****************************************************************************/

typedef unsigned char byte;

/****************************************************************************/
/** Some helper functions
Shamelessly plagiarized from the drawstuff.cpp used in ODE's sample apps

TODO: make these throw instead

*****************************************************************************/
static void errorBox (char *title, char *msg, va_list ap)
{
#ifdef _DEBUG
  char s[1000];
  vsprintf (s,msg,ap);
  MessageBox (0,s,title,MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
#endif
}


static void dsWarning (char *msg, ...)
{
#ifdef _DEBUG
  va_list ap;
  va_start (ap,msg);
  errorBox ("Warning",msg,ap);
#endif
}


extern "C" void dsError (char *msg, ...)
{
#ifdef _DEBUG
  va_list ap;
  va_start (ap,msg);
  errorBox ("Error",msg,ap);
  //exit (1);
#endif
}


extern "C" void dsDebug (char *msg, ...)
{
#ifdef _DEBUG
  va_list ap;
  va_start (ap,msg);
  errorBox ("INTERNAL ERROR",msg,ap);
  // *((char *)0) = 0;	 ... commit SEGVicide ?
  //abort();
  //exit (1);	  // should never get here, but just in case...
#endif
}


extern "C" void dsPrint (char *msg, ...)
{
  va_list ap;
  va_start (ap,msg);
  vprintf (msg,ap);
}

/****************************************************************************/
/** Image 
Shamelessly plagiarized from the drawstuff.cpp used in ODE's sample apps
*****************************************************************************/

// skip over whitespace and comments in a stream.

static void skipWhiteSpace (char *filename, FILE *f)
{
	int c,d;
	for(;;)
	{
		c = fgetc(f);
		if (c==EOF)
			dsError ("unexpected end of file in \"%s\"",filename);

		// skip comments
		if (c == '#') 
		{
			do 
			{
				d = fgetc(f);
				if (d==EOF)
					dsError ("unexpected end of file in \"%s\"",filename);
			} while (d != '\n');
			continue;
		}

		if (c > ' ') 
		{
			ungetc (c,f);
			return;
		}
	}
}

// read a number from a stream, this return 0 if there is none (that's okay
// because 0 is a bad value for all PPM numbers anyway).

static int readNumber (char *filename, FILE *f)
{
  int c,n=0;
  for(;;) {
    c = fgetc(f);
    if (c==EOF) dsError ("unexpected end of file in \"%s\"",filename);
    if (c >= '0' && c <= '9') n = n*10 + (c - '0');
    else {
      ungetc (c,f);
      return n;
    }
  }
}

Image::Image (char *filename) : image_width (0), image_height (0), image_data (0)
{
  FILE *f = fopen (filename,"rb");
  if (!f) 
  {
	  // dsError ("Can't open image file `%s'",filename);
	  return;
  }

  // read in header
  if (fgetc(f) != 'P' || fgetc(f) != '6')
    dsError ("image file \"%s\" is not a binary PPM (no P6 header)",filename);
  skipWhiteSpace (filename,f);

  // read in image parameters
  image_width = readNumber (filename,f);
  skipWhiteSpace (filename,f);
  image_height = readNumber (filename,f);
  skipWhiteSpace (filename,f);
  int max_value = readNumber (filename,f);

  // check values
  if (image_width < 1 || image_height < 1)
    dsError ("bad image file \"%s\"",filename);
  if (max_value != 255)
    dsError ("image file \"%s\" must have color range of 255",filename);

  // read either nothing, LF (10), or CR,LF (13,10)
  int c = fgetc(f);
  if (c == 10) {
    // LF
  }
  else if (c == 13) {
    // CR
    c = fgetc(f);
    if (c != 10) ungetc (c,f);
  }
  else ungetc (c,f);

  // read in rest of data
  image_data = new byte [image_width*image_height*3];
  if (fread (image_data,image_width*image_height*3,1,f) != 1)
    dsError ("Can not read data from image file `%s'",filename);
  fclose (f);
}


Image::~Image()
{
  delete[] image_data;
}

/****************************************************************************/
/** Texture
Shamelessly plagiarized from the drawstuff.cpp used in ODE's sample apps
*****************************************************************************/

Texture::Texture (char *filename) : name (0), image (0)
{
  image = new Image (filename);
  glGenTextures (1,&name);
  vGetError ();

  glBindTexture (GL_TEXTURE_2D,name);
  vGetError ();

  // set pixel unpacking mode
  glPixelStorei (GL_UNPACK_SWAP_BYTES, 0);
  glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei (GL_UNPACK_SKIP_ROWS, 0);
  glPixelStorei (GL_UNPACK_SKIP_PIXELS, 0);

  // glTexImage2D (GL_TEXTURE_2D, 0, 3, image->width(), image->height(), 0,
  //		   GL_RGB, GL_UNSIGNED_BYTE, image->data());
  gluBuild2DMipmaps (GL_TEXTURE_2D, 3, image->width(), image->height(),
		     GL_RGB, GL_UNSIGNED_BYTE, image->data());

  // set texture parameters - will these also be bound to the texture???
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		   GL_LINEAR_MIPMAP_LINEAR);

  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}


Texture::~Texture()
{
  delete image;
  glDeleteTextures (1,&name);
}


void Texture::vBind (bool fModulate)
{
  glBindTexture (GL_TEXTURE_2D,name);
  vGetError ();

  glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
	     fModulate ? GL_MODULATE : GL_DECAL);
  vGetError ();
}

/****************************************************************************/
/** 
*****************************************************************************/
#define CASE(x) case x: szError = #x; break;

void Juice::Render::vGetError ()
{
	char *szError = null;
	GLenum e = glGetError ();
	switch (e)
	{
		CASE (GL_NO_ERROR);
		CASE (GL_INVALID_ENUM);
		CASE (GL_INVALID_VALUE);
		CASE (GL_INVALID_OPERATION);
		CASE (GL_STACK_OVERFLOW);
		CASE (GL_STACK_UNDERFLOW);
		CASE (GL_OUT_OF_MEMORY);
//		CASE (GL_INVALID_OPERATION);
	}

	if (GL_NO_ERROR != e)
	{
#ifdef _DEBUG
		Breakpoint ("Juice::Render::vGetError");
		//::MessageBox (NULL, szError, "OpenGL Error", MB_OK);
#endif
		OutputDebugString ("Juice: OpenGL Error: ");
		OutputDebugString (szError);
		OutputDebugString ("\n");
	}
}

void Juice::Render::vSetMaterial (Color &clr)
{
	glEnable (GL_LIGHTING);
	glShadeModel (GL_SMOOTH);

	// Diffuse color: the color an object has when reflecting light
	GLfloat Color[] = 
	{ 
		(GLfloat) clr.r, 
		(GLfloat) clr.g,
		(GLfloat) clr.b, 
		(GLfloat) 1.0 // clr.a
	}; 
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Color);

	// Ambient color: the color without lighting, or the color that comes from "ambient" light.
	// There's really only one light in the scene, but this gives the illusion of ambient light.
	Color[0] /= 3;
	Color[1] /= 3;
	Color[2] /= 3;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Color);
}

void Juice::Render::vSetShadow (Color &clr)
{
	glDisable (GL_LIGHTING);
	glShadeModel (GL_FLAT);
    glColor3d (clr.r, clr.g, clr.b);
}

void Juice::Render::vSetLine (Color &clr)
{
	glDisable (GL_LIGHTING);
	glShadeModel (GL_SMOOTH);
	glColor3d (clr.r, clr.g, clr.b);
}

// RenderInternal.cpp ends here ->