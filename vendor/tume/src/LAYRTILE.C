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
 * LAYRTILE.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 06/07/93
 *   MODIFIED : 02/09/95
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Routines to deal with smart flipping.
 *
 *		See also LAYRTILI.C.
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include <echidna/listfunc.h>
#include "switches.h"

#include "tuglbl.h"
#include "layrtili.h"
#include "layrtile.h"						/* Verify function prototypes. */

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

ListType *plstLYT;
static UWORD cntLYT = 0;					/* count of LYT nodes allocated */

/* The following two variables are used by
	RememberRoomWithLayer() and SaveSmartFlipNewXY(). */

static UWORD layerLast = 0;				/* last layer smart flipped from */
static RoomType *prmLast = NULL;			/* last source room smart flipped to */

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * PLYTFromLayer
 *
 * SYNOPSIS
 *		LYT *PLYTFromLayer(UWORD layer)
 *
 * PURPOSE
 *		Returns the node that corresponds with layer. Create the node
 *		if needed. If the node already exists, clear it out before
 *		returning.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns NULL if OOM.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		Assumes plstLYT has been initialized by OpenLayerType().
*/
LYT *PLYTFromLayer(UWORD layer)
{
	LYT *plyt;
	UWORD lyr = 1;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PLYTFromLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	plyt = Head(plstLYT);

	while (!IsEOList(plyt))					/* See if node already exists */
	{
		if (layer == lyr)						/* Found layer node? */
			return plyt;

		plyt = Next(plyt);
		lyr++;
	}

	while (1)									/* Create nodes as needed */
	{
		if ( (plyt = CreateNode(sizeof(LYT), NULL)) == NULL)
			return NULL;						/* OOM */

		cntLYT = lyr;
		plyt->prmLast = NULL;
		plyt->TS_Type = NULL;
		plyt->cntType = 0;
		AddTail(plstLYT, plyt);

		if (layer == lyr)						/* Created enough nodes? */
			return plyt;

		lyr++;
	}

} /* PLYTFromLayer */


/*********************************************************************
 *
 * RememberRoomWithLayer
 *
 * PURPOSE
 *		Remember <prm> as the last source room displayed that is associated
 *		with <layer>.
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
 *
 *
 * SEE ALSO
 *
*/
static short RememberRoomWithLayer(UWORD layer, RoomType *prm, ULONG dx, ULONG dy)
{
	LYT *plyt;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RememberRoomWithLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ( (plyt = PLYTFromLayer(layer)) == NULL)
		return FALSE;							/* OOM */

	plyt->prmLast = prm;
	plyt->dxLast = dx;
	plyt->dyLast = dy;

	/* Save values for calls to SaveSmartFlipNewXY() */

	layerLast = layer;
	prmLast = prm;

	return TRUE;

} /* RememberRoomWithLayer */


/*********************************************************************
 *
 * RegisterTileType
 *
 * PURPOSE
 *		Register tiles of tileset type <TS_Type>, being displayed in
 *		room <prm>, at pixel coord <dx>, <dy> as the most recently
 *		selected source tiles for layer N (N is searched by this routine).
 *
 *		So this routine has to search the layers to see which layer
 *		<TS_Type> belongs to, and then set that layer node's previous room
 *		to <prm>.
 *
 *		This routine should be only called after selecting a brush from
 *		a source tile room.
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
void RegisterTileType(UWORD TS_Type, RoomType *prm, ULONG dx, ULONG dy)
{
	UWORD l;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RegisterTileType";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (l = 1; l < cntLYT; l++)
	{
		LYT *plyt = PLYTFromLayer(l);

		if (plyt)
		{
			short i;

			for (i = 0; i < plyt->cntType; i++)	/* check against all tile types */
			{
				if (plyt->TS_Type[i] == TS_Type)
				{
					RememberRoomWithLayer(l, prm, dx, dy);
					break;
				}
			}
		}
	}
} /* RegisterTileType */


/*********************************************************************
 *
 * PRMPreviousFromLayer
 *
 * PURPOSE
 *		Returns previous source room displayed for layer <layer>.
 *		If there wasn't a previous source room, returns NULL.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns NULL if no previous source room or OOM.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static RoomType *PRMPreviousFromLayer(UWORD layer, ULONG *pdx, ULONG *pdy)
{
	LYT *plyt;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PRMPreviousFromLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ( (plyt = PLYTFromLayer(layer)) == NULL)
		return NULL;							/* OOM */

	*pdx = plyt->dxLast;
	*pdy = plyt->dyLast;

	return plyt->prmLast;					/* return previous room or NULL if none */
} /* PRMPreviousFromLayer */


/*********************************************************************
 *
 * PRMNextFromLayer
 *
 * PURPOSE
 *		Search for any source room that is associated with <layer>.
 *		Remembers the source room so subsequent calls to
 *		PRMPreviousFromLayer() will return the same source room.
 *		This should allow you to make multiple calls to PRMNextFromLayer().
 *
 *		Returns NULL if unsuccessful.
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
static RoomType *PRMNextFromLayer(UWORD layer, ULONG *pdx, ULONG *pdy)
{
	LYT *plyt;
	short i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PRMNextFromLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ( (plyt = PLYTFromLayer(layer)) == NULL)
		return NULL;							/* OOM */

	for (i = 0; i < plyt->cntType; i++)	/* check against all tile types */
	{
		TileSetType *ptst = Head(&GlobalTileSpace->TileSets);

		if (ptst)
		{
			while (!IsEOList(ptst))			/* check against all tilesets */
			{
				if (ptst->UserType == plyt->TS_Type[i])
				{
					*pdx = 0;
					*pdy = ptst->wDisplayRow * ptst->Height;
					return ptst->SourceRoom;
				}
				ptst = Next(ptst);
			}
		}
	}
	return NULL;
} /* PRMNextFromLayer */


/*********************************************************************
 *
 * SaveSmartFlipNewXY
 *
 * PURPOSE
 *		Record the new location on screen after user scrolls around
 *		the room.
 *
 *		This routine should be called after the room scrolls.
 *
 *		This routine looks at the persistent variable <prmLast>
 *		to see if we are still scrolling around the same room before
 *		saving the tile position.
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
void SaveSmartFlipNewXY(RoomType *prm, ULONG dx, ULONG dy)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveSmartFlipNewXY";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (layerLast == 0)						/* not remembering any room */
		return;

	if (prm != prmLast)						/* no longer the same room */
	{
		layerLast = 0;
		return;
	}

	RememberRoomWithLayer(layerLast, prmLast, dx, dy);

} /* SaveSmartFlipNewXY */


/*********************************************************************
 *
 * SmartFlipPrw
 *
 * SYNOPSIS
 *		void SmartFlipPrw(RoomWindowType *prw)
 *
 * PURPOSE
 *		Use Smart Flip to toggle <prw> to the other "pane".
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
void SmartFlipPrw(RoomWindowType *prw)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SmartFlipPrw";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prw)
	{
		TogglePrw(prw);
		if (IsEdit(prw->OtherRoom->Room))
		{
			RoomType *prm;
			ULONG dx, dy;

			prm = PRMPreviousFromLayer(prw->OtherRoom->Room->FloorNumber,
												&dx, &dy);
			if (!prm)
			{
				prm = PRMNextFromLayer(prw->OtherRoom->Room->FloorNumber,
												&dx, &dy);
			}
			if (!prm)
				return;							/* No smart flip, just do normal flip */

			/* Smart flip to appropriate room */

			prw->CurrentRoom->Room = prm;
			prw->CurrentRoom->dxSrc = dx;
			prw->CurrentRoom->dySrc = dy;

			RememberRoomWithLayer(prw->OtherRoom->Room->FloorNumber,
											prm, dx, dy);
		}
		else
		{
			layerLast = 0;						/* No longer on same source room */

			/* Do any smart flip from source to edit rooms here. */
		}
	}
} /* SmartFlipPrw */


/*********************************************************************
 *
 * RemoveSmartFlipRoom
 *
 * SYNOPSIS
 *		void RemoveSmartFlipRoom(RoomType *prm)
 *
 * PURPOSE
 *		Search all structures, clear any reference to <prm>
 *		(as it is about to be deleted).
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
void RemoveSmartFlipRoom(RoomType *prm)
{
	UWORD l;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RemoveSmartFlipRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (l = 1; l < cntLYT; l++)			/* Search all layer tile nodes */
	{
		LYT *plyt = PLYTFromLayer(l);

		if (plyt && plyt->prmLast == prm)
		{
			plyt->prmLast = NULL;
		}
	}

	if (prmLast == prm)
		layerLast = 0;
} /* RemoveSmartFlipRoom */

