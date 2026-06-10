/* 
 * The contents of this file are subject to the Mozilla Public 
 * License Version 1.1 (the "License"); you may not use this file 
 * except in compliance with the License. You may obtain a copy of 
 * the License at http://www.mozilla.org/MPL/ 
 * 
 * Software distributed under the License is distributed on an "AS 
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
 * implied. See the License for the specific language governing 
 * rights and limitations under the License. 
 * 
 * The Original Code is tUME: the Universal Map Editor, released 
 * June 18, 2000. 
 * 
 * The Initial Developer of the Original Code is Echidna. 
 * Portions created by Echidna are Copyright (c) 1989-2000 Echidna. 
 * All Rights Reserved. 
 * 
 * Contributor(s): 
 */

/*
 * IBMMOUSE.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 07/09/90
 *   MODIFIED : 10/19/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		IBM Mouse Glue modified from original code by Dan Chang.
 *
 * HISTORY
 *		09/16/92 Wednesday (dcc) - support added to save mouse events
 *							to a text file, and to read mouse events from
 *							a text file.
 *		09/28/93 Tuesday (dcc) - enhanced to support MS-DOS Watcom C32.
 *
*/

#include <echidna/platform.h>
#include "switches.h"
#include "switch1.h"					// for DEBUG

#if __MSDOS32X__
#include <echidna/rmdos.h>
#else
#include <dos.h>
#endif
#include <stdio.h>

#include <echidna/etypes.h>
#include <echidna/mouse.h>
#include <echidna/memory.h>
#include <echidna/eerrors.h>
#include <echidna/timer.h>
#include <echidna/grafx.h>
#include <echidna/recorder.h>
#include "ibmmouse.h"					/* Verify function prototypes. */

/**************************** C O N S T A N T S ***************************/

#define MOUSE_INT 0x33

#define MOUSE_XSIZE	32
#define MOUSE_YSIZE	32

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

static short inside;
static short ShowCount = 0;
static short OldShowCount = 0;
static WORD	 MSIntHndl;

static short MouseX;
static short MouseY;
static short MouseButtons;

static short MouseXMin	 = 0;
static short MouseXRange = 320;
static short MouseYMin   = 0;
static short MouseYRange = 200;

static short MouseXRes   = 640;
static short MouseYRes   = 400;

static short MouseXHot	 = 0;
static short MouseYHot	 = 0;

static short OldMouseX;
static short OldMouseY;
static short OldRx;
static short OldRy;
static short OldRWidth;
static short OldRHeight;

static short MouseScrnWidth  = 320;
static short MouseScrnHeight = 200;

static short RestoreState = 0;

static GShape	*MouseShape;
static GShape	*OldMouseShape;

static ByteMap	 MouseSaveByteMap;
static ByteMap	 ScreenByteMap;

static short fLastSaveLoad = 0;
static short fSaveLoad = 0;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

#if 0
INT 33 - MS MOUSE - READ MOTION COUNTERS
	AX = 000Bh
Return: CX = number of mickeys mouse moved horizontally since last call
	DX = number of mickeys mouse moved vertically
Notes:	a mickey is the smallest increment the mouse can sense
	positive values indicate down/right
SeeAlso: AX=0003h,AX=001Bh
#endif


/* One level only "push" of where to get mouse input. */

void SaveMouseLocation(void)
{
	fLastSaveLoad = fSaveLoad;
}

/* Set where to get mouse input. */

void SetMouseEvents(short f)
{
	fSaveLoad = f;
}

/* One level only "pop" of where to get mouse input from. */

void RestoreMouseLocation(void)
{
	fSaveLoad = fLastSaveLoad;
}


#if __TURBOC__
#pragma option -N-
#elif __WATCOMC__
#pragma off (check_stack);
#endif

static void RestoreMouseArea (void)
{

	if (RestoreState)
	{
		short	left;
		short	top;
		short	width;
		short	height;

		GetClipDimensions (left, top, width, height);
		SetClipDimensions (0, 0, MouseScrnWidth, MouseScrnHeight);

		RestoreState = FALSE;
 		ClippedCopyRect (&MouseSaveByteMap, 0, 0, &ScreenByteMap, OldRx, OldRy, OldRWidth, OldRHeight);

		SetClipDimensions (left, top, width, height);
	}


}

static InterruptRoutineType MouseInterrupt (void)
{
	if (!inside)
	{
		short	mouseOn;

		inside = TRUE;
		{
			union REGS regs;
#if (__WATCOMC__ && __MSDOS32X__)
			regs.w.ax = 0x03;

			int386(MOUSE_INT, &regs, &regs);

			MouseButtons = regs.w.bx & 0x03;

			regs.w.ax = 0x0B;

			int386(MOUSE_INT, &regs, &regs);

			MouseX += (short)regs.w.cx;
			MouseY += (short)regs.w.dx;
#elif __TURBOC__
			regs.x.ax = 0x03;

			int86(MOUSE_INT, &regs, &regs);

			MouseButtons = regs.x.bx & 0x03;
	
			regs.x.ax = 0x0B;

			int86(MOUSE_INT, &regs, &regs);

			MouseX += (short)regs.x.cx;
			MouseY += (short)regs.x.dx;
#else
#error Need Something Here!
#endif

			if (MouseX < 0)          MouseX = 0;
			if (MouseX >= MouseXRes) MouseX = MouseXRes - 1;
			if (MouseY < 0)          MouseY = 0;
			if (MouseY >= MouseYRes) MouseY = MouseYRes - 1;
		}

		mouseOn = ShowCount > 0;
		if (mouseOn)
		{
			short	x;
			short	y;
			short	oldMouseOn;
			short	doit0;
			short	doit1;
			short	doit2;
			short	doit3;

			x = (short) (((long)MouseX * MouseScrnWidth  / MouseXRes) + MouseXHot);
			y = (short) (((long)MouseY * MouseScrnHeight / MouseYRes) + MouseYHot);

			oldMouseOn = OldShowCount > 0;
			doit0 = mouseOn != oldMouseOn;
			doit1 = MouseShape != OldMouseShape;
			doit2 = x != OldMouseX;
			doit3 = y != OldMouseY;
			if (doit0 || doit1 || doit2 || doit3)
			{
				short	left;
				short	top;
				short	width;
				short	height;

				GetClipDimensions (left, top, width, height);
				SetClipDimensions (0, 0, MouseScrnWidth, MouseScrnHeight);

				RestoreMouseArea ();
			
				//
				// Save Area
				//
				{
					OldRx      = x;
					OldRy      = y;
					OldRWidth  = MOUSE_XSIZE;
					OldRHeight = MOUSE_YSIZE;

					if (OldRx < 0) {
						OldRWidth += OldRx;
						OldRx      = 0;
					}
					if (OldRx > MouseScrnWidth - MOUSE_XSIZE)
					{
						OldRWidth -= OldRx - (MouseScrnWidth - MOUSE_XSIZE);
					}

					if (OldRy < 0) {
						OldRHeight += OldRy;
						OldRy       = 0;
					}
					if (OldRy > MouseScrnHeight - MOUSE_YSIZE)
					{
						OldRHeight -= OldRy - (MouseScrnHeight - MOUSE_YSIZE);
					}

					if (OldRWidth && OldRHeight)
					{

						ClippedCopyRect (&ScreenByteMap, OldRx, OldRy, &MouseSaveByteMap, 0, 0, MOUSE_XSIZE, MOUSE_YSIZE);
						RestoreState = TRUE;
					}
				}

				//
				// Draw Mouse
				//
				if (MouseShape)
				{
					DrawGShape (MouseShape, x, y);
				}

				//
				// Save old position
				//
				OldMouseShape = MouseShape;
				OldMouseX     = x;
				OldMouseY     = y;

				SetClipDimensions (left, top, width, height);
			}
		}
		OldShowCount = ShowCount;
		inside = FALSE;
	}
}

void ShowMouse(void)
{
	if (!inside)
	{
		ShowCount++;
	}
}

void HideMouse(void)
{
	if (!inside)
	{
		ShowCount--;
		if (!ShowCount) {
			RestoreMouseArea ();
			OldShowCount = 0;
		}
	}
}


#if __TURBOC__
#pragma option -N.
#endif

void SetMousePosition (short x, short y)
{
	MouseX = (short) ((long) (x - MouseXMin) * MouseXRes / MouseXRange);
	MouseY = (short) ((long) (y - MouseYMin) * MouseYRes / MouseYRange);

	MouseInterrupt();
}

void ReadMouse(MouseInfo *m)
{
	if (fSaveLoad == fReadEvents)
	{
		char sz[40];

		sz[0] = 'M';
		sz[1] = 0;

		m->OldButtons = m->Buttons;

		if (ReadEvent(sz))
		{
			short X;
			short Y;
			UWORD Buttons;

			sscanf(sz+1, "%d %d %d", &X, &Y, &Buttons);

			if (X != m->X || Y != m->Y)
			{
				/* Set the mouse pointer display position */

				SetMousePosition(X, Y);
			}
			m->X = X;
			m->Y = Y;
			m->Buttons = Buttons;
		}
	}
	else
	{
		short X;
		short Y;

		X             = (short) (MouseXMin + (long)MouseX * (long)MouseXRange / MouseXRes);
		Y             = (short) (MouseYMin + (long)MouseY * (long)MouseYRange / MouseYRes);
		m->OldButtons = m->Buttons;
		m->Buttons    = MouseButtons;

		if (fSaveLoad == fSaveAllEvents)
		{
			char sz[40];

			sprintf(sz, "M %d %d %d\n", X, Y, m->Buttons);
			SaveEvent(sz);
		}
		else if (fSaveLoad == fSaveSomeEvents)
		{
			if (m->Buttons || m->Buttons != m->OldButtons || m->X != X || m->Y != Y)
			{
				char sz[40];

				sprintf(sz, "M %d %d %d\n", X, Y, m->Buttons);
				SaveEvent(sz);
			}
		}
		m->X = X;
		m->Y = Y;
	}
}

void SetMouseHotSpot (short xhot, short yhot)
{
	MouseXHot = xhot;
	MouseYHot = yhot;
}

void SetMouseResolution (short xres, short yres)
{
	MouseXRes = xres;
	MouseYRes = yres;
}

void SetMouseRange (short xmin, short xmax, short ymin, short ymax)
{
	MouseXMin   = xmin;
	MouseYMin   = ymin;
	MouseXRange = xmax - xmin + 1;
	MouseYRange = ymax - ymin + 1;
}

void SetMouseScreenDimensions (short width, short height)
{
	MouseScrnWidth  = width;
	MouseScrnHeight = height;
}

void SetMouseShape (GShape *shape)
{
	MouseShape = shape;
}

short OpenMouse(void)
{
	union REGS regs;
	unsigned char far32 *hMouse;
	short	result = FALSE;

#if (__WATCOMC__ && __MSDOS32X__)
	regs.w.ax = 0x00;

	if ((hMouse = *(unsigned char * far32 *) rmMK_FP32(0x0, MOUSE_INT*4)) == 0)
#else
	regs.x.ax = 0x00;

	if ((hMouse = *(unsigned char * far32 *) MK_FP(0x0, MOUSE_INT*4)) == 0)
#endif
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nMouse Driver not found");
		return FALSE;
	}
	if (*hMouse == 0xCF /* IRET */) {
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nMouse Driver not found");
		return FALSE;
	}
#if (__WATCOMC__ && __MSDOS32X__)
	int386(MOUSE_INT, &regs, &regs);

	result = (regs.w.ax == (unsigned short)(-1));
#else
	int86(MOUSE_INT, &regs, &regs);

	result = (regs.x.ax == (unsigned short)(-1));
#endif

	if (result)
	{
		ScreenByteMap.width  = 320;
		ScreenByteMap.height = 200;
#if __MSDOS32X__
		ScreenByteMap.data   = rmMK_FP32 (0xA000, 0x0000);
#else
		ScreenByteMap.data   = MK_FP (0xA000, 0x0000);
#endif

		MouseSaveByteMap.width  = MOUSE_XSIZE;
		MouseSaveByteMap.height = MOUSE_YSIZE;
		MouseSaveByteMap.data   = AllocMemOnPara (MOUSE_XSIZE * MOUSE_YSIZE);

		if (MouseSaveByteMap.data)
		{
			MSIntHndl  = InstallVBlank (MouseInterrupt);
			if (MSIntHndl) {
				return TRUE;
			}
		}
		CloseMouse ();
	}

	SetGlobalErr (ERR_OUT_OF_MEMORY);
	GEcatf ("\nOut of memory.");
	return FALSE;
}

void CloseMouse (void)
{
	if (MSIntHndl)	RemoveVBlank (MSIntHndl);
	MSIntHndl = 0;
	if (MouseSaveByteMap.data)	FreeMemOnPara (MouseSaveByteMap.data);
	MouseSaveByteMap.data = NULL;


	{
		union REGS regs;
#if (__WATCOMC__ && __MSDOS32X__)
		regs.w.ax = 0x00;

		int386(MOUSE_INT, &regs, &regs);
#else
		regs.x.ax = 0x00;

		int86(MOUSE_INT, &regs, &regs);
#endif
	}
}

