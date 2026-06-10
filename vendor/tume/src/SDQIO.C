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
 * SDQIO.C
 *
 * PROGRAMMER : Dan Chang
 *    VERSION : 00.000
 *    CREATED : 04/02/92
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
 *		(Mostly) fast sliding dictionary quick file I/O routine.
 *		Instead of calling read(), call InitSDQRead(), then SDQRead().
 *		Instead of calling write(), call InitSDQWrite(), SDQWrite(),
 *		then SDQWriteLast().
 *
 *		The SDQ format is similar to LZSS format, except we use a
 *		5-bit offset and a 3-bit length, and encode both into one byte.
 *
 * HISTORY
 *		01/16/93 Saturday (dcc) - don't include save stuff if compile
 *							time variable fSaveCompressed == 0.
 *
*/

#include <echidna/platform.h>
#include "switches.h"
#include "switch1.h"

#include <io.h>
#include <string.h>
#include <stdlib.h>

#include "sdqio.h"

/**************************** C O N S T A N T S ***************************/

#define FALSE 0
#define TRUE 1

/* Only read in nine bytes at a time, so we don't overrun the packed data. */

#define cBytesInMac (9)
#define cBytesOutMac (64)

#define MAXOFFSET 31	/* maximum offset that can be encoded (2^5-1) */
#define N			64	/* size of ring buffer */
#define F		   10	/* upper limit for match_length */
#define THRESHOLD	2  /* encode string into position and length
						   if match_length is greater than this */
#define NIL			N	/* index for root of binary search trees */

/******************************** T Y P E S *******************************/

typedef int BOOL;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/* COMPRESS */

#if fSaveCompressed
static unsigned char
		text_buf[N + F - 1];	/* ring buffer of size N,
			with extra F-1 bytes to facilitate string comparison */
static int match_position, match_length,  /* of longest match.  These are
			set by the InsertNode() procedure. */
		lson[N + 1], rson[N + 257], dad[N + 1];  /* left & right children &
			parents -- These constitute binary search trees. */

static void InitTree(void)  /* initialize trees */
{
	int  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = N + 1; i <= N + 256; i++) rson[i] = NIL;
	for (i = 0; i < N; i++) dad[i] = NIL;
}

/* Inserts string of length F, text_buf[r..r+F-1], into one of the
	trees (text_buf[r]'th tree) and returns the longest-match position
	and length via the global variables match_position and match_length.
	If match_length = F, then removes the old node in favor of the new
	one, because the old one will be deleted sooner.
	Note r plays double role, as tree node and position in buffer. */

static void InsertNode(int r)
{
	int  i, p, cmp;
	unsigned char  *key;

	cmp = 1;  key = &text_buf[r];  p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;  match_length = 0;
	for ( ; ; )
	{
		unsigned char *pub;

		if (cmp >= 0)
		{
			if (rson[p] != NIL)
			{
				p = rson[p];
				pub = &text_buf[p];
			}
			else
			{
				rson[p] = r;  dad[r] = p;  return;
			}
		}
		else
		{
			if (lson[p] != NIL)
			{
				p = lson[p];
				pub = &text_buf[p];
			}
			else
			{
				lson[p] = r;  dad[r] = p;  return;
			}
		}
		for (i = 1; i < F; i++)
			if ((cmp = key[i] - pub[i]) != 0)
				break;
		if (i > match_length)
		{
			match_position = p;
			if ((match_length = i) >= F)
				break;
		}
	}
	dad[r] = dad[p];  lson[r] = lson[p];  rson[r] = rson[p];
	dad[lson[p]] = r;  dad[rson[p]] = r;

	if (rson[dad[p]] == p)
		rson[dad[p]] = r;
	else
		lson[dad[p]] = r;
	dad[p] = NIL;  /* remove p */
}

static void DeleteNode(int p)  /* deletes node p from tree */
{
	int  q;
	
	if (dad[p] == NIL) return;  /* not in tree */
	if (rson[p] == NIL) q = lson[p];
	else if (lson[p] == NIL) q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != NIL) {
			do {  q = rson[q];  } while (rson[q] != NIL);
			rson[dad[q]] = lson[q];  dad[lson[q]] = dad[q];
			lson[q] = lson[p];  dad[lson[p]] = q;
		}
		rson[q] = rson[p];  dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p) rson[dad[p]] = q;  else lson[dad[p]] = q;
	dad[p] = NIL;
}

static int fEncode;



/*********************************************************************
 *
 * InitSDQWrite
 *
 * SYNOPSIS
 *		void InitSDQWrite(void)
 *
 * PURPOSE
 *		Initialize packed data writer.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void InitSDQWrite(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitSDQWrite";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	fEncode = 0;

} /* InitSDQWrite */



/*********************************************************************
 *
 * SDQWrite
 *
 * SYNOPSIS
 *		int SDQWrite(int hFileOut, char *pBuf, int iMac)
 *
 * PURPOSE
 *		Pack data pointed to by <pBuf>, and write packed data to <hFileOut>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns number of packed bytes written, or -1 if error.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int SDQWrite(int hFileOut, char *pBuf, int iMac)
{
	static int len, r, s;
	static int cch, iCrnt;
	static int	last_match_length;
	static int	code_buf_ptr;
	static int	iFlags;
	static unsigned char code_buf[64];
	static unsigned char mask;

	unsigned codesize = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SDQWrite";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	iCrnt = 0;

	if (fEncode == 0)
	{
		InitTree();  /* initialize trees */

		code_buf[iFlags] = 0;  /* code_buf[1..16] saves eight units of code, and
			code_buf[iFlags] works as eight flags, "1" representing that the unit
			is an unencoded letter (1 byte), "0" a position-and-length pair
			(2 bytes).  Thus, eight units require at most 16 bytes of code. */

		code_buf_ptr = mask = 1;

		iFlags = 0;		len = 0;		s = 0;	r = N - F;

//		for (i = s; i < r; i++) text_buf[i] = ' ';  /* Clear the buffer with
//			any character that will appear often. */

		fEncode = 1;
	}

	if (fEncode == 1)
	{
		/* Read F bytes into the last F bytes of the buffer */

		iCrnt = min(F-len, iMac);
		memcpy(&text_buf[r+len], pBuf, iCrnt);

		len += iCrnt;
		if (len < F)
			return codesize;

		/* Finally, insert the whole string just read.  The
			global variables match_length and match_position are set. */

		InsertNode(r);

		fEncode = 2;
	}

	do
	{
		if (fEncode == 2 || fEncode == 5)
		{
			if (match_length > len) match_length = len;  /* match_length
				may be spuriously long near the end of text. */
			if (match_length <= THRESHOLD)
			{
				match_length = 1;  /* Not long enough match.  Send one byte. */
				code_buf[iFlags] |= mask;  /* 'send one byte' flag */
				code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
			}
			else
			{
				int offset = r - match_position;

				/* Send position and length pair. Note match_length > THRESHOLD. */

				code_buf[code_buf_ptr++] = (unsigned char) ((offset << 3) |
														  	(match_length - (THRESHOLD + 1)));
			}
			if ((mask <<= 1) == 0)					/* Shift mask left one bit. */
			{
				if (code_buf_ptr < 64-9)
				{
					iFlags = code_buf_ptr++;
				}
				else
				{
					if (write(hFileOut, code_buf, code_buf_ptr) != code_buf_ptr)
						return -1;
					codesize += code_buf_ptr;
					iFlags = 0;
					code_buf_ptr = 1;
				}
				code_buf[iFlags] = 0;
				mask = 1;
			}

			last_match_length = match_length;

			cch = 0;

			fEncode++;
		}

		if (fEncode == 3)
		{
			for ( ; cch < last_match_length; cch++)
			{
				char c;

				if (iCrnt >= iMac)			/* More bytes to write? */
					return codesize;

				c = pBuf[iCrnt++];

				DeleteNode((s+(N-MAXOFFSET-F)) & (N-1));	/* Delete old strings and */
				text_buf[s] = c;	/* read new bytes */

				/* If the position is near the end of buffer, extend the
					buffer to make string comparison easier. */

				if (s < F - 1)
					text_buf[s + N] = c;

				/* Since this is a ring buffer, increment the position
			   	modulo N. */

				s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);

				InsertNode(r);	/* Register the string in text_buf[r..r+F-1] */
			}
			fEncode = 2;
		}

		if (fEncode == 4 || fEncode == 6)
		{
			while (cch++ < last_match_length)		/* After the end of text, */
			{
				DeleteNode((s+(N-MAXOFFSET-F)) & (N-1));	/* no need to read, but */
				s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
				if (--len) InsertNode(r);		/* buffer may not be empty. */
			}
			fEncode = 5;
		}
	} while (len > 0);	/* until length of string to be processed is zero */

	/* Mark end-of-file with offset and length of zero */

	do
	{
		code_buf[code_buf_ptr++] = 0;
	}
	while ((mask <<= 1) != 0);					/* Shift mask left one bit. */

	if (code_buf_ptr > 1)							/* Send remaining code. */
	{
		if (write(hFileOut, code_buf, code_buf_ptr) != code_buf_ptr)
			return -1;
		codesize += code_buf_ptr;
	}
	return codesize;
} /* SDQWrite */



/*********************************************************************
 *
 * SDQWriteLast
 *
 * SYNOPSIS
 *		int SDQWriteLast(int hFileOut)
 *
 * PURPOSE
 *		Flush any packed bytes that remain to be written.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns number of packed bytes flushed (written), or -1 if error.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int SDQWriteLast(int hFileOut)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SDQWriteLast";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	fEncode = 4;
	return SDQWrite(hFileOut, NULL, 0);
} /* SDQWriteLast */

#endif

/* DECOMPRESS */

static int cValidBits;
static int iFlagInCrnt;
static int iByteInCrnt;
static int iBytesInMac;
static int iByteOutCrnt;

static BOOL fPartialCopy;
static int iOffset;
static int iLength;

static BOOL fSDQReadEOF;

static unsigned char ubIn[cBytesInMac];
static unsigned char ubOut[cBytesOutMac];



/*********************************************************************
 *
 * InitSDQRead
 *
 * SYNOPSIS
 *		void InitSDQRead(void)
 *
 * PURPOSE
 *		Initialize the packed data reader.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void InitSDQRead(void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InitSDQRead";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	cValidBits = 0;
	iFlagInCrnt = 0;
	iByteInCrnt = 0;
	iBytesInMac = 0;
	iByteOutCrnt = 0;
	fPartialCopy = FALSE;
	fSDQReadEOF = FALSE;
} /* InitSDQRead */



/*********************************************************************
 *
 * SDQRead
 *
 * SYNOPSIS
 *		int SDQRead(int hFile, unsigned char *pBuf, unsigned uLen)
 *
 * PURPOSE
 *		Read packed data, unpack <uLen> bytes into <pBuf>.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns the number of packed bytes read, or -1 if error.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int SDQRead(int hFile, unsigned char *pBuf, unsigned uLen)
{

	unsigned int cBytesOut = 0;
	unsigned int cBytesRead = 0;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SDQRead";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (fSDQReadEOF)
		return 0;

	if (fPartialCopy)
	{
		int i;
		int iByteOutSrc = (iByteOutCrnt - iOffset) & (cBytesOutMac-1);
		int cBytesCopy = min(uLen, iLength);

		for (i = 0; i < cBytesCopy; i++)
		{
			*pBuf++ = ubOut[iByteOutCrnt++] = ubOut[iByteOutSrc++];
			iByteOutCrnt &= (cBytesOutMac-1);
			iByteOutSrc &= (cBytesOutMac-1);
		}

		if ((fPartialCopy = cBytesCopy < iLength) != 0)
		{
			iOffset -= cBytesCopy;
			iLength -= cBytesCopy;
		}
		cBytesOut += cBytesCopy;
	}

	while (cBytesOut < uLen)
	{
		if (!cValidBits)
		{
			if (iByteInCrnt < iBytesInMac)
			{
				iFlagInCrnt = iByteInCrnt++;
			}
			else
			{
				if ((iBytesInMac = read(hFile, ubIn, cBytesInMac)) != cBytesInMac)
					return -1;
				cBytesRead += cBytesInMac;
				iFlagInCrnt = 0;
				iByteInCrnt = 1;
			}
			cValidBits = 8;
		}
		if (ubIn[iFlagInCrnt] & 1)
		{
			*pBuf++ = ubOut[iByteOutCrnt++] = ubIn[iByteInCrnt++];
			iByteOutCrnt &= (cBytesOutMac-1);

			cBytesOut++;
		}
		else
		{
			int i;
			int iByteOutSrc;
			int cBytesCopy;
			unsigned char ub;

			if (!(ub = ubIn[iByteInCrnt++]))
			{
				fSDQReadEOF = TRUE;
				return cBytesRead;
			}

			iOffset = ub >> 3;
			iLength = (ub & 7)+THRESHOLD+1;
			iByteOutSrc = (iByteOutCrnt - iOffset) & (cBytesOutMac-1);
			cBytesCopy = min(uLen - cBytesOut, iLength);

			for (i = 0; i < cBytesCopy; i++)
			{
				*pBuf++ = ubOut[iByteOutCrnt++] = ubOut[iByteOutSrc++];
				iByteOutCrnt &= (cBytesOutMac-1);
				iByteOutSrc &= (cBytesOutMac-1);
			}

			if ((fPartialCopy = cBytesCopy < iLength) != 0)
			{
				iLength -= cBytesCopy;
			}

			cBytesOut += cBytesCopy;
		}
		ubIn[iFlagInCrnt] >>= 1;
		cValidBits--;
	}
	return cBytesRead;
} /* SDQRead */
