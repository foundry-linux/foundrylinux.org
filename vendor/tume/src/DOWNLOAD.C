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
 * DOWNLOAD.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 02/03/92
 *   MODIFIED : 01/22/95
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
#if dvpSNASM
 *		Download current screen to SNASM-connected SNES or Genesis.
#elif dvpPsyQ
 *		Download current screen to Psy-Q-connected SNES or Genesis.
#elif dvpCLD
 *		Download current screen to Chip Level Designs-connected SNES.
#elif dvpPARALLEL
 *		Download current screen to parallel port connected Genesis.
#elif dvpNONE
 *		No downloading.
#endif
 *
 * HISTORY
 *		02/03/92 (dcc) - Created.
 *		11/12/92 (dcc) - Support added for 256 color characters.
 *		01/22/93 (dcc) - Support for no downloading added.
 *		01/28/93 (dcc) - Support for Genesis downloading added.
 *		04/14/93 (dcc) - Don't throw away 32K per 64K in Genesis mode
 *							  in DownloadMap () and MakeMapRowPointers().
 *		08/10/93 (dcc) - Untested: Only called InitCLD() once.
 *		08/11/93 (dcc) - Fix SNASM Genesis downloading (broke 04/14/93).
 *		03/22/94 (dcc) - Changed address of palette in SNASM & PsyQ
 *							  SNES & Genesis. Also added a word which indicates
 *							  the number of bytes downloaded. Note that dvpPARALLEL
 *							  remains unchanged (no extra word, same address).
 *		01/22/95 (dcc) - Make module lint with no errors.
 *
*/

#define BST 0

#include <stdlib.h>

#include <echidna/platform.h>
#include "switches.h"
#include "switch2.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include <echidna/etypes.h>
#include <echidna/trees.h>

#include "tuglbl.h"
#if dvpSNASM
#include "snasmio.h"
#elif dvpPsyQ
#include "psyqio.h"
#elif dvpCLD
#include "cldioglu.h"
#elif dvpPARALLEL
#include "paraglue.h"
#elif dvpNONE
#endif
#include "tilebits.h"
#include "download.h"
#include "colorreq.h"
#include "grabpal.h"

extern short DownloadOneScreenSTATE;

/**************************** C O N S T A N T S ***************************/

#define uwPaletteMax		(128+1)	/* Total number of colors in palette */

#define wXPixWidth		(8)		/* Width of char in pixels */
#define wYPixHeight		(8)		/* Height of char in pixels */
#define wXSNESScrWidth	(32)		/* Width of screen in chars */
#define wXGenesisScrWidth	(40)	/* Width of screen in chars */
#define wYScrHeight		(32)		/* Height of screen in chars */

#define wSNESColorMask	(7)		/* significant color set bits to store */
#define wSNESColorBit	(10)		/* Bit position to store color set value */

#define uwSNESFlipYBit	(0x8000)	/* Bit representing Y flip */
#define uwSNESFlipXBit	(0x4000)	/* Bit representing X flip */

#define wGenesisColorMask	(3)	/* significant color set bits to store */
#define wGenesisColorBit	(5)	/* Bit position to store color set value */

#define uwGenesisFlipYBit	(0x10)/* Bit representing Y flip */
#define uwGenesisFlipXBit	(0x08)/* Bit representing X flip */

#define fCharPristine	(0x00)	/* fCharFlags: character not defined */
#define fCharDownloaded	(0x80)	/* fCharFlags: character already downloaded */
#define fCharDefined		(0x01)	/* fCharFlags: character just defined */

#define ulSNESCharAddress0		(0x018000UL)	/* SNES address to store 1st 32K chars */
#define uwSNESCharAddress0Len	(0x8000U)		/* SNES length of 1st 32K chars */
#define ulSNESCharAddress1		(0x028000UL)	/* SNES address to store 2nd 32K chars */
#define uwSNESCharAddress1Len	(0x8000U)		/* SNES length of 2nd 32K chars */
#define ulSNESPaletteAddress	(0x000200UL)	/* SNES address to store palette */
#define uwSNESPaletteCnt		(128+1)			/* SNES download 258 bytes */
#define ulSNESMapInfoAddress	(0x008200UL)	/* SNES address to store map info */
#define ulSNESMapBankAddress	(0x00A000UL)	/* SNES address to store map start banks */

#define ulSNESMapBankAddr256	(0x038000UL)	/* SNES address to store map */

#define ulSNESMapBankAddr16	(0x028000UL)	/* SNES address to store map */

#define uwSNESMapBankSize		(0x8000U)		/* SNES map bank size */

#define ulGenesisHeaderAddress	(0x000200UL)	/* Genesis address of addresses table */

#if (dvpSNASM || dvpPsyQ)
#define ulGenesisCharAddress0		(0x001300UL)	/* Genesis address to store 1st 32K chars */
#define uwGenesisCharAddress0Len	(0x8000U)		/* Genesis length of 1st 32K chars */
#define ulGenesisCharAddress1		(0x009300UL)	/* Genesis address to store 2nd 32K chars */
#define uwGenesisCharAddress1Len	(0x4000U)		/* Genesis length of 2nd 32K chars */
#define ulGenesisPaletteAddress	(0xFFFF7EUL)	/* Genesis address to store palette */
#define uwGenesisPaletteCnt		(64+1)			/* Genesis download 130 bytes */
#define ulGenesisMapInfoAddress	(0x001200UL)	/* Genesis address to store map info */
#define ulGenesisMapBankAddress	(0x010000UL)	/* Genesis address to store map line start info */
#define ulGenesisMapBankAddr		(0x018000UL)	/* Genesis address to store map */

#define ulGenesisMapBankSize		(0x008000UL)	/* Genesis map bank size */
#endif

//#define ulParGenesisCharAddress0	 (0xFF000000UL)/* Genesis address to store 1st 32K chars */
//#define uwParGenesisCharAddress0Len	(0x8000U)	/* Genesis length of 1st 32K chars */
//#define ulParGenesisCharAddress1	 (0xFF008000UL)/* Genesis address to store 2nd 32K chars */
//#define uwParGenesisCharAddress1Len	(0x4000U)	/* Genesis length of 2nd 32K chars */
//#define ulParGenesisPaletteAddress	(0x002000UL)/* Genesis address to store palette */
//#define ulParGenesisPaletteAddress	(0xFF0000UL)/* Genesis address to store palette */
//#define ulParGenesisMapInfoAddress	(0xFF0080UL)/* Genesis address to store map info */
//#define ulParGenesisMapBankAddress	(0xFF0400UL)/* Genesis address to store map line start info */
//#define ulParGenesisMapBankAddr		(0xFF1000UL)/* Genesis address to store map */

//#define ulParGenesisMapBankSize		(0x008000UL)/* Genesis map bank size */

#define fRightDone		(1)
#define fDownDone			(2)
#define fLeftDone			(4)
#define fTopDone			(8)

#define xtrCharBankSize	(32768U)				// size of bank to store all chars

/* BEGIN Tile Banks constants */

#define winTB				(2)					// XTRA mem window #
#define xtrBankSize		(16384U)				// size of tile bank to allocate

/* END Tile Banks constants */

/* BEGIN Char Banks constants */

#define sizeofMapChar	(sizeof(UWORD))
#define sizeofMapAddrL	(2)
#define sizeofMapAddrH	(1)

/* END Char Banks constants */

/* Don't let map be smaller than <wSNESMinMapWidth, wSNESMinMapHeight> */

#define wSNESMinMapWidth	(66)
#define wSNESMinMapHeight	(34)

/******************************** T Y P E S *******************************/


typedef struct
{
	UWORD uwVersion;			/* version number (ignored) */
	UWORD uwADRBlockSize;	/* info block size (ignored) */
	ULONG ulPalette;			/* address to store palette */
	UWORD uwPaletteCnt;		/* number of palette entries (* 2 == bytes) */
	ULONG ulMapInfo;			/* address to store map info */
	UWORD uwMapInfoLength;	/* length of map info block in bytes */
	ULONG ulMapStart;			/* address to store map row start pointers */
	UWORD uwMapStartMax;		/* number of map row pointers (*3 == bytes) (ignored) */
	UWORD uwCharSetAreas;	/* number of char set storage areas (ignored) */
	ULONG ulCharAddress0;	/* address to store 1st 32K chars */
	UWORD uwCharAddress0Len;/* length of 1st 32K chars */
	ULONG ulCharAddress1;	/* address to store 2nd 32K chars */
	UWORD uwCharAddress1Len;/* length of 2nd 32K chars */
	ULONG ulMapBankAddr;		/* address to store character map */
	ULONG ulMapBankSize;		/* size of each character map bank area */
	ULONG ulMapBankSkip;		/* add to ulMapBankAddr to get next char bank area */
} ADR;


/****************************** G L O B A L S *****************************/

WORD wSCSIAddress = 0;						/* Default target SCSI device address */

static BOOL fInited = FALSE;

#if !dvpNONE

static UWORD uwBitsPerPixel = 8;	/* Number of bits per pixel */
static UWORD uwBytesPerChar = 64;/* Number of bytes required to store a char */
static unsigned uwCharMax		 = 1024;/* Total number of characters in char set */
static UWORD uwCharsInBank	 = xtrCharBankSize / 64; // uwBytesPerChar;

#endif/*!dvpNONE*/

UWORD *uwPalette;		/* Array of palette values */

#if !dvpNONE

/* BEGIN Screen "constants" */

static WORD wXScrWidth = wXSNESScrWidth;

#if BST
/* trChar stores all characters that have been defined or downloaded */

static TreeType trChar;			/* Pointer to char search tree */
#endif

static UBYTE *fCharFlags;		/* Array of char flags */
//static UWORD *uwCharUsage;	/* Array of char referenced count */

//static UWORD *uwMap;				/* Two-dimensional array of map values */
//static UBYTE *fMapValid;		/* Two-dimensional array of map valid flags */

static UBYTE *pubCharImageTemp;/* Memory to store a single char image */

static XTRAPntr pxtCharImage[2];	/* Two arrays of all character images */

static ByteMap bmChar;			/* Byte map with current character */

static UWORD uwPrevEmptySlot;	/* Assert: always <= index of lowest empty slot */

static WORD TileWidth;			/* Width of each tile in current map */
static WORD TileHeight;			/* Height of each tile in current map */
static WORD XCharsInTile;		/* # of chars across per tile */
static WORD YCharsInTile;		/* # of chars down per tile */

static WORD wXMapMinex;
static WORD wXMapMaxex;
static WORD wYMapMinex;
static WORD wYMapMaxex;

/* BEGIN Tile Banks variables */

static int			 TileBanksMax;		/* Number of tile banks we allocated */
static UWORD		 TilesPerBank;		/* Number of tile infos we can store in each bank. */
static LONG			*lTileBase;			/* array of LONG index of 1st tile of tileset */
static XTRAPntr	*pxmtTileBanks;	/* array of pointers to tile banks */
static WORD			 CharsInTile;		/* number of characters per tile */
static WORD			 sizeofTile;		/* Number of bytes to store all char slots (UWORD) in each tile */

/* END Tile Banks variables */


/* BEGIN Char Map Banks variables */

/* Size of output map. wXSNESMapMaxex is > wXMapMaxex (size of room
	in characters), as there are minimum room size to be met, plus
	1 character border all around the map. */

static WORD			 wXSNESMapMaxex;
static WORD			 wYSNESMapMaxex;

static WORD			 wXSNESCharInit;	/* Initial screen left coordinate on map */
static WORD			 wYSNESCharInit;	/* Initial screen upper coordinate on map */

static ADR adrTarget;	/* addresses to store various downloaded components */

static int			 CharBanksMax;		/* Number of map banks we allocated */
static unsigned	 CharsPerBank;		/* Number of map chars we can store in each bank. */
static LONG			 lSNESMapRowBytes;/* Map width in chars * sizeofMapChar */
static XTRAPntr	*pxmtCharBanks;	/* array of pointers to map banks */

static int			 AddrLBanksMax;	/* Number of start of map rows L banks we allocated */
static UWORD		 AddrLPerBank;		/* Number of start of map rows L we can store in each bank. */
static LONG			 lAddrLSize;		/* Height of map * sizeofMapAddrL */
static XTRAPntr	*pxmtAddrLBanks;	/* array of pointers to start of map rows L banks */

static int			 AddrHBanksMax;	/* Number of start of map rows H banks we allocated */
static UWORD		 AddrHPerBank;		/* Number of start of map rows H we can store in each bank. */
static LONG			 lAddrHSize;		/* Height of map * sizeofMapAddrH */
static XTRAPntr	*pxmtAddrHBanks;	/* array of pointers to start of map rows H banks */

/* END Char Map Banks variables */

static WORD fTarget;						/* downloading to SNASM68000_ID or SNASM65816_ID */
static BOOL (*GrabPalette)(CS_Color *);
static void (*CharImageFromByteMap)(UBYTE *, ByteMap *);

#endif/*!dvpNONE*/

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


#if dvpNONE

BOOL InitDownloader(void)
{
	/* No download hardware available. */

	fEnableDownload = FALSE;
	fSetDownloadHardwareAvailable(FALSE);
	fInited = TRUE;
	return TRUE;
}

void GrabNDownloadPalette(ColorInfo *ci)
{
	ci = ci;
}

BOOL DoDownloadMap(TileSpaceType *ptsp, RoomType *prt, WORD wxrt, WORD wyrt, WORD wColors)
{
	ptsp    = ptsp;
	prt     = prt;
	wxrt    = wxrt;
	wyrt    = wyrt;
	wColors = wColors;

	return TRUE;
}

#else


/*********************************************************************
 *
 * FreeXTRAArray
 *
 * SYNOPSIS
 *		static void FreeXTRAArray(XTRAPntr **hxmt, WORD max)
 *
 * PURPOSE
 *		Free memory pointed to by <hxmt>.
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
static void FreeXTRAArray(XTRAPntr **hxmt, WORD max)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FreeXTRAArray";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (*hxmt)
	{
		WORD i;

		for (i = 0; i < max; i++)
		{
			if ((*hxmt)[i])
				FreeXTRA((*hxmt)[i]);
		}
		MEM_free(*hxmt);
		*hxmt = NULL;
	}

} /* FreeXTRAArray */


/*********************************************************************
 *
 * AllocXTRAArray
 *
 * SYNOPSIS
 *		static BOOL AllocXTRAArray(XTRAPntr **hxmt, WORD max, WORD c, UWORD uwSize)
 *
 * PURPOSE
 *		Allocate <max> banks of XTRA memory of <uwSize> each,
 *		make array to store them all and save in array pointed
 *		to by <hxmt>. Set all allocated memory bytes to <c>.
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
static BOOL AllocXTRAArray(XTRAPntr **hxmt, WORD max, WORD c, UWORD uwSize)
{
	WORD i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AllocXTRAArray";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((*hxmt = MEM_calloc(max, sizeof(XTRAPntr))) == NULL)
/**/	goto ABORT;

	/* Allocate XTRA memory banks that we need. */

	for (i = 0; i < max; i++)
	{
		UBYTE *pub;

		/* Try allocating in regular memory first. */

		if (((*hxmt)[i] = AllocXTRAofXMT(uwSize, xmtSTD)) == NULL)
		{
			/* No more regular memory, so use EMS or XMS memory. */

			if (((*hxmt)[i] = AllocXTRA(uwSize)) == NULL)
/**/			goto ABORT;			/* Still can't allocate, give up. */
		}
		pub = ActivateXTRAatWin((*hxmt)[i], winTB);
		memset(pub, c, uwSize);
		UpdateXTRA((*hxmt)[i]);
	}
	return TRUE;

ABORT:
	FreeXTRAArray(hxmt, max);
	return FALSE;

} /* AllocXTRAArray */


/* InitDownloader - initialize downloader routine.
	Returns TRUE if successful. */

BOOL InitDownloader(void)
{
	if (!fInited)
	{
#if dvpSNASM
		if (!InitSNASM(wSCSIAddress))
#elif dvpPsyQ
		if (!InitPsyQ(wSCSIAddress))
#elif dvpCLD
		if (!InitCLD())
#elif dvpPARALLEL
		if (!fInitParallel())
#endif
		{
			/* No download hardware available. */

			fSetDownloadHardwareAvailable(FALSE);
			fInited = TRUE;
			return TRUE;
		}

#if dvpSNASM
		SetDownloadTarget(SNASMID);
#elif dvpPsyQ
		SetDownloadTarget(PsyQID);
#elif dvpCLD
		SetDownloadTarget(fSNES);
#elif dvpPARALLEL
		SetDownloadTarget(wGetParallelID());
#endif
		if ((uwPalette = MEM_calloc(uwPaletteMax, sizeof(UWORD))) == NULL)
		{
			return FALSE;
		}

#if BST
		InitBST(&trChar);						/* Create new empty tree */
#endif

		/* Set all fCharFlags[] to fCharPristine */

		if ((fCharFlags = MEM_calloc(uwCharMax, sizeof(UBYTE))) == NULL)
		{
			return FALSE;
		}
//		if ((uwCharUsage = MEM_calloc(uwCharMax, sizeof(UWORD))) == NULL)
//		{
//			return FALSE;
//		}
//		if ((uwMap = MEM_calloc(wYScrHeight * wXScrWidth, sizeof(UWORD))) == NULL)
//		{
//			return FALSE;
//		}
//		if ((fMapValid = MEM_calloc(wYScrHeight * wXScrWidth, sizeof(UBYTE))) == NULL)
//		{
//			return FALSE;
//		}
		if ((bmChar.data = MEM_calloc(wYPixHeight, wXPixWidth)) == NULL)
		{
			return FALSE;
		}
		if ((pubCharImageTemp = MEM_calloc(1, uwBytesPerChar)) == NULL)
		{
			return FALSE;
		}
		if ((pxtCharImage[0] = AllocXTRA(xtrCharBankSize)) == NULL)
		{
			return FALSE;
		}
		if ((pxtCharImage[1] = AllocXTRA(xtrCharBankSize)) == NULL)
		{
			return FALSE;
		}
//		if ((pxtCharImage = AllocXTRA(uwCharMax * uwBytesPerChar)) == NULL)
//		{
//			return FALSE;
//		}

		bmChar.width = wXPixWidth;
		bmChar.height = wYPixHeight;

		fSetDownloadHardwareAvailable(TRUE);
		UpdateHrdwrColorRoutinePtr = NULL;
		fInited = TRUE;

#if dvpSNASM
		RestartSNASM(FALSE);
#elif dvpPsyQ
		RestartPsyQ(FALSE);
#elif dvpCLD
//		Lawrence sez nothing goes here
#elif dvpPARALLEL
//		dcc sez nothing goes here
#endif
	}
	return TRUE;
}


/*********************************************************************
 *
 * fSameChar
 *
 * SYNOPSIS
 *		static int fSameChar(void *pub1, void *pub2)
 *
 * PURPOSE
 *		This function is used to order characters in the binary search tree.
 *		Compare function: return  1 if *pub1 >  *pub2
 *						 else return  0 if *pub1 == *pub2
 *						 else return -1 if *pub1 <  *pub2
 *
 * INPUT
 *		pub1:			pointer to first tile character
 *		pub2:			pointer to second tile character
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		0 if equal, 1 if >, or -1 if <.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static int fSameChar(void *pub1, void *pub2)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fSameChar";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	return memcmp(pub1, pub2, uwBytesPerChar);
} /* fSameChar */


/*********************************************************************
 *
 * FindAlreadyDefinedChar
 *
 * SYNOPSIS
 *		static UWORD FindAlreadyDefinedChar(UBYTE *pub)
 *
 * PURPOSE
 *		See if the char image pointed to by <pub> has already been
 *		downloaded or defined by searching the binary search tree for it.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Index of slot of matching character, or 0xFFFF if not found.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static UWORD FindAlreadyDefinedChar(UBYTE *pub)
{
#if BST
	UBYTE *pubChar;
	UBYTE	*pubC;
#else
	UWORD uw;
	UBYTE *pubC;
#endif

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindAlreadyDefinedChar";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if BST
	pubC = ActivateXTRA(pxtCharImage);
	if ((pubChar = BSTfind(&trChar, pub, fSameChar)) != NULL)
	{
		ReleaseXTRA(pxtCharImage);
		return (pubChar - pubC) / uwBytesPerChar;
	}
	else
	{
		ReleaseXTRA(pxtCharImage);
		return 0xffff;
	}
#else
	pubC = ActivateXTRA(pxtCharImage[0]);
	for (uw = 0; uw < uwCharsInBank; uw++)
	{
		if (fCharFlags[uw])
		{
			if (fSameChar(pub, pubC + uw * uwBytesPerChar) == 0)
			{
				ReleaseXTRA(pxtCharImage[0]);
				return uw;
			}
		}
	}
	ReleaseXTRA(pxtCharImage[0]);
	pubC = ActivateXTRA(pxtCharImage[1]);
	for (uw = uwCharsInBank; uw < uwCharMax; uw++)
	{
		if (fCharFlags[uw])
		{
			if (fSameChar(pub, pubC + (uw-uwCharsInBank) * uwBytesPerChar) == 0)
			{
				ReleaseXTRA(pxtCharImage[1]);
				return uw;
			}
		}
	}
	ReleaseXTRA(pxtCharImage[1]);
	return 0xffff;
#endif
} /* FindAlreadyDefinedChar */


/*********************************************************************
 *
 * SaveCharImage
 *
 * SYNOPSIS
 *		static void SaveCharImage(UWORD uwSlot, UBYTE *pubSrc)
 *
 * PURPOSE
 *		Save character <pubSrc> in character set array at <uwSlot>.
 *		Add the character to the binary search tree.
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
static void SaveCharImage(UWORD uwSlot, UBYTE *pubSrc)
{

#if BST
	UBYTE *pub;
#endif

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveCharImage";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if BST
	pub = ActivateXTRA(pxtCharImage);
	WriteXTRA(pubSrc, uwBytesPerChar, pxtCharImage, uwSlot * uwBytesPerChar);
	memcpy(pub + uwSlot * uwBytesPerChar, pubSrc, uwBytesPerChar);
	BSTinsert(&trChar, pub + uwSlot * uwBytesPerChar, fSameChar);
	UpdateXTRA(...);
#else
	if (uwSlot < uwCharsInBank)
		WriteXTRA(pubSrc, uwBytesPerChar, pxtCharImage[0], uwSlot * uwBytesPerChar);
	else
		WriteXTRA(pubSrc, uwBytesPerChar, pxtCharImage[1], (uwSlot-uwCharsInBank) * uwBytesPerChar);
#endif
} /* SaveCharImage */


/*********************************************************************
 *
 * InitFindNextEmptyCharSlot
 *
 * SYNOPSIS
 *		static void InitFindNextEmptyCharSlot(void)
 *
 * PURPOSE
 *		Initialize variable so we can find next empty character slot.
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
static void InitFindNextEmptyCharSlot(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitFindNextEmptyCharSlot";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	uwPrevEmptySlot = 0;
} /* InitFindNextEmptyCharSlot */


/*********************************************************************
 *
 * FindNextEmptyCharSlot
 *
 * SYNOPSIS
 *		static UWORD FindNextEmptyCharSlot(void)
 *
 * PURPOSE
 *		Find the first unused char slot.
 *
 * INPUT
 *
 * EFFECTS
 *		uwPrevEmptySlot is set to empty slot found.
 *
 * RETURN VALUE
 *		Index of empty slot, or 0xFFFF if not found.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static UWORD FindNextEmptyCharSlot(void)
{
	UWORD uw;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindNextEmptyCharSlot";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (uw = uwPrevEmptySlot; uw < uwCharMax; uw++)
	{
		if (!(fCharFlags[uw] & (fCharDownloaded | fCharDefined)))
		{
			return uwPrevEmptySlot = uw;
		}
	}
	return uwPrevEmptySlot = 0xffff;
} /* FindNextEmptyCharSlot */


/*********************************************************************
 *
 * MakeBlankCharImage
 *
 * SYNOPSIS
 *		static void MakeBlankCharImage(UBYTE *pub)
 *
 * PURPOSE
 *		Make <pub> into a blank character.
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
static void MakeBlankCharImage(UBYTE *pub)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MakeBlankCharImage";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	memset(pub, 0, uwBytesPerChar);

} /* MakeBlankCharImage */


/*********************************************************************
 *
 * GenesisCharImageFromByteMap
 *
 * SYNOPSIS
 *		static void GenesisCharImageFromByteMap(UBYTE *pub, ByteMap *bm)
 *
 * PURPOSE
 *		Transform <pbm> into a Genesis character image, save at <pub>.
 *
 *		Genesis chars are store 2 pixels per byte. Bits 7..4 define the
 *		left pixel, and bits 3..0 define the right pixel.
 *
 * INPUT
 *		pbm:				address of ByteMap to transform
 *		pub:				address to save new character
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		The colorset of this char, which is determined by the
 *		first non-transparent pixel.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static void GenesisCharImageFromByteMap(UBYTE *pub, ByteMap *pbm)
{
	WORD wXp, wYp;
	UBYTE *pubBM;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GenesisCharImageFromByteMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pubBM = pbm->data;

	for (wYp = 0; wYp < wYPixHeight; wYp++)
	{
		for (wXp = 0; wXp < wXPixWidth; wXp += 2)
		{
			UBYTE ubL = (*pubBM++) & 0x0F;
			UBYTE ubR = (*pubBM++) & 0x0F;

			(*pub++) = (ubL << 4) | ubR;
		}
	}

} /* GenesisCharImageFromByteMap */


/*********************************************************************
 *
 * SNESCharImageFromByteMap
 *
 * SYNOPSIS
 *		static void SNESCharImageFromByteMap(UBYTE *pub, ByteMap *bm)
 *
 * PURPOSE
 *		Transform <pbm> into a SNES character image, save at <pub>.
 *
 *		SNES characters are stored planar.
 *		The first byte represents bit 0 of each of the top 8 pixels,
 *		the second byte represents bit 1 of each of the top 8 pixels,
 *		the 17th byte represents bit 2 of each of the top 8 pixels,
 *		the 18th byte represents bit 3 of each of the top 8 pixels.
 *
 * INPUT
 *		pbm:				address of ByteMap to transform
 *		pub:				address to save new character
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		The colorset of this char, which is determined by the
 *		first non-transparent pixel.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static void SNESCharImageFromByteMap(UBYTE *pub, ByteMap *pbm)
{
	WORD wYp;
	WORD wP;
	UBYTE ub[8];
	UBYTE ubPlane[8];
	UBYTE bit;
	UBYTE *pubBM;
	UBYTE *pub16 = pub+16;
	UBYTE *pub32 = pub+32;
	UBYTE *pub48 = pub+48;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SNESCharImageFromByteMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pubBM = pbm->data;

	for (wYp = 0; wYp < wYPixHeight; wYp++)
	{
		ub[0] = *pubBM++;
		ub[1] = *pubBM++;
		ub[2] = *pubBM++;
		ub[3] = *pubBM++;
		ub[4] = *pubBM++;
		ub[5] = *pubBM++;
		ub[6] = *pubBM++;
		ub[7] = *pubBM++;
		ubPlane[0] = 0;
		ubPlane[1] = 0;
		ubPlane[2] = 0;
		ubPlane[3] = 0;
		bit = 0x01;
		for (wP = 0; wP < uwBitsPerPixel; wP++)
		{
			ubPlane[wP] =	((ub[0] & bit) ? 0x80 : 0x00) |
								((ub[1] & bit) ? 0x40 : 0x00) |
								((ub[2] & bit) ? 0x20 : 0x00) |
								((ub[3] & bit) ? 0x10 : 0x00) |
								((ub[4] & bit) ? 0x08 : 0x00) |
								((ub[5] & bit) ? 0x04 : 0x00) |
								((ub[6] & bit) ? 0x02 : 0x00) |
								((ub[7] & bit) ? 0x01 : 0x00) ;
			bit <<= 1;
		}
		*pub++ = ubPlane[0];
		*pub++ = ubPlane[1];
		*pub16++ = ubPlane[2];
		*pub16++ = ubPlane[3];
		if (uwBytesPerChar > 32)
		{
			*pub32++ = ubPlane[4];
			*pub32++ = ubPlane[5];
			*pub48++ = ubPlane[6];
			*pub48++ = ubPlane[7];
		}
	}

} /* SNESCharImageFromByteMap */


/*********************************************************************
 *
 * ColorFromByteMap
 *
 * SYNOPSIS
 *		static WORD ColorFromByteMap(ByteMap *pbm)
 *
 * PURPOSE
 *		Find 1st non-transparent pixel, and return color.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns color of <pbm>,
 *		or -1 if <pbm> is completely transparent.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static WORD ColorFromByteMap(ByteMap *pbm)
{

	UBYTE *pubBM;
	WORD w, wMax;
#if 1
	short bcolor;
#endif

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ColorFromByteMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pubBM = pbm->data;

	/* Find 1st non-transparent pixel. */

#if 1
	bcolor = -1;

	wMax = pbm->width * pbm->height - 1;

	for (w = 0; w < wMax; w += 2)
	{
		UBYTE p0, p1;
		UBYTE c0, c1;

		p0 = (*pubBM   & 0x0F);
		c0 = (*pubBM++ & 0xF0) >> 4;

		p1 = (*pubBM   & 0x0F);
		c1 = (*pubBM++ & 0xF0) >> 4;

		if (p0 < 2)
		{
			if (p0 && c0 && bcolor == -1)
			{
				bcolor = c0;
			}
			c0 = 0;
		}
		if (p1 < 2)
		{
			if (p1 && c1 && bcolor == -1)
			{
				bcolor = c1;
			}
			c1 = 0;
		}
		if (c0 || c1)
		{
			if (c0)
			{
//				if (c0 < 7)
//				{
					return c0;
//				}
			}
			else
			{
//				if (c1 < 7)
//				{
					return c1;
//				}
			}
		}
	}

	if (bcolor == -1)
		return 0;

	return bcolor;
#else
	wMax = pbm->width * pbm->height;

	for (w = 0; w < wMax; w++)
	{
		if (*pubBM != pbm->transparentColor)
			return (*pubBM >> 4) & 7;		/* Return colorset of that pixel. */
		pubBM++;
	}

	return -1;									/* It's all transparent to me */
#endif

} /* ColorFromByteMap */

/*
while processing:

!Downloaded & !Defined: empty
!Downloaded &  Defined: not empty
 Downloaded & !Defined: not empty
 Downloaded &  Defined: not empty

at end of processing:
!Downloaded & !Defined: skip character
!Downloaded &  Defined: download character. Set Downloaded TRUE
 Downloaded & !Defined: skip character
 Downloaded &  Defined: skip character

char: defined/not defined (effectively downloaded/not downloaded)
char: in use counter (# times used on current screen)
char: newly defined (download it!)

linked list of up to 1024 that says what is not defined.

* mark all chars as not defined (was: add all char slots to not defined linked list)
* reset all in use counters
* reset all newly defined flags

* for every display char:
*if (found in BST [it is already defined in character set])
*{
*	set up value in map/mark as valid
*	mark char in charset as being in use/inc in use counter
*}
else
{
*	if (empty char slots not defined)
*	{
*		add new char to BST (remove from not defined linked list)
*		set up value in map/mark as valid
*		mark char in charset as being in use/inc in use counter
*	}
	else
	{
		define a new char in new char array (index + 1024)
		set up value in map/mark as not valid (save const to add to index)
		mark char in charset as being in use/inc in use counter
	}
}

for every char in new char array:
{
	if (there is a char in charset not used)
	{
		delete unused char from BST (add to not defined linked list)
		add new char to BST (remove from not defined linked list)
		replace new char array char data with new index ( < 1024 index)
	}
}

for every char still in new char array:
{
	find the least frequently used char in charset:
	if (it occurs more frequently than some char in charset)
	{
		delete infrequently used char from BST (add to not defined linked list)
		add new char to BST (remove from not defined linked list)
		replace new char array char data with new index ( < 1024 index)
	}
}

for every value in map:
{
	if (not valid)
		lookup new valid index
}

- download new map
- download all characters that are newly defined

*/


/*********************************************************************
 *
 * DownloadCharSet
 *
 * SYNOPSIS
 *		static void DownloadCharSet(void)
 *
 * PURPOSE
 *		Download the character set to Super Nintendo in least time possible.
 *		Only down characters that have been defined and haven't already
 *		been downloaded (fCharDownloaded == FALSE && fCharDefined == TRUE).
 *
 *		It's cheaper to download several characters than one character.
 *		The break-even point is three undefined characters; if we encounter
 *		four empty characters in a row, we'll download what we already
 *		have, and start a new group, otherwise, include the three undefined
 *		characters as part of the current group and keep searching until
 *		you encounter a break of four undefined characters.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static void DownloadCharSet(void)
{
	UWORD uw = 0;
	UWORD uwStart = 0;			/* Start character to download */
	UWORD uwCount = 0;			/* Number of characters to download */
	UWORD uwUndef = 0;			/* Count number of contiguous 'undefined' chars */

#define uwUndefBreakEven (3)

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DownloadCharSet";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	while (uw < uwCharsInBank)
	{
		if (!(fCharFlags[uw] & fCharDownloaded) && (fCharFlags[uw] & fCharDefined))
		{
			fCharFlags[uw] |= fCharDownloaded;
			uwCount++;
			uwCount += uwUndef;
			uwUndef = 0;
			uw++;
		}
		else
		{
			uwUndef++;
			uw++;
			if (uwUndef > uwUndefBreakEven)
			{
				if (uwCount)
				{
					UBYTE *pub;

					pub = ActivateXTRA(pxtCharImage[0]);
#if dvpSNASM
					UploadToSNASM(	(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress0,
										pub + uwStart * uwBytesPerChar,
										(ULONG) uwCount * uwBytesPerChar);
#elif dvpPsyQ
					UploadToPsyQ(	(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress0,
										pub + uwStart * uwBytesPerChar,
										(ULONG) uwCount * uwBytesPerChar);
#elif dvpCLD
					DownloadToCLD(pub + uwStart * uwBytesPerChar,
										(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress0,
										(ULONG) uwCount * uwBytesPerChar);
#elif dvpPARALLEL
					fParallelBlockWrite( (ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress0,
										pub + uwStart * uwBytesPerChar,
										(ULONG) uwCount * uwBytesPerChar);
#endif
					ReleaseXTRA(pxtCharImage[0]);
				}
				while (uw < uwCharsInBank &&
						!(!(fCharFlags[uw] & fCharDownloaded) &&
							(fCharFlags[uw] & fCharDefined)))
				{
					uw++;
				}
				uwStart = uw;
				uwCount = 0;
				uwUndef = 0;
			}
		}
	}
	if (uwCount)
	{
		UBYTE *pub;

		pub = ActivateXTRA(pxtCharImage[0]);
#if dvpSNASM
		UploadToSNASM(	(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress0,
							pub + uwStart * uwBytesPerChar,
							(ULONG) uwCount * uwBytesPerChar);
#elif dvpPsyQ
		UploadToPsyQ(	(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress0,
							pub + uwStart * uwBytesPerChar,
							(ULONG) uwCount * uwBytesPerChar);
#elif dvpCLD
		DownloadToCLD(pub + uwStart * uwBytesPerChar,
							(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress0,
							(ULONG) uwCount * uwBytesPerChar);
#elif dvpPARALLEL
		fParallelBlockWrite( (ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress0,
							pub + uwStart * uwBytesPerChar,
							(ULONG) uwCount * uwBytesPerChar);
#endif
		ReleaseXTRA(pxtCharImage[0]);
	}

	uw = uwCharsInBank;
	uwStart = 0;			/* Start character to download */
	uwCount = 0;			/* Number of characters to download */
	uwUndef = 0;			/* Count number of contiguous 'undefined' chars */

	while (uw < uwCharMax)
	{
		if (!(fCharFlags[uw] & fCharDownloaded) && (fCharFlags[uw] & fCharDefined))
		{
			fCharFlags[uw] |= fCharDownloaded;
			uwCount++;
			uwCount += uwUndef;
			uwUndef = 0;
			uw++;
		}
		else
		{
			uwUndef++;
			uw++;
			if (uwUndef > uwUndefBreakEven)
			{
				if (uwCount)
				{
					UBYTE *pub;

					pub = ActivateXTRA(pxtCharImage[1]);
#if dvpSNASM
					UploadToSNASM(	(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress1,
										pub + uwStart * uwBytesPerChar,
										(ULONG) uwCount * uwBytesPerChar);
#elif dvpPsyQ
					UploadToPsyQ(	(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress1,
										pub + uwStart * uwBytesPerChar,
										(ULONG) uwCount * uwBytesPerChar);
#elif dvpCLD
					DownloadToCLD(pub + uwStart * uwBytesPerChar,
										(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress1,
										(ULONG) uwCount * uwBytesPerChar);
#elif dvpPARALLEL
					fParallelBlockWrite( (ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress1,
										pub + uwStart * uwBytesPerChar,
										(ULONG) uwCount * uwBytesPerChar);
#endif
					ReleaseXTRA(pxtCharImage[1]);
				}
				while (uw < uwCharMax &&
						!(!(fCharFlags[uw] & fCharDownloaded) &&
							(fCharFlags[uw] & fCharDefined)))
				{
					uw++;
				}
				uwStart = uw - uwCharsInBank;
				uwCount = 0;
				uwUndef = 0;
			}
		}
	}
	if (uwCount)
	{
		UBYTE *pub;

		pub = ActivateXTRA(pxtCharImage[1]);
#if dvpSNASM
		UploadToSNASM(	(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress1,
							pub + uwStart * uwBytesPerChar,
							(ULONG) uwCount * uwBytesPerChar);
#elif dvpPsyQ
		UploadToPsyQ(	(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress1,
							pub + uwStart * uwBytesPerChar,
							(ULONG) uwCount * uwBytesPerChar);
#elif dvpCLD
		DownloadToCLD(pub + uwStart * uwBytesPerChar,
							(ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress1,
							(ULONG) uwCount * uwBytesPerChar);
#elif dvpPARALLEL
		fParallelBlockWrite( (ULONG) uwStart * uwBytesPerChar + adrTarget.ulCharAddress1,
							pub + uwStart * uwBytesPerChar,
							(ULONG) uwCount * uwBytesPerChar);
#endif
		ReleaseXTRA(pxtCharImage[1]);
	}
} /* DownloadCharSet */


/*********************************************************************
 *
 * DownloadMap
 *
 * SYNOPSIS
 *		static void DownloadMap(void)
 *
 * PURPOSE
 *		Download the map to the Super Nintendo.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static void DownloadMap(void)
{

	WORD i;
	UWORD uwSNESBase;
	UWORD puw[7];
	int RowsPerBank = (int) (adrTarget.ulMapBankSize / lSNESMapRowBytes);
	ULONG ul, ulAddress;
	LONG lSNESMapSize;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DownloadMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	lSNESMapSize = lSNESMapRowBytes * wYSNESMapMaxex;

	puw[0] = wXSNESMapMaxex;
	puw[1] = wYSNESMapMaxex;
	puw[2] = (UWORD) wXSNESCharInit*8;
	puw[3] = (UWORD) wYSNESCharInit*8;
	puw[4] = uwBitsPerPixel;
	puw[5] = (UWORD) (adrTarget.ulMapBankAddr & 0xFFFFU);
	puw[6] = (UWORD) (adrTarget.ulMapBankAddr >> 16);

#if dvpSNASM
	UploadToSNASM(adrTarget.ulMapInfo, puw, 14L);
#elif dvpPsyQ
	UploadToPsyQ(adrTarget.ulMapInfo, puw, 14L);
#elif dvpCLD
	DownloadToCLD(puw, adrTarget.ulMapInfo, 14L);
#elif dvpPARALLEL
	fParallelBlockWrite(adrTarget.ulMapInfo, puw, 14L);
#endif

	/* Download MapAddrH. */

	ulAddress = adrTarget.ulMapStart;
	ul = 0;
	for (i = 0; i < AddrHBanksMax; i++)
	{
		unsigned uw = min((unsigned) (lAddrHSize - ul), AddrHPerBank * sizeofMapAddrH);
		UBYTE *pub = ActivateXTRA(pxmtAddrHBanks[i]);

#if dvpSNASM
		UploadToSNASM(ulAddress + ul, pub, uw);
#elif dvpPsyQ
		UploadToPsyQ(ulAddress + ul, pub, uw);
#elif dvpCLD
		DownloadToCLD(pub, ulAddress + ul, uw);
#elif dvpPARALLEL
		fParallelBlockWrite(ulAddress + ul, pub, uw);
#endif
		ReleaseXTRA(pxmtAddrHBanks[i]);
		ul += uw;
	}

	/* Download MapAddrL. */

	ulAddress += ul;
	ul = 0;
	for (i = 0; i < AddrLBanksMax; i++)
	{
		unsigned uw = min((unsigned) (lAddrLSize - ul), AddrLPerBank * sizeofMapAddrL);
		UBYTE *pub = ActivateXTRA(pxmtAddrLBanks[i]);

#if dvpSNASM
		UploadToSNASM(ulAddress + ul, pub, uw);
#elif dvpPsyQ
		UploadToPsyQ(ulAddress + ul, pub, uw);
#elif dvpCLD
		DownloadToCLD(pub, ulAddress + ul, uw);
#elif dvpPARALLEL
		fParallelBlockWrite(ulAddress + ul, pub, uw);
#endif
		ReleaseXTRA(pxmtAddrLBanks[i]);
		ul += uw;
	}

	/* Download Map. */

	ulAddress	= adrTarget.ulMapBankAddr;
	uwSNESBase	= 0;
	ul = 0;
	for (i = 0; i < CharBanksMax; i++)
	{
		UWORD uwBase = 0;
		UBYTE *pub;

		pub = ActivateXTRA(pxmtCharBanks[i]);

		while (uwBase < CharsPerBank * sizeofMapChar)
		{
			unsigned uw;

			uw = min((unsigned) (lSNESMapSize - ul), CharsPerBank * sizeofMapChar - uwBase);
			uw = min(uw, (unsigned) (lSNESMapRowBytes * RowsPerBank - uwSNESBase));

#if dvpSNASM
			UploadToSNASM(ulAddress + uwSNESBase, pub + uwBase, uw);
#elif dvpPsyQ
			UploadToPsyQ(ulAddress + uwSNESBase, pub + uwBase, uw);
#elif dvpCLD
			DownloadToCLD(pub + uwBase, ulAddress + uwSNESBase, uw);
#elif dvpPARALLEL
			fParallelBlockWrite(ulAddress + uwSNESBase, pub + uwBase, uw);
#endif
			ul += uw;
			uwBase += uw;
			uwSNESBase += uw;
			if (uwSNESBase >= lSNESMapRowBytes * RowsPerBank)
			{
				ulAddress += adrTarget.ulMapBankSkip;
				uwSNESBase	= 0;
			}
			if (lSNESMapSize == ul)
				break;
		}
		ReleaseXTRA(pxmtCharBanks[i]);

		if (lSNESMapSize == ul)
			break;
	}
} /* DownloadMap */


/*********************************************************************
 *
 * DownloadPalette
 *
 * SYNOPSIS
 *		static void DownloadPalette(void)
 *
 * PURPOSE
 *		Download the palette to the Super Nintendo.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static void DownloadPalette(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DownloadPalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if dvpSNASM
	UploadToSNASM(adrTarget.ulPalette, uwPalette, (ULONG) adrTarget.uwPaletteCnt * sizeof(UWORD));
#elif dvpPsyQ
	UploadToPsyQ(adrTarget.ulPalette, uwPalette, (ULONG) adrTarget.uwPaletteCnt * sizeof(UWORD));
#elif dvpCLD
	DownloadToCLD(uwPalette, adrTarget.ulPalette, (ULONG) adrTarget.uwPaletteCnt * sizeof(UWORD));
#elif dvpPARALLEL
	fParallelBlockWrite(adrTarget.ulPalette, uwPalette+1, (ULONG) adrTarget.uwPaletteCnt * sizeof(UWORD));
#endif

} /* DownloadPalette */


/*********************************************************************
 *
 * InitGrabCharacters
 *
 * SYNOPSIS
 *		static void InitGrabCharacters(void)
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
static void InitGrabCharacters(void)
{

	UWORD uw;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitGrabCharacters";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Reset all chars as not fCharDefined.
		Save all fCharDownloaded states.
		Reset all usage counts to zero. */

	/* No, just as not fCharDefined and not fCharDownloaded */

	for (uw = 0; uw < uwCharMax; uw++)
	{
//was		fCharFlags[uw] &= fCharDownloaded;
		fCharFlags[uw] = 0;
//		uwCharUsage[uw] = 0;
	}
	InitFindNextEmptyCharSlot();

	/* Force character 0 to be a blank character. */

	MakeBlankCharImage(pubCharImageTemp);
	SaveCharImage(0, pubCharImageTemp);
	fCharFlags[0] |= fCharDefined;

} /* InitGrabCharacters */


/*********************************************************************
 *
 * CloseCharSlotFromTileSetIndex
 *
 * SYNOPSIS
 *		static void CloseCharSlotFromTileSetIndex(void)
 *
 * PURPOSE
 *		Clean up memory we allocated.
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
static void CloseCharSlotFromTileSetIndex(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CloseCharSlotFromTileSetIndex";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	MEM_free(lTileBase);
	FreeXTRAArray(&pxmtTileBanks, TileBanksMax);

} /* CloseCharSlotFromTileSetIndex */


/*********************************************************************
 *
 * OpenCharSlotFromTileSetIndex
 *
 * SYNOPSIS
 *		static BOOL OpenCharSlotFromTileSetIndex(TileSpaceType *ptsp)
 *
 * PURPOSE
 *		Allocate memory to save character slot indexes.
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
static BOOL OpenCharSlotFromTileSetIndex(TileSpaceType *ptsp)
{

	UWORD i;
	LONG lTileCrnt = 1;	/* Start w/ 1, as TileSet = 0, TileNum = 0 should be 0xFFFFU */

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenCharSlotFromTileSetIndex";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pxmtTileBanks	= NULL;
	CharsInTile		= YCharsInTile * XCharsInTile;
	sizeofTile		= CharsInTile * (sizeof(UWORD) + sizeof(UBYTE));
	TilesPerBank	= xtrBankSize / sizeofTile;

	if ((lTileBase = MEM_calloc(ptsp->HighestID+1, sizeof(LONG))) == NULL)
/**/	goto ABORT;

	/* Count total number of tiles in all tilesets */

	for (i = 0; i <= ptsp->HighestID; i++)
	{
		TileSetType *ptst = ptsp->TileSetArray[i];

		/* Only define for valid tilesets with same tile width & tile height */

		if (ptst != NULL && ptst->Width == TileWidth && ptst->Height == TileHeight)
		{
			lTileBase[i] = lTileCrnt;
			lTileCrnt	+= ptst->TileCount;
		}
	}

	TileBanksMax	= (int) (lTileCrnt / TilesPerBank + 1);

	if (!AllocXTRAArray(&pxmtTileBanks, TileBanksMax, 0xFF, xtrBankSize))
/**/	goto ABORT;

	return TRUE;

ABORT:
	CloseCharSlotFromTileSetIndex();
	return FALSE;

} /* OpenCharSlotFromTileSetIndex */


/*********************************************************************
 *
 * IndexFromTileOffsets
 *
 * SYNOPSIS
 *		static WORD IndexFromTileOffsets(PlotType pplt, WORD wXTOffset, WORD wYTOffset)
 *
 * PURPOSE
 *		IndexFromTileOffsets - convert the offsets <wXTOffset> and
 *		<wYTOffset> within the tile into a linear offset 0..n. The
 * 	characters within the tiles are numbered starting in the upper
 *		left corner, proceeding right first, then down.
 *
 *		The only complication comes when the tile is X or Y flipped,
 *		in which case the number changes. The idea is the same patch
 *		of the tile should always return the same linear offset.
 *
 * INPUT
 *		TileWidth:		global variable containing current map's tile width
 *		TileHeight:		global variable containing current map's tile height
 *		XCharsInTile:	global variable containing # of chars across per tile
 *		YCharsInTile:	global variable containing # of chars down per tile
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns index within tile of character.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static WORD IndexFromTileOffsets(PlotType *pplt, WORD wXTOffset, WORD wYTOffset)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "IndexFromTileOffsets";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pplt)
	{
		return (((pplt->Plot_Flags & ubYFlipBit) ?
					(TileHeight - wYPixHeight - wYTOffset) : wYTOffset )/wYPixHeight) * XCharsInTile +
				 (((pplt->Plot_Flags & ubXFlipBit) ?
					(TileWidth - wXPixWidth - wXTOffset) : wXTOffset )/wXPixWidth);
	}
	return 0;

} /* IndexFromTileOffsets */


/*********************************************************************
 *
 * CharSlotFromTileSetIndex
 *
 * SYNOPSIS
 *		static UWORD CharSlotFromTileSetIndex(PlotType *pplt, UWORD TileIndex)
 *
 * PURPOSE
 *		Given a tile <pplt>, and a <TileIndex> within that tile,
 *		lookup if the CharSlot has already been set (!= 0xFFFF).
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		If pplt == NULL, or pplt->TileSet_ID == 0, return 0.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static UWORD CharSlotFromTileSetIndex(PlotType *pplt, UWORD TileIndex)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CharSlotFromTileSetIndex";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pplt)
	{
		if (pplt->TileSet_ID)
		{
			LONG lIndex		= lTileBase[pplt->TileSet_ID] + pplt->Tile_ID;
			int TileBank	= (int) (lIndex / TilesPerBank);
			int BankOffset	= (int) ((lIndex % TilesPerBank) * sizeofTile + (TileIndex * sizeof(UWORD)) + (sizeof(UBYTE) * CharsInTile));

#if 0
			UWORD uw;

			ReadXTRA(pxmtTileBanks[TileBank], BankOffset, sizeof(UWORD), &uw);

			return uw;
#else
			UBYTE *pub = ActivateXTRAatWin(pxmtTileBanks[TileBank], 2);
			UWORD uw = *(UWORD *) (pub+BankOffset);

			ReleaseXTRA(pxmtTileBanks[TileBank]);

			return uw;
#endif
		}
		return 0;
	}
	else
		return 0;

} /* CharSlotFromTileSetIndex */


/*********************************************************************
 *
 * ColorFromTileSetIndex
 *
 * SYNOPSIS
 *		static UBYTE ColorFromTileSetIndex(PlotType *pplt, UWORD TileIndex)
 *
 * PURPOSE
 *		Given a tile <pplt>, lookup the color for that tile.
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
static UBYTE ColorFromTileSetIndex(PlotType *pplt, UWORD TileIndex)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ColorFromTileSetIndex";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pplt)
	{
		if (pplt->TileSet_ID)
		{
			LONG lIndex		= lTileBase[pplt->TileSet_ID] + pplt->Tile_ID;
			int TileBank	= (int) (lIndex / TilesPerBank);
			int BankOffset	= (int) ((lIndex % TilesPerBank) * sizeofTile + (TileIndex * sizeof(UBYTE)));

#if 0
			UBYTE ub;

			ReadXTRA(pxmtTileBanks[TileBank], BankOffset, sizeof(UBYTE), &ub);

			return ub;
#else
			UBYTE *pub = ActivateXTRAatWin(pxmtTileBanks[TileBank], 2);
			UBYTE ub = *(UBYTE *) (pub+BankOffset);

			ReleaseXTRA(pxmtTileBanks[TileBank]);

			return ub;
#endif
		}
		return 0;
	}
	else
		return 0;

} /* ColorFromTileSetIndex */


/*********************************************************************
 *
 * SetCharSlotForTileSetIndex
 *
 * SYNOPSIS
 *		static void SetCharSlotForTileSetIndex(UWORD slot, PlotType *pplt, UWORD TileIndex)
 *
 * PURPOSE
 *		Given a tile <pplt>, and a <TileIndex> within that tile,
 *		set the <slot> for that <TileIndex>.
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
static void SetCharSlotForTileSetIndex(UWORD slot, PlotType *pplt, UWORD TileIndex)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetCharSlotForTileSetIndex";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pplt && pplt->TileSet_ID)
	{
		LONG lIndex		= lTileBase[pplt->TileSet_ID] + pplt->Tile_ID;
		int TileBank	= (int) (lIndex / TilesPerBank);
		int BankOffset	= (int) ((lIndex % TilesPerBank) * sizeofTile + (TileIndex * sizeof(UWORD)) + (sizeof(UBYTE) * CharsInTile));

		WriteXTRA(&slot, sizeof(UWORD), pxmtTileBanks[TileBank], BankOffset);
	}

} /* SetCharSlotForTileSetIndex */


/*********************************************************************
 *
 * SetColorForTileSetIndex
 *
 * SYNOPSIS
 *		static void SetColorForTileSetIndex(UBYTE ubColor, PlotType *pplt, UWORD TileIndex)
 *
 * PURPOSE
 *		Given a tile <pplt>, set the <ubColor> for that tile.
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
static void SetColorForTileSetIndex(UBYTE ubColor, PlotType *pplt, UWORD TileIndex)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetColorForTileSetIndex";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pplt && pplt->TileSet_ID)
	{
		LONG lIndex		= lTileBase[pplt->TileSet_ID] + pplt->Tile_ID;
		int TileBank	= (int) (lIndex / TilesPerBank);
		int BankOffset	= (int) ((lIndex % TilesPerBank) * sizeofTile + (TileIndex * sizeof(UBYTE)));

		WriteXTRA(&ubColor, sizeof(UBYTE), pxmtTileBanks[TileBank], BankOffset);
	}

} /* SetColorForTileSetIndex */


/*********************************************************************
 *
 * CloseSaveMap
 *
 * SYNOPSIS
 *		void CloseSaveMap(void)
 *
 * PURPOSE
 *		Clean up memory we allocated.
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
void CloseSaveMap(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CloseSaveMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	FreeXTRAArray(&pxmtCharBanks, CharBanksMax);
	FreeXTRAArray(&pxmtAddrLBanks, AddrLBanksMax);
	FreeXTRAArray(&pxmtAddrHBanks, AddrHBanksMax);

} /* CloseSaveMap */


/*********************************************************************
 *
 * OpenSaveMap
 *
 * SYNOPSIS
 *		static BOOL OpenSaveMap(void)
 *
 * PURPOSE
 *		Allocate memory to save character map.
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
static BOOL OpenSaveMap(void)
{

	LONG lSNESMapSize;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenSaveMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pxmtCharBanks	= NULL;
	pxmtAddrLBanks	= NULL;
	pxmtAddrHBanks	= NULL;

	/* Allocate memory needed to store map characters. */

	lSNESMapRowBytes	= (LONG) wXSNESMapMaxex * sizeofMapChar;
	lSNESMapSize		= lSNESMapRowBytes * wYSNESMapMaxex;
	CharsPerBank		= (unsigned) ((xtrBankSize / lSNESMapRowBytes) * wXSNESMapMaxex);
	CharBanksMax		= (int) (lSNESMapSize / (CharsPerBank * sizeofMapChar) + 1);

	/* Allocate XTRA memory banks that we need. */

	if (!AllocXTRAArray(&pxmtCharBanks, CharBanksMax, 0, xtrBankSize))
/**/	goto ABORT;

	/* Allocate memory needed to save pointers to start of each map row. */

	AddrLPerBank	= xtrBankSize / sizeofMapAddrL;
	AddrHPerBank	= xtrBankSize / sizeofMapAddrH;

	lAddrLSize		= wYSNESMapMaxex * sizeofMapAddrL;
	lAddrHSize		= wYSNESMapMaxex * sizeofMapAddrH;

	AddrLBanksMax	= (int) (lAddrLSize / AddrLPerBank + 1);
	AddrHBanksMax	= (int) (lAddrHSize / AddrHPerBank + 1);

	if (!AllocXTRAArray(&pxmtAddrLBanks, AddrLBanksMax, 0, xtrBankSize))
/**/	goto ABORT;

	if (!AllocXTRAArray(&pxmtAddrHBanks, AddrHBanksMax, 0, xtrBankSize))
/**/	goto ABORT;

	return TRUE;

ABORT:
	CloseSaveMap();
	return FALSE;

} /* OpenSaveMap */


/*********************************************************************
 *
 * SaveMapIndex
 *
 * SYNOPSIS
 *		static void SaveMapIndex(WORD x, WORD y,
 *						BOOL fYFlip, BOOL fXFlip, UBYTE ubColorset, UWORD uwIndex)
 *
 * PURPOSE
 *		Save <uw> at map coordinate <x, y>.
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
static void SaveMapIndex(WORD x, WORD y,
						BOOL fYFlip, BOOL fXFlip, UBYTE ubColorset, UWORD uwIndex)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveMapIndex";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		LONG lIndex		= lSNESMapRowBytes * (y+1) + ((x+1) * sizeofMapChar);
		int CharBank	= (int) (lIndex / (CharsPerBank * sizeofMapChar));
		int BankOffset	= (int) (lIndex % (CharsPerBank * sizeofMapChar));

		UWORD uw;

		if (fTarget == fSNES)
		{
			uw = (fYFlip ? uwSNESFlipYBit : 0) |
				  (fXFlip ? uwSNESFlipXBit : 0) |
				 ((ubColorset & wSNESColorMask) << wSNESColorBit) | uwIndex;
		}
		else
		{
			uw = (fYFlip ? uwGenesisFlipYBit : 0) |
				  (fXFlip ? uwGenesisFlipXBit : 0) |
				 ((ubColorset & wGenesisColorMask) << wGenesisColorBit) |
				 ((uwIndex & 0xff) << 8) |
				 ((uwIndex & 0xff00) >> 8);
		}

		WriteXTRA(&uw, sizeofMapChar, pxmtCharBanks[CharBank], BankOffset);
	}
//	uwMap[y * wXScrWidth + x] = uw;

} /* SaveMapIndex */


/*********************************************************************
 *
 * MakeMapRowPointers
 *
 * SYNOPSIS
 *		static void MakeMapRowPointers(void)
 *
 * PURPOSE
 *		Make pointers to start of each map row.
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
static void MakeMapRowPointers(void)
{
	WORD y;
	WORD LastAddrLBank = -1;
	WORD LastAddrHBank = -1;

	unsigned uwAddrL = (unsigned) (adrTarget.ulMapBankAddr & 0xFFFFU);
	UWORD uwAddrH = (UWORD) (adrTarget.ulMapBankAddr >> 16);

	unsigned uwAddrNL;
	unsigned uwAddrNH;

	int RowsPerBank = (int) (adrTarget.ulMapBankSize / lSNESMapRowBytes);
	WORD RowsCrnt = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MakeMapRowPointers";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		ULONG ul = ((ULONG) uwAddrH << 16) + uwAddrL + adrTarget.ulMapBankSkip;

		uwAddrNL = (unsigned) (ul & 0xFFFFU);
		uwAddrNH = (unsigned) (ul >> 16);
	}

	for (y = 0; y < wYSNESMapMaxex; y++)
	{
		int AddrLBank		= (int) ((LONG) y * sizeofMapAddrL / AddrLPerBank);
		int AddrLOffset	= (int) ((LONG) y * sizeofMapAddrL % AddrLPerBank);
		int AddrHBank		= (int) ((LONG) y * sizeofMapAddrH / AddrHPerBank);
		int AddrHOffset	= (int) ((LONG) y * sizeofMapAddrH % AddrHPerBank);

		UBYTE *pubAddrL;
		UBYTE *pubAddrH;

		if (AddrLBank != LastAddrLBank)
		{
			if (LastAddrLBank >= 0)
				UpdateXTRA(pxmtAddrLBanks[LastAddrLBank]);
			pubAddrL = ActivateXTRAatWin(pxmtAddrLBanks[AddrLBank], 0);
			LastAddrLBank = AddrLBank;
		}
		if (AddrHBank != LastAddrHBank)
		{
			if (LastAddrHBank >= 0)
				UpdateXTRA(pxmtAddrHBanks[LastAddrLBank]);
			pubAddrH = ActivateXTRAatWin(pxmtAddrHBanks[AddrHBank], 2);
			LastAddrHBank = AddrHBank;
		}
		*(UWORD *) (pubAddrL + AddrLOffset) = uwAddrL;
		*(UBYTE *) (pubAddrH + AddrHOffset) = uwAddrH;
		uwAddrL += (unsigned) lSNESMapRowBytes;
		if (++RowsCrnt >= RowsPerBank)
		{
			ULONG ul;

			uwAddrL = uwAddrNL;
			uwAddrH = uwAddrNH;
			ul = ((ULONG) uwAddrH << 16) + uwAddrL + adrTarget.ulMapBankSkip;

			uwAddrNL = (unsigned) (ul & 0xFFFFU);
			uwAddrNH = (unsigned) (ul >> 16);

			RowsCrnt = 0;
		}
	}
	UpdateXTRA(pxmtAddrLBanks[LastAddrLBank]);
	UpdateXTRA(pxmtAddrHBanks[LastAddrLBank]);

} /* MakeMapRowPointers */


/*********************************************************************
 *
 * CloseDownloader
 *
 * SYNOPSIS
 *		static void CloseDownloader(void)
 *
 * PURPOSE
 *		Clean up memory we allocated.
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
static void CloseDownloader(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CloseDownloader";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	CloseSaveMap();

	CloseCharSlotFromTileSetIndex();

} /* CloseDownloader */


/*********************************************************************
 *
 * OpenDownloader
 *
 * SYNOPSIS
 *		static BOOL OpenDownloader(TileSpaceType *ptsp, RoomType *prmt)
 *
 * PURPOSE
 *		Initialize variables before we download.
 *
 * INPUT
 *
 *
 * EFFECTS
 *		Global variables set:
 *		wXMapMinex, wXMapMaxex, wYMapMinex, wYMapMaxex
 *		wXSNESMapMaxex, wYSNESMapMaxex
 *
 * RETURN VALUE
 *		TRUE if successful, FALSE if failed.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL OpenDownloader(TileSpaceType *ptsp, RoomType *prmt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenDownloader";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	InitGrabCharacters();

	TileWidth		= prmt->FloorLayer->cxTile;
	TileHeight		= prmt->FloorLayer->cyTile;

	XCharsInTile	= TileWidth / wXPixWidth;
	YCharsInTile	= TileHeight / wYPixHeight;

	wXMapMinex = -1;
	wXMapMaxex = prmt->FloorLayer->ctilx * XCharsInTile;
	wYMapMinex = -1;
	wYMapMaxex = prmt->FloorLayer->ctily * YCharsInTile;

	wXSNESMapMaxex = max(prmt->FloorLayer->ctilx * XCharsInTile + 2, wSNESMinMapWidth);
	wYSNESMapMaxex = max(prmt->FloorLayer->ctily * YCharsInTile + 2, wSNESMinMapHeight);

	/* Make sure room tile width is some multiple of display char width */

	if (TileWidth % wXPixWidth != 0)
		return FALSE;
	if (TileHeight % wYPixHeight != 0)
		return FALSE;

	if (!OpenCharSlotFromTileSetIndex(ptsp))
		return FALSE;

	if (!OpenSaveMap())
		return FALSE;

	return TRUE;

} /* OpenDownloader */


/*********************************************************************
 *
 * GrabCharacters
 *
 * SYNOPSIS
 *		static BOOL GrabCharacters(RoomType *prt, WORD wxrt, WORD wyrt)
 *
 * PURPOSE
 *		Fill in character set and define map.
 *
 * INPUT
 *		prt:			pointer to room to display
 *		wxrt:			leftmost x tile position to start display
 *		wyrt:			topmost y tile position to start display
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns TRUE if successful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL GrabCharacters(RoomType *prt, WORD wxrt, WORD wyrt)
{

	LONG lCharCnt = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GrabCharacters";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

//	memset(uwMap, 0, wYScrHeight * wXScrWidth * sizeof(UWORD));	//K

	PushClipValues(0, 0, 32767, 32767);
	{
		WORD wXCharCrnt = wxrt * XCharsInTile;
		WORD wYCharCrnt = wyrt * YCharsInTile;

		WORD wDeltaMacin = 0;
		WORD wDeltaCrnt = 0;

		WORD fSpiralState = 3;
		WORD fSpiralDone = 0;

		WORD wXDownMinex = wXMapMinex;
		WORD wXDownMaxex = wXMapMaxex;
		WORD wYDownMinex = wYMapMinex;
		WORD wYDownMaxex = wYMapMaxex;

		if (DownloadOneScreenSTATE)
		{
			if (wXCharCrnt - (wXScrWidth/2)-1 < wXMapMinex)
			{
				wXDownMinex = max(wXMapMinex, wXCharCrnt - (wXScrWidth/2)-1);
				wXDownMaxex = min(wXMapMaxex, wXDownMinex + wXScrWidth+1);
			}
			else
			{
				wXDownMaxex = min(wXMapMaxex, wXCharCrnt + (wXScrWidth/2));
				wXDownMinex = max(wXMapMinex, wXDownMaxex - wXScrWidth-1);
			}
			if (wYCharCrnt - (wYScrHeight/2)-1 < wYMapMinex)
			{
				wYDownMinex = max(wYMapMinex, wYCharCrnt - (wYScrHeight/2)-1);
				wYDownMaxex = min(wYMapMaxex, wYDownMinex + wYScrHeight-3);
			}
			else
			{
				wYDownMaxex = min(wYMapMaxex, wYCharCrnt + (wYScrHeight/2)-2);
				wYDownMinex = max(wYMapMinex, wYDownMaxex - wYScrHeight+3);
			}
		}

		wXSNESCharInit = max(min(wXCharCrnt - (wXScrWidth/2), wXSNESMapMaxex - wXScrWidth-2), 0);
		wYSNESCharInit = max(min(wYCharCrnt - (wYScrHeight/2), wYSNESMapMaxex - wYScrHeight-2), 0);

		for ( ; ; )
		{
			if (fSpiralDone == (fTopDone | fRightDone | fDownDone | fLeftDone))
				break;

			if (wXDownMinex < wXCharCrnt && wXCharCrnt < wXDownMaxex &&
				 wYDownMinex < wYCharCrnt && wYCharCrnt < wYDownMaxex)
			{
				WORD wXrt = wXCharCrnt / XCharsInTile;
				WORD wYrt = wYCharCrnt / YCharsInTile;

				WORD wXrtOffset = (wXCharCrnt % XCharsInTile) * wXPixWidth;
				WORD wYrtOffset = (wYCharCrnt % YCharsInTile) * wYPixHeight;

				UWORD uwSlot;
				WORD  wCharColor;
				WORD  TileIndex;
				UBYTE ubFlags;
				PlotType *pPlot;

				if (lCharCnt % 1000 == 0)
				{
					char sz[256];

					sprintf(sz, "Collecting %ld characters...", lCharCnt);
					ShowStatus(sz);
				}
				lCharCnt++;

				pPlot = GetRoomPlot(prt, wXrt, wYrt);

				TileIndex = IndexFromTileOffsets(pPlot, wXrtOffset, wYrtOffset);

				/* Has this tile already been defined as a character? */

				if ((uwSlot = CharSlotFromTileSetIndex(pPlot, TileIndex)) == 0xFFFFU)
				{
					/* No, get character image, see if matches some other. */

					ubFlags = GetCharByteMap(&bmChar, pPlot, wXrtOffset, wYrtOffset);

					CharImageFromByteMap(pubCharImageTemp, &bmChar);
					wCharColor = ColorFromByteMap(&bmChar);

					/* Does char image match some other in char set? */

					if ((uwSlot = FindAlreadyDefinedChar(pubCharImageTemp)) != 0xFFFFU)
					{
						/* Yes. Set map char to matching char in char set. */

						SaveMapIndex(wXCharCrnt, wYCharCrnt,
							ubFlags & ubYFlipBit, ubFlags & ubXFlipBit,
							wCharColor, uwSlot);

//						fMapValid[wYt * wXScrWidth + wXt] = TRUE;
//						fCharFlags[uwSlot] |= fCharDefined;
//						uwCharUsage[uwSlot]++;
					}
					else
					{
						/* No. See if we can add character to char set. */

						if ((uwSlot = FindNextEmptyCharSlot()) != 0xFFFFU)
						{
							/* Yes, add the character. */

							SaveCharImage(uwSlot, pubCharImageTemp);

							SaveMapIndex(wXCharCrnt, wYCharCrnt,
								ubFlags & ubYFlipBit, ubFlags & ubXFlipBit,
								wCharColor, uwSlot);

//							fMapValid[wYt * wXScrWidth + wXt] = TRUE;
							fCharFlags[uwSlot] |= fCharDefined;
//							uwCharUsage[uwSlot]++;
						}
						else
						{
							/* We have no more space for new characters.
								All future reference to this TileSet_ID &
								Tile_ID should return 0. */

							uwSlot = 0;

//							fMapValid[wYt * wXScrWidth + wXt] = FALSE;
						}
					}
					SetCharSlotForTileSetIndex(uwSlot, pPlot, TileIndex);
					if (wCharColor >= 0)
						SetColorForTileSetIndex(wCharColor, pPlot, TileIndex);
				}
				else
				{
					if (pPlot->Plot_Flags & 0x80)
					{
						/* Colorset is being set by user (over-write). */

						wCharColor = pPlot->Plot_Flags;
					}
					else
					{
						/* Get color from tile. */

						wCharColor = ColorFromTileSetIndex(pPlot, TileIndex);
					}

					SaveMapIndex(wXCharCrnt, wYCharCrnt,
						pPlot->Plot_Flags & ubYFlipBit,
						pPlot->Plot_Flags & ubXFlipBit,
						wCharColor, uwSlot);

//					fMapValid[wYt * wXScrWidth + wXt] = TRUE;
//					uwCharUsage[uwSlot]++;
				}
			}
			wDeltaCrnt++;
			switch (fSpiralState)
			{
			case	0:
					if (wDeltaCrnt <= wDeltaMacin)
					{
						wXCharCrnt++;
						if (wXCharCrnt <= wXDownMinex)
						{
							wDeltaCrnt += (wXDownMinex+1) - wXCharCrnt;
							wXCharCrnt  = (wXDownMinex+1);
						}
						else if (wXCharCrnt >= wXDownMaxex ||
									wYCharCrnt <= wYDownMinex || wYCharCrnt >= wYDownMaxex)
						{
							wXCharCrnt += wDeltaMacin - wDeltaCrnt;
							wDeltaCrnt  = wDeltaMacin;
						}
						if (wXCharCrnt >= wXDownMaxex)
						{
							fSpiralDone |= fRightDone;
						}
					}
					else
					{
						wYCharCrnt++;
						wDeltaCrnt = 1;
						fSpiralState = 1;
					}
					break;
			case	1:
					if (wDeltaCrnt <= wDeltaMacin)
					{
						wYCharCrnt++;
						if (wYCharCrnt <= wYDownMinex)
						{
							wDeltaCrnt += (wYDownMinex+1) - wYCharCrnt;
							wYCharCrnt  = (wYDownMinex+1);
						}
						else if (wYCharCrnt >= wYDownMaxex ||
									wXCharCrnt <= wXDownMinex || wXCharCrnt >= wXDownMaxex)
						{
							wYCharCrnt += wDeltaMacin - wDeltaCrnt;
							wDeltaCrnt  = wDeltaMacin;
						}
						if (wYCharCrnt >= wYDownMaxex)
						{
							fSpiralDone |= fDownDone;
						}
					}
					else
					{
						wXCharCrnt--;
						wDeltaMacin++;
						wDeltaCrnt = 1;
						fSpiralState = 2;
					}
					break;
			case	2:
					if (wDeltaCrnt <= wDeltaMacin)
					{
						wXCharCrnt--;
						if (wXCharCrnt >= wXDownMaxex)
						{
							wDeltaCrnt += wXCharCrnt - (wXDownMaxex-1);
							wXCharCrnt  = (wXDownMaxex-1);
						}
						else if (wXCharCrnt <= wXDownMinex ||
									wYCharCrnt <= wYDownMinex || wYCharCrnt >= wYDownMaxex)
						{
							wXCharCrnt -= wDeltaMacin - wDeltaCrnt;
							wDeltaCrnt  = wDeltaMacin;
						}
						if (wXCharCrnt <= wXDownMinex)
						{
							fSpiralDone |= fLeftDone;
						}
					}
					else
					{
						wYCharCrnt--;
						wDeltaCrnt = 1;
						fSpiralState = 3;
					}
					break;
			case	3:
					if (wDeltaCrnt <= wDeltaMacin)
					{
						wYCharCrnt--;
						if (wYCharCrnt >= wYDownMaxex)
						{
							wDeltaCrnt += wYCharCrnt - (wYDownMaxex-1);
							wYCharCrnt  = (wYDownMaxex-1);
						}
						else if (wYCharCrnt <= wYDownMinex ||
									wXCharCrnt <= wXDownMinex || wXCharCrnt >= wXDownMaxex)
						{
							wYCharCrnt -= wDeltaMacin - wDeltaCrnt;
							wDeltaCrnt  = wDeltaMacin;
						}
						if (wYCharCrnt <= wYDownMinex)
						{
							fSpiralDone |= fTopDone;
						}
					}
					else
					{
						wXCharCrnt++;
						wDeltaMacin++;
						wDeltaCrnt = 1;
						fSpiralState = 0;
					}
					break;
			}
		}
		{
			char sz[256];

			sprintf(sz, "Collecting %ld characters...", lCharCnt);
			ShowStatus(sz);
		}
	}
	MakeMapRowPointers();
	PopClipValues();
	return TRUE;
} /* GrabCharacters */


#if 0
/*********************************************************************
 *
 * GrabPalette
 *
 * SYNOPSIS
 *		static BOOL GrabPalette(CS_Color *pc)
 *
 * PURPOSE
 *		Convert palette for downloading.
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
static BOOL GrabPalette(CS_Color *pc)
{
	WORD w;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GrabPalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (w = 1; w < uwPaletteMax; w++)
	{
		uwPalette[w] = ((pc->Red   >> 3) <<  0) |
							((pc->Green >> 3) <<  5) |
							((pc->Blue  >> 3) << 10) ;
		pc++;
	}
	return TRUE;
} /* GrabPalette */
#endif


/*********************************************************************
 *
 * SetDownloadTarget
 *
 * SYNOPSIS
 *		void SetDownloadTarget(WORD f)
 *
 * PURPOSE
 *		Set downloader to work with _fGenesis_ or _fSNES_.
 *		This routine sets <fTarget>, <GrabPalette> &
 *		<CharImageFromByteMap>. Note that these variables MUST
 *		initialized before you can call GrabNDownloadPalette()
 *		or GrabCharacters().
 *
 *		It should also be possible to attach an event to this
 *		procedure to allow the user to select which target
 *		hardware to download to. Currently it is automatic.
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
void SetDownloadTarget(WORD f)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetDownloadTarget";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((fTarget = f) == fSNES)
	{
		GrabPalette = SNESGrabPalette;
		CharImageFromByteMap = SNESCharImageFromByteMap;
//		adrTarget.uwVersion
//		adrTarget.uwADRBlockSize
		adrTarget.ulPalette = ulSNESPaletteAddress;
		adrTarget.uwPaletteCnt = uwSNESPaletteCnt;
		adrTarget.ulMapInfo = ulSNESMapInfoAddress;
//		adrTarget.uwMapInfoLength
		adrTarget.ulMapStart = ulSNESMapBankAddress;
//		adrTarget.uwMapStartMax
//		adrTarget.uwCharSetAreas
		adrTarget.ulCharAddress0 = ulSNESCharAddress0;
		adrTarget.uwCharAddress0Len = uwSNESCharAddress0Len;
		adrTarget.ulCharAddress1 = ulSNESCharAddress1;
		adrTarget.uwCharAddress1Len = uwSNESCharAddress1Len;
		adrTarget.ulMapBankAddr = ulSNESMapBankAddr256;
		adrTarget.ulMapBankSize = uwSNESMapBankSize;
		adrTarget.ulMapBankSkip = 0x010000UL;
	}
#if (dvpSNASM || dvpPsyQ || dvpPARALLEL)
	else
	{
		GrabPalette = GenesisGrabPalette;
		CharImageFromByteMap = GenesisCharImageFromByteMap;
#if dvpPARALLEL
		fParallelBlockRead(ulGenesisHeaderAddress,&adrTarget, sizeof(ADR));
//		adrTarget.uwVersion
//		adrTarget.uwADRBlockSize
//		adrTarget.ulPalette = ulParGenesisPaletteAddress;
//		adrTarget.uwPaletteCnt = uwParPaletteCnt;
//		adrTarget.ulMapInfo = ulParGenesisMapInfoAddress;
//		adrTarget.uwMapInfoLength
//		adrTarget.ulMapStart = ulParGenesisMapBankAddress;
//		adrTarget.uwMapStartMax
//		adrTarget.uwCharSetAreas
//		adrTarget.ulCharAddress0 = ulParGenesisCharAddress0;
//		adrTarget.uwCharAddress0Len = uwParGenesisCharAddress0Len;
//		adrTarget.ulCharAddress1 = ulParGenesisCharAddress1;
//		adrTarget.uwCharAddress1Len = uwParGenesisCharAddress1Len;
//		adrTarget.ulMapBankAddr = ulParGenesisMapBankAddr;
//		adrTarget.ulMapBankSize = ulParGenesisMapBankSize;
//		adrTarget.ulMapBankSkip = ulParGenesisMapBankSize;
#else
//		adrTarget.uwVersion
//		adrTarget.uwADRBlockSize
		adrTarget.ulPalette = ulGenesisPaletteAddress;
		adrTarget.uwPaletteCnt = uwGenesisPaletteCnt;
		adrTarget.ulMapInfo = ulGenesisMapInfoAddress;
//		adrTarget.uwMapInfoLength
		adrTarget.ulMapStart = ulGenesisMapBankAddress;
//		adrTarget.uwMapStartMax
//		adrTarget.uwCharSetAreas
		adrTarget.ulCharAddress0 = ulGenesisCharAddress0;
		adrTarget.uwCharAddress0Len = uwGenesisCharAddress0Len;
		adrTarget.ulCharAddress1 = ulGenesisCharAddress1;
		adrTarget.uwCharAddress1Len = uwGenesisCharAddress1Len;
		adrTarget.ulMapBankAddr = ulGenesisMapBankAddr;
		adrTarget.ulMapBankSize = ulGenesisMapBankSize;
		adrTarget.ulMapBankSkip = ulGenesisMapBankSize;
#endif
	}
#endif
} /* SetDownloadTarget */


/*********************************************************************
 *
 * GrabNDownloadPalette
 *
 * SYNOPSIS
 *		void GrabNDownloadPalette(ColorInfo *ci)
 *
 * PURPOSE
 *		Grab the palette specified by <ci>, and download it.
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
void GrabNDownloadPalette(ColorInfo *ci)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GrabNDownloadPalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		BOOL f = GrabPalette(ActivateXTRA(ci->pxtColors));
		ReleaseXTRA(ci->pxtColors);

		if (!f)
			return;
	}
	{
#if dvpSNASM
		if (!InitSNASM(wSCSIAddress))
			return;

		if (SNASMID != fTarget)
			return;
#elif dvpPsyQ
		if (!InitPsyQ(wSCSIAddress))
			return;

		if (PsyQID != fTarget)
			return;
#elif dvpCLD
//		if (!InitCLD())
//			return;
#elif dvpPARALLEL
		if (!fInitParallel())
			return;
#endif

		DownloadPalette();
#if dvpSNASM
		RestartSNASM(FALSE);
#elif dvpPsyQ
		RestartPsyQ(FALSE);
#elif dvpCLD
//		Lawrence sez nothing goes here
#elif dvpPARALLEL
//		dcc sez nothing goes here
#endif
	}

} /* GrabNDownloadPalette */


/*********************************************************************
 *
 * DoDownloadMap
 *
 * SYNOPSIS
 *		BOOL DoDownloadMap(TileSpaceType *ptsp, RoomType *prt,
 *									WORD wxrt, WORD wyrt, WORD wColors)
 *
 * PURPOSE
 *		Actually download the map, character set & palette.
 *
 * INPUT
 *		prt:			pointer to room to display
 *		wxrt:			leftmost x tile position to start display
 *		wyrt:			topmost y tile position to start display
 *		wColors:		number of colors in characters to download
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns TRUE if successful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL DoDownloadMap(TileSpaceType *ptsp, RoomType *prt,
						WORD wxrt, WORD wyrt, WORD wColors)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DoDownloadMap";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowStatus("Initializing downloader");

	if (fTarget == fSNES)
	{
		/* SNES downloading */

		if (wColors <= 16)
		{
			wXScrWidth = wXSNESScrWidth;

			/* 16-color characters */

			uwBitsPerPixel = 4;
			uwBytesPerChar = 32;
			uwCharMax		= 1024;
			uwCharsInBank	= 1024;

			adrTarget.ulMapBankAddr = ulSNESMapBankAddr16;
		}
		else
		{
			wXScrWidth = wXSNESScrWidth;

			/* 256-color characters */

			uwBitsPerPixel = 8;
			uwBytesPerChar = 64;
			uwCharMax		= 1024-128;
			uwCharsInBank	= 512;

			adrTarget.ulMapBankAddr = ulSNESMapBankAddr256;
		}
		GrabPalette = SNESGrabPalette;
		CharImageFromByteMap = SNESCharImageFromByteMap;
	}
	else
	{
		/* Genesis downloading */

		wXScrWidth = wXGenesisScrWidth;

		/* 16-color characters */

		uwBitsPerPixel = 4;
		uwBytesPerChar = 32;
#if dvpPARALLEL
		uwCharMax		= (unsigned) ((ULONG) adrTarget.uwCharAddress0Len +
								(ULONG) adrTarget.uwCharAddress1Len) / uwBytesPerChar;
		uwCharsInBank	= adrTarget.uwCharAddress0Len / uwBytesPerChar;
#else
//		uwCharMax		= 1536;
//		uwCharsInBank	= 1024;
		uwCharMax		= (unsigned) (((ULONG) adrTarget.uwCharAddress0Len +
								(ULONG) adrTarget.uwCharAddress1Len) / uwBytesPerChar);
		uwCharsInBank	= adrTarget.uwCharAddress0Len / uwBytesPerChar;
#endif

		GrabPalette = GenesisGrabPalette;
		CharImageFromByteMap = GenesisCharImageFromByteMap;
	}

	if (!OpenDownloader(ptsp, prt))
		return FALSE;

	if (!GrabCharacters(prt, wxrt, wyrt))
		return FALSE;

	{
		BOOL f = GrabPalette(ActivateXTRA(GlobalColors->pxtColors));
		ReleaseXTRA(GlobalColors->pxtColors);

		if (!f)
			return FALSE;
	}

#if dvpSNASM
	if (!InitSNASM(wSCSIAddress))
		return FALSE;

	if (SNASMID != fTarget)
		return FALSE;
#elif dvpPsyQ
	if (!InitPsyQ(wSCSIAddress))
		return FALSE;

	if (PsyQID != fTarget)
		return FALSE;
#elif dvpCLD
//	if (!InitCLD())
//		return FALSE;
#elif dvpPARALLEL
	if (!fInitParallel())
		return FALSE;
#endif

	DownloadPalette();
	DownloadMap();
	DownloadCharSet();

#if dvpSNASM
	RestartSNASM(TRUE);
#elif dvpPsyQ
	RestartPsyQ(TRUE);
#elif dvpCLD
	ResetCLD();
#elif dvpPARALLEL
	fRestartParallelTarget();
#endif

	CloseDownloader();

	return TRUE;
} /* DoDownloadMap */

#endif/*!dvpNONE*/

