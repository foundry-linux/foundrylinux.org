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
 * COLORREQ.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 02/20/92
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
 *		Echinda Color Requester. (Color Sequencer)
 *
 * HISTORY
 *	
 *		07/27/93 Tuesday (GAT) - added palette loading and saving
 *	
 *		12/04/92 Friday (GAT) - added copy from instance to instance
 *	
 *		04/07/92 Tuesday (dcc) - enhanced to store palettes (CS_ColorInfo,
 *								which are currently 2K big) in XTRA memory.
 *
 * TODO
 *
 *		* Load and Save HSV chunk
 *	
*/

#include <stdio.h>

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/strings.h>
#include <echidna/listfunc.h>
#include <echidna/emacros.h>
#include <echidna/eerrors.h>
#include <echidna/grafx.h>
#include <echidna/fonts.h>
#include <echidna/hitareas.h>
#include <echidna/windows.h>
#include <echidna/eui.h>
#include <echidna/edraw.h>
#include <echidna/events.h>
#include <echidna/mouse.h>
#include "keyboard.h" //<echidna/keyboard.h>
#include <echidna/xtramem.h>
#include <echidna/menus2.h>
#include <ctype.h>

#include "colorreq.h"					/* Verify prototypes. */
#include "colorseq.h"
#include "events.h"
#include "numbers.h"
#include "tuglbl.h"
#include "tumemain.h"

#include "szerror.h"

/* TOTAL KLUDGES begins */

extern short lastMouseY;
extern short lastMenuHeight;

extern HitArea GlobalAreas[];

/* TOTAL KLUDGES ends */

#define LOCAL static

/**************************** C O N S T A N T S ***************************/

#define UNDO_BUTTON			0

#define LOCAL_ONEPERTICK	128	/* Square Root of ONEPERTICK */
#define FIRSTCYCLECOLOR		232

#define CR_WHITE		0
#define CR_BLACK		1
#define CR_BLUE			2
#define CR_ORANGE		3

#define CR_WIDTH		252
#define CR_HEIGHT		188
#define CR_LEFT			((320 / 2) - (CR_WIDTH / 2))
#define CR_TOP			((((200 - 12) / 2) - (CR_HEIGHT / 2)) + 12)

#define PALETTE_LEFT	7
#define PALETTE_TOP		18
#define PALETTE_WIDTH	194
#define PALETTE_HEIGHT	58

#define PCOLOR_WIDTH	6
#define	PCOLOR_HEIGHT	7
#define PCOLORS_ACROSS	32
#define PCOLORS_DOWN	8

#define RGB_LEFT		209
#define RGB_TOP			14
#define RGB_WIDTH		32
#define RGB_HEIGHT		10
#define RGB_MODE		0
#define HSV_MODE		1

#define CARROW_HEIGHT	8
#define CUP_ARROW_TOP	(RGB_TOP + RGB_HEIGHT + 1)
#define CDN_ARROW_TOP	(CUP_ARROW_TOP + CARROW_HEIGHT + SLIDE_HEIGHT + 2)

#define SLIDE_WIDTH		10
#define SLIDE_HEIGHT	71
#define SLIDE_TOP		(CUP_ARROW_TOP + CARROW_HEIGHT + 1)
#define RSLIDE_LEFT		209
#define GSLIDE_LEFT		(209 + (SLIDE_WIDTH + 1) * 1)
#define BSLIDE_LEFT		(209 + (SLIDE_WIDTH + 1) * 2)
#define RVALUE_HEIGHT	19
#define SLIDE_POSITIONS	(SLIDE_HEIGHT - 8)

#define SLIDER_WIDTH	7
#define SLIDER_HEIGHT	5

#define BUTTON_WIDTH	40
#define BUTTON_HEIGHT	10
#define INSERT_LEFT		150
#define INSERT_TOP		139
#define COPY_LEFT		INSERT_LEFT
#define COPY_TOP		(INSERT_TOP + BUTTON_HEIGHT + 1)
#define SPREAD_LEFT		INSERT_LEFT
#define SPREAD_TOP		(COPY_TOP + BUTTON_HEIGHT + 1)
#define UNDO_LEFT		INSERT_LEFT
#define UNDO_TOP		(SPREAD_TOP + BUTTON_HEIGHT + 1)
#define DELETE_LEFT		(INSERT_LEFT + BUTTON_WIDTH + 1)
#define DELETE_TOP		INSERT_TOP
#define SWAP_LEFT		DELETE_LEFT
#define SWAP_TOP		COPY_TOP
#define BLEND_LEFT		DELETE_LEFT
#define BLEND_TOP		SPREAD_TOP
#define CANCEL_LEFT		DELETE_LEFT
#define CANCEL_TOP		UNDO_TOP
#define CANCEL_ID		100

#define	OK_LEFT			232
#define OK_TOP			150
#define OK_WIDTH		10
#define OK_HEIGHT		32
#define OK_ID			101

#define LISTCOLOR_WIDTH		6
#define LISTCOLOR_HEIGHT	6
#define EDIT_LEFT			19
#define EDIT_WIDTH			99
#define REGS_TOP			87
#define REGS_HEIGHT			29
#define COLORS_TOP			123
#define COLORS_HEIGHT		9
#define	CYCLES_TOP			139
#define	CYCLES_HEIGHT		16
#define NUM_LEFT			(EDIT_LEFT + EDIT_WIDTH + 1 + ARROW_WIDTH + 2)
#define NUM_WIDTH			21
#define NUM_HEIGHT			9

#define ARROW_WIDTH		11
#define ARROW_HEIGHT	9
#define LARROW_LEFT		(EDIT_LEFT - ARROW_WIDTH - 1)
#define RARROW_LEFT		(EDIT_LEFT + EDIT_WIDTH + 1)

#define SPEED_LEFT		7
#define SPEED_TOP		163
#define SPEED_WIDTH		78
#define SPEED_HEIGHT	8
#define SPSLIDER_WIDTH	5
#define SPSLIDER_HEIGHT	7
#define SPEED_POSITIONS	(SPEED_WIDTH - 8)

#define CFLAG_LEFT		7
#define CFLAG_TOP		172
#define CFLAG_WIDTH		56
#define CFLAG_HEIGHT	10

#define DIR_LEFT		64
#define DIR_TOP			172
#define DIR_WIDTH		21
#define DIR_HEIGHT		10

#define FOCUS_NONE		0
#define	FOCUS_PALETTE	1
#define FOCUS_REGS		2
#define FOCUS_COLORS	3
#define FOCUS_CYCLES	4
#define	FOCUS_CCBUFFER	5

#define AFFECT_NONE		0
#define AFFECT_COPY		1
#define AFFECT_BLEND	2
#define AFFECT_SWAP		3

/******************************** T Y P E S *******************************/



/****************************** G L O B A L S *****************************/

LOCAL XTRAPntr		 CCBuffer;
LOCAL short			 CCCount;

LOCAL Window		*CRWin;
LOCAL Font			*CRFont;
LOCAL UBYTE			 CrColors[4];

LOCAL ColorInfo		*EditCI;
LOCAL CS_Color		*pcscEdit;

LOCAL ColorInfo		*ciUndo = NULL;

LOCAL short			 CRdone = TRUE;	/* CHEAT: also flag for requester is down */
LOCAL short			 CRabort;

LOCAL short			 ColorFocus = FOCUS_NONE;
LOCAL short			 SlideMode  = RGB_MODE;
LOCAL short			 SpreadMode = RGB_MODE;

/* Routine Pointer for Downloader */
void (*UpdateHrdwrColorRoutinePtr)(ColorInfo *ci);

/* Palette Dividers */
short				 ColorReqDivideX;
short				 ColorReqDivideY;

/* Palette */
LOCAL short			 PalX1 = 31;
LOCAL short			 PalY1 = 5;
LOCAL short			 PalX2 = 31;
LOCAL short			 PalY2 = 5;
LOCAL short			 PalStart;
LOCAL short			 PalEnd;

/* Sliders */
LOCAL short			 RGBSliderPos[3];
LOCAL short			 RGBSliderOld[3];
LOCAL CS_Color		 KludgeCS;

/* Cycles */
LOCAL short			 FirstCycle;
LOCAL short			 LastCycle;
LOCAL short			 CurrentCycle;
LOCAL CS_CycleInfo	*CurrentEditCycle;

/* Colors */
LOCAL short			 FirstColor;
LOCAL short			 LastColor;

/* Regs */
LOCAL short			 FirstReg;
LOCAL short			 LastReg;

/* Affect */
LOCAL short			 OldAffectVal[3];
LOCAL short			 AffectVal[3];
LOCAL short			 Spreading;

/* Speed */
LOCAL short			 OldSpeed;

/* GetNext */
LOCAL short			 SrcFirst;
LOCAL short			 SrcLast;
LOCAL short			 SrcColor;
LOCAL short			 SrcDir;
LOCAL CS_RegTracker	*SrcCSrt;

LOCAL short			 DstFocus;
LOCAL short			 DstFirst;
LOCAL short			 DstLast;
LOCAL short			 DstColor;
LOCAL short			 DstDir;

/* Affect Dest (Copy,Swap,Blend) */
LOCAL short			 AffectMode;
LOCAL short			 OneColor;	// Used to tell whether to do relative
LOCAL short			 OldOneClr;	// color adjusting or direct.

LOCAL PopupItem		 ColorPopupX;
PopupItem		*ColorPopup = &ColorPopupX;	/* should be LOCAL but for COLORREI.C */

/******************************* M A C R O S ******************************/

#define ResetAffect()	{ AffectVal[0] = AffectVal[1] = AffectVal[2] = (-1); }

#define ED_Button(x,y,w,h)	\
	ED_Rect ((x), (y), (w) - 1, (h) -1),				\
	ED_RelHLine ((x) + 1, (w) - 1, (y) + (h) - 1),		\
	ED_RelVLine ((x) + (w) - 1, (y) + 1, (h) - 1)		\

/******************************* T A B L E S ******************************/

LOCAL short SliderLeft[] = { 
	CR_LEFT + RSLIDE_LEFT + 1,
	CR_LEFT + GSLIDE_LEFT + 1,
	CR_LEFT + BSLIDE_LEFT + 1,
};

LOCAL char *s0  = "COLOR SEQUENCER";
LOCAL char *s1  = "CYCLE";
LOCAL char *s2  = "REGS";
LOCAL char *s4  = "COLORS";
LOCAL char *s5  = "INSERT";
LOCAL char *s6  = "DELETE";
LOCAL char *s7  = "COPY";
LOCAL char *s8  = "SWAP";
LOCAL char *s9  = "SPREAD";
LOCAL char *s10 = "BLEND";
LOCAL char *s11 = "UNDO";
LOCAL char *s12 = "CANCEL";
LOCAL char *s13 = "O";
LOCAL char *s14 = "K";
LOCAL char *s15 = "CYCLES";
LOCAL char *s16 = "SPEED";
LOCAL char *s17 = "DIRECTION";
LOCAL char *s18 = "HUSPRD";

LOCAL char **CrStr[] = {
#define	COLOR_SEQUENCER_STR	0
	&s0,
#define CYCLE_STR			1
	&s1,
#define REGS_STR			2
	&s2,
#define COLORS_STR			3
	&s4,
#define INSERT_STR			4
	&s5,
#define DELETE_STR			5
	&s6,
#define COPY_STR			6
	&s7,
#define SWAP_STR			7
	&s8,
#define SPREAD_STR			8
	&s9,
#define BLEND_STR			9
	&s10,
#define UNDO_STR			10
	&s11,
#define CANCEL_STR			11
	&s12,
#define O_STR				12
	&s13,
#define K_STR				13
	&s14,
#define CYCLES_STR			14
	&s15,
#define SPEED_STR			15
	&s16,
#define DIRECTION_STR		16
	&s17,
#define HUESPREAD_STR		17
	&s18,
};

LOCAL struct TextFont **CrFonts[] = {
	(struct TextFont **)&CRFont,
};

#define DOT_EPIC	0
LOCAL EPic Dot[] = {
	ED_Plot (0, 0),
	ED_End,
};

LOCAL EPic PalGrid0[] = {
	ED_ForX (1, 30, 1, 6, DOT_EPIC),
	ED_End,
};

LOCAL EPic PalGrid1[] = {
	ED_ForX (1, 7, 1, 24, DOT_EPIC),
	ED_End,
};

LOCAL EPic SlidGrid0[] = {
	ED_ForY (1, 32, 1, 2, DOT_EPIC),
	ED_End,
};

LOCAL EPic SlidGrid1[] = {
	ED_ForY (1, 7, 1, 8, DOT_EPIC),
	ED_End,
};

LOCAL EPic Button[] = {
	ED_Button (0, 0, BUTTON_WIDTH, BUTTON_HEIGHT),
	ED_End,
};


LOCAL EPic RGBEPic[] = {
	ED_Button (RGB_LEFT, RGB_TOP, RGB_WIDTH, RGB_HEIGHT),
	ED_End,
};

LOCAL EPic RArrow[] = {
	ED_Button (0, 0, ARROW_WIDTH, ARROW_HEIGHT),
	ED_Plot (5, 1),
	ED_RelHLine (5, 2, 2),
	ED_RelHLine (2, 6, 3),
	ED_RelHLine (2, 6, 4),
	ED_RelHLine (5, 2, 5),
	ED_Plot (5, 6),
	ED_End,
};

LOCAL EPic LArrow[] = {
	ED_Button (0, 0, ARROW_WIDTH, ARROW_HEIGHT),
	ED_Plot (4, 1),
	ED_RelHLine (3, 2, 2),
	ED_RelHLine (2, 6, 3),
	ED_RelHLine (2, 6, 4),
	ED_RelHLine (3, 2, 5),
	ED_Plot (4, 6),
	ED_End,
};

LOCAL EPic Slide[] = {
	/* Up Arrow */
	ED_Button (0, 0, SLIDE_WIDTH, CARROW_HEIGHT),
	ED_Plot (4, 1),
	ED_RelHLine (3, 3, 2),
	ED_RelHLine (2, 5, 3),
	ED_RelHLine (1, 7, 4),
	ED_RelHLine (3, 3, 5),

	/* Slider */
	ED_DrawMode (JAM2),
	ED_Button (0, CARROW_HEIGHT + 1, SLIDE_WIDTH, SLIDE_HEIGHT),
	ED_DrawMode (JAM1),

	/* Down Arrow */
	ED_Button (0, CARROW_HEIGHT + 1 + SLIDE_HEIGHT + 1, SLIDE_WIDTH, CARROW_HEIGHT),
	ED_Plot (4, CARROW_HEIGHT + 1 + SLIDE_HEIGHT + 1 + 5),
	ED_RelHLine (3, 3, CARROW_HEIGHT + 1 + SLIDE_HEIGHT + 1 + 4),
	ED_RelHLine (2, 5, CARROW_HEIGHT + 1 + SLIDE_HEIGHT + 1 + 3),
	ED_RelHLine (1, 7, CARROW_HEIGHT + 1 + SLIDE_HEIGHT + 1 + 2),
	ED_RelHLine (3, 3, CARROW_HEIGHT + 1 + SLIDE_HEIGHT + 1 + 1),

	/* Value */
	ED_RelVLine (0, CARROW_HEIGHT * 2 + 2 + SLIDE_HEIGHT + 1, RVALUE_HEIGHT),
	ED_RelVLine (SLIDE_WIDTH - 2, CARROW_HEIGHT * 2 + 2 + SLIDE_HEIGHT + 1, RVALUE_HEIGHT),
	ED_RelHLine (0, SLIDE_WIDTH - 2, CARROW_HEIGHT * 2 + 2 + SLIDE_HEIGHT + 1 + RVALUE_HEIGHT - 1),

	ED_End,
};

LOCAL EPic *CrEPics[] = {
#define DOT_EPIC		0
	Dot,
#define SLIDE_EPIC		1
	Slide,
#define LARROW_EPIC		2
	LArrow,
#define RARROW_EPIC		3
	RArrow,
#define BUTTON_EPIC		4
	Button,
#define SLIDEGRID1_EPIC	5
	SlidGrid1,
#define SLIDEGRID0_EPIC	6
	SlidGrid0,
#define PALGRID1_EPIC	7
	PalGrid1,
#define PALGRID0_EPIC	8
	PalGrid0,
#define RGB_EPIC		9
	RGBEPic,
};

LOCAL EPic CSEPic[] = {
	ED_Font (0),
	ED_DrawMode (JAM2),
	ED_FgColorTbl (CR_WHITE),
	ED_Rect (0, 0, CR_WIDTH, CR_HEIGHT),
	ED_DrawMode (JAM1),
	ED_FgColorTbl (CR_BLACK),
	ED_Button (0, 0, CR_WIDTH, CR_HEIGHT),
	ED_RelHLine (1,CR_WIDTH - 1, 8),
	ED_PosText (82, 2, COLOR_SEQUENCER_STR),

	/* Palette */
	ED_Rect (PALETTE_LEFT,PALETTE_TOP, PALETTE_WIDTH, PALETTE_HEIGHT),
	ED_Move (PALETTE_LEFT + 7, PALETTE_TOP - 2),
	ED_Gosub (PALGRID0_EPIC),
	ED_Move (PALETTE_LEFT + 7, PALETTE_TOP + PALETTE_HEIGHT + 1),
	ED_Gosub (PALGRID0_EPIC),
	ED_Move (PALETTE_LEFT + 25, PALETTE_TOP - 3),
	ED_Gosub (PALGRID1_EPIC),
	ED_Move (PALETTE_LEFT + 25, PALETTE_TOP + PALETTE_HEIGHT + 2),
	ED_Gosub (PALGRID1_EPIC),
	ED_Plot (PALETTE_LEFT + 97, PALETTE_TOP - 4),
	ED_Plot (PALETTE_LEFT + 97, PALETTE_TOP + PALETTE_HEIGHT + 3),

	/* Sliders */
	ED_Move (0, 0),
	ED_Gosub (RGB_EPIC),
	ED_Move (RSLIDE_LEFT, CUP_ARROW_TOP),
	ED_ForX (1, 3, 1, SLIDE_WIDTH + 1, SLIDE_EPIC),

	/* Slider Grids */
	ED_Move (RSLIDE_LEFT - 2, SLIDE_TOP + 4),
	ED_Gosub (SLIDEGRID0_EPIC),
	ED_Move (BSLIDE_LEFT + SLIDE_WIDTH + 1, SLIDE_TOP + 4),
	ED_Gosub (SLIDEGRID0_EPIC),
	ED_Move (RSLIDE_LEFT - 3, SLIDE_TOP + 4 + 6),
	ED_Gosub (SLIDEGRID1_EPIC),
	ED_Move (BSLIDE_LEFT + SLIDE_WIDTH + 2, SLIDE_TOP + 4 + 6),
	ED_Gosub (SLIDEGRID1_EPIC),
	ED_Plot (RSLIDE_LEFT - 4, SLIDE_TOP + 4 + 6 + 8 * 3),
	ED_Plot (BSLIDE_LEFT + SLIDE_WIDTH + 3, SLIDE_TOP + 4 + 6 + 8 * 3),

	/* Buttons */
	#if UNDO_BUTTON
	ED_Move (INSERT_LEFT, INSERT_TOP),
	ED_ForY (1, 4, 1, BUTTON_HEIGHT + 1, BUTTON_EPIC),
	#else
	ED_Move (INSERT_LEFT, INSERT_TOP),
	ED_ForY (1, 3, 1, BUTTON_HEIGHT + 1, BUTTON_EPIC),
	#endif
	ED_Move (DELETE_LEFT, DELETE_TOP),
	ED_ForY (1, 4, 1, BUTTON_HEIGHT + 1, BUTTON_EPIC),
	ED_Button (OK_LEFT, OK_TOP, OK_WIDTH, OK_HEIGHT),
	ED_PosText (INSERT_LEFT + 2, INSERT_TOP + 2, INSERT_STR),
	ED_PosText (DELETE_LEFT + 2, DELETE_TOP + 2, DELETE_STR),
	ED_PosText (COPY_LEFT + 2, COPY_TOP + 2, COPY_STR),
	ED_PosText (SWAP_LEFT + 2, SWAP_TOP + 2, SWAP_STR),
	ED_PosText (SPREAD_LEFT + 2, SPREAD_TOP + 2, SPREAD_STR),
	ED_PosText (BLEND_LEFT + 2, BLEND_TOP + 2, BLEND_STR),
	#if UNDO_BUTTON
	ED_PosText (UNDO_LEFT + 2, UNDO_TOP + 2, UNDO_STR),
	#endif
	ED_PosText (CANCEL_LEFT + 2, CANCEL_TOP + 2, CANCEL_STR),
	ED_PosText (OK_LEFT + 2, OK_TOP + 2, O_STR),
	ED_PosText (OK_LEFT + 2, OK_TOP + 2 + 6, K_STR),

	/* Edit Arrows */
	ED_Move (LARROW_LEFT, REGS_TOP),
	ED_Gosub (LARROW_EPIC),
	ED_Move (LARROW_LEFT, COLORS_TOP),
	ED_Gosub (LARROW_EPIC),
	ED_Move (LARROW_LEFT, CYCLES_TOP),
	ED_Gosub (LARROW_EPIC),
	ED_Move (RARROW_LEFT, REGS_TOP),
	ED_Gosub (RARROW_EPIC),
	ED_Move (RARROW_LEFT, COLORS_TOP),
	ED_Gosub (RARROW_EPIC),
	ED_Move (RARROW_LEFT, CYCLES_TOP),
	ED_Gosub (RARROW_EPIC),

	/* Cycle Areas */
	ED_Button (EDIT_LEFT, REGS_TOP, EDIT_WIDTH, REGS_HEIGHT),
	ED_RelHLine (EDIT_LEFT, EDIT_WIDTH, REGS_TOP + 7),
	ED_Button (EDIT_LEFT, COLORS_TOP, EDIT_WIDTH, COLORS_HEIGHT),
	ED_Button (EDIT_LEFT, CYCLES_TOP, EDIT_WIDTH, CYCLES_HEIGHT),
	ED_PosText (EDIT_LEFT, CYCLES_TOP + CYCLES_HEIGHT + 1, CYCLES_STR),

	/* Cycle Numbers */
	ED_Rect (NUM_LEFT, REGS_TOP, NUM_WIDTH, NUM_HEIGHT),
	ED_PosText (NUM_LEFT + NUM_WIDTH + 1, REGS_TOP, CYCLE_STR),
	ED_PosText (NUM_LEFT + NUM_WIDTH + 1, REGS_TOP + 6, REGS_STR),
	ED_Rect (NUM_LEFT, COLORS_TOP, NUM_WIDTH, NUM_HEIGHT),
	ED_PosText (NUM_LEFT + NUM_WIDTH + 1, COLORS_TOP, CYCLE_STR),
	ED_PosText (NUM_LEFT + NUM_WIDTH + 1, COLORS_TOP + 6, COLORS_STR),

	/* Speed */
	ED_DrawMode (JAM2),
	ED_Button (SPEED_LEFT, SPEED_TOP, SPEED_WIDTH, SPEED_HEIGHT),
	ED_DrawMode (JAM1),
	ED_PosText (SPEED_LEFT + SPEED_WIDTH + 3, SPEED_TOP + 2, SPEED_STR),

	/* Cycle Flag */
	ED_Button (CFLAG_LEFT, CFLAG_TOP, CFLAG_WIDTH, CFLAG_HEIGHT),
	ED_PosText (CFLAG_LEFT + 2, CFLAG_TOP + 2, CYCLE_STR),

	/* Direction */
	ED_Button (DIR_LEFT, DIR_TOP, DIR_WIDTH, DIR_HEIGHT),
	ED_PosText (DIR_LEFT + DIR_WIDTH + 3, DIR_TOP + 2, DIRECTION_STR),

	ED_End,
};

LOCAL EPicture CREPicture = {
	CSEPic,
	CrFonts,
	CrStr,
	CrEPics,
	NULL,
	CrColors,
};

/***************************** R O U T I N E S ****************************/

#if 0
void PrintMsg (char *s) {
	SetPenColor (255);
	DrawRect (0,0, 320, 10);
	DrawString (&SmallFont, 1, 1, s);
}
#endif

/*********************************************************************
 *
 * UpdateHardwareColors
 *
 * SYNOPSIS
 *		void UpdateHardwareColors (CSEQ_Color *cseqc, short force)
 *
 * PURPOSE
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
LOCAL void UpdateHardwareColors (CSEQ_Color *cseqc, short force)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateHardwareColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

		ComputeColorRanges (cseqc, force);
		if (UpdateHrdwrColorRoutinePtr) {
			UpdateHrdwrColorRoutinePtr (EditCI);
		}
} /* UpdateHardwareColors */


/*********************************************************************
 *
 * RedrawTitle
 *
 * SYNOPSIS
 *		LOCAL void RedrawTitle (short color)
 *
 * PURPOSE
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
LOCAL void RedrawTitle (short color)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RedrawTitle";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		static short	 oldcolor;
		static char		*titles[] = {
			"COLOR SEQUENCER",
			"COPY TO->",
			"BLEND WITH->",
			"SWAP WITH->",
		};

		BeforeGraphics ();

		if (color >= 0) {
			SetPenColor (color);
			oldcolor = color;
		} else {
			SetPenColor (oldcolor);
		}

		DrawRect (CR_LEFT + 1, CR_TOP + 1, CR_WIDTH - 3, 7);
		DrawString (CRFont, CR_LEFT + 82, CR_TOP + 2, titles[AffectMode]);

		AfterGraphics ();
	}
} /* RedrawTitle */

/*********************************************************************
 *
 * RedrawPalTitle
 *
 * SYNOPSIS
 *		LOCAL void RedrawPalTitle (short color)
 *
 * PURPOSE
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
LOCAL void RedrawPalTitle (short color)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RedrawPalTitle";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	BeforeGraphics ();

	RedrawTitle (color);

	{
		char	line[40];

		sprintf (line, "COLOR $%02x %3d", color, color);
		DrawString (CRFont, CR_LEFT + 2, CR_TOP + 2, line);
	}

	AfterGraphics ();

} /* RedrawPalTitle */

/*********************************************************************
 *
 * FindColorNode
 *
 * SYNOPSIS
 *		LOCAL void *FindColorNode (ListType *list, short count)
 *
 * PURPOSE
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
LOCAL void *FindColorNode (ListType *list, short count)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindColorNode";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		NodeType	*nd;

		nd = Head (list);
		for (; count > 0 && !EndOfList (nd); count--) {
			nd = Next (nd);
		}
		return nd;
	}
} /* FindColorNode */

/*********************************************************************
 *
 * DrawRightArrow
 *
 * SYNOPSIS
 *		LOCAL void DrawRightArrow (short x, short y)
 *
 * PURPOSE
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
LOCAL void DrawRightArrow (short x, short y)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawRightArrow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		BeforeGraphics ();
		
		SetPenColor (CrColors[CR_BLACK]);
		DrawRect (x, y + 2, 5, 2);
		DrawRect (x + 2, y, 1, 6);
		DrawRect (x + 3, y + 1, 1, 4);
		AfterGraphics ();
	}
} /* DrawRightArrow */

/*********************************************************************
 *
 * DrawLeftArrow
 *
 * SYNOPSIS
 *		LOCAL void DrawLeftArrow (short x, short y)
 *
 * PURPOSE
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
LOCAL void DrawLeftArrow (short x, short y)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawLeftArrow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		BeforeGraphics ();
		
		SetPenColor (CrColors[CR_BLACK]);
		DrawRect (x, y + 2, 5, 2);
		DrawRect (x + 2, y, 1, 6);
		DrawRect (x + 1, y + 1, 1, 4);
		AfterGraphics ();
	}
} /* DrawLeftArrow */

/*********************************************************************
 *
 * DrawDownArrow
 *
 * SYNOPSIS
 *		LOCAL void DrawDownArrow (short x, short y)
 *
 * PURPOSE
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
LOCAL void DrawDownArrow (short x, short y)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawDownArrow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		BeforeGraphics ();
		
		SetPenColor (CrColors[CR_BLACK]);
		DrawRect (x + 2, y, 2, 5);
		DrawRect (x, y + 2, 6, 1);
		DrawRect (x + 1, y + 3, 4, 1);
		AfterGraphics ();
	}
} /* DrawDownArrow */

/*********************************************************************
 *
 * DrawListEnd
 *
 * SYNOPSIS
 *		LOCAL void DrawListEnd (short x, short y)
 *
 * PURPOSE
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
LOCAL void DrawListEnd (short x, short y)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawListEnd";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		BeforeGraphics ();

		SetPenColor (CrColors[CR_WHITE]);
		DrawRect (x, y, LISTCOLOR_WIDTH, LISTCOLOR_HEIGHT);
		SetPenColor (CrColors[CR_BLACK]);
		DrawPixel (x + 2, y + 1);
		DrawPixel (x + 4, y + 1);
		DrawPixel (x + 1, y + 2);
		DrawPixel (x + 3, y + 2);
		DrawPixel (x + 2, y + 3);
		DrawPixel (x + 4, y + 3);
		DrawPixel (x + 1, y + 4);
		DrawPixel (x + 3, y + 4);
		AfterGraphics ();
	}
} /* DrawListEnd */

/*********************************************************************
 *
 * RedrawSpread
 *
 * SYNOPSIS
 *		LOCAL void RedrawSpread (void)
 *
 * PURPOSE
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
LOCAL void RedrawSpread (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RedrawSpread";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		BeforeGraphics ();

		SetPenColor (CrColors[CR_WHITE]);
		DrawRect (CR_LEFT + SPREAD_LEFT + 1,
				  CR_TOP  + SPREAD_TOP  + 1,
				  BUTTON_WIDTH - 3, BUTTON_HEIGHT - 3);
		DrawString (CRFont, CR_LEFT + SPREAD_LEFT + 2,
					 CR_TOP + SPREAD_TOP + 2,
					 ((SpreadMode == RGB_MODE) ? s9 : s18));
		AfterGraphics ();
	}
} /* RedrawSpread */

/*********************************************************************
 *
 * ResetSpread
 *
 * SYNOPSIS
 *		LOCAL void ResetSpread (void)
 *
 * PURPOSE
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
LOCAL void ResetSpread (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ResetSpread";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SpreadMode = RGB_MODE;
	RedrawSpread ();

} /* ResetSpread */


/*********************************************************************
 *
 * DrawNumber
 *
 * SYNOPSIS
 *		LOCAL void DrawNumber (short x, short y, short val)
 *
 * PURPOSE
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
LOCAL void DrawNumber (short x, short y, short val)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawNumber";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		char	value[6];

		BeforeGraphics ();
		SetPenColor (CrColors[CR_WHITE]);
		DrawRect (x, y, 6 * 3, 5);

		sprintf (value, "%03d", val);
		DrawStringN (CRFont, x, y, value, 3);
		AfterGraphics ();
	}
} /* DrawNumber */

/*********************************************************************
 *
 * XORBox
 *
 * SYNOPSIS
 *		LOCAL void XORBox (short x, short y, short w, short h)
 *
 * PURPOSE
 *		XOR an UNFILLED rectangle.
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
LOCAL void XORBox (short x, short y, short w, short h)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "XORBox";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (w == 1 || h == 1) {
		XORRect (x, y, w, h);
	} else {
		XORRect (x, y, w, 1);
		XORRect (x, y + h - 1, w, 1);
		if (h > 2) {
			XORRect (x, y + 1, 1, h - 2);
			XORRect (x + w - 1, y + 1, 1, h -2);
		}
	}

} /* XORBox */

/*********************************************************************
 *
 * DrawPaletteBox
 *
 * SYNOPSIS
 *		LOCAL void DrawPaletteBox (void)
 *
 * PURPOSE
 *		Draw Selection box for palette. (XOR it so it can be erased by
 *		calling the same routine.
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
LOCAL void DrawPaletteBox (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawPaletteBox";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		short	x1;
		short	y1;
		short	x2;
		short	y2;
		short	w;
		short	h;

		if (PalX1 <= PalX2) {
			x1 = PalX1;
			y1 = PalY1;
			x2 = PalX2;
			y2 = PalY2;
		} else {
			x1 = PalX2;
			y1 = PalY2;
			x2 = PalX1;
			y2 = PalY1;
		}

		BeforeGraphics ();
		SetPenColor (255);

		/*

		cases

		1  2  3  4  5  6
		O   O OO OO  O  OO
		O     OO OO OO OOO
		O  O  OO O  OO OO


		*/

		//
		// Case 1, 3
		//
		if (x1 == x2 || (y1 == 0 && y2 == 7))
		{
			x1 = min (PalX1, PalX2);
			y1 = min (PalY1, PalY2);
			x2 = max (PalX1, PalX2);
			y2 = max (PalY1, PalY2);
		
			w  = x2 - x1 + 1;
			h  = y2 - y1 + 1;

			XORBox (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					CR_TOP  + PALETTE_TOP  + 1 + y1 * PCOLOR_HEIGHT,
					w * PCOLOR_WIDTH,
					h * PCOLOR_HEIGHT);
		}
		//
		// Case 2
		//
		else if (x1 + 1 == x2 && y1 > y2)
		{
			XORBox (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					CR_TOP  + PALETTE_TOP  + 1 + y1 * PCOLOR_HEIGHT,
					PCOLOR_WIDTH, (8 - y1) * PCOLOR_HEIGHT);
			XORBox (CR_LEFT + PALETTE_LEFT + 1 + x2 * PCOLOR_WIDTH,
					CR_TOP  + PALETTE_TOP  + 1,
					PCOLOR_WIDTH, (y2 + 1) * PCOLOR_HEIGHT);
		}
		//
		// Case 4
		//
		else if (y1 == 0)
		{
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1,
					 1, 8 * PCOLOR_HEIGHT);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1,
					 (x2 - x1 + 1) * PCOLOR_WIDTH, 1);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + (x2 + 1) * PCOLOR_WIDTH - 1,
					 CR_TOP  + PALETTE_TOP  + 1,
					 1, (y2 + 1) * PCOLOR_HEIGHT);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x2 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1 + (y2 + 1) * PCOLOR_HEIGHT - 1,
					 PCOLOR_WIDTH, 1);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x2 * PCOLOR_WIDTH - 1,
					 CR_TOP  + PALETTE_TOP  + 1 + (y2 + 1) * PCOLOR_HEIGHT,
					 1,  (7 - y2) * PCOLOR_HEIGHT);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1 + 8 * PCOLOR_HEIGHT - 1,
					 (x2 - x1) * PCOLOR_WIDTH,  1);
		}
		//
		// Case 5
		//
		else if (y2 == 7)
		{
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + (x1 + 1) * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1,
					 1, y1 * PCOLOR_HEIGHT);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1 + y1 * PCOLOR_HEIGHT,
					 PCOLOR_WIDTH, 1);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1 + y1 * PCOLOR_HEIGHT,
					 1, (8 - y1) * PCOLOR_HEIGHT);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1 + 8  * PCOLOR_HEIGHT - 1,
					 (x2 - x1 + 1) * PCOLOR_WIDTH, 1);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + (x2 + 1) * PCOLOR_WIDTH - 1,
					 CR_TOP  + PALETTE_TOP  + 1,
					 1,  8 * PCOLOR_HEIGHT);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + (x1 + 1) * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1,
					 (x2 - x1) * PCOLOR_WIDTH,  1);
		}
		//
		// Case 6
		//
		else
		{
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + (x1 + 1) * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1,
					 (x2 - x1) * PCOLOR_WIDTH, 1);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + (x2 + 1) * PCOLOR_WIDTH - 1,
					 CR_TOP  + PALETTE_TOP  + 1,
					 1, (y2 + 1) * PCOLOR_HEIGHT);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x2 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1 + (y2 + 1) * PCOLOR_HEIGHT - 1,
					 PCOLOR_WIDTH, 1);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x2 * PCOLOR_WIDTH - 1,
					 CR_TOP  + PALETTE_TOP  + 1 + (y2 + 1) * PCOLOR_HEIGHT,
					 1, (7 - y2) * PCOLOR_HEIGHT);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1 + 8  * PCOLOR_HEIGHT - 1,
					 (x2 - x1) * PCOLOR_WIDTH, 1);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1 + y1 * PCOLOR_HEIGHT,
					 1, (8 - y1) * PCOLOR_HEIGHT);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + x1 * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1 + y1 * PCOLOR_HEIGHT,
					 PCOLOR_WIDTH, 1);
			XORRect (CR_LEFT + PALETTE_LEFT + 1 + (x1 + 1) * PCOLOR_WIDTH,
					 CR_TOP  + PALETTE_TOP  + 1,
					 1, y1 * PCOLOR_HEIGHT);
		}


		#if 0
		{
			char line[80];

			sprintf (line, "px1 = %3d py1 = %3d px2 = %3d py2 = %3d",
				PalX1, PalY1, PalX2, PalY2);
			SetPenColor (255);
			DrawRect (0,0, 320, 10);
			DrawString (&SmallFont, 1, 1, line);
		}
		#endif
		AfterGraphics ();
	}
} /* DrawPaletteBox */

/*********************************************************************
 *
 * EraseArrow
 *
 * SYNOPSIS
 *		LOCAL void EraseArrow (short y)
 *
 * PURPOSE
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
LOCAL void EraseArrow (short y)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "EraseArrow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		BeforeGraphics ();

		SetPenColor (CrColors[CR_WHITE]);
		DrawRect (CR_LEFT + EDIT_LEFT + 1, y, LISTCOLOR_WIDTH * 16, 6);

		AfterGraphics ();
	}
} /* EraseArrow */

/*********************************************************************
 *
 * UpdateArrow
 *
 * SYNOPSIS
 *		LOCAL void UpdateArrow (short x1, short x2)
 *
 * PURPOSE
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
LOCAL void UpdateArrow (short x1, short x2, short y)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateArrow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		short	lx = min (x1, x2);
		short	rx = max (x1, x2);

		BeforeGraphics ();

		EraseArrow (y);
		SetPenColor (CrColors[CR_BLACK]);
		if (lx < 0 && rx < 0) {
			DrawLeftArrow (CR_LEFT + EDIT_LEFT + 1, y);
		} else if (lx > 15 && rx > 15) {
			DrawRightArrow (CR_LEFT + EDIT_LEFT + 1 + LISTCOLOR_WIDTH * 15, y);
		} else if (lx == rx) {
			DrawDownArrow (CR_LEFT + EDIT_LEFT + 1 + lx * LISTCOLOR_WIDTH, y);
 		} else if (lx < 0 && rx > 15) {
			DrawRect (CR_LEFT + EDIT_LEFT + 1, y, 16 * LISTCOLOR_WIDTH, 1);
		} else if (lx < 0) {
			DrawRect (CR_LEFT + EDIT_LEFT + 1, y, rx * LISTCOLOR_WIDTH + 2, 1);
			DrawDownArrow (CR_LEFT + EDIT_LEFT + 1 + rx * LISTCOLOR_WIDTH, y);
		} else if (rx > 15) {
			short	w;

			w = (15 - lx) * LISTCOLOR_WIDTH + 2;
			DrawRect (CR_LEFT + EDIT_LEFT + 1 + 16 * LISTCOLOR_WIDTH - w, y, w, 1);
			DrawDownArrow (CR_LEFT + EDIT_LEFT + 1 + lx * LISTCOLOR_WIDTH, y);
		} else {
			DrawDownArrow (CR_LEFT + EDIT_LEFT + 1 + lx * LISTCOLOR_WIDTH, y);
			DrawDownArrow (CR_LEFT + EDIT_LEFT + 1 + rx * LISTCOLOR_WIDTH, y);
			DrawRect (CR_LEFT + EDIT_LEFT + 1 + lx * LISTCOLOR_WIDTH + 4, y,
					(rx - lx - 1) * LISTCOLOR_WIDTH + 4, 1);
		}

		AfterGraphics ();
	}
} /* UpdateArrow */

/*********************************************************************
 *
 * UpdateColorArrow
 *
 * SYNOPSIS
 *		LOCAL void UpdateColorArrow (void)
 *
 * PURPOSE
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
LOCAL void UpdateColorArrow (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateColorArrow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
		CS_ColorTracker	*csct    = CurrentEditCycle->EditColor;
		short			 firstID;

		if (!EndOfList (csct)) {
			firstID = csct->ID;
		} else {
			firstID = CurrentEditCycle->NumColors;
		}

		UpdateArrow (FirstColor - firstID, LastColor - firstID, CR_TOP + COLORS_TOP - 6);
	}
} /* UpdateColorArrow */

/*********************************************************************
 *
 * UpdateRegsArrow
 *
 * SYNOPSIS
 *		LOCAL void UpdateRegsArrow (void)
 *
 * PURPOSE
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
LOCAL void UpdateRegsArrow (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateRegsArrow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
		CS_RegTracker	*csrt    = CurrentEditCycle->EditReg;
		short			 firstID;

		if (!EndOfList (csrt)) {
			firstID = csrt->ID;
		} else {
			firstID = CurrentEditCycle->NumRegs;
		}

		UpdateArrow (FirstReg - firstID, LastReg - firstID, CR_TOP + REGS_TOP - 6);
	}
} /* UpdateRegsArrow */

/*********************************************************************
 *
 * UpdateCycleArrow
 *
 * SYNOPSIS
 *		LOCAL void UpdateCycleArrow (void)
 *
 * PURPOSE
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
LOCAL void UpdateCycleArrow (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateCycleArrow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		CS_CycleInfo	*csci    = EditCI->EditCycle;
		short			 firstID;

		if (!EndOfList (csci)) {
			firstID = csci->ID;
		} else {
			firstID = EditCI->NumCycles;
		}

		if (CurrentEditCycle) {
			if (ColorFocus == FOCUS_CYCLES) {
				UpdateArrow (FirstCycle - firstID, LastCycle - firstID, CR_TOP + CYCLES_TOP - 6);
			} else {
				BeforeGraphics ();
				EraseArrow (CR_TOP + CYCLES_TOP - 6);
				SetPenColor (CrColors[CR_BLACK]);
				if (CurrentCycle < firstID) {
					DrawLeftArrow (CR_LEFT + EDIT_LEFT + 1, CR_TOP + CYCLES_TOP - 6);
				} else if (CurrentCycle > firstID + 15) {
					DrawRightArrow (CR_LEFT + EDIT_LEFT + 1 + LISTCOLOR_WIDTH * 15, CR_TOP + CYCLES_TOP - 6);
				} else {
					short	x;

					x = CR_LEFT + EDIT_LEFT + 1 + LISTCOLOR_WIDTH * (CurrentCycle - firstID);
					DrawPixel (x    , CR_TOP + CYCLES_TOP - 4);
					DrawPixel (x + 5, CR_TOP + CYCLES_TOP - 4);
					DrawPixel (x + 1, CR_TOP + CYCLES_TOP - 3);
					DrawPixel (x + 4, CR_TOP + CYCLES_TOP - 3);
					DrawPixel (x + 2, CR_TOP + CYCLES_TOP - 2);
					DrawPixel (x + 3, CR_TOP + CYCLES_TOP - 2);
				}
				AfterGraphics ();
			}
		} else {
			EraseArrow (CR_TOP + CYCLES_TOP - 6);
		}
	}
} /* UpdateCycleArrow */

/*********************************************************************
 *
 * UpdateRGBvsHSV
 *
 * SYNOPSIS
 *		LOCAL void UpdateRGBvsHSV (void)
 *
 * PURPOSE
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
LOCAL void UpdateRGBvsHSV (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateRGBvsHSV";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		static char	*label[] = {
			"RGB",
			"HSV",
		};

		char	*str;

		str = label[SlideMode];

		BeforeGraphics ();

		SetPenColor (CrColors[CR_WHITE]);
		DrawRect (CR_LEFT + RGB_LEFT + 1, CR_TOP + RGB_TOP + 1, RGB_WIDTH - 3, RGB_HEIGHT - 3);
		DrawStringN (CRFont, CR_LEFT + RGB_LEFT +  2, CR_TOP + RGB_TOP + 2, str    , 1);
		DrawStringN (CRFont, CR_LEFT + RGB_LEFT + 13, CR_TOP + RGB_TOP + 2, str + 1, 1);
		DrawStringN (CRFont, CR_LEFT + RGB_LEFT + 24, CR_TOP + RGB_TOP + 2, str + 2, 1);
		AfterGraphics ();
	}

} /* UpdateRGBvsHSV */

/*********************************************************************
 *
 * UpdateRGBSlider
 *
 * SYNOPSIS
 *		LOCAL void UpdateRGBSlider (short slide)
 *
 * PURPOSE
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
LOCAL void UpdateRGBSlider (short slide)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateRGBSlider";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		short	left;
		short	top;
		short	crng;
		short	cmax;
		short	new;

		left = SliderLeft[slide];
		crng = ColorRange[slide];
		cmax = ColorMax[slide];
		if (SlideMode == HSV_MODE) {
			top  = RGBSliderOld[slide];
			new  = RGBSliderPos[slide];
		} else {
			top  = OldOneClr ? 
					((short)((long)RGBSliderOld[slide] * (long)crng / (long)MAX_COLOR_RANGE)) : 
					((short)((long)RGBSliderOld[slide] * (long)(crng * 2 - 1) / (long)(511L)));
			new  = OneColor  ? 
					((short)((long)RGBSliderPos[slide] * (long)crng / (long)MAX_COLOR_RANGE)) : 
					((short)((long)RGBSliderPos[slide] * (long)(crng * 2 - 1) / (long)(511L)));
		}
		if (top != new) {
			BeforeGraphics ();
			if (SlideMode == HSV_MODE) {
				top = OldOneClr ?
						((short)((long)top * (long)SLIDE_POSITIONS / (long)MAX_HARDWARE_COLOR)) :
						((short)((long)top * (long)SLIDE_POSITIONS / (long)511L));
				new = OneColor ?
						((short)((long)new * (long)SLIDE_POSITIONS / (long)MAX_HARDWARE_COLOR)) :
						((short)((long)new * (long)SLIDE_POSITIONS / (long)511L));
			} else {
				top = OldOneClr ?
						((short)((long)top * (long)SLIDE_POSITIONS / (long)cmax)) :
						((short)((long)top * (long)SLIDE_POSITIONS / (long)(cmax * 2 + 1)));
				new = OneColor ?
						((short)((long)new * (long)SLIDE_POSITIONS / (long)cmax)) :
						((short)((long)new * (long)SLIDE_POSITIONS / (long)(cmax * 2 + 1)));
			}
			if (top != new) {

				if (top >= 0) {
					SetPenColor (CrColors[CR_BLACK]);
					DrawRect (	left,
								CR_TOP + SLIDE_TOP + 1 + top,
								SLIDER_WIDTH,
								SLIDER_HEIGHT);
				}

				if (SlideMode == HSV_MODE) {
					top  = RGBSliderPos[slide];
					top  = OneColor ?
							((short)((long)top * (long)SLIDE_POSITIONS / (long)MAX_HARDWARE_COLOR)) :
							((short)((long)top * (long)SLIDE_POSITIONS / (long)511L)) ;
				} else {
					top  = OneColor ?
							((short)((long)RGBSliderPos[slide] * (long)crng / (long)MAX_COLOR_RANGE)) :
							((short)((long)RGBSliderPos[slide] * (long)(crng * 2 - 1) / (long)511L)) ;
					top  = OneColor ?
							((short)((long)top * (long)SLIDE_POSITIONS / (long)cmax)) :
							((short)((long)top * (long)SLIDE_POSITIONS / (long)(cmax * 2 + 1))) ;
				}

				top += CR_TOP + SLIDE_TOP + 1;

				SetPenColor (CrColors[CR_WHITE]);
				DrawRect ( left + 2, top    , 3, 1);
				DrawRect ( left + 1, top + 1, 5, 1);
				DrawRect ( left + 0, top + 2, 7, 1);
				DrawRect ( left + 1, top + 3, 5, 1);
				DrawRect ( left + 2, top + 4, 3, 1);
			}

			RGBSliderOld[slide] = RGBSliderPos[slide];

			SetPenColor (CrColors[CR_WHITE]);
			left++;
			top = CR_TOP + CDN_ARROW_TOP + CARROW_HEIGHT + 1;
			DrawRect ( left,
					   top,
					   SLIDE_WIDTH - 4,
					   RVALUE_HEIGHT - 1);

			if (OneColor) {
				char	value[6];
				short	v;

				v = 255 - RGBSliderPos[slide];
				if (SlideMode == HSV_MODE && slide == 0) {
					v = (short)((long)v * (long)359 / (long)MAX_HARDWARE_COLOR); 
				}
				if (SlideMode == RGB_MODE) {
					v = (short)((long)v * (long)crng / (long)MAX_COLOR_RANGE);
				}
				sprintf (value, "%03d", v);

				DrawStringN (CRFont, left, top     , &value[0], 1);
				DrawStringN (CRFont, left, top +  6, &value[1], 1);
				DrawStringN (CRFont, left, top + 12, &value[2], 1);
			} else {
				char	value[6];
				short	v;

				v = -(RGBSliderPos[slide] - 256);
				if (SlideMode == RGB_MODE) {
					v = (short)((long)v * (long)crng / (long)MAX_COLOR_RANGE);
				}

				v = (v > 255) ? 255 : ((v < -255) ? -255 : v);
				sprintf (value, "%c%02x", ((v < 0) ? '-' : '+'), ((v < 0) ? (-v) : v));

				DrawStringN (CRFont, left + 1, top , &value[0], 1);
				DrawStringN (CRFont, left, top +  6, &value[1], 1);
				DrawStringN (CRFont, left, top + 12, &value[2], 1);
			}

			AfterGraphics ();

		}
	}

} /* UpdateRGBSlider */

/*********************************************************************
 *
 * SetupOneColor
 *
 * SYNOPSIS
 *		LOCAL void SetupOneColor (void)
 *
 * PURPOSE
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
LOCAL void SetupOneColor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetupOneColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		if (!OneColor)
		{
			RGBSliderPos[0] = 256;
			RGBSliderPos[1] = 256;
			RGBSliderPos[2] = 256;
			AffectVal[0] = 256;
			AffectVal[1] = 256;
			AffectVal[2] = 256;
			OldAffectVal[0] = 256;
			OldAffectVal[1] = 256;
			OldAffectVal[2] = 256;
		}
		UpdateRGBSlider (0);
		UpdateRGBSlider (1);
		UpdateRGBSlider (2);
		OldOneClr = OneColor;
	}
} /* SetupOneColor */

/*********************************************************************
 *
 * UpdateRGB
 *
 * SYNOPSIS
 *		LOCAL void UpdateRGB (CS_Color *csc)
 *
 * PURPOSE
 *		Update the RGB color sliders with the specified colors.
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
LOCAL void UpdateRGB (CS_Color *csc)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateRGB";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		static CS_Color	 cs;

		if (csc) {
			KludgeCS = *csc;
			cs = *csc;
		}
		BeforeGraphics ();
		if (SlideMode == RGB_MODE) {
			RGBSliderPos[0] = 255 - cs.Red;
			RGBSliderPos[1] = 255 - cs.Green;
			RGBSliderPos[2] = 255 - cs.Blue;
		} else {
			RGBSliderPos[0] = 255 - (short)((long)cs.Hue * (long)MAX_HARDWARE_COLOR / (long)359);
			RGBSliderPos[1] = 255 - cs.Saturation;
			RGBSliderPos[2] = 255 - cs.Value;
		}
		RGBSliderOld [0] = -100;
		RGBSliderOld [1] = -100;
		RGBSliderOld [2] = -100;

		{
			short	i;

			for (i = 0; i < 3; i++) {
		 		SetPenColor (CrColors[CR_BLACK]);
		 		DrawRect (	SliderLeft[i],
		 					CR_TOP + SLIDE_TOP + 1,
		 					SLIDER_WIDTH,
		 					SLIDE_HEIGHT - 1);
				
			}
		}

		SetupOneColor ();

		AfterGraphics ();
	}
} /* UpdateRGB */

/*********************************************************************
 *
 * UpdateRGBReg
 *
 * SYNOPSIS
 *		LOCAL void UpdateRGBReg (CS_RegTracker *csrt)
 *
 * PURPOSE
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
LOCAL void UpdateRGBReg (CS_RegTracker *csrt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateRGBReg";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		CS_Color	 cs;

		cs.Red = cs.Green = cs.Blue = cs.Saturation = cs.Value = csrt->Reg;
		cs.Hue = csrt->Reg * 359 / MAX_HARDWARE_COLOR;
		UpdateRGB (&cs);
	}
} /* UpdateRGBReg */

/*********************************************************************
 *
 * UseCyclePalette
 *
 * SYNOPSIS
 *		LOCAL void UseCyclePalette (void)
 *
 * PURPOSE
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
LOCAL void UseCyclePalette (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UseCyclePalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		CS_ColorTracker	*csct;
		short			 i;

		if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
			csct = CurrentEditCycle->EditColor;
		} else {
			csct = NULL;
		}

		for (i = 0; i < 16; i++) {
			if (csct && !EndOfList (csct)) {
				GlobalPalette[FIRSTCYCLECOLOR + i].Red   = csct->Color.Red;
				GlobalPalette[FIRSTCYCLECOLOR + i].Green = csct->Color.Green;
				GlobalPalette[FIRSTCYCLECOLOR + i].Blue  = csct->Color.Blue;
				csct = Next (csct);
			} else {
				GlobalPalette[FIRSTCYCLECOLOR + i].Red   = pcscEdit[FIRSTCYCLECOLOR + i].Red;
				GlobalPalette[FIRSTCYCLECOLOR + i].Green = pcscEdit[FIRSTCYCLECOLOR + i].Green;
				GlobalPalette[FIRSTCYCLECOLOR + i].Blue  = pcscEdit[FIRSTCYCLECOLOR + i].Blue;
			}
			GlobalPalette[FIRSTCYCLECOLOR + i].Flags |= PAL_CHANGED;
		}
		UpdateHardwareColors (GlobalPalette, FALSE);
	}

} /* UseCyclePalette */

/*********************************************************************
 *
 * UpdateCycleRegs
 *
 * SYNOPSIS
 *		LOCAL void UpdateCycleRegs (void)
 *
 * PURPOSE
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
LOCAL void UpdateCycleRegs (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateCycleRegs";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short			 i;
		short			 x;
		short			 endflag;
		short			 reg;
		CS_RegTracker	*csrt;

		BeforeGraphics ();

		if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
			csrt = CurrentEditCycle->EditReg;
		} else {
			csrt = NULL;
		}

		endflag = FALSE;
		x       = CR_LEFT + EDIT_LEFT + 1;
		for (i = 0; i < 16; i++) {
			if (csrt && !EndOfList (csrt)) {
				reg  = csrt->Reg;
				csrt = Next (csrt);
			} else {
				reg = -1;
			}
			if (reg >= 0) {
				SetPenColor (reg);
				DrawRect (x, CR_TOP + REGS_TOP + 1, LISTCOLOR_WIDTH, LISTCOLOR_HEIGHT);
				SetPenColor (CrColors[CR_WHITE]);
				DrawRect (x, CR_TOP + REGS_TOP + 1 + 6 + 2, LISTCOLOR_WIDTH, 3 * 6);
				{
					char	value[6];
				
					sprintf (value, "%3d", reg);

					DrawStringN (CRFont, x + 1, CR_TOP + REGS_TOP + 1 + 6 + 2     , &value[0], 1);
					DrawStringN (CRFont, x + 1, CR_TOP + REGS_TOP + 1 + 6 + 2 +  6, &value[1], 1);
					DrawStringN (CRFont, x + 1, CR_TOP + REGS_TOP + 1 + 6 + 2 + 12, &value[2], 1);
				}
				
			} else {
				if (!endflag) {
					endflag = TRUE;
					DrawListEnd (x, CR_TOP + REGS_TOP + 1);
				} else {
					SetPenColor (CrColors[CR_BLACK]);
					DrawRect (x, CR_TOP + REGS_TOP + 1, LISTCOLOR_WIDTH, LISTCOLOR_HEIGHT);
				}
				SetPenColor (CrColors[CR_WHITE]);
				DrawRect (x, CR_TOP + REGS_TOP + 9, LISTCOLOR_WIDTH, 6 * 3);
			}
			x += LISTCOLOR_WIDTH;
		}

		AfterGraphics ();
	}

} /* UpdateCycleRegs */

/*********************************************************************
 *
 * UpdateCycleColors
 *
 * SYNOPSIS
 *		LOCAL void UpdateCycleColors (void)
 *
 * PURPOSE
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
LOCAL void UpdateCycleColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateCycleColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short			 i;
		short			 x;
		short			 endflag;
		CS_ColorTracker	*csct;

		BeforeGraphics ();

		if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
			csct = CurrentEditCycle->EditColor;
		} else {
			csct = NULL;
		}

		endflag = FALSE;
		x       = CR_LEFT + EDIT_LEFT + 1;
		for (i = 0; i < 16; i++) {
			if (csct && !EndOfList (csct)) {
				SetPenColor (i + FIRSTCYCLECOLOR);
				DrawRect (x, CR_TOP + COLORS_TOP + 1, LISTCOLOR_WIDTH, LISTCOLOR_HEIGHT);
				csct = Next (csct);
			} else {
				if (!endflag) {
					endflag = TRUE;
					DrawListEnd (x, CR_TOP + COLORS_TOP + 1);
				} else {
					SetPenColor (CrColors[CR_BLACK]);
					DrawRect (x, CR_TOP + COLORS_TOP + 1, LISTCOLOR_WIDTH, LISTCOLOR_HEIGHT);
				}
			}
			x += LISTCOLOR_WIDTH;
		}
		UseCyclePalette ();

		AfterGraphics ();
	}
} /* UpdateCycleColors */

/*********************************************************************
 *
 * UpdateCycles
 *
 * SYNOPSIS
 *		LOCAL void UpdateCycles (void)
 *
 * PURPOSE
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
LOCAL void UpdateCycles (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateCycles";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		short			 i;
		short			 x;
		short			 endflag;
		CS_CycleInfo	*csci;

		BeforeGraphics ();

		csci    = EditCI->EditCycle;
		endflag = FALSE;
		x       = CR_LEFT + EDIT_LEFT + 1;
		for (i = 0; i < 16; i++) {
			if (!EndOfList (csci)) {
				SetPenColor ((csci->ID & 0x01) ? CrColors[CR_BLUE] : CrColors[CR_ORANGE]);
				DrawRect (x, CR_TOP + CYCLES_TOP + 1, LISTCOLOR_WIDTH, CYCLES_HEIGHT - 3);
				{
					char	value[6];

					sprintf (value, "%02d", csci->ID);
					DrawStringN (CRFont, x + 1, CR_TOP + CYCLES_TOP + 2    , &value[0], 1);
					DrawStringN (CRFont, x + 1, CR_TOP + CYCLES_TOP + 2 + 6, &value[1], 1);
				}
				csci = Next (csci);
			} else {
				if (!endflag) {
					endflag = TRUE;
					SetPenColor (CrColors[CR_WHITE]);
					DrawRect (x, CR_TOP + CYCLES_TOP + 1, LISTCOLOR_WIDTH, CYCLES_HEIGHT - 3);
					SetPenColor (CrColors[CR_BLACK]);
					DrawPixel (x + 2, CR_TOP + CYCLES_TOP + 1 +  1);
					DrawPixel (x + 4, CR_TOP + CYCLES_TOP + 1 +  1);
					DrawPixel (x + 1, CR_TOP + CYCLES_TOP + 1 +  2);
					DrawPixel (x + 3, CR_TOP + CYCLES_TOP + 1 +  2);
					DrawPixel (x + 2, CR_TOP + CYCLES_TOP + 1 +  3);
					DrawPixel (x + 4, CR_TOP + CYCLES_TOP + 1 +  3);
					DrawPixel (x + 1, CR_TOP + CYCLES_TOP + 1 +  4);
					DrawPixel (x + 3, CR_TOP + CYCLES_TOP + 1 +  4);
					DrawPixel (x + 2, CR_TOP + CYCLES_TOP + 1 +  5);
					DrawPixel (x + 4, CR_TOP + CYCLES_TOP + 1 +  5);
					DrawPixel (x + 1, CR_TOP + CYCLES_TOP + 1 +  6);
					DrawPixel (x + 3, CR_TOP + CYCLES_TOP + 1 +  6);
					DrawPixel (x + 2, CR_TOP + CYCLES_TOP + 1 +  7);
					DrawPixel (x + 4, CR_TOP + CYCLES_TOP + 1 +  7);
					DrawPixel (x + 1, CR_TOP + CYCLES_TOP + 1 +  8);
					DrawPixel (x + 3, CR_TOP + CYCLES_TOP + 1 +  8);
					DrawPixel (x + 2, CR_TOP + CYCLES_TOP + 1 +  9);
					DrawPixel (x + 4, CR_TOP + CYCLES_TOP + 1 +  9);
					DrawPixel (x + 1, CR_TOP + CYCLES_TOP + 1 + 10);
					DrawPixel (x + 3, CR_TOP + CYCLES_TOP + 1 + 10);
					DrawPixel (x + 2, CR_TOP + CYCLES_TOP + 1 + 11);
					DrawPixel (x + 4, CR_TOP + CYCLES_TOP + 1 + 11);
				} else {
					SetPenColor (CrColors[CR_BLACK]);
					DrawRect (x, CR_TOP + CYCLES_TOP + 1, LISTCOLOR_WIDTH, CYCLES_HEIGHT - 2);
				}
			}
			x += LISTCOLOR_WIDTH;
		}

		AfterGraphics ();
	}

} /* UpdateCycles */

/*********************************************************************
 *
 * UpdateCycleFlag
 *
 * SYNOPSIS
 *		LOCAL void UpdateCycleFlag (void)
 *
 * PURPOSE
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
LOCAL void UpdateCycleFlag (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateCycleFlag";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		short			 on;

		if (CurrentEditCycle && !EndOfList (CurrentEditCycle) && (CurrentEditCycle->Flags & CI_ENABLED)) {
			on = TRUE;
		} else {
			on = FALSE;
		}
			
		BeforeGraphics ();

		SetPenColor (CrColors[CR_WHITE]);
		DrawRect (CR_LEFT + CFLAG_LEFT + 36, CR_TOP + CFLAG_TOP + 2, 3 * 6, 5);
		DrawString (CRFont, CR_LEFT + CFLAG_LEFT + 36, CR_TOP + CFLAG_TOP + 2, (on) ? "ON" : "OFF");	

		AfterGraphics ();
	}
} /* UpdateCycleFlag */

/*********************************************************************
 *
 * UpdateDirection
 *
 * SYNOPSIS
 *		LOCAL void UpdateDirection (void)
 *
 * PURPOSE
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
LOCAL void UpdateDirection (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateDirection";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short			 i;
		short			 x;

		BeforeGraphics ();

		SetPenColor (CrColors[CR_WHITE]);
		DrawRect (CR_LEFT + DIR_LEFT + 1, CR_TOP + DIR_TOP + 1, DIR_WIDTH - 3, DIR_HEIGHT - 3);
		SetPenColor (CrColors[CR_BLACK]);

		if (CurrentEditCycle && !EndOfList (CurrentEditCycle) && CurrentEditCycle->Direction) {
			static short ptbl[] = {
				CR_LEFT + DIR_LEFT + 2,
				CR_LEFT + DIR_LEFT + 3,
				CR_LEFT + DIR_LEFT + 9,
				CR_LEFT + DIR_LEFT + 11,
				CR_LEFT + DIR_LEFT + 13,
				CR_LEFT + DIR_LEFT + 15,
			};

			DrawRect (CR_LEFT + DIR_LEFT + 2, CR_TOP + DIR_TOP + 4, DIR_WIDTH - 7, 1);

			for (i = 0; i < 6; i++) {
				x = ptbl[i];
				DrawPixel (x    , CR_TOP + DIR_TOP + 4);
				DrawPixel (x + 1, CR_TOP + DIR_TOP + 3);
				DrawPixel (x + 1, CR_TOP + DIR_TOP + 5);
				DrawPixel (x + 2, CR_TOP + DIR_TOP + 2);
				DrawPixel (x + 2, CR_TOP + DIR_TOP + 6);
			}
		} else {
			static short ptbl[] = {
				CR_LEFT + DIR_LEFT + 17,
				CR_LEFT + DIR_LEFT + 16,
				CR_LEFT + DIR_LEFT + 10,
				CR_LEFT + DIR_LEFT + 8,
				CR_LEFT + DIR_LEFT + 6,
				CR_LEFT + DIR_LEFT + 4,
			};

			DrawRect (CR_LEFT + DIR_LEFT +  4, CR_TOP + DIR_TOP + 4, DIR_WIDTH - 7, 1);

			for (i = 0; i < 6; i++) {
				x = ptbl[i];
				DrawPixel (x    , CR_TOP + DIR_TOP + 4);
				DrawPixel (x - 1, CR_TOP + DIR_TOP + 3);
				DrawPixel (x - 1, CR_TOP + DIR_TOP + 5);
				DrawPixel (x - 2, CR_TOP + DIR_TOP + 2);
				DrawPixel (x - 2, CR_TOP + DIR_TOP + 6);
			}
		}

		AfterGraphics ();
	}
} /* UpdateDirection */

/*********************************************************************
 *
 * UpdateColorCount
 *
 * SYNOPSIS
 *		LOCAL void UpdateColorCount (void)
 *
 * PURPOSE
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
LOCAL void UpdateColorCount (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateColorCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short			 count;

		if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
			count = CurrentEditCycle->NumColors;
		} else {
			count = 0;
		}
		DrawNumber (CR_LEFT + NUM_LEFT + 2, CR_TOP + COLORS_TOP + 2, count);
	}
} /* UpdateColorCount */

/*********************************************************************
 *
 * UpdateRegCount
 *
 * SYNOPSIS
 *		LOCAL void UpdateRegCount (void)
 *
 * PURPOSE
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
LOCAL void UpdateRegCount (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateRegCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short			 count;

		if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
			count = CurrentEditCycle->NumRegs;
		} else {
			count = 0;
		}
		DrawNumber (CR_LEFT + NUM_LEFT + 2, CR_TOP + REGS_TOP + 2, count);
	}
} /* UpdateRegCount */

/*********************************************************************
 *
 * intsqrt
 *
 * SYNOPSIS
 *		LOCAL short intsqrt (short val)
 *
 * PURPOSE
 *		Take the square root of a number between 0 and 16384.
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
LOCAL short intsqrt (short val)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "intsqrt";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short	i;
		for	(i = 0; i < 150; i++) {
			if (i * i >= val) {
				return i;
			}
		}
		return 0;
	}
} /* intsqrt */

/*********************************************************************
 *
 * UpdateSpeed
 *
 * SYNOPSIS
 *		LOCAL void UpdateSpeed (void)
 *
 * PURPOSE
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
LOCAL void UpdateSpeed (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateSpeed";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
		short	left;

		left = OldSpeed;
		if (left != CurrentEditCycle->Speed) {
			left = (short)((long)intsqrt(left) * (long)SPEED_POSITIONS / (long)LOCAL_ONEPERTICK);
			BeforeGraphics ();

			SetPenColor (CrColors[CR_BLACK]);
			DrawRect (	CR_LEFT + SPEED_LEFT + 1 + left,
						CR_TOP + SPEED_TOP + 1,
						SPSLIDER_WIDTH,
						SPSLIDER_HEIGHT);

			left  = OldSpeed = CurrentEditCycle->Speed;
			left  = (short)((long)intsqrt(left) * (long)SPEED_POSITIONS / (long)LOCAL_ONEPERTICK);
			left += CR_LEFT + SPEED_LEFT + 1;

			SetPenColor (CrColors[CR_WHITE]);
			DrawRect ( left + 1, CR_TOP + SPEED_TOP + 2, 3, 3);
			DrawRect ( left + 2, CR_TOP + SPEED_TOP + 1, 1, 5);
			DrawRect ( left    , CR_TOP + SPEED_TOP + 3, 5, 1);

			AfterGraphics ();

		#if 0
		{
			char line[80];

			sprintf (line, "speed = %5d left = %5d", CurrentEditCycle->Speed, left);
			SetPenColor (255);
			DrawRect (0,0, 320, 10);
			DrawString (&SmallFont, 1, 1, line);
		}
		#endif
		}
	}
} /* UpdateSpeed */

/*********************************************************************
 *
 * GetAColor
 *
 * SYNOPSIS
 *		LOCAL CS_Color *GetAColor (short focus, short color, short dst)
 *
 * PURPOSE
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
LOCAL CS_Color *GetAColor (short focus, short color, short dst)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetAColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/


	{
		CS_Color	*cs = NULL;

		switch (focus) {
		case FOCUS_PALETTE:
			cs = &pcscEdit[color];
			if (dst) {
				DstColor = color;
			} else {
				SrcColor = color;
			}
			break;
		case FOCUS_CCBUFFER:
			UpdateXTRA (CCBuffer);
			cs  = ActivateXTRAatWin (CCBuffer, 1);
			cs += DstFirst;
			if (dst) {
				DstColor = color;
			} else {
				SrcColor = color;
			}
			break;
		case FOCUS_COLORS:
			if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
				CS_ColorTracker	*csct;

				csct = FindColorNode (CurrentEditCycle->ColorList, color);
				if (!EndOfList (csct)) {
					short	firstID;
					short	rcolor;

					cs = &csct->Color;
					firstID = CurrentEditCycle->EditColor->ID;
					rcolor  = color - firstID;
					if (rcolor < 0 || rcolor > 15) {
						rcolor = (-1);
					} else {
						rcolor = rcolor + FIRSTCYCLECOLOR;
					}
					if (dst) {
						DstColor = rcolor;
					} else {
						SrcColor = rcolor;
					}
					
				}
			}
			break;
		case FOCUS_REGS:
			if (dst) {
				DstColor = (-1);
			} else {
				SrcColor = (-1);
			}
			if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
				CS_RegTracker	*csrt;
				static CS_Color	 csreg;

				csrt = FindColorNode (CurrentEditCycle->RegList, color);
				if (!EndOfList (csrt)) {
					csreg.Red = csreg.Green = csreg.Blue = csreg.Saturation = csreg.Value = csrt->Reg;
					csreg.Hue = (short)((long)csrt->Reg * (long)359 / (long)MAX_HARDWARE_COLOR);
					cs        = &csreg;
					SrcCSrt   = csrt;
				}
			}
			break;
		}

		return cs;
	}
} /* GetAColor */

/*********************************************************************
 *
 * GetNextSrcColor
 *
 * SYNOPSIS
 *		CS_Color *GetNextSrcColor (void)
 *
 * PURPOSE
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
LOCAL CS_Color *GetNextSrcColor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetNextSrcColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		CS_Color	*cs = NULL;

		switch (ColorFocus) {
		case FOCUS_PALETTE:
			if (SrcDir == 1) {
				if (SrcFirst <= SrcLast) {
					cs = GetAColor (ColorFocus, SrcFirst, FALSE);
					SrcFirst++;
				}
			} else {
				if (SrcLast >= SrcFirst) {
					cs = GetAColor (ColorFocus, SrcLast, FALSE);
					SrcLast--;
				}
			}
			break;
		case FOCUS_CCBUFFER:
			if (SrcFirst <= SrcLast)
			{
				UpdateXTRA (CCBuffer);
				cs = ActivateXTRAatWin (CCBuffer, 1);
				cs += SrcFirst;
				SrcFirst++;
			}
			break;
		case FOCUS_COLORS:
		case FOCUS_REGS:
			if (SrcDir == (-1)) {
				if (SrcFirst >= SrcLast) {
					cs = GetAColor (ColorFocus, SrcFirst, FALSE);
					SrcFirst--;
				}
			} else {
				if (SrcFirst <= SrcLast) {
					cs = GetAColor (ColorFocus, SrcFirst, FALSE);
					SrcFirst++;
				}
			}
			break;
		}

		return cs;
	}

} /* GetNextSrcColor */

/*********************************************************************
 *
 * GetNextDstColor
 *
 * SYNOPSIS
 *		CS_Color *GetNextDstColor (void)
 *
 * PURPOSE
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
LOCAL CS_Color *GetNextDstColor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetNextDstColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		CS_Color	*cs = NULL;

		switch (DstFocus) {
		case FOCUS_PALETTE:
			if (DstDir == 1) {
				if (DstFirst <= DstLast) {
					cs = GetAColor (DstFocus, DstFirst, TRUE);
					DstFirst++;
				}
			} else if (DstDir == (-1)) {
				if (DstLast >= DstFirst) {
					cs = GetAColor (DstFocus, DstLast, TRUE);
					DstLast--;
				}
			} else {
				if (DstFirst < 256) {
					cs = GetAColor (DstFocus, DstFirst, TRUE);
					DstFirst++;
				}
			}
			break;
		case FOCUS_CCBUFFER:
			if (DstFirst < 256)
			{
				UpdateXTRA (CCBuffer);
				cs  = ActivateXTRAatWin (CCBuffer, 1);
				cs += DstFirst;
				DstFirst++;
			}
			break;
		case FOCUS_COLORS:
		case FOCUS_REGS:
			cs = GetAColor (DstFocus, DstFirst, TRUE);
			if (DstDir == (-1)) {
				DstFirst--;
			} else {
				DstFirst++;
			}
			break;
		}

		return cs;
	}

} /* GetNextDstColor */

/*********************************************************************
 *
 * GetFirstDstColor
 *
 * SYNOPSIS
 *		CS_Color *GetFirstDstColor (void)
 *
 * PURPOSE
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
LOCAL CS_Color *GetFirstDstColor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetFirstDstColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		CS_Color	*cs = NULL;

		switch (DstFocus) {
		case FOCUS_PALETTE:
		case FOCUS_COLORS:
		case FOCUS_REGS:
		case FOCUS_CCBUFFER:
			cs = GetAColor (DstFocus, DstFirst, TRUE);
			break;
		}

		return cs;
	}

} /* GetFirstDstColor */

/*********************************************************************
 *
 * GetFirstSrcColor
 *
 * SYNOPSIS
 *		CS_Color *GetFirstSrcColor (void)
 *
 * PURPOSE
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
LOCAL CS_Color *GetFirstSrcColor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetFirstSrcColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		CS_Color	*cs = NULL;

		switch (ColorFocus) {
		case FOCUS_PALETTE:
			cs = GetAColor (ColorFocus, PalStart, FALSE);
			SrcFirst = PalStart;
			SrcLast  = PalEnd;
			#if 0
			if (SrcDir == 1) {
				SrcYW = PalY1;
			} else {
				SrcYW = PalY2;
			}
			#endif
			break;
		case FOCUS_CCBUFFER:
			cs = GetAColor (ColorFocus, 0, FALSE);
			SrcFirst = 0;
			SrcLast  = CCCount - 1;
			break;
		case FOCUS_COLORS:
			cs = GetAColor (ColorFocus, FirstColor, FALSE);
			SrcFirst = FirstColor;
			SrcLast  = LastColor;
			break;
		case FOCUS_REGS:
			cs = GetAColor (ColorFocus, FirstReg, FALSE);
			SrcFirst = FirstReg;
			SrcLast  = LastReg;
			break;
		}

		return cs;
	}

} /* GetFirstSrcColor */

/*********************************************************************
 *
 * GetLastSrcColor
 *
 * SYNOPSIS
 *		CS_Color *GetLastSrcColor (void)
 *
 * PURPOSE
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
LOCAL CS_Color *GetLastSrcColor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetLastSrcColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		CS_Color	*cs = NULL;

		switch (ColorFocus) {
		case FOCUS_PALETTE:
			cs = GetAColor (ColorFocus, PalEnd, FALSE);
			break;
		case FOCUS_CCBUFFER:
			cs = GetAColor (ColorFocus, CCCount - 1, FALSE);
			break;
		case FOCUS_COLORS:
			if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
				if (LastColor == CurrentEditCycle->NumColors) {
					LastColor--;
				}
				if (LastColor > 0) {
					cs = GetAColor (ColorFocus, LastColor, FALSE);
				}
			}
			break;
		case FOCUS_REGS:
			if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
				if (LastReg == CurrentEditCycle->NumRegs) {
					LastReg--;
				}
				if (LastReg > 0) {
					cs = GetAColor (ColorFocus, LastReg, FALSE);
				}
			}
			break;
		}

		return cs;
	}
} /* GetLastSrcColor */

/*********************************************************************
 *
 * AffectPaletteReg
 *
 * SYNOPSIS
 *		LOCAL void AffectPaletteReg (CS_Color *cs, short palettereg)
 *
 * PURPOSE
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
LOCAL void AffectPaletteReg (CS_Color *cs, short palettereg)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AffectPaletteReg";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (palettereg >= 0) {
		UWORD	temp;
		short	cval;

		cval = BOUNDCOLOR(cs->Red);
		temp = (UWORD)cval * (UWORD)ColorRange[0]/ (UWORD)MAX_COLOR_RANGE;
		GlobalPalette[palettereg].Red    = (UWORD)temp * (UWORD)MAX_HARDWARE_COLOR / (UWORD)ColorMax[0];
		cval = BOUNDCOLOR(cs->Green);
		temp = (UWORD)cval * (UWORD)ColorRange[1]/ (UWORD)MAX_COLOR_RANGE;
		GlobalPalette[palettereg].Green  = (UWORD)temp * (UWORD)MAX_HARDWARE_COLOR / (UWORD)ColorMax[1];
		cval = BOUNDCOLOR(cs->Blue);
		temp = (UWORD)cval * (UWORD)ColorRange[2]/ (UWORD)MAX_COLOR_RANGE;
		GlobalPalette[palettereg].Blue   = (UWORD)temp * (UWORD)MAX_HARDWARE_COLOR / (UWORD)ColorMax[2];
		GlobalPalette[palettereg].Flags |= PAL_CHANGED;
	}
} /* AffectPaletteReg */

/*********************************************************************
 *
 * AffectColor
 *
 * SYNOPSIS
 *		LOCAL void AffectColor (CS_Color *cs)
 *
 * PURPOSE
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
LOCAL void AffectColor (CS_Color *cs, short palettereg)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AffectColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		KludgeCS = *cs;

		#if 0
		{
			char	line[80];

			BeforeGraphics ();

			sprintf (line, "%3d %3d %3d:%3d %3d %3d:%4d %4d %4d",
				AffectVal[0], AffectVal[1], AffectVal[2],
				OldAffectVal[0], OldAffectVal[1], OldAffectVal[2],
				cs->Red, cs->Green, cs->Blue);

			SetPenColor (CrColors[CR_WHITE]);
			DrawRect (0, 0, 320, 7);
			DrawString (CRFont, 0, 0, line);
			AfterGraphics ();
		}
		#endif

		if (SlideMode == RGB_MODE) {
			if (OneColor || Spreading) {
				if (AffectVal[0] >= 0) {
					cs->Red = AffectVal[0];
				}
				if (AffectVal[1] >= 0) {
					cs->Green = AffectVal[1];
				}
				if (AffectVal[2] >= 0) {
					cs->Blue = AffectVal[2];
				}
			} else {
				cs->Red   += (AffectVal[0] - 256) - (OldAffectVal[0] - 256);
				cs->Green += (AffectVal[1] - 256) - (OldAffectVal[1] - 256);
				cs->Blue  += (AffectVal[2] - 256) - (OldAffectVal[2] - 256);
			}
			ConvertRGBtoHSV (cs);
		} else {
			if (OneColor || Spreading) {
				if (AffectVal[0] >= 0) {
					if (Spreading) {
						cs->Hue = AffectVal[0];
					} else {
						cs->Hue = (short)((long)AffectVal[0] * (long)359 / (long)MAX_HARDWARE_COLOR);
					}
				}
				if (AffectVal[1] >= 0) {
					cs->Saturation = AffectVal[1];
				}
				if (AffectVal[2] >= 0) {
					cs->Value = AffectVal[2];
				}
			} else {
				cs->Hue        += (AffectVal[0] - 256) - (OldAffectVal[0] - 256);
				cs->Saturation += (AffectVal[1] - 256) - (OldAffectVal[1] - 256);
				cs->Value      += (AffectVal[2] - 256) - (OldAffectVal[2] - 256);
			}
			ConvertHSVtoRGB (cs);
		}

		AffectPaletteReg (cs, palettereg);
			
	}
} /* AffectColor */

/*********************************************************************
 *
 * AffectDest
 *
 * SYNOPSIS
 *		LOCAL void AffectDest (void)
 *
 * PURPOSE
 *		Affect the currently selected colors or regs with the
 *		specified values.
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
LOCAL void AffectDest (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AffectDest";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	switch (ColorFocus) {
	case FOCUS_PALETTE:
		{
			short		 x;
			CS_Color	*cs;

			cs = &pcscEdit[PalStart];
			for (x = PalStart; x <= PalEnd; x++) {
				AffectColor (cs, x);
				cs++;
			}
			UpdateHardwareColors (GlobalPalette, FALSE);
		}
		break;
	case FOCUS_COLORS:
		{
			CS_ColorTracker	*csct;
			short			 i;

			if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
				csct = FindColorNode (CurrentEditCycle->ColorList, FirstColor);
				for (i = FirstColor; i <= LastColor; i++) {
					if (!EndOfList (csct)) {
						short	firstID;
						short   color;

						firstID = CurrentEditCycle->EditColor->ID;						
						color   = i - firstID;
						if (color < 0 || color > 15) {
							color = (-1);
						} else {
							color += FIRSTCYCLECOLOR;
						}

						AffectColor (&csct->Color, color);
						csct = Next (csct);
					}
				}
			}
			UpdateHardwareColors (GlobalPalette, FALSE);
		}
		break;
	case FOCUS_REGS:
		{
			CS_RegTracker	*csrt;
			short			 i;
			short			 val;
			short			 new = (-1);

			if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
				csrt = FindColorNode (CurrentEditCycle->RegList, FirstReg);
				for (i = FirstReg; i <= LastReg; i++) {
					if (!EndOfList (csrt)) {
						if (AffectVal[0] >= 0) {
							val = AffectVal[0];
						} else if (AffectVal[1] >= 0) {
							val = AffectVal[1];
						} else {
							val = AffectVal[2];
						}
						if (csrt->Reg != val) {
							csrt->Reg = val;
							new = val;
						}
						csrt = Next (csrt);
					}
				}
				if (new >= 0) {
					BeforeGraphics ();
					UpdateCycleRegs ();
					RedrawTitle (new);
					AfterGraphics ();
				}
			}
		}
		break;
	}
	if (OneColor)
	{
		ResetAffect ();
	} else {
		OldAffectVal[0] = AffectVal[0];
		OldAffectVal[1] = AffectVal[1];
		OldAffectVal[2] = AffectVal[2];
	}

} /* AffectDest */

/*********************************************************************
 *
 * AffectSrcColor
 *
 * SYNOPSIS
 *		LOCAL void AffectSrcColor (CS_Color *cs, short color)
 *
 * PURPOSE
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
LOCAL void AffectSrcColor (CS_Color *cs, short color)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AffectSrcColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	switch (ColorFocus) {
	case FOCUS_PALETTE:
	case FOCUS_COLORS:
		AffectColor (cs, color);
		break;
	case FOCUS_REGS:
		AffectColor (cs, color);
		{
			short	val;

			if (AffectVal[0] >= 0) {
				val = AffectVal[0];
			} else if (AffectVal[1] >= 0) {
				val = AffectVal[1];
			} else {
				val = AffectVal[2];
			}

			SrcCSrt->Reg = val;
		}
		break;
	}
	ResetAffect ();
} /* AffectSrcColor */

#if 0
/*********************************************************************
 *
 * AffectDstColor
 *
 * SYNOPSIS
 *		LOCAL void AffectDstColor (CS_Color *cs, short color)
 *
 * PURPOSE
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
LOCAL void AffectDstColor (CS_Color *cs, short color)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AffectDstColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	switch (ColorFocus) {
	case FOCUS_PALETTE:
	case FOCUS_COLORS:
		AffectColor (cs, color);
		break;
	case FOCUS_REGS:
		AffectColor (cs, color);
		{
			short	val;

			if (AffectVal[0] >= 0) {
				val = AffectVal[0];
			} else if (AffectVal[1] >= 0) {
				val = AffectVal[1];
			} else {
				val = AffectVal[2];
			}

			SrcCSrt->Reg = val;
		}
		break;
	}
	ResetAffect ();
} /* AffectDstColor */
#endif

/*********************************************************************
 *
 * AffectColorDest
 *
 * SYNOPSIS
 *		LOCAL void AffectColorDest (short x, short y, short focus)
 *
 * PURPOSE
 *		If a Copy, Blend or Swap operation is choosen then complete it.
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
LOCAL void AffectColorDest (short x, short y, short focus)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AffectColorDest";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (AffectMode != AFFECT_NONE) {
		DstFocus = focus;
		DstDir   = 0;
		SrcDir   = 1;

		switch (DstFocus) {
		case FOCUS_PALETTE:
			DstFirst = x * 8 + y;
			GetFirstSrcColor ();
			if (ColorFocus == FOCUS_PALETTE) {
				DstLast = DstFirst + (SrcLast - SrcFirst);
				DstDir  = 1;
				SrcDir  = 1;
				if (DstLast > 255) {
					SrcFirst -= DstLast - 255;
					DstLast   = 255;
				}
				if (SrcFirst <= DstFirst) {
					DstDir = (-1);
					SrcDir = (-1);
				}
			}
			break;
		case FOCUS_COLORS:
			if (!CurrentEditCycle || EndOfList (CurrentEditCycle)) {
				return;
			}
			{
				CS_ColorTracker	*csct;
				short			 firstID;
				short			 numcolors;

				csct = CurrentEditCycle->EditColor;
				if (EndOfList (csct)) {
					return;
				}

				firstID   = csct->ID;
				numcolors = CurrentEditCycle->NumColors;
				if (firstID == numcolors) {
					return;
				}

				DstFirst = firstID + x;

				if (!GetFirstSrcColor ()) {
					return;
				}

				if (ColorFocus == FOCUS_COLORS) {
					short	dstlast;
					short	temp;

					dstlast = DstFirst + SrcLast - SrcFirst;
					if (dstlast >= numcolors) {
						SrcLast -= dstlast - (numcolors - 1);
						dstlast  = numcolors - 1;
					}
					if (SrcFirst < DstFirst) {
						temp     = SrcFirst;
						SrcFirst = SrcLast;
						SrcLast  = temp;
						DstFirst = dstlast;
						SrcDir   = (-1);
						DstDir   = (-1);
					}
				}
			}
			break;
		case FOCUS_REGS:
			if (!CurrentEditCycle || EndOfList (CurrentEditCycle)) {
				return;
			}
			{
				CS_RegTracker	*csrt;
				short			 firstID;
				short			 numregs;

				csrt = CurrentEditCycle->EditReg;
				if (EndOfList (csrt)) {
					return;
				}

				firstID = csrt->ID;
				numregs = CurrentEditCycle->NumRegs;
				if (firstID == numregs) {
					return;
				}

				DstFirst = firstID + x;

				if (ColorFocus == FOCUS_COLORS) {
					return;
				}

				if (!GetFirstSrcColor ()) {
					return;
				}

				if (ColorFocus == FOCUS_REGS) {
					short	dstlast;
					short	temp;

					dstlast = DstFirst + SrcLast - SrcFirst;
					if (dstlast >= numregs) {
						SrcLast -= dstlast - (numregs - 1);
						dstlast  = numregs - 1;
					}
					if (SrcFirst < DstFirst) {
						temp     = SrcFirst;
						SrcFirst = SrcLast;
						SrcLast  = temp;
						DstFirst = dstlast;
						SrcDir   = (-1);
						DstDir   = (-1);
					}
				}
			}
			break;
		case FOCUS_CCBUFFER:
			DstFirst = 0;
			GetFirstSrcColor ();
			break;
		case FOCUS_CYCLES:
		default:
			return;
		}

		if (ColorFocus == FOCUS_CYCLES) {
			return;
		}

		if (GetFirstDstColor ()) {
			if ((ColorFocus == FOCUS_COLORS || ColorFocus == FOCUS_PALETTE || ColorFocus == FOCUS_CCBUFFER) &&
				(DstFocus   == FOCUS_COLORS || DstFocus   == FOCUS_PALETTE || DstFocus   == FOCUS_CCBUFFER)) {
				CS_Color	*srccs;
				CS_Color	*dstcs;

				for (;;) {
					srccs = GetNextSrcColor ();
					dstcs = GetNextDstColor ();
					if (!srccs || !dstcs) {
						break;
					}
					switch (AffectMode) {
					case AFFECT_COPY:
						*dstcs = *srccs;
						CCCount = DstFirst;
						AffectPaletteReg (dstcs, DstColor);
						break;
					case AFFECT_BLEND:
						dstcs->Red   = (dstcs->Red   + srccs->Red  ) / 2;
						dstcs->Green = (dstcs->Green + srccs->Green) / 2;
						dstcs->Blue  = (dstcs->Blue  + srccs->Blue ) / 2;
						ConvertRGBtoHSV (dstcs);
						AffectPaletteReg (dstcs, DstColor);
						break;
					case AFFECT_SWAP:
						{
							CS_Color	 temp;

							temp   = *srccs;
							*srccs = *dstcs;
							*dstcs = temp;
							AffectPaletteReg (dstcs, DstColor);
							AffectPaletteReg (srccs, SrcColor);
						}
						break;
					}
					/* Effect GlobalPalette */
				}
				UpdateHardwareColors (GlobalPalette, FALSE);
			} else {
				if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
					if (ColorFocus == FOCUS_REGS) {
						if (DstFocus == FOCUS_REGS) {
							CS_RegTracker	*src_csrt;
							CS_RegTracker	*dst_csrt;
							short			 len;
							short			 temp;

							len = SrcLast - SrcFirst;
							if (len < 0) {
								len = (-len);
							}

							for (; len >= 0; len--) {
								src_csrt = FindColorNode (CurrentEditCycle->RegList, SrcFirst);
								dst_csrt = FindColorNode (CurrentEditCycle->RegList, DstFirst);
								switch (AffectMode) {
								case AFFECT_COPY:
									dst_csrt->Reg = src_csrt->Reg;
									break;
								case AFFECT_SWAP:
									temp          = src_csrt->Reg;
									src_csrt->Reg = dst_csrt->Reg;
									dst_csrt->Reg = temp;
									break;
								}
								if (DstDir == (-1)) {
									DstFirst--;
								} else {
									DstFirst++;
								}
								if (SrcDir == (-1)) {
									SrcFirst--;
								} else {
									SrcFirst++;
								}
							}
							UpdateCycleRegs ();
						} else {
							CS_RegTracker	*src_csrt;
							CS_Color		*dstcs;
							short			 reg;

							for (;;) {
								src_csrt = FindColorNode (CurrentEditCycle->RegList, SrcFirst);
								if (!GetNextSrcColor ()) {
									break;
								}
								dstcs    = GetNextDstColor ();
								if (!dstcs) {
									break;
								}
								reg      = src_csrt->Reg;

								switch (AffectMode) {
								case AFFECT_COPY:
									dstcs->Red        = pcscEdit[reg].Red;
									dstcs->Green      = pcscEdit[reg].Green;
									dstcs->Blue       = pcscEdit[reg].Blue;
									dstcs->Hue        = pcscEdit[reg].Hue;
									dstcs->Saturation = pcscEdit[reg].Saturation;
									dstcs->Value      = pcscEdit[reg].Value;
									AffectPaletteReg (dstcs, DstColor);
									break;
								case AFFECT_SWAP:
									dstcs->Red        = pcscEdit[reg].Red;
									dstcs->Green      = pcscEdit[reg].Green;
									dstcs->Blue       = pcscEdit[reg].Blue;
									dstcs->Hue        = pcscEdit[reg].Hue;
									dstcs->Saturation = pcscEdit[reg].Saturation;
									dstcs->Value      = pcscEdit[reg].Value;
									AffectPaletteReg (dstcs, DstColor);
									if (DstFocus != FOCUS_COLORS) {
										src_csrt->Reg = DstColor;
									}
									break;
								}
							}
							UpdateHardwareColors (GlobalPalette, FALSE);
							UpdateCycleRegs ();
						}					
					} else { /* DstFocus == FOCUS_REGS */
						CS_RegTracker	*dst_csrt;
						CS_Color		*srccs;
						CS_Color		*dstcs;
						short			 reg;

						for (;;) {
							dst_csrt = FindColorNode (CurrentEditCycle->RegList, DstFirst);
							srccs    = GetNextSrcColor ();
							dstcs    = GetNextDstColor ();
							if (!srccs || !dstcs) {
								break;
							}
							reg      = dst_csrt->Reg;

							switch (AffectMode) {
							case AFFECT_COPY:
								dst_csrt->Reg = SrcColor;
								break;
							case AFFECT_SWAP:
								dst_csrt->Reg = SrcColor;
								srccs->Red        = pcscEdit[reg].Red;
								srccs->Green      = pcscEdit[reg].Red;
								srccs->Blue       = pcscEdit[reg].Red;
								srccs->Hue        = pcscEdit[reg].Red;
								srccs->Saturation = pcscEdit[reg].Red;
								srccs->Value      = pcscEdit[reg].Red;
								AffectPaletteReg (srccs, SrcColor);
								break;
							}
						}
						UpdateHardwareColors (GlobalPalette, FALSE);
						UpdateCycleRegs ();
					}
				}
			}
		}
	}

} /* AffectColorDest */

/*********************************************************************
 *
 * ChangeColorFocus
 *
 * SYNOPSIS
 *		LOCAL void ChangeColorFocus (short newfocus)
 *
 * PURPOSE
 *		Erase old focus visual feedback.
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
LOCAL void ChangeColorFocus (short newfocus)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ChangeColorFocus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	BeforeGraphics ();
	if (AffectMode != AFFECT_NONE) {
		AffectMode = AFFECT_NONE;
		RedrawTitle (-1);
	}
	ResetSpread ();

	switch (ColorFocus) {
	case FOCUS_PALETTE:
		DrawPaletteBox ();
		break;
	case FOCUS_REGS:
		EraseArrow (CR_TOP + REGS_TOP - 6);
		break;
	case FOCUS_COLORS:
		EraseArrow (CR_TOP + COLORS_TOP - 6);
		break;
	case FOCUS_CYCLES:
		ColorFocus = FOCUS_NONE;
		UpdateCycleArrow ();
		break;
	}
	AfterGraphics ();

	ColorFocus = newfocus;

	switch (ColorFocus) {
	case FOCUS_PALETTE:
		{
			short	 i;

			for (i = 0; i < 16; i++) {
				GlobalPalette[FIRSTCYCLECOLOR + i].Red    = pcscEdit[FIRSTCYCLECOLOR + i].Red;
				GlobalPalette[FIRSTCYCLECOLOR + i].Green  = pcscEdit[FIRSTCYCLECOLOR + i].Green;
				GlobalPalette[FIRSTCYCLECOLOR + i].Blue   = pcscEdit[FIRSTCYCLECOLOR + i].Blue;
				GlobalPalette[FIRSTCYCLECOLOR + i].Flags |= PAL_CHANGED;
			}
			UpdateHardwareColors (GlobalPalette, FALSE);
		}
		break;
	case FOCUS_REGS:
	case FOCUS_COLORS:
	case FOCUS_CYCLES:
		UseCyclePalette ();
		break;
	}

} /* ChangeColorFocus */

/*********************************************************************
 *
 * FixColors
 *
 * SYNOPSIS
 *		LOCAL void FixColors (void)
 *
 * PURPOSE
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
LOCAL void FixColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FixColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		CS_Color	*cs1;
		CS_Color	*cs2;
		CS_Color	*cs;
		short		 dist;

		switch (ColorFocus) {
		case FOCUS_PALETTE:
			dist = PalEnd - PalStart + 1;
			break;
		case FOCUS_COLORS:
			dist = LastColor - FirstColor + 1;
			break;
		case FOCUS_REGS:
			dist = LastReg - FirstReg + 1;
			break;
		default:
			return;
		}

		cs2 = GetLastSrcColor ();

		SrcDir = 1;
		cs1 = GetFirstSrcColor ();
		if (dist < 1 || !cs1 || !cs2) {
			return;
		}

		while ((cs = GetNextSrcColor ()) != NULL) {
			cs->Red   = BOUNDCOLOR(cs->Red  );
			cs->Green = BOUNDCOLOR(cs->Green);
			cs->Blue  = BOUNDCOLOR(cs->Blue );
			cs->Hue   = BOUNDCOLORh(cs->Hue );
			cs->Saturation = BOUNDCOLOR(cs->Saturation);
			cs->Value = BOUNDCOLOR(cs->Value);
		}
		ResetAffect ();
	}

} /* FixColors */

/*********************************************************************
 *
 * SelectPaletteColors
 *
 * SYNOPSIS
 *		LOCAL void SelectPaletteColors (HitInfo *hi)
 *
 * PURPOSE
 *		Select (Highlight) one or more colors in the color palette.
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
LOCAL void SelectPaletteColors (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SelectPaletteColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short		 x      = hi->X - 1;
		short		 y      = hi->Y - 1;
		UWORD		 button = hi->Buttons;

		x = max (0, x);
		y = max (0, y);
		x = x / PCOLOR_WIDTH;
		y = y / PCOLOR_HEIGHT;
		x = min (x, PCOLORS_ACROSS - 1);
		y = min (y, PCOLORS_DOWN   - 1);

		OneColor = TRUE;
		FixColors ();
		if (button & MOUSE_FIRSTDOWN) {
			AffectColorDest (x, y, FOCUS_PALETTE);
			ChangeColorFocus (FOCUS_PALETTE);
			PalX1 = PalX2 = x;
			PalY1 = PalY2 = y;
			BeforeGraphics ();
			DrawPaletteBox ();
			UpdateRGB (&pcscEdit[PalX2 * 8 + PalY2]);
			RedrawPalTitle (PalX2 * 8 + PalY2);
			AfterGraphics ();
		} else if (button & MOUSE_LEFTBUTTON) {
			if (x != PalX2 || y != PalY2) {
				BeforeGraphics ();
				DrawPaletteBox ();
				PalX2 = x;
				PalY2 = y;
				UpdateRGB (&pcscEdit[PalX2 * 8 + PalY2]);
				DrawPaletteBox ();
				RedrawPalTitle (PalX2 * 8 + PalY2);
				AfterGraphics ();
			}
		} else {
			short	temp;

			PalStart = PalX1 * 8 + PalY1;
			PalEnd   = PalX2 * 8 + PalY2;

			if (PalStart > PalEnd)
			{
				temp     = PalStart;
				PalStart = PalEnd;
				PalEnd   = temp;
			}

			OneColor = (PalStart == PalEnd);
			UpdateRGB (NULL);
//			UpdateRGB (&pcscEdit[PalX2 * 8 + PalY2]);
		}
	}
} /* SelectPaletteColors */

/*********************************************************************
 *
 * EditColorSlider
 *
 * SYNOPSIS
 *		LOCAL void EditColor (HitInfo *hi)
 *
 * PURPOSE
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
LOCAL void EditColor (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "EditColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		static short SLin;
		static short SLorigp;
		static short SLfirst;

		short		 y      = hi->Y;
		UWORD		 button = hi->Buttons;
		HitArea		*ha     = hi->HitArea;
		short		 slide  = ha->ID;
		short		 pos;
		short		 step;

		if (SlideMode == RGB_MODE) {
			step = SLIDE_POSITIONS / ColorRange[slide];
			if (SLIDE_POSITIONS % ColorRange[slide]) {
				step++;
			}
			step = max (step, 4);
		} else {
			step = 4;
		}

		if (button & MOUSE_FIRSTDOWN) {
			short	 slidepos;

			if (SlideMode == HSV_MODE) {
				pos = RGBSliderPos[slide];
			} else {
				pos = OneColor ?
					((UWORD)RGBSliderPos[slide] * (UWORD)ColorRange[slide] / (UWORD)MAX_COLOR_RANGE) :
					(short)(((ULONG)RGBSliderPos[slide] * (ULONG)(ColorRange[slide] * 2L - 1L) / (ULONG)511L)) ;
				pos = OneColor ?
					((UWORD)pos * (UWORD)MAX_HARDWARE_COLOR / (UWORD)ColorMax[slide]) :
					((UWORD)((ULONG)pos * (ULONG)511L / (ULONG)(ColorMax[slide] * 2L + 1L))) ;
			}

			slidepos = OneColor ?
							((short)((long)pos * (long)SLIDE_POSITIONS / (long)MAX_HARDWARE_COLOR)) :
							((short)((long)pos * (long)SLIDE_POSITIONS / (long)511L)) ;
			SLfirst  = y;
			SLorigp  = slidepos;
			if (y >= slidepos && y <= slidepos + SLIDER_HEIGHT) {
				SLin = TRUE;
			} else {
				SLin = FALSE;
				if (y < slidepos) {
					slidepos -= step;
				} else {
					slidepos += step;
				}
				slidepos = max (0, slidepos);
				slidepos = min (slidepos, SLIDE_POSITIONS);
				RGBSliderPos[slide] = OneColor ?
										((short)((long)slidepos * (long)MAX_HARDWARE_COLOR / (long)SLIDE_POSITIONS)) :
										((short)((long)slidepos * (long)511L / (long)SLIDE_POSITIONS)) ;
				UpdateRGBSlider (slide);
				AffectVal[slide] = OneColor ?
									(MAX_HARDWARE_COLOR - RGBSliderPos[slide]) :
									(511 - RGBSliderPos[slide]) ;
				AffectDest ();
			}
		} else if (button & MOUSE_LEFTBUTTON) {
			if (SLin) {
				short	 slidepos;

				slidepos = SLorigp + (y - SLfirst);
				slidepos = max (0, slidepos);
				slidepos = min (slidepos, SLIDE_POSITIONS);
				RGBSliderPos[slide] = OneColor ?
										((short)((long)slidepos * (long)MAX_HARDWARE_COLOR / (long)SLIDE_POSITIONS)) :
										((short)((long)slidepos * (long)511L / (long)SLIDE_POSITIONS)) ;
				UpdateRGBSlider (slide);
				AffectVal[slide] = OneColor ?
									(MAX_HARDWARE_COLOR - RGBSliderPos[slide]) :
									(511 - RGBSliderPos[slide]) ;
				AffectDest ();
			}
		}
	}
} /* EditColor */

/*********************************************************************
 *
 * IncDecColors
 *
 * SYNOPSIS
 *		LOCAL void IncDecColors (HitInfo *hi)
 *
 * PURPOSE
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
LOCAL void IncDecColors (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "IncDecColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		UWORD		 button = hi->Buttons;
		HitArea		*ha     = hi->HitArea;
		UWORD		 slideD = ha->ID;
		short		 slide;
		short		 dir;
		short		 slidepos;
		short		 step;
		

		if (!EUICountDownTimer || (button & MOUSE_FIRSTDOWN)) {
			if (button & MOUSE_FIRSTDOWN) {
				EUICountDownTimer = EUI_TIMER_HERTZ / 3;
			} else {
				EUICountDownTimer = EUI_TIMER_HERTZ / 10;
			}

			slide    = slideD & 0x0F;
			dir      = (slideD & 0x10) ? ( 1) : (-1);

			if (SlideMode == RGB_MODE) {
				step     = MAX_COLOR_RANGE / ColorRange[slide];
				if (MAX_COLOR_RANGE % ColorRange[slide]) {
					step++;
				}
			} else {
				step = 1;
			}

			slidepos = RGBSliderPos[slide] + dir * step;
			slidepos = max (0, slidepos);
			slidepos = OneColor ?
							min (slidepos, MAX_HARDWARE_COLOR) :
							min (slidepos, 511) ;
			RGBSliderPos[slide] = slidepos;
			UpdateRGBSlider (slide);
			AffectVal[slide] = OneColor ?
								(MAX_HARDWARE_COLOR - slidepos) :
								(511 - RGBSliderPos[slide]);
			AffectDest ();
		}

	}

} /* IncDecColors */

/*********************************************************************
 *
 * ToggleRGBvsHSV
 *
 * SYNOPSIS
 *		LOCAL void ToggleRGBvsHSV (void)
 *
 * PURPOSE
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
LOCAL void ToggleRGBvsHSV (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleRGBvsHSV";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	FixColors ();
	SlideMode = (SlideMode + 1) % 2;
	UpdateRGBvsHSV ();
	UpdateRGB (&KludgeCS);

} /* ToggleRGBvsHSV */

/*********************************************************************
 *
 * ToggleCycleOnOff
 *
 * SYNOPSIS
 *		LOCAL void ToggleCycleOnOff (void) 
 *
 * PURPOSE
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
LOCAL void ToggleCycleOnOff (void) 
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleCycleOnOff";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		FixColors ();
		if (CurrentEditCycle) {
			CS_CycleInfo	*csci;
			short			 i;

			csci = CurrentEditCycle;
			for (i = FirstCycle; i <= LastCycle; i++) {
				if (!EndOfList (csci)) {
					csci->Flags ^= CI_ENABLED;
					csci = Next (csci);
				}
			}
		}
		UpdateCycleFlag ();
	}
} /* ToggleCycleOnOff */

/*********************************************************************
 *
 * ToggleDirection
 *
 * SYNOPSIS
 *		LOCAL void ToggleDirection (void)
 *
 * PURPOSE
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
LOCAL void ToggleDirection (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleDirection";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		FixColors ();
		if (CurrentEditCycle) {
			CS_CycleInfo	*csci;
			short			 i;

			csci = CurrentEditCycle;
			for (i = FirstCycle; i <= LastCycle; i++) {
				if (!EndOfList (csci)) {
					csci->Direction = !csci->Direction;
					csci = Next (csci);
				}
			}
		}
		UpdateDirection ();
	}
} /* ToggleDirection */

/*********************************************************************
 *
 * RealIncDecCycles
 *
 * SYNOPSIS
 *		LOCAL void RealIncDecCycles (short dir)
 *
 * PURPOSE
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
LOCAL void RealIncDecCycles (short dir)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RealIncDecCycles";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		CS_CycleInfo	*csci    = EditCI->EditCycle;
		short			 moved   = FALSE;
		short			 firstID;

		FixColors ();
		UseCyclePalette ();

		if (!EndOfList (csci)) {
			firstID = csci->ID;
		} else {
			firstID = EditCI->NumCycles;
		}

		if (dir == 1) {
			if (EditCI->NumCycles - firstID > 15) {
				EditCI->EditCycle = Next (EditCI->EditCycle);
				moved = TRUE;
			}
		} else {
			if (firstID) {
				moved = TRUE;
				EditCI->EditCycle = Prev (EditCI->EditCycle);
			}
		}

		if (moved) {
			BeforeGraphics ();
			UpdateCycles ();
			UpdateCycleArrow ();
			AfterGraphics ();
		}
	}
} /* RealIncDecCycles */

/*********************************************************************
 *
 * RealIncDecColors
 *
 * SYNOPSIS
 *		LOCAL void RealIncDecColors (short dir)
 *
 * PURPOSE
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
LOCAL void RealIncDecColors (short dir)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RealIncDecColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	UseCyclePalette ();

	FixColors ();
	if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {

		CS_ColorTracker	*csct    = CurrentEditCycle->EditColor;
		short			 moved   = FALSE;
		short			 firstID;

		if (!EndOfList (csct)) {
			firstID = csct->ID;
		} else {
			firstID = CurrentEditCycle->NumColors;
		}

		if (dir == 1) {
			if (CurrentEditCycle->NumColors - firstID > 15) {
				CurrentEditCycle->EditColor = Next (CurrentEditCycle->EditColor);
				moved = TRUE;
			}
		} else {
			if (firstID) {
				moved = TRUE;
				CurrentEditCycle->EditColor = Prev (CurrentEditCycle->EditColor);
			}
		}

		if (moved) {
			BeforeGraphics ();
			UpdateCycleColors ();
			UpdateColorArrow ();
			AfterGraphics ();
		}
	}
} /* RealIncDecColors */

/*********************************************************************
 *
 * RealIncDecRegs
 *
 * SYNOPSIS
 *		LOCAL void RealIncDecRegs (short dir)
 *
 * PURPOSE
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
LOCAL void RealIncDecRegs (short dir)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RealIncDecRegs";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	UseCyclePalette ();

	FixColors ();
	if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {

		CS_RegTracker	*csrt    = CurrentEditCycle->EditReg;
		short			 moved   = FALSE;
		short			 firstID;

		if (!EndOfList (csrt)) {
			firstID = csrt->ID;
		} else {
			firstID = CurrentEditCycle->NumRegs;
		}

		if (dir == 1) {
			if (CurrentEditCycle->NumRegs - firstID > 15) {
				CurrentEditCycle->EditReg = Next (CurrentEditCycle->EditReg);
				moved = TRUE;
			}
		} else {
			if (firstID) {
				moved = TRUE;
				CurrentEditCycle->EditReg = Prev (CurrentEditCycle->EditReg);
			}
		}

		if (moved) {
			BeforeGraphics ();
			UpdateCycleRegs ();
			UpdateRegsArrow ();
			AfterGraphics ();
		}
	}
} /* RealIncDecRegs */

/*********************************************************************
 *
 * UpdateCycleInfo
 *
 * SYNOPSIS
 *		LOCAL void UpdateCycleInfo (void)
 *
 * PURPOSE
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
LOCAL void UpdateCycleInfo (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateCycleInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	BeforeGraphics ();
	UpdateCycleColors ();
	UpdateCycleRegs ();
	UpdateColorCount ();
	UpdateRegCount ();
	UpdateCycleFlag ();
	UpdateSpeed ();
	UpdateCycleFlag ();
	UpdateDirection ();
	AfterGraphics ();

} /* UpdateCycleInfo */

/*********************************************************************
 *
 * SelectCycles
 *
 * SYNOPSIS
 *		LOCAL void SelectCycles (HitInfo *hi)
 *
 * PURPOSE
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
LOCAL void SelectCycles (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SelectCycles";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		short			 x       = hi->X - 1;
		UWORD			 button  = hi->Buttons;
		CS_CycleInfo	*csci    = EditCI->EditCycle;
		short			 firstID;

		x = x / LISTCOLOR_WIDTH;

		OneColor = TRUE;
		FixColors ();
		if (button & MOUSE_FIRSTDOWN) {
			EUICountDownTimer = 0;
			if (!EndOfList (csci)) {
				firstID = csci->ID;
			} else {
				firstID = EditCI->NumCycles;
			}

			x = max (0, x);
			x = min (15, x);
			AffectColorDest (x, 0, FOCUS_CYCLES);
			ChangeColorFocus (FOCUS_CYCLES);
			FirstCycle = LastCycle = firstID + x;
			if (FirstCycle <= EditCI->NumCycles) {
				CurrentEditCycle = FindColorNode (EditCI->CycleList, LastCycle);
			} else {
				CurrentEditCycle = NULL;
				CurrentCycle     = 0;
			}
			BeforeGraphics ();
			UpdateCycleArrow ();
			UpdateCycleInfo ();
			AfterGraphics ();
		} else if (button & MOUSE_LEFTBUTTON) {
			if (FirstCycle <= EditCI->NumCycles) {
				short	newcycle;

				if (!EUICountDownTimer && (x < 0 || x > 15)) {
					EUICountDownTimer = EUI_TIMER_HERTZ / 15;
					if (x < 0) {
						RealIncDecCycles (-1);
					} if (x > 15) {
						RealIncDecCycles ( 1);
					}
				}

				x = max (0, x);
				x = min (15, x);

				if (!EndOfList (csci)) {
					firstID = csci->ID;
				} else {
					firstID = EditCI->NumCycles;
				}

				newcycle = firstID + x;
				newcycle = min (newcycle, EditCI->NumCycles);

				if (newcycle != LastCycle) {
					CurrentEditCycle = FindColorNode (EditCI->CycleList, newcycle);
					LastCycle = CurrentCycle = newcycle;
					BeforeGraphics ();
					UpdateCycleArrow ();
					UpdateCycleInfo ();
					AfterGraphics ();
				}
			}
		} else {
			if (FirstCycle <= EditCI->NumCycles) {
				if (FirstCycle > LastCycle) {
					short	temp;

					temp         = FirstCycle;
					FirstCycle   = LastCycle;
					LastCycle    = temp;
					CurrentCycle = FirstCycle;
					CurrentEditCycle = FindColorNode (EditCI->CycleList, CurrentCycle);
				} else {
					CurrentCycle = LastCycle;
				}
				CurrentEditCycle = FindColorNode (EditCI->CycleList, CurrentCycle);
			}
		}
	}
} /* SelectCycles */

/*********************************************************************
 *
 * SelectColors
 *
 * SYNOPSIS
 *		LOCAL void SelectColors (HitInfo *hi)
 *
 * PURPOSE
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
LOCAL void SelectColors (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SelectColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {

		short			 x       = hi->X - 1;
		UWORD			 button  = hi->Buttons;
		CS_ColorTracker	*csct    = CurrentEditCycle->EditColor;
		short			 firstID;

		x = x / LISTCOLOR_WIDTH;

		OneColor = TRUE;
		FixColors ();
		if (button & MOUSE_FIRSTDOWN) {
			EUICountDownTimer = 0;
			if (!EndOfList (csct)) {
				firstID = csct->ID;
			} else {
				firstID = CurrentEditCycle->NumColors;
			}

			x = max (0, x);
			x = min (15, x);
			AffectColorDest (x, 0, FOCUS_COLORS);
			ChangeColorFocus (FOCUS_COLORS);
			FirstColor = LastColor = firstID + x;
			if (FirstColor <= CurrentEditCycle->NumColors) {
				BeforeGraphics ();
				if (LastColor != CurrentEditCycle->NumColors) {
					UpdateRGB (&((CS_ColorTracker *)FindColorNode (CurrentEditCycle->ColorList, LastColor))->Color);
					RedrawTitle (FIRSTCYCLECOLOR + x);
				}
				UpdateColorArrow ();
				AfterGraphics ();
			}
		} else if (button & MOUSE_LEFTBUTTON) {
			if (FirstColor <= CurrentEditCycle->NumColors) {
				short	newcolor;

				if (!EUICountDownTimer && (x < 0 || x > 15)) {
					EUICountDownTimer = EUI_TIMER_HERTZ / 15;
					if (x < 0) {
						RealIncDecColors (-1);
					} if (x > 15) {
						RealIncDecColors ( 1);
					}
				}

				x = max (0, x);
				x = min (15, x);

				if (!EndOfList (csct)) {
					firstID = csct->ID;
				} else {
					firstID = CurrentEditCycle->NumColors;
				}

				newcolor = firstID + x;
				newcolor = min (newcolor, CurrentEditCycle->NumColors);

				if (newcolor != LastColor) {
					LastColor = newcolor;
					BeforeGraphics ();
					if (LastColor != CurrentEditCycle->NumColors) {
						UpdateRGB (&((CS_ColorTracker *)FindColorNode (CurrentEditCycle->ColorList, LastColor))->Color);
						RedrawTitle (FIRSTCYCLECOLOR + x);
					}
					UpdateColorArrow ();
					AfterGraphics ();
				}
			}
		} else {
			if (FirstColor > LastColor) {
				short	temp;

				temp         = FirstColor;
				FirstColor   = LastColor;
				LastColor    = temp;
			}
			OneColor = (FirstColor == LastColor);
			UpdateRGB (NULL);
//			UpdateRGB (&((CS_ColorTracker *)FindColorNode (CurrentEditCycle->ColorList, LastColor))->Color);
		}
	}
} /* SelectColors */

/*********************************************************************
 *
 * SelectRegs
 *
 * SYNOPSIS
 *		LOCAL void SelectRegs (HitInfo *hi)
 *
 * PURPOSE
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
LOCAL void SelectRegs (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SelectRegs";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {

		short			 x       = hi->X - 1;
		UWORD			 button  = hi->Buttons;
		CS_RegTracker	*csrt    = CurrentEditCycle->EditReg;
		short			 firstID;

		x = x / LISTCOLOR_WIDTH;

		OneColor = TRUE;
		FixColors ();
		if (button & MOUSE_FIRSTDOWN) {
			EUICountDownTimer = 0;
			if (!EndOfList (csrt)) {
				firstID = csrt->ID;
			} else {
				firstID = CurrentEditCycle->NumRegs;
			}

			x = max (0, x);
			x = min (15, x);
			AffectColorDest (x, 0, FOCUS_REGS);
			ChangeColorFocus (FOCUS_REGS);
			FirstReg = LastReg = firstID + x;
			if (FirstReg <= CurrentEditCycle->NumRegs) {
				BeforeGraphics ();
				if (LastReg != CurrentEditCycle->NumRegs) {
					CS_RegTracker	*rt;

					rt = FindColorNode (CurrentEditCycle->RegList, LastReg);
					UpdateRGBReg (rt);
					RedrawTitle (rt->Reg);
				}
				UpdateRegsArrow ();
				AfterGraphics ();
			}
		} else if (button & MOUSE_LEFTBUTTON) {
			if (FirstReg <= CurrentEditCycle->NumRegs) {
				short	newreg;

				if (!EUICountDownTimer && (x < 0 || x > 15)) {
					EUICountDownTimer = EUI_TIMER_HERTZ / 15;
					if (x < 0) {
						RealIncDecRegs (-1);
					} if (x > 15) {
						RealIncDecRegs ( 1);
					}
				}

				x = max (0, x);
				x = min (15, x);

				if (!EndOfList (csrt)) {
					firstID = csrt->ID;
				} else {
					firstID = CurrentEditCycle->NumRegs;
				}

				newreg = firstID + x;
				newreg = min (newreg, CurrentEditCycle->NumRegs);

				if (newreg != LastReg) {
					LastReg = newreg;
					BeforeGraphics ();
					if (LastReg != CurrentEditCycle->NumRegs) {
						CS_RegTracker	*rt;

						rt = FindColorNode (CurrentEditCycle->RegList, LastReg);
						UpdateRGBReg (rt);
						RedrawTitle (rt->Reg);
					}
					UpdateRegsArrow ();
					AfterGraphics ();
				}
			}
		} else {
			if (FirstReg > LastReg) {
				short	temp;

				temp         = FirstReg;
				FirstReg   = LastReg;
				LastReg    = temp;
			}
		}
	}
} /* SelectRegs */

/*********************************************************************
 *
 * IncDecCycles
 *
 * SYNOPSIS
 *		LOCAL void IncDecCycles (HitInfo *hi)
 *
 * PURPOSE
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
LOCAL void IncDecCycles (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "IncDecCycles";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		UWORD		 button = hi->Buttons;
		HitArea		*ha     = hi->HitArea;
		short		 dir    = (ha->ID == 1) ? 1 : (-1);

		FixColors ();
		if (button & MOUSE_FIRSTDOWN) {
			EUICountDownTimer = 0;
		} else if (!EUICountDownTimer) {
			EUICountDownTimer = EUI_TIMER_HERTZ / 10;
			RealIncDecCycles (dir);
		}

	}
} /* IncDecCycles */

/*********************************************************************
 *
 * IncDecCycleColors
 *
 * SYNOPSIS
 *		LOCAL void IncDecCycleColors (HitInfo *hi)
 *
 * PURPOSE
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
LOCAL void IncDecCycleColors (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "IncDecCycleColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		UWORD		 button = hi->Buttons;
		HitArea		*ha     = hi->HitArea;
		short		 dir    = (ha->ID == 1) ? 1 : (-1);

		FixColors ();
		if (button & MOUSE_FIRSTDOWN) {
			EUICountDownTimer = 0;
		} else if (!EUICountDownTimer) {
			EUICountDownTimer = EUI_TIMER_HERTZ / 10;
			RealIncDecColors (dir);
		}

	}
} /* IncDecCycleColors */

/*********************************************************************
 *
 * IncDecCycleRegs
 *
 * SYNOPSIS
 *		LOCAL void IncDecCycleRegs (HitInfo *hi)
 *
 * PURPOSE
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
LOCAL void IncDecCycleRegs (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "IncDecCycleRegs";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		UWORD		 button = hi->Buttons;
		HitArea		*ha     = hi->HitArea;
		short		 dir    = (ha->ID == 1) ? 1 : (-1);

		FixColors ();
		if (button & MOUSE_FIRSTDOWN) {
			EUICountDownTimer = 0;
		} else if (!EUICountDownTimer) {
			EUICountDownTimer = EUI_TIMER_HERTZ / 10;
			RealIncDecRegs (dir);
		}

	}
} /* IncDecCycleRegs */

/*********************************************************************
 *
 * InsertColors
 *
 * SYNOPSIS
 *		LOCAL void InsertColors (void)
 *
 * PURPOSE
 *		Insert Colors / Regs or Cycles.
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
LOCAL void InsertColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InsertColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		FixColors ();
		switch (ColorFocus) {
		case FOCUS_CYCLES:
			if (CurrentEditCycle) {
				CS_CycleInfo	*csci;
				CS_CycleInfo	*ipoint;

				ipoint = FindColorNode (EditCI->CycleList, FirstCycle);
				csci   = CreateCycleInfo ();
				if (csci) {
					InsertBefore (ipoint, csci);
					EditCI->NumCycles++;
					FirstCycle++;
					LastCycle++;
					CurrentCycle++;
					RenumberCycleInfos (EditCI->CycleList);
					ipoint = Prev (EditCI->EditCycle);
					if (!StartOfList (ipoint)) {
						EditCI->EditCycle = ipoint;
					}
					BeforeGraphics ();
					UpdateCycles ();
					UpdateCycleArrow ();
					AfterGraphics ();
				}
			}
			break;
		case FOCUS_COLORS:
			if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
				CS_ColorTracker	*csct;
				CS_ColorTracker	*ipoint;

				ipoint = FindColorNode (CurrentEditCycle->ColorList, FirstColor);
				csct   = CreateCycleColor ();
				if (csct) {
					InsertBefore (ipoint, csct);
					CurrentEditCycle->NumColors++;
					FirstColor++;
					LastColor++;
					RenumberCS_Colors (CurrentEditCycle->ColorList);
					ipoint = Prev (CurrentEditCycle->EditColor);
					if (!StartOfList (ipoint)) {
						CurrentEditCycle->EditColor = ipoint;
					}
					BeforeGraphics ();
					UpdateCycleColors ();
					UpdateColorArrow ();
					UpdateColorCount ();
					AfterGraphics ();
				}
			}
			break;
		case FOCUS_REGS:
			if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
				CS_RegTracker	*csrt;
				CS_RegTracker	*ipoint;

				ipoint = FindColorNode (CurrentEditCycle->RegList, FirstReg);
				csrt   = CreateRegTracker ();
				if (csrt) {
					InsertBefore (ipoint, csrt);
					CurrentEditCycle->NumRegs++;
					FirstReg++;
					LastReg++;
					RenumberCS_Regs (CurrentEditCycle->RegList);
					ipoint = Prev (CurrentEditCycle->EditReg);
					if (!StartOfList (ipoint)) {
						CurrentEditCycle->EditReg = ipoint;
					}
					BeforeGraphics ();
					UpdateCycleRegs ();
					UpdateRegsArrow ();
					UpdateRegCount ();
					AfterGraphics ();
				}
			}
			break;
		}
	}
} /* InsertColors */

/*********************************************************************
 *
 * DeleteColors
 *
 * SYNOPSIS
 *		LOCAL void DeleteColors (void)
 *
 * PURPOSE
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
LOCAL void DeleteColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		FixColors ();
		switch (ColorFocus) {
		case FOCUS_CYCLES:
			if (CurrentEditCycle) {
				CS_CycleInfo	*nextcs;
				CS_CycleInfo	*ipoint;
				short			 i;
				short			 new = FALSE;

				ipoint = EditCI->EditCycle;
				if (!EndOfList(ipoint) && 
					(ipoint->ID >= FirstCycle && ipoint->ID <= LastCycle)) {
					new = TRUE;
				}

				ipoint = FindColorNode (EditCI->CycleList, FirstCycle);
				if (!EndOfList (ipoint)) {
					for (i = FirstCycle; i <= LastCycle; i++) {
						nextcs = Next (ipoint);
						if (!EndOfList (ipoint)) {
							Remove (ipoint);
							DeleteCycleInfo (ipoint);
							EditCI->NumCycles--;
							ipoint = nextcs;
						}
					}
				}

				if (new) {
					if (EditCI->NumCycles) {
						ipoint = Prev (ipoint);
						if (StartOfList (ipoint)) {
							ipoint = Next (ipoint);
						}
					}
					EditCI->EditCycle = ipoint;
				}

				RenumberCycleInfos (EditCI->CycleList);

				if (!EndOfList (ipoint)) {
					CurrentCycle     = ipoint->ID;
					CurrentEditCycle = ipoint;
				} else {
					CurrentCycle     = EditCI->NumCycles;
					CurrentEditCycle = 0;
				}

				BeforeGraphics ();
				ChangeColorFocus (FOCUS_NONE);
				UpdateCycles ();
				UpdateCycleInfo ();
				AfterGraphics ();
			}
			break;
		case FOCUS_COLORS:
			if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {

				CS_ColorTracker	*nextct;
				CS_ColorTracker	*ipoint;
				short			 i;
				short			 new = FALSE;

				ipoint = CurrentEditCycle->EditColor;
				if (!EndOfList(ipoint) && 
					(ipoint->ID >= FirstColor && ipoint->ID <= LastColor)) {
					new = TRUE;
				}

				ipoint = FindColorNode (CurrentEditCycle->ColorList, FirstColor);
				if (!EndOfList (ipoint)) {
					for (i = FirstColor; i <= LastColor; i++) {
						nextct = Next (ipoint);
						if (!EndOfList (ipoint)) {
							Remove (ipoint);
							DeleteCycleColor (ipoint);
							CurrentEditCycle->NumColors--;
							ipoint = nextct;
						}
					}
				}

				if (new) {
					if (CurrentEditCycle->NumColors) {
						ipoint = Prev (ipoint);
						if (StartOfList (ipoint)) {
							ipoint = Next (ipoint);
						}
					}
					CurrentEditCycle->EditColor = ipoint;
				}

				RenumberCS_Colors (CurrentEditCycle->ColorList);

				BeforeGraphics ();
				ChangeColorFocus (FOCUS_NONE);
				UpdateCycleColors ();
				UpdateColorCount ();
				AfterGraphics ();
			}
			break;
		case FOCUS_REGS:
			if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {

				CS_RegTracker	*nextct;
				CS_RegTracker	*ipoint;
				short			 i;
				short			 new = FALSE;

				ipoint = CurrentEditCycle->EditReg;
				if (!EndOfList(ipoint) && 
					(ipoint->ID >= FirstReg && ipoint->ID <= LastReg)) {
					new = TRUE;
				}

				ipoint = FindColorNode (CurrentEditCycle->RegList, FirstReg);
				if (!EndOfList (ipoint)) {
					for (i = FirstReg; i <= LastReg; i++) {
						nextct = Next (ipoint);
						if (!EndOfList (ipoint)) {
							Remove (ipoint);
							DeleteRegTracker (ipoint);
							CurrentEditCycle->NumRegs--;
							ipoint = nextct;
						}
					}
				}

				if (new) {
					if (CurrentEditCycle->NumRegs) {
						ipoint = Prev (ipoint);
						if (StartOfList (ipoint)) {
							ipoint = Next (ipoint);
						}
					}
					CurrentEditCycle->EditReg = ipoint;
				}

				RenumberCS_Regs (CurrentEditCycle->RegList);

				BeforeGraphics ();
				ChangeColorFocus (FOCUS_NONE);
				UpdateCycleRegs ();
				UpdateRegCount ();
				AfterGraphics ();
			}
			break;
		}
	}
} /* DeleteColors */

/*********************************************************************
 *
 * SpreadColors
 *
 * SYNOPSIS
 *		LOCAL void SpreadColors (void)
 *
 * PURPOSE
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
LOCAL void SpreadColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SpreadColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		CS_Color	*cs1;
		CS_Color	*cs2;
		CS_Color	*cs;
		long		 start[3];
		long		 goal[3];
		long		 add[3];
		short		 dist;
		short		 i;
		short		 oldmode = SlideMode;

		FixColors ();
		SlideMode = SpreadMode;

		switch (ColorFocus) {
		case FOCUS_PALETTE:
			dist = PalEnd - PalStart + 1;
			break;
		case FOCUS_COLORS:
			dist = LastColor - FirstColor + 1;
			break;
		case FOCUS_REGS:
			dist = LastReg - FirstReg + 1;
			break;
		default:
			return;
		}
		dist--;

		cs2 = GetLastSrcColor ();
		if (SpreadMode == RGB_MODE) {
			goal[0]  = cs2->Red;
			goal[1]  = cs2->Green;
			goal[2]  = cs2->Blue;
		} else {	
			goal[0]  = cs2->Hue;
			goal[1]  = cs2->Saturation;
			goal[2]  = cs2->Value;
		}

		SrcDir = 1;
		cs1 = GetFirstSrcColor ();
		if (dist < 2 || !cs1 || !cs2) {
			return;
		}

		if (SpreadMode == RGB_MODE) {
			start[0] = cs1->Red;
			start[1] = cs1->Green;
			start[2] = cs1->Blue;
		} else {
			start[0] = cs1->Hue;
			start[1] = cs1->Saturation;
			start[2] = cs1->Value;

			{
				long	absdist;

				absdist = goal[0] - start[0];
				if (absdist < 0) {
					absdist = (-absdist);
				}

				if (absdist > 180) {
					if (start[0] < goal[0]) {
						start[0] += 360;
					} else {
						goal[0]  += 360;
					}
				}
			}
		}

		for (i = 0; i < 3; i++) {
			start[i] *= 256;
			add[i]    = ((goal[i] * 256) - start[i]) / dist;
		}

		if (GetNextSrcColor ()) {
			BeforeGraphics ();
			Spreading = TRUE;
			while (dist && ((cs = GetNextSrcColor ())) != NULL) {
				for (i = 0; i < 3; i++) {
					start[i]    += add[i];
					AffectVal[i] = (short) ((start[i] / 256) % 360);
				}
				AffectSrcColor (cs, SrcColor);
			}
			Spreading = FALSE;
			UpdateHardwareColors (GlobalPalette, FALSE);
			UpdateCycleRegs ();
			AfterGraphics ();
		}
		SlideMode  = oldmode;
		SpreadMode = (SpreadMode == RGB_MODE) ? HSV_MODE : RGB_MODE;
		RedrawSpread ();
		UpdateRGB (NULL);
	}

} /* SpreadColors */

/*********************************************************************
 *
 * CopyColors
 *
 * SYNOPSIS
 *		LOCAL void CopyColors (void)
 *
 * PURPOSE
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
LOCAL void CopyColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CopyColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	FixColors ();
	AffectMode = AFFECT_COPY;
	RedrawTitle (-1);

} /* CopyColors */

/*********************************************************************
 *
 * SwapColors
 *
 * SYNOPSIS
 *		LOCAL void SwapColors (void)
 *
 * PURPOSE
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
LOCAL void SwapColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SwapColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	FixColors ();
	AffectMode = AFFECT_SWAP;
	RedrawTitle (-1);

} /* SwapColors */

/*********************************************************************
 *
 * BlendColors
 *
 * SYNOPSIS
 *		LOCAL void BlendColors (void)
 *
 * PURPOSE
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
LOCAL void BlendColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "BlendColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	FixColors ();
	AffectMode = AFFECT_BLEND;
	RedrawTitle (-1);

} /* BlendColors */

/*********************************************************************
 *
 * EditSpeed
 *
 * SYNOPSIS
 *		LOCAL void EditSpeed (HitInfo *hi)
 *
 * PURPOSE
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
LOCAL void EditSpeed (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "EditSpeed";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		static short SLin;
		static short SLorigp;
		static short SLfirst;
		static UWORD Flags;
		static UWORD CycleFlag;

		short		 x      = hi->X;
		UWORD		 button = hi->Buttons;

		if (CurrentEditCycle && !EndOfList (CurrentEditCycle)) {
			if (button & MOUSE_FIRSTDOWN) {
				short	 slidepos;

				FixColors ();
				#if 1
				ResetColorInfo (EditCI);
				Flags      = CurrentEditCycle->Flags;
				CurrentEditCycle->Flags |= CI_ENABLED;
				CycleFlag  = EditCI->CycleFlag;
				EditCI->CycleFlag = TRUE;
				VBCycle    = FALSE;
				CyclePause = FALSE;
				ComputeNextPalette (GlobalPalette, EditCI, TRUE);
				VBCycle    = TRUE;
				#endif

				slidepos = (short)((long)(intsqrt(CurrentEditCycle->Speed)) * (long)SPEED_POSITIONS / (long)LOCAL_ONEPERTICK);
				SLfirst  = x;
				SLorigp  = slidepos;
				if (x >= slidepos && x < slidepos + SPSLIDER_WIDTH) {
					SLin = TRUE;
				} else {
					SLin = FALSE;
					if (x < slidepos) {
						slidepos -= 4;
					} else {
						slidepos += 4;
					}
					slidepos = max (0, slidepos);
					slidepos = min (slidepos, SPEED_POSITIONS);
					CurrentEditCycle->Speed  = (short)((long)slidepos * (long)LOCAL_ONEPERTICK / (long)SPEED_POSITIONS);
					CurrentEditCycle->Speed *= CurrentEditCycle->Speed;
					UpdateSpeed ();
				}
			} else if (button & MOUSE_LEFTBUTTON) {
				if (SLin) {
					short	 slidepos;

					slidepos = SLorigp + (x - SLfirst);
					slidepos = max (0, slidepos);
					slidepos = min (slidepos, SPEED_POSITIONS);
					CurrentEditCycle->Speed  = (short)((long)slidepos * (long)LOCAL_ONEPERTICK / (long)SPEED_POSITIONS);
					CurrentEditCycle->Speed *= CurrentEditCycle->Speed;
					UpdateSpeed ();
				}
			} else {
				#if 1
				EditCI->CycleFlag = CycleFlag;
				CurrentEditCycle->Flags = Flags;
				CyclePause = TRUE;
				ResetColorInfo (EditCI);
				ComputeNextPalette (GlobalPalette, EditCI, TRUE);
				#endif
			}
		}
	}
} /* EditSpeed */

#if UNDO_BUTTON
/*********************************************************************
 *
 * UndoColors
 *
 * PURPOSE
 *		UNDO button handler.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		
 *
 * HISTORY
 *		08/06/93 Friday (dcc) - created.
 *
*/
LOCAL void UndoColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UndoColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (ciUndo)
	{
		ColorInfo *ci;

		ci = ciUndo;
		ciUndo = EditCI;
		EditCI = ci;

		ComputeNextPalette (GlobalPalette, EditCI, TRUE);
		UpdateHardwareColors (GlobalPalette, FALSE);
	}

} /* UndoColors */
#endif

#if 0	/* DEFAULT */
	{
		short		 x      = hi->X;
		short		 y      = hi->Y;
		UWORD		 button = hi->Buttons;
		UWORD		 sflags = hi->Shiftflags;
		HitArea		*ha     = hi->HitArea;

		if (button & MOUSE_FIRSTDOWN) {
		} else if (button & MOUSE_LEFTBUTTON) {
		} else {
		}
	}
#endif

/********************************* Gadgets ********************************/

LOCAL HitArea CRHitArea[] = {
	{ PALETTE_LEFT + 1, PALETTE_TOP + 1, PALETTE_WIDTH - 2, PALETTE_HEIGHT - 2,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, SelectPaletteColors, },
	{ RGB_LEFT, RGB_TOP, RGB_WIDTH, RGB_HEIGHT,
		NULL, 0, ToggleRGBvsHSV, },
	{ RSLIDE_LEFT, CUP_ARROW_TOP, SLIDE_WIDTH, CARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT, 0x00, IncDecColors, },
	{ GSLIDE_LEFT, CUP_ARROW_TOP, SLIDE_WIDTH, CARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT, 0x01, IncDecColors, },
	{ BSLIDE_LEFT, CUP_ARROW_TOP, SLIDE_WIDTH, CARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT, 0x02, IncDecColors, },
	{ RSLIDE_LEFT, CDN_ARROW_TOP, SLIDE_WIDTH, CARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT, 0x10, IncDecColors, },
	{ GSLIDE_LEFT, CDN_ARROW_TOP, SLIDE_WIDTH, CARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT, 0x11, IncDecColors, },
	{ BSLIDE_LEFT, CDN_ARROW_TOP, SLIDE_WIDTH, CARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT, 0x12, IncDecColors, },
	{ RSLIDE_LEFT, SLIDE_TOP, SLIDE_WIDTH, SLIDE_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, EditColor, },
	{ GSLIDE_LEFT, SLIDE_TOP, SLIDE_WIDTH, SLIDE_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 1, EditColor, },
	{ BSLIDE_LEFT, SLIDE_TOP, SLIDE_WIDTH, SLIDE_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 2, EditColor, },
	{ INSERT_LEFT, INSERT_TOP, BUTTON_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, 0, InsertColors, },
	{ DELETE_LEFT, DELETE_TOP, BUTTON_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, 0, DeleteColors, },
	{ COPY_LEFT, COPY_TOP, BUTTON_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, 0, CopyColors, },
	{ SWAP_LEFT, SWAP_TOP, BUTTON_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, 0, SwapColors, },
	{ SPREAD_LEFT, SPREAD_TOP, BUTTON_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, 0, SpreadColors, },
	{ BLEND_LEFT, BLEND_TOP, BUTTON_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, 0, BlendColors, },
	#if UNDO_BUTTON
	{ UNDO_LEFT, UNDO_TOP, BUTTON_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, 0, UndoColors, },
	#endif
	{ CANCEL_LEFT, CANCEL_TOP, BUTTON_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, CANCEL_ID, NULL, },
	{ OK_LEFT, OK_TOP, OK_WIDTH, OK_HEIGHT,
		HTA_COMPLEMENT, OK_ID, NULL, },
	{ SPEED_LEFT, SPEED_TOP, SPEED_WIDTH, SPEED_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, EditSpeed, },
	{ CFLAG_LEFT, CFLAG_TOP, CFLAG_WIDTH, CFLAG_HEIGHT,
		NULL, 0, ToggleCycleOnOff, },
	{ DIR_LEFT, DIR_TOP, DIR_WIDTH, DIR_HEIGHT,
		NULL, 0, ToggleDirection, },
	{ LARROW_LEFT, REGS_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT, -1, IncDecCycleRegs, },
	{ RARROW_LEFT, REGS_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT,  1, IncDecCycleRegs, },
	{ LARROW_LEFT, COLORS_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT, -1, IncDecCycleColors, },
	{ RARROW_LEFT, COLORS_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT,  1, IncDecCycleColors, },
	{ LARROW_LEFT, CYCLES_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT, -1, IncDecCycles, },
	{ RARROW_LEFT, CYCLES_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_COMPLEMENT,  1, IncDecCycles, },
	{ EDIT_LEFT, REGS_TOP, EDIT_WIDTH, REGS_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, SelectRegs, },
	{ EDIT_LEFT, COLORS_TOP, EDIT_WIDTH, COLORS_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, SelectColors, },
	{ EDIT_LEFT, CYCLES_TOP, EDIT_WIDTH, CYCLES_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, SelectCycles, },
	{ HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, },
};

/*********************************************************************
 *
 * ColorSequencer
 *
 * SYNOPSIS
 *		short ColorSequencer (ColorInfo *ci)
 *
 * PURPOSE
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
 *		08/05/93 Thursday (dcc) - fix operation of [Spacebar].
 *
 * SEE ALSO
 *
*/
short ColorSequencer (Window *win, ColorInfo *ci)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ColorSequencer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		SaveAreaType		*sa = NULL;
		HitArea				*ha;
		ColorInfo			*cancelci;
		struct	RastPort	 rp = { NULL, };
		short				 vbtemp;
		short				 cptemp;

		CRdone  = FALSE;
		CRabort = FALSE;

		cptemp     = CyclePause;
		vbtemp     = VBCycle;
		CyclePause = TRUE;
		VBCycle    = TRUE;

		SetColorsNPointer (MainWindow);

		if (!CCBuffer)
		{
			CCBuffer = AllocXTRA (sizeof (CS_Color) * 256);
		}

		OneColor  = TRUE;
		OldOneClr = TRUE;
		ResetColorInfo (ci);
		RenumberColorInfo (ci);
		PushGlobalColors();
		SetGlobalColors (ci);
		cancelci = DuplicateColorInfo (GlobalColors);
		if (cancelci) {
			ciUndo = DuplicateColorInfoInSTD(cancelci);

			EditCI  = GlobalColors;	/* Assert: EditCI->pxtColors is in STD memory */
#if __MSDOS16__
			if (XmtFromPxtr (EditCI->pxtColors) == xmtSTD) {
#endif
				pcscEdit = ActivateXTRAatWin (EditCI->pxtColors, 0);

				/* No need for matching release because in standard memory */
#if __MSDOS16__
			} else {
				/* ci->pxtColors must reside in standard memory! */

				puts ("ColorSequencer(): bad XTRA memory type\n");
				exit (1);
			}
#endif
			CRWin   = win;
			CRFont  = &SmallFont;
			RGBSliderOld[0] = -100;
			RGBSliderOld[1] = -100;
			RGBSliderOld[2] = -100;
			OldSpeed        = -100;

			if (!CCCount)
			{
				ColorFocus = FOCUS_PALETTE;
				AffectMode = AFFECT_NONE;
			}
			else
			{
				// PrintMsg ("Colors In Buffer");
				ColorFocus = FOCUS_CCBUFFER;
				AffectMode = AFFECT_COPY;
			}
			PalX2 = PalX1;
			PalY2 = PalY1;
			PalStart = PalX1 * 8 + PalY1;
			PalEnd   = PalX2 * 8 + PalY2;

			ResetAffect ();
			CurrentEditCycle = EditCI->EditCycle;
			if (!EndOfList (CurrentEditCycle)) {
				CurrentCycle = CurrentEditCycle->ID;
			} else {
				CurrentCycle = EditCI->NumCycles;
			}

			CrColors[CR_WHITE ] = win->White;
			CrColors[CR_BLACK ] = win->Black;
			CrColors[CR_BLUE  ] = win->Blue;
			CrColors[CR_ORANGE] = win->Orange;

			if (PushWindowClipValues (win)) {
				sa = SaveArea (CR_LEFT, CR_TOP, CR_WIDTH, CR_HEIGHT);
				if (!sa) {
					GEcatf ("\nOOM: FileReq Area");
					goto dcrcleanup;
				}

				BeforeGraphics ();

				StartEPicture (&rp, &CREPicture, CR_LEFT, CR_TOP);

				{
					short	x;
					short	y;
					short	c;

					c = 0;
					for (x = 0; x < PCOLORS_ACROSS; x++) {
						for (y = 0 ; y < PCOLORS_DOWN; y++) {
							SetPenColor (c);
							DrawRect (CR_LEFT + PALETTE_LEFT + 1 + x * PCOLOR_WIDTH,
								  	CR_TOP  + PALETTE_TOP  + 1 + y * PCOLOR_HEIGHT,
								  	PCOLOR_WIDTH,
								  	PCOLOR_HEIGHT);
							c++;
						}
 					}
				}

				if (ColorReqDivideX) {
					short	x;

					SetPenColor (CrColors[CR_WHITE]);
					for (x = 0; x < PCOLORS_ACROSS; x += ColorReqDivideX) {
						DrawRect (CR_LEFT + PALETTE_LEFT + 1 + x * PCOLOR_WIDTH,
								  CR_TOP  + PALETTE_TOP + 1,
								  1, PCOLOR_HEIGHT * PCOLORS_DOWN);
						if ((x + ColorReqDivideX) * PCOLOR_WIDTH - 1 < PCOLOR_WIDTH * PCOLORS_ACROSS) {
							DrawRect (CR_LEFT + PALETTE_LEFT + 1 + (x + ColorReqDivideX) * PCOLOR_WIDTH - 1,
								  	CR_TOP  + PALETTE_TOP + 1,
								  	1, PCOLOR_HEIGHT * PCOLORS_DOWN);
						}
					}
				}
				if (ColorReqDivideY) {
					short	y;

					SetPenColor (CrColors[CR_WHITE]);
					for (y = 0; y < PCOLORS_DOWN; y += ColorReqDivideY) {
						DrawRect (CR_LEFT + PALETTE_LEFT + 1,
								  CR_TOP  + PALETTE_TOP + 1 + y * PCOLOR_HEIGHT,
								  PCOLOR_WIDTH * PCOLORS_ACROSS, 1);
						if ((y + ColorReqDivideY) * PCOLOR_HEIGHT - 1 < PCOLOR_HEIGHT * PCOLORS_DOWN) {
							DrawRect (CR_LEFT + PALETTE_LEFT + 1,
								  	CR_TOP  + PALETTE_TOP + 1 + (y + ColorReqDivideY) * PCOLOR_HEIGHT - 1,
								  	PCOLOR_WIDTH * PCOLORS_ACROSS, 1);
						}
					}
				}

				DrawPaletteBox ();
				RedrawTitle (PalX2 * 8 + PalY2);
				ComputeNextPalette (GlobalPalette, EditCI, TRUE);
				UpdateRGB (&pcscEdit[PalX2 * 8 + PalY2]);
				UpdateRGBvsHSV ();
				UpdateCycleArrow ();
				UpdateCycles ();
				UpdateCycleInfo ();

				AfterGraphics ();

				lastMouseY = TITLE_HEIGHT+1;	/* KLUDGE: Force call to SetColors() if needed */
				while (!CRdone) {
					ha = CheckHitAreas (CRHitArea, CR_LEFT, CR_TOP);
					if (ha) {
						if (ha->ID == OK_ID) {
							CRdone = TRUE;
							break;
						}
						if (ha->ID == CANCEL_ID) {
							CRabort = TRUE;
							CRdone  = TRUE;
							break;
						}
					}
					else
					{
						MouseInfo	mi;

						ReadMouse (&mi);

						/* If mouse moves into title bar, jam palette */

						if (lastMouseY >= lastMenuHeight && mi.Y < GlobalAreas[0].Height)
						{
							/* If menu bar was not showing, show it */

							AutoShowMenus();
						}

						/* If mouse moves out of title bar, un-jam palette */

						if (lastMouseY < lastMenuHeight && mi.Y >= GlobalAreas[0].Height)
						{
							RestoreColors(MainWindow);

							/* If menu bar should not be showing, hide it */

							if (!ShowTitleSTATE)
							{
								AutoHideMenus();
							}
						}
						lastMouseY = mi.Y;

						if (mi.Buttons & MOUSE_RIGHTBUTTON ||
							(mi.Buttons & MOUSE_LEFTBUTTON && mi.Y < GlobalAreas[0].Height))
//							(mi.Buttons & MOUSE_LEFTBUTTON && mi.Y < MenuBarHeight))
						{
							DoLocalMenus (ColorPopup, NULL, &mi);
						}
					}
					#if QUAL_ALT
					{
						short	 key;
						short	 qual;
						short	 ascii;

						key = GetKeyNoWait ();
						if (key) {
							qual  = (key >> 8);
							ascii = ConvertKeyToAscii (key);
//							if (isalpha (ascii)) {
//								ascii = toupper (ascii);
//							}
							switch (ascii)
							{
							case 13:
								CRdone = TRUE;
								break;
							case 27:
								CRabort = TRUE;
								CRdone  = TRUE;
								break;
							case ' ':
								if (AffectMode != AFFECT_NONE)
								{
									AffectMode = AFFECT_NONE;
									RedrawTitle (-1);
//									ChangeColorFocus (FOCUS_PALETTE);
								}
								ResetSpread ();
								break;
							case 'p':
							case EASCII_HOME:
								if ((ascii == 'p' && (qual & QUAL_ALT)) ||
									(ascii == EASCII_HOME)) {
									GlobalPalette[CrColors[CR_WHITE]].Red    = 192;
									GlobalPalette[CrColors[CR_WHITE]].Green  = 192;
									GlobalPalette[CrColors[CR_WHITE]].Blue   = 192;
									GlobalPalette[CrColors[CR_WHITE]].Flags |= PAL_CHANGED;
									GlobalPalette[CrColors[CR_BLACK]].Red    = 0;
									GlobalPalette[CrColors[CR_BLACK]].Green  = 0;
									GlobalPalette[CrColors[CR_BLACK]].Blue   = 0;
									GlobalPalette[CrColors[CR_BLACK]].Flags |= PAL_CHANGED;
									ComputeColorRanges (GlobalPalette, FALSE);
								}
								break;
#if 1
							case 'b':
								BlendColors();
								break;
							case 'c':
								CopyColors();
								break;
							case 'd':
								DeleteColors();
								break;
							case 'i':
								InsertColors();
								break;
							case 's':
								SwapColors();
								break;
#endif
#if 0
							default:
								{
									MenuItem	*mi;
									MouseInfo	 mouse = { 0, };

									mi = RealCheckMenuKeys (ColorPopup, key, TRUE);
									if (mi) {
										DoLocalMenus (ColorPopup, mi, &mouse);
									}
								}
								break;
#endif
							}
						}
					}
					#endif
				}

				CCCount = 0;

				if (!CRabort)
				{
					if (AffectMode == AFFECT_COPY && CCBuffer)
					{
						// PrintMsg ("Copying Colors");
						AffectColorDest (0, 0, FOCUS_CCBUFFER);
						UpdateXTRA (CCBuffer);

						#if 0
						{
							char	line[80];
							sprintf (line, "Copying Colors %d", CCCount);
							PrintMsg (line);
						}
						#endif
					}
				}

				FixColors ();

dcrcleanup:
				if (sa)		RestoreArea (sa);
				FlushKeyboardBuffer ();

				PopClipValues ();
			}

			if (CRabort)
			{
				CopyColorInfoToColorInfo (cancelci, GlobalColors);
			}
#if UNDO_BUTTON
			else if (EditCI != GlobalColors)
				CopyColorInfoToColorInfo(EditCI, GlobalColors);
#endif
			DeleteColorInfo (cancelci);
			DeleteColorInfo (ciUndo);

			ComputeNextPalette (GlobalPalette, GlobalColors, TRUE);
			UpdateHardwareColors (GlobalPalette, FALSE);

			ResetColorInfo (GlobalColors);
			UpdateGlobalColors ();
			PopGlobalColors();
			UpdateGlobalColors ();
			VBCycle    = vbtemp;
			CyclePause = cptemp;
		}

		RestoreColorsNPointer (MainWindow);

		return TRUE;
	}
} /* ColorSequencer */


typedef struct {
	ChunkHeader		CH1;
	ULONG			ID;
	ChunkHeader		CHBM;
	BitMapHeader	BM;
	ChunkHeader		CHCM;
	UBYTE			CR[256][3];
	ChunkHeader		CRNG;
	CrngChunk		ColorRange;
	ChunkHeader		CHBD;
	UBYTE			Pixels[20*5];
} PBMFile;

#if __MSDOS__
#undef	FlipWord
#undef	FlipLong
#define FlipWord(v)	(((((UWORD)(v)) & 0x00FF) << 8) | ((((UWORD)(v)) & 0xFF00) >> 8))
#define FlipLong(a) ((	((((ULONG)(a)) & 0xFF000000L) >> 24) |	\
						((((ULONG)(a)) & 0x00FF0000L) >>  8) |	\
						((((ULONG)(a)) & 0x0000FF00L) <<  8) |	\
						((((ULONG)(a)) & 0x000000FFL) << 24) ))
#else
#error Need support here
#endif

PBMFile PBM = {
	{
		FORM_CHUNK,
		FlipLong(sizeof (PBMFile) - 8),
	},
	PBM_FORM,
	{
		BMHD_CHUNK,
		FlipLong(sizeof (BitMapHeader)),
	},
	{
		FlipWord(20),
		FlipWord(5),
		0,
		0,
		8,	/* 8 Bit Planes */
		2,
		0,
		0,
		0,
		10,
		11,
		FlipWord(320),
		FlipWord(200),
	},
	{
		CMAP_CHUNK,
		FlipLong (256 * 3),	 /* 256 Colors */
	},
	{
		{ 0x0, 0x0, 0x0, },					/* Change */
	},
	{
		CRNG_CHUNK,
		FlipLong (sizeof (CrngChunk)),
	},
	{
		0,
		0,
		0,
		0,0		/* Change */
	},
	{
		BODY_CHUNK,
		FlipLong (20*5U),
	},
	{
		0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x01,0x01,0x01,0x01,
		0x00,0x01,0x01,0x01,0x00,0x01,0x00,0x00,0x01,0x00,0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x00,0x00,0x00,
		0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x01,0x01,0x00,
		0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x00,0x00,
		0x00,0x00,0x01,0x01,0x00,0x01,0x01,0x01,0x01,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x01,0x01,0x01,0x01,
	},
};


/*********************************************************************
 *
 * fGoodRange
 *
 * PURPOSE
 *		Check to see that the palette requester is up and that the
 *		user has selected an appropriate range. If not, display some
 *		appropriate error message, and return FALSE.
 *
 * INPUT
 *		None.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		FALSE if not good to load/save.
 *
 * HISTORY
 *		07/31/93 Saturday (dcc) - created.
 *
*/
BOOL fGoodRange(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fGoodRange";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CRdone)							/* palette requester inactive? */
	{
		TellUser(NCD, szerrPalNotUp);
		return FALSE;
	}
	if (ColorFocus != FOCUS_PALETTE)	/* color range not selected? */
	{
		TellUser(NCD, szerrNoPalRange);
		return FALSE;
	}
	return TRUE;
} /* fGoodRange */


/*********************************************************************
 *
 * LoadSomePalette
 *
 * SYNOPSIS
 *		LOCAL void LoadSomePalette ()
 *
 * PURPOSE
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
 *		07/31/93 Saturday (dcc) - added test for good palette range.
 *
 * SEE ALSO
 *
*/
LOCAL void LoadSomePalette (short range, short anywhere)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LoadSomePalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		if (!fGoodRange())
			return;
	}

	{
		char	*filename;

		filename = GetFileName (MainWindow, EasyFR[PATH_PALETTE]);
		if (filename)
		{
			ILBMInfo	*ilbm;
			short		 from;
			short		 len;
			short		 to;
			short		 end;
			short		 doit = TRUE;
			short		 err;

			ClearGlobalError();
			ilbm = CreatePalette (filename);
			if (!ilbm)
			{
				TellUser(NCD, GlobalErrMsg);
				ClearGlobalError();
			}
			else
			{
				from = 0;
				len  = ilbm->NumberOfColors;
				to   = 0;

				if (range)
				{
					{
						CrngNode	*cn;

						cn = Head (ilbm->CrngList);
						if (!EndOfList (cn))
						{
							from = cn->Crng.low;
							len  = cn->Crng.high - cn->Crng.low + 1;

							end = from + len - 1;
							if (from > end || from < 0 || from > MAX_HARDWARE_COLOR
								|| end < 0 || end > MAX_HARDWARE_COLOR)
							{
								from = 0;
								len  = ilbm->NumberOfColors;
							}
						}
					}

					do
					{
						err  = FALSE;
#if 1
						doit = GetOneNumber (
								MainWindow,
								"Source Range",
								"First Color",
								&from);

						len = max(len, (PalEnd - PalStart + 1));
						len = min(len, (MAX_COLOR_RANGE - from));
#else
						doit = GetTwoNumbers (
								MainWindow,
								"Source Range",
								"First Color",
								"Num. Colors",
								&from,
								&len);
#endif

						end = from + len - 1;

						if (doit &&
							(from > end || from < 0 || from > MAX_HARDWARE_COLOR
							|| end < 0 || end > MAX_HARDWARE_COLOR))
						{
#if 1
							TellUser (NCD, szerrBadRange);
#else
							TellUser (NCD, szerrBadRange);
#endif
							err = TRUE;
						}
					}
					while (doit && err);

					to = PalStart;

					if (doit)
					{
#if 1
#else
						do
						{
							err  = FALSE;
							doit = GetOneNumber (
									MainWindow,
									"Copy To Color #",
									"Color Number",
									&to);

							if (to < 0 || to > MAX_HARDWARE_COLOR)
							{
								TellUser (NCD, "Color number is out of bounds.");
								err = TRUE;
							}
						}
						while (doit && err);
#endif

						end = (to + len) - 1;
						if (end > MAX_HARDWARE_COLOR)
						{
							len = MAX_COLOR_RANGE - to;
						}
					}
				}

				if (doit)
				{
					#if 0
					if (anywhere)
					{
						//
						// copy to the currently selected thing
						//
					}
					else
					#else
						anywhere = anywhere;	// stop warning
					#endif
					{
						//
						// copy to the main palette
						//
						CS_Color		*cs;
						ColorRegister	*cr;

						cs = &pcscEdit[to];
						cr = &ilbm->Colors[from];
						while (len)
						{
							cs->Red   = cr->red;
							cs->Green = cr->green;
							cs->Blue  = cr->blue;

							ConvertRGBtoHSV (cs);
							AffectPaletteReg (cs, to); 

							cs++;
							cr++;
							len--;
							to++;
						}

						UpdateHardwareColors (GlobalPalette, FALSE);
					}
				}

				DestroyPalette (ilbm);
			}
		}
	}

} /* LoadSomePalette */


/*********************************************************************
 *
 * SaveSomePalette
 *
 * SYNOPSIS
 *		LOCAL void SaveSomePalette ()
 *
 * PURPOSE
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
 *		07/31/93 Saturday (dcc) - added test for good palette range.
 *		08/01/93 Sunday (dcc) - change to save entire palette (not just range).
 *
 * SEE ALSO
 *
*/
LOCAL void SaveSomePalette (short range, short anywhere)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveSomePalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		if (!fGoodRange())
			return;
	}
	{
		char	*filename;

		filename = GetFileName (MainWindow, EasyFR[PATH_PALETTE]);
		if (filename)
		{
			short		 from;
			short		 len;
			short		 doit = TRUE;
#if 1
#else
			short		 end;
			short		 err;
#endif

			from = 0;
			len  = MAX_COLOR_RANGE;

			if (range)
			{
				from = PalStart;
				len  = PalEnd - PalStart + 1;

#if 1
#else
				do
				{
					err  = FALSE;
					doit = GetTwoNumbers (
							MainWindow,
							"Range To Save",
							"First Color",
							"Num. Colors",
							&from,
							&len);

					end = from + len - 1;

					if (doit &&
						(from > end || from < 0 || from > MAX_HARDWARE_COLOR
						|| end < 0 || end > MAX_HARDWARE_COLOR))
					{
						TellUser (NCD, "Range is out of bounds.");
						err = TRUE;
					}
				}
				while (doit && err);
#endif
			}

			if (doit)
			{
				#if 0
				if (anywhere)
				{
					//
					// save the currently selected colors.
					//
				}
				else
				#else
					anywhere = anywhere;	// stop warning
				#endif
				{
					//
					// save from the main palette
					//
					CS_Color	*cs;
					UBYTE		*cr;

					PBM.ColorRange.low  = from;
					PBM.ColorRange.high = from + len - 1;

#if 1
					cs = &pcscEdit[0];
					cr = &PBM.CR[0][0];
					len = MAX_COLOR_RANGE;
#else
					cs = &pcscEdit[from];
					cr = &PBM.CR[from][0];
#endif
					while (len)
					{
						*cr++ = cs->Red;
						*cr++ = cs->Green;
						*cr++ = cs->Blue;

						cs++;
						len--;
					}
				}

				{
					int	fh;

					fh = EIO_WriteOpen (filename);
					if (fh == (-1))
					{
						TellUser (NCD, szerrCantWritePal);
					}
					else
					{
						EIO_Write (fh, &PBM, sizeof (PBM));
						EIO_Close (fh);
					}

				}
			}
		}
	}

} /* SaveSomePalette */


/*********************************************************************
 *
 * LoadPalette
 *
 * SYNOPSIS
 *		void LoadPalette (void)
 *
 * PURPOSE
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
void LoadPalette (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LoadPalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		SetFreqTitle (EasyFR[PATH_PALETTE], "Load Palette");
		LoadSomePalette (FALSE, FALSE);
	}

} /* LoadPalette */


/*********************************************************************
 *
 * SavePalette
 *
 * SYNOPSIS
 *		void SavePalette (void)
 *
 * PURPOSE
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
void SavePalette (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SavePalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetFreqTitle (EasyFR[PATH_PALETTE], "Save Palette");
	SaveSomePalette (FALSE, FALSE);

} /* SavePalette */


/*********************************************************************
 *
 * LoadPaletteRange
 *
 * SYNOPSIS
 *		void LoadPaletteRange (void)
 *
 * PURPOSE
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
void LoadPaletteRange (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LoadPaletteRange";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

		SetFreqTitle (EasyFR[PATH_PALETTE], "Load Palette Range");
		LoadSomePalette (TRUE, TRUE);

} /* LoadPaletteRange */


/*********************************************************************
 *
 * SavePaletteRange
 *
 * SYNOPSIS
 *		void SavePaletteRange (void)
 *
 * PURPOSE
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
void SavePaletteRange (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SavePaletteRange";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetFreqTitle (EasyFR[PATH_PALETTE], "Save Palette Range");
	SaveSomePalette (TRUE, TRUE);

} /* SavePaletteRange */

