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
 * KEYBOARD.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 11/16/90
 *   MODIFIED : 02/05/95
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Keyboard routines?
 *
 * HISTORY
 *		09/16/92 (dcc) - Support added to save keyboard events
 *						 to a text file, and to read keyboard events
 *						 from a text file. This support is only on
 *						 the GetKeyNoWait() routine.
 *		05/13/93 (GAT) - Change routines to use int 16H, 10H
 *						 instead of chaining in 09H.
 *		09/28/93 (dcc) - Fix so F11 & F12 are recognized (0x12 to 12)
 *		10/12/93 (dcc) - Updated to support MS-DOS Watcom C32.
 *		02/04/94 (dcc) - Make LEFT-SHIFT generate QUAL_SHIFT as well.
 *		02/04/94 (dcc) - Fix [Shift]-[,], [Shift]-[.] to output '<' & '>'
 *		01/22/95 (dcc) - Make module lint with no errors.
 *
*/

#include <ctype.h>
#include <stdio.h>

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/etypes.h>
#include <echidna/eerrors.h>
#include "keyboard.h" //<echidna/keyboard.h>
#include <echidna/recorder.h>

#if __TURBOC__
#include <bios.h>
#include <conio.h>
#include <dos.h>
#elif __WATCOMC__
#include <bios.h>
#include <dos.h>
#endif

#if __MSDOS32X__
#include <echidna/rmdos.h>
#endif

/**************************** C O N S T A N T S ***************************/

#define EAT_REPEAT	1		/* Throw Away Repeated Keys in Buffer */

#if __MSDOS__

#define	KEYBIT_NUMLOCK		0x20
#define KEYBIT_CAPSLOCK		0x40
#define KEYBIT_LEFTSHIFT	0x02
#define KEYBIT_RIGHTSHIFT	0x01
#define KEYBIT_ALT			0x08

#else
#error need stuff
#endif

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/
static short fLastSaveLoad = 0;
static short fSaveLoad = 0;
/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

#define NKEY	0x00
#define GREY	0x80

static UWORD ModKeyTable[] = {
	NKEY,	//			/* 00 */
	NKEY,	//	ESC
	NKEY,	//	1 !
	NKEY,	//	2 @
	NKEY,	//	3 #
	NKEY,	//	4 $
	NKEY,	//	5 %
	NKEY,	//	6 ^
	NKEY,	//	7 &		/* 08 */
	NKEY,	//	8 *
	NKEY,	//	9 (
	NKEY,	//	0 )
	NKEY,	//	- _
	NKEY,	//	= +
	0x0E,	//	BKSP
	0x0F,	//	TAB
	0x10,	//	q Q		/* 10 */
	0x11,	//	w W
	0x12,	//	e E
	0x13,	//	r R
	0x14,	//	t T
	0x15,	//	y Y
	0x16,	//	u U
	0x17,	//	i I
	0x18,	//	o O		/* 18 */
	0x19,	//	p P
	0x1A,	//	[ {
	0x1B,	//	] }
	NKEY,	//	CR
	NKEY,	//	CTRL --??--
	0x1E,	//	a A
	0x1F,	//	s S
	0x20,	//	d D		/* 20 */
	0x21,	//	f F
	0x22,	//	g G
	0x23,	//	h H
	0x24,	//	j J
	0x25,	//	k K
	0x26,	//	l L
	0x27,	//	; :
	0x28,	//	' "		/* 28 */
	0x29,	//	` ~
	NKEY,	//	LSHIFT --??--
	NKEY,	//	\ |
	0x2C,	//	z Z
	0x2D,	//	x X
	0x2E,	//	c C
	0x2F,	//	v V
	0x30,	//	b B		/* 30 */
	0x31,	//	n N
	0x32,	//	m M
	0x33,	//	, <
	0x34,	//	. >
	0x35,	//	/ ?
	NKEY,	//	RSHIFT --??--
	NKEY,	//	PRTSC  --??--
	NKEY,	//	ALT --??--	/* 38 */
	NKEY,	//	SPACE
	NKEY,	//	CAPS
	0x3B,	//	F1
	0x3C,	//	F2
	0x3D,	//	F3
	0x3E,	//	F4
	0x3F,	//	F5
	0x40,	//	F6			/* 40 */
	0x41,	//	F7
	0x42,	//	F8
	0x43,	//	F9
	0x44,	//	F10
	NKEY,	//	NUMLOCK
	NKEY,	//	SCROLLLOCK
	0x47,	//	KEYPAD HOME
	0x48,	//	KEYPAD UP	/* 48 */
	0x49,	//	KEYPAD PGUP
	0x4A,	//	KEYPAD GREY -
	0x4B,	//	KEYPAD LEFT
	0x4C,	//	KEYPAD CENTER
	0x4D,	//	KEYPAD RIGHT
	0x4E,	//	KEYPAD GREY +
	0x4F,	//	KEYPAD END
	0x50,	//	KEYPAD DOWN	/* 50 */
	0x51,	//	KEYPAD PGDN
	0x52,	//	KEYPAD INS
	0x53,	//	KEYPAD DEL
	0x3B,	//	Shift-F1
	0x3C,	//	Shift-F2
	0x3D,	//	Shift-F3
	0x3E,	//	Shift-F4
	0x3F,	//	Shift-F5			/* 58 */
	0x40,	//	Shift-F6
	0x41,	//	Shift-F7
	0x42,	//	Shift-F8
	0x43,	//	Shift-F9
	0x44,	//	Shift-F10
	0x3B,	//	Ctrl-F1
	0x3C,	//	Ctrl-F2
	0x3D,	//	Ctrl-F3			/* 60 */
	0x3E,	//	Ctrl-F4
	0x3F,	//	Ctrl-F5
	0x40,	//	Ctrl-F6
	0x41,	//	Ctrl-F7
	0x42,	//	Ctrl-F8
	0x43,	//	Ctrl-F9
	0x44,	//	Ctrl-F10
	0x3B,	//	Alt-F1			/* 68 */
	0x3C,	//	Alt-F2
	0x3D,	//	Alt-F3
	0x3E,	//	Alt-F4
	0x3F,	//	Alt-F5
	0x40,	//	Alt-F6
	0x41,	//	Alt-F7
	0x42,	//	Alt-F8
	0x43,	//	Alt-F9			/* 70 */
	0x44,	//	Alt-F10
	NKEY,	//
	0x4B,	//	Ctrl-Keypad-4
	0x4D,	//	Ctrl-Keypad-6
	0x4F,	//	Ctrl-Keypad-1
	0x51,	//	Ctrl-Keypad-3
	0x47,	//	Ctrl-Keypad-7
	0x02,	//	Alt-1	/* 78 */
	0x03,	//	Alt-2
	0x04,	//	Alt-3
	0x05,	//	Alt-4
	0x06,	//	Alt-5
	0x07,	//	Alt-6
	0x08,	//	Alt-7
	0x09,	//	Alt-8

	0x0A,	//	Alt-9	/* 80 */
	0x0B,	//	Alt-0
	0x0C,	//	Alt--
	0x0D,	//	Alt-=
	0x49,	//	Ctrl-Keypad-9
	11+GREY,	//	F11
	12+GREY,	//	F12
	11+GREY,	//	Shift-F11
	12+GREY,	//	Shift-F12	/* 88 */
	11+GREY,	//	Ctrl-F11
	12+GREY,	//	Ctrl-F12
	11+GREY,	//	Alt-F11
	12+GREY,	//	Alt-F12
	0x48,	//	Ctrl-Keypad-8
	0x4A,	//	Ctrl-Keypad--
	0x4C,	//	Ctrl-Keypad-5
	0x4E,	//	Ctrl-Keypad-+	/* 90 */
	0x50,	//	Ctrl-Keypad-2
	0x52,	//	Ctrl-Keypad-0
	0x53,	//	Ctrl-Keypad-.
	0x0F,	//	Ctrl-Tab
	0x35+GREY,	//	Ctrl-Keypad-/
	0x37,	// Ctrl-Keyppad-*
	0x47+GREY,	//	Alt-Home
	0x48+GREY,	//	Alt-Up	/* 98 */
	0x49+GREY,	//	Alt-PgUp
	NKEY,	//
	0x4B+GREY,	//	Alt-Left
	NKEY,	//
	0x4D+GREY,	//	Alt-Right
	NKEY,	//
	0x4F+GREY,	//	Alt-End
	0x50+GREY,	//	Alt-Down	/* A0 */
	0x51+GREY,	//	Alt-PgDn
	0x52+GREY,	//	Alt-Insert
	0x53+GREY,	//	Alt-Delete
	0x35+GREY,	//	Alt-Keypad-/
	NKEY,	//
	0x1C+GREY,	//	Alt-Keypad-Enter
	NKEY,	//
	NKEY,	//		/* A8 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* B0 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* B8 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* C0 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* C8 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* D0 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* D8 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* E0 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* E8 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* F0 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* F8 */
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//
	NKEY,	//		/* 100 */
};


/*********************************************************************
 *
 * fAnyKeyPressed
 *
 * SYNOPSIS
 *		static BOOL fAnyKeyPressed(void)
 *
 * PURPOSE
 *		Call this routine to see if there is a keystroke waiting.
 *
 *		By using fAnyKeyPressed() instead of kbhit(), we prevent '^C's from
 *		being printed on screen when the user presses [Ctrl]-[Break].
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL fAnyKeyPressed(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fAnyKeyPressed";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if (__WATCOMC__ && __MSDOS32X__)
   {
	union rmREGS regs;

	regs.h.ah = 0x11;
	rmint86(0x16,&regs,&regs);
	return (regs.x.flags & 0x40) == 0;
   }
#elif __TURBOC__
   {
	union	REGS regs;

	regs.h.ah = 0x11;
	int86(0x16,&regs,&regs);
	return (regs.x.flags & 0x40) == 0;
   }
#else
#error Need Something Here!
#endif

} /* fAnyKeyPressed */


/*********************************************************************
 *
 * SaveKeyboardLocation
 *
 * SYNOPSIS
 *		void SaveKeyboardLocation(void)
 *
 * PURPOSE
 *		One level only "push" of where to get keyboard input.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void SaveKeyboardLocation(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveKeyboardLocation";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	fLastSaveLoad = fSaveLoad;

} /* SaveKeyboardLocation */


/*********************************************************************
 *
 * SetKeyboardEvents
 *
 * SYNOPSIS
 *		void SetKeyboardEvents(short f)
 *
 * PURPOSE
 *		Set where to get keyboard events from, whether to save
 *		events or not.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void SetKeyboardEvents(short f)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetKeyboardEvents";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	fSaveLoad = f;

} /* SetKeyboardEvents */


/*********************************************************************
 *
 * RestoreKeyboardLocation
 *
 * SYNOPSIS
 *		void RestoreKeyboardLocation(void)
 *
 * PURPOSE
 *		One level only "pop" of where to get keyboard input from.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void RestoreKeyboardLocation(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RestoreKeyboardLocation";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	fSaveLoad = fLastSaveLoad;

} /* RestoreKeyboardLocation */


/*********************************************************************
 *
 * FlushKeyboardBuffer
 *
 * SYNOPSIS
 *		void FlushKeyboardBuffer (void)
 *
 * PURPOSE
 *		Flush all unread keys in keyboard buffer.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void FlushKeyboardBuffer (void)
{
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FlushKeyboardBuffer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	#if 0
		OutPoint = 0;
		InPoint  = 1;
	#else
		while (fAnyKeyPressed())
		{
#if (__WATCOMC__ && __MSDOS32X__)
			union rmREGS regs;

			regs.h.ah = 0x10;
			rmint86(0x16, &regs, &regs);	// read key, discard
#elif __TURBOC__
			union	REGS regs;

			regs.h.ah = 0x10;
			int86(0x16,&regs,&regs);	// read key, discard
#else
#error Need Something Here!
#endif
		}
	#endif

}} /* FlushKeyboardBuffer */

#if 0
static short my_getch (void) {
	union REGS inregs;
	union REGS outregs;

	inregs.h.ah = 0x06;
	inregs.h.dl = 0xFF;
	intdos (&inregs, &outregs);

	return (outregs.x.flags & 0x40) ? (-1) : outregs.h.al;
}
#endif

#if 0
static UBYTE Modifiers[] = {
	0x2A,	/* Left Shift		*/
	0x1D,	/* Left Ctrl		*/
	0x3A,	/* Caps Lock		*/
	0x38,	/* Left Alt			*/
	0xB8,	/* Right Alt		*/
	0x9D,	/* Right Ctrl		*/
	0x36,	/* Right Shift		*/
	0xAA,	/* Grey/NUMLOCK Mod?*/
	0x00,
};
#endif

static UBYTE GetModifiers (void)
{
#if (__WATCOMC__ && __MSDOS32X__)
	UBYTE ub;

	union rmREGS regs;

	regs.h.ah = 0x02;
	rmint86 (0x16, &regs, &regs);

	ub = regs.h.al;
	if (ub & 0x02)	/* LEFT-SHIFT? */
		ub = (ub & ~0x02) | (UBYTE) QUAL_SHIFT;
	return ub;
#elif __TURBOC__
	UBYTE ub;

	union REGS	 inregs;
	union REGS	 outregs;

	inregs.h.ah = 0x02;
	int86 (0x16, &inregs, &outregs);

	ub = outregs.h.al;
	if (ub & 0x02)	/* LEFT-SHIFT? */
		ub = (ub & ~0x02) | (UBYTE) QUAL_SHIFT;
	return ub;
#else
#error Need Something Here!
#endif
	
	//	mov	ah,02h
	//	int	16h
	//	mov	[cs:_ModBuffer+bx],al
}

/*********************************************************************
 *
 * GetKeyNoWait
 *
 * SYNOPSIS
 *		UWORD GetKeyNoWait (void)
 *
 * PURPOSE
 *		Get a Scancode from the Scancode buffer if one is available.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 * 		0 if no key available.
 *		Else returns scancode:
 *			Low Byte is code for key.
 *			High Byte is code for quailifier. Shift it down to low byte
 *			to compare to constansts: QUAL_SHIFT, QUAL_CTRL, QUAL_ALT,
 *				QUAL_KEYPAD.
 *		Pass this entire scancode to ConvertKeyToAscii() to get an ASCII
 *		value.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UWORD GetKeyNoWait (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetKeyNoWait";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
	UWORD	code = 0;

	if (fSaveLoad == fReadEvents)
	{
		char sz[40];

		sz[0] = 'K';
		sz[1] = 0;

		if (ReadEvent(sz))
		{
			sscanf(sz+1, "%u", &code);
		}
	}
	else
	{
		UBYTE	ascii;
		UBYTE	key;
		UBYTE	mod;

		if (fAnyKeyPressed())
		{
			{
#if (__WATCOMC__ && __MSDOS32X__)
				union rmREGS regs;

				regs.h.ah = 0x10;
				rmint86(0x16, &regs, &regs);
#elif __TURBOC__
				union	REGS regs;

				regs.h.ah = 0x10;
				int86(0x16,&regs,&regs);
#else
#error Need Something Here!
#endif

				key   = regs.h.ah;
				ascii = regs.h.al;
				mod   = GetModifiers ();
			}

			#if EAT_REPEAT
			while (fAnyKeyPressed())
			{
#if (__WATCOMC__ && __MSDOS32X__)
			union rmREGS regs;

			regs.h.ah = 0x10;
			rmint86(0x16, &regs, &regs);
#elif __TURBOC__
			union	REGS regs;

			regs.h.ah = 0x10;
			int86(0x16,&regs,&regs);
#else
#error Need Something Here!
#endif
			}
			#endif

			if (!ascii)
			{
				key = ModKeyTable[key];
			}
			else if (ascii == 0xE0)
			{
				if (key > 0x60)
				{
					key = ModKeyTable[key];
				}
				key |= GREY;
			}
			else
			{
				if	(key == 0xE0)
				{
					switch (ascii)
					{
					case 0x2F:	key = 0x35+GREY;	break;
					case 0x0D:	key = 0x1C+GREY;	break;
					case 0x0A:	key = 0x1C+GREY;	break;
					default:	key = 0;			break;
					}
				}
			}


			/*
			** Handle NumLock and the Keypad
			*/
			if (!(mod & KEYBIT_NUMLOCK)) {

				if (key >= 0x46 && key <= 0x53 &&
					key != 0x4A &&
					key != 0x4E) {

					if (key == 0x4C) {
						key  = 0;
					} else {
						key |= GREY;
					}
				}
			}

			code = key | (mod << 8);
		}
		if ((fSaveLoad == fSaveSomeEvents || fSaveLoad == fSaveAllEvents) && code)
		{
			char sz[40];

			sprintf(sz, "K %u\n", code);
			SaveEvent(sz);
		}
	}
	return (code);

}} /* GetKeyNoWait */

#if 0
/*********************************************************************
 *
 * ShiftFlags
 *
 * SYNOPSIS
 *		UWORD ShiftFlags (void)
 *
 * PURPOSE
 *		Return Key Modifier Flags as defined in ECHIDNA/KEYBOARD.H
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Use contants defined in ECHIDNA/KEYBOARD.H: KEY_SHIFT, KEY_ALT,
 *			KEY_CTRL to check bit flags in the return value.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UWORD ShiftFlags (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShiftFlags";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
#if __MSDOS__
#if (__TURBOC__ || __WATCOMC__)
	{
		UWORD	keys;

		keys = _bios_keybrd(_KEYBRD_SHIFTSTATUS) & 0x0F;
		if (keys & 0x02) {
			keys = (keys & (~0x02)) | KEY_SHIFT;
		}
		return keys;
	}
#else
#error Need Support for this compiler
#endif
#else
#error Need Support for this platform
#endif

}} /* ShiftFlags */

/*********************************************************************
 *
 * ShiftFlagsFromScanCode
 *
 * SYNOPSIS
 *		UWORD ShiftFlagsFromScanCode (UWORD scancode)
 *
 * PURPOSE
 *		To get bit flags you can check with mask constants: KEY_SHIFT,
 *			KEY_CTRL, KEY_ALT.  This is the kind of value that would
 *		be returned by ShiftFlags().
 *
 * INPUT
 *		scancode: A value returned from GetKeyNoWait();
 *
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		Bit flag value. Check flags with masks defined in ECHIDNA/KEYBOARD.H:
 *			KEY_SHIFT, KEY_CTRL, KEY_ALT.
 *
 * HISTORY
 *		Long and sordid.
 *
 * SEE ALSO
 *
*/
/* High byte masks for qualifiers */
#define HB_QUAL_SHIFT	(QUAL_SHIFT << 8)
#define HB_QUAL_CTRL	(QUAL_CTRL << 8)
#define HB_QUAL_ALT	(QUAL_ALT << 8)
UWORD ShiftFlagsFromScanCode (UWORD scancode)
{
	return ((scancode & HB_QUAL_SHIFT) ? KEY_SHIFT : 0) |
		((scancode & HB_QUAL_CTRL) ? KEY_CTRL : 0) |
		((scancode & HB_QUAL_ALT) ? KEY_ALT : 0);
		
} /* ShiftFlagsFromScanCode */
#endif

/* The following two variables are common to both KeyWaiting() and GetKey() */

static short wLastKey = 0;
static short fLastKeyValid = FALSE;

/*********************************************************************
 *
 * KeyWaiting
 *
 * SYNOPSIS
 *		short KeyWaiting(void)
 *
 * PURPOSE
 *		Returns TRUE if a key is waiting at input buffer.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short KeyWaiting(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "KeyWaiting";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fSaveLoad == fReadEvents)
	{
		char sz[40];

		if (fLastKeyValid)
			return TRUE;

		sz[0] = 'T';
		sz[1] = 0;

		if (ReadEvent(sz))
		{
			sscanf(sz+1, "%d", &wLastKey);
			return fLastKeyValid = TRUE;
		}
		return FALSE;
	}
	else
	{
		return (short) fAnyKeyPressed();
	}

} /* KeyWaiting */


/*********************************************************************
 *
 * GetKey
 *
 * SYNOPSIS
 *		short GetKey (void)
 *
 * PURPOSE
 *		Get key including extended keys.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short GetKey (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetKey";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fSaveLoad == fReadEvents)
	{
		if (!fLastKeyValid)
			KeyWaiting();

		if (!fLastKeyValid)
			printf("Out of Sync!\n");

		fLastKeyValid = FALSE;
		return wLastKey;
	}
	else
	{
		short	key;

		key = getch ();
		if (!key) {
			key = getch () + 256;
		}

		if (fSaveLoad == fSaveAllEvents || fSaveLoad == fSaveSomeEvents)
		{
			char sz[40];

			sprintf(sz, "T %d\n", key);
			SaveEvent(sz);
		}
		return key;	
	}
} /* GetKey */

UWORD KeyToAsciiTable[][2] = {
{	0x00,	0x00,	},	//			/* 00 */
{	0x1B,	0x1B,	},	//	ESC
{	0x31,	0x21,	},	//	1 !
{	0x32,	0x40,	},	//	2 @
{	0x33,	0x23,	},	//	3 #
{	0x34,	0x24,	},	//	4 $
{	0x35,	0x25,	},	//	5 %
{	0x36,	0x5E,	},	//	6 ^
{	0x37,	0x26,	},	//	7 &		/* 08 */
{	0x38,	0x2A,	},	//	8 *
{	0x39,	0x28,	},	//	9 (
{	0x30,	0x29,	},	//	0 )
{	0x2D,	0x5F,	},	//	- _
{	0x3D,	0x2B,	},	//	= +
{	0x08,	0x08,	},	//	BKSP
{	0x09,	0x09,	},	//	TAB
{	0x71,	0x51,	},	//	q Q		/* 10 */
{	0x77,	0x57,	},	//	w W
{	0x65,	0x45,	},	//	e E
{	0x72,	0x52,	},	//	r R
{	0x74,	0x54,	},	//	t T
{	0x79,	0x59,	},	//	y Y
{	0x75,	0x55,	},	//	u U
{	0x69,	0x49,	},	//	i I
{	0x6F,	0x4F,	},	//	o O		/* 18 */
{	0x70,	0x50,	},	//	p P
{	0x5B,	0x7B,	},	//	[ {
{	0x5D,	0x7D,	},	//	] }
{	0x0D,	0x0D,	},	//	CR
{	0x00,	0x00,	},	//	CTRL --??--
{	0x61,	0x41,	},	//	a A
{	0x73,	0x53,	},	//	s S
{	0x64,	0x44,	},	//	d D		/* 20 */
{	0x66,	0x46,	},	//	f F
{	0x67,	0x47,	},	//	g G
{	0x68,	0x48,	},	//	h H
{	0x6A,	0x4A,	},	//	j J
{	0x6B,	0x4B,	},	//	k K
{	0x6C,	0x4C,	},	//	l L
{	0x3B,	0x3A,	},	//	; :
{	0x27,	0x22,	},	//	' "		/* 28 */
{	0x60,	0x7E,	},	//	` ~
{	0x00,	0x00,	},	//	LSHIFT --??--
{	0x5C,	0x7C,	},	//	\ |
{	0x7A,	0x5A,	},	//	z Z
{	0x78,	0x58,	},	//	x X
{	0x63,	0x43,	},	//	c C
{	0x76,	0x56,	},	//	v V
{	0x62,	0x42,	},	//	b B		/* 30 */
{	0x6E,	0x4E,	},	//	n N
{	0x6D,	0x4D,	},	//	m M
{	0x2C,	0x3C,	},	//	, <
{	0x2E,	0x3E,	},	//	. >
{	0x2F,	0x3F,	},	//	/ ?
{	0x00,	0x00,	},	//	RSHIFT --??--
{	0x00,	0x00,	},	//	PRTSC  --??--
{	0x00,	0x00,	},	//	ALT --??--	/* 38 */
{	0x20,	0x20,	},	//	SPACE
{	0x00,	0x00,	},	//	CAPS
{	EASCII_F1,	EASCII_F1,	},	//	F1
{	EASCII_F2,	EASCII_F2,	},	//	F2
{	EASCII_F3,	EASCII_F3,	},	//	F3
{	EASCII_F4,	EASCII_F4,	},	//	F4
{	EASCII_F5,	EASCII_F5,	},	//	F5
{	EASCII_F6,	EASCII_F6,	},	//	F6			/* 40 */
{	EASCII_F7,	EASCII_F7,	},	//	F7
{	EASCII_F8,	EASCII_F8,	},	//	F8
{	EASCII_F9,	EASCII_F9,	},	//	F9
{	EASCII_F10,	EASCII_F10,	},	//	F10
{	0x00,	0x00,			},	//	NUMLOCK
{	0x00,	0x00,			},	//	SCROLLLOCK
{	0x37,	EASCII_HOME,	},	//	KEYPAD HOME
{	0x38,	EASCII_UP,		},	//	KEYPAD UP	/* 48 */
{	0x39,	EASCII_PGUP,	},	//	KEYPAD PGUP
{	0x2D,	0x2D,			},	//	KEYPAD GREY -
{	0x34,	EASCII_LEFT,	},	//	KEYPAD LEFT
{	0x35,	EASCII_CENTER,	},	//	KEYPAD CENTER
{	0x36,	EASCII_RIGHT,	},	//	KEYPAD RIGHT
{	0x2B,	0x2B,			},	//	KEYPAD GREY +
{	0x31,	EASCII_END,		},	//	KEYPAD END
{	0x32,	EASCII_DOWN,	},	//	KEYPAD DOWN	/* 50 */
{	0x33,	EASCII_PGDN,	},	//	KEYPAD PGDN
{	0x30,	EASCII_INSERT,	},	//	KEYPAD INS
{	0x2E,	EASCII_DELETE,	},	//	KEYPAD DEL
{	0x00,	0x00,			},	//
{	0x00,	0x00,			},	//
{	0x00,	0x00,			},	//
{	EASCII_F11,	EASCII_F11,	},	//
{	EASCII_F12,	EASCII_F12,	},	//		/* 58 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* 60 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* 68 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* 70 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* 78 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//

	#if 0
{	0x00,	0x00,	},	//		/* 80 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* 88 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* 90 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* 98 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* A0 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* A8 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* B0 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* B8 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* C0 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* C8 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* D0 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* D8 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* E0 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* E8 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* F0 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* F8 */
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//
{	0x00,	0x00,	},	//		/* 100 */
	#endif
};

/*********************************************************************
 *
 * ConvertKeyToAscii
 *
 * SYNOPSIS
 *		short ConvertKeyToAscii (short keyvalue)
 *
 * PURPOSE
 *		Converts a Keyvalue returned by GetKeyNoWait to Ascii.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short ConvertKeyToAscii (short keyvalue)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ConvertKeyToAscii";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		USHORT	qual;
		UWORD	key;

		key  = keyvalue & 0xFF;
		qual = (keyvalue >> 8) & 0xFF;
		if (qual & QUAL_SHIFT) { /* if one shift pressed, set both bit flags*/
			qual |= (USHORT) QUAL_SHIFT;
		}


		if (key == 0x9C) {	/* Keypad Enter */
			return 0x0D;
		} else if (key == 0xB5) {	/* Keypad '/' */
			return '/';
		} else if (key >= 0x47 && key < 0x52) {
			return KeyToAsciiTable[key][(qual & KEYBIT_NUMLOCK) != KEYBIT_NUMLOCK];
		} else if (key < 0x80) {
			key = KeyToAsciiTable[key][(qual & QUAL_SHIFT) == QUAL_SHIFT];
			if (isalpha (key) && (qual & KEYBIT_CAPSLOCK)) {
				key = toupper (key);
			}
			return key;
		} else {
			switch (key) {
			case 0xD2:	/* Insert */
				return EASCII_INSERT;
			case 0xC7:	/* Home   */
				return EASCII_HOME;
			case 0xC9:	/* PageUp */
				return EASCII_PGUP;
			case 0xD3:	/* Delete */
				return EASCII_DELETE;
			case 0xCF:	/* End    */
				return EASCII_END;
			case 0xD1:	/* PageDn */
				return EASCII_PGDN;
			case 0xC8:	/* Up     */
				return EASCII_UP;
			case 0xCB:	/* Left   */
				return EASCII_LEFT;
			case 0xCD:	/* Right  */
				return EASCII_RIGHT;
			case 0xD0:	/* Down   */
				return EASCII_DOWN;
			}
			return 0;
		}
	}
	
} /* ConvertKeyToAscii */


/*********************************************************************
 *
 * OpenKeyboard
 *
 * SYNOPSIS
 *		short far32 OpenKeyboard (void)
 *
 * PURPOSE
 *		Setup and Initialize Keyboard Support
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short far32 OpenKeyboard (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenKeyboard";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return TRUE;
} /* OpenKeyboard */

/*********************************************************************
 *
 * CloseKeyboard
 *
 * SYNOPSIS
 *		void far32 CloseKeyboard (void)
 *
 * PURPOSE
 *		Free resources used by keyboard routines
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void far32 CloseKeyboard (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CloseKeyboard";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

} /* CloseKeyboard */

