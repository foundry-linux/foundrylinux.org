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
 * COLORSEQ.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 02/21/92
 *   MODIFIED : 01/22/95
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *
 *		Note: You will need to link with MWRTCRNG.ASM
 *		
 *
 * HISTORY
 *		04/07/92 (dcc) - Enhanced to store palettes (CS_ColorInfo,
 *						 which are currently 2K big) in XTRA memory.
 *		05/02/92 (dcc) - CopyColorInfoToColorInfo() - only make copy
 *						 of CS_CycleInfo, instead of entire ColorInfo
 *						 structure. This means the program is less
 *						 likely to crash under low memory conditions.
 *		08/03/92 (dcc) - Change ComputeNextPalette() so it does NOT
 *						 allocate CS_Color rgcsc[MAX_CI_COLORS] on
 *						 the stack. The stack could have been an
 *						 interrupt stack, which would be bad, as 2K
 *						 was most likely not available.
 *		10/14/93 (dcc) - Updated for MS-DOS Watcom C32.
 *		01/22/95 (dcc) - Make module lint with no errors.
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include <echidna/eerrors.h>
#include <echidna/listfunc.h>
#include <echidna/timer.h>
#include <stdio.h>

#include "colorseq.h"

#define LOCAL static

/******************** E X T E R N A L   R O U T I N E S *******************/

extern InterruptRoutineType MCGA_ASM_WriteColors (void);
extern void InitMenuColors (ColorInfo *pColorInfo);

/**************************** C O N S T A N T S ***************************/
#define MAX_COPY_RANGES					16

/******************************** T Y P E S *******************************/

typedef struct {
    UWORD	 NumColors; 
    void	*ColorPtr;  
    UWORD	 ColorReg;  
} CopyRangeType;

/****************************** G L O B A L S *****************************/

extern int				 ColorStuffPause;
extern int				 ActivationFlag;
extern CopyRangeType	*CopyRanges;
short					 ColorRange[3] = { 256, 256, 256, };
short					 ColorMax[3]   = { 255, 255, 255, };
short					 VBCycle;
short					 CyclePause;
ColorInfo				*GlobalColors;
LOCAL ColorInfo			*pcinLast = NULL;
LOCAL ColorInfo			*pcinGlobal = NULL;
LOCAL ColorInfo			*pcinLastSave;
LOCAL ColorInfo			*pcinGlobalSave;

LOCAL short				 OldPause;

LOCAL CopyRangeType		 CopyRange[MAX_COPY_RANGES + 1];

LOCAL WORD				 CSIntHndl;
LOCAL WORD				 CRIntHndl;

LOCAL BOOL				 fCreateColorInfoInXTRA = TRUE;

CSEQ_Color				 GlobalPalette[MAX_CI_COLORS];

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

#if __TURBOC__
#pragma option -N-
#elif __WATCOMC__
#pragma off (check_stack);
#endif

/*********************************************************************
 *
 * ComputeColorRanges
 *
 * SYNOPSIS
 *		void ComputeColorRanges (CSEQ_Color *cseqc, short force)
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
InterruptRoutineType ComputeColorRanges (CSEQ_Color *cseqc, short force)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ComputeColorRanges";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		ColorStuffPause = TRUE;
				
		CopyRange[0].NumColors = 0;

		if (!force) {
			CopyRangeType	*crt;
			CSEQ_Color		*cs;
			short			 i;
			short			 start;
			short			 crcnt;

			i     = 0;
			crcnt = 0;
			cs    = cseqc;
			crt   = CopyRange;

			while (i < MAX_CI_COLORS) {
				if (cs->Flags & (PAL_CHANGED | PAL_UPDATE)) {
					if (crcnt == MAX_COPY_RANGES) {
						force = TRUE;
						break;
					}
					start = i;
					cs++;
					i++;
					while (cs->Flags && i < MAX_CI_COLORS) {
						i++;
						cs++;
					}
					crt->NumColors = i - start;
					crt->ColorPtr  = &cseqc[start];
					crt->ColorReg  = start;
					crcnt++;
					crt++;
					
				}
				cs++;
				i++;
			}

			crt->NumColors = 0;
		}

		if (force) {
			CopyRange[0].NumColors = MAX_CI_COLORS;
			CopyRange[0].ColorPtr  = cseqc;
			CopyRange[0].ColorReg  = 0;
			CopyRange[1].NumColors = 0;
		}

		ActivationFlag = TRUE;

		{
			CSEQ_Color		*dstcseqc;
			short			 i;
			UBYTE			 flag;

			dstcseqc = cseqc;
			for (i = 0; i < MAX_CI_COLORS; i++) {
				flag = dstcseqc->Flags;
				if (flag & PAL_CHANGED) {
					flag = (flag & (~PAL_CHANGED)) | PAL_UPDATE;
					dstcseqc->Flags = flag;
				}
				dstcseqc++;
			}
		}

		ColorStuffPause = FALSE;

	}
} /* ComputeColorRanges */

/*********************************************************************
 *
 * ComputeNextPalette
 *
 * SYNOPSIS
 *		void ComputeNextPalette (CSEQ_Color *cseqc, ColorInfo *ci, short force)
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
InterruptRoutineType ComputeNextPalette (CSEQ_Color *cseqc, ColorInfo *ci, short force)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ComputeNextPalette";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		if (force /* || !ci->CycleFlag */) {
			CSEQ_Color		*dstcseqc;
			CS_Color		*srccsc;
			short			 i;

			dstcseqc = cseqc;
#if __MSDOS16__
			if (XmtFromPxtr (ci->pxtColors) == xmtSTD) {
#endif
				srccsc = ActivateXTRA (ci->pxtColors);	// already in STD RAM

				/* No need for matching ReleaseXTRA because it's STD RAM */
#if __MSDOS16__
			} else {
				srccsc = MEM_malloc (MAX_CI_COLORS * sizeof (CS_Color));
				ReadXTRA(ci->pxtColors, 0,				// read into new buffer
								MAX_CI_COLORS * sizeof (CS_Color), srccsc);
			}
#endif

			for (i = 0; i < MAX_CI_COLORS; i++) {
				UWORD	 temp;
				short	 cval;

				cval = BOUNDCOLOR(srccsc->Red);
				temp = ((UWORD)cval * (UWORD)ColorRange[0] / (UWORD)MAX_COLOR_RANGE);
				dstcseqc->Red    = (UWORD)temp * (UWORD)MAX_HARDWARE_COLOR / (UWORD)ColorMax[0];
				cval = BOUNDCOLOR(srccsc->Green);
				temp = ((UWORD)cval * (UWORD)ColorRange[1] / (UWORD)MAX_COLOR_RANGE);
				dstcseqc->Green  = (UWORD)temp * (UWORD)MAX_HARDWARE_COLOR / (UWORD)ColorMax[1];
				cval = BOUNDCOLOR(srccsc->Blue);
				temp = ((UWORD)cval * (UWORD)ColorRange[2] / (UWORD)MAX_COLOR_RANGE);
				dstcseqc->Blue   = (UWORD)temp * (UWORD)MAX_HARDWARE_COLOR / (UWORD)ColorMax[2];
				dstcseqc->Flags |= PAL_CHANGED;
				dstcseqc++;
				srccsc++;
			}

#if __MSDOS16__
			if (XmtFromPxtr (ci->pxtColors) != xmtSTD) {
				MEM_free (srccsc);
			}
#endif
		}

		if (!CyclePause && ci->CycleFlag) {
			CS_CycleInfo	*csci;

			csci = Head (ci->CycleList);
			while (!EndOfList (csci)) {

				if (csci->Flags & CI_ENABLED) {
					short	movit = FALSE;

					csci->Timer += csci->Speed;
					if (csci->Timer >= ONEPERTICK) {
						csci->Timer -= ONEPERTICK;
						movit = TRUE;
					}
					
					if (movit || force) {
						CS_ColorTracker	*csct;
						CS_RegTracker	*csrt;

						csrt = Head (csci->RegList);
						csct = csci->CurrentColor;

						while (!EndOfList (csrt)) {
							if (!EndOfList (csct)) {
								CSEQ_Color	*cs;

								cs = &cseqc[csrt->Reg];
								if (cs->Flags & PAL_CHANGED) {
									short	cval;

									cval       = BOUNDCOLOR(csct->Color.Red);
									cs->Red    = (cs->Red   + cval) >> 1;
									cval       = BOUNDCOLOR(csct->Color.Green);
									cs->Green  = (cs->Green + cval) >> 1;
									cval       = BOUNDCOLOR(csct->Color.Blue);
									cs->Blue   = (cs->Blue  + cval) >> 1;
								} else {
									UWORD	temp;
									short	cval;

									cval = BOUNDCOLOR(csct->Color.Red);
									temp = ((UWORD)cval * (UWORD)ColorRange[0] / (UWORD)MAX_COLOR_RANGE);
									cs->Red    = (UWORD)temp * (UWORD)MAX_HARDWARE_COLOR / (UWORD)ColorMax[0];
									cval = BOUNDCOLOR(csct->Color.Green);
									temp = ((UWORD)cval * (UWORD)ColorRange[1] / (UWORD)MAX_COLOR_RANGE);
									cs->Green  = (UWORD)temp * (UWORD)MAX_HARDWARE_COLOR / (UWORD)ColorMax[1];
									cval = BOUNDCOLOR(csct->Color.Blue);
									temp = ((UWORD)cval * (UWORD)ColorRange[2] / (UWORD)MAX_COLOR_RANGE);
									cs->Blue   = (UWORD)temp * (UWORD)MAX_HARDWARE_COLOR / (UWORD)ColorMax[2];
									#if 0
									cs->Red    = csct->Color.Red   * ColorRange[0] / MAX_HARDWARE_COLOR;
									cs->Green  = csct->Color.Green * ColorRange[1] / MAX_HARDWARE_COLOR;
									cs->Blue   = csct->Color.Blue  * ColorRange[2] / MAX_HARDWARE_COLOR;
									#endif
									cs->Flags |= PAL_CHANGED;
								}

								csct = Next (csct);
								if (EndOfList (csct)) {
									csct = Head (csci->ColorList);
								}
							}
							csrt = Next (csrt);
						}

						if (movit && !EndOfList (csci->CurrentColor)) {
							if (csci->Direction == 1) {
								csci->CurrentColor = Next (csci->CurrentColor);
								if (EndOfList (csci->CurrentColor)) {
									csci->CurrentColor = Head (csci->ColorList);
								}
							} else {
								csci->CurrentColor = Prev (csci->CurrentColor);
								if (StartOfList (csci->CurrentColor)) {
									csci->CurrentColor = Tail (csci->ColorList);
								}
							}
						}
					}
				}

				csci = Next (csci);
			}
		}

		ComputeColorRanges (cseqc, force);
	}
} /* ComputeNextPalette */

/*********************************************************************
 *
 * UpdateCurrentColorseq
 *
 * SYNOPSIS
 *		LOCAL InterruptTimerType UpdateCurrentColorseq (void)
 *
 * PURPOSE
 *		Updates a ColorInfo
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
LOCAL InterruptRoutineType UpdateCurrentColorseq (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateCurrentColorseq";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (VBCycle && !CyclePause) {
		ComputeNextPalette (GlobalPalette, GlobalColors, CyclePause != OldPause);
	}
	OldPause = CyclePause;

} /* UpdateCurrentColorseq */

#if __TURBOC__
#pragma option -N.
#endif

/*********************************************************************
 *
 * CloseColorseqInterrupt
 *
 * SYNOPSIS
 *		void CloseColorseqInterrupt (void)
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
void CloseColorseqInterrupt (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CloseColorseqInterrupt";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (CRIntHndl)	RemoveVBlank (CRIntHndl);
	if (CSIntHndl)	RemoveInterrupt (CSIntHndl);

} /* CloseColorseqInterrupt */

/*********************************************************************
 *
 * OpenColorseqInterrupt
 *
 * SYNOPSIS
 *		short OpenColorseqInterrupt (void)
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
short OpenColorseqInterrupt (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenColorseqInterrupt";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	CopyRanges = CopyRange;
	CSIntHndl  = InstallInterrupt (UpdateCurrentColorseq, COLORSEQ_HERTZ);
	if (CSIntHndl) {
		CRIntHndl = InstallVBlank (MCGA_ASM_WriteColors);
		if (CRIntHndl) {
			return TRUE;
		}
	}
	CloseColorseqInterrupt ();
	return FALSE;

} /* OpenColorseqInterrupt */

/*********************************************************************
 *
 * RenumberCS_Colors
 *
 * SYNOPSIS
 *		void RenumberCS_Colors (ListType *ColorList)
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
void RenumberCS_Colors (ListType *ColorList)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RenumberCS_Colors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		CS_ColorTracker	*csct;
		short			 ID = 0;

		csct = Head (ColorList);
		while (!EndOfList (csct)) {
			csct->ID = ID++;
			csct = Next (csct);
		}
	}
} /* RenumberCS_Colors */

/*********************************************************************
 *
 * RenumberCS_Regs
 *
 * SYNOPSIS
 *		void RenumberCS_Regs (ListType *RegList)
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
void RenumberCS_Regs (ListType *RegList)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RenumberCS_Regs";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		CS_RegTracker	*csrt;
		short			 ID = 0;

		csrt = Head (RegList);
		while (!EndOfList (csrt)) {
			csrt->ID = ID++;
			csrt = Next (csrt);
		}
	}
} /* RenumberCS_Regs */

/*********************************************************************
 *
 * RenumberCycleInfos
 *
 * SYNOPSIS
 *		void RenumberCycleInfos (ListType *CycleList)
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
void RenumberCycleInfos (ListType *CycleList)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RenumberCycleInfos";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		CS_CycleInfo	*csci;
		short			 ID = 0;

		csci = Head (CycleList);
		while (!EndOfList (csci)) {
			csci->ID = ID++;
			csci = Next (csci);
		}
	}
} /* RenumberCycleInfos */

/*********************************************************************
 *
 * RenumberColorInfo
 *
 * SYNOPSIS
 *		void RenumberColorInfo (ColorInfo *ci)
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
void RenumberColorInfo (ColorInfo *ci)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RenumberColorInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		CS_CycleInfo	*csci;

		RenumberCycleInfos (ci->CycleList);
		csci = Head (ci->CycleList);
		while (!EndOfList (csci)) {
			RenumberCS_Regs (csci->RegList);
			RenumberCS_Colors (csci->ColorList);
			csci = Next (csci);
		}
		
	}
} /* RenumberColorInfo */

/*********************************************************************
 *
 * DeleteRegTracker
 *
 * SYNOPSIS
 *		void DeleteRegTracker (CS_RegTracker *csrt)
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
void DeleteRegTracker (CS_RegTracker *csrt)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteRegTracker";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	DeleteNode (csrt);

} /* DeleteRegTracker */

/*********************************************************************
 *
 * CreateRegTracker
 *
 * SYNOPSIS
 *		CS_RegTracker *CreateRegTracker (void)
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
CS_RegTracker *CreateRegTracker (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateRegTracker";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return CreateNode (sizeof (CS_RegTracker), NULL);

} /* CreateRegTracker */

/*********************************************************************
 *
 * DeleteCycleColor
 *
 * SYNOPSIS
 *		void DeleteCycleColor (CS_ColorTracker *csct)
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
void DeleteCycleColor (CS_ColorTracker *csct)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteCycleColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	DeleteNode (csct);

} /* DeleteCycleColor */

/*********************************************************************
 *
 * CreateCycleColor
 *
 * SYNOPSIS
 *		CS_ColorTracker *CreateCycleColor (void)
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
CS_ColorTracker *CreateCycleColor (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateCycleColor";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return CreateNode (sizeof (CS_ColorTracker), NULL);

} /* CreateCycleColor */

/*********************************************************************
 *
 * DeleteCycleInfo
 *
 * SYNOPSIS
 *		void DeleteCycleInfo (CS_CycleInfo *csci)
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
void DeleteCycleInfo (CS_CycleInfo *csci)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteCycleInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	if (csci) {
		CS_ColorTracker	*csct;
		CS_RegTracker	*csrt;

		while ((csct = RemTail (csci->ColorList)) != NULL) {
			DeleteCycleColor (csct);
		}

		while ((csrt = RemTail (csci->RegList)) != NULL) {
			DeleteRegTracker (csrt);
		}

		DeleteNode (csci);
	}
} /* DeleteCycleInfo */

/*********************************************************************
 *
 * CreateCycleInfo
 *
 * SYNOPSIS
 *		CS_CycleInfo	*CreateCycleInfo (void)
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
CS_CycleInfo	*CreateCycleInfo (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateCycleInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		CS_CycleInfo	*csci;

		csci = CreateNode (sizeof (CS_CycleInfo), NULL);
		if (csci) {
			csci->ColorList = &csci->ColorListX;
			InitList (csci->ColorList);
			csci->CurrentColor = Head (csci->ColorList);
			csci->EditColor    = Head (csci->ColorList);

			csci->RegList = &csci->RegListX;
			InitList (csci->RegList);
			csci->EditReg = Head (csci->RegList);

			csci->Direction = 1;
			csci->Flags     = CI_ENABLED;
		}

		return csci;
	}
} /* CreateCycleInfo */

/*********************************************************************
 *
 * EmptyColorInfo
 *
 * SYNOPSIS
 *		LOCAL EmptyColorInfo (ColorInfo *ci)
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
LOCAL void EmptyColorInfo (ColorInfo *ci)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "EmptyColorInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (ci->pxtColors) {
		FreeXTRA (ci->pxtColors);
	}
	{
		CS_CycleInfo	*csci;

		while ((csci = RemTail (ci->CycleList)) != NULL) {
			DeleteCycleInfo (csci);	
		}
	}
} /* EmptyColorInfo */

/*********************************************************************
 *
 * InitColorInfo
 *
 * SYNOPSIS
 *		void InitColorInfo (ColorInfo *ci)
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
void InitColorInfo (ColorInfo *ci)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitColorInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		CS_CycleInfo	*csci;

		ResetColorInfo (ci);
		RenumberColorInfo (ci);

		ci->EditCycle = Head (ci->CycleList);
		csci = Head (ci->CycleList);
		while (!EndOfList (csci)) {
			csci->EditColor = Head (csci->ColorList);
			csci->EditReg   = Head (csci->RegList);
			csci = Next (csci);
		}
	}
} /* InitColorInfo */

/*********************************************************************
 *
 * DeleteColorInfo
 *
 * SYNOPSIS
 *		void DeleteColorInfo (ColorInfo	*ci)
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
void DeleteColorInfo (ColorInfo	*ci)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeleteColorInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		if (ci) {
			EmptyColorInfo (ci);
			MEM_free (ci);
		}
	}

} /* DeleteColorInfo */

/*********************************************************************
 *
 * CreateColorInfo
 *
 * SYNOPSIS
 *		ColorInfo *CreateColorInfo (void)
 *
 * PURPOSE
 *		Alloc and initialize a ColorInfo.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns NULL if OOM.
 *
 * HISTORY
 *		03/17/93 Wednesday (dcc) - fill with initial colors (instead of black).
 *
 * SEE ALSO
 *
*/
ColorInfo *CreateColorInfo (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateColorInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		ColorInfo	*ci;

		ci = MEM_calloc (1, sizeof (ColorInfo));
		if (ci) {
			ci->CycleList = &ci->CycleListX;
			InitList (ci->CycleList);
			ci->EditCycle	= Head (ci->CycleList);
			ci->NumColors	= MAX_CI_COLORS;

			if (fCreateColorInfoInXTRA)
				ci->pxtColors = AllocXTRA (MAX_CI_COLORS * sizeof (CS_Color));
			else
				ci->pxtColors = AllocXTRAofXMT (MAX_CI_COLORS * sizeof (CS_Color), xmtSTD);

			if (ci->pxtColors == NULL) {
				MEM_free (ci);
				ci = NULL;
			} else {
#if 1
				InitMenuColors (ci);
#else
				CS_Color rgcsc[MAX_CI_COLORS];

				memset (rgcsc, 0, MAX_CI_COLORS * sizeof (CS_Color));
				WriteXTRA (rgcsc, MAX_CI_COLORS * sizeof (CS_Color),
							ci->pxtColors, 0);
#endif
			}
		}

		return ci;
	}
} /* CreateColorInfo */

/*********************************************************************
 *
 * ResetColorInfo
 *
 * SYNOPSIS
 *		void ResetColorInfo (ColorInfo *ci)
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
void ResetColorInfo (ColorInfo *ci)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ResetColorInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		CS_CycleInfo	*csci;

		csci = Head (ci->CycleList);
		while (!EndOfList (csci)) {
			csci->Timer = 0;
			csci->CurrentColor = Head (csci->ColorList);
			csci = Next (csci);
		}
	}
} /* ResetColorInfo */

/*********************************************************************
 *
 * CopyColorInfoToColorInfo
 *
 * SYNOPSIS
 *		short CopyColorInfoToColorInfo (ColorInfo *src, ColorInfo *dest)
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
 *		TRUE if successful.
 *
 * HISTORY
 *		05/02/92 Saturday (dcc) - only make copy of CS_CycleInfo.
 *
 * SEE ALSO
 *
*/
short CopyColorInfoToColorInfo (ColorInfo *src, ColorInfo *dest)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CopyColorInfoToColorInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		ListType cycNewDst;
		CS_CycleInfo	*srccsci;
		CS_CycleInfo	*dstcsci;

		/* Make a copy of all the cycle stuff, tack in onto <cycNewDst> */

		InitList(&cycNewDst);

		srccsci = Head (src->CycleList);
		while (!EndOfList (srccsci)) {
			dstcsci = CreateCycleInfo ();
			if (!dstcsci) {
/**/			goto cleanup;
			}
			AddTail (&cycNewDst, dstcsci);
			dstcsci->Speed     = srccsci->Speed    ;
			dstcsci->Direction = srccsci->Direction;
			dstcsci->Flags     = srccsci->Flags    ;
			dstcsci->ID        = srccsci->ID       ;
			dstcsci->NumColors = srccsci->NumColors;
			dstcsci->NumRegs   = srccsci->NumRegs  ;
			{
				CS_RegTracker	*srcrt;
				CS_RegTracker	*dstrt;

				srcrt = Head (srccsci->RegList);
				while (!EndOfList (srcrt)) {
					dstrt = CreateRegTracker ();
					if (!dstrt) {
/**/					goto cleanup;
					}
					*dstrt = *srcrt;
					AddTail (dstcsci->RegList, dstrt);
					srcrt = Next (srcrt);
				}
			}
			{
				CS_ColorTracker	*srcct;
				CS_ColorTracker	*dstct;

				srcct = Head (srccsci->ColorList);
				while (!EndOfList (srcct)) {
					dstct = CreateCycleColor ();
					if (!dstct) {
/**/					goto cleanup;
					}
					*dstct = *srcct;
					AddTail (dstcsci->ColorList, dstct);
					srcct = Next (srcct);
				}
			}
			srccsci = Next (srccsci);
		}
		/* Delete old cycle stuff from <dest> */
		{
			CS_CycleInfo	*csci;

			while ((csci = RemTail (dest->CycleList)) != NULL) {
				DeleteCycleInfo (csci);	
			}
		}

		/* Attach new copy of cycle stuff to <dest> */

		dest->CycleList = &dest->CycleListX;
		InitList (dest->CycleList);
		ConcatLists (dest->CycleList, &cycNewDst);
		dest->EditCycle = Head (dest->CycleList);

		/* Update other variables in <dest> */

		dest->NumColors = src->NumColors;
		dest->CycleFlag = src->CycleFlag;
		dest->NumCycles = src->NumCycles;

		/* Copy palette to <dest> */
		{
			CS_Color	 rgcsc[MAX_CI_COLORS];

			ReadXTRA(src->pxtColors, 0,				// read onto stack
								MAX_CI_COLORS * sizeof (CS_Color), rgcsc);

			WriteXTRA(rgcsc, MAX_CI_COLORS * sizeof (CS_Color),
						dest->pxtColors, 0);
		}
		InitColorInfo (dest);

		return TRUE;

cleanup:
		{
			CS_CycleInfo	*csci;

			while ((csci = RemTail (&cycNewDst)) != NULL) {
				DeleteCycleInfo (csci);	
			}
		}

		return FALSE;
	}
} /* CopyColorInfoToColorInfo */

/*********************************************************************
 *
 * DuplicateColorInfo
 *
 * SYNOPSIS
 *		ColorInfo *DuplicateColorInfo (ColorInfo *ci)
 *
 * PURPOSE
 *		Make a copy of <ci> in standard or XTRA memory.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns NULL if OOM.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
ColorInfo *DuplicateColorInfo (ColorInfo *ci)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DuplicateColorInfo";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		ColorInfo	*newci;

		newci = CreateColorInfo ();
		if (newci) {
			if (CopyColorInfoToColorInfo (ci, newci)) {
				return newci;
			}
			DeleteColorInfo (newci);
		}
		return NULL;
	}

} /* DuplicateColorInfo */


/*********************************************************************
 *
 * DuplicateColorInfoInSTD
 *
 * PURPOSE
 *		Make a copy of <ci> in standard memory.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Returns NULL if OOM.
 *
 * HISTORY
 *		08/06/93 Friday (dcc) - created.
 *
*/
ColorInfo *DuplicateColorInfoInSTD (ColorInfo *ci)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DuplicateColorInfoInSTD";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	{
		ColorInfo	*ciDup;
		BOOL f = fCreateColorInfoInXTRA;

		fCreateColorInfoInXTRA = FALSE;
		ciDup = DuplicateColorInfo(ci);
		fCreateColorInfoInXTRA = f;

		return ciDup;
	}
} /* DuplicateColorInfoInSTD */



/*********************************************************************
 *
 * ConvertRGBtoHSV
 *
 * SYNOPSIS
 *		void ConvertRGBtoHSV (CS_Color *cs)
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
void ConvertRGBtoHSV (CS_Color *cs)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ConvertRGBtoHSV";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		long	 r, g, b;
		long	r1,g1,b1;
		long	 h, s, v;
		long	 m, t;

		r = BOUNDCOLOR(cs->Red);
		g = BOUNDCOLOR(cs->Green);
		b = BOUNDCOLOR(cs->Blue);

		v = max (r, max (g, b));
		m = min (r, min (g, b));
		t = v - m;
		if (v) {
			s = (t * MAX_HARDWARE_COLOR) / v;
		} else {
			s = 0;
		}
		if (s) {
			r1 = ((v - r) * 256) / t;
			g1 = ((v - g) * 256) / t;
			b1 = ((v - b) * 256) / t;
			if (v == r) {
				if (m == g) {
					h = 5 * 256 + b1;
				} else {
					h = 1 * 256 - g1;
				}
			} else if (v == g) {
				if (m == b) {
					h = 1 * 256 + r1;
				} else {
					h = 3 * 256 - b1;
				}
			} else {
				if (m == r) {
					h = 3 * 256 + g1;
				} else {
					h = 5 * 256 - r1;
				}
			}
			h = (h * 360) / 1536;
		} else {
			h = cs->Hue;
		}
		cs->Hue        = (short) h;
		cs->Saturation = (short) s;
		cs->Value      = (short) v;
	}
} /* ConvertRGBtoHSV */

/*********************************************************************
 *
 * ConvertHSVtoRGB
 *
 * SYNOPSIS
 *		void ConvertHSVtoRGB (CS_Color *cs)
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
void ConvertHSVtoRGB (CS_Color *cs)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ConvertHSVtoRGB";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	{
		long	i;
		long	f;
		long	p1,p2,p3;
		long	 r, g, b;
		long	 h, s, v;

		h  = BOUNDCOLORh(cs->Hue);
		s  = BOUNDCOLOR(cs->Saturation);
		v  = BOUNDCOLOR(cs->Value);

		h  = h * MAX_HARDWARE_COLOR;
		i  = h / (60 * MAX_HARDWARE_COLOR);
		f  = (h % (60 * MAX_HARDWARE_COLOR)) / 60;
		p1 = (v * (MAX_HARDWARE_COLOR - s)) / MAX_HARDWARE_COLOR;
		p2 = (v * (MAX_HARDWARE_COLOR - (s * f) / MAX_HARDWARE_COLOR)) / MAX_HARDWARE_COLOR;
		p3 = (v * (MAX_HARDWARE_COLOR - (s * (MAX_HARDWARE_COLOR - f)) / MAX_HARDWARE_COLOR)) / MAX_HARDWARE_COLOR;
		switch (i) {
		case 0:
			r = v;
			g = p3;
			b = p1;
			break;
		case 1:
			r = p2;
			g = v;
			b = p1;
			break;
		case 2:
			r = p1;
			g = v;
			b = p3;
			break;
		case 3:
			r = p1;
			g = p2;
			b = v;
			break;
		case 4:
			r = p3;
			g = p1;
			b = v;
			break;
		case 5:
			r = v;
			g = p1;
			b = p2;
			break;
		}

		cs->Red   = (short) r;
		cs->Green = (short) g;
		cs->Blue  = (short) b;

	}
} /* ConvertHSVtoRGB */


/*********************************************************************
 *
 * UpdateGlobalColors
 *
 * SYNOPSIS
 *		void UpdateGlobalColors(void)
 *
 * PURPOSE
 *		Copy <pcinGlobal> to <pcinLast> if it is non-NULL.
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
void UpdateGlobalColors(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UpdateGlobalColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (pcinLast != NULL)
	{
		CopyColorInfoToColorInfo(pcinGlobal, pcinLast);
	}
} /* UpdateGlobalColors */


/*********************************************************************
 *
 * SetGlobalColors
 *
 * SYNOPSIS
 *		BOOL SetGlobalColors(ColorInfo *pcin)
 *
 * PURPOSE
 *		Call this routine to set <GlobalColors> instead of setting
 *		it directly yourself. This routine does the following:
 *
 *		1. Copy <pcinGlobal> to <pcinLast> if it is non-NULL.
 *		2. Make a copy of <pcin>. Make sure that <pcinGlobal->pxtColors>
 *		   resides in standard memory.
 *		3. Set <pcinLast> = <pcin> so next call to this routine
 *		   updates GlobalColors.
 *
 *		The reason that <pcinGlobal->pxtColors> MUST reside in standard
 *		memory is so that we can change it in colorreq.c without
 *		calling UpdateXTRA(), and so the color-cycling interrupt routine
 *		can get the info it needs without paging in XTRA memory.
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
BOOL SetGlobalColors(ColorInfo *pcin)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetGlobalColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	UpdateGlobalColors();

	fCreateColorInfoInXTRA = FALSE;

	if (!pcinGlobal)
		pcinGlobal = CreateColorInfo();

	if (!pcinGlobal)
		return FALSE;

	if (!CopyColorInfoToColorInfo(pcin, pcinGlobal))
	{
		fCreateColorInfoInXTRA = TRUE;
		return FALSE;
	}
	fCreateColorInfoInXTRA = TRUE;

	pcinLast = pcin;

	GlobalColors = pcinGlobal;
	return TRUE;

} /* SetGlobalColors */


/*********************************************************************
 *
 * PushGlobalColors
 *
 * SYNOPSIS
 *		void PushGlobalColors(void)
 *
 * PURPOSE
 *		One level 'push' for saving GlobalColors.
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
void PushGlobalColors(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PushGlobalColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	pcinLastSave	= pcinLast;
	pcinGlobalSave	= pcinGlobal;

	pcinLast			= NULL;
	pcinGlobal		= NULL;	/* cause SetGlobalColors() to allocate new ColorInfo */

} /* PushGlobalColors */


/*********************************************************************
 *
 * PopGlobalColors
 *
 * SYNOPSIS
 *		void PopGlobalColors(void)
 *
 * PURPOSE
 *		'Pop' the one level push of GlobalColors;
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
void PopGlobalColors(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PopGlobalColors";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	DeleteColorInfo(pcinGlobal);
	pcinLast	= pcinLastSave;
	pcinGlobal	= pcinGlobalSave;

	GlobalColors = pcinGlobal;

} /* PopGlobalColors */



/*********************************************************************
 *
 * IsGlobalColorsCopyOf
 *
 * SYNOPSIS
 *		BOOL IsGlobalColorsCopyOf(ColorInfo *pcin)
 *
 * PURPOSE
 *		See if last call to SetGlobalColors was with <pcin>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		TRUE if last SetGlobalColors was on <pcin>.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL IsGlobalColorsCopyOf(ColorInfo *pcin)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "IsGlobalColorsCopyOf";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return pcinLast == pcin;

} /* IsGlobalColorsCopyOf */

