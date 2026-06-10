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
 * PRINTMAP.H
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 04/09/92
 *   MODIFIED : 07/28/94
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

#ifndef PRINTMAP_H
#define PRINTMAP_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/

#define HPPCL3	0
#define HPGL2	1

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

extern short			 OutputDevice;

extern long				 cencxPage;
extern long				 cencyPage;

extern short			 wPrtSrcX;
extern short			 wPrtSrcY;
extern short			 wPrtDestX;
extern short			 wPrtDestY;

#if TEST_PRINT
extern char				*PrintFile;
#endif

extern short			 PrintColorMode;
extern short			 UseLightShades;
extern short			 FirstPageOnly;
extern short			 UniformPixels;
extern short			 Resolution;
extern short		   	 SizeMode;
extern short			 IdiotNumbers;

/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern short PrintRoom (RoomWindowType	*roomwindow);
extern short PrintMapReq (RoomWindowType *roomwindow);

#endif /* PRINTMAP_H */

