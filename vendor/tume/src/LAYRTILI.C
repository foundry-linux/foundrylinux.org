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
 * LAYRTILI.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 06/07/93
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
 *		Keep track of which tiles are being placed into which layers.
 *
 *		This routine reads the LayerTypes= section named in the [Type Groups]
 *		section of the tUME.INI file.
 *
 *		It then generates a linked list of all layer. If only the 99th layer
 *		is defined, it will still generated a linked list with 99 nodes.
 *
 *		For each layer node, there is an array of tile-types that is
 *		associated with that node.
 *
 *		For each layer node, there is also a pointer to the most recently
 *		display source room of that type.
 *
 *		See also LAYRTILE.C.
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include <echidna/listfunc.h>
#include <echidna/readini.h>
#include <echidna/windows.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include "layrtili.h"						/* Verify function prototypes. */
#include "layrtile.h"
#include "parseini.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * AddLayerType
 *
 * SYNOPSIS
 *		short AddLayerType(UWORD layer, char *pszTypes)
 *
 * PURPOSE
 *		Add node layer <layer>, with tile types in comma-separated
 *		string <pszTypes>.
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
short AddLayerType(UWORD layer, char *pszTypes)
{
	LYT *plyt;
	short cntType;
	short i;
	char *pszComma = pszTypes;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddLayerType";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((cntType = CountCommaSeparated(pszTypes)) == 0)
		return FALSE;							/* no comma-separated fields */

	if ( (plyt = PLYTFromLayer(layer)) == NULL)
		return FALSE;							/* OOM */

	if (plyt->TS_Type)						/* Something already allocated? */
	{
		MEM_free(plyt->TS_Type);
		plyt->prmLast = NULL;
		plyt->TS_Type = NULL;
		plyt->cntType = 0;
	}

	plyt->cntType = cntType;
	if ( (plyt->TS_Type = MEM_calloc(cntType, sizeof(UWORD))) == NULL)
		return FALSE;							/* OOM */

	for (i = 0; i < cntType; i++)
	{
		plyt->TS_Type[i] = atoi(pszComma);
		pszComma = strstr(pszComma, ",")+1;
	}
	return TRUE;

} /* AddLayerType */


/*********************************************************************
 *
 * ReadLayerType
 *
 * SYNOPSIS
 *		short ReadLayerType(SectionTracker *psec)
 *
 * PURPOSE
 *		Read the layer type section lines from tUME.INI.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns TRUE if successful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short ReadLayerType(SectionTracker *psec)
{
	ConfigLine *pcfl;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReadLayerType";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	while (((pcfl = GetNextINILine(psec))) != NULL)
	{
		char *psz1, *psz2;
		UWORD layer;

		if ( (psz1 = strstr(szConfigLine(pcfl), "Layer")) == NULL)
		{
			printf("layer types: keyword 'Layer' not found.\n");
			return FALSE;
		}
		if ( (psz2 = strstr(szConfigLine(pcfl), "=")) == NULL)
		{
			printf("layer types: '=' not found.\n");
			return FALSE;
		}
		if (psz1 > psz2)
		{
			printf("layer types: bad syntax.\n");
			return FALSE;
		}
		layer = atoi(psz1+5);
		AddLayerType(layer, psz2+1);
	}
	return TRUE;
} /* ReadLayerType */


/*********************************************************************
 *
 * OpenLayerType
 *
 * SYNOPSIS
 *		short OpenLayerType(void)
 *
 * PURPOSE
 *		Initialize layer type stuff.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if OOM.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short OpenLayerType(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenLayerType";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ( (plstLYT = CreateList(NULL)) == NULL)
		return FALSE;							/* unable to allocate list header */

	return TRUE;
} /* OpenLayerType */

