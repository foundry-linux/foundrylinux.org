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
 * Rmlst.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 03/17/92
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
 *		Displays a dialog Room List
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/strings.h>
#include <echidna/listfunc.h>
#include <echidna/emacros.h>
#include <echidna/eerrors.h>
#include <echidna/grafx.h>
#include <echidna/fonts.h>
#include <echidna/lstgadg.h>
#include <echidna/hitareas.h>
#include <echidna/windows.h>
#include <echidna/eui.h>
#include <echidna/edraw.h>
#include <echidna/events.h>
#include <echidna/mouse.h>
#include "keyboard.h" //<echidna/keyboard.h>

#include "tuglbl.h"
#include "rmlst.h"						/* Verify function prototypes. */

#define LOCAL static
/**************************** C O N S T A N T S ***************************/

#define RT_WHITE	0
#define RT_BLACK	1
#define RT_BLUE		2
#define RT_ORANGE	3

#define RTDLG_WIDTH		168
#define RTDLG_HEIGHT	178
#define RTDLG_LEFT		((SCREEN_WIDTH / 2) - (RTDLG_WIDTH / 2))
#define RTDLG_TOP		((((SCREEN_HEIGHT - 12) / 2) - (RTDLG_HEIGHT / 2)) + 12)

#define RTDLG_TLST_LEFT		3
#define RTDLG_TLST_TOP		14
#define RTDLG_TLST_WIDTH	161
#define RTDLG_TLST_HEIGHT	146
#define RTDLG_TLST_ARROW_W	12
#define RTDLG_TLST_NAME_W	(RMDLG_TLIST_WIDTH - 1 - RMDLG_TLST_ARROW_W)

#define CANCEL_LEFT			3
#define CANCEL_TOP			OK_TOP
#define CANCEL_WIDTH		49
#define CANCEL_ID			10
#define OK_LEFT				118
#define	OK_TOP				162
#define OK_WIDTH			46
#define OK_ID				11
#define BUTTON_HEIGHT		12

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

LOCAL Window	*RTWin;
LOCAL Font		*RTFont;
LOCAL UBYTE		 RtColors[4];
LOCAL short		 RTtop;

LOCAL short		 RTdone;
LOCAL short		 RTsuccess;

/******************************* M A C R O S ******************************/

#define ED_Button(x,y,w,h)	\
	ED_Rect ((x), (y), (w) - 1, (h) -1),				\
	ED_RelHLine ((x) + 1, (w) - 1, (y) + (h) - 1),		\
	ED_RelVLine ((x) + (w) - 1, (y) + 1, (h) - 1)		\

/******************************* T A B L E S ******************************/

LOCAL char *s0 = "Okay";
LOCAL char *s1 = "Cancel";

LOCAL char **RtStr[] = {
#define OK_STR	0
	&s0,
#define CANCEL_STR	1
	&s1,
};

LOCAL struct TextFont **RtFonts[] = {
	(struct TextFont **)&RTFont,
};

LOCAL EPic RTEPic[] = {
	ED_Font (0),
	ED_DrawMode (JAM2),
	ED_FgColorTbl (RT_WHITE),
	ED_Rect (0, 0, RTDLG_WIDTH - 1, RTDLG_HEIGHT - 1),
	ED_FgColorTbl (RT_ORANGE),
	ED_Rect (1, 1, RTDLG_WIDTH - 3, 10),
	ED_DrawMode (JAM1),
	ED_FgColorTbl (RT_BLACK),
	ED_Button (0, 0, RTDLG_WIDTH, RTDLG_HEIGHT),
	ED_RelHLine (0, RTDLG_WIDTH, 11),

	/* Okay/Cancel */
	ED_Button (OK_LEFT, OK_TOP, OK_WIDTH, BUTTON_HEIGHT),
	ED_PosText (OK_LEFT + 2, OK_TOP + 2, OK_STR),
	ED_Button (CANCEL_LEFT, CANCEL_TOP, CANCEL_WIDTH, BUTTON_HEIGHT),
	ED_PosText (CANCEL_LEFT + 2, CANCEL_TOP + 2, CANCEL_STR),

	ED_End,
};

LOCAL EPicture RTEPicture = {
	RTEPic,
	RtFonts,
	RtStr,
	NULL,
	NULL,
	RtColors,
};

/***************************** R O U T I N E S ****************************/

/*********************************************************************
 *
 * void	SelectFunc
 *
 * SYNOPSIS
 *		LOCAL void	SelectFunc (void *pvdata, UWORD fWhy)
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
LOCAL void SelectFunc (void *pvdata, UWORD fWhy)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SelectFunc";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		pvdata = pvdata;

		if (fWhy & LGAD_DOUBLE_CLICK) {
			RTdone    = TRUE;
			RTsuccess = TRUE;
		}
	}
} /* void SelectFunc */

static void DisplayText (
	NodeType *pnod,
	short x, short y
)	
{
	BeforeGraphics ();
	if (pnod && ((RoomType *)pnod)->Name) {
		DrawString (RTFont, x + 1, y + 1, ((RoomType *)pnod)->Name);
	}
	AfterGraphics ();
}

/********************************* Gadgets ********************************/
	
LOCAL HitArea RTHitArea[] = {
#define LIST_GAD	0
	{ RTDLG_TLST_LEFT, RTDLG_TLST_TOP, RTDLG_TLST_WIDTH, RTDLG_TLST_HEIGHT,
		HTA_SUBAREAS, 0, NULL, NULL, },

	/* Ok / Cancel */
	{ OK_LEFT, OK_TOP, OK_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, OK_ID, NULL},
	{ CANCEL_LEFT, CANCEL_TOP, CANCEL_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, CANCEL_ID, NULL},
	{ HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, },
};

/*********************************************************************
 *
 * RoomListReq
 *
 * SYNOPSIS
 *		RoomType *RoomListReq (ListType *roomlist, RoomType *currentroom)
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
RoomType *RoomListReq (ListType *roomlist, RoomType *currentroom)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RoomListReq";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		SaveAreaType		*sa = NULL;
		HitArea				*ha;
		struct	RastPort	 rp = { NULL, };
		LGAD				 lgadx;
		LGAD				*lgad = &lgadx;

		RTdone    = FALSE;
		RTsuccess = FALSE;
		
		RTWin   = MainWindow;
		RTFont  = &MainFont;
		RtColors[RT_WHITE ] = RTWin->White;
		RtColors[RT_BLACK ] = RTWin->Black;
		RtColors[RT_BLUE  ] = RTWin->Blue;
		RtColors[RT_ORANGE] = RTWin->Orange;

		{
			NodeType	*rt;

			rt = Head (roomlist);
			while (!EndOfList (rt)) {
				rt->Priority = 0;
				if ((void *)rt == (void *)currentroom) {
					rt->Priority |= LGAD_NOD_SELECTED;
				}
				rt = Next (rt);
			}
		}

		if (PushWindowClipValues (RTWin)) {
			sa = SaveArea (RTDLG_LEFT, RTDLG_TOP, RTDLG_WIDTH, RTDLG_HEIGHT);
			if (!sa) {
				GEcatf ("\nOOM: FileReq Area");
				goto dRTcleanup;
			}

			if (InitLGAD (
					lgad,
					RTDLG_TLST_WIDTH,
					RTDLG_TLST_HEIGHT,
					roomlist,
					LGAD_V_ELEVATOR | LGAD_V_REALTIME_SLIDER | LGAD_EXCLUSIVE,
					RTDLG_TLST_ARROW_W,
					20,20,
					1000,
					GetFontHeight(RTFont) + 1,
					GetFontHeight(RTFont),
					DisplayText,
					SelectFunc,
					lgad)) {

				SetLGADListPos (lgad, RTtop, LGAD_POS_TOP);

				RTHitArea[LIST_GAD].Data = phaLGAD(lgad);

				BeforeGraphics ();

				StartEPicture (&rp, &RTEPicture, RTDLG_LEFT, RTDLG_TOP);
				DrawLGAD (lgad, RTDLG_LEFT + RTDLG_TLST_LEFT, RTDLG_TOP + RTDLG_TLST_TOP);
				DrawString (RTFont, RTDLG_LEFT + RTDLG_WIDTH / 2 - StringLen (RTFont, "Choose A Room") / 2, RTDLG_TOP + 2, "Choose A Room");

				AfterGraphics ();

				while (!RTdone) {

					ha = CheckHitAreas (RTHitArea, RTDLG_LEFT, RTDLG_TOP);
					if (ha) {
						if (ha->ID == OK_ID) {
							RTdone    = TRUE;
							RTsuccess = TRUE;
							break;
						}
						if (ha->ID == CANCEL_ID) {
							RTdone  = TRUE;
							break;
						}
					}
				}

				RTtop = ieLGADGetTopElmntNdx (lgad);
			}

dRTcleanup:
			if (sa)		RestoreArea (sa);
			FlushKeyboardBuffer ();

			PopClipValues ();
		}

		if (RTsuccess) {
			RoomType	*rt;

			rt = (RoomType *)pnodLGADFirstSelected (lgad);
			if (rt) {
				currentroom = rt;
			}
		}
	 	return currentroom;
	}
} /* RoomListReq */

