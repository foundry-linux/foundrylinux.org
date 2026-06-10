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
 * MCOPYREC.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 10/19/94
 *   MODIFIED : 10/19/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Function prototypes for MCOPYREC.ASM.
 *
 * HISTORY
 *		10/19/94 (dcc) - created.
 *
*/

#ifndef MCOPYREC_H
#define MCOPYREC_H

#include <echidna/platform.h>
#include "switches.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

far32 MCGA_MaskedCopyTransRect (ByteMap *sbm, int fx, int fy,
						  ByteMap *dbm, int tx, int ty,
						  int width, int height);

far32 MCGA_ScaledMaskedCopyTransRect (
		ByteMap	*src,
		int	 fromx,
		int	 fromy,
		ByteMap	*dest,
		LONG	 tox,
		LONG	 toy,
		int	 wDstWidth,
		int	 wDstHeight,
		int	 wSrcSkipX,
		int	 wDstDupX,
		int	 wSrcSkipY,
		int	 wDstDupY);

#endif /* MCOPYREC_H */

