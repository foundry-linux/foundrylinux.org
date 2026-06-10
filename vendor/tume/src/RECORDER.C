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
 * RECORDER.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 09/16/92
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
 *		Routines to record and restore mouse and keyboard events from
 *		a text file.
 *
 *		Currently, we save three event types:
 *		M xxxxx yyyyy b						- mouse x, y, button state
 *		K sssss									- keyboard scancode [GetKeyNoWait()]
 *		T aaaaa									- user typed [GetKey()]
 *
 *		We process the following addition events:
 *		H<message title>~<message>			- show box with help information
 *		R											- restart script from beginning
 *
 *		We do not currently process ShiftFlags() (used in conjunction with
 *		reading the mouse).
 *
 * HISTORY
 *		03/19/93 Friday (dcc) - when playing back, if user hits [ESC],
 *							call raise(SIGTERM) to stop program immediately.
 *
*/

#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <echidna/platform.h>
#include "switches.h"

#include "keyboard.h" //<echidna/keyboard.h>
#include <echidna/mouse.h>
#include <echidna/recorder.h>
#include <echidna/timer.h>

#include "tuglbl.h"
#include "ibmmouse.h"
#include "message.h"

/**************************** C O N S T A N T S ***************************/

#define szLengthMax 256

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

static FILE *hFilIn = NULL;
static FILE *hFilOut = NULL;

static char szLast[szLengthMax];
static char fLastLine = FALSE;

static WORD hTim;
static volatile UWORD uwWait = 0;

static BOOL fDontWait = FALSE;

static short wEarlyExit = -1;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * SaveInputLocation
 *
 * SYNOPSIS
 *		static void SaveInputLocation(void)
 *
 * PURPOSE
 *		One level only "push" of where to get input.
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
static void SaveInputLocation(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveInputLocation";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SaveMouseLocation();
	SaveKeyboardLocation();

} /* SaveInputLocation */


/*********************************************************************
 *
 * SetInputFromUser
 *
 * SYNOPSIS
 *		static void SetInputFromUser(void)
 *
 * PURPOSE
 *		Set state so we actually read mouse and keyboard.
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
static void SetInputFromUser(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetInputFromUser";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetMouseEvents(fFromUser);
	SetKeyboardEvents(fFromUser);

} /* SetInputFromUser */


/*********************************************************************
 *
 * RestoreInputLocation
 *
 * SYNOPSIS
 *		static void RestoreInputLocation(void)
 *
 * PURPOSE
 *		One level only "pop" of where to get input from.
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
static void RestoreInputLocation(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RestoreInputLocation";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	RestoreMouseLocation();
	RestoreKeyboardLocation();

} /* RestoreInputLocation */


/*********************************************************************
 *
 * SetEarlyExitTime
 *
 * SYNOPSIS
 *		void SetEarlyExitTime(short w)
 *
 * PURPOSE
 *		Set the time to wait before automatically exiting a text help
 *		box put up by the recorder.
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
void SetEarlyExitTime(short w)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetEarlyExitTime";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	wEarlyExit = w;

} /* SetEarlyExitTime */


/*********************************************************************
 *
 * OpenEventRecorder
 *
 * SYNOPSIS
 *		short OpenEventRecorder(char *szFile, short f)
 *
 * PURPOSE
 *		Open the file <szFile> for reading if <f> == fReadEvents,
 *		or for writing if <f> == fSaveAllEvents or == fSaveSomeEvents.
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
short OpenEventRecorder(char *szFile, short f)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "OpenEventRecorder";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (f & fReadFast)
		fDontWait = TRUE;

	f &= 0x7F;

	SetMouseEvents(f);
	SetKeyboardEvents(f);

	if (f == fReadEvents)
	{
		if ((hFilIn = fopen(szFile, "rt")) == NULL)
		{
			return FALSE;
		}
	}
	else if (f == fSaveAllEvents || f == fSaveSomeEvents)
	{
		if ((hFilOut = fopen(szFile, "wt")) == NULL)
		{
			return TRUE;
		}
	}
	OpenTimers();
	hTim = InstallTimer(&uwWait, 60, sizeof(uwWait), -1);
	return TRUE;
} /* OpenEventRecorder */


/*********************************************************************
 *
 * CloseEventRecorder
 *
 * SYNOPSIS
 *		void CloseEventRecorder(void)
 *
 * PURPOSE
 *		Close the event recorder.
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
void CloseEventRecorder(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CloseEventRecorder";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (hFilIn)
		fclose(hFilIn);

	if (hFilOut)
		fclose(hFilOut);

} /* CloseEventRecorder */


/*********************************************************************
 *
 * ShowHelp
 *
 * SYNOPSIS
 *		void ShowHelp(char *sz)
 *
 * PURPOSE
 *		Show user the string <sz>.
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
void ShowHelp(char *sz)
{

	char *szTitle;
	char *szText;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ShowHelp";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((szTitle = strchr(sz, '~')) != NULL)
	{
		*szTitle = 0;
		szText = szTitle+1;
		szTitle = sz;
	}
	else
	{
		szText = sz;
		szTitle = strchr(sz, 0);
	}

	SaveInputLocation();
	SetInputFromUser();

	AllowTimedExit(wEarlyExit);
	InitKeyList();
	AddKeyOption(32, 1);						/* SPACE returns option 1 */
//	AddKeyOption(13, 1);						/* ENTER returns 1, TellUser() adds this */
	AddKeyOption(27, 2);						/* ESCAPE returns option 2 */
	if (TellUser(szTitle, szText) == 2)
	{
		AllowTimedExit(-1);					/* Don't allow timed exit */
		QuitAndExit();		if (ExitProgram)	raise(SIGTERM);
	}
	AllowTimedExit(-1);						/* Don't allow timed exit */

	RestoreInputLocation();

} /* ShowHelp */


/*********************************************************************
 *
 * ReadEvent
 *
 * SYNOPSIS
 *		short ReadEvent(char *sz)
 *
 * PURPOSE
 *		Read next string from event file. If <sz> doesn't match
 *		the beginning of the string from the event file, return
 *		FALSE. Else copy the string into <sz> and return TRUE.
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
short ReadEvent(char *sz)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReadEvent";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!fLastLine)
	{
		short fAgain = TRUE;

		if (!hFilIn)
			return FALSE;

		while (fAgain)
		{
			/* See if ESC pressed, ask user if he wants to quit. */

			{
				UWORD code;

				SaveInputLocation();
				SetInputFromUser();

				if ((code = GetKeyNoWait()) != 0)
				{
					if ((code = ConvertKeyToAscii(code)) == 0x1B) /* ESC */
					{
						AllowTimedExit(-1);	/* Don't allow timed exit */
						QuitAndExit();		if (ExitProgram)	raise(SIGTERM);
					}
				}

				RestoreInputLocation();
			}

			if (fgets(szLast, szLengthMax, hFilIn) == NULL)
			{
				ExitProgram = TRUE;
				fAgain = FALSE;
			}

			if (*szLast == 'H')
			{
				ShowHelp(szLast+1);
			}
			else if (*szLast == 'R')
			{
				rewind(hFilIn);
			}
			else
				fAgain = FALSE;
		}
	}

	if (strncmp(sz, szLast, strlen(sz)) == 0)
	{
		fLastLine = FALSE;
		strcpy(sz, szLast);

		if (hTim && !fDontWait)
		{
			while (uwWait) {}

			ResetTimer(hTim, 1);
		}
		return TRUE;
	}
	else
	{
		fLastLine = TRUE;
		return FALSE;
	}
} /* ReadEvent */


/*********************************************************************
 *
 * SaveEvent
 *
 * SYNOPSIS
 *		void SaveEvent(char *sz)
 *
 * PURPOSE
 *		Save string to the event file.
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
void SaveEvent(char *sz)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SaveEvent";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (hFilOut)
		fputs(sz, hFilOut);

	if (hTim)
	{
		while (uwWait) {}

		ResetTimer(hTim, 1);
	}
} /* SaveEvent */

