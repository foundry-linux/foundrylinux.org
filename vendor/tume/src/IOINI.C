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
 * IOINI.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 04/29/93
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
 *		Routines to store tileset loading search directories.
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/eio.h>
#include <echidna/readini.h>

#include "ioini.h"							/* Verify function prototypes. */
#include "readtume.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * ProcessStrings
 *
 * SYNOPSIS
 *		static BOOL ProcessStrings(IniList *plstIni, char *szSec, char *sz,
 *											ListType *plst)
 *
 * PURPOSE
 *		Add all strings in section <szSec> of <plstIni> that initially
 *		match <sz> to <plst>. <plst> should be an initialized list.
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
static BOOL ProcessStrings(IniList *plstIni, char *szSec, char *sz,
									ListType *plst)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessStrings";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (plstIni)
	{
		SectionTracker st;

		if (FindSection(&st, plstIni, szSec) != NULL)
		{
			ConfigLine *pLine;

			while ((pLine = GetNextINILine(&st)) != NULL)
			{
				char *pch;

				pch = strstr(szConfigLine(pLine), sz);

				if (pch)
				{
					NodeType *pnod;
					char sztmp[256];

					pch += strlen(sz);
					strcpy(sztmp, pch);
					EIO_InsureEndSlash(sztmp);
					if ((pnod = CreateNode(sizeof(NodeType), sztmp)) == NULL)
						return FALSE;

					AddTail(plst, pnod);
				}
			}
		}
	}
	return TRUE;

} /* ProcessStrings */


/*********************************************************************
 *
 * ProcessSearchDirs
 *
 * SYNOPSIS
 *		BOOL ProcessSearchDirs(IniList *plstIni, char *szSec, char *sz)
 *
 * PURPOSE
 *		Add all directory search paths specified in section <szSec>
 *		in <plstIni> to PathList. <sz> is the string to match, e.g.,
 *		"SearchDir=".
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
BOOL ProcessSearchDirs(IniList *plstIni, char *szSec, char *sz)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessSearchDirs";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	PathList = CreateList(NULL);

	return ProcessStrings(plstIni, szSec, sz, PathList);

} /* ProcessSearchDirs */

