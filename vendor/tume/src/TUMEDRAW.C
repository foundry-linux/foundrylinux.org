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
 * TUMEDRAW.C
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 10/02/89 
 *   MODIFIED : 06/07/95
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		Assorted display routines for tUME.
 *
 *		BUGBUG: the constants SCREEN_WIDTH & SCREEN_HEIGHT should be
 *		replaced with the actual width & height of current pane.
 *
 * HISTORY
 *		10/02/89 (RGM) - Created.
 *		01/16/93 (dcc) - if drawing only a single layer (EditOnlyFloorSTATE ==
 *						TRUE), don't draw tiles transparently.
 *		01/25/93 (dcc) - change ShowOnlyFloor to EditOnlyFloor;
 *						when TRUE, edit only floor layer.
 *		04/05/93 (dcc) - use rectplot.h header.
 *		11/05/93 (dcc) - fix 320x31 16x16 rooms not plotting on HP-650C
 *						(see ShowRoomRectLayer()).
 *		08/21/94 (dcc) - add support for UseSparseArray
 *
*/
#include <echidna/platform.h>
#include "switches.h"
#include "switch1.h"

#include "rectplot.h"
#include "tuglbl.h"
#if __AMIGAOS__
#include "blitter.h"

#include <echidna/shape.h>
#elif __MSDOS__
#include <echidna/grafx.h>
#include <echidna/listfunc.h>
#include <stdlib.h>
#endif/*__AMIGAOS__/__MSDOS__*/
#if NEWTUME
#include <limits.h>
#endif

#include "events.h"
#include "hilitile.h"
#include "histogrm.h"
#include "layrtili.h"
#include "layrtile.h"
#if	   PixelLayers
#include "maketile.h"
#endif	/* PixelLayers */
#include "misc.h"
#include "search.h"
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray
#include "tilebits.h"
#include "tumemain.h"
#include "tumedraw.h"				/* Verify function prototypes. */
#include "assert.h"

/**************************** C O N S T A N T S ***************************/

#define DIRECTCOPY 0xC0

#if TEST_PRINT
#define TEST_WIDTH	320

#define TileSet_ID	TilesetID
#define	Tile_ID		TileID
#define Plot_Flags	PlotFlags

#define wGuideXOrigin	GuideX
#define wGuideYOrigin	GuideY
#define wGuideXWidth	GuideWidth
#define wGuideYHeight	GuideHeight

#define GetPixelORMask(a,b)		0x00
#define GetPixelANDMask(a,b)	0xFF
#define GetColorEnableANDMask(a)	0x00
#endif /* TEST_PRINT */

/******************************** T Y P E S *******************************/


/****************************** E X T E R N S *****************************/

extern WORD OldColumn;
extern WORD OldRow;

/****************************** G L O B A L S *****************************/




#if TEST_PRINT

UBYTE		 ubYFlipBit = 0x20;
UBYTE		 ubXFlipBit = 0x40;
short		 GlobalBackground = 0x00;

WORD		 wShowXTiles;
WORD		 wShowYTiles;

int		 TopAdd = 12;

UBYTE		 ImageBuffer[1024];
#endif

UBYTE ubGlobalShowFlagAND = 0xFF;
UBYTE ubGlobalShowFlagOR = 0x00;
UBYTE ubGlobalShowFlagXOR = 0x00;

/******************************* L O C A L S ******************************/

static short fStampMode = fSetStampPaint;

static BOOL fDrawNULLTiles = FALSE;
static LONG lNULLTileDispWidth;
static LONG lNULLTileDispHeight;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/















































#if TEST_PRINT
/*********************************************************************
 *
 * FAST_TILE_PTR
 *
 * SYNOPSIS
 *		TileType *FAST_TILE_PTR(UWORD set_id, UWORD tile_id)
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
TileType *FAST_TILE_PTR (UWORD set_id, UWORD tile_id)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FAST_TILE_PTR";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (set_id) {
		static	TileType	tt;

		TilesetType		*ts;
		TileDataGroup		*tdg;
		UWORD			 tcount;
		UBYTE			*tiledata;
		short			 found;
		UWORD			 tilesize;

		ts = GlobalMap->Tilesets[set_id];
		tilesize = ts->TMG.Width * ts->TMG.Height;

		tcount = tile_id - 1;
		found  = FALSE;

		if (tcount >= ts->TMG.TileCount) {
			return NULL;
		}

		tdg = Head (ts->TMG.TileGroupList);
		while (!found) {
			if (tcount < tdg->NumTiles) {
				found     = TRUE;
				tiledata  = ActivateXTRAatWin (tdg->TileData, winTil);
				tiledata += tilesize * tcount;

			} else {
				tcount -= tdg->NumTiles;
				tdg = Next (tdg);
			}
		}

		memcpy (ImageBuffer, tiledata, ts->TMG.Width * ts->TMG.Height);
		tt.Data = ImageBuffer;
		ReleaseXTRA(tdg->TileData);
		return (&tt);
	} else {
		return NULL;
	}
} /* FAST_TILE_PTR */
#endif /* TEST_PRINT */

/*********************************************************************
 *
 * wSrcSizeToDstSize
 *
 * PURPOSE
 *		Convert source width (height) to destination width (height).
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
 *		11/16/93 Tuesday (dcc) - change wSrcSkip, wDstDup and wMod
 *							from WORD to unsigned int.
 *
 * SEE ALSO
 *
*/
WORD wSrcSizeToDstSize(WORD w, unsigned int wSrcSkip, unsigned int wDstDup)
{

	WORD wDstWidth = 0;
	unsigned int wMod = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "wSrcSizeToDstSize";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Count how many pixels we are drawing each destination line. */

	for ( ; w > 0; w--)
	{
		if (wMod < wDstDup)
		{
			wDstWidth++;
		}
		wMod += wDstDup;

		if (wMod >= wSrcSkip)
		{
			wMod -= wSrcSkip;
		}

		while (wMod >= wSrcSkip)
		{
			wDstWidth++;
			wMod -= wSrcSkip;
		}
	}
	return wDstWidth;
} /* wSrcSizeToDstSize */


/*********************************************************************
 *
 * wSrcSizeToDstSizeWithMod
 *
 * PURPOSE
 *		Convert source width (height) to destination width (height).
 *
 *		This routine does the exact same thing as wSrcSizeToDstSize()
 *		with one exception: <wMod> is an input to the function
 *		(instead of being initialized to zero).
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
 *		11/16/93 Tuesday (dcc) - change wSrcSkip, wDstDup and wMod
 *							from WORD to unsigned int.
 *
 * SEE ALSO
 *
*/
WORD wSrcSizeToDstSizeWithMod(WORD w,
			unsigned int wSrcSkip, unsigned int wDstDup, unsigned int wMod)
{

	WORD wDstWidth = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "wSrcSizeToDstSizeWithMod";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Count how many pixels we are drawing each destination line. */

	for ( ; w > 0; w--)
	{
		if (wMod < wDstDup)
		{
			wDstWidth++;
		}
		wMod += wDstDup;

		if (wMod >= wSrcSkip)
		{
			wMod -= wSrcSkip;
		}

		while (wMod >= wSrcSkip)
		{
			wDstWidth++;
			wMod -= wSrcSkip;
		}
	}
	return wDstWidth;
} /* wSrcSizeToDstSizeWithMod */


/*********************************************************************
 *
 * SetGlobalZoom
 *
 * SYNOPSIS
 *		void SetGlobalZoom(RoomType *prt)
 *
 * PURPOSE
 *		Set to global scaling variables.
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
void SetGlobalZoom(RoomType *prt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetGlobalZoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	#if !TEST_PRINT
	if (prt && IsZooming(prt))
	{
		int i = prt->wZoomIndex & wZOOMINDEX;

/* These boundary checks are here for two reasons:
1. The default zoom index comes from tUME.INI and may be out of range; and
2. If there is only one zoom event at 1:1, then both wZoomInOne and
	wZoomOutOne remain at -1. This can cause prt->wZoomIndex to go to -1. */

		if (i < 0)
		{
			prt->wZoomIndex = i = 0;
			SetZoomOn(prt);
		}
		if (i > GetZoomMax()-1)
		{
			prt->wZoomIndex = i = GetZoomMax()-1;
			SetZoomOn(prt);
		}

		wDstDupX	= wGetZoomDstDup(i);
		wDstDupY	= wGetZoomDstDup(i);
		wSrcSkipX	= wGetZoomSrcSkip(i);
		wSrcSkipY	= wGetZoomSrcSkip(i);
	}
	else
	{
		wDstDupX	= wDstDupY = wSrcSkipX = wSrcSkipY = 1;
	}
	#else /* TEST_PRINT */
	prt = prt;
	#endif /* TEST_PRINT */

} /* SetGlobalZoom */


/*********************************************************************
 *
 * StampTile
 *
 * PURPOSE
 *		Stamp tile <ppt> to <pbmpDst> at <lX>, <lY>.
 *		if <fNotTransparent> is true, draw every pixel (no transparencies).
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
 *		12/15/93 (dcc) - add check for ptst->fAlwaysTransp.
 *		08/21/94 (dcc) - add support for UseSparseArray
 *		10/12/94 (dcc) - Check for tile out of range, just return.
 *
 * SEE ALSO
 *
*/
void StampTile(ByteMap *pbmpDst, PlotType *ppt, LONG lX, LONG lY,
				BOOL fNotTransparent, UBYTE fFlags)
{

#if __AMIGAOS__
	TileType	*tile		= FAST_TILE_PTR (ppt->TileSet_ID, ppt->Tile_ID);
#elif __MSDOS__
#if TEST_PRINT
	TileType	*tile		= FAST_TILE_PTR (ppt->TileSet_ID, ppt->Tile_ID);
#endif
#endif/*__AMIGAOS__/__MSDOS__*/

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "StampTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	if (tile)
#elif __MSDOS__
#if TEST_PRINT
	if (tile)
#else
	if (!fNullTile(ppt))
#endif
#endif/*__AMIGAOS__/__MSDOS__*/
	{
		TileSetType *tileset = FAST_TILESET_PTR(ppt->TileSet_ID);

		if ((!tileset) || (ppt->Tile_ID > tileset->TileCount))
			return;				/* Can't draw it... */

		/* Toggle fFlags flip info based on ppt->Plot_Flags. */

		if (ppt->Plot_Flags & ubXFlipBit)
		{
			fFlags ^= ubXFlipBit;
		}
		if (ppt->Plot_Flags & ubYFlipBit)
		{
			fFlags ^= ubYFlipBit;
		}

		/* If fFlags already has a color mask, then it overrides the
		   one in ppt->Plot_Flags, otherwise copy the mask in ppt->Plot_Flags
		   into fFlags. */

		#if !TEST_PRINT
		{
			ColorGroupMask *pcgm;

			pcgm = FirstColorGroup();

			while (!IsEOList(pcgm))
			{
				UBYTE ubEnableANDMask = GetColorEnableANDMask(pcgm);

				if ((fFlags & ubEnableANDMask) == 0)
				{
					if (ppt->Plot_Flags & ubEnableANDMask)
					{
						fFlags |= ubEnableANDMask;
						
						/* Wherever GetColorConfigANDMask() bit is 1, copy corresponding
						   bit from ppt->Plot_Flags to fFlags. */

						fFlags ^= (fFlags ^ ppt->Plot_Flags) & GetColorConfigANDMask(pcgm);
					}
				}
				pcgm = Next(pcgm);
			}
		}
		#endif /* TEST_PRINT */

		if ((tileset->Flags & GRID_TYPE_MODIFIER) != TILESET_COMPOSITE)
		{
			int			 tilewidth = tileset->Width;
			int			 tileheight = tileset->Height;
#if __MSDOS__
			UWORD		 col;
			UWORD		 idx;

			ByteMap		 bm;
#endif/*__MSDOS__*/

#if __AMIGAOS__
			Shape		*t_shape = tileset->SharedShape;

			{
				register MyPlanePtr *dest_planes, *source_planes;
				register MyPlanePtr *stop;

				source_planes	= tile->ImageBitMap.Planes;
				dest_planes	= t_shape->Pic->BitMap.Planes;

				stop = source_planes + MAXPLANES;

				for ( ; source_planes < stop; )
				{
					*dest_planes	= *source_planes;

					dest_planes++;
					source_planes++;
				}

				if (!fNotTransparent)
				{
					source_planes	= tile->MaskPlanes;
					dest_planes		= t_shape->MaskBm.Planes;

					stop = source_planes + MAXPLANES;

					for ( ; source_planes < stop; ) {
						*dest_planes	= *source_planes;

						dest_planes++;
						source_planes++;
					}
				}
			}
#endif/*__AMIGAOS__*/

			/******************************/
			/* TILE DISPLAY MASKING STUFF */

			fFlags &= ubGlobalShowFlagAND;
			fFlags |= ubGlobalShowFlagOR;
			fFlags ^= ubGlobalShowFlagXOR;

			/*************************/
			/* PALETTE MASKING STUFF */

			copyMode = COPYNOMASKS;
			pixelAND = 0xff;
			pixelOR = 0;

			#if !TEST_PRINT
			{
				ColorGroupMask *pcgm;

				pcgm = FirstColorGroup();

				while (!IsEOList(pcgm))
				{
					if (fFlags & GetColorEnableANDMask(pcgm))
					{
						copyMode = COPYPIXELMASK;
						pixelAND &= GetPixelANDMask(pcgm, fFlags);
						pixelOR |= GetPixelORMask(pcgm, fFlags);
					}
					pcgm = Next(pcgm);
				}
			}
			#endif /* TEST_PRINT */

#if __AMIGAOS__
			/*************************/
			/* PALETTE MASKING STUFF */

/* To the clever hack implementing the Amiga version of this code:
well, at this point, pixelAND and pixelOR tell you which planes
need to be set to FullPlane, and which plane need to be set to EmptyPlane.
If the corresponding bit in pixelAND is 0, then the corresponding
plane should be set the FullPlane if pixelOR is 1, or EmptyPlane if
pixelOR is 0. Unfortunately, I can't find the header definition for
a Shape or a BitMap, so I can't decide how many planes to affect (just
the low 4, all 8, or what?) You look it up and finish this code. */

/* FIXME starts */
			if (fFlags & COLORGROUP_FLAG)
			{
				MyPlanePtr		*change_image;

				change_image	= t_shape->Pic->BitMap.Planes;

				if (fFlags & 1) {
					change_image[2]	= tileset->FullPlane;
				}
				else {
					change_image[2]	= tileset->EmptyPlane;
				}

				if (fFlags & 2) {
					change_image[3]	= tileset->FullPlane;
				}
				else {
					change_image[3]	= tileset->EmptyPlane;
				}
			}
/* FIXME ends */

			if (fNotTransparent)
			{
				CopyBitMap (
					&(t_shape->Pic->BitMap),
					0,
					0,
					rpbitmap,
					wDstXxSrcSkipX,
					wDstYxSrcSkipY,
					tilewidth,
					tileheight,
					0xFF
				);
			}
			else {
				FastDrawShape (
					rp,
					t_shape,
					wDstXxSrcSkipX,
					wDstYxSrcSkipY
				);
			}
#elif __MSDOS__
			bm.width	= tilewidth;
			bm.height	= tileheight;
			#if TEST_PRINT
			bm.data	= tile->Data;
			bm.transparentColor = tileset->TMG.TransparentColor;
			#else
			{
			Assert(tileset->tilesPerCol != 0);
			col = (ppt->Tile_ID - 1) / tileset->tilesPerCol;
			idx = (ppt->Tile_ID - 1) % tileset->tilesPerCol;

			bm.data = (UBYTE *) ActivateXTRAatWin(tileset->pxtrTilCol[col], winTil)
										+ idx * tileset->sizeofTile;
			}
			bm.transparentColor = tileset->SharedtransparentColor;
			#endif

			if (!fNotTransparent || tileset->fAlwaysTransp)
			{
				copyMode |= COPYTRANSMASK;
			}

			copyMode |= (copyMode & (COPYTRANSMASK | COPYPIXELMASK));
			if (fFlags & ubXFlipBit)
				copyMode |= XFLIPMASK;
			if (fFlags & ubYFlipBit)
				copyMode |= YFLIPMASK;

			if (wSrcSkipX == wDstDupX && wSrcSkipY == wDstDupY)
			{
				Assert(wSrcSkipX != 0);
				Assert(wSrcSkipY != 0);
				MCGA_ClippedMaskedCopyTransRect(&bm, 0, 0, pbmpDst,
						(short) (lX / wSrcSkipX), (short) (lY / wSrcSkipY),
						tilewidth, tileheight);
			}
			else
			{
				MCGA_ClippedScaledMaskedCopyTransRect(&bm, 0, 0,
						pbmpDst, lX, lY, tilewidth, tileheight,
						wSrcSkipX, wDstDupX, wSrcSkipY, wDstDupY);
			}
			ReleaseXTRA(tileset->pxtrTilCol[col]);
#endif/*__AMIGAOS__/__MSDOS__*/
		}
		#if !TEST_PRINT
		else		/* Draw composite tile. */
		{
			WORD wTileX, wTileY;
			LONG lDstDispWidth, lDstDispHeight;
			WORD wDX, wDY;
			WORD wTA = tileset->wTilesAcross;
			WORD wTD = tileset->wTilesDown;
			LayerType *layer = Head(&tileset->SourceRoom->Layers);

//			wTileX = tile->Original_Column * wTA;
//			wTileY = tile->Original_Row * wTD;
			wTileX = ((ppt->Tile_ID-1)%(tileset->Across+1)) * wTA;
			wTileY = ((ppt->Tile_ID-1)/(tileset->Across+1)) * wTD;
			Assert(wTA != 0);
			lDstDispWidth	= (LONG) (tileset->Width / wTA) * wDstDupX;
			Assert(wTD != 0);
			lDstDispHeight	= (LONG) (tileset->Height / wTD) * wDstDupY;

			if (fFlags & ubXFlipBit)
			{
				wDX = -1;
				lX += lDstDispWidth * (wTA-1);
			}
			else
			{
				wDX = 1;
			}
			if (fFlags & ubYFlipBit)
			{
				wDY = -1;
				lY += lDstDispHeight * (wTD-1);
			}
			else
			{
				wDY = 1;
			}

			for (wTD = tileset->wTilesDown; wTD > 0; wTD--)
			{
				WORD wTX = wTileX;
				LONG lx = lX;
				
				for (wTA = tileset->wTilesAcross; wTA > 0; wTA--)
				{
					/* Only plot if it's possibly on dest BM. */

					if  (lx + lDstDispWidth > 0 &&
						lY + lDstDispHeight > 0 &&
						lx < (LONG) pbmpDst->width * wSrcSkipX &&
						lY < (LONG) pbmpDst->height * wSrcSkipY)
					{
						PlotType plt;

#if UseSparseArray
						LAY_ReadPlotXY(layer, wTX, wTileY, &plt);
#else // !UseSparseArray
						ReadNPlotXY(layer->rgrgplt, wTX, wTileY, 1, &plt);
#endif // !UseSparseArray

						StampTile(pbmpDst, &plt, lx, lY, fNotTransparent, fFlags);
					}
					wTX++;
					lx += lDstDispWidth * wDX;
				}
				wTileY++;
				lY += lDstDispHeight * wDY;
			}
		}
		#endif /* TEST_PRINT */
	}
	else
	{
		if (fDrawNULLTiles)
		{
			SetPenColor(GlobalBackground);

			/* Warning: we are coercing longs into shorts. We are making
				the assumption that the screen is less than 32768 wide,
				and therefore it'll get clipped anyway. */

			Assert(wSrcSkipX != 0);
			Assert(wSrcSkipY != 0);
			DrawRect((short) (lX / wSrcSkipX), (short) (lY / wSrcSkipY),
					(short) lNULLTileDispWidth, (short) lNULLTileDispHeight);
		}
	}
} /* StampTile */


#if PixelLayers
/*********************************************************************
 *
 * LookupObjectType
 *
 * PURPOSE
 *		Lookup the string type that corresponds to uwTsID.
 *
 * INPUT
 *		UWORD uwTsID	: TileSet_ID to lookup
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		char *sz		: string that corresponds to uwTsID
 *
 * HISTORY
 *		10/20/94 (dcc) - created.
 *
*/
void LookupObjectType(char *sz, UWORD uwTsID)
{

	OBJ_Type		*pobj;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LookupObjectType";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pobj = Head(plstObjectType);

	while (!IsEOList(pobj))
	{
		if (pobj->Type == uwTsID)
		{
			strncpy(sz, NodeName(pobj), 5);
			return;
		}
		pobj = Next(pobj);
	}
	sprintf(sz, "%05u", uwTsID);

} /* LookupObjectType */


/*********************************************************************
 *
 * StampTileAsNumbers
 *
 * PURPOSE
 *		Stamp tile <ppt> on-screen at <lX>, <lY> as its tile numbers.
 *		if <fNotTransparent> is true, draw every pixel (no transparencies).
 *
 * INPUT
 *		LONG lX				: x-offset to draw on screen * <wSrcSkipX>
 *		LONG lY				: y-offset to draw on screen * <wSrcSkipY>
 *		BOOL fNotTransparent	: if TRUE, no transparent background
 *
 * ASSUMES
 *		There is a border around each number;
 *		the numbers are centered about the upper-left corner pixel of the tile;
 *		Tile_ID is display as a 5 digit decimal number; and
 *		Plot_Flags and TileSet_ID are display as 4 digit hex number.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/23/94 (dcc) - created (based on StampTile()).
 *
*/
void StampTileAsNumbers(ByteMap *pbmpDst, PlotType *pplt, LONG lX, LONG lY,
					BOOL fNotTransparent, UBYTE fFlags)
{
	int dxNumbers, dyNumbers;
	int cxNumbers, cyNumbers;
	int cxNumber2, cyNumber2;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "StampTileAsNumbers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	cxNumbers = 6 * 5 + 3;
	cyNumbers = 6 * 2 + 3;
	cxNumber2 = cxNumbers / 2;
	cyNumber2 = cyNumbers / 2;

	pbmpDst = pbmpDst;
	fFlags = fFlags;

	/*	Warning: we are coercing longs into ints. We are making
		the assumption that the screen is less than 32768 wide,
		and therefore it'll get clipped anyway. */

	Assert(wSrcSkipX != 0);
	dxNumbers = (int) (lX / wSrcSkipX - cxNumber2);
	Assert(wSrcSkipY != 0);
	dyNumbers = (int) (lY / wSrcSkipY - cyNumber2);

	BeforeGraphics();

	if (fNotTransparent)
	{
#if 0
		/* If the tile is larger than the tile, then erase extra */

		if ( fZeroTile(pplt) ||
			lNULLTileDispWidth > cxNumber2 ||
			lNULLTileDispHeight> cyNumber2 )
		{
			SetPenColor(GlobalBackground);
			DrawRect((int) (lX / wSrcSkipX), (int) (lY / wSrcSkipY),
					(int) lNULLTileDispWidth, (int) lNULLTileDispHeight);
		}
#endif

		if (!fZeroTile(pplt))
		{
			/* Erase background for the tile numbers */

			SetPenColor(255);
			DrawRect(dxNumbers+1, dyNumbers+1, cxNumbers-1, cyNumbers-1);
		}
	}
	if (!fZeroTile(pplt))
	{
		char szShow[10];

		/* Draw border */

		SetPenColor(254);	//GlobalBackground);
		DrawRect(dxNumbers, dyNumbers, 1, cyNumbers);
		DrawRect(dxNumbers+cxNumbers-1, dyNumbers, 1, cyNumbers);
		DrawRect(dxNumbers, dyNumbers, cxNumbers, 1);
		DrawRect(dxNumbers, dyNumbers+cyNumbers-1, cxNumbers, 1);

#if 0
		/* Draw border */

		XORvLine(pbmpDst, dxNumbers, dyNumbers, dyNumbers+cyNumbers-1, 255);
		XORvLine(pbmpDst, dxNumbers+cxNumbers-1, dyNumbers, dyNumbers+cyNumbers-1, 255);
		XORhLine(pbmpDst, dxNumbers, dxNumbers+cxNumbers-1, dyNumbers, 255);
		XORhLine(pbmpDst, dxNumbers, dxNumbers+cxNumbers-1, dyNumbers+cyNumbers-1, 255);
#endif

		/* Draw pixel in center */

		DrawRect(dxNumbers+(cxNumbers/2), dyNumbers+(cyNumbers/2), 1, 1);

		dxNumbers += 2;
		dyNumbers += 2;

//		int cxTilDst, cyTilDst;

//		cxTilDst	= wSrcSizeToDstSize(cxTile, wSrcSkipX, wDstDupX) + sep;
//		cyTilDst	= wSrcSizeToDstSize(cyTile, wSrcSkipY, wDstDupY) + sep;

#if 1
		LookupObjectType(szShow, pplt->Tile_ID);
#else
		sprintf(szShow, "%05u", pplt->Tile_ID);
#endif

		DrawStringN(&SmallFont, dxNumbers, dyNumbers, szShow, 5);

		sprintf(szShow, "%04X", (pplt->Plot_Flags << 8) | pplt->TileSet_ID);

		DrawStringN(&SmallFont, dxNumbers + 3, dyNumbers + 6, szShow, 4);
	}
	AfterGraphics();

} /* StampTileAsNumbers */
#endif // PixelLayers


#if !TEST_PRINT
#if PIXELSELECT
/*********************************************************************
 *
 * DrawSelectionOutline
 *
 * PURPOSE
 *		<dx> & <dy> are pixel offsets of the tile origin. <dtilx> and
 *		<dtily> are the tile offsets from the tile origin. <ctilx> and
 *		<ctily> defines how many tiles to outline. <cxTile> & <cyTile>
 *		defines the size of each tile, and <sep> defines whether tiles
 *		are separated.
 *
 * INPUT
 *		dx		: x pixel offset of left-most tile on-screen
 *		dy		: y pixel offset of top-most tile on-screen
 *		cx		: pixel width of area on screen that MAY be drawn to
 *		cy		: pixel height of area on screen that MAY be drawn to
 *		dtilx	: x tile offset from left-most tile on-screen
 *		dtily	: y tile offset from top-most tile on-screen
 *		ctilx	: tiles wide to outline
 *		ctily	: tiles high to outline
 *		dxModSrcL	: pixel offset within left-most tile to draw line
 *		dyModSrcT	: pixel offset within top-most tile to draw line
 *		dxModSrcR	: pixel offset within right-most tile to draw line
 *		dyModSrcB	: pixel offset within bottom-most tile to draw line
 *		cxTile	: pixel width of a single tile
 *		cyTile	: pixel height of a single tile
 *		sep		: 1 if tiles are separated
 *		eor_mask	: pixel value to exclusive-or draw line with
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static void DrawSelectionOutline(short dx, short dy, short cx, short cy,
				short dtilx, short dtily, short ctilx, short ctily,
				int dxModSrcL, int dyModSrcT, int dxModSrcR, int dyModSrcB,
				UWORD cxTile, UWORD cyTile, short sep, UBYTE eor_mask)
{
	short cxTilDst, cyTilDst;
	short dxOutline, dyOutline;
	short cxOutline, cyOutline;
	int dxModDstL, dyModDstT, dxModDstR, dyModDstB;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawSelectionOutline";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	cxTilDst	= wSrcSizeToDstSize(cxTile, wSrcSkipX, wDstDupX) + sep;
	cyTilDst	= wSrcSizeToDstSize(cyTile, wSrcSkipY, wDstDupY) + sep;

	if (dxModSrcL >= 0)
	{
		dxModDstL	= wSrcSizeToDstSize(dxModSrcL, wSrcSkipX, wDstDupX);
	}
	else
	{
		if (sep)
			dxModDstL = -1;
		else
			dxModDstL = 0;
	}
	if (dyModSrcT >= 0)
	{
		dyModDstT	= wSrcSizeToDstSize(dyModSrcT, wSrcSkipY, wDstDupY);
	}
	else
	{
		if (sep)
			dyModDstT = -1;
		else
			dyModDstT = 0;
	}
	if (dxModSrcR >= 0)
	{
		if (dxModSrcR == cxTile)
			dxModDstR = cxTilDst-1;
		else
			dxModDstR	= wSrcSizeToDstSize(dxModSrcR+1, wSrcSkipX, wDstDupX)-1;
	}
	else
	{
		if (sep)
			dxModDstR = -1;
		else
			dxModDstR = 0;
	}
	if (dyModSrcB >= 0)
	{
		if (dyModSrcB == cyTile)
			dyModDstB = cyTilDst-1;
		else
			dyModDstB	= wSrcSizeToDstSize(dyModSrcB+1, wSrcSkipY, wDstDupY)-1;
	}
	else
	{
		if (sep)
			dyModDstB = -1;
		else
			dyModDstB = 0;
	}
	/* figure out where to draw */

	dxOutline	= dtilx * cxTilDst + dx + dxModDstL + sep;
	dyOutline	= dtily * cyTilDst + dy + dyModDstT + sep;
	cxOutline = dxModDstR-dxModDstL;
	cyOutline = dyModDstB-dyModDstT;

	if (ctilx > 1)
		cxOutline	+= (ctilx-1) * cxTilDst;
	if (ctily > 1)
		cyOutline	+= (ctily-1) * cyTilDst;

	if (	!(dyOutline < dy && (dyOutline+cyOutline) < dy) &&
		!(dyOutline >= dy+cy && (dyOutline+cyOutline) >= dy+cy) &&
		!(dxOutline < dx && (dxOutline+cxOutline) < dx) &&
		!(dxOutline >= dx+cx && (dxOutline+cxOutline) >= dx+cx) &&
		dxOutline <= (dxOutline+cxOutline) && dyOutline <= (dyOutline+cyOutline))
	{
		BeforeGraphics();

		if (cxOutline > 0 && cyOutline > 0)
		{
			if (dyOutline >= GetClipTop() && dyOutline < GetClipTop() + GetClipHeight() &&
				(dxOutline+cxOutline) - 1 >= GetClipLeft() && dxOutline + 1 <= GetClipLeft() + GetClipWidth()-1)
				XORhLine(DisplayBM,
						max(dxOutline + 1, GetClipLeft()),
						min((dxOutline+cxOutline) - 1, GetClipLeft() + GetClipWidth()-1),
						dyOutline, eor_mask);
			if ((dyOutline+cyOutline) >= GetClipTop() && (dyOutline+cyOutline) < GetClipTop() + GetClipHeight() &&
				(dxOutline+cxOutline) >= GetClipLeft() && dxOutline <= GetClipLeft() + GetClipWidth()-1)
				XORhLine(DisplayBM,
						max(dxOutline, GetClipLeft()),
						min((dxOutline+cxOutline), GetClipLeft() + GetClipWidth()-1),
						(dyOutline+cyOutline), eor_mask);
			if ((dxOutline+cxOutline) >= GetClipLeft() && (dxOutline+cxOutline) < GetClipLeft() + GetClipWidth() &&
				(dyOutline+cyOutline) - 1 >= GetClipTop() && dyOutline <= GetClipTop() + GetClipHeight()-1)
				XORvLine(DisplayBM,
						(dxOutline+cxOutline),
						max(dyOutline, GetClipTop()),
						min((dyOutline+cyOutline) - 1, GetClipTop() + GetClipHeight()-1), eor_mask);
			if (dxOutline >= GetClipLeft() && dxOutline < GetClipLeft() + GetClipWidth() &&
				(dyOutline+cyOutline) - 1 >= GetClipTop() && dyOutline <= GetClipTop() + GetClipHeight()-1)
				XORvLine(DisplayBM, dxOutline, max(dyOutline, GetClipTop()), min((dyOutline+cyOutline) - 1, GetClipTop() + GetClipHeight()-1), eor_mask);
		}
		else
		{
			if (cxOutline == 0)
			{
				if (dxOutline >= GetClipLeft() && dxOutline < GetClipLeft() + GetClipWidth() &&
					(dyOutline+cyOutline) - 1 >= GetClipTop() && dyOutline <= GetClipTop() + GetClipHeight()-1)
					XORvLine(DisplayBM, dxOutline, max(dyOutline, GetClipTop()), min((dyOutline+cyOutline), GetClipTop() + GetClipHeight()-1), eor_mask);
			}
			else if (cyOutline == 0)
			{
				if ((dyOutline+cyOutline) >= GetClipTop() && (dyOutline+cyOutline) < GetClipTop() + GetClipHeight() &&
					(dxOutline+cxOutline) >= GetClipLeft() && dxOutline <= GetClipLeft() + GetClipWidth()-1)
					XORhLine(DisplayBM,
							max(dxOutline, GetClipLeft()),
							min((dxOutline+cxOutline), GetClipLeft() + GetClipWidth()-1),
							dyOutline, eor_mask);
			}
		}
		AfterGraphics();
	}
} /* DrawSelectionOutline */
#endif  // PIXELSELECT


/*********************************************************************
 *
 * DrawTileOutline
 *
 * PURPOSE
 *		<dx> & <dy> are pixel offsets of the tile origin. <dtilx> and
 *		<dtily> are the tile offsets from the tile origin. <ctilx> and
 *		<ctily> defines how many tiles to outline. <cxTile> & <cyTile>
 *		defines the size of each tile, and <sep> defines whether tiles
 *		are separated.
 *
 * INPUT
 *		short dx		: x-offset to draw outline
 *		short dy		: y-offset to draw outline
 *		short cx		: width to draw outline within
 *		short cy		: height to draw outline within
 *		short dtilx	: x-tile offset to draw outline
 *		short dtily	: y-tile offset to draw outline
 *		short ctilx	: width of outline in tiles to draw
 *		short ctily	: height of outline in tiles to draw
 *		UWORD cxTile	: width of a single tile in pixels
 *		UWORD cyTile	: height of a single tile in pixels
 *		short sep		: if !0, separate tiles with 1 pixel layer
 *		UBYTE eor_mask	: byte value to XOR screen with
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		09/07/94 (dcc) - change <dtilx> from offset within screen
 *						to offset within current room
 *		09/07/94 (dcc) - add playBig which looks-up size of biggest layer
 *
 * SEE ALSO
 *
*/
void DrawTileOutline(int dx, int dy, int cx, int cy, RoomStuffType *prst,
				int dtilx, int dtily, int ctilx, int ctily,
				UWORD cxTile, UWORD cyTile, int sep, UBYTE eor_mask)
{
	int cxTilDst, cyTilDst;
	int dxOutline, dyOutline;
	int cxOutline, cyOutline;
	LayerType *playBig;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawTileOutline";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Find layer with the largest tiles */

	playBig = FindBiggestLayer(&prst->Room->Layers);
	if (!playBig)
		return;

	cxTilDst	= wSrcSizeToDstSize(playBig->cxTile, wSrcSkipX, wDstDupX) + sep;
	cyTilDst	= wSrcSizeToDstSize(playBig->cyTile, wSrcSkipY, wDstDupY) + sep;

	/* figure out where to draw */

	Assert(playBig->cxTile != 0);
	dxOutline = (int) (((ULONG) dtilx * cxTile - prst->dxSrc)
				* cxTilDst / playBig->cxTile) + dx;
	Assert(playBig->cyTile != 0);
	dyOutline = (int) (((ULONG) dtily * cyTile - prst->dySrc)
				* cyTilDst / playBig->cyTile) + dy;

	if (cxTile != playBig->cxTile)
		dxOutline += (sep ? 1 : 0);

	if (cyTile != playBig->cyTile)
		dyOutline += (sep ? 1 : 0);

	cxOutline = (int) (((ULONG) (dtilx+ctilx) * cxTile - prst->dxSrc)
				* cxTilDst / playBig->cxTile) + dx - dxOutline;
	cyOutline = (int) (((ULONG) (dtily+ctily) * cyTile - prst->dySrc)
				* cyTilDst / playBig->cyTile) + dy - dyOutline;

	if (cxTile == playBig->cxTile)
		cxOutline	+= (sep ? 0 : -1);

	if (cyTile == playBig->cyTile)
		cyOutline	+= (sep ? 0 : -1);

	if (	!(dyOutline < dy && (dyOutline+cyOutline) < dy) &&
		!(dyOutline >= dy+cy && (dyOutline+cyOutline) >= dy+cy) &&
		!(dxOutline < dx && (dxOutline+cxOutline) < dx) &&
		!(dxOutline >= dx+cx && (dxOutline+cxOutline) >= dx+cx) &&
		dxOutline <= (dxOutline+cxOutline) && dyOutline <= (dyOutline+cyOutline))
	{
		BeforeGraphics();

		if (dyOutline >= GetClipTop() && dyOutline < GetClipTop() + GetClipHeight() &&
			(dxOutline+cxOutline) - 1 >= GetClipLeft() && dxOutline + 1 <= GetClipLeft() + GetClipWidth()-1)
			XORhLine(DisplayBM,
					max(dxOutline + 1, GetClipLeft()),
					min((dxOutline+cxOutline) - 1, GetClipLeft() + GetClipWidth()-1),
					dyOutline, eor_mask);
		if ((dyOutline+cyOutline) >= GetClipTop() && (dyOutline+cyOutline) < GetClipTop() + GetClipHeight() &&
			(dxOutline+cxOutline) >= GetClipLeft() && dxOutline <= GetClipLeft() + GetClipWidth()-1)
			XORhLine(DisplayBM,
					max(dxOutline, GetClipLeft()),
					min((dxOutline+cxOutline), GetClipLeft() + GetClipWidth()-1),
					(dyOutline+cyOutline), eor_mask);
		if ((dxOutline+cxOutline) >= GetClipLeft() && (dxOutline+cxOutline) < GetClipLeft() + GetClipWidth() &&
			(dyOutline+cyOutline) - 1 >= GetClipTop() && dyOutline <= GetClipTop() + GetClipHeight()-1)
			XORvLine(DisplayBM,
					(dxOutline+cxOutline),
					max(dyOutline, GetClipTop()),
					min((dyOutline+cyOutline) - 1, GetClipTop() + GetClipHeight()-1), eor_mask);
		if (dxOutline >= GetClipLeft() && dxOutline < GetClipLeft() + GetClipWidth() &&
			(dyOutline+cyOutline) - 1 >= GetClipTop() && dyOutline <= GetClipTop() + GetClipHeight()-1)
			XORvLine(DisplayBM, dxOutline, max(dyOutline, GetClipTop()), min((dyOutline+cyOutline) - 1, GetClipTop() + GetClipHeight()-1), eor_mask);

		AfterGraphics();
	}
} /* DrawTileOutline */


#if PIXELSELECT
/**************************************************************************
 *
 * DrawPixelSelect
 *
 * PURPOSE
 *		To draw the selection box and/or the draw cursor.
 *
 *		This version draws an exact pixel selection box.
 *
 *		NOTE: selection box will be clipped to destination room ctilx & ctily.
 *
 * INPUT
 *		pblk		: BlockCopyType to get selection box size from
 *		cxTile	: each tile is this wide
 *		cyTile	: each tile is this high
 *		eor_mask	: pixel value to exclusive-or image with
 *		destflag	: if TRUE get info from <pblk->Dest>
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		01/12/94 Wednesday (dcc) - created based on DrawSelect().
 *
 * SEE ALSO
 *
*/
void DrawPixelSelect (
	BlockCopyType	*block,
	UWORD		cxTile,
	UWORD		cyTile,
	UBYTE		eor_mask,
	BOOL			destflag
)
{
	RoomStuffType		*roomstuff;
	WORD				row, column;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawPixelSelect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (destflag)
	{
		roomstuff		= block->DestStuff;
		column		= block->DestX;
		row			= block->DestY;
	}
	else
	{
		roomstuff		= block->SourceStuff;
		column		= block->SourceX;
		row			= block->SourceY;
	}

  {
	short ctilxMin = min(block->ctilxSelect,
				((LayerType *) Head(&roomstuff->Room->Layers))->ctilx - column);

	short ctilyMin = min(block->ctilySelect,
				((LayerType *) Head(&roomstuff->Room->Layers))->ctily - row);

	Assert(cxTile != 0);
	Assert(cyTile != 0);
	DrawSelectionOutline(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd,
			(column - (int) (roomstuff->dxSrc / cxTile)),
			(row    - (int) (roomstuff->dySrc / cyTile)),
			ctilxMin, ctilyMin,
			block->dxModSrcL, block->dyModSrcT,
			block->dxModSrcR, block->dyModSrcB,
			cxTile, cyTile,
			((roomstuff->Flags) & DISPLAY_SEPERATED), eor_mask);
  }
	return;
} /* DrawPixelSelect */
#endif  // PIXELSELECT


/**************************************************************************
 *
 * DrawSelect
 *
 * PURPOSE
 *		To draw the selection box and/or the draw cursor.
 *
 *		NOTE: selection box will be clipped to destination room ctilx & ctily.
 *
 * INPUT
 *		pblk		: BlockCopyType to get selection box size from
 *		ctilx	: selection box is this many tiles wide
 *		ctily	: selection box is this many tiles high
 *		cxTile	: each tile is this wide
 *		cyTile	: each tile is this high
 *		eor_mask	: pixel value to exclusive-or image with
 *		destflag	: if TRUE get info from <pblk->Dest>
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		03/11/92 Wednesday - support added for locked composite rooms (DCC)
 *		10/06/89 Friday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void DrawSelect (
	BlockCopyType	*block,
	short		ctilx,
	short		ctily,
	UWORD		cxTile,
	UWORD		cyTile,
	UBYTE		eor_mask,
	BOOL			destflag
)
{
	RoomStuffType		*roomstuff;
	WORD				row, column;
#if __AMIGAOS__
	WORD				plot_height, plot_width;
	int				left, right, upper, lower;
	struct Window		*window;
	struct RastPort	*rp;
	UBYTE			inverse_eor_mask;
	UBYTE			savemask;
	WORD				sep;
#endif/*__AMIGAOS__*/

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DrawSelect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

//	if (FSelectedPblk(block))
	{
#if __AMIGAOS__
		inverse_eor_mask	= eor_mask ^ ((UBYTE) 0xFF);
#endif/*__AMIGAOS__*/
		if (destflag)
		{
#if __AMIGAOS__
			window		= block->DestRW->Window;
#endif/*__AMIGAOS__*/
			roomstuff		= block->DestStuff;
			column		= block->DestX;
			row			= block->DestY;
		}
		else
		{
#if __AMIGAOS__
			window		= block->SourceRW->Window;
#endif/*__AMIGAOS__*/
			roomstuff		= block->SourceStuff;
			column		= block->SourceX;
			row			= block->SourceY;
		}

#if __AMIGAOS__
		rp = window->RPort;

		sep			= ((roomstuff->Flags) & DISPLAY_SEPERATED);
		plot_width	= wSrcSizeToDstSize(cxTile, wSrcSkipX, wDstDupX) + sep;
		plot_height	= wSrcSizeToDstSize(cyTile, wSrcSkipY, wDstDupY) + sep;

		/**************************/
		/* find out where to draw */

		upper	= ( (row    - dtilySrc) * (plot_height) ) + TopAdd;
		left		= ( (column - dtilxSrc) * (plot_width) );
		lower = min(ctily,
				  ((LayerType *) Head(&roomstuff->Room->Layers))->ctily - row) *
					(plot_height) + upper + (sep ? 0 : -1);
		right = min(ctilx,
				  ((LayerType *) Head(&roomstuff->Room->Layers))->ctilx - column) *
					(plot_width) + left + (sep ? 0 : -1);

		/**************************/
		/* set up draw parameters */

		SetDrMd (rp, COMPLEMENT);
		SetAPen (rp, 1);
		savemask	= rp->Mask;

		/*******************/
		/* draw the cursor */

		rp->Mask	= eor_mask;
		Move (rp, left + 1, upper);
		Draw (rp, right - 1, upper);
		Move (rp, left, lower);
		Draw (rp, right, lower);

		rp->Mask	= inverse_eor_mask;
		Move (rp, right, upper);
		Draw (rp, right, lower - 1);
		Move (rp, left, upper);
		Draw (rp, left, lower - 1);

		/*******************/
		/* restore rp mask */

		rp->Mask = savemask;
#elif __MSDOS__
	{
		int ctilxMin = min(ctilx,
				  (roomstuff->Room->FloorLayer->ctilx - column));

		int ctilyMin = min(ctily,
				  (roomstuff->Room->FloorLayer->ctily - row));

		DrawTileOutline(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd,
				roomstuff,
				column, row,
				ctilxMin, ctilyMin,
				cxTile, cyTile,
				((roomstuff->Flags) & DISPLAY_SEPERATED), eor_mask);
	}
#endif/*__AMIGAOS__/__MSDOS__*/
	}

	return;
} /* DrawSelect */


/*********************************************************************
 *
 * XORDrawPBlk
 *
 * PURPOSE
 *		XOR draw select box. Get tile width, tile height, selection width,
 *		and selection height from first non-empty layer of <pblk>.
 *
 *		Draw an exact pixel selection box if fPixelSelect is TRUE.
 *
 * INPUT
 *		pblk		: BlockCopyType to get selection box size from
 *		eor_mask	: pixel value to exclusive-or image with
 *		destflag	: if TRUE get info from <pblk->Dest>
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		07/07/94 (dcc) - Add support for fPixelSelect.
 *		07/18/94 (dcc) - Call FindNonEmptyLayer() AFTER verifying FSelectedPblk().
 *		03/26/95 (dcc) - Abort if FindNonEmptyLayer() returns NULL.
 *
 * SEE ALSO
 *
*/
void XORDrawPBlk(BlockCopyType *pblk, UBYTE eor_mask, BOOL destflag)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "XORDrawPBlk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (FSelectedPblk(pblk))
	{
		LayerType *play = FindNonEmptyLayer(&pblk->Layers, pblk->BC_FloorLayer);

		if (!play)
			return;				// ABORT

		if (fPixelSelect)
			DrawPixelSelect(pblk, play->cxTile, play->cyTile,
						eor_mask, destflag);
		else
			DrawSelect(pblk, play->ctilx, play->ctily,
					play->cxTile, play->cyTile,
					eor_mask, destflag);
	}
} /* XORDrawPBlk */
#endif /* !TEST_PRINT */


#if 0
Fine and dandy, but we're not going to use it...
/*********************************************************************
 *
 * ClipSubRectToDisplay
 *
 * PURPOSE
 *		Clip variables to display.
 *
 * INPUT
 *		*dtilDstX			: x tile-offset within screen to start drawing
 *		*dtilDstY			: y tile-offset within screen to start drawing
 *		*dtilx			: x tile-offset within floor layer to start drawing
 *		*dtily			: y tile-offset within floor layer to start drawing
 *		*ctilx			: number of tiles wide to draw
 *		*ctily			: number of tiles high to draw
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns FALSE if area doesn't need to be drawn at all.
 *
 * HISTORY
 *		07/08/93 Thursday (dcc) - created.
 *
*/
BOOL ClipSubRectToDisplay(short *dtilDstX, short *dtilDstY,
					short *dtilx, short *dtily, short *ctilx, short *ctily)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ClipSubRectToDisplay";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (*dtilDstX < 0)
	{
		*dtilx -= *dtilDstX;
		if (*ctilx)
		{
			*ctilx += *dtilDstX;
			if (*ctilx <= 0)
				return FALSE;
		}
		*dtilDstX = 0;
	}
	if (*dtilDstY < 0)
	{
		*dtily -= *dtilDstY;
		if (*ctily)
		{
			*ctily += *dtilDstY;
			if (*ctily <= 0)
				return FALSE;
		}
		*dtilDstY = 0;
	}
	return TRUE;
} /* ClipSubRectToDisplay */
#endif


int ctilxOverhang = 0;
int ctilyOverhang = 0;


#if 1 // BUGBUG PixelLayers
/**************************************************************************
 *
 * ShowRoomRectObjectLayer
 *
 * PURPOSE
 *		Draw <layer> in <destBM>.
 *
 *		Zoom is expressed as wDstDupX:wSrcSkipX. For example 4:1 means
 *		every pixel in source tile is magnified four times. 1:2 means
 *		every other pixel in source is displayed in destination.
 *
 *		The burden of pixel level clipping is place upon the caller
 *		(it should call SetClipValues() or PushClipValues()).
 *
 *		This routine should only be called for object tiles (tiles
 *		places in a 1x1 grid that are display as their numbers).
 *
 *		The <cxTile> and <cyTile> refer to non-object layers.
 *
 * INPUT
 *		showcolumn	: tile column offset (from dxDst) to start drawing RectObjectLayer (0=left)
 *		showrow		: tile row offset (from dyDst) to start drawing RectObjectLayer(0=top row)
 *		dtilxSrc		: tile column offset within <layer> to start drawing
 *		dtilySrc		: tile row offset withing <layer> to start drawing
 *		ctilx		: number of tiles to display across (or 0 for max across)
 *		ctily		: number of tiles to display down (or 0 for max down)
 *		dxDst		: x-offset of left edge within screen to draw (in pixels)
 *		dyDst		: y-offset of top edge within screen to draw (in pixels)
 *		cxDst		: width of area on screen to draw (in pixels)
 *		cyDst		: height of area on screen to draw (in pixels)
 *		cxTile		: width of a single tile in non-object layer
 *		cyTile		: height of a single tile in non-object layer
 *		sep			: 1 if display is separated
 *
 * ASSUMES
 *		That the caller has set the clip limits already.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/25/94 (dcc) - created based upon ShowRoomRectLayer()
 *		09/06/94 (dcc) - added dxModcxTile, dyModcyTile
 *		10/13/94 (dcc) - Redefined: this routine is called whenever tiles
 *					  are 1 pixel wide by 1 pixel high. So add
 *					  #if PixelLayers inside this routine.
 *
*/
static void ShowRoomRectObjectLayer (
		ByteMap		*destBM,
		LayerType		*layer,
		int			showcolumn,
		int			showrow,
		int			dtilxSrc,
		int			dtilySrc,
		int			ctilx,
		int			ctily,
		int			dxDst,
		int			dyDst,
		int			cxDst,
		int			cyDst,
		int			cxTile,
		int			cyTile,
		int			sep,
		BOOL			notransp_flag
)
{
register	PlotType		*plot;
			int			row;
			int			column;
			WORD			maxrow, maxcolumn;
register	LONG			lDstDispWidth;
			LONG			lDstDispHeight;
//			LONG			lWindowWidth, lWindowHeight;
			short		ctilxPlay;
			LONG			lInitDstXxSrcSkipX;
			LONG			lInitDstYxSrcSkipY;

			int			ctilxO;	/* extra tiles to draw off left */
			int			ctilyO;	/* extra tiles to draw off top */

			int			dxModcxTile;	/* brush offset within big tile */
			int			dyModcyTile;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowRoomRectObjectLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

//	if (EditOnlyFloorSTATE && (layer != roomwindow->CurrentRoom->Room->FloorLayer)) goto ABORT;

	dtilxSrc += layer->dtilx;
	if (dtilxSrc < 0)
	{
		showcolumn -= dtilxSrc;
		if (ctilx)
		{
			ctilx += dtilxSrc;
			if (ctilx <= 0)
/**/			goto ABORT;
		}
		dtilxSrc = 0;
	}
	dtilySrc += layer->dtily;
	if (dtilySrc < 0)
	{
		showrow -= dtilySrc;
		if (ctily)
		{
			ctily += dtilySrc;
			if (ctily <= 0)
/**/			goto ABORT;
		}
		dtilySrc = 0;
	}
	
	if (showcolumn < 0)
	{
		dtilxSrc -= showcolumn;
		if (ctilx)
		{
			ctilx += showcolumn;
			if (ctilx <= 0)
/**/			goto ABORT;
		}
		showcolumn = 0;
	}
	if (showrow < 0)
	{
		dtilySrc -= showrow;
		if (ctily)
		{
			ctily += showrow;
			if (ctily <= 0)
/**/			goto ABORT;
		}
		showrow = 0;
	}

	cxDst = cxDst;	//KLUDGE BUGBUG FIXME
	cyDst = cyDst;
//	lWindowWidth	= (LONG) (cxDst + cpixyObjectTile * 2) * wSrcSkipX;
//	lWindowHeight	= (LONG) (cyDst + cpixyObjectTile * 2) * wSrcSkipY;

	ctilxPlay		= layer->ctilx;

	ctilxO = min(ctilxOverhang, dtilxSrc);
	ctilyO = min(ctilyOverhang, dtilySrc);

	dtilxSrc -= ctilxO;
	showcolumn -= ctilxO;
	if (ctilx)
		ctilx += ctilxO;

	dtilySrc -= ctilyO;
	showrow -= ctilyO;
	if (ctily)
		ctily += ctilyO;

	lDstDispWidth	= (LONG) (wSrcSizeToDstSize(cxTile, wSrcSkipX, wDstDupX) + sep) * wSrcSkipX;
	lDstDispHeight	= (LONG) (wSrcSizeToDstSize(cyTile, wSrcSkipY, wDstDupY) + sep) * wSrcSkipY;

	if (fDrawNULLTiles)
	{
		lNULLTileDispWidth	= (LONG) wSrcSizeToDstSize(cxTile, wSrcSkipX, wDstDupX);
		lNULLTileDispHeight	= (LONG) wSrcSizeToDstSize(cyTile, wSrcSkipY, wDstDupY);
	}

	if (ctilx)
	{
		maxcolumn	= min (ctilxPlay, ctilx + dtilxSrc);
	}
	else
	{
		maxcolumn	= ctilxPlay;
	}

	if (ctily)
	{
		maxrow	= min (layer->ctily, ctily + dtilySrc);
	}
	else
	{
		maxrow	= layer->ctily;
	}

	dxModcxTile = showcolumn % cxTile;
	dyModcyTile = showrow % cyTile;

	lInitDstXxSrcSkipX =lDstDispWidth *(showcolumn-dxModcxTile)*layer->cxTile/cxTile
					+(LONG)(sep+dxDst)*wSrcSkipX;
	lInitDstYxSrcSkipY =lDstDispHeight*(showrow   -dyModcyTile)*layer->cyTile/cyTile
					+(LONG)(sep+dyDst)*wSrcSkipY;

//	maxcolumn	= min (maxcolumn,(WORD)((lWindowWidth *cxTile/lDstDispWidth +ctilxOverhang+1)+dtilxSrc));
//	maxrow	= min (maxrow,	  (WORD)((lWindowHeight*cyTile/lDstDispHeight+ctilyOverhang+1)+dtilySrc));

	if (wShowXTiles > 0)
		maxcolumn = min(maxcolumn, wShowXTiles + dtilxSrc + ctilxOverhang);

	if (wShowYTiles > 0)
		maxrow = min(maxrow, wShowYTiles + dtilySrc + ctilyOverhang);

#if __MSDOS__
	BeforeGraphics();
#endif/*__MSDOS__*/

	/*****************/
	/* plot the rows */

#if UseSparseArray
	if (layer && layer->ctilx && layer->ctily)	/* Make sure layer exists. */
#else // !UseSparseArray
	if (layer->rgrgplt.mpYtMpXtPt)			/* Make sure layer exists. */
#endif // !UseSparseArray
	{
		if (IsSparse(layer))
		{
			SetSparseLayerLimits(layer->p.pspa, dtilxSrc, dtilySrc,
							 maxcolumn-dtilxSrc, maxrow-dtilySrc);

			while ((plot = GetNextSparsePlotInRange(layer->p.pspa,
											&column, &row)) != NULL)
			{
				LONG lx, ly;

				Assert(cxTile != 0);
				lx = lInitDstXxSrcSkipX + (column + dxModcxTile - dtilxSrc) * lDstDispWidth  / cxTile;
				Assert(cyTile != 0);
				ly = lInitDstYxSrcSkipY + (row    + dyModcyTile - dtilySrc) * lDstDispHeight / cyTile;

#if PixelLayers
				StampTileAsNumbers(destBM, plot, lx, ly, notransp_flag, 0);
#else // !PixelLayers
				StampTile(destBM, plot, lx, ly, notransp_flag, 0);
#endif // !PixelLayers
			}
		}
		else
		{
			for (row = dtilySrc; row < maxrow; row++)
			{
				column	= dtilxSrc;

				/*********************/
				/* plot a single row */

#if UseSparseArray
				plot = ActivatePlotXYatWin(layer->p.rgrgplt, dtilxSrc, row, winDst);
#else // !UseSparseArray
				plot = ActivatePlotXYatWin(layer->rgrgplt, dtilxSrc, row, winDst);
#endif // !UseSparseArray

				for (column = dtilxSrc; column < maxcolumn; column++)
				{
					if (!fZeroTile(plot))
					{
						LONG lx, ly;

						Assert(cxTile != 0);
						lx = lInitDstXxSrcSkipX + (column + dxModcxTile - dtilxSrc) * lDstDispWidth  / cxTile;
						Assert(cyTile != 0);
						ly = lInitDstYxSrcSkipY + (row    + dyModcyTile - dtilySrc) * lDstDispHeight / cyTile;

#if PixelLayers
						StampTileAsNumbers(destBM, plot, lx, ly, notransp_flag, 0);
#else // !PixelLayers
						StampTile(destBM, plot, lx, ly, notransp_flag, 0);
#endif // !PixelLayers
					}
					plot++;
				}
#if UseSparseArray
				ReleasePlotRow(layer->p.rgrgplt, row);
#else // !UseSparseArray
				ReleasePlotRow(layer->rgrgplt, row);
#endif // !UseSparseArray
			}
		}
	}
#if __MSDOS__
	AfterGraphics();
#endif/*__MSDOS__*/

ABORT:
	return;

} /* ShowRoomRectObjectLayer */
#endif // 1 BUGBUG // PixelLayers


/**************************************************************************
 *
 * ShowRoomRectLayer
 *
 * PURPOSE
 *		Draw <layer> in <destBM>.
 *
 *		Zoom is expressed as wDstDupX:wSrcSkipX. For example 4:1 means
 *		every pixel in source tile is magnified four times. 1:2 means
 *		every other pixel in source is displayed in destination.
 *
 *		The burden of pixel level clipping is place upon the caller
 *		(it should call SetClipValues() or PushClipValues()).
 *
 * INPUT
 *		showcolumn	: tile column offset (from dxDst) to start drawing RectLayer (0=left)
 *		showrow		: tile row offset (from dyDst) to start drawing RectLayer(0=top row)
 *		dtilxSrc		: tile column offset within <layer> to start drawing
 *		dtilySrc		: tile row offset withing <layer> to start drawing
 *		ctilx		: number of tiles to display across (or 0 for max across)
 *		ctily		: number of tiles to display down (or 0 for max down)
 *		dxDst		: x-offset of left edge within screen to draw (in pixels)
 *		dyDst		: y-offset of top edge within screen to draw (in pixels)
 *		cxDst		: width of area on screen to draw (in pixels)
 *		cyDst		: height of area on screen to draw (in pixels)
 *
 * ASSUMES
 *		That the caller has set the clip limits already.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		10/10/89 (RGM) - Created out of old ShowRoom, generalized,
 *					  optimized.
 *		01/16/92 (dcc) - Burden of checking <EditOnlyFloorSTATE>
 *					  placed on calling routine.
 *		01/28/92 (dcc) - Support added for zoomed display.
 *		03/11/92 (dcc) - Support added for locked composite rooms.
 *		03/19/92 (dcc) - Make routine clip off top & left edges.
 *		05/30/93 (dcc) - Add fCompositeSource flag (instead of
 *					  trying to determine value internally).
 *		11/05/93 (dcc) - Fix lInitDstXxSrcSkipX computation (add (LONG)).
 *		02/20/94 (dcc) - Add support for play->dtilx, ->dtily.
 *		03/02/94 (dcc) - Add support for ctilxOverhang, ctilyOverhang.
 *		08/21/94 (dcc) - Add support for UseSparseArray.
 *		03/26/95 (dcc) - Check for FindNonEmptyLayer() returning NULL.
 *		03/26/95 (dcc) - Add check for 0 lDstDispWidth, lDstDispHeight.
 *
*/
static void ShowRoomRectLayer (
		ByteMap		*destBM,
		LayerType		*layer,
		RoomWindowType	*roomwindow,
		int			showcolumn,
		int			showrow,
		int			dtilxSrc,
		int			dtilySrc,
		int			ctilx,
		int			ctily,
		int			dxDst,
		int			dyDst,
		int			cxDst,
		int			cyDst,
		BOOL			fCompositeSource,
		BOOL			notransp_flag
)
{
register	PlotType		*plot;
#if PLOTARRAY
			PlotType		*temp_plot;
#endif
			int			row;
			int			column;
			int			maxrow, maxcolumn;
register	LONG			lDstXxSrcSkipX;
			LONG			lDstYxSrcSkipY;
register	LONG			lDstDispWidth;
			LONG			lDstDispHeight;
			LONG			lWindowWidth, lWindowHeight;
			WORD			sep;
			short		ctilxPlay;
#if __AMIGAOS__
struct		Window		*window;
struct		RastPort		*rp;
#endif/*__AMIGAOS__*/
			RoomType		*room;
			RoomStuffType	*roomstuff;
			LONG			lInitDstXxSrcSkipX;
			LONG			lInitDstYxSrcSkipY;

			int			tilewidth;	/* tile pixel width */
			int			tileheight;	/* tile pixel height */

			int			ctilxO;	/* extra tiles to draw off left */
			int			ctilyO;	/* extra tiles to draw off top */

#if __AMIGAOS__
			struct BitMap	*rpbitmap;
#endif/*__AMIGAOS__*/

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowRoomRectLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

//	if (EditOnlyFloorSTATE && (layer != roomwindow->CurrentRoom->Room->FloorLayer)) goto ABORT;

	dtilxSrc += layer->dtilx;
	if (dtilxSrc < 0)
	{
		showcolumn -= dtilxSrc;
		if (ctilx)
		{
			ctilx += dtilxSrc;
			if (ctilx <= 0)
/**/			goto ABORT;
		}
		dtilxSrc = 0;
	}
	dtilySrc += layer->dtily;
	if (dtilySrc < 0)
	{
		showrow -= dtilySrc;
		if (ctily)
		{
			ctily += dtilySrc;
			if (ctily <= 0)
/**/			goto ABORT;
		}
		dtilySrc = 0;
	}
	
	if (showcolumn < 0)
	{
		dtilxSrc -= showcolumn;
		if (ctilx)
		{
			ctilx += showcolumn;
			if (ctilx <= 0)
/**/			goto ABORT;
		}
		showcolumn = 0;
	}
	if (showrow < 0)
	{
		dtilySrc -= showrow;
		if (ctily)
		{
			ctily += showrow;
			if (ctily <= 0)
/**/			goto ABORT;
		}
		showrow = 0;
	}

#if __AMIGAOS__
	window		= roomwindow->Window;
#endif/*__AMIGAOS__*/
	lWindowWidth	= (LONG) cxDst * wSrcSkipX;
	lWindowHeight	= (LONG) cyDst * wSrcSkipY;

#if __AMIGAOS__
	rp			= window->RPort;
	rpbitmap		= rp->BitMap;
#endif/*__AMIGAOS__*/

	roomstuff		= roomwindow->CurrentRoom;
	room			= roomstuff->Room;
	ctilxPlay		= layer->ctilx;
	if ((tilewidth = layer->cxTile) == 0)
	{
#if !TEST_PRINT
		RoomType *prm;
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
		if (play)
			tilewidth = play->cxTile;
#endif
	}
	if ((tileheight = layer->cyTile) == 0)
	{
#if !TEST_PRINT
		RoomType *prm;
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
		if (play)
			tileheight = play->cyTile;
#endif
	}
	ctilxO = min(ctilxOverhang, dtilxSrc);
	ctilyO = min(ctilyOverhang, dtilySrc);

	dtilxSrc -= ctilxO;
	showcolumn -= ctilxO;
	if (ctilx)
		ctilx += ctilxO;

	dtilySrc -= ctilyO;
	showrow -= ctilyO;
	if (ctily)
		ctily += ctilyO;

	sep			= 0 + ((roomstuff->Flags) & (DISPLAY_SEPERATED));

	lDstDispWidth	= (LONG) (wSrcSizeToDstSize(tilewidth,  wSrcSkipX, wDstDupX) + sep) * wSrcSkipX;
	lDstDispHeight	= (LONG) (wSrcSizeToDstSize(tileheight, wSrcSkipY, wDstDupY) + sep) * wSrcSkipY;

	if (lDstDispWidth == 0 || lDstDispHeight == 0)
/**/	goto ABORT;

	if (fDrawNULLTiles)
	{
		lNULLTileDispWidth	= (LONG) wSrcSizeToDstSize(tilewidth,  wSrcSkipX, wDstDupX);
		lNULLTileDispHeight	= (LONG) wSrcSizeToDstSize(tileheight, wSrcSkipY, wDstDupY);
	}

	if (ctilx)
	{
		maxcolumn	= min (ctilxPlay, ctilx + dtilxSrc);
	}
	else
	{
		maxcolumn	= ctilxPlay;
	}

	if (ctily)
	{
		maxrow	= min (layer->ctily, ctily + dtilySrc);
	}
	else
	{
		maxrow	= layer->ctily;
	}

	lInitDstXxSrcSkipX =lDstDispWidth *showcolumn+(LONG)(sep+dxDst)*wSrcSkipX;
	lInitDstYxSrcSkipY =lDstDispHeight*showrow   +(LONG)(sep+dyDst)*wSrcSkipY;

	maxcolumn	= min (maxcolumn,(WORD)((lWindowWidth /lDstDispWidth +ctilxOverhang+1)+dtilxSrc));
	maxrow	= min (maxrow,	  (WORD)((lWindowHeight/lDstDispHeight+ctilyOverhang+1)+dtilySrc));

	if (wShowXTiles > 0)
		maxcolumn = min(maxcolumn, wShowXTiles + dtilxSrc + ctilxOverhang);

	if (wShowYTiles > 0)
		maxrow = min(maxrow, wShowYTiles + dtilySrc + ctilyOverhang);

#if __MSDOS__
	BeforeGraphics();
#endif/*__MSDOS__*/

	/*****************/
	/* plot the rows */

	lDstYxSrcSkipY	= lInitDstYxSrcSkipY;

#if PLOTARRAY
	plot		= (layer->Plot);
	plot		+= ((dtilySrc * ctilxPlay) + dtilxSrc);
#endif

	#if !TEST_PRINT
#if UseSparseArray
	if (layer && layer->ctilx && layer->ctily)	/* Make sure layer exists. */
#else // !UseSparseArray
	if (layer->rgrgplt.mpYtMpXtPt)			/* Make sure layer exists. */
#endif // !UseSparseArray
	#endif
	{
		for (row = dtilySrc; row < maxrow; row++)
		{
			lDstXxSrcSkipX	= lInitDstXxSrcSkipX;
			column	= dtilxSrc;

			#if !TEST_PRINT
			if (fCompositeSource)
			{
				PlotType plot;

				for (column = dtilxSrc; column < maxcolumn; column++)
				{
					if (column < layer->ctilx && row < layer->ctily)
					{
						plot.TileSet_ID = room->ptsComposite->TS_id;
						plot.Tile_ID = row * layer->ctilx + column + 1;
					}
					else
					{
						plot.TileSet_ID = 0;
						plot.Tile_ID = 0;
					}
					plot.Plot_Flags = 0;
#if PixelLayers
					// KLUDGE BUGBUG FIXME display object tiles as numbers
					if (layer->cxTile == 1 && layer->cyTile == 1)
						StampTileAsNumbers(destBM, &plot, lDstXxSrcSkipX, lDstYxSrcSkipY, notransp_flag, 0);
					else
						StampTile(destBM, &plot, lDstXxSrcSkipX, lDstYxSrcSkipY, notransp_flag, 0);
#else // !PixelLayers
					StampTile(destBM, &plot, lDstXxSrcSkipX, lDstYxSrcSkipY, notransp_flag, 0);
#endif // !PixelLayers
					lDstXxSrcSkipX	+= lDstDispWidth;
				}
			}
			else
			#endif
			{
				/*********************/
				/* plot a single row */

				#if TEST_PRINT
				plot = (PlotType *) ActivateXTRAatWin(layer->Plot[row], winDst)+dtilxSrc;
				#else
#if UseSparseArray
				if (!IsSparse(layer))
					plot = ActivatePlotXYatWin(layer->p.rgrgplt, dtilxSrc, row, winDst);
#else // !UseSparseArray
				plot = ActivatePlotXYatWin(layer->rgrgplt, dtilxSrc, row, winDst);
#endif // !UseSparseArray
				#endif

#if PLOTARRAY
				temp_plot	= plot;
#endif
				for (column = dtilxSrc; column < maxcolumn; column++)
				{
#if UseSparseArray
					if (IsSparse(layer))
						plot = GetSparsePlotXY(layer->p.pspa, column, row);
#endif // UseSparseArray
#if PixelLayers
					// KLUDGE BUGBUG FIXME display object tiles as numbers
					if (layer->cxTile == 1 && layer->cyTile == 1)
						StampTileAsNumbers(destBM, plot, lDstXxSrcSkipX, lDstYxSrcSkipY, notransp_flag, 0);
					else
						StampTile(destBM, plot, lDstXxSrcSkipX, lDstYxSrcSkipY, notransp_flag, 0);
#else // !PixelLayers
					StampTile(destBM, plot, lDstXxSrcSkipX, lDstYxSrcSkipY, notransp_flag, 0);
#endif // !PixelLayers
					plot++;
					lDstXxSrcSkipX	+= lDstDispWidth;
				}
#if PLOTARRAY
				plot		= temp_plot;
				plot		+= ctilxPlay;
#endif
			}
			lDstYxSrcSkipY	+= lDstDispHeight;
			#if TEST_PRINT
			ReleaseXTRA(layer->Plot[row]);
			#else
#if UseSparseArray
			if (!IsSparse(layer))
				ReleasePlotRow(layer->p.rgrgplt, row);
#else // !UseSparseArray
			ReleasePlotRow(layer->rgrgplt, row);
#endif // !UseSparseArray
			#endif
		}
	}
#if __MSDOS__
	AfterGraphics();
#endif/*__MSDOS__*/

ABORT:
	return;

} /* ShowRoomRectLayer */


#if NEWTUME
/*********************************************************************
 *
 * lmod
 *
 * PURPOSE
 *		Compute <l> mod <divisor>. Special code to handle negative
 *		number in a "consistent" fashion.
 *
 *		Also just returns <l> if <divisor> is 0.
 *
 * INPUT
 *		l					: number to calculate the modulo
 *		divisor			: number to divide by
 *
 * ASSUMES
 *		
 *
 *	SEE ALSO
 *		ldivfloor() for a comparison of lmod() with %.
 *
 * RETURN VALUE
 *		<l> mod <divisor>.
 *
 * HISTORY
 *		09/15/93 Wednesday (dcc) - created.
 *
*/
long lmod(long l, long divisor)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "lmod";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (divisor != 0L)
	{
		if (l >= 0L)
		{
			return l % divisor;
		}
		else
		{
			long m = (0L - l) % divisor;

			if (m)
				return divisor - m;
			else
				return 0L;
		}
	}
	return l;

} /* lmod */


/*********************************************************************
 *
 * ldivfloor
 *
 * PURPOSE
 *		Compute <l> mod <divisor>. Special code to handle negative
 *		numbers in a "consistent" fashion.
 *
 *		Also just returns 0 if <divisor> is 0.
 *
 *		Here is a table that compares lmod() with % and ldivfloor() with /:
 *
 *		l		divisor 	lmod			%		ldivfloor	/
 *		-4			4			0			0			-1			-1
 *		-3			4			1			-3			-1			0
 *		-2			4			2			-2			-1			0
 *		-1			4			3			-1			-1			0
 *		0			4			0			0			0			0
 *		1			4			1			1			0			0
 *		2			4			2			2			0			0
 *		3			4			3			3			0			0
 *		4			4			0			0			1			1
 *
 * INPUT
 *		l				: number to be divided
 *		divisor			: number to divide by
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		<l> div <divisor>.
 *
 * HISTORY
 *		09/16/93 Thursday (dcc) - created.
 *
*/
long ldivfloor(long l, long divisor)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ldivfloor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (divisor != 0L)
	{
		if (l >= 0L)
		{
			return l / divisor;
		}
		else
		{
			return (l - (divisor - 1L)) / divisor;
		}
	}
	return 0L;

} /* ldivfloor */


/*********************************************************************
 *
 * ShowGuide
 *
 * PURPOSE
 *		Draw the guide on screen. Call this after drawing all tiles
 *		layers underneath.
 *
 *		This routine should be called only if
 *		IsGuideVisible(roomwindow->CurrentRoom->Room) == TRUE.
 *
 *		This routine should be call immediately following a call
 *		to ShowRoomRectLayer(), with the same parameters.
 *
 * INPUT
 *		showcolumn	: tile column on screen (from dxDst) to start drawing guide (0=left)
 *		showrow		: tile row on screen (from dyDst) to start drawing guide (0=top row)
 *		dtilxSrc		: tile column on map to start drawing
 *		dtilySrc		: tile row on map to start drawing
 *		ctilx		: number of tiles across to outline (or 0 for max across)
 *		ctily		: number of tiles down to outline (or 0 for max down)
 *		dxDst		: x-offset of left edge within screen to draw
 *		dyDst		: y-offset of top edge within screen to draw
 *		cxDst		: width of area on screen to draw (in pixels)
 *		cyDst		: height of area on screen to draw (in pixels)
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		09/14/93 (dcc) - Enhanced to support guides at an angle.
 *		02/27/94 (dcc) - Change call to ClipPushClipValues().
 *		09/09/94 (dcc) - Add support for PixelLayers.
 *		03/26/95 (dcc) - Check for FindNonEmptyLayer() returning NULL.
 *		03/26/95 (dcc) - Add check for 0 lDstDispWidth, lDstDispHeight.
 *
*/
static void ShowGuide (
		ByteMap		*drawBM,
		RoomWindowType	*roomwindow,
		WORD			showcolumn,
		WORD			showrow,
		WORD			dtilxSrc,
		WORD			dtilySrc,
		WORD			ctilx,
		WORD			ctily,
		WORD			dxDst,
		WORD			dyDst,
		short		cxDst,
		short		cyDst)
{
		WORD			maxrow, maxcolumn;
		LayerType		*play;
		LONG			lInitDstXxSrcSkipX;
		LONG			lInitDstYxSrcSkipY;
		LONG			lDstDispWidth;
		LONG			lDstDispHeight;
		LONG			lWindowWidth, lWindowHeight;
		short		i;

		RoomType		*room;
		RoomStuffType	*roomstuff;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowGuide";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (showcolumn < 0)
	{
		dtilxSrc -= showcolumn;
		if (ctilx)
		{
			ctilx += showcolumn;
			if (ctilx < 0)
/**/			goto ABORT;
		}
		showcolumn = 0;
	}
	if (showrow < 0)
	{
		dtilySrc -= showrow;
		if (ctily)
		{
			ctily += showrow;
			if (ctily < 0)
/**/			goto ABORT;
		}
		showrow = 0;
	}

	lWindowWidth	= (LONG) cxDst * wSrcSkipX;
	lWindowHeight	= (LONG) cyDst * wSrcSkipY;

	roomstuff		= roomwindow->CurrentRoom;
	room			= roomstuff->Room;
#if PixelLayers
	if ((play		= FindBiggestLayer(&room->Layers)) == NULL)
		play		= room->FloorLayer;
#else // !PixelLayers
	if ((play		= FindNonEmptyLayer(&room->Layers, room->FloorLayer)) == NULL)
		play		= room->FloorLayer;
#endif // !PixelLayers

//	if (IsGuideVisible(roomwindow->CurrentRoom->Room))
{
	int			tilewidth;	/* tile pixel width */
	int			tileheight;	/* tile pixel height */
	WORD			sep;

	WORD wDispWidth, wDispHeight;

#if __MSDOS__
	BeforeGraphics();
#endif/*__MSDOS__*/

	if ((tilewidth = play->cxTile) == 0)
	{
#if !TEST_PRINT
		RoomType *prm;
		LayerType *play;

		if (FSelectedPblk(GlobalBlockCopy))
		{
#if PixelLayers
			play = FindBiggestLayer(&GlobalBlockCopy->Layers);
#else // !PixelLayers
			play = FindNonEmptyLayer(&GlobalBlockCopy->Layers, GlobalBlockCopy->BC_FloorLayer);
#endif // !PixelLayers
		}
		else
		{
			prm = roomwindow->SourceRoom.Room;
#if PixelLayers
			play = FindBiggestLayer(&prm->Layers);
#else // !PixelLayers
			play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
#endif // !PixelLayers
		}
		if (play)
			tilewidth = play->cxTile;
#endif
	}
	if ((tileheight = play->cyTile) == 0)
	{
#if !TEST_PRINT
		RoomType *prm;
		LayerType *play;

		if (FSelectedPblk(GlobalBlockCopy))
		{
#if PixelLayers
			play = FindBiggestLayer(&GlobalBlockCopy->Layers);
#else // !PixelLayers
			play = FindNonEmptyLayer(&GlobalBlockCopy->Layers, GlobalBlockCopy->BC_FloorLayer);
#endif // !PixelLayers
		}
		else
		{
			prm = roomwindow->SourceRoom.Room;
#if PixelLayers
			play = FindBiggestLayer(&prm->Layers);
#else // !PixelLayers
			play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
#endif // !PixelLayers
		}
		if (play)
			tileheight = play->cyTile;
#endif
	}

	sep			= 0 + ((roomstuff->Flags) & (DISPLAY_SEPERATED));

	wDispWidth  = wSrcSizeToDstSize(tilewidth,  wSrcSkipX, wDstDupX) + sep;
	wDispHeight = wSrcSizeToDstSize(tileheight, wSrcSkipY, wDstDupY) + sep;
	lDstDispWidth	= (LONG) (wSrcSizeToDstSize(tilewidth,  wSrcSkipX, wDstDupX) + sep) * wSrcSkipX;
	lDstDispHeight	= (LONG) (wSrcSizeToDstSize(tileheight, wSrcSkipY, wDstDupY) + sep) * wSrcSkipY;

	if (lDstDispWidth == 0 || lDstDispHeight == 0)
/**/	goto ABORT;

	if (ctilx)
	{
		maxcolumn	= min (play->ctilx, ctilx + dtilxSrc);
	}
	else
	{
		maxcolumn	= play->ctilx;
	}

	if (ctily)
	{
		maxrow	= min (play->ctily, ctily + dtilySrc);
	}
	else
	{
		maxrow	= play->ctily;
	}

	lInitDstXxSrcSkipX = lDstDispWidth *showcolumn+(LONG)(dxDst)*wSrcSkipX;
	lInitDstYxSrcSkipY = lDstDispHeight*showrow   +(LONG)(dyDst)*wSrcSkipY;

	maxcolumn	= min (maxcolumn,(WORD)((lWindowWidth /lDstDispWidth + 1)+dtilxSrc));
	maxrow	= min (maxrow,	  (WORD)((lWindowHeight/lDstDispHeight+ 1)+dtilySrc));

	if (wShowXTiles > 0)
		maxcolumn = min(maxcolumn, wShowXTiles + dtilxSrc);

	if (wShowYTiles > 0)
		maxrow = min(maxrow, wShowYTiles + dtilySrc);

	/* Process all guide lines */

	for (i = room->cGuideLines-1; i >= 0; i--)
	{
		if (room->fShowGuide[i] &&
			(room->pcenGuideA[i] != 0 || room->pcenGuideB[i] != 0))
		{
			long cenC;
			long cenCOrigin;
			long cenCMod;
			long cenCMin = LONG_MAX;
			long cenCMax = LONG_MIN;

			/* Plug XOrigin & YOrigin into equation to obtain COrigin.
				Equation is Ax + By = C */

			cenCOrigin =  ((room->pcenGuideA[i] * room->pcenGuideXOrigin[i]) +
						(room->pcenGuideB[i] * room->pcenGuideYOrigin[i])) / 100L;

			/* Divide cenCOrigin by pcenGuideDC to obtain modulo */

			cenCMod = lmod(cenCOrigin, room->pcenGuideDC[i]);

			/* Now check the four corners of the update area to find
				smallest cenCMin and largest cenCMax */

			/* check upper-left corner */

			cenC =	(room->pcenGuideA[i] * dtilxSrc) +
					(room->pcenGuideB[i] * dtilySrc);

			cenCMin = min(cenCMin, cenC);
			cenCMax = max(cenCMax, cenC);

			/* check upper-right corner */

			cenC =	(room->pcenGuideA[i] * maxcolumn) +
					(room->pcenGuideB[i] * dtilySrc);

			cenCMin = min(cenCMin, cenC);
			cenCMax = max(cenCMax, cenC);

			/* check lower-left corner */

			cenC =	(room->pcenGuideA[i] * dtilxSrc) +
					(room->pcenGuideB[i] * maxrow);

			cenCMin = min(cenCMin, cenC);
			cenCMax = max(cenCMax, cenC);

			/* check lower-right corner */

			cenC =	(room->pcenGuideA[i] * maxcolumn) +
					(room->pcenGuideB[i] * maxrow);

			cenCMin = min(cenCMin, cenC);
			cenCMax = max(cenCMax, cenC);

			/* fix cenCMin and cenCMax so their modulo represents a valid line */

			cenCMin = (ldivfloor(cenCMin, room->pcenGuideDC[i]) - 1L) *
					room->pcenGuideDC[i] + cenCMod;

			cenCMax = (ldivfloor(cenCMax, room->pcenGuideDC[i]) + 1L) *
					room->pcenGuideDC[i] + cenCMod;

			{
				short dxL, dyT;
				short cxWin, cyWin;

				/* calculate pos on-screen of left edge and top row */

				Assert(wSrcSkipX != 0);
				dxL = (short) (lInitDstXxSrcSkipX / wSrcSkipX);
				Assert(wSrcSkipY != 0);
				dyT = (short) (lInitDstYxSrcSkipY / wSrcSkipY);

				cxWin = (maxcolumn-dtilxSrc) * wDispWidth;
				cyWin = (maxrow-dtilySrc) * wDispHeight;

				ClipPushClipValues(dxL, dyT, min(cxWin, cxDst), min(cyWin, cyDst));

				/* Are guide lines predominately vertical or horizontal? */

				if (labs(room->pcenGuideA[i]) > labs(room->pcenGuideB[i]))
				{
					short dyB;		/* predominately vertical lines */

					/* calculate Y position on-screen of bottom rows */

					dyB = dyT + cyWin;

					for (cenC = cenCMin; cenC <= cenCMax; cenC += room->pcenGuideDC[i])
					{
						short dxT, dxB;

						if (room->pcenGuideDC[i] == 0L)
							cenC = cenCOrigin;

						/* solve for x : Ax + By = C	==>
						Ax = C - By		==>		x = (C - By) / A */

						dxT = (short) (((cenC - room->pcenGuideB[i] * dtilySrc) * 100L /
							room->pcenGuideA[i] - (dtilxSrc-showcolumn) * 100L)
							* wDispWidth / 100L + dxDst);

						dxB = (short) (((cenC - room->pcenGuideB[i] * maxrow) * 100L /
							room->pcenGuideA[i] - (dtilxSrc-showcolumn) * 100L)
							* wDispWidth / 100L + dxDst);

						XORDrawLine(drawBM, dxT, dyT, dxB, dyB, GUIDE_EOR);

						if (room->pcenGuideDC[i] == 0L)
							break;
					}
				}
				else
				{
					short dxR;		/* predominately horizontal lines */

					/* calculate X position on-screen of right rows */

					dxR = dxL + cxWin;

					for (cenC = cenCMin; cenC <= cenCMax; cenC += room->pcenGuideDC[i])
					{
						short dyL, dyR;

						if (room->pcenGuideDC[i] == 0L)
							cenC = cenCOrigin;

						/* solve for y : Ax + By = C	==>
						By = C - Ax		==>		y = (C - Ax) / B */

						dyL = (short) (((cenC - room->pcenGuideA[i] * dtilxSrc) * 100L /
							room->pcenGuideB[i] - (dtilySrc - showrow) * 100L)
							* wDispHeight / 100L + dyDst);

						dyR = (short) (((cenC - room->pcenGuideA[i] * maxcolumn) * 100L /
							room->pcenGuideB[i] - (dtilySrc - showrow) * 100L)
							* wDispHeight / 100L + dyDst);

						XORDrawLine(drawBM, dxL, dyL, dxR, dyR, GUIDE_EOR);

						if (room->pcenGuideDC[i] == 0L)
							break;
					}
				}
				PopClipValues();
			}
		}
	}
#if __MSDOS__
	AfterGraphics();
#endif/*__MSDOS__*/
}
ABORT:
	return;
} /* ShowGuide */
#else	/* !NEWTUME */
/*********************************************************************
 *
 * ShowGuide
 *
 * PURPOSE
 *		Draw the guide on screen. Call this after drawing all tiles
 *		layers underneath.
 *
 *		This routine should be called only if
 *		IsGuideVisible(roomwindow->CurrentRoom->Room) == TRUE.
 *
 *		This routine should be call immediately following a call
 *		to ShowRoomRectLayer(), with the same parameters.
 *
 * INPUT
 *		showcolumn	: tile column (from dxDst) to start drawing guide (0=left)
 *		showrow		: tile row (from dyDst) to start drawing guide (0=top row)
 *		ctilx		: number of tiles across to outline (or 0 for max across)
 *		ctily		: number of tiles down to outline (or 0 for max down)
 *		dxDst		: x-offset of left edge within screen to draw
 *		dyDst		: y-offset of top edge within screen to draw
 *		cxDst		: width of area on screen to draw (in pixels)
 *		cyDst		: height of area on screen to draw (in pixels)
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		03/26/95 (dcc) - Check for FindNonEmptyLayer() returning NULL.
 *		03/26/95 (dcc) - Add check for 0 lDstDispWidth, lDstDispHeight.
 *
*/
static void ShowGuide (
		ByteMap		*drawBM,
		RoomWindowType	*roomwindow,
		WORD			showcolumn,
		WORD			showrow,
		WORD			dtilxSrc,
		WORD			dtilySrc,
		WORD			ctilx,
		WORD			ctily,
		WORD			dxDst,
		WORD			dyDst,
		short		cxDst,
		short		cyDst)
{

		WORD			maxrow, maxcolumn;
		LayerType		*play;
		LONG			lInitDstXxSrcSkipX;
		LONG			lInitDstYxSrcSkipY;
		LONG			lDstDispWidth;
		LONG			lDstDispHeight;
		LONG			lWindowWidth, lWindowHeight;

		RoomType		*room;
		RoomStuffType	*roomstuff;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowGuide";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (showcolumn < 0)
	{
		dtilxSrc -= showcolumn;
		if (ctilx)
		{
			ctilx += showcolumn;
			if (ctilx < 0)
/**/			goto ABORT;
		}
		showcolumn = 0;
	}
	if (showrow < 0)
	{
		dtilySrc -= showrow;
		if (ctily)
		{
			ctily += showrow;
			if (ctily < 0)
/**/			goto ABORT;
		}
		showrow = 0;
	}

	lWindowWidth	= (LONG) cxDst * wSrcSkipX;
	lWindowHeight	= (LONG) cyDst * wSrcSkipY;

	roomstuff		= roomwindow->CurrentRoom;
	room			= roomstuff->Room;
#if PixelLayers
	if ((play		= FindBiggestLayer(&room->Layers)) == NULL)
		play		= room->FloorLayer;
#else // !PixelLayers
	if ((play		= FindNonEmptyLayer(&room->Layers, room->FloorLayer)) == NULL)
		play		= room->FloorLayer;
#endif // !PixelLayers

//	if (IsGuideVisible(roomwindow->CurrentRoom->Room))
	{
		WORD wModX, wModY;
		WORD wX, wY;

		int			tilewidth;	/* tile pixel width */
		int			tileheight;	/* tile pixel height */
		WORD			sep;

		WORD wDispWidth, wDispHeight;

#if __MSDOS__
		BeforeGraphics();
#endif/*__MSDOS__*/

		wModX = room->wGuideXOrigin % room->wGuideXWidth;
		wModY = room->wGuideYOrigin % room->wGuideYHeight;

		if ((tilewidth = play->cxTile) == 0)
		{
#if !TEST_PRINT
			RoomType *prm;
			LayerType *play;

			if (FSelectedPblk(GlobalBlockCopy))
			{
#if PixelLayers
				play = FindBiggestLayer(&GlobalBlockCopy->Layers);
#else // !PixelLayers
				play = FindNonEmptyLayer(&GlobalBlockCopy->Layers, GlobalBlockCopy->BC_FloorLayer);
#endif // !PixelLayers
			}
			else
			{
				prm = roomwindow->SourceRoom.Room;
#if PixelLayers
				play = FindBiggestLayer(&prm->Layers);
#else // !PixelLayers
				play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
#endif // !PixelLayers
			}
			if (play)
				tilewidth = play->cxTile;
#endif
		}
		if ((tileheight = play->cyTile) == 0)
		{
#if !TEST_PRINT
			RoomType *prm;
			LayerType *play;

			if (FSelectedPblk(GlobalBlockCopy))
			{
#if PixelLayers
				play = FindBiggestLayer(&GlobalBlockCopy->Layers);
#else // !PixelLayers
				play = FindNonEmptyLayer(&GlobalBlockCopy->Layers, GlobalBlockCopy->BC_FloorLayer);
#endif // !PixelLayers
			}
			else
			{
				prm = roomwindow->SourceRoom.Room;
#if PixelLayers
				play = FindBiggestLayer(&prm->Layers);
#else // !PixelLayers
				play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
#endif // !PixelLayers
			}
			if (play)
				tileheight = play->cyTile;
#endif
		}

		sep			= 0 + ((roomstuff->Flags) & (DISPLAY_SEPERATED));

		wDispWidth  = wSrcSizeToDstSize(tilewidth,  wSrcSkipX, wDstDupX) + sep;
		wDispHeight = wSrcSizeToDstSize(tileheight, wSrcSkipY, wDstDupY) + sep;
		lDstDispWidth	= (LONG) (wSrcSizeToDstSize(tilewidth,  wSrcSkipX, wDstDupX) + sep) * wSrcSkipX;
		lDstDispHeight	= (LONG) (wSrcSizeToDstSize(tileheight, wSrcSkipY, wDstDupY) + sep) * wSrcSkipY;

		if (lDstDispWidth == 0 || lDstDispHeight == 0)
/**/		goto ABORT;

		if (ctilx)
		{
			maxcolumn	= min (play->ctilx, ctilx + dtilxSrc);
		}
		else
		{
			maxcolumn	= play->ctilx;
		}

		if (ctily)
		{
			maxrow	= min (play->ctily, ctily + dtilySrc);
		}
		else
		{
			maxrow	= play->ctily;
		}

		lInitDstXxSrcSkipX =lDstDispWidth *showcolumn+(LONG)(dxDst)*wSrcSkipX;
		lInitDstYxSrcSkipY =lDstDispHeight*showrow   +(LONG)(dyDst)*wSrcSkipY;

		maxcolumn	= min (maxcolumn, (lWindowWidth /lDstDispWidth + 1)+dtilxSrc);
		maxrow	= min (maxrow,	   (lWindowHeight/lDstDispHeight+ 1)+dtilySrc);

		if (wShowXTiles > 0)
			maxcolumn = min(maxcolumn, wShowXTiles + dtilxSrc);

		if (wShowYTiles > 0)
			maxrow = min(maxrow, wShowYTiles + dtilySrc);

		{
			short dx, dy;

			Assert(wSrcSkipX != 0);
			dx = lInitDstXxSrcSkipX / wSrcSkipX;
			Assert(wSrcSkipY != 0);
			dy = lInitDstYxSrcSkipY / wSrcSkipY;

			wX = wModX - dtilxSrc % room->wGuideXWidth;
			if (wX < 0)
				wX += room->wGuideXWidth;

			for ( ; wX < maxcolumn-dtilxSrc; wX += room->wGuideXWidth)
			{
				XORvLine(drawBM, wX * wDispWidth + dx,
					dy,
					(maxrow-dtilySrc) * wDispHeight+dy-1, GUIDE_EOR);
			}
			wY = wModY - dtilySrc % room->wGuideYHeight;
			if (wY < 0)
				wY += room->wGuideYHeight;

			for ( ; wY < maxrow-dtilySrc; wY += room->wGuideYHeight)
			{
				XORhLine(drawBM,
					dx,
					(maxcolumn-dtilxSrc) * wDispWidth+dx-1,
					wY * wDispHeight+dy, GUIDE_EOR);
			}
		}
#if __MSDOS__
		AfterGraphics();
#endif/*__MSDOS__*/
	}
ABORT:
	return;
} /* ShowGuide */
#endif	/* NEWTUME */


#if 1	// BUGBUG PixelLayers
/*********************************************************************
 *
 * ShowLayersInSubRect
 *
 * PURPOSE
 *		Shows layers of <pLayers> with the guide in the ByteMap <pbm>,
 *		offset <dx>,<dy> pixels, in a rectangle <cx>,<cy> pixels in size.
 *		If <ctilx> > 0 then draw no more than <ctilx> wide,
 *		if <ctily> > 0 then draw no more than <ctily> high.
 *
 *		The burden of pixel level clipping is place upon the caller
 *		(it should call SetClipValues() or PushClipValues()).
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		<dtilx>, <dtily>, <ctilx>, <ctily> refer to the largest tile layer.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		05/30/93 (dcc) - added fCompositeSource
 *		08/25/94 (dcc) - added code to deal with different size layers
 *		09/05/94 (dcc) - draw extra on pixel object layers so object tiles
 *						don't get cut when stamping or scrolling
 *
*/
void ShowLayersInSubRect(ListType *pLayers, LayerType *playFloor,
					RoomWindowType *roomwindow, ByteMap *pbm,
					short dx, short dy, short cx, short cy,
					short dtilx, short dtily, short ctilx, short ctily,
					BOOL fShowGuide, BOOL fCompositeSource)
{
		BOOL			 bottom;
		LayerType		*play;
		LayerType		*playBig = NULL;
		int			 sep;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowLayersInSubRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Find layer with the largest tiles */

	playBig = FindBiggestLayer(pLayers);
	if (!playBig)
		return;

	/* plot the layers */

	sep = (roomwindow->CurrentRoom->Flags & DISPLAY_SEPERATED) ? 1 : 0;

	/* Draw the layers */

	bottom = TRUE;

	play = Head(pLayers);

	while (!IsEOList(play))
	{
		if (!(play->L_Flags & LAYER_INVISIBLE))
		{
			if (!EditOnlyFloorSTATE ||
				(EditOnlyFloorSTATE && play == playFloor))
			{
				if (play->cxTile == 1 && play->cyTile == 1)
				{
					UWORD cxTile = playBig->cxTile;
					UWORD cyTile = playBig->cyTile;
#if 1
					int dtilxAdj, dtilyAdj, ctilxAdj, ctilyAdj;
					int dtilxObj, dtilyObj, ctilxObj, ctilyObj;
					int dxObj, dyObj, cxObj, cyObj;
					int cpixxTile, cpixyTile;

					cpixxTile = wSrcSizeToDstSize(cxTile, wSrcSkipX, wDstDupX)+sep;
					cpixyTile = wSrcSizeToDstSize(cyTile, wSrcSkipY, wDstDupY)+sep;

					Assert(cpixxTile != 0);
					dtilxAdj = ((cpixxObjectTile/2)+cpixxTile-1) / cpixxTile;
					Assert(cpixyTile != 0);
					dtilyAdj = ((cpixyObjectTile/2)+cpixyTile-1) / cpixyTile;
					Assert(cpixxTile+1 != 0);
					ctilxAdj = ( cpixxObjectTile   +cpixxTile-1) / cpixxTile+1;
					Assert(cpixyTile+1 != 0);
					ctilyAdj = ( cpixyObjectTile   +cpixyTile-1) / cpixyTile+1;

					dtilxAdj = min(dtilx, dtilxAdj);
					dtilyAdj = min(dtily, dtilyAdj);

					dxObj = dx - dtilxAdj * cpixxTile;
					dyObj = dy - dtilyAdj * cpixyTile;
					cxObj = cx + dtilxAdj * cpixxTile;
					cyObj = cy + dtilyAdj * cpixyTile;

					dtilxObj = dtilx - dtilxAdj;
					dtilyObj = dtily - dtilyAdj;
					ctilxObj = ctilx;
					if (ctilxObj)
						ctilxObj += ctilxAdj;

					ctilyObj = ctily;
					if (ctilyObj)
						ctilyObj += ctilyAdj;
#endif

					ShowRoomRectObjectLayer (pbm, play,
								0, 0,
								dtilxObj * cxTile, dtilyObj * cyTile,
								ctilxObj * cyTile, ctilyObj * cyTile,
								dxObj, dyObj, cxObj, cyObj,
								cxTile, cyTile, sep,
								bottom | EditOnlyFloorSTATE);
				}
				else
				{
					ShowRoomRectLayer (pbm, play, roomwindow,
								0, 0,
								dtilx, dtily, ctilx, ctily,
								dx, dy, cx, cy,
								fCompositeSource, bottom | EditOnlyFloorSTATE);
				}
			}
		}
		play = Next(play);
		bottom = FALSE;
	}

	/* Show the guides, if any. */

	if (fShowGuide)
	{
		ShowGuide(pbm, roomwindow,
				0, 0,
				dtilx, dtily, ctilx, ctily,
				dx, dy, cx, cy);
	}
	return;

} /* ShowLayersInSubRect */
#else // !1 BUGBUG // !PixelLayers
/*********************************************************************
 *
 * ShowLayersInSubRect
 *
 * PURPOSE
 *		Shows layers of <pLayers> with the guide in the ByteMap <pbm>,
 *		offset <dx>,<dy> pixels, in a rectangle <cx>,<cy> pixels in size.
 *		If <ctilx> > 0 then draw no more than <ctilx> wide,
 *		if <ctily> > 0 then draw no more than <ctily> high.
 *
 *		The burden of pixel level clipping is place upon the caller
 *		(it should call SetClipValues() or PushClipValues()).
 *
 * INPUT
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		05/30/93 Sunday (dcc) - added fCompositeSource
 *
*/
void ShowLayersInSubRect(ListType *pLayers, LayerType *playFloor,
					RoomWindowType *roomwindow, ByteMap *pbm,
					short dx, short dy, short cx, short cy,
					short dtilx, short dtily, short ctilx, short ctily,
					BOOL fShowGuide, BOOL fCompositeSource)
{
		BOOL			 bottom;
		LayerType		*play;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowLayersInSubRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* plot the layers */

#if TEST_PRINT
	bottom = TRUE;
	play = Head(pLayers);

	ShowRoomRectLayer (pbm, play, roomwindow,
					0, 0,
					dtilx, dtily, ctilx, ctily,
					dx, dy, cx, cy,
					fCompositeSource, bottom);
#else
	bottom = TRUE;
	play = Head(pLayers);

	while (!IsEOList(play))
	{
		if (!(play->L_Flags & LAYER_INVISIBLE))
		{
			if (!EditOnlyFloorSTATE ||
				(EditOnlyFloorSTATE && play == playFloor))
			{
				ShowRoomRectLayer (pbm, play, roomwindow,
								0, 0,
								dtilx, dtily, ctilx, ctily,
								dx, dy, cx, cy,
								fCompositeSource, bottom | EditOnlyFloorSTATE);
			}
		}
		play = Next(play);
		bottom = FALSE;
	}
#endif /* TEST_PRINT */

	/* Show the guides, if any. */

	if (fShowGuide)
	{
		ShowGuide(pbm, roomwindow,
				0, 0,
				dtilx, dtily, ctilx, ctily,
				dx, dy, cx, cy);
	}
	return;

} /* ShowLayersInSubRect */
#endif // !1 BUGBUG // !PixelLayers


#if !TEST_PRINT
/**************************************************************************
 *
 * RedrawSelectArea
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *		fShowIt:			if TRUE draw GlobalBlockCopy (show brush), else redraw area
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		01/28/92 (dcc) - if (block == GlobalBlockCopy && destflag == FALSE)
 *					  then draw only floor layer. And if GlobalRoomWindow->
 *					  CurrentRoom->Room->FloorNumber != 1 && fShowIt == TRUE,
 *					  then draw area in with transparencies, and don't erase
 *					  what's underneath (in fact, redraw what's underneath.)
 *		01/31/93 (dcc) - Modifies above; do not draw transparently
 *					  if EditOnlyFloorSTATE is TRUE.
 *		02/20/94 (dcc) - Add ctilxOverhang, ctilyOverhang.
 *		02/25/94 (dcc) - Add ctilxOverhang, ctilyOverhang for guides.
 *		09/05/94 (dcc) - Update to support pixel tile objects.
 *		06/07/95 (dcc) - KLUDGE for PixelLayers: pixel layers DO count, so
 *					  don't call FindNonEmptyLayer() in that case.
 *
 * SEE ALSO
 *
*/
void RedrawSelectArea (
	BlockCopyType	*block,
	BOOL			destflag,
	BOOL			fShowIt,
	WORD			showcolumn,
	WORD			showrow
)
{
	WORD			left, right, upper, lower;
	WORD			row, column;
	WORD			plot_height, plot_width;
	RoomWindowType	*roomwindow;
	RoomStuffType	*roomstuff;
	RoomType		*room;
#if __AMIGAOS__
	struct Window	*window;
	struct RastPort	*rp;
#endif/*__AMIGAOS__*/
	WORD			sep;
	BOOL			bottom;

	WORD			ctilxRedraw;
	WORD			ctilyRedraw;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RedrawSelectArea";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (FSelectedPblk(block))
	{
		LayerType		*play, *playbc;

		if ((playbc = FindNonEmptyLayer(&block->Layers, block->BC_FloorLayer)) == NULL)
		{
			playbc = block->BC_FloorLayer;
		}

		if (destflag)
		{
			roomwindow	= block->DestRW;
#if __AMIGAOS__
			window		= block->DestRW->Window;
#endif/*__AMIGAOS__*/
			roomstuff		= block->DestStuff;
			play			= FindNonEmptyLayer(&roomstuff->Room->Layers,
										 roomstuff->Room->FloorLayer);
			if (!play)
				play		= roomstuff->Room->FloorLayer;
#if PIXELSELECT
			column		= block->DestX+block->dtilxOffset;
			row			= block->DestY+block->dtilyOffset;
#endif // PIXELSELECT
#if !PIXELSELECT
			column		= block->DestX;
			row			= block->DestY;
#endif // !PIXELSELECT
		}
		else
		{
			roomwindow	= block->SourceRW;
#if __AMIGAOS__
			window		= block->SourceRW->Window;
#endif/*__AMIGAOS__*/
			roomstuff		= block->SourceStuff;
			play			= playbc;
#if PIXELSELECT
			column		= block->SourceX+block->dtilxOffset;
			row			= block->SourceY+block->dtilyOffset;
#endif // PIXELSELECT
#if !PIXELSELECT
			column		= block->SourceX;
			row			= block->SourceY;
#endif // !PIXELSELECT
		}
		ctilxRedraw = playbc->ctilx;
		ctilyRedraw = playbc->ctily;

		if (block == GlobalBlockCopy && destflag == FALSE && fShowIt == TRUE)
		{
			RoomType *prm = roomwindow->CurrentRoom->Room;
#if	  PixelLayers
			LayerType *play = prm->FloorLayer;

//			if (!(play != NULL &&
//				 play->cxTile == 1 && play->cyTile == 1 && IsSparse(play)))
			if (play == NULL)
				play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
#else  //!PixelLayers
			LayerType *play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
#endif //!PixelLayers
			if (!play)
				play = prm->FloorLayer;

			column	= 0;
			row		= 0;
#if PIXELSELECT
			ctilxRedraw = min(ctilxRedraw, play->ctilx - block->DestX - block->dtilxOffset);
			ctilyRedraw = min(ctilyRedraw, play->ctily - block->DestY - block->dtilyOffset);
#endif // PIXELSELECT
#if !PIXELSELECT
			ctilxRedraw = min(ctilxRedraw, play->ctilx - block->DestX);
			ctilyRedraw = min(ctilyRedraw, play->ctily - block->DestY);
#endif // !PIXELSELECT
		}
//		else
//		{
//			row		= max (row, roomstuff->InitY);
//		}

		if (wShowXTiles > 0)
			ctilxRedraw = min(ctilxRedraw, wShowXTiles - showcolumn);
		if (wShowYTiles > 0)
			ctilyRedraw = min(ctilyRedraw, wShowYTiles - showrow);

		if (ctilxRedraw <= 0 || ctilyRedraw <= 0)
/**/		goto ABORT;

		if (!(play->cxTile))
		{
			if ((block->SourceRoom) && (playbc->cxTile))
			{
				play = playbc;
			}
			else
			{
/**/			goto ABORT;
			}
		}

#if __AMIGAOS__
		rp = window->RPort;
#endif/*__AMIGAOS__*/

//		sep	= ((roomstuff->Flags) & DISPLAY_SEPERATED);
		sep	= ((roomwindow->CurrentRoom->Flags) & DISPLAY_SEPERATED);
		plot_width	= wSrcSizeToDstSize(play->cxTile, wSrcSkipX, wDstDupX) + sep;
		plot_height	= wSrcSizeToDstSize(play->cyTile, wSrcSkipY, wDstDupY) + sep;

		/**************************/
		/* find out where to draw */

//		upper	= ( (row - roomstuff->InitY) * (plot_height) ) + TopAdd;
//		left		= ( (column - roomstuff->InitX) * (plot_width) );
		upper	= showrow * plot_height + TopAdd;
		left		= showcolumn * plot_width;
		lower	= (ctilyRedraw+ctilyOverhang) * (plot_height) + upper + (sep ? 0 : -1);
		right	= (ctilxRedraw+ctilxOverhang) * (plot_width) + left + (sep ? 0 : -1);

		/************************************/
		/* draw in the area, layer by layer */

		if (block != GlobalBlockCopy || destflag != FALSE || fShowIt != TRUE)
		{
			/******************/
			/* erase the area */
#if __AMIGAOS__
				SetDrMd (rp, JAM1);
				SetAPen (rp, GlobalBackground);
				RectFill (rp, left, upper, right, lower);
#elif __MSDOS__
				SetPenColor(GlobalBackground);
				DrawRect(left, upper, right-left+1, lower-upper+1);
		}
		else
		{
				CopyDisplayToBack(left, upper, right-left+1, lower-upper+1, BACKTODISPLAY);

				/* Erase old guide */

#if NEWTUME
				//
#else
				if (!(roomwindow->CurrentRoom->Flags & DISPLAY_SEPERATED))
				{
#endif

#if !NEWTUME
					if (IsGuideVisible(roomwindow->CurrentRoom->Room))
					{
#endif
						ShowGuide(DisplayBM, roomwindow,
							showcolumn,
							showrow,
							GlobalBlockCopy->DestX, //roomstuff->InitX+showcolumn,
							GlobalBlockCopy->DestY, //roomstuff->InitY+showrow,
							ctilxRedraw+ctilxOverhang,
							ctilyRedraw+ctilyOverhang,
							0, TopAdd,
							SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd);
#if !NEWTUME
					}
#endif
#if NEWTUME
				//
#else
				}
#endif
#endif/*__AMIGAOS__/__MSDOS__*/
		}
		bottom	= TRUE;
		room		= roomstuff->Room;

		if (block == GlobalBlockCopy && destflag == FALSE && fShowIt == TRUE)
		{
			LayerType *play = Head (&(GlobalBlockCopy->Layers));

			if (GlobalRoomWindow->CurrentRoom->Room->FloorNumber != 1)
				bottom = FALSE;

			if (fStampMode == fSetStampReplace)
				fDrawNULLTiles = TRUE;

		/*	Show the brush being dragged around. Since we are far too
			lazy to figure out how to display the brush properly while
			it is being dragged around, we only draw one layer of the
			brush (the bottommost layer). */

			if (! IsEOList (play))
			{
#if 1	// BUGBUG PixelLayers
				if (play->cxTile == 1 && play->cyTile == 1)
				{
					LayerType *playBig = FindBiggestLayer(&room->Layers);
					int sep = ((roomwindow->CurrentRoom->Flags) & (DISPLAY_SEPERATED)) ? 1 : 0;

					if (playBig)
					{
						ShowRoomRectObjectLayer (
							DisplayBM,
							play,
							showcolumn,
							showrow,
							column,
							row,
							ctilxRedraw,
							ctilyRedraw,
							0, TopAdd,
							SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
							playBig->cxTile, playBig->cyTile, sep,
							bottom | EditOnlyFloorSTATE);			/* transparency flag */
					}
				}
				else
				{
					ShowRoomRectLayer (
						DisplayBM,
						play,
						roomwindow,
						showcolumn,
						showrow,
						column,
						row,
						ctilxRedraw,
						ctilyRedraw,
						0, TopAdd,
						SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
						FALSE,
						bottom | EditOnlyFloorSTATE);			/* transparency flag */
				}
#else // !1 BUGBUG // !PixelLayers
				ShowRoomRectLayer (
					DisplayBM,
					play,
					roomwindow,
					showcolumn,
					showrow,
					column,
					row,
					ctilxRedraw,
					ctilyRedraw,
					0, TopAdd,
					SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
					FALSE,
					bottom | EditOnlyFloorSTATE);			/* transparency flag */
#endif // !1 BUGBUG // !PixelLayers
			}
			fDrawNULLTiles = FALSE;

			/* Show the guide. */

#if NEWTUME
				//
#else
			if (!(roomwindow->CurrentRoom->Flags & DISPLAY_SEPERATED))
			{
#endif
#if !NEWTUME
				if (IsGuideVisible(roomwindow->CurrentRoom->Room))
				{
#endif
					ShowGuide(DisplayBM, roomwindow,
						showcolumn,
						showrow,
						GlobalBlockCopy->DestX, //roomstuff->InitX+showcolumn,
						GlobalBlockCopy->DestY, //roomstuff->InitY+showrow,
						ctilxRedraw+ctilxOverhang,
						ctilyRedraw+ctilyOverhang,
						0, TopAdd,
						SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd);
#if !NEWTUME
				}
#endif
#if NEWTUME
				//
#else
			}
#endif
		}
		else
		{
			LayerType *play = Head (&(room->Layers));

			while (! IsEOList (play))
			{
				if (! (play->L_Flags & LAYER_INVISIBLE))
				{
					if (!EditOnlyFloorSTATE ||
						(EditOnlyFloorSTATE && play == roomwindow->CurrentRoom->Room->FloorLayer))
					{
#if 1	// BUGBUG PixelLayers
						if (play->cxTile == 1 && play->cyTile == 1)
						{
							LayerType *playBig = FindBiggestLayer(&room->Layers);
							int sep = ((roomwindow->CurrentRoom->Flags) & (DISPLAY_SEPERATED)) ? 1 : 0;

							if (playBig)
							{
								ShowRoomRectObjectLayer (
									DisplayBM,
									play,
									showcolumn,
									showrow,
									column,
									row,
									ctilxRedraw,
									ctilyRedraw,
									0, TopAdd,
									SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
									playBig->cxTile, playBig->cyTile, sep,
									bottom | EditOnlyFloorSTATE);			/* transparency flag */
							}
						}
						else
						{
							ShowRoomRectLayer (
								DisplayBM,
								play,
								roomwindow,
								showcolumn, showrow,
								column, row,
								ctilxRedraw, ctilyRedraw,
								0, TopAdd,
								SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
								FALSE,
								bottom | EditOnlyFloorSTATE);			/* transparency flag */
						}
#else // !1 BUGBUG // !PixelLayers
						ShowRoomRectLayer (
							DisplayBM,
							play,
							roomwindow,
							showcolumn,
							showrow,
							column,
							row,
							ctilxRedraw,
							ctilyRedraw,
							0, TopAdd,
							SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
							FALSE,
							bottom | EditOnlyFloorSTATE);	/* transparency flag */
#endif // !1 BUGBUG // !PixelLayers
					}
				}
				else if (bottom)
				{
					/* erase the area goes here? */
				}

				if (IsEOList(Next(play)))	/* Last thing to be drawn? */
				{
#if !NEWTUME
					if (IsGuideVisible(roomwindow->CurrentRoom->Room))
					{
#endif
						/* Show the guide. */

						ShowGuide(DisplayBM, roomwindow,
							showcolumn,
							showrow,
							GlobalBlockCopy->DestX, //roomstuff->InitX+showcolumn,
							GlobalBlockCopy->DestY, //roomstuff->InitY+showrow,
							ctilxRedraw+ctilxOverhang,
							ctilyRedraw+ctilyOverhang,
							0, TopAdd,
							SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd);
#if !NEWTUME
					}
#endif
				}

				bottom	= FALSE;
				play		= Next (play);
			}
		}
#if 0
		ShowCountsInSubRect(room, roomwindow,
						0, TopAdd,
						SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
						showcolumn, showrow,
						GlobalBlockCopy->DestX,
						GlobalBlockCopy->DestY,
						ctilxRedraw,
						ctilyRedraw);
#endif
	}

	return;
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:
	return;

} /* RedrawSelectArea */


/*********************************************************************
 *
 * RevampTileOverhang
 *
 * PURPOSE
 *		Re-compute tile overhang for all tilesets. Tile overhang
 *		is how much the tile extends beyond the right or bottom
 *		edge of the tile grid.
 *
 * INPUT
 *		ptsp			: tilespace to delete data from
 *
 * ASSUMES
 *		User has added or deleted a tileset.
 *		ptsp is not NULL.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		12/18/93 Saturday (dcc) - created.
 *
*/
void RevampTileOverhang(TileSpaceType *ptsp)
{
	TileSetType *ptst;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RevampTileOverhang";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ctilxOverhang = 0;
	ctilyOverhang = 0;

	ptst = Head(&ptsp->TileSets);
	if (ptst)
	{
		while (!IsEOList(ptst))			/* check all tilesets */
		{
			Assert(ptst->cxDisplay != 0);
			ctilxOverhang = max(ctilxOverhang,
							(ptst->Width  - 1) / ptst->cxDisplay);
			Assert(ptst->cyDisplay != 0);
			ctilyOverhang = max(ctilyOverhang,
							(ptst->Height - 1) / ptst->cyDisplay);
			ptst = Next(ptst);
		}
	}
} /* RevampTileOverhang */


/*********************************************************************
 *
 * HideBrush
 *
 * SYNOPSIS
 *		void HideBrush(void)
 *
 * PURPOSE
 *		Hide the brush being carried around by user so we can draw
 *		underneath.
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
 *		12/17/93 Friday (dcc) - use ->ctilxDisplay vs Head()->ctilx
 *		12/18/93 Saturday (dcc) - added ctilxOverhang
 *
 * SEE ALSO
 *
*/
void HideBrush(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "HideBrush";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (FSelectedPblk(GlobalBlockCopy))
	{
		if (TrackingRoomWindow)
		{
			int dtilx, dtily;

			Assert(GlobalBlockCopy->BC_FloorLayer->cxTile != 0);
			dtilx = (int) (GlobalBlockCopy->DestRW->CurrentRoom->dxSrc / GlobalBlockCopy->BC_FloorLayer->cxTile);
			Assert(GlobalBlockCopy->BC_FloorLayer->cyTile != 0);
			dtily = (int) (GlobalBlockCopy->DestRW->CurrentRoom->dySrc / GlobalBlockCopy->BC_FloorLayer->cyTile);

			GlobalBlockCopy->DestRW		= TrackingRoomWindow;
			GlobalBlockCopy->DestStuff	= TrackingRoomWindow->CurrentRoom;

#if __AMIGAOS__
			DrawSelect (GlobalBlockCopy, DEST_EOR, TRUE);
#elif __MSDOS__

#if PIXELSELECT
			CopyDisplayTilesToBack(GlobalBlockCopy->DestRW,
						GlobalBlockCopy->DestX + GlobalBlockCopy->dtilxOffset,
						GlobalBlockCopy->DestY + GlobalBlockCopy->dtilyOffset,
						GlobalBlockCopy->ctilxDisplay,
						GlobalBlockCopy->ctilyDisplay,
						BACKTODISPLAY);

			ShowCountsInSubRect(GlobalBlockCopy->DestStuff->Room,
							GlobalBlockCopy->DestRW,
							0, TopAdd,
							SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
							GlobalBlockCopy->DestX + GlobalBlockCopy->dtilxOffset - dtilx - ctilxOverhang,
							GlobalBlockCopy->DestY + GlobalBlockCopy->dtilyOffset - dtily - ctilyOverhang,
							GlobalBlockCopy->DestX + GlobalBlockCopy->dtilxOffset - ctilxOverhang,
							GlobalBlockCopy->DestY + GlobalBlockCopy->dtilyOffset - ctilyOverhang,
							GlobalBlockCopy->ctilxDisplay+ctilxOverhang,
							GlobalBlockCopy->ctilyDisplay+ctilyOverhang);
//							GlobalBlockCopy->BC_FloorLayer->ctilx,
//							GlobalBlockCopy->BC_FloorLayer->ctily);
#endif // PIXELSELECT
#if !PIXELSELECT
			CopyDisplayTilesToBack(GlobalBlockCopy->DestRW,
						GlobalBlockCopy->DestX, GlobalBlockCopy->DestY,
						GlobalBlockCopy->ctilxDisplay,
						GlobalBlockCopy->ctilyDisplay,
						BACKTODISPLAY);

			ShowCountsInSubRect(GlobalBlockCopy->DestStuff->Room,
							GlobalBlockCopy->DestRW,
							0, TopAdd,
							SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
							GlobalBlockCopy->DestX - dtilx - ctilxOverhang,
							GlobalBlockCopy->DestY - dtily - ctilyOverhang,
							GlobalBlockCopy->DestX - ctilxOverhang,
							GlobalBlockCopy->DestY - ctilyOverhang,
							GlobalBlockCopy->ctilxDisplay+ctilxOverhang,
							GlobalBlockCopy->ctilyDisplay+ctilyOverhang);
//							GlobalBlockCopy->BC_FloorLayer->ctilx,
//							GlobalBlockCopy->BC_FloorLayer->ctily);
#endif // !PIXELSELECT
#endif/*__AMIGAOS__/__MSDOS__*/
			TrackingRoomWindow = NULL;
		}
	}

} /* HideBrush */


/*********************************************************************
 *
 * ShowBrush
 *
 * SYNOPSIS
 *		void ShowBrush(void)
 *
 * PURPOSE
 *		Show the brush being carried around by the user.
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
 *		12/17/93 Friday (dcc) - use ->ctilxDisplay vs Head()->ctilx
 *		12/18/93 Saturday (dcc) - added ctilxOverhang
 *
 *
 * SEE ALSO
 *
*/
void ShowBrush(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowBrush";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (FSelectedPblk(GlobalBlockCopy))
	{
#if __AMIGAOS__
		DrawSelect (GlobalBlockCopy, DEST_EOR, TRUE);
#elif __MSDOS__
		if (ShowBrushSTATE)
		{
			int dtilx, dtily;

			int dtilDstX;
			int dtilDstY;

			Assert(GlobalBlockCopy->BC_FloorLayer->cxTile != 0);
			dtilx = (int) (GlobalBlockCopy->DestRW->CurrentRoom->dxSrc / GlobalBlockCopy->BC_FloorLayer->cxTile);
			Assert(GlobalBlockCopy->BC_FloorLayer->cyTile != 0);
			dtily = (int) (GlobalBlockCopy->DestRW->CurrentRoom->dySrc / GlobalBlockCopy->BC_FloorLayer->cyTile);

#if PIXELSELECT
			dtilDstX = GlobalBlockCopy->DestX+GlobalBlockCopy->dtilxOffset - dtilx;
			dtilDstY = GlobalBlockCopy->DestY+GlobalBlockCopy->dtilyOffset - dtily;
#endif // PIXELSELECT
#if !PIXELSELECT
			dtilDstX = GlobalBlockCopy->DestX - dtilx;
			dtilDstY = GlobalBlockCopy->DestY - dtily;
#endif // !PIXELSELECT

			RedrawSelectArea(GlobalBlockCopy, FALSE, TRUE, dtilDstX, dtilDstY);

			ShowCountsInSubRect(GlobalBlockCopy->DestStuff->Room,
							GlobalBlockCopy->DestRW,
							0, TopAdd,
							SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
							dtilDstX-ctilxOverhang,
							dtilDstY-ctilyOverhang,
#if PIXELSELECT
							GlobalBlockCopy->DestX+GlobalBlockCopy->dtilxOffset-ctilxOverhang,
							GlobalBlockCopy->DestY+GlobalBlockCopy->dtilyOffset-ctilyOverhang,
#endif // PIXELSELECT
#if !PIXELSELECT
							GlobalBlockCopy->DestX-ctilxOverhang,
							GlobalBlockCopy->DestY-ctilyOverhang,
#endif // !PIXELSELECT
							GlobalBlockCopy->ctilxDisplay+ctilxOverhang,
							GlobalBlockCopy->ctilyDisplay+ctilyOverhang);
//							GlobalBlockCopy->BC_FloorLayer->ctilx+ctilxOverhang,
//							GlobalBlockCopy->BC_FloorLayer->ctily+ctilyOverhang);
		}
		else
		{
			XORDrawPBlk (GlobalBlockCopy, DEST_EOR, TRUE);
		}
#endif/*__MSDOS__*/
		TrackingRoomWindow	= GlobalRoomWindow;
	}
} /* ShowBrush */

#endif /* TEST_PRINT */


/*********************************************************************
 *
 * SetDestinationClip
 *
 * PURPOSE
 *		Set the clip values on the destination bytemap (screen).
 *
 * INPUT
 *		NONE.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		12/20/93 Monday (dcc) - created.
 *		02/20/94 Sunday (dcc) - add check for non-zero play->cxTile, ->cyTile
 *		05/15/94 Sunday (dcc) - modified to take to arguments; get all values
 *						    from GlobalRoomWindow->CurrentRoom instead
 *		07/12/94 Tuesday (dcc) - make sure prm is not NULL before starting
 *		07/19/94 Tuesday (dcc) - don't clip to current position in room,
 *							but clip to whole room instead
 *		08/09/94 Tuesday (dcc) - if separated, go one pixel beyond end of room
 *
*/
void SetDestinationClip(void)
{
	RoomStuffType *prs;		// used to get <InitX> and <InitY> values
	RoomType *prm;			// used to find <play> layer to use
	LayerType *play;		// layer to get <ctilx> and <ctily> values

	WORD wXMax, wYMax;
	WORD sep;
	int ctilxShow, ctilyShow;
	UWORD cxTile, cyTile;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetDestinationClip";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	wXMax = SCREEN_WIDTH;
	wYMax = SCREEN_HEIGHT - TopAdd;

	if (GlobalRoomWindow)
	{
		if ((prs = GlobalRoomWindow->CurrentRoom) != NULL)
		{
			if ((prm = prs->Room) != NULL)
			{
				if ((play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer)) == NULL)
					play = prm->FloorLayer;

				sep	= (prs->Flags & DISPLAY_SEPERATED);

//				ctilxShow = play->ctilx - prs->InitX;
//				ctilyShow = play->ctily - prs->InitY;
				ctilxShow = play->ctilx;
				ctilyShow = play->ctily;

				if (wShowXTiles > 0)
					ctilxShow = min(ctilxShow, wShowXTiles);

				if (wShowYTiles > 0)
					ctilyShow = min(ctilyShow, wShowYTiles);

				cxTile = play->cxTile;
				cyTile = play->cyTile;

				if (FSelectedPblk(GlobalBlockCopy))
				{
					if (!cxTile)
					{
						cxTile = GlobalBlockCopy->BC_FloorLayer->cxTile;
					}
					if (!cyTile)
					{
						cyTile = GlobalBlockCopy->BC_FloorLayer->cyTile;
					}
				}

				if (cxTile)
					wXMax =min(wXMax,
						ctilxShow*(wSrcSizeToDstSize(cxTile, wSrcSkipX, wDstDupX)+sep)+sep
							);
				if (cyTile)
					wYMax =min(wYMax,
						ctilyShow*(wSrcSizeToDstSize(cyTile, wSrcSkipY, wDstDupY)+sep)+sep
							);
			}
		}
	}

	SetClipValues(0, TopAdd, wXMax, wYMax);

} /* SetDestinationClip */


/**************************************************************************
 *
 * ShowRoom
 *
 * PURPOSE
 *		Show the room <roomwindow->CurrentRoom->Room> in the current
 *		display pane.
 *
 * INPUT
 *		roomwindow->CurrentRoom->dxSrc :	x coord of tile to draw in left column of screen
 *		roomwindow->CurrentRoom->dySrc :	y coord of tile to draw in upper row of screen
 *		wShowXTiles				 :	if != 0, only show this many tiles wide
 *		wShowYTiles				 :	if != 0, only show this many tiles high
 *
 * ASSUMES
 *		roomwindow->CurrentRoom->Room != NULL;
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		12/20/93 (dcc) - clip destination to limits of room as well
 *		08/01/94 (dcc) - call SetDestinationClip() even if play == NULL
 *		10/20/94 (dcc) - Move some code to separate UpdateMenuCheckmars().
 *
*/
void ShowRoom (RoomWindowType *roomwindow)
{
		LayerType 	*play;
#if __AMIGAOS__
struct	Window		*window;
struct	RastPort		*rp;
#endif/*__AMIGAOS__*/
		RoomType		*room;
		RoomStuffType	*roomstuff;

#if __AMIGAOS__
		void			*old_pointer;
#elif __MSDOS__
		short		old_pointer;
		BOOL			fAutoMagicMenus;
#endif/*__AMIGAOS__/__MSDOS__*/

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	#if TEST_PRINT
	old_pointer = 0;
	old_pointer = old_pointer;
	#else
	old_pointer = GlobalPointer;
	#endif

	if (! ((roomwindow) && (roomwindow->Window)))
	{
/**/	goto ABORT;
	}

	/********************/
	/* clear the window */

	roomstuff	= roomwindow->CurrentRoom;
	room		= roomstuff->Room;
	if (room)
	{
//		play	= FindNonEmptyLayer(&room->Layers, room->FloorLayer);
		play = FindBiggestLayer(&room->Layers);
		if (!play)
			play = room->FloorLayer;
	}
	else
		play	= NULL;

	SetGlobalZoom(room);

#if __AMIGAOS__
	window	= roomwindow->Window;
	rp		= window->RPort;

	SetRast (rp, (int) GlobalBackground);
#elif __MSDOS__

// Maybe we should call ClearPane here instead.../*dcc*/

	fAutoMagicMenus = AutoHideMenus();
	SetPenColor(GlobalBackground);
	DrawRect(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd);

	SetDestinationClip();

	if (!play)
/**/	goto ABORT;
#endif/*__AMIGAOS__/__MSDOS__*/

	#if !TEST_PRINT
	OldColumn = OldRow = -1;
	#endif

	if (	(!(roomwindow->CurrentRoom)) ||
		(!(roomwindow->CurrentRoom->Room)) ) {

			CopyDisplayToBack(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd, DISPLAYTOBACK);
/**/		goto ABORT;
	}

#if !TEST_PRINT
#if __MSDOS__
	UpdateMenuCheckmarks(roomstuff);
#endif/*__MSDOS__*/

	/* Set the screen color if showing edit room or !UseEditPaletteSTATE */

	if (!UseEditPaletteSTATE ||
	    (UseEditPaletteSTATE && IsEdit(room)))
	{
		if (room->R_ColorInfo) {
			SetGlobalColors(room->R_ColorInfo);	/* Use current room palette. */
		}
		else {
			SetGlobalColors(&DefaultColorInfo);	/* else use default palette. */
		}

#if __AMIGAOS__
		SetColorRangeForReals (MainVp, 0, MAXCOLORREG - 1, (void *) GlobalColors->Colors);
#elif __MSDOS__
		SetDisplayPalette (ActivateXTRA(GlobalColors->pxtColors), GlobalColors->NumColors);
		ReleaseXTRA(GlobalColors->pxtColors);
#endif/*__AMIGAOS__/__MSDOS__*/
	}

/*	Show room if tiles stamped in room (room->TileWidth != 0), or
	if room is empty, then show room if user is holding a brush
	(GlobalBlockCopy->Width != 0). */

	if ((play && play->cxTile) || FSelectedPblk(GlobalBlockCopy))
	{
		int dtilx;
		int dtily;

		if (play->cxTile)
			dtilx = (int) (roomstuff->dxSrc / play->cxTile);
		else
		{
			Assert(GlobalBlockCopy->BC_FloorLayer->cxTile != 0);
			dtilx = (int) (roomstuff->dxSrc / GlobalBlockCopy->BC_FloorLayer->cxTile);
		}

		if (play->cyTile)
			dtily = (int) (roomstuff->dySrc / play->cyTile);
		else
		{
			Assert(GlobalBlockCopy->BC_FloorLayer->cyTile != 0);
			dtily = (int) (roomstuff->dySrc / GlobalBlockCopy->BC_FloorLayer->cyTile);
		}

		/***********************/
		/* show a busy pointer */

#if __AMIGAOS__
		GlobalPointer = RGM_SetPointer (&YieldMPointer, roomwindow->Window);
#elif __MSDOS__
		GlobalPointer = DCC_SetPointer (BPI_WAIT_POINTER);
#endif/*__AMIGAOS__/__MSDOS__*/

#endif /* TEST_PRINT */

		ShowLayersInSubRect(&room->Layers, room->FloorLayer,
						roomwindow, DisplayBM,
						0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
						dtilx, dtily,
						wShowXTiles, wShowYTiles,
#if NEWTUME
						TRUE,
#else
						(IsGuideVisible(room)),
#endif
						(IsComposite(room) && IsLocked(room)));

	#if !TEST_PRINT

		/****************************************/
		/* plot the select marker if we need to */

		if (
			(GlobalBlockCopy->SourceRoom == roomwindow->CurrentRoom->Room) &&
			(GlobalBlockCopy->SourceStuff == roomwindow->CurrentRoom) ) {

			XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);
		}
//#if __MSDOS__
//		CopyDisplayTilesToBack(roomwindow, dtilx, dtily, 0, 0, DISPLAYTOBACK);
//#endif/*__MSDOS__*/

		/*****************************/
		/* restore the mouse pointer */

#if __AMIGAOS__
		GlobalPointer = RGM_SetPointer (old_pointer, MainWindow);
#elif __MSDOS__
		GlobalPointer = DCC_SetPointer (old_pointer);
#endif/*__AMIGAOS__/__MSDOS__*/
#if __MSDOS__
		CopyDisplayToBack(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd, DISPLAYTOBACK);
#endif/*__MSDOS__*/

		ShowCountsInSubRect(room,
						roomwindow,
						0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
						0, 0,
						dtilx, dtily,
						wShowXTiles, wShowYTiles);
	}
		ShowNewSearchOutline();
		DrawTileHighlight();

	/***************************************************/
	/* tell Tracking that we don't have the same image */

	if (TrackingRoomWindow == GlobalRoomWindow)
	{
		TrackingRoomWindow = NULL;
	}

	if (fAutoMagicMenus)
		AutoShowMenus();

	#endif /* TEST_PRINT */

	return;

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

ABORT:
	return;

} /* ShowRoom */


#if !TEST_PRINT
/*********************************************************************
 *
 * ShowSelectBox
 *
 * SYNOPSIS
 *		static void ShowSelectBox(void)
 *
 * PURPOSE
 *		Draw the select box. NB: this gets clipped by clip values (duh!)
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
static void ShowSelectBox(void)
{
	RoomType	*currentroom;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowSelectBox";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	currentroom = GlobalRoomWindow->CurrentRoom->Room;

	if (FSelectedPblk(GlobalBlockCopy) && (currentroom))
	{

		if ((currentroom == GlobalBlockCopy->SourceRoom) && 
			(GlobalRoomWindow->Window
				== GlobalBlockCopy->SourceRW->Window))
		{
			XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);
		}
	}
} /* ShowSelectBox */


/**************************************************************************
 *
 * Scroll
 *
 * PURPOSE
 *		If fUpdateSelect is TRUE, the select rectangle around the
 *		source rectangle will be updated after the scroll.
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
 *		10/02/89 (RGM) - Created.
 *		10/10/89 (RGM) - Resurrected to roomwindow support.
 *		02/24/92 (dcc) - in MS-DOS version, scrolling will
 *						erase the brush being dragged around, and
 *						the new part scrolled in needs to have the
 *						selection rectangle drawn in.
 *		12/20/93 (dcc) - add clip to limits of room as well
 *		02/25/94 (dcc) - add ctilxOverhang, ctilyOverhang (OBSOLETED)
 *		03/02/94 (dcc) - revised to use ShowLayersInSubRect()
 *		05/13/94 (dcc) - add calls to ClipPushClipValues()
 *		07/14/94 (dcc) - fix jumping from say (16, 16) to (42, 76)
 *		07/18/94 (dcc) - optimize by processing Y before X
 *		07/19/94 (dcc) - fix not passing FloorLayer to ShowLayersInSubRect()
 *		08/23/94 (dcc) - call SetPenColor() immediately B4 each DrawRect()
 *		10/19/94 (dcc) - Fix parameters passed to SaveSmartFlipNewXY().
 *
 * SEE ALSO
 *
*/
#if __AMIGAOS__
void Scroll (
	RoomWindowType	*roomwindow,
	WORD			delta_x,
	WORD			delta_y
)
#elif __MSDOS__
void Scroll (
	RoomWindowType	*roomwindow,
	WORD			delta_x,
	WORD			delta_y,
	BOOL			fUpdateSelect
)
#endif/*__AMIGAOS__/__MSDOS__*/
{
	UWORD		cxTileScroll;
	UWORD		cyTileScroll;
	WORD			plot_width, plot_height;
	WORD			sep, extra_w, extra_h;
	int			initx, inity;
	WORD			window_wtile_width, window_wtile_height;
	WORD			window_tile_width, window_tile_height;
	WORD			wXPixMax;
	WORD			wYPixMax;
	int			deltapixx, deltapixy;
	LayerType		*layer;
	struct Window	*window;
	RoomType		*room;
#if __AMIGAOS__
	struct RastPort *rp;
#endif/*__AMIGAOS__*/
	RWInitsType		*rwinit;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Scroll";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	room	= roomwindow->CurrentRoom->Room;
	window	= roomwindow->Window;

	if (! (delta_x || delta_y))
	{
/**/	goto ABORT;
	}
	if (!room)
/**/	goto ABORT;

	if ((layer = FindBiggestLayer(&room->Layers)) == NULL)
//	if ((layer = FindNonEmptyLayer(&room->Layers, room->FloorLayer)) == NULL)
	{
/**/	goto ABORT;
	}
	cxTileScroll = layer->cxTile;
	cyTileScroll = layer->cyTile;

	wXPixMax = window->Width;
	wYPixMax = window->Height;

#if __AMIGAOS__
	rp = window->RPort;
#endif/*__AMIGAOS__*/

#if __MSDOS__
	BeforeGraphics();
#endif/*__MSDOS__*/

	HideSearchOutline();
	HideTileHighlight();

	/*********************************/
	/* update the various data items */

	GlobalRoomWindow->CurrentRoom->dxSrc -= (ULONG) delta_x * cxTileScroll;
	GlobalRoomWindow->CurrentRoom->dySrc -= (ULONG) delta_y * cyTileScroll;
	Assert(cxTileScroll != 0);
	initx	= (int) (GlobalRoomWindow->CurrentRoom->dxSrc / cxTileScroll);
	Assert(cyTileScroll != 0);
	inity	= (int) (GlobalRoomWindow->CurrentRoom->dySrc / cyTileScroll);

	SetDestinationClip();
	SaveSmartFlipNewXY(	GlobalRoomWindow->CurrentRoom->Room,
					GlobalRoomWindow->CurrentRoom->dxSrc,
					GlobalRoomWindow->CurrentRoom->dySrc);

	rwinit = Head (&room->RWInits);

	if (! IsEOList (rwinit)) {
		rwinit->dxSrcPrev = (ULONG) initx * cxTileScroll;
		rwinit->dySrcPrev = (ULONG) inity * cyTileScroll;
	}
	else {
		/* printf ("ERROR - no RWInits for this room!!!"); */
	}

	sep	= ((roomwindow->CurrentRoom->Flags) & DISPLAY_SEPERATED);
	plot_width	= wSrcSizeToDstSize(cxTileScroll, wSrcSkipX, wDstDupX) + sep;
	plot_height	= wSrcSizeToDstSize(cyTileScroll, wSrcSkipY, wDstDupY) + sep;

	Assert(plot_width != 0);
	window_wtile_width	= (window->Width - sep) / plot_width;
	Assert(plot_height != 0);
	window_wtile_height	= (window->Height - (sep + TopAdd)) / plot_height;

	extra_w = ((window->Width - sep) % plot_width) ? 1 : 0;
	extra_h = ((window->Height - (sep + TopAdd)) % plot_width) ? 1 : 0;

	if (wShowXTiles > 0)
	{
		if (wShowXTiles < window_wtile_width)
		{
			window_wtile_width = wShowXTiles;
			wXPixMax = window_wtile_width * plot_width;
			extra_w = 0;
		}
	}
	if (wShowYTiles > 0)
	{
		if (wShowYTiles < window_wtile_height)
		{
			window_wtile_height = wShowYTiles;
			wYPixMax = window_wtile_height * plot_height + TopAdd;
			extra_h = 0;
		}
	}

	window_tile_width	= window_wtile_width + extra_w;
	window_tile_height	= window_wtile_height + extra_h;

	deltapixx = delta_x * plot_width;
	deltapixy = delta_y * plot_height;

	/***************************/
	/* do the actual scrolling */

#if __AMIGAOS__
	SetBPen (rp, GlobalBackground);

	ScrollRaster (
		rp,
		(int) ((-delta_x) * plot_width),
		(int) ((-delta_y) * plot_height),
		(int) sep,
		(int) sep + TopAdd,
		(int) wXPixMax,
		(int) wYPixMax
	);
#elif __MSDOS__
	ClipPushClipValues(0, TopAdd, wXPixMax, wYPixMax - TopAdd);

	CopyBigRectToSmall(BigBackBM, 0, TopAdd,
				DisplayBM,
				deltapixx, TopAdd + deltapixy,
				wXPixMax, wYPixMax);

	{
		WORD wXleft = 0;
		WORD wYtop = TopAdd;
		WORD wXwidth = wXPixMax;
		WORD wYheight = wYPixMax - TopAdd;
		int	dtilxRefresh, dtilyRefresh, ctilxRefresh, ctilyRefresh;
		BOOL fRedrawSelect =	fUpdateSelect && GlobalRoomWindow->CurrentRoom->Room &&
									FSelectedPblk(GlobalBlockCopy) &&
									GlobalRoomWindow->CurrentRoom->Room == GlobalBlockCopy->SourceRoom &&
									GlobalRoomWindow->Window == GlobalBlockCopy->SourceRW->Window;

		dtilxRefresh = initx;
		dtilyRefresh = inity;
		ctilxRefresh = window_tile_width;
		ctilyRefresh = window_tile_height;

		if (delta_y > 0)
		{
			if (window_wtile_height < delta_y)
			{
				delta_y = window_wtile_height;
				deltapixy = delta_y * plot_height;
			}

			ClipPushClipValues(wXleft, wYtop, wXwidth, deltapixy);
			SetPenColor(GlobalBackground);
			DrawRect(wXleft, wYtop, wXwidth, deltapixy);

//			dtilyRefresh = inity;
//			ctilyRefresh = delta_y;

			ShowLayersInSubRect(&room->Layers, room->FloorLayer,
							roomwindow, DisplayBM,
							wXleft, wYtop,
							wXwidth, deltapixy,
							dtilxRefresh, inity,
							ctilxRefresh, delta_y,
#if NEWTUME
							TRUE,
#else
							(IsGuideVisible(room)),
#endif
							(IsComposite(room) && IsLocked(room)));
			if (fRedrawSelect)
				ShowSelectBox();
			PopClipValues();

			wYtop	+= deltapixy;
			wYheight	-= deltapixy;
			dtilyRefresh += delta_y;
			ctilyRefresh -= delta_y;
		}
		else if (delta_y < 0)
		{
			wYheight = window_wtile_height * plot_height + deltapixy;

			ClipPushClipValues(wXleft, wYtop + wYheight,
					wXwidth, wYPixMax - wYtop - wYheight);
			SetPenColor(GlobalBackground);
			DrawRect(wXleft, wYtop + wYheight,
					wXwidth, wYPixMax - wYtop - wYheight);

//			dtilyRefresh = inity + window_wtile_height - abs(delta_y);
//			ctilyRefresh = abs(delta_y) + extra_h;

			ShowLayersInSubRect(&room->Layers, room->FloorLayer,
							roomwindow, DisplayBM,
							wXleft, wYtop + wYheight,
							wXwidth, wYPixMax - wYtop - wYheight,
							dtilxRefresh, inity+window_wtile_height-abs(delta_y),
							ctilxRefresh, abs(delta_y) + extra_h,
#if NEWTUME
							TRUE,
#else
							(IsGuideVisible(room)),
#endif
							(IsComposite(room) && IsLocked(room)));
			if (fRedrawSelect)
				ShowSelectBox();
			PopClipValues();

			ctilyRefresh += delta_y;
		}
		if (delta_x > 0)
		{
			if (window_wtile_width < delta_x)
			{
				delta_x = window_wtile_width;
				deltapixx = delta_x * plot_width;
			}

			ClipPushClipValues(wXleft, wYtop, deltapixx, wYheight);
			SetPenColor(GlobalBackground);
			DrawRect(wXleft, wYtop, deltapixx, wYheight);

			dtilxRefresh = initx;
			ctilxRefresh = delta_x;

			ShowLayersInSubRect(&room->Layers, room->FloorLayer,
							roomwindow, DisplayBM,
							wXleft, wYtop,
							deltapixx, wYheight,
							dtilxRefresh, dtilyRefresh,
							ctilxRefresh, ctilyRefresh,
#if NEWTUME
							TRUE,
#else
							(IsGuideVisible(room)),
#endif
							(IsComposite(room) && IsLocked(room)));
			if (fRedrawSelect)
				ShowSelectBox();
			PopClipValues();
		}
		else if (delta_x < 0)
		{
			wXwidth = window_wtile_width * plot_width + deltapixx;

			ClipPushClipValues(wXleft + wXwidth, wYtop,
					wXPixMax - wXleft - wXwidth, wYheight);
			SetPenColor(GlobalBackground);
			DrawRect(wXleft + wXwidth, wYtop,
					wXPixMax - wXleft - wXwidth, wYheight);

			dtilxRefresh = initx + window_wtile_width - abs(delta_x);
			ctilxRefresh = abs(delta_x) + extra_w;

			ShowLayersInSubRect(&room->Layers, room->FloorLayer,
							roomwindow, DisplayBM,
							wXleft + wXwidth, wYtop,
							wXPixMax - wXleft - wXwidth, wYheight,
							dtilxRefresh, dtilyRefresh,
							ctilxRefresh, ctilyRefresh,
#if NEWTUME
							TRUE,
#else
							(IsGuideVisible(room)),
#endif
							(IsComposite(room) && IsLocked(room)));
			if (fRedrawSelect)
				ShowSelectBox();
			PopClipValues();
		}
	}

#endif/*__AMIGAOS__/__MSDOS__*/

#if __MSDOS__
	CopyDisplayToBack(0, 0, wXPixMax, wYPixMax, DISPLAYTOBACK);
#endif/*__MSDOS__*/

	ShowCountsInSubRect(room,
					roomwindow,
					0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
					0, 0,
					initx, inity,
					wShowXTiles, wShowYTiles);

	ShowSearchOutline();
	DrawTileHighlight();

#if __MSDOS__
	PopClipValues();

	AfterGraphics();
#endif/*__MSDOS__*/
	return;
/*=======================================================================*/
ABORT:
	return;

} /* Scroll */


/*********************************************************************
 *
 * SetStampMode
 *
 * SYNOPSIS
 *		void SetStampMode(short f)
 *
 * PURPOSE
 *		Set brush stamp mode to <f>.
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
void SetStampMode(short f)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetStampMode";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	fStampMode = f;

} /* SetStampMode */


/*********************************************************************
 *
 * BigTileCoordsFromSmallTile
 *
 * PURPOSE
 *		Convert the offset and width/height of a rectangle on a small
 *		tile layer to the corresponding offset & width/height in the
 *		larger tile layer that covers the same area.
 *
 * INPUT
 *		ListType *pLayers	: this is searched to find layer w/ largest tile
 *		LayerType *play	: this is the layer to be converted
 *		int *pdtilx		: input and output x-offset
 *		int *pdtily		: input and output y-offset
 *		int *pctilx		: input and output width
 *		int *pctily		: input and output height
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/30/94 (dcc) - Created.
 *		10/20/94 (dcc) - Changed to use <pLayers> instead of <prm>.
 *		10/20/94 (dcc) - Add support for <*pctilx> == 0 && <*pctily> == 0.
 *
*/
void BigTileCoordsFromSmallTile(ListType *pLayers, LayerType *play,
					int *pdtilx, int *pdtily, int *pctilx, int *pctily)
{
	UWORD cxTile = play->cxTile;
	UWORD cyTile = play->cyTile;
	UWORD cxTileBig, cyTileBig;
	LayerType *playBig = FindBiggestLayer(pLayers);

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "BigTileCoordsFromSmallTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (playBig)
	{
		cxTileBig = playBig->cxTile;
		cyTileBig = playBig->cyTile;
		if (cxTile && cyTile && cxTileBig && cyTileBig)
		{
			if (*pctilx)
				*pctilx = (int)(((long) *pdtilx+ *pctilx) * cxTile/cxTileBig - (long) *pdtilx * cxTile / cxTileBig+1);
			if (*pctily)
				*pctily = (int)(((long) *pdtily+ *pctily) * cyTile/cyTileBig - (long) *pdtily * cyTile / cyTileBig+1);
			*pdtilx = (int)((long) *pdtilx * cxTile / cxTileBig);
			*pdtily = (int)((long) *pdtily * cyTile / cyTileBig);
		}
	}

} /* BigTileCoordsFromSmallTile */


/**************************************************************************
 *
 * PlaceBlockCopy
 *
 * PURPOSE
 *		User is pasting or erasing with the current tile brush.
 *		Update the appropriate layers of the current room, then
 *		show the newly updated room.
 *
 * USAGE
 *
 *
 * INPUT
 *		BOOL image_flag	: == 0 (ERASE), erasing tiles with tile brush
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *		07/12/93 (dcc) - Add support to update tile usage counts.
 *		02/18/94 (dcc) - Fix for pasting off top or left edge.
 *		02/20/94 (dcc) - add support for play->dtilx, ->dtily
 *		03/02/94 (dcc) - change call to ClipPushClipValues()
 *		07/19/94 (dcc) - fix not passing FloorLayer to ShowLayersInSubRect()
 *		08/21/94 (dcc) - add support for UseSparseArray
 *		09/08/94 (dcc) - PixelSelect: scale values for AddTilesInSubRect()
 *		09/29/94 (dcc) - fix stratify pasting into an empty layer not setting
 *								play->cxTile and play->cyTile
 *		10/20/94 (dcc) - Updated to use GetNextSparsePlotInRange().
 *
 * SEE ALSO
 *
*/
void PlaceBlockCopy (
	BlockCopyType	*block,
	BOOL			image_flag
)
{
register	PlotType	*blockplot;
register	PlotType	*destplot;

#if PLOTARRAY
register	PlotType	*startrow;
#endif

	UWORD				column, row;
	WORD				blockwidth, blockheight;
	int				destx, desty;
	WORD				roomwidth, roomheight;
	WORD				collision;
#if __AMIGAOS__
	struct	RastPort	*rp;
#endif/*__AMIGAOS__*/

	LayerType			*blocklayer;
	LayerType			*destlayer;
	LayerType			*floorlayer;

#if PLOTARRAY
	long				row_add;
#endif

	ColorInfo			*temp_cinfo;
/*	BOOL				bottom = FALSE;	*/

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PlaceBlockCopy";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((block) && (block->DestStuff) && (block->DestStuff->Room))
	{
		RoomType		*prm = block->DestStuff->Room;
		LayerType		*play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer);
#if PixelLayers
		int dtilxKey, dtilyKey, ctilxKey, ctilyKey;
#endif // PixelLayers

		if (!play)
		{
			/**************************************/
			/* the destination room has no tiles, */
			/* so init the color info structure   */

			temp_cinfo = prm->R_ColorInfo;
			prm->R_ColorInfo =
					DuplicateColorInfo (block->SourceRoom->R_ColorInfo);

			if (IsGlobalColorsCopyOf(temp_cinfo))
			{
				SetGlobalColors(prm->R_ColorInfo);
#if __AMIGAOS__
				SetColorRangeForReals (MainVp, 0, MAXCOLORREG - 1, (void *) GlobalColors->Colors);
#elif __MSDOS__
				SetDisplayPalette (ActivateXTRA(GlobalColors->pxtColors), GlobalColors->NumColors);
				ReleaseXTRA(GlobalColors->pxtColors);
#endif/*__AMIGAOS__/__MSDOS__*/
			}
			if (temp_cinfo)
			{
				DeleteColorInfo (temp_cinfo);
			}
		}

#if __AMIGAOS__
		rp = block->DestRW->Window->RPort;
#endif/*__AMIGAOS__*/

#if PIXELSELECT
		destx		= block->DestX + block->dtilxOffset;
		desty		= block->DestY + block->dtilyOffset;
#endif // PIXELSELECT
#if !PIXELSELECT
		destx		= block->DestX;
		desty		= block->DestY;
#endif // !PIXELSELECT

		roomheight	= prm->FloorLayer->ctily;
		roomwidth		= prm->FloorLayer->ctilx;

		blockheight	= ((LayerType *) Head(&block->Layers))->ctily;
		blockwidth	= ((LayerType *) Head(&block->Layers))->ctilx;

#if PixelLayers
		dtilxKey = destx;
		dtilyKey = desty;
		ctilxKey = blockwidth;
		ctilyKey = blockheight;

		BigTileCoordsFromSmallTile(&prm->Layers, prm->FloorLayer,
							&dtilxKey, &dtilyKey, &ctilxKey, &ctilyKey);

		UpdateUndo(prm, UndoRoom, dtilxKey, dtilyKey,
					dtilxKey+ctilxKey-1, dtilyKey+ctilyKey-1);
#else // !PixelLayers
		UpdateUndo (
			prm,
			UndoRoom,
			destx,
			desty,
			(WORD) ((destx + blockwidth) - 1),
			(WORD) ((desty + blockheight) - 1)
		);
#endif // !PixelLayers

#if PLOTARRAY
		row_add	= (long) desty * roomwidth;
#endif

		blocklayer= block->BC_FloorLayer;
		destlayer	= floorlayer = prm->FloorLayer;

		if ((! blocklayer) || (! destlayer))
		{
			goto ABORT;
		}

#if 0
		if (prm->FloorNumber == 1)
		{
			bottom = TRUE;
		}
#endif
		collision	= 0;

#if 0
		{
			switch (TU_DrawMode)
			{
			case DMODE_DRAW:
				printf ("DMODE_DRAW.\n");
				break;
			case DMODE_FILL:
				printf ("DMODE_FILL.\n");
				break;
			case DMODE_RANDOM:
				printf ("DMODE_RANDOM.\n");
				break;
			case DMODE_COLOR1:
				printf ("DMODE_COLOR1.\n");
				break;
			case DMODE_COLOR2:
				printf ("DMODE_COLOR2.\n");
				break;
			case DMODE_COLOR3:
				printf ("DMODE_COLOR3.\n");
				break;
			case DMODE_COLOR4:
				printf ("DMODE_COLOR4.\n");
				break;
			}
		}
#endif

		/* See if we are stratify pasting. If so,
		   then set all LAYER_NOT_YET_PASTED bits. */

		if (StratifyPasteSTATE)
		{
			LayerType *play;

			if ((play = Head(&prm->Layers)) != NULL)
			{
				while (!IsEOList(play))
				{
					play->L_Flags |= LAYER_NOT_YET_PASTED;
					play = Next(play);
				}
			}
		}

		/* Subtract out tile usage counts for area to be pasted/erased */

#if PixelLayers
		if (TU_DrawMode < DMODE_COLOR1)
			AddTilesInSubRect(&prm->Layers, dtilxKey, dtilyKey,
							ctilxKey, ctilyKey,
							-1, &prm->prgtct, &prm->fcntValid);
#else // !PixelLayers
		if (TU_DrawMode < DMODE_COLOR1)
			AddTilesInSubRect(&prm->Layers, destx, desty,
							blockwidth, blockheight,
							-1, &prm->prgtct, &prm->fcntValid);
#endif // !PixelLayers

		while ( (! IsEOList (destlayer)) && (! IsEOList (blocklayer)) )
		{
			if (!StratifyPasteSTATE)
			{
				/* If tile grid size not set, then fill it in */

				if (!destlayer->cxTile)
					destlayer->cxTile = blocklayer->cxTile;

				if (!destlayer->cyTile)
					destlayer->cyTile = blocklayer->cyTile;
			}
#if PLOTARRAY
			if (((destlayer->L_Flags) & (LAYER_LOCKED)) && !StratifyPasteSTATE)
			{
				collision ++;
			}
			else if (blocklayer->Plot)
			{
				UWORD col0 = 0;
				UWORD row0 = 0;

				startrow	= destlayer->Plot;
				startrow	+= (row_add + destx);

				blockplot	= blocklayer->Plot;

/* FUDGE-O-MOTO: KLUDGE: FIXME: ETC: I'm sick of trying to write code
	that'll probably never get used, so this stuff needs to be fixed!!!!) */

				if (destx < 0)
				{
					col0 = -destx;
				}
				if (desty < 0)
				{
					row0 = -desty;
					blockplot += row0 * blockwidth;
					startrow += row0 * roomwidth;
				}
				destplot	= startrow;

				for (row = row0; ((row < blockheight) && ((row + desty) < roomheight))
				; row ++)
				{
					for (column = 0; (column < blockwidth); column++)
					{
						if (StratifyPasteSTATE)
						{
							TileSetType *ptst = TILESET_AT_PLOT(blockplot);

							if (ptst)
							{
								UWORD uwDestLayer = ptst->LayerToUse;

								destlayer = Head(&prm->Layers);

								if (destlayer && !IsEOList(destlayer) && uwDestLayer > 0)
									while (--uwDestLayer)
									{
										if (IsEOList(destlayer))
										{
											destlayer = NULL;
											break;
										}
										destlayer = Next(destlayer);
									}

								if (destlayer && !IsEOList(destlayer) &&
								    (!EditOnlyFloorSTATE ||
								     (EditOnlyFloorSTATE && destlayer == floorlayer)))
								{
									if (destlayer->L_Flags & LAYER_LOCKED)
									{
										collision++;
										destplot = NULL;
									}
									else
									{
									/*	If we are stratify pasting into
										this layer for the first time, ... */

										if (destlayer->L_Flags & LAYER_NOT_YET_PASTED)
										{
											/* If tile grid size not set, then fill it in */

											if (!destlayer->cxTile)
												destlayer->cxTile = blocklayer->cxTile;

											if (!destlayer->cyTile)
												destlayer->cyTile = blocklayer->cyTile;

										/*	... and we are in replace mode,
											then make sure the rectangle is
											erase with NULL tiles first. */

											if (fStampMode == fSetStampReplace)
											{
												int x, y;

												for (y = row0; (y < blockheight) && (y + desty < roomheight); y++)
												{
													destplot = destlayer->Plot + (row_add + (y * roomwidth) + destx);
													for (x = 0; (x < blockwidth) && (x + destx < roomwidth); x++)
													{
														destplot->Plot_Flags	= 0;
														destplot->TileSet_ID	= 0;
														destplot->Tile_ID		= 0;
														destplot++;
													}
												}
											}
											destlayer->L_Flags &= ~(LAYER_NOT_YET_PASTED);
										}
										destplot = destlayer->Plot + (row_add + (row * roomwidth) + destx) + column;
									}
								}
								else
								{
									destplot = NULL;
								}
							}
							else
							{
								destplot = NULL;
							}
						}
						if (destplot)
						{
							if (((column + destx) < roomwidth) &&
							    (fStampMode == fSetStampReplace || blockplot->Tile_ID))
							{
								if (image_flag ||
								    (TU_DrawMode >= DMODE_COLOR1))
								{
									if (TU_DrawMode >= DMODE_COLOR1)
									{
#if __AMIGAOS__
										if (TU_DrawMode == DMODE_COLORREVERT)
										{
											destplot->Plot_Flags &=
												INVERSE_B(COLORGROUP_FLAG);
											destplot->Plot_Flags &=
												INVERSE_B(ubColorGroupMask);
										}
										else
										{
											destplot->Plot_Flags |=
												COLORGROUP_FLAG;
											destplot->Plot_Flags &=
												INVERSE_B(ubColorGroupMask);
											destplot->Plot_Flags |= 
												COLORGROUP_NUMBER(TU_DrawMode - DMODE_COLOR1);
										}
#elif __MSDOS__
										if (image_flag)
										{
											destplot->Plot_Flags &= GetTileBitsANDMaskL();
											destplot->Plot_Flags |= GetTileBitsORMaskL();
											destplot->Plot_Flags ^= GetTileBitsXORMaskL();
										}
										else
										{
											destplot->Plot_Flags &= GetTileBitsANDMaskR();
											destplot->Plot_Flags |= GetTileBitsORMaskR();
											destplot->Plot_Flags ^= GetTileBitsXORMaskR();
										}
										destplot->Plot_Flags &=
											FAST_TILESET_PTR(destplot->TileSet_ID)->ubShowFlagAND;
#endif/*__AMIGAOS__/__MSDOS__*/
									}
									else
									{
										*destplot = *blockplot;
									}
								}
								else
								{
									destplot->Plot_Flags	= 0;
									destplot->TileSet_ID	= 0;
									destplot->Tile_ID		= 0;
								}
							}
							destplot ++;
						}
						blockplot ++;
					}
					startrow	+= roomwidth;
					destplot	= startrow;
				}
			}
#else	/* !PLOTARRAY */
#if UseSparseArray
			if (((destlayer->L_Flags) & (LAYER_LOCKED)) && !StratifyPasteSTATE)
			{
				collision++;
			}
			else if (!IsSparse(blocklayer) && !IsSparse(destlayer) &&
					blocklayer->p.rgrgplt.mpYtMpXtPt)
			{
				RGRGPLT *prgrgpltLast = NULL;
				WORD wRowLast = -1;
				UWORD col0 = 0;
				UWORD row0 = 0;

				if (destx + destlayer->dtilx < 0)
				{
					col0 = -(destx + destlayer->dtilx);
				}
				if (desty + destlayer->dtily < 0)
				{
					row0 = -(desty + destlayer->dtily);
				}

				for (row = row0; ((row < blockheight) && ((row + desty + destlayer->dtily) < roomheight)) ; row++)
				{
					if (!StratifyPasteSTATE)
					{
						if (prgrgpltLast != NULL)
						{
							UpdatePlotRow(*prgrgpltLast, wRowLast);
						}
						prgrgpltLast = &destlayer->p.rgrgplt;
						wRowLast = row+desty+destlayer->dtily;
						destplot = ActivatePlotXYatWin(*prgrgpltLast,
									(col0+destx+destlayer->dtilx),
									wRowLast, winDst);
					}
					blockplot = ActivatePlotXYatWin(blocklayer->p.rgrgplt,
												col0, row, winSrc);

					for (column = col0; column < blockwidth; column++)
					{
						if (StratifyPasteSTATE)
						{
							TileSetType *ptst = TILESET_AT_PLOT(blockplot);

							if (ptst)
							{
								UWORD uwDestLayer = ptst->LayerToUse;

								destlayer = Head(&prm->Layers);

								if (destlayer && !IsEOList(destlayer) && uwDestLayer > 0)
									while (--uwDestLayer)
									{
										if (IsEOList(destlayer))
										{
											destlayer = NULL;
											break;
										}
										destlayer = Next(destlayer);
									}

								if (destlayer && !IsEOList(destlayer) &&
								    (!EditOnlyFloorSTATE ||
								     (EditOnlyFloorSTATE && destlayer == floorlayer)))
								{
									if (destlayer->L_Flags & LAYER_LOCKED)
									{
										collision++;
										destplot = NULL;
									}
									else
									{
										RGRGPLT *prgrgpltNew;	WORD wRowNew;

									/*	If we are stratify pasting into
										this layer for the first time, ... */

										if (destlayer->L_Flags & LAYER_NOT_YET_PASTED)
										{
											/* If tile grid size not set, then fill it in */

											if (!destlayer->cxTile)
												destlayer->cxTile = blocklayer->cxTile;

											if (!destlayer->cyTile)
												destlayer->cyTile = blocklayer->cyTile;

										/*	... and we are in replace mode,
											then make sure the rectangle is
											erase with NULL tiles first. */

											if (fStampMode == fSetStampReplace)
											{
												int x, y;

												for (y = row0; (y < blockheight) && (y + desty + destlayer->dtily < roomheight); y++)
												{
													prgrgpltNew = &destlayer->p.rgrgplt;
													wRowNew = y+desty+destlayer->dtily;
													if ((prgrgpltLast != prgrgpltNew) ||
														(wRowLast != wRowNew))
													{
														if (prgrgpltLast != NULL)
														{
															UpdatePlotRow(*prgrgpltLast, wRowLast);
														}
														prgrgpltLast = prgrgpltNew;
														wRowLast = wRowNew;
														destplot = ActivatePlotXYatWin(*prgrgpltLast,
																	(col0+destx+destlayer->dtilx),
																	wRowLast, winDst);
													}
													for (x = col0; (x < blockwidth) && (x + destx + destlayer->dtilx < roomwidth); x++)
													{
														destplot->Plot_Flags	= 0;
														destplot->TileSet_ID	= 0;
														destplot->Tile_ID		= 0;
														destplot++;
													}
												}
											}
											destlayer->L_Flags &= ~(LAYER_NOT_YET_PASTED);
										}
										prgrgpltNew = &destlayer->p.rgrgplt;
										wRowNew = row+desty+destlayer->dtily;

										if ((prgrgpltLast != prgrgpltNew) ||
											(wRowLast != wRowNew))
										{
											if (prgrgpltLast != NULL)
											{
												UpdatePlotRow(*prgrgpltLast, wRowLast);
											}
											prgrgpltLast = prgrgpltNew;
											wRowLast = wRowNew;
											destplot = NULL;	/* Following if() sets destplot */
										}
										if (!destplot)
											destplot = ActivatePlotXYatWin(*prgrgpltLast, destx+destlayer->dtilx+column, wRowLast, winDst);
									}
								}
								else
								{
									destplot = NULL;
								}
							}
							else
							{
								destplot = NULL;
							}
						}
						if (destplot)
						{
							if (((column + destx + destlayer->dtilx) < roomwidth) &&
							    (fStampMode == fSetStampReplace || blockplot->Tile_ID))
							{
								if (image_flag ||
								    (TU_DrawMode >= DMODE_COLOR1))
								{
									if (TU_DrawMode >= DMODE_COLOR1)
									{
										if (image_flag)
										{
											destplot->Plot_Flags &= GetTileBitsANDMaskL();
											destplot->Plot_Flags |= GetTileBitsORMaskL();
											destplot->Plot_Flags ^= GetTileBitsXORMaskL();
										}
										else
										{
											destplot->Plot_Flags &= GetTileBitsANDMaskR();
											destplot->Plot_Flags |= GetTileBitsORMaskR();
											destplot->Plot_Flags ^= GetTileBitsXORMaskR();
										}
										destplot->Plot_Flags &=
											FAST_TILESET_PTR(destplot->TileSet_ID)->ubShowFlagAND;
									}
									else
									{
										*destplot = *blockplot;
									}
								}
								else
								{
									destplot->Plot_Flags	= 0;
									destplot->TileSet_ID	= 0;
									destplot->Tile_ID		= 0;
								}
							}
							destplot++;
						}
						blockplot++;
					}
					ReleasePlotRow(blocklayer->p.rgrgplt, row);
				}
				if (prgrgpltLast != NULL)
				{
					UpdatePlotRow(*prgrgpltLast, wRowLast);
				}
			}
			else if (IsSparse(blocklayer))
			{
				PlotType *pplt;
				int ctily;
				int ctilx;
				int ixt, iyt;

				int col0 = 0;
				int row0 = 0;

				if (destx + destlayer->dtilx < 0)
				{
					col0 = -(destx + destlayer->dtilx);
				}
				if (desty + destlayer->dtily < 0)
				{
					row0 = -(desty + destlayer->dtily);
				}

				ctily = min(blockheight - row0,
						  roomheight - desty - destlayer->dtily - row0);
				ctilx = min(blockwidth - col0,
						  roomwidth - destx - destlayer->dtilx - col0);

				SetSparseLayerLimits(blocklayer->p.pspa,
								 col0, row0, ctilx, ctily);

				while ((pplt = GetNextSparsePlotInRange(blocklayer->p.pspa,
												&ixt, &iyt)) != NULL)
				{
					PlotType plt = *pplt;
					BOOL fPlaceDst = FALSE;

					if (StratifyPasteSTATE)
					{
						TileSetType *ptst = TILESET_AT_PLOT(&plt);

						if (ptst)
						{
							UWORD uwDestLayer = ptst->LayerToUse;

							destlayer = Head(&prm->Layers);

							if (destlayer && !IsEOList(destlayer) && uwDestLayer > 0)
								while (--uwDestLayer)
								{
									if (IsEOList(destlayer))
									{
										destlayer = NULL;
										break;
									}
									destlayer = Next(destlayer);
								}

							if (destlayer && !IsEOList(destlayer) &&
								(!EditOnlyFloorSTATE ||
								(EditOnlyFloorSTATE && destlayer == floorlayer)))
							{
								if (destlayer->L_Flags & LAYER_LOCKED)
								{
									collision++;
									fPlaceDst = FALSE;
								}
								else
								{
								/*	If we are stratify pasting into
									this layer for the first time, ... */

									if (destlayer->L_Flags & LAYER_NOT_YET_PASTED)
									{
										/* If tile grid size not set, then fill it in */

										if (!destlayer->cxTile)
											destlayer->cxTile = blocklayer->cxTile;

										if (!destlayer->cyTile)
											destlayer->cyTile = blocklayer->cyTile;

									/*	... and we are in replace mode,
										then make sure the rectangle is
										erase with NULL tiles first. */

										if (fStampMode == fSetStampReplace)
										{
											if (IsSparse(destlayer))
											{
												ZeroRectInSparseArray(destlayer->p.pspa,
																row0+destx+destlayer->dtilx,
																col0+desty+destlayer->dtily,
																ctilx, ctily);
											}
											else
											{
												int x, y;
												PlotType plt0 = {0, 0, 0};

												for (y = row0; (y < blockheight) && (y + desty + destlayer->dtily < roomheight); y++)
												{
													for (x = col0; (x < blockwidth) && (x + destx + destlayer->dtilx < roomwidth); x++)
													{
														LAY_WritePlotXY(&plt0,
																destlayer,
																x+destx+destlayer->dtilx,
																y+desty+destlayer->dtily);
													}
												}
											}
										}
										destlayer->L_Flags &= ~(LAYER_NOT_YET_PASTED);
									}
								}
							}
							else
							{
								fPlaceDst = FALSE;
							}
						}
						else
						{
							fPlaceDst = FALSE;
						}
					}
					if (fPlaceDst || !StratifyPasteSTATE)
					{
						if (fStampMode == fSetStampReplace || pplt->Tile_ID)
						{
							int dtilx, dtily;

							dtilx = ixt+destx+destlayer->dtilx;
							dtily = iyt+desty+destlayer->dtily;

							if (image_flag ||
								(TU_DrawMode >= DMODE_COLOR1))
							{
								if (TU_DrawMode >= DMODE_COLOR1)
								{
									LAY_ReadPlotXY(destlayer,
													dtilx, dtily,
													&plt);

									if (image_flag)
									{
										plt.Plot_Flags &= GetTileBitsANDMaskL();
										plt.Plot_Flags |= GetTileBitsORMaskL();
										plt.Plot_Flags ^= GetTileBitsXORMaskL();
									}
									else
									{
										plt.Plot_Flags &= GetTileBitsANDMaskR();
										plt.Plot_Flags |= GetTileBitsORMaskR();
										plt.Plot_Flags ^= GetTileBitsXORMaskR();
									}
									plt.Plot_Flags &=
										FAST_TILESET_PTR(plt.TileSet_ID)->ubShowFlagAND;
								}
							}
							else
							{
								plt.Plot_Flags	= 0;
								plt.TileSet_ID	= 0;
								plt.Tile_ID	= 0;
							}
							LAY_WritePlotXY(&plt, destlayer, dtilx, dtily);
						}
					}
				}
			}
			else				// blocklayer or destlayer is sparse
			{
				UWORD col0 = 0;
				UWORD row0 = 0;

				if (destx + destlayer->dtilx < 0)
				{
					col0 = -(destx + destlayer->dtilx);
				}
				if (desty + destlayer->dtily < 0)
				{
					row0 = -(desty + destlayer->dtily);
				}

				for (row = row0; ((row < blockheight) && ((row + desty + destlayer->dtily) < roomheight)) ; row++)
				{
					for (column = col0; column < blockwidth; column++)
					{
						PlotType plt;
						BOOL fPlaceDst = FALSE;

						LAY_ReadPlotXY(blocklayer, column, row, &plt);

						if (StratifyPasteSTATE)
						{
							TileSetType *ptst = TILESET_AT_PLOT(&plt);

							if (ptst)
							{
								UWORD uwDestLayer = ptst->LayerToUse;

								destlayer = Head(&prm->Layers);

								if (destlayer && !IsEOList(destlayer) && uwDestLayer > 0)
									while (--uwDestLayer)
									{
										if (IsEOList(destlayer))
										{
											destlayer = NULL;
											break;
										}
										destlayer = Next(destlayer);
									}

								if (destlayer && !IsEOList(destlayer) &&
								    (!EditOnlyFloorSTATE ||
								     (EditOnlyFloorSTATE && destlayer == floorlayer)))
								{
									if (destlayer->L_Flags & LAYER_LOCKED)
									{
										collision++;
										fPlaceDst = FALSE;
									}
									else
									{
									/*	If we are stratify pasting into
										this layer for the first time, ... */

										if (destlayer->L_Flags & LAYER_NOT_YET_PASTED)
										{
											/* If tile grid size not set, then fill it in */

											if (!destlayer->cxTile)
												destlayer->cxTile = blocklayer->cxTile;

											if (!destlayer->cyTile)
												destlayer->cyTile = blocklayer->cyTile;

										/*	... and we are in replace mode,
											then make sure the rectangle is
											erase with NULL tiles first. */

											if (fStampMode == fSetStampReplace)
											{
												if (IsSparse(destlayer))
												{
													int ctily = min(blockheight - row0,
							  									roomheight - desty - destlayer->dtily - row0);
													int ctilx = min(blockwidth - col0,
							  									roomwidth - destx - destlayer->dtilx - col0);

													ZeroRectInSparseArray(destlayer->p.pspa,
																	row0+destx+destlayer->dtilx,
																	col0+desty+destlayer->dtily,
																	ctilx, ctily);
												}
												else
												{
													int x, y;
													PlotType plt0 = {0, 0, 0};

													for (y = row0; (y < blockheight) && (y + desty + destlayer->dtily < roomheight); y++)
													{
														for (x = col0; (x < blockwidth) && (x + destx + destlayer->dtilx < roomwidth); x++)
														{
															LAY_WritePlotXY(&plt0,
																	destlayer,
																	x+destx+destlayer->dtilx,
																	y+desty+destlayer->dtily);
														}
													}
												}
											}
											destlayer->L_Flags &= ~(LAYER_NOT_YET_PASTED);
										}
									}
								}
								else
								{
									fPlaceDst = FALSE;
								}
							}
							else
							{
								fPlaceDst = FALSE;
							}
						}
						if (fPlaceDst || !StratifyPasteSTATE)
						{
							if (((column + destx + destlayer->dtilx) < roomwidth) &&
							    (fStampMode == fSetStampReplace || plt.Tile_ID))
							{
								UWORD dtilx, dtily;

								dtilx = column+destx+destlayer->dtilx;
								dtily = row+desty+destlayer->dtily;

								if (image_flag ||
								    (TU_DrawMode >= DMODE_COLOR1))
								{
									if (TU_DrawMode >= DMODE_COLOR1)
									{
										LAY_ReadPlotXY(destlayer,
														dtilx, dtily,
														&plt);

										if (image_flag)
										{
											plt.Plot_Flags &= GetTileBitsANDMaskL();
											plt.Plot_Flags |= GetTileBitsORMaskL();
											plt.Plot_Flags ^= GetTileBitsXORMaskL();
										}
										else
										{
											plt.Plot_Flags &= GetTileBitsANDMaskR();
											plt.Plot_Flags |= GetTileBitsORMaskR();
											plt.Plot_Flags ^= GetTileBitsXORMaskR();
										}
										plt.Plot_Flags &=
											FAST_TILESET_PTR(plt.TileSet_ID)->ubShowFlagAND;
									}
								}
								else
								{
									plt.Plot_Flags	= 0;
									plt.TileSet_ID	= 0;
									plt.Tile_ID	= 0;
								}
								LAY_WritePlotXY(&plt, destlayer, dtilx, dtily);
							}
						}
					}
				}
			}
#else // !UseSparseArray
			if (((destlayer->L_Flags) & (LAYER_LOCKED)) && !StratifyPasteSTATE)
			{
				collision++;
			}
			else if (blocklayer->rgrgplt.mpYtMpXtPt)
			{
				RGRGPLT *prgrgpltLast = NULL;
				WORD wRowLast = -1;
				UWORD col0 = 0;
				UWORD row0 = 0;

				if (destx + destlayer->dtilx < 0)
				{
					col0 = -(destx + destlayer->dtilx);
				}
				if (desty + destlayer->dtily < 0)
				{
					row0 = -(desty + destlayer->dtily);
				}

				for (row = row0; ((row < blockheight) && ((row + desty + destlayer->dtily) < roomheight)) ; row++)
				{
					if (!StratifyPasteSTATE)
					{
						if (prgrgpltLast != NULL)
						{
							UpdatePlotRow(*prgrgpltLast, wRowLast);
						}
						prgrgpltLast = &destlayer->rgrgplt;
						wRowLast = row+desty+destlayer->dtily;
						destplot = ActivatePlotXYatWin(*prgrgpltLast, (col0+destx+destlayer->dtilx), wRowLast, winDst);
					}
					blockplot = ActivatePlotXYatWin(blocklayer->rgrgplt, col0, row, winSrc);

					for (column = col0; column < blockwidth; column++)
					{
						if (StratifyPasteSTATE)
						{
							TileSetType *ptst = TILESET_AT_PLOT(blockplot);

							if (ptst)
							{
								UWORD uwDestLayer = ptst->LayerToUse;

								destlayer = Head(&prm->Layers);

								if (destlayer && !IsEOList(destlayer) && uwDestLayer > 0)
									while (--uwDestLayer)
									{
										if (IsEOList(destlayer))
										{
											destlayer = NULL;
											break;
										}
										destlayer = Next(destlayer);
									}

								if (destlayer && !IsEOList(destlayer) &&
								    (!EditOnlyFloorSTATE ||
								     (EditOnlyFloorSTATE && destlayer == floorlayer)))
								{
									if (destlayer->L_Flags & LAYER_LOCKED)
									{
										collision++;
										destplot = NULL;
									}
									else
									{
										RGRGPLT *prgrgpltNew;	WORD wRowNew;

									/*	If we are stratify pasting into
										this layer for the first time, ... */

										if (destlayer->L_Flags & LAYER_NOT_YET_PASTED)
										{
											/* If tile grid size not set, then fill it in */

											if (!destlayer->cxTile)
												destlayer->cxTile = blocklayer->cxTile;

											if (!destlayer->cyTile)
												destlayer->cyTile = blocklayer->cyTile;

										/*	... and we are in replace mode,
											then make sure the rectangle is
											erase with NULL tiles first. */

											if (fStampMode == fSetStampReplace)
											{
												int x, y;

												for (y = row0; (y < blockheight) && (y + desty + destlayer->dtily < roomheight); y++)
												{
													prgrgpltNew = &destlayer->rgrgplt;
													wRowNew = y+desty+destlayer->dtily;
													if ((prgrgpltLast != prgrgpltNew) ||
														(wRowLast != wRowNew))
													{
														if (prgrgpltLast != NULL)
														{
															UpdatePlotRow(*prgrgpltLast, wRowLast);
														}
														prgrgpltLast = prgrgpltNew;
														wRowLast = wRowNew;
														destplot = ActivatePlotXYatWin(*prgrgpltLast, (col0+destx+destlayer->dtilx), wRowLast, winDst);
													}
													for (x = col0; (x < blockwidth) && (x + destx + destlayer->dtilx < roomwidth); x++)
													{
														destplot->Plot_Flags	= 0;
														destplot->TileSet_ID	= 0;
														destplot->Tile_ID		= 0;
														destplot++;
													}
												}
											}
											destlayer->L_Flags &= ~(LAYER_NOT_YET_PASTED);
										}
										prgrgpltNew = &destlayer->rgrgplt;
										wRowNew = row+desty+destlayer->dtily;

										if ((prgrgpltLast != prgrgpltNew) ||
											(wRowLast != wRowNew))
										{
											if (prgrgpltLast != NULL)
											{
												UpdatePlotRow(*prgrgpltLast, wRowLast);
											}
											prgrgpltLast = prgrgpltNew;
											wRowLast = wRowNew;
											destplot = NULL;	/* Following if() sets destplot */
										}
										if (!destplot)
											destplot = ActivatePlotXYatWin(*prgrgpltLast, destx+destlayer->dtilx+column, wRowLast, winDst);
									}
								}
								else
								{
									destplot = NULL;
								}
							}
							else
							{
								destplot = NULL;
							}
						}
						if (destplot)
						{
							if (((column + destx + destlayer->dtilx) < roomwidth) &&
							    (fStampMode == fSetStampReplace || blockplot->Tile_ID))
							{
								if (image_flag ||
								    (TU_DrawMode >= DMODE_COLOR1))
								{
									if (TU_DrawMode >= DMODE_COLOR1)
									{
#if __AMIGAOS__
										if (TU_DrawMode == DMODE_COLORREVERT)
										{
											destplot->Plot_Flags &=
												INVERSE_B(COLORGROUP_FLAG);
											destplot->Plot_Flags &=
												INVERSE_B(ubColorGroupMask);
										}
										else
										{
											destplot->Plot_Flags |=
												COLORGROUP_FLAG;
											destplot->Plot_Flags &=
												INVERSE_B(ubColorGroupMask);
											destplot->Plot_Flags |= 
												COLORGROUP_NUMBER(TU_DrawMode - DMODE_COLOR1);
										}
#elif __MSDOS__
										if (image_flag)
										{
											destplot->Plot_Flags &= GetTileBitsANDMaskL();
											destplot->Plot_Flags |= GetTileBitsORMaskL();
											destplot->Plot_Flags ^= GetTileBitsXORMaskL();
										}
										else
										{
											destplot->Plot_Flags &= GetTileBitsANDMaskR();
											destplot->Plot_Flags |= GetTileBitsORMaskR();
											destplot->Plot_Flags ^= GetTileBitsXORMaskR();
										}
										destplot->Plot_Flags &=
											FAST_TILESET_PTR(destplot->TileSet_ID)->ubShowFlagAND;
#endif/*__AMIGAOS__/__MSDOS__*/
									}
									else
									{
										*destplot = *blockplot;
									}
								}
								else
								{
									destplot->Plot_Flags	= 0;
									destplot->TileSet_ID	= 0;
									destplot->Tile_ID		= 0;
								}
							}
							destplot++;
						}
						blockplot++;
					}
					ReleasePlotRow(blocklayer->rgrgplt, row);
				}
				if (prgrgpltLast != NULL)
				{
					UpdatePlotRow(*prgrgpltLast, wRowLast);
				}
			}
#endif // !UseSparseArray
#endif
			if (!StratifyPasteSTATE)
			{
				if (EditOnlyFloorSTATE)
					break;			/* do only floor layer */
				destlayer = Next (destlayer);
			}

			blocklayer = Next (blocklayer);

/*			bottom = FALSE;*/
		}

		/* Add in tile usage counts for newly pasted/erased area */

#if PixelLayers
		if (TU_DrawMode < DMODE_COLOR1)
			AddTilesInSubRect(&prm->Layers, dtilxKey, dtilyKey,
							ctilxKey, ctilyKey,
							1, &prm->prgtct, &prm->fcntValid);
#else // !PixelLayers
		if (TU_DrawMode < DMODE_COLOR1)
			AddTilesInSubRect(&prm->Layers, destx, desty,
							blockwidth, blockheight,
							1, &prm->prgtct, &prm->fcntValid);
#endif // !PixelLayers

		/* See if we are stratify pasting. If so,
		   then reset all LAYER_NOT_YET_PASTED bits. */

		if (StratifyPasteSTATE)
		{
			LayerType *play;

			if ((play = Head(&prm->Layers)) != NULL)
			{
				while (!IsEOList(play))
				{
					play->L_Flags &= ~(LAYER_NOT_YET_PASTED);
					play = Next(play);
				}
			}
		}

		/*******************************/
		/* Draw the newly stamped area */

#if __MSDOS__
		if (
			(GlobalBlockCopy->SourceRoom == block->DestRW->CurrentRoom->Room) &&
			(GlobalBlockCopy->SourceStuff == block->DestRW->CurrentRoom) ) {

			/* ERASE SOURCE OUTLINE */

			XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);
		}
#endif/*__MSDOS__*/
#if PIXELSELECT
//		RedrawSelectArea (block, TRUE, TRUE,
//					destx - block->DestRW->CurrentRoom->InitX,
//					desty - block->DestRW->CurrentRoom->InitY);

#if 1
		{
//#if PixelLayers
		UWORD cxTile;
		UWORD cyTile;

		LayerType *playBig = FindBiggestLayer(&prm->Layers);
//#endif // PixelLayers
		int cxTilDst, cyTilDst;
		int dtilx, dtily;
		int ctilx, ctily;
		int dpixx, dpixy;
		int cpixx, cpixy;
		int sep;

		sep = (block->DestStuff->Flags) & DISPLAY_SEPERATED;

#if PixelLayers
	{
		if (playBig && playBig->cxTile)
		{
			cxTile = playBig->cxTile;
			cyTile = playBig->cyTile;
		}
		else
		{
			cxTile = prm->FloorLayer->cxTile;
			cyTile = prm->FloorLayer->cyTile;
		}

		cxTilDst	= wSrcSizeToDstSize(cxTile, wSrcSkipX, wDstDupX) + sep;
		cyTilDst	= wSrcSizeToDstSize(cyTile, wSrcSkipY, wDstDupY) + sep;

		ctilx = blockwidth+ctilxOverhang;
		ctily = blockheight+ctilyOverhang;

		if (prm->FloorLayer->cxTile == 1 && prm->FloorLayer->cyTile == 1)
		{
#if 1
			BigTileCoordsFromSmallTile(&prm->Layers, prm->FloorLayer,
									&destx, &desty, &ctilx, &ctily);
#else
			Assert(cxTile != 0);
			ctilx = (destx + ctilx) / cxTile - destx / cxTile + 1;
			Assert(cyTile != 0);
			ctily = (desty + ctily) / cyTile - desty / cyTile + 1;
			destx = destx / cxTile;
			desty = desty / cyTile;
#endif

			// KLUDGE BUGBUG FIXME: Fudge values for width & height of pixel tile

			Assert(cxTilDst != 0);
			ctilx += ( cpixxObjectTile   +cxTilDst-1) / cxTilDst + 1;
			Assert(cyTilDst != 0);
			ctily += ( cpixyObjectTile   +cyTilDst-1) / cyTilDst + 1;
			destx -= ((cpixxObjectTile/2)+cxTilDst-1) / cxTilDst;
			desty -= ((cpixyObjectTile/2)+cyTilDst-1) / cyTilDst;

			if (destx < 0)
			{
				ctilx += destx;
				destx = 0;
			}
			if (desty < 0)
			{
				ctily += desty;
				desty = 0;
			}
		}

		Assert(cxTile != 0);
		dtilx = destx-(int)(block->DestRW->CurrentRoom->dxSrc/cxTile);
		Assert(cyTile != 0);
		dtily = desty-(int)(block->DestRW->CurrentRoom->dySrc/cyTile);

		if (dtilx < 0)
		{
			ctilx += dtilx;
			destx -= dtilx;
			dtilx = 0;
		}
		if (dtily < 0)
		{
			ctily += dtily;
			desty -= dtily;
			dtily = 0;
		}
	}
#else // !PixelLayers
		if (playBig && playBig->cxTile)
		{
			cxTile = playBig->cxTile;
			cyTile = playBig->cyTile;
		}
		else
		{
			cxTile = prm->FloorLayer->cxTile;
			cyTile = prm->FloorLayer->cyTile;
		}

		cxTilDst	= wSrcSizeToDstSize(cxTile, wSrcSkipX, wDstDupX) + sep;
		cyTilDst	= wSrcSizeToDstSize(cyTile, wSrcSkipY, wDstDupY) + sep;
//		cxTilDst	= wSrcSizeToDstSize(prm->FloorLayer->cxTile, wSrcSkipX, wDstDupX) + sep;
//		cyTilDst	= wSrcSizeToDstSize(prm->FloorLayer->cyTile, wSrcSkipY, wDstDupY) + sep;

		Assert(cxTile != 0);
		dtilx = destx-(int)(block->DestRW->CurrentRoom->dxSrc/cxTile);	//-ctilxO;
		Assert(cyTile != 0);
		dtily = desty-(int)(block->DestRW->CurrentRoom->dySrc/cyTile);	//-ctilyO;
//		dtilx = destx-(int)(block->DestRW->CurrentRoom->dxSrc/prm->FloorLayer->cxTile);	//-ctilxO;
//		dtily = desty-(int)(block->DestRW->CurrentRoom->dySrc/prm->FloorLayer->cyTile);	//-ctilyO;

		ctilx = blockwidth+ctilxOverhang;
		ctily = blockheight+ctilyOverhang;
#endif // !PixelLayers

		dpixx = dtilx*cxTilDst + 0;
		dpixy = dtily*cyTilDst + TopAdd;
		cpixx = ctilx*cxTilDst;
		cpixy = ctily*cyTilDst;

		SetPenColor(GlobalBackground);
		DrawRect(dpixx, dpixy, cpixx, cpixy);

		ClipPushClipValues(dpixx, dpixy, cpixx, cpixy);

#if PixelLayers
		ShowLayersInSubRect(&prm->Layers, prm->FloorLayer,
						block->DestRW, DisplayBM,
						dpixx, dpixy,
						SCREEN_WIDTH - dpixx, SCREEN_HEIGHT - dpixy,
						destx, desty,
						ctilx, ctily,
#if NEWTUME
						TRUE,
#else
						(IsGuideVisible(prm)),
#endif
						(IsComposite(prm) && IsLocked(prm)));
#else // !PixelLayers
		ShowLayersInSubRect(&prm->Layers, prm->FloorLayer,
						block->DestRW, DisplayBM,
						dpixx, dpixy,
						SCREEN_WIDTH - dpixx, SCREEN_HEIGHT - dpixy,
						destx, desty,
						ctilx, ctily,
#if NEWTUME
						TRUE,
#else
						(IsGuideVisible(prm)),
#endif
						(IsComposite(prm) && IsLocked(prm)));
#endif // !PixelLayers

		PopClipValues();
#endif
#endif // PIXELSELECT
#if !PIXELSELECT
		RedrawSelectArea (block, TRUE, TRUE,
					block->DestX - (int)(block->DestRW->CurrentRoom->dxSrc/cxTile),
					block->DestY - (int)(block->DestRW->CurrentRoom->dySrc/cyTile));
//		RedrawSelectArea (block, TRUE, TRUE,
//					block->DestX - (int)(block->DestRW->CurrentRoom->dxSrc/prm->FloorLayer->cxTile),
//					block->DestY - (int)(block->DestRW->CurrentRoom->dySrc/prm->FloorLayer->cyTile));
#endif // !PIXELSELECT
#if __MSDOS__
		if (
			(GlobalBlockCopy->SourceRoom == block->DestRW->CurrentRoom->Room) &&
			(GlobalBlockCopy->SourceStuff == block->DestRW->CurrentRoom) )
		{
			/* REDRAW SOURCE OUTLINE */

			XORDrawPBlk (GlobalBlockCopy, SOURCE_EOR, FALSE);
		}
#if PixelLayers
		CopyDisplayToBack(dpixx, dpixy, cpixx, cpixy, DISPLAYTOBACK);
#else // !PixelLayers
		CopyDisplayTilesToBack(block->DestRW, destx, desty,
						((LayerType *) Head(&block->Layers))->ctilx+ctilxOverhang,
						((LayerType *) Head(&block->Layers))->ctily+ctilyOverhang,
						DISPLAYTOBACK);
#endif // !PixelLayers

		if (fShowTileUsage && TU_DrawMode < DMODE_COLOR1)
		{
			CopyDisplayToBack(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd, BACKTODISPLAY);
			ShowCountsInSubRect(prm,
						GlobalBlockCopy->DestRW,
						0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT - TopAdd,
						0, 0,
						(int) (GlobalBlockCopy->DestStuff->dxSrc / cxTile),
						(int) (GlobalBlockCopy->DestStuff->dySrc / cyTile),
//						(int) (GlobalBlockCopy->DestStuff->dxSrc / prm->FloorLayer->cxTile),
//						(int) (GlobalBlockCopy->DestStuff->dySrc / prm->FloorLayer->cyTile),
						wShowXTiles, wShowYTiles);
		}
#endif/*__MSDOS__*/
		}
		if (! IsEOList (blocklayer))
		{
/*			ShowStatus ("WARNING : SHORT ON LAYERS"); */
		}
		if (collision)
		{
/*			ShowStatus ("WARNING : BLOCK COPY COLLISION."); */
		}
	}
#if REQUIRE
	else
	{
		/* printf ("Couldn't place new block copy.\n"); */
	}
#endif
	return;
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:
	return;

} /* PlaceBlockCopy */

#endif /* TEST_PRINT */

