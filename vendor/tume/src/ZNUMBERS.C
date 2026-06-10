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
 * NUMBERS.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 01/17/91
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
 *		Requesters to get numbers?
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/etypes.h>
#include <echidna/eerrors.h>
#include <echidna/grafx.h>
#include <echidna/fonts.h>
#include <echidna/edraw.h>
#include "keyboard.h" //<echidna/keyboard.h>
#include <echidna/hitareas.h>
#include <echidna/windows.h>
#include <echidna/eui.h>

#include "numbers.h"					/* Verify function prototypes. */

#define LOCAL	static

/**************************** C O N S T A N T S ***************************/

#define NUM_WIDTH		188
#define NUM_HEIGHT		60
#define NUM_X			(SCREEN_WIDTH  / 2 - NUM_WIDTH  / 2)
#define NUM_Y			(SCREEN_HEIGHT / 2 - NUM_HEIGHT / 2)

#define NUM_BLUE			0
#define NUM_WHITE		1
#define NUM_BLACK		2
#define NUM_ORANGE		3

#define NUMM_X			5

#define NUMB_X			103
#define	NUMB1_Y			15
#define NUMB1_ID		10
#define NUMB2_Y			29
#define NUMB2_ID		11
#define NUMB_WIDTH		57
#define NUMB_HEIGHT		12

#define BUTTON_Y		44
#define BUTTON_HEIGHT	12
#define CANCEL_ID		1
#define CANCEL_X		3
#define CANCEL_WIDTH	49
#define OKAY_ID			3
#define OKAY_X			125
#define OKAY_WIDTH		35

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/********************************* D A T A ********************************/

LOCAL Font	*NUMFont;

LOCAL char *s0 = "Cancel";
LOCAL char *s1 = "Okay";

LOCAL char **NUMStr[] = {
#define CANCEL_STR	0
	&s0,
#define OKAY_STR	1
	&s1,
};

LOCAL struct TextFont **NUMFonts[] = {
	(struct TextFont **)&NUMFont,
};

LOCAL EPic NUMEPic[] = {
	ED_Font (0),
	ED_DrawMode (JAM2),
	ED_FgColorTbl (NUM_WHITE),
	ED_Rect (1, 1, NUM_WIDTH - 3, NUM_HEIGHT - 3),
	ED_FgColorTbl (NUM_ORANGE),
	ED_Rect (1, 1, NUM_WIDTH - 3, 10),
	ED_DrawMode (JAM1),
	ED_FgColorTbl (NUM_BLACK),
	ED_ShadowRect (0, 0, NUM_WIDTH, NUM_HEIGHT),
	ED_RelHLine (0, NUM_WIDTH, 11),
	ED_Rect (NUMB_X, NUMB1_Y, NUMB_WIDTH, NUMB_HEIGHT),
	ED_Rect (NUMB_X, NUMB2_Y, NUMB_WIDTH, NUMB_HEIGHT),
	ED_ShadowRect (CANCEL_X, BUTTON_Y, CANCEL_WIDTH, BUTTON_HEIGHT),
	ED_PosText (CANCEL_X + 2, BUTTON_Y + 2, CANCEL_STR),
	ED_ShadowRect (OKAY_X, BUTTON_Y, OKAY_WIDTH, BUTTON_HEIGHT),
	ED_PosText (OKAY_X + 2, BUTTON_Y + 2, OKAY_STR),
	ED_End,
};

LOCAL UBYTE NUMColors[4];

LOCAL EPicture NUMEPicture = {
	NUMEPic,
	NUMFonts,
	NUMStr,
	NULL,
	NULL,
	NUMColors,
};

LOCAL char NUMString1[(NUMB_WIDTH - 5) / 8 + 2];
LOCAL char NUMString2[(NUMB_WIDTH - 5) / 8 + 2];

LOCAL EditString NUMStrings[2] = {
	{ NUMString1, (NUMB_WIDTH - 5) / 8, NUM_X + NUMB_X + 2, NUM_Y + NUMB1_Y + 2, &NUMStrings[1], &NUMStrings[1], },
	{ NUMString2, (NUMB_WIDTH - 5) / 8, NUM_X + NUMB_X + 2, NUM_Y + NUMB2_Y + 2, &NUMStrings[0], &NUMStrings[0], },
};

LOCAL HitArea NUMHitArea[] = {
	{ NUMB_X, NUMB1_Y, NUMB_WIDTH, NUMB_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, NUMB1_ID, HighlightString, &NUMStrings[0], },
	{ NUMB_X, NUMB2_Y, NUMB_WIDTH, NUMB_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, NUMB2_ID, HighlightString, &NUMStrings[1], },
	{ CANCEL_X, BUTTON_Y, CANCEL_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, CANCEL_ID, NULL, },
	{ OKAY_X, BUTTON_Y, OKAY_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, OKAY_ID, NULL, },
	{ HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, },
};

LOCAL HitArea NUM1HitArea[] = {
	{ NUMB_X, NUMB1_Y, NUMB_WIDTH, NUMB_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, NUMB1_ID, HighlightString, &NUMStrings[0], },
	{ CANCEL_X, BUTTON_Y, CANCEL_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, CANCEL_ID, NULL, },
	{ OKAY_X, BUTTON_Y, OKAY_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, OKAY_ID, NULL, },
	{ HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, },
};
/***************************** R O U T I N E S ****************************/



/*********************************************************************
 *
 * GetTwoNumbers
 *
 * PURPOSE
 *		Display requester asking for two numbers?
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful, FALSE if user hit CANCEL.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short GetTwoNumbers (
		Window	*win,
		char	*title,
		char	*num1msg,
		char	*num2msg,
		short	*num1,
		short	*num2
) {

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetTwoNumbers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		SaveAreaType		*sa = NULL;
		HitArea				*ha;
		struct	RastPort	 rp = { NULL, };
		short				 status = FALSE;
		short				 done   = FALSE;
		short				 key;
		short				 keystat;

		if (PushWindowClipValues (win)) {
			sa = SaveArea (NUM_X, NUM_Y, NUM_WIDTH, NUM_HEIGHT);
			if (!sa) {
				GEcatf ("OOM: Get2Numbers SaveArea");
				goto g2ncleanup;
			}
			NUMFont = win->Font;
			NUMColors[NUM_WHITE ] = win->White;
			NUMColors[NUM_BLACK ] = win->Black;
			NUMColors[NUM_BLUE  ] = win->Blue;
			NUMColors[NUM_ORANGE] = win->Orange;
			itoa (*num1, NUMString1, 10);
			itoa (*num2, NUMString2, 10);

			BeforeGraphics ();
			StartEPicture (&rp, &NUMEPicture, NUM_X, NUM_Y);
			{
				short len;

				len = StringLen (NUMFont, title);
				DrawString (NUMFont, NUM_X + ((NUM_WIDTH - 3) / 2) - (len / 2), NUM_Y + 3, title);
				DrawString (NUMFont, NUM_X + NUMM_X, NUM_Y + NUMB1_Y + 3, num1msg);
				DrawString (NUMFont, NUM_X + NUMM_X, NUM_Y + NUMB2_Y + 3, num2msg);
				UpdateStrings (win, &NUMStrings[0], 2, EDIT_NUM_ONLY);
			}
			AfterGraphics ();

			while (!done) {
				ha = CheckHitAreas (NUMHitArea, NUM_X, NUM_Y);
				if (ha) {
					switch (ha->ID) {
					case OKAY_ID:
						status = TRUE;
						done   = TRUE;
						break;
					case CANCEL_ID:
						done   = TRUE;
						break;
					}
				}
				keystat = CheckEditKeys (&key);
				if (keystat) {
					done = TRUE;
					if (keystat > 0) {
						status = TRUE;
					}
				}
			}

			if (status) {
				*num1 = atoi (NUMString1);
				*num2 = atoi (NUMString2);
			}

g2ncleanup:
			if (sa)		RestoreArea (sa);
			FlushKeyboardBuffer ();
			PopClipValues ();
		}
		return status;
	}

} /* GetTwoNumbers */


/*********************************************************************
 *
 * GetOneNumber
 *
 * PURPOSE
 *		Display requester asking for one number?
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
short GetOneNumber (
		Window	*win,
		char	*title,
		char	*num1msg,
		short	*num1
) {

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetOneNumber";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		SaveAreaType		*sa = NULL;
		HitArea				*ha;
		struct	RastPort	 rp = { NULL, };
		short				 status = FALSE;
		short				 done   = FALSE;
		short				 key;
		short				 keystat;

		if (PushWindowClipValues (win)) {
			sa = SaveArea (NUM_X, NUM_Y, NUM_WIDTH, NUM_HEIGHT);
			if (!sa) {
				GEcatf ("OOM: Get1Number SaveArea");
				goto g1ncleanup;
			}
			NUMFont = win->Font;
			NUMColors[NUM_WHITE ] = win->White;
			NUMColors[NUM_BLACK ] = win->Black;
			NUMColors[NUM_BLUE  ] = win->Blue;
			NUMColors[NUM_ORANGE] = win->Orange;
			itoa (*num1, NUMString1, 10);

			BeforeGraphics ();
			StartEPicture (&rp, &NUMEPicture, NUM_X, NUM_Y);
			SetPenColor (win->White);
			DrawRect (NUM_X + NUMB_X, NUM_Y + NUMB2_Y, NUMB_WIDTH, NUMB_HEIGHT);
			{
				short len;

				len = StringLen (NUMFont, title);
				DrawString (NUMFont, NUM_X + ((NUM_WIDTH - 3) / 2) - (len / 2), NUM_Y + 3, title);
				DrawString (NUMFont, NUM_X + NUMM_X, NUM_Y + NUMB1_Y + 3, num1msg);
				UpdateStrings (win, &NUMStrings[0], 1, EDIT_NUM_ONLY);
			}
			AfterGraphics ();

			while (!done) {
				ha = CheckHitAreas (NUM1HitArea, NUM_X, NUM_Y);
				if (ha) {
					switch (ha->ID) {
					case OKAY_ID:
						status = TRUE;
						done   = TRUE;
						break;
					case CANCEL_ID:
						done   = TRUE;
						break;
					}
				}
				keystat = CheckEditKeys (&key);
				if (keystat) {
					done = TRUE;
					if (keystat > 0) {
						status = TRUE;
					}
				}
			}

			if (status) {
				*num1 = atoi (NUMString1);
			}

g1ncleanup:
			if (sa)		RestoreArea (sa);
			FlushKeyboardBuffer ();
			PopClipValues ();
		}
		return status;
	}

} /* GetOneNumber */

