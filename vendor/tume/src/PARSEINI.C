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
 * PARSEINI.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 01/20/92
 *   MODIFIED : 02/09/95
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Read tUME.INI file and process it.
 *
 * HISTORY
 *		08/13/94 Saturday (dcc) - add [Save Options] processing
 *
*/

#include <echidna/platform.h>
#include <echidna/listfunc.h>
#include <echidna/eui.h>
#include <echidna/mouse.h>
#include <stdio.h>
#include <string.h>
#include "switches.h"
#include "switch1.h"

#include "count.h"
#include "download.h"
#include <echidna/readini.h>
#include "tilebits.h"
#include "grid.h"
#include "parseini.h"						/* Verify function prototypes. */
#include "tuglbl.h"
#include "typelist.h"
#include "switch2.h"
#include "ioini.h"
#include "search.h"
#include "objlist.h"

#if dvpPARALLEL
#include "paraglue.h"
#endif

#include "assert.h"

extern short	 ColorReqDivideX;
extern short	 ColorReqDivideY;
extern short	 ColorRange[3];
extern short	 ColorMax[3];

extern BOOL		 fSearchAsSpecified;
extern BOOL		 fSearchCurrentDir;
#if SaveNew
extern UWORD	 uwSaveFormat;
#endif //SaveNew

extern short	 IdiotNumbers;

/**************************** C O N S T A N T S ***************************/

#define szDigits "0123456789"

/* Table of LinkCharVariables.fType */

#if NEWTUME
#define maskType	(3U << 14)
#define maskIndex	((1U<<12)-1)

#define fUBMask	(0U << 14)
#define fWORD		(1U << 14)
#define fSZ			(2U << 14)
#define fCEN		(3U << 14)

/* BUGBUG: fCEN must currently always be used in conjunction with fIndirect */

/* BUGBUG: fIndirect is currently only implemented for fCEN | fWORD ! */

#define fIndirect	(1U << 13)

/* BUGBUG: fArray is currently only implemented for (fCEN | fWORD) & fIndirect ! */

#define fArray		(1U << 12)

#else
#define fUBMask	(0)
#define fWORD		(1)
#define fSZ			(2)
#endif

/******************************** T Y P E S *******************************/

typedef struct
{
	char *sz;
	void *pVar;
#if NEWTUME
	UWORD fType;
#else
	char fType;
#endif
} LinkCharVariables;

typedef struct
{
	char *sz;
	UBYTE ubFlag;
} MergeCharFlag;

/****************************** E X T E R N S *****************************/

extern short wShiftXUnit;
extern short wShiftYUnit;

/****************************** G L O B A L S *****************************/

static UBYTE ub[1];

#define wCntMax 2
static short w[wCntMax];

static char szEnable[] = "Enable=";
#define wLenSzEnable (7)

static char szConfig[] = "Config=";
#define wLenSzConfig (7)

static char szsd[] = "SearchDir=";

static char szxflip[]= "[X Flip Bit]";
static char szyflip[]= "[Y Flip Bit]";
static char sztd[]	= "[Tile Display]";
static char szrd[]	= "[Room Defaults]";
static char szcc[]	= "[Count Characters]";
static char szcr[]	= "[Color Requester]";
static char szlo[]	= "[Load Options]";
static char szcmg[]	= "[Color Mask Groups]";
static char sztme[]	= "[Tile Mask Events]";
static char szdme[]	= "[Tile Mask Display Events]";
static char szcme[]	= "[Cursor Movement Events]";
static char szsle[]	= "[Shift Layer Events]";
static char szzme[]	= "[Zoom Events]";
static char szpm[]	= "[Print Maps]";
static char szdwn[]	= "[Download]";
static char szinit[]	= "[Initial Events]";
static char szms[]	= "[Mouse]";
static char szfr[]	= "[File Requester]";
static char szsea[]	= "[Search Options]";
static char szsl[]	= "[Shift Units]";
#if SaveNew
static char szsv[]	= "[Save Options]";
#endif //SaveNew

char PrintFile[256] = "LPT1";				/* File to print to */

static char szcg[80];						/* Section with list of character groups */

static char sztmeErr[] = "Tile Mask";
static char szdmeErr[] = "Tile Mask Display";
static char szcmvErr[] = "Cursor movement";
static char szsleErr[] = "Shift Layer";
static char szctmErr[] = "Color Tile Mask";
static char szcpmErr[] = "Color Pixel Mask";
static char szcmeErr[] = "Color Mask Enable";
static char szcmcErr[] = "Color Mask Config";
static char szzmeErr[] = "Zoom Amount";

#if NEWTUME
static char szXO[]	= "GuideXOrigin=";
static char szYO[]	= "GuideYOrigin=";
static char szGV[]	= "GuideVisible=";
#endif

static LinkCharVariables lXFlipBit[] =
{
	{	"Enable=",			&ubXFlipBit, fUBMask }
};

#define wXFlipBitCnt (sizeof(lXFlipBit)/sizeof(LinkCharVariables))

static LinkCharVariables lYFlipBit[] =
{
	{	"Enable=",			&ubYFlipBit, fUBMask }
};

#define wYFlipBitCnt (sizeof(lYFlipBit)/sizeof(LinkCharVariables))

static LinkCharVariables lCountChar[] =
{
	{	"CharacterWidth=",	&uwCharWidth, fWORD },
	{	"CharacterHeight=",	&uwCharHeight, fWORD },
	{	"CompareMask=",		&ubCharacterANDMask, fUBMask },
	{	"Groups=",				&szcg[1], fSZ }
};

#define wCountCharCnt (sizeof(lCountChar)/sizeof(LinkCharVariables))

static LinkCharVariables lColorReq[] =
{
	{	"ColorReqDivideX=",	&ColorReqDivideX, fWORD },
	{	"ColorReqDivideY=",	&ColorReqDivideY, fWORD },
	{	"RedRange=",			&ColorRange[0], fWORD },
	{	"GreenRange=",			&ColorRange[1], fWORD },
	{	"BlueRange=",			&ColorRange[2], fWORD }
};

#define wColorReqCnt (sizeof(lColorReq)/sizeof(LinkCharVariables))

static LinkCharVariables lLoadOpt[] =
{
	{	"SearchAsSpecified=",&fSearchAsSpecified, fWORD },
	{	"SearchCurrentDir=",	&fSearchCurrentDir, fWORD }
};

#define wLoadOptCnt (sizeof(lLoadOpt)/sizeof(LinkCharVariables))

static LinkCharVariables lTileDisplay[] =
{
	{	"ShowXTiles=",		&wShowXTiles, fWORD },
	{	"ShowYTiles=",		&wShowYTiles, fWORD }
};

#define wTileDisplayCnt (sizeof(lTileDisplay)/sizeof(LinkCharVariables))

static LinkCharVariables lRoomDefaults[] =
{
	{	"GridXSize=",		&wDefaultGridXWidth, fWORD },
	{	"GridYSize=",		&wDefaultGridYHeight, fWORD },
	{	"GridXOrigin=",	&wDefaultGridXOrigin, fWORD },
	{	"GridYOrigin=",	&wDefaultGridYOrigin, fWORD },
	{	"UseGrid=",			&fDefaultUseGrid, fWORD },
#if NEWTUME
	{	"GuideXSize=",		&pcenDefaultGuideDC, fCEN | fIndirect | 1 },
	{	"GuideYSize=",		&pcenDefaultGuideDC, fCEN | fIndirect | 0 },
	{	szXO,					&pcenDefaultGuideXOrigin, fCEN | fIndirect | 1 },
	{	szXO,					&pcenDefaultGuideXOrigin, fCEN | fIndirect | 0 },
	{	szYO,					&pcenDefaultGuideYOrigin, fCEN | fIndirect | 1 },
	{	szYO,					&pcenDefaultGuideYOrigin, fCEN | fIndirect | 0 },
	{	"GuideA[",			&pcenDefaultGuideA, fCEN | fIndirect | fArray },
	{	"GuideB[",			&pcenDefaultGuideB, fCEN | fIndirect | fArray },
	{	"GuideDC[",			&pcenDefaultGuideDC, fCEN | fIndirect | fArray },
	{	"GuideXOrigin[",	&pcenDefaultGuideXOrigin, fCEN | fIndirect | fArray },
	{	"GuideYOrigin[",	&pcenDefaultGuideYOrigin, fCEN | fIndirect | fArray },
	{	"ShowGuide[",		&fDefaultShowGuide, fWORD | fIndirect | fArray },
	{	szGV,					&fDefaultGuideVisible, fWORD },
	{	szGV,					&fDefaultShowGuide, fWORD | fIndirect | 1 },
	{	szGV,					&fDefaultShowGuide, fWORD | fIndirect | 0 },
#else
	{	"GuideXSize=",		&wDefaultGuideXWidth, fWORD },
	{	"GuideYSize=",		&wDefaultGuideYHeight, fWORD },
	{	"GuideXOrigin=",	&wDefaultGuideXOrigin, fWORD },
	{	"GuideYOrigin=",	&wDefaultGuideYOrigin, fWORD },
	{	"GuideVisible=",	&fDefaultGuideVisible, fWORD },
#endif
	{	"ZoomIndex=",		&wDefaultZoomIndex, fWORD }
};

#define wPrintCnt (sizeof(lPrint)/sizeof(LinkCharVariables))

static LinkCharVariables lPrint[] =
{
	{	"PrintTo=",				&PrintFile[0], fSZ },
	{	"NumberPages=",		&IdiotNumbers, fWORD }
};

#define wDownCnt (sizeof(lDown)/sizeof(LinkCharVariables))

static LinkCharVariables lDown[] =
{
	{	"Enable=",				&fEnableDownload, fWORD },
#if   dvpPsyQ
	{	"PsyQID=",				&wSCSIAddress, fWORD }
#elif dvpSNASM
	{	"SNASMID=",				&wSCSIAddress, fWORD }
#elif dvpPARALLEL
	{	"LPT=",					&w[0], fWORD }
#endif
};

#define wRoomDefaultsCnt (sizeof(lRoomDefaults)/sizeof(LinkCharVariables))

static LinkCharVariables lMouse[] =
{
	{	"MouseXRes=",		&w[0], fWORD },
	{	"MouseYRes=",		&w[1], fWORD }
};

#define wMouseCnt (sizeof(lMouse)/sizeof(LinkCharVariables))

static LinkCharVariables lFileReq[] =
{
	{	"DirsAtTop=",		&w[0], fWORD }
};

#define wFileReqCnt (sizeof(lFileReq)/sizeof(LinkCharVariables))

static LinkCharVariables lSearchOpt[] =
{
	{	"FlagBits=",		&ub[0], fUBMask }
};

#define wSearchOptCnt (sizeof(lSearchOpt)/sizeof(LinkCharVariables))

static LinkCharVariables lShiftUnits[] =
{
	{	"ShiftXUnit=",		&wShiftXUnit, fWORD },
	{	"ShiftYUnit=",		&wShiftYUnit, fWORD }
};

#define wShiftUnitsCnt (sizeof(lShiftUnits)/sizeof(LinkCharVariables))

static MergeCharFlag lMerge[] =
{
	{	"MergeDuplicates",	bMergeDuplicates },
	{	"MergeXFlips",			bMergeXFlips },
	{	"MergeYFlips",			bMergeYFlips },
	{	"MergeXYFlips",		bMergeXYFlips }
};

#define wMergeCnt (sizeof(lMerge)/sizeof(MergeCharFlag))

#if SaveNew
static LinkCharVariables lSaveOpt[] =
{
	{	"SaveFormat=",&uwSaveFormat, fWORD },
};

#define wSaveOptCnt (sizeof(lSaveOpt)/sizeof(LinkCharVariables))
#endif //SaveNew

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

static UBYTE ubANDMask;
static UBYTE ubORMask;
static UBYTE ubXORMask;


/*********************************************************************
 *
 * CountCommaSeparated
 *
 * SYNOPSIS
 *		short CountCommaSeparated(char *psz)
 *
 * PURPOSE
 *		Count number of comma-separated fields contained in <psz>.
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
short CountCommaSeparated(char *psz)
{
	short cnt = 0;
	char *pszComma = psz;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CountCommaSeparated";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (psz == NULL)
		return cnt;

	if (*psz == 0)
		return cnt;

	cnt++;										/* there is at least one field */

	while ((pszComma = strstr(pszComma, ",")) != NULL)
	{
		pszComma++;
		cnt++;
	}
	return cnt;
} /* CountCommaSeparated */


/*********************************************************************
 *
 * GetParseORMask
 *
 * SYNOPSIS
 *		UBYTE GetParseORMask(void)
 *
 * PURPOSE
 *		Return current <ubORMask>.
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
UBYTE GetParseORMask(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetParseORMask";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return ubORMask;

} /* GetParseORMask */


/* ParseMask - turn mask string into <ubANDMask>, <ubORMask>, and
	<ubXORMask>. <szBits> should point to eight characters.
	If character == '0' or '1', set corresponding bit in AND mask to 0.
	If character == '1', set corresponding bit in OR mask to 1.
	If character == '~', set corresponding bit in XOR mask to 1.
	Set all other AND bits to 1, all other OR & XOR bits to 0.

	<szErrorHeader> - error header (Eg., "Line 17"),
	<fOR> - if TRUE, 0 bits are not permitted
	<fXOR> - if TRUE, ~ bits are permitted (XOR mask may be non-zero.
	Returns TRUE if successful, FALSE if error. */

BOOL ParseMask(char *szBits, BOOL fOR, BOOL fXOR, char *szErrorHeader)
{
	UBYTE ubBit = 128;
	int i;

	ubANDMask = 0xff;
	ubORMask = 0;
	ubXORMask = 0;

	if (strlen(szBits) != 8)
	{
		printf("%s: should be 8-bit field.\n", szErrorHeader);
		return FALSE;
	}

	for (i = 0; i < 8; i++)
	{
		if (szBits[i] == '0')
		{
			ubANDMask &= 0xFF - ubBit;

			if (fOR)
			{
				printf("%s: 0 bits not permitted.\n", szErrorHeader);
				return FALSE;
			}
		}
		if (szBits[i] == '1')
		{
			ubANDMask &= 0xFF - ubBit;
			ubORMask |= ubBit;
		}
		if (szBits[i] == '~')
		{
			ubXORMask |= ubBit;
			if (fXOR)
			{
				printf("%s: ~ bits not permitted.\n", szErrorHeader);
				return FALSE;
			}
		}
		ubBit >>= 1;
	}
	return TRUE;
}

/*********************************************************************
 *
 * ProcessVariableDefaults
 *
 * SYNOPSIS
 *		static BOOL ProcessVariableDefaults(IniList *pIniList,
 *					char *szSection, LinkCharVariables *lcv, short wCnt)
 *
 * PURPOSE
 *		Process initializations for variables <lcv> in sections <szSection>.
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
static BOOL ProcessVariableDefaults(IniList *pIniList,
						char *szSection, LinkCharVariables *lcv, short wCnt)
{

	SectionTracker st;
	short i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessVariableDefaults";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	for (i = 0; i < wCntMax; i++)
		w[i] = 0;

	if (FindSection(&st, pIniList, szSection) != NULL)
	{
		short i, w;
		char *sz;

		for (i = 0; i < wCnt; i++)
		{
#if NEWTUME
			do
			{
#endif
				if ((sz = FindNextINILine(&st, lcv[i].sz)) != NULL)
				{
#if NEWTUME
					short idx;

					if (lcv[i].fType & fArray)
					{
						if (sscanf(sz, "%d", &idx) != 1)
						{
							printf("%s can't parse '%s'.\n", lcv[i].sz, sz);
							return FALSE;
						}
						sz = strchr(sz, '=');
						if (!sz)
						{
							printf("%s: '=' not found.\n", lcv[i].sz);
							return FALSE;
						}
						sz += 1;
					}

					if ((lcv[i].fType & maskType) == fUBMask)
					{
						if (ParseMask(sz, NOZEROBITS, NOXORBITS, szSection))
						{
							*(UBYTE *) lcv[i].pVar = ubORMask;
						}
						else
							return FALSE;
					}
					else if ((lcv[i].fType & maskType) == fWORD)
					{
						if (sscanf(sz, "%d", &w) != 1)
						{
							printf("%s can't parse '%s'.\n", lcv[i].sz, sz);
							return FALSE;
						}
						if (w != 0)
						{
							if (lcv[i].fType & fIndirect)
							{
								if (lcv[i].fType & fArray)
								{
									*((*((short **) lcv[i].pVar)) + idx) = w;
								}
								else
								{
									*((*((short **) lcv[i].pVar)) + (lcv[i].fType & maskIndex)) = w;
								}
							}
							else
							{
								*(short *) lcv[i].pVar = w;
							}
						}
					}
					else if ((lcv[i].fType & maskType) == fSZ)
					{
						strcpy((char *) lcv[i].pVar, sz);
					}
					else if ((lcv[i].fType & maskType) == fCEN)
					{
						long l, ldec;
						char c1 = 0;
						char c2 = 0;

						if (sscanf(sz, "%ld.%c%c", &l, &c1, &c2) < 1)
						{
							printf("%s can't parse '%s'.\n", lcv[i].sz, sz);
							return FALSE;
						}
						ldec = (c1 ? c1 - '0' : 0) * 10 + (c2 ? c2 - '0' : 0);

						l = l * 100L + ldec;
						if (lcv[i].fType & fIndirect)
						{
							if (lcv[i].fType & fArray)
							{
								*((*((long **) lcv[i].pVar)) + idx) = l;
							}
							else
							{
								*((*((long **) lcv[i].pVar)) + (lcv[i].fType & maskIndex)) = l;
							}
						}
					}
#else
					if (lcv[i].fType == fUBMask)
					{
						if (ParseMask(sz, NOZEROBITS, NOXORBITS, szSection))
						{
							*(UBYTE *) lcv[i].pVar = ubORMask;
						}
						else
							return FALSE;
					}
					else if (lcv[i].fType == fWORD)
					{
						if (sscanf(sz, "%d", &w) != 1)
						{
							printf("%s can't parse '%s'.\n", lcv[i].sz, sz);
							return FALSE;
						}
						if (w > 0)
							*(short *) lcv[i].pVar = w;
					}
					else if (lcv[i].fType == fSZ)
					{
						strcpy((char *) lcv[i].pVar, sz);
					}
#endif
				}
#if NEWTUME
				else
					break;
			} while (lcv[i].fType & fArray);
#endif
			ResetSection(&st);
		}
	}
	return TRUE;
} /* ProcessVariableDefaults */


/* ProcessEachColorMask - parse color mask and save it away. */

static BOOL ProcessEachColorMask(ColorGroupMask *pcgm, char *szBits)
{
	UBYTE ubTileANDMask;
	UBYTE ubTileORMask;

	if (szBits[8] == ':')
	{
		szBits[8] = 0;

		if (!ParseMask(szBits, ZEROBITS, NOXORBITS, szctmErr))
			return FALSE;

		ubTileANDMask = ubANDMask;
		ubTileORMask = ubORMask;

		szBits[8] = ':';

		if (!ParseMask(szBits+9, ZEROBITS, NOXORBITS, szcpmErr))
			return FALSE;

		SetColorMasks(pcgm, ubTileANDMask, ubTileORMask,
							ubANDMask, ubORMask);
	}
	else
	{
		printf("Color Masks: can't parse '%s'.\n", szBits);
		return FALSE;
	}
	return TRUE;
}

/* ProcessEveryColorMask - read and process individual color mask info
	from INI file. */

static BOOL ProcessEveryColorMask(ColorGroupMask *pcgm, SectionTracker *pst)
{
	ConfigLine *pLine;

	while ((pLine = GetNextINILine(pst)) != NULL)
	{
		if (strncmp(szConfigLine(pLine), szEnable, wLenSzEnable) == 0)
		{
			char *sz;

			sz = szConfigLine(pLine) + wLenSzEnable;

			if (ParseMask(sz, NOZEROBITS, NOXORBITS, szcmeErr))
				SetColorEnableANDMask(pcgm, ubORMask);
			else
				return FALSE;
		}
		else if (strncmp(szConfigLine(pLine), szConfig, wLenSzConfig) == 0)
		{
			char *sz;

			sz = szConfigLine(pLine) + wLenSzConfig;

			if (ParseMask(sz, NOZEROBITS, NOXORBITS, szcmcErr))
				SetColorConfigANDMask(pcgm, ubORMask);
			else
				return FALSE;
		}
		else
		{
			if (!ProcessEachColorMask(pcgm, szConfigLine(pLine)))
				return FALSE;
		}
	}
	return TRUE;
}

/* ProcessColorMaskGroups - read and process color mask info from INI file. */

static BOOL ProcessColorMaskGroups(IniList *pIniList)
{
	if (pIniList)
	{
		SectionTracker st;

		if (FindSection(&st, pIniList, szcmg) != NULL)
		{
			ColorGroupMask *pcgm;
			ConfigLine *pLine;

			pcgm = FirstColorGroup();

			while ((pLine = GetNextINILine(&st)) != NULL)
			{
				SectionTracker st;
				char sz[256];

				sz[0] = '[';
				strcpy(sz+1, szConfigLine(pLine));
				strcat(sz, "]");

				if (IsEOList(pcgm))
				{
					printf("Color Masks: extra group?!\n");
					return FALSE;
				}

				if (FindSection(&st, pIniList, sz) != NULL)
				{
					if (!ProcessEveryColorMask(pcgm, &st))
						return FALSE;
				}
				else
				{
					printf("Color Masks: can't find %s.\n", sz);
					return FALSE;
				}
				pcgm = (ColorGroupMask *) Next(pcgm);
			}
		}
	}
	return TRUE;
}


/*********************************************************************
 *
 * ProcessCountCharacterGroups
 *
 * SYNOPSIS
 *		static BOOL ProcessCountCharacterGroups(IniList *pIniList, short wMax)
 *
 * PURPOSE
 *		Process every line in every count character group.
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
static BOOL ProcessCountCharacterGroups(IniList *pIniList, short wMax)
{

	short w;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessCountCharacterGroups";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pIniList)
	{
		SectionTracker st;

		if (FindSection(&st, pIniList, szcg) != NULL)
		{
			for (w = 0; w < wMax; w++)
			{
				char *szEQ;
				char szG[256];
				SectionTracker st2;
				ConfigLine *pLine;

				pLine = GetNextINILine(&st);

				szG[0] = '[';
				strcpy(&szG[1], szConfigLine(pLine));
				szEQ = strstr(szG, "=");
				*szEQ++ = ']';
				*szEQ++ = 0;

				if (FindSection(&st2, pIniList, szG) != NULL)
				{
					GroupSet *pGS;
					ConfigLine *pLine;

					pGS = (GroupSet *) Head(&GroupDef[w]);

					while (!IsEOList(pGS))
					{
						char *sz, *szT, *szR;

						pLine = GetNextINILine(&st2);

						szT = szConfigLine(pLine);
						if ((szR = strstr(szT, ",")) == NULL)
						{
							printf("count char: %s: bad form.\n", szT);
							return FALSE;
						}
						*szR++ = 0;

						if ((szT = strpbrk(szT, szDigits)) != NULL)
						{
							pGS->uwTileLow = (UWORD) (strtoul(szT, &sz, 10));
							if ((sz = strstr(sz, "..")) != NULL)
							{
								pGS->uwTileHigh = (UWORD) (strtoul(strpbrk(sz, szDigits), &sz, 10));
							}
							else
							{
								pGS->uwTileHigh = pGS->uwTileLow;
							}
						}
						else
						{
							printf("count char: %s: bad form.\n", szConfigLine(pLine));
							return FALSE;
						}
						if ((szR = strpbrk(szR, szDigits)) != NULL)
						{
							pGS->uwRoomLow = (UWORD) (strtoul(szR, &sz, 10));
							if ((sz = strstr(sz, "..")) != NULL)
							{
								pGS->uwRoomHigh = (UWORD) (strtoul(strpbrk(sz, szDigits), &sz, 10));
							}
							else
							{
								pGS->uwRoomHigh = pGS->uwRoomLow;
							}
							pGS->fRoom = TRUE;
						}
						else
						{
							pGS->fRoom = FALSE;
						}
						pGS = (GroupSet *) Next(pGS);
					}
				}
			}
		}
	}
	return TRUE;
} /* ProcessCountCharacterGroups */


/* CountLinesInSection - count number of lines in section <sz>. */

static short CountLinesInSection(IniList *pIniList, char *sz)
{
	if (pIniList)
	{
		SectionTracker st;

		if (FindSection(&st, pIniList, sz) != NULL)
		{
			return CountINILines(&st);
		}
	}
	return 0;
}


/*********************************************************************
 *
 * CountCharacterCountGroup
 *
 * SYNOPSIS
 *		static short CountCharacterCountGroup(IniList *pIniList, char *sz)
 *
 * PURPOSE
 *		Process flags for each character count group (that is,
 *		save flags in bGroupFlags[]), and counter number of lines
 *		in each character count group and save count in uwCharMax[]
 *		(we are 'overloading' uwCharMax[]).
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Number of groups, or -1 if error.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static short CountCharacterCountGroup(IniList *pIniList, char *sz)
{

	short wLines = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CountCharacterCountGroup";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pIniList)
	{
		SectionTracker st;

		if (FindSection(&st, pIniList, sz) != NULL)
		{
			ConfigLine *pLine;

			while ((pLine = GetNextINILine(&st)) != NULL)
			{
				char *szEQ;
				char szG[256];

				bGroupFlags[wLines] = 0;
				szG[0] = '[';
				strcpy(&szG[1], szConfigLine(pLine));

				if ((szEQ = strstr(szG, "=")) != NULL)
				{
					short w;

					for (w = 0; w < wMergeCnt; w++)
					{
						if (strstr(szEQ, lMerge[w].sz) != NULL)
						{
							bGroupFlags[wLines] |= lMerge[w].ubFlag;
						}
					}
					*szEQ++ = ']';
					*szEQ++ = 0;

					uwCharMax[wLines] = CountLinesInSection(pIniList, szG);
					wLines++;
					if (wLines >= wGroupCntMax)
					{
						printf("Count chars: too many groups!\n");
						return -1;
					}
				}
				else
				{
					printf("Count chars: %s: bad line.\n", szConfigLine(pLine));
					return -1;
				}
			}
			return wLines;
		}
	}
	return 0;
} /* CountCharacterCountGroup */


/* CountBytesB4Equal - count number of bytes needed to store all strings
	in front of the '=' sign. */

static short CountBytesB4Equal(IniList *pIniList, char *sz)
{
	short wBytes = 0;

	if (pIniList)
	{
		SectionTracker st;

		if (FindSection(&st, pIniList, sz) != NULL)
		{
			ConfigLine *pLine;

			while ((pLine = GetNextINILine(&st)) != NULL)
			{
				char *pch;

				pch = strchr(szConfigLine(pLine), '=');

				if (pch)
				{
					wBytes += (short) (pch - szConfigLine(pLine) + 1);
				}
			}
		}
	}
	return wBytes;
}

/* ProcessEachTileMask - process each tile mask string.
	return TRUE if successful. */

static BOOL ProcessEachTileMask(EventState *pes, short wEventNum, char *sz, char *szErr)
{
	UBYTE ubTileANDMaskL;
	UBYTE ubTileORMaskL;
	UBYTE ubTileXORMaskL;

	pes->Function = TileMaskEvent;

	if (sz[8] == ',')
	{
		sz[8] = 0;

		if (!ParseMask(sz, ZEROBITS, XORBITS, szErr))
			return FALSE;

		ubTileANDMaskL = ubANDMask;
		ubTileORMaskL = ubORMask;
		ubTileXORMaskL = ubXORMask;

		sz[8] = ',';

		if (!ParseMask(sz+9, ZEROBITS, XORBITS, szErr))
			return FALSE;

		SaveMaskEventMask(wEventNum, ubTileANDMaskL, ubTileORMaskL, ubTileXORMaskL,
								ubANDMask, ubORMask, ubXORMask);
		return TRUE;
	}
	else
	{
		printf("Tile Masks: can't parse '%s'.\n", sz);
	}

	return FALSE;
}

/*********************************************************************
 *
 * ProcessEachDisplayMask
 *
 * SYNOPSIS
 *		static BOOL ProcessEachDisplayMask(EventState *pes, short wEventNum, char *sz, char *szErr)
 *
 * PURPOSE
 *		Process each tile mask display string.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL ProcessEachDisplayMask(EventState *pes, short wEventNum, char *sz, char *szErr)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessEachDisplayMask";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pes->Function = DisplayMaskEvent;

	if (ParseMask(sz, ZEROBITS, XORBITS, szErr))
	{
		SaveDisplayMask(pes, wEventNum, ubANDMask, ubORMask, ubXORMask);
		return TRUE;
	}
	return FALSE;

} /* ProcessEachDisplayMask */


/* ProcessEachCursorMovement - process each cursor movement coordinate pair.
	Returns TRUE if successful. */

static BOOL ProcessEachCursorMovement(EventState *pes, short wEventNum, char *sz, char *szErr)
{
	short wDX, wDY;

	pes->Function = MoveCursorEvent;

	if (sscanf(sz, "%d,%d", &wDX, &wDY) != 2)
	{
		printf("%s: can't parse '%s'.\n", szErr, sz);
		return FALSE;
	}

	SaveCursorMovement(wEventNum, wDX, wDY);
	return TRUE;
}


/*********************************************************************
 *
 * ProcessEachShiftLayer
 *
 * PURPOSE
 *		Process each shift layer movement coordinate pair.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *		02/20/94 Sunday (dcc) - created.
 *
*/
static BOOL ProcessEachShiftLayer(EventState *pes, short wEventNum, char *sz, char *szErr)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessEachShiftLayer";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	short wDX, wDY;

	pes->Function = ShiftLayerEvent;

	if (sscanf(sz, "%d,%d", &wDX, &wDY) != 2)
	{
		printf("%s: can't parse '%s'.\n", szErr, sz);
		return FALSE;
	}

	SaveShiftLayer(wEventNum, wDX, wDY);
	return TRUE;
} /* ProcessEachShiftLayer */


/*********************************************************************
 *
 * ProcessEachZoomEvent
 *
 * SYNOPSIS
 *		static BOOL ProcessEachZoomEvent(EventState *pes, short wEventNum, char *sz, char *szErr)
 *
 * PURPOSE
 *		Save each zoom event.
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
static BOOL ProcessEachZoomEvent(EventState *pes, short wEventNum, char *sz, char *szErr)
{

	short wDst, wSrc;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessEachZoomEvent";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pes->Function = SetZoomEvent;

	if (sscanf(sz, "%d:%d", &wDst, &wSrc) != 2)
	{
		printf("%s: can't parse '%s'.\n", szErr, sz);
		return FALSE;
	}

	SaveZoomAmount(wEventNum, wDst, wSrc);
	return TRUE;
} /* ProcessEachZoomEvent */


/* ProcessEvents - from .INI file, read and process individual
	tile mask events if wGroup == wTileMaskGroup (pFunc is set to
	ProcessEachTileMask), or read and process individual cursor
	movements if wGroup == wCursorGroup (and pFunc is set to
	ProcessEachCursorMovement). */

static BOOL ProcessEvents(short wGroup, char *sz, char *szErr,
								IniList *pIni, short wEventMax,
								BOOL (*pFunc)(EventState *pes, short wEventNum, char *sz, char *szErr))
{
	if (pIni)
	{
		SectionTracker st;
		short wCntEvent = 0;

		if (FindSection(&st, pIni, sz) != NULL)
		{
			ConfigLine *pLine;
			EventState *pes = GetFirstEvent(wGroup);

			while ((pLine = GetNextINILine(&st)) != NULL)
			{
				char *pch, *pch2;

				pch = strchr(szConfigLine(pLine), '=');

				if (pch)
				{
					if (wCntEvent >= wEventMax)
					{
						printf("%s: more events?!\n", szErr);
						return FALSE;
					}

					*pch = 0;

					if (FindEvent(szConfigLine(pLine)) != NULL)
					{
						printf("%s: duplicate event '%s'.\n", szErr, szConfigLine(pLine));
						return FALSE;
					}

					if ((pch2 = SaveEventString(wGroup, szConfigLine(pLine))) != NULL)
					{
						NodeName(pes) = pch2;
					}
					else
					{
						printf("%s: more strings?!\n", szErr);
						return FALSE;
					}
					*pch = '=';

					if (!pFunc(pes, wCntEvent, pch + 1, szErr))
						return FALSE;

					pes = (EventState *) Next(pes);
					wCntEvent++;
				}
				else
				{
					printf("%s: '=' not found.\n", szErr);
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}


/*********************************************************************
 *
 * ProcessINI
 *
 * PURPOSE
 *		ProcessINI - read and process INI file.
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
 *		08/12/93 Thursday (dcc) - modified to take <szINI> as input.
 *
*/
BOOL ProcessINI(char *szINI)
{
	IniList	*pIniList = NULL;
	short wCntCharCntGroups;
	short wCntColorGroups;
	short wCntTileMaskEvents;
	short wCntDisplayMaskEvents;
	short wSizeTileMaskStrings;
	short wSizeDisplayMaskStrings;
	short wCntCursorEvents;
	short wCntShiftLayerEvents;
	short wSizeCursorStrings;
	short wSizeShiftLayerStrings;
	short wCntZoomEvents;
	short wSizeZoomStrings;
	char szPath[EIO_MAXPATH];

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessINI";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!EIO_FindFile (szINI, NULL, szPath))
	{
		printf ("Couldn't find file '%s' in path\n", szINI);
/**/	goto ABORT;
	}

	/* Read .INI file, find out what we need to allocate, close .INI file. */
	{
		if ((pIniList = ReadINI(szPath)) == NULL)
/**/		goto ABORT;

		/* Process X Flip Bit. */

		if (!ProcessVariableDefaults(pIniList, szxflip, lXFlipBit, wXFlipBitCnt))
/**/		goto ABORT;

		/* Process Y Flip Bit. */

		if (!ProcessVariableDefaults(pIniList, szyflip, lYFlipBit, wYFlipBitCnt))
/**/		goto ABORT;

		/* Process how many tile to show across, how many tiles to show down. */

		if (!ProcessVariableDefaults(pIniList, sztd, lTileDisplay, wTileDisplayCnt))
/**/		goto ABORT;

		/* Process grid & guide defaults. */

		if (!ProcessVariableDefaults(pIniList, szrd, lRoomDefaults, wRoomDefaultsCnt))
/**/		goto ABORT;

#if NEWTUME
		Assert(fDefaultShowGuide[0] == fDefaultGuideVisible);
		Assert(fDefaultShowGuide[1] == fDefaultGuideVisible);
#endif

		/* Process character counting variables. */

		szcg[0] = '[';
		if (!ProcessVariableDefaults(pIniList, szcc, lCountChar, wCountCharCnt))
/**/		goto ABORT;
		strcat(szcg, "]");

		/* Process color requester divider lines, palette resolution. */

		if (!ProcessVariableDefaults(pIniList, szcr, lColorReq, wColorReqCnt))
/**/		goto ABORT;

		/* Process palette resolution. */
		{
			short i;

			for (i = 0; i < 3; i++)
			{
				if (ColorRange[i] > 256)
					ColorRange[i] = 256;
				ColorMax[i] = ColorRange[i]-1;
			}
		}

		/* Process load options. */

		if (!ProcessVariableDefaults(pIniList, szlo, lLoadOpt, wLoadOptCnt))
/**/		goto ABORT;

		/* Process print to file. */

		if (!ProcessVariableDefaults(pIniList, szpm, lPrint, wPrintCnt))
/**/		goto ABORT;

		/* Process download variables. */

		if (!ProcessVariableDefaults(pIniList, szdwn, lDown, wDownCnt))
/**/		goto ABORT;

#if dvpPARALLEL
		SetLPTn(w[0]);							/* set parallel port address */
#endif

		/* Process mouse sensitivity. */

		if (!ProcessVariableDefaults(pIniList, szms, lMouse, wMouseCnt))
/**/		goto ABORT;
		if (w[0] && w[1])
			SetMouseResolution(w[0], w[1]);

		/* Process file requester directory order. */

		if (!ProcessVariableDefaults(pIniList, szfr, lFileReq, wFileReqCnt))
/**/		goto ABORT;
		if (w[0])
			SetDirSortType(w[0]);

		/* Process search options. */

		if (!ProcessVariableDefaults(pIniList, szsea, lSearchOpt, wSearchOptCnt))
/**/		goto ABORT;
		else
			SetSearchMask(ub[0]);

		/* Process shift layer units. */

		if (!ProcessVariableDefaults(pIniList, szsl, lShiftUnits, wShiftUnitsCnt))
/**/		goto ABORT;
		if (!wShiftXUnit)
			wShiftXUnit = 1;
		if (!wShiftYUnit)
			wShiftYUnit = 1;

#if SaveNew
		/* Process save options. */

		if (!ProcessVariableDefaults(pIniList, szsv, lSaveOpt, wSaveOptCnt))
/**/		goto ABORT;
#endif //SaveNew

		/* Process flags for each character count group, and
			count number of lines in each character count groups */

		if ((wCntCharCntGroups = CountCharacterCountGroup(pIniList, szcg)) < 0)
/**/		goto ABORT;

		/* Count number of color mask groups */

		wCntColorGroups = CountLinesInSection(pIniList, szcmg);

		/* Count number of tile mask events */

		wCntTileMaskEvents = CountLinesInSection(pIniList, sztme);

		/* Count number of bytes needed to store all tile mask event strings. */

		wSizeTileMaskStrings = CountBytesB4Equal(pIniList, sztme);

		/* Count number of tile mask display events */

		wCntDisplayMaskEvents = CountLinesInSection(pIniList, szdme);

		/* Count number of bytes needed to store all tile mask display event strings. */

		wSizeDisplayMaskStrings = CountBytesB4Equal(pIniList, szdme);

		/* Count number of cursor movement events */

		wCntCursorEvents = CountLinesInSection(pIniList, szcme);

		/* Count number of shift layer events */

		wCntShiftLayerEvents = CountLinesInSection(pIniList, szsle);

		/* Count number of bytes needed to store all cursor movement event strings. */

		wSizeCursorStrings = CountBytesB4Equal(pIniList, szcme);

		/* Count number of bytes needed to store all shift layer event strings. */

		wSizeShiftLayerStrings = CountBytesB4Equal(pIniList, szsle);

		/* Count number of zoom events */

		wCntZoomEvents = CountLinesInSection(pIniList, szzme);

		/* Count number of bytes needed to store all zoom event strings. */

		wSizeZoomStrings = CountBytesB4Equal(pIniList, szzme);

		FreeINI(pIniList);						/* Free memory used by .INI file. */
	}

	/* Allocate stuff we need to save .INI information. */
	{
		short i;

		OpenColorGroupMask();

		/* Allocate character count list entries. */

		if (!OpenCountChars(wCntCharCntGroups))
			return FALSE;

		for (i = 0; i < wCntColorGroups; i++)/* Allocate memory for color mask groups */
		{
			if (AddNewColorGroup() == NULL)
				return FALSE;
		}

		/* Allocate mask arrays to store tile masks. */

		if (!AllocateTileMaskArrays(wCntTileMaskEvents))
			return FALSE;

		/* Add empty events to store tile mask events. */

		if (!AddEmptyEvents(wTileMaskGroup, wCntTileMaskEvents, wSizeTileMaskStrings))
			return FALSE;

		/* Allocate display mask arrays to store tile masks. */

		if (!AllocateDisplayMaskArrays(wCntDisplayMaskEvents))
			return FALSE;

		/* Add empty events to store display mask events. */

		if (!AddEmptyEvents(wDisplayMaskGroup, wCntDisplayMaskEvents, wSizeDisplayMaskStrings))
			return FALSE;

		/* Allocate cursor movement arrays. */

		if (!AllocateCursorMovements(wCntCursorEvents))
			return FALSE;

		/* Allocate shift layer arrays. */

		if (!AllocateShiftLayer(wCntShiftLayerEvents))
			return FALSE;

		/* Add empty events to store cursor movement events. */

		if (!AddEmptyEvents(wCursorGroup, wCntCursorEvents, wSizeCursorStrings))
			return FALSE;

		/* Add empty events to store shift layer events. */

		if (!AddEmptyEvents(wShiftLayerGroup, wCntShiftLayerEvents, wSizeShiftLayerStrings))
			return FALSE;

		if (wCntZoomEvents)
		{
			/* Allocate zoom arrays to store zoom events. */

			if (!AllocateZoomEventArrays(wCntZoomEvents))
				return FALSE;

			/* Add empty events to store zoom events. */

			if (!AddEmptyEvents(wZoomGroup, wCntZoomEvents, wSizeZoomStrings))
				return FALSE;
		}
		else
		{
			/* Allocate zoom arrays to store zoom events. */

			if (!AllocateZoomEventArrays(1))
				return FALSE;

			/* Add empty events to store zoom events. */

			if (!AddEmptyEvents(wZoomGroup, 1, wSizeZoomStrings))
				return FALSE;
		}
	}

	/* Open .INI, process and save all information, close .INI file. */
	{
		pIniList = ReadINI(szPath);

		if (!InitTypeLists(pIniList))
/**/		goto ABORT;

#if PixelLayers
		if (!InitObjectTypeList(pIniList))
/**/		goto ABORT;
#endif // PixelLayers

		if (!ProcessColorMaskGroups(pIniList))
/**/		goto ABORT;

		if (!ProcessEvents(wTileMaskGroup, sztme, sztmeErr,
								pIniList, wCntTileMaskEvents,
								ProcessEachTileMask))
/**/		goto ABORT;

		if (!ProcessEvents(wDisplayMaskGroup, szdme, szdmeErr,
								pIniList, wCntDisplayMaskEvents,
								ProcessEachDisplayMask))
/**/		goto ABORT;

		if (!ProcessEvents(wCursorGroup, szcme, szcmvErr,
								pIniList, wCntCursorEvents,
								ProcessEachCursorMovement))
/**/		goto ABORT;

		if (!ProcessEvents(wShiftLayerGroup, szsle, szsleErr,
								pIniList, wCntCursorEvents,
								ProcessEachShiftLayer))
/**/		goto ABORT;

		if (wCntZoomEvents)
		{
			if (!ProcessEvents(wZoomGroup, szzme, szzmeErr,
									pIniList, wCntZoomEvents,
									ProcessEachZoomEvent))
/**/			goto ABORT;
		}
		else
		{
			SaveZoomAmount(0, 1, 1);
		}

		if (!ProcessCountCharacterGroups(pIniList, wCntCharCntGroups))
/**/		goto ABORT;

		if (!ProcessSearchDirs(pIniList, szlo, szsd))
			goto ABORT;

		FreeINI(pIniList);
	}

	return TRUE;

ABORT:
	FreeINI(pIniList);

	return FALSE;
} /* ProcessINI */


/*********************************************************************
 *
 * ProcessInitialINIEvents
 *
 * PURPOSE
 *		Open INI file (again), and process initial events found therein.
 *
 * INPUT
 *
 *
 * ASSUMES
 *		Function assumes that it can find <szINI> in the path.
 *		This is verified to be TRUE if you've already called ProcessINI().
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *		08/12/93 Thursday (dcc) - modified to take <szINI> as input.
 *
 * SEE ALSO
 *
*/
BOOL ProcessInitialINIEvents(char *szINI)
{

	IniList	*pIniList;
	SectionTracker st;
	ConfigLine *pLine;
	char szPath[EIO_MAXPATH];

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ProcessInitialINIEvents";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	EIO_FindFile (szINI, NULL, szPath);
	if ((pIniList = ReadINI(szPath)) == NULL)
		return FALSE;

	if (FindSection(&st, pIniList, szinit) != NULL)
	{
		while ((pLine = GetNextINILine(&st)) != NULL)
		{
			EventState *pevt;

			if ((pevt = FindEvent(szConfigLine(pLine))) == NULL)
			{
				sprintf(szPath, "Unknown initial event: %s", szConfigLine(pLine));
				Quit(szPath);
			}
			TriggerEvent(pevt, 0, NULL);
		}
	}

	FreeINI(pIniList);
	return TRUE;

} /* ProcessInitialINIEvents */

