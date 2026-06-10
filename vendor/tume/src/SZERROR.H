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
 * SZERROR.H
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 07/31/93
 *   MODIFIED : 07/28/94
 *       TABS : 04 07
 *
 * DESCRIPTION
 *		Consolidated error messages.
 *		Someday, all error message will be here...
 *
 * HISTORY
 *		07/31/93 Saturday (dcc) - created.
 *
*/

#ifndef SZERROR_H
#define SZERROR_H

#include <echidna/platform.h>

/**************************** C O N S T A N T S ***************************/

#define NCD "Error"
#define OOM "Out of Memory Error"

#define szerrBadRange				"Range is out of bounds."
#define szerrCantWritePal			"Couldn't create palette file."
#define szerrNoPalRange				"Please select a palette range."
#define szerrPalNotUp				"Color requester not active."
#define szerrUnique					"Please enter a unique room name."

#endif /* SZERROR_H */

