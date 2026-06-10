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
 * HISTOGRM.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 06/19/93
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
 *		Collect tile usage statistics and display them.
 *
 * ASSUMES
 *		These routines definately assume that tileset == 0 <==> NULL tileset,
 *		and tile_ID == 0 is not used.
 *
 *		should tct in Zero be changed to ptct
 *		can ptct be changed in Localize & Distance
 *
 * HISTORY
 *		06/19/93 (dcc) - created.
 *		08/19/94 (dcc) - add support for UseSparseArray
 *		
*/

#include <echidna/platform.h>
#include "switches.h"

#include	"switch1.h"							// only for PixelLayers
//#include "switch1.h"						// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>


#include <echidna/xtramem.h>
#include <stdlib.h>

#include "events.h"
#include "tuglbl.h"
#include "rectplot.h"
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray
#include "tumedraw.h"
#include "histogrm.h"						/* Verify function prototypes. */

/**************************** C O N S T A N T S ***************************/

/* define size of small digit in pixels */

#define cxDigit 5
#define cyDigit 5

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

static RoomType *prmLast = NULL;			/* Last room that was Localized */

static UWORD cntMaxin = 32767U;			/* Largest number to display */
static int cntDigitsMaxin = 5;			/* don't display more than this N digits */
													/* (the number of digits in cntMaxin) */

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


//for each room, we add a pointer to an array of pointers to tileset HISTOGRAMS.
//This array 


/*********************************************************************
 *
 * ZeroTilesetCount
 *
 * PURPOSE
 *		Zero out tileset count.
 *
 * INPUT
 *		tct			: pointer to pointer to array of tile counts to zero out
 *		cnt			: size of array (number of UWORDs)
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		06/23/93 Wednesday (dcc) - created.
 *
*/
static void near ZeroTilesetCount(XTRAPntr tct, UWORD cnt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ZeroTilesetCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (tct)
	{
		UWORD i;
		UWORD *puw;

		puw = (UWORD *) ActivateXTRA(tct);

		for (i = 0; i < cnt; i++)
			puw[i] = 0;

		UpdateXTRA(tct);
	}
} /* ZeroTilesetCount */


/*********************************************************************
 *
 * ZeroAllTilesetCounts
 *
 * PURPOSE
 *		Zero out all tileset counts contained in <prgtct>.
 *
 * INPUT
 *		prgtct		: pointer to array of pointers to array of tile counts from an edit room
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		06/22/93 Tuesday (dcc) - created.
 *
*/
static void near ZeroAllTilesetCounts(XTRAPntr *prgtct)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ZeroAllTilesetCounts";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prgtct)
	{
		short i;

		for (i = 1; i <= GlobalTileSpace->HighestID; i++)
		{
			ZeroTilesetCount(prgtct[i-1], FAST_TILESET_PTR(i)->TileCount);
		}
	}
} /* ZeroAllTilesetCounts */


/*********************************************************************
 *
 * AllocateTilesetCount
 *
 * PURPOSE
 *		Allocate a tile count array sufficient to store <cnt> values.
 *		This routine tries to allocate in main memory first (this is
 *		the same as having called LocalizedTilesetCount()).
 *
 *		The allocate array is set to zeroes.
 *
 * INPUT
 *		cnt			: number of tiles to count
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns NULL if OOM.
 *
 * HISTORY
 *		06/23/93 Wednesday (dcc) - created.
 *
*/
static XTRAPntr near AllocateTilesetCount(UWORD cnt)
{
	XTRAPntr pxtr;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AllocateTilesetCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((pxtr = AllocXTRAofXMT(cnt * sizeof(UWORD), xmtSTD)) == NULL)
		if ((pxtr = AllocXTRA(cnt * sizeof(UWORD))) == NULL)
			return NULL;

	ZeroTilesetCount(pxtr, cnt);

	return pxtr;
} /* AllocateTilesetCount */


#if __MSDOS16__
/*********************************************************************
 *
 * LocalizeTilesetCount
 *
 * PURPOSE
 *		Make <ptct> appear in main memory.
 *
 *		This routine is not guaranteed to place <ptct> in main memory.
 *		You should check it yourself if needed with
 *			if (XmtFromPxtr(pxtr) != xmtSTD) ...
 *
 * INPUT
 *		ptct			: pointer to tileset counts to localize
 *		cnt			: size of ptct (number of UWORDs)
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		06/23/93 Wednesday (dcc) - created.
 *
*/
static void near LocalizeTilesetCount(XTRAPntr *ptct, UWORD cnt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LocalizeTilesetCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (*ptct && XmtFromPxtr(*ptct) != xmtSTD)
	{
		XTRAPntr pxtr;

		if ((pxtr = AllocXTRAofXMT(cnt * sizeof(UWORD), xmtSTD)) != NULL)
		{
			UWORD *puw = ActivateXTRA(pxtr);

			ReadXTRA(*ptct, 0, cnt*sizeof(UWORD), puw);

//			UpdateXTRA(pxtr);						not needed because it must be xmtSTD

			FreeXTRA(*ptct);
			*ptct = pxtr;
		}
	}
} /* LocalizeTilesetCount */


/*********************************************************************
 *
 * LocalizeAllTilesetCounts
 *
 * PURPOSE
 *		Localize all tileset counts contained in <prgtct>.
 *
 * INPUT
 *		prgtct		: pointer to array of pointers to array of tile counts from an edit room
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		06/23/93 Wednesday (dcc) - created.
 *
*/
static void near LocalizeAllTilesetCounts(XTRAPntr *prgtct)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LocalizeAllTilesetCounts";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prgtct)
	{
		short i;

		for (i = 1; i <= GlobalTileSpace->HighestID; i++)
		{
			LocalizeTilesetCount(&prgtct[i-1], FAST_TILESET_PTR(i)->TileCount);
		}
	}
} /* LocalizeAllTilesetCounts */


/*********************************************************************
 *
 * DistanceTilesetCount
 *
 * PURPOSE
 *		Make <ptct> return to XTRA memory.
 *
 *		This routine is not guaranteed to place <ptct> in main memory.
 *		You should check it yourself if needed with
 *			if (XmtFromPxtr(pxtr) != xmtSTD) ...
 *
 * INPUT
 *		ptct			: pointer to tileset counts to distance
 *		cnt			: size of ptct (number of UWORDs)
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		06/23/93 Wednesday (dcc) - created.
 *
*/
static void near DistanceTilesetCount(XTRAPntr *ptct, UWORD cnt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DistanceTilesetCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (*ptct && XmtFromPxtr(*ptct) == xmtSTD)
	{
		XTRAPntr pxtr;

		if ((pxtr = AllocXTRA(cnt * sizeof(UWORD))) != NULL)
		{
			UWORD *puw = ActivateXTRA(*ptct);

			WriteXTRA(puw, cnt*sizeof(UWORD), pxtr, 0);

//			ReleaseXTRA(*ptct);				not needed because it must be xmtSTD

			FreeXTRA(*ptct);
			*ptct = pxtr;
		}
	}
} /* DistanceTilesetCount */


/*********************************************************************
 *
 * DistanceAllTilesetCounts
 *
 * PURPOSE
 *		Distance all tileset counts contained in <prgtct>.
 *
 * INPUT
 *		prgtct		: pointer to array of pointers to array of tile counts from an edit room
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		06/23/93 Wednesday (dcc) - created.
 *
*/
static void near DistanceAllTilesetCounts(XTRAPntr *prgtct)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DistanceAllTilesetCounts";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prgtct)
	{
		short i;

		for (i = 1; i <= GlobalTileSpace->HighestID; i++)
		{
			DistanceTilesetCount(&prgtct[i-1], FAST_TILESET_PTR(i)->TileCount);
		}
	}
} /* DistanceAllTilesetCounts */
#endif /*__MSDOS16__*/


/*********************************************************************
 *
 * FreeAllTilesetCounts
 *
 * PURPOSE
 *		Free all tileset counts contained in an edit room's <prgtct>.
 *
 * INPUT
 *		pprgtct		: pointer to pointer to array of tile counts from an edit room
 *
 * ASSUMES
 *		Tilesets pointed to by <pprgtct> haven't been deleted yet.
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		06/23/93 Wednesday (dcc) - created.
 *		03/30/94 Wednesday (dcc) - only invalidate tileset if room contains
 *											counts for that particular tileset.
 *
*/
static void near FreeAllTilesetCounts(XTRAPntr **pprgtct)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FreeAllTilesetCounts";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (*pprgtct)
	{
		short i;

		for (i = 1; i <= GlobalTileSpace->HighestID; i++)
		{
			if ((*pprgtct)[i-1])
			{
				FAST_TILESET_PTR(i)->fcntValid = FALSE;
				FreeXTRA((*pprgtct)[i-1]);
			}
		}
		MEM_free(*pprgtct);
		*pprgtct = NULL;
	}
} /* FreeAllTilesetCounts */


/*********************************************************************
 *
 * InvalidateEditRoomTileCounts
 *
 * PURPOSE
 *		Mark an edit rooms count as no longer being current.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		Call this after you make major changes to a room (but you
 *		don't delete the room), such as after deleting a layer.
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		07/12/93 Monday (dcc) - created.
 *
*/
void InvalidateEditRoomTileCounts(XTRAPntr **pprgtct, BOOL *pfcntValid)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InvalidateEditRoomTileCounts";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	FreeAllTilesetCounts(pprgtct);
	*pfcntValid = FALSE;
	return;

} /* InvalidateEditRoomTileCounts */


/*********************************************************************
 *
 * FreeTileCounts
 *
 * PURPOSE
 *		Free all tile counts for edit room <prm>.
 *
 * INPUT
 *		prm			: edit room to free
 *
 * ASSUMES
 *		This function should be called before you delete <prm>.
 *		Tilesets pointed to by <prm> haven't been deleted yet.
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		06/23/93 Wednesday (dcc) - created.
 *
*/
void FreeTileCounts(RoomType *prm)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FreeTileCounts";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prm)
	{
		InvalidateEditRoomTileCounts(&prm->prgtct, &prm->fcntValid);
		if (prm == prmLast)
			prmLast = NULL;
	}
} /* FreeTileCounts */


/*********************************************************************
 *
 * RevampTileCounts
 *
 * PURPOSE
 *		Flushes ALL tilecount data structures from all edit rooms
 *		and all tilesets (Ugh).
 *
 * INPUT
 *		ptsp			: tilespace to delete data from
 *
 * ASSUMES
 *		User has added or deleted a tileset.
 *		ptsp is not NULL.
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		06/23/93 Wednesday (dcc) - created.
 *
*/
void RevampTileCounts(TileSpaceType *ptsp)
{
	RoomType *prm;
	TileSetType *ptst;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RevampTileCounts";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalMap)
	{
		prm = Head(&GlobalMap->Rooms);
		while (!IsEOList(prm))
		{
			if (IsEdit(prm) || IsComposite(prm))
				FreeTileCounts(prm);
			prm = Next(prm);
		}
	}

	ptst = Head(&ptsp->TileSets);
	if (ptst)
	{
		while (!IsEOList(ptst))			/* check all tilesets */
		{
			if (ptst->ptct)
				FreeXTRA(ptst->ptct);
			ptst->ptct = NULL;
			ptst->fcntValid = FALSE;

			ptst = Next(ptst);
		}
	}
} /* RevampTileCounts */


/*********************************************************************
 *
 * AddTilesInSubRect
 *
 * PURPOSE
 *		Count the tiles in all layers <plys> of the sub-rect defined by
 *		<dtilx>, <dtily>, <ctilx>, <ctily>, and add to the data structure
 *		<prgtct> (or subtract if <prgtct> == -1).
 *
 *		If global <fShowTileUsage> is FALSE, then merely mark room as
 *		no longer containing a valid count (set *<pfcntValid> = FALSE).
 *
 * INPUT
 *		plys			: layers with tiles to add/subtract
 *		dtilx			: x-offset to start adding/subtracting
 *		dtily			: y-offset to start adding/subtracting
 *		ctilx			: tiles wide to add/subtract
 *		ctily			: tiles high to add/subtract
 *		dir			: 1 == add, -1 == subtract
 *		pprgtct		: pointer to pointer to tileset count array to add counts to
 *		fcntValid	: pointer to flag to set FALSE if <fShowTileUsage> is FALSE
 *
 *	ASSUMES
 *		<plys> is a valid layer list header
 *
 * RETURN VALUE
 *		False if OOM.
 *
 * HISTORY
 *		06/23/93 (dcc) - created.
 *		08/19/94 (dcc) - add support for UseSparseArray
 *		09/08/94 (dcc) - KLUDGE BUGBUG FIXME PixelLayers-don't process 1x1 layers
 *		09/08/94 (dcc) - don't total if no tiles stamped into layer yet
 *
*/
BOOL AddTilesInSubRect(ListType *plys, short dtilx, short dtily,
								short ctilx, short ctily, short dir,
								XTRAPntr **pprgtct, BOOL *pfcntValid)
{
	LayerType *play = Head(plys);

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddTilesInSubRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!fShowTileUsage)
	{
		InvalidateEditRoomTileCounts(pprgtct, pfcntValid);
		return TRUE;
	}

	/* If editing a composite room, then just mark composite tileset
		as no longer valid and return. */

	if (IsComposite(play->Parent_Room))
	{
		InvalidateEditRoomTileCounts(pprgtct, pfcntValid);
		play->Parent_Room->ptsComposite->fcntValid = FALSE;
		return FALSE;
	}

	if (!*pprgtct)
		if ((*pprgtct = MEM_calloc(GlobalTileSpace->HighestID, sizeof(XTRAPntr))) == NULL)
			return FALSE;

#if __MSDOS16__
	LocalizeAllTilesetCounts(*pprgtct);
#endif /*__MSDOS16__*/

	while (!IsEOList(play))
	{
		int itilx, itily;

#if PixelLayers
		if (play->cxTile > 1 && play->cyTile > 1)	// KLUDGE BUGBUG FIXME
#else // !PixelLayers
		if (play->cxTile != 0 && play->cyTile != 0)	// tiles stamped into layer?
#endif // !PixelLayers
		{
			for (itily = dtily; itily < dtily+ctily; itily++)
			{
				PlotType *pplt;

#if UseSparseArray
				if (!IsSparse(play))
					pplt = ActivatePlotXYatWin(play->p.rgrgplt, dtilx, itily, winDst);
#else // !UseSparseArray
				pplt = ActivatePlotXYatWin(play->rgrgplt, dtilx, itily, winDst);
#endif // !UseSparseArray

				for (itilx = 0; itilx < ctilx; itilx++)
				{
					UBYTE tst;
					UWORD uw;

#if UseSparseArray
					if (IsSparse(play))
						pplt = GetSparsePlotXY(play->p.pspa, itilx, itily);

#endif // UseSparseArray
					tst = pplt->TileSet_ID;
					if (tst)
					{
						UWORD til = pplt->Tile_ID;

						if (!(*pprgtct)[tst-1])
							if (((*pprgtct)[tst-1] = AllocateTilesetCount(FAST_TILESET_PTR(tst)->TileCount)) == NULL)
							{
#if __MSDOS16__
								DistanceAllTilesetCounts(*pprgtct);
#endif /*__MSDOS16__*/
								return FALSE;
							}

						FAST_TILESET_PTR(tst)->fcntValid = FALSE;
						ReadXTRA((*pprgtct)[tst-1], (til-1)*sizeof(UWORD), sizeof(UWORD), &uw);
						uw += dir;
						WriteXTRA(&uw, sizeof(UWORD), (*pprgtct)[tst-1], (til-1)*sizeof(UWORD));
					}
					pplt++;
				}
#if UseSparseArray
				if (!IsSparse(play))
					ReleasePlotRow(play->p.rgrgplt, itily);
#else // !UseSparseArray
				ReleasePlotRow(play->rgrgplt, itily);
#endif // !UseSparseArray
			}
		}
		play = Next(play);
	}
#if __MSDOS16__
	DistanceAllTilesetCounts(*pprgtct);
#endif /*__MSDOS16__*/
	return TRUE;
} /* AddTilesInSubRect */


/*********************************************************************
 *
 * TotalTilesetsInEditRoom
 *
 * PURPOSE
 *		For edit room <prm>, update its data structure which shows
 *		how often a particular tile of a particular tileset is used.
 *
 * INPUT
 *		prm			: edit room to count tiles
 *
 * RETURN VALUE
 *		Returns FALSE if OOM.
 *
 * HISTORY
 *		06/22/93 Tuesday (dcc) - created.
 *		09/08/94 (dcc) - PixelSelect: scale values for AddTilesInSubRect()
 *
*/
static BOOL near TotalTilesetsInEditRoom(RoomType *prm)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TotalTilesetsInEditRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsEdit(prm) && !prm->fcntValid)
	{
#if PixelLayers
		LayerType *play = FindBiggestLayer(&prm->Layers);

		if (!play)
			play = prm->FloorLayer;
#else // !PixelLayers
		LayerType *play = prm->FloorLayer;
#endif // !PixelLayers

		ZeroAllTilesetCounts(prm->prgtct);

		if (!AddTilesInSubRect(&prm->Layers, 0, 0,
									play->ctilx, play->ctily,
									1, &prm->prgtct, &prm->fcntValid))
			return FALSE;

		prm->fcntValid = TRUE;
	}
	return TRUE;
} /* TotalTilesetsInEditRoom */


/*********************************************************************
 *
 * TotalTilesetCountsFromRooms
 *
 * PURPOSE
 *		For tileset <ptst>, count how often each tile is used by
 *		summing the each room's tileset usage.
 *
 * INPUT
 *		ptst			: tileset to total
 *
 * RETURN VALUE
 *		Returns FALSE if OOM.
 *
 * HISTORY
 *		06/22/93 Tuesday (dcc) - created.
 *
*/
static BOOL near TotalTilesetCountsFromRooms(TileSetType *ptst)
{
	UWORD *pcnts;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TotalTilesetCountsFromRooms";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* If counts are no longer valid, then recompute by totalling tileset
		counts from every room. */

	if (!ptst->fcntValid)
	{
		RoomType *prm;
		short idx;

		/* Make sure tileset counts in all rooms are valid. */

		prm = Head(&GlobalMap->Rooms);
		while (!IsEOList(prm))
		{
			if (!TotalTilesetsInEditRoom(prm))
				return FALSE;
			prm = Next(prm);
		}

		/* Now sum tileset counts by adding in tileset counts from all rooms */

		idx = ptst->TS_id-1;

		if (ptst->ptct)
		{
#if __MSDOS16__
			LocalizeTilesetCount(&ptst->ptct, ptst->TileCount);
#endif /*__MSDOS16__*/
			ZeroTilesetCount(ptst->ptct, ptst->TileCount);
		}
		else
		{
			if ((ptst->ptct = AllocateTilesetCount(ptst->TileCount)) == NULL)
				return FALSE;
		}

#if __MSDOS16__
		if (XmtFromPxtr(ptst->ptct) != xmtSTD)
			return FALSE;
#endif /*__MSDOS16__*/

		pcnts = ActivateXTRA(ptst->ptct);

		prm = Head(&GlobalMap->Rooms);

		while (!IsEOList(prm))
		{
			if ((IsEdit(prm) || IsComposite(prm)) && prm->prgtct)
			{
				XTRAPntr ptct;
				UWORD i;
				UWORD *pdst;
				UWORD *psrc;

				ptct = prm->prgtct[idx];

				if (ptct)						/* tiles in room belonging to this TST */
				{
					psrc = pcnts;
					pdst = ActivateXTRA(ptct);

					/* Total all tile counts from room */

					for (i = 0; i < ptst->TileCount; i++)
					{
						*psrc += *pdst;
						psrc++;	pdst++;
					}
					ReleaseXTRA(ptct);
				}
			}
			prm = Next(prm);
		}
//		ReleaseXTRA(ptst->ptct);			not needed because it must be xmtSTD
		ptst->fcntValid = TRUE;
	}
	return TRUE;
} /* TotalTilesetCountsFromRooms */


/*********************************************************************
 *
 * TotalTilesetsInSourceRoom
 *
 * PURPOSE
 *		For a source room <prm>, total counts for all tileset displayed
 *		in that room.
 *
 * INPUT
 *		prm			: source room to total
 *
 * RETURN VALUE
 *		Returns FALSE if OOM.
 *
 * HISTORY
 *		06/22/93 Tuesday (dcc) - created.
 *
*/
static BOOL near TotalTilesetsInSourceRoom(RoomType *prm)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TotalTilesetsInSourceRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prm)
	{
		TileSetType *ptst = Head(&GlobalTileSpace->TileSets);

		if (ptst)
		{
			while (!IsEOList(ptst))			/* check all tilesets */
			{
				if (ptst->SourceRoom == prm)	/* tileset being display in <prm> */
					if (!TotalTilesetCountsFromRooms(ptst))
						return FALSE;
				ptst = Next(ptst);
			}
		}
	}
	return TRUE;
} /* TotalTilesetsInSourceRoom */


/*********************************************************************
 *
 * TotalTilesetsInCompositeRoom
 *
 * PURPOSE
 *		For composite room <prm>, update its data structure which
 *		shows how often a particular tile of a particular tileset
 *		is used.
 *
 * INPUT
 *		prm			: composite room to count tiles
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns FALSE if OOM.
 *
 * HISTORY
 *		03/26/94 (dcc) - created.
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
*/
static BOOL near TotalTilesetsInCompositeRoom(RoomType *prm)
{
	UWORD *pcnts;
	TileSetType *ptst = prm->ptsComposite;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TotalTilesetsInCompositeRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsComposite(prm) && !ptst->fcntValid)
	{
		int itilx, itily;
		short icmpx, imodx, icmpy, imody, ccmpx;
		LayerType *play = prm->FloorLayer;

		if (!TotalTilesetsInSourceRoom(prm))
			return FALSE;

		if (IsLocked(prm))
		{
			SetUnlockedCompositeRoom(prm);
		}
		InvalidateEditRoomTileCounts(&prm->prgtct, &prm->fcntValid);

		if ((prm->prgtct = MEM_calloc(GlobalTileSpace->HighestID, sizeof(XTRAPntr))) == NULL)
		{
			if (IsLocked(prm))
				SetLockedCompositeRoom(prm);

			return FALSE;
		}

#if __MSDOS16__
		LocalizeAllTilesetCounts(prm->prgtct);
		LocalizeTilesetCount(&ptst->ptct, ptst->TileCount);
#endif /*__MSDOS16__*/

		pcnts = ActivateXTRA(ptst->ptct);

		ccmpx = play->ctilx / ptst->wTilesAcross;
		icmpy = 0;	imody = 0;
		for (itily = 0; itily < prm->FloorLayer->ctily; itily++)
		{
			PlotType *pplt;

#if UseSparseArray
			if (!IsSparse(play))
				pplt = ActivatePlotRowatWin(play->p.rgrgplt, itily, winDst);
#else // !UseSparseArray
			pplt = ActivatePlotRowatWin(play->rgrgplt, itily, winDst);
#endif // !UseSparseArray

			icmpx = 0;	imodx = 0;
			for (itilx = 0; itilx < play->ctilx; itilx++)
			{
				UBYTE tst;
				UWORD uw;

#if UseSparseArray
				if (IsSparse(play))
					pplt = GetSparsePlotXY(play->p.pspa, itilx, itily);

#endif // UseSparseArray
				tst = pplt->TileSet_ID;
				if (tst)
				{
					UWORD til = pplt->Tile_ID;

					if (!prm->prgtct[tst-1])
						if ((prm->prgtct[tst-1] = AllocateTilesetCount(FAST_TILESET_PTR(tst)->TileCount)) == NULL)
						{
			#if __MSDOS16__
							DistanceAllTilesetCounts(prm->prgtct);
			#endif /*__MSDOS16__*/
							if (IsLocked(prm))
								SetLockedCompositeRoom(prm);

							return FALSE;
						}

					FAST_TILESET_PTR(tst)->fcntValid = FALSE;
					ReadXTRA(prm->prgtct[tst-1], (til-1)*sizeof(UWORD), sizeof(UWORD), &uw);
					uw += pcnts[icmpy * ccmpx + icmpx];
					WriteXTRA(&uw, sizeof(UWORD), prm->prgtct[tst-1], (til-1)*sizeof(UWORD));
				}
				pplt++;	//BUGBUG: this should not be done if layer is a
							//sparse array, but it should be harmless
				imodx++;
				if (imodx >= ptst->wTilesAcross)
				{
					icmpx++;	imodx = 0;
				}
			}
#if UseSparseArray
			if (!IsSparse(play))
				ReleasePlotRow(play->p.rgrgplt, itily);
#else // !UseSparseArray
			ReleasePlotRow(play->rgrgplt, itily);
#endif // !UseSparseArray
			imody++;
			if (imody >= ptst->wTilesDown)
			{
				icmpy++;	imody = 0;
			}
		}
#if __MSDOS16__
		DistanceAllTilesetCounts(prm->prgtct);
		DistanceTilesetCount(&ptst->ptct, ptst->TileCount);
#endif /*__MSDOS16__*/

		if (IsLocked(prm))
		{
			SetLockedCompositeRoom(prm);
		}
		prm->fcntValid = TRUE;
	}
	return TRUE;
} /* TotalTilesetsInCompositeRoom */


/*********************************************************************
 *
 * ValidateAllCompositeRooms
 *
 * PURPOSE
 *		If there are composite rooms loaded, then we need to make
 *		sure that they are up to date before we show the tile counts
 *		for a composite room or source room.
 *
 * INPUT
 *		NONE.
 *
 * ASSUMES
 *		That there aren't any circular-referenced composite rooms.
 *
 * RETURN VALUE
 *		Returns FALSE if OOM.
 *
 * HISTORY
 *		03/30/94 Wednesday (dcc) - created.
 *
*/
static BOOL near ValidateAllCompositeRooms(void)
{
	BOOL fAllValid;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ValidateAllCompositeRooms";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	do
	{
		TileSetType *ptst = Head(&GlobalTileSpace->TileSets);

		fAllValid = TRUE;

		if (ptst)
		{
			while (!IsEOList(ptst))			/* check all tilesets */
			{
				if (IsTSTComposite(ptst))	/* only check composite tilesets */
				{
					if (!ptst->fcntValid)	/* count is not valid? */
					{
						fAllValid = FALSE;
						if (!TotalTilesetsInCompositeRoom(ptst->SourceRoom))
							return FALSE;
					}
				}
				ptst = Next(ptst);
			}
		}
	} while (!fAllValid);

	return TRUE;

} /* ValidateAllCompositeRooms */


/*********************************************************************
 *
 * UpdateTileCountsForRoom
 *
 * PURPOSE
 *		This routine will update the tile counts so that the current
 *		room can be displayed properly.
 *
 * INPUT
 *		prm			: room to total
 *
 * RETURN VALUE
 *		Returns FALSE if OOM.
 *
 * HISTORY
 *		06/22/93 Tuesday (dcc) - created.
 *
*/
static BOOL near UpdateTileCountsForRoom(RoomType *prm)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateTileCountsForRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prmLast)
	{
#if __MSDOS16__
		DistanceAllTilesetCounts(prm->prgtct);
#endif /*__MSDOS16__*/
		prmLast = NULL;
	}

	if (IsEdit(prm))
	{
		return TotalTilesetsInEditRoom(prm);
	}
	if (IsComposite(prm))
	{
		if (!ValidateAllCompositeRooms())
			return FALSE;

		return TotalTilesetsInCompositeRoom(prm);
	}
	else
	{
		if (!ValidateAllCompositeRooms())
			return FALSE;

		return TotalTilesetsInSourceRoom(prm);
	}
} /* UpdateTileCountsForRoom */


/*********************************************************************
 *
 * CNTTileFromEditRoom
 *
 * PURPOSE
 *		Returns how often the tile <tst>, <tile> appears in an edit room.
 *
 * INPUT
 *		prgtct		: pointer to array of TCT for edit room
 *		tst			: tileset
 *		tile			: tile_ID
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		06/22/93 Tuesday (dcc) - created.
 *
*/
static UWORD near CNTTileFromEditRoom(XTRAPntr *prgtct, UBYTE tst, UWORD tile)
{
	UWORD w = 65535U;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CNTTileFromEditRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prgtct)
	{
		XTRAPntr ptct;

		ptct = prgtct[tst-1];

		ReadXTRA(ptct, (tile-1)*sizeof(UWORD), sizeof(UWORD), &w);
	}
	return w;
} /* CNTTileFromEditRoom */


/*********************************************************************
 *
 * CNTTileFromSourceRoom
 *
 * PURPOSE
 *		Returns how often the tile <tst>, <tile> appears in a source room.
 *
 * INPUT
 *		tst			: tileset
 *		tile			: tile_ID
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		06/22/93 Tuesday (dcc) - created.
 *
*/
static UWORD near CNTTileFromSourceRoom(UBYTE tst, UWORD tile)
{
	UWORD w = 65535U;
	TileSetType *ptst = FAST_TILESET_PTR(tst);

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CNTTileFromSourceRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ReadXTRA(ptst->ptct, (tile-1)*sizeof(UWORD), sizeof(UWORD), &w);
	return w;
} /* CNTTileFromSourceRoom */


/*********************************************************************
 *
 * StampCount
 *
 * PURPOSE
 *		Draw the tile count on-screen.
 *
 * INPUT
 *		cnt			: number to draw
 *		dx				: x-offset within bytemap to draw count
 *		dy				: y-offset within bytemap to draw count
 *		cxtilDst		: pixel width of a single tile
 *		cytilDst		: pixel height of a single tile
 *		cntDigits	: number of digits to draw
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		07/10/93 Saturday (dcc) - created.
 *
*/
static void near StampCount(UWORD cnt, short dx, short dy,
									short cxtilDst, short cytilDst, short cntDigits)
{
	short dxDraw, dyDraw;
	char szCnt[10];
	char szShow[10];
	short len;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "StampCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (cnt > cntMaxin)
		return;									/* number too large, don't show */

	if (cntDigits > cntDigitsMaxin)
		cntDigits = cntDigitsMaxin;

	dxDraw = dx + (cxtilDst - cntDigits * (cxDigit+1)) / 2;
	dyDraw = dy + (cytilDst - (cxDigit+1)) / 2;

	sprintf(szCnt, "%u", cnt);
	len = strlen(szCnt);
	if (len <= cntDigits)
	{
		int i;

		for (i = 0 ; i < (cntDigits - len); i ++)
			szShow[i] = '0';
		strcpy(szShow+(cntDigits-len), szCnt);
	}
	else
	{
		int i;

		for (i = 0 ; i < cntDigits; i ++)
			szShow[i] = '+';
		szShow[cntDigits] = 0;
	}
	BeforeGraphics();
	SetPenColor(255);
	DrawStringN (&SmallFont, dxDraw, dyDraw, szShow, cntDigits);
	AfterGraphics();

} /* StampCount */


/*********************************************************************
 *
 * ShowCountsInSubRect
 *
 * PURPOSE
 *		Show the tile usage counts for the floor layer in room <prm>.
 *
 *		If global <fShowTileUsage> is FALSE, then do nothing.
 *
 * INPUT
 *		prm			: room with floor layer to show tile counts
 *		roomwindow	:
 *		dx				: x-offset of left edge within bytemap to draw first count
 *		dy				: y-offset of top edge within bytemap to draw first count
 *		cx				: pixel width in bytemap to draw counts
 *		cy				: pixel height in bytemap to draw counts
 *		dtilDstX		: x tile-offset on screen (from dx) to start drawing
 *		dtilDstY		: y tile-offset on screen (from dy) to start drawing
 *		dtilx			: left x tile-offset within floor layer to draw counts
 *		dtily			: top y tile-offset within floor layer to draw counts
 *		ctilx			: number of tiles wide to draw
 *		ctily			: number of tiles high to draw
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		
 *
 * HISTORY
 *		06/24/93 (dcc) - created.
 *		12/17/93 (dcc) - compute tile size individually for each tile
 *		08/19/94 (dcc) - added calls to ReleasePlotRow()
 *		08/19/94 (dcc) - add support for UseSparseArray
 *		09/08/94 (dcc) - KLUDGE BUGBUG FIXME PixelLayers-don't process 1x1 layers
 *
*/
void ShowCountsInSubRect(RoomType *prm,
								RoomWindowType *roomwindow,
								short dx, short dy, short cx, short cy,
								short dtilDstX, short dtilDstY,
								short dtilx, short dtily, short ctilx, short ctily)
{
	short	sep;

	RoomStuffType	*roomstuff = roomwindow->CurrentRoom;
	LayerType		*play = prm->FloorLayer;

	short				cxtilGrid;	/* width of tile on destination screen */
	short				cytilGrid;	/* height of tile on destination screen */
	short				cntDigits;	/* number of digits that will fit in tile */

	short				itilxmax;	/* number of tiles across to draw */
	short				itilymax;	/* number of tiles down to draw */
	short				ixInit;		/* first x-pixel position to draw */
	short				iyInit;		/* first y-pixel position to draw */
	short				ix;			/* index x-pixel positions */
	short				iy;			/* index y-pixel positions */
	int				itilx;		/* index x-tile positions */
	int				itily;		/* index y-tile positions */
	BOOL				fGridTooSmall = FALSE;	/* too small to display digit? */

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowCountsInSubRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!fShowTileUsage)
		return;

	if (dtilDstX < 0)
	{
		dtilx -= dtilDstX;
		if (ctilx)
		{
			ctilx += dtilDstX;
			if (ctilx <= 0)
				return;							/* nothing to draw! */
		}
		dtilDstX = 0;
	}
	if (dtilDstY < 0)
	{
		dtily -= dtilDstY;
		if (ctily)
		{
			ctily += dtilDstY;
			if (ctily <= 0)
				return;							/* nothing to draw! */
		}
		dtilDstY = 0;
	}

	if (play->cxTile == 0 || play->cyTile == 0)
		return;								/* no tiles in floor layer */

	sep			= 0 + ((roomstuff->Flags) & (DISPLAY_SEPERATED));

	/* Calculate how many pixels each tile will occupy on destination screen */

	cxtilGrid = wSrcSizeToDstSize(play->cxTile, wSrcSkipX, wDstDupX) + sep;
	cytilGrid = wSrcSizeToDstSize(play->cyTile, wSrcSkipY, wDstDupY) + sep;

	if (cxtilGrid < cxDigit || cytilGrid < cyDigit)
		fGridTooSmall = TRUE;				/* too small to display a digit */

	if (ctilx)
		itilxmax = min(play->ctilx, ctilx + dtilx);
	else
		itilxmax = play->ctilx;

	if (ctily)
		itilymax = min(play->ctily, ctily + dtily);
	else
		itilymax = play->ctily;

	ixInit = dtilDstX * cxtilGrid + dx + sep;
	iyInit = dtilDstY * cytilGrid + dy + sep;

	itilxmax = min(itilxmax, (cx / cxtilGrid + 1) + dtilx);
	itilymax = min(itilymax, (cy / cytilGrid + 1) + dtily);

	/* Use one pixel to separate digits, don't need one after final digit */

	cntDigits = cxtilGrid / (cxDigit+1);

	if (!UpdateTileCountsForRoom(prm))
		return;								/* OOM trying to compute counts */

	if (IsEdit(prm))
	{
#if PixelLayers
	 if (play->cxTile != 1 || play->cyTile != 1)	// KLUDGE BUGBUG FIXME
#endif // PixelLayers
		for (itily = dtily, iy = iyInit; itily < itilymax; itily++, iy += cytilGrid)
		{
			PlotType *pplt;

#if UseSparseArray
			if (!IsSparse(play))
				pplt = ActivatePlotXYatWin(play->p.rgrgplt, dtilx, itily, winDst);
#else // !UseSparseArray
			pplt = ActivatePlotXYatWin(play->rgrgplt, dtilx, itily, winDst);
#endif // !UseSparseArray

			for (itilx = dtilx, ix = ixInit; itilx < itilxmax; itilx++, ix += cxtilGrid)
			{
#if UseSparseArray
				if (IsSparse(play))
					pplt = GetSparsePlotXY(play->p.pspa, itilx, itily);

#endif // UseSparseArray
				if (pplt->TileSet_ID)
				{
					int cxTile, cyTile, cDigits;
					BOOL fDontShow = FALSE;
					TileSetType *ptst = FAST_TILESET_PTR(pplt->TileSet_ID);

					if (ptst->Width == ptst->cxDisplay &&
						ptst->Height == ptst->cyDisplay)
					{
						cxTile = cxtilGrid;
						cyTile = cytilGrid;
						cDigits = cntDigits;
						fDontShow = fGridTooSmall;
					}
					else
					{
						cxTile = wSrcSizeToDstSize(ptst->Width,  wSrcSkipX, wDstDupX) + sep;
						cyTile = wSrcSizeToDstSize(ptst->Height, wSrcSkipY, wDstDupY) + sep;
						cDigits = cxTile / (cxDigit+1);
						if (cxTile < cxDigit || cyTile < cyDigit)
							fDontShow = TRUE;		/* too small to display a digit */
					}
					if (!fDontShow)
					{
						UWORD cnt = CNTTileFromEditRoom(prm->prgtct, pplt->TileSet_ID, pplt->Tile_ID);

						StampCount(cnt, ix, iy, cxTile, cyTile, cDigits);
					}
				}
				pplt++;	//BUGBUG: this should not be done if layer is a
							//sparse array, but it should be harmless
			}
#if UseSparseArray
			if (!IsSparse(play))
				ReleasePlotRow(play->p.rgrgplt, itily);
#else // !UseSparseArray
			ReleasePlotRow(play->rgrgplt, itily);
#endif // !UseSparseArray
		}
	}
	else if (IsSource(prm))
	{
		for (itily = dtily, iy = iyInit; itily < itilymax; itily++, iy += cytilGrid)
		{
			PlotType *pplt;

#if UseSparseArray
			if (!IsSparse(play))
				pplt = ActivatePlotXYatWin(play->p.rgrgplt, dtilx, itily, winDst);
#else // !UseSparseArray
			pplt = ActivatePlotXYatWin(play->rgrgplt, dtilx, itily, winDst);
#endif // !UseSparseArray

			for (itilx = dtilx, ix = ixInit; itilx < itilxmax; itilx++, ix += cxtilGrid)
			{
#if UseSparseArray
				if (IsSparse(play))
					pplt = GetSparsePlotXY(play->p.pspa, itilx, itily);

#endif // UseSparseArray
				if (pplt->TileSet_ID)
				{
					int cxTile, cyTile, cDigits;
					BOOL fDontShow = FALSE;
					TileSetType *ptst = FAST_TILESET_PTR(pplt->TileSet_ID);

					if (ptst->Width == ptst->cxDisplay &&
						ptst->Height == ptst->cyDisplay)
					{
						cxTile = cxtilGrid;
						cyTile = cytilGrid;
						cDigits = cntDigits;
						fDontShow = fGridTooSmall;
					}
					else
					{
						cxTile = wSrcSizeToDstSize(ptst->Width,  wSrcSkipX, wDstDupX) + sep;
						cyTile = wSrcSizeToDstSize(ptst->Height, wSrcSkipY, wDstDupY) + sep;
						cDigits = cxTile / (cxDigit+1);
						if (cxTile < cxDigit || cyTile < cyDigit)
							fDontShow = TRUE;		/* too small to display a digit */
					}
					if (!fDontShow)
					{
						UWORD cnt = CNTTileFromSourceRoom(pplt->TileSet_ID, pplt->Tile_ID);

						StampCount(cnt, ix, iy, cxTile, cyTile, cDigits);
					}
				}
				pplt++;	//BUGBUG: this should not be done if layer is a
							//sparse array, but it should be harmless
			}
#if UseSparseArray
			if (!IsSparse(play))
				ReleasePlotRow(play->p.rgrgplt, itily);
#else // !UseSparseArray
			ReleasePlotRow(play->rgrgplt, itily);
#endif // !UseSparseArray
		}
	}
	else if (IsComposite(prm))
	{
		if (IsLocked(prm))
		{
			for (itily = dtily, iy = iyInit; itily < itilymax; itily++, iy += cytilGrid)
			{
				for (itilx = dtilx, ix = ixInit; itilx < itilxmax; itilx++, ix += cxtilGrid)
				{
					UWORD cnt = CNTTileFromSourceRoom(prm->ptsComposite->TS_id,
																itily * play->ctilx + itilx + 1);

					StampCount(cnt, ix, iy, cxtilGrid, cytilGrid, cntDigits);
				}
			}
		}
		else
		{
			TileSetType *ptst = prm->ptsComposite;
			int ccmpx = play->ctilx / ptst->wTilesAcross;

			for (itily = dtily, iy = iyInit; itily < itilymax; itily++, iy += cytilGrid)
			{
				for (itilx = dtilx, ix = ixInit; itilx < itilxmax; itilx++, ix += cxtilGrid)
				{
					UWORD cnt = CNTTileFromSourceRoom(ptst->TS_id,
						(itily / ptst->wTilesDown) * ccmpx + (itilx / ptst->wTilesAcross) + 1);

					StampCount(cnt, ix, iy, cxtilGrid, cytilGrid, cntDigits);
				}
			}
		}
	}

} /* ShowCountsInSubRect */


/*********************************************************************
 *
 * GetLargestTileUsageCount
 *
 * PURPOSE
 *		Return the largest tile usage count that will be displayed.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Largest tile usage count.
 *
 * HISTORY
 *		07/12/93 Monday (dcc) - created.
 *
*/
UWORD GetLargestTileUsageCount(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetLargestTileUsageCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return cntMaxin;

} /* GetLargestTileUsageCount */


/*********************************************************************
 *
 * SetLargestTileUsageCount
 *
 * PURPOSE
 *		Set the largest tile usage count that will be displayed.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		07/12/93 Monday (dcc) - created.
 *
*/
void SetLargestTileUsageCount(UWORD cnt)
{
	char sz[33];

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetLargestTileUsageCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	cntMaxin = cnt;
	cntDigitsMaxin = strlen(ultoa(cnt, sz, 10));

} /* SetLargestTileUsageCount */

