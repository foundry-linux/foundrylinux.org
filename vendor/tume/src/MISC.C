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
 * MISC.C
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 09/20/89 
 *   MODIFIED : 10/24/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *
 *
 * HISTORY
 *		09/20/89 (RGM) - Made this header.
 *		04/05/93 (dcc) - use rectplot.h header.
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
*/
#include <echidna/platform.h>

#if __MSDOS__
#if !__MSDOS32X__
#include <dir.h>
#endif
#include <dos.h>
#include <io.h>
#include <string.h>

#include <echidna/grafx.h>
#include "colorseq.h"
#include "euicolor.h"
#include "rectplot.h"
#endif/*__MSDOS__*/

#include "switches.h"
#include "switch2.h"

#include "switch1.h"							// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#if __AMIGAOS__
#include <intuition/preferences.h>

#include <intuition/intuition.h>
#endif/*__AMIGAOS__*/

#include <echidna/eio.h>
#include <echidna/version.h>

#include "events.h"
#include "tuglbl.h"
#include "copyrigh.h"
#include "message.h"
#include "misc.h"								/* Verify function prototypes. */
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray
#include "tilebits.h"

/**************************** C O N S T A N T S ***************************/

#define NAMES	4
#define FSIZE	126

#define ALL				40
#define MAXMAX			256
#define OKMAX			96

#define ROOMNAMEFIELD	12

#if __MSDOS__

/* Mouse black */

#define C00R		(0)
#define C00G		(0)
#define C00B		(0)

/* Mouse white */

#define C15R		(255)
#define C15G		(255)
#define C15B		(255)

/* Menu colors */

#define MENU_BLACK_R	(0)
#define MENU_BLACK_G	(0)
#define MENU_BLACK_B	(0)
#define MENU_WHITE_R	(192)	//63
#define MENU_WHITE_G	(192)	//63
#define MENU_WHITE_B	(192)	//63
#define EUI_ORANGE_R	(240)	//63
#define EUI_ORANGE_G	(240)	//54
#define EUI_ORANGE_B	(240)	//42
#define EUI_BLUE_R	(64)	//0
#define EUI_BLUE_G	(64)	//0
#define EUI_BLUE_B	(64)	//45

#endif/*__MSDOS__*/

/******************************** T Y P E S *******************************/


/****************************** E X T E R N S *****************************/

#if __MSDOS__/*dcc*/
extern void DrawTitleBar (void);

extern short ExitProgram;

extern int MouseY;
#endif/*__MSDOS__*/

/****************************** G L O B A L S *****************************/

short CycleColorsSTATE	= FALSE;
#if __AMIGAOS__
short ShowTitleSTATE	= TRUE;
#elif __MSDOS__
short ShowTitleSTATE	= FALSE;

short JamPaletteSTATE	= FALSE;
#endif/*__AMIGAOS__/__MSDOS__*/

short ScrollLockSTATE	= FALSE;
short SwankyModeSTATE	= FALSE;
short LockRoomSTATE	= FALSE;

short InvisiLayerSTATE	= FALSE;
short LockLayerSTATE	= FALSE;

short SwankyMode		= FALSE;

int		LastRealTBar;

short wShiftXUnit	= 1;
short wShiftYUnit	= 1;

/*************************************************************/
/* DefaultColorInfo is initialized during Initialization     */
/* using 'ShockColors', and is used to hold 'default colors' */
/* thereafter                                                */

ColorInfo	DefaultColorInfo	= {0, };
//ColorInfo	*GlobalColors		= &DefaultColorInfo;

BOOL		Colorz = FALSE;

/******************************* L O C A L S ******************************/

static const	int		trans_table [3] = {-1, 1, 0};
#if __AMIGAOS__
static			char	StatusMessage[40];
#endif/*__AMIGAOS__*/

static			char	*Blanks		=
		"                                        ";
#if __AMIGAOS__
static			char	StrBuff	[ALL];
#elif __MSDOS__/*dcc*/
char	StrBuff	[ALL];
#endif/*__AMIGAOS__/__MSDOS__*/

static char FixedPath[256];

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

#if __AMIGAOS__
#define INPUT_SLASH	'\\'
#define OUTPUT_SLASH	'/'
#elif __MSDOS__
#define INPUT_SLASH	'/'
#define OUTPUT_SLASH	'\\'
#endif/*__AMIGAOS__/__MSDOS__*/

/* FixPath - fix input path so separator conform to local customs. */

char *FixPath(char *name)
{
	char *pdst = FixedPath;

	for ( ; *name != 0; name++)
	{
		if (*name != INPUT_SLASH)
			*pdst++ = *name;
		else
			*pdst++ = OUTPUT_SLASH;
	}
	*pdst = 0;

	return FixedPath;
}


#if __AMIGAOS__
#define PMessage Message
#elif __MSDOS__

/*********************************************************************
 *
 * PMessage
 *
 * SYNOPSIS
 *		short PMessage (Window *window, char *title, char *message, char *options)
 *
 * PURPOSE
 *		Set the palette, show the message, restore the palette.
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
short PMessage (Window *window, char *title, char *message, char *options)
{

	short w;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PMessage";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetColorsNPointer (MainWindow);
	w = Message (window, title, message, options);
	RestoreColorsNPointer (MainWindow);
	return w;

} /* PMessage */

#endif/*__AMIGAOS__/__MSDOS__*/


#if __MSDOS__
/*********************************************************************
 *
 * SetColorReg
 *
 * SYNOPSIS
 *		static void SetColorReg(int i, UBYTE ubRed, UBYTE ubGreen, UBYTE ubBlue)
 *
 * PURPOSE
 *		Set colors in GlobalPalette[i]. After changing all colors,
 *		you should call ComputeColorRanges().
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
static void SetColorReg(int i, UBYTE ubRed, UBYTE ubGreen, UBYTE ubBlue)
{

	UWORD	 temp;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetColorReg";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	temp = (UWORD) ubRed   * (UWORD) ColorRange[0] / MAX_COLOR_RANGE;
	GlobalPalette[i].Red   = temp * MAX_HARDWARE_COLOR / (UWORD) ColorMax[0];
	temp = (UWORD) ubGreen * (UWORD) ColorRange[1] / MAX_COLOR_RANGE;
	GlobalPalette[i].Green = temp * MAX_HARDWARE_COLOR / (UWORD) ColorMax[1];
	temp = (UWORD) ubBlue  * (UWORD) ColorRange[2] / MAX_COLOR_RANGE;
	GlobalPalette[i].Blue  = temp * MAX_HARDWARE_COLOR / (UWORD) ColorMax[2];
	GlobalPalette[i].Flags |= PAL_CHANGED;

} /* SetColorReg */
#endif/*__MSDOS__*/


#if __AMIGAOS__
/**************************************************************************
 *
 * SetColorRangeForReals
 *
 * SYNOPSIS
 *		void SetColorRangeForReals (
 *			struct	ViewPort	*vp,
 *			int					c1,
 *			int					c2,
 *			UWORD				*colors
 *		)
 *
 * PURPOSE
 *   Sets colors 'c1' thru 'c2' in ViewPort 'vp' to the colors values
 *   pointed to by 'colors' and updates GlobalColors array.
 *
 * USAGE
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
void SetColorRangeForReals (
	struct	ViewPort	*vp,
	int					c1,
	int					c2,
	UWORD				*colors
)
{

	Color			*cp1 = &GlobalColors->Colors[c1];
	Color			*cp2 = (Color *)colors;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetColorRangeForReals";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (; c1<=c2; c1++) {
		*cp1++ = *cp2++;
	}
	LoadRGB4 (vp, (short *) GlobalColors->Colors, 32);

} /* SetColorRangeForReals */
#elif __MSDOS__
/**************************************************************************
 *
 * SetDisplayPalette
 *
 * SYNOPSIS
 *		void SetDisplayPalette (CS_Color *pcsc, WORD cscCnt)
 *
 * PURPOSE
 *   Sets colors 0 thru <GlobalNumberColors> in MCGA display
 *   to the colors values in GlobalColors array.
 *
*/
void SetDisplayPalette(CS_Color *pcsc, WORD cscCnt)
{

	WORD i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetDisplayPalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (i = 0; i < cscCnt; i++)
	{
		SetColorReg(i, pcsc->Red, pcsc->Green, pcsc->Blue);
		pcsc++;
	}

	if ((MouseY < 0 && ShowTitleSTATE) ||	/* Mouse above menu bar? */
		JamPaletteSTATE)
		SetColors(MainWindow);

	ComputeColorRanges(GlobalPalette, FALSE);
}
#endif/*__AMIGAOS__/__MSDOS__*/


/**************************************************************************
 *
 * ToggleCycleColors
 *
 * SYNOPSIS
 *		int ToggleCycleColors (void)
 *
 * PURPOSE
 *
 *
 * USAGE
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
int ToggleCycleColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleCycleColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CycleColorsSTATE) {
		ResetColorInfo (GlobalColors);
		VBCycle = TRUE;
	} else {
		VBCycle = FALSE;

#if __AMIGAOS__
		/* SetRGB4 (MainVp, GlobalColors->Colors, 32); */

		LoadRGB4 (MainVp, (short *) GlobalColors->Colors, 32);
#endif/*__AMIGAOS__*/
	}
	GlobalColors->CycleFlag = CycleColorsSTATE;
	UpdateGlobalColors ();

	return 1;

} /* ToggleCycleColors */


#if __AMIGAOS__
/**************************************************************************
 *
 * SetColorRange
 *
 * SYNOPSIS
 *		void SetColorRange (
 *			struct	ViewPort	*vp,
 *			int					c1,
 *			int					c2,
 *			UWORD				*colors
 *		)
 *
 * PURPOSE
 *   Sets colors 'c1' thru 'c2' in ViewPort 'vp' to the colors values
 *   pointed to by 'colors'
 *
 * USAGE
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
void SetColorRange (
	struct	ViewPort	*vp,
	int					c1,
	int					c2,
	UWORD				*colors
)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetColorRange";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (; c1<=c2; c1++) {
		SetRGB4 (vp, (ULONG)c1,
			(ULONG)(*colors & 0x0F00) >> 8,
			(ULONG)(*colors & 0x00F0) >> 4,
			(ULONG)(*colors & 0x000F));
		colors++;
	}
} /* SetColorRange */
#endif/*__AMIGAOS__*/


/**************************************************************************
 *
 * SetColors
 *
 * SYNOPSIS
 *		void SetColors (struct Window *CWindow)
 *
 * PURPOSE
 *   Sets color 0-3 and 29-31 to something viewable and suitable for
 *   interface images like menus, requesters...
 *
 * USAGE
 *   win = pointer to window to have colors effected.
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
void SetColors (struct Window *CWindow)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	if (CWindow == MainWindow) {
		CyclePause = TRUE;

		SetColorRange (&CWindow->WScreen->ViewPort,
			28, 31, &(UserPrefs.color0));

		SetColorRange (&CWindow->WScreen->ViewPort,
			17, 19, &(UserPrefs.color17));

		SetColorRange (&CWindow->WScreen->ViewPort,
			0,  3, &(UserPrefs.color0));
	}
	return;
#elif __MSDOS__

	CWindow = CWindow;				// disable CWindow warning

	CyclePause = TRUE;

#if 0
	/* Mouse black & mouse white */

	SetColorReg ( 0, C00R, C00G, C00B);
	SetColorReg (15, C15R, C15G, C15B);
#endif

	/* Menu colors */

	SetColorReg (MENU_BLACK, MENU_BLACK_R, MENU_BLACK_G, MENU_BLACK_B);
	SetColorReg (MENU_WHITE, MENU_WHITE_R, MENU_WHITE_G, MENU_WHITE_B);
	SetColorReg (EUI_ORANGE, EUI_ORANGE_R, EUI_ORANGE_G, EUI_ORANGE_B);
	SetColorReg (EUI_BLUE,	  EUI_BLUE_R,	 EUI_BLUE_G,	EUI_BLUE_B);

	ComputeColorRanges(GlobalPalette, FALSE);
#endif/*__AMIGAOS__/__MSDOS__*/
} /* SetColors */


/*********************************************************************
 *
 * SetColorsNPointer
 *
 * SYNOPSIS
 *		void SetColorsNPointer (struct Window *CWindow)
 *
 * PURPOSE
 *   Sets color 0-3 and 29-31 to something viewable and suitable for
 *   interface images like menus, requesters...
 *   Also set the mouse to a normal pointer.
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
void SetColorsNPointer (struct Window *CWindow)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetColorsNPointer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetColors(CWindow);
#if __MSDOS__
	DCC_TempSetPointer(BPI_NORMAL_POINTER);
#endif/*__MSDOS__*/

} /* SetColorsNPointer */


#if __AMIGAOS__
/**************************************************************************
 *
 * ShockColors
 *
 * SYNOPSIS
 *		void ShockColors (struct Window *CWindow)
 *
 * PURPOSE
 *		Change the colors in the window and the global color array to the
 *		AMIGA WORKBENCH preferences colors.
 *
 *		Uses SetColorForReals and a copy of the PREFERENCES structure.
 *
 * USAGE
 *		ShockColors (WINDOW);
 *
 * INPUT
 *		WINDOW	: A pointer to an AMIGA window structure.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void ShockColors (struct Window *CWindow)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShockColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CWindow == MainWindow) {
		CyclePause = TRUE;
		SetColorRangeForReals (&CWindow->WScreen->ViewPort,
			28, 31, &(UserPrefs.color17));

		SetColorRangeForReals (&CWindow->WScreen->ViewPort,
			17, 19, &(UserPrefs.color17));

		SetColorRangeForReals (&CWindow->WScreen->ViewPort,
			0,  3, &(UserPrefs.color0));
	}
	return;

} /* ShockColors */
#elif __MSDOS__
/**************************************************************************
 *
 * InitMenuColors
 *
 * SYNOPSIS
 *		void InitMenuColors (ColorInfo *pColorInfo)
 *
 * PURPOSE
 *		Initialize pColorInfo with colors to show menus.
 *
 * USAGE
 *		InitMenuColors (ColorInfo);
 *
 * INPUT
 *		Colorinfo	: A pointer to a color info structure.
 *
 * RETURN VALUE
 *		NONE.
 *
*/
void InitMenuColors (ColorInfo *pColorInfo)
{

	CS_Color	 rgcsc[MAX_CI_COLORS];

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitMenuColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if 0
{
	int i;

	for (i = 0; i < MAXCOLORREG; i++)
	{
		rgcsc[i].Red = i;
		rgcsc[i].Green = i;
		rgcsc[i].Blue = i;
		rgcsc[i].Hue = 0;
		ConvertRGBtoHSV (&rgcsc[i]);
	}
}
#endif
{
	short	i;
	short	j;
	short	c;

	for (j = 0; j < 16; j++) {
		for (i = 0; i < 16; i++) {
			c = j * 16 + i;
			rgcsc[c].Hue        = j ? ((j - 1) * 24) : 0;
			rgcsc[c].Saturation = j ? (i * 8 + 64) : 0;
			rgcsc[c].Value      = j ? (i * 12 + 64) : (i * 17);
			ConvertHSVtoRGB (&rgcsc[c]);
		}
	}
}
#if 0
	rgcsc[0].Red = C00R;
	rgcsc[0].Green = C00G;
	rgcsc[0].Blue = C00B;
	rgcsc[0].Hue = 0;
	ConvertRGBtoHSV (&rgcsc[0]);

	/* Mouse white color */

	rgcsc[15].Red = C15R;
	rgcsc[15].Green = C15G;
	rgcsc[15].Blue = C15B;
	rgcsc[15].Hue = 0;
	ConvertRGBtoHSV (&rgcsc[15]);
#endif

	/* Menu colors */

	rgcsc[MENU_BLACK].Red = MENU_BLACK_R;
	rgcsc[MENU_BLACK].Green = MENU_BLACK_G;
	rgcsc[MENU_BLACK].Blue = MENU_BLACK_B;
	rgcsc[MENU_BLACK].Hue = 0;
	ConvertRGBtoHSV (&rgcsc[MENU_BLACK]);

	rgcsc[MENU_WHITE].Red = MENU_WHITE_R;
	rgcsc[MENU_WHITE].Green = MENU_WHITE_G;
	rgcsc[MENU_WHITE].Blue = MENU_WHITE_B;
	rgcsc[MENU_WHITE].Hue = 0;
	ConvertRGBtoHSV (&rgcsc[MENU_WHITE]);

	rgcsc[EUI_ORANGE].Red = EUI_ORANGE_R;
	rgcsc[EUI_ORANGE].Green = EUI_ORANGE_G;
	rgcsc[EUI_ORANGE].Blue = EUI_ORANGE_B;
	rgcsc[EUI_ORANGE].Hue = 0;
	ConvertRGBtoHSV (&rgcsc[EUI_ORANGE]);

	rgcsc[EUI_BLUE].Red = EUI_BLUE_R;
	rgcsc[EUI_BLUE].Green = EUI_BLUE_G;
	rgcsc[EUI_BLUE].Blue = EUI_BLUE_B;
	rgcsc[EUI_BLUE].Hue = 0;
	ConvertRGBtoHSV (&rgcsc[EUI_BLUE]);

	WriteXTRA(rgcsc, MAX_CI_COLORS*sizeof(CS_Color), pColorInfo->pxtColors, 0);

	pColorInfo->NumColors = MAXCOLORREG;
} /* InitMenuColors */
#endif/*__AMIGAOS__/__MSDOS__*/


/**************************************************************************
 *
 * RestoreColors
 *
 * SYNOPSIS
 *		void RestoreColors (struct Window *CWindow)
 *
 * PURPOSE
 *		restores the colors of the window to their previous values 
 *		(before SetColors was called)
 *
 * USAGE
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
void RestoreColors (struct Window *CWindow)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RestoreColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	if ((! Colorz) && (CWindow == MainWindow)) {
			/* was 28 - 31 */
		CyclePause = FALSE;
		SetColorRange (&CWindow->WScreen->ViewPort,
			28, 31, (UWORD *) &GlobalColors->Colors[28]);

		SetColorRange (&CWindow->WScreen->ViewPort,
			17, 19, (UWORD *) &GlobalColors->Colors[17]);

		SetColorRange (&CWindow->WScreen->ViewPort,
			0,  3, (UWORD *) &GlobalColors->Colors[0]);
	}
#elif __MSDOS__
	CWindow = CWindow;				// disable CWindow warning

	if (!Colorz && !JamPaletteSTATE)
	{
		CS_Color *pcsc = ActivateXTRA(GlobalColors->pxtColors);

		CyclePause = FALSE;

#if 0
		/* Mouse black & mouse white */

		SetColorReg (0,  pcsc[0].Red,
					  pcsc[0].Green,
					  pcsc[0].Blue);

		SetColorReg (15, pcsc[15].Red,
					  pcsc[15].Green,
					  pcsc[15].Blue);
#endif

		/* Menu colors */

		SetColorReg (MENU_BLACK, pcsc[MENU_BLACK].Red,
							pcsc[MENU_BLACK].Green,
							pcsc[MENU_BLACK].Blue);
		SetColorReg (MENU_WHITE, pcsc[MENU_WHITE].Red,
							pcsc[MENU_WHITE].Green,
							pcsc[MENU_WHITE].Blue);
		SetColorReg (EUI_ORANGE, pcsc[EUI_ORANGE].Red,
							pcsc[EUI_ORANGE].Green,
							pcsc[EUI_ORANGE].Blue);
		SetColorReg (EUI_BLUE,	pcsc[EUI_BLUE].Red,
							pcsc[EUI_BLUE].Green,
							pcsc[EUI_BLUE].Blue);

		UpdateXTRA(GlobalColors->pxtColors);

		ComputeColorRanges(GlobalPalette, FALSE);
	}
#endif/*__AMIGAOS__/__MSDOS__*/
} /* RestoreColors */


/*********************************************************************
 *
 * RestoreColorsNPointer
 *
 * SYNOPSIS
 *		void RestoreColorsNPointer (struct Window *CWindow)
 *
 * PURPOSE
 *		restores the colors of the window to their previous values 
 *		(before SetColors was called)
 *		Also restore mouse pointer to previous pointer.
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
void RestoreColorsNPointer (struct Window *CWindow)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RestoreColorsNPointer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	RestoreColors(CWindow);
#if __MSDOS__
	if (!Colorz)
		DCC_SetPointer(GlobalPointer);
#endif/*__MSDOS__*/

} /* RestoreColorsNPointer */


/**************************************************************************
 *
 * SetTheColors
 *
 * SYNOPSIS
 *		int SetTheColors (void)
 *
 * PURPOSE
 *   Allow the user to set all the colors.
 *
 * USAGE
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
int SetTheColors (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetTheColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (UseEditPaletteSTATE &&
		GlobalRoomWindow && GlobalRoomWindow->CurrentRoom &&
		GlobalRoomWindow->CurrentRoom->Room &&
		!IsGlobalColorsCopyOf(GlobalRoomWindow->CurrentRoom->Room->R_ColorInfo))
	{
#if __AMIGAOS__
		ColorReq (MainWindow, GlobalRoomWindow->CurrentRoom->Room->R_ColorInfo);
#elif __MSDOS__
		ColorSequencer (MainWindow, GlobalRoomWindow->CurrentRoom->Room->R_ColorInfo);
#endif/*__AMIGAOS__/__MSDOS__*/

#if __AMIGAOS__
		SetColorRangeForReals (MainVp, 0, MAXCOLORREG - 1, (void *) GlobalColors->Colors);
#elif __MSDOS__
		SetDisplayPalette (ActivateXTRA(GlobalColors->pxtColors), GlobalColors->NumColors);
		ReleaseXTRA(GlobalColors->pxtColors);
#endif/*__AMIGAOS__/__MSDOS__*/
	}
	else
	{
#if __AMIGAOS__
		ColorReq (MainWindow, GlobalColors);
#elif __MSDOS__
		ColorSequencer (MainWindow, GlobalColors);
#endif/*__AMIGAOS__/__MSDOS__*/
	}
#if __MSDOS__
	if	(JamPaletteSTATE)
	{
		SetColors(MainWindow);
	}
#endif/*__MSDOS__*/

	return TRUE;

} /* SetTheColors */


#if __AMIGAOS__
/**************************************************************************
 *
 * RestorePointer
 *
 * SYNOPSIS
 *		void RestorePointer (void)
 *
 * PURPOSE
 *   Should put normal preferences pointer back so that user can use
 *   interface.
 *
 * USAGE
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
void RestorePointer (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RestorePointer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (MainWindow) {
		RGM_SetPointer (GlobalPointer, MainWindow);
	}
	return;

} /* RestorePointer */


/**************************************************************************
 *
 * RemovePointer
 *
 * SYNOPSIS
 *		void RemovePointer (void)
 *
 * PURPOSE
 *		Should set pointer to current mode of application.??!?!?!
 *
 * USAGE
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
void RemovePointer (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RemovePointer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return;
} /* RemovePointer */
#endif/*__AMIGAOS__*/


/**************************************************************************
 *
 * AreYouSure
 *
 * SYNOPSIS
 *		int AreYouSure(char *mess)
 *
 * PURPOSE
 *		Presents the message 'message' to the user and asks the question
 *		"Are you sure?"  Returns TRUE if the user is sure.
 *
 * USAGE
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
int AreYouSure(char *mess)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AreYouSure";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	InitKeyList();
	AddKeyOption(27, 1);			/* ESCAPE returns NO option */
	AddKeyOption(13, 2);			/* ENTER returns YES option */
	return (PMessage (MainWindow, "Are you sure?", mess, " NO | YES ") - 1);

} /* AreYouSure */


/**************************************************************************
 *
 * DoThis2
 *
 * SYNOPSIS
 *		int DoThis2 (
 *			char *title,
 *			char *mes1,
 *			char *mes2
 *		)
 *
 * PURPOSE
 *		Presents the messages 'mes1' + 'mes2' to the user and displays the
 *		header "title".  Returns TRUE if the user is sure.
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns either YES or NO, or ERROR if OOM.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int DoThis2 (
	char *title,
	char *mes1,
	char *mes2
)
{
	char *message;
	int	result;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DoThis2";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ( (message = MEM_malloc (strlen (mes1) + strlen (mes2) + 2)) == NULL) {
/**/		goto ABORT;
	}

	strcpy (message, mes1);
	strcat (message, mes2);

	InitKeyList();
	AddKeyOption(27, 1);			/* ESCAPE returns NO option */
	AddKeyOption(13, 2);			/* ENTER returns YES option */
	result = (PMessage (MainWindow, title, message, " NO | YES ") - 1);
	
	result = result ? YES : NO; 

	SAFEFREE (message);
	return (result);
/*----------------------------------------------------------------------*/
ABORT:
	SAFEFREE (message);
	return (ERROR);

} /* DoThis2 */


/**************************************************************************
 *
 * IsItThisOne
 *
 * SYNOPSIS
 *		int IsItThisOne (
 *			char *title,
 *			char *message
 *		)
 *
 * PURPOSE
 *		Presents the message 'message' with the title 'title' to the user.
 *		Displays the buttons [yes] [no] [abort].
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns NO, YES, or ABORT.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int IsItThisOne (
	char *title,
	char *message
)
{
	int result;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "IsItThisOne";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	InitKeyList();
	AddKeyOption(27, 1);			/* ESCAPE returns CANCEL option */
	AddKeyOption(13, 2);			/* ENTER returns YES option */
	result = trans_table [PMessage (MainWindow, title,
						message, " CANCEL | YES | NO ") - 1];

	return (result);

} /* IsItThisOne */


/**************************************************************************
 *
 * SYSMESS
 *
 * SYNOPSIS
 *		void SYSMESS (char *mess)
 *
 * PURPOSE
 *		Displays 'message' to the user and waits for the user to acknowledge
 *		the message has been read.
 *
 * USAGE
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
void SYSMESS (char *mess)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SYSMESS";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __MSDOS__
	SetColorsNPointer(MainWindow);
#endif/*__MSDOS__*/
	InitKeyList();
	AddKeyOption(13, 1);			/* ENTER returns OK option */
	if (!PMessage (MainWindow, ">> WARNING <<", mess, " OK ")) {
		printf ("ERROR: %s\n", mess);
	}
#if __MSDOS__
	RestoreColorsNPointer(MainWindow);
#endif/*__MSDOS__*/
	return;

} /* SYSMESS */


/**************************************************************************
 *
 * TellUser
 *
 * SYNOPSIS
 *		short TellUser (
 *			char	*title,
 *			char	*mess
 *		)
 *
 * PURPOSE
 *   Displays 'message' to the user and waits for the user to acknowledge
 *   the message has been read.
 *
 * USAGE
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
short TellUser (
	char	*title,
	char	*mess
)
{

	short w;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TellUser";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __MSDOS__
	SetColorsNPointer(MainWindow);
#endif/*__MSDOS__*/
	AddKeyOption(13, 1);			/* ENTER returns option 1 */
	w = PMessage (MainWindow, title, mess, " OK ");
#if __MSDOS__
	RestoreColorsNPointer(MainWindow);
#endif/*__MSDOS__*/

	return w;

} /* TellUser */


/**************************************************************************
 *
 * NotYet
 *
 * SYNOPSIS
 *		void NotYet(void)
 *
 * PURPOSE
 *		Displays the message "Not Yet Implemented" to the user.  Used as
 *		a stub for routines not yet implemented.
 *
 * USAGE
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
void NotYet(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "NotYet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SYSMESS ("Not yet implemented.");

} /* NotYet */


/*------------------------------------------------------------------------*/


/**************************************************************************
 *
 * QuitAndExit
 *
 * SYNOPSIS
 *		int QuitAndExit (void)
 *
 * PURPOSE
 *		
 *
 * USAGE
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
int QuitAndExit (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "QuitAndExit";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	return (!AreYouSure ("Do you want to Quit and Exit?"));
#elif __MSDOS__
	SetColorsNPointer(MainWindow);
	ExitProgram = AreYouSure("Do you want to Quit and Exit?");
	RestoreColorsNPointer(MainWindow);
	return TRUE;
#endif/*__AMIGAOS__/__MSDOS__*/

} /* QuitAndExit */


/**************************************************************************
 *
 * About
 *
 * SYNOPSIS
 *		int About (void)
 *
 * PURPOSE
 *
 *
 * USAGE
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
int About (void)
{

#if __MSDOS__
	char	cAbout[256];
#endif/*__MSDOS__*/

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "About";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	PMessage (MainWindow,
		"About tUME",
		"tUME\t\nthe Universal Map Editor\t\nby R.G. Marquez\t\n",
		" Pretty Swanky "
		);
	return 1;
#elif __MSDOS__

	{
#if dvpSNASM
		char szDvp[] = "s";
#elif (dvpCLD == 2)
		char szDvp[] = "C";
#elif dvpCLD
		char szDvp[] = "c";
#elif dvpPARALLEL
		char szDvp[] = "p";
#elif dvpPsyQ
		char szDvp[] = "Q";
#elif dvpNONE
		char szDvp[] = "";
#endif

		sprintf (cAbout,
				"the Universal Map Editor\n"
				"v%s%s by Dan Chang,\n"
				"R.G. Marquez & Gregg Tavares\n\n"
				"%s\n"
				"%ldK",
				Version, szDvp, Copyright,
				AvailXTRA(xmtSTD));
		{
			LONG l;
			char sz[64];

			l = AvailXTRA(xmtEMS);
			if (l >= 0)
			{
				sprintf (sz, ", %ldK EMS", l);
				strcat (cAbout, sz);
			}

			l = AvailXTRA(xmtXMS);
			if (l >= 0)
			{
				sprintf (sz, ", %ldK XMS", l);
				strcat (cAbout, sz);
			}
		}
		strcat (cAbout, " free.");

		InitKeyList();
		AddKeyOption(13, 1);		/* ENTER returns More option */
#if fDoSaveRooms
		PMessage (MainWindow, "About tUME", cAbout, "More");

		sprintf (cAbout,
				"tUME is subject to the Mozilla Public License Version 1.1 "
				"(the \"License\"). You may obtain a copy of the License at "
				"http://www.mozilla.org/MPL/ "
				);

		InitKeyList();
		AddKeyOption(13, 1);		/* ENTER returns OK option */
		PMessage (MainWindow, "About tUME", cAbout, "More");

		sprintf (cAbout,
				"Software distributed under the License is distributed on an \"AS "
				"IS\" basis, WITHOUT WARRANTY OF ANY KIND, either express or "
				"implied. See the License for the specific language governing "
				"rights and limitations under the License."
				);

		InitKeyList();
		AddKeyOption(13, 1);		/* ENTER returns OK option */
		PMessage (MainWindow, "NOTICE", cAbout, " OK ");
#else	// !fDoSaveRooms
		PMessage (MainWindow, "About tUME", cAbout, " OK ");
#endif // !fDoSaveRooms

		return 1;
	}
#endif/*__AMIGAOS__/__MSDOS__*/

} /* About */


#if __AMIGAOS__
/**************************************************************************
 *
 * ToggleTitleBar
 *
 * SYNOPSIS
 *		int ToggleTitleBar (int *tstate)
 *
 * PURPOSE
 *
 *
 * USAGE
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
int ToggleTitleBar (int *tstate)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleTitleBar";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowTitle (MainScreen, (*tstate != FALSE));
	TopAdd = ShowTitleSTATE ? 11 : 0;
	ShowRoom (GlobalRoomWindow);
	return 1;

} /* ToggleTitleBar */
#endif/*__AMIGAOS__*/


/**************************************************************************
 *
 * ToggleSwankyMode
 *
 * SYNOPSIS
 *		int ToggleSwankyMode (void)
 *
 * PURPOSE
 *
 *
 * USAGE
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
int ToggleSwankyMode (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleSwankyMode";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return TRUE;

} /* ToggleSwankyMode */


#if PLOTARRAY
/**************************************************************************
 *
 * FirstTilesetInLayers
 *
 * SYNOPSIS
 *		TileSetType *FirstTilesetInLayers (
 *			LayerType	*layer,
 *			long		layer_size
 *		)
 *
 * PURPOSE
 *		Return a pointer to the first non-zero tile in a group of layers.
 *
 * USAGE
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
 *	01/13/91 Sunday (RGM) - Created.
 *
 *
 * SEE ALSO
 *
*/
static TileSetType *FirstTilesetInLayers (
	LayerType	*layer,
	long		layer_size
)
{

	PlotType	*plot, *maxplot;
	TileSetType	*tileset = NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FirstTilesetInLayers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (! IsEOList (layer)) {
		plot	= layer->Plot;
		maxplot	= plot;
		maxplot	+= layer_size;

		for ( ; plot < maxplot; plot ++) {
			if (plot->TileSet_ID) break;
		}

		if (plot < maxplot) {
			tileset	= TILESET_AT_PLOT (plot);
		}
		else {
			/* TellUser (NoCanDo, "No plots to find tileset from!!"); */
		}
	}

	return (tileset);

} /* FirstTilesetInLayers */
#else
/**************************************************************************
 *
 * FirstTilesetInLayers
 *
 * SYNOPSIS
 *		TileSetType *FirstTilesetInLayers (
 *			LayerType	*play
 *		)
 *
 * PURPOSE
 *		Return a pointer to the first non-zero tile in a group of layers.
 *
 * USAGE
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
 *		01/13/91 (RGM) - Created.
 *		08/19/94 (dcc) - use play->ctilx instead of play->rgrgplt.PlotWidth
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
static TileSetType *FirstTilesetInLayers (LayerType *play)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FirstTilesetInLayers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (! IsEOList (play))
	{
		int ixt, iyt;

		for (iyt = 0; iyt < play->ctily; iyt++)
		{
			PlotType	*pplt;

#if UseSparseArray
			if (!IsSparse(play))
				pplt = ActivatePlotRowatWin(play->p.rgrgplt, iyt, winDst);
#else // !UseSparseArray
			pplt = ActivatePlotRowatWin(play->rgrgplt, iyt, winDst);
#endif // !UseSparseArray

			for (ixt = 0; ixt < play->ctilx; ixt++)
			{
#if UseSparseArray
				if (IsSparse(play))
					pplt = GetSparsePlotXY(play->p.pspa, ixt, iyt);

#endif // UseSparseArray
				if (pplt[ixt].Tile_ID)
				{
					TileSetType *ptst = TILESET_AT_PLOT (&pplt[ixt]);
#if UseSparseArray
					if (!IsSparse(play))
						ReleasePlotRow(play->p.rgrgplt, iyt);
#else // !UseSparseArray
					ReleasePlotRow(play->rgrgplt, iyt);
#endif // !UseSparseArray
					return ptst;
				}
			}
#if UseSparseArray
			if (!IsSparse(play))
				ReleasePlotRow(play->p.rgrgplt, iyt);
#else // !UseSparseArray
			ReleasePlotRow(play->rgrgplt, iyt);
#endif // !UseSparseArray
		}
	}

	return NULL;

} /* FirstTilesetInLayers */
#endif


/**************************************************************************
 *
 * FindFirstTileSet
 *
 * SYNOPSIS
 *		TileSetType *FindFirstTileSet (RoomType	*room)
 *
 * PURPOSE
 *		To return a pointer to the first tileset in a source room.
 *
 * USAGE
 *		foundtileset = FindFirstTileSet (SOURCE_ROOM);
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
 *		11/10/89 Friday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
TileSetType *FindFirstTileSet (RoomType	*room)
{

	TileSetType		*tileset	= NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindFirstTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (room) {
		if (IsComposite(room)) {
			tileset	= room->ptsComposite;
		}
		else if (IsSource(room)) {
#if PLOTARRAY
			LayerType	*layer;
			long		plots;

			layer = Head (&room->Layers);
			plots = (layer->ctilx) * (layer->ctily);

			tileset	= FirstTilesetInLayers (layer, plots);
#else
			tileset	= FirstTilesetInLayers (Head(&room->Layers));
#endif
		}
	}

	return (tileset);

} /* FindFirstTileSet */


/**************************************************************************
 *
 * FindFirstSelect
 *
 * SYNOPSIS
 *		TileSetType *FindFirstSelect (BlockCopyType	*block)
 *
 * PURPOSE
 *		To return a pointer to the first tileset in <block>.
 *
 *
 * USAGE
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
TileSetType *FindFirstSelect (BlockCopyType	*block)
{
	RoomType		*room;
	TileSetType		*tileset = NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindFirstSelect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = block->SourceRoom;

	if (room) {
		if (IsComposite(room)) {
			tileset	= room->ptsComposite;
		}
//		else if (IsSource(room)) {
		else {
#if PLOTARRAY
			LayerType	*layer;
			long		plots;

			layer = Head (&block->Layers);
			plots = (layer->ctilx) * (layer->ctily);

			tileset	= FirstTilesetInLayers (layer, plots);
#else
			tileset	= FirstTilesetInLayers (Head(&block->Layers));
#endif
		}
	}

	return (tileset);

} /* FindFirstSelect */


#if 0
/**************************************************************************
 *
 * FoundDir
 *
 * SYNOPSIS
 *		BOOL FoundDir (char *dirname)
 *
 * PURPOSE
 *		Check to see if a directory exists.
 *
 *		Uses AMIGA SPECIFIC file locks to check for directory.
 *
 * USAGE
 *		ok = FoundDir (DIRECTORY_NAME);
 *
 * INPUT
 *		DIRECTORY_NAME	: A string pointer to a complete path name.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns TRUE if directory DIRECTORY_NAME exists, FALSE otherwise.
 *
 * HISTORY
 *		11/06/89 Monday - Created out of FoundFile. (RGM)
 *
 * SEE ALSO
 *		FoundFile ().
 *
*/
BOOL FoundDir (char *dirname)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FoundDir";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
{

	struct FileLock	*filelock;

	filelock = (struct FileLock	*) Lock (dirname, ACCESS_READ);
	if (filelock)  UnLock ((BPTR) filelock);

	return ((BOOL) (filelock != NULL));
}
#elif __MSDOS__/*dcc*/
#if __TURBOC__
{
	int attrib;
	int len;
	char szName[256];

	len = strlen(dirname);
	strcpy(szName, dirname);
	if (szName[len-1] == '\\')
		szName[len-1] = '\0';

	if ((attrib = _chmod(szName, 0)) == -1)
		return FALSE;
	return (attrib & FA_DIREC) ? TRUE : FALSE;
}
#elif __WATCOMC__
{
	return (EIO_FileType(dirname) == EIO_TYPE_DIRECTORY);
}
#else
#error FoundDir() not yet implemented for this platform.
#endif
#else
#error FoundDir() not yet implemented for this platform.
#endif

} /* FoundDir */


/**************************************************************************
 *
 * EasyMakeDir
 *
 * SYNOPSIS
 *		BOOL EasyMakeDir (char *dirname)
 *
 * PURPOSE
 *		Create a new directory the easy way. Allows Amiga dir names ending
 *		in '/'.  Wow.
 *
 *		Uses AMIGADOS CreateDir ().
 *
 * USAGE
 *		ok = EasyMakeDir (DIRECTORY_NAME);
 *
 * INPUT
 *		DIRECTORY_NAME	: the path name of a directory.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful, FALSE otherwise.
 *
 * HISTORY
 *		11/06/89 Monday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
BOOL EasyMakeDir (char *dirname)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "EasyMakeDir";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
{
	char	*temp;
	long	len;
	BOOL	status;

	len = strlen (dirname);
	if (! (temp = (char *) MEM_malloc (len))) {
/**/	goto ABORT;
	}
	strcpy (temp, dirname);
	if (*(temp + len - 1) == '/') {
		*(temp + len - 1) = '\0';
	}
	status = EIO_MakeDir (temp);
	MEM_free (temp);

	return (status);
/*------------------------------------------------------------------------*/
ABORT:
	return (NULL);
}
#elif __MSDOS__/*dcc*/
{
	size_t len;
	char szName[256];

	len = strlen(dirname);
	strcpy(szName, dirname);
	if (szName[len-1] == '\\')
		szName[len-1] = '\0';

	return EIO_MakeDir(szName);
}
#else
#error EasyMakeDir() not yet implemented for this platform.
#endif
} /* EasyMakeDir */
#endif


/**************************************************************************
 *
 * FullPath
 *
 * SYNOPSIS
 *		ULONG FullPath (
 *			char	*in_name,
 *			char	*out_name,
 *			ULONG	maxlen
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *		length = FullPath (IN_NAME, OUT_NAME, MAXLEN);
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
 *		11/09/89 Thursday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
ULONG FullPath (
	char	*in_name,
	char	*out_name,
#if __AMIGAOS__
	ULONG	maxlen
#elif __MSDOS__
	UWORD	maxlen
#endif/*__AMIGAOS__/__MSDOS__*/
)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FullPath";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	struct FileLock	*filelock;
	ULONG			length = 0L;
	char				*temp;

	if (temp = MEM_calloc ((NAMES * FSIZE) + 1, 1)) {

		filelock = (struct FileLock *) Lock (in_name, ACCESS_READ);

		if (filelock) {
			length = PathName ((BPTR) filelock, temp, (LONG) (NAMES * FSIZE));
			UnLock ((BPTR) filelock);
			strncpy (out_name, temp, maxlen-1);
			out_name[maxlen - 1] = '\0';
		}
		MEM_free (temp);
	}
	return (length);
#elif __MSDOS__/*dcc*//*TESTME, esp C:\ */

#if 0
	out_name[0] = 0;
	return TRUE;
#else
	strncpy(out_name, in_name, maxlen);
	out_name[maxlen - 1] = 0;
	return TRUE;
#endif

#else
#error FullPath() not yet implemented for this platform.
#endif
} /* FullPath */


/**************************************************************************
 *
 * NameAndExt
 *
 * SYNOPSIS
 *		void NameAndExt (
 *			char	*in_path,
 *			char	*out_name,
 *			ULONG	maxlen
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *		NameAndExt (IN_PATH, OUT_NAME, MAXLEN);
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
 *		11/09/89 Thursday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void NameAndExt (
	char	*in_path,
	char	*out_name,
	ULONG	maxlen
)
{
	char *base_ptr;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "NameAndExt";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	base_ptr = BaseName (in_path);
	strncpy (out_name, base_ptr, (size_t) maxlen);

	return;

} /* NameAndExt */


/**************************************************************************
 *
 * ShowStatus
 *
 * SYNOPSIS
 *		void ShowStatus (char *message)
 *
 * PURPOSE
 *		Displays a string message on the screen.
 *
 * USAGE
 *		ShowStatus (STRING_PTR);
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
void ShowStatus (char *message)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowStatus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	strncpy (StatusMessage, message, 39);
	StatusMessage[39] = '\0';
	SetWindowTitles (GlobalRoomWindow->Window, NULL, StatusMessage);
#elif __MSDOS__
	strncpy (StrBuff, message, 40);
	StrBuff[39] = '\0';
	DrawTitleBar();
#endif/*__AMIGAOS__/__MSDOS__*/
	return;

} /* ShowStatus */


/*********************************************************************
 *
 * UpdateMenuCheckmarks
 *
 * PURPOSE
 *		Update the checkmarks in the menus. Also updates certain
 *		STATE variables.
 *
 * INPUT
 *		RoomStuffType *prs	: RoomStuffType to use in updating menus
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		10/20/94 (dcc) - created.
 *
*/
void UpdateMenuCheckmarks(RoomStuffType *prs)
{

	RoomType *prm = prs->Room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateMenuCheckmarks";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Set Locked Room state checkmark in menus for current room. */

	LockRoomSTATE = IsLocked(prm);
	SetStateVar(&ToggleLockRoomState, *ToggleLockRoomState.StateVar);

	/* Set Invisible Layer state checkmark in menus for current layer. */

	InvisiLayerSTATE = prm->FloorLayer->L_Flags & LAYER_INVISIBLE;
	SetStateVar(&ToggleLInvisiState, *ToggleLInvisiState.StateVar);

	/* Set Locked Layer state checkmark in menus for current layer. */

	LockLayerSTATE = prm->FloorLayer->L_Flags & LAYER_LOCKED;
	SetStateVar(&ToggleLLockState, *ToggleLLockState.StateVar);

	/* Set Use Grid state checkmark in menus for current room. */

	UseGridSTATE = IsGridOn(prm);
	SetStateVar(&ToggleUseGridState, *ToggleUseGridState.StateVar);

	/* Set Show Guide state checkmark in menus for current room. */

	ShowGuideSTATE = IsGuideVisible(prm);
	SetStateVar(&ToggleShowGuideState, *ToggleShowGuideState.StateVar);

	/* Set ToggleZoom state checkmark in menuts for current room. */

	ShowZoomSTATE = IsZooming(prm);
	SetStateVar(&ToggleZoomState, *ToggleZoomState.StateVar);

	/* Set spaced tile display checkmark in menus for current room. */

	SpaceSTATE = prs->Flags & DISPLAY_SEPERATED;
	SetStateVar(&SpaceToggleState, *SpaceToggleState.StateVar);

	/* Set scroll lock display checkmark in menus for current room. */

	ScrollLockSTATE = prs->Flags & SCROLL_LOCK;
	SetStateVar(&ScrollLockState, *ScrollLockState.StateVar);

	if (prs->Flags & EDIT_LOCK_MASK)
	{
//		ExcludeItems(WLockToSourceState.ExcludeList);
		SetStateVar(&WLockToSourceState, TRUE);//*WLockToSourceState.StateVar);
	}
	else if (prs->Flags & SOURCE_LOCK_MASK)
	{
//		ExcludeItems(WLockToEditState.ExcludeList);
		SetStateVar(&WLockToEditState, TRUE);//*WLockToEditState.StateVar);
	}
	else
	{
//		ExcludeItems(WLockClearState.ExcludeList);
		SetStateVar(&WLockClearState, TRUE);//*WLockClearState.StateVar);
	}
} /* UpdateMenuCheckmarks */


/**************************************************************************
 *
 * ShowState
 *
 * SYNOPSIS
 *		void ShowState (int number)
 *
 * PURPOSE
 *		Show information messages in the screen's title bar.
 *
 * USAGE
 *		ShowState (STATE_NUMBER)
 *
 * INPUT
 *		STATE_NUMBER	: As defined in typedefs.h.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		10/20/94 (dcc) - Add call to UpdateMenuCheckmarks().
 *
 * SEE ALSO
 *
*/
void ShowState (int number)
{
	RoomType	*room;
	PlotType	*plot		= NULL;
#if __AMIGAOS__
	TileType	*tile		= NULL;
#endif/*__AMIGAOS__*/
	TileSetType	*tileset	= NULL;
	char		hold_buff[MAXMAX];
	char		tempname[MAXMAX];
	char		tempext[MAXMAX];

	BOOL		fGoodTUser = FALSE;
	BOOL		noimages	= FALSE;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowState";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowCursor		= FALSE;
	ShowTUser		= FALSE;
	ShowUser			= FALSE;
	fShowDefaultCursor	= FALSE;

	strncpy (StrBuff, Blanks, ALL - 1);
	room = GlobalRoomWindow->CurrentRoom->Room;
	UpdateMenuCheckmarks(GlobalRoomWindow->CurrentRoom);
	LastTBar = number;

	if ((number == SHOW_TUSER) && (room)) {
		if (IsSource(room)) {
			if (FSelectedPblk(GlobalBlockCopy) &&
				(GlobalBlockCopy->SourceRoom == room)) {
				if ((plot = GetRoomPlot(GlobalBlockCopy->SourceRoom,
						GlobalBlockCopy->SourceX, GlobalBlockCopy->SourceY)) != NULL) {
					if (plot && plot->TileSet_ID && plot->Tile_ID) {
						tileset = TILESET_AT_PLOT(plot);
					}
				}
				fGoodTUser = TRUE;
			}
		}
		else if (IsComposite(room) && IsLocked(room)) {
			if (FSelectedPblk(GlobalBlockCopy) &&
				(GlobalBlockCopy->SourceRoom == room)) {
				static PlotType plt;
				LayerType *play = Head(&room->Layers);

				tileset = room->ptsComposite;
				plt.TileSet_ID = tileset->TS_id;
				plt.Tile_ID = GlobalBlockCopy->SourceY * play->ctilx +
							GlobalBlockCopy->SourceX + 1;
				plot = &plt;

				fGoodTUser = TRUE;
			}
		}
		else {
			LayerType *playSrc = Head(&GlobalBlockCopy->Layers);

			if (FSelectedPblk(GlobalBlockCopy) &&
				(playSrc->cxTile == room->FloorLayer->cxTile) &&
				(playSrc->cyTile == room->FloorLayer->cyTile)) {
				if (CurrentX < room->FloorLayer->ctilx &&
					 CurrentY < room->FloorLayer->ctily)
					plot		= GetRoomPlot (room, CurrentX, CurrentY);
#if __AMIGAOS__
				tile		= TILE_AT_PLOT(plot);
				if (tile) {
#elif __MSDOS__
				if (plot && plot->TileSet_ID && plot->Tile_ID) {
#endif/*__AMIGAOS__/__MSDOS__*/
					tileset	= TILESET_AT_PLOT(plot);
				}
				else {
					tileset	= NULL;
				}
				fGoodTUser = TRUE;
			}
		}
	}
	else if ((room) && (room->Flags & SOURCEROOM)) {
		if (GlobalBlockCopy && GlobalBlockCopy->FirstTileSet) {
			tileset	= GlobalBlockCopy->FirstTileSet;

			/* Is 1st tile set in copy block within source room? */

			if (tileset->SourceRoom != room)
				tileset = FindFirstTileSet (room);
		}
		else {
			tileset = FindFirstTileSet (room);
		}
	}

	noimages = tileset ? (tileset->Flags & DONT_SAVEIMAGES) : FALSE;

	switch (number) {

	case (SHOW_DEFAULT):
	case (SHOW_CURSOR):
	case (SHOW_USER):
	case (SHOW_TUSER):
	case (SHOW_LAYERSHIFT):
		if (room) {
			if (IsComposite(room)) {
				strcpy (StrBuff, "C");
			}
			else {
				if (IsSource(room)) {
					strcpy (StrBuff, "S");
				}
				else {
					strcpy (StrBuff, "E");
				}
			}
			if (IsLocked(room)) {
				strcat (StrBuff, "L");
			}
			else {
				strcat (StrBuff, "U");
			}
		}
		else {
			strcpy (StrBuff, "(no rooms attached)");
		}
		break;
	default:	/* includes SHOW_TITLE */
		break;
	}

	switch (number) {

	case (SHOW_DEFAULT):
	case (SHOW_CURSOR):
	case (SHOW_USER):
	case (SHOW_LAYERSHIFT):
		if (room) {
			if (tileset && tileset->Filespec) {
				if (noimages) {
					strcat (StrBuff, "-");
				}
				else {
					strcat (StrBuff, " ");
				}
				EIO_fnsplit (tileset->Filespec, NULL, tempname, tempext);
				strncat (tempname, tempext, MAXMAX - 1);
				sprintf (hold_buff, "%-12s", tempname);
				strncat (StrBuff, hold_buff, ROOMNAMEFIELD);
			}
			else if (room->Name) {
				strcat (StrBuff, " ");
				sprintf (hold_buff, "%-12s", room->Name);
				strncat (StrBuff, hold_buff, ROOMNAMEFIELD);
			}
			else {
				strcat (StrBuff, " (no name)   ");
			}
		}
		break;
	default:
		break;
	}

	switch (number) {

	case (SHOW_VERSION):
		strcpy (StrBuff, Name);
		strcat (StrBuff, " v");
		strcat (StrBuff, Version);
#if dvpSNASM
		strcat (StrBuff, "s");
#elif (dvpCLD == 2)
		strcat (StrBuff, "C");
#elif dvpCLD
		strcat (StrBuff, "c");
#elif dvpPARALLEL
		strcat (StrBuff, "p");
#elif dvpPsyQ
		strcat (StrBuff, "Q");
#elif dvpNONE
#endif
		strcat (StrBuff, " ");
		strcat (StrBuff, CreationDate);
		LastTBar = LastRealTBar;
		break;

	case (SHOW_DEFAULT):
	case (SHOW_CURSOR):
	case (SHOW_LAYERSHIFT):
		LastRealTBar = number;
		fShowDefaultCursor = TRUE;
		if (number == SHOW_CURSOR)
		{
			ShowCursor = TRUE;
		}
		if (room) {
			strcat (StrBuff, "  ");

			if (number == SHOW_CURSOR)
			{
				LayerType *play = FindNonEmptyLayer(&room->Layers, room->FloorLayer);

				if (IsSource(room) ||
					(IsComposite(room) && IsLocked(room)) ||
					(IsEdit(room) && !play)) {
					strcat (StrBuff, "(-----,-----)");
				}
				else {
					strcat (StrBuff, "(");
					sprintf (hold_buff, "%05d", CurrentX);
					strncat (StrBuff, hold_buff, 5);
					strcat (StrBuff, ",");
					sprintf (hold_buff, "%05d", CurrentY);
					strncat (StrBuff, hold_buff, 5);
					strcat (StrBuff, ")");
				}
			}
			else if (number == SHOW_LAYERSHIFT)
			{
				strcat(StrBuff, "<");
				sprintf(hold_buff, "%+05d", -room->FloorLayer->dtilx/wShiftXUnit);
				strncat(StrBuff, hold_buff, 5);
				strcat(StrBuff, ",");
				sprintf(hold_buff, "%+05d", -room->FloorLayer->dtily/wShiftYUnit);
				strncat(StrBuff, hold_buff, 5);
				strcat(StrBuff, ">");
			}
			else
			{
				strcat (StrBuff, " ");

				sprintf (hold_buff, "%05d", room->FloorLayer->ctilx);
				strncat (StrBuff, hold_buff, 5);

				strcat (StrBuff, " ");

				sprintf (hold_buff, "%05d", room->FloorLayer->ctily);
				strncat (StrBuff, hold_buff, 5);

				strcat (StrBuff, " ");
			}

			strcat (StrBuff, " ");
			sprintf (hold_buff, "%02d", room->LayerCount);
			strncat (StrBuff, hold_buff, 2);
		}
		break;

	case (SHOW_USER):
		LastRealTBar = number;
		ShowUser = TRUE;
//		strcpy (StrBuff, Name);
		if (room) {
			strcat (StrBuff, " ");

			if (IsSource(room) || IsComposite(room)) {
				if (IsComposite(room))
					tileset = room->ptsComposite;

				if (tileset) {
					strcat (StrBuff, "TT:");
					sprintf (hold_buff, "%05d", tileset->UserType);
					strncat (StrBuff, hold_buff, 5);

					strcat (StrBuff, " ");

					strcat (StrBuff, "TN:");
					sprintf (hold_buff, "%05d", tileset->UserNumber);
					strncat (StrBuff, hold_buff, 5);
				}
				else {
					strcat (StrBuff, " (no tileset)    ");
				}
			}
			else {
				strcat (StrBuff, "RT:");
				sprintf (hold_buff, "%05d", room->UserType);
				strncat (StrBuff, hold_buff, 5);

				strcat (StrBuff, " ");

				strcat (StrBuff, "RN:");
				sprintf (hold_buff, "%05d", room->UserNumber);
				strncat (StrBuff, hold_buff, 5);
			}
		}
		break;

	case (SHOW_TUSER):
		LastRealTBar = number;
		ShowTUser = TRUE;
		if (fGoodTUser) {
#if __AMIGAOS__
			if ((tile) && (tileset)) {
#elif __MSDOS__
			if ((plot && plot->TileSet_ID && plot->Tile_ID) && (tileset)) {
#endif/*__AMIGAOS__/__MSDOS__*/
				if (tileset->Filespec) {
					if (noimages) {
						strcat (StrBuff, "-");
					}
					else {
						strcat (StrBuff, " ");
					}
					EIO_fnsplit (tileset->Filespec, NULL, tempname, tempext);
					strncat (tempname, tempext, MAXMAX - 1);
					sprintf (hold_buff, "%-12s", tempname);
					strncat (StrBuff, hold_buff, 12);
				}
				else {
					strcat (StrBuff, " ??? ??? ??? ");
				}
				strcat (StrBuff, " ");
				sprintf (hold_buff, "%05d", plot->Tile_ID);
				strncat (StrBuff, hold_buff, 5);

				strcat (StrBuff, " ");
				sprintf (hold_buff, "%05d", tileset->UserType);
				strncat (StrBuff, hold_buff, 5);

				strcat (StrBuff, " ");
				sprintf (hold_buff, "%05d", tileset->UserNumber);
				strncat (StrBuff, hold_buff, 5);

			} /* if */
			else {
				strcat (StrBuff, " (null tile)                   ");
			}
		} /* else fGoodTUser */
		else {
			strcat (StrBuff, " (no tile selected)            ");
		}
		break;

	default:	/* includes SHOW_TITLE */
#if 0
		strcpy (StrBuff, Name);
		strcat (StrBuff, " ");
		strcat (StrBuff, Copyright);
#else
		strcpy (StrBuff, Copyright);
#endif
		LastTBar = LastRealTBar;
		break;
	}

	switch (number) {

	case (SHOW_DEFAULT):
	case (SHOW_CURSOR):
	case (SHOW_USER):
	case (SHOW_TUSER):
	case (SHOW_LAYERSHIFT):
		if (room) {
			if (room->FloorLayer->dtilx == 0 && room->FloorLayer->dtily == 0)
				strcat (StrBuff, " ");
			else
				strcat (StrBuff, "*");
			sprintf (hold_buff, "%02d", room->FloorNumber);
			strncat (StrBuff, hold_buff, 2);

			if (room->FloorLayer->L_Flags & LAYER_INVISIBLE) {
				strcat (StrBuff, "i");
			}
			else {
				strcat (StrBuff, "v");
			}

			if (room->FloorLayer->L_Flags & LAYER_LOCKED) {
				strcat (StrBuff, "l");
			}
			else {
				strcat (StrBuff, "u");
			}
			if (EditOnlyFloorSTATE) {
				strcat (StrBuff, "f");
			}
			else {
				strcat (StrBuff, " ");
			}
		}
		break;
	default:	/* includes SHOW_TITLE */
		break;
	}

#if __AMIGAOS__
	if ((number == SHOW_CURSOR) || (number == SHOW_TUSER)) {
		SetDrMd (&MainScreen->RastPort, JAM2);
		SetAPen (&MainScreen->RastPort, 0);
		SetBPen (&MainScreen->RastPort, 1);
	}

	SetWindowTitles (GlobalRoomWindow->Window, NULL, StrBuff);
#elif __MSDOS__
	DrawTitleBar();
#endif

	return;

} /* ShowState */


/**************************************************************************
 *
 * GetRoomPlot
 *
 * SYNOPSIS
 *		PlotType *GetRoomPlot (
 *			RoomType	*room,
 *			WORD		x,
 *			WORD		y
 *		)
 *
 * PURPOSE
 *		Get a tile from a room at coordinates x & y.
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		MSDOS version returns a pointer to a static PlotType.
 *		This value will get overwritten by subsequent calls to
 *		this function.
 *
 * HISTORY
 *		07/21/92 (dcc) - works with locked composite rooms.
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/

PlotType *GetRoomPlot (
	RoomType	*room,
	WORD		x,
	WORD		y
)
{

#if PLOTARRAY
#else
	static PlotType plot;
#endif

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetRoomPlot";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if PLOTARRAY
	if (room)
	{
		LayerType	*roomlayer;
		PlotType	*plot;

		if (IsComposite(room) && IsLocked(room))
		{
			static PlotType plot;

			plot.Plot_Flags = 0;
			plot.TileSet_ID = room->ptsComposite->TS_id;
			plot.Tile_ID = y * room->FloorLayer->ctilx + x + 1;
			return &plot;
		}

		roomlayer	= room->FloorLayer;
		plot		= roomlayer->Plot;
		plot		+= (y * room->FloorLayer->ctilx) + x;
		return (plot);
	}
#else
	if (room)
	{
		if (IsComposite(room) && IsLocked(room))
		{
			plot.Plot_Flags = 0;
			plot.TileSet_ID = room->ptsComposite->TS_id;
			plot.Tile_ID = y * room->FloorLayer->ctilx + x + 1;
			return &plot;
		}
#if 1
#if UseSparseArray
		LAY_ReadPlotXY(room->FloorLayer, x, y, &plot);
#else // !UseSparseArray
		ReadNPlotXY(room->FloorLayer->rgrgplt, x, y, 1, &plot);
#endif // !UseSparseArray
#else
		UBYTE *pub = ActivateXTRAatWin(room->FloorLayer->mpYtMpXtPt[y], 0);

		plot = *(PlotType *) (pub + (x * PLOTSIZE));

		ReleaseXTRA(room->FloorLayer->mpYtMpXtPt[y]);
#endif
		return &plot;
	}
#endif
	else
	{
		return NULL;
	}
} /* GetRoomPlot */

