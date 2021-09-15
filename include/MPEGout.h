/*****************************************************************************

MPEGout.h is the work of David Whittaker.  Here's what he has to say...

(Note that what he refers to as "license.txt" is distributed with just under 
the name "DaliLicense.txt" to avoid confusion.)

--

First of all, many thanks go to the Dali team at Cornell University, the image and video
which this classes uses for all conversion functions.  See accompanying license.txt for
Dali's licensing restrictions.  As far as I am concerned, I am providing my code with no
strings attached.

Quick start instructions.
This class is designed to be as simple to use as possible, while still granting you
per-frame access to the level of compression. It's original purpose was to allow someone to
create a memory DC, create a bitmap with CreateDibSection, select it into the memory DC,
and then have a one-line instruction to put	this bitmap into an mpeg on every refresh.
Since then the class's purposes have expanded and it's methods have been generalized to
allow bitmap and ppm, and other formats (with direct access to the r, g, and b color spaces),
and to allow the level of compression to be changed both at the beginning for the whole movie,
and between each frame so the file size/image quality ratio can have the best output.

The simplest way to use the class is as follows:

BYTE *bitmap;
CMPEGout mpeg(640, 480, bitmap, PICTYPE_BMP, "output.mpg");	//if no filename specified,
															//it will use "untitled.mpg
while(!exit)
{
	//....generate the next scene and store it in bitmap.
	mpeg.AddFrame();
}

You don't even have to call Start() and Stop() because AddFrame calls Start if it hasn't been
called yet, and the destructor calls Stop() if it hasn't been called.  Start and Stop are
included to allow you to use the same object repeatedly to output to different mpeg's, and so
you get to decide when the memory gets allocated.  A slight pause might be noticed when calling
Start between frames of video output, but as long as the video source isn't too large, AddFrame
can run in real-time.  My 1.2GHz Athlon MP can simultaneously process three different 640x480
videos at 24 frames per second.  This was my target value, so I stopped optimizing when I
reached this point.  It is possible that there is room for improvement.

Read the comments in the rest of this file for an in-depth explanation of the compression
system and a description of each function.

*****************************************************************************/

#ifdef MPEGOUT_EXPORTS
#define MPEGOUT_API __declspec(dllexport)
#else
#define MPEGOUT_API __declspec(dllimport)

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <math.h>

//structs from other projects (DALI) that this dll uses included here to increase portability
typedef struct ByteImage {int width;int height;int x;int y;int parentWidth;unsigned char isVirtual;unsigned char *firstByte;} ByteImage;
typedef struct ScBlock {int intracoded;char skipMB;char skipBlock;short dc;char numOfAC;char index[63];short value[63];} ScBlock;
typedef struct ScImage {int width;int height;int x;int y;int parentWidth;unsigned char isVirtual;ScBlock *firstBlock;} ScImage;
typedef struct Vector {char exists;char down;char right;} Vector;
typedef struct VectorImage {int width;int height;int x;int y;int parentWidth;unsigned char isVirtual;Vector *firstVector;} VectorImage;
typedef struct BitStream {unsigned char *buffer;unsigned char *endDataPtr;unsigned char *endBufPtr;unsigned char isVirtual;int size;} BitStream;
typedef struct BitParser {BitStream *bs;unsigned char *offsetPtr;int currentBits;int bitCount;} BitParser;
typedef struct MpegSeqHdr {short width;short height;short mb_width;short mb_height;int bitrate;int vbv_buffer_size;char pel_aspect_ratio;char picture_rate;char constrained;char default_qt;unsigned char iqt[64];unsigned char niqt[64];} MpegSeqHdr;
typedef struct MpegGopHdr {char drop_frame_flag;char time_code_hours;char time_code_minutes;char time_code_seconds;char time_code_pictures;char closed_gop;char broken_link;} MpegGopHdr;
typedef struct MpegPicHdr {short temporal_reference;char type;unsigned short vbv_delay;char full_pel_forward_vector;char forward_r_size, forward_f;char full_pel_backward_vector;char backward_r_size, backward_f;} MpegPicHdr;

#endif	//else MPEGOUT_EXPORTS not defined (external project)

#define PICTYPE_BMP		1
#define PICTYPE_PPM		2
#define PICTYPE_OTHER	3
//use PICTYPE_OTHER with GetRPtr(), GetGPtr(), and GetBPtr() and your own conversion function
//to use a picture format other than bmp or ppm.  See comments after those functions.


// This class is exported from the MPEGout.dll
class MPEGOUT_API CMPEGout
{
	int framenum, pictures, seconds, minutes, hours, compression, pictype, temporalRef;
	bool decode, wasI, wasDecoded, useOut;
	double fps;
    FILE *outFile;
    char fileName[100];
    BitParser *obp;			/* output bitparser */
    BitStream *obs;			/* output bitstream */
	ByteImage bmBits;

    ByteImage *debug, *r, *g, *b, *y, *u, *v, *prevY, *prevU, *prevV, *outY, *outU, *outV;
	ByteImage *qScale, *inter[3];
    ScImage *scY, *scU, *scV;
    VectorImage *fmv;                       /* forward motion vector */

	MpegSeqHdr *seqHdr;
    MpegGopHdr *gopHdr;
    MpegPicHdr *picHdr;

    int sliceInfo[1];       /* this should be enough for one frame */
    int sliceInfoLen;

	bool isRunning;


	void IncrementTime();
	void Swap();

public:
	~CMPEGout() { Stop(); }
	CMPEGout(	int w,		//width of every picture (only set outside of start/stop commands)
				int h,		//height of every picture
				BYTE *bits,	//pointer 
				int type,	//type of picture (currently PICTYPE_BMP or PICTYPE_PPM)
				const char *outfile = "untitled.mpg",	//output file name
				double FPS = 24,	//frames per second: one of 23.976, 24, 25, 29.97, 30, 50, 59.94, 60
				int complevel = 1,	//compression level, see below
				bool decode = true	//use decoded previous picture for motion vector search, see below
			);
/*	
	All compression levels above 0 are valid, but file size levels off between 8 and 12.
	
	Frames are ordered IPPPPIPPPPIPPPP with an I frame (basically just a JPEG) occurring every
	complevel times, and P frames (use motion vectors between the previous current frame to
	increase compression) in between.
	
	Specifying decode = false will tell the engine to use the unencoded picture for the next
	frame's motion vector search. This will make the engine run faster, since it doesn't have
	to decode every frame it just encoded that has a P frame following it), but since the
	target decoder will not have access to these pictures, it will have to use the decoded
	pictures, propagating any errors until the next I frame. So, do not use high values for
	complevel (it starts looking bad between 3 and 6, depending on motion) with the decode flag
	set to false.  If complevel is 1, the decode flag will have no effect on final output.
	
	decode = false is not recommended for any movies with lots of motion in them.
	
	If you know there will be some sections with lots of motion and some with mostly stills,
	you are allowed to change both the complevel and the decode flag between subsequent calls
	to addFrame by using SetCompression(complevel), SetDecode(), and UnsetDecode().
	
	If you know a low motion section is coming, set complevel to 1 just before calling 
	addFrame to force an I frame into the file, then optionally UnsetDecode() if it is
	close to completely still, and set complevel as high as you would like (stillSeconds*fps
	works well if it is perfectly still, lower for more motion) making sure to lower complevel
	and SetDecode() before motion resumes.  This will give the maximum compression.

	Note that further compression could be achieved by using bi-directional motion vectors,
	but that would mean pictures would have to be cached and processed when all the pictures
	in a group were received.  In a real-time application, this would cause extremely choppy
	motion, not to mention even more memory than this thing already uses.
*/

	void AddFrame();
	//workhorse function: call this to convert and copy the data in bits out to the mpeg

	void GetLastFrame(BYTE *bits, int outPicType = 0);
	//decodes the last frame put to the mpeg, so WYSIWYG.  Store the data in bits, unless
	//PICTYPE_OTHER is specified, then leave it in the r, g, and b components.
	//outPicType = 0 will use the same pictype as the input functions.

	void Start();
	void Stop();
	//Allocates and deallocates memory used in the mpeg creation.
	//These are optional if you are only creating one mpeg per CMPEGout object.
	//Start is automatically called when AddFrame is called and the memory is not allocated.
	//Stop is automatically called by the destructor if the memory has not been deallocated.

	void SetFPS(double newFPS)	//picks the closest legal frames per second
	{
		if (!isRunning)
		{
			double legal[8] = {23.976, 24, 25, 29.97, 30, 50, 59.94, 60};
			int closest = 0;
			for (int i = 1; i < 8; i++)
			{
				if (fabs(legal[closest] - newFPS) > fabs(legal[i] - newFPS)) closest = i;
			}
			
			fps = legal[closest];
		}
	}
	double GetFPS() { return fps; }

	void SetSize(int newWidth, int newHeight)	//sets the size of the bitmap.
	{											//no effect if called between Start and Stop
		if (!isRunning)
		{
			bmBits.width = newWidth * 3;	//newWidth*3 for 3 bytes per pixel
			bmBits.height = newHeight;
			
			sliceInfo[0] = newWidth * newHeight * 3;
			sliceInfoLen = 1;
		}
	}
	int GetWidth() { return bmBits.width; }
	int GetHeight(){ return bmBits.height;}
	bool IsRunning() { return isRunning; }
	//checks if engine is currently in recording mode (between start and stop)
	
	void SetFileName(const char *newFileName)	//sets the file name for the mpeg
	{										//no effect if called between Start and Stop
		if (!isRunning)
			strcpy(fileName, newFileName);
	}
	char *GetFileName() { return fileName; }
	
	void SetBits(BYTE *bits)	//sets the pointer to the bitmap, can be called at any time.
	{							//it is your responsibility to make sure the bitmap complies
		if (bits)				//with the values in width and height. (size cannot change
			bmBits.firstByte = bits;	//between Start and Stop)
	}
	BYTE *GetBits() { return bmBits.firstByte; }
	
	void SetCompression(int complevel)	//set compression level. See long comment above.
	{
		if (complevel > 0)
			compression = complevel;
	}
	int GetCompression() { return compression; }
	
	void SetDecode()		//Set decode flag.  See long comment above.
	{	decode = true; }
	
	void UnsetDecode()		//Unset decode flag.  See long comment above.
	{	decode = false; }

	bool GetDecode() { return decode; }
	
	void SetPicType(int type)	//Set picture type to one of the PICTYPE_* defines
	{
		if (type == PICTYPE_BMP || type == PICTYPE_PPM || type == PICTYPE_OTHER)
			pictype = type;		
	}
	int GetPicType() { return pictype; }

	BYTE *GetRPtr() { if (isRunning) return r->firstByte; return NULL; }
	BYTE *GetGPtr() { if (isRunning) return g->firstByte; return NULL; }
	BYTE *GetBPtr() { if (isRunning) return b->firstByte; return NULL; }
	//use these in combination with PICTYPE_OTHER to use a picture format other than bmp or ppm
	//convert your picture to rgb color space, with the first byte in the top left corner of
	//the picture, across first, then down.  Store each component separately in the arrays
	//that these functions return.  The arrays are (width*height) bytes each.
	//For example, here is the code that converts ppm data to rgb data.
	
	//register BYTE *currbits = bits;
	//register BYTE *currr = GetRPtr();
	//register BYTE *currg = GetGPtr();
	//register BYTE *currb = GetBPtr();
	//register BYTE *endbits = bits + width*height*3;
	//while (currbits < endbits)
	//{
	//	*currr++ = *currbits++;
	//	*currg++ = *currbits++;
	//	*currb++ = *currbits++;
	//}

	//When pictype is PICTYPE_OTHER, AddFrame uses the values in r, g, and b instead of
	//converting the data in bits and placing them in r, g, and b.  Also, these pointers will
	//not change between Start and Stop calls.  An interesting application of this would be to
	//make your frame generation routine render directly into these arrays, knocking one step
	//out of the way, and increasing speed even more.  You just have to figure out a way to
	//render r, g, and b components separately.

	//Note: GetLastFrame also stores it's data in r, g, and b, so you can also use these
	//pointers to convert this data back into your native file format.
};

// David Whittaker's MPEGout.h ends here ->