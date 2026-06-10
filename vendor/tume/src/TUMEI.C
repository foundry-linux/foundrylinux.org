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
 * TUMEI.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 09/14/93
 *   MODIFIED : 02/09/95
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		tUME initialization stuff that can be jettisoned once executed.
 *
 * HISTORY
 *		09/14/93 Tuesday (dcc) - created (copied out of tUME.c).
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include "tuglbl.h"
#include "grid.h"
#include "layrtili.h"

#include <echidna/exit.h>

#if STACKCHECK
#include "stackchk.h"
#endif

extern void PrintFinalWord(void);
extern void TumeSetUpFirst (void);

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


#if NEWTUME
/*********************************************************************
 *
 * InitGuides
 *
 * PURPOSE
 *		Initialize guides, set to default values.
 *
 * INPUT
 *		NONE.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		FALSE if OOM.
 *
 * HISTORY
 *		09/14/93 Tuesday (dcc) - created.
 *
*/
static BOOL InitGuides(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitGuides";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((pcenDefaultGuideA = MEM_calloc(cDefaultGuideLines, sizeof(LONG))) == NULL)
		return FALSE;
	if ((pcenDefaultGuideB = MEM_calloc(cDefaultGuideLines, sizeof(LONG))) == NULL)
		return FALSE;
	if ((pcenDefaultGuideDC= MEM_calloc(cDefaultGuideLines, sizeof(LONG))) == NULL)
		return FALSE;
	if ((pcenDefaultGuideXOrigin = MEM_calloc(cDefaultGuideLines, sizeof(LONG))) == NULL)
		return FALSE;
	if ((pcenDefaultGuideYOrigin = MEM_calloc(cDefaultGuideLines, sizeof(LONG))) == NULL)
		return FALSE;
	if ((pcenDefaultGuideYOrigin = MEM_calloc(cDefaultGuideLines, sizeof(LONG))) == NULL)
		return FALSE;
	if ((fDefaultShowGuide = MEM_calloc(cDefaultGuideLines, sizeof(BOOL))) == NULL)
		return FALSE;

	pcenDefaultGuideA[0]			= cenDefaultGuideA0Init;
	pcenDefaultGuideB[0]			= cenDefaultGuideB0Init;
	pcenDefaultGuideDC[0]		= cenDefaultGuideDC0Init;
	pcenDefaultGuideXOrigin[0]	= cenDefaultGuideXOrigin0Init;
	pcenDefaultGuideYOrigin[0]	= cenDefaultGuideYOrigin0Init;
	pcenDefaultGuideA[1]			= cenDefaultGuideA1Init;
	pcenDefaultGuideB[1]			= cenDefaultGuideB1Init;
	pcenDefaultGuideDC[1]		= cenDefaultGuideDC1Init;
	pcenDefaultGuideXOrigin[1]	= cenDefaultGuideXOrigin1Init;
	pcenDefaultGuideYOrigin[1]	= cenDefaultGuideYOrigin1Init;

	return TRUE;
} /* InitGuides */
#endif


/**************************************************************************
 *
 * Initialize
 *
 * PURPOSE
 *
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
 *
 *
*/
void Initialize (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Initialize";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if __AMIGAOS__
	if (!(GfxBase = (struct GfxBase *) OpenLibrary ("graphics.library", LIB_REV)))
		Quit ("Can't open Graphics library.");

	if (!( IntuitionBase = (struct IntuitionBase *)OpenLibrary ("intuition.library", LIB_REV)))
		Quit ("Can't open Intuition library.");

	if (!(ArpBase = (struct ArpBase *) OpenLibrary(ArpName, ArpVersion)))
		Quit ("Can't open ARP library.");
#elif __MSDOS__

#if STACKCHECK
	AddExitFunc (StackUsed);
#endif
	AddExitFunc (PrintFinalWord);
#endif/*__AMIGAOS__/__MSDOS__*/

#if NEWTUME
	if (!InitGuides())
		Quit ("OOM");
#endif

	TumeSetUpFirst ();

#if __AMIGAOS__
	if (!(asfont = (struct TextFont *)OpenFont (&ASmallFont))) {
		if (!(asfont = InitFont ())) {
			Quit ("Couldn't install font");
		}
	} else {
		if (asfont->tf_YSize != 5) {
			CloseFont (asfont);
			asfont = NULL;
			if (!InitFont ()) {
				Quit ("Couldn't install font");
			}
		}
	}

	EchidnaFont = asfont;

	if (!(TopazFont = (struct TextFont *)OpenFont (&NormFont))) {
			Quit ("Couldn't allocate Topaz font");
	}

	if (!(InitKeys()))
		Quit ("Couldn't init key routines");

	if (!(Parse ("tume.key"))) 
		Quit ("Couldn't successfully read 'tume.key'");

	if (!( MainScreen = OpenScreen (&NewScreen)))
		Quit ("Couldn't open Main screen");

	TextTop	= (MainScreen->RastPort.TxBaseline) + 1;

	MainVp = &MainScreen->ViewPort;	/* GAT */

	NewWindow.Screen = MainScreen;
	SetColorRangeForReals (MainVp, 0, min(1L << DEPTH, 32),
		(void *) MainScreen->ViewPort.ColorMap->ColorTable);

	if (!InitColorSeqReq ())					/* GAT */
		Quit ("Couldn't init ColorSeqReq");		/* GAT */

	InitList (&GlobalColors->CycleList);
	ResetColorInfo (GlobalColors);
	UpdateGlobalColors ();
	InitColorVB ();

	if (!( MainWindow = OpenWindow (&NewWindow)))
		Quit ("Couldn't open window");

	MainRp = MainWindow->RPort;

	ActiveWindow = MainWindow;

	OurTask = (struct Process *)FindTask(0L);
	old_pr_WindowPtr = (struct Window *)OurTask->pr_WindowPtr;
	OurTask->pr_WindowPtr = (APTR)MainWindow;

	SetMenuStrip (MainWindow, MenuStrip);

	if (!(EasyFR = InitFileReqs (EasyFRI)))
		Quit ("Couldn't init file requester paths");
#endif/*__AMIGAOS__*/

#if __MSDOS__
	DefaultColorInfo.CycleList = &DefaultColorInfo.CycleListX;
	InitList (DefaultColorInfo.CycleList);
	DefaultColorInfo.EditCycle    = Head (DefaultColorInfo.CycleList);
	DefaultColorInfo.NumColors    = MAX_CI_COLORS;
	DefaultColorInfo.pxtColors	= AllocXTRA (MAX_CI_COLORS * sizeof (CS_Color));

	if (!DefaultColorInfo.pxtColors)
		Quit("OOXM");

	{
		UBYTE *pub = ActivateXTRA (DefaultColorInfo.pxtColors);
		memset (pub, 0, MAX_CI_COLORS * sizeof (CS_Color));
		UpdateXTRA (DefaultColorInfo.pxtColors);
	}
#endif/*__MSDOS__*/

	TumeSetUpLast ();
	OpenLayerType();

	return;

} /* Initialize */

