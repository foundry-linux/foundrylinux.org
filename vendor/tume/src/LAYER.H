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
 * LAYER.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 11/08/94
 *   MODIFIED : 11/08/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Functional prototypes for LAYER.C.
 *
 * HISTORY
 *		11/08/94 (dcc) - Created.
 *
*/

#ifndef LAYER_H
#define LAYER_H

#include <echidna/platform.h>
#include "switches.h"
#include "switch1.h"

#if UseSparseArray

#include "tuglbl.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

void SetLayerLimits(LayerType *play, int dtilx, int dtily, int ctilx, int ctily);
PlotType *GetNextPlotInRange(LayerType *play, int *pdtilx, int *pdtily);

#endif // UseSparseArray
#endif /* LAYER_H */

