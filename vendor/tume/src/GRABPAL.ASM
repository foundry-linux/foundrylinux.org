;* 
;* The contents of this file are subject to the Mozilla Public 
;* License Version 1.1 (the "License"); you may not use this file 
;* except in compliance with the License. You may obtain a copy of 
;* the License at http://www.mozilla.org/MPL/ 
;* 
;* Software distributed under the License is distributed on an "AS 
;* IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
;* implied. See the License for the specific language governing 
;* rights and limitations under the License. 
;* 
;* The Original Code is tUME: the Universal Map Editor, released 
;* June 18, 2000. 
;* 
;* The Initial Developer of the Original Code is Echidna. 
;* Portions created by Echidna are Copyright (c) 1989-2000 Echidna. 
;* All Rights Reserved. 
;* 
;* Contributor(s): 
;* 

;**
;** GRABPAL.ASM
;**
;** PROGRAMMER : Dan Chang
;**    VERSION : 00.000
;**   MODIFIED : 07/27/94
;**       TABS : 09 17
;**
;** DESCRIPTION
;**	Converts palette to SNES or Genesis palette.
;**
;** HISTORY
;**	03/21/94 Monday (dcc) - store in 1..128 instead of 0..255
;**			The first word now has the number of bytes downloaded.
;**

; This routine has been extended to deal with
;   negative palette values - these get set to 0, and
;   colors > 255 - these get set to 255.

	IDEAL

	MODEL	HUGE

	PUBLIC	_SNESGrabPalette
	PUBLIC	_GenesisGrabPalette
	EXTRN	_uwPalette

uwPaletteMax	=	(128)
cSNESBytes	=	(256)
cGenesisBytes	=	(128)

	CODESEG

;static BOOL SNESGrabPalette(CS_Color *pc)
;{
;	WORD w;
;
;	for (w = 1; w <= uwPaletteMax; w++)
;	{
;		uwPalette[w] =	((pc->Red   >> 3) <<  0) |
;				((pc->Green >> 3) <<  5) |
;				((pc->Blue  >> 3) << 10) ;
;		pc++;
;	}
;	return TRUE;
;}

	PROC	_SNESGrabPalette
	ARG	paloff:WORD,palseg:WORD

	push	bp
	mov	bp, sp
	push	ds
	push	di
	push	si

	cld

	mov	ax, SEG _uwPalette
	mov	ds, ax
	les	di, [dword ptr _uwPalette]
	mov	ax, cSNESBytes
	stosw
	lds	si, [dword ptr paloff]

	mov	bx, uwPaletteMax
	mov	cl, 3
@@loop:
	lodsw
	or	ah, ah
	js	@@negR
	jnz	@@veryR
	mov	ch, al			;ch = 8-bit R value
@@doG:
	lodsw
	or	ah, ah
	js	@@negG
	jnz	@@veryG
	mov	ah, al			;ah = 8-bit G value
	mov	al, ch

	shr	ah, cl			;ah = 5-bit G value
	shr	ax, cl			;9..5: 5-bit G, 4..0: 5-bit R
@@doB:
	mov	dx, ax

	lodsw
	or	ah, ah
	js	@@negB
	jnz	@@veryB
	shr	al, 1

	mov	ah, dh
	xor	ah, al
	and	ah, 11b
	xor	ah, al
	mov	al, dl
@@doC:
	stosw
	add	si, 7			;skip to next CS_Color record

	dec	bx
	jne	@@loop

	mov	ax, 1			;return TRUE

	pop	si
	pop	di
	pop	ds
	pop	bp
	ret
@@negR:
	sub	ch, ch
	jmp	@@doG
@@veryR:
	mov	ch, 0FFh
	jmp	@@doG
@@negG:
	mov	al, ch
	shr	al, cl
	and	ax, 11111b
	jmp	@@doB
@@veryG:
	mov	al, ch
	shr	al, cl
	or	ax, 1111100000b
	jmp	@@doB
@@negB:
	mov	ax, dx
	and	ax, 1111111111b
	jmp	@@doC
@@veryB:
	mov	ax, dx
	or	ax, 111110000000000b
	jmp	@@doC
	ENDP

;static BOOL GenesisGrabPalette(CS_Color *pc)
;{
;	WORD w;
;
;	for (w = 1; w <= uwPaletteMax; w++)
;	{
;		// Recall that 68000's and 8086 store bytes differently.
;		// This code is meant to run on a 8086 (duh!), and
;		// generate data for a 68000.
;
;		uwPalette[w] =	((pc->Red   >> 5) <<  9) |
;				((pc->Green >> 5) << 13) |
;				((pc->Blue  >> 5) <<  1) ;
;		pc++;
;	}
;	return TRUE;
;}

	PROC	_GenesisGrabPalette
	ARG	paloff:WORD,palseg:WORD

	push	bp
	mov	bp, sp
	push	ds
	push	di
	push	si

	cld

	mov	ax, SEG _uwPalette
	mov	ds, ax
	les	di, [dword ptr _uwPalette]
	mov	ax, cGenesisBytes SHL 8
	stosw
	lds	si, [dword ptr paloff]

	mov	bx, uwPaletteMax
	mov	cl, 4
@@loop:
	lodsw
	or	ah, ah
	js	@@negR
	jnz	@@veryR

	shr	al, cl
	and	al, 1110b
	mov	ch, al			;ch 3..1: 3-bit R value
@@doG:
	lodsw
	or	ah, ah
	js	@@negG
	jnz	@@veryG
	and	al, 11100000b
	or	ch, al			;7..5: 3-bit G, 3..1: 3-bit R
@@negG:
@@doB:
	lodsw
	or	ah, ah
	js	@@negB
	jnz	@@veryB
	shr	al, cl
	and	al, 1110b
@@doC:
	mov	ah, ch
	stosw
	add	si, 7			;skip to next CS_Color record

	dec	bx
	jne	@@loop

	mov	ax, 1			;return TRUE

	pop	si
	pop	di
	pop	ds
	pop	bp
	ret
@@negR:
	sub	ch, ch
	jmp	@@doG
@@veryR:
	mov	ch, 1110b
	jmp	@@doG
@@veryG:
	mov	al, 11100000b
	or	ch, al
	jmp	@@doB
@@negB:
	sub	al, al
	jmp	@@doC
@@veryB:
	mov	al, 1110b
	jmp	@@doC
	ENDP
	ENDS
	END

