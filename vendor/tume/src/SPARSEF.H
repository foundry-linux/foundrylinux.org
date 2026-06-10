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
 * SPARSEF.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 08/18/94
 *   MODIFIED : 11/08/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Functional prototypes for SPARSE.C.
 *
 * HISTORY
 *		08/18/94 Thursday (dcc) - created.
 *
*/

#ifndef SPARSEF_H
#define SPARSEF_H

#include <echidna/platform.h>
#include "switches.h"
#include "switch1.h"

#if UseSparseArray
#include "rectplot.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/*********************************************************************
 *
 * LAY_ReadPlotXY MACRO
 *
 * SYNOPSIS
 *		void LAY_ReadPlotXY(LayerType *play,
 *									int dtilx, int dtily, PlotType *pplt)
 *
 * PURPOSE
 *		Read tile at coordinate <dtilx>, <dtily> from layer <play>
 *		into <pplt>.
 *
 *		This routine checks to see if we should read from the sparse
 *		array <p.pspa> or the two-dimensional array <p.rgrgplt>.
 *
 * INPUT
 *		LayerType *play	: layer to read from
 *		int dtilx			: x-tile coordinate to read tile
 *		int dtily			: y-tile coordinate to read tile
 *		PlotType *pplt		: where to store tile
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/21/94 Sunday (dcc) - created.
 *
*/
#define LAY_ReadPlotXY(play,dtilx,dtily,pplt)									\
			{																					\
				if (IsSparse(play))														\
					ReadSparsePlotXY((play)->p.pspa,(dtilx),(dtily),(pplt));	\
				else																			\
					ReadNPlotXY((play)->p.rgrgplt,(dtilx),(dtily),1,(pplt));	\
			}


/*********************************************************************
 *
 * LAY_WritePlotXY MACRO
 *
 * SYNOPSIS
 *		void LAY_WritePlotXY(PlotType *pplt, 
 *									LayerType *play, int dtilx, int dtily)
 * PURPOSE
 *		Write <pplt> to coordinate <dtilx>, <dtily> of layer <play>.
 *
 *		This routine checks to see if we are writing into the sparse
 *		array <p.pspa> or the two-dimensional array <p.rgrgplt>.
 *
 * INPUT
 *		PlotType *pplt		: tile to write
 *		LayerType *play	: layer to write to
 *		int dtilx			: x-tile coordinate to write tile
 *		int dtily			: y-tile coordinate to write tile
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/21/94 Sunday (dcc) - created.
 *
*/
#define LAY_WritePlotXY(pplt,play,dtilx,dtily)									\
			{																					\
				if (IsSparse(play))														\
					WriteSparsePlotXY((pplt),(play)->p.pspa,(dtilx),(dtily));\
				else																			\
					WriteNPlotXY((pplt),1,(play)->p.rgrgplt,(dtilx),(dtily));\
			}

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

BOOL InitSparse(void);
SparseArray *MakeNewSparseArray(void);
void SetSparseLayerLimits(SparseArray *pspa,
							int dtilx, int dtily, int ctilx, int ctily);
PlotType *GetNextSparsePlotInRange(SparseArray *pspa, int *pdtilx, int *pdtily);
PlotType *GetSparsePlotXY(SparseArray *pspa, int dtilx, int dtily);
void ReadSparsePlotXY(SparseArray *pspa, int dtilx, int dtily, PlotType *pplt);
void WriteSparsePlotXY(PlotType *pplt, SparseArray *pspa, int dtilx, int dtily);
BOOL DeleteSparseArrayRows(SparseArray *pspa0, int wRow, int wNumRows);
void FreeSparseArray(SparseArray *pspa);
void ZeroRectInSparseArray(SparseArray *pspa,
									int dtilx, int dtily, int ctilx, int ctily);
void SwapRectInSparseArray(SparseArray *pspa1, SparseArray *pspa2,
									int dtilx, int dtily, int ctilx, int ctily);

#endif // UseSparseArray
#endif /* SPARSEF_H */

