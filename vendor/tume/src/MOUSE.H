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
 * MOUSE.H
 *
 * PROGRAMMER : Gregg A. Tavares 
 *    VERSION : 00.000
 *    CREATED : 08/29/89 
 *   MODIFIED : 10/19/94
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *
 *
 * HISTORY
 *
*/
#ifndef MOUSE_H
#define MOUSE_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef void (*voidfuncptr)(void);

/****************************** E X T E R N S *****************************/

extern int		MouseX;
extern int		MouseY;
extern int		Shifted;
extern int		LeftButton;
extern int		RightButton;

extern int		DontWait;

/****************************** G L O B A L S *****************************/


/******************************* L O C A L S ******************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern voidfuncptr	FN_LeftUp;
extern voidfuncptr	FN_LeftDown;
extern voidfuncptr	FN_RightUp;
extern voidfuncptr	FN_RightDown;

#if __AMIGAOS__
extern voidfuncptr	FN_DontWait;
#endif /*__AMIGAOS__*/

#endif /* MOUSE_H */

