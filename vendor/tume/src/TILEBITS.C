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
 * TILEBITS.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 01/19/92
 *   MODIFIED : 10/02/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Pixel color mask routines for tUME.
 *
 * HISTORY
 *		03/20/93 Saturday (dcc) - change calls to calloc(), malloc(), & CreateNode()
 *					to call callocx(), mallocx(), & CreateNodex(), which also calls
 *					SetGlobalErr() if out of memory.
 *
 *		03/19/93 Friday (dcc) - fix AllocateTileMaskArrays(), AddEmptyEvents(),
 *					AllocateDisplayMaskArrays(), AllocateCursorMovements(),
 *					and AllocateZoomEventArrays(), so they check for 0 b4 malloc'ing.
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include <echidna/etypes.h>
#include <echidna/listfunc.h>
#include <echidna/eerrors.h>
#include <echidna/menus2.h>
#include "tilebits.h"
#include "tuglbl.h"
#include "tumedraw.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

UBYTE ubXFlipBit;
UBYTE ubYFlipBit;

WORD wShowXTiles = 0;						/* Tiles across to show on-screen */
WORD wShowYTiles = 0;						/* Tiles high to show on-screen */

WORD wZoomInOne = -1;	/* Zoom index to use when user zooms in one from no zoom */
WORD wZoomOutOne = -1;	/* Zoom index to use when user zooms out one from no zoom */

static ListType ColorGroupMaskList;

static char *pcEventStrings[wGroupMax];/* Store all events text */
static EventState *pFirstEvent[wGroupMax];/* Points to first tile mask event */

static WORD wEventChMax[wGroupMax];		/* size of pcEventStrings[] array */
static WORD wCntCh[wGroupMax];			/* next free char in pcEventStrings[] */

static UBYTE *ubMaskEventANDMaskL;		/* array of AND Masks for L mouse button */
static UBYTE *ubMaskEventORMaskL;		/* array of OR Masks for L mouse button */
static UBYTE *ubMaskEventXORMaskL;		/* array of XOR Masks for L mouse button */
static UBYTE *ubMaskEventANDMaskR;		/* array of AND Masks for R mouse button */
static UBYTE *ubMaskEventORMaskR;		/* array of OR Masks for R mouse button */
static UBYTE *ubMaskEventXORMaskR;		/* array of XOR Masks for R mouse button */

static WORD wLastEventIndex;				/* current index into ubMaskEvent arrays */

static WORD *wMoveCursorX;					/* array of x cursor movements */
static WORD *wMoveCursorY;					/* array of y cursor movements */

static WORD *wShiftLayerX;					/* array of x shift layer amounts */
static WORD *wShiftLayerY;					/* array of y shift layer amounts */

static WORD *wZoomDstDup;					/* array of zoom destination amounts */
static WORD *wZoomSrcSkip;					/* array of zoom source amounts */
static WORD wMaxZooms;						/* number of entries in zoom arrays */

static UBYTE *ubDisplayANDMask;			/* array of display AND Masks */
static UBYTE *ubDisplayORMask;			/* array of display OR Masks */
static UBYTE *ubDisplayXORMask;			/* array of display XOR Masks */
static short *fDisplayMasking;			/* array of display masks OFF or ON */

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/* callocx - calls MEM_calloc(), if out of memory, calls SetGlobalErr(). */

static void *callocx(size_t nitems, size_t size)
{
	void *pv;

	if ((pv = MEM_calloc(nitems, size)) == NULL)
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEcatf ("\nOut of memory.");
	}
	return pv;
}


/* mallocx - calls MEM_malloc(), if out of memory, calls SetGlobalErr(). */

static void *mallocx(size_t size)
{
	void *pv;

	if ((pv = MEM_malloc(size)) == NULL)
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEcatf ("\nOut of memory.");
	}
	return pv;
}


/* CreateNodex - calls CreateNode(), if out of memory, calls SetGlobalErr(). */

static void *CreateNodex(size_t size, char *name)
{
	void *pv;

	if ((pv = CreateNode(size, name)) == NULL)
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEcatf ("\nOut of memory.");
	}
	return pv;
}


/* OpenColorGroupMask - initialize list. */

void OpenColorGroupMask(void)
{
	InitList(&ColorGroupMaskList);
}

/* AddNewColorGroup - add a new ColorGroupMask to ColorGroupMaskList.
	Initialize new ColorGroupMask so it doesn't affect anything.
	Returns pointer to node just added, or NULL if failed. */

ColorGroupMask *AddNewColorGroup(void)
{
	ColorGroupMask *pcgm;

	if ((pcgm = CreateNodex(sizeof(ColorGroupMask), NULL)) != NULL)
	{
		int i;

		for (i = 0; i < wColorMaskMax; i++)
		{
			pcgm->ubPixelANDMask[i] = 0xff;
			pcgm->ubPixelORMask[i] = 0;
		}
		pcgm->ubEnableANDMask = 0;

		AddTail(&ColorGroupMaskList, pcgm);
	}
	return pcgm;
}

/* FirstColorGroup - return pointer to 1st ColorGroupMask.
	You should check return value for IsEOList.
	Use Next() to get next color group. */

ColorGroupMask *FirstColorGroup(void)
{
	return Head(&ColorGroupMaskList);
}

/* GetColorEnableANDMask - return <ubEnableANDMask> for ColorGroupMask <pcgm>. */

UBYTE GetColorEnableANDMask(ColorGroupMask *pcgm)
{
	return pcgm->ubEnableANDMask;
}

/* GetColorConfigANDMask - return <ubConfigANDMask> for ColorGroupMask <pcgm>. */

UBYTE GetColorConfigANDMask(ColorGroupMask *pcgm)
{
	return pcgm->ubConfigANDMask;
}

/* GetPixelANDMask - return <ubPixelANDMask[w]> for ColorGroupMask <pcgm>. */

UBYTE GetPixelANDMask(ColorGroupMask *pcgm, WORD w)
{
	return pcgm->ubPixelANDMask[w];
}

/* GetPixelORMask - return <ubPixelORMask[w]> for ColorGroupMask <pcgm>. */

UBYTE GetPixelORMask(ColorGroupMask *pcgm, WORD w)
{
	return pcgm->ubPixelORMask[w];
}

/* SetColorEnableANDMask - set <ubEnableANDMask> for ColorGroupMask <pcgm>. */

void SetColorEnableANDMask(ColorGroupMask *pcgm, UBYTE ub)
{
	pcgm->ubEnableANDMask = ub;
}

/* SetColorConfigANDMask - set <ubConfigANDMask> for ColorGroupMask <pcgm>. */

void SetColorConfigANDMask(ColorGroupMask *pcgm, UBYTE ub)
{
	pcgm->ubConfigANDMask = ub;
}

/* SetColorMasks - set all ColorGroupMask <pcgm> mask 
	<ubPixelANDMask>, and <ubPixelORMask> values
	that match <ubTileANDMask> and <ubTileORMask>. */

void SetColorMasks(ColorGroupMask *pcgm,
				UBYTE ubTileANDMask, UBYTE ubTileORMask,
				UBYTE ubPixelANDMask, UBYTE ubPixelORMask)
{
	int i;

	for (i = 0; i < wColorMaskMax; i++)
	{
		if (((UBYTE) i & (~ubTileANDMask)) == ubTileORMask)
		{
			pcgm->ubPixelANDMask[i] &= ubPixelANDMask;
			pcgm->ubPixelORMask[i] |= ubPixelORMask;
		}
	}
}

/* SaveFirstEvent - save pointer to 1st event that we add. */

static void SaveFirstEvent(WORD wGroup, EventState *pes)
{
	pFirstEvent[wGroup] = pes;
}

/* GetFirstEvent - return pointer to 1st event. */

EventState *GetFirstEvent(WORD wGroup)
{
	return pFirstEvent[wGroup];
}

/* SaveEventString - save string in next free space in pcEventStrings[].
	Return <NULL> if unable to save. */

char *SaveEventString(WORD wGroup, char *sz)
{
	char *pch;
	WORD wszLen = strlen(sz);

	if (wCntCh[wGroup] + wszLen >= wEventChMax[wGroup])
		return NULL;							/* Out of space. */

	pch = strcpy(pcEventStrings[wGroup] + wCntCh[wGroup], sz);
	wCntCh[wGroup] += wszLen + 1;
	return pch;
}

/* SaveMaskEventMask - save <ubANDMaskL>, <ubORMaskL>, <ubXORMaskL>,
	<ubANDMaskR>, <ubORMaskL>, ubXORMaskL>	in ubMaskEventANDMaskL[w], etc. */

void SaveMaskEventMask(WORD w, UBYTE ubANDMaskL, UBYTE ubORMaskL, UBYTE ubXORMaskL,
							UBYTE ubANDMaskR, UBYTE ubORMaskR, UBYTE ubXORMaskR)
{
	ubMaskEventANDMaskL[w] = ubANDMaskL;
	ubMaskEventORMaskL[w] = ubORMaskL;
	ubMaskEventXORMaskL[w] = ubXORMaskL;
	ubMaskEventANDMaskR[w] = ubANDMaskR;
	ubMaskEventORMaskR[w] = ubORMaskR;
	ubMaskEventXORMaskR[w] = ubXORMaskR;
}

/* AllocateTileMaskArrays - allocate <wCnt> array of <ubMaskEventANDMaskL>,
	<ubMaskEventORMaskL>, <ubMaskEventXORMaskL>, <ubMaskEventANDMaskR>,
	<ubMaskEventORMaskR>, <ubMaskEventXORMaskR>.	Return TRUE if successful. */

BOOL AllocateTileMaskArrays(WORD wCnt)
{
	if (wCnt)
	{
		if ((ubMaskEventANDMaskL = mallocx(wCnt)) == NULL)
			return FALSE;

		if ((ubMaskEventORMaskL = mallocx(wCnt)) == NULL)
			return FALSE;

		if ((ubMaskEventXORMaskL = mallocx(wCnt)) == NULL)
			return FALSE;

		if ((ubMaskEventANDMaskR = mallocx(wCnt)) == NULL)
			return FALSE;

		if ((ubMaskEventORMaskR = mallocx(wCnt)) == NULL)
			return FALSE;

		if ((ubMaskEventXORMaskR = mallocx(wCnt)) == NULL)
			return FALSE;
	}
	return TRUE;
}

/* SaveCursorMovement - save cursor movements <wDX>, <wDY> associated
	with event <w>. */

void SaveCursorMovement(WORD w, WORD wDX, WORD wDY)
{
	wMoveCursorX[w] = wDX;
	wMoveCursorY[w] = wDY;
}

/* AllocateCursorMovements - allocate <wCnt> array of <wMoveCursorX>,
	<wMoveCursorY>. Return TRUE if successful. */

BOOL AllocateCursorMovements(WORD wCnt)
{
	if (wCnt)
	{
		if ((wMoveCursorX = callocx(wCnt, sizeof(WORD))) == NULL)
			return FALSE;

		if ((wMoveCursorY = callocx(wCnt, sizeof(WORD))) == NULL)
			return FALSE;
	}
	return TRUE;
}


/*********************************************************************
 *
 * SaveShiftLayer
 *
 * PURPOSE
 *		Save the amounts to shift (offset) each layer.
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
void SaveShiftLayer(WORD w, WORD wDX, WORD wDY)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveShiftLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	wShiftLayerX[w] = wDX;
	wShiftLayerY[w] = wDY;

} /* SaveShiftLayer */


/*********************************************************************
 *
 * AllocateShiftLayer
 *
 * PURPOSE
 *		Allocate <wCnt> array of <wShiftLayerX>, <wShiftLayerY>.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *		02/20/94 Sunday (dcc) - created.
 *
*/
BOOL AllocateShiftLayer(WORD wCnt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AllocateShiftLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (wCnt)
	{
		if ((wShiftLayerX = callocx(wCnt, sizeof(WORD))) == NULL)
			return FALSE;

		if ((wShiftLayerY = callocx(wCnt, sizeof(WORD))) == NULL)
			return FALSE;
	}
	return TRUE;
} /* AllocateShiftLayer */


/*********************************************************************
 *
 * SaveZoomAmount
 *
 * SYNOPSIS
 *		void SaveZoomAmount(WORD w, WORD wDst, WORD wSrc)
 *
 * PURPOSE
 *		Save zoom amount in array[w]. Also,
 *		set wZoomInOne to last zoom setting where wDst > wSrc, and
 *		set wZoomOutOne to first zoom setting where wDst < wSrc.
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
void SaveZoomAmount(WORD w, WORD wDst, WORD wSrc)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveZoomAmount";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	wZoomDstDup[w] = wDst;
	wZoomSrcSkip[w] = wSrc;

	/* Set wZoomInOne to last zoom setting where wDst > wSrc */

	if (wDst > wSrc)
		SetZoomInOne(w);

	/* Set wZoomOutOne to first zoom setting where wDst < wSrc */

	if (wDst < wSrc && (GetZoomOutOne() == -1))
		SetZoomOutOne(w);

} /* SaveZoomAmount */


/*********************************************************************
 *
 * AllocateZoomEventArrays
 *
 * SYNOPSIS
 *		BOOL AllocateZoomEventArrays(WORD wCnt)
 *
 * PURPOSE
 *		Allocate arrays to store zoom amounts.
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
BOOL AllocateZoomEventArrays(WORD wCnt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AllocateZoomEventArrays";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	wMaxZooms = wCnt;

	if (wCnt)
	{
		if ((wZoomDstDup = callocx(wCnt, sizeof(WORD))) == NULL)
			return FALSE;

		if ((wZoomSrcSkip = callocx(wCnt, sizeof(WORD))) == NULL)
			return FALSE;
	}
	return TRUE;
} /* AllocateZoomEventArrays */


/* AddEmptyEvents - allocate memory for <wCnt> empty events,
	and <wSize> bytes of strings. Return TRUE if successful. */

BOOL AddEmptyEvents(WORD wGroup, WORD wCnt, WORD wSize)
{
	WORD i;

	wEventChMax[wGroup] = wSize;
	wCntCh[wGroup] = 0;

	if (wSize)
		if ((pcEventStrings[wGroup] = mallocx(wSize)) == NULL)
			return FALSE;

	for (i = 0; i < wCnt; i++)
	{
		EventState *pes;

		if ((pes = CreateNodex(sizeof(EventState), NULL)) == NULL)
		{
			return FALSE;
		}

		AddTail(EventList, pes);

		if (i == 0)
			SaveFirstEvent(wGroup, pes);
	}
	return TRUE;
}

/*********************************************************************
 *
 * GetTileBitsANDMaskL
 *
 * SYNOPSIS
 *		UBYTE GetTileBitsANDMaskL(void)
 *
 * PURPOSE
 *		Returns current AND ubMaskEvent mask for left mouse button.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		ubMaskEventANDMaskL[wLastEventIndex]
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UBYTE GetTileBitsANDMaskL(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetTileBitsANDMaskL";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return ubMaskEventANDMaskL[wLastEventIndex];

} /* GetTileBitsANDMaskL */

/*********************************************************************
 *
 * GetTileBitsORMaskL
 *
 * SYNOPSIS
 *		UBYTE GetTileBitsORMaskL(void)
 *
 * PURPOSE
 *		Returns current OR ubMaskEvent mask for left mouse button.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		ubMaskEventORMaskL[wLastEventIndex]
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UBYTE GetTileBitsORMaskL(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetTileBitsORMaskL";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return ubMaskEventORMaskL[wLastEventIndex];

} /* GetTileBitsORMaskL */

/*********************************************************************
 *
 * GetTileBitsXORMaskL
 *
 * SYNOPSIS
 *		UBYTE GetTileBitsXORMaskL(void)
 *
 * PURPOSE
 *		Returns current XOR ubMaskEvent mask for left mouse button.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		ubMaskEventXORMaskL[wLastEventIndex]
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UBYTE GetTileBitsXORMaskL(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetTileBitsXORMaskL";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return ubMaskEventXORMaskL[wLastEventIndex];

} /* GetTileBitsXORMaskL */

/*********************************************************************
 *
 * GetTileBitsANDMaskR
 *
 * SYNOPSIS
 *		UBYTE GetTileBitsANDMaskR(void)
 *
 * PURPOSE
 *		Returns current AND ubMaskEvent mask for left mouse button.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		ubMaskEventANDMaskR[wLastEventIndex]
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UBYTE GetTileBitsANDMaskR(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetTileBitsANDMaskR";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return ubMaskEventANDMaskR[wLastEventIndex];

} /* GetTileBitsANDMaskR */

/*********************************************************************
 *
 * GetTileBitsORMaskR
 *
 * SYNOPSIS
 *		UBYTE GetTileBitsORMaskR(void)
 *
 * PURPOSE
 *		Returns current OR ubMaskEvent mask for left mouse button.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		ubMaskEventORMaskR[wLastEventIndex]
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UBYTE GetTileBitsORMaskR(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetTileBitsORMaskR";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return ubMaskEventORMaskR[wLastEventIndex];

} /* GetTileBitsORMaskR */

/*********************************************************************
 *
 * GetTileBitsXORMaskR
 *
 * SYNOPSIS
 *		UBYTE GetTileBitsXORMaskR(void)
 *
 * PURPOSE
 *		Returns current XOR ubMaskEvent mask for left mouse button.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		ubMaskEventXORMaskR[wLastEventIndex]
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UBYTE GetTileBitsXORMaskR(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetTileBitsXORMaskR";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return ubMaskEventXORMaskR[wLastEventIndex];

} /* GetTileBitsXORMaskR */


/*********************************************************************
 *
 * TileMaskEvent
 *
 * SYNOPSIS
 *		short TileMaskEvent(EventInfo *ei)
 *
 * PURPOSE
 *		This routine deals with all the events defined the
 *		[Tile Mask Events] in the .INI file. All such events call this
 *		routine This routine figures out which event generated the call by
 *		searching forward from pFirstEvent[wTileMaskGroup] until it encounters
 *		the same event. It then uses this event number to index the arrays
 *		ubMaskEventANDMask, ubMaskEventORMask, & ubMaskEventXORMask.
 *		It then also sets <TU_DrawMode> = DMODE_COLOR1.
 *
 * INPUT
 *		<ei> which describes which event occurred.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful, FALSE otherwise.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short TileMaskEvent(EventInfo *ei)
{
	EventState *pes;
	WORD i = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TileMaskEvent";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pes = GetFirstEvent(wTileMaskGroup);

	while (!IsEOList(pes))
	{
		if (pes == ei->Event)
		{
			wLastEventIndex = i;

			TU_DrawMode = DMODE_COLOR1;
			SetPointerMode();
			return TRUE;
		}

		i++;
		pes = Next(pes);
	}

	return FALSE;
} /* TileMaskEvent */


/*********************************************************************
 *
 * DisplayMaskEvent
 *
 * SYNOPSIS
 *		short DisplayMaskEvent(EventInfo *ei)
 *
 * PURPOSE
 *		This routine deals with all the events defined the
 *		[Tile Mask Display Events] in the .INI file. All such events call this
 *		routine. This routine figures out which event generated the call by
 *		searching forward from pFirstEvent[wDisplayMaskGroup] until it encounters
 *		the same event. It then uses this event number to index the arrays
 *		ubDisplayANDMask.
 *		It then updates <ubGlobalShowFlagAND>.
 *
 * INPUT
 *		<ei> which describes which event occurred.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful, FALSE otherwise.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short DisplayMaskEvent(EventInfo *ei)
{
	EventState *pes;
	WORD i = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DisplayMaskEvent";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ei = ei;										/* Disable warning */

	pes = GetFirstEvent(wDisplayMaskGroup);

	ubGlobalShowFlagAND = 0xFF;
	ubGlobalShowFlagOR = 0x00;
	ubGlobalShowFlagXOR = 0x00;

	while (!IsEOList(pes))
	{
		if (pes->Function != DisplayMaskEvent)
			break;								/* Processed all DisplayMaskEvents */

		if (fDisplayMasking[i])
		{
			ubGlobalShowFlagAND &= ubDisplayANDMask[i];
			ubGlobalShowFlagOR |= ubDisplayORMask[i];
			ubGlobalShowFlagXOR |= ubDisplayXORMask[i];
		}
		i++;
		pes = Next(pes);
	}
	ShowRoom (GlobalRoomWindow);
	return TRUE;

} /* DisplayMaskEvent */


/*********************************************************************
 *
 * MoveCursorEvent
 *
 * PURPOSE
 *		This routine deals with all the events defined the
 *		[Cursor Movement Events] in the .INI file. All such events call this
 *		routine. This routine figures out which event generated the call by
 *		searching forward from pFirstEvent[wCursorGroup] until it encounters
 *		the same event. It then uses this event number to index the arrays
 *		wMoveCursorX, & wMoveCursorY. Pass these values to MoveRequest().
 *
 * INPUT
 *		<ei> which describes which event occurred.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful, FALSE otherwise.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short MoveCursorEvent(EventInfo *ei)
{
	EventState *pes;
	WORD i = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MoveCursorEvent";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pes = GetFirstEvent(wCursorGroup);

	while (!IsEOList(pes))
	{
		if (pes == ei->Event)
		{
			MoveRequest(wMoveCursorX[i], wMoveCursorY[i]);
			return TRUE;
		}

		i++;
		pes = Next(pes);
	}

	return FALSE;
} /* MoveCursorEvent */


/*********************************************************************
 *
 * ShiftLayerEvent
 *
 * PURPOSE
 *		This routine deals with all the events defined the
 *		[Shift Layer Events] in the .INI file. All such events call this
 *		routine. This routine figures out which event generated the call by
 *		searching forward from pFirstEvent[wShiftLayerGroup] until it encounters
 *		the same event. It then uses this event number to index the arrays
 *		wShiftLayerX, & wShiftLayerY. Pass these values to ShiftLayer().
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *		02/20/94 Sunday (dcc) - created.
 *
*/
short ShiftLayerEvent(EventInfo *ei)
{
	EventState *pes;
	WORD i = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShiftLayerEvent";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pes = GetFirstEvent(wShiftLayerGroup);

	while (!IsEOList(pes))
	{
		if (pes == ei->Event)
		{
			ShiftLayer(wShiftLayerX[i], wShiftLayerY[i]);
			return TRUE;
		}

		i++;
		pes = Next(pes);
	}

	return FALSE;
} /* ShiftLayerEvent */


/*********************************************************************
 *
 * SetZoomEvent
 *
 * SYNOPSIS
 *		short SetZoomEvent(EventInfo *ei)
 *
 * PURPOSE
 *		This routine deals with all the events defined the
 *		[Zoom Events] in the .INI file. All such events call this
 *		routine This routine figures out which event generated the call by
 *		searching forward from pFirstEvent[wZoomGroup] until it encounters
 *		the same event. It then saves this number in the currently visible
 *		room's->wZoomIndex variable.
 *
 * INPUT
 *		<ei> which describes which event occurred.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful, FALSE otherwise.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short SetZoomEvent(EventInfo *ei)
{

	EventState *pes;
	WORD i = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetZoomEvent";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pes = GetFirstEvent(wZoomGroup);

	while (!IsEOList(pes))
	{
		if (pes == ei->Event)
		{
			if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom &&
				 GlobalRoomWindow->CurrentRoom->Room)
			{
				int	wx, wy;

				GetCursorTilePosition (&wx, &wy, GlobalRoomWindow);

				GlobalRoomWindow->CurrentRoom->Room->wZoomIndex = i;
				SetZoomOn(GlobalRoomWindow->CurrentRoom->Room);
				SetGlobalZoom(GlobalRoomWindow->CurrentRoom->Room);
				MoveCenterTo(wx, wy, FALSE);
			}
			return TRUE;
		}

		i++;
		pes = Next(pes);
	}

	return FALSE;
} /* SetZoomEvent */


/*********************************************************************
 *
 * GetZoomMax
 *
 * SYNOPSIS
 *		WORD GetZoomMax(void)
 *
 * PURPOSE
 *		Return number of entries in zoom [].
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
WORD GetZoomMax(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetZoomMax";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return wMaxZooms;
} /* GetZoomMax */


/*********************************************************************
 *
 * wGetZoomDstDup
 *
 * SYNOPSIS
 *		WORD wGetZoomDstDup(WORD w)
 *
 * PURPOSE
 *		Get wZoomDstDup[] for index <w>.
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
WORD wGetZoomDstDup(WORD w)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "wGetZoomDstDup";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return wZoomDstDup[w];

} /* wGetZoomDstDup */


/*********************************************************************
 *
 * wGetZoomSrcSkip
 *
 * SYNOPSIS
 *		WORD wGetZoomSrcSkip(WORD w)
 *
 * PURPOSE
 *		Get wZoomSrcSkip[] for index <w>.
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
WORD wGetZoomSrcSkip(WORD w)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "wGetZoomSrcSkip";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return wZoomSrcSkip[w];

} /* wGetZoomSrcSkip */


/*********************************************************************
 *
 * AllocateDisplayMaskArrays
 *
 * SYNOPSIS
 *		BOOL AllocateDisplayMaskArrays(WORD wCnt)
 *
 * PURPOSE
 *		Allocate <wCnt> array of <ubDisplayANDMask>, <ubDisplayORMask>,
 *		and <ubDisplayXORMask>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL AllocateDisplayMaskArrays(WORD wCnt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AllocateDisplayMaskArrays";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (wCnt)
	{
		if ((ubDisplayANDMask = mallocx(wCnt)) == NULL)
			return FALSE;

		if ((ubDisplayORMask = mallocx(wCnt)) == NULL)
			return FALSE;

		if ((ubDisplayXORMask = mallocx(wCnt)) == NULL)
			return FALSE;

		if ((fDisplayMasking = callocx(wCnt, sizeof(short))) == NULL)
			return FALSE;
	}
	return TRUE;

} /* AllocateDisplayMaskArrays */


/*********************************************************************
 *
 * SaveDisplayMask
 *
 * SYNOPSIS
 *		void SaveDisplayMask(EventState *pes, WORD w, UBYTE ubANDMask, UBYTE ubORMask, UBYTE ubXORMask)
 *
 * PURPOSE
 *		Save <ubANDMask> in ubDisplayANDMask[w], etc.
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
void SaveDisplayMask(EventState *pes, WORD w, UBYTE ubANDMask, UBYTE ubORMask, UBYTE ubXORMask)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveDisplayMask";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pes->StateVar = &fDisplayMasking[w];
	pes->Flags = MNU_HAS_CHECK;

	ubDisplayANDMask[w] = ubANDMask;
	ubDisplayORMask[w] = ubORMask;
	ubDisplayXORMask[w] = ubXORMask;
	fDisplayMasking[w] = FALSE;

} /* SaveDisplayMask */

