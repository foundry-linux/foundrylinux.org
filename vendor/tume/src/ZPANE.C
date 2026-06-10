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
 * PANE.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 01/28/91
 *   MODIFIED : 01/22/95
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
 *		01/22/95 (dcc) - Make module lint with no errors.
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/grafx.h>
#include <echidna/hitareas.h>
#include <echidna/mouse.h>

#include "pane.h"						/* Verify function prototypes. */
#include "stubs.h"	// KLUDGE

#include "events.h"
#include "icons.h"
#include "tuglbl.h"

#include "tumedraw.h"

/**************************** C O N S T A N T S ***************************/

#define TITLE_VERSION	0
#define TITLE_COORDS	1
#define TITLE_MSG		2

#define NIY				TitleMsg ("Not Implemented Yet.")

#define WINDOW0_AREA	(GlobalAreas[0])
#define WINDOW1_AREA	(GlobalAreas[1])

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/
extern char	StrBuff[];

extern short lastMouseY;

short	 ShowToolsSTATE = FALSE;
short	 TitleType = 0;
char	*TitleString;

static short zoomed;
static short pnf[NUM_PANES];

/******************************* M A C R O S ******************************/

#define TOOLS_WIDTH (Icons[BPI_TOOLPALETTE]->Width)

#define StopWarnings	ei = ei;

/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * DrawTitleBar
 *
 * SYNOPSIS
 *		void DrawTitleBar (void)
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
void DrawTitleBar (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawTitleBar";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (ShowTitleSTATE) {
		char	 msg[40];

		strncpy(msg, StrBuff, 40);
		msg[39] = 0;

		if (PushWindowClipValues (MainWindow)) {
			BeforeGraphics ();
			SetPenColor (MainWindow->White);
			DrawRect (0, 0, SCREEN_WIDTH, TITLE_HEIGHT - 1);
			SetPenColor (MainWindow->Black);
			DrawRect (0, TITLE_HEIGHT - 1, SCREEN_WIDTH, 1);
			DrawString (MainWindow->Font, 1, 1, msg);
			AfterGraphics ();
			PopClipValues ();
		}
	}

} /* DrawTitleBar */


/*********************************************************************
 *
 * RefreshDisplay
 *
 * SYNOPSIS
 *		void RefreshDisplay (void)
 *
 * PURPOSE
 *		Redraw the entire display.
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
void RefreshDisplay (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RefreshDisplay";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	BeforeGraphics ();
	if (PushWindowClipValues (MainWindow)) {
		DrawTitleBar ();
		ShowRoom (GlobalRoomWindow);
		PopClipValues ();
	}
	AfterGraphics ();

} /* RefreshDisplay */


/*********************************************************************
 *
 * RefigurePanes
 *
 * PURPOSE
 *		Figure out the sizes and position of the panes and dividers
 *		based on various flags.
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
 *		05/15/94 Sunday (dcc) - call SetDestinationClip() vs SetClipDimensions()
 *
 * SEE ALSO
 *
*/
void RefigurePanes (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RefigurePanes";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short	screentop;

		screentop   = (ShowTitleSTATE) ? TITLE_HEIGHT : 0;

		TopAdd = screentop;
		SetDestinationClip();

		WINDOW0_AREA.Height	= max(screentop, 1);

		WINDOW1_AREA.Y      = screentop;
		WINDOW1_AREA.Height = SCREEN_HEIGHT - WINDOW1_AREA.Y;

		lastMouseY = TITLE_HEIGHT+1;	/* Force call to SetColors() in main if needed */
		RefreshDisplay ();
	}

} /* RefigurePanes */

/*********************************************************************
 *
 * ToggleTitleBar
 *
 * SYNOPSIS
 *		short ToggleTitleBar (EventInfo *ei)
 *
 * PURPOSE
 *		Turn TitleBar on and off.
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
short ToggleTitleBar (EventInfo *ei)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleTitleBar";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	StopWarnings;
	RefigurePanes ();
	return TRUE;

} /* ToggleTitleBar */


/*********************************************************************
 *
 * SavePanePos
 *
 * SYNOPSIS
 *		void SavePanePos (void)
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
void SavePanePos (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SavePanePos";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short	 i;
		Pane	*p;

		for (i = 0; i < NUM_PANES; i++) {
			p      = Panes[i];
			pnf[i] = p->P1.Flags;
		}
	}
} /* SavePanePos */

/*********************************************************************
 *
 * RestorePanePos
 *
 * SYNOPSIS
 *		void RestorePanePos (void)
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
void RestorePanePos (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RestorePanePos";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short	 i;
		Pane	*p;

		for (i = 0; i < NUM_PANES; i++) {
			p = Panes[i];
			p->P1.Flags    = (p->P1.Flags & (~PANEF_OFF)) | (pnf[i] & PANEF_OFF);
		}
	}
} /* RestorePanePos */

/*********************************************************************
 *
 * ZoomActiveWindow
 *
 * SYNOPSIS
 *		short ZoomActiveWindow (EventInfo *ei)
 *
 * PURPOSE
 *		Zoom the active window to full screen?
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
short ZoomActiveWindow (EventInfo *ei)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ZoomActiveWindow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	StopWarnings;
	{
		if (zoomed) {
			RestorePanePos ();
		} else {
			SavePanePos ();
			Pane1->P1.Flags      |= PANEF_OFF;
			ActivePane->P1.Flags &= (~PANEF_OFF);
		}
		RefigurePanes ();
		zoomed = !zoomed;
	}
	return TRUE;

} /* ZoomActiveWindow */


/*********************************************************************
 *
 * SetPaneToEdit
 *
 * SYNOPSIS
 *		short SetPaneToEdit (EventInfo *ei)
 *
 * PURPOSE
 *		Set ActivePane to an Edit type pane.
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
short SetPaneToEdit (EventInfo *ei)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetPaneToEdit";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	StopWarnings;
	ActivePane->P1.Type = PANE_EDIT;
	return TRUE;

} /* SetPaneToEdit */

/*********************************************************************
 *
 * ActivatePane
 *
 * SYNOPSIS
 *		void ActivatePane (Pane *p)
 *
 * PURPOSE
 *		Set active pane and update pane display.
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
void ActivatePane (Pane *p)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ActivatePane";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (ActivePane != p) {
		ActivePane = p;

		p->P1.LeftFirstDown = NULL;
		p->P1.LeftDown      = NULL;
		p->P1.LeftUp        = NULL;

		switch (p->P1.Type) {
		case PANE_EDIT:
			SetupEditPane (p);
			break;
		}
	}

} /* ActivatePane */

extern int MouseX;
extern int MouseY;

/*********************************************************************
 *
 * ProcessPane
 *
 * SYNOPSIS
 *		short ProcessPane (HitInfo *hi)
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
short ProcessPane (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessPane";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		Pane	*p;
		UWORD	 buttons;

		p       = hi->HitArea->UserData;
		buttons = hi->Buttons;

//dcc
		MouseX = hi->X;
		MouseY = hi->Y;
//dcc
		if (buttons) {
			if (buttons & MOUSE_FIRSTDOWN) {
				ActivatePane (p);
				if (p->P1.LeftFirstDown) {
					p->P1.LeftFirstDown (hi);
				}
			} else {
				if (p->P1.LeftDown) {
					p->P1.LeftDown (hi);
				}
			}
		} else {
			if (p->P1.LeftUp) {
				p->P1.LeftUp (hi);
			}
		}
		return TRUE;
	}
} /* ProcessPane */

