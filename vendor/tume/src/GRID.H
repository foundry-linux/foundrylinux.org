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
 * GRID.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *   MODIFIED : 07/28/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		GRID and GUID header stuff.
 *
 * HISTORY
 *
*/

#ifndef GRID_H
#define GRID_H

/**************************** C O N S T A N T S ***************************/

#if NEWTUME
#define cDefaultGuideLinesInit	4

/* [0] - horizontal lines, [1] - vertical lines */

#define cenDefaultGuideA0Init			  0L
#define cenDefaultGuideB0Init			100L
#define cenDefaultGuideDC0Init		200L
#define cenDefaultGuideXOrigin0Init	  0L
#define cenDefaultGuideYOrigin0Init	  0L
#define cenDefaultGuideA1Init			100L
#define cenDefaultGuideB1Init			  0L
#define cenDefaultGuideDC1Init		200L
#define cenDefaultGuideXOrigin1Init	  0L
#define cenDefaultGuideYOrigin1Init	  0L
#endif

extern WORD wDefaultGridXWidth;
extern WORD wDefaultGridYHeight;
extern WORD wDefaultGridXOrigin;
extern WORD wDefaultGridYOrigin;
extern BOOL fDefaultUseGrid;

#if NEWTUME
/* ASSERT: fDefaultShowGuide[0] & fDefaultShowGuide[1] == fDefaultGuideVisible */

extern WORD cDefaultGuideLines;
extern LONG *pcenDefaultGuideA;
extern LONG *pcenDefaultGuideB;
extern LONG *pcenDefaultGuideDC;
extern LONG *pcenDefaultGuideXOrigin;
extern LONG *pcenDefaultGuideYOrigin;
extern BOOL	*fDefaultShowGuide;
#else
extern WORD wDefaultGuideXWidth;
extern WORD wDefaultGuideYHeight;
extern WORD wDefaultGuideXOrigin;
extern WORD wDefaultGuideYOrigin;
#endif
extern BOOL fDefaultGuideVisible;

extern WORD wDefaultZoomIndex;

#define SetDefaultGridX(w)				(wDefaultGridXWidth = w)
#define SetDefaultGridY(w)				(wDefaultGridYHeight = w)
#define SetDefaultGridXOrigin(w)		(wDefaultGridXOrigin = w)
#define SetDefaultGridYOrigin(w)		(wDefaultGridYOrigin = w)
#define SetDefaultUseGrid(w)			(fDefaultUseGrid = w)

#define SetDefaultGuideX(w)			(wDefaultGuideXWidth = w)
#define SetDefaultGuideY(w)			(wDefaultGuideYHeight = w)
#define SetDefaultGuideXOrigin(w)	(wDefaultGuideXOrigin = w)
#define SetDefaultGuideYOrigin(w)	(wDefaultGuideYOrigin = w)
#define SetDefaultGuideVisible(w)	(fDefaultGuideVisible = w)

#define SetDefaultZoomIndex(w)		(wDefaultZoomIndex = w)

#define wGetDefaultGridX()				(wDefaultGridXWidth)
#define wGetDefaultGridY()				(wDefaultGridYHeight)
#define wGetDefaultGridXOrigin()		(wDefaultGridXOrigin)
#define wGetDefaultGridYOrigin()		(wDefaultGridYOrigin)
#define fGetDefaultUseGrid()			(fDefaultUseGrid)

#if NEWTUME
#else
#define wGetDefaultGuideX()			(wDefaultGuideXWidth)
#define wGetDefaultGuideY()			(wDefaultGuideYHeight)
#define wGetDefaultGuideXOrigin()	(wDefaultGuideXOrigin)
#define wGetDefaultGuideYOrigin()	(wDefaultGuideYOrigin)
#endif
#define fGetDefaultGuideVisible()	(fDefaultGuideVisible)

#define wGetDefaultZoomIndex()		(wDefaultZoomIndex)

#endif /* GRID_H */
