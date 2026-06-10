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
 * RM_TSINF.H
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 03/17/92
 *   MODIFIED : 02/09/95
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

#ifndef RM_TSINF_H
#define RM_TSINF_H

#include <echidna/platform.h>
#include "switches.h"

#include "tuglbl.h"

/**************************** C O N S T A N T S ***************************/

#define itstLayer		0
#define itstFlag		1
#define itstTransparent	2
#define itstcxDisplay	3
#define itstcyDisplay	4
#define itstctilxSkip	5
#define itstctilxDispO	6
#define itstctilyDispO	7
#define itstctilxDispE	8
#define itstctilyDispE	9

#define ctstInfoN		10

#define LayerToUseDefault	-1
#define ubShowFlagDefault	-1
#define fTransparentDefault	0
#define cxDisplayDefault	-1
#define cyDisplayDefault	-1
#define ctilxSkipDefault	1
#define ctilxDispODefault	0
#define ctilyDispODefault	1
#define ctilxDispEDefault	0
#define ctilyDispEDefault	1

/******************************** T Y P E S *******************************/

typedef struct
{
	NodeType	 Node;
	int			 Type;
	int			 Info[ctstInfoN];
} RT_Type;

/****************************** G L O B A L S *****************************/

extern char			*RoomTypeGroup;
extern char			*TileTypeGroup;

/******************************* M A C R O S ******************************/


/****************************** E X T E R N S *****************************/

extern ListType	*TileTypeList;
extern ListType	*RoomTypeList;

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

#if 0
extern int LookupInfoN(int FindType, int iInfoN);
#else
extern void LookupSomeTSTInfoFromUserType(TileSetType *ptst);
#endif
extern short RoomNTileInfoReq (RoomType *room, TileSetType *tst,
						short *pctilx, short *pctily, short fSetCtil);

#endif /* RM_TSINF_H */

