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
 * SPARSE.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 08/17/94
 *   MODIFIED : 04/03/95
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Routines to deal with sparse array representation of layers.
 *		This is DEFINITELY gross code to deal with 16-bit MS-DOS.
 *
 * HISTORY
 *		08/17/94 (dcc) - Created.
 *		10/12/94 (dcc) - Changed coordinate representation from UWORDs to int's.
 *		10/20/94 (dcc) - Changed definition of sparse array so that psaeLastRow
 *							  and psaeLastCol are associated with each sparse array.
 *		03/05/95 (dcc) - Fix GetNextSparsePlotInRange() not always leaving
 *							  pspa->psaeLastRow in a known state.
 *		04/03/95 (dcc) - Fixed DeleteSparseArrayRows() invalidating psaeLastRow.
 *		04/03/95 (dcc) - Fixed ZeroRectInSparseArray() invalidating psaeLastCol.
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#if UseSparseArray

#include <echidna/etypes.h>
#include <echidna/listfunc.h>
#include <echidna/xtramem.h>

#include <alloc.h>

#if DEBUG
#include <stdio.h>
#endif

#include "assert.h"
#include "sparse.h"
#include "tuglbl.h"
#include "sparsef.h"							/* Verify function prototypes. */

/**************************** C O N S T A N T S ***************************/

#define EndOfRow	0xFFFFU
#define EndOfCol	0xFFFFU

#define BankofBanksMax	256				// must fit in 8 bits
#define BankPtrMax		256				// must fit in 8 bits
#define BankOfSparse	 16384				// must be AllocXTRA()able

/******************************** T Y P E S *******************************/

/*
For each two-dimensional sparse array:
1. There is a linked list (of SparseRowType) of pointers to the
   first entry in each sparse row.
2. Each sparse row is a linked list (from left to right) of SparsePlotType's.

Notes:
1. Current implementation assumes sizeof(SparsePlotType) == sizeof(SparseRowType)
   (This means that pointers must be 32-bits in size)
*/

typedef struct
{
	XTRAPntr	pBank[BankPtrMax];
} rgpBank;

typedef rgpBank *prgpBank;

/****************************** G L O B A L S *****************************/

static prgpBank huge32	*pBankofBanks;

static PSAE					psaeFree;			// address of 1st free sparse array element
static UWORD				iBankofBanksFree;	// address of 1st free bank of banks
static UWORD				iBanksFree;			// address of 1st free bank

static PlotType			pltNull = {0, 0, 0};

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * InitSparse
 *
 * PURPOSE
 *		Initialize the sparse array routines.
 *
 * INPUT
 *		NONE.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *		08/17/94 Wednesday (dcc) - Created.
 *
*/
BOOL InitSparse(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitSparse";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((pBankofBanks = MEM_calloc(BankofBanksMax, sizeof(prgpBank))) == NULL)
		return FALSE;

	if ((pBankofBanks[0] = MEM_calloc(1, sizeof(rgpBank))) == NULL)
		return FALSE;

	psaeFree.iBank = EndOfCol;
	iBankofBanksFree = 0;
	iBanksFree = 0;

	return TRUE;
} /* InitSparse */


/*********************************************************************
 *
 * ReadSparse
 *
 * PURPOSE
 *		Read the sparse array element pointed to by <pspa> into <pspr>.
 *
 * INPUT
 *		PSAE psae			: pointer to sparse element to read
 *		void *pspr			: location to store sparse array element
 *
 * ASSUMES
 *		This is a very-low level function that assumes everything is OK.
 *
 * RETURN VALUE
 *		
 *
 * HISTORY
 *		08/17/94 Wednesday (dcc) - Created.
 *
*/
static void ReadSparse(PSAE psae, void *pspr)
{
	prgpBank	prgBank;
	UWORD iBankofBanks	= psae.irgpBank >> 8;
	UWORD iBanks			= psae.irgpBank & 0xFF;
	XTRAPntr xp;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReadSparse";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	prgBank = pBankofBanks[iBankofBanks];
	Assert(prgBank != NULL);
	if (prgBank == NULL)
		return;									// This is an error!

	Assert(psae.iBank != EndOfCol);
	xp = prgBank->pBank[iBanks];
	Assert(xp != NULL);
	if (xp == NULL)
		return;									// This is also an error!

	ReadXTRA(xp, psae.iBank * sizeof(SparsePlotType), sizeof(SparsePlotType), pspr);

} /* ReadSparse */


/*********************************************************************
 *
 * WriteSparse
 *
 * PURPOSE
 *		Write the sparse array element <pspr> into <psae>.
 *
 * INPUT
 *		void *pspr			: sparse array element to store
 *		PSAE psae			: pointer to sparse element to write
 *
 * ASSUMES
 *		This is a very-low level function that assumes everything is OK.
 *
 * RETURN VALUE
 *		
 *
 * HISTORY
 *		08/18/94 Thursday (dcc) - Created.
 *
*/
static void WriteSparse(void *pspr, PSAE psae)
{
	prgpBank	prgBank;
	UWORD iBankofBanks	= psae.irgpBank >> 8;
	UWORD iBanks			= psae.irgpBank & 0xFF;
	XTRAPntr xp;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteSparse";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	prgBank = pBankofBanks[iBankofBanks];
	Assert(prgBank != NULL);
	if (prgBank == NULL)
		return;									// This is an error!

	Assert(psae.iBank != EndOfCol);
	xp = prgBank->pBank[iBanks];
	Assert(xp != NULL);
	if (xp == NULL)
		return;									// This is also an error!

	WriteXTRA(pspr, sizeof(SparsePlotType), xp, psae.iBank * sizeof(SparsePlotType));

} /* WriteSparse */


#if DEBUG
/*********************************************************************
 *
 * FGoodSparseArray
 *
 * PURPOSE
 *		Walk the sparse array, make sure it is still "good".
 *		(That is, rows are still in increasing order, and columns
 *		are still in increasing order within a row).
 *
 * INPUT
 *		SparseArray *pspa	: sparse array to verify
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		FALSE if failed to verify.
 *
 * HISTORY
 *		03/27/95 (dcc) - Created.
 *
*/
static BOOL FGoodSparseArray(SparseArray *pspa)
{
	SparseRowType spr;
	int dtilyLast = -1;
	BOOL fOnLastRow;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FGoodSparseArray";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Guaranteed to have at least one row */

	Assert(pspa->psaeFirstRow.iBank != EndOfRow);

	/* Is next row same as pspa->psaeLastRow? */

	if (pspa->psaeFirstRow.irgpBank	== pspa->psaeLastRow.irgpBank &&
		 pspa->psaeFirstRow.iBank		== pspa->psaeLastRow.iBank)
	{
		ReadSparse(pspa->psaeFirstRow, &spr);

		Assert(spr.NextRow.irgpBank	== pspa->sprLast.NextRow.irgpBank);
		Assert(spr.NextRow.iBank		== pspa->sprLast.NextRow.iBank);
		Assert(spr.dtily					== pspa->sprLast.dtily);
		Assert(spr.FirstCol.irgpBank	== pspa->sprLast.FirstCol.irgpBank);
		Assert(spr.FirstCol.iBank		== pspa->sprLast.FirstCol.iBank);
		fOnLastRow = TRUE;
	}
	else
	{
		ReadSparse(pspa->psaeFirstRow, &spr);
		fOnLastRow = FALSE;
	}

	while (1)
	{
		SparsePlotType spl;
		int dtilxLast = -1;

		/* Each row should be greater than last row */

		Assert(spr.dtily > dtilyLast);

		/* Guaranteed to have at least one column */

		Assert(spr.FirstCol.iBank != EndOfCol);

		/* Is next column same as pspa->psaeLastCol? */

		if (spr.FirstCol.irgpBank	== pspa->psaeLastCol.irgpBank &&
			 spr.FirstCol.iBank		== pspa->psaeLastCol.iBank)
		{
			/* psaeLastRow must point to list with current column! */

			Assert(fOnLastRow);

			ReadSparse(spr.FirstCol, &spl);

			Assert(spl.plt.Plot_Flags	== pspa->splLast.plt.Plot_Flags);
			Assert(spl.plt.TileSet_ID	== pspa->splLast.plt.TileSet_ID);
			Assert(spl.plt.Tile_ID		== pspa->splLast.plt.Tile_ID);
			Assert(spl.dtilx				== pspa->splLast.dtilx);
			Assert(spl.NextCol.irgpBank== pspa->splLast.NextCol.irgpBank);
			Assert(spl.NextCol.iBank	== pspa->splLast.NextCol.iBank);

		}
		else
		{
			ReadSparse(spr.FirstCol, &spl);
		}

		while (1)
		{
			/* Each column should be greater than last column */

			Assert (spl.dtilx > dtilxLast);

			dtilxLast = spl.dtilx;

			if (spl.NextCol.iBank == EndOfCol)
				break;

			if (spl.NextCol.irgpBank	== pspa->psaeLastCol.irgpBank &&
				 spl.NextCol.iBank		== pspa->psaeLastCol.iBank)
			{
				/* psaeLastRow must point to list with current column! */

				Assert(fOnLastRow);

				ReadSparse(spl.NextCol, &spl);

				Assert(spl.plt.Plot_Flags	== pspa->splLast.plt.Plot_Flags);
				Assert(spl.plt.TileSet_ID	== pspa->splLast.plt.TileSet_ID);
				Assert(spl.plt.Tile_ID		== pspa->splLast.plt.Tile_ID);
				Assert(spl.dtilx				== pspa->splLast.dtilx);
				Assert(spl.NextCol.irgpBank== pspa->splLast.NextCol.irgpBank);
				Assert(spl.NextCol.iBank	== pspa->splLast.NextCol.iBank);

			}
			else
			{
				ReadSparse(spl.NextCol, &spl);
			}

		}

		dtilyLast = spr.dtily;

		if (spr.NextRow.iBank == EndOfRow)
			break;

		/* Is next row same as pspa->psaeLastRow? */

		if (spr.NextRow.irgpBank	== pspa->psaeLastRow.irgpBank &&
			 spr.NextRow.iBank		== pspa->psaeLastRow.iBank)
		{
			ReadSparse(spr.NextRow, &spr);

			Assert(spr.NextRow.irgpBank	== pspa->sprLast.NextRow.irgpBank);
			Assert(spr.NextRow.iBank		== pspa->sprLast.NextRow.iBank);
			Assert(spr.dtily					== pspa->sprLast.dtily);
			Assert(spr.FirstCol.irgpBank	== pspa->sprLast.FirstCol.irgpBank);
			Assert(spr.FirstCol.iBank		== pspa->sprLast.FirstCol.iBank);
			fOnLastRow = TRUE;
		}
		else
		{
			ReadSparse(spr.NextRow, &spr);
			fOnLastRow = FALSE;
		}
	}

	return TRUE;
} /* FGoodSparseArray */
#endif


/*********************************************************************
 *
 * AddNewBank
 *
 * PURPOSE
 *		Add a new 16K bank of sparse array elements.
 *
 * INPUT
 *		NONE.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		-> to a new list of free elements, or .iBank == EndOfCol if OOM.
 *
 * HISTORY
 *		08/18/94 Thursday (dcc) - Created.
 *
*/
static PSAE AddNewBank(void)
{
	PSAE psae;
	SparsePlotType spl;
	prgpBank	prgBank;
	int i;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddNewBank";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (iBanksFree > BankPtrMax-1)
	{
		if (iBankofBanksFree >= BankofBanksMax-1)
			goto ABORT;							// can't represent anymore

		if ((pBankofBanks[iBankofBanksFree+1] = MEM_calloc(1, sizeof(rgpBank))) == NULL)
			goto ABORT;							// out of memory

		iBanksFree = 0;
		iBankofBanksFree++;
	}
	prgBank = pBankofBanks[iBankofBanksFree];
	prgBank->pBank[iBanksFree] = AllocXTRA(BankOfSparse);

	spl.plt.Plot_Flags	= 0;
	spl.plt.TileSet_ID	= 0;
	spl.plt.Tile_ID		= 0;
	spl.dtilx				= 0;

	psae.irgpBank = (iBankofBanksFree << 8) | iBanksFree;

	for (i = 0; i < BankOfSparse / sizeof(SparsePlotType) - 1; i++)
	{
		psae.iBank = i;
		spl.NextCol.irgpBank	= psae.irgpBank;
		spl.NextCol.iBank		= i+1;
		WriteSparse(&spl, psae);
	}
	psae.iBank = i;
	spl.NextCol.irgpBank	= psae.irgpBank;
	spl.NextCol.iBank		= EndOfCol;
	WriteSparse(&spl, psae);

	psae.iBank = 0;
	iBanksFree++;
	return psae;
ABORT:
	psae.iBank = EndOfCol;
	return psae;

} /* AddNewBank */


/*********************************************************************
 *
 * GetFreeSparseArray
 *
 * PURPOSE
 *		Return a free sparse array element from the free list.
 *
 * INPUT
 *		NONE.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		-> to free sparse array element, or .iBank == EndOfRow if OOM.
 *
 * HISTORY
 *		08/18/94 Thursday (dcc) - Created.
 *
*/
static PSAE GetFreeSparseArray(void)
{
	PSAE psae;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetFreeSparseArray";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (psaeFree.iBank == EndOfCol)
	{
		psaeFree = AddNewBank();
		if (psaeFree.iBank == EndOfCol)
		{
			return psaeFree;
		}
	}
	{
		SparsePlotType spl;

		psae = psaeFree;
		ReadSparse(psae, &spl);
		psaeFree = spl.NextCol;
	}

	return psae;
} /* GetFreeSparseArray */


/*********************************************************************
 *
 * AddSparseArrayToFree
 *
 * PURPOSE
 *		Return freed sparse array element back to the free list.
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
 *		08/18/94 Thursday (dcc) - Created.
 *
*/
static void AddSparseArrayToFree(PSAE psae)
{
	SparsePlotType spl;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddSparseArrayToFree";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ReadSparse(psae, &spl);
	spl.NextCol = psaeFree;
	WriteSparse(&spl, psae);
	psaeFree = psae;

} /* AddSparseArrayToFree */


/*********************************************************************
 *
 * AddNewCol
 *
 * PURPOSE
 *		Add a new col of <dtilx> after the current col <splLast>.
 *
 * INPUT
 *		SparseArray *pspa	: sparse array to update psaeLastCol
 *		int dtilx			: x-coordinate of new col to add
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		TRUE if successful, FALSE if OOM.
 *
 * HISTORY
 *		08/18/94 Thursday (dcc) - Created.
 *
*/
static BOOL AddNewCol(SparseArray *pspa, int dtilx)
{

	PSAE psae, psaeNext;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddNewCol";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Assert(pspa->splLast.dtilx < dtilx);
	Assert(FGoodSparseArray(pspa));

	psae = GetFreeSparseArray();			// get new column marker
	if (psae.iBank == EndOfCol)
		return FALSE;							// out of memory!

	psaeNext = pspa->splLast.NextCol;
	pspa->splLast.NextCol = psae;					// make prev col point to new col
	WriteSparse(&pspa->splLast, pspa->psaeLastCol);	// save prev col

	pspa->psaeLastCol = psae;				// point to new col marker

	/* Fill out <pspa->splLast>. */

	pspa->splLast.plt.Plot_Flags	= 0;
	pspa->splLast.plt.TileSet_ID	= 0;
	pspa->splLast.plt.Tile_ID		= 0;
	pspa->splLast.dtilx				= dtilx;
	pspa->splLast.NextCol			= psaeNext;	// point to cols after this new one

	/* Write <pspa->splLast> (new column). */

	WriteSparse(&pspa->splLast, pspa->psaeLastCol);
	Assert(FGoodSparseArray(pspa));

	return TRUE;
} /* AddNewCol */


/*********************************************************************
 *
 * AddNewRow
 *
 * PURPOSE
 *		Add a new row of <dtily> after the current row <sprLast>.
 *
 * INPUT
 *		PSAE *hspaLastRow				: points to previous row
 *		SparseRowType *psprLast		: -> previous row marker
 *		int dtily						: y-coordinate of new row to add
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		TRUE if successful, FALSE if OOM.
 *
 *		PSAE *hspaLastRow				: updated to point to new "previous" row
 *		PSAE *hspaLastCol				: updated to point to new column 0 for row
 *		SparsePlotType *psplLast	: updated with new column 0 for new row
 *
 * HISTORY
 *		08/18/94 (dcc) - Created.
 *		09/07/94 (dcc) - Added <hspaLastRow>, <psprLast>, <hspaLastCol>.
 *		03/28/95 (dcc) - Added <psplLast>.
 *
*/
static BOOL AddNewRow(	PSAE *hspaLastRow, SparseRowType *psprLast,
								int dtily,
								PSAE *hspaLastCol, SparsePlotType *psplLast)
{
	PSAE psae, psaeNext;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "AddNewRow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Assert(psprLast->dtily < dtily);

	psae = GetFreeSparseArray();			// get new row marker
	if (psae.iBank == EndOfRow)
		return FALSE;							// out of memory!

	psaeNext = psprLast->NextRow;
	psprLast->NextRow = psae;				// make prev row point to new row
	WriteSparse(psprLast, *hspaLastRow);	// save prev row

	*hspaLastRow = psae;						// point to new row marker
	psprLast->NextRow = psaeNext;			// point to rows after this new one
	psprLast->dtily = dtily;				// mark y tiles position of new row

	psae = GetFreeSparseArray();			// get new column marker
	if (psae.iBank == EndOfCol)
		return FALSE;							// out of memory!

	psprLast->FirstCol = psae;				// make row -> to 1st column
	WriteSparse(psprLast, *hspaLastRow);// save new row

	*hspaLastCol = psae;						// point to new col marker

	/* Fill out <psplLast>. */

	psplLast->plt.Plot_Flags	= 0;
	psplLast->plt.TileSet_ID	= 0;
	psplLast->plt.Tile_ID		= 0;
	psplLast->dtilx				= 0;
	psplLast->NextCol.iBank	= EndOfCol;	// no more col after this new one

	/* Write <psplLast> (new column). */

	WriteSparse(psplLast, *hspaLastCol);

	return TRUE;
} /* AddNewRow */


/*********************************************************************
 *
 * MakeNewSparseArray
 *
 * PURPOSE
 *		Create a new sparse array.
 *
 * INPUT
 *		NONE.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Pointer to new SparseArray if successful, NULL if OOM.
 *
 * HISTORY
 *		08/18/94 Thursday (dcc) - Created.
 *
*/
SparseArray *MakeNewSparseArray(void)
{
	SparseArray *pspa;
	SparseRowType spr;
	SparsePlotType spl;
	PSAE psae;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MakeNewSparseArray";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((pspa = MEM_calloc(1, sizeof(SparseArray))) == NULL)
		return NULL;							// out of memory!

	pspa->psaeFirstRow = GetFreeSparseArray(); // get new row marker
	if (pspa->psaeFirstRow.iBank == EndOfRow)
		return NULL;							// out of memory!

	/* Fill out <spr>. */

	spr.NextRow.iBank = EndOfRow;			// no more rows after this one
	spr.dtily = 0;								// mark y tiles position of new row

	psae = GetFreeSparseArray();			// get new column marker
	if (psae.iBank == EndOfCol)
		return NULL;							// out of memory!

	spr.FirstCol = psae;						// make row -> to 1st column

	/* Save <spr>. */

	WriteSparse(&spr, pspa->psaeFirstRow);	// save new row

	/* Fill out <spl>. */

	spl.plt.Plot_Flags	= 0;
	spl.plt.TileSet_ID	= 0;
	spl.plt.Tile_ID		= 0;
	spl.dtilx				= 0;
	spl.NextCol.iBank		= EndOfCol;		// no more col after this new one

	/* Save <spl>. */

	WriteSparse(&spl, psae);

	/* Initialize psaeLastRow, pspa->psaeLastCol. */

	pspa->psaeLastRow.iBank = EndOfRow;
	pspa->psaeLastCol.iBank = EndOfCol;

	Assert(FGoodSparseArray(pspa));
	return pspa;
} /* MakeNewSparseArray */


/*********************************************************************
 *
 * SetSparseLayerLimits
 *
 * PURPOSE
 *		Set room limits for subsequent calls to GetNextSparsePlotInRange().
 *
 * INPUT
 *		SparseArray *pspa	: sparse array to check
 *		int dtilx			: left edge of tiles to include
 *		int dtily			: top edge of tiles to include
 *		int ctilx			: width of tiles to include (if 0, all x values)
 *		int ctily			: height of tiles to include (if 0, all y values)
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/25/94 (dcc) - Created.
 *
*/
void SetSparseLayerLimits(SparseArray *pspa,
							int dtilx, int dtily, int ctilx, int ctily)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetSparseLayerLimits";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Assert(pspa != NULL);
	Assert(FGoodSparseArray(pspa));

	pspa->psaeLimitRow			= pspa->psaeFirstRow;
	pspa->psaeLimitCol.iBank	= EndOfCol;

	pspa->dtilxLimit	= dtilx;
	pspa->dtilyLimit	= dtily;
	pspa->ctilxLimit	= ctilx;
	pspa->ctilyLimit	= ctily;

} /* SetSparseLayerLimits */


/*********************************************************************
 *
 * GetNextSparsePlotInRange
 *
 * PURPOSE
 *		Return the next plot that is in the x-range <dtilx>..<dtilx+ctilx-1>
 *		and in the y-range <dtily>..<dtily+ctily-1>.
 *
 * INPUT
 *		SparseArray *pspa	: sparse array to get plot from
 *
 * ASSUMES
 *		Assumes no tiles are inserted or deleted between calls to 
 *		GetNextSparsePlotInRange().
 *
 * RETURN VALUE
 *		Pointer to plot, or NULL if there are no more plots in range.
 *		int *pdtilx			: filled with x-coordinate of plot
 *		int *pdtily			: filled with y-coordinate of plot
 *
 * HISTORY
 *		08/25/94 (dcc) - Created.
 *		10/20/94 (dcc) - Changed to update psaeLastRow, psaeLastCol,
 *							  sprLast and splLast.
 *		03/05/95 (dcc) - Set pspa->psaeLastCol.iBank to EndOfCol when we
 *							  change pspa->psaeLastRow.
 *
*/
PlotType *GetNextSparsePlotInRange(SparseArray *pspa, int *pdtilx, int *pdtily)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetNextSparsePlotInRange";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Assert(FGoodSparseArray(pspa));

	do
	{
		do
		{
			if (pspa->psaeLimitCol.iBank == EndOfCol)
			{
				do
				{
					if (pspa->psaeLimitRow.iBank == EndOfRow)
					{
						*pdtilx = 0;
						*pdtily = 0;
						return NULL;					// Hey! we're done!
					}

					pspa->psaeLastCol.iBank = EndOfCol;	// 03/05/95 // mark pspa->splLast as not valid
					pspa->psaeLastRow = pspa->psaeLimitRow;
					ReadSparse(pspa->psaeLastRow, &pspa->sprLast);
					pspa->psaeLimitRow = pspa->sprLast.NextRow;
					pspa->psaeLimitCol = pspa->sprLast.FirstCol;
				}
				while (pspa->ctilyLimit != 0 && pspa->sprLast.dtily < pspa->dtilyLimit);

				if (pspa->ctilyLimit != 0 && pspa->sprLast.dtily >= pspa->dtilyLimit+pspa->ctilyLimit)
				{
					pspa->psaeLimitRow.iBank = EndOfRow;
					pspa->psaeLimitCol.iBank = EndOfCol;
					*pdtilx = 0;
					*pdtily = 0;
					return NULL;						// we're done!
				}
			}

			pspa->psaeLastCol = pspa->psaeLimitCol;
			ReadSparse(pspa->psaeLastCol, &pspa->splLast);
			pspa->psaeLimitCol = pspa->splLast.NextCol;
		}
		while (	pspa->ctilxLimit != 0 &&
					(pspa->splLast.dtilx < pspa->dtilxLimit || pspa->splLast.dtilx >= pspa->dtilxLimit+pspa->ctilxLimit)	);
	}
	while (pspa->splLast.plt.Plot_Flags == 0 && pspa->splLast.plt.TileSet_ID == 0 && pspa->splLast.plt.Tile_ID == 0);

	*pdtilx = pspa->splLast.dtilx;
	*pdtily = pspa->sprLast.dtily;
	return &pspa->splLast.plt;
} /* GetNextSparsePlotInRange */


/*********************************************************************
 *
 * GetSparsePlotXY
 *
 * PURPOSE
 *		Read PlotType from coordinate <dtilx>, <dtily> in sparse array
 *		<pspa>. Return pointer to PlotType read.
 *
 * INPUT
 *		SparseArray *pspa			: which sparse array to get value from
 *		int dtilx					: x-offset to read pplt from
 *		int dtily					: y-offset to read pplt from
 *
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/17/94 Wednesday (dcc) - Created.
 *
 *
*/
PlotType *GetSparsePlotXY(SparseArray *pspa, int dtilx, int dtily)
{


/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetSparsePlotXY";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* We guarantee that any valid pspa will have at least one row */

	Assert(pspa != NULL);
	Assert(pspa->psaeFirstRow.iBank != EndOfRow);
	Assert(FGoodSparseArray(pspa));
	if (pspa->psaeLastRow.iBank != EndOfRow)		// row valid?
	{
		if (pspa->sprLast.dtily == dtily)	// same row?
		{
			if (pspa->psaeLastCol.iBank != EndOfCol)	// column valid?
			{
				if (pspa->splLast.dtilx == dtilx)// same column?
				{


					Assert(FGoodSparseArray(pspa));
					return &pspa->splLast.plt;
				}
				if (pspa->splLast.dtilx < dtilx)
					goto OnRightRowAndColIsB4;
			}
			goto OnRightRowSoResetCol;
		}
		else									// not same row
		{
			if (pspa->sprLast.dtily < dtily)	// not same row, but is it before?
				goto RowIsB4;
		}
	}
ResetRow:
	// Assert: pspa->psaeFirstRow is correct, pspa->sprLast & pspa->splLast are garbage (don't care)

	Assert(pspa->psaeFirstRow.iBank != EndOfRow);

	pspa->psaeLastRow = pspa->psaeFirstRow;
	ReadSparse(pspa->psaeLastRow, &pspa->sprLast);

RowIsB4:
	// pspa->sprLast is correct and pspa->sprLast.dtily <= dtily), but
	//	pspa->splLast is garbage (don't care)

	Assert(pspa->psaeFirstRow.iBank != EndOfRow);
	Assert(pspa->psaeLastRow.iBank != EndOfRow);
	Assert(pspa->sprLast.dtily <= dtily);

	pspa->psaeLastCol.iBank = EndOfCol;	// mark pspa->splLast as not valid

	while (pspa->sprLast.dtily < dtily)
	{
		if (pspa->sprLast.NextRow.iBank == EndOfRow)
		{
			Assert(FGoodSparseArray(pspa));



			return &pltNull;					// no more rows!
		}

		pspa->psaeLastRow = pspa->sprLast.NextRow;
		ReadSparse(pspa->psaeLastRow, &pspa->sprLast);
		Assert(pspa->psaeLastRow.iBank != EndOfRow);
	}

	Assert(pspa->psaeLastRow.iBank != EndOfRow);
	if (pspa->sprLast.dtily > dtily)
	{
		// we've gone from pspa->sprLast.dtily < dtily to pspa->sprLast.dtily > dtily
		// without finding the row where pspa->sprLast.dtily == dtily, so quit

		Assert(FGoodSparseArray(pspa));




		return &pltNull;						// skipped row in question
	}
OnRightRowSoResetCol:
	// Assert: pspa->psaeFirstRow, pspa->sprLast correct, pspa->splLast is garbage (don't care)

	Assert(pspa->psaeFirstRow.iBank != EndOfRow);
	Assert(pspa->psaeLastRow.iBank != EndOfRow);
	Assert(pspa->sprLast.dtily == dtily);

	pspa->psaeLastCol = pspa->sprLast.FirstCol;

	/* We guarantee at least one SparsePlotType (column 0) per row */

	Assert(pspa->psaeLastCol.iBank != EndOfCol);
	ReadSparse(pspa->psaeLastCol, &pspa->splLast);

OnRightRowAndColIsB4:
	// Assert: pspa->psaeFirstRow, pspa->sprLast & pspa->splLast up to date, pspa->splLast.dtilx <= dtilx

	Assert(pspa->psaeFirstRow.iBank != EndOfRow);
	Assert(pspa->psaeLastRow.iBank != EndOfRow);
	Assert(pspa->sprLast.dtily == dtily);
	Assert(pspa->psaeLastCol.iBank != EndOfCol);
	Assert(pspa->splLast.dtilx <= dtilx);
	Assert(FGoodSparseArray(pspa));

	while (pspa->splLast.dtilx < dtilx)
	{
		if (pspa->splLast.NextCol.iBank == EndOfCol)
		{
			Assert(FGoodSparseArray(pspa));



			return &pltNull;					// no more columns!
		}

		pspa->psaeLastCol = pspa->splLast.NextCol;
		ReadSparse(pspa->psaeLastCol, &pspa->splLast);
		Assert(pspa->psaeLastCol.iBank != EndOfCol);
	}

	Assert(pspa->psaeLastCol.iBank != EndOfCol);
	if (pspa->splLast.dtilx > dtilx)
	{
		// we've gone from pspa->splLast.dtilx < dtilx to pspa->splLast.dtilx > dtilx
		// without finding the col where pspa->splLast.dtilx == dtilx, so quit




		return &pltNull;						// skipped column in question
	}


	return &pspa->splLast.plt;
} /* GetSparsePlotXY */


/*********************************************************************
 *
 * ReadSparsePlotXY
 *
 * PURPOSE
 *		Read PlotType from coordinate <dtilx>, <dtily> in sparse array
 *		<pspa> into <*pplt>.
 *
 * INPUT
 *		SparseArray *pspa			: which sparse array to get value from
 *		int dtilx					: x-offset to read pplt from
 *		int dtily					: y-offset to read pplt from
 *		PlotType *pplt				: where to store results
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/19/94 Friday (dcc) - Created.
 *
*/
void ReadSparsePlotXY(SparseArray *pspa, int dtilx, int dtily, PlotType *pplt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReadSparsePlotXY";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	*pplt = *GetSparsePlotXY(pspa, dtilx, dtily);

} /* ReadSparsePlotXY */


/*********************************************************************
 *
 * WriteSparsePlotXY
 *
 * PURPOSE
 *		Write <*pplt> to coordinate <dtilx>, <dtily> in sparse array
 *		<pspa>.
 *
 * INPUT
 *		PlotType *pplt		: plot value to store
 *		SparseArray *pspa	: which sparse array to write value to
 *		int dtilx			: x-offset to write pplt to
 *		int dtily			: y-offset to write pplt to
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/18/94 Thursday (dcc) - Created.
 *		08/19/94 Friday (dcc) - Enhanced to NOT insert NULL tiles.
 *
*/
void WriteSparsePlotXY(PlotType *pplt, SparseArray *pspa, int dtilx, int dtily)
{
	static PSAE psaePrevRow, psaePrevCol;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteSparsePlotXY";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* We guarantee that any valid pspa will have at least one row */

	Assert(pspa != NULL);
	Assert(pspa->psaeFirstRow.iBank != EndOfRow);
	Assert(FGoodSparseArray(pspa));
	if (pspa->psaeLastRow.iBank != EndOfRow)		// row valid?
	{
		if (pspa->sprLast.dtily == dtily)	// same row?
		{
			if (pspa->psaeLastCol.iBank != EndOfCol)	// column valid?
			{
				if (pspa->splLast.dtilx == dtilx)// same column?
				{
					pspa->splLast.plt = *pplt;
					WriteSparse(&pspa->splLast, pspa->psaeLastCol);
					Assert(FGoodSparseArray(pspa));
					return;
				}
				if (pspa->splLast.dtilx < dtilx)
					goto OnRightRowAndColIsB4;
			}
			goto OnRightRowSoResetCol;
		}
		else									// not same row
		{
			if (pspa->sprLast.dtily < dtily)	// not same row, but is it before?
				goto RowIsB4;
		}
	}
ResetRow:
	// Assert: pspa->psaeFirstRow is correct, pspa->sprLast & pspa->splLast are garbage (don't care)

	Assert(pspa->psaeFirstRow.iBank != EndOfRow);

	pspa->psaeLastRow = pspa->psaeFirstRow;
	ReadSparse(pspa->psaeLastRow, &pspa->sprLast);

RowIsB4:
	// pspa->sprLast is correct and pspa->sprLast.dtily <= dtily), but
	//	pspa->splLast is garbage (don't care)

	Assert(pspa->psaeFirstRow.iBank != EndOfRow);
	Assert(pspa->psaeLastRow.iBank != EndOfRow);
	Assert(pspa->sprLast.dtily <= dtily);

	pspa->psaeLastCol.iBank = EndOfCol;	// mark pspa->splLast as not valid

	while (pspa->sprLast.dtily < dtily)
	{
		if (pspa->sprLast.NextRow.iBank == EndOfRow)
		{
			Assert(FGoodSparseArray(pspa));
			if (fZeroTile(pplt))   return;// don't insert NULL tiles
			if (!AddNewRow(&pspa->psaeLastRow, &pspa->sprLast, dtily, &pspa->psaeLastCol, &pspa->splLast))	return;// out of memory!
			Assert(FGoodSparseArray(pspa));
			goto OnRightRowSoResetCol;
		}
		psaePrevRow = pspa->psaeLastRow;
		pspa->psaeLastRow = pspa->sprLast.NextRow;
		ReadSparse(pspa->psaeLastRow, &pspa->sprLast);
		Assert(pspa->psaeLastRow.iBank != EndOfRow);
	}

	Assert(pspa->psaeLastRow.iBank != EndOfRow);
	if (pspa->sprLast.dtily > dtily)
	{
		// we've gone from pspa->sprLast.dtily < dtily to pspa->sprLast.dtily > dtily
		// without finding the row where pspa->sprLast.dtily == dtily, add new row

		Assert(FGoodSparseArray(pspa));
		if (fZeroTile(pplt))		return;	// don't insert NULL tiles
		pspa->psaeLastRow = psaePrevRow;
		ReadSparse(pspa->psaeLastRow, &pspa->sprLast);// back up one row...
		if (!AddNewRow(&pspa->psaeLastRow, &pspa->sprLast, dtily, &pspa->psaeLastCol, &pspa->splLast))	return;	// out of memory!
		Assert(FGoodSparseArray(pspa));
	}
OnRightRowSoResetCol:
	// Assert: pspa->psaeFirstRow, pspa->sprLast correct, pspa->splLast is garbage (don't care)

	Assert(pspa->psaeFirstRow.iBank != EndOfRow);
	Assert(pspa->psaeLastRow.iBank != EndOfRow);
	Assert(pspa->sprLast.dtily == dtily);

	pspa->psaeLastCol = pspa->sprLast.FirstCol;

	/* We guarantee at least one SparsePlotType (column 0) per row */

	Assert(pspa->psaeLastCol.iBank != EndOfCol);
	ReadSparse(pspa->psaeLastCol, &pspa->splLast);

OnRightRowAndColIsB4:
	// Assert: pspa->psaeFirstRow, pspa->sprLast & pspa->splLast up to date, pspa->splLast.dtilx <= dtilx

	Assert(pspa->psaeFirstRow.iBank != EndOfRow);
	Assert(pspa->psaeLastRow.iBank != EndOfRow);
	Assert(pspa->sprLast.dtily == dtily);
	Assert(pspa->psaeLastCol.iBank != EndOfCol);
	Assert(pspa->splLast.dtilx <= dtilx);
	Assert(FGoodSparseArray(pspa));

	while (pspa->splLast.dtilx < dtilx)
	{
		if (pspa->splLast.NextCol.iBank == EndOfCol)
		{
			Assert(FGoodSparseArray(pspa));
			if (fZeroTile(pplt))   return;// don't insert NULL tiles
			if (!AddNewCol(pspa, dtilx)) return;// out of memory!
			Assert(FGoodSparseArray(pspa));
			goto OnRightRowAndCol;
		}
		psaePrevCol = pspa->psaeLastCol;
		pspa->psaeLastCol = pspa->splLast.NextCol;
		ReadSparse(pspa->psaeLastCol, &pspa->splLast);
		Assert(pspa->psaeLastCol.iBank != EndOfCol);
	}

	Assert(pspa->psaeLastCol.iBank != EndOfCol);
	if (pspa->splLast.dtilx > dtilx)
	{
		// we've gone from pspa->splLast.dtilx < dtilx to pspa->splLast.dtilx > dtilx
		// without finding the col where pspa->splLast.dtilx == dtilx, add new col

		if (fZeroTile(pplt))		return;	// don't insert NULL tiles
		pspa->psaeLastCol = psaePrevCol;
		ReadSparse(pspa->psaeLastCol, &pspa->splLast);// back up one col...
		if (!AddNewCol(pspa, dtilx))	return;	// out of memory!
	}
OnRightRowAndCol:
	pspa->splLast.plt = *pplt;
	WriteSparse(&pspa->splLast, pspa->psaeLastCol);
	Assert(FGoodSparseArray(pspa));
} /* WriteSparsePlotXY */


/*********************************************************************
 *
 * DeleteSparseArrayRows
 *
 * PURPOSE
 *		Delete <wNumRows> starting at <wRow> from <pspa>.
 *
 * INPUT
 *		SparseArray *pspa0: sparse array to delete rows
 *		int wRow				: which row to start deleting from
 *		int wNumRows		: number of rows to delete
 *
 * ASSUMES
 *		There is a tile at (0, 0).
 *
 * RETURN VALUE
 *		TRUE if successful.
 *
 * HISTORY
 *		08/19/94 (dcc) - Created.
 *		04/03/95 (dcc) - Invalidate psaeLastRow if it was in deleted range.
 *
*/
BOOL DeleteSparseArrayRows(SparseArray *pspa0, int wRow, int wNumRows)
{
	PSAE psaeD, psae, psaePrevRow;

#define DoNothing	0
#define DeleteRow	1
#define Renumber	2

	int state = DoNothing;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteSparseArrayRows";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Assert(pspa0 != NULL);
	Assert(FGoodSparseArray(pspa0));

	if (pspa0->psaeFirstRow.iBank == EndOfRow)
		return TRUE;							// nothing to do!

	if (wNumRows == 0)
		return TRUE;							// nothing to do!

	if (pspa0->psaeLastRow.iBank != EndOfRow)
	{
		if (wRow <= pspa0->sprLast.dtily && pspa0->sprLast.dtily < wRow+wNumRows)
		{
			pspa0->psaeLastRow.iBank = EndOfRow;
			pspa0->psaeLastCol.iBank = EndOfCol;
		}
	}

	psae = pspa0->psaeFirstRow;
	psaeD.iBank = EndOfRow;
	psaePrevRow.iBank = EndOfRow;

	while (psae.iBank != EndOfRow)
	{
		SparseRowType spr;

		ReadSparse(psae, &spr);

		if (state == DoNothing && wRow <= spr.dtily && spr.dtily < wRow+wNumRows)
		{
			psaePrevRow = psaeD;
			state = DeleteRow;
		}

		if (state != Renumber && spr.dtily >= wRow+wNumRows)
		{
			if (wRow == 0 && wNumRows == spr.dtily)	// replacing (0,0) tile?
			{
				WriteSparse(&spr, pspa0->psaeFirstRow);
				Assert(FGoodSparseArray(pspa0));
				psae = pspa0->psaeFirstRow;
			}
			else if (psaePrevRow.iBank != EndOfRow)
			{
				SparseRowType sprPrev;

				ReadSparse(psaePrevRow, &sprPrev);// link previous row to current row
				sprPrev.NextRow = psae;
				WriteSparse(&sprPrev, psaePrevRow);
				Assert(FGoodSparseArray(pspa0));
			}
			state = Renumber;
		}

		if (state == DeleteRow)
		{
			PSAE psaeC = spr.FirstCol;

			while (psaeC.iBank != EndOfCol)
			{
				SparsePlotType spl;

				ReadSparse(psaeC, &spl);

				psaeD = psaeC;
				psaeC = spl.NextCol;
				AddSparseArrayToFree(psaeD);
			}
		}
		else if (state == Renumber)
		{
			spr.dtily -= wNumRows;
			WriteSparse(&spr, psae);
			Assert(FGoodSparseArray(pspa0));
		}
		psaeD	= psae;
		psae	= spr.NextRow;
		if (state == DeleteRow)
		{
			if (spr.dtily != 0)
			{
				AddSparseArrayToFree(psaeD);
			}
			else
			{
				PSAE psaeC;
				SparsePlotType spl;

				psaeC = GetFreeSparseArray();// get new column marker
				if (psaeC.iBank == EndOfCol)
					return FALSE;				// out of memory!

				spr.FirstCol = psaeC;
				WriteSparse(&spr, pspa0->psaeFirstRow);
				Assert(FGoodSparseArray(pspa0));

				spl.plt.Plot_Flags	= 0;
				spl.plt.TileSet_ID	= 0;
				spl.plt.Tile_ID		= 0;
				spl.dtilx				= 0;
				spl.NextCol.iBank		= EndOfCol;// no more col after this new one
				WriteSparse(&spl, psaeC);	// save new col
				Assert(FGoodSparseArray(pspa0));
			}
		}
	}
	Assert(FGoodSparseArray(pspa0));
	return TRUE;
} /* DeleteSparseArrayRows */


/*********************************************************************
 *
 * FreeSparseArray
 *
 * PURPOSE
 *		Free the memory used by sparse array <pspa>.
 *
 * INPUT
 *		SparseArray *pspa		: sparse array to free
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/18/94 (dcc) - Created.
 *		04/03/95 (dcc) - Invalidate psaeLastRow & psaeLastCol.
 *
*/
void FreeSparseArray(SparseArray *pspa)
{
	PSAE psae;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FreeSparseArray";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Assert(pspa != NULL);
	Assert(FGoodSparseArray(pspa));

	pspa->psaeLastRow.iBank = EndOfRow;
	pspa->psaeLastCol.iBank = EndOfCol;

	psae = pspa->psaeFirstRow;

	while (psae.iBank != EndOfRow)
	{
		PSAE psaeF;
		SparseRowType spr;

		ReadSparse(psae, &spr);
		{
			PSAE psaeC = spr.FirstCol;

			while (psaeC.iBank != EndOfCol)
			{
				SparsePlotType spl;

				ReadSparse(psaeC, &spl);

				psaeF = psaeC;
				psaeC = spl.NextCol;
				AddSparseArrayToFree(psaeF);
			}
		}
		psaeF = psae;
		psae = spr.NextRow;
		AddSparseArrayToFree(psaeF);
	}
	MEM_free(pspa);
} /* FreeSparseArray */


/*********************************************************************
 *
 * FindRangeInRow
 *
 * PURPOSE
 *		For a row in the sparse array, find the last element in the row
 *		that is < <dtilx>, and the first element in the row that is
 *		>= <dtilx>+<ctilx>.
 *
 * INPUT
 *		SparseRowType *pspr	: pointer to row to check
 *		int dtilx				: leftmost x coordinate to check
 *		int ctilx				: width to check
 *
 * ASSUMES
 *		The first element of every row will have a spl.dtilx == 0.
 *		If dtilx == 0, then pspaBefore->iBank will point to 1st element
 *		to BE DELETED, instead of the element BEFORE the 1st element.
 *
 * RETURN VALUE
 *		PSAE *psaeBefore		: pointer to last element B4 dtilx. If
 *									  psaeBefore.iBank == EndOfCol, then none found
 *									  in requested range
 *		PSAE *psaeLast			: pointer to last element in range. If
 *									  psaeLast.iBank == EndOfCol, then none found
 *									  in requested range
 *		PSAE *psaeAfter		: pointer to 1st element after dtilx+ctilx
 *
 * HISTORY
 *		09/04/94 (dcc) - Created.
 *
*/
static void FindRangeInRow(SparseRowType *pspr,
									int dtilx, int ctilx,
									PSAE *psaeBefore,
									PSAE *psaeLast,
									PSAE *psaeAfter)
{
	PSAE psaeC = pspr->FirstCol;
	SparsePlotType spl;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FindRangeInRow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	psaeBefore->iBank = EndOfCol;
	psaeAfter->iBank = EndOfCol;
	psaeLast->iBank = EndOfCol;

	while (psaeC.iBank != EndOfCol)
	{
		ReadSparse(psaeC, &spl);

		if (	(dtilx == 0 && spl.dtilx == dtilx) ||
				spl.dtilx < dtilx	)
			*psaeBefore = psaeC;				// remember element B4 1st in range

		if (dtilx <= spl.dtilx && spl.dtilx < dtilx+ctilx)
		{
			*psaeLast = psaeC;
		}
		if (spl.dtilx >= dtilx+ctilx)
		{
			*psaeAfter = psaeC;
			return;
		}
		psaeC = spl.NextCol;
	}
} /* FindRangeInRow */


/*********************************************************************
 *
 * ZeroRangeInRow
 *
 * PURPOSE
 *		Zero tiles (delete elements) in range pointed to by
 *		psaeBefore..psaeAfter.
 *
 * INPUT
 *		PSAE psaeBefore	: B4 1st element to be deleted
 *		PSAE psaeAfter		: After last element to be deleted
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		09/07/94 (dcc) - Created.
 *
*/
static void ZeroRangeInRow(int dtilx,
									PSAE psaeBefore,
									PSAE psaeLast,
									PSAE psaeAfter)
{
	SparsePlotType spl;
	PSAE psaeZ;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ZeroRangeInRow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (psaeBefore.iBank == EndOfCol || psaeLast.iBank == EndOfCol)
		return;									// none to delete

	/* Link psaeBefore to point to psaeAfter, set psaeZ to 1st element to zero */

	ReadSparse(psaeBefore, &spl);
	psaeZ = spl.NextCol;
	spl.NextCol = psaeAfter;
	if (spl.dtilx == 0 && dtilx == 0)
	{
		spl.plt.Plot_Flags	= 0;
		spl.plt.TileSet_ID	= 0;
		spl.plt.Tile_ID		= 0;
	}
	WriteSparse(&spl, psaeBefore);

	/* Delete all elements between psaeBefore and psaeAfter */

	while (	psaeZ.iBank != EndOfCol && 
			psaeZ.irgpBank != psaeAfter.irgpBank &&
				psaeZ.iBank != psaeAfter.iBank)
	{
		PSAE psaeF;

		ReadSparse(psaeZ, &spl);

		psaeF = psaeZ;
		psaeZ = spl.NextCol;
		AddSparseArrayToFree(psaeF);
	}

} /* ZeroRangeInRow */


/*********************************************************************
 *
 * ZeroRectInSparseArray
 *
 * PURPOSE
 *		Zero tiles in range <dtilx>..<dtilx>+<ctilx>-1, <dtily>..
 *		<dtily>+<ctily>-1.
 *
 * INPUT
 *		SparseArray *pspa	: sparse array to zero
 *		int dtilx			: left edge of tiles to include
 *		int dtily			: top edge of tiles to include
 *		int ctilx			: width of tiles to include
 *		int ctily			: height of tiles to include
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		09/04/94 (dcc) - Created.
 *		09/07/94 (dcc) - Revised to use FindRangeInRow() and ZeroRangeInRow().
 *		04/03/95 (dcc) - Invalidate psaeLastCol if it was in zero'ed range.
 *
*/
void ZeroRectInSparseArray(SparseArray *pspa,
									int dtilx, int dtily, int ctilx, int ctily)
{
	PSAE psae;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ZeroRectInSparseArray";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Assert(pspa != NULL);
	Assert(FGoodSparseArray(pspa));

	if (pspa->psaeLastRow.iBank != EndOfRow)
	{
		if (dtily <= pspa->sprLast.dtily && pspa->sprLast.dtily < dtily+ctily)
		{
			if (pspa->psaeLastCol.iBank != EndOfCol)
			{
				if (dtilx <= pspa->splLast.dtilx && pspa->splLast.dtilx < dtilx+ctilx)
				{
					pspa->psaeLastCol.iBank = EndOfCol;
				}
			}
		}
	}

	psae = pspa->psaeFirstRow;

	while (psae.iBank != EndOfRow)
	{
		SparseRowType spr;

		ReadSparse(psae, &spr);
		if (dtily <= spr.dtily && spr.dtily < dtily+ctily)
		{
			PSAE psaeBefore, psaeLast, psaeAfter;

			FindRangeInRow(&spr, dtilx, ctilx, &psaeBefore, &psaeLast, &psaeAfter);
			ZeroRangeInRow(dtilx, psaeBefore, psaeLast, psaeAfter);
			Assert(FGoodSparseArray(pspa));
		}
		psae = spr.NextRow;
	}
	Assert(FGoodSparseArray(pspa));
} /* ZeroRectInSparseArray */


/*********************************************************************
 *
 * SwapRectInSparseArray
 *
 * PURPOSE
 *		Swap tiles from <pspa1> in range <dtilx>..<dtilx>+<ctilx>-1,
 *		<dtily>..<dtily>+<ctily>-1 with those in <pspa2> in the same
 *		range.
 *
 * INPUT
 *		SparseArray *pspa1: sparse array to swap
 *		SparseArray *pspa2: other sparse array to swap
 *		int dtilx			: left edge of tiles to include
 *		int dtily			: top edge of tiles to include
 *		int ctilx			: width of tiles to include
 *		int ctily			: height of tiles to include
 *
 * ASSUMES
 *		pspa1->psaeFirstRow.iBank != EndOfRow &&
 *		pspa2->psaeFirstRow.iBank != EndOfRow on entry.
 *		There is a tile at (0, 0), and at (0, y) of each row.
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		09/07/94 (dcc) - Created.
 *		04/03/95 (dcc) - Check psaeLastRow in range B4 invalidating it.
 *
*/
void SwapRectInSparseArray(SparseArray *pspa1, SparseArray *pspa2,
									int dtilx, int dtily, int ctilx, int ctily)
{
	PSAE psae1, psae2;
	PSAE psaePrevRow1, psaePrevRow2;
	SparseRowType spr1, spr2;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SwapRectInSparseArray";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Assert(pspa1 != NULL);
	Assert(FGoodSparseArray(pspa1));
	psae1 = pspa1->psaeFirstRow;
	psaePrevRow1 = psae1;
	if (psae1.iBank == EndOfRow)
		return;									// error!
	ReadSparse(psae1, &spr1);
	while (spr1.dtily < dtily)
	{
		psaePrevRow1 = psae1;
		psae1 = spr1.NextRow;
		if (psae1.iBank == EndOfRow)
			break;
		ReadSparse(psae1, &spr1);
	}

	Assert(pspa2 != NULL);
	Assert(FGoodSparseArray(pspa2));
	psae2 = pspa2->psaeFirstRow;
	psaePrevRow2 = psae2;
	if (psae2.iBank == EndOfRow)
		return;									// error!
	ReadSparse(psae2, &spr2);
	while (spr2.dtily < dtily)
	{
		psaePrevRow2 = psae2;
		psae2 = spr2.NextRow;
		if (psae2.iBank == EndOfRow)
			break;
		ReadSparse(psae2, &spr2);
	}

	/* Mark pspaN->psaeLastRow, pspaN->psaeLastCol as no longer valid. */

	if (pspa1->psaeLastRow.iBank != EndOfRow)
	{
		if (dtily <= pspa1->sprLast.dtily && pspa1->sprLast.dtily < dtily+ctily)
		{
			pspa1->psaeLastRow.iBank = EndOfRow;
			pspa1->psaeLastCol.iBank = EndOfCol;
		}
	}
	if (pspa1->psaeLastRow.iBank != EndOfRow)
	{
		if (dtily <= pspa1->sprLast.dtily && pspa1->sprLast.dtily < dtily+ctily)
		{
			pspa2->psaeLastRow.iBank = EndOfRow;
			pspa2->psaeLastCol.iBank = EndOfCol;
		}
	}

	while (	(psae1.iBank != EndOfRow && dtily <= spr1.dtily && spr1.dtily < dtily+ctily) ||
				(psae2.iBank != EndOfRow && dtily <= spr2.dtily && spr2.dtily < dtily+ctily)	)
	{
		PSAE psaeBefore1, psaeLast1, psaeAfter1;
		PSAE psaeBefore2, psaeLast2, psaeAfter2;
		SparsePlotType spl1, spl2;
		BOOL fInc1 = FALSE;
		BOOL fInc2 = FALSE;

		if (	psae1.iBank != EndOfRow && psae2.iBank != EndOfRow &&
				spr1.dtily == spr2.dtily)
		{
			/* row exists in psae1 and psae2 */

			FindRangeInRow(&spr1, dtilx, ctilx, &psaeBefore1, &psaeLast1, &psaeAfter1);
			FindRangeInRow(&spr2, dtilx, ctilx, &psaeBefore2, &psaeLast2, &psaeAfter2);
			fInc1 = TRUE;
			fInc2 = TRUE;
		}
		else if (	(psae1.iBank != EndOfRow && psae2.iBank != EndOfRow && spr1.dtily < spr2.dtily) ||
						(psae1.iBank != EndOfRow && psae2.iBank == EndOfRow)	)
		{
			/* row exists in psae1 */

			FindRangeInRow(&spr1, dtilx, ctilx, &psaeBefore1, &psaeLast1, &psaeAfter1);
			fInc1 = TRUE;

			if (psaeLast1.iBank != EndOfCol)
			{
				/* need to create psae2 */

				Assert(psaePrevRow2.iBank != EndOfRow);

				ReadSparse(psaePrevRow2, &spr2);
				AddNewRow(&psaePrevRow2, &spr2, spr1.dtily, &psaeBefore2, &spl2);
				Assert(FGoodSparseArray(pspa2));
				psae2 = psaePrevRow2;
				fInc2 = TRUE;
			}
			psaeLast2.iBank = EndOfCol;
			psaeAfter2.iBank = EndOfCol;
		}
		else if (	(psae1.iBank != EndOfRow && psae2.iBank != EndOfRow && spr2.dtily < spr1.dtily) ||
						(psae2.iBank != EndOfRow && psae1.iBank == EndOfRow)	)
		{
			/* row exists in psae2 */

			FindRangeInRow(&spr2, dtilx, ctilx, &psaeBefore2, &psaeLast2, &psaeAfter2);
			fInc2 = TRUE;

			if (psaeLast2.iBank != EndOfCol)
			{
				/* need to create psae1 */

				Assert(psaePrevRow1.iBank != EndOfRow);

				ReadSparse(psaePrevRow1, &spr1);
				AddNewRow(&psaePrevRow1, &spr1, spr2.dtily, &psaeBefore1, &spl1);
				Assert(FGoodSparseArray(pspa1));
				psae1 = psaePrevRow1;
				fInc1 = TRUE;
			}
			psaeLast1.iBank = EndOfCol;
			psaeAfter1.iBank = EndOfCol;
		}

		/* Do links at beginning */

		if (dtilx == 0)
		{
			/* swap links to beginning of the regions */

			Assert(psaeBefore2.iBank != EndOfCol);

			spr1.FirstCol = psaeBefore2;
			WriteSparse(&spr1, psae1);

			Assert(psaeBefore1.iBank != EndOfCol);

			spr2.FirstCol = psaeBefore1;
			WriteSparse(&spr2, psae2);
		}
		else
		{
			/* swap links to beginning of the regions */

			if (psaeLast1.iBank != EndOfCol && psaeLast2.iBank != EndOfCol)
			{
				Assert(psaeBefore1.iBank != EndOfCol);

				ReadSparse(psaeBefore1, &spl1);
				psaeAfter1 = spl1.NextCol;

				Assert(psaeBefore2.iBank != EndOfCol);

				ReadSparse(psaeBefore2, &spl2);
				psaeAfter2 = spl2.NextCol;

				spl1.NextCol = psaeAfter2;
				WriteSparse(&spl1, psaeBefore1);

				spl2.NextCol = psaeAfter1;
				WriteSparse(&spl2, psaeBefore2);
			}
			else if (psaeLast1.iBank != EndOfCol)
			{
				PSAE psae;

				Assert(psaeBefore1.iBank != EndOfCol);

				ReadSparse(psaeBefore1, &spl1);
				psae = spl1.NextCol;
				spl1.NextCol = psaeAfter1;
				WriteSparse(&spl1, psaeBefore1);

				Assert(psaeBefore2.iBank != EndOfCol);

				ReadSparse(psaeBefore2, &spl2);
				psaeAfter2 = spl2.NextCol;

				spl2.NextCol = psae;
				WriteSparse(&spl2, psaeBefore2);
			}
			else if (psaeLast2.iBank != EndOfCol)
			{
				PSAE psae;

				Assert(psaeBefore2.iBank != EndOfCol);

				ReadSparse(psaeBefore2, &spl2);
				psae = spl2.NextCol;
				spl2.NextCol = psaeAfter2;
				WriteSparse(&spl2, psaeBefore2);

				Assert(psaeBefore1.iBank != EndOfCol);

				ReadSparse(psaeBefore1, &spl1);
				psaeAfter1 = spl1.NextCol;

				spl1.NextCol = psae;
				WriteSparse(&spl1, psaeBefore1);
			}
		}

		/* Do links at end */

		if (psaeLast1.iBank != EndOfCol && psaeLast2.iBank != EndOfCol)
		{
			/* swap links at end of the regions */

			ReadSparse(psaeLast1, &spl1);
			psaeAfter1 = spl1.NextCol;

			ReadSparse(psaeLast2, &spl2);
			psaeAfter2 = spl2.NextCol;

			spl1.NextCol = psaeAfter2;
			WriteSparse(&spl1, psaeLast1);

			spl2.NextCol = psaeAfter1;
			WriteSparse(&spl2, psaeLast2);
		}
		else if (psaeLast1.iBank != EndOfCol)
		{
			ReadSparse(psaeLast1, &spl1);

			spl1.NextCol = psaeAfter2;
			WriteSparse(&spl1, psaeLast1);
		}
		else if (psaeLast2.iBank != EndOfCol)
		{
			ReadSparse(psaeLast2, &spl2);

			spl2.NextCol = psaeAfter1;
			WriteSparse(&spl2, psaeLast2);
		}

		Assert(FGoodSparseArray(pspa1));
		Assert(FGoodSparseArray(pspa2));

		/* read next rows */

		if (fInc1)
			if (psae1.iBank != EndOfRow)
			{
				psaePrevRow1 = psae1;
				psae1 = spr1.NextRow;
				if (psae1.iBank != EndOfRow)
					ReadSparse(psae1, &spr1);
			}

		if (fInc2)
			if (psae2.iBank != EndOfRow)
			{
				psaePrevRow2 = psae2;
				psae2 = spr2.NextRow;
				if (psae2.iBank != EndOfRow)
					ReadSparse(psae2, &spr2);
			}
	}
	Assert(FGoodSparseArray(pspa1));
	Assert(FGoodSparseArray(pspa2));
} /* SwapRectInSparseArray */


#if DEBUG
#if 1
/*********************************************************************
 *
 * PrintSparseArray
 *
 * PURPOSE
 *		Print out the contents of the sparse array <pspa>.
 *
 *		This routine is used to debug the sparse array code.
 *
 * INPUT
 *		SparseArray *pspa	: sparse array to print
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/18/94 Thursday (dcc) - Created.
 *
*/
void PrintSparseArray(SparseArray *pspa)
{
	PlotType *pplt;
	int dtilx, dtily;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PrintSparseArray";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetSparseLayerLimits(pspa, 0, 0, 0, 0); //30000, 20000); //65535U, 65535U);

	while ((pplt = GetNextSparsePlotInRange(pspa, &dtilx, &dtily)) != NULL)
	{
		printf("(%d,%d):{%u,%u,%u} \n", dtilx, dtily,
				pplt->Plot_Flags, pplt->TileSet_ID, pplt->Tile_ID);
	}
} /* PrintSparseArray */
#endif

#if 0
/*********************************************************************
 *
 * PrintSparseArray
 *
 * PURPOSE
 *		Print out the contents of the sparse array <pspa>.
 *
 *		This routine is used to debug the sparse array code.
 *
 * INPUT
 *		SparseArray *pspa	: sparse array to print
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		NONE.
 *
 * HISTORY
 *		08/18/94 Thursday (dcc) - Created.
 *
*/
void PrintSparseArray(SparseArray *pspa)
{
	PSAE psae;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PrintSparseArray";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	Assert(pspa != NULL);

	psae = pspa->psaeFirstRow;
	while (psae.iBank != EndOfRow)
	{
		SparseRowType spr;

		ReadSparse(psae, &spr);
		printf("Row = %d\n", spr.dtily);
		{
			PSAE psaeC = spr.FirstCol;

			while (psaeC.iBank != EndOfCol)
			{
				SparsePlotType spl;

				ReadSparse(psaeC, &spl);
				printf("(%d,%d):{%u,%u,%u} ", spl.dtilx, spr.dtily,
							spl.plt.Plot_Flags, spl.plt.TileSet_ID, spl.plt.Tile_ID);

				psaeC = spl.NextCol;
			}
		}
		printf("\n");
		psae = spr.NextRow;
	}
} /* PrintSparseArray */
#endif
#endif // DEBUG

#if 0

void Quit(char *sz)
{
	printf(sz);
	exit(1);
}

void main()
{
	PlotType plt;
	SparseArray *pspa, *pspa2;

	InstallCheckPointer();

	InitSparse();

	if ((pspa2 = MakeNewSparseArray()) == NULL)
		return;									// OOM

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 11;
	plt.Tile_ID		= 11;
	WriteSparsePlotXY(&plt, pspa2, 1, 1);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 0;
	plt.Tile_ID		= 22;
	WriteSparsePlotXY(&plt, pspa2, 0, 2);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 22;
	plt.Tile_ID		= 22;
	WriteSparsePlotXY(&plt, pspa2, 2, 2);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 33;
	plt.Tile_ID		= 33;
//	WriteSparsePlotXY(&plt, pspa2, 3, 3);

	if ((pspa = MakeNewSparseArray()) == NULL)
		return;									// OOM

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 3;
	plt.Tile_ID		= 3;
	WriteSparsePlotXY(&plt, pspa, 3, 3);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 0;
	plt.Tile_ID		= 44;
	WriteSparsePlotXY(&plt, pspa, 0, 4);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 3;
	plt.Tile_ID		= 44;
	WriteSparsePlotXY(&plt, pspa, 3, 4);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 3;
	plt.Tile_ID		= 11;
	WriteSparsePlotXY(&plt, pspa, 3, 1);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 5;
	plt.Tile_ID		= 33;
	WriteSparsePlotXY(&plt, pspa, 5, 3);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 0;
	plt.Tile_ID		= 11;
	WriteSparsePlotXY(&plt, pspa, 0, 1);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 2;
	plt.Tile_ID		= 11;
	WriteSparsePlotXY(&plt, pspa, 2, 1);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 7;
	plt.Tile_ID		= 7;
	WriteSparsePlotXY(&plt, pspa, 25000, 16000);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 65;
	plt.Tile_ID		= 535;
	WriteSparsePlotXY(&plt, pspa, 25000, 25535);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 65;
	plt.Tile_ID		= 535;
	WriteSparsePlotXY(&plt, pspa, 25535, 25535);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 65;
	plt.Tile_ID		= 0;
	WriteSparsePlotXY(&plt, pspa, 25000, 25535);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 64;
	plt.Tile_ID		= 0;
	WriteSparsePlotXY(&plt, pspa, 24000, 25535);

	plt.Plot_Flags	= 0;
	plt.TileSet_ID	= 30;
	plt.Tile_ID		= 0;
	WriteSparsePlotXY(&plt, pspa, 30000, 30000);

	ReadSparsePlotXY(pspa, 30000, 30000, &plt);
	printf("(30000, 30000):{%u,%u,%u}\n", plt.Plot_Flags, plt.TileSet_ID, plt.Tile_ID);

	ReadSparsePlotXY(pspa, 29999, 29999, &plt);
	printf("(29999, 29999):{%u,%u,%u}\n", plt.Plot_Flags, plt.TileSet_ID, plt.Tile_ID);

	ReadSparsePlotXY(pspa, 30000, 30001, &plt);
	printf("(30000, 30001):{%u,%u,%u}\n", plt.Plot_Flags, plt.TileSet_ID, plt.Tile_ID);

	PrintSparseArray(pspa);

	printf("\npspa2\n");
	PrintSparseArray(pspa2);

	printf("\nSwapping (1,1) w: 2, h:4\n");
	SwapRectInSparseArray(pspa, pspa2, 1, 1, 2, 4);

	PrintSparseArray(pspa);

	printf("\npspa2\n");
	PrintSparseArray(pspa2);

	printf("\nZeroing (1,1) w: 2, h:4\n");
	ZeroRectInSparseArray(pspa, 1, 1, 2, 4);

	PrintSparseArray(pspa);

	printf("\nDeleting 10000 rows from row 10\n");
	DeleteSparseArrayRows(pspa, 10, 10000);

	PrintSparseArray(pspa);

	FreeSparseArray(pspa);
	return;
}
#endif // 0
#endif // UseSparseArray

