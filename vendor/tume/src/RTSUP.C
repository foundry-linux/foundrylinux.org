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
 * RTSUPPORT.C
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 09/26/89 
 *   MODIFIED : 11/08/94
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		Stuff having to do with room/tile interaction.
 *
 * HISTORY
 *		09/26/89 (RGM) - Created.
 *		03/17/93 (dcc) - Change TileSetType to use cinShared instead
 *						of SharedNumberColors & SharedColors[]. This has
 *						the effect of putting the palette in XTRA memory.
 *		04/05/93 (dcc) - use rectplot.h header
 *		08/22/94 (dcc) - add support for UseSparseArray
 *		11/08/94 (dcc) - Revised MarkUsedTileSet(), SetTilesetTo(),
 *					  and SetTileIDTo() to use GetNextSparsePlotInRange().
 *		11/08/94 (dcc) - Don't process pixel tile layers.
 *
*/
#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include <string.h>

#if __AMIGAOS__
#include <libraries/arpbase.h>		/* for BaseName() */

#if LATTICE
#include <proto/exec.h>
#endif
#endif/*__AMIGAOS__*/

#include "rectplot.h"
#include "tuglbl.h"
#include "rmtsglue.h"
#include "rm_tsinf.h"
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray

/**************************** C O N S T A N T S ***************************/

#define append_query	"\tWould you like to\tappend these tiles\tto an existing room?\t"

#define append_title	"Append tiles"
#define append_prefix	"Do you wish to append\tto room \t"
#define append_suffix	"?\t"

/******************************** T Y P E S *******************************/


/****************************** E X T E R N S *****************************/

#if __AMIGAOS__
extern	struct ArpBase	*ArpBase;
#endif/*__AMIGAOS__*/

/****************************** G L O B A L S *****************************/

WORD GlobalTileSetID	= 1;
WORD GlobalRoomID		= 1;

/******************************* L O C A L S ******************************/

static	char	choose_append [255];

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/**************************************************************************
 *
 * GetAppendRoom
 *
 * PURPOSE
 *		Keep bugging the user to choose from a list of existing rooms
 *		to append the tileset to.
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
 *		09/28/89 Thursday - created. (RGM)
 *		12/14/93 Tuesday (dcc) - revised to use cxDisplay & cyDisplay.
 *
 * SEE ALSO
 *
*/
RoomType *GetAppendRoom (
	TileSetType	*tileset,
	MapType		*map
)
{
	MatchRoomsType	possible_rooms;
	MatchType		*match;
	RoomType		*room;
	RoomType		*finalroom = NULL;
	int				response;
	BOOL			try_again;
	int cxDisplay = tileset->cxDisplay ? tileset->cxDisplay : tileset->Width;
	int cyDisplay = tileset->cyDisplay ? tileset->cyDisplay : tileset->Height;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetAppendRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((tileset) && (map))
	{
		/****************************************************/
		/* make a list of all the rooms with this tile size */

		InitList ((void *) &possible_rooms);

		room = Head (&(map->Rooms));

		while (! IsEOList (room))
		{
			if ((room->Flags & SOURCEROOM) &&
			  	(room->FloorLayer->cxTile == cxDisplay) &&
		  		(room->FloorLayer->cyTile == cyDisplay))
			{
				if ( (match = MEM_calloc (1, MATCHSIZE)) == NULL)
				{
/**/				goto ABORT;
				}
				match->MatchRoom = room;
				AddTail ((ListType *) &possible_rooms, match);
			}
			room = Next (room);
		}

		/**************************************/
		/* repeat this until the              */
		/* user tells us to use a new room or */
		/* to append to one of the ones in    */
		/* the room list                      */

		if (! (IsEmpty (&possible_rooms)) )
		{
			do
			{
				/**********************************************/
				/* ask the user if they want to append these  */
				/* tiles to one of the rooms in the room list */

				try_again = DoThis2 ("Load Tileset",
						BaseName (tileset->Filespec), append_query);

				if (try_again)
				{
					/***************************************/
					/* go through the rooms and            */
					/* ask the user permission to append   */
					/* a particular one                    */

					match = Head (&possible_rooms);

					while ( (! (IsEOList (match)) ) && (try_again) )
					{
						strcpy (choose_append, append_prefix);
						strcat (choose_append,
								BaseName(match->MatchRoom->Name));
						strcat (choose_append, append_suffix);

						response = IsItThisOne (append_title, choose_append);

						if (response)
						{
							try_again = FALSE;
							if (response == YES)
							{
								finalroom = match->MatchRoom;
							}
						}
						match = Next (match);
					}
				}
			} while (try_again);
		}

		/*************************************/
		/* get rid of the matching room list */

		while ((match = RemHead ((ListType *) &possible_rooms)) != NULL)
		{
			MEM_free (match);
		}

		/********************************/
		/* return the room to append to */

	}

	return (finalroom);

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:

	while ((match = RemHead ((ListType *) &possible_rooms)) != NULL)
	{
		MEM_free (match);
	}
	return (NULL);

} /* GetAppendRoom */


/**************************************************************************
 *
 * AppendTiles
 *
 * PURPOSE
 *		Place tileset in a pre-existing source room.
 *
 * USAGE
 *		ok = AppendTiles (NEW_TILESET_PTR, APPEND_ROOM_PTR);
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
 *		09/28/89 Thursday - Created. (RGM)
 *		03/07/91 Thursday (RGM) - Added support for placing 'as found' tiles
 *						on appending by increasing room width (if needed).
 *		12/14/93 Tuesday (dcc)-use CtilxDisplayFromTST & CtilyDisplayFromTST().
 *
 * SEE ALSO
 *
*/
BOOL AppendTiles (
	TileSetType	*tileset,
	RoomType		*room
)
{
#if PLOTARRAY
	long			tiles;
#endif
	short		old_height;
	short		height;
	short		width;
#if PLOTARRAY
	short		line_mod;
#endif

#if PLOTARRAY
	LayerType		*first_layer;
	PlotType		*newplot	= NULL;
#endif
	int			ctilxtst;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AppendTiles";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ctilxtst = CtilxDisplayFromTST(tileset);

	/************************************/
	/* allocate new plots for the layer */

	old_height	= room->FloorLayer->ctily;
	height		= room->FloorLayer->ctily + CtilyDisplayFromTST(tileset);

	if (room->FloorLayer->ctilx >= ctilxtst)
	{
		width	= room->FloorLayer->ctilx;
#if PLOTARRAY
		line_mod	= 0;
#endif
	}
	else
	{
		width	= ctilxtst;
#if PLOTARRAY
		line_mod	= width - room->Width;
#endif
	}

#if PLOTARRAY
	tiles	= (height * width);

	if (! (newplot = MEM_calloc (tiles, PLOTSIZE)) )
	{
/**/	goto ABORT;
	}

	/********************************************/
	/* copy the existing tile pointers from the */
	/* old plot layer to the new plot layer     */

	first_layer = (Head (&(room->Layers)));

	{
		PlotType	*source_plot	= first_layer->Plot;
		PlotType	*dest_plot		= newplot;

		short	row;
		short	column;

		for (row = 0; row < room->Height; row ++)
		{
			for (column = 0; column < room->Width; column ++)
			{
				*dest_plot++	= *source_plot++;
			}
			dest_plot	+= line_mod;
		}
	}

	/*************************************************************/
	/* de-allocate the old plot & point the layer to the new one */

	MEM_free (first_layer->Plot);
	first_layer->Plot = newplot;

	/******************************/
	/* adjust the room dimensions */

	room->Height	= height;
	room->Width	= width;
#endif

	/***********************************************/
	/* add in the tile pointers to the new tileset */

#if PLOTARRAY
	InitTileSetLayer (
		room,
		tileset,
		(long) (old_height * width)
	);
#else
	if (!ResizeRoom(room, width, height))
/**/	goto ABORT;

	InitTileSetLayer(room, tileset, old_height);
#endif

	/*************************************************/
	/* let the tileset know where its append room is */

	tileset->SourceRoom = room;

	return (TRUE);

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:

#if PLOTARRAY
	if (newplot) {
		MEM_free (newplot);
		newplot = NULL;
	}
#endif

	return (NULL);

} /* AppendTiles */


/*********************************************************************
 *
 * ComputeNewSourceWidth
 *
 * PURPOSE
 *		Figure out how wide the new source room should be
 *		after <ptst> has been deleted.
 *
 *		Also set new ptst->wDisplayRow of any tilesets that
 *		appear in the same source room after the tileset being
 *		deleted.
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
 *		06/08/93 Tuesday (dcc) - enhanced to fix ptst->wDisplayRow's.
 *		12/14/93 Tuesday (dcc)-use CtilxDisplayFromTST & CtilyDisplayFromTST().
 *
 * SEE ALSO
 *
*/
static WORD ComputeNewSourceWidth(TileSetType *ptst)
{

	TileSetType *ptstTmp;
	WORD wWidthNew = 0;
	int			ctilytst;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ComputeNewSourceWidth";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ctilytst = CtilyDisplayFromTST(ptst);

#if 0
	/* Tileset height == source display room height, only one tileset in
	   this room, so delete this room entirely (width = 0). */

	if (ctilytst == ptst->SourceRoom->Height)
		return 0;
#endif

	ptstTmp = Head(&ptst->MyTileSpace->TileSets);

	while (!IsEOList(ptstTmp))
	{
		if (ptstTmp != ptst && ptstTmp->SourceRoom == ptst->SourceRoom)
		{
			wWidthNew = max(wWidthNew, CtilxDisplayFromTST(ptstTmp));

			if (ptstTmp->wDisplayRow > ptst->wDisplayRow)
				ptstTmp->wDisplayRow -= ctilytst;
		}
		ptstTmp = Next(ptstTmp);
	}

	return wWidthNew;

} /* ComputeNewSourceWidth */


/*********************************************************************
 *
 * DeleteTiles
 *
 * PURPOSE
 *		Delete tileset <ptst>, and remove it from the room it
 *		was placed in.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *		08/09/92 Sunday (dcc) - created.
 *		12/14/93 Tuesday (dcc) - use CtilyDisplayFromTST().
 *
 * SEE ALSO
 *
*/
BOOL DeleteTiles(TileSetType *ptst)
{
	int ctilytst;
	WORD wWidthNew, wHeightNew;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteTiles";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ctilytst = CtilyDisplayFromTST(ptst);

	/* Find new size of display room */

	wWidthNew = ComputeNewSourceWidth(ptst);
	wHeightNew = ptst->SourceRoom->FloorLayer->ctily - ctilytst;

	if (wWidthNew)
	{
		RoomType *prm = GlobalRoomWindow->CurrentRoom->Room;

		if (!DeleteRoomRows(prm, ptst->wDisplayRow, ctilytst))
			return FALSE;

		if (!ResizeRoom(prm, wWidthNew, wHeightNew))
			return FALSE;
	}
	else	/* Just delete the room */
	{
		RoomType *prm;

		prm = GlobalRoomWindow->CurrentRoom->Room;
		WNRoom();
		if (prm == GlobalRoomWindow->CurrentRoom->Room) {
			TotalRoomUnhook (prm, GlobalRoomWindow);
			ShowState (LastTBar);
			ShowRoom (GlobalRoomWindow);
			MButtonsE ();
		}
		Remove (prm);
		DeAllocateRoom (prm);
	}

	/* Delete tileset from tileset list */

	RemoveTileSet(ptst);
	UnLoadTileSet(ptst);

	return TRUE;
} /* DeleteTiles */


/**************************************************************************
 *
 * PlaceTiles
 *
 * PURPOSE
 *
 *
 * USAGE
 *		OK = PlaceTiles (NEWTILESET_PTR, REQUESTED_ROOM_PTR, MAP_PTR);
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
 *		12/14/93 Tuesday (dcc) - revised to use cxDisplay & cyDisplay.
 *
 * SEE ALSO
 *
*/
RoomType *PlaceTiles (
	TileSetType	*tileset,
	RoomType	*requested_room,
	MapType		*map
)
{
//	BOOL			asfoundflag;

	RoomType		*newroom	= NULL;
	RoomType		*finalroom	= NULL;
	RoomType		*actualroom	= NULL;
	int cxDisplay = tileset->cxDisplay ? tileset->cxDisplay : tileset->Width;
	int cyDisplay = tileset->cyDisplay ? tileset->cyDisplay : tileset->Height;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PlaceTiles";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

//	asfoundflag	= (tileset->Flags & ASFOUND);

	/********************************/
	/* get a room to place tiles in */

	if ((requested_room) &&
		(requested_room->Flags & SOURCEROOM) &&
		(requested_room->FloorLayer->cxTile == cxDisplay) &&
		(requested_room->FloorLayer->cyTile == cyDisplay)) {

		finalroom = requested_room;
	}
	else if (!(tileset->Flags & DONT_APPEND)) {
		finalroom = GetAppendRoom (tileset, map);
	}

	/******************************************/
	/* now either we have a room to append to */
	/* or we have to make a new one           */

	if (finalroom) {
		tileset->Flags &= INVERSE_W(DONT_APPEND);

		/**********************************************/
		/* we have a match, so we append to this room */

		if (! AppendTiles (tileset, finalroom)) {
/**/		goto ABORT;
		}

		actualroom = finalroom;
	}
	else {
		/**************************************/
		/* no match, make up a whole new room */

		tileset->Flags |= DONT_APPEND;

		if ( (newroom = CreateTileSetRoom (tileset)) == NULL)
		{
/**/			goto ABORT;
		}

		/*****************************************/
		/* append this room to the map structure */

		if (map) {
			AddTail (&(map->Rooms), newroom);
		}

		actualroom = newroom;
	}
	return (actualroom);

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:

	if (newroom) {
		DeAllocateRoom (newroom);
	}

	return (NULL);

} /* PlaceTiles */


/**************************************************************************
 *
 * MarkUsedTileSets
 *
 * PURPOSE
 *		Mark all the tilesets used in a room with a flag <mark>.
 *		if <fAllLayers> is FALSE, then only check the floor layer
 *		of <prm>.
 *
 * USAGE
 *		MarkUsedTileSets (ROOM_PTR, TRUE, TILESPACE_PTR, MARKS);
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
 *		11/13/89 (RGM) - Created.
 *		05/11/93 (dcc) - fAllLayers added.
 *		08/01/94 (dcc) - add check for locked composite rooms.
 *		08/01/94 (dcc) - if composite room, mark composite TST
 *		08/22/94 (dcc) - add support for UseSparseArray
 *		11/08/94 (dcc) - Revised to use GetNextSparsePlotInRange().
 *		11/08/94 (dcc) - Don't process pixel tile layers.
 *
 * SEE ALSO
 *
*/
void MarkUsedTileSets (
	RoomType		*prm,
	BOOL			 fAllLayers,
	TileSpaceType	*tilespace,
	UWORD		 mark
)
{
	LayerType		*layer;
#if PLOTARRAY
	int			 plots;
#endif

#if __AMIGAOS__
	void			*old_pointer;
#endif/*__AMIGAOS__*/

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MarkUsedTileSets";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	old_pointer	= GlobalPointer;
#endif/*__AMIGAOS__*/

	if (prm && tilespace) {

		/***********************/
		/* show a busy pointer */

#if __AMIGAOS__
		GlobalPointer = RGM_SetPointer (&YieldMPointer, MainWindow);
#elif __MSDOS__
		DCC_TempSetPointer (BPI_WAIT_POINTER);
#endif/*__AMIGAOS__/__MSDOS__*/

		/****************************************/
		/* clear out the mark from all tilesets */

		UnmarkTileSets (tilespace, mark);

		/*********************************************/
		/* go through all the layers in the room and */
		/* mark all the tilesets that are used.      */

		if (IsComposite(prm) && IsLocked(prm))
			SetUnlockedCompositeRoom(prm);

#if PLOTARRAY
		plots = (prm->Width) * (prm->Height);
		layer = fAllLayers ? Head (&prm->Layers) : prm->FloorLayer;
		while (! IsEOList (layer)) {
			PlotType	*plot;
			PlotType	*maxplot;

			plot	= layer->Plot;
			maxplot	= plot;
			maxplot	+= plots;
			for ( ; plot < maxplot; plot ++) {

				if (plot->TileSet_ID) {
					TileSetType		*plot_tileset;
					
					plot_tileset = FAST_TILESET_PTR(plot->TileSet_ID);
					plot_tileset->SaveFlags |= mark;
				}

			}
		if (fAllLayers)
			layer = Next (layer);
		else
			break;
		}
#else
		layer = fAllLayers ? Head (&prm->Layers) : prm->FloorLayer;
		while (!IsEOList(layer))
		{
			int ixt, iyt;

#if UseSparseArray
			if (IsSparse(layer))
			{
#if PixelLayers
				if (layer->cxTile != 1 || layer->cyTile != 1)	// KLUDGE BUGBUG FIXME
				{
#endif // PixelLayers
				PlotType *pplt;

				SetSparseLayerLimits(layer->p.pspa, 0, 0,
								 layer->ctilx, layer->ctily);

				while ((pplt = GetNextSparsePlotInRange(layer->p.pspa,
												&ixt, &iyt)) != NULL)
				{
					if (pplt->TileSet_ID)
					{
						TileSetType *plot_tileset;

						plot_tileset = FAST_TILESET_PTR(pplt->TileSet_ID);
						plot_tileset->SaveFlags |= mark;
					}
				}
#if PixelLayers
				}
#endif // PixelLayers
			}
			else
			{
				for (iyt = 0; iyt < layer->ctily; iyt++)
				{
					PlotType *pplt;

					pplt = ActivatePlotRowatWin(layer->p.rgrgplt, iyt, winSrc);

					for (ixt = 0; ixt < layer->ctilx; ixt++)
					{
						if (pplt->TileSet_ID)
						{
							TileSetType *plot_tileset;

							plot_tileset = FAST_TILESET_PTR(pplt->TileSet_ID);
							plot_tileset->SaveFlags |= mark;
						}
						pplt++;
					}
					ReleasePlotRow(layer->p.rgrgplt, iyt);
				}
			}
#else // !UseSparseArray
			for (iyt = 0; iyt < layer->ctily; iyt++)
			{
				PlotType *pplt;

				pplt = ActivatePlotRowatWin(layer->rgrgplt, iyt, winSrc);

				for (ixt = 0; ixt < layer->ctilx; ixt++)
				{
					if (pplt->TileSet_ID)
					{
						TileSetType		*plot_tileset;

						plot_tileset = FAST_TILESET_PTR(pplt->TileSet_ID);
						plot_tileset->SaveFlags |= mark;
					}
					pplt++;
				}
				ReleasePlotRow(layer->rgrgplt, iyt);
			}
#endif // !UseSparseArray
		if (fAllLayers)
			layer = Next (layer);
		else
			break;
		}
#endif
		if (IsComposite(prm))
		{
			prm->ptsComposite->SaveFlags |= mark;
			if (IsLocked(prm))
				SetLockedCompositeRoom(prm);
		}

		/*****************************/
		/* restore the mouse pointer */

#if __AMIGAOS__
		GlobalPointer = RGM_SetPointer (old_pointer, MainWindow);
#elif __MSDOS__
		DCC_SetPointer (GlobalPointer);
#endif/*__AMIGAOS__/__MSDOS__*/
	}
	return;

} /* MarkUsedTileSets */


/**************************************************************************
 *
 * UnmarkTileSets
 *
 * PURPOSE
 *
 *
 * USAGE
 *		UnmarkTileSets (TILESPACE_PTR, MARKS);
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
 *		11/15/89 Wednesday - Created from MarkUsedTileSets. (RGM)
 *
 * SEE ALSO
 *
*/
void UnmarkTileSets (
	TileSpaceType	*tilespace,
	UWORD			mark
)
{
	TileSetType	*tileset;	

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UnmarkTileSets";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (tilespace) {
		tileset = Head (&tilespace->TileSets);
		while (!IsEOList (tileset)) {
			tileset->SaveFlags &= INVERSE_W(mark);
			tileset = Next (tileset);
		}
	}
	return;

} /* UnmarkTileSets */


/*********************************************************************
 *
 * FillCompositeTileSet
 *
 * PURPOSE
 *		Fill out rest of tileset to point at room. The following
 *		values must be complete before calling this routine:
 *		ptst->TS_id;
 *		ptst->Flags;
 *		ptst->Width;
 *		ptst->Height;
 *		Also, all values in <prm> must be set to the unlocked state.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns TRUE if successful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL FillCompositeTileSet(TileSetType *ptst, RoomType *prm)
{

	int iWidth, iHeight;
#if __AMIGAOS__
	WORD wCnt = 0;
#endif/*__AMIGAOS__*/

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FillCompositeTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	iWidth	= ptst->Width / prm->FloorLayer->cxTile;
	iHeight	= ptst->Height / prm->FloorLayer->cyTile;

	ptst->Depth	= MAXPLANES;
	ptst->Across	= prm->FloorLayer->ctilx / iWidth - 1;
	ptst->Down	= prm->FloorLayer->ctily / iHeight - 1;
	ptst->TileCount= (UWORD) (ptst->Across+1) * (ptst->Down+1);
	ptst->MaxAcross= ptst->Across+1;

	InitList(&ptst->Tiles);

#if __AMIGAOS__
	if ((ptst->TilePtr = MEM_calloc((ptst->TileCount+1), sizeof(TileType *))) == NULL)
	{
/**/	goto ABORT;
	}

	{
		WORD wX, wY;

		for (wY = 0; wY <= ptst->Down; wY++)
		{
			for (wX = 0; wX <= ptst->Across; wX++)
			{
				TileType *pt;

				if ((pt = pTilNew()) == NULL)
				{
/**/				goto ABORT;
				}
				wCnt++;

				pt->TileNumber = wCnt;
				pt->Original_Column = wX;
				pt->Original_Row = wY;
				AddTail(&ptst->Tiles, pt);
				ptst->TilePtr[wCnt] = pt;	/* add tile to tile pointer array */
			}
		}
	}
#endif/*__AMIGAOS__*/

/*?????*/	ptst->SharedtransparentColor = 0;

	if (!CopyColorInfoToColorInfo(prm->R_ColorInfo, ptst->cinShared))
/**/	goto ABORT;

	ptst->SourceRoom	= prm;
	ptst->wTilesAcross	= iWidth;
	ptst->wTilesDown	= iHeight;
	return TRUE;

ABORT:
#if __AMIGAOS__
	{
		WORD i;

		for (i = 0; i < wCnt; i++)
			pTilDelete(ptst->TilePtr[wCnt]);
	}
#endif/*__AMIGAOS__*/
	return FALSE;
} /* FillCompositeTileSet */


/*********************************************************************
 *
 * MakeCompositeTileSet
 *
 * PURPOSE
 *		Actually make the composite tileset.
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
 *		08/01/94 Monday (dcc) - call LookupSomeTSTInfoFromUserType(),
 *					even though GetTilesetInfoReq() calls it, to
 *					make sure ptst->cxDisplay and ptst->cyDisplay
 *					are set properly.
 *
 * SEE ALSO
 *
*/
BOOL MakeCompositeTileSet(RoomType *prm)
{

	TileSetType *ptst;
#if 0
	int f;
#endif
	int iWidth, iHeight;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MakeCompositeTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if 0
	while ((prm->Name == NULL) ||
			(prm->Name != NULL && FindDuplicateRoomName(prm, GlobalMap) != NULL))
	{
		ShowStatus("Please enter a unique room name.\n");
		RenameRoom();
	}

	SetColorsNPointer (MainWindow);
	f = TS_Request(&iWidth, &iHeight);
	RestoreColorsNPointer (MainWindow);

	if (f)
#endif
	{
		if ((ptst = MEM_calloc(1, TILESETSIZE)) != NULL)
		{
			if ((ptst->cinShared = CreateColorInfo()) == NULL)
/**/			goto ABORT;

			ptst->TS_id = GetNextTilesetID (GlobalTileSpace);

#if 1
			ptst->SourceRoom = prm;
			prm->ptsComposite = ptst;
			do
			{
				if (!GetTilesetInfoReq (ptst, TRUE))
				{
					prm->ptsComposite = NULL;
/**/				goto ABORT;
				}
				iWidth	= ptst->Width;
				iHeight	= ptst->Height;

				if (iWidth > prm->FloorLayer->ctilx)
					TellUser(NoCanDo, "Composite tile width must be less than or equal to room width.");
				if (iHeight > prm->FloorLayer->ctily)
					TellUser(NoCanDo, "Composite tile height must be less than or equal to room height.");
			}
			while (iWidth > prm->FloorLayer->ctilx || iHeight > prm->FloorLayer->ctily);
#endif

#if 0
			if ((ptst->Filespec = MEM_calloc(strlen(prm->Name)+1, sizeof(char))) == NULL)
			{
/**/			goto ABORT;
			}
			strcpy(ptst->Filespec, prm->Name);
#endif
			ptst->Flags = TILESET_COMPOSITE;
			ptst->Width = prm->FloorLayer->cxTile * iWidth;
			ptst->Height = prm->FloorLayer->cyTile * iHeight;

			/* Set ptst->cxDisplay and ptst->cyDisplay */

			LookupSomeTSTInfoFromUserType(ptst);

			if (!FillCompositeTileSet(ptst, prm))
/**/			goto ABORT;

			GlobalTileSet = ptst;
			if (GlobalTileSpace)
			{
				AddTileSet(ptst, GlobalTileSpace);
			}
			prm->ptsComposite = ptst;
			prm->wOldRoomWidth = prm->FloorLayer->ctilx;
			prm->wOldRoomHeight = prm->FloorLayer->ctily;
			prm->Flags |= LOCKEDROOM;

			SetLockedCompositeRoom(prm);

			ShowState (LastTBar);
			ShowRoom (GlobalRoomWindow);

			return TRUE;
		}
	}
	return FALSE;

ABORT:
	DeleteColorInfo(ptst->cinShared);
	MEM_free(ptst);
	return FALSE;
} /* MakeCompositeTileSet */


/*********************************************************************
 *
 * LinkCompositeTileSets
 *
 * PURPOSE
 *		After loading in file, call this routine to link composite
 *		tilesets to the appropriate rooms.
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
BOOL LinkCompositeTileSets(TileSpaceType *ptsp, MapType *pmt, void *Translator)
{

	TileSetType *ptst;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LinkCompositeTileSets";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ptst = Head(&ptsp->TileSets);

	while (!IsEOList(ptst))
	{
		if ((ptst->Flags & GRID_TYPE_MODIFIER) == TILESET_COMPOSITE &&
			ptst->SourceRoom == NULL)	/* Make sure composite tileset 'unwired' */
		{
			RoomType *prm;
			RoomType *prmOld = NULL;
			RoomType *prmPre = NULL;

			prm = FindFirstSameRoomName(ptst->Filespec, pmt);

			if (!prm)
				return FALSE;		/* Can't find matching composite room!? */

			if (prm->Flags & PREROOM)
				prmPre = prm;
			else
				prmOld = prm;

			prm = FindNextSameRoomName(ptst->Filespec);

			if (prm && prm->Flags & PREROOM)
				prmPre = prm;
			else
				prmOld = prm;

			if (prmOld)				/* Another room w/ same name */
			{
				WORD wAnswer;
				char sz[256];

				strcpy(sz, "Composite tileset ");
				strcat(sz, BaseName(ptst->Filespec));
				strcat(sz, " already exists. Replace old composite tileset with new?");

				wAnswer = IsItThisOne("Load Room/Map", sz);
				if (wAnswer == CANCEL)
				{
					return FALSE;
				}
				else
				{
					WORD w;

					for (w = 0; w < MAX_TILESETS; w++)
					{
						if (GetCurrentID(w, Translator) == ptst->TS_id)
						{
							break;
						}
					}
					if (w >= MAX_TILESETS)	/* can't find id!? */
						return FALSE;

					if (wAnswer == YES)
					{
						/* Make new composite use same TS_id as old */

						ptst->TS_id = prmOld->ptsComposite->TS_id;

						/* Update references to new composite */

						PutID_Translate(w, ptst->TS_id, Translator);

						/* Link array to new tileset. */

						ptsp->TileSetArray[ptst->TS_id] = ptst;

						/* Delete old composite tileset, room */

						Remove(prmOld->ptsComposite);
						UnLoadTileSet(prmOld->ptsComposite);
						Remove(prmOld);
						DeAllocateRoom(prmOld);
					}
					else if (wAnswer == NO)
					{
						/* References to composite should use old */

						PutID_Translate(w, prmOld->ptsComposite->TS_id, Translator);

						/* Delete new composite tileset, room */

						Remove(ptst);
						UnLoadTileSet(ptst);
						Remove(prmPre);
						DeAllocateRoom(prmPre);
						prmPre = NULL;
					}
				}
			}

			if (prmPre)	/* Make sure composite room 'unwired' */
			{
				if (!FillCompositeTileSet(ptst, prmPre))
					return FALSE;

				prmPre->ptsComposite	= ptst;
				prmPre->wOldRoomWidth	= prmPre->FloorLayer->ctilx;
				prmPre->wOldRoomHeight	= prmPre->FloorLayer->ctily;

				if (IsLocked(prmPre))
					SetLockedCompositeRoom(prmPre);
			}
		}
		ptst = Next(ptst);
	}
	return TRUE;
} /* LinkCompositeTileSets */



/*********************************************************************
 *
 * SetLockedCompositeRoom
 *
 * PURPOSE
 *		Set room variables to reflect locked state.
 *
 * INPUT
 *		Room variables must be in unlocked state.
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
void SetLockedCompositeRoom(RoomType *prm)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetLockedCompositeRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	prm->FloorLayer->cxTile	*= prm->ptsComposite->wTilesAcross;
	prm->FloorLayer->cyTile	*= prm->ptsComposite->wTilesDown;
	prm->FloorLayer->ctilx	/= prm->ptsComposite->wTilesAcross;
	prm->FloorLayer->ctily	/= prm->ptsComposite->wTilesDown;

} /* SetLockedCompositeRoom */


/*********************************************************************
 *
 * SetUnlockedCompositeRoom
 *
 * PURPOSE
 *		Set room variables to reflect unlocked state.
 *
 * INPUT
 *		Room variables must be in unlocked state.
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
void SetUnlockedCompositeRoom(RoomType *prm)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetUnlockedCompositeRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	prm->FloorLayer->cxTile	/= prm->ptsComposite->wTilesAcross;
	prm->FloorLayer->cyTile	/= prm->ptsComposite->wTilesDown;
	prm->FloorLayer->ctilx	 = prm->wOldRoomWidth;
	prm->FloorLayer->ctily	 = prm->wOldRoomHeight;

} /* SetUnlockedCompositeRoom */


/*********************************************************************
 *
 * SetTilesetTo
 *
 * PURPOSE
 *		Change all occurrences of tileset <ubIDOld> in <prm> to <ubIDNew>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *		08/09/92 (dcc) - created.
 *		08/01/94 (dcc) - add check for locked composite rooms.
 *		08/22/94 (dcc) - add support for UseSparseArray
 *		11/08/94 (dcc) - Revised to use GetNextSparsePlotInRange().
 *		11/08/94 (dcc) - Don't process pixel tile layers.
 *
 * SEE ALSO
 *
*/
static BOOL SetTilesetTo(RoomType *prm, UBYTE ubIDOld, UBYTE ubIDNew)
{

	LayerType *play = Head(&prm->Layers);

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetTilesetTo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsComposite(prm) && IsLocked(prm))
		SetUnlockedCompositeRoom(prm);

	/* Process all layers. */

	while (!IsEOList(play))
	{
		int i;

#if UseSparseArray
		if (IsSparse(play))
		{
#if PixelLayers
			if (play->cxTile != 1 || play->cyTile != 1)	// KLUDGE BUGBUG FIXME
			{
#endif // PixelLayers
			int j;
			PlotType *pplt;

			SetSparseLayerLimits(play->p.pspa, 0, 0, play->ctilx, play->ctily);

			while ((pplt = GetNextSparsePlotInRange(play->p.pspa,
											&j, &i)) != NULL)
			{
				if (pplt->TileSet_ID == ubIDOld)
				{
					pplt->TileSet_ID = ubIDNew;

					WriteSparsePlotXY(pplt, play->p.pspa, j, i);
				}
			}
#if PixelLayers
			}
#endif // PixelLayers
		}
		else
		{
			for (i = 0; i < play->ctily; i++)
			{
				int j;
				PlotType *pplt;

				pplt = ActivatePlotRowatWin(play->p.rgrgplt, i, winDst);

				for (j = 0; j < play->ctilx; j++)
				{
					if (pplt->TileSet_ID == ubIDOld)
						pplt->TileSet_ID = ubIDNew;

					pplt++;
				}
				UpdatePlotRow(play->p.rgrgplt, i);
			}
		}
#else // !UseSparseArray
		for (i = 0; i < play->ctily; i++)
		{
			int j;
			PlotType *pPt;

			pPt = ActivatePlotRowatWin(play->rgrgplt, i, winDst);

			for (j = 0; j < play->ctilx; j++)
			{
				if (pPt->TileSet_ID == ubIDOld)
					pPt->TileSet_ID = ubIDNew;

				pPt++;
			}
			UpdatePlotRow(play->rgrgplt, i);
		}
#endif // !UseSparseArray
		play = Next(play);
	}

	if (IsComposite(prm) && IsLocked(prm))
		SetLockedCompositeRoom(prm);

	return TRUE;
} /* SetTilesetTo */


/*********************************************************************
 *
 * SetRoomTilesTo
 *
 * PURPOSE
 *		Set all tiles of type <ubIDOld> in all rooms to <ubIDNew>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *		08/09/92 Sunday (dcc) - created.
 *
 * SEE ALSO
 *
*/
BOOL SetRoomTilesTo(UBYTE ubIDOld, UBYTE ubIDNew)
{

	RoomType *prm;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetRoomTilesTo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	prm = Head (&GlobalMap->Rooms);

	while (!IsEOList(prm))
	{
		if (!IsSource(prm))
			SetTilesetTo(prm, ubIDOld, ubIDNew);
		prm = Next(prm);
	}
	return TRUE;
} /* SetRoomTilesTo */


/*********************************************************************
 *
 * SetTileIDTo
 *
 * PURPOSE
 *		This routine fixes all references to composite tiles
 *		after the user resizes a composite room.
 *
 *		Fix all tile IDs so they correspond to the old IDs, and
 *		make sure that the new IDs are within range. If the new
 *		ID is out of range, it is set to zero.
 *
 *		<ubID> contains the tileset ID to match, <wWidthOld>
 *		contains the old width of the
 *		composite tileset, and <wWidthNew> & <wHeightNew>
 *		contains the new width and height of the tileset.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *		08/31/92 (dcc) - created.
 *		04/27/94 (dcc) - fix not fixing all in locked composite rooms
 *		08/22/94 (dcc) - add support for UseSparseArray
 *		11/08/94 (dcc) - Revised to use GetNextSparsePlotInRange().
 *		11/08/94 (dcc) - Don't process pixel tile layers.
 *
 * SEE ALSO
 *
*/
static BOOL SetTileIDTo(RoomType *prm, UBYTE ubID,
		UWORD wWidthOld, UWORD wWidthNew, UWORD wHeightNew)
{

	LayerType *play = Head(&prm->Layers);
	short ctilx = play->ctilx;
	short ctily = play->ctily;


/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetTileIDTo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsComposite(prm) && IsLocked(prm))
	{
		ctilx *= prm->ptsComposite->wTilesAcross;
		ctily *= prm->ptsComposite->wTilesDown;
	}

	/* Process all layers. */

	while (!IsEOList(play))
	{
		int i;

#if UseSparseArray
		if (IsSparse(play))
		{
#if PixelLayers
			if (play->cxTile != 1 || play->cyTile != 1)	// KLUDGE BUGBUG FIXME
			{
#endif // PixelLayers
			int j;
			PlotType *pplt;

			SetSparseLayerLimits(play->p.pspa, 0, 0, ctilx, ctily);

			while ((pplt = GetNextSparsePlotInRange(play->p.pspa,
											&j, &i)) != NULL)
			{
				if (pplt->TileSet_ID == ubID)
				{
					UWORD wOldX = (pplt->Tile_ID-1) % wWidthOld;
					UWORD wOldY = (pplt->Tile_ID-1) / wWidthOld;

					if (wOldX < wWidthNew && wOldY < wHeightNew)
						pplt->Tile_ID = wOldY * wWidthNew + wOldX + 1;
					else
					{
						pplt->Plot_Flags = 0;
						pplt->TileSet_ID = 0;
						pplt->Tile_ID    = 0;
					}
					WriteSparsePlotXY(pplt, play->p.pspa, j, i);
				}
			}
#if PixelLayers
			}
#endif // PixelLayers
		}
		else
		{
			for (i = 0; i < ctily; i++)
			{
				int j;
				PlotType *pplt;

				pplt = ActivatePlotRowatWin(play->p.rgrgplt, i, winDst);

				for (j = 0; j < ctilx; j++)
				{
					if (pplt->TileSet_ID == ubID)
					{
						UWORD wOldX = (pplt->Tile_ID-1) % wWidthOld;
						UWORD wOldY = (pplt->Tile_ID-1) / wWidthOld;

						if (wOldX < wWidthNew && wOldY < wHeightNew)
							pplt->Tile_ID = wOldY * wWidthNew + wOldX + 1;
						else
						{
							pplt->Plot_Flags = 0;
							pplt->TileSet_ID = 0;
							pplt->Tile_ID    = 0;
						}
					}
					pplt++;
				}
				UpdatePlotRow(play->p.rgrgplt, i);
			}
		}
#else // !UseSparseArray
		for (i = 0; i < ctily; i++)
		{
			int j;
			PlotType *pplt;

			pplt = ActivatePlotRowatWin(play->rgrgplt, i, winDst);

			for (j = 0; j < ctilx; j++)
			{
				if (pplt->TileSet_ID == ubID)
				{
					UWORD wOldX = (pplt->Tile_ID-1) % wWidthOld;
					UWORD wOldY = (pplt->Tile_ID-1) / wWidthOld;

					if (wOldX < wWidthNew && wOldY < wHeightNew)
						pplt->Tile_ID = wOldY * wWidthNew + wOldX + 1;
					else
					{
						pplt->Plot_Flags = 0;
						pplt->TileSet_ID = 0;
						pplt->Tile_ID    = 0;
					}
				}
				pplt++;
			}
			UpdatePlotRow(play->rgrgplt, i);
		}
#endif // !UseSparseArray
		play = Next(play);
	}
	return TRUE;
} /* SetTileIDTo */


/*********************************************************************
 *
 * FixCompositeIDs
 *
 * PURPOSE
 *		Change old composite tile IDs to new IDs in all rooms.
 *		<ubID> contains the tileset ID to match, <wWidthOld> &
 *		<wHeightOld> contains the old width and height of the
 *		composite tileset, and <wWidthNew> & <wHeightNew>
 *		contains the new width and height of the tileset.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *		08/31/92 Monday (dcc) - created.
 *
 * SEE ALSO
 *
*/
BOOL FixCompositeIDs(UBYTE ubID, UWORD wWidthOld, UWORD wHeightOld, UWORD wWidthNew, UWORD wHeightNew)
{

	RoomType *prm;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FixCompositeIDs";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	wHeightOld = wHeightOld;

	prm = Head (&GlobalMap->Rooms);

	while (!IsEOList(prm))
	{
		if (!IsSource(prm))
			SetTileIDTo(prm, ubID, wWidthOld, wWidthNew, wHeightNew);
		prm = Next(prm);
	}
	return TRUE;
} /* FixCompositeIDs */

