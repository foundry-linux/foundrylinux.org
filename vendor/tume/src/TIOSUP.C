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
 * TIOSUP.C
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 02/06/90
 *   MODIFIED : 04/07/95
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *	Stuff having to do with io/tpackio.
 *
 * HISTORY
 *		01/03/91 (RGM) - Extensive re-write begins. . .this won't
 *						be easy. . .
 *		04/05/93 (dcc) - use rectplot.h header.
 *		03/05/95 (dcc) - Fix TranslateRooms() not working on sparse layers.
 *
*/
#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include <string.h>

#include "rectplot.h"
#include "tuglbl.h"
#include "readtume.h"
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** E X T E R N S *****************************/


/****************************** G L O B A L S *****************************/


/******************************* L O C A L S ******************************/

static	char TRMessage[50];

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

/**************************************************************************
 *
 * CreateID_Translate
 *
 * SYNOPSIS
 *		void *CreateID_Translate (void)
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
 *	11/14/89 Tuesday - Created (02:05 am). (RGM)
 *
 *	01/11/91 Friday (RGM) - Now uses array for translation values.
 *
 * SEE ALSO
 *
*/
void *CreateID_Translate (void)
{
	void	*id_translator;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateID_Translate";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	id_translator = MEM_calloc (MAX_TILESETS, 1);

	return (id_translator);

} /* CreateID_Translate */


/**************************************************************************
 *
 * DeleteID_Translate
 *
 * SYNOPSIS
 *		void DeleteID_Translate (void *id_translator)
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
 *		11/14/89 Tuesday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void DeleteID_Translate (void *id_translator)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteID_Translate";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SAFEFREE (id_translator);

	return;

} /* DeleteID_Translate */


/**************************************************************************
 *
 * PutID_Translate
 *
 * SYNOPSIS
 *		void PutID_Translate (
 *			WORD	old_id,
 *			WORD	new_id,
 *			void	*id_translator
 *		)
 *
 * PURPOSE
 *		To insert a new id translation into the table.
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
 *	01/11/91 Friday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
void PutID_Translate (
	WORD	old_id,
	WORD	new_id,
	void	*id_translator
)
{

	UBYTE	*array	= (UBYTE *) id_translator;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PutID_Translate";
#endif
#if REQUIRE
	if (old_id >= MAX_TILESETS) {

	}
	if (! old_id) {

	}

#endif
/*-------------------------------------------------------------------------*/

	array[old_id]	= new_id;

	return;

} /* PutID_Translate */


/**************************************************************************
 *
 * GetCurrentID (usta be TSTranslate)
 *
 * SYNOPSIS
 *		WORD GetCurrentID (
 *			WORD	old_id,
 *			void	*id_translator
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *		NEW_NUM = GetCurrentID (OLD_ID, TRANSLATOR_PTR);
 *
 * INPUT
 *		An old ID and the translator structure.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		A valid (current) id #.
 *
 * HISTORY
 *		11/14/89 Tuesday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
WORD GetCurrentID (
	WORD	old_id,
	void	*id_translator
)
{
	UBYTE	*array	= (UBYTE *) id_translator;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetCurrentID";
#endif
#if REQUIRE
	if (old_id >= MAX_TILESETS) {

	}
	if (! old_id) {

	}
#endif
/*-------------------------------------------------------------------------*/

	return ((WORD) array[old_id]);

} /* GetCurrentID */


/**************************************************************************
 *
 * TranslateRooms (usta be TransmorgrifyRoom (sic).
 *
 * PURPOSE
 *		To prepare a room for use when importing into an existing map.
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *		The room structure's data gets changed from virtual to actual
 *		tileset ID numbers.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		10/17/89 (RGM) - Created.
 *		11/14/89 (RGM) - Modified for translation table.
 *		01/03/91 (RGM) - Changed from pointer array based rooms to
 *					  ID array based rooms.
 *		01/11/91 (RGM) - New translator.
 *		08/19/94 (dcc) - Add support for UseSparseArray.
 *		03/05/95 (dcc) - Call WriteSparsePlotXY() to write updated
 *					  values back to sparse array.
 *		04/07/95 (dcc) - KLUDGE: Don't translate sparse layers with
 *					  tilewidth == 1 && tileheight == 1.
 *
 * SEE ALSO
 *
*/
void TranslateRooms (
	MapType	*map,
	WORD	match_flags,
	void	*id_translator
)
{
	RoomType	*room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TranslateRooms";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = Head (&map->Rooms);

	while (! IsEOList (room)) {
		if (room->Flags & match_flags) {

			strcpy (TRMessage, "Converting ");
			if (room->Name) {
				strncat	(TRMessage, room->Name, 20);
			}
			else {
				strncat	(TRMessage, "unnamed room.", 20);
			}
			ShowStatus (TRMessage);

			{
#if PLOTARRAY
				register	int				plots;
#endif
				register	LayerType		*layer;
				WORD	wRoomWidth	= ((LayerType *) Head(&room->Layers))->ctilx;
				WORD wRoomHeight	= ((LayerType *) Head(&room->Layers))->ctily;

				if (IsComposite(room))
				{
					wRoomWidth	= room->wOldRoomWidth;
					wRoomHeight	= room->wOldRoomHeight;
				}

#if PLOTARRAY
				plots = (wRoomWidth) * (wRoomHeight);
#endif
				layer = Head (&room->Layers);

				while (! IsEOList (layer)) {
#if !PLOTARRAY
					int ixt, iyt;
#endif
					register	PlotType		*plot;
#if PLOTARRAY
					register	PlotType		*maxplot;

					plot	= layer->Plot;
					maxplot	= plot;
					maxplot	+= plots;

					for ( ; plot < maxplot; plot ++) {
						if (plot->TileSet_ID) {
							plot->TileSet_ID	=
								GetCurrentID(
									(WORD) plot->TileSet_ID,
									id_translator
								);
						}
						if (plot->Tile_ID) {
							TileSetType	*ts;
							ts = TILESET_AT_PLOT(plot);

							if ((!ts) || (plot->Tile_ID > ts->TileCount)) {
								plot->TileSet_ID	= 0;
								plot->Tile_ID		= 0;
							}
						}
					}
#else // !PLOTARRAY
#if	UseSparseArray
					if (IsSparse(layer))
					{
/* 04/07/95 KLUDGE */		if (layer->cxTile != 1 || layer->cyTile != 1)
						{
							SetSparseLayerLimits(layer->p.pspa, 0, 0, 0, 0);

							while ((plot = GetNextSparsePlotInRange(layer->p.pspa, &ixt, &iyt)) != NULL)
							{
								if (plot->TileSet_ID)
								{
									plot->TileSet_ID =
										GetCurrentID(
												(WORD) plot->TileSet_ID,
												id_translator
											  	);
								}
								if (plot->Tile_ID)
								{
									TileSetType	*ts;
									ts = TILESET_AT_PLOT(plot);

									if ((!ts) || (plot->Tile_ID > ts->TileCount))
									{
										plot->TileSet_ID	= 0;
										plot->Tile_ID		= 0;
									}
								}
								WriteSparsePlotXY(plot, layer->p.pspa, ixt, iyt);
							}
						}
					}
					else
					{
						for (iyt = 0; iyt < wRoomHeight; iyt++)
						{
							plot = ActivatePlotRowatWin(layer->p.rgrgplt, iyt, winDst);

							for (ixt = 0; ixt < wRoomWidth; ixt++)
							{
								if (plot->TileSet_ID)
								{
									plot->TileSet_ID =
										GetCurrentID(
											(WORD) plot->TileSet_ID,
											id_translator
										);
								}
								if (plot->Tile_ID)
								{
									TileSetType	*ts;
									ts = TILESET_AT_PLOT(plot);

									if ((!ts) || (plot->Tile_ID > ts->TileCount))
									{
										plot->TileSet_ID	= 0;
										plot->Tile_ID		= 0;
									}
								}
								plot++;
							}
							UpdatePlotRow(layer->p.rgrgplt, iyt);
						}
					}
#else //!UseSparseArray
					for (iyt = 0; iyt < wRoomHeight; iyt++)
					{
						plot = ActivatePlotRowatWin(layer->rgrgplt, iyt, winDst);

						for (ixt = 0; ixt < wRoomWidth; ixt++)
						{
							if (plot->TileSet_ID)
							{
								plot->TileSet_ID =
									GetCurrentID(
										(WORD) plot->TileSet_ID,
										id_translator
									);
							}
							if (plot->Tile_ID)
							{
								TileSetType	*ts;
								ts = TILESET_AT_PLOT(plot);

								if ((!ts) || (plot->Tile_ID > ts->TileCount))
								{
									plot->TileSet_ID	= 0;
									plot->Tile_ID		= 0;
								}
							}
							plot++;
						}
						UpdatePlotRow(layer->rgrgplt, iyt);
					}
#endif//!UseSparseArray
#endif // !PLOTARRAY
					layer = Next (layer);
				}
			}
			room->Flags &= INVERSE_W(match_flags);
		}
		room = Next (room);
	}

	ShowState (LastTBar);
	return;

} /* TranslateRooms */


/**************************************************************************
 *
 * RenumberRooms
 *
 * SYNOPSIS
 *		UWORD RenumberRooms (MapType *map)
 *
 * PURPOSE
 *		Renumber all rooms with numbers 1 on up, and return the maximum.
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
 *		The maximum assigned room number.
 *
 * HISTORY
 *		11/05/89 Sunday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
UWORD RenumberRooms (MapType *map)
{
	RoomType	*room;
	UWORD		id = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RenumberRooms";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (map) {
		room = Head (&map->Rooms);
		while (! IsEOList(room)) {
			id += 1;
			room->R_id = id;
			room = Next (room);
		}
	}
	return (id);

} /* RenumberRooms */


/**************************************************************************
 *
 * PromoteRooms
 *
 * SYNOPSIS
 *		void PromoteRooms (MapType *map)
 *
 * PURPOSE
 *		Mark all rooms in the map as valid.
 *
 * USAGE
 *
 *
 * INPUT
 *		map		:	a pointer to a map.
 *
 * EFFECTS
 *		All rooms in the map become valid.
 *
 * RETURN VALUE
 *		NONE
 *
 * HISTORY
 *		11/15/89 Wednesday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void PromoteRooms (MapType *map)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PromoteRooms";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = Head (&map->Rooms);

	while (! IsEOList (room)) {
		if (room->Flags & PREROOM) {
			room->Flags &= INVERSE_W(PREROOM);
		}
		room = Next (room);
	}
	return;

} /* PromoteRooms */



/**************************************************************************
 *
 * CleanRooms
 *
 * SYNOPSIS
 *		void CleanRooms (MapType *map)
 *
 * PURPOSE
 *		To get rid of all rooms in a map that are invalid (as could
 *		happen during an aborted load/import).
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *		All invalid rooms (marked with PREROOM flag) are deleted from the
 *		map.
 *
 * RETURN VALUE
 *		NONE
 *
 * HISTORY
 *		11/27/89 Monday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void CleanRooms (MapType *map)
{
	RoomType *room;
	RoomType *nextroom;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CleanRooms";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = Head (&map->Rooms);

	while (! IsEOList (room)) {
		nextroom = Next(room);
		if (room->Flags & PREROOM) {
			Remove (room);
			DeAllocateRoom (room);
		}
		room = nextroom;
	}
	return;

} /* CleanRooms */


/*********************************************************************
 *
 * AppendorInsertLayersIntoRoom
 *
 * SYNOPSIS
 *		BOOL AppendorInsertLayersIntoRoom(LayerType *playDst, MapType *pmap, short f)
 *
 * PURPOSE
 *		Search <pmap> for the first newly loaded room (those marked
 *		with the PREROOM flag) with same size tiles, and insert
 *		that room into <playDst>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if OOM.
 *
 * HISTORY
 *		01/28/94 Friday (dcc) - fix so it doesn't delete used tilesets
 *
 * SEE ALSO
 *
*/
BOOL AppendorInsertLayersIntoRoom(LayerType *playDst, MapType *pmap, short f)
{
	RoomType *prm, *prmDst;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InsertLayersIntoRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	prm = Head (&pmap->Rooms);
	prmDst = playDst->Parent_Room;

	while (!IsEOList(prm))
	{
		LayerType *play = Head(&prm->Layers);
		RoomType *prmNext = Next(prm);
		if ((prm->Flags & PREROOM) && IsEdit(prm))
		{
			if (playDst->cxTile == play->cxTile &&
			    playDst->cyTile == play->cyTile)
			{
				LayerType *play = Head(&prm->Layers);

				ShowStatus("Inserting layers...");

				if (playDst->ctilx != play->ctilx ||
				    playDst->ctily != play->ctily)
				{
					ShowStatus("Adjusting inserted layer size.");

					if (!ResizeRoom(prm, playDst->ctilx, playDst->ctily))
					{
						CleanRooms(pmap);
						return FALSE;
					}
				}
				if (f & LOAD_LAYER)
					prmDst->FloorLayer = InsertList(playDst, play);
				else
					MergeLists(playDst, play);

				prmDst->LayerCount += prm->LayerCount;

				/* Delete the room we just move the layers out of */

				Remove(prm);
				DeAllocateRoom(prm);
				return TRUE;
			}
		}
		prm = prmNext;
	}
	/* Didn't find any rooms, so delete everything and exit */

	CleanRooms(pmap);
	return TRUE;

} /* AppendorInsertLayersIntoRoom */


/**************************************************************************
 *
 * PromoteTileSets
 *
 * SYNOPSIS
 *		void PromoteTileSets (TileSpaceType	*tilespace)
 *
 * PURPOSE
 *		Mark all tilesets in the map as valid.
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *		All tilesets in the map become valid.
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		11/15/89 Wednesday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void PromoteTileSets (TileSpaceType	*tilespace)
{
	TileSetType *tileset;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PromoteTileSets";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	tileset = Head (&tilespace->TileSets);

	while (! IsEOList (tileset)) {
		if (tileset->Flags & PRETILESET) {
			tileset->Flags &= INVERSE_W(PRETILESET);
		}
		tileset = Next (tileset);
	}
	return;

} /* PromoteTileSets */


/**************************************************************************
 *
 * CleanTileSets
 *
 * SYNOPSIS
 *		void CleanTileSets (TileSpaceType	*tilespace)
 *
 * PURPOSE
 *		To get rid of all tile sets in a map that are invalid (as could
 *		happen during an aborted load/import).
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *		All invalid tile sets (marked with PRETILESET flag) are deleted
 *		from the map.
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
void CleanTileSets (TileSpaceType	*tilespace)
{
	TileSetType *tileset;
	TileSetType *nexttileset;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CleanTileSets";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	tileset = Head (&tilespace->TileSets);

	while (! IsEOList (tileset)) {
		nexttileset = Next(tileset);
		if (tileset->Flags & PRETILESET) {
			Remove (tileset);
#if __INPUTONLY__
#else
			UnLoadTileSet (tileset);
#endif
		}
		tileset = nexttileset;
	}
	return;
} /* CleanTileSets */


/* prtLastMatch - last room matched, used by FindFirstSameRoomName() and
	FindNextSameRoomName(). */

static RoomType *prtLastMatch;


/*********************************************************************
 *
 * FindNextSameRoomName
 *
 * SYNOPSIS
 *		RoomType *FindNextSameRoomName(char *sz)
 *
 * PURPOSE
 *		Find next room with same name as <sz>.
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
RoomType *FindNextSameRoomName(char *sz)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindNextSameRoomName";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	while (! IsEOList (prtLastMatch))
	{
		RoomType *prt = prtLastMatch;

		prtLastMatch = Next (prtLastMatch);

		if (! stricmp (BaseName (sz), BaseName (prt->Name)))
			return prt;
	}
	return NULL;
} /* FindNextSameRoomName */


/*********************************************************************
 *
 * FindFirstSameRoomName
 *
 * SYNOPSIS
 *		RoomType *FindFirstSameRoomName(char *sz, MapType *pmt)
 *
 * PURPOSE
 *		Find first room in <pmt> with same name as <sz>. Set local
 *		variables so subsequent calls to FindNextSameRoomName()
 *		will find subsequent matches.
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
RoomType *FindFirstSameRoomName(char *sz, MapType *pmt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindFirstSameRoomName";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	prtLastMatch = Head (&pmt->Rooms);
	return FindNextSameRoomName(sz);

} /* FindFirstSameRoomName */


/*********************************************************************
 *
 * FindDuplicateRoomName
 *
 * SYNOPSIS
 *		RoomType *FindDuplicateRoomName(RoomType *prt, MapType *pmt)
 *
 * PURPOSE
 *		A composite room may not have the same name as another room.
 *		Non-composite rooms may have the same name as another room,
 *		as long as the other room is not a composite room. Thus,
 *
 *		If prt is a composite room, then
 *		{
 *			See if prt->Name appears in <pmt>, and is not the
 *			same occurrence as <prt>.
 *			If so, return <prt2> for second match.
 *		}
 *		else
 *		{
 *			See if prt->Name appears in <pmt>, and is not the
 *			same occurrence as <prt>, and is a composite room.
 *			If so, return <prt2> for second match.
 *		}
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns pointer to matching room, or NULL if not found.
 *
 * HISTORY
 *		03/11/92 Wednesday (dcc) - created.
 *		04/11/92 Saturday (dcc) - rules about composite rooms added.
 *
 * SEE ALSO
 *
*/
RoomType *FindDuplicateRoomName(RoomType *prt, MapType *pmt)
{

	RoomType *prt2;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindDuplicateRoomName";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsComposite(prt))
	{
		if ((prt2 = FindFirstSameRoomName(prt->Name, pmt)) != NULL)
		{
			if (prt != prt2)
				return prt2;

			while ((prt2 = FindNextSameRoomName(prt->Name)) != NULL)
			{
				if (prt != prt2)
					return prt2;
			}
		}
	}
	else
	{
		if ((prt2 = FindFirstSameRoomName(prt->Name, pmt)) != NULL)
		{
			if (prt != prt2 && IsComposite(prt2))
				return prt2;

			while ((prt2 = FindNextSameRoomName(prt->Name)) != NULL)
			{
				if (prt != prt2 && IsComposite(prt2))
					return prt2;
			}
		}
	}
	return NULL;

} /* FindDuplicateRoomName */


#if 0
/**************************************************************************
 *
 * FindTile
 *
 * SYNOPSIS
 *		TileType *FindTile (
 *			WORD	tileset_id,
 *			WORD	tilenumber
 *		)
 *
 * PURPOSE
 *		To get a tile pointer for a given tile id & tileset id.
 *
 *		For faster access with known valid IDs :
 *
 *			#define FAST_TILE_PTR(set_id, tile_id) \
 *				GlobalTileSpace->TileSetArray[set_id]->TilePtr[tile_id]
 *
 *
 * USAGE
 *		FOUND_TILE = FindTile (TILESET_ID, TILE_NUMBER);
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
 *		10/17/89 Tuesday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
TileType *FindTile (
	WORD	tileset_id,
	WORD	tilenumber
)
{
	TileSetType		*tileset;
	TileType		*foundtile;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	foundtile	= NULL;

	if (tileset_id && tilenumber) {
		tileset = GlobalTileSpace->TileSetArray[tileset_id];
		if (tileset) {
			if (tileset->TileCount >= tilenumber){
				foundtile = tileset->TilePtr[tilenumber];
			}
		}
	}
	return (foundtile);

} /* FindTile */
#endif


/**************************************************************************
 *
 * GetNextTilesetID
 *
 * SYNOPSIS
 *		WORD	GetNextTilesetID (TileSpaceType	*tilespace)
 *
 * PURPOSE
 *		To get the lowest available tileset id #.
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
 *		A tileset ID #, or 0 if no can find.
 *
 * HISTORY
 *	01/03/91 Thursday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
WORD	GetNextTilesetID (TileSpaceType	*tilespace)
{

	WORD	index;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetNextTilesetID";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (index = 1; index < MAX_TILESETS; index ++) {
		if (! tilespace->TileSetArray[index]) {
			return index;
		}
	}

	return 0;

} /* GetNextTilesetID */


/**************************************************************************
 *
 * GetHighestTilesetID
 *
 * SYNOPSIS
 *		WORD	GetHighestTilesetID (TileSpaceType	*tilespace)
 *
 * PURPOSE
 *		Find the highest actual tileset id number.
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
 *	01/10/91 Thursday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
WORD GetHighestTilesetID (TileSpaceType	*tilespace)
{

	TileSetType	*tileset;
	WORD		max_id	= 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetHighestTilesetID";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	tileset	= Head (&tilespace->TileSets);

	while (! IsEOList (tileset)) {
		max_id	= max (max_id, tileset->TS_id);
		tileset	= Next (tileset);
	}

	return (max_id);

} /* GetHighestTilesetID */

