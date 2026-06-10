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
 * SWITCHES.H
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 12/22/89
 *   MODIFIED : 10/11/94
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
 *		10/14/93 Thursday (dcc) - updated for MS-DOS Watcom C32.
 *
*/
#ifndef SWITCHES_H
#define SWITCHES_H

#include <echidna/platform.h>

/******************************** SWITCHES ********************************/

#define	ERRORS				1
#define	SHOW_MEM			0
#define NO_ERROR_NUMBERS	0
#define DEBUG_LISTFUNC		0
#define PLOTARRAY			0

#define MEM_TEST			0
#define	FUNC_NAMES			0
#define REQUIRE				1

#define USE_GRAFX			1

#define NEWTUME				1
#define PIXELSELECT			1
#define TEST				0
#define __INPUTONLY__		0
#define TEST_PRINT			0

#if FUNC_NAMES
extern char	*CurrentFuncName;
#endif

#if MEM_TEST
#include "memdbug.h"
#endif

#define LINK_INTO_TUME		1
#define DifferentTileSizes	1
#define SaveNew				1
#define UseSparseArray		1

#endif /* SWITCHES_H */

