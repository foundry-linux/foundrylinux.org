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
 * RECTPLOT.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 04/05/93
 *   MODIFIED : 08/19/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Macros to deal with two dimensional array of plots stored in XTRA memory.
 *
 * HISTORY
 *		05/18/93 Tuesday (dcc) - ReleasePlotRow() added.
 *
*/

#ifndef RECTPLOT_H
#define RECTPLOT_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/*********************************************************************
 *
 * ActivatePlotXYatWin MACRO
 *
 * SYNOPSIS
 *		PlotType *ActivatePlotXYatWin(RGRGPLT rgrgplt, int wX, int wY, int wWin)
 *
 * PURPOSE
 *		This macro copies from <wX> to the end of row <wY> from 'two-
 *		dimensional array' <rgrgplt> in XTRA memory into one of the XTRA
 *		memory windows in main memory.
 *
 *		After a copy has been made in main memory, you can read and
 *		write to the copy in main memory.
 *
 *		If you write to the copy in main memory, you should call
 *		UpdatePlotRow() to update the changes back to XTRA memory.
 *
 *		If you only read from the copy in main memory, you should call
 *		ReleasePlotRow() when you are done with it.
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
 *		ActivateXTRAatWin()
 *
*/
#define ActivatePlotXYatWin(rgrgplt,wX,wY,wWin) \
			((PlotType*)ActivateXTRAatWin((rgrgplt).mpYtMpXtPt\
			[(wY)/(rgrgplt).wRowsPerXTRA],wWin)\
			+((wY)%(rgrgplt).wRowsPerXTRA)*(rgrgplt).PlotWidth+(wX))


/*********************************************************************
 *
 * ActivatePlotRowatWin MACRO
 *
 * SYNOPSIS
 *		PlotType *ActivatePlotRowatWin(RGRGPLT rgrgplt, int wY, int wWin)
 *
 * PURPOSE
 *		This macro copies row <wY> from 'two-dimensional array' <rgrgplt>
 *		in XTRA memory into one of the XTRA memory windows in main memory.
 *
 *		After a copy has been made in main memory, you can read and
 *		write to the copy in main memory.
 *
 *		If you write to the copy in main memory, you should call
 *		UpdatePlotRow() to update the changes back to XTRA memory.
 *
 *		If you only read from the copy in main memory, you should call
 *		ReleasePlotRow() when you are done with it.
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
 *		ActivateXTRAatWin()
 *
*/
#define ActivatePlotRowatWin(rgrgplt,wY,wWin) \
			((PlotType*)ActivateXTRAatWin((rgrgplt).mpYtMpXtPt\
			[(wY)/(rgrgplt).wRowsPerXTRA],wWin)\
			+((wY)%(rgrgplt).wRowsPerXTRA)*(rgrgplt).PlotWidth)


/*********************************************************************
 *
 * ActivatePlotRow MACRO
 *
 * SYNOPSIS
 *		PlotType *ActivatePlotRow(RGRGPLT rgrgplt, int wY)
 *
 * PURPOSE
 *		This macro copies row <wY> from 'two-dimensional array' <rgrgplt>
 *		in XTRA memory into XTRA memory windows 0 in main memory.
 *
 *		After a copy has been made in main memory, you can read and
 *		write to the copy in main memory.
 *
 *		If you write to the copy in main memory, you should call
 *		UpdatePlotRow() to update the changes back to XTRA memory.
 *
 *		If you only read from the copy in main memory, you should call
 *		ReleasePlotRow() when you are done with it.
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
 *		ActivateXTRA()
 *
*/
#define ActivatePlotRow(rgrgplt,wY) \
			((PlotType*)ActivateXTRA((rgrgplt).mpYtMpXtPt\
			[(wY)/(rgrgplt).wRowsPerXTRA])\
			+((wY)%(rgrgplt).wRowsPerXTRA)*(rgrgplt).PlotWidth)


/*********************************************************************
 *
 * UpdatePlotRow MACRO
 *
 * SYNOPSIS
 *		void UpdatePlotRow(RGRGPLT rgrgplt, int wY)
 *
 * PURPOSE
 *		This macro copies the changes you have made to the main memory
 *		copy of row <wY> back to 'two-dimensional array' <rgrgplt> in
 *		XTRA memory.
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
 *		UpdateXTRA()
 *
*/
#define UpdatePlotRow(rgrgplt,wY) UpdateXTRA((rgrgplt).mpYtMpXtPt\
			[(wY)/(rgrgplt).wRowsPerXTRA])


/*********************************************************************
 *
 * ReleasePlotRow MACRO
 *
 * SYNOPSIS
 *		void ReleasePlotRow(RGRGPLT rgrgplt, int wY)
 *
 * PURPOSE
 *		This macro indicates to the XTRA memory routines that we are
 *		done looking at row <wY> of the 'two-dimensional array'
 *		<rgrgplt>.
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
 *		ReleaseXTRA()
 *
*/
#define ReleasePlotRow(rgrgplt,wY) ReleaseXTRA((rgrgplt).mpYtMpXtPt\
			[(wY)/(rgrgplt).wRowsPerXTRA])


/*********************************************************************
 *
 * ReadNPlotXY MACRO
 *
 * SYNOPSIS
 *		void ReadNPlotXY(RGRGPLT rgrgplt,
 *									int wX, int wY, int wN, PlotType *pplt)
 *
 * PURPOSE
 *		Read <wN> PlotTypes from coordinates <wX>,<wY> in 'two-dimensional
 *		array' <rgrgplt> in XTRA memory to PlotType pointer <pplt>.
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
 *		ReadXTRA()
 *
*/
#define ReadNPlotXY(rgrgplt,wX,wY,wN,pplt) \
			ReadXTRA((rgrgplt).mpYtMpXtPt\
			[(wY)/(rgrgplt).wRowsPerXTRA],\
			(((wY)%(rgrgplt).wRowsPerXTRA)*(rgrgplt).PlotWidth+(wX))*PLOTSIZE,wN*PLOTSIZE,pplt)


/*********************************************************************
 *
 * WriteNPlotXY MACRO
 *
 * SYNOPSIS
 *		void WriteNPlotXY(PlotType *pplt, int wN, RGRGPLT rgrgplt,
 *									int wX, int wY)
 *
 * PURPOSE
 *		Write <wN> PlotTypes from PlotType pointer <pplt> to
 *		coordinates <wX>,<wY> in 'two-dimensional array' <rgrgplt>
 *		in XTRA memory to PlotType pointer <pplt>.
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
 *		ReadXTRA()
 *
*/
#define WriteNPlotXY(pplt,wN,rgrgplt,wX,wY) \
			WriteXTRA(pplt,wN*PLOTSIZE,(rgrgplt).mpYtMpXtPt\
			[(wY)/(rgrgplt).wRowsPerXTRA],\
			(((wY)%(rgrgplt).wRowsPerXTRA)*(rgrgplt).PlotWidth+(wX))*PLOTSIZE)


/****************** F U N C T I O N   P R O T O T Y P E S *****************/


#endif /* RECTPLOT_H */

