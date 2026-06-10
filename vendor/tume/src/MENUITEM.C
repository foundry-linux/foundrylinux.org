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
 * MENUITEMS.C
 *
 * PROGRAMMER : R.G. Marquez
 *    VERSION : 00.000
 *    CREATED : 09/26/89
 *   MODIFIED : 03/26/95
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *	Menu selections for tUME.
 *
 * HISTORY
 *		09/26/89 (RGM) - Created.
 *		04/05/93 (dcc) - use rectplot.h header.
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
*/
#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include <string.h>

#include "rectplot.h"
#include "szerror.h"
#include "tuglbl.h"
#include "mouse.h"
#include "readtume.h"
#include "writetum.h"

#include <echidna/events.h>
#include <echidna/eio.h>

#include "events.h"
#include "tilebits.h"
#include "download.h"
#include "count.h"
#include "rmtsglue.h"
#include "printmap.h"
#include "colorreq.h"
#include "search.h"
#include "tumedraw.h"
#include "hilitile.h"
#include "layrtili.h"
#include "layrtile.h"
#include "histogrm.h"
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray

/**************************** C O N S T A N T S ***************************/

/* Note that the following two constants are the same as roomio.c.
	I assure you, this is merely coin-ink-e-dink. */

#define ADD_LAYER			(0x00)
#define INSERT_LAYER		(0x80)

#define MENUSAVE_WORK_MAP			((WORD) 1)
#define MENUSAVE_TILES_AS_TTI		((WORD) 2)
#define MENUSAVE_TMGX_MAP			((WORD) 4)
#define	MENUSAVE_TILES_AS_BRUSHES	((WORD) 8)
#define MENUSAVE_TMGC_MAP			((WORD)16)

#define QUERY_EXISTS	"Save over existing file?"

#define DIREXTENSION	".DIR"

#if __AMIGAOS__
#define DIRSEPERATOR	"/"
#elif __MSDOS__
#define DIRSEPERATOR	"\\"
#endif/*__AMIGAOS__/__MSDOS__*/

/******************************** T Y P E S *******************************/

#if __MSDOS__
extern WORD OldColumn;
extern WORD OldRow;
extern short lastMouseY;
#endif/*__MSDOS__*/

/****************************** E X T E R N S *****************************/


/****************************** G L O B A L S *****************************/

short		SpaceSTATE		= FALSE;
short		EditOnlyFloorSTATE	= FALSE;

#if __MSDOS__
short		ShowBrushSTATE		= FALSE;
short		XFlipBrushSTATE	= FALSE;
short		YFlipBrushSTATE	= FALSE;
#endif/*__MSDOS__*/
short		UseGridSTATE		= FALSE;
short		ShowGuideSTATE		= FALSE;
#if NEWTUME
short		ShowGuide2STATE	= FALSE;
#endif
short		ShowZoomSTATE		= FALSE;
short		KeepDownloadPaletteSTATE = FALSE;
short		SmartFlipSTATE		= FALSE;
short		StratifyPasteSTATE	= FALSE;
short		DownloadOneScreenSTATE = FALSE;
short		UseEditPaletteSTATE	= FALSE;
short		fShowTileUsage		= FALSE;
short		fPixelSelect		= FALSE;

short		SetStampPaintSTATE;
short		SetStampReplaceSTATE;

short		WLSSTATE;
short		WLESTATE;
short		WLCSTATE;

short		HidePointerSTATE	= FALSE;

BOOL		fEnableDownload	= FALSE;
BOOL		fDownloadHardwareAvail = FALSE;

/******************************* L O C A L S ******************************/

static char szAdd[]	= "add layer to";
static char szClr[]	= "clear";
static char szDel[]	= "delete";
static char szDelLay[]	= "delete layer";
static char szIns[]	= "insert layer to";
static char szLod[]	= "load layer to";

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

static void RoomSetup (RoomType *room)
{
	if (room) {
		WORD	flags;
		flags = room->Flags;
 
		if ( (!(flags & SOURCEROOM)) && (!(flags & LOCKEDROOM)) &&
			FSelectedPblk(GlobalBlockCopy) ) {
#if __AMIGAOS__
			switch (TU_DrawMode) {
			case DMODE_DRAW:
				GlobalPointer	= RGM_SetPointer (NULL, MainWindow);
				break;
			case DMODE_FILL:
				GlobalPointer	= RGM_SetPointer (NULL, MainWindow);
				break;
			case DMODE_RANDOM:
				GlobalPointer	= RGM_SetPointer (NULL, MainWindow);
				break;
			case DMODE_COLOR1:
				GlobalPointer	= RGM_SetPointer (&C1MPointer, MainWindow);
				break;
			case DMODE_COLOR2:
				GlobalPointer	= RGM_SetPointer (&C2MPointer, MainWindow);
				break;
			case DMODE_COLOR3:
				GlobalPointer	= RGM_SetPointer (&C3MPointer, MainWindow);
				break;
			case DMODE_COLOR4:
				GlobalPointer	= RGM_SetPointer (&C4MPointer, MainWindow);
				break;
			case DMODE_COLORREVERT:
				GlobalPointer	= RGM_SetPointer (NULL, MainWindow);
				break;
			}
#elif __MSDOS__
			if (TU_DrawMode >= DMODE_COLOR1)
				GlobalPointer	= DCC_SetPointer (BPI_COLORSET_POINTER);
			else
				GlobalPointer	= DCC_SetPointer (BPI_NORMAL_POINTER);
#endif/*__AMIGAOS__/__MSDOS__*/
		}
		else {
#if __AMIGAOS__
			GlobalPointer	= RGM_SetPointer (&GrabMPointer, MainWindow);
#elif __MSDOS__
			GlobalPointer	= DCC_SetPointer (BPI_CROSSHAIR_POINTER);
			lastMouseY = TITLE_HEIGHT+1;	/* Force call to DCC_SetPointer() in main if needed */
#endif/*__AMIGAOS__/__MSDOS__*/
		}
	}
	else {
#if __AMIGAOS__
		GlobalPointer	= RGM_SetPointer (NULL, MainWindow);
#elif __MSDOS__
		GlobalPointer	= DCC_SetPointer (BPI_NORMAL_POINTER);
#endif/*__AMIGAOS__/__MSDOS__*/
	}

	return;
}


/*********************************************************************
 *
 * SetPointerMode
 *
 * SYNOPSIS
 *		void SetPointerMode(void)
 *
 * PURPOSE
 *		Set the pointer based on the current drawing mode.
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
void SetPointerMode(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetPointerMode";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (	GlobalRoomWindow &&
		GlobalRoomWindow->CurrentRoom &&
		GlobalRoomWindow->CurrentRoom->Room)
		RoomSetup (GlobalRoomWindow->CurrentRoom->Room);

} /* SetPointerMode */


/*********************************************************************
 *
 * ClearNLoadMap
 *
 * PURPOSE
 *		Load map with filename <szMap>. Clear map in memory if <fClear>
 *		is TRUE.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns FALSE if OOM or unable to load.
 *
 * HISTORY
 *		08/04/93 Wednesday (dcc) - created.
 *
*/
int ClearNLoadMap(BOOL fClear, char *szMap)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ClearNLoadMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (szMap)
	{
		short fGoodLoad = S_ERROR;

		GlobalRoomWindow->CurrentRoom = &GlobalRoomWindow->SourceRoom;
		GlobalRoomWindow->OtherRoom = &GlobalRoomWindow->EditRoom;
		UnHookRoomWindow (GlobalRoomWindow->SourceRoom.Room,
						GlobalRoomWindow);
		GlobalRoomWindow->CurrentRoom = &GlobalRoomWindow->EditRoom;
		GlobalRoomWindow->OtherRoom = &GlobalRoomWindow->SourceRoom;
		UnHookRoomWindow (GlobalRoomWindow->EditRoom.Room,
						GlobalRoomWindow);
		GlobalRoomWindow->CurrentRoom = &GlobalRoomWindow->SourceRoom;
		GlobalRoomWindow->OtherRoom = &GlobalRoomWindow->EditRoom;

		if (fClear)
		{
			NewMap ();
		}
		if ((fGoodLoad = LoadMap (szMap)) == S_OK)
		{
			RoomType	*room;

			/*******************************/
			/* Try to load the RoomWindows */

			room = FirstSourceRoom (GlobalMap);
			if (room)
			{
				GlobalRoomWindow->CurrentRoom =
					&GlobalRoomWindow->SourceRoom;
				GlobalRoomWindow->OtherRoom =
					&GlobalRoomWindow->EditRoom;
				TryToHookRoomWindow (room, GlobalRoomWindow);
			}
			room = FirstEditRoom (GlobalMap);
			if (room)
			{
				GlobalRoomWindow->CurrentRoom =
					&GlobalRoomWindow->EditRoom;
				GlobalRoomWindow->OtherRoom =
					&GlobalRoomWindow->SourceRoom;
				TryToHookRoomWindow (room, GlobalRoomWindow);
			}

			CurrentX = CurrentY = 0;
			if (LastTBar == SHOW_TITLE)
			{
				ShowState (SHOW_DEFAULT);
			}
			else
			{
				ShowState (LastTBar);
			}
			ShowRoom (GlobalRoomWindow);
			SetPointerMode();
		}
		else
		{
			if (fGoodLoad == S_LOW_MEM)
				TellUser (OOM, "Not enough memory to load map.");
			else
			{
				char sz[256];

				sprintf(sz, "Can't seem to load map '%s'.", szMap);
				TellUser (NCD, sz);
			}
			return FALSE;
		}
	}
	return TRUE;
} /* ClearNLoadMap */


/**************************************************************************
 *
 * LMapx
 *
 * SYNOPSIS
 *		static int LMapx (BOOL clear_flag)
 *
 * PURPOSE
 *
 *
 * USAGE
 *	LMapx (TRUE);
 *		or
 *	LMapx (FALSE);
 *
 * INPUT
 *		clear_flag	: TRUE: clear the current map
 *					: FALSE: DON'T clear the current map (append rooms)
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *	11/14/89 Tuesday - Created out of old LMap. (RGM)
 *
 * SEE ALSO
 *
*/

static int LMapx (BOOL clear_flag)
{
	void		*fname;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LMapx";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Colorz = TRUE;			/* lock out RestoreColors */
	SetColorsNPointer (MainWindow);

	if ((clear_flag) && (! IsEmpty (&GlobalMap->Rooms))) {
		if (! AreYouSure ("This operation will forget the current map.  Continue?")) {
/**/			goto ABORT;
		}
	}

	if (clear_flag) {
//		SetFreqTitle (EasyFR[PATH_MAP], "Load Map");
		fname = GetFileName (MainWindow, EasyFR[PATH_MAP]);
	}
	else {
//		SetFreqTitle (EasyFR[PATH_ROOM], "Append Map/Room");
		fname = GetFileName (MainWindow, EasyFR[PATH_ROOM]);
	}

	ClearNLoadMap(clear_flag, fname);

ABORT:
	Colorz = FALSE;			/* enable RestoreColors */
	RestoreColorsNPointer (MainWindow);

	return (TRUE);
} /* LMapx */


#if fDoSaveRooms
/**************************************************************************
 *
 * SMap
 *
 * SYNOPSIS
 *		int SMap (WORD save_type)
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
 *	10/19/89 Thursday - Created. (RGM)
 *
 *	11/05/89 Sunday   - Generalized for NORMAL/RUNTUME. (RGM)
 *
 * SEE ALSO
 *
*/

static int SMap (WORD save_type)
{
	void		*fname;
	char		*fullpath;
	char		*savefilespec;

	char		*dir;
	char		*file;
	char		*ext;

#if __AMIGAOS__
	void		*old_pointer;
#endif/*__AMIGAOS__*/

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	old_pointer = GlobalPointer;
#endif/*__AMIGAOS__*/

	fullpath		= MEM_calloc (EIO_MAXPATH, 1);
	savefilespec	= MEM_calloc (EIO_MAXPATH, 1);

	dir			= MEM_calloc (EIO_MAXDIR, 1);
	file			= MEM_calloc (EIO_MAXFILE, 1);
	ext			= MEM_calloc (EIO_MAXEXT, 1);

	if (!(fullpath && savefilespec && dir && file && ext)) {
/**/	goto ABORT;
	}

	Colorz = TRUE;			/* lock out RestoreColors */
	SetColorsNPointer (MainWindow);

	GlobalMap->SaveFlags		= 0;
	GlobalTileSpace->SaveFlags	= 0;

	if (save_type & MENUSAVE_TILES_AS_TTI) {
		SetFreqTitle (EasyFR[PATH_MAP], "Xave Map");
		fname = GetFileName (MainWindow, EasyFR[PATH_MAP]);
	}
	else if (save_type & MENUSAVE_TMGC_MAP) {
		SetFreqTitle (EasyFR[PATH_MAP], "Save+TMGC");
		fname = GetFileName (MainWindow, EasyFR[PATH_MAP]);
	}
	else if (save_type & MENUSAVE_TMGX_MAP) {
		SetFreqTitle (EasyFR[PATH_MAP], "Save+TMGX");
		fname = GetFileName (MainWindow, EasyFR[PATH_MAP]);
	}
	else if (save_type & MENUSAVE_WORK_MAP) {
		SetFreqTitle (EasyFR[PATH_MAP], "Save Map");
		fname = GetFileName (MainWindow, EasyFR[PATH_MAP]);
	}
	else if (save_type & MENUSAVE_TILES_AS_BRUSHES) {
		fname = "";
	}
	else {
		fname = GetFileName (MainWindow, EasyFR[PATH_MAP]);
		/* fname = GetPathName (MainWindow, EasyFR[PATH_MAP]); */
	}

	if (fname) {

		strcpy (fullpath, fname);

				/* eg fullpath		= "DH0:SUB.MAP" */

		EIO_fnsplit (fullpath, dir, file, ext);

		strcpy (savefilespec, dir);		/* "DH0:" */
		strcat (savefilespec, file);	/* + "SUB" */

				/* eg savefilespec	= "DH0:SUB"     */

		if (save_type & MENUSAVE_TILES_AS_TTI) {
			strcat (savefilespec, DIREXTENSION);	/* + ".DIR " */	
			strcat (savefilespec, DIRSEPERATOR);	/* + "/"     */
			strcat (savefilespec, file);			/* + "SUB"   */

				/* ( eg savefilespec= "DH0:SUB.DIR/SUB" ) */
		}

		strcat (savefilespec, ext);

				/* eg savefilespec	= "DH0:SUB[.DIR/SUB].MAP" */

		if (! (save_type & MENUSAVE_TILES_AS_BRUSHES)) {
			if ( (EIO_FileExists (savefilespec)) && (!(AreYouSure(QUERY_EXISTS))) ) {
/**/			goto ABORT;
			}
		}

#if __AMIGAOS__
		GlobalPointer = RGM_SetPointer (&YieldMPointer, MainWindow);
#elif __MSDOS__
		DCC_TempSetPointer (BPI_WAIT_POINTER);
#endif/*__AMIGAOS__/__MSDOS__*/

		if (save_type & MENUSAVE_TILES_AS_TTI) {
			ShowStatus ("Xaving tUME tile files...");

			/* eg savefilespec	= "DH0:SUB.DIR/SUB.MAP" */

			if (! SaveTileSpace (GlobalTileSpace, savefilespec, SAVETILES_AS_TTI)) {
				TellUser (NoCanDo, "Can't seem to save tile images.");
/**/				goto ABORT;
			}
		}
		if (save_type & MENUSAVE_TILES_AS_BRUSHES) {
			ShowStatus ("Saving tUME tile brush files...");
			if (! SaveTileSpace (GlobalTileSpace, savefilespec, SAVETILES_AS_NAMED)) {
				TellUser (NoCanDo, "Can't seem to save tile images.");
/**/				goto ABORT;
			}
		}


		/****************************************************/
		/* This is where we actually save the tUME IFF file */

		if (save_type & (MENUSAVE_WORK_MAP | MENUSAVE_TMGX_MAP | MENUSAVE_TMGC_MAP)) {

			UWORD	save_options	= 0;

			if (save_type & MENUSAVE_TMGX_MAP) {
				ShowStatus ("Saving tUME IFF + TMGX file...");
			}
			else if (save_type & MENUSAVE_TMGC_MAP) {
				ShowStatus ("Saving tUME IFF + TMGC file...");
			}
			else if (save_type & MENUSAVE_WORK_MAP) {
				ShowStatus ("Saving tUME IFF file...");
			}
			else {
				ShowStatus ("Saving data...");
			}

			if (save_type & MENUSAVE_TMGX_MAP) {
				save_options	= TMGX_MAP;
			}
			if (save_type & MENUSAVE_TMGC_MAP) {
				save_options	= TMGC_MAP;
			}

			if (! SaveMap (savefilespec, save_options)) {
				TellUser (NoCanDo, "Can't seem to save map.");
			}
		}

	}

	SAFEFREE (fullpath);
	SAFEFREE (savefilespec);

	SAFEFREE (dir);
	SAFEFREE (file);
	SAFEFREE (ext);

#if __AMIGAOS__
	GlobalPointer	= RGM_SetPointer (old_pointer, MainWindow);
#elif __MSDOS__
	DCC_SetPointer (GlobalPointer);
#endif/*__AMIGAOS__/__MSDOS__*/
	ShowState (LastTBar);
	Colorz			= FALSE;			/* enable RestoreColors */
	RestoreColorsNPointer (MainWindow);

	return (TRUE);
/*--------------------------------------------------------------------*/
ABORT:
	SAFEFREE (fullpath);
	SAFEFREE (savefilespec);

	SAFEFREE (dir);
	SAFEFREE (file);
	SAFEFREE (ext);

#if __AMIGAOS__
	GlobalPointer	= RGM_SetPointer (old_pointer, MainWindow);
#elif __MSDOS__
	DCC_SetPointer (GlobalPointer);
#endif/*__AMIGAOS__/__MSDOS__*/
	ShowState (LastTBar);
	Colorz			= FALSE;			/* enable RestoreColors */
	RestoreColorsNPointer (MainWindow);

	return (TRUE);
} /* SMap */
#else
/*********************************************************************
 *
 * SMap
 *
 * SYNOPSIS
 *		static int SMap (short w)
 *
 * PURPOSE
 *		Tell user no save in this version.
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
static int SMap (short w)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	w = w;

	TellUser (NoCanDo, "Saving is disabled in this demo version.");
	return TRUE;

} /* SMap */
#endif/*fDoSaveRooms*/


/**************************************************************************
 *
 * MenuLoadTileSet
 *
 * SYNOPSIS
 *		int MenuLoadTileSet (UWORD	loadflags)
 *
 * PURPOSE
 *		Get a tileset name from the user and attempt to load it.
 *
 * USAGE
 *		MenuLoadTileSet (LOAD_FLAGS);
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
static	int	MenuLoadTileSet (UWORD loadflags)
{
	void *fname;
	void *newtileset;
	void *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MenuLoadTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Colorz = TRUE;			/* lock out RestoreColors */
	SetColorsNPointer (MainWindow);

	if ((fname = GetFileName (MainWindow, EasyFR[PATH_TILES])) != NULL) {

		if ( (newtileset = LoadTileSet (fname, loadflags)) == NULL) {
/**/		goto ABORT;
		}
		else {
			GlobalTileSet = newtileset;

			if (GlobalTileSpace) {
				AddTileSet (newtileset, GlobalTileSpace);
			}

			/*******************************/
			/* now put the tiles in a room */

										/* tset,requested room, map */
			room = (void *) PlaceTiles (newtileset, NULL, GlobalMap);

			if (! room) {
				TellUser (OOM, "Can't seem to put this tileset in a room.");
/**/				goto ABORT;
			}
			FriendlyShowRoom (room, GlobalRoomWindow);
			MButtonsE ();
		}
	}

	Colorz = FALSE;			/* enable RestoreColors */
	RestoreColorsNPointer (MainWindow);

	return (TRUE);
/*--------------------------------------------------------------------*/
ABORT:
	if (newtileset) {
		RemoveTileSet (newtileset);
		UnLoadTileSet (newtileset);
		newtileset = NULL;
	}
	Colorz = FALSE;			/* enable RestoreColors */
	RestoreColorsNPointer (MainWindow);

	return (TRUE);

} /* MenuLoadTileSet */


#if fDoSaveRooms
/**************************************************************************
 *
 * SaveRoom
 *
 * SYNOPSIS
 *		static int SaveRoom (BOOL limitflag, BOOL fAllLayers)
 *
 * PURPOSE
 *		To provide a 'save room' intersection for the menu items.
 *
 * INPUT
 *		limitflag	: FALSE	= save room + ALL tile infos.
 *				: TRUE	= save room + only tile infos used in this room.
 *
 *		fAllLayers: TRUE	= save all layers of rooms
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *	11/13/89 Monday - Created. (RGM)
 *	05/11/93 Tuesday (dcc) - fAllLayers added.
 *
 * SEE ALSO
 *
*/
static	int SaveRoom (BOOL limitflag, BOOL fAllLayers)
{
	RoomType		*room;
	char			*fname;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;
	Colorz = TRUE;			/* lock out RestoreColors */
	SetColorsNPointer (MainWindow);

	SetFreqTitle (EasyFR[PATH_ROOM], fAllLayers ? "Save Room" : "Save Layer");

	if (room) {
		if ((fname = GetFileName (MainWindow, EasyFR[PATH_ROOM])) != NULL) {
			if ( (EIO_FileExists (fname)) && (!(AreYouSure(QUERY_EXISTS))) ) {
/**/			goto ABORT;
			}

			GlobalMap->SaveFlags	|= TLIMITSAVEFLAG;
			room->SaveFlags		|= TLIMITSAVEFLAG;

			if (limitflag) {
				GlobalTileSpace->SaveFlags |= TLIMITSAVEFLAG;
				MarkUsedTileSets (room, fAllLayers, GlobalTileSpace, TLIMITSAVEFLAG);
			}

			ShowStatus ("Saving tUME IFF file...");
			/** go for the save **/
			if (! SaveMap (fname, (fAllLayers ? 0 : ONLY_FLOOR))) {
				TellUser (NoCanDo, "Trouble saving room.");
			}
			GlobalMap->SaveFlags	&= INVERSE_W(TLIMITSAVEFLAG);
			room->SaveFlags		&= INVERSE_W(TLIMITSAVEFLAG);

			if (limitflag) {
				UnmarkTileSets (GlobalTileSpace, TLIMITSAVEFLAG);
				GlobalTileSpace->SaveFlags &= INVERSE_W(TLIMITSAVEFLAG);
			}

			ShowState (LastTBar);
		}
	}
	else {
		TellUser (NoCanDo, "Can't save a null room.");
	}
ABORT:
	Colorz = FALSE;			/* enable RestoreColors */
	RestoreColorsNPointer (MainWindow);
	return (TRUE);

} /* SaveRoom */
#else
/*********************************************************************
 *
 * SaveRoom
 *
 * SYNOPSIS
 *		static int SaveRoom(BOOL f, BOOL f2)
 *
 * PURPOSE
 *		Tell user no save in this version.
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
#define SaveRoom(f,f2) SMap(f)
#endif/*fDoSaveRooms*/


#if 0
/**************************************************************************
 *
 * TileComment_n
 *
 * SYNOPSIS
 *		int TileComment_n (UBYTE n)
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
 *		03/12/90 Monday - Created from RenameRoom. (RGM)
 *
 *
 * SEE ALSO
 *
*/
static	int TileComment_n (UBYTE n)
{
	TileSetType	*tileset = NULL;
	RoomType		*room;
	char			*oldname	= NULL;
	char			*newname;
	char			*nullname	= NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TileComment_n";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;

	if (FSelectedPblk(pblk)) {
		tileset	= GlobalBlockCopy->FirstTileSet;
	}
	else if (IsSource(room) || IsComposite(room)) {
		tileset	= FindFirstTileSet (room);
	}
	if (tileset) {
		SetColorsNPointer (MainWindow);

		if (tileset->Comment[n]) {
			oldname = tileset->Comment[n];
		}
		else {
			oldname = nullname;
		}

		newname = GetName (GlobalRoomWindow->Window, oldname);

		if ((newname) && (*newname)) {
			oldname = MEM_calloc (strlen (newname) + 1, 1);
			if (oldname) {
				strcpy (oldname, newname);
			}
			MEM_free (tileset->Comment[n]);
			tileset->Comment[n] = oldname;
		}

		RestoreColorsNPointer (MainWindow);
	}
	else {
		ShowStatus ("I can't find a tile for info.");
	}
	return (TRUE);

} /* TileComment_n */
#endif


#if __AMIGAOS__
/**************************************************************************
 *
 * MakeColorX
 *
 * SYNOPSIS
 *		static	int MakeColorX (short color_set)
 *
 * PURPOSE
 *		Change into NINTENDO color group select draw mode.
 *
 * USAGE
 *
 *
 * INPUT
 *		A color set number (1 through 4).
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *	12/30/90 Sunday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
static	int MakeColorX (short color_set)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MakeColorX";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	TU_DrawMode	= DMODE_COLOR1 + (color_set - 1);
	SetPointerMode();

	return (TRUE);

} /* MakeColorX */
#endif/*__AMIGAOS__*/


/*********************************************************************
 *
 * FCompositeRoom
 *
 * SYNOPSIS
 *		static BOOL FCompositeRoom(RoomType *prm, char *szOp)
 *
 * PURPOSE
 *		If room is a composite room, tell user
 *		"Can't <szOp> a composite room.", then return TRUE.
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
static BOOL FCompositeRoom(RoomType *prm, char *szOp)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FCompositeRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsComposite(prm))
	{
		char sz[256];

		sprintf(sz, "Can't %s a composite room.", szOp);
		TellUser(NoCanDo, sz);

		return TRUE;
	}
	return FALSE;

} /* FCompositeRoom */


/*********************************************************************
 *
 * FLockedRoom
 *
 * SYNOPSIS
 *		static BOOL FLockedRoom(RoomType *prm, char *szOp)
 *
 * PURPOSE
 *		If room is locked, tell user
 *		"Can't <szOp> a locked room.", then return TRUE.
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
static BOOL FLockedRoom(RoomType *prm, char *szOp)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FLockedRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsLocked(prm))
	{
		char sz[256];

		sprintf(sz, "Can't %s a locked room.", szOp);
		TellUser(NoCanDo, sz);

		return TRUE;
	}
	return FALSE;

} /* FLockedRoom */


/*********************************************************************
 *
 * FSourceRoom
 *
 * SYNOPSIS
 *		static BOOL FSourceRoom(RoomType *prm, char *szOp)
 *
 * PURPOSE
 *		If room is a source room, tell user
 *		"Can't <szOp> a source room.", then return TRUE.
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
static BOOL FSourceRoom(RoomType *prm, char *szOp)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FSourceRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsSource(prm))
	{
		char sz[256];

		sprintf(sz, "Can't %s a tileset room.", szOp);
		TellUser(NoCanDo, sz);

		return TRUE;
	}
	return FALSE;

} /* FSourceRoom */


/*********************** M E N U   R O U T I N E S ************************/


/**************************************************************************
 *
 * NMap
 *
 * SYNOPSIS
 *		int NMap (void)
 *
 * PURPOSE
 *	Menu item to clear current map.
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

int NMap (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "NMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (! IsEmpty (&GlobalMap->Rooms)) {
		if (AreYouSure ("Do you want to clear this map? (Can't Undo!)")) {
			ShowState (SHOW_TITLE);
			NewMap ();
		}
	}
	else {
		ShowState (SHOW_TITLE);
	}
	return (TRUE);
} /* NMap */


/**************************************************************************
 *
 * LMap
 *
 * SYNOPSIS
 *		int LMap (void)
 *
 * PURPOSE
 *	Menu item to clear current map and load a new one.
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
 *	10/22/89 Sunday - Created from SMap. (RGM)
 *
 * SEE ALSO
 *
*/
int LMap (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetFreqTitle (EasyFR[PATH_MAP], "Load Map");
	LMapx (TRUE);		/* DO CLEAR the current map */
	return (TRUE);
} /* LMap */


/**************************************************************************
 *
 * AMap
 *
 * SYNOPSIS
 *		int AMap (void)
 *
 * PURPOSE
 *	Menu item to append to current map.
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
int AMap (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetFreqTitle (EasyFR[PATH_ROOM], "Append Map");
	LMapx (FALSE);		/* DON'T CLEAR the current map */
	return (TRUE);
} /* AMap */


/*********************************************************************
 *
 * AppendorLoadLayer
 *
 * SYNOPSIS
 *		static int AppendorLoadLayer(short f)
 *
 * PURPOSE
 *		Load layers in map to end of current room (Append), or
 *		load layers in map to current floor layer of current room.
 *		Bump up previouse floor layer and layers above it.
 *
 * INPUT
 *		f: either APPEND_LAYER or LOAD_LAYER.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		05/11/93 Tuesday (dcc) - created.
 *
 * SEE ALSO
 *
*/
static int AppendorLoadLayer(short f)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AppendorLoadLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetFreqTitle (EasyFR[PATH_ROOM], "Append Map");
	room = GlobalRoomWindow->CurrentRoom->Room;
	if (room)
	{
		if (FLockedRoom(room, szLod))
			return TRUE;
		else if (FSourceRoom(room, szLod))
			return TRUE;
		else if (FCompositeRoom(room, szLod))
			return TRUE;
		else
		{
			if (f & LOAD_LAYER)
			{
				SetLoadLayer(room->FloorLayer, f);
				SetFreqTitle (EasyFR[PATH_ROOM], "Load Layer");
			}
			else
			{
				SetLoadLayer(Tail(&room->Layers), f);
				SetFreqTitle (EasyFR[PATH_ROOM], "Append Layer");
			}
		}
	}
	LMapx (FALSE);		/* DON'T CLEAR the current map */
	return (TRUE);

} /* AppendorLoadLayer */


/*********************************************************************
 *
 * AppendLayer
 *
 * SYNOPSIS
 *		int AppendLayer(void)
 *
 * PURPOSE
 *		Append layers in map to end of current room.
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
 *		05/11/93 Tuesday (dcc) - created.
 *
 * SEE ALSO
 *
*/
int AppendLayer(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AppendLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return AppendorLoadLayer(APPEND_LAYER);

} /* AppendLayer */


/*********************************************************************
 *
 * LoadLayer
 *
 * SYNOPSIS
 *		int LoadLayer(void)
 *
 * PURPOSE
 *		Load layers in map to current floor layer of current room.
 *		Bump up previouse floor layer and layers above it.
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
 *		05/11/93 Tuesday (dcc) - created.
 *
 * SEE ALSO
 *
*/
int LoadLayer(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LoadLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return AppendorLoadLayer(LOAD_LAYER);

} /* LoadLayer */


/**************************************************************************
 *
 * SMap1
 *
 * SYNOPSIS
 *		int SMap1 (void)
 *
 * PURPOSE
 *	Menu item to save current map.
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

int SMap1 (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SMap1";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (SMap (MENUSAVE_WORK_MAP));

} /* SMap1 */


/**************************************************************************
 *
 * SMap2
 *
 * SYNOPSIS
 *		int SMap2 (void)
 *
 * PURPOSE
 *	Menu item to save current map + tiles.
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

int SMap2 (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SMap2";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (SMap (MENUSAVE_WORK_MAP | MENUSAVE_TILES_AS_TTI));

} /* SMap2 */


int SMap3 (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SMap3";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (SMap (MENUSAVE_WORK_MAP | MENUSAVE_TMGX_MAP));

} /* SMap3 */


int SMap4 (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SMap4";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (SMap (MENUSAVE_WORK_MAP | MENUSAVE_TMGC_MAP));

} /* SMap4 */


/**************************************************************************
 *
 * TE_SaveTilesAsBrushes
 *
 * SYNOPSIS
 *		int TE_SaveTilesAsBrushes (void)
 *
 * PURPOSE
 *		Menu item to save tiles as named individual brushes.
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
 *	03/09/91 Saturday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
int TE_SaveTilesAsBrushes (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TE_SaveTilesAsBrushes";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (SMap (MENUSAVE_TILES_AS_BRUSHES));

} /* TE_SaveTilesAsBrushes */


/**************************************************************************
 *
 * tumeInfo
 *
 * SYNOPSIS
 *		int tumeInfo (void)
 *
 * PURPOSE
 *	Menu item to print map information, etc?
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

int tumeInfo (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "tumeInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	NotYet ();
	return (TRUE);

} /* tumeInfo */


/**************************************************************************
 *
 * EditRoom
 *
 * SYNOPSIS
 *		int EditRoom (void)
 *
 * PURPOSE
 *		menu item to put tUME in 'edit room' mode.
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

int EditRoom (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "EditRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	FN_LeftUp		= ButtonsE;
	FN_LeftDown		= ButtonsE;
	FN_RightUp		= ButtonsE;
	FN_RightDown	= ButtonsE;

	/* FN_RightDown = M_WFRoom; */
#endif/*__AMIGAOS__*/

	TU_DrawMode	= DMODE_DRAW;

	SetPointerMode();

	return (TRUE);

} /* EditRoom */


/**************************************************************************
 *
 * DrawRandom
 *
 * SYNOPSIS
 *		int DrawRandom (void)
 *
 * PURPOSE
 *	Menu item to put tUME in random draw mode.
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
int DrawRandom (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawRandom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	NotYet ();
	return (TRUE);

} /* DrawRandom */


/**************************************************************************
 *
 * TE_LoadTilesAll
 *
 * SYNOPSIS
 *		int TE_LoadTilesAll (void)
 *
 * PURPOSE
 *		To load all tiles (including blank ones.)
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
 *		02/04/92 Tuesday (dcc) - created.
 *
 * SEE ALSO
 *
*/
int TE_LoadTilesAll (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TE_LoadTilesAll";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (
		MenuLoadTileSet ((UWORD)
			(	ASFOUND |
				GRID_IS_NOT_BOXED |
				GRID_START_FROM_TOP |
				GRID_TILEDIMENSIONS_FROM_USER |
				GRID_OFFSET_FROM_TILEDIMENSIONS
			)
		));

} /* TE_LoadTilesAll */


/**************************************************************************
 *
 * TE_LoadTilesFullTiled
 *
 * SYNOPSIS
 *		int TE_LoadTilesFullTiled (void)
 *
 * PURPOSE
 *		To load the Virgin Mastertronic / Arcadia way.
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
 *	03/06/91 Wednesday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
int TE_LoadTilesFullTiled (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TE_LoadTilesFullTiled";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (
		MenuLoadTileSet ((UWORD)
			(	ASFOUND |
				TWO_BLANK_TILE_ENDING |
				GRID_IS_NOT_BOXED |
				GRID_START_FROM_TOP |
				GRID_TILEDIMENSIONS_FROM_USER |
				GRID_OFFSET_FROM_TILEDIMENSIONS
			)
		));

} /* TE_LoadTilesFullTiled */


/**************************************************************************
 *
 * TE_LoadTilesFullTiledNoBlank
 *
 * SYNOPSIS
 *		int TE_LoadTilesFullTiledNoBlank (void)
 *
 * PURPOSE
 *		To load tiles the new LIVE studios way.
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
 *	03/06/91 Wednesday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
int TE_LoadTilesFullTiledNoBlank (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TE_LoadTilesFullTiledNoBlank";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (
		MenuLoadTileSet ((UWORD)
			(	ASFOUND |
				TWO_BLANK_TILE_ENDING |
				THROW_AWAY_BLANK_TILES |
				GRID_IS_NOT_BOXED |
				GRID_START_FROM_TOP |
				GRID_TILEDIMENSIONS_FROM_USER |
				GRID_OFFSET_FROM_TILEDIMENSIONS
			)
		));

} /* TE_LoadTilesFullTiledNoBlank */



/**************************************************************************
 *
 * TE_LoadTilesCookieCutter
 *
 * SYNOPSIS
 *		int TE_LoadTilesCookieCutter (void)
 *
 * PURPOSE
 *		To load tiles the JMA 'cookie cutter' way.
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
 *	03/06/91 Wednesday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
int TE_LoadTilesCookieCutter (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TE_LoadTilesCookieCutter";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (
		MenuLoadTileSet ((UWORD)
			(	ASFOUND |
				TWO_BLANK_TILE_ENDING |
				GRID_IS_NOT_BOXED |
				GRID_START_FROM_FIRST_NOTRANSPARENT |
				GRID_TILEDIMENSIONS_FROM_FIRST_NOTRANSPARENT |
				GRID_OFFSET_FROM_FIRST_NOTRANSPARENT
			)
		));

} /* TE_LoadTilesCookieCutter */


/**************************************************************************
 *
 * TE_LoadTilesBoxed
 *
 * SYNOPSIS
 *		int TE_LoadTilesBoxed (void)
 *
 * PURPOSE
 *		To load tiles the old LIVE Studios (Future Classics) way.
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
 *	03/06/91 Wednesday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
int TE_LoadTilesBoxed (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TE_LoadTilesBoxed";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (
		MenuLoadTileSet ((UWORD)
			(	ASFOUND |
				GRID_IS_BOXED
			)
		));

} /* TE_LoadTilesBoxed */


/**************************************************************************
 *
 * CRoom
 *
 * PURPOSE
 *		Menu item to create a new room.
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
 *		08/21/94 (dcc) - add support for UseSparseArray
 *		10/20/94 (dcc) - Start with an initial non-sparse layer.
 *
 * SEE ALSO
 *
*/
int CRoom (void)
{
	short ctilx = 0;
	short ctily = 0;
	RoomType *room		= NULL;
	RoomType *prtTemp	= NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Colorz = TRUE;			/* lock out RestoreColors */
	SetColorsNPointer (MainWindow);

#if 1
	if ((prtTemp = MEM_calloc(1, ROOMSIZE)) == NULL)
/**/	goto ABORT;

	if (GetRoomInfoReq (prtTemp, &ctilx, &ctily, TRUE)) {
#else
	if (RS_Request (&ctilx, &ctily)) {
#endif
#if UseSparseArray
		// USESPARSEARRAY FIXME BUGBUG
		room = CreateRoomWithUndo ((WORD) ctilx, (WORD) ctily, FREEROOM, FALSE);
#else // !UseSparseArray
		room = CreateRoomWithUndo ((WORD) ctilx, (WORD) ctily, FREEROOM);
#endif // !UseSparseArray

		if (room) {
#if 1
			room->UserType		= prtTemp->UserType;
			room->UserNumber	= prtTemp->UserNumber;

			if (prtTemp->Name)
				room->Name = prtTemp->Name;
			if (prtTemp->Comment[0])
				room->Comment[0] = prtTemp->Comment[0];
			if (prtTemp->Comment[1])
				room->Comment[1] = prtTemp->Comment[1];
#endif
			AddTail (&(GlobalMap->Rooms), room);
		} else {
			SAFEFREE(prtTemp->Name);
			SAFEFREE(prtTemp->Comment[0]);
			SAFEFREE(prtTemp->Comment[1]);
/**/		goto ABORT;
		}
	}

	FriendlyShowRoom (room, GlobalRoomWindow);
	SetPointerMode();

	MButtonsE ();

	Colorz = FALSE;			/* enable RestoreColors */
	RestoreColorsNPointer (MainWindow);

	MEM_free(prtTemp);
	return (TRUE);
/*-----------------------------------------------------------------------*/
ABORT:
	Colorz = FALSE;			/* enable RestoreColors */
	RestoreColorsNPointer (MainWindow);

	MEM_free(prtTemp);
	return (TRUE);

} /* CRoom */


/**************************************************************************
 *
 * DRoom
 *
 * SYNOPSIS
 *		int DRoom (void)
 *
 * PURPOSE
 *	Menu item to delete a room.
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
 *		11/13/89 Monday   - Created. (RGM)
 *		12/07/89 Thursday - For reals created. (RGM)
 *		01/18/93 Monday (dcc) - erase any brush pointing to this room as well.
 *
 * SEE ALSO
 *
*/

int DRoom (void)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;
	if (room)
	{
		if (FLockedRoom(room, szDel))
			;
		else if (FSourceRoom(room, szDel))
			;
		else if (FCompositeRoom(room, szDel))
			;
		else
		{
			char sz[256];

			strcpy(sz, "Do you want to delete this room");

			if (GlobalBlockCopy->SourceRoom == room)
			{
				strcat(sz, ", and the current brush");
			}
			strcat(sz, "? (Can't Undo!)");

			if (AreYouSure (sz))
			{
				if (GlobalBlockCopy->SourceRoom == room)
				{
					Unselect();
				}
				ClearSearchBufferIfFromRoom(room);

				WNRoom ();
				if (room == GlobalRoomWindow->CurrentRoom->Room) {
					TotalRoomUnhook (room, GlobalRoomWindow);
					ShowState (LastTBar);
					ShowRoom (GlobalRoomWindow);
					MButtonsE ();
				}
				Remove (room);
				DeAllocateRoom (room);
			}
		}
	}
	return (TRUE);

} /* DRoom */


/**************************************************************************
 *
 * LRoom
 *
 * SYNOPSIS
 *		int LRoom (void)
 *
 * PURPOSE
 *	Menu item to load and append a room.
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

int LRoom (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetFreqTitle (EasyFR[PATH_ROOM], "Load Room");
	LMapx (FALSE);		/* DON'T CLEAR the current map */
	return (TRUE);

} /* LRoom */


/**************************************************************************
 *
 * SRoom
 *
 * SYNOPSIS
 *		int SRoom (void)
 *
 * PURPOSE
 *		Save current room + tile infos for only tiles in this room.
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
int SRoom (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SaveRoom (TRUE, TRUE);
	return (TRUE);

} /* SRoom */


/**************************************************************************
 *
 * SRoomAll
 *
 * SYNOPSIS
 *		int SRoomAll (void)
 *
 * PURPOSE
 *		Save current room + tile infos for entire map.
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
int SRoomAll (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SRoomAll";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SaveRoom (FALSE, TRUE);
	return (TRUE);

} /* SRoomAll */


/*********************************************************************
 *
 * SaveLayer
 *
 * SYNOPSIS
 *		int SaveLayer(void)
 *
 * PURPOSE
 *		Save current floor of current room + tile info for only
 *		tiles used in this room.
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
int SaveLayer(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return SaveRoom (TRUE, FALSE);

} /* SaveLayer */


/**************************************************************************
 *
 * WCreate
 *
 * SYNOPSIS
 *		int WCreate (void)
 *
 * PURPOSE
 *		Menu item to bring up a new window (also creates a new room/window).
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
int WCreate (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WCreate";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	NotYet ();
	return (TRUE);

} /* WCreate */



/**************************************************************************
 *
 * WClose
 *
 * SYNOPSIS
 *		int WClose (void)
 *
 * PURPOSE
 *		Menu item to close a previously brought up tUME window.
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
int WClose (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WClose";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	NotYet ();
	return (TRUE);

} /* WClose */


/**************************************************************************
 *
 * WPRoom
 *
 * SYNOPSIS
 *		int WPRoom (void)
 *
 * PURPOSE
 *		Menu item to "slide" window to previous room.
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
int WPRoom (void)
{
	RoomType		*room, *oldroom;
	RoomStuffType	*roomstuff;
	BOOL			sourceflag, editflag;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WPRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ( (! IsEmpty (& (GlobalMap->Rooms))) && (GlobalRoomWindow) ) {

		roomstuff		= GlobalRoomWindow->CurrentRoom;
		room			= roomstuff->Room;
		sourceflag	= (roomstuff->Flags & SOURCE_LOCK);
		editflag		= (roomstuff->Flags & EDIT_LOCK);
		oldroom = room;
		if (room) {
			do {
				room = Prev (room);
				if (IsSOList (room)) {
					room = Tail (&(GlobalMap->Rooms));
				}
/**/				if (room == oldroom) goto ABORT;
			} while	(
				(sourceflag && (! (room->Flags & LOCKEDROOM))) ||
				(editflag && (room->Flags & LOCKEDROOM))
					);
		}
		else {
			room = Tail (&(GlobalMap->Rooms));
			while	(
				(sourceflag && (! (room->Flags & LOCKEDROOM))) ||
				(editflag && (room->Flags & LOCKEDROOM))
					) {

				room = Prev (room);
				if (IsSOList (room)) {
/**/					goto ABORT;
				}
			}
		}

		TryToHookRoomWindow (room, GlobalRoomWindow);
		ShowState (LastTBar);
		ShowRoom (GlobalRoomWindow);
		SetPointerMode();
		MButtonsE ();
	}

	return (TRUE);

/*-----------------------------------------------------------------------*/
ABORT:
/*	TellUser (NoCanDo, "No more to choose, either load tiles or allow."); */
	ShowStatus ("(no more to show)");
#if __AMIGAOS__
	DisplayBeep (GlobalRoomWindow->Window->WScreen);
#endif/*__AMIGAOS__*/
	return (TRUE);

} /* WPRoom */


/**************************************************************************
 *
 * WNRoom
 *
 * SYNOPSIS
 *		int WNRoom (void)
 *
 * PURPOSE
 *		Menu item to "slide" window to next room.
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
int WNRoom (void)
{
	RoomType		*room, *oldroom;
	RoomStuffType	*roomstuff;
	BOOL			sourceflag, editflag;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WNRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ( (! IsEmpty (& (GlobalMap->Rooms))) && (GlobalRoomWindow) ) {

		roomstuff		= GlobalRoomWindow->CurrentRoom;
		room			= roomstuff->Room;
		sourceflag	= (roomstuff->Flags & SOURCE_LOCK);
		editflag		= (roomstuff->Flags & EDIT_LOCK);
		oldroom = room;
		if (room) {
			do {
				room = Next (room);
				if (IsEOList (room)) {
					room = Head (&(GlobalMap->Rooms));
				}
/**/				if (room == oldroom) goto ABORT;
			} while	(
				(sourceflag && (! (room->Flags & LOCKEDROOM))) ||
				(editflag && (room->Flags & LOCKEDROOM))
					);
		}
		else {
			room = Head (&(GlobalMap->Rooms));
			while	(
				(sourceflag && (! (room->Flags & LOCKEDROOM))) ||
				(editflag && (room->Flags & LOCKEDROOM))
					) {

				room = Next (room);
				if (IsEOList (room)) {
/**/					goto ABORT;
				}
			}
		}

		TryToHookRoomWindow (room, GlobalRoomWindow);
		ShowState (LastTBar);
		ShowRoom (GlobalRoomWindow);
		SetPointerMode();
		MButtonsE ();
	}

	return (TRUE);
/*------------------------------------------------------------------------*/
ABORT:
	ShowStatus ("(no more to show)");
#if __AMIGAOS__
	DisplayBeep (GlobalRoomWindow->Window->WScreen);
#endif/*__AMIGAOS__*/
	return (TRUE);

} /* WNRoom */


/**************************************************************************
 *
 * WFRoom
 *
 * SYNOPSIS
 *		int WFRoom (void)
 *
 * PURPOSE
 *		Menu item to "flip" window to other side.
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
 *		06/07/93 Monday (dcc) - add "Smart Flip".
 *
 * SEE ALSO
 *
*/
int WFRoom (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WFRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow)
	{
		if (SmartFlipSTATE)
		{
			SmartFlipPrw(GlobalRoomWindow);
		}
		else
		{
			TogglePrw(GlobalRoomWindow);
			CurrentX = CurrentY = 0;
		}

		if (LastTBar == SHOW_TITLE)
			ShowState (SHOW_DEFAULT);
		else
			ShowState (LastTBar);

		ShowRoom (GlobalRoomWindow);
		SetPointerMode();
	}

	MButtonsE ();

	return (TRUE);

} /* WFRoom */


/**************************************************************************
 *
 * US
 *
 * SYNOPSIS
 *		int US (void)
 *
 * PURPOSE
 *		Menu item to "unselect" a brush.
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
int US (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "US";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Unselect ();
	ShowState (LastTBar);
	return (TRUE);

} /* US */


/**************************************************************************
 *
 * US2
 *
 * SYNOPSIS
 *		int US2 (void)
 *
 * PURPOSE
 *		Menu item to "unselect" a brush in prep for SINGLE LEVEL copy.
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
 *		11/12/89 Sunday - Created out of US. (RGM)
 *
 * SEE ALSO
 *
*/
int US2 (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "US2";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Unselect2 ();
	ShowState (LastTBar);
	return (TRUE);

} /* US2 */


/**************************************************************************
 *
 * WLS
 *
 * SYNOPSIS
 *		int WLS (void)
 *
 * PURPOSE
 *		Menu item to lock window side to source rooms.
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
int WLS (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WLS";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	GlobalRoomWindow->CurrentRoom->Flags &= EDIT_LOCK_MASK;
	GlobalRoomWindow->CurrentRoom->Flags |= SOURCE_LOCK;

#if __MSDOS__
//	ExcludeItems(WLockToSourceState.ExcludeList);
	SetStateVar(&WLockToSourceState, *WLockToSourceState.StateVar);
#endif/*__MSDOS__*/

	return (TRUE);

} /* WLS */


/**************************************************************************
 *
 * WLE
 *
 * SYNOPSIS
 *		int WLE (void)
 *
 * PURPOSE
 *		Menu item to lock window side to edit rooms.
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
int WLE (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WLE";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	GlobalRoomWindow->CurrentRoom->Flags &= SOURCE_LOCK_MASK;
	GlobalRoomWindow->CurrentRoom->Flags |= EDIT_LOCK;

#if __MSDOS__
//	ExcludeItems(WLockToEditState.ExcludeList);
	SetStateVar(&WLockToEditState, *WLockToEditState.StateVar);
#endif/*__MSDOS__*/

	return (TRUE);

} /* WLE */


/**************************************************************************
 *
 * WLC
 *
 * SYNOPSIS
 *		int WLC (void)
 *
 * PURPOSE
 *		Menu item to lock window side to current type of rooms..
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
int WLC (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WLC";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom->Room) {
		if (GlobalRoomWindow->CurrentRoom->Room->Flags & LOCKEDROOM) {
			WLS ();
		}
		else {
			WLE ();
		}
	}
	return (TRUE);

} /* WLC */



/**************************************************************************
 *
 * WLCl
 *
 * SYNOPSIS
 *		int WLCl (void)
 *
 * PURPOSE
 *		Menu item to clear window side locks.
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
int WLCl (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WLCl";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	GlobalRoomWindow->CurrentRoom->Flags &= EDIT_LOCK_MASK;
	GlobalRoomWindow->CurrentRoom->Flags &= SOURCE_LOCK_MASK;
	return (TRUE);

} /* WLCl */


/**************************************************************************
 *
 * SpaceToggle
 *
 * SYNOPSIS
 *		int SpaceToggle (void)
 *
 * PURPOSE
 *		Menu item to toggle tiles spaced/flush.
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
int SpaceToggle (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SpaceToggle";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom) {
		if (SpaceSTATE) {
			GlobalRoomWindow->CurrentRoom->Flags |= DISPLAY_SEPERATED;
		}
		else {
			GlobalRoomWindow->CurrentRoom->Flags &= DISPLAY_SEPERATED_MASK;
		}
		ShowRoom (GlobalRoomWindow);
	}
	return (TRUE);

} /* SpaceToggle */


/*********************************************************************
 *
 * MoveFloorUp
 *
 * PURPOSE
 *		Menu item to bring the current room's floor up.
 *
 * INPUT
 *		fAllTheWay	: if TRUE, move the topmost floor
 *					: if FALSE, move up one layer
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Always returns TRUE.
 *
 * HISTORY
 *		08/05/93 Thursday (dcc) - created out of old FloorUp().
 *		08/24/94 (dcc) - add support for moving brush into different size layer
 *		09/04/94 (dcc) - added call to MButtonsE()
 *
*/
static int near MoveFloorUp(BOOL fAllTheWay)
{
	LayerType	*layer;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MoveFloorUp";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom->Room)
	{
		layer = GlobalRoomWindow->CurrentRoom->Room->FloorLayer;

		while ((layer) && (! IsEOList (layer)))
		{
			layer = Next (layer);
			if (! IsEOList (layer))
			{
				GlobalRoomWindow->CurrentRoom->Room->FloorLayer = layer;
				(GlobalRoomWindow->CurrentRoom->Room->FloorNumber) ++;
			}
			if (!fAllTheWay)
				break;
		}
		ShowState (LastTBar);
#if PixelLayers
//		if (	EditOnlyFloorSTATE || fShowTileUsage ||
//			(GlobalBlockCopy && GlobalBlockCopy->BC_FloorLayer->cxTile != layer->cxTile) ||
//			(GlobalBlockCopy && GlobalBlockCopy->BC_FloorLayer->cyTile != layer->cyTile))
#else // !PixelLayers
		if (EditOnlyFloorSTATE || fShowTileUsage)
#endif // !PixelLayers
		{
			ShowRoom (GlobalRoomWindow);
		}
#if PixelLayers
		MButtonsE();
#endif // PixelLayers
	}
	return TRUE;
} /* MoveFloorUp */


/**************************************************************************
 *
 * FloorUp
 *
 * SYNOPSIS
 *		int FloorUp (void)
 *
 * PURPOSE
 *		Menu item to bring the current room's floor up one layer.
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
int FloorUp (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FloorUp";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return MoveFloorUp(FALSE);

} /* FloorUp */


/*********************************************************************
 *
 * TopFloor
 *
 * PURPOSE
 *		Menu item to move the floor to the topmost layer.
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
 *		08/05/93 Thursday (dcc) - created.
 *
*/
int TopFloor(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TopFloor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return MoveFloorUp(TRUE);

} /* TopFloor */


/*********************************************************************
 *
 * MoveFloorDown
 *
 * PURPOSE
 *		Menu item to bring the current room's floor down.
 *
 * INPUT
 *		fAllTheWay	: if TRUE, move the bottommost floor
 *					: if FALSE, move down one layer
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Always returns TRUE.
 *
 * HISTORY
 *		08/05/93 (dcc) - created out of old FloorDown().
 *		08/24/94 (dcc) - add support for moving brush into different size layer
 *		09/04/94 (dcc) - added call to MButtonsE()
 *
*/
static int near MoveFloorDown(BOOL fAllTheWay)
{
	LayerType	*layer;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MoveFloorDown";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom->Room)
	{
		layer = GlobalRoomWindow->CurrentRoom->Room->FloorLayer;

		while ((layer) && (! IsSOList (layer)))
		{
			layer = Prev (layer);
			if (! IsSOList (layer))
			{
				GlobalRoomWindow->CurrentRoom->Room->FloorLayer = layer;
				(GlobalRoomWindow->CurrentRoom->Room->FloorNumber) --;
			}
			if (!fAllTheWay)
				break;
		}
		ShowState (LastTBar);
#if PixelLayers
//		if (	EditOnlyFloorSTATE || fShowTileUsage ||
//			(GlobalBlockCopy && GlobalBlockCopy->BC_FloorLayer->cxTile != layer->cxTile) ||
//			(GlobalBlockCopy && GlobalBlockCopy->BC_FloorLayer->cyTile != layer->cyTile))
#else // !PixelLayers
		if (EditOnlyFloorSTATE || fShowTileUsage)
#endif // !PixelLayers
		{
			ShowRoom (GlobalRoomWindow);
		}
#if PixelLayers
		MButtonsE();
#endif // PixelLayers
	}
	return TRUE;
} /* MoveFloorDown */


/**************************************************************************
 *
 * FloorDown
 *
 * SYNOPSIS
 *		int FloorDown (void)
 *
 * PURPOSE
 *		Menu item to bring the current room's floor down one layer.
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
int FloorDown (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FloorDown";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return MoveFloorDown(FALSE);

} /* FloorDown */


/*********************************************************************
 *
 * BottomFloor
 *
 * PURPOSE
 *		Menu item to move the floor to the bottommost layer.
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
 *		08/05/93 Thursday (dcc) - created.
 *
*/
int BottomFloor(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "BottomFloor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return MoveFloorDown(TRUE);

} /* BottomFloor */


/**************************************************************************
 *
 * AddorInsertLayer
 *
 * PURPOSE
 *		Menu item to add or a layer to the current room.
 *
 * USAGE
 *
 *
 * INPUT
 *		short f			: either ADD_LAYER or INSERT_LAYER.
#if UseSparseArray
 *		UWORD cxTileToUse	: if != 0 == tile size, else use floor tile size
 *		UWORD cyTileToUse	: if != 0 == tile size, else use floor tile size
 *		BOOL fMakeSparse	: if TRUE make a sparse array
#endif // !UseSparseArray
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
#if UseSparseArray
static int AddorInsertLayer(short f, UWORD cxTileToUse, UWORD cyTileToUse, BOOL fMakeSparse)
#else // !UseSparseArray
static int AddorInsertLayer(short f)
#endif // !UseSparseArray
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddorInsertLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;
	if (room)
	{
		if (FLockedRoom(room, (f & INSERT_LAYER) ? szIns : szAdd))
			;
		else if (FSourceRoom(room, (f & INSERT_LAYER) ? szIns : szAdd))
			;
		else if (FCompositeRoom(room, (f & INSERT_LAYER) ? szIns : szAdd))
			;
		else
		{
			LayerType *play;
			UWORD cxTile, cyTile;
			int ctilx, ctily;

			cxTile = room->FloorLayer->cxTile;
			ctilx  = room->FloorLayer->ctilx;

#if UseSparseArray
			if (cxTileToUse > 0)
			{
				int cx = cxTile * ctilx;

				if (cx > 0)
					ctilx = cx / cxTileToUse;
				cxTile = cxTileToUse;
			}
#endif // UseSparseArray

			cyTile = room->FloorLayer->cyTile;
			ctily  = room->FloorLayer->ctily;

#if UseSparseArray
			if (cyTileToUse > 0)
			{
				int cy = cyTile * ctily;

				if (cy > 0)
					ctily = cy / cyTileToUse;
				cyTile = cyTileToUse;
			}
#endif // UseSparseArray

			if (f & INSERT_LAYER)
			{
#if UseSparseArray
				play = InsertClearedLayerToLayers(room, room->FloorLayer,
						ctilx, ctily, fMakeSparse);
#else // !UseSparseArray
				play = InsertClearedLayerToLayers(room, room->FloorLayer,
						ctilx, ctily);
#endif // !UseSparseArray
				room->FloorLayer = play;
			}
			else
			{
#if UseSparseArray
				play = AddClearedLayer(room, ctilx, ctily, fMakeSparse);
#else // !UseSparseArray
				play = AddClearedLayer(room, ctilx, ctily);
#endif // !UseSparseArray
			}

			if (play)
			{
				play->cxTile = cxTile;
				play->cyTile = cyTile;

				(room->LayerCount)++;
				ShowState (LastTBar);
				if (f & INSERT_LAYER)
					ShowRoom(GlobalRoomWindow);
			}
			else
			{
				TellUser (OOM, "Trouble allocating new layer.");
			}
		}
	}
	return TRUE;

} /* AddorInsertLayer */


/**************************************************************************
 *
 * OpenLayer
 *
 * PURPOSE
 *		Menu item to add a layer to the current room.
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
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
int OpenLayer (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if UseSparseArray
	// USESPARSEARRAY FIXME BUGBUG
	return AddorInsertLayer(ADD_LAYER, 0, 0, FALSE);
#else // !UseSparseArray
	return AddorInsertLayer(ADD_LAYER);
#endif // !UseSparseArray

} /* OpenLayer */


/*********************************************************************
 *
 * InsertLayer
 *
 * PURPOSE
 *		Menu item to insert a layer to the current room.
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
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
int InsertLayer(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InsertLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if UseSparseArray
	// USESPARSEARRAY FIXME BUGBUG
	return AddorInsertLayer(INSERT_LAYER, 0, 0, FALSE);
#else // !UseSparseArray
	return AddorInsertLayer(INSERT_LAYER);
#endif // !UseSparseArray

} /* InsertLayer */


#if UseSparseArray
#if PixelLayers
/*********************************************************************
 *
 * AddPixelLayer
 *
 * PURPOSE
 *		Menu item to add a pixel layer to the current room.
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
 *		08/23/94 (dcc) - created (based on OpenLayer()).
 *
*/
int AddPixelLayer (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddPixelLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	// USESPARSEARRAY FIXME BUGBUG
	return AddorInsertLayer(ADD_LAYER, 1, 1, TRUE);

} /* AddPixelLayer */


/*********************************************************************
 *
 * InsertPixelLayer
 *
 * PURPOSE
 *		Menu item to insert a pixel layer to the current room.
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
 *		08/23/94 (dcc) - created (based on InsertLayer()).
 *
*/
int InsertPixelLayer(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InsertPixelLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	// USESPARSEARRAY FIXME BUGBUG
	return AddorInsertLayer(INSERT_LAYER, 1, 1, TRUE);

} /* InsertPixelLayer */
#endif // PixelLayers
#endif // UseSparseArray


/**************************************************************************
 *
 * DeleteLayer
 *
 * SYNOPSIS
 *		int DeleteLayer (void)
 *
 * PURPOSE
 *		Menu item to delete a layer from the current room.
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
int DeleteLayer (void)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;
	if (room)
	{
		if (FLockedRoom(room, szDelLay))
			;
		else if (FSourceRoom(room, szDelLay))
			;
		else if (FCompositeRoom(room, szDelLay))
			;
		else if (room->LayerCount == 1)
		{
			TellUser (NoCanDo, "Can't delete the last layer.");
		}
		else if (room->FloorLayer->L_Flags & LAYER_LOCKED)
		{
			TellUser (NoCanDo, "Can't delete locked layer.");
		}
		else
		{
			char sz[256];

			sprintf(sz, "Do you want to delete layer %d? (Can't Undo!)", room->FloorNumber);

			if (AreYouSure(sz))
			{
				LayerType *layer = room->FloorLayer;

				if (!IsEOList(Next(layer)))
				{
					room->FloorLayer = Next(layer);
				}
				else
				{
					room->FloorLayer = Prev(layer);
					(room->FloorNumber) --;
				}
				Remove(layer);
				DeAllocateLayer(layer);
				(room->LayerCount) --;
				InvalidateEditRoomTileCounts(&room->prgtct, &room->fcntValid);
				ShowState (LastTBar);
				ShowRoom (GlobalRoomWindow);
			}
		}
	}
	return (TRUE);

} /* DeleteLayer */


/**************************************************************************
 *
 * RoomStatus
 *
 * SYNOPSIS
 *		int RoomStatus (void)
 *
 * PURPOSE
 *		Menu item to cause titlebar to display room information.
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
int RoomStatus (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RoomStatus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowState (SHOW_DEFAULT);
#if __MSDOS__
	OldColumn = OldRow = -1;
#endif/*__MSDOS__*/
	return (TRUE);

} /* RoomStatus */


/**************************************************************************
 *
 * UserStatus
 *
 * SYNOPSIS
 *		int UserStatus (void)
 *
 * PURPOSE
 *		Menu item to cause titlebar to display room-USER information.
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
int UserStatus (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UserStatus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowState (SHOW_USER);
#if __MSDOS__
	OldColumn = OldRow = -1;
#endif/*__MSDOS__*/
	return (TRUE);

} /* UserStatus */



/**************************************************************************
 *
 * VersionStatus
 *
 * SYNOPSIS
 *		int VersionStatus (void)
 *
 * PURPOSE
 *		Menu item to cause titlebar to briefly display tUME version info.
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
int VersionStatus (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "VersionStatus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowState (SHOW_VERSION);
	return (TRUE);

} /* VersionStatus */


/**************************************************************************
 *
 * CopyrightStatus
 *
 * SYNOPSIS
 *		int CopyrightStatus (void)
 *
 * PURPOSE
 *		Menu item to cause tilebar to breifly display tUME Copyright info.
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
int CopyrightStatus (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CopyrightStatus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowState (SHOW_TITLE);
	return (TRUE);

} /* CopyrightStatus */



/**************************************************************************
 *
 * CursorStatus
 *
 * SYNOPSIS
 *		int CursorStatus (void)
 *
 * PURPOSE
 *		Menu item to cause tilebar to display cursor co-ordinates.
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
int CursorStatus (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CursorStatus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowState (SHOW_CURSOR);
#if __MSDOS__
	OldColumn = OldRow = -1;
#endif/*__MSDOS__*/
	MButtonsE ();
	return (TRUE);

} /* CursorStatus */


/*********************************************************************
 *
 * LayerShiftStatus
 *
 * PURPOSE
 *		Menu item to cause tile bar to display layer shift amounts.
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
 *		02/20/94 Sunday (dcc) - created.
 *
*/
short LayerShiftStatus(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LayerShiftStatus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowState (SHOW_LAYERSHIFT);
	return TRUE;

} /* LayerShiftStatus */


/**************************************************************************
 *
 * TUserStatus
 *
 * SYNOPSIS
 *		int TUserStatus (void)
 *
 * PURPOSE
 *		Menu item to cause tilebar to display cursor co-ordinates.
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
int TUserStatus (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TUserStatus";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowState (SHOW_TUSER);
#if __MSDOS__
	OldColumn = OldRow = -1;
#endif/*__MSDOS__*/
	MButtonsE ();
	return (TRUE);

} /* TUserStatus */


/**************************************************************************
 *
 * ClearRoom
 *
 * SYNOPSIS
 *		int ClearRoom (void)
 *
 * PURPOSE
 *		Menu item to zero out the layers in a room.
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
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
int ClearRoom (void)
{
	RoomType	*room;
	LayerType	*layer;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ClearRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;

	if (room)
	{
		layer = Head (&(room->Layers));

		if (FLockedRoom(room, szClr))
			;
		else if (AreYouSure ("Do you want to clear this room? (Can't Undo!)")) {
			while (! IsEOList (layer))
			{
#if PLOTARRAY
				memset (layer->Plot, 0, PLOTSIZE * layer->ctilx * layer->ctily);
#else
#if UseSparseArray
				if (IsSparse(layer))
				{
					FreeSparseArray(layer->p.pspa);
					layer->p.pspa = MakeNewSparseArray();
				}
				else
				{
					UWORD iyt;

					for (iyt = 0; iyt < layer->ctily; iyt++)
					{
						PlotType *pplt;

						pplt = ActivatePlotRowatWin(layer->p.rgrgplt, iyt, winDst);

						memset(pplt, 0, layer->ctilx * PLOTSIZE);

						UpdatePlotRow(layer->p.rgrgplt, iyt);
					}
				}
#else // !UseSparseArray
				WORD		iyt;

				for (iyt = 0; iyt < layer->ctily; iyt++)
				{
					PlotType *pPt;

					pPt = ActivatePlotRowatWin(layer->rgrgplt, iyt, winDst);

					memset(pPt, 0, layer->ctilx * PLOTSIZE);

					UpdatePlotRow(layer->rgrgplt, iyt);
				}
#endif // !UseSparseArray
#endif
				layer = Next (layer);
			}
			ShowRoom (GlobalRoomWindow);
		}
	}
	return (TRUE);

} /* ClearRoom */


/**************************************************************************
 *
 * ClearRoom2
 *
 * SYNOPSIS
 *		int ClearRoom2 (void)
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
int ClearRoom2 (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ClearRoom2";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	NotYet ();
	return (TRUE);

} /* ClearRoom2 */


/**************************************************************************
 *
 * ClearRoom3
 *
 * SYNOPSIS
 *		int ClearRoom3 (void)
 *
 * PURPOSE
 *		Menu item to completely clear room (including tile size).
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
 *		01/18/93 (dcc) - erase any brush pointing to this room as well.
 *		04/14/91 (RGM) - Added ShowRoom call et al to make look right.
 *
 * SEE ALSO
 *
*/
int ClearRoom3 (void)
{
	RoomType	*room;
	LayerType *layer;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ClearRoom3";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;

	if (room)
	{
		if (FLockedRoom(room, szClr))
			;
		else
		{
			char sz[256];

			strcpy(sz, "Do you want to clear this room");

			if (GlobalBlockCopy->SourceRoom == room)
			{
				strcat(sz, ", and the current brush");
			}
			strcat(sz, "? (Can't Undo!)");

			if (AreYouSure (sz))
			{
				LayerType *play = Head(&room->Layers);
				short ctilxOld = play->ctilx;
				short ctilyOld = play->ctily;

				if (GlobalBlockCopy->SourceRoom == room)
				{
					Unselect();
				}
				ClearSearchBufferIfFromRoom(room);

				DeAllocateLayers (&(room->Layers));
				DeAllocateItems (&(room->Items));
#if UseSparseArray
				// USESPARSEARRAY FIXME BUGBUG
				if ( (layer = AddClearedLayer (room, ctilxOld, ctilyOld, TRUE)) == NULL)
#else // !UseSparseArray
				if ( (layer = AddClearedLayer (room, ctilxOld, ctilyOld)) == NULL)
#endif // !UseSparseArray
				{
					TellUser (OOM, "Can't initialize this room!");
				}
				else
				{
					room->FloorLayer	= layer;
					room->FloorNumber	= 1;
					room->LayerCount	= 1;
				}

				if (LastTBar == SHOW_TITLE)
				{
					ShowState (SHOW_DEFAULT);
				}
				else
				{
					ShowState (LastTBar);
				}

				ShowRoom (GlobalRoomWindow);
				SetPointerMode();
			}
		}
	}
	return (TRUE);

} /* ClearRoom3 */


/**************************************************************************
 *
 * ScrollLock
 *
 * SYNOPSIS
 *		int ScrollLock (void)
 *
 * PURPOSE
 *		Menu item to toggle scroll enable on & off.
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
int ScrollLock (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ScrollLock";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((GlobalRoomWindow->CurrentRoom->Flags) & SCROLL_LOCK) {
		GlobalRoomWindow->CurrentRoom->Flags &= INVERSE_W (SCROLL_LOCK);
	}
	else {
		GlobalRoomWindow->CurrentRoom->Flags |= SCROLL_LOCK;
	}
	return (TRUE);

} /* ScrollLock */


/**************************************************************************
 *
 * ScrollLockKEY
 *
 * SYNOPSIS
 *		int ScrollLockKEY (void)
 *
 * PURPOSE
 *		Key item to use ScrollLock with checking, DOESN'T WORK, THOUGH!!
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
int ScrollLockKEY (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ScrollLockKEY";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	TriggerEvent (&ScrollLockState);
#elif __MSDOS__
	TriggerEvent (&ScrollLockState, 0, NULL);
#endif/*__AMIGAOS__/__MSDOS__*/
	return (TRUE);

} /* ScrollLockKEY */


/**************************************************************************
 *
 * ToggleLInvisi
 *
 * SYNOPSIS
 *		int ToggleLInvisi (void)
 *
 * PURPOSE
 *		Menu item to toggle layer invisibility.
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
int ToggleLInvisi (void)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleLInvisi";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;
	if (room) {
		if (room->FloorLayer->L_Flags & LAYER_INVISIBLE) {
			room->FloorLayer->L_Flags &= INVERSE_W(LAYER_INVISIBLE);
		}
		else {
			room->FloorLayer->L_Flags |= LAYER_INVISIBLE;
		}
		ShowState (LastTBar);
		ShowRoom (GlobalRoomWindow);
	}
	return (TRUE);

} /* ToggleLInvisi */


/**************************************************************************
 *
 * ToggleLLock
 *
 * SYNOPSIS
 *		int ToggleLLock (void)
 *
 * PURPOSE
 *		Menu item to toggle layer locking.
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
int ToggleLLock (void)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleLLock";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;
	if (room) {
		if (room->FloorLayer->L_Flags & LAYER_LOCKED) {
			room->FloorLayer->L_Flags &= INVERSE_W(LAYER_LOCKED);
		}
		else {
			room->FloorLayer->L_Flags |= LAYER_LOCKED;
		}
		ShowState (LastTBar);
	}
	return (TRUE);

} /* ToggleLLock */


/**************************************************************************
 *
 * ToggleLAll
 *
 * SYNOPSIS
 *		int ToggleLAll (void)
 *
 * PURPOSE
 *		Menu item to toggle both layer locking AND invisibility.
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
int ToggleLAll (void)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleLAll";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;
	if (room) {
		if ((room->FloorLayer->L_Flags & LAYER_INVISIBLE) ||
			(room->FloorLayer->L_Flags & LAYER_LOCKED)) {
			room->FloorLayer->L_Flags &= INVERSE_W(LAYER_INVISIBLE);
			room->FloorLayer->L_Flags &= INVERSE_W(LAYER_LOCKED);
		}
		else {
			room->FloorLayer->L_Flags |= LAYER_INVISIBLE;
			room->FloorLayer->L_Flags |= LAYER_LOCKED;
		}
		ShowState (LastTBar);
		ShowRoom (GlobalRoomWindow);
	}
	return (TRUE);

} /* ToggleLAll */



/**************************************************************************
 *
 * ToggleLockRoom
 *
 * SYNOPSIS
 *		int ToggleLockRoom (void)
 *
 * PURPOSE
 *		Menu item to toggle locking of rooms.
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
int ToggleLockRoom (void)
{

	RoomType *prt = GlobalRoomWindow->CurrentRoom->Room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleLockRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prt)
	{
		if (FSelectedPblk(GlobalBlockCopy) && IsComposite(GlobalBlockCopy->SourceRoom))
		{
			Unselect ();
		}

		if (prt->Flags & LOCKEDROOM)
		{
			prt->Flags &= INVERSE_W(LOCKEDROOM);

			if (IsComposite(prt))
			{
				SetUnlockedCompositeRoom(prt);
			}
		}
		else {
			prt->Flags |= LOCKEDROOM;

			if (IsComposite(prt))
			{
				SetLockedCompositeRoom(prt);
			}
		}
		ShowState (LastTBar);
		ShowRoom (GlobalRoomWindow);
		MButtonsE ();
	}
	return (TRUE);

} /* ToggleLockRoom */



/**************************************************************************
 *
 * Undo
 *
 * SYNOPSIS
 *		int Undo (void)
 *
 * PURPOSE
 *		Menu item to Undo.
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
 *		08/30/94 (dcc) - add call to DCC_SetPointer()
 *
 * SEE ALSO
 *
*/
int Undo (void)
{
	short old_pointer;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Undo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	old_pointer = GlobalPointer;
	GlobalPointer = DCC_SetPointer (BPI_WAIT_POINTER);

	RetrieveUndo (UndoRoom);
	ShowState (LastTBar);
	ShowRoom (GlobalRoomWindow);

	GlobalPointer = DCC_SetPointer (old_pointer);
	return (TRUE);

} /* Undo */


#if 0
/**************************************************************************
 *
 * RenameRoom
 *
 * SYNOPSIS
 *		int RenameRoom (void)
 *
 * PURPOSE
 *		Menu item to rename a room.
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
int RenameRoom (void)
{
	RoomType	*room;
	char		*oldname = NULL;
	char		*newname;
	char		*nullname;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RenameRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	nullname = "";
	room = GlobalRoomWindow->CurrentRoom->Room;

/**/	if (! room) goto ABORT;

	SetColorsNPointer (MainWindow);
	if (room->Name) {
		oldname = room->Name;
	}
	else {
		oldname = nullname;
	}

	newname = GetName (GlobalRoomWindow->Window, oldname);

	if ((newname) && (*newname)) {
		oldname = MEM_calloc (strlen (newname) + 1, 1);
		if (oldname) {
			strcpy (oldname, newname);
		}
		MEM_free (room->Name);
		room->Name = oldname;

		if (IsComposite(room)) {
			oldname = MEM_calloc(strlen (newname) + 1, 1);
			if (oldname) {
				strcpy (oldname, newname);
			}
			MEM_free (room->ptsComposite->Filespec);
			room->ptsComposite->Filespec = oldname;
		}
	}
	ShowState (LastTBar);
ABORT:
	RestoreColorsNPointer (MainWindow);
	return (TRUE);

} /* RenameRoom */
#endif


#if 0
/**************************************************************************
 *
 * RInfo
 *
 * SYNOPSIS
 *		int RInfo (void)
 *
 * PURPOSE
 *		Menu item to display and get room info. ("RoomInfo")
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
int RInfo (void)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;

	if (room) {
		if (FSourceRoom(room, "change stats on"))
			;
		else {
			SetColorsNPointer (MainWindow);
			RoomInfo (GlobalRoomWindow->Window, room);
			RestoreColorsNPointer (MainWindow);
			ShowState (LastTBar);
			ShowRoom (GlobalRoomWindow);
		}
	}
	return (TRUE);

} /* RInfo */
#endif


#if 0
/**************************************************************************
 *
 * TUser
 *
 * SYNOPSIS
 *		int TUser (void)
 *
 * PURPOSE
 *		Menu item to get tile user numbers.
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
int TUser (void)
{
	TileSetType	*tileset = NULL;
	RoomType		*room;
	int			number;
	int			type;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TUser";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;

	if (FSelectedPblk(GlobalBlockCopy)) {
		tileset	= GlobalBlockCopy->FirstTileSet;
	}
	else if (IsSource(room) || IsComposite(room)) {
		tileset	= FindFirstTileSet (room);
	}
	if (tileset) {
		type		= tileset->UserType;
		number	= tileset->UserNumber;
		SetColorsNPointer (MainWindow);
		if (TU2_Request (&type, &number,
					BaseName (tileset->Filespec))) {
			tileset->UserType	= type;
			tileset->UserNumber	= number;
			ShowState (LastTBar);
		}
		RestoreColorsNPointer (MainWindow);
	}
	else {
		ShowStatus ("I can't find a tile for info.");
	}
	return (TRUE);

} /* TUser */
#endif


#if 0
/**************************************************************************
 *
 * URoom
 *
 * SYNOPSIS
 *		int URoom (void)
 *
 * PURPOSE
 *		Menu item to get room user numbers. ("RoomUser")
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
int URoom (void)
{
	int number;
	int type;
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "URoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;

	if (room) {
		if (! IS_SOURCE_ROOM (room)) {
			type = room->UserType;
			number = room->UserNumber;
			SetColorsNPointer (MainWindow);
			if (RU_Request (&type, &number)) {
				room->UserType = type;
				room->UserNumber = number;
				ShowState (LastTBar);
			}
			RestoreColorsNPointer (MainWindow);
		}
		else {
			TUser ();
		}
	}
	return (TRUE);

} /* URoom */
#endif


/**************************************************************************
 *
 * StripDownBlockCopy
 *
 * SYNOPSIS
 *		int StripDownBlockCopy (void)
 *
 * PURPOSE
 *		Menu item to strip draw block to one layer (floor).
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
int StripDownBlockCopy (void)
{
	LayerType *layer;
	LayerType *next_layer;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "StripDownBlockCopy";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (FSelectedPblk(GlobalBlockCopy)) {
		layer = Head (&GlobalBlockCopy->Layers);
		if (! IsEOList (layer)) {
			layer = Next (layer);
			while (! IsEOList (layer)) {
				next_layer = Next (layer);
				Remove (layer);
				DeAllocateLayer (layer);
				layer = next_layer;
			}
		}				

	}	
	return (TRUE);

} /* StripDownBlockCopy */


/**************************************************************************
 *
 * CopyColors1
 *
 * SYNOPSIS
 *		int CopyColors1 (void)
 *
 * PURPOSE
 *		Menu item to copy color stuff from one room to another.
 *		(The entire ColorInfo structure).
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
 *		11/12/89 Sunday - Created. (RGM)
 *		11/05/93 Friday (dcc) - add check for DuplicateColorInfo() failing.
 *
 * SEE ALSO
 *
*/
int CopyColors1 (void)
{
	ColorInfo		*temp_cinfo;
	RoomType		*source_room;
	RoomType		*dest_room;
	BlockCopyType	*block;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CopyColors1";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	block = GlobalBlockCopy;
	if (FSelectedPblk(block) && block->SourceRoom) {

		source_room	= block->SourceRoom;
		dest_room		= GlobalRoomWindow->CurrentRoom->Room;

		if (source_room && dest_room) {
			temp_cinfo = dest_room->R_ColorInfo;
			dest_room->R_ColorInfo =
					DuplicateColorInfo (source_room->R_ColorInfo);

			if (!dest_room->R_ColorInfo)	//unsuccessful?
			{
				dest_room->R_ColorInfo = temp_cinfo;
				TellUser (OOM, OOM);
				return TRUE;
			}
			if (IsGlobalColorsCopyOf(temp_cinfo)) {
				SetGlobalColors(dest_room->R_ColorInfo);
#if __AMIGAOS__
				SetColorRangeForReals (MainVp, 0, MAXCOLORREG - 1, (void *) GlobalColors->Colors);
#elif __MSDOS__
				SetDisplayPalette (ActivateXTRA(GlobalColors->pxtColors), GlobalColors->NumColors);
				ReleaseXTRA(GlobalColors->pxtColors);
#endif/*__AMIGAOS__/__MSDOS__*/
			}
			if (temp_cinfo) {
				DeleteColorInfo (temp_cinfo);
			}
		}
		else {
			TellUser (NoCanDo, "No room exists here.");
		}
	}
	else {
		TellUser (NoCanDo, "Please select a block first.");
	}
	return (TRUE);

} /* CopyColors1 */


/**************************************************************************
 *
 * CopyColors2
 *
 * PURPOSE
 *		Menu item to copy color stuff from one room to another.
 *		(just CMAP).
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
 *		11/12/89 Sunday - Created. (RGM)
 *		11/05/93 Friday (dcc) - Fix not working. Copy to GlobalColors->pxtColors
 *				instead of dest_room->R_ColorInfo->pxtColors.
 *				Add UseEditPaletteSTATE support.
 *
 * SEE ALSO
 *
*/
int CopyColors2 (void)
{
	/* ColorInfo		*temp_cinfo; */
	RoomType		*source_room;
	RoomType		*dest_room;
	BlockCopyType	*block;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CopyColors2";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	block = GlobalBlockCopy;
	if (FSelectedPblk(block) && block->SourceRoom) {

		source_room	= block->SourceRoom;
		dest_room		= GlobalRoomWindow->CurrentRoom->Room;

		if (source_room && dest_room) {
#if __AMIGAOS__
			unsigned short index;

			for (index = 0; index < MAXCOLORREG; index ++) {
				dest_room->R_ColorInfo->Colors[index] =
					source_room->R_ColorInfo->Colors[index];
			}
#elif __MSDOS__
			CS_Color	 rgcsc[MAX_CI_COLORS];

			ReadXTRA(source_room->R_ColorInfo->pxtColors, 0,		// read onto stack
					source_room->R_ColorInfo->NumColors * sizeof (CS_Color), rgcsc);

			if (UseEditPaletteSTATE && !IsEdit(dest_room))
			{
				/* Need to write directly to that room's palette. */

				WriteXTRA(rgcsc, source_room->R_ColorInfo->NumColors * sizeof (CS_Color),
					dest_room->R_ColorInfo->pxtColors, 0);
				dest_room->R_ColorInfo->NumColors = source_room->R_ColorInfo->NumColors;
			}
			else
			{
				WriteXTRA(rgcsc, source_room->R_ColorInfo->NumColors * sizeof (CS_Color),
					GlobalColors->pxtColors, 0);
				GlobalColors->NumColors = source_room->R_ColorInfo->NumColors;
			}
#endif/*__AMIGAOS__/__MSDOS__*/
#if __AMIGAOS__
			SetColorRangeForReals (MainVp, 0, MAXCOLORREG - 1, (void *) GlobalColors->Colors);
#elif __MSDOS__
			SetDisplayPalette (ActivateXTRA(GlobalColors->pxtColors), GlobalColors->NumColors);
			ReleaseXTRA(GlobalColors->pxtColors);
#endif/*__AMIGAOS__/__MSDOS__*/
		}
		else {
			TellUser (NoCanDo, "No room exists here.");
		}
	}
	else {
		TellUser (NoCanDo, "Please select a block first.");
	}
	return (TRUE);

} /* CopyColors2 */


/**************************************************************************
 *
 * CopyColors3
 *
 * SYNOPSIS
 *		int CopyColors3 (void)
 *
 * PURPOSE
 *		Menu item to copy color stuff from one room to another.
 *		(just CYCL).
 *
 *		NOTE: Amiga version not implemented.
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
 *		11/05/93 Friday (dcc) - created (based on CopyColor1()).
 *
 * SEE ALSO
 *
*/
int CopyColors3 (void)
{
	ColorInfo		*temp_cinfo;
	RoomType		*source_room;
	RoomType		*dest_room;
	BlockCopyType	*block;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CopyColors3";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	block = GlobalBlockCopy;
	if (FSelectedPblk(block) && block->SourceRoom) {

		source_room	= block->SourceRoom;
		dest_room		= GlobalRoomWindow->CurrentRoom->Room;

		if (source_room && dest_room) {
			CS_Color	 rgcsc[MAX_CI_COLORS];

			temp_cinfo = dest_room->R_ColorInfo;

			if (temp_cinfo)
			{
				ReadXTRA(temp_cinfo->pxtColors, 0,		// read old colors onto stack
					temp_cinfo->NumColors * sizeof (CS_Color), rgcsc);
			}
			dest_room->R_ColorInfo =
					DuplicateColorInfo (source_room->R_ColorInfo);

			if (!dest_room->R_ColorInfo)	//unsuccessful?
			{
				dest_room->R_ColorInfo = temp_cinfo;
				TellUser (OOM, OOM);
				return TRUE;
			}
			WriteXTRA(rgcsc, temp_cinfo->NumColors * sizeof (CS_Color),
				dest_room->R_ColorInfo->pxtColors, 0);	// restore old colors
			dest_room->R_ColorInfo->NumColors = temp_cinfo->NumColors;

			if (IsGlobalColorsCopyOf(temp_cinfo)) {
				SetGlobalColors(dest_room->R_ColorInfo);
#if __AMIGAOS__
				SetColorRangeForReals (MainVp, 0, MAXCOLORREG - 1, (void *) GlobalColors->Colors);
#elif __MSDOS__
				SetDisplayPalette (ActivateXTRA(GlobalColors->pxtColors), GlobalColors->NumColors);
				ReleaseXTRA(GlobalColors->pxtColors);
#endif/*__AMIGAOS__/__MSDOS__*/
			}
			if (temp_cinfo) {
				DeleteColorInfo (temp_cinfo);
			}
		}
		else {
			TellUser (NoCanDo, "No room exists here.");
		}
	}
	else {
		TellUser (NoCanDo, "Please select a block first.");
	}
	return (TRUE);

} /* CopyColors3 */


/**************************************************************************
 *
 * EditOnlyFloor
 *
 * SYNOPSIS
 *		int EditOnlyFloor (void)
 *
 * PURPOSE
 *		Menu item to toggle EditOnlyFloor.
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
 *		11/16/89 Thursday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
int EditOnlyFloor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "EditOnlyFloor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowState (LastTBar);
	ShowRoom (GlobalRoomWindow);
	return (TRUE);

} /* EditOnlyFloor */


#if 0
/**************************************************************************
 *
 * NTIMark
 *
 * SYNOPSIS
 *		int NTIMark (void)
 *
 * PURPOSE
 *		Menu item to toggle tileset image save marks.
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
 *		11/16/89 Thursday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
int NTIMark (void)
{
	RoomType		*room;
	TileSetType	*tileset;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "NTIMark";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;
	if (room->Flags & SOURCEROOM) {

		if (FSelectedPblk(GlobalBlockCopy)) {
			tileset	= GlobalBlockCopy->FirstTileSet;
		}
		else {
			tileset = FindFirstTileSet (room);
		}
		if (tileset) {
			if (tileset->Flags & DONT_SAVEIMAGES) {
				tileset->Flags &= INVERSE_W (DONT_SAVEIMAGES);
			}
			else {
				tileset->Flags |= DONT_SAVEIMAGES;
			}
			ShowState (LastTBar);
		}
		else {
			ShowStatus ("I can't find a tile for info.");
		}
	}
	else {
		ShowStatus ("Can only work with SOURCE rooms.");
	}
	return (TRUE);

} /* NTIMark */
#endif


/**************************************************************************
 *
 * IncGColor
 *
 * SYNOPSIS
 *		int IncGColor (void)
 *
 * PURPOSE
 *		Menu item to increment global color value.
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
 *		12/09/89 Saturday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
int IncGColor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "IncGColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	GlobalBackground += 1;
	if (GlobalBackground > MAXCOLORREG-1) GlobalBackground = 0;
	ShowRoom (GlobalRoomWindow);
	return (TRUE);

} /* IncGColor */


/**************************************************************************
 *
 * DecGColor
 *
 * SYNOPSIS
 *		int DecGColor (void)
 *
 * PURPOSE
 *		Menu item to decrement global color value.
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
 *		12/09/89 Saturday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
int DecGColor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DecGColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	GlobalBackground -= 1;
	if (GlobalBackground < 0) GlobalBackground = MAXCOLORREG-1;
	ShowRoom (GlobalRoomWindow);
	return (TRUE);

} /* DecGColor */


/**************************************************************************
 *
 * ZGColor
 *
 * SYNOPSIS
 *		int ZGColor (void)
 *
 * PURPOSE
 *		Menu item to reset global color value.
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
 *		12/09/89 Saturday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
int ZGColor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ZGColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	GlobalBackground = 0;
	ShowRoom (GlobalRoomWindow);
	return (TRUE);

} /* ZGColor */


#if 0
/**************************************************************************
 *
 * TileComment1
 *
 * SYNOPSIS
 *		int TileComment1 (void)
 *
 * PURPOSE
 *		Menu item to enter a tileset comment.
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
int TileComment1 (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TileComment1";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return TileComment_n (0);

} /* TileComment1 */

/**************************************************************************
 *
 * TileComment2
 *
 * SYNOPSIS
 *		int TileComment2 (void)
 *
 * PURPOSE
 *		Menu item to enter a tileset comment.
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
int TileComment2 (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TileComment2";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return TileComment_n (1);

} /* TileComment2 */
#endif


/**************************************************************************
 *
 * TE_TogglePointer
 *
 * SYNOPSIS
 *		int TE_TogglePointer (void)
 *
 * PURPOSE
 *	Menu item to toggle pointer on/off.
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
 *	09/20/90 Thursday (RGM) - Created.
 *
 *
 * SEE ALSO
 *
*/

#if __AMIGAOS__
static	void	*Previous_Pointer	= NULL;
#endif/*__AMIGAOS__*/

int TE_TogglePointer (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TE_TogglePointer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	if (HidePointerSTATE) {
		Previous_Pointer	= GlobalPointer;
		GlobalPointer		= RGM_SetPointer (&BlankMPointer, MainWindow);
	}
	else {
		GlobalPointer		= RGM_SetPointer (Previous_Pointer, MainWindow);
		Previous_Pointer	= NULL;
	}
#elif __MSDOS__
	DCC_SetPointer (GlobalPointer);
#endif/*__AMIGAOS__/__MSDOS__*/

	return (TRUE);
} /* TE_TogglePointer */


int TE_Debug (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TE_Debug";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (TRUE);
} /* TE_Debug */


#if __AMIGAOS__
int	TE_MakeColor1 (void)
{
	return (MakeColorX (1));
}

int	TE_MakeColor2 (void)
{
	return (MakeColorX (2));
}

int	TE_MakeColor3 (void)
{
	return (MakeColorX (3));
}

int	TE_MakeColor4 (void)
{
	return (MakeColorX (4));
}

int TE_MakeColorRevert (void)
{
	TU_DrawMode	= DMODE_COLORREVERT;
	SetPointerMode();
	return (TRUE);
}
#endif/*__AMIGAOS__*/

short ToggleJamPalette(void)
{
#if __MSDOS__
	if	(JamPaletteSTATE)
	{
		SetColors(MainWindow);
	}
	else
	{
		if (MouseY < 0 && ShowTitleSTATE)
			SetColors(MainWindow);
		else
			RestoreColors(MainWindow);
	}
#endif/*__MSDOS__*/
	return 1;
}

short ToggleShowBrush(void)
{
	return 1;
}


/*********************************************************************
 *
 * ToggleShowGuide
 *
 * PURPOSE
 *		Toggle whether or not to display guide.
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
 *
 *
 * SEE ALSO
 *
*/
short ToggleShowGuide(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleShowGuide";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom && GlobalRoomWindow->CurrentRoom->Room)
	{
		if (IsGuideVisible(GlobalRoomWindow->CurrentRoom->Room))
		{
			ResetGuideVisible(GlobalRoomWindow->CurrentRoom->Room);
		}
		else
		{
			SetGuideVisible(GlobalRoomWindow->CurrentRoom->Room);
		}
		ShowRoom (GlobalRoomWindow);
	}
	return (TRUE);

} /* ToggleShowGuide */


/*********************************************************************
 *
 * SetGridOrigin
 *
 * SYNOPSIS
 *		short SetGridOrigin(void)
 *
 * PURPOSE
 *		Set grid's origin to current mouse tile position.
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
short SetGridOrigin(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetGridOrigin";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom && GlobalRoomWindow->CurrentRoom->Room)
	{
		GlobalRoomWindow->CurrentRoom->Room->wGridXOrigin = CurrentX;
		GlobalRoomWindow->CurrentRoom->Room->wGridYOrigin = CurrentY;
		SetGridOn(GlobalRoomWindow->CurrentRoom->Room);
		ShowRoom (GlobalRoomWindow);
	}
	return (TRUE);

} /* SetGridOrigin */


/*********************************************************************
 *
 * SetGridSize
 *
 * SYNOPSIS
 *		short SetGridSize(void)
 *
 * PURPOSE
 *		Ask user for grid size.
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
short SetGridSize(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetGridSize";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom && GlobalRoomWindow->CurrentRoom->Room)
	{
		RoomType *prt = GlobalRoomWindow->CurrentRoom->Room;

		if (GR_Request(&prt->wGridXWidth,
					 &prt->wGridYHeight))
		{
			if (prt->wGridXWidth <= 0)
				prt->wGridXWidth = 1;
			if (prt->wGridYHeight <= 0)
				prt->wGridYHeight = 1;
			SetGridOn(GlobalRoomWindow->CurrentRoom->Room);
			ShowRoom (GlobalRoomWindow);
		}
	}
	return (TRUE);

} /* SetGridSize */


/*********************************************************************
 *
 * GetGridFromBrush
 *
 * SYNOPSIS
 *		short GetGridFromBrush(void)
 *
 * PURPOSE
 *		Set grid size from current brush.
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
short GetGridFromBrush(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetGridFromBrush";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom && GlobalRoomWindow->CurrentRoom->Room)
	{
		RoomType *prt = GlobalRoomWindow->CurrentRoom->Room;

		if (FSelectedPblk(GlobalBlockCopy))
		{
			LayerType *play = Head(&GlobalBlockCopy->Layers);

			prt->wGridXWidth = (play->ctilx > 0) ? play->ctilx : 1;
			prt->wGridYHeight = (play->ctily > 0) ? play->ctily : 1;
			ShowRoom (GlobalRoomWindow);
		}
	}
	return (TRUE);

} /* GetGridFromBrush */


/*********************************************************************
 *
 * ToggleUseGrid
 *
 * SYNOPSIS
 *		short ToggleUseGrid(void)
 *
 * PURPOSE
 *		Toggle whether to use grid or not.
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
short ToggleUseGrid(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleUseGrid";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom && GlobalRoomWindow->CurrentRoom->Room)
	{
		if (IsGridOn(GlobalRoomWindow->CurrentRoom->Room))
		{
			ResetGridOn(GlobalRoomWindow->CurrentRoom->Room);
		}
		else
		{
			SetGridOn(GlobalRoomWindow->CurrentRoom->Room);
		}
		ShowRoom (GlobalRoomWindow);
	}
	return (TRUE);

} /* ToggleUseGrid */


/*********************************************************************
 *
 * SetGuideOrigin
 *
 * SYNOPSIS
 *		short SetGuideOrigin(void)
 *
 * PURPOSE
 *		Set guide's origin to current mouse tile position.
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
short SetGuideOrigin(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetGuideOrigin";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom && GlobalRoomWindow->CurrentRoom->Room)
	{
#if NEWTUME
		RoomType *prm = GlobalRoomWindow->CurrentRoom->Room;

		short i;

		for (i = prm->cGuideLines-1; i >= 0; i--)
		{
			prm->pcenGuideXOrigin[i] = 100L * CurrentX;
			prm->pcenGuideYOrigin[i] = 100L * CurrentY;
		}
#else
		GlobalRoomWindow->CurrentRoom->Room->wGuideXOrigin = CurrentX;
		GlobalRoomWindow->CurrentRoom->Room->wGuideYOrigin = CurrentY;
#endif
		SetGuideVisible(GlobalRoomWindow->CurrentRoom->Room);
		ShowRoom (GlobalRoomWindow);
	}
	return (TRUE);

} /* SetGuideOrigin */


/*********************************************************************
 *
 * SetGuideSize
 *
 * SYNOPSIS
 *		short SetGuideSize(void)
 *
 * PURPOSE
 *		Ask user for guide size.
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
short SetGuideSize(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetGuideSize";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom && GlobalRoomWindow->CurrentRoom->Room)
	{
		RoomType *prt = GlobalRoomWindow->CurrentRoom->Room;
#if NEWTUME
		short y = (short) (prt->pcenGuideDC[0] / 100);
		short x = (short) (prt->pcenGuideDC[1] / 100);

		if (GU_Request(&x, &y))
		{
			if (x <= 0)
				x = 1;
			if (y <= 0)
				y = 1;

			prt->pcenGuideDC[0] = 100L * y;
			prt->pcenGuideDC[1] = 100L * x;

			SetGuideVisible(GlobalRoomWindow->CurrentRoom->Room);
			ShowRoom (GlobalRoomWindow);
		}
#else

		if (GU_Request(&prt->wGuideXWidth,
					 &prt->wGuideYHeight))
		{
			if (prt->wGuideXWidth <= 0)
				prt->wGuideXWidth = 1;
			if (prt->wGuideYHeight <= 0)
				prt->wGuideYHeight = 1;
			SetGuideVisible(GlobalRoomWindow->CurrentRoom->Room);
			ShowRoom (GlobalRoomWindow);
		}
#endif
	}
	return (TRUE);

} /* SetGuideSize */


/*********************************************************************
 *
 * GetGuideFromBrush
 *
 * SYNOPSIS
 *		short GetGuideFromBrush(void)
 *
 * PURPOSE
 *		Set guide size from current brush.
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
short GetGuideFromBrush(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetGuideFromBrush";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom && GlobalRoomWindow->CurrentRoom->Room)
	{
		RoomType *prt = GlobalRoomWindow->CurrentRoom->Room;

		if (FSelectedPblk(GlobalBlockCopy))
		{
			LayerType *play = Head(&GlobalBlockCopy->Layers);

#if NEWTUME
			prt->pcenGuideDC[0] = 100L * ((play->ctily > 0) ? play->ctily : 1);
			prt->pcenGuideDC[1] = 100L * ((play->ctilx > 0) ? play->ctilx : 1);
#else
			prt->wGuideXWidth = (play->ctilx > 0) ? play->ctilx : 1;
			prt->wGuideYHeight = (play->ctily > 0) ? play->ctily : 1;
#endif
			ShowRoom (GlobalRoomWindow);
		}
	}
	return (TRUE);

} /* GetGuideFromBrush */


/*********************************************************************
 *
 * XFlipBrush
 *
 * PURPOSE
 *		Menu item to flip the brush about the x-axis.
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
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
*/
short XFlipBrush(void)
{
#if __MSDOS__
	LayerType		*layer;

	if (FSelectedPblk(GlobalBlockCopy) && ubXFlipBit)
	{
		layer = Head (&(GlobalBlockCopy->Layers));

		while (! IsEOList (layer))
		{
			WORD	iX;
			WORD iY;
			
			for (iY = 0; iY < layer->ctily; iY++)
			{
#if UseSparseArray
				for (iX = 0; iX < layer->ctilx / 2; iX++)
				{
					PlotType pltSrc, pltDst;

					LAY_ReadPlotXY(layer, iX, iY, &pltSrc);
					pltSrc.Plot_Flags ^= ubXFlipBit;
					pltSrc.Plot_Flags &=
						FAST_TILESET_PTR(pltSrc.TileSet_ID)->ubShowFlagAND;

					LAY_ReadPlotXY(layer, layer->ctilx-iX-1, iY, &pltDst);
					pltDst.Plot_Flags ^= ubXFlipBit;
					pltDst.Plot_Flags &=
						FAST_TILESET_PTR(pltDst.TileSet_ID)->ubShowFlagAND;

					LAY_WritePlotXY(&pltSrc, layer, layer->ctilx-iX-1, iY);
					LAY_WritePlotXY(&pltDst, layer, iX, iY);
				}
				if (layer->ctilx & 1)
				{
					PlotType plt;

					LAY_ReadPlotXY(layer, layer->ctilx / 2, iY, &plt);

					plt.Plot_Flags ^= ubXFlipBit;
					plt.Plot_Flags &=
						FAST_TILESET_PTR(plt.TileSet_ID)->ubShowFlagAND;

					LAY_WritePlotXY(&plt, layer, layer->ctilx / 2, iY);
				}
#else // !UseSparseArray
				PlotType	*plot;
				PlotType	 temp_plot;

				plot = ActivatePlotRowatWin(layer->rgrgplt, iY, winDst);

				for (iX = 0; iX < layer->ctilx / 2; iX++)
				{
					temp_plot = plot[iX];
					temp_plot.Plot_Flags ^= ubXFlipBit;
					temp_plot.Plot_Flags &=
						FAST_TILESET_PTR(temp_plot.TileSet_ID)->ubShowFlagAND;
					plot[iX] = plot[layer->ctilx-iX-1];
					plot[iX].Plot_Flags ^= ubXFlipBit;
					plot[iX].Plot_Flags &=
						FAST_TILESET_PTR(plot[iX].TileSet_ID)->ubShowFlagAND;
					plot[layer->ctilx-iX-1] = temp_plot;
				}
				if (layer->ctilx & 1)
					plot[layer->ctilx / 2].Plot_Flags ^= ubXFlipBit;
					plot[layer->ctilx / 2].Plot_Flags &=
						FAST_TILESET_PTR(plot[layer->ctilx / 2].TileSet_ID)->ubShowFlagAND;

				UpdatePlotRow(layer->rgrgplt, iY);
#endif // !UseSparseArray
			}

			layer	= Next (layer);
		}
		if (ShowBrushSTATE)
		{
			ShowBrush();
		}
	}
#endif/*__MSDOS__*/
	return 1;
} /* XFlipBrush */


/*********************************************************************
 *
 * YFlipBrush
 *
 * PURPOSE
 *		Menu item to flip the brush about the y-axis.
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
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
*/
short YFlipBrush(void)
{
#if __MSDOS__
	LayerType		*layer;

	if (FSelectedPblk(GlobalBlockCopy) && ubYFlipBit)
	{
		layer = Head (&(GlobalBlockCopy->Layers));

		while (! IsEOList (layer))
		{
			UWORD iX;
			UWORD iY;

			for (iY = 0; iY < layer->ctily / 2; iY++)
			{
#if UseSparseArray
				UWORD itilyDst = layer->ctily-iY-1;

				for (iX = 0; iX < layer->ctilx; iX++)
				{
					PlotType pltSrc, pltDst;

					LAY_ReadPlotXY(layer, iX, iY, &pltSrc);
					pltSrc.Plot_Flags ^= ubYFlipBit;
					pltSrc.Plot_Flags &=
						FAST_TILESET_PTR(pltSrc.TileSet_ID)->ubShowFlagAND;

					LAY_ReadPlotXY(layer, iX, itilyDst, &pltDst);
					pltDst.Plot_Flags ^= ubYFlipBit;
					pltDst.Plot_Flags &=
						FAST_TILESET_PTR(pltDst.TileSet_ID)->ubShowFlagAND;

					LAY_WritePlotXY(&pltSrc, layer, iX, itilyDst);
					LAY_WritePlotXY(&pltDst, layer, iX, iY);
				}
#else // !UseSparseArray
				PlotType	*SrcPlot, *DstPlot;
				PlotType	 temp_plot;

				SrcPlot = ActivatePlotRowatWin(layer->rgrgplt, iY, winSrc);
				DstPlot = ActivatePlotRowatWin(layer->rgrgplt, layer->ctily-iY-1, winDst);

				for (iX = 0; iX < layer->ctilx; iX++)
				{
					temp_plot = SrcPlot[iX];
					temp_plot.Plot_Flags ^= ubYFlipBit;
					temp_plot.Plot_Flags &=
						FAST_TILESET_PTR(temp_plot.TileSet_ID)->ubShowFlagAND;
					SrcPlot[iX] = DstPlot[iX];
					SrcPlot[iX].Plot_Flags ^= ubYFlipBit;
					SrcPlot[iX].Plot_Flags &=
						FAST_TILESET_PTR(SrcPlot[iX].TileSet_ID)->ubShowFlagAND;
					DstPlot[iX] = temp_plot;
				}
				UpdatePlotRow(layer->rgrgplt, layer->ctily-iY-1);
				UpdatePlotRow(layer->rgrgplt, iY);
#endif // !UseSparseArray
			}

			if (layer->ctily & 1)
			{
#if UseSparseArray
				UWORD dtily = layer->ctily / 2;

				for (iX = 0; iX < layer->ctilx; iX++)
				{
					PlotType plt;

					LAY_ReadPlotXY(layer, iX, dtily, &plt);
					plt.Plot_Flags ^= ubYFlipBit;
					plt.Plot_Flags &=
						FAST_TILESET_PTR(plt.TileSet_ID)->ubShowFlagAND;
					LAY_WritePlotXY(&plt, layer, iX, dtily);
				}
#else // !UseSparseArray
				PlotType	*DstPlot;

				DstPlot = ActivatePlotRowatWin(layer->rgrgplt, layer->ctily / 2, winDst);

				for (iX = 0; iX < layer->ctilx; iX++)
				{
					DstPlot[iX].Plot_Flags ^= ubYFlipBit;
					DstPlot[iX].Plot_Flags &=
						FAST_TILESET_PTR(DstPlot[iX].TileSet_ID)->ubShowFlagAND;
				}
				UpdatePlotRow(layer->rgrgplt, layer->ctily / 2);
#endif // !UseSparseArray
			}
			layer	= Next (layer);
		}
		if (ShowBrushSTATE)
		{
			ShowBrush();
		}
	}
#endif/*__MSDOS__*/
	return 1;
} /* YFlipBrush */


/*********************************************************************
 *
 * DownloadMap
 *
 * SYNOPSIS
 *		short DownloadMap(short wColors)
 *
 * PURPOSE
 *		Call routine to download current screen.
 *
 * INPUT
 *		wColors:	number of colors to use per character.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		02/04/92 Tuesday (dcc) - created.
 *
 * SEE ALSO
 *
*/
static short DownloadMap(short wColors)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DownloadMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fEnableDownload)
	{
		if (GlobalRoomWindow->CurrentRoom->Room)
		{
			if (fGetDownloadHardwareAvailable())
			{
				int	wx, wy;

				GetCursorTilePosition (&wx, &wy, GlobalRoomWindow);

				SetColorsNPointer (MainWindow);
				DoDownloadMap(GlobalTileSpace, GlobalRoomWindow->CurrentRoom->Room, wx, wy, wColors);
				RestoreColorsNPointer (MainWindow);
			}
			else
				TellUser (NoCanDo, "Can't find target hardware.");
		}
	}
	else
		TellUser (NoCanDo, "Downloading disabled.");

	return 1;
} /* DownloadMap */


/*********************************************************************
 *
 * DownloadMap16
 *
 * SYNOPSIS
 *		short DownloadMap16(void)
 *
 * PURPOSE
 *		Download map using 16-color characters.
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
 *		11/13/92 Friday (dcc) - created.
 *
 * SEE ALSO
 *
*/
short DownloadMap16(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DownloadMap16";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	DownloadMap(16);

	return 1;
} /* DownloadMap16 */


/*********************************************************************
 *
 * DownloadMap256
 *
 * SYNOPSIS
 *		short DownloadMap256(void)
 *
 * PURPOSE
 *		Download map using 256-color characters.
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
 *		11/13/92 Friday (dcc) - created.
 *
 * SEE ALSO
 *
*/
short DownloadMap256(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DownloadMap256";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	DownloadMap(256);

	return 1;
} /* DownloadMap256 */


/*********************************************************************
 *
 * CountChars
 *
 * SYNOPSIS
 *		short CountChars(void)
 *
 * PURPOSE
 *		Call the routine to count characters.
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
short CountChars(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CountChars";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetColorsNPointer (MainWindow);
	ShowStatus("Counting characters...");
	DoCountChars(GlobalTileSpace, GlobalMap, GlobalRoomWindow->CurrentRoom->Room);
	ShowState(LastTBar);
	RestoreColorsNPointer (MainWindow);

	return 1;
} /* CountChars */


/*********************************************************************
 *
 * MakeCompositeTiles
 *
 * SYNOPSIS
 *		short MakeCompositeTiles(void)
 *
 * PURPOSE
 *		Turn tiles in room into composite tiles.
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
 *		09/08/94 (dcc) - KLUDGE BUGBUG FIXME - don't allow composite 1x1 tiles
 *
 * SEE ALSO
 *
*/
short MakeCompositeTiles(void)
{

	RoomType *prt = GlobalRoomWindow->CurrentRoom->Room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MakeCompositeTiles";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prt)
	{
		/* Make sure it's not a source room. */

		if (FSourceRoom(prt, "GridRoomAsTiles on"))
			;
		else
		{
			/* Make sure there is only one layer. */
			if (prt->LayerCount != 1)
			{
				TellUser (NoCanDo, "Room may only have one layer.");
			}
			else
			{
				/* Make sure it's not already a composite tileset. */

				if (IsComposite(prt))
				{
					TellUser (NoCanDo, "Room is already a composite tileset.");
				}
				else
				{
					/* Make sure there are tiles in room. */

					if (	prt->FloorLayer->cxTile == 0 ||
						prt->FloorLayer->cyTile == 0)
					{
						TellUser (NoCanDo, "No tiles for composite tileset.");
					}
					else
					{
#if PixelLayers
						if (	prt->FloorLayer->cxTile == 1 &&
							prt->FloorLayer->cyTile == 1)	// KLUDGE BUGBUG FIXME
						{
							TellUser (NoCanDo, "Can't composite 1x1 tiles.");
							return TRUE;
						}
#endif // PixelLayers
						if (!MakeCompositeTileSet(prt))
/**/						goto ABORT;
					}
				}
			}
		}
	}
	return TRUE;

ABORT:
	return FALSE;
} /* MakeCompositeTiles */


/*********************************************************************
 *
 * ToggleZoom
 *
 * SYNOPSIS
 *		short ToggleZoom(void)
 *
 * PURPOSE
 *		Toggle the show zoom state of the current room.
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
short ToggleZoom(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleZoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom &&
		GlobalRoomWindow->CurrentRoom->Room)
	{
		int	wx, wy;

		GetCursorTilePosition (&wx, &wy, GlobalRoomWindow);

		GlobalRoomWindow->CurrentRoom->Room->wZoomIndex ^= fSHOWZOOM;
		SetGlobalZoom(GlobalRoomWindow->CurrentRoom->Room);
		MoveCenterTo(wx, wy, FALSE);
	}
	return TRUE;
} /* ToggleZoom */


/*********************************************************************
 *
 * ZoomIn
 *
 * SYNOPSIS
 *		short ZoomIn(void)
 *
 * PURPOSE
 *		Zoom in to next level.
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
short ZoomIn(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ZoomIn";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom &&
		GlobalRoomWindow->CurrentRoom->Room)
	{
		RoomType *prt = GlobalRoomWindow->CurrentRoom->Room;

		if (((prt->wZoomIndex & wZOOMINDEX) > 0) ||
		    (!IsZooming(prt)))	
		{
			int	wx, wy;

			GetCursorTilePosition (&wx, &wy, GlobalRoomWindow);

			if (IsZooming(prt))
				prt->wZoomIndex--;
			else
				prt->wZoomIndex = GetZoomInOne();

			SetZoomOn(prt);
			SetGlobalZoom(prt);
			MoveCenterTo(wx, wy, FALSE);
		}
	}
	return TRUE;
} /* ZoomIn */


/*********************************************************************
 *
 * ZoomOut
 *
 * SYNOPSIS
 *		short ZoomOut(void)
 *
 * PURPOSE
 *		Zoom out to next level.
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
short ZoomOut(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ZoomOut";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom &&
		GlobalRoomWindow->CurrentRoom->Room)
	{
		RoomType *prt = GlobalRoomWindow->CurrentRoom->Room;

		if (((prt->wZoomIndex & wZOOMINDEX) < GetZoomMax()-1) ||
		    (!IsZooming(prt)))	
		{
			int	wx, wy;

			GetCursorTilePosition (&wx, &wy, GlobalRoomWindow);

			if (IsZooming(prt))
				prt->wZoomIndex++;
			else
				prt->wZoomIndex = GetZoomOutOne();

			SetZoomOn(prt);
			SetGlobalZoom(prt);
			MoveCenterTo(wx, wy, FALSE);
		}
	}
	return TRUE;
} /* ZoomOut */


/*********************************************************************
 *
 * SetRoomInfo
 *
 * SYNOPSIS
 *		short SetRoomInfo(void)
 *
 * PURPOSE
 *		Set the various info pertaining to rooms.
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
 *		04/27/94 Wednesday (dcc) - fix resizing composite assuming 2x2 tiles
 *
 * SEE ALSO
 *
*/
short SetRoomInfo(void)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetRoomInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;

	if (room)
	{
		if (FSourceRoom(room, "change stats on"))
			return TRUE;
		if (IsComposite(room) && IsLocked(room))
		{
			TellUser (NoCanDo, "Can't change locked composite room stats.");
			return TRUE;
		}
		{
#if 1
			LayerType *play = Head(&room->Layers);
			short ctilxOld = play->ctilx;
			short ctilyOld = play->ctily;
			short ctilxNew = ctilxOld;
			short ctilyNew = ctilyOld;

			GetRoomInfoReq (room, &ctilxNew, &ctilyNew, TRUE);

			if (ctilxOld != ctilxNew || ctilyOld != ctilyNew)
			{
				if (!ResizeRoom(room, ctilxNew, ctilyNew))
					TellUser (OOM, "Unable to re-size room.");

				if (IsComposite(room))
				{
					TileSetType *pts = room->ptsComposite;

					UWORD wWidthOld  = pts->Across+1;
					UWORD wHeightOld = pts->Down+1;

					pts->Across    = ctilxNew/pts->wTilesAcross-1;
					pts->Down      = ctilyNew/pts->wTilesDown-1;
					pts->MaxAcross = ctilxNew/pts->wTilesAcross;
					pts->TileCount = (UWORD) (pts->Across+1) * (pts->Down+1);

					FixCompositeIDs(pts->TS_id,
						wWidthOld, wHeightOld, pts->Across+1, pts->Down+1);
				}
			}

			ShowState (LastTBar);
			ShowRoom (GlobalRoomWindow);
#endif
#if 0
			GetRoomInfoReq (room, FALSE);
			ShowState (LastTBar);
#endif
		}
	}
	return TRUE;
} /* SetRoomInfo */


/*********************************************************************
 *
 * SetTilesetInfo
 *
 * SYNOPSIS
 *		short SetTilesetInfo(void)
 *
 * PURPOSE
 *		Set the various info pertaining to tilesets.
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
short SetTilesetInfo(void)
{
	TileSetType	*tileset = NULL;
	RoomType		*room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetTilesetInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;

	if (room)
	{
		if (IsSource(room))
		{
			if (FSelectedPblk(GlobalBlockCopy) &&
				(GlobalBlockCopy->SourceRoom == room))
			{
					tileset	= GlobalBlockCopy->FirstTileSet;
			}
			else
			{
				tileset	= FindFirstTileSet (room);
			}
		}
		else if (IsComposite(room) && IsLocked(room))
		{
			tileset = room->ptsComposite;
		}
		else if (FSelectedPblk(GlobalBlockCopy))
		{
			tileset	= GlobalBlockCopy->FirstTileSet;
		}
	}
	if (tileset)
	{
		GetTilesetInfoReq (tileset, FALSE);
		ShowState (LastTBar);
		ShowRoom (GlobalRoomWindow);
	}
	else
	{
		TellUser (NoCanDo, "No tiles selected for setting tileset info.");
	}
	return TRUE;
} /* SetTilesetInfo */


/*********************************************************************
 *
 * DeleteTileSet
 *
 * SYNOPSIS
 *		short DeleteTileSet(void)
 *
 * PURPOSE
 *		Delete selected tileset.
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
short DeleteTileSet(void)
{

	TileSetType	*tileset = NULL;
	RoomType		*room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = GlobalRoomWindow->CurrentRoom->Room;

	if (room)
	{
		if (IsSource(room))
		{
			if (FSelectedPblk(GlobalBlockCopy) &&
				(GlobalBlockCopy->SourceRoom == room))
			{
					tileset	= GlobalBlockCopy->FirstTileSet;
			}
			else
			{
				tileset	= FindFirstTileSet (room);
			}
		}
		else if (IsComposite(room) && IsLocked(room))
		{
			tileset = room->ptsComposite;
		}
	}
	if (tileset)
	{
		char sz[256];

		sprintf(sz, "All tiles in edit rooms using this tileset will be set to NULL. "
						"Do you want to delete tileset '%s'? (Can't Undo!)\n", tileset->Filespec);

		if (AreYouSure(sz))
		{
			Unselect ();			/* Unselect current brush */
			ClearSearchBuffer();

			SetRoomTilesTo(tileset->TS_id, 0);
			DeleteTiles(tileset);

			ShowState (LastTBar);
			ShowRoom (GlobalRoomWindow);
		}
	}
	else
	{
		TellUser (NoCanDo, "No tileset selected for deletion.");
	}
	return TRUE;

} /* DeleteTileSet */


/*********************************************************************
 *
 * PrintMap
 *
 * PURPOSE
 *		Call requester to print current room map.
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
 *		03/26/95 (dcc) - Add check for NULL FindNonEmptyLayer().
 *
 * SEE ALSO
 *
*/
short PrintMap(void)
{
	RoomType *prm;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PrintMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	prm = GlobalRoomWindow->CurrentRoom->Room;

	if (prm)
	{
		LayerType *play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);

		if (play && play->cxTile && play->cyTile)
			PrintMapReq(GlobalRoomWindow);
	}
	return TRUE;
} /* PrintMap */


/*********************************************************************
 *
 * KeepDownloadPalette
 *
 * SYNOPSIS
 *		short KeepDownloadPalette(void)
 *
 * PURPOSE
 *		Toggle continually download palette mode.
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
short KeepDownloadPalette(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "KeepDownloadPalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (KeepDownloadPaletteSTATE)
	{
		if (fEnableDownload && fGetDownloadHardwareAvailable())
			UpdateHrdwrColorRoutinePtr = GrabNDownloadPalette;
	}
	else
	{
		UpdateHrdwrColorRoutinePtr = NULL;
	}
	return TRUE;

} /* KeepDownloadPalette */


/*********************************************************************
 *
 * ShowBrushCount
 *
 * SYNOPSIS
 *		short ShowBrushCount(void)
 *
 * PURPOSE
 *		Count the occurrences of the global brush in the
 *		current layers and above of the current room.
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
 *		09/09/94 (dcc) - don't allow counting if tile at 0, 0 is NULL
 *
 * SEE ALSO
 *
*/
short ShowBrushCount(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowBrushCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (FSelectedPblk(GlobalBlockCopy))
	{
		RoomType *prt = GlobalRoomWindow->CurrentRoom->Room;
		if (prt)
		{
			char rgch[256];

#if PixelLayers
			if (	GlobalBlockCopy->BC_FloorLayer->cxTile == 1 &&
				GlobalBlockCopy->BC_FloorLayer->cyTile == 1 &&
				(GlobalBlockCopy->BC_FloorLayer->ctilx != 1 ||
				 GlobalBlockCopy->BC_FloorLayer->ctily != 1)	)
			{
				TellUser(NCD, "Please select only one object tile.");
				return TRUE;
			}

			{
				PlotType plt;

				LAY_ReadPlotXY(GlobalBlockCopy->BC_FloorLayer, 0, 0, &plt);
				if (fZeroTile(&plt))
				{
					TellUser(NCD, "First tile in brush must not be null.");
					return TRUE;
				}
			}
#endif // PixelLayers

			SetColorsNPointer (MainWindow);
			DCC_TempSetPointer (BPI_WAIT_POINTER);
			ShowStatus("Counting brush...");

			sprintf(rgch, "%lu occurrence(s) of the selected brush found.",
				CountBrush(prt, GlobalBlockCopy));
			TellUser("Brush Count", rgch);

			ShowState(LastTBar);
			RestoreColorsNPointer (MainWindow);
		}
	}
	return TRUE;
} /* ShowBrushCount */


/*********************************************************************
 *
 * SetStampPaint
 *
 * SYNOPSIS
 *		short SetStampPaint(void)
 *
 * PURPOSE
 *		Set brush to stamp only the floor layer.
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
short SetStampPaint(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetStampPaint";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetStampMode(fSetStampPaint);
	ForceBrushRedraw();
	return TRUE;

} /* SetStampPaint */


/*********************************************************************
 *
 * SetStampReplace
 *
 * SYNOPSIS
 *		short SetStampReplace(void)
 *
 * PURPOSE
 *		Set brush to stamp only the floor layer.
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
short SetStampReplace(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetStampReplace";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetStampMode(fSetStampReplace);
	ForceBrushRedraw();
	return TRUE;

} /* SetStampReplace */

short ToggleDownloadOneScreen(void)
{
	return 1;
}


/*********************************************************************
 *
 * UseEditPalette
 *
 * SYNOPSIS
 *		short UseEditPalette(void)
 *
 * PURPOSE
 *		Toggle whether to display using edit palette only or not.
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
short UseEditPalette(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UseEditPalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowRoom (GlobalRoomWindow);
	return TRUE;

} /* UseEditPalette */


/*********************************************************************
 *
 * GoHome
 *
 * SYNOPSIS
 *		short GoHome(void)
 *
 * PURPOSE
 *		Move display to upper-left corner of map (coordinate (0, 0) ).
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
//BUGBUG: assumes 15-bit room sizes.
 *
 * SEE ALSO
 *
*/
short GoHome(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GoHome";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	MoveRequest (32767, 32767);
	return TRUE;

} /* GoHome */


/*********************************************************************
 *
 * GoEnd
 *
 * SYNOPSIS
 *		short GoEnd(void)
 *
 * PURPOSE
 *		Move display to lower-right corner of map.
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
//BUGBUG: assumes 15-bit room sizes.
 *
 * SEE ALSO
 *
*/
short GoEnd(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GoEnd";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	MoveRequest (-32767, -32767);
	return TRUE;

} /* GoEnd */


/*********************************************************************
 *
 * HighlightTile
 *
 * SYNOPSIS
 *		short HighlightTile(void)
 *
 * PURPOSE
 *		Show the next tile to be hilighted.
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
short HighlightTile(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "HighlightTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	HighlightNextNonNullTile();
	ShowHighlightedTile();
	return TRUE;

} /* HighlightTile */

