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
 * READtUME.H
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
 *		Function prototypes for READtUME.C.
 *
 * HISTORY
 *		10/19/94 (dcc) - created.
 *
*/

#ifndef READtUME_H
#define READtUME_H

#include <echidna/platform.h>
#include "switches.h"

#include "tuglbl.h"

/**************************** C O N S T A N T S ***************************/

/* These two constants are used by menuitem#AppendorLoadLayer(),
	and tiosup#InsertLayersIntoRoom(). */

#define APPEND_LAYER		(0x00)
#define LOAD_LAYER		(0x01)

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

extern ListType *PathList;

/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

void SetLoadLayer(LayerType *play, short f);
short LoadMap (char *filename);

#endif /* READtUME_H */

