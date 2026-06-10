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
 * READtUME.C
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 10/16/89 
 *   MODIFIED : 10/19/94
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		Routines to read tUME IFF files.
 *
 * HISTORY
 *		10/16/89 (RGM) - Created.
 *		10/31/89 (RGM) - Made compatible with both io.c & tpackio.c.
 *		08/31/90 (RGM) - Put in UNIX style IO (unbuffered).
 *		01/11/91 (RGM) - Changes having to do with new plots.
 *		02/26/92 (GAT) - Added support for new color info chunks.
 * 	02/26/92 (dcc) - added IO for GUID and GRID chunks.
 * 	03/17/92 (dcc) - added IO for ZOOM chunk.
 * 	12/16/92 (dcc) - add GUID and GRID chunks support for tilesets.
 *		03/17/93 (dcc) - CurrentColorInfo->NumColors always = MAXCOLORREG.
 *		04/01/93 (dcc) - ProcessCmntChunk() & WriteCmntData() added
 *		04/29/93 (dcc) - add support for search-dirs in tUME.INI
 *		05/11/93 (dcc) - add support to load a single layer
 *		08/25/93 (dcc) - add support for fSearchAsSpecified switch
 *		08/13/94 (dcc) - created from old IO.C
 *		08/22/94 (dcc) - add support to read 'LAYR' and 'LAY1' chunks
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
*/

#include "rwtume.h"
#include "readtume.h"				/* Verify function prototypes. */
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray

/****************************** G L O B A L S *****************************/

BOOL	fSearchAsSpecified = FALSE;	/* Search for TSETs as specified in file? */
BOOL	fSearchCurrentDir = FALSE;	/* Add current dir to search list? */

/******************************* L O C A L S ******************************/

static	BOOL			tUME_EOF;

/* If playLoad is != NULL then we are trying to insert layers into a room. */

static	LayerType		*playLoad = NULL;	/* where to insert loaded layer */
static	short		fAppendorLoad;		/* appending or loading layers? */

static	RoomType		*CurrentRoom			= NULL;
static	TileSetType	*CurrentTileSet		= NULL;
static	ColorInfo		*CurrentColorInfo		= NULL;
static	CS_CycleInfo	*CurrentCycleInfo		= NULL;
static	UserInfoType	*CurrentUserInfo		= NULL;
static	char			*CurrentUserComment[2]	= {NULL, NULL};
static	void			*Translator			= NULL;

/* PathList is initialized by IOINI.C with initial search directories */

		ListType		*PathList				= NULL;

static	int			TopRoom;

static	char			szCurrentDir[256];

#if SaveNew
static	RoomBuff1Type	rb1Current;
static	BOOL			fReadCompressed;
static	char			szCurrentRoom[256];
static	BOOL			fCurrentRoomName;
#endif // SaveNew

static	BOOL	AutoSub	= FALSE;

static	short fErrorType;	// was it a S_ERROR or S_LOW_MEM error?

/***************************** R O U T I N E S ****************************/


/**************************************************************************
 *
 * u_getc
 *
 * PURPOSE
 *		get a character from a UNIX type file.
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
 *	08/31/90 Friday (RGM) - Created.
 *
 * SEE ALSO
 *
*/
static int	u_getc (
	FileHandleType		file
)
{
	char	buf;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "u_getc";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	read (file, (void *) &buf, 1);

	return ((int) buf);

} /* u_getc */


/**************************************************************************
**
** GetHeader
**
** USAGE
**	ok = GetHeader (INFILE, &HEADER);
**
** FUNCTION
**	Get the chunk header from an IFF file stream.
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/16/89 Monday - Created. (RGM)
**
** SEE ALSO
**
*/
static BOOL GetHeader (
	FileHandleType		file,
	IFFChunkHeaderType	*header
)
{
	register	bytes_in;
	BOOL		status	= TRUE;

	bytes_in = read (file, (void *) header, IFFCH_SIZE);

	if (bytes_in == 0) {
		tUME_EOF	= TRUE;
	}
	else if (bytes_in == IFFCH_SIZE) {
		LongSex(header->DataSize);
	}
	else {
		status = FALSE;
	}

	return (status);
} /* GetHeader */


/**************************************************************************
**
** FSkipEven
**
** USAGE
**	ok = FSkipEven (INFILE, SIZE);
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/16/89 Monday - Create. (RGM)
**
** SEE ALSO
**
*/
static BOOL FSkipEven (
	FileHandleType	file,
	long			size
)
{
	long	adjusted;

	adjusted = size + (ODD (size) ? 1 : 0);

	return ((BOOL) OK_SEEK (lseek (file, adjusted, SEEK_CUR)));
} /* FSkipEven */


/**************************************************************************
**
** GetFormID
**
** USAGE
**	bytes_read = GetFormID (INFILE, ID_PTR);
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/16/89 Monday - Created. (RGM)
**
** SEE ALSO
**
*/
static unsigned long GetFormID (
	FileHandleType	file,
	unsigned long	*ID_ptr
)
{
	unsigned long status = 0L;

	if (read (file, (void *) ID_ptr, ULONGSIZE) == ULONGSIZE) {
		status = ULONGSIZE;
	}

	return (status);
} /* GetFormID */


/**************************************************************************
 *
 * TS_idFromNameNumberType
 *
 * PURPOSE
 *		Find the tileset with the same name as <szName>, the same
 *		UserType as <UserType>, the same UserNumber as <UserNumber>,
 *		and the same load flags as <Flags>, then
 *		return the TS_id of that matching tileset.
 *
 * INPUT
 *
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns 0 if not found, else tileset ID of matching tileset.
 *
 * HISTORY
 *		11/14/89 (RGM) - Created as MatchTileSetID().
 *		10/01/94 (dcc) - moved here from TIOSUP.C. Renamed function.
 *		10/01/94 (dcc) - compare UserType and UserNumber in addition to name.
 *
 * SEE ALSO
 *
*/
static int TS_idFromNameNumberType (
	char			*szName,
	WORD			 UserType,
	WORD			 UserNumber,
	TileSpaceType	*ptsp
)
{
	TileSetType	*ptst;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TS_idFromNameNumberType";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ptst = Head (&ptsp->TileSets);
	while (! IsEOList (ptst))
	{
		if (	stricmp(BaseName(szName), BaseName(ptst->Filespec)) == 0 &&
			UserType == ptst->UserType &&
			UserNumber == ptst->UserNumber)
		{
			return ptst->TS_id;
		}
		ptst = Next (ptst);
	}
	return 0;

} /* TS_idFromNameNumberType */


/**************************************************************************
**
** ProcessDataRChunk
**
** USAGE
**
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**		10/22/89 (RGM) - Created.
**		08/22/94 (dcc) - add support for UseSparseArray
**
** SEE ALSO
**
*/
static BOOL ProcessDataRChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	unsigned long	count	= 0;
	unsigned long	layers;
#if PLOTARRAY
	unsigned long	layersize;
#else
	UWORD		cbRowsize;
#endif
	LayerType		*layer;
	RoomBuffType	roombuffer;
	char			name[255] = {0, };
	WORD			wName0 = 0;
	BOOL			fCompress;

/**/	if (CurrentRoom) goto ABORT;

	/*******************************/
	/* read in the fixed size info */

	if (read (file, (void *) &roombuffer, ROOMBUFFSIZE) != ROOMBUFFSIZE) {
/**/		goto ABORT;
	}

	WordSex(roombuffer.Flags);
	WordSex(roombuffer.R_id);
	WordSex(roombuffer.Width);
	WordSex(roombuffer.Height);
	WordSex(roombuffer.TileWidth);
	WordSex(roombuffer.TileHeight);
	WordSex(roombuffer.LayerCount);
	WordSex(roombuffer.FloorNumber);

	fCompress = roombuffer.Flags & 1;
	roombuffer.Flags &= 0xFFFE;

	count += ROOMBUFFSIZE;

	/****************************************/
	/* skip ahead and read in the room name */

	if (!fCompress)
	{
		long lOffset = (long) roombuffer.LayerCount * roombuffer.Width *
							roombuffer.Height * PLOTSIZE;

		if (count + lOffset < size)
		{
			long lHere = lseek(file, 0L, SEEK_CUR);

			wName0 = (WORD) (size - count - lOffset);

			if (lHere < 0)
/**/			goto ABORT;

			if (lseek(file, lOffset, SEEK_CUR) < 0)
/**/			goto ABORT;

			if (read (file, (void *) name, wName0) != wName0)
			{
/**/			goto ABORT;
			}
			name [wName0] = 0;

			if (lseek(file, lHere, SEEK_SET) < 0)
/**/			goto ABORT;
		}
		else
		{
			strcpy(name, "(no name)");
			wName0 = 9;
		}
		strcat(name, ":");
		wName0++;
	}

	/*******************/
	/* make a new room */

	strcpy(name+wName0, "creating room");
	ShowStatus(name);

#if UseSparseArray
	CurrentRoom = CreateRoomWithUndo (
						roombuffer.Width,
						roombuffer.Height,
						(UWORD) (roombuffer.Flags | PREROOM | RAWROOM),
						FALSE );
#else // !UseSparseArray
	CurrentRoom = CreateRoomWithUndo (
						roombuffer.Width,
						roombuffer.Height,
						(UWORD) (roombuffer.Flags | PREROOM | RAWROOM) );
#endif // !UseSparseArray

	if (! CurrentRoom) {
/**/	goto ABORT;
	}

	layer	= Head (&CurrentRoom->Layers);

	CurrentRoom->R_id			= roombuffer.R_id + TopRoom;
	layer->cxTile				= roombuffer.TileWidth;
	layer->cyTile				= roombuffer.TileHeight;
	CurrentRoom->LayerCount		= roombuffer.LayerCount;
	CurrentRoom->FloorNumber		= roombuffer.FloorNumber;

	/*********************/
	/* add needed layers */

	if (roombuffer.LayerCount > 1) {
		for (layers = 1; layers < roombuffer.LayerCount; layers++) {

			sprintf(name+wName0, "adding layer %d", layers+1);
			ShowStatus(name);

#if UseSparseArray
			layer = AddClearedLayer(CurrentRoom,
							roombuffer.Width, roombuffer.Height, FALSE);
#else // !UseSparseArray
			layer = AddClearedLayer (
						CurrentRoom,
						roombuffer.Width,
						roombuffer.Height);
#endif // !UseSparseArray

			if (! layer) {
				fErrorType = S_LOW_MEM;
/**/			goto ABORT;
			}
			layer->cxTile = roombuffer.TileWidth;
			layer->cyTile = roombuffer.TileHeight;
		}
	}

	/**************************/
	/* put in the right floor */

	layers = 0;
	layer = Head (&CurrentRoom->Layers);

	while (! IsEOList (layer)) {
		layers ++;
		if (layers == CurrentRoom->FloorNumber) {
			CurrentRoom->FloorLayer = layer;
		}
		layer = Next (layer);
	}

	/****************************************/
	/* read in the room data layer by layer */

	if (fCompress)
		InitSDQRead();

	layer	= Head (&CurrentRoom->Layers);
#if PLOTARRAY
	layersize = CurrentRoom->Width * CurrentRoom->Height * PLOTSIZE;
#else
	cbRowsize = layer->ctilx * PLOTSIZE;
#endif

	for (layers = 1; layers <= roombuffer.LayerCount; layers++) {
#if PLOTARRAY
		PlotType	*plot;
#else
		WORD		iyt;
#endif
		long		in_count	= 0L;

		sprintf(name+wName0, "reading layer %d", layers);
		ShowStatus(name);

#if PLOTARRAY
		plot = layer->Plot;

		if (fCompress)
		{
			in_count	= SDQRead (file, (void *) plot, layersize);
			if (in_count == -1) {
/**/			goto ABORT;
			}
		}
		else
		{
			in_count	= read (file, (void *) plot, layersize);
			if (in_count != layersize) {
/**/			goto ABORT;
			}
		}
#else
		for (iyt = 0; iyt < layer->ctily; iyt++)
		{
#if __MSDOS__
			WORD ixt;
#endif/*__MSDOS__*/
			WORD cBytes;

			PlotType *pPt;

#if UseSparseArray
			pPt = ActivatePlotRowatWin(layer->p.rgrgplt, iyt, winDst);
#else // !UseSparseArray
			pPt = ActivatePlotRowatWin(layer->rgrgplt, iyt, winDst);
#endif // !UseSparseArray

			if (fCompress)
				cBytes = SDQRead(file, (void *) pPt, cbRowsize);
			else
				cBytes = read(file, (void *) pPt, cbRowsize);

#if __MSDOS__
			for (ixt = 0; ixt < layer->ctilx; ixt++)
			{
				WordSex(pPt[ixt].Tile_ID);
			}
#endif/*__MSDOS__*/

#if UseSparseArray
			UpdatePlotRow(layer->p.rgrgplt, iyt);
#else // !UseSparseArray
			UpdatePlotRow(layer->rgrgplt, iyt);
#endif // !UseSparseArray

			if (fCompress)
			{
				if (cBytes == -1)
				{
/**/				goto ABORT;
				}
			}
			else
			{
				if (cBytes != cbRowsize)
				{
/**/				goto ABORT;
				}
			}
			in_count += cBytes;
		}
#endif/* PLOTARRAY */

#if __MSDOS__
#if PLOTARRAY
		{
			PlotType	*iplot;
			PlotType	*maxiplot;

			iplot		= plot;
			maxiplot	= iplot;
			maxiplot	+= (layersize / PLOTSIZE);

			for ( ; iplot < maxiplot; iplot++) {
				WordSex(iplot->Tile_ID);
			}
		}
#endif
#endif/*__MSDOS__*/
		count += in_count;
		layer = Next (layer);
	}

	/*************************/
	/* read in the room name */

	if (count < size) {
		UWORD dif;

		dif = (UWORD) (size - count);

		if (read (file, (void *) name, dif) != dif) {
/**/		goto ABORT;
		}
		name [dif] = 0;

		count = size;
		CurrentRoom->Name = MEM_calloc (1, strlen (name) + 1);
		if (CurrentRoom->Name) {
			strcpy (CurrentRoom->Name, name);
		}
		ShowStatus(name);
	}
	else {
		ShowStatus("(no name)");
	}

/**/	if (count > size) goto ABORT;

	return (TRUE);

/*========================================================================*/
ABORT:
	if (CurrentRoom) {
		DeAllocateRoom (CurrentRoom);
		CurrentRoom = NULL;
	}

	return (FALSE);
} /* ProcessDataRChunk */


#if SaveNew
/*********************************************************************
 *
 * ProcessROOMNAMEChunk
 *
 * PURPOSE
 *		Process a 'ROOM''NAME' chunk. Just read the information into
 *		<rb1Current>, <fReadCompressed>, <szCurrentRoom> and
 *		<fCurrentRoomName>.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns TRUE if successful.
 *
 * HISTORY
 *		08/14/94 Sunday (dcc) - created from ProcessDataRChunk().
 *
*/
static BOOL ProcessROOMNAMEChunk(FileHandleType file, unsigned long size)
{
	unsigned long	count = 0;
	WORD			wName0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessROOMNAMEChunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CurrentRoom)
		return FALSE;

	/*******************************/
	/* read in the fixed size info */

	if (read (file, (void *) &rb1Current, ROOMBUFF1SIZE) != ROOMBUFF1SIZE)
		return FALSE;

	BigWord2Native(rb1Current.Flags);
	BigWord2Native(rb1Current.R_id);
	BigWord2Native(rb1Current.LayerCount);
	BigWord2Native(rb1Current.FloorNumber);

	fReadCompressed = rb1Current.Flags & 1;
	rb1Current.Flags &= 0xFFFE;

	count += ROOMBUFF1SIZE;

	/* read in the room name */

	wName0 = (WORD) (size - count);

	if (wName0 < 0)
		return FALSE;

	szCurrentRoom[wName0] = 0;
	if (wName0 > 0)
	{
		if (read (file, szCurrentRoom, wName0) != wName0)
			return FALSE;

		count += wName0;
		fCurrentRoomName = TRUE;
	}
	else
	{
		strcpy(szCurrentRoom, "(no name)");
		fCurrentRoomName = FALSE;
	}
	if (count > size)
		return FALSE;

	return TRUE;
} /* ProcessROOMNAMEChunk */


/*********************************************************************
 *
 * MakeNewLoadRoom
 *
 * PURPOSE
 *		Make a new room based on information from 'ROOM''NAME' chunk
 *		(stored in <rb1Current> and <szCurrentRoom>) and information
 *		from the first 'ROOM''LAYn' chunk (which hopefully is info
 *		for the first layer).
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		The 'LAYR' and 'LAY1' chunks are in order from layer 1
 *		to the topmost layer.
 *
 *	NOTES
 *		The first layer's ->cxTile and ->cyTile are NOT set on exit.
 *
 * RETURN VALUE
 *		Returns TRUE if successful.
 *
 * HISTORY
 *		08/15/94 (dcc) - created.
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
*/
static BOOL MakeNewLoadRoom(LayerBuffType *plyb)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MakeNewLoadRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CurrentRoom)
		return FALSE;

	{
	char szShow[256];

	strcpy(szShow, szCurrentRoom);
	strcat(szShow, ":creating room");
	ShowStatus(szShow);
	}

	/*******************/
	/* make a new room */

#if UseSparseArray
	CurrentRoom = CreateRoomWithUndo(plyb->ctilx, plyb->ctily,
								rb1Current.Flags | PREROOM | RAWROOM,
								TRUE);
#else // !UseSparseArray
	CurrentRoom = CreateRoomWithUndo(plyb->ctilx, plyb->ctily,
								rb1Current.Flags | PREROOM | RAWROOM);
#endif // !UseSparseArray

	if (!CurrentRoom)
		return FALSE;

	CurrentRoom->R_id			= rb1Current.R_id + TopRoom;
	CurrentRoom->LayerCount		= rb1Current.LayerCount;
	CurrentRoom->FloorNumber		= rb1Current.FloorNumber;

	if (fCurrentRoomName)
	{
		if ((CurrentRoom->Name = MEM_calloc(1, strlen(szCurrentRoom)+1)) == NULL)
/**/		goto ABORT;

		strcpy(CurrentRoom->Name, szCurrentRoom);
	}
	return TRUE;

/*========================================================================*/
ABORT:
	if (CurrentRoom)
	{
		DeAllocateRoom (CurrentRoom);
		CurrentRoom = NULL;
	}
	return FALSE;
} /* MakeNewLoadRoom */


/*********************************************************************
 *
 * VerifyGoodLayer
 *
 * PURPOSE
 *		Make sure the the current room has a layer <plyb->LayerNumber>
 *		and that the layer is of size <ctilx>, <ctily>.
 *
 *		If the layer does not exist, then make the layer.
 *		If it is the wrong size, then re-size the layer.
#if UseSparseArray
 *		If the layer exists but is not the correct sparse/non-sparse
 *		type, then re-make the layer.
#endif // UseSparseArray
 *
 *		Also if <plyb->LayerNumber> == <rb1Current.FloorNumber>,
 *		then set CurrentRoom->FloorLayer to point at the proper layer.
 *
 * INPUT
 *		LayerBuffType *plyb	: linked list of layers to check
#if UseSparseArray
 *		BOOL fMakeSparse	: if FALSE <=> then make a 2-D array
 *							TRUE <=> then make a sparse array
#endif // UseSparseArray
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns correct layer if successful, otherwise return NULL.
 *
 * HISTORY
 *		08/15/94 (dcc) - created.
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
*/
#if UseSparseArray
static LayerType *VerifyGoodLayer(LayerBuffType *plyb, BOOL fMakeSparse)
#else // !UseSparseArray
static LayerType *VerifyGoodLayer(LayerBuffType *plyb)
#endif // !UseSparseArray
{
	LayerType	*play;
	int		 ilay;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "VerifyGoodLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!CurrentRoom)
		return NULL;

	play = Head(&CurrentRoom->Layers);
	ilay = 1;

	while (ilay != plyb->LayerNumber)
	{
		play = Next(play);
		if (IsEOList(play))
		{
#if UseSparseArray
			if ((play = AddClearedLayer(CurrentRoom, plyb->ctilx, plyb->ctily, fMakeSparse)) == NULL)
#else // !UseSparseArray
			if ((play = AddClearedLayer(CurrentRoom, plyb->ctilx, plyb->ctily)) == NULL)
#endif // !UseSparseArray
				return NULL;

			play->dtilx = 0;
			play->dtily = 0;
		}
		ilay++;
	}
#if UseSparseArray
	// see if the layer is what is requested by <fMakeSparse>

	if ((fMakeSparse != FALSE) != (IsSparse(play) != FALSE))
	{
		LayerType *playNext;
		playNext = Next(play);
		Remove(play);
		DeAllocateLayer(play);
		if (IsEOList(playNext))
		{
			if ((play = AddClearedLayer(CurrentRoom, plyb->ctilx, plyb->ctily, fMakeSparse)) == NULL)
				return NULL;
		}
		else
		{
			if ((play = InsertClearedLayerToLayers(CurrentRoom, play,
									plyb->ctilx, plyb->ctily, fMakeSparse)) == NULL)
				return NULL;
		}
		play->dtilx = 0;
		play->dtily = 0;
	}
#endif // UseSparseArray

	/* Make sure the layer is the correct size */

	if (play->ctilx != plyb->ctilx || play->ctily != plyb->ctily)
	{
#if UseSparseArray
		if (!IsSparse(play))
			if (plyb->ctilx > play->ctilx || plyb->ctily > play->ctily)
				if (!ExpandPlot(&play->p.rgrgplt, plyb->ctilx, plyb->ctily))
					return NULL;
#else // !UseSparseArray
		if (plyb->ctilx > play->ctilx || plyb->ctily > play->ctily)
			if (!ExpandPlot(&play->rgrgplt, plyb->ctilx, plyb->ctily))
				return NULL;
#endif // !UseSparseArray

		play->ctilx = plyb->ctilx;
		play->ctily = plyb->ctily;
	}
	play->cxTile = plyb->cxTile;
	play->cyTile = plyb->cyTile;

	if (ilay == CurrentRoom->FloorNumber)
		CurrentRoom->FloorLayer = play;

	return play;
} /* VerifyGoodLayer */


/*********************************************************************
 *
 * ProcessROOMLAYRChunk
 *
 * PURPOSE
 *		Process a 'ROOM''LAYR' chunk.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns TRUE if successful.
 *
 * HISTORY
 *		08/15/94 (dcc) - created.
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
*/
static BOOL ProcessROOMLAYRChunk(FileHandleType file, unsigned long size)
{
	unsigned long	ulCount = 0;
	LayerBuffType lyb;
	LayerType *play;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessROOMLAYRChunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Read in the fixed size info */

	if (read (file, (void *) &lyb, LAYERBUFFSIZE) != LAYERBUFFSIZE)
/**/	goto ABORT;

	BigWord2Native(lyb.LayerType);
	BigWord2Native(lyb.LayerNumber);
	BigWord2Native(lyb.ctilx);
	BigWord2Native(lyb.ctily);
	BigWord2Native(lyb.cxTile);
	BigWord2Native(lyb.cyTile);

	ulCount += LAYERBUFFSIZE;

	/* If room has not been created yet, now is the time to do it... */

	if (!CurrentRoom)
		if (!MakeNewLoadRoom(&lyb))
/**/		goto ABORT;

#if UseSparseArray
	if ((play = VerifyGoodLayer(&lyb, FALSE)) == NULL)
#else // !UseSparseArray
	if ((play = VerifyGoodLayer(&lyb)) == NULL)
#endif // !UseSparseArray
/**/	goto ABORT;

	/**************************/
	/* read in the room layer */

	if (fReadCompressed)
		InitSDQRead();

#if PLOTARRAY
#error Not implemented!
#endif

	{
	char szShow[256];

	sprintf(szShow, "%s:reading layer %d", szCurrentRoom, lyb.LayerNumber);
	ShowStatus(szShow);
	}

	{
		int		iyt;
		unsigned	cbRowsize;

		cbRowsize = play->ctilx * PLOTSIZE;

		for (iyt = 0; iyt < play->ctily; iyt++)
		{
#if __LITTLEENDIAN__
			int ixt;
#endif/*__LITTLEENDIAN__*/
			int cBytes;

			PlotType *pPt;

#if UseSparseArray
			pPt = ActivatePlotRowatWin(play->p.rgrgplt, iyt, winDst);
#else // !UseSparseArray
			pPt = ActivatePlotRowatWin(play->rgrgplt, iyt, winDst);
#endif // !UseSparseArray

			if (fReadCompressed)
				cBytes = SDQRead(file, (void *) pPt, cbRowsize);
			else
				cBytes = read(file, (void *) pPt, cbRowsize);

#if __LITTLEENDIAN__
			for (ixt = 0; ixt < play->ctilx; ixt++)
			{
				BigWord2Native(pPt[ixt].Tile_ID);
			}
#endif/*__LITTLEENDIAN__*/

#if UseSparseArray
			UpdatePlotRow(play->p.rgrgplt, iyt);
#else // !UseSparseArray
			UpdatePlotRow(play->rgrgplt, iyt);
#endif // !UseSparseArray

			if (fReadCompressed)
			{
				if (cBytes == -1)
				{
/**/				goto ABORT;
				}
			}
			else
			{
				if (cBytes != cbRowsize)
				{
/**/				goto ABORT;
				}
			}
			ulCount += cBytes;
		}
	}
	if (size != ulCount)
/**/	goto ABORT;

	return TRUE;

/*========================================================================*/
ABORT:
	if (CurrentRoom)
	{
		DeAllocateRoom (CurrentRoom);
		CurrentRoom = NULL;
	}

	return FALSE;
} /* ProcessROOMLAYRChunk */


/*********************************************************************
 *
 * ProcessROOMLAY1Chunk
 *
 * PURPOSE
 *		Process a 'ROOM''LAY1' chunk. Convert into a normal layer.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns TRUE if successful.
 *
 * HISTORY
 *		08/15/94 (dcc) - created
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
*/
static BOOL ProcessROOMLAY1Chunk(FileHandleType file, unsigned long size)
{
	ULONG ulCount = 0;
	ULONG ulTiles;
	LayerBuffType lyb;
	LayerType *play;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessROOMLAY1Chunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Read in the fixed size info */

	if (read (file, (void *) &lyb, LAYERBUFFSIZE) != LAYERBUFFSIZE)
/**/	goto ABORT;

	BigWord2Native(lyb.LayerType);
	BigWord2Native(lyb.LayerNumber);
	BigWord2Native(lyb.ctilx);
	BigWord2Native(lyb.ctily);
	BigWord2Native(lyb.cxTile);
	BigWord2Native(lyb.cyTile);

	ulCount += LAYERBUFFSIZE;

	if (read (file, (void *) &ulTiles, sizeof(ULONG)) != sizeof(ULONG))
/**/	goto ABORT;

	BigLong2Native(ulTiles);

	ulCount += sizeof(ULONG);

	/* If room has not been created yet, now is the time to do it... */

	if (!CurrentRoom)
		if (!MakeNewLoadRoom(&lyb))
/**/		goto ABORT;

#if UseSparseArray
	if ((play = VerifyGoodLayer(&lyb, TRUE)) == NULL)
#else // !UseSparseArray
	if ((play = VerifyGoodLayer(&lyb)) == NULL)
#endif // !UseSparseArray
/**/	goto ABORT;

	/**************************/
	/* read in the room layer */

	if (fReadCompressed)
		InitSDQRead();

#if PLOTARRAY
#error Not implemented!
#endif

	{
	char szShow[256];

	sprintf(szShow, "%s:reading layer %d", szCurrentRoom, lyb.LayerNumber);
	ShowStatus(szShow);
	}

	while (ulTiles--)
	{
		int cBytes;
		XYTileBuffType xyt;

		if (fReadCompressed)
		{
			if ((cBytes = SDQRead(file, (void *) &xyt, XYTILEBUFFSIZE)) == -1)
/**/			goto ABORT;
		}
		else
		{
			if ((cBytes = read (file, (void *) &xyt, XYTILEBUFFSIZE)) != XYTILEBUFFSIZE)
/**/			goto ABORT;
		}

		BigWord2Native(xyt.dtilx);
		BigWord2Native(xyt.dtily);
		BigWord2Native(xyt.plt.Tile_ID);

		ulCount += cBytes;

#if UseSparseArray
		WriteSparsePlotXY(&xyt.plt, play->p.pspa, xyt.dtilx, xyt.dtily);
#else // !UseSparseArray
		WriteNPlotXY(&xyt.plt, 1, play->rgrgplt, xyt.dtilx, xyt.dtily);
#endif // !UseSparseArray
	}
	if (size != ulCount)
/**/	goto ABORT;

	return TRUE;

/*========================================================================*/
ABORT:
	if (CurrentRoom)
	{
		DeAllocateRoom (CurrentRoom);
		CurrentRoom = NULL;
	}

	return FALSE;
} /* ProcessROOMLAY1Chunk */
#endif // SaveNew


/**************************************************************************
**
** ProcessUserRChunk
**
** USAGE
**
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/31/89 Tuesday - Created (on Halloween). (RGM)
**
** SEE ALSO
**
*/
static BOOL ProcessUserRChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	unsigned long	count	= 0;

/**/	if (! CurrentUserInfo) goto ABORT;

	/*****************************************/
	/* read in the first user info (4 words) */

	if (read (file, (void *) CurrentUserInfo, USERINFOSIZE) != USERINFOSIZE){
/**/	goto ABORT;
	}

	WordSex(CurrentUserInfo->UserType);
	WordSex(CurrentUserInfo->UserNumber);
	WordSex(CurrentUserInfo->UserExtra1);
	WordSex(CurrentUserInfo->UserExtra2);

	count += USERINFOSIZE;

/**/	if (count > size) goto ABORT;

	if (count < size) {
		FSkipEven (file, size - count);
		count = size;
	}
	return (TRUE);

/*========================================================================*/
ABORT:
	return (FALSE);
} /* ProcessUserRChunk */


/*********************************************************************
 *
 * ProcessCmntChunk
 *
 * PURPOSE
 *		Read room comments from CMNT chunk in tUME file.
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
 *		04/01/93 Thursday (dcc)	- created.
 *
 * SEE ALSO
 *
*/
static BOOL ProcessCmntChunk(FileHandleType file, unsigned long size)
{
	unsigned long	count	= 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessCmntChunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

/**/if (! CurrentUserInfo) goto ABORT;

	if (size) {
		WORD i;
		for (i = 0; i < 2; i ++) {
			UWORD comment_size;

			if (read (file, (void *) &comment_size, sizeof (UWORD)) != sizeof (UWORD)){
/**/			goto ABORT;
			}

			WordSex(comment_size);	/*dcc*/

			count += sizeof (UWORD);
			if (comment_size) {
				CurrentUserComment[i]	= MEM_calloc (1, comment_size + 1);

				if (CurrentUserComment[i]) {
					if (read (file, (void *) CurrentUserComment[i], comment_size) != comment_size) {
/**/					goto ABORT;
					}
					count += comment_size;
					CurrentUserComment[i][comment_size] = 0;
				}
				else {
					fErrorType = S_LOW_MEM;
/**/				goto ABORT;
				}
			}
		}
	}

	if (count < size) {
		FSkipEven (file, size - count);
		count = size;
	}
	return (TRUE);

/*========================================================================*/
ABORT:
	return (FALSE);
} /* ProcessCmntChunk */


/**************************************************************************
**
** ProcessCmapChunk
**
** USAGE
**
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/22/89 Sunday - Created. (RGM)
**	02/26/92 Wednesday - Modified. (GAT)
**
** SEE ALSO
**
*/
static BOOL ProcessCmapChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	ColorRegister	in_color;
	CS_Color		*color_ptr, *max_color_ptr;

	/**************************/
	/* fill in the CMAP stuff */

	color_ptr		 = ActivateXTRA (CurrentColorInfo->pxtColors);
	max_color_ptr	 = color_ptr;
	max_color_ptr	+= (UWORD) (size / COLORREGSIZE);
	for ( ; color_ptr < max_color_ptr; color_ptr ++) {
		/****************************/
		/* read in the color values */

		if (read (file, (void *) &in_color, COLORREGSIZE) != COLORREGSIZE) {
/**/		goto ABORT;
		}

		color_ptr->Red   = in_color.red;
		color_ptr->Green = in_color.green;
		color_ptr->Blue  = in_color.blue;

		color_ptr->Hue   = 0;
		ConvertRGBtoHSV (color_ptr);
	}

	CurrentColorInfo->NumColors = MAXCOLORREG; //size / COLORREGSIZE;

	UpdateXTRA (CurrentColorInfo->pxtColors);

	return (TRUE);
/*========================================================================*/
ABORT:
	UpdateXTRA (CurrentColorInfo->pxtColors);

	return (FALSE);
} /* ProcessCmapChunk */


/**************************************************************************
**
** ProcessHsvpChunk
**
** USAGE
**
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	02/26/92 Wednesday - Created. (GAT)
**
** SEE ALSO
**
*/
static BOOL ProcessHsvpChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	HSVType		in_color;
	CS_Color		*color_ptr, *max_color_ptr;

	/**************************/
	/* fill in the Hsvp stuff */

	color_ptr		 = ActivateXTRA (CurrentColorInfo->pxtColors);
	max_color_ptr	 = color_ptr;
	max_color_ptr	+= (UWORD) (size / HSVSIZE);
	for ( ; color_ptr < max_color_ptr; color_ptr ++) {
		/****************************/
		/* read in the color values */

		if (read (file, (void *) &in_color, HSVSIZE) != HSVSIZE) {
/**/		goto ABORT;
		}

		WordSex (in_color.Hue);

		color_ptr->Hue        = in_color.Hue;
		color_ptr->Saturation = in_color.Saturation;
		color_ptr->Value      = in_color.Value;
	}
	UpdateXTRA (CurrentColorInfo->pxtColors);

	return (TRUE);
/*========================================================================*/
ABORT:
	UpdateXTRA (CurrentColorInfo->pxtColors);

	return (FALSE);
}	/* ProcessHsvpChunk */


/**************************************************************************
**
** ProcessCflgChunk
**
** USAGE
**
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/22/89 Sunday - Created. (RGM)
**
** SEE ALSO
**
*/
static BOOL ProcessCflgChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	CycleFlagType	in_cycleflag;
	unsigned long	count	= 0;

	if (read (file, (void *) &in_cycleflag, CYCLEFLAGSIZE) != CYCLEFLAGSIZE){
/**/	goto ABORT;
	}

	WordSex(in_cycleflag);

	count += CYCLEFLAGSIZE;
/**/	if (count != size) goto ABORT;
	CurrentColorInfo->CycleFlag = in_cycleflag;
	return (TRUE);
/*========================================================================*/
ABORT:
	return (FALSE);
}	/* ProcessCflgChunk */


/**************************************************************************
**
** ProcessCyclChunk
**
** USAGE
**
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/22/89 Sunday - Created. (RGM)
**	02/26/92 Wednesday - Modified, turns data into CINF chunk. (GAT)
** 04/08/92 Wednesday (dcc) - bug fix: set cycle_info->NumRegs & ->NumColors.
**
** SEE ALSO
**
*/
static BOOL ProcessCyclChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	unsigned long		count		= 0;
	char				regs[CIARRAYSIZE];
	ColorRegister		in_color;
	CS_CycleInfo		*cycle_info;
	CS_ColorTracker	*cycle_color;
	WORD				colors, maxcolors;
	OtherCIinfoType	other;


	if ( (cycle_info = CreateCycleInfo ()) == NULL)
/**/	goto ABORT;

	if (read (file, (void *) regs, CIARRAYSIZE) != CIARRAYSIZE)
/**/	goto ABORT;

	count += CIARRAYSIZE;

	{
		CS_RegTracker		*csrt;
		char		*r;
		short	 i;

		r = regs;
		for (i = 0; ((i < 32) && (*r >= 0)); i++, r++) {
			csrt = CreateRegTracker ();
			if (!csrt) {
/**/			goto ABORT;
			}
			csrt->Reg = *r;
			AddTail (cycle_info->RegList, csrt);
		}
		cycle_info->NumRegs = i;
	}

	if (read (file, (void *) &other, OTHERSIZE) != OTHERSIZE)
/**/	goto ABORT;

	WordSex(other.Speed);
	WordSex(other.Direction);
	WordSex(other.Flag);

	count += OTHERSIZE;

	cycle_info->Speed		= other.Speed;
	cycle_info->Direction	= other.Direction;
	cycle_info->Flags		= other.Flag;

	maxcolors = (WORD) ((size - count) / COLORREGSIZE);
	for (colors = 0; colors < maxcolors; colors ++)
	{
		if (read (file, (void *) &in_color, COLORREGSIZE) != COLORREGSIZE)
/**/		goto ABORT;

		if ( (cycle_color = CreateCycleColor ()) == NULL)
/**/		goto ABORT;

		cycle_color->Color.Red   = in_color.red;
		cycle_color->Color.Green = in_color.green;
		cycle_color->Color.Blue  = in_color.blue;
		cycle_color->Color.Hue   = 0;

		ConvertRGBtoHSV (&cycle_color->Color);

		AddTail (cycle_info->ColorList, cycle_color);
	}
	cycle_info->NumColors = maxcolors;
	count += COLORREGSIZE * maxcolors;
	if (count != size)
/**/	goto ABORT;

	AddTail (CurrentColorInfo->CycleList, cycle_info);
	CurrentColorInfo->NumCycles++;

	return (TRUE);
/*========================================================================*/
ABORT:

	if (cycle_info) {
		DeleteCycleInfo (cycle_info);
		cycle_info = NULL;
	}
	return (FALSE);
}	/* ProcessCyclChunk */


/**************************************************************************
**
** ProcessCinfChunk
**
** USAGE
**
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	02/26/92 Wednesday - Created. (GAT)
**
** SEE ALSO
**
*/
static BOOL ProcessCinfChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	unsigned long		count		= 0;
	CS_CycleInfo		*cycle_info;
	CS_ColorTracker	*cycle_color;
	CS_RegTracker		*cycle_reg;
	CINFType			cinf;
	CINFColorType		cinfcolor;
	UWORD			reg;
	short			i;

	if ( (cycle_info = CreateCycleInfo ()) == NULL)
/**/	goto ABORT;

	if (read (file, (void *) &cinf, CINFSIZE) != CINFSIZE)
/**/	goto ABORT;

	WordSex (cinf.Speed);
	WordSex (cinf.Direction);
	WordSex (cinf.Flags);
	WordSex (cinf.NumColors);
	WordSex (cinf.NumRegs);

	cycle_info->Speed     = cinf.Speed;
	cycle_info->Direction = cinf.Direction;
	cycle_info->Flags     = cinf.Flags;
	cycle_info->NumColors = cinf.NumColors;
	cycle_info->NumRegs   = cinf.NumRegs;

	count += CINFSIZE;

	for (i = 0; i < cinf.NumColors; i++)
	{
		if (read (file, (void *) &cinfcolor, CINFCOLORSIZE) != CINFCOLORSIZE){
/**/		goto ABORT;
		}

		WordSex (cinfcolor.Hue);

		cycle_color = CreateCycleColor ();
		if (!cycle_color) {
/**/		goto ABORT;
		}

		cycle_color->Color.Red        = cinfcolor.Red;
		cycle_color->Color.Green      = cinfcolor.Green;
		cycle_color->Color.Blue       = cinfcolor.Blue;
		cycle_color->Color.Hue        = cinfcolor.Hue;
		cycle_color->Color.Saturation = cinfcolor.Saturation;
		cycle_color->Color.Value      = cinfcolor.Value;

		AddTail (cycle_info->ColorList, cycle_color);

		count += CINFCOLORSIZE;
	}

	for (i = 0;  i < cinf.NumRegs; i++) {

		if (read (file, (void *) &reg, sizeof (UWORD)) != sizeof (UWORD)) {
/**/		goto ABORT;
		}

		WordSex (reg);

		cycle_reg = CreateRegTracker ();
		if (!cycle_reg) {
/**/		goto ABORT;
		}

		cycle_reg->Reg = reg;
		AddTail (cycle_info->RegList, cycle_reg);

		count += sizeof (UWORD);
	}

/**/	if (count != size) goto ABORT;

	AddTail (CurrentColorInfo->CycleList, cycle_info);
	CurrentColorInfo->NumCycles++;

	return (TRUE);
/*========================================================================*/
ABORT:

	if (cycle_info) {
		DeleteCycleInfo (cycle_info);
		cycle_info = NULL;
	}
	return (FALSE);
}	/* ProcessCinfChunk */


/*********************************************************************
 *
 * ReadGridChunk
 *
 * PURPOSE
 *		Read room grid info.
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
static BOOL ReadGridChunk(FileHandleType file, unsigned long size,
						GridInfoChunkType *pgic)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReadGridChunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (read(file, pgic, GRIDINFOSIZE) != GRIDINFOSIZE)
	{
/**/	goto ABORT;
	}

	WordSex(pgic->wXWidth);
	WordSex(pgic->wYHeight);
	WordSex(pgic->wXOrigin);
	WordSex(pgic->wYOrigin);

/**/	if (GRIDINFOSIZE > size) goto ABORT;

	if (GRIDINFOSIZE < size)
	{
		FSkipEven (file, size - GRIDINFOSIZE);
	}
	return (TRUE);

/*========================================================================*/
ABORT:
	return (FALSE);
} /* ReadGridChunk */


/*********************************************************************
 *
 * ReadGuidChunk
 *
 * PURPOSE
 *		Read room guide info.
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
#define ReadGuidChunk(file,size,pgic) ReadGridChunk(file,size,pgic)


/*********************************************************************
 *
 * ProcessGridChunk
 *
 * PURPOSE
 *		Stuff grid info <pgic> into <prt>.
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
static BOOL ProcessGridChunk(RoomType *prt, GridInfoChunkType *pgic)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessGridChunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prt && pgic)
	{
		prt->wGridXWidth	= (pgic->wXWidth > 0) ? pgic->wXWidth : 1;
		prt->wGridYHeight	= (pgic->wYHeight > 0) ? pgic->wYHeight : 1;
		prt->wGridXOrigin	= pgic->wXOrigin;
		prt->wGridYOrigin	= pgic->wYOrigin;
		ResetGridOn(prt);
		if (pgic->bActive)
			SetGridOn(prt);

		return TRUE;
	}
	return FALSE;
} /* ProcessGridChunk */


/*********************************************************************
 *
 * ProcessGuidChunk
 *
 * PURPOSE
 *		Stuff guide info <pgic> into <prt>.
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
static BOOL ProcessGuidChunk(RoomType *prt, GridInfoChunkType *pgic)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessGuidChunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prt && pgic)
	{
#if NEWTUME
		if (!ReAllocateRoomGuides(prt, 2))
			return FALSE;			/* OOM */

		prt->pcenGuideA[0] = cenDefaultGuideA0Init;
		prt->pcenGuideA[1] = cenDefaultGuideA1Init;
		prt->pcenGuideB[0] = cenDefaultGuideB0Init;
		prt->pcenGuideB[1] = cenDefaultGuideB1Init;
		prt->pcenGuideDC[0]	= 100L * ((pgic->wYHeight > 0) ? pgic->wYHeight : 1);
		prt->pcenGuideDC[1]	= 100L * ((pgic->wXWidth > 0) ? pgic->wXWidth : 1);
		prt->pcenGuideXOrigin[0]	= 100L * pgic->wXOrigin;
		prt->pcenGuideXOrigin[1]	= 100L * pgic->wXOrigin;
		prt->pcenGuideYOrigin[0]	= 100L * pgic->wYOrigin;
		prt->pcenGuideYOrigin[1]	= 100L * pgic->wYOrigin;
#else
		prt->wGuideXWidth	= (pgic->wXWidth > 0) ? pgic->wXWidth : 1;
		prt->wGuideYHeight	= (pgic->wYHeight > 0) ? pgic->wYHeight : 1;
		prt->wGuideXOrigin	= pgic->wXOrigin;
		prt->wGuideYOrigin	= pgic->wYOrigin;
#endif
		ResetGuideVisible(prt);
		if (pgic->bActive)
			SetGuideVisible(prt);

#if NEWTUME
		Assert((prt->fShowGuide[0] != 0) == (IsGuideVisible(prt) != 0));
		Assert((prt->fShowGuide[1] != 0) == (IsGuideVisible(prt) != 0));
#endif

		return TRUE;
	}
	return FALSE;
} /* ProcessGuidChunk */


#if NEWTUME
/*********************************************************************
 *
 * ReadGuideChunk
 *
 * PURPOSE
 *		Read room guide info.
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
 *		09/28/93 Tuesday (dcc) - created (based on ReadGridChunk()).
 *
 * SEE ALSO
 *
*/
static BOOL ReadGuideChunk(FileHandleType file, unsigned long size,
						GuideInfoChunk **ppGuide, short *pcCount)
{
	ldiv_t l;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReadGuideChunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	l = ldiv(size, sizeof(GuideInfoChunk));

	if (l.rem != 0)
		return FALSE;

	*pcCount = (short) l.quot;

	if ((*ppGuide = MEM_calloc(*pcCount, sizeof(GuideInfoChunk))) == NULL)
		return FALSE;

	if (read(file, *ppGuide, (size_t) size) != size)
	{
		MEM_free(*ppGuide);
		return FALSE;
	}

#if __MSDOS__
	{
		short i;

		for (i = 0; i < *pcCount; i++)
		{
			LongSex((*ppGuide)[i].GuideA);
			LongSex((*ppGuide)[i].GuideB);
			LongSex((*ppGuide)[i].GuideDC);
			LongSex((*ppGuide)[i].GuideXOrigin);
			LongSex((*ppGuide)[i].GuideYOrigin);
		}
	}
#endif

	return TRUE;
} /* ReadGuideChunk */


/*********************************************************************
 *
 * ProcessGuideChunk
 *
 * PURPOSE
 *		Stuff guide info <pGuide> into <prt>.
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
 *		09/28/93 Tuesday (dcc) - created (based on ProcessGuidChunk()).
 *
 * SEE ALSO
 *
*/
static BOOL ProcessGuideChunk(RoomType *prt, GuideInfoChunk *pGuide, short cGuide)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessGuideChunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prt && pGuide)
	{
		short i;

		if (!ReAllocateRoomGuides(prt, cGuide))
			return FALSE;			/* OOM */

		for (i = 0; i < cGuide; i++)
		{
			prt->pcenGuideA[i] = pGuide[i].GuideA;
			prt->pcenGuideB[i] = pGuide[i].GuideB;
			prt->pcenGuideDC[i] = pGuide[i].GuideDC;
			prt->pcenGuideXOrigin[i] = pGuide[i].GuideXOrigin;
			prt->pcenGuideYOrigin[i] = pGuide[i].GuideYOrigin;
			prt->fShowGuide[i] = pGuide[i].ShowGuide;
		}
		if (prt->fShowGuide[0] || prt->fShowGuide[1])
			SetGuideVisible(prt);

#if NEWTUME
		Assert((prt->fShowGuide[0] != 0) == (IsGuideVisible(prt) != 0));
		Assert((prt->fShowGuide[1] != 0) == (IsGuideVisible(prt) != 0));
#endif

		return TRUE;
	}
	return FALSE;
} /* ProcessGuideChunk */
#endif


/*********************************************************************
 *
 * ReadZoomChunk
 *
 * PURPOSE
 *		Read zoom info.
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
static BOOL ReadZoomChunk(FileHandleType file, unsigned long size,
						ZoomInfoChunkType *pzic)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReadZoomChunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (read(file, pzic, ZOOMINFOSIZE) != ZOOMINFOSIZE)
	{
/**/	goto ABORT;
	}

	WordSex(pzic->wDstDup);
	WordSex(pzic->wSrcSkip);

/**/	if (ZOOMINFOSIZE > size) goto ABORT;

	if (ZOOMINFOSIZE < size)
	{
		FSkipEven (file, size - ZOOMINFOSIZE);
	}
	return (TRUE);

/*========================================================================*/
ABORT:
	return (FALSE);
} /* ReadZoomChunk */


/*********************************************************************
 *
 * ProcessZoomChunk
 *
 * PURPOSE
 *		Process zoom info <pzic> and stuff into <prt>.
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
static BOOL ProcessZoomChunk(RoomType *prt, ZoomInfoChunkType *pzic)
{

	WORD i, wZoom;
	ULONG ulFind, ulLast;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessZoomChunk";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prt && pzic)
	{
		if (!pzic->wSrcSkip)
			pzic->wSrcSkip = 1;

		ulFind	= ((ULONG) pzic->wDstDup << 16) / pzic->wSrcSkip;
		ulLast	= ULONG_MAX;
		wZoom	= 0;

		for (i = 0; i < GetZoomMax(); i++)
		{
			ULONG ulZoom = ((ULONG) wGetZoomDstDup(i) << 16) / wGetZoomSrcSkip(i);
			ULONG ulDist = (ulZoom > ulFind) ? ulZoom-ulFind : ulFind-ulZoom;

			if (ulDist < ulLast)
			{
				ulLast = ulDist;
				wZoom = i;
			}
		}

		prt->wZoomIndex = wZoom;

		if (pzic->bActive)
			SetZoomOn(prt);
		else
			SetZoomOff(prt);

		return TRUE;
	}
	return FALSE;
} /* ProcessZoomChunk */


/**************************************************************************
**
** ProcessDataTChunk
**
** USAGE
**
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**		10/22/89 (RGM) - Created.
**		12/11/93 (dcc) - Add check 4 already loaded composite tiles.
**		06/17/94 (dcc) - Add support for '.' and '..' to SearchDir's.
**		10/11/94 (dcc) - Call dupstr() instead of strdup().
**
** SEE ALSO
**
*/
static BOOL ProcessDataTChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	TS_DataHeaderType	ts_dataheader;

	char				*iname;
	char				*name;
	char				*status;

	WORD				old_id;
	WORD				new_id;

#if GET_OK_FROM_USER_TO_LOAD_OLD_TILESET_AS_NEW
	int					usersay = 0;
#endif
	WORD				found_num;
	BOOL				found;

	BOOL				new_one		= FALSE;
	unsigned long		count		= 0;
	short				width		= 0;
	short				height		= 0;
	NodeType			*newpath	= NULL;
	NodeType			*temppath	= NULL;

#if __INPUTONLY__
#else
	WORD				option;
	char				*fname;
#endif

/**/if (CurrentTileSet) goto ABORT;
	iname	= MEM_malloc (255);
	status	= MEM_malloc (300);
	if (! (iname && status)) {
			fErrorType = S_LOW_MEM;
/**/		goto ABORT;
	}


	/************************************/
	/* Read in the tileset data header */

	if (read (file, (void *) &ts_dataheader, TSDHSIZE) != TSDHSIZE) {
/**/	goto ABORT;
	}

	WordSex(ts_dataheader.TSDH_id);
	WordSex(ts_dataheader.TSDH_flags);

	count += TSDHSIZE;
/**/if (count >= size) goto ABORT;


	old_id	= ts_dataheader.TSDH_id;


	/************************************************/
	/* KLUDGE to read in extra Nintendo information */

	if ((ts_dataheader.TSDH_flags & GRID_TYPE_MODIFIER) != GRID_IS_BOXED) {

		if (read (file, (void *) &width, sizeof (short)) != sizeof (short)){
/**/		goto ABORT;
		}

		if (read (file, (void *) &height, sizeof (short)) != sizeof (short)){
/**/		goto ABORT;
		}

		WordSex(width);
		WordSex(height);

		count += (2 * sizeof(short));

/**/	if (count >= size) goto ABORT;
	}

	/*******************************************************************/
	/* KLUDGE to read in the remaining data (the source picture name). */

	{
		UWORD dif;

		dif = (UWORD) (size - count);

		if (read (file, (void *) iname, dif) != dif)
/**/		goto ABORT;

		iname [dif] = 0;
	}

	name = FixPath(iname);

	count = size;

#if __INPUTONLY__
	if ((found_num = TS_idFromNameNumberType (	name,
										CurrentUserInfo->UserType,
										CurrentUserInfo->UserNumber,
										GlobalTileSpace)) != 0)
	{
		new_id	= found_num;
	}
#else
	if ((ts_dataheader.TSDH_flags & GRID_TYPE_MODIFIER) == TILESET_COMPOSITE)
	{
#if GET_OK_FROM_USER_TO_LOAD_OLD_TILESET_AS_NEW
		if ( (found_num = TS_idFromNameNumberType (	name,
											CurrentUserInfo->UserType,
											CurrentUserInfo->UserNumber,
											GlobalTileSpace)) &&
		((usersay = DoThis2 ("Load Room/Map", BaseName(name), 
				"\tis already loaded.\tLoad as a new tileset?")) == NO) )
		{
			new_id	= found_num;
		}
#else
		if ((found_num = TS_idFromNameNumberType (	name,
											CurrentUserInfo->UserType,
											CurrentUserInfo->UserNumber,
											GlobalTileSpace)) != 0)
		{
			new_id	= found_num;
		}
#endif
		else
		{
			new_id	= GetNextTilesetID(GlobalTileSpace);
			if ((CurrentTileSet = MEM_calloc (1, TILESETSIZE)) == NULL)
			{
				fErrorType = S_LOW_MEM;
/**/			goto ABORT;
			}
			CurrentTileSet->Filespec = dupstr (name);
			if (! CurrentTileSet->Filespec) {
				MEM_free (CurrentTileSet);
				CurrentTileSet = NULL;
				fErrorType = S_LOW_MEM;
/**/			goto ABORT;
			}
			if ((CurrentTileSet->cinShared = CreateColorInfo()) == NULL)
			{
				fErrorType = S_LOW_MEM;
/**/			goto ABORT;
			}
			CurrentTileSet->TS_id		= new_id;
			CurrentTileSet->Flags		= ts_dataheader.TSDH_flags;
			CurrentTileSet->Width		= width;
			CurrentTileSet->Height		= height;
			CurrentTileSet->SourceRoom	= NULL;
		}
	}
	else
	{
#if GET_OK_FROM_USER_TO_LOAD_OLD_TILESET_AS_NEW
		if ( (found_num = TS_idFromNameNumberType (	name,
											CurrentUserInfo->UserType,
											CurrentUserInfo->UserNumber,
											GlobalTileSpace)) &&
		((usersay = DoThis2 ("Load Room/Map", BaseName(name), 
				"\tis already loaded.\tLoad as a new tileset?")) == NO) ) {
			new_id	= found_num;
		}
#else
		if ((found_num = TS_idFromNameNumberType (	name,
											CurrentUserInfo->UserType,
											CurrentUserInfo->UserNumber,
											GlobalTileSpace)) != 0) {
			new_id	= found_num;
		}
#endif
#endif/*__INPUTONLY__*/
		else {

#if GET_OK_FROM_USER_TO_LOAD_OLD_TILESET_AS_NEW
			if (usersay == ERROR) {
				fErrorType = S_LOW_MEM;
/**/			goto ABORT;
			}
#endif

			/************************************/
			/* We have to load in a new tileset */

			found = FALSE;

#if !__INPUTONLY__
			/********************************/
			/* try to find the tileset file */

			newpath = Head (PathList);
			while ((! found) || (! CurrentTileSet))
			{
#if __MSDOS__
				SetFloppy(name);	/* If refer to floppy, & 1 drive machine, set it */
#endif/*__MSDOS__*/
				Assert(sizeof(fSearchAsSpecified) == sizeof(short));

				if ((found = EIO_FileExists (name)) == 0 ||
					fSearchAsSpecified == FALSE)	/* ignore "AS SPECIFIED"? */
				{
					/** ok, we didn't find the tileset **/
#if __AMIGAOS__
					AutoReqOff ();
#endif/*__AMIGAOS__*/
					while (AutoSub && (!IsEOList(newpath)))
					{
						/****************************************/
						/** try to use the autosub directories **/

						if (newpath->Name[0] == '.' && newpath->Name[1] == '.')
						{
							size_t i;
							char *pch;

							strcpy(status, szCurrentDir);
							i = strlen(status)-1;
							*(status+i) = 0;
							pch = strrchr(status, DIRSEP);
							if (pch)
							{
								*(pch+1) = 0;
							}
							else
							{
								*(status+i) = DIRSEP;
							}

							/* NB: BUGBUG: this assumes that newpath->Name[2] is a
							   separator character ('\' on MSDOS) */

							strcat(status, newpath->Name+3);
						}
						else if (newpath->Name[0] == '.')
						{
							strcpy(status, szCurrentDir);

							/* NB: BUGBUG: this assumes that newpath->Name[1] is a
								separator character ('\' on MSDOS) */

							strcat(status, newpath->Name+2);
						}
						else
						{
							strcpy (status, newpath->Name);
						}
						strcat (status, BaseName (name));
						strcpy (name, status);
						newpath = Next(newpath);
#if __MSDOS__
						SetFloppy(name);	/* If refer to floppy, & 1 drive machine, set it */
#endif/*__MSDOS__*/
						if (EIO_FileExists(name))
							break;
					}
				}

				if ((found = EIO_FileExists (name)) != 0)
				{
					GridInfoType	temp_grid_info	= {0, 0, 0, 0, 0, 0};

					if ((ts_dataheader.TSDH_flags & GRID_TYPE_MODIFIER)
					!= GRID_IS_BOXED) {

						temp_grid_info.image_width	= width;
						temp_grid_info.image_height	= height;
					}

					ClearGlobalError ();
					CurrentTileSet =	ForceLoadTileSet (
										name,
										ts_dataheader.TSDH_flags,
										&temp_grid_info,
										FALSE
									);
				}

				if (! CurrentTileSet)
				{
					if (GlobalErr != ERR_OUT_OF_MEMORY)
					{
						/***************************************/
						/** ask the user for a different name **/

						strcpy (status, "Can't load '");
						strcat (status, BaseName(name));
						strcat (status, "'.\nWould you like to try a different file?");
						option = IsItThisOne ("Load Room/Map", status);
/**/					if ((option == CANCEL)||(option == NO)) goto ABORT;

						strcpy (status, BaseName (name));
						strcat (status, ", AKA...");

						SetFreqTitle (EasyFR[PATH_RELOADTILES], status);
						SetFreqFilename (EasyFR[PATH_RELOADTILES], name);

						fname = GetFileName(MainWindow,EasyFR[PATH_RELOADTILES]);
/**/					if (! fname) goto ABORT;

						strcpy (name, fname);
						new_one = TRUE;
					}
					else
/**/					goto ABORT;
				}

				/***************************************************/
				/** wow, we've succesfully loaded a replacement	**/
				/** tileset.  let's save the directory it came	**/
				/** from, just in case the user wants to load	**/
				/** from it again						     **/

				if (new_one && (CurrentTileSet != NULL)) {
					char	A_dir[256];

					EIO_fnsplit (name, A_dir, NULL, NULL);

					strcpy (status, A_dir);
					if (status == "") {
						strcpy (status, "(current directory)");
					}
					strcat (status, "\nTry to load other 'problem' tilesets from here?");
					option = IsItThisOne ("Load Room/Map", status);
/**/				if (option == YES) {
						AutoSub = TRUE;
						strcpy (status, A_dir);
						temppath = CreateNode(sizeof(NodeType), status);
/**/					if (! temppath) goto ABORT;
						AddHead (PathList, temppath);
					}
				}
			}
#else/*__INPUTONLY__*/
			CurrentTileSet = MEM_calloc (1, TILESETSIZE);
			CurrentTileSet->Filespec = dupstr (name);
			if (! CurrentTileSet->Filespec) {
				printf ("Can't allocate tileset name.\n");
/**/			goto ABORT;
			}
			if ((CurrentTileSet->cinShared = CreateColorInfo()) == NULL)
/**/			goto ABORT;
#endif/*__INPUTONLY__*/
			new_id	= CurrentTileSet->TS_id;
		}
	}

	PutID_Translate (old_id, new_id, Translator);

	SAFEFREE (status);
	SAFEFREE (iname);

	return (TRUE);
/*========================================================================*/
ABORT:

	if (CurrentTileSet) {
#if !__INPUTONLY__
		UnLoadTileSet (CurrentTileSet);
		CurrentTileSet = NULL;
#else
		DeleteColorInfo(CurrentTileSet->cinShared);
		MEM_free (CurrentTileSet);
		CurrentTileSet = NULL;
#endif
	}
	SAFEFREE (status);
	SAFEFREE (iname);

	return (FALSE);
} /* ProcessDataTChunk */


/**************************************************************************
**
** ProcessUserTChunk
**
** USAGE
**
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	11/03/89 Friday - Created out of ProcessUserRChunk. (RGM)
**
** SEE ALSO
**
*/
static BOOL ProcessUserTChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	unsigned long	count	= 0;

/**/	if (! CurrentUserInfo) goto ABORT;

	/*****************************************/
	/* read in the first user info (4 words) */

	if (read (file, (void *) CurrentUserInfo, USERINFOSIZE) != USERINFOSIZE){
/**/	goto ABORT;
	}

	WordSex(CurrentUserInfo->UserType);
	WordSex(CurrentUserInfo->UserNumber);
	WordSex(CurrentUserInfo->UserExtra1);
	WordSex(CurrentUserInfo->UserExtra2);

	count += USERINFOSIZE;

/**/	if (count > size) goto ABORT;

	if (count < size) {
		WORD i;
		for (i = 0; i < 2; i ++) {
			UWORD comment_size;

			if (read (file, (void *) &comment_size, sizeof (UWORD)) != sizeof (UWORD)){
/**/			goto ABORT;
			}

			WordSex(comment_size);	/*dcc*/

			count += sizeof (UWORD);
			if (comment_size) {
				CurrentUserComment[i] = MEM_calloc (1, comment_size + 1);

				if (CurrentUserComment[i]) {
					if (read (file, (void *) CurrentUserComment[i], comment_size) != comment_size) {
/**/					goto ABORT;
					}
					count += comment_size;
					CurrentUserComment[i][comment_size] = 0;
				}
				else {
					fErrorType = S_LOW_MEM;
/**/				goto ABORT;
				}
			}
		}
	}

	if (count < size) {
		FSkipEven (file, size - count);
		count = size;
	}
	return (TRUE);

/*========================================================================*/
ABORT:
	return (FALSE);
} /* ProcessUserTChunk */


/**************************************************************************
**
** ProcessRoomForm
**
** USAGE
**	ok = ProcessRoomForm (INFILE, SIZE_LEFT);
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**	10/17/89 Tuesday - Created. (RGM)
**	04/01/93 Thursday (dcc)	- don't copy CurrentUserComment[], just use ptr.
**	08/14/94 Sunday (dcc) - add support for 'ROOM''NAME', 'LAYR', and 'LAY1'
**
** SEE ALSO
**
*/
static BOOL ProcessRoomForm (
	FileHandleType	file,
	unsigned long	size
)
{
	IFFChunkHeaderType	chunk_id;
	unsigned long		count;
	BOOL				status	= TRUE;

	if ( (CurrentColorInfo	= CreateColorInfo () ) == NULL)
		status = FALSE;

	if ( (CurrentUserInfo	= MEM_calloc (1, USERINFOSIZE) ) == NULL)
		status = FALSE;

	count = 0;
	while ( (! tUME_EOF) && (count < size) && status)
	{
		status = GetHeader (file, &chunk_id);

		if (! tUME_EOF) {
			if (status) {
				count += IFFCH_SIZE;

				if (chunk_id.DataSize <= (size - count)) {
					switch (chunk_id.TypeID.ID_Number) {

					case (CHUNK_DATA):
						status = ProcessDataRChunk (file, chunk_id.DataSize);
						break;

#if SaveNew
					case (CHUNK_NAME):
						status = ProcessROOMNAMEChunk(file, chunk_id.DataSize);
						break;

					case (CHUNK_LAYR):
						status = ProcessROOMLAYRChunk(file, chunk_id.DataSize);
						break;

					case (CHUNK_LAY1):
						status = ProcessROOMLAY1Chunk(file, chunk_id.DataSize);
						break;

#endif //SaveNew
					case (CHUNK_USER):
						status = ProcessUserRChunk (file, chunk_id.DataSize);
						break;

					case (CHUNK_CMNT):
						status = ProcessCmntChunk (file, chunk_id.DataSize);
						break;

					case (CHUNK_CMAP):
						status = ProcessCmapChunk (file, chunk_id.DataSize);
						break;

					case (CHUNK_CFLG):
						status = ProcessCflgChunk (file, chunk_id.DataSize);
						break;

					case (CHUNK_CYCL):
						status = ProcessCyclChunk (file, chunk_id.DataSize);
						break;

					case (CHUNK_CINF):
						status = ProcessCinfChunk (file, chunk_id.DataSize);
						break;

					case (CHUNK_HSVP):
						status = ProcessHsvpChunk (file, chunk_id.DataSize);
						break;

					case (CHUNK_GRID):
					{
						GridInfoChunkType gic;

						if ((status = ReadGridChunk(file, chunk_id.DataSize, &gic)) != FALSE)
							status = ProcessGridChunk(CurrentRoom, &gic);
					}
						break;

					case (CHUNK_GUID):
					{
						GridInfoChunkType gic;

						if ((status = ReadGuidChunk(file, chunk_id.DataSize, &gic)) != FALSE)
							status = ProcessGuidChunk(CurrentRoom, &gic);
					}
						break;

#if NEWTUME
					case (CHUNK_guid):
					{
						GuideInfoChunk		*pGuide = NULL;
						short			cGuide;

						if ((status = ReadGuideChunk(file, chunk_id.DataSize, &pGuide, &cGuide)) != FALSE)
						{
							status = ProcessGuideChunk(CurrentRoom, pGuide, cGuide);
							MEM_free(pGuide);
						}
					}
						break;
#endif

					case (CHUNK_ZOOM):
					{
						ZoomInfoChunkType zic;

						if ((status = ReadZoomChunk(file, chunk_id.DataSize, &zic)) != FALSE)
							status = ProcessZoomChunk(CurrentRoom, &zic);
					}
						break;

					default:
						status = FSkipEven (file, chunk_id.DataSize);
						count += ODD (chunk_id.DataSize);
						break;
					}
					count += chunk_id.DataSize;
				}
			}
			if ((status) && (ODD (count))) {
				count ++;
				/* fgetc (file); */
				u_getc (file);
			}
		}
	}
	status =  ((status) && (size == count) && (CurrentRoom));

	if (status) {
#if __AMIGAOS__
		ResetColorInfo (CurrentColorInfo);
#elif __MSDOS__
		InitColorInfo (CurrentColorInfo);
#endif/*__AMIGAOS__/__MSDOS__*/
		if (CurrentRoom->R_ColorInfo) {
			DeleteColorInfo (CurrentRoom->R_ColorInfo);
			CurrentRoom->R_ColorInfo = NULL;
		}
		CurrentRoom->R_ColorInfo	= CurrentColorInfo;
		CurrentColorInfo			= NULL;

		if (CurrentUserInfo) {
			CurrentRoom->UserType	= CurrentUserInfo->UserType;
			CurrentRoom->UserNumber	= CurrentUserInfo->UserNumber;
			if (CurrentUserComment[0]) {
				CurrentRoom->Comment[0] = CurrentUserComment[0];
				CurrentUserComment[0] = NULL;
			}
			if (CurrentUserComment[1]) {
				CurrentRoom->Comment[1] = CurrentUserComment[1];
				CurrentUserComment[1] = NULL;
			}
		}

		AddTail (&GlobalMap->Rooms, CurrentRoom);
	}
	else if (CurrentRoom) {
		DeAllocateRoom (CurrentRoom);
	}

	CurrentRoom = NULL;

	MEM_free (CurrentUserInfo);
	CurrentUserInfo = NULL;

	if (CurrentColorInfo) {
		DeleteColorInfo (CurrentColorInfo);
		CurrentColorInfo = NULL;
	}
	return (status);

} /* ProcessRoomForm */


/**************************************************************************
**
** ProcessTsetForm
**
** USAGE
**	ok = ProcessTsetForm (INFILE, SIZE_LEFT);
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/22/89 Sunday - Created out of ProcessRoomForm. (RGM)
**	04/01/93 Thursday (dcc)	- don't copy CurrentUserComment[], just use ptr.
**	12/14/93 Tuesday (dcc) - add call to LookupSomeTSTInfoFromUserType().
**
** SEE ALSO
**
*/
static BOOL ProcessTsetForm (
	FileHandleType	file,
	unsigned long	size
)
{
	IFFChunkHeaderType	chunk_id;
	unsigned long		count;
	BOOL				status	= TRUE;
#if __INPUTONLY__
#else
	RoomType			*trynfindroom;
	RoomType			*room;
#endif
	BOOL				fGrid	= FALSE;
	BOOL				fGuid	= FALSE;
#if NEWTUME
	BOOL				fGuide	= FALSE;
	short			cGuide;
#endif
	BOOL				fZoom	= FALSE;
	GridInfoChunkType	gicGrid;
	GridInfoChunkType	gicGuide;
#if NEWTUME
	GuideInfoChunk		*pGuide	= NULL;
#endif
	ZoomInfoChunkType	zic;

	if ( (CurrentUserInfo	= MEM_calloc (1, USERINFOSIZE) ) == NULL)
		status = FALSE;

	count = 0;
	while ( (! tUME_EOF) && (count < size) && status) {

		status = GetHeader (file, &chunk_id);

		if (! tUME_EOF) {
			if (status) {
				count += IFFCH_SIZE;

				if (chunk_id.DataSize <= (size - count)) {
					switch (chunk_id.TypeID.ID_Number) {

					case (CHUNK_DATA):
						status = ProcessDataTChunk (file, chunk_id.DataSize);
						break;

					case (CHUNK_USER):
						status = ProcessUserTChunk (file, chunk_id.DataSize);
						break;

					case (CHUNK_GRID):
						status = fGrid = ReadGridChunk(file, chunk_id.DataSize, &gicGrid);
						break;

					case (CHUNK_GUID):
						status = fGuid = ReadGuidChunk(file, chunk_id.DataSize, &gicGuide);
						break;

#if NEWTUME
					case (CHUNK_guid):
						status = fGuide= ReadGuideChunk(file, chunk_id.DataSize, &pGuide, &cGuide);
						break;
#endif

					case (CHUNK_ZOOM):
						status = fZoom = ReadZoomChunk(file, chunk_id.DataSize, &zic);
						break;

					default:
						status = FSkipEven (file, chunk_id.DataSize);
						count += ODD (chunk_id.DataSize);
						break;
					}
					count += chunk_id.DataSize;
				}
			}
		}
		if ((status) && (ODD (count))) {
			count ++;
			/* fgetc (file); */
			u_getc (file);
		}
	}
	status = ((status) && (size == count));

	if (status && CurrentTileSet && CurrentUserInfo) {
		CurrentTileSet->UserType = CurrentUserInfo->UserType;
		CurrentTileSet->UserNumber = CurrentUserInfo->UserNumber;
		LookupSomeTSTInfoFromUserType(CurrentTileSet);
#if __INPUTONLY__
		CurrentTileSet->TileCount = CurrentUserInfo->UserExtra2;
#endif
		if (CurrentUserComment[0]) {
			CurrentTileSet->Comment[0] = CurrentUserComment[0];
			CurrentUserComment[0] = NULL;
		}
		if (CurrentUserComment[1]) {
			CurrentTileSet->Comment[1] = CurrentUserComment[1];
			CurrentUserComment[1] = NULL;
		}

	}
#if !__INPUTONLY__
	if (status && CurrentTileSet) {
		if (CurrentUserInfo && (CurrentUserInfo->UserExtra1)) { /* room # */

			/**************************************************/
			/* Try to find the source room for these tiles    */
			/* We use the original room # (->UserExtra1) plus */
			/* the highest current room number because the    */
			/* current rooms have all been renumbered from 1  */
			/* on up, and new rooms get id numbers of their   */
			/* original id # plus the highest current one (to */
			/* avoid collisions).                             */

			trynfindroom =
				TryNFindRoomID (
					(WORD) (CurrentUserInfo->UserExtra1 + TopRoom),
					GlobalMap);

			/***************************************/
			/* Is its source room already loaded ? */

			if (trynfindroom)
			{
				if ((CurrentTileSet->Flags & GRID_TYPE_MODIFIER) != TILESET_COMPOSITE &&
				    (! AppendTiles (CurrentTileSet, trynfindroom)))
				{
					status			= FALSE;
				}
			}
			else
			{
				if ( (trynfindroom = CreateTileSetRoom (CurrentTileSet)) == NULL)
				{
					status			= FALSE;
				}
				else
				{
					/*****************************************/
					/* append this room to the map structure */

					trynfindroom->R_id =
									CurrentUserInfo->UserExtra1 + TopRoom;
					trynfindroom->Flags |= PREROOM;
					AddTail (&(GlobalMap->Rooms), trynfindroom);
				}
			}
			if (fGrid)
				status = ProcessGridChunk(trynfindroom, &gicGrid);
			if (fGuid)
				status = ProcessGuidChunk(trynfindroom, &gicGuide);
#if NEWTUME
			if (fGuide)
			{
				status = ProcessGuideChunk(trynfindroom, pGuide, cGuide);
				MEM_free(pGuide);
			}
#endif
			if (fZoom)
				status = ProcessZoomChunk(trynfindroom, &zic);
			if (status) {
				AddTileSet (CurrentTileSet, GlobalTileSpace);
			}
			trynfindroom	= NULL;
			CurrentTileSet	= NULL;
		}
		else {
			room =	PlaceTiles (CurrentTileSet, NULL, GlobalMap);

			if (room) {
				if (fGrid)
					status = ProcessGridChunk(room, &gicGrid);
				if (fGuid)
					status = ProcessGuidChunk(room, &gicGuide);
#if NEWTUME
				if (fGuide)
				{
					status = ProcessGuideChunk(room, pGuide, cGuide);
					MEM_free(pGuide);
				}
#endif
				if (fZoom)
					status = ProcessZoomChunk(room, &zic);
				room->Flags |= PREROOM;
				AddTileSet (CurrentTileSet, GlobalTileSpace);
				CurrentTileSet = NULL;
			}
			else {
				status = FALSE;
			}
		}
	}
#else
	if (status && CurrentTileSet) {
		CurrentTileSet->Flags |= PRETILESET;
		AddTileSet (CurrentTileSet, GlobalTileSpace);
		CurrentTileSet = NULL;
	}
#endif/*__INPUTONLY__*/

	MEM_free (CurrentUserInfo);
	CurrentUserInfo = NULL;
	return (status);
} /* ProcessTsetForm */


/**************************************************************************
**
** ProcessTumeFormChunk
**
** USAGE
**	ok = ProcessTumeFormChunk (INFILE, SIZE);
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
** SEE ALSO
**
*/
static BOOL ProcessTumeFormChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	unsigned long	result;
	unsigned long	form_id;
	BOOL			status = FALSE;

	result = GetFormID (file, &form_id);
	size -= result;
	if (result) {
		switch (form_id) {

		case (FORM_ROOM):
			status = ProcessRoomForm (file, size);
			break;

		case (FORM_TSET):
			status = ProcessTsetForm (file, size);
			break;

		default:
			status = FSkipEven (file, size);
			break;
		}
	}
	return (status);
} /* ProcessTumeFormChunk */


/**************************************************************************
**
** ProcessTumeForm
**
** USAGE
**	ok = ProcessTumeForm (INFILE, SIZE_LEFT);
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/16/89 Monday - Created. (RGM)
**
** SEE ALSO
**
*/
static BOOL ProcessTumeForm (
	FileHandleType	file,
	unsigned long	size
)
{
	IFFChunkHeaderType	chunk_id;
	unsigned long		count;
	BOOL				status		= TRUE;

	count = 0;
	while ((! tUME_EOF) && (count < size) && status) {

		status = GetHeader (file, &chunk_id);

		if (! tUME_EOF) {
			if (status) {
				count += IFFCH_SIZE;

				if (chunk_id.DataSize <= (size - count)) {
					switch (chunk_id.TypeID.ID_Number) {
					case (CHUNK_FORM):
						status = ProcessTumeFormChunk (file, chunk_id.DataSize);
						break;

					default:
						status = FSkipEven (file, chunk_id.DataSize);
						break;
					}
					count += chunk_id.DataSize;
				}
			}
		}
	}
	return ((BOOL) ((status) && (size == count)));
} /* ProcessTumeForm */


/**************************************************************************
**
** ProcessMainFormChunk
**
** USAGE
**	ok = ProcessMainFormChunk (INFILE, SIZE_LEFT);
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/16/89 Monday - Created. (RGM)
**
** SEE ALSO
**
*/
static BOOL ProcessMainFormChunk (
	FileHandleType	file,
	unsigned long	size
)
{
	unsigned long	result;
	unsigned long	form_id;
	BOOL			status = FALSE;

	result = GetFormID (file, &form_id);
	size -= result;
	if (result) {
		switch (form_id) {

		case (FORM_TUME):
			status = ProcessTumeForm (file, size);
			break;

		default:
			status = FSkipEven (file, size);
			break;
		}
	}
	return (status);
} /* ProcessMainFormChunk */


/**************************************************************************
**
** ProcessMainForm
**
** USAGE
**	ok = ProcessMainForm (INFILE);
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/17/89 Tuesday - Created. (RGM)
**	05/19/93 Wednesday (dcc) - KLUDGE: changed "while ((! tUME_EOF) && status) {"
**						to "if ((! tUME_EOF) && status)". This is a kludge
**						because it does not enforce the IFF standard
**						"There should be no data after the chunk end.";
**						However, it does follow the rule
**						"An IFF file is a single FORM, LIST, or CAT chunk."
**
** SEE ALSO
**
*/
static BOOL ProcessMainForm (FileHandleType file)
{
	IFFChunkHeaderType	chunk_id;
	BOOL				status	= TRUE;

	if ((! tUME_EOF) && status) {

		status = GetHeader (file, &chunk_id);

		if (! tUME_EOF) {
			if ((status) && (chunk_id.DataSize)) {
				switch (chunk_id.TypeID.ID_Number) {
				case (CHUNK_FORM):
					status =
						ProcessMainFormChunk (file, chunk_id.DataSize);
					break;

				default:
					status = FSkipEven (file, chunk_id.DataSize);
					break;
				}
			}
		}
		else {
			status = TRUE;
		}
	}

	return (status);
} /* ProcessMainForm */


/*********************************************************************
 *
 * SetLoadLayer
 *
 * PURPOSE
 *		Sets the layer <play> to insert loaded layer, or set to NULL if not
 *		inserting layers.
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
 *		05/11/93 Tuesday (dcc) - created.
 *
 * SEE ALSO
 *
*/
void SetLoadLayer(LayerType *play, short f)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetLoadLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	playLoad = play;
	fAppendorLoad = f;

} /* SetLoadLayer */


/**************************************************************************
**
** LoadMap
**
** USAGE
**	err = LoadMap (LOAD_FILE_NAME);
**	if (err < 0) goto ABORT;
**
** FUNCTION
**
**
** INPUTS
**
**
** RESULTS
**
**
** METHOD
**
**
** BUGS
**
**
** HISTORY
**
**	10/22/89 Sunday    - Created form SaveMap. (RGM)
**
**	12/06/89 Wednesday - Modified to return STATUS value, added
**	                     PathList support. (RGM)
**
**	04/29/93 Thursday (dcc) - changed PathList support so it is initialized
**					in ioini.c, and support "fixed" search-dirs from tUME.INI
**	BUGBUG: this means ProcessSearchDirs() must be called B4 this module will work!
**	BUGBUG: SetLoadLayer() must be called B4 LoadMap() is called
**
**	06/17/94 Friday (dcc) - add support for '.' and '..' to SearchDir's
**
** SEE ALSO
**
*/
short LoadMap (char *filename)
{
	FileHandleType	infile;
#if __AMIGAOS__
	void			*old_pointer;
#endif/*__AMIGAOS__*/
	BOOL			result;
	NodeType		*pnodCurrentDir = NULL;	/* Node of current directory */

	short		status = S_OK;

	NodeType		*pnodINI = Head(PathList);	/* Mark 1st node from tUME.INI */

#if __INPUTONLY__
#else
	SetFreqDirname (EasyFR[PATH_RELOADTILES], filename);
#if __MSDOS__
	DCC_TempSetPointer (BPI_WAIT_POINTER);
#endif/*__MSDOS__*/
#endif

#if __AMIGAOS__
	old_pointer		= GlobalPointer;
#endif/*__AMIGAOS__*/
	errno			= 0;
	tUME_EOF			= FALSE;
	MARK_CLOSE(infile);

	CurrentRoom			= NULL;
	CurrentTileSet			= NULL;
	CurrentColorInfo		= NULL;
	CurrentCycleInfo		= NULL;
	CurrentUserInfo		= NULL;
	CurrentUserComment[0]	= NULL;
	CurrentUserComment[1]	= NULL;

	AutoSub				= !IsEmpty(PathList);

	TopRoom				= RenumberRooms (GlobalMap);

	Translator = CreateID_Translate();
	if (! Translator) {
		status = S_LOW_MEM;
/**/	goto ABORT;
	}

	infile = open (filename, INMODE);

	if (! OK_OPEN(infile)) {
		status = S_ERROR;
/**/	goto ABORT;
	}

	Assert(sizeof(fSearchCurrentDir) == sizeof(short));

	/* Add current sub-directory to search list? */

	EIO_fnsplit(filename, szCurrentDir, NULL, NULL);
	if (fSearchCurrentDir)
	{
		AutoSub = TRUE;
		if ((pnodCurrentDir = CreateNode(sizeof(NodeType), szCurrentDir)) == NULL)
		{
/**/		goto ABORT;
		}
		AddTail (PathList, pnodCurrentDir);
	}

	/****************************************/
	/* Attempt to read in the tUME iff file */

	fErrorType	= S_ERROR;
	result		= ProcessMainForm (infile);

#if __AMIGAOS__
	AutoReqOn ();
#endif

	if (! result) {
		status = fErrorType;
/**/	goto ABORT;
	}

	close (infile);
	MARK_CLOSE(infile);

	/* Link all new loaded composite tileset to appropriate rooms. */

	if (!LinkCompositeTileSets(GlobalTileSpace, GlobalMap, Translator))
	{
		status = S_ERROR;
/**/	goto ABORT;
	}

	/*******************************************/
	/* Convert the rooms back to internal rep. */

#if __INPUTONLY__
#else
#if __AMIGAOS__
	GlobalPointer = RGM_SetPointer (&YieldMPointer, MainWindow);
#endif/*__AMIGAOS__*/
#endif

	TranslateRooms (GlobalMap, RAWROOM, Translator);

	if (playLoad)
	{
		BOOL f = AppendorInsertLayersIntoRoom(playLoad, GlobalMap, fAppendorLoad);
		playLoad = NULL;
		if (!f)
		{
			status = S_LOW_MEM;
/**/		goto ABORT;
		}
	}

#if __INPUTONLY__
#else
#if __AMIGAOS__
	GlobalPointer = RGM_SetPointer (old_pointer, MainWindow);
#elif __MSDOS__
	DCC_SetPointer (GlobalPointer);
#endif/*__AMIGAOS__/__MSDOS__*/
#endif

	DeleteID_Translate (Translator);

	PromoteTileSets (GlobalTileSpace);
	PromoteRooms (GlobalMap);

/**/	goto EXIT;

/*========================================================================*/
ABORT:
	if (OK_OPEN(infile)) {
		close (infile);
		MARK_CLOSE (infile);
	}
	DeleteID_Translate (Translator);
	CleanTileSets (GlobalTileSpace);
	CleanRooms (GlobalMap);

EXIT:
	{
		/* Delete all nodes up to pnodINI (1st "permanent" search-dir node */

		NodeType *pnod = Head(PathList);

		while (pnod != pnodINI)
		{
			pnod = RemHead(PathList);
			DeleteNode(pnod);
			pnod = Head(PathList);
		}

		/* Delete current dir node if it was added */

		if (pnodCurrentDir != NULL)
		{
			pnod = RemTail(PathList);
			DeleteNode(pnod);
			pnodCurrentDir = NULL;
		}
	}

	TopRoom		= RenumberRooms (GlobalMap);

#if __INPUTONLY__
#else
	ShowState (LastTBar);
#endif

	return (status);
} /* LoadMap */

