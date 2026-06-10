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
 * EUI.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 12/17/90
 *   MODIFIED : 03/02/95
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		OpenEUI() and CloseEUI().
 *
 * HISTORY
 *		04/05/94 Tuesday (dcc) - put EUIMessageLoop() in EUIMSG.C.
 *		04/05/94 Tuesday (dcc) - KLUDGED to set palette to black so garbage
 *											put on-screen by LoadBPI() is not visible.
 *
*/

#include <echidna/platform.h>

#include <echidna/etypes.h>
#include <echidna/eui.h>
#include "keyboard.h" //<echidna/keyboard.h>
#include <echidna/windows.h>
#include <echidna/eerrors.h>
#include <echidna/eio.h>
#include <echidna/grafx.h>
#include <echidna/mouse.h>
#include <echidna/brushsup.h>
#include <echidna/timer.h>
#include <echidna/readini.h>
#include <echidna/menus2.h>

#include "system.h"
#include "small.h"
#include "euibpi.h"
#include "euidata.h"

/**************************** C O N S T A N T S ***************************/

#define MENU_WHITE	255
#define MENU_BLACK	254
#define EUI_ORANGE	253
#define EUI_BLUE	252

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

static short		 openkeyboard;
static short		 initkeys;
static short		 initmenus;
static short		 opengraphics;
static short		 openmouse;
static short		 opentimers;
static BPIType		*FontBPI;
static IniList		*EUIinilist;

static WORD			 EUICountDownTimerHndl;
UWORD				 EUICountDownTimer;
short				 EUIDBuf;

Font SmallFont = {
	NULL,
	SMALL_FIRSTCHAR,
	SMALL_LASTCHAR,
};

Font MainFont = {
	NULL,
	SYSTEM_FIRSTCHAR,
	SYSTEM_LASTCHAR,
};

Window MainWindowX = {
	NULL,
	NULL,
	&MainFont,
	NULL,
	NULL,
	0,
	0,
	320,
	200,
	MENU_BLACK,
	MENU_WHITE,
	EUI_ORANGE,
	EUI_BLUE,
	EUI_ORANGE,
	EUI_BLUE,
};

Window *MainWindow = &MainWindowX;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * CloseEUI
 *
 * SYNOPSIS
 *		void CloseEUI (void)
 *
 * PURPOSE
 *		Free all EUI resources and go back to text mode.
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
void CloseEUI (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CloseEUI";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		if (EUICountDownTimerHndl) {
			RemoveTimer (EUICountDownTimerHndl);
		}

		if (FontBPI) {
			FreeBPI (FontBPI);
		}

		if (openmouse) {
			HideMouse ();
			CloseMouse ();
		}

		if (opentimers) {
			CloseTimers ();
		}

		if (opengraphics) {
			if (EUIDBuf) {
				CloseDBufGraphics ();
			} else {
				CloseGraphics ();
			}
		}

		if (initkeys) {
			UninitKeys ();
		}

		if (EUIinilist) {
			FreeINI (EUIinilist);
		}

		if (openkeyboard) {
			CloseKeyboard ();
		}
	}
} /* CloseEUI */

/*********************************************************************
 *
 * OpenEUI
 *
 * SYNOPSIS
 *		short OpenEUI (char *inifile)
 *
 * PURPOSE
 *		Start up the Easy User Interface.
 *
 *		* Calls OpenKeyboard to start the keyboard interrupt.
 *		* Calls OpenGraphics to start up the graphics systems
 *		* Calls OpenMouse to start the mouse
 *		* Loads the EUI fonts and symbols.
 *		* Reads the specified .ini file.
 *			* Read the key assignments.
 *			* Read the menu setup.
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
short OpenEUI (char *inifile)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenEUI";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		char	inipath[EIO_MAXPATH];
		char	fpfname[EIO_MAXPATH];
		char	datapath[EIO_MAXPATH];
		short	gfxopn;

		initmenus  = 0;
		initmenus = initmenus;

		if (!EIO_FindFile (inifile, NULL, inipath)) {
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nCouldn' find file '%s' in path", inifile);
			return FALSE;
		}

		EIO_fnmerge (fpfname, NULL, EIO_Name (inifile), ".fpf");

		if (!EIO_FindFile (fpfname, NULL, datapath)) {
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nCouldn' find file '%s' in path", fpfname);
			return FALSE;
		}

		if (OpenKeyboard ()) {
			openkeyboard = TRUE;
			EUIinilist = ReadINI (inipath);
			if (EUIinilist) {
				if (InitKeys (EUIinilist)) {
					initkeys = TRUE;
					if (InitMenus (EUIinilist)) {
						FreeINI (EUIinilist);
						EUIinilist = NULL;
						if (EUIDBuf) {
							gfxopn = (OpenDBufGraphics (MODE_MCGA, EUIDBuf, 64000U, 256, 8, 30));
						} else {
							gfxopn = (OpenGraphics (MODE_MCGA, 64000U, 256, 8, 30));
						}
						if (gfxopn) {
#if __MSDOS16__	// KLUDGE
							{
								struct SREGS sr;
								union REGS r;

								sr.es  = 0xA000;
								r.x.dx = 0x0000;
								r.x.ax = 0x1012;
								r.x.bx = 0;
								r.x.cx = 256;

								memset(MK_FP(0xA000,0x0000), 0, 768);
								int86x(0x10, &r, &r, &sr);
							}
#endif
#if __MSDOS32X__
#need support!
#endif
							opengraphics = TRUE;
							OpenTimers ();
							opentimers = TRUE;
							if (OpenMouse ()) {
								openmouse = TRUE;

								FontBPI = LoadBPI (datapath, FPI_SYSTEM_FONT);
								if (FontBPI) {

									MainFont.Letters         = &FontBPI->Shapes[BPI_SYSTEM_SYSBPI];
									SmallFont.Letters        = &FontBPI->Shapes[BPI_SMALL_SMALL];

									SetMouseShape (FontBPI->Shapes[BPI_MOUSE_POINTERS]);

									SetMenuInfo (320, 200, &MainFont, &FontBPI->Shapes[BPI_MENU_SYMBOLS]);

									ShowMouse ();

									BeforeGraphicsFunc = HideMouse;
									AfterGraphicsFunc  = ShowMouse;

									EUICountDownTimerHndl = InstallTimer (	&EUICountDownTimer,
																			EUI_TIMER_HERTZ,
																			sizeof (UWORD),
																			(-1));
									if (EUICountDownTimerHndl) {
										return TRUE;
									}
								}
							}
						}
					}
				}
			}
		}
		CloseEUI ();
		return FALSE;
	}
} /* OpenEUI */

