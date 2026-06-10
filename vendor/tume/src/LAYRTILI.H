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
 * LAYRTILI.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 06/07/93
 *   MODIFIED : 02/09/95
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Local header file for LAYRTILI.C and LAYRTILE.C.
 *
 * HISTORY
 *
*/

#ifndef LAYRTILI_H
#define LAYRTILI_H

#include <echidna/platform.h>
#include "switches.h"

#include "tudef.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef struct LayerTiles
{
	NodeType		 Node;
	RoomType		*prmLast;			/* last source room associated with this layer */
	ULONG			 dxLast;				/* last x pixel position displayed in room */
	ULONG			 dyLast;				/* last y pixel position displayed in room */
	int			 cntType;			/* count of how many TS_type are there? */
	unsigned		*TS_Type;			/* tileset types associated with this layer */
} LYT;

/****************************** G L O B A L S *****************************/

extern ListType *plstLYT;

/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

short ReadLayerType(SectionTracker *psec);
short OpenLayerType(void);

#endif /* LAYRTILI_H */

