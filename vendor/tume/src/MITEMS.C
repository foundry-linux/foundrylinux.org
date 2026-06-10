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
 * MOUSEITEMS.C
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 10/05/89 
 *   MODIFIED : 04/03/95
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *
 *
 * HISTORY
 *		10/05/89 (RGM) - Created.
 *		04/13/91 (RGM) - Bug fix to allow an 'Undo' of a 'cut'.
 *		04/05/93 (dcc) - use rectplot.h header.
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
*/

#include <echidna/platform.h>
#include "switches.h"
#include "switch1.h"				// only for PixelLayers

#include <stdio.h>
#include <string.h>

#include "rectplot.h"
#include "events.h"
#include "tuglbl.h"
#include "mitems.h"				/* Verify function prototypes. */
#include "mouse.h"
#include "tilebits.h"
#include "tumedraw.h"
#include "hilitile.h"
#include "layrtili.h"
#include "layrtile.h"
#include "histogrm.h"
#include "rm_tsinf.h"

#if __MSDOS__
#include <echidna/grafx.h>

#endif/*__MSDOS__*/
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray
#include "szerror.h"
#include "assert.h"

#include <echidna/eio.h>

extern WORD wSrcSizeToDstSize(WORD w, unsigned int wSrcSkip, unsigned int wDstDup);

/**************************** C O N S T A N T S ***************************/

#define	OFF		(0)
#define	COPY		(1)
#define	CUT		(3)

#define	ERASE	(0)
#define	DRAW		(1)

#define CANSHOW(it) (! ((it == SHOW_TITLE) || (it == SHOW_VERSION)))

#if __AMIGAOS__
#define	SNX		((03*8) - 6)
#define XX		((19*8) - 6)
#define YX		((25*8) - 6)
#elif __MSDOS__
#define	SNX		((03*8) - 7)
#define XX		((19*8) - 7)
#define YX		((25*8) - 7)
#endif/*__AMIGAOS__/__MSDOS__*/

/******************************** T Y P E S *******************************/

typedef struct
{
	NodeType	 Node;
	int		 TileID;				/* tileset ID of special tile */
	LayerType	*play;				/* layer of special tile */
	int		 dtilx;				/* x-offset within layer */
	int		 dtily;				/* y-offset within layer */
} SpecialTileset;

/****************************** E X T E R N S *****************************/

#if PIXELSELECT
extern int ctilxOverhang, ctilyOverhang;
#endif  // PIXELSELECT

/****************************** G L O B A L S *****************************/

WORD AnchorX, AnchorY;
WORD CurrentX, CurrentY;
WORD ScrollDX, ScrollDY;
WORD DrawMode, SelectMode;

RoomWindowType	*TrackingRoomWindow;

char	X_Pos[20];
char	Y_Pos[20];

WORD OldColumn	= -1;
WORD OldRow		= -1;

#if PIXELSELECT
static int dxModTile, dyModTile;

static int dxModTileAnchor, dyModTileAnchor;
static int dxModTileCurrent, dyModTileCurrent;
#endif // PIXELSELECT


HitInfo hitGlobal;


/******************************* L O C A L S ******************************/

static	char TitleStuff [60];
static	char HoldBuffer[60];
static	char TempName[60];
static	char TempExt[60];

static BOOL fForceBrushRedraw = FALSE;

#if __AMIGAOS__
static	void *Old_Pointer	= NULL;

static void Tracking (void);
static void Drawing (void);
static void StartSelect (void);
static void Selecting (void);
static void FinishSelect (void);
#elif __MSDOS__
static short Hovering(HitInfo *hi);
static short Tracking(HitInfo *hi);
static short Drawing(HitInfo *hi);
static void StartSelect (void);
static short Selecting(HitInfo *hi);
static void FinishSelect (void);
#endif/*__AMIGAOS__/__MSDOS__*/

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


#if __MSDOS__
/* ShowCoordinates - write new coordinates to title bar. */

static void ShowCoordinates(void)
{
	if (PushWindowClipValues (MainWindow))
	{
		BeforeGraphics ();
		SetPenColor (MainWindow->White);
		DrawRect (XX, 0, 40, TITLE_HEIGHT - 1);
		DrawRect (YX, 0, 40, TITLE_HEIGHT - 1);
		SetPenColor (MainWindow->Black);
		DrawString (MainWindow->Font, XX, 1, X_Pos);
		DrawString (MainWindow->Font, YX, 1, Y_Pos);
		AfterGraphics ();
		PopClipValues ();
	}
}
#endif/*__MSDOS__*/


/*********************************************************************
 *
 * FindBiggestLayer
 *
 * PURPOSE
 *		Find the layer with the largest tiles in <pLayers>.
 *
 *		If the largest layer is not sized, then return NULL.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns layer with largest tile in <pLayers>.
 *
 * HISTORY
 *		08/28/94 (dcc) - Created.
 *		03/19/95 (dcc) - KLUDGE for PixelLayers: they NEVER count as the
 *					  largest layer.
 *
*/
LayerType *FindBiggestLayer(ListType *pLayers)
{

		LayerType		*play;
		LayerType		*playBig = NULL;
		UWORD		 cxTileBig = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindBiggestLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	play = Head(pLayers);
	while (!IsEOList(play))
	{
#if	  PixelLayers
		if (play->cxTile > cxTileBig &&	// KLUDGE: pixel layers don't count!
		    !(play->cxTile == 1 && play->cyTile == 1 && IsSparse(play)))
		{
			cxTileBig = play->cxTile;
			playBig = play;
		}
#else  //!PixelLayers
		if (play->cxTile > cxTileBig)
		{
			cxTileBig = play->cxTile;
			playBig = play;
		}
#endif //!PixelLayers
		play = Next(play);
	}
	if (cxTileBig)
		return playBig;
	else
		return NULL;
} /* FindBiggestLayer */


/*********************************************************************
 *
 * FindNonEmptyLayer
 *
 * PURPOSE
 *		Returns the layer in layers <pLayers> that is has tiles
 *		stamped into it. If a layer doesn't have tiles stamped into
 *		it, then it's <cxTile> for that layer is still 0.
 *
 *		This routine searches UP from the floor layer <playFloor>.
 *		If no information is found, then it searches UP from the first
 *		layer in <pLayers>.
 *
 *		This routine may be used to search rooms (pass it &prm->Layers
 *		and prm->FloorLayer), or GlobalBlockCopy (pass it &pbc->Layers
 *		and pbc->BC_FloorLayer).
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NULL if no layer with information is available.
 *
 * HISTORY
 *		12/08/93 (dcc) - Created.
 *		07/18/94 (dcc) - Add check for NULL players, playFloor
 *		03/19/95 (dcc) - KLUDGE for PixelLayers: they NEVER count as a
 *					  non-empty layer.
 *
*/
LayerType *FindNonEmptyLayer(ListType *pLayers, LayerType *playFloor)
//GetRoomLayerWithInfo(RoomType *prm)
{

	LayerType *play;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindNonEmptyLayer"
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((play = playFloor) != NULL)
	{
		while (!IsEOList(play))
		{
#if	  PixelLayers
			if (play->cxTile &&		// KLUDGE: pixel layers don't count!
			    !(play->cxTile == 1 && play->cyTile == 1 && IsSparse(play)))
			{
				return play;
			}
#else  //!PixelLayers
			if (play->cxTile)
			{
				return play;
			}
#endif //!PixelLayers
			play = Next(play);
		}
	}
	if (pLayers != NULL)
	{
		play = Head(pLayers);

		while (!IsEOList(play))
		{
#if	  PixelLayers
			if (play->cxTile &&		// KLUDGE: pixel layers don't count!
			    !(play->cxTile == 1 && play->cyTile == 1 && IsSparse(play)))
			{
				return play;
			}
#else  //!PixelLayers
			if (play->cxTile)
			{
				return play;
			}
#endif //!PixelLayers
			play = Next(play);
		}
	}
	return NULL;

} /* FindNonEmptyLayer */


/**************************************************************************
 *
 * OKToScroll
 *
 * SYNOPSIS
 *		static BOOL OKToScroll (
 *			RoomWindowType	*roomwindow,
 *			WORD			*scroll_dx,
 *			WORD			*scroll_dy
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
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
 *		01/28/92 Tuesday - Add check for *scroll_dx < 0 so user can scroll
 *				back into range. (DCC)
 *
 * SEE ALSO
 *
*/
static BOOL OKToScroll (
	RoomWindowType	*roomwindow,
	WORD			*scroll_dx,
	WORD			*scroll_dy
)
{
	BOOL		lock;
	UWORD	cxTileScroll;
	UWORD	cyTileScroll;
	short	wRoomWidth;
	short	wRoomHeight;
	RoomType	*prm;
	LayerType *play;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OKToScroll";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	prm = roomwindow->CurrentRoom->Room;

	if (prm == NULL)
	{
		lock = TRUE;
	}
	else if ((play = FindBiggestLayer(&prm->Layers)) != NULL)
//	else if ((play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer)) != NULL)
	{
		wRoomWidth   = play->ctilx;
		wRoomHeight  = play->ctily;
		cxTileScroll = play->cxTile;
		cyTileScroll = play->cyTile;
		lock = FALSE;
	}
	else
	{
		lock = TRUE;
	}

	if (lock) {
		*scroll_dx	= 0;
		*scroll_dy	= 0;
	}
	else {
		struct	Window	*window;
		WORD			extra;
		LONG			upperleft_x;
		LONG			upperleft_y;
		WORD			plot_width;
		WORD			plot_height;
		unsigned		initx;
		unsigned		inity;

		WORD			wWinWidth;
		WORD			wWinHeight;

		Assert(cxTileScroll != 0);
		initx	= (unsigned) (roomwindow->CurrentRoom->dxSrc / cxTileScroll);
		Assert(cyTileScroll != 0);
		inity	= (unsigned) (roomwindow->CurrentRoom->dySrc / cyTileScroll);

		window	= roomwindow->Window;
		extra	= ((roomwindow->CurrentRoom->Flags) & DISPLAY_SEPERATED);

		plot_width	= wSrcSizeToDstSize(cxTileScroll,
									wSrcSkipX, wDstDupX) + extra;
		plot_height	= wSrcSizeToDstSize(cyTileScroll,
									wSrcSkipY, wDstDupY) + extra;

		wWinWidth = window->Width;
		wWinHeight = window->Height - TopAdd;

		if (wShowXTiles)
			wWinWidth = min(wWinWidth, wShowXTiles * plot_width);

		if (wShowYTiles)
			wWinHeight = min(wWinHeight, wShowYTiles * plot_height);

		upperleft_x	= (LONG) initx - *scroll_dx;
		upperleft_y	= (LONG) inity - *scroll_dy;

		Assert(plot_width != 0);
		if (upperleft_x < 0L) {

			/****************************************************/
			/* we've hit the left edge, give them the left edge */

			*scroll_dx	= initx;
		}
		else if (*scroll_dx < 0 && ((LONG) wWinWidth -
					(((LONG) wRoomWidth - upperleft_x) * plot_width)) >= plot_width) {
			*scroll_dx	=
				- max(0, wRoomWidth - (int) initx - (wWinWidth / plot_width));
		}
		else {
		}

		Assert(plot_height != 0);
		if (upperleft_y < 0L) {

			/****************************************************/
			/* we've hit the top edge, give them the top edge */

			*scroll_dy	= inity;
		}
		else if (*scroll_dy < 0 && ((LONG) wWinHeight -
					(((LONG) wRoomHeight - upperleft_y) * plot_height)) >= plot_height) {

			*scroll_dy	=
				- max(0, wRoomHeight - (int) inity - (wWinHeight / plot_height));
		}
		else {
		}
	}

	return ((BOOL) ((*scroll_dx) || (*scroll_dy)));

} /* OKToScroll */


/**************************************************************************
 *
 * FollowCursor
 *
 * PURPOSE
 *		Where the cursor goes in the active room/window, so follows the
 *		GlobalBlockCopy image. . .
 *
 * 	So says (RGM). Well, I think the routine does this (DCC):
 *
 * 	Convert the current mouse position on screen to tile <column> and
 * 	<row> information.
 *
 *	NOTES
 *		dxModTile will be set = -1 if it is on the line between two tiles.
 *		Same with dyModTile.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * ASSUMES
 *		GlobalRoomWindow->CurrentRoom->Room != NULL;
 *
 * RETURN VALUE
 *		Returns TRUE if cursor has moved onto a new tile (from last
 *		call to this routine).
 *
 * HISTORY
 *		10/06/89 (RGM) - Created.
 *		03/11/92 (dcc) - support added for locked composite rooms
#if PIXELSELECT
 *		12/22/93 (dcc) - save offset within tile in dxModTile & dyModTile
#endif // PIXELSELECT
 *		08/28/94 (dcc) - add support for PixelLayers
 *		11/11/94 (dcc) - Change ULONG casts to LONG.
 *
 * SEE ALSO
 *
*/
static BOOL FollowCursor (
	WORD		*column,
	WORD		*row,
	WORD		*scroll_dx,
	WORD		*scroll_dy,
	RoomType	*altroom,
	BOOL		scrollok
)
{
	struct Window	*window;
	RoomStuffType	*roomstuff;
	RoomType		*room;
	LayerType		*play;
	WORD			cursor_x, cursor_y;
	unsigned		initx, inity;
	WORD			roomwidth, roomheight;
	WORD			plot_width, plot_height;
	WORD			extra;

	WORD			out_x	= 0;
	WORD			out_y	= 0;
	BOOL			result	= FALSE;

	WORD			wXPixMax;
	WORD			wYPixMax;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FollowCursor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	
	if (! GlobalRoomWindow)
	{
		goto ABORT;
	}

	window		= GlobalRoomWindow->Window;
	roomstuff		= GlobalRoomWindow->CurrentRoom;
	room			= roomstuff->Room;

#if PixelLayers
	if ((play = FindBiggestLayer(&room->Layers)) == NULL)
#else // !PixelLayers
	if ((play = FindNonEmptyLayer(&room->Layers, room->FloorLayer)) == NULL)
#endif // !PixelLayers
	{
		play		= room->FloorLayer;
	}
#if PixelLayers
	roomwidth		= room->FloorLayer->ctilx;
	roomheight	= room->FloorLayer->ctily;
#else // !PixelLayers
	roomwidth		= play->ctilx;
	roomheight	= play->ctily;
#endif // !PixelLayers

	if (!(play->cxTile))
	{
#if PixelLayers
		if ((altroom) &&
			((play = FindBiggestLayer(&altroom->Layers)) != NULL))
#else // !PixelLayers
		if ((altroom) &&
			((play = FindNonEmptyLayer(&altroom->Layers, altroom->FloorLayer)) != NULL))
#endif // !PixelLayers
		{
			room = altroom;
		}
		else
		{
/**/		goto ABORT;
		}
	}

	if ((room) && (window))
	{
		extra = ((roomstuff->Flags) & DISPLAY_SEPERATED);

		plot_width	= wSrcSizeToDstSize(play->cxTile, wSrcSkipX, wDstDupX) + extra;
		plot_height	= wSrcSizeToDstSize(play->cyTile, wSrcSkipY, wDstDupY) + extra;

		wXPixMax = window->Width;
		wYPixMax = window->Height;

		if (wShowXTiles > 0)
			wXPixMax = min(wXPixMax, wShowXTiles * plot_width);
		if (wShowYTiles > 0)
			wYPixMax = min(wYPixMax, wShowYTiles * plot_height);
#if __AMIGAOS__
		cursor_x	= window->MouseX + extra;
		cursor_y	= window->MouseY + extra;
#elif __MSDOS__
		cursor_x	= MouseX;
		cursor_y	= MouseY;
#endif/*__AMIGAOS__/__MSDOS__*/

		Assert(play->cxTile != 0);
		initx = (unsigned) (roomstuff->dxSrc / play->cxTile);
		Assert(play->cyTile != 0);
		inity = (unsigned) (roomstuff->dySrc / play->cyTile);

		/*********************************/
		/* find out if we want to scroll */

		*scroll_dx = *scroll_dy = 0;

#if __AMIGAOS__
		if (cursor_x < LeftField)
#elif __MSDOS__
		if (cursor_x <= LeftField)
#endif/*__AMIGAOS__/__MSDOS__*/
		{
			*scroll_dx	= 1;
			out_x		= -1;
		}
#if __AMIGAOS__
		else if (cursor_x > (wXPixMax) - RightField)
#elif __MSDOS__
		else if (cursor_x+1 >= (wXPixMax) - RightField)
#endif/*__AMIGAOS__/__MSDOS__*/
		{
			*scroll_dx	= -1;
			out_x		= 1;
		}

#if __AMIGAOS__
		if (cursor_y < TopField + TopAdd)
#elif __MSDOS__
		if (cursor_y <= TopField)
#endif/*__AMIGAOS__/__MSDOS__*/
		{
			*scroll_dy	= 1;
			out_y		= -1;
		}
#if __AMIGAOS__
		else if (cursor_y > (wYPixMax) - BottomField)
#elif __MSDOS__
		else if (cursor_y+1 >= (wYPixMax) - BottomField - TopAdd)
#endif/*__AMIGAOS__/__MSDOS__*/
		{
			*scroll_dy	= -1;
			out_y		= 1;
		}

		/***************************************/
		/* find out if we're allowed to scroll */


		if (! scrollok)
		{
			*scroll_dx	= 0;
			*scroll_dy	= 0;
		}
		else
		{
			OKToScroll (GlobalRoomWindow, scroll_dx, scroll_dy);
		}

		/*************************************/
		/* compute row and column for cursor */

		Assert(plot_width != 0);
		if (! out_x)
		{
#if PixelLayers
			if (play->cxTile && room->FloorLayer->cxTile)
				*column	= (int) (((LONG) cursor_x * play->cxTile / room->FloorLayer->cxTile) / plot_width) +
							(int) ((LONG) initx * play->cxTile / room->FloorLayer->cxTile);
			else
				*column	= (cursor_x / plot_width) + initx;
#else // !PixelLayers
			*column	= (cursor_x / plot_width) + initx;
#endif // !PixelLayers
		}
		else if (out_x < 0)
		{
#if PixelLayers
			if (play->cxTile && room->FloorLayer->cxTile)
				*column	= (int) ((LONG) initx * play->cxTile / room->FloorLayer->cxTile) - *scroll_dx;
			else
				*column	= initx - *scroll_dx;
#else // !PixelLayers
			*column	= initx - *scroll_dx;
#endif // !PixelLayers
		}
		else if (out_x > 0)
		{
#if PixelLayers
			if (play->cxTile && room->FloorLayer->cxTile)
				*column	= (int) (((LONG) wXPixMax * play->cxTile / room->FloorLayer->cxTile) / plot_width) +
							(int) ((LONG) initx * play->cxTile / room->FloorLayer->cxTile) - 1 - *scroll_dx;
			else
				*column	= (initx + (wXPixMax / plot_width) - 1) - *scroll_dx;
#else // !PixelLayers
			*column	= (initx + (wXPixMax / plot_width) - 1) - *scroll_dx;
#endif // !PixelLayers
			if (wXPixMax % plot_width)
			{
				(*column) += 1;
			}
		}
		Assert(plot_height != 0);
		if (! out_y)
		{
#if __AMIGAOS__
			*row		= ((cursor_y - TopAdd) / plot_height) + inity;
#elif __MSDOS__
#if PixelLayers
			if (play->cyTile && room->FloorLayer->cyTile)
				*row		= (int) (((LONG) cursor_y * play->cyTile / room->FloorLayer->cyTile) / plot_height) +
							(int) ((LONG) inity * play->cyTile / room->FloorLayer->cyTile);
			else
				*row		= (cursor_y / plot_height) + inity;
#else // !PixelLayers
			*row		= (cursor_y / plot_height) + inity;
#endif // !PixelLayers
#endif/*__AMIGAOS__/__MSDOS__*/
		}
		else if (out_y < 0)
		{
#if PixelLayers
			if (play->cyTile && room->FloorLayer->cyTile)
				*row		= (int) ((LONG) inity * play->cyTile / room->FloorLayer->cyTile) - *scroll_dy;
			else
				*row		= inity - *scroll_dy;
#else // !PixelLayers
			*row		= inity - *scroll_dy;
#endif // !PixelLayers
		}		
		else if (out_y > 0)
		{
#if PixelLayers
			if (play->cyTile && room->FloorLayer->cyTile)
				*row		= (int) (((LONG) (wYPixMax - TopAdd) * play->cyTile / room->FloorLayer->cyTile) / plot_height) +
							(int) ((LONG) inity * play->cyTile / room->FloorLayer->cyTile) - 1 - *scroll_dy;
			else
				*row		= (inity + ((wYPixMax - TopAdd) / plot_height) - 1) - *scroll_dy;
#else // !PixelLayers
			*row		= (inity + ((wYPixMax - TopAdd) / plot_height) - 1) - *scroll_dy;
#endif // !PixelLayers
			if (wYPixMax % plot_height)
			{
				(*row) += 1;
			}
		}

#if PIXELSELECT
		if (extra)
		{
			if (cursor_x % plot_width)
			{
				Assert(plot_width  -extra != 0);
				dxModTile = (cursor_x % plot_width - 1) * play->cxTile / (plot_width  -extra);
			}
			else
				dxModTile = -1;

			if (cursor_y % plot_height)
			{
				Assert(plot_height -extra != 0);
				dyModTile = (cursor_y % plot_height- 1) * play->cyTile / (plot_height -extra);
			}
			else
				dyModTile = -1;
		}
		else
		{
			dxModTile = (cursor_x % plot_width ) * play->cxTile / plot_height;
			dyModTile = (cursor_y % plot_height) * play->cyTile / plot_height;
		}
#endif // PIXELSELECT

		if (*column < 0)
		{
			*column = initx;
#if PIXELSELECT
			dxModTile = 0;
#endif // PIXELSELECT
		}
		else if (*column >= roomwidth)
		{
			*column = roomwidth - 1;
#if PIXELSELECT
			dxModTile = plot_width-1;
#endif // PIXELSELECT
		}

		if (*row < 0)
		{
			*row = inity;
#if PIXELSELECT
			dyModTile = 0;
#endif // PIXELSELECT
		}
		else if (*row >= roomheight)
		{
			*row = roomheight - 1;
#if PIXELSELECT
			dyModTile = plot_height-1;
#endif // PIXELSELECT
		}

		/********************************************************/
		/* if there is a change from the last time, move things */

		if ( (*column != OldColumn) || (*row != OldRow) ||
			(scrollok && (*scroll_dx || *scroll_dy)) ||
			(fForceBrushRedraw) )
		{

			result = TRUE;
			fForceBrushRedraw = FALSE;
		}

		/*********************************/
		/* save the values for next time */

		OldRow	= *row;
		OldColumn	= *column;

	}
	return (result);

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:
	return (FALSE);

} /* FollowCursor */


#if PIXELSELECT
/*********************************************************************
 *
 * FollowPixelCursor
 *
 * PURPOSE
 * 	Convert the current mouse position on screen to tile <column> and
 * 	<row> information.
 *
 *	NOTES
 *		dxModTile will be set = -1 if it is on the line between two tiles.
 *		Same with dyModTile.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		GlobalRoomWindow->CurrentRoom->Room != NULL;
 *
 * RETURN VALUE
 *		Returns TRUE if user has moved a single display pixel.
 *
 * HISTORY
 *		01/13/94 Thursday (dcc) - created.
 *
*/
static BOOL FollowPixelCursor (
	WORD		*column,
	WORD		*row,
	WORD		*scroll_dx,
	WORD		*scroll_dy,
	RoomType	*altroom,
	BOOL		scrollok
)
{
	BOOL		fResult;
	int		dxModTileSave, dyModTileSave;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FollowPixelCursor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	dxModTileSave = dxModTile;
	dyModTileSave = dyModTile;

	fResult = FollowCursor(column, row, scroll_dx, scroll_dy, altroom, scrollok);

	if ((dxModTile != dxModTileSave) || (dyModTile != dyModTileSave))
	{
		fResult = TRUE;
		fForceBrushRedraw = FALSE;
	}
	return fResult;

} /* FollowPixelCursor */
#endif // PIXELSELECT


/*********************************************************************
 *
 * ForceBrushRedraw
 *
 * SYNOPSIS
 *		void ForceBrushRedraw(void)
 *
 * PURPOSE
 *		Set fForceBrushRedraw to TRUE,
 *		so that FollowCursor() will return TRUE.
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
void ForceBrushRedraw(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ForceBrushRedraw";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	fForceBrushRedraw = TRUE;

} /* ForceBrushRedraw */


/*********************************************************************
 *
 * FMayDrawHere
 *
 * SYNOPSIS
 *		static BOOL FMayDrawHere(BlockCopyType *pblk, RoomType *prm)
 *
 * PURPOSE
 *		Returns TRUE if you may draw the brush <pblk> on the floor layer
 *		and up of <prm>.
 *
 *		You should make sure FSelectedPblk(pblk) is TRUE and <prm> != NULL
 *		before you call this function.
 *
 *		You also need to check that <prm> is not a source room and
 *		that it is not locked.
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
static BOOL FMayDrawHere(BlockCopyType *pblk, RoomType *prm)
{
	LayerType *playSrc = Head(&pblk->Layers);
	LayerType *playDst = prm->FloorLayer;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FMayDrawHere";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	while (!IsEOList(playSrc))
	{
		if (IsEOList(playDst))
			break;

		if (playDst->cxTile && playDst->cyTile)
		{
			if (playSrc->cxTile != playDst->cxTile)
				return FALSE;
			if (playSrc->cyTile != playDst->cyTile)
				return FALSE;
		}
		playSrc = Next(playSrc);
		playDst = Next(playDst);
	}
	return TRUE;

} /* FMayDrawHere */


/**************************************************************************
 *
 * RealButtonsE
 *
 * PURPOSE
 *	Change the state of tume depending on previous state and mouse events.
 *
 * USAGE
 *	RealButtonsE (BUTTON_EVENT_FLAG);
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
 *		12/12/93 (dcc) - Pass ctilx,ctily to StartUndo()(not cxTile,cyTile!)
 *		04/02/95 (dcc) - KLUDGE: check to make sure floor layer has tiles
 *					  it it as much code assumes floor layer of brush
 *					  has tiles with non-zero width & height.
 *
 * SEE ALSO
 *
*/
void RealButtonsE (BOOL buttons)
{
	RoomType	*rw_room;
//04/02/95 KLUDGE	LayerType *play;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RealButtonsE";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!	((GlobalRoomWindow) &&
			(GlobalRoomWindow->CurrentRoom) &&
			(GlobalRoomWindow->CurrentRoom->Room) &&
			(GlobalBlockCopy)) ) {
		goto ABORT;
	}

#if __MSDOS__
	{
		MouseInfo mi = {-1, -1, 0, 0};

		ReadMouse(&mi);
		LeftButton = mi.Buttons & MOUSE_LEFTBUTTON;
		RightButton = mi.Buttons & MOUSE_RIGHTBUTTON;
	}
#endif/*__MSDOS__*/

	rw_room = GlobalRoomWindow->CurrentRoom->Room;
//04/02/95 KLUDGE	play = FindNonEmptyLayer(&rw_room->Layers, rw_room->FloorLayer);

	if	/* it is possible to draw here */
	(
		(! (IsEOList (Head (&(GlobalBlockCopy->Layers))))) &&
		(!(rw_room->Flags & SOURCEROOM)) &&
		(!(rw_room->Flags & LOCKEDROOM)) &&
		FMayDrawHere(GlobalBlockCopy, rw_room)
	) {

		if (LeftButton || RightButton) {

			FollowCursor (&CurrentX, &CurrentY, &ScrollDX, &ScrollDY,
				 GlobalBlockCopy->SourceRoom, FALSE);

#if PixelLayers
		{
			int dtilx, dtily, ctilx, ctily;

			dtilx = CurrentX;
			dtily = CurrentY;
			ctilx = ((LayerType *) Head(&GlobalBlockCopy->Layers))->ctilx;
			ctily = ((LayerType *) Head(&GlobalBlockCopy->Layers))->ctily;

			BigTileCoordsFromSmallTile(&GlobalRoomWindow->CurrentRoom->Room->Layers,
								GlobalRoomWindow->CurrentRoom->Room->FloorLayer,
								&dtilx, &dtily, &ctilx, &ctily);

			StartUndo(GlobalRoomWindow->CurrentRoom->Room, UndoRoom,
					dtilx, dtily, dtilx+ctilx-1, dtily+ctily-1);
		}
#else // !PixelLayers
			StartUndo (GlobalRoomWindow->CurrentRoom->Room, UndoRoom,
				CurrentX, CurrentY,
				(WORD) ((CurrentX + ((LayerType *) Head(&GlobalBlockCopy->Layers))->ctilx) - 1),
				(WORD) ((CurrentY + ((LayerType *) Head(&GlobalBlockCopy->Layers))->ctily) - 1));
#endif // !PixelLayers

			OldColumn	= OldRow	= -1;

			FN_DontWait	= (MouseFunc) Drawing;
			DontWait	= TRUE;

			if (RightButton) {
				DrawMode = ERASE;
			}
			else {
				DrawMode = DRAW;
			}

		/*	Here we invoke FN_DontWait() ourselves directly NOW,
			since if there are many layers, StartUndo() will take
			a good quantity of time to run, and the mouse button
			may have been released already before we return to
			HitThisArea() (and that function can do its repeat). */

			FN_DontWait(&hitGlobal);
		}
		else {
		/*	We've place this call here. The condition we are looking for
			is the first time a tile-brush is about to be re-drawn in
			a room. We need to call LimitTilePositions() to see if there
			are any special tiles (such as isometric) with special
			placement requirements. */

			if (FN_DontWait == Hovering)
			{
				int i;

				i = DontWait;
				DontWait = i;
			}

			FN_DontWait		= Tracking;
			DontWait		= TRUE;
		}
	}
	else if ((rw_room->FloorLayer->cxTile) && (rw_room->FloorLayer->cyTile)) {
//04/02/95 KLUDGE	else if (play) {
		if (LeftButton || RightButton) {
			if (buttons) {
				Unselect ();
				StartSelect ();
				ShowState (LastTBar);
			}
			FN_DontWait	= Selecting;
			DontWait	= TRUE;
			if ( RightButton && 
					(! (rw_room->Flags & LOCKEDROOM)) &&
					(! (rw_room->Flags & SOURCEROOM)) ) {
				SelectMode = CUT;
			}
			else {
				SelectMode = COPY;
			}
		}
		else {
			DontWait	= FALSE;
#if __AMIGAOS__
			FN_DontWait	= NULL;
#elif __MSDOS__
			FN_DontWait	= Hovering;
#endif/*__AMIGAOS__/__MSDOS__*/
			if (buttons) {
				FinishSelect ();
			}
		}
	}
	else {
		DontWait		= FALSE;
		FN_DontWait	= NULL;
	}

	return;
/*------------------------------------------------------------------------*/
ABORT:
	return;

} /* RealButtonsE */


/**************************************************************************
 *
 * MButtonsE
 *
 * SYNOPSIS
 *		void MButtonsE(void)
 *
 * PURPOSE
 *		Menu interface to the state changer (RealButtonsE).
 *
 * USAGE
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
void MButtonsE(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MButtonsE";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	RealButtonsE (FALSE);
	return;

} /* MButtonsE */


/**************************************************************************
 *
 * ButtonsE
 *
 * SYNOPSIS
 *		void ButtonsE (void)
 *
 * PURPOSE
 *		Button interface to the state changer (RealButtonsE).
 *
 * USAGE
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
void ButtonsE (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ButtonsE";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	RealButtonsE (TRUE);
	return;

} /* ButtonsE */


#if __AMIGAOS__
/**************************************************************************
 *
 * EraseIndicators
 *
 * SYNOPSIS
 *		static void EraseIndicators (void)
 *
 * PURPOSE
 *		Erase the select box and tracking cursor.
 *
 * USAGE
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
static void EraseIndicators (void)
{
	RoomType	*currentroom;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "EraseIndicators";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	currentroom = GlobalRoomWindow->CurrentRoom->Room;

	if (FSelectedPblk(GlobalBlockCopy) && (currentroom)) {

		if ((SelectMode != OFF) && 
			(!(currentroom->Flags & SOURCEROOM)) &&
			(!(currentroom->Flags & LOCKEDROOM))) {

			DrawSelect (GlobalBlockCopy, DEST_EOR, TRUE);
		}
		if ((currentroom == GlobalBlockCopy->SourceRoom) && 
			(GlobalRoomWindow->Window
				== GlobalBlockCopy->SourceRW->Window)) {
			DrawSelect (GlobalBlockCopy, SOURCE_EOR, FALSE);
		}
	}
} /* EraseIndicators */


/**************************************************************************
 *
 * DrawIndicators
 *
 * SYNOPSIS
 *		void DrawIndicators (void)
 *
 * PURPOSE
 *		Draw the select box and tracking cursor.
 *
 * USAGE
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
static void DrawIndicators (void)
{
	RoomType	*currentroom;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawIndicators";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	currentroom = GlobalRoomWindow->CurrentRoom->Room;

	if (FSelectedPblk(GlobalBlockCopy) && (currentroom)) {

		if ((currentroom == GlobalBlockCopy->SourceRoom) && 
			(GlobalRoomWindow->Window
				== GlobalBlockCopy->SourceRW->Window)) {
			DrawSelect (GlobalBlockCopy, SOURCE_EOR, FALSE);
		}
		if ((SelectMode != OFF) && 
			(!(currentroom->Flags & SOURCEROOM)) &&
			(!(currentroom->Flags & LOCKEDROOM))) {

			DrawSelect (GlobalBlockCopy, DEST_EOR, TRUE);
		}
	}
} /* DrawIndicators */
#endif/*__AMIGAOS__*/


#if 0
/**************************************************************************
 *
 * RightScroll
 *
 * SYNOPSIS
 *		int RightScroll (void)
 *
 * PURPOSE
 *		Try to scroll RIGHT.
 *
 * USAGE
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
int RightScroll (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RightScroll";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	MoveRequest (-1, 0);
	return (TRUE);

} /* RightScroll */

int RightScrollMultiple (void)
{
	MoveRequest (-5, 0);
	return (TRUE);

}

/**************************************************************************
 *
 * LeftScroll
 *
 * SYNOPSIS
 *		int LeftScroll (void)
 *
 * PURPOSE
 *		Try to scroll LEFT.
 *
 * USAGE
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
int LeftScroll (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LeftScroll";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	MoveRequest (1, 0);
	return (TRUE);

} /* LeftScroll */

int LeftScrollMultiple (void)
{
	MoveRequest (5, 0);
	return (TRUE);

}


/**************************************************************************
 *
 * UpScroll
 *
 * SYNOPSIS
 *		int UpScroll (void)
 *
 * PURPOSE
 *		Try to scroll UP.
 *
 * USAGE
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
int UpScroll (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpScroll";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	MoveRequest (0, 1);
	return (TRUE);

} /* UpScroll */

int UpScrollMultiple (void)
{
	MoveRequest (0, 5);
	return (TRUE);

}


/**************************************************************************
 *
 * DownScroll
 *
 * SYNOPSIS
 *		int DownScroll (void)
 *
 * PURPOSE
 *		Try to scroll DOWN.
 *
 * USAGE
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
int DownScroll (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DownScroll";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	MoveRequest (0, -1);
	return (TRUE);

} /* DownScroll */

int DownScrollMultiple (void)
{
	MoveRequest (0, -5);
	return (TRUE);

}
#endif


/**************************************************************************
 *
 * GetCursorTilePosition
 *
 * PURPOSE
 *		To get the cursor's position in a room.
 *
 *		Note that return values are expressed in terms of the largest tile
 *		in the current room.
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		<*cursor_column> and <*cursor_row> filled with cursor's position.
 *
 * HISTORY
 *		01/22/91 (RGM) - Created.
 *		11/11/94 (dcc) - Change ULONG casts to LONG.
 *		03/19/95 (dcc) - Fix divide by 0 error (add cxTilF, cxTilL).
 *
 * SEE ALSO
 *
*/
void GetCursorTilePosition (
	int			*cursor_column,
	int			*cursor_row,
	RoomWindowType	*rw
)
{
	int	mouse_x, mouse_y;
	int	mouse_window_x, mouse_window_y;
	int	upperleft_x, upperleft_y;
	unsigned	init_column, init_row;
	int	max_column, max_row;
	int	mouse_window_column, mouse_window_row;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetCursorTilePosition";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (rw->CurrentRoom->Room)
	{
		RoomType	*prm = rw->CurrentRoom->Room;
		UWORD cxTilF, cyTilF;		// size of tile in floor
		UWORD cxTilL, cyTilL;		// size of tile in largest
		int	plot_width, plot_height;

		if (prm->FloorLayer->cxTile != 0 && prm->FloorLayer->cyTile != 0)
		{
			cxTilF = prm->FloorLayer->cxTile;
			cyTilF = prm->FloorLayer->cyTile;
		}
		else
		{
			if (FSelected())
			{
				if (GlobalBlockCopy->BC_FloorLayer->cxTile != 0 &&
				    GlobalBlockCopy->BC_FloorLayer->cyTile != 0)
				{
					cxTilF = GlobalBlockCopy->BC_FloorLayer->cxTile;
					cyTilF = GlobalBlockCopy->BC_FloorLayer->cyTile;
				}
			}
			else
			{
				cxTilF = 0;
				cyTilF = 0;
			}
		}

		/* Find size of tile in layer with largest tiles. */

		{
			LayerType *play;
#if PixelLayers
			if ((play = FindBiggestLayer(&prm->Layers)) != NULL)
#else // !PixelLayers
			if ((play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer)) != NULL)
#endif // !PixelLayers
			{
				cxTilL = play->cxTile;
				cyTilL = play->cyTile;
			}
			else
			{
				cxTilL = cxTilF;
				cyTilL = cyTilF;
			}
		}

		plot_width	= wSrcSizeToDstSize(cxTilL, wSrcSkipX, wDstDupX);
		plot_height	= wSrcSizeToDstSize(cyTilL, wSrcSkipY, wDstDupY);

		if (plot_width && plot_height && cxTilF && cyTilF)
		{
			int		tile_separation;

			tile_separation = rw->CurrentRoom->Flags & DISPLAY_SEPERATED;
			plot_width	+= tile_separation;
			plot_height	+= tile_separation;

#if __AMIGAOS__
			mouse_x	= rw->Window->MouseX;
			mouse_y	= rw->Window->MouseY;
#elif __MSDOS__
			mouse_x	= MouseX;
			mouse_y	= MouseY;
#endif/*__AMIGAOS__/__MSDOS__*/

			upperleft_x	= 0;
#if __AMIGAOS__
			upperleft_y	= TopAdd;
#elif __MSDOS__
			upperleft_y	= 0;
#endif/*__AMIGAOS__/__MSDOS__*/

			Assert(cxTilF != 0);
			init_column	= (unsigned) (rw->CurrentRoom->dxSrc / cxTilF);
			Assert(cyTilF != 0);
			init_row		= (unsigned) (rw->CurrentRoom->dySrc / cyTilF);

			max_column	= prm->FloorLayer->ctilx;
			max_row		= prm->FloorLayer->ctily;

			mouse_window_x	= mouse_x - upperleft_x;
			mouse_window_y	= mouse_y - upperleft_y;

			Assert(plot_width != 0);
			mouse_window_column	= (int) (((LONG) mouse_window_x * cxTilL / cxTilF) / plot_width);
			Assert(plot_height != 0);
			mouse_window_row	= (int) (((LONG) mouse_window_y * cyTilL / cyTilF) / plot_height);

			*cursor_column	= init_column + mouse_window_column;
			*cursor_row	= init_row + mouse_window_row;

			*cursor_column	= min (max (*cursor_column, 0), max_column);
			*cursor_row	= min (max (*cursor_row, 0), max_row);

			return;
		} 
	}
	*cursor_column	= 0;
	*cursor_row	= 0;

} /* GetCursorTilePosition */


/*********************************************************************
 *
 * MoveCenterTo
 *
 * PURPOSE
 *		Move the center of the screen to <wx>, <wy>.
 *
 * INPUT
 *		fScroll:	if TRUE, scroll to new location, else re-draw screen
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		08/28/94 (dcc) - add support for PixelLayers.
 *		10/19/94 (dcc) - Fix parameters passed to SaveSmartFlipNewXY().
 *		11/11/94 (dcc) - Change ULONG casts to LONG.
 *
 * SEE ALSO
 *
*/
void MoveCenterTo(int wx, int wy, BOOL fScroll)
{

	WORD	dx, dy;
	RoomType	*room;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MoveCenterTo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room	= GlobalRoomWindow->CurrentRoom->Room;
	if (room) {
		LayerType *play;
		WORD	extra;
		WORD	plot_width;
		WORD	plot_height;
		WORD	w_height;
		WORD	w_width;

#if PixelLayers
		if ((play = FindBiggestLayer(&room->Layers)) == NULL)
#else // !PixelLayers
		if ((play = FindNonEmptyLayer(&room->Layers, room->FloorLayer)) == NULL)
#endif // !PixelLayers
		{
			play = room->FloorLayer;
		}
#if PixelLayers
		if (room->FloorLayer->cxTile && play->cxTile)
			wx = (int) ((LONG) wx * room->FloorLayer->cxTile / play->cxTile);

		if (room->FloorLayer->cyTile && play->cyTile)
			wy = (int) ((LONG) wy * room->FloorLayer->cyTile / play->cyTile);
#endif // PixelLayers

		extra = ((GlobalRoomWindow->CurrentRoom->Flags) & DISPLAY_SEPERATED);
		plot_width	= wSrcSizeToDstSize(play->cxTile, wSrcSkipX, wDstDupX) + extra;
		plot_height	= wSrcSizeToDstSize(play->cyTile, wSrcSkipY, wDstDupY) + extra;

		if (plot_width && plot_height)
		{
			w_width		= GlobalRoomWindow->Window->Width / plot_width;
			if (wShowXTiles > 0)
				w_width = min(w_width, wShowXTiles);
			w_height		= GlobalRoomWindow->Window->Height / plot_height;
			if (wShowYTiles > 0)
				w_width = min(w_width, wShowYTiles);

			{
				Assert(play->cxTile != 0);
				dx =	(unsigned) (GlobalRoomWindow->CurrentRoom->dxSrc / play->cxTile) + (w_width / 2);
				dx -=	wx;		/* CurrentX */

				Assert(play->cyTile != 0);
				dy =	(unsigned) (GlobalRoomWindow->CurrentRoom->dySrc / play->cyTile) + (w_height / 2);
				dy -=	wy;		/* CurrentY */
			}

			if (fScroll)
			{
				MoveRequest (dx, dy);
			}
			else
			{
				if (OKToScroll(GlobalRoomWindow, &dx, &dy))
				{
					GlobalRoomWindow->CurrentRoom->dxSrc -= (LONG) dx * play->cxTile;
					GlobalRoomWindow->CurrentRoom->dySrc -= (LONG) dy * play->cyTile;

					Assert(play->cxTile != 0);
					Assert(play->cyTile != 0);
					SaveSmartFlipNewXY(	GlobalRoomWindow->CurrentRoom->Room,
									GlobalRoomWindow->CurrentRoom->dxSrc,
									GlobalRoomWindow->CurrentRoom->dySrc);
				}
				ShowRoom(GlobalRoomWindow);
			}
		}
	}
} /* MoveCenterTo */


int CenterRoomOnCursor (void)
{
	int wx, wy;

	GetCursorTilePosition (&wx, &wy, GlobalRoomWindow);

	MoveCenterTo(wx, wy, TRUE);

	return (TRUE);
}


/**************************************************************************
 *
 * MoveRequest
 *
 * PURPOSE
 *		Try to scroll <x_magnitude, y_magnitude>.
 *
 * USAGE
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
void MoveRequest (
	WORD	x_magnitude,
	WORD	y_magnitude
)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MoveRequest";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ScrollDX	=	x_magnitude;
	ScrollDY	=	y_magnitude;

	if (OKToScroll (GlobalRoomWindow, &ScrollDX, &ScrollDY)) {

#if __AMIGAOS__
		EraseIndicators ();
#endif/*__AMIGAOS__*/

		Scroll (GlobalRoomWindow, ScrollDX, ScrollDY, TRUE);

#if __AMIGAOS__
		DrawIndicators ();
#endif/*__AMIGAOS__*/
	}
	return;

} /* MoveRequest */


/*********************************************************************
 *
 * ShiftLayer
 *
 * PURPOSE
 *		Shift current layer by <dtilx, dtily>.
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
 *		02/20/94 Sunday (dcc) - created.
 *
*/
void ShiftLayer(short dtilx, short dtily)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShiftLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom)
	{
		RoomType *prm = GlobalRoomWindow->CurrentRoom->Room;
		if (prm)
		{
			LayerType *play = prm->FloorLayer;

			if (!(play->L_Flags & LAYER_LOCKED))
			{
				play->dtilx -= dtilx;
				play->dtily -= dtily;

				ShowRoom(GlobalRoomWindow);
				ShowState(LastTBar);
			}
		}
	}
} /* ShiftLayer */


/*********************************************************************
 *
 * UpdateStatusBar
 *
 * SYNOPSIS
 *		void UpdateStatusBar(void)
 *
 * PURPOSE
 *		Gets called by Hovering() and Tracking() to update the status
 *		display.
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
void UpdateStatusBar(RoomType *croom)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateStatusBar";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (croom)
	{
		PlotType *plot;
		TileSetType	*tileset	= NULL;
		BOOL		noimages;

		plot = GetRoomPlot (croom, CurrentX, CurrentY);
		if (plot != NULL && plot->TileSet_ID != 0 && plot->Tile_ID != 0) {
			tileset	= TILESET_AT_PLOT(plot);
		}
		else {
			tileset	= NULL;
		}
		noimages	= tileset ? (tileset->Flags & DONT_SAVEIMAGES) : FALSE;

		if (ShowTitleSTATE && fShowDefaultCursor) {
#if __MSDOS__
			if (PushWindowClipValues (MainWindow))
			{
				BeforeGraphics ();
#endif/*__MSDOS__*/
				if (IsSource(croom)) {
					char sz[256];

					if (tileset) {
						if (tileset->Filespec) {
							if (noimages) {
								strcpy (sz, "-");
							}
							else {
								strcpy (sz, " ");
							}
							EIO_fnsplit (tileset->Filespec, NULL,
								TempName, TempExt);
							strncat (TempName, TempExt, 20);
							strncat (sz, TempName, 12);
						}
						else {
							strcpy (sz, " ??? ??? ??? ");
						}
					}
					else {
							strcpy (sz, " (null tile) ");
					}
#if __AMIGAOS__
					Move (&MainScreen->RastPort, SNX, TextTop);
					Text (&MainScreen->RastPort, sz, 13);
#elif __MSDOS__
					SetPenColor (MainWindow->White);
					DrawRect (SNX, 0, 13*8, TITLE_HEIGHT - 1);
					SetPenColor (MainWindow->Black);
					DrawString (MainWindow->Font, SNX, 1, sz);
#endif/*__AMIGAOS__/__MSDOS__*/
				}
				else if (ShowCursor && 
						(IsEdit(croom) || (IsComposite(croom) && !IsLocked(croom)))) {
					sprintf (X_Pos, "%05d", CurrentX);
					sprintf (Y_Pos, "%05d", CurrentY);
#if __AMIGAOS__
					Move (&MainScreen->RastPort, XX, TextTop);
					Text (&MainScreen->RastPort, X_Pos, 5);

					Move (&MainScreen->RastPort, YX, TextTop);
					Text (&MainScreen->RastPort, Y_Pos, 5);
#elif __MSDOS__
					ShowCoordinates();
#endif/*__AMIGAOS__/__MSDOS__*/
				}
#if __MSDOS__
				AfterGraphics ();
				PopClipValues ();
			}
#endif/*__MSDOS__*/
		}

		else if (ShowTitleSTATE && ((ShowUser && IsSource(croom)) || ShowTUser)) {
			LayerType	*roomlayer	= NULL;

			roomlayer	= croom->FloorLayer;

			if (croom) {
				if (IsComposite(croom)) {
					strcpy (TitleStuff, "C");
				}
				else {
					if (IsSource(croom)) {
						strcpy (TitleStuff, "S");
					}
					else {
						strcpy (TitleStuff, "E");
					}
				}
				if (IsLocked(croom)) {
					strcat (TitleStuff, "L");
				}
				else {
					strcat (TitleStuff, "U");
				}
				if ((roomlayer) && (tileset)) {
					if (tileset->Filespec) {
						if (noimages) {
							strcat (TitleStuff, "-");
						}
						else {
							strcat (TitleStuff, " ");
						}
						EIO_fnsplit (tileset->Filespec, NULL,
							TempName, TempExt);
						strncat (TempName, TempExt, 20);
						sprintf (HoldBuffer, "%-12s", TempName);
						strncat (TitleStuff, HoldBuffer, 12);
					}
					else {
						strcat (TitleStuff, " ??? ??? ??? ");
					}
					strcat (TitleStuff, " ");
					if (ShowUser) {
						strcat (TitleStuff, "TT:");
						sprintf (HoldBuffer, "%05d", tileset->UserType);
						strncat (TitleStuff, HoldBuffer, 5);

						strcat (TitleStuff, " ");

						strcat (TitleStuff, "TN:");
						sprintf (HoldBuffer, "%05d", tileset->UserNumber);
						strncat (TitleStuff, HoldBuffer, 5);
					} else {
						sprintf (HoldBuffer, "%05d", plot->Tile_ID);
						strncat (TitleStuff, HoldBuffer, 5);

						strcat (TitleStuff, " ");
						sprintf (HoldBuffer, "%05d", tileset->UserType);
						strncat (TitleStuff, HoldBuffer, 5);

						strcat (TitleStuff, " ");
						sprintf (HoldBuffer, "%05d", tileset->UserNumber);
						strncat (TitleStuff, HoldBuffer, 5);
					}
				}
				else {
					strcat (TitleStuff, " (null tile)                   ");
				}
				if (croom->FloorLayer->dtilx == 0 && croom->FloorLayer->dtily == 0)
					strcat (TitleStuff, " ");
				else
					strcat (TitleStuff, "*");
				sprintf (HoldBuffer, "%02d", croom->FloorNumber);
				strncat (TitleStuff, HoldBuffer, 2);

				if (croom->FloorLayer->L_Flags & LAYER_INVISIBLE) {
					strcat (TitleStuff, "i");
				}
				else {
					strcat (TitleStuff, "v");
				}

				if (croom->FloorLayer->L_Flags & LAYER_LOCKED) {
					strcat (TitleStuff, "l");
				}
				else {
					strcat (TitleStuff, "u");
				}
				if (EditOnlyFloorSTATE) {
					strcat (TitleStuff, "f");
				}
				else {
					strcat (TitleStuff, " ");
				}
			}
			else {
				strcpy (TitleStuff, "(no rooms attached)");
			}
#if __AMIGAOS__
			Move (&MainScreen->RastPort, (8 - 8 + 2), TextTop);
			Text (&MainScreen->RastPort, TitleStuff, strlen (TitleStuff));
#elif __MSDOS__
			if (PushWindowClipValues (MainWindow))
			{
				BeforeGraphics ();
				SetPenColor (MainWindow->White);
				DrawRect (8 - 8 + 1, 0, SCREEN_WIDTH - 8 - (8 - 8 + 1), TITLE_HEIGHT - 1);
				SetPenColor (MainWindow->Black);
				DrawString (MainWindow->Font, 8 - 8 + 1, 1, TitleStuff);
				AfterGraphics ();
				PopClipValues ();
			}
#endif/*__AMIGAOS__/__MSDOS__*/
		}
	}

} /* UpdateStatusBar */


#if 1
/*********************************************************************
 *
 * SnapToGrid
 *
 * PURPOSE
 *		Adjust <*dx> and <*dy> to nearest grid position if
 *		there is a grid active in room.
 *
 *		BUGBUG: currently, if there are special tiles (like
 *		isometric tiles), we will only snap to the special tiles,
 *		and ignore the grid completely.
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
 *		12/15/93 Wednesday (dcc) - created.
#if PIXELSELECT
 *		01/16/94 Sunday (dcc) - add support for GlobalBlockCopy->dtilxOffset
 *		02/19/94 Saturday (dcc) - fix so we actually snap to special tiles
#endif // PIXELSELECT
 *
*/
static void SnapToGrid(RoomType *prm, WORD *dx, WORD *dy)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SnapToGrid";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalBlockCopy->pSpecialTilesets)
	{
		SpecialTileset *psts;

		psts = Head(GlobalBlockCopy->pSpecialTilesets);

		while (!IsEOList(psts))
		{
			int i, x0, mody;
			int dtilx, dtily;		// KLUDGE
			TileSetType *ptst = FAST_TILESET_PTR(psts->TileID);

			mody = ptst->ctilyDispO + ptst->ctilyDispE;

#if PIXELSELECT
//			i = (*dy+psts->dtily) % mody;
			dtily = psts->dtily;	// KLUDGE
			if (dtily < 0)			// KLUDGE
				dtily += mody;		// KLUDGE

			i = (*dy+dtily) % mody;	// KLUDGE
#endif // PIXELSELECT
#if !PIXELSELECT
			i = *dy % mody;
#endif // !PIXELSELECT
			if (i < ptst->ctilyDispO)
			{
				*dy -= i;
				x0 = 0;
			}
			else
			{
				*dy -= (i - ptst->ctilyDispO);
				x0 = ptst->ctilxDispO;
			}
#if PIXELSELECT
			Assert(mody != 0);
			x0 += ((*dy+psts->dtily) / mody) * (ptst->ctilxDispO + ptst->ctilxDispE);
//			i = ((*dx+psts->dtilx) % ptst->ctilxSkip - x0 % ptst->ctilxSkip);
			dtilx = psts->dtilx;	// KLUDGE
			if (dtilx < 0)			// KLUDGE
				dtilx += ptst->ctilxSkip;	// KLUDGE

			i = ((*dx+dtilx) % ptst->ctilxSkip - x0 % ptst->ctilxSkip);	// KLUDGE
#endif // PIXELSELECT
#if !PIXELSELECT
			x0 += (*dy / mody) * (ptst->ctilxDispO + ptst->ctilxDispE);
			i = (*dx % ptst->ctilxSkip - x0 % ptst->ctilxSkip);
#endif // !PIXELSELECT
			if (i < 0)
				i += ptst->ctilxSkip;
			*dx -= i;
			psts = Next(psts);
		}
		return;
	}
	if (IsGridOn(prm))
	{
		WORD w;

		w = (*dx % prm->wGridXWidth - prm->wGridXOrigin % prm->wGridXWidth);
		if (w < 0)
			w += prm->wGridXWidth;
		*dx -= w;
		if (*dx < 0)
			*dx += prm->wGridXWidth;

		w = (*dy % prm->wGridYHeight - prm->wGridYOrigin % prm->wGridYHeight);
		if (w < 0)
			w += prm->wGridYHeight;
		*dy -= w;
		if (*dy < 0)
			*dy += prm->wGridYHeight;
	}
} /* SnapToGrid */
#else
/*********************************************************************
 *
 * SnapToGrid
 *
 * PURPOSE
 *		Adjust <*dx> and <*dy> to nearest grid position if
 *		there is a grid active in room.
 *
 *		BUGBUG: currently, if there are special tiles (like
 *		isometric tiles), we will only snap to the special tiles,
 *		and ignore the grid completely.
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
 *		12/15/93 Wednesday (dcc) - created.
#if PIXELSELECT
 *		01/16/94 Sunday (dcc) - add support for GlobalBlockCopy->dtilxOffset
#endif // PIXELSELECT
 *
*/
static void SnapToGrid(RoomType *prm, WORD *dx, WORD *dy)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SnapToGrid";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalBlockCopy->pSpecialTilesets)
	{
		SpecialTileset *psts;

		psts = Head(GlobalBlockCopy->pSpecialTilesets);

		while (!IsEOList(psts))
		{
			int i, x0, mody;
			TileSetType *ptst = FAST_TILESET_PTR(psts->TileID);

			mody = ptst->ctilyDispO + ptst->ctilyDispE;

#if PIXELSELECT
			i = (*dy+GlobalBlockCopy->dtilyOffset) % mody;
#endif // PIXELSELECT
#if !PIXELSELECT
			i = *dy % mody;
#endif // !PIXELSELECT
			if (i < ptst->ctilyDispO)
			{
				*dy -= i;
				x0 = 0;
			}
			else
			{
				*dy -= (i - ptst->ctilyDispO);
				x0 = ptst->ctilxDispO;
			}
#if PIXELSELECT
			Assert(mody != 0);
			x0 += ((*dy+GlobalBlockCopy->dtilyOffset) / mody) * (ptst->ctilxDispO + ptst->ctilxDispE);
			i = ((*dx+GlobalBlockCopy->dtilxOffset) % ptst->ctilxSkip - x0 % ptst->ctilxSkip);
#endif // PIXELSELECT
#if !PIXELSELECT
			x0 += (*dy / mody) * (ptst->ctilxDispO + ptst->ctilxDispE);
			i = (*dx % ptst->ctilxSkip - x0 % ptst->ctilxSkip);
#endif // !PIXELSELECT
			if (i < 0)
				i += ptst->ctilxSkip;
			*dx -= i;
			psts = Next(psts);
		}
		return;
	}
	if (IsGridOn(prm))
	{
		WORD w;

		w = (*dx % prm->wGridXWidth - prm->wGridXOrigin % prm->wGridXWidth);
		if (w < 0)
			w += prm->wGridXWidth;
		*dx -= w;
		if (*dx < 0)
			*dx += prm->wGridXWidth;

		w = (*dy % prm->wGridYHeight - prm->wGridYOrigin % prm->wGridYHeight);
		if (w < 0)
			w += prm->wGridYHeight;
		*dy -= w;
		if (*dy < 0)
			*dy += prm->wGridYHeight;
	}
} /* SnapToGrid */
#endif


/**************************************************************************
 *
 * Hovering
 *
 * PURPOSE
 *		Track the cursor in preparation for a selection event.
 *
 * USAGE
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
#if __AMIGAOS__
static void Hovering (void)
#elif __MSDOS__
static short Hovering(HitInfo *hi)
#endif/*__AMIGAOS__/__MSDOS__*/
{
	RoomType	*croom;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Hovering";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	hi = hi;	/* Turn off warning */

	croom = GlobalRoomWindow->CurrentRoom->Room;


	if (!croom || !FindNonEmptyLayer(&croom->Layers, croom->FloorLayer))
	{
/**/		goto ABORT;
	}

	if (FollowCursor (&CurrentX, &CurrentY, &ScrollDX, &ScrollDY,
			 GlobalBlockCopy->SourceRoom, FALSE)) {

		UpdateStatusBar(croom);
	}
#if __AMIGAOS__
	return;
#elif __MSDOS__
	return TRUE;
#endif/*__AMIGAOS__/__MSDOS__*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:
#if __AMIGAOS__
	return;
#elif __MSDOS__
	return FALSE;
#endif/*__AMIGAOS__/__MSDOS__*/

} /* Hovering */


/**************************************************************************
 *
 * Tracking
 *
 * SYNOPSIS
 *		void Tracking (void)
 *
 * PURPOSE
 *		Track the cursor (dragging brush around) in preparation
 *		for a draw event.
 *
 * USAGE
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
#if __AMIGAOS__
static void Tracking (void)
#elif __MSDOS__
static short Tracking(HitInfo *hi)
#endif/*__AMIGAOS__/__MSDOS__*/
{
	RoomType	*sroom;
	RoomType	*croom;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Tracking";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	hi = hi;	/* Turn off warning */

	if (! FSelectedPblk(GlobalBlockCopy)) {
/**/		goto ABORT;
	}

	sroom = GlobalBlockCopy->SourceRoom;
	croom = GlobalRoomWindow->CurrentRoom->Room;

	if (! (sroom && croom)) {
/**/		goto ABORT;
	}

	if (!FMayDrawHere(GlobalBlockCopy, croom))
	{
/**/	goto ABORT;
	}

	if (FollowCursor (&CurrentX, &CurrentY, &ScrollDX, &ScrollDY,
			 GlobalBlockCopy->SourceRoom, FALSE)) {

		/* Dragging already selected brush around.
			Movement: erase the old brush. */

		HideBrush();

		GlobalBlockCopy->DestRW		= GlobalRoomWindow;
		GlobalBlockCopy->DestStuff	= GlobalRoomWindow->CurrentRoom;

		SnapToGrid(croom, &CurrentX, &CurrentY);

		GlobalBlockCopy->DestX		= CurrentX;
		GlobalBlockCopy->DestY		= CurrentY;

		/* Dragging already selected brush around.
			Movement: draw the new brush. */

		ShowBrush();

		UpdateStatusBar(croom);
	}
#if __AMIGAOS__
	return;
#elif __MSDOS__
	return TRUE;
#endif/*__AMIGAOS__/__MSDOS__*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:
#if __AMIGAOS__
	return;
#elif __MSDOS__
	return FALSE;
#endif/*__AMIGAOS__/__MSDOS__*/

} /* Tracking */


/**************************************************************************
 *
 * Drawing
 *
 * SYNOPSIS
 *		void Drawing (void)
 *
 * PURPOSE
 *		Drawing with the selected tile(s).
 *
 * USAGE
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
#if __AMIGAOS__
static void Drawing (void)
#else
static short Drawing(HitInfo *hi)
#endif/*__AMIGAOS__*/
{
#if 0
	BOOL	same_as_source;
#endif
	RoomType *sroom;
	RoomType *croom;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Drawing";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	hi = hi;	/* Turn off warning */

	if (! FSelectedPblk(GlobalBlockCopy)) {
		goto ABORT;
	}

	sroom = GlobalBlockCopy->SourceRoom;
	croom = GlobalRoomWindow->CurrentRoom->Room;

	if (! (sroom && croom)) {
/**/		goto ABORT;
	}

	if (!FMayDrawHere(GlobalBlockCopy, croom))
	{
/**/	goto ABORT;
	}

	if (FollowCursor (&CurrentX, &CurrentY, &ScrollDX, &ScrollDY,
			 GlobalBlockCopy->SourceRoom, TRUE)) {
		/******************************/
		/* undraw the tracking cursor */

		HideBrush();

		Scroll (GlobalRoomWindow, ScrollDX, ScrollDY, FALSE);

		GlobalBlockCopy->DestRW		= GlobalRoomWindow;
		GlobalBlockCopy->DestStuff	= GlobalRoomWindow->CurrentRoom;

		SnapToGrid(croom, &CurrentX, &CurrentY);

		GlobalBlockCopy->DestX		= CurrentX;
		GlobalBlockCopy->DestY		= CurrentY;


#if 0
		same_as_source = 
		((GlobalBlockCopy->SourceRoom == GlobalBlockCopy->DestStuff->Room)
		&&
		(GlobalBlockCopy->SourceRW->Window ==
			GlobalBlockCopy->DestRW->Window));

#if __AMIGAOS__
		if (same_as_source) {
#elif __MSDOS__
		if (ScrollDX == 0 && ScrollDY == 0 && same_as_source) {
#endif/*__AMIGAOS__/__MSDOS__*/
			/* ERASE SOURCE OUTLINE */

			XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);
		}
#endif

		PlaceBlockCopy (GlobalBlockCopy, DrawMode);

#if 0
		if (same_as_source) {
			/* REDRAW SOURCE OUTLINE */

			XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);
		}
#endif

		ShowBrush();

		if (ShowCursor && ShowTitleSTATE) {
			sprintf (X_Pos, "%05d", CurrentX);
			sprintf (Y_Pos, "%05d", CurrentY);
#if __AMIGAOS__
			Move (&MainScreen->RastPort, XX, TextTop);
			Text (&MainScreen->RastPort, X_Pos, 5);

			Move (&MainScreen->RastPort, YX, TextTop);
			Text (&MainScreen->RastPort, Y_Pos, 5);
#elif __MSDOS__
			ShowCoordinates();
#endif/*__AMIGAOS__/__MSDOS__*/
		}
	}
#if __AMIGAOS__
	return;
#elif __MSDOS__
	return TRUE;
#endif/*__AMIGAOS__/__MSDOS__*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:
#if __AMIGAOS__
	return;
#elif __MSDOS__
	return FALSE;
#endif/*__AMIGAOS__/__MSDOS__*/

} /* Drawing */


/**************************************************************************
 *
 * Unselect
 *
 * PURPOSE
 *		Unselect any selected tile(s).
 *
 * USAGE
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
 *		12/16/93 (dcc) - added code to empty pSpecialTilesets.
 *		09/07/94 (dcc) - call DCC_SetPointer() instead of DCC_TempSetPointer()
 *
 * SEE ALSO
 *
*/
void Unselect (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Unselect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	if (GlobalPointer != (&GrabMPointer)) {
		Old_Pointer		= GlobalPointer;
		GlobalPointer	= RGM_SetPointer (&GrabMPointer, MainWindow);
	}
#elif __MSDOS__
	GlobalPointer = DCC_SetPointer(BPI_CROSSHAIR_POINTER);
#endif/*__AMIGAOS__/__MSDOS__*/

	if (FSelectedPblk(GlobalBlockCopy)) {
#if __AMIGAOS__
		if (	(GlobalBlockCopy->SourceRoom
			== GlobalRoomWindow->CurrentRoom->Room) &&
			(GlobalBlockCopy->SourceStuff	==
			GlobalRoomWindow->CurrentRoom) ) {

			/* Erase selection box around old brush source on same page. */

			XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);
		}

		HideBrush();
#elif __MSDOS__
		/* Erase old brush that user was dragging around. */

		HideBrush();

		if (	(GlobalBlockCopy->SourceRoom
			== GlobalRoomWindow->CurrentRoom->Room) &&
			(GlobalBlockCopy->SourceStuff	==
			GlobalRoomWindow->CurrentRoom) ) {

			if (fShowTileUsage)
			{
				/* Erase tile usage numbers in source selection box */

				CopyDisplayTilesToBack(GlobalBlockCopy->SourceRW,
						GlobalBlockCopy->SourceX, GlobalBlockCopy->SourceY,
						((LayerType *) Head(&GlobalBlockCopy->Layers))->ctilx,
						((LayerType *) Head(&GlobalBlockCopy->Layers))->ctily,
						BACKTODISPLAY);
			}
			/* Erase selection box around old brush source on same page. */

			XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);

			/* Update back buffer. */

			CopyDisplayTilesToBack(GlobalBlockCopy->SourceRW,
						GlobalBlockCopy->SourceX, GlobalBlockCopy->SourceY,
						((LayerType *) Head(&GlobalBlockCopy->Layers))->ctilx,
						((LayerType *) Head(&GlobalBlockCopy->Layers))->ctily,
						DISPLAYTOBACK);
			if (fShowTileUsage)
			{
				/* Refresh tile usage numbers in source selection box */

				Assert(GlobalBlockCopy->BC_FloorLayer->cxTile != 0);
				Assert(GlobalBlockCopy->BC_FloorLayer->cyTile != 0);
				ShowCountsInSubRect(GlobalBlockCopy->SourceRoom,
							GlobalBlockCopy->SourceRW,
							0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
							GlobalBlockCopy->SourceX -
								(unsigned) (GlobalBlockCopy->SourceStuff->dxSrc / GlobalBlockCopy->BC_FloorLayer->cxTile),
							GlobalBlockCopy->SourceY -
								(unsigned) (GlobalBlockCopy->SourceStuff->dySrc / GlobalBlockCopy->BC_FloorLayer->cyTile),
							GlobalBlockCopy->SourceX,
							GlobalBlockCopy->SourceY,
							GlobalBlockCopy->BC_FloorLayer->ctilx,
							GlobalBlockCopy->BC_FloorLayer->ctily);
			}
		}

#endif/*__AMIGAOS__/__MSDOS__*/

		GlobalBlockCopy->SourceX		= 0;
		GlobalBlockCopy->SourceY		= 0;
		DeAllocateLayers (&(GlobalBlockCopy->Layers));
		GlobalBlockCopy->BC_FloorLayer= NULL;
		GlobalBlockCopy->LayerCount	= 0;
		GlobalBlockCopy->FirstTileSet	= NULL;
		if (GlobalBlockCopy->pSpecialTilesets != NULL)
		{
			DeleteList(GlobalBlockCopy->pSpecialTilesets);
			GlobalBlockCopy->pSpecialTilesets = NULL;
		}
		ResetHighlightTile();
	}
	SelectMode = OFF;

	MButtonsE ();
	return;

} /* Unselect */


/**************************************************************************
 *
 * Unselect2
 *
 * SYNOPSIS
 *		void Unselect2 (void)
 *
 * PURPOSE
 *	Unselect the any selected tile(s) + get ready for 1 layer copy.
 *
 * USAGE
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
void Unselect2 (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Unselect2";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Unselect ();
	OneLayerCopy = TRUE;
	return;

} /* Unselect2 */

#if !PIXELSELECT
/* ctilxGlobalSelect & ctilyGlobalSelect are local to
	StartSelect(), Selecting(), and FinishSelect() */

static int ctilxGlobalSelect;
static int ctilyGlobalSelect;
#endif // !PIXELSELECT


/**************************************************************************
 *
 * StartSelect
 *
 * PURPOSE
 *		Start selecting a tile or group of tiles.
 *
 * INPUT
 *
 *
 * EFFECTS
 *		The following variables are set on exit from this routine:
 *
 *		GlobalBlockCopy->SourceX	: left edge of selection
 *		GlobalBlockCopy->SourceY	: top edge of selection
#if PIXELSELECT
 *		GlobalBlockCopy->ctilxSelect : width of selection in tiles
 *		GlobalBlockCopy->ctilySelect : height of selection in tiles
#endif // PIXELSELECT
#if !PIXELSELECT
 *		ctilxGlobalSelect		: width of selection in tiles
 *		ctilyGlobalSelect		: height of selection in tiles
#endif // !PIXELSELECT
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		07/07/94 (dcc) - Add support for fPixelSelect.
 *		08/21/94 (dcc) - Add support for UseSparseArray.
 *		03/26/95 (dcc) - Make sure FindNonEmptyLayer() returns != NULL.
 *		04/02/95 (dcc) - KLUDGE: check to make sure floor layer has tiles
 *					  it it as much code assumes floor layer of brush
 *					  has tiles with non-zero width & height.
 *
 * SEE ALSO
 *
*/
static void StartSelect (void)
{
	BOOL			 fFollow;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "StartSelect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	OldColumn	= -1;
	OldRow	= -1;

#if PIXELSELECT
	if (fPixelSelect)
	{
		fFollow = FollowPixelCursor (&AnchorX, &AnchorY, &ScrollDX, &ScrollDY, NULL, FALSE);
	}
	else
	{
		fFollow = FollowCursor (&AnchorX, &AnchorY, &ScrollDX, &ScrollDY, NULL, FALSE);
	}

	if (fFollow)
#endif // PIXELSELECT
#if !PIXELSELECT
	if (FollowCursor (&AnchorX, &AnchorY, &ScrollDX, &ScrollDY, NULL, FALSE))
#endif // !PIXELSELECT
	{
		RoomType		*prm;
		LayerType	*play;

		prm = GlobalRoomWindow->CurrentRoom->Room;

		//04/02/95 KLUDGE: make sure floor layer has tiles in it!

		play = prm->FloorLayer;
		if (play && play->cxTile != 0 && play->cyTile != 0)
//04/02/95 KLUDGE		play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
//04/02/95 KLUDGE		if (play)
		{
#if PIXELSELECT
			GlobalBlockCopy->dtilxOffset	= 0;
			GlobalBlockCopy->dtilyOffset	= 0;

			GlobalBlockCopy->ctilxSelect	= 1;
			GlobalBlockCopy->ctilySelect	= 1;
#endif // PIXELSELECT
#if !PIXELSELECT
			ctilxGlobalSelect	= 1;
			ctilyGlobalSelect	= 1;
#endif // !PIXELSELECT

			SnapToGrid(prm, &AnchorX, &AnchorY);

			if (IsGridOn(prm))
			{
#if PIXELSELECT
				GlobalBlockCopy->ctilxSelect	= prm->wGridXWidth;
				GlobalBlockCopy->ctilySelect	= prm->wGridYHeight;
#endif // PIXELSELECT
#if !PIXELSELECT
				ctilxGlobalSelect	= prm->wGridXWidth;
				ctilyGlobalSelect	= prm->wGridYHeight;
#endif // !PIXELSELECT
			}
			GlobalBlockCopy->SourceX	= AnchorX;
			GlobalBlockCopy->SourceY	= AnchorY;
#if PIXELSELECT
			dxModTileAnchor = dxModTile;
			dyModTileAnchor = dyModTile;

			GlobalBlockCopy->dxModSrcL	= dxModTileAnchor;
			GlobalBlockCopy->dyModSrcT	= dyModTileAnchor;
			GlobalBlockCopy->dxModSrcR	= dxModTileAnchor;
			GlobalBlockCopy->dyModSrcB	= dyModTileAnchor;
#endif // PIXELSELECT

			GlobalBlockCopy->SourceRW	= GlobalRoomWindow;
			GlobalBlockCopy->SourceStuff	= GlobalRoomWindow->CurrentRoom;
			GlobalBlockCopy->SourceRoom	= prm;
#if PIXELSELECT
			GlobalBlockCopy->playSrcFloor	= prm->FloorLayer;
#endif // PIXELSELECT

			if (CANSHOW (LastTBar) && prm && (prm->Flags & SOURCEROOM))
			{
				LayerType	*layer;

				layer = Head(&prm->Layers);
				if (! IsEOList (layer))
				{
#if PLOTARRAY
					if (IsComposite(prm) && IsLocked(prm))
					{
						GlobalBlockCopy->FirstTileSet = prm->ptsComposite;
					}
					else
					{
						PlotType	*plot;

						plot		= layer->Plot;
						plot		+= (AnchorX + (AnchorY * layer->ctilx));

						if (plot->TileSet_ID)
						{
							GlobalBlockCopy->FirstTileSet = TILESET_AT_PLOT (plot);
						}
					}
#else
					if (IsComposite(prm) && IsLocked(prm))
					{
						GlobalBlockCopy->FirstTileSet = prm->ptsComposite;
					}
					else
					{
#if UseSparseArray
						PlotType	plt;

						LAY_ReadPlotXY(layer, AnchorX, AnchorY, &plt);

						if (plt.TileSet_ID)
							GlobalBlockCopy->FirstTileSet = TILESET_AT_PLOT (&plt);
#else // !UseSparseArray
						PlotType	*plot;

						plot = ActivatePlotXYatWin(layer->rgrgplt, AnchorX, AnchorY, winDst);

						if (plot->TileSet_ID)
						{
							GlobalBlockCopy->FirstTileSet = TILESET_AT_PLOT (plot);
						}
						ReleasePlotRow(layer->rgrgplt, AnchorY);
#endif // !UseSparseArray
					}
#endif
				}
			}

#if PIXELSELECT
			if (fPixelSelect)
			{
				DrawPixelSelect (GlobalBlockCopy,
							play->cxTile, play->cyTile,
							SOURCE_EOR, FALSE);
			}
			else
			{
				DrawSelect (GlobalBlockCopy,
							GlobalBlockCopy->ctilxSelect,
							GlobalBlockCopy->ctilySelect,
							play->cxTile, play->cyTile,
							SOURCE_EOR, FALSE);
			}
#endif // PIXELSELECT
#if !PIXELSELECT
			DrawSelect (GlobalBlockCopy,
						ctilxGlobalSelect, ctilyGlobalSelect,
						play->cxTile, play->cyTile,
						SOURCE_EOR, FALSE);
#endif // !PIXELSELECT
			if (ShowCursor && ShowTitleSTATE)
			{
				sprintf (X_Pos, "%05d", 1);
				sprintf (Y_Pos, "%05d", 1);
#if __AMIGAOS__
				Move (&MainScreen->RastPort, XX, TextTop);
				Text (&MainScreen->RastPort, X_Pos, 5);

				Move (&MainScreen->RastPort, YX, TextTop);
				Text (&MainScreen->RastPort, Y_Pos, 5);
#elif __MSDOS__
				ShowCoordinates();
#endif/*__AMIGAOS__/__MSDOS__*/
			}
		}
	}
	return;

} /* StartSelect */


/**************************************************************************
 *
 * Selecting
 *
 * PURPOSE
 *		Still in the process of selecting.
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *		The following variables are set on exit from this routine:
 *
 *		GlobalBlockCopy->SourceX	: left edge of selection
 *		GlobalBlockCopy->SourceY	: top edge of selection
#if PIXELSELECT
 *		GlobalBlockCopy->ctilxSelect : width of selection in tiles
 *		GlobalBlockCopy->ctilySelect : height of selection in tiles
#endif // PIXELSELECT
#if !PIXELSELECT
 *		ctilxGlobalSelect		: width of selection in tiles
 *		ctilyGlobalSelect		: height of selection in tiles
#endif // !PIXELSELECT
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		07/07/94 (dcc) - Add support for fPixelSelect.
 *		03/26/95 (dcc) - Make sure FindNonEmptyLayer() returns != NULL.
 *		04/02/95 (dcc) - KLUDGE: check to make sure floor layer has tiles
 *					  it it as much code assumes floor layer of brush
 *					  has tiles with non-zero width & height.
 *
 * SEE ALSO
 *
*/
#if __AMIGAOS__
static void Selecting (void)
#elif __MSDOS__
static short Selecting(HitInfo *hi)
#endif/*__AMIGAOS__/__MSDOS__*/
{
	WORD	x;
	WORD	y;
	WORD	width;
	WORD	height;
	WORD	wXGridSize = 1;
	WORD	wYGridSize = 1;
	BOOL	fFollow;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Selecting";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	hi = hi;	/* Turn off warning */

#if PIXELSELECT
	if (fPixelSelect)
	{
		fFollow = FollowPixelCursor (&CurrentX, &CurrentY, &ScrollDX, &ScrollDY, NULL, TRUE);
	}
	else
	{
		fFollow = FollowCursor (&CurrentX, &CurrentY, &ScrollDX, &ScrollDY, NULL, TRUE);
	}

	if (fFollow)
#endif // PIXELSELECT
#if !PIXELSELECT
	if (FollowCursor (&CurrentX, &CurrentY, &ScrollDX, &ScrollDY, NULL, TRUE))
#endif // !PIXELSELECT
	{
		RoomType		*prm;
		LayerType	*play;

		prm = GlobalRoomWindow->CurrentRoom->Room;

		//04/02/95 KLUDGE: make sure floor layer has tiles in it!

		play = prm->FloorLayer;
		if (play && play->cxTile != 0 && play->cyTile != 0)
//04/02/95 KLUDGE		play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
//04/02/95 KLUDGE		if (play)
		{
#if PIXELSELECT
			if (fPixelSelect)
			{
				DrawPixelSelect (GlobalBlockCopy,
							play->cxTile, play->cyTile,
							SOURCE_EOR, FALSE);
			}
			else
			{
				DrawSelect (GlobalBlockCopy,
							GlobalBlockCopy->ctilxSelect,
							GlobalBlockCopy->ctilySelect,
							play->cxTile, play->cyTile,
							SOURCE_EOR, FALSE);
			}
#endif // PIXELSELECT
#if !PIXELSELECT
			DrawSelect (GlobalBlockCopy,
						ctilxGlobalSelect, ctilyGlobalSelect,
						play->cxTile, play->cyTile,
						SOURCE_EOR, FALSE);
#endif // !PIXELSELECT

			Scroll (GlobalRoomWindow, ScrollDX, ScrollDY, FALSE);

			SnapToGrid(prm, &CurrentX, &CurrentY);

			if (IsGridOn(prm))
			{
				wXGridSize = prm->wGridXWidth;
				wYGridSize = prm->wGridYHeight;
			}
#if PIXELSELECT
			dxModTileCurrent = dxModTile;
			dyModTileCurrent = dyModTile;
#endif // PIXELSELECT

			if (AnchorX > CurrentX) {
				x		= CurrentX;
				width	= wXGridSize + AnchorX - CurrentX;
#if PIXELSELECT
				GlobalBlockCopy->dxModSrcL = dxModTileCurrent;
				GlobalBlockCopy->dxModSrcR = dxModTileAnchor;
#endif // PIXELSELECT
			}
			else {
				x		= AnchorX;
				width	= wXGridSize + CurrentX - AnchorX;
#if PIXELSELECT
				GlobalBlockCopy->dxModSrcL = dxModTileAnchor;
				GlobalBlockCopy->dxModSrcR = dxModTileCurrent;
				if (AnchorX == CurrentX && dxModTileAnchor > dxModTileCurrent)
				{
					GlobalBlockCopy->dxModSrcL = dxModTileCurrent;
					GlobalBlockCopy->dxModSrcR = dxModTileAnchor;
				}
#endif // PIXELSELECT
			}

			if (AnchorY > CurrentY) {
				y		= CurrentY;
				height	= wYGridSize + AnchorY - CurrentY;
#if PIXELSELECT
				GlobalBlockCopy->dyModSrcT = dyModTileCurrent;
				GlobalBlockCopy->dyModSrcB = dyModTileAnchor;
#endif // PIXELSELECT
			}
			else {
				y		= AnchorY;
				height	= wYGridSize + CurrentY - AnchorY;
#if PIXELSELECT
				GlobalBlockCopy->dyModSrcT = dyModTileAnchor;
				GlobalBlockCopy->dyModSrcB = dyModTileCurrent;
				if (AnchorY == CurrentY && dyModTileAnchor > dyModTileCurrent)
				{
					GlobalBlockCopy->dyModSrcT = dyModTileCurrent;
					GlobalBlockCopy->dyModSrcB = dyModTileAnchor;
				}
#endif // PIXELSELECT
			}

			GlobalBlockCopy->SourceX	= x;
			GlobalBlockCopy->SourceY	= y;

			/* Set DestX & DestY as well so that call to ShowGuide() in
		   	RedrawSelectArea() works properly. */

			GlobalBlockCopy->DestX	= x;
			GlobalBlockCopy->DestY	= y;

#if PIXELSELECT
			GlobalBlockCopy->ctilxSelect		= width;
			GlobalBlockCopy->ctilySelect		= height;
#endif // PIXELSELECT
#if !PIXELSELECT
			ctilxGlobalSelect		= width;
			ctilyGlobalSelect		= height;
#endif // !PIXELSELECT

#if PIXELSELECT
			if (fPixelSelect)
			{
				DrawPixelSelect (GlobalBlockCopy,
							play->cxTile, play->cyTile,
							SOURCE_EOR, FALSE);
			}
			else
			{
				DrawSelect (GlobalBlockCopy,
							GlobalBlockCopy->ctilxSelect,
							GlobalBlockCopy->ctilySelect,
							play->cxTile, play->cyTile,
							SOURCE_EOR, FALSE);
			}
#endif // PIXELSELECT
#if !PIXELSELECT
			DrawSelect (GlobalBlockCopy,
						ctilxGlobalSelect, ctilyGlobalSelect,
						play->cxTile, play->cyTile,
						SOURCE_EOR, FALSE);
#endif // !PIXELSELECT

			if (ShowCursor && ShowTitleSTATE) {
				sprintf (X_Pos, "%05d", width);
				sprintf (Y_Pos, "%05d", height);
#if __AMIGAOS__
				Move (&MainScreen->RastPort, XX, TextTop);
				Text (&MainScreen->RastPort, X_Pos, 5);

				Move (&MainScreen->RastPort, YX, TextTop);
				Text (&MainScreen->RastPort, Y_Pos, 5);
#elif __MSDOS__
				ShowCoordinates();
#endif/*__AMIGAOS__/__MSDOS__*/
			}
		}
	}
#if __AMIGAOS__
	return;
#elif __MSDOS__
	return TRUE;
#endif/*__AMIGAOS__/__MSDOS__*/
} /* Selecting */


/*********************************************************************
 *
 * FAlreadySpecial
 *
 * PURPOSE
 *		See if <TileID> is has already been added to list <pstl>.
 *
 * INPUT
 *		pstl		: list of SpecialTileset's to check
 *		TileID	: tileset ID to match.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		TRUE if appears in list.
 *
 * HISTORY
 *		12/16/93 Thursday (dcc) - created.
 *
*/
static BOOL FAlreadySpecial(ListType *pstl, int TileID)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FAlreadySpecial";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pstl)
	{
		SpecialTileset *psts = Head(pstl);
		while (!IsEOList(psts))
		{
			if (psts->TileID == TileID)
				return TRUE;
			psts = Next(psts);
		}
	}
	return FALSE;
} /* FAlreadySpecial */


/*********************************************************************
 *
 * AddSpecial
 *
 * PURPOSE
 *		Create a SpecialTileset node with information (<TileID>,
 *		<play>, <dtilx>, and <dtily>) passed to this function, and
 *		add it the the <ppstl> list.
 *
 * INPUT
 *		dtilx	: x-tile-offset in GlobalBlockCopy
 *		dtily	: y-tile-offset in GlobalBlockCopy
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		FALSE if OOM.
 *
 * HISTORY
 *		12/16/93 Thursday (dcc) - created.
 *
*/
static BOOL AddSpecial(ListType **ppstl, int TileID, LayerType *play,
					int dtilx, int dtily)
{
	SpecialTileset *psts;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddSpecial";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!*ppstl)
	{
		if ((*ppstl = CreateList(NULL)) == NULL)
			return FALSE;
	}
	if ((psts = CreateNode(sizeof(SpecialTileset), NULL)) == NULL)
		return FALSE;

	psts->TileID = TileID;
	psts->play   = play;
	psts->dtilx  = dtilx;
	psts->dtily  = dtily;
	AddTail(*ppstl, psts);
	return TRUE;

} /* AddSpecial */


/*********************************************************************
 *
 * MakeSpecialTiles
 *
 * PURPOSE
 *		Fill out pSpecialTilesets list for <pbc>. This list is used
 *		to further limit tile placement in a room.
 *
 * INPUT
 *		pbc		: tile-brush to fill out
 *
 * ASSUMES
 *		"Special" tiles in the brush are already in the proper alignment
 *		with respect to other "special" tiles in the brush. This means
 *		that once a single tile of a tileset has been added to this list,
 *		no other tiles of that type need to be added.
 *
 * RETURN VALUE
 *		FALSE if OOM. pSpecialTilesets set NULL if no "special" tiles in <pbc>.
 *
 * HISTORY
 *		12/16/93 (dcc) - created.
 *		02/19/94 (dcc) - fix offsets so snap to special works right
 *		08/21/94 (dcc) - add support for UseSparseArray
 *		08/24/94 (dcc) - don't figure for 1x1 pixel tile objects FIXME BUGBUG KLUDGE
 *
*/
static BOOL MakeSpecialTiles(BlockCopyType *pbc)
{
	LayerType *playbc = pbc->BC_FloorLayer;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MakeSpecialTiles";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pbc->pSpecialTilesets = NULL;
	if (!playbc)
		return TRUE;

	while (!IsEOList(playbc))
	{
		int ctilx = playbc->ctilx;
		int ctily = playbc->ctily;
		int x, y;

#if PixelLayers
		if (playbc->cxTile != 1 || playbc->cyTile != 1)	//KLUDGE FIXME BUGBUG
		{
#endif // !PixelLayers
		for (y = 0; y < ctily; y++)
		{
#if UseSparseArray
			PlotType *ppltsrc;

			if (!IsSparse(playbc))
				ppltsrc = ActivatePlotRowatWin(playbc->p.rgrgplt, y, winSrc);

#else // !UseSparseArray
			PlotType *ppltsrc = ActivatePlotRowatWin(playbc->rgrgplt, y, winSrc);

#endif // !UseSparseArray
			for (x = 0; x < ctilx; x++)
			{
#if UseSparseArray
				if (IsSparse(playbc))
					ppltsrc = GetSparsePlotXY(playbc->p.pspa, x, y);

#endif // UseSparseArray
				if (	!fNullTile(ppltsrc) &&
					!FAlreadySpecial(pbc->pSpecialTilesets, ppltsrc->TileSet_ID))
				{
					TileSetType *ptst = FAST_TILESET_PTR(ppltsrc->TileSet_ID);

					if (	ptst->ctilxSkip  != ctilxSkipDefault  ||
						ptst->ctilxDispO != ctilxDispODefault ||
						ptst->ctilyDispO != ctilyDispODefault ||
						ptst->ctilxDispE != ctilxDispEDefault ||
						ptst->ctilyDispE != ctilyDispEDefault )
					{
						if (!AddSpecial(&pbc->pSpecialTilesets,
									ppltsrc->TileSet_ID,
									playbc, x+pbc->dtilxOffset, y+pbc->dtilyOffset))
							return FALSE;
					}
				}
				ppltsrc++;
			}
#if UseSparseArray
			if (!IsSparse(playbc))
				ReleasePlotRow(playbc->p.rgrgplt, y);
#else // !UseSparseArray
			ReleasePlotRow(playbc->rgrgplt, y);
#endif // !UseSparseArray
		}
#if PixelLayers
		}				//KLUDGE FIXME BUGBUG
#endif // !PixelLayers
		playbc = Next(playbc);
	}

	return TRUE;
} /* MakeSpecialTiles */


/*********************************************************************
 *
 * ComputeBrushSize
 *
 * PURPOSE
 *		Since there are now tiles that are wider than the grid they
 *		fit into (cxDisplay >= Width, cyDisplay >= Height), figure
 *		how many extra tiles off to the right and bottom a brush
 *		occupies.
 *
 * INPUT
 *		pbc		: tile-brush to compute size
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		12/17/93 (dcc) - created.
 *		08/21/94 (dcc) - add support for UseSparseArray
 *		09/08/94 (dcc) - add support for PixelLayers
 *
*/
static void ComputeBrushSize(BlockCopyType *pbc)
{
	LayerType *playbc = pbc->BC_FloorLayer;
	int ctilxDisplay = 0;
	int ctilyDisplay = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ComputeBrushSize";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!playbc)
		return;

	while (!IsEOList(playbc))
	{
		int ctilx = playbc->ctilx;
		int ctily = playbc->ctily;
		int x, y;

#if PixelLayers
		if (playbc->cxTile == 1 && playbc->cyTile == 1)
		{
			ctilxDisplay = max(ctilxDisplay, ctilx);
			ctilyDisplay = max(ctilyDisplay, ctily);
		}
		else // if (IsSparse(playbc))
#endif // !PixelLayers

#if UseSparseArray
		if (IsSparse(playbc))
		{
			PlotType *pplt;
			int ixt, iyt;

			SetSparseLayerLimits(playbc->p.pspa, 0, 0, 0, 0);

			while ((pplt = GetNextSparsePlotInRange(playbc->p.pspa,
											&ixt, &iyt)) != NULL)
			{
				TileSetType *ptst = FAST_TILESET_PTR(pplt->TileSet_ID);

				ctilxDisplay = max(ctilxDisplay,
							ixt+(ptst->Width +ptst->cxDisplay-1)/ptst->cxDisplay);
				ctilyDisplay = max(ctilyDisplay,
							iyt+(ptst->Height+ptst->cyDisplay-1)/ptst->cyDisplay);
			}
		}
		else
		{
			for (y = 0; y < ctily; y++)
			{
				PlotType *ppltsrc = ActivatePlotRowatWin(playbc->p.rgrgplt, y, winSrc);

				for (x = 0; x < ctilx; x++)
				{
					if (!fNullTile(ppltsrc))
					{
						TileSetType *ptst = FAST_TILESET_PTR(ppltsrc->TileSet_ID);

						ctilxDisplay = max(ctilxDisplay,
									x+(ptst->Width +ptst->cxDisplay-1)/ptst->cxDisplay);
						ctilyDisplay = max(ctilyDisplay,
									y+(ptst->Height+ptst->cyDisplay-1)/ptst->cyDisplay);
					}
					ppltsrc++;
				}
				ReleasePlotRow(playbc->p.rgrgplt, y);
			}
		}
#else // !UseSparseArray
		for (y = 0; y < ctily; y++)
		{
			PlotType *ppltsrc = ActivatePlotRowatWin(playbc->rgrgplt, y, winSrc);

			for (x = 0; x < ctilx; x++)
			{
				if (!fNullTile(ppltsrc))
				{
					TileSetType *ptst = FAST_TILESET_PTR(ppltsrc->TileSet_ID);

					ctilxDisplay = max(ctilxDisplay,
								x+(ptst->Width +ptst->cxDisplay-1)/ptst->cxDisplay);
					ctilyDisplay = max(ctilyDisplay,
								y+(ptst->Height+ptst->cyDisplay-1)/ptst->cyDisplay);
				}
				ppltsrc++;
			}
			ReleasePlotRow(playbc->rgrgplt, y);
		}
#endif // !UseSparseArray

		playbc = Next(playbc);
	}
	pbc->ctilxDisplay = ctilxDisplay;
	pbc->ctilyDisplay = ctilyDisplay;
} /* ComputeBrushSize */


/*********************************************************************
 *
 * SetCompositeTileRect
 *
 * PURPOSE
 *		Set a rectangular array of <ctilx>, <ctily> tiles to
 *		<dtilxDst>, <dtilyDst> in the destination layer as if
 *		copied <dtilxSrc>, <dtilySrc> in the source layer <playSrc>.
 *
 * INPUT
 *		tilesetID	: tileset ID of composite room
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		
 *
 * HISTORY
 *		01/14/94 (dcc) - created.
 *		02/20/94 (dcc) - add support for play->dtilx, ->dtily
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
*/
void SetCompositeTileRect(LayerType *playSrc, int dtilxSrc, int dtilySrc,
				LayerType *playDst, int dtilxDst, int dtilyDst,
				int ctilx, int ctily, int tilesetID)
{

#if !UseSparseArray
	PlotType	*ppltDst;
#endif // !UseSparseArray
	int iy, ix;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetCompositeTileRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	dtilxSrc += playSrc->dtilx;
	if (dtilxSrc + ctilx > playSrc->ctilx)
		ctilx = playSrc->ctilx - dtilxSrc;

	dtilySrc += playSrc->dtily;
	if (dtilySrc + ctily > playSrc->ctily)
		ctily = playSrc->ctily - dtilySrc;

#if PLOTARRAY && !UseSparseArray
	ppltDst	= playDst->Plot + (dtilyDst * playDst->ctilx) + dtilxDst;
#endif

	for (iy = 0; iy < ctily; iy++)
	{
#if UseSparseArray
		for (ix = 0; ix < ctilx; ix++)
		{
			PlotType plt;

			if ((dtilxSrc + ix) < playSrc->ctilx &&
				(dtilySrc + iy) < playSrc->ctily)
			{
				plt.TileSet_ID	= tilesetID;
				plt.Tile_ID	= (dtilySrc + iy) * playSrc->ctilx +
								(dtilxSrc + ix) + 1;
			}
			else
			{
				plt.TileSet_ID = 0;
				plt.Tile_ID	= 0;
			}
			plt.Plot_Flags = 0;
			LAY_WritePlotXY(&plt, playDst, dtilxDst+ix, dtilyDst+iy);
		}
#else // !UseSparseArray
#if PLOTARRAY
#else
		ppltDst = ActivatePlotXYatWin(playDst->rgrgplt,
								dtilxDst, dtilyDst+iy, winDst);
#endif
		for (ix = 0; ix < ctilx; ix++)
		{
			if ((dtilxSrc + ix) < playSrc->ctilx &&
				(dtilySrc + iy) < playSrc->ctily)
			{
				ppltDst->TileSet_ID = tilesetID;
				ppltDst->Tile_ID = (dtilySrc + iy) * playSrc->ctilx +
								(dtilxSrc + ix) + 1;
			}
			else
			{
				ppltDst->TileSet_ID = 0;
				ppltDst->Tile_ID = 0;
			}
			ppltDst->Plot_Flags = 0;
			ppltDst++;
		}
#if PLOTARRAY
#else
		UpdatePlotRow(playDst->rgrgplt, iy+dtilyDst);
#endif
#endif // !UseSparseArray
	}
} /* SetCompositeTileRect */


/*********************************************************************
 *
 * CopyTileRect
 *
 * PURPOSE
 *		Copy a rectangular array of <ctilx>, <ctily> tiles from
 *		<dtilxSrc>, <dtilySrc> in the source layer <playSrc> to
 *		<dtilxDst>, <dtilyDst> in the destination layer.
 *
 * INPUT
 *		fSelect	: if == CUT, set tiles in SOURCE to NULL TILES
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		
 *
 * HISTORY
 *		01/14/94 (dcc) - created.
 *		02/20/94 (dcc) - add support for play->dtilx, ->dtily
 *		02/20/94 (dcc) - add check for NULL tile b4 ->ubShowFlagAND
 *		08/21/94 (dcc) - add support for UseSparseArray
 *		10/20/94 (dcc) - Updated to use GetNextSparsePlotInRange().
 *
*/
static void CopyTileRect(LayerType *playSrc, int dtilxSrc, int dtilySrc,
				LayerType *playDst, int dtilxDst, int dtilyDst,
				int ctilx, int ctily, int fSelect)
{

	PlotType	*ppltDst;
	PlotType	*ppltSrc;
#if PLOTARRAY
	PlotType	*startrow;
#endif
	int iy, ix;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CopyTileRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (dtilxSrc + playSrc->dtilx + ctilx > playSrc->ctilx)
		ctilx = playSrc->ctilx - (dtilxSrc + playSrc->dtilx);

	if (dtilySrc + playSrc->dtily + ctily > playSrc->ctily)
		ctily = playSrc->ctily - (dtilySrc + playSrc->dtily);

#if PLOTARRAY
	ppltDst	= playDst->Plot + (dtilyDst * playDst->ctilx) + dtilxDst;
	startrow	= playSrc->Plot + (dtilySrc * playSrc->ctilx) + dtilxSrc+playSrc->dtilx;
	ppltSrc	= startrow;
#endif

#if	   UseSparseArray
	if (IsSparse(playSrc) && IsSparse(playDst))
	{
		PlotType *pplt;
		int ixt, iyt;
		int dxt = dtilxSrc+playSrc->dtilx-dtilxDst;
		int dyt = dtilySrc+playSrc->dtily-dtilyDst;

		ZeroRectInSparseArray(playDst->p.pspa,
							dtilxDst, dtilyDst,
							ctilx, ctily);

		SetSparseLayerLimits(playSrc->p.pspa,
						 dtilxSrc+playSrc->dtilx,
						 dtilySrc+playSrc->dtily,
						 ctilx, ctily);

		while ((pplt = GetNextSparsePlotInRange(playSrc->p.pspa,
										&ixt, &iyt)) != NULL)
		{
#if PixelLayers
			// KLUDGE BUGBUG FIXME don't mung flag bits if tile object!
			if (playSrc->cxTile != 1 || playSrc->cyTile != 1)
#endif // !PixelLayers
				if (pplt->TileSet_ID)
					pplt->Plot_Flags &=
						FAST_TILESET_PTR(pplt->TileSet_ID)->ubShowFlagAND;

			WriteSparsePlotXY(pplt, playDst->p.pspa,
					ixt-dxt, iyt-dyt);

			if (fSelect == CUT && !(playSrc->L_Flags & LAYER_LOCKED))
			{
				PlotType plt;

				plt.Plot_Flags	= 0;
				plt.TileSet_ID	= 0;
				plt.Tile_ID	= 0;
				WriteSparsePlotXY(&plt, playSrc->p.pspa, ixt, iyt);
			}
		}
	}
	else
	{
#endif	/* UseSparseArray */
		for (iy = 0; iy < ctily; iy++)
		{
#if PLOTARRAY
#else
#if UseSparseArray
			if (!IsSparse(playDst))
				ppltDst = ActivatePlotXYatWin(playDst->p.rgrgplt,
										dtilxDst, dtilyDst+iy, winDst);
			if (!IsSparse(playSrc))
				ppltSrc = ActivatePlotXYatWin(playSrc->p.rgrgplt,
										dtilxSrc+playSrc->dtilx,
										dtilySrc+playSrc->dtily+iy, winSrc);
#else // !UseSparseArray
			ppltDst = ActivatePlotXYatWin(playDst->rgrgplt,
									dtilxDst, dtilyDst+iy, winDst);
			ppltSrc = ActivatePlotXYatWin(playSrc->rgrgplt,
									dtilxSrc+playSrc->dtilx,
									dtilySrc+playSrc->dtily+iy, winSrc);
#endif // !UseSparseArray
#endif
			for (ix = 0; ix < ctilx; ix++)
			{
#if UseSparseArray
				PlotType plt;

				if (IsSparse(playSrc))
				{
					ReadSparsePlotXY(playSrc->p.pspa,
									dtilxSrc+playSrc->dtilx+ix,
									dtilySrc+playSrc->dtily+iy,
									&plt);
				}
				else
				{
					plt = *ppltSrc;
				}
#if PixelLayers
				// KLUDGE BUGBUG FIXME don't mung flag bits if tile object!
				if (playSrc->cxTile != 1 || playSrc->cyTile != 1)
#endif // !PixelLayers
					if (plt.TileSet_ID)
						plt.Plot_Flags &=
							FAST_TILESET_PTR(plt.TileSet_ID)->ubShowFlagAND;

				if (IsSparse(playDst))
					WriteSparsePlotXY(&plt, playDst->p.pspa,
									dtilxDst+ix, dtilyDst+iy);
				else
					*ppltDst = plt;

				if (fSelect == CUT && !(playSrc->L_Flags & LAYER_LOCKED))
				{
					if (IsSparse(playSrc))
					{
						plt.Plot_Flags	= 0;
						plt.TileSet_ID	= 0;
						plt.Tile_ID	= 0;
						WriteSparsePlotXY(&plt, playSrc->p.pspa,
										dtilxSrc+playSrc->dtilx+ix,
										dtilySrc+playSrc->dtily+iy);
					}
					else
					{
						ppltSrc->Plot_Flags	= 0;
						ppltSrc->TileSet_ID	= 0;
						ppltSrc->Tile_ID	= 0;
					}
				}
#else // !UseSparseArray
				/**************************************/
				/* structure assignment for the plots */

				*ppltDst = *ppltSrc;

				if (ppltDst->TileSet_ID)
					ppltDst->Plot_Flags &=
						FAST_TILESET_PTR(ppltDst->TileSet_ID)->ubShowFlagAND;

				if (fSelect == CUT &&
					!(playSrc->L_Flags & LAYER_LOCKED))
				{
					ppltSrc->Plot_Flags	= 0;
					ppltSrc->TileSet_ID	= 0;
					ppltSrc->Tile_ID	= 0;
				}
#endif // !UseSparseArray
				ppltDst ++;
				ppltSrc ++;
			}
#if PLOTARRAY
			startrow	+= playSrc->ctilx;
			ppltSrc	= startrow;
#else
#if UseSparseArray
			if (!IsSparse(playDst))
				UpdatePlotRow(playDst->p.rgrgplt, iy+dtilyDst);

			if (!IsSparse(playSrc))
			{
				if (fSelect == CUT && !(playSrc->L_Flags & LAYER_LOCKED))
					UpdatePlotRow(playSrc->p.rgrgplt, iy+dtilySrc);
				else
					ReleasePlotRow(playSrc->p.rgrgplt, iy+dtilySrc);
			}
#else // !UseSparseArray
			UpdatePlotRow(playDst->rgrgplt, iy+dtilyDst);

			if (fSelect == CUT && !(playSrc->L_Flags & LAYER_LOCKED))
				UpdatePlotRow(playSrc->rgrgplt, iy+dtilySrc);
			else
				ReleasePlotRow(playSrc->rgrgplt, iy+dtilySrc);
#endif // !UseSparseArray
#endif
		}
#if	   UseSparseArray
	}
#endif	/* UseSparseArray */
} /* CopyTileRect */


/*********************************************************************
 *
 * FTileIntersectsSelection
 *
 * PURPOSE
 *		See if a tile of <pplt> at <dtilx>, <dtily> intersects
 *		a selection rectangle with tile coordinates of
 *		<dtilxSelL>, <dtilySelT>, <dtilxSelL>+<ctilxSel>, &
 *		<dtilySelB>+<ctilySel> and offsets within the tile of
 *		<dxModSrcL>, <dyModSrcT>, <dxModSrcR>, & <dyModSrcB>.
 *
 *		<cxTileSpacing> & <cyTileSpacing> specify how big (in pixels)
 *		are the tiles in the grid.
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
 *		01/14/94 Friday (dcc) - created.
 *
*/
static BOOL FTileIntersectsSelection(	PlotType *pplt, int dtilx, int dtily,
								int dtilxSelL, int dtilySelT,
								int ctilxSel, int ctilySel,
								int dxModSrcL, int dyModSrcT,
								int dxModSrcR, int dyModSrcB,
								int cxTileSpacing, int cyTileSpacing)
{
	TileSetType *ptst;
	int cxTile, cyTile;
	int dxL, dxR, dyT, dyB;
	int dxSelL, dxSelR, dySelT, dySelB;
	int i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FTileIntersectsSelection";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fNullTile(pplt))
	{
		cxTile = cxTileSpacing;
		cyTile = cyTileSpacing;
	}
	else
	{
		ptst = TILESET_AT_PLOT(pplt);

		cxTile = ptst->Width;
		cyTile = ptst->Height;
	}
	if (dtilx < dtilxSelL)			/* use dtilx as x-reference? */
	{
		dxL = 0;
		dxR = cxTile-1;
		dxSelL = (dtilxSelL-dtilx) * cxTileSpacing + dxModSrcL;
		dxSelR = (dtilxSelL-dtilx+ctilxSel-1) * cxTileSpacing + dxModSrcR;
	}
	else							/* or use dtilxSelL as x-reference? */
	{
		dxL = (dtilx - dtilxSelL) * cxTileSpacing;
		dxR = dxL + cxTile-1;
		dxSelL = dxModSrcL;
		dxSelR = (ctilxSel-1) * cxTileSpacing + dxModSrcR;
	}
	if (dtily < dtilySelT)			/* use dtily as y-reference? */
	{
		dyT = 0;
		dyB = cyTile-1;
		dySelT = (dtilySelT-dtily) * cyTileSpacing + dyModSrcT;
		dySelB = (dtilySelT-dtily+ctilySel-1) * cyTileSpacing + dyModSrcT;
	}
	else
	{
		dyT = (dtily - dtilySelT) * cyTileSpacing;
		dyB = dyT + cyTile-1;
		dySelT = dyModSrcT;
		dySelB = (ctilySel-1) * cyTileSpacing + dyModSrcB;
	}

	/* See if entirety of tile falls entirely outside of selection rectangle. */

	if (dxR < dxSelL || dxL > dxSelR || dyB < dySelT || dyT > dySelB)
		return FALSE;

	/* See if entirety of tile falls in selection rectangle. */

	if (dxSelL <= dxL && dxR <= dxSelR && dySelT <= dyT && dyB <= dySelB)
		return TRUE;

	/* If it is a null tile, then nothing else to check. Return FALSE. */

	if (fNullTile(pplt))
		return FALSE;

	/* If any horizontal lines in the tile intersects, return TRUE */

	for (i = 0; i < ptst->Height; i++)
	{
		int dxL1, dxR1, dyT1, dyB1;

		dxL1 = dxL + ptst->pdxLeft[i];
		dxR1 = dxL + ptst->pdxRight[i];
		dyT1 = dyT + i;
		dyB1 = dyT + i;

		/* If not an entire miss, then must intersect in some way... */

		if (!(dxR1 < dxSelL || dxL1 > dxSelR || dyB1 < dySelT || dyT1 > dySelB))
			return TRUE;
	}

	/* If any vertical lines in the tile intersects, return TRUE */

	for (i = 0; i < ptst->Height; i++)
	{
		int dxL1, dxR1, dyT1, dyB1;

		dxL1 = dxL + i;
		dxR1 = dxL + i;
		dyT1 = dyT + ptst->pdyTop[i];
		dyB1 = dyT + ptst->pdyBottom[i];

		/* If not an entire miss, then must intersect in some way... */

		if (!(dxR1 < dxSelL || dxL1 > dxSelR || dyB1 < dySelT || dyT1 > dySelB))
			return TRUE;
	}

	return FALSE;
} /* FTileIntersectsSelection */


#if PIXELSELECT
/*********************************************************************
 *
 * CopyOverlapTileRect
 *
 * PURPOSE
 *		Copy a rectangular array of <ctilx>, <ctily> tiles from
 *		<dtilxSrc>, <dtilySrc> in the source layer <playSrc> to
 *		<dtilxDst>, <dtilyDst> in the destination layer.
 *
 *		This routine is "just like" CopyTileRect(), except that
 *		it sees if the tile overlaps the selection rectangle
 *		B4 it is copied to the destination.
 *
 * INPUT
 *		fSelect	: if == CUT, set tiles in SOURCE to NULL TILES
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		01/14/94 (dcc) - created.
 *		02/20/94 (dcc) - add support for play->dtilx, ->dtily
 *		02/20/94 (dcc) - add check for NULL tile b4 ->ubShowFlagAND
 *		08/19/94 (dcc) - add support for UseSparseArray
 *
*/
static void CopyOverlapTileRect(LayerType *playSrc, int dtilxSrc, int dtilySrc,
				LayerType *playDst, int dtilxDst, int dtilyDst,
				int ctilx, int ctily, int fSelect,
				int dtilxSelL, int dtilySelT, int ctilxSel, int ctilySel,
				int dxModSrcL, int dyModSrcT, int dxModSrcR, int dyModSrcB)
{

	PlotType	*ppltDst;
	PlotType	*ppltSrc;
#if PLOTARRAY
	PlotType	*startrow;
#endif
	int iy, ix;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CopyOverlapTileRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (dtilxSrc + playSrc->dtilx + ctilx > playSrc->ctilx)
		ctilx = playSrc->ctilx - (dtilxSrc + playSrc->dtilx);

	if (dtilySrc + playSrc->dtily + ctily > playSrc->ctily)
		ctily = playSrc->ctily - (dtilySrc + playSrc->dtily);

#if PLOTARRAY
	ppltDst	= playDst->Plot + (dtilyDst * playDst->ctilx) + dtilxDst;
	startrow	= playSrc->Plot + (dtilySrc * playSrc->ctilx) + dtilxSrc+playSrc->dtilx;
	ppltSrc	= startrow;
#endif

	for (iy = 0; iy < ctily; iy++)
	{
#if PLOTARRAY
#else
#if UseSparseArray
		if (!IsSparse(playDst))
			ppltDst = ActivatePlotXYatWin(playDst->p.rgrgplt,
									dtilxDst, dtilyDst+iy, winDst);
		if (!IsSparse(playSrc))
			ppltSrc = ActivatePlotXYatWin(playSrc->p.rgrgplt,
									dtilxSrc+playSrc->dtilx,
									dtilySrc+playSrc->dtily+iy, winSrc);
#else // !UseSparseArray
		ppltDst = ActivatePlotXYatWin(playDst->rgrgplt,
								dtilxDst, dtilyDst+iy, winDst);
		ppltSrc = ActivatePlotXYatWin(playSrc->rgrgplt,
								dtilxSrc+playSrc->dtilx,
								dtilySrc+playSrc->dtily+iy, winSrc);
#endif // !UseSparseArray
#endif
		for (ix = 0; ix < ctilx; ix++)
		{
#if UseSparseArray
			PlotType plt;

			if (IsSparse(playSrc))
			{
				ReadSparsePlotXY(playSrc->p.pspa,
								dtilxSrc+playSrc->dtilx+ix,
								dtilySrc+playSrc->dtily+iy,
								&plt);

				ppltSrc = &plt;
			}
			if (FTileIntersectsSelection(	ppltSrc,
									dtilxSrc+ix, dtilySrc+iy,
									dtilxSelL, dtilySelT,
									ctilxSel, ctilySel,
									dxModSrcL, dyModSrcT,
									dxModSrcR, dyModSrcB,
									playSrc->cxTile, playSrc->cyTile))
			{
				plt = *ppltSrc;

				if (plt.TileSet_ID)
					plt.Plot_Flags &=
						FAST_TILESET_PTR(plt.TileSet_ID)->ubShowFlagAND;

				if (IsSparse(playDst))
					WriteSparsePlotXY(&plt, playDst->p.pspa,
									dtilxDst+ix, dtilyDst+iy);
				else
					*ppltDst = plt;

				if (fSelect == CUT && !(playSrc->L_Flags & LAYER_LOCKED))
				{
					if (IsSparse(playSrc))
					{
						plt.Plot_Flags	= 0;
						plt.TileSet_ID	= 0;
						plt.Tile_ID	= 0;
						WriteSparsePlotXY(&plt, playSrc->p.pspa,
										dtilxSrc+playSrc->dtilx+ix,
										dtilySrc+playSrc->dtily+iy);
					}
					else
					{
						ppltSrc->Plot_Flags	= 0;
						ppltSrc->TileSet_ID	= 0;
						ppltSrc->Tile_ID	= 0;
					}
				}
			}
#else // !UseSparseArray
			if (FTileIntersectsSelection(	ppltSrc,
									dtilxSrc+ix, dtilySrc+iy,
									dtilxSelL, dtilySelT,
									ctilxSel, ctilySel,
									dxModSrcL, dyModSrcT,
									dxModSrcR, dyModSrcB,
									playSrc->cxTile, playSrc->cyTile))
			{
				/**************************************/
				/* structure assignment for the plots */

				*ppltDst = *ppltSrc;

				if (ppltDst->TileSet_ID)
					ppltDst->Plot_Flags &=
						FAST_TILESET_PTR(ppltDst->TileSet_ID)->ubShowFlagAND;

				if (fSelect == CUT &&
					!(playSrc->L_Flags & LAYER_LOCKED))
				{
					ppltSrc->Plot_Flags	= 0;
					ppltSrc->TileSet_ID	= 0;
					ppltSrc->Tile_ID	= 0;
				}
			}
#endif // !UseSparseArray
			ppltDst ++;
			ppltSrc ++;
		}
#if PLOTARRAY
		startrow	+= playSrc->ctilx;
		ppltSrc	= startrow;
#else
#if UseSparseArray
		if (!IsSparse(playDst))
			UpdatePlotRow(playDst->p.rgrgplt, iy+dtilyDst);

		if (!IsSparse(playSrc))
		{
			if (fSelect == CUT && !(playSrc->L_Flags & LAYER_LOCKED))
				UpdatePlotRow(playSrc->p.rgrgplt, iy+dtilySrc);
			else
				ReleasePlotRow(playSrc->p.rgrgplt, iy+dtilySrc);
		}
#else // !UseSparseArray
		UpdatePlotRow(playDst->rgrgplt, iy+dtilyDst);

		if (fSelect == CUT && !(playSrc->L_Flags & LAYER_LOCKED))
			UpdatePlotRow(playSrc->rgrgplt, iy+dtilySrc);
		else
			ReleasePlotRow(playSrc->rgrgplt, iy+dtilySrc);
#endif // !UseSparseArray
#endif
	}
} /* CopyOverlapTileRect */
#endif // PIXELSELECT


/**************************************************************************
 *
 * FinishSelect
 *
 * PURPOSE
 *		Finished selecting.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * ASSUMES
 *		Unselect() has been called.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		04/13/91 (RGM) - Now you can undo a CUT.
 *		07/11/93 (dcc) - support tile counts by calling copy BACKTODISPLAY
 *						& XORDrawPBlk B4 copy DISPLAYTOBACK
 *		12/15/93 (dcc) - clip selection to limits of source room
 *		12/16/93 (dcc) - add call to MakeSpecialTiles()
 *		07/07/94 (dcc) - add support for fPixelSelect
 *		07/18/94 (dcc) - check for NULL ->FirstTileSet B4 calling
 *						RegisterTileType()
 *		08/03/94 (dcc) - add calls to AddTilesInSubRect() if CUTting
 *		08/03/94 (dcc) - ShowCountsInSubRect(whole room) if CUTting
 *		08/03/94 (dcc) - only adjust width & height if fPixelSelect
 *		08/22/94 (dcc) - add support for UseSparseArray
 *		08/30/94 (dcc) - add call to DCC_SetPointer()
 *		09/08/94 (dcc) - PixelSelect: scale values for AddTilesInSubRect()
 *		09/17/94 (dcc) - make brush layer same sparse/non-sparse as room layer
 *		10/19/94 (dcc) - Fix parameters passed to RegisterTileType().
 *		10/20/94 (dcc) - Kludge cut pixel-object to redraw room. (UGH).
 *
*/
static void FinishSelect (void)
{
	WORD		height, width;
	LayerType	*destlayer;
	LayerType	*roomlayer;
	LayerType	*templayer;
	RoomType	*room;
	unsigned	dtilxSrc, dtilySrc;
	BOOL		did_select	= FALSE;
	BOOL		leave		= FALSE;
#if PixelLayers
	BOOL		fFirstLayer = TRUE;	//KLUDGE FIXME BUGBUG only copy objects if first layer
#endif // !PixelLayers

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FinishSelect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalBlockCopy)
	{
#if PixelLayers
		int dtilx, dtily, ctilx, ctily;
#endif // PixelLayers

		if (! GlobalBlockCopy->SourceRoom) goto THANKYOUVERYMUCH;

		GlobalPointer = DCC_SetPointer (BPI_WAIT_POINTER);

		room		= GlobalBlockCopy->SourceStuff->Room;
		roomlayer = GlobalBlockCopy->SourceStuff->Room->FloorLayer;

		GlobalBlockCopy->FirstTileSet  = NULL;

#if PIXELSELECT
		if (fPixelSelect)
		{
			/*	If lower right corner is between tiles, then subtract one
				from the width (height), and set dxModSrcR (dyModSrcB)
				to play->ctilx (play->ctily). */

			if (GlobalBlockCopy->dxModSrcR == -1 && GlobalBlockCopy->ctilxSelect > 1)
			{
				GlobalBlockCopy->ctilxSelect--;
				GlobalBlockCopy->dxModSrcR = roomlayer->cxTile;
			}
			if (GlobalBlockCopy->dyModSrcB == -1 && GlobalBlockCopy->ctilySelect > 1)
			{
				GlobalBlockCopy->ctilySelect--;
				GlobalBlockCopy->dyModSrcB = roomlayer->cyTile;
			}
		}
		width	= (WORD) GlobalBlockCopy->ctilxSelect;
		height	= (WORD) GlobalBlockCopy->ctilySelect;
#endif // PIXELSELECT
#if !PIXELSELECT
		width	= (WORD) ctilxGlobalSelect;
		height	= (WORD) ctilyGlobalSelect;
#endif // !PIXELSELECT

#if PIXELSELECT
		/* Adjust selected rectangle based upon ctilxOverhang, ctilyOverhang */

		if (fPixelSelect)
		{
			int i;

			i = min(ctilxOverhang, GlobalBlockCopy->SourceX);
			GlobalBlockCopy->dtilxOffset = -i;
			width += i;
			i = min(ctilyOverhang, GlobalBlockCopy->SourceY);
			GlobalBlockCopy->dtilyOffset = -i;
			height += i;
		}
#if 0
		/* Adjust selected rectangle based upon ctilxOverhang, ctilyOverhang */

	    {
		int i;

		i = min(ctilxOverhang, GlobalBlockCopy->SourceX);
		GlobalBlockCopy->SourceX -= i;
		width += i;
		i = min(ctilyOverhang, GlobalBlockCopy->SourceY);
		GlobalBlockCopy->SourceY -= i;
		height += i;
	    }
#endif
#endif // PIXELSELECT

#if PIXELSELECT
		if (GlobalBlockCopy->SourceX + GlobalBlockCopy->dtilxOffset + width  > roomlayer->ctilx)
			width  = roomlayer->ctilx - (GlobalBlockCopy->SourceX+GlobalBlockCopy->dtilxOffset);

		if (GlobalBlockCopy->SourceY + GlobalBlockCopy->dtilyOffset + height > roomlayer->ctily)
			height = roomlayer->ctily - (GlobalBlockCopy->SourceY+GlobalBlockCopy->dtilyOffset);

		/**********************************************/
		/* This is the fix to allow you to undo a cut */

		if (SelectMode == CUT)
		{
#if PixelLayers
			dtilx = GlobalBlockCopy->SourceX + GlobalBlockCopy->dtilxOffset,
			dtily = GlobalBlockCopy->SourceY + GlobalBlockCopy->dtilyOffset,
			ctilx = GlobalBlockCopy->dtilxOffset + width;
			ctily = GlobalBlockCopy->dtilyOffset + height;

			BigTileCoordsFromSmallTile(&room->Layers,
									room->FloorLayer,
									&dtilx, &dtily, &ctilx, &ctily);

 			StartUndo(room, UndoRoom, dtilx, dtily, dtilx+ctilx-1, dtily+ctily-1);

			AddTilesInSubRect(&room->Layers,
						dtilx, dtily, ctilx, ctily,
						-1, &room->prgtct, &room->fcntValid);
#else // !PixelLayers
 			StartUndo (
				room,
				UndoRoom,
				GlobalBlockCopy->SourceX + GlobalBlockCopy->dtilxOffset,
				GlobalBlockCopy->SourceY + GlobalBlockCopy->dtilyOffset,
				(WORD) (GlobalBlockCopy->SourceX+GlobalBlockCopy->dtilxOffset + width - 1),
				(WORD) (GlobalBlockCopy->SourceY+GlobalBlockCopy->dtilyOffset + height - 1)
			);
			AddTilesInSubRect(&room->Layers,
						GlobalBlockCopy->SourceX + GlobalBlockCopy->dtilxOffset,
						GlobalBlockCopy->SourceY + GlobalBlockCopy->dtilyOffset,
						width, height,
						-1, &room->prgtct, &room->fcntValid);
#endif // !PixelLayers
		}
#endif // PIXELSELECT
#if !PIXELSELECT
		if (GlobalBlockCopy->SourceX + width  > roomlayer->ctilx)
			width  = roomlayer->ctilx - GlobalBlockCopy->SourceX;

		if (GlobalBlockCopy->SourceY + height > roomlayer->ctily)
			height = roomlayer->ctily - GlobalBlockCopy->SourceY;

		/**********************************************/
		/* This is the fix to allow you to undo a cut */

		if (SelectMode == CUT)
		{
#if PixelLayers
			dtilx = GlobalBlockCopy->SourceX;
			dtily = GlobalBlockCopy->SourceY;
			ctilx = width - 1;
			ctily = height - 1;

			BigTileCoordsFromSmallTile(&room->Layers,
									room->FloorLayer,
									&dtilx, &dtily, &ctilx, &ctily);

 			StartUndo(room, UndoRoom, dtilx, dtily, dtilx+ctilx-1, dtilx+ctily-1);

			AddTilesInSubRect(&room->Layers,
						dtilx, dtily, ctilx, ctily,
						-1, &room->prgtct, &room->fcntValid);
#else // !PixelLayers
 			StartUndo (
				room,
				UndoRoom,
				GlobalBlockCopy->SourceX,
				GlobalBlockCopy->SourceY,
				(WORD) (GlobalBlockCopy->SourceX + width - 1),
				(WORD) (GlobalBlockCopy->SourceY + height - 1)
			);
			AddTilesInSubRect(&room->Layers,
						GlobalBlockCopy->SourceX,
						GlobalBlockCopy->SourceY,
						width, height,
						-1, &room->prgtct, &room->fcntValid);
#endif // !PixelLayers
		}
#endif // !PIXELSELECT

		/* Allocate layers we need to store tile brush */

		if (roomlayer)
		{
			Assert(roomlayer->cxTile != 0);
			dtilxSrc = (unsigned) (GlobalBlockCopy->SourceStuff->dxSrc / roomlayer->cxTile);
			Assert(roomlayer->cyTile != 0);
			dtilySrc = (unsigned) (GlobalBlockCopy->SourceStuff->dySrc / roomlayer->cyTile);

			while (!IsEOList(roomlayer) && !leave)
			{
#if PLOTARRAY
				short layerwidth = roomlayer->ctilx;
#endif
				did_select = TRUE;

#if UseSparseArray
				destlayer = AddClearedLayerToLayers (&(GlobalBlockCopy->Layers), width, height, IsSparse(roomlayer));
#else // !UseSparseArray
				destlayer = AddClearedLayerToLayers (&(GlobalBlockCopy->Layers), width, height);
#endif // !UseSparseArray
				if (! destlayer)
				{
/**/				goto ABORT;
				}
				(GlobalBlockCopy->LayerCount) ++;
				roomlayer	= Next (roomlayer);
				leave	= (OneLayerCopy ? TRUE : FALSE);
			}
			/* Copy selected tiles to tile-brush. */

			roomlayer = GlobalBlockCopy->SourceStuff->Room->FloorLayer;
			destlayer = Head(&(GlobalBlockCopy->Layers));
			leave	= FALSE;

			while (!IsEOList (roomlayer) && !leave)
			{
				if (IsComposite(room) && IsLocked(room))
				{
#if PIXELSELECT
					SetCompositeTileRect(
								roomlayer,
								GlobalBlockCopy->SourceX+GlobalBlockCopy->dtilxOffset,
								GlobalBlockCopy->SourceY+GlobalBlockCopy->dtilyOffset,
								destlayer, 0, 0,
								width, height, room->ptsComposite->TS_id);
#endif // PIXELSELECT
#if !PIXELSELECT
					SetCompositeTileRect(
								roomlayer,
								GlobalBlockCopy->SourceX, GlobalBlockCopy->SourceY,
								destlayer, 0, 0,
								width, height, room->ptsComposite->TS_id);
#endif // !PIXELSELECT
				}
				else
				{
#if PIXELSELECT
#if PixelLayers
					// KLUDGE BUGBUG FIXME only copy objects if 1st layer
					if (!fFirstLayer && roomlayer->cxTile == 1 && roomlayer->cyTile == 1)
					{
						;	// KLUDGE BUGBUG FIXME do nothing!
					}
					else
					{
#endif // !PixelLayers
					if (fPixelSelect)
					{
						CopyOverlapTileRect(
								roomlayer,
								GlobalBlockCopy->SourceX+GlobalBlockCopy->dtilxOffset,
								GlobalBlockCopy->SourceY+GlobalBlockCopy->dtilyOffset,
								destlayer, 0, 0,
								width, height, SelectMode,
								GlobalBlockCopy->SourceX, GlobalBlockCopy->SourceY,
								GlobalBlockCopy->ctilxSelect, GlobalBlockCopy->ctilySelect,
								GlobalBlockCopy->dxModSrcL, GlobalBlockCopy->dyModSrcT,
								GlobalBlockCopy->dxModSrcR, GlobalBlockCopy->dyModSrcB);
					}
					else
					{
						CopyTileRect(	roomlayer,
								GlobalBlockCopy->SourceX,
								GlobalBlockCopy->SourceY,
								destlayer, 0, 0,
								width, height, SelectMode);
					}
#if PixelLayers
					}		// KLUDGE BUGBUG FIXME only copy objects if 1st layer
#endif // !PixelLayers
#endif // PIXELSELECT
#if !PIXELSELECT
					CopyTileRect(	roomlayer,
								GlobalBlockCopy->SourceX,
								GlobalBlockCopy->SourceY,
								destlayer, 0, 0,
								width, height, SelectMode);
#endif // !PIXELSELECT
				}
				destlayer->cxTile = roomlayer->cxTile;
				destlayer->cyTile = roomlayer->cyTile;

				roomlayer	= Next(roomlayer);
				destlayer = Next(destlayer);
				leave	= (OneLayerCopy ? TRUE : FALSE);
#if PixelLayers
				fFirstLayer = FALSE;	// KLUDGE BUGBUG FIXME only copy objects if 1st layer
#endif // !PixelLayers
			}
		}

		templayer  = Head (&(GlobalBlockCopy->Layers));
		if (! IsEOList (templayer))
		{
			GlobalBlockCopy->BC_FloorLayer =
							Head (&(GlobalBlockCopy->Layers));
		}
		else
		{
			GlobalBlockCopy->BC_FloorLayer = NULL;
		}

		if (!MakeSpecialTiles(GlobalBlockCopy))
/**/		goto ABORT;

		ComputeBrushSize(GlobalBlockCopy);

		if (SelectMode == CUT)
		{
#if PixelLayers
			AddTilesInSubRect(&room->Layers,
						dtilx, dtily, ctilx, ctily,
						1, &room->prgtct, &room->fcntValid);
#else // !PixelLayers
#if PIXELSELECT
			AddTilesInSubRect(&room->Layers,
						GlobalBlockCopy->SourceX + GlobalBlockCopy->dtilxOffset,
						GlobalBlockCopy->SourceY + GlobalBlockCopy->dtilyOffset,
						width, height,
						1, &room->prgtct, &room->fcntValid);
#endif // PIXELSELECT
#if !PIXELSELECT
			AddTilesInSubRect(&room->Layers,
						GlobalBlockCopy->SourceX,
						GlobalBlockCopy->SourceY,
						width, height,
						1, &room->prgtct, &room->fcntValid);
#endif // !PIXELSELECT
#endif // !PixelLayers
		}

		if ((did_select) && (SelectMode == CUT))
		{
#if PIXELSELECT
//			if (	GlobalBlockCopy->BC_FloorLayer->cxTile == 1 &&
//					GlobalBlockCopy->BC_FloorLayer->cyTile == 1	)
//			{
				ShowRoom(GlobalRoomWindow);	//KLUDGE BUGBUG FIXME
//			}
//			else
//			{
//				RedrawSelectArea (GlobalBlockCopy, FALSE, FALSE,
//					GlobalBlockCopy->SourceX+GlobalBlockCopy->dtilxOffset - dtilxSrc,
//					GlobalBlockCopy->SourceY+GlobalBlockCopy->dtilyOffset - dtilySrc);
//
//				XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);
//			}
#endif  // PIXELSELECT
#if !PIXELSELECT
			RedrawSelectArea (GlobalBlockCopy, FALSE, FALSE,
				GlobalBlockCopy->SourceX - dtilxSrc,
				GlobalBlockCopy->SourceY - dtilySrc);

			XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);
#endif  // !PIXELSELECT
		}
#if __MSDOS__
		else
		{
#if PIXELSELECT
			CopyDisplayTilesToBack(GlobalBlockCopy->SourceRW,
							GlobalBlockCopy->SourceX+GlobalBlockCopy->dtilxOffset,
							GlobalBlockCopy->SourceY+GlobalBlockCopy->dtilyOffset,
							width, height,
							BACKTODISPLAY);
#endif  // PIXELSELECT
#if !PIXELSELECT
			CopyDisplayTilesToBack(GlobalBlockCopy->SourceRW,
							GlobalBlockCopy->SourceX, GlobalBlockCopy->SourceY,
							width, height,
							BACKTODISPLAY);
#endif  // !PIXELSELECT
			XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);
		}
#if PIXELSELECT
		CopyDisplayTilesToBack(GlobalBlockCopy->SourceRW,
						GlobalBlockCopy->SourceX+GlobalBlockCopy->dtilxOffset,
						GlobalBlockCopy->SourceY+GlobalBlockCopy->dtilyOffset,
						width, height,
						DISPLAYTOBACK);

		if (SelectMode == CUT)
		{
			CopyDisplayToBack(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd, BACKTODISPLAY);
			ShowCountsInSubRect(GlobalBlockCopy->SourceRoom,
						GlobalBlockCopy->SourceRW,
						0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
						0, 0,
						dtilxSrc, dtilySrc,
						wShowXTiles, wShowYTiles);
		}
		else
		{
			ShowCountsInSubRect(GlobalBlockCopy->SourceRoom,
						GlobalBlockCopy->SourceRW,
						0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
						GlobalBlockCopy->SourceX+GlobalBlockCopy->dtilxOffset - dtilxSrc,
						GlobalBlockCopy->SourceY+GlobalBlockCopy->dtilyOffset - dtilySrc,
						GlobalBlockCopy->SourceX+GlobalBlockCopy->dtilxOffset,
						GlobalBlockCopy->SourceY+GlobalBlockCopy->dtilyOffset,
						GlobalBlockCopy->BC_FloorLayer->ctilx,
						GlobalBlockCopy->BC_FloorLayer->ctily);
		}
#endif  // PIXELSELECT
#if !PIXELSELECT
		CopyDisplayTilesToBack(GlobalBlockCopy->SourceRW,
						GlobalBlockCopy->SourceX, GlobalBlockCopy->SourceY,
						width, height,
						DISPLAYTOBACK);

		if (SelectMode == CUT)
		{
			CopyDisplayToBack(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd, BACKTODISPLAY);
			ShowCountsInSubRect(GlobalBlockCopy->SourceRoom,
						GlobalBlockCopy->SourceRW,
						0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
						0, 0,
						dtilxSrc, dtilySrc,
						wShowXTiles, wShowYTiles);
		}
		else
		{
			ShowCountsInSubRect(GlobalBlockCopy->SourceRoom,
						GlobalBlockCopy->SourceRW,
						0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
						GlobalBlockCopy->SourceX - dtilxSrc,
						GlobalBlockCopy->SourceY - dtilySrc,
						GlobalBlockCopy->SourceX,
						GlobalBlockCopy->SourceY,
						GlobalBlockCopy->BC_FloorLayer->ctilx,
						GlobalBlockCopy->BC_FloorLayer->ctily);
		}
#endif  // !PIXELSELECT
#endif/*__MSDOS__*/
		GlobalBlockCopy->FirstTileSet  =
							FindFirstSelect (GlobalBlockCopy);

		if (!IsEdit(GlobalBlockCopy->SourceRoom) &&
			GlobalBlockCopy->FirstTileSet != NULL)
		{
			Assert(GlobalBlockCopy->BC_FloorLayer->cxTile);
			Assert(GlobalBlockCopy->BC_FloorLayer->cyTile);
			RegisterTileType(	GlobalBlockCopy->FirstTileSet->UserType,
							GlobalRoomWindow->CurrentRoom->Room,
							GlobalRoomWindow->CurrentRoom->dxSrc,
							GlobalRoomWindow->CurrentRoom->dySrc);
		}

		if (CANSHOW(LastTBar) && (IsSource(GlobalBlockCopy->SourceRoom)))
		{
			ShowState (LastTBar);
		}
	}
	MButtonsE ();
	OldColumn	= -1;
	OldRow	= -1;

THANKYOUVERYMUCH:
#if __AMIGAOS__
	GlobalPointer = RGM_SetPointer (Old_Pointer, MainWindow);
#elif __MSDOS__
	SetPointerMode();
#endif/*__AMIGAOS__/__MSDOS__*/

	OneLayerCopy = FALSE;
	return;
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:
#if __AMIGAOS__
	GlobalPointer = RGM_SetPointer (Old_Pointer, MainWindow);
#elif __MSDOS__
	SetPointerMode();
#endif/*__AMIGAOS__/__MSDOS__*/

	OneLayerCopy = FALSE;
	TellUser (OOM, "Unable to select block.");
	Unselect ();
	return;

} /* FinishSelect */

