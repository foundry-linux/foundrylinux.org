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
 * KEYBOARD.H
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 12/04/89
 *   MODIFIED : 07/28/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		
 *
 * HISTORY
 *
*/
#ifndef ECHIDNA_KEYBOARD_H
#define ECHIDNA_KEYBOARD_H

/***************************** I N C L U D E S ****************************/
#include <echidna/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <echidna/etypes.h>

/****************************** D E F I N E S *****************************/

/*
 * Echidna ASCII (extened codes for special keys)
 * Returned by ConvertKeyToAscii (GetKeyNoWait());
 */
#define EASCII_F1			0x101
#define EASCII_F2			0x102
#define EASCII_F3			0x103
#define EASCII_F4			0x104
#define EASCII_F5			0x105
#define EASCII_F6			0x106
#define EASCII_F7			0x107
#define EASCII_F8			0x108
#define EASCII_F9			0x109
#define EASCII_F10			0x10A
#define EASCII_F11			0x10B
#define EASCII_F12			0x10C
#define EASCII_INSERT		0x120
#define EASCII_HOME			0x121
#define EASCII_PGUP			0x122
#define EASCII_DELETE		0x123
#define EASCII_END			0x124
#define EASCII_PGDN			0x125
#define EASCII_UP			0x126
#define EASCII_LEFT			0x127
#define EASCII_RIGHT		0x128
#define EASCII_DOWN			0x129
#define EASCII_CENTER		0x12A

/* Commonly referenced ASCII key values that are not alphanumeric */
#define ASCII_BKSP		8
#define ASCII_TAB		9
#define ASCII_ESC		27
#define ASCII_RETURN	13

/* The following three masks should be used on the value returned by 
 * 	the function ShiftFlags().
 */
#define KEY_SHIFT	(0x01)
#define KEY_ALT		(0x04)
#define KEY_CTRL	(0x08)

#define KBUF_SIZE	30


#if __AMIGAOS__
/*
** Qualifier Data
*/
#define QUAL_SHIFT	(1L << 0)
#define QUAL_CTRL	(1L << 3)
#define QUAL_ALT	(1L << 4)
#define QUAL_KEYPAD	(1L << 8)
#elif __MSDOS__/*dcc*/
/*
** Qualifier Data
*/
#define QUAL_SHIFT	(1L << 0)
#define QUAL_CTRL	(1L << 2)
#define QUAL_ALT	(1L << 3)
#define QUAL_KEYPAD	(1L << 8)
#else
#error Need sompin
#endif

/********************************* GetKey *********************************/

#if 1
#define UP_KEY		(256 + 72)
#define DOWN_KEY	(256 + 80)
#define LEFT_KEY	(256 + 75)
#define RIGHT_KEY	(256 + 77)
#define	PGUP_KEY	(256 + 73)
#define	PGDN_KEY	(256 + 81)
#define BKSP_KEY	8
#define TAB_KEY		9
#define ESC_KEY		27
#define RETURN_KEY	13
#define DEL_KEY		(256 + 83)
#define HOME_KEY	(256 + 71)
#define END_KEY		(256 + 79)
#endif

/***************************** T Y P E D E F S ****************************/

#if __AMIGAOS__

typedef char KeyValue[10];

#elif __MSDOS__

typedef USHORT KeyValue;

#else
#error Need Support for this platform
#endif

/****************************** G L O B A L S *****************************/

#if 0
extern UBYTE far32 KeyTable[256];		/* Key not pressed = FALSE			*/
extern UBYTE far32 KeyLatchTable[256];	/* Key not pressed = FALSE			*/
extern UBYTE far32 KeyHitFlag;			/* Key has not been pressed = FALSE	*/
extern UBYTE far32 KeyDownFlag;			/* Key is not being pressed = FALSE	*/
extern UBYTE far32 LastKeyPressed;		/* Last Scan code read				*/
extern UBYTE far32 KeyBuffer[KBUF_SIZE];/* ScanCode Buffer					*/
extern UBYTE far32 ModBuffer[KBUF_SIZE];/* Modifier Buffer (SHIFT, ALT...)	*/
extern UBYTE far32 InPoint;				/* In pointer for key INPUT			*/
extern UBYTE far32 OutPoint;			/* Out Pointer for key OUTPUT		*/
#endif

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

#if __NOT_ANSI__		/** Non-ANSI Prototypes **/

#else					/** ANSI Prototypes **/

#if __MSDOS__

#include <conio.h>

//#define KeyWaiting()	(kbhit())

#else
#error Need support for this platform
#endif

extern void		 SaveKeyboardLocation(void);
extern void		 SetKeyboardEvents(short f);
extern void		 RestoreKeyboardLocation(void);

/* Good routines: */
extern short	far32 OpenKeyboard (void);
extern void		far32 CloseKeyboard (void);
extern UWORD	 GetKeyNoWait (void);
extern short	 ConvertKeyToAscii (short keyvalue);
extern void		 FlushKeyboardBuffer (void);

/* Evil routines: */
// extern short	 KeyPressed (UBYTE keycode);
// extern short	 KeyHit (void);
extern UWORD	 ShiftFlags (void);
extern short	 KeyWaiting(void);
extern short	 GetKey (void);

/* Good Routine to help poor soul that used Evil routines above */
extern UWORD	 ShiftFlagsFromScanCode (UWORD scancode);

#endif /** Function Prototypes **/

#ifdef __cplusplus
}
#endif

#endif /* ECHIDNA_KEYBOARD_H */
