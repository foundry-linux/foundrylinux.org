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
 * PARAGLUE.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 02/17/93
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
 *		Header file for paraglue.c.
 *
 * HISTORY
 *
*/

#ifndef PARAGLUE_H
#define PARAGLUE_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern BOOL fParallelBlockRead(ULONG ulAddress, void *pub, ULONG ulLength);
extern BOOL fParallelBlockWrite(ULONG ulAddress, void *pub, ULONG ulLength);
extern WORD wGetParallelID(void);
extern BOOL fRestartParallelTarget(void);
extern BOOL fInitParallel(void);
extern void SetLPTn(WORD wPort);

#endif /* PARAGLUE_H */

