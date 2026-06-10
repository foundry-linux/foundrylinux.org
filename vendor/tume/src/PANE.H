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
 * PANE.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 10/19/94
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Functional prototypes for PANE.C.
 *
 * HISTORY
 *		10/19/94 (dcc) - created.
 *
*/

#ifndef PANE_H
#define PANE_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/

#define NUM_PANES		3

#define PANE_PARTS	1
#define PANE_OBJECTS	2
#define PANE_EDIT		3

#define PANEF_OFF		(1U << 0)
#define PANEF_SPACED	(1U << 1)

/******************************** T Y P E S *******************************/

typedef struct {
	short		 Type;
	UWORD		 Flags;
	short		 ZeroX;
	short		 ZeroY;
	short		 DGX;
	short		 DGY;
//	DisplayGrid	*DGrid;
//	Object		*Object;
//	Frame		*OldFrame;
//	long		 OldFrameID;
	MouseFunc	 LeftFirstDown;
	MouseFunc	 LeftDown;
	MouseFunc	 LeftUp;
} Side;

typedef struct {
	HitArea		*HitArea;
	UWORD		 Number;
	Side		 P1;
	Side		 P2;
} Pane;

/****************************** G L O B A L S *****************************/

extern HitArea		 GlobalAreas[];

extern Pane			*Pane1;
extern Pane			*Pane2;
extern Pane			*Pane3;
extern Pane			*ActivePane;
extern Pane			*Panes[];

/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

void DrawTitleBar (void);
void ActivatePane (Pane *p);
short ProcessPane (HitInfo *hi);

#endif /* PANE_H */

