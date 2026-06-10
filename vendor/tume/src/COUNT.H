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
 * COUNT.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 02/21/92
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
 *		Header file for tile count routines.
 *
 * HISTORY
 *
*/

#ifndef COUNT_H
#define COUNT_H

#include <echidna/platform.h>
#include "switches.h"

#include "tudef.h"

/**************************** C O N S T A N T S ***************************/

#define bMergeDuplicates	(0x01)
#define bMergeXFlips			(0x40)
#define bMergeYFlips			(0x20)
#define bMergeXYFlips		(0x80)

/* wGroupCntMax sez we can have up to 16 groups. Each group is represented
	by one bit in TileSetType->bGroupTileNum and TileUseFlag->bGroupMatch.
	The lowest bit in each case corresponds to the first count group. */

#define wGroupCntMax (16)

/******************************** T Y P E S *******************************/

typedef struct
{
	NodeType Node;
	UWORD uwTileLow;
	UWORD uwTileHigh;
	UWORD uwRoomLow;
	UWORD uwRoomHigh;
	BOOL fRoom;
} GroupSet;

typedef ListType GroupDefList;

/****************************** G L O B A L S *****************************/

extern UBYTE ubCharacterANDMask;

extern UWORD uwCharWidth;
extern UWORD uwCharHeight;

extern UWORD uwCharMax[wGroupCntMax];
extern GroupDefList GroupDef[wGroupCntMax];
extern UWORD bGroupFlags[wGroupCntMax];

/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

BOOL OpenCountChars(WORD wMax);
BOOL DoCountChars(TileSpaceType *pTileSpace, MapType *pMT, RoomType *pRT);

#endif /* COUNT_H */
