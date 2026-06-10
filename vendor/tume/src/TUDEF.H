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
 * TUDEF.H
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 08/09/90 
 *   MODIFIED : 10/20/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w-----
 *	 E C H I D N A
 *	--------------
 *
 * DESCRIPTION
 *		defines for tUME, created from typedefs.h (CREATED : 11/01/89).
 *
 * HISTORY
 *	04/13/91 Saturday (RGM) - Added Parent_Room field to Layer structure.
 *
 *	03/17/93 Wednesday (dcc) - Change TileSetType to use cinShared instead
 *							of SharedNumberColors & SharedColors[]. This has
 *							the effect of putting the palette in XTRA memory.
 *
 *	04/01/93 Thursday (dcc) - add *Comment[] to RoomType
 *
 *	05/30/93 Sunday (dcc) - add ctilx, ctily, cxTile, & cyTile to LayerType
 *						delete Width, Height, TileWidth, TileHeight from RoomType
 *						delete Width & Height from BlockCopyType.
 *	12/13/93 Monday (dcc) - added cxDisplay, ctilxSkip, ctilxDispO, etc.
 *						to TileSetType
 *	12/17/93 Friday (dcc) - added ctilxDisplay & ctilyDisplay to BlockCopyType
 *	08/18/94 Thursday (dcc) - add support for sparse arrays
 *
*/
#ifndef TUDEF_H
#define TUDEF_H

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/xtramem.h>
#include <echidna/etypes.h>

#if __AMIGAOS__
#include <graphics/gfx.h>
#include <echidna/picture.h>
#elif __MSDOS__/*dcc*/
#include "picture.h"
#include "tumebpi.h"
#endif/*__AMIGAOS__/__MSDOS__*/

#if __AMIGAOS__
#define MAXCOLORREG	(32)
#elif __MSDOS__
#define MAXCOLORREG	(256)
#endif/*__AMIGAOS__/__MSDOS__*/

#include "colorseq.h"
#include "sparse.h"

/**************************** C O N S T A N T S ***************************/

#define NO		0
#define YES		1
#define CANCEL	(-1)
#define ERROR	(-2)

#define PATH_MAP			0
#define PATH_ROOM			1
#define PATH_TILES			2
#define PATH_SAVETILES		3
#define PATH_RELOADTILES	4
#define	PATH_PICTURES		5
#define PATH_PALETTE		6

/*---------------------- tUME CONSTANTS ------------------------------*/

#define INVERSE_W(ARG1)		( (ARG1) ^ ((UWORD) 0xFFFF) )
#define INVERSE_B(ARG1)		( (ARG1) ^ ((UBYTE) 0xFF) )

#define	MAX_TILESETS	256

#define MAXPLANES 	(8)

#define MAXDIRECTIONS (4)

/*----------------------- Structure size stuff -----------------------*/

#define TILESIZE		(sizeof (TileType))
#define TILESETSIZE		(sizeof (TileSetType))
#define TILESPACESIZE	(sizeof (TileSpaceType))

#define PLOTSIZE		(sizeof (PlotType))
#define LAYERSIZE		(sizeof (LayerType))
#define ROOMSIZE		(sizeof (RoomType))
#define MAPSIZE			(sizeof (MapType))

#define MATCHSIZE		(sizeof (MatchType))

#define BLOCKCOPYSIZE	(sizeof (BlockCopyType))

#define ROOMWINDOWSIZE		(sizeof (RoomWindowType))
#define ROOMWINDOWLISTSIZE	(sizeof (RoomWindowListType))
#define RWLOCKSIZE			(sizeof (RWLockType))

#define IDTENTRYSIZE		sizeof (ID_TranslateEntryType)
#define IDTSIZE				sizeof (ID_TranslateType)

#define RWINITSSIZE	sizeof (RWInitsType)

/*------------------------- Modes & Flags -----------------------------*/

/*** Draw mode constants ***/

#define DMODE_DRAW			0
#define DMODE_FILL			1
#define DMODE_RANDOM		2

#define DMODE_COLOR1		3
#define DMODE_COLOR2		4
#define DMODE_COLOR3		5
#define DMODE_COLOR4		6
#define DMODE_COLORREVERT	7

/***************************/


/*** TILESET FLAGS ***/

#define PRETILESET		(((UWORD) 0x01) << 0)
#define ASFOUND			(((UWORD) 0x01) << 1)
#define DONT_APPEND		(((UWORD) 0x01) << 2)
#define DONT_SAVEIMAGES	(((UWORD) 0x01) << 3)

#define TWO_BLANK_TILE_ENDING	(((UWORD) 0x01) << 4)
#define THROW_AWAY_REPEAT_TILES	(((UWORD) 0x01) << 5)
#define THROW_AWAY_BLANK_TILES	(((UWORD) 0x01) << 6)

#define	GRID_TYPE_MODIFIER					(((UWORD) 0x03) << 8)
#define	GRID_IS_BOXED						(((UWORD) 0x00) << 8)
#define	GRID_IS_NOT_BOXED					(((UWORD) 0x01) << 8)
#define	TILESET_COMPOSITE					(((UWORD) 0x03) << 8)

#define	GRID_START_MODIFIER					(((UWORD) 0x03) << 10)
#define	GRID_START_FROM_TOP					(((UWORD) 0x00) << 10)
#define	GRID_START_FROM_FIRST_NOTRANSPARENT	(((UWORD) 0x01) << 10)
#define	GRID_START_FROM_USER				(((UWORD) 0x02) << 10)

#define	GRID_TILEDIMENSIONS_MODIFIER				(((UWORD) 0x03) << 12)
#define	GRID_TILEDIMENSIONS_FROM_USER				(((UWORD) 0x00) << 12)
#define	GRID_TILEDIMENSIONS_FROM_FIRST_NOTRANSPARENT (((UWORD) 0x01) << 12)

#define	GRID_OFFSET_MODIFIER				(((UWORD) 0x03) << 14)
#define	GRID_OFFSET_FROM_TILEDIMENSIONS			(((UWORD) 0x00) << 14)
#define	GRID_OFFSET_FROM_FIRST_NOTRANSPARENT	(((UWORD) 0x01) << 14)
#define	GRID_OFFSET_FROM_USER					(((UWORD) 0x02) << 14)

#define IsTSTComposite(ptst)	(((ptst)->Flags & GRID_TYPE_MODIFIER) == TILESET_COMPOSITE)

/* these are old also. . .
#define LIVE_STUDIOS	(((UWORD) 0) << 8)
#define NINTENDO8		(((UWORD) 1) << 8)
*/

/*********************/


/*** SAVE TILESPACE CONSTANTS ***/
#define SAVETILES_AS_TTI	((UWORD) (0x01 << 0))
#define SAVETILES_AS_NAMED	((UWORD) (0x01 << 1))


/*** GRAB TILES CONSTANTS ***\

this is old stuff, I want to
get rid of it. . .so it's commented out!

#define	NOBORDER		(0)
#define	FIRSTFOUND		(1)
#define	DESIGNATED		(2)
#define	NINTENDO		(4)

\****************************/


/*** ROOM FLAGS ***/

#define FREEROOM 			((UWORD) 0x00)
#define PREROOM				((UWORD) 0x01)
#define RAWROOM				((UWORD) 0x02)
#define SOURCEROOM			((UWORD) 0x04)
#define LOCKEDROOM			((UWORD) 0x08)
#define fGridOn				((UWORD) 0x10)
#define MaskfGridOn			((UWORD) fGridOn ^ (0xFFFFU) )
#define fGuideVisible		((UWORD) 0x20)
#define MaskfGuideVisible	((UWORD) fGuideVisible ^ (0xFFFFU) )

#define IsComposite(room)		(((room)->ptsComposite) != NULL)
#define IsSource(room)			(((room)->Flags) & SOURCEROOM)
#define IsEdit(room)			(!IsSource(room) && !IsComposite(room))
#define IsLocked(room)			(((room)->Flags) & LOCKEDROOM)
#define IsGridOn(room)			(((room)->Flags) & fGridOn)
#define IsGuideVisible(room)	(((room)->Flags) & fGuideVisible)

#define ResetGridOn(room)		(((room)->Flags) &= MaskfGridOn)
#define SetGridOn(room)			(((room)->Flags) |= fGridOn)
#if NEWTUME
#define ResetGuideVisible(room)	{	((room)->Flags) &= MaskfGuideVisible;	\
									((room)->fShowGuide[0]) = FALSE;		\
									((room)->fShowGuide[1]) = FALSE; }
#define SetGuideVisible(room) 	{	((room)->Flags) |= fGuideVisible;	\
									((room)->fShowGuide[0]) = TRUE;		\
									((room)->fShowGuide[1]) = TRUE; }
#else
#define ResetGuideVisible(room)	(((room)->Flags) &= MaskfGuideVisible)
#define SetGuideVisible(room) 	(((room)->Flags) |= fGuideVisible)
#endif

#define IsReadyForUse(room)		(((room)->Flags) & USABLEROOM)
#define NeedsToBePrepped(room)	(! (((room)->Flags) & USABLEROOM))


/*** LAYER FLAGS ***/

#define LAYER_LOCKED			((UWORD) 0x0001)
#define LAYER_INVISIBLE			((UWORD) 0x0002)
#if UseSparseArray
#define LAYER_SPARSEARRAY		((UWORD) 0x0100)
#endif // UseSparseArray
#define LAYER_NOT_YET_PASTED	((UWORD) 0x8000)

#if UseSparseArray
#define IsSparse(play)			((play)->L_Flags & LAYER_SPARSEARRAY)
#endif // UseSparseArray

/*** ROOMSTUFF FLAGS ***/
#define DISPLAY_FLUSH			((UWORD) 0x00)
#define DISPLAY_SEPERATED		((UWORD) 0x01)
#define DISPLAY_SEPERATED_MASK	( DISPLAY_SEPERATED  ^ ((UWORD) 0xFFFF) )
#define SOURCE_LOCK				((UWORD) 0x02)
#define SOURCE_LOCK_MASK		( SOURCE_LOCK ^ ((UWORD) 0xFFFF) )
#define EDIT_LOCK				((UWORD) 0x04)
#define EDIT_LOCK_MASK		 	( EDIT_LOCK ^ ((UWORD) 0xFFFF) )
#define SCROLL_LOCK				((UWORD) 0x08)

/*** BLOCKCOPY FLAGS ***/
#define SAME_AS_SOURCE 			((UWORD) 1)

/*** TITLEBAR FLAGS ***/
#define SHOW_TITLE		0
#define SHOW_DEFAULT	1
#define SHOW_VERSION	2
#define SHOW_USER		3
#define SHOW_CURSOR		4
#define SHOW_NAME		5
#define SHOW_TUSER		6
#define SHOW_LAYERSHIFT	7

/*** MISC STUFF ***/
#define SOURCE_EOR	((UBYTE) 0xEA)
#define DEST_EOR	((UBYTE) 0xF5)
#define GUIDE_EOR	((UBYTE) 0xFF)

/*** STATUS VALUES ***/
#define S_OK		(0)
#define S_LOW_MEM	(-2)
#define S_ERROR		(-1)

/* pixel width and height of a pixel object tile KLUDGE BUGBUG FIXME */

#define cpixxObjectTile	33
#define cpixyObjectTile	15

/********************************* XTRAMEM ********************************/

/* Don't allocate anything larger than 32768 bytes at any given time */

#define wLargestXTRA 32768U
#define wAllocXTRAMax 32728U

#define winDst 0
#define winSrc 2
#define winTil 2

/******************************** T Y P E S *******************************/

/*---------------------- TILE BASED STRUCTURES ----------------------*/

#if __AMIGAOS__
typedef UBYTE *MyPlanePtr;
#endif/*__AMIGAOS__*/

#if __AMIGAOS__
typedef struct {
//			NodeType	 T_Node;
	struct	NodeType	*Next;
	struct	NodeType	*Prev;
			UBYTE		 Type;
			WORD		 TileNumber;
			WORD		 Original_Column;
			WORD		 Original_Row;
	struct	BitMap		 ImageBitMap;
			MyPlanePtr	 MaskPlanes [MAXPLANES];
} TileType;
#endif/*__AMIGAOS__*/

#define MAX_USER_TEXT	2

typedef struct TileSetType {
		NodeType		TS_Node;		/* includes "TS_Node.name" label */
		WORD			TS_id;
		UWORD			Flags;
		UWORD			SaveFlags;
		char			*Filespec;
		char			*Comment[MAX_USER_TEXT];
		WORD			UserType;
		WORD			UserNumber;
struct	RoomType		*SourceRoom;
		WORD			wDisplayRow;	/* Which row to start showing tiles */
		UWORD			TileCount;
		ListType		Tiles;
#if __AMIGAOS__
		TileType		**TilePtr;		/* TilePtr[n] */
#endif/*__AMIGAOS__*/
		WORD			Width;			/* width of tile in pixels */
		WORD			Height;			/* height of tile in pixels */
		BOOL			fAlwaysTransp;	/* NOT SAVED! always draw tile transparently? */
		int				cxDisplay;		/* NOT SAVED! pixels across to display each tile */
		int				cyDisplay;		/* NOT SAVED! pixels down to display each tile */
		int				ctilxSkip;		/* NOT SAVED! x tile displacement for next tile */
		int				ctilxDispO;		/* NOT SAVED! x tile displacement for start of odd row */
		int				ctilyDispO;		/* NOT SAVED! y tile displacement for start of odd row */
		int				ctilxDispE;		/* NOT SAVED! x tile displacement for start of even row */
		int				ctilyDispE;		/* NOT SAVED! y tile displacement for start of even row */
		WORD			Depth;
		WORD			Across;
		WORD			Down;
#if __AMIGAOS__
		MyPlanePtr		OriginalImagePlanes [MAXPLANES];
		MyPlanePtr		OriginalMaskPlanes [MAXPLANES];
		MyPlanePtr		FullPlane;		/* NEW FOR NINTENDO */
		MyPlanePtr		EmptyPlane;		/* NEW FOR NINTENDO */
		ULONG			PlaneSize;		/* NEW FOR NINTENDO */
		Shape			*SharedShape;
#elif __MSDOS__
		UWORD			MaxAcross;		/* used as RowBytes in DeltaOriginalPos */
		UWORD			sizeofTile;
		UWORD			tilesPerCol;
		UWORD			numTilCols;
		XTRAPntr	   *pxtrTilCol;
		UWORD			numDelCols;
		XTRAPntr	   *pxtrDelCol;
		UWORD			SharedtransparentColor;
		ColorInfo	   *cinShared;
#endif/*__AMIGAOS__/__MSDOS__*/
struct	TileSpaceType	*MyTileSpace;
		UWORD			bGroupTileNum;	/* group tilesets belong in, valid when counting tiles. */
		XTRAPntr		rgRoomUseFlags;	/* -> room use flags, valid when counting tiles. */
		short			wTilesAcross;	/* Number of composite tiles / tile */
		short			wTilesDown;		/* Number of composite tiles / tile */
		short			LayerToUse;		/* NOT SAVED! Layer to paste tile when stratifying */
		UBYTE			ubShowFlagAND;	/* NOT SAVED! AND flags with the byte before display */

#if PIXELSELECT
	/*	<pdxLeft> and <pdxRight> are used to keep track of which pixels are
		"occupied" on each pixel line. For a regular tile (normally), <pdxLeft>
		is an array of all zero values, and <pdxRight> are the width of the tile.
		For an isometric tiles, these values be different.

		<pdyTop> and <pdyBottom> are used to keep track of which pixels are
		"occupied" on each pixel line. For a regular tile (normally), <pdyTop>
		is an array of all zero values, and <pdyBottom> are the height of the tile.
		For an isometric tiles, these values be different.

		These values are used for calculation whether a exact-pixel selection
		brush picks up a tile or not.

		In the current implementation, we only compute these values for the
		entire tileset. In some future implementation, we may want to change
		this to be for each tile within the tileset. */

		int				*pdxLeft;		/* NOT SAVED! left edge of each tile */
		int				*pdxRight;		/* NOT SAVED! right edge of each tile */
		int				*pdyTop;		/* NOT SAVED! top edge of each tile */
		int				*pdyBottom;		/* NOT SAVED! bottom edge of each tile */

	/*	The following variables are used to draw an outline around a tile.

		DRAW SEGMENT	START HERE	END HERE

		LEFT			dyL0		dyL1
		TOP-LEFT		dyL1		dxT1
		TOP				dxT0		dxT1
		TOP-RIGHT		dxT0		dyR1
		RIGHT			dyR0		dyR1
		RIGHT-BOTTOM	dyR0		dxB0
		BOTTOM			dxB0		dxB1
		LEFT-BOTTOM		dyL0		dxB1	*/

		int				dyL0;			/* NOT SAVED! start left segment here */
		int				dyL1;			/* NOT SAVED! end left segment here */
		int				dyR0;			/* NOT SAVED! start right segment here */
		int				dyR1;			/* NOT SAVED! end right segment here */

		int				dxT0;			/* NOT SAVED! start top segment here */
		int				dxT1;			/* NOT SAVED! end top segment here */
		int				dxB0;			/* NOT SAVED! start bottom segment here */
		int				dxB1;			/* NOT SAVED! end bottom segment here */
#endif // PIXELSELECT

		BOOL			fcntValid;		/* TRUE when tile counts are valid */
		XTRAPntr		ptct;			/* array of tile counts */
} TileSetType;


typedef struct TileSpaceType {
	UWORD			HighestID;
	UWORD			SaveFlags;
	ListType		TileSets;
	TileSetType		*TileSetArray[MAX_TILESETS];	/* TileSet[n] */
} TileSpaceType;

/*---------------------- LAYER BASED STRUCTURES --------------------------*/

#include "plottype.h"

#if !PLOTARRAY
typedef	XTRAPntr	MPXTPT;
typedef MPXTPT		*MPYTMPXTPT;

typedef struct
{
	MPYTMPXTPT	mpYtMpXtPt;
	WORD		PlotWidth;
	WORD		PlotHeight;
	WORD		wRowsPerXTRA;		/* Number of rows per XTRAPntr */
} RGRGPLT;

#endif

typedef struct
{
		NodeType	 L_Node;
		UWORD		 L_Flags;
		int			 ctilx;				/* width of layer in tiles */
		int			 ctily;				/* height of layer in tiles */
		int			 dtilx;				/* 02/20/94: x-offset of layer: NOT SAVED */
		int			 dtily;				/* 02/20/94: y-offset of layer: NOT SAVED */
		UWORD		 cxTile;			/* width of a single tile */
		UWORD		 cyTile;			/* height of a single tile */
struct	RoomType	*Parent_Room;		/* ADDED 4/13/91 for Undo thingy */
#if PLOTARRAY
		PlotType	*Plot;
#else
#if UseSparseArray
		union
		{
			RGRGPLT			rgrgplt;
			SparseArray		*pspa;
		} p;
#else // !UseSparseArray
		RGRGPLT		 rgrgplt;
#endif // !UseSparseArray
#endif // PLOTARRAY
} LayerType;


typedef struct RoomType
{
	NodeType	R_Node;
	WORD		R_id;
	WORD		Flags;
	UWORD		SaveFlags;
	UWORD		LayerCount;
	UWORD		FloorNumber;
	WORD		UserType;
	WORD		UserNumber;
	WORD		BackgroundColor;
	WORD		Direction[MAXDIRECTIONS];
#if NEWTUME
	WORD		cGuideLines;
	LONG		*pcenGuideA;
	LONG		*pcenGuideB;
	LONG		*pcenGuideDC;
	LONG		*pcenGuideXOrigin;
	LONG		*pcenGuideYOrigin;
	BOOL		*fShowGuide;
#else
	WORD		wGuideXWidth;
	WORD		wGuideYHeight;
	WORD		wGuideXOrigin;
	WORD		wGuideYOrigin;
#endif
	WORD		wGridXWidth;
	WORD		wGridYHeight;
	WORD		wGridXOrigin;
	WORD		wGridYOrigin;
	WORD		wZoomIndex;			/* Index into zoom [], bit 15 set==zoom off */
	LayerType	*FloorLayer;
	ColorInfo	*R_ColorInfo;
	char		*Name;
	char		*Comment[MAX_USER_TEXT];
	ListType	Layers;
	ListType	Items;				/* Items in this room */
	ListType	Displays;			/* rw's showing this room */
	ListType	RWInits;
	TileSetType	*ptsComposite;		/* If ! NULL, this room is composite TS, -> TS */
	WORD		wOldRoomWidth;		/* Before compositing */
	WORD		wOldRoomHeight;		/* Before compositing */

	BOOL		fcntValid;			/* TRUE when tile counts are valid */
	XTRAPntr	*prgtct;			/* array of pointer to array of tile counts */
} RoomType;


typedef struct {
	UWORD		SaveFlags;
	ListType	Rooms;
} MapType;


/*----------------------- RoomStuff Structures ---------------------------*/

typedef struct {
			RoomType	*Room;
			WORD		Flags;
			ULONG		dxSrc;		// 08/26/94 (dcc) - revised to be pixel
			ULONG		dySrc;		// offset in source instead of tile offset
			WORD		ScreenPlotRows;
} RoomStuffType;


typedef struct {
			NodeType		RW_Node;
	struct	Window			*Window;
			RoomStuffType	*CurrentRoom;
			RoomStuffType	*OtherRoom;
			RoomStuffType	SourceRoom;
			RoomStuffType	EditRoom;
} RoomWindowType;


typedef struct
{
	WORD			BC_Flags;
	RoomWindowType	*SourceRW;
	RoomStuffType	*SourceStuff;
	RoomType		*SourceRoom;
#if PIXELSELECT
	LayerType		*playSrcFloor;
#endif // PIXELSELECT
	RoomWindowType	*DestRW;
	RoomStuffType	*DestStuff;
	WORD			SourceX;
	WORD			SourceY;
	WORD			DestX;
	WORD			DestY;
#if PIXELSELECT
	int				dtilxOffset;		/* x-displacement of layer from pointer */
	int				dtilyOffset;		/* y-displacement of layer from pointer */

	/*	Please note that due to the search for overlapping tiles, 
		ctilxSelect & ctilySelect may be smaller than the play->ctilx & ctily. */

	int				ctilxSelect;		/* x-tile width of selection rectangle */
	int				ctilySelect;		/* y-tile height of selection rectangle */

	int				dxModSrcL;
	int				dyModSrcT;
	int				dxModSrcR;
	int				dyModSrcB;
#endif // PIXELSELECT
	int				ctilxDisplay;
	int				ctilyDisplay;
	UWORD			LayerCount;
	ListType		Layers;
	LayerType		*BC_FloorLayer;
	TileSetType		*FirstTileSet;
#if __AMIGAOS__
	PlotType		*Background;
#endif/*__AMIGAOS__*/
	ListType		*pSpecialTilesets;	/* tiles in brush that need special processing */
} BlockCopyType;



typedef struct {
			ListType	RoomWindows;
} RoomWindowListType;


typedef struct {
			NodeType		RWL_Node;
			RoomWindowType	*RoomWindow;
			RoomStuffType	*RoomStuff;
} RWLockType;

/*----------------------- MISCELANEOUS STRUCTURES ----------------------*/

typedef struct {
	short	start_x;
	short	start_y;
	short	image_width;
	short	image_height;
	short	offset_x;
	short	offset_y;
} GridInfoType;

typedef struct {
	NodeType		M_Node;
	RoomType		*MatchRoom;
} MatchType;

typedef struct {
	ListType	Matches;
	UWORD		Count;
} MatchRoomsType;

typedef struct {
	NodeType		RWI_Node;
	ULONG			dxSrcPrev;
	ULONG			dySrcPrev;
	RoomWindowType	*RW;
} RWInitsType;

typedef WORD STATUS;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/

#define ReportError(a, b, c) printf (a, b);printf ("   %s.\n", c)
#define SAFEFREE(pointer)				{MEM_free (pointer); pointer = NULL;}

#define IS_SOURCE_ROOM(room)			(room->Flags & SOURCEROOM)
#define IS_SOURCE_LOCKED(roomstuff)		(roomstuff->Flags & SOURCE_LOCK)
#define IS_EDIT_LOCKED(roomstuff)		(roomstuff->Flags & EDIT_LOCK)

#if __AMIGAOS__
#define FAST_TILE_PTR(set_id, tile_id) \
	((set_id) ? GlobalTileSpace->TileSetArray[set_id]->TilePtr[tile_id] : NULL)
#endif/*__AMIGAOS__*/

#define	fNullTile(pplt) (!((pplt) && (pplt)->TileSet_ID && (pplt)->Tile_ID))

/*	fZeroTile() is a completely zero tile: tileset, tile_id & flags.
	This is only used to check object tiles. */

#define	fZeroTile(pplt) (!(pplt)->TileSet_ID && !(pplt)->Tile_ID && !(pplt)->Plot_Flags)

#define FAST_TILESET_PTR(set_id) \
				(GlobalTileSpace->TileSetArray[set_id])

#if __AMIGAOS__
#define TILE_AT_PLOT(plot) \
	((plot) ? FAST_TILE_PTR((plot)->TileSet_ID, (plot)->Tile_ID) : NULL)
#endif/*__AMIGAOS__*/

#define TILESET_AT_PLOT(plot) \
	((plot) ? FAST_TILESET_PTR((plot)->TileSet_ID) : NULL)

/*--------------------- TileSource MACROS ---------------------------*/
#if __AMIGAOS__
#define TileSourceType				Picture

#define TileSourceWidth				PicWidth
#define TileSourceHeight			PicHeight
#define TileSourceColorMap			PicColorMap
#define TileSourceColors			PicColors
#define TileSourceTColor			PicTColor
#define TileSourceRastPort(ARG1)	(&((ARG1)->RastPort))

#define LoadTileSource(STRING_PTR)	((void *)  CreatePic (STRING_PTR))
#define UnLoadTileSource(TS_PTR)	(DeletePic (TS_PTR))
#elif __MSDOS__/*dcc*/
#define TileSourceType				BlockOPixels

#define TileSourceWidth(BOP)		((BOP)->Pixels.BigByteMap.width)
#define TileSourceHeight(BOP)		((BOP)->Pixels.BigByteMap.height)
#define TileSourceColorMap(BOP)		((BOP)->Colors)
#define TileSourceColors(BOP)		((BOP)->NumberOfColors)
#define TileSourceTColor(BOP)		((BOP)->Pixels.BigByteMap.transparentColor)
#define TileSourceRastPort(BOP)		(&((BOP)->Pixels.BigByteMap))

#define LoadTileSource(STRING_PTR)	((void *)  CreateBlockOPixels(STRING_PTR))
#define UnLoadTileSource(TS_PTR)	(DestroyBlockOPixels(TS_PTR))
#endif/*__AMIGAOS__/__MSDOS__*/

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

#endif /* TUDEF_H */

