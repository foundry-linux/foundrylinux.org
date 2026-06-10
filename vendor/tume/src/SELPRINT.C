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
 * SELPRINT.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 08/29/93
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
 *		Displays a dialog to select a printer. Based heavily on GAT's RM_TSINF.C.
 *
 * HISTORY
 *		08/29/93 (dcc) - Created.
 *		01/22/95 (dcc) - Make module lint with no errors.
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
#include "parseini.h"
#include "printmap.h"
#include "rm_tsinf.h"

#define LOCAL static
/**************************** C O N S T A N T S ***************************/

#define RT_WHITE	0
#define RT_BLACK	1
#define RT_BLUE		2
#define RT_ORANGE	3

#define RTDLG_WIDTH		299
#define RTDLG_HEIGHT	105
#define RTDLG_LEFT		((SCREEN_WIDTH / 2) - (RTDLG_WIDTH / 2))
#define RTDLG_TOP		((((SCREEN_HEIGHT - 12) / 2) - (RTDLG_HEIGHT / 2)) + 12)

#define RTDLG_TLST_LEFT		3
#define RTDLG_TLST_TOP		27
#define RTDLG_TLST_WIDTH	122
#define RTDLG_TLST_HEIGHT	74
#define RTDLG_TLST_ARROW_W	12
#define RTDLG_TLST_NAME_W	(RMDLG_TLIST_WIDTH - 1 - RMDLG_TLST_ARROW_W)

#define ARROW_WIDTH			11
#define ARROW_HEIGHT		11
#define NUMBER_WIDTH		57
#define NUMBER_HEIGHT		11

#define RTDLG_NUM_LEFT		127
#define RT_WIDTH_TOP		27
#define RT_HEIGHT_TOP			41
#define LABEL_LEFT			(RTDLG_NUM_LEFT + ARROW_WIDTH * 2 + NUMBER_WIDTH)

#define CANCEL_LEFT			198
#define CANCEL_TOP			OK_TOP
#define CANCEL_WIDTH		49
#define CANCEL_ID			10
#define OK_LEFT				(CANCEL_LEFT + CANCEL_WIDTH + 2)
#define	OK_TOP				89
#define OK_WIDTH			43
#define OK_ID				11
#define BUTTON_HEIGHT		12
#define TSET_TOP			103
#define	TSET_HEIGHT			37


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

LOCAL Window	*RTWin;
LOCAL Font		*RTFont;
LOCAL Font		*RTSFont;
LOCAL UBYTE		 RtColors[4];

LOCAL short		 RTdone;
LOCAL short		 RTsuccess;

/* Forward (not extern) references. */

extern RT_Type rtHPPCL3;
extern RT_Type rtHPGL2;
extern RT_Type rtEOL;

ListType lstPrinter =
{
	(NodeType *)&rtHPPCL3,
	NULL,
	(NodeType *)&rtHPGL2,
	sizeof (ListType),
	0,
	NULL,
};

static RT_Type rtHPPCL3 =
{
	{
		(NodeType *) &rtHPGL2,
		(NodeType *) &lstPrinter,
		sizeof (RT_Type),
		0,
		"HP LaserJet",
	},
	HPPCL3, {0, 0,}
};

static RT_Type rtHPGL2 =
{
	{
		(NodeType *) &rtEOL,
		(NodeType *) &rtHPPCL3,
		sizeof (RT_Type),
		0,
		"HP DesignJet 650C",
	},
	HPGL2, {0, 0,}
};

static RT_Type rtEOL =
{
	{
		NULL,
		(NodeType *) &rtHPGL2,
		sizeof (RT_Type),
		0,
		NULL,
	},
	0, {0, 0,}
};

LOCAL LGAD		*RTlgad;

/******************************* M A C R O S ******************************/

#define ED_Button(x,y,w,h)	\
	ED_Rect ((x), (y), (w) - 1, (h) -1),				\
	ED_RelHLine ((x) + 1, (w) - 1, (y) + (h) - 1),		\
	ED_RelVLine ((x) + (w) - 1, (y) + 1, (h) - 1)		\

/******************************* T A B L E S ******************************/

LOCAL char *s0 = "Okay";
LOCAL char *s1 = "Cancel";

LOCAL char **RtStr[] = {
#define OK_STR			0
	&s0,
#define CANCEL_STR		1
	&s1,
};

LOCAL struct TextFont **RtFonts[] = {
	(struct TextFont **)&RTFont,
	(struct TextFont **)&RTSFont,
};

LOCAL EPic NumberEPic[] = {
	ED_FgColorTbl (RT_BLACK),
	ED_DrawMode (JAM1),
	ED_Rect (0, 0, ARROW_WIDTH * 2 + NUMBER_WIDTH - 2, NUMBER_HEIGHT),
	ED_Rect (ARROW_WIDTH - 1, 0, NUMBER_WIDTH, ARROW_HEIGHT),

	/* Left Arrow */
	ED_DrawMode (JAM2),
	ED_Rect (3, 4, 6, 3),
	ED_DrawMode (JAM1),
	ED_RelVLine (5, 2, 7),
	ED_RelVLine (4, 3, 5),
	ED_Plot (2, 5),

	/* Right Arrow */
	ED_DrawMode (JAM2),
	ED_Rect (ARROW_WIDTH + NUMBER_WIDTH, 4, 6, 3),
	ED_DrawMode (JAM1),
	ED_RelVLine (ARROW_WIDTH + NUMBER_WIDTH + 3, 2, 7),
	ED_RelVLine (ARROW_WIDTH + NUMBER_WIDTH + 4, 3, 5),
	ED_Plot (ARROW_WIDTH + NUMBER_WIDTH + 6, 5),

	ED_End,
};

LOCAL EPic *RtEPics[] = {
#define NUMBER_EPIC	0
	NumberEPic,
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

	/* Numbers */
	ED_Move (RTDLG_NUM_LEFT, RT_WIDTH_TOP),
	ED_Gosub (NUMBER_EPIC),
	ED_Move (RTDLG_NUM_LEFT, RT_HEIGHT_TOP),
	ED_Gosub (NUMBER_EPIC),

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
	RtEPics,
	NULL,
	RtColors,
};


#define NUM_WIDTH_NDX	0
#define NUM_HEIGHT_NDX	1

#define NUM_STRING_GADS	2

LOCAL char	NumStrings[NUM_STRING_GADS][EIO_MAXPATH];

LOCAL EditString NumString[NUM_STRING_GADS] = {
{ NumStrings[0], (NUMBER_WIDTH - 5) / 8, RTDLG_LEFT + RTDLG_NUM_LEFT + ARROW_WIDTH + 1, RTDLG_TOP + RT_WIDTH_TOP + 2, NULL, &NumString[1], },
{ NumStrings[1], (NUMBER_WIDTH - 5) / 8, RTDLG_LEFT + RTDLG_NUM_LEFT + ARROW_WIDTH + 1, RTDLG_TOP + RT_HEIGHT_TOP + 2, NULL, &NumString[0], },
};

typedef EditString	*ESPtr;


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * UpdateListMark
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
LOCAL void UpdateListMark (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateListMark";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		RT_Type	*rt;
		short	 type;
		short	 count;
		short	 pos = (-1);

		count = 0;
		type  = OutputDevice;
		rt    = (RT_Type *) Head (&lstPrinter);
		while (!EndOfList (rt)) {
			rt->Node.Priority = 0;
			if (rt->Type == type) {
				type = -30000;
				pos  = count;
				rt->Node.Priority |= LGAD_NOD_SELECTED;
			}
			count++;
			rt = (RT_Type *) Next (rt);
		}

		if (pos < 0) {
			SetLGADList (RTlgad, &lstPrinter, 1000);
		} else {
			SetLGADListPos (RTlgad, pos, LGAD_POS_MID);
		}
	}

} /* UpdateListMark */


/*********************************************************************
 *
 * IncDecCenNumber
 *
 * PURPOSE
 *		Increment or decrement a "number" <psz> that contains two
 *		fractional digits of information.
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
 *		08/29/93 Sunday (dcc) - created.
 *
*/
void IncDecCenNumber(char *psz, short dir)
{
	long		 val;
	long		 dec;
	char c1 = 0;
	char c2 = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "IncDecCenNumber";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	sscanf (psz, "%lu.%c%c", &val, &c1, &c2);
	dec = (c1 ? c1 - '0' : 0) * 10 + (c2 ? c2 - '0' : 0);
	val = val * 100L + dec;

	if (dir > 0)
	{
		if		(val <   10L)
			val +=    2L;
		else if (val <  100L)
			val +=   10L;
		else if (val <  200L)
			val +=   20L;
		else if (val <  500L)
			val +=   50L;
		else if (val < 1000L)
			val +=  100L;
		else if (val < 2000L)
			val +=  200L;
		else if (val < 5000L)
			val +=  500L;
		else
			val += 1000L;
	}
	else
	{
		if		(val <=   10L)
			val -=    2L;
		else if (val <=  100L)
			val -=   10L;
		else if (val <=  200L)
			val -=   20L;
		else if (val <=  500L)
			val -=   50L;
		else if (val <= 1000L)
			val -=  100L;
		else if (val <= 2000L)
			val -=  200L;
		else if (val <= 5000L)
			val -=  500L;
		else
			val -= 1000L;
	}
	val = max (0L, val);
	val = min (3276600L, val);

	dec = val % 100;
	val = val / 100;

	sprintf (psz, "%ld.%02ld", val, dec);

} /* IncDecCenNumber */


/*********************************************************************
 *
 * IncDecNumber
 *
 * SYNOPSIS
 *		LOCAL void IncDecNumber (void) 
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
LOCAL void IncDecNumber (HitInfo *hi) 
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "IncDecNumber";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		HitArea		*ha  = hi->HitArea;
		short		 num = ha->ID & 0x0F;
		short		 dir = (ha->ID & 0x10) ? (-1) : (1);

		IncDecCenNumber(NumStrings[num], dir);

		BeforeGraphics ();
		UpdateEditString (&NumString[num]);
		AfterGraphics ();

	}
} /* IncDecNumber */


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

	fWhy = fWhy;
	{
		LGAD *plgad = (LGAD *) pvdata;
		RT_Type	*pnod = (RT_Type *) pnodLGADFirstSelected (plgad);

		if (pnod)
		{
			OutputDevice = pnod->Type;
		}
	}
} /* void SelectFunc */


/********************************* Gadgets ********************************/
	
LOCAL HitArea RTHitArea[] = {
#define LIST_GAD	0
	{ RTDLG_TLST_LEFT, RTDLG_TLST_TOP, RTDLG_TLST_WIDTH, RTDLG_TLST_HEIGHT,
		HTA_SUBAREAS, 0, NULL, NULL, },

	/* Width Number */
	{ RTDLG_NUM_LEFT, RT_WIDTH_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x10, IncDecNumber, },
	{ RTDLG_NUM_LEFT + ARROW_WIDTH, RT_WIDTH_TOP, NUMBER_WIDTH - 2, NUMBER_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, HighlightString, &NumString[NUM_WIDTH_NDX], },
	{ RTDLG_NUM_LEFT + ARROW_WIDTH + NUMBER_WIDTH - 1, RT_WIDTH_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x00, IncDecNumber, },

	/* Height Number */
	{ RTDLG_NUM_LEFT, RT_HEIGHT_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x11, IncDecNumber, },
	{ RTDLG_NUM_LEFT + ARROW_WIDTH, RT_HEIGHT_TOP, NUMBER_WIDTH - 2, NUMBER_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, HighlightString, &NumString[NUM_HEIGHT_NDX], },
	{ RTDLG_NUM_LEFT + ARROW_WIDTH + NUMBER_WIDTH - 1, RT_HEIGHT_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x01, IncDecNumber, },

	/* Ok / Cancel */
	{ OK_LEFT, OK_TOP, OK_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, OK_ID, NULL},
	{ CANCEL_LEFT, CANCEL_TOP, CANCEL_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, CANCEL_ID, NULL},
	{ HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, },
};


/*********************************************************************
 *
 * ChoosePrinterReq
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * ASSUMES
 *
 *
 * RETURN VALUE
 *		TRUE if user pressed OK, FALSE if user canceled.
 *
 * HISTORY
 *
*/
short ChoosePrinterReq (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ChoosePrinterReq";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetColorsNPointer (MainWindow);
	{
		SaveAreaType		*sa = NULL;
		HitArea				*ha;
		struct	RastPort	 rp = { NULL, };
		LGAD				 lgadx;
		LGAD				*lgad = &lgadx;

		RTlgad    = lgad;
		RTdone    = FALSE;
		RTsuccess = FALSE;
		
		RTWin   = MainWindow;
		RTFont  = &MainFont;
		RTSFont = &SmallFont;
		RtColors[RT_WHITE ] = RTWin->White;
		RtColors[RT_BLACK ] = RTWin->Black;
		RtColors[RT_BLUE  ] = RTWin->Blue;
		RtColors[RT_ORANGE] = RTWin->Orange;

		sprintf (NumStrings[NUM_WIDTH_NDX],  "%ld.%02ld", cencxPage / 100L, cencxPage % 100L);
		sprintf (NumStrings[NUM_HEIGHT_NDX], "%ld.%02ld", cencyPage / 100L, cencyPage % 100L);

		if (PushWindowClipValues (RTWin)) {
			sa = SaveArea (RTDLG_LEFT, RTDLG_TOP, RTDLG_WIDTH, RTDLG_HEIGHT);
			if (!sa) {
				GEcatf ("\nOOM: FileReq Area");
				goto dRTcleanup;
			}

			if (InitTextLGAD (
					lgad,
					RTDLG_TLST_WIDTH,
					RTDLG_TLST_HEIGHT,
					&lstPrinter,
					LGAD_V_ELEVATOR | LGAD_V_REALTIME_SLIDER | LGAD_EXCLUSIVE,
					RTDLG_TLST_ARROW_W,
					20,20,
					1000,
					SelectFunc,
					lgad)) {

				RTlgad->x = RTDLG_LEFT + RTDLG_TLST_LEFT;	//dcc KLUDGE
				RTlgad->y = RTDLG_TOP + RTDLG_TLST_TOP;		//dcc KLUDGE
				UpdateListMark ();

				RTHitArea[LIST_GAD].Data = phaLGAD(lgad);

				BeforeGraphics ();

				StartEPicture (&rp, &RTEPicture, RTDLG_LEFT, RTDLG_TOP);

				DrawLGAD (lgad, RTDLG_LEFT + RTDLG_TLST_LEFT, RTDLG_TOP + RTDLG_TLST_TOP);

				DrawString (RTFont, RTDLG_LEFT + LABEL_LEFT, RTDLG_TOP + RT_WIDTH_TOP + 2, "Page Width");
				DrawString (RTFont, RTDLG_LEFT + LABEL_LEFT, RTDLG_TOP + RT_HEIGHT_TOP + 2, "Page Height");

				DrawString (RTFont, RTDLG_LEFT + RTDLG_WIDTH / 2 - StringLen (RTFont, "Choose Printer") / 2, RTDLG_TOP + 2, "Choose Printer");

				UpdateStrings (MainWindow, &NumString[0], NUM_STRING_GADS, NULL);

				AfterGraphics ();

				while (!RTdone) {
					short	status;
					short	key;

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
					status = CheckEditKeys (&key);
					if (status) {
						RTdone = TRUE;
						if (status >= 0) {
							RTsuccess = TRUE;
						}
					}
				}
			}

dRTcleanup:
			if (sa)		RestoreArea (sa);
			FlushKeyboardBuffer ();

			PopClipValues ();
		}

		if (RTsuccess)
		{
			long l;
			long ldec;
			char c1 = 0;
			char c2 = 0;

			sscanf (NumStrings[NUM_WIDTH_NDX], "%lu.%c%c", &l, &c1, &c2);
			ldec = (c1 ? c1 - '0' : 0) * 10 + (c2 ? c2 - '0' : 0);
			cencxPage = l * 100L + ldec;

			c1 = 0;	c2 = 0;

			sscanf (NumStrings[NUM_HEIGHT_NDX], "%lu.%c%c", &l, &c1, &c2);
			ldec = (c1 ? c1 - '0' : 0) * 10 + (c2 ? c2 - '0' : 0);
			cencyPage = l * 100L + ldec;
		}

		RestoreColorsNPointer (MainWindow);
		return RTsuccess;
	}
} /* ChoosePrinterReq */

