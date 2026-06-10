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
 * COLORSEQ.H
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 02/21/92
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
 *		Color stuff.
 *
 *		Note: You will need to link with MWRTCRNG.ASM
 *	
 * HISTORY
 *
*/

#ifndef COLORSEQ_H
#define COLORSEQ_H

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/etypes.h>
#include <echidna/bilbm.h>
#include <echidna/timer.h>
#include <echidna/listfunc.h>

/**************************** C O N S T A N T S ***************************/

#define ONEPERTICK	16384
#define ONEPERSEC	(ONEPERTICK / 60)

#define MAX_CI_COLORS		256
#define MAX_HARDWARE_COLOR	255
#define MAX_COLOR_RANGE		256

/* CycleInfo Flag Bits */
#define CI_ENABLED		0x0001

/* Palette flags */
#define PAL_UPDATE		0x01
#define	PAL_CHANGED		0x02

#define COLORSEQ_HERTZ	60

/******************************** T Y P E S *******************************/

typedef struct {
	UBYTE	 Red;
	UBYTE	 Green;
	UBYTE	 Blue;
	UBYTE	 Flags;
} CSEQ_Color;

typedef struct {
	short	 Red;
	short	 Green;
	short	 Blue;
	UBYTE	 Flags;
	short	 Hue;
	short	 Saturation;
	short	 Value;
} CS_Color;

typedef struct {
	NodeType	 Node;
	short		 ID;
	CS_Color	 Color;
} CS_ColorTracker;

typedef struct {
	NodeType	 Node;
	short		 ID;
	UWORD		 Reg;
} CS_RegTracker;

typedef struct {
	NodeType		 Node;
	short			 Speed;			// Speed of Cycle
	short			 Direction;		// Direction
	short			 Timer;			// Runtime Overflow Timer
	UWORD			 Flags;			// Flags (CI_ENABLE)
	short			 ID;			// Position in List
	short			 NumColors;		// Number of Colors in Cycle
	CS_ColorTracker	*CurrentColor;	// Current Position in Cycle List (runtime)
	CS_ColorTracker	*EditColor;		// First displayed Color when editing
	ListType		 ColorListX;	// List of colors
	ListType		*ColorList;		// Pointer to List of colors
	short			 NumRegs;		// Numver of Registers in Cycle
	CS_RegTracker	*EditReg;		// First displayed Register when editing
	ListType		 RegListX;		// List of Regs
	ListType		*RegList;		// Pointer to list of Regs.
} CS_CycleInfo;

typedef struct {
	short			 NumColors;
	XTRAPntr		 pxtColors;
	UWORD			 CycleFlag;
	short			 NumCycles;
	CS_CycleInfo	*EditCycle;
	ListType		 CycleListX;
	ListType		*CycleList;
} ColorInfo;

/****************************** G L O B A L S *****************************/

extern short		 ColorRange[3];
extern short		 ColorMax[3];
extern short		 VBCycle;
extern short		 CyclePause;
extern ColorInfo	*GlobalColors;
extern CSEQ_Color	 GlobalPalette[MAX_CI_COLORS];

/******************************* M A C R O S ******************************/

#if 1
#define BOUNDCOLOR(val)		(((val) < 0) ? 0 : ((((val) > 255) ? 255 : (val))))
#define BOUNDCOLORh(val)	(((val) < 0) ? (val + 360) : ((((val) > 359) ? (val - 360) : (val))))
#else
#define	BOUNDCOLOR(val)		(val)
#define	BOUNDCOLORh(val)	(val)
#endif

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern void				 ConvertRGBtoHSV (CS_Color *cs);
extern void				 ConvertHSVtoRGB (CS_Color *cs);
extern void				 RenumberCS_Colors (ListType *ColorList);
extern void				 RenumberCS_Regs (ListType *RegList);
extern void				 RenumberCycleInfos (ListType *CycleList);
extern void				 RenumberColorInfo (ColorInfo *ci);
extern CS_CycleInfo		*CreateCycleInfo (void);
extern CS_RegTracker	*CreateRegTracker (void);
extern CS_ColorTracker	*CreateCycleColor (void);
extern void				 DeleteCycleColor (CS_ColorTracker *cc);
extern void				 DeleteRegTracker (CS_RegTracker *cr);
extern void				 DeleteCycleInfo (CS_CycleInfo *ci);
extern void				 InitColorInfo (ColorInfo *ci);
extern void				 ResetColorInfo (ColorInfo	*ci);
extern void				 DeleteColorInfo (ColorInfo	*ci);
extern ColorInfo		*CreateColorInfo (void);
extern short			 CopyColorInfoToColorInfo (
								ColorInfo	*src,
								ColorInfo	*dest
							);
extern ColorInfo		*DuplicateColorInfo (ColorInfo *ci);
extern ColorInfo		*DuplicateColorInfoInSTD (ColorInfo *ci);
extern InterruptRoutineType ComputeColorRanges (CSEQ_Color *cseqc, short force);
extern InterruptRoutineType ComputeNextPalette (CSEQ_Color *cseqc, ColorInfo *ci, short force);
extern void				 CloseColorseqInterrupt (void);
extern short			 OpenColorseqInterrupt (void);

#if __AMIGAOS__
extern void __far	__saveds	MyVBlank (void);
#elif __MSDOS__/*dcc*/
#else
#error NEED TO IMPLEMENT NON-LATTICE 5.0x MyVBlank().
#endif

extern void UpdateGlobalColors(void);
extern BOOL SetGlobalColors(ColorInfo *pcin);
extern void PushGlobalColors(void);
extern void PopGlobalColors(void);
extern BOOL IsGlobalColorsCopyOf(ColorInfo *pcin);

#endif /* COLORSEQ_H */
