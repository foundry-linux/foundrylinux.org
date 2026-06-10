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
 * OBJROOM.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 08/24/94
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		Create a dummy source room.
 *
 * HISTORY
 *		08/24/94 (dcc) - created.
 *
*/

#include <echidna/platform.h>
#include "switches.h"
#include "switch1.h"

#if PixelLayers
#include "tuglbl.h"
#include "objroom.h"							/* Verify function prototype. */

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

RoomWindowType	RW1x1;
RoomStuffType	RS1x1;
RoomType			RM1x1;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * InitDummySourceRoom
 *
 * PURPOSE
 *		Set up a dummy room so that Make/Edit Tile works properly.
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
 *		08/24/94 (dcc) - created.
 *
*/
BOOL InitDummySourceRoom(void)
{
	LayerType *play;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitDummySourceRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	RW1x1.Window			= NULL;
	RW1x1.CurrentRoom		= &RS1x1;

	RS1x1.Room				= &RM1x1;
	RS1x1.Flags				= 0;
	RS1x1.dxSrc				= 0;
	RS1x1.dySrc				= 0;
	RS1x1.ScreenPlotRows	= 0;

	InitList(&RM1x1.Layers);
	play = AddClearedLayerToLayers (&RM1x1.Layers, 1, 1, FALSE);

	play->cxTile = 1;
	play->cyTile = 1;

	RM1x1.Flags				= 0;
	RM1x1.SaveFlags		= 0;
	RM1x1.LayerCount		= 1;
	RM1x1.FloorNumber		= 1;
	RM1x1.FloorLayer		= play;
	RM1x1.R_ColorInfo		= &DefaultColorInfo;
	RM1x1.Name				= 0;

	return TRUE;

} /* InitDummySourceRoom */
#endif // PixelLayers

