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
 * tUME.C
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 08/29/89
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		Why, the Universal Map Editor, of course!
 *
 * HISTORY
 *		08/29/89 Tuesday - Created main. (RGM)
 *		10/14/93 Thursday (dcc) - updated for MS-DOS Watcom C32.
 *		07/18/94 Monday (dcc) - reduced BCC stack size from 12K to 10K.
 *
*/
#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"							// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#if __AMIGAOS__
#include <exec/types.h>
#include <exec/memory.h>

#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include <intuition/intuition.h>
#include <intuition/preferences.h>

#include <workbench/startup.h>

#include <stdio.h>

#if AZTEC_C
#include <functions.h>
#endif
#if LATTICE
#include <proto/exec.h>
#include <proto/intuition.h>
#endif

#include "tuglbl.h"
#include "pointer.h"
#include "defs.h"
#include "sizedefs.h"
#include "szerror.h"

#include <echidna/picture.h>
#include <echidna/events.h>
#elif __MSDOS__
//!!#include <echidna/grafx.h>
//!!#include <echidna/fonts.h>
#include <echidna/eerrors.h>
#include <echidna/brushsup.h>
//!!#include <echidna/listfunc.h>
//!!#include <echidna/menus2.h>
//!!#include <echidna/eui.h>
//!!#include <echidna/windows.h>
//!!#include <echidna/hitareas.h>
//!!#include "keyboard.h" //<echidna/keyboard.h>
//!!#include <echidna/mouse.h>
#include <echidna/exit.h>
#include <echidna/recorder.h>

//!!#include <stdio.h>
//!!#include <conio.h>
//!!#include <dos.h>
//!!#include <io.h>

//!!#include "tufunc.h"
#include "tuglbl.h"

#include "pane.h"
#include "stubs.h"	// KLUDGE

#include "colorreq.h"
#include "events.h"
//!!#include "icons.h"
#include "parseini.h"
#include "download.h"
#include "search.h"
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray
#include "szerror.h"
#if DIV0TRAP
#include "div0trap.h"
#endif
#if NULLPTR
#include "nullptr.h"
#endif
#if STACKCHECK
#include "stackchk.h"
#endif
#include "tumemain.h"						/* Verify function prototypes. */
//!!#include "malloclow.h"
#include "objroom.h"

#define DS(msg)	\
	SetPenColor (255);						\
	DrawRect (10, 150, 25*8, 8);			\
 	DrawString (&MainFont, 10, 150, msg);

//!!#include "euicolor.h"

BPIType	*FontBPI;
BPIType	*IconsBPI;
GShape	**Icons;

short ExitProgram = FALSE;

EasyFileReq	**EasyFR;
FileReqInt	EasyFRI[] = {
		/* TITLE					PATH		NAME */

	{	"Load/Save Map",			"",		""	},
	{	"Load Append/Save Room",		"", 		""	},
	{	"Load Tiles",				"",		""	},
	{	"Save Tiles",				"",		""	},
	{	"Choose a File",			"",		""	},
	{	"Save IFF Picture",			"",		""	},
	{	"Load/Save Palette",		"",		""	},
	{	NULL, 					NULL,	NULL	}
};

extern void Initialize (void);
extern void *SetPaneToEditState;
//extern void *SetPaneToObjectsState;
//extern void *SetPaneToPartsState;

//extern EventState ToggleTitleBarState;
extern EventState SpaceToggleState;
//extern EventState ToggleShowBrushState;

extern unsigned _stklen = 10240U;

#endif/*__AMIGAOS__/__MSDOS__*/

/**************************** C O N S T A N T S ***************************/

#define WMODES1		BORDERLESS|ACTIVATE|SMART_REFRESH
#define WMODES2		BACKDROP
#define WMODES		WMODES1|WMODES2

#define TUMETITLE	"tUME"

#define LIB_REV		(33L)

#define INI_FILENAME "tUME.ini"
#define FPF_FILENAME	"tUME.fpf"

/******************************** T Y P E S *******************************/


/****************************** E X T E R N S *****************************/

extern struct TextFont	 AnimSeqSmallFont;

#if __MSDOS__

extern int MouseX;
extern int MouseY;
extern int LeftButton;
extern int RightButton;

extern short SetPaneToEdit (EventInfo *ei);
#endif/*__MSDOS__*/

extern BOOL TE_PointerFlag;

/****************************** G L O B A L S *****************************/

char Name[] = "tUME";
char Copyright[] = "Copyright \177 1989-2000 Echidna";

static char szIni[] = INI_FILENAME;

#if FUNC_NAMES
char *CurrentFuncName = "(NO FUNCTION NAME INFO)";
#endif

char *PreFix = "";
char *NoCanDo = NCD;

#if __AMIGAOS__
struct	IntuitionBase	*IntuitionBase		= NULL;
struct	GfxBase			*GfxBase			= NULL;
struct	ArpBase			*ArpBase			= NULL;

struct	Process			*OurTask			= NULL;
#endif
struct	Window			*old_pr_WindowPtr	= NULL;

#if __AMIGAOS__
struct	Screen			*MainScreen			= NULL;
struct	Window			*MainWindow			= NULL;
struct	RastPort		*MainRp				= NULL;
struct	ViewPort		*MainVp				= NULL;	/* GAT */

struct	Window			*ActiveWindow		= NULL;

struct	Preferences	UserPrefs;
struct	Screen		WBScreenCopy;

int	ClipLeft	= 0;
int	ClipTop		= 0;
int	ClipWidth	= WIDTH;
int	ClipHeight	= HEIGHT;

#endif/*__AMIGAOS__*/
int	TopAdd		= 0;
#if __AMIGAOS__

struct	TextFont	*asfont = 0;

struct TextAttr ASmallFont = {
	(UBYTE *)"AnimSeqSmall.font",	/* Font Name	*/
	5,								/* Font Height	*/
	0,								/* Style		*/
	NULL							/* Preferences	*/
};

int	NormalScreenWidth	= WIDTH;
int	NormalScreenHeight	= HEIGHT;

int	ScreenWidth;
int	ScreenHeight;

TileSetType		*GlobalTileSet		= NULL;
RoomType		*GlobalRoom			= NULL;
#endif/*__AMIGAOS__*/
RoomType		*UndoRoom			= NULL;
#if __AMIGAOS__
RoomWindowType	*GlobalRoomWindow	= NULL;

TileSpaceType		*GlobalTileSpace		= NULL;
MapType				*GlobalMap				= NULL;
#endif/*__AMIGAOS__*/
RoomWindowListType	*GlobalRoomWindowList	= NULL;
#if __AMIGAOS__

BlockCopyType		*GlobalBlockCopy;

RGM_MPointerType		*GlobalPointer	= NULL;
#elif __MSDOS__
short				 GlobalPointer = -1;
#endif/*__AMIGAOS__/__MSDOS__*/

WORD		LeftField;
WORD		RightField;
WORD		TopField;
WORD		BottomField;

#if __AMIGAOS__
int LastTBar;

struct TextFont *TopazFont;
struct TextFont *EchidnaFont;

struct TextAttr NormFont = {
	(UBYTE *)"topaz.font",	/* Font Name	*/
	8,						/* Font Height	*/
	FS_NORMAL,				/* Style		*/
	FPF_ROMFONT				/* Preferences	*/
};

struct	NewWindow	NewWindow = {
	0, 0, WIDTH, HEIGHT, 0, 1,	
	ACTIVEMODE, WMODES, 
	NULL, NULL, NULL, NULL, NULL,	
	0,0,0,0, CUSTOMSCREEN	
};

struct NewScreen NewScreen = {
	0, 0, WIDTH, HEIGHT, 5, 0, 1, NULL,
	CUSTOMSCREEN, NULL, (UBYTE *) TUMETITLE, NULL, NULL,
};

/**************************************************************************/


UWORD __chip BlankData [] = {
	0, 0,

	0x0000, 0x0000,

	0, 0
};

RGM_MPointerType	BlankMPointer = {
	BlankData, 1, 16, 0, 0
};


UWORD __chip StopData [] = {
	0, 0,

	0x0000,0x0002,
	0x0000,0x0c61,
	0x0000,0x1291,
	0x0000,0x0922,
	0x0000,0x07ec,
	0x0180,0x0ffe,
	0x0990,0x1ffb,
	0x0990,0x3fff,
	0x0980,0xdfff,
	0x0000,0x0fe6,
	0x0000,0x0490,
	0x0000,0x0248,
	0x0000,0x0490,

	0, 0
};

RGM_MPointerType	YieldMPointer = {
	StopData, 13, 16, (SHORT) -7, (SHORT) -7
};


UWORD __chip FillData [] = {
   0, 0,

   0x8000,0x4000,
   0x4000,0xE000,
   0x2000,0x7800,
   0x1800,0x3E00,
   0x1E00,0x3F00,
   0x0F00,0x1F80,
   0x0B80,0x1E40,
   0x07C0,0x0C20,
   0x02E0,0x0510,
   0x01F0,0x0208,
   0x00B0,0x0148,
   0x0070,0x008A,
   0x000A,0x007F,
   0x0004,0x000E,
   0x0008,0x001C,
   0x0000,0x0008,

   0, 0
};

RGM_MPointerType	FillMPointer = {
	FillData, 16, 16, (SHORT) 0, (SHORT) 0
};


UWORD __chip GrabData [] = {
   0, 0,

   0x0100,0x0100,
   0x0100,0x0100,
   0x0100,0x0100,
   0x0100,0x0100,
   0x0100,0x0820,
   0x0100,0x0440,
   0x0000,0x0000,
   0xFC7E,0xF01E,
   0x0000,0x0000,
   0x0100,0x0440,
   0x0100,0x0820,
   0x0100,0x0100,
   0x0100,0x0100,
   0x0100,0x0100,
   0x0100,0x0100,

   0, 0
};

RGM_MPointerType	GrabMPointer = {
	GrabData, 15, 16, (SHORT) (-7), (SHORT) (-7)
};


UWORD __chip C1Data [] = {

   0, 0,

   0x0000,0x0620,
   0x0620,0x0F70,
   0x0860,0x1EF0,
   0x0820,0x1C70,
   0x0820,0x1E70,
   0x0670,0x0FF8,
   0x0000,0x0E70,
   0x0E70,0x1FF8,
   0x1008,0x3E7C,
   0x2004,0x700E,
   0x4002,0xE007,
   0x4002,0xE007,
   0x0000,0x4002,
   0x4002,0xE007,
   0x4002,0xE007,
   0x2004,0x700E,
   0x1008,0x3E7C,
   0x0E70,0x1FF8,
   0x0000,0x0E70,

   0, 0
};

RGM_MPointerType	C1MPointer = {
	C1Data, 19, 16, (SHORT) -7, (SHORT) -12
};


UWORD __chip C2Data[] = {

   0, 0,

   0x0000,0x0660,
   0x0660,0x0FF0,
   0x0810,0x1E78,
   0x0820,0x1C70,
   0x0840,0x1EF0,
   0x0670,0x0FF8,
   0x0000,0x0E70,
   0x0E70,0x1FF8,
   0x1008,0x3E7C,
   0x2004,0x700E,
   0x4002,0xE007,
   0x4002,0xE007,
   0x0000,0x4002,
   0x4002,0xE007,
   0x4002,0xE007,
   0x2004,0x700E,
   0x1008,0x3E7C,
   0x0E70,0x1FF8,
   0x0000,0x0E70,

   0, 0
};

RGM_MPointerType	C2MPointer = {
	C2Data, 19, 16, (SHORT) -7, (SHORT) -12
};


UWORD __chip C3Data[] = {

   0, 0,

   0x0000,0x0670,
   0x0670,0x0FF8,
   0x0810,0x1E78,
   0x0830,0x1C78,
   0x0810,0x1E78,
   0x0670,0x0FF8,
   0x0000,0x0E70,
   0x0E70,0x1FF8,
   0x1008,0x3E7C,
   0x2004,0x700E,
   0x4002,0xE007,
   0x4002,0xE007,
   0x0000,0x4002,
   0x4002,0xE007,
   0x4002,0xE007,
   0x2004,0x700E,
   0x1008,0x3E7C,
   0x0E70,0x1FF8,
   0x0000,0x0E70,

   0, 0
};

RGM_MPointerType	C3MPointer = {
	C3Data, 19, 16, (SHORT) -7, (SHORT) -12
};



UWORD __chip C4Data[] = {

   0, 0,

   0x0000,0x0650,
   0x0650,0x0FF8,
   0x0850,0x1EF8,
   0x0870,0x1CF8,
   0x0810,0x1E78,
   0x0610,0x0F38,
   0x0000,0x0E70,
   0x0E70,0x1FF8,
   0x1008,0x3E7C,
   0x2004,0x700E,
   0x4002,0xE007,
   0x4002,0xE007,
   0x0000,0x4002,
   0x4002,0xE007,
   0x4002,0xE007,
   0x2004,0x700E,
   0x1008,0x3E7C,
   0x0E70,0x1FF8,
   0x0000,0x0E70,

   0, 0
};


RGM_MPointerType	C4MPointer = {
	C4Data, 19, 16, (SHORT) -7, (SHORT) -12
};


/**************************************************************************/


EasyFileReq	**EasyFR	= NULL;

FileReqInt	EasyFRI[] = {
		/* TITLE					PATH		NAME */

		"Load/Save Map",			"",			"",
		"Load Append/Save Room",	"", 		"",
		"Xave Map",					"",			"",
		"Load Tiles",				"",			"",
		"Save Tiles",				"",			"",
		"Choose a File",			"",			"",
		NULL, 						NULL, 		NULL
};

int		ShowCursor;
int		ShowTUser;

SHORT	TextTop;
#endif/*__AMIGAOS__*/
BOOL	OneLayerCopy;
short	GlobalBackground = 0;

short	TU_DrawMode		= DMODE_DRAW;


#if __MSDOS__

/**************************************************************************/

void ProcessMenus (HitInfo *hi);

short lastMouseY = TITLE_HEIGHT+1;
short lastMenuHeight = 0;

ByteMap  DisplayBMX;
ByteMap *DisplayBM = &DisplayBMX;
ByteMap  WorkBMX;
ByteMap *WorkBM = &WorkBMX;
ByteMap  BackBMX;
ByteMap *BackBM = &BackBMX;
BigByteMap	 BigBackBMX;
BigByteMap	*BigBackBM = &BigBackBMX;

//PartsPic	**PicTable;
//ListType	 PicListX;
//ListType	*PicList = &PicListX;
//ListType	 DGridListX;
//ListType	*DGridList = &DGridListX;
//ListType	 ObjListX;
//ListType	*ObjList = &ObjListX;
//short		 PicNumber = 1;

Pane	 Pane1X = { &GlobalAreas[0], 0, { PANE_PARTS, PANEF_SPACED, }, };
Pane	*Pane1  = &Pane1X;
Pane	*ActivePane = &Pane1X;
Pane	*Panes[] = {
	&Pane1X,
	NULL,
};

HitArea	GlobalAreas[] = {
	{   0,   0, 320,   1,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_REPORTRIGHT, 0, ProcessMenus, NULL,
	},
	{   0,   0, 320, 200,
		HTA_DOWN | HTA_REPEAT | HTA_INSURE_UP | HTA_REPORTRIGHT, 0, ProcessPane, &Pane1X,
	},
	{ HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, HTA_LAST_AREA, },
};
#endif/*__MSDOS__*/

/******************************* L O C A L S ******************************/

#if __MSDOS__
static SaveAreaType *psaMenuBar = NULL;
#endif/*__MSDOS__*/

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


#if __MSDOS__


/*********************************************************************
 *
 * AutoHideMenus
 *
 * SYNOPSIS
 *		BOOL AutoHideMenus(void)
 *
 * PURPOSE
 *		If we made the menus appear, then we should make them disappear.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns TRUE if we made auto-magic menus disappear.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL AutoHideMenus(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AutoHideMenus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (psaMenuBar)
	{
		RestoreArea(psaMenuBar);
		psaMenuBar = NULL;
		return TRUE;
	}
	return FALSE;
} /* AutoHideMenus */


/*********************************************************************
 *
 * AutoShowMenus
 *
 * SYNOPSIS
 *		void AutoShowMenus(void)
 *
 * PURPOSE
 *		Make menus automagically appear if pointer is on topmost line.
 *
 *		You must check yourself for the condition that the pointer
 *		is indeed on the topmost line!
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
void AutoShowMenus(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AutoShowMenus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!JamPaletteSTATE)
		SetColorsNPointer(MainWindow);

	if (!ShowTitleSTATE)
	{
		if (!psaMenuBar)
		{
			psaMenuBar = SaveArea(0, 0, SCREEN_WIDTH, TITLE_HEIGHT);
		}
		ShowTitleSTATE = TRUE;
		DrawTitleBar();
		ShowTitleSTATE = FALSE;
	}
} /* AutoShowMenus */


/*********************************************************************
 *
 * ProcessMenus
 *
 * SYNOPSIS
 *		void ProcessMenus (HitInfo *hi)
 *
 * PURPOSE
 *		Processes EUI Events in the most standard way.
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
void ProcessMenus (HitInfo *hi)
{

	UWORD Buttons = hi->Buttons;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessMenus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	
	if (Buttons & MOUSE_FIRSTDOWN)
	{
		MouseInfo	mi;

		/* Did we put up the menus (hiddens status bar?) */

		AutoHideMenus();
		SetColorsNPointer(MainWindow);
		mi.X = hi->X;
		mi.Y = hi->Y;
		mi.Buttons = hi->Buttons;
//		ReadMouse (&mi);
		HandleMenus (MainWindow, &mi);
		RestoreColorsNPointer(MainWindow);
		lastMouseY = TITLE_HEIGHT+1;	/* Force call to SetColors() in main if needed */
	}

} /* ProcessMenus */
#endif/*__MSDOS__*/


/**************************************************************************
 *
 * TumeSetUpFirst
 *
 * SYNOPSIS
 *		void TumeSetUpFirst (void)
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
void TumeSetUpFirst (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TumeSetUpFirst";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	TrackingRoomWindow	= NULL;
	ShowCursor			= FALSE;
	ShowTUser			= FALSE;
	OneLayerCopy		= FALSE;

	if ( (GlobalTileSpace = CreateTileSpace ()) == NULL) {
/**/	goto ABORT;
	}

	if ( (GlobalMap = CreateMap ()) == NULL) {
/**/	goto ABORT;
	}

	if ( (GlobalRoomWindowList = MEM_calloc (1, ROOMWINDOWLISTSIZE)) == NULL) {
/**/	goto ABORT;
	}
	else {
		InitList (&(GlobalRoomWindowList->RoomWindows));
	}

	if ( (GlobalBlockCopy = MEM_calloc (1, BLOCKCOPYSIZE)) == NULL) {
/**/	goto ABORT;
	}
	else {
		InitList (&(GlobalBlockCopy->Layers));
	}

#if __AMIGAOS__
	GetPrefs ((struct Preferences *) &UserPrefs, sizeof (struct Preferences));

	if (! (GetScreenData ((void *) &WBScreenCopy, sizeof (struct Screen),
						WBENCHSCREEN, NULL))) {

		ScreenWidth		= NormalScreenWidth;
		ScreenHeight	= NormalScreenHeight;

	}
	else {
		ScreenWidth		= WBScreenCopy.Width / 2;
		ScreenHeight	= WBScreenCopy.Height;
	}

	if (UserPrefs.LaceWB) {
		ScreenHeight = ScreenHeight / 2;
	}

	NewScreen.Width		= ScreenWidth;
	NewScreen.Height	= ScreenHeight;

	NewWindow.Width		= ScreenWidth;
	NewWindow.Height	= ScreenHeight;

	ClipLeft		= 0;
	ClipTop			= 0;
	ClipWidth		= ScreenWidth;
	ClipHeight		= ScreenHeight;

	GlobalPointer = NULL;

	LeftField = RightField = TopField = BottomField = 2;
#endif/*__AMIGAOS__*/

	EditRoom ();

	return;

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:

	if (GlobalBlockCopy) {
		MEM_free (GlobalBlockCopy);
		GlobalBlockCopy = NULL;
	}

	if (GlobalRoomWindowList) {
		MEM_free (GlobalRoomWindowList);
		GlobalRoomWindowList = NULL;
	}

	GlobalMap = DeleteMap (GlobalMap);

	GlobalTileSpace = DeleteTileSpace (GlobalTileSpace);

	Quit ("Out of memory.");

} /* TumeSetUpFirst */


/**************************************************************************
 *
 * TumeSetUpLast
 *
 * SYNOPSIS
 *		void TumeSetUpLast (void)
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
 *		10/04/89 Wednesday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void TumeSetUpLast (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TumeSetUpLast";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	ShockColors (MainWindow);
#elif __MSDOS__
	CyclePause = TRUE;
	InitMenuColors(&DefaultColorInfo);
#endif/*__AMIGAOS__/__MSDOS__*/

	if (!SetGlobalColors (&DefaultColorInfo))
/**/	goto ABORT;

	SetColorsNPointer (MainWindow);

	if ( (GlobalRoomWindow = CreateRoomWindow (MainWindow)) == NULL) {
/**/	goto ABORT;
	}
	else {
		AddTail (&(GlobalRoomWindowList->RoomWindows), GlobalRoomWindow);
	}

	LastRealTBar	= SHOW_TITLE;
	LastTBar		= LastRealTBar;
	ShowState (SHOW_VERSION);

	TopAdd = ShowTitleSTATE ? 11 : 0;

	return;

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:

	if (GlobalRoomWindow) {
		MEM_free (GlobalRoomWindow);
		GlobalRoomWindow = NULL;
	}
	Quit ("Out of memory.");

} /* TumeSetUpLast */



/**************************************************************************
 *
 * PreTumeCleanUp
 *
 * SYNOPSIS
 *		void PreTumeCleanUp (void)
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
 *		10/04/89 Wednesday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void PreTumeCleanUp (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PreTumeCleanUp";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetGlobalColors (&DefaultColorInfo);

	if (MainWindow) {
		SetColorsNPointer (MainWindow);
	}

	if (UndoRoom) {
		DeAllocateRoom (UndoRoom);
		UndoRoom = NULL;
	}
	return;

} /* PreTumeCleanUp */


/**************************************************************************
 *
 * PostTumeCleanUp
 *
 * SYNOPSIS
 *		void PostTumeCleanUp (void)
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
void PostTumeCleanUp (void)
{
	RoomWindowType	*roomwindow;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PostTumeCleanUp";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalBlockCopy) {
		DeAllocateLayers (&(GlobalBlockCopy->Layers));
		MEM_free (GlobalBlockCopy);
		GlobalBlockCopy = NULL;
	}

	if (GlobalRoomWindowList) {
		while ((roomwindow = RemHead (&(GlobalRoomWindowList->RoomWindows))) != NULL) {
			DeAllocateRoomWindow (roomwindow);
		}
		MEM_free (GlobalRoomWindowList);
		GlobalRoomWindowList	= NULL;
		GlobalRoomWindow		= NULL;
	}

	ClearSearchBuffer();

	GlobalMap = DeleteMap (GlobalMap);

	GlobalTileSpace = DeleteTileSpace (GlobalTileSpace);

	TrackingRoomWindow = NULL;

	return;

} /* PostTumeCleanUp */


/**************************************************************************
 *
 * NewMap
 *
 * SYNOPSIS
 *		int NewMap (void)
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
int NewMap (void)
{

#if __AMIGAOS__
	void	*old_pointer;
#endif/*__AMIGAOS__*/

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "NewMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	old_pointer = GlobalPointer;
#endif/*__AMIGAOS__*/

	/***********************/
	/* show a busy pointer */

#if __AMIGAOS__
	GlobalPointer = RGM_SetPointer (&YieldMPointer, MainWindow);
#elif __MSDOS__
	DCC_TempSetPointer (BPI_WAIT_POINTER);
#endif/*__AMIGAOS__/__MSDOS__*/

	PreTumeCleanUp ();
	PostTumeCleanUp ();

	TumeSetUpFirst ();
	TumeSetUpLast ();

	ShowRoom (GlobalRoomWindow);

	/*****************************/
	/* restore the mouse pointer */

#if __AMIGAOS__
	GlobalPointer = RGM_SetPointer (old_pointer, MainWindow);
#elif __MSDOS__
	DCC_SetPointer (GlobalPointer);
#endif/*__AMIGAOS__/__MSDOS__*/

	return (TRUE);

} /* NewMap */


#if __MSDOS__
static char *szFinalWord = NULL;

/*********************************************************************
 *
 * PrintFinalWord
 *
 * PURPOSE
 *		Print the final exit just before exiting the program.
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
 *		07/30/93 Friday (dcc) - created.
 *
*/
void PrintFinalWord(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PrintFinalWord";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (szFinalWord)
	{
		printf("%s\n", szFinalWord);
	}
} /* PrintFinalWord */
#endif/*__MSDOS__*/


/**************************************************************************
 *
 * Quit
 *
 * SYNOPSIS
 *		void Quit (char *message)
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
void Quit (char *message)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Quit";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	if (MainWindow) {
		GlobalPointer = RGM_SetPointer (NULL, MainWindow);
	}

	PreTumeCleanUp ();

	if (MainWindow) {
		/* ALWAYS RESTORE pr_WindowPtrX BEFORE CLOSING THE WINDOW!!! */
		OurTask->pr_WindowPtr = (APTR)old_pr_WindowPtr;
		ClearMenuStrip (MainWindow);
		CloseWindow (MainWindow);
		MainWindow = NULL;
	}
	UninitColorVB ();		/* GAT */
	UninitColorSeqReq ();	/* GAT */

	if (MainScreen)		CloseScreen (MainScreen);

	UninitKeys ();

	if (asfont)			CloseFont (asfont);
	if (TopazFont)		CloseFont (TopazFont);

	if (message) {
		SetColorsNPointer (MainWindow);
		Message (NULL, "Error Message", message, " Quit ");
		RestoreColorsNPointer (MainWindow);
	}
#elif __MSDOS__
	szFinalWord = message;
#endif/*__AMIGAOS__/__MSDOS__*/

	PostTumeCleanUp ();

	/*******************/
	/* close libraries */

#if __AMIGAOS__
	if (ArpBase) CloseLibrary ((struct Library *) ArpBase);
	if (IntuitionBase) CloseLibrary ((struct Library *) IntuitionBase);
	if (GfxBase) CloseLibrary ((struct Library *) GfxBase);

	exit (0);
#elif __MSDOS__
	MEM_ShowHeap();
	if	(message)
		exit (1);
	else
		exit (0);
#endif/*__AMIGAOS__/__MSDOS__*/

} /* Quit */


#if __AMIGAOS__
/**************************************************************************
 *
 * AutoReqOn
 *
 * SYNOPSIS
 *		void AutoReqOn (void)
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
 *		12/06/89 Wednesday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void AutoReqOn (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AutoReqOn";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	OurTask->pr_WindowPtr = (APTR) MainWindow;
	return;

} /* AutoReqOn */



/**************************************************************************
 *
 * AutoReqOff
 *
 * SYNOPSIS
 *		void AutoReqOff (void)
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
void AutoReqOff (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AutoReqOff";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	OurTask->pr_WindowPtr = (APTR) (-1);
	return;

} /* AutoReqOff */
#endif/*__AMIGAOS__*/


#if __MSDOS__
/*********************************************************************
 *
 * MyBeforeEvent
 *
 * SYNOPSIS
 *		static short MyBeforeEvent(HitInfo *hi)
 *
 * PURPOSE
 *		If user triggering event with keys instead of through
 *		menus, then call this routine to show the mouse if
 *		the mouse was hidden.
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
short MyBeforeEvent(HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MyBeforeEvent";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	hi = hi;		//turn off warnings

#if 0
	if (!TE_PointerFlag)
		ShowMouse();
#endif

	return 1;
} /* MyBeforeEvent */


/*********************************************************************
 *
 * MyAfterEvent
 *
 * SYNOPSIS
 *		static short MyAfterEvent(HitInfo *hi)
 *
 * PURPOSE
 *		If user triggering event with keys instead of through
 *		menus, then call this routine to hide the mouse if
 *		the mouse was hidden.
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
static short MyAfterEvent(HitInfo *hi)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MyAfterEvent";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	hi = hi;		//turn off warnings

#if 0
	if (!TE_PointerFlag)
		HideMouse();
#endif

	return 1;
} /* MyAfterEvent */
#endif/*__MSDOS__*/


/**************************************************************************
 *
 * main
 *
 * SYNOPSIS
 *		main (void)
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

#if __AMIGAOS__
main (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "main";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Initialize ();

	DoIt ();

	Quit (NULL);

} /* main */
#elif __MSDOS__

#if __MSDOS32X__
#else
static int ReturnTRUE(void)
{
	return TRUE;
}
#endif

int main (int argc, char **argv)
{
	char *szInitialMap = NULL;

#if NULLPTR
	InstallCheckPointer();
#endif
#if STACKCHECK
	StackFill();
#endif

	MEM_SetCheckHeapRate(100);

#if (__WATCOMC__ && __MSDOS32X__)
	if (!OpenMallocLow())
	{
		printf(OOM "\n");
		exit(1);
	}
#endif /*(__WATCOMC__ && __MSDOS32X__)*/

	if (argc > 1)
	{
		int i;

		for (i = 1; i < argc; i++)
		{
			if (*argv[i] == '-')
			{
				if (*(argv[i]+1) == 'r')
				{
					OpenEventRecorder(argv[i]+2, fSaveSomeEvents);
				}
				else if (*(argv[i]+1) == 'R')
				{
					OpenEventRecorder(argv[i]+2, fSaveAllEvents);
				}
				else if (*(argv[i]+1) == 'p')
				{
					OpenEventRecorder(argv[i]+2, fReadEvents + fReadFast);
				}
				else if (*(argv[i]+1) == 'P')
				{
					OpenEventRecorder(argv[i]+2, fReadEvents);
				}
				else if (*(argv[i]+1) == 't' || *(argv[i]+1) == 'T')
				{
					SetEarlyExitTime(atoi(argv[i]+2));
				}
				else if (*(argv[i]+1) == 'm' || *(argv[i]+1) == 'M')
				{
					SetXTRAMemoryPoolSize(atoi(argv[i]+2));
				}
			}
			else
			{
				szInitialMap = argv[i];
			}
		}
	}

	wSrcSkipX = 1;
	wDstDupX = 1;
	wSrcSkipY = 1;
	wDstDupY = 1;

//	InitXMS(0);					/* we won't require any buffers */
	SetLargestXTRA(wLargestXTRA);	/* we'll allocate XTRA block <= 32768U bytes */

//	InitList (PicList);
//	InitList (DGridList);
//	InitList (ObjList);
//	InitObjects ();
//	InitDisParts ();
//	InitRestore ();

#if UseSparseArray
	if (!InitSparse())
		return 0;
#endif // UseSparseArray
#if PixelLayers
	if (!InitDummySourceRoom())
		return 0;
#endif // PixelLayers

	Initialize ();

	if (ProcessINI(szIni))
	{
		if (OpenEUI (szIni))
		{
			AddExitFunc (CloseEUI);

#if DIV0TRAP
			InstallDivide0Trap();
#endif

			/* Intercept Ctrl-Breaks while tUME is running. */

#if __MSDOS32X__
//			ctrlbrk(ReturnTRUE);
#else
			ctrlbrk(ReturnTRUE);
#endif

			/* Initialize palette requester */

			if (!OpenColorReq(szIni))
				Quit(GlobalErrMsg);

			/* Initialize downloader if needed */

			if (fEnableDownload)
				if (!InitDownloader())
					Quit("Can't init downloader.\n");

			HideMouse();
			if (OpenMousePointers(FPF_FILENAME))
			{
				ShowMouse();
				GlobalPointer = DCC_SetPointer(BPI_NORMAL_POINTER);

				if (OpenColorseqInterrupt ())
	//			if (InitKeys ("tume.key"))
				{
	//				if (OpenDBufGraphics (MODE_MCGA, 200, 64000U))
					{
						SetWorkPage (DisplayBuffer);
						DisplayBM->width  = SCREEN_WIDTH;
						DisplayBM->height = SCREEN_HEIGHT;
						DisplayBM->data   = GetDisplayAddress();
						WorkBM->width     = SCREEN_WIDTH;
						WorkBM->height    = SCREEN_HEIGHT;
#if __MSDOS32X__
						WorkBM->data      = *WorkBuffer;
#else
						WorkBM->data      = MK_FP (*WorkBuffer, 0x0000);
#endif
						BackBM->width     = SCREEN_WIDTH;
						BackBM->height    = SCREEN_HEIGHT;
#if __MSDOS32X__
						BackBM->data      = *BackBuffer;
#else
						BackBM->data      = MK_FP (*BackBuffer, 0x0000);
#endif
						BigBackBM->width  = SCREEN_WIDTH;
						BigBackBM->height = SCREEN_HEIGHT;
						BigBackBM->mpYpMpXpPix = AllocateBigByteMap(SCREEN_WIDTH, SCREEN_HEIGHT);
	//					if (OpenMouse ())
						{
							EasyFR = InitFileReqs (EasyFRI);
							if (EasyFR) {
								AddExitFunc (RestoreOriginalDrive);
	//							ShowMouse ();

	//							BeforeGraphicsFunc = HideMouse;
	//							AfterGraphicsFunc  = ShowMouse;

	//							Initialize ();

	//							SetColors(MainWindow);
		// 						SetColorReg (MENU_BLACK,  0,  0,  0);
		// 						SetColorReg (MENU_WHITE, 63, 63, 63);
		//						SetColorReg (EUI_ORANGE, 63, 54, 42);
		//						SetColorReg (EUI_BLUE,    0,  0, 45);

	//							FontBPI = LoadBPI (FPF_FILENAME, FPI_SYSTEM_FONT);
	//							if (FontBPI)
								{
	//								MainFont.Letters         = &FontBPI->Shapes[BPI_SYSTEM_SYSBPI];
	//								MainMenuStrip.Symbols    = &FontBPI->Shapes[BPI_MENU_SYMBOLS];
	//								MainMenuStrip.FrontWidth = MainMenuStrip.Symbols[MNU_RADIO_OFF_SHAPE]->Width + 2;
	//								MainMenuStrip.BackWidth  = MainMenuStrip.Symbols[MNU_SUBMENU_SHAPE]->Width + 2;
	//								IconsBPI = LoadBPI (FPF_FILENAME, FPI_ICONS);
	//								if (IconsBPI)
									{
	//									Icons = IconsBPI->Shapes;

										BeforeGraphics ();

										SetPenColor (MainWindow->Black);
										DrawRect (0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	//									CopyrightStatus();

										ProcessInitialINIEvents(szIni);

	//									TriggerEvent ((void *)&ToggleTitleBarState, 0, NULL);
	//									TriggerEvent ((void *)&ToggleShowBrushState, 0, NULL);
		//								TriggerEvent ((void *)&ToggleToolPaletteState, 0, NULL);
		//								TriggerEvent ((void *)&ToggleCenterPointState, 0, NULL);

										SetStateVar(&SpaceToggleState, SpaceSTATE);

		//								ActivatePane (Pane3);
		//								TriggerEvent ((void *)&SetPaneToEditState, 0, NULL);

		//								ActivatePane (Pane2);
		//								TriggerEvent ((void *)&SetPaneToObjectsState, 0, NULL);

										ActivatePane (Pane1);
		/*dcc*/							SetPaneToEdit(NULL);
		/*dcc*/							SetupEditPane(Pane1);
		//								TriggerEvent ((void *)&SetPaneToPartsState, 0, NULL);

	#if NULLPTR
	{
		SetPenColor (MainWindow->White);
		DrawRect (0, TITLE_HEIGHT, SCREEN_WIDTH, 11);
		DrawString (MainWindow->Font, 1, 13, "Debugging version; may run slowly");
	}
	#endif

	#if fDemoBanner
	{
		SetPenColor (MainWindow->White);
		DrawRect (0, TITLE_HEIGHT, SCREEN_WIDTH, 41);
	//	SetPenColor (MainWindow->Black);
		DrawString (MainWindow->Font, 1, 13, "Evaluation version of tUME");
		DrawString (MainWindow->Font, 1, 23, "For evaluation use only.");
		DrawString (MainWindow->Font, 1, 33, "Proprietary Information of Echidna");
		DrawString (MainWindow->Font, 1, 43, "Contact Dan Chang: (619) 295-0936");
	}
	#endif

										AfterGraphics ();

										if (szInitialMap)
										{
											SetFreqFilename(EasyFR[PATH_MAP], szInitialMap);
											SetFreqDirname(EasyFR[PATH_MAP], szInitialMap);
											ClearNLoadMap(TRUE, szInitialMap);
										}

#if DEBUG
	{
		int i;

		for (i = 0; i < 16; i++)
		{
			char *sz;

			sz = MEM_malloc(2048);
			sz = sz;
		}
	}
#endif
										{
											MouseInfo	 mi;
											HitInfo		 hi;

											while (!ExitProgram) {

												ReadMouse (&mi);

		//										LeftButton = mi.Buttons & MOUSE_LEFTBUTTON;
		//										RightButton = mi.Buttons & MOUSE_RIGHTBUTTON;

												MouseX = mi.X;
												MouseY = mi.Y - TopAdd;

												/* If mouse moves into title bar, jam palette */

												if (lastMouseY >= lastMenuHeight && mi.Y < GlobalAreas[0].Height)
												{
													/* If menu bar was not showing, show it */

													AutoShowMenus();

													/* Set normal pointer while in menus */

													DCC_TempSetPointer(BPI_NORMAL_POINTER);
	//												if (!TE_PointerFlag)
	//													ShowMouse();
												}

												/* If mouse moves out of title bar, un-jam palette */

												if (lastMouseY < lastMenuHeight && mi.Y >= GlobalAreas[0].Height)
												{
													if (!JamPaletteSTATE)
														RestoreColorsNPointer(MainWindow);

													/* If menu bar should not be showing, hide it */

													if (!ShowTitleSTATE)
													{
														AutoHideMenus();
													}
													/* Restore pointer if not in menus */

													DCC_SetPointer(GlobalPointer);

	//												if (!TE_PointerFlag)
	//													HideMouse();
												}

												lastMouseY = mi.Y;
												lastMenuHeight = GlobalAreas[0].Height;

	//											if (mi.Buttons && mi.Y < TITLE_HEIGHT-1) {
	////												SetColors(MainWindow);
	//												LeftButton = FALSE;
	//												RightButton = FALSE;
	////												HandleMenus (MainWindow, &mi);
	//												CheckHitAreas (GlobalAreas, 0, 0);
	//											} else {
													FN_BeforeEvent	= MyBeforeEvent;
													FN_AfterEvent	= MyAfterEvent;
													HandleKeys (MainWindow);
													FN_BeforeEvent	= NULL;
													FN_AfterEvent	= NULL;

													CheckHitAreas (GlobalAreas, 0, 0);

													/* Only process button up mouse movements if below menu bar */

													if (FN_DontWait && mi.Y >= GlobalAreas[0].Height) {
														hi.X          = mi.X;
														hi.Y          = mi.Y;
														hi.Buttons    = mi.Buttons;
														hi.Shiftflags = ShiftFlags();

														LeftButton = hi.Buttons & MOUSE_LEFTBUTTON;
														RightButton = hi.Buttons & MOUSE_RIGHTBUTTON;

														MouseX = hi.X;
														MouseY = hi.Y - TopAdd;

														FN_DontWait (&hi);
													}
	//											}
											}
	//										CloseTimers();	//DEBUGEXIT
										}
								
	//									FreeBPI (IconsBPI);
									}
	//								FreeBPI (FontBPI);
								}
	//							HideMouse ();
							}
	//						CloseMouse ();
						}
	//					else
	//					{
	//						GlobalErr = TRUE;
	//						GlobalErrMsg = "tUME requires a mouse.";
	//					}
						FreeBigByteMap(BigBackBM->mpYpMpXpPix, BigBackBM->height);
	//					CloseDBufGraphics ();
					}
				CloseColorseqInterrupt ();
//				UninitKeys ();
				}
				CloseMousePointers();
			}
		}
	}

	CloseEventRecorder();

	if (GlobalErr)
	{
		Quit (GlobalErrMsg);
	}

	Quit (NULL);

	return 0;
}
#endif/*__AMIGAOS__/__MSDOS__*/

