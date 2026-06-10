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
 * TUFUNC.H
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
 *		Function headers for tUME.
 *
 * HISTORY
 *		08/09/90 (RGM) - Created out of "headers.h"(CREATED : 11/11/89).
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
*/
#ifndef TUFUNC_H
#define TUFUNC_H

#include <echidna/platform.h>
#include "switches.h"

#include "tudef.h"

/**************************** C O N S T A N T S ***************************/

#define	DISPLAYTOBACK	0
#define	BACKTODISPLAY	1

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

#if __AMIGAOS__
/***** FOR NEW LIBRARY TEST ROUTINES *****/
/* EUI.LIB : TEST_freqsupport.c */

extern void SetFreqFilename (
	EasyFileReq	*ezfrq,
	char		*filename
);

extern void SetFreqDirname (
	EasyFileReq	*ezfrq,
	char		*dirname
);
#endif/*__AMIGAOS__*/

/******************************** BLITTER.C *******************************/

extern void CopyMaskBitMap (
	struct BitMap	*frombm,
	int				fromleft,
	int				fromtop,
	struct BitMap	*tobm,
	int		 		toleft,
	int		 		totop,
	int		 		inwidth,
	int		 		inheight,
	UBYTE			planemask,
	UBYTE			*mask
);

extern void CopyBitMap (
	struct BitMap	*frombm,
	int				fromleft,
	int				fromtop,
	struct BitMap	*tobm,
	int		 		toleft,
	int		 		totop,
	int		 		inwidth,
	int		 		inheight,
	UBYTE			planemask
);

#if __AMIGAOS__
extern short BitplanesAreSame (
	UBYTE	**plane_array1,
	UBYTE	**plane_array2,
	short	width,
	short	height,
	UBYTE	depth
);
#elif __MSDOS__

/*********************** MCOPYREC.ASM / COPYRECP.ASM **********************/

#define COPYNOMASKS		(0)
#define COPYTRANSMASK	(1)
#define COPYPIXELMASK	(2)

#define YFLIPMASK		(32)
#define XFLIPMASK		(64)

extern far32 UBYTE copyMode;		/* MCGA_ClippedMaskedCopyTransRect copy mode */
extern far32 UBYTE pixelAND;		/* AND every pixel with this mask */
extern far32 UBYTE pixelOR;			/* OR every pixel with this mask */

extern far32 int wSrcSkipX;
extern far32 int wDstDupX;
extern far32 int wSrcSkipY;
extern far32 int wDstDupY;

/********************************* MSDOS.C ********************************/

#if NEWTUME
extern void XORDrawLine (struct ByteMap *pbm,
						int x0, int y0, int x1, int y1, UBYTE ub);

#define XORvLine(pbm,x1,y1,y2,ub) XORDrawLine(pbm,x1,y1,x1,y2,ub)
#define XORhLine(pbm,x1,x2,y1,ub) XORDrawLine(pbm,x1,y1,x2,y1,ub)
#else
extern void XORvLine(struct ByteMap *bytemap, int x1, int y1, int y2, UBYTE ub);
extern void XORhLine(struct ByteMap *bytemap, int x1, int x2, int y1, UBYTE ub);
#endif

extern int ReadPixel(BigByteMap *pbbm, int xp, int yp);
extern BOOL isByteMapTransparent(ByteMap *bm);

/* These following function is used by xportmap#WritePBM(). */

extern ULONG WriteCMAP(int hout, UWORD uwNumberColors, CS_Color *Colors);

extern BOOL SaveByteMap(ByteMap *bm, UBYTE ubDepth,
					UWORD uwNumberColors, CS_Color *Colors, char *path);

extern void CopyBigRectToSmall(BigByteMap *pbbmSrc, int xpSrc, int ypSrc,
					ByteMap *pbmDst, int xpDst, int ypDst,
					int xpWidth, int ypHeight);

extern void CopySmallRectToBig(ByteMap *pbmSrc, int xpSrc, int ypSrc,
							BigByteMap *pbbmDst, int xpDst, int ypDst,
							int xpWidth, int ypHeight);

extern void CopyDisplayToBack(int xp, int yp, int xpWidth, int ypHeight,
								int direction);
extern void CopyDisplayTilesToBack(RoomWindowType *roomwindow, int column, int row,
								int width, int height, int direction);

extern void MCGA_ClippedMaskedCopyTransRect (ByteMap *sbm, int fx, int fy,
					ByteMap	*dbm, int tx, int ty,
					int w, int h);

extern void MCGA_ClippedScaledMaskedCopyTransRect (
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
				);

extern char *BaseName(char *pathname);

extern void SetFreqFilename (
	EasyFileReq	*ezfrq,
	char		*filename
);

extern void SetFreqDirname (
	EasyFileReq	*ezfrq,
	char		*dirname
);

extern UBYTE GetCharByteMap(ByteMap *pbm, PlotType *pplt,
						int wXTOffset, int wYTOffset);

extern BOOL OpenMousePointers(char *sz);
extern void CloseMousePointers(void);
extern void DCC_TempSetPointer(short wMouse);
extern short DCC_SetPointer(short wMouse);
#endif/*__AMIGAOS__/__MSDOS__*/

/******************************* COLORSEQ.C *******************************/

/* colorseq.h */

/****************************** COLORSEQREQ.C *****************************/

#if __AMIGAOS__
extern int ColorReq (
	struct	Window	*win,
	ColorInfo		*ci
);
#elif __MSDOS__
extern short ColorSequencer (
	struct	Window	*win,
	ColorInfo		*ci
);
#endif/*__AMIGAOS__/__MSDOS__*/

extern void UninitColorSeqReq (void);
extern int InitColorSeqReq (void);

#if __AMIGAOS__
/******************************** DEADKEY.C *******************************/

extern long DeadKeyConvert (
	struct	IntuiMessage	*msg,
	UBYTE					*kbuffer,
	ULONG					kbsize,
	struct	KeyMap			*kmap
);
#endif/*__AMIGAOS__*/

/******************************* EASYFREQ.C *******************************/

extern char *EasyGetFileName (
	struct	Window	*window,
	char			*title
);

/******************************** ERRORS.C ********************************/

extern void KeyError (char *mess);
extern void KeyError1 (
	char *mess,
	void *arg
);

/******************************** EVENT2.C ********************************/

extern void DoIt (void);

/****************************** FREQSUPPORT.C *****************************/

#if __AMIGAOS__
/******************************* INACTIVE.C *******************************/

extern int Inactive (struct IntuiMessage *im);
#endif/*__AMIGAOS__*/

/********************************** IO.C **********************************/

/* io.h */

/********************************* KEYS.C *********************************/

/* keycommands.h */

/******************************* MENUITEMS.C ******************************/

extern void SetPointerMode(void);
extern int ClearNLoadMap(BOOL fClear, char *sz);
extern int NMap (void);
extern int LMap (void);
extern int AMap (void);
extern int SMap1 (void);
extern int SMap2 (void);
extern int SMap3 (void);
extern int TE_SaveTilesAsBrushes (void);
extern int tumeInfo (void);
extern int EditRoom (void);
extern int DrawRandom (void);
extern int TE_LoadTilesFullTiled (void);
extern int TE_LoadTilesFullTiledNoBlank (void);
extern int TE_LoadTilesCookieCutter (void);
extern int TE_LoadTilesBoxed (void);
extern int CRoom (void);
extern int DRoom (void);
extern int LRoom (void);
extern int SRoom (void);
extern int SRoomAll (void);
extern int WCreate (void);
extern int WClose (void);
extern int WPRoom (void);
extern int WNRoom (void);
extern int WFRoom (void);
extern int US (void);
extern int US2 (void);
extern int WLS (void);
extern int WLE (void);
extern int WLC (void);
extern int WLCl (void);
extern int SpaceToggle (void);
extern int FloorUp (void);
extern int FloorDown (void);
extern int OpenLayer (void);
extern int DeleteLayer (void);
extern int RoomStatus (void);
extern int UserStatus (void);
extern int VersionStatus (void);
extern int CopyrightStatus (void);
extern int CursorStatus (void);
extern int TUserStatus (void);
extern int ClearRoom (void);
extern int ClearRoom2 (void);
extern int ClearRoom3 (void);
extern int ScrollLock (void);
extern int ScrollLockKEY (void);
extern int ToggleLInvisi (void);
extern int ToggleLLock (void);
extern int ToggleLAll (void);
extern int ToggleLockRoom (void);
extern int Undo (void);
extern int RenameRoom (void);
extern int RInfo (void);
extern int TUser (void);
extern int URoom (void);
extern int StripDownBlockCopy (void);
extern int CopyColors1 (void);
extern int CopyColors2 (void);
extern int CopyColors3 (void);
extern int EditOnlyFloor (void);
extern int NTIMark (void);
extern int IncGColor (void);
extern int DecGColor (void);
extern int ZGColor (void);
extern int TileComment1 (void);
extern int TileComment2 (void);
extern int TE_TogglePointer (void);
extern int TE_Debug (void);
#if __AMIGAOS__
extern int TE_MakeColor1 (void);
extern int TE_MakeColor2 (void);
extern int TE_MakeColor3 (void);
extern int TE_MakeColor4 (void);
extern int TE_MakeColorRevert (void);
#endif/*__AMIGAOS__*/

/****************************** MENUVERIFY.C ******************************/

/********************************* MISC.C *********************************/

#if __AMIGAOS__

#define calloc_chip(size,count) \
		AllocMem ((ULONG) (size) * (count), (ULONG) (MEMF_CHIP | MEMF_CLEAR))

#define malloc_chip(size) \
		AllocMem ((ULONG) (size), (ULONG) (MEMF_CHIP))

#define	free_chips(buf,size) \
		FreeMem ((void *) (buf), (ULONG) (size))

#endif/*__AMIGAOS__*/

extern char *FixPath(char *name);

extern short PMessage (Window *window, char *title, char *message, char *options);

#if __AMIGAOS__
extern void SetColorRangeForReals (
	struct	ViewPort	*vp,
	int					c1,
	int					c2,
	UWORD				*colors
);
#elif __MSDOS__

extern void SetDisplayPalette(CS_Color *pcsc, WORD cscCnt);

#endif/*__AMIGAOS__/__MSDOS__*/

extern int ToggleCycleColors (void);

#if __AMIGAOS__
extern void SetColorRange (
	struct	ViewPort	*vp,
	int					c1,
	int					c2,
	UWORD				*colors
);
#endif

extern void SetColors (struct Window *CWindow);
extern void SetColorsNPointer (struct Window *CWindow);

#if __AMIGAOS__
extern void ShockColors (struct Window *CWindow);
#elif __MSDOS__
extern void InitMenuColors (ColorInfo *pColorInfo);
#endif/*__AMIGAOS__/__MSDOS__*/

extern void RestoreColors (struct Window *CWindow);
extern void RestoreColorsNPointer (struct Window *CWindow);

extern int SetTheColors (void);

extern void RestorePointer (void);

extern void RemovePointer (void);

extern int AreYouSure(char *mess);

extern int DoThis2 (
	char *title,
	char *mes1,
	char *mes2
);

extern int IsItThisOne (
	char *title,
	char *message
);

extern void SYSMESS (char *mess);

extern short TellUser (
	char	*title,
	char	*mess
);

extern void NotYet(void);

extern int QuitAndExit (void);

extern int About (void);

#if __AMIGAOS__
extern int ToggleTitleBar (int *tstate);
#elif __MSDOS__
short ToggleTitleBar (EventInfo *ei);
#endif/*__AMIGAOS__/__MSDOS__*/

extern int ToggleSwankyMode (void);

extern TileSetType *FindFirstTileSet (RoomType	*room);

extern TileSetType *FindFirstSelect (BlockCopyType	*block);

#if 0
extern BOOL FoundDir (char *dirname);

extern BOOL MakeDir (char *dirname);

extern BOOL EasyMakeDir (char *dirname);
#endif

extern ULONG FullPath (
	char	*in_name,
	char	*out_name,
#if __AMIGAOS__
	ULONG	maxlen
#elif __MSDOS__
	UWORD	maxlen
#endif/*__AMIGAOS__/__MSDOS__*/
);

extern void NameAndExt (
	char	*in_path,
	char	*out_name,
	ULONG	maxlen
);

extern void ShowStatus (char *message);

extern void ShowState (int number);

extern PlotType *GetRoomPlot (
	RoomType	*room,
	WORD		x,
	WORD		y
);

#if __AMIGAOS__
/***************************** MOUSEBUTTONS.C *****************************/

extern void FN_Nothing (void);

extern void RightButtonDown (void);

extern void RightButtonUp (void);

extern void LeftButtonDown (void);

extern void LeftButtonUp (void);

extern int MouseButtons (struct IntuiMessage *im);
#endif/*__AMIGAOS__*/

/****************************** MOUSEITEMS.C ******************************/

/* FSelectedPblk - returns non-zero if BlockCopyType pblk contains a selection */

#define FSelectedPblk(pblk)		((pblk) && (pblk)->LayerCount)

extern LayerType *FindBiggestLayer(ListType *pLayers);
extern LayerType *FindNonEmptyLayer(ListType *pLayers, LayerType *playFloor);

extern void ForceBrushRedraw(void);
extern void MButtonsE(void);

extern void ButtonsE (void);

#if 0
extern int RightScroll (void);
extern int LeftScroll (void);
extern int UpScroll (void);
extern int DownScroll (void);

extern int RightScrollMultiple (void);
extern int LeftScrollMultiple (void);
extern int UpScrollMultiple (void);
extern int DownScrollMultiple (void);
#endif

extern void GetCursorTilePosition (
	int			*cursor_column,
	int			*cursor_row,
	RoomWindowType	*rw
);

extern void MoveCenterTo(int wx, int wy, BOOL fScroll);

extern int CenterRoomOnCursor (void);

extern void MoveRequest (
	WORD	x_magnitude,
	WORD	y_magnitude
);

extern void ShiftLayer(short dtilx, short dtily);

extern void Unselect (void);

extern void Unselect2 (void);

/******************************* MYSTRING.C *******************************/

extern void StripComment (char	*line);

/******************************** NAMEREQ.C *******************************/

extern char *GetName (
	struct	Window	*win,
	char			*str
);

/********************************* PARSE.C ********************************/

extern int Parse (char *filename);

/******************************** POINTER.C *******************************/

/* pointer.h */

/******************************* ROOMINFO.C *******************************/

extern int RoomInfo (
	struct	Window	*win,
	RoomType		*room
);

/******************************** ROOMIO.C ********************************/

extern BOOL ExpandPlot(RGRGPLT *prgrgplt, WORD wNewWidth, WORD wNewHeight);

#if NEWTUME
extern BOOL ReAllocateRoomGuides(RoomType *prm, short cGuides);

#endif
#if PLOTARRAY
extern void InitTileSetLayer (
	RoomType		*room,
	TileSetType		*tileset,
	long			start_offset
);
#else
extern void InitTileSetLayer (
	RoomType		*room,
	TileSetType		*tileset,
	WORD			height
);
#endif
#if UseSparseArray
extern LayerType *AddClearedLayer(RoomType *room, int width, int height, BOOL fMakeSparse);
extern LayerType *AddClearedLayerToLayers(ListType *layers, int width, int height, BOOL fMakeSparse);
extern LayerType *InsertClearedLayerToLayers(RoomType *prm, LayerType *play, int width, int height, BOOL fMakeSparse);
#else // !UseSparseArray
extern LayerType *AddClearedLayer (
	RoomType	*room,
	WORD		width,
	WORD		height
);
extern LayerType *AddClearedLayerToLayers (
	ListType	*layers,
	WORD		width,
	WORD		height
);
extern LayerType *InsertClearedLayerToLayers(RoomType *prm, LayerType *play, short width, short height);
#endif // !UseSparseArray

extern int CtilxDisplayFromTST(TileSetType *ptst);
extern int CtilyDisplayFromTST(TileSetType *ptst);

extern RoomType *CreateTileSetRoom (TileSetType *tileset);

#if UseSparseArray
extern RoomType *CreateRoomWithUndo(int width, int height, UWORD room_flags, BOOL fMakeSparse);
#else // !UseSparseArray
extern RoomType *CreateRoomWithUndo (
	WORD	width,
	WORD	height,
	UWORD	room_flags
);
#endif // !UseSparseArray

extern BOOL ResizeRoom(RoomType *prt, WORD wNewWidth, WORD wNewHeight);

extern void DeAllocateLayer (LayerType *layer);
extern void DeAllocateLayers (ListType *layers);
extern void DeAllocateRWInits (ListType *rwinits);
extern void DeAllocateDisplays (ListType *displays);
extern void DeAllocateItems (ListType *items);
extern void DeAllocateRoom (RoomType *room);
extern RoomType *FirstSourceRoom (MapType *map);
extern RoomType *FirstEditRoom (MapType *map);
extern MapType *CreateMap (void);
extern void *DeleteMap (MapType *map);
//extern RoomType *DuplicateRoom (RoomType room);
extern RoomType *TryNFindRoomID (
	WORD	id,
	MapType	*map
);

extern BOOL DeleteRoomColumns(RoomType *prt, WORD wColumn, WORD wNumColumns);
extern BOOL DeleteRoomRows(RoomType *prt, WORD wRow, WORD wNumRows);

extern BOOL RoomIsUsedByUndo (RoomType *room);

extern void InvalidateUndo (void);
extern void StartUndo (
	RoomType	*room,
	RoomType	*undo_room,
	WORD		left,
	WORD		top,
	WORD		right,
	WORD		bottom
);
extern void UpdateUndo (
	RoomType	*room,
	RoomType	*undo_room,
	WORD		o_left,
	WORD		o_top,
	WORD		o_right,
	WORD		o_bottom
);
extern void RetrieveUndo (
	RoomType	*undo_room
);

/****************************** ROOMWINDOW.C ******************************/

extern void DeAllocateRoomWindow (RoomWindowType *roomwindow);
extern RoomWindowType *CreateRoomWindow (struct Window *window);
extern void TogglePrw(RoomWindowType *prw);
extern BOOL HookRoomWindow (
	RoomType		*room,
	RoomWindowType	*roomwindow
);
extern void UnHookRoomWindow (
	RoomType		*room,
	RoomWindowType	*roomwindow
);
extern void TotalRoomUnhook (
	RoomType		*room,
	RoomWindowType	*roomwindow
);
extern void TryToHookRoomWindow (
	RoomType		*room,
	RoomWindowType	*roomwindow
);
extern BOOL SameTypeRoom (
	RoomType		*room,
	RoomStuffType	*roomstuff
);
extern void FriendlyShowRoom (
	RoomType		*room,
	RoomWindowType	*roomwindow
);

/******************************* RSREQUEST.C ******************************/

#if 0
extern int RS_Request (
	int	*width,
	int	*height
);
extern int RU_Request (
	int	*user_type,
	int	*user_num
);
extern int TU_Request (
	int	*user_type,
	int	*user_num
);
extern int TU2_Request (
	int	*user_type,
	int	*user_num,
	char *name
);
extern int TS_Request (
	int	*width,
	int	*height
);
#endif

extern int GR_Request (
	WORD	*width,
	WORD	*height
);

extern int GU_Request (
	WORD	*width,
	WORD	*height
);
/******************************* RTSUPPORT.C ******************************/

extern RoomType *GetAppendRoom (
	TileSetType	*tileset,
	MapType		*map
);
extern BOOL AppendTiles (
	TileSetType		*tileset,
	RoomType		*room
);
extern RoomType *PlaceTiles (
	TileSetType	*tileset,
	RoomType	*requested_room,
	MapType		*map
);
extern void MarkUsedTileSets (
	RoomType		*room,
	BOOL			 fAllLayers,
	TileSpaceType	*tilespace,
	UWORD			 mark
);
extern void UnmarkTileSets (
	TileSpaceType	*tilespace,
	UWORD			mark
);

extern BOOL DeleteTiles(TileSetType *ptst);

extern BOOL MakeCompositeTileSet(RoomType *prt);

extern BOOL LinkCompositeTileSets(TileSpaceType *ptst, MapType *pmt, void *Translator);

extern void SetLockedCompositeRoom(RoomType *prt);

extern void SetUnlockedCompositeRoom(RoomType *prt);

extern BOOL SetRoomTilesTo(UBYTE ubIDOld, UBYTE ubIDNew);
extern BOOL FixCompositeIDs(UBYTE ubID, UWORD wWidthOld, UWORD wHeightOld, UWORD wWidthNew, UWORD wHeightNew);

/******************************* SMALLFONT.C ******************************/

extern struct TextFont *InitFont (void);

/******************************* TILESETIO.C ******************************/

#if __AMIGAOS__
extern void pTilDelete(TileType *ptil);
extern TileType *pTilNew(void);
#endif/*__AMIGAOS__*/

extern void UnLoadTileSet (TileSetType	*tileset);
extern TileSetType *LoadTileSet (
	char	*filename,
	UWORD	tileset_flags
);
extern TileSetType *ForceLoadTileSet (
	char			*filename,
	UWORD			 tileset_flags,
	GridInfoType	*original_grid_info,
	BOOL			 fGetTSTTypeFromUser
);
extern void AddTileSet (
	TileSetType *tileset,
	TileSpaceType *tilespace
);
extern void RemoveTileSet (TileSetType	*tileset);

extern BOOL SaveTileSpace (
	TileSpaceType	*tilespace,
	char			*path,
	UWORD			flags
);
extern TileSpaceType *CreateTileSpace (void);
extern void *DeleteTileSpace (TileSpaceType *tilespace);
#if 0
extern TileSetType *DuplicateTileSet (TileSetType tileset);
#endif

/******************************** TIOSUP.C ********************************/

extern void	*CreateID_Translate (void);
extern void	DeleteID_Translate (void *id_translator);
extern void PutID_Translate (
	WORD	old_id,
	WORD	new_id,
	void	*id_translator
);
extern WORD GetCurrentID (
	WORD	old_id,
	void	*id_translator
);
extern void TranslateRooms (
	MapType	*map,
	WORD	match_flags,
	void	*id_translator
);

extern UWORD RenumberRooms (MapType *map);
extern void PromoteRooms (MapType *map);
extern void CleanRooms (MapType *map);
extern BOOL AppendorInsertLayersIntoRoom(LayerType *playDst, MapType *pmap, short f);

extern void PromoteTileSets (TileSpaceType	*tilespace);
extern void CleanTileSets (TileSpaceType	*tilespace);

extern RoomType *FindNextSameRoomName(char *sz);
extern RoomType *FindFirstSameRoomName(char *sz, MapType *pmt);

extern RoomType *FindDuplicateRoomName(RoomType *prt, MapType *pmt);

#if 0
extern TileType *FindTile (
	WORD	tileset_id,
	WORD	tilenumber
);
#endif

extern WORD	GetNextTilesetID (TileSpaceType	*tilespace);
extern WORD GetHighestTilesetID (TileSpaceType	*tilespace);

/********************************* TUME.C *********************************/

extern void TumeSetUpFirst (void);
extern void TumeSetUpLast (void);
extern void PreTumeCleanUp (void);
extern void PostTumeCleanUp (void);
extern int NewMap (void);
extern void Quit (char *message);
extern void AutoReqOn (void);
extern void AutoReqOff (void);

/******************************* TUMEDRAW.C *******************************/

extern void SetGlobalZoom(RoomType *prt);

extern void DrawTileOutline(int dx, int dy, int cx, int cy, RoomStuffType *prst,
				int dtilx, int dtily, int ctilx, int ctily,
				UWORD cxTile, UWORD cyTile, int sep, UBYTE eor_mask);

extern void DrawPixelSelect (
	BlockCopyType	*block,
	UWORD		cxTile,
	UWORD		cyTile,
	UBYTE		eor_mask,
	BOOL			destflag
);

extern void DrawSelect (
	BlockCopyType	*block,
	short			ctilx,
	short			ctily,
	UWORD			cxTile,
	UWORD			cyTile,
	UBYTE			eor_mask,
	BOOL			destflag
);

extern void XORDrawPBlk(BlockCopyType *pblk, UBYTE eor_mask, BOOL destflag);

extern void RedrawSelectArea (
	BlockCopyType	*block,
	BOOL			destflag,
	BOOL			fShowIt,
	WORD			showcolumn,
	WORD			showrow
);

extern void RevampTileOverhang(TileSpaceType *ptsp);

extern void ShowRoom (RoomWindowType *roomwindow);
#if __AMIGAOS__
extern void Scroll (
	RoomWindowType	*roomwindow,
	WORD			delta_x,
	WORD			delta_y
);
#elif __MSDOS__
extern void Scroll (
	RoomWindowType	*roomwindow,
	WORD			delta_x,
	WORD			delta_y,
	BOOL			fUpdateSelect
);
#endif/*__AMIGAOS__/__MSDOS__*/

void BigTileCoordsFromSmallTile(ListType *pLayers, LayerType *play,
					int *pdtilx, int *pdtily, int *pctilx, int *pctily);

extern void PlaceBlockCopy (
	BlockCopyType	*block,
	BOOL			image_flag
);


#endif /* TUFUNC_H */

