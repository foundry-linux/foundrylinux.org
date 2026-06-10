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
 * TILEBITS.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 01/19/92
 *   MODIFIED : 07/28/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Header file for color masks.
 *
 * HISTORY
 *
*/

#ifndef TILEBITS_H
#define TILEBITS_H

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/etypes.h>
#include <echidna/events.h>
#include <echidna/hitareas.h>

/**************************** C O N S T A N T S ***************************/

/*** CLOR GROUP FLAGS AND VALUES ***/
#if __AMIGAOS__
#define COLORGROUP_FLAG		(0x80)
#define	ubColorGroupMask	(0x03)
#define	COLORGROUP_NUMBER(value)	(((UBYTE) (value)) & ubColorGroupMask)
#endif/*__AMIGAOS__*/

#define wColorMaskMax (256)

#define wTileMaskGroup		(0)
#define wCursorGroup			(1)
#define wZoomGroup			(2)
#define wDisplayMaskGroup	(3)
#define wShiftLayerGroup	(4)
#define wGroupMax				(5)

#define wZOOMINDEX	(0x7FFF)
#define fSHOWZOOM		(0x8000)

/******************************** T Y P E S *******************************/

typedef struct
{
	NodeType	Node;
	UBYTE ubEnableANDMask;
	UBYTE ubConfigANDMask;
	UBYTE ubPixelANDMask[wColorMaskMax];
	UBYTE ubPixelORMask[wColorMaskMax];
} ColorGroupMask;

/****************************** G L O B A L S *****************************/

extern UBYTE ubXFlipBit;
extern UBYTE ubYFlipBit;

extern WORD wShowXTiles;
extern WORD wShowYTiles;

extern WORD wZoomInOne;
extern WORD wZoomOutOne;

/******************************* M A C R O S ******************************/

#define IsZooming(prt)		(((prt)->wZoomIndex) & fSHOWZOOM)
#define SetZoomOff(prt)		((prt)->wZoomIndex) &= wZOOMINDEX
#define SetZoomOn(prt)		((prt)->wZoomIndex) |= fSHOWZOOM

#define GetZoomOutOne()		(wZoomOutOne)
#define GetZoomInOne()		(wZoomInOne)
#define SetZoomOutOne(w)	wZoomOutOne = (w);
#define SetZoomInOne(w)		wZoomInOne = (w);

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern void OpenColorGroupMask(void);
extern ColorGroupMask *AddNewColorGroup(void);
extern ColorGroupMask *FirstColorGroup(void);
extern UBYTE GetColorEnableANDMask(ColorGroupMask *pcgm);
extern UBYTE GetColorConfigANDMask(ColorGroupMask *pcgm);
extern UBYTE GetPixelANDMask(ColorGroupMask *pcgm, WORD w);
extern UBYTE GetPixelORMask(ColorGroupMask *pcgm, WORD w);
extern void SetColorEnableANDMask(ColorGroupMask *pcgm, UBYTE ub);
extern void SetColorConfigANDMask(ColorGroupMask *pcgm, UBYTE ub);
extern void SetColorMasks(ColorGroupMask *pcgm,
				UBYTE ubTileANDMask, UBYTE ubTileORMask,
				UBYTE ubPixelANDMask, UBYTE ubPixelORMask);
extern EventState *GetFirstEvent(WORD wGroup);
extern char *SaveEventString(WORD wGroup, char *sz);
extern void SaveMaskEventMask(WORD w, UBYTE ubANDMaskL, UBYTE ubORMaskL, UBYTE ubXORMaskL,
										UBYTE ubANDMaskR, UBYTE ubORMaskR, UBYTE ubXORMaskR);
extern BOOL AllocateTileMaskArrays(WORD wCnt);
extern void SaveCursorMovement(WORD w, WORD wDX, WORD wDY);
extern BOOL AllocateCursorMovements(WORD wCnt);
extern void SaveShiftLayer(WORD w, WORD wDX, WORD wDY);
extern BOOL AllocateShiftLayer(WORD wCnt);
extern void SaveZoomAmount(WORD w, WORD wDst, WORD wSrc);
extern BOOL AllocateZoomEventArrays(WORD wCnt);
extern BOOL AddEmptyEvents(WORD wGroup, WORD wCnt, WORD wSize);
extern BOOL AllocateDisplayMaskArrays(WORD wCnt);
extern void SaveDisplayMask(EventState *pes, WORD w, UBYTE ubANDMask, UBYTE ubORMask, UBYTE ubXORMask);

extern UBYTE GetTileBitsANDMaskL(void);
extern UBYTE GetTileBitsORMaskL(void);
extern UBYTE GetTileBitsXORMaskL(void);
extern UBYTE GetTileBitsANDMaskR(void);
extern UBYTE GetTileBitsORMaskR(void);
extern UBYTE GetTileBitsXORMaskR(void);

extern short TileMaskEvent(EventInfo *ei);
extern short DisplayMaskEvent(EventInfo *ei);
extern short MoveCursorEvent(EventInfo *ei);
extern short ShiftLayerEvent(EventInfo *ei);
extern short SetZoomEvent(EventInfo *ei);

extern WORD GetZoomMax(void);
extern WORD wGetZoomDstDup(WORD w);
extern WORD wGetZoomSrcSkip(WORD w);
#endif /* TILEBITS_H */

