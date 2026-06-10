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
 * RWTUME.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 08/13/94
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Stuff that should only be #include'd by READ.C and WRITE.C.
 *
 * HISTORY
 *		08/13/94 Saturday (dcc) - created.
 *		08/13/94 Saturday (dcc) - add support to write 'LAYR' chunk.
 *
*/

#ifndef RWTUME_H
#define RWTUME_H

#define GET_OK_FROM_USER_TO_LOAD_OLD_TILESET_AS_NEW 0

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>
#include <echidna/strings.h>

#include <fcntl.h>
#include <limits.h>

#if __MSDOS__
#include <sys/stat.h>
#include <io.h>
#endif/*__MSDOS__*/

#if __AMIGAOS__
#if LATTICE
#include <error.h>
#include <stdio.h>
#endif/* LATTICE */
#endif/*__AMIGAOS__*/

#include "tilebits.h"
#include "rectplot.h"

#if __INPUTONLY__
#include "tpackio.h"
#include "tudef.h"
#else
#include "tuglbl.h"
#endif/*__INPUTONLY__*/

#if __AMIGAOS__
#include <iff/ilbm.h>
#include <echidna/eio.h>
#endif/*__AMIGAOS__*/

#include <echidna/eerrors.h>
#include "rm_tsinf.h"
#if NEWTUME
#include "grid.h"
#endif

#include "sdqio.h"
#include "assert.h"

/**************************** C O N S T A N T S ***************************/

#define REAL	TRUE

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#if __MSDOS__
#define INMODE			(O_RDONLY | O_BINARY)
#define OUTMODE		(O_WRONLY | O_CREAT | O_TRUNC | O_BINARY)
#endif/*__MSDOS__*/

#if __AMIGAOS__
#define INMODE			(O_RDONLY)
#define OUTMODE		(O_WRONLY | O_CREAT | O_TRUNC)

#define WordSex(a)
#define LongSex(a)
#endif/*__AMIGAOS__*/

#define	PROT_FLAGS	(S_IWRITE | S_IREAD)

#define OK_OPEN(ffh)	((ffh) >= 0)
#define OK_CLOSE(cl)	((cl) == 0)
#define OK_SEEK(pos)	((pos) != (-1L))
#define OK_TELL(pos)	((pos) != (-1L))
#define MARK_CLOSE(t)	t = -1

#define	FileHandleType	int

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#define ROOMBUFFSIZE		sizeof (RoomBuffType)
#if SaveNew
#define ROOMBUFF1SIZE	sizeof (RoomBuff1Type)
#define LAYERBUFFSIZE	sizeof (LayerBuffType)
#define XYTILEBUFFSIZE	sizeof (XYTileBuffType)
#endif //SaveNew
#define IFFCH_SIZE		sizeof (IFFChunkHeaderType)
#define ULONGSIZE			sizeof (unsigned long)
#define OTHERSIZE			sizeof (OtherCIinfoType)
#define USERINFOSIZE		sizeof (UserInfoType)
#define COLORREGSIZE		sizeofColorRegister
#define HSVSIZE			sizeof (HSVType)
#define CINFSIZE			sizeof (CINFType)
#define CINFCOLORSIZE	sizeof (CINFColorType)
#define GRIDINFOSIZE		sizeof (GridInfoChunkType)
#define ZOOMINFOSIZE		sizeof (ZoomInfoChunkType)

//*dcc*/ Yo, Dan! CIARRAYSIZE should be 514 to save all colors ! ! ! !

#define CIARRAYSIZE 66

#define CHUNK_FORM	MakeID ('F','O','R','M')

#define CHUNK_DATA	MakeID ('D','A','T','A')
#define CHUNK_USER	MakeID ('U','S','E','R')
#define CHUNK_CMNT	MakeID ('C','M','N','T')

#define CHUNK_TMGC	MakeID ('T','M','G','C')
#define CHUNK_TMGX	MakeID ('T','M','G','X')

#define CHUNK_CMAP	MakeID ('C','M','A','P')
#define CHUNK_CFLG	MakeID ('C','F','L','G')
#define CHUNK_CYCL	MakeID ('C','Y','C','L')
#define CHUNK_CINF	MakeID ('C','I','N','F')
#define CHUNK_HSVP	MakeID ('H','S','V','P')
#define CHUNK_GRID	MakeID ('G','R','I','D')
#define CHUNK_GUID	MakeID ('G','U','I','D')
#define CHUNK_guid	MakeID ('g','u','i','d')
#define CHUNK_ZOOM	MakeID ('Z','O','O','M')
#define CHUNK_NONE	MakeID ('N','O','N','E')
#if SaveNew
#define CHUNK_NAME	MakeID ('N','A','M','E')
#define CHUNK_LAYR	MakeID ('L','A','Y','R')
#define CHUNK_LAY1	MakeID ('L','A','Y','1')
#endif //SaveNew

#define FORM_TUME		MakeID ('t','U','M','E')
#define FORM_ROOM		MakeID ('R','O','O','M')
#define FORM_TSET		MakeID ('T','S','E','T')

#define ID_SIZE		4

#define CHUNK_ERROR		(-1L)

#define ODD(arg) ((arg) & (1L))

#define CycleFlagType	WORD

#define TSDHSIZE		sizeof (TS_DataHeaderType)
#define CYCLEFLAGSIZE	sizeof (CycleFlagType)

/******************************** T Y P E S *******************************/

typedef struct {
	UWORD	Hue;
	UBYTE	Saturation;
	UBYTE	Value;
} HSVType;

typedef struct {
	UBYTE	Red;
	UBYTE	Green;
	UBYTE	Blue;
	UWORD	Hue;
	UBYTE	Saturation;
	UBYTE	Value;
} CINFColorType;

typedef struct {
	WORD		Speed;
	WORD		Direction;
	UWORD	Flags;
	WORD		NumColors;
	WORD		NumRegs;
} CINFType;

typedef struct {
	WORD	TSDH_id;
	WORD	TSDH_flags;
} TS_DataHeaderType;

typedef union {
	char			ID_Name [4];
	unsigned long	ID_Number;
} IFFidType;

typedef struct {
	IFFidType		TypeID;
	unsigned long	DataSize;
} IFFChunkHeaderType;

typedef struct {
	UWORD	Flags;
	WORD	R_id;
	WORD	Width;
	WORD	Height;
	UWORD	TileWidth;
	UWORD	TileHeight;
	UWORD	LayerCount;
	UWORD	FloorNumber;
} RoomBuffType;

#if SaveNew
typedef struct {
	UWORD	Flags;
	WORD	R_id;
	UWORD	LayerCount;
	UWORD	FloorNumber;
} RoomBuff1Type;

typedef struct
{
	UWORD	LayerType;
	UWORD	LayerNumber;
	UWORD	ctilx;
	UWORD	ctily;
	UWORD	cxTile;
	UWORD	cyTile;
} LayerBuffType;

typedef struct
{
	WORD	dtilx;
	WORD	dtily;
	PlotType plt;
} XYTileBuffType;
#endif //SaveNew

typedef struct {
	WORD Speed;
	WORD Direction;
	WORD Flag;
} OtherCIinfoType;

typedef struct {
	WORD		UserType;
	WORD		UserNumber;
	WORD		UserExtra1;
	WORD		UserExtra2;
} UserInfoType;

typedef struct
{
	WORD		wXWidth;
	WORD		wYHeight;
	WORD		wXOrigin;
	WORD		wYOrigin;
	BYTE		bActive;
	BYTE		unused;
} GridInfoChunkType;

typedef struct
{
	LONG		GuideA;
	LONG		GuideB;
	LONG		GuideDC;
	LONG		GuideXOrigin;
	LONG		GuideYOrigin;
	BYTE		ShowGuide;
	BYTE		unused[3];
} GuideInfoChunk;

typedef struct
{
	WORD		wDstDup;
	WORD		wSrcSkip;
	BYTE		bActive;
} ZoomInfoChunkType;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/


#endif /* RWTUME_H */

