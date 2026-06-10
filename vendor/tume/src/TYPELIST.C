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
 * TYPELIST.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 01/16/93
 *   MODIFIED : 02/09/95
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Read and initialize type lists.
 *
 * HISTORY
 *		06/07/93 Monday (dcc) - add support to read LayerTypes.
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/eerrors.h>
#include <echidna/strings.h>

#include "tuglbl.h"
#include "parseini.h"
#include "rm_tsinf.h"
#include "layrtili.h"

#define LOCAL static
/**************************** C O N S T A N T S ***************************/

static int InfoDefault[ctstInfoN] =
{
	LayerToUseDefault,
	ubShowFlagDefault,
	fTransparentDefault,
	cxDisplayDefault,
	cyDisplayDefault,
	ctilxSkipDefault,
	ctilxDispODefault,
	ctilyDispODefault,
	ctilxDispEDefault,
	ctilyDispEDefault
};

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * ReadTypeList
 *
 * PURPOSE
 *		
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
 *		12/14/93 Tuesday (dcc) - change Info0,Info1 to array Info[0..ctstInfoN].
 *
 * SEE ALSO
 *
*/
LOCAL short ReadTypeList (SectionTracker *sec, ListType *list)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReadTypeList";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
#define MAX_LINE	80
	{
		ConfigLine		*cl;
		RT_Type			*rt;
		char			 line[MAX_LINE + 1];
		char			*s;
		short			 pos;

		while (((cl = GetNextINILine (sec))) != NULL)
		{
			s     = NodeName (cl);
			pos   = 0;
			while (*s && *s != '=')
			{
				s++;
				pos++;
			}
			if (*s == '=' && pos)
			{
				strncpy (line, NodeName (cl), pos);
				line[pos] = '\0';
				s  = TrimWhiteSpace (line);
				if (strlen (s))
				{
					rt = (RT_Type *) CreateNode (sizeof (RT_Type), s);
					if (!rt)
					{
						SetGlobalErr (ERR_OUT_OF_MEMORY);
						GEcatf ("\nOOM: Reading Room/Tileset Types");
						return FALSE;
					}
					rt->Type = atoi (NodeName (cl) + pos + 1);
					{
						int i;
						char *psz;

						for (i = 0; i < ctstInfoN; i++)
							rt->Info[i] = InfoDefault[i];

						if ((psz = strchr(NodeName(cl)+pos+1, ',')) != NULL)
						{
							rt->Info[0] = atoi(psz+1);
							if ((psz = strchr(psz+1, ',')) != NULL)
							{
								BOOL comma = FALSE;

								if (*(psz+9) == ',')
								{
									*(psz+9) = 0;
									comma = TRUE;
								}
								if (ParseMask(psz+1, NOZEROBITS, NOXORBITS, "Tileset Types"))
								{
									rt->Info[1] = GetParseORMask();

									if (comma)
									{
										*(psz+9) = ',';

										for (i = 2; i < ctstInfoN; i++)
										{
											if ((psz = strchr(psz+1, ',')) != NULL)
												rt->Info[i] = atoi(psz+1);
											else
												break;
										}
									}
								}
							}
						}
					}
					AddTail (list, rt);
				}
			}
		}

		return TRUE;
	}
} /* ReadTypeList */


/*********************************************************************
 *
 * InitTypeLists
 *
 * SYNOPSIS
 *		short InitTypeLists (IniList *inilist)
 *
 * PURPOSE
 *		
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
short InitTypeLists (IniList *inilist)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitTypeLists";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		SectionTracker	 secx;
		SectionTracker	*sec = &secx;

		SectionTracker	 sec2x;
		SectionTracker	*sec2 = &sec2x;

		char			*s;
		char			 work[100];

		InitList (TileTypeList);
		InitList (RoomTypeList);

		sec = FindSection (sec, inilist, "[Type Groups]");
		if (sec) {

			ResetSection (sec);
			s = FindNextINILine (sec, "TileTypes=");
			if (s) {
				TileTypeGroup = dupstr (s);
				strcpy (work + 1, s);
				work[0] = '[';
				strcat (work, "]");

				sec2 = FindSection (sec2, inilist, work);
				if (sec2) {
					if (!ReadTypeList (sec2, TileTypeList)) {
						return FALSE;
					}
				}
			}

			ResetSection (sec);
			s = FindNextINILine (sec, "RoomTypes=");
			if (s) {
				RoomTypeGroup = dupstr (s);
				strcpy (work + 1, s);
				work[0] = '[';
				strcat (work, "]");

				sec2 = FindSection (sec2, inilist, work);
				if (sec2) {
					if (!ReadTypeList (sec2, RoomTypeList)) {
						return FALSE;
					}
				}
			}

			ResetSection (sec);
			s = FindNextINILine (sec, "LayerTypes=");
			if (s) {
				RoomTypeGroup = dupstr (s);
				strcpy (work + 1, s);
				work[0] = '[';
				strcat (work, "]");

				sec2 = FindSection (sec2, inilist, work);
				if (sec2) {
					if (!ReadLayerType (sec2)) {
						return FALSE;
					}
				}
			}
		}

		return TRUE;
	}
} /* InitTypeLists */

