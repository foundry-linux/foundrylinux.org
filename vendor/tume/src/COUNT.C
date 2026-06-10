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
 * COUNT.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 02/19/92
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
 *		Routine to count tiles.
 *
 * HISTORY
 *		02/19/92 (dcc) - created
 *		04/05/93 (dcc) - use rectplot.h header.
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include	"switch1.h"							// only for PixelLayers
//#include "switch1.h"						// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include <echidna/etypes.h>
#include <echidna/listfunc.h>
#include <stdio.h>

#include "rectplot.h"
#include "tuglbl.h"
#include "count.h"
#if UseSparseArray
#include "layer.h"
#endif // UseSparseArray
#include "szerror.h"

/**************************** C O N S T A N T S ***************************/

#define uwBytesPerBank (32704)

#define sizeofCharHeader (sizeof(ByteMap)+sizeof(UWORD))

/******************************** T Y P E S *******************************/

typedef struct
{
	NodeType Node;
	XTRAPntr pxtCharBank;
} NodPxtBank;

typedef struct
{
	UWORD bGroupMatch;
	UWORD fInARoom;
} TileUseFlag;

/****************************** G L O B A L S *****************************/

UBYTE ubCharacterANDMask = 0xff;

UWORD uwCharWidth = 0;
UWORD uwCharHeight = 0;

UWORD uwCharMax[wGroupCntMax];					/* Number of characters saved */
static ListType lisCharBanks[wGroupCntMax];	/* Linked list of all character banks */

static ByteMap bmChar;								/* Place to grab a character */
static ByteMap bmXFlip;								/* Place to store x-flipped char */
static ByteMap bmYFlip;								/* Place to store y-flipped char */
static ByteMap bmXYFlip;							/* Place to store xy-flipped char */

static UWORD uwBytesPerChar;
static UWORD uwCharsPerBank;

GroupDefList GroupDef[wGroupCntMax];			/* Array of list of GroupSets */
UWORD bGroupFlags[wGroupCntMax];					/* Merge, XFlip, YFlip, XYFlip */
static UWORD uwGroupCharCount[wGroupCntMax];	/* Count of chars in group. */

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * OpenCountChars
 *
 * SYNOPSIS
 *		BOOL OpenCountChars(WORD wMax)
 *
 * PURPOSE
 *		Initialize variables for count tiles.
 *		Allocate list nodes for every line in every group.
 *		<w> contains total number of groups.
 *		uwCharMax[] contains number of lines per group.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		FALSE if unsuccessful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL OpenCountChars(WORD wMax)
{

	WORD w;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenCountChars";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (w = 0; w < wGroupCntMax; w++)
	{
		InitList(&GroupDef[w]);
	}
	for (w = 0; w < wMax; w++)
	{
		WORD i;

		for (i = 0; i < uwCharMax[w]; i++)
		{
			GroupSet *pGS;

			if ((pGS = CreateNode(sizeof(GroupSet), NULL)) != NULL)
			{
				AddTail(&GroupDef[w], pGS);
			}
			else
			{
				return FALSE;
			}
		}
	}
	return TRUE;
} /* OpenCountChars */


/*********************************************************************
 *
 * UpdateCharSize
 *
 * SYNOPSIS
 *		void UpdateCharSize(UWORD uwCharWidth, UWORD uwCharHeight)
 *
 * PURPOSE
 *		If uwCharWidth * uwCharHeight + (overhead) > uwBytesPerChar then
 *			uwBytesPerChar = uwCharWidth * uwCharHeight + (overhead).
 *
 *		If uwCharWidth == 0, this means we will count all tiles,
 *		regardless of width. This is why we need to keep track of the
 *		largest tile we have encountered.
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
void UpdateCharSize(UWORD uwCharWidth, UWORD uwCharHeight)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateCharSize";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (uwCharWidth * uwCharHeight + sizeofCharHeader > uwBytesPerChar)
	{
		uwBytesPerChar = uwCharWidth * uwCharHeight + sizeofCharHeader;
		uwCharsPerBank = uwBytesPerBank / uwBytesPerChar;
	}
} /* UpdateCharSize */

/*********************************************************************
 *
 * CloseTileUseFlags
 *
 * SYNOPSIS
 *		static void CloseTileUseFlags(TileSpaceType *pTileSpace)
 *
 * PURPOSE
 *		Free flag structure used to track tile usage.
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
static void CloseTileUseFlags(TileSpaceType *pTileSpace)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CloseTileUseFlags";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pTileSpace)
	{
		TileSetType *pTS;

		pTS = Head(&pTileSpace->TileSets);

		while (!IsEOList(pTS))
		{
			if (pTS->rgRoomUseFlags)
			{
				FreeXTRA(pTS->rgRoomUseFlags);
			}
			pTS = Next(pTS);
		}
	}
} /* CloseTileUseFlags */


/*********************************************************************
 *
 * OpenTileUseFlags
 *
 * SYNOPSIS
 *		static BOOL OpenTileUseFlags(TileSpaceType *pTileSpace)
 *
 * PURPOSE
 *		Allocate memory for flag structure to keep track of tile usage,
 *		and reset all flags.
 *
 * INPUT
 *
 *
 * EFFECTS
 *		All TileSetType->rgRoomUseFlags point to some XTRA memory, and
 *		all TileSetType->bGroupTileNum = 0.
 *
 *
 * RETURN VALUE
 *		FALSE if unsuccessful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL OpenTileUseFlags(TileSpaceType *pTileSpace)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenTileUseFlags";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pTileSpace)
	{
		TileSetType *pTS = Head(&pTileSpace->TileSets);

		/* Set all tile use flags to NULL. */

		while (!IsEOList(pTS))
		{
			pTS->rgRoomUseFlags = NULL;
			pTS->bGroupTileNum = 0;

			pTS = Next(pTS);
		}

		/* Allocate memory for all tile use flags. */

		pTS = Head(&pTileSpace->TileSets);

		while (!IsEOList(pTS))
		{
			/* Only include tile sets that are some integral multiple
				of a character size. */

			if ((!uwCharWidth ||  (uwCharWidth &&  pTS->Width % uwCharWidth  == 0)) &&
				(!uwCharHeight || (uwCharHeight && pTS->Height % uwCharHeight == 0)))
			{
				UBYTE *pub;

				if ((pTS->rgRoomUseFlags = AllocXTRA(pTS->TileCount * sizeof(TileUseFlag))) == NULL)
				{
					CloseTileUseFlags(pTileSpace);
					return NULL;
				}

				pub = ActivateXTRA(pTS->rgRoomUseFlags);
				memset(pub, 0, pTS->TileCount * sizeof(TileUseFlag));
				UpdateXTRA(pTS->rgRoomUseFlags);
			}
			pTS = Next(pTS);
		}
	}
	return TRUE;
} /* OpenTileUseFlags */


/*********************************************************************
 *
 * MarkTilesetGroups
 *
 * SYNOPSIS
 *		static void MarkTilesetGroups(TileSpaceType *pTileSpace)
 *
 * PURPOSE
 *		Figure out which groups each tileset belongs in, and mark
 *		appropriate bit in group.
 *
 *		Look through array GroupDef[] which has a linked list for each
 *		group. Look through each link list, and mark the appropriate
 *		bit in tileset->bGroupTileNum if the tileset->UserType is in the
 *		range GroupSet->uwTileLow..GroupSet->uwTileHigh.
 *
 * INPUT
 *
 *
 * EFFECTS
 *		All TileSetType->bGroupTileNum group bits are set if
 *			TileSetType->UserType falls in range for some GroupSet in group.
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
static void MarkTilesetGroups(TileSpaceType *pTileSpace)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MarkTilesetGroups";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pTileSpace)
	{
		TileSetType *pTS = Head(&pTileSpace->TileSets);

		while (!IsEOList(pTS))						/* For every tileset */
		{
			/* Only include tile sets that are some integral multiple
				of a character size. */

			if ((!uwCharWidth ||  (uwCharWidth &&  pTS->Width % uwCharWidth  == 0)) &&
				(!uwCharHeight || (uwCharHeight && pTS->Height % uwCharHeight == 0)))
			{
				WORD w;
				UWORD uwBitMask = 1;

				for (w = 0; w < wGroupCntMax; w++)	/* Search all groups */
				{
					GroupSet *pGS;

					pGS = Head(&GroupDef[w]);

					while (!IsEOList(pGS))	/* Try to match each definition. */
					{
						if (pGS->uwTileLow <= (UWORD) pTS->UserType &&
							(UWORD) pTS->UserType <= pGS->uwTileHigh)
						{
							pTS->bGroupTileNum |= uwBitMask;
							break;
						}
						pGS = Next(pGS);
					}
					uwBitMask <<= 1;
				}
			}
			pTS = Next(pTS);
		}
	}
} /* MarkTilesetGroups */


/*********************************************************************
 *
 * MarkRoomGroups
 *
 * SYNOPSIS
 *		static void MarkRoomGroups(MapType *pMT, RoomType *pRT)
 *
 * PURPOSE
 *		For every tile in every edit room, mark the corresponding tile
 *		as appearing in an edit room.
 *
 *		Then see if it belongs to some group (tileset->UserType is in the
 *		range GroupSet->uwTileLow..GroupSet->uwTileHigh) and room->UserType
 *		is in the range GroupSet->uwRoomLow..GroupSet->uwRoomHigh. If so,
 *		set the corrsponding bit for the group in <bGroupMatch[]>.
 *
 *		All <bGroupTileNum> flags should have been set by MarkTilesetGroups()
 *		before you call this routine.
 *
 * INPUT
 *
 *
 * EFFECTS
 *		All TileSet->rgRoomUseFlags->TileUseFlag[].bGroupMatch group bit
 *			set if some tile in some room meets both the tileset type
 *			criteria and the room user type criteria.
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		08/19/94 (dcc) - add support for UseSparseArray
 *		09/08/94 (dcc) - KLUDGE BUGBUG FIXME PixelLayers-don't process 1x1 layers
 *		09/08/94 (dcc) - don't mark if no tiles stamped into layer yet
 *		11/08/94 (dcc) - Revised to use GetNextPlotInRange().
 *
 * SEE ALSO
 *
*/
static void MarkRoomGroups(MapType *pMT, RoomType *pRT)
{

	RoomType *prt;

	XTRAPntr pxtLastWinDst = NULL;
	TileUseFlag *pTUF;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MarkRoomGroups";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pRT = pRT;									// Turn off pRT not used warning

	prt = Head(&pMT->Rooms);

	while (!IsEOList(prt))
	{
		if (IsEdit(prt))
		{
			LayerType *pl;
			UWORD bGroupRoomNum = 0;
			WORD w;
			UWORD uwBitMask = 1;

			/* Set all bits in bGroupRoomNum based on prt->UserType falling
				between uwRoomLow..uwRoomHigh in some GroupSet. This will
				speed up search for tiles that meet BOTH conditions at
				the same time. */

			for (w = 0; w < wGroupCntMax; w++)
			{
				GroupSet *pGS;

				pGS = Head(&GroupDef[w]);

				while (!IsEOList(pGS))	/* Try to match each definition. */
				{
					if (pGS->fRoom &&
						pGS->uwRoomLow <= (UWORD) prt->UserType &&
						prt->UserType <= (UWORD) pGS->uwRoomHigh)
					{
						/* Roomtype matched at least one GroupSet in group. */

						bGroupRoomNum |= uwBitMask;
						break;
					}
					pGS = Next(pGS);
				}
				uwBitMask <<= 1;
			}

			pl = Head(&prt->Layers);

			while (!IsEOList(pl))
			{
				int wx, wy;

#if PixelLayers
				if (pl->cxTile > 1 && pl->cyTile > 1)	// KLUDGE BUGBUG FIXME
#else // !PixelLayers
				if (pl->cxTile != 0 && pl->cyTile != 0)	// tiles stamped into layer?
#endif // !PixelLayers
				{
#if UseSparseArray
					PlotType *pp;

					SetLayerLimits(pl, 0, 0, pl->ctilx, pl->ctily);

					while ((pp = GetNextPlotInRange(pl, &wx, &wy)) != NULL)
					{
						TileSetType *pTS;
						BOOL fFirstTimeInTS = TRUE;

						pTS = FAST_TILESET_PTR(pp->TileSet_ID);
						if (pTS->bGroupTileNum)		/* tileset in SOME group */
						{
							WORD w;
							UWORD uwBitMask = 1;

							/* If pTS->rgRoomUseFlags has changed (!= pxtLastWinDst)
								then write out old pTUF[], read in new pTUF[] */

							if (pTS->rgRoomUseFlags != pxtLastWinDst)
							{
								if (pxtLastWinDst)
								{
									UpdateXTRA(pxtLastWinDst);
								}
								pTUF = (TileUseFlag *) ActivateXTRAatWin(pTS->rgRoomUseFlags, winDst);
								pxtLastWinDst = pTS->rgRoomUseFlags;
							}

							/* Mark tile as appearing in a room. */

							pTUF[pp->Tile_ID-1].fInARoom = TRUE;

							/* Search through all group lists. */

							for (w = 0; w < wGroupCntMax; w++)
							{
								/* Make sure tileset type && room type both match
									for a particular GroupSet in the group. Up to
									this point we have determined (individually)
									if the tileset type matched and if the room
									type matched. We must verify that both occurred
									at the SAME time (i.e.,	on the same .ini line)
									(i.e., in the same GroupSet) before we set
									bGroupMatch[]. */

								if ((pTS->bGroupTileNum & uwBitMask) &&	/* tileset type matched */
									(bGroupRoomNum & uwBitMask))	/* room user type matched */
								{
									/* tileset type matched, room user type matched, but
										was it in the same GroupSet? */

									GroupSet *pGS;

									pGS = Head(&GroupDef[w]);

									while (!IsEOList(pGS))	/* Try to match each definition. */
									{
										if (pGS->uwTileLow <= (UWORD) pTS->UserType &&
											(UWORD) pTS->UserType <= pGS->uwTileHigh)
										{
											if (pGS->fRoom &&
												pGS->uwRoomLow <= (UWORD) prt->UserType &&
												prt->UserType <= (UWORD) pGS->uwRoomHigh)
											{
												pTUF[pp->Tile_ID-1].bGroupMatch |= uwBitMask;
												if (fFirstTimeInTS)
												{
													UpdateCharSize(uwCharWidth ? uwCharWidth : pTS->Width,
																		uwCharHeight ? uwCharHeight : pTS->Height);
													fFirstTimeInTS = FALSE;
												}
												break;
											}
										}
										pGS = Next(pGS);
									}
								}
								uwBitMask <<= 1;
							}
						}
					}
#else // !UseSparseArray
					for (wy = 0; wy < pl->ctily; wy++)
					{
						PlotType *pp;

						pp = ActivatePlotRowatWin(pl->rgrgplt, wy, winSrc);

						for (wx = 0; wx < pl->ctilx; wx++)
						{
							if (pp->TileSet_ID)
							{
								TileSetType *pTS;
								BOOL fFirstTimeInTS = TRUE;

								pTS = FAST_TILESET_PTR(pp->TileSet_ID);
								if (pTS->bGroupTileNum)		/* tileset in SOME group */
								{
									WORD w;
									UWORD uwBitMask = 1;

									/* If pTS->rgRoomUseFlags has changed (!= pxtLastWinDst)
										then write out old pTUF[], read in new pTUF[] */

									if (pTS->rgRoomUseFlags != pxtLastWinDst)
									{
										if (pxtLastWinDst)
										{
											UpdateXTRA(pxtLastWinDst);
										}
										pTUF = (TileUseFlag *) ActivateXTRAatWin(pTS->rgRoomUseFlags, winDst);
										pxtLastWinDst = pTS->rgRoomUseFlags;
									}

									/* Mark tile as appearing in a room. */

									pTUF[pp->Tile_ID-1].fInARoom = TRUE;

									/* Search through all group lists. */

									for (w = 0; w < wGroupCntMax; w++)
									{
										/* Make sure tileset type && room type both match
											for a particular GroupSet in the group. Up to
											this point we have determined (individually)
											if the tileset type matched and if the room
											type matched. We must verify that both occurred
											at the SAME time (i.e.,	on the same .ini line)
											(i.e., in the same GroupSet) before we set
											bGroupMatch[]. */

										if ((pTS->bGroupTileNum & uwBitMask) &&	/* tileset type matched */
											(bGroupRoomNum & uwBitMask))	/* room user type matched */
										{
											/* tileset type matched, room user type matched, but
												was it in the same GroupSet? */

											GroupSet *pGS;

											pGS = Head(&GroupDef[w]);

											while (!IsEOList(pGS))	/* Try to match each definition. */
											{
												if (pGS->uwTileLow <= (UWORD) pTS->UserType &&
													(UWORD) pTS->UserType <= pGS->uwTileHigh)
												{
													if (pGS->fRoom &&
														pGS->uwRoomLow <= (UWORD) prt->UserType &&
														prt->UserType <= (UWORD) pGS->uwRoomHigh)
													{
														pTUF[pp->Tile_ID-1].bGroupMatch |= uwBitMask;
														if (fFirstTimeInTS)
														{
															UpdateCharSize(uwCharWidth ? uwCharWidth : pTS->Width,
																				uwCharHeight ? uwCharHeight : pTS->Height);
															fFirstTimeInTS = FALSE;
														}
														break;
													}
												}
												pGS = Next(pGS);
											}
										}
										uwBitMask <<= 1;
									}
								}
							}
							pp++;
						}
						ReleasePlotRow(pl->rgrgplt, wy);
					}
#endif // !UseSparseArray
				}
				pl = Next(pl);
			}
		}
		prt = Next(prt);
	}

	if (pxtLastWinDst)
	{
		UpdateXTRA(pxtLastWinDst);
	}
} /* MarkRoomGroups */


/*********************************************************************
 *
 * MarkNotInRoomGroups
 *
 * SYNOPSIS
 *		static void MarkNotInRoomGroups(TileSpaceType *pTileSpace)
 *
 * PURPOSE
 *		Check for tiles that do not appear in a room,
 *		whose TileSet types match some GroupSet,
 *		and the GroupSet room match is set to none,
 *		(E.g., tileset=0,roomtype=none) and
 *		set the corresponding bit in bGroupMatch[].
 *
 * INPUT
 *
 *
 * EFFECTS
 *		All TileSetType->rgRoomUseFlags->TileUseFlag[].bGroupMatch set
 *		if both tileset type and tile does not appear in a room (and
 *		some GroupSet specifies appearing in no rooms).
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
static void MarkNotInRoomGroups(TileSpaceType *pTileSpace)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MarkNotInRoomGroups";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pTileSpace)
	{
		TileSetType *pTS = Head(&pTileSpace->TileSets);

		while (!IsEOList(pTS))
		{
			/* Only include tile sets that are some integral multiple
				of a character size. */

			if ((!uwCharWidth ||  (uwCharWidth &&  pTS->Width % uwCharWidth  == 0)) &&
				(!uwCharHeight || (uwCharHeight && pTS->Height % uwCharHeight == 0)))
			{
				TileUseFlag *pTUF;
				UWORD uw;
				BOOL fFirstTimeInTS = TRUE;

				pTUF = ActivateXTRA(pTS->rgRoomUseFlags);

				for (uw = 0; uw < pTS->TileCount; uw++)
				{
					WORD wg;
					UWORD uwBitMask = 1;

					for (wg = 0; wg < wGroupCntMax; wg++)
					{
						if ((pTS->bGroupTileNum & uwBitMask) && /* a tileset type match */
							!(pTUF[uw].bGroupMatch & uwBitMask) && /* no GroupSet match yet */
							!pTUF[uw].fInARoom)				/* tile does not appear in a room */
						{
							GroupSet *pGS;

							pGS = Head(&GroupDef[wg]);

							while (!IsEOList(pGS))	/* Try to match each definition. */
							{
								/* If tileset type matches and room type == none */

								if (pGS->uwTileLow <= (UWORD) pTS->UserType &&
									(UWORD) pTS->UserType <= pGS->uwTileHigh &&
									!pGS->fRoom)
								{
									pTUF[uw].bGroupMatch |= uwBitMask;
									if (fFirstTimeInTS)
									{
										UpdateCharSize(uwCharWidth ? uwCharWidth : pTS->Width,
															uwCharHeight ? uwCharHeight : pTS->Height);
										fFirstTimeInTS = FALSE;
									}
									break;
								}
								pGS = Next(pGS);
							}
						}
					}
				}
				UpdateXTRA(pTS->rgRoomUseFlags);
			}
			pTS = Next(pTS);
		}
	}
} /* MarkNotInRoomGroups */


/*********************************************************************
 *
 * CloseCharStorage
 *
 * SYNOPSIS
 *		static void CloseCharStorage(void)
 *
 * PURPOSE
 *		Free variables used to keep track of characters stored.
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
static void CloseCharStorage(void)
{

	WORD w;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CloseCharStorage";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (w = 0; w < wGroupCntMax; w++)
	{
		NodPxtBank *pnod;

		pnod = Head(&lisCharBanks[w]);

		while (!IsEOList(pnod))
		{
			FreeXTRA(pnod->pxtCharBank);
			pnod = Next(pnod);
		}
		EmptyList(&lisCharBanks[w]);
	}
	MEM_free(bmChar.data);
	MEM_free(bmXFlip.data);
	MEM_free(bmYFlip.data);
	MEM_free(bmXYFlip.data);

} /* CloseCharStorage */


/*********************************************************************
 *
 * OpenCharStorage
 *
 * SYNOPSIS
 *		static BOOL OpenCharStorage(void)
 *
 * PURPOSE
 *		Initialize variables use to keep track of characters stored.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if failed.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL OpenCharStorage(void)
{

	WORD w;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenCharStorage";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (w = 0; w < wGroupCntMax; w++)
	{
		uwCharMax[w] = 0;
		InitList(&lisCharBanks[w]);
	}

	if ((bmChar.data = MEM_malloc(uwBytesPerChar)) == NULL)
		return FALSE;

	if ((bmXFlip.data = MEM_malloc(uwBytesPerChar)) == NULL)
	{
		MEM_free(bmChar.data);
		return FALSE;
	}

	if ((bmYFlip.data = MEM_malloc(uwBytesPerChar)) == NULL)
	{
		MEM_free(bmChar.data);
		MEM_free(bmXFlip.data);
		return FALSE;
	}

	if ((bmXYFlip.data = MEM_malloc(uwBytesPerChar)) == NULL)
	{
		MEM_free(bmChar.data);
		MEM_free(bmXFlip.data);
		MEM_free(bmYFlip.data);
		return FALSE;
	}

	return TRUE;
} /* OpenCharStorage */


/*********************************************************************
 *
 * ByteMapFromByteMap
 *
 * SYNOPSIS
 *		static ByteMap *ByteMapFromByteMap(ByteMap *pSrcBM)
 *
 * PURPOSE
 *		Mask the bytemap through <ubCharacterANDMask>.
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
static ByteMap *ByteMapFromByteMap(ByteMap *pSrcBM)
{

	UWORD uw;
	UWORD uwWxH = pSrcBM->width * pSrcBM->height;
	UBYTE *pub;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ByteMapFromByteMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pub = pSrcBM->data;

	for (uw = 0; uw < uwWxH; uw++)
		*pub++ &= ubCharacterANDMask;

	return pSrcBM;
} /* ByteMapFromByteMap */


/*********************************************************************
 *
 * MatchChar
 *
 * SYNOPSIS
 *		BOOL MatchChar(ByteMap *pRefBM, ByteMap *pbm, UBYTE fFlip)
 *
 * PURPOSE
 *		See if pRefBM is the same char as pbm. If fFlip is non-zero,
 *		then check for flips.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns TRUE if pbm == pRefBM.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL MatchChar(ByteMap *pRefBM, ByteMap *pbm, UBYTE fFlip)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MatchChar";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((pRefBM->width != pbm->width) || (pRefBM->height != pbm->height))
	{
		return FALSE;
	}

	if (memcmp(pRefBM->data, pbm->data, pRefBM->width * pRefBM->height) == 0)
	{
		return TRUE;
	}
	if ((fFlip & bMergeXFlips) &&
		(memcmp(pRefBM->data, bmXFlip.data, pRefBM->width * pRefBM->height) == 0))
	{
		return TRUE;
	}
	if ((fFlip & bMergeYFlips) &&
		(memcmp(pRefBM->data, bmYFlip.data, pRefBM->width * pRefBM->height) == 0))
	{
		return TRUE;
	}
	if ((fFlip & bMergeXYFlips) &&
		(memcmp(pRefBM->data, bmXYFlip.data, pRefBM->width * pRefBM->height) == 0))
	{
		return TRUE;
	}

	return FALSE;
} /* MatchChar */


/*********************************************************************
 *
 * FindChar
 *
 * SYNOPSIS
 *		static UWORD FindChar(ByteMap *pbm, ListType *plis,
 *									UWORD uwCharMax, UBYTE fFlip)
 *
 * PURPOSE
 *		See if character already exists in the character set.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns 0 if found, else returns "signature" for pbm->data.
 *		"signature" is sum of all bytes in pbm->data. If "signature"
 *		is zero, it is coerced to one (guaranteed non-zero).
 *		We compare "signatures" for a quick check to see if two tiles
 *		are possibly the same tile.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static UWORD FindChar(ByteMap *pbm, ListType *plis, UWORD uwCharMax, UBYTE fFlip)
{

	NodPxtBank *pnodBank;
	UWORD uwChars = 0;
	UWORD uwSignature = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindChar";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fFlip & bMergeXFlips)
	{
		WORD wx, wy;

		UBYTE *pubSrc = pbm->data;
		UBYTE *pubDst = bmXFlip.data + pbm->width - 1;

		for (wy = 0; wy < pbm->height; wy++)
		{
			for (wx = 0; wx < pbm->width; wx++)
			{
				*pubDst-- = *pubSrc++;
			}
			pubDst += pbm->width * 2;
		}
	}
	if (fFlip & bMergeYFlips)
	{
		WORD wy;

		UBYTE *pubSrc = pbm->data;
		UBYTE *pubDst = bmYFlip.data + (pbm->height - 1) * pbm->width;

		for (wy = 0; wy < pbm->height; wy++)
		{
			memcpy(pubDst, pubSrc, pbm->width);

			pubDst -= pbm->width;
			pubSrc += pbm->width;
		}
	}

	/* Always compute bmXYFlips and uwSignature */

//	if (fFlip & bMergeXYFlips)
	{
		UWORD uw;
		UWORD uwWxH = pbm->width * pbm->height;

		UBYTE *pubSrc = pbm->data;
		UBYTE *pubDst = bmXYFlip.data + uwWxH - 1;

		for (uw = 0; uw < uwWxH; uw++)
		{
			uwSignature += *pubSrc;
			*pubDst-- = *pubSrc++;
		}
		if (uwSignature == 0)
			uwSignature = 1;
	}

	pnodBank = Head(plis);

	while (!IsEOList(pnodBank))
	{
		UBYTE *pub = ActivateXTRAatWin(pnodBank->pxtCharBank, winSrc);
		UWORD uw;
		UWORD uwEnd = min(uwCharsPerBank, uwCharMax - uwChars);

		for (uw = 0; uw < uwEnd; uw++)
		{
			/* Before checking every pixels and every flip,
				1st make sure that the signatures are the same. */

			if (uwSignature == *(UWORD *) (pub + sizeof(ByteMap)))
			{
				/* Bytemap headers are saved without pointing to data. Fix pointer. */

				((ByteMap*) pub)->data = pub + sizeofCharHeader;

				if (MatchChar((ByteMap *) pub, pbm, fFlip))
				{
					ReleaseXTRA(pnodBank->pxtCharBank);
					return 0;
				}
			}
			pub += uwBytesPerChar;
		}
		uwChars += uwEnd;

		ReleaseXTRA(pnodBank->pxtCharBank);
		pnodBank = Next(pnodBank);
	}
	return uwSignature;
} /* FindChar */


/*********************************************************************
 *
 * AddNewCharBank
 *
 * SYNOPSIS
 *		static BOOL AddNewCharBank(WORD wGroup)
 *
 * PURPOSE
 *		Allocate XTRA memory for a new character bank, add to end
 *		of lisCharBanks[wGroup].
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if unsuccessful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL AddNewCharBank(WORD wGroup)
{

	NodPxtBank *pnodBank;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddNewCharBank";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((pnodBank = CreateNode(sizeof(NodPxtBank), NULL)) == NULL)
		return FALSE;
	
	if ((pnodBank->pxtCharBank = AllocXTRA(uwBytesPerBank)) == NULL)
	{
		DeleteNode(pnodBank);
		return FALSE;
	}
	AddTail(&lisCharBanks[wGroup], pnodBank);

	return TRUE;
} /* AddNewCharBank */


/*********************************************************************
 *
 * AddChar
 *
 * SYNOPSIS
 *		static BOOL AddChar(WORD wGroup, ByteMap *pbm, UBYTE fMerge)
 *
 * PURPOSE
 *		Add character to group <wGroup>. If fMerge says to
 *		bMergeDuplicates, then see if char already exists
 *		before adding new character.
 *
 * INPUT
 *		WORD wGroup:	which group to affect.
 *		pbm:				pointer to character to add.
 *		UWORD *uw:		Users count of # characters added. Inc if we add a char.
 *
 * EFFECTS
 *		uwGroupCharCount[wGroup] is incremented if a new character is added.
 *
 * RETURN VALUE
 *		Return FALSE if unsuccessful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL AddChar(WORD wGroup, ByteMap *pbm, UBYTE fMerge)
{

	UBYTE *pub;
	UWORD uwSignature;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddChar";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fMerge & bMergeDuplicates)
	{
		if ((uwSignature = FindChar(pbm, &lisCharBanks[wGroup], uwCharMax[wGroup], fMerge)) == 0)
			return TRUE;
	}

	if (uwCharMax[wGroup] % uwCharsPerBank == 0)
	{
		if (!AddNewCharBank(wGroup))
			return FALSE;
	}
	pub = (UBYTE *) ActivateXTRAatWin(((NodPxtBank *) Tail(&lisCharBanks[wGroup]))->pxtCharBank, winSrc)
			+ (uwCharMax[wGroup] % uwCharsPerBank) * uwBytesPerChar;

	/* Save bytemap header, followed by signature, followed by bytemap->data. */

	memcpy(pub, pbm, sizeof(ByteMap));
	memcpy(pub+sizeof(ByteMap), &uwSignature, sizeof(UWORD));
	memcpy(pub+sizeofCharHeader, pbm->data, pbm->width * pbm->height);

	UpdateXTRA(((NodPxtBank *) Tail(&lisCharBanks[wGroup]))->pxtCharBank);
	uwCharMax[wGroup]++;
	uwGroupCharCount[wGroup]++;

	return TRUE;
} /* AddChar */


/*********************************************************************
 *
 * CollectChars
 *
 * SYNOPSIS
 *		static LONG CollectChars(TileSpaceType *pTileSpace)
 *
 * PURPOSE
 *		Collect all tiles from all tilesets defined in all groups.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Total number of characters defined, or -1 if failed.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static LONG CollectChars(TileSpaceType *pTileSpace)
{

	ULONG l = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CollectChars";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pTileSpace)
	{
		WORD wg;
		UWORD uwBitMask = 1;

		memset(uwGroupCharCount, 0, wGroupCntMax * sizeof(UWORD));

		for (wg = 0; wg < wGroupCntMax; wg++)	/* Process all groups */
		{
			if (!IsEOList(Head(&GroupDef[wg])))	/* Only groups with GroupSets */
			{
				WORD i;

				for (i = 0; i <= pTileSpace->HighestID; i++)
				{
					TileSetType *pTS = pTileSpace->TileSetArray[i];

					/* Only include valid tile sets that are some
						integral multiple of a character size. */

					if (pTS != NULL &&
						((!uwCharWidth ||  (uwCharWidth &&  pTS->Width % uwCharWidth  == 0)) &&
						(!uwCharHeight || (uwCharHeight && pTS->Height % uwCharHeight == 0))))
					{
						if (pTS->bGroupTileNum & uwBitMask)	/* Tileset type match..maybe some tiles to count */
						{
							TileUseFlag *pTUF;
							UWORD uw;
							UWORD uwXPixWidth, uwYPixHeight;
							PlotType pplt;

							pplt.TileSet_ID = i;
							pplt.Plot_Flags = 0;

							uwXPixWidth = uwCharWidth ? uwCharWidth : pTS->Width;
							uwYPixHeight = uwCharHeight ? uwCharHeight : pTS->Height;

							bmChar.width = uwXPixWidth;
							bmChar.height = uwYPixHeight;

							pTUF = (TileUseFlag *) ActivateXTRAatWin(pTS->rgRoomUseFlags, winDst);

							for (uw = 0; uw < pTS->TileCount; uw++)
							{
								if (pTUF[uw].bGroupMatch & uwBitMask)
								{
									WORD wXrtOffset, wYrtOffset;

									for (wYrtOffset = 0; wYrtOffset < pTS->Height; wYrtOffset += uwYPixHeight)
									{
										for (wXrtOffset = 0; wXrtOffset < pTS->Width; wXrtOffset += uwXPixWidth)
										{
											ByteMap *pbm;

											pplt.Tile_ID = uw+1;

											GetCharByteMap(&bmChar, &pplt,
																wXrtOffset, wYrtOffset);
											pbm = ByteMapFromByteMap(&bmChar);
											if (!AddChar(wg, pbm, bGroupFlags[wg]))
											{
												ReleaseXTRA(pTS->rgRoomUseFlags);
												return -1;
											}
										}
									}
								}
							}
							ReleaseXTRA(pTS->rgRoomUseFlags);
						}
					}
				}
			}
			uwBitMask <<= 1;
		}

		for (wg = 0; wg < wGroupCntMax; wg++)	/* sum all counts */
		{
			l += uwGroupCharCount[wg];
		}
	}
	return l;
} /* CollectChars */


/*********************************************************************
 *
 * ShowCharCount
 *
 * SYNOPSIS
 *		static void ShowCharCount(ULONG ul)
 *
 * PURPOSE
 *		Tell user how many characters we counted.
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
static void ShowCharCount(ULONG ul)
{

	char	rgch[256];

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowCharCount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	sprintf(rgch, "%lu characters counted.\n", ul);

	TellUser("Character Count", rgch);
} /* ShowCharCount */


/*********************************************************************
 *
 * CountTileUse
 *
 * SYNOPSIS
 *		static BOOL CountTileUse(TileSpaceType *pTileSpace,
 *										MapType *pMT, RoomType *pRT)
 *
 * PURPOSE
 *		Actually count the tiles.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if unsuccessful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL CountTileUse(TileSpaceType *pTileSpace,
								MapType *pMT, RoomType *pRT)
{

	LONG l;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CountTileUse";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	MarkTilesetGroups(pTileSpace);
	MarkRoomGroups(pMT, pRT);
	MarkNotInRoomGroups(pTileSpace);

	if (!OpenCharStorage())
		return FALSE;

	l = CollectChars(pTileSpace);
	if (l >= 0)
		ShowCharCount(l);

	CloseCharStorage();

	return TRUE;
} /* CountTileUse */


/*********************************************************************
 *
 * DoCountChars
 *
 * SYNOPSIS
 *		BOOL DoCountChars(TileSpaceType *pTileSpace,
 *								MapType *pMT, RoomType *pRT)
 *
 * PURPOSE
 *		Count tiles.
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
BOOL DoCountChars(TileSpaceType *pTileSpace, MapType *pMT, RoomType *pRT)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DoCountChars";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	PushClipValues(0, 0, 32767, 32767);
	DCC_TempSetPointer (BPI_WAIT_POINTER);
	if (pTileSpace)
	{
		uwBytesPerChar = 0;
		UpdateCharSize(uwCharWidth, uwCharHeight);

		if (!OpenTileUseFlags(pTileSpace))
		{
/**/		goto ABORT;
		}
		if (!CountTileUse(pTileSpace, pMT, pRT))
		{
/**/		goto ABORT;
		}

		CloseTileUseFlags(pTileSpace);
	}
	PopClipValues();
	return TRUE;

ABORT:
	TellUser(OOM, "Can't count characters.");
	PopClipValues();
	return FALSE;
} /* DoCountChars */

