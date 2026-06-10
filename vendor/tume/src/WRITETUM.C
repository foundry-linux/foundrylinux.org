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
 * WRITEtUME.C
 *
 * PROGRAMMER : R.G. Marquez
 *    VERSION : 00.000
 *    CREATED : 10/16/89
 *   MODIFIED : 11/08/94
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		Routines to write tUME IFF files.
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
 *		08/13/94 (dcc) - add uwSaveFormat, support for uwSaveFormat == 1
 *					  (write 'ROOM''NAME' instead of 'ROOM''DATA')
 *					  (write 'LAYR' chunk)
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
*/

#include "rwtume.h"
#if UseSparseArray
#include "sparsef.h"
#endif // UseSparseArray
#include "writetum.h"				/* Verify function prototypes. */

/****************************** G L O B A L S *****************************/

#if SaveNew
UWORD				uwSaveFormat = 0;
#endif //SaveNew

/******************************* L O C A L S ******************************/

#if fDoSaveRooms
#if !__INPUTONLY__

static	UWORD		IO_Options;

static	RoomType		*CurrentRoom			= NULL;
static	TileSetType	*CurrentTileSet		= NULL;
static	ColorInfo		*CurrentColorInfo		= NULL;
static	CS_CycleInfo	*CurrentCycleInfo		= NULL;
static	UserInfoType	*CurrentUserInfo		= NULL;

#if SaveNew
static	LayerType		*playCurrent			= NULL;

static	UWORD		uwlayCurrent;
#endif //SaveNew

static	int		TopRoom;

/***************************** R O U T I N E S ****************************/


/**************************************************************************
**
** PutHeader
**
** USAGE
**	ok = PutHeader (OUTFILE, &HEADER);
**
** FUNCTION
**	Put the chunk header into an IFF file stream.
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
**		10/17/89 Tuesday - Created from PutHeader. (RGM)
**
** SEE ALSO
**
*/
static BOOL PutHeader (
	FileHandleType		file,
	IFFChunkHeaderType	*header
)
{
	MakeBigLong(header->DataSize);	/*dcc*/

	return (BOOL) (write (file, (void *) header, IFFCH_SIZE) == IFFCH_SIZE);
} /* PutHeader */


/**************************************************************************
**
** PutFormID
**
** USAGE
**	bytes_written = PutFormID (OUTFILE, ID_PTR);
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
** RETURN VALUE
**		Returns number of bytes written, or 0 if error.
**
** HISTORY
**	10/17/89 Tuesday - Created from GetFormID. (RGM)
**
** SEE ALSO
**
*/
static unsigned long PutFormID (
	FileHandleType	file,
	unsigned long	*ID_ptr
)
{
	if (write (file, (void *) ID_ptr, ULONGSIZE) == ULONGSIZE)
	{
		return ULONGSIZE;
	}

	return 0;	// error
} /* PutFormID */


/**************************************************************************
**
** GenericWriteChunk
**
** USAGE
**	size = GenericWriteChunk (OUTFILE, CHUNK_NAME, OUT_FUNCTION);
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
** RETURN VALUE
**		Returns the number of bytes written, or 0 if error.
**
** HISTORY
**	10/17/89 Tuesday - Created from WriteTumeChunk. (RGM)
**
** SEE ALSO
**
*/
static unsigned long GenericWriteChunk (
	FileHandleType	file,
	unsigned long	typeid,
	unsigned long	(*datafunction) (FileHandleType)
)
{
	long				last_pos;
	long				header_pos;
	unsigned long		size;
	IFFChunkHeaderType	header;
	BOOL				fOdd;
	char				pad	= 0;

	header_pos	= tell (file);

	if (! OK_TELL(header_pos))
/**/	goto ABORT;

	header.TypeID.ID_Number	= CHUNK_NONE;
	header.DataSize		= 0L;

	if (! PutHeader (file, &header))
/**/	goto ABORT;

	size			= (*datafunction) (file);

	if (size == (unsigned long) CHUNK_ERROR)
/**/	goto ABORT;

	fOdd	= (BOOL) ODD(size);
	if (fOdd)
	{
		if (write (file, (void *) &pad, 1) != 1)
/**/		goto ABORT;
	}

	last_pos	= tell (file);

	if (! OK_TELL(last_pos))
/**/	goto ABORT;

	if (! OK_SEEK(lseek (file, header_pos, SEEK_SET)))
/**/	goto ABORT;

	header.TypeID.ID_Number	= typeid;
	header.DataSize		= size;

	if (! PutHeader (file, &header))
/**/	goto ABORT;

	size	+= IFFCH_SIZE;

	if (! OK_SEEK(lseek (file, last_pos, SEEK_SET)))
/**/	goto ABORT;

	if (fOdd) size ++;

	return size;
/*------------------------------------------------------------------------*/
ABORT:
	return 0;
} /* GenericWriteChunk */


/*********************************************************************
 *
 * WriteLAYR
 *
 * PURPOSE
 *		Write the current layer as a two dimensional array of tiles.
 *
 *		This is used within 'ROOM''DATA' and 'ROOM''LAYR' chunks.
 *
 * INPUT
 *		FileHandleType file		: where to write bytes to
 *		LayerType *playCurrent	: layer to write
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns the number of bytes written, or CHUNK_ERROR if error.
 *
 * HISTORY
 *		08/13/94 (dcc) - created from old WriteDataRData().
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
*/
static unsigned long WriteLAYR(FileHandleType file)
{
#if PLOTARRAY
	unsigned long	layersize;
#else
	UWORD		cbRowsize;
#endif
	unsigned long	ulSize	= 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteLAYR";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if PLOTARRAY
	layersize	= (ULONG) CurrentRoom->Width * CurrentRoom->Height * PLOTSIZE;
#else
	cbRowsize = playCurrent->ctilx * PLOTSIZE;
#endif

#if PLOTARRAY
	plot = playCurrent->Plot;

#if __AMIGAOS__
	if (write (file, (void *) plot, layersize) != layersize)
	{
/**/	goto ABORT;
	}
	return layersize;
#elif __MSDOS__	/*dcc*/
	{
		PlotType	*iplot;
		PlotType	*maxiplot;
		unsigned long result;

		maxiplot = plot + (layersize / PLOTSIZE);

		/* Flip data around to external format (HI..LO) */

		for (iplot = plot; iplot < maxiplot; iplot++)
			MakeBigWord(iplot->Tile_ID);

#if fSaveCompressed
		result = SDQWrite(file, (void *) plot, layersize);
#else
		result = write(file, (void *) plot, layersize);
#endif
		ulSize += result;

		/* Restore data to internal format (LO..HI) */

		for (iplot = plot; iplot < maxiplot; iplot++)
			BigWord2Native(iplot->Tile_ID);

#if fSaveCompressed
		if (result == -1)
/**/		goto ABORT;
#else
		if (result != layersize)
/**/		goto ABORT;
#endif
	}
	return ulSize;
#endif/*__AMIGAOS__/__MSDOS__*/
#else //!PLOTARRAY
	{
		int iyt;

		for (iyt = 0; iyt < playCurrent->ctily; iyt++)
		{
#if UseSparseArray
			if (IsSparse(playCurrent))
			{
				int ixt;

				for (ixt = 0; ixt < playCurrent->ctilx; ixt++)
				{
					PlotType plt;
					int cBytes;

					ReadSparsePlotXY(playCurrent->p.pspa, ixt, iyt, &plt);
					MakeBigWord(plt.Tile_ID);
#if fSaveCompressed
					if ((cBytes = SDQWrite(file, (void *) &plt, PLOTSIZE)) == -1)
/**/					goto ABORT;
#else
					if ((cBytes = write(file, (void *) &plt, PLOTSIZE)) != PLOTSIZE)
/**/					goto ABORT;
#endif
					ulSize += cBytes;
				}
			}
			else
			{
#if __LITTLEENDIAN__
				UWORD ixt;
#endif/*__LITTLEENDIAN__*/
				PlotType *pplt;
				int cBytes;

				pplt = ActivatePlotRowatWin(playCurrent->p.rgrgplt, iyt, winDst);
#if __LITTLEENDIAN__
				/* Flip data around to external format (HI..LO) */

				for (ixt = 0; ixt < playCurrent->ctilx; ixt++)
				{
					MakeBigWord(pplt[ixt].Tile_ID);
				}

#endif/*__LITTLEENDIAN__*/

#if fSaveCompressed
				cBytes = SDQWrite(file, (void *) pplt, cbRowsize);
#else
				cBytes = write(file, (void *) pplt, cbRowsize);
#endif
				ulSize += cBytes;
#if __LITTLEENDIAN__

				/* Restore data to internal format (LO..HI) */

				for (ixt = 0; ixt < playCurrent->ctilx; ixt++)
				{
					BigWord2Native(pplt[ixt].Tile_ID);
				}

				/* Don't call UpdatePlotRow(), since we didn't really want
			   	to change the data -- we just MakeBigWord'ed it to write it. */

				ReleasePlotRow(playCurrent->p.rgrgplt, iyt);
#endif/*__LITTLEENDIAN__*/
#if fSaveCompressed
				if (cBytes == -1)
/**/				goto ABORT;
#else
				if (cBytes != cbRowsize)
/**/				goto ABORT;
#endif
			}
#else // !UseSparseArray
#if __LITTLEENDIAN__
			UWORD ixt;
#endif/*__LITTLEENDIAN__*/
			PlotType *pplt;
			int cBytes;

			pplt = ActivatePlotRowatWin(playCurrent->rgrgplt, iyt, winDst);
#if __LITTLEENDIAN__
			/* Flip data around to external format (HI..LO) */

			for (ixt = 0; ixt < playCurrent->ctilx; ixt++)
			{
				MakeBigWord(pplt[ixt].Tile_ID);
			}

#endif/*__LITTLEENDIAN__*/

#if fSaveCompressed
			cBytes = SDQWrite(file, (void *) pplt, cbRowsize);
#else
			cBytes = write(file, (void *) pplt, cbRowsize);
#endif
			ulSize += cBytes;
#if __LITTLEENDIAN__

			/* Restore data to internal format (LO..HI) */

			for (ixt = 0; ixt < playCurrent->ctilx; ixt++)
			{
				BigWord2Native(pplt[ixt].Tile_ID);
			}

			/* Don't call UpdatePlotRow(), since we didn't really want
			   to change the data -- we just MakeBigWord'ed it to write it. */

			ReleasePlotRow(playCurrent->rgrgplt, iyt);
#endif/*__LITTLEENDIAN__*/
#if fSaveCompressed
			if (cBytes == -1)
/**/			goto ABORT;
#else
			if (cBytes != cbRowsize)
/**/			goto ABORT;
#endif
#endif // !UseSparseArray
		}
	}
	return ulSize;
#endif // !PLOTARRAY

ABORT:
	return CHUNK_ERROR;
} /* WriteLAYR */


#if SaveNew
#if UseSparseArray
/*********************************************************************
 *
 * WriteLAY1
 *
 * PURPOSE
 *		Write the current layer as:
 *		1. The number of tiles (to follow); and
 *		2. A list of tile coordinates & tile values.
 *		This is part of a 'LAY1' chunk.
 *
 * INPUT
 *		FileHandleType file		: where to write bytes to
 *		LayerType *playCurrent	: layer to write
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns the number of bytes written, or CHUNK_ERROR if error.
 *
 * HISTORY
 *		08/14/94 (dcc) - created.
 *		08/22/94 (dcc) - add support for UseSparseArray
 *
*/
static unsigned long WriteLAY1(FileHandleType file)
{
	ULONG	ulSize	= 0;
	ULONG	ulTiles	= 0;
	long		ulTilesPos;
	int		iyt;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteLAY1";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if PLOTARRAY
#error Not implemented!
#else // !PLOTARRAY

	if (!OK_TELL(ulTilesPos = tell(file)))	// mark location of # of tiles
		return CHUNK_ERROR;

	if (write(file, &ulTiles, sizeof(ULONG)) != sizeof(ULONG))	// dummy ULONG
		return CHUNK_ERROR;

#if fSaveCompressed
	InitSDQWrite();
#endif // fSaveCompressed

#if 1
#if UseSparseArray
	if (IsSparse(playCurrent))
	{
		PlotType *pplt;
		int ixt;

		SetSparseLayerLimits(playCurrent->p.pspa, 0, 0,
						 playCurrent->ctilx, playCurrent->ctily);

		while ((pplt = GetNextSparsePlotInRange(playCurrent->p.pspa,
										&ixt, &iyt)) != NULL)
		{
			XYTileBuffType xyt;
			int cBytes;

			xyt.dtilx	= ixt;
			xyt.dtily	= iyt;
			xyt.plt	= *pplt;

			MakeBigWord(xyt.dtilx);
			MakeBigWord(xyt.dtily);
			MakeBigWord(xyt.plt.Tile_ID);

#if fSaveCompressed
			if ((cBytes = SDQWrite(file, (void *) &xyt, XYTILEBUFFSIZE)) == -1)
				return CHUNK_ERROR;
#else
			if ((cBytes = write(file, &xyt, XYTILEBUFFSIZE)) != XYTILEBUFFSIZE)
				return CHUNK_ERROR;
#endif
			ulSize += cBytes;
			ulTiles++;
		}
	}
	else
	{
#endif // UseSparseArray
		for (iyt = 0; iyt < playCurrent->ctily; iyt++)
		{
			PlotType *pplt;
			unsigned ixt;

#if UseSparseArray
			pplt = ActivatePlotRowatWin(playCurrent->p.rgrgplt, iyt, winDst);
#else // !UseSparseArray
			pplt = ActivatePlotRowatWin(playCurrent->rgrgplt, iyt, winDst);
#endif // !UseSparseArray

			for (ixt = 0; ixt < playCurrent->ctilx; ixt++)
			{
				if (pplt->TileSet_ID != 0)
				{
					XYTileBuffType xyt;
					int cBytes;

					xyt.dtilx	= ixt;
					xyt.dtily	= iyt;
					xyt.plt	= *pplt;

					MakeBigWord(xyt.dtilx);
					MakeBigWord(xyt.dtily);
					MakeBigWord(xyt.plt.Tile_ID);

#if fSaveCompressed
					if ((cBytes = SDQWrite(file, (void *) &xyt, XYTILEBUFFSIZE)) == -1)
						return CHUNK_ERROR;
#else
					if ((cBytes = write(file, &xyt, XYTILEBUFFSIZE)) != XYTILEBUFFSIZE)
						return CHUNK_ERROR;
#endif
					ulSize += cBytes;
					ulTiles++;
				}
				pplt++;
			}
#if UseSparseArray
			ReleasePlotRow(playCurrent->p.rgrgplt, iyt);
#else // !UseSparseArray
			ReleasePlotRow(playCurrent->rgrgplt, iyt);
#endif // !UseSparseArray
		}
#if UseSparseArray
	}
#endif // UseSparseArray
#else // !1
	for (iyt = 0; iyt < playCurrent->ctily; iyt++)
	{
		PlotType *pplt;
		int ixt;

#if UseSparseArray
		if (!IsSparse(playCurrent))
			pplt = ActivatePlotRowatWin(playCurrent->p.rgrgplt, iyt, winDst);
#else // !UseSparseArray
		pplt = ActivatePlotRowatWin(playCurrent->rgrgplt, iyt, winDst);
#endif // !UseSparseArray

		for (ixt = 0; ixt < playCurrent->ctilx; ixt++)
		{
#if UseSparseArray
			if (IsSparse(playCurrent))
				pplt = GetSparsePlotXY(playCurrent->p.pspa, ixt, iyt);
#endif // UseSparseArray

			if (pplt->TileSet_ID != 0)
			{
				XYTileBuffType xyt;
				int cBytes;

				xyt.dtilx	= ixt;
				xyt.dtily	= iyt;
				xyt.plt	= *pplt;

				MakeBigWord(xyt.dtilx);
				MakeBigWord(xyt.dtily);
				MakeBigWord(xyt.plt.Tile_ID);

#if fSaveCompressed
				if ((cBytes = SDQWrite(file, (void *) &xyt, XYTILEBUFFSIZE)) == -1)
					return CHUNK_ERROR;
#else
				if ((cBytes = write(file, &xyt, XYTILEBUFFSIZE)) != XYTILEBUFFSIZE)
					return CHUNK_ERROR;
#endif
				ulSize += cBytes;
				ulTiles++;
			}
			pplt++;
		}
#if UseSparseArray
		if (!IsSparse(playCurrent))
			ReleasePlotRow(playCurrent->p.rgrgplt, iyt);
#else // !UseSparseArray
		ReleasePlotRow(playCurrent->rgrgplt, iyt);
#endif // !UseSparseArray
	}
#endif // !1
#if fSaveCompressed
	{
		int cBytes;

		cBytes = SDQWriteLast(file);
		if (!cBytes)
			return CHUNK_ERROR;

		ulSize += cBytes;
	}
#endif // fSaveCompressed
	{
		long ulLastPos;

		if (!OK_TELL(ulLastPos = tell(file)))
			return CHUNK_ERROR;

		if (!OK_SEEK(lseek(file, ulTilesPos, SEEK_SET)))
			return CHUNK_ERROR;

		MakeBigLong(ulTiles);

		if (write(file, &ulTiles, sizeof(ULONG)) != sizeof(ULONG))
			return CHUNK_ERROR;

		if (!OK_SEEK(lseek(file, ulLastPos, SEEK_SET)))
			return CHUNK_ERROR;

		ulSize += sizeof(ULONG);
	}
	return ulSize;
#endif // !PLOTARRAY
} /* WriteLAY1 */
#endif // UseSparseArray


/*********************************************************************
 *
 * WriteROOMLAYn
 *
 * PURPOSE
 *		Write the header for the current layer, then call
 *		either WriteLAYR() or WriteLAY1() to write <playCurrent>.
 *		To invoke, pass this function to GenericWriteChunk().
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns the number of bytes written, or CHUNK_ERROR if error.
 *
 * HISTORY
 *		08/14/94 Sunday (dcc) - created.
 *
*/
static unsigned long WriteROOMLAYn(FileHandleType file)
{
	LayerBuffType lyb;
	unsigned long ulSize = 0;
	unsigned long ulResult;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteROOMLAYn";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	lyb.LayerType		= 0;
	lyb.LayerNumber	= uwlayCurrent;
	lyb.ctilx			= playCurrent->ctilx;
	lyb.ctily			= playCurrent->ctily;
	lyb.cxTile		= playCurrent->cxTile;
	lyb.cyTile		= playCurrent->cyTile;

	MakeBigWord(lyb.LayerType);
	MakeBigWord(lyb.LayerNumber);
	MakeBigWord(lyb.ctilx);
	MakeBigWord(lyb.ctily);
	MakeBigWord(lyb.cxTile);
	MakeBigWord(lyb.cyTile);

	if (write (file, (void *) &lyb, LAYERBUFFSIZE) != LAYERBUFFSIZE)
		return CHUNK_ERROR;

	ulSize += LAYERBUFFSIZE;

#if UseSparseArray
	if (IsSparse(playCurrent))
		ulResult = WriteLAY1(file);
	else
		ulResult = WriteLAYR(file);
#else // !UseSparseArray
	ulResult = WriteLAYR(file);
#endif // !UseSparseArray

	if (ulResult == (unsigned long) CHUNK_ERROR)
		return CHUNK_ERROR;

	ulSize += ulResult;

	return ulSize;
} /* WriteROOMLAYn */


/*********************************************************************
 *
 * WriteROOMNAME
 *
 * PURPOSE
 *		Write the 'ROOM''NAME' chunk.
 *		To invoke, pass this function to GenericWriteChunk().
 *
 * INPUT
 *		FileHandleType file		: where to write bytes to
 *
 * ASSUMES
 *		<CurrentRoom> != NULL.
 *		If the room is a composite room, it is UNLOCKED.
 *
 * RETURN VALUE
 *		Returns the number of bytes written, or CHUNK_ERROR if error.
 *
 * HISTORY
 *		08/14/94 Sunday (dcc) - created.
 *
*/
static unsigned long WriteROOMNAME(FileHandleType file)
{
	UWORD		namelen;
	RoomBuff1Type	rb1;

	unsigned long	ulSize		= 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteROOMNAME";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/**********************/
	/* fill in the buffer */

	rb1.Flags	= (CurrentRoom->Flags & 0xFFFE) | fSaveCompressed;
	rb1.R_id	= CurrentRoom->R_id;
	if (IO_Options & ONLY_FLOOR)
	{
		rb1.LayerCount		= 1;
		rb1.FloorNumber	= 1;
	}
	else
	{
		rb1.LayerCount		= CurrentRoom->LayerCount;
		rb1.FloorNumber	= CurrentRoom->FloorNumber;
	}

	MakeBigWord(rb1.Flags);
	MakeBigWord(rb1.R_id);
	MakeBigWord(rb1.LayerCount);
	MakeBigWord(rb1.FloorNumber);

	/*********************************/
	/* write out the fixed size info */

	if (write (file, (void *) &rb1, ROOMBUFF1SIZE) != ROOMBUFF1SIZE)
		return CHUNK_ERROR;

	ulSize += ROOMBUFF1SIZE;

	/***************************/
	/* write out the room name */

	if (CurrentRoom->Name)
	{
		namelen = strlen (CurrentRoom->Name);
		if (namelen)
		{
			if (write (file,(void *) CurrentRoom->Name, namelen) != namelen)
				return CHUNK_ERROR;

			ulSize += namelen;
		}
	}
	return ulSize;
} /* WriteROOMNAME */


/*********************************************************************
 *
 * WriteROOMNAMEnLAYRs
 *
 * PURPOSE
 *		Write 'NAME' and 'LAYR' chunks for current room.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Number of bytes written, or 0 if error.
 *
 * HISTORY
 *		08/14/94 Sunday (dcc) - created from WriteDataRData().
 *
*/
static unsigned long WriteROOMNAMEnLAYRs(FileHandleType file)
{
	BOOL			fLocked;
	unsigned long	ulSize		= 0;
	unsigned long	ulResult;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteROOMNAMEnLAYRs";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (! CurrentRoom)
		return CHUNK_ERROR;

	fLocked = IsLocked(CurrentRoom);

	/* If saving a composite room,
	   make sure variables are saved in UNLOCKED state. */

	if (IsComposite(CurrentRoom) && fLocked)
	{
		SetUnlockedCompositeRoom(CurrentRoom);
	}

	if ((ulResult = GenericWriteChunk(file, CHUNK_NAME, WriteROOMNAME)) == 0)
	{
/**/	goto ABORT;
	}
	ulSize += ulResult;

	/* write out the room data 'LAYR' by 'LAYR' */

	if (IO_Options & ONLY_FLOOR)
	{
		playCurrent = CurrentRoom->FloorLayer;
	}
	else
	{
		playCurrent = Head(&CurrentRoom->Layers);
	}

	uwlayCurrent = 1;

	while (! IsEOList (playCurrent))
	{
		unsigned long ul;

		if (playCurrent->cxTile)	// only write iff tiles have been stamped in layer
		{
#if UseSparseArray
			if (IsSparse(playCurrent))
				ul = GenericWriteChunk(file, CHUNK_LAY1, WriteROOMLAYn);
			else
				ul = GenericWriteChunk(file, CHUNK_LAYR, WriteROOMLAYn);
#else // !UseSparseArray
			ul = GenericWriteChunk(file, CHUNK_LAYR, WriteROOMLAYn);
#endif // !UseSparseArray
			if (ul == (unsigned long) CHUNK_ERROR)
/**/			goto ABORT;
			ulSize += ul;
		}
		if (IO_Options & ONLY_FLOOR)
			break;
		else
		{
			playCurrent = Next(playCurrent);
			uwlayCurrent++;
		}
	}

	if (IsComposite(CurrentRoom) && fLocked)
	{
		SetLockedCompositeRoom(CurrentRoom);
	}
	return ulSize;

/*========================================================================*/
ABORT:

	if (IsComposite(CurrentRoom) && fLocked)
	{
		SetLockedCompositeRoom(CurrentRoom);
	}
	return 0;
} /* WriteROOMNAMEnLAYRs */
#endif // SaveNew


/**************************************************************************
**
** WriteDataRData
**
** USAGE
**	size = WriteDataRData (OUT_FILE)
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
**	10/18/89 Wednesday - Created. (RGM)
**	05/11/93 Tuesday (dcc) - enhanced to support ONLY_FLOOR flag.
**
** SEE ALSO
**
*/
static unsigned long WriteDataRData (FileHandleType file)
{
	UWORD		namelen;
	LayerType		*layer;
#if PLOTARRAY
	PlotType		*plot;
#endif
	RoomBuffType	roombuffer;
	BOOL			fLocked;

	unsigned long	size		= 0;

/**/if (! CurrentRoom)
		return CHUNK_ERROR;

	fLocked = IsLocked(CurrentRoom);

	/* If saving a composite room,
	   make sure variables are saved in UNLOCKED state. */

	if (IsComposite(CurrentRoom) && fLocked)
	{
		SetUnlockedCompositeRoom(CurrentRoom);
	}

	if (IO_Options & ONLY_FLOOR)
	{
		layer = CurrentRoom->FloorLayer;
	}
	else
	{
		layer = FindNonEmptyLayer(&CurrentRoom->Layers, CurrentRoom->FloorLayer);
		if (!layer)
			layer = Head(&CurrentRoom->Layers);
	}

	/**********************/
	/* fill in the buffer */

	roombuffer.Flags		= (CurrentRoom->Flags & 0xFFFE) | fSaveCompressed;
	roombuffer.R_id		= CurrentRoom->R_id;
	roombuffer.Width		= layer->ctilx;
	roombuffer.Height		= layer->ctily;
	roombuffer.TileWidth	= layer->cxTile;
	roombuffer.TileHeight	= layer->cyTile;
	if (IO_Options & ONLY_FLOOR)
	{
		roombuffer.LayerCount	= 1;
		roombuffer.FloorNumber	= 1;
	}
	else
	{
		roombuffer.LayerCount	= CurrentRoom->LayerCount;
		roombuffer.FloorNumber	= CurrentRoom->FloorNumber;
	}

	MakeBigWord(roombuffer.Flags);
	MakeBigWord(roombuffer.R_id);
	MakeBigWord(roombuffer.Width);
	MakeBigWord(roombuffer.Height);
	MakeBigWord(roombuffer.TileWidth);
	MakeBigWord(roombuffer.TileHeight);
	MakeBigWord(roombuffer.LayerCount);
	MakeBigWord(roombuffer.FloorNumber);

	/*********************************/
	/* write out the fixed size info */

	if (write (file, (void *) &roombuffer, ROOMBUFFSIZE) != ROOMBUFFSIZE)
	{
/**/	goto ABORT;
	}
	size += ROOMBUFFSIZE;

#if fSaveCompressed
	InitSDQWrite();
#endif // fSaveCompressed

	{
		/******************************************/
		/* write out the room data layer by layer */

		if (!(IO_Options & ONLY_FLOOR))
			layer = Head(&CurrentRoom->Layers);

		playCurrent = layer;

		while (! IsEOList (playCurrent))
		{
			unsigned long ul;

			ul = WriteLAYR(file);
			if (ul == (unsigned long) CHUNK_ERROR)
	/**/		goto ABORT;
			size += ul;

			if (IO_Options & ONLY_FLOOR)
				break;
			else
				playCurrent = Next (playCurrent);
		}
	}

#if fSaveCompressed
	{
		int cBytes;

		cBytes = SDQWriteLast(file);
		if (!cBytes)
		{
/**/		goto ABORT;
		}
		size += cBytes;
	}
#endif // fSaveCompressed

	/***************************/
	/* write out the room name */

	if (CurrentRoom->Name)
	{
		namelen = strlen (CurrentRoom->Name);
		if (namelen)
		{
			if (write (file,(void *) CurrentRoom->Name, namelen) != namelen)
			{
/**/			goto ABORT;
			}
			size += namelen;
		}
	}

	if (IsComposite(CurrentRoom) && fLocked)
	{
		SetLockedCompositeRoom(CurrentRoom);
	}
	return (size);

/*========================================================================*/
ABORT:

	if (IsComposite(CurrentRoom) && fLocked)
	{
		SetLockedCompositeRoom(CurrentRoom);
	}
	return (CHUNK_ERROR);
} /* WriteDataRData */


/**************************************************************************
**
** WriteUserRData
**
** USAGE
**	size = WriteUserRData (OUT_FILE);
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

static unsigned long WriteUserRData (FileHandleType file)
{
	UserInfoType	userinfo;
	unsigned long	size		= 0;

/**/if (! CurrentRoom) goto ABORT;

	/*********************************/
	/* write out the fixed size info */

	userinfo.UserType	= CurrentRoom->UserType;
	userinfo.UserNumber	= CurrentRoom->UserNumber;
	userinfo.UserExtra1	= 0;
	userinfo.UserExtra2	= 0;

	MakeBigWord(userinfo.UserType);
	MakeBigWord(userinfo.UserNumber);
	MakeBigWord(userinfo.UserExtra1);
	MakeBigWord(userinfo.UserExtra2);

	if (write (file, (void *) &userinfo, USERINFOSIZE) != USERINFOSIZE) {
/**/	goto ABORT;
	}

	size += USERINFOSIZE;

	return (size);
/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteUserRData */


/**************************************************************************
 *
 * WriteCmntData
 *
 * PURPOSE
 *		Write out room comment information.
 *		To invoke, pass this function to GenericWriteChunk().
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
 *	04/01/93 Thursday (dcc)	- created.
 *
 * SEE ALSO
 *
*/

static unsigned long WriteCmntData (FileHandleType file)
{
	unsigned long	size		= 0;
	WORD			i;

/**/if (! CurrentRoom) goto ABORT;

	/**************************/
	/* write out the comments */

	for (i = 0; i < 2; i++) {
		UWORD clen;

		if (CurrentRoom->Comment[i]) {
			clen = strlen (CurrentRoom->Comment[i]);
		}
		else {
			clen = 0;
		}

		MakeBigWord(clen);	/*dcc*/

		if (write (file, (void *) &clen, sizeof (UWORD)) != sizeof (UWORD)){
/**/		goto ABORT;
		}

		MakeBigWord(clen);	/*dcc*/

		if (clen) {
			if (write (file, (void *) CurrentRoom->Comment[i], clen) != clen) {
/**/			goto ABORT;
			}
		}

		size += sizeof (UWORD) + clen;
	}

	return (size);
/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteCmntData */


/**************************************************************************
**
** WriteCmapData
**
** USAGE
**	size = WriteCmapData (OUT_FILE);
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
**	10/19/89 Thursday - Created. (RGM)
**	02/26/92 Wednesday - Modified for new ColorInfo. (GAT)
**
** SEE ALSO
**
*/

static unsigned long WriteCmapData (FileHandleType file)
{
	ColorRegister	 out_color;
	CS_Color		*color_ptr = NULL;
	CS_Color		*max_color_ptr;
	unsigned long	 size = 0;

/**/if (! CurrentRoom) goto ABORT;

	/**************************/
	/* fill in the CMAP stuff */

	color_ptr	     = ActivateXTRA (CurrentRoom->R_ColorInfo->pxtColors);
	max_color_ptr  = color_ptr;
	max_color_ptr += CurrentRoom->R_ColorInfo->NumColors;

	for ( ; color_ptr < max_color_ptr; color_ptr ++) {
		out_color.red   = color_ptr->Red;
		out_color.green = color_ptr->Green;
		out_color.blue  = color_ptr->Blue;

		/*********************************/
		/* write out the fixed size info */

		if (write (file, (void *) &out_color, COLORREGSIZE) != COLORREGSIZE){
/**/		goto ABORT;
		}
	}

	size += COLORREGSIZE * CurrentRoom->R_ColorInfo->NumColors;

	ReleaseXTRA (CurrentRoom->R_ColorInfo->pxtColors);
	return (size);

/*========================================================================*/
ABORT:
	if (color_ptr)
		ReleaseXTRA (CurrentRoom->R_ColorInfo->pxtColors);
	return (CHUNK_ERROR);
} /* WriteCmapData */


/**************************************************************************
**
** WriteHsvpData
**
** USAGE
**	size = WriteHsvpData (OUT_FILE);
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
static unsigned long WriteHsvpData (FileHandleType file)
{
	HSVType			 out_hsv;
	CS_Color			*color_ptr = NULL;
	CS_Color			*max_color_ptr;
	unsigned long	size		= 0;

/**/if (! CurrentRoom) goto ABORT;

	/**************************/
	/* fill in the CMAP stuff */

	color_ptr	     = ActivateXTRA (CurrentRoom->R_ColorInfo->pxtColors);
	max_color_ptr  = color_ptr;
	max_color_ptr += CurrentRoom->R_ColorInfo->NumColors;

	for ( ; color_ptr < max_color_ptr; color_ptr ++) {
		out_hsv.Hue        = color_ptr->Hue;
		out_hsv.Saturation = color_ptr->Saturation;
		out_hsv.Value      = color_ptr->Value;

		MakeBigWord (out_hsv.Hue);

		/*********************************/
		/* write out the fixed size info */

		if (write (file, (void *) &out_hsv, HSVSIZE) != HSVSIZE) {
/**/		goto ABORT;
		}
	}

	size += HSVSIZE * CurrentRoom->R_ColorInfo->NumColors;

	ReleaseXTRA (CurrentRoom->R_ColorInfo->pxtColors);

	return (size);

/*========================================================================*/
ABORT:
	if (color_ptr)
		ReleaseXTRA (CurrentRoom->R_ColorInfo->pxtColors);
	return (CHUNK_ERROR);
} /* WriteHsvpData */


/**************************************************************************
**
** WriteCflgData
**
** USAGE
**	size = WriteCflgData (OUT_FILE);
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
**	10/19/89 Thursday - Created. (RGM)
**
** SEE ALSO
**
*/
static unsigned long WriteCflgData (FileHandleType file)
{
	CycleFlagType	out_cycle;
	unsigned long	size		= 0;

/**/if (! CurrentRoom) goto ABORT;

	/**************************/
	/* fill in the CMAP stuff */

	out_cycle = CurrentRoom->R_ColorInfo->CycleFlag;

	/*********************************/
	/* write out the fixed size info */

	MakeBigWord(out_cycle);	/*dcc*/

	if (write (file, (void *) &out_cycle, CYCLEFLAGSIZE) != CYCLEFLAGSIZE) {
/**/	goto ABORT;
	}
	size += CYCLEFLAGSIZE;

	return (size);

/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteCflgData */


#if 0 /* gat */
/**************************************************************************
**
** WriteCyclData
**
** USAGE
**	size = WriteCyclData (FileHandleType file);
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
**	10/21/89 Saturday - Created. (RGM)
**
** SEE ALSO
**
*/
static unsigned long WriteCyclData (FileHandleType file)
{
	ColorRegister		out_color;
	CycleColor			*cycle_color;
	WORD				colors;
	OtherCIinfoType		other;
	unsigned long		size		= 0;

/**/if (! CurrentRoom) goto ABORT;

	if (write (file, (void *) CurrentCycleInfo->Regs, CIARRAYSIZE) != CIARRAYSIZE) {
/**/	goto ABORT;
	}
	size += CIARRAYSIZE;

	other.Speed		= CurrentCycleInfo->Speed;
	other.Direction	= CurrentCycleInfo->Direction;
	other.Flag		= CurrentCycleInfo->Flag;

	MakeBigWord(other.Speed);	/*dcc*/
	MakeBigWord(other.Direction);
	MakeBigWord(other.Flag);

	if (write (file, (void *) &other, OTHERSIZE) != OTHERSIZE) {
/**/	goto ABORT;
	}
	size += OTHERSIZE;

	colors = 0;
	cycle_color = Head (&CurrentCycleInfo->ColorList);
	while (! IsEOList (cycle_color)) {

#if __AMIGAOS__
		out_color.red	=
			(RED_VAL(cycle_color->Color.ColorValue) << 4)
			| RED_VAL(cycle_color->Color.ColorValue);

		out_color.green	=
			(GREEN_VAL(cycle_color->Color.ColorValue) << 4)
			| GREEN_VAL(cycle_color->Color.ColorValue);

		out_color.blue	=
			(BLUE_VAL(cycle_color->Color.ColorValue) << 4)
			| BLUE_VAL(cycle_color->Color.ColorValue);
#elif __MSDOS__
#if 0
		out_color.red = (cycle_color->Color.Red * 255) / 63;
		out_color.green = (cycle_color->Color.Green * 255) / 63;
		out_color.blue = (cycle_color->Color.Blue * 255) / 63;
#endif
		out_color.red = cycle_color->Color.Red;
		out_color.green = cycle_color->Color.Green;
		out_color.blue = cycle_color->Color.Blue;
#endif/*__AMIGAOS__/__MSDOS__*/


		/*********************************/
		/* write out the fixed size info */

		if (write (file, (void *) &out_color, COLORREGSIZE) != COLORREGSIZE){
/**/		goto ABORT;
		}

		colors ++;

		cycle_color = Next (cycle_color);
	}

	size += COLORREGSIZE * colors;

	return (size);

/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteCyclData */
#endif /* gat */


/**************************************************************************
**
** WriteCinfData
**
** USAGE
**	size = WriteCinfData (FileHandleType file);
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
static unsigned long WriteCinfData (FileHandleType file)
{
	CINFType			cinf;
	CINFColorType		cinfcolor;
	UWORD			reg;
	CS_ColorTracker	*csct;
	CS_RegTracker		*csrt;
	unsigned long		size		= 0;

/**/if (! CurrentRoom) goto ABORT;

	cinf.Speed     = CurrentCycleInfo->Speed;
	cinf.Direction = CurrentCycleInfo->Direction;
	cinf.Flags     = CurrentCycleInfo->Flags;
	cinf.NumColors = CurrentCycleInfo->NumColors;
	cinf.NumRegs   = CurrentCycleInfo->NumRegs;

	MakeBigWord (cinf.Speed);
	MakeBigWord (cinf.Direction);
	MakeBigWord (cinf.Flags);
	MakeBigWord (cinf.NumColors);
	MakeBigWord (cinf.NumRegs);

	if (write (file, (void *) &cinf, CINFSIZE) != CINFSIZE) {
/**/	goto ABORT;
	}

	size += CINFSIZE;

	csct = Head (CurrentCycleInfo->ColorList);
	while (! IsEOList (csct)) {
		cinfcolor.Red        = csct->Color.Red;
		cinfcolor.Green      = csct->Color.Green;
		cinfcolor.Blue       = csct->Color.Blue;
		cinfcolor.Hue        = csct->Color.Hue;
		cinfcolor.Saturation = csct->Color.Saturation;
		cinfcolor.Value      = csct->Color.Value;

		MakeBigWord (cinfcolor.Hue);

		/*********************************/
		/* write out the fixed size info */

		if (write (file, (void *) &cinfcolor, CINFCOLORSIZE) != CINFCOLORSIZE){
/**/		goto ABORT;
		}

		size += CINFCOLORSIZE;

		csct = Next (csct);
	}

	csrt = Head (CurrentCycleInfo->RegList);
	while (! IsEOList (csrt)) {
		reg = csrt->Reg;

		MakeBigWord (reg);

		/*********************************/
		/* write out the fixed size info */

		if (write (file, (void *) &reg, sizeof (UWORD)) != sizeof (UWORD)){
/**/		goto ABORT;
		}

		size += sizeof (UWORD);

		csrt = Next (csrt);
	}

	return (size);

/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteCinfData */


/*********************************************************************
 *
 * WriteGridData
 *
 * PURPOSE
 *		Write out room grid information.
 *		To invoke, pass this function to GenericWriteChunk().
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
static unsigned long WriteGridData(FileHandleType file)
{

	GridInfoChunkType	GridInfo;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteGridData";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

/**/	if (! CurrentRoom) goto ABORT;

	GridInfo.wXWidth	= CurrentRoom->wGridXWidth;
	GridInfo.wYHeight	= CurrentRoom->wGridYHeight;
	GridInfo.wXOrigin	= CurrentRoom->wGridXOrigin;
	GridInfo.wYOrigin	= CurrentRoom->wGridYOrigin;
	GridInfo.bActive	= (IsGridOn(CurrentRoom) != 0);

	MakeBigWord(GridInfo.wXWidth);
	MakeBigWord(GridInfo.wYHeight);
	MakeBigWord(GridInfo.wXOrigin);
	MakeBigWord(GridInfo.wYOrigin);

	if (write (file, (void *) &GridInfo, GRIDINFOSIZE) != GRIDINFOSIZE)
/**/	goto ABORT;

	return GRIDINFOSIZE;

/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteGridData */


/*********************************************************************
 *
 * WriteGuidData
 *
 * PURPOSE
 *		Write out room guide information.
 *		To invoke, pass this function to GenericWriteChunk().
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
static unsigned long WriteGuidData(FileHandleType file)
{

	GridInfoChunkType	GuidInfo;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteGuidData";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

/**/if (! CurrentRoom) goto ABORT;

#if NEWTUME
	GuidInfo.wYHeight	= (WORD) (CurrentRoom->pcenGuideDC[0] / 100);
	GuidInfo.wXWidth	= (WORD) (CurrentRoom->pcenGuideDC[1] / 100);
	GuidInfo.wXOrigin	= (WORD) (CurrentRoom->pcenGuideXOrigin[1] / 100);
	GuidInfo.wYOrigin	= (WORD) (CurrentRoom->pcenGuideYOrigin[0] / 100);
#else
	GuidInfo.wXWidth	= CurrentRoom->wGuideXWidth;
	GuidInfo.wYHeight	= CurrentRoom->wGuideYHeight;
	GuidInfo.wXOrigin	= CurrentRoom->wGuideXOrigin;
	GuidInfo.wYOrigin	= CurrentRoom->wGuideYOrigin;
#endif
	GuidInfo.bActive	= (IsGuideVisible(CurrentRoom) != 0);

	MakeBigWord(GuidInfo.wXWidth);
	MakeBigWord(GuidInfo.wYHeight);
	MakeBigWord(GuidInfo.wXOrigin);
	MakeBigWord(GuidInfo.wYOrigin);

	if (write (file, (void *) &GuidInfo, GRIDINFOSIZE) != GRIDINFOSIZE)
/**/	goto ABORT;

	return GRIDINFOSIZE;

/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteGuidData */



#if NEWTUME
/*********************************************************************
 *
 * WriteGuideData
 *
 * PURPOSE
 *		Write out all room guide information (if there are more than
 *		two guide lines active).
 *		To invoke, pass this function to GenericWriteChunk().
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
static unsigned long WriteGuideData(FileHandleType file)
{
	GuideInfoChunk	GuideInfo;
	short i;
	LONG l = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteGuideData";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (! CurrentRoom)
/**/	goto ABORT;

	GuideInfo.unused[0] = 0;
	GuideInfo.unused[1] = 0;
	GuideInfo.unused[2] = 0;

	for (i = 0; i < CurrentRoom->cGuideLines; i++)
	{
		GuideInfo.GuideA		= CurrentRoom->pcenGuideA[i];
		GuideInfo.GuideB		= CurrentRoom->pcenGuideB[i];
		GuideInfo.GuideDC		= CurrentRoom->pcenGuideDC[i];
		GuideInfo.GuideXOrigin	= CurrentRoom->pcenGuideXOrigin[i];
		GuideInfo.GuideYOrigin	= CurrentRoom->pcenGuideYOrigin[i];
		GuideInfo.ShowGuide		= (BYTE) (CurrentRoom->fShowGuide[i] != 0);

		LongSex(GuideInfo.GuideA);
		LongSex(GuideInfo.GuideB);
		LongSex(GuideInfo.GuideDC);
		LongSex(GuideInfo.GuideXOrigin);
		LongSex(GuideInfo.GuideYOrigin);

		if (write (file, (void *) &GuideInfo, sizeof(GuideInfoChunk)) != sizeof(GuideInfoChunk))
/**/		goto ABORT;
		l += sizeof(GuideInfoChunk);
	}

	return l;

/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteGuideData */
#endif


/*********************************************************************
 *
 * WriteZoomData
 *
 * PURPOSE
 *		Write out room zoom information.
 *		To invoke, pass this function to GenericWriteChunk().
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
static unsigned long WriteZoomData(FileHandleType file)
{

	ZoomInfoChunkType	ZoomInfo;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteZoomData";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

/**/if (! CurrentRoom) goto ABORT;

	ZoomInfo.wDstDup	= wGetZoomDstDup(CurrentRoom->wZoomIndex);
	ZoomInfo.wSrcSkip	= wGetZoomSrcSkip(CurrentRoom->wZoomIndex);
	ZoomInfo.bActive	= (IsZooming(CurrentRoom) != 0);

	MakeBigWord(ZoomInfo.wDstDup);
	MakeBigWord(ZoomInfo.wSrcSkip);

	if (write (file, (void *) &ZoomInfo, ZOOMINFOSIZE) != ZOOMINFOSIZE)
/**/	goto ABORT;

	return ZOOMINFOSIZE;

/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteZoomData */


/**************************************************************************
**
** WriteRoomForm
**
** USAGE
**	size = WriteRoomForm (OUTFILE);
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
**		10/17/89 (RGM) - Created from WriteTumeForm.
**		08/14/94 (dcc) - Add call to WriteROOMNAMEnLAYRs().
**		10/14/94 (dcc) - Write out new format ROOM chunk if room contains
**					  ANY sparse layers.
**
** SEE ALSO
**
*/
static unsigned long WriteRoomForm (FileHandleType file)
{
	unsigned long	size;
	unsigned long	result;
	unsigned long	form_id;
	BOOL			status;

	size	= 0;
	status	= TRUE;

	form_id	= FORM_ROOM;
	result	= PutFormID (file, &form_id);
	size	+= result;
	status 	= (result != 0);

	if (status)
	{
#if SaveNew
		if (uwSaveFormat)
			result = WriteROOMNAMEnLAYRs(file);
		else
		{
			BOOL fWriteNew = FALSE;

			if (CurrentRoom)
			{
				LayerType *play = Head(&CurrentRoom->Layers);

				while (!IsEOList(play))
				{
					if (IsSparse(play))
					{
						fWriteNew = TRUE;
						break;
					}
					play = Next(play);
				}
			}

			if (fWriteNew)
				result = WriteROOMNAMEnLAYRs(file);
			else
				result = GenericWriteChunk (file, CHUNK_DATA, WriteDataRData);
		}
#else // !SaveNew
			result = GenericWriteChunk (file, CHUNK_DATA, WriteDataRData);
#endif // !SaveNew

		size	+= result;
		status	= (result != 0);
	}
	if (status)
	{
		result = GenericWriteChunk (file, CHUNK_USER, WriteUserRData);
		size	+= result;
		status	= (result != 0);
	}
	/* Only write out comments if there are comments */

	if (status)
	{
		if ((CurrentRoom->Comment[0] && *CurrentRoom->Comment[0]) ||
			 (CurrentRoom->Comment[1] && *CurrentRoom->Comment[1]))
		{
			result = GenericWriteChunk (file, CHUNK_CMNT, WriteCmntData);
			size	+= result;
			status	= (result != 0);
		}
	}
	if (status)
	{
		result = GenericWriteChunk (file, CHUNK_CMAP, WriteCmapData);
		size	+= result;
		status	= (result != 0);
	}
	if (status)
	{
		result = GenericWriteChunk (file, CHUNK_HSVP, WriteHsvpData);
		size	+= result;
		status	= (result != 0);
	}
	if (status)
	{
		result = GenericWriteChunk (file, CHUNK_CFLG, WriteCflgData);
		size	+= result;
		status	= (result != 0);
	}
	if (status)
	{
		CurrentCycleInfo = Head (CurrentRoom->R_ColorInfo->CycleList);
		while ((! IsEOList (CurrentCycleInfo)) && status) {
			result = GenericWriteChunk (file, CHUNK_CINF, WriteCinfData);
			size	+= result;
			status	= (result != 0);
			CurrentCycleInfo = Next (CurrentCycleInfo);
		}
	}
	if (status)
	{
		result = GenericWriteChunk (file, CHUNK_GRID, WriteGridData);
		size	+= result;
		status	= (result != 0);
	}
	if (status)
	{
		result = GenericWriteChunk (file, CHUNK_GUID, WriteGuidData);
		size	+= result;
		status	= (result != 0);
	}
#if NEWTUME
	if (status)
	{
		if (CurrentRoom->cGuideLines > 2)
		{
			BOOL fDoSave = FALSE;
			short i;

			for (i = 2; i < CurrentRoom->cGuideLines; i++)
			{
				if (CurrentRoom->pcenGuideA[i] != 0 || CurrentRoom->pcenGuideB[i] != 0)
				{
					fDoSave = TRUE;
					break;
				}
			}
			if (fDoSave)
			{
				result = GenericWriteChunk (file, CHUNK_guid, WriteGuideData);
				size	+= result;
				status	= (result != 0);
			}
		}
	}
#endif
	if (status)
	{
		result = GenericWriteChunk (file, CHUNK_ZOOM, WriteZoomData);
		size	+= result;
		status	= (result != 0);
	}
	if (status)
	{
		return size;
	}
	else
	{
		return CHUNK_ERROR;
	}
} /* WriteRoomForm */


/**************************************************************************
**
** WriteDataTData
**
** USAGE
**	size = WriteDataTData (OUT_FILE);
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
**	10/20/89 Friday - Created. (RGM)
**
** SEE ALSO
**
*/
static unsigned long WriteDataTData (FileHandleType file)
{
	TS_DataHeaderType	ts_dataheader;
	UWORD			namelen;
	unsigned long		size		= 0;

/**/if (! CurrentTileSet) goto ABORT;

	/***********************************/
	/* save the tileset id # and flags */

	ts_dataheader.TSDH_id		= CurrentTileSet->TS_id;
	ts_dataheader.TSDH_flags	= CurrentTileSet->Flags;

	MakeBigWord(ts_dataheader.TSDH_id);
	MakeBigWord(ts_dataheader.TSDH_flags);

	if (write (file, (void *) &ts_dataheader, TSDHSIZE) != TSDHSIZE) {
/**/	goto ABORT;
	}
	size += TSDHSIZE;

	/********************************************************/
	/* KLUDGE to write extra info needed for NINTENDO stuff */

	if ((CurrentTileSet->Flags & GRID_TYPE_MODIFIER) != GRID_IS_BOXED) {
		WORD	width	= CurrentTileSet->Width;
		WORD	height	= CurrentTileSet->Height;

		MakeBigWord(width);
		MakeBigWord(height);

		if (write (file, (void *) &width, sizeof(WORD)) != sizeof(WORD)){
/**/		goto ABORT;
		}

		if (write (file, (void *) &height, sizeof(WORD)) != sizeof(WORD)){
/**/		goto ABORT;
		}

		size += (2 * sizeof(WORD));
	}

	/********************************************/
	/* save the tileset source picture filename */

	if (CurrentTileSet->Filespec) {
		namelen = strlen (CurrentTileSet->Filespec);

		if (write (file, (void *) CurrentTileSet->Filespec, namelen) != namelen){
/**/		goto ABORT;
		}
		size += namelen;
	}
	return (size);

/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteDataTData */


/**************************************************************************
**
** WriteUserTData
**
** USAGE
**	size = WriteUserTData (OUT_FILE);
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
**	11/03/89 Friday - Created out of WriteUserRData. (RGM)
**	03/12/90 Monday - Added support for two comments. (RGM)
**	04/01/93 Thursday (dcc)	- only write out comments if they exist.
**
** SEE ALSO
**
*/
static unsigned long WriteUserTData (FileHandleType file)
{
	UserInfoType	userinfo;
	unsigned long	size		= 0;

/**/if (! CurrentTileSet) goto ABORT;

	/*********************************/
	/* write out the fixed size info */

	userinfo.UserType	= CurrentTileSet->UserType;
	userinfo.UserNumber	= CurrentTileSet->UserNumber;
	userinfo.UserExtra1	= CurrentTileSet->SourceRoom->R_id;
	userinfo.UserExtra2	= CurrentTileSet->TileCount;

	MakeBigWord(userinfo.UserType);
	MakeBigWord(userinfo.UserNumber);
	MakeBigWord(userinfo.UserExtra1);
	MakeBigWord(userinfo.UserExtra2);

	if (write (file, (void *) &userinfo, USERINFOSIZE) != USERINFOSIZE) {
/**/	goto ABORT;
	}

	size += USERINFOSIZE;

	if ((CurrentTileSet->Comment[0] && *CurrentTileSet->Comment[0]) ||
		 (CurrentTileSet->Comment[1] && *CurrentTileSet->Comment[1])) {

		/**************************/
		/* write out the comments */

		WORD			i;
		for (i = 0; i < 2; i++) {
			UWORD clen;

			if (CurrentTileSet->Comment[i]) {
				clen = strlen (CurrentTileSet->Comment[i]);
			}
			else {
				clen = 0;
			}

			MakeBigWord(clen);	/*dcc*/

			if (write (file, (void *) &clen, sizeof (UWORD)) != sizeof (UWORD)){
/**/			goto ABORT;
			}

			MakeBigWord(clen);	/*dcc*/

			if (clen) {
				if (write (file, (void *) CurrentTileSet->Comment[i], clen) != clen) {
/**/				goto ABORT;
				}
			}

			size += sizeof (UWORD) + clen;
		}
	}

	return (size);
/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteUserTData */


#if __MSDOS__
/**************************************************************************
**
** WriteTmgcTData
**
** USAGE
**	size = WriteTmgcTData (OUT_FILE);
**
** FUNCTION
**		Write chunky pixel version of tile images.
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
**	09/26/90 Wednesday (RGM) - Created.
**	05/27/91 Monday (DCC) - Created from RGM's WriteTmgxTData
**
** SEE ALSO
**
*/
typedef struct {
	UWORD		count;
	UWORD		width;
	UWORD		height;
	UWORD		depth;
	UWORD		trans;
} TmgcHeaderType;

static unsigned long WriteTmgcTData (FileHandleType file)
{
	TmgcHeaderType	tmgc_header;
	int			rassize;
	unsigned long	size		= 0;
	UWORD		TileNumber;

/**/if (! CurrentTileSet) goto ABORT;

	tmgc_header.count	= CurrentTileSet->TileCount;
	tmgc_header.width	= CurrentTileSet->Width;
	tmgc_header.height	= CurrentTileSet->Height;
	tmgc_header.depth	= CurrentTileSet->Depth;
	tmgc_header.trans	= CurrentTileSet->SharedtransparentColor;

	MakeBigWord(tmgc_header.count);
	MakeBigWord(tmgc_header.width);
	MakeBigWord(tmgc_header.height);
	MakeBigWord(tmgc_header.depth);
	MakeBigWord(tmgc_header.trans);

	if (write (	file, (void *) &tmgc_header, sizeof(TmgcHeaderType)) !=
										sizeof(TmgcHeaderType)) {
/**/	goto ABORT;
	}

	size += sizeof(TmgcHeaderType);

	rassize = CurrentTileSet->Width * CurrentTileSet->Height;

	for (TileNumber = 0; TileNumber < CurrentTileSet->TileCount; TileNumber++)
	{
		UBYTE *pBMData;
		UWORD col;
		UWORD idx;

		{
			col = TileNumber / CurrentTileSet->tilesPerCol;
			idx = TileNumber % CurrentTileSet->tilesPerCol;

			pBMData = (UBYTE *) ActivateXTRAatWin(CurrentTileSet->pxtrTilCol[col], winTil)
										+ idx * CurrentTileSet->sizeofTile;
		}

		{
			short i = write (file, (void *) pBMData, rassize);

			ReleaseXTRA(CurrentTileSet->pxtrTilCol[col]);

			if (i != rassize)
			{
/**/			goto ABORT;
			}
		}

		size += rassize;
	}

	return (size);
/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteTmgcTData */
#endif/*__MSDOS__*/


/**************************************************************************
**
** WriteTmgxTData
**
** USAGE
**	size = WriteTmgxTData (OUT_FILE);
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
**	09/26/90 Wednesday (RGM) - Created.
**
** SEE ALSO
**
*/
typedef struct {
	UWORD		count;
	UWORD		width;
	UWORD		height;
	UWORD		depth;
	UWORD		trans;
} TmgxHeaderType;

static unsigned long WriteTmgxTData (FileHandleType file)
{
	TmgxHeaderType	tmgx_header;
#if __AMIGAOS__
	TileType		*next_tile;
	short			depth;
	int				rassize;
#elif __MSDOS__
	UWORD			TileNumber = 0;
	UWORD			bytesperline;
#endif/*__AMIGAOS__/__MSDOS__*/
	unsigned long	size		= 0;

/**/if (! CurrentTileSet) goto ABORT;

	tmgx_header.count	= CurrentTileSet->TileCount;
	tmgx_header.width	= CurrentTileSet->Width;
	tmgx_header.height	= CurrentTileSet->Height;
	tmgx_header.depth	= CurrentTileSet->Depth;
#if __AMIGAOS__
	tmgx_header.trans	= PicTColor(ShapePic(CurrentTileSet->SharedShape));
#elif __MSDOS__
	tmgx_header.trans	= CurrentTileSet->SharedtransparentColor;
#endif/*__AMIGAOS__/__MSDOS__*/

	MakeBigWord(tmgx_header.count);
	MakeBigWord(tmgx_header.width);
	MakeBigWord(tmgx_header.height);
	MakeBigWord(tmgx_header.depth);
	MakeBigWord(tmgx_header.trans);

	if (write (	file, (void *) &tmgx_header, sizeof(TmgxHeaderType)) !=
													sizeof(TmgxHeaderType)) {
/**/	goto ABORT;
	}

	size += sizeof(TmgxHeaderType);

#if __AMIGAOS__
	depth	= CurrentTileSet->Depth;
	rassize = RASSIZE (CurrentTileSet->Width, CurrentTileSet->Height);
#elif __MSDOS__
	bytesperline = ((CurrentTileSet->Width + 15) / 16) * 2;	/* pad to nearest word */
#endif/*__MSDOS__*/

#if __AMIGAOS__
	next_tile	= Head (&(CurrentTileSet->Tiles));
	while (! IsEOList (next_tile)) {
		short	plane;

		for (plane = 0; plane < depth; plane ++) {

			if (write (file, (void *) next_tile->ImageBitMap.Planes[plane],
													rassize) != rassize) {
/**/			goto ABORT;
			}

			size += rassize;

		}
		next_tile = Next (next_tile);
	}
#elif __MSDOS__
	for (TileNumber = 0; TileNumber < CurrentTileSet->TileCount; TileNumber++)
	{
		UWORD plane, scanline, bytecount;
		UWORD pixelcount;
		UBYTE pixels[256];					/* Buffer to store bit planes */
		UBYTE *pSavePixels = pixels;
		UBYTE *pBMData;
		UWORD col;
		UWORD idx;

		{
		col = TileNumber / CurrentTileSet->tilesPerCol;
		idx = TileNumber % CurrentTileSet->tilesPerCol;

		pBMData = (UBYTE *) ActivateXTRAatWin(CurrentTileSet->pxtrTilCol[col], winTil)
									+ idx * CurrentTileSet->sizeofTile;
		}
		for (plane = 0; plane < CurrentTileSet->Depth; plane++)	/* every plane */
		{
			UBYTE bBit = 1 << plane;

			for (scanline = 0; scanline < CurrentTileSet->Height; scanline++)	/* every line */
			{
				for (bytecount = 0; bytecount < bytesperline; bytecount++)	/* every byte */
				{
					register UBYTE *pPix;
					register UWORD uByte8 = bytecount * 8;
					register UBYTE pixel = 0;

					pPix =	pBMData +
								scanline * CurrentTileSet->Width + uByte8;

					for (pixelcount = 0; pixelcount < 8; pixelcount++)	/* every pixel */
					{
						pixel <<= 1;
						if (uByte8 + pixelcount < CurrentTileSet->Width)
						{
							pixel |= (*(pPix + pixelcount) & bBit) >> plane;
						}
					}
					pSavePixels[bytecount] = pixel;
				}
				pSavePixels += bytesperline;
				size += bytesperline;
				if (pSavePixels - pixels + bytesperline > 256)
				{
					if (write (file, pixels, (unsigned) (pSavePixels-pixels)) != pSavePixels-pixels)
					{
						ReleaseXTRA(CurrentTileSet->pxtrTilCol[col]);
/**/					goto ABORT;
					}
					pSavePixels = pixels;
				}
			}
		}
		ReleaseXTRA(CurrentTileSet->pxtrTilCol[col]);

		if (pSavePixels != pixels)
		{
			if (write (file, pixels, (unsigned) (pSavePixels-pixels)) != pSavePixels-pixels)
			{
/**/			goto ABORT;
			}
		}
	}
#endif/*__AMIGAOS__/__MSDOS__*/

	return (size);
/*========================================================================*/
ABORT:
	return (CHUNK_ERROR);
} /* WriteTmgxTData */


/**************************************************************************
**
** WriteTsetForm
**
** USAGE
**	size = WriteTsetForm (OUTFILE);
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
**	10/17/89 Tuesday - Created form WriteTumeForm. (RGM)
**
** SEE ALSO
**
*/
static unsigned long WriteTsetForm (FileHandleType file)
{
	unsigned long	size;
	unsigned long	result;
	unsigned long	form_id;
	BOOL			status;

	size	= 0;
	status	= TRUE;

	form_id	= FORM_TSET;
	result	= PutFormID (file, &form_id);
	size	+= result;
	status 	= (result != 0);

	if (status) {
		result = GenericWriteChunk (file, CHUNK_DATA, WriteDataTData);
		size	+= result;
		status	= (result != 0);
	}
	if (status) {
		result = GenericWriteChunk (file, CHUNK_USER, WriteUserTData);
		size	+= result;
		status	= (result != 0);
	}
	if (status) {
		result = GenericWriteChunk (file, CHUNK_GRID, WriteGridData);
		size	+= result;
		status	= (result != 0);
	}
	if (status) {
		result = GenericWriteChunk (file, CHUNK_GUID, WriteGuidData);
		size	+= result;
		status	= (result != 0);
	}
#if NEWTUME
	if (status)
	{
		if (CurrentRoom->cGuideLines > 2)
		{
			BOOL fDoSave = FALSE;
			short i;

			for (i = 2; i < CurrentRoom->cGuideLines; i++)
			{
				if (CurrentRoom->pcenGuideA[i] != 0 || CurrentRoom->pcenGuideB[i] != 0)
				{
					fDoSave = TRUE;
					break;
				}
			}
			if (fDoSave)
			{
				result = GenericWriteChunk (file, CHUNK_guid, WriteGuideData);
				size	+= result;
				status	= (result != 0);
			}
		}
	}
#endif
	if (status) {
		result = GenericWriteChunk (file, CHUNK_ZOOM, WriteZoomData);
		size	+= result;
		status	= (result != 0);
	}
	if (status) {
		if ((CurrentTileSet->Flags & GRID_TYPE_MODIFIER) != TILESET_COMPOSITE) {
			if (IO_Options & TMGX_MAP) {
				result = GenericWriteChunk (file, CHUNK_TMGX, WriteTmgxTData);
				size	+= result;
				status	= (result != 0);
			} else if (IO_Options & TMGC_MAP) {
				result = GenericWriteChunk (file, CHUNK_TMGC, WriteTmgcTData);
				size	+= result;
				status	= (result != 0);
			}
		}
	}
	if (status) {
		return (size);
	}
	else {
		return (CHUNK_ERROR);
	}
} /* WriteTsetForm */


/**************************************************************************
**
** WriteTumeForm
**
** USAGE
**	size = WriteTumeForm (OUTFILE);
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
**	Modifies CurrentRoom & CurrentTileSet.
**
** BUGS
**
**
** HISTORY
**
**	10/17/89 Tuesday - Created. (RGM)
**
** SEE ALSO
**
*/
static unsigned long WriteTumeForm (FileHandleType file)
{
	unsigned long	size;
	unsigned long	result;
	unsigned long	form_id;
	UWORD			map_sflag;
	UWORD			room_sflag;
	UWORD			tilespace_sflag;
	UWORD			tileset_sflag;
	BOOL			status;

	size			= 0;
	status			= TRUE;
	map_sflag 		= GlobalMap->SaveFlags;
	tilespace_sflag	= GlobalTileSpace->SaveFlags;

	form_id	= FORM_TUME;
	result	= PutFormID (file, &form_id);
	size	+= result;

	status 	= (result != 0);

	if (status) {
		{
			/************************************/
			/* Write out all the 'source' rooms */

			CurrentRoom	= Head (&GlobalMap->Rooms);
			while ((! IsEOList (CurrentRoom)) && status) {
				room_sflag = CurrentRoom->SaveFlags;

				if ( (!(CurrentRoom->Flags & SOURCEROOM)) &&
										(map_sflag == room_sflag) ) {

					result	= GenericWriteChunk (
								file,
								CHUNK_FORM,
								WriteRoomForm);

					size	+= result;
					status	= (result != 0);
				}
				CurrentRoom = Next (CurrentRoom);
			}
		}

		CurrentTileSet	= Head (&GlobalTileSpace->TileSets);
		while ((! IsEOList (CurrentTileSet)) && status) {
			tileset_sflag = CurrentTileSet->SaveFlags;
			CurrentRoom = CurrentTileSet->SourceRoom;

			if (tilespace_sflag ==  tileset_sflag) {
				result	= GenericWriteChunk (file, CHUNK_FORM, WriteTsetForm);
				size	+= result;
				status	= (result != 0);
			}
			CurrentTileSet = Next(CurrentTileSet);
		}
	}

	if (status) {
		return (size);
	}
	else {
		return (CHUNK_ERROR);
	}
} /* WriteTumeForm */


/**************************************************************************
**
** WriteTumeChunk
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
**
** SEE ALSO
**
*/
static unsigned long WriteTumeChunk (FileHandleType file)
{
	unsigned long result;

	result = GenericWriteChunk (file, CHUNK_FORM, WriteTumeForm);
	if (result)
		return (result);
	else
		return (CHUNK_ERROR);
} /* WriteTumeChunk */


/**************************************************************************
**
** SaveMap
**
** USAGE
**	err	= SaveMap (SAVE_FILE_NAME, options);
**
**	<options> bits may be a combination of the following values:
**
**	0		- no special save options
**
**	TMGC_MAP	- save TMGC chunks as well
**	TMGX_MAP	- save TMGX chunks as well
**
**	ONLY_FLOOR- save only floor layers of rooms
**
**	Usually, if TMGC_MAP is set, then TMGX_MAP is not, and vice-versa.
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
short SaveMap (
	char			*filename,
	UWORD		 options
)
{
	FileHandleType outfile;
	unsigned long result;

	IO_Options		= options;

#if REAL
	CurrentRoom		= NULL;
	CurrentTileSet		= NULL;
	CurrentColorInfo	= NULL;
	CurrentCycleInfo	= NULL;
	CurrentUserInfo	= NULL;

	MARK_CLOSE(outfile);

	outfile = open (filename, OUTMODE, PROT_FLAGS);

	if (! OK_OPEN(outfile))
/**/	goto ABORT;

	TopRoom		= RenumberRooms (GlobalMap);

	result = WriteTumeChunk (outfile);

	if (result == (unsigned long) CHUNK_ERROR)
/**/	goto ABORT;

	close (outfile);
	MARK_CLOSE(outfile);
#endif/* REAL */
	return (TRUE);

/*========================================================================*/
ABORT:
	if (OK_OPEN(outfile)) {
		close (outfile);
		MARK_CLOSE(outfile);
	}
	return (FALSE);
} /* SaveMap */
#endif/*__INPUTONLY__*/
#endif/*fDoSaveRooms*/

