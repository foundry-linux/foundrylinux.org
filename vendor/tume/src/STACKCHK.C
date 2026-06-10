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
 * STACKCHK.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 03/19/93
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Routine to determine how much stack remains unused.
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include <echidna/etypes.h>
#include "switches.h"
#include "switch1.h"

#if STACKCHECK

#include <stdio.h>

#if __BORLANDC__
#if defined(__COMPACT__)|defined(__LARGE__)|defined(__HUGE__)

#include <dos.h>
#include <mem.h>
#include "stackchk.h"						/* Verify function prototypes. */

/**************************** C O N S T A N T S ***************************/

/* ubFillValue: fill stack with this value. */

#define ubFillValue 0xFE

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * StackFill
 *
 * SYNOPSIS
 *		void StackFill(void)
 *
 * PURPOSE
 *		Fill stack with a known value. Call StackFill() at start of
 *		program execution, then call StackUsed() at end of program
 *		execution to determine how much of the stack was used.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void StackFill(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "StackFill";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	memset(MK_FP(_SS, 0), ubFillValue, _SP-256);

} /* StackFill */


/*********************************************************************
 *
 * StackUsed
 *
 * SYNOPSIS
 *		void StackUsed(void)
 *
 * PURPOSE
 *		Print statistics about how much of the stack was used.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void StackUsed(void)
{
	UWORD uLen;
	UBYTE *pStk = MK_FP(_SS, 0);

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "StackUsed";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (uLen = 0; uLen < _stklen; uLen++, pStk++)
		if (*pStk != ubFillValue)
			break;

	printf("Stack size: %u, Unused: %u (should be >= 512)\n", _stklen, uLen);
} /* StackUsed */


#else/*__TINY__|__SMALL__|__MEDIUM__*/
#error No support for this memory model.
#endif/* Memory Model */

#else/*__BORLANDC__*/
#error No support for this compiler.
#endif/* Compiler */

#endif/*STACKCHECK*/

