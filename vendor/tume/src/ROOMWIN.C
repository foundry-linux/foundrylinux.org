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
 * ROOMWINDOW.C
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 10/04/89
 *   MODIFIED : 10/02/94
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *		Create, delete, and manage roomwindow structures.
 *
 * HISTORY
 *		10/04/89 Wednesday - Created. (RGM)
*/
#include <echidna/platform.h>
#include "switches.h"

#include "switch1.h"				// this must precede <echidna/alloc.h>
#include <echidna/alloc.h>

#include "tuglbl.h"
#include "szerror.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** E X T E R N S *****************************/


/****************************** G L O B A L S *****************************/


/******************************* L O C A L S ******************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/**************************************************************************
 *
 * DeAllocateRoomWindow
 *
 * SYNOPSIS
 *		void DeAllocateRoomWindow (RoomWindowType *roomwindow)
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
 *		10/04/89 Wednesday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void DeAllocateRoomWindow (RoomWindowType *roomwindow)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DeAllocateRoomWindow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (roomwindow) {
		MEM_free (roomwindow);
	}
	return;

} /* DeAllocateRoomWindow */



/**************************************************************************
 *
 * CreateRoomWindow
 *
 * SYNOPSIS
 *		RoomWindowType *CreateRoomWindow (struct Window *window)
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
 *		10/04/89 Wednesday - Created. (RGM)
 *
 * SEE ALSO
 *
*/

RoomWindowType *CreateRoomWindow (struct Window *window)
{
	RoomWindowType	*newroomwindow = NULL;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "CreateRoomWindow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (window) {
		if ( (newroomwindow = MEM_calloc (1, ROOMWINDOWSIZE)) == NULL)
		{
/**/		goto ABORT;
		}
		else
		{
			newroomwindow->Window			= window;
			newroomwindow->SourceRoom.Flags	=
							(DISPLAY_SEPERATED | SOURCE_LOCK);
			newroomwindow->EditRoom.Flags		=
							(DISPLAY_FLUSH | EDIT_LOCK);
			newroomwindow->CurrentRoom	= &(newroomwindow->SourceRoom);
			newroomwindow->OtherRoom		= &(newroomwindow->EditRoom);
			CurrentX = CurrentY = 0;
		}
	}
	return (newroomwindow);

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
ABORT:

	if (newroomwindow) {
		DeAllocateRoomWindow (newroomwindow);
		newroomwindow = NULL;
	}
	return (NULL);

} /* CreateRoomWindow */


/*********************************************************************
 *
 * TogglePrw
 *
 * SYNOPSIS
 *		void TogglePrw(RoomWindowType *prw)
 *
 * PURPOSE
 *		Toggle <prw> to the other "pane".
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
void TogglePrw(RoomWindowType *prw)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TogglePrw";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (prw)
	{
		if (prw->CurrentRoom == (&(prw->SourceRoom)))
		{
			prw->CurrentRoom = (&prw->EditRoom);
			prw->OtherRoom = (&prw->SourceRoom);
		}
		else
		{
			prw->CurrentRoom = (&prw->SourceRoom);
			prw->OtherRoom = (&prw->EditRoom);
		}
	}

} /* TogglePrw */


/**************************************************************************
 *
 * HookRoomWindow
 *
 * SYNOPSIS
 *		BOOL HookRoomWindow (
 *			RoomType		*room,
 *			RoomWindowType	*roomwindow
 *		)
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
 *		10/05/89 Thursday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
BOOL HookRoomWindow (
	RoomType		*room,
	RoomWindowType	*roomwindow
)
{
	RWLockType	*rwlock;
	RWInitsType	*rwinit;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "HookRoomWindow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (room)
	{
		if ( (rwlock = MEM_calloc (1, RWLOCKSIZE)) == NULL)
/**/		goto ABORT;

		rwlock->RoomWindow	= roomwindow;
		rwlock->RoomStuff	= roomwindow->CurrentRoom;

		AddTail ( &(room->Displays), rwlock);
	}

	/***********************************************/
	/* search room->RWinits & GlobalRoomWindowList */
	/* for a match                                 */

	if ((room) && (! IsEmpty (&room->RWInits))){
		/** cheat for now **/
		rwinit = Head(&room->RWInits); /* get match to roomwindow */
		roomwindow->CurrentRoom->dxSrc	= rwinit->dxSrcPrev;
		roomwindow->CurrentRoom->dySrc	= rwinit->dySrcPrev;
	}
	else {
		roomwindow->CurrentRoom->dxSrc	= 0;
		roomwindow->CurrentRoom->dySrc	= 0;
	}

	roomwindow->CurrentRoom->Room		= room;
	CurrentX = CurrentY = 0;

	return (TRUE);
/*------------------------------------------------------------------------*/
ABORT:
	if (rwlock) {
		MEM_free (rwlock);
		rwlock = NULL;
	}

	return (FALSE);

} /* HookRoomWindow */



/**************************************************************************
 *
 * UnHookRoomWindow
 *
 * SYNOPSIS
 *		void UnHookRoomWindow (
 *			RoomType		*room,
 *			RoomWindowType	*roomwindow
 *		)
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
 *		10/05/89 Thursday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void UnHookRoomWindow (
	RoomType		*room,
	RoomWindowType	*roomwindow
)
{
	RWLockType	*rwlock;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "UnHookRoomWindow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (room) {
		rwlock = Head (&(room->Displays));
		while ( (! IsEOList (rwlock)) && (rwlock->RoomWindow != roomwindow) ) {
			rwlock = Next (rwlock);
		}
		if (! IsEOList (rwlock)) {
			Remove (rwlock);
			MEM_free (rwlock);
		}
		rwlock = NULL;
	}

	if (roomwindow) {
		roomwindow->CurrentRoom->Room		= NULL;
		roomwindow->CurrentRoom->dxSrc	= 0;
		roomwindow->CurrentRoom->dySrc	= 0;
		CurrentX = CurrentY = 0;
	}

	return;

} /* UnHookRoomWindow */


/**************************************************************************
 *
 * TotalRoomUnhook
 *
 * SYNOPSIS
 *		void TotalRoomUnhook (
 *			RoomType		*room,
 *			RoomWindowType	*roomwindow
 *		)
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
 *		12/07/89 Thursday - Created out of UnHookRoomWindow. (RGM)
 *
 * SEE ALSO
 *
*/
void TotalRoomUnhook (
	RoomType		*room,
	RoomWindowType	*roomwindow
)
{
	RWLockType	*rwlock;
	RWLockType	*next_rwlock;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TotalRoomUnhook";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	SetGlobalColors(&DefaultColorInfo);	/*dcc*//*BUG FIX*/

	if (room) {
		rwlock = Head (&(room->Displays));
		while ( (! IsEOList (rwlock))) {
			next_rwlock = Next (rwlock);
			Remove (rwlock);
			MEM_free (rwlock);
			rwlock = next_rwlock;
		}
	}

	if (roomwindow) {
		CurrentX = CurrentY = 0;
		roomwindow->CurrentRoom->Room		= NULL;
		roomwindow->CurrentRoom->dxSrc	= 0;
		roomwindow->CurrentRoom->dySrc	= 0;
	}
	return;

} /* TotalRoomUnhook */


/**************************************************************************
 *
 * TryToHookRoomWindow
 *
 * SYNOPSIS
 *		void TryToHookRoomWindow (
 *			RoomType		*room,
 *			RoomWindowType	*roomwindow
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *		TryToHookRoomWindow (NEWROOM, ROOMWINDOW);
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
void TryToHookRoomWindow (
	RoomType		*room,
	RoomWindowType	*roomwindow
)
{
	RoomType		*oldroom;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TryToHookRoomWindow";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (room && roomwindow) {
		oldroom = roomwindow->CurrentRoom->Room;
		if (room) {
			UnHookRoomWindow (oldroom, roomwindow);
			if (! HookRoomWindow (room, roomwindow)) {
				TellUser (OOM, "Memory critical, reverting to last display.");
			}
		}
#if 0
		UndoRoom =
			ReMakeUndoRoom (GlobalRoomWindow->CurrentRoom->Room, UndoRoom);
#endif
	}
	return;

} /* TryToHookRoomWindow */



/**************************************************************************
 *
 * SameTypeRoom
 *
 * SYNOPSIS
 *		BOOL SameTypeRoom (
 *			RoomType		*room,
 *			RoomStuffType	*roomstuff
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *		ok = SameTypeRoom (ROOM_PTR, ROOMSTUFF_PTR);
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
 *		11/12/89 Sunday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
BOOL SameTypeRoom (
	RoomType		*room,
	RoomStuffType	*roomstuff
)
{
	BOOL status = FALSE;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SameTypeRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (room && roomstuff) {
		if (IS_SOURCE_ROOM (room)) {
			if (! IS_EDIT_LOCKED (roomstuff)) {
				status = TRUE;
			}
		}
		else {
			if (! IS_SOURCE_LOCKED (roomstuff)) {
				status = TRUE;
			}
		}
	}
	return (status);

} /* SameTypeRoom */


/**************************************************************************
 *
 * FriendlyShowRoom
 *
 * SYNOPSIS
 *		void FriendlyShowRoom (
 *			RoomType		*room,
 *			RoomWindowType	*roomwindow
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *		FriendlyShowRoom (ROOM_PTR, ROOMWINDOW_PTR);
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
 *		11/12/89 Sunday - Created. (RGM)
 *
 * SEE ALSO
 *
*/
void FriendlyShowRoom (
	RoomType		*room,
	RoomWindowType	*roomwindow
)
{
	RoomStuffType *tempRS;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FriendlyShowRoom";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (room && roomwindow) {
		if (SameTypeRoom (room, roomwindow->CurrentRoom)) {
		}
		else if (SameTypeRoom (room, roomwindow->OtherRoom)) {
			tempRS = roomwindow->CurrentRoom;
			roomwindow->CurrentRoom = roomwindow->OtherRoom;
			roomwindow->OtherRoom = tempRS;
		}
		else {
/**/		goto ABORT;
		}

		TryToHookRoomWindow (room, roomwindow);

		if (LastTBar == SHOW_TITLE) {
			ShowState (SHOW_DEFAULT);
		}
		else {
			ShowState (LastTBar);
		}
		ShowRoom (GlobalRoomWindow);
	}
ABORT:
	return;

} /* FriendlyShowRoom */

