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
 * WRITEtUME.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 10/19/94
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		Function prototypes for WRITEtUME.C.
 *
 * HISTORY
 *		10/19/94 (dcc) - created.
 *
*/

#ifndef WRITEtUME_H
#define WRITEtUME_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/

#define TMGC_MAP		((UWORD) 1 << 0)	/* Save TMGC chunks as well */
#define TMGX_MAP		((UWORD) 1 << 1)	/* Save TMGX chunks as well */
#define ONLY_FLOOR	((UWORD) 1 << 2)	/* Save only floor layers of rooms */

/*** SAVEFLAGS STUFF ***/
/* Normally, GlobalMap->SaveFlags == 0 && GlobalTileSpace->SaveFlags == 0.
	However, if you only want to save certain rooms & tilesets, then set
	both GlobalMap->SaveFlags and GlobalTileSpace->SaveFlags to TLIMITSAVEFLAG
	and mark each individual rooms & tilesets with TLIMITSAVEFLAG. */

#define TLIMITSAVEFLAG	((UWORD) 1)

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern short	SaveMap (char *filename, UWORD options);

#endif /* WRITEtUME_H */

