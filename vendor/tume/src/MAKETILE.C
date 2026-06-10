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
 * MAKETILE.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 08/24/94
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Dialog box to make a new tile.
 *
 * HISTORY
 *		08/24/94 (dcc) - created (based on RM_TSINF.C).
 *
*/

#include <echidna/platform.h>
#include "switches.h"
#include "switch1.h"

#if PixelLayers
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
#include "maketile.h"						/* Verify function prototypes. */

#define LOCAL static
/**************************** C O N S T A N T S ***************************/

#define RT_WHITE	0
#define RT_BLACK	1
#define RT_BLUE		2
#define RT_ORANGE	3

#define RTDLG_WIDTH		299
#define RTDLG_HEIGHT	91
#if 0
#define RTDLG_HEIGHT	141
#endif
#define RTDLG_LEFT		((SCREEN_WIDTH / 2) - (RTDLG_WIDTH / 2))
#define RTDLG_TOP		((((SCREEN_HEIGHT - 12) / 2) - (RTDLG_HEIGHT / 2)) + 12)

#if 0
#define TS_HEIGHT		141
#define TS_TOP			((((SCREEN_HEIGHT - 12) / 2) - (TS_HEIGHT / 2)) + 12)
#endif

#define RTDLG_TLST_LEFT		3
#define RTDLG_TLST_TOP		14
#define RTDLG_TLST_WIDTH	122
#define RTDLG_TLST_HEIGHT	74
#define RTDLG_TLST_ARROW_W	12
#define RTDLG_TLST_NAME_W	(RMDLG_TLIST_WIDTH - 1 - RMDLG_TLST_ARROW_W)

#if 0
#define RT_NAME_LEFT		3
#define RT_NAME_TOP			14
#define RT_NAME_WIDTH		292
#define RT_NAME_HEIGHT		11
#endif

#define ARROW_WIDTH			11
#define ARROW_HEIGHT		11
#define NUMBER_WIDTH		57
#define NUMBER_HEIGHT		11
#define NUMBER2_WIDTH		136
#define NUMBER2_HEIGHT		11

#define RTDLG_NUM_LEFT		216
#define RTDLG_NUM2_LEFT		127
#if 0
#define RTDLG_NUM_LEFT		127
#endif
#define RT_TYPE_TOP			14
#define RT_NUM2_TOP			38
#if 0
#define RT_NUM_TOP			41
#define RT_WIDTH_TOP			55
#define RT_HEIGHT_TOP		69
#define NTI_TOP				83
#define NTI_WIDTH			9
#define	NTI_HEIGHT			9
#define LABEL_LEFT			(RTDLG_NUM_LEFT + ARROW_WIDTH * 2 + NUMBER_WIDTH)
#endif
#define LABEL_LEFT			127

#define CANCEL_LEFT			198
#define CANCEL_TOP			OK_TOP
#define CANCEL_WIDTH		49
#define CANCEL_ID			10
#define OK_LEFT				(CANCEL_LEFT + CANCEL_WIDTH + 2)
#define	OK_TOP				76
#define OK_WIDTH			43
#define OK_ID				11
#define BUTTON_HEIGHT		12
#if 0
#define TSET_TOP			103
#define	TSET_HEIGHT			37

#define TS_COM_LEFT			3
#define	TS_COM1_TOP			113
#define TS_COM2_TOP			126
#endif


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

LOCAL Window	*RTWin;
LOCAL Font		*RTFont;
LOCAL Font		*RTSFont;
LOCAL UBYTE		 RtColors[4];

LOCAL short		 RTdone;
LOCAL short		 RTsuccess;

LOCAL short		 RTTop;
LOCAL ListType	*RTList;
LOCAL LGAD		*RTlgad;

LOCAL UWORD		 uwTileNumber = 0;
LOCAL UWORD		 uwFlagNTileset = 0;
#if 0
LOCAL short		 RoomWidth = 20;
LOCAL short		 RoomHeight = 12;
#endif

//LOCAL ListType	 TileTypeListX;
//static ListType	*TileTypeList = &TileTypeListX;
//LOCAL ListType	 RoomTypeListX;
//static ListType	*RoomTypeList = &RoomTypeListX;

LOCAL ListType	 ObjectTypeListX;
ListType			*plstObjectType = &ObjectTypeListX;

//static char			*RoomTypeGroup = "No Room Type Group";
//static char			*TileTypeGroup = "No Tile Type Group";

/******************************* M A C R O S ******************************/

#define ED_Button(x,y,w,h)	\
	ED_Rect ((x), (y), (w) - 1, (h) -1),				\
	ED_RelHLine ((x) + 1, (w) - 1, (y) + (h) - 1),		\
	ED_RelVLine ((x) + (w) - 1, (y) + 1, (h) - 1)		\

/******************************* T A B L E S ******************************/

LOCAL char *s0 = "Okay";
LOCAL char *s1 = "Cancel";
#if 0
LOCAL char *s2R= "Room Comments";
LOCAL char *s2T= "Tile Comments";
#endif
LOCAL char *s3 = "Save Grafx";

#if 0
#define szRoomComments &s2R
#define szTileComments &s2T
#endif

LOCAL char **RtStr[] = {
#define OK_STR	0
	&s0,
#define CANCEL_STR	1
	&s1,
#if 0
#define COMMENT_STR	2
	&s2T,
#endif
#define SAVE_STR	3
	&s3,
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

LOCAL EPic Number2EPic[] = {
	ED_FgColorTbl (RT_BLACK),
	ED_DrawMode (JAM1),
	ED_Rect (0, 0, NUMBER2_WIDTH - 2, NUMBER2_HEIGHT),
#if 0
	ED_Rect (ARROW_WIDTH - 1, 0, NUMBER2_WIDTH, ARROW_HEIGHT),
#endif

	ED_End,
};

LOCAL EPic *RtEPics[] = {
#define NUMBER_EPIC	0
	NumberEPic,
#define NUMBER2_EPIC	1
	Number2EPic,
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

#if 0
	/* string gad */
	ED_Rect (RT_NAME_LEFT, RT_NAME_TOP, RT_NAME_WIDTH, RT_NAME_HEIGHT),
#endif

	/* Numbers */
	ED_Move (RTDLG_NUM_LEFT, RT_TYPE_TOP),
	ED_Gosub (NUMBER_EPIC),
	ED_Move (RTDLG_NUM2_LEFT, RT_NUM2_TOP),
	ED_Gosub (NUMBER2_EPIC),
#if 0
	ED_Move (RTDLG_NUM_LEFT, RT_NUM_TOP),
	ED_Gosub (NUMBER_EPIC),
#endif

	/* Okay/Cancel */
	ED_Button (OK_LEFT, OK_TOP, OK_WIDTH, BUTTON_HEIGHT),
	ED_PosText (OK_LEFT + 2, OK_TOP + 2, OK_STR),
	ED_Button (CANCEL_LEFT, CANCEL_TOP, CANCEL_WIDTH, BUTTON_HEIGHT),
	ED_PosText (CANCEL_LEFT + 2, CANCEL_TOP + 2, CANCEL_STR),

#if 0
	/* Box for comment string gads */
	ED_Font (1),
	ED_DrawMode (JAM2),
	ED_FgColorTbl (RT_WHITE),
	ED_Rect (0, TSET_TOP + 1, RTDLG_WIDTH - 1, TSET_HEIGHT - 2),
	ED_DrawMode (JAM1),
	ED_FgColorTbl (RT_BLACK),
	ED_Button (0, TSET_TOP, RTDLG_WIDTH, TSET_HEIGHT),

	/* comment string gads */
	ED_Rect (TS_COM_LEFT, TS_COM1_TOP, RT_NAME_WIDTH, RT_NAME_HEIGHT),
	ED_Rect (TS_COM_LEFT, TS_COM2_TOP, RT_NAME_WIDTH, RT_NAME_HEIGHT),
	ED_PosText (4, TSET_TOP + 3, COMMENT_STR),
#endif

	ED_End,
};

#if 0
LOCAL EPic TSetEPic[] = {
	/* NTI thing */
	ED_Font (1),
	ED_DrawMode (JAM1),
	ED_FgColorTbl (RT_BLACK),
	ED_Rect (RTDLG_NUM_LEFT, NTI_TOP, NTI_WIDTH, NTI_HEIGHT),
	ED_PosText (RTDLG_NUM_LEFT + NTI_WIDTH + 2, NTI_TOP + 2, SAVE_STR),

	ED_End,
};
#endif

LOCAL EPicture RTEPicture = {
	RTEPic,
	RtFonts,
	RtStr,
	RtEPics,
	NULL,
	RtColors,
};

#if 0
LOCAL EPicture NumEPicture = {
	NumberEPic,
	RtFonts,
	RtStr,
	RtEPics,
	NULL,
	RtColors,
};

LOCAL EPicture TSetEPicture = {
	TSetEPic,
	RtFonts,
	RtStr,
	RtEPics,
	NULL,
	RtColors,
};
#endif
	

#define NUM_TILENUM_NDX	0
#define NUM_FLAG_NDX	1
#if 0
#define NUM_WIDTH_NDX	2
#define NUM_HEIGHT_NDX	3
#define NUM_COM1_NDX	4
#define NUM_COM2_NDX	5
#define NUM_NAME_NDX	6

LOCAL short SaveGrafx;

LOCAL char	Naem[EIO_MAXPATH];
LOCAL char	Com1[EIO_MAXPATH];
LOCAL char	Com2[EIO_MAXPATH];
#endif
LOCAL char	NumStrings[2][EIO_MAXPATH];

#define NUM_STRING_GADS	2
#if 0
#define NUM_STRING_GADS	7
#endif

#if 0
LOCAL short StrHeights[] = {
	RT_TYPE_TOP + 2, 
	RT_NUM_TOP + 2,
	RT_WIDTH_TOP + 2,
	RT_HEIGHT_TOP + 2,
	TS_COM1_TOP + 2,
	TS_COM2_TOP + 2,
 	RT_NAME_TOP + 2,
};

LOCAL short NewGadWidths[] = {
	ARROW_WIDTH, 
	NUMBER_WIDTH - 2,
	ARROW_WIDTH, 
	ARROW_WIDTH,
	NUMBER_WIDTH - 2,
	ARROW_WIDTH,
	NTI_WIDTH,
	RT_NAME_WIDTH,
	RT_NAME_WIDTH,
};
#endif

LOCAL EditString NumString[NUM_STRING_GADS] = {
{ NumStrings[0], (NUMBER_WIDTH - 5) / 8, RTDLG_LEFT + RTDLG_NUM_LEFT + ARROW_WIDTH + 1, RTDLG_TOP + RT_TYPE_TOP + 2, NULL, &NumString[1], },
{ NumStrings[1], (NUMBER2_WIDTH - 5) / 8, RTDLG_LEFT + RTDLG_NUM2_LEFT + 2, RTDLG_TOP + RT_NUM2_TOP + 2, NULL, &NumString[0], },
#if 0
{ NumStrings[0], (NUMBER_WIDTH - 5) / 8, RTDLG_LEFT + RTDLG_NUM_LEFT + ARROW_WIDTH + 1, RTDLG_TOP + RT_TYPE_TOP + 2, NULL, &NumString[1], },
{ NumStrings[1], (NUMBER_WIDTH - 5) / 8, RTDLG_LEFT + RTDLG_NUM_LEFT + ARROW_WIDTH + 1, RTDLG_TOP + RT_NUM_TOP + 2, NULL, &NumString[2], },
{ NumStrings[2], (NUMBER_WIDTH - 5) / 8, RTDLG_LEFT + RTDLG_NUM_LEFT + ARROW_WIDTH + 1, RTDLG_TOP + RT_WIDTH_TOP + 2, NULL, &NumString[3],},
{ NumStrings[3], (NUMBER_WIDTH - 5) / 8, RTDLG_LEFT + RTDLG_NUM_LEFT + ARROW_WIDTH + 1, RTDLG_TOP + RT_HEIGHT_TOP + 2, NULL, &NumString[4], },
{ Com1, (RT_NAME_WIDTH - 5) / 8, RTDLG_LEFT + TS_COM_LEFT + 2, RTDLG_TOP + TS_COM1_TOP + 2, NULL, &NumString[5], },
{ Com2, (RT_NAME_WIDTH - 5) / 8, RTDLG_LEFT + TS_COM_LEFT + 2, RTDLG_TOP + TS_COM2_TOP + 2, NULL, &NumString[6], },
{ Naem, (RT_NAME_WIDTH - 5) / 8, RTDLG_LEFT + RT_NAME_LEFT + 2, RTDLG_TOP + RT_NAME_TOP + 2, NULL, &NumString[0], },
#endif
};

typedef EditString	*ESPtr;

#if 0
LOCAL ESPtr ESTable[2][NUM_STRING_GADS] = {
{ &NumString[1], &NumString[2], &NumString[3], &NumString[4], &NumString[5], &NumString[6], &NumString[0], }, /* fNew */
{ &NumString[1], &NumString[4], NULL         , NULL         , &NumString[5], &NumString[6], &NumString[0], }, /*      */
};
#endif

/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * UpdateListMark
 *
 * SYNOPSIS
 *		LOCAL void UpdateListMark (void)
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
		OBJ_Type	*pobj;
		short	 type;
		short	 count;
		short	 pos = (-1);

		count = 0;
		type  = atoi (NumStrings[NUM_TILENUM_NDX]);
		pobj    = (OBJ_Type *) Head (RTList);
		while (!EndOfList (pobj))
		{
			pobj->Node.Priority = 0;
			if (pobj->Type == type)
			{
				type = -30000;
				pos  = count;
				pobj->Node.Priority |= LGAD_NOD_SELECTED;
			}
			count++;
			pobj = (OBJ_Type *) Next (pobj);
		}

		if (pos < 0)
		{
			SetLGADList (RTlgad, RTList, 1000);
		}
		else
		{
			SetLGADListPos (RTlgad, pos, LGAD_POS_MID);
		}
	}

} /* UpdateListMark */


/*********************************************************************
 *
 * RedrawList
 *
 * SYNOPSIS
 *		void RedrawList (void)
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
static void RedrawList (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RedrawList";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	UpdateListMark ();
	/*
	SetLGADListPos (RTlgad, UpdateListMark(), LGAD_POS_MID);
	UpdateListMark ();
	SetLGADList (RTlgad, RTList, 1000);
	*/

} /* RedrawList */


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
		UWORD		 val;

		val  = atoi (NumStrings[num]);
		val += dir;
#if 0
		val  = max (0, val);
		val  = min (32766, val);

		itoa (val, NumStrings[num], 10);
#endif

		sprintf(NumStrings[num], "%u", val);
		BeforeGraphics ();
		UpdateEditString (&NumString[num]);
		if (num == NUM_TILENUM_NDX) {
			RedrawList ();
		}
		AfterGraphics ();

	}
} /* IncDecNumber */


#if 0
/*********************************************************************
 *
 * UpdateNTI
 *
 * SYNOPSIS
 *		void UpdateNTI (void)
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
void UpdateNTI (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateNTI";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		static short xy[][2] = {
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 1, NTI_TOP + 1, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 2, NTI_TOP + 2, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 3, NTI_TOP + 3, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 4, NTI_TOP + 4, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 5, NTI_TOP + 5, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 6, NTI_TOP + 6, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 7, NTI_TOP + 7, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 7, NTI_TOP + 1, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 6, NTI_TOP + 2, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 5, NTI_TOP + 3, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 4, NTI_TOP + 4, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 3, NTI_TOP + 5, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 2, NTI_TOP + 6, },
			{ RTDLG_LEFT + RTDLG_NUM_LEFT + 1, NTI_TOP + 7, },
			{ -1, },
		};
		short	i = 0;

		BeforeGraphics ();

		SetPenColor (RtColors[RT_WHITE]);
		DrawRect (
				RTDLG_LEFT + RTDLG_NUM_LEFT + 1,
				RTTop  + NTI_TOP + 1,
				NTI_WIDTH - 2,
				NTI_HEIGHT - 2);

		if (SaveGrafx) {
			SetPenColor (RtColors[RT_BLACK]);
			while (xy[i][0] >= 0) {
				DrawPixel (xy[i][0], RTTop + xy[i][1]);
				i++;
			}
		}

		AfterGraphics ();
	}

} /* UpdateNTI */

/*********************************************************************
 *
 * ToggleNTI
 *
 * SYNOPSIS
 *		LOCAL void ToggleNTI (void) 
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
LOCAL void ToggleNTI (void) 
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleNTI";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SaveGrafx = !SaveGrafx;
	UpdateNTI ();

} /* ToggleNTI */
#endif


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
 CurrentFuncName = "void	SelectFunc";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	fWhy = fWhy;
	{
		LGAD *plgad = (LGAD *) pvdata;
		OBJ_Type	*pnod = (OBJ_Type *) pnodLGADFirstSelected (plgad);

		if (pnod)
		{
#if 0
			itoa (pnod->Type, NumStrings[0], 10);
#endif
			sprintf(NumStrings[0], "%u", pnod->Type);
			UpdateEditString (&NumString[0]);
		}
	}
} /* void SelectFunc */


/********************************* Gadgets ********************************/
	
LOCAL HitArea RTHitArea[] = {
#if 0
	{ RT_NAME_LEFT, RT_NAME_TOP, RT_NAME_WIDTH, RT_NAME_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, HighlightString, &NumString[NUM_NAME_NDX], },
#define LIST_GAD	1
#endif
#define LIST_GAD	0
	{ RTDLG_TLST_LEFT, RTDLG_TLST_TOP, RTDLG_TLST_WIDTH, RTDLG_TLST_HEIGHT,
		HTA_SUBAREAS, 0, NULL, NULL, },

	/* Type Number */
	{ RTDLG_NUM_LEFT, RT_TYPE_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x10, IncDecNumber, },
	{ RTDLG_NUM_LEFT + ARROW_WIDTH, RT_TYPE_TOP, NUMBER_WIDTH - 2, NUMBER_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, HighlightString, &NumString[NUM_TILENUM_NDX], },
	{ RTDLG_NUM_LEFT + ARROW_WIDTH + NUMBER_WIDTH - 1, RT_TYPE_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x00, IncDecNumber, },

	/* IDNum Number */
#if 0
	{ RTDLG_NUM_LEFT, RT_NUM2_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x11, IncDecNumber, },
#endif
	{ RTDLG_NUM2_LEFT, RT_NUM2_TOP, NUMBER2_WIDTH - 2, NUMBER2_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, HighlightString, &NumString[NUM_FLAG_NDX], },
#if 0
	{ RTDLG_NUM_LEFT + ARROW_WIDTH + NUMBER_WIDTH - 1, RT_NUM2_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x01, IncDecNumber, },

	/* Width Number */
#define WIDTH_GADS	8
	{ RTDLG_NUM_LEFT, RT_WIDTH_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x12, IncDecNumber, },
	{ RTDLG_NUM_LEFT + ARROW_WIDTH, RT_WIDTH_TOP, NUMBER_WIDTH - 2, NUMBER_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, HighlightString, &NumString[NUM_WIDTH_NDX], },
	{ RTDLG_NUM_LEFT + ARROW_WIDTH + NUMBER_WIDTH - 1, RT_WIDTH_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x02, IncDecNumber, },

#define HEIGHT_GADS	11
	/* Height Number */
	{ RTDLG_NUM_LEFT, RT_HEIGHT_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x13, IncDecNumber, },
	{ RTDLG_NUM_LEFT + ARROW_WIDTH, RT_HEIGHT_TOP, NUMBER_WIDTH - 2, NUMBER_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, HighlightString, &NumString[NUM_HEIGHT_NDX], },
	{ RTDLG_NUM_LEFT + ARROW_WIDTH + NUMBER_WIDTH - 1, RT_HEIGHT_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x03, IncDecNumber, },

#define NTI_GAD	14
	/* NTI Gadget */
	{ RTDLG_NUM_LEFT, NTI_TOP, NTI_WIDTH, NTI_HEIGHT,
		0, 0, ToggleNTI, },

#define COMMENT_GADS	15
	/* Comment Gads */
	{ TS_COM_LEFT, TS_COM1_TOP,  RT_NAME_WIDTH, RT_NAME_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, HighlightString, &NumString[NUM_COM1_NDX], },
	{ TS_COM_LEFT, TS_COM2_TOP,  RT_NAME_WIDTH, RT_NAME_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, HighlightString, &NumString[NUM_COM2_NDX], },
#endif

	/* Ok / Cancel */
	{ OK_LEFT, OK_TOP, OK_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, OK_ID, NULL},
	{ CANCEL_LEFT, CANCEL_TOP, CANCEL_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, CANCEL_ID, NULL},
	{ HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, },
};

/*********************************************************************
 *
 * MakeTileReq
 *
 * PURPOSE
 *		Make a new tile, and place the value in <pplt>.
 *
 * INPUT
 *		PlotType *pplt	: input values, location to store results
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		TRUE if user pressed OK, FALSE if user canceled.
 *
 * HISTORY
 *		08/24/94 (dcc) - created (based on RoomNTileInfoReq()).
 *
*/
int MakeTileReq (PlotType *pplt)
{
	char szTitle[256];

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MakeTileReq";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		SaveAreaType		*sa = NULL;
		HitArea				*ha;
		struct	RastPort	 rp = { NULL, };
		short				 height;
		short				 top;
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

		top    = RTDLG_TOP;
		height = RTDLG_HEIGHT;
		RTTop = top;
		RTList = plstObjectType;

		uwTileNumber	= pplt->Tile_ID;
		uwFlagNTileset	= (pplt->Plot_Flags << 8) | pplt->TileSet_ID;

		if (uwTileNumber == 0 && uwFlagNTileset == 0)
		{
			uwTileNumber = 1;
			strcpy(szTitle, "Make Tile");
		}
		else
		{
			strcpy(szTitle, "Edit Tile");
		}
		sprintf(NumStrings[NUM_TILENUM_NDX], "%u", uwTileNumber);
		{
			char sz[256];

			itoa (uwFlagNTileset, sz, 2);
			strcpy (NumStrings[NUM_FLAG_NDX], "0000000000000000");
			strcpy (NumStrings[NUM_FLAG_NDX]+(16-strlen(sz)), sz);
		}

		SetColorsNPointer (MainWindow);

		if (PushWindowClipValues (RTWin))
		{
			sa = SaveArea (RTDLG_LEFT, top, RTDLG_WIDTH, height);
			if (!sa)
			{
				GEcatf ("\nOOM: FileReq Area");
				goto dRTcleanup;
			}

			if (InitTextLGAD (
					lgad,
					RTDLG_TLST_WIDTH,
					RTDLG_TLST_HEIGHT,
					RTList,
					LGAD_V_ELEVATOR | LGAD_V_REALTIME_SLIDER | LGAD_EXCLUSIVE,
					RTDLG_TLST_ARROW_W,
					20,20,
					1000,
					SelectFunc,
					lgad))
			{

				RTlgad->x = RTDLG_LEFT + RTDLG_TLST_LEFT;	//dcc KLUDGE
				RTlgad->y = top + RTDLG_TLST_TOP;			//dcc KLUDGE
				UpdateListMark ();

				RTHitArea[LIST_GAD].Data = phaLGAD(lgad);

				BeforeGraphics ();

				StartEPicture (&rp, &RTEPicture, RTDLG_LEFT, top);

				DrawLGAD (lgad, RTDLG_LEFT + RTDLG_TLST_LEFT, top + RTDLG_TLST_TOP);

				DrawString (RTFont, RTDLG_LEFT + LABEL_LEFT, top + RT_TYPE_TOP + 2, "Object Type");
				DrawString (RTFont, RTDLG_LEFT + LABEL_LEFT, top + RT_NUM2_TOP - 10, "Flag Bits:");

				DrawString (RTFont, RTDLG_LEFT + RTDLG_WIDTH / 2 - StringLen (RTFont, "Edit Tile") / 2, top + 2, szTitle);

				UpdateStrings (MainWindow, &NumString[0], 5, NULL);

				AfterGraphics ();

				while (!RTdone)
				{
					short	status;
					short	key;

					ha = CheckHitAreas (RTHitArea, RTDLG_LEFT, top);
					if (ha)
					{
						if (ha->ID == OK_ID)
						{
							RTdone    = TRUE;
							RTsuccess = TRUE;
							break;
						}
						if (ha->ID == CANCEL_ID)
						{
							RTdone  = TRUE;
							break;
						}
					}
					status = CheckEditKeys (&key);
					if (status)
					{
						RTdone = TRUE;
						if (status >= 0)
						{
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
			uwTileNumber		= atoi(NumStrings[NUM_TILENUM_NDX]);
			{
				int i, cnt;
				char *c;

				c = NumStrings[NUM_FLAG_NDX];
				cnt = strlen(NumStrings[NUM_FLAG_NDX]);

				uwFlagNTileset = 0;
				for (i = 0; i < cnt; i++)
				{
					uwFlagNTileset <<= 1;

					if (*c == '1')
						uwFlagNTileset += 1;
					c++;
				}
			}
			pplt->Plot_Flags	= uwFlagNTileset >> 8;
			pplt->TileSet_ID	= uwFlagNTileset & 0xFF;
			pplt->Tile_ID		= uwTileNumber;
		}

		RestoreColorsNPointer (MainWindow);
		return RTsuccess;
	}
} /* MakeTileReq */
#endif // PixelLayers

