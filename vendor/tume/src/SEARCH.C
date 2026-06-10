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
 * SEARCH.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 09/24/92
 *   MODIFIED : 11/08/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		This module searches for the next occurrence of a brush
 *		in the current room.
 *
 * HISTORY
 *		01/17/93 (dcc) - match Plot_Flags as well when
 *								searching tiles.
 *		04/05/93 (dcc) - use rectplot.h header.
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include "rectplot.h"
#include "tuglbl.h"
#include "message.h"
#include "search.h"							/* Verify function prototypes. */
#include "tumedraw.h"
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray
#include "szerror.h"

/**************************** C O N S T A N T S ***************************/

#define fSearchNew			0
#define fSearching			1
#define fSearchFromStart	2

char *szNOSB = "No more occurrences of Search Buffer found. ";

/******************************** T Y P E S *******************************/

typedef struct
{
	RoomType *prt;
	short x;
	short y;
} RSS;							// start searching at <x>+1!

/****************************** G L O B A L S *****************************/

/* Bits to match & bits to ignore when matching tile flags bits */

static ubFlagMask = 0xFF;	/* default to matching all bits */

/* rssSB, fSearchSTATE & fShowingSearchOutline are
	used by ResetRoomSearch() & SearchNext() */

static RSS rssSB;
static WORD fSearchSTATE = fSearchNew;
static WORD fShowingSearchOutline = FALSE;

static BlockCopyType *pbcSB = NULL;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * SetSearchMask
 *
 * SYNOPSIS
 *		void SetSearchMask(UBYTE ub)
 *
 * PURPOSE
 *		Set search mask to <ub>.
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
void SetSearchMask(UBYTE ub)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetSearchMask";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ubFlagMask = ub;

} /* SetSearchMask */


/*********************************************************************
 *
 * StartSearch
 *
 * SYNOPSIS
 *		static void StartSearch(RSS *prss, RoomType *prt, short x, short y)
 *
 * PURPOSE
 *		Initialize <prss> to start searching at <x>, <y>.
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
static void StartSearch(RSS *prss, RoomType *prt, short x, short y)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "StartSearch";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	prss->prt = prt;
	prss->x	 = x;
	prss->y   = y;

} /* StartSearch */


/*********************************************************************
 *
 * InitSearch
 *
 * SYNOPSIS
 *		static void InitSearch(RSS *prss, RoomType *prt)
 *
 * PURPOSE
 *		Initialize <prss> to search the entire room.
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
#define InitSearch(prss,prt) StartSearch(prss,prt,-1,0)


/*********************************************************************
 *
 * Matches
 *
 * SYNOPSIS
 *		BOOL Matches(RSS *prss, BlockCopyType *pbc)
 *
 * PURPOSE
 *		Returns TRUE if all layers in <pbc> matches the floor
 *		layer and up in room <prss->prt> at <prss->x>, <prss->y>.
 *
 *		This routine will only match layers that are visible.
 *		However, if all room layers are invisible, then this
 *		routine always returns FALSE.
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
BOOL Matches(RSS *prss, BlockCopyType *pbc)
{

	LayerType *play = prss->prt->FloorLayer;
	LayerType *playbc = pbc->BC_FloorLayer;
	short xmax = ((LayerType *) Head(&pbc->Layers))->ctilx;
	short ymax = ((LayerType *) Head(&pbc->Layers))->ctily;
	BOOL fVisible = FALSE;					/* Have we searched a visible layer? */

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Matches";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	while (!IsEOList(play) && !IsEOList(playbc))
	{
		if (!(play->L_Flags & LAYER_INVISIBLE))	/* If layer is visible... */
		{
			short x, y;

			fVisible = TRUE;					/* We search at least one layer */

			for (y = 0; y < ymax; y++)
			{
#if UseSparseArray
				if (!IsSparse(playbc) && !IsSparse(play))
				{
					PlotType *ppltdst = ActivatePlotXYatWin(play->p.rgrgplt,
																		prss->x, prss->y+y, winDst);
					PlotType *ppltsrc = ActivatePlotRowatWin(playbc->p.rgrgplt, y, winSrc);

					for (x = 0; x < xmax; x++)
					{
						if (ppltsrc->TileSet_ID != ppltdst->TileSet_ID ||
							 ppltsrc->Tile_ID    != ppltdst->Tile_ID ||
		 (ppltsrc->Plot_Flags & ubFlagMask) != (ppltdst->Plot_Flags & ubFlagMask))
						{
							ReleasePlotRow(playbc->p.rgrgplt, y);
							ReleasePlotRow(play->p.rgrgplt, prss->y+y);
							return FALSE;
						}
						ppltdst++;
						ppltsrc++;
					}
					ReleasePlotRow(playbc->p.rgrgplt, y);
					ReleasePlotRow(play->p.rgrgplt, prss->y+y);
				}
				else
				{
					for (x = 0; x < xmax; x++)
					{
						PlotType pltDst, pltSrc;

						LAY_ReadPlotXY(playbc, x, y, &pltSrc);
						LAY_ReadPlotXY(play, prss->x+x, prss->y+y, &pltDst);

						if (pltSrc.TileSet_ID != pltDst.TileSet_ID ||
							 pltSrc.Tile_ID    != pltDst.Tile_ID ||
		 (pltSrc.Plot_Flags & ubFlagMask) != (pltDst.Plot_Flags & ubFlagMask))
						{
							return FALSE;
						}
					}
				}
#else // !UseSparseArray
				PlotType *ppltdst = ActivatePlotXYatWin(play->rgrgplt,
																	prss->x, prss->y+y, winDst);
				PlotType *ppltsrc = ActivatePlotRowatWin(playbc->rgrgplt, y, winSrc);

				for (x = 0; x < xmax; x++)
				{
					if (ppltsrc->TileSet_ID != ppltdst->TileSet_ID ||
						 ppltsrc->Tile_ID    != ppltdst->Tile_ID ||
	 (ppltsrc->Plot_Flags & ubFlagMask) != (ppltdst->Plot_Flags & ubFlagMask))
					{
						ReleasePlotRow(playbc->rgrgplt, y);
						ReleasePlotRow(play->rgrgplt, prss->y+y);
						return FALSE;
					}
					ppltdst++;
					ppltsrc++;
				}
				ReleasePlotRow(playbc->rgrgplt, y);
				ReleasePlotRow(play->rgrgplt, prss->y+y);
#endif // !UseSparseArray
			}
		}
		play = Next(play);
		playbc = Next(playbc);
	}
	return fVisible;
} /* Matches */


/*********************************************************************
 *
 * NextSearch
 *
 * SYNOPSIS
 *		static BOOL NextSearch(RSS *prss, BlockCopyType *pbc)
 *
 * PURPOSE
 *		Find the next occurrence of brush <pbc> in <prss>.
 *		Update <prss> for subsequent searches.
 *
 *		This routine searches left to right, then top to bottom.
 *
 * INPUT
 *
 *
 * ASSUMES
 *		NOTE: BUGBUG This routine has a very SERIOUS bug for sparse
 *		arrays! It will NOT work properly if the brush contains
 *		a NULL tile at location 0,0!
 *
 * RETURN VALUE
 *		TRUE if brush found at (prss->x, prss->y), else FALSE.
 *
 * HISTORY
 *		09/09/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
static BOOL NextSearch(RSS *prss, BlockCopyType *pbc)
{
	LayerType *play = prss->prt->FloorLayer;
	int xmaxin = play->ctilx - ((LayerType *) Head(&pbc->Layers))->ctilx;
	int ymaxin = play->ctily - ((LayerType *) Head(&pbc->Layers))->ctily;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "NextSearch";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if UseSparseArray
	if (IsSparse(play))
	{
		int x, y;

		SetSparseLayerLimits(play->p.pspa, 0, prss->y,
									xmaxin+1, ymaxin-prss->y+1);

		while (GetNextSparsePlotInRange(play->p.pspa, &x, &y) != NULL)
		{
			if (	(y == prss->y && x > prss->x) ||
					y > prss->y	)
			{
				prss->x = x;
				prss->y = y;

				if (Matches(prss, pbc))
					return TRUE;
			}
		}
		prss->x = 0;
		prss->y = ymaxin+1;
	}
	else
	{
#endif // UseSparseArray
		prss->x++;

		for ( ; prss->y <= ymaxin; prss->y++)
		{
			for ( ; prss->x <= xmaxin; prss->x++)
			{
				if (Matches(prss, pbc))
					return TRUE;
			}
			prss->x = 0;
		}
#if UseSparseArray
	}
#endif // UseSparseArray
	return FALSE;
} /* NextSearch */


/*********************************************************************
 *
 * CountBrush
 *
 * SYNOPSIS
 *		ULONG CountBrush(RoomType *prt, BlockCopyType *pbc)
 *
 * PURPOSE
 *		Count the occurrences of brush <pbc> in the floor
 *		layer and above of <prt>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Number of times brush <pbc> occurs.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
ULONG CountBrush(RoomType *prt, BlockCopyType *pbc)
{

	RSS rss;
	ULONG ul = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CountBrush";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	InitSearch(&rss, prt);

	while (NextSearch(&rss, pbc))
		ul++;

	return ul;

} /* CountBrush */


/*********************************************************************
 *
 * HideSearchOutlineOnly
 *
 * SYNOPSIS
 *		void HideSearchOutlineOnly(void)
 *
 * PURPOSE
 *		Erase the outline around what was found.
 *		This routine does not erase or draw the tile-brush.
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
void HideSearchOutlineOnly(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "HideSearchOutlineOnly";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fShowingSearchOutline)
	{
		if (GlobalRoomWindow &&
			 GlobalRoomWindow->CurrentRoom->Room == rssSB.prt)
		{
			XORDrawPBlk(pbcSB, 0x11, DEST_EOR);
			CopyDisplayTilesToBack(pbcSB->DestRW, pbcSB->DestX, pbcSB->DestY,
//#if !DifferentTileSizes
								  		((LayerType *) Head(&pbcSB->Layers))->ctilx,
								  		((LayerType *) Head(&pbcSB->Layers))->ctily,
//#endif /* !DifferentTileSizes */
										DISPLAYTOBACK);
		}
		fShowingSearchOutline = FALSE;
	}
} /* HideSearchOutlineOnly */


/*********************************************************************
 *
 * HideSearchOutline
 *
 * SYNOPSIS
 *		void HideSearchOutline(void)
 *
 * PURPOSE
 *		Erase the outline around what was found.
 *		This routine does erase and re-draw the tile-brush.
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
void HideSearchOutline(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "HideSearchOutline";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fShowingSearchOutline)
	{
		if (GlobalRoomWindow &&
			 GlobalRoomWindow->CurrentRoom->Room == rssSB.prt)
		{
			if (IsEdit(rssSB.prt) || (IsComposite(rssSB.prt) && !IsLocked(rssSB.prt)))
				HideBrush();

			HideSearchOutlineOnly();

			if (IsEdit(rssSB.prt) || (IsComposite(rssSB.prt) && !IsLocked(rssSB.prt)))
				ShowBrush();
		}
		fShowingSearchOutline = FALSE;
	}
} /* HideSearchOutline */


/*********************************************************************
 *
 * ShowNewSearchOutline
 *
 * SYNOPSIS
 *		void ShowNewSearchOutline(void)
 *
 * PURPOSE
 *		Draw a dotted outline around what was found.
 *		This routine always draws one, as it is called by ShowRoom().
 *		It also doesn't hide the brush or show the brush.
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
void ShowNewSearchOutline(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowNewSearchOutline";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pbcSB && pbcSB->BC_FloorLayer && rssSB.x != -1)
	{
		if (GlobalRoomWindow &&
			 GlobalRoomWindow->CurrentRoom->Room == rssSB.prt)
		{
			XORDrawPBlk(pbcSB, 0x11, DEST_EOR);
			CopyDisplayTilesToBack(pbcSB->DestRW, pbcSB->DestX, pbcSB->DestY,
//#if !DifferentTileSizes
								  		((LayerType *) Head(&pbcSB->Layers))->ctilx,
								  		((LayerType *) Head(&pbcSB->Layers))->ctily,
//#endif /* !DifferentTileSizes */
										DISPLAYTOBACK);
			fShowingSearchOutline = TRUE;
		}
	}

} /* ShowNewSearchOutline */


/*********************************************************************
 *
 * ShowSearchOutline
 *
 * SYNOPSIS
 *		void ShowSearchOutline(void)
 *
 * PURPOSE
 *		Draw a dotted outline around what was found.
 *		Only draw outline if not already drawn.
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
void ShowSearchOutline(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowSearchOutline";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!fShowingSearchOutline && pbcSB && pbcSB->BC_FloorLayer && rssSB.x != -1)
	{
		if (GlobalRoomWindow &&
			 GlobalRoomWindow->CurrentRoom->Room == rssSB.prt)
		{
			if (IsEdit(rssSB.prt) || (IsComposite(rssSB.prt) && !IsLocked(rssSB.prt)))
				HideBrush();

			ShowNewSearchOutline();

			if (IsEdit(rssSB.prt) || (IsComposite(rssSB.prt) && !IsLocked(rssSB.prt)))
				ShowBrush();
		}
	}
} /* ShowSearchOutline */


/*********************************************************************
 *
 * ResetRoomSearch
 *
 * SYNOPSIS
 *		static void ResetRoomSearch(void)
 *
 * PURPOSE
 *		This routine resets the search function. It should be called
 *		prior to display a new room, before the call the ShowRoom().
 *
 *		Since ShowRoom() will be called, there is no need to erase
 *		the old outline surrounding the found tiles.
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
static void ResetRoomSearch(void)
{

	RoomType *prt;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ResetRoomSearch";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom &&
		 GlobalRoomWindow->CurrentRoom->Room)
	{
		prt = GlobalRoomWindow->CurrentRoom->Room;

		HideSearchOutline();
		InitSearch(&rssSB, prt);
		fSearchSTATE = fSearchNew;
	}

} /* ResetRoomSearch */


/*********************************************************************
 *
 * SearchNext
 *
 * SYNOPSIS
 *		short SearchNext(void)
 *
 * PURPOSE
 *		This is the menu event that searches for the next occurrence
 *		of the Search Buffer, and outlines what is found.
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
short SearchNext(void)
{

	static WORD wInitX;
	static WORD wInitY;
	static ULONG ul;
	char sz[256];
	RoomType *prt;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SearchNext";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!pbcSB || !pbcSB->BC_FloorLayer)
	{
		TellUser(NCD, "No Search Buffer specified.");
		return 0;
	}

	if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom &&
		 GlobalRoomWindow->CurrentRoom->Room)
	{
		prt = GlobalRoomWindow->CurrentRoom->Room;
	}
	else
	{
		return 0;
	}

//BUGBUG: Perhaps we should call mitems#FMayDrawHere() instead

	if (((LayerType *) Head(&pbcSB->Layers))->cxTile != prt->FloorLayer->cxTile ||
		 ((LayerType *) Head(&pbcSB->Layers))->cyTile != prt->FloorLayer->cyTile)
	{
		TellUser(NCD, "Search Buffer has different size tile.");
		return 0;
	}

	if (fSearchSTATE == fSearchNew || rssSB.prt != prt)
	{
		wInitX = CurrentX;
		wInitY = CurrentY;
		ul = 0;

		/* Set variables pbcSB to point to destination room */

		pbcSB->DestRW = GlobalRoomWindow;
		pbcSB->DestStuff = GlobalRoomWindow->CurrentRoom;

		/* Use current pointer X, Y position to start search */

		StartSearch(&rssSB, prt, CurrentX, CurrentY);
		fSearchSTATE = fSearching;
	}

	while (1)
	{
		HideSearchOutline();

		if (NextSearch(&rssSB, pbcSB))
		{
			if (fSearchSTATE == fSearchFromStart &&
				((rssSB.x > wInitX && rssSB.y == wInitY) || rssSB.y > wInitY))
			{
				break;
			}
			else
			{
				ul++;

				pbcSB->DestX = rssSB.x;
				pbcSB->DestY = rssSB.y;

				MoveRequest(CurrentX-rssSB.x, CurrentY-rssSB.y);

				ShowSearchOutline();
				return 1;
			}
		}
		else
		{
			if (fSearchSTATE == fSearching)
			{
				if (DoThis2("Search", szNOSB, "Continue from beginning of room?"))
				{
					InitSearch(&rssSB, prt);
					fSearchSTATE = fSearchFromStart;
				}
				else
				{
					return 1;
				}
			}
			else
			{
				break;
			}
		}
	}
	sprintf(sz, "%sSearch Buffer found %lu times(s).", szNOSB, ul);
	TellUser("Search", sz);
	ResetRoomSearch();
	return 1;

} /* SearchNext */


/*********************************************************************
 *
 * Replace
 *
 * SYNOPSIS
 *		short Replace(void)
 *
 * PURPOSE
 *		This is the menu event that searches for the next occurrence
 *		of the Search Buffer, and replaces what was found with the
 *		contents of the current brush.
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
short Replace(void)
{

	static WORD wInitX;
	static WORD wInitY;
	static ULONG ul, ulR;

	WORD wSaveX;
	WORD wSaveY;
	BOOL fGlobal = FALSE;

	char sz[256];
	RoomType *prt;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Replace";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!pbcSB || !pbcSB->BC_FloorLayer)
	{
		TellUser(NCD, "No Search Buffer specified.");
		return 0;
	}

	if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom &&
		 GlobalRoomWindow->CurrentRoom->Room)
	{
		prt = GlobalRoomWindow->CurrentRoom->Room;
	}
	else
	{
		return 0;
	}

//BUGBUG: Perhaps we should call mitems#FMayDrawHere() instead

	if (((LayerType *) Head(&pbcSB->Layers))->cxTile != prt->FloorLayer->cxTile ||
		 ((LayerType *) Head(&pbcSB->Layers))->cyTile != prt->FloorLayer->cyTile)
	{
		TellUser(NCD, "Search Buffer has different size tile.");
		return 0;
	}

	if (fSearchSTATE == fSearchNew || rssSB.prt != prt)
	{
		wInitX = CurrentX;
		wInitY = CurrentY;
		ul = ulR = 0;

		/* Set variables pbcSB to point to destination room */

		pbcSB->DestRW = GlobalRoomWindow;
		pbcSB->DestStuff = GlobalRoomWindow->CurrentRoom;

		/* Use current pointer X, Y position to start search */

		StartSearch(&rssSB, prt, CurrentX, CurrentY);
		fSearchSTATE = fSearching;
	}

	HideBrush();
	wSaveX = GlobalBlockCopy->DestX;
	wSaveY = GlobalBlockCopy->DestY;

	while (1)
	{
		HideSearchOutlineOnly();

		if (NextSearch(&rssSB, pbcSB))
		{
			if (fSearchSTATE == fSearchFromStart &&
				((rssSB.x > wInitX && rssSB.y == wInitY) || rssSB.y > wInitY))
			{
				break;
			}
			else
			{
				WORD fResult;

				ul++;

				pbcSB->DestX = rssSB.x;
				pbcSB->DestY = rssSB.y;

				ShowNewSearchOutline();

				MoveRequest(CurrentX-rssSB.x, CurrentY-rssSB.y);

				/* Fake out CurrentX, CurrentY for subsequent MoveRequest()'s */

				CurrentX = rssSB.x;
				CurrentY = rssSB.y;

//				ShowNewSearchOutline();

				if (!fGlobal)
				{
					InitKeyList();
					AddKeyOption(27, 4);			/* ESCAPE returns EXIT option */
					AddKeyOption(13, 2);			/* ENTER returns YES option */
					fResult = PMessage(MainWindow, "Replace", "Replace tiles?",
											" All | Yes | No | Exit ");

#define fALL	1
#define fYES	2
#define fNO		3
#define fEXIT	4

					if (fResult == fALL)
					{
						fGlobal = TRUE;
					}
				}
				if (fResult == fYES || fGlobal)
				{
					HideSearchOutlineOnly();

					GlobalBlockCopy->DestX = rssSB.x;
					GlobalBlockCopy->DestY = rssSB.y;

					/* If first replace, start new undo information */

					if (ulR == 0)
					{
						StartUndo (GlobalRoomWindow->CurrentRoom->Room, UndoRoom,
							CurrentX, CurrentY,
							(WORD) ((CurrentX + ((LayerType *) Head(&GlobalBlockCopy->Layers))->ctilx) - 1),
							(WORD) ((CurrentY + ((LayerType *) Head(&GlobalBlockCopy->Layers))->ctily) - 1));
					}
#define	ERASE	(0)
#define	DRAW	(1)

					PlaceBlockCopy(GlobalBlockCopy, DRAW);
					ulR++;

					if (fGlobal)
					{
						UWORD	key;

						key = GetKeyNoWait ();
						if (key)
						{
							WORD w = ConvertKeyToAscii(key);

							if (w == 0x1B || w == 0x20)
							{
								goto EXIT;
							}
						}
					}
				}
				else if (fResult == fEXIT)
					goto EXIT;
			}
		}
		else
		{
			if (fSearchSTATE == fSearching)
			{
				if (DoThis2("Replace", szNOSB, "Continue from beginning of room?"))
				{
					fGlobal = FALSE;
					InitSearch(&rssSB, prt);
					fSearchSTATE = fSearchFromStart;
				}
				else
				{
					goto EXIT;
				}
			}
			else
			{
				break;
			}
		}
	}
	sprintf(sz, "%sFound %lu, replaced %lu.", szNOSB, ul, ulR);
	TellUser("Search", sz);
	ResetRoomSearch();
EXIT:
	GlobalBlockCopy->DestX = wSaveX;
	GlobalBlockCopy->DestY = wSaveY;
	ShowBrush();
	return 1;

} /* Replace */


/*********************************************************************
 *
 * BCcpy
 *
 * SYNOPSIS
 *		static BOOL BCcpy(BlockCopyType *pbcDst, BlockCopyType *pbcSrc)
 *
 * PURPOSE
 *		Make a copy of <pbcSrc> in <pbcDst>.
 *		Use to fill the Search Buffer.
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
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
static BOOL BCcpy(BlockCopyType *pbcDst, BlockCopyType *pbcSrc)
{

	LayerType *playSrc, *playDst;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "BCcpy";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Free any layers currently in pbcDst */

	DeAllocateLayers(&(pbcDst->Layers));

	/* The following elements of structure BlockCopyType are set correctly
		by the structure assignment:

		BC_Flags
		SourceRW							SourceStuff						SourceRoom
		SourceX							SourceY							Width
		Height							LayerCount						FirstTileSet */

	*pbcDst = *pbcSrc;

	/* Now we need to copy all the layers */

	InitList(&(pbcDst->Layers));

	playSrc = Head(&(pbcSrc->Layers));

	while (!IsEOList(playSrc))
	{
		/* Allocate new layer */

#if UseSparseArray
		if ((playDst = AddClearedLayerToLayers(&(pbcDst->Layers),
				playSrc->ctilx, playSrc->ctily, IsSparse(playSrc))) != NULL)
#else // !UseSparseArray
		if ((playDst = AddClearedLayerToLayers(&(pbcDst->Layers),
				playSrc->ctilx, playSrc->ctily)) != NULL)
#endif // !UseSparseArray
		{
			int iy;

			/* Copy a layer from pbcSrc to pbcDst */

			for (iy = 0; iy < playSrc->ctily; iy++)
			{
#if UseSparseArray
				if (IsSparse(playSrc) && IsSparse(playDst))
				{
					int itilx;

					for (itilx = 0; itilx < playSrc->ctilx; itilx++)
					{
						PlotType plt;

						ReadSparsePlotXY(playSrc->p.pspa, itilx, iy, &plt);
						WriteSparsePlotXY(&plt, playDst->p.pspa, itilx, iy);
					}
				}
				else if (IsSparse(playSrc) && !IsSparse(playDst))
				{
					int itilx;

					for (itilx = 0; itilx < playSrc->ctilx; itilx++)
					{
						PlotType plt;

						ReadSparsePlotXY(playSrc->p.pspa, itilx, iy, &plt);
						WriteNPlotXY(&plt, 1, playDst->p.rgrgplt, itilx, iy);
					}
				}
				else if (!IsSparse(playSrc) && IsSparse(playDst))
				{
					int itilx;

					for (itilx = 0; itilx < playSrc->ctilx; itilx++)
					{
						PlotType plt;

						ReadNPlotXY(playSrc->p.rgrgplt, itilx, iy, 1, &plt);
						WriteSparsePlotXY(&plt, playDst->p.pspa, itilx, iy);
					}
				}
				else
				{
					PlotType *ppltSrc;

					ppltSrc = ActivatePlotRow(playSrc->p.rgrgplt, iy);
					WriteNPlotXY(ppltSrc, playSrc->ctilx,
							 	playDst->p.rgrgplt, 0, iy);
					ReleasePlotRow(playSrc->p.rgrgplt, iy);
				}
#else // !UseSparseArray
				PlotType *ppltSrc;

				ppltSrc = ActivatePlotRow(playSrc->rgrgplt, iy);
				WriteNPlotXY(ppltSrc, playSrc->ctilx,
							 playDst->rgrgplt, 0, iy);
				ReleasePlotRow(playSrc->rgrgplt, iy);
#endif // !UseSparseArray
			}
			playDst->cxTile = playSrc->cxTile;
			playDst->cyTile = playSrc->cyTile;
		}
		else
		{
			return FALSE;
		}
		playSrc = Next(playSrc);
	}

	/* Set BC_FloorLayer */

	playDst = Head(&(pbcDst->Layers));
	if (!IsEOList(playDst))
	{
		pbcDst->BC_FloorLayer = playDst;
	}
	else
	{
		pbcDst->BC_FloorLayer = NULL;
	}

	/* The following elements of structure BlockCopyType still need to be
		set depending on which room we are searching:

		DestRW							DestStuff
		DestX								DestY */

	return TRUE;

} /* BCcpy */


/*********************************************************************
 *
 * ClearSearchBuffer
 *
 * SYNOPSIS
 *		void ClearSearchBuffer(void)
 *
 * PURPOSE
 *		Clear the Search Buffer, free any allocated memory.
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
void ClearSearchBuffer(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ClearSearchBuffer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pbcSB)
	{
		HideSearchOutline();

		/* Free any layers currently in pbcDst */

		DeAllocateLayers(&(pbcSB->Layers));
		pbcSB->BC_FloorLayer = NULL;		/* This indicates no Search Buffer */
	}

} /* ClearSearchBuffer */


/*********************************************************************
 *
 * ClearSearchBufferIfFromRoom
 *
 * SYNOPSIS
 *		void ClearSearchBufferIfFromRoom(RoomType *prt)
 *
 * PURPOSE
 *		Clear the Search Buffer if it was from room <prt>.
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
 *		07/18/94 Monday (dcc) - make sure pbcSB != NULL B4 checking SourceRoom
 *
 *
 * SEE ALSO
 *
*/
void ClearSearchBufferIfFromRoom(RoomType *prt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ClearSearchBufferIfFromRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pbcSB != NULL && pbcSB->SourceRoom == prt)
		ClearSearchBuffer();

} /* ClearSearchBufferIfFromRoom */


/*********************************************************************
 *
 * SetSearchBuffer
 *
 * SYNOPSIS
 *		short SetSearchBuffer(void)
 *
 * PURPOSE
 *		This is the menu event that sets the Search Buffer.
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
 *		09/09/94 (dcc) - don't allow setting search buffer if tile at 0, 0 is NULL
 *
 * SEE ALSO
 *
*/
short SetSearchBuffer(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetSearchBuffer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!pbcSB)
	{
		if ((pbcSB = MEM_calloc(1, sizeof(BlockCopyType))) == NULL)
			goto ABORT;

		InitList(&(pbcSB->Layers));
	}

#if UseSparseArray
	{	//KLUDGE BUGBUG FIXME
		PlotType plt;

		LAY_ReadPlotXY(GlobalBlockCopy->BC_FloorLayer, 0, 0, &plt);
		if (fZeroTile(&plt))
		{
			TellUser(NCD, "First tile in brush must not be null.");
			return 0;
		}
	}
#endif // UseSparseArray

	ResetRoomSearch();
	if (BCcpy(pbcSB, GlobalBlockCopy))
	{
		ShowStatus ("Search Buffer defined.");
		return 1;
	}

ABORT:
	TellUser(OOM, "Unable to set Search Buffer.");
	return 0;

} /* SetSearchBuffer */

