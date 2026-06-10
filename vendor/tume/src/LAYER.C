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
 * LAYER.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 11/08/94
 *   MODIFIED : 11/08/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Add SetLayerLimits() and GetNextPlotInRange() which deal with
 *		rectangular array layers which operate in the same fashion as
 *		SetSparseLayerLimits() and GetNextSparsePlotInRange() deal with
 *		sparse arrays.
 *
 * HISTORY
 *		11/08/94 (dcc) - Created.
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>

#if UseSparseArray

#include "tuglbl.h"

#include "layer.h"							/* Verify function prototypes. */
#include "sparse.h"
#include "sparsef.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

static int dtilxL, dtilyT, dtilxRp1, dtilyBp1;
static int dtilxLast, dtilyLast;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * SetLayerLimits
 *
 * PURPOSE
 *		Set layer limits for subsequent calls to GetNextPlotInRange().
 *
 * INPUT
 *		LayerType *play	: layer to check
 *		int dtilx			: left edge of tiles to include
 *		int dtily			: top edge of tiles to include
 *		int ctilx			: width of tiles to include (if 0, all x values)
 *		int ctily			: height of tiles to include (if 0, all y values)
 *
 * ASSUMES
 *		Since we save the input in local static variables, this routine assumes
 *		that only one two-dimensional layer is processed at a time.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		11/08/94 (dcc) - Created.
 *
*/
void SetLayerLimits(LayerType *play, int dtilx, int dtily, int ctilx, int ctily)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetLayerLimits";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsSparse(play))
	{
		SetSparseLayerLimits(play->p.pspa, dtilx, dtily, ctilx, ctily);
	}
	else
	{
		dtilxL	= dtilx;
		dtilyT	= dtily;
		dtilxRp1	= dtilx + ctilx;
		dtilyBp1	= dtily + ctily;
		dtilxLast= dtilxRp1-1;
		dtilyLast= dtilyT-1;
	}
} /* SetLayerLimits */


/*********************************************************************
 *
 * GetNextPlotInRange
 *
 * PURPOSE
 *		Return the next plot that is in the x-range <dtilx>..<dtilx+ctilx-1>
 *		and in the y-range <dtily>..<dtily+ctily-1>.
 *
 * INPUT
 *		LayerType *play	: layer to get plot from
 *
 * ASSUMES
 *		XTRA memory window <winSrc> is available.
 *
 *		Since we save the input in local static variables, this routine assumes
 *		that only one two-dimensional layer is processed at a time.
 *
 * RETURN VALUE
 *		Pointer to plot, or NULL if there are no more plots in range.
 *		int *pdtilx			: filled with x-coordinate of plot
 *		int *pdtily			: filled with y-coordinate of plot
 *
 * HISTORY
 *		11/08/94 (dcc) - created.
 *
*/
PlotType *GetNextPlotInRange(LayerType *play, int *pdtilx, int *pdtily)
{
	static PlotType *pplt = NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetNextPlotInRange";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (IsSparse(play))
	{
		return GetNextSparsePlotInRange(play->p.pspa, pdtilx, pdtily);
	}
	else
	{
		dtilxLast++;
		pplt++;

		while (dtilyLast < dtilyBp1)
		{
			for ( ; dtilxLast < dtilxRp1; dtilxLast++)
			{
				if (pplt->TileSet_ID)
				{
					*pdtilx = dtilxLast;
					*pdtily = dtilyLast;
					return pplt;
				}
			}
			if (dtilyLast >= dtilyT)
				ReleasePlotRow(play->p.rgrgplt, dtilyLast);

			dtilyLast++;

			if (dtilyLast < dtilyBp1)
			{
				dtilxLast = 0;
				pplt = ActivatePlotRowatWin(play->p.rgrgplt, dtilyLast, winSrc);
			}
		}
		*pdtilx = 0;
		*pdtily = 0;
		return NULL;
	}
} /* GetNextPlotInRange */
#endif // UseSparseArray

