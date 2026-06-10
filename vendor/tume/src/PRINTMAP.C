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
 * PRINTMAP.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 04/08/92
 *   MODIFIED : 03/26/95
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Routines to print a map.
 *
 *		Note somewhat confusing terminology:
 *		We talk about width (cx) and height (cy) of page in portrait mode,
 *		but about width (cx) and height (cy) of map in landscape mode. Ugh.
 *
 * HISTORY
 *		01/16/93 Saturday (dcc)	 - dynamically allocate data structures.
 *		08/11/93 Wednesday (dcc) - fix not dealing with separated tiles
 *								   properly. Fix not setting sizes properly
 *								   using the SPECIFY SIZE BY PAGES option.
 *		08/17/93 Tuesday (dcc)	 - fix to handle numbers such as "1.08"
 *		08/18/93 Wednesday (dcc) - fix so you can increment numbers like "1.08"
 *								 - scale increment amount to size of number
 *		08/18/93 Wednesday (dcc) - specify to nearest 0.01 inch (was 0.02)
 *								 - support different page sizes
 *								 - dynamically allocate LineBuf as well
 *		08/19/93 Thursday (dcc) - add support for HP-GL/2 color devices
 *		08/30/93 Monday (dcc)	- add HP-GL/2 support for UseLightShades
 *		09/07/93 Tuesday (dcc) - HP-GL/2: force white to palette slot 0
 *		09/22/93 Wednesday (dcc) - BUG FIX: refer to PRINT_PIXELS_DOWN
 *									instead of PRINT_BUFFER_SIZE (overwrite buffer)
 *		09/30/93 Thursday (dcc) - change Lightener from 20 to 40
 *		10/26/93 Tuesday (dcc) - change HP-GL/2 UseLightShades: Gamma correct 0.45
 *		11/11/93 Thursday (dcc) - turn PRINT_PIXELS_ACROSS into (long)
 *		11/15/93 Monday (dcc) - updated for MS-DOS Watcom C32.
 *		07/08/94 Friday (dcc) - don't display UNIFORM PIXELS option
 *		07/08/94 Friday (dcc) - add support to print in portrait mode.
 *
 * TODO
 *		* Make Size Editing more consistant.
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include <stdlib.h>
#include "tuglbl.h"
#include "tumedraw.h"

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
#include "printmap.h"
#include "colorseq.h"

#define LOCAL static

#if TEST_PRINT
#include <echidna/grafx.h>
#include <echidna/fonts.h>

#define DS(msg)	\
	SetPenColor (255);						\
	DrawRect (10, 150, 25*8, 8);			\
 	DrawString (&MainFont, 10, 150, msg);	\

extern Font MainFont;
extern short ExitProgram;
#else
#include "events.h"
#endif

#include "switch1.h"
#include "assert.h"

/**************************** C O N S T A N T S ***************************/

/* Shading methods */

#define RGBAVE_MODE			0
#define V2S_MODE			1
#define COLORCORRECT_MODE	2

/* Page size specification methods */

#define PAGES_MODE				0
#define INCHES_MODE				1
#define TILES_PER_PAGE_MODE		2
#define INCHES_PER_TILE_MODE	3

/* Maximum width of page in 0.01 inches for HPGL2 device */

#define cencxPageMaxHPGL2				3227L

/* Default size of page in 0.01 inches */

#define cencxPageDefault				850L
#define cencyPageDefault				1100L

/* Default border size in 0.01 inches */

#define cencxBorderHPPCL3				100L
#define cencyBorderHPPCL3				100L

/* Default border size in 0.01 inches */

#define cencxBorderHPGL2				127L
#define cencyBorderHPGL2				224L

#define	DPI_150							150
#define	DPI_300							300

/* Height of idiot numbers in 0.01 inches */

#define cencyIdiot						50L

#define PRINT_BUFFER_SIZE		2250

#define RT_WHITE	0
#define RT_BLACK	1
#define RT_BLUE		2
#define RT_ORANGE	3

#define WAIT_WIDTH		175
#define WAIT_HEIGHT		60
#define WAIT_LEFT		((SCREEN_WIDTH / 2) - (WAIT_WIDTH / 2))
#define WAIT_TOP		((((SCREEN_HEIGHT - 12) / 2) - (WAIT_HEIGHT / 2)) + 12)

#define BAR_LEFT			8
#define BAR_TOP				29
#define BAR_WIDTH			158
#define BAR_HEIGHT			12

#define	RTDLG_WIDTH			271
#define RTDLG_HEIGHT		142
#define RTDLG_LEFT			((SCREEN_WIDTH / 2) - (RTDLG_WIDTH / 2))
#define RTDLG_TOP			((((SCREEN_HEIGHT - 12) / 2) - (RTDLG_HEIGHT / 2)) + 12)

#define ARROW_WIDTH			11
#define ARROW_HEIGHT		11
#define NUMBER_WIDTH		57
#define NUMBER_HEIGHT		11

#define	RTNUM_LEFT			8
#define	RTNUM1_TOP			84
#define	RTNUM2_TOP			(RTNUM1_TOP + 14)

#define BUTTON_HEIGHT		12
#define CANCEL_LEFT			(PRINT_LEFT + PRINT_WIDTH + 2)
#define CANCEL_TOP			PRINT_TOP
#define CANCEL_WIDTH		49
#define CANCEL_ID			10
#define PRINT_LEFT			8
#define	PRINT_TOP			124
#define PRINT_WIDTH			43
#define PRINT_ID			11

#define	RADIO_HEIGHT		9
#define RADIO_WIDTH			9

#define	SHADES_TOP			19
#define SHADES_LEFT			149
#define	SHADES_WIDTH		113
#define SHADES_HEIGHT		39
#define SHADES_TITLE		70

#define ORIENTATION_TOP		68
#define ORIENTATION_LEFT	149
#define ORIENTATION_WIDTH	72
#define ORIENTATION_HEIGHT	29
#define ORIENTATION_TITLE	63

#define RES_TOP				68
#define RES_LEFT			226
#define RES_WIDTH			36
#define RES_HEIGHT			29
#define RES_TITLE			19

#define SIZE_TOP			19
#define SIZE_LEFT			8
#define SIZE_WIDTH			130
#define SIZE_HEIGHT			49
#define SIZE_TITLE			85

#define CHECK_WIDTH			9
#define CHECK_HEIGHT		9

#define OPTION1_TOP			107
#define OPTION1_LEFT		157
#define OPTION1_WIDTH		100

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

short OutputDevice = HPPCL3;

long		 cencxPage = cencxPageDefault; /* width of page in 0.01 inches */
long		 cencyPage = cencyPageDefault; /* height of page in 0.01 inches */

static UBYTE	*PrintBuffer = NULL;
static UBYTE	*LineBuf = NULL;
static UBYTE	*cLookup;

static ByteMap		PrintBM = {
	1, PRINT_BUFFER_SIZE, NULL/*PrintBuffer*/, 0,
};

#if fDoSaveRooms
static FILE	*PrintFP;
#endif/*fDoSaveRooms*/

#if TEST_PRINT
char			*PrintFile = "D:\\TEMP\\P.TST";
#else
extern char		PrintFile[];
#endif

short			 wPrtSrcX  = 1;
short			 wPrtSrcY  = 1;
short			 wPrtDestX = 1;
short			 wPrtDestY = 1;

#if fDoSaveRooms
LOCAL UBYTE			 PrintColors[MAX_CI_COLORS];
#endif/*fDoSaveRooms*/

LOCAL Window	*RTWin;
LOCAL Font		*RTFont;
LOCAL Font		*RTSFont;
LOCAL UBYTE		 RtColors[4];

LOCAL short		 RTdone;
LOCAL short		 RTsuccess;

short		 PrintColorMode = COLORCORRECT_MODE;
short		 UseLightShades = TRUE;
short		 FirstPageOnly  = FALSE;
#if 0
short		 UniformPixels  = FALSE;
#endif
short		 SizeMode       = PAGES_MODE;
short		 IdiotNumbers	= TRUE;

LOCAL short		 DownMajor = 1;

LOCAL RoomWindowType	*PrintRoomWindow;

LOCAL short		 SrcSkip;
LOCAL short		 DstDup;

LOCAL short		 pagesacross = 1;
LOCAL short		 pagesdown;
LOCAL short		 tilesacrosspage;
LOCAL short		 tilesdownpage;

LOCAL long		 cencxBorder = cencxBorderHPPCL3;	/* width of border in 0.01 inches */
LOCAL long		 cencyBorder = cencyBorderHPPCL3;	/* height of border in 0.01 inches */

LOCAL long		 cencinPrintA;				/* width of map in 0.01 inches */
LOCAL long		 cencinPrintD;				/* height of map in 0.01 inches */

LOCAL long		 cencinTileA;				/* width of tile in 0.01 inches */
LOCAL long		 cencinTileD;				/* height of tile in 0.01 inches */

LOCAL short		 fPrintPortrait				= FALSE;
LOCAL short		 PRINT_DPI					= DPI_150;
LOCAL long		 PRINT_PIXELS_DOWN			= (cencxPageDefault - cencxBorderHPPCL3) * (long) DPI_150 / 100L;
LOCAL long		 PRINT_PIXELS_ACROSS		= (cencyPageDefault - cencyBorderHPPCL3) * (long) DPI_150 / 100L;
LOCAL long		 sizeofBufferCurrent		= 0L;	/* sizeof(PrintBuffer) */
LOCAL long		 sizeofPrintBufferNew		= (cencxPageDefault - cencxBorderHPPCL3) * (long) DPI_150 / 100L;

#if fDoSaveRooms
LOCAL UBYTE GammaCorrect[256] =
{
0x00,0x15,0x1d,0x23,0x27,0x2b,0x2f,0x33,
0x36,0x39,0x3b,0x3e,0x40,0x43,0x45,0x47,
0x49,0x4b,0x4d,0x4f,0x51,0x53,0x55,0x56,
0x58,0x5a,0x5b,0x5d,0x5e,0x60,0x61,0x63,
0x64,0x66,0x67,0x68,0x6a,0x6b,0x6c,0x6e,
0x6f,0x70,0x71,0x72,0x74,0x75,0x76,0x77,
0x78,0x79,0x7a,0x7c,0x7d,0x7e,0x7f,0x80,
0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,
0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,0x8f,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x96,
0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9c,0x9d,
0x9e,0x9f,0xa0,0xa0,0xa1,0xa2,0xa3,0xa4,
0xa4,0xa5,0xa6,0xa7,0xa7,0xa8,0xa9,0xaa,
0xaa,0xab,0xac,0xad,0xad,0xae,0xaf,0xaf,
0xb0,0xb1,0xb2,0xb2,0xb3,0xb4,0xb4,0xb5,
0xb6,0xb6,0xb7,0xb8,0xb8,0xb9,0xba,0xba,
0xbb,0xbc,0xbc,0xbd,0xbe,0xbe,0xbf,0xc0,
0xc0,0xc1,0xc1,0xc2,0xc3,0xc3,0xc4,0xc5,
0xc5,0xc6,0xc6,0xc7,0xc8,0xc8,0xc9,0xc9,
0xca,0xcb,0xcb,0xcc,0xcc,0xcd,0xce,0xce,
0xcf,0xcf,0xd0,0xd0,0xd1,0xd2,0xd2,0xd3,
0xd3,0xd4,0xd4,0xd5,0xd6,0xd6,0xd7,0xd7,
0xd8,0xd8,0xd9,0xd9,0xda,0xdb,0xdb,0xdc,
0xdc,0xdd,0xdd,0xde,0xde,0xdf,0xdf,0xe0,
0xe0,0xe1,0xe1,0xe2,0xe3,0xe3,0xe4,0xe4,
0xe5,0xe5,0xe6,0xe6,0xe7,0xe7,0xe8,0xe8,
0xe9,0xe9,0xea,0xea,0xeb,0xeb,0xec,0xec,
0xed,0xed,0xee,0xee,0xef,0xef,0xf0,0xf0,
0xf1,0xf1,0xf2,0xf2,0xf2,0xf3,0xf3,0xf4,
0xf4,0xf5,0xf5,0xf6,0xf6,0xf7,0xf7,0xf8,
0xf8,0xf9,0xf9,0xfa,0xfa,0xfa,0xfb,0xfb,
0xfc,0xfc,0xfd,0xfd,0xfe,0xfe,0xff,0xff,
};
#endif/*fDoSaveRooms*/

/******************************* M A C R O S ******************************/

#define ED_Button(x,y,w,h)	\
	ED_Rect ((x), (y), (w) - 1, (h) -1),				\
	ED_RelHLine ((x) + 1, (w) - 1, (y) + (h) - 1),		\
	ED_RelVLine ((x) + (w) - 1, (y) + 1, (h) - 1)		\

/***************************** R O U T I N E S ****************************/


/******************************* M A C R O S ******************************/

/******************************* T A B L E S ******************************/

LOCAL char *s0  = "SPECIFY SIZE BY";
LOCAL char *s1  = "PAGES";
LOCAL char *s2  = "INCHES";
LOCAL char *s3  = "TILES PER PAGE";
LOCAL char *s4  = "INCHES PER TILE";
LOCAL char *s5  = "ACROSS";
LOCAL char *s6  = "DOWN";
LOCAL char *s7  = "Print";
LOCAL char *s8  = "Cancel";
LOCAL char *s9  = "SHADE METHOD";
LOCAL char *s10 = "RGB AVERAGE";
LOCAL char *s11 = "V";
LOCAL char *s12 = "2";
LOCAL char *s13 = "S";
LOCAL char *s14 = "COLOR CORRECT";
LOCAL char *s15 = "ORIENTATION";
LOCAL char *s16 = "PORTRAIT";
LOCAL char *s17 = "LANDSCAPE";
LOCAL char *s18 = "DPI";
LOCAL char *s19 = "150";
LOCAL char *s20 = "300";
#if 0
LOCAL char *s21 = "UNIFORM PIXELS";
#else
LOCAL char *s21 = "";
#endif
LOCAL char *s22 = "FIRST PAGE ONLY";
LOCAL char *s23 = "USE LIGHT SHADES";

LOCAL char **RtStr[] = {
#define SPEC_STR	0
	&s0,
#define PAGES_STR	1
	&s1,
#define INCHES_STR	2
	&s2,
#define TAP_STR		3
	&s3,
#define TWII_STR	4
	&s4,
#define ACROSS_STR	5
	&s5,
#define DOWN_STR	6
	&s6,
#define PRINT_STR	7
	&s7,
#define CANCEL_STR	8
	&s8,
#define SHADE_STR	9
	&s9,
#define RGB_STR		10
	&s10,
#define V_STR		11
	&s11,
#define TWO_STR		12
	&s12,
#define S_STR		13
	&s13,
#define COLOR_STR	14
	&s14,
#define ORIENTATION_STR	15
	&s15,
#define PORTRAIT_STR	16
	&s16,
#define LANDSCAPE_STR	17
	&s17,
#define RES_STR		18
	&s18,
#define R150_STR	19
	&s19,
#define R300_STR	20
	&s20,
#define UNI_STR		21
	&s21,
#define FIRST_STR	22
	&s22,
#define LIGHT_STR	23
	&s23,
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

LOCAL EPic CircleEPic[] = {
	ED_FgColorTbl (RT_BLACK),
	ED_DrawMode (JAM1),
	ED_RelHLine (3, 3, 0),
	ED_RelHLine (3, 3, 8),
	ED_RelVLine (0, 3, 3),
	ED_RelVLine (8, 3, 3),
	ED_Plot (1, 1),
	ED_Plot (1, 2),
	ED_Plot (2, 1),
	ED_Plot (6, 1),
	ED_Plot (7, 1),
	ED_Plot (7, 2),
	ED_Plot (1, 6),
	ED_Plot (1, 7),
	ED_Plot (2, 7),
	ED_Plot (6, 7),
	ED_Plot (7, 7),
	ED_Plot (7, 6),
	ED_End,
};

LOCAL EPic XEPic[] = {
	ED_FgColorTbl (RT_BLACK),
	ED_DrawMode (JAM1),
	ED_Plot (0, 0),
	ED_Plot (1, 1),
	ED_Plot (2, 2),
	ED_Plot (3, 3),
	ED_Plot (4, 4),
	ED_Plot (5, 5),
	ED_Plot (6, 6),
	ED_Plot (0, 6),
	ED_Plot (1, 5),
	ED_Plot (2, 4),
	ED_Plot (3, 3),
	ED_Plot (4, 2),
	ED_Plot (5, 1),
	ED_Plot (6, 0),
	ED_End,
};

LOCAL EPic MarkEPic[] = {
	ED_DrawMode (JAM2),
	ED_Rect (3, 2, 3, 5),
	ED_Rect (2, 3, 5, 3),
	ED_End,
};

LOCAL EPic *RtEPics[] = {
#define NUMBER_EPIC	0
	NumberEPic,
#define CIRCLE_EPIC	1
	CircleEPic,
};

LOCAL EPic RTEPic[] = {
	ED_Font (1),
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
	ED_Move (RTNUM_LEFT, RTNUM1_TOP),
	ED_Gosub (NUMBER_EPIC),
	ED_PosText (RTNUM_LEFT + 82, RTNUM1_TOP + 3, ACROSS_STR),
	ED_Move (RTNUM_LEFT, RTNUM2_TOP),
	ED_Gosub (NUMBER_EPIC),
	ED_PosText (RTNUM_LEFT + 82, RTNUM2_TOP + 3, DOWN_STR),

	/* Size */
	ED_Rect (SIZE_LEFT, SIZE_TOP, SIZE_WIDTH, SIZE_HEIGHT),
	ED_FgColorTbl (RT_WHITE),
	ED_RelHLine (SIZE_LEFT + 3, SIZE_TITLE, SIZE_TOP),
	ED_FgColorTbl (RT_BLACK),
	ED_Move (SIZE_LEFT + 6, SIZE_TOP + 6 + 10 * 0), ED_Gosub (CIRCLE_EPIC),
	ED_Move (SIZE_LEFT + 6, SIZE_TOP + 6 + 10 * 1), ED_Gosub (CIRCLE_EPIC),
	ED_Move (SIZE_LEFT + 6, SIZE_TOP + 6 + 10 * 2), ED_Gosub (CIRCLE_EPIC),
	ED_Move (SIZE_LEFT + 6, SIZE_TOP + 6 + 10 * 3), ED_Gosub (CIRCLE_EPIC),
	ED_PosText (SIZE_LEFT + 4, SIZE_TOP - 2, SPEC_STR),
	ED_PosText (SIZE_LEFT + 6 + 11, SIZE_TOP + 8 + 10 * 0, PAGES_STR),
	ED_PosText (SIZE_LEFT + 6 + 11, SIZE_TOP + 8 + 10 * 1, INCHES_STR),
	ED_PosText (SIZE_LEFT + 6 + 11, SIZE_TOP + 8 + 10 * 2, TAP_STR),
	ED_PosText (SIZE_LEFT + 6 + 11, SIZE_TOP + 8 + 10 * 3, TWII_STR),

	/* Shade */
	ED_Rect (SHADES_LEFT, SHADES_TOP, SHADES_WIDTH, SHADES_HEIGHT),
	ED_FgColorTbl (RT_WHITE),
	ED_RelHLine (SHADES_LEFT + 3, SHADES_TITLE, SHADES_TOP),
	ED_FgColorTbl (RT_BLACK),
	ED_Move (SHADES_LEFT + 6, SHADES_TOP + 6 + 10 * 0), ED_Gosub (CIRCLE_EPIC),
	ED_Move (SHADES_LEFT + 6, SHADES_TOP + 6 + 10 * 1), ED_Gosub (CIRCLE_EPIC),
	ED_Move (SHADES_LEFT + 6, SHADES_TOP + 6 + 10 * 2), ED_Gosub (CIRCLE_EPIC),
	ED_PosText (SHADES_LEFT + 4, SHADES_TOP - 2, SHADE_STR),
	ED_PosText (SHADES_LEFT + 6 + 11, SHADES_TOP + 8 + 10 * 0, RGB_STR),
	ED_PosText (SHADES_LEFT + 6 + 11, SHADES_TOP + 8 + 10 * 1     , V_STR),
	ED_PosText (SHADES_LEFT + 6 + 11 + 10, SHADES_TOP + 8 + 10 * 1, TWO_STR),
	ED_PosText (SHADES_LEFT + 6 + 11 + 20, SHADES_TOP + 8 + 10 * 1, S_STR),
	ED_PosText (SHADES_LEFT + 6 + 11, SHADES_TOP + 8 + 10 * 2, COLOR_STR),
	ED_Plot (SHADES_LEFT + 6 + 11 + 6, SHADES_TOP + 8 + 10 * 1 + 1),
	ED_Plot (SHADES_LEFT + 6 + 11 + 8, SHADES_TOP + 8 + 10 * 1 + 1),
	ED_Plot (SHADES_LEFT + 6 + 11 + 6, SHADES_TOP + 8 + 10 * 1 + 3),
	ED_Plot (SHADES_LEFT + 6 + 11 + 8, SHADES_TOP + 8 + 10 * 1 + 3),
	ED_Plot (SHADES_LEFT + 6 + 11 + 7, SHADES_TOP + 8 + 10 * 1 + 2),
	ED_RelHLine (SHADES_LEFT + 6 + 11 + 16, 3, SHADES_TOP + 8 + 10 * 1 + 2),
	ED_RelVLine (SHADES_LEFT + 6 + 11 + 17, SHADES_TOP + 8 + 10 * 1 + 1, 3),

	/* Orientation */
	ED_Rect (ORIENTATION_LEFT, ORIENTATION_TOP, ORIENTATION_WIDTH, ORIENTATION_HEIGHT),
	ED_FgColorTbl (RT_WHITE),
	ED_RelHLine (ORIENTATION_LEFT + 3, ORIENTATION_TITLE, ORIENTATION_TOP),
	ED_FgColorTbl (RT_BLACK),
	ED_Move (ORIENTATION_LEFT + 4, ORIENTATION_TOP + 6 + 10 * 0), ED_Gosub (CIRCLE_EPIC),
	ED_Move (ORIENTATION_LEFT + 4, ORIENTATION_TOP + 6 + 10 * 1), ED_Gosub (CIRCLE_EPIC),
	ED_PosText (ORIENTATION_LEFT + 4, ORIENTATION_TOP - 2, ORIENTATION_STR),
	ED_PosText (ORIENTATION_LEFT + 4 + 11, ORIENTATION_TOP + 8 + 10 * 0, PORTRAIT_STR),
	ED_PosText (ORIENTATION_LEFT + 4 + 11, ORIENTATION_TOP + 8 + 10 * 1, LANDSCAPE_STR),

	/* Res */
	ED_Rect (RES_LEFT, RES_TOP, RES_WIDTH, RES_HEIGHT),
	ED_FgColorTbl (RT_WHITE),
	ED_RelHLine (RES_LEFT + 3, RES_TITLE, RES_TOP),
	ED_FgColorTbl (RT_BLACK),
	ED_Move (RES_LEFT + 4, RES_TOP + 6 + 10 * 0), ED_Gosub (CIRCLE_EPIC),
	ED_Move (RES_LEFT + 4, RES_TOP + 6 + 10 * 1), ED_Gosub (CIRCLE_EPIC),
	ED_PosText (RES_LEFT + 4, RES_TOP - 2, RES_STR),
	ED_PosText (RES_LEFT + 4 + 11, RES_TOP + 8 + 10 * 0, R150_STR),
	ED_PosText (RES_LEFT + 4 + 11, RES_TOP + 8 + 10 * 1, R300_STR),

	/* Options */
#if 0
	ED_PosText (OPTION1_LEFT + 11, OPTION1_TOP + 2 + 10 * 0, UNI_STR),
#endif
	ED_PosText (OPTION1_LEFT + 11, OPTION1_TOP + 2 + 10 * 1, FIRST_STR),
	ED_PosText (OPTION1_LEFT + 11, OPTION1_TOP + 2 + 10 * 2, LIGHT_STR),
#if 0
	ED_Rect (OPTION1_LEFT, OPTION1_TOP + 10 * 0, 9, 9),
#endif
	ED_Rect (OPTION1_LEFT, OPTION1_TOP + 10 * 1, 9, 9),
	ED_Rect (OPTION1_LEFT, OPTION1_TOP + 10 * 2, 9, 9),

	/* Okay/Cancel */
	ED_Font (0),
	ED_Button (PRINT_LEFT, PRINT_TOP, PRINT_WIDTH, BUTTON_HEIGHT),
	ED_PosText (PRINT_LEFT + 2, PRINT_TOP + 2, PRINT_STR),
	ED_Button (CANCEL_LEFT, CANCEL_TOP, CANCEL_WIDTH, BUTTON_HEIGHT),
	ED_PosText (CANCEL_LEFT + 2, CANCEL_TOP + 2, CANCEL_STR),

	ED_End,
};

#if fDoSaveRooms
LOCAL EPic WaitEPic[] = {
	ED_Font (1),
	ED_DrawMode (JAM2),
	ED_FgColorTbl (RT_WHITE),
	ED_Rect (0, 0, WAIT_WIDTH - 1, WAIT_HEIGHT - 1),
	ED_FgColorTbl (RT_ORANGE),
	ED_Rect (1, 1, WAIT_WIDTH - 3, 10),
	ED_DrawMode (JAM1),
	ED_FgColorTbl (RT_BLACK),
	ED_Button (0, 0, WAIT_WIDTH, WAIT_HEIGHT),
	ED_RelHLine (0, WAIT_WIDTH, 11),

	/* BAR */
	ED_Rect (BAR_LEFT - 1, BAR_TOP - 1, BAR_WIDTH + 2, BAR_HEIGHT + 2),
	ED_End,
};



LOCAL EPicture WaitEPicture = {
	WaitEPic,
	RtFonts,
	RtStr,
	RtEPics,
	NULL,
	RtColors,
};
#endif/*fDoSaveRooms*/

LOCAL EPicture RTEPicture = {
	RTEPic,
	RtFonts,
	RtStr,
	RtEPics,
	NULL,
	RtColors,
};

LOCAL EPicture XEPicture = {
	XEPic,
	RtFonts,
	RtStr,
	RtEPics,
	NULL,
	RtColors,
};

LOCAL EPicture MarkEPicture = {
	MarkEPic,
	RtFonts,
	RtStr,
	RtEPics,
	NULL,
	RtColors,
};

#define NUM_ACROSS_NDX	0
#define NUM_DOWN_NDX	1

LOCAL char	Across[20] = "1";
LOCAL char	Down[20] = "1";

LOCAL char	*Nums[2] = {
	Across,
	Down,
};

#define NUM_STRING_GADS	2

LOCAL EditString NumString[NUM_STRING_GADS] = {
{ Across, (NUMBER_WIDTH - 5) / 8, RTDLG_LEFT + RTNUM_LEFT + ARROW_WIDTH + 1, RTDLG_TOP + RTNUM1_TOP + 2, NULL, &NumString[1], },
{ Down, (NUMBER_WIDTH - 5) / 8,   RTDLG_LEFT + RTNUM_LEFT + ARROW_WIDTH + 1, RTDLG_TOP + RTNUM2_TOP + 2, NULL, &NumString[0], },
};

#define Pattern(a,b,c,d,e,f,g,h)	\
	(((a) << 7) | ((b) << 6) | ((c) << 5) | ((d) << 4) | ((e) << 3) | ((f) << 2) | ((g) << 1) | ((h) << 0))

#if fDoSaveRooms
static UBYTE ShadePatterns[65][8] = {
	{ /* -1 */
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 0 */
		Pattern( 0,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 1 */
		Pattern( 0,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 2 */
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 3 */
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 4 */
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 5 */
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 6 */
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,1,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 7 */
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 8 */
		Pattern( 0,1,0,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,1,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 9 */
		Pattern( 0,1,0,1,0,1,1,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,1,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 10 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,1,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 11 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 12 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 13 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,1,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 14 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 1,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 15 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 16 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 17 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 18 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 19 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 20 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,1 ),
	},
	{ /* 21 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,0 ),
	},
	{ /* 22 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,1,1,1,1,0 ),
	},
	{ /* 23 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
	},
	{ /* 24 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
	},
	{ /* 25 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,1 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
	},
	{ /* 26 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,1,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
	},
	{ /* 27 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
	},
	{ /* 28 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,1,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,1,1,0 ),
	},
	{ /* 29 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,1,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,0,1,0 ),
	},
	{ /* 30 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,1,1,0,1,0,1,0 ),
	},
	{ /* 31 */
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 32 */
		Pattern( 0,0,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 33 */
		Pattern( 0,0,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 34 */
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 35 */
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 36 */
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 37 */
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 38 */
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,1,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 39 */
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 40 */
		Pattern( 0,0,0,0,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,1,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 41 */
		Pattern( 0,0,0,0,0,0,0,1 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,1,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 42 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,1,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 43 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 44 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 45 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,1,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 46 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,1,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 47 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 48 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 49 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 50 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 51 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 52 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,1,0 ),
	},
	{ /* 53 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,0,0 ),
	},
	{ /* 54 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,1,0,1,0,0,0 ),
	},
	{ /* 55 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
	},
	{ /* 56 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
	},
	{ /* 57 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,1,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
	},
	{ /* 58 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,1,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
	},
	{ /* 59 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
	},
	{ /* 60 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,1,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,1,0,0,0 ),
	},
	{ /* 61 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,1,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,0,0,0,0 ),
	},
	{ /* 62 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 1,0,0,0,0,0,0,0 ),
	},
	{ /* 63 */
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
		Pattern( 0,0,0,0,0,0,0,0 ),
	},
};
#endif/*fDoSaveRooms*/
#if 0
static UBYTE ShadePatterns[10][8][8] =){
	{ /* 0 */
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,1,1,1,1,1, },
	},
	{ /* 0 */
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,0,1,1,1,0,1, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,0,1,1,1,0,1, },
		{ 1,1,1,1,1,1,1,1, },
	},
	{ /* 1 */
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,0,1,1,1,0, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,1,1,0,1,1,1,0, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,0,1,0,1,0,1,0, },
	},
	{ /* 3 */
		{ 1,1,1,1,1,1,1,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 1,1,1,1,1,1,1,1, },
		{ 1,0,1,0,1,0,1,0, },
	},
	{ /* 4 */
		{ 1,1,0,1,1,1,0,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,1,1,1,0,1,1,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 1,1,0,1,1,1,0,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,1,1,1,0,1,1,1, },
		{ 1,0,1,0,1,0,1,0, },
	},
	{ /* 5 */
		{ 0,1,0,1,0,1,0,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,1,0,1,0,1,0,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,1,0,1,0,1,0,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,1,0,1,0,1,0,1, },
		{ 1,0,1,0,1,0,1,0, },
	},
	{ /* 6 */
		{ 0,0,0,1,0,0,0,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,1,0,0,0,1,0,0, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,0,0,1,0,0,0,1, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,1,0,0,0,1,0,0, },
		{ 1,0,1,0,1,0,1,0, },
	},
	{ /* 7 */
		{ 0,0,0,0,0,0,0,0, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 1,0,1,0,1,0,1,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 1,0,1,0,1,0,1,0, },
	},
	{ /* 8 */
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 1,0,0,0,1,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 1,0,0,0,1,0,0,0, },
	},
	{ /* 9 */
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
		{ 0,0,0,0,0,0,0,0, },
	},
};
#endif
/***************************** R O U T I N E S ****************************/


#if fDoSaveRooms
/*********************************************************************
 *
 * ReorderColors
 *
 * PURPOSE
 *		This routine finds the "whitest" color and assigns to color
 *		zero. This prevents the printer from filling in the borders
 *		with some solid color.
 *
 *		This also forces color zero to white (255, 255, 255).
 *
 *		This routines maps all other white values to slot 0.
 *
 * INPUT
 *		pcsc			: input palette to search for whitest color
 *
 * OUTPUT
 *		cLookup[]		: created
 *
 * ASSUMES
 *		User is sending palette to HP RTL device.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		09/07/93 Tuesday (dcc) - created.
 *
*/
LOCAL void ReorderColors(CS_Color *pcsc)
{
	short i;
	short iWhitest = 0;
	short Brightest = 0;			/* value of "brightest" color */
	short Whitest = 32767;			/* value of "whitest" color */
	/* BUGBUG: should be some constant (not 32767) */

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReorderColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (i = MAX_CI_COLORS-1; i >= 0; i--)	/* Search for whitest color */
	{
		short Bright = pcsc[i].Red+pcsc[i].Green+pcsc[i].Blue;
		short White =	abs(pcsc[i].Red-pcsc[i].Green) / 8 +
						abs(pcsc[i].Green-pcsc[i].Blue) / 8 +
						abs(pcsc[i].Blue-pcsc[i].Red) / 8;

		if (White <= Whitest && Bright >= Brightest)
		{
			iWhitest = i;
			Brightest = Bright;
			Whitest = White;
		}

		cLookup[i] = i;			/* could be in its own loop, we stuck it here */
	}

	/* Swap whitest color with color in slot 0 */

	cLookup[0] = iWhitest;
	cLookup[iWhitest] = 0;

	/* Set all other white entries to palette slot 0 */

	for (i = MAX_CI_COLORS-1; i > 0; i--)	/* Search for whitest color */
	{
		if (pcsc[i].Red == MAX_HARDWARE_COLOR &&
			pcsc[i].Green == MAX_HARDWARE_COLOR &&
			pcsc[i].Blue == MAX_HARDWARE_COLOR)
		{
			cLookup[i] = 0;
		}
	}

	/* Set palette slot 0 to white */

	pcsc[i].Red = MAX_HARDWARE_COLOR;
	pcsc[i].Green = MAX_HARDWARE_COLOR;
	pcsc[i].Blue = MAX_HARDWARE_COLOR;

} /* ReorderColors */


/*********************************************************************
 *
 * StartPage
 *
 * PURPOSE
 *		Do stuff (including sending data to printer) that should
 *		occur at the start of each page.
 *
 *		NOTE: currently, <cxPrint> and <cyPrint> are only used to center
 *		the printout in certain (bizarre?) situations. Go figure.
 *
 * INPUT
 *		short cxPrint	: width in pixels to print on page
 *		short cyPrint	: height in pixels to print on page
 *		short onepage	: (set TRUE if only one row of pages. Go figure).
 *		short xpage		: print which page across we're on
 *		short ypage		: print which page down we're on
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/19/93 Thursday (dcc) - updated to support HP-GL/2.
 *		08/27/93 Friday (dcc) - Try label in lower-left instead of upper-left.
 *
 * SEE ALSO
 *
*/
LOCAL void near StartPage (short cxPrint, short cyPrint, short onepage,
					short xpage, short ypage, CS_Color *pcsc)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "StartPage";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	switch (OutputDevice)
	{
	case HPPCL3:
	{
		static UBYTE PageStart[] = {
			#if 0
/*  0 */	0x1B, 0x45,	/* Reset */
/*  2 */	0x1B, 0x26, 0x6B, 0x35, 0x57,	/* Text Scale Off */
/*  7 */	0x1B, 0x26, 0x6C, 0x30, 0x6F, 0x32, 0x45,	/* Top Margin/Num Lines */
/* 14 */	0x1B, 0x2A,	0x74, 0x33, 0x30, 0x30, 0x52,	/* Raster Graphics 300dpi */
/* 21 */	0x1B, '*', 'p', '0', '0', '0', '0', 'X',	/* Horizontal Dots */
/* 29 */	0x1B, '*', 'p', '0', '0', '0', '0', 'Y',	/* Vertical Dots */
			#endif
			#if 1
			0x1B, 0x2A, 0x72, 0x31, 0x41,	/* Start Graphics at C.A.P. */
			#else
			0x1B, 0x2A, 0x72, 0x30, 0x41,	/* Start Graphics at Left */
			#endif
			#if 0
			0x1B, 0x2A, 0x62, 0x32, 0x4D,	/* Use Compression Method 2 */
			#else
			0x1B, 0x2A, 0x62, 0x30, 0x4D,	/* Use Compression Method 0 */
			#endif
		};
		short	skip;

		fprintf (PrintFP, "\033E");		/* Reset */
		fprintf (PrintFP, "\033&k5W");	/* Text Scale Off */
		fprintf (PrintFP, "\033(0U");	/* Primary symbol set = ISO 6: ASCII */
		fprintf (PrintFP, "\033(s0p"	/* Primary font spacing, fixed */
						"10h"	   		/* Primary font pitch = 10 chars per inch */
						"12v"	   		/* Primary font height = 12 points */
						"0s"	   		/* Primary font style = Upright */
						"0b"	   		/* Primary font stroke weight = 0 */
						"3T");   		/* Primary typeface = Courier */

		fprintf (PrintFP, "\033&l0o2E");/* Top Margin/Num Lines */
		if (IdiotNumbers) // && !onepage)
			fprintf (PrintFP, "pg. (%d,%d)\n", xpage, ypage);

										/* Set Print Resolution */
		fprintf (PrintFP, "\033*t%dR", PRINT_DPI);

		/* Set Horizontal Position */
		if (onepage) {
					/* page height in 300DPI - height to print in 300DPI */
			skip = (short) ((cencyPage - cencyBorder) * (long) DPI_300 / 100L)
				- (short) ((long) cyPrint * (long) DPI_300 / (long) PRINT_DPI);
			if (skip < 0) {
				skip = 0;
			}
		} else {
			skip = 0;
		}
		fprintf (PrintFP, "\033*p%dY",
				(skip / 2) + (IdiotNumbers ? (150) : 0));

		/* Set Vertical Position */
				/* page width in 300DPI - width to print in 300DPI */
		skip  = (short) ((cencxPage - cencxBorder) * (long) DPI_300 / 100L)
				- (short) ((long) cxPrint * (long) DPI_300 / (long) PRINT_DPI);
		if (onepage) {
			skip /= 2;
		}
		if (skip < 0) {
			skip = 0;
		}
		fprintf (PrintFP, "\033*p%dX", skip );

		fwrite (PageStart, sizeof (PageStart), 1, PrintFP);
		break;
	}
	case HPGL2:
	{
		short i;

										/* Reset */
										/* Enter HP-GL/2 mode */
										/* Turn off autorotation */
		fprintf (PrintFP, "\033E\033%0BBP5,1");
										/* Set logical page size */
		fprintf (PrintFP, "PS%ld,%ld",
				(cencyPage - cencyBorder) * 1016L / 100L,
				(cencxPage - cencxBorder) * 1016L / 100L);

		if (IdiotNumbers)
		{
										/* Label Origin in lower-left */
										/* "p.(1,1)" */
										/* Pen up */
										/* Goto 0, cencyIdiot */
			fprintf (PrintFP, "LO1LBp.(%d,%d)\003PUPA0,%ld",
					xpage, ypage, cencyIdiot * 1016L / 100L);
		}
		else
		{
			fprintf (PrintFP, "PUPA0,0");/* Pen up, Goto 0, 0 */
		}
										/* Enter HP-RTL mode */
		fprintf (PrintFP, "\033%1A");
										/* Set Print Resolution */
										/* Configure Image Data */
		fprintf (PrintFP, "\033*t%dR\033*v6W", PRINT_DPI);

		putc (0, PrintFP);
		putc (0, PrintFP);
		fprintf (PrintFP, "\010\010\010\010");

		ReorderColors(pcsc);

		if (UseLightShades)
			for (i = 0; i < MAX_CI_COLORS; i++)	/* Send palette to HP RTL device */
			{
				fprintf (PrintFP, "\033*v%da%db%dc%dI",
						GammaCorrect[pcsc[cLookup[i]].Red],
						GammaCorrect[pcsc[cLookup[i]].Green],
						GammaCorrect[pcsc[cLookup[i]].Blue], i);
			}
		else
			for (i = 0; i < MAX_CI_COLORS; i++)	/* Send palette to HP RTL device */
			{
				fprintf (PrintFP, "\033*v%da%db%dc%dI",
						pcsc[cLookup[i]].Red,
						pcsc[cLookup[i]].Green,
						pcsc[cLookup[i]].Blue, i);
			}

										/* No Compression */
										/* Start Raster Graphics at CAP */
		fprintf (PrintFP, "\033*b0M\033*r1A");
		break;
	}
	}
} /* StartPage */


/*********************************************************************
 *
 * PrintLineBuf
 *
 * PURPOSE
 *		
 *
 * INPUT
 *	bytes			: number of bytes to write to printer
 *	fDone			: TRUE if last plane of current row
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		08/19/93 Thursday (dcc) - fDone added to support HP-GL/2.
 *
 * SEE ALSO
 *
*/
LOCAL void near PrintLineBuf (UWORD bytes, BOOL fDone)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PrintLineBuf";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
#if 1
		register UWORD uw = bytes - 1;

		while (uw != 0 && LineBuf[uw] == (UBYTE) 0)
			uw--;

		bytes = uw+1;
#else
		while (bytes > 1 && !LineBuf[bytes - 1]) {
			bytes--;
		}
#endif
		if (fDone)
			fprintf (PrintFP, "\033*b%dW", bytes);
		else
			fprintf (PrintFP, "\033*b%dV", bytes);
		fwrite (LineBuf, (size_t)bytes, 1, PrintFP);
	}
} /* PrintLineBuf */


/*********************************************************************
 *
 * FinishPage
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
 *		08/19/93 Thursday (dcc) - updated to support HP-GL/2.
 *		08/27/93 Friday (dcc) - add [ESC]E to end of HP-GL/2 string. Try LO 1.
 *
 * SEE ALSO
 *
*/
LOCAL void near FinishPage (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FinishPage";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	switch (OutputDevice)
	{
	case HPPCL3:
	{
		static UBYTE PageEnd[] = {
 			0x1B, 0x2A, 0x72,
 			0x42, 0x1B, 0x28, 0x30, 0x55, 0x1B, 0x28, 0x73, 0x30, 0x70, 0x31, 0x30, 0x68, 0x31, 0x32, 0x76,
 			0x30, 0x73, 0x30, 0x62, 0x33, 0x54, 0x0C, 0x1B, 0x45,
		};

		fwrite (PageEnd, sizeof (PageEnd), 1, PrintFP);
		break;
	}
	case HPGL2:
	{
											/* End Raster Graphics */
											/* Re-enter HP-GL/2 mode */
											/* Reset */
		fprintf (PrintFP, "\033*rC" "\033%0B" "\033E");
		break;
	}
	}
} /* FinishPage */


/*********************************************************************
 *
 * SendLineToPrinter
 *
 * PURPOSE
 *		
 *
 * INPUT
 *		len				: length of row in bytes
 *		xpos			: x-pos on page (for dither pattern lookup)
 *
 * USES
 *		PrintBuffer		: row of bytes to send to printer
 *		cLookup[]		: color-lookup table to translate bytes through
 *
 * EFFECTS
 *		If HP-GL/2 mode, PrintBuffer is modified to contain lookup'ed colors.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		07/08/94 Friday (dcc) - created from SendLineReversedToPrinter
 *
 * SEE ALSO
 *
*/
LOCAL void near SendLineToPrinter (short len, short xpos)
{
	static	 endmask[8] =
	{
		0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE,
	};

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SendLineToPrinter";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	#if 0
	#if TEST_PRINT
	{
		UBYTE	*screen;
		UBYTE	*pixel;
		UWORD	 newx;
		short	 y;
		UWORD	 pixels;

		newx   = xpos % SCREEN_WIDTH;
		screen = MK_FP (0xA000U, newx);
		pixel  = PrintBuffer;

		for (y = 0; y < SCREEN_HEIGHT; y++) {
			*screen = *pixel++;
			screen += SCREEN_WIDTH;
		}
	}
	#endif
	#endif

	switch (OutputDevice)
	{
	case HPPCL3:
	{
		static	 xmasks[8] = {
			0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
		};
		UBYTE	*pdata;
		UBYTE	*pixel;
		UBYTE	 pbyte;
		UBYTE	 xposmask;
		short	 bytes;
		short	 ypos;
		short	 shade;
		UWORD	 pixels;

		xposmask = xmasks[xpos & 0x0007];
		ypos  = 0;

		pixels = len;
		len   += 7;
		len   /= 8;
		bytes  = len;

		pixel = PrintBuffer;
		pdata = LineBuf;
		while (len) {
			pbyte  = 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x80 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x40 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x20 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x10 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x08 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x04 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x02 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x01 : 0;
			*pdata++ = pbyte;
			ypos &= 0x07;
			len--;
		}

		LineBuf[bytes-1] &= endmask[pixels & 0x0007];

		PrintLineBuf (bytes, TRUE);
		break;
	}
	case HPGL2:
	{
		short	 bytes;
		UWORD	 pixels;
		short	 i;
		UBYTE	 bitMask = 1;

		/* Translate bytes through color lookup table */

		for (i = 0; i < len; i++)
		{
			PrintBuffer[i] = cLookup[PrintBuffer[i]];
		}

		/* Convert row of bytes into planes, send to printer */

		pixels = len;
		bytes  = (len + 7) / 8;

		for (i = 0; i < 8; i++)			/* do 8 planes */
		{
			UBYTE	*pdata;
			UBYTE	*pixel;

			pixel = PrintBuffer;
			pdata = LineBuf;

			for (len = bytes; len > 0; len--)
			{
				UBYTE ub = 0;

				ub |= (*pixel++ & bitMask) ? 0x80 : 0;
				ub |= (*pixel++ & bitMask) ? 0x40 : 0;
				ub |= (*pixel++ & bitMask) ? 0x20 : 0;
				ub |= (*pixel++ & bitMask) ? 0x10 : 0;
				ub |= (*pixel++ & bitMask) ? 0x08 : 0;
				ub |= (*pixel++ & bitMask) ? 0x04 : 0;
				ub |= (*pixel++ & bitMask) ? 0x02 : 0;
				ub |= (*pixel++ & bitMask) ? 0x01 : 0;
				*pdata++ = ub;
			}

			LineBuf[bytes-1] &= endmask[pixels & 0x0007];

			PrintLineBuf (bytes, i == 7);
			bitMask <<= 1;
		}
		break;
	}
	}
} /* SendLineToPrinter */


/*********************************************************************
 *
 * SendLineReversedToPrinter
 *
 * PURPOSE
 *		
 *
 * INPUT
 *		len				: length of row in bytes
 *		xpos			: x-pos on page (for dither pattern lookup)
 *
 * USES
 *		PrintBuffer		: row of bytes to send to printer
 *		cLookup[]		: color-lookup table to translate bytes through
 *
 * EFFECTS
 *		If HP-GL/2 mode, PrintBuffer is modified to contain lookup'ed colors.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/19/93 Thursday (dcc) - updated to support HP-GL/2.
 *		07/08/94 Friday (dcc) - changed endmask[0] from 0x00 to 0xFF
 *
 * SEE ALSO
 *
*/
LOCAL void near SendLineReversedToPrinter (short len, short xpos)
{
	static	 endmask[8] =
	{
		0xFF, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F,
	};

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SendLineReversedToPrinter";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	#if 0
	#if TEST_PRINT
	{
		UBYTE	*screen;
		UBYTE	*pixel;
		UWORD	 newx;
		short	 y;
		UWORD	 pixels;

		newx   = xpos % SCREEN_WIDTH;
		screen = MK_FP (0xA000U, newx);
		pixel  = PrintBuffer;

		for (y = 0; y < SCREEN_HEIGHT; y++) {
			*screen = *pixel++;
			screen += SCREEN_WIDTH;
		}
	}
	#endif
	#endif

	switch (OutputDevice)
	{
	case HPPCL3:
	{
		static	 xmasks[8] = {
			0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
		};
		UBYTE	*pdata;
		UBYTE	*pixel;
		UBYTE	 pbyte;
		UBYTE	 xposmask;
		short	 bytes;
		short	 ypos;
		short	 shade;
		UWORD	 pixels;

		xposmask = xmasks[xpos & 0x0007];
		ypos  = 0;

		pixels = len;
		len   += 7;
		len   /= 8;
		bytes  = len;

		pixel = PrintBuffer;
		pdata = LineBuf + bytes - 1;
		while (len) {
			pbyte  = 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x01 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x02 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x04 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x08 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x10 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x20 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x40 : 0;
			shade  = PrintColors[*pixel++];
			pbyte |= ((ShadePatterns[shade][ypos++]) & xposmask) ? 0x80 : 0;
			*pdata-- = pbyte;
			ypos &= 0x07;
			len--;
		}

		LineBuf[0] &= endmask[pixels & 0x0007];

		PrintLineBuf (bytes, TRUE);
		break;
	}
	case HPGL2:
	{
		short	 bytes;
		UWORD	 pixels;
		short	 i;
		UBYTE	 bitMask = 1;

		/* Translate bytes through color lookup table */

		for (i = 0; i < len; i++)
		{
			PrintBuffer[i] = cLookup[PrintBuffer[i]];
		}

		/* Convert row of bytes into planes, send to printer */

		pixels = len;
		bytes  = (len + 7) / 8;

		for (i = 0; i < 8; i++)			/* do 8 planes */
		{
			UBYTE	*pdata;
			UBYTE	*pixel;

			pixel = PrintBuffer;
			pdata = LineBuf + bytes - 1;

			for (len = bytes; len > 0; len--)
			{
				UBYTE ub = 0;

				ub |= (*pixel++ & bitMask) ? 0x01 : 0;
				ub |= (*pixel++ & bitMask) ? 0x02 : 0;
				ub |= (*pixel++ & bitMask) ? 0x04 : 0;
				ub |= (*pixel++ & bitMask) ? 0x08 : 0;
				ub |= (*pixel++ & bitMask) ? 0x10 : 0;
				ub |= (*pixel++ & bitMask) ? 0x20 : 0;
				ub |= (*pixel++ & bitMask) ? 0x40 : 0;
				ub |= (*pixel++ & bitMask) ? 0x80 : 0;
				*pdata-- = ub;
			}

			LineBuf[0] &= endmask[pixels & 0x0007];

			PrintLineBuf (bytes, i == 7);
			bitMask <<= 1;
		}
		break;
	}
	}
} /* SendLineReversedToPrinter */


/*********************************************************************
 *
 * PrintRoom
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
 * ASSUMES
 *		roomwindow->CurrentRoom->Room != NULL
 *		FindNonEmptyLayer() returns != NULL
 *
 * RETURN VALUE
 *		FALSE if unsuccessful.
 *
 * HISTORY
 *		09/23/93 Thursday (dcc) - make sure PrintBuffer & LineBuf are allocated
 *		07/08/94 Friday (dcc) - make thermometer reach 100% instead of stopping short
 *		07/08/94 Friday (dcc) - add support to print in portrait mode.
 *
 * SEE ALSO
 *
*/
short PrintRoom (
		RoomWindowType	*roomwindow
		)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PrintRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		SaveAreaType		*sa = NULL;
		RoomType	*room;
		LayerType	*play;
		WORD		 wPrtTileWidth;
		WORD		 wPrtTileHeight;
		WORD		 wPrtTilesAcross;
		WORD		 wPrtTilesDown;
		WORD		 wPrtPagesAcross;
		WORD		 wPrtPagesDown;
		WORD		 starttilerow;
		WORD		 starttilecolumn;
		WORD		 tilesacross;
		WORD		 tilesdown;
		WORD		 pixelsacross;
		WORD		 pixelsdown;
		WORD		 pagesdown;
		WORD		 pagesacross;
		WORD		 xtile;
		WORD		 ytile;
		WORD		 x;
		WORD		 y;
		WORD		 xpos;
		WORD		 sep;
		short		 abort = FALSE;
		short		 len;
		char		 msg[40];

		if (!PrintBuffer && !LineBuf)
			return FALSE;

		if (!PushWindowClipValues (RTWin)) {
			return FALSE;
		}
		sa = SaveArea (WAIT_LEFT, WAIT_TOP, WAIT_WIDTH, WAIT_HEIGHT);
		if (!sa) {
			SetGlobalErr (ERR_GENERIC);
			GEcatf ("\nOOM: Printing Room Area");
			return FALSE;
		}
				
		PrintFP = fopen (PrintFile, "wb");
		if (!PrintFP) {
			Message (MainWindow, "Error!", "Couldn't open printer file!", " DARN! ");
			goto cleanup;
		}
		
		#if __MSDOS__
		
		{
			int	fh;

			fh = fileno (PrintFP);
			if (isatty (fh)) {
#if __WATCOMC__
				union REGS regs;

				regs.w.ax = 0x4401;
				regs.w.bx = fh;
				regs.w.dx = 0x00E0;

				intdos(&regs, &regs);
#else
				ioctl (fh, 1, 0x00E0, 0);
#endif
			}
		}

		#else
		#error Need sompin here
		#endif

		BeforeGraphics ();

		{
			struct	RastPort	 rp = { NULL, };
			StartEPicture (&rp, &WaitEPicture, WAIT_LEFT, WAIT_TOP);
		}
		DrawString (&MainFont,
				WAIT_LEFT + (WAIT_WIDTH / 2) - (StringLen (&MainFont, "Printing...") / 2),
				WAIT_TOP + 2,
				"Printing...");
		DrawString (&MainFont,
				WAIT_LEFT + (WAIT_WIDTH / 2) - (StringLen (&MainFont, "Press ESC to stop") / 2),
				WAIT_TOP + 46,
				"Press ESC to stop");

		room = roomwindow->CurrentRoom->Room;
		play = FindNonEmptyLayer(&room->Layers, room->FloorLayer);
		sep	 = 0 + ((roomwindow->CurrentRoom->Flags) & (DISPLAY_SEPERATED));

		#if TEST_PRINT
		{
			short	i;
			long	val;

			for (i = 0; i < 256; i++) {
				switch (PrintColorMode) {
				case RGBAVE_MODE:
					val = ( room->ColorInfo.Colors[i].red +
							room->ColorInfo.Colors[i].green +
							room->ColorInfo.Colors[i].blue) / 3;
					break;
				case V2S_MODE:
					val = ((long)room->ColorInfo.HSVs[i].Value * 2 -
					   	  room->ColorInfo.HSVs[i].Saturation) / 2;
					val = max (0, val);
					break;
				case COLORCORRECT_MODE:
					val =  (room->ColorInfo.Colors[i].red   * 120L +
							room->ColorInfo.Colors[i].green * 236L +
							room->ColorInfo.Colors[i].blue  *  44L ) / (120+236+44);
					break;
				}

				if (UseLightShades) {
					val  = 255 - val;
					val *= val;
					val /= 255;
					val  = 255 - val;
				}

				PrintColors[i] = val * 64 / 255;
			}
		}
		#else
		if (OutputDevice == HPPCL3)
		{
			short	i;
			long	val;
			CS_Color	rgcsc[MAX_CI_COLORS];

			ReadXTRA(room->R_ColorInfo->pxtColors, 0,	// read onto stack
							 MAX_CI_COLORS * sizeof (CS_Color), rgcsc);

			for (i = 0; i < MAX_CI_COLORS; i++) {
				switch (PrintColorMode) {
				case RGBAVE_MODE:
					val = ( rgcsc[i].Red + 
							rgcsc[i].Green + 
							rgcsc[i].Blue ) / 3; 
					break;
				case V2S_MODE:
					val = ((long)rgcsc[i].Value * 2 -
					   	  rgcsc[i].Saturation) / 2;
					val = max (0, val);
					break;
				case COLORCORRECT_MODE:
					val =  (rgcsc[i].Red * 120L +
							rgcsc[i].Green * 236L +
							rgcsc[i].Blue * 44L) / (120+236+44);
					break;
				}

				if (UseLightShades) {
					val  = MAX_HARDWARE_COLOR - val;
					val *= val;
					val /= MAX_HARDWARE_COLOR;
					val  = MAX_HARDWARE_COLOR - val;
				}

				PrintColors[i] = (UBYTE) (val * 64 / MAX_HARDWARE_COLOR);
			}
		}
		#endif

		#if 0
		wPrtTileWidth  = (wSrcSizeToDstSize (room->TileWidth,  wPrtSrcX, wPrtDestX) + sep) * wPrtSrcX;
		wPrtTileHeight = (wSrcSizeToDstSize (room->TileHeight, wPrtSrcY, wPrtDestY) + sep) * wPrtSrcY;
		#else
		wPrtTileWidth  = (wSrcSizeToDstSize (play->cxTile, wPrtSrcX, wPrtDestX) + sep);
		wPrtTileHeight = (wSrcSizeToDstSize (play->cyTile, wPrtSrcY, wPrtDestY) + sep);
		#endif

		wPrtTilesAcross = (WORD) (PRINT_PIXELS_ACROSS / wPrtTileWidth);
		wPrtTilesDown   = (WORD) (PRINT_PIXELS_DOWN / wPrtTileHeight);

		if (FirstPageOnly) {
			wPrtPagesAcross = 1;
			wPrtPagesDown   = 1;
		} else {
			wPrtPagesAcross = (play->ctilx + wPrtTilesAcross - 1) / wPrtTilesAcross;
			wPrtPagesDown   = (play->ctily + wPrtTilesDown   - 1) / wPrtTilesDown;
		}

		/*
		** Set Zoom Vars
		*/
		wSrcSkipX = wPrtSrcX;
		wSrcSkipY = wPrtSrcY;
		wDstDupX  = wPrtDestX;
		wDstDupY  = wPrtDestY;

		/* Loop through each page row in the map */

		for (pagesdown = 0; pagesdown < wPrtPagesDown && !abort; pagesdown++)
		{
			/* Loop through each page column in the map */

			for (pagesacross = 0; pagesacross < wPrtPagesAcross && !abort; pagesacross++)
			{
				starttilerow	= pagesdown * wPrtTilesDown;
				tilesdown		= min (wPrtTilesDown, play->ctily - starttilerow);
				pixelsdown		= tilesdown * wPrtTileHeight;

				starttilecolumn	= pagesacross * wPrtTilesAcross;
				tilesacross		= min (wPrtTilesAcross, play->ctilx - starttilecolumn);
				pixelsacross	= tilesacross * wPrtTileWidth;

				/* For each page in the map: */

				xpos            = 0;
				{
					CS_Color	rgcsc[MAX_CI_COLORS];

					ReadXTRA(room->R_ColorInfo->pxtColors, 0,	// read onto stack
							 		MAX_CI_COLORS * sizeof (CS_Color), rgcsc);

					if (fPrintPortrait)
					{
						StartPage (
							pixelsacross,
							pixelsdown,
							wPrtPagesDown == 1, pagesacross + 1, pagesdown + 1,
							rgcsc);
					}
					else
					{
						StartPage (
							pixelsdown,
							pixelsacross,
							wPrtPagesDown == 1, pagesacross + 1, pagesdown + 1,
							rgcsc);
					}
				}

				SetClipValues(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				sprintf (msg, "page %d of %d",
					pagesdown * wPrtPagesAcross + pagesacross + 1,
					wPrtPagesAcross * wPrtPagesDown);
				SetPenColor (RtColors[RT_WHITE]);
				DrawRect (WAIT_LEFT + 1, WAIT_TOP + 17, WAIT_WIDTH - 3, 8);
				DrawRect (WAIT_LEFT + BAR_LEFT, WAIT_TOP + BAR_TOP, BAR_WIDTH , BAR_HEIGHT);
				DrawString (&MainFont,
						WAIT_LEFT + (WAIT_WIDTH / 2) - (StringLen (&MainFont, msg) / 2),
						WAIT_TOP + 17,
						msg);
				
				memset (PrintBuffer, 0, (size_t) sizeofBufferCurrent);

				if (fPrintPortrait)
				{
					for (ytile = 0; ytile < tilesdown && !abort; ytile++)
					{
						SetClipValues(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
						#if TEST_PRINT
							sprintf (msg, "%03d,%03d,%03d", pagesacross, pagesdown, xtile);
							DS (msg);
						#endif

						SetPenColor (RtColors[RT_ORANGE]);
						len = (ytile+1) * BAR_WIDTH / tilesdown;
						if (len)
						{
							DrawRect (WAIT_LEFT + BAR_LEFT, WAIT_TOP + BAR_TOP,
								len, BAR_HEIGHT);
						}
						SetClipValues(0, 0, (int) PRINT_PIXELS_ACROSS, 1);

						for (y = 0; y < wPrtTileHeight; y++)
						{
						#if TEST_PRINT
							LayerType	*play;

							memset (PrintBuffer, GlobalBackground, pixelsacross);

							play = room->Layers[0];

//DCC: FIXME				ShowLayersInSubRect(play, play,
												roomwindow, &PrintBM
												0, -y, PRINT_PIXELS_ACROSS, 1,
												starttilecolumn, starttilerow,
												tilesacross, 1, FALSE, FALSE);
						#else
							memset (PrintBuffer, GlobalBackground, pixelsacross);

							Assert((short) PRINT_PIXELS_ACROSS == PRINT_PIXELS_ACROSS);
							ShowLayersInSubRect(&room->Layers, room->FloorLayer,
												roomwindow, &PrintBM,
												0, -y, (short) PRINT_PIXELS_ACROSS, 1,
												starttilecolumn, starttilerow,
												tilesacross, 1,
#if NEWTUME
												TRUE,
#else
												(IsGuideVisible(room)),
#endif
												(IsComposite(room) && IsLocked(room)));
						#endif /* TEST_PRINT */

							SendLineToPrinter (pixelsacross, xpos++);
						}
						{
							UWORD	key;

							key = GetKeyNoWait ();
							if (key)
							{
								if (ConvertKeyToAscii (key) == 0x1B)
								{
									abort = TRUE;
								}
							}
						}
						starttilerow++;
					}
				}
				else
				{
					for (xtile = 0; xtile < tilesacross && !abort; xtile++)
					{
						SetClipValues(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
						#if TEST_PRINT
							sprintf (msg, "%03d,%03d,%03d", pagesacross, pagesdown, xtile);
							DS (msg);
						#endif

						SetPenColor (RtColors[RT_ORANGE]);
						len = (xtile+1) * BAR_WIDTH / tilesacross;
						if (len)
						{
							DrawRect (WAIT_LEFT + BAR_LEFT, WAIT_TOP + BAR_TOP,
								len, BAR_HEIGHT);
						}
						SetClipValues(0, 0, 1, (int) PRINT_PIXELS_DOWN);

						for (x = 0; x < wPrtTileWidth; x++)
						{
						#if TEST_PRINT
							LayerType	*play;

							memset (PrintBuffer, GlobalBackground, pixelsdown);

							play = room->Layers[0];

//DCC: FIXME				ShowLayersInSubRect(play, play,
												roomwindow, &PrintBM
												-x, 0, 1, PRINT_PIXELS_DOWN,
												starttilecolumn, starttilerow,
												1, tilesdown, FALSE, FALSE);
						#else
							memset (PrintBuffer, GlobalBackground, pixelsdown);

							Assert((short) PRINT_PIXELS_DOWN == PRINT_PIXELS_DOWN);
							ShowLayersInSubRect(&room->Layers, room->FloorLayer,
												roomwindow, &PrintBM,
												-x, 0, 1, (short) PRINT_PIXELS_DOWN,
												starttilecolumn, starttilerow,
												1, tilesdown,
#if NEWTUME
												TRUE,
#else
												(IsGuideVisible(room)),
#endif
												(IsComposite(room) && IsLocked(room)));
						#endif /* TEST_PRINT */

							SendLineReversedToPrinter (pixelsdown, xpos++);
						}
						{
							UWORD	key;

							key = GetKeyNoWait ();
							if (key)
							{
								if (ConvertKeyToAscii (key) == 0x1B)
								{
									abort = TRUE;
								}
							}
						}
						starttilecolumn++;
					}
				}
				FinishPage ();
			}
		}
		AfterGraphics ();

		SetGlobalZoom(room);

		fclose (PrintFP);

cleanup:
		if (sa) RestoreArea (sa);
		FlushKeyboardBuffer ();
		PopClipValues ();

		return !abort;
	}
} /* PrintRoom */
#else
short PrintRoom (RoomWindowType	*prw)
{
	prw = prw;
	TellUser (NoCanDo, "Printing is disabled in this demo version.");
	return TRUE;
}
#endif/*fDoSaveRooms*/


/*********************************************************************
 *
 * ClearPrintData
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
LOCAL void near ClearPrintData (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ClearPrintData";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	cencinPrintA    =
	cencinPrintD    =
	cencinTileA     =
	cencinTileD     = 0L;
	pagesacross     =
	pagesdown       =
	tilesacrosspage =
	tilesdownpage   = 0;

} /* ClearPrintData */


#if 1
/*********************************************************************
 *
 * ComputePrintRatio
 *
 * PURPOSE
 *		Scale the print scaling factors <worksrcskip> and
 *		<workdstdup> so that they are both in the range 1..32767.
 *
 *		We now compute the two numbers by figuring out GCD.
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
 *		08/11/93 Wednesday (dcc) - created.
 *
*/
LOCAL short near ComputePrintRatio (
				long	 worksrcskip,
				long	 workdstdup,
				long	*outsrcskip,
				long	*outdstdup)
{
	long lBig, lSmall;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ComputePrintRatio";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (worksrcskip > workdstdup)
	{
		lBig	= worksrcskip;
		lSmall	= workdstdup;
	}
	else
	{
		lBig	= workdstdup;
		lSmall	= worksrcskip;
	}
	while (lBig % lSmall != 0)
	{
		long lm	= lBig % lSmall;
		lBig	= lSmall;
		lSmall	= lm;
	}
	*outsrcskip	= worksrcskip / lSmall;
	*outdstdup	= workdstdup / lSmall;

	if (*outsrcskip > 32767 || *outdstdup > 32767)
	{
		if (*outsrcskip > *outdstdup)
		{
			*outsrcskip	= *outsrcskip / *outdstdup;
			*outdstdup	= 1;
			if (*outsrcskip > 32767)
				*outsrcskip = 32767;
		}
		else
		{
			*outdstdup	= *outdstdup / *outsrcskip;
			*outsrcskip	= 1;
			if (*outdstdup > 32767)
				*outdstdup = 32767;
		}
	}
	return TRUE;
} /* ComputePrintRatio */
#else
/*********************************************************************
 *
 * ComputePrintRatio
 *
 * PURPOSE
 *		Scale the print scaling factors <worksrcskip> and
 *		<workdstdup> so that they are both in the range 1..255.
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
LOCAL short near ComputePrintRatio (
				long	 worksrcskip,
				long	 workdstdup,
				long	*outsrcskip,
				long	*outdstdup)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ComputePrintRatio";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		short	dotnum;
		long	pixelnum;
		long	mod;
		long	bestmod = -1;
		long	bestpix = -1;
		short	bestnum = -1;

		for (dotnum = 255; dotnum > 0; dotnum--) {
			pixelnum = dotnum * worksrcskip / workdstdup;
			mod      = dotnum * worksrcskip % workdstdup;
			if (pixelnum && pixelnum < 255) {
					if (!mod || mod > bestmod) {
						bestmod = mod;
						bestnum = dotnum;
						bestpix = pixelnum;
						if (!mod) {
							break;
						}
					}
			}
		}

		if (bestnum < 0) {
			return FALSE;
		}

		*outsrcskip = bestpix + (bestmod != 0);
		*outdstdup  = bestnum;

		return TRUE;

	}
} /* ComputePrintRatio */
#endif


/*********************************************************************
 *
 * ComputePrintData
 *
 * PURPOSE
 *		Figure out how much to scale each tile so we get desired
 *		end result (whether it be PAGES, INCHES, TILES/PAGE, or
 *		TILE WIDTH IN INCHES).
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		room != NULL
 *		FindNonEmptyLayer() returns != NULL
 *
 * RETURN VALUE
 *		
 *
 * HISTORY
 *		08/11/93 Wednesday (dcc) - fix not dealing with separated tiles
 *									properly. Fix not setting sizes properly
 *									using the SPECIFY SIZE BY PAGES option.
 *
*/
LOCAL short near ComputePrintData (RoomType *room, short sep)
{
	short			 roomwidth;
	short			 roomheight;
	short			 tilewidth;
	short			 tileheight;
	LayerType		*play = FindNonEmptyLayer(&room->Layers, room->FloorLayer);

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ComputePrintData";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	roomwidth  = play->ctilx;
	roomheight = play->ctily;
	tilewidth  = play->cxTile;
	tileheight = play->cyTile;

	{
		long	 srcsize;
		long	 destsize;
		long	 worksrcskip;
		long	 workdstdup;
//		long	 srctilesize;

		if (tilesacrosspage) {
			/* Printer pixels across tile */
			destsize = PRINT_PIXELS_ACROSS / tilesacrosspage - sep;

			/* pixels across tile */
			srcsize = tilewidth;
//			srctilesize = tilewidth;
		}

		if (tilesdownpage) {
			/* Printer pixels down tile */
			destsize = PRINT_PIXELS_DOWN / tilesdownpage - sep;

			/* pixels down tile */
			srcsize = tileheight;
//			srctilesize = tileheight;
		}			

		if (cencinTileA) {
		
			/* destsize is width of tile in printer pixels */
			destsize = cencinTileA * (long) PRINT_DPI / 100L - sep;
			
			/* pixels across tile */
			srcsize = tilewidth;
//			srctilesize = tilewidth;
		}

		if (cencinTileD) {
		
			/* Printer pixels down tile */
			destsize = cencinTileD * (long) PRINT_DPI / 100L - sep;
			
			/* pixels down tile */
			srcsize = tileheight;
//			srctilesize = tileheight;
		}


		if (pagesacross) {

			/* approx pixels across page */
			srcsize  = roomwidth / pagesacross + ((roomwidth % pagesacross) ? 1 : 0);

			/* Printer Pixels Across Page */
			destsize = PRINT_PIXELS_ACROSS - srcsize * sep;

			srcsize *= tilewidth;
//			srctilesize = tilewidth;
		}

		if (pagesdown) {

			/* approx pixels down page */
			srcsize  = roomheight / pagesdown + ((roomheight % pagesdown) ? 1 : 0);

			/* Printer Pixels Down Page */
			destsize = PRINT_PIXELS_DOWN - srcsize * sep;

			srcsize *= tileheight;
//			srctilesize = tileheight;
		}

		if (cencinPrintA) {

			/* printer pixels across printed document */
			destsize = cencinPrintA * (long) PRINT_DPI / 100L - roomwidth * sep;

			/* pixels across room */
			srcsize = roomwidth * tilewidth;
//			srctilesize = tilewidth;

		}

		if (cencinPrintD) {

			/* printer pixels down printed document */
			destsize = cencinPrintD * (long) PRINT_DPI / 100L - roomheight * sep;

			/* pixels down room */
			srcsize = roomheight * tileheight;
//			srctilesize = tileheight;

		}


		{
//			long	maxtilesize;
//			long	acttilesize;

			if (ComputePrintRatio (
					srcsize,
					destsize,
					&worksrcskip,
					&workdstdup)) {

				SrcSkip = (short) worksrcskip;
				DstDup  = (short) workdstdup;

//				maxtilesize = srctilesize * DstDup / SrcSkip;
//				if (!maxtilesize) {
//					maxtilesize = 1;
//				}
// 				acttilesize = (wSrcSizeToDstSize (tilewidth,  SrcSkip, DstDup) + sep);

//				if (acttilesize > maxtilesize) {
//					SrcSkip = tilewidth;
//					DstDup  = maxtilesize;
//				}
			}
		}

		/* cencinTileA & cencinTileD are width & height of tile in PIXELS */
		cencinTileA = (long) (wSrcSizeToDstSize (tilewidth,  SrcSkip, DstDup) + sep);
		cencinTileD = (long) (wSrcSizeToDstSize (tileheight, SrcSkip, DstDup) + sep);

		/* cencinPrintA & print height are width & height of map in 0.01 inches */
		cencinPrintA = cencinTileA * (long)roomwidth  * 100L / (long) PRINT_DPI;
		cencinPrintD = cencinTileD * (long)roomheight * 100L / (long) PRINT_DPI;

		tilesacrosspage = max((short) (PRINT_PIXELS_ACROSS / cencinTileA), 1);
		tilesdownpage   = max((short) (PRINT_PIXELS_DOWN / cencinTileD), 1);

		pagesacross = (roomwidth  + tilesacrosspage - 1) / tilesacrosspage;
		pagesdown   = (roomheight + tilesdownpage - 1)   / tilesdownpage;

		/* cencinTileA & cencinTileD are width & height of tile in 100th inch */
		cencinTileA = cencinTileA * 100L / (long) PRINT_DPI;
		cencinTileD = cencinTileD * 100L / (long) PRINT_DPI;
	}

	return TRUE;
} /* ComputePrintData */


/*********************************************************************
 *
 * ComputeNewPrintData
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
 *		08/17/93 Tuesday (dcc) - fix to handle numbers such as "1.08"
 *		07/11/94 Monday (dcc) - fix to handle numbers like ".5"
 *
 * SEE ALSO
 *
*/
LOCAL void ComputeNewPrintData (short newmode)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ComputeNewPrintData";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		long	across;
		long	down;
		long	ainch;
		long	dinch;
		long	adec;
		long	ddec;
		char c1 = 0;
		char c2 = 0;
		short	sep;

		across = 0;
		adec   = 0;
		down   = 0;
		ddec   = 0;
		c1     = 0;
		c2     = 0;

		if (Across[0] == '.')
		{
			sscanf (Across+1, "%c%c", &c1, &c2);
			across = 0L;
		}
		else
		{
			sscanf (Across, "%lu.%c%c", &across, &c1, &c2);
		}
		adec = (c1 ? c1 - '0' : 0) * 10 + (c2 ? c2 - '0' : 0);

		if (Down[0] == '.')
		{
			sscanf (Down+1, "%c%c", &c1, &c2);
			down = 0L;
		}
		else
		{
			sscanf (Down, "%lu.%c%c", &down, &c1, &c2);
		}
		ddec = (c1 ? c1 - '0' : 0) * 10 + (c2 ? c2 - '0' : 0);

		ainch  = across * 100L + adec;
		dinch  = down   * 100L + ddec;

		if ((DownMajor && !dinch) || (!DownMajor && !ainch)) {
			return;
		}

		ClearPrintData ();
		switch (SizeMode) {
		case PAGES_MODE:
			if (DownMajor) {
				pagesdown = (short) down;
			} else {
				pagesacross = (short) across;
			}
			break;
		case INCHES_MODE:
			if (DownMajor) {
				cencinPrintD = dinch;
			} else {
				cencinPrintA = ainch;
			}
			break;
		case TILES_PER_PAGE_MODE:
			if (DownMajor) {
				tilesdownpage = (short) down;
			} else {
				tilesacrosspage = (short) across;
			}
			break;
		case INCHES_PER_TILE_MODE:
			if (DownMajor) {
				cencinTileD = dinch;
			} else {
				cencinTileA = ainch;
			}
			break;
		}

		sep	 = 0 + ((PrintRoomWindow->CurrentRoom->Flags) & (DISPLAY_SEPERATED));
		ComputePrintData (PrintRoomWindow->CurrentRoom->Room, sep);

		SizeMode = newmode;

		switch (SizeMode) {
		case PAGES_MODE:
			itoa (pagesacross, Across, 10);
			itoa (pagesdown, Down, 10);
			break;
		case INCHES_MODE:
			sprintf (Down,   "%ld.%02ld", cencinPrintD / 100L, cencinPrintD % 100L);
			sprintf (Across, "%ld.%02ld", cencinPrintA / 100L, cencinPrintA % 100L);
			break;
		case TILES_PER_PAGE_MODE:
			itoa (tilesacrosspage, Across, 10);
			itoa (tilesdownpage, Down, 10);
			break;
		case INCHES_PER_TILE_MODE:
			sprintf (Down,   "%ld.%02ld", cencinTileD / 100L, cencinTileD % 100L);
			sprintf (Across, "%ld.%02ld", cencinTileA / 100L, cencinTileA % 100L);
			break;
		}

		NewUpdateStrings (MainWindow, &NumString[0], NUM_STRING_GADS, NULL);

	}
} /* ComputeNewPrintData */


/*********************************************************************
 *
 * DrawX
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
LOCAL void near DrawX (short x, short y, short on)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawX";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		struct	RastPort	 rp = { NULL, };

		SetPenColor (RtColors[RT_WHITE]);
		DrawRect (x + 1, y + 1, 7, 7);
 
		if (on) {
			StartEPicture (&rp, &XEPicture, x + 1, y + 1);
		}
	}
} /* DrawX */


/*********************************************************************
 *
 * DrawMark
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
LOCAL void near DrawMark (short x, short y, short on)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawMark";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		struct	RastPort	 rp = { NULL, };

		if (on) {
			SetPenColor (RtColors[RT_BLACK]);
		} else {
			SetPenColor (RtColors[RT_WHITE]);
		}
		StartEPicture (&rp, &MarkEPicture, x, y);
	}

} /* DrawMark */


/*********************************************************************
 *
 * UpdateOptions
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
LOCAL void near UpdateOptions (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateOptions";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if 0
	DrawX (RTDLG_LEFT + OPTION1_LEFT, RTDLG_TOP + OPTION1_TOP + 10 * 0, UniformPixels);
#endif
	DrawX (RTDLG_LEFT + OPTION1_LEFT, RTDLG_TOP + OPTION1_TOP + 10 * 1, FirstPageOnly);
	DrawX (RTDLG_LEFT + OPTION1_LEFT, RTDLG_TOP + OPTION1_TOP + 10 * 2, UseLightShades);

} /* UpdateOptions */


/*********************************************************************
 *
 * UpdateOrientation
 *
 * PURPOSE
 *		Display dot in cirle before either portrait or landscape.
 *
 * INPUT
 *		NONE.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		07/07/94 Thursday (dcc) - created.
 *
*/
LOCAL void near UpdateOrientation(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateOrientation";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	DrawMark (RTDLG_LEFT + ORIENTATION_LEFT + 4, RTDLG_TOP + ORIENTATION_TOP +  6,
				fPrintPortrait);
	DrawMark (RTDLG_LEFT + ORIENTATION_LEFT + 4, RTDLG_TOP + ORIENTATION_TOP + 16,
				!fPrintPortrait);
} /* UpdateOrientation */


/*********************************************************************
 *
 * UpdateRes
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
LOCAL void near UpdateRes (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateRes";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	DrawMark (RTDLG_LEFT + RES_LEFT + 4, RTDLG_TOP + RES_TOP +  6,
				PRINT_DPI == DPI_150);
	DrawMark (RTDLG_LEFT + RES_LEFT + 4, RTDLG_TOP + RES_TOP + 16,
				PRINT_DPI == DPI_300);
} /* UpdateRes */


/*********************************************************************
 *
 * UpdateShade
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
LOCAL void near UpdateShade (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateShade";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short	i;

		for (i = 0; i < 3; i++) {
			DrawMark (RTDLG_LEFT + SHADES_LEFT + 6,
						RTDLG_TOP + SHADES_TOP + 6 + 10 * i,
						i == PrintColorMode);
		}
	}
} /* UpdateShade */


/*********************************************************************
 *
 * UpdateSize
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
LOCAL void near UpdateSize (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateSize";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short	i;

		for (i = 0; i < 4; i++) {
			DrawMark (RTDLG_LEFT + SIZE_LEFT + 6,
						RTDLG_TOP + SIZE_TOP + 6+ 10 * i,
						i == SizeMode);
		}
	}
} /* UpdateSize */


/*********************************************************************
 *
 * SetOption
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
LOCAL void SetOption (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetOption";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		HitArea		*ha = hi->HitArea;
		short		num = ha->ID;

		switch (num) {
		case 0:
			#if 0
			UniformPixels = !UniformPixels;
			ComputeNewPrintData (SizeMode);
			#endif
			break;
		case 1:
			FirstPageOnly = !FirstPageOnly;
			break;
		case 2:
			UseLightShades = !UseLightShades;
			break;
		}
		UpdateOptions ();
	}
} /* SetOption */


/*********************************************************************
 *
 * SetShades
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
LOCAL void SetShades (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetShades";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		PrintColorMode = hi->HitArea->ID;
		UpdateShade ();
	}
} /* SetShades */


/*********************************************************************
 *
 * SetSize
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
LOCAL void SetSize (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetSize";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		ComputeNewPrintData (hi->HitArea->ID);
		UpdateSize ();
	}
} /* SetSize */


/*********************************************************************
 *
 * SetResVars
 *
 * PURPOSE
 *		Set other "constants" based on value of PRINT_DPI.
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
 *		07/08/94 Friday (dcc) - add support to print in portrait mode.
 *
 * SEE ALSO
 *
*/
LOCAL void near SetResVars (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetResVars";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	switch (OutputDevice)
	{
	case HPPCL3:
		cencxBorder = cencxBorderHPPCL3;
		cencyBorder = cencyBorderHPPCL3;
		break;
	case HPGL2:
		cencxBorder = cencxBorderHPGL2;
		cencyBorder = cencyBorderHPGL2;
		if (cencxPage > cencxPageMaxHPGL2)
			cencxPage = cencxPageMaxHPGL2;
		break;
	}

	if (fPrintPortrait)
	{
		PRINT_PIXELS_ACROSS		= (cencxPage - cencxBorder) * (long) PRINT_DPI / 100L;
		sizeofPrintBufferNew	= PRINT_PIXELS_ACROSS;
		PRINT_PIXELS_DOWN		= (cencyPage - cencyBorder) * (long) PRINT_DPI / 100L;
		if (IdiotNumbers)
			PRINT_PIXELS_DOWN	-= cencyIdiot * (long) PRINT_DPI / 100L;
	}
	else
	{
		PRINT_PIXELS_DOWN		= (cencxPage - cencxBorder) * (long) PRINT_DPI / 100L;
		sizeofPrintBufferNew	= PRINT_PIXELS_DOWN;
		PRINT_PIXELS_ACROSS		= (cencyPage - cencyBorder) * (long) PRINT_DPI / 100L;
		if (IdiotNumbers)
			PRINT_PIXELS_ACROSS	-= cencyIdiot * (long) PRINT_DPI / 100L;
	}
} /* SetResVars */


/*********************************************************************
 *
 * ReAllocateBuffers
 *
 * PURPOSE
 *		Reallocate buffers to new sizeofPrintBufferNew.
 *		Only allocate a buffer if new size is larger than old.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		SetResVars() has been called.
 *
 * RETURN VALUE
 *		Returns FALSE if unable to allocate (OOM).
 *
 * HISTORY
 *		09/23/93 Thursday (dcc) - created.
 *		07/08/94 Friday (dcc) - add support to print in portrait mode.
 *
*/
LOCAL BOOL ReAllocateBuffers(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReAllocateBuffers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!PrintBuffer || !LineBuf)
		sizeofBufferCurrent = 0L;

	if (sizeofBufferCurrent < sizeofPrintBufferNew)
	{
		if (LineBuf)
		{
			MEM_free(LineBuf);
			LineBuf = NULL;
		}
		if (PrintBuffer)
		{
			MEM_free(PrintBuffer);
			PrintBuffer = NULL;
		}

		/* The following assert is to make sure MEM_malloc()'s don't mess up */
		Assert(sizeofPrintBufferNew == (size_t) sizeofPrintBufferNew);

		if ((PrintBuffer = MEM_malloc((size_t) sizeofPrintBufferNew)) == NULL)
		{
			sizeofBufferCurrent = 0L;
			return FALSE;
		}

		if ((LineBuf = MEM_malloc((size_t) ((sizeofPrintBufferNew + 7L) / 8L + 1))) == NULL)
		{
			MEM_free(PrintBuffer);
			PrintBuffer = NULL;
			sizeofBufferCurrent = 0L;
			return FALSE;
		}
		sizeofBufferCurrent = sizeofPrintBufferNew;
	}

	if (fPrintPortrait)
	{
		PrintBM.width = (UWORD) PRINT_PIXELS_ACROSS;
		PrintBM.height = 1;
		Assert(PrintBM.width == PRINT_PIXELS_ACROSS);
	}
	else
	{
		PrintBM.width = 1;
		PrintBM.height = (UWORD) PRINT_PIXELS_DOWN;
		Assert(PrintBM.height == PRINT_PIXELS_DOWN);
	}
	PrintBM.data = PrintBuffer;

	return TRUE;
} /* ReAllocateBuffers */


/*********************************************************************
 *
 * SetOrientation
 *
 * PURPOSE
 *		Set either portrait or landscape printing orientation.
 *
 * INPUT
 *		HitInfo *phi	: portrait or landscape button hit?
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		07/07/94 Thursday (dcc) - created.
 *
*/
LOCAL void SetOrientation(HitInfo *phi)
{
	BOOL fPrintPortraitOld = fPrintPortrait;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetOrientation";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	fPrintPortrait = (phi->HitArea->ID == 0);
	if (fPrintPortraitOld != fPrintPortrait)
	{
		SetResVars();
		ReAllocateBuffers();
		ComputeNewPrintData (SizeMode);
		UpdateOrientation();
	}

} /* SetOrientation */


/*********************************************************************
 *
 * SetRes
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
 *		09/23/93 Thursday (dcc) - updated to reallocate buffers if
 *								switching from 150 DPI to 300 DPI
 *
 * SEE ALSO
 *
*/
LOCAL void SetRes (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetRes";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		short OldDPI = PRINT_DPI;

		if (hi->HitArea->ID)
			PRINT_DPI = DPI_300;
		else
			PRINT_DPI = DPI_150;
		SetResVars ();
		if (OldDPI != PRINT_DPI)	/* change buffer size? */
		{
			if (!ReAllocateBuffers())
			{
				PRINT_DPI = DPI_150;
				SetResVars ();
				ReAllocateBuffers();
			}
		}
		ComputeNewPrintData (SizeMode);
		UpdateRes ();
	}
} /* SetRes */


/*********************************************************************
 *
 * LHighlightString
 *
 * SYNOPSIS
 *		void LHighlightString (HitInfo *hi)
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
LOCAL void LHighlightString (HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LHighlightString";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		static short OldMajor = -1;

		DownMajor = hi->HitArea->ID;

		if (OldMajor != DownMajor) {
			OldMajor = DownMajor;
			ComputeNewPrintData (SizeMode);
		}

		HighlightString (hi);
	}

} /* LHighlightString */


/*********************************************************************
 *
 * IncDecNumber
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
 *		08/18/93 Wednesday (dcc) - fix so you can increment numbers like "1.08"
 *								 - scale increment amount to size of number
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
		long		 val;
		long		 dec;

		DownMajor = num;

		switch (SizeMode) {
		case PAGES_MODE:
		case TILES_PER_PAGE_MODE:
			val = atoi (Nums[num]) * 100;
			if (dir > 0)
			{
				if		(val < 1000L)
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
				if		(val <= 1000L)
					val -=  100L;
				else if (val <= 2000L)
					val -=  200L;
				else if (val <= 5000L)
					val -=  500L;
				else
					val -= 1000L;
			}
			break;
		case INCHES_MODE:
		case INCHES_PER_TILE_MODE:
			{
				char c1 = 0;
				char c2 = 0;

				if (*Nums[num] == '.')
				{
					sscanf (Nums[num]+1, "%c%c", &c1, &c2);
					val = 0L;
				}
				else
				{
					sscanf (Nums[num], "%lu.%c%c", &val, &c1, &c2);
				}
				dec = (c1 ? c1 - '0' : 0) * 10 + (c2 ? c2 - '0' : 0);
				val = val * 100 + dec;

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
			}
			break;
		}
		val = max (0L, val);
		val = min (3276600L, val);

		dec = val % 100;
		val = val / 100;

		switch (SizeMode) {
		case PAGES_MODE:
		case TILES_PER_PAGE_MODE:
			itoa ((short)val, Nums[num], 10);
			break;
		case INCHES_MODE:
		case INCHES_PER_TILE_MODE:
			sprintf (Nums[num], "%ld.%02ld", val, dec);
			break;
		}
		BeforeGraphics ();
		ComputeNewPrintData (SizeMode);
		SelectString (&NumString[num]);
		#if 0
		UpdateEditString (&NumString[num]);
		#endif
		AfterGraphics ();

	}
} /* IncDecNumber */

/********************************* Gadgets ********************************/

LOCAL HitArea RTHitArea[] = {
	/* Across Number */
	{ RTNUM_LEFT, RTNUM1_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x10, IncDecNumber, },
	{ RTNUM_LEFT + ARROW_WIDTH, RTNUM1_TOP, NUMBER_WIDTH - 2, NUMBER_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 0, LHighlightString, &NumString[NUM_ACROSS_NDX], },
	{ RTNUM_LEFT + ARROW_WIDTH + NUMBER_WIDTH - 1, RTNUM1_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x00, IncDecNumber, },

	/* Down Number */
	{ RTNUM_LEFT, RTNUM2_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x11, IncDecNumber, },
	{ RTNUM_LEFT + ARROW_WIDTH, RTNUM2_TOP, NUMBER_WIDTH - 2, NUMBER_HEIGHT,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP, 1, LHighlightString, &NumString[NUM_DOWN_NDX], },
	{ RTNUM_LEFT + ARROW_WIDTH + NUMBER_WIDTH - 1, RTNUM2_TOP, ARROW_WIDTH, ARROW_HEIGHT,
		HTA_COMPLEMENT, 0x01, IncDecNumber, },

	/* Size */
	{ SIZE_LEFT + 6, SIZE_TOP + 6 + 10 * 0, SIZE_WIDTH - 15, 9,
		0, 0, SetSize, },
	{ SIZE_LEFT + 6, SIZE_TOP + 6 + 10 * 1, SIZE_WIDTH - 15, 9,
		0, 1, SetSize, },
	{ SIZE_LEFT + 6, SIZE_TOP + 6 + 10 * 2, SIZE_WIDTH - 15, 9,
		0, 2, SetSize, },
	{ SIZE_LEFT + 6, SIZE_TOP + 6 + 10 * 3, SIZE_WIDTH - 15, 9,
		0, 3, SetSize, },

	/* Shade */
	{ SHADES_LEFT + 6, SHADES_TOP + 6 + 10 * 0, SHADES_WIDTH - 15, 9,
		0, 0, SetShades, },
	{ SHADES_LEFT + 6, SHADES_TOP + 6 + 10 * 1, SHADES_WIDTH - 15, 9,
		0, 1, SetShades, },
	{ SHADES_LEFT + 6, SHADES_TOP + 6 + 10 * 2, SHADES_WIDTH - 15, 9,
		0, 2, SetShades, },

	/* Orientation */
	{ ORIENTATION_LEFT + 4, ORIENTATION_TOP + 6 + 10 * 0, ORIENTATION_WIDTH - 8, 9,
		0, 0, SetOrientation, },
	{ ORIENTATION_LEFT + 4, ORIENTATION_TOP + 6 + 10 * 1, ORIENTATION_WIDTH - 8, 9,
		0, 1, SetOrientation, },

	/* Resolution */
	{ RES_LEFT + 4, RES_TOP + 6 + 10 * 0, RES_WIDTH - 8, 9,
		0, 0, SetRes, },
	{ RES_LEFT + 4, RES_TOP + 6 + 10 * 1, RES_WIDTH - 8, 9,
		0, 1, SetRes, },

	/* Options */
	{ OPTION1_LEFT, OPTION1_TOP + 10 * 0, OPTION1_WIDTH - 15, 9,
		0, 0, SetOption, },
	{ OPTION1_LEFT, OPTION1_TOP + 10 * 1, OPTION1_WIDTH - 15, 9,
		0, 1, SetOption, },
	{ OPTION1_LEFT, OPTION1_TOP + 10 * 2, OPTION1_WIDTH - 15, 9,
		0, 2, SetOption, },

	/* Print / Cancel */
	{ PRINT_LEFT, PRINT_TOP, PRINT_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, PRINT_ID, NULL},
	{ CANCEL_LEFT, CANCEL_TOP, CANCEL_WIDTH, BUTTON_HEIGHT,
		HTA_COMPLEMENT, CANCEL_ID, NULL},
	{ HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, },
};


/*********************************************************************
 *
 * PrintMapReq
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
 *		09/22/93 Wednesday (dcc) - call SetResVars() upon entry into function.
 *
 * SEE ALSO
 *
*/
short PrintMapReq (RoomWindowType *roomwindow)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PrintMapReq";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		SaveAreaType		*sa = NULL;
		HitArea				*ha;
		struct	RastPort	 rp = { NULL, };
		short				 RTprint;

		SetResVars ();
		if (!ReAllocateBuffers())
			return FALSE;

		if ((cLookup = MEM_malloc(MAX_CI_COLORS)) == NULL)
			return FALSE;

		SetColorsNPointer (MainWindow);

		RTdone    = FALSE;
		RTsuccess = FALSE;

		PrintRoomWindow = roomwindow;
		RTWin   = MainWindow;
		RTFont  = &MainFont;
		RTSFont = &SmallFont;
		RtColors[RT_WHITE ] = RTWin->White;
		RtColors[RT_BLACK ] = RTWin->Black;
		RtColors[RT_BLUE  ] = RTWin->Blue;
		RtColors[RT_ORANGE] = RTWin->Orange;

		do {
			RTprint = FALSE;
			if (PushWindowClipValues (RTWin)) {
				sa = SaveArea (RTDLG_LEFT, RTDLG_TOP, RTDLG_WIDTH, RTDLG_HEIGHT);
				if (!sa) {
					RTsuccess = TRUE;
					GEcatf ("\nOOM: PrintMapReq Area");
					goto dRTcleanup;
				}

				BeforeGraphics ();

				StartEPicture (&rp, &RTEPicture, RTDLG_LEFT, RTDLG_TOP);

				DrawString (RTFont, RTDLG_LEFT + RTDLG_WIDTH / 2 - StringLen (RTFont, "Print Room") / 2, RTDLG_TOP + 2, "Print Room");

				ComputeNewPrintData (SizeMode);

				UpdateStrings (MainWindow, &NumString[0], NUM_STRING_GADS, NULL);

				UpdateOptions ();
				UpdateOrientation();
				UpdateRes ();
				UpdateShade ();
				UpdateSize ();

				AfterGraphics ();

				while (!RTdone) {
					short	key;
					short	status;

					ha = CheckHitAreas (RTHitArea, RTDLG_LEFT, RTDLG_TOP);

					if (ha) {
						if (ha->ID == PRINT_ID) {
							ComputeNewPrintData (SizeMode);
							RTdone  = TRUE;
							RTprint = TRUE;
							break;
						}
						if (ha->ID == CANCEL_ID) {
							RTdone  = TRUE;
							break;
						}
					}
					status = CheckEditKeys (&key);
					if (status == -1)
					{
						RTdone  = TRUE;
						break;
					}
					if (status) {
						ComputeNewPrintData (SizeMode);
					}
				}

dRTcleanup:
				if (sa)		RestoreArea (sa);
				FlushKeyboardBuffer ();

				PopClipValues ();
			}

			if (RTprint) {
				if (SrcSkip && DstDup) {
					wPrtSrcX  = SrcSkip;
					wPrtSrcY  = SrcSkip;
					wPrtDestX = DstDup;
					wPrtDestY = DstDup;
					if (!PrintRoom (roomwindow)) {
						RTdone = FALSE;
					}
				} else {
					RTdone = FALSE;
				}
			}

		} while (!RTdone);

		MEM_free(cLookup);
		MEM_free(LineBuf);					LineBuf = NULL;
		MEM_free(PrintBuffer);				PrintBuffer = NULL;
		RestoreColorsNPointer (MainWindow);
		return TRUE;
	}
} /* PrintMapReq */

