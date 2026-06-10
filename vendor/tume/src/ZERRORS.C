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
 * ERRORS.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 01/09/91
 *   MODIFIED : 10/19/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Routines to deal with errors?
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/eerrors.h>
#include <echidna/eui.h>

#include "misc.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * SysError1
 *
 * SYNOPSIS
 *		void SysError1 (char *msg, char *arg1)
 *
 * PURPOSE
 *		Show an error to the user?
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
void SysError1 (char *msg, char *arg1)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SysError1";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

#if !ERRORS
	msg	= msg;		/* Turn off warnings */
	arg1 = arg1;	/* Turn off warnings */
#endif

	{
		GEcatf("\n");
		GEcatf1 (msg, arg1);
		SetColorsNPointer (MainWindow);
		Message (MainWindow, "Error:", GlobalErrMsg, " Boo Hoo ");
		RestoreColorsNPointer (MainWindow);
		ClearGlobalErr ();
	}
} /* SysError1 */

