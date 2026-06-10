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
 * MSDOS.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 05/11/91
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
 *		MS-DOS specific functions for tUME.
 *
 * HISTORY
 *		Chapter VII. The young Jedi Knight, Danno Skywalker, has just finished
 *		the latest version of tUME. Suddenly, the phone rings. It is an uSER.
 *		"Young Danno, the rebel alliance needs your assistance in eradicating
 *		a new tUME bug. Princess INeedMore requests another minor new addition."
*/

#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "switch1.h"

#include <echidna/etypes.h>
#include <echidna/brushsup.h>
#include <echidna/eerrors.h>
#include <echidna/emacros.h>
#include <echidna/eui.h>
#include <echidna/grafx.h>
#include <echidna/mouse.h>

#include <echidna/readiff.h>
#include "mcopyrec.h"
#include "mitems.h"
#include "events.h"
#include "tuglbl.h"
#include "tilebits.h"
#include "tume.h"
#include "tumedraw.h"

/***************************** E X T E R N A L ****************************/


/**************************** C O N S T A N T S ***************************/

#define COLORREGSIZE	sizeofColorRegister

int MouseXHotSpot[] =
{
	 0,
	 0,
	 0,
	-6,
	 0
};

int MouseYHotSpot[] =
{
	 0,
	 0,
	 0,
	-6,
	 0
};

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

int MouseX			= 0;
int MouseY			= 0;
int LeftButton		= FALSE;
int RightButton	= FALSE;

static BPIType		*MouseBPI = NULL;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


#if NEWTUME
/*********************************************************************
 *
 * XORDrawLine
 *
 * PURPOSE
 *		XOR draw w/ <ub> a line into <pbm> from (x0, y0) to (x1, y1).
 *
 *		We implement stupid clipping, base on this assumption: since
 *		we are drawing a straight line, it will appear in the viewable
 *		area only once.
 *
 *		Thus, if we haven't drawn any pixels, but we are out of bounds,
 *		we'll keep trying to draw pixels.
 *
 *		Once we've drawn one pixel, and we step out of bounds, we'll stop.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		09/08/93 Wednesday (dcc) - created from DrawLine
 *		09/09/93 Thursday (dcc) - use GetClipTop(), etc.
 *
 * SEE ALSO
 *
*/
void XORDrawLine (struct ByteMap *pbm,
						int x0, int y0, int x1, int y1, UBYTE ub)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "XORDrawLine";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		UBYTE *pmap;

		int dxClipL = max(GetClipLeft(), 0);
		int dyClipT = max(GetClipTop(), 0);
		int dxClipR = min((GetClipLeft() + GetClipWidth()), pbm->width);
		int dyClipB = min((GetClipTop() + GetClipHeight()), pbm->height);
		int	dx;
		int	dy;
		int	xdir;
		int	ydir;
		int	ych;
		int	incrE;
		int	incrNE;
		int	d;
		int	x;
		int	y;
		BOOL	fDrawnOne = FALSE;

		dx   = x1 - x0;
		xdir = sign (dx);
		dx   = abs (dx);

		dy   = y1 - y0;
		ydir = sign (dy);
		ych  = ydir * pbm->width;
		dy   = abs (dy);

		if (dx > dy) {
			d  = dy + dy - dx;

			incrE  = dy + dy;
			incrNE = 2 * (dy - dx);

			x = x0;
			y = y0;
			pmap = pbm->data + y0 * pbm->width + x0;
			if (dxClipL <= x && x < dxClipR &&
				 dyClipT <= y && y < dyClipB)
			{
				*pmap ^= ub;
				fDrawnOne = TRUE;
			}

			while ( x < x1 ) {
				if (d <= 0) {
					d += incrE;
					x += xdir;
					pmap += xdir;
				} else {
					d += incrNE;
					x += xdir;
					y += ydir;
					pmap += ych + xdir;
				}
				if (dxClipL <= x && x < dxClipR &&
					 dyClipT <= y && y < dyClipB)
				{
					*pmap ^= ub;
					fDrawnOne = TRUE;
				}
				else
				{
					if (fDrawnOne)
						return;
				}
			}
		} else {
			d  = dx + dx - dy;

			incrE  = dx + dx;
			incrNE = 2 * (dx - dy);

			x = x0;
			y = y0;
			pmap = pbm->data + y0 * pbm->width + x0;
			if (dxClipL <= x && x < dxClipR &&
				 dyClipT <= y && y < dyClipB)
			{
				*pmap ^= ub;
				fDrawnOne = TRUE;
			}

			while ( y < y1 ) {
				if (d <= 0) {
					d += incrE;
					y += ydir;
					pmap += ych;
				} else {
					d += incrNE;
					y += ydir;
					x += xdir;
					pmap += ych + xdir;
				}
				if (dxClipL <= x && x < dxClipR &&
					 dyClipT <= y && y < dyClipB)
				{
					*pmap ^= ub;
					fDrawnOne = TRUE;
				}
				else
				{
					if (fDrawnOne)
						return;
				}
			}
		}
	}
} /* XORDrawLine */
#else
/* XORvLine - exclusive-or draw vertical line from x1,y1 to x1,y2
	with value ub. */

void XORvLine(struct ByteMap *bytemap, int x1, int y1, int y2, UBYTE ub)
{
	UBYTE *mapPtr;

	if (x1 < 0 || x1 >= bytemap->width)
		return;

	if (y1 > y2)
	{
		int i;

		i = y2;
		y2 = y1;
		y1 = i;
	}
	if (y1 < 0)
		y1 = 0;
	if (y2 >= bytemap->height)
		y2 = bytemap->height-1;

	mapPtr = bytemap->data + y1 * bytemap->width + x1;

	for (;y1<=y2;y1++) {
		*mapPtr ^= ub;
		mapPtr += bytemap->width;
	}
}

/* XORhLine - exclusive-or draw horizontal line from x1,y1 to x2,y1
	with value ub. */

void XORhLine(struct ByteMap *bytemap, int x1, int x2, int y1, UBYTE ub)
{
	UBYTE *mapPtr;

	if (y1 < 0 || y1 >= bytemap->height)
		return;

	if (x1 > x2)
	{
		int i;

		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (x1 < 0)
		x1 = 0;
	if (x2 >= bytemap->width)
		x2 = bytemap->width-1;

	mapPtr = bytemap->data + y1 * bytemap->width + x1;

	for (;x1<=x2;x1++) {
		*mapPtr++ ^= ub;
	}
}
#endif


/* ReadPixel - returns the pixel value at coordinate <xp, yp> from
	BigByteMap <pbbm>. */

int ReadPixel(BigByteMap *pbbm, int xp, int yp)
{
	UBYTE ubPixel = 0;

	ReadXTRA(pbbm->mpYpMpXpPix[yp], xp, sizeof(UBYTE), &ubPixel);

	return ubPixel;
}

/* isByteMapTransparent - returns TRUE if ByteMap <bm>
	is completely transparent. */

BOOL isByteMapTransparent(ByteMap *bm)
{
	int i, j;
	UBYTE *srcPtr;
	register char transparentColor = bm->transparentColor;

	srcPtr = bm->data;

	for (i = 0; i < bm->height; i++)
	{
		for (j = 0; j < bm->width; j++)
		{
			if (*(srcPtr+j) != transparentColor)
			{
				return FALSE;
			}
		}
		srcPtr += bm->width;
	}
	return TRUE;
}

#if 0
/* transparentRow - return TRUE if entire row <y> of bytemap is transparent. */

static int transparentRow(struct ByteMap *bytemap, int x, int y, int width)
{
	int i;

	for (i=0; i<width; i++) {
		if ((unsigned char) *(bytemap->data+y*bytemap->width+x+i) != bytemap->transparentColor)
			return FALSE;
	}
	return TRUE;
}

/* transparentColumn - return TRUE if entire column <x> of bytemape is transparent. */

static int transparentColumn(struct ByteMap *bytemap, int x, int y, int height)
{
	int i;

	for (i=0; i<height; i++) {
		if ((unsigned char) *(bytemap->data+(y+i)*bytemap->width+x) != bytemap->transparentColor)
			return FALSE;
	}
	return TRUE;
}
#endif

/* WriteBMHD - write BMHD chunk to file. Return number of bytes written,
	or 0 if error. */

static ULONG WriteBMHD(int hout, ByteMap *bm, UBYTE ubDepth)
{
	UWORD uw;
	ULONG ul;
	UBYTE ub;

	if (write(hout, "BMHD", 4) != 4)
		return 0;

	ul = 20;				LongSex(ul);		/* length of BMHD chunk */
	if (write(hout, &ul, sizeof(ul)) != sizeof(ul))
		return 0;

	uw = bm->width;	WordSex(uw);		/* width in pixels */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	uw = bm->height;	WordSex(uw);		/* height in pixels */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	uw = 0;				WordSex(uw);		/* x pixel position */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	uw = 0;				WordSex(uw);		/* y pixel position */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

													/* # source bitplanes */
	if (write(hout, &ubDepth, sizeof(ubDepth)) != sizeof(ubDepth))
		return 0;

	ub = 2;										/* masking = mskHasTransparentColor */
	if (write(hout, &ub, sizeof(ub)) != sizeof(ub))
		return 0;

	uw = 0;				WordSex(uw);		/* no compression */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	uw = bm->transparentColor;	WordSex(uw);	/* transparent color */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	ub = 10;										/* x aspect ratio */
	if (write(hout, &ub, sizeof(ub)) != sizeof(ub))
		return 0;

	ub = 11;										/* y aspect ratio */
	if (write(hout, &ub, sizeof(ub)) != sizeof(ub))
		return 0;

	uw = bm->width;	WordSex(uw);		/* source width in pixels */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	uw = bm->height;	WordSex(uw);		/* source height in pixels */
	if (write(hout, &uw, sizeof(uw)) != sizeof(uw))
		return 0;

	return 28;									/* number of bytes written */
}

/* WriteCMAP - write CMAP chunk to file. Return number of bytes written,
	or 0 if error. */

ULONG WriteCMAP(int hout, UWORD uwNumberColors, CS_Color *Colors)
{
	UWORD uw;
	ULONG ul;
	ColorRegister	out_color;

	if (write(hout, "CMAP", 4) != 4)
		return 0;

	ul = uwNumberColors * COLORREGSIZE;	LongSex(ul);	/* length of BMHD chunk */
	if (write(hout, &ul, sizeof(ul)) != sizeof(ul))
		return 0;
	LongSex(ul);

	for (uw = 0; uw < uwNumberColors; uw++)
	{
#if 0
		out_color.red = (Colors+uw)->Red * 255 / 63;
		out_color.green = (Colors+uw)->Green * 255 / 63;
		out_color.blue = (Colors+uw)->Blue * 255 / 63;
#endif
		out_color.red = (Colors+uw)->Red;
		out_color.green = (Colors+uw)->Green;
		out_color.blue = (Colors+uw)->Blue;

		if (write(hout, (void *) &out_color, COLORREGSIZE) != COLORREGSIZE)
			return 0;
	}

	return ul + 8;
}

/* WriteBODY - write actual picture pixel data. Return number of bytes
	written or 0 if error. */

static ULONG WriteBODY(int hout, ByteMap *bm, UBYTE ubDepth)
{
	UWORD			bytesperline;
	UWORD plane, scanline, bytecount;
	UWORD pixelcount;
	ULONG ul;
	UBYTE pixels[256];

	if (write(hout, "BODY", 4) != 4)
		return 0;

	bytesperline = ((bm->width + 15) / 16) * 2;	/* pad to nearest word */

	ul = (ULONG) bytesperline * bm->height * ubDepth;	LongSex(ul);
	if (write(hout, &ul, sizeof(ul)) != sizeof(ul))
		return 0;
	LongSex(ul);


	for (scanline = 0; scanline < bm->height; scanline++)	/* every line */
	{
		for (plane = 0; plane < ubDepth; plane++)	/* every plane */
		{
			for (bytecount = 0; bytecount < bytesperline; bytecount++)	/* every byte */
			{
				register UWORD StartofLine = scanline * bm->width;
				register UBYTE pixel = 0;

				for (pixelcount = 0; pixelcount < 8; pixelcount++)	/* every pixel */
				{
					pixel <<= 1;
					if (bytecount * 8 + pixelcount < bm->width)
					{
						pixel |= (*(bm->data + StartofLine
							+ bytecount * 8 + pixelcount) & (1 << plane)) >> plane;
					}
				}
				pixels[bytecount] = pixel;
			}
			if (write (hout, pixels, bytesperline) != bytesperline)
				return 0;
		}
	}

	return ul + 8;
}

/* WriteILBM - write ILBM chunk to file. Return number of bytes written,
	or 0 if error. */

static ULONG WriteILBM(int hout, ByteMap *bm,
							UBYTE ubDepth, UWORD uwNumberColors, CS_Color *Colors)
{
	ULONG ulResult;
	ULONG ulLength = 0;

	if ((ulLength = write(hout, "ILBM", 4)) != 4)
		return 0;

	if ((ulResult = WriteBMHD(hout, bm, ubDepth)) == 0)
		return 0;

	ulLength += ulResult;

	if ((ulResult = WriteCMAP(hout, uwNumberColors, Colors)) == 0)
		return 0;

	ulLength += ulResult;

	if ((ulResult = WriteBODY(hout, bm, ubDepth)) == 0)
		return 0;

	return ulLength + ulResult;
}

/* SaveByteMap - save raw bytemap with header information
	in IFF format. */

BOOL SaveByteMap(ByteMap *bm, UBYTE ubDepth,
					UWORD uwNumberColors, CS_Color *Colors, char *path)
{
	int hout;
	ULONG ulLength = 0;

	if ((hout = open(path, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, S_IREAD|S_IWRITE)) == -1)
		return FALSE;

	if (write(hout, "FORM    ", 8) != 8)
/**/	goto ABORT;

	if ((ulLength = WriteILBM(hout, bm, ubDepth, uwNumberColors, Colors)) == 0)
/**/	goto ABORT;
		
	/* Go back and fill in length of ILBM chunk. */

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
}

/* CopyBigRectToSmall - CopyRect that works from a BigByteMap to
	a regular ByteMap. */

void CopyBigRectToSmall(BigByteMap *pbbmSrc, int xpSrc, int ypSrc,
							ByteMap *pbmDst, int xpDst, int ypDst,
							int xpWidth, int ypHeight)
{
	if (xpDst >= (int) pbmDst->width || xpDst >= GetClipLeft() + GetClipWidth())
		return;
	if (ypDst >= (int) pbmDst->height || ypDst >= GetClipTop() + GetClipHeight())
		return;
	if (xpDst < GetClipLeft())
	{
		xpWidth -= GetClipLeft() - xpDst;
		xpSrc += GetClipLeft() - xpDst;
		xpDst = GetClipLeft();
	}
	if (ypDst < GetClipTop())
	{
		ypHeight -= GetClipTop() - ypDst;
		ypSrc += GetClipTop() - ypDst;
		ypDst = GetClipTop();
	}

	/* Clip width and height to copy.
		Need (int) cast so math will generate negative numbers. */

	xpWidth = min(xpWidth, (int) pbmDst->width - xpDst);
	ypHeight = min(ypHeight, (int) pbmDst->height - ypDst);

	xpWidth = min(xpWidth, GetClipLeft() + GetClipWidth() - xpDst);
	ypHeight = min(ypHeight, GetClipTop() + GetClipHeight() - ypDst);

	if (xpSrc < 0)
	{
		xpDst -= xpSrc;
		xpWidth += xpSrc;
		xpSrc = 0;
	}
	if (ypSrc < 0)
	{
		ypDst -= ypSrc;
		ypHeight += ypSrc;
		ypSrc = 0;
	}
	xpWidth = min(xpWidth, (int) pbbmSrc->width - xpSrc);
	ypHeight = min(ypHeight, (int) pbbmSrc->height - ypSrc);

	if (xpWidth > 0 && ypHeight > 0)
	{
		int iyp;

		for (iyp = 0; iyp < ypHeight; iyp++)
		{
			ReadXTRA(pbbmSrc->mpYpMpXpPix[iyp+ypSrc], xpSrc, xpWidth,
						pbmDst->data + (iyp+ypDst) * pbmDst->width + xpDst);
		}
	}
}

/* CopySmallRectToBig - CopyRect that works from a regular ByteMap to
	a BigByteMap.
	BUGBUG: This routine does not deal with pbbmDst width > 32767!
	(Actually, if you look at the definition of BigByteMap.width, it
	is defined as a short. So, this makes sense. But note that
	ByteMap.width is defined as an UWORD. Go figure. */

void CopySmallRectToBig(ByteMap *pbmSrc, int xpSrc, int ypSrc,
							BigByteMap *pbbmDst, int xpDst, int ypDst,
							int xpWidth, int ypHeight)
{
	if (xpDst >= (int) pbbmDst->width || xpDst >= GetClipLeft() + GetClipWidth())
		return;
	if (ypDst >= (int) pbbmDst->height || ypDst >= GetClipTop() + GetClipHeight())
		return;
	if (xpDst < GetClipLeft())
	{
		xpWidth -= GetClipLeft() - xpDst;
		xpSrc += GetClipLeft() - xpDst;
		xpDst = GetClipLeft();
	}
	if (ypDst < GetClipTop())
	{
		ypHeight -= GetClipTop() - ypDst;
		ypSrc += GetClipTop() - ypDst;
		ypDst = GetClipTop();
	}

	/* Clip width and height to copy.
		Need (int) cast so math will generate negative numbers. */

	xpWidth = min(xpWidth, (int) pbbmDst->width - xpDst);
	ypHeight = min(ypHeight, (int) pbbmDst->height - ypDst);

	xpWidth = min(xpWidth, GetClipLeft() + GetClipWidth() - xpDst);
	ypHeight = min(ypHeight, GetClipTop() + GetClipHeight() - ypDst);

	if (xpSrc < 0)
	{
		xpDst -= xpSrc;
		xpWidth += xpSrc;
		xpSrc = 0;
	}
	if (ypSrc < 0)
	{
		ypDst -= ypSrc;
		ypHeight += ypSrc;
		ypSrc = 0;
	}
	xpWidth = min(xpWidth, (int) pbmSrc->width - xpSrc);
	ypHeight = min(ypHeight, (int) pbmSrc->height - ypSrc);

	if (xpWidth > 0 && ypHeight > 0)
	{
		int iyp;

		for (iyp = 0; iyp < ypHeight; iyp++)
		{
			WriteXTRA(pbmSrc->data + (iyp+ypSrc) * pbmSrc->width + xpSrc, xpWidth,
						 pbbmDst->mpYpMpXpPix[iyp+ypDst], xpDst);
		}
	}
}

/* CopyDisplayToBack - copies a rectangular pixel region of size
	<xpWidth>, <ypHeight> from location <xp>, <yp> either from back buffer
	to foreground buffer or from foreground buffer to back buffer based on
	<direction>. */

void CopyDisplayToBack(int xp, int yp, int xpWidth, int ypHeight,
								int direction)
{
	BeforeGraphics();

	if (direction == DISPLAYTOBACK)
		CopySmallRectToBig(DisplayBM, xp, yp, BigBackBM, xp, yp,
						xpWidth, ypHeight);
	else
		CopyBigRectToSmall(BigBackBM, xp, yp, DisplayBM, xp, yp,
						xpWidth, ypHeight);

	AfterGraphics();
}


#if PixelLayers
/* CopyDisplayTilesToBack - copies a rectangular tile region of size
	<width>, <height> from tile location <column>, <row> either from
	back buffer to foreground buffer or from foreground buffer to back
	buffer based on <direction>.
 *
 * HISTORY
 *		03/11/92 (dcc) - Support added for locked composite rooms.
 *		08/24/94 (dcc) - Add support for object tiles KLUDGED BUGBUG FIXME.
 *		09/07/94 (dcc) - Fix computation of xpWidth & ypHeight.
 *		03/26/95 (dcc) - Add check to ABORT on NULL <play>.
 *
*/
void CopyDisplayTilesToBack(RoomWindowType *roomwindow, int column, int row,
								int width, int height, int direction)
{
	{
		int xp, yp, xpWidth, ypHeight, tilewidth, tileheight;
		int			sep;
		int			wDispWidth, wDispHeight;
		RoomType		*prm;
		LayerType	*play;
		int cxTile, cyTile;

		sep = 0 + ((roomwindow->CurrentRoom->Flags) & (DISPLAY_SEPERATED));
		prm = roomwindow->CurrentRoom->Room;
		if (prm)
			play = FindBiggestLayer(&prm->Layers);
		else
			play = NULL;

		if (play)
		{
			if (width)
				width = min(prm->FloorLayer->ctilx - column, width);
			else
				width = prm->FloorLayer->ctilx - column;

			if (height)
				height = min(prm->FloorLayer->ctily - row, height);
			else
				height = prm->FloorLayer->ctily - row;
		}

		if (!play || (tilewidth = play->cxTile) == 0)
		{
			LayerType *play;

			if (FSelectedPblk(GlobalBlockCopy))
			{
				play = FindBiggestLayer(&GlobalBlockCopy->Layers);
			}
			else
			{
				prm = roomwindow->SourceRoom.Room;
				play = FindBiggestLayer(&prm->Layers);
			}
			if (!play)
				return;							// ABORT
			tilewidth = play->cxTile;
		}
		if (!play || (tileheight = play->cyTile) == 0)
		{
			LayerType *play;

			if (FSelectedPblk(GlobalBlockCopy))
			{
				play = FindBiggestLayer(&GlobalBlockCopy->Layers);
			}
			else
			{
				prm = roomwindow->SourceRoom.Room;
				play = FindBiggestLayer(&prm->Layers);
			}
			if (!play)
				return;							// ABORT
			tileheight = play->cyTile;
		}

		wDispWidth  = wSrcSizeToDstSize(tilewidth,  wSrcSkipX, wDstDupX) + sep;
		wDispHeight = wSrcSizeToDstSize(tileheight, wSrcSkipY, wDstDupY) + sep;

		if ((cxTile = prm->FloorLayer->cxTile) == 0)
			cxTile = GlobalBlockCopy->BC_FloorLayer->cxTile;

		if ((cyTile = prm->FloorLayer->cyTile) == 0)
			cyTile = GlobalBlockCopy->BC_FloorLayer->cyTile;

		xp = (int) (((ULONG) column * cxTile - roomwindow->CurrentRoom->dxSrc) * wDispWidth  / tilewidth);
		yp = (int) (((ULONG) row    * cyTile - roomwindow->CurrentRoom->dySrc) * wDispHeight / tileheight) + TopAdd;

		xpWidth  = (int) (((ULONG) (column+width) * cxTile - roomwindow->CurrentRoom->dxSrc)
						* wDispWidth  / tilewidth)  - xp + sep;

		ypHeight = (int) (((ULONG) (row  +height) * cyTile - roomwindow->CurrentRoom->dySrc)
						* wDispHeight / tileheight) - yp + sep + TopAdd;

		if (cxTile == 1 && cyTile == 1)
			CopyDisplayToBack(xp-(cpixxObjectTile/2),
									yp-(cpixyObjectTile/2),
									xpWidth+cpixxObjectTile,
									ypHeight+cpixyObjectTile, direction); // KLUDGE BUGBUG FIXME
		else
			CopyDisplayToBack(xp, yp, xpWidth, ypHeight, direction);
	}
}
#else // !PixelLayers
/* CopyDisplayTilesToBack - copies a rectangular tile region of size
	<width>, <height> from tile location <column>, <row> either from
	back buffer to foreground buffer or from foreground buffer to back
	buffer based on <direction>.
 *
 * HISTORY
 *		03/11/92 (dcc) - Aupport added for locked composite rooms.
 *		03/26/95 (dcc) - Add check to ABORT on NULL <play>.
 *
*/
void CopyDisplayTilesToBack(RoomWindowType *roomwindow, int column, int row,
								int width, int height, int direction)
{
	{
		int xp, yp, xpWidth, ypHeight, tilewidth, tileheight;
		int			sep;
		int			wDispWidth, wDispHeight;
		RoomType		*prm;
		LayerType	*play;

		sep = 0 + ((roomwindow->CurrentRoom->Flags) & (DISPLAY_SEPERATED));
		prm = roomwindow->CurrentRoom->Room;
		if (prm)
			play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
		else
			play = NULL;

		if (play)
		{
			if (width)
				width = min(play->ctilx - column, width);
			else
				width = play->ctilx - column;

			if (height)
				height = min(play->ctily - row, height);
			else
				height = play->ctily - row;
		}

		if (!play || (tilewidth = play->cxTile) == 0)
		{
			LayerType *play;

			if (FSelectedPblk(GlobalBlockCopy))
			{
				play = FindNonEmptyLayer(&GlobalBlockCopy->Layers, GlobalBlockCopy->BC_FloorLayer);
			}
			else
			{
				prm = roomwindow->SourceRoom.Room;
				play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
			}
			if (!play)
				return;							// ABORT
			tilewidth = play->cxTile;
		}
		if (!play || (tileheight = play->cyTile) == 0)
		{
			LayerType *play;

			if (FSelectedPblk(GlobalBlockCopy))
			{
				play = FindNonEmptyLayer(&GlobalBlockCopy->Layers, GlobalBlockCopy->BC_FloorLayer);
			}
			else
			{
				prm = roomwindow->SourceRoom.Room;
				play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
			}
			if (!play)
				return;							// ABORT
			tileheight = play->cyTile;
		}

		wDispWidth  = wSrcSizeToDstSize(tilewidth,  wSrcSkipX, wDstDupX) + sep;
		wDispHeight = wSrcSizeToDstSize(tileheight, wSrcSkipY, wDstDupY) + sep;

		xp = wDispWidth  * (column - (int) (roomwindow->CurrentRoom->dxSrc / tilewidth));
		yp = wDispHeight * (row    - (int) (roomwindow->CurrentRoom->dySrc / tileheight)) + TopAdd;

		xpWidth  = wDispWidth  * width + sep;
		ypHeight = wDispHeight * height + sep;

		CopyDisplayToBack(xp, yp, xpWidth, ypHeight, direction);
	}
}
#endif // !PixelLayers


/*********************************************************************
 *
 * MCGA_ClippedMaskedCopyTransRect
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
void MCGA_ClippedMaskedCopyTransRect (
					ByteMap *sbm,
					int	 fx,
					int	 fy,
					ByteMap	*dbm,
					int	 tx,
					int	 ty,
					int	 w,
					int	 h
				)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MCGA_ClippedMaskedCopyTransRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/


	{
		if (tx >= (int) dbm->width || tx >= GetClipLeft() + GetClipWidth())
			return;
		if (ty >= (int) dbm->height || ty >= GetClipTop() + GetClipHeight())
			return;
		if (tx < GetClipLeft())
		{
			if (!(copyMode & XFLIPMASK))
			{
				fx += GetClipLeft() - tx;
			}
			w  -= GetClipLeft() - tx;
			tx  = GetClipLeft();
		}
		if (w > (int) dbm->width - tx)
		{
			if ((copyMode & XFLIPMASK))
			{
				fx += w - ((int) dbm->width - tx);
			}
			w  = (int) dbm->width - tx;
		}
		if (w > GetClipLeft() + GetClipWidth() - tx)
		{
			if ((copyMode & XFLIPMASK))
			{
				fx += w - (GetClipLeft() + GetClipWidth() - tx);
			}
			w = GetClipLeft() + GetClipWidth() - tx;
		}

		if (ty < GetClipTop())
		{
			if (!(copyMode & YFLIPMASK))
			{
				fy += GetClipTop() - ty;
			}
			h  -= GetClipTop() - ty;
			ty  = GetClipTop();
		}
		if (h > (int) dbm->height - ty)
		{
			if ((copyMode & YFLIPMASK))
			{
				fy += h - ((int) dbm->height - ty);
			}
			h  = (int) dbm->height - ty;
		}
		if (h > GetClipTop() + GetClipHeight() - ty)
		{
			if ((copyMode & YFLIPMASK))
			{
				fy += h - (GetClipTop() + GetClipHeight() - ty);
			}
			h = GetClipTop() + GetClipHeight() - ty;
		}

		if (fx < 0)
		{
			tx -= fx;
			w  += fx;
			fx  = 0;
		}
		if (w > (int) sbm->width - fx)
		{
			w  = (int) sbm->width - fx;
		}

		if (fy < 0)
		{
			ty -= fy;
			h  += fy;
			fy  = 0;
		}
		if (h > (int) sbm->height - fy)
		{
			h  = (int) sbm->height - fy;
		}

		if (w > 0 && h > 0)
		{
			MCGA_MaskedCopyTransRect (sbm, fx, fy, dbm, tx, ty, w, h);
		}
	}

} /* MCGA_ClippedMaskedCopyTransRect */


/*********************************************************************
 *
 * MCGA_ClippedScaledMaskedCopyTransRect
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
 *		09/23/93 Thursday (dcc) - fixed to handle wSrcSkipX & wDstDupX > 256
 *		10/14/93 Thursday (dcc) - fixed to use wSrcSizeToDstSizeWithMod()
 *											instead of wSrcSizeToDstSize()
 *
 * SEE ALSO
 *
*/
void MCGA_ClippedScaledMaskedCopyTransRect (
					ByteMap *sbm,
					int	 fx,
					int	 fy,
					ByteMap	*dbm,
					LONG	 tx,
					LONG	 ty,
					int	 w,
					int	 h,
					int	 wSrcSkipX,
					int	 wDstDupX,
					int	 wSrcSkipY,
					int	 wDstDupY
				)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MCGA_ClippedScaledMaskedCopyTransRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/


	{
		LONG dw;
		LONG dh;

		/* Do source clipping first */

		if (fx < 0)
		{
			tx -= (LONG) fx * wDstDupX;
			w  += fx;
			fx  = 0;
		}
		if (w > sbm->width - fx)
		{
			w = sbm->width - fx;
		}

		if (fy < 0)
		{
			ty -= (LONG) fy * wDstDupY;
			h  += fy;
			fy  = 0;
		}
		if (h > sbm->height - fy)
		{
			h = sbm->height - fy;
		}

		/* Find out destination width */

		dw = (LONG) wSrcSizeToDstSizeWithMod(w, wSrcSkipX, wDstDupX, (unsigned int) (tx % wSrcSkipX)) * wSrcSkipX;
		dh = (LONG) wSrcSizeToDstSizeWithMod(h, wSrcSkipY, wDstDupY, (unsigned int) (ty % wSrcSkipY)) * wSrcSkipY;

		/* Do destination clipping */

		if (tx >= (LONG) dbm->width * wSrcSkipX || tx >= (LONG) (GetClipLeft() + GetClipWidth()) * wSrcSkipX)
			return;
		if (ty >= (LONG) dbm->height * wSrcSkipY || ty >= (LONG) (GetClipTop() + GetClipHeight()) * wSrcSkipY)
			return;
		if (tx < (LONG) GetClipLeft() * wSrcSkipX)
		{
			if (!(copyMode & XFLIPMASK))
			{
				fx += (int) (((LONG) GetClipLeft() * wSrcSkipX - tx) / wDstDupX);
			}
			dw -= ((LONG) GetClipLeft() * wSrcSkipX - tx);
			tx  = (LONG) GetClipLeft() * wSrcSkipX;
		}
		if (dw > ((LONG) dbm->width * wSrcSkipX - tx))
		{
			if ((copyMode & XFLIPMASK))
			{
				fx += (int) ((dw - ((LONG) dbm->width * wSrcSkipX - tx)) / wDstDupX);
			}
			dw  = ((LONG) dbm->width * wSrcSkipX - tx);
		}
		if (dw > ((LONG) (GetClipLeft() + GetClipWidth()) * wSrcSkipX - tx))
		{
			if ((copyMode & XFLIPMASK))
			{
				fx += (int) ((dw - ((LONG) dbm->width * wSrcSkipX - tx)) / wDstDupX);
			}
			dw  = ((LONG) (GetClipLeft() + GetClipWidth()) * wSrcSkipX - tx);
		}

		if (ty < (LONG) GetClipTop() * wSrcSkipY)
		{
			if (!(copyMode & YFLIPMASK))
			{
				fy += (int) (((LONG) GetClipTop() * wSrcSkipY - ty) / wDstDupY);
			}
			dh -= ((LONG) GetClipTop() * wSrcSkipY - ty);
			ty  = (LONG) GetClipTop() * wSrcSkipY;
		}
		if (dh > ((LONG) dbm->height * wSrcSkipY - ty))
		{
			if ((copyMode & YFLIPMASK))
			{
				fy += (int) ((dh - ((LONG) dbm->height * wSrcSkipY  - ty)) / wDstDupY);
			}
			dh  = ((LONG) dbm->height * wSrcSkipY - ty);
		}
		if (dh > ((LONG) (GetClipTop() + GetClipHeight()) * wSrcSkipY - ty))
		{
			if ((copyMode & YFLIPMASK))
			{
				fy += (int) ((dh - ((LONG) dbm->height * wSrcSkipY - ty)) / wDstDupY);
			}
			dh = ((LONG) (GetClipTop() + GetClipHeight()) * wSrcSkipY - ty);
		}

		if (dw > 0L && dh > 0L)
		{
			MCGA_ScaledMaskedCopyTransRect (sbm, fx, fy, dbm, tx, ty,
										(int) (dw / wSrcSkipX), (int) (dh / wSrcSkipY),
										wSrcSkipX, wDstDupX, wSrcSkipY, wDstDupY);
		}
	}

} /* MCGA_ClippedScaledMaskedCopyTransRect */


/* BaseName - return pointer to filename only (instead of full path).
	This is the last part of the path, but may be the entire filename. */

char *BaseName(char *pathname)
{
	char *str;

	if ((str = strrchr(pathname, '\\')) != NULL)
		return str+1;
	if ((str = strrchr(pathname, ':')) != NULL)
		return str+1;
	return pathname;
}


/**************************************************************************
 *
 * SetFreqFilename
 *
 * SYNOPSIS
 *		void SetFreqFilename (
 *			EasyFileReq	*ezfrq,
 *			char		*filename
 *		)
 *
 * PURPOSE
 *		Set the file name of a freq struct.
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
 *	03/10/91 Sunday (RGM) - Created.
 *
 *
 * SEE ALSO
 *
*/
void SetFreqFilename (
	EasyFileReq	*ezfrq,
	char		*filename
)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetFreqFilename";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	strcpy (ezfrq->Name, BaseName(filename));

} /* SetFreqFilename */


/**************************************************************************
 *
 * SetFreqDirname
 *
 * SYNOPSIS
 *		void SetFreqDirname (
 *			EasyFileReq	*ezfrq,
 *			char		*dirname
 *		)
 *
 * PURPOSE
 *		Set the file name of a freq struct.
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
 *	03/10/91 Sunday (RGM) - Created.
 *
 *
 * SEE ALSO
 *
*/
void SetFreqDirname (
	EasyFileReq	*ezfrq,
	char		*dirname
)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetFreqFilename";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	EIO_fnsplit (dirname, ezfrq->Path, NULL, NULL);
	strcat (ezfrq->Path, "*.*");

} /* SetFreqDirname */

/* MouseButtons - event handler for mouse buttons. */

short MouseButtons(HitInfo *hi)
{
	LeftButton = hi->Buttons & MOUSE_LEFTBUTTON;
	RightButton = hi->Buttons & MOUSE_RIGHTBUTTON;

	MouseX = hi->X;
	MouseY = hi->Y;

	hitGlobal = *hi;
	RealButtonsE(TRUE);

	return TRUE;
}

/* MouseDown - event handler for mouse button held down. */

short MouseDown(HitInfo *hi)
{
	LeftButton = hi->Buttons & MOUSE_LEFTBUTTON;
	RightButton = hi->Buttons & MOUSE_RIGHTBUTTON;

	MouseX = hi->X;
	MouseY = hi->Y;

	if (FN_DontWait)
		FN_DontWait(hi);

	return TRUE;
}


/*********************************************************************
 *
 * GetCharByteMap
 *
 * PURPOSE
 *		Cut out from tile image <pTile> with Plot_Flags <ubFlags>
 *		and pixel offset <wXTOffset>, <wYTOffset> within the tile,
 *		of size pbm->width, pbm->height, and save in <pbm>.
 *
 *		If the tile is x-flipped, then adjust wXTOffset.
 *		If the tile is y-flipped, then adjust wYTOffset.
 *
 *		The saved image does not get flipped even if flip bits are set.
 *		pbm->transparentColors is set.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		UBYTE with tile flags.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UBYTE GetCharByteMap(ByteMap *pbm, PlotType *pplt,
						int wXTOffset, int wYTOffset)
{
#if __MSDOS32X__
	ULONG wDstDupXSave;
	ULONG wDstDupYSave;
	ULONG wSrcSkipXSave;
	ULONG wSrcSkipYSave;
#else
	UWORD wDstDupXSave;
	UWORD wDstDupYSave;
	UWORD wSrcSkipXSave;
	UWORD wSrcSkipYSave;
#endif
	UBYTE ptfSave;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetCharByteMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ptfSave = pplt->Plot_Flags;

	if (ptfSave & ubXFlipBit)
		wXTOffset = (TILESET_AT_PLOT(pplt))->Width - pbm->width - wXTOffset;

	if (ptfSave & ubYFlipBit)
		wYTOffset = (TILESET_AT_PLOT(pplt))->Height - pbm->height - wYTOffset;

	pplt->Plot_Flags = ptfSave & (~ubXFlipBit) & (~ubYFlipBit);

	wDstDupXSave	= wDstDupX;
	wDstDupYSave	= wDstDupY;
	wSrcSkipXSave	= wSrcSkipX;
	wSrcSkipYSave	= wSrcSkipY;

	wDstDupX	= wDstDupY = wSrcSkipX = wSrcSkipY = 1;

	StampTile(pbm, pplt, 0-wXTOffset, 0-wYTOffset, TRUE, 0);

	wDstDupX			= wDstDupXSave;
	wDstDupY			= wDstDupYSave;
	wSrcSkipX		= wSrcSkipXSave;
	wSrcSkipY		= wSrcSkipYSave;

	return pplt->Plot_Flags = ptfSave;

} /* GetCharByteMap */


/*********************************************************************
 *
 * OpenMousePointers
 *
 * SYNOPSIS
 *		BOOL OpenMousePointers(char *sz)
 *
 * PURPOSE
 *		Load mouse pointers, allocate resources to store them.
 *		<sz> is the file to load the pointers from.
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
BOOL OpenMousePointers(char *sz)
{

	char	datapath[EIO_MAXPATH];

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenMousePointers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!EIO_FindFile (sz, NULL, datapath))
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("\nCouldn' find file '%s' in path", sz);
		return FALSE;
	}

	return (MouseBPI = LoadBPI (datapath, FPI_TUME_BPI)) != NULL;

} /* OpenMousePointers */


/*********************************************************************
 *
 * CloseMousePointers
 *
 * SYNOPSIS
 *		void CloseMousePointers(void)
 *
 * PURPOSE
 *		Free the resources used to store mouse pointers.
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
void CloseMousePointers(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CloseMousePointers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (MouseBPI)
	{
		FreeBPI (MouseBPI);
	}

} /* CloseMousePointers */


/*********************************************************************
 *
 * DCC_TempSetPointer
 *
 * PURPOSE
 *		Set mouse pointer to MouseBPI->Shapes[<wMouse>].
 *		Ignore current hide mouse or show mouse state (HidePointerSTATE).
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
 *		02/20/94 Sunday (dcc) - add check for NULL MouseBPI
 *
 * SEE ALSO
 *
*/
void DCC_TempSetPointer(short wMouse)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DCC_TempSetPointer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (MouseBPI)
	{
		HideMouse();
		SetMouseShape(MouseBPI->Shapes[wMouse]);
		SetMouseHotSpot(MouseXHotSpot[wMouse], MouseYHotSpot[wMouse]);
		ShowMouse();
	}
} /* DCC_TempSetPointer */


/*********************************************************************
 *
 * DCC_SetPointer
 *
 * SYNOPSIS
 *		short DCC_SetPointer(short wMouse)
 *
 * PURPOSE
 *		Set mouse pointer to MouseBPI->Shapes[<wMouse>].
 *		Track current hide mouse or show mouse state (HidePointerSTATE).
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
short DCC_SetPointer(short wMouse)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DCC_SetPointer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (HidePointerSTATE)
		DCC_TempSetPointer(BPI_BLANK_POINTER);
	else
		DCC_TempSetPointer(wMouse);

	return wMouse;

} /* DCC_SetPointer */


#if 0
/*********************************************************************
 *
 * GetCharByteMap
 *
 * PURPOSE
 *		Cut out from tile image <pTile> with Plot_Flags <ubFlags>
 *		and pixel offset <wXTOffset>, <wYTOffset> within the tile,
 *		of size pbm->width, pbm->height, and save in <pbm>.
 *
 *		If the tile is x-flipped, then adjust wXTOffset.
 *		If the tile is y-flipped, then adjust wYTOffset.
 *
 *		The saved image does not get flipped even if flip bits are set.
 *		pbm->transparentColors is set.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		UBYTE with tile flags.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UBYTE GetCharByteMap(ByteMap *pbm, PlotType *pplt,
						int wXTOffset, int wYTOffset)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetCharByteMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	copyMode = COPYNOMASKS;
	pixelAND = 0xff;
	pixelOR = 0;

	{
		ColorGroupMask *pcgm;

		pcgm = FirstColorGroup();

		while (!IsEOList(pcgm))
		{
			if (pplt->Plot_Flags & GetColorEnableANDMask(pcgm))
			{
				copyMode = COPYPIXELMASK;
				pixelAND &= GetPixelANDMask(pcgm, pplt->Plot_Flags);
				pixelOR |= GetPixelORMask(pcgm, pplt->Plot_Flags);
			}
			pcgm = Next(pcgm);
		}
	}

	if ((TILE_AT_PLOT(pplt)))
	{
		ByteMap bm;
		UWORD col;
		UWORD idx;
		TileSetType *ptst = TILESET_AT_PLOT(pplt);

		bm.width		= ptst->Width;
		bm.height	= ptst->Height;
		{
		col = (pplt->Tile_ID - 1) / ptst->tilesPerCol;
		idx = (pplt->Tile_ID - 1) % ptst->tilesPerCol;

		bm.data = (UBYTE *) ActivateXTRAatWin(ptst->pxtrTilCol[col], winTil)
														+ idx * ptst->sizeofTil;
		}
		bm.transparentColor = pbm->transparentColor = ptst->SharedtransparentColor;

		if (pplt->Plot_Flags & ubXFlipBit)
			wXTOffset = bm.width - pbm->width - wXTOffset;

		if (pplt->Plot_Flags & ubYFlipBit)
			wYTOffset = bm.height - pbm->height - wYTOffset;

		MCGA_ClippedMaskedCopyTransRect(&bm, wXTOffset, wYTOffset,
													pbm, 0, 0, pbm->width, pbm->height);

		ReleaseXTRA(ptst->pxtrTilCol[col]);
	}
	else
	{
		int i;
		UBYTE *pub;

		pub = pbm->data;

		for (i = 0; i < pbm->width * pbm->height; i++)
		{
			*pub++ = 0;
		}
	}
	return pplt->Plot_Flags;
} /* GetCharByteMap */
#endif

