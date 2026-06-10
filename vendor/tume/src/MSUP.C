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
 * MSUP.c
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 12/20/89
 *   MODIFIED : 10/02/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		C routine support for MCGA mode
 *
 * HISTORY
 *		??/??/92 (dcc) - MCGA_SaveArea() & MCGA_RestoreArea uses BigByteMaps.
 *		10/02/92 Friday (dcc) - Turn off stack checking in functions
 *							MCGA_DrawShape(), MCGA_ReallyClippedCopyRect(),
 *							and MCGA_ClippedCopyRect(), as they are called
 *							by the new mouse routines.
 *		12/04/93 Saturday (dcc) - updated for Watcom C.
 *
 * TODO
 *	* Fix ReallyClippedCopyRect so that it clips to the bytemap and not
 *	  to the GetClipValues.
*/

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

//#include <alloc.h>
//#include <mem.h>
#include <dos.h>
#include <limits.h>
#include <echidna/eerrors.h>
#include <echidna/graphics.h>
#include <echidna/pixlscan.h>

#if USE_GRAFX
#include <echidna/grafx.h>
#include <echidna/bbytemap.h>
#include <echidna/bilbm.h>
#include "tuglbl.h"
#endif

/***************************** E X T E R N A L ****************************/
extern far32 MCGA_ASMOpenGraphics (void);
extern far32 MCGA_ASMCloseGraphics (void);
extern far32 MCGA_DrawRectAsm (int x, int y, int width, int height);
#if USE_GRAFX
extern far32 MCGA_XORRect (int x, int y, int width, int height);
extern far32 MCGA_CopyRect (ByteMap *sbm, int fx, int fy,
						  ByteMap *dbm, int tx, int ty,
						  int width, int height);
#endif
extern GfxRoutinePtr MCGARoutineTable[];
extern GfxDataTbl	 MCGADataTable;

/**************************** C O N S T A N T S ***************************/
#define GHEADERSIZE	(GSHAPESIZE + MCGASDATASIZE)

/************************************  ************************************/

/*********************************************************************
 *
 * MCGA_GetGShapeLoadBuf
 *
 * SYNOPSIS
 *		void far32 *MCGA_GetGShapeLoadBuf (void)
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
void far32 *MCGA_GetGShapeLoadBuf (void)
{
#if __MSDOS32X__
	if (*BackBuffer) {
		return *BackBuffer;
	} else {
		return *DisplayBuffer;
	}
#else
	if (*BackBuffer) {
		return MK_FP(*BackBuffer, 0x0000);
	} else {
		return MK_FP(*DisplayBuffer, 0x0000);
	}
#endif
} /* MCGA_GetGShapeLoadBuf */

/*********************************************************************
 *
 * MCGA_DrawRect
 *
 * SYNOPSIS
 *		MCGA_DrawRect (int x, int y, int width, int height)
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
void MCGA_DrawRect (int x, int y, int width, int height)
{
	if (x < GetClipLeft()) {
		width = width - (GetClipLeft() - x);
		x     = GetClipLeft();
	}
	if (x > GetClipLeft() + GetClipWidth() - width) {
		width = (GetClipLeft() + GetClipWidth()) - x;
	}

	if (y < GetClipTop()) {
		height = height - (GetClipTop() - y);
		y      = GetClipTop();
	}
	if (y > GetClipTop() + GetClipHeight() - height) {
		height = (GetClipTop() + GetClipHeight()) - y;
	}

	if (width > 0 && height > 0) {
		MCGA_DrawRectAsm (x, y, width, height);
	}
} /* MCGA_DrawRect */

/*********************************************************************
 *
 * GetMCGARoutineTable
 *
 * SYNOPSIS
 *		GfxRoutinePtr *GetMCGARoutineTable (void)
 *
 * PURPOSE
 *		Return pointer to MCGARoutineTable.  Calling this routine
 *		should force loading of the MCGA routine overlay.
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
void GetMCGARoutineTable (
	GfxRoutinePtr	**grptr,
	GfxDataTbl		**gdptr
) {
	*grptr = MCGARoutineTable;
	*gdptr = &MCGADataTable;
} /* GetMCGARoutineTable */

#if !USE_GRAFX
/*********************************************************************
 *
 * MCGA_ConvertShape
 *
 * SYNOPSIS
 *		GShape	*MCGA_ConvertShape (
 *			GShape			*gshape, 
 *			int					 Hflip,
 *			int					 Vflip,
 *			UBYTE			 pcc,
 *			GShape			*outshape
 *		)
 *
 * PURPOSE
 *		To Create a shape usable by the MCGA shape routines.
 *
 * INPUT
 *		gshape:		Raw unpacked MCGA shape.
 *		attrib:		Value from BrushPak that tells me if I need to flip
 *					and/or remap the shape.
 *		outshape:	Temporary Work buffer that I can use during conversion.
 *
 * EFFECTS
 *		Makes Converted copy of 'gshape' in 'outshape' and then allocates 
 *		space for and copies 'outshape'
 *
 * RETURN VALUE
 *		Returns pointer to allocated MCGA shape if succesful.
 *		Returns NULL if unable to allocate space.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
GShape	*MCGA_ConvertShape (
	ConvertSType	*cs
)
{
	GShape	*gshape;
	UBYTE	*ps_ColorMap;
	int		 Hflip;
	int		 Vflip;
	GShape	*outshape;
	UBYTE 	*outbuf;
	UBYTE 	*countbyteptr;
	UBYTE 	 tcolor;
	UBYTE	 count;
	UWORD 	 origsize;
#if ERRORS
	ULONG 	 packedsize = 0;
#else
	UWORD 	 packedsize = 0;
#endif
	UBYTE	*ps_PixlPtr;
	int		 ps_Pixl;
	int		 ps_DeltaCol;
	int		 ps_DeltaRow;
	int		 ps_DWidth;
	int		 ps_2xDWidth;
	int		 ps_ColCount;
	UWORD	 ps_Size;


	if (!cs) {
		return NULL;
	}

	gshape 		= cs->ShapeData; 
	ps_ColorMap	= cs->ColorMap;
	Hflip		= cs->HFlip;
	Vflip		= cs->VFlip;

	/* Allocate Space for Worst Case Packing */
	{ ULONG size;
		size = 3 * (ULONG)gshape->Size + GHEADERSIZE;
		origsize = min (size, (long)USHRT_MAX);
	}

	if (! (outshape = (GShape *) MEM_malloc (origsize)))
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("OOM: MCGA_ConvertShape");
		return NULL;
	}

	/* Copy Shape Header info to outshape */
	memcpy (outshape, gshape, GHEADERSIZE);
   
	ResetPixlScan (gshape, Hflip, Vflip);
	if (Hflip) {
		(outshape)->Data.MCGA.XOffset =	(outshape)->Width -
			ps_DWidth - (outshape)->Data.MCGA.XOffset;
	}
	if (Vflip) {
		(outshape)->Data.MCGA.YOffset =	(outshape)->Height -
			 (outshape)->Data.MCGA.DHeight - (outshape)->Data.MCGA.YOffset;
	}

	/* Pack Shape */
	outbuf = ((UBYTE *)outshape) + GHEADERSIZE;
	tcolor = gshape->TColor;
	while (ps_Size) {
		/* Count Run of Transparent Pixels */
		count = 0;
		while (++count && ps_ColorMap[*ps_PixlPtr] == tcolor && ps_Size) {
			{												
				--ps_Size;
				if (++ps_ColCount < ps_DWidth) {			
					ps_PixlPtr += ps_DeltaCol;				
				} else {									
					ps_ColCount = 0;						
					ps_PixlPtr += ps_DeltaCol + ps_DeltaRow;
				}											
			} /* GetPixl(); */
			/*if (++count == 255) break; */
		}
		*outbuf++ = count - 1;
		++packedsize;

		/* Count Run of Non-Transparent Pixels */
		countbyteptr = outbuf++;
		++packedsize;
		count = 0;
		while (++count && (ps_Pixl = ps_ColorMap[*ps_PixlPtr]) != tcolor && 
			ps_Size) 
		{
			*outbuf++ = ps_Pixl + 1;
			{												
				--ps_Size;
				if (++ps_ColCount < ps_DWidth) {			
					ps_PixlPtr += ps_DeltaCol;				
				} else {									
					ps_ColCount = 0;						
					ps_PixlPtr += ps_DeltaCol + ps_DeltaRow;
				}											
			} /* GetPixl() */
			/*if (++count == 255) break; */
		}
		--count;
		*countbyteptr = count;
		packedsize += count;
	}
	
#if ERRORS
	if (packedsize > (long)USHRT_MAX) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("MCGA_ConvertShape: Converted brush too large ( > 65535)\n");
		return NULL;
	}
#endif

	outshape->Size = packedsize + GHEADERSIZE;

	/* ReAlloc result to proper size. */
	freesome (origsize - outshape->Size);

	return outshape;
} /* MCGA_ConvertShape */
#else
GShape *MCGA_ConvertShape (ConvertSType *cs)
{
	GShape	*gs;
	GShape	*origshape;
	size_t	 size;

	if (!cs) {
		return NULL;
	}

	origshape = cs->ShapeData;
	size      = origshape->Size + GSHAPESIZE + MCGASDATASIZE;

	gs = MEM_malloc (size);
	if (gs) {
		memcpy (gs, origshape, size);
	}

	return gs;
}
#endif

/*********************************************************************
 *
 * MCGA_FreeShape
 *
 * SYNOPSIS
 *		void	MCGA_FreeShape (
 *			GShape	*shapeptr
 *		)
 *
 * PURPOSE
 *		To Free the memory allocated for shape by MCGA_ConvertShape.
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
void MCGA_FreeShape(GShape *shape)
{
	MEM_free (shape);
} /* MCGA_FreeShape */

#if USE_GRAFX

#if __TURBOC__
#pragma option -N-
#elif __WATCOMC__
#pragma off (check_stack);
#endif

void MCGA_DrawShape (GShape *shape, int tx, int ty)
{
	int	w;
	int	h;
	int	fx = 0;
	int	fy = 0;
	int	sw;

	{
		int	iw;
		int	ih;

		tx += shape->Data.MCGA.XOffset;
		ty += shape->Data.MCGA.YOffset;
		iw  = w = sw = shape->Data.MCGA.DWidth;
		ih  = h =      shape->Data.MCGA.DHeight;

		if (tx < GetClipLeft()) {
			w   = iw - (GetClipLeft() - tx);
			tx  = GetClipLeft();
			fx += iw - w;
		}
		if (tx > GetClipLeft() + GetClipWidth() - w) {
			w   = (GetClipLeft() + GetClipWidth()) - tx;
		}

		if (ty < GetClipTop()) {
			h   = ih - (GetClipTop() - ty);
			ty  = GetClipTop();
			fy += ih - h;
		}
		if (ty > GetClipTop() + GetClipHeight() - h) {
			h   = (GetClipTop() + GetClipHeight()) - ty;
		}
	}


	if (w > 0 && h > 0) {
		int	 smod;
		int	 dmod;
		UBYTE	*src;
		UBYTE	*dest;
		UBYTE	 pixel;
		UBYTE	 tcolor;
		int	 i;

		BeforeGraphics();

		smod   = sw  - w;
		dmod   = SCREEN_WIDTH - w;
		tcolor = shape->TColor;
		src    = ((UBYTE *)shape) + GSHAPESIZE + MCGASDATASIZE + fx + fy * sw;
#if __MSDOS32X__
		dest   = *GrDraw + tx + ty * SCREEN_WIDTH;
#else
		dest   = MK_FP (*GrSeg, tx + ty * SCREEN_WIDTH);
#endif

		for (; h > 0; h--) {
			for (i = w; i > 0; i--) {
				pixel = *src++;
				if (pixel != tcolor) {
 					*dest++ = RemapTable[pixel];
				} else {
					dest++;
				}
			}
			dest += dmod;
			src  += smod;
		}

		AfterGraphics ();

		
	}

}

#if __TURBOC__
#pragma option -N.
#endif

/*********************************************************************
 *
 * MCGA_ClippedXORRect
 *
 * SYNOPSIS
 *		void MCGA_ClippedXORRect (int x, int y, int w, int h)
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
void MCGA_ClippedXORRect (int x, int y, int width, int height)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MCGA_ClippedXORRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		if (x < GetClipLeft()) {
			width = width - (GetClipLeft() - x);
			x     = GetClipLeft();
		} 
		if (x > GetClipLeft() + GetClipWidth() - width) {
			width = (GetClipLeft() + GetClipWidth()) - x;
		}

		if (y < GetClipTop()) {
			height = height - (GetClipTop() - y);
			y      = GetClipTop();
		}

		if (y > GetClipTop() + GetClipHeight() - height) {
			height = (GetClipTop() + GetClipHeight()) - y;
		}

		if (width > 0 && height > 0) {
			MCGA_XORRect (x, y, width, height);
		}
	}

} /* MCGA_ClippedXORRect */

#if __TURBOC__
#pragma option -N-
#elif __WATCOMC__
#pragma off (check_stack);
#endif

/*********************************************************************
 *
 * MCGA_ClippedCopyRect
 *
 * SYNOPSIS
 *		void MCGA_ClippedCopyRect (
 *							ByteMap *sbm,
 *							int	 fx,
 *							int	 fy,
 *							ByteMap	*dbm,
 *							int	 tx,
 *							int	 ty,
 *							int	 inwidth,
 *							int	 inheight
 *						)
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
void MCGA_ClippedCopyRect (
					ByteMap *sbm,
					int	 fx,
					int	 fy,
					ByteMap	*dbm,
					int	 tx,
					int	 ty,
					int	 iw,
					int	 ih
				)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MCGA_ClippedCopyRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/


	{
		int	w;
		int	h;

		w = iw;
		h = ih;

		if (tx < 0) {
			w   = iw - (0 - tx);
			tx  = 0;
			fx += iw - w;
		}
		if (tx > dbm->width - w) {
			w   = (dbm->width) - tx;
		}

		if (ty < 0) {
			h   = ih - (0 - ty);
			ty  = 0;
			fy += ih - h;
		}
		if (ty > dbm->height - h) {
			h   = (dbm->height) - ty;
		}

		if (w > 0 && h > 0) {
			MCGA_CopyRect (sbm, fx, fy, dbm, tx, ty, w, h);
		}
	}

} /* MCGA_ClippedCopyRect */

/*********************************************************************
 *
 * MCGA_ReallyClippedCopyRect
 *
 * SYNOPSIS
 *		void MCGA_ReallyClippedCopyRect (
 *							ByteMap *sbm,
 *							int	 fx,
 *							int	 fy,
 *							ByteMap	*dbm,
 *							int	 tx,
 *							int	 ty,
 *							int	 inwidth,
 *							int	 inheight
 *						)
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
void MCGA_ReallyClippedCopyRect (
					ByteMap *sbm,
					int	 fx,
					int	 fy,
					ByteMap	*dbm,
					int	 tx,
					int	 ty,
					int	 iw,
					int	 ih
				)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MCGA_ReallyClippedCopyRect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/


	{
		int	w;
		int	h;

		w = iw;
		h = ih;


		if (tx < GetClipLeft()) {
			w   = iw - (GetClipLeft() - tx);
			tx  = GetClipLeft();
			fx += iw - w;
		}
		if (tx > GetClipLeft() + GetClipWidth() - w) {
			w   = (GetClipLeft() + GetClipWidth()) - tx;
		}

		if (ty < GetClipTop()) {
			h   = ih - (GetClipTop() - ty);
			ty  = GetClipTop();
			fy += ih - h;
		}
		if (ty > GetClipTop() + GetClipHeight() - h) {
			h   = (GetClipTop() + GetClipHeight()) - ty;
		}

		if (w > 0 && h > 0) {
			MCGA_ClippedCopyRect (sbm, fx, fy, dbm, tx, ty, w, h);
		}
	}

} /* MCGA_ReallyClippedCopyRect */

#if __TURBOC__
#pragma option -N.
#endif

#endif

/*********************************************************************
 *
 * MCGA_OpenGraphics
 *
 * SYNOPSIS
 *		short MCGA_OpenGraphics(void)
 *
 * PURPOSE
 *		Init MCGA graphics routines.
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
short MCGA_OpenGraphics(void)
{

	MCGA_ASMOpenGraphics ();

	return 1;

} /* MCGA_OpenGraphics */

/*********************************************************************
 *
 * MCGA_CloseGraphics
 *
 * SYNOPSIS
 *		void MCGA_CloseGraphics (void)
 *
 * PURPOSE
 *		Free resources used by MCGA graphics routines.
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
void MCGA_CloseGraphics (void)
{
	MCGA_ASMCloseGraphics ();

} /* MCGA_CloseGraphics */

/*********************************************************************
 *
 * MCGA_DrawShapeNoTrans
 *
 * SYNOPSIS
 *		MCGA_DrawShapeNoTrans (GShape *shape, int x, int y)
 *
 * PURPOSE
 *		DrawShape without transparency.
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
void MCGA_DrawShapeNoTrans (GShape *shape, int x, int y)
{
	SetPenColor (shape->TColor);
	DrawRect (x, y, shape->Width, shape->Height);
	DrawGShape (shape, x, y);

} /* MCGA_DrawShapeNoTrans */

#if USE_GRAFX
/*********************************************************************
 *
 * MCGA_RestoreArea
 *
 * SYNOPSIS
 *		void MCGA_RestoreArea (SaveAreaType *sa)
 *
 * PURPOSE
 *		Restore area of screen defined by <sa>.
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
void MCGA_RestoreArea (SaveAreaType *sa)
{
	BigByteMap	 srcbbm;
	ByteMap		 destbm;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MCGA_RestoreArea";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	srcbbm.width		= sa->Width;
	srcbbm.height		= sa->Height;
	srcbbm.mpYpMpXpPix	= sa->Data;

	destbm.width  = SCREEN_WIDTH;
	destbm.height = SCREEN_HEIGHT;
	destbm.data   = GetDisplayAddress();

	BeforeGraphics();
	PushClipValues(0, 0, 32767, 32767);
	CopyBigRectToSmall (&srcbbm, 0, 0, &destbm, sa->X, sa->Y,
						sa->Width, sa->Height);
	PopClipValues();
	AfterGraphics();

	FreeBigByteMap (srcbbm.mpYpMpXpPix, srcbbm.height);
	MEM_free (sa);

} /* MCGA_RestoreArea */


/*********************************************************************
 *
 * MCGA_SaveArea
 *
 * SYNOPSIS
 *		SaveAreaType *MCGA_SaveArea (int x, int y, int w, int h)
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
SaveAreaType *MCGA_SaveArea (int x, int y, int w, int h)
{
	SaveAreaType	*sa;
	MPYPMPXPPIX		 mpYpMpXpPix;
	ByteMap			 srcbm;
	BigByteMap		 destbbm;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MCGA_SaveArea";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	sa = MEM_calloc (1, sizeof (SaveAreaType));
	if (!sa) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("OOM: MCGA_SaveArea");
		return NULL;
	}

/* KLUDGE! KLUDGE! KLUDGE! KLUDGE! KLUDGE! */
/* In order to make XMS run faster, we make sure width <w> is even. */

	if (w & 1)
		w++;

/* KLUDGE! KLUDGE! KLUDGE! KLUDGE! KLUDGE! */

	mpYpMpXpPix = AllocateBigByteMap (w, h);
	if (!mpYpMpXpPix) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("OOXM: MCGA_SaveArea");
		return NULL;
	}

	sa->X		= x;
	sa->Y		= y;
	sa->Width	= w;
	sa->Height	= h;
	sa->Data	= mpYpMpXpPix;

	destbbm.width		= w;
	destbbm.height		= h;
	destbbm.mpYpMpXpPix	= mpYpMpXpPix;

	srcbm.width  = SCREEN_WIDTH;
	srcbm.height = SCREEN_HEIGHT;
	srcbm.data   = GetDisplayAddress();

	BeforeGraphics();
	PushClipValues(0, 0, 32767, 32767);
	CopySmallRectToBig (&srcbm, x, y, &destbbm, 0, 0, w, h);
	PopClipValues();
	AfterGraphics();

	return sa;

} /* MCGA_SaveArea */

#endif /* USE_GRAFX */

