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
 * HISTOGRM.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 06/23/93
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
 *		Header file for HISTOGRM.C.
 *
 * HISTORY
 *		06/23/93 Wednesday (dcc) - created.
 *
*/

#ifndef HISTOGRM_H
#define HISTOGRM_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern void InvalidateEditRoomTileCounts(XTRAPntr **pprgtct, BOOL *pfcntValid);
extern void FreeTileCounts(RoomType *prm);
extern void RevampTileCounts(TileSpaceType *ptsp);

extern BOOL AddTilesInSubRect(ListType *plys, short dtilx, short dtily,
								short ctilx, short ctily, short dir,
								XTRAPntr **pprgtct, BOOL *pfcntValid);

extern void ShowCountsInSubRect(RoomType *prm,
								RoomWindowType *roomwindow,
								short dx, short dy, short cx, short cy,
								short dtilDstX, short dtilDstY,
								short dtilx, short dtily, short ctilx, short ctily);

extern UWORD GetLargestTileUsageCount(void);
extern void SetLargestTileUsageCount(UWORD cnt);

#endif /* HISTOGRM_H */
