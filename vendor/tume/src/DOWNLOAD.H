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
 * DOWNLOAD.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 02/05/92
 *   MODIFIED : 07/28/94
 *       TABS : 04 07
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Header file for download routines.
 *
 * HISTORY
 *
*/

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

/* SetDownloadTarget() takes one of the values below.
	NOTE: these MUST correspond with SNASM?????_IDs found in snasmio.h! */

#define fGenesis	0
#define fSNES		1

/****************************** G L O B A L S *****************************/

extern BOOL fEnableDownload;
extern BOOL fDownloadHardwareAvail;
extern WORD wSCSIAddress;

/******************************* M A C R O S ******************************/

#define fSetDownloadHardwareAvailable(f)	fDownloadHardwareAvail = f
#define fGetDownloadHardwareAvailable()	(fDownloadHardwareAvail)

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern BOOL InitDownloader(void);
extern void SetDownloadTarget(WORD f);
extern void GrabNDownloadPalette(ColorInfo *ci);
extern BOOL DoDownloadMap(TileSpaceType *ptsp, RoomType *prt,
									WORD wXrt, WORD wYrt, WORD wColors);

#endif /* DOWNLOAD_H */
