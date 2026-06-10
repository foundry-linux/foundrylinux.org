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
 * SPARSE.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 08/18/94
 *   MODIFIED : 10/20/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		typedef's for SPARSE.C.
 *
 * HISTORY
 *		08/18/94 Thursday (dcc) - created.
 *
*/

#ifndef SPARSE_H
#define SPARSE_H

#include <echidna/platform.h>
#include "switches.h"

#if UseSparseArray

#include "plottype.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

/* These are PRIVATE. */

typedef struct
{
										/* These next two UWORDS could be replaced by a
											32-bit pointer in a linear address machine. */

	UWORD		irgpBank;			/* index into array of pointers to 16K banks */
	UWORD		iBank;				/* offset within 16K bank */
} PSAE;								/* "pointer to Sparse Array Element" */

typedef struct
{
	PSAE NextRow;					/* pointer to next row in sparse array */

	int		dtily;				/* y-offset of current row */

	PSAE FirstCol;					/* pointer to 1st column in sparse array */
} SparseRowType;

typedef struct
{
	PlotType	plt;

	int		dtilx;				/* x-offset of current PlotType */

	PSAE NextCol;					/* pointer to next column in sparse array */
} SparsePlotType;

/* This is PUBLIC. */

typedef struct
{
	PSAE		psaeFirstRow;		// pointer to first sparse array entry

	PSAE		psaeLastRow;		// address of sprLast
	SparseRowType	sprLast;		// valid iff psaeLastRow.iBank != EndOfRow

	PSAE		psaeLastCol;		// address of splLast
	SparsePlotType	splLast;		// valid iff psaeLastCol.iBank != EndOfCol

	PSAE		psaeLimitRow;		//address of current row in array
	PSAE		psaeLimitCol;		//address of current column in array

	int		dtilxLimit, dtilyLimit, ctilxLimit, ctilyLimit;
} SparseArray;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/


#endif // UseSparseArray
#endif /* SPARSE_H */

