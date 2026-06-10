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
 * STUBS.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Stubs.
 *
 * HISTORY
 *
*/

#include <echidna/listfunc.h>
#include <echidna/grafx.h>
#include <echidna/fonts.h>

#include "tuglbl.h"
#include "stubs.h"							/* Verify function prototypes. */

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

int		DontWait			= FALSE;

int		LastTBar;
int		ShowCursor = FALSE;
int		ShowTUser = FALSE;
int		ShowUser = FALSE;
int		fShowDefaultCursor = FALSE;

BlockCopyType		*GlobalBlockCopy;
RoomWindowType	*GlobalRoomWindow	= NULL;
TileSpaceType		*GlobalTileSpace		= NULL;

extern Font MainFont;

#define DS(msg)	\
	SetPenColor (255);						\
	DrawRect (10, 150, 25*8, 8);			\
 	DrawString (&MainFont, 10, 150, msg);	\

MapType				*GlobalMap		= NULL;
TileSetType		*GlobalTileSet		= NULL;

extern short MouseButtons(HitInfo *hi);
extern short MouseDown(HitInfo *hi);

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * SetupEditsPane
 *
 * SYNOPSIS
 *		void SetupEditsPane (Pane *p)
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
void SetupEditPane (Pane *p)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetupEditPane";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	p->P1.LeftFirstDown = MouseButtons;
	p->P1.LeftDown = MouseDown;
	p->P1.LeftUp = MouseButtons;

} /* SetupEditPane */

