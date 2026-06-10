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
 * MOUSEITEMS.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 10/19/94
 *   MODIFIED : 03/19/95
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		Function prototypes for MOUSEITEMS.C.
 *		Currently this files is a KLUDGE used by MSDOS.C.
 *
 * HISTORY
 *		10/19/94 (dcc) - created.
 *
*/

#ifndef MOUSEITEMS_H
#define MOUSEITEMS_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

extern HitInfo hitGlobal;

/******************************* M A C R O S ******************************/

#define FSelected() (GlobalBlockCopy && GlobalBlockCopy->BC_FloorLayer != NULL)

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

void RealButtonsE (BOOL buttons);

#endif /* MOUSEITEMS_H */

