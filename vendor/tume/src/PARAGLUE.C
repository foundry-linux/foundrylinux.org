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
 * PARAGLUE.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 02/17/93
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		"High"-level parallel port communication routines to read and
 *		write a block of memory, with handshaking.
 *
 * HISTORY
 *		04/14/93 Wednesday (dcc) - make fInitParallel() actually check 4 target.
 *
*/

#include "switch2.h"
#if dvpPARALLEL

#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/timer.h>
#include <echidna/etypes.h>
#include "parallel.h"
#include "paraglue.h"						/* Verify function prototypes. */

/**************************** C O N S T A N T S ***************************/

#define uwMaxBlockSize 32768U				/* arbitrary largest block to send */

#define cmdWrite	0
#define cmdRead	1
#define cmdID		0x7F
#define cmdRestart 0xFE
#define cmdReset	0xFF

#define bit6 (1<<6)

#define cntToggleMax 9						/* Toggle bit6 this many times */

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

static UWORD wTimeOut = 0;

/******************************* M A C R O S ******************************/

#define fSendCommand(ub)	ParallelSendByte(ub)

/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * fTargetAvailable
 *
 * SYNOPSIS
 *		static BOOL fTargetAvailable(void)
 *
 * PURPOSE
 *		Toggle bit 6. See if target toggles bit 6 as well.
 *		Repeat several times. Return TRUE if bit 6 changes match.
 *
 *		The timer library must have been opened already.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns TRUE if target is active.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL fTargetAvailable(void)
{
	WORD hTim;
	WORD cntToggle;
	UBYTE ubOut = 1;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fTargetAvailable";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!ParallelPortAddress)
		return FALSE;				/* BIOS sez no printer port at specified LPTn! */

	hTim = InstallTimer(&wTimeOut, 60, sizeof(wTimeOut), -1);
	ResetTimer(hTim, 45);

	for (cntToggle = 0; cntToggle < cntToggleMax && wTimeOut; cntToggle++)
	{
		if (ubOut & bit6)
		{
			ubOut -= bit6;
			outp(ParallelPortAddress, ubOut);
			while (((inp(ParallelPortAddress+1) & bit6) != 0) && wTimeOut)
				;
		}
		else
		{
			ubOut += bit6;
			outp(ParallelPortAddress, ubOut);
			while (((inp(ParallelPortAddress+1) & bit6) == 0) && wTimeOut)
				;
		}
	}

	RemoveTimer(hTim);

	return (cntToggle >= cntToggleMax);

} /* fTargetAvailable */


/*********************************************************************
 *
 * fEstablishComm
 *
 * SYNOPSIS
 *		static BOOL fEstablishComm(void)
 *
 * PURPOSE
 *		Get the attention of the machine on the target parallel port.
 *		BUGBUG dcc 02/17/93 - should do some sort of timeout check.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if unable to establish communications.
 *		BUGBUG dcc 02/17/93 - Ha! It just locks up for now...
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL fEstablishComm(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fEstablishComm";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	do
	{
		ParallelSendByte(0xDC);
	} while (ParallelGetByte() != 0xAC);

	return TRUE;

} /* fEstablishComm */


/*********************************************************************
 *
 * fSendUWORD
 *
 * SYNOPSIS
 *		static BOOL fSendUWORD(UWORD uw)
 *
 * PURPOSE
 *		Send <uw> to parallel port, low byte first, then high.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if unable to send UWORD.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL fSendUWORD(UWORD uw)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fSendUWORD";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!ParallelSendByte((uw      ) & 0xff))
		return FALSE;
	if (!ParallelSendByte((uw >>  8) & 0xff))
		return FALSE;

	return TRUE;

} /* fSendUWORD */


/*********************************************************************
 *
 * fSendULONG
 *
 * SYNOPSIS
 *		static BOOL fSendULONG(ULONG ul)
 *
 * PURPOSE
 *		Send <ul> to parallel port as 4 bytes, low byte to high.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if unable to send ULONG.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static BOOL fSendULONG(ULONG ul)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fSendULONG";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!ParallelSendByte((ul      ) & 0xff))
		return FALSE;
	if (!ParallelSendByte((ul >>  8) & 0xff))
		return FALSE;
	if (!ParallelSendByte((ul >> 16) & 0xff))
		return FALSE;
	if (!ParallelSendByte((ul >> 24) & 0xff))
		return FALSE;

	return TRUE;

} /* fSendULONG */


/*********************************************************************
 *
 * fParallelBlockRead
 *
 * SYNOPSIS
 *		BOOL fParallelBlockRead(ULONG ulAddress, UBYTE *pub, ULONG ulLength)
 *
 * PURPOSE
 *		Read <ulLength> bytes from target machine address <ulAddress>
 *		to <pub>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if unable to read block.
 *		BUGBUG: Ha! when reading block, returns TRUE (or just locks up).
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL fParallelBlockRead(ULONG ulAddress, void *pub, ULONG ulLength)
{

	UWORD uwLength, uw;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fParallelBlockRead";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	while (ulLength)
	{
		uwLength = min(ulLength, (ULONG) uwMaxBlockSize);

		if (!fEstablishComm())
			return FALSE;

		if (!fSendCommand(cmdRead))
			return FALSE;

		if (!fSendULONG(ulAddress))
			return FALSE;

		if (!fSendUWORD(uwLength))
			return FALSE;

		for (uw = 0; uw < uwLength; uw++)
		{
			*((UBYTE *)pub)++ = ParallelGetByte();
		}
		ulAddress += uwLength;
		ulLength -= uwLength;
	}
	return TRUE;

} /* fParallelBlockRead */


/*********************************************************************
 *
 * fParallelBlockWrite
 *
 * SYNOPSIS
 *		BOOL fParallelBlockWrite(ULONG ulAddress, UBYTE *pub, ULONG ulLength)
 *
 * PURPOSE
 *		Send <ulLength> bytes starting from <pub> to target machine
 *		at address <ulAddress>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if unable to write block.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL fParallelBlockWrite(ULONG ulAddress, void *pub, ULONG ulLength)
{

	UWORD uwLength, uw;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fParallelBlockWrite";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	while (ulLength)
	{
		uwLength = min(ulLength, (ULONG) uwMaxBlockSize);

		if (!fEstablishComm())
			return FALSE;

		if (!fSendCommand(cmdWrite))
			return FALSE;

		if (!fSendULONG(ulAddress))
			return FALSE;

		if (!fSendUWORD(uwLength))
			return FALSE;

		for (uw = 0; uw < uwLength; uw++)
		{
			if (!ParallelSendByte(*((UBYTE *)pub)++))
				return FALSE;
		}
		ulAddress += uwLength;
		ulLength -= uwLength;
	}
	return TRUE;

} /* fParallelBlockWrite */


/*********************************************************************
 *
 * wGetParallelID
 *
 * SYNOPSIS
 *		WORD wGetParallelID(void)
 *
 * PURPOSE
 *		Returns target machine identification word.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns target machine ID word, or -1 if unsuccessful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
WORD wGetParallelID(void)
{

	WORD wLO, wHI;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "wGetParallelID";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!fEstablishComm())
		return -1;

	if (!fSendCommand(cmdID))
		return -1;

	if ((wLO = ParallelGetByte()) == -1)
		return -1;

	if ((wHI = ParallelGetByte()) == -1)
		return -1;

	return (wHI << 8) | wLO;

} /* wGetParallelID */


/*********************************************************************
 *
 * fRestartParallelTarget
 *
 * SYNOPSIS
 *		BOOL fRestartParallelTarget(void)
 *
 * PURPOSE
 *		Warm restart the target program.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if unsuccessful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL fRestartParallelTarget(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fRestartParallelTarget";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (!fEstablishComm())
		return FALSE;

	if (!fSendCommand(cmdRestart))
		return FALSE;

	return TRUE;
} /* fRestartParallelTarget */


/*********************************************************************
 *
 * fInitParallel
 *
 * SYNOPSIS
 *		BOOL fInitParallel(void)
 *
 * PURPOSE
 *		Initialize parallel communications.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns FALSE if unable to initialize communications, or
 *		if target is unavailable (or not attached).
 *
 * HISTORY
 *		04/14/93 Wednesday (dcc) - make procedure actually check for target.
 *
 * SEE ALSO
 *
*/
BOOL fInitParallel(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "fInitParallel";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fTargetAvailable())
	{
		ParallelPrevControl = inp(ParallelPortAddress+2) & 0x1F;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
} /* fInitParallel */


/*********************************************************************
 *
 * SetLPTn
 *
 * SYNOPSIS
 *		void SetLPTn(WORD wPort)
 *
 * PURPOSE
 *		Set parallel download port address to <wPort>.
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
void SetLPTn(WORD wPort)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SetLPTn";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	/* Peek into BIOS data area to get parallel port address */

	ParallelPortAddress = peek(0x40,wPort*2+6);

} /* SetLPTn */

#endif/*dvpPARALLEL*/

