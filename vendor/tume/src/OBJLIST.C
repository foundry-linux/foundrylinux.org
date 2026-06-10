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
 * OBJLIST.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 08/24/94
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
 *		Read and initialize object type lists.
 *
 * HISTORY
 *		08/24/94 (dcc) - created (based on TYPELIST.C).
 *
*/

#include <echidna/platform.h>
#include "switches.h"
#include "switch1.h"

#if PixelLayers
#include <echidna/eerrors.h>
#include <echidna/readini.h>
#include <echidna/strings.h>

#include "tuglbl.h"
#include "parseini.h"
#include "maketile.h"
#include "objlist.h"							/* Verify function prototypes. */

/**************************** C O N S T A N T S ***************************/

#define MAX_LINE	80

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/



/*********************************************************************
 *
 * InitObjectTypeList
 *
 * PURPOSE
 *		Process [Object Types] section of tUME.INI file.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		TRUE if successful, FALSE if OOM.
 *
 * HISTORY
 *		08/24/94 (dcc) - created.
 *
*/
int InitObjectTypeList(IniList *inilist)
{
	SectionTracker	 secx;
	SectionTracker	*psec = &secx;


/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitObjectTypeList";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	InitList (plstObjectType);

	psec = FindSection (psec, inilist, "[Object Types]");
	if (psec)
	{
		ConfigLine	*pcl;

		while (((pcl = GetNextINILine (psec))) != NULL)
		{
			char			*s;
			short			 pos;

			s     = NodeName(pcl);
			pos   = 0;
			while (*s && *s != '=')
			{
				s++;
				pos++;
			}
			if (*s == '=' && pos)
			{
				OBJ_Type		*pobj;
				char			 line[MAX_LINE + 1];

				strncpy (line, NodeName (pcl), pos);
				line[pos] = '\0';
				s  = TrimWhiteSpace (line);
				if (strlen (s))
				{
					pobj = (OBJ_Type *) CreateNode (sizeof (OBJ_Type), s);
					if (!pobj)
					{
						SetGlobalErr (ERR_OUT_OF_MEMORY);
						GEcatf ("\nOOM: Reading Ojbect Types");
						return FALSE;
					}
					pobj->Type = atoi(NodeName (pcl) + pos + 1);
				}
				AddTail (plstObjectType, pobj);
			}
		}
	}
	return TRUE;
} /* InitObjectTypeList */
#endif // PixelLayers

