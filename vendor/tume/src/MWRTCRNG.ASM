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
;** MWRTCRNG.ASM
;**
;** PROGRAMMER : Dan Chang
;**    VERSION : 00.000
;**    CREATED : 04/14/91
;**   MODIFIED : 07/27/94
;**       TABS : 09 17
;**
;** DESCRIPTION
;**	Write <NumColors> from <ColorPtr> to palette register <ColorReg>.
;**
;** HISTORY
;**	04/14/91 Sunday (dcc) - translated from Juan Alvarado's C routine.
;**	07/10/92 Friday (dcc) - added CLD to make sure data copied correctly.
;**
;**

	IDEAL

	INCLUDE "EUIMDL.inc"
	WSEG	MGFX_TEXT

	PUBLIC	_MCGA_ASM_WriteColors

	PUBLIC	_ActivationFlag
	PUBLIC	_CopyRanges
	PUBLIC	_ColorStuffPause

;	WSEG	MGFX_TEXT

; Note: if you change CopyRangeType structure,
; please fix SIZEOF_CopyRange constant below!

	STRUC	CopyRangeType
NumColors	dw	?
ColorPtr	dd	?
ColorReg	dw	?
	ENDS

SIZEOF_CopyRange	equ	(8)

	EVEN

_ColorStuffPause	dw	?	;do work or not (pause)
_ActivationFlag		dw	?	;do work or not
_CopyRanges		dd	?	;pointer to data to copy

	MPROC	_MCGA_ASM_WriteColors

	mov	ax, [cs:_ColorStuffPause]
	or	ax, ax
	jnz	@@OutDone		;=> nothing to do!
	mov	ax, [cs:_ActivationFlag]
	or	ax, ax
	jz	@@OutDone		;=> nothing to do!

	push	ds
	push	si
	push	es
	push	di

	cld
	mov	dx, 03C9h
	les	bx, [cs:_CopyRanges]
@@forLoop:
	mov	cx, [es:bx+CopyRangeType.NumColors]
	or	cx, cx
	jz	@@exit			;=> no colors to copy

	mov	al, [byte ptr es:bx+CopyRangeType.ColorReg]

	lds	si, [es:bx+CopyRangeType.ColorPtr]	;-> 1st byte to write

	dec	dx	;= 03C8h
	out	dx, al

	inc	dx	;= 03C9h
@@writeLoop:
	lodsb		;Red
	shr	al,2
	out	dx, al
	lodsb		;Green
	shr	al,2
	out	dx, al
	lodsb		;Blue
	shr	al,2
	out	dx, al
	mov	ax,ds
	mov	es,ax
	mov	di,si
	lodsb		;Flag
	and	al,0FEh
	mov	[es:di],al
	loop	@@writeLoop

	add	bx, SIZEOF_CopyRange		;do next CopyRange
	jmp	@@forLoop
@@exit:
	mov	[cs:_ActivationFlag], cx	;= 0

	pop	di
	pop	es
	pop	si
	pop	ds
@@OutDone:
	ret
	ENDP

	ENDS
	END
