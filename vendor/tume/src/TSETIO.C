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
 * TILESETIO.C
 *
 * PROGRAMMER : R.G. Marquez
 *    VERSION : 00.000
 *    CREATED : 08/22/89
 *   MODIFIED : 01/03/95
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		Routines to load and save a tileset.
 *
 * HISTORY
 *		08/22/89 (RGM) - Created.
 *		09/04/89 (RGM) - Debugged LoadTileSet() and StampTile().
 *					  (StampTile is now in tUMEdraw.c)
 *		03/17/93 (dcc) - newtileset->SharedNumberColors always = MAXCOLORREG.
 *		01/03/95 (dcc) - Fix loaded BOXED pictures crashing with div by 0.
 *					  (broke v00.2650).
 *
*/
#include <echidna/platform.h>
#include <echidna/etypes.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#if __AMIGAOS__
#include <exec/memory.h>
#endif/*__AMIGAOS__*/
#include <string.h>

#include "tuglbl.h"

#include <echidna/eio.h>
#include <echidna/eerrors.h>

#if MEM_TEST
#define AllocMem(a,b)	TEST_AllocMem(a,b)
#define FreeMem(a,b)	TEST_FreeMem(a,b)
#endif

#if __MSDOS__
#include <echidna/grafx.h>
#endif/*__MSDOS__*/

#include "rm_tsinf.h"
#include "rmtsglue.h"
#include "collect.h"
#include "histogrm.h"
#include "szerror.h"

/**************************** C O N S T A N T S ***************************/

#define MAX_FNAME	128
#define MAX_PATH	256

#define OKMAX	(96)
#define MAXMAX	(256)

#define TILEFILES	"TILE"

#if __AMIGAOS__
#define STR_DIRSEP	"/"
#elif __MSDOS__
#define STR_DIRSEP	"\\"
#endif/*__AMIGAOS__/__MSDOS__*/

#define BUNCHOFTILES	80

#if __MSDOS__
#define sizeofTileCollection (32768U)
#endif/*__MSDOS__*/

/******************************** T Y P E S *******************************/

typedef struct {
	BOOL	be_first;
	BOOL	be_blank;
	BOOL	be_repeat;
	BOOL	be_gone;
} TileStatusType;

/****************************** E X T E R N S *****************************/


/****************************** G L O B A L S *****************************/


/******************************* L O C A L S ******************************/

static	char	ScratchDirName[EIO_MAXDIR + 1];
static	char	ScratchFileName[EIO_MAXFILE + 1];
static	char	ScratchExtName[EIO_MAXEXT + 1];

#if __AMIGAOS__
static	ListType lstFreeTiles;
#endif/*__AMIGAOS__*/

extern BOOL PixelScan (
	TileSourceType	*tilesource,
	short			start_x,
	short			start_y,
	short			end_x,
	short			end_y,
	UWORD			operation,
	short			color,
	short			*pixel_x,
	short			*pixel_y
);

#if 0
extern void PrintPlanes (
	UBYTE	**planes,
	short	width,
	short	height,
	short	depth,
	short	start
);
#endif

/******************************* M A C R O S ******************************/


#define SCAN_TYPE_MODIFIER		((UWORD) (0x0F << 0))
#define CORNER_SCAN				((UWORD) (0x00 << 0))
#define LINE_SCAN				((UWORD) (0x01 << 0))
#define EDGE_SCAN				((UWORD) (0x02 << 0))

#define SCAN_DIRECTION_MODIFIER	((UWORD) (0x03 << 4))
#define HORIZONTAL_SCAN			((UWORD) (0x00 << 4))
#define VERTICAL_SCAN			((UWORD) (0x01 << 4))

#define SCAN_UNTIL_MODIFIER		((UWORD) (0x03 << 6))
#define SCAN_UNTIL_NOT			((UWORD) (0x00 << 6))
#define SCAN_UNTIL				((UWORD) (0x01 << 6))

#define NOTEQU_EQU(flag, a, b)	((flag) ? (a != b) : (a == b))

/***************************** R O U T I N E S ****************************/



/**************************************************************************
 *
 * PixelScan
 *
 * SYNOPSIS
 *		static BOOL PixelScan (
 *			TileSourceType	*tilesource,
 *			short			start_x,
 *			short			start_y,
 *			short			end_x,
 *			short			end_y,
 *			UWORD			operation,
 *			short			color,
 *			short			*pixel_x,
 *			short			*pixel_y,
 *		)
 *
 * PURPOSE
 *		Scan pixels according to operation until operation says it's
 *		time to stop.  Return resultant coordinates & TRUE if successful,
 *		FALSE otherwise.
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *		If PixelScan returns TRUE, *pixel_x & *pixel_y will contain
 *		resultant coordinates, otherwise they remain unchanged.
 *
 * RETURN VALUE
 *		TRUE if ok & changed *pixel_x & *pixel_y, FALSE otherwise.
 *
 * HISTORY
 *	03/08/91 Friday (RGM) - Created.
 *
 *
 * SEE ALSO
 *
*/

BOOL PixelScan (
	TileSourceType	*tilesource,
	short			start_x,
	short			start_y,
	short			end_x,
	short			end_y,
	UWORD			operation,
	short			color,
	short			*pixel_x,
	short			*pixel_y
)
{
	UWORD	scan_type;
	UWORD	scan_dir;
	UWORD	scan_until;

	short	check_color;

	short	source_width;
	short	source_height;

#if __AMIGAOS__
	struct	RastPort	*rp;
#elif __MSDOS__
	BigByteMap	*rp;
#endif/*__AMIGAOS__/__MSDOS__*/

	short	real_x;
	short	real_y;

	BOOL	result	= FALSE;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PixelScan";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	rp	= TileSourceRastPort (tilesource);

	scan_type	= operation & SCAN_TYPE_MODIFIER;
	scan_dir	= operation & SCAN_DIRECTION_MODIFIER;
	scan_until	= operation & SCAN_UNTIL_MODIFIER;

	source_width	= TileSourceWidth (tilesource);
	source_height	= TileSourceHeight (tilesource);

	if (end_x < 0) end_x = source_width - 1;
	if (end_y < 0) end_y = source_height - 1;

	check_color		= (scan_until == SCAN_UNTIL) ? color + 1 : color;

	if (scan_type == CORNER_SCAN) {

		short	scan_position;
		short	max_scan_position;
		short	x;
		short	y;

		/************************************************************/
		/* Do a corner scan of the area, don't stop until we're out */
		/* of the area or we've found the pixel.                    */

		scan_position		= 0;
		max_scan_position	= max (end_x - start_x, end_y - start_y);

		while (NOTEQU_EQU(scan_until,check_color,color)
		&& (scan_position <= max_scan_position)) {

			x	= start_x + scan_position;
			if (x > end_x) x = end_x;

			while (NOTEQU_EQU(scan_until,check_color,color) && (x >= start_x)) {
				y	= (start_y + scan_position) - x;
				if (y > end_y) y = end_y;

				check_color	= ReadPixel (rp, x, y);
				--x;
			}
			scan_position++;
		}

		/*******************************************************************/
		/* We've either found the pixel or we've scanned the whole area.   */

		if (! NOTEQU_EQU(scan_until,check_color,color)) {
			x	+= 1;
			scan_position	-= 1;

			real_x	= x;
			real_y	= start_y + scan_position - x;

			result	= TRUE;
		}
	}
	else if (scan_type == LINE_SCAN) {
		if (scan_dir == HORIZONTAL_SCAN) {
			short	x;
			short	y;

			x	= start_x;
			y	= start_y;

			while (NOTEQU_EQU(scan_until,check_color,color) && (x <= end_x)){
				check_color	= ReadPixel (rp, x, y);
				x ++;
			}
			if (! NOTEQU_EQU(scan_until,check_color,color)) {
				x	-= 1;

				real_x	= x;
				real_y	= y;

				result	= TRUE;
			}
		}
		else if (scan_dir == VERTICAL_SCAN) {
			short	x;
			short	y;

			x	= start_x;
			y	= start_y;

			while (NOTEQU_EQU(scan_until,check_color,color) && (y <= end_y)){
				check_color	= ReadPixel (rp, x, y);
				y ++;
			}
			if (! NOTEQU_EQU(scan_until,check_color,color)) {
				y	-= 1;

				real_x	= x;
				real_y	= y;

				result	= TRUE;
			}
		}
	}
	else if (scan_type == EDGE_SCAN) {
		short	x;
		short	y;
		UWORD	new_operation;

		new_operation	=
			(operation & SCAN_UNTIL_MODIFIER) | LINE_SCAN | HORIZONTAL_SCAN;

		result	= PixelScan (
					tilesource,
					start_x,
					start_y,
					end_x,
					end_y,
					new_operation,
					color,
					&x,
					&y);

		if (result) {
			real_x	= x;

			new_operation	=
			(operation & SCAN_UNTIL_MODIFIER) | LINE_SCAN | VERTICAL_SCAN;

			result	= PixelScan (
						tilesource,
						start_x, start_y,
						end_x, end_y,
						new_operation,
						color,
						&x, &y);
			if (result) {
				real_y	= y;
			}
		}
	}

	if (result) {
		*pixel_x	= real_x;
		*pixel_y	= real_y;
	}

	return (result);

} /* PixelScan */


#if __AMIGAOS__
/*********************************************************************
 *
 * pTilDelete
 *
 * SYNOPSIS
 *		void pTilDelete(TileType *ptil)
 *
 * PURPOSE
 *		Add ptil to lstFreeTiles.
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
void pTilDelete(TileType *ptil)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "pTilDelete";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	AddTail(&lstFreeTiles, ptil);

} /* pTilDelete */


/*********************************************************************
 *
 * pTilNew
 *
 * SYNOPSIS
 *		TileType *pTilNew(void)
 *
 * PURPOSE
 *		Return a TileType from lstFreeTiles if available,
 *		otherwise, allocate a new block of TileTypes.
 *
 *		Rationale: currently, sizeof (TileType) == 19. Turbo C allocates
 *		in 16 byte units. Therefore, if you allocate TileType one at a
 *		time, you will waste 13 bytes each time (32 - 19 = 13). Thus we
 *		allocate BUNCHOFTILES at once.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Pointer to a free TileType to use.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
TileType *pTilNew(void)
{

	static BOOL	lstFreeInited = FALSE;
	UBYTE		*pMem;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "pTilNew";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!lstFreeInited)
	{
		InitList(&lstFreeTiles);
		lstFreeInited = TRUE;
	}

	if ((pMem = RemTail(&lstFreeTiles)) != NULL)
	{
		memset(pMem, 0, TILESIZE);	/* zero out node info */
		return (TileType *) pMem;
	}
	else
	{
		pMem = MEM_calloc (BUNCHOFTILES, TILESIZE);
		if (pMem)
		{
			int i;

			for (i = 1; i < BUNCHOFTILES; i++)
			{
				AddTail(&lstFreeTiles, pMem + i * TILESIZE);
			}
			return (TileType *) pMem;
		}
	}

	return NULL;
} /* pTilNew */


/**************************************************************************
 *
 * DeAllocateTile
 *
 * SYNOPSIS
 *		static void DeAllocateTile (
 *			TileSetType	*tileset,
 *			TileType		*tile
 *		)
 *
 * PURPOSE
 *	Free up the resources used by a tile.
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
 *		09/04/89 Monday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
static void DeAllocateTile (
	TileSetType	*tileset,
	TileType		*tile
)
{
	WORD		width, height;
	WORD		depth;
	int			rassize;
	short		i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeAllocateTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (tile) {
		if (tileset) {
			width	= tileset->Width;
			height	= tileset->Height;
			depth	= tileset->Depth;

			rassize = RASSIZE (width, height);

			for (i = 0; i < depth; i++) {
				if (tile->ImageBitMap.Planes[i]) {
					free_chips (tile->ImageBitMap.Planes[i], rassize);
				}
			}
			if (tile->MaskPlanes[0]) {
				free_chips (tile->MaskPlanes[0], rassize);
			}

			pTilDelete (tile);
		}
	}

	return;

} /* DeAllocateTile */
#endif/*__AMIGAOS__*/


/**************************************************************************
 *
 * UnLoadTileSet
 *
 * PURPOSE
 *		Unloads (de-allocates) a tileset.
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
 *		NONE.
 *
 * HISTORY
 *		09/04/89 Monday - Created. (RGM)
 *		03/12/90 Monday - added stuff for comments. (RGM)
#if PIXELSELECT
 *		12/21/93 Tuesday (dcc) - added support for <pdxLeft> & <pdxRight>.
 *		01/11/94 Tuesday (dcc) - ... <pdyTop>, and <pdyBottom>.
#endif // PIXELSELECT
 *
 * SEE ALSO
 *
*/
void UnLoadTileSet (TileSetType	*tileset)
{
#if __AMIGAOS__
	TileType	*tile;
#endif/*__AMIGAOS__*/
	short		i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UnLoadTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (tileset) {

		/*********************************************/
		/* Get rid of color information in XTRA mem. */

		DeleteColorInfo(tileset->cinShared);

		/*********************************************/
		/* de-allocate the tiles and their bitplanes */

#if __AMIGAOS__
		while ((tile = RemHead (&(tileset->Tiles))) != NULL) {
			DeAllocateTile (tileset, tile);
		}
#endif/*__AMIGAOS__*/

#if __MSDOS__
		/*************************************************/
		/* Get rid of collection of tiles in XTRA memory */

		{
			UWORD i;

			for (i = 0; i < tileset->numTilCols; i++)
			{
				FreeXTRA(tileset->pxtrTilCol[i]);
			}
			MEM_free(tileset->pxtrTilCol);
		}

		/**************************************************/
		/* Get rid of collection of deltas in XTRA memory */

		{
			UWORD i;

			for (i = 0; i < tileset->numDelCols; i++)
			{
				FreeXTRA(tileset->pxtrDelCol[i]);
			}
			MEM_free(tileset->pxtrDelCol);
		}
#endif/*__MSDOS__*/

#if __AMIGAOS__
		/****************************************/
		/* get rid of the group selector planes */

		if (tileset->FullPlane) {
			free_chips (tileset->FullPlane, tileset->PlaneSize);
		}
		if (tileset->EmptyPlane) {
			free_chips (tileset->EmptyPlane, tileset->PlaneSize);
		}

		/********************************/
		/* get rid of the tileptr array */

		SAFEFREE (tileset->TilePtr);

		/*****************************************************/
		/* before we can de-allocate the shape, we've gotta  */
		/* patch the original bitmap and mask into the shape */
		/* then de-allocate the shared shape                 */

		if (tileset->SharedShape) {
			for (i= 0; i < MAXPLANES; i++) {
				tileset->SharedShape->Pic->BitMap.Planes[i] =
					tileset->OriginalImagePlanes[i];

				tileset->SharedShape->MaskBm.Planes[i] =
					tileset->OriginalMaskPlanes[i];
			}
			DeleteShape (tileset->SharedShape);
			tileset->SharedShape = NULL;
		}
#endif/*__AMIGAOS__*/

		/*************************************************************/
		/* next, de-allocate the mem used for the tileset's filename */

		MEM_free (tileset->Filespec);

		/*******************************************************/
		/* de-allocate the mem used for the tileset's comments */

		for (i = 0; i < 2; i ++) {
			MEM_free (tileset->Comment[i]);
		}

#if PIXELSELECT
		/* de-allocated tile span arrays */

		MEM_free(tileset->pdxLeft);
		MEM_free(tileset->pdxRight);
		MEM_free(tileset->pdyTop);
		MEM_free(tileset->pdyBottom);
#endif // PIXELSELECT

		/******************************************/
		/* lastly, de-allocate the tileset itself */

		MEM_free (tileset);
	}

	return;

} /* UnLoadTileSet */


/**************************************************************************
**
** LoadTileSource (MACRO)
**
** SYNOPSIS
**	void *LoadTileSource (char *filename)
**
** USAGE
**	TileSourcePtr = LoadTileSource (FILENAME_PTR);
**
** FUNCTION
**	Load the tile images from disk.
**
** INPUTS
**	FILENAME_PTR : A pointer to the file name.
**
** RESULTS
**	Returns a pointer to the newly loaded tilesource, NULL otherwise.
**
** METHOD
**
**
** BUGS
**	NOT TESTED.
**
** HISTORY
**
**	08/22/89 Tuesday - Created. (RGM)
**
** SEE ALSO
**
*/



/**************************************************************************
 *
 * GetTileSourceStats
 *
 * SYNOPSIS
 *		static BOOL GetTileSourceStats (
 *			UWORD			tileset_flags,
 *			TileSourceType	*tilesource,
 *			short			*tilesource_width,
 *			short			*tilesource_height,
 *			short			*depth,
 *			short			*transparent_color,
 *			GridInfoType	*grid_info
 *		)
 *
 * PURPOSE
 *		Fills in the stats for a newly loaded tile source.
 *
 * USAGE
 *		GetTileSourceStats (TILESET_FLAGS, &TILESOURCE,
 *			&TILESOURCE_WIDTH, &TILESOURCE_HEIGHT,
 *			&DEPTH, &GRID_INFORMATION);
 *
 * INPUT
 *
 *
 * EFFECTS
 *		NOT FULLY TESTED.  Might not work if first tile not found
 *		before MIN(maxY, maxX).
 *
 *
 * RETURN VALUE
 *	Changes the values of tileset-WIDTH-HEIGHT and tile-X-Y-WIDTH-HEIGHT-GAP
 *	to the appropriate values.  Returns TRUE if valid values,
 *	FALSE otherwise.
 *
 * HISTORY
 *	08/22/89 Tuesday   - Created. (RGM)
 *
 *	08/28/89 Monday    - First completion. (RGM)
 *
 *	09/03/89 Sunday    - For reals debugging. (RGM)
 *
 *	10/25/89 Wednesday - Re-did scanner for transparencies. (RGM)
 *
 *	11/27/89 Monday    - Fixed to work with single tile tilesets. (RGM)
 *
 *	03/06/91 Wednesday (RGM) - Changed to work with new tile flags and with
 *		the GridInfo structure (sp?).  No we can make things generic by
 *		getting methods from the 'tileset_flags' and putting the info in
 *		the 'grid_info' structure.
 *
 *	03/08/91 Friday (RGM) - Added semi-check of already current values in
 *		the 'grid_info' structure..
 *
 * 04/07/92 Tuesday (dcc) - be sure pixel is in range b4 calling ReadPixel().
 *
 * SEE ALSO
 *
*/

static BOOL GetTileSourceStats (
	UWORD			tileset_flags,
	TileSourceType	*tilesource,
	short			*tilesource_width,
	short			*tilesource_height,
	short			*depth,
	short			*transparent_color,
	GridInfoType	*grid_info
)
{
			UWORD		grid_type_mod;
			UWORD		grid_start_mod;
			UWORD		grid_tiledim_mod;
			UWORD		grid_offset_mod;

#if __AMIGAOS__
	struct	RastPort	*rp;
#elif __MSDOS__
	BigByteMap	*rp;
#endif/*__AMIGAOS__/__MSDOS__*/
			BOOL		status	= TRUE;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetTileSourceStats";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	grid_type_mod	= (tileset_flags & GRID_TYPE_MODIFIER);
	grid_start_mod	= (tileset_flags & GRID_START_MODIFIER);
	grid_tiledim_mod= (tileset_flags & GRID_TILEDIMENSIONS_MODIFIER);
	grid_offset_mod	= (tileset_flags & GRID_OFFSET_MODIFIER);

	rp = (TileSourceRastPort (tilesource));	/* For ReadPixel() */

	if (rp) {
		short			background_color;
		short			tswidth, tsheight;
		short			max_coordinate;
		short			x, y;

		*tilesource_width	= TileSourceWidth (tilesource);
		*tilesource_height	= TileSourceHeight (tilesource);
#if __AMIGAOS__
		*depth			= PicDepth (tilesource);
#elif __MSDOS__
		*depth			= 0;	/* count planes based on NumberOfColors */
		while ((1 << *depth) < tilesource->NumberOfColors)
			(*depth)++;
#endif/*__AMIGAOS__/__MSDOS__*/
		*transparent_color	= TileSourceTColor (tilesource);

		tswidth				= *tilesource_width;
		tsheight			= *tilesource_height;
		background_color	= *transparent_color;

		max_coordinate		= max(tswidth, tsheight);

		if (grid_type_mod == GRID_IS_BOXED) {

			long	first_color;
			long	bounding_color;
			long	penno;
			short	max;
			short	m;

			/*********************************************************/
			/* Find first bounding pixel (for FIRSTFOUND extraction) */

			first_color	= ReadPixel (rp, 0, 0);
			penno		= background_color;
			m			= 0;
			max			= 0;

			while ( ((penno == background_color) || (penno == first_color))
									&& (max < max_coordinate) ) {
				m = max;
				while ( ((penno == background_color) || (penno == first_color))
												&& (m >= 0) ) {
					penno = ReadPixel (rp, m, max - m);
					if (penno < 0) {
						penno = background_color;
					}
					--m;
				}
				max++;
			}
	
			x = m + 1;					/* adjust to actual */
			y = (max - 1) - (m + 1);	/* image location   */

			grid_info->start_x	= x + 1;
			grid_info->start_y	= y + 1;

			/************************************************/
			/* Now find the tile width and x offset of the  */
			/* next tile                                    */

			bounding_color = penno;

			while (penno == bounding_color) {	/* Find tile width */
				x++;
				penno = ReadPixel (rp, x, y);
			}

			grid_info->image_width	= (x - grid_info->start_x) - 1;

			/*****************/
			/* Find x offset */

			while (penno != bounding_color) {
				x++;
				if (x < tswidth)
					penno = ReadPixel (rp, x, y);
				else
					break;
			}

			if (x < tswidth) {
				grid_info->offset_x	= (x + 1) - grid_info->start_x;
			}
			else {
				grid_info->offset_x = 0;
			}

			/*************************************************/
			/* . . .and the tile height and y offset of the  */
			/* next tile (for FIRSTFOUND extraction). . .    */

			x		= grid_info->start_x - 1;	/* reset x for scan */
			penno	= bounding_color;			/* ditto for penno  */

			while (penno == bounding_color) {	/* Find tile height */
				y++;
				penno = ReadPixel (rp, x, y);
			}

			grid_info->image_height = (y - grid_info->start_y) - 1;

			/*****************/
			/* Find y offset */

			while (penno != bounding_color) {
				y++;
				if (y < tsheight)
					penno = ReadPixel (rp, x, y);
				else
					break;
			}

			if (y < tsheight) {
				grid_info->offset_y = (y+1) - grid_info->start_y;
			}
			else {
				grid_info->offset_y = 0;
			}
		}
		else if (grid_type_mod == GRID_IS_NOT_BOXED){

			if (status) switch (grid_start_mod) {
			case GRID_START_FROM_FIRST_NOTRANSPARENT:
				{
					short	x, y;

					if (PixelScan (	tilesource,
									0, 0,
									-1, -1,
									(CORNER_SCAN | SCAN_UNTIL_NOT),
									background_color,
									&x, &y )){

						grid_info->start_x	= x;
						grid_info->start_y	= y;
					}
					else {
						grid_info->start_x	= 0;
						grid_info->start_y	= 0;
						status = FALSE;
					}
				}
				break;

			case GRID_START_FROM_USER:
			case GRID_START_FROM_TOP:
			default:
				grid_info->start_x	= 0;
				grid_info->start_y	= 0;
				break;

			}

			if (status) switch (grid_tiledim_mod) {
			case GRID_TILEDIMENSIONS_FROM_FIRST_NOTRANSPARENT:
				{
					short	x, y;

					if (PixelScan (
							tilesource,
							grid_info->start_x,
							grid_info->start_y,
							-1,
							-1,
							(EDGE_SCAN | SCAN_UNTIL),
							background_color,
							&x,
							&y)){

						grid_info->image_width	= x - grid_info->start_x;
						grid_info->image_height	= y - grid_info->start_y;
					}
					else {
						grid_info->image_width	= 0;
						grid_info->image_height	= 0;
						status = FALSE;
					}
				}
				break;

			case GRID_TILEDIMENSIONS_FROM_USER:
			default:
#if 0
				if ((!grid_info->image_width)&&(!grid_info->image_height)) {
					int	width, height;

					if	(!	TS_Request (&width, &height)) {
						grid_info->image_width	= 0;
						grid_info->image_height	= 0;
						status = FALSE;
					}
					else {
						grid_info->image_width	= width;
						grid_info->image_height	= height;
					}
				}
#endif
				break;
			}

			if (status) switch (grid_offset_mod) {
			case GRID_OFFSET_FROM_FIRST_NOTRANSPARENT:
				{
					short	x, y;

					BOOL	local_status;

					local_status =
						PixelScan (
							tilesource,
							(short) (grid_info->start_x + grid_info->image_width),
							grid_info->start_y,
							-1, -1,
							(LINE_SCAN | HORIZONTAL_SCAN | SCAN_UNTIL_NOT),
							background_color,
							&x,&y
						);

					if (local_status) {
						grid_info->offset_x	=  x - grid_info->start_x;
					}
					else {
						grid_info->offset_x	=  0;
					}

					local_status =
						PixelScan (
							tilesource,
							grid_info->start_x,
							(short) (grid_info->start_y + grid_info->image_height),
							-1, -1,
							(LINE_SCAN | VERTICAL_SCAN | SCAN_UNTIL_NOT),
							background_color,
							&x,&y
						);

					if (local_status) {
						grid_info->offset_y	=  y - grid_info->start_y;
					}
					else {
						grid_info->offset_y	=  0;
					}
				}
				break;
			case GRID_OFFSET_FROM_USER:
			case GRID_OFFSET_FROM_TILEDIMENSIONS:
			default:
				grid_info->offset_x	= grid_info->image_width;
				grid_info->offset_y	= grid_info->image_height;
				break;

			}

		}
	}
	else {
		status = FALSE;
	}

	return (status);

} /* GetTileSourceStats */


/**************************************************************************
 *
 * GrabTiles
 *
 * SYNOPSIS
 *		static LONG GrabTiles (
 *			UWORD			tileset_flags,
 *			TileSourceType	*tilesource,
 *			short			tilesource_width,
 *			short			tilesource_height,
 *			short			depth,
 *			short			transparent_color,
 *			GridInfoType	*grid_info,
 *			TileSetType		*newtileset
 *		)
 *
 * PURPOSE
 *		Given a screen full of tiles in <tilesource>, cut them out
 *		and add them to <newtileset->Tiles>.
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
 *		Returns -1 if failed due to out of memory.
 *
 * HISTORY
 *		08/28/89 Monday - Created. (RGM)
 *		03/07/91 Thursday (RGM) - Changed to use tileset_flags & grid_info.
 *		08/17/92 Monday (dcc) - return -1 if FAILED (was return (- tilecount)).
 *
 * SEE ALSO
 *
*/

static LONG GrabTiles (
	UWORD		tileset_flags,
	TileSourceType	*tilesource,
	short		tilesource_width,
	short		tilesource_height,
	short		depth,
	short		transparent_color,
	GridInfoType	*grid_info,
	TileSetType	*newtileset
)
{
	short		current_x, current_y;
	LONG			border_color;
#if __AMIGAOS__
	int			rassize;
	short		i;
#endif/*__AMIGAOS__*/
	BOOL			more;

	BOOL			throw_away_blanks;
//	BOOL			throw_away_repeats;
	BOOL			two_blank_ending;

	UWORD		grid_type_mod;

#if __AMIGAOS__
	UBYTE		**transparent_planes	= NULL;
	TileType		*new_tile				= NULL;
	Shape		*temp_shape			= NULL;
#endif/*__AMIGAOS__*/
	LONG			tilecount				= 0;
	WORD			across				= 0;
	WORD			down					= 0;

	WORD			prev_across;
	WORD			prev_down;

#if __MSDOS__
/* OriginalPositionLast == Original_Row * (newtileset->Across+1) + Original_Column */

	LONG			OriginalPositionLastLast = 0;
	LONG			OriginalPositionLast = 0;
	WORD			OriginalCol;
	WORD			OriginalRow;

	UWORD		numColMax;

	UWORD		colLast				= 0xFFFF;
	UWORD		col;

	UWORD		numDelColMax;

	UWORD		delColLast			= 0xFFFF;
	UWORD		delCol;
#endif/*__MSDOS__*/

	TileStatusType	last;
	TileStatusType	this					= {FALSE, FALSE, FALSE, TRUE};

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GrabTiles";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	depth = depth;		/* Turn off warnings */

#if __MSDOS__
	/* Try to allocate memory we need to point to the collections.
	   Figure the most tiles we will possibly collect, and allocate
	   enough collections to store all possible tiles. */

	{
		ULONG numTiles;
		UWORD MaxDown;

		if (grid_info->offset_x)
		{
			newtileset->MaxAcross = (UWORD) ((tilesource_width - grid_info->start_x) / grid_info->offset_x);
			if (((tilesource_width - grid_info->start_x) % grid_info->offset_x)
					> grid_info->image_width)
				newtileset->MaxAcross++;
		}
		else
			newtileset->MaxAcross = 1;

		if (grid_info->offset_y)
		{
			MaxDown = (UWORD) ((tilesource_height - grid_info->start_y) / grid_info->offset_y);
			if (((tilesource_height - grid_info->start_y) % grid_info->offset_y)
					> grid_info->image_height)
				MaxDown++;
		}
		else
			MaxDown = 1;

		numTiles	= (ULONG) newtileset->MaxAcross * MaxDown;
		numColMax = (UWORD) ((numTiles + newtileset->tilesPerCol-1) / newtileset->tilesPerCol);

		/* Must use MEM_calloc() as we check for not as yet allocated XTRA pointers! */

		if ((newtileset->pxtrTilCol = MEM_calloc(numColMax, sizeof(XTRAPntr))) == NULL)
/**/		goto OOM_ABORT;

		numDelColMax = (UWORD) ((numTiles + deltasPerCol-1) / deltasPerCol);

		if ((newtileset->pxtrDelCol = MEM_calloc(numDelColMax, sizeof(XTRAPntr))) == NULL)
/**/		goto OOM_ABORT;
	}
#endif/*__MSDOS__*/

	throw_away_blanks	= (tileset_flags & THROW_AWAY_BLANK_TILES);
//	throw_away_repeats	= (tileset_flags & THROW_AWAY_REPEAT_TILES);
	two_blank_ending	= (tileset_flags & TWO_BLANK_TILE_ENDING);

	grid_type_mod		= (tileset_flags & GRID_TYPE_MODIFIER);


	/**************/
	/* initialize */

	newtileset->Across = newtileset->Down = 0;
	prev_across	= across;
	prev_down	= down;

#if __AMIGAOS__

	/**************************/
	/* make up a 'blank tile' */

	if (throw_away_blanks || two_blank_ending) {
		short	index;
		short	backbits	= transparent_color;

		transparent_planes	= newtileset->SharedShape->Pic->BitMap.Planes;

		for (index = 0; index < depth; index ++) {
			if (backbits & 0x01) {
				transparent_planes[index]	= newtileset->FullPlane;
			}
			else {
				transparent_planes[index]	= newtileset->EmptyPlane;
			}
			backbits	= backbits >> 1;
		}
	}
#endif/*__AMIGAOS__*/

	current_x = grid_info->start_x;
	current_y = grid_info->start_y;

	if (grid_type_mod == GRID_IS_BOXED) {

		border_color =	ReadPixel (
							TileSourceRastPort(tilesource),
							(int) (current_x - 1),
							(int) (current_y - 1)
						);
	}

#if __AMIGAOS__
	rassize = RASSIZE (grid_info->image_width, grid_info->image_height);
#endif/*__AMIGAOS__*/

	more = TRUE;

	/******************************/
	/* while more tiles, get them */

#if __MSDOS__
	PushClipValues(0, 0, 32767, 32767);
#endif/*__MSDOS__*/

	while (more) {
		short	box_pixel;
		BOOL	force_newline	= FALSE;

		/**********************************************/
		/* see if there is actually a tile image here */

		if (grid_type_mod == GRID_IS_BOXED) {
			box_pixel	= 	ReadPixel (
								TileSourceRastPort(tilesource),
								(int) (current_x - 1),
								(int) (current_y - 1)
							);
		}

		if (more &&
		((grid_type_mod != GRID_IS_BOXED) || (box_pixel == border_color))) {
#if __MSDOS__
			ByteMap bm;
			UBYTE *pubCol, *pubDelCol;
#endif/*__MSDOS__*/

#if __AMIGAOS__
			/*****************************************************/
			/* get the tile image at current_x and current_y and */
			/* put it in the tileset                             */

			if (! (new_tile = pTilNew())) {
/**/			goto OOM_ABORT;
			}

			/***********************************************/
			/* make a BitMap allocation for the tile image */

			if (! (new_tile->MaskPlanes[0] = malloc_chip (rassize))) {
/**/			goto OOM_ABORT;
			}

			for (i = 0; i < depth; i++) {
				if (! (new_tile->ImageBitMap.Planes[i] =
												malloc_chip (rassize))) {
/**/				goto OOM_ABORT;
				}
				new_tile->MaskPlanes[i] = new_tile->MaskPlanes[0];
			}
#elif __MSDOS__
			/**********************************************/
			/* If current tile collection is full, make a */
			/* new collection to store the tile image     */

			col = (UWORD) (tilecount / newtileset->tilesPerCol);

			if (colLast != col)			/* Working with different collection? */
			{
				if (colLast != 0xFFFF)	/* First collection? */
				{
					/* No, so update last collection */

					UpdateXTRA(newtileset->pxtrTilCol[colLast]);
				}
				/* It is possible that we are going back to a previous
				   collection, as somewhere down below we actually
				   decrement tilecount. Therefore, make sure the
				   collection doesn't already exists b4 we allocate
				   a new collection. */

				if (!newtileset->pxtrTilCol[col])
				{
					if ((newtileset->pxtrTilCol[col] =
						AllocXTRA(newtileset->sizeofTile * newtileset->tilesPerCol)) == NULL)
					{
/**/					goto OOM_ABORT;
					}
				}
				pubCol = (UBYTE *) ActivateXTRAatWin(newtileset->pxtrTilCol[col], winDst);
				colLast = col;
			}

			/***********************************************/
			/* If current delta collection is full, make a */
			/* new collection to store the tile image      */

			delCol = (UWORD) (tilecount / deltasPerCol);

			if (delColLast != delCol)	/* Add new collection? */
			{
				if (delColLast != 0xFFFF)	/* First collection? */
				{
					/* No, so update last collection */

					UpdateXTRA(newtileset->pxtrDelCol[delColLast]);
				}
				if (!newtileset->pxtrDelCol[delCol])
				{
					if ((newtileset->pxtrDelCol[delCol] =
						AllocXTRA(sizeofDeltasCollection)) == NULL)
					{
/**/					goto OOM_ABORT;
					}
				}
				pubDelCol = (UBYTE *) ActivateXTRAatWin(newtileset->pxtrDelCol[delCol], winSrc);
				delColLast = delCol;
			}

#endif/*__AMIGAOS__/__MSDOS__*/

#if __AMIGAOS__
			/************************************************/
			/* make a temporary shape out of the tile image */

			if (! (temp_shape =
					MakeShape (TileSourceRastPort(tilesource),
						(int) current_x,
						(int) current_y,
						(int) grid_info->image_width,
						(int) grid_info->image_height,
						(int) TileSourceColors (tilesource),
						(void *) &TileSourceColorMap (tilesource),
						(int) TileSourceTColor (tilesource)))) {
/**/				goto OOM_ABORT;
			}

			/******************/
			/* copy the image */

			memcpy (new_tile->MaskPlanes[0],
				temp_shape->MaskBm.Planes[0], rassize);

			for (i = 0; i < depth; i++) {
				memcpy (new_tile->ImageBitMap.Planes[i],
					temp_shape->Pic->BitMap.Planes[i], rassize);

			}


			new_tile->ImageBitMap.BytesPerRow =
				temp_shape->Pic->BitMap.BytesPerRow;

			new_tile->ImageBitMap.Rows =
				temp_shape->Pic->BitMap.Rows;

			new_tile->ImageBitMap.Flags =
				temp_shape->Pic->BitMap.Flags;

			new_tile->ImageBitMap.Depth =
				temp_shape->Pic->BitMap.Depth;

			/********************/
			/* deallocate shape */

			if (temp_shape) {
				DeleteShape (temp_shape);
			}
			temp_shape = NULL;
#elif __MSDOS__
			/******************/
			/* copy the image */

			bm.width	= grid_info->image_width;
			bm.height	= grid_info->image_height;
			bm.data	= pubCol + (UWORD) (tilecount % newtileset->tilesPerCol)
								* newtileset->sizeofTile;
			bm.transparentColor = transparent_color;

			CopyBigRectToSmall(TileSourceRastPort(tilesource), current_x, current_y,
					&bm, 0, 0,
					grid_info->image_width, grid_info->image_height);

#endif/*__AMIGAOS__/__MSDOS__*/

			/************************************************/
			/* Ahahahahahahahaha!  This is the new stuff.   */
			/* We want to check the tile flags and see if   */
			/* we get to keep this tile. . .and also see if */
			/* we should stop scanning tiles.               */

			last			= this;

			this.be_first	=	(current_x == grid_info->start_x);
#if __AMIGAOS__
			this.be_blank	=	BitplanesAreSame (
									new_tile->ImageBitMap.Planes,
									transparent_planes,
									grid_info->image_width,
									grid_info->image_height,
									(UBYTE) depth
								);
#elif __MSDOS__
			this.be_blank	=	isByteMapTransparent(&bm);
#endif/*__AMIGAOS__/__MSDOS__*/
			this.be_repeat	=	FALSE;
			this.be_gone	=	FALSE;


			if (two_blank_ending && this.be_blank && last.be_blank) {
				if (! last.be_gone) {
#if __AMIGAOS__
					TileType	*temp_tile;

					temp_tile = RemTail (&newtileset->Tiles);
					if (temp_tile) {
						DeAllocateTile (tileset, temp_tile);
						tilecount--;
						OriginalPositionLast = OriginalPositionLastLast;
					}
#elif __MSDOS__
					if (tilecount) {
						tilecount--;
						OriginalPositionLast = OriginalPositionLastLast;
					}
#endif/*__AMIGAOS__/__MSDOS__*/
					across		= prev_across;
					down			= prev_down;
					last.be_gone	= TRUE;
				}
				if (! this.be_gone) {
#if __AMIGAOS__
					DeAllocateTile (tileset, new_tile);
#endif/*__AMIGAOS__*/
					this.be_gone	= TRUE;
				}
				if (last.be_first) {
					more			= FALSE;
				}
				else {
					force_newline	= TRUE;
				}
			}


			if (throw_away_blanks && this.be_blank && (! this.be_gone)) {
#if __AMIGAOS__
				DeAllocateTile (tileset, new_tile);
#endif/*__AMIGAOS__*/
				this.be_gone	= TRUE;
			}


			/******************************************/
			/* throw_away_repeats handling goes here */


			if (! this.be_gone) {
				if (grid_info->offset_x) {
#if __AMIGAOS__
					new_tile->Original_Column =
#elif __MSDOS__
					OriginalCol =
#endif/*__AMIGAOS__/__MSDOS__*/
						(current_x - grid_info->start_x) / grid_info->offset_x;
				}
				else {
#if __AMIGAOS__
					new_tile->Original_Column = 0;
#elif __MSDOS__
					OriginalCol = 0;
#endif/*__AMIGAOS__/__MSDOS__*/
				}

				if (grid_info->offset_y) {
#if __AMIGAOS__
					new_tile->Original_Row =
#elif __MSDOS__
					OriginalRow =
#endif/*__AMIGAOS__/__MSDOS__*/
						(current_y - grid_info->start_y) / grid_info->offset_y;
				}
				else {
#if __AMIGAOS__
					new_tile->Original_Row = 0;
#elif __MSDOS__
					OriginalRow = 0;
#endif/*__AMIGAOS__/__MSDOS__*/
				}
#if __MSDOS__
				{
					LONG Position = (LONG) OriginalRow * newtileset->MaxAcross + OriginalCol;
					WORD w = (WORD) (Position - OriginalPositionLast);

//					new_tile->DeltaOriginalPos = Position - OriginalPositionLast;
					memcpy(pubDelCol + ((UWORD) (tilecount % deltasPerCol) * sizeof(WORD)),
						&w, sizeof(WORD));
					OriginalPositionLastLast = OriginalPositionLast;
					OriginalPositionLast = Position;
				}
#endif/*__MSDOS__*/

				tilecount++;
#if __AMIGAOS__
				new_tile->TileNumber = tilecount;

				AddTail (&(newtileset->Tiles), new_tile);
#endif/*__AMIGAOS__*/

				prev_across	= across;
				prev_down	= down;

				across	= max (across, current_x);
				down	= max (down, current_y);
			}

#if __AMIGAOS__
			new_tile = NULL;
#endif/*__AMIGAOS__*/
		}

		/**************************************/
		/* move on to the next potential tile */

		if (more) {
			current_x += grid_info->offset_x;

			if (
				((current_x + grid_info->image_width) > tilesource_width)
				|| (force_newline)
				|| (! grid_info->offset_x)) {

				force_newline	= FALSE;
				this.be_blank	= FALSE;

				current_x	= grid_info->start_x;
				current_y	+= grid_info->offset_y;

				if (((current_y + grid_info->image_height) > tilesource_height)
				|| (! grid_info->offset_y)) {
					more = FALSE;
				}
			}
		}
	}

#if __MSDOS__
	PopClipValues();
#endif/*__MSDOS__*/

	if (across) {
		if (grid_info->offset_x) {
			newtileset->Across	= (across - grid_info->start_x) / grid_info->offset_x;
		}
		else {
			newtileset->Across	= 0;
		}
	}
	if (down) {
		if (grid_info->offset_y) {
			newtileset->Down	= (down - grid_info->start_y) / grid_info->offset_y;
		}
		else {
			newtileset->Down	= 0;
		}
	}

#if __MSDOS__
	/* Update last tile collection */

	UpdateXTRA(newtileset->pxtrTilCol[colLast]);

	/* If we allocated too many collections, then shrink number of collections */

	if (++colLast < numColMax)
		newtileset->pxtrTilCol = MEM_realloc(newtileset->pxtrTilCol,
								colLast * sizeof(XTRAPntr));

	newtileset->numTilCols = colLast;

	/* Update last delta collection */

	UpdateXTRA(newtileset->pxtrDelCol[delColLast]);

	/* If we allocated too many collections, then shrink number of collections */

	if (++delColLast < numDelColMax)
		newtileset->pxtrDelCol = MEM_realloc(newtileset->pxtrDelCol,
								delColLast * sizeof(XTRAPntr));

	newtileset->numDelCols = delColLast;

#endif/*__MSDOS__*/

	return (tilecount);

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
OOM_ABORT:

	/*********************/
	/* clean up the mess */

	if (colLast != 0xFFFF)	/* First collection? */
	{
		/* No, so update last collection */

		UpdateXTRA(newtileset->pxtrTilCol[colLast]);
	}

	if (delColLast != 0xFFFF)	/* First collection? */
	{
		/* No, so update last collection */

		UpdateXTRA(newtileset->pxtrDelCol[delColLast]);
	}
#if __AMIGAOS__
	if (temp_shape) {
		DeleteShape (temp_shape);
	}
#endif/*__AMIGAOS__*/

#if __AMIGAOS__
	DeAllocateTile (tileset, new_tile);
#elif __MSDOS__
	newtileset->numTilCols = colLast+1;
#endif/*__AMIGAOS__/__MSDOS__*/

	return -1L;

} /* GrabTiles */


/**************************************************************************
 *
 * LoadTileSet
 *
 * SYNOPSIS
 *		TileSetType *LoadTileSet (
 *			char	*filename,
 *			UWORD	tileset_flags
 *		)
 *
 * PURPOSE
 *	Creates and initializes a new tileset, loads in the tiles from the given
 *	file, and returns a pointer to the tileset.
 *
 * USAGE
 *		TileSetPtr = LoadTileSet (FILENAME_PTR, LOADING_FLAGS);
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns a pointer to the newly loaded tileset, NULL otherwise.
 *
 * HISTORY
 *	08/22/89 Tuesday - Newly created.  Now isn't that special? (RGM)
 *
 *	03/06/91 Wednesday (RGM) - Consolidated loading all different tiles
 *		into one LoadTileSet (as it should have been in the first place!)
 *		To load different formats, we just pass the flags to
 *		ForceLoadTileSet..
 *
 * SEE ALSO
 *
*/
TileSetType *LoadTileSet (
	char	*filename,
	UWORD	tileset_flags

)
{
	char		*realpath;
	char		*fname1;
	char		*fname2;
	char		*outfname;

	int			notfound;
	int			userin;

	TileSetType	*temptileset	= NULL;
	TileSetType	*newtileset	= NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "LoadTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	realpath	= MEM_calloc (MAX_PATH, 1);
	fname1	= MEM_calloc (MAX_FNAME, 1);
	fname2	= MEM_calloc (MAX_FNAME, 1);
	outfname	= MEM_calloc (MAX_FNAME + 1, 1);

	if (! (realpath && fname1 && fname2 && outfname)) {
		goto OOM_ABORT;
	}

	notfound = TRUE;

	/**********************************************/
	/* see if we already have this tileset loaded */

	if (GlobalTileSpace) {
		if (! FullPath (filename, realpath, MAX_PATH)) {
			strcpy (realpath, filename);
		}
		NameAndExt (realpath, fname1, MAX_FNAME);

		temptileset = Head (&(GlobalTileSpace->TileSets));
		while ( (! IsEOList (temptileset)) && (notfound)) {

			NameAndExt (temptileset->Filespec, fname2, MAX_FNAME);

			notfound = stricmp (fname2, fname1);

			if (! notfound) {
				userin = DoThis2 ("Load Tileset", BaseName(fname1),
				"\tis already loaded.\tLoad as a new tileset?");
/**/				if (userin == NO) goto USER_ABORT;
/**/				if (userin == ERROR) goto OOM_ABORT;
			}
			temptileset = Next (temptileset);
		}
	}

	/**********************/
	/* load the new tiles */

	newtileset = ForceLoadTileSet (filename, tileset_flags, NULL, TRUE);

/**/if (! newtileset) goto USER_ABORT;


	SAFEFREE (outfname);
	SAFEFREE (fname2);
	SAFEFREE (fname1);
	SAFEFREE (realpath);

	return (newtileset);

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
OOM_ABORT:
	TellUser (OOM, "Trouble loading tileset.");

USER_ABORT:
	/*********************/
	/* clean up the mess */

	if (newtileset) {
		UnLoadTileSet (newtileset);
	}

	SAFEFREE (outfname);
	SAFEFREE (fname2);
	SAFEFREE (fname1);
	SAFEFREE (realpath);

	return (NULL);

} /* LoadTileSet */


#if PIXELSELECT
/*********************************************************************
 *
 * InitTilePixelScan
 *
 * PURPOSE
 *		Initialize the pixel span arrays <pdxLeft>, <pdxRight>
 *		<pdyTop>, and <pdyBottom>.
 *
 * INPUT
 *		cx		: pixel width of each tile
 *		cy		: pixel height of each tile
 *		pdxLeft	: array of left values to initialize
 *		pdxRight	: array of right values to initialize
 *		pdyTop	: array of top values to initialize
 *		pdyBottom	: array of bottom values to initialize
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		12/22/93 Wednesday (dcc) - created.
 *
*/
static void InitTilePixelScan(int cx, int cy, int *pdxLeft, int *pdxRight,
										int *pdyTop, int *pdyBottom)
{
	int i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitTilePixelScan";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (i = 0; i < cy; i++)
	{
		*(pdxLeft+i)  = cx;
		*(pdxRight+i) = -1;
	}
	for (i = 0; i < cx; i++)
	{
		*(pdyTop+i)  = cy;
		*(pdyBottom+i) = -1;
	}
} /* InitTilePixelScan */


/*********************************************************************
 *
 * UpdateTilePixelScan
 *
 * PURPOSE
 *		Compute the pixel span for tile image contained in <pbm>,
 *		and return results in array <pdxLeft>, <pdxRight>,
 *		<pdyTop>, and <pdyBottom>.
 *
 * INPUT
 *		pbm		: bytemap of tile image to scan
 *		pdxLeft	: array of left values to update
 *		pdxRight	: array of right values to update
 *		pdyTop	: array of top values to update
 *		pdyBottom	: array of bottom values to update
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		12/22/93 Wednesday (dcc) - created.
 *
*/
static void UpdateTilePixelScan(ByteMap *pbm, int *pdxLeft, int *pdxRight,
										int *pdyTop, int *pdyBottom)
{
	int ix, iy;
	UBYTE *ppix;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateTilePixelScan";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ppix = pbm->data;

	for (iy = 0; iy < pbm->height; iy++)
	{
		for (ix = 0; ix < pbm->width; ix++)
		{
			if (*ppix != pbm->transparentColor)
			{
				if (ix < *(pdxLeft+iy))
					*(pdxLeft+iy) = ix;
				if (ix > *(pdxRight+iy))
					*(pdxRight+iy) = ix;

				if (iy < *(pdyTop+ix))
					*(pdyTop+ix) = iy;
				if (iy > *(pdyBottom+ix))
					*(pdyBottom+ix) = iy;
			}
			ppix++;
		}
	}
} /* UpdateTilePixelScan */


/*********************************************************************
 *
 * ComputeTSTPixelSpan
 *
 * PURPOSE
 *		Compute how many pixels are occupied on each pixel line of
 *		the tile. Currently, we only compute these values only once
 *		for every tile in the tileset, and we save the results in
 *		<pdxLeft> and <pdxRight>.
 *
 *		We also compute how many pixels are in each column and
 *		saving the results in <pdyTop> & <pdyBottom>.
 *
 * INPUT
 *		ptst		: tileset to compute pixel span
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		FALSE if OOM.
 *
 * HISTORY
 *		12/21/93 Tuesday (dcc) - created.
 *
*/
static BOOL ComputeTSTPixelSpan(TileSetType *ptst)
{
	int i;
	ByteMap bm;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ComputeTSTPixelSpan";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((ptst->pdxLeft	= MEM_calloc(ptst->Height, sizeof(int))) == NULL)
		return FALSE;

	if ((ptst->pdxRight	= MEM_calloc(ptst->Height, sizeof(int))) == NULL)
		return FALSE;

	if ((ptst->pdyTop	= MEM_calloc(ptst->Width, sizeof(int))) == NULL)
		return FALSE;

	if ((ptst->pdyBottom= MEM_calloc(ptst->Width, sizeof(int))) == NULL)
		return FALSE;

	bm.width  = ptst->Width;
	bm.height = ptst->Height;
	bm.transparentColor = ptst->SharedtransparentColor;

	InitTilePixelScan(bm.width, bm.height, ptst->pdxLeft, ptst->pdxRight,
									ptst->pdyTop, ptst->pdyBottom);

	for (i = 0; i < ptst->TileCount; i++)
	{
		unsigned int col, idx;

		col = i / ptst->tilesPerCol;
		idx = i % ptst->tilesPerCol;

		bm.data = (UBYTE *) ActivateXTRAatWin(ptst->pxtrTilCol[col], winTil)
									+ idx * ptst->sizeofTile;
		UpdateTilePixelScan(&bm, ptst->pdxLeft, ptst->pdxRight,
							ptst->pdyTop, ptst->pdyBottom);
		ReleaseXTRA(ptst->pxtrTilCol[col]);
	}
	return TRUE;
} /* ComputeTSTPixelSpan */


/*********************************************************************
 *
 * ComputeTSTOutlines
 *
 * PURPOSE
 *		Compute the tileset outline. This routine computes a list of
 *		points that draw an outline around the perimeter of the tile
 *		when followed. This routine computes then marks the points
 *		on this list to draw the following segments around the tile:
 *		TL, T, TR, R, RB, B, LB, L.
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
 *		01/10/94 Monday (dcc) - created.
 *
*/
static void ComputeTSTOutlines(TileSetType *ptst)
{
	int i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ComputeTSTOutlines";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* 	for left line, follow outline from
		(pdxLeft[dyL0], dyL0) to (pdxLeft[dyL1], dyL1): */

	ptst->dyL0 = 0;				ptst->dyL1 = ptst->Height-1;

	for (i = 0; i < ptst->Height; i++)
	{
		if (ptst->pdxLeft[i] < ptst->Width)
		{
			ptst->dyL0 = i;
			break;
		}
	}
	for (i = ptst->Height-1; i >= 0; i--)
	{
		if (ptst->pdxLeft[i] < ptst->Width)
		{
			ptst->dyL1 = i;
			break;
		}
	}

	/* 	for right line, follow outline from
		(pdxRight[dyR0], dyR0) to (pdxRight[dyR1], dyR1): */

	ptst->dyR0 = 0;				ptst->dyR1 = ptst->Height-1;

	for (i = 0; i < ptst->Height; i++)
	{
		if (ptst->pdxRight[i] > -1)
		{
			ptst->dyR0 = i;
			break;
		}
	}
	for (i = ptst->Height-1; i >= 0; i--)
	{
		if (ptst->pdxRight[i] > -1)
		{
			ptst->dyR1 = i;
			break;
		}
	}

	/*	for top line, follow outline from
		(dxT0, pdyTop[dxT0]) to (dxT1, pdyTop[dxT1]): */

	ptst->dxT0 = 0;				ptst->dxT1 = ptst->Width-1;

	for (i = 0; i < ptst->Width; i++)
	{
		if (ptst->pdyTop[i] < ptst->Height)
		{
			ptst->dxT0 = i;
			break;
		}
	}
	for (i = ptst->Width-1; i >= 0; i--)
	{
		if (ptst->pdyTop[i] < ptst->Height)
		{
			ptst->dxT1 = i;
			break;
		}
	}

	/*	for bottom line, follow outline from
		(dxB0, pdyTop[dxB0]) to (dxB1, pdyTop[dxB1]): */

	ptst->dxB0 = 0;				ptst->dxB1 = ptst->Width-1;

	for (i = 0; i < ptst->Width; i++)
	{
		if (ptst->pdyBottom[i] > -1)
		{
			ptst->dxB0 = i;
			break;
		}
	}
	for (i = ptst->Width-1; i >= 0; i--)
	{
		if (ptst->pdyBottom[i] > -1)
		{
			ptst->dxB1 = i;
			break;
		}
	}
} /* ComputeTSTOutlines */
#endif // PIXELSELECT


/**************************************************************************
 *
 * ForceLoadTileSet
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
 *		03/08/91 (RGM)	- Added use of original_grid_info.
 *		04/01/93 (dcc)	- added FNF_ABORT.
 *		08/25/93 (dcc) - show entire path instead of basename.
#if PIXELSELECT
 *		12/21/93 (dcc) - add call to ComputeTSTPixelSpan()
#endif // PIXELSELECT
 *		01/03/95 (dcc) - Loading a BOXED picture causes tUME to crash with
 *					  a divide by zero error. This is because when 
 *					  GetTilesetInfoReq() is called, newtileset->Width
 *					  is still not filled out. We fix this bug by adding
 *					  a call to LookupSomeTSTInfoFromUserType() after
 *					  newtileset->Width has been filled out.
 *
 * SEE ALSO
 *
*/
TileSetType *ForceLoadTileSet (
	char			*filename,
	UWORD		 tileset_flags,
	GridInfoType	*grid_info,
	BOOL			 fGetTSTTypeFromUser
)
{
#if __AMIGAOS__
	TileType		*tile;
#endif/*__AMIGAOS__*/
	char			*realpath;
	LONG			tilecount;

	short 			tilesource_width, tilesource_height;
	short 			depth;
	short 			transparent_color;
#if __AMIGAOS__
	short 			i;

	void				*old_pointer;
#endif/*__AMIGAOS__*/

	GridInfoType	new_grid_info		= {0, 0, 0, 0, 0, 0};

	TileSourceType 	*tilesource_ptr	= NULL;
	TileSetType		*newtileset		= NULL;
	char			*newfilename	= NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ForceLoadTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	old_pointer		= GlobalPointer;
#endif/*__AMIGAOS__*/

	realpath		= MEM_calloc (MAX_PATH, 1);

	if (! (realpath)) {
/**/	goto OOM_ABORT;
	}

	/******************************/
	/* get us a name to work with */

	if (! FullPath (filename, realpath, MAX_PATH)) {
		strcpy (realpath, filename);
	}

	if (!EIO_FileExists(realpath))
	{
/**/	goto FNF_ABORT;
	}

	/***********************/
	/* show a busy pointer */

#if __AMIGAOS__
	GlobalPointer = RGM_SetPointer (&YieldMPointer, MainWindow);
#elif __MSDOS__
	DCC_TempSetPointer (BPI_WAIT_POINTER);
#endif/*__AMIGAOS__/__MSDOS__*/

	ShowStatus (realpath);			//	ShowStatus (BaseName (realpath));

	/*************************************************/
	/* Open the tile-file and attempt to load it in. */

	ClearGlobalError ();
	if ( (tilesource_ptr = LoadTileSource (realpath)) == NULL)
	{
		if (GlobalErr == ERR_OUT_OF_MEMORY)
/**/		goto OOM_ABORT;
		else
/**/		goto ABORT;
	}

#if __MSDOS__
	tilesource_ptr->Pixels.BigByteMap.transparentColor = tilesource_ptr->BM.transparentColor;
#endif/*__MSDOS__*/

	/************************************************/
	/* Find upper left, width, height, and spacing. */

	if (! grid_info) {
		grid_info	= &new_grid_info;
	}

	/**************************/
	/* Initilaize the TileSet */

	if ( (newtileset = MEM_calloc (1, TILESETSIZE)) == NULL)
/**/		goto OOM_ABORT;

	InitList (&(newtileset->Tiles));	/* Initialize tile list */

	if ( (newfilename = MEM_calloc (1, strlen (realpath) + 1)) == NULL)
/**/	goto OOM_ABORT;

	strcpy (newfilename, realpath);
	newtileset->Filespec = newfilename;

#if __MSDOS__
	newtileset->SharedtransparentColor = tilesource_ptr->BM.transparentColor;
	if ((newtileset->cinShared = CreateColorInfo()) == NULL)
/**/	goto OOM_ABORT;
//	newtileset->SharedNumberColors = TileSourceColors(tilesource_ptr);
	{
		short i;
		CS_Color	 rgcsc[MAX_CI_COLORS];

		ReadXTRA(newtileset->cinShared->pxtColors, 0,
				MAX_CI_COLORS*sizeof(CS_Color), rgcsc);

		for (i = 0; i < TileSourceColors(tilesource_ptr); i++)
		{
			rgcsc[i].Red = tilesource_ptr->Colors[i].red;
			rgcsc[i].Green = tilesource_ptr->Colors[i].green;
			rgcsc[i].Blue = tilesource_ptr->Colors[i].blue;
			rgcsc[i].Hue = 0;
			ConvertRGBtoHSV (&rgcsc[i]);
		}
		WriteXTRA(rgcsc, MAX_CI_COLORS*sizeof(CS_Color),
				newtileset->cinShared->pxtColors, 0);
	}
#endif/*__MSDOS__*/

	if (fGetTSTTypeFromUser)
	{
		if (	(tileset_flags & GRID_TYPE_MODIFIER) == GRID_IS_NOT_BOXED &&
			(tileset_flags & GRID_START_MODIFIER) == GRID_TILEDIMENSIONS_FROM_USER)
		{
#if __MSDOS__
			/* Stamp source tile LBM so user can see it. */

			BeforeGraphics();
			SetDisplayPalette(ActivateXTRA(newtileset->cinShared->pxtColors),
							newtileset->cinShared->NumColors);
			ReleaseXTRA(newtileset->cinShared->pxtColors);
			CopyBigRectToSmall(TileSourceRastPort(tilesource_ptr), 0, 0,
				DisplayBM, 0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd);
			AfterGraphics();
#endif/*__MSDOS__*/

			ClearGlobalError ();
			if (!GetTilesetInfoReq (newtileset, TRUE))
			{
				if (GlobalErr == ERR_OUT_OF_MEMORY)
/**/				goto OOM_ABORT;
				else
/**/				goto USER_ABORT;
			}

			/* Set DONT_SAVEIMAGES in tileset_flags. */

			tileset_flags |= newtileset->Flags & DONT_SAVEIMAGES;

			/* Set width & height so GetTileSourceStats doesn't ask for them. */

			grid_info->image_width	= newtileset->Width;
			grid_info->image_height	= newtileset->Height;
		}
		else
		{
#if __MSDOS__
			/* Stamp source tile LBM so user can see it. */

			BeforeGraphics();
			SetDisplayPalette(ActivateXTRA(newtileset->cinShared->pxtColors),
							newtileset->cinShared->NumColors);
			ReleaseXTRA(newtileset->cinShared->pxtColors);
			CopyBigRectToSmall(TileSourceRastPort(tilesource_ptr), 0, 0,
				DisplayBM, 0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd);
			AfterGraphics();
#endif/*__MSDOS__*/

			ClearGlobalError ();
			if (!GetTilesetInfoReq (newtileset, FALSE))
				if (GlobalErr == ERR_OUT_OF_MEMORY)
/**/				goto OOM_ABORT;
				else
/**/				goto USER_ABORT;
		}
	}

	if (! (GetTileSourceStats (
				tileset_flags,
				tilesource_ptr,
				&tilesource_width,
			    	&tilesource_height,
				&depth,
				&transparent_color,
		    		grid_info)))
	{
/**/		goto ABORT;
	}

	if ((long) grid_info->image_width * (long) grid_info->image_height > sizeofTileCollection)
	{
		TellUser (NCD, "Tile image is too large.");
/**/	goto USER_ABORT;
	}

	newtileset->Width	= grid_info->image_width;
	newtileset->Height	= grid_info->image_height;
	newtileset->Depth	= depth;

	newtileset->Flags	= tileset_flags;

	LookupSomeTSTInfoFromUserType(newtileset);

#if __MSDOS__
	newtileset->sizeofTile	= grid_info->image_width * grid_info->image_height;

	/* If tile size is less than 16384, than make each collection 16384 bytes
		big, so we only have to page in one EMS/XMS page at a time. */

	if (newtileset->sizeofTile <= 16384)
		newtileset->tilesPerCol	= 16384 / newtileset->sizeofTile;
	else
		newtileset->tilesPerCol	= sizeofTileCollection / newtileset->sizeofTile;
#endif/*__MSDOS__*/

#if __AMIGAOS__
	/*********************************************************/
	/* initialize the shared shape we use to draw tiles with */

	if (! (newtileset->SharedShape =
			MakeShape (TileSourceRastPort(tilesource_ptr),
				(int) grid_info->start_x,
				(int) grid_info->start_y,
				(int) grid_info->image_width,
				(int) grid_info->image_height,
				(int) TileSourceColors (tilesource_ptr),
				(void *) TileSourceColorMap (tilesource_ptr),
				(int) TileSourceTColor (tilesource_ptr)))) {
/**/		goto OOM_ABORT;
	}

	for (i= 0; i < MAXPLANES; i++) {
		newtileset->OriginalImagePlanes[i] =
			newtileset->SharedShape->Pic->BitMap.Planes[i];

		newtileset->OriginalMaskPlanes[i] =
			newtileset->SharedShape->MaskBm.Planes[i];

	}

	/********************************************/
	/* create and hook in full and empty planes */

	{
		UWORD	image_height	= grid_info->image_height;
		UWORD	image_width		= grid_info->image_width;
		UWORD	mem_needed;

static	const	UWORD	finish_words [] = {
					0xFFFF,

					0x8000,
					0xC000,
					0xE000,
					0xF000,
					0xF800,
					0xFC00,
					0xFE00,

					0xFF00,

					0xFF80,
					0xFFC0,
					0xFFE0,
					0xFFF0,
					0xFFF8,
					0xFFFC,
					0xFFFE
				};

		MyPlanePtr	full_plane;
		MyPlanePtr	empty_plane;

		mem_needed	= RASSIZE(image_width, image_height);
		full_plane	= malloc_chip (mem_needed);
		empty_plane	= calloc_chip (mem_needed, 1);

		if (full_plane) {
			UWORD	remainder_bits;

			memset (full_plane, 0xFF, mem_needed);
			remainder_bits	= image_width % (8 * sizeof (UWORD));

			if (remainder_bits) {
				UWORD	whole_words;
				UWORD	*current_word;
				UWORD	finish_word;
				UWORD	row_words;

				short	index;

				whole_words		= image_width / (8 * sizeof (UWORD));

				row_words		= whole_words + 1;	/* we have remainder */
				finish_word		= finish_words [remainder_bits];

				current_word	= full_plane;
				current_word	+= whole_words;

				for (index = 0; index < image_height; index ++) {
					*current_word	= finish_word;
					current_word	+= row_words;
				}
			}
		}

		newtileset->FullPlane	= full_plane;
		newtileset->EmptyPlane	= empty_plane;
		newtileset->PlaneSize	= mem_needed;
	}
#endif/*__AMIGAOS__*/

	/**********************************/
	/* put the tiles into the tileset */

	tilecount =	GrabTiles (
					tileset_flags,
					tilesource_ptr,
					tilesource_width,
					tilesource_height,
					depth,
					transparent_color,
					grid_info,
					newtileset
				);

	/*****************************/
	/* get rid of the tilesource */

	UnLoadTileSource (tilesource_ptr);
	tilesource_ptr = NULL;
		
	if (tilecount < 0) {
/**/	goto OOM_ABORT;
	}

	/****************************************/
	/* enter the tilecount into the tileset */

	newtileset->TileCount = (UWORD) tilecount; 

#if __AMIGAOS__
	/*******************************/
	/* make the tile pointer array */

	/* newtileset->TilePtr[0]	= NULL; */

	if (! (newtileset->TilePtr =
			MEM_calloc ((tilecount+1), sizeof (TileType *)) ) ) {
/**/		goto OOM_ABORT;
	}

	tile = Head (&newtileset->Tiles);
	while (! IsEOList (tile)) {
		newtileset->TilePtr[tile->TileNumber] = tile;
		tile = Next (tile);
	}
#endif/*__AMIGAOS__*/


	/*******************************/
	/* give the tileset a new ID # */

	newtileset->TS_id	= GetNextTilesetID (GlobalTileSpace);

#if PIXELSELECT
	if(!ComputeTSTPixelSpan(newtileset))
/**/	goto OOM_ABORT;
	ComputeTSTOutlines(newtileset);
#endif // PIXELSELECT

	/*****************************/
	/* restore the mouse pointer */

#if __AMIGAOS__
	GlobalPointer = RGM_SetPointer (old_pointer, MainWindow);
#elif __MSDOS__
	DCC_SetPointer (GlobalPointer);
#endif/*__AMIGAOS__/__MSDOS__*/

	SAFEFREE (realpath);

#if __MSDOS__
	/* Erase the source tile LBM from screen. */

	SetDisplayPalette (ActivateXTRA(GlobalColors->pxtColors), GlobalColors->NumColors);
	ReleaseXTRA(GlobalColors->pxtColors);
	CopyDisplayToBack(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd, BACKTODISPLAY);
#endif/*__MSDOS__*/

	return (newtileset);

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
OOM_ABORT:
	SetGlobalErr (ERR_OUT_OF_MEMORY);
	GEcatf ("ForceLoadTileSet:OOM");
	TellUser (OOM, "Out of memory loading tileset.");
/**/goto USER_ABORT;

FNF_ABORT:
	{
		char sz[256];

		sprintf(sz, "Can't find tileset '%s'.", realpath);
		TellUser (NCD, sz);
/**/	goto USER_ABORT;
	}
ABORT:
	TellUser (NCD, "Trouble loading tileset.");

USER_ABORT:

#if __MSDOS__
	/* Erase the source tile LBM from screen. */

	SetDisplayPalette (ActivateXTRA(GlobalColors->pxtColors), GlobalColors->NumColors);
	ReleaseXTRA(GlobalColors->pxtColors);
	CopyDisplayToBack(0, TopAdd, SCREEN_WIDTH, SCREEN_HEIGHT-TopAdd, BACKTODISPLAY);
#endif/*__MSDOS__*/

	/*********************/
	/* clean up the mess */

	if (tilesource_ptr) {
		UnLoadTileSource (tilesource_ptr);
	}

	if (newtileset) {
		UnLoadTileSet (newtileset);
	}

	/*****************************/
	/* restore the mouse pointer */

#if __AMIGAOS__
	GlobalPointer = RGM_SetPointer (old_pointer, MainWindow);
#elif __MSDOS__
	DCC_SetPointer (GlobalPointer);
#endif/*__AMIGAOS__/__MSDOS__*/

	SAFEFREE (realpath);

	return (NULL);

} /* ForceLoadTileSet */


/**************************************************************************
 *
 * AddTileSet
 *
 * SYNOPSIS
 *		void AddTileSet (TileSetType *tileset, TileSpaceType *tilespace)
 *
 * PURPOSE
 *		To add a tileset to a tilespace, setting all the proper links.
 *
 * USAGE
 *
 *
 * INPUT
 *		NOTE : tileset->TS_id should be set prior to calling.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		01/10/91 Thursday (RGM) - Created.
 *		03/30/94 Wednesday (dcc) - call RevampTileCounts() B4 changing HighestID
 *
 * SEE ALSO
 *
*/
void AddTileSet (TileSetType *tileset, TileSpaceType *tilespace)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	tileset->MyTileSpace	= tilespace;

	RevampTileCounts(tilespace);

	tilespace->HighestID	= max (tileset->TS_id, tilespace->HighestID);

	tilespace->TileSetArray[tileset->TS_id]	= tileset;
	AddTail (&tilespace->TileSets, tileset);

	RevampTileOverhang(tilespace);
	return;

} /* AddTileSet */

/**************************************************************************
 *
 * RemoveTileSet
 *
 * SYNOPSIS
 *		void RemoveTileSet (TileSetType	*tileset)
 *
 * PURPOSE
 *		To remove a tileset from its tilespace.
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
 *	01/10/91 Thursday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
void RemoveTileSet (TileSetType	*tileset)
{

	TileSpaceType	*my_tilespace	= tileset->MyTileSpace;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RemoveTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (my_tilespace) {
		RevampTileCounts(my_tilespace);
		RevampTileOverhang(my_tilespace);

		Remove (tileset);
		my_tilespace->TileSetArray[tileset->TS_id]	= NULL;

		my_tilespace->HighestID	= GetHighestTilesetID (my_tilespace);

		tileset->MyTileSpace	= NULL;
	}

	return;

} /* RemoveTileSet */


/**************************************************************************
 *
 * SaveTile
 *
 * SYNOPSIS
 *		
 *		static BOOL SaveTile (
 *			TileSetType	*tileset,
 *			TileType		*tile,
 *			char			*path
 *		)
 *
 * PURPOSE
 *		Save a tile's image as an IFF ILBM file.
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
 *	03/09/91 Saturday (RGM) - Created out of SaveTileSpace..
 *
 *
 * SEE ALSO
 *
*/

static BOOL SaveTile (
	TileSetType	*tileset,
#if __AMIGAOS__
	TileType		*tile,
#elif __MSDOS__
	UWORD		 TileNumber,
#endif/*__AMIGAOS__/__MSDOS__*/
	char			*path
)
{
	BOOL	status	= FALSE;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	if (tile) {

		MyPlanePtr	*draw_image,  *stored_image;
		MyPlanePtr	*draw_mask, *stored_mask;
		MyPlanePtr	*stop;

		/*****************************************/
		/* put this tile's image into the shared */
		/* Shape/Picture for saving              */

		draw_image		= tileset->SharedShape->Pic->BitMap.Planes;
		draw_mask		= tileset->SharedShape->MaskBm.Planes;
		stored_image	= tile->ImageBitMap.Planes;
		stored_mask		= tile->MaskPlanes;
		stop 			= stored_mask + MAXPLANES;

		for ( ; stored_mask < stop; ) {
			*draw_image++	= *stored_image++;
			*draw_mask++	= *stored_mask++;
		}

		/*****************************************/
		/* Save the now-valid Picture's RastPort */

		status = SaveShape (tileset->SharedShape, path);
	}
#elif __MSDOS__
		{
			ByteMap	bm;

			UWORD col;
			UWORD idx;

			bm.width	= tileset->Width;
			bm.height	= tileset->Height;

			{
			col = (TileNumber-1) / tileset->tilesPerCol;
			idx = (TileNumber-1) % tileset->tilesPerCol;

			bm.data = (UBYTE *) ActivateXTRAatWin(tileset->pxtrTilCol[col], winTil)
										+ idx * tileset->sizeofTile;
			}
			bm.transparentColor = tileset->SharedtransparentColor;
			{
				CS_Color *pcscShared = ActivateXTRAatWin(tileset->cinShared->pxtColors, winDst);

				status = SaveByteMap(&bm,
								tileset->Depth,
								tileset->cinShared->NumColors,
								pcscShared, path);
			}
			ReleaseXTRA(tileset->cinShared->pxtColors);
			ReleaseXTRA(tileset->pxtrTilCol[col]);
		}
#endif/*__AMIGAOS__/__MSDOS__*/

	return (status);

} /* SaveTile */


/**************************************************************************
 *
 * SaveTileSpace
 *
 * PURPOSE
 *		To save out each tile image in a tilespace as an ILBM file.
 *
 * USAGE
 *
 *
 * INPUT
 *		SAVETILES_AS_TTI vs. SAVETILES_AS_NAMED
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		03/09/91 Saturday (RGM) - Added flags, added support for named save..
 *		08/10/93 Tuesday (dcc) - Return TRUE if user CANCELs operation
 *		07/27/94 Wednesday (dcc) - revised to use EIO_FileType() vs FoundDir()
 *		07/27/94 Wednesday (dcc) - revised to use EIO_MakeDir() vs EasyMakeDir()
 *
 * SEE ALSO
 *
*/

BOOL SaveTileSpace (
	TileSpaceType	*tilespace,
	char			*path,
	UWORD		flags
)
{

static	char	tumepack_dir	[EIO_MAXDIR + 1];
static	char	tileset_dir		[EIO_MAXDIR + 1];
static	char	full_tileset_dir[EIO_MAXDIR + 1];
static	char	tile_name		[EIO_MAXFILE + 1];
static	char	full_tile_filespec	[EIO_MAXPATH + 1];
static	char	message			[EIO_MAXPATH + 1];

	TileSetType	*tileset;
#if __AMIGAOS__
	TileType	*tile;
#elif __MSDOS__
	UWORD i;
#endif/*__AMIGAOS__/__MSDOS__*/

	long		tilecount;
	BOOL		first_tile_set	= TRUE;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveTileSpace";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* eg path		= "DH0:SUB.DIR/SUB.MAP" */

	if (flags == SAVETILES_AS_TTI) {

		/**************************************************/
		/* We have 'TTI' tiles, so we need to get or make */
		/* a tumepack directory to hold them.             */

		EIO_fnsplit (
			path,
			ScratchDirName,
			ScratchFileName,
			ScratchExtName
		);

		strcpy (tumepack_dir, ScratchDirName);

						/* eg tumepack_dir	= "DH0:SUB.DIR/" */

		if (EIO_FileType(tumepack_dir) != EIO_TYPE_DIRECTORY)
		{
			if (! EIO_MakeDir (tumepack_dir))
			{
				ShowStatus ("Can't find/make 'tumepack' directory.");
				goto ABORT;
			}
		}
	}

	/***********************************************/
	/* Now we walk through all the tilesets in our */
	/* project and see which ones we want to save  */


	tileset = Head (&tilespace->TileSets);

	while (! IsEOList (tileset)) {

		if (! (tileset->Flags & DONT_SAVEIMAGES)) {

			if (! tileset->Filespec) {
/**/			goto ABORT;
			}

			EIO_fnsplit (
				tileset->Filespec,
				ScratchDirName,
				ScratchFileName,
				ScratchExtName
			);

			if (flags == SAVETILES_AS_TTI) {

				strcpy (tileset_dir, ScratchFileName);
				strcat (tileset_dir, ".TTI");

							/* eg tileset_dir =  "ICELAND.TTI" */
			}
			else if (flags == SAVETILES_AS_NAMED) {

				char	*fname_fr;

				if (first_tile_set) {
					SetFreqDirname (EasyFR[PATH_SAVETILES], ScratchDirName);
				}

				strcpy (message, "TBI for ");
				strcat (message, ScratchFileName);

				SetFreqTitle (EasyFR[PATH_SAVETILES], message);
				SetFreqFilename (EasyFR[PATH_SAVETILES], ScratchFileName);
#if __MSDOS__
				DCC_TempSetPointer (BPI_NORMAL_POINTER);
#endif/*__MSDOS__*/
				fname_fr	= GetFileName (MainWindow, EasyFR[PATH_SAVETILES]);
#if __MSDOS__
				DCC_TempSetPointer (BPI_WAIT_POINTER);
#endif/*__MSDOS__*/
/**/			if (! fname_fr) goto CANCELLED;

						/* eg fname_fr	= "DH0:BRUSHES/DIALS.PAR" */

				EIO_fnsplit (
					fname_fr,
					ScratchDirName,
					ScratchFileName,
					ScratchExtName
				);

				strcpy (tumepack_dir, ScratchDirName);	/* "DH0:BRUSHES/" */

				strcpy (tileset_dir, ScratchFileName);	/* "DIALS"  */
				strcat (tileset_dir, ".TBI");			/* + ".TBI" */

							/* eg tileset_dir =  "DIALS.TBI" */
			}

			strcpy (full_tileset_dir, tumepack_dir);	/* "DH0:SUB.DIR/"  */
			strcat (full_tileset_dir, tileset_dir);		/* + "ICELAND.TTI" */
			strcat (full_tileset_dir, STR_DIRSEP);		/* + "/"           */

					/* eg newtiledir = "DH0:SUB.DIR/ICELAND.TTI/" */
					/* eg newtiledir = "DH0:BRUSHES/DIALS.TTI/" */

			if (EIO_FileType(full_tileset_dir) != EIO_TYPE_DIRECTORY)
			{
				if (! EIO_MakeDir (full_tileset_dir))
				{
					ShowStatus ("Can't find or make tileset directory.");
					goto ABORT;
				}
			}

			tilecount	= 0;
#if __AMIGAOS__
			tile		= Head (&tileset->Tiles);

			/******************************/
			/* Save each tile, one by one */

			while (! IsEOList (tile)) {
#elif __MSDOS__
			for (i = 0; i < tileset->TileCount; i++) {
#endif/*__AMIGAOS__/__MSDOS__*/
				tilecount ++;

				if (flags == SAVETILES_AS_TTI) {
					strcpy (tile_name, TILEFILES);	/* "TILE"         */
				}
				else if (flags == SAVETILES_AS_NAMED) {
					EIO_fnsplit (
						tileset->Filespec,
						ScratchDirName,
						ScratchFileName,
						ScratchExtName
					);

					strcpy (tile_name, ScratchFileName);	/* "<brush name>" */
				}

				sprintf (
					(tile_name + min (8, strlen (tile_name))),
					".%03d\0",
					tilecount					/* + ".xxx" */
				);
							/* eg tile_name = "TILE.xxx" */

				strcpy (full_tile_filespec, full_tileset_dir);
											/* "DH0:SUB.DIR/ICELAND.TTI/" */

				strcat (full_tile_filespec, tile_name);
											/* + "TILE.xxx" */

				/* eg full_tile_filespec="DH0:SUB.DIR/ICELAND.TTI/TILE.xxx"*/

				strcpy (message, "Saving ...");	/* "Saving "  */
				strcat (
					message,
					&full_tile_filespec[max(0,((WORD) strlen(full_tile_filespec)-25))]
				);

				ShowStatus (message);			

#if __AMIGAOS__
				if (! SaveTile (tileset, tile, full_tile_filespec)) {
#elif __MSDOS__
				if (! SaveTile (tileset, i+1, full_tile_filespec)) {
#endif/*__AMIGAOS__/__MSDOS__*/
					ShowStatus ("Error saving tile image.");
/**/					goto ABORT;
				}
#if __AMIGAOS__
				tile = Next (tile);
#endif/*__AMIGAOS__*/

			}

			first_tile_set	= FALSE;

		} /* if */

		tileset = Next (tileset);

	} /* while (! IsEOList (tileset)) */

/*------------------------------------------------------------------------*/
CANCELLED:
	return (TRUE);
/*------------------------------------------------------------------------*/
ABORT:
	return (FALSE);

} /* SaveTileSpace */


/**************************************************************************
 *
 * CreateTileSpace
 *
 * SYNOPSIS
 *		TileSpaceType *CreateTileSpace (void)
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
 *
 *
 * SEE ALSO
 *
*/
TileSpaceType *CreateTileSpace (void)
{
	TileSpaceType *newtilespace = NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateTileSpace";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((newtilespace = MEM_calloc (1, TILESPACESIZE)) != NULL) {
		InitList (&(newtilespace->TileSets));
	}
	return (newtilespace);

} /* CreateTileSpace */


/**************************************************************************
 *
 * DeleteTileSpace
 *
 * SYNOPSIS
 *		void *DeleteTileSpace (TileSpaceType *tilespace)
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
 *
 *
 * SEE ALSO
 *
*/
void *DeleteTileSpace (TileSpaceType *tilespace)
{
	TileSetType *tileset;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteTileSpace";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (tilespace) {
		RevampTileCounts(tilespace);
		RevampTileOverhang(tilespace);

		while (! IsEmpty (&tilespace->TileSets)) {
			tileset = Head (&tilespace->TileSets);
			RemoveTileSet (tileset);
			UnLoadTileSet (tileset);
		}
		SAFEFREE (tilespace);
	}
	return (NULL);

} /* DeleteTileSpace */


#if 0
/**************************************************************************
 *
 * DuplicateTileSet
 *
 * SYNOPSIS
 *		TileSetType *DuplicateTileSet (TileSetType tileset)
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
 *
 *
 * SEE ALSO
 *
*/
TileSetType *DuplicateTileSet (TileSetType tileset)
{
	TileSetType *newtileset = NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DuplicateTileSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (newtileset);
/*-----------------------------------------------------------------------*/
ABORT:
	return (NULL);

} /* DuplicateTileSet */
#endif


#if 0

void PrintPlanes (
	UBYTE	**planes,
	short	width,
	short	height,
	short	depth,
	short	start
)
{
	short	y;
	short	x;
	short	plane;
	short	b;
	UWORD	data;
	UWORD	*planedata;
	short	row_words;

	row_words	= ((width - 1) / 16) + 1;

	for (plane = start; plane < (start + depth); plane++) {

		printf ("plane # %d:\n", plane);

		planedata	= (UWORD *) planes[plane];

		for (y = 0; y < height; y++) {
			for (x = 0; x < row_words; x++) {
				printf (" ");
				data = *planedata++;
				for (b = 0; b < 16; b++) {
					if (data & 0x8000) {
						printf ("*");
					} else {
						printf (".");
					}
					data <<= 1;
				}
			}
			printf ("\n");
		}
		printf ("\n");
	}
	printf ("\n");

	return;

} /* PrintPlane  */
#endif

