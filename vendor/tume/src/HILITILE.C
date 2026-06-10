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
 * HILITILE.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 05/31/93
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
 *		Highlight tiles in the current brush, one at a time.
 *
 * HISTORY
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include "hilitile.h"						/* Verify function prototypes. */
#include "tuglbl.h"
#include "rectplot.h"
#include "tumedraw.h"
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

/* These three variables track our current position in GlobalBlockCopy */

static LayerType *playHighlight;
static short itilxHighlight;
static short itilyHighlight;

/* These three variables track which source room and the position of the
	highlighted tile within the source room: */

static RoomType *prmHighlight = NULL;
static short itilxRoom;
static short itilyRoom;

static BOOL fShowingTileHighlight = FALSE;	// showing tile highlight?

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * ResetHighlightTile
 *
 * SYNOPSIS
 *		void ResetHighlightTile(void)
 *
 * PURPOSE
 *		Reset variables involved in highlighting tiles in current brush.
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
void ResetHighlightTile(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ResetHighlightTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	playHighlight = NULL;
	itilxHighlight = -1;
	itilyHighlight = 0;
	prmHighlight = NULL;

} /* ResetHighlightTile */


/*********************************************************************
 *
 * HighlightNextTile
 *
 * SYNOPSIS
 *		void HighlightNextTile(void)
 *
 * PURPOSE
 *		Set internal variables to next tile in brush to highlight.
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
void HighlightNextTile(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "HighlightNextTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!playHighlight)
	{
		if (FSelectedPblk(GlobalBlockCopy))
		{
			playHighlight = Head(&GlobalBlockCopy->Layers);
			itilxHighlight = 0;
			itilyHighlight = 0;
		}
		return;
	}
	itilxHighlight++;
	if (itilxHighlight >= playHighlight->ctilx)
	{
		itilxHighlight = 0;

		itilyHighlight++;
		if (itilyHighlight >= playHighlight->ctily)
		{
			itilyHighlight = 0;

			playHighlight = Next(playHighlight);
			if (IsEOList(playHighlight))
			{
				playHighlight = Head(&GlobalBlockCopy->Layers);
			}
		}
	}
} /* HighlightNextTile */


/*********************************************************************
 *
 * HighlightNextNonNullTile
 *
 * PURPOSE
 *		Highlight the next the tile that isn't a NULL tile.
 *
 *		Sadly, if all tiles are NULL, then we have to return with
 *		a NULL tile highlighted.
 *
 * INPUT
 *		NONE.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
void HighlightNextNonNullTile(void)
{
	PlotType		 plt;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "HighlightNextNonNullTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (FSelectedPblk(GlobalBlockCopy))
	{
		HighlightNextTile();
#if UseSparseArray
		LAY_ReadPlotXY(playHighlight, itilxHighlight, itilyHighlight, &plt);
#else // !UseSparseArray
		ReadNPlotXY(playHighlight->rgrgplt, itilxHighlight, itilyHighlight, 1, &plt);
#endif // !UseSparseArray

		if (fNullTile(&plt))
		{
			LayerType	*play	 = playHighlight;
			short			 itilx = itilxHighlight;
			short			 itily = itilyHighlight;

			while (fNullTile(&plt))
			{
				HighlightNextTile();
#if UseSparseArray
				LAY_ReadPlotXY(playHighlight, itilxHighlight, itilyHighlight, &plt);
#else // !UseSparseArray
				ReadNPlotXY(playHighlight->rgrgplt, itilxHighlight, itilyHighlight, 1, &plt);
#endif // !UseSparseArray
				if (play == playHighlight &&
					itilx == itilxHighlight && itily == itilyHighlight)
				{
					break;							// we checked all tiles in brush...
				}
			}
		}
	}
	return;

} /* HighlightNextNonNullTile */


/*********************************************************************
 *
 * XORDrawTileHighlight
 *
 * SYNOPSIS
 *		static BOOL XORDrawTileHighlight(void)
 *
 * PURPOSE
 *		Actually draw the outline for the tile in question on screen.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns TRUE if actually tried to draw something.
 *
 * HISTORY
 *		04/27/94 Wednesday (dcc) - updated to deal with composite rooms
 *
 * SEE ALSO
 *
*/
static BOOL XORDrawTileHighlight(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "XORDrawTileHighlight";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prmHighlight)
	{
		if (GlobalRoomWindow &&
			 GlobalRoomWindow->CurrentRoom->Room == prmHighlight)
		{
			unsigned cxTile, cyTile;

			cxTile = prmHighlight->FloorLayer->cxTile;
			cyTile = prmHighlight->FloorLayer->cyTile;

//			HideBrush();

			if (IsComposite(prmHighlight) && !IsLocked(prmHighlight))
			{
				int wTA = prmHighlight->ptsComposite->wTilesAcross;
				int wTD = prmHighlight->ptsComposite->wTilesDown;
				int itilx = itilxRoom * wTA;
				int itily = itilyRoom * wTD;

				DrawTileOutline(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd,
									GlobalRoomWindow->CurrentRoom,
									itilx, itily,
									wTA, wTD,
									cxTile, cyTile,
									((GlobalRoomWindow->CurrentRoom->Flags) & DISPLAY_SEPERATED),
									0x3F);

				CopyDisplayTilesToBack(GlobalRoomWindow, itilx, itily,
								  			wTA, wTD,
											DISPLAYTOBACK);
			}
			else
			{
				DrawTileOutline(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd,
									GlobalRoomWindow->CurrentRoom,
									itilxRoom, itilyRoom,
									1, 1,
									cxTile, cyTile,
									((GlobalRoomWindow->CurrentRoom->Flags) & DISPLAY_SEPERATED),
									0x3F);

				CopyDisplayTilesToBack(GlobalRoomWindow, itilxRoom, itilyRoom,
								  			1, 1,
											DISPLAYTOBACK);
			}

//			ShowBrush();

			return TRUE;
		}
	}
	return FALSE;
} /* XORDrawTileHighlight */


/*********************************************************************
 *
 * HideTileHighlight
 *
 * SYNOPSIS
 *		void HideTileHighlight(void)
 *
 * PURPOSE
 *		If there is a tile highlight on-screen, erase it.
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
void HideTileHighlight(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "HideTileHighlight";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fShowingTileHighlight)
	{
		XORDrawTileHighlight();
		fShowingTileHighlight = FALSE;
	}
} /* HideTileHighlight */


/*********************************************************************
 *
 * DrawTileHighlight
 *
 * SYNOPSIS
 *		void DrawTileHighlight(void)
 *
 * PURPOSE
 *		Actually draw the outline for the tile in question on screen.
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
void DrawTileHighlight(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawTileHighlight";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	fShowingTileHighlight = XORDrawTileHighlight();

} /* DrawTileHighlight */


/*********************************************************************
 *
 * FFindPpltInPrm
 *
 * PURPOSE
 *		Search the room <prm> for the occurrence of <pplt>.
 *		<*pitilx> & <*pitily> are the initial tile to search.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns TRUE if found. <*pitilx> & <*pitily> contains the coord.
 *
 * HISTORY
 *		04/27/94 (dcc) - updated to deal with composite rooms
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *		Well, if we had designed it properly, we should be using the search
 *		module instead of this routine. Unfortunately, it is too much
 *		bother to construct a <pcbSB>, so we cobble up this routine instead.
 *
*/
static BOOL FFindPpltInPrm(PlotType *pplt, RoomType *prm,
									short *pitilx, short *pitily)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FFindPpltInPrm";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsComposite(prm))
	{
		*pitilx = (pplt->Tile_ID-1)%(prm->ptsComposite->Across+1);
		*pitily = (pplt->Tile_ID-1)/(prm->ptsComposite->Across+1);
		return TRUE;
	}
	else
	{
		for ( ; *pitily < prm->FloorLayer->ctily; (*pitily)++)
		{
			for ( ; *pitilx < prm->FloorLayer->ctilx; (*pitilx)++)
			{
				PlotType plt;

#if UseSparseArray
				LAY_ReadPlotXY(prm->FloorLayer, *pitilx, *pitily, &plt);
#else // !UseSparseArray
				ReadNPlotXY(prm->FloorLayer->rgrgplt, *pitilx, *pitily, 1, &plt);
#endif // !UseSparseArray
				if (plt.TileSet_ID == pplt->TileSet_ID && plt.Tile_ID == pplt->Tile_ID)
					return TRUE;
			}
			*pitilx = 0;
		}
	}
	return FALSE;
} /* FFindPpltInPrm */


/*********************************************************************
 *
 * ShowHighlightedTile
 *
 * PURPOSE
 *		Change room to source room containing currently highlighted tile,
 *		show the room, then highlight the tile.
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
 *		08/19/94 (dcc) - add support for UseSparseArray
 *		09/08/94 (dcc) - make sure ptst != NULL before searching 4 source room
 *		09/08/94 (dcc) - move TogglePrw() inside NULL ptst test
 *
 * SEE ALSO
 *
*/
void ShowHighlightedTile(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowHighlightedTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom->Room && playHighlight)
	{
		RoomType *prm = GlobalRoomWindow->CurrentRoom->Room;
		PlotType plt;

#if UseSparseArray
		LAY_ReadPlotXY(playHighlight, itilxHighlight, itilyHighlight, &plt);
#else // !UseSparseArray
		ReadNPlotXY(playHighlight->rgrgplt, itilxHighlight, itilyHighlight, 1, &plt);
#endif // !UseSparseArray

		if (!fNullTile(&plt))
		{
			TileSetType *ptst = FAST_TILESET_PTR(plt.TileSet_ID);

			if (ptst)
			{
				if (!(IsComposite(prm) || IsSource(prm)))
				{
					TogglePrw(GlobalRoomWindow);
				}
				prmHighlight = ptst->SourceRoom;
				TryToHookRoomWindow(prmHighlight, GlobalRoomWindow);

				itilxRoom = 0;
				itilyRoom = ptst->wDisplayRow;

				if (FFindPpltInPrm(&plt, prmHighlight, &itilxRoom, &itilyRoom))
				{
					MoveCenterTo(itilxRoom, itilyRoom, FALSE);

					SetPointerMode();
					MButtonsE();
				}
			}
		}
	}
} /* ShowHighlightedTile */

