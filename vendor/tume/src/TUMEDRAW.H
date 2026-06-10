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
 * TUMEDRAW.H
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 04/09/92
 *   MODIFIED : 10/19/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		
 *
 * HISTORY
 *
*/

#ifndef TUMEDRAW_H
#define TUMEDRAW_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/

#define fSetStampPaint		0
#define fSetStampReplace	1

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

extern UBYTE ubGlobalShowFlagAND;
extern UBYTE ubGlobalShowFlagOR;
extern UBYTE ubGlobalShowFlagXOR;

/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

WORD wSrcSizeToDstSize(WORD w, unsigned int wSrcSkip, unsigned int wDstDup);

WORD wSrcSizeToDstSizeWithMod(WORD w,
			unsigned int wSrcSkip, unsigned int wDstDup, unsigned int wMod);

void StampTile(ByteMap *pbmpDst, PlotType *ppt, LONG lX, LONG lY,
				BOOL fNotTransparent, UBYTE fFlags);

void ShowLayersInSubRect(ListType *pLayers, LayerType *playFloor,
					RoomWindowType *roomwindow, ByteMap *pbm,
					short dx, short dy, short cx, short cy,
					short dtilx, short dtily, short ctilx, short ctily,
					BOOL fShowGuide, BOOL fCompositeSource);

void HideBrush(void);
void ShowBrush(void);
void SetDestinationClip(void);
void SetStampMode(short f);

#endif /* TUMEDRAW_H */

