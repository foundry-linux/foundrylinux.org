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
 * ROOMIO.C
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 09/04/89 
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
 *		Load, save, create and delete rooms.
 *
 * HISTORY
 *		09/04/89 (RGM) - Created.
 *		04/05/93 (dcc) - use rectplot.h header.
 *		04/05/93 (dcc) - support multiple plot rows per XTRAPntr.
 *		07/12/93 (dcc) - add support for tile usage counts to Undo.
 *		11/05/93 (dcc) - fix UNDO breaking if room widened.
 *		04/05/94 (dcc) - fix FirstEditRoom() & FirstSourceRoom
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include <string.h>

#if LATTICE
#include <proto/exec.h>
#endif

#include "rectplot.h"
#include "tuglbl.h"
#include "grid.h"
#include "collect.h"
#include "layrtili.h"
#include "layrtile.h"
#include "histogrm.h"
#include "switch1.h"
#include "szerror.h"
#include "assert.h"
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray

/**************************** C O N S T A N T S ***************************/

#define	CLEAR_LAYERS		(0x01)
#define	RESET_UNDO		(0x02)

#define ADD_LAYER			(0x00)
#define INSERT_LAYER		(0x80)
#define DONTCLEAR_LAYER	(0x01)

/* Constants for parameter fDirection in function RectRoomToRoom(). */

#define	COPYROOMRECT		0
#define SWAPROOMRECT		1

/* Note that wPlotXTRAMax must be <= wAllocXTRAMax ! */

#define	wPlotXTRAMax		16384U


/******************************** T Y P E S *******************************/


/****************************** E X T E R N S *****************************/


/****************************** G L O B A L S *****************************/

WORD wDefaultGridXWidth = 1;
WORD wDefaultGridYHeight = 1;
WORD wDefaultGridXOrigin = 0;
WORD wDefaultGridYOrigin = 0;
BOOL fDefaultUseGrid = FALSE;

#if NEWTUME
WORD cDefaultGuideLines = cDefaultGuideLinesInit;
LONG *pcenDefaultGuideA;
LONG *pcenDefaultGuideB;
LONG *pcenDefaultGuideDC;
LONG *pcenDefaultGuideXOrigin;
LONG *pcenDefaultGuideYOrigin;
BOOL *fDefaultShowGuide;
#else
WORD wDefaultGuideXWidth = 2;
WORD wDefaultGuideYHeight = 2;
WORD wDefaultGuideXOrigin = 0;
WORD wDefaultGuideYOrigin = 0;
#endif
BOOL fDefaultGuideVisible = FALSE;

WORD wDefaultZoomIndex = 0;

/******************************* L O C A L S ******************************/

#if UseSparseArray
static RoomType *ReMakeUndoRoom(RoomType *undo_room, RoomType *prmModel);
#else // UseSparseArray
static RoomType *ReMakeUndoRoom (
	RoomType	*undo_room,
	int		width,
	int		height,
	int		depth
);
#endif // UseSparseArray

/******************************* M A C R O S ******************************/

#if __AMIGAOS__

#if LATTICE
#define CopyPlots(s, d, size) CopyMemQuick (((char *) s),((char *) d),((long) size))
#else
#define CopyPlots(s, d, size) CopyMemQuick (((ULONG *) s),((ULONG *) d),((ULONG) size))
#endif

#else

#define CopyPlots(s, d, size) \
		memcpy ( (void *) (d), (void *) (s), (unsigned int) (size) )

#endif

/***************************** R O U T I N E S ****************************/


/*-=-=-=-=-=-=-=-=-=-=-=-=-=- LOCAL ROUTINES -=-=-=-=-=-=-=-=-=-=-=-=-=-*/


/**************************************************************************
 *
 * FreePlot
 *
 * PURPOSE
 *		Frees actual memory used by plot array.
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
 *		10/02/92 Friday (dcc) - set pointer to NULL so subsequent calls
 *							to FreePlot() doesn't crash.
 *		04/05/93 Monday (dcc) - support multiple plot rows per XTRAPntr.
 *
 * SEE ALSO
 *
*/
#if PLOTARRAY
static void FreePlot (PlotType **Plot)
#else
static void FreePlot (RGRGPLT *prgrgplt)
#endif
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FreePlot";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/


#if PLOTARRAY
	if (*Plot)
	{
		MEM_free (*Plot);
		*Plot = NULL;							/* disable subsequent calls to FreePlot */
	}
#else
	if (prgrgplt->mpYtMpXtPt)
	{
		WORD iyt;
		WORD wXTRACnt = (prgrgplt->PlotHeight+prgrgplt->wRowsPerXTRA-1) /
						prgrgplt->wRowsPerXTRA;

		for (iyt = 0; iyt < wXTRACnt; iyt++)
		{
			if (prgrgplt->mpYtMpXtPt[iyt])
			{
				FreeXTRA(prgrgplt->mpYtMpXtPt[iyt]);
			}
		}

		MEM_free(prgrgplt->mpYtMpXtPt);
		prgrgplt->mpYtMpXtPt = NULL;		/* disable subsequent calls to FreePlot */
	}
#endif

} /* FreePlot */


/**************************************************************************
 *
 * AllocatePlot
 *
 * PURPOSE
 *		Allocates actual memory used by plot array. Fill out RGRGPLT
 *		structure pointed to by <prgrgplt>.
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
 *		TRUE if successful.
 *
 * HISTORY
 *		04/05/93 Monday (dcc) - support multiple plot rows per XTRAPntr.
 *
 * SEE ALSO
 *
*/
static BOOL AllocatePlot(RGRGPLT *prgrgplt, WORD width, WORD height)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AllocatePlot";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/


	prgrgplt->PlotWidth = 0;
	prgrgplt->PlotHeight = 0;
	if (width != 0 && height != 0 && ((long) width * (long) PLOTSIZE < (long) wAllocXTRAMax))
	{
		WORD	iyt;
		WORD wRowsPerXTRA = 1;
		WORD wXTRACnt = height;

		if ((long) width * PLOTSIZE <= wPlotXTRAMax)
		{
			wRowsPerXTRA = (WORD) ((long) wPlotXTRAMax / ((long) width * PLOTSIZE));
			wXTRACnt = (height+wRowsPerXTRA-1) / wRowsPerXTRA;
		}

		if ((prgrgplt->mpYtMpXtPt = MEM_calloc(wXTRACnt, sizeof(MPXTPT))) == 0)
		{
			return FALSE;
		}

		prgrgplt->PlotWidth = width;
		prgrgplt->PlotHeight = height;
		prgrgplt->wRowsPerXTRA = wRowsPerXTRA;

		for (iyt = 0; iyt < wXTRACnt; iyt++)
		{
			PlotType *pPt;

			if ((prgrgplt->mpYtMpXtPt[iyt] =
				AllocXTRA(width*min(wRowsPerXTRA, height)*PLOTSIZE))==NULL)
			{
/**/			goto ABORT;
			}

			pPt = (PlotType *) ActivateXTRAatWin(prgrgplt->mpYtMpXtPt[iyt], winDst);

			memset(pPt, 0, width * min(wRowsPerXTRA, height) * PLOTSIZE);

			UpdateXTRA(prgrgplt->mpYtMpXtPt[iyt]);

			height -= wRowsPerXTRA;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}

/*========================================================================*/
ABORT:

	FreePlot(prgrgplt);

	return FALSE;

} /* AllocatePlot */


/*********************************************************************
 *
 * ExpandPlot
 *
 * PURPOSE
 *		Make a new RGRGPLT of size wNewWidth, wNewHeight, copy
 *		information from old RGRGPLT to the new one, and free
 *		the old one.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		New RGRGPLT, or NULL if unsuccessful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL ExpandPlot(RGRGPLT *prgrgplt, WORD wNewWidth, WORD wNewHeight)
{
	RGRGPLT rgrgpltNew;
#if 0
	WORD i;
#endif

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ExpandPlot";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* If we are not making it wider nor taller, then just return TRUE */

	if (wNewWidth <= prgrgplt->PlotWidth && wNewHeight <= prgrgplt->PlotHeight)
	{
		return TRUE;
	}
	/* If one row will not fit into 32K, fail */

#if (!__MSDOS32X__)
	if ((long) wNewWidth * (long) PLOTSIZE >= (long) wAllocXTRAMax)
		return FALSE;
#endif

	/* If new width == 0 or new height == 0, fail */

	if (wNewWidth == 0 || wNewHeight == 0)
		return FALSE;

#if 0
	/* Allocate new RGRGPLT */
{
	WORD wRowsPerXTRA = (long) wPlotXTRAMax / ((long) wNewWidth * PLOTSIZE);
	WORD wXTRACnt = (wNewHeight+wRowsPerXTRA-1) / wRowsPerXTRA;

	if ((rgrgpltNew.mpYtMpXtPt = MEM_calloc(wNewHeight, sizeof(MPXTPT))) == 0)
	{
		return FALSE;
	}

	rgrgpltNew.PlotWidth = wNewWidth;
	rgrgpltNew.PlotHeight = wNewHeight;
	rgrgpltNew.wRowsPerXTRA = wRowsPerXTRA;
}
	/* Allocate new rows. */

	for (i = prgrgplt->PlotHeight; i < wNewHeight; i++)
	{
		if ((rgrgpltNew.mpYtMpXtPt[i] = AllocXTRA(wNewWidth * PLOTSIZE)) != NULL)
		{
			PlotType	*pPtDst = (PlotType *)
				ActivateXTRAatWin(rgrgpltNew.mpYtMpXtPt[i], winDst);

			/* Set new row to NULL tiles. */

			memset(pPtDst, 0, wNewWidth * PLOTSIZE);

			UpdateXTRA(rgrgpltNew.mpYtMpXtPt[i]);
		}
		else
		{
			FreePlot(&rgrgpltNew);
			return FALSE;
		}
	}

	/*	If we are not making it wider, then we must be making it taller.
		Just copy old rows to new, and make new rows blank. */

	if (wNewWidth <= prgrgplt->PlotWidth)
	{
		/* Copy old rows to new */

		for (i = 0; i < prgrgplt->PlotHeight; i++)
		{
			rgrgpltNew.mpYtMpXtPt[i] = prgrgplt->mpYtMpXtPt[i];
			prgrgplt->mpYtMpXtPt[i] = NULL;
		}
	}

	/*	We're making it wider. Maybe taller, maybe shorter. */
	else
	{
		WORD wHeight = min(wNewHeight, prgrgplt->PlotHeight);
		WORD wDeltaWidth = wNewWidth - prgrgplt->PlotWidth;

		for (i = 0; i < wHeight; i++)
		{
			if ((rgrgpltNew.mpYtMpXtPt[i] = AllocXTRA(wNewWidth * PLOTSIZE)) != NULL)
			{
				PlotType	*pPtDst = (PlotType *)
					ActivateXTRAatWin(rgrgpltNew.mpYtMpXtPt[i], winDst);

				ReadXTRA(prgrgplt->mpYtMpXtPt[i], 0, prgrgplt->PlotWidth * PLOTSIZE, pPtDst);

				/* Set rest of row to NULL tiles. */

				memset(pPtDst + prgrgplt->PlotWidth, 0, wDeltaWidth * PLOTSIZE);

				UpdateXTRA(rgrgpltNew.mpYtMpXtPt[i]);

				FreeXTRA(prgrgplt->mpYtMpXtPt[i]);
				prgrgplt->mpYtMpXtPt[i] = NULL;
			}

			/*	Doh. We've run out of memory.
				Just make old RGRGPLT point at new (wider) rows.
				Though the resulting RGRGPLT will have some
				wider rows than others, this shouldn't matter. */
			else
			{
				WORD j;

				for (j = 0; j < i; j++)
				{
					prgrgplt->mpYtMpXtPt[j] = rgrgpltNew.mpYtMpXtPt[j];
					rgrgpltNew.mpYtMpXtPt[j] = NULL;
				}
				MEM_free(rgrgpltNew.mpYtMpXtPt);
				return FALSE;
			}
		}
	}
	FreePlot(prgrgplt);
#endif
{
	WORD iy;
	WORD wPlotsToCopy;
	WORD wRowsToCopy;

	if (!AllocatePlot(&rgrgpltNew, wNewWidth, wNewHeight))
		return FALSE;

	wPlotsToCopy = min(prgrgplt->PlotWidth, wNewWidth);
	wRowsToCopy = min(prgrgplt->PlotHeight, wNewHeight);

	for (iy = 0; iy < wRowsToCopy; iy++)
	{
		PlotType *pplt;

		pplt = ActivatePlotRowatWin(*prgrgplt, iy, winDst);
		WriteNPlotXY(pplt, wPlotsToCopy, rgrgpltNew, 0, iy);
		ReleasePlotRow(*prgrgplt, iy);
	}
	FreePlot(prgrgplt);
}
	*prgrgplt = rgrgpltNew;
	return TRUE;

} /* ExpandPlot */


/**************************************************************************
 *
 * AddLayer
 *
 * PURPOSE
 *		To make a new layer and add it to a list of layers.
 *
 *		May or may not inform user of OOM...(oops).
 *
 * INPUT
 *		RoomType *prm		: room that layers belong to (may be NULL)
 *		LayerType *play	: if al_flag & INSERT_LAYER, insert B4 <play>,
 *							else insert after <play>
 *		int width			: tiles wide to create layer
 *		int height		: tiles high to create layer
 *		WORD al_flag		: check DONTCLEAR_LAYER | INSERT_LAYER bits
#if UseSparseArray
 *		BOOL fMakeSparse	: if TRUE, make sparse array
#endif // UseSparseArray
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns pointer to layer, or NULL if OOM.
 *
 * HISTORY
 *		10/04/89 (RGM) - Created from split-up of AddTileSetLayer.
 *		10/07/89 (RGM) - Changed from room specific to generic.
 *		10/12/89 (RGM) - Made to return a pointer 'stead of BOOL, and to add
 *						NULL plot layers.
 *		04/13/91 (RGM) - Made to take a room so's I can tell the
 *						layer who its parent is (needed for 'improved undo').
 *		04/06/93 (dcc) - BUG FIX: call RemTail() if needed when OOM
 *		05/10/93 (dcc) - add support for INSERT_LAYER
 *		01/28/94 (dcc) - take room AND layer as input arguments
 *		02/20/94 (dcc) - add ->dtilx and ->dtily
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
#if UseSparseArray
static LayerType *AddLayer(RoomType *prm, LayerType *play,
						int width, int height,
						WORD al_flag, BOOL fMakeSparse)
#else // !UseSparseArray
static LayerType *AddLayer (
	RoomType	*prm,
	LayerType	*play,
	WORD		width,
	WORD		height,
	WORD		al_flag
)
#endif // !UseSparseArray
{
	BOOL		clear_flag;
	LayerType	*newlayer_ptr	= NULL;
#if PLOTARRAY
	int			tiles;
#endif

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddLayer";
#endif
#if REQUIRE

	if ((! width) || (! height))
	{
		printf ("REQUIRE failure in AddLayer()\n");
/**/	goto ABORT;
	}
#endif
/*-------------------------------------------------------------------------*/

	clear_flag = ((al_flag & DONTCLEAR_LAYER) == 0);

#if PLOTARRAY
	tiles = width * height;
#endif

	if ( (newlayer_ptr = MEM_calloc (1, LAYERSIZE)) == NULL)
/**/		goto ABORT;

	newlayer_ptr->ctilx = width;
	newlayer_ptr->ctily = height;
	newlayer_ptr->dtilx = 0;
	newlayer_ptr->dtily = 0;

#if PLOTARRAY
	if (tiles) {
		if (clear_flag) {		/* this is a displayable room */
			if (! (newlayer_ptr->Plot = MEM_calloc (tiles, PLOTSIZE)) ) {
/**/				goto ABORT;
			}

		}
		else {					/* this is a scratchpad room */
			if (! (newlayer_ptr->Plot = MEM_malloc (tiles * PLOTSIZE)) ) {
/**/				goto ABORT;
			}

		}
	}
	else {
		newlayer_ptr->Plot			= NULL;
	}
#else
#if UseSparseArray
	if (width != 0 && height != 0)
	{
		if (fMakeSparse)
		{
			newlayer_ptr->L_Flags |= LAYER_SPARSEARRAY;
			if ((newlayer_ptr->p.pspa = MakeNewSparseArray()) == NULL)
/**/			goto ABORT;
		}
		else
		{
			if (!AllocatePlot(&newlayer_ptr->p.rgrgplt, width, height))
/**/			goto ABORT;
		}
	}
	else
	{
		newlayer_ptr->p.rgrgplt.mpYtMpXtPt	= NULL;
		newlayer_ptr->p.rgrgplt.PlotWidth	= 0;
		newlayer_ptr->p.rgrgplt.PlotHeight	= 0;
	}
#else // !UseSparseArray
	if (width != 0 && height != 0)
	{
		if (!AllocatePlot(&newlayer_ptr->rgrgplt, width, height))
/**/		goto ABORT;
	}
	else
	{
		newlayer_ptr->rgrgplt.mpYtMpXtPt	= NULL;
		newlayer_ptr->rgrgplt.PlotWidth	= 0;
		newlayer_ptr->rgrgplt.PlotHeight	= 0;
	}
#endif // !UseSparseArray
#endif

	newlayer_ptr->Parent_Room	= prm;		/* for 'improved undo'. */
	if (al_flag & INSERT_LAYER)
	{
		InsertBefore(play, newlayer_ptr);
	}
	else
	{
		InsertAfter(play, newlayer_ptr);
	}

	/* check to see if this was a change to a 'displayable' room */
	/* and (re)make the undo room to fit it */

	if (clear_flag)
	{
		if (prm)
		{
#if UseSparseArray
			UndoRoom = ReMakeUndoRoom(UndoRoom, prm);
#else // UseSparseArray
			ListType *plst = &prm->Layers;
			LayerType *layer_ptr = NULL;

			WORD	depth;

			depth = 0;
			layer_ptr = Head (plst);
			while (! IsEOList (layer_ptr))
			{
				depth ++;
				layer_ptr = Next (layer_ptr);
			}

			UndoRoom	= ReMakeUndoRoom (UndoRoom, width, height, depth);
#endif // UseSparseArray
			if (!UndoRoom)		//dcc
			{
				Remove(newlayer_ptr);
/**/			goto ABORT;
			}
		}
	}
	else {
		InvalidateUndo ();
	}

	return (newlayer_ptr);

/*========================================================================*/
ABORT:

	if (newlayer_ptr)
	{
		DeAllocateLayer (newlayer_ptr);
		newlayer_ptr = NULL;
	}

	return (NULL);

} /* AddLayer */


#if NEWTUME
/*********************************************************************
 *
 * DeAllocateRoomGuides
 *
 * PURPOSE
 *		Deallocate memory allocated for room guides.
 *
 * INPUT
 *		prm			: room to deallocate guides from.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		09/14/93 Tuesday (dcc) - created.
 *
*/
static void DeAllocateRoomGuides(RoomType *prm)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeAllocateRoomGuides";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SAFEFREE(prm->pcenGuideA);
	SAFEFREE(prm->pcenGuideB);
	SAFEFREE(prm->pcenGuideDC);
	SAFEFREE(prm->pcenGuideXOrigin);
	SAFEFREE(prm->pcenGuideYOrigin);
	SAFEFREE(prm->fShowGuide);
	prm->cGuideLines = 0;

} /* DeAllocateRoomGuides */


/*********************************************************************
 *
 * ReAllocateRoomGuides
 *
 * PURPOSE
 *		Reallocate room <prm> guides so it will hold <cGuides>.
 *
 * INPUT
 *		prm			: pointer to room to reallocate guides
 *		cGuides		: number of guides to allocate memory for
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		
 *
 * HISTORY
 *		09/14/93 Tuesday (dcc) - created.
 *
*/
BOOL ReAllocateRoomGuides(RoomType *prm, short cGuides)
{
	short bytes = cGuides * sizeof(LONG);

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReAllocateRoomGuides";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((prm->pcenGuideA = MEM_realloc(prm->pcenGuideA, bytes)) == NULL)
/**/	goto ABORT;
	if ((prm->pcenGuideB = MEM_realloc(prm->pcenGuideB, bytes)) == NULL)
/**/	goto ABORT;
	if ((prm->pcenGuideDC = MEM_realloc(prm->pcenGuideDC, bytes)) == NULL)
/**/	goto ABORT;
	if ((prm->pcenGuideXOrigin = MEM_realloc(prm->pcenGuideXOrigin, bytes)) == NULL)
/**/	goto ABORT;
	if ((prm->pcenGuideYOrigin = MEM_realloc(prm->pcenGuideYOrigin, bytes)) == NULL)
/**/	goto ABORT;
	if ((prm->fShowGuide = MEM_realloc(prm->fShowGuide, cGuides * sizeof(BOOL))) == NULL)
/**/	goto ABORT;

	prm->cGuideLines = cGuides;
	return TRUE;

ABORT:
	DeAllocateRoomGuides(prm);

	return FALSE;
} /* ReAllocateRoomGuides */


/*********************************************************************
 *
 * SetRoomGuideFromDefault
 *
 * PURPOSE
 *		Copy guide values from default values to <prm>.
 *
 * INPUT
 *		prm			: pointer to room to set
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		FALSE if OOM.
 *
 * HISTORY
 *		09/14/93 Tuesday (dcc) - created.
 *
*/
static BOOL SetRoomGuideFromDefault(RoomType *prm)
{
	short bytes = cDefaultGuideLines * sizeof(LONG);

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetRoomGuideFromDefault";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!ReAllocateRoomGuides(prm, cDefaultGuideLines))
		return FALSE;

	memcpy(prm->pcenGuideA, pcenDefaultGuideA, bytes);
	memcpy(prm->pcenGuideB, pcenDefaultGuideB, bytes);
	memcpy(prm->pcenGuideDC, pcenDefaultGuideDC, bytes);
	memcpy(prm->pcenGuideXOrigin, pcenDefaultGuideXOrigin, bytes);
	memcpy(prm->pcenGuideYOrigin, pcenDefaultGuideYOrigin, bytes);
	memcpy(prm->fShowGuide, fDefaultShowGuide, cDefaultGuideLines * sizeof(BOOL));

	return TRUE;
} /* SetRoomGuideFromDefault */
#endif


/**************************************************************************
 *
 * CreateRoom
 *
 * PURPOSE
 *		Create a room.
 *
 *		If OOM, informs user w/ a dialog box.
 *
 * USAGE
 *
 *
 * INPUT
 *		int width			: tiles wide to create 1st layer
 *		int height		: tiles high to create 1st layer
 *		UWORD create_type	: check CLEAR_LAYERS | RESET_UNDO
 *		UWORD room_flags	: set the new room's <Flags> with this UWORD
#if UseSparseArray
 *		BOOL fMakeSparse	: if TRUE, make sparse array
#endif // UseSparseArray
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns pointer to room, or NULL if OOM.
 *
 * HISTORY
 *		01/08/91 (RGM) - Created to merge a couple of seperate functions
 *						into one (as they should be).
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
#if UseSparseArray
static RoomType *CreateRoom(int width, int height,
					UWORD create_type, UWORD room_flags, BOOL fMakeSparse)
#else // !UseSparseArray
static RoomType *CreateRoom (
	WORD	width,
	WORD	height,
	UWORD	create_type,
	UWORD	room_flags
)
#endif // !UseSparseArray
{
	RoomType	*newroom	= NULL;
	RWInitsType	*rwinits	= NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/*************************************/
	/* allocate the new room's structure */

	if ( (newroom = MEM_calloc (1, ROOMSIZE)) == NULL)
/**/		goto ABORT;

#if NEWTUME
	if (!SetRoomGuideFromDefault(newroom))
/**/	goto ABORT;

	newroom->Flags		= room_flags;
#else

	newroom->Flags		= room_flags;

	newroom->wGuideXWidth	= (wGetDefaultGuideX() > 0) ? wGetDefaultGuideX() : 1;
	newroom->wGuideYHeight	= (wGetDefaultGuideY() > 0) ? wGetDefaultGuideY() : 1;
	newroom->wGuideXOrigin	= wGetDefaultGuideXOrigin();
	newroom->wGuideYOrigin	= wGetDefaultGuideYOrigin();
#endif

	newroom->wGridXWidth	= (wGetDefaultGridX() > 0) ? wGetDefaultGridX() : 1;
	newroom->wGridYHeight	= (wGetDefaultGridY() > 0) ? wGetDefaultGridY() : 1;
	newroom->wGridXOrigin	= wGetDefaultGridXOrigin();
	newroom->wGridYOrigin	= wGetDefaultGridYOrigin();

	newroom->wZoomIndex		= wGetDefaultZoomIndex();

	if (fGetDefaultUseGrid())
		SetGridOn(newroom);
	if (fGetDefaultGuideVisible())
		SetGuideVisible(newroom);

#if NEWTUME
	if (IsGuideVisible(newroom))
	{
		newroom->fShowGuide[0] = TRUE;
		newroom->fShowGuide[1] = TRUE;
	}
	else
	{
		newroom->fShowGuide[0] = FALSE;
		newroom->fShowGuide[1] = FALSE;
	}
	Assert((newroom->fShowGuide[0] != 0) == (IsGuideVisible(newroom) != 0));
	Assert((newroom->fShowGuide[1] != 0) == (IsGuideVisible(newroom) != 0));
#endif

	/********************/
	/* Set up the lists */

	InitList (&(newroom->Layers));
	InitList (&(newroom->Items));
	InitList (&(newroom->Displays));
	InitList (&(newroom->RWInits));

	/**************************************/
	/* set up a new node for each         */
	/* roomwindow in GlobalRoomWindowList */

	if ( (rwinits = MEM_calloc (1, RWINITSSIZE)) == NULL)
/**/		goto ABORT;
	
	rwinits->RW = GlobalRoomWindow;

	AddTail (&(newroom->RWInits), rwinits);
	rwinits = NULL;

	{
		WORD	al_flags;

		if (create_type & CLEAR_LAYERS) {
			al_flags = ADD_LAYER;
		}
		else {
			al_flags = ADD_LAYER | DONTCLEAR_LAYER;
		}

#if UseSparseArray
		if (! AddLayer(newroom, Tail(&newroom->Layers), width, height, al_flags, fMakeSparse))
#else // !UseSparseArray
		if (! AddLayer(newroom, Tail(&newroom->Layers), width, height, al_flags))
#endif // !UseSparseArray
/**/		goto ABORT;
	}

	newroom->FloorLayer		= Head (&(newroom->Layers));
	newroom->FloorNumber	= 1;
	newroom->LayerCount		= 1;

	/****************************************/
	/* we're successful in creating a room, */
	/* give this room a new ID number       */

	/* newroom->R_id = GlobalRoomID; */
	/* GlobalRoomID += 1; */

	if (create_type & RESET_UNDO)
	{
#if UseSparseArray
		UndoRoom	= ReMakeUndoRoom(UndoRoom, newroom);
#else // !UseSparseArray
		UndoRoom	= ReMakeUndoRoom (UndoRoom, width, height, 1);
#endif // !UseSparseArray
		if (!UndoRoom)		//dcc
		{
/**/		goto ABORT;
		}
	}

	return (newroom);

/*-----------------------------------------------------------------------*/
ABORT:

	TellUser (OOM, "Unable to create room.");

	SAFEFREE (rwinits);
	SAFEFREE (newroom);

	return (NULL);

} /* CreateRoom */

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


/*********************************************************************
 *
 * PlaceXYFromTSTXY
 *
 * PURPOSE
 *		Convert tileset input position coordinates <xorig>, <yorig>
 *		to display coordinates and place results in <*xdst>, <*ydst>.
 *
 *		This mapping used to be one-to-one, but now we need to
 *		deal with ctilxSkip, ctilxDispO, and ctilxDispE.
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
 *		12/15/93 Wednesday (dcc) - created.
 *
*/
static void PlaceXYFromTSTXY(TileSetType *ptst,
							int xorig, int yorig, int *xdst, int *ydst)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PlaceXYFromTSTXY";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	*xdst = xorig * ptst->ctilxSkip +
		   (yorig / 2) * ptst->ctilxDispE + ((yorig+1) / 2) * ptst->ctilxDispO;
	*ydst = (yorig / 2) * ptst->ctilyDispE + ((yorig+1) / 2) * ptst->ctilyDispO;

} /* PlaceXYFromTSTXY */


#if PLOTARRAY
/**************************************************************************
 *
 * InitTileSetLayer
 *
 * PURPOSE
 *		Place tiles in a source room, either packed or 'as found'.
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
 *		10/04/89 Wednesday - Created from split-up of AddTileSetLayer. (RGM)
 *		03/07/91 Thursday (RGM) - Added offset support..
 *
 * SEE ALSO
 *
*/
void InitTileSetLayer (
	RoomType		*room,
	TileSetType		*tileset,
	long			start_offset
)
{
	PlotType		*nextplot_ptr;
#if __AMIGAOS__
	TileType		*tile_ptr;
#endif/*__AMIGAOS__*/
	LayerType		*layer;
	WORD			room_width;
#if __MSDOS__
	UWORD		i = 0;
	WORD			OriginalCol = 0;
	WORD			OriginalRow = 0;
#endif/*__MSDOS__*/
	int			DisplayCol;
	int			DisplayRow;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitTileSetLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room_width	= room->Width;
	layer		= Head (&(room->Layers));

	/***********************************/
	/* initialize the layer with tiles */

#if __AMIGAOS__
	tile_ptr = Head (&(tileset->Tiles));

	while (! IsEOList (tile_ptr))
	{
		PlaceXYFromTSTXY(tileset, tile_ptr->Original_Column, tile_ptr->Original_Row,
						&DisplayCol, &DisplayRow);

		nextplot_ptr	= layer->Plot;
		nextplot_ptr	+= start_offset;
		nextplot_ptr	+= DisplayCol;
		nextplot_ptr	+= DisplayRow * room_width;
#elif __MSDOS__
	for (i = 0; i < tileset->TileCount; i++)
	{
		if ((tileset->Flags & GRID_TYPE_MODIFIER) != TILESET_COMPOSITE)
		{
			WORD w;

			ReadXTRA(tileset->pxtrDelCol[i/deltasPerCol],
					(i%deltasPerCol)*sizeof(WORD), sizeof(WORD), &w);
			OriginalCol += w;
		}
		else
		{
			OriginalCol += i ? 1 : 0;
		}
		OriginalRow += OriginalCol / tileset->MaxAcross;
		OriginalCol  = OriginalCol % tileset->MaxAcross;

		PlaceXYFromTSTXY(tileset, OriginalCol, OriginalRow,
						&DisplayCol, &DisplayRow);

		nextplot_ptr	= layer->Plot;
		nextplot_ptr	+= start_offset;
		nextplot_ptr	+= DisplayCol;
		nextplot_ptr	+= DisplayRow * room_width;
#endif/*__AMIGAOS__/__MSDOS__*/

		/* nextplot_ptr->PlotFlags	= NULL; */
		nextplot_ptr->TileSet_ID		= tileset->TS_id;
#if __AMIGAOS__
		nextplot_ptr->Tile_ID		= tile_ptr->TileNumber;

		tile_ptr = Next (tile_ptr);
	}
#elif __MSDOS__
		nextplot_ptr->Tile_ID		= i+1;
	}
#endif/*__AMIGAOS__/__MSDOS__*/
	return;

} /* InitTileSetLayer */
#else // !PLOTARRAY
#if UseSparseArray
/**************************************************************************
 *
 * InitTileSetLayer
 *
 * PURPOSE
 *		Place tiles in a source room, either packed or 'as found'.
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
 *		10/04/89 (RGM) - Created from split-up of AddTileSetLayer.
 *		03/07/91 (RGM) - Added offset support..
 *		08/19/94 (dcc) - changed to use WriteNPlotXY()
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
void InitTileSetLayer(RoomType *room, TileSetType *tileset, WORD height)
{
	LayerType		*play;
	UWORD		i = 0;
	WORD			OriginalCol = 0;
	WORD			OriginalRow = 0;
	int			DisplayCol;
	int			DisplayRow;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitTileSetLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	tileset->wDisplayRow = height;

	play		= Head (&(room->Layers));

	/***********************************/
	/* initialize the layer with tiles */

	for (i = 0; i < tileset->TileCount; i++)
	{
		PlotType plt;

		if ((tileset->Flags & GRID_TYPE_MODIFIER) != TILESET_COMPOSITE)
		{
			WORD w;

			ReadXTRA(tileset->pxtrDelCol[i/deltasPerCol],
					(i%deltasPerCol)*sizeof(WORD), sizeof(WORD), &w);
			OriginalCol += w;
		}
		else
		{
			OriginalCol += i ? 1 : 0;
		}
		OriginalRow += OriginalCol / tileset->MaxAcross;
		OriginalCol  = OriginalCol % tileset->MaxAcross;

		PlaceXYFromTSTXY(tileset, OriginalCol, OriginalRow,
						&DisplayCol, &DisplayRow);

		plt.Plot_Flags	= 0;
		plt.TileSet_ID	= tileset->TS_id;
		plt.Tile_ID	= i+1;

		LAY_WritePlotXY(&plt, play, DisplayCol, height+DisplayRow);
	}
	return;

} /* InitTileSetLayer */
#else // !UseSparseArray
/**************************************************************************
 *
 * InitTileSetLayer
 *
 * PURPOSE
 *		Place tiles in a source room, either packed or 'as found'.
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
 *		10/04/89 Wednesday - Created from split-up of AddTileSetLayer. (RGM)
 *		03/07/91 Thursday (RGM) - Added offset support.
 *
 * SEE ALSO
 *
*/
void InitTileSetLayer (
	RoomType		*room,
	TileSetType		*tileset,
	WORD			height
)
{
	PlotType		*nextplot_ptr;
#if __AMIGAOS__
	TileType		*tile_ptr;
#endif/*__AMIGAOS__*/
	LayerType		*layer;
#if __MSDOS__
	UWORD		i = 0;
	WORD			OriginalCol = 0;
	WORD			OriginalRow = 0;
#endif/*__MSDOS__*/
	int			DisplayCol;
	int			DisplayRow;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitTileSetLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	tileset->wDisplayRow = height;

	layer		= Head (&(room->Layers));

	/***********************************/
	/* initialize the layer with tiles */

#if __AMIGAOS__
	tile_ptr = Head (&(tileset->Tiles));

	while (!IsEOList(tile_ptr))
	{
		PlaceXYFromTSTXY(tileset, tile_ptr->Original_Column, tile_ptr->Original_Row,
						&DisplayCol, &DisplayRow);

		nextplot_ptr = ActivatePlotXYatWin(layer->rgrgplt,
								DisplayCol,
								height+DisplayRow, winDst);
#elif __MSDOS__
	for (i = 0; i < tileset->TileCount; i++)
	{
		if ((tileset->Flags & GRID_TYPE_MODIFIER) != TILESET_COMPOSITE)
		{
			WORD w;

			ReadXTRA(tileset->pxtrDelCol[i/deltasPerCol],
					(i%deltasPerCol)*sizeof(WORD), sizeof(WORD), &w);
			OriginalCol += w;
		}
		else
		{
			OriginalCol += i ? 1 : 0;
		}
		OriginalRow += OriginalCol / tileset->MaxAcross;
		OriginalCol  = OriginalCol % tileset->MaxAcross;
		
		PlaceXYFromTSTXY(tileset, OriginalCol, OriginalRow,
						&DisplayCol, &DisplayRow);

		nextplot_ptr = ActivatePlotXYatWin(layer->rgrgplt,
								DisplayCol, height+DisplayRow, winDst);
#endif/*__AMIGAOS__/__MSDOS__*/

		/* nextplot_ptr->PlotFlags	= NULL; */
		nextplot_ptr->TileSet_ID		= tileset->TS_id;
#if __AMIGAOS__
		nextplot_ptr->Tile_ID		= tile_ptr->TileNumber;

		UpdatePlotRow(layer->rgrgplt, height+Display_Row);

		tile_ptr = Next (tile_ptr);
	}
#elif __MSDOS__
		nextplot_ptr->Tile_ID		= i+1;

		UpdatePlotRow(layer->rgrgplt, height+DisplayRow);
	}
#endif/*__AMIGAOS__/__MSDOS__*/
	return;

} /* InitTileSetLayer */
#endif // !UseSparseArray
#endif


#if !UseSparseArray
/**************************************************************************
 *
 * AddSimpleLayer
 *
 * PURPOSE
 *		To add a layer to an undo room (DON'T clear layer to 0's).
 *
 * INPUT
 *		RoomType *room		: room to add layer to
 *		int width			: tiles wide to create layer
 *		int height		: tiles high to create layer
 *
 * ASSUMES
 *		That this routine is only ever called by ReMakeUndoRoom().
 *
 * RETURN VALUE
 *		Returns pointer to layer, or NULL if OOM.
 *
 * HISTORY
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
static LayerType *AddSimpleLayer(RoomType *room, int width, int height)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddSimpleLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (AddLayer (	room,
					Tail(&room->Layers),
					width,
					height,
					ADD_LAYER | DONTCLEAR_LAYER));

} /* AddSimpleLayer */
#endif // !UseSparseArray


/**************************************************************************
 *
 * AddClearedLayer
 *
 * PURPOSE
 *		Add a layer to a room (DO clear layer to 0's)
 *
 * INPUT
 *		RoomType *room		: room to add layer to
 *		int width			: tiles wide to create layer
 *		int height		: tiles high to create layer
#if UseSparseArray
 *		BOOL fMakeSparse	: if TRUE, make sparse array
#endif // UseSparseArray
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns pointer to layer, or NULL if OOM.
 *
 * HISTORY
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
#if UseSparseArray
LayerType *AddClearedLayer(RoomType *room, int width, int height, BOOL fMakeSparse)
#else // !UseSparseArray
LayerType *AddClearedLayer (
	RoomType	*room,
	WORD		width,
	WORD		height
)
#endif // !UseSparseArray
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddClearedLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if UseSparseArray
	return (AddLayer (room, Tail(&room->Layers), width, height, ADD_LAYER, fMakeSparse));
#else // !UseSparseArray
	return (AddLayer (room, Tail(&room->Layers), width, height, ADD_LAYER));
#endif // !UseSparseArray

} /* AddClearedLayer */


/**************************************************************************
 *
 * AddClearedLayerToLayers
 *
 * PURPOSE
 *		Add a layer to a layer structure (DO clear layer to 0's)
 *
 * INPUT
 *		ListType *layers	: layers to add layer to
 *		int width			: tiles wide to create layer
 *		int height		: tiles high to create layer
#if UseSparseArray
 *		BOOL fMakeSparse	: if TRUE, make sparse array
#endif // UseSparseArray
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns pointer to layer, or NULL if OOM.
 *
 * HISTORY
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
#if UseSparseArray
LayerType *AddClearedLayerToLayers(ListType *layers, int width, int height, BOOL fMakeSparse)
#else // !UseSparseArray
LayerType *AddClearedLayerToLayers (
	ListType	*layers,
	WORD		width,
	WORD		height
)
#endif // !UseSparseArray
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddClearedLayerToLayers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if UseSparseArray
	return (AddLayer(NULL, Tail(layers), width, height, ADD_LAYER, fMakeSparse));
#else // !UseSparseArray
	return (AddLayer (	NULL,
					Tail(layers),
					width,
					height,
					ADD_LAYER));
#endif // !UseSparseArray

} /* AddClearedLayerToLayers */


/*********************************************************************
 *
 * InsertClearedLayerToLayers
 *
 * PURPOSE
 *		Insert a layer before layer <play> in the room.
 *		DO clear layer to 0's.
 *
 * INPUT
 *		RoomType *prm		: room that layers belong to (may be NULL)
 *		LayerType *play	: layer to insert new layer B4
 *		int width			: tiles wide to create layer
 *		int height		: tiles high to create layer
#if UseSparseArray
 *		BOOL fMakeSparse	: if TRUE, make sparse array
#endif // UseSparseArray
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Pointer to layer inserted.
 *
 * HISTORY
 *		01/28/94 (dcc) - add <prm> input parameter
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
#if UseSparseArray
LayerType *InsertClearedLayerToLayers(RoomType *prm, LayerType *play,
								int width, int height, BOOL fMakeSparse)
#else // !UseSparseArray
LayerType *InsertClearedLayerToLayers(RoomType *prm, LayerType *play, short width, short height)
#endif // !UseSparseArray
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InsertClearedLayerToLayers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if UseSparseArray
	return (AddLayer (prm, play, width, height, INSERT_LAYER, fMakeSparse));
#else // !UseSparseArray
	return (AddLayer (prm, play, width, height, INSERT_LAYER));
#endif // !UseSparseArray

} /* InsertClearedLayerToLayers */


#if 0
#if __MSDOS__
/**************************************************************************
 *
 * CopyColors
 *
 * PURPOSE
 *		Copy a range of colors 0 to <end>-1 from
 *		the Color array pointed to by <SrcColors>
 *		to the ColorInfo structure pointed to by <DstColorInfo>.
*/

static void CopyColors(ColorInfo *DstColorInfo, UWORD end, CS_Color *SrcColors)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CopyColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	WriteXTRA(SrcColors, end * sizeof(CS_Color), DstColorInfo->pxtColors, 0);
	DstColorInfo->NumColors = end;

} /* CopyColors */
#endif/*__MSDOS__*/
#endif


/*********************************************************************
 *
 * CtilxDisplayFromTST
 *
 * PURPOSE
 *		Figure out how many tiles across are needed to display the
 *		tileset <ptst>.
 *
 *		This used to be just tileset->Across + 1, but now we need to
 *		deal with cxDisplay, ctilxSkip, etc.
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
 *		12/14/93 Tuesday (dcc) - created.
 *
*/
int CtilxDisplayFromTST(TileSetType *ptst)
{
	int ctilxE, ctilxO;
	int cxDisplay = ptst->cxDisplay ? ptst->cxDisplay : ptst->Width;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CtilxDisplayFromTST";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/


	ctilxO =	ptst->ctilxDispO +
			(ptst->Across * ptst->ctilxSkip) +
			(ptst->Width + cxDisplay - 1) / cxDisplay;

	ctilxE =	ptst->ctilxDispO + ptst->ctilxDispE +
			(ptst->Across * ptst->ctilxSkip) +
			(ptst->Width + cxDisplay - 1) / cxDisplay;

	return max(ctilxE, ctilxO);

} /* CtilxDisplayFromTST */


/*********************************************************************
 *
 * CtilyDisplayFromTST
 *
 * PURPOSE
 *		Figure out how many tiles down are needed to display the
 *		tileset <ptst>.
 *
 *		This used to be just tileset->Down + 1, but now we need to
 *		deal with cyDisplay, ctilySkip, etc.
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
 *		12/14/93 Tuesday (dcc) - created.
 *
*/
int CtilyDisplayFromTST(TileSetType *ptst)
{
	int cyDisplay = ptst->cyDisplay ? ptst->cyDisplay : ptst->Height;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CtilyDisplayFromTST";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return	(ptst->Down / 2) * ptst->ctilyDispE +
			( (ptst->Down+1) / 2) * ptst->ctilyDispO +
			(ptst->Height + cyDisplay - 1) / cyDisplay;

} /* CtilyDisplayFromTST */


/**************************************************************************
 *
 * CreateTileSetRoom
 *
 * PURPOSE
 *		Create a new tileset room.
#if UseSparseArray
 *
 *		Always create a non-sparse (i.e., 2-D) layer.
#endif // UseSparseArray
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
 *		01/08/91 (RGM) - Created to use new common CreateRoom().
 *		12/14/93 (dcc) - use CtilxDisplayFromTST & CtilyDisplayFromTST().
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
RoomType *CreateTileSetRoom (TileSetType *tileset)
{
	WORD		width;
	WORD		height;
	char		*newname	= NULL;

	RoomType	*newroom	= NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateTileSetRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	width	= (WORD) CtilxDisplayFromTST(tileset);
	height	= (WORD) CtilyDisplayFromTST(tileset);

#if UseSparseArray
	newroom = CreateRoom (width, height, CLEAR_LAYERS, SOURCEROOM | LOCKEDROOM, FALSE);
#else // !UseSparseArray
	newroom = CreateRoom (width, height, CLEAR_LAYERS, SOURCEROOM | LOCKEDROOM);
#endif // !UseSparseArray

	if (newroom) {
		int cxDisplay = tileset->cxDisplay ? tileset->cxDisplay : tileset->Width;
		int cyDisplay = tileset->cyDisplay ? tileset->cyDisplay : tileset->Height;

		if ( (newname = MEM_calloc (1, strlen (tileset->Filespec) + 1)) == NULL)
		{
/**/		goto ABORT;
		}
		else
		{
			strcpy (newname, tileset->Filespec);
			newroom->Name = newname;
		}

		newroom->FloorLayer->cxTile = cxDisplay;
		newroom->FloorLayer->cyTile = cyDisplay;

		if ((newroom->R_ColorInfo = CreateColorInfo ()) != NULL) {

#if __AMIGAOS__
			SetColorInfoColorRange (newroom->R_ColorInfo,
				0, tileset->SharedShape->Pic->NumberOfColors - 1,
				ShapeColorMap (tileset->SharedShape));
#elif __MSDOS__
			CopyColorInfoToColorInfo(tileset->cinShared, newroom->R_ColorInfo);
#endif/*__AMIGAOS__/__MSDOS__*/
		}
		else {
/**/		goto ABORT;
		}

#if PLOTARRAY
		InitTileSetLayer (newroom, tileset, 0L);
#else
		InitTileSetLayer (newroom, tileset, 0);
#endif

		tileset->SourceRoom	= newroom;
	}

	return (newroom);

/*-----------------------------------------------------------------------*/
ABORT:

	SAFEFREE (newname);

	if (newroom) {
		DeAllocateRoom (newroom);
	}

	return (NULL);

} /* CreateTileSetRoom */


/**************************************************************************
 *
 * CreateUserRoom
 *
 * PURPOSE
 *		Create a user type room.
 *
 *		If OOM, informs user w/ a dialog box.
 *
 * USAGE
 *
 *
 * INPUT
 *		int width			: tiles wide to create 1st layer
 *		int height		: tiles high to create 1st layer
 *		UWORD create_type	: check CLEAR_LAYERS | RESET_UNDO
 *		UWORD room_flags	: set the new room's <Flags> with this UWORD
#if UseSparseArray
 *		BOOL fMakeSparse	: if TRUE, make sparse array
#endif // UseSparseArray
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns pointer to room, or NULL if OOM.
 *
 * HISTORY
 *		01/08/91 (RGM) - Created to use new common CreateRoom().
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
#if UseSparseArray
static RoomType *CreateUserRoom(int width, int height,
					UWORD create_type, UWORD room_flags, BOOL fMakeSparse)
#else // !UseSparseArray
static RoomType *CreateUserRoom (
	WORD	width,
	WORD	height,
	UWORD	create_type,
	UWORD	room_flags
)
#endif // !UseSparseArray
{
	RoomType	*newroom	= NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateUserRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if UseSparseArray
	newroom	= CreateRoom (width, height, create_type, room_flags, fMakeSparse);
#else // !UseSparseArray
	newroom	= CreateRoom (width, height, create_type, room_flags);
#endif // !UseSparseArray

	if (newroom)
	{
		newroom->R_ColorInfo	= DuplicateColorInfo (&DefaultColorInfo);
		if (!newroom->R_ColorInfo)
		{
			DeAllocateRoom (newroom);
			TellUser (OOM, "Unable to create room.");
			newroom = NULL;
		}
	}
	return newroom;

} /* CreateUserRoom */


#if !UseSparseArray
/**************************************************************************
 *
 * CreateSimpleRoom
 *
 * PURPOSE
 *		Create an uncleared room and don't update the "undo" room.
 *
 *		If OOM, informs user w/ a dialog box.
 *
 * INPUT
 *
 *
 * ASSUMES
 *		That this routine is only ever called by ReMakeUndoRoom().
 *
 * RETURN VALUE
 *		Returns pointer to room, or NULL if OOM.
 *
 * HISTORY
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
static RoomType *CreateSimpleRoom(int width, int height, UWORD room_flags)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateSimpleRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return CreateUserRoom(width, height, NULL, room_flags);

} /* CreateSimpleRoom */
#endif // !UseSparseArray


/**************************************************************************
 *
 * CreateRoomWithUndo
 *
 * PURPOSE
 *		To create a cleared room and update the "undo" room.
 *
 *		If OOM, informs user w/ a dialog box.
 *
 * USAGE
 *
 *
 * INPUT
 *		int width			: tiles wide to create 1st layer
 *		int height		: tiles high to create 1st layer
 *		UWORD room_flags	: set the new room's <Flags> with this UWORD
#if UseSparseArray
 *		BOOL fMakeSparse	: if TRUE, make sparse array
#endif // UseSparseArray
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns pointer to room, or NULL if OOM.
 *
 * HISTORY
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
#if UseSparseArray
RoomType *CreateRoomWithUndo(int width, int height, UWORD room_flags, BOOL fMakeSparse)
#else // !UseSparseArray
RoomType *CreateRoomWithUndo (
	WORD	width,
	WORD	height,
	UWORD	room_flags
)
#endif // !UseSparseArray
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateRoomWithUndo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if UseSparseArray
	return (CreateUserRoom (width, height,
						(CLEAR_LAYERS | RESET_UNDO), room_flags, fMakeSparse));
#else // !UseSparseArray
	return (CreateUserRoom (width, height,
						(CLEAR_LAYERS | RESET_UNDO), room_flags));
#endif // !UseSparseArray

} /* CreateRoomWithUndo */


/**************************************************************************
 *
 * DeAllocateLayer
 *
 * PURPOSE
 *		Frees memory used by layer & invalidate undo (whatever that means.)
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
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
void DeAllocateLayer (LayerType *layer)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeAllocateLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/


	if (layer) {
		if (layer->Parent_Room) {
			if (RoomIsUsedByUndo (layer->Parent_Room)) {
				InvalidateUndo ();
			}
		}

#if PLOTARRAY
		FreePlot(&layer->Plot);
#else
#if UseSparseArray
		if (IsSparse(layer))
			FreeSparseArray(layer->p.pspa);
		else
			FreePlot(&layer->p.rgrgplt);
#else // !UseSparseArray
		FreePlot(&layer->rgrgplt);
#endif // !UseSparseArray
#endif
		MEM_free (layer);
	}

} /* DeAllocateLayer */



/**************************************************************************
 *
 * DeAllocateLayers
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
void DeAllocateLayers (ListType *layers)
{
	LayerType *currentlayer;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeAllocateLayers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (layers) {
		while ((currentlayer = RemHead (layers)) != NULL) {
			DeAllocateLayer (currentlayer);
		}
	}

	return;

} /* DeAllocateLayers */


/*********************************************************************
 *
 * ResizeRoom
 *
 * PURPOSE
 *		Change room from its current Width and Height to a new
 *		<wNewWidth> and <wNewHeight>.
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
 *		11/05/93 (dcc) - call ReMakeUndoRoom before exiting.
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
BOOL ResizeRoom(RoomType *prm, WORD wNewWidth, WORD wNewHeight)
{
	BOOL fSucceed = TRUE;
	WORD cntLayers = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ResizeRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (wNewWidth == 0 || wNewHeight == 0)
		return FALSE;

	{
#if UseSparseArray
		ULONG cx, cy;
#endif // UseSparseArray

		/* Expand plots of all layers */

		LayerType *play = Head(&prm->Layers);

#if UseSparseArray
		cx = (ULONG) wNewWidth  * play->cxTile;
		cy = (ULONG) wNewHeight * play->cyTile;
#endif // UseSparseArray

		while (!IsEOList(play))
		{
#if UseSparseArray
			if (!IsSparse(play))
			{
				int ctilxNew, ctilyNew;
				BOOL fExpand = FALSE;

				if (cx && cy && play->cxTile && play->cyTile)
				{
					if (	cx > (ULONG) play->p.rgrgplt.PlotWidth  * play->cxTile ||
						cy > (ULONG) play->p.rgrgplt.PlotHeight * play->cyTile)
					{
						ctilxNew = (int) (cx / play->cxTile);
						ctilyNew = (int) (cy / play->cyTile);
						fExpand = TRUE;
					}
				}
				else if (	wNewWidth  > play->p.rgrgplt.PlotWidth ||
						wNewHeight > play->p.rgrgplt.PlotHeight)
				{
					ctilxNew = wNewWidth;
					ctilyNew = wNewHeight;
					fExpand = TRUE;
				}

				if (fExpand)
				{
					if (!ExpandPlot(&play->p.rgrgplt, ctilxNew, ctilyNew))
						fSucceed = FALSE;
				}
			}
			cntLayers++;
			play = Next(play);
#else // !UseSparseArray
			if (	wNewWidth  > play->rgrgplt.PlotWidth ||
				wNewHeight > play->rgrgplt.PlotHeight)
			{
				if (!ExpandPlot(&play->rgrgplt, wNewWidth, wNewHeight))
					fSucceed = FALSE;
			}
			cntLayers++;
			play = Next(play);
#endif // !UseSparseArray
		}

		if (fSucceed)
		{
			LayerType *play = Head(&prm->Layers);

			while (!IsEOList(play))
			{
#if UseSparseArray
				if (play->cxTile)
					play->ctilx = (int) (cx / play->cxTile);
				else
					play->ctilx = wNewWidth;

				if (play->cyTile)
					play->ctily = (int) (cy / play->cyTile);
				else
					play->ctily = wNewHeight;
#else // !UseSparseArray
				play->ctilx = wNewWidth;
				play->ctily = wNewHeight;
#endif // !UseSparseArray

				play = Next(play);
			}

			prm->wOldRoomWidth = wNewWidth;
			prm->wOldRoomHeight = wNewHeight;
#if UseSparseArray
			UndoRoom	= ReMakeUndoRoom(UndoRoom, prm);
#else // !UseSparseArray
			UndoRoom	= ReMakeUndoRoom (UndoRoom, wNewWidth, wNewHeight, cntLayers);
#endif // !UseSparseArray
			if (!UndoRoom)		//dcc
			{
				return FALSE;
			}
			return TRUE;
		}

		return fSucceed;
	}
} /* ResizeRoom */


/**************************************************************************
 *
 * DeAllocateRWInits
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
void DeAllocateRWInits (ListType *rwinits)
{
	RWInitsType *rwinit;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeAllocateRWInits";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (rwinits) {
		while ((rwinit = RemHead (rwinits)) != NULL) {
			MEM_free (rwinit);
		}
	}

	return;

} /* DeAllocateRWInits */



/**************************************************************************
 *
 * DeAllocateDisplays
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
void DeAllocateDisplays (ListType *displays)
{
	RWLockType *display;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeAllocateDisplays";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (displays) {
		while ((display = RemHead (displays)) != NULL) {
			MEM_free (display);
		}
	}

	return;

} /* DeAllocateDisplays */



/**************************************************************************
 *
 * DeAllocateItems
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
void DeAllocateItems (ListType *items)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeAllocateItems";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	items = items;		/* Turn off warnings */
	return;

} /* DeAllocateItems */



/**************************************************************************
 *
 * DeAllocateRoom
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
void DeAllocateRoom (RoomType *room)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeAllocateRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (room) {
		RemoveSmartFlipRoom(room);
		FreeTileCounts(room);

		if (RoomIsUsedByUndo (room)) {
			InvalidateUndo ();
		}

#if NEWTUME
		DeAllocateRoomGuides(room);
#endif
		DeAllocateRWInits (&(room->RWInits));
		DeAllocateDisplays (&(room->Displays));
		DeAllocateLayers (&(room->Layers));
		DeAllocateItems (&(room->Items));

		if (room->Name) {
			MEM_free (room->Name);
			room->Name = NULL;
		}
		if (room->R_ColorInfo) {
			DeleteColorInfo (room->R_ColorInfo);
			room->R_ColorInfo = NULL;
		}
		MEM_free (room);
	}
	return;

} /* DeAllocateRoom */


/**************************************************************************
 *
 * FirstSourceRoom
 *
 * PURPOSE
 *
 *
 * USAGE
 *		ROOM_PTR = FirstSourceRoom (MAP_PTR);
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
 *		10/24/89 Tuesday - Created. (RGM)
 *		04/05/94 Tuesday (dcc) - revised to find 1st locked rooms.
 *
 * SEE ALSO
 *
*/
RoomType *FirstSourceRoom (MapType *map)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FirstSourceRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = Head (&map->Rooms);

	while ( (! IsEOList (room)) && (!IsLocked(room)) )
	{
		room = Next (room);
	}
	if (! IsEOList (room))	return (room);
	else					return (NULL);

} /* FirstSourceRoom */


/**************************************************************************
 *
 * FirstEditRoom
 *
 * PURPOSE
 *
 *
 * USAGE
 *		ROOM_PTR = FirstEditRoom (MAP_PTR);
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
 *		10/24/89 Tuesday - Created. (RGM)
 *		04/05/94 Tuesday (dcc) - revised to find 1st unlocked rooms.
 *
 * SEE ALSO
 *
*/
RoomType *FirstEditRoom (MapType *map)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FirstEditRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room = Head (&map->Rooms);

	while ( (! IsEOList (room)) && (IsLocked(room)) )
	{
		room = Next (room);
	}
	if (! IsEOList (room))	return (room);
	else					return (NULL);

} /* FirstEditRoom */


#if UseSparseArray
/**************************************************************************
 *
 * ReMakeUndoRoom
 *
 * PURPOSE
 *		Create an undo room with the same layers and layer sizes as
 *		prmModel.
 *
 *		If OOM, informs user w/ a dialog box.
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
 *		11/01/89 (RGM) - Created.
 *		11/05/93 (dcc) - added check to see if room is already large enough
 *		08/22/94 (dcc) - add support for UseSparseArray
 *		09/17/94 (dcc) - make each layer same sparse/non-sparse as prmModel
 *		10/11/94 (dcc) - move playU inside if() to remove NULL ptr reference
 *		10/20/94 (dcc) - Always make undo rooms layers sparse. Also, make
 *					  sure each layer is large enough to store the
 *					  largest layers in each room.
 *
 * SEE ALSO
 *
*/
static RoomType *ReMakeUndoRoom(RoomType *undo_room, RoomType *prmModel)
{
	LayerType *playM;
	LayerType *playU;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReMakeUndoRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	playM = Head(&prmModel->Layers);
	if (!undo_room)
	{
		if ((undo_room = CreateUserRoom(playM->ctilx, playM->ctily,
								  0, FREEROOM, TRUE)) == NULL)
//								  0, FREEROOM, IsSparse(playM))) == NULL)
/**/		goto ABORT;
	}

	playU = Head(&undo_room->Layers);

	/* Make sure undo room has enough layers... */

	while (!IsEOList(playM))
	{
		if (IsEOList(playU))
		{
			if (!AddLayer(undo_room, Tail(&undo_room->Layers),
					    playM->ctilx, playM->ctily,
					    ADD_LAYER | DONTCLEAR_LAYER, TRUE))
//					    ADD_LAYER | DONTCLEAR_LAYER, IsSparse(playM)))
/**/			goto ABORT;
			undo_room->LayerCount++;
			playU = Tail(&undo_room->Layers);
		}
		/* ...and make sure each layer is large enough. */

		if (	playU->ctilx < playM->ctilx ||
			playU->ctily < playM->ctily )
		{
		/* 	Hey, since the undo room is a sparse array,
			we'll just go ahead and set the layer size
			directly right here, right now. */

			playU->ctilx = max(playU->ctilx, playM->ctilx);
			playU->ctily = max(playU->ctily, playM->ctily);
		}
		playM = Next(playM);
		playU = Next(playU);
	}
	return undo_room;
/*-----------------------------------------------------------------------*/
ABORT:
	if (undo_room)
	{
		DeAllocateRoom (undo_room);
		undo_room = NULL;
	}
	return NULL;

} /* ReMakeUndoRoom */
#else // !UseSparseArray
/**************************************************************************
 *
 * ReMakeUndoRoom
 *
 * PURPOSE
 *		If OOM, informs user w/ a dialog box.
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
 *		11/01/89 (RGM) - Created.
 *		11/05/93 (dcc) - added check to see if room is already large enough
 *
 * SEE ALSO
 *
*/
static RoomType *ReMakeUndoRoom (
	RoomType	*undo_room,
	int		width,
	int		height,
	int		depth
)
{
	UWORD		layer_count;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReMakeUndoRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Is existing undo room large enough already? */

	if (undo_room)
	{
		LayerType *play = Head(&undo_room->Layers);

		width	= max (play->ctilx, width);
		height	= max (play->ctily, height);
		depth	= max (undo_room->LayerCount, depth);

		if ((width <= play->ctilx) && (height <= play->ctily) &&
				(depth <= undo_room->LayerCount))
		{
			return undo_room;
		}
	}

	/********************************/
	/* get rid of the old undo room */

	if (undo_room)
	{
		DeAllocateRoom (undo_room);
		undo_room = NULL;
	}

	/**************/
	/* re-make it */

	undo_room = CreateSimpleRoom (width, height, FREEROOM);

/**/if (! undo_room) goto ABORT;

	layer_count = 1;

	while (depth > layer_count)
	{
		if (! AddSimpleLayer (undo_room, width, height))
		{
/**/		goto ABORT;
		}
		layer_count ++;
	}

	undo_room->LayerCount	= layer_count;

	return (undo_room);
/*-----------------------------------------------------------------------*/
ABORT:
	if (undo_room) {
		DeAllocateRoom (undo_room);
		undo_room = NULL;
	}

	return (NULL);

} /* ReMakeUndoRoom */
#endif // !UseSparseArray


/**************************************************************************
 *
 * CreateMap
 *
 * PURPOSE
 *		Create & initialize a new map structure.
 *
 * USAGE
 *		MAP_PTR = CreateMap ();
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Pointer to a map if OK, FALSE otherwise.
 *
 * HISTORY
 *		11/13/89 Monday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
MapType *CreateMap (void)
{
	MapType *newmap = NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((newmap = MEM_calloc (1, MAPSIZE)) != NULL) {
		InitList (&(newmap->Rooms));
	}
	return (newmap);

} /* CreateMap */


/**************************************************************************
 *
 * DeleteMap
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
 *		11/13/89 Monday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void *DeleteMap (MapType *map)
{
	RoomType *room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (map) {
		while ((room = RemHead (&(map->Rooms))) != NULL) {
			DeAllocateRoom (room);
		}
		SAFEFREE (map);
	}
	return (NULL);

} /* DeleteMap */


#if 0
/**************************************************************************
 *
 * DuplicateRoom
 *
 * PURPOSE
 *
 *
 * USAGE
 *		NEW_ROOM = DuplicateRoom (OLD_ROOM);
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
RoomType *DuplicateRoom (RoomType room)
{

	RoomType *newroom	= NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DuplicateRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (newroom);
/*-----------------------------------------------------------------------*/
ABORT:
	return (NULL);

} /* DuplicateRoom */
#endif


/**************************************************************************
 *
 * TryNFindRoomID
 *
 * PURPOSE
 *		To find a room that has a certain id number.
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
RoomType *TryNFindRoomID (
	WORD	id,
	MapType	*map
)
{
	RoomType	*room;
	RoomType	*result = NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TryNFindRoomID";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (map) {
		room = Head (&map->Rooms);
		while (! IsEOList (room)) {
			if (room->R_id == id) {
				result = room;
				break;
			}
			room = Next (room);
		}
	}
	return (result);

} /* TryNFindRoomID */


#if 0	// need to write UseSparseArray stuff for DeleteRoomColumns()...
/*********************************************************************
 *
 * DeletePlotColumns
 *
 * PURPOSE
 *		Delete <wNumColumns> starting at <wColumn> from <prgrgplt>. The
 *		columns following <wNumColumns> are copied to <wColumn>, and the
 *		space remaining is set to NULL tiles.
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
 *		08/10/92 Monday (dcc) - created.
 *
 * SEE ALSO
 *
*/
static BOOL DeletePlotColumns(RGRGPLT *prgrgplt, WORD wColumn, WORD wNumColumns)
{

	WORD i, wByteDst, wByteSrc, wBytesToCopy, wBytesToZero;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeletePlotColumns";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	wByteDst = wColumn * PLOTSIZE;
	wByteSrc = (wColumn + wNumColumns) * PLOTSIZE;
	wBytesToCopy = max(0, prgrgplt->PlotWidth - (wColumn+wNumColumns)) * PLOTSIZE;
	wBytesToZero = max(0, prgrgplt->PlotWidth * (WORD) PLOTSIZE - (wByteDst + wBytesToCopy));

	for (i = 0; i < prgrgplt->PlotHeight; i++)
	{
		PlotType	*pPtDst;

		pPtDst = ActivatePlotRowatWin(*prgrgplt, i, winDst);

		if (wBytesToCopy)
		{
			memcpy(pPtDst+wByteDst, pPtDst+wByteSrc, wBytesToCopy);
		}
		if (wBytesToZero)
		{
			memset(pPtDst+wByteDst+wBytesToCopy, 0, wBytesToZero);
		}

		UpdatePlotRow(*prgrgplt, i);
	}

	return TRUE;
} /* DeletePlotColumns */


/*********************************************************************
 *
 * DeleteRoomColumns
 *
 * PURPOSE
 *		Delete <wNumColumns> starting at <wColumn> from <prm>. The
 *		columns following <wNumColumns> are copied to <wColumn>, and the
 *		space remaining is set to NULL tiles.
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
 *		08/10/92 Monday (dcc) - created.
 *
 * SEE ALSO
 *
*/
BOOL DeleteRoomColumns(RoomType *prm, WORD wColumn, WORD wNumColumns)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteRoomColumns";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		/* Delete columns in all layers */

		LayerType *play = Head(&prm->Layers);

		while (!IsEOList(play))
		{
			if (!DeletePlotColumns(&play->rgrgplt, wColumn, wNumColumns))
			{
				return FALSE;
			}
			play = Next(play);
		}
	}
	return TRUE;
} /* DeleteRoomColumns */
#endif


/*********************************************************************
 *
 * DeletePlotRows
 *
 * PURPOSE
 *		Delete <wNumRows> starting at <wRow> from <prgrgplt>. The rows
 *		following <wNumRows> are copied to <wRow>, and the space
 *		remaining is set to NULL tiles.
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
 *		08/10/92 Monday (dcc) - created.
 *
 * SEE ALSO
 *
*/
static BOOL DeletePlotRows(RGRGPLT *prgrgplt, WORD wRow, WORD wNumRows)
{

	WORD i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeletePlotRows";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (i = wRow; i < prgrgplt->PlotHeight; i++)
	{
		PlotType	*pPtDst;

		pPtDst = ActivatePlotRowatWin(*prgrgplt, i, winDst);
		if (i + wNumRows < prgrgplt->PlotHeight)
		{
			PlotType	*pPtSrc =
				ActivatePlotRowatWin(*prgrgplt, i+wNumRows, winSrc);

			memcpy(pPtDst, pPtSrc, prgrgplt->PlotWidth * PLOTSIZE);

			ReleasePlotRow(*prgrgplt, i+wNumRows);
		}
		else
		{
			memset(pPtDst, 0, prgrgplt->PlotWidth * PLOTSIZE);
		}

		UpdatePlotRow(*prgrgplt, i);
	}

	return TRUE;
} /* DeletePlotRows */


/*********************************************************************
 *
 * DeleteRoomRows
 *
 * PURPOSE
 *		Delete <wNumRows> starting at <wRow> from <prm>. The rows
 *		following <wNumRows> are copied to <wRow>, and the space
 *		remaining is set to NULL tiles.
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
 *		08/10/92 (dcc) - created.
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
 * SEE ALSO
 *
*/
BOOL DeleteRoomRows(RoomType *prm, WORD wRow, WORD wNumRows)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteRoomRows";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		/* Delete rows in all layers */

		LayerType *play = Head(&prm->Layers);

		while (!IsEOList(play))
		{
#if UseSparseArray
			if (IsSparse(play))
			{
				if (!DeleteSparseArrayRows(play->p.pspa, wRow, wNumRows))
					return FALSE;
			}
			else
			{
				if (!DeletePlotRows(&play->p.rgrgplt, wRow, wNumRows))
					return FALSE;
			}
#else // !UseSparseArray
			if (!DeletePlotRows(&play->rgrgplt, wRow, wNumRows))
				return FALSE;
#endif // !UseSparseArray

			play = Next(play);
		}
	}
	return TRUE;
} /* DeleteRoomRows */


/************************************************************************/
/* call 'StartUndo(room, undo_room, left, top, right, bottom)'          */
/*  to initialize */

/* call 'UpdateUndo(room, undo_room, o_left, o_top, o_right, o_bottom)' */
/*  to update */

/* call 'RetrieveUndo (undo_room)' */
/*  to retrieve undo */

/* call 'RectRoomToRoom (room, undo_room, left, top, right, bottom)' */
/*  to copy from room to room */

#define SHOW_US		(FALSE)


#if PixelLayers
/*********************************************************************
 *
 * RectRoomToRoom
 *
 * PURPOSE
 *		Copy a rectangular area of tiles from room to room.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/19/94 (dcc) - add support for UseSparseArray
 *		08/30/94 (dcc) - add support for tile object pixel layers
 *		09/04/94 (dcc) - use GetNextSparsePlotInRange() to speed up sparse array access
 *		10/20/94 (dcc) - Fix routine locking near edge of rooms.
 *
*/
static void RectRoomToRoom (
	RoomType	*source_room,
	RoomType	*dest_room,
	int		Left,
	int		Top,
	int		Right,
	int		Bottom,
	BOOL		fDirection
)
{
register	PlotType	*source_room_plot;
register	PlotType	*dest_room_plot;

#if PLOTARRAY
register	PlotType	*source_room_plot_hold;
register	PlotType	*dest_room_plot_hold;
#endif

			LayerType	*source_room_layer;
			LayerType	*dest_room_layer;

register	int		row;
#if PLOTARRAY
register	int		column;
#endif
			int		max_row;
			int		max_column;

#if PLOTARRAY
			int		source_room_row_add;
			int		dest_room_row_add;
#endif

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RectRoomToRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

{
	// make sure that Left <= Right and Top <= Bottom

	if (Left > Right)
	{
		int i;
		i = Right;
		Right = Left;
		Left = i;
	}
	if (Top > Bottom)
	{
		int i;
		i = Bottom;
		Bottom = Top;
		Top = i;
	}
}

	if (source_room && dest_room)
	{
#if SHOW_US
		printf ("\tRectRoomToRoom %d (%d, %d)(%d, %d)\n", fDirection, left, top, right, bottom);
#endif

#if PLOTARRAY
		source_room_row_add	= sr_width;
		dest_room_row_add	= dr_width;
#endif
		source_room_layer	= Head (&source_room->Layers);
		dest_room_layer	= Head (&dest_room->Layers);

		while ((! IsEOList (source_room_layer)) && (! IsEOList (dest_room_layer)))
		{
			int		sr_width;
			int		sr_height;
			int		dr_width;
			int		dr_height;
			int		min_width;
			int		min_height;
			int		top, left, bottom, right;

			sr_width	= source_room_layer->ctilx;
			sr_height	= source_room_layer->ctily;
			dr_width	= dest_room_layer->ctilx;
			dr_height	= dest_room_layer->ctily;

			min_height= min (sr_height, dr_height);
			min_width	= min (sr_width, dr_width);

			if (source_room_layer->cxTile && dest_room_layer->cxTile)
				if (source_room_layer->cxTile != dest_room_layer->cxTile)
/**/				goto SKIP;

			if (source_room_layer->cyTile && dest_room_layer->cyTile)
				if (source_room_layer->cyTile != dest_room_layer->cyTile)
/**/				goto SKIP;

			top		= Top;
			left		= Left;
			bottom	= Bottom;
			right	= Right;

			if (source_room_layer->cxTile && source_room_layer->cyTile)
			{
				LayerType *playBig = FindBiggestLayer(&source_room->Layers);

				Assert(playBig != NULL);
				if (playBig->cxTile && playBig->cyTile)
				{
					top	 = Top      * playBig->cyTile / source_room_layer->cyTile;
					left	 = Left     * playBig->cxTile / source_room_layer->cxTile;
					bottom=(Bottom+1)* playBig->cyTile / source_room_layer->cyTile-1;
					right =(Right+1) * playBig->cxTile / source_room_layer->cxTile-1;
				}
				dest_room_layer->cxTile = source_room_layer->cxTile;
				dest_room_layer->cyTile = source_room_layer->cyTile;
			}

			if (	(right >= 0) && (bottom >= 0) &&
				(top < min_height) && (left < min_width))
			{
				left		= max (left, 0);
				top		= max (top, 0);
				right	= min (right, (min_width - 1));
				bottom	= min (bottom, (min_height - 1));

				max_column	= abs(right - left) + 1;
				max_row		= abs(bottom - top) + 1;

#if SHOW_US
				printf ("\t(after clipping : (%d, %d)(%d, %d))\n", left, top, left + max_column - 1, top + max_row - 1);
#endif
#if PLOTARRAY
				source_room_plot_hold	= source_room_layer->Plot;
				source_room_plot_hold	+= ((source_room_row_add * top) + left);

				dest_room_plot_hold		= dest_room_layer->Plot;
				dest_room_plot_hold		+= ((dest_room_row_add * top) + left);

				for (row = max_row; row != 0; row --)
				{
					source_room_plot	= source_room_plot_hold;
					dest_room_plot		= dest_room_plot_hold;

					if (fDirection == SWAPROOMRECT)
					{
						for (column = max_column; column != 0; column --)
						{
							PlotType ptTemp	= *source_room_plot;

							*source_room_plot++	= *dest_room_plot;
							*dest_room_plot++	= ptTemp;
						}
					}
					else
					{
						for (column = max_column; column != 0; column --)
						{
							*dest_room_plot++	= *source_room_plot++;
						}
					}

					source_room_plot_hold	+= source_room_row_add;
					dest_room_plot_hold		+= dest_room_row_add;
				}
#else // !PLOTARRAY
#if UseSparseArray
				if (fDirection == SWAPROOMRECT)
				{
					if (IsSparse(source_room_layer) && IsSparse(dest_room_layer))
					{
						SwapRectInSparseArray(source_room_layer->p.pspa,
											dest_room_layer->p.pspa,
											left, top,
											max_column, max_row);
					}
					else if (IsSparse(source_room_layer) && !IsSparse(dest_room_layer))
					{
						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt1, plt2;

								ReadSparsePlotXY(source_room_layer->p.pspa, itilx, row+top, &plt1);
								ReadNPlotXY(dest_room_layer->p.rgrgplt, itilx, row+top, 1, &plt2);
								WriteNPlotXY(&plt1, 1, dest_room_layer->p.rgrgplt, itilx, row+top);
								WriteSparsePlotXY(&plt2, source_room_layer->p.pspa, itilx, row+top);
							}
						}
					}
					else if (!IsSparse(source_room_layer) && IsSparse(dest_room_layer))
					{
						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt1, plt2;

								ReadNPlotXY(source_room_layer->p.rgrgplt, itilx, row+top, 1, &plt1);
								ReadSparsePlotXY(dest_room_layer->p.pspa, itilx, row+top, &plt2);
								WriteSparsePlotXY(&plt1, dest_room_layer->p.pspa, itilx, row+top);
								WriteNPlotXY(&plt2, 1, source_room_layer->p.rgrgplt, itilx, row+top);
							}
						}
					}
					else
					{
						for (row = 0; row < max_row; row++)
						{
							WORD w;

							dest_room_plot   = ActivatePlotXYatWin(dest_room_layer->p.rgrgplt, left, row+top, winDst);
							source_room_plot = ActivatePlotXYatWin(source_room_layer->p.rgrgplt, left, row+top, winSrc);

							for (w = 0; w < max_column; w++)
							{
								PlotType ptTemp	= *source_room_plot;

								*source_room_plot++	= *dest_room_plot;
								*dest_room_plot++	= ptTemp;
							}
							UpdatePlotRow(source_room_layer->p.rgrgplt, row+top);
							UpdatePlotRow(dest_room_layer->p.rgrgplt, row+top);
						}
					}
				}
				else
				{
					if (IsSparse(source_room_layer) && IsSparse(dest_room_layer))
					{
#if 0
						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt;

								ReadSparsePlotXY(source_room_layer->p.pspa, itilx, row+top, &plt);
								WriteSparsePlotXY(&plt, dest_room_layer->p.pspa, itilx, row+top);
							}
						}
#else
						PlotType *pplt;
						int ixt, iyt;

						ZeroRectInSparseArray(dest_room_layer->p.pspa,
											left, top,
											max_column, max_row);

						SetSparseLayerLimits(source_room_layer->p.pspa,
										 left, top,
										 max_column, max_row);

						while ((pplt = GetNextSparsePlotInRange(source_room_layer->p.pspa, &ixt, &iyt)) != NULL)
						{
							WriteSparsePlotXY(pplt, dest_room_layer->p.pspa, ixt, iyt);
						}
#endif
					}
					else if (IsSparse(source_room_layer) && !IsSparse(dest_room_layer))
					{
						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt;

								ReadSparsePlotXY(source_room_layer->p.pspa, itilx, row+top, &plt);
								WriteNPlotXY(&plt, 1, dest_room_layer->p.rgrgplt, itilx, row+top);
							}
						}
					}
					else if (!IsSparse(source_room_layer) && IsSparse(dest_room_layer))
					{
						ZeroRectInSparseArray(dest_room_layer->p.pspa,
											left, top,
											max_column, max_row);

						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt;

								ReadNPlotXY(source_room_layer->p.rgrgplt, itilx, row+top, 1, &plt);
								if (plt.Plot_Flags != 0 || plt.TileSet_ID != 0 || plt.Tile_ID != 0)
									WriteSparsePlotXY(&plt, dest_room_layer->p.pspa, itilx, row+top);
							}
						}
					}
					else
					{
						for (row = 0; row < max_row; row++)
						{
							dest_room_plot   = ActivatePlotXYatWin(dest_room_layer->p.rgrgplt, left, row+top, winDst);
							source_room_plot = ActivatePlotXYatWin(source_room_layer->p.rgrgplt, left, row+top, winSrc);

							memcpy(dest_room_plot, source_room_plot, max_column * PLOTSIZE);

							ReleasePlotRow(source_room_layer->p.rgrgplt, row+top);
							UpdatePlotRow(dest_room_layer->p.rgrgplt, row+top);
						}
					}
				}
#else // !UseSparseArray
				for (row = 0; row < max_row; row++)
				{
					dest_room_plot   = ActivatePlotXYatWin(dest_room_layer->rgrgplt, left, row+top, winDst);
					source_room_plot = ActivatePlotXYatWin(source_room_layer->rgrgplt, left, row+top, winSrc);

					if (fDirection == SWAPROOMRECT)
					{
						WORD w;

						for (w = 0; w < max_column; w++)
						{
							PlotType ptTemp	= *source_room_plot;

							*source_room_plot++	= *dest_room_plot;
							*dest_room_plot++	= ptTemp;
						}
						UpdatePlotRow(source_room_layer->rgrgplt, row+top);
						UpdatePlotRow(dest_room_layer->rgrgplt, row+top);
					}
					else
					{
						memcpy(dest_room_plot, source_room_plot, max_column * PLOTSIZE);

						ReleasePlotRow(source_room_layer->rgrgplt, row+top);
						UpdatePlotRow(dest_room_layer->rgrgplt, row+top);
					}
				}
#endif // !UseSparseArray
#endif // !PLOTARRAY
			} /* in bounds */
SKIP:
				source_room_layer = Next (source_room_layer);
				dest_room_layer = Next (dest_room_layer);
		} /* while */
	}
#if SHOW_US
	else
	{
		printf ("\tRectRoomToRoom, NO UNDO ROOM.\n");
	}
#endif

	return;

} /* RectRoomToRoom */
#else // !PixelLayers
/*********************************************************************
 *
 * RectRoomToRoom
 *
 * PURPOSE
 *		Copy a rectangular area of tiles from room to room.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
*/
static void RectRoomToRoom (
	RoomType	*source_room,
	RoomType	*dest_room,
	WORD		left,
	WORD		top,
	WORD		right,
	WORD		bottom,
	BOOL		fDirection
)
{
register	PlotType	*source_room_plot;
register	PlotType	*dest_room_plot;

#if PLOTARRAY
register	PlotType	*source_room_plot_hold;
register	PlotType	*dest_room_plot_hold;
#endif

			LayerType	*source_room_layer;
			LayerType	*dest_room_layer;

register	WORD		row;
#if PLOTARRAY
register	WORD		column;
#endif
			WORD		max_row;
			WORD		max_column;

#if PLOTARRAY
			WORD		source_room_row_add;
			WORD		dest_room_row_add;
#endif

			WORD		sr_width;
			WORD		sr_height;
			WORD		dr_width;
			WORD		dr_height;
			WORD		min_width;
			WORD		min_height;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RectRoomToRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (source_room && dest_room)
	{
#if SHOW_US
		printf ("\tRectRoomToRoom %d (%d, %d)(%d, %d)\n", fDirection, left, top, right, bottom);
#endif

		sr_width	= ((LayerType *) Head(&source_room->Layers))->ctilx;
		sr_height	= ((LayerType *) Head(&source_room->Layers))->ctily;
		dr_width	= ((LayerType *) Head(&dest_room->Layers))->ctilx;
		dr_height	= ((LayerType *) Head(&dest_room->Layers))->ctily;

		min_height	= min (sr_height, dr_height);
		min_width	= min (sr_width, dr_width);

		top		= min (top, bottom);
		left	= min (left, right);
		bottom	= max (top, bottom);
		right	= max (left, right);

		if ((right >= 0) && (bottom >= 0) &&
				(top < min_height) && (left < min_width))
		{

			left	= max (left, 0);
			top		= max (top, 0);
			right	= min (right, (min_width - 1));
			bottom	= min (bottom, (min_height - 1));

			max_column	= abs(right - left) + 1;
			max_row		= abs(bottom - top) + 1;

#if SHOW_US
			printf ("\t(after clipping : (%d, %d)(%d, %d))\n", left, top, left + max_column - 1, top + max_row - 1);
#endif

#if PLOTARRAY
			source_room_row_add	= sr_width;
			dest_room_row_add	= dr_width;
#endif

			source_room_layer	= Head (&source_room->Layers);
			dest_room_layer	= Head (&dest_room->Layers);

			while ((! IsEOList (source_room_layer)) && (! IsEOList (dest_room_layer))) {

#if PLOTARRAY
				source_room_plot_hold	= source_room_layer->Plot;
				source_room_plot_hold	+= ((source_room_row_add * top) + left);

				dest_room_plot_hold		= dest_room_layer->Plot;
				dest_room_plot_hold		+= ((dest_room_row_add * top) + left);

				for (row = max_row; row != 0; row --) {

					source_room_plot	= source_room_plot_hold;
					dest_room_plot		= dest_room_plot_hold;

					if (fDirection == SWAPROOMRECT) {
						for (column = max_column; column != 0; column --) {
							PlotType ptTemp	= *source_room_plot;

							*source_room_plot++	= *dest_room_plot;
							*dest_room_plot++	= ptTemp;
						}
					} else {
						for (column = max_column; column != 0; column --) {
							*dest_room_plot++	= *source_room_plot++;
						}
					}

					source_room_plot_hold	+= source_room_row_add;
					dest_room_plot_hold		+= dest_room_row_add;
				}
#else // !PLOTARRAY
#if UseSparseArray
				if (fDirection == SWAPROOMRECT)
				{
					if (IsSparse(source_room_layer) && IsSparse(dest_room_layer))
					{
						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt1, plt2;

								ReadSparsePlotXY(source_room_layer->p.pspa, itilx, row+top, &plt1);
								ReadSparsePlotXY(dest_room_layer->p.pspa, itilx, row+top, &plt2);
								WriteSparsePlotXY(&plt1, dest_room_layer->p.pspa, itilx, row+top);
								WriteSparsePlotXY(&plt2, source_room_layer->p.pspa, itilx, row+top);
							}
						}
					}
					else if (IsSparse(source_room_layer) && !IsSparse(dest_room_layer))
					{
						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt1, plt2;

								ReadSparsePlotXY(source_room_layer->p.pspa, itilx, row+top, &plt1);
								ReadNPlotXY(dest_room_layer->p.rgrgplt, itilx, row+top, 1, &plt2);
								WriteNPlotXY(&plt1, 1, dest_room_layer->p.rgrgplt, itilx, row+top);
								WriteSparsePlotXY(&plt2, source_room_layer->p.pspa, itilx, row+top);
							}
						}
					}
					else if (!IsSparse(source_room_layer) && IsSparse(dest_room_layer))
					{
						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt1, plt2;

								ReadNPlotXY(source_room_layer->p.rgrgplt, itilx, row+top, 1, &plt1);
								ReadSparsePlotXY(dest_room_layer->p.pspa, itilx, row+top, &plt2);
								WriteSparsePlotXY(&plt1, dest_room_layer->p.pspa, itilx, row+top);
								WriteNPlotXY(&plt2, 1, source_room_layer->p.rgrgplt, itilx, row+top);
							}
						}
					}
					else
					{
						for (row = 0; row < max_row; row++)
						{
							WORD w;

							dest_room_plot   = ActivatePlotXYatWin(dest_room_layer->p.rgrgplt, left, row+top, winDst);
							source_room_plot = ActivatePlotXYatWin(source_room_layer->p.rgrgplt, left, row+top, winSrc);

							for (w = 0; w < max_column; w++)
							{
								PlotType ptTemp	= *source_room_plot;

								*source_room_plot++	= *dest_room_plot;
								*dest_room_plot++	= ptTemp;
							}
							UpdatePlotRow(source_room_layer->p.rgrgplt, row+top);
							UpdatePlotRow(dest_room_layer->p.rgrgplt, row+top);
						}
					}
				}
				else
				{
					if (IsSparse(source_room_layer) && IsSparse(dest_room_layer))
					{
						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt;

								ReadSparsePlotXY(source_room_layer->p.pspa, itilx, row+top, &plt);
								WriteSparsePlotXY(&plt, dest_room_layer->p.pspa, itilx, row+top);
							}
						}
					}
					else if (IsSparse(source_room_layer) && !IsSparse(dest_room_layer))
					{
						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt;

								ReadSparsePlotXY(source_room_layer->p.pspa, itilx, row+top, &plt);
								WriteNPlotXY(&plt, 1, dest_room_layer->p.rgrgplt, itilx, row+top);
							}
						}
					}
					else if (!IsSparse(source_room_layer) && IsSparse(dest_room_layer))
					{
						for (row = 0; row < max_row; row++)
						{
							int itilx;

							for (itilx = left; itilx < left+max_column; itilx++)
							{
								PlotType plt;

								ReadNPlotXY(source_room_layer->p.rgrgplt, itilx, row+top, 1, &plt);
								WriteSparsePlotXY(&plt, dest_room_layer->p.pspa, itilx, row+top);
							}
						}
					}
					else
					{
						for (row = 0; row < max_row; row++)
						{
							dest_room_plot   = ActivatePlotXYatWin(dest_room_layer->p.rgrgplt, left, row+top, winDst);
							source_room_plot = ActivatePlotXYatWin(source_room_layer->p.rgrgplt, left, row+top, winSrc);

							memcpy(dest_room_plot, source_room_plot, max_column * PLOTSIZE);

							ReleasePlotRow(source_room_layer->p.rgrgplt, row+top);
							UpdatePlotRow(dest_room_layer->p.rgrgplt, row+top);
						}
					}
				}
#else // !UseSparseArray
				for (row = 0; row < max_row; row++)
				{
					dest_room_plot   = ActivatePlotXYatWin(dest_room_layer->rgrgplt, left, row+top, winDst);
					source_room_plot = ActivatePlotXYatWin(source_room_layer->rgrgplt, left, row+top, winSrc);

					if (fDirection == SWAPROOMRECT)
					{
						WORD w;

						for (w = 0; w < max_column; w++)
						{
							PlotType ptTemp	= *source_room_plot;

							*source_room_plot++	= *dest_room_plot;
							*dest_room_plot++	= ptTemp;
						}
						UpdatePlotRow(source_room_layer->rgrgplt, row+top);
						UpdatePlotRow(dest_room_layer->rgrgplt, row+top);
					}
					else
					{
						memcpy(dest_room_plot, source_room_plot, max_column * PLOTSIZE);

						ReleasePlotRow(source_room_layer->rgrgplt, row+top);
						UpdatePlotRow(dest_room_layer->rgrgplt, row+top);
					}
				}
#endif // !UseSparseArray
#endif // !PLOTARRAY
				source_room_layer = Next (source_room_layer);
				dest_room_layer = Next (dest_room_layer);
			} /* while */
		} /* in bounds */
#if SHOW_US
		else
		{
		printf ("\t(failed bounds : (%d, %d)(%d, %d))\n", left, top, right, bottom);
		}
#endif
	}
#if SHOW_US
	else
	{
		printf ("\tRectRoomToRoom, NO UNDO ROOM.\n");
	}
#endif

	return;

} /* RectRoomToRoom */
#endif // !PixelLayers


/************************** U N D O  S T U F F *************************/

static	WORD	U_left;
static	WORD	U_right;
static	WORD	U_top;
static	WORD	U_bottom;

static	WORD	U_flag	= FALSE;

static	RoomType	*ChangedRoom	= NULL;

BOOL RoomIsUsedByUndo (RoomType *room)
{
	return ((BOOL) (room == ChangedRoom));
}


void InvalidateUndo (void)
{
	U_flag		= FALSE;
	ChangedRoom	= NULL;

#if SHOW_US
	printf ("InvalidateUndo()\n");
#if FUNC_NAMES
	printf ("\t>%s\n", CurrentFuncName);
#endif
#endif

	return;
}

void StartUndo (
	RoomType	*room,
	RoomType	*undo_room,
	WORD		left,
	WORD		top,
	WORD		right,
	WORD		bottom
)
{
	U_left		= max (left, 0);
	U_right		= min (right, (((LayerType *) Head(&undo_room->Layers))->ctilx - 1));
	U_top		= max (top, 0);
	U_bottom		= min (bottom, (((LayerType *) Head(&undo_room->Layers))->ctily - 1));

	U_flag		= TRUE;
	ChangedRoom	= room;

#if SHOW_US
	printf ("StartUndo U(%d, %d) U(%d, %d)\n", U_left, U_top, U_right, U_bottom);
#endif

	RectRoomToRoom (room, undo_room,
				U_left, U_top, U_right, U_bottom, COPYROOMRECT);

	return;
}

void UpdateUndo (
	RoomType	*room,
	RoomType	*undo_room,
	WORD		o_left,
	WORD		o_top,
	WORD		o_right,
	WORD		o_bottom
)
{
	WORD	c_left, c_right, c_top, c_bottom;

	if (U_flag) {
		ChangedRoom	= room;	/* may want to take out room as a         */
							/* parameter and just use ChangedRoom (?) */

		o_left		= max (o_left, 0);
		o_right		= min (o_right, (((LayerType *) Head(&undo_room->Layers))->ctilx - 1));
		o_top		= max (o_top, 0);
		o_bottom		= min (o_bottom, (((LayerType *) Head(&undo_room->Layers))->ctily - 1));

#if SHOW_US
		printf ("UpdateUndo o(%d, %d) o(%d, %d)\n", o_left, o_top, o_right, o_bottom);
#endif

		c_left		= min (U_left, o_left);
		c_right		= max (U_right, o_right);

		c_top		= min (U_top, o_top);
		c_bottom	= max (U_bottom, o_bottom);

		/* Phase I */
		if (U_top != c_top) {
			RectRoomToRoom (room, undo_room,
						c_left, c_top, c_right, ((WORD) (U_top - 1)),
						COPYROOMRECT);
		}

		/* Phase II */
		if (U_bottom != c_bottom) {
			RectRoomToRoom (room, undo_room,
						c_left, ((WORD) (U_bottom + 1)), c_right, c_bottom,
						COPYROOMRECT);
		}

		/* Phase III */
		if (U_left != c_left) {
			RectRoomToRoom (room, undo_room,
						c_left, U_top, ((WORD) (U_left - 1)), U_bottom,
						COPYROOMRECT);
		}

		/* Phase IV */
		if (U_right != c_right) {
			RectRoomToRoom (room, undo_room,
						((WORD) (U_right + 1)), U_top, c_right, U_bottom,
						COPYROOMRECT);
		}

		U_left		= c_left;
		U_right		= c_right;
		U_top		= c_top;
		U_bottom	= c_bottom;

	}
	else {
#if SHOW_US
		printf ("UpdateUndo with non-valid undo!!!\n", o_left, o_top, o_right, o_bottom);
#endif
	}

	return;
}


void RetrieveUndo (
	RoomType	*undo_room
)
{
	if (U_flag)
	{
		if ((ChangedRoom->Flags & LOCKEDROOM) || (ChangedRoom->Flags & SOURCEROOM))
		{
		}
		else
		{
			/* Subtract out tile usage counts for area to be Undone */

			AddTilesInSubRect(&ChangedRoom->Layers, U_left, U_top,
						U_right - U_left + 1, U_bottom - U_top + 1,
						-1, &ChangedRoom->prgtct, &ChangedRoom->fcntValid);

			RectRoomToRoom (undo_room, ChangedRoom,
						U_left, U_top, U_right, U_bottom,
						SWAPROOMRECT);

			/* Add in tile usage counts for newly Undone area */

			AddTilesInSubRect(&ChangedRoom->Layers, U_left, U_top,
						U_right - U_left + 1, U_bottom - U_top + 1,
						1, &ChangedRoom->prgtct, &ChangedRoom->fcntValid);

/*			InvalidateUndo ();	Allow Undo/Redo */

#if SHOW_US
			printf ("RetrieveUndo U(%d, %d) U(%d, %d)\n", U_left, U_top, U_right, U_bottom);
#endif
		}
	}
	else
	{
#if SHOW_US
		printf ("RetrieveUndo, no valid undo.\n", U_left, U_top, U_right, U_bottom);
#endif
	}

	return;
}

