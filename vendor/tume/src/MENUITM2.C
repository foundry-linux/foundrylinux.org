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
 * MENUITM2.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 05/11/93
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Overlaid menu selections for tUME.
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"
#include "switch1.h"

#include "events.h"
#include "tuglbl.h"
#include "xportmap.h"
#include "histogrm.h"
#include "numbers.h"
#include "maketile.h"
#include "sparsef.h"
#include "objroom.h"
#include "tumedraw.h"

/**************************** C O N S T A N T S ***************************/

#define QUERY_EXISTS	"Save over existing file?"

#define szMaxUsageTitle	"Tile Usage Count"
#define szMaxUsageString "Count limit:"

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


#if fDoSaveRooms
/*********************************************************************
 *
 * ExportScreen
 *
 * SYNOPSIS
 *		short ExportScreen(void)
 *
 * PURPOSE
 *		Export the current screen as an IFF file.
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
short ExportScreen(void)
{
	char *szFile;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ExportScreen";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetColorsNPointer (MainWindow);

	if ((szFile = GetFileName (MainWindow, EasyFR[PATH_PICTURES])) != NULL)
	{
		BOOL f;

		if ( (EIO_FileExists (szFile)) && (!(AreYouSure(QUERY_EXISTS))) )
		{
/**/		goto ABORT;
		}
		f = HidePointerSTATE;
		HidePointerSTATE = TRUE;
		RestoreColorsNPointer (MainWindow);	/* Hide pointer */
		HidePointerSTATE = f;
		if (!SaveByteMap(DisplayBM, MAXPLANES, GlobalColors->NumColors,
								ActivateXTRA(GlobalColors->pxtColors), szFile))
		{
			TellUser(NoCanDo, "Trouble saving picture.");
		}
		ReleaseXTRA(GlobalColors->pxtColors);
	}
ABORT:
	RestoreColorsNPointer (MainWindow);
	return TRUE;

} /* ExportScreen */


/*********************************************************************
 *
 * ExportRoom
 *
 * SYNOPSIS
 *		short ExportRoom(void)
 *
 * PURPOSE
 *		Export the current room as an IFF PBM file.
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
short ExportRoom(void)
{
	LayerType *play;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ExportRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom && GlobalRoomWindow->CurrentRoom->Room)
	{
		RoomType *prm = GlobalRoomWindow->CurrentRoom->Room;

		if ((play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer)) != NULL)
		{
			char *szFile;

			SetColorsNPointer (MainWindow);

			if ((szFile = GetFileName (MainWindow, EasyFR[PATH_PICTURES])) != NULL)
			{
				BOOL f;

				if ( (EIO_FileExists (szFile)) && (!(AreYouSure(QUERY_EXISTS))) )
				{
/**/				goto ABORT;
				}
				f = HidePointerSTATE;
				HidePointerSTATE = TRUE;
				RestoreColorsNPointer (MainWindow);	/* Hide pointer */
				HidePointerSTATE = FALSE;

				//BUGBUG: well, the wait pointer doesn't hang around for some reason

				DCC_TempSetPointer(BPI_WAIT_POINTER);
			  {
				//Weird bug in BCC 3.1 requires this convolution...

				BOOL f = SaveLayersAsByteMap(GlobalRoomWindow,
										ActivateXTRA(GlobalColors->pxtColors), szFile,
										&prm->Layers,
										prm->FloorLayer,
										play->ctilx,
										play->ctily,
#if NEWTUME
										TRUE,
#else
										(IsGuideVisible(prm)),
#endif
										(IsComposite(prm) && IsLocked(prm)));
				if (!f)
				{
					TellUser(NoCanDo, "Trouble saving picture.");
				}
			  }
				ReleaseXTRA(GlobalColors->pxtColors);
				HidePointerSTATE = f;
			}
		}
ABORT:
		RestoreColorsNPointer (MainWindow);
	}
	return TRUE;

} /* ExportRoom */


/*********************************************************************
 *
 * ExportBrush
 *
 * SYNOPSIS
 *		short ExportBrush(void)
 *
 * PURPOSE
 *		Export the current room as an IFF PBM file.
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
short ExportBrush(void)
{
	RoomWindowType *prw;
	LayerType *play;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ExportBrush";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalBlockCopy->SourceRW)
	{
		char *szFile;

		prw = GlobalBlockCopy->SourceRW;

		if (GlobalBlockCopy->DestRW && GlobalBlockCopy->DestRW->CurrentRoom &&
			GlobalBlockCopy->DestRW->CurrentRoom->Room)
		{
			RoomType *prm = GlobalBlockCopy->DestRW->CurrentRoom->Room;

			if ((play = FindNonEmptyLayer(&prm->Layers, prm->FloorLayer)) != NULL)
			{
				if (play->cxTile)
				{
					prw = GlobalBlockCopy->DestRW;
				}
			}
		}
		SetColorsNPointer (MainWindow);

		if ((play = FindNonEmptyLayer(&GlobalBlockCopy->Layers, GlobalBlockCopy->BC_FloorLayer)) == NULL)
/**/		goto ABORT;

		if ((szFile = GetFileName (MainWindow, EasyFR[PATH_PICTURES])) != NULL)
		{
			BOOL f;

			if ( (EIO_FileExists (szFile)) && (!(AreYouSure(QUERY_EXISTS))) )
			{
/**/			goto ABORT;
			}
			f = HidePointerSTATE;
			HidePointerSTATE = TRUE;
			RestoreColorsNPointer (MainWindow);	/* Hide pointer */
			HidePointerSTATE = FALSE;

			//BUGBUG: well, the wait pointer doesn't hang around for some reason

			DCC_TempSetPointer(BPI_WAIT_POINTER);
			if (!SaveLayersAsByteMap(prw,
									ActivateXTRA(GlobalColors->pxtColors), szFile,
									&GlobalBlockCopy->Layers,
									GlobalBlockCopy->BC_FloorLayer,
									play->ctilx,
									play->ctily,
									FALSE, FALSE))
			{
				TellUser(NoCanDo, "Trouble saving picture.");
			}
			ReleaseXTRA(GlobalColors->pxtColors);
			HidePointerSTATE = f;
		}
ABORT:
		RestoreColorsNPointer (MainWindow);
	}
	return TRUE;

} /* ExportBrush */
#else
short NoExport(void)
{
	TellUser (NoCanDo, "Export is disabled in this demo version.");
	return TRUE;
}
short ExportScreen(void)
{
	return NoExport();
}
short ExportRoom(void)
{
	return NoExport();
}
short ExportBrush(void)
{
	return NoExport();
}
#endif/*fDoSaveRooms*/


/*********************************************************************
 *
 * ToggleShowTileUsage
 *
 * PURPOSE
 *		Toggle show tile usage on/off.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Always TRUE.
 *
 * HISTORY
 *		07/11/93 Sunday (dcc) - created.
 *
*/
short ToggleShowTileUsage(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleShowTileUsage";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowRoom(GlobalRoomWindow);			/* either show usage, or erase 'em */
	return TRUE;
} /* ToggleShowTileUsage */


/*********************************************************************
 *
 * SetMaxTileUsage
 *
 * PURPOSE
 *		Set the largest tile usage that will be displayed.
 *
 * INPUT
 *		
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Always TRUE.
 *
 * HISTORY
 *		07/12/93 Monday (dcc) - created.
 *
*/
short SetMaxTileUsage(void)
{
	UWORD uw;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetMaxTileUsage";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	uw = GetLargestTileUsageCount();

	if (GetOneNumber(MainWindow, szMaxUsageTitle, szMaxUsageString, (short *) &uw))
	{
		SetLargestTileUsageCount(uw);

		if (!fShowTileUsage)
		{
			fShowTileUsage = TRUE;
			SetStateVar(&ToggleShowTileUsageState, *ToggleShowTileUsageState.StateVar);
		}
		ShowRoom(GlobalRoomWindow);			/* either show usage, or erase 'em */
	}
	return TRUE;
} /* SetMaxTileUsage */


/*********************************************************************
 *
 * ToggleShowGuide2
 *
 * PURPOSE
 *		Toggle whether or not to display other guides.
 *
 * INPUT
 *
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Always TRUE.
 *
 * HISTORY
 *		09/28/93 Tuesday (dcc) - created based on ToggleShowGuide().
 *
*/
short ToggleShowGuide2(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ToggleShowGuide2";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow->CurrentRoom && GlobalRoomWindow->CurrentRoom->Room)
	{
		RoomType *prm = GlobalRoomWindow->CurrentRoom->Room;

		if (prm->fShowGuide[2] || prm->fShowGuide[3])
		{
			prm->fShowGuide[2] = FALSE;
			prm->fShowGuide[3] = FALSE;
		}
		else
		{
			prm->fShowGuide[2] = TRUE;
			prm->fShowGuide[3] = TRUE;
		}
		ShowRoom (GlobalRoomWindow);
	}
	return (TRUE);

} /* ToggleShowGuide2 */


/*********************************************************************
 *
 * ClearLayerShift
 *
 * PURPOSE
 *		Clear the shift values for the current floor layer.
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
 *		02/20/94 Sunday (dcc) - created.
 *
*/
short ClearLayerShift(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ClearLayerShift";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom)
	{
		RoomType *prm = GlobalRoomWindow->CurrentRoom->Room;
		if (prm)
		{
			LayerType *play = prm->FloorLayer;

			if (!(play->L_Flags & LAYER_LOCKED))
			{
				play->dtilx = 0;
				play->dtily = 0;

				ShowRoom(GlobalRoomWindow);
				ShowState(LastTBar);
			}
		}
	}
	return TRUE;
} /* ClearLayerShift */


/*********************************************************************
 *
 * ClearAllLayerShifts
 *
 * PURPOSE
 *		Clear the shift values for all layers in the current room.
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
 *		02/20/94 Sunday (dcc) - created.
 *
*/
short ClearAllLayerShifts(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ClearAllLayerShifts";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalRoomWindow && GlobalRoomWindow->CurrentRoom)
	{
		RoomType *prm = GlobalRoomWindow->CurrentRoom->Room;
		if (prm)
		{
			LayerType *play = Head(&prm->Layers);

			while(!IsEOList(play))
			{
				if (!(play->L_Flags & LAYER_LOCKED))
				{
					play->dtilx = 0;
					play->dtily = 0;
				}
				play = Next(play);
			}
			ShowRoom(GlobalRoomWindow);
			ShowState(LastTBar);
		}
	}
	return TRUE;
} /* ClearAllLayerShifts */


/*********************************************************************
 *
 * TogglePixelSelect
 *
 * PURPOSE
 *		Menu item to toggle pixel-level tile-brush selection.
 *
 * INPUT
 *		NONE.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Always TRUE.
 *
 * HISTORY
 *		07/07/94 Thursday (dcc) - created.
 *
*/
short TogglePixelSelect(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TogglePixelSelect";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ShowRoom(GlobalRoomWindow);			/* either show pixel or tile select */
	return TRUE;
} /* TogglePixelSelect */

#if PixelLayers
/*********************************************************************
 *
 * MakeAnObjectTile
 *
 * PURPOSE
 *		Menu item to ask the user for an object tile and place it
 *		in the tile-brush.
 *
 * INPUT
 *		NONE.
 *
 * ASSUMES
 *		
 *
 * RETURN VALUE
 *		Always TRUE.
 *
 * HISTORY
 *		08/24/94 (dcc) - created.
 *
*/
int MakeAnObjectTile(void)
{
	PlotType plt = {0, 0, 0};
	BOOL fHid = FALSE;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "MakeAnObjectTile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GlobalBlockCopy != NULL && GlobalBlockCopy->BC_FloorLayer != NULL)
	{
		LayerType *play = GlobalBlockCopy->BC_FloorLayer;

		if (play->cxTile == 1 && play->cyTile == 1)
		{
			int itilx, itily;

			/* Find first non-zero tile */

			for (itily = 0; itily < play->ctily; itily++)
			{
				for (itilx = 0; itilx < play->ctilx; itilx++)
				{
					LAY_ReadPlotXY(play, itilx, itily, &plt);
					if (!fZeroTile(&plt))
/**/					goto FoundNonZero;
				}
			}
FoundNonZero:
		}

		HideBrush();
		fHid = TRUE;
	}

	if (MakeTileReq(&plt))
	{
		LayerType *play;

		Unselect();

		play = AddClearedLayerToLayers (&(GlobalBlockCopy->Layers), 1, 1, FALSE);
		if (!play)
/**/		goto ABORT;

		LAY_WritePlotXY(&plt, play, 0, 0);

		play->cxTile = 1;
		play->cyTile = 1;

		GlobalBlockCopy->LayerCount = 1;
		GlobalBlockCopy->BC_FloorLayer = play;

//		GlobalBlockCopy->SourceRW = &RW1x1;

		GlobalBlockCopy->SourceRW		= GlobalRoomWindow;
		GlobalBlockCopy->SourceStuff	= GlobalRoomWindow->CurrentRoom;
//		GlobalBlockCopy->SourceRoom	= GlobalRoomWindow->CurrentRoom->Room;
		GlobalBlockCopy->SourceRoom	= &RM1x1;

		GlobalBlockCopy->ctilxDisplay = 1;
		GlobalBlockCopy->ctilyDisplay = 1;

		GlobalBlockCopy->DestRW		= GlobalRoomWindow;
		GlobalBlockCopy->DestStuff	= GlobalRoomWindow->CurrentRoom;

		MButtonsE();
	}
	if (fHid)
		ShowBrush();

	SetPointerMode();
	return TRUE;
ABORT:
	return TRUE;
} /* MakeAnObjectTile */
#endif // PixelLayers

