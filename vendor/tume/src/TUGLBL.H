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
 * TUGLBL.H
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 08/09/90 
 *   MODIFIED : 10/19/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w-----
 *	 E C H I D N A
 *	--------------
 *
 * DESCRIPTION
 *		Global definitions for tUME.
 *
 * HISTORY
 *	08/09/90 Thursday (RGM) - Created out of "globals.h".
 *	                          (CREATED : 08/29/89)
 *
 *
*/
#ifndef TUGLBL_H
#define TUGLBL_H

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/etypes.h>
#if __AMIGAOS__
#include <graphics/text.h>
#include <graphics/gfx.h>
#include <intuition/preferences.h>
#endif/*__AMIGAOS__*/

#if LATTICE
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#endif

//!!#include <stdio.h>
//!!#include <stdlib.h>
//!!#include <stddef.h>

#include <echidna/listfunc.h>
#if __AMIGAOS__
#include <echidna/easyinterface.h>
#include <echidna/picture.h>
#include "pointer.h"
#elif __MSDOS__
#include <echidna/eui.h>
#include "picture.h"
#endif/*__AMIGAOS__/__MSDOS__*/

#include "tudef.h"
#include "tufunc.h"

/**************************** C O N S T A N T S ***************************/

#define TITLE_HEIGHT	11

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

#if __AMIGAOS__
extern	struct	ArpBase		*ArpBase;
#endif

/*-------------------- GLOBAL SCREEN VARIABLES ----------------------*/
#if __AMIGAOS__
extern	struct	Screen	*MainScreen;
#endif
extern	struct	Window	*MainWindow;
#if __AMIGAOS__
extern	struct	Menu	*MenuStrip;

extern	struct	ViewPort	*MainVp;
#endif
extern	struct	RastPort	*MainRp;
#if __AMIGAOS__
extern	struct	TextAttr	NormFont;
#endif

extern 	struct	Window	*ActiveWindow;

/*-------------------------- tUME GLOBALS -------------------------------*/
extern char *NoCanDo;

extern	int	NormalScreenWidth;
extern	int	NormalScreenHeight;
extern	int	ScreenWidth;
extern	int	ScreenHeight;

#if __AMIGAOS__
extern	struct Preferences UserPrefs;
#endif

extern	TileSetType			*GlobalTileSet;
extern	RoomType			*GlobalRoom;
extern	RoomType			*UndoRoom;
extern	RoomWindowType		*GlobalRoomWindow;
extern	RoomWindowType		*TrackingRoomWindow;

extern	TileSpaceType		*GlobalTileSpace;
extern	MapType				*GlobalMap;
extern	RoomWindowListType	*GlobalRoomWindowList;

extern	ColorInfo			DefaultColorInfo;
extern	ColorInfo			*GlobalColors;

extern	BlockCopyType		*GlobalBlockCopy;

extern 	WORD				GlobalRoomID;

#if __AMIGAOS__
extern	RGM_MPointerType	BlankMPointer;
extern	RGM_MPointerType	YieldMPointer;
extern	RGM_MPointerType	FillMPointer;
extern	RGM_MPointerType	GrabMPointer;
extern	RGM_MPointerType	C1MPointer;
extern	RGM_MPointerType	C2MPointer;
extern	RGM_MPointerType	C3MPointer;
extern	RGM_MPointerType	C4MPointer;

extern	RGM_MPointerType	*GlobalPointer;
#elif __MSDOS__
extern	short					GlobalPointer;
#endif/*__AMIGAOS__/__MSDOS__*/

extern	WORD		LeftField;
extern	WORD		RightField;
extern	WORD		TopField;
extern	WORD		BottomField;

extern	int		TopAdd;
extern	int		LastTBar;
extern	int		LastRealTBar;
extern	int		ShowCursor;
extern	int		ShowTUser;
extern	int		ShowUser;
extern	int		fShowDefaultCursor;

extern	struct	TextFont	*TopazFont;
extern	struct	TextFont	*EchidnaFont;

extern 	BOOL		Colorz;
extern	BOOL		OneLayerCopy;
extern	SHORT		TextTop;

extern WORD 	CurrentX;
extern WORD 	CurrentY;

extern	short	GlobalBackground;

extern	EasyFileReq		**EasyFR;

extern	short	TU_DrawMode;

/**************************** G L O B A L S . H ***************************/

extern ByteMap		*DisplayBM;
extern BigByteMap	*BigBackBM;

/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

#endif /* TUGLBL_H */

