/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: RenderInternal.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __RenderInternal__
#define __RenderInternal__

#include <JuiceTypes.h>

/*****************************************************************************
*****************************************************************************/

namespace Juice {
namespace Render {

/****************************************************************************/
/** Display Lists
*****************************************************************************/
enum DisplayList
{
	dlInvalid = 0,
	dlSetup,
	dlLighting,
	//dlBackground,
	dlCage,
	dlCageFloor,
	dlGround,
	dlCursor,
	dlModel,
	dlSimpleScene
};

/****************************************************************************/
/** Image and texture helper classes
*****************************************************************************/

typedef unsigned char byte;

class Image
{
private:
	int image_width,image_height;
	byte *image_data;

public:
	Image (char *filename);
	// load from PPM file
	~Image();
	int width() { return image_width; }
	int height() { return image_height; }
	byte *data() { return image_data; }
};

class Texture
{
private:
	Image *image;
	Generic::UINT32 name;

public:
	Texture (char *szFilename);
	~Texture();
	void vBind (bool fModulate);
};

/****************************************************************************/
/** Set the current color for GL_LINES
*****************************************************************************/

void vSetLine (Color &clr);

/****************************************************************************/
/** Prepare to draw object polygons of the given color
*****************************************************************************/

void vSetMaterial (Color &clr);

/****************************************************************************/
/** Prepare to draw shadow polygons of the given color
*****************************************************************************/

void vSetShadow (Color &clr);

/****************************************************************************/
/** Reports OpenGL errors.
*****************************************************************************/

void vGetError ();

}; // end namespace Render

}; // end namespace Juice

#endif

// RenderInternal.h ends here ->