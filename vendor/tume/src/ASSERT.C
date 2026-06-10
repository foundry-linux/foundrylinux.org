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
 * ASSERT.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 09/29/94
 *   MODIFIED : 09/29/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Routine to handle Assert().
 *
 * HISTORY
 *		09/29/94 (dcc) - created.
 *
*/

#include <echidna/platform.h>

//#include <stdio.h>
//#include <stdlib.h>
#include "tuglbl.h"
#include "assert.h"

#if DEBUG

/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * __Assert
 *
 * PURPOSE
 *		Show the Assertion failed message.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		
 *
 * HISTORY
 *		09/29/94 (dcc) - created.
 *
*/
void __Assert(char *__cond, char *__file, int __line)
{
#if 1	// tUME version
	static char sz[256];

	sprintf (sz, "Assertion failed: %s, file %s, line %d", __cond, __file, __line);

	Quit(sz);
#else	// "normal" version
	printf ("Assertion failed: %s, file %s, line %d\n", __cond, __file, __line);

	exit(1);
#endif
} /* __Assert */
#endif	/* !DEBUG */

