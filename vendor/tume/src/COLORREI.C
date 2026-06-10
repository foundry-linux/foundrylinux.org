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
 * COLORREI.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 07/30/93
 *   MODIFIED : 02/09/95
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Initialize color requester.
 *
 * HISTORY
 *		07/30/93 Friday (dcc) - created.
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/eerrors.h>
#include <echidna/eio.h>
#include <echidna/menus2.h>
#include <echidna/readini.h>

#include "colorreq.h"					/* Verify prototypes. */

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

extern PopupItem *ColorPopup;				/* in COLORREQ.C */

/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * OpenColorReq
 *
 * PURPOSE
 *		Open the *.INI, read the [COLOR MENU] section, process the lines.
 *
 * INPUT
 *		szIni				: .INI file to read
 *
 * ASSUMES
 *		Keyboard binding have already been read by OpenEUI().
 *
 *		Function assumes that it can find <szIni> in the path.
 *		This is verified to be TRUE if you've already called ProcessINI().
 *
 * RETURN VALUE
 *		FALSE if failed or OOM.
 *
 * HISTORY
 *		07/30/93 Friday (dcc) - read the .INI file ourselves (allows
 *					processing after OpenEUI())
 *		08/12/93 Thursday (dcc) - search path for <szINI>.
 *
*/
BOOL OpenColorReq (char *szIni)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenColorReq";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		IniList *pIniList;

		SectionTracker	 secx;
		SectionTracker	*sec = &secx;

		BOOL f;
		char szPath[EIO_MAXPATH];

		EIO_FindFile (szIni, NULL, szPath);
		if ((pIniList = ReadINI(szPath)) == NULL)
			return FALSE;

		sec = FindSection (sec, pIniList, "[COLOR MENU]");
		if (!sec) {
			SetGlobalErr (ERR_GENERIC);
			GEcatf ("\n[COLOR MENU] section not found in .INI file");
			return FALSE;
		}

		ColorPopup->MenuItemList = &ColorPopup->MenuItemListX;
		InitList (ColorPopup->MenuItemList);

		SetShowKeyCmds(FALSE);
		f = ParsePopup (sec, ColorPopup, TRUE);

		FreeINI(pIniList);

		return f;
	}
} /* OpenColorReq */

