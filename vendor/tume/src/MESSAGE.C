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
 * MESSAGE.c
 *
 * PROGRAMMER : Gregg A. Tavares 
 *    VERSION : 00.000
 *    CREATED : ??/??/87 
 *   MODIFIED : 10/19/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w-----
 *	 E C H I D N A
 *	--------------
 *
 * DESCRIPTION
 *	Put up a requester with a message and get a reply
 *		
 *
 * HISTORY
 *	08/08/90 (RGM) - Added support for Lattice 5.0x.
 *	09/17/92 (dcc) - add option to time out & return w/ first choice.
 *  09/28/92 (dcc) - add AddKeyOption() to process special keys.
 *	10/06/94 (dcc) - updated to use <echidna/alloc.h>
 *
 * TODO
 *
 *  * make ProccessMesage and Findoptions use pointers for speed.
 *
*/
#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include <echidna/etypes.h>
#include <echidna/grafx.h>
#include <echidna/hitareas.h>
#include <echidna/windows.h>
#include <echidna/eerrors.h>
#include <echidna/mouse.h>
#include "keyboard.h" //<echidna/keyboard.h>
#include <echidna/timer.h>
#include <string.h>
#include <ctype.h>
#include "message.h"					/* Verify function prototypes. */

extern short ExitProgram;


#define LOCAL static


#if TEST
#define LOCAL
#else
#define LOCAL static
#endif

/**************************** C O N S T A N T S ***************************/
#define NORMAL_MESSAGE		0
#define VERTICAL_MESSAGE	1

#define LEFT	1
#define CENTER	2

#define MESSLEFT  32
#define MESSTOP   20
#define MESSWIDTH 260

#define wKeysCntMax	4

/******************************** T Y P E S *******************************/

typedef short (*LineFunc)(short mode, char *line, short len, void *extra);

typedef struct {
	short	  X;
	short	  Y;
	Font	 *Font;
} LineTracker;

/****************************** G L O B A L S *****************************/

static WORD hTim = 0;
static short wTimeLeft = 0;
static short wAutoExitTime = -1;

static short wKeysCnt;
static short wKey[wKeysCntMax];
static short wOpt[wKeysCntMax];

/************************************  ************************************/


/*********************************************************************
 *
 * AllowTimedExit
 *
 * PURPOSE
 *		Set message box to exit automatically after <wTime>.
 *		If <wTime> < 0, then it doesn't exit automatically.
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
void AllowTimedExit(short wTime)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AllowTimedExit";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	wAutoExitTime = wTime;

} /* AllowTimedExit */


/*********************************************************************
 *
 * InitKeyList
 *
 * PURPOSE
 *		Reset list of special keys to process.
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
void InitKeyList(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitKeyList";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	wKeysCnt = 0;

} /* InitKeyList */


/*********************************************************************
 *
 * AddKeyOption
 *
 * PURPOSE
 *		Add key <key> to the keylist. When <key> is pressed,
 *		return <option>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if unable to add to list.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL AddKeyOption(short key, short option)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddKeyOption";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (wKeysCnt < wKeysCntMax)
	{
		wKey[wKeysCnt] = key;
		wOpt[wKeysCnt] = option;
		wKeysCnt++;

		return TRUE;
	}
	return FALSE;

} /* AddKeyOption */


/*********************************************************************
 *
 * PrintLine
 *
 * SYNOPSIS
 *		void PrintLine (
 *			short		 mode,
 *			char		*line,
 *			short		 len,
 *			LineTracker	*lt
 *		)
 *
 * PURPOSE
 *		Print A Line on the Screen at lt->X, lt->Y in mode 'mode' in
 *		font lt->Font and update lt->X, lt->Y
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
LOCAL short PrintLine (
	short		 mode,
	char		*line,
	short		 length,
	LineTracker	*lt
) {

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PrintLine";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		short	x;

		switch (mode) {
			case LEFT:
				x = lt->X;
				break;
			case CENTER:
				x = lt->X + ((MESSWIDTH - 16) - StringLenN (lt->Font, line, length)) / 2;
				break;
		}
		DrawStringN (lt->Font, x, lt->Y, line, length);
		lt->Y += lt->Font->Letters[0]->Height;
	}

	return TRUE;
} /* PrintLine */

#if 0
int GetInput (InputWindow)
struct Window *InputWindow;
{
	struct IntuiMessage *MessMessage;
	struct Screen *OldScreen;
	ULONG IDCMPFlags;
	int flag;
#if !TEST
	SetColors (InputWindow);
#endif
	flag = 0;
	
	IDCMPFlags = InputWindow->IDCMPFlags;
	ModifyIDCMP (InputWindow, (ULONG)GADGETUP);
	if (Request (&MessageRequester, InputWindow)) {
		RestorePointer ();
		OldScreen = IntuitionBase->ActiveScreen;
		if (InputWindow->WScreen != OldScreen) {
			ScreenToFront (InputWindow->WScreen);
		}		
		do {
			while ((MessMessage = (struct IntuiMessage *)
				GetMsg (InputWindow->UserPort)) != NULL) {
				if (MessMessage->Class == GADGETUP) {
					flag = ((struct Gadget *)MessMessage->
						IAddress)->GadgetID;
				}
				ReplyMsg ((struct Message *)MessMessage);
			}
			if (flag == 0) {
				Wait (1L << InputWindow->UserPort->mp_SigBit);
			}
		} while (flag == 0);
		if (OldScreen != IntuitionBase->ActiveScreen) {
			ScreenToFront (OldScreen);
		}
		RemovePointer ();
	}
	ModifyIDCMP (InputWindow, IDCMPFlags);
#if !TEST
	RestoreColors (InputWindow);
#endif
	return (flag);
}

void FreeAll ()
{
	int i;
	struct Gadget *oldGad;

	while (MGadget) {
		oldGad = MGadget->NextGadget;
		FreeChoice (MGadget);
		MGadget = oldGad;
	}
	for (i = 0; i < TotalChoices - 1; i++) {
		while (*GOptions != '\0') {
			GOptions++;
		}
		*GOptions++ = '|';
	}
}
#endif

/*********************************************************************
 *
 * ProcessMessage
 *
 * SYNOPSIS
 *		short ProcessMessage (char *message, LineFunc lfunc)
 *
 * PURPOSE
 *		Separate Message into lines and call 'lfunc' on each line
 *		also count the lines.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		number of lines.
 *
 * HISTORY
 *		06/24/93 Thursday (dcc) - split lines right even if > 30 char w/out space
 *
 * SEE ALSO
 *
*/
LOCAL short ProcessMessage (char *message, LineFunc lfunc, void *extra)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessMessage";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		short	 	 i;
		short		 length;
		short		 mode;
		short		 space;
		short		 count;
		char		 work[31];

		count	= 0;
		i		= 0;
		while (message[i] != NULL) {
			length = 0;
			mode   = 0;
			space  = -1;
			while(length < 30 && message[i] != NULL && mode == 0) {
				if (message[i] == '\n') {
					mode = LEFT;
				} else if (message[i] == '\t') {
					mode = CENTER;
				} else {
					work[length] = message[i];
					length++;
				}
				i++;
				if (message[i] == ' ') {
					space = length;
				}
			}
			if (mode == 0 && message[i] != NULL) {
				if (space == -1) {
					work[30] = NULL;
				} else {
					work[space] = NULL;
					i = i - (length - space) + 1;
				}
			} else {
				work[length] = NULL;
			}
			if (mode == 0) {
				mode = LEFT;
			}
			count++;
			if (lfunc) {
				lfunc (mode, work, strlen (work), extra);
			}

		}

		return count;
	}

} /* ProcessMessage */

/*********************************************************************
 *
 * Message
 *
 * SYNOPSIS
 *		short Message (
 *			struct Window	 *MessWindow,
 *			char 			 *heading,
 *			char 			 *message,
 *			char 			 *options
 *		)
 *
 * PURPOSE
 *		Display Message Requester/Dialog with choices and return number
 *		of choosen choice.
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
short Message (
	struct Window	 *window,
	char 			 *heading,
	char 			 *message,
	char 			 *options
)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Message";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short			 height;
		short			 lines;
		short			 input   = 0;
		HitArea			*ha;
		HitArea			*buttons = NULL;
		SaveAreaType	*sa		 = NULL;
		LineTracker		 lt;

		short			 wKeysCntS;
		short			 wKeyS[wKeysCntMax];
		short			 wOptS[wKeysCntMax];

		/* Copy information onto stack so this routine is (mostly) re-entrant. */
		{
			short i;

			wKeysCntS = wKeysCnt;

			for (i = 0; i < wKeysCntS; i++)
			{
				wKeyS[i] = wKey[i];
				wOptS[i] = wOpt[i];
			}
		}

		if (!PushWindowClipValues (window))
			return 0;					/* Big failure. */

		{
			BeforeGraphics ();

			lines = ProcessMessage (message, NULL, NULL);

			/** Count Choices **/
			{
				short	 clines  = 0;
				short	 cparts  = 1;
				short	 ccount	 = 1;
				short	 lasteol = FALSE;
				char	*s		 = options;

				while (*s) {
					if (*s == '|') {
						ccount++;
						cparts++;
						if (cparts == 3) {
							clines++;
							cparts = 0;
						}
					}
					if (*s == '\n') {
						lasteol = 1;
						if (cparts >= 1) {
							clines++;
						}
						clines++;
						cparts = -1;
					} else {
						lasteol = 0;
					}
					s++;
				}

				buttons = MEM_calloc (ccount + 1, sizeof (HitArea));
				if (!buttons) {
					SetGlobalErr (ERR_OUT_OF_MEMORY);
					GEprintf ("OOM: Allocating message buttons");
					goto msgcleanup;
				}

				height = lines * 8 + 10 + 10 + 2 + (clines - lasteol + (cparts > 0)) * 14;

				sa = SaveArea (MESSLEFT, MESSTOP, MESSWIDTH, height);
				if (!sa) {
					GEcatf ("OOM: Message Area");
					goto msgcleanup;
				}
			}

			SetPenColor (window->Black);
			DrawRect (MESSLEFT, MESSTOP, MESSWIDTH - 2, height - 2);
			DrawRect (MESSLEFT + 2, MESSTOP + 2, MESSWIDTH - 2, height - 2);
			SetPenColor (window->Orange);
			DrawRect (MESSLEFT + 1, MESSTOP + 1, MESSWIDTH - 4, 10);
			SetPenColor (window->White);
			DrawRect (MESSLEFT + 1, MESSTOP + 1 + 11,  MESSWIDTH - 4, height - 10 - 4 - 1);

			lt.Font = window->Font;
			lt.X    = MESSLEFT + 8;
			lt.Y	= MESSTOP  + 2;

			PrintLine (CENTER, heading, strlen (heading), &lt); 

			lt.Y += 4;

			ProcessMessage (message, (LineFunc)PrintLine, &lt);

			lt.Y += 4;

			{
				short	 length;
				short	 x;
				short	 y;
				short	 bx;
				short	 pwidth;
				short	 pheight;
				short	 hcount = 0;
				short	 pcount = 0;
				short	 leftmid;
				short	 rightmid;
				short	 nextline = FALSE;
				short	 center;
				char	*opt;
				char	*owork;

				pheight = lt.Font->Letters[0]->Height;
				ha      = buttons;
				opt     = options;
				do {
					owork  = opt;
					length = 0;
					while (*owork != '|' && *owork) {
						owork++;
						length++;
					}
					if (opt[length - 1] == '\n') {
						length--;
						if (pcount) {
							lt.Y += pheight + 6;
						}
						nextline  = TRUE;
						pcount    = 1;
					}
					if (opt[length - 1] == '\t') {
						length--;
						center = TRUE;
					} else {
						center = FALSE;
					}

					y       = lt.Y;
					pwidth  = StringLenN (lt.Font, opt, length);
					switch (pcount % 3) {
					case 0:
						x        = MESSLEFT + MESSWIDTH - pwidth - 6 - 5;
						rightmid = x;
						break;
					case 1:
						x       = MESSLEFT + 4;
						leftmid = x + pwidth + 5;
						break;
					case 2:
						x        = (((rightmid - leftmid) / 2) + leftmid) - (pwidth + 5) / 2;
						nextline = TRUE;
						break;
					}
					if (nextline) {
						pcount    = 0;
						nextline  = FALSE;
						lt.Y     += pheight + 6;
					} else {
						pcount++;
					}

					if (center) {
						x      = MESSLEFT + 2 + (MESSWIDTH - 3) / 2 - pwidth / 2;
						pwidth = MESSWIDTH - 9 - 5;
						bx     = MESSLEFT + 4;
					} else {
						bx     = x;
						x     += 2;
					}
				
					SetPenColor (window->Black);
					DrawBox (bx, y, pwidth + 4, pheight + 4);
					DrawRect (bx + 1, y + pheight + 4, pwidth + 4, 1);
					DrawRect (bx + pwidth + 4, y + 1, 1, pheight + 4);
					SetPenColor (window->Orange);
					DrawRect (bx + 2, y + 2, pwidth, pheight);
					DrawStringN (lt.Font, x, y + 2, opt, length);

					hcount++;

					ha->X      = bx;
					ha->Y      = y;
					ha->Width  = pwidth + 5;
					ha->Height = pheight + 5;
					ha->Flags  = HTA_COMPLEMENT;
					ha->ID     = hcount;

					ha++;
					opt = owork + 1;
				} while (*owork);

				ha->Width = HTA_LAST_AREA;
			}

			AfterGraphics ();

			if (!hTim)
				hTim = InstallTimer(&wTimeLeft, 1, sizeof(wTimeLeft), -1);

			if (hTim && wAutoExitTime >= 0)
				ResetTimer(hTim, wAutoExitTime);


			for (;;) {
				MouseInfo	mi = {-1, -1, 0, 0};

				/* See if we've timed out automatically. */

				if (wAutoExitTime >= 0 && wTimeLeft == 0)
				{
					ha = buttons;
					break;
				}

				/* See if user quit this program. */

				if (ExitProgram)
				{
					ha = buttons;
					break;
				}

				ReadMouse (&mi);

				if (mi.Buttons) {
					ha = CheckHitAreas (buttons, 0, 0);
					if (ha) {
						break;
					}
				}

				{
					UWORD	 code;
					char	*s;

					code = GetKeyNoWait ();
					if (code) {
						short i;

						code = ConvertKeyToAscii (code);

						for (i = 0; i < wKeysCntS; i++)
						{
							if (code == wKeyS[i])
							{
								input = wOptS[i];
								goto msgcleanup;
							}
						}
						s    = options;
						ha   = buttons;
						while (s && *s && !END_HITAREA (ha)) {
							while (*s && *s != '|' && !isalnum(*s)) {
								s++;
							}
							if (toupper (*s) == toupper (code)) {
								break;
							}
							s  = strchr (s, '|');
							if (s) {
								s++;
							}
							ha = NEXT_HITAREA (ha);
						}
						if (!END_HITAREA (ha)) {
							break;
						}
					}
				}

			}
			input = ha->ID;
		}

msgcleanup:
		PopClipValues ();

		if (sa)			RestoreArea (sa);
		MEM_free (buttons);

		InitKeyList();					/* Reset key list for next call. */

		return (input);
	}
} /* Message */

