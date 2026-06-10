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
 * XPORTMAP.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 05/17/93
 *   MODIFIED : 03/26/95
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Export current room as an IFF PBM picture.
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include "tuglbl.h"
#include "tumedraw.h"
#include "xportmap.h"						/* Verify function prototypes. */

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * WriteBMHD
 *
 * PURPOSE
 *		Write BMHD chunk to file. Returns number of bytes written,
 *		or 0 if error.
 *
 *		//BUGBUG: assumes 256-colors!
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
static ULONG WriteBMHD(int hout, RoomWindowType *prw,
								LayerType *play, short ctilx, short ctily)
{
	RoomStuffType *prs;
	short fSep;
	short cxtilDst, cytilDst;
	LONG lcx, lcy;
	UWORD uw;
	ULONG ul;
	UBYTE ub;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteBMHD";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (write(hout, "BMHD", 4) != 4)
		return 0;

	prs = prw->CurrentRoom;

	fSep			= ((prs->Flags) & DISPLAY_SEPERATED);
	cxtilDst		= wSrcSizeToDstSize(play->cxTile, wSrcSkipX, wDstDupX) + fSep;
	cytilDst		= wSrcSizeToDstSize(play->cyTile, wSrcSkipY, wDstDupY) + fSep;

	lcx = (LONG) cxtilDst * ctilx;
	lcy = (LONG) cytilDst * ctily;

	ul = 20;				LongSex(ul);		/* length of BMHD chunk */
	if (write(hout, &ul, sizeof(ul)) != sizeof(ul))
		return 0;

	uw = (UWORD) lcx;	WordSex(uw);		/* width in pixels */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	uw = (UWORD) lcy;	WordSex(uw);		/* height in pixels */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	uw = 0;				WordSex(uw);		/* x pixel position */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	uw = 0;				WordSex(uw);		/* y pixel position */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	ub = 8;										/* # source bitplanes */
	if (write(hout, &ub, sizeof(ub)) != sizeof(ub))
		return 0;

	ub = 2;										/* masking = mskHasTransparentColor */
	if (write(hout, &ub, sizeof(ub)) != sizeof(ub))
		return 0;

	uw = 0;				WordSex(uw);		/* no compression */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	uw = GlobalBackground;	WordSex(uw);	/* transparent color */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	ub = 10;										/* x aspect ratio */
	if (write(hout, &ub, sizeof(ub)) != sizeof(ub))
		return 0;

	ub = 11;										/* y aspect ratio */
	if (write(hout, &ub, sizeof(ub)) != sizeof(ub))
		return 0;

	uw = (UWORD) lcx;	WordSex(uw);		/* source width in pixels */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	uw = (UWORD) lcy;	WordSex(uw);		/* source height in pixels */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	return 28;									/* number of bytes written */
} /* WriteBMHD */


/*********************************************************************
 *
 * WriteLayersAsBODY
 *
 * PURPOSE
 *		Export the current room as an PBM body.
 *
 * INPUT
 *		hout:			file to write to
 *		prw:			RoomWindowType of room to write
 *		pLayers		: linked list of layers to write
 *		playFloor:	show only this layer if EditOnlyFloorSTATE is TRUE
 *		ctilx:		tiles across to save (starting from 0)
 *		ctily:		tiles down to save (starting from 0)
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns number of bytes written, or 0 if error.
 *
 * HISTORY
 *		05/30/93 Sunday (dcc) - add fShowGuide & fCompositeSource flags.
 *		08/17/93 Tuesday (dcc) - fix passing 0 for <cx> & <cy> to ShowLayersInSubRect()
 *		03/26/95 (dcc) - Make sure FindNonEmptyLayer() returns != NULL.
 *
 * SEE ALSO
 *
*/
static ULONG WriteLayersAsBODY(int hout, RoomWindowType *prw,
							ListType *pLayers, LayerType *playFloor,
							short ctilx, short ctily,
							BOOL fShowGuide, BOOL fCompositeSource)
{
	RoomStuffType *prs;
	LayerType *play;
	short fSep;
	short cxtilDst, cytilDst;
	LONG lcx, lcy;
	UWORD ctilPerSeg;
	short ytil, y, xtil;
	ULONG ul;
	ByteMap bm;

	BOOL fAbort = FALSE;
	UBYTE *pub = NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteLayersAsBODY";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	play = FindNonEmptyLayer(pLayers, playFloor);
	if (!play)
		return 0;

	if (write(hout, "BODY", 4) != 4)
		return 0;

	prs = prw->CurrentRoom;

	fSep			= ((prs->Flags) & DISPLAY_SEPERATED);
	cxtilDst		= wSrcSizeToDstSize(play->cxTile, wSrcSkipX, wDstDupX) + fSep;
	cytilDst		= wSrcSizeToDstSize(play->cyTile, wSrcSkipY, wDstDupY) + fSep;

	lcx = (LONG) cxtilDst * ctilx;
	lcy = (LONG) cytilDst * ctily;

	if (lcx & 1)					/* Pad each row of pixels to a word boundary. */
		lcx++;
{
	ul = (ULONG) lcx * lcy;	LongSex(ul);

	if (write(hout, &ul, sizeof(ul)) != sizeof(ul))
		return 0;

	LongSex(ul);
}
{
	ctilPerSeg = 32767U / cxtilDst;

	/* ASSERT: ctilPerSeg had better be > 0 at this point! */

	while (ctilPerSeg && (pub = MEM_calloc(ctilPerSeg, cxtilDst)) == NULL)
	{
		ctilPerSeg /= 2;
	}
	if (!pub)
		return 0;			/* Error: unable to allocate buffer */
}
{
	bm.width					= ctilPerSeg * cxtilDst;
	bm.height				= 1;
	bm.data					= pub;
	bm.transparentColor	= 0;
}

	PushClipValues(0, 0, bm.width, bm.height);

	for (ytil = 0; ytil < ctily && !fAbort; ytil++)
	{
		for (y = 0; y < cytilDst && !fAbort; y++)
		{
			for (xtil = 0; xtil < ctilx && !fAbort; xtil += ctilPerSeg)
			{
				UWORD cx = min(ctilPerSeg * cxtilDst, (ctilx-xtil) * cxtilDst);

				memset(pub, GlobalBackground, cx);

				ShowLayersInSubRect(pLayers, playFloor,
											prw, &bm,
											0, -y, bm.width, bm.height,
											xtil, ytil,
											ctilPerSeg, 1, fShowGuide, fCompositeSource);

				if (write(hout, pub, cx) != cx)
				{
/**/				goto ABORT;
				}

				/* Pad each row of pixels to a word boundary. */

				if (cx & 1)
				{
					UBYTE ub = 0;

					if (write(hout, &ub, 1) != 1)
					{
/**/					goto ABORT;
					}
				}
			}
		}
	}
	PopClipValues();

	MEM_free(pub);
	return ul + 8;
ABORT:
	MEM_free(pub);
	return 0;

} /* WriteLayersAsBODY */


/*********************************************************************
 *
 * WritePBM
 *
 * PURPOSE
 *		Write PBM chunk to file. Returns number of bytes written, or
 *		0 if error.
 *
 *		//BUGBUG: assumes 256-colors!
 *
 * INPUT
 *		hout:			file to write to
 *		prw:			RoomWindowType of room to write
 *		Colors:		room colors
 *		pLayers		: linked list of layers to write
 *		playFloor:	show only this layer if EditOnlyFloorSTATE is TRUE
 *		ctilx:		tiles across to save (starting from 0)
 *		ctily:		tiles down to save (starting from 0)
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns number of bytes written, or 0 if error.
 *
 * HISTORY
 *		05/30/93 (dcc) - Add fShowGuide & fCompositeSource flags.
 *		03/26/95 (dcc) - Make sure FindNonEmptyLayer() returns != NULL.
 *
 * SEE ALSO
 *
*/
static ULONG WritePBM(int hout, RoomWindowType *prw, CS_Color *Colors,
							ListType *pLayers, LayerType *playFloor,
							short ctilx, short ctily,
							BOOL fShowGuide, BOOL fCompositeSource)
{
	ULONG ulResult;
	ULONG ulLength = 0;
	LayerType *play;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WritePBM";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	play = FindNonEmptyLayer(pLayers, playFloor);
	if (!play)
		return 0;

	if ((ulLength = write(hout, "PBM ", 4)) != 4)
		return 0;

	if ((ulResult = WriteBMHD(hout, prw, play, ctilx, ctily)) == 0)
		return 0;

	ulLength += ulResult;

	if ((ulResult = WriteCMAP(hout, 256, Colors)) == 0)
		return 0;

	ulLength += ulResult;

	if ((ulResult = WriteLayersAsBODY(hout, prw, pLayers, playFloor,
												ctilx, ctily,
												fShowGuide, fCompositeSource)) == 0)
		return 0;

	return ulLength + ulResult;

} /* WritePBM */


/*********************************************************************
 *
 * SaveLayersAsByteMap
 *
 * PURPOSE
 *		Saves the current room as a bytemap.
 *
 *		//BUGBUG: assumes 256-colors!
 *
 * INPUT
 *		prw:			RoomWindowType of room to write
 *		Colors:		room colors
 *		path:			filename to write
 *		pLayers		: linked list of layers to write
 *		playFloor:	show only this layer if EditOnlyFloorSTATE is TRUE
 *		ctilx:		tiles across to save (starting from 0)
 *		ctily:		tiles down to save (starting from 0)
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		05/30/93 Sunday (dcc) - add fShowGuide & fCompositeSource flags.
 *
 * SEE ALSO
 *
*/
BOOL SaveLayersAsByteMap(RoomWindowType *prw, CS_Color *Colors, char *path,
							ListType *pLayers, LayerType *playFloor,
							short ctilx, short ctily,
							BOOL fShowGuide, BOOL fCompositeSource)
{
	int hout;
	ULONG ulLength = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveLayersAsByteMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((hout = open(path, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, S_IREAD|S_IWRITE)) == -1)
		return FALSE;

	if (write(hout, "FORM    ", 8) != 8)
/**/	goto ABORT;

	if ((ulLength = WritePBM(hout, prw, Colors,
									pLayers, playFloor, ctilx, ctily,
									fShowGuide, fCompositeSource)) == 0)
/**/	goto ABORT;
		
	/* Go back and fill in length of PBM chunk. */

	if (lseek(hout, 4, SEEK_SET) != 4)
/**/	goto ABORT;

	LongSex(ulLength);

	if (write(hout, &ulLength, sizeof(ulLength)) != sizeof(ulLength))
/**/	goto ABORT;

	if (!close(hout))
		return TRUE;
	else
		return FALSE;

ABORT:
	close(hout);
	return FALSE;

} /* SaveLayersAsByteMap */

