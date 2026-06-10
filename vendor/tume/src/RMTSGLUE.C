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
 * RMTSGLUE.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 04/11/92
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
 *		Glue for getting room information / tileset information.
 *		Make sure room names do not clash.
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include <echidna/etypes.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>
#include <echidna/strings.h>

#include "tuglbl.h"
#include "rmtsglue.h"						/* Verify function prototypes. */
#include "rm_tsinf.h"
#include "szerror.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * GetRoomInfoReq
 *
 * SYNOPSIS
 *		BOOL GetRoomInfoReq(RoomType *prt, BOOL fSize)
 *
 * PURPOSE
 *		Get room info, make sure room name doesn't clash.
 *		If room is a composite room, set corresponding tileset name.
 *
 * INPUT
 *		*pctilx:		current width of room in tiles
 *		*pctily:		current height of room in tiles
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		10/11/94 (dcc) - Call dupstr() instead of strdup().
 *
 *
 * SEE ALSO
 *
*/
BOOL GetRoomInfoReq(RoomType *prt, short *pctilx, short *pctily, BOOL fSize)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetRoomInfoReq";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetColorsNPointer (MainWindow);

	if (prt->Name != NULL && FindDuplicateRoomName(prt, GlobalMap) != NULL)
	{
		ShowStatus(szerrUnique);
	}
	do
	{
		if (!RoomNTileInfoReq (prt, NULL, pctilx, pctily, fSize))
/**/		goto ABORT;

		/* Make sure room name & composite tileset name match. */

		if (IsComposite(prt))
		{
			MEM_free(prt->ptsComposite->Filespec);

			if ((prt->ptsComposite->Filespec = dupstr(prt->Name)) == NULL)
/**/			goto ABORT;
		}
		if (prt->Name != NULL && FindDuplicateRoomName(prt, GlobalMap) != NULL)
		{
			ShowStatus(szerrUnique);
		}
	}
	while (prt->Name != NULL && FindDuplicateRoomName(prt, GlobalMap) != NULL);

	RestoreColorsNPointer (MainWindow);
	return TRUE;
ABORT:
	RestoreColorsNPointer (MainWindow);
	return FALSE;
} /* GetRoomInfoReq */


/*********************************************************************
 *
 * GetTilesetInfoReq
 *
 * SYNOPSIS
 *		BOOL GetTilesetInfoReq(TileSetType *ptst, BOOL fSize)
 *
 * PURPOSE
 *		Get tileset info. If tileset is a composite tileset,
 *		set corresponding room name, and make sure the room
 *		name doesn't clash.
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
 *		12/14/93 (dcc) - Add call to LookupSomeTSTInfoFromUserType().
 *		10/11/94 (dcc) - Call dupstr() instead of strdup().
 *
 * SEE ALSO
 *
*/
BOOL GetTilesetInfoReq(TileSetType *ptst, BOOL fSize)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetTilesetInfoReq";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetColorsNPointer (MainWindow);

	if (ptst->SourceRoom != NULL && IsComposite(ptst->SourceRoom))
	{
		RoomType *prt = ptst->SourceRoom;

		if (prt->Name != NULL && prt->Name[0] != 0)
		{
			MEM_free(ptst->Filespec);

			if ((ptst->Filespec = dupstr(prt->Name)) == NULL)
/**/			goto ABORT;
		}
		else
		{
			/* Empty room name */

			if ((ptst->Filespec = MEM_calloc(1, sizeof(char))) == NULL)
/**/			goto ABORT;
		}

		if ((prt->Name == NULL) || (prt->Name[0] == 0) ||
		(prt->Name != NULL && FindDuplicateRoomName(prt, GlobalMap) != NULL))
		{
			ShowStatus(szerrUnique);
		}
	}

	do
	{
		if (!RoomNTileInfoReq (NULL, ptst, NULL, NULL, fSize))
/**/		goto ABORT;

		LookupSomeTSTInfoFromUserType(ptst);

		/* Make sure room name & composite tileset name match. */

		if (ptst->SourceRoom)
		{
			if (IsComposite(ptst->SourceRoom))
			{
				MEM_free(ptst->SourceRoom->Name);

				if ((ptst->SourceRoom->Name = dupstr(ptst->Filespec)) == NULL)
/**/				goto ABORT;
			}
			if (ptst->SourceRoom->Name != NULL && FindDuplicateRoomName(ptst->SourceRoom, GlobalMap) != NULL)
			{
				ShowStatus(szerrUnique);
			}
		}
	}
	while (ptst->SourceRoom != NULL && IsComposite(ptst->SourceRoom) &&
			((ptst->SourceRoom->Name == NULL) || (ptst->SourceRoom->Name[0] == 0) ||
			(ptst->SourceRoom->Name != NULL && FindDuplicateRoomName(ptst->SourceRoom, GlobalMap) != NULL)));

	RestoreColorsNPointer (MainWindow);
	return TRUE;
ABORT:
	RestoreColorsNPointer (MainWindow);
	return FALSE;
} /* GetTilesetInfoReq */

